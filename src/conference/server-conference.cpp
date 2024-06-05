/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone
 * (see https://gitlab.linphone.org/BC/public/liblinphone).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "call/call-log.h"
#include "conference/params/media-session-params-p.h"
#include "conference/participant-device-identity.h"
#include "conference/participant-info.h"
#include "conference/participant.h"
#include "conference/session/media-session-p.h"
#include "conference/session/mixers.h"
#include "core/core-p.h"
#include "factory/factory.h"
#include "linphone/api/c-chat-room-cbs.h"
#include "linphone/api/c-event.h"
#include "logger/logger.h"
#include "participant.h"
#include "server-conference.h"
#ifdef HAVE_ADVANCED_IM
#include "chat/chat-room/server-chat-room.h"
#include "conference/handlers/server-conference-list-event-handler.h"
#include "handlers/server-conference-event-handler.h"
#endif

// TODO: Remove me later.
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

ServerConference::ServerConference(const shared_ptr<Core> &core,
                                   const std::shared_ptr<Address> &myAddress,
                                   CallSessionListener *listener,
                                   const std::shared_ptr<ConferenceParams> params)
    : Conference(core, myAddress, listener, params) {
}

ServerConference::~ServerConference() {
	if ((mState != ConferenceInterface::State::Terminated) && (mState != ConferenceInterface::State::Deleted)) {
		terminate();
	}
	cleanup();
}

void ServerConference::initFromDb(BCTBX_UNUSED(const std::shared_ptr<Participant> &me),
                                  const ConferenceId conferenceId,
                                  const unsigned int lastNotifyId,
                                  BCTBX_UNUSED(bool hasBeenLeft)) {
	mMe = Participant::create(getSharedFromThis(), mConfParams->getMe());
	setLastNotify(lastNotifyId);
	setConferenceId(conferenceId);
	if (mConfParams->chatEnabled()) {
		getCore()->getPrivate()->registerListener(this);
#ifdef HAVE_ADVANCED_IM
		auto chatRoom =
		    dynamic_pointer_cast<ServerChatRoom>((new ServerChatRoom(getCore(), getSharedFromThis()))->toSharedPtr());
		setChatRoom(chatRoom);
		createEventHandler(this);
#endif // HAVE_ADVANCED_IM
	}

	setState(ConferenceInterface::State::Instantiated);
	const auto &conferenceAddress = mConfParams->getConferenceAddress();
	if (conferenceAddress) {
		setConferenceAddress(conferenceAddress);
	}
}

void ServerConference::init(SalCallOp *op, ConferenceListener *confListener) {

	// Set last notify to 1 in order to ensure that the 1st notify to remote conference is correctly processed
	// Remote conference sets last notify to 0 in its constructor
	setLastNotify(1);
	mMe = Participant::create(getSharedFromThis(), mConfParams->getMe());
	mOrganizer = op ? Address::create(op->getFrom()) : mMe->getAddress();

	createEventHandler(confListener);
	if (mConfParams->chatEnabled()) {
		if (op) {
			const auto from = Address::create(op->getFrom());
			const auto to = Address::create(op->getTo());
			shared_ptr<CallSession> session = getMe()->createSession(*this, nullptr, true, this);
			session->configure(LinphoneCallIncoming, nullptr, op, from, to);
		}
		mConfParams->enableLocalParticipant(false);
		getCore()->getPrivate()->registerListener(this);
#ifdef HAVE_ADVANCED_IM
		auto chatRoom =
		    dynamic_pointer_cast<ServerChatRoom>((new ServerChatRoom(getCore(), getSharedFromThis()))->toSharedPtr());
		setChatRoom(chatRoom);
#endif // HAVE_ADVANCED_IM
		setState(ConferenceInterface::State::Instantiated);
	} else {
		setState(ConferenceInterface::State::Instantiated);
		LinphoneCore *lc = getCore()->getCCore();
		if (op) {
			configure(op);
		} else {
			// Update proxy contact address to add conference ID
			// Do not use organizer address directly as it may lack some parameter like gruu
			auto account = getCore()->lookupKnownAccount(mOrganizer, true);
			char *contactAddressStr = nullptr;
			if (account && account->getOp()) {
				contactAddressStr = sal_address_as_string(account->getOp()->getContactAddress());
			} else {
				LinphoneAddress *cAddress = mOrganizer->toC();
				contactAddressStr =
				    ms_strdup(linphone_core_find_best_identity(lc, const_cast<LinphoneAddress *>(cAddress)));
			}
			std::shared_ptr<Address> contactAddress = Address::create(contactAddressStr);
			char confId[ServerConference::sConfIdLength];
			belle_sip_random_token(confId, sizeof(confId));
			contactAddress->setUriParam("conf-id", confId);
			if (contactAddressStr) {
				ms_free(contactAddressStr);
			}

			setConferenceAddress(contactAddress);
			mMe->setRole(Participant::Role::Speaker);
			mMe->setAdmin(true);
			mMe->setFocus(true);

			bool_t eventLogEnabled = FALSE;
#ifdef HAVE_ADVANCED_IM
			eventLogEnabled =
			    linphone_config_get_bool(linphone_core_get_config(lc), "misc", "conference_event_log_enabled", TRUE);
#endif // HAVE_ADVANCED_IM

			if (!eventLogEnabled) {
				setConferenceId(ConferenceId(contactAddress, contactAddress));
			}

#ifdef HAVE_DB_STORAGE
			auto conferenceInfo = createOrGetConferenceInfo();
			auto &mainDb = getCore()->getPrivate()->mainDb;
			if (mainDb) {
				mainDb->insertConferenceInfo(conferenceInfo);
			}
#endif // HAVE_DB_STORAGE
		}
		if (mConfParams->videoEnabled() && !linphone_core_conference_server_enabled(lc)) {
			lWarning()
			    << "Video capability in a conference is not supported when a device that is not a server is hosting "
			       "a conference.";
			mConfParams->enableVideo(false);
		}
	}
}

void ServerConference::createEventHandler(BCTBX_UNUSED(ConferenceListener *confListener),
                                          BCTBX_UNUSED(bool addToListEventHandler)) {
#ifdef HAVE_ADVANCED_IM
	LinphoneCore *lc = getCore()->getCCore();
	bool eventLogEnabled =
	    !!linphone_config_get_bool(linphone_core_get_config(lc), "misc", "conference_event_log_enabled", TRUE);
	if (eventLogEnabled) {
		eventHandler = std::make_shared<ServerConferenceEventHandler>(getSharedFromThis(), confListener);
		const auto chatEnabled = mConfParams->chatEnabled();
		if (chatEnabled && getCore()->getPrivate()->serverListEventHandler && getConferenceId().isValid()) {
			getCore()->getPrivate()->serverListEventHandler->addHandler(eventHandler);
		}
		addListener(eventHandler);
	} else {
#endif // HAVE_ADVANCED_IM
		lInfo() << "Unable to add listener to local conference as conference event package (RFC 4575) is disabled or "
		           "the SDK was not compiled with ENABLE_ADVANCED_IM flag set to on";
#ifdef HAVE_ADVANCED_IM
	}
#endif // HAVE_ADVANCED_IM
}

bool ServerConference::validateNewParameters(const ConferenceParams &newConfParams) const {
	if (!mConfParams) {
		return true;
	}

	const auto &oldFactoryAddress = mConfParams->getConferenceFactoryAddress();
	const auto &newFactoryAddress = newConfParams.getConferenceFactoryAddress();
	if (((oldFactoryAddress == nullptr) != (newFactoryAddress == nullptr)) ||
	    (oldFactoryAddress && newFactoryAddress && (*oldFactoryAddress != *newFactoryAddress))) {
		lError() << "Factory address change is not allowed: actual "
		         << (oldFactoryAddress ? oldFactoryAddress->toString() : std::string("sip:")) << " new value "
		         << (newFactoryAddress ? newFactoryAddress->toString() : std::string("sip:"));
		return false;
	}

	const auto &oldConferenceAddress = mConfParams->getConferenceAddress();
	const auto &newConferenceAddress = newConfParams.getConferenceAddress();
	if (((oldConferenceAddress == nullptr) != (newConferenceAddress == nullptr)) ||
	    (oldConferenceAddress && newConferenceAddress && (*oldConferenceAddress != *newConferenceAddress))) {
		lError() << "Conference address change is not allowed: actual "
		         << (oldConferenceAddress ? oldConferenceAddress->toString() : std::string("sip:")) << " new value "
		         << (newConferenceAddress ? newConferenceAddress->toString() : std::string("sip:"));
		return false;
	}

	if (mConfParams->getSecurityLevel() != newConfParams.getSecurityLevel()) {
		lError() << "Conference security level change is not allowed: actual " << mConfParams->getSecurityLevel()
		         << " new value " << newConfParams.getSecurityLevel();
		return false;
	}

	return true;
}

bool ServerConference::update(const ConferenceParamsInterface &newParameters) {
	/* Only adding or removing video is supported. */
	bool previousVideoEnablement = mConfParams->videoEnabled();
	bool previousAudioEnablement = mConfParams->audioEnabled();
	bool previousChatEnablement = mConfParams->chatEnabled();
	const ConferenceParams &newConfParams = static_cast<const ConferenceParams &>(newParameters);
	if (!validateNewParameters(newConfParams)) {
		return false;
	}
	mConfParams = ConferenceParams::create(newConfParams);

	if (!linphone_core_conference_server_enabled(getCore()->getCCore()) && mConfParams->videoEnabled()) {
		lWarning() << "Video capability in a conference is not supported when a device that is not a server is hosting "
		              "a conference.";
		mConfParams->enableVideo(false);
	}

	bool newVideoEnablement = mConfParams->videoEnabled();
	bool newAudioEnablement = mConfParams->audioEnabled();
	bool newChatEnablement = mConfParams->chatEnabled();

	// Update endpoints only if audio or video settings have changed
	if ((newVideoEnablement != previousVideoEnablement) || (newAudioEnablement != previousAudioEnablement)) {
		/* Don't forget the local participant. For simplicity, a removeLocalEndpoint()/addLocalEndpoint() does the job.
		 */
		removeLocalEndpoint();
		addLocalEndpoint();
	}

	if ((newChatEnablement != previousChatEnablement) || (newVideoEnablement != previousVideoEnablement) ||
	    (newAudioEnablement != previousAudioEnablement)) {
		time_t creationTime = time(nullptr);
		notifyAvailableMediaChanged(creationTime, false, getMediaCapabilities());
	}

	bool mediaChanged = false;
	for (auto &meDev : mMe->getDevices()) {
		mediaChanged = false;
		mediaChanged |= meDev->setStreamCapability(
		    (newAudioEnablement ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive),
		    LinphoneStreamTypeAudio);
		mediaChanged |= meDev->setStreamCapability(
		    (newVideoEnablement ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive),
		    LinphoneStreamTypeVideo);
		mediaChanged |= meDev->setStreamCapability(
		    (newChatEnablement ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive),
		    LinphoneStreamTypeText);

		if (mediaChanged) {
			time_t creationTime = time(nullptr);
			notifyParticipantDeviceMediaCapabilityChanged(creationTime, false, mMe, meDev);
		}
	}
	return true;
}
void ServerConference::updateConferenceInformation(SalCallOp *op) {
	auto remoteContact = op->getRemoteContactAddress();
	if (remoteContact) {
		char *salAddress = sal_address_as_string(remoteContact);
		std::shared_ptr<Address> address = Address::create(std::string(salAddress));
		if (salAddress) {
			ms_free(salAddress);
		}
		auto invited = (findInvitedParticipant(address) != nullptr);
		std::shared_ptr<Address> remoteAddress =
		    Address::create((op->getDir() == SalOp::Dir::Incoming) ? op->getFrom() : op->getTo());

		if (findParticipantDevice(remoteAddress, address) || invited || address->weakEqual(*mOrganizer)) {
			lInfo() << "Updating conference informations of conference " << *getConferenceAddress();
			const auto &remoteMd = op->getRemoteMediaDescription();

			const auto times = remoteMd->times;

			// The following informations are retrieved from the received INVITE:
			// - start and end time from the SDP active time attribute
			// - conference active media:
			//    - if the SDP has at least one active audio stream, audio is enabled
			//    - if the SDP has at least one active video stream, video is enabled
			// - Subject is got from the "Subject" header in the INVITE
			const auto audioEnabled = (remoteMd->nbActiveStreamsOfType(SalAudio) > 0);
			auto videoEnabled = (linphone_core_conference_server_enabled(getCore()->getCCore()))
			                        ? (remoteMd->nbActiveStreamsOfType(SalVideo) > 0)
			                        : false;
			if (!linphone_core_conference_server_enabled(getCore()->getCCore())) {
				lWarning() << "Video capability in a conference is not supported when a device that is not a server is "
				              "hosting a conference.";
			}

			bool previousVideoEnablement = mConfParams->videoEnabled();
			bool previousAudioEnablement = mConfParams->audioEnabled();

			mConfParams->enableAudio(audioEnabled);
			mConfParams->enableVideo(videoEnabled);

			if ((mConfParams->videoEnabled() != previousVideoEnablement) ||
			    (mConfParams->audioEnabled() != previousAudioEnablement)) {
				time_t creationTime = time(nullptr);
				notifyAvailableMediaChanged(creationTime, false, getMediaCapabilities());
			}
			setSubject(op->getSubject());

			mConfParams->enableOneParticipantConference(true);

			auto session = const_pointer_cast<MediaSession>(static_pointer_cast<MediaSession>(getMe()->getSession()));
			auto msp = session->getPrivate()->getParams();
			msp->enableAudio(audioEnabled);
			msp->enableVideo(videoEnabled);
			msp->getPrivate()->setInConference(true);

			if (times.size() > 0) {
				const auto [startTime, endTime] = times.front();
				mConfParams->setStartTime(startTime);
				mConfParams->setEndTime(endTime);
				msp->getPrivate()->setStartTime(startTime);
				msp->getPrivate()->setEndTime(endTime);
			}

			mMe->setRole(Participant::Role::Speaker);
			mMe->setAdmin(true);
			mMe->setFocus(true);

			const auto resourceList = op->getContentInRemote(ContentType::ResourceLists);
			bool isEmpty = !resourceList || resourceList.value().get().isEmpty();
			fillInvitedParticipantList(op, mOrganizer, isEmpty);

			const auto &conferenceInfo =
			    createConferenceInfoWithCustomParticipantList(mOrganizer, mInvitedParticipants);
			auto infoState = ConferenceInfo::State::New;
			if (isEmpty) {
				infoState = ConferenceInfo::State::Cancelled;
			} else {
				infoState = ConferenceInfo::State::Updated;
			}
			conferenceInfo->setState(infoState);

#ifdef HAVE_DB_STORAGE
			auto &mainDb = getCore()->getPrivate()->mainDb;
			if (mainDb) {
				lInfo()
				    << "Inserting conference information to database in order to be able to recreate the conference "
				    << *getConferenceAddress() << " in case of restart";
				mConferenceInfoId = mainDb->insertConferenceInfo(conferenceInfo);
			}
#endif // HAVE_DB_STORAGE
			auto callLog = session->getLog();
			if (callLog) {
				callLog->setConferenceInfo(conferenceInfo);
				callLog->setConferenceInfoId(mConferenceInfoId);
			}
			if (isEmpty) {
				setState(ConferenceInterface::State::TerminationPending);
			}
		} else {
			lWarning() << "Device with address " << address
			           << " is not allowed to update the conference because they have not been invited nor are "
			              "participants to conference "
			           << *getConferenceAddress() << " nor are the organizer";
		}
	}
}

void ServerConference::configure(SalCallOp *op) {
	LinphoneCore *lc = getCore()->getCCore();
	bool admin = ((sal_address_has_param(op->getRemoteContactAddress(), "admin") &&
	               (strcmp(sal_address_get_param(op->getRemoteContactAddress(), "admin"), "1") == 0)));

	std::shared_ptr<ConferenceInfo> info = nullptr;
#ifdef HAVE_DB_STORAGE
	auto &mainDb = getCore()->getPrivate()->mainDb;
	if (mainDb) {
		info = getCore()->getPrivate()->mainDb->getConferenceInfoFromURI(Address::create(op->getTo()));
	}
#endif // HAVE_DB_STORAGE

	bool audioEnabled = true;
	std::string subject;
	time_t startTime = ms_time(NULL);
	time_t endTime = ms_time(NULL);

	time_t startTimeSdp = 0;
	time_t endTimeSdp = 0;

	const auto &remoteMd = op->getRemoteMediaDescription();
	const auto times = remoteMd->times;
	if (times.size() > 0) {
		startTimeSdp = times.front().first;
		endTimeSdp = times.front().second;
	}

	const bool createdConference = (info && info->isValidUri());
	// If start time or end time is not -1, then the client wants to update the conference
	const auto isUpdate = (admin && ((startTimeSdp != -1) || (endTimeSdp != -1)) && info);

	ConferenceParams::SecurityLevel securityLevel = ConferenceParams::SecurityLevel::None;
	if (createdConference && info) {
		securityLevel = info->getSecurityLevel();
	} else {
		const auto &toAddrStr = op->getTo();
		Address toAddr(toAddrStr);
		if (toAddr.hasUriParam(Conference::SecurityModeParameter)) {
			securityLevel = ConferenceParams::getSecurityLevelFromAttribute(
			    toAddr.getUriParamValue(Conference::SecurityModeParameter));
		}
	}

	mConfParams->setSecurityLevel(securityLevel);
	if (mMixerSession) {
		mMixerSession->setSecurityLevel(mConfParams->getSecurityLevel());
	}

	if (isUpdate || (admin && !createdConference)) {
		// The following informations are retrieved from the received INVITE:
		// - start and end time from the SDP active time attribute
		// - conference active media:
		//    - if the SDP has at least one active audio stream, audio is enabled
		//    - if the core is a conference server, video is enabled
		// - Subject is got from the "Subject" header in the INVITE
		audioEnabled = (remoteMd->nbActiveStreamsOfType(SalAudio) > 0);
		if (!op->getSubject().empty()) {
			subject = op->getSubject();
		}
		mOrganizer = Address::create(op->getFrom());

		startTime = startTimeSdp;
		if (startTime <= 0) {
			startTime = ms_time(NULL);
		}
		endTime = endTimeSdp;
		if (endTime <= 0) {
			endTime = -1;
		}
		fillInvitedParticipantList(op, mOrganizer, false);
	} else if (info) {
		subject = info->getSubject();
		mOrganizer = info->getOrganizerAddress();

		startTime = info->getDateTime();
		const auto duration = info->getDuration();
		if ((duration > 0) && (startTime >= 0)) {
			endTime = startTime + static_cast<time_t>(duration) * 60;
		} else {
			endTime = -1;
		}
		mInvitedParticipants = info->getParticipants();
	}

	mConfParams->enableAudio(audioEnabled);
	auto videoEnabled = linphone_core_video_enabled(lc);
	mConfParams->enableVideo(videoEnabled);

	if (!subject.empty()) {
		mConfParams->setSubject(subject);
	}
	mConfParams->enableLocalParticipant(false);
	mConfParams->enableOneParticipantConference(true);

	mConfParams->setStartTime(startTime);
	mConfParams->setEndTime(endTime);

	if (!isUpdate && !info) {
		// Set joining mode only when creating a conference
		bool immediateStart = (startTimeSdp < 0);
		const auto joiningMode =
		    (immediateStart) ? ConferenceParams::JoiningMode::DialOut : ConferenceParams::JoiningMode::DialIn;
		mConfParams->setJoiningMode(joiningMode);
	}

	if (admin && (isUpdate || !createdConference)) {
		std::shared_ptr<Address> conferenceAddress = Address::create(op->getTo());
		shared_ptr<CallSession> session = getMe()->createSession(*this, nullptr, true, nullptr);
		session->configure(LinphoneCallIncoming, nullptr, op, mOrganizer, conferenceAddress);
		auto msp = dynamic_pointer_cast<MediaSession>(session)->getMediaParams()->clone();
		msp->enableAudio(audioEnabled);
		msp->enableVideo(videoEnabled);
		msp->getPrivate()->enableToneIndications(false);
		msp->getPrivate()->setInConference(true);
		msp->getPrivate()->setStartTime(startTime);
		msp->getPrivate()->setEndTime(endTime);
		// Set parameters here in order to set the start and end time as well disabling tone notifications
		dynamic_pointer_cast<MediaSession>(session)->setParams(msp);
		delete msp;
	}

	mMe->setRole(Participant::Role::Speaker);
	mMe->setAdmin(true);
	mMe->setFocus(true);

	if (createdConference) {
		const auto &conferenceAddress = info->getUri();
		setConferenceId(ConferenceId(conferenceAddress, conferenceAddress));
		setConferenceAddress(conferenceAddress);
	}

	if (isUpdate) {
		const auto &conferenceInfo = createOrGetConferenceInfo();
		auto callLog = getMe()->getSession()->getLog();
		if (callLog) {
			callLog->setConferenceInfo(conferenceInfo);
		}
		updateConferenceInformation(op);
	}
}

std::list<std::shared_ptr<const Address>> ServerConference::getAllowedAddresses() const {
	auto allowedAddresses = getInvitedAddresses();
	if (!findInvitedParticipant(mOrganizer)) {
		allowedAddresses.push_back(mOrganizer);
	}
	return allowedAddresses;
}

bool ServerConference::isIn() const {
	return mIsIn;
}

std::shared_ptr<Call> ServerConference::getCall() const {
	return nullptr;
}

void ServerConference::notifyStateChanged(ConferenceInterface::State state) {
	if (mConfParams->audioEnabled() || mConfParams->videoEnabled()) {
		// Call callbacks before calling listeners because listeners may change state
		linphone_core_notify_conference_state_changed(getCore()->getCCore(), toC(),
		                                              (LinphoneConferenceState)getState());
	}

	Conference::notifyStateChanged(state);
}

/*
 * We go in this method in two cases:
 * - when the client who created the conference INVITEs the conference address that was specified
 *   in the redirect response consecutive to the first INVITE that was made to the factory uri.
 *   In this case, joiningPendingAfterCreation is set to true.
 * - when an already joined participant device reconnects for whatever reason.
 */
void ServerConference::confirmJoining(BCTBX_UNUSED(SalCallOp *op)) {
#ifdef HAVE_ADVANCED_IM
	shared_ptr<Participant> participant;

	shared_ptr<ServerChatRoom> serverGroupChatRoom;
	auto chatRoom = getChatRoom();
	if (chatRoom) {
		serverGroupChatRoom = dynamic_pointer_cast<ServerChatRoom>(chatRoom);
	}

	std::shared_ptr<Address> contactAddr = Address::create(op->getRemoteContact());
	if (contactAddr->getUriParamValue("gr").empty()) {
		lError() << "Conference " << *getConferenceAddress()
		         << ": Declining INVITE because the contact does not have a 'gr' uri parameter [" << *contactAddr
		         << "]";
		op->decline(SalReasonDeclined, "");
		if (serverGroupChatRoom) serverGroupChatRoom->setJoiningPendingAfterCreation(false);
		return;
	}

	std::shared_ptr<Address> gruu(contactAddr);
	shared_ptr<ParticipantDevice> device;
	shared_ptr<CallSession> deviceSession;
	auto joiningPendingAfterCreation =
	    serverGroupChatRoom ? serverGroupChatRoom->isJoiningPendingAfterCreation() : false;
	if (joiningPendingAfterCreation) {
		// Check if the participant is already there, this INVITE may come from an unknown device of an already present
		// participant
		participant = addParticipantToList(Address::create(op->getFrom()));
		participant->setAdmin(true);
		device = participant->addDevice(gruu);
		deviceSession = device->getSession();
		serverGroupChatRoom->setInitiatorDevice(device);

		/*Since the initiator of the chatroom has not yet subscribed at this stage, this won't generate NOTIFY, the
		 * events will be queued. */
		shared_ptr<ConferenceParticipantDeviceEvent> deviceEvent =
		    notifyParticipantDeviceAdded(time(nullptr), false, participant, device);
		getCore()->getPrivate()->mainDb->addEvent(deviceEvent);
		if (getCurrentParams()->isGroup()) {
			shared_ptr<ConferenceParticipantEvent> adminEvent =
			    notifyParticipantSetAdmin(time(nullptr), false, participant, true);
			getCore()->getPrivate()->mainDb->addEvent(adminEvent);
		}
	} else {
		// INVITE coming from an invited participant
		participant = serverGroupChatRoom->findCachedParticipant(Address::create(op->getFrom()));
		if (!participant) {
			lError() << "Conference " << *getConferenceAddress()
			         << ": Declining INVITE coming from someone that is not a participant";
			op->decline(SalReasonDeclined, "");
			return;
		}
		// In protocol < 1.1, one to one chatroom can be resurected by a participant, but the participant actually never
		// leaves from server's standpoint.
		if (getCurrentParams()->isGroup() && op->isContentInRemote(ContentType::ResourceLists)) {
			lError() << "Conference " << *getConferenceAddress()
			         << "Receiving ressource list body while not in creation step.";
			op->decline(SalReasonNotAcceptable);
			return;
		}
		device = participant->addDevice(gruu);
		if (!getCurrentParams()->isGroup()) {
			if (device->getState() == ParticipantDevice::State::Left) {
				lInfo() << "Conference " << *getConferenceAddress() << " " << *gruu
				        << " is reconnected to the one to one chatroom.";
				setParticipantDeviceState(device, ParticipantDevice::State::Joining);
			}
			participant->setAdmin(true);
		}
		deviceSession = device->getSession();
	}

	shared_ptr<CallSession> newDeviceSession = deviceSession;
	const auto &deviceAddress = device->getAddress();
	auto rejectSession = false;
	if (serverGroupChatRoom && (!deviceSession || (deviceSession->getPrivate()->getOp() != op))) {
		newDeviceSession = participant->createSession(*getSharedFromThis(), nullptr, true, this);
		newDeviceSession->configure(LinphoneCallIncoming, nullptr, op, participant->getAddress(),
		                            Address::create(op->getTo()));
		newDeviceSession->startIncomingNotification(false);
		const auto &contactAddress = getAccount()->getContactAddress();
		std::shared_ptr<Address> addr = getConferenceAddress()->clone()->toSharedPtr();
		if (contactAddress && contactAddress->hasUriParam("gr")) {
			addr->setUriParam("gr", contactAddress->getUriParamValue("gr"));
		}
		addr->setParam("isfocus");
		// to force is focus to be added
		newDeviceSession->getPrivate()->getOp()->setContactAddress(addr->getImpl());
		const auto &deviceState = device->getState();
		// Reject a session if there is already an active outgoing session and the participant device is trying to leave
		// the conference
		rejectSession = deviceSession && (deviceSession->getDirection() == LinphoneCallOutgoing) &&
		                ParticipantDevice::isLeavingState(deviceState);

		if (rejectSession) {
			lInfo() << "Device " << *deviceAddress << " is trying to establish a session in chatroom " << *addr
			        << ". However it is in state " << Utils::toString(deviceState)
			        << "therefore the session is likely to be immediately terminated";
		} else {
			if (deviceSession) {
				// The client changed the session possibly following a loss the network
				lInfo() << "Device " << *deviceAddress << " is replacing its session in chatroom " << *addr
				        << ", hence the old one " << deviceSession << " is immediately terminated";
				deviceSession->removeListener(this);
				deviceSession->terminate();
			}
			lInfo() << "Setting session " << newDeviceSession << " to device " << *deviceAddress
			        << " is replacing its session in chatroom " << *addr << ", hence the old one " << deviceSession
			        << " is immediately terminated";
			device->setSession(newDeviceSession);
		}
	}

	// Changes are only allowed from admin participants
	if (participant->isAdmin()) {
		if (joiningPendingAfterCreation) {
			if (!initializeParticipants(participant, op)) {
				op->decline(SalReasonNotAcceptable, "");
				requestDeletion();
			}
			/* we don't accept the session yet: initializeParticipants() has launched queries for device information
			 * that will later populate the chatroom*/
		} else if (rejectSession) {
			lInfo() << "Reject session because admin device " << *deviceAddress
			        << " has already an established session";
			op->decline(SalReasonDeclined, "");
		} else {
			/* after creation, only changes to the subject and ephemeral settings are allowed*/
			handleSubjectChange(op);
			if (serverGroupChatRoom) serverGroupChatRoom->handleEphemeralSettingsChange(newDeviceSession);
			acceptSession(newDeviceSession);
		}
	} else {
		if (rejectSession) {
			lInfo() << "Reject session because device " << *deviceAddress << " has already an established session";
			op->decline(SalReasonDeclined, "");
		} else {
			/*it is a non-admin participant that reconnected to the chatroom*/
			acceptSession(newDeviceSession);
		}
	}
#endif // HAVE_ADVANCED_IM
}

void ServerConference::confirmCreation() {
	if ((mState != ConferenceInterface::State::Instantiated) &&
	    (mState != ConferenceInterface::State::CreationPending)) {
		lError() << "Unable to confirm the creation of the conference in state " << mState;
	}

	shared_ptr<MediaSession> session = dynamic_pointer_cast<MediaSession>(getMe()->getSession());
	if (session) {
		/* Assign a random conference address to this new conference, with domain
		 * set according to the proxy config used to receive the INVITE.
		 */
		auto account = session->getPrivate()->getDestAccount();
		if (!account) {
			account = getCore()->getDefaultAccount();
		}

		if (account) {
			std::shared_ptr<Address> conferenceAddress = account->getContactAddress()->clone()->toSharedPtr();
			char confId[ServerConference::sConfIdLength];
			belle_sip_random_token(confId, sizeof(confId));
			conferenceAddress->setUriParam("conf-id", confId);
			setConferenceAddress(conferenceAddress);
			mConfParams->setAccount(account);
		}

		const_cast<CallSessionParamsPrivate *>(L_GET_PRIVATE(session->getParams()))->setInConference(true);
		const auto &actualConferenceAddress = getConferenceAddress();
		session->getPrivate()->setConferenceId(actualConferenceAddress->getUriParamValue("conf-id"));
		/* We have to call initiateIncoming() and startIncomingNotification() in order to perform the first
		 * offer/answer, and make sure that the caller has compatible SDP offer. However, ICE creates problem here
		 * because the gathering is asynchronous, and is useless anyway because the MediaSession here will anyway
		 * terminate immediately by only two possibilities:
		 * - 488 if SDP offer is not compatible
		 * - or 302 if ok.
		 * We have no need to perform ICE gathering for this session, so we set the NatPolicy to nullptr.
		 */
		session->setNatPolicy(nullptr);
		session->initiateIncoming();
		session->startIncomingNotification(false);

		const auto &conferenceInfo = createOrGetConferenceInfo();

#ifdef HAVE_ADVANCED_IM
		const auto &remoteContactAddress = session->getRemoteContactAddress();
		if (mConfParams->chatEnabled() && remoteContactAddress->hasParam("+org.linphone.specs")) {
			const auto linphoneSpecs = remoteContactAddress->getParamValue("+org.linphone.specs");
			// The creator of the chatroom must have the capability "groupchat"
			auto protocols = Utils::parseCapabilityDescriptor(linphoneSpecs.substr(1, linphoneSpecs.size() - 2));
			auto groupchat = protocols.find("groupchat");
			if (groupchat == protocols.end()) {
				lError() << "Creator " << remoteContactAddress->asStringUriOnly()
				         << " has no groupchat capability set: " << linphoneSpecs;
				setState(ConferenceInterface::State::CreationFailed);
				auto errorInfo = linphone_error_info_new();
				linphone_error_info_set(errorInfo, nullptr, LinphoneReasonNotAcceptable, 488,
				                        "\"groupchat\" capability has not been found in remote contact address",
				                        nullptr);
				session->decline(errorInfo);
				linphone_error_info_unref(errorInfo);
				return;
			}
			setState(ConferenceInterface::State::Created);
		}
#endif // HAVE_ADVANCED_IM

#ifdef HAVE_DB_STORAGE
		// Method startIncomingNotification can move the conference to the CreationFailed state if the organizer
		// doesn't have any of the codecs the server supports
		if ((mConfParams->audioEnabled() || mConfParams->videoEnabled()) &&
		    (getState() != ConferenceInterface::State::CreationFailed)) {
			// Store into DB after the start incoming notification in order to have a valid conference address being the
			// contact address of the call
			auto &mainDb = getCore()->getPrivate()->mainDb;
			if (mainDb) {
				const auto conferenceAddressStr =
				    (getConferenceAddress() ? getConferenceAddress()->toString() : std::string("sip::unknown"));
				lInfo()
				    << "Inserting conference information to database in order to be able to recreate the conference "
				    << conferenceAddressStr << " in case of restart";
				mConferenceInfoId = mainDb->insertConferenceInfo(conferenceInfo);
			}
		}
#endif // HAVE_DB_STORAGE
		auto callLog = session->getLog();
		if (callLog) {
			callLog->setConferenceInfo(conferenceInfo);
			callLog->setConferenceInfoId(mConferenceInfoId);
		}
	} else {
		lError() << "Unable to confirm the creation of the conference because no session was created";
	}
}

void ServerConference::handleSubjectChange(SalCallOp *op) {
	if (sal_custom_header_find(op->getRecvCustomHeaders(), "Subject")) {
		// Handle subject change
		lInfo() << this << ": New subject \"" << op->getSubject() << "\"";
		// Already calling Conference::setSubject
		setUtf8Subject(op->getSubject());
	}
}

bool ServerConference::initializeParticipants(const shared_ptr<Participant> &initiator, SalCallOp *op) {
	handleSubjectChange(op);
	// Handle participants addition
	const auto participantList = Utils::parseResourceLists(op->getContentInRemote(ContentType::ResourceLists));
	std::list<std::shared_ptr<const Address>> identAddresses;
	// DO not try to add participants with invalid address
	for (auto it = participantList.begin(); it != participantList.end(); ++it) {
		const auto &address = (*it)->getAddress();
		if (!(address->isValid())) {
			lError() << "ServerConference::initializeParticipants(): removing invalid address " << *address
			         << " at position " << std::distance(it, participantList.begin());
		} else {
			identAddresses.push_back(address);
		}
	}
	if (identAddresses.empty()) {
		lError() << "ServerConference::initializeParticipants(): empty list !";
		return false;
	}

	identAddresses.unique([](const auto &addr1, const auto &addr2) { return addr1->weakEqual(*addr2); });

	if (!getCurrentParams()->isGroup()) {
		if (identAddresses.size() > 1) {
			lError() << "ServerConference::initializeParticipants(): chatroom is one to one but the list "
			            "contains multiple participants !";
			return false;
		}
	}
	identAddresses.push_back(initiator->getAddress());
#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	auto serverGroupChatRoom = chatRoom ? dynamic_pointer_cast<ServerChatRoom>(chatRoom) : nullptr;
	if (mConfParams->chatEnabled() && serverGroupChatRoom) {
		if (!serverGroupChatRoom->subscribeRegistrationForParticipants(identAddresses, true)) {
			/* If we are not waiting for any registration information, then we can conclude immediately. */
			conclude();
		}
	}
#endif // HAVE_ADVANCED_IM
	return true;
}

std::shared_ptr<ConferenceInfo> ServerConference::createConferenceInfo() const {
	return createConferenceInfoWithCustomParticipantList(mOrganizer, getFullParticipantList());
}

void ServerConference::finalizeCreation() {
	const auto chatEnabled = mConfParams->chatEnabled();
	if ((getState() == ConferenceInterface::State::CreationPending) || chatEnabled) {
#ifdef HAVE_ADVANCED_IM
		const auto &chatRoom = getChatRoom();
		if (chatEnabled && chatRoom) {
			/* Application (conference server) callback to register the name.
			 * In response, the conference server will call setConferenceAddress().
			 * It has the possibility to change the conference address.
			 */
			LINPHONE_HYBRID_OBJECT_INVOKE_CBS_NO_ARG(ChatRoom, chatRoom,
			                                         linphone_chat_room_cbs_get_conference_address_generation);
		}
#endif // HAVE_ADVANCED_IM
		const std::shared_ptr<Address> &conferenceAddress = getConferenceAddress();
		setConferenceId(ConferenceId(conferenceAddress, conferenceAddress));
		std::shared_ptr<ConferenceInfo> info = nullptr;
#ifdef HAVE_DB_STORAGE
		auto &mainDb = getCore()->getPrivate()->mainDb;
		if (mainDb && (linphone_core_get_global_state(getCore()->getCCore()) == LinphoneGlobalOn)) {
			info = getCore()->getPrivate()->mainDb->getConferenceInfoFromURI(getConferenceAddress());
		}
#endif // HAVE_DB_STORAGE
		const bool createdConference = (info && info->isValidUri());
		if (createdConference && !chatEnabled) {
			lInfo() << "Conference [" << this << "] with address " << *conferenceAddress
			        << " has already been created therefore no need to carry out the redirection to its address";
		} else {
			shared_ptr<CallSession> session = mMe->getSession();
			if (session) {
				if (mConfParams->getJoiningMode() == ConferenceParams::JoiningMode::DialOut) {
					mConfParams->setStartTime(ms_time(NULL));
				}

				auto addr = *conferenceAddress;
				lInfo() << "Conference " << this << " [" << addr << "] has been created created";
				addr.setParam("isfocus");
				if (session->getState() == CallSession::State::Idle) {
					lInfo() << " Scheduling redirection to [" << addr << "] for Call session [" << session << "]";
					getCore()->doLater([session, addr] { session->redirect(addr); });
				} else {
					session->redirect(addr);
				}
#ifdef HAVE_ADVANCED_IM
				if (chatRoom) {
					auto serverGroupChatRoom = dynamic_pointer_cast<ServerChatRoom>(chatRoom);
					getCore()->getPrivate()->serverListEventHandler->addHandler(eventHandler);
					serverGroupChatRoom->setJoiningPendingAfterCreation(true);
					getCore()->getPrivate()->insertChatRoomWithDb(chatRoom);
				}
#endif // HAVE_ADVANCED_IM
			} else {
				lError() << "Session of the me participant " << *mMe->getAddress() << " of conference [" << this
				         << "] with address " << *conferenceAddress
				         << " is not known therefore it is not possible to carry out the redirection";
			}
		}
	}
}

// -----------------------------------------------------------------------------

void ServerConference::subscribeReceived(const shared_ptr<EventSubscribe> &event) {
	if (!event) return;

	const auto &participantAddress = event->getFrom();
	shared_ptr<Participant> participant = findParticipant(participantAddress);
	shared_ptr<ParticipantDevice> device = nullptr;
	if (participant) {
		const auto &contactAddr = event->getRemoteContact();
		device = participant->findDevice(contactAddr);
	}

#ifdef HAVE_ADVANCED_IM
	const auto chatEnabled = mConfParams->chatEnabled();
	const auto &chatRoom = getChatRoom();
	const auto deviceState = device ? device->getState() : ParticipantDevice::State::ScheduledForJoining;
	if (chatEnabled && device && (deviceState == ParticipantDevice::State::ScheduledForJoining)) {
		lInfo() << "Inviting device " << *device->getAddress() << " because it was scheduled to join the chat room";
		// Invite device as last time round it was attempted, the INVITE session errored out
		inviteDevice(device);
	}

	if (eventHandler) {
		if (eventHandler->subscribeReceived(event) == 0) {
			if (device && chatEnabled) {
				vector<string> acceptedContents = vector<string>();
				const auto message = (belle_sip_message_t *)event->getOp()->getRecvCustomHeaders();
				if (message) {
					for (belle_sip_header_t *acceptHeader = belle_sip_message_get_header(message, "Accept");
					     acceptHeader != NULL; acceptHeader = belle_sip_header_get_next(acceptHeader)) {
						acceptedContents.push_back(L_C_TO_STRING(belle_sip_header_get_unparsed_value(acceptHeader)));
					}
				}
				const auto protocols = Utils::parseCapabilityDescriptor(device->getCapabilityDescriptor());
				const auto ephemeral = protocols.find("ephemeral");
				if (ephemeral != protocols.end()) {
					const auto ephemeralVersion = ephemeral->second;
					device->enableAdminModeSupport((ephemeralVersion > Utils::Version(1, 1)));
				} else {
					device->enableAdminModeSupport(false);
				}
			}

			if (mConfParams->audioEnabled() || mConfParams->videoEnabled()) {
				participantDeviceJoined(participant, device);
			}
		}
	} else {
#endif // HAVE_ADVANCED_IM
		event->deny(LinphoneReasonNotAcceptable);
#ifdef HAVE_ADVANCED_IM
	}
#endif // HAVE_ADVANCED_IM

#ifdef HAVE_ADVANCED_IM
	if (chatEnabled && chatRoom) {
		// Store last notify ID in the database
		getCore()->getPrivate()->mainDb->insertChatRoom(chatRoom, getLastNotify());
	}
#endif // HAVE_ADVANCED_IM
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void ServerConference::subscriptionStateChanged(shared_ptr<EventSubscribe> event, LinphoneSubscriptionState state) {
#ifdef HAVE_ADVANCED_IM
	if (eventHandler) {
		eventHandler->subscriptionStateChanged(event, state);
	} else {
#endif // HAVE_ADVANCED_IM
		lInfo() << "Unable to handle subscription state change because conference event package (RFC 4575) is disabled "
		           "or the SDK was not compiled with ENABLE_ADVANCED_IM flag set to on";
#ifdef HAVE_ADVANCED_IM
	}
#endif // HAVE_ADVANCED_IM
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

void ServerConference::notifyFullState() {
	Conference::notifyFullState();
}

shared_ptr<ConferenceParticipantEvent> ServerConference::notifyParticipantAdded(
    time_t creationTime, const bool isFullState, const std::shared_ptr<Participant> &participant) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	incrementLastNotify();
	return Conference::notifyParticipantAdded(creationTime, isFullState, participant);
}

shared_ptr<ConferenceParticipantEvent> ServerConference::notifyParticipantRemoved(
    time_t creationTime, const bool isFullState, const std::shared_ptr<Participant> &participant) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	incrementLastNotify();
	return Conference::notifyParticipantRemoved(creationTime, isFullState, participant);
}

shared_ptr<ConferenceParticipantEvent> ServerConference::notifyParticipantSetAdmin(
    time_t creationTime, const bool isFullState, const std::shared_ptr<Participant> &participant, bool isAdmin) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	incrementLastNotify();
	return Conference::notifyParticipantSetAdmin(creationTime, isFullState, participant, isAdmin);
}

shared_ptr<ConferenceSubjectEvent>
ServerConference::notifySubjectChanged(time_t creationTime, const bool isFullState, const std::string subject) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	incrementLastNotify();
	return Conference::notifySubjectChanged(creationTime, isFullState, subject);
}

shared_ptr<ConferenceEphemeralMessageEvent>
ServerConference::notifyEphemeralModeChanged(time_t creationTime, const bool isFullState, const EventLog::Type type) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	incrementLastNotify();
	return Conference::notifyEphemeralModeChanged(creationTime, isFullState, type);
}

shared_ptr<ConferenceEphemeralMessageEvent>
ServerConference::notifyEphemeralMessageEnabled(time_t creationTime, const bool isFullState, const bool enable) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	incrementLastNotify();
	return Conference::notifyEphemeralMessageEnabled(creationTime, isFullState, enable);
}

shared_ptr<ConferenceEphemeralMessageEvent>
ServerConference::notifyEphemeralLifetimeChanged(time_t creationTime, const bool isFullState, const long lifetime) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	incrementLastNotify();
	return Conference::notifyEphemeralLifetimeChanged(creationTime, isFullState, lifetime);
}

shared_ptr<ConferenceParticipantDeviceEvent> ServerConference::notifyParticipantDeviceScreenSharingChanged(
    time_t creationTime,
    const bool isFullState,
    const std::shared_ptr<Participant> &participant,
    const std::shared_ptr<ParticipantDevice> &participantDevice) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++mLastNotify;
	return Conference::notifyParticipantDeviceScreenSharingChanged(creationTime, isFullState, participant,
	                                                               participantDevice);
}

shared_ptr<ConferenceParticipantDeviceEvent>
ServerConference::notifyParticipantDeviceAdded(time_t creationTime,
                                               const bool isFullState,
                                               const std::shared_ptr<Participant> &participant,
                                               const std::shared_ptr<ParticipantDevice> &participantDevice) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	incrementLastNotify();
	return Conference::notifyParticipantDeviceAdded(creationTime, isFullState, participant, participantDevice);
}

shared_ptr<ConferenceParticipantDeviceEvent>
ServerConference::notifyParticipantDeviceRemoved(time_t creationTime,
                                                 const bool isFullState,
                                                 const std::shared_ptr<Participant> &participant,
                                                 const std::shared_ptr<ParticipantDevice> &participantDevice) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	incrementLastNotify();
	return Conference::notifyParticipantDeviceRemoved(creationTime, isFullState, participant, participantDevice);
}

shared_ptr<ConferenceParticipantDeviceEvent>
ServerConference::notifyParticipantDeviceStateChanged(time_t creationTime,
                                                      const bool isFullState,
                                                      const std::shared_ptr<Participant> &participant,
                                                      const std::shared_ptr<ParticipantDevice> &participantDevice) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	incrementLastNotify();
	// This method is called by participant devices whenever they change state
	auto event =
	    Conference::notifyParticipantDeviceStateChanged(creationTime, isFullState, participant, participantDevice);
	if (mConfParams->chatEnabled()) {
		getCore()->getPrivate()->mainDb->addEvent(event);
	}
	return event;
}

shared_ptr<ConferenceParticipantDeviceEvent> ServerConference::notifyParticipantDeviceMediaCapabilityChanged(
    time_t creationTime,
    const bool isFullState,
    const std::shared_ptr<Participant> &participant,
    const std::shared_ptr<ParticipantDevice> &participantDevice) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	incrementLastNotify();
	auto event = Conference::notifyParticipantDeviceMediaCapabilityChanged(creationTime, isFullState, participant,
	                                                                       participantDevice);
	if (mConfParams->chatEnabled()) {
		getCore()->getPrivate()->mainDb->addEvent(event);
	}
	return event;
}

shared_ptr<ConferenceAvailableMediaEvent> ServerConference::notifyAvailableMediaChanged(
    time_t creationTime, const bool isFullState, const std::map<ConferenceMediaCapabilities, bool> mediaCapabilities) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	incrementLastNotify();
	return Conference::notifyAvailableMediaChanged(creationTime, isFullState, mediaCapabilities);
}

int ServerConference::inviteAddresses(const list<std::shared_ptr<const Address>> &addresses,
                                      const LinphoneCallParams *params) {

	const auto &coreCurrentCall = getCore()->getCurrentCall();
	const bool startingConference = (getState() == ConferenceInterface::State::CreationPending);

	const auto &outputDevice = (coreCurrentCall) ? coreCurrentCall->getOutputAudioDevice() : nullptr;
	const auto &inputDevice = (coreCurrentCall) ? coreCurrentCall->getInputAudioDevice() : nullptr;

	auto lc = getCore()->getCCore();

	for (const auto &address : addresses) {
		std::shared_ptr<Call> call = nullptr;
		const auto &device = findParticipantDevice(address, address);

		/*
		 * In the case of a conference server, it is enough to look if there is already a participant with the same
		 * address as the one searched. If this is the case, then pick the first device (if there is one) and search the
		 * call on the list held by the core. A non-conference server may be wishing to add an already running call to a
		 * conference, therefore the search is done through the remote address. A use case is the following:
		 * - A has establishehd individual calls towards B and C and wants to add them to a conference hosted on its
		 * device
		 * - A can call inviteAddresses({B,C}, params) and we should not start any new call
		 * Note that this scenario is not possible for a conference server as it is a passive component.
		 */
		if (linphone_core_conference_server_enabled(lc)) {
			if (device) {
				const auto &session = device->getSession();
				const auto sessionState = session ? session->getState() : CallSession::State::Idle;
				const auto &callId = device->getCallId();
				if (device->getState() == ParticipantDevice::State::Joining &&
				    (sessionState == CallSession::State::OutgoingProgress ||
				     sessionState == CallSession::State::Connected)) {
					lInfo() << "Conference " << *getConferenceAddress() << ": outgoing INVITE already in progress.";
					return -1;
				}
				if (sessionState == CallSession::State::IncomingReceived) {
					lInfo() << "Conference " << *getConferenceAddress() << ": incoming INVITE in progress.";
					return -1;
				}
				setParticipantDeviceState(device, ParticipantDevice::State::Joining);
				if (!callId.empty()) {
					call = getCore()->getCallByCallId(callId);
				} else if (session) {
					const auto &calls = getCore()->getCalls();
					auto it = std::find_if(calls.cbegin(), calls.cend(),
					                       [&session](const auto &c) { return (c->getActiveSession() == session); });
					if (it != calls.cend()) {
						call = (*it);
					}
				}
			}
		} else {
			call = getCore()->getCallByRemoteAddress(address);
		}

		const auto audioEnabled = mConfParams->audioEnabled();
		const auto videoEnabled = mConfParams->videoEnabled();
		if (!call) {
			const auto chatEnabled = mConfParams->chatEnabled();
			LinphoneCallParams *new_params;
			if (params) {
				new_params = _linphone_call_params_copy(params);
			} else {
				new_params = linphone_core_create_call_params(lc, nullptr);
				linphone_call_params_enable_audio(new_params, audioEnabled);
				linphone_call_params_enable_video(new_params, videoEnabled);
			}
			linphone_call_params_enable_tone_indications(new_params, !chatEnabled);
			linphone_call_params_set_in_conference(new_params, TRUE);
			linphone_call_params_set_start_time(new_params, mConfParams->getStartTime());
			linphone_call_params_set_account(new_params, getAccount()->toC());

			const std::shared_ptr<Address> &conferenceAddress = getConferenceAddress();
			const string &confId = conferenceAddress->getUriParamValue("conf-id");
			linphone_call_params_set_conference_id(new_params, confId.c_str());

			std::shared_ptr<CallSession> session = nullptr;

			if (audioEnabled || videoEnabled) {
				if (chatEnabled) {
					L_GET_CPP_PTR_FROM_C_OBJECT(new_params)->addCustomContactParameter("text", std::string());
				}
				call =
				    Call::toCpp(linphone_core_invite_address_with_params_2(
				                    lc, address->toC(), new_params, L_STRING_TO_C(mConfParams->getUtf8Subject()), NULL))
				        ->getSharedFromThis();
				session = call->getActiveSession();
				session->addListener(this);

				tryAddMeDevice();

				if (!call) {
					lError() << "ServerConference::inviteAddresses(): could not invite participant";
				} else {
					addParticipant(call);
					auto participant = findParticipant(address);
					participant->setPreserveSession(false);
				}
			} else {
				session = makeSession(device, L_GET_CPP_PTR_FROM_C_OBJECT(new_params));
				session->startInvite(nullptr, getUtf8Subject(), nullptr);
			}
			if (device) {
				device->setSession(session);
			}
			linphone_call_params_unref(new_params);
		} else if (audioEnabled || videoEnabled) {
			/* There is already a call to this address, so simply join it to the local conference if not already
			 * done */
			if (!call->getCurrentParams()->getPrivate()->getInConference()) {
				addParticipant(call);
				auto participant = findParticipant(address);
				participant->setPreserveSession(true);
			}
		}
		/* If the local participant is not yet created, created it and it to the conference */
		addLocalEndpoint();
		if (call) {
			call->setConference(getSharedFromThis());
		}
	}

	// If current call is not NULL and the conference is in the creating pending state or instantied, then try to change
	// audio route to keep the one currently used
	if (startingConference) {
		if (outputDevice) {
			setOutputAudioDevice(outputDevice);
		}
		if (inputDevice) {
			setInputAudioDevice(inputDevice);
		}
	}

	return 0;
}

bool ServerConference::dialOutAddresses(const std::list<std::shared_ptr<const Address>> &addressList) {
	auto new_params = linphone_core_create_call_params(getCore()->getCCore(), nullptr);
	linphone_call_params_enable_audio(new_params, mConfParams->audioEnabled());
	linphone_call_params_enable_video(new_params, mConfParams->videoEnabled());

	linphone_call_params_set_in_conference(new_params, TRUE);

	const std::shared_ptr<Address> &conferenceAddress = getConferenceAddress();
	const string &confId = conferenceAddress->getUriParamValue("conf-id");
	linphone_call_params_set_conference_id(new_params, confId.c_str());

	if (mConfParams->chatEnabled()) {
		if (!getCurrentParams()->isGroup())
			L_GET_CPP_PTR_FROM_C_OBJECT(new_params)->addCustomHeader("One-To-One-Chat-Room", "true");
		if (getCurrentParams()->getChatParams()->isEncrypted())
			L_GET_CPP_PTR_FROM_C_OBJECT(new_params)->addCustomHeader("End-To-End-Encrypted", "true");
		if (getCurrentParams()->getChatParams()->ephemeralAllowed()) {
			L_GET_CPP_PTR_FROM_C_OBJECT(new_params)->addCustomHeader("Ephemerable", "true");
			L_GET_CPP_PTR_FROM_C_OBJECT(new_params)
			    ->addCustomHeader("Ephemeral-Life-Time",
			                      to_string(getCurrentParams()->getChatParams()->getEphemeralLifetime()));
		}
	}

	std::list<Address> addresses;
	for (const auto &p : getInvitedAddresses()) {
		addresses.push_back(*p);
	}
	// Add participants already in the conference to the list of addresses if they are not part of the invitees
	for (const auto &p : getParticipants()) {
		const auto &pAddress = p->getAddress();
		auto pIt = std::find_if(addresses.begin(), addresses.end(),
		                        [&pAddress](const auto &address) { return (pAddress->weakEqual(address)); });
		if (pIt == addresses.end()) {
			addresses.push_back(*pAddress);
		}
	}

	auto resourceList = Content::create();
	resourceList->setBodyFromUtf8(Utils::getResourceLists(addresses));
	resourceList->setContentType(ContentType::ResourceLists);
	if (mConfParams->audioEnabled() || mConfParams->videoEnabled()) {
		resourceList->setContentDisposition(ContentDisposition::RecipientList);
	} else {
		resourceList->setContentDisposition(ContentDisposition::RecipientListHistory);
	}
	if (linphone_core_content_encoding_supported(getCore()->getCCore(), "deflate")) {
		resourceList->setContentEncoding("deflate");
	}
	if (!resourceList->isEmpty()) {
		L_GET_CPP_PTR_FROM_C_OBJECT(new_params)->addCustomContent(resourceList);
	}

	if (mOrganizer && (mConfParams->audioEnabled() || mConfParams->videoEnabled())) {
		auto sipfrag = Content::create();
		const auto organizerUri = mOrganizer->getUri();
		sipfrag->setBodyFromLocale("From: <" + organizerUri.toString() + ">");
		sipfrag->setContentType(ContentType::SipFrag);
		L_GET_CPP_PTR_FROM_C_OBJECT(new_params)->addCustomContent(sipfrag);
	}
	auto success = (inviteAddresses(addressList, new_params) == 0);
	linphone_call_params_unref(new_params);
	return success;
}

// returns true if a new session has been created, false if there is already an existing and valid one.
shared_ptr<CallSession> ServerConference::makeSession(const std::shared_ptr<ParticipantDevice> &device,
                                                      BCTBX_UNUSED(const MediaSessionParams *csp)) {
	shared_ptr<CallSession> session = device->getSession();
#ifdef HAVE_ADVANCED_IM

	if (session) {
		switch (session->getState()) {
			case CallSession::State::End:
			case CallSession::State::Error:
			case CallSession::State::Released:
				session = nullptr; // our session is dead, we'll make a new one.
				break;
			default:
				break;
		}
	}
	if (!session) {
		shared_ptr<Participant> participant =
		    const_pointer_cast<Participant>(device->getParticipant()->getSharedFromThis());
		MediaSessionParams *currentParams = csp->clone();
		if (mConfParams->chatEnabled()) {
			currentParams->addCustomContactParameter("text", std::string());
		}
		currentParams->getPrivate()->enableToneIndications(mConfParams->audioEnabled() || mConfParams->videoEnabled());
		currentParams->getPrivate()->setInConference(TRUE);
		const std::shared_ptr<Address> &conferenceAddress = getConferenceAddress();
		const string &confId = conferenceAddress->getUriParamValue("conf-id");
		if (!confId.empty()) {
			currentParams->getPrivate()->setConferenceId(confId);
		}
		session = participant->createSession(*this, currentParams, true, this);
		delete currentParams;
		session->configure(LinphoneCallOutgoing, nullptr, nullptr, conferenceAddress, device->getAddress());
		device->setSession(session);
		session->initiateOutgoing();
		session->getPrivate()->createOp();
	}
#endif // HAVE_ADVANCED_IM
	return session;
}

void ServerConference::inviteDevice(const shared_ptr<ParticipantDevice> &device) {
	lInfo() << "Conference " << *getConferenceAddress() << ": Inviting device '" << *device->getAddress() << "'";
	dialOutAddresses({device->getAddress()});
}

void ServerConference::byeDevice(const std::shared_ptr<ParticipantDevice> &device) {
	const std::shared_ptr<Address> &conferenceAddress = getConferenceAddress();
	lInfo() << "Conference " << conferenceAddress << ": Asking device '" << *device->getAddress() << "' to leave";
	setParticipantDeviceState(device, ParticipantDevice::State::Leaving);
	MediaSessionParams csp;
	csp.enableAudio(mConfParams->audioEnabled());
	csp.enableVideo(mConfParams->videoEnabled());
	if (mConfParams->chatEnabled()) {
		if (!getCurrentParams()->isGroup()) csp.addCustomHeader("One-To-One-Chat-Room", "true");
		if (getCurrentParams()->getChatParams()->isEncrypted()) csp.addCustomHeader("End-To-End-Encrypted", "true");
		if (getCurrentParams()->getChatParams()->ephemeralAllowed()) {
			csp.addCustomHeader("Ephemerable", "true");
			csp.addCustomHeader("Ephemeral-Life-Time",
			                    to_string(getCurrentParams()->getChatParams()->getEphemeralLifetime()));
		}
	}
	csp.getPrivate()->enableToneIndications(mConfParams->audioEnabled() || mConfParams->videoEnabled());
	csp.getPrivate()->setInConference(TRUE);
	const string &confId = conferenceAddress->getUriParamValue("conf-id");
	if (!confId.empty()) {
		csp.getPrivate()->setConferenceId(confId);
	}
	shared_ptr<CallSession> session = makeSession(device, &csp);
	switch (session->getState()) {
		case CallSession::State::OutgoingInit:
			session->startInvite(nullptr, getUtf8Subject(), nullptr);
			break;
		case CallSession::State::Connected:
		case CallSession::State::StreamsRunning:
			session->terminate();
			break;
		default:
			break;
	}
}

bool ServerConference::finalizeParticipantAddition(std::shared_ptr<Call> call) {
	const auto &newParticipantSession = call->getMediaSession();
	const auto &device = findParticipantDevice(newParticipantSession);
	if (device) {
		const auto deviceState = device->getState();
		if (deviceState == ParticipantDevice::State::Joining) {
			const std::shared_ptr<Address> &remoteAddress = call->getRemoteAddress();
			const auto &p = findParticipant(remoteAddress);
			if (device && p) {
				participantDeviceJoined(p, device);
			}
		} else if (deviceState == ParticipantDevice::State::ScheduledForJoining) {
			device->setState(ParticipantDevice::State::Joining);
			auto contactAddress = newParticipantSession->getContactAddress();

			if (contactAddress && contactAddress->isValid() && !contactAddress->hasParam("isfocus")) {
				getCore()->doLater([this, call] {
					const std::shared_ptr<Address> &conferenceAddress = getConferenceAddress();
					const string &confId = conferenceAddress->getUriParamValue("conf-id");

					LinphoneCallParams *params = linphone_core_create_call_params(getCore()->getCCore(), call->toC());
					linphone_call_params_set_in_conference(params, TRUE);
					linphone_call_params_set_conference_id(params, confId.c_str());
					linphone_call_params_set_start_time(params, mConfParams->getStartTime());
					linphone_call_params_set_end_time(params, mConfParams->getEndTime());
					if (getCurrentParams()->videoEnabled()) {
						linphone_call_params_enable_video(
						    params, linphone_call_params_video_enabled(linphone_call_get_remote_params(call->toC())));
					} else {
						linphone_call_params_enable_video(params, FALSE);
					}

					linphone_call_update(call->toC(), params);
					linphone_call_params_unref(params);
				});
			}
		}
	}

	return true;
}

void ServerConference::addLocalEndpoint() {
	if (mConfParams->localParticipantEnabled()) {
		StreamMixer *mixer = mMixerSession->getMixerByType(SalAudio);
		if (mixer) {
			mixer->enableLocalParticipant(true);
			// Get ssrc of me because it must be sent to participants through NOTIFY
			auto audioMixer = dynamic_cast<MS2AudioMixer *>(mixer);
			auto audioStream = audioMixer->getAudioStream();
			auto meSsrc = audio_stream_get_send_ssrc(audioStream);
			for (auto &device : mMe->getDevices()) {
				device->setSsrc(LinphoneStreamTypeAudio, meSsrc);
			}
		}

		if (mConfParams->videoEnabled()) {
			mixer = mMixerSession->getMixerByType(SalVideo);
			if (mixer) {
				mixer->enableLocalParticipant(true);
#ifdef VIDEO_ENABLED
				auto videoMixer = dynamic_cast<MS2VideoMixer *>(mixer);
				auto videoStream = videoMixer->getVideoStream();
				auto meSsrc = media_stream_get_send_ssrc(&videoStream->ms);
				for (auto &device : mMe->getDevices()) {
					device->setSsrc(LinphoneStreamTypeVideo, meSsrc);
					videoMixer->setLocalParticipantLabel(device->getLabel(LinphoneStreamTypeVideo));
				}
#endif // VIDEO_ENABLED
				VideoControlInterface *vci = getVideoControlInterface();
				if (vci) {
					vci->setNativePreviewWindowId(getCore()->getCCore()->preview_window_id);
					vci->setNativeWindowId(getCore()->getCCore()->video_window_id);
				}
			}
		}

		if (!isIn()) {
			mIsIn = true;
			time_t creationTime = time(nullptr);
			notifyParticipantAdded(creationTime, false, getMe());
			for (auto &device : mMe->getDevices()) {
				notifyParticipantDeviceAdded(creationTime, false, getMe(), device);
			}
		}
	}
}

void ServerConference::removeLocalEndpoint() {
	mMixerSession->enableLocalParticipant(false);

	if (isIn()) {
		mIsIn = false;

		time_t creationTime = time(nullptr);
		for (auto &device : mMe->getDevices()) {
			notifyParticipantDeviceRemoved(creationTime, false, getMe(), device);
		}
		notifyParticipantRemoved(creationTime, false, getMe());
	}
}

bool ServerConference::tryAddMeDevice() {
	const auto &account = getAccount();
	if (mConfParams->localParticipantEnabled() && mMe->getDevices().empty() && account) {
		const auto &contactAddress = account->getContactAddress();
		if (contactAddress) {
			std::shared_ptr<Address> devAddr = contactAddress->clone()->toSharedPtr();
			auto meDev = mMe->addDevice(devAddr);
			const auto &meSession = mMe->getSession();

			char label[Conference::sLabelLength];
			belle_sip_random_token(label, sizeof(label));
			meDev->setLabel(label, LinphoneStreamTypeAudio);
			belle_sip_random_token(label, sizeof(label));
			meDev->setLabel(label, LinphoneStreamTypeVideo);
			meDev->setThumbnailStreamLabel(label);
			meDev->setSession(meSession);
			meDev->setJoiningMethod(ParticipantDevice::JoiningMethod::FocusOwner);
			meDev->setState(ParticipantDevice::State::Present);

			// Initialize media directions
			meDev->setStreamCapability(
			    (mConfParams->audioEnabled() ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive),
			    LinphoneStreamTypeAudio);
			meDev->setStreamCapability(
			    (mConfParams->videoEnabled() ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive),
			    LinphoneStreamTypeVideo);
			meDev->setStreamCapability(
			    (mConfParams->chatEnabled() ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive),
			    LinphoneStreamTypeText);

			meDev->updateStreamAvailabilities();

			return true;
		}
	}
	return false;
}

/* This function is used to re-join devices of a participant that has left previously. Its device are still referenced
 * until they 're all left. */
void ServerConference::resumeParticipant(BCTBX_UNUSED(const std::shared_ptr<Participant> &participant)) {
#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		addParticipantToList(participant->getAddress());
		for (auto device : participant->getDevices()) {
			switch (device->getState()) {
				case ParticipantDevice::State::Leaving:
				case ParticipantDevice::State::Left:
				case ParticipantDevice::State::ScheduledForLeaving:
					setParticipantDeviceState(device, ParticipantDevice::State::ScheduledForJoining);
					updateParticipantDeviceSession(device);
					break;
				default:
					break;
			}
		}
	}
#endif // HAVE_ADVANCED_IM
}

int ServerConference::getParticipantCount() const {
#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		auto serverGroupChatRoom = dynamic_pointer_cast<ServerChatRoom>(chatRoom);
		const auto &cachedParticipants = serverGroupChatRoom->getCachedParticipants();
		return (int)cachedParticipants.size();
	}
#endif // HAVE_ADVANCED_IM
	return (int)mParticipants.size();
}

shared_ptr<Participant> ServerConference::addParticipantToList(BCTBX_UNUSED(const std::shared_ptr<Address> &addr)) {
	shared_ptr<Participant> participant = nullptr;
#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		auto serverGroupChatRoom = dynamic_pointer_cast<ServerChatRoom>(chatRoom);
		participant = serverGroupChatRoom->findCachedParticipant(addr);
		if (!participant) {
			participant = Participant::create(getSharedFromThis(), addr);
			serverGroupChatRoom->addCachedParticipant(participant);
		}
		/* Case of participant that is still referenced in the chatroom, but no longer authorized because it has been
		 * removed previously OR a totally new participant. */
		if (findParticipant(addr) == nullptr) {
			mParticipants.push_back(participant);
			shared_ptr<ConferenceParticipantEvent> event = notifyParticipantAdded(time(nullptr), false, participant);
			getCore()->getPrivate()->mainDb->addEvent(event);
		}
	}
#endif // HAVE_ADVANCED_IM
	return participant;
}

bool ServerConference::addParticipants(const std::list<std::shared_ptr<Call>> &calls) {
	const auto &coreCurrentCall = getCore()->getCurrentCall();
	const bool startingConference = (getState() == ConferenceInterface::State::CreationPending);
	const auto &outputDevice = (coreCurrentCall) ? coreCurrentCall->getOutputAudioDevice() : nullptr;
	const auto &inputDevice = (coreCurrentCall) ? coreCurrentCall->getInputAudioDevice() : nullptr;

	bool success = Conference::addParticipants(calls);
	// If current call is not NULL and the conference is in the creating pending state or instantied, then try to change
	// audio route to keep the one currently used Do not change audio route if participant addition is not successful
	if (success && startingConference) {
		if (outputDevice) {
			setOutputAudioDevice(outputDevice);
		}
		if (inputDevice) {
			setInputAudioDevice(inputDevice);
		}
	}

	return success;
}

bool ServerConference::addParticipants(const std::list<std::shared_ptr<const Address>> &addresses) {
	return Conference::addParticipants(addresses);
}

bool ServerConference::addParticipant(std::shared_ptr<Call> call) {

	const std::shared_ptr<Address> &conferenceAddress = getConferenceAddress();
	const auto &remoteAddress = call->getRemoteAddress();
	if (linphone_call_params_get_in_conference(linphone_call_get_current_params(call->toC()))) {
		lError() << "Call (local address " << *call->getLocalAddress() << " remote address "
		         << (remoteAddress ? remoteAddress->toString() : "Unknown") << ") is already in conference "
		         << *conferenceAddress;
		return false;
	}

	const auto &localAddress = call->getLocalAddress();
	if (mConfParams->getParticipantListType() == ConferenceParams::ParticipantListType::Closed) {
		const auto allowedAddresses = getAllowedAddresses();
		auto p = std::find_if(allowedAddresses.begin(), allowedAddresses.end(),
		                      [&remoteAddress](const auto &address) { return (remoteAddress->weakEqual(*address)); });
		if (p == allowedAddresses.end()) {
			lError() << "Unable to add call (local address " << *localAddress << " remote address " << *remoteAddress
			         << ") because participant " << *remoteAddress
			         << " is not in the list of allowed participants of conference " << *conferenceAddress;

			LinphoneErrorInfo *ei = linphone_error_info_new();
			linphone_error_info_set(ei, NULL, LinphoneReasonUnknown, 403, "Call forbidden to join the conference",
			                        NULL);
			call->terminate(ei);
			linphone_error_info_unref(ei);
			return false;
		}
	}

	const auto initialState = getState();
	const auto dialout = (mConfParams->getJoiningMode() == ConferenceParams::JoiningMode::DialOut);
	// If conference must start immediately, then the organizer will call the conference server and the other
	// participants will be dialed out
	if ((initialState == ConferenceInterface::State::CreationPending) && dialout &&
	    !remoteAddress->weakEqual(*mOrganizer)) {
		lError() << "The conference must immediately start (start time: " << mConfParams->getStartTime()
		         << " end time: " << mConfParams->getEndTime() << "). Unable to add participant " << *remoteAddress
		         << " because participants will be dialed out by the conference server as soon as " << *mOrganizer
		         << " dials in";
		return false;
	}

	// Add participants event after the conference ends if it is active (i.e in the Created state)
	if (initialState != ConferenceInterface::State::Created) {
		if (!isConferenceStarted()) {
			lError() << "Unable to add call (local address " << *localAddress << " remote address " << *remoteAddress
			         << ") because conference " << *conferenceAddress << " has not started yet";
			LinphoneErrorInfo *ei = linphone_error_info_new();
			linphone_error_info_set(ei, NULL, LinphoneReasonUnknown, 403, "Conference not started yet", NULL);
			call->terminate(ei);
			linphone_error_info_unref(ei);
			return false;
		}

		if (isConferenceEnded()) {
			lError() << "Unable to add call (local address " << *localAddress << " remote address " << *remoteAddress
			         << ") because conference " << *conferenceAddress << " is already terminated";
			LinphoneErrorInfo *ei = linphone_error_info_new();
			linphone_error_info_set(ei, NULL, LinphoneReasonUnknown, 403, "Conference already terminated", NULL);
			call->terminate(ei);
			linphone_error_info_unref(ei);
			return false;
		}
	}

	const string &confId = conferenceAddress->getUriParamValue("conf-id");
	const string &callConfId = call->getConferenceId();

	const auto &coreCurrentCall = getCore()->getCurrentCall();
	const bool startingConference = (getState() == ConferenceInterface::State::CreationPending);

	const auto &outputDevice = (coreCurrentCall) ? coreCurrentCall->getOutputAudioDevice() : nullptr;
	const auto &inputDevice = (coreCurrentCall) ? coreCurrentCall->getInputAudioDevice() : nullptr;

	// Add participant only if creation is successful or call was previously part of the conference
	bool canAddParticipant =
	    ((callConfId.compare(confId) == 0) || (getState() == ConferenceInterface::State::CreationPending) ||
	     (getState() == ConferenceInterface::State::Created));

	if (canAddParticipant) {
		auto session = call->getMediaSession();
		const auto &remoteContactAddress = session->getRemoteContactAddress();
		LinphoneCallState state = static_cast<LinphoneCallState>(call->getState());

		auto participantDevice = (remoteContactAddress && remoteContactAddress->isValid())
		                             ? findParticipantDevice(session->getRemoteAddress(), remoteContactAddress)
		                             : nullptr;
		if (participantDevice) {
			auto deviceSession = participantDevice->getSession();
			if (deviceSession) {
				if (session == deviceSession) {
					lWarning() << "Try to add again a participant device with session " << session;
					return false;
				} else {
					lInfo() << "Already found a participant device with address " << *remoteContactAddress
					        << ". Recreating it";
					deviceSession->terminate();
				}
			}
		}

		if (!getAccount()) {
			// Set proxy configuration used for the conference
			auto callAccount = call->getDestAccount();
			if (callAccount) {
				mConfParams->setAccount(callAccount);
			} else {
				auto account = getCore()->lookupKnownAccount(call->getToAddress(), true);
				mConfParams->setAccount(account);
			}
		}

		// Get contact address here because it may be modified by a change in the local parameters. As the participant
		// enters the conference, in fact attributes conf-id and isfocus are added later on (based on local parameters)
		// therefore there is no way to know if the remote client already knew that the call was in a conference or not.
		auto contactAddress = session->getContactAddress();
		tryAddMeDevice();

		if (!mMixerSession) {
			mMixerSession.reset(new MixerSession(*getCore().get()));
			mMixerSession->setSecurityLevel(mConfParams->getSecurityLevel());
		}

		// Add participant to the conference participant list
		switch (state) {
			case LinphoneCallOutgoingInit:
			case LinphoneCallOutgoingProgress:
			case LinphoneCallOutgoingRinging:
			case LinphoneCallIncomingReceived:
			case LinphoneCallPausing:
			case LinphoneCallPaused:
			case LinphoneCallResuming:
			case LinphoneCallStreamsRunning: {
				if (call->toC() == linphone_core_get_current_call(getCore()->getCCore()))
					L_GET_PRIVATE_FROM_C_OBJECT(getCore()->getCCore())->setCurrentCall(nullptr);
				mMixerSession->joinStreamsGroup(session->getStreamsGroup());
				/*
				 * Modifying the MediaSession's params directly is a bit hacky.
				 */
				const_cast<MediaSessionParamsPrivate *>(L_GET_PRIVATE(call->getParams()))->setInConference(true);
				const_cast<MediaSessionParamsPrivate *>(L_GET_PRIVATE(call->getParams()))->setConferenceId(confId);
				const_cast<MediaSessionParamsPrivate *>(L_GET_PRIVATE(call->getParams()))
				    ->setStartTime(mConfParams->getStartTime());
				const_cast<MediaSessionParamsPrivate *>(L_GET_PRIVATE(call->getParams()))
				    ->setEndTime(mConfParams->getEndTime());
				if (getCurrentParams()->videoEnabled()) {
					if (getCurrentParams()->localParticipantEnabled()) {
						const_cast<MediaSessionParams *>(call->getParams())->enableVideo(true);
					} else {
						if (call->getRemoteParams()) {
							const_cast<MediaSessionParams *>(call->getParams())
							    ->enableVideo(call->getRemoteParams()->videoEnabled());
						}
					}
				} else {
					const_cast<MediaSessionParams *>(call->getParams())->enableVideo(false);
				}

				Conference::addParticipant(call);

				const auto &participant = findParticipant(session->getRemoteAddress());
				LinphoneMediaDirection audioDirection = LinphoneMediaDirectionInactive;
				LinphoneMediaDirection videoDirection = LinphoneMediaDirectionInactive;
				if (participant) {
					const auto &role = participant->getRole();
					switch (role) {
						case Participant::Role::Speaker:
							audioDirection = LinphoneMediaDirectionSendRecv;
							videoDirection = LinphoneMediaDirectionSendRecv;
							break;
						case Participant::Role::Listener:
							audioDirection = LinphoneMediaDirectionSendOnly;
							videoDirection = LinphoneMediaDirectionSendOnly;
							break;
						case Participant::Role::Unknown:
							audioDirection = LinphoneMediaDirectionInactive;
							videoDirection = LinphoneMediaDirectionInactive;
							break;
					}
				}

				const_cast<MediaSessionParams *>(call->getParams())->setAudioDirection(audioDirection);
				const_cast<MediaSessionParams *>(call->getParams())->setVideoDirection(videoDirection);
			}

			break;
			default:
				lError() << "Call " << call << " (local address " << *call->getLocalAddress() << " remote address "
				         << (remoteAddress ? remoteAddress->toString() : "Unknown") << ") is in state "
				         << Utils::toString(call->getState())
				         << ", hence it cannot be added to the conference right now";
				return false;
				break;
		}

		// Update call
		auto device = findParticipantDevice(session);
		switch (state) {
			case LinphoneCallPausing:
				// Call cannot be resumed immediately, hence delay it until next state change
				session->delayResume();
				break;
			case LinphoneCallOutgoingInit:
			case LinphoneCallOutgoingProgress:
			case LinphoneCallOutgoingRinging:
			case LinphoneCallIncomingReceived:
				break;
			case LinphoneCallPaused:
				// Conference resumes call that previously paused in order to add the participant
				getCore()->doLater([call] { call->resume(); });
				break;
			case LinphoneCallStreamsRunning:
				// Calling enter here because update will lock sound resources
				enter();
				BCTBX_NO_BREAK; /* Intentional no break */
			case LinphoneCallResuming: {
				if (contactAddress && contactAddress->isValid() && !contactAddress->hasParam("isfocus")) {
					lInfo() << "Call " << call << " (local address " << *call->getLocalAddress() << " remote address "
					        << (remoteAddress ? remoteAddress->toString() : "Unknown") << " because contact address "
					        << (contactAddress ? contactAddress->toString() : "Unknown")
					        << " has not 'isfocus' parameter";

					getCore()->doLater([call, session] {
						const MediaSessionParams *params = session->getMediaParams();
						MediaSessionParams *currentParams = params->clone();
						call->update(currentParams);
						delete currentParams;
					});
				}
			} break;
			default:
				lError() << "Call " << call << " (local address " << *call->getLocalAddress() << " remote address "
				         << (remoteAddress ? remoteAddress->toString() : "Unknown") << ") is in state "
				         << Utils::toString(call->getState())
				         << ", hence the call cannot be updated following it becoming part of the conference";
				return false;
				break;
		}

		// If current call is not NULL and the conference is in the creating pending state or instantied, then try to
		// change audio route to keep the one currently used
		if (startingConference) {
			if (outputDevice) {
				setOutputAudioDevice(outputDevice);
			}
			if (inputDevice) {
				setInputAudioDevice(inputDevice);
			}
		}
		setState(ConferenceInterface::State::Created);

		auto op = session->getPrivate()->getOp();
		auto resourceList = op ? op->getContentInRemote(ContentType::ResourceLists) : nullopt;
		bool isEmpty = resourceList ? resourceList.value().get().isEmpty() : true;

		// If no resource list is provided in the INVITE, there is not need to call participants
		if ((initialState == ConferenceInterface::State::CreationPending) && dialout && !isEmpty) {
			list<std::shared_ptr<const Address>> addresses;
			for (auto &participant : mInvitedParticipants) {
				const auto &addr = participant->getAddress();
				// Do not invite organizer as it is already dialing in
				if (*addr != *mOrganizer) {
					addresses.push_back(addr);
				}
			}
			dialOutAddresses(addresses);
		}

		return true;
	}

	lError() << "Unable to add call (local address " << *call->getLocalAddress() << " remote address "
	         << (remoteAddress ? remoteAddress->toString() : "Unknown") << ") to conference "
	         << *getConferenceAddress();
	return false;
}

bool ServerConference::addParticipant(const std::shared_ptr<const Address> &participantAddress) {
	bool ret = false;
	if (mConfParams->audioEnabled() || mConfParams->videoEnabled()) {
		auto participantInfo = Factory::get()->createParticipantInfo(participantAddress);
		// Participants invited after the start of a conference through the address can only listen to it
		participantInfo->setRole(Participant::Role::Listener);
		ret = addParticipant(participantInfo);
	} else {
#ifdef HAVE_ADVANCED_IM
		const auto &chatRoom = getChatRoom();
		auto serverGroupChatRoom = chatRoom ? dynamic_pointer_cast<ServerChatRoom>(chatRoom) : nullptr;
		if (mConfParams->chatEnabled() && serverGroupChatRoom) {
			if (participantAddress->hasUriParam("gr")) {
				lInfo() << "Conference " << *getConferenceAddress() << ": Not adding participant '"
				        << *participantAddress << "' because it is a gruu address.";
				return false;
			}

			if (findParticipant(participantAddress)) {
				lInfo() << "Conference " << *getConferenceAddress() << ": Not adding participant '"
				        << *participantAddress << "' because it is already a participant";
				return false;
			}

			shared_ptr<Participant> participant = serverGroupChatRoom->findCachedParticipant(participantAddress);

			if (participant == nullptr && !mConfParams->isGroup() && getParticipantCount() == 2) {
				lInfo() << "Conference " << *getConferenceAddress() << ": Not adding participant '"
				        << *participantAddress << "' because this OneToOne chat room already has 2 participants";
				return false;
			}

			/* Handle the case where a participant is removed then re-added to chat room. In such case, until all
			 * devices have left the chatroom, the participant is still referenced in the chatroom, with devices either
			 * left or leaving. Furthermore, registration subscription is still active, so we don't need to wait for a
			 * notify, and we can instead proceed immediately with the INVITE of its devices.
			 */
			if (participant) {
				resumeParticipant(participant);
			} else {
				lInfo() << "Conference " << *getConferenceAddress() << ": Requested to add participant '"
				        << *participantAddress << "', checking capabilities first.";
				list<std::shared_ptr<const Address>> participantsList;
				participantsList.push_back(participantAddress);
				serverGroupChatRoom->subscribeRegistrationForParticipants(participantsList, true);
			}
			ret = true;
		}
#endif // HAVE_ADVANCED_IM
	}
	return ret;
}

bool ServerConference::addParticipant(const std::shared_ptr<ParticipantInfo> &info) {
	const auto &participantAddress = info->getAddress();
	if (!isConferenceEnded() && isConferenceStarted()) {
		const auto initialState = getState();
		if ((initialState == ConferenceInterface::State::CreationPending) ||
		    (initialState == ConferenceInterface::State::Created)) {

			const auto allowedAddresses = getAllowedAddresses();
			auto p = std::find_if(
			    allowedAddresses.begin(), allowedAddresses.end(),
			    [&participantAddress](const auto &address) { return (participantAddress->weakEqual(*address)); });
			if (p == allowedAddresses.end()) {
				auto participantInfo = info->clone()->toSharedPtr();
				participantInfo->setSequenceNumber(-1);
				mInvitedParticipants.push_back(participantInfo);
			}

			std::list<std::shared_ptr<const Address>> addressesList{participantAddress};
			return dialOutAddresses(addressesList);
		}
	} else {
		const auto &endTime = mConfParams->getEndTime();
		const auto &startTime = mConfParams->getStartTime();
		const auto now = time(NULL);
		lError() << "Could not add participant " << *participantAddress << " to the conference because the conference "
		         << *getConferenceAddress() << " is not active right now.";
		if (startTime >= 0) {
			lError() << "Expected start time (" << startTime << "): " << ctime(&startTime);
		} else {
			lError() << "Expected start time: none";
		}
		if (endTime >= 0) {
			lError() << "Expected end time (" << endTime << "): " << ctime(&endTime);
		} else {
			lError() << "Expected end time: none";
		}
		lError() << "Now: " << ctime(&now);
		return false;
	}
	return false;
}

void ServerConference::addParticipantDevice(BCTBX_UNUSED(const shared_ptr<Participant> &participant),
                                            BCTBX_UNUSED(const shared_ptr<ParticipantDeviceIdentity> &deviceInfo)) {
#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		auto serverGroupChatRoom = dynamic_pointer_cast<ServerChatRoom>(chatRoom);

		shared_ptr<ParticipantDevice> device = participant->findDevice(deviceInfo->getAddress(), false);

		if (device) {
			// Nothing to do, but set the name and capabilities because they are not known for the initiator device.
			device->setName(deviceInfo->getName());
			device->setCapabilityDescriptor(deviceInfo->getCapabilityDescriptor());
			serverGroupChatRoom->updateProtocolVersionFromDevice(device);
		} else if (findParticipant(participant->getAddress())) {
			bool allDevLeft = !participant->getDevices().empty() && ServerConference::allDevicesLeft(participant);
			/*
			 * This is a really new device.
			 */
			device = participant->addDevice(deviceInfo->getAddress(), deviceInfo->getName());
			device->setCapabilityDescriptor(deviceInfo->getCapabilityDescriptor());
			serverGroupChatRoom->updateProtocolVersionFromDevice(device);
			shared_ptr<ConferenceParticipantDeviceEvent> event =
			    notifyParticipantDeviceAdded(time(nullptr), false, participant, device);
			getCore()->getPrivate()->mainDb->addEvent(event);

			if (serverGroupChatRoom->getProtocolVersion() < Utils::Version(1, 1) && !getCurrentParams()->isGroup() &&
			    allDevLeft) {
				/* If all other devices have left, let this new device to left state too, it will be invited to join if
				 * a message is sent to it. */
				setParticipantDeviceState(device, ParticipantDevice::State::Left);
			} else {
				setParticipantDeviceState(device, ParticipantDevice::State::ScheduledForJoining);
			}
		} else {
			lWarning() << "Conference " << *getConferenceAddress() << ": Participant device " << participant
			           << " cannot be added because not authorized";
		}
	}
#endif // HAVE_ADVANCED_IM
}

bool ServerConference::addParticipantDevice(std::shared_ptr<Call> call) {
	bool success = Conference::addParticipantDevice(call);
	if (success) {
		call->setConference(getSharedFromThis());
		auto session = call->getActiveSession();
		session->addListener(this);
		auto device = findParticipantDevice(session);
		if (device) {
			device->setJoiningMethod((call->getDirection() == LinphoneCallIncoming)
			                             ? ParticipantDevice::JoiningMethod::DialedIn
			                             : ParticipantDevice::JoiningMethod::DialedOut);
			char label[Conference::sLabelLength];
			belle_sip_random_token(label, sizeof(label));
			device->setLabel(label, LinphoneStreamTypeAudio);
			belle_sip_random_token(label, sizeof(label));
			device->setLabel(label, LinphoneStreamTypeVideo);
			device->setThumbnailStreamLabel(label);
			auto op = session->getPrivate()->getOp();
			auto displayName = L_C_TO_STRING(sal_address_get_display_name(
			    (call->getDirection() == LinphoneCallIncoming) ? op->getFromAddress() : op->getToAddress()));
			if (!displayName.empty()) {
				device->setName(displayName);
			}
			enableScreenSharing(session, false);
			const auto &p = device->getParticipant();
			if (p) {
				time_t creationTime = time(nullptr);
				notifyParticipantDeviceAdded(creationTime, false, p, device);
			}
		}
	}
	return success;
}

int ServerConference::removeParticipant(const std::shared_ptr<Address> &addr) {
	const std::shared_ptr<Participant> participant = findParticipant(addr);
	if (!participant) return -1;
	return removeParticipant(participant) ? 0 : -1;
}

int ServerConference::removeParticipant(const std::shared_ptr<CallSession> &session, const bool preserveSession) {
	int err = 0;

	auto op = session->getPrivate()->getOp();
	shared_ptr<Call> call = getCore()->getCallByCallId(op->getCallId());
	CallSession::State sessionState = session->getState();

	const std::shared_ptr<Address> &remoteAddress = session->getRemoteAddress();
	std::shared_ptr<Participant> participant = findParticipant(remoteAddress);
	if (participant) {
		Conference::removeParticipant(session, preserveSession);
		if (mConfParams->audioEnabled() || mConfParams->videoEnabled()) {
			auto &streamsGroup = dynamic_pointer_cast<MediaSession>(session)->getStreamsGroup();
			mMixerSession->unjoinStreamsGroup(streamsGroup);
		}
	} else {
		if ((sessionState != CallSession::State::Released) && (sessionState != CallSession::State::End)) {
			lError() << "Trying to remove participant " << *session->getRemoteAddress() << " with session " << session
			         << " which is not part of conference " << *getConferenceAddress();
		}
		return -1;
	}

	if (getState() != ConferenceInterface::State::TerminationPending) {

		// Detach call from conference
		if (call) {
			call->setConference(nullptr);
		}

		if (participant->getPreserveSession()) {
			// If the session is already paused,then send an update to kick the participant out of the conference, pause
			// the call otherwise
			if (sessionState == CallSession::State::Paused) {
				lInfo() << "Updating call to notify of conference removal.";
				const MediaSessionParams *params = static_pointer_cast<MediaSession>(session)->getMediaParams();
				MediaSessionParams *currentParams = params->clone();
				currentParams->getPrivate()->setInConference(FALSE);
				currentParams->getPrivate()->setConferenceId("");
				err = static_pointer_cast<MediaSession>(session)->updateFromConference(currentParams);
				delete currentParams;
			} else if ((sessionState != CallSession::State::End) && (sessionState != CallSession::State::Released)) {
				lInfo() << "Pause call to notify of conference removal.";
				/* Kick the session out of the conference by moving to the Paused state. */
				const_cast<MediaSessionParamsPrivate *>(
				    L_GET_PRIVATE(static_pointer_cast<MediaSession>(session)->getMediaParams()))
				    ->setInConference(false);
				const_cast<MediaSessionParamsPrivate *>(
				    L_GET_PRIVATE(static_pointer_cast<MediaSession>(session)->getMediaParams()))
				    ->setConferenceId("");
				err = static_pointer_cast<MediaSession>(session)->pauseFromConference();
			}
		} else {
			// Terminate session (i.e. send a BYE) as per RFC
			// This is the default behaviour
			if (sessionState != CallSession::State::End) {
				err = static_pointer_cast<MediaSession>(session)->terminate();
			}
		}

		/*
		 * Handle the case where only the local participant and a unique remote participant are remaining.
		 * In this case, if the session linked to the participant has to be preserved after the conference, then destroy
		 * the conference and let these two participants to connect directly thanks to a simple call. Indeed, the
		 * conference adds latency and processing that is useless to do for 1-1 conversation.
		 */
		if (!mConfParams->oneParticipantConferenceEnabled() && (mParticipants.size() == 1) && (!preserveSession)) {
			std::shared_ptr<Participant> remainingParticipant = mParticipants.front();
			const bool lastParticipantPreserveSession = remainingParticipant->getPreserveSession();
			auto &devices = remainingParticipant->getDevices();
			if (lastParticipantPreserveSession && (devices.size() == 1)) {

				std::shared_ptr<MediaSession> lastSession =
				    static_pointer_cast<MediaSession>(devices.front()->getSession());

				if (lastSession) {
					lInfo() << "Participant [" << remainingParticipant << "] with " << *lastSession->getRemoteAddress()
					        << " is the last call in conference " << *getConferenceAddress()
					        << ", we will reconnect directly to it.";

					const MediaSessionParams *params = lastSession->getMediaParams();
					// If only one participant is in the conference, the conference is destroyed.
					if (isIn()) {
						lInfo() << "Updating call to notify of conference removal.";
						MediaSessionParams *currentParams = params->clone();
						// If the local participant is in, then an update is sent in order to notify that the call is
						// exiting the conference
						currentParams->getPrivate()->setInConference(FALSE);
						currentParams->getPrivate()->setConferenceId("");
						err = lastSession->updateFromConference(currentParams);
						delete currentParams;
					} else {
						// If the local participant is not in, the call is paused as the local participant is busy
						const_cast<MediaSessionParamsPrivate *>(L_GET_PRIVATE(params))->setInConference(false);

						err = lastSession->pauseFromConference();
					}
				}

				setState(ConferenceInterface::State::TerminationPending);

				leave();

				/* invoke removeParticipant() recursively to remove this last participant. */
				bool success = Conference::removeParticipant(remainingParticipant);
				mMixerSession->unjoinStreamsGroup(lastSession->getStreamsGroup());

				if (lastSession) {
					// Detach call from conference
					auto lastOp = lastSession->getPrivate()->getOp();
					if (lastOp) {
						shared_ptr<Call> lastSessionCall = getCore()->getCallByCallId(lastOp->getCallId());
						if (lastSessionCall) {
							lastSessionCall->setConference(nullptr);
						}
					}
				}

				checkIfTerminated();
				return success ? 0 : -1;
			}
		}
	}

	// If call that we are trying to remove from the conference is in paused by remote state, then it temporarely left
	// the conference therefore it must not be terminated
	if (sessionState != CallSession::State::PausedByRemote) {
		checkIfTerminated();
	}

	return err ? 0 : -1;
}

bool ServerConference::hasAdminLeft() const {
	for (const auto &participant : getParticipants()) {
		if (participant->isAdmin()) return true;
	}
	return false;
}

bool ServerConference::removeParticipant(const std::shared_ptr<Participant> &participant) {
	const auto devices = participant->getDevices();
	const auto participantHasNoDevices = (devices.size() == 0);

#ifdef HAVE_ADVANCED_IM
	if (mConfParams->chatEnabled()) {
		if (participant->isAdmin()) setParticipantAdminStatus(participant, false);
		for (const auto &device : devices) {
			if ((device->getState() == ParticipantDevice::State::Leaving) ||
			    (device->getState() == ParticipantDevice::State::Left))
				continue;
			setParticipantDeviceState(device, ParticipantDevice::State::ScheduledForLeaving);
			updateParticipantDeviceSession(device);
		}
	}
#endif // HAVE_ADVANCED_IM

	bool success = true;
	if ((mConfParams->audioEnabled() || mConfParams->videoEnabled()) && !participantHasNoDevices) {
		for (const auto &d : devices) {
			success &= (removeParticipant(d->getSession(), false) == 0);
		}
	} else {
		success = Conference::removeParticipant(participant);
	}

#ifdef HAVE_ADVANCED_IM
	// Add event to DB only for server group chat rooms
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		auto serverGroupChatRoom = dynamic_pointer_cast<ServerChatRoom>(chatRoom);
		unique_ptr<MainDb> &mainDb = getCore()->getPrivate()->mainDb;
		time_t creationTime = time(nullptr);
		shared_ptr<ConferenceParticipantEvent> event = notifyParticipantRemoved(creationTime, false, participant);
		mainDb->addConferenceParticipantEventToDb(event);

		serverGroupChatRoom->removeQueuedParticipantMessages(participant);

		// Remove participant from the database immediately because it has no devices associated.
		// In case of registration in the future, the devices will attempt to subscribe and the conference server will
		// reply 603 Decline
		if (participantHasNoDevices) {
			lInfo()
			    << "Conference " << *getConferenceAddress() << ": Participant '" << *participant->getAddress()
			    << "' is immediately removed because there has been an explicit request to do it and it has no devices "
			       "associated to it, unsubscribing";
			serverGroupChatRoom->unSubscribeRegistrationForParticipant(participant->getAddress());
			mainDb->deleteChatRoomParticipant(serverGroupChatRoom, participant->getAddress());
		}

		if (!hasAdminLeft()) chooseAnotherAdminIfNoneInConference();
	}
#endif // HAVE_ADVANCED_IM

	return success;
}

void ServerConference::setParticipantAdminStatus(const shared_ptr<Participant> &participant, bool isAdmin) {
	if (isAdmin != participant->isAdmin()) {
		participant->setAdmin(isAdmin);
		time_t creationTime = time(nullptr);
		auto event = notifyParticipantSetAdmin(creationTime, false, participant, isAdmin);
		const auto &chatRoom = getChatRoom();
		if (mConfParams->chatEnabled() && chatRoom && mConfParams->isGroup()) {
			getCore()->getPrivate()->mainDb->addEvent(event);
		}
	}
}

void ServerConference::chooseAnotherAdminIfNoneInConference() {
	if ((!mConfParams->chatEnabled() || (mConfParams->chatEnabled() && mConfParams->isGroup())) &&
	    !mParticipants.empty()) {
		const auto adminParticipant = std::find_if(mParticipants.cbegin(), mParticipants.cend(),
		                                           [&](const auto &p) { return (p->isAdmin() == true); });
		// If not admin participant is found
		if (adminParticipant == mParticipants.cend()) {
			const auto &newAdmin = mParticipants.front();
			setParticipantAdminStatus(newAdmin, true);
			lInfo() << "Conference " << *getConferenceAddress() << " has just automatically designed "
			        << *newAdmin->getAddress() << " as its new admin";
		}
	}
}

void ServerConference::setLocalParticipantStreamCapability(const LinphoneMediaDirection &direction,
                                                           const LinphoneStreamType type) {
	const auto &account = getAccount();
	if (mConfParams->localParticipantEnabled() && !mMe->getDevices().empty() && account &&
	    (type != LinphoneStreamTypeUnknown)) {
		const auto &contactAddress = account->getContactAddress();
		if (contactAddress) {
			std::shared_ptr<Address> devAddr = contactAddress->clone()->toSharedPtr();
			const auto &meDev = mMe->findDevice(devAddr);
			if (meDev) {
				lInfo() << "Setting direction of stream of type " << std::string(linphone_stream_type_to_string(type))
				        << " to " << std::string(linphone_media_direction_to_string(direction)) << " of device "
				        << *meDev->getAddress();
				const auto mediaChanged = meDev->setStreamCapability(direction, type);
				meDev->updateStreamAvailabilities();
				for (const auto &p : getParticipants()) {
					for (const auto &d : p->getDevices()) {
						d->updateStreamAvailabilities();
					}
				}

				if (mediaChanged) {
					time_t creationTime = time(nullptr);
					notifyParticipantDeviceMediaCapabilityChanged(creationTime, false, mMe, meDev);
				}
			} else {
				lError() << "Unable to find device with address " << *devAddr
				         << " among those in the local participant " << *mMe->getAddress();
			}
		}
	}
}

void ServerConference::setSubject(const std::string &subject) {
	const auto previousSubject = getUtf8Subject();
	Conference::setSubject(subject);
	if (subject.compare(previousSubject) != 0) {
		const auto &chatRoom = getChatRoom();
		time_t creationTime = time(nullptr);
		auto event = notifySubjectChanged(creationTime, false, getUtf8Subject());
		if (mConfParams->chatEnabled() && chatRoom) {
			getCore()->getPrivate()->mainDb->addEvent(event);
		}
	}
}

void ServerConference::cleanup() {
	if (mMixerSession) {
		mMixerSession.reset();
	}
	// Detach conference from the session if not already done
	for (const auto &device : getParticipantDevices()) {
		auto session = device->getSession();
		if (session) {
			session->removeListener(this);
		}
	}
	try {
#ifdef HAVE_ADVANCED_IM
		const auto chatEnabled = mConfParams->chatEnabled();
		if (chatEnabled && getCore()->getPrivate()->serverListEventHandler) {
			getCore()->getPrivate()->serverListEventHandler->removeHandler(eventHandler);
		}
#endif // HAVE_ADVANCED_IM
		getCore()->getPrivate()->unregisterListener(this);
	} catch (const bad_weak_ptr &) {
		// Unable to unregister listener here. Core is destroyed and the listener doesn't exist.
	}
#ifdef HAVE_ADVANCED_IM
	if (eventHandler) {
		eventHandler.reset();
	}
#endif // HAVE_ADVANCED_IM
}

void ServerConference::onConferenceTerminated(const std::shared_ptr<Address> &addr) {
	cleanup();
	Conference::onConferenceTerminated(addr);
}

void ServerConference::moveDeviceToPresent(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session)) {
#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		auto serverGroupChatRoom = dynamic_pointer_cast<ServerChatRoom>(chatRoom);
		shared_ptr<ParticipantDevice> device = serverGroupChatRoom->findCachedParticipantDevice(session);
		moveDeviceToPresent(device);
	}
#endif // HAVE_ADVANCED_IM
}

void ServerConference::moveDeviceToPresent(BCTBX_UNUSED(const std::shared_ptr<ParticipantDevice> &device)) {
#ifdef HAVE_ADVANCED_IM
	if (mConfParams->chatEnabled() && device && !ParticipantDevice::isLeavingState(device->getState())) {
		setParticipantDeviceState(device, ParticipantDevice::State::Present);
	}
#endif // HAVE_ADVANCED_IM
}

void ServerConference::onFirstNotifyReceived(BCTBX_UNUSED(const std::shared_ptr<Address> &addr)) {
#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		auto serverGroupChatRoom = dynamic_pointer_cast<ServerChatRoom>(chatRoom);
		for (const auto &participant : getParticipants()) {
			for (const auto &device : participant->getDevices()) {
				if ((*device->getAddress() == *addr) && (serverGroupChatRoom->dispatchMessagesAfterFullState(device))) {
					moveDeviceToPresent(device);
					return;
				}
			}
		}
	}
#endif // HAVE_ADVANCED_IM
}

void ServerConference::checkIfTerminated() {
	const auto &endTime = mConfParams->getEndTime();
	// A conference is considered as terminated if there is no participant left and either it has no end time or it is
	// overdue
	if ((getParticipantDevices(false).size() == 0) && (isConferenceEnded() || (endTime < 0))) {
		// Take a ref because the conference may be immediately go to deleted state
		const auto ref = getSharedFromThis();
		leave();
		if (getState() == ConferenceInterface::State::TerminationPending) {
			setState(ConferenceInterface::State::Terminated);
		} else {
			setState(ConferenceInterface::State::TerminationPending);
		}
		// The server deletes the conference info once the conference is terminated.
		// It avoids having a growing database on the server side
		getCore()->getPrivate()->mainDb->deleteConferenceInfo(getConferenceAddress());
	}
}

void ServerConference::enableScreenSharing(const std::shared_ptr<LinphonePrivate::CallSession> &session, bool notify) {
	if (mConfParams->videoEnabled()) {
		const auto &device = findParticipantDevice(session);
		if (device) {
			const auto screenSharingDevice = getScreenSharingDevice();
			auto mediaSession = static_pointer_cast<LinphonePrivate::MediaSession>(session);
			bool isScreenSharingNegotiated = mediaSession->isScreenSharingNegotiated();
			// A participant device cannot take over screen sharing from another device
			// Check that the device disabling screen sharing is the one who is actually screensharing
			if ((screenSharingDevice && (screenSharingDevice->getSession() == session) && !isScreenSharingNegotiated) ||
			    !screenSharingDevice) {
				bool changed = device->enableScreenSharing(isScreenSharingNegotiated);
				lInfo() << "Screen sharing is " << std::string(isScreenSharingNegotiated ? "enabled" : "disabled")
				        << " for device " << *device->getAddress();
				mMixerSession->enableScreenSharing(isScreenSharingNegotiated, &mediaSession->getStreamsGroup());
				if (changed && notify) {
					// Detect media changes to avoid having to send 2 NOTIFYs:
					// - one for the screen sharing
					// - one for the changes in the media capabilities and availabilities
					device->updateMediaCapabilities();
					device->updateStreamAvailabilities();
					time_t creationTime = time(nullptr);
					notifyParticipantDeviceScreenSharingChanged(creationTime, false, device->getParticipant(), device);
				}
			}
		}
	}
}

int ServerConference::participantDeviceAlerting(const std::shared_ptr<CallSession> &session) {
	const std::shared_ptr<Address> &remoteAddress = session->getRemoteAddress();
	std::shared_ptr<Participant> p = findParticipant(remoteAddress);
	if (p) {
		std::shared_ptr<ParticipantDevice> device = p->findDevice(session);
		if (device) {
			return participantDeviceAlerting(p, device);
		} else {
			lDebug() << "Participant alerting: Unable to find device with session " << session
			         << " among devices of participant " << *p->getAddress() << " of conference "
			         << *getConferenceAddress();
		}
	}
	return -1;
}

int ServerConference::participantDeviceAlerting(BCTBX_UNUSED(const std::shared_ptr<Participant> &participant),
                                                const std::shared_ptr<ParticipantDevice> &device) {
	lInfo() << "Device " << *device->getAddress() << " changed state to alerting";
	device->updateMediaCapabilities();
	device->updateStreamAvailabilities();
	device->setState(ParticipantDevice::State::Alerting);
	return 0;

	return -1;
}

int ServerConference::participantDeviceJoined(const std::shared_ptr<CallSession> &session) {
	const std::shared_ptr<Address> &remoteAddress = session->getRemoteAddress();
	std::shared_ptr<Participant> p = findParticipant(remoteAddress);
	if (p) {
		std::shared_ptr<ParticipantDevice> device = p->findDevice(session);
		if (device) {
			return participantDeviceJoined(p, device);
		} else {
			lDebug() << "Participant joined: Unable to find device with session " << session
			         << " among devices of participant " << *p->getAddress() << " of conference "
			         << *getConferenceAddress();
		}
	}
	return -1;
}

int ServerConference::participantDeviceJoined(BCTBX_UNUSED(const std::shared_ptr<Participant> &participant),
                                              const std::shared_ptr<ParticipantDevice> &device) {
	int success = -1;
	const auto mediaCapabilitiesChanged = device->updateMediaCapabilities();
	if ((!mediaCapabilitiesChanged.empty() || (device->getState() != ParticipantDevice::State::Present)) &&
	    (getState() == ConferenceInterface::State::Created)) {
		lInfo() << "Device " << *device->getAddress() << " joined conference " << *getConferenceAddress();
		device->updateStreamAvailabilities();
		device->setState(ParticipantDevice::State::Present);
		return 0;
	}
	return success;
}

int ServerConference::participantDeviceLeft(const std::shared_ptr<CallSession> &session) {
	const std::shared_ptr<Address> &remoteAddress = session->getRemoteAddress();
	std::shared_ptr<Participant> p = findParticipant(remoteAddress);
	if (p) {
		std::shared_ptr<ParticipantDevice> device = p->findDevice(session);
		if (device) {
			return participantDeviceLeft(p, device);
		} else {
			lWarning() << "Participant left: Unable to find device with session " << session
			           << " among devices of participant " << *p->getAddress() << " of conference "
			           << *getConferenceAddress();
		}
	}
	return -1;
}

int ServerConference::participantDeviceLeft(BCTBX_UNUSED(const std::shared_ptr<Participant> &participant),
                                            const std::shared_ptr<ParticipantDevice> &device) {
	int success = -1;
	const auto mediaCapabilitiesChanged = device->updateMediaCapabilities();
	if ((!mediaCapabilitiesChanged.empty() || (device->getState() != ParticipantDevice::State::OnHold)) &&
	    (getState() == ConferenceInterface::State::Created)) {
		lInfo() << "Device " << *device->getAddress() << " left conference " << *getConferenceAddress();
		device->updateStreamAvailabilities();
		device->setState(ParticipantDevice::State::OnHold);
		return 0;
	}
	return success;
}

int ServerConference::participantDeviceMediaCapabilityChanged(const std::shared_ptr<CallSession> &session) {
	const std::shared_ptr<Address> &remoteAddress = session->getRemoteAddress();
	std::shared_ptr<Participant> p = findParticipant(remoteAddress);
	int success = -1;
	if (p) {
		std::shared_ptr<ParticipantDevice> device = p->findDevice(session);
		if (device) {
			success = participantDeviceMediaCapabilityChanged(p, device);
		} else {
			lWarning() << "Participant media capability changed: Unable to find device with session " << session
			           << " among devices of participant " << *p->getAddress() << " of conference "
			           << *getConferenceAddress();
		}
	}
	return success;
}

int ServerConference::participantDeviceMediaCapabilityChanged(const std::shared_ptr<Address> &addr) {
	std::shared_ptr<Participant> p = findParticipant(addr);
	int success = -1;
	for (const auto &d : p->getDevices()) {
		success = participantDeviceMediaCapabilityChanged(p, d);
	}
	return success;
}

int ServerConference::participantDeviceMediaCapabilityChanged(const std::shared_ptr<Participant> &participant,
                                                              const std::shared_ptr<ParticipantDevice> &device) {
	int success = -1;
	const auto mediaCapabilitiesChanged = device->updateMediaCapabilities();
	if (!mediaCapabilitiesChanged.empty() &&
	    ((getState() == ConferenceInterface::State::CreationPending) ||
	     (getState() == ConferenceInterface::State::Created)) &&
	    (device->getState() == ParticipantDevice::State::Present)) {
		lInfo() << "Device " << *device->getAddress() << " in conference " << *getConferenceAddress()
		        << " changed its media capabilities";
		device->updateStreamAvailabilities();
		time_t creationTime = time(nullptr);
		notifyParticipantDeviceMediaCapabilityChanged(creationTime, false, participant, device);
		return 0;
	}
	return success;
}

int ServerConference::participantDeviceSsrcChanged(const std::shared_ptr<CallSession> &session,
                                                   const LinphoneStreamType type,
                                                   uint32_t ssrc) {
	const std::shared_ptr<Address> &remoteAddress = session->getRemoteAddress();
	std::shared_ptr<Participant> p = findParticipant(remoteAddress);
	int success = -1;
	if (p) {
		std::shared_ptr<ParticipantDevice> device = p->findDevice(session);
		if (device) {
			bool updated = device->setSsrc(type, ssrc);
			if (updated) {
				lInfo() << "Setting " << std::string(linphone_stream_type_to_string(type))
				        << " ssrc of participant device " << *device->getAddress() << " in conference "
				        << *getConferenceAddress() << " to " << ssrc;
				time_t creationTime = time(nullptr);
				notifyParticipantDeviceMediaCapabilityChanged(creationTime, false, p, device);
			} else {
				lInfo() << "Leaving unchanged ssrc of participant device " << *device->getAddress() << " in conference "
				        << *getConferenceAddress() << " whose value is " << ssrc;
			}
			return 0;
		}
	}
	lInfo() << "Unable to set " << std::string(linphone_stream_type_to_string(type)) << " ssrc to " << ssrc
	        << " because participant device with session " << session << " cannot be found in conference "
	        << *getConferenceAddress();
	return success;
}

int ServerConference::participantDeviceSsrcChanged(const std::shared_ptr<CallSession> &session,
                                                   uint32_t audioSsrc,
                                                   uint32_t videoSsrc) {
	const std::shared_ptr<Address> &remoteAddress = session->getRemoteAddress();
	std::shared_ptr<Participant> p = findParticipant(remoteAddress);
	int success = -1;
	if (p) {
		std::shared_ptr<ParticipantDevice> device = p->findDevice(session);
		if (device) {
			if (device->setSsrc(LinphoneStreamTypeAudio, audioSsrc) ||
			    device->setSsrc(LinphoneStreamTypeVideo, videoSsrc)) {
				time_t creationTime = time(nullptr);
				notifyParticipantDeviceMediaCapabilityChanged(creationTime, false, p, device);
			} else {
				lInfo() << "Leaving unchanged ssrcs of participant device " << *device->getAddress()
				        << " in conference " << *getConferenceAddress() << " whose values are";
				lInfo() << "- audio -> " << audioSsrc;
				lInfo() << "- video -> " << videoSsrc;
			}
			return 0;
		}
	}
	lInfo() << "Unable to set audio ssrc to " << audioSsrc << " and video ssrc to " << videoSsrc
	        << " because participant device with session " << session << " cannot be found in conference "
	        << *getConferenceAddress();
	return success;
}

int ServerConference::getParticipantDeviceVolume(const std::shared_ptr<ParticipantDevice> &device) {
	MS2AudioMixer *mixer = dynamic_cast<MS2AudioMixer *>(mMixerSession->getMixerByType(SalAudio));

	if (mixer) {
		MSAudioConference *conf = mixer->getAudioConference();
		return ms_audio_conference_get_participant_volume(conf, device->getSsrc(LinphoneStreamTypeAudio));
	}

	return AUDIOSTREAMVOLUMES_NOT_FOUND;
}

int ServerConference::terminate() {
	if (mConfParams->audioEnabled() || mConfParams->videoEnabled()) {
		const auto conferenceAddressStr =
		    (getConferenceAddress() ? getConferenceAddress()->toString() : std::string("sip::unknown"));
		lInfo() << "Terminate conference " << conferenceAddressStr;
		// Take a ref because the conference may be immediately go to deleted state if terminate is called when there
		// are 0 participants
		const auto ref = getSharedFromThis();
		setState(ConferenceInterface::State::TerminationPending);

		size_t noDevices = 0;
		auto participantIt = mParticipants.begin();
		while (participantIt != mParticipants.end()) {
			auto participant = *participantIt;
			const auto devices = participant->getDevices();
			noDevices += devices.size();
			participantIt++;
			if (devices.size() > 0) {
				for (const auto &d : devices) {
					std::shared_ptr<MediaSession> session = static_pointer_cast<MediaSession>(d->getSession());
					if (session) {
						lInfo() << "Terminating session of participant device " << *d->getAddress();
						session->terminate();
					}
				}
			} else {
				removeParticipant(participant);
			}
		}

		const auto zeroDevices = (noDevices == 0);
		if (zeroDevices
#ifdef HAVE_ADVANCED_IM
		    || !eventHandler
#endif // HAVE_ADVANCED_IM
		) {
			setState(ConferenceInterface::State::Terminated);
		}

		// The server deletes the conference info once the conference is terminated.
		// It avoids having a growing database on the server side
		if (isConferenceEnded() || (linphone_core_get_global_state(getCore()->getCCore()) == LinphoneGlobalOn)) {
			getCore()->getPrivate()->mainDb->deleteConferenceInfo(getConferenceAddress());
		}
	} else {
		setChatRoom(nullptr);
	}
	return 0;
}

int ServerConference::enter() {
	if (mConfParams->localParticipantEnabled()) {
		if (linphone_core_sound_resources_locked(getCore()->getCCore())) return -1;
		if (linphone_core_get_current_call(getCore()->getCCore()))
			linphone_call_pause(linphone_core_get_current_call(getCore()->getCCore()));

		const auto &meAddress = mMe->getAddress();
		lInfo() << *meAddress << " is rejoining conference " << *getConferenceAddress();
		mOrganizer = meAddress;

		addLocalEndpoint();
		if (mMe->getDevices().size() > 0) {
			participantDeviceJoined(mMe, mMe->getDevices().front());
		}
	}
	return 0;
}

void ServerConference::leave() {
	if (isIn() && ((mConfParams->audioEnabled() || mConfParams->videoEnabled()))) {
		lInfo() << *getMe()->getAddress() << " is leaving conference " << *getConferenceAddress();
		if (mMe->getDevices().size() > 0) {
			participantDeviceLeft(mMe, mMe->getDevices().front());
		}
		removeLocalEndpoint();
	}
}

const std::shared_ptr<Address> ServerConference::getOrganizer() const {
	return mOrganizer;
}

AudioControlInterface *ServerConference::getAudioControlInterface() const {
	return mMixerSession ? dynamic_cast<AudioControlInterface *>(mMixerSession->getMixerByType(SalAudio)) : nullptr;
}

VideoControlInterface *ServerConference::getVideoControlInterface() const {
	return mMixerSession ? dynamic_cast<VideoControlInterface *>(mMixerSession->getMixerByType(SalVideo)) : nullptr;
}

AudioStream *ServerConference::getAudioStream() {
	MS2AudioMixer *mixer =
	    mMixerSession ? dynamic_cast<MS2AudioMixer *>(mMixerSession->getMixerByType(SalAudio)) : nullptr;
	return mixer ? mixer->getAudioStream() : nullptr;
}

int ServerConference::startRecording(const std::string &path) {
	MS2AudioMixer *mixer =
	    mMixerSession ? dynamic_cast<MS2AudioMixer *>(mMixerSession->getMixerByType(SalAudio)) : nullptr;
	if (mixer) {
		mixer->setRecordPath(path);
		mixer->startRecording();
		// TODO: error reporting is absent.
	} else {
		lError() << "ServerConference::startRecording(): no audio mixer.";
		return -1;
	}
	return 0;
}

void ServerConference::onCallSessionStateChanged(const std::shared_ptr<CallSession> &session,
                                                 CallSession::State state,
                                                 BCTBX_UNUSED(const std::string &message)) {

	const auto &chatRoom = getChatRoom();
	const auto conferenceAddressStr =
	    (getConferenceAddress() ? getConferenceAddress()->toString() : std::string("sip::unknown"));
	if (mConfParams->chatEnabled() && chatRoom) {
#ifdef HAVE_ADVANCED_IM
		auto serverGroupChatRoom = dynamic_pointer_cast<ServerChatRoom>(chatRoom);
		auto initiatorDevice = serverGroupChatRoom->getInitiatorDevice();
		shared_ptr<ParticipantDevice> device = serverGroupChatRoom->findCachedParticipantDevice(session);
		if (!device) {
			lInfo() << this << " onCallSessionStateChanged on unknown device (maybe not yet).";
			if ((state == CallSession::State::Released) && (session->getReason() == LinphoneReasonNotAcceptable) &&
			    (getState() == ConferenceInterface::State::CreationFailed)) {
				// Delete the chat room from the main DB as its termination process started and it cannot be retrieved
				// in the future
				lInfo() << this << ": Delete chatroom from MainDB as its creation failed";
				unique_ptr<MainDb> &mainDb = getCore()->getPrivate()->mainDb;
				mainDb->deleteChatRoom(getConferenceId());
				setState(ConferenceInterface::State::TerminationPending);
				setState(ConferenceInterface::State::Terminated);
				requestDeletion();
				session->removeListener(this);
			}
			return;
		}
		switch (state) {
			case CallSession::State::Connected:
				if (device->getState() == ParticipantDevice::State::Leaving) {
					byeDevice(device);
				} else {
					if ((session->getDirection() == LinphoneCallOutgoing) &&
					    !serverGroupChatRoom->dispatchMessagesAfterFullState(session)) {
						// According to RFC3261, a request is successful once the 200Ok is received
						moveDeviceToPresent(session);
					}

					auto joiningPendingAfterCreation = serverGroupChatRoom->isJoiningPendingAfterCreation();
					if (joiningPendingAfterCreation && initiatorDevice && initiatorDevice == device) {
						lInfo() << "Session of the initiation of the chatroom is in state " << Utils::toString(state)
						        << " things can start now.";
						serverGroupChatRoom->setJoiningPendingAfterCreation(false);
						updateParticipantsSessions();
					}
				}
				break;
			case CallSession::State::End: {
				const auto errorInfo = session->getErrorInfo();
				if (errorInfo != nullptr && linphone_error_info_get_protocol_code(errorInfo) > 299) {
					if (device->getState() == ParticipantDevice::State::Joining ||
					    device->getState() == ParticipantDevice::State::Present) {
						lWarning() << this << ": Received a BYE from " << *device->getAddress() << " with reason "
						           << linphone_error_info_get_protocol_code(errorInfo)
						           << ", setting it back to ScheduledForJoining.";
						setParticipantDeviceState(device, ParticipantDevice::State::ScheduledForJoining);
						if (linphone_error_info_get_protocol_code(errorInfo) == 408 && initiatorDevice &&
						    initiatorDevice == device) {
							// Recovering if the initiator of the chatroom did not receive the 200Ok or the ACK has been
							// lost
							inviteDevice(device);
						}
					}
				} else {
					// Do not kick a participant out when the core loses its network
					if (linphone_core_is_network_reachable(getCore()->getCCore()) &&
					    (device->getState() == ParticipantDevice::State::Present)) {
						lInfo() << this << ": " << *device->getParticipant()->getAddress()
						        << " is leaving the chatroom.";
						serverGroupChatRoom->onBye(device);
					}
				}
			} break;
			case CallSession::State::Released:
				/* Handle the case of participant we've send a BYE. */
				if (device->getState() == ParticipantDevice::State::Leaving &&
				    session->getPreviousState() == CallSession::State::End) {
					if (session->getReason() == LinphoneReasonNone) {
						/* We've received a 200 Ok for our BYE, so it is assumed to be left. */
						setParticipantDeviceState(device, ParticipantDevice::State::Left);
					} else if (session->getReason() == LinphoneReasonNoMatch) {
						/* Our current session was lost, but the device is currently reachable, so retry to send the BYE
						 * now. */
						byeDevice(device);
					}
				}
				session->removeListener(this);
				break;
			case CallSession::State::UpdatedByRemote: {
				shared_ptr<Participant> participant = findParticipant(session);
				if (participant && participant->isAdmin()) {
					/* The only thing that a participant can change with re-INVITE is the subject. */
					handleSubjectChange(session->getPrivate()->getOp());
					serverGroupChatRoom->handleEphemeralSettingsChange(session);
				}
			} break;
			default:
				break;
		}
		linphone_chat_room_notify_session_state_changed(chatRoom->toC(), static_cast<LinphoneCallState>(state),
		                                                message.c_str());
#endif // HAVE_ADVANCED_IM
	} else {
		const std::shared_ptr<Address> &remoteAddress = session->getRemoteAddress();
		const auto &device = findParticipantDevice(session);
		const auto &deviceState = device ? device->getState() : ParticipantDevice::State::ScheduledForJoining;
		const auto conferenceAddressStr =
		    (getConferenceAddress() ? getConferenceAddress()->toString() : std::string("sip::unknown"));
		switch (state) {
			case CallSession::State::OutgoingRinging:
				participantDeviceAlerting(session);
				break;
			case CallSession::State::Connected:
				if (getState() == ConferenceInterface::State::Created) {
					enter();
				}
				break;
			case CallSession::State::StreamsRunning: {
				auto op = session->getPrivate()->getOp();
				auto cppCall = getCore()->getCallByCallId(op->getCallId());
				if (!addParticipantDevice(cppCall)) {
					// If the participant is already in the conference
					const auto &participant = findParticipant(remoteAddress);
					auto remoteContactAddress = session->getRemoteContactAddress();
					if (participant) {
						if (device) {
							const auto deviceAddr = device->getAddress();
							const std::shared_ptr<Address> newDeviceAddress = remoteContactAddress;

							// Compare the adresses as strings because a simple comparison as done by the operator== is
							// not enough. In fact, it will return that the addresses are equal if one has a different
							// set of parameters than the other
							bool hasCallContactChanged =
							    (deviceAddr->toStringOrdered() != newDeviceAddress->toStringOrdered());
							if (hasCallContactChanged) {
								// The remote contact address of the device changed during the call. This may be caused
								// by a call that started before the registration was completed
								lInfo() << "Updating address of participant device " << device << " with session "
								        << device->getSession() << " from " << *deviceAddr << " to "
								        << *newDeviceAddress;
								auto otherDevice = participant->findDevice(newDeviceAddress);
								// If a device with the same address has been found, then remove it from the participant
								// list and copy subscription event. Otherwise, notify that it has been added
								if (otherDevice && (otherDevice != device)) {
									time_t creationTime = time(nullptr);
									device->setTimeOfDisconnection(creationTime);
									device->setDisconnectionMethod(ParticipantDevice::DisconnectionMethod::Booted);
									const auto reason("Reason: SIP;text=address changed");
									device->setDisconnectionReason(reason);
									// As the device changed address, notify that the current device has been removed
									notifyParticipantDeviceRemoved(creationTime, false, participant, device);

									if (!device->getConferenceSubscribeEvent() &&
									    otherDevice->getConferenceSubscribeEvent()) {
										// Move subscription event pointer to device.
										// This is required because if the call starts before the registration process,
										// the device address may have an unresolved address whereas the subscription
										// may have started after the device is fully registered, hence the full device
										// address is known.
										device->setConferenceSubscribeEvent(otherDevice->getConferenceSubscribeEvent());
										otherDevice->setConferenceSubscribeEvent(nullptr);
									}
									// Delete device having the same address
									// First remove device from the device list to avoid sending a participant device
									// removed
									participant->removeDevice(newDeviceAddress);
									auto otherDeviceSession = otherDevice->getSession();
									if (otherDeviceSession) {
										otherDeviceSession->terminate();
									}

									creationTime = time(nullptr);
									device->setAddress(remoteContactAddress);
									notifyParticipantDeviceMediaCapabilityChanged(creationTime, false, participant,
									                                              device);
								} else {
									device->setAddress(remoteContactAddress);
									participantDeviceJoined(session);
								}
							}
						}
						if (deviceState == ParticipantDevice::State::Present) {
							participantDeviceMediaCapabilityChanged(session);
						} else if ((deviceState == ParticipantDevice::State::Joining) ||
						           (deviceState == ParticipantDevice::State::ScheduledForJoining)) {
							if (!cppCall->mediaInProgress() ||
							    !!!linphone_config_get_int(linphone_core_get_config(getCore()->getCCore()), "sip",
							                               "update_call_when_ice_completed", TRUE)) {
								// Participants complete their addition to a conference when the call goes back to
								// the StreamsRunning state
								finalizeParticipantAddition(cppCall);
							} else {
								auto contactAddress = session->getContactAddress();
								if (contactAddress && contactAddress->isValid() &&
								    contactAddress->hasParam("isfocus")) {
									device->setState(ParticipantDevice::State::Joining);
								}
							}
						} else {
							participantDeviceJoined(session);
						}
					} else {
						lError() << "Unable to update device with address " << *remoteContactAddress
						         << " because it was not found in conference " << conferenceAddressStr;
					}
					bool admin = remoteContactAddress->hasParam("admin") &&
					             Utils::stob(remoteContactAddress->getParamValue("admin"));
					setParticipantAdminStatus(participant, admin);
				} else {
					lError() << "Unable to update admin status and device address as no participant with address "
					         << *remoteAddress << " has been found in conference " << conferenceAddressStr;
				}
			} break;
			case CallSession::State::PausedByRemote:
				// The participant temporarely left the conference and put its call in pause
				// If a call in a local conference is paused by remote, it means that the remote participant temporarely
				// left the call, hence notify that no audio and video is available
				lInfo() << "Call in conference has been put on hold by remote device, hence participant "
				        << *remoteAddress << " temporarely left conference " << conferenceAddressStr;
				participantDeviceLeft(session);
				break;
			case CallSession::State::UpdatedByRemote: {
				if (session && device &&
				    ((deviceState == ParticipantDevice::State::Present) ||
				     (deviceState == ParticipantDevice::State::Joining))) {
					enableScreenSharing(session, true);
					const auto op = session->getPrivate()->getOp();
					// The remote participant requested to change subject
					if (sal_custom_header_find(op->getRecvCustomHeaders(), "Subject")) {
						const auto &subject = op->getSubject();
						auto protocols = Utils::parseCapabilityDescriptor(device->getCapabilityDescriptor());
						auto conferenceProtocol = protocols.find("conference");
						// Do not change subject if the value of the Subject header is a predefined one. This may mean
						// that the reINVITE is coming from an old SDK which added some predefined subjects when events
						// occurred
						if (((conferenceProtocol != protocols.end()) &&
						     (conferenceProtocol->second >= Utils::Version(1, 0))) ||
						    !CallSession::isPredefinedSubject(subject)) {
							// Handle subject change
							lInfo() << "conference " << conferenceAddressStr << " changed subject to \"" << subject
							        << "\"";
							setSubject(subject);
						}
					}
				}
			} break;
			case CallSession::State::End:
			case CallSession::State::Error: {
				lInfo() << "Removing terminated call (local address " << *session->getLocalAddress()
				        << " remote address " << *remoteAddress << ") from conference " << this << " ("
				        << conferenceAddressStr << ")";
				const auto &sessionErrorInfo = session->getErrorInfo();
				if (sessionErrorInfo && (linphone_error_info_get_reason(sessionErrorInfo) == LinphoneReasonBusy)) {
					removeParticipantDevice(session);
				} else {
					removeParticipant(session, false);
				}
			} break;
			default:
				break;
		}
	}
}

void ServerConference::onAckReceived(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
                                     BCTBX_UNUSED(LinphoneHeaders *headers)) {
#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		auto serverGroupChatRoom = dynamic_pointer_cast<ServerChatRoom>(chatRoom);
		if (serverGroupChatRoom && !serverGroupChatRoom->dispatchMessagesAfterFullState(session)) {
			moveDeviceToPresent(session);
		}
	}
#endif // HAVE_ADVANCED_IM
}

/* The removal of participant device is done only when such device disapears from registration database, ie when a
 * device unregisters explicitely or removed by an administrator.
 */
void ServerConference::removeParticipantDevice(BCTBX_UNUSED(const shared_ptr<Participant> &participant),
                                               BCTBX_UNUSED(const std::shared_ptr<Address> &deviceAddress)) {
#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		shared_ptr<Participant> participantCopy = participant; // make a copy of the shared_ptr because the participant
		                                                       // may be removed by setParticipantDeviceState().
		lInfo() << "Device " << *deviceAddress << " is removed from conference " << *getConferenceAddress();
		auto participantDevice = participant->findDevice(deviceAddress);
		if (!participantDevice) {
			lInfo() << "Device " << *deviceAddress << " should be removed but it is not found in conference "
			        << *getConferenceAddress();
			return;
		}
		// Notify to everyone the retirement of this device.
		auto deviceEvent = notifyParticipantDeviceRemoved(time(nullptr), false, participant, participantDevice);
		getCore()->getPrivate()->mainDb->addEvent(deviceEvent);

		// First set it as left, so that it may eventually trigger the destruction of the chatroom if no device are
		// present for any participant.
		setParticipantDeviceState(participantDevice, ParticipantDevice::State::Left, false);
		participantCopy->removeDevice(deviceAddress);
	}
#endif // HAVE_ADVANCED_IM
}

int ServerConference::removeParticipantDevice(const std::shared_ptr<CallSession> &session) {
	return Conference::removeParticipantDevice(session);
}

void ServerConference::setParticipantDeviceState(BCTBX_UNUSED(const shared_ptr<ParticipantDevice> &device),
                                                 BCTBX_UNUSED(ParticipantDevice::State state),
                                                 BCTBX_UNUSED(bool notify)) {
#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		auto serverGroupChatRoom = dynamic_pointer_cast<ServerChatRoom>(chatRoom);
		// Do not change state of participants if the core is shutting down.
		// If a participant is about to leave and its call session state is End, it will be released during shutdown
		// event though the participant may not be notified yet as it is offline
		if (linphone_core_get_global_state(getCore()->getCCore()) == LinphoneGlobalOn) {
			auto deviceAddress = device->getAddress();
			lInfo() << "Conference " << *getConferenceAddress() << ": Set participant device '" << *deviceAddress
			        << "' state to " << state;
			device->setState(state, notify);
			getCore()->getPrivate()->mainDb->updateChatRoomParticipantDevice(chatRoom, device);
			const auto &participant = findParticipant(deviceAddress);
			switch (state) {
				case ParticipantDevice::State::ScheduledForLeaving:
				case ParticipantDevice::State::Leaving:
					serverGroupChatRoom->removeQueuedParticipantMessages(participant);
					break;
				case ParticipantDevice::State::Left:
					serverGroupChatRoom->removeQueuedParticipantMessages(participant);
					onParticipantDeviceLeft(device);
					break;
				case ParticipantDevice::State::Present:
					serverGroupChatRoom->dispatchQueuedMessages();
					break;
				default:
					break;
			}
		}
	}
#endif // HAVE_ADVANCED_IM
}

bool ServerConference::allDevicesLeft(const std::shared_ptr<Participant> &participant) {
	bool allLeft = true;

	for (const auto &device : participant->getDevices()) {
		if (device->getState() != ParticipantDevice::State::Left) {
			allLeft = false;
			break;
		}
	}
	return allLeft;
}

void ServerConference::onParticipantDeviceLeft(BCTBX_UNUSED(const std::shared_ptr<ParticipantDevice> &device)) {
#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		auto serverGroupChatRoom = dynamic_pointer_cast<ServerChatRoom>(chatRoom);
		unique_ptr<MainDb> &mainDb = getCore()->getPrivate()->mainDb;
		lInfo() << "Conference " << *getConferenceAddress() << ": Participant device '" << *device->getAddress()
		        << "' left";

		auto session = device->getSession();
		if (session) session->removeListener(this);

		if (getCurrentParams()->isGroup() || serverGroupChatRoom->getProtocolVersion() >= Utils::Version(1, 1)) {
			shared_ptr<Participant> participant =
			    const_pointer_cast<Participant>(device->getParticipant()->getSharedFromThis());
			if (ServerConference::allDevicesLeft(participant) &&
			    findParticipant(participant->getAddress()) == nullptr) {
				lInfo() << "Conference " << *getConferenceAddress() << ": Participant '" << *participant->getAddress()
				        << "'removed and last device left, unsubscribing";
				serverGroupChatRoom->unSubscribeRegistrationForParticipant(participant->getAddress());
				mainDb->deleteChatRoomParticipant(serverGroupChatRoom, participant->getAddress());
			}
		}

		// device left, we no longuer need to receive subscription info from it
		if (device->isSubscribedToConferenceEventPackage()) {
			lError() << "Conference " << *getConferenceAddress() << " still subscription pending for [" << device
			         << "], terminating in emergency";
			// try to terminate subscription if any, but do not wait for anser.
			auto ev = device->getConferenceSubscribeEvent();
			ev->clearCallbacksList();
			ev->terminate();
			device->setConferenceSubscribeEvent(nullptr);
		}

		/* if all devices of participants are left we'll delete the chatroom*/
		if (getCore()->emptyChatroomsDeletionEnabled()) {
			bool allLeft = true;
			for (const auto &participant : serverGroupChatRoom->getCachedParticipants()) {
				if (!ServerConference::allDevicesLeft(participant)) {
					allLeft = false;
					break;
				}
			}
			if (allLeft) {
				// Delete the chat room from the main DB as its termination process started and it cannot be
				// retrieved in the future
				lInfo() << "Conference " << *getConferenceAddress()
				        << ": Delete chatroom from MainDB as last participant has left";
				mainDb->deleteChatRoom(getConferenceId());
				if (getState() != ConferenceInterface::State::TerminationPending) {
					setState(ConferenceInterface::State::TerminationPending);
				}
				setState(ConferenceInterface::State::Terminated);
				lInfo() << "Conference " << *getConferenceAddress() << ": No participant left, deleting the chat room";
				requestDeletion();
			}
		}
	}
#endif // HAVE_ADVANCED_IM
}

/*
 * This method is in charge of applying the state of a participant device to the SIP session
 */
void ServerConference::updateParticipantDeviceSession(BCTBX_UNUSED(const shared_ptr<ParticipantDevice> &device),
                                                      BCTBX_UNUSED(bool freshlyRegistered)) {
	if (mConfParams->chatEnabled()) {
		switch (device->getState()) {
			case ParticipantDevice::State::ScheduledForJoining:
				inviteDevice(device);
				break;
			case ParticipantDevice::State::Joining:
				if (freshlyRegistered) inviteDevice(device);
				break;
			case ParticipantDevice::State::ScheduledForLeaving:
				byeDevice(device);
				break;
			case ParticipantDevice::State::Leaving:
				if (freshlyRegistered) byeDevice(device);
				break;
			case ParticipantDevice::State::Alerting:
			case ParticipantDevice::State::Present:
			case ParticipantDevice::State::OnHold:
			case ParticipantDevice::State::MutedByFocus:
			case ParticipantDevice::State::Left:
				// nothing to do
				break;
		}
	}
}

void ServerConference::updateParticipantDevices(
    BCTBX_UNUSED(const std::shared_ptr<Address> &participantAddress),
    BCTBX_UNUSED(const list<shared_ptr<ParticipantDeviceIdentity>> &devices)) {
#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		auto serverGroupChatRoom = dynamic_pointer_cast<ServerChatRoom>(chatRoom);
		bool newParticipantReginfo = false;
		const auto &registrationSubscriptions = serverGroupChatRoom->getRegistrationSubscriptions();
		auto it = registrationSubscriptions.find(participantAddress->getUri().toString());
		/*
		 * For security, since registration information might come from outside, make sure that the device list we
		 * are asked to add are from a participant for which we have requested device information, ie a participant
		 * that is in the process of being added to the chatroom
		 */
		if (it == registrationSubscriptions.end()) {
			lError() << "updateParticipantDevices(): " << *participantAddress
			         << " registration info was not requested.";
			return;
		} else {
			newParticipantReginfo = serverGroupChatRoom->removeInvitedParticipants(participantAddress);
		}
		shared_ptr<Participant> participant;

		if (newParticipantReginfo) {
			if (!devices.empty()) {
				participant = addParticipantToList(participantAddress);
			} else {
				lInfo() << "Conference " << *getConferenceAddress() << " " << *participantAddress
				        << " has no compatible devices.";
				serverGroupChatRoom->unSubscribeRegistrationForParticipant(participantAddress);
				serverGroupChatRoom->removeCachedParticipant(participantAddress);
				return;
			}
		} else {
			participant = serverGroupChatRoom->findCachedParticipant(participantAddress);
		}

		if (!participant) {
			lError() << "Conference " << *getConferenceAddress()
			         << " participant devices updated for unknown participant, ignored.";
			return;
		}
		lInfo() << "Conference " << *getConferenceAddress() << ": Setting " << devices.size()
		        << " participant device(s) for " << *participantAddress;

		// Remove devices that are in the chatroom but no longer in the given list
		list<shared_ptr<ParticipantDevice>> devicesToRemove;
		for (const auto &device : participant->getDevices()) {
			auto predicate = [device](const shared_ptr<ParticipantDeviceIdentity> &deviceIdentity) {
				return *device->getAddress() == *deviceIdentity->getAddress();
			};
			auto it = find_if(devices.cbegin(), devices.cend(), predicate);
			if (it == devices.cend()) {
				lInfo() << "Conference " << *getConferenceAddress() << " Device " << *device->getAddress()
				        << " is no longer registered, it will be removed from the chatroom.";
				devicesToRemove.push_back(device);
			}
		}
		// Add all the devices in the given list, if already present they will be ignored
		for (const auto &device : devices)
			addParticipantDevice(participant, device);

		// Remove all devices that are no longer existing.
		for (auto &device : devicesToRemove)
			removeParticipantDevice(participant, device->getAddress());

		auto protocolVersion = serverGroupChatRoom->getProtocolVersion();
		if (protocolVersion < CorePrivate::groupChatProtocolVersion) {
			/* we need to recheck in case some devices have upgraded. */
			serverGroupChatRoom->determineProtocolVersion();
			if (protocolVersion == CorePrivate::groupChatProtocolVersion) {
				lInfo() << "It's marvellous, all devices are now up to date !";
			}
		}
	}
#endif // HAVE_ADVANCED_IM
}

void ServerConference::declineSession(const shared_ptr<CallSession> &session, LinphoneReason reason) {
	session->decline(reason);
}

void ServerConference::acceptSession(const shared_ptr<CallSession> &session) {
	if (session->getState() == CallSession::State::UpdatedByRemote) session->acceptUpdate();
	else session->accept();
}

void ServerConference::conclude() {
#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		auto serverGroupChatRoom = dynamic_pointer_cast<ServerChatRoom>(chatRoom);
		lInfo() << "Conference " << *getConferenceAddress()
		        << " All devices are known, the chatroom creation can be concluded.";
		shared_ptr<CallSession> session = serverGroupChatRoom->getInitiatorDevice()->getSession();

		if (!session) {
			lError() << "Conference " << *getConferenceAddress()
			         << "ServerConference::conclude(): initiator's session died.";
			requestDeletion();
			return;
		}

		const auto device = findParticipantDevice(session);
		if (getParticipants().size() < 2) {
			lError() << "Conference " << *getConferenceAddress()
			         << ": there are less than 2 participants in this chatroom, refusing creation.";
			declineSession(session, LinphoneReasonNotAcceptable);
			requestDeletion();
		} else if (!device || (device->getState() != ParticipantDevice::State::Joining)) {
			// We may end up here if a client successfully created a chat room but the conference server thinks it
			// should be allowed to be part of a conference. A scenario where this branch is hit is the following.
			// The client creating the chatroom registered into the server without the groupchat capability.
			// Nonetheless, the capability is added in the INVITE creating the chatroom. Upon reception of the 302
			// Moved Temporarely, the client will dial the chatroom URI directly and the server will look for
			// devices allowed to join the chatroom in method
			// ServerConference::subscribeRegistrationForParticipants(). Since "groupchat" capability was not there,
			// then the server doesn't allow to conclude the creation of the chatroom
			// -
			lError() << this
			         << ": Declining session because it looks like the device creating the chatroom is not allowed to "
			            "be part of this chatroom";
			declineSession(session, LinphoneReasonForbidden);
			requestDeletion();
		} else {
			/* Ok we are going to accept the session with 200Ok. However we want to wait for the ACK to be sure
			 * that the initiator is aware that he's now part of the conference, before we invite the others.
			 */
			acceptSession(session);
			if (!getCurrentParams()->isGroup() && (serverGroupChatRoom->getCachedParticipants().size() == 2)) {
				// Insert the one-to-one chat room in Db if participants count is 2.
				// This is necessary for protocol version < 1.1, and for backward compatibility in case these prior
				// versions are subsequently used by device that gets joined to the chatroom.
				getCore()->getPrivate()->mainDb->insertOneToOneConferenceChatRoom(
				    chatRoom, getCurrentParams()->getChatParams()->isEncrypted());
			}
		}
	}
#endif // HAVE_ADVANCED_IM
}

void ServerConference::setParticipantDevices(BCTBX_UNUSED(const std::shared_ptr<Address> &participantAddress),
                                             BCTBX_UNUSED(const list<shared_ptr<ParticipantDeviceIdentity>> &devices)) {
#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		updateParticipantDevices(participantAddress, devices);
		auto serverGroupChatRoom = dynamic_pointer_cast<ServerChatRoom>(chatRoom);
		if (serverGroupChatRoom->isJoiningPendingAfterCreation()) {
			if (serverGroupChatRoom->getUnnotifiedRegistrationSubscriptions() == 0) {
				conclude();
			}
		} else {
			updateParticipantsSessions();
		}
	}
#endif // HAVE_ADVANCED_IM
}

// -----------------------------------------------------------------------------

/*
 * This method is in charge of applying to the SIP session
 * the state of all participant devices belonging to a participant.
 */
void ServerConference::updateParticipantsSessions() {
	for (const auto &p : getParticipants()) {
		for (const auto &device : p->getDevices()) {
			updateParticipantDeviceSession(device);
		}
	}
}

void ServerConference::requestDeletion() {
#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		auto serverGroupChatRoom = dynamic_pointer_cast<ServerChatRoom>(chatRoom);
		/*
		 * Our ServerChatRoom is registered as listener to the numerous CallSession it manages,
		 * and is subscribing for registration events.
		 * It is time to remove this listener and unsubscribe, because we are going to disapear and we don't want
		 * to be notified anymore of anything.
		 */
		for (auto participant : getParticipants()) {
			serverGroupChatRoom->unSubscribeRegistrationForParticipant(participant->getAddress());
			for (auto devices : participant->getDevices()) {
				auto session = devices->getSession();
				if (session) session->removeListener(this);
			}
		}
		const auto &registrationSubscriptions = serverGroupChatRoom->getRegistrationSubscriptions();
		if (!registrationSubscriptions.empty()) {
			lError() << "Conference " << *getConferenceAddress() << " still " << registrationSubscriptions.size()
			         << " registration subscriptions pending while deletion is requested.";
		}
		chatRoom->deleteFromDb();
	}
#endif // HAVE_ADVANCED_IM
}

std::shared_ptr<Player> ServerConference::getPlayer() const {
	AudioControlInterface *intf = getAudioControlInterface();
	return intf ? intf->getPlayer() : nullptr;
}

bool ServerConference::sessionParamsAllowThumbnails() const {
	if (!mIsIn) {
		// The local participant is not in the conference therefore by default allow thumbnails
		return true;
	}
	auto session = static_pointer_cast<MediaSession>(mMe->getSession());
	return session->getMediaParams()->rtpBundleEnabled();
}

std::pair<bool, LinphoneMediaDirection> ServerConference::getMainStreamVideoDirection(
    const std::shared_ptr<CallSession> &session, bool localIsOfferer, bool useLocalParams) const {
	const auto ms = static_pointer_cast<MediaSession>(session);
	auto participantDevice = findParticipantDevice(session);
	auto deviceState = ParticipantDevice::State::ScheduledForJoining;
	if (participantDevice) {
		deviceState = participantDevice->getState();
	}
	auto isVideoConferenceEnabled = mConfParams->videoEnabled();
	auto callVideoEnabled =
	    (useLocalParams) ? ms->getMediaParams()->videoEnabled() : ms->getCurrentParams()->videoEnabled();
	const auto videoDirInParams = ms->getMediaParams()->getVideoDirection();
	bool enableVideoStream = false;
	LinphoneMediaDirection videoDir = LinphoneMediaDirectionInactive;
	if (!isVideoConferenceEnabled) {
		// Disable main video stream if:
		// - the conference doesn't support video capabilities
		// - a participant is trying to take over screen sharing
		enableVideoStream = false;
	} else if ((deviceState == ParticipantDevice::State::Joining) ||
	           (deviceState == ParticipantDevice::State::Present) ||
	           (deviceState == ParticipantDevice::State::OnHold)) {
		enableVideoStream = (deviceState == ParticipantDevice::State::Joining) ? true : callVideoEnabled;
		// Enable video based on conference capabilities if:
		// - joining conference
		// - receiving an offer
		videoDir = videoDirInParams;
	} else if (deviceState == ParticipantDevice::State::ScheduledForJoining) {
		enableVideoStream = true;
		videoDir = videoDirInParams;
	} else {
		videoDir = participantDevice->getStreamCapability(LinphoneStreamTypeVideo);
		enableVideoStream = (localIsOfferer) ? callVideoEnabled : isVideoConferenceEnabled;
	}
	return std::make_pair(enableVideoStream, videoDir);
}

void ServerConference::onGlobalStateChanged(BCTBX_UNUSED(LinphoneGlobalState state)) {
#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		auto serverGroupChatRoom = dynamic_pointer_cast<ServerChatRoom>(chatRoom);
		if (state == LinphoneGlobalOn) {
			// Try to subscribe again to cached participants when the core goes to GlobalOn state because the first
			// time around (when the chat room goes into state Created) the core might have still being
			// initializing. We must do be sure that the core is initialized because if one or more participants are
			// in another domain that the chatroom. In such a scenarion, in fact, the server might be sending out
			// SIP SUBSCRIBE and therefore the channel should not be destroyed by network changes that occur during
			// startup
			lInfo() << "The core has reached the GlobalOn state, therefore try to subscribe participants of chatroom "
			        << *getConferenceAddress();
			list<shared_ptr<const Address>> participantAddresses;
			const auto &cachedParticipants = serverGroupChatRoom->getCachedParticipants();
			for (const auto &participant : cachedParticipants) {
				participantAddresses.emplace_back(participant->getAddress());
			}
			// Subscribe to the registration events from the proxy
			serverGroupChatRoom->subscribeRegistrationForParticipants(participantAddresses, false);
		}
	}
#endif // HAVE_ADVANCED_IM
}

LinphoneMediaDirection
ServerConference::verifyVideoDirection(const std::shared_ptr<CallSession> &session,
                                       const LinphoneMediaDirection suggestedVideoDirection) const {
	const auto &device = findParticipantDevice(session);
	// By default do not change the direction
	auto videoDir = suggestedVideoDirection;
	if (device) {
		const auto &participant = device->getParticipant();
		if (participant) {
			const auto &role = participant->getRole();
			// From the conference server standpoint, a listener has only the send component of the video stream as the
			// server sends video streams to the client
			if (role == Participant::Role::Listener) {
				if (videoDir == LinphoneMediaDirectionSendRecv) {
					videoDir = LinphoneMediaDirectionSendOnly;
				} else if (videoDir == LinphoneMediaDirectionRecvOnly) {
					videoDir = LinphoneMediaDirectionInactive;
				}
			}
		}
	}
	return videoDir;
}

LINPHONE_END_NAMESPACE
