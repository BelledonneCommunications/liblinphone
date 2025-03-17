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

#include "alert/alert.h"
#include "c-wrapper/c-wrapper.h"
#include "call/call-log.h"
#include "call/video-source/video-source-descriptor.h"
#include "conference/params/media-session-params.h"
#include "conference/session/call-session-listener.h"
#include "conference/session/call-session.h"
#include "core/core-accessor.h"
#include "linphone/api/c-types.h"
#include "object/object-p.h"
#include "object/object.h"
#include "utils/background-task.h"

// TODO: Remove me later.
#include "private.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Address;
class CallSessionPrivate;
class Conference;
class MediaSessionPrivate;
class AbstractChatRoom;
class Player;

class CallCbs : public bellesip::HybridObject<LinphoneCallCbs, CallCbs>, public Callbacks {
public:
	LinphoneCallCbsDtmfReceivedCb dtmfReceivedCb;
	LinphoneCallCbsGoClearAckSentCb goClearAckSentCb;
	LinphoneCallCbsSecurityLevelDowngradedCb securityLevelDowngradedCb;
	LinphoneCallCbsEncryptionChangedCb encryptionChangedCb;
	LinphoneCallCbsAuthenticationTokenVerifiedCb authenticationTokenVerifiedCb;
	LinphoneCallCbsSendMasterKeyChangedCb sendMasterKeyChangedCb;
	LinphoneCallCbsReceiveMasterKeyChangedCb receiveMasterKeyChangedCb;
	LinphoneCallCbsInfoMessageReceivedCb infoMessageReceivedCb;
	LinphoneCallCbsStateChangedCb stateChangedCb;
	LinphoneCallCbsStatsUpdatedCb statsUpdatedCb;
	LinphoneCallCbsTransferStateChangedCb transferStateChangedCb;
	LinphoneCallCbsReferRequestedCb referRequestedCb;
	LinphoneCallCbsAckProcessingCb ackProcessing;
	LinphoneCallCbsTmmbrReceivedCb tmmbrReceivedCb;
	LinphoneCallCbsSnapshotTakenCb snapshotTakenCb;
	LinphoneCallCbsNextVideoFrameDecodedCb nextVideoFrameDecodedCb;
	LinphoneCallCbsCameraNotWorkingCb cameraNotWorkingCb;
	LinphoneCallCbsVideoDisplayErrorOccurredCb videoDisplayErrorOccurredCb;
	LinphoneCallCbsAudioDeviceChangedCb audioDeviceChangedCb;
	LinphoneCallCbsRemoteRecordingCb remoteRecordingCb;
	LinphoneCallCbsBaudotDetectedCb baudotDetectedCb;
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
	friend class Conference;

public:
	Call(std::shared_ptr<Core> core);

	virtual ~Call() = default;

	void configure(LinphoneCallDir direction, const std::string &callid);
	void configure(LinphoneCallDir direction,
	               const std::shared_ptr<const Address> &from,
	               const std::shared_ptr<const Address> &to,
	               const std::shared_ptr<Account> &account,
	               SalCallOp *op,
	               const MediaSessionParams *msp);

	bool isOpConfigured() const;

	LinphoneStatus accept(const MediaSessionParams *msp = nullptr);
	LinphoneStatus acceptEarlyMedia(const MediaSessionParams *msp = nullptr);
	std::shared_ptr<MediaSession> getMediaSession() const;
	LinphoneStatus acceptUpdate(const MediaSessionParams *msp);
	void cancelDtmfs();
	LinphoneStatus decline(LinphoneReason reason);
	LinphoneStatus decline(const LinphoneErrorInfo *ei);
	LinphoneStatus deferUpdate();
	bool hasTransferPending() const;
	void oglRender() const;
	LinphoneStatus pause();
	LinphoneStatus redirect(const std::string &redirectUri);
	LinphoneStatus redirect(const std::shared_ptr<Address> &redirectAddress);
	LinphoneStatus resume();
	LinphoneStatus sendDtmf(char dtmf);
	LinphoneStatus sendDtmfs(const std::string &dtmfs);
	void sendVfuRequest();
	void startRecording();
	void stopRecording();
	bool isRecording();
	LinphoneStatus takePreviewSnapshot(const std::string &file);
	LinphoneStatus takeVideoSnapshot(const std::string &file);
	LinphoneStatus terminate(const LinphoneErrorInfo *ei = nullptr);
	LinphoneStatus transfer(const std::shared_ptr<Call> &dest);
	LinphoneStatus transfer(const Address &dest);
	LinphoneStatus transfer(const std::string &dest);
	LinphoneStatus update(const MediaSessionParams *msp = nullptr);
	LinphoneStatus updateFromConference(const MediaSessionParams *msp = nullptr);
	void zoomVideo(float zoomFactor, float *cx, float *cy);
	void zoomVideo(float zoomFactor, float cx, float cy);

	bool cameraEnabled() const;
	bool echoCancellationEnabled() const;
	bool echoLimiterEnabled() const;
	void enableCamera(bool value);
	void enableEchoCancellation(bool value);
	void enableEchoLimiter(bool value);
	bool getAllMuted() const;
	std::shared_ptr<CallStats> getAudioStats() const;
	const std::string &getAuthenticationToken() const;
	const std::string &forgeLocalAuthenticationToken() const;
	const std::string &forgeRemoteAuthenticationToken() const;
	void storeAndSortRemoteAuthToken(const std::string &remoteAuthToken) const;
	const std::list<std::string> &getRemoteAuthenticationTokens() const;
	const bctbx_list_t *getCListRemoteAuthenticationTokens() const;
	bool getAuthenticationTokenVerified() const;
	void skipZrtpAuthentication();
	bool getZrtpCacheMismatch() const;
	float getAverageQuality() const;
	const MediaSessionParams *getCurrentParams() const;
	float getCurrentQuality() const;
	LinphoneCallDir getDirection() const;
	std::shared_ptr<const Address> getDiversionAddress() const;
	int getDuration() const;
	const LinphoneErrorInfo *getErrorInfo() const;
	const std::shared_ptr<Address> getLocalAddress() const;
	std::shared_ptr<CallLog> getLog() const;
	RtpTransport *getMetaRtcpTransport(int streamIndex) const;
	RtpTransport *getMetaRtpTransport(int streamIndex) const;
	float getMicrophoneVolumeGain() const;
	void *getNativeVideoWindowId() const;
	void *createNativeVideoWindowId() const;
	const MediaSessionParams *getParams() const;
	std::shared_ptr<Player> getPlayer() const;
	float getPlayVolume() const;
	LinphoneReason getReason() const;
	float getRecordVolume() const;
	std::shared_ptr<Call> getReferer() const;
	const std::string &getReferTo() const;
	std::shared_ptr<Address> getReferToAddress() const;
	std::shared_ptr<const Address> getReferredBy() const;
	std::shared_ptr<Address> getRemoteAddress() const;
	std::shared_ptr<Address> getRemoteContactAddress() const;
	const std::string &getRemoteContact() const;
	const MediaSessionParams *getRemoteParams() const;
	const std::string &getRemoteUserAgent();
	std::shared_ptr<Call> getReplacedCall() const;
	float getSpeakerVolumeGain() const;
	CallSession::State getState() const;
	std::shared_ptr<CallStats> getStats(LinphoneStreamType type) const;
	std::shared_ptr<CallStats> getPrivateStats(LinphoneStreamType type) const;
	int getStreamCount() const;
	MSFormatType getStreamType(int streamIndex) const;
	std::shared_ptr<CallStats> getTextStats() const;
	const std::shared_ptr<Address> getToAddress() const;
	const char *getToHeader(const std::string &name) const;
	CallSession::State getTransferState() const;
	std::shared_ptr<Call> getTransferTarget() const;
	std::shared_ptr<CallStats> getVideoStats() const;
	bool isInConference() const;
	std::string getConferenceId() const;
	void setConferenceId(const std::string &conferenceId);
	bool mediaInProgress() const;
	void checkAuthenticationTokenSelected(const std::string &selectedValue);
	void setAuthenticationTokenVerified(bool value);
	void setMicrophoneVolumeGain(float value);
	void setNativeVideoWindowId(void *id);
	void setNextVideoFrameDecodedCallback(LinphoneCallCbFunc cb, void *user_data);
	void requestNotifyNextVideoFrameDecoded();
	void setParams(const MediaSessionParams *msp);
	void setSpeakerVolumeGain(float value);
	MediaSessionParams *createCallParams();

	// -----------------------------------------------------------------------------

	void setInputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice);
	bool setInputAudioDevicePrivate(const std::shared_ptr<AudioDevice> &audioDevice);
	void setOutputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice);
	bool setOutputAudioDevicePrivate(const std::shared_ptr<AudioDevice> &audioDevice);
	std::shared_ptr<AudioDevice> getInputAudioDevice() const;
	std::shared_ptr<AudioDevice> getOutputAudioDevice() const;

	// -----------------------------------------------------------------------------
	void createPlayer();
	void initiateIncoming();
	bool initiateOutgoing(const std::string &subject = "", const std::shared_ptr<const Content> content = nullptr);
	void iterate(time_t currentRealTime, bool oneSecondElapsed);
	void notifyRinging();
	void startIncomingNotification();
	void startPushIncomingNotification();
	void startBasicIncomingNotification();
	void pauseForTransfer();
	int startInvite(const std::shared_ptr<Address> &destination,
	                const std::string subject = std::string(),
	                const std::shared_ptr<const Content> content = nullptr);
	void executeTransfer();
	void scheduleTransfer();
	std::shared_ptr<Call> startReferredCall(const MediaSessionParams *params);

	// -----------------------------------------------------------------------------
	std::shared_ptr<CallSession> getActiveSession() const;
	std::shared_ptr<AbstractChatRoom> getChatRoom();
	const std::shared_ptr<Account> &getDestAccount() const;
	IceSession *getIceSession() const;
	unsigned int getAudioStartCount() const;
	unsigned int getAudioStopCount() const;
	unsigned int getVideoStartCount() const;
	unsigned int getTextStartCount() const;
	// don't make new code relying on this method.
	MediaStream *getMediaStream(LinphoneStreamType type) const;
	int getMediaStreamIndex(LinphoneStreamType type) const;
	size_t getMediaStreamsNb(LinphoneStreamType type) const;
	SalCallOp *getOp() const;
	bool getSpeakerMuted() const;
	void setSpeakerMuted(bool muted);
	bool getMicrophoneMuted() const;
	void setMicrophoneMuted(bool muted);

	// -----------------------------------------------------------------------------

	void terminateBecauseOfLostMedia();

	// -----------------------------------------------------------------------------

	void setVideoSource(std::shared_ptr<const VideoSourceDescriptor> descriptor);
	std::shared_ptr<const VideoSourceDescriptor> getVideoSource() const;

	// -----------------------------------------------------------------------------

	void enableBaudotDetection(bool enabled);
	void setBaudotMode(LinphoneBaudotMode mode);
	void setBaudotSendingStandard(LinphoneBaudotStandard standard);
	void setBaudotPauseTimeout(uint8_t seconds);

	// -----------------------------------------------------------------------------
	/* CallSessionListener */
	void onAckBeingSent(const std::shared_ptr<CallSession> &session, LinphoneHeaders *headers) override;
	void onAckReceived(const std::shared_ptr<CallSession> &session, LinphoneHeaders *headers) override;
	void onBackgroundTaskToBeStarted(const std::shared_ptr<CallSession> &session) override;
	void onBackgroundTaskToBeStopped(const std::shared_ptr<CallSession> &session) override;
	void onCallSessionAccepting(const std::shared_ptr<CallSession> &session) override;
	void onCallSessionEarlyFailed(const std::shared_ptr<CallSession> &session, LinphoneErrorInfo *ei) override;
	void onCallSessionSetReleased(const std::shared_ptr<CallSession> &session) override;
	void onCallSessionSetTerminated(const std::shared_ptr<CallSession> &session) override;
	void onCallSessionStateChanged(const std::shared_ptr<CallSession> &session,
	                               CallSession::State state,
	                               const std::string &message) override;
	void onCallSessionTransferStateChanged(const std::shared_ptr<CallSession> &session,
	                                       CallSession::State state) override;
	void onCheckForAcceptation(const std::shared_ptr<CallSession> &session) override;
	void onDtmfReceived(const std::shared_ptr<CallSession> &session, char dtmf) override;
	void onIncomingCallSessionNotified(const std::shared_ptr<CallSession> &session) override;
	void onIncomingCallSessionStarted(const std::shared_ptr<CallSession> &session) override;
	void onIncomingCallSessionTimeoutCheck(const std::shared_ptr<CallSession> &session,
	                                       int elapsed,
	                                       bool oneSecondElapsed) override;
	void onPushCallSessionTimeoutCheck(const std::shared_ptr<CallSession> &session, int elapsed) override;
	void onInfoReceived(const std::shared_ptr<CallSession> &session, const LinphoneInfoMessage *im) override;
	void onLossOfMediaDetected(const std::shared_ptr<CallSession> &session) override;
	void onSecurityLevelDowngraded(const std::shared_ptr<CallSession> &session) override;
	void onEncryptionChanged(const std::shared_ptr<CallSession> &session,
	                         bool activated,
	                         const std::string &authToken) override;
	void onAuthenticationTokenVerified(const std::shared_ptr<CallSession> &session, bool verified) override;
	void onSendMasterKeyChanged(const std::shared_ptr<CallSession> &session, const std::string &masterKey) override;
	void onReceiveMasterKeyChanged(const std::shared_ptr<CallSession> &session, const std::string &masterKey) override;
	void onGoClearAckSent() override;
	void onCallSessionStateChangedForReporting(const std::shared_ptr<CallSession> &session) override;
	void onRtcpUpdateForReporting(const std::shared_ptr<CallSession> &session, SalStreamType type) override;
	void onStatsUpdated(const std::shared_ptr<CallSession> &session, const std::shared_ptr<CallStats> &stats) override;
	void onUpdateMediaInfoForReporting(const std::shared_ptr<CallSession> &session, int statsType) override;
	void onResetCurrentSession(const std::shared_ptr<CallSession> &session) override;
	void onSetCurrentSession(const std::shared_ptr<CallSession> &session) override;
	void onFirstVideoFrameDecoded(const std::shared_ptr<CallSession> &session) override;
	void onResetFirstVideoFrameDecoded(const std::shared_ptr<CallSession> &session) override;
	void onCameraNotWorking(const std::shared_ptr<CallSession> &session, const char *camera_name) override;
	void onVideoDisplayErrorOccurred(const std::shared_ptr<CallSession> &session, int error_code) override;
	bool areSoundResourcesAvailable(const std::shared_ptr<CallSession> &session) override;
	bool isPlayingRingbackTone(const std::shared_ptr<CallSession> &session) override;
	void onRealTimeTextCharacterReceived(const std::shared_ptr<CallSession> &session,
	                                     RealtimeTextReceivedCharacter *character) override;
#ifdef HAVE_BAUDOT
	void onBaudotCharacterReceived(const std::shared_ptr<CallSession> &session, char receivedCharacter) override;
	void onBaudotDetected(const std::shared_ptr<CallSession> &session, MSBaudotStandard standard) override;
#endif /* HAVE_BAUDOT */
	void onTmmbrReceived(const std::shared_ptr<CallSession> &session, int streamIndex, int tmmbr) override;
	void onSnapshotTaken(const std::shared_ptr<CallSession> &session, const char *file_path) override;
	void onStartRingtone(const std::shared_ptr<CallSession> &session) override;
	void onRemoteRecording(const std::shared_ptr<CallSession> &session, bool recording) override;

	void confirmGoClear() const override;

	void onAlertNotified(std::shared_ptr<Alert> &alert) override;
	std::unique_ptr<LogContextualizer> getLogContextualizer() override;

	std::shared_ptr<Conference> getConference() const;
	void setConference(std::shared_ptr<Conference> ref);
	void reenterLocalConference(const std::shared_ptr<CallSession> &session);
	MSAudioEndpoint *getEndpoint() const;
	void setEndpoint(MSAudioEndpoint *endpoint);
	bool canSoundResourcesBeFreed() const;
	const std::list<LinphoneMediaEncryption> getSupportedEncryptions() const;
	const LinphoneStreamInternalStats *getStreamInternalStats(LinphoneStreamType type) const;
	/**
	 * set the EKT to all audio and video streams active in the call
	 *
	 * @param[in] ekt_params	All data needed to set the EKT
	 */
	void setEkt(const MSEKTParametersSet *ekt_params) const;

	std::shared_ptr<Event> createNotify(const std::string &eventName);

private:
	std::shared_ptr<Participant> mParticipant;
	mutable std::shared_ptr<Player> mPlayer = nullptr;
	mutable std::shared_ptr<Address> mDiversionAddress;
	CallCallbackObj mNextVideoFrameDecoded;
	mutable std::shared_ptr<AbstractChatRoom> mChatRoom = nullptr;
	bool mPlayingRingbackTone = false;

	BackgroundTask mBgTask;
	std::weak_ptr<Conference> mConfRef;
	MSAudioEndpoint *mEndpoint = nullptr;

	mutable std::string mLocalAuthToken;
	mutable std::string mRemoteAuthToken;

	void cleanupSessionAndUnrefCObjectCall();

	void updateRecordState(SalMediaRecord state);
	void createClientConference(const std::shared_ptr<CallSession> &session);
	void tryToAddToConference(std::shared_ptr<Conference> &conference, const std::shared_ptr<CallSession> &session);
	void configureSoundCardsFromCore(const MediaSessionParams *msp);

	void forgeHalfAuthenticationToken(bool localHalfAuthToken) const;
};

inline std::ostream &operator<<(std::ostream &str, const Call &call) {
	const auto &localAddress = call.getLocalAddress();
	auto localAddressStr = (localAddress ? localAddress->toString() : std::string("sip:"));
	const auto &remoteAddress = call.getRemoteAddress();
	auto remoteAddressStr = (remoteAddress ? remoteAddress->toString() : std::string("sip:"));
	str << "Call [" << &call << "] (local=" << localAddressStr << " remote=" << remoteAddressStr << ")";
	return str;
}

class CallLogContextualizer : public CoreLogContextualizer {
public:
	CallLogContextualizer(const Call &call) : CoreLogContextualizer(call) {
		pushTag(call);
	}
	CallLogContextualizer(const LinphoneCall *call) : CoreLogContextualizer(*Call::toCpp(call)) {
		if (call) pushTag(*Call::toCpp(call));
	}
	virtual ~CallLogContextualizer();

private:
	void pushTag(const Call &call);
	bool mPushed = false;
	static constexpr char sTagIdentifier[] = "2.call.linphone";
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CALL_CALL_H_
