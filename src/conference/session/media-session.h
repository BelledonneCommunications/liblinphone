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

#ifndef _L_MEDIA_SESSION_H_
#define _L_MEDIA_SESSION_H_

#include "call-session.h"
#include "call/audio-device/audio-device.h"
#include "call/video-source/video-source-descriptor.h"
#include "conference/params/media-session-params.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class AlertMonitor;
class Core;
class IceAgent;
class MediaSessionPrivate;
class Participant;
class ParticipantDevice;
class StreamsGroup;
class NatPolicy;
class VideoControlInterface;

class Conference;

class LINPHONE_PUBLIC MediaSession : public CallSession {
	friend class AlertMonitor;
	friend class Call;
	friend class Conference;
	friend class Core;
	friend class IceAgent;
	friend class ServerConference;
	friend class ClientConference;
	friend class Stream;
	friend class StreamsGroup;
	friend class ToneManager;

public:
	ConferenceLayout computeConferenceLayout(const std::shared_ptr<SalMediaDescription> &md) const;

	MediaSession(const std::shared_ptr<Core> &core,
	             std::shared_ptr<Participant> me,
	             const CallSessionParams *params,
	             CallSessionListener *listener);
	~MediaSession();

	virtual void acceptDefault() override;
	LinphoneStatus accept(const MediaSessionParams *msp = nullptr);
	LinphoneStatus acceptEarlyMedia(const MediaSessionParams *msp = nullptr);
	LinphoneStatus acceptUpdate(const MediaSessionParams *msp);
	void cancelDtmfs();
	void setNatPolicy(const std::shared_ptr<NatPolicy> &pol);
	void setSubject(const std::string &subject);
	bool toneIndicationsEnabled() const;
	void configure(LinphoneCallDir direction,
	               const std::shared_ptr<Account> &account,
	               SalCallOp *op,
	               const std::shared_ptr<const Address> &from,
	               const std::shared_ptr<const Address> &to) override;
	LinphoneStatus deferUpdate() override;
	void initiateIncoming() override;
	bool initiateOutgoing(const std::string &subject = "",
	                      const std::shared_ptr<const Content> content = nullptr) override;
	void iterate(time_t currentRealTime, bool oneSecondElapsed) override;
	LinphoneStatus pauseFromConference();
	LinphoneStatus pause();
	LinphoneStatus resume();
	LinphoneStatus delayResume();
	LinphoneStatus sendDtmf(char dtmf);
	LinphoneStatus sendDtmfs(const std::string &dtmfs);
	void sendVfuRequest();
	void startIncomingNotification(bool notifyRinging = true) override;
	int startInvite(const std::shared_ptr<Address> &destination,
	                const std::string &subject = "",
	                const std::shared_ptr<const Content> content = nullptr) override;
	bool startRecording();
	void stopRecording();
	bool isRecording();
	void setRecordPath(const std::string &path);
	void terminateBecauseOfLostMedia();
	LinphoneStatus updateFromConference(const MediaSessionParams *msp, const std::string &subject = "");
	LinphoneStatus update(const MediaSessionParams *msp,
	                      const UpdateMethod method = UpdateMethod::Default,
	                      const bool isCapabilityNegotiationUpdate = false,
	                      const std::string &subject = "");

	void requestNotifyNextVideoFrameDecoded();
	LinphoneStatus takePreviewSnapshot(const std::string &file);
	LinphoneStatus takeVideoSnapshot(const std::string &file);
	void zoomVideo(float zoomFactor, float *cx, float *cy);
	void zoomVideo(float zoomFactor, float cx, float cy);

	MediaSessionParams *createMediaSessionParams();
	bool cameraEnabled() const;
	bool echoCancellationEnabled() const;
	bool echoLimiterEnabled() const;
	void enableCamera(bool value);
	void enableEchoCancellation(bool value);
	void enableEchoLimiter(bool value);
	bool getAllMuted() const;
	std::shared_ptr<CallStats> getAudioStats() const;
	const std::string &getAuthenticationToken() const;
	void storeAndSortRemoteAuthToken(const std::string &remoteAuthToken) const;
	const std::list<std::string> &getRemoteAuthenticationTokens() const;
	const bctbx_list_t *getCListRemoteAuthenticationTokens() const;
	bool getAuthenticationTokenVerified() const;
	bool getZrtpCacheMismatch() const;
	float getAverageQuality() const;
	MediaSessionParams *getCurrentParams() const;
	float getCurrentQuality() const;
	const MediaSessionParams *getMediaParams() const;
	RtpTransport *getMetaRtcpTransport(int streamIndex) const;
	RtpTransport *getMetaRtpTransport(int streamIndex) const;
	float getMicrophoneVolumeGain() const;
	void *
	getNativeVideoWindowId(const std::string label = "", const bool isMe = false, const bool isThumbnail = false) const;
	void *getNativePreviewVideoWindowId() const;
	void *createNativePreviewVideoWindowId() const;
	void *createNativeVideoWindowId(const std::string label = "",
	                                const bool isMe = false,
	                                const bool isThumbnail = false) const;
	const CallSessionParams *getParams() const override;
	float getPlayVolume() const;
	float getRecordVolume() const;
	const MediaSessionParams *getRemoteParams() const;
	float getSpeakerVolumeGain() const;
	std::shared_ptr<CallStats> getStats(LinphoneStreamType type) const;
	int getStreamCount() const;
	MSFormatType getStreamType(int streamIndex) const;
	std::shared_ptr<CallStats> getTextStats() const;
	std::shared_ptr<CallStats> getVideoStats() const;
	bool mediaInProgress() const;
	void checkAuthenticationTokenSelected(const std::string &selectedValue, const std::string &halfAuthToken);
	void skipZrtpAuthentication();
	void setAuthenticationTokenVerified(bool value);
	void setAuthenticationTokenCheckDone(bool value);
	void setMicrophoneVolumeGain(float value);
	void setNativeVideoWindowId(void *id,
	                            const std::string label = "",
	                            const bool isMe = false,
	                            const bool isThumbnail = false);
	void setNativePreviewWindowId(void *id);
	void setParams(const MediaSessionParams *msp);
	void setSpeakerVolumeGain(float value);

	bool setInputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice);
	bool setOutputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice);
	std::shared_ptr<AudioDevice> getInputAudioDevice() const;
	std::shared_ptr<AudioDevice> getOutputAudioDevice() const;

	std::shared_ptr<ParticipantDevice> getParticipantDevice(const LinphoneStreamType type, const std::string &label);
	void *getParticipantWindowId(const std::string &label);

	StreamsGroup &getStreamsGroup() const;
	bool pausedByApp() const;
	bool isTerminator() const;
	void notifySpeakingDevice(uint32_t ssrc, bool isSpeaking);
	void notifyMutedDevice(uint32_t ssrc, bool muted);
	void onGoClearAckSent();

	void queueIceCompletionTask(const std::function<LinphoneStatus()> &lambda);

	bool canOfferVideoSharing(bool enableLog) const;
	void setVideoSource(const std::shared_ptr<const VideoSourceDescriptor> &descriptor);
	std::shared_ptr<const VideoSourceDescriptor> getVideoSource() const;

	void confirmGoClear();

	uint32_t getSsrc(LinphoneStreamType type) const;
	uint32_t getSsrc(std::string content) const;

	int getLocalThumbnailStreamIdx() const;
	int getThumbnailStreamIdx(const std::shared_ptr<SalMediaDescription> &md) const;
	int getMainVideoStreamIdx(const std::shared_ptr<SalMediaDescription> &md) const;

	/**
	 * set the EKT to all audio and video streams of this media session
	 *
	 * @param[in] ekt_params	All data needed to set the EKT
	 */
	void setEkt(const MSEKTParametersSet *ekt_params) const;
	bool dtmfSendingAllowed() const;

	LinphoneMediaDirection getDirectionOfStream(const std::string content) const;
	bool isScreenSharingNegotiated() const;
	const std::shared_ptr<const VideoSourceDescriptor> getVideoSourceDescriptor() const;

	bool requestThumbnail(const std::shared_ptr<ParticipantDevice> &device) const;

private:
	L_DECLARE_PRIVATE(MediaSession);
	L_DISABLE_COPY(MediaSession);

	int getRandomRtpPort(const SalStreamDescription &stream) const;
	const std::shared_ptr<Conference> getLocalConference() const;

	std::shared_ptr<const VideoSourceDescriptor> mVideoSourceDescriptor;

	VideoControlInterface *getVideoControlInterface(const std::string label = "",
	                                                const bool isMe = false,
	                                                const bool isThumbnail = false) const;
};

inline std::ostream &operator<<(std::ostream &str, const MediaSession &callSession) {
	str << "MediaSession [" << &callSession << "]";
	return str;
}

/**
 * Convert enum LinphoneSrtpSuite into enum MSCryptoSuite
 * Enums definitions are not perferctly matching
 * any input without corresponding MSCryptoSuite value gives a MS_CRYPTO_SUITE_INVALID output
 *
 * @param[in]	suite	The LinphoneSrtpSuite to be converted
 * @return	the matching MSCryptoSuite value
 **/
MSCryptoSuite LinphoneSrtpSuite2MSCryptoSuite(const LinphoneSrtpSuite suite);

/**
 * Convert a list of enum LinphoneSrtpSuite into a list enum MSCryptoSuite
 * Enums definitions are not perferctly matching
 * input giving MS_CRYPTO_SUITE_INVALID are skipped in the output list
 *
 * @param[in]	suite	The list of LinphoneSrtpSuite to be converted
 * @return	the matching MSCryptoSuite list, unconvertible input are skipped
 **/
std::list<MSCryptoSuite> LinphoneSrtpSuite2MSCryptoSuite(const std::list<LinphoneSrtpSuite> suites);

LINPHONE_END_NAMESPACE

#endif // ifndef _L_MEDIA_SESSION_H_
