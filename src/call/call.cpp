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

#include <bctoolbox/defs.h>

#include "c-wrapper/c-wrapper.h"
#include "call.h"
#include "chat/chat-room/abstract-chat-room-p.h"
#include "conference/params/media-session-params-p.h"
#include "conference/session/call-session-p.h"
#include "conference/session/media-session-p.h"
#include "conference/participant.h"
#include "conference/conference.h"
#include "core/core-p.h"
#include "factory/factory.h"
#include "logger/logger.h"
#include "sal/sal_media_description.h"

#include "conference_private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// =============================================================================
shared_ptr<CallSession> Call::getActiveSession () const {
	return mParticipant->getSession();
}

shared_ptr<AbstractChatRoom> Call::getChatRoom () {
	if ((getState() != CallSession::State::End) && (getState() != CallSession::State::Released)) {
		mChatRoom = getCore()->getOrCreateBasicChatRoom(getLocalAddress(), *getRemoteAddress());
		if (mChatRoom) {
			lInfo() << "Setting call id [" << getLog()->getCallId() << "] to ChatRoom [" << mChatRoom << "]";
			mChatRoom->getPrivate()->setCallId(getLog()->getCallId());
		}
	}
	return mChatRoom;
}

LinphoneProxyConfig *Call::getDestProxy () const {
	return getActiveSession()->getPrivate()->getDestProxy();
}

/* This a test-only method.*/
IceSession *Call::getIceSession () const {
	return static_pointer_cast<MediaSession>(getActiveSession())->getPrivate()->getIceSession();
	return nullptr;
}

const LinphoneStreamInternalStats * Call::getStreamInternalStats (LinphoneStreamType type) const {
	return getMediaSession()->getPrivate()->getStreamInternalStats(type);
}


std::shared_ptr<MediaSession> Call::getMediaSession()const{
	return static_pointer_cast<MediaSession>(getActiveSession());
}

MediaStream *Call::getMediaStream (LinphoneStreamType type) const {
	auto ms = static_pointer_cast<MediaSession>(getActiveSession())->getPrivate();
	StreamsGroup & sg = ms->getStreamsGroup();
	MS2Stream *s = nullptr;
	switch(type){
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
	if (!s){
		//lError() << "CallPrivate::getMediaStream() : no stream with type " << type;
		return nullptr;
	}
	return s->getMediaStream();
}

int Call::getMediaStreamIndex (LinphoneStreamType type) const {
	auto ms = static_pointer_cast<MediaSession>(getActiveSession())->getPrivate();
	StreamsGroup & sg = ms->getStreamsGroup();
	Stream *s = nullptr;
	switch(type){
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
	if (!s){
		return -1;
	}
	return (int)s->getIndex();
}

int Call::getMediaStreamsNb (LinphoneStreamType type) const {
	int nb = 0;
	auto ms = static_pointer_cast<MediaSession>(getActiveSession())->getPrivate();
	StreamsGroup & sg = ms->getStreamsGroup();
	SalStreamType nType;
	switch(type){
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
	for (auto &stream : sg.getStreams()){
		if (stream && (stream->getType() == nType) && (stream->getState() == Stream::Running)){
			nb ++;
		}
	}
	return nb;
}

SalCallOp * Call::getOp () const {
	return getActiveSession()->getPrivate()->getOp();
}

bool Call::getSpeakerMuted () const {
	return static_pointer_cast<MediaSession>(getActiveSession())->getPrivate()->getSpeakerMuted();
}

void Call::setSpeakerMuted (bool muted) {
	static_pointer_cast<MediaSession>(getActiveSession())->getPrivate()->setSpeakerMuted(muted);
}

bool Call::getMicrophoneMuted () const {
	return static_pointer_cast<MediaSession>(getActiveSession())->getPrivate()->getMicrophoneMuted();
}

void Call::setMicrophoneMuted (bool muted) {
	static_pointer_cast<MediaSession>(getActiveSession())->getPrivate()->setMicrophoneMuted(muted);

	if (getConference()) {
		MediaConference::Conference::toCpp(getConference())->notifyLocalMutedDevices(muted || !linphone_core_mic_enabled(getCore()->getCCore()));
	}
}

LinphoneCallStats *Call::getPrivateStats (LinphoneStreamType type) const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getPrivate()->getStats(type);
}

// =============================================================================

void Call::initiateIncoming () {
	getActiveSession()->initiateIncoming();
}

bool Call::initiateOutgoing (const string &subject, const Content *content) {
	shared_ptr<CallSession> session = getActiveSession();
	bool defer = session->initiateOutgoing(subject, content);
	session->getPrivate()->createOp();
	return defer;
}

void Call::iterate (time_t currentRealTime, bool oneSecondElapsed) {
	getActiveSession()->iterate(currentRealTime, oneSecondElapsed);
}

void Call::notifyRinging () {
	if (getState() == CallSession::State::IncomingReceived) {
		getActiveSession()->getPrivate()->handleIncoming(true);
	}
}

void Call::startIncomingNotification () {
	getActiveSession()->startIncomingNotification();
}

void Call::startPushIncomingNotification () {
	getActiveSession()->startPushIncomingNotification();
}

void Call::startBasicIncomingNotification () {
	getActiveSession()->startBasicIncomingNotification();
}

void Call::pauseForTransfer () {
	static_pointer_cast<MediaSession>(getActiveSession())->getPrivate()->pauseForTransfer();
}

int Call::startInvite (const Address *destination, const std::string subject, const Content *content) {
	return getActiveSession()->startInvite(destination, subject, content);
}

shared_ptr<Call> Call::startReferredCall (const MediaSessionParams *params) {
	// Create and initiate parameters before pausing the call because initialization of some settings requires knowledge of the current call held by the core
	MediaSessionParams msp;
	if (params)
		msp = *params;
	else {
		msp.initDefault(getCore(), LinphoneCallOutgoing);
		msp.enableAudio(getCurrentParams()->audioEnabled());
		msp.enableVideo(getCurrentParams()->videoEnabled());
	}

	if (getState() != CallSession::State::Paused) {
		pauseForTransfer();
	}
	lInfo() << "Starting new call to referred address " << getActiveSession()->getReferTo();
	L_GET_PRIVATE(&msp)->setReferer(getActiveSession());
	L_GET_PRIVATE(getActiveSession())->setReferPending(false);
	LinphoneCallParams *lcp = L_GET_C_BACK_PTR(&msp);
	LinphoneCall *newCall = linphone_core_invite_with_params(getCore()->getCCore(), getActiveSession()->getReferTo().c_str(), lcp);
	if (newCall) {
		getActiveSession()->getPrivate()->setTransferTarget(Call::toCpp(newCall)->getActiveSession());
		Call::toCpp(newCall)->getActiveSession()->getPrivate()->notifyReferState();
		Call::toCpp(newCall)->setMicrophoneMuted(getMicrophoneMuted());
		return Call::toCpp(newCall)->getSharedFromThis();
	}
	return nullptr;
}

// =============================================================================

void Call::createPlayer () const{
	mPlayer = linphone_call_build_player((LinphoneCall *)(this->toC()));
}

// -----------------------------------------------------------------------------
void Call::terminateBecauseOfLostMedia () {
	lInfo() << "Call [" << this << "]: Media connectivity with " << getRemoteAddress()->asString()
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
	auto session = getActiveSession();
	if (session)
		session->getPrivate()->setCallSessionListener(nullptr);
	linphone_call_unref(this->toC());
}

   

// -----------------------------------------------------------------------------

void Call::onAckBeingSent (UNUSED(const shared_ptr<CallSession> & session), LinphoneHeaders *headers) {
	linphone_call_notify_ack_processing(this->toC(), headers, false);
}

void Call::onAckReceived (UNUSED(const shared_ptr<CallSession> & session), LinphoneHeaders *headers) {
	linphone_call_notify_ack_processing(this->toC(), headers, true);
}

void Call::onBackgroundTaskToBeStarted (UNUSED(const shared_ptr<CallSession> & session)) {
	mBgTask.start(getCore(),30);
}

void Call::onBackgroundTaskToBeStopped (UNUSED(const shared_ptr<CallSession> & session)) {
	mBgTask.stop();
}

void Call::onCallSessionAccepting (UNUSED(const shared_ptr<CallSession> & session)) {
	accept();
}

void Call::onCallSessionEarlyFailed (const shared_ptr<CallSession> &session, LinphoneErrorInfo *ei) {
	shared_ptr<CallLog> log = session->getLog();
	if (session->getState() == CallSession::State::PushIncomingReceived) {
		session->setStateToEnded();
	}

	getCore()->reportEarlyCallFailed(log->getDirection(),	
		linphone_address_clone(log->getFromAddress()),
		linphone_address_clone(log->getToAddress()),
		ei,
		log->getCallId());

	cleanupSessionAndUnrefCObjectCall();
}

void Call::onCallSessionSetReleased (UNUSED(const shared_ptr<CallSession> & session)) {
	cleanupSessionAndUnrefCObjectCall();
}

void Call::onCallSessionSetTerminated (UNUSED(const shared_ptr<CallSession> & session)) {
	LinphoneCore *core = getCore()->getCCore();
	
	if (getSharedFromThis() == getCore()->getCurrentCall()) {
		lInfo() << "Resetting the current call";
		getCore()->getPrivate()->setCurrentCall(nullptr);
	}

	if (getCore()->getPrivate()->removeCall(getSharedFromThis()) != 0)
		lError() << "Could not remove the call from the list!!!";
#if 0
	if (mChatRoom)
		linphone_chat_room_set_call(mChatRoom, nullptr);
#endif // if 0
	if (!getCore()->getPrivate()->hasCalls())
		ms_bandwidth_controller_reset_state(core->bw_controller);

	if (linphone_core_get_calls_nb(core) == 0) {
		linphone_core_notify_last_call_ended(core);
	}
}

void Call::onCallSessionStartReferred (UNUSED(const shared_ptr<CallSession> & session)) {
	startReferredCall(nullptr);
}

void Call::reenterLocalConference(UNUSED(const shared_ptr<CallSession> &session)) {
	if (getConference()) {
		auto conference = MediaConference::Conference::toCpp(getConference());
		if (conference->getState() == ConferenceInterface::State::Created) {
			conference->enter();
		} else {
			char * conf_state = linphone_conference_state_to_string (linphone_conference_get_state (getConference()));
			lInfo() << "Unable to add participant because conference is in state " << conf_state;
			ms_free(conf_state);
		}
	}
}

void Call::onCallSessionStateChanged (const shared_ptr<CallSession> &session, CallSession::State state, const string &message) {
	LinphoneCore *lc = getCore()->getCCore();
	const auto op = session->getPrivate()->getOp();
	switch(state) {
		case CallSession::State::OutgoingInit:
		case CallSession::State::IncomingReceived:
			getPlatformHelpers(lc)->acquireWifiLock();
			getPlatformHelpers(lc)->acquireMcastLock();
			getPlatformHelpers(lc)->acquireCpuLock();
			if (linphone_core_get_calls_nb(lc) == 1) {
				linphone_core_notify_first_call_started(lc);
			}
			break;
		case CallSession::State::Released:
			getPlatformHelpers(lc)->releaseWifiLock();
			getPlatformHelpers(lc)->releaseMcastLock();
			getPlatformHelpers(lc)->releaseCpuLock();
			break;
		case CallSession::State::Paused:
			if (!getConference() && op && op->getRemoteContactAddress()) {
				auto conference = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->findAudioVideoConference(ConferenceId(ConferenceAddress(Address(op->getTo())), ConferenceAddress(Address(op->getTo()))));
				if (!op->getTo().empty() && conference) {
					// This code is usually executed when the following scenario occurs:
					// - ICE is enabled
					// - during the ICE negotiations, the core receives a call, hence this one is paused
					// - once ICE negotiation are concluded, the call is updated and the call goes back to the previous paused state
					tryToAddToConference(conference, session);
				}
			}
			break;
		case CallSession::State::UpdatedByRemote:
		{
			if (op && op->getRemoteContactAddress() && !getConference()) {
				char * remoteContactAddressStr = sal_address_as_string(op->getRemoteContactAddress());
				Address remoteContactAddress(remoteContactAddressStr);
				ms_free(remoteContactAddressStr);

				if (remoteContactAddress.hasParam("isfocus")) {
					// Check if the request was sent by the focus (remote conference)
					createRemoteConference(session);
					auto conference = getConference() ? MediaConference::Conference::toCpp(getConference()) : nullptr;
					if (conference && conference->getState() == ConferenceInterface::State::CreationPending) {
						conference->finalizeCreation();
					}
				}
			}
		}
		break;
		case CallSession::State::Connected:
		case CallSession::State::StreamsRunning:
		{
			if (op && !getConference()) {
				auto conference = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->findAudioVideoConference(ConferenceId(ConferenceAddress(Address(op->getTo())), ConferenceAddress(Address(op->getTo()))));
				if (!op->getTo().empty() && conference) {
					const auto & resourceList = op->getContentInRemote(ContentType::ResourceLists);
					if (resourceList.isEmpty()) {
						tryToAddToConference(conference, session);
					}
				} else if (op->getRemoteContactAddress()) {
					const auto & confId = session->getPrivate()->getConferenceId();
					char * remoteContactAddressStr = sal_address_as_string(op->getRemoteContactAddress());
					Address remoteContactAddress(remoteContactAddressStr);
					ms_free(remoteContactAddressStr);

					// Check if the request was sent by the focus
					if (remoteContactAddress.hasParam("isfocus")) {
						createRemoteConference(session);
					} else if (!confId.empty()) {
						auto localAddress = session->getContactAddress();
						if (localAddress.isValid()) {
							ConferenceId localConferenceId = ConferenceId(localAddress, localAddress);
							conference = getCore()->findAudioVideoConference(localConferenceId, false);
							if (conference) {
								setConference(conference->toC());
								reenterLocalConference(session);
								conference->addParticipantDevice(getSharedFromThis());
							}
						} else {
							lError() << "Call " << this << " cannot be added to conference with ID " << confId << " because the contact address has not been retrieved";
						}
					}
				}
			}
		}
		break;
		default:
			break;
	}
	linphone_call_notify_state_changed(this->toC(), static_cast<LinphoneCallState>(state), message.c_str());
}

void Call::tryToAddToConference(shared_ptr<MediaConference::Conference> & conference, const shared_ptr<CallSession> &session) {
	// If the call is for a conference stored in the core, then add call to conference once ICE negotiations are terminated
	if (!mediaInProgress() || !!!linphone_config_get_int(linphone_core_get_config(session->getCore()->getCCore()), "sip", "update_call_when_ice_completed", TRUE)){
		const auto & device = conference->findParticipantDevice(session);
		if (device) {
			const auto & deviceState = device->getState();
			if ((deviceState == ParticipantDevice::State::Joining) || (deviceState == ParticipantDevice::State::ScheduledForJoining)) {
				// Participants complete their addition to a conference when the call goes back to the StreamsRunning state
				conference->finalizeParticipantAddition(getSharedFromThis());
			}
		} else {
			conference->addParticipant(getSharedFromThis());
		}
	}
}

void Call::createRemoteConference(const shared_ptr<CallSession> &session) {
	// If the call is for a conference stored in the core, then add call to conference once ICE negotiations are terminated
	const auto op = session->getPrivate()->getOp();
	char * remoteContactAddressStr = sal_address_as_string(op->getRemoteContactAddress());
	Address remoteContactAddress(remoteContactAddressStr);
	ms_free(remoteContactAddressStr);
	ConferenceId conferenceId = ConferenceId(remoteContactAddress, getLocalAddress());

	const auto & conference = getCore()->findAudioVideoConference(conferenceId, false);

	std::shared_ptr<MediaConference::RemoteConference> remoteConference = nullptr;

	if (conference) {
		lInfo() << "Attaching call (local address " << session->getLocalAddress().asString() << " remote address " << session->getRemoteAddress()->asString() << ") to conference " << conference->getConferenceAddress() << " ID " << conferenceId;
		remoteConference = dynamic_pointer_cast<MediaConference::RemoteConference>(conference);
		if (remoteConference) {
			remoteConference->setMainSession(session);
		}
	} else {
		auto confParams = ConferenceParams::create(getCore()->getCCore());
		std::shared_ptr<ConferenceInfo> conferenceInfo = 
		#ifdef HAVE_DB_STORAGE
			getCore()->getPrivate()->mainDb ? getCore()->getPrivate()->mainDb->getConferenceInfoFromURI(remoteContactAddress) :
		#endif
			nullptr;

		const auto op = session->getPrivate()->getOp();
		const auto sipfrag = op->getContentInRemote(ContentType::SipFrag);
		const auto resourceList = op->getContentInRemote(ContentType::ResourceLists);
		if (conferenceInfo) {
			confParams->setUtf8Subject(conferenceInfo->getUtf8Subject());
			auto startTime = conferenceInfo->getDateTime();
			confParams->setStartTime(startTime);
			auto duration = conferenceInfo->getDuration();
			if (duration > 0) {
				// duration is in minutes therefore convert it to seconds by multiplying it by 60
				time_t endTime = startTime + static_cast<time_t>(duration) * 60;
				confParams->setEndTime(endTime);
			}
			std::list<IdentityAddress> invitees {conferenceInfo->getOrganizerAddress()};
			for (const auto & participant : conferenceInfo->getParticipants()) {
				invitees.push_back(participant.first);
			}

			const ConferenceAddress confAddr(conferenceInfo->getUri());
			const ConferenceId confId(confAddr, session->getLocalAddress());
			remoteConference = std::shared_ptr<MediaConference::RemoteConference>(new MediaConference::RemoteConference(getCore(), session, confAddr, confId, invitees, nullptr, confParams), [](MediaConference::RemoteConference * c){c->unref();});
#ifdef HAVE_ADVANCED_IM
		} else if (!resourceList.isEmpty() || !sipfrag.isEmpty()) {
			const auto & remoteParams = static_pointer_cast<MediaSession>(session)->getRemoteParams();
			confParams->setStartTime(remoteParams->getPrivate()->getStartTime());
			confParams->setEndTime(remoteParams->getPrivate()->getEndTime());
			auto organizer = Utils::getSipFragAddress(sipfrag);
			auto invitees = Utils::parseResourceLists(resourceList);
			invitees.push_back(IdentityAddress(organizer));
			remoteConference = std::shared_ptr<MediaConference::RemoteConference>(new MediaConference::RemoteConference(getCore(), session, remoteContactAddress, conferenceId, invitees, nullptr, confParams), [](MediaConference::RemoteConference * c){c->unref();});
#endif // HAVE_ADVANCED_IM
		} else {
			const auto & remoteParams = static_pointer_cast<MediaSession>(session)->getRemoteParams();
			confParams->setStartTime(remoteParams->getPrivate()->getStartTime());
			confParams->setEndTime(remoteParams->getPrivate()->getEndTime());
			// It is expected that the core of the remote conference is the participant one
			remoteConference = std::shared_ptr<MediaConference::RemoteConference>(new MediaConference::RemoteConference(getCore(), getSharedFromThis(), conferenceId, nullptr, confParams), [](MediaConference::RemoteConference * c){c->unref();});
		}
	}

	setConference(remoteConference->toC());

	// Record conf-id to be used later when terminating the remote conference
	if (remoteContactAddress.hasUriParam("conf-id")) {
		setConferenceId(remoteContactAddress.getUriParamValue("conf-id"));
	}
}

void Call::onCallSessionTransferStateChanged (UNUSED(const shared_ptr<CallSession> &session), CallSession::State state) {
	linphone_call_notify_transfer_state_changed(this->toC(), static_cast<LinphoneCallState>(state));
}

void Call::onCheckForAcceptation (UNUSED(const shared_ptr<CallSession> &session)) {
	// If the core is a conference server, there is no need to ensure that media resources are not shared
	if(!linphone_core_conference_server_enabled(getCore()->getCCore()) && (linphone_core_get_media_resource_mode(getCore()->getCCore()) != LinphoneSharedMediaResources)) {
		list<shared_ptr<Call>> calls = getCore()->getCalls();
		shared_ptr<Call> currentCall = getSharedFromThis();
		for (const auto &call : calls) {
			if (call == currentCall)
				continue;
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

void Call::onDtmfReceived (UNUSED(const shared_ptr<CallSession> &session), char dtmf) {
	linphone_call_notify_dtmf_received(this->toC(), dtmf);
}

void Call::onIncomingCallSessionNotified (UNUSED(const shared_ptr<CallSession> &session)) {
	/* The call is acceptable so we can now add it to our list */
	getCore()->getPrivate()->addCall(getSharedFromThis());
}

void Call::onIncomingCallSessionStarted (UNUSED(const shared_ptr<CallSession> &session)) {
	if (linphone_core_get_calls_nb(getCore()->getCCore()) == 1 && !isInConference()) {
		L_GET_PRIVATE_FROM_C_OBJECT(getCore()->getCCore())->setCurrentCall(getSharedFromThis());
	}
}

void Call::onIncomingCallSessionTimeoutCheck (UNUSED(const shared_ptr<CallSession> &session), int elapsed, bool oneSecondElapsed) {
	if (oneSecondElapsed)
		lInfo() << "Incoming call ringing for " << elapsed << " seconds";
	if (elapsed > getCore()->getCCore()->sip_conf.inc_timeout) {
		lInfo() << "Incoming call timeout (" << getCore()->getCCore()->sip_conf.inc_timeout << ")";
		auto config = linphone_core_get_config(getCore()->getCCore());
		int statusCode = linphone_config_get_int(config, "sip", "inc_timeout_status_code", 486);
		getActiveSession()->declineNotAnswered(linphone_error_code_to_reason(statusCode));
	}
}

void Call::onPushCallSessionTimeoutCheck (UNUSED(const std::shared_ptr<CallSession> &session), int elapsed) {
	if (elapsed > getCore()->getCCore()->sip_conf.push_incoming_call_timeout) {
		lInfo() << "Push incoming call timeout (" << getCore()->getCCore()->sip_conf.push_incoming_call_timeout << ")";
		auto config = linphone_core_get_config(getCore()->getCCore());
		int statusCode = linphone_config_get_int(config, "sip", "push_incoming_call_timeout_status_code", 410); //LinphoneReasonGone
		getActiveSession()->decline(linphone_error_code_to_reason(statusCode));
		getActiveSession()->getPrivate()->setState(LinphonePrivate::CallSession::State::Released, "Call released");
	}
}

void Call::onInfoReceived (UNUSED(const shared_ptr<CallSession> &session), const LinphoneInfoMessage *im) {
	linphone_call_notify_info_message_received(this->toC(), im);
}

void Call::onLossOfMediaDetected (UNUSED(const shared_ptr<CallSession> &session)) {
	terminateBecauseOfLostMedia();
}

void Call::onEncryptionChanged (UNUSED(const shared_ptr<CallSession> &session), bool activated, const string &authToken) {
	linphone_call_notify_encryption_changed(this->toC(), activated, authToken.empty() ? nullptr : authToken.c_str());
}

void Call::onGoClearAckSent() {
	linphone_call_notify_goclear_ack_sent(this->toC());
}

void Call::onCallSessionStateChangedForReporting (UNUSED(const shared_ptr<CallSession> &session)) {
	linphone_reporting_call_state_updated(this->toC());
}

void Call::onRtcpUpdateForReporting (UNUSED(const shared_ptr<CallSession> &session), SalStreamType type) {
	linphone_reporting_on_rtcp_update(this->toC(), type);
}

void Call::onStatsUpdated (UNUSED(const shared_ptr<CallSession> &session), const LinphoneCallStats *stats) {
	linphone_call_notify_stats_updated(this->toC(), stats);
}

void Call::onUpdateMediaInfoForReporting (UNUSED(const shared_ptr<CallSession> &session), int statsType) {
	linphone_reporting_update_media_info(this->toC(), statsType);
}


void Call::onResetCurrentSession (UNUSED(const shared_ptr<CallSession> &session)) {
	getCore()->getPrivate()->setCurrentCall(nullptr);
}

void Call::onSetCurrentSession (UNUSED(const shared_ptr<CallSession> &session)) {
	getCore()->getPrivate()->setCurrentCall(getSharedFromThis());
}

void Call::onFirstVideoFrameDecoded (UNUSED(const shared_ptr<CallSession> &session)) {
	if (mNextVideoFrameDecoded._func) {
		mNextVideoFrameDecoded._func(this->toC(), mNextVideoFrameDecoded._user_data);
		mNextVideoFrameDecoded._func = nullptr;
		mNextVideoFrameDecoded._user_data = nullptr;
	}
	linphone_call_notify_next_video_frame_decoded(this->toC());
}

void Call::onResetFirstVideoFrameDecoded (UNUSED(const shared_ptr<CallSession> &session)) {
	/*we are called here by the MediaSession when the stream start to know whether there is the deprecated nextVideoFrameDecoded callback set,
	 * so that we can request the notification of the next frame decoded.*/
#ifdef VIDEO_ENABLED
	if (mNextVideoFrameDecoded._func)
		requestNotifyNextVideoFrameDecoded();
#endif // ifdef VIDEO_ENABLED
}

void Call::requestNotifyNextVideoFrameDecoded(){
	static_pointer_cast<MediaSession>(getActiveSession())->requestNotifyNextVideoFrameDecoded();
}

void Call::onCameraNotWorking (UNUSED(const std::shared_ptr<CallSession> &session), const char *camera_name) {
	linphone_call_notify_camera_not_working(this->toC(), camera_name);
}

bool Call::areSoundResourcesAvailable (UNUSED(const shared_ptr<CallSession> &session)) {
	LinphoneCore *lc = getCore()->getCCore();
	shared_ptr<Call> currentCall = getCore()->getCurrentCall();
	// If core is in a conference, then check if the call is in the same conference
	// If the core left the conference or it is not hosting any conference, then check that there is no active call or the active one is the current one.
	bool soundResourcesFree = linphone_core_is_in_conference(lc) ? (linphone_core_get_conference(lc) == getConference()) : (!currentCall || (currentCall == getSharedFromThis()));
	return soundResourcesFree;
}

bool Call::isPlayingRingbackTone (UNUSED(const shared_ptr<CallSession> &session)) {
	return mPlayingRingbackTone;
}

LinphoneConference * Call::getCallSessionConference (UNUSED(const shared_ptr<CallSession> &session)) const {
	return getConference();
}

void Call::onRealTimeTextCharacterReceived (UNUSED(const shared_ptr<CallSession> &session), RealtimeTextReceivedCharacter *data) {
	shared_ptr<AbstractChatRoom> chatRoom = getChatRoom();
	if (chatRoom) {
		chatRoom->getPrivate()->realtimeTextReceived(data->character, getSharedFromThis());
	} else {
		lError()<<"CallPrivate::onRealTimeTextCharacterReceived: no chatroom.";
	}
}

void Call::onTmmbrReceived (UNUSED(const shared_ptr<CallSession> &session), int streamIndex, int tmmbr) {
	linphone_call_notify_tmmbr_received(this->toC(), streamIndex, tmmbr);
}

void Call::onSnapshotTaken(UNUSED(const shared_ptr<CallSession> &session), const char *file_path) {
	linphone_call_notify_snapshot_taken(this->toC(), file_path);
}

void Call::onStartRingtone(UNUSED(const shared_ptr<CallSession> &session)){
	//Already handled by tone manager.
}

void Call::onRemoteRecording(UNUSED(const std::shared_ptr<CallSession> &session), bool recording) {
	linphone_call_notify_remote_recording(this->toC(), recording);
}

// =============================================================================

Call::Call (
	shared_ptr<Core> core,
	LinphoneCallDir direction,
	const Address &from,
	const Address &to,
	LinphoneProxyConfig *cfg,
	SalCallOp *op,
	const MediaSessionParams *msp
) : CoreAccessor(core) {
	mNextVideoFrameDecoded._func = nullptr;
	mNextVideoFrameDecoded._user_data = nullptr;

	mBgTask.setName("Liblinphone call notification");

	//create session
	mParticipant = Participant::create(nullptr, IdentityAddress((direction == LinphoneCallIncoming) ? to : from));
	mParticipant->createSession(getCore(), msp, TRUE, this);
	mParticipant->getSession()->configure(direction, cfg, op, from, to);

	configureSoundCardsFromCore(msp);
}

Call::Call (
	std::shared_ptr<Core> core,
	LinphoneCallDir direction,
	const string &callid
): CoreAccessor(core) {
	mNextVideoFrameDecoded._func = nullptr;
	mNextVideoFrameDecoded._user_data = nullptr;

	mBgTask.setName("Liblinphone call notification");

	mParticipant = Participant::create();
	mParticipant->createSession(getCore(), nullptr, TRUE, this);
	mParticipant->getSession()->configure(direction, callid);

	configureSoundCardsFromCore(nullptr);
}

Call::~Call () {
}

void Call::configureSoundCardsFromCore(const MediaSessionParams *msp) {
	if (ms_snd_card_manager_reload_requested(ms_factory_get_snd_card_manager(getCore()->getCCore()->factory))) {
		linphone_core_reload_sound_devices(getCore()->getCCore());
	}
	if (msp) {
		setMicrophoneMuted(!msp->isMicEnabled());
		setInputAudioDevicePrivate(msp->getInputAudioDevice());
		setOutputAudioDevicePrivate(msp->getOutputAudioDevice());
		return;
	}
	auto outputAudioDevice = getCore()->getDefaultOutputAudioDevice();
	if (outputAudioDevice) {
		setOutputAudioDevicePrivate(outputAudioDevice);
	} else if(!getCore()->getCCore()->use_files){
		lWarning() << "Failed to find audio device matching default output sound card [" << getCore()->getCCore()->sound_conf.play_sndcard << "]";
	}
	auto inputAudioDevice = getCore()->getDefaultInputAudioDevice();
	if (inputAudioDevice) {
		setInputAudioDevicePrivate(inputAudioDevice);
	} else if(!getCore()->getCCore()->use_files){
		lWarning() << "Failed to find audio device matching default input sound card [" << getCore()->getCCore()->sound_conf.capt_sndcard << "]";
	}
}

void Call::configure (
	LinphoneCallDir direction,
	const Address &from,
	const Address &to,
	LinphoneProxyConfig *cfg,
	SalCallOp *op,
	UNUSED(const MediaSessionParams *msp)
) {
	mParticipant->configure(nullptr, IdentityAddress((direction == LinphoneCallIncoming) ? to : from));
	mParticipant->getSession()->configure(direction, cfg, op, from, to);
}

bool Call::isOpConfigured () const {
	return getActiveSession()->isOpConfigured();
}

// =============================================================================

LinphoneStatus Call::accept (const MediaSessionParams *msp) {
	return static_pointer_cast<MediaSession>(getActiveSession())->accept(msp);
}

LinphoneStatus Call::acceptEarlyMedia (const MediaSessionParams *msp) {
	return static_pointer_cast<MediaSession>(getActiveSession())->acceptEarlyMedia(msp);
}

LinphoneStatus Call::acceptUpdate (const MediaSessionParams *msp) {
	return static_pointer_cast<MediaSession>(getActiveSession())->acceptUpdate(msp);
}

void Call::cancelDtmfs () {
	static_pointer_cast<MediaSession>(getActiveSession())->cancelDtmfs();
}

LinphoneStatus Call::decline (LinphoneReason reason) {
	return getActiveSession()->decline(reason);
}

LinphoneStatus Call::decline (const LinphoneErrorInfo *ei) {
	return getActiveSession()->decline(ei);
}

LinphoneStatus Call::deferUpdate () {
	return getActiveSession()->deferUpdate();
}

bool Call::hasTransferPending () const {
	return getActiveSession()->hasTransferPending();
}

void Call::oglRender () const {
	static_pointer_cast<MediaSession>(getActiveSession())->getPrivate()->oglRender();
}

LinphoneStatus Call::pauseFromConference () {
	return static_pointer_cast<MediaSession>(getActiveSession())->pauseFromConference();
}

LinphoneStatus Call::pause () {
	return static_pointer_cast<MediaSession>(getActiveSession())->pause();
}

bool Call::canSoundResourcesBeFreed () const {
	return static_pointer_cast<MediaSession>(getActiveSession())->getPrivate()->canSoundResourcesBeFreed();
}

LinphoneStatus Call::redirect (const string &redirectUri) {
	return getActiveSession()->redirect(redirectUri);
}

LinphoneStatus Call::redirect (const Address &redirectAddress) {
	return getActiveSession()->redirect(redirectAddress);
}

LinphoneStatus Call::resume () {
	return static_pointer_cast<MediaSession>(getActiveSession())->resume();
}

LinphoneStatus Call::sendDtmf (char dtmf) {
	return static_pointer_cast<MediaSession>(getActiveSession())->sendDtmf(dtmf);
}

LinphoneStatus Call::sendDtmfs (const string &dtmfs) {
	return static_pointer_cast<MediaSession>(getActiveSession())->sendDtmfs(dtmfs);
}

void Call::sendVfuRequest () {
	static_pointer_cast<MediaSession>(getActiveSession())->sendVfuRequest();
}

void Call::updateRecordState(SalMediaRecord state) {
	if (linphone_core_is_record_aware_enabled(getCore()->getCCore())
		&& getRemoteParams()->recordAwareEnabled()) {
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

void Call::startRecording () {
	if (static_pointer_cast<MediaSession>(getActiveSession())->startRecording()) {
		updateRecordState(SalMediaRecordOn);
	}
}

void Call::stopRecording () {
	static_pointer_cast<MediaSession>(getActiveSession())->stopRecording();
	updateRecordState(SalMediaRecordOff);
}

bool Call::isRecording () {
	return static_pointer_cast<MediaSession>(getActiveSession())->getMediaParams()->isRecording();
}

LinphoneStatus Call::takePreviewSnapshot (const string &file) {
	return static_pointer_cast<MediaSession>(getActiveSession())->takePreviewSnapshot(file);
}

LinphoneStatus Call::takeVideoSnapshot (const string &file) {
	return static_pointer_cast<MediaSession>(getActiveSession())->takeVideoSnapshot(file);
}

LinphoneStatus Call::terminate (const LinphoneErrorInfo *ei) {
	bool cleanCall = getActiveSession()->getState() == CallSession::State::OutgoingInit;
	LinphoneStatus status = getActiveSession()->terminate(ei);
	if( cleanCall && !status )
		unref();
	return status;
}

LinphoneStatus Call::transfer (const shared_ptr<Call> &dest) {
	return getActiveSession()->transfer(dest->getActiveSession());
}

LinphoneStatus Call::transfer (const string &dest) {
	return getActiveSession()->transfer(dest);
}

LinphoneStatus Call::transfer (const Address &dest) {
	return getActiveSession()->transfer(dest);
}

LinphoneStatus Call::updateFromConference (const MediaSessionParams *msp) {
	return static_pointer_cast<MediaSession>(getActiveSession())->updateFromConference(msp);
}

LinphoneStatus Call::update (const MediaSessionParams *msp) {
	return static_pointer_cast<MediaSession>(getActiveSession())->update(msp);
}

void Call::zoomVideo (float zoomFactor, float *cx, float *cy) {
	zoomVideo(zoomFactor, *cx, *cy);
}

void Call::zoomVideo (float zoomFactor, float cx, float cy) {
	static_pointer_cast<MediaSession>(getActiveSession())->zoomVideo(zoomFactor, cx, cy);
}

// -----------------------------------------------------------------------------

bool Call::cameraEnabled () const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->cameraEnabled();
}

bool Call::echoCancellationEnabled () const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->echoCancellationEnabled();
}

bool Call::echoLimiterEnabled () const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->echoLimiterEnabled();
}

void Call::enableCamera (bool value) {
	static_pointer_cast<MediaSession>(getActiveSession())->enableCamera(value);
}

void Call::enableEchoCancellation (bool value) {
	static_pointer_cast<MediaSession>(getActiveSession())->enableEchoCancellation(value);
}

void Call::enableEchoLimiter (bool value) {
	static_pointer_cast<MediaSession>(getActiveSession())->enableEchoLimiter(value);
}

bool Call::getAllMuted () const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getAllMuted();
}

LinphoneCallStats *Call::getAudioStats () const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getAudioStats();
}

const string &Call::getAuthenticationToken ()  {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getAuthenticationToken();
}

bool Call::getAuthenticationTokenVerified () const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getAuthenticationTokenVerified();
}

float Call::getAverageQuality () const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getAverageQuality();
}

const MediaSessionParams *Call::getCurrentParams () const {
	return static_pointer_cast<MediaSession>(getActiveSession())->getCurrentParams();
}

float Call::getCurrentQuality () const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getCurrentQuality();
}

LinphoneCallDir Call::getDirection () const {
	return getActiveSession()->getDirection();
}

const Address &Call::getDiversionAddress () const {
	return getActiveSession()->getDiversionAddress();
}

int Call::getDuration () const {
	return getActiveSession()->getDuration();
}

const LinphoneErrorInfo *Call::getErrorInfo () const {
	return getActiveSession()->getErrorInfo();
}

const Address &Call::getLocalAddress () const {
	return getActiveSession()->getLocalAddress();
}

shared_ptr<CallLog> Call::getLog () const {
	return getActiveSession()->getLog();
}

RtpTransport *Call::getMetaRtcpTransport (int streamIndex) const {
	return static_pointer_cast<MediaSession>(getActiveSession())->getMetaRtcpTransport(streamIndex);
}

RtpTransport *Call::getMetaRtpTransport (int streamIndex) const {
	return static_pointer_cast<MediaSession>(getActiveSession())->getMetaRtpTransport(streamIndex);
}

float Call::getMicrophoneVolumeGain () const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getMicrophoneVolumeGain();
}

void *Call::getNativeVideoWindowId () const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getNativeVideoWindowId();
}

void *Call::createNativeVideoWindowId () const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->createNativeVideoWindowId();
}

const MediaSessionParams *Call::getParams () const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getMediaParams();
}

LinphonePlayer *Call::getPlayer () const {
	if (!mPlayer)
		createPlayer();
	return mPlayer;
}

float Call::getPlayVolume () const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getPlayVolume();
}

LinphoneReason Call::getReason () const {
	return getActiveSession()->getReason();
}

float Call::getRecordVolume () const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getRecordVolume();
}

shared_ptr<Call> Call::getReferer () const {
	shared_ptr<CallSession> referer = getActiveSession()->getReferer();
	if (!referer)
		return nullptr;
	for (const auto &call : getCore()->getCalls()) {
		if (call->getActiveSession() == referer)
			return call;
	}
	return nullptr;
}

const string &Call::getReferTo () const {
	return getActiveSession()->getReferTo();
}

const Address &Call::getReferToAddress () const {
	return getActiveSession()->getReferToAddress();
}

const Address *Call::getRemoteAddress () const {
	return getActiveSession()->getRemoteAddress();
}

const Address *Call::getRemoteContactAddress () const {
	return getActiveSession()->getRemoteContactAddress();
}

const string &Call::getRemoteContact () const {
	return getActiveSession()->getRemoteContact();
}

const MediaSessionParams *Call::getRemoteParams () const {
	return static_pointer_cast<MediaSession>(getActiveSession())->getRemoteParams();
}

const string &Call::getRemoteUserAgent () {
	return getActiveSession()->getRemoteUserAgent();
}

shared_ptr<Call> Call::getReplacedCall () const {
	shared_ptr<CallSession> replacedCallSession = getActiveSession()->getReplacedCallSession();
	if (!replacedCallSession)
		return nullptr;
	for (const auto &call : getCore()->getCalls()) {
		if (call->getActiveSession() == replacedCallSession)
			return call;
	}
	return nullptr;
}

float Call::getSpeakerVolumeGain () const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getSpeakerVolumeGain();
}

CallSession::State Call::getState () const {
	return getActiveSession()->getState();
}

LinphoneCallStats *Call::getStats (LinphoneStreamType type) const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getStats(type);
}

int Call::getStreamCount () const {
	return static_pointer_cast<MediaSession>(getActiveSession())->getStreamCount();
}

MSFormatType Call::getStreamType (int streamIndex) const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getStreamType(streamIndex);
}

LinphoneCallStats *Call::getTextStats () const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getTextStats();
}

const Address &Call::getToAddress () const {
	return getActiveSession()->getToAddress();
}

const char *Call::getToHeader (const string &name) const {
	return getActiveSession()->getToHeader(name);
}

CallSession::State Call::getTransferState () const {
	return getActiveSession()->getTransferState();
}

shared_ptr<Call> Call::getTransferTarget () const {
	shared_ptr<CallSession> transferTarget = getActiveSession()->getTransferTarget();
	if (!transferTarget)
		return nullptr;
	for (const auto &call : getCore()->getCalls()) {
		if (call->getActiveSession() == transferTarget)
			return call;
	}
	return nullptr;
}

LinphoneCallStats *Call::getVideoStats () const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getVideoStats();
}

// Boolean to state whether it is the focus of a local conference
bool Call::isInConference () const {
	return getActiveSession()->getPrivate()->isInConference();
}

void Call::setConferenceId (const std::string & conferenceId) {
	return getActiveSession()->getPrivate()->setConferenceId(conferenceId);
}

std::string Call::getConferenceId () const {
	return getActiveSession()->getPrivate()->getConferenceId();
}

bool Call::mediaInProgress () const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->mediaInProgress();
}

void Call::setAuthenticationTokenVerified (bool value) {
	static_pointer_cast<MediaSession>(getActiveSession())->setAuthenticationTokenVerified(value);
}

void Call::setMicrophoneVolumeGain (float value) {
	static_pointer_cast<MediaSession>(getActiveSession())->setMicrophoneVolumeGain(value);
}

void Call::setNativeVideoWindowId (void *id) {
	static_pointer_cast<MediaSession>(getActiveSession())->setNativeVideoWindowId(id);
}

void Call::setNextVideoFrameDecodedCallback (LinphoneCallCbFunc cb, void *user_data) {
	mNextVideoFrameDecoded._func = cb;
	mNextVideoFrameDecoded._user_data = user_data;
	requestNotifyNextVideoFrameDecoded();
}

void Call::setParams (const MediaSessionParams *msp) {
	static_pointer_cast<MediaSession>(getActiveSession())->setParams(msp);
}

void Call::setSpeakerVolumeGain (float value) {
	static_pointer_cast<MediaSession>(getActiveSession())->setSpeakerVolumeGain(value);
}

void Call::setInputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice) {
	if(getCore()->getCCore()->use_files) {
		lInfo() << "Trying to change input audio device on call while use_files mode is on : do nothing";
		return;
	}
	if (setInputAudioDevicePrivate(audioDevice)) {
		linphone_call_notify_audio_device_changed(getSharedFromThis()->toC(), audioDevice->toC());
	}
}

void Call::setOutputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice) {
	if(getCore()->getCCore()->use_files) {
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
		case CallSession::State::IncomingReceived:
		{
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

// -----------------------------------------------------------------------------

LinphoneConference *Call::getConference () const{
	return mConfRef;
}

void Call::setConference (LinphoneConference *ref) {
	mConfRef = ref;
}

MSAudioEndpoint *Call::getEndpoint () const {
	return mEndpoint;
}

void Call::setEndpoint (MSAudioEndpoint *endpoint) {
	mEndpoint = endpoint;
}

// -----------------------------------------------------------------------------

void Call::setVideoSource (std::shared_ptr<const VideoSourceDescriptor> descriptor) {
	getMediaSession()->setVideoSource(descriptor);
}

std::shared_ptr<const VideoSourceDescriptor> Call::getVideoSource () const {
	return getMediaSession()->getVideoSource();
}

LINPHONE_END_NAMESPACE
