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

#include "linphone/api/c-bearer-token.h"
#include "auth-info/bearer-token.h"

using namespace ::LinphonePrivate;

const char *linphone_bearer_token_get_token(const LinphoneBearerToken *obj) {
	return L_STRING_TO_C(BearerToken::toCpp(obj)->getToken());
}

time_t linphone_bearer_token_get_expiration_time(const LinphoneBearerToken *obj) {
	return BearerToken::toCpp(obj)->getExpirationTime();
}

LinphoneBearerToken *linphone_bearer_token_ref(LinphoneBearerToken *obj) {
	BearerToken::toCpp(obj)->ref();
	return obj;
}

void linphone_bearer_token_unref(LinphoneBearerToken *obj) {
	BearerToken::toCpp(obj)->unref();
}
