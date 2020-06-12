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

#ifndef _L_CALL_CALL_H_
#define _L_CALL_CALL_H_

#include "conference/params/media-session-params.h"
#include "conference/session/call-session.h"
#include "core/core-accessor.h"
#include "object/object.h"

#include <belle-sip/object++.hh>
#include "linphone/api/c-types.h"

#include "object/object-p.h"

#include "conference/session/call-session-listener.h"
#include "utils/background-task.h"

// TODO: Remove me later.
#include "private.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Address;
class CallSessionPrivate;
class MediaSessionPrivate;
class RealTimeTextChatRoom;
class ConferencePrivate;

class Call : public bellesip::HybridObject<LinphoneCall, Call>, public CoreAccessor, public CallSessionListener {
public:
	Call (
		std::shared_ptr<Core> core,
		LinphoneCallDir direction,
		const Address &from,
		const Address &to,
		LinphoneProxyConfig *cfg,
		SalCallOp *op,
		const MediaSessionParams *msp
	);

	Call (
		std::shared_ptr<Core> core,
		LinphoneCallDir direction,
		const std::string &callid
	);

	~Call ();

	void configure (
		LinphoneCallDir direction,
		const Address &from,
		const Address &to,
		LinphoneProxyConfig *cfg,
		SalCallOp *op,
		const MediaSessionParams *msp
	);

	bool isOpConfigured () const;

	LinphoneStatus accept (const MediaSessionParams *msp = nullptr);
	LinphoneStatus acceptEarlyMedia (const MediaSessionParams *msp = nullptr);
	std::shared_ptr<MediaSession> getMediaSession()const;
	LinphoneStatus acceptUpdate (const MediaSessionParams *msp);
	void cancelDtmfs ();
	LinphoneStatus decline (LinphoneReason reason);
	LinphoneStatus decline (const LinphoneErrorInfo *ei);
	LinphoneStatus deferUpdate ();
	bool hasTransferPending () const;
	void oglRender () const;
	LinphoneStatus pause ();
	LinphoneStatus redirect (const std::string &redirectUri);
	LinphoneStatus resume ();
	LinphoneStatus sendDtmf (char dtmf);
	LinphoneStatus sendDtmfs (const std::string &dtmfs);
	void sendVfuRequest ();
	void startRecording ();
	void stopRecording ();
	bool isRecording ();
	LinphoneStatus takePreviewSnapshot (const std::string &file);
	LinphoneStatus takeVideoSnapshot (const std::string &file);
	LinphoneStatus terminate (const LinphoneErrorInfo *ei = nullptr);
	LinphoneStatus transfer (const std::shared_ptr<Call> &dest);
	LinphoneStatus transfer (const std::string &dest);
	LinphoneStatus update (const MediaSessionParams *msp = nullptr);
	void zoomVideo (float zoomFactor, float *cx, float *cy);
	void zoomVideo (float zoomFactor, float cx, float cy);

	bool cameraEnabled () const;
	bool echoCancellationEnabled () const;
	bool echoLimiterEnabled () const;
	void enableCamera (bool value);
	void enableEchoCancellation (bool value);
	void enableEchoLimiter (bool value);
	bool getAllMuted () const;
	LinphoneCallStats *getAudioStats () const;
	const std::string &getAuthenticationToken () ;
	bool getAuthenticationTokenVerified () const;
	float getAverageQuality () const;
	const MediaSessionParams *getCurrentParams () const;
	float getCurrentQuality () const;
	LinphoneCallDir getDirection () const;
	const Address &getDiversionAddress () const;
	int getDuration () const;
	const LinphoneErrorInfo *getErrorInfo () const;
	const Address &getLocalAddress () const;
	LinphoneCallLog *getLog () const;
	RtpTransport *getMetaRtcpTransport (int streamIndex) const;
	RtpTransport *getMetaRtpTransport (int streamIndex) const;
	float getMicrophoneVolumeGain () const;
	void *getNativeVideoWindowId () const;
	const MediaSessionParams *getParams () const;
	LinphonePlayer *getPlayer () const;
	float getPlayVolume () const;
	LinphoneReason getReason () const;
	float getRecordVolume () const;
	std::shared_ptr<Call> getReferer () const;
	const std::string &getReferTo ();
	const Address *getRemoteAddress () const;
	const std::string &getRemoteContact ();
	const MediaSessionParams *getRemoteParams () const;
	const std::string &getRemoteUserAgent ();
	std::shared_ptr<Call> getReplacedCall () const;
	float getSpeakerVolumeGain () const;
	CallSession::State getState () const;
	LinphoneCallStats *getStats (LinphoneStreamType type) const;
	LinphoneCallStats *getPrivateStats (LinphoneStreamType type) const;
	int getStreamCount () const;
	MSFormatType getStreamType (int streamIndex) const;
	LinphoneCallStats *getTextStats () const;
	const Address &getToAddress () const;
	const char *getToHeader (const std::string &name);
	CallSession::State getTransferState () const;
	std::shared_ptr<Call> getTransferTarget () const;
	LinphoneCallStats *getVideoStats () const;
	bool isInConference () const;
	bool mediaInProgress () const;
	void setAudioRoute (LinphoneAudioRoute route);
	void setAuthenticationTokenVerified (bool value);
	void setMicrophoneVolumeGain (float value);
	void setNativeVideoWindowId (void *id);
	void setNextVideoFrameDecodedCallback (LinphoneCallCbFunc cb, void *user_data);
	void requestNotifyNextVideoFrameDecoded ();
	void setParams (const MediaSessionParams *msp);
	void setSpeakerVolumeGain (float value);
	
	// -----------------------------------------------------------------------------
	
	void setInputAudioDevice(AudioDevice *audioDevice);
	void setInputAudioDevicePrivate(AudioDevice *audioDevice);
	void setOutputAudioDevice(AudioDevice *audioDevice);
	void setOutputAudioDevicePrivate(AudioDevice *audioDevice);
	AudioDevice *getInputAudioDevice() const;
	AudioDevice *getOutputAudioDevice() const;
	
	// -----------------------------------------------------------------------------
	void createPlayer () const;
	void initiateIncoming ();
	bool initiateOutgoing ();
	void iterate (time_t currentRealTime, bool oneSecondElapsed);
	void startIncomingNotification ();
	void startPushIncomingNotification ();
	void startBasicIncomingNotification ();
	void pauseForTransfer ();
	int startInvite (const Address *destination);
	std::shared_ptr<Call> startReferredCall (const MediaSessionParams *params);
	
	// -----------------------------------------------------------------------------
	std::shared_ptr<CallSession> getActiveSession () const;
	std::shared_ptr<RealTimeTextChatRoom> getChatRoom ();
	LinphoneProxyConfig *getDestProxy () const;
	IceSession *getIceSession () const;
	unsigned int getAudioStartCount () const;
	unsigned int getVideoStartCount () const;
	unsigned int getTextStartCount () const;
	// don't make new code relying on this method.
	MediaStream *getMediaStream (LinphoneStreamType type) const;
	SalCallOp *getOp () const;
	bool getSpeakerMuted () const;
	void setSpeakerMuted (bool muted);
	bool getMicrophoneMuted () const;
	void setMicrophoneMuted (bool muted);
	
	// -----------------------------------------------------------------------------
	void startRemoteRing ();
	void terminateBecauseOfLostMedia ();
	
	// -----------------------------------------------------------------------------
	/* CallSessionListener */
	void onAckBeingSent (const std::shared_ptr<CallSession> &session, LinphoneHeaders *headers) override;
	void onAckReceived (const std::shared_ptr<CallSession> &session, LinphoneHeaders *headers) override;
	void onBackgroundTaskToBeStarted (const std::shared_ptr<CallSession> &session) override;
	void onBackgroundTaskToBeStopped (const std::shared_ptr<CallSession> &session) override;
	void onCallSessionAccepting (const std::shared_ptr<CallSession> &session) override;
	bool onCallSessionAccepted (const std::shared_ptr<CallSession> &session) override;
	void onCallSessionEarlyFailed (const std::shared_ptr<CallSession> &session, LinphoneErrorInfo *ei) override;
	void onCallSessionSetReleased (const std::shared_ptr<CallSession> &session) override;
	void onCallSessionSetTerminated (const std::shared_ptr<CallSession> &session) override;
	void onCallSessionStartReferred (const std::shared_ptr<CallSession> &session) override;
	void onCallSessionStateChanged (const std::shared_ptr<CallSession> &session, CallSession::State state, const std::string &message) override;
	void onCallSessionTransferStateChanged (const std::shared_ptr<CallSession> &session, CallSession::State state) override;
	void onCheckForAcceptation (const std::shared_ptr<CallSession> &session) override;
	void onDtmfReceived (const std::shared_ptr<CallSession> &session, char dtmf) override;
	void onIncomingCallSessionNotified (const std::shared_ptr<CallSession> &session) override;
	void onIncomingCallSessionStarted (const std::shared_ptr<CallSession> &session) override;
	void onIncomingCallSessionTimeoutCheck (const std::shared_ptr<CallSession> &session, int elapsed, bool oneSecondElapsed) override;
	void onPushCallSessionTimeoutCheck (const std::shared_ptr<CallSession> &session, int elapsed) override;
	void onInfoReceived (const std::shared_ptr<CallSession> &session, const LinphoneInfoMessage *im) override;
	void onLossOfMediaDetected (const std::shared_ptr<CallSession> &session) override;
	void onEncryptionChanged (const std::shared_ptr<CallSession> &session, bool activated, const std::string &authToken) override;
	void onCallSessionStateChangedForReporting (const std::shared_ptr<CallSession> &session) override;
	void onRtcpUpdateForReporting (const std::shared_ptr<CallSession> &session, SalStreamType type) override;
	void onStatsUpdated (const std::shared_ptr<CallSession> &session, const LinphoneCallStats *stats) override;
	void onUpdateMediaInfoForReporting (const std::shared_ptr<CallSession> &session, int statsType) override;
	void onResetCurrentSession (const std::shared_ptr<CallSession> &session) override;
	void onSetCurrentSession (const std::shared_ptr<CallSession> &session) override;
	void onFirstVideoFrameDecoded (const std::shared_ptr<CallSession> &session) override;
	void onResetFirstVideoFrameDecoded (const std::shared_ptr<CallSession> &session) override;
	void onCameraNotWorking (const std::shared_ptr<CallSession> &session, const char *camera_name) override;
	bool areSoundResourcesAvailable (const std::shared_ptr<CallSession> &session) override;
	bool isPlayingRingbackTone (const std::shared_ptr<CallSession> &session) override;
	void onRealTimeTextCharacterReceived (const std::shared_ptr<CallSession> &session, RealtimeTextReceivedCharacter *character) override;
	void onTmmbrReceived(const std::shared_ptr<CallSession> &session, int streamIndex, int tmmbr) override;
	void onSnapshotTaken(const std::shared_ptr<CallSession> &session, const char *file_path) override;


	LinphoneConference *getConference () const;
	void setConference (LinphoneConference *ref);
	MSAudioEndpoint *getEndpoint () const;
	void setEndpoint (MSAudioEndpoint *endpoint);
	bctbx_list_t *getCallbacksList () const;
	LinphoneCallCbs *getCurrentCbs () const;
	void setCurrentCbs (LinphoneCallCbs *cbs);
	void addCallbacks (LinphoneCallCbs *cbs);
	void removeCallbacks (LinphoneCallCbs *cbs);
	
	void *getUserData () const;
	void setUserData (void *ud);
	

private:
	std::shared_ptr<Participant> mParticipant;
	mutable LinphonePlayer *mPlayer = nullptr;
	CallCallbackObj mNextVideoFrameDecoded;
	mutable std::shared_ptr<RealTimeTextChatRoom> mChatRoom = nullptr;
	bool mPlayingRingbackTone = false;

	BackgroundTask mBgTask;
	
	bctbx_list_t *mCallbacks = nullptr;
	LinphoneCallCbs *mCurrentCbs = nullptr;
	LinphoneConference *mConfRef = nullptr;
	MSAudioEndpoint *mEndpoint = nullptr;
	
	void *mUserData = nullptr;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CALL_CALL_H_
