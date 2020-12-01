/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
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
	const char *realm,
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
 * @param auth_info The #LinphoneAuthInfo object to be cloned. @notnil
 * @return The newly created #LinphoneAuthInfo object. @notnil
 */
LINPHONE_PUBLIC LinphoneAuthInfo *linphone_auth_info_clone(const LinphoneAuthInfo *auth_info);

/**
 * Take a reference on a #LinphoneAuthInfo.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @return the same #LinphoneAuthInfo object.
 */
LINPHONE_PUBLIC LinphoneAuthInfo *linphone_auth_info_ref(LinphoneAuthInfo *auth_info);

/**
 * Release a #LinphoneAuthInfo.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 */
LINPHONE_PUBLIC void linphone_auth_info_unref(LinphoneAuthInfo *auth_info);

/**
 * Sets the password.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @param password The password. @maybenil
**/
LINPHONE_PUBLIC void linphone_auth_info_set_password(LinphoneAuthInfo *auth_info, const char *password);

/**
 * Sets the username.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @param username The username. @maybenil
**/
LINPHONE_PUBLIC void linphone_auth_info_set_username(LinphoneAuthInfo *auth_info, const char *username);

/**
 * Sets the algorithm to use.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @param algorithm The algorithm. @maybenil
**/
LINPHONE_PUBLIC void linphone_auth_info_set_algorithm(LinphoneAuthInfo *auth_info, const char *algorithm);

/**
 * Sets the available algorithms list without testing unicity
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @param algorithms The available algorithms list. @bctbx_list{const char *} @maybenil
*/
LINPHONE_PUBLIC void linphone_auth_info_set_available_algorithms(LinphoneAuthInfo *auth_info, const bctbx_list_t *algorithms);

/**
 * Add an unique algorithm in the the available algorithms list : Algorithms that already exist will not be added.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @param algorithm The algorithm to add. @maybenil
**/
LINPHONE_PUBLIC void linphone_auth_info_add_available_algorithm(LinphoneAuthInfo *auth_info, const char *algorithm);

/**
 * Sets the user ID.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @param user_id The userid.@maybenil
**/
LINPHONE_PUBLIC void linphone_auth_info_set_userid(LinphoneAuthInfo *auth_info, const char *user_id);

/**
 * Sets the realm.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @param realm The realm. @maybenil
**/
LINPHONE_PUBLIC void linphone_auth_info_set_realm(LinphoneAuthInfo *auth_info, const char *realm);

/**
 * Sets the domain for which this authentication is valid.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @param domain The domain. @maybenil
 * This should not be necessary because realm is supposed to be unique and sufficient.
 * However, many SIP servers don't set realm correctly, then domain has to be used to distinguish between several SIP account bearing the same username.
**/
LINPHONE_PUBLIC void linphone_auth_info_set_domain(LinphoneAuthInfo *auth_info, const char *domain);

/**
 * Sets the ha1.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @param ha1 The ha1. @maybenil
**/
LINPHONE_PUBLIC void linphone_auth_info_set_ha1(LinphoneAuthInfo *auth_info, const char *ha1);

/**
 * Sets the TLS certificate.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @param tls_cert The TLS certificate. @maybenil
**/
LINPHONE_PUBLIC void linphone_auth_info_set_tls_cert(LinphoneAuthInfo *auth_info, const char *tls_cert);

/**
 * Sets the TLS key.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @param tls_key The TLS key. @maybenil
**/
LINPHONE_PUBLIC void linphone_auth_info_set_tls_key(LinphoneAuthInfo *auth_info, const char *tls_key);

/**
 * Sets the TLS certificate path.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @param tls_cert_path The TLS certificate path. @maybenil
**/
LINPHONE_PUBLIC void linphone_auth_info_set_tls_cert_path(LinphoneAuthInfo *auth_info, const char *tls_cert_path);

/**
 * Sets the TLS key path.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @param tls_key_path The TLS key path. @maybenil
**/
LINPHONE_PUBLIC void linphone_auth_info_set_tls_key_path(LinphoneAuthInfo *auth_info, const char *tls_key_path);

/**
 * Remove all algorithms from the available algorithms list.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
**/
LINPHONE_PUBLIC void linphone_auth_info_clear_available_algorithms(LinphoneAuthInfo *auth_info);

/*
 * Sets the TLS key password, this is only usefull is the TLS key is encrypted.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @param tls_key_password The TLS key password. @maybenil
**/
LINPHONE_PUBLIC void linphone_auth_info_set_tls_key_password(LinphoneAuthInfo *auth_info, const char *tls_key_password);


/**
 * Gets the username.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @return The username. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_auth_info_get_username(const LinphoneAuthInfo *auth_info);

/**
 * Gets the algorithm.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @return The algorithm. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_auth_info_get_algorithm(const LinphoneAuthInfo *auth_info);

/**
 * Gets all available algorithms.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @return A list of available algorithms. @bctbx_list{const char *} @maybenil
 */
LINPHONE_PUBLIC bctbx_list_t * linphone_auth_info_get_available_algorithms(const LinphoneAuthInfo *auth_info);

/**
 * Gets the password.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @return The password. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_auth_info_get_password(const LinphoneAuthInfo *auth_info);

/**
 * Gets the user id.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @return The user id. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_auth_info_get_userid(const LinphoneAuthInfo *auth_info);

/**
 * Gets the realm.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @return The realm. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_auth_info_get_realm(const LinphoneAuthInfo *auth_info);

/**
 * Gets the domain.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @return The domain. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_auth_info_get_domain(const LinphoneAuthInfo *auth_info);

/**
 * Gets the ha1.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @return The ha1. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_auth_info_get_ha1(const LinphoneAuthInfo *auth_info);

/**
 * Gets the TLS certificate.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @return The TLS certificate. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_auth_info_get_tls_cert(const LinphoneAuthInfo *auth_info);

/**
 * Gets the TLS key.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @return The TLS key. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_auth_info_get_tls_key(const LinphoneAuthInfo *auth_info);

/**
 * Gets the TLS certificate path.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @return The TLS certificate path. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_auth_info_get_tls_cert_path(const LinphoneAuthInfo *auth_info);

/**
 * Gets the TLS key path.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @return The TLS key path. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_auth_info_get_tls_key_path(const LinphoneAuthInfo *auth_info);
	
/*
 * Gets the TLS key password.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @return The TLS key password. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_auth_info_get_tls_key_password(const LinphoneAuthInfo *auth_info);

/**
 * Check if Authinfos are the same without taking account algorithms
 * @param auth_info_1 The first #LinphoneAuthInfo object. @maybenil
 * @param auth_info_2 The second #LinphoneAuthInfo object. @maybenil
 * @return TRUE if all fields (Username, UserId, Realm, Domain) are the same.
 */
LINPHONE_PUBLIC bool_t linphone_auth_info_is_equal_but_algorithms(const LinphoneAuthInfo *auth_info_1,const LinphoneAuthInfo *auth_info_2);

/* you don't need those function*/
LINPHONE_PUBLIC LINPHONE_DEPRECATED void linphone_auth_info_destroy(LinphoneAuthInfo *info);

void linphone_auth_info_write_config(LpConfig *config, LinphoneAuthInfo *auth_info, int pos);

LinphoneAuthInfo *linphone_auth_info_new_from_config_file(LpConfig *config, int pos);

/************ */
/* DEPRECATED */
/* ********** */

/**
 * Sets the password.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @param passwd The password.
 * @deprecated 13/10/2017 use linphone_auth_info_set_password instead
 * @donotwrap
**/
LINPHONE_PUBLIC LINPHONE_DEPRECATED void linphone_auth_info_set_passwd(LinphoneAuthInfo *auth_info, const char *passwd);

/**
 * Gets the password.
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @return The password.
 * @deprecated 13/10/2017 use linphone_auth_info_get_password instead
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED const char *linphone_auth_info_get_passwd(const LinphoneAuthInfo *auth_info);

/**
 * Destroys the auth info
 * @param auth_info The #LinphoneAuthInfo object. @notnil
 * @deprecated 13/10/2017 use linphone_auth_info_unref() instead
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED void linphone_auth_info_destroy(LinphoneAuthInfo *auth_info);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_AUTH_INFO_H */
