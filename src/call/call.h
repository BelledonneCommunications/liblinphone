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

#ifndef _L_CALL_CALL_H_
#define _L_CALL_CALL_H_

#include "conference/params/media-session-params.h"
#include "conference/session/call-session.h"
#include "core/core-accessor.h"
#include "object/object.h"

#include <c-wrapper/c-wrapper.h>
#include "linphone/api/c-types.h"

#include "object/object-p.h"

//#include "call/audio-device/audio-device.h"
#include "conference/session/call-session-listener.h"
#include "utils/background-task.h"

#include "call/call-log.h"
#include "call/video-source/video-source-descriptor.h"

// TODO: Remove me later.
#include "private.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Address;
class CallSessionPrivate;
class MediaSessionPrivate;
class AbstractChatRoom;
class ConferencePrivate;

namespace MediaConference {
	class Conference;
}

class CallCbs : public bellesip::HybridObject<LinphoneCallCbs, CallCbs>, public Callbacks{
public:
	LinphoneCallCbsDtmfReceivedCb dtmfReceivedCb;
	LinphoneCallCbsGoClearAckSentCb goClearAckSentCb;
	LinphoneCallCbsEncryptionChangedCb encryptionChangedCb;
	LinphoneCallCbsInfoMessageReceivedCb infoMessageReceivedCb;
	LinphoneCallCbsStateChangedCb stateChangedCb;
	LinphoneCallCbsStatsUpdatedCb statsUpdatedCb;
	LinphoneCallCbsTransferStateChangedCb transferStateChangedCb;
	LinphoneCallCbsAckProcessingCb ackProcessing;
	LinphoneCallCbsTmmbrReceivedCb tmmbrReceivedCb;
	LinphoneCallCbsSnapshotTakenCb snapshotTakenCb;
	LinphoneCallCbsNextVideoFrameDecodedCb nextVideoFrameDecodedCb;
	LinphoneCallCbsCameraNotWorkingCb cameraNotWorkingCb;
	LinphoneCallCbsAudioDeviceChangedCb audioDeviceChangedCb;
	LinphoneCallCbsRemoteRecordingCb remoteRecordingCb;
};

class LINPHONE_PUBLIC Call : public bellesip::HybridObject<LinphoneCall, Call>, 
				public CoreAccessor, 
				public CallSessionListener,
				public CallbacksHolder<CallCbs>,
				public UserDataAccessor {
	friend class CallSessionPrivate;
	friend class ChatMessage;
	friend class ChatMessagePrivate;
	friend class CorePrivate;
	friend class MediaSessionPrivate;
	friend class Stream;

	friend class MediaConference::Conference;
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
	LinphoneStatus pauseFromConference ();
	LinphoneStatus pause ();
	LinphoneStatus redirect (const std::string &redirectUri);
	LinphoneStatus redirect (const Address &redirectAddress);
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
	LinphoneStatus transfer (const Address &dest);
	LinphoneStatus transfer (const std::string &dest);
	LinphoneStatus update (const MediaSessionParams *msp = nullptr);
	LinphoneStatus updateFromConference (const MediaSessionParams *msp = nullptr);
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
	std::shared_ptr<CallLog> getLog () const;
	RtpTransport *getMetaRtcpTransport (int streamIndex) const;
	RtpTransport *getMetaRtpTransport (int streamIndex) const;
	float getMicrophoneVolumeGain () const;
	void *getNativeVideoWindowId () const;
	void *createNativeVideoWindowId () const;
	const MediaSessionParams *getParams () const;
	LinphonePlayer *getPlayer () const;
	float getPlayVolume () const;
	LinphoneReason getReason () const;
	float getRecordVolume () const;
	std::shared_ptr<Call> getReferer () const;
	const std::string &getReferTo () const;
	const Address &getReferToAddress () const;
	const Address *getRemoteAddress () const;
	const Address *getRemoteContactAddress () const;
	const std::string &getRemoteContact () const;
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
	const char *getToHeader (const std::string &name) const;
	CallSession::State getTransferState () const;
	std::shared_ptr<Call> getTransferTarget () const;
	LinphoneCallStats *getVideoStats () const;
	bool isInConference () const;
	std::string getConferenceId () const;
	void setConferenceId (const std::string & conferenceId);
	bool mediaInProgress () const;
	void setAuthenticationTokenVerified (bool value);
	void setMicrophoneVolumeGain (float value);
	void setNativeVideoWindowId (void *id);
	void setNextVideoFrameDecodedCallback (LinphoneCallCbFunc cb, void *user_data);
	void requestNotifyNextVideoFrameDecoded ();
	void setParams (const MediaSessionParams *msp);
	void setSpeakerVolumeGain (float value);
	
	// -----------------------------------------------------------------------------
	
	void setInputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice);
	bool setInputAudioDevicePrivate(const std::shared_ptr<AudioDevice> &audioDevice);
	void setOutputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice);
	bool setOutputAudioDevicePrivate(const std::shared_ptr<AudioDevice> &audioDevice);
	std::shared_ptr<AudioDevice> getInputAudioDevice() const;
	std::shared_ptr<AudioDevice> getOutputAudioDevice() const;
	
	// -----------------------------------------------------------------------------

	void createPlayer () const;
	void initiateIncoming ();
	bool initiateOutgoing (const std::string &subject = "", const Content *content = nullptr);
	void iterate (time_t currentRealTime, bool oneSecondElapsed);
	void notifyRinging ();
	void startIncomingNotification ();
	void startPushIncomingNotification ();
	void startBasicIncomingNotification ();
	void pauseForTransfer ();
	int startInvite (const Address *destination, const std::string subject = std::string(), const Content *content = nullptr);
	std::shared_ptr<Call> startReferredCall (const MediaSessionParams *params);
	
	// -----------------------------------------------------------------------------

	std::shared_ptr<CallSession> getActiveSession () const;
	std::shared_ptr<AbstractChatRoom> getChatRoom ();
	LinphoneProxyConfig *getDestProxy () const;
	IceSession *getIceSession () const;
	unsigned int getAudioStartCount () const;
	unsigned int getAudioStopCount () const;
	unsigned int getVideoStartCount () const;
	unsigned int getTextStartCount () const;
	// don't make new code relying on this method.
	MediaStream *getMediaStream (LinphoneStreamType type) const;
	int getMediaStreamIndex (LinphoneStreamType type) const;
	int getMediaStreamsNb (LinphoneStreamType type) const;
	SalCallOp *getOp () const;
	bool getSpeakerMuted () const;
	void setSpeakerMuted (bool muted);
	bool getMicrophoneMuted () const;
	void setMicrophoneMuted (bool muted);
	
	// -----------------------------------------------------------------------------

	void terminateBecauseOfLostMedia ();

	// -----------------------------------------------------------------------------

	void setVideoSource (std::shared_ptr<const VideoSourceDescriptor> descriptor);
	std::shared_ptr<const VideoSourceDescriptor> getVideoSource () const;

	// -----------------------------------------------------------------------------
	/* CallSessionListener */
	void onAckBeingSent (const std::shared_ptr<CallSession> &session, LinphoneHeaders *headers) override;
	void onAckReceived (const std::shared_ptr<CallSession> &session, LinphoneHeaders *headers) override;
	void onBackgroundTaskToBeStarted (const std::shared_ptr<CallSession> &session) override;
	void onBackgroundTaskToBeStopped (const std::shared_ptr<CallSession> &session) override;
	void onCallSessionAccepting (const std::shared_ptr<CallSession> &session) override;
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
	void onGoClearAckSent() override;
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
	LinphoneConference *getCallSessionConference (const std::shared_ptr<CallSession> &session) const override;
	void onRealTimeTextCharacterReceived (const std::shared_ptr<CallSession> &session, RealtimeTextReceivedCharacter *character) override;
	void onTmmbrReceived(const std::shared_ptr<CallSession> &session, int streamIndex, int tmmbr) override;
	void onSnapshotTaken(const std::shared_ptr<CallSession> &session, const char *file_path) override;
	void onStartRingtone(const std::shared_ptr<CallSession> &session) override;
	void onRemoteRecording(const std::shared_ptr<CallSession> &session, bool recording) override;

	void confirmGoClear() const override;

	LinphoneConference *getConference () const;
	void reenterLocalConference(const std::shared_ptr<CallSession> &session);
	void setConference (LinphoneConference *ref);
	MSAudioEndpoint *getEndpoint () const;
	void setEndpoint (MSAudioEndpoint *endpoint);
	bool canSoundResourcesBeFreed () const;
	const std::list<LinphoneMediaEncryption> getSupportedEncryptions() const;
	const LinphoneStreamInternalStats *getStreamInternalStats(LinphoneStreamType type)const;

private:
	std::shared_ptr<Participant> mParticipant;
	mutable LinphonePlayer *mPlayer = nullptr;
	CallCallbackObj mNextVideoFrameDecoded;
	mutable std::shared_ptr<AbstractChatRoom> mChatRoom = nullptr;
	bool mPlayingRingbackTone = false;

	BackgroundTask mBgTask;
	LinphoneConference *mConfRef = nullptr;
	MSAudioEndpoint *mEndpoint = nullptr;

	void cleanupSessionAndUnrefCObjectCall();

	void updateRecordState(SalMediaRecord state);
	void createRemoteConference(const std::shared_ptr<CallSession> &session);
	void tryToAddToConference(std::shared_ptr<MediaConference::Conference> & conference, const std::shared_ptr<CallSession> &session);
	void configureSoundCardsFromCore(const MediaSessionParams *msp);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CALL_CALL_H_
