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

#include "remote_conference.h"
#include "call/call.h"
#include "conference/conference-params.h"
#include "conference/params/media-session-params-p.h"
#include "conference/params/media-session-params.h"
#include "conference/participant.h"
#include "conference/session/media-session-p.h"
#include "conference/session/media-session.h"
#include "conference/session/ms2-streams.h"
#include "core/core-p.h"
#include "core/core.h"
#include "event/event.h"
#include "sal/refer-op.h"
#ifdef HAVE_ADVANCED_IM
#include "conference/handlers/remote-conference-event-handler.h"
#endif // HAVE_ADVANCED_IM

using namespace std;

LINPHONE_BEGIN_NAMESPACE

namespace MediaConference {

RemoteConference::RemoteConference(const shared_ptr<Core> &core,
                                   const std::shared_ptr<Address> &meAddr,
                                   CallSessionListener *listener,
                                   const std::shared_ptr<LinphonePrivate::ConferenceParams> params)
    : Conference(core, meAddr, listener, params) {
}

RemoteConference::RemoteConference(const shared_ptr<Core> &core,
                                   const std::shared_ptr<Address> &focusAddr,
                                   const ConferenceId &conferenceId,
                                   CallSessionListener *listener,
                                   const std::shared_ptr<LinphonePrivate::ConferenceParams> params)
    : Conference(core, conferenceId.getLocalAddress(), listener, params) {

	createFocus(focusAddr);
	pendingSubject = confParams->getSubject();

	getMe()->setAdmin(true);

	confParams->enableLocalParticipant(false);

	// Store conference ID to retrieve later the local address when the focus call goes to StreamsRunning state
	this->conferenceId = conferenceId;

	setState(ConferenceInterface::State::Instantiated);
}

RemoteConference::~RemoteConference() {
	terminate();
#ifdef HAVE_ADVANCED_IM
	eventHandler.reset();
#endif // HAVE_ADVANCED_IM
}

void RemoteConference::createFocus(const std::shared_ptr<Address> focusAddr,
                                   const std::shared_ptr<LinphonePrivate::CallSession> focusSession) {
	focus = Participant::create(this, focusAddr, focusSession);
	lInfo() << "Create focus '" << *focus->getAddress() << "' from address : " << *focusAddr;
}

void RemoteConference::initWithInvitees(const std::shared_ptr<LinphonePrivate::Call> &focusCall,
                                        const ConferenceInfo::participant_list_t &invitees,
                                        const ConferenceId &conferenceId) {
	const auto &focusSession = focusCall->getActiveSession();
	const auto &conferenceAddress = focusSession->getRemoteContactAddress();
	createFocus(conferenceAddress, focusSession);
	pendingSubject = confParams->getSubject();
	setConferenceId(conferenceId);

#ifdef HAVE_DB_STORAGE
	auto &mainDb = getCore()->getPrivate()->mainDb;
	if (mainDb) {
		const auto &confInfo = mainDb->getConferenceInfoFromURI(conferenceAddress);
		// me is admin if the organizer is the same as me
		getMe()->setAdmin((confInfo && (confInfo->getOrganizerAddress()->weakEqual(*getMe()->getAddress()))));
	}
#endif
	mInvitedParticipants = invitees;

	setState(ConferenceInterface::State::Instantiated);

	setConferenceAddress(conferenceAddress);
	finalizeCreation();
}

void RemoteConference::initWithInvitees(const std::shared_ptr<Address> confAddr,
                                        const std::shared_ptr<Address> focusAddr,
                                        const std::shared_ptr<LinphonePrivate::CallSession> focusSession,
                                        const ConferenceInfo::participant_list_t &invitees,
                                        const ConferenceId &conferenceId) {
	createFocus(focusAddr, focusSession);
	confParams->enableLocalParticipant(false);
	pendingSubject = confParams->getSubject();

	std::shared_ptr<Address> organizer;
#ifdef HAVE_DB_STORAGE
	auto &mainDb = getCore()->getPrivate()->mainDb;
	if (mainDb) {
		const auto &confInfo = mainDb->getConferenceInfoFromURI(confAddr);
		// me is admin if the organizer is the same as me
		if (confInfo) {
			organizer = confInfo->getOrganizerAddress();
		}
	}
#endif
	getMe()->setAdmin((organizer == nullptr) || organizer->weakEqual(*(getMe()->getAddress())));

	mInvitedParticipants = invitees;

	setState(ConferenceInterface::State::Instantiated);

	setConferenceId(conferenceId);
	setConferenceAddress(confAddr);
	finalizeCreation();
}
void RemoteConference::finalizeCreation() {

	if (getState() == ConferenceInterface::State::CreationPending) {
		if (finalized) {
			lDebug() << "Conference " << this << " has already been finalized";
			return;
		} else {
			finalized = true;
			addListener(
			    std::shared_ptr<ConferenceListenerInterface>(static_cast<ConferenceListenerInterface *>(this),
			                                                 [](BCTBX_UNUSED(ConferenceListenerInterface * p)) {}));
#ifdef HAVE_ADVANCED_IM
			bool_t eventLogEnabled = linphone_config_get_bool(linphone_core_get_config(getCore()->getCCore()), "misc",
			                                                  "conference_event_log_enabled", TRUE);
			if (eventLogEnabled) {
				eventHandler = std::make_shared<RemoteConferenceEventHandler>(this, this);
				eventHandler->subscribe(getConferenceId());
			} else {
#endif // HAVE_ADVANCED_IM
				lInfo() << "Unable to send SUBSCRIBE to finalize creation of conference " << *getConferenceAddress()
				        << " because conference event package (RFC 4575) is disabled or the SDK was not compiled with "
				           "ENABLE_ADVANCED_IM flag set to on";
#ifdef HAVE_ADVANCED_IM
			}
#endif // HAVE_ADVANCED_IM
		}
	} else {
		lError() << "Cannot finalize creation of Conference in state " << getState();
	}
}

void RemoteConference::updateAndSaveConferenceInformations() {
	const auto conferenceInfo = getUpdatedConferenceInfo();
	long long conferenceInfoId = -1;

#ifdef HAVE_DB_STORAGE
	// Store into DB after the start incoming notification in order to have a valid conference address being the contact
	// address of the call
	auto &mainDb = getCore()->getPrivate()->mainDb;
	if (mainDb) {
		lInfo() << "Inserting or updating conference information to database related to conference "
		        << *getConferenceAddress();
		conferenceInfoId = mainDb->insertConferenceInfo(conferenceInfo);
	}
#endif // HAVE_DB_STORAGE

	auto callLog = getMainSession() ? getMainSession()->getLog() : nullptr;
	if (callLog) {
		callLog->setConferenceInfo(conferenceInfo);
		callLog->setConferenceInfoId(conferenceInfoId);
	}
}

std::shared_ptr<ConferenceInfo> RemoteConference::createConferenceInfo() const {
	auto session = static_pointer_cast<MediaSession>(getMainSession());
	const auto referer = (session ? L_GET_PRIVATE(session->getMediaParams())->getReferer() : nullptr);
	const auto guessedOrganizer = getOrganizer();
	std::shared_ptr<Address> organizer = nullptr;
	if (guessedOrganizer) {
		organizer = guessedOrganizer;
	} else if (referer) {
		organizer = referer->getRemoteAddress();
	} else {
		organizer = getMe()->getAddress();
	}
	return createConferenceInfoWithCustomParticipantList(organizer, getFullParticipantList());
}

void RemoteConference::setMainSession(const std::shared_ptr<LinphonePrivate::CallSession> &session) {
	if (focus) {
		focus->setSession(session);
	}
}

const std::shared_ptr<CallSession> RemoteConference::getMainSession() const {
	return focus ? focus->getSession() : nullptr;
}

int RemoteConference::startRecording(const char *path) {
	auto session = static_pointer_cast<MediaSession>(getMainSession());
	if (session) {
		session->setRecordPath(path);
		session->startRecording();
	} else {
		lError() << "RemoteConference::startRecording(): no audio session.";
		return -1;
	}
	return 0;
}

void RemoteConference::notifyStateChanged(LinphonePrivate::ConferenceInterface::State state) {
	// Call callbacks before calling listeners because listeners may change state
	linphone_core_notify_conference_state_changed(getCore()->getCCore(), toC(), (LinphoneConferenceState)getState());

	Conference::notifyStateChanged(state);
}

bool RemoteConference::dialOutAddresses(BCTBX_UNUSED(const std::list<std::shared_ptr<Address>> &addressList)) {
	lError() << "RemoteConference::dialOutAddresses() not implemented";
	return false;
}

int RemoteConference::inviteAddresses(BCTBX_UNUSED(const list<std::shared_ptr<Address>> &addresses),
                                      BCTBX_UNUSED(const LinphoneCallParams *params)) {
	lError() << "RemoteConference::inviteAddresses() not implemented";
	return -1;
}

int RemoteConference::participantDeviceLeft(
    BCTBX_UNUSED(const std::shared_ptr<LinphonePrivate::CallSession> &session)) {
	lError() << "RemoteConference::participantDeviceLeft() not implemented";
	return -1;
}

int RemoteConference::participantDeviceLeft(
    BCTBX_UNUSED(const std::shared_ptr<LinphonePrivate::Participant> &participant),
    BCTBX_UNUSED(const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device)) {
	lError() << "RemoteConference::participantDeviceLeft() not implemented";
	return -1;
}

int RemoteConference::participantDeviceAlerting(
    BCTBX_UNUSED(const std::shared_ptr<LinphonePrivate::CallSession> &session)) {
	lError() << "RemoteConference::participantDeviceAlerting() not implemented";
	return -1;
}

int RemoteConference::participantDeviceAlerting(
    BCTBX_UNUSED(const std::shared_ptr<LinphonePrivate::Participant> &participant),
    const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) {
	device->setState(ParticipantDevice::State::Alerting);
	return 0;
}

int RemoteConference::participantDeviceJoined(
    BCTBX_UNUSED(const std::shared_ptr<LinphonePrivate::CallSession> &session)) {
	lError() << "RemoteConference::participantDeviceJoined() not implemented";
	return -1;
}

int RemoteConference::participantDeviceJoined(
    BCTBX_UNUSED(const std::shared_ptr<LinphonePrivate::Participant> &participant),
    BCTBX_UNUSED(const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device)) {
	lError() << "RemoteConference::participantDeviceJoined() not implemented";
	return -1;
}

int RemoteConference::participantDeviceMediaCapabilityChanged(
    BCTBX_UNUSED(const std::shared_ptr<LinphonePrivate::CallSession> &session)) {
	lError() << "RemoteConference::participantDeviceMediaCapabilityChanged() not implemented";
	return -1;
}

int RemoteConference::participantDeviceMediaCapabilityChanged(BCTBX_UNUSED(const std::shared_ptr<Address> &addr)) {
	lError() << "RemoteConference::participantDeviceMediaCapabilityChanged() not implemented";
	return -1;
}

int RemoteConference::participantDeviceMediaCapabilityChanged(
    BCTBX_UNUSED(const std::shared_ptr<LinphonePrivate::Participant> &participant),
    BCTBX_UNUSED(const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device)) {
	lError() << "RemoteConference::participantDeviceMediaCapabilityChanged() not implemented";
	return -1;
}

int RemoteConference::participantDeviceSsrcChanged(
    BCTBX_UNUSED(const std::shared_ptr<LinphonePrivate::CallSession> &session),
    BCTBX_UNUSED(const LinphoneStreamType type),
    BCTBX_UNUSED(uint32_t ssrc)) {
	lError() << "RemoteConference::participantDeviceSsrcChanged() not implemented";
	return -1;
}

int RemoteConference::participantDeviceSsrcChanged(
    BCTBX_UNUSED(const std::shared_ptr<LinphonePrivate::CallSession> &session),
    BCTBX_UNUSED(uint32_t audioSsrc),
    BCTBX_UNUSED(uint32_t videoSsrc)) {
	lError() << "RemoteConference::participantDeviceSsrcChanged() not implemented";
	return -1;
}

void RemoteConference::setLocalParticipantStreamCapability(const LinphoneMediaDirection &direction,
                                                           const LinphoneStreamType type) {
	auto session = static_pointer_cast<MediaSession>(getMainSession());
	if (session) {
		const MediaSessionParams *params = session->getMediaParams();
		MediaSessionParams *currentParams = params->clone();
		if (!currentParams->rtpBundleEnabled()) {
			currentParams->enableRtpBundle(true);
		}
		lInfo() << "Setting direction of stream of type " << std::string(linphone_stream_type_to_string(type)) << " to "
		        << std::string(linphone_media_direction_to_string(direction)) << " of main session " << session;
		switch (type) {
			case LinphoneStreamTypeAudio:
				currentParams->enableAudio((direction != LinphoneMediaDirectionInactive) &&
				                           (direction != LinphoneMediaDirectionInvalid));
				currentParams->setAudioDirection(direction);
				break;
			case LinphoneStreamTypeVideo:
				currentParams->enableVideo((direction != LinphoneMediaDirectionInactive) &&
				                           (direction != LinphoneMediaDirectionInvalid));
				currentParams->setVideoDirection(direction);
				break;
			case LinphoneStreamTypeText:
				currentParams->enableRealtimeText((direction != LinphoneMediaDirectionInactive) &&
				                                  (direction != LinphoneMediaDirectionInvalid));
				break;
			case LinphoneStreamTypeUnknown:
				lError() << "Unable to set direction of stream of type "
				         << std::string(linphone_stream_type_to_string(type));
				return;
		}
		session->update(currentParams);
		delete currentParams;
	}
}

int RemoteConference::getParticipantDeviceVolume(const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) {
	AudioStream *as = getAudioStream();
	if (as != nullptr) {
		return audio_stream_get_participant_volume(as, device->getSsrc(LinphoneStreamTypeAudio));
	}

	return AUDIOSTREAMVOLUMES_NOT_FOUND;
}

bool RemoteConference::addParticipantDevice(std::shared_ptr<LinphonePrivate::Call> call) {

	bool success = Conference::addParticipantDevice(call);

	if (success) {
		auto device = findParticipantDevice(call->getActiveSession());
		if (device) {
			// In a remote conference, the participant has no session attached ot it.
			device->setSession(nullptr);
			const auto &p = device->getParticipant();
			if (p) {
				time_t creationTime = time(nullptr);
				notifyParticipantDeviceAdded(creationTime, false, p, device);
			}
		}
	}

	return success;
}

bool RemoteConference::addParticipant(BCTBX_UNUSED(const std::shared_ptr<ParticipantInfo> &info)) {
	lError() << "addParticipant() with a participant info is not allowed on a RemoteConference";
	return false;
}

bool RemoteConference::addParticipant(const std::shared_ptr<Address> &participantAddress) {
	// Search call that matches participant session
	const std::list<std::shared_ptr<Call>> &coreCalls = getCore()->getCalls();
	auto callIt = std::find_if(coreCalls.cbegin(), coreCalls.cend(), [&](const std::shared_ptr<Call> &c) {
		return (participantAddress->weakEqual(*c->getRemoteAddress()));
	});
	bool ret = false;
	if (callIt != coreCalls.cend()) {
		std::shared_ptr<Call> call = *callIt;
		ret = addParticipant(call);
	} else {
		const list<std::shared_ptr<Address>> addresses{participantAddress};
		ret = addParticipants(addresses);
	}
	return ret;
}

bool RemoteConference::addParticipant(std::shared_ptr<LinphonePrivate::Call> call) {

#if 0
	if (getMe()->isAdmin() && !isConferenceEnded() && isConferenceStarted()) {
#endif
	if (getMe()->isAdmin()) {
		std::shared_ptr<Address> focusAddress = focus->getAddress();
		LinphoneCallParams *params;
		std::shared_ptr<LinphonePrivate::Call> focusCall = nullptr;
		const auto &remoteAddress = call->getRemoteAddress();
		const auto conferenceAddressStr =
		    (getConferenceAddress() ? getConferenceAddress()->toString() : std::string("<address-not-defined>"));
		const auto remoteAddressStr = (remoteAddress ? remoteAddress->toString() : "Unknown");
		switch (state) {
			case ConferenceInterface::State::None:
			case ConferenceInterface::State::Instantiated:
			case ConferenceInterface::State::CreationFailed: {
				lInfo() << "Calling the conference focus (" << *focusAddress << ")";
				params = linphone_core_create_call_params(getCore()->getCCore(), nullptr);
				// Participant with the focus call is admin
				L_GET_CPP_PTR_FROM_C_OBJECT(params)->addCustomContactParameter("admin", Utils::toString(true));
				linphone_call_params_enable_video(params, confParams->videoEnabled());
				Conference::setSubject(pendingSubject);
				auto focusCallC = linphone_core_invite_address_with_params_2(
				    getCore()->getCCore(), focusAddress->toC(), params, L_STRING_TO_C(pendingSubject), nullptr);
				linphone_call_params_unref(params);
				if (focusCallC) {
					focusCall = Call::toCpp(focusCallC)->getSharedFromThis();
					focusCall->setConference(getSharedFromThis());
					focus->setSession(focusCall->getActiveSession());
				}
				auto callIt = std::find(m_pendingCalls.begin(), m_pendingCalls.end(), call);
				if (callIt == m_pendingCalls.end()) {
					lInfo() << "Adding call (local address " << *call->getLocalAddress() << " remote address "
					        << remoteAddressStr << ") to the list of call to add to conference " << conferenceAddressStr
					        << " (" << this << ")";
					m_pendingCalls.push_back(call);
					Conference::addParticipant(call->getRemoteAddress());
				} else {
					lError() << "Trying to add call (local address " << *call->getLocalAddress() << " remote address "
					         << remoteAddressStr << ") twice to conference " << conferenceAddressStr << " (" << this
					         << ")";
				}
			}
				return true;
			case ConferenceInterface::State::CreationPending:
			case ConferenceInterface::State::Created:
				if (focus->getSession()) {
					if (focusIsReady()) {
						Conference::addParticipant(call->getRemoteAddress());
						transferToFocus(call);
					} else {
						auto callIt = std::find(m_pendingCalls.begin(), m_pendingCalls.end(), call);
						if (callIt == m_pendingCalls.end()) {
							lInfo() << "Adding call (local address " << call->getLocalAddress()->toString()
							        << " remote address " << remoteAddressStr
							        << ") to the list of call to add to conference " << *getConferenceAddress() << " ("
							        << this << ")";
							m_pendingCalls.push_back(call);
							Conference::addParticipant(call);
						} else {
							lError() << "Trying to add call (local address " << call->getLocalAddress()->toString()
							         << " remote address " << remoteAddressStr << ") twice to conference "
							         << conferenceAddressStr << " (" << this << ")";
						}
					}
				} else {
					lInfo() << "Calling the conference focus (" << *focusAddress << ")";
					params = linphone_core_create_call_params(getCore()->getCCore(), nullptr);
					// Participant with the focus call is admin
					L_GET_CPP_PTR_FROM_C_OBJECT(params)->addCustomContactParameter("admin", Utils::toString(true));
					linphone_call_params_enable_video(params, confParams->videoEnabled());
					Conference::setSubject(pendingSubject);
					focusCall = Call::toCpp(linphone_core_invite_address_with_params_2(
					                            getCore()->getCCore(), focusAddress->toC(), params,
					                            L_STRING_TO_C(pendingSubject), nullptr))
					                ->getSharedFromThis();
					focusCall->setConference(getSharedFromThis());
					focus->setSession(focusCall->getActiveSession());
					m_pendingCalls.push_back(call);
					linphone_call_params_unref(params);
				}
				return true;
			default:
				lError() << "Could not add call " << call << " to the conference. Bad conference state ("
				         << Utils::toString(state) << ")";
				return false;
		}
	} else if (!getMe()->isAdmin()) {
		lError() << "Could not add call " << call << " to the conference because local participant "
		         << getMe()->getAddress() << " is not admin.";
	} else {
		const auto &endTime = confParams->getEndTime();
		const auto &startTime = confParams->getStartTime();
		const auto now = time(NULL);
		lError() << "Could not add call " << call
		         << " to the conference because the conference is not active right now.";
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

bool RemoteConference::finalizeParticipantAddition(BCTBX_UNUSED(std::shared_ptr<LinphonePrivate::Call> call)) {
	lError() << "RemoteConference::finalizeParticipantAddition() not implemented";
	return false;
}

// Removes own address and existing participants from the list.
// Also removes gruu from kept addresses
list<std::shared_ptr<Address>>
RemoteConference::cleanAddressesList(const list<std::shared_ptr<Address>> &addresses) const {
	list<std::shared_ptr<Address>> cleanedList(addresses);
	cleanedList.sort();
	cleanedList.unique();
	for (auto it = cleanedList.begin(); it != cleanedList.end();) {
		auto address = (*it);
		if (findParticipant(address) || (address->weakEqual(*getMe()->getAddress()))) {
			it = cleanedList.erase(it);
		} else {
			it++;
		}
	}
	return cleanedList;
}

bool RemoteConference::addParticipants(const std::list<std::shared_ptr<Call>> &calls) {
	const auto &coreCurrentCall = getCore()->getCurrentCall();
	const bool startingConference = (getState() == ConferenceInterface::State::CreationPending);
	const auto &outputDevice = (coreCurrentCall) ? coreCurrentCall->getOutputAudioDevice() : nullptr;
	const auto &inputDevice = (coreCurrentCall) ? coreCurrentCall->getInputAudioDevice() : nullptr;

#if 0
	if (getMe()->isAdmin() && !isConferenceEnded() && isConferenceStarted()) {
#endif
	if (getMe()->isAdmin()) {
		bool success = Conference::addParticipants(calls);
		// If current call is not NULL and the conference is in the creating pending state or instantied, then try to
		// change audio route to keep the one currently used Do not change audio route if participant addition is not
		// successful
		if (success && startingConference) {
			if (outputDevice) {
				setOutputAudioDevice(outputDevice);
			}
			if (inputDevice) {
				setInputAudioDevice(inputDevice);
			}
		}
	}

	return false;
}

bool RemoteConference::addParticipants(const list<std::shared_ptr<Address>> &addresses) {
#if 0
	if (getMe()->isAdmin() && !isConferenceEnded()) {
#endif
	if (getMe()->isAdmin()) {
		if ((state == ConferenceInterface::State::Instantiated) ||
		    (state == ConferenceInterface::State::CreationPending)) {
			getCore()->createConferenceOnServer(confParams, getMe()->getAddress(), addresses);
		} else {
			SalReferOp *referOp = new SalReferOp(getCore()->getCCore()->sal.get());
			LinphoneAddress *lAddr = getConferenceAddress()->toC();
			linphone_configure_op(getCore()->getCCore(), referOp, lAddr, nullptr, true);
			for (const auto &addr : addresses) {
				std::shared_ptr<Address> referToAddr = addr;
				referToAddr->setParam("isfocus");
				referOp->sendRefer(referToAddr->getImpl());
			}
			referOp->unref();
		}
	} else {
		const auto &endTime = confParams->getEndTime();
		const auto now = time(NULL);
		lError() << "Could not add participants to the conference because local participant " << getMe()->getAddress()
		         << " is not admin or conference already ended (expected endtime: " << asctime(gmtime(&endTime))
		         << " now: " << asctime(gmtime(&now));
		return false;
	}

	return true;
}

int RemoteConference::removeParticipant(const std::shared_ptr<LinphonePrivate::CallSession> &session,
                                        BCTBX_UNUSED(const bool preserveSession)) {
	const std::shared_ptr<Address> &remoteAddress = session->getRemoteAddress();
	std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(remoteAddress);
	if (getMe()->isAdmin()) {
		if (p) {
			return removeParticipant(p) ? 0 : -1;
		}
	} else {
		lError() << "Unable to remove participant " << p->getAddress()->toString() << " because focus "
		         << getMe()->getAddress()->toString() << " is not admin";
	}
	return -1;
}

bool RemoteConference::removeParticipant(const std::shared_ptr<LinphonePrivate::Participant> &participant) {
	if (getMe()->isAdmin()) {
		return (removeParticipant(participant->getAddress()) == 0) ? true : false;
	} else {
		lError() << "Unable to remove participant " << participant->getAddress()->toString() << " because focus "
		         << getMe()->getAddress()->toString() << " is not admin";
	}
	return false;
}

int RemoteConference::removeParticipant(const std::shared_ptr<Address> &addr) {
	auto session = getMainSession();
	if (getMe()->isAdmin()) {
		const auto conferenceAddressStr =
		    (getConferenceAddress() ? getConferenceAddress()->toString() : std::string("<address-not-defined>"));
		std::shared_ptr<LinphonePrivate::Participant> p = findParticipant(addr);
		if (p) {
			switch (state) {
				case ConferenceInterface::State::Created:
				case ConferenceInterface::State::TerminationPending: {
					if (!findParticipant(addr)) {
						lError() << "Conference: could not remove participant \'" << *addr
						         << "\': not in the participants list";
						return -1;
					}
					LinphoneCore *cCore = getCore()->getCCore();
					SalReferOp *referOp = new SalReferOp(cCore->sal.get());
					LinphoneAddress *lAddr = linphone_address_new(session->getRemoteContact().c_str());
					linphone_configure_op(cCore, referOp, lAddr, nullptr, false);
					linphone_address_unref(lAddr);
					std::shared_ptr<Address> referToAddr = addr;
					referToAddr->setMethodParam("BYE");
					auto res = referOp->sendRefer(referToAddr->getImpl());
					referOp->unref();

					if (res != 0) {
						lError() << "Conference: could not remove participant \'" << *addr
						         << "\': REFER with BYE has failed";
						return -1;
					}
				} break;
				default:
					lError() << "Could not remove participant " << *addr << " from conference " << conferenceAddressStr
					         << ". Bad conference state (" << Utils::toString(state) << ")";
					return -1;
			}
			return 0;
		} else {
			lWarning() << "Unable to remove participant " << *addr << " because it is not part of the conference "
			           << conferenceAddressStr;
		}
	} else {
		lWarning() << "Unable to remove participant " << *addr << " because local participant "
		           << *getMe()->getAddress() << " is not admin.";
	}
	return -1;
}

int RemoteConference::terminate() {
	auto savedState = state;
	auto session = getMainSession();
	shared_ptr<Call> sessionCall = nullptr;
	if (session) {
		auto op = session->getPrivate()->getOp();
		sessionCall = op ? getCore()->getCallByCallId(op->getCallId()) : nullptr;
	}

	switch (savedState) {
		case ConferenceInterface::State::Created:
		case ConferenceInterface::State::CreationPending:
		case ConferenceInterface::State::CreationFailed:
			if (sessionCall) {
				// Conference will be deleted by terminating the session
				session->terminate();
				return 0;
			}
			break;
		default:
			break;
	}

	if ((state != ConferenceInterface::State::Terminated) && (state != ConferenceInterface::State::Deleted)) {
		setState(ConferenceInterface::State::TerminationPending);
	}
	return 0;
}

int RemoteConference::enter() {
	const auto &conferenceAddress = getConferenceAddress();
	if (!conferenceAddress || !conferenceAddress->isValid()) {
		lError() << "Could not enter in the conference because its conference address (" << conferenceAddress
		         << ") is not valid";
		return -1;
	}

	auto session = static_pointer_cast<MediaSession>(getMainSession());

	if (session) {
		LinphoneCallState callState = static_cast<LinphoneCallState>(session->getState());
		switch (callState) {
			case LinphoneCallStreamsRunning:
				break;
			case LinphoneCallPaused:
				session->resume();
				participantDeviceJoined(me, me->getDevices().front());
				break;
			default:
				lError() << "Could not join the conference: bad focus call state (" << Utils::toString(callState)
				         << ")";
				return -1;
		}
	} else {
		/* Start a new call by indicating that it has to be put into the conference directly */
		LinphoneCallParams *new_params = linphone_core_create_call_params(getCore()->getCCore(), nullptr);
		linphone_call_params_enable_video(new_params, confParams->videoEnabled());

		linphone_call_params_set_in_conference(new_params, FALSE);

		L_GET_CPP_PTR_FROM_C_OBJECT(new_params)
		    ->addCustomContactParameter("admin", Utils::toString(getMe()->isAdmin()));

		const std::shared_ptr<Address> &address = getConferenceAddress();
		const string &confId = address->getUriParamValue("conf-id");
		linphone_call_params_set_conference_id(new_params, confId.c_str());

		std::string subject = getMe()->isAdmin() ? getSubject() : std::string();

		auto cCall = linphone_core_invite_address_with_params_2(getCore()->getCCore(), address->toC(), new_params,
		                                                        L_STRING_TO_C(subject), nullptr);

		linphone_call_params_unref(new_params);

		auto cppCall = Call::toCpp(cCall);
		cppCall->setConference(getSharedFromThis());
		focus->setSession(cppCall->getActiveSession());
	}
	return 0;
}

void RemoteConference::leave() {
	if (state != ConferenceInterface::State::Created) {
		lError() << "Could not leave the conference: bad conference state (" << Utils::toString(state) << ")";
	}

	auto session = static_pointer_cast<MediaSession>(getMainSession());
	LinphoneCallState callState = static_cast<LinphoneCallState>(session->getState());
	switch (callState) {
		case LinphoneCallPaused:
			lInfo() << getMe()->getAddress() << " is leaving conference " << *getConferenceAddress()
			        << " while focus call is paused.";
			break;
		case LinphoneCallStreamsRunning:
			lInfo() << getMe()->getAddress() << " is leaving conference " << *getConferenceAddress()
			        << ". Focus call is going to be paused.";
			session->pause();
			participantDeviceLeft(me, me->getDevices().front());
			break;
		default:
			lError() << getMe()->getAddress() << " cannot leave conference " << *getConferenceAddress()
			         << " because focus call is in state " << linphone_call_state_to_string(callState);
	}
}

bool RemoteConference::isIn() const {
	if (state != ConferenceInterface::State::Created) return false;
	const auto &session = getMainSession();
	if (!session) return false;
	LinphoneCallState callState = static_cast<LinphoneCallState>(session->getState());
	const auto &focusContactAddress = session->getRemoteContactAddress();
	return ((callState == LinphoneCallUpdatedByRemote) || (callState == LinphoneCallUpdating) ||
	        (callState == LinphoneCallStreamsRunning)) &&
	       focusContactAddress->hasUriParam("conf-id");
}

const std::shared_ptr<Address> RemoteConference::getOrganizer() const {
	std::shared_ptr<Address> organizer = nullptr;
#ifdef HAVE_DB_STORAGE
	auto &mainDb = getCore()->getPrivate()->mainDb;
	if (mainDb) {
		const auto &confInfo = mainDb->getConferenceInfoFromURI(getConferenceAddress());
		// me is admin if the organizer is the same as me
		if (confInfo) {
			organizer = confInfo->getOrganizerAddress();
		}
	}
#endif
	return organizer;
}

bool RemoteConference::focusIsReady() const {
	LinphoneCallState focusState;
	const auto &session = getMainSession();
	if (!session || !session->getRemoteContactAddress()) return false;
	focusState = static_cast<LinphoneCallState>(session->getState());
	return (focusState == LinphoneCallStreamsRunning) || (focusState == LinphoneCallPaused);
}

bool RemoteConference::transferToFocus(std::shared_ptr<LinphonePrivate::Call> call) {
	auto session = getMainSession();
	std::shared_ptr<Address> referToAddr(session->getRemoteContactAddress());
	const std::shared_ptr<Address> &remoteAddress = call->getRemoteAddress();
	std::shared_ptr<Participant> participant = findParticipant(remoteAddress);
	if (participant) {
		referToAddr->setParam("admin", Utils::toString(participant->isAdmin()));
		const auto &remoteAddress = call->getRemoteAddress();
		lInfo() << "Transfering call (local address " << call->getLocalAddress()->toString() << " remote address "
		        << (remoteAddress ? remoteAddress->toString() : "Unknown") << ") to focus " << *referToAddr;
		updateParticipantInConferenceInfo(participant);
		if (call->transfer(referToAddr->toString()) == 0) {
			m_transferingCalls.push_back(call);
			return true;
		} else {
			lError() << "Conference: could not transfer call " << call << " to " << *referToAddr;
			return false;
		}
	} else {
		lError() << "Conference: could not transfer call " << call << " to " << *referToAddr
		         << " because participant with session " << call->getActiveSession()
		         << " cannot be found  - guessed address " << *referToAddr;
		return false;
	}
	return false;
}

void RemoteConference::reset() {
	auto session = getMainSession();
	if (session) {
		auto op = session->getPrivate()->getOp();
		auto sessionCall = op ? getCore()->getCallByCallId(op->getCallId()) : nullptr;
		if (sessionCall) sessionCall->setConference(nullptr);
	}
	m_pendingCalls.clear();
	m_transferingCalls.clear();
}

void RemoteConference::onFocusCallStateChanged(LinphoneCallState state) {
	auto session = getMainSession();
	std::shared_ptr<Address> focusContactAddress;
	std::shared_ptr<Call> call = nullptr;

	// Take a ref as conference may be deleted when the call goes to states PausedByRemote or End
	shared_ptr<Conference> ref = getSharedFromThis();
	SalCallOp *op = nullptr;

	if (session) {
		focusContactAddress = session->getRemoteContactAddress();
		op = session->getPrivate()->getOp();
		call = op ? getCore()->getCallByCallId(op->getCallId()) : nullptr;
	}

	list<std::shared_ptr<LinphonePrivate::Call>>::iterator it;
	switch (state) {
		case LinphoneCallStreamsRunning: {

			updateParticipantInConferenceInfo(getMe());
			const auto &previousState = session->getPreviousState();
			// NOTIFY that a participant has been added only if it follows a resume of the call
			if (previousState == CallSession::State::Resuming) {
				// The participant rejoins the conference
				time_t creationTime = time(nullptr);
				notifyParticipantAdded(creationTime, false, getMe());
			}
			for (const auto &device : getParticipantDevices()) {
				device->updateStreamAvailabilities();
			}
			if (focusContactAddress->hasParam("isfocus") &&
			    ((call && !call->mediaInProgress()) ||
			     !!!linphone_config_get_int(linphone_core_get_config(session->getCore()->getCCore()), "sip",
			                                "update_call_when_ice_completed", TRUE)) &&
			    finalized && fullStateReceived && (getState() == ConferenceInterface::State::CreationPending)) {
				auto requestStreams = [this]() -> LinphoneStatus {
					lInfo() << "Sending re-INVITE in order to get streams after joining conference "
					        << *getConferenceAddress();
					setState(ConferenceInterface::State::Created);
					auto ret = updateMainSession();
					return ret;
				};

				if (requestStreams() != 0) {
					lInfo() << "Delaying re-INVITE in order to get streams after joining conference "
					        << *getConferenceAddress()
					        << " because the dialog is not available yet to accept this transaction";
					scheduleUpdate = true;
				} else {
					// An update has been successfully sent therefore clear the flag scheduleUpdate to avoid sending it
					// twice.
					scheduleUpdate = false;
				}
			}
		}
			BCTBX_NO_BREAK; /* Intentional no break */
		case LinphoneCallConnected:
		case LinphoneCallPaused:
		case LinphoneCallUpdatedByRemote:
			if (focusContactAddress->hasParam("isfocus")) {
				setConferenceAddress(focusContactAddress);
				it = m_pendingCalls.begin();
				while (it != m_pendingCalls.end()) {
					std::shared_ptr<LinphonePrivate::Call> pendingCall = *it;
					LinphoneCallState pendingCallState = static_cast<LinphoneCallState>(pendingCall->getState());
					if ((pendingCallState == LinphoneCallStreamsRunning) || (pendingCallState == LinphoneCallPaused)) {
						it = m_pendingCalls.erase(it);
						transferToFocus(pendingCall);
					} else it++;
				}

				if (!finalized) {
					setConferenceId(ConferenceId(std::shared_ptr<Address>(focusContactAddress),
					                             getConferenceId().getLocalAddress()));
					if (call) {
						if (focusContactAddress->hasUriParam("conf-id")) {
							call->setConferenceId(focusContactAddress->getUriParamValue("conf-id"));
						}
						if (!call->mediaInProgress() ||
						    !!!linphone_config_get_int(linphone_core_get_config(session->getCore()->getCCore()), "sip",
						                               "update_call_when_ice_completed", TRUE)) {
							finalizeCreation();
						}
					}
				}
			}
			BCTBX_NO_BREAK; /* Intentional no break */
		case LinphoneCallPausedByRemote:
			if (!focusContactAddress->hasParam("isfocus") && (state != LinphoneCallConnected)) {
				// The call was in conference and the focus removed its attribute to show that the call exited the
				// conference
				lInfo() << "Ending conference " << this << "(" << *getConferenceAddress()
				        << ") because server removed isfocus attribute from its remote address " << focusContactAddress;
				endConference();
			}
			break;
		case LinphoneCallError:
			setState(ConferenceInterface::State::CreationFailed);
			if (call) call->setConference(nullptr);
			it = m_pendingCalls.begin();
			while (it != m_pendingCalls.end()) {
				std::shared_ptr<LinphonePrivate::Call> pendingCall = *it;
				pendingCall->setConference(nullptr);
				it++;
			}
			break;
		case LinphoneCallEnd:
			lInfo() << "Ending conference " << this << "(" << *getConferenceAddress()
			        << ") because focus call (local address "
			        << (session ? session->getLocalAddress()->toString() : "<unknown>") << " remote address "
			        << (session ? session->getRemoteAddress()->toString() : "<unknown>") << ") has ended";
			endConference();
			break;
		default:
			break;
	}

	const auto &confState = ref->getState();
	auto ms = static_pointer_cast<MediaSession>(session);
	// Do not send updates if the conference is ending or about to end or the call is in temporary state
	if ((confState != ConferenceInterface::State::Terminated) &&
	    (confState != ConferenceInterface::State::TerminationPending) && scheduleUpdate &&
	    ms->getPrivate()->canSoundResourcesBeFreed()) {
		lInfo() << "Executing scheduled update of the focus session of conference "
		        << (getConferenceAddress() ? getConferenceAddress()->toString() : std::string("<address-not-defined>"));
		if (updateMainSession() == 0) {
			scheduleUpdate = false;
		} else {
			lInfo() << "Scheduled update of the focus session of conference "
			        << (getConferenceAddress() ? getConferenceAddress()->toString()
			                                   : std::string("<address-not-defined>"))
			        << " cannot be executed right now - Retrying at the next state change";
		}
	}
}

void RemoteConference::onConferenceTerminated(const std::shared_ptr<Address> &addr) {
	auto session = getMainSession();
	std::shared_ptr<Call> call = nullptr;
	SalCallOp *op = nullptr;

	if (session) {
		op = session->getPrivate()->getOp();
		call = op ? getCore()->getCallByCallId(op->getCallId()) : nullptr;
	}

	if (call) call->setConference(nullptr);

	Conference::onConferenceTerminated(addr);
}

void RemoteConference::endConference() {
	// Take a ref as conference may be deleted
	shared_ptr<Conference> ref = getSharedFromThis();
	setState(ConferenceInterface::State::TerminationPending);
	if (!finalized) {
		Conference::terminate();
		setState(ConferenceInterface::State::Terminated);
	}
}

void RemoteConference::onPendingCallStateChanged(std::shared_ptr<LinphonePrivate::Call> call,
                                                 LinphoneCallState callState) {
	auto session = getMainSession();
	switch (callState) {
		case LinphoneCallStreamsRunning:
		case LinphoneCallPaused: {
			if (session) {
				CallSession::State focusCallState = session->getState();
				// Transfer call to focus is the conference is in creation pending or created state and if the focus
				// call is in connected, streams running or updated by remote state
				if (((focusCallState == CallSession::State::StreamsRunning) ||
				     (focusCallState == CallSession::State::Connected) ||
				     (focusCallState == CallSession::State::UpdatedByRemote)) &&
				    ((state == ConferenceInterface::State::CreationPending) ||
				     (state == ConferenceInterface::State::Created))) {
					// Transfer call only if focus call remote contact address is available (i.e. the call has been
					// correctly established and passed through state StreamsRunning)
					if (session->getRemoteContactAddress()) {
						m_pendingCalls.remove(call);
						transferToFocus(call);
					}
				}
			}
		} break;
		case LinphoneCallError:
		case LinphoneCallEnd:
			m_pendingCalls.remove(call);
			Conference::removeParticipant(call);
			if ((participants.size() + m_pendingCalls.size() + m_transferingCalls.size()) == 0) terminate();
			break;
		default:
			break;
	}
}

void RemoteConference::onTransferingCallStateChanged(std::shared_ptr<LinphonePrivate::Call> transfered,
                                                     LinphoneCallState newCallState) {
	switch (newCallState) {
		case LinphoneCallConnected:
			m_transferingCalls.push_back(transfered);
			break;
		case LinphoneCallError:
			m_transferingCalls.remove(transfered);
			Conference::removeParticipant(transfered);
			if ((participants.size() + m_pendingCalls.size() + m_transferingCalls.size()) == 0) terminate();
			break;
		default:
			break;
	}
}

void RemoteConference::callStateChangedCb(LinphoneCore *lc,
                                          LinphoneCall *call,
                                          LinphoneCallState cstate,
                                          BCTBX_UNUSED(const char *message)) {
	LinphoneCoreVTable *vtable = linphone_core_get_current_vtable(lc);
	RemoteConference *conf = (RemoteConference *)linphone_core_v_table_get_user_data(vtable);
	if (Call::toCpp(call)->getActiveSession() == conf->getMainSession()) conf->onFocusCallStateChanged(cstate);
	else {
		list<std::shared_ptr<LinphonePrivate::Call>>::iterator it =
		    find(conf->m_pendingCalls.begin(), conf->m_pendingCalls.end(), Call::toCpp(call)->getSharedFromThis());
		if (it != conf->m_pendingCalls.end())
			conf->onPendingCallStateChanged(Call::toCpp(call)->getSharedFromThis(), cstate);
	}
}

void RemoteConference::transferStateChangedCb(LinphoneCore *lc,
                                              LinphoneCall *transfered,
                                              LinphoneCallState new_call_state) {
	LinphoneCoreVTable *vtable = linphone_core_get_current_vtable(lc);
	RemoteConference *conf = (RemoteConference *)linphone_core_v_table_get_user_data(vtable);
	list<std::shared_ptr<LinphonePrivate::Call>>::iterator it = find(
	    conf->m_transferingCalls.begin(), conf->m_transferingCalls.end(), Call::toCpp(transfered)->getSharedFromThis());
	if (it != conf->m_transferingCalls.end())
		conf->onTransferingCallStateChanged(Call::toCpp(transfered)->getSharedFromThis(), new_call_state);
}

AudioControlInterface *RemoteConference::getAudioControlInterface() const {
	auto session = getMainSession();
	if (!session) return nullptr;
	auto ms = static_pointer_cast<MediaSession>(session);
	return ms->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
}

VideoControlInterface *RemoteConference::getVideoControlInterface() const {
	auto session = getMainSession();
	if (!session) return nullptr;
	auto ms = static_pointer_cast<MediaSession>(session);
	return ms->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
}

AudioStream *RemoteConference::getAudioStream() {
	auto session = getMainSession();
	if (!session) return nullptr;
	auto ms = static_pointer_cast<MediaSession>(session);
	MS2AudioStream *stream = ms->getStreamsGroup().lookupMainStreamInterface<MS2AudioStream>(SalAudio);
	return stream ? (AudioStream *)stream->getMediaStream() : nullptr;
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void RemoteConference::multipartNotifyReceived(const std::shared_ptr<Event> &notifyLev, const Content &content) {
#ifdef HAVE_ADVANCED_IM
	if (eventHandler) {
		eventHandler->multipartNotifyReceived(notifyLev, content);
		return;
	}
#endif // HAVE_ADVANCED_IM
	lInfo() << "Unable to handle multi part NOTIFY because conference event package (RFC 4575) is disabled or the SDK "
	           "was not compiled with ENABLE_ADVANCED_IM flag set to on";
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void RemoteConference::notifyReceived(const std::shared_ptr<Event> &notifyLev, const Content &content) {
#ifdef HAVE_ADVANCED_IM
	if (eventHandler) {
		eventHandler->notifyReceived(notifyLev, content);
		return;
	}
#endif // HAVE_ADVANCED_IM
	lInfo() << "Unable to handle NOTIFY because conference event package (RFC 4575) is disabled or the SDK was not "
	           "compiled with ENABLE_ADVANCED_IM flag set to on";
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

void RemoteConference::onStateChanged(LinphonePrivate::ConferenceInterface::State state) {
	auto session = getMainSession();
	string subject = getSubject();

	shared_ptr<Call> sessionCall = nullptr;
	if (session) {
		auto op = session->getPrivate()->getOp();
		sessionCall = op ? getCore()->getCallByCallId(op->getCallId()) : nullptr;
	}

	switch (state) {
		case ConferenceInterface::State::None:
		case ConferenceInterface::State::Instantiated:
		case ConferenceInterface::State::CreationPending:
		case ConferenceInterface::State::TerminationFailed:
		case ConferenceInterface::State::Terminated:
			break;
		case ConferenceInterface::State::CreationFailed:
			reset();
			Conference::terminate();
			break;
		case ConferenceInterface::State::Created:
			if (session && getMe()->isAdmin() && (subject.compare(pendingSubject) != 0)) {
				lInfo() << "Updating main session to set conference subject " << subject;
				session->update(nullptr, CallSession::UpdateMethod::Default, subject);
			}
			break;
		case ConferenceInterface::State::TerminationPending:
#ifdef HAVE_ADVANCED_IM
			if (eventHandler) {
				eventHandler->unsubscribe();
			}
#endif // HAVE_ADVANCED_IM
			resetLastNotify();
			if (session) {
				// Do not terminate focus call when terminating the remote conference
				// This is required because the local conference creates a remote conference for every participant and
				// the call from the participant to the local conference is the focus call
				if (sessionCall) {
					sessionCall->setConference(nullptr);
				}
			}
			Conference::terminate();
			setState(ConferenceInterface::State::Terminated);
			break;
			break;
		case ConferenceInterface::State::Deleted:
			reset();
			break;
	}
}

void RemoteConference::setParticipantAdminStatus(const shared_ptr<Participant> &participant, bool isAdmin) {
	if (isAdmin == participant->isAdmin()) return;

	if (!getMe()->isAdmin()) {
		lError() << "Unable to set admin status of participant " << participant->getAddress()->toString() << " to "
		         << (isAdmin ? "true" : "false") << " because focus " << getMe()->getAddress()->toString()
		         << " is not admin";
		return;
	}

	LinphoneCore *cCore = getCore()->getCCore();
	auto session = getMainSession();

	SalReferOp *referOp = new SalReferOp(cCore->sal.get());
	LinphoneAddress *lAddr = linphone_address_new(session->getRemoteContact().c_str());
	linphone_configure_op(cCore, referOp, lAddr, nullptr, false);
	linphone_address_unref(lAddr);
	std::shared_ptr<Address> referToAddr = participant->getAddress();
	referToAddr->setParam("admin", Utils::toString(isAdmin));
	referOp->sendRefer(referToAddr->getImpl());
	referOp->unref();
}

void RemoteConference::setSubject(const std::string &subject) {
	if (!getMe()->isAdmin()) {
		lError() << "Unable to update conference subject because focus " << getMe()->getAddress()->toString()
		         << " is not admin";
		return;
	}

	auto session = static_pointer_cast<MediaSession>(getMainSession());
	if (session) {
		if (subject.compare(pendingSubject) != 0) {
			pendingSubject = subject;
			auto updateSubject = [this, subject]() -> LinphoneStatus {
				auto session = static_pointer_cast<MediaSession>(getMainSession());
				if (session) {
					lInfo() << "Sending re-INVITE to update subject from \"" << getSubject() << "\" to \"" << subject
					        << "\"";
					const MediaSessionParams *params = session->getMediaParams();
					MediaSessionParams *currentParams = params->clone();
					auto ret = session->update(currentParams, CallSession::UpdateMethod::Default, false, subject);
					delete currentParams;
					if (ret != 0) {
						lInfo() << "re-INVITE to update subject to \"" << subject << "\" cannot be sent right now";
					}
					return ret;
				}
				return -1;
			};

			if (updateSubject() != 0) {
				session->addPendingAction(updateSubject);
			}
		}
	} else {
		pendingSubject = subject;
		lInfo() << "Unable to update subject to \"" << subject
		        << "\" right now because the focus session has not been established yet.";
	}
}

bool RemoteConference::update(const LinphonePrivate::ConferenceParamsInterface &newParameters) {
	// Any remote participant can change the layout of the conference
	bool ret = false;
	if (getMe()->isAdmin()) {
		ret = Conference::update(newParameters);
	} else {
		lError() << "Unable to update conference parameters because focus " << getMe()->getAddress()->toString()
		         << " is not admin";
	}
	return ret;
}

std::shared_ptr<Call> RemoteConference::getCall() const {
	auto session = getMainSession();
	if (session) {
		return getCore()->getCallByRemoteAddress(session->getRemoteAddress());
	}
	return nullptr;
}

void RemoteConference::onParticipantAdded(const shared_ptr<ConferenceParticipantEvent> &event,
                                          const std::shared_ptr<Participant> &participant) {
	const std::shared_ptr<Address> &pAddr = event->getParticipantAddress();

	lInfo() << "Updating conference information of conference " << *getConferenceAddress()
	        << " because the core has been notified that participat " << *participant->getAddress()
	        << " has been added";
	// When receiving a participant added notification, we must recreate the conference informations in order to get the
	// participant list up to date
	updateAndSaveConferenceInformations();

	if (isMe(pAddr)) {
		if (getState() == ConferenceInterface::State::CreationPending) {
#ifdef HAVE_ADVANCED_IM
			bool_t eventLogEnabled = linphone_config_get_bool(linphone_core_get_config(getCore()->getCCore()), "misc",
			                                                  "conference_event_log_enabled", TRUE);
			if (eventLogEnabled) {
				if (!eventHandler) {
					eventHandler = std::make_shared<RemoteConferenceEventHandler>(this, this);
				}
				lInfo() << "Subscribing me (address " << *pAddr << ") to conference " << *getConferenceAddress();
				eventHandler->subscribe(getConferenceId());
			} else {
#endif // HAVE_ADVANCED_IM
				lInfo() << "Unable to send SUBSCRIBE following me " << *participant->getAddress()
				        << " being added because conference event package (RFC 4575) is disabled or the SDK was not "
				           "compiled with ENABLE_ADVANCED_IM flag set to on";
#ifdef HAVE_ADVANCED_IM
			}
#endif // HAVE_ADVANCED_IM
		}
	} else if (findParticipant(pAddr)) {
		lInfo() << "Addition of participant with address " << *pAddr << " to conference " << *getConferenceAddress()
		        << " has been successful";
	} else {
		lWarning() << "Addition of participant with address " << *pAddr
		           << " has been failed because the participant is not part of the conference"
		           << *getConferenceAddress();
	}
}

void RemoteConference::onSubjectChanged(BCTBX_UNUSED(const std::shared_ptr<ConferenceSubjectEvent> &event)) {
	lInfo() << "Updating conference information of conference " << *getConferenceAddress()
	        << " because the core has been notified that the subject has been changed to " << getSubject();
	updateAndSaveConferenceInformations();
}

void RemoteConference::onParticipantSetRole(BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantEvent> &event),
                                            const std::shared_ptr<Participant> &participant) {
	lInfo() << "Updating conference information of conference " << *getConferenceAddress()
	        << " because the core has been notified that participat " << *participant->getAddress()
	        << " has changed its role to " << participant->getRole();
	updateAndSaveConferenceInformations();
}

void RemoteConference::onParticipantRemoved(const shared_ptr<ConferenceParticipantEvent> &event,
                                            BCTBX_UNUSED(const std::shared_ptr<Participant> &participant)) {
	const std::shared_ptr<Address> &pAddr = event->getParticipantAddress();
	if (isMe(pAddr)) {
		lInfo() << "Unsubscribing all devices of me (address " << *pAddr << ") from conference "
		        << *getConferenceAddress();
		// Unsubscribe all devices of me
		std::for_each(getMe()->getDevices().cbegin(), getMe()->getDevices().cend(),
		              [&](const std::shared_ptr<ParticipantDevice> &device) {
			              shared_ptr<EventSubscribe> ev = device->getConferenceSubscribeEvent();
			              if (ev) {
				              // try to terminate subscription if any, but do not wait for answer.
				              ev->clearCallbacksList();
				              ev->terminate();
			              }
		              });
	} else if (!findParticipant(pAddr)) {
		lInfo() << "Removal of participant with address " << *pAddr << " from conference " << *getConferenceAddress()
		        << " has been successful";
	} else {
		lWarning() << "Removal of participant with address " << *pAddr
		           << " has been failed because the participant is still part of the conference"
		           << *getConferenceAddress();
	}
}

void RemoteConference::onParticipantDeviceAdded(
    BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event),
    const std::shared_ptr<ParticipantDevice> &device) {
	auto session = static_pointer_cast<MediaSession>(getMainSession());
	if (isMe(device->getAddress())) {
		notifyLocalMutedDevices(session->getPrivate()->getMicrophoneMuted());
	}
}

void RemoteConference::onParticipantDeviceRemoved(
    BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event),
    const std::shared_ptr<ParticipantDevice> &device) {
	auto session = static_pointer_cast<MediaSession>(getMainSession());
	const MediaSessionParams *params = session->getMediaParams();

	const auto &deviceAddress = device->getAddress();
	const auto &confSecurityLevel = confParams->getSecurityLevel();
	const auto &audioAvailable = device->getStreamAvailability(LinphoneStreamTypeAudio);
	const auto audioNeedsReInvite = ((confSecurityLevel == ConferenceParams::SecurityLevel::EndToEnd) &&
	                                 confParams->audioEnabled() && params->audioEnabled() && audioAvailable);

	const auto videoNeedsReInvite = (confParams->videoEnabled() && params->videoEnabled());

	if ((audioNeedsReInvite || videoNeedsReInvite) && (getState() == ConferenceInterface::State::Created) &&
	    !isMe(deviceAddress) && (device->getTimeOfJoining() >= 0)) {
		auto updateSession = [this, deviceAddress]() -> LinphoneStatus {
			lInfo() << "Sending re-INVITE in order to update streams because participant device " << *deviceAddress
			        << " has been removed from conference " << *getConferenceAddress();
			auto ret = updateMainSession();
			if (ret != 0) {
				lInfo() << "re-INVITE to update streams because participant device " << *deviceAddress
				        << " has been removed from conference " << *getConferenceAddress()
				        << " cannot be sent right now";
			}
			return ret;
		};

		if (updateSession() != 0) {
			scheduleUpdate = true;
		}
	}
}

void RemoteConference::onParticipantDeviceStateChanged(
    BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event),
    const std::shared_ptr<ParticipantDevice> &device) {
	auto session = static_pointer_cast<MediaSession>(getMainSession());
	const MediaSessionParams *params = session->getMediaParams();

	auto callIt = std::find_if(m_pendingCalls.cbegin(), m_pendingCalls.cend(), [&device](const auto &call) {
		if (!call) return false;
		const auto &devAddr = device->getAddress();
		const auto contactAddress = call->getActiveSession()->getRemoteContactAddress()->getUri();
		return (*devAddr == contactAddress);
	});

	const auto &deviceAddress = device->getAddress();
	const auto &audioAvailable = device->getStreamAvailability(LinphoneStreamTypeAudio);
	const auto &confSecurityLevel = confParams->getSecurityLevel();
	const auto audioNeedsReInvite = ((confSecurityLevel == ConferenceParams::SecurityLevel::EndToEnd) &&
	                                 confParams->audioEnabled() && params->audioEnabled() && audioAvailable);
	const auto &videoAvailable = device->getStreamAvailability(LinphoneStreamTypeVideo);
	const auto videoNeedsReInvite = (confParams->videoEnabled() && params->videoEnabled() && videoAvailable);
	if ((getState() == ConferenceInterface::State::Created) && (callIt == m_pendingCalls.cend()) && isIn() &&
	    (device->getState() == ParticipantDevice::State::Present) && ((videoNeedsReInvite || audioNeedsReInvite)) &&
	    !isMe(deviceAddress)) {
		auto updateSession = [this, deviceAddress]() -> LinphoneStatus {
			lInfo() << "Sending re-INVITE in order to get streams for participant device " << *deviceAddress
			        << " that joined recently the conference " << *getConferenceAddress();
			auto ret = updateMainSession();
			if (ret != 0) {
				lInfo() << "re-INVITE to get streams for participant device " << *deviceAddress
				        << " that recently joined the conference " << *getConferenceAddress()
				        << " cannot be sent right now";
			}
			return ret;
		};

		if (updateSession() != 0) {
			scheduleUpdate = true;
		}
	}
}

void RemoteConference::onParticipantDeviceMediaAvailabilityChanged(
    BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event),
    const std::shared_ptr<ParticipantDevice> &device) {
	if ((!isMe(device->getAddress())) && (getState() == ConferenceInterface::State::Created) && isIn()) {
		auto updateSession = [this, device]() -> LinphoneStatus {
			lInfo() << "Sending re-INVITE because device " << *device->getAddress()
			        << " has changed its stream availability";
			auto ret = updateMainSession();
			if (ret != 0) {
				lInfo() << "re-INVITE due to device " << *device->getAddress()
				        << " changing its stream availability cannot be sent right now";
			}
			return ret;
		};

		if (updateSession() != 0) {
			scheduleUpdate = true;
		}
	}
}

void RemoteConference::onFullStateReceived() {
	// When receiving a full state, we must recreate the conference informations in order to get the security level and
	// the participant list up to date
	updateAndSaveConferenceInformations();

	auto requestStreams = [this]() -> LinphoneStatus {
		lInfo() << "Sending re-INVITE in order to get streams after joining conference " << *getConferenceAddress();
		setState(ConferenceInterface::State::Created);
		auto ret = updateMainSession();
		return ret;
	};

	auto session = static_pointer_cast<MediaSession>(focus->getSession());
	// Notify local participant that the microphone is muted when receiving the full state as participants are added as
	// soon as possible
	if (session) {
		notifyLocalMutedDevices(session->getPrivate()->getMicrophoneMuted());
	}
	if (session && (!session->mediaInProgress() || !session->getPrivate()->isUpdateSentWhenIceCompleted())) {
		if (requestStreams() != 0) {
			scheduleUpdate = true;
		}
	} else {
		lInfo() << "Delaying re-INVITE in order to get streams after joining conference " << *getConferenceAddress()
		        << " because ICE negotiations didn't end yet";
	}

	fullStateReceived = true;
}

void RemoteConference::onAvailableMediaChanged(
    BCTBX_UNUSED(const std::shared_ptr<ConferenceAvailableMediaEvent> &event)) {
	auto session = static_pointer_cast<MediaSession>(getMainSession());
	const bool videoEnabled = (session) ? session->getCurrentParams()->videoEnabled() : false;
	if (!confParams->videoEnabled() && videoEnabled && isIn()) {
		auto updateSession = [this]() -> LinphoneStatus {
			lInfo() << "Sending re-INVITE because the conference has no longer video capabilities";
			auto ret = updateMainSession();
			if (ret != 0) {
				lInfo() << "Sending re-INVITE because the conference has no longer video capabilities";
				lInfo() << "re-INVITE to remove video cannot be sent right now";
			}
			return ret;
		};

		if (updateSession() != 0) {
			scheduleUpdate = true;
		}
	}
}

void RemoteConference::onParticipantsCleared() {
	clearParticipants();
}

void RemoteConference::notifyDisplayedSpeaker(uint32_t csrc) {
	displayedSpeaker = csrc;

	if (csrc != 0) {
		auto device = findParticipantDeviceBySsrc(csrc, LinphoneStreamTypeVideo);

		if (device) {
			notifyActiveSpeakerParticipantDevice(device);
			lastNotifiedSsrc = csrc;
		} else {
			lError() << "Conference [" << this << "]: Active speaker changed with csrc: " << csrc
			         << " but it does not correspond to any participant device";
		}
	} else {
		if (louderSpeaker != lastNotifiedSsrc) notifyLouderSpeaker(louderSpeaker);
	}
}

void RemoteConference::notifyLouderSpeaker(uint32_t ssrc) {
	louderSpeaker = ssrc;

	if (displayedSpeaker > 0) return;

	auto device = findParticipantDeviceBySsrc(ssrc, LinphoneStreamTypeAudio);

	if (device && !device->getStreamAvailability(LinphoneStreamTypeVideo)) {
		notifyActiveSpeakerParticipantDevice(device);
		lastNotifiedSsrc = ssrc;
	}
}

} // end of namespace MediaConference

LINPHONE_END_NAMESPACE
