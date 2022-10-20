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

#ifndef LINPHONE_FRIEND_PHONE_NUMBER_H
#define LINPHONE_FRIEND_PHONE_NUMBER_H

#include "linphone/api/c-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup buddy_list
 * @{
 */

/**
 * Creates a new #LinphoneFriendPhoneNumber.
 * @param phone_number The phone number. @notnil
 * @param label the type of phone number, for example "home", "cell", etc. Use NULL or empty for no label. @maybenil
 * @return The newly created #LinphoneFriendPhoneNumber object. @notnil
 */
LINPHONE_PUBLIC LinphoneFriendPhoneNumber* linphone_friend_phone_number_new(const char *phone_number, const char *label);

/**
 * Clones a phone number.
 * @param phone_number The #LinphoneFriendPhoneNumber object to be cloned. @notnil
 * @return The newly created #LinphoneFriendPhoneNumber object. @notnil
 */
LINPHONE_PUBLIC LinphoneFriendPhoneNumber* linphone_friend_phone_number_clone(const LinphoneFriendPhoneNumber *phone_number);

/**
 * Takes a reference on a #LinphoneFriendPhoneNumber.
 * @param phone_number The #LinphoneFriendPhoneNumber object. @notnil
 * @return the same #LinphoneFriendPhoneNumber object. @notnil
 */
LINPHONE_PUBLIC LinphoneFriendPhoneNumber* linphone_friend_phone_number_ref(LinphoneFriendPhoneNumber *phone_number);

/**
 * Releases a #LinphoneFriendPhoneNumber.
 * @param phone_number The #LinphoneFriendPhoneNumber object. @notnil
 */
LINPHONE_PUBLIC void linphone_friend_phone_number_unref(LinphoneFriendPhoneNumber *phone_number);

// =============================================================================

/**
 * Sets the phone number.
 * @param phone_number The #LinphoneFriendPhoneNumber object. @notnil
 * @param number the phone number to set. @notnil
 */
LINPHONE_PUBLIC void linphone_friend_phone_number_set_phone_number(LinphoneFriendPhoneNumber *phone_number, const char *number);

/**
 * Gets the phone number.
 * @param phone_number The #LinphoneFriendPhoneNumber object. @notnil
 * @return the phone number stored. @notnil
 */
LINPHONE_PUBLIC const char *linphone_friend_phone_number_get_phone_number(const LinphoneFriendPhoneNumber *phone_number);

/**
 * Sets the label for this phone number.
 * @param phone_number The #LinphoneFriendPhoneNumber object. @notnil
 * @param label the label to set. @maybenil
 */
LINPHONE_PUBLIC void linphone_friend_phone_number_set_label(LinphoneFriendPhoneNumber *phone_number, const char *label);

/**
 * Gets the label associated to this phone number.
 * @param phone_number The #LinphoneFriendPhoneNumber object. @notnil
 * @return the label set if any, NULL otherwise. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_friend_phone_number_get_label(const LinphoneFriendPhoneNumber *phone_number);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_FRIEND_PHONE_NUMBER_H */
