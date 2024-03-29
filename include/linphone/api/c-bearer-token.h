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

#ifndef LINPHONE_BEARER_TOKEN_H
#define LINPHONE_BEARER_TOKEN_H

#include "linphone/api/c-types.h"

/**
 * @addtogroup authentication
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get the token as a string.
 * @param obj the #LinphoneBearerToken @notnil
 * @return the token. @notnil
 */
LINPHONE_PUBLIC const char *linphone_bearer_token_get_token(const LinphoneBearerToken *obj);

/**
 * Get the token exiration time, as a number of seconds since EPOCH.
 * @param obj the #LinphoneBearerToken @notnil
 * @return the expiration time
 */
LINPHONE_PUBLIC time_t linphone_bearer_token_get_expiration_time(const LinphoneBearerToken *obj);

/**
 * Increment the reference count of the token object.
 * @param obj the #LinphoneBearerToken @notnil
 * @return the same #LinphoneBearerToken @notnil
 */
LINPHONE_PUBLIC LinphoneBearerToken *linphone_bearer_token_ref(LinphoneBearerToken *obj);

/**
 * Decrement the reference count of the token object, which may free it.
 * @param obj the #LinphoneBearerToken @notnil
 */
LINPHONE_PUBLIC void linphone_bearer_token_unref(LinphoneBearerToken *obj);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif
