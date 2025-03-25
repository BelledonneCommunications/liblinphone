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

#ifndef _L_PAYLOAD_TYPE_HANDLER_H_
#define _L_PAYLOAD_TYPE_HANDLER_H_

#include <list>

#include "linphone/utils/general.h"

#include "c-wrapper/internal/c-sal.h"
#include "core/core-accessor.h"
#include "core/core.h"

#define PAYLOAD_TYPE_ENABLED PAYLOAD_TYPE_USER_FLAG_0
#define PAYLOAD_TYPE_BITRATE_OVERRIDE PAYLOAD_TYPE_USER_FLAG_3
#define PAYLOAD_TYPE_FROZEN_NUMBER PAYLOAD_TYPE_USER_FLAG_4
#define PAYLOAD_TYPE_PRIORITY_BONUS PAYLOAD_TYPE_USER_FLAG_5 // Used for codec sorting and offer-answer.

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

struct VbrCodecBitrate {
	int maxAvailableBitrate;
	int minClockRate;
	int recommendedBitrate;
};

class Core;

class PayloadTypeHandler : public CoreAccessor {
public:
	explicit PayloadTypeHandler(const std::shared_ptr<Core> &core) : CoreAccessor(core) {
	}

	std::list<OrtpPayloadType *> makeCodecsList(SalStreamType type,
	                                            int bandwidthLimit,
	                                            int maxCodecs,
	                                            const std::list<OrtpPayloadType *> &previousList,
	                                            bool bundle_enabled);

	static bool bandwidthIsGreater(int bandwidth1, int bandwidth2);
	static int getAudioPayloadTypeBandwidth(const OrtpPayloadType *pt, int maxBandwidth);
	static int getVideoPayloadTypeBandwidth(const OrtpPayloadType *pt, int maxBandwidth);
	static double getAudioPayloadTypeBandwidthFromCodecBitrate(const OrtpPayloadType *pt);
	static int getMaxCodecSampleRate(const std::list<OrtpPayloadType *> &codecs);
	static int getMinBandwidth(int downBandwidth, int upBandwidth);
	static int getRemainingBandwidthForVideo(int total, int audio);
	static bool
	isPayloadTypeNumberAvailable(const std::list<OrtpPayloadType *> &codecs, int number, const OrtpPayloadType *ignore);
	static void clearPayloadList(std::list<OrtpPayloadType *> &payloads);

private:
	static int findPayloadTypeNumber(const std::list<OrtpPayloadType *> &assigned, const OrtpPayloadType *pt);
	static bool hasTelephoneEventPayloadType(const std::list<OrtpPayloadType *> &tev, int rate);
	static bool isPayloadTypeUsableForBandwidth(const OrtpPayloadType *pt, int bandwidthLimit);
	static int lookupTypicalVbrBitrate(int maxBandwidth, int clockRate);

	void assignPayloadTypeNumbers(const std::list<OrtpPayloadType *> &codecs,
	                              const std::list<OrtpPayloadType *> &previousList);
	std::list<OrtpPayloadType *> createSpecialPayloadTypes(const std::list<OrtpPayloadType *> &codecs);
	std::list<OrtpPayloadType *> createTelephoneEventPayloadTypes(const std::list<OrtpPayloadType *> &codecs);
	OrtpPayloadType *createFecPayloadType();
	bool isPayloadTypeUsable(const OrtpPayloadType *pt);

	static const int udpHeaderSize;
	static const int rtpHeaderSize;
	static const int ipv4HeaderSize;
	static const VbrCodecBitrate defaultVbrCodecBitrates[];
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_PAYLOAD_TYPE_HANDLER_H_
