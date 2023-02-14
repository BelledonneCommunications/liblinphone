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

#ifndef _L_MEDIA_SESSION_PARAMS_H_
#define _L_MEDIA_SESSION_PARAMS_H_

#include <ortp/payloadtype.h>

#include "call/audio-device/audio-device.h"
#include "call-session-params.h"
#include "utils/general-internal.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class MediaSession;
class MediaSessionPrivate;
class MediaSessionParamsPrivate;

namespace MediaConference {
	class LocalConference;
}

class LINPHONE_INTERNAL_PUBLIC MediaSessionParams : public CallSessionParams {
	friend class Call;
	friend class MediaSession;
	friend class MediaSessionPrivate;
	friend class MS2Stream;
	friend class MS2AudioStream;
	friend class MS2VideoStream;
	friend class MS2RTTStream;
	friend class ParticipantDevice;

	friend class Conference;
	friend class MediaConference::LocalConference;
public:
	MediaSessionParams ();
	MediaSessionParams (const MediaSessionParams &other);
	virtual ~MediaSessionParams ();

	MediaSessionParams* clone () const override {
		return new MediaSessionParams(*this);
	}

	MediaSessionParams &operator= (const MediaSessionParams &other);

	void initDefault (const std::shared_ptr<Core> &core, LinphoneCallDir dir) override;

	bool audioEnabled () const;
	bool audioMulticastEnabled () const;
	void enableAudio (bool value);
	void enableAudioMulticast (bool value);
	int getAudioBandwidthLimit () const;
	LinphoneMediaDirection getAudioDirection () const;
	const OrtpPayloadType * getUsedAudioCodec () const;
	LinphonePayloadType * getUsedAudioPayloadType () const;
	void setAudioBandwidthLimit (int value);
	void setAudioDirection (SalStreamDir direction);
	void setAudioDirection (LinphoneMediaDirection direction);

	void enableVideo (bool value);
	void enableVideoMulticast (bool value);
	float getReceivedFps () const;
	LinphoneVideoDefinition * getReceivedVideoDefinition () const;
	float getSentFps () const;
	LinphoneVideoDefinition * getSentVideoDefinition () const;
	const OrtpPayloadType * getUsedVideoCodec () const;
	LinphonePayloadType * getUsedVideoPayloadType () const;
	LinphoneMediaDirection getVideoDirection () const;
	void setVideoDirection (SalStreamDir direction);
	void setVideoDirection (LinphoneMediaDirection direction);
	bool videoEnabled () const;
	bool videoMulticastEnabled () const;

	void enableRealtimeText (bool value);
	void setRealtimeTextKeepaliveInterval (unsigned int interval);
	const OrtpPayloadType * getUsedRealtimeTextCodec () const;
	LinphonePayloadType * getUsedRealtimeTextPayloadType () const;
	bool realtimeTextEnabled () const;
	unsigned int realtimeTextKeepaliveInterval () const;

	bool avpfEnabled () const;
	bool hasAvpfEnabledBeenSet () const;
	void enableAvpf (bool value);
	uint16_t getAvpfRrInterval () const;
	void setAvpfRrInterval (uint16_t value);

	bool lowBandwidthEnabled () const;
	void enableLowBandwidth (bool value);

	const std::string& getRecordFilePath () const;
	void setRecordFilePath (const std::string &path);

	bool earlyMediaSendingEnabled () const;
	void enableEarlyMediaSending (bool value);

	void enableMandatoryMediaEncryption (bool value);
	LinphoneMediaEncryption getMediaEncryption () const;
	bool mandatoryMediaEncryptionEnabled () const;
	void setMediaEncryption (LinphoneMediaEncryption encryption);

	SalMediaProto getMediaProto (const LinphoneMediaEncryption mediaEnc, const bool avpf) const;
	SalMediaProto getMediaProto () const;
	const char * getRtpProfile () const;

	void addCustomSdpAttribute (const std::string &attributeName, const std::string &attributeValue);
	void clearCustomSdpAttributes ();
	const char * getCustomSdpAttribute (const std::string &attributeName) const;

	void addCustomSdpMediaAttribute (LinphoneStreamType lst, const std::string &attributeName, const std::string &attributeValue);
	void clearCustomSdpMediaAttributes (LinphoneStreamType lst);
	const char * getCustomSdpMediaAttribute (LinphoneStreamType lst, const std::string &attributeName) const;
	
	void enableRtpBundle (bool value);
	bool rtpBundleEnabled () const;

	bool recordAwareEnabled () const;
	void enableRecordAware (bool value);

	bool isRecording () const;
	void setRecordingState (SalMediaRecord recordState);
	SalMediaRecord getRecordingState () const;

	// The following methods are only used to set some default settings for call creation!
	void enableMic (bool value);
	bool isMicEnabled () const;
	void setInputAudioDevice(const std::shared_ptr<AudioDevice> &device);
	void setOutputAudioDevice(const std::shared_ptr<AudioDevice> &device);
	std::shared_ptr<AudioDevice> getInputAudioDevice() const;
	std::shared_ptr<AudioDevice> getOutputAudioDevice() const;

private:
	L_DECLARE_PRIVATE(MediaSessionParams);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_MEDIA_SESSION_PARAMS_H_
