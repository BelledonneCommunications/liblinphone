/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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

#include "local_conference.h"
#include "call/call-log.h"
#include "conference/params/media-session-params-p.h"
#include "conference/participant-info.h"
#include "conference/participant.h"
#include "conference/session/media-session-p.h"
#include "conference/session/mixers.h"
#include "core/core-p.h"
#include "db/main-db.h"
#include "factory/factory.h"
#ifdef HAVE_ADVANCED_IM
#include "conference/handlers/local-audio-video-conference-event-handler.h"
#endif // HAVE_ADVANCED_IM

using namespace std;

LINPHONE_BEGIN_NAMESPACE

namespace MediaConference {

LocalConference::LocalConference(const shared_ptr<Core> &core,
                                 const std::shared_ptr<Address> &myAddress,
                                 CallSessionListener *listener,
                                 const std::shared_ptr<LinphonePrivate::ConferenceParams> params)
    : Conference(core, myAddress, listener, params) {

	bool_t eventLogEnabled = FALSE;
#ifdef HAVE_ADVANCED_IM
	eventLogEnabled = linphone_config_get_bool(linphone_core_get_config(getCore()->getCCore()), "misc",
	                                           "conference_event_log_enabled", TRUE);
	if (eventLogEnabled) {
		eventHandler = std::make_shared<LocalAudioVideoConferenceEventHandler>(this);
		addListener(eventHandler);
	} else {
#endif // HAVE_ADVANCED_IM
		lInfo() << "Unable to add listener to local conference as conference event package (RFC 4575) is disabled or "
		           "the SDK was not compiled with ENABLE_ADVANCED_IM flag set to on";
#ifdef HAVE_ADVANCED_IM
	}
#endif // HAVE_ADVANCED_IM

	if (!linphone_core_conference_server_enabled(core->getCCore())) {
		lWarning() << "Video capability in a conference is not supported when a device that is not a server is hosting "
		              "a conference.";
		confParams->enableVideo(false);
	}
	mMixerSession.reset(new MixerSession(*core.get()));
	mMixerSession->setSecurityLevel(confParams->getSecurityLevel());

	setState(ConferenceInterface::State::Instantiated);

	organizer = myAddress;

	// Update proxy contact address to add conference ID
	// Do not use myAddress directly as it may lack some parameter like gruu
	LinphoneAddress *cAddress = myAddress->toC();
	auto account = core->lookupKnownAccount(myAddress, true);
	char *contactAddressStr = nullptr;
	if (account && account->getOp()) {
		contactAddressStr = sal_address_as_string(account->getOp()->getContactAddress());
	} else {
		contactAddressStr =
		    ms_strdup(linphone_core_find_best_identity(core->getCCore(), const_cast<LinphoneAddress *>(cAddress)));
	}
	std::shared_ptr<Address> contactAddress = Address::create(contactAddressStr);
	char confId[LinphonePrivate::MediaConference::LocalConference::confIdLength];
	belle_sip_random_token(confId, sizeof(confId));
	contactAddress->setUriParam("conf-id", confId);
	if (contactAddressStr) {
		ms_free(contactAddressStr);
	}

	setConferenceAddress(contactAddress);
	me->setRole(Participant::Role::Speaker);
	me->setAdmin(true);
	me->setFocus(true);

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

LocalConference::LocalConference(const std::shared_ptr<Core> &core, SalCallOp *op)
    : Conference(core, Address::create(op->getTo()), nullptr, ConferenceParams::create(core->getCCore())) {
}

LocalConference::~LocalConference() {
	if ((state != ConferenceInterface::State::Terminated) && (state != ConferenceInterface::State::Deleted)) {
		terminate();
	}
#ifdef HAVE_ADVANCED_IM
	eventHandler.reset();
#endif // HAVE_ADVANCED_IM
	mMixerSession.reset();
}

void LocalConference::createEventHandler() {
#ifdef HAVE_ADVANCED_IM
	LinphoneCore *lc = getCore()->getCCore();
	bool_t eventLogEnabled =
	    linphone_config_get_bool(linphone_core_get_config(lc), "misc", "conference_event_log_enabled", TRUE);
	if (eventLogEnabled) {
		eventHandler = std::make_shared<LocalAudioVideoConferenceEventHandler>(this);
		addListener(eventHandler);
	} else {
#endif // HAVE_ADVANCED_IM
		lInfo() << "Unable to add listener to local conference as conference event package (RFC 4575) is disabled or "
		           "the SDK was not compiled with ENABLE_ADVANCED_IM flag set to on";
#ifdef HAVE_ADVANCED_IM
	}
#endif // HAVE_ADVANCED_IM
}

void LocalConference::initWithOp(SalCallOp *op) {
	mMixerSession.reset(new MixerSession(*getCore().get()));
	setState(ConferenceInterface::State::Instantiated);
	createEventHandler();
	configure(op);
}

void LocalConference::updateConferenceInformation(SalCallOp *op) {
	auto remoteContact = op->getRemoteContactAddress();
	if (remoteContact) {
		char *salAddress = sal_address_as_string(remoteContact);
		std::shared_ptr<Address> address = Address::create(std::string(salAddress));
		if (salAddress) {
			ms_free(salAddress);
		}
		auto invited =
		    std::find_if(mInvitedParticipants.begin(), mInvitedParticipants.end(), [&address](const auto &invitee) {
			    return address->weakEqual(*invitee->getAddress());
		    }) != mInvitedParticipants.end();

		std::shared_ptr<Address> remoteAddress =
		    Address::create((op->getDir() == SalOp::Dir::Incoming) ? op->getFrom() : op->getTo());

		if (findParticipantDevice(remoteAddress, address) || invited || address->weakEqual(*organizer)) {
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
			                        ? linphone_core_video_enabled(getCore()->getCCore())
			                        : false;
			if (!linphone_core_conference_server_enabled(getCore()->getCCore())) {
				lWarning() << "Video capability in a conference is not supported when a device that is not a server is "
				              "hosting a conference.";
			}

			bool previousVideoEnablement = confParams->videoEnabled();
			bool previousAudioEnablement = confParams->audioEnabled();

			confParams->enableAudio(audioEnabled);
			confParams->enableVideo(videoEnabled);

			if ((confParams->videoEnabled() != previousVideoEnablement) ||
			    (confParams->audioEnabled() != previousAudioEnablement)) {
				time_t creationTime = time(nullptr);
				notifyAvailableMediaChanged(creationTime, false, getMediaCapabilities());
			}
			setSubject(op->getSubject());

			confParams->enableOneParticipantConference(true);
			confParams->setStatic(true);

			auto session = const_pointer_cast<LinphonePrivate::MediaSession>(
			    static_pointer_cast<LinphonePrivate::MediaSession>(getMe()->getSession()));
			auto msp = session->getPrivate()->getParams();
			msp->enableAudio(audioEnabled);
			msp->enableVideo(videoEnabled);
			msp->getPrivate()->setInConference(true);

			if (times.size() > 0) {
				const auto [startTime, endTime] = times.front();
				confParams->setStartTime(startTime);
				confParams->setEndTime(endTime);
				msp->getPrivate()->setStartTime(startTime);
				msp->getPrivate()->setEndTime(endTime);
			}

			me->setRole(Participant::Role::Speaker);
			me->setAdmin(true);
			me->setFocus(true);

			const auto resourceList = op->getContentInRemote(ContentType::ResourceLists);
			bool isEmpty = !resourceList || resourceList.value().get().isEmpty();
			fillInvitedParticipantList(op, isEmpty);

			const auto &conferenceInfo = createConferenceInfoWithCustomParticipantList(organizer, mInvitedParticipants);
			auto infoState = ConferenceInfo::State::New;
			if (isEmpty) {
				infoState = ConferenceInfo::State::Cancelled;
			} else {
				infoState = ConferenceInfo::State::Updated;
			}
			conferenceInfo->setState(infoState);

			long long conferenceInfoId = -1;
#ifdef HAVE_DB_STORAGE
			auto &mainDb = getCore()->getPrivate()->mainDb;
			if (mainDb) {
				lInfo()
				    << "Inserting conference information to database in order to be able to recreate the conference "
				    << *getConferenceAddress() << " in case of restart";
				conferenceInfoId = mainDb->insertConferenceInfo(conferenceInfo);
			}
#endif
			auto callLog = session->getLog();
			if (callLog) {
				callLog->setConferenceInfo(conferenceInfo);
				callLog->setConferenceInfoId(conferenceInfoId);
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

void LocalConference::fillInvitedParticipantList(SalCallOp *op, bool cancelling) {
	mInvitedParticipants.clear();
	const auto &resourceList = op->getContentInRemote(ContentType::ResourceLists);
	if (resourceList && !resourceList.value().get().isEmpty()) {
		auto invitees = Utils::parseResourceLists(resourceList);
		mInvitedParticipants = invitees;
		if (!cancelling) {
			auto organizerNotFound =
			    std::find_if(mInvitedParticipants.begin(), mInvitedParticipants.end(), [this](const auto &invitee) {
				    return organizer->weakEqual(*invitee->getAddress());
			    }) == mInvitedParticipants.end();
			if (organizerNotFound && organizer) {
				Participant::Role role = Participant::Role::Speaker;
				lInfo() << "Setting role of organizer " << *organizer << " to " << role;
				auto organizerInfo = Factory::get()->createParticipantInfo(organizer);
				organizerInfo->setRole(role);
				mInvitedParticipants.push_back(organizerInfo);
			}
		}
	}
}
void LocalConference::configure(SalCallOp *op) {
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
	confParams->setSecurityLevel(securityLevel);

	mMixerSession->setSecurityLevel(confParams->getSecurityLevel());

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
		organizer = Address::create(op->getFrom());

		startTime = startTimeSdp;
		if (startTime <= 0) {
			startTime = ms_time(NULL);
		}
		endTime = endTimeSdp;
		if (endTime <= 0) {
			endTime = -1;
		}
		fillInvitedParticipantList(op, false);
	} else if (info) {
		subject = info->getSubject();
		organizer = info->getOrganizerAddress();

		startTime = info->getDateTime();
		const auto duration = info->getDuration();
		if ((duration > 0) && (startTime >= 0)) {
			endTime = startTime + static_cast<time_t>(duration) * 60;
		} else {
			endTime = -1;
		}
		mInvitedParticipants = info->getParticipants();
	}

	auto videoEnabled = linphone_core_video_enabled(lc);
	if (videoEnabled && !linphone_core_conference_server_enabled(lc)) {
		lWarning() << "Video capability in a conference is not supported when a device that is not a server is hosting "
		              "a conference.";
		videoEnabled = false;
	}
	confParams->enableAudio(audioEnabled);
	confParams->enableVideo(videoEnabled);

	if (!subject.empty()) {
		confParams->setSubject(subject);
	}
	confParams->enableLocalParticipant(false);
	confParams->enableOneParticipantConference(true);
	confParams->setStatic(true);

	confParams->setStartTime(startTime);
	confParams->setEndTime(endTime);

	if (!isUpdate && !info) {
		// Set joining mode only when creating a conference
		bool immediateStart = (startTimeSdp < 0);
		const auto joiningMode =
		    (immediateStart) ? ConferenceParams::JoiningMode::DialOut : ConferenceParams::JoiningMode::DialIn;
		confParams->setJoiningMode(joiningMode);
	}

	if (info || admin) {
		MediaSessionParams msp;
		msp.enableAudio(audioEnabled);
		msp.enableVideo(videoEnabled);
		msp.getPrivate()->setInConference(true);
		msp.getPrivate()->setStartTime(startTime);
		msp.getPrivate()->setEndTime(endTime);

		std::shared_ptr<Address> conferenceAddress;
		if (info) {
			conferenceAddress = info->getUri();
		} else if (admin) {
			conferenceAddress = Address::create(op->getTo());
		}
		shared_ptr<CallSession> session = getMe()->createSession(*this, &msp, true, nullptr);
		session->configure(LinphoneCallIncoming, nullptr, op, organizer, conferenceAddress);
	}

	me->setRole(Participant::Role::Speaker);
	me->setAdmin(true);
	me->setFocus(true);

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

std::list<std::shared_ptr<Address>> LocalConference::getAllowedAddresses() const {
	auto allowedAddresses = getInvitedAddresses();
	;
	auto organizerIt =
	    std::find_if(mInvitedParticipants.begin(), mInvitedParticipants.end(),
	                 [this](const auto &participant) { return participant->getAddress()->weakEqual(*organizer); });
	if (organizerIt == mInvitedParticipants.end()) {
		allowedAddresses.push_back(organizer);
	}
	return allowedAddresses;
}

void LocalConference::notifyStateChanged(LinphonePrivate::ConferenceInterface::State state) {
	// Call callbacks before calling listeners because listeners may change state
	linphone_core_notify_conference_state_changed(getCore()->getCCore(), toC(), (LinphoneConferenceState)getState());

	Conference::notifyStateChanged(state);
}

void LocalConference::confirmCreation() {

	if ((state != ConferenceInterface::State::Instantiated) && (state != ConferenceInterface::State::CreationPending)) {
		lError() << "Unable to confirm the creation of the conference in state " << state;
	}

	shared_ptr<MediaSession> session = dynamic_pointer_cast<MediaSession>(getMe()->getSession());

	if (session) {
		/* Assign a random conference address to this new conference, with domain
		 * set according to the proxy config used to receive the INVITE.
		 */
		auto account = session->getPrivate()->getDestAccount();
		if (!account) {
			const auto cAccount = linphone_core_get_default_account(getCore()->getCCore());
			if (cAccount) {
				account = Account::toCpp(cAccount)->getSharedFromThis();
			}
		}

		char confId[LinphonePrivate::MediaConference::LocalConference::confIdLength];
		if (account) {
			const auto accountParams = account->getAccountParams();
			std::shared_ptr<Address> conferenceAddress = accountParams->getIdentityAddress()->clone()->toSharedPtr();
			belle_sip_random_token(confId, sizeof(confId));
			conferenceAddress->setUriParam("conf-id", confId);
			setConferenceId(ConferenceId(conferenceAddress, conferenceAddress));
		}
		const_cast<LinphonePrivate::CallSessionParamsPrivate *>(L_GET_PRIVATE(session->getParams()))
		    ->setInConference(true);
		session->getPrivate()->setConferenceId(confId);

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
		long long conferenceInfoId = -1;
#ifdef HAVE_DB_STORAGE
		/// Method startIncomingNotification can move the conference to the CreationFailed state if the organizer
		/// doesn't have any of the codecs the server supports
		if (getState() != ConferenceInterface::State::CreationFailed) {
			// Store into DB after the start incoming notification in order to have a valid conference address being the
			// contact address of the call
			auto &mainDb = getCore()->getPrivate()->mainDb;
			if (mainDb) {
				const auto conferenceAddressStr = (getConferenceAddress() ? getConferenceAddress()->toString()
				                                                          : std::string("<address-not-defined>"));
				lInfo()
				    << "Inserting conference information to database in order to be able to recreate the conference "
				    << conferenceAddressStr << " in case of restart";
				conferenceInfoId = mainDb->insertConferenceInfo(conferenceInfo);
			}
		}
#endif
		auto callLog = session->getLog();
		if (callLog) {
			callLog->setConferenceInfo(conferenceInfo);
			callLog->setConferenceInfoId(conferenceInfoId);
		}

	} else {
		lError() << "Unable to confirm the creation of the conference because no session was created";
	}
}

std::shared_ptr<ConferenceInfo> LocalConference::createConferenceInfo() const {
	return createConferenceInfoWithCustomParticipantList(organizer, getFullParticipantList());
}

void LocalConference::finalizeCreation() {
	if (getState() == ConferenceInterface::State::CreationPending) {
		const std::shared_ptr<Address> &conferenceAddress = getConferenceAddress();
		setConferenceId(ConferenceId(conferenceAddress, conferenceAddress));
		shared_ptr<CallSession> session = me->getSession();
		if (session) {
			std::shared_ptr<ConferenceInfo> info = nullptr;
#ifdef HAVE_DB_STORAGE
			auto &mainDb = getCore()->getPrivate()->mainDb;
			if (mainDb) {
				info = getCore()->getPrivate()->mainDb->getConferenceInfoFromURI(getConferenceAddress());
			}
#endif // HAVE_DB_STORAGE
			const bool createdConference = (info && info->isValidUri());

			if (confParams->getJoiningMode() == ConferenceParams::JoiningMode::DialOut) {
				confParams->setStartTime(ms_time(NULL));
			}

			if (!createdConference) {
				auto addr = *conferenceAddress;
				addr.setParam("isfocus");
				if (session->getState() == CallSession::State::Idle) {
					lInfo() << " Scheduling redirection to [" << addr << "] for Call session [" << session << "]";
					getCore()->doLater([session, addr] { session->redirect(addr); });
				} else {
					session->redirect(addr);
				}
			}
		}
#ifdef HAVE_ADVANCED_IM
		if (eventHandler) {
			eventHandler->setConference(this);
		}
#endif // HAVE_ADVANCED_IM
	}
}

void LocalConference::subscribeReceived(shared_ptr<EventSubscribe> event) {
#ifdef HAVE_ADVANCED_IM
	if (eventHandler) {
		const auto ret = eventHandler->subscribeReceived(event);
		if (ret == 0) {
			// A client joins when the conference receives the SUBSCRIBE. This allows to ensure that no NOTIFY is missed
			// and we don't have to necessarely wait for the client reINVITE or ICE reINVITE to start sending NOTIFYs
			// regarding the conference to him/her
			const auto &participantAddress = event->getFrom();
			auto participant = findParticipant(participantAddress);
			if (participant) {
				const auto &contactAddr = event->getRemoteContact();
				auto device = participant->findDevice(contactAddr);
				if (device) {
					participantDeviceJoined(participant, device);
				}
			}
		}
		return;
	} else {
#endif // HAVE_ADVANCED_IM
		lInfo() << "Unable to accept SUBSCRIBE because conference event package (RFC 4575) is disabled or the SDK was "
		           "not compiled with ENABLE_ADVANCED_IM flag set to on";
#ifdef HAVE_ADVANCED_IM
	}
#endif // HAVE_ADVANCED_IM
	event->deny(LinphoneReasonNotAcceptable);
}

void LocalConference::setParticipantAdminStatus(const shared_ptr<Participant> &participant, bool isAdmin) {
	if (isAdmin != participant->isAdmin()) {
		participant->setAdmin(isAdmin);
		time_t creationTime = time(nullptr);
		notifyParticipantSetAdmin(creationTime, false, participant, isAdmin);
	}
}

void LocalConference::onConferenceTerminated(const std::shared_ptr<Address> &addr) {
#ifdef HAVE_ADVANCED_IM
	if (eventHandler) {
		eventHandler->setConference(nullptr);
	}
#endif // HAVE_ADVANCED_IM
	Conference::onConferenceTerminated(addr);
}

void LocalConference::addLocalEndpoint() {
	if (confParams->localParticipantEnabled()) {
		StreamMixer *mixer = mMixerSession->getMixerByType(SalAudio);
		if (mixer) {
			mixer->enableLocalParticipant(true);
			// Get ssrc of me because it must be sent to participants through NOTIFY
			auto audioMixer = dynamic_cast<MS2AudioMixer *>(mixer);
			auto audioStream = audioMixer->getAudioStream();
			auto meSsrc = audio_stream_get_send_ssrc(audioStream);
			for (auto &device : me->getDevices()) {
				device->setSsrc(LinphoneStreamTypeAudio, meSsrc);
			}
		}

		if (confParams->videoEnabled()) {
			mixer = mMixerSession->getMixerByType(SalVideo);
			if (mixer) {
				mixer->enableLocalParticipant(true);
#ifdef VIDEO_ENABLED
				auto videoMixer = dynamic_cast<MS2VideoMixer *>(mixer);
				auto videoStream = videoMixer->getVideoStream();
				auto meSsrc = media_stream_get_send_ssrc(&videoStream->ms);
				for (auto &device : me->getDevices()) {
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
			for (auto &device : me->getDevices()) {
				notifyParticipantDeviceAdded(creationTime, false, getMe(), device);
			}
		}
	}
}

int LocalConference::inviteAddresses(const list<std::shared_ptr<Address>> &addresses,
                                     const LinphoneCallParams *params) {

	const auto &coreCurrentCall = getCore()->getCurrentCall();
	const bool startingConference = (getState() == ConferenceInterface::State::CreationPending);

	const auto &outputDevice = (coreCurrentCall) ? coreCurrentCall->getOutputAudioDevice() : nullptr;
	const auto &inputDevice = (coreCurrentCall) ? coreCurrentCall->getInputAudioDevice() : nullptr;

	auto lc = getCore()->getCCore();

	for (const auto &address : addresses) {
		std::shared_ptr<Call> call = nullptr;

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
			auto participant = findParticipant(address);
			if (participant) {
				const auto &devices = participant->getDevices();
				if (!devices.empty()) {
					const auto &device = devices.front();
					if (!device->getCallId().empty()) {
						call = getCore()->getCallByCallId(device->getCallId());
					} else if (device->getSession()) {
						const auto &session = device->getSession();
						const auto &calls = getCore()->getCalls();
						auto it = std::find_if(calls.cbegin(), calls.cend(), [&session](const auto &c) {
							return (c->getActiveSession() == session);
						});
						if (it != calls.cend()) {
							call = (*it);
						}
					}
				}
			}
		} else {
			call = getCore()->getCallByRemoteAddress(address);
		}
		if (!call) {
			/* Start a new call by indicating that it has to be put into the conference directly */
			LinphoneCallParams *new_params;
			if (params) {
				new_params = _linphone_call_params_copy(params);
			} else {
				new_params = linphone_core_create_call_params(lc, nullptr);
				linphone_call_params_enable_video(new_params, confParams->videoEnabled());
			}

			linphone_call_params_set_in_conference(new_params, TRUE);
			linphone_call_params_set_start_time(new_params, confParams->getStartTime());

			const std::shared_ptr<Address> &conferenceAddress = getConferenceAddress();
			const string &confId = conferenceAddress->getUriParamValue("conf-id");
			linphone_call_params_set_conference_id(new_params, confId.c_str());

			call = Call::toCpp(linphone_core_invite_address_with_params_2(
			                       lc, address->toC(), new_params, L_STRING_TO_C(confParams->getSubject()), NULL))
			           ->getSharedFromThis();

			if (!confParams->getAccount()) {
				// Set proxy configuration used for the conference
				auto callAccount = call->getDestAccount();
				if (callAccount) {
					confParams->setAccount(callAccount);
				} else {
					auto account = getCore()->lookupKnownAccount(address, true);
					confParams->setAccount(account);
				}
			}

			tryAddMeDevice();

			if (!call) {
				lError() << "LocalConference::inviteAddresses(): could not invite participant";
			} else {
				addParticipant(call);
				auto participant = findParticipant(address);
				participant->setPreserveSession(false);
			}
			linphone_call_params_unref(new_params);
		} else {
			/* There is already a call to this address, so simply join it to the local conference if not already done */
			if (!call->getCurrentParams()->getPrivate()->getInConference()) {
				addParticipant(call);
				auto participant = findParticipant(address);
				participant->setPreserveSession(true);
			}
		}
		/* If the local participant is not yet created, created it and it to the conference */
		addLocalEndpoint();
		call->setConference(getSharedFromThis());
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

void LocalConference::enableScreenSharing(const std::shared_ptr<LinphonePrivate::CallSession> &session, bool notify) {
	if (confParams->videoEnabled()) {
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

int LocalConference::participantDeviceAlerting(const std::shared_ptr<LinphonePrivate::CallSession> &session) {
	const std::shared_ptr<Address> &remoteAddress = session->getRemoteAddress();
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(remoteAddress);
	if (p) {
		std::shared_ptr<ParticipantDevice> device = p->findDevice(session);
		if (device) {
			return participantDeviceAlerting(p, device);
		} else {
			lDebug() << "Participant alerting: Unable to find device with session " << session
			         << " among devices of participant " << p->getAddress()->toString() << " of conference "
			         << *getConferenceAddress();
		}
	}
	return -1;
}

int LocalConference::participantDeviceAlerting(
    BCTBX_UNUSED(const std::shared_ptr<LinphonePrivate::Participant> &participant),
    const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) {
	lInfo() << "Device " << *device->getAddress() << " changed state to alerting";
	device->updateMediaCapabilities();
	device->updateStreamAvailabilities();
	device->setState(ParticipantDevice::State::Alerting);
	return 0;
}

int LocalConference::participantDeviceJoined(const std::shared_ptr<LinphonePrivate::CallSession> &session) {
	const std::shared_ptr<Address> &remoteAddress = session->getRemoteAddress();
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(remoteAddress);
	if (p) {
		std::shared_ptr<ParticipantDevice> device = p->findDevice(session);
		if (device) {
			return participantDeviceJoined(p, device);
		} else {
			lDebug() << "Participant joined: Unable to find device with session " << session
			         << " among devices of participant " << p->getAddress()->toString() << " of conference "
			         << *getConferenceAddress();
		}
	}
	return -1;
}

int LocalConference::participantDeviceJoined(
    BCTBX_UNUSED(const std::shared_ptr<LinphonePrivate::Participant> &participant),
    const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) {
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

int LocalConference::participantDeviceLeft(const std::shared_ptr<LinphonePrivate::CallSession> &session) {
	const std::shared_ptr<Address> &remoteAddress = session->getRemoteAddress();
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(remoteAddress);
	if (p) {
		std::shared_ptr<ParticipantDevice> device = p->findDevice(session);
		if (device) {
			return participantDeviceLeft(p, device);
		} else {
			lWarning() << "Participant left: Unable to find device with session " << session
			           << " among devices of participant " << p->getAddress()->toString() << " of conference "
			           << *getConferenceAddress();
		}
	}
	return -1;
}

int LocalConference::participantDeviceLeft(
    BCTBX_UNUSED(const std::shared_ptr<LinphonePrivate::Participant> &participant),
    const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) {
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
int LocalConference::participantDeviceMediaCapabilityChanged(
    const std::shared_ptr<LinphonePrivate::CallSession> &session) {
	const std::shared_ptr<Address> &remoteAddress = session->getRemoteAddress();
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(remoteAddress);
	int success = -1;
	if (p) {
		std::shared_ptr<ParticipantDevice> device = p->findDevice(session);
		if (device) {
			success = participantDeviceMediaCapabilityChanged(p, device);
		} else {
			lWarning() << "Participant media capability changed: Unable to find device with session " << session
			           << " among devices of participant " << p->getAddress()->toString() << " of conference "
			           << *getConferenceAddress();
		}
	}
	return success;
}

int LocalConference::participantDeviceMediaCapabilityChanged(const std::shared_ptr<Address> &addr) {
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(addr);
	int success = -1;
	for (const auto &d : p->getDevices()) {
		success = participantDeviceMediaCapabilityChanged(p, d);
	}
	return success;
}

int LocalConference::participantDeviceMediaCapabilityChanged(
    const std::shared_ptr<LinphonePrivate::Participant> &participant,
    const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) {
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

int LocalConference::participantDeviceSsrcChanged(const std::shared_ptr<LinphonePrivate::CallSession> &session,
                                                  const LinphoneStreamType type,
                                                  uint32_t ssrc) {
	const std::shared_ptr<Address> &remoteAddress = session->getRemoteAddress();
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(remoteAddress);
	int success = -1;
	if (p) {
		std::shared_ptr<ParticipantDevice> device = p->findDevice(session);
		if (device) {
			bool updated = device->setSsrc(type, ssrc);
			if (updated) {
				lInfo() << "Setting " << std::string(linphone_stream_type_to_string(type))
				        << " ssrc of participant device " << device->getAddress()->toString() << " in conference "
				        << *getConferenceAddress() << " to " << ssrc;
				time_t creationTime = time(nullptr);
				notifyParticipantDeviceMediaCapabilityChanged(creationTime, false, p, device);
			} else {
				lInfo() << "Leaving unchanged ssrc of participant device " << device->getAddress()->toString()
				        << " in conference " << *getConferenceAddress() << " whose value is " << ssrc;
			}
			return 0;
		}
	}
	lInfo() << "Unable to set " << std::string(linphone_stream_type_to_string(type)) << " ssrc to " << ssrc
	        << " because participant device with session " << session << " cannot be found in conference "
	        << *getConferenceAddress();
	return success;
}

int LocalConference::participantDeviceSsrcChanged(const std::shared_ptr<LinphonePrivate::CallSession> &session,
                                                  uint32_t audioSsrc,
                                                  uint32_t videoSsrc) {
	const std::shared_ptr<Address> &remoteAddress = session->getRemoteAddress();
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(remoteAddress);
	int success = -1;
	if (p) {
		std::shared_ptr<ParticipantDevice> device = p->findDevice(session);
		if (device) {
			if (device->setSsrc(LinphoneStreamTypeAudio, audioSsrc) ||
			    device->setSsrc(LinphoneStreamTypeVideo, videoSsrc)) {
				time_t creationTime = time(nullptr);
				notifyParticipantDeviceMediaCapabilityChanged(creationTime, false, p, device);
			} else {
				lInfo() << "Leaving unchanged ssrcs of participant device " << device->getAddress()->toString()
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

int LocalConference::getParticipantDeviceVolume(const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) {
	MS2AudioMixer *mixer = dynamic_cast<MS2AudioMixer *>(mMixerSession->getMixerByType(SalAudio));
	if (mixer) {
		MSAudioConference *conf = mixer->getAudioConference();
		return ms_audio_conference_get_participant_volume(conf, device->getSsrc(LinphoneStreamTypeAudio));
	}

	return AUDIOSTREAMVOLUMES_NOT_FOUND;
}

bool LocalConference::dialOutAddresses(const std::list<std::shared_ptr<Address>> &addressList) {
	auto new_params = linphone_core_create_call_params(getCore()->getCCore(), nullptr);
	linphone_call_params_enable_video(new_params, confParams->videoEnabled());

	linphone_call_params_set_in_conference(new_params, TRUE);

	const std::shared_ptr<Address> &conferenceAddress = getConferenceAddress();
	const string &confId = conferenceAddress->getUriParamValue("conf-id");
	linphone_call_params_set_conference_id(new_params, confId.c_str());

	std::list<std::shared_ptr<Address>> addresses = getInvitedAddresses();
	// Add participants already in the conference to the list of addresses if they are not part of the invitees
	for (const auto &p : getParticipants()) {
		const auto &pAddress = p->getAddress();
		auto pIt = std::find_if(addresses.begin(), addresses.end(),
		                        [&pAddress](const auto &address) { return (pAddress->weakEqual(*address)); });
		if (pIt == addresses.end()) {
			addresses.push_back(pAddress);
		}
	}

	auto resourceList = Content::create();
	resourceList->setBodyFromUtf8(Utils::getResourceLists(addresses));
	resourceList->setContentType(ContentType::ResourceLists);
	resourceList->setContentDisposition(ContentDisposition::RecipientList);
	if (linphone_core_content_encoding_supported(getCore()->getCCore(), "deflate")) {
		resourceList->setContentEncoding("deflate");
	}
	if (!resourceList->isEmpty()) {
		L_GET_CPP_PTR_FROM_C_OBJECT(new_params)->addCustomContent(resourceList);
	}

	auto sipfrag = Content::create();
	const auto organizerUri = organizer->getUri();
	sipfrag->setBodyFromLocale("From: <" + organizerUri.toString() + ">");
	sipfrag->setContentType(ContentType::SipFrag);
	L_GET_CPP_PTR_FROM_C_OBJECT(new_params)->addCustomContent(sipfrag);
	auto success = (inviteAddresses(addressList, new_params) == 0);
	linphone_call_params_unref(new_params);
	return success;
}

bool LocalConference::addParticipants(const std::list<std::shared_ptr<Call>> &calls) {
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

bool LocalConference::addParticipants(const std::list<std::shared_ptr<Address>> &addresses) {
	return Conference::addParticipants(addresses);
}

bool LocalConference::addParticipantDevice(std::shared_ptr<LinphonePrivate::Call> call) {
	bool success = Conference::addParticipantDevice(call);
	if (success) {
		call->setConference(getSharedFromThis());
		auto session = call->getActiveSession();
		auto device = findParticipantDevice(session);
		if (device) {
			device->setJoiningMethod((call->getDirection() == LinphoneCallIncoming)
			                             ? ParticipantDevice::JoiningMethod::DialedIn
			                             : ParticipantDevice::JoiningMethod::DialedOut);
			char label[LinphonePrivate::Conference::labelLength];
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

bool LocalConference::tryAddMeDevice() {
	if (confParams->localParticipantEnabled() && me->getDevices().empty() && confParams->getAccount()) {
		const auto &contactAddress = confParams->getAccount()->getContactAddress();
		if (contactAddress) {
			std::shared_ptr<Address> devAddr = contactAddress->clone()->toSharedPtr();
			auto meDev = me->addDevice(devAddr);
			const auto &meSession = me->getSession();

			char label[Conference::labelLength];
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
			    (confParams->audioEnabled() ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive),
			    LinphoneStreamTypeAudio);
			meDev->setStreamCapability(
			    (confParams->videoEnabled() ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive),
			    LinphoneStreamTypeVideo);
			meDev->setStreamCapability(
			    (confParams->chatEnabled() ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive),
			    LinphoneStreamTypeText);

			meDev->updateStreamAvailabilities();

			return true;
		}
	}
	return false;
}

bool LocalConference::addParticipant(std::shared_ptr<LinphonePrivate::Call> call) {

	const auto &remoteAddress = call->getRemoteAddress();
	if (linphone_call_params_get_in_conference(linphone_call_get_current_params(call->toC()))) {
		lError() << "Call (local address " << call->getLocalAddress()->toString() << " remote address "
		         << (remoteAddress ? remoteAddress->toString() : "Unknown") << ") is already in conference "
		         << *getConferenceAddress();
		return false;
	}

	if (confParams->getParticipantListType() == ConferenceParams::ParticipantListType::Closed) {
		const auto allowedAddresses = getAllowedAddresses();
		auto p = std::find_if(allowedAddresses.begin(), allowedAddresses.end(),
		                      [&remoteAddress](const auto &address) { return (remoteAddress->weakEqual(*address)); });
		if (p == allowedAddresses.end()) {
			lError() << "Unable to add call (local address " << call->getLocalAddress()->toString()
			         << " remote address " << (remoteAddress ? remoteAddress->toString() : "Unknown")
			         << ") because participant " << *remoteAddress
			         << " is not in the list of allowed participants of conference " << *getConferenceAddress();
			LinphoneErrorInfo *ei = linphone_error_info_new();
			linphone_error_info_set(ei, NULL, LinphoneReasonUnknown, 403, "Call forbidden to join the conference",
			                        NULL);
			call->terminate(ei);
			linphone_error_info_unref(ei);
			return false;
		}
	}

	const auto initialState = getState();
	const auto dialout = (confParams->getJoiningMode() == ConferenceParams::JoiningMode::DialOut);
	// If conference must start immediately, then the organizer will call the conference server and the other
	// participants will be dialed out
	if ((initialState == ConferenceInterface::State::CreationPending) && dialout &&
	    !remoteAddress->weakEqual(*organizer)) {
		lError() << "The conference must immediately start (start time: " << confParams->getStartTime()
		         << " end time: " << confParams->getEndTime() << "). Unable to add participant "
		         << remoteAddress->toString()
		         << " because participants will be dialed out by the conference server as soon as " << organizer
		         << " dials in";
		return false;
	}

#if 0
	if (!isConferenceStarted()) {
		lError() << "Unable to add call (local address " << call->getLocalAddress()->toString() << " remote address " <<  (remoteAddress ? remoteAddress->toString() : "Unknown") << ") because participant " << *remoteAddress << " is not in the list of allowed participants of conference " << *getConferenceAddress();
		LinphoneErrorInfo *ei = linphone_error_info_new();
		linphone_error_info_set(ei, NULL, LinphoneReasonUnknown, 403, "Conference not started yet", NULL);
		call->terminate(ei);
		linphone_error_info_unref(ei);
		return false;
	}

	if (isConferenceEnded()) {
		lError() << "Unable to add call (local address " << call->getLocalAddress()->toString() << " remote address " <<  (remoteAddress ? remoteAddress->toString() : "Unknown") << ") because participant " << *remoteAddress << " is not in the list of allowed participants of conference " << *getConferenceAddress();
		LinphoneErrorInfo *ei = linphone_error_info_new();
		linphone_error_info_set(ei, NULL, LinphoneReasonUnknown, 403, "Conference already terminated", NULL);
		call->terminate(ei);
		linphone_error_info_unref(ei);
		return false;
	}
#endif

	const std::shared_ptr<Address> &conferenceAddress = getConferenceAddress();
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

		if (!confParams->getAccount()) {
			// Set proxy configuration used for the conference
			auto callAccount = call->getDestAccount();
			if (callAccount) {
				confParams->setAccount(callAccount);
			} else {
				auto account = getCore()->lookupKnownAccount(call->getToAddress(), true);
				confParams->setAccount(account);
			}
		}

		// Get contact address here because it may be modified by a change in the local parameters. As the participant
		// enters the conference, in fact attributes conf-id and isfocus are added later on (based on local parameters)
		// therefore there is no way to know if the remote client already knew that the call was in a conference or not.
		auto contactAddress = session->getContactAddress();
		tryAddMeDevice();

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
				const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(L_GET_PRIVATE(call->getParams()))
				    ->setInConference(true);
				const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(L_GET_PRIVATE(call->getParams()))
				    ->setConferenceId(confId);
				const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(L_GET_PRIVATE(call->getParams()))
				    ->setStartTime(confParams->getStartTime());
				const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(L_GET_PRIVATE(call->getParams()))
				    ->setEndTime(confParams->getEndTime());
				if (getCurrentParams().videoEnabled()) {
					if (getCurrentParams().localParticipantEnabled()) {
						const_cast<LinphonePrivate::MediaSessionParams *>(call->getParams())->enableVideo(true);
					} else {
						if (call->getRemoteParams()) {
							const_cast<LinphonePrivate::MediaSessionParams *>(call->getParams())
							    ->enableVideo(call->getRemoteParams()->videoEnabled());
						}
					}
				} else {
					const_cast<LinphonePrivate::MediaSessionParams *>(call->getParams())->enableVideo(false);
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

				const_cast<LinphonePrivate::MediaSessionParams *>(call->getParams())->setAudioDirection(audioDirection);
				const_cast<LinphonePrivate::MediaSessionParams *>(call->getParams())->setVideoDirection(videoDirection);
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
			case LinphoneCallResuming: {
				if (state == LinphoneCallStreamsRunning) {
					// Calling enter here because update will lock sound resources
					enter();
				}
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

		// If no resource list is provided in the INVITE, there is no need to call participants
		if ((initialState == ConferenceInterface::State::CreationPending) && dialout && !isEmpty) {
			list<std::shared_ptr<Address>> addresses;
			for (auto &participant : mInvitedParticipants) {
				const auto &addr = participant->getAddress();
				// Do not invite organizer as it is already dialing in
				if (*addr != *organizer) {
					addresses.push_back(addr);
				}
			}
			dialOutAddresses(addresses);
		}

		return true;
	}

	lError() << "Unable to add call (local address " << call->getLocalAddress()->toString() << " remote address "
	         << (remoteAddress ? remoteAddress->toString() : "Unknown") << ") to conference "
	         << *getConferenceAddress();
	return false;
}

bool LocalConference::addParticipant(const std::shared_ptr<Address> &participantAddress) {
	auto participantInfo = Factory::get()->createParticipantInfo(participantAddress);
	// Participants invited after the start of a conference through the address can only listen to it
	participantInfo->setRole(Participant::Role::Listener);
	return addParticipant(participantInfo);
}

bool LocalConference::addParticipant(const std::shared_ptr<ParticipantInfo> &info) {
#if 0
	if (!isConferenceEnded() && isConferenceStarted()) {
#endif
	const auto initialState = getState();
	if ((initialState == ConferenceInterface::State::CreationPending) ||
	    (initialState == ConferenceInterface::State::Created)) {

		const auto allowedAddresses = getAllowedAddresses();
		const auto &participantAddress = info->getAddress();
		auto p =
		    std::find_if(allowedAddresses.begin(), allowedAddresses.end(), [&participantAddress](const auto &address) {
			    return (participantAddress->weakEqual(*address));
		    });
		if (p == allowedAddresses.end()) {
			auto participantInfo = info->clone()->toSharedPtr();
			participantInfo->setSequenceNumber(-1);
			mInvitedParticipants.push_back(participantInfo);
		}

		std::list<std::shared_ptr<Address>> addressesList{participantAddress};
		return dialOutAddresses(addressesList);
	}
#if 0
	} else {
		const auto & endTime = confParams->getEndTime();
		const auto & startTime = confParams->getStartTime();
		const auto now = time(NULL);
		lError() << "Could not add participant " << *participantAddress << " to the conference because the conference " << *getConferenceAddress() << " is not active right now.";
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
#endif
	return false;
}

void LocalConference::setLocalParticipantStreamCapability(const LinphoneMediaDirection &direction,
                                                          const LinphoneStreamType type) {
	if (confParams->localParticipantEnabled() && !me->getDevices().empty() && confParams->getAccount() &&
	    (type != LinphoneStreamTypeUnknown)) {
		const auto &contactAddress = confParams->getAccount()->getContactAddress();
		if (contactAddress) {
			std::shared_ptr<Address> devAddr = contactAddress->clone()->toSharedPtr();
			const auto &meDev = me->findDevice(devAddr);
			if (meDev) {
				lInfo() << "Setting direction of stream of type " << std::string(linphone_stream_type_to_string(type))
				        << " to " << std::string(linphone_media_direction_to_string(direction)) << " of device "
				        << meDev->getAddress()->toString();
				const auto mediaChanged = meDev->setStreamCapability(direction, type);
				meDev->updateStreamAvailabilities();
				for (const auto &p : getParticipants()) {
					for (const auto &d : p->getDevices()) {
						d->updateStreamAvailabilities();
					}
				}

				if (mediaChanged) {
					time_t creationTime = time(nullptr);
					notifyParticipantDeviceMediaCapabilityChanged(creationTime, false, me, meDev);
				}
			} else {
				lError() << "Unable to find device with address " << devAddr->toString()
				         << " among those in the local participant " << me->getAddress()->toString();
			}
		}
	}
}

bool LocalConference::finalizeParticipantAddition(std::shared_ptr<LinphonePrivate::Call> call) {
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
					linphone_call_params_set_start_time(params, confParams->getStartTime());
					linphone_call_params_set_end_time(params, confParams->getEndTime());
					if (getCurrentParams().videoEnabled()) {
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

int LocalConference::removeParticipant(const std::shared_ptr<LinphonePrivate::CallSession> &session,
                                       const bool preserveSession) {
	int err = 0;

	auto op = session->getPrivate()->getOp();
	shared_ptr<Call> call = getCore()->getCallByCallId(op->getCallId());
	if (call) {
		if (linphone_call_get_conference(call->toC()) != toC()) {
			const auto &remoteAddress = call->getRemoteAddress();
			lError() << "Call (local address " << call->getLocalAddress()->toString() << " remote address "
			         << (remoteAddress ? remoteAddress->toString() : "Unknown") << ") is not part of conference "
			         << *getConferenceAddress();
			return -1;
		}
	}

	CallSession::State sessionState = session->getState();

	const std::shared_ptr<Address> &remoteAddress = session->getRemoteAddress();
	std::shared_ptr<LinphonePrivate::Participant> participant = findParticipant(remoteAddress);
	if (participant) {
		Conference::removeParticipant(session, preserveSession);
		mMixerSession->unjoinStreamsGroup(
		    static_pointer_cast<LinphonePrivate::MediaSession>(session)->getStreamsGroup());
	} else {
		if ((sessionState != LinphonePrivate::CallSession::State::Released) &&
		    (sessionState != LinphonePrivate::CallSession::State::End)) {
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
				const MediaSessionParams *params =
				    static_pointer_cast<LinphonePrivate::MediaSession>(session)->getMediaParams();
				MediaSessionParams *currentParams = params->clone();
				currentParams->getPrivate()->setInConference(FALSE);
				currentParams->getPrivate()->setConferenceId("");
				err = static_pointer_cast<LinphonePrivate::MediaSession>(session)->updateFromConference(currentParams);
				delete currentParams;
			} else if ((sessionState != CallSession::State::End) && (sessionState != CallSession::State::Released)) {
				lInfo() << "Pause call to notify of conference removal.";
				/* Kick the session out of the conference by moving to the Paused state. */
				const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(
				    L_GET_PRIVATE(static_pointer_cast<LinphonePrivate::MediaSession>(session)->getMediaParams()))
				    ->setInConference(false);
				const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(
				    L_GET_PRIVATE(static_pointer_cast<LinphonePrivate::MediaSession>(session)->getMediaParams()))
				    ->setConferenceId("");
				err = static_pointer_cast<LinphonePrivate::MediaSession>(session)->pauseFromConference();
			}
		} else {
			// Terminate session (i.e. send a BYE) as per RFC
			// This is the default behaviour
			if (sessionState != LinphonePrivate::CallSession::State::End) {
				err = static_pointer_cast<LinphonePrivate::MediaSession>(session)->terminate();
			}
		}

		/*
		 * Handle the case where only the local participant and a unique remote participant are remaining.
		 * In this case, if the session linked to the participant has to be preserved after the conference, then destroy
		 * the conference and let these two participants to connect directly thanks to a simple call. Indeed, the
		 * conference adds latency and processing that is useless to do for 1-1 conversation.
		 */
		if (!confParams->oneParticipantConferenceEnabled() && (getParticipantCount() == 1) && (!preserveSession)) {
			std::shared_ptr<LinphonePrivate::Participant> remainingParticipant = participants.front();
			const bool lastParticipantPreserveSession = remainingParticipant->getPreserveSession();
			auto &devices = remainingParticipant->getDevices();
			if (lastParticipantPreserveSession && (devices.size() == 1)) {

				std::shared_ptr<LinphonePrivate::MediaSession> lastSession =
				    static_pointer_cast<LinphonePrivate::MediaSession>(devices.front()->getSession());

				if (lastSession) {
					lInfo() << "Participant [" << remainingParticipant << "] with "
					        << lastSession->getRemoteAddress()->toString() << " is the last call in conference "
					        << *getConferenceAddress() << ", we will reconnect directly to it.";

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
						const_cast<LinphonePrivate::MediaSessionParamsPrivate *>(L_GET_PRIVATE(params))
						    ->setInConference(false);

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
	if (sessionState != LinphonePrivate::CallSession::State::PausedByRemote) {
		checkIfTerminated();
	}

	return err ? 0 : -1;
}

int LocalConference::removeParticipant(const std::shared_ptr<Address> &addr) {
	const std::shared_ptr<LinphonePrivate::Participant> participant = findParticipant(addr);
	if (!participant) return -1;
	return removeParticipant(participant) ? 0 : -1;
}

bool LocalConference::removeParticipant(const std::shared_ptr<LinphonePrivate::Participant> &participant) {
	const auto devices = participant->getDevices();
	bool success = true;
	if (devices.size() > 0) {
		for (const auto &d : devices) {
			success &= (removeParticipant(d->getSession(), false) == 0);
		}
	} else {
		lInfo() << "Remove participant with address " << *participant->getAddress() << " from conference "
		        << *getConferenceAddress();
		participants.remove(participant);
		time_t creationTime = time(nullptr);
		notifyParticipantRemoved(creationTime, false, participant);
		success = true;
	}
	return success;
}

void LocalConference::checkIfTerminated() {
	if (!confParams->isStatic() && (getParticipantCount() == 0)) {
		leave();
		if (getState() == ConferenceInterface::State::TerminationPending) {
			setState(ConferenceInterface::State::Terminated);
		} else {
			setState(ConferenceInterface::State::TerminationPending);
#ifdef HAVE_ADVANCED_IM
			bool_t eventLogEnabled = linphone_config_get_bool(linphone_core_get_config(getCore()->getCCore()), "misc",
			                                                  "conference_event_log_enabled", TRUE);
			if (!eventLogEnabled || !eventHandler) {
#endif // HAVE_ADVANCED_IM
				setState(ConferenceInterface::State::Terminated);
#ifdef HAVE_ADVANCED_IM
			}
#endif // HAVE_ADVANCED_IM
		}
	}
}

void LocalConference::chooseAnotherAdminIfNoneInConference() {
	if (participants.empty() == false) {
		const auto adminParticipant = std::find_if(participants.cbegin(), participants.cend(),
		                                           [&](const auto &p) { return (p->isAdmin() == true); });
		// If not admin participant is found
		if (adminParticipant == participants.cend()) {
			setParticipantAdminStatus(participants.front(), true);
			lInfo() << this << ": New admin designated is " << *(participants.front());
		}
	}
}

/* ConferenceInterface */
void LocalConference::setSubject(const std::string &subject) {
	if (subject.compare(getUtf8Subject()) != 0) {
		Conference::setSubject(subject);
		time_t creationTime = time(nullptr);
		notifySubjectChanged(creationTime, false, subject);
	}
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void LocalConference::subscriptionStateChanged(shared_ptr<EventSubscribe> event, LinphoneSubscriptionState state) {
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

int LocalConference::terminate() {
	const auto conferenceAddressStr =
	    (getConferenceAddress() ? getConferenceAddress()->toString() : std::string("<address-not-defined>"));
	lInfo() << "Terminate conference " << conferenceAddressStr;
	// Take a ref because the conference may be immediately go to deleted state if terminate is called when there are 0
	// participants
	const auto ref = getSharedFromThis();
	setState(ConferenceInterface::State::TerminationPending);

	size_t noDevices = 0;
	auto participantIt = participants.begin();
	while (participantIt != participants.end()) {
		auto participant = *participantIt;
		const auto devices = participant->getDevices();
		noDevices += devices.size();
		participantIt++;
		if (devices.size() > 0) {
			for (const auto &d : devices) {
				std::shared_ptr<LinphonePrivate::MediaSession> session =
				    static_pointer_cast<LinphonePrivate::MediaSession>(d->getSession());
				if (session) {
					lInfo() << "Terminating session of participant device " << d->getAddress();
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

	return 0;
}

int LocalConference::enter() {

	if (confParams->localParticipantEnabled()) {
		if (linphone_core_sound_resources_locked(getCore()->getCCore())) return -1;
		if (linphone_core_get_current_call(getCore()->getCCore()))
			linphone_call_pause(linphone_core_get_current_call(getCore()->getCCore()));

		const auto &meAddress = me->getAddress();
		lInfo() << *meAddress << " is rejoining conference " << *getConferenceAddress();
		organizer = meAddress;
		addLocalEndpoint();
		if (me->getDevices().size() > 0) {
			participantDeviceJoined(me, me->getDevices().front());
		}
	}

	return 0;
}

void LocalConference::removeLocalEndpoint() {
	mMixerSession->enableLocalParticipant(false);

	if (isIn()) {
		mIsIn = false;

		time_t creationTime = time(nullptr);
		for (auto &device : me->getDevices()) {
			notifyParticipantDeviceRemoved(creationTime, false, getMe(), device);
		}
		notifyParticipantRemoved(creationTime, false, getMe());
	}
}

void LocalConference::leave() {
	if (isIn()) {
		lInfo() << *getMe()->getAddress() << " is leaving conference " << *getConferenceAddress();
		if (me->getDevices().size() > 0) {
			participantDeviceLeft(me, me->getDevices().front());
		}
		removeLocalEndpoint();
	}
}

bool LocalConference::validateNewParameters(const LinphonePrivate::ConferenceParams &newConfParams) const {
	if (!confParams) {
		return true;
	}

	if (confParams->getConferenceFactoryAddress() != newConfParams.getConferenceFactoryAddress()) {
		lError() << "Factory address change is not allowed: actual " << confParams->getConferenceFactoryAddress()
		         << " new value " << newConfParams.getConferenceFactoryAddress();
		return false;
	}

	if (confParams->getConferenceAddress() != newConfParams.getConferenceAddress()) {
		lError() << "Conference address change is not allowed: actual " << confParams->getConferenceAddress()
		         << " new value " << newConfParams.getConferenceAddress();
		return false;
	}

	if (confParams->getSecurityLevel() != newConfParams.getSecurityLevel()) {
		lError() << "Conference security level change is not allowed: actual " << confParams->getSecurityLevel()
		         << " new value " << newConfParams.getSecurityLevel();
		return false;
	}

	return true;
}

bool LocalConference::update(const LinphonePrivate::ConferenceParamsInterface &newParameters) {
	/* Only adding or removing video is supported. */
	bool previousVideoEnablement = confParams->videoEnabled();
	bool previousAudioEnablement = confParams->audioEnabled();
	bool previousChatEnablement = confParams->chatEnabled();
	const LinphonePrivate::ConferenceParams &newConfParams = static_cast<const ConferenceParams &>(newParameters);
	if (!validateNewParameters(newConfParams)) {
		return false;
	}
	confParams = ConferenceParams::create(newConfParams);

	if (!linphone_core_conference_server_enabled(getCore()->getCCore()) && confParams->videoEnabled()) {
		lWarning() << "Video capability in a conference is not supported when a device that is not a server is hosting "
		              "a conference.";
		confParams->enableVideo(false);
	}

	// Update endpoints only if audio or video settings have changed
	if ((confParams->videoEnabled() != previousVideoEnablement) ||
	    (confParams->audioEnabled() != previousAudioEnablement)) {
		/* Don't forget the local participant. For simplicity, a removeLocalEndpoint()/addLocalEndpoint() does the job.
		 */
		removeLocalEndpoint();
		addLocalEndpoint();
	}
	if ((confParams->chatEnabled() != previousChatEnablement) ||
	    (confParams->videoEnabled() != previousVideoEnablement) ||
	    (confParams->audioEnabled() != previousAudioEnablement)) {
		time_t creationTime = time(nullptr);
		notifyAvailableMediaChanged(creationTime, false, getMediaCapabilities());
	}

	bool mediaChanged = false;
	for (auto &meDev : me->getDevices()) {
		mediaChanged = false;
		mediaChanged |= meDev->setStreamCapability(
		    (confParams->audioEnabled() ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive),
		    LinphoneStreamTypeAudio);
		mediaChanged |= meDev->setStreamCapability(
		    (confParams->videoEnabled() ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive),
		    LinphoneStreamTypeVideo);
		mediaChanged |= meDev->setStreamCapability(
		    (confParams->chatEnabled() ? LinphoneMediaDirectionSendRecv : LinphoneMediaDirectionInactive),
		    LinphoneStreamTypeText);

		if (mediaChanged) {
			time_t creationTime = time(nullptr);
			notifyParticipantDeviceMediaCapabilityChanged(creationTime, false, me, meDev);
		}
	}
	return true;
}

int LocalConference::startRecording(const string &path) {
	MS2AudioMixer *mixer =
	    mMixerSession ? dynamic_cast<MS2AudioMixer *>(mMixerSession->getMixerByType(SalAudio)) : nullptr;
	if (mixer) {
		mixer->setRecordPath(path);
		mixer->startRecording();
		// TODO: error reporting is absent.
	} else {
		lError() << "LocalConference::startRecording(): no audio mixer.";
		return -1;
	}
	return 0;
}

bool LocalConference::isIn() const {
	return mIsIn;
}

const std::shared_ptr<Address> LocalConference::getOrganizer() const {
	return organizer;
}

AudioControlInterface *LocalConference::getAudioControlInterface() const {
	return mMixerSession ? dynamic_cast<AudioControlInterface *>(mMixerSession->getMixerByType(SalAudio)) : nullptr;
}

VideoControlInterface *LocalConference::getVideoControlInterface() const {
	return mMixerSession ? dynamic_cast<VideoControlInterface *>(mMixerSession->getMixerByType(SalVideo)) : nullptr;
}

AudioStream *LocalConference::getAudioStream() {
	MS2AudioMixer *mixer =
	    mMixerSession ? dynamic_cast<MS2AudioMixer *>(mMixerSession->getMixerByType(SalAudio)) : nullptr;
	return mixer ? mixer->getAudioStream() : nullptr;
}

void LocalConference::notifyFullState() {
	++lastNotify;
	Conference::notifyFullState();
}

std::shared_ptr<Call> LocalConference::getCall() const {
	return nullptr;
}

void LocalConference::callStateChangedCb(LinphoneCore *lc,
                                         LinphoneCall *call,
                                         LinphoneCallState cstate,
                                         BCTBX_UNUSED(const char *message)) {
	LinphoneCoreVTable *vtable = linphone_core_get_current_vtable(lc);
	LocalConference *conf = (LocalConference *)linphone_core_v_table_get_user_data(vtable);
	auto cppCall = Call::toCpp(call)->getSharedFromThis();
	if (conf && conf->getSharedFromThis() == cppCall->getConference()) {
		const auto &session = cppCall->getActiveSession();
		const std::shared_ptr<Address> &remoteAddress = cppCall->getRemoteAddress();
		const auto &device = findParticipantDevice(session);
		const auto &deviceState = device ? device->getState() : ParticipantDevice::State::ScheduledForJoining;

		switch (cstate) {
			case LinphoneCallStateOutgoingRinging:
				participantDeviceAlerting(session);
				break;
			case LinphoneCallStateConnected:
				if (getState() == ConferenceInterface::State::Created) {
					enter();
				}
				break;
			case LinphoneCallStateStreamsRunning: {
				if (!addParticipantDevice(cppCall)) {
					// If the participant is already in the conference
					const auto &participant = findParticipant(remoteAddress);
					auto remoteContactAddress = session->getRemoteContactAddress();
					if (participant) {
						if (device) {
							const auto deviceAddr = device->getAddress();
							const std::shared_ptr<Address> newDeviceAddress = remoteContactAddress;
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
							if (deviceState == ParticipantDevice::State::Present) {
								participantDeviceMediaCapabilityChanged(session);
							} else if ((deviceState == ParticipantDevice::State::Joining) ||
							           (deviceState == ParticipantDevice::State::ScheduledForJoining)) {
								// Participants complete their addition to a conference when the call goes back to the
								// StreamsRunning state
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
							         << " because it was not found in conference " << *getConferenceAddress();
						}
						bool admin = remoteContactAddress->hasParam("admin") &&
						             Utils::stob(remoteContactAddress->getParamValue("admin"));
						setParticipantAdminStatus(participant, admin);
					} else {
						lError() << "Unable to update admin status and device address as no participant with address "
						         << *remoteAddress << " has been found in conference " << *getConferenceAddress();
					}
				}
			} break;
			case LinphoneCallStatePausedByRemote:
				// The participant temporarely left the conference and put its call in pause
				// If a call in a local conference is paused by remote, it means that the remote participant temporarely
				// left the call, hence notify that no audio and video is available
				lInfo() << "Call in conference has been put on hold by remote device, hence participant "
				        << *remoteAddress << " temporarely left conference " << *getConferenceAddress();
				participantDeviceLeft(session);
				break;
			case LinphoneCallStateUpdatedByRemote: {
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
							lInfo() << "conference " << *getConferenceAddress() << " changed subject to \"" << subject
							        << "\"";
							setSubject(subject);
						}
					}
				}
			} break;
			case LinphoneCallStateEnd:
			case LinphoneCallStateError:
				lInfo() << "Removing terminated call (local address " << *session->getLocalAddress()
				        << " remote address " << *remoteAddress << ") from conference " << this << " ("
				        << *getConferenceAddress() << ")";
				if (session->getErrorInfo() &&
				    (linphone_error_info_get_reason(session->getErrorInfo()) == LinphoneReasonBusy)) {
					removeParticipantDevice(session);
				} else {
					removeParticipant(session, false);
				}
				break;
			default:
				break;
		}
	}
}

void LocalConference::transferStateChangedCb(LinphoneCore *lc,
                                             LinphoneCall *transfered,
                                             BCTBX_UNUSED(LinphoneCallState new_call_state)) {
	LinphoneCoreVTable *vtable = linphone_core_get_current_vtable(lc);
	LocalConference *conf = (LocalConference *)linphone_core_v_table_get_user_data(vtable);
	auto cppCall = Call::toCpp(transfered)->getSharedFromThis();
	if (conf && conf->findParticipantDevice(cppCall->getActiveSession())) {
		lInfo() << "LocalConference::" << __func__ << " not implemented";
	}
}

shared_ptr<ConferenceParticipantEvent> LocalConference::notifyParticipantAdded(
    time_t creationTime, const bool isFullState, const std::shared_ptr<Participant> &participant) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantAdded(creationTime, isFullState, participant);
}

shared_ptr<ConferenceParticipantEvent> LocalConference::notifyParticipantRemoved(
    time_t creationTime, const bool isFullState, const std::shared_ptr<Participant> &participant) {
	if (getState() != ConferenceInterface::State::TerminationPending) {
		// Increment last notify before notifying participants so that the delta can be calculated correctly
		++lastNotify;
		return Conference::notifyParticipantRemoved(creationTime, isFullState, participant);
	}

	return nullptr;
}

shared_ptr<ConferenceParticipantEvent> LocalConference::notifyParticipantSetAdmin(
    time_t creationTime, const bool isFullState, const std::shared_ptr<Participant> &participant, bool isAdmin) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantSetAdmin(creationTime, isFullState, participant, isAdmin);
}

shared_ptr<ConferenceSubjectEvent>
LocalConference::notifySubjectChanged(time_t creationTime, const bool isFullState, const std::string subject) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifySubjectChanged(creationTime, isFullState, subject);
}

shared_ptr<ConferenceAvailableMediaEvent> LocalConference::notifyAvailableMediaChanged(
    time_t creationTime, const bool isFullState, const std::map<ConferenceMediaCapabilities, bool> mediaCapabilities) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyAvailableMediaChanged(creationTime, isFullState, mediaCapabilities);
}

shared_ptr<ConferenceParticipantDeviceEvent> LocalConference::notifyParticipantDeviceScreenSharingChanged(
    time_t creationTime,
    const bool isFullState,
    const std::shared_ptr<Participant> &participant,
    const std::shared_ptr<ParticipantDevice> &participantDevice) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantDeviceScreenSharingChanged(creationTime, isFullState, participant,
	                                                               participantDevice);
}

shared_ptr<ConferenceParticipantDeviceEvent>
LocalConference::notifyParticipantDeviceAdded(time_t creationTime,
                                              const bool isFullState,
                                              const std::shared_ptr<Participant> &participant,
                                              const std::shared_ptr<ParticipantDevice> &participantDevice) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantDeviceAdded(creationTime, isFullState, participant, participantDevice);
}

shared_ptr<ConferenceParticipantDeviceEvent>
LocalConference::notifyParticipantDeviceRemoved(time_t creationTime,
                                                const bool isFullState,
                                                const std::shared_ptr<Participant> &participant,
                                                const std::shared_ptr<ParticipantDevice> &participantDevice) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	if ((getState() != ConferenceInterface::State::TerminationPending)) {
		++lastNotify;
		// Send notify only if it is not in state TerminationPending and:
		// - there are two or more participants in the conference
		return Conference::notifyParticipantDeviceRemoved(creationTime, isFullState, participant, participantDevice);
	}
	return nullptr;
}

shared_ptr<ConferenceParticipantDeviceEvent>
LocalConference::notifyParticipantDeviceStateChanged(time_t creationTime,
                                                     const bool isFullState,
                                                     const std::shared_ptr<Participant> &participant,
                                                     const std::shared_ptr<ParticipantDevice> &participantDevice) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantDeviceStateChanged(creationTime, isFullState, participant, participantDevice);
}

shared_ptr<ConferenceParticipantDeviceEvent> LocalConference::notifyParticipantDeviceMediaCapabilityChanged(
    time_t creationTime,
    const bool isFullState,
    const std::shared_ptr<Participant> &participant,
    const std::shared_ptr<ParticipantDevice> &participantDevice) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantDeviceMediaCapabilityChanged(creationTime, isFullState, participant,
	                                                                 participantDevice);
}

std::shared_ptr<Player> LocalConference::getPlayer() const {
	AudioControlInterface *intf = getAudioControlInterface();
	return intf ? intf->getPlayer() : nullptr;
}

bool LocalConference::sessionParamsAllowThumbnails() const {
	if (!mIsIn) {
		// The local participant is not in the conference therefore by default allow thumbnails
		return true;
	}
	auto session = static_pointer_cast<MediaSession>(me->getSession());
	return session->getMediaParams()->rtpBundleEnabled();
}

std::pair<bool, LinphoneMediaDirection> LocalConference::getMainStreamVideoDirection(
    const std::shared_ptr<CallSession> &session, bool localIsOfferer, bool useLocalParams) const {
	const auto ms = static_pointer_cast<MediaSession>(session);
	auto participantDevice = findParticipantDevice(session);
	auto deviceState = ParticipantDevice::State::ScheduledForJoining;
	if (participantDevice) {
		deviceState = participantDevice->getState();
	}
	auto isVideoConferenceEnabled = confParams->videoEnabled();
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

LinphoneMediaDirection
LocalConference::verifyVideoDirection(const std::shared_ptr<CallSession> &session,
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

} // end of namespace MediaConference

LINPHONE_END_NAMESPACE
