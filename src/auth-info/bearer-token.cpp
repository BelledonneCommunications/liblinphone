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

#include "bearer-token.h"

LINPHONE_BEGIN_NAMESPACE

BearerToken *BearerToken::createFromConfig(const std::string &configString) {
	std::istringstream istr(configString);
	std::string token;
	long timevalue;
	istr >> token;
	istr >> timevalue;
	return new BearerToken(token, (time_t)timevalue);
}

BearerToken::BearerToken(const std::string &token, time_t expirationTime) {
	auto bt = belle_sip_bearer_token_new(token.c_str(), expirationTime, nullptr);
	mToken = bellesip::toCpp<bellesip::BearerToken>(bt)->toSharedPtr();
}

const std::string &BearerToken::getTargetHostname() const {
	return mToken->getTargetHostname();
}

const std::string &BearerToken::getToken() const {
	return mToken->getToken();
}

time_t BearerToken::getExpirationTime() const {
	return mToken->getExpirationTime();
}

std::string BearerToken::toConfigString() const {
	return getToken() + " " + std::to_string((long)getExpirationTime());
}

bool BearerToken::isExpired() const {
	time_t expirationTime = getExpirationTime();
	if (expirationTime == 0) {
		lWarning() << "No expiration time set for bearer token. Will assume it is not expired.";
		return false;
	}
	return time(nullptr) >= getExpirationTime();
}

LINPHONE_END_NAMESPACE
