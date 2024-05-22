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

#ifndef _L_FRIEND_PHONE_NUMBER_H_
#define _L_FRIEND_PHONE_NUMBER_H_

#ifdef VCARD_ENABLED
#include <belcard/belcard_communication.hpp>
#endif
#include "c-wrapper/c-wrapper.h"
#include "linphone/api/c-types.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class FriendPhoneNumber : public bellesip::HybridObject<LinphoneFriendPhoneNumber, FriendPhoneNumber> {
public:
	FriendPhoneNumber(const std::string &phoneNumber, const std::string label);
	FriendPhoneNumber(const std::string &phoneNumber);
#ifdef VCARD_ENABLED
	FriendPhoneNumber(const std::shared_ptr<belcard::BelCardPhoneNumber> &belcardPhoneNumber);
#endif
	FriendPhoneNumber(const FriendPhoneNumber &other);
	~FriendPhoneNumber();

	FriendPhoneNumber *clone() const override;
#ifdef VCARD_ENABLED
	std::shared_ptr<belcard::BelCardPhoneNumber> toBelcardPhoneNumber(bool useV3Grammar) const;
#endif

	void setPhoneNumber(const std::string &phoneNumber);
	const std::string &getPhoneNumber() const;

	void setLabel(const std::string &label);
	const std::string &getLabel() const;

private:
	std::string mPhoneNumber;
	std::string mLabel;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_FRIEND_PHONE_NUMBER_H_
