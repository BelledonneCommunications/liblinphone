/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "c-wrapper/c-wrapper.h"
#include "call.h"
#include "chat/chat-room/real-time-text-chat-room-p.h"
#include "conference/params/media-session-params-p.h"
#include "conference/session/call-session-p.h"
#include "conference/session/media-session-p.h"
#include "conference/participant.h"
#include "core/core-p.h"
#include "logger/logger.h"
#include "conference/handlers/remote-conference-event-handler.h"

#include "conference_private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// =============================================================================
shared_ptr<CallSession> Call::getActiveSession () const {
	return mParticipant->getSession();
}

shared_ptr<RealTimeTextChatRoom> Call::getChatRoom () {
	if (!mChatRoom && (getState() != CallSession::State::End) && (getState() != CallSession::State::Released)) {
		mChatRoom = static_pointer_cast<RealTimeTextChatRoom>(getCore()->getOrCreateBasicChatRoom(*getRemoteAddress(), true));
		if (mChatRoom) mChatRoom->getPrivate()->setCallId(linphone_call_log_get_call_id(getLog()));
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

unsigned int Call::getAudioStartCount () const {
	return static_pointer_cast<MediaSession>(getActiveSession())->getPrivate()->getAudioStartCount();
}

unsigned int Call::getVideoStartCount () const {
	return static_pointer_cast<MediaSession>(getActiveSession())->getPrivate()->getVideoStartCount();
}

unsigned int Call::getTextStartCount () const {
	return static_pointer_cast<MediaSession>(getActiveSession())->getPrivate()->getTextStartCount();
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
		lError() << "CallPrivate::getMediaStream() : no stream with type " << type;
		return nullptr;
	}
	return s->getMediaStream();
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
}

LinphoneCallStats *Call::getPrivateStats (LinphoneStreamType type) const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->getPrivate()->getStats(type);
}

// =============================================================================

void Call::initiateIncoming () {
	getActiveSession()->initiateIncoming();
}

bool Call::initiateOutgoing () {
	shared_ptr<CallSession> session = getActiveSession();
	bool defer = session->initiateOutgoing();
	
	AudioDevice *outputAudioDevice = getCore()->getDefaultOutputAudioDevice();
	if (outputAudioDevice) {
		setOutputAudioDevicePrivate(outputAudioDevice);
	} else {
		lWarning() << "Failed to find audio device matching default output sound card [" << getCore()->getCCore()->sound_conf.play_sndcard << "]";
	}

	AudioDevice *inputAudioDevice = getCore()->getDefaultInputAudioDevice();
	if (inputAudioDevice) {
		setInputAudioDevicePrivate(inputAudioDevice);
	} else {
		lWarning() << "Failed to find audio device matching default input sound card [" << getCore()->getCCore()->sound_conf.capt_sndcard << "]";
	}

	session->getPrivate()->createOp();
	return defer;
}

void Call::iterate (time_t currentRealTime, bool oneSecondElapsed) {
	getActiveSession()->iterate(currentRealTime, oneSecondElapsed);
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

int Call::startInvite (const Address *destination) {
	return getActiveSession()->startInvite(destination, "");
}

shared_ptr<Call> Call::startReferredCall (const MediaSessionParams *params) {
	if (getState() != CallSession::State::Paused) {
		pauseForTransfer();
	}
	MediaSessionParams msp;
	if (params)
		msp = *params;
	else {
		msp.initDefault(getCore(), LinphoneCallOutgoing);
		msp.enableAudio(getCurrentParams()->audioEnabled());
		msp.enableVideo(getCurrentParams()->videoEnabled());
	}
	lInfo() << "Starting new call to referred address " << getActiveSession()->getReferTo();
	L_GET_PRIVATE(&msp)->setReferer(getActiveSession());
	L_GET_PRIVATE(getActiveSession())->setReferPending(false);
	LinphoneCallParams *lcp = L_GET_C_BACK_PTR(&msp);
	LinphoneCall *newCall = linphone_core_invite_with_params(getCore()->getCCore(), getActiveSession()->getReferTo().c_str(), lcp);
	if (newCall) {
		getActiveSession()->getPrivate()->setTransferTarget(Call::toCpp(newCall)->getActiveSession());
		Call::toCpp(newCall)->getActiveSession()->getPrivate()->notifyReferState();
	}
	return Call::toCpp(newCall)->getSharedFromThis();
}

// =============================================================================

void Call::createPlayer () const{
	mPlayer = linphone_call_build_player((LinphoneCall *)(this->toC()));
}

// -----------------------------------------------------------------------------
void Call::startRemoteRing () {
	LinphoneCore *lc = getCore()->getCCore();
	if (!lc->sound_conf.play_sndcard)
		return;

	MSSndCard *ringCard = lc->sound_conf.lsd_card ? lc->sound_conf.lsd_card : lc->sound_conf.play_sndcard;
	SalMediaDescription *md = static_pointer_cast<MediaSession>(getActiveSession())->getPrivate()->getLocalDesc();
	if (md){
		int maxRate = md->streams[0].max_rate;
		if (maxRate > 0)
			ms_snd_card_set_preferred_sample_rate(ringCard, maxRate);
	}
	if (lc->sound_conf.remote_ring) {
		ms_snd_card_set_stream_type(ringCard, MS_SND_CARD_STREAM_VOICE);
		lc->ringstream = ring_start(lc->factory, lc->sound_conf.remote_ring, 2000, ringCard);
	}
}

void Call::terminateBecauseOfLostMedia () {
	lInfo() << "Call [" << this << "]: Media connectivity with " << getRemoteAddress()->asString()
		<< " is lost, call is going to be terminated";
	static_pointer_cast<MediaSession>(getActiveSession())->terminateBecauseOfLostMedia();
	getCore()->getPrivate()->getToneManager()->startNamedTone(getActiveSession(), LinphoneToneCallLost);
}

void Call::setInputAudioDevicePrivate(AudioDevice *audioDevice) {
	if ((audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Record)) == 0) {
		lError() << "Audio device [" << audioDevice << "] doesn't have Record capability";
		return;
	}

	static_pointer_cast<MediaSession>(getActiveSession())->setInputAudioDevice(audioDevice);

}

void Call::setOutputAudioDevicePrivate(AudioDevice *audioDevice) {
	if ((audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Play)) == 0) {
		lError() << "Audio device [" << audioDevice << "] doesn't have Play capability";
		return;
	}

	RingStream *ringStream = nullptr;
	switch (getState()) {
		case CallSession::State::OutgoingRinging:
		case CallSession::State::Pausing:
		case CallSession::State::Paused:
			ringStream = getCore()->getCCore()->ringstream;
			if (ringStream) {
				ring_stream_set_output_ms_snd_card(ringStream, audioDevice->getSoundCard());
			}
			break;
		case CallSession::State::IncomingReceived:
			ringStream = linphone_ringtoneplayer_get_stream(getCore()->getCCore()->ringtoneplayer);
			if (ringStream) {
				ring_stream_set_output_ms_snd_card(ringStream, audioDevice->getSoundCard());
			}
			break;
		default:
			static_pointer_cast<MediaSession>(getActiveSession())->setOutputAudioDevice(audioDevice);
			break;
	}
}

// -----------------------------------------------------------------------------

void Call::onAckBeingSent (const shared_ptr<CallSession> &session, LinphoneHeaders *headers) {
	linphone_call_notify_ack_processing(this->toC(), headers, false);
}

void Call::onAckReceived (const shared_ptr<CallSession> &session, LinphoneHeaders *headers) {
	linphone_call_notify_ack_processing(this->toC(), headers, true);
}

void Call::onBackgroundTaskToBeStarted (const shared_ptr<CallSession> &session) {
	mBgTask.start(getCore(),30);
}

void Call::onBackgroundTaskToBeStopped (const shared_ptr<CallSession> &session) {
	mBgTask.stop();
}

void Call::onCallSessionAccepting (const std::shared_ptr<CallSession> &session) {
	accept();
}

bool Call::onCallSessionAccepted (const shared_ptr<CallSession> &session) {
	LinphoneCore *lc = getCore()->getCCore();
	bool wasRinging = false;

	if (getCore()->getCurrentCall() != getSharedFromThis())
		linphone_core_preempt_sound_resources(lc);

	return wasRinging;
}

void Call::onCallSessionEarlyFailed (const shared_ptr<CallSession> &session, LinphoneErrorInfo *ei) {
	LinphoneCallLog *log = session->getLog();
	linphone_core_report_early_failed_call(getCore()->getCCore(),
		linphone_call_log_get_dir(log),
		linphone_address_clone(linphone_call_log_get_from(log)),
		linphone_address_clone(linphone_call_log_get_to(log)),
		ei,
		log->call_id);
	linphone_call_unref(this->toC());
}

void Call::onCallSessionSetReleased (const shared_ptr<CallSession> &session) {
	linphone_call_unref(this->toC());
}

void Call::onCallSessionSetTerminated (const shared_ptr<CallSession> &session) {
	LinphoneCore *core = getCore()->getCCore();
	
	if (getSharedFromThis() == getCore()->getCurrentCall()) {
		lInfo() << "Resetting the current call";
		getCore()->getPrivate()->setCurrentCall(nullptr);
	}
	if (getCore()->getPrivate()->removeCall(getSharedFromThis()) != 0)
		lError() << "Could not remove the call from the list!!!";
	if (getConference()) {
		lInfo() << "Removing terminated call from LinphoneConference " << getConference();
		MediaConference::Conference::toCpp(getConference())->removeParticipant(getSharedFromThis());
	}
#if 0
	if (mChatRoom)
		linphone_chat_room_set_call(mChatRoom, nullptr);
#endif // if 0
	if (!getCore()->getPrivate()->hasCalls())
		ms_bandwidth_controller_reset_state(core->bw_controller);
}

void Call::onCallSessionStartReferred (const shared_ptr<CallSession> &session) {
	startReferredCall(nullptr);
}

void Call::onCallSessionStateChanged (const shared_ptr<CallSession> &session, CallSession::State state, const string &message) {
	getCore()->getPrivate()->getToneManager()->update(session);
	LinphoneCore *lc = getCore()->getCCore();

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
		case CallSession::State::PausedByRemote:
		{
			// If it is not in a conference, the remote conference must be terminated if it exists
			if (!isInConference()) {
				char * remoteContactAddressStr = sal_address_as_string(session->getPrivate()->getOp()->getRemoteContactAddress());
				Address remoteContactAddress(remoteContactAddressStr);
				ms_free(remoteContactAddressStr);

				// Check if the request was sent by the focus
				ConferenceId remoteConferenceId = ConferenceId(remoteContactAddress, getLocalAddress());
				shared_ptr<MediaConference::Conference> conference = getCore()->findAudioVideoConference(remoteConferenceId, false);

				// Terminate conference is found
				if (conference != nullptr) {
					conference->setState(ConferenceInterface::State::TerminationPending);
				}
			}
		}
		break;
		case CallSession::State::End:
		case CallSession::State::Error:
		{
			char * remoteContactAddressStr = sal_address_as_string(session->getPrivate()->getOp()->getRemoteContactAddress());
			Address remoteContactAddress(remoteContactAddressStr);
			ms_free(remoteContactAddressStr);

			// Check if the request was sent by the focus
			ConferenceId remoteConferenceId = ConferenceId(remoteContactAddress, getLocalAddress());
			shared_ptr<MediaConference::Conference> conference = getCore()->findAudioVideoConference(remoteConferenceId, false);

			// Terminate conference is found
			if (conference != nullptr) {
				conference->setState(ConferenceInterface::State::TerminationPending);
			}

			if (linphone_core_get_calls_nb(lc) == 0) {
				linphone_core_notify_last_call_ended(lc);
			}
		}
		break;
		case CallSession::State::UpdatedByRemote:
		{
			char * remoteContactAddressStr = sal_address_as_string(session->getPrivate()->getOp()->getRemoteContactAddress());
			Address remoteContactAddress(remoteContactAddressStr);
			ms_free(remoteContactAddressStr);

			// Check if the request was sent by the focus
			if (remoteContactAddress.hasParam("isfocus")) {
				ConferenceId remoteConferenceId = ConferenceId(remoteContactAddress, getLocalAddress());
				// It is expected that the core of the remote conference is the participant one
				std::shared_ptr<MediaConference::RemoteConference>(new MediaConference::RemoteConference(getCore(), remoteContactAddress, remoteConferenceId, nullptr, ConferenceParams::create(getCore()->getCCore())), [](MediaConference::RemoteConference * c){c->unref();});
			}
		}
		break;
		case CallSession::State::StreamsRunning:
		{
			char * remoteContactAddressStr = sal_address_as_string(session->getPrivate()->getOp()->getRemoteContactAddress());
			Address remoteContactAddress(remoteContactAddressStr);
			ms_free(remoteContactAddressStr);

			// Check if the request was sent by the focus
			if (remoteContactAddress.hasParam("isfocus")) {
				ConferenceId remoteConferenceId = ConferenceId(remoteContactAddress, getLocalAddress());
				shared_ptr<MediaConference::Conference> conference = getCore()->findAudioVideoConference(remoteConferenceId, false);
				shared_ptr<MediaConference::RemoteConference> remoteConf = nullptr;

				// Create remote conference if no conference with the expected ID is found in the database
				if (conference == nullptr) {
					// It is expected that the core of the remote conference is the participant one
					remoteConf = std::shared_ptr<MediaConference::RemoteConference>(new MediaConference::RemoteConference(getCore(), remoteContactAddress, remoteConferenceId, nullptr, ConferenceParams::create(getCore()->getCCore())), [](MediaConference::RemoteConference * c){c->unref();});
				} else {
					remoteConf = static_pointer_cast<MediaConference::RemoteConference>(conference);
				}
				// Here, the conference subscribes to the handler
				remoteConf->finalizeCreation();
			}
		}
		break;
		default:
			break;
	}
	linphone_call_notify_state_changed(this->toC(), static_cast<LinphoneCallState>(state), message.c_str());
}

void Call::onCallSessionTransferStateChanged (const shared_ptr<CallSession> &session, CallSession::State state) {
	linphone_call_notify_transfer_state_changed(this->toC(), static_cast<LinphoneCallState>(state));
}

void Call::onCheckForAcceptation (const shared_ptr<CallSession> &session) {
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

void Call::onDtmfReceived (const shared_ptr<CallSession> &session, char dtmf) {
	linphone_call_notify_dtmf_received(this->toC(), dtmf);
}

void Call::onIncomingCallSessionNotified (const shared_ptr<CallSession> &session) {
	/* The call is acceptable so we can now add it to our list */
	getCore()->getPrivate()->addCall(getSharedFromThis());
}

void Call::onIncomingCallSessionStarted (const shared_ptr<CallSession> &session) {
	if (linphone_core_get_calls_nb(getCore()->getCCore()) == 1 && !isInConference()) {
		L_GET_PRIVATE_FROM_C_OBJECT(getCore()->getCCore())->setCurrentCall(getSharedFromThis());
	}
	getCore()->getPrivate()->getToneManager()->startRingtone(session);
}

void Call::onIncomingCallSessionTimeoutCheck (const shared_ptr<CallSession> &session, int elapsed, bool oneSecondElapsed) {
	if (oneSecondElapsed)
		lInfo() << "Incoming call ringing for " << elapsed << " seconds";
	if (elapsed > getCore()->getCCore()->sip_conf.inc_timeout) {
		lInfo() << "Incoming call timeout (" << getCore()->getCCore()->sip_conf.inc_timeout << ")";
		auto config = linphone_core_get_config(getCore()->getCCore());
		int statusCode = linphone_config_get_int(config, "sip", "inc_timeout_status_code", 486);
		getActiveSession()->declineNotAnswered(linphone_error_code_to_reason(statusCode));
	}
}

void Call::onPushCallSessionTimeoutCheck (const std::shared_ptr<CallSession> &session, int elapsed) {
	if (elapsed > getCore()->getCCore()->sip_conf.push_incoming_call_timeout) {
		lInfo() << "Push incoming call timeout (" << getCore()->getCCore()->sip_conf.push_incoming_call_timeout << ")";
		auto config = linphone_core_get_config(getCore()->getCCore());
		int statusCode = linphone_config_get_int(config, "sip", "push_incoming_call_timeout_status_code", 410); //LinphoneReasonGone
		getActiveSession()->decline(linphone_error_code_to_reason(statusCode));
		getActiveSession()->getPrivate()->setState(LinphonePrivate::CallSession::State::Released, "Call released");
	}
}

void Call::onInfoReceived (const shared_ptr<CallSession> &session, const LinphoneInfoMessage *im) {
	linphone_call_notify_info_message_received(this->toC(), im);
}

void Call::onLossOfMediaDetected (const shared_ptr<CallSession> &session) {
	terminateBecauseOfLostMedia();
}

void Call::onEncryptionChanged (const shared_ptr<CallSession> &session, bool activated, const string &authToken) {
	linphone_call_notify_encryption_changed(this->toC(), activated, authToken.empty() ? nullptr : authToken.c_str());
}

void Call::onCallSessionStateChangedForReporting (const shared_ptr<CallSession> &session) {
	linphone_reporting_call_state_updated(this->toC());
}

void Call::onRtcpUpdateForReporting (const shared_ptr<CallSession> &session, SalStreamType type) {
	linphone_reporting_on_rtcp_update(this->toC(), type);
}

void Call::onStatsUpdated (const shared_ptr<CallSession> &session, const LinphoneCallStats *stats) {
	linphone_call_notify_stats_updated(this->toC(), stats);
}

void Call::onUpdateMediaInfoForReporting (const shared_ptr<CallSession> &session, int statsType) {
	linphone_reporting_update_media_info(this->toC(), statsType);
}

void Call::onResetCurrentSession (const shared_ptr<CallSession> &session) {
	getCore()->getPrivate()->setCurrentCall(nullptr);
}

void Call::onSetCurrentSession (const shared_ptr<CallSession> &session) {
	getCore()->getPrivate()->setCurrentCall(getSharedFromThis());
}

void Call::onFirstVideoFrameDecoded (const shared_ptr<CallSession> &session) {
	if (mNextVideoFrameDecoded._func) {
		mNextVideoFrameDecoded._func(this->toC(), mNextVideoFrameDecoded._user_data);
		mNextVideoFrameDecoded._func = nullptr;
		mNextVideoFrameDecoded._user_data = nullptr;
	}
	linphone_call_notify_next_video_frame_decoded(this->toC());
}

void Call::onResetFirstVideoFrameDecoded (const shared_ptr<CallSession> &session) {
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

void Call::onCameraNotWorking (const std::shared_ptr<CallSession> &session, const char *camera_name) {
	linphone_call_notify_camera_not_working(this->toC(), camera_name);
}

bool Call::areSoundResourcesAvailable (const shared_ptr<CallSession> &session) {
	LinphoneCore *lc = getCore()->getCCore();
	shared_ptr<Call> currentCall = getCore()->getCurrentCall();
	return !linphone_core_is_in_conference(lc) && (!currentCall || (currentCall == getSharedFromThis()));
}

bool Call::isPlayingRingbackTone (const shared_ptr<CallSession> &session) {
	return mPlayingRingbackTone;
}

void Call::onRealTimeTextCharacterReceived (const shared_ptr<CallSession> &session, RealtimeTextReceivedCharacter *data) {
	shared_ptr<RealTimeTextChatRoom> cr = getChatRoom();
	if (cr) cr->getPrivate()->realtimeTextReceived(data->character, getSharedFromThis());
	else lError()<<"CallPrivate::onRealTimeTextCharacterReceived: no chatroom.";
}

void Call::onTmmbrReceived (const shared_ptr<CallSession> &session, int streamIndex, int tmmbr) {
	linphone_call_notify_tmmbr_received(this->toC(), streamIndex, tmmbr);
}

void Call::onSnapshotTaken(const shared_ptr<CallSession> &session, const char *file_path) {
	linphone_call_notify_snapshot_taken(this->toC(), file_path);
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
}

Call::~Call () {
	auto session = getActiveSession();
	if (session)
		session->getPrivate()->setCallSessionListener(nullptr);
	
	bctbx_list_free_with_data(mCallbacks, (bctbx_list_free_func)linphone_call_cbs_unref);
}

void Call::configure (
	LinphoneCallDir direction,
	const Address &from,
	const Address &to,
	LinphoneProxyConfig *cfg,
	SalCallOp *op,
	const MediaSessionParams *msp
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

LinphoneStatus Call::pause () {
	return static_pointer_cast<MediaSession>(getActiveSession())->pause();
}

LinphoneStatus Call::redirect (const string &redirectUri) {
	return getActiveSession()->redirect(redirectUri);
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

void Call::startRecording () {
	static_pointer_cast<MediaSession>(getActiveSession())->startRecording();
}

void Call::stopRecording () {
	static_pointer_cast<MediaSession>(getActiveSession())->stopRecording();
}

bool Call::isRecording () {
	return static_pointer_cast<MediaSession>(getActiveSession())->isRecording();
}

LinphoneStatus Call::takePreviewSnapshot (const string &file) {
	return static_pointer_cast<MediaSession>(getActiveSession())->takePreviewSnapshot(file);
}

LinphoneStatus Call::takeVideoSnapshot (const string &file) {
	return static_pointer_cast<MediaSession>(getActiveSession())->takeVideoSnapshot(file);
}

LinphoneStatus Call::terminate (const LinphoneErrorInfo *ei) {
	return getActiveSession()->terminate(ei);
}

LinphoneStatus Call::transfer (const shared_ptr<Call> &dest) {
	return getActiveSession()->transfer(dest->getActiveSession());
}

LinphoneStatus Call::transfer (const string &dest) {
	return getActiveSession()->transfer(dest);
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

LinphoneCallLog *Call::getLog () const {
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

const Address *Call::getRemoteAddress () const {
	return getActiveSession()->getRemoteAddress();
}

const string &Call::getRemoteContact () {
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

bool Call::isInConference () const {
	return getActiveSession()->getPrivate()->isInConference();
}

bool Call::mediaInProgress () const {
	return static_pointer_cast<const MediaSession>(getActiveSession())->mediaInProgress();
}

void Call::setAudioRoute (LinphoneAudioRoute route) {
	static_pointer_cast<MediaSession>(getActiveSession())->setAudioRoute(route);
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

void Call::setInputAudioDevice(AudioDevice *audioDevice) {
	setInputAudioDevicePrivate(audioDevice);

	linphone_call_notify_audio_device_changed(getSharedFromThis()->toC(), audioDevice->toC());

}

void Call::setOutputAudioDevice(AudioDevice *audioDevice) {
	setOutputAudioDevicePrivate(audioDevice);

	linphone_call_notify_audio_device_changed(getSharedFromThis()->toC(), audioDevice->toC());
}

AudioDevice* Call::getInputAudioDevice() const {
	return static_pointer_cast<MediaSession>(getActiveSession())->getInputAudioDevice();
}

AudioDevice* Call::getOutputAudioDevice() const {
	RingStream *ringStream = nullptr;
	switch (getState()) {
		case CallSession::State::OutgoingRinging:
		case CallSession::State::Pausing:
		case CallSession::State::Paused:
			ringStream = getCore()->getCCore()->ringstream;
			if (ringStream) {
				MSSndCard *card = ring_stream_get_output_ms_snd_card(ringStream);
				if (card) {
					return getCore()->findAudioDeviceMatchingMsSoundCard(card);
				}
			}
			break;
		case CallSession::State::IncomingReceived:
			ringStream = linphone_ringtoneplayer_get_stream(getCore()->getCCore()->ringtoneplayer);
			if (ringStream) {
				MSSndCard *card = ring_stream_get_output_ms_snd_card(ringStream);
				if (card) {
					return getCore()->findAudioDeviceMatchingMsSoundCard(card);
				}
			}
			break;
		default:
			return static_pointer_cast<MediaSession>(getActiveSession())->getOutputAudioDevice();
	}

	return nullptr;
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

bctbx_list_t *Call::getCallbacksList () const {
	return mCallbacks;
}

LinphoneCallCbs *Call::getCurrentCbs () const{
	return mCurrentCbs;
}

void Call::setCurrentCbs (LinphoneCallCbs *cbs) {
	mCurrentCbs = cbs;
}

void Call::addCallbacks (LinphoneCallCbs *cbs) {
	mCallbacks = bctbx_list_append(mCallbacks, belle_sip_object_ref(cbs));
}

void Call::removeCallbacks (LinphoneCallCbs *cbs) {
	mCallbacks = bctbx_list_remove(mCallbacks, cbs);
	belle_sip_object_unref(cbs);
}

void *Call::getUserData () const{
	return mUserData;
}

void Call::setUserData (void *ud) {
	mUserData = ud;
}

LINPHONE_END_NAMESPACE
