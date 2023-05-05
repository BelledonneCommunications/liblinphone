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

#ifndef _L_EKT_INFO_H_
#define _L_EKT_INFO_H_

#include <map>
#include <vector>

#include <belle-sip/object++.hh>

#include "address/address.h"
#include "dictionary/dictionary.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC EktInfo : public bellesip::HybridObject<LinphoneEktInfo, EktInfo> {
public:
	std::shared_ptr<Address> getFrom() const;
	void setFrom(const Address &from);

	uint16_t getSSpi() const;
	void setSSpi(uint16_t sSpi);

	const std::vector<uint8_t> &getCSpi() const;
	void setCSpi(const std::vector<uint8_t> &cSpi);

	std::shared_ptr<Dictionary> getCiphers() const;
	void setCiphers(const std::shared_ptr<Dictionary> &ciphers);
	void addCipher(const std::string &to, const std::vector<uint8_t> &cipher);

private:
	std::shared_ptr<Dictionary> mCiphers;
	std::shared_ptr<Address> mFrom;
	std::vector<uint8_t> mCSpi = {};
	uint16_t mSSpi;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_EKT_INFO_H_