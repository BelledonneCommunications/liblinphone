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

#ifndef LINPHONE_LDAP_PARAMS_H
#define LINPHONE_LDAP_PARAMS_H

#include "linphone/api/c-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup ldap
 * @{
 */

/**
 * Instantiate a new #LinphoneLdapParams with values from source.
 * @param params The #LinphoneLdapParams object to be cloned. @notnil
 * @return The newly created #LinphoneLdapParams object. @notnil
 */
LINPHONE_PUBLIC LinphoneLdapParams *linphone_ldap_params_clone(const LinphoneLdapParams *params);

/**
 * Take a reference on a #LinphoneLdapParams.
 * @param params The #LinphoneLdapParams object. @notnil
 * @return the same #LinphoneLdapParams object. @notnil
 */
LINPHONE_PUBLIC LinphoneLdapParams *linphone_ldap_params_ref(LinphoneLdapParams *params);

/**
 * Release a #LinphoneLdapParams.
 * @param params The #LinphoneLdapParams object. @notnil
 */
LINPHONE_PUBLIC void linphone_ldap_params_unref(LinphoneLdapParams *params);

/*****************************************************************************************************************/

/**
 * @brief Set custom field
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @param key The key string. @notnil
 * @param value The value string. @maybenil
 **/
LINPHONE_PUBLIC void
linphone_ldap_params_set_custom_value(LinphoneLdapParams *params, const char *key, const char *value);

/**
 * @brief Get the value from field
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @param key The key string. @notnil
 * @return The Value associated  to the key. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_ldap_params_get_custom_value(const LinphoneLdapParams *params, const char *key);

/*****************************************************************************************************************/

/**
 * @brief LDAP Server.  eg: ldap:/// for a localhost server or ldap://ldap.example.org/
 * Default value: "ldap:///". This field is required.
 *
 * You must use 'ldap' scheme. 'ldaps' for LDAP over SSL is non-standardized and deprecated.
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @param server LDAP Server address. @notnil
 * @deprecated 18/11/2024 use linphone_remote_contact_directory_set_server_url() instead.
 **/
LINPHONE_PUBLIC void linphone_ldap_params_set_server(LinphoneLdapParams *params, const char *server);

/**
 * @brief Get the LDAP Server.
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @return LDAP Server address. @notnil
 * @deprecated 18/11/2024 use linphone_remote_contact_directory_get_server_url() instead.
 **/
LINPHONE_PUBLIC const char *linphone_ldap_params_get_server(const LinphoneLdapParams *params);

/*************************************************************************************/
/**
 * @brief Bind DN to use for bindings. The bindDN DN is the credential that is used to authenticate against an LDAP. If
 *empty, the connection will be Anonymous. eg: cn=ausername,ou=people,dc=bc,dc=com Default value : "".
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @param bind_dn The Bind DN to use for bindings. @maybenil
 **/
LINPHONE_PUBLIC void linphone_ldap_params_set_bind_dn(LinphoneLdapParams *params, const char *bind_dn);

/**
 * @brief Get the Bind DN to use for bindings. The bindDN DN is the credential that is used to authenticate against an
 *LDAP. If empty, the connection will be Anonymous. eg: cn=ausername,ou=people,dc=bc,dc=com
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @return The Bind DN to use for bindings. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_ldap_params_get_bind_dn(const LinphoneLdapParams *params);

/*************************************************************************************/

/**
 * @brief BaseObject is a specification for LDAP Search Scopes that specifies that the Search Request should only be
 *performed against the entry specified as the search base DN. No entries above it will be considered. This field is
 *required. Default value : "dc=example,dc=com"
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @param base_object The specification. @notnil
 **/
LINPHONE_PUBLIC void linphone_ldap_params_set_base_object(LinphoneLdapParams *params, const char *base_object);

/**
 * @brief Get the BaseObject. It is a specification for LDAP Search Scopes that specifies that the Search Request should
 *only be performed against the entry specified as the search base DN. No entries above it will be considered. This
 *field is required.
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @return The specification. @notnil
 **/
LINPHONE_PUBLIC const char *linphone_ldap_params_get_base_object(const LinphoneLdapParams *params);

/*************************************************************************************/

/**
 * @brief Timeout for requests in seconds. It limits the time for searchs and the value is passed to Ldap with
 *`LDAP_OPT_NETWORK_TIMEOUT`. Default value : 5.
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @param timeout The timeout in seconds.
 * @deprecated 18/11/2024 use linphone_remote_contact_directory_set_timeout() instead.
 **/
LINPHONE_PUBLIC void linphone_ldap_params_set_timeout(LinphoneLdapParams *params, int timeout);

/**
 * @brief Get the timeout for requests in seconds
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @return The timeout in seconds.
 * @deprecated 18/11/2024 use linphone_remote_contact_directory_get_timeout() instead.
 **/
LINPHONE_PUBLIC int linphone_ldap_params_get_timeout(const LinphoneLdapParams *params);

/**
 * @brief Timeout for TLS connection in milliseconds.
 * Default value : 1000.
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @param timeout The timeout in milliseconds.
 **/
LINPHONE_PUBLIC void linphone_ldap_params_set_timeout_tls_ms(LinphoneLdapParams *params, int timeout);

/**
 * @brief Get the timeout for TLS connection in milliseconds.
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @return The timeout in milliseconds.
 **/
LINPHONE_PUBLIC int linphone_ldap_params_get_timeout_tls_ms(const LinphoneLdapParams *params);
/*************************************************************************************/

/**
 * @brief The max results when requesting searches.
 * Default value : 5. This value fit for standard cases where only first results are needed.
 * Also, it avoids latency on each searchs.
 * Set this value to 0 to have an unlimited search (but magic search limitation may apply).
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @param max_results The max results when requesting searches.
 * @deprecated 18/11/2024 use linphone_remote_contact_directory_set_limit() instead.
 **/
LINPHONE_PUBLIC void linphone_ldap_params_set_max_results(LinphoneLdapParams *params, int max_results);

/**
 * @brief Get the max results when requesting searches.
 * 0 means the results aren't limited (but magic search limitation may apply).
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @return The max results when requesting searches.
 * @deprecated 18/11/2024 use linphone_remote_contact_directory_get_limit() instead.
 **/
LINPHONE_PUBLIC int linphone_ldap_params_get_max_results(const LinphoneLdapParams *params);

/*************************************************************************************/

/**
 * @brief The minimum characters needed for doing a search on LDAP servers.
 * Default value : 0.
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @param min_chars The minimum characters needed by a search.
 * @deprecated 18/11/2024 use linphone_remote_contact_directory_set_min_characters() instead.
 **/
LINPHONE_PUBLIC void linphone_ldap_params_set_min_chars(LinphoneLdapParams *params, int min_chars);

/**
 * @brief Get the minimum characters needed for doing a search on LDAP servers.
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @return The minimum characters needed by a search.
 * @deprecated 18/11/2024 use linphone_remote_contact_directory_get_min_characters() instead.
 **/
LINPHONE_PUBLIC int linphone_ldap_params_get_min_chars(const LinphoneLdapParams *params);

/*************************************************************************************/

/**
 * @brief Delay between each search in milliseconds
 * Default value : 500.
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @param delay The timeout in milliseconds.
 **/
LINPHONE_PUBLIC void linphone_ldap_params_set_delay(LinphoneLdapParams *params, int delay);

/**
 * @brief Get the delay between each search in milliseconds
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @return The delay in milliseconds.
 **/
LINPHONE_PUBLIC int linphone_ldap_params_get_delay(const LinphoneLdapParams *params);

/*************************************************************************************/

/**
 * @brief Authentification method. Check #LinphoneLdapAuthMethod for authentification values.
 * Default value : LinphoneLdapAuthMethodSimple
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @param auth_method The #LinphoneLdapAuthMethod.
 **/
LINPHONE_PUBLIC void linphone_ldap_params_set_auth_method(LinphoneLdapParams *params,
                                                          LinphoneLdapAuthMethod auth_method);

/**
 * @brief Get the authentification method. Check #LinphoneLdapAuthMethod for authentification values.
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @return The #LinphoneLdapAuthMethod.
 **/
LINPHONE_PUBLIC LinphoneLdapAuthMethod linphone_ldap_params_get_auth_method(const LinphoneLdapParams *params);
/*************************************************************************************/

/**
 * @brief The password to pass to server when binding.
 * Default value : "".
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @param password The password to pass to server when binding. @maybenil
 **/
LINPHONE_PUBLIC void linphone_ldap_params_set_password(LinphoneLdapParams *params, const char *password);

/**
 * @brief Get the password to pass to server when binding.
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @return The password to pass to server when binding. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_ldap_params_get_password(const LinphoneLdapParams *params);
/*************************************************************************************/

/**
 * @brief The search is based on this filter to search contacts.
 * Default value : "(sn=*%s*)".
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @param filter The filter to use. @maybenil
 **/
LINPHONE_PUBLIC void linphone_ldap_params_set_filter(LinphoneLdapParams *params, const char *filter);

/**
 * @brief Get the search is based on this filter to search contacts.
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @return The filter to use. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_ldap_params_get_filter(const LinphoneLdapParams *params);

/*************************************************************************************/

/**
 * @brief Check these attributes to build Name Friend, separated by a comma and the first is the highest priority.
 * Default value : "sn".
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @param name_attribute The comma separated attributes for the search. @maybenil
 **/
LINPHONE_PUBLIC void linphone_ldap_params_set_name_attribute(LinphoneLdapParams *params, const char *name_attribute);

/**
 * @brief Get the attributes to build Name Friend, separated by a comma and the first is the highest priority.
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @return The comma separated attributes for the search. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_ldap_params_get_name_attribute(const LinphoneLdapParams *params);

/*************************************************************************************/

/**
 * @brief  Check these attributes to build the SIP username in address of Friend. Attributes are separated by a comma.
 * Default value : "mobile,telephoneNumber,homePhone,sn".
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @param sip_attribute The comma separated attributes for building Friend. @maybenil
 **/
LINPHONE_PUBLIC void linphone_ldap_params_set_sip_attribute(LinphoneLdapParams *params, const char *sip_attribute);

/**
 * @brief Get the attributes to build the SIP username in address of Friend. Attributes are separated by a comma.
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @return The comma separated attributes for building Friend. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_ldap_params_get_sip_attribute(const LinphoneLdapParams *params);

/*************************************************************************************/

/**
 * @brief Add the domain to the sip address(sip:username\@domain).
 * By default or if it is empty, the domain will be specify while searching on the current proxy account.
 * Default value : "".
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @param sip_domain The SIP domain for the friend. @maybenil
 **/
LINPHONE_PUBLIC void linphone_ldap_params_set_sip_domain(LinphoneLdapParams *params, const char *sip_domain);

/**
 * @brief Get the domain to the sip address(sip:username\@domain).
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @return The SIP domain for the friend. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_ldap_params_get_sip_domain(const LinphoneLdapParams *params);

/*************************************************************************************/

/**
 * @brief If this config is enabled.
 * Default value : FALSE.
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @param enable Enable or not the LDAP configuration.
 **/
LINPHONE_PUBLIC void linphone_ldap_params_set_enabled(LinphoneLdapParams *params, bool_t enable);

/**
 * @brief Return if the configuration is enabled.
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @return Enable or not the LDAP configuration.
 **/
LINPHONE_PUBLIC bool_t linphone_ldap_params_get_enabled(const LinphoneLdapParams *params);

/*************************************************************************************/

/**
 * @brief Handle the DNS resolution using liblinphone's own resolver, which allows to use DNS SRV records.
 * Default value : TRUE.
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @param enable Enable or not the use of own DNS resolver.
 **/
LINPHONE_PUBLIC void linphone_ldap_params_enable_sal(LinphoneLdapParams *params, bool_t enable);

/**
 * @brief Handle the DNS resolution using liblinphone's own resolver, which allows to use DNS SRV records.
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @return whether liblinphone'own DNS resolver is used.
 **/
LINPHONE_PUBLIC bool_t linphone_ldap_params_sal_enabled(const LinphoneLdapParams *params);

/*************************************************************************************/

/**
 * @brief Encrypt transactions by LDAP over TLS(StartTLS). You must use 'ldap' scheme. 'ldaps' for LDAP over SSL is
 *non-standardized and deprecated. StartTLS in an extension to the LDAP protocol which uses the TLS protocol to encrypt
 *communication. It works by establishing a normal - i.e. unsecured - connection with the LDAP server before a handshake
 *negotiation between the server and the web services is carried out. Here, the server sends its certificate to prove
 *its identity before the secure connection is established. Default value : TRUE.
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @param enable Enable or not the use of TLS.
 **/
LINPHONE_PUBLIC void linphone_ldap_params_enable_tls(LinphoneLdapParams *params, bool_t enable);

/**
 * @brief Return if transactions are encrypted by LDAP over TLS(StartTLS). You must use \'ldap\' scheme. \'ldaps\' for
 *LDAP over SSL is non-standardized and deprecated. StartTLS in an extension to the LDAP protocol which uses the TLS
 *protocol to encrypt communication. It works by establishing a normal - i.e. unsecured - connection with the LDAP
 *server before a handshake negotiation between the server and the web services is carried out. Here, the server sends
 *its certificate to prove its identity before the secure connection is established.
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @return Enable or not the use of TLS.
 **/
LINPHONE_PUBLIC bool_t linphone_ldap_params_tls_enabled(const LinphoneLdapParams *params);

/*************************************************************************************/

/**
 * @brief The debug verbosity level from internal LDAP API. Values are #LinphoneLdapDebugLevel
 * Default value: LinphoneLdapDebugLevelOff
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @param level The #LinphoneLdapDebugLevel debug level.
 **/
LINPHONE_PUBLIC void linphone_ldap_params_set_debug_level(LinphoneLdapParams *params, LinphoneLdapDebugLevel level);

/**
 * @brief Return the debug verbosity level.
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @return The #LinphoneLdapDebugLevel debug level.
 **/
LINPHONE_PUBLIC LinphoneLdapDebugLevel linphone_ldap_params_get_debug_level(const LinphoneLdapParams *params);

/*************************************************************************************/

/**
 * @brief Specify whether the tls server certificate must be verified when connecting to a LDAP server.
 * Values are #LinphoneLdapCertVerificationMode
 * Default value : LinphoneLdapCertVerificationDefault (auto)
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @param verify_server_certificates The TLS verification mode from #LinphoneLdapCertVerificationMode
 **/
LINPHONE_PUBLIC void linphone_ldap_params_set_server_certificates_verification_mode(
    LinphoneLdapParams *params, LinphoneLdapCertVerificationMode verify_server_certificates);

/**
 * @brief Return whether the tls server certificate must be verified when connecting to a LDAP server.
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @return The TLS verification mode from #LinphoneLdapCertVerificationMode
 **/
LINPHONE_PUBLIC LinphoneLdapCertVerificationMode
linphone_ldap_params_get_server_certificates_verification_mode(const LinphoneLdapParams *params);

/**
 * @brief Check parameters and return what are wrong.
 *
 * @param params The #LinphoneLdapParams object. @notnil
 * @return The #LinphoneLdapCheck values. LinphoneLdapCheckOk if everything is ok.
 **/
LINPHONE_PUBLIC int linphone_ldap_params_check(const LinphoneLdapParams *params);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_LDAP_PARAMS_H */
