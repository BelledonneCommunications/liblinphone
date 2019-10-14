/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LINPHONE_AUTH_INFO_H
#define LINPHONE_AUTH_INFO_H

#include "linphone/api/c-types.h"

/**
 * @addtogroup authentication
 * @{
 */

/**
 * Safely cast a belle_sip_object_t into #LinphoneAuthInfo
 */

#ifdef __cplusplus
extern "C" {
#endif

LINPHONE_PUBLIC LinphoneAuthInfo *linphone_auth_info_new(
	const char *username,
	const char *userid,
	const char *passwd,
	const char *ha1,
	const char *rfealm,
	const char *domain);

LINPHONE_PUBLIC LinphoneAuthInfo *linphone_auth_info_new_for_algorithm(
	const char *username,
	const char *userid,
	const char *passwd,
	const char *ha1,
	const char *realm,
	const char *domain,
	const char *algorithm);
/**
 * Instantiates a new auth info with values from source.
 * @param[in] source The #LinphoneAuthInfo object to be cloned
 * @return The newly created #LinphoneAuthInfo object.
 */
LINPHONE_PUBLIC LinphoneAuthInfo *linphone_auth_info_clone(const LinphoneAuthInfo *source);

/**
 * Take a reference on a #LinphoneAuthInfo.
 */
LINPHONE_PUBLIC LinphoneAuthInfo *linphone_auth_info_ref(LinphoneAuthInfo *info);

/**
 * Release a #LinphoneAuthInfo.
 */
LINPHONE_PUBLIC void linphone_auth_info_unref(LinphoneAuthInfo *info);

/**
 * Sets the password.
 * @param[in] info The #LinphoneAuthInfo object
 * @param[in] passwd The password.
 * @deprecated, use linphone_auth_info_set_password instead
**/
LINPHONE_PUBLIC void linphone_auth_info_set_passwd(LinphoneAuthInfo *info, const char *passwd);

/**
 * Sets the password.
 * @param[in] info The #LinphoneAuthInfo object
 * @param[in] passwd The password.
**/
LINPHONE_PUBLIC void linphone_auth_info_set_password(LinphoneAuthInfo *info, const char *passwd);

/**
 * Sets the username.
 * @param[in] info The #LinphoneAuthInfo object
 * @param[in] username The username.
**/
LINPHONE_PUBLIC void linphone_auth_info_set_username(LinphoneAuthInfo *info, const char *username);

/**
 * Sets the algorithm.
 * @param[in] info The #LinphoneAuthInfo object
 * @param[in] algorithm The algorithm.
**/
LINPHONE_PUBLIC void linphone_auth_info_set_algorithm(LinphoneAuthInfo *info, const char *algorithm);

/**
 * Sets the userid.
 * @param[in] info The #LinphoneAuthInfo object
 * @param[in] userid The userid.
**/
LINPHONE_PUBLIC void linphone_auth_info_set_userid(LinphoneAuthInfo *info, const char *userid);

/**
 * Sets the realm.
 * @param[in] info The #LinphoneAuthInfo object
 * @param[in] realm The realm.
**/
LINPHONE_PUBLIC void linphone_auth_info_set_realm(LinphoneAuthInfo *info, const char *realm);

/**
 * Sets the domain for which this authentication is valid.
 * @param[in] info The #LinphoneAuthInfo object
 * @param[in] domain The domain.
 * This should not be necessary because realm is supposed to be unique and sufficient.
 * However, many SIP servers don't set realm correctly, then domain has to be used to distinguish between several SIP account bearing the same username.
**/
LINPHONE_PUBLIC void linphone_auth_info_set_domain(LinphoneAuthInfo *info, const char *domain);

/**
 * Sets the ha1.
 * @param[in] info The #LinphoneAuthInfo object
 * @param[in] ha1 The ha1.
**/
LINPHONE_PUBLIC void linphone_auth_info_set_ha1(LinphoneAuthInfo *info, const char *ha1);

/**
 * Sets the TLS certificate.
 * @param[in] info The #LinphoneAuthInfo object
 * @param[in] tls_cert The TLS certificate.
**/
LINPHONE_PUBLIC void linphone_auth_info_set_tls_cert(LinphoneAuthInfo *info, const char *tls_cert);

/**
 * Sets the TLS key.
 * @param[in] info The #LinphoneAuthInfo object
 * @param[in] tls_key The TLS key.
**/
LINPHONE_PUBLIC void linphone_auth_info_set_tls_key(LinphoneAuthInfo *info, const char *tls_key);

/**
 * Sets the TLS certificate path.
 * @param[in] info The #LinphoneAuthInfo object
 * @param[in] tls_cert_path The TLS certificate path.
**/
LINPHONE_PUBLIC void linphone_auth_info_set_tls_cert_path(LinphoneAuthInfo *info, const char *tls_cert_path);

/**
 * Sets the TLS key path.
 * @param[in] info The #LinphoneAuthInfo object
 * @param[in] tls_key_path The TLS key path.
**/
LINPHONE_PUBLIC void linphone_auth_info_set_tls_key_path(LinphoneAuthInfo *info, const char *tls_key_path);

/**
 * Gets the username.
 * @param[in] info The #LinphoneAuthInfo object
 * @return The username.
 */
LINPHONE_PUBLIC const char *linphone_auth_info_get_username(const LinphoneAuthInfo *info);

/**
 * Gets the algorithm.
 * @param[in] info The #LinphoneAuthInfo object
 * @return The algorithm.
 */
LINPHONE_PUBLIC const char *linphone_auth_info_get_algorithm(const LinphoneAuthInfo *info);

/**
 * Gets the password.
 * @param[in] info The #LinphoneAuthInfo object
 * @return The password.
 * @deprecated, use linphone_auth_info_get_password instead
 */
LINPHONE_PUBLIC const char *linphone_auth_info_get_passwd(const LinphoneAuthInfo *info);

/**
 * Gets the password.
 * @param[in] info The #LinphoneAuthInfo object
 * @return The password.
 */
LINPHONE_PUBLIC const char *linphone_auth_info_get_password(const LinphoneAuthInfo *info);


/**
 * Gets the userid.
 * @param[in] info The #LinphoneAuthInfo object
 * @return The userid.
 */
LINPHONE_PUBLIC const char *linphone_auth_info_get_userid(const LinphoneAuthInfo *info);

/**
 * Gets the realm.
 * @param[in] info The #LinphoneAuthInfo object
 * @return The realm.
 */
LINPHONE_PUBLIC const char *linphone_auth_info_get_realm(const LinphoneAuthInfo *info);

/**
 * Gets the domain.
 * @param[in] info The #LinphoneAuthInfo object
 * @return The domain.
 */
LINPHONE_PUBLIC const char *linphone_auth_info_get_domain(const LinphoneAuthInfo *info);

/**
 * Gets the ha1.
 * @param[in] info The #LinphoneAuthInfo object
 * @return The ha1.
 */
LINPHONE_PUBLIC const char *linphone_auth_info_get_ha1(const LinphoneAuthInfo *info);

/**
 * Gets the TLS certificate.
 * @param[in] info The #LinphoneAuthInfo object
 * @return The TLS certificate.
 */
LINPHONE_PUBLIC const char *linphone_auth_info_get_tls_cert(const LinphoneAuthInfo *info);

/**
 * Gets the TLS key.
 * @param[in] info The #LinphoneAuthInfo object
 * @return The TLS key.
 */
LINPHONE_PUBLIC const char *linphone_auth_info_get_tls_key(const LinphoneAuthInfo *info);

/**
 * Gets the TLS certificate path.
 * @param[in] info The #LinphoneAuthInfo object
 * @return The TLS certificate path.
 */
LINPHONE_PUBLIC const char *linphone_auth_info_get_tls_cert_path(const LinphoneAuthInfo *info);

/**
 * Gets the TLS key path.
 * @param[in] info The #LinphoneAuthInfo object
 * @return The TLS key path.
 */
LINPHONE_PUBLIC const char *linphone_auth_info_get_tls_key_path(const LinphoneAuthInfo *info);

/* you don't need those function*/
LINPHONE_PUBLIC LINPHONE_DEPRECATED void linphone_auth_info_destroy(LinphoneAuthInfo *info);

void linphone_auth_info_write_config(LpConfig *config, LinphoneAuthInfo *obj, int pos);

LinphoneAuthInfo *linphone_auth_info_new_from_config_file(LpConfig *config, int pos);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_AUTH_INFO_H */