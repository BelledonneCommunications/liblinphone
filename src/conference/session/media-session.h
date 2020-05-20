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

#ifndef _L_MEDIA_SESSION_H_
#define _L_MEDIA_SESSION_H_

#include "call-session.h"
#include "conference/params/media-session-params.h"
#include "call/audio-device/audio-device.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class CallPrivate;
class Core;
class IceAgent;
class MediaSessionPrivate;
class Participant;
class StreamsGroup;

class LINPHONE_PUBLIC MediaSession : public CallSession {
	friend class Call;
	friend class CallPrivate;
	friend class IceAgent;
	friend class ToneManager;
	friend class Stream;
	friend class StreamsGroup;

public:
	MediaSession (const std::shared_ptr<Core> &core, Participant* me, const CallSessionParams *params, CallSessionListener *listener);
	~MediaSession ();

	virtual void acceptDefault() override;
	LinphoneStatus accept (const MediaSessionParams *msp = nullptr);
	LinphoneStatus acceptEarlyMedia (const MediaSessionParams *msp = nullptr);
	LinphoneStatus acceptUpdate (const MediaSessionParams *msp);
	void cancelDtmfs ();
	void configure (LinphoneCallDir direction, LinphoneProxyConfig *cfg, SalCallOp *op, const Address &from, const Address &to) override;
	LinphoneStatus deferUpdate () override;
	void initiateIncoming () override;
	bool initiateOutgoing () override;
	void iterate (time_t currentRealTime, bool oneSecondElapsed) override;
	LinphoneStatus pause ();
	LinphoneStatus resume ();
	LinphoneStatus sendDtmf (char dtmf);
	LinphoneStatus sendDtmfs (const std::string &dtmfs);
	void sendVfuRequest ();
	void startIncomingNotification (bool notifyRinging = true) override;
	int startInvite (const Address *destination, const std::string &subject = "", const Content *content = nullptr) override;
	void startRecording ();
	void stopRecording ();
	bool isRecording ();
	void terminateBecauseOfLostMedia ();
	LinphoneStatus update (const MediaSessionParams *msp, const std::string &subject = "");

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
	std::string getAuthenticationToken () const;
	bool getAuthenticationTokenVerified () const;
	float getAverageQuality () const;
	MediaSessionParams *getCurrentParams () const;
	float getCurrentQuality () const;
	const MediaSessionParams *getMediaParams () const;
	RtpTransport * getMetaRtcpTransport (int streamIndex) const;
	RtpTransport * getMetaRtpTransport (int streamIndex) const;
	float getMicrophoneVolumeGain () const;
	void * getNativeVideoWindowId () const;
	void * getNativePreviewVideoWindowId () const;
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
	void setAudioRoute (LinphoneAudioRoute route);
	void setAuthenticationTokenVerified (bool value);
	void setMicrophoneVolumeGain (float value);
	void setNativeVideoWindowId (void *id);
	void setNativePreviewWindowId (void *id);
	void setParams (const MediaSessionParams *msp);
	void setSpeakerVolumeGain (float value);

	void setInputAudioDevice(AudioDevice *audioDevice);
	void setOutputAudioDevice(AudioDevice *audioDevice);
	AudioDevice* getInputAudioDevice() const;
	AudioDevice* getOutputAudioDevice() const;

	StreamsGroup & getStreamsGroup()const;
private:
	L_DECLARE_PRIVATE(MediaSession);
	L_DISABLE_COPY(MediaSession);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_MEDIA_SESSION_H_
