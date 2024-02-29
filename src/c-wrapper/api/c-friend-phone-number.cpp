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

#include <ctype.h>

#include "c-wrapper/c-wrapper.h"
#include "friend/friend-phone-number.h"
#include "linphone/api/c-friend-phone-number.h"
#include "linphone/wrapper_utils.h"

using namespace LinphonePrivate;

// =============================================================================

LinphoneFriendPhoneNumber *linphone_friend_phone_number_new(const char *phone_number, const char *label) {
	return FriendPhoneNumber::createCObject(L_C_TO_STRING(phone_number), L_C_TO_STRING(label));
}

LinphoneFriendPhoneNumber *linphone_friend_phone_number_clone(const LinphoneFriendPhoneNumber *phone_number) {
	return FriendPhoneNumber::toCpp(phone_number)->clone()->toC();
}

LinphoneFriendPhoneNumber *linphone_friend_phone_number_ref(LinphoneFriendPhoneNumber *phone_number) {
	FriendPhoneNumber::toCpp(phone_number)->ref();
	return phone_number;
}

void linphone_friend_phone_number_unref(LinphoneFriendPhoneNumber *phone_number) {
	FriendPhoneNumber::toCpp(phone_number)->unref();
}

// =============================================================================

void linphone_friend_phone_number_set_phone_number(LinphoneFriendPhoneNumber *phone_number, const char *number) {
	FriendPhoneNumber::toCpp(phone_number)->setPhoneNumber(L_C_TO_STRING(number));
}

const char *linphone_friend_phone_number_get_phone_number(const LinphoneFriendPhoneNumber *phone_number) {
	return L_STRING_TO_C(FriendPhoneNumber::toCpp(phone_number)->getPhoneNumber());
}

void linphone_friend_phone_number_set_label(LinphoneFriendPhoneNumber *phone_number, const char *label) {
	FriendPhoneNumber::toCpp(phone_number)->setLabel(L_C_TO_STRING(label));
}

const char *linphone_friend_phone_number_get_label(const LinphoneFriendPhoneNumber *phone_number) {
	return L_STRING_TO_C(FriendPhoneNumber::toCpp(phone_number)->getLabel());
}

// =============================================================================