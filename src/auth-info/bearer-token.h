/*
 * Copyright (c) 2024-2024 Belledonne Communications SARL.
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
#ifndef BEARER_TOKEN_H
#define BEARER_TOKEN_H

#include "belle-sip/bearer-token.h"
#include "c-wrapper/c-wrapper.h"
#include "linphone/api/c-types.h"

LINPHONE_BEGIN_NAMESPACE

class BearerToken : public bellesip::HybridObject<LinphoneBearerToken, BearerToken> {
public:
	// Instanciate from LinphoneConfig value (token SP expire-time)
	static BearerToken *createFromConfig(const std::string &configString);
	BearerToken(const std::string &token, time_t expirationTime);
	BearerToken(const BearerToken &other) = default;
	const std::string &getTargetHostname() const;
	const std::string &getToken() const;
	time_t getExpirationTime() const;
	std::string toConfigString() const;
	std::shared_ptr<bellesip::BearerToken> getImpl() const {
		return mToken;
	}
	bool isExpired() const;

private:
	std::shared_ptr<bellesip::BearerToken> mToken;
};

LINPHONE_END_NAMESPACE

#endif
