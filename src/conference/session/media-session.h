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
#include "conference/params/media-session-params.h"
#include "call/audio-device/audio-device.h"
#include "call/video-source/video-source-descriptor.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Core;
class IceAgent;
class MediaSessionPrivate;
class Participant;
class StreamsGroup;

namespace MediaConference {
	class Conference;
	class RemoteConference;
}

class LINPHONE_PUBLIC MediaSession : public CallSession {
	friend class Call;
	friend class Core;
	friend class IceAgent;
	friend class ToneManager;
	friend class Stream;
	friend class StreamsGroup;

	friend class MediaConference::LocalConference;
	friend class MediaConference::RemoteConference;
public:
	static ConferenceLayout computeConferenceLayout(const std::shared_ptr<SalMediaDescription> & md);

	MediaSession (const std::shared_ptr<Core> &core, std::shared_ptr<Participant> me, const CallSessionParams *params, CallSessionListener *listener);
	~MediaSession ();

	virtual void acceptDefault() override;
	LinphoneStatus accept (const MediaSessionParams *msp = nullptr);
	LinphoneStatus acceptEarlyMedia (const MediaSessionParams *msp = nullptr);
	LinphoneStatus acceptUpdate (const MediaSessionParams *msp);
	void cancelDtmfs ();
	void setNatPolicy(LinphoneNatPolicy *pol);
	void setSubject(const std::string & subject);
	void enableToneIndications(bool enabled);
	bool toneIndicationsEnabled()const;
	void configure (LinphoneCallDir direction, LinphoneProxyConfig *cfg, SalCallOp *op, const Address &from, const Address &to) override;
	LinphoneStatus deferUpdate () override;
	void initiateIncoming () override;
	bool initiateOutgoing (const std::string &subject = "", const Content *content = nullptr) override;
	void iterate (time_t currentRealTime, bool oneSecondElapsed) override;
	LinphoneStatus pauseFromConference ();
	LinphoneStatus pause ();
	LinphoneStatus resume ();
	LinphoneStatus delayResume();
	LinphoneStatus sendDtmf (char dtmf);
	LinphoneStatus sendDtmfs (const std::string &dtmfs);
	void sendVfuRequest ();
	void startIncomingNotification (bool notifyRinging = true) override;
	int startInvite (const Address *destination, const std::string &subject = "", const Content *content = nullptr) override;
	bool startRecording ();
	void stopRecording ();
	bool isRecording ();
	void setRecordPath(const std::string &path);
	void terminateBecauseOfLostMedia ();
	LinphoneStatus updateFromConference (const MediaSessionParams *msp, const std::string &subject = "");
	LinphoneStatus update (const MediaSessionParams *msp, const UpdateMethod method = UpdateMethod::Default, const bool isCapabilityNegotiationUpdate = false, const std::string &subject = "");

	void requestNotifyNextVideoFrameDecoded ();
	LinphoneStatus takePreviewSnapshot (const std::string& file);
	LinphoneStatus takeVideoSnapshot (const std::string& file);
	void zoomVideo (float zoomFactor, float *cx, float *cy);
	void zoomVideo (float zoomFactor, float cx, float cy);

	bool cameraEnabled () const;
	bool echoCancellationEnabled () const;
	bool echoLimiterEnabled () const;
	void enableCamera (bool value);
	void enableEchoCancellation (bool value);
	void enableEchoLimiter (bool value);
	bool getAllMuted () const;
	LinphoneCallStats * getAudioStats () const;
	const std::string &getAuthenticationToken () const;
	bool getAuthenticationTokenVerified () const;
	float getAverageQuality () const;
	MediaSessionParams *getCurrentParams () const;
	float getCurrentQuality () const;
	const MediaSessionParams *getMediaParams () const;
	RtpTransport * getMetaRtcpTransport (int streamIndex) const;
	RtpTransport * getMetaRtpTransport (int streamIndex) const;
	float getMicrophoneVolumeGain () const;
	void * getNativeVideoWindowId (const std::string label = "") const;
	void * getNativePreviewVideoWindowId () const;
	void * createNativePreviewVideoWindowId () const;
	void * createNativeVideoWindowId (const std::string label = "") const;
	const CallSessionParams *getParams () const override;
	float getPlayVolume () const;
	float getRecordVolume () const;
	const MediaSessionParams *getRemoteParams ();
	float getSpeakerVolumeGain () const;
	LinphoneCallStats * getStats (LinphoneStreamType type) const;
	int getStreamCount () const;
	MSFormatType getStreamType (int streamIndex) const;
	LinphoneCallStats * getTextStats () const;
	LinphoneCallStats * getVideoStats () const;
	bool mediaInProgress () const;
	void setAuthenticationTokenVerified (bool value);
	void setMicrophoneVolumeGain (float value);
	void setNativeVideoWindowId (void *id, const std::string label = "");
	void setNativePreviewWindowId (void *id);
	void setParams (const MediaSessionParams *msp);
	void setSpeakerVolumeGain (float value);

	bool setInputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice);
	bool setOutputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice);
	std::shared_ptr<AudioDevice> getInputAudioDevice() const;
	std::shared_ptr<AudioDevice> getOutputAudioDevice() const;

	void * getParticipantWindowId(const std::string label);

	StreamsGroup & getStreamsGroup()const;
	bool pausedByApp()const;
	bool isTerminator()const;
	void notifySpeakingDevice(uint32_t ssrc, bool isSpeaking);
	void notifyMutedDevice(uint32_t ssrc, bool muted);
	void onGoClearAckSent();

	void queueIceCompletionTask(const std::function<LinphoneStatus()> &lambda);

	void setVideoSource (const std::shared_ptr<const VideoSourceDescriptor> &descriptor);
	std::shared_ptr<const VideoSourceDescriptor> getVideoSource () const;

	void confirmGoClear();

	uint32_t getSsrc(LinphoneStreamType type) const;

private:
	L_DECLARE_PRIVATE(MediaSession);
	L_DISABLE_COPY(MediaSession);

	int getRandomRtpPort (const SalStreamDescription & stream) const;
};

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
