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

#ifndef _L_C_DIGEST_AUTHENTICATION_POLICY_H_
#define _L_C_DIGEST_AUTHENTICATION_POLICY_H_

#include "linphone/api/c-callbacks.h"
#include "linphone/api/c-types.h"

// =============================================================================

#ifdef __cplusplus
extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup authentication
 * @{
 */

/**
 * Acquire a reference.
 * @param policy The #LinphoneDigestAuthenticationPolicy object. @notnil
 * @return The same object. @notnil
 **/
LINPHONE_PUBLIC LinphoneDigestAuthenticationPolicy *
linphone_digest_authentication_policy_ref(LinphoneDigestAuthenticationPolicy *policy);

/**
 * Release reference.
 * @param policy The #LinphoneDigestAuthenticationPolicy object. @notnil
 **/
LINPHONE_PUBLIC void linphone_digest_authentication_policy_unref(LinphoneDigestAuthenticationPolicy *policy);

/**
 * Set whether MD5 hash algorithm is allowed.
 * The default value is TRUE, in order to maximize interoperability.
 * MD5 is considered as a weak algorithm, some might want to disable it, in which case
 * SHA-256 will be required to perform digest authentication.
 * @param policy The #LinphoneDigestAuthenticationPolicy object. @notnil
 * @param value a boolean value.
 **/
LINPHONE_PUBLIC void linphone_digest_authentication_policy_set_allow_md5(LinphoneDigestAuthenticationPolicy *policy,
                                                                         bool_t value);

/**
 * Get whether MD5 hash algorithm is allowed.
 * The default value is TRUE, in order to maximize interoperability.
 * MD5 is considered as a weak algorithm, some might want to disable it, in which case
 * SHA-256 will be required to perform digest authentication.
 * @param policy The #LinphoneDigestAuthenticationPolicy object. @notnil
 * @return a boolean value
 **/
LINPHONE_PUBLIC bool_t
linphone_digest_authentication_policy_get_allow_md5(const LinphoneDigestAuthenticationPolicy *policy);

/**
 * Set whether digest authentication without 'qop=auth' mode is allowed.
 * The default value is TRUE, in order to maximize interoperability.
 * 'qop=auth' mode enforces security thanks to the use of a client nonce, which makes
 * password brute forcing more difficult.
 * When set to FALSE, linphone will refuse to authenticate to servers that are not
 * implementing the qop=auth mode.
 * @param policy The #LinphoneDigestAuthenticationPolicy object. @notnil
 * @param value a boolean value.
 **/
LINPHONE_PUBLIC void linphone_digest_authentication_policy_set_allow_no_qop(LinphoneDigestAuthenticationPolicy *policy,
                                                                            bool_t value);

/**
 * Get whether digest authentication without 'qop=auth' mode is allowed.
 * The default value is TRUE, in order to maximize interoperability.
 * 'qop=auth' mode enforces security thanks to the use of a client nonce, which makes
 * password brute forcing more difficult.
 * When set to FALSE, linphone will refuse to authenticate to servers that are not
 * implementing the qop=auth mode.
 * @param policy The #LinphoneDigestAuthenticationPolicy object. @notnil
 * @return a boolean value
 **/
LINPHONE_PUBLIC bool_t
linphone_digest_authentication_policy_get_allow_no_qop(const LinphoneDigestAuthenticationPolicy *policy);

/**
 * Retrieve the user pointer associated with the #LinphoneDigestAuthenticationPolicy object.
 * @param policy The #LinphoneDigestAuthenticationPolicy object. @notnil
 * @return The user pointer. @maybenil
 **/
LINPHONE_PUBLIC void *
linphone_digest_authentication_policy_get_user_data(const LinphoneDigestAuthenticationPolicy *policy);

/**
 * Assign a user pointer to the #LinphoneDigestAuthenticationPolicy object.
 * @param policy The #LinphoneDigestAuthenticationPolicy object. @notnil
 * @param user_data The user pointer. @maybenil
 **/
LINPHONE_PUBLIC void linphone_digest_authentication_policy_set_user_data(LinphoneDigestAuthenticationPolicy *policy,
                                                                         void *user_data);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_DIGEST_AUTHENTICATION_POLICY_H_
