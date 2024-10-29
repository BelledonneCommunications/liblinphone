/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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

#ifndef _L_PAYLOAD_TYPE_H_
#define _L_PAYLOAD_TYPE_H_

#include "belle-sip/object++.hh"
#include "core/core-accessor.h"
#include "linphone/api/c-types.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

class LINPHONE_PUBLIC PayloadType : public bellesip::HybridObject<LinphonePayloadType, PayloadType>,
                                    public CoreAccessor {
public:
	struct Deleter {
		void operator()(::PayloadType *p) {
			payload_type_destroy(p);
		}
	};

	PayloadType(const PayloadType &payloadType, std::shared_ptr<Core> core);
	PayloadType(std::shared_ptr<Core> core, OrtpPayloadType *ortpPt);

	virtual ~PayloadType();

	PayloadType *clone() const override;

	int enable(bool enabled);
	void setNormalBitrate(int bitrate);
	void setNumber(int number);
	void setRecvFmtp(const std::string &recvFmtp);
	void setSendFmtp(const std::string &sendFmtp);

	int getType() const;
	const std::string &getDescription() const;
	const std::string &getEncoderDescription() const;
	int getNormalBitrate() const;
	std::string getMimeType() const;
	int getChannels() const;
	int getNumber() const;
	std::string getRecvFmtp() const;
	std::string getSendFmtp() const;
	int getClockRate() const;
	OrtpPayloadType *getOrtpPt() const;

	const char *getMimeTypeCstr() const;
	const char *getRecvFmtpCstr() const;
	const char *getSendFmtpCstr() const;

	bool isEnabled() const;
	bool isVbr() const;
	bool isUsable() const;
	void setPriorityBonus(bool value);
	bool weakEquals(const PayloadType &other) const;

private:
	OrtpPayloadType *mPt;
	bool mOwnOrtpPayloadType = false;

	mutable std::string mDescription;
	mutable std::string mEncoderDescription;
};

LINPHONE_END_NAMESPACE

#endif /* _L_PAYLOAD_TYPE_H_ */
