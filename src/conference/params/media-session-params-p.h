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

#ifndef _L_MEDIA_SESSION_PARAMS_P_H_
#define _L_MEDIA_SESSION_PARAMS_P_H_

#include "call-session-params-p.h"

#include "media-session-params.h"

// =============================================================================

extern LinphoneCallParams * linphone_call_params_new_for_wrapper(void);

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class MediaSessionParamsPrivate : public CallSessionParamsPrivate {
public:
	void clone (const MediaSessionParamsPrivate *src);
	void clean ();

	static SalStreamDir mediaDirectionToSalStreamDir (LinphoneMediaDirection direction);
	static LinphoneMediaDirection salStreamDirToMediaDirection (SalStreamDir dir);

	void adaptToNetwork (LinphoneCore *core, int pingTimeMs);

	SalStreamDir getSalAudioDirection () const;
	SalStreamDir getSalVideoDirection () const;

	void enableImplicitRtcpFb (bool value) { _implicitRtcpFbEnabled = value; }
	bool implicitRtcpFbEnabled () const { return _implicitRtcpFbEnabled; }
	int getDownBandwidth () const { return downBandwidth; }
	void setDownBandwidth (int value) { downBandwidth = value; }
	int getUpBandwidth () const { return upBandwidth; }
	void setUpBandwidth (int value) { upBandwidth = value; }
	int getDownPtime () const { return downPtime; }
	void setDownPtime (int value) { downPtime = value; }
	int getUpPtime () const { return upPtime; }
	void setUpPtime (int value) { upPtime = value; }
	bool getUpdateCallWhenIceCompleted () const;
	void setUpdateCallWhenIceCompleted(bool value){
		/* apply to both case when set explicitely */
		updateCallWhenIceCompleted = value;
		updateCallWhenIceCompletedWithDTLS = value;
	}

	void setReceivedFps (float value) { receivedFps = value; }
	void setReceivedVideoDefinition (LinphoneVideoDefinition *value);
	void setSentFps (float value) { sentFps = value; }
	void setSentVideoDefinition (LinphoneVideoDefinition *value);
	void setUsedAudioCodec (OrtpPayloadType *pt) { usedAudioCodec = pt; }
	void setUsedVideoCodec (OrtpPayloadType *pt) { usedVideoCodec = pt; }
	void setUsedRealtimeTextCodec (OrtpPayloadType *pt) { usedRealtimeTextCodec = pt; }

	SalCustomSdpAttribute * getCustomSdpAttributes () const;
	void setCustomSdpAttributes (const SalCustomSdpAttribute *csa);
	SalCustomSdpAttribute * getCustomSdpMediaAttributes (LinphoneStreamType lst) const;
	void setCustomSdpMediaAttributes (LinphoneStreamType lst, const SalCustomSdpAttribute *csa);

public:
	bool audioEnabled = true;
	int audioBandwidthLimit = 0;
	LinphoneMediaDirection audioDirection = LinphoneMediaDirectionSendRecv;
	bool audioMulticastEnabled = false;
	PayloadType *usedAudioCodec = nullptr;

	bool videoEnabled = false;
	LinphoneMediaDirection videoDirection = LinphoneMediaDirectionSendRecv;
	bool videoMulticastEnabled = false;
	PayloadType *usedVideoCodec = nullptr;
	float receivedFps = 0.f;
	LinphoneVideoDefinition *receivedVideoDefinition = nullptr;
	float sentFps = 0.f;
	LinphoneVideoDefinition *sentVideoDefinition = nullptr;

	unsigned int realtimeTextKeepaliveInterval = 25000;
	PayloadType *usedRealtimeTextCodec = nullptr;
	int videoDownloadBandwidth = 0;

	bool realtimeTextEnabled = false;
	bool avpfEnabled = false;
	bool hasAvpfEnabledBeenSet = false;
	uint16_t avpfRrInterval = 0; /* In milliseconds */

	bool lowBandwidthEnabled = false;

	std::string recordFilePath;

	bool earlyMediaSendingEnabled = false; /* Send real media even during early media (for outgoing calls) */

	LinphoneMediaEncryption encryption = LinphoneMediaEncryptionNone;
	bool mandatoryMediaEncryptionEnabled = false;

	bool rtpBundle = false;

	bool recordAware = false;
	SalMediaRecord recordState = SalMediaRecordNone;

private:
	bool _implicitRtcpFbEnabled = false;
	int downBandwidth = 0;
	int upBandwidth = 0;
	int downPtime = 0;
	int upPtime = 0;
	
	bool updateCallWhenIceCompleted = true;
	bool updateCallWhenIceCompletedWithDTLS = false;
	SalCustomSdpAttribute *customSdpAttributes = nullptr;
	SalCustomSdpAttribute *customSdpMediaAttributes[LinphoneStreamTypeUnknown];

	bool micEnabled = true;
	std::shared_ptr<AudioDevice> inputAudioDevice = nullptr;
	std::shared_ptr<AudioDevice> outputAudioDevice = nullptr;

	L_DECLARE_PUBLIC(MediaSessionParams);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_MEDIA_SESSION_PARAMS_P_H_
