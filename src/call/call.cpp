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

#include "bctoolbox/defs.h"

#include "c-wrapper/c-wrapper.h"
#include "call.h"
#include "chat/chat-room/abstract-chat-room.h"
#include "conference/client-conference.h"
#include "conference/conference.h"
#include "conference/params/media-session-params-p.h"
#include "conference/participant.h"
#include "conference/session/call-session-p.h"
#include "conference/session/media-session-p.h"
#include "core/core-p.h"
#include "factory/factory.h"
#include "logger/logger.h"
#include "player/call-player.h"
#include "sal/sal_media_description.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// =============================================================================
shared_ptr<CallSession> Call::getActiveSession() const {
	return mParticipant->getSession();
}

shared_ptr<AbstractChatRoom> Call::getChatRoom() {
	if ((getState() != CallSession::State::End) && (getState() != CallSession::State::Released)) {
		mChatRoom = getCore()->getOrCreateBasicChatRoom(getLocalAddress(), getRemoteAddress());
		if (mChatRoom) {
			lInfo() << "Setting call id [" << getLog()->getCallId() << "] to ChatRoom [" << mChatRoom << "]";
			mChatRoom->setCallId(getLog()->getCallId());
		}
	}
	return mChatRoom;
}

const shared_ptr<Account> &Call::getDestAccount() const {
	return getActiveSession()->getPrivate()->getDestAccount();
}

/* This a test-only method.*/
IceSession *Call::getIceSession() const {
	return static_pointer_cast<MediaSession>(getActiveSession())->getPrivate()->getIceSession();
	return nullptr;
}

const LinphoneStreamInternalStats *Call::getStreamInternalStats(LinphoneStreamType type) const {
	return getMediaSession()->getPrivate()->getStreamInternalStats(type);
}

std::shared_ptr<MediaSession> Call::getMediaSession() const {
	return static_pointer_cast<MediaSession>(getActiveSession());
}

MediaStream *Call::getMediaStream(LinphoneStreamType type) const {
	auto ms = static_pointer_cast<MediaSession>(getActiveSession())->getPrivate();
	StreamsGroup &sg = ms->getStreamsGroup();
	MS2Stream *s = nullptr;
	switch (type) {
		case LinphoneStreamTypeAudio:
			s = sg.lookupMainStreamInterface<MS2Stream>(SalAudio);
			break;
		case LinphoneStreamTypeVideo:
			s = sg.lookupMainStreamInterface<MS2Stream>(SalVideo);
			break;
		case LinphoneStreamTypeText:
			s = sg.lookupMainStreamInterface<MS2Stream>(SalText);
			break;
		default:
			break;
	}
	if (!s) {
		// lError() << "CallPrivate::getMediaStream() : no stream with type " << type;
		return nullptr;
	}
	return s->getMediaStream();
}

int Call::getMediaStreamIndex(LinphoneStreamType type) const {
	auto ms = static_pointer_cast<MediaSession>(getActiveSession())->getPrivate();
	StreamsGroup &sg = ms->getStreamsGroup();
	Stream *s = nullptr;
	switch (type) {
		case LinphoneStreamTypeAudio:
			s = sg.lookupMainStream(SalAudio);
			break;
		case LinphoneStreamTypeVideo:
			s = sg.lookupMainStream(SalVideo);
			break;
		case LinphoneStreamTypeText:
			s = sg.lookupMainStream(SalText);
			break;
		default:
			break;
	}
	if (!s) {
		return -1;
	}
	return (int)s->getIndex();
}

size_t Call::getMediaStreamsNb(LinphoneStreamType type) const {
	size_t nb = 0;
	auto ms = static_pointer_cast<MediaSession>(getActiveSession())->getPrivate();
	StreamsGroup &sg = ms->getStreamsGroup();
	SalStreamType nType;
	switch (type) {
		case LinphoneStreamTypeAudio:
			nType = SalAudio;
			break;
		case LinphoneStreamTypeVideo:
			nType = SalVideo;
			break;
		case LinphoneStreamTypeText:
			nType = SalText;
			break;
		default:
			return nb;
	}
	for (auto &stream : sg.getStreams()) {
		if (stream && (stream->getType() == nType) && (stream->getState() == Stream::Running)) {
			nb++;
		}
	}
	return nb;
}

void Call::setEkt(const MSEKTParametersSet *ekt_params) const {
	getMediaSession()->setEkt(ekt_params);
}

SalCallOp *Call::getOp() const {
	return getActiveSession()->getPrivate()->getOp();
}

bool Call::getSpeakerMuted() const {
	return static_pointer_cast<MediaSession>(getActiveSession())->getPrivate()->getSpeakerMuted();
}

void Call::setSpeakerMuted(bool muted) {
	static_pointer_cast<MediaSession>(getActiveSession())->getPrivate()->setSpeakerMuted(muted);
}

bool Call::getMicrophoneMuted() const {
	return static_pointer_cast<MediaSession>(getActiveSession())->getPrivate()->getMicrophoneMuted();
}

void Call::setMicrophoneMuted(bool muted) {
	static_pointer_cast<MediaSession>(getActiveSession())->getPrivate()->setMicrophoneMuted(muted);
}

shared_ptr<CallStats> Call::getPrivateStats(LinphoneStreamType type) const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getPrivate()->getStats(type);
}

// =============================================================================

void Call::initiateIncoming() {
	getActiveSession()->initiateIncoming();
	// initiateIncoming creates the Streams (through makeLocalMediaDescription())
	// the configuration of sound devices must be done after streams are created.
	configureSoundCardsFromCore(nullptr);
}

bool Call::initiateOutgoing(const string &subject, const std::shared_ptr<const Content> content) {
	shared_ptr<CallSession> session = getActiveSession();
	session->addListener(getSharedFromThis());
	bool defer = session->initiateOutgoing(subject, content);
	// initiateOutgoing creates the Streams (through makeLocalMediaDescription())
	// the configuration of sound devices must be done after streams are created.
	configureSoundCardsFromCore(static_cast<const MediaSessionParams *>(getActiveSession()->getParams()));
	return defer;
}

void Call::iterate(time_t currentRealTime, bool oneSecondElapsed) {
	getActiveSession()->iterate(currentRealTime, oneSecondElapsed);
}

void Call::notifyRinging() {
	if (getState() == CallSession::State::IncomingReceived) {
		getActiveSession()->getPrivate()->handleIncoming(true);
	}
}

void Call::startIncomingNotification() {
	auto ref = getSharedFromThis();
	shared_ptr<CallSession> session = getActiveSession();
	session->addListener(ref);
	session->startIncomingNotification();
}

void Call::startPushIncomingNotification() {
	getActiveSession()->startPushIncomingNotification();
}

void Call::startBasicIncomingNotification() {
	auto ref = getSharedFromThis();
	shared_ptr<CallSession> session = getActiveSession();
	session->addListener(ref);
	session->startBasicIncomingNotification();
}

void Call::pauseForTransfer() {
	static_pointer_cast<MediaSession>(getActiveSession())->getPrivate()->pauseForTransfer();
}

int Call::startInvite(const std::shared_ptr<Address> &destination,
                      const std::string subject,
                      const std::shared_ptr<const Content> content) {
	return getActiveSession()->startInvite(destination, subject, content);
}

void Call::scheduleTransfer() {
	LinphoneCore *lc = getCore()->getCCore();
	shared_ptr<Call> self = getSharedFromThis();
	if (linphone_config_get_int(linphone_core_get_config(lc), "sip", "auto_accept_refer", 1)) {
		getCore()->doLater([self]() { self->executeTransfer(); });
	}
}

void Call::executeTransfer() {
	if (getMediaSession()->hasTransferPending()) {
		startReferredCall(getMediaSession()->getMediaParams());
	}
}

shared_ptr<Call> Call::startReferredCall(const MediaSessionParams *params) {
	// Create and initiate parameters before pausing the call because initialization of some settings requires knowledge
	// of the current call held by the core
	MediaSessionParams msp;
	if (params) msp = *params;
	else {
		msp.initDefault(getCore(), LinphoneCallOutgoing);
		msp.enableAudio(getCurrentParams()->audioEnabled());
		msp.enableVideo(getCurrentParams()->videoEnabled());
		msp.setAccount(getCurrentParams()->getAccount());
	}

	if (getState() != CallSession::State::Paused) {
		pauseForTransfer();
	}
	lInfo() << "Starting new call to referred address " << getActiveSession()->getReferTo();
	L_GET_PRIVATE(&msp)->setReferer(getActiveSession());
	L_GET_PRIVATE(getActiveSession())->setReferPending(false);
	LinphoneCallParams *lcp = L_GET_C_BACK_PTR(&msp);
	LinphoneCall *newCall =
	    linphone_core_invite_with_params(getCore()->getCCore(), getActiveSession()->getReferTo().c_str(), lcp);
	if (newCall) {
		Call::toCpp(newCall)->getMediaSession()->getPrivate()->setEncryptionStatus(
		    getMediaSession()->getPrivate()->getEncryptionStatus());
		getActiveSession()->getPrivate()->setTransferTarget(Call::toCpp(newCall)->getActiveSession());
		Call::toCpp(newCall)->getActiveSession()->getPrivate()->notifyReferState();
		Call::toCpp(newCall)->setMicrophoneMuted(getMicrophoneMuted());
		return Call::toCpp(newCall)->getSharedFromThis();
	}
	return nullptr;
}

// =============================================================================

void Call::createPlayer() {
	AudioStream *audioStream = reinterpret_cast<AudioStream *>(getMediaStream(LinphoneStreamTypeAudio));
	if (audioStream) mPlayer = CallPlayer::create<CallPlayer>(getCore(), audioStream);
}

// -----------------------------------------------------------------------------
void Call::terminateBecauseOfLostMedia() {
	lInfo() << "Call [" << this << "]: Media connectivity with " << *getRemoteAddress()
	        << " is lost, call is going to be terminated";
	static_pointer_cast<MediaSession>(getActiveSession())->terminateBecauseOfLostMedia();
}

bool Call::setInputAudioDevicePrivate(const std::shared_ptr<AudioDevice> &audioDevice) {
	if (!audioDevice) {
		lError() << "Unable to use audio device [" << audioDevice << "] as recording device";
		return false;
	}
	if ((audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Record)) == 0) {
		lError() << "Audio device [" << audioDevice << "] doesn't have Record capability";
		return false;
	}
	lInfo() << "Call's input audio device is " << audioDevice->getDeviceName();

	return static_pointer_cast<MediaSession>(getActiveSession())->setInputAudioDevice(audioDevice);
}

bool Call::setOutputAudioDevicePrivate(const std::shared_ptr<AudioDevice> &audioDevice) {
	if (!audioDevice) {
		lError() << "Unable to use audio device [" << audioDevice << "] as playback device";
		return false;
	}
	if ((audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Play)) == 0) {
		lError() << "Audio device [" << audioDevice << "] doesn't have Play capability";
		return false;
	}
	bool ret = static_pointer_cast<MediaSession>(getActiveSession())->setOutputAudioDevice(audioDevice);
	switch (getState()) {
		case CallSession::State::OutgoingRinging:
		case CallSession::State::Pausing:
		case CallSession::State::Paused:
		case CallSession::State::IncomingReceived:
			getCore()->getPrivate()->getToneManager().setOutputDevice(getActiveSession(), audioDevice);
			break;
		default:
			break;
	}

	return ret;
}

void Call::cleanupSessionAndUnrefCObjectCall() {
	linphone_call_unref(this->toC());
}

// -----------------------------------------------------------------------------

void Call::onAckBeingSent(BCTBX_UNUSED(const shared_ptr<CallSession> &session), LinphoneHeaders *headers) {
	linphone_call_notify_ack_processing(this->toC(), headers, false);
}

void Call::onAckReceived(BCTBX_UNUSED(const shared_ptr<CallSession> &session), LinphoneHeaders *headers) {
	linphone_call_notify_ack_processing(this->toC(), headers, true);
}

void Call::onBackgroundTaskToBeStarted(BCTBX_UNUSED(const shared_ptr<CallSession> &session)) {
	mBgTask.start(getCore(), 30);
}

void Call::onBackgroundTaskToBeStopped(BCTBX_UNUSED(const shared_ptr<CallSession> &session)) {
	mBgTask.stop();
}

void Call::onCallSessionAccepting(BCTBX_UNUSED(const shared_ptr<CallSession> &session)) {
	accept();
}

void Call::onCallSessionEarlyFailed(const shared_ptr<CallSession> &session, LinphoneErrorInfo *ei) {
	shared_ptr<CallLog> log = session->getLog();
	if (session->getState() == CallSession::State::PushIncomingReceived) {
		session->setStateToEnded();
	}

	getCore()->reportEarlyCallFailed(log->getDirection(), log->getFromAddress(), log->getToAddress(), ei,
	                                 log->getCallId());

	cleanupSessionAndUnrefCObjectCall();
}

void Call::onCallSessionSetReleased(BCTBX_UNUSED(const shared_ptr<CallSession> &session)) {
	getCore()->getPrivate()->removeReleasingCall(getSharedFromThis());
	cleanupSessionAndUnrefCObjectCall();
}

void Call::onCallSessionSetTerminated(BCTBX_UNUSED(const shared_ptr<CallSession> &session)) {
	if (getSharedFromThis() == getCore()->getCurrentCall()) {
		lInfo() << "Resetting the current call";
		getCore()->getPrivate()->setCurrentCall(nullptr);
	}

	auto ref = getSharedFromThis();

	// This is the last reference to the call therefore delete it as late as possible to ensure that also listener
	// methods notifying the termination of a call are called
	if (getCore()->getPrivate()->removeCall(ref) != 0) lError() << "Could not remove the call from the list!!!";

	LinphoneCore *core = getCore()->getCCore();
	if (!getCore()->getPrivate()->hasCalls()) ms_bandwidth_controller_reset_state(core->bw_controller);

	if (linphone_core_get_calls_nb(core) == 0) {
		getCore()->getPrivate()->notifySoundcardUsage(false);
		linphone_core_notify_last_call_ended(core);
	}

	getCore()->getPrivate()->addReleasingCall(ref);
}

void Call::reenterLocalConference(BCTBX_UNUSED(const shared_ptr<CallSession> &session)) {
	auto conference = getConference();
	if (conference) {
		auto confState = conference->getState();
		if (confState == ConferenceInterface::State::Created) {
			conference->enter();
		} else {
			lInfo() << "Unable to add participant because conference is in state " << Utils::toString(confState);
		}
	}
}

void Call::onCallSessionStateChanged(const shared_ptr<CallSession> &session,
                                     CallSession::State state,
                                     const string &message) {
	LinphoneCore *lc = getCore()->getCCore();
	const auto op = session->getPrivate()->getOp();

	bool remoteContactIsFocus = false;
	std::shared_ptr<Conference> conference = nullptr;
	if (op) {
		if (op->getRemoteContactAddress()) {
			Address remoteContactAddress;
			remoteContactAddress.setImpl(op->getRemoteContactAddress());
			remoteContactIsFocus = (remoteContactAddress.hasParam(Conference::IsFocusParameter));
		}

		if (!op->getTo().empty()) {
			const auto to = Address::create(op->getTo());
			conference = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->findConference(
			    ConferenceId(to, to, getCore()->createConferenceIdParams()), false);
		}
	}

	switch (state) {
		case CallSession::State::OutgoingInit:
		case CallSession::State::IncomingReceived:
			getPlatformHelpers(lc)->acquireWifiLock();
			getPlatformHelpers(lc)->acquireMcastLock();
			getPlatformHelpers(lc)->acquireCpuLock();
			if (linphone_core_get_calls_nb(lc) == 1) {
				linphone_core_notify_first_call_started(lc);
			}
			break;
		case CallSession::State::End:
			if (session->hasTransferPending()) scheduleTransfer();
			break;
		case CallSession::State::Released:
			getPlatformHelpers(lc)->releaseWifiLock();
			getPlatformHelpers(lc)->releaseMcastLock();
			getPlatformHelpers(lc)->releaseCpuLock();
			break;
		case CallSession::State::Paused:
			if (!getConference() && op && op->getRemoteContactAddress()) {
				if (!op->getTo().empty() && conference) {
					// This code is usually executed when the following scenario occurs:
					// - ICE is enabled
					// - during the ICE negotiations, the core receives a call, hence this one is paused
					// - once ICE negotiation are concluded, the call is updated and the call goes back to the previous
					// paused state
					tryToAddToConference(conference, session);
				}
			}
			if (session->hasTransferPending()) scheduleTransfer();
			break;
		case CallSession::State::UpdatedByRemote: {
			if (op && !getConference() && remoteContactIsFocus) {
				// Check if the request was sent by the focus (client conference)
				createClientConference(session);
				auto conference = getConference();
				if (conference && conference->getState() == ConferenceInterface::State::CreationPending) {
					conference->finalizeCreation();
				}
			}
		} break;
		case CallSession::State::Referred:
			/* This is somewhat redundant with call_state_changed(LinphoneCallStateReferred) notification */
			linphone_call_notify_refer_requested(this->toC(), session->getReferToAddress()->toC());
			if (session->hasTransferPending()) scheduleTransfer();
			break;
		case CallSession::State::Connected:
		case CallSession::State::StreamsRunning: {
			if (op && !getConference()) {
				if (!op->getTo().empty() && conference) {
					const auto &resourceList = op->getContentInRemote(ContentType::ResourceLists);
					if (!resourceList || resourceList.value().get().isEmpty()) {
						tryToAddToConference(conference, session);
					}
				} else if (op->getRemoteContactAddress()) {
					const auto &confId = session->getPrivate()->getConferenceId();
					// Check if the request was sent by the focus
					if (remoteContactIsFocus) {
						createClientConference(session);
					} else if (!confId.empty()) {
						auto localAddress = session->getContactAddress();
						if (localAddress && localAddress->isValid()) {
							ConferenceId serverConferenceId =
							    ConferenceId(localAddress, localAddress, getCore()->createConferenceIdParams());
							conference = getCore()->findConference(serverConferenceId, false);
							if (conference) {
								setConference(conference);
								reenterLocalConference(session);
								conference->addParticipantDevice(getSharedFromThis());
							}
						} else {
							lError() << "Call " << this << " cannot be added to conference with ID " << confId
							         << " because the contact address has not been retrieved";
						}
					}
				}
			}
		} break;
		default:
			break;
	}
	linphone_call_notify_state_changed(this->toC(), static_cast<LinphoneCallState>(state), message.c_str());
}

void Call::tryToAddToConference(shared_ptr<Conference> &conference, const shared_ptr<CallSession> &session) {
	// If the call is for a conference stored in the core, then add call to conference once ICE negotiations are
	// terminated
	if (!mediaInProgress() || !!!linphone_config_get_int(linphone_core_get_config(session->getCore()->getCCore()),
	                                                     "sip", "update_call_when_ice_completed", TRUE)) {
		const auto &device = conference->findParticipantDevice(session);
		if (device) {
			const auto &deviceState = device->getState();
			if ((deviceState == ParticipantDevice::State::Joining) ||
			    (deviceState == ParticipantDevice::State::ScheduledForJoining)) {
				// Participants complete their addition to a conference when the call goes back to the StreamsRunning
				// state
				conference->finalizeParticipantAddition(getSharedFromThis());
			}
		} else {
			auto conferenceInfo = conference->createOrGetConferenceInfo();
			if ((!Conference::isTerminationState(conference->getState())) &&
			    (conferenceInfo->getState() != ConferenceInfo::State::Cancelled)) {
				conference->addParticipant(getSharedFromThis());
			}
		}
	}
}

void Call::createClientConference(const shared_ptr<CallSession> &session) {
	// If the call is for a conference stored in the core, then add call to conference once ICE negotiations are
	// terminated
	const auto op = session->getPrivate()->getOp();
	std::shared_ptr<Address> remoteContactAddress = Address::create();
	remoteContactAddress->setImpl(op->getRemoteContactAddress());
	ConferenceId conferenceId =
	    ConferenceId(remoteContactAddress, getLocalAddress(), getCore()->createConferenceIdParams());

	const auto &conference = getCore()->findConference(conferenceId, false);

	std::shared_ptr<ClientConference> clientConference = nullptr;

	if (conference) {
		const auto &conferenceAddress = conference->getConferenceAddress();
		const auto conferenceAddressStr = (conferenceAddress ? conferenceAddress->toString() : std::string("sip:"));
		lInfo() << "Attaching call (local address " << *session->getLocalAddress() << " remote address "
		        << *session->getRemoteAddress() << ") to conference " << conference << " (address "
		        << conferenceAddressStr << ") ID " << conferenceId;
		clientConference = dynamic_pointer_cast<ClientConference>(conference);
		if (clientConference) {
			clientConference->attachCall(session);
		}
	} else {
		auto confParams = ConferenceParams::create(getCore());
		confParams->setAccount(getParams()->getAccount());
		std::shared_ptr<SalMediaDescription> md = (op) ? op->getFinalMediaDescription() : nullptr;

		if (op && op->getSal()->mediaDisabled()) md = op->getRemoteMediaDescription();

		if (md) {
			confParams->enableAudio(md->nbActiveStreamsOfType(SalAudio) > 0);
			confParams->enableVideo(md->nbActiveStreamsOfType(SalVideo) > 0);
		}
		confParams->enableChat(remoteContactAddress && remoteContactAddress->hasParam(Conference::TextParameter));

		if (confParams->audioEnabled() || confParams->videoEnabled() || confParams->chatEnabled()) {
			clientConference = dynamic_pointer_cast<ClientConference>(
			    (new ClientConference(getCore(), nullptr, confParams))->toSharedPtr());
			clientConference->initWithFocus(remoteContactAddress, session, op);
		} else {
			lError() << "Unable to attach call (local address " << *session->getLocalAddress() << " remote address "
			         << *session->getRemoteAddress()
			         << ") to a conference because it has all capabilities (audio, video and chat) disabled";
		}
	}

	setConference(clientConference);

	// Record conf-id to be used later when terminating the client conference
	if (clientConference && remoteContactAddress->hasUriParam(Conference::ConfIdParameter)) {
		setConferenceId(remoteContactAddress->getUriParamValue(Conference::ConfIdParameter));
	}
}

void Call::onCallSessionTransferStateChanged(BCTBX_UNUSED(const shared_ptr<CallSession> &session),
                                             CallSession::State state) {
	linphone_call_notify_transfer_state_changed(this->toC(), static_cast<LinphoneCallState>(state));
}

void Call::onCheckForAcceptation(BCTBX_UNUSED(const shared_ptr<CallSession> &session)) {
	// If the core is a conference server, there is no need to ensure that media resources are not shared
	if (!linphone_core_conference_server_enabled(getCore()->getCCore()) &&
	    (linphone_core_get_media_resource_mode(getCore()->getCCore()) != LinphoneSharedMediaResources)) {
		list<shared_ptr<Call>> calls = getCore()->getCalls();
		shared_ptr<Call> currentCall = getSharedFromThis();
		for (const auto &call : calls) {
			if (call == currentCall) continue;
			switch (call->getState()) {
				case CallSession::State::OutgoingInit:
				case CallSession::State::OutgoingProgress:
				case CallSession::State::OutgoingRinging:
				case CallSession::State::OutgoingEarlyMedia:
					lInfo() << "Already existing call [" << call << "] in state [" << Utils::toString(call->getState())
					        << "], canceling it before accepting new call [" << currentCall << "]";
					call->terminate();
					break;
				default:
					break; // Nothing to do
			}
		}
	}
}

void Call::onDtmfReceived(BCTBX_UNUSED(const shared_ptr<CallSession> &session), char dtmf) {
	linphone_call_notify_dtmf_received(this->toC(), dtmf);
}

void Call::onIncomingCallSessionNotified(BCTBX_UNUSED(const shared_ptr<CallSession> &session)) {
	/* The call is acceptable so we can now add it to our list */
	getCore()->getPrivate()->addCall(getSharedFromThis());
}

void Call::onIncomingCallSessionStarted(BCTBX_UNUSED(const shared_ptr<CallSession> &session)) {
	shared_ptr<Call> currentCall = getCore()->getCurrentCall();
	if (!currentCall && !isInConference()) {
		L_GET_PRIVATE_FROM_C_OBJECT(getCore()->getCCore())->setCurrentCall(getSharedFromThis());
	}
}

void Call::onIncomingCallSessionTimeoutCheck(BCTBX_UNUSED(const shared_ptr<CallSession> &session),
                                             int elapsed,
                                             bool oneSecondElapsed) {
	auto timeout = getCore()->getCCore()->sip_conf.inc_timeout;
	if (oneSecondElapsed) {
		lInfo() << "Incoming call " << this << " with session " << session << " (local address "
		        << *session->getLocalAddress() << " remote address " << *session->getRemoteAddress() << ") ringing for "
		        << elapsed << " seconds (timeout: " << timeout << ")";
	}
	if (elapsed > timeout) {
		lInfo() << "Incoming call " << this << " with session " << session << " (local address "
		        << *session->getLocalAddress() << " remote address " << *session->getRemoteAddress()
		        << ") timeout reached (" << timeout << ")";
		auto config = linphone_core_get_config(getCore()->getCCore());
		int statusCode = linphone_config_get_int(config, "sip", "inc_timeout_status_code", 486);
		getActiveSession()->declineNotAnswered(linphone_error_code_to_reason(statusCode));
	}
}

void Call::onPushCallSessionTimeoutCheck(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session), int elapsed) {
	if (elapsed > getCore()->getCCore()->sip_conf.push_incoming_call_timeout) {
		lInfo() << "Push incoming call timeout (" << getCore()->getCCore()->sip_conf.push_incoming_call_timeout << ")";
		auto config = linphone_core_get_config(getCore()->getCCore());
		int statusCode =
		    linphone_config_get_int(config, "sip", "push_incoming_call_timeout_status_code", 410); // LinphoneReasonGone
		getActiveSession()->decline(linphone_error_code_to_reason(statusCode));
	}
}

void Call::onInfoReceived(BCTBX_UNUSED(const shared_ptr<CallSession> &session), const LinphoneInfoMessage *im) {
	linphone_call_notify_info_message_received(this->toC(), im);
}

void Call::onLossOfMediaDetected(BCTBX_UNUSED(const shared_ptr<CallSession> &session)) {
	terminateBecauseOfLostMedia();
}

void Call::onSecurityLevelDowngraded(BCTBX_UNUSED(const shared_ptr<CallSession> &session)) {
	linphone_call_notify_security_level_downgraded(this->toC());
}

void Call::onEncryptionChanged(BCTBX_UNUSED(const shared_ptr<CallSession> &session),
                               bool activated,
                               const string &authToken) {
	linphone_call_notify_encryption_changed(this->toC(), activated, L_STRING_TO_C(authToken));
}

void Call::onAuthenticationTokenVerified(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session), bool verified) {
	linphone_call_notify_authentication_token_verified(this->toC(), verified);
}

void Call::onSendMasterKeyChanged(BCTBX_UNUSED(const shared_ptr<CallSession> &session), const string &masterKey) {
	linphone_call_notify_send_master_key_changed(this->toC(), L_STRING_TO_C(masterKey));
}

void Call::onReceiveMasterKeyChanged(BCTBX_UNUSED(const shared_ptr<CallSession> &session), const string &masterKey) {
	linphone_call_notify_receive_master_key_changed(this->toC(), L_STRING_TO_C(masterKey));
}

void Call::onGoClearAckSent() {
	linphone_call_notify_goclear_ack_sent(this->toC());
}

void Call::onCallSessionStateChangedForReporting(BCTBX_UNUSED(const shared_ptr<CallSession> &session)) {
	linphone_reporting_call_state_updated(this->toC());
}

void Call::onRtcpUpdateForReporting(BCTBX_UNUSED(const shared_ptr<CallSession> &session), SalStreamType type) {
	linphone_reporting_on_rtcp_update(this->toC(), type);
}

void Call::onStatsUpdated(BCTBX_UNUSED(const shared_ptr<CallSession> &session), const shared_ptr<CallStats> &stats) {
	linphone_call_notify_stats_updated(this->toC(), stats->toC());
}

void Call::onUpdateMediaInfoForReporting(BCTBX_UNUSED(const shared_ptr<CallSession> &session), int statsType) {
	linphone_reporting_update_media_info(this->toC(), statsType);
}

void Call::onResetCurrentSession(BCTBX_UNUSED(const shared_ptr<CallSession> &session)) {
	getCore()->getPrivate()->setCurrentCall(nullptr);
}

void Call::onSetCurrentSession(BCTBX_UNUSED(const shared_ptr<CallSession> &session)) {
	getCore()->getPrivate()->setCurrentCall(getSharedFromThis());
}

void Call::onFirstVideoFrameDecoded(BCTBX_UNUSED(const shared_ptr<CallSession> &session)) {
	if (mNextVideoFrameDecoded._func) {
		mNextVideoFrameDecoded._func(this->toC(), mNextVideoFrameDecoded._user_data);
		mNextVideoFrameDecoded._func = nullptr;
		mNextVideoFrameDecoded._user_data = nullptr;
	}
	linphone_call_notify_next_video_frame_decoded(this->toC());
}

void Call::onResetFirstVideoFrameDecoded(BCTBX_UNUSED(const shared_ptr<CallSession> &session)) {
	/*we are called here by the MediaSession when the stream start to know whether there is the deprecated
	 * nextVideoFrameDecoded callback set, so that we can request the notification of the next frame decoded.*/
#ifdef VIDEO_ENABLED
	if (mNextVideoFrameDecoded._func) requestNotifyNextVideoFrameDecoded();
#endif // ifdef VIDEO_ENABLED
}

void Call::requestNotifyNextVideoFrameDecoded() {
	static_pointer_cast<MediaSession>(getActiveSession())->requestNotifyNextVideoFrameDecoded();
}

void Call::onCameraNotWorking(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session), const char *camera_name) {
	linphone_call_notify_camera_not_working(this->toC(), camera_name);
}

void Call::onVideoDisplayErrorOccurred(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session), int error_code) {
	linphone_call_notify_video_display_error_occurred(this->toC(), error_code);
}

bool Call::areSoundResourcesAvailable(BCTBX_UNUSED(const shared_ptr<CallSession> &session)) {
	LinphoneCore *lc = getCore()->getCCore();
	shared_ptr<Call> currentCall = getCore()->getCurrentCall();
	// If core is in a conference, then check if the call is in the same conference
	// If the core left the conference or it is not hosting any conference, then check that there is no active call or
	// the active one is the current one.
	bool soundResourcesFree = linphone_core_is_in_conference(lc)
	                              ? (getConference() && (linphone_core_get_conference(lc) == getConference()->toC()))
	                              : (!currentCall || (currentCall == getSharedFromThis()));
	return soundResourcesFree;
}

bool Call::isPlayingRingbackTone(BCTBX_UNUSED(const shared_ptr<CallSession> &session)) {
	return mPlayingRingbackTone;
}

void Call::onRealTimeTextCharacterReceived(BCTBX_UNUSED(const shared_ptr<CallSession> &session),
                                           RealtimeTextReceivedCharacter *data) {
	shared_ptr<AbstractChatRoom> chatRoom = getChatRoom();
	if (chatRoom) {
		chatRoom->realtimeTextReceived(data->character, getSharedFromThis());
	} else {
		lError() << "CallPrivate::onRealTimeTextCharacterReceived: no chatroom.";
	}
}

#ifdef HAVE_BAUDOT
void Call::onBaudotCharacterReceived(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
                                     char receivedCharacter) {
	shared_ptr<AbstractChatRoom> chatRoom = getChatRoom();
	if (chatRoom) {
		chatRoom->baudotCharacterReceived(receivedCharacter, getSharedFromThis());
	} else {
		lError() << "Call::onBaudotCharacterReceived: no chatroom.";
	}
}

void Call::onBaudotDetected(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session), MSBaudotStandard standard) {
	linphone_call_notify_baudot_detected(
	    this->toC(), (standard == MSBaudotStandardEurope) ? LinphoneBaudotStandardEurope : LinphoneBaudotStandardUs);
}
#endif /* HAVE_BAUDOT */

void Call::onTmmbrReceived(BCTBX_UNUSED(const shared_ptr<CallSession> &session), int streamIndex, int tmmbr) {
	linphone_call_notify_tmmbr_received(this->toC(), streamIndex, tmmbr);
}

void Call::onSnapshotTaken(BCTBX_UNUSED(const shared_ptr<CallSession> &session), const char *file_path) {
	linphone_call_notify_snapshot_taken(this->toC(), file_path);
}

void Call::onStartRingtone(BCTBX_UNUSED(const shared_ptr<CallSession> &session)) {
	// Already handled by tone manager.
}

void Call::onRemoteRecording(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session), bool recording) {
	linphone_call_notify_remote_recording(this->toC(), recording);
}

std::unique_ptr<LogContextualizer> Call::getLogContextualizer() {
	return unique_ptr<LogContextualizer>(new CallLogContextualizer(*this));
}

// =============================================================================

Call::Call(std::shared_ptr<Core> core) : CoreAccessor(core) {
	mNextVideoFrameDecoded._func = nullptr;
	mNextVideoFrameDecoded._user_data = nullptr;

	mBgTask.setName("Liblinphone call notification");
}

void Call::configureSoundCardsFromCore(const MediaSessionParams *msp) {
	shared_ptr<AudioDevice> inputAudioDevice;
	shared_ptr<AudioDevice> outputAudioDevice;
	if (ms_snd_card_manager_reload_requested(ms_factory_get_snd_card_manager(getCore()->getCCore()->factory))) {
		linphone_core_reload_sound_devices(getCore()->getCCore());
	}
	if (msp) {
		setMicrophoneMuted(!msp->isMicEnabled());
		inputAudioDevice = msp->getInputAudioDevice();
		outputAudioDevice = msp->getOutputAudioDevice();
	}
	if (!outputAudioDevice) {
		outputAudioDevice = getCore()->getDefaultOutputAudioDevice();
		if (!outputAudioDevice && !getCore()->getCCore()->use_files) {
			lWarning() << "Failed to find audio device matching default output sound card ["
			           << getCore()->getCCore()->sound_conf.play_sndcard << "]";
		}
	}
	if (!inputAudioDevice) {
		inputAudioDevice = getCore()->getDefaultInputAudioDevice();
		if (!inputAudioDevice && !getCore()->getCCore()->use_files) {
			lWarning() << "Failed to find audio device matching default input sound card ["
			           << getCore()->getCCore()->sound_conf.capt_sndcard << "]";
		}
	}
	if (outputAudioDevice) setOutputAudioDevicePrivate(outputAudioDevice);
	if (inputAudioDevice) setInputAudioDevicePrivate(inputAudioDevice);
}

void Call::configure(LinphoneCallDir direction, const string &callid) {
	mParticipant = Participant::create();
	mParticipant->createSession(getCore(), nullptr, TRUE);
	mParticipant->getSession()->configure(direction, callid);
}

void Call::configure(LinphoneCallDir direction,
                     const std::shared_ptr<const Address> &from,
                     const std::shared_ptr<const Address> &to,
                     const std::shared_ptr<Account> &account,
                     SalCallOp *op,
                     BCTBX_UNUSED(const MediaSessionParams *msp)) {

	if (!mParticipant) {
		// create session
		mParticipant = Participant::create(nullptr, ((direction == LinphoneCallIncoming) ? to : from));
		mParticipant->createSession(getCore(), msp, TRUE);
	}

	mParticipant->configure(nullptr, (direction == LinphoneCallIncoming) ? to : from);
	mParticipant->getSession()->configure(direction, account, op, from, to);
}

bool Call::isOpConfigured() const {
	return getActiveSession()->isOpConfigured();
}

// =============================================================================

LinphoneStatus Call::accept(const MediaSessionParams *msp) {
	return static_pointer_cast<MediaSession>(getActiveSession())->accept(msp);
}

LinphoneStatus Call::acceptEarlyMedia(const MediaSessionParams *msp) {
	return static_pointer_cast<MediaSession>(getActiveSession())->acceptEarlyMedia(msp);
}

LinphoneStatus Call::acceptUpdate(const MediaSessionParams *msp) {
	return static_pointer_cast<MediaSession>(getActiveSession())->acceptUpdate(msp);
}

void Call::cancelDtmfs() {
	static_pointer_cast<MediaSession>(getActiveSession())->cancelDtmfs();
}

LinphoneStatus Call::decline(LinphoneReason reason) {
	return getActiveSession()->decline(reason);
}

LinphoneStatus Call::decline(const LinphoneErrorInfo *ei) {
	return getActiveSession()->decline(ei);
}

LinphoneStatus Call::deferUpdate() {
	return getActiveSession()->deferUpdate();
}

bool Call::hasTransferPending() const {
	return getActiveSession()->hasTransferPending();
}

void Call::oglRender() const {
	static_pointer_cast<MediaSession>(getActiveSession())->getPrivate()->oglRender();
}

LinphoneStatus Call::pause() {
	return static_pointer_cast<MediaSession>(getActiveSession())->pause();
}

bool Call::canSoundResourcesBeFreed() const {
	return static_pointer_cast<MediaSession>(getActiveSession())->getPrivate()->canSoundResourcesBeFreed();
}

LinphoneStatus Call::redirect(const string &redirectUri) {
	return getActiveSession()->redirect(redirectUri);
}

LinphoneStatus Call::redirect(const std::shared_ptr<Address> &redirectAddress) {
	return getActiveSession()->redirect(*redirectAddress);
}

LinphoneStatus Call::resume() {
	return static_pointer_cast<MediaSession>(getActiveSession())->resume();
}

LinphoneStatus Call::sendDtmf(char dtmf) {
	return static_pointer_cast<MediaSession>(getActiveSession())->sendDtmf(dtmf);
}

LinphoneStatus Call::sendDtmfs(const string &dtmfs) {
	return static_pointer_cast<MediaSession>(getActiveSession())->sendDtmfs(dtmfs);
}

void Call::sendVfuRequest() {
	static_pointer_cast<MediaSession>(getActiveSession())->sendVfuRequest();
}

void Call::updateRecordState(SalMediaRecord state) {
	if (linphone_core_is_record_aware_enabled(getCore()->getCCore()) && getRemoteParams()->recordAwareEnabled()) {
		if (getState() == CallSession::State::StreamsRunning) {
			MediaSessionParams params(*getParams());
			params.setRecordingState(state);
			static_pointer_cast<MediaSession>(getActiveSession())->update(&params, CallSession::UpdateMethod::Update);
		} else {
			lWarning() << "Recording cannot sent an update when the call is not in StreamRunning";
		}
	} else {
		MediaSessionParams *params = new MediaSessionParams(*getParams());
		params->setRecordingState(state);
		static_pointer_cast<MediaSession>(getActiveSession())->getPrivate()->setParams(params);
	}
}

void Call::startRecording() {
	if (static_pointer_cast<MediaSession>(getActiveSession())->startRecording()) {
		updateRecordState(SalMediaRecordOn);
	}
}

void Call::stopRecording() {
	static_pointer_cast<MediaSession>(getActiveSession())->stopRecording();
	updateRecordState(SalMediaRecordOff);
}

bool Call::isRecording() {
	return static_pointer_cast<MediaSession>(getActiveSession())->getMediaParams()->isRecording();
}

LinphoneStatus Call::takePreviewSnapshot(const string &file) {
	return static_pointer_cast<MediaSession>(getActiveSession())->takePreviewSnapshot(file);
}

LinphoneStatus Call::takeVideoSnapshot(const string &file) {
	return static_pointer_cast<MediaSession>(getActiveSession())->takeVideoSnapshot(file);
}

LinphoneStatus Call::terminate(const LinphoneErrorInfo *ei) {
	return getActiveSession()->terminate(ei);
}

LinphoneStatus Call::transfer(const shared_ptr<Call> &dest) {
	return getActiveSession()->transfer(dest->getActiveSession());
}

LinphoneStatus Call::transfer(const string &dest) {
	auto address = getCore()->interpretUrl(dest, true);
	return transfer(*address);
}

LinphoneStatus Call::transfer(const Address &dest) {
	LinphoneStatus ret = 0;
	int pauseBeforeTransfer =
	    linphone_config_get_int(linphone_core_get_config(getCore()->getCCore()), "sip", "pause_before_transfer", 0);
	if (pauseBeforeTransfer && getState() != CallSession::State::Paused) {
		lInfo() << *this << " must be paused before transfer.";
		ret = pause();
		if (ret == 0) {
			/* request a future action to be executed when reaching the Paused state */
			getActiveSession()->addPendingAction([this, dest]() {
				switch (getState()) {
					case CallSession::State::Paused:
						lInfo() << "Call is now paused, requesting the transfer.";
						getActiveSession()->transfer(dest);
						break;
					case CallSession::State::Pausing:
						return -1; // try again
						break;
					default:
						// unexpected
						lWarning() << "The call could not be paused, transfer request aborted.";
						break;
				}
				/* Even in failure case we return 0 because we don't want the action to be re-tried. */
				return 0;
			});
		}
	} else {
		ret = getActiveSession()->transfer(dest);
	}
	return ret;
}

LinphoneStatus Call::updateFromConference(const MediaSessionParams *msp) {
	return static_pointer_cast<MediaSession>(getActiveSession())->updateFromConference(msp);
}

LinphoneStatus Call::update(const MediaSessionParams *msp) {
	return static_pointer_cast<MediaSession>(getActiveSession())->update(msp);
}

void Call::zoomVideo(float zoomFactor, float *cx, float *cy) {
	zoomVideo(zoomFactor, *cx, *cy);
}

void Call::zoomVideo(float zoomFactor, float cx, float cy) {
	static_pointer_cast<MediaSession>(getActiveSession())->zoomVideo(zoomFactor, cx, cy);
}

// -----------------------------------------------------------------------------

bool Call::cameraEnabled() const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->cameraEnabled();
}

bool Call::echoCancellationEnabled() const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->echoCancellationEnabled();
}

bool Call::echoLimiterEnabled() const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->echoLimiterEnabled();
}

void Call::enableCamera(bool value) {
	static_pointer_cast<MediaSession>(getActiveSession())->enableCamera(value);
}

void Call::enableEchoCancellation(bool value) {
	static_pointer_cast<MediaSession>(getActiveSession())->enableEchoCancellation(value);
}

void Call::enableEchoLimiter(bool value) {
	static_pointer_cast<MediaSession>(getActiveSession())->enableEchoLimiter(value);
}

bool Call::getAllMuted() const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getAllMuted();
}

shared_ptr<CallStats> Call::getAudioStats() const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getAudioStats();
}

const string &Call::getAuthenticationToken() const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getAuthenticationToken();
}

void Call::forgeHalfAuthenticationToken(bool localHalfAuthToken) const {
	const string &authToken = getAuthenticationToken();
	if (authToken.empty()) return; // ZRTP disabled
	shared_ptr<CallStats> stats = getStats(LinphoneStreamTypeAudio);
	int sasAlgo = stats->getZrtpAlgo()->sas_algo;
	if (localHalfAuthToken) {
		if (sasAlgo == MS_ZRTP_SAS_B32) {
			if (getDirection() == LinphoneCallOutgoing) {
				mLocalAuthToken = authToken.substr(2, 2);
			} else {
				mLocalAuthToken = authToken.substr(0, 2);
			}
		} else {
			auto pos = authToken.find(':');
			if (getDirection() == LinphoneCallOutgoing) {
				mLocalAuthToken = authToken.substr(pos + 1);
			} else {
				mLocalAuthToken = authToken.substr(0, pos);
			}
		}
	} else {
		if (sasAlgo == MS_ZRTP_SAS_B32) {
			if (getDirection() == LinphoneCallOutgoing) {
				mRemoteAuthToken = authToken.substr(0, 2);
			} else {
				mRemoteAuthToken = authToken.substr(2, 2);
			}
		} else {
			auto pos = authToken.find(':');
			if (getDirection() == LinphoneCallOutgoing) {
				mRemoteAuthToken = authToken.substr(0, pos);
			} else {
				mRemoteAuthToken = authToken.substr(pos + 1);
			}
		}
	}
}

const string &Call::forgeLocalAuthenticationToken() const {
	if (mLocalAuthToken.empty()) {
		forgeHalfAuthenticationToken(true);
	}
	return mLocalAuthToken;
}

const string &Call::forgeRemoteAuthenticationToken() const {
	if (mRemoteAuthToken.empty()) {
		forgeHalfAuthenticationToken(false);
	}
	return mRemoteAuthToken;
}

void Call::storeAndSortRemoteAuthToken(const string &remoteAuthToken) const {
	static_pointer_cast<MediaSession>(getActiveSession())->storeAndSortRemoteAuthToken(remoteAuthToken);
}

const list<string> &Call::getRemoteAuthenticationTokens() const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getRemoteAuthenticationTokens();
}

const bctbx_list_t *Call::getCListRemoteAuthenticationTokens() const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getCListRemoteAuthenticationTokens();
}

void Call::skipZrtpAuthentication() {
	static_pointer_cast<MediaSession>(getActiveSession())->skipZrtpAuthentication();
}

bool Call::getAuthenticationTokenVerified() const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getAuthenticationTokenVerified();
}

bool Call::getZrtpCacheMismatch() const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getZrtpCacheMismatch();
}

float Call::getAverageQuality() const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getAverageQuality();
}

const MediaSessionParams *Call::getCurrentParams() const {
	return static_pointer_cast<MediaSession>(getActiveSession())->getCurrentParams();
}

float Call::getCurrentQuality() const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getCurrentQuality();
}

LinphoneCallDir Call::getDirection() const {
	return getActiveSession()->getDirection();
}

std::shared_ptr<const Address> Call::getDiversionAddress() const {
	mDiversionAddress = (new Address(getActiveSession()->getDiversionAddress()))->toSharedPtr();
	return mDiversionAddress;
}

int Call::getDuration() const {
	return getActiveSession()->getDuration();
}

const LinphoneErrorInfo *Call::getErrorInfo() const {
	return getActiveSession()->getErrorInfo();
}

const std::shared_ptr<Address> Call::getLocalAddress() const {
	return getActiveSession()->getLocalAddress();
}

shared_ptr<CallLog> Call::getLog() const {
	return getActiveSession()->getLog();
}

RtpTransport *Call::getMetaRtcpTransport(int streamIndex) const {
	return static_pointer_cast<MediaSession>(getActiveSession())->getMetaRtcpTransport(streamIndex);
}

RtpTransport *Call::getMetaRtpTransport(int streamIndex) const {
	return static_pointer_cast<MediaSession>(getActiveSession())->getMetaRtpTransport(streamIndex);
}

float Call::getMicrophoneVolumeGain() const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getMicrophoneVolumeGain();
}

void *Call::getNativeVideoWindowId() const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getNativeVideoWindowId();
}

void *Call::createNativeVideoWindowId() const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->createNativeVideoWindowId();
}

const MediaSessionParams *Call::getParams() const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getMediaParams();
}

std::shared_ptr<Player> Call::getPlayer() const {
	if (!mPlayer) const_cast<Call *>(this)->createPlayer();
	return mPlayer;
}

float Call::getPlayVolume() const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getPlayVolume();
}

LinphoneReason Call::getReason() const {
	return getActiveSession()->getReason();
}

float Call::getRecordVolume() const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getRecordVolume();
}

shared_ptr<Call> Call::getReferer() const {
	shared_ptr<CallSession> referer = getActiveSession()->getReferer();
	if (!referer) return nullptr;
	for (const auto &call : getCore()->getCalls()) {
		if (call->getActiveSession() == referer) return call;
	}
	return nullptr;
}

const string &Call::getReferTo() const {
	return getActiveSession()->getReferTo();
}

std::shared_ptr<Address> Call::getReferToAddress() const {
	auto session = getActiveSession();
	return session ? session->getReferToAddress() : nullptr;
}

std::shared_ptr<const Address> Call::getReferredBy() const {
	return getActiveSession()->getReferredBy();
}

std::shared_ptr<Address> Call::getRemoteAddress() const {
	auto session = getActiveSession();
	return session ? session->getRemoteAddress() : nullptr;
}

std::shared_ptr<Address> Call::getRemoteContactAddress() const {
	auto session = getActiveSession();
	return session ? session->getRemoteContactAddress() : nullptr;
}

const string &Call::getRemoteContact() const {
	return getActiveSession()->getRemoteContact();
}

const MediaSessionParams *Call::getRemoteParams() const {
	return static_pointer_cast<MediaSession>(getActiveSession())->getRemoteParams();
}

const string &Call::getRemoteUserAgent() {
	return getActiveSession()->getRemoteUserAgent();
}

shared_ptr<Call> Call::getReplacedCall() const {
	shared_ptr<CallSession> replacedCallSession = getActiveSession()->getReplacedCallSession();
	if (!replacedCallSession) return nullptr;
	for (const auto &call : getCore()->getCalls()) {
		if (call->getActiveSession() == replacedCallSession) return call;
	}
	return nullptr;
}

float Call::getSpeakerVolumeGain() const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getSpeakerVolumeGain();
}

CallSession::State Call::getState() const {
	return getActiveSession()->getState();
}

shared_ptr<CallStats> Call::getStats(LinphoneStreamType type) const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getStats(type);
}

int Call::getStreamCount() const {
	return static_pointer_cast<MediaSession>(getActiveSession())->getStreamCount();
}

MSFormatType Call::getStreamType(int streamIndex) const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getStreamType(streamIndex);
}

shared_ptr<CallStats> Call::getTextStats() const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getTextStats();
}

const std::shared_ptr<Address> Call::getToAddress() const {
	return getActiveSession()->getToAddress();
}

const char *Call::getToHeader(const string &name) const {
	return getActiveSession()->getToHeader(name);
}

CallSession::State Call::getTransferState() const {
	return getActiveSession()->getTransferState();
}

shared_ptr<Call> Call::getTransferTarget() const {
	shared_ptr<CallSession> transferTarget = getActiveSession()->getTransferTarget();
	if (!transferTarget) return nullptr;
	for (const auto &call : getCore()->getCalls()) {
		if (call->getActiveSession() == transferTarget) return call;
	}
	return nullptr;
}

std::shared_ptr<CallStats> Call::getVideoStats() const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getVideoStats();
}

// Boolean to state whether it is the focus of a local conference
bool Call::isInConference() const {
	return getActiveSession()->getPrivate()->isInConference();
}

void Call::setConferenceId(const std::string &conferenceId) {
	return getActiveSession()->getPrivate()->setConferenceId(conferenceId);
}

std::string Call::getConferenceId() const {
	return getActiveSession()->getPrivate()->getConferenceId();
}

bool Call::mediaInProgress() const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->mediaInProgress();
}

void Call::checkAuthenticationTokenSelected(const string &selectedValue) {
	auto remoteHalfAuthToken = forgeRemoteAuthenticationToken();
	static_pointer_cast<MediaSession>(getActiveSession())
	    ->checkAuthenticationTokenSelected(selectedValue, remoteHalfAuthToken);
}

void Call::setAuthenticationTokenVerified(bool value) {
	static_pointer_cast<MediaSession>(getActiveSession())->setAuthenticationTokenVerified(value);
}

void Call::setMicrophoneVolumeGain(float value) {
	static_pointer_cast<MediaSession>(getActiveSession())->setMicrophoneVolumeGain(value);
}

void Call::setNativeVideoWindowId(void *id) {
	static_pointer_cast<MediaSession>(getActiveSession())->setNativeVideoWindowId(id);
}

void Call::setNextVideoFrameDecodedCallback(LinphoneCallCbFunc cb, void *user_data) {
	mNextVideoFrameDecoded._func = cb;
	mNextVideoFrameDecoded._user_data = user_data;
	requestNotifyNextVideoFrameDecoded();
}

void Call::setParams(const MediaSessionParams *msp) {
	static_pointer_cast<MediaSession>(getActiveSession())->setParams(msp);
}

void Call::setSpeakerVolumeGain(float value) {
	static_pointer_cast<MediaSession>(getActiveSession())->setSpeakerVolumeGain(value);
}

MediaSessionParams *Call::createCallParams() {
	auto *params = static_pointer_cast<MediaSession>(getActiveSession())->createMediaSessionParams();

	// Clear custom contents as we don't want them to be copied to another MediaSessionParams
	params->clearCustomContents();

	return params;
}

void Call::setInputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice) {
	if (getCore()->getCCore()->use_files) {
		lInfo() << "Trying to change input audio device on call while use_files mode is on : do nothing";
		return;
	}
	if (setInputAudioDevicePrivate(audioDevice)) {
		linphone_call_notify_audio_device_changed(getSharedFromThis()->toC(), audioDevice->toC());
	}
}

void Call::setOutputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice) {
	if (getCore()->getCCore()->use_files) {
		lInfo() << "Trying to change output audio device on call while use_files mode is on : do nothing";
		return;
	}
	if (setOutputAudioDevicePrivate(audioDevice)) {
		linphone_call_notify_audio_device_changed(getSharedFromThis()->toC(), audioDevice->toC());
	}
}

std::shared_ptr<AudioDevice> Call::getInputAudioDevice() const {
	return static_pointer_cast<MediaSession>(getActiveSession())->getInputAudioDevice();
}

std::shared_ptr<AudioDevice> Call::getOutputAudioDevice() const {
	switch (getState()) {
		case CallSession::State::Paused:
		case CallSession::State::OutgoingRinging:
		case CallSession::State::Pausing:
		case CallSession::State::IncomingReceived: {
			/* In these states, the AudioDevice may be used by the ToneManager, transciently, to play a waiting
			 * tone indication. */
			auto toneManagerDevice = getCore()->getPrivate()->getToneManager().getOutputDevice(getActiveSession());
			if (toneManagerDevice) return toneManagerDevice;
		}
		default:
			break;
	}

	return static_pointer_cast<MediaSession>(getActiveSession())->getOutputAudioDevice();
}

const std::list<LinphoneMediaEncryption> Call::getSupportedEncryptions() const {
	return getActiveSession()->getSupportedEncryptions();
}

void Call::confirmGoClear() const {
	getMediaSession()->confirmGoClear();
}

void Call::onAlertNotified(std::shared_ptr<Alert> &alert) {
	alert->setCall(getSharedFromThis());
}

// -----------------------------------------------------------------------------

std::shared_ptr<Conference> Call::getConference() const {
	return mConfRef.lock();
}

void Call::setConference(std::shared_ptr<Conference> ref) {
	mConfRef = ref;
}

MSAudioEndpoint *Call::getEndpoint() const {
	return mEndpoint;
}

void Call::setEndpoint(MSAudioEndpoint *endpoint) {
	mEndpoint = endpoint;
}

void Call::setVideoSource(std::shared_ptr<const VideoSourceDescriptor> descriptor) {
	getMediaSession()->setVideoSource(descriptor);
}

std::shared_ptr<const VideoSourceDescriptor> Call::getVideoSource() const {
	return getMediaSession()->getVideoSource();
}

// -----------------------------------------------------------------------------

void Call::enableBaudotDetection(bool enabled) {
	getMediaSession()->enableBaudotDetection(enabled);
}

void Call::setBaudotMode(LinphoneBaudotMode mode) {
	getMediaSession()->setBaudotMode(mode);
}

void Call::setBaudotSendingStandard(LinphoneBaudotStandard standard) {
	getMediaSession()->setBaudotSendingStandard(standard);
}

void Call::setBaudotPauseTimeout(uint8_t seconds) {
	getMediaSession()->setBaudotPauseTimeout(seconds);
}

// -----------------------------------------------------------------------------

std::shared_ptr<Event> Call::createNotify(const std::string &eventName) {
	SalOp *callOp = getMediaSession()->getPrivate()->getOp();
	if (callOp == nullptr) {
		lError() << "Call::createNotify(): no op for this call.";
		return nullptr;
	}
	if (!callOp->hasDialog()) {
		lError() << "Call::createNotify(): no dialog established for this call.";
		return nullptr;
	}
	SalSubscribeOp *op = new SalSubscribeOp(callOp, eventName);
	return (new EventSubscribe(getCore()->getSharedFromThis(), LinphoneSubscriptionIncoming, eventName, op))
	    ->toSharedPtr();
}

// -----------------------------------------------------------------------------

CallLogContextualizer::~CallLogContextualizer() {
	if (mPushed) bctbx_pop_log_tag(sTagIdentifier);
}

void CallLogContextualizer::pushTag(const Call &call) {
	auto address = call.getRemoteAddress();
	if (address) {
		const char *value = address->getUsernameCstr();
		if (!value) value = address->getDomainCstr();
		if (value) {
			bctbx_push_log_tag(sTagIdentifier, value);
			mPushed = true;
		}
	}
}

LINPHONE_END_NAMESPACE
