/*
 * Copyright (c) 2010-2025 Belledonne Communications SARL.
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

#ifndef LINPHONE_ACCOUNT_PARAMS_H
#define LINPHONE_ACCOUNT_PARAMS_H

#include "linphone/api/c-types.h"
#include "linphone/sipsetup.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup account
 * @{
 */

/**
 * Create a new #LinphoneAccountParams object.
 * @param lc The #LinphoneCore object. @maybenil
 * @param use_default_values If TRUE, use proxy default values
 * @return The newly created #LinphoneAccountParams object. @notnil
 */
LINPHONE_PUBLIC LinphoneAccountParams *linphone_account_params_new(LinphoneCore *lc, bool_t use_default_values);

/**
 * Create a new #LinphoneAccountParams object from a configuration.
 * @param lc The #LinphoneCore object. @notnil
 * @param index The index of the configuration.
 * @return The newly created #LinphoneAccountParams object. @notnil
 */
LINPHONE_PUBLIC LinphoneAccountParams *linphone_account_params_new_with_config(LinphoneCore *lc, int index);

/**
 * Instantiate a new account params with values from source.
 * @param params The #LinphoneAccountParams object to be cloned. @notnil
 * @return The newly created #LinphoneAccountParams object. @notnil
 */
LINPHONE_PUBLIC LinphoneAccountParams *linphone_account_params_clone(const LinphoneAccountParams *params);

/**
 * Take a reference on a #LinphoneAccountParams.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return the same #LinphoneAccountParams object. @notnil
 */
LINPHONE_PUBLIC LinphoneAccountParams *linphone_account_params_ref(LinphoneAccountParams *params);

/**
 * Release a #LinphoneAccountParams.
 * @param params The #LinphoneAccountParams object. @notnil
 */
LINPHONE_PUBLIC void linphone_account_params_unref(LinphoneAccountParams *params);

/**
 * Assign a user pointer to the account params.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param user_data The user data. @maybenil
 */
LINPHONE_PUBLIC void linphone_account_params_set_user_data(LinphoneAccountParams *params, void *user_data);

/**
 * Retrieve the user pointer associated with the account params.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The user data. @maybenil
 */
LINPHONE_PUBLIC void *linphone_account_params_get_user_data(const LinphoneAccountParams *params);

/**
 * Sets the SIP proxy or registrar address.
 *
 * Examples of valid sip proxy address are:
 * - IP address: sip:87.98.157.38
 * - IP address with port: sip:87.98.157.38:5062
 * - hostnames : sip:sip.example.net
 *
 * @param params The #LinphoneAccountParams object. @notnil
 * @param server_address The proxy as #LinphoneAddress to set. @maybenil
 * @return 0 if successful, -1 otherwise.
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_account_params_set_server_address(LinphoneAccountParams *params,
                                                                          const LinphoneAddress *server_address);

/**
 * Sets the proxy address
 *
 * Examples of valid sip proxy address are:
 * - IP address: sip:87.98.157.38
 * - IP address with port: sip:87.98.157.38:5062
 * - hostnames : sip:sip.example.net
 *
 * @param params The #LinphoneAccountParams object. @notnil
 * @param server_address The proxy address to set. @maybenil
 * @return 0 if successful, -1 otherwise.
 * @deprecated 01/03/2021 Use #linphone_account_params_set_server_address() instead.
 **/
LINPHONE_DEPRECATED LINPHONE_PUBLIC LinphoneStatus
linphone_account_params_set_server_addr(LinphoneAccountParams *params, const char *server_address);

/**
 * Sets the user identity as a SIP address.
 *
 * This identity is normally formed with display name, username and domain, such
 * as:
 * Alice <sip:alice@example.net>
 * The REGISTER messages will have from and to set to this identity.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param identity The #LinphoneAddress of the identity to set. @maybenil
 * @return 0 if successful, -1 otherwise.
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_account_params_set_identity_address(LinphoneAccountParams *params,
                                                                            const LinphoneAddress *identity);

/**
 * Sets a list of SIP route.
 * When a route is set, all outgoing calls will go to the route's destination if this account
 * is the default one (see linphone_core_set_default_account()).
 *
 * @warning This function shall not be used in conjunction with linphone_account_params_enable_outbound_proxy().
 * @param params The #LinphoneAccountParams object. @notnil
 * @param routes A list of routes. \bctbx_list{LinphoneAddress} @maybenil
 * @return -1 if routes are invalid, 0 otherwise.
 **/
LINPHONE_PUBLIC LinphoneStatus linphone_account_params_set_routes_addresses(LinphoneAccountParams *params,
                                                                            const bctbx_list_t *routes);

/**
 * Sets the registration expiration time in seconds.
 * @param params #LinphoneAccountParams object. @notnil
 * @param expires The expiration time to set.
 **/
LINPHONE_PUBLIC void linphone_account_params_set_expires(LinphoneAccountParams *params, int expires);

/**
 * Indicates  either or not, REGISTRATION must be issued for this #LinphoneAccountParams.
 * @param params #LinphoneAccountParams object. @notnil
 * @param enable If TRUE, registration will be engaged.
 * @deprecated 16/12/2021 Use linphone_account_params_enable_register() instead.
 */
LINPHONE_DEPRECATED LINPHONE_PUBLIC void linphone_account_params_set_register_enabled(LinphoneAccountParams *params,
                                                                                      bool_t enable);

/**
 * Indicates  either or not, REGISTRATION must be issued for this #LinphoneAccountParams.
 * @param params #LinphoneAccountParams object. @notnil
 * @param enable If TRUE, registration will be engaged.
 */
LINPHONE_PUBLIC void linphone_account_params_enable_register(LinphoneAccountParams *params, bool_t enable);

/**
 * Indicates either or not, PUBLISH must be issued for this #LinphoneAccountParams.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param enable If TRUE, publish will be engaged.
 * @deprecated 16/12/2021 Use linphone_account_params_enable_publish() instead.
 */
LINPHONE_DEPRECATED LINPHONE_PUBLIC void linphone_account_params_set_publish_enabled(LinphoneAccountParams *params,
                                                                                     bool_t enable);

/**
 * Indicates either or not, PUBLISH must be issued for this #LinphoneAccountParams.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param enable If TRUE, publish will be engaged.
 */
LINPHONE_PUBLIC void linphone_account_params_enable_publish(LinphoneAccountParams *params, bool_t enable);

/**
 * Set the publish expiration time in second.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param expires The expire time in seconds.
 * */
LINPHONE_PUBLIC void linphone_account_params_set_publish_expires(LinphoneAccountParams *params, int expires);

/**
 * Get the publish expiration time in second. Default value is the registration expiration value.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The expire time in seconds.
 * */
LINPHONE_PUBLIC int linphone_account_params_get_publish_expires(const LinphoneAccountParams *params);

/**
 * Set whether liblinphone should replace "+" by international calling prefix in dialed numbers (passed to
 * #linphone_core_invite).
 * @param params The #LinphoneAccountParams object. @notnil
 * @param enable TRUE to replace + by the international prefix, FALSE otherwise.
 * @deprecated 16/12/2021 Use linphone_account_params_enable_dial_escape_plus() instead.
 **/
LINPHONE_DEPRECATED LINPHONE_PUBLIC void
linphone_account_params_set_dial_escape_plus_enabled(LinphoneAccountParams *params, bool_t enable);

/**
 * Set whether liblinphone should replace "+" by international calling prefix in dialed numbers (passed to
 * #linphone_core_invite).
 * @param params The #LinphoneAccountParams object. @notnil
 * @param enable TRUE to replace + by the international prefix, FALSE otherwise.
 **/
LINPHONE_PUBLIC void linphone_account_params_enable_dial_escape_plus(LinphoneAccountParams *params, bool_t enable);

/**
 * Sets an international prefix to be automatically prepended when inviting a number with
 * linphone_core_invite();
 * This international prefix shall usually be the country code of the country where the user is living, without "+".
 * @param params The #LinphoneAccountParams object. @notnil
 * @param prefix The prefix to set (withouth the +). @maybenil
 **/
LINPHONE_PUBLIC void linphone_account_params_set_international_prefix(LinphoneAccountParams *params,
                                                                      const char *prefix);

/**
 * Sets the ISO country code matching the international prefix.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param prefix_iso_country_code The ISO country code to set. @maybenil
 **/
LINPHONE_PUBLIC void
linphone_account_params_set_international_prefix_iso_country_code(LinphoneAccountParams *params,
                                                                  const char *prefix_iso_country_code);

/**
 * If enabled, the international prefix will automaticaly be used for calls and chats.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param enable TRUE to use the international prefix for calls, FALSE otherwise.
 */
LINPHONE_PUBLIC void
linphone_account_params_set_use_international_prefix_for_calls_and_chats(LinphoneAccountParams *params, bool_t enable);

/**
 * Indicates whether quality statistics during call should be stored and sent to a collector according to RFC 6035.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param enable TRUE to store quality statistics and send them to the collector, FALSE to disable it.
 * @deprecated 16/12/2021 Use linphone_account_params_enable_quality_reporting() instead.
 */
LINPHONE_DEPRECATED LINPHONE_PUBLIC void
linphone_account_params_set_quality_reporting_enabled(LinphoneAccountParams *params, bool_t enable);

/**
 * Indicates whether quality statistics during call should be stored and sent to a collector according to RFC 6035.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param enable TRUE to store quality statistics and send them to the collector, FALSE to disable it.
 */
LINPHONE_PUBLIC void linphone_account_params_enable_quality_reporting(LinphoneAccountParams *params, bool_t enable);

/**
 * Indicates whether quality statistics during call should be stored and sent to a collector according to RFC 6035.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return TRUE if quality repotring is enabled, FALSE otherwise.
 * @deprecated 16/12/2021 Use linphone_account_params_quality_reporting_enabled() instead.
 */
LINPHONE_DEPRECATED LINPHONE_PUBLIC bool_t
linphone_account_params_get_quality_reporting_enabled(const LinphoneAccountParams *params);

/**
 * Indicates whether quality statistics during call should be stored and sent to a collector according to RFC 6035.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return TRUE if quality repotring is enabled, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_account_params_quality_reporting_enabled(const LinphoneAccountParams *params);

/**
 * Set the route of the collector end-point when using quality reporting. This SIP address
 * should be used on server-side to process packets directly before discarding packets. Collector address
 * should be a non existing account and will not receive any messages.
 * If NULL, reports will be send to the proxy domain.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param collector route of the collector end-point, if NULL PUBLISH will be sent to the proxy domain. @maybenil
 */
LINPHONE_PUBLIC void linphone_account_params_set_quality_reporting_collector(LinphoneAccountParams *params,
                                                                             const char *collector);

/**
 * Get the route of the collector end-point when using quality reporting. This SIP address
 * should be used on server-side to process packets directly before discarding packets. Collector address
 * should be a non existing account and will not receive any messages.
 * If NULL, reports will be send to the proxy domain.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The SIP address of the collector end-point. @maybenil
 */
LINPHONE_PUBLIC const char *
linphone_account_params_get_quality_reporting_collector(const LinphoneAccountParams *params);

/**
 * Set the interval between 2 interval reports sending when using quality reporting. If call exceed interval size, an
 * interval report will be sent to the collector. On call termination, a session report will be sent
 * for the remaining period. Value must be 0 (disabled) or positive.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param interval The interval in seconds, 0 means interval reports are disabled.
 */
LINPHONE_PUBLIC void linphone_account_params_set_quality_reporting_interval(LinphoneAccountParams *params,
                                                                            int interval);

/**
 * Get the interval between interval reports when using quality reporting.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The interval in seconds, 0 means interval reports are disabled.
 */
LINPHONE_PUBLIC int linphone_account_params_get_quality_reporting_interval(const LinphoneAccountParams *params);

/**
 * Get the domain name of the given account params.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The domain name of the account params. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_account_params_get_domain(const LinphoneAccountParams *params);

/**
 * Get the realm of the given account params.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The realm of the account params. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_account_params_get_realm(const LinphoneAccountParams *params);

/**
 * Set the realm of the given account params.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param realm New realm value. @maybenil
 **/
LINPHONE_PUBLIC void linphone_account_params_set_realm(LinphoneAccountParams *params, const char *realm);

/**
 * Gets the list of the routes set for this account params.
 *
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The list of routes. \bctbx_list{LinphoneAddress} @maybenil
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_account_params_get_routes_addresses(const LinphoneAccountParams *params);

/**
 * Get the identity address of the account params.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The SIP identity that belongs to this account params. @maybenil
 **/
LINPHONE_PUBLIC const LinphoneAddress *
linphone_account_params_get_identity_address(const LinphoneAccountParams *params);

/**
 * Get the identity of the account params.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The SIP identity that belongs to this account params. @maybenil
 * @deprecated 01/03/2021 Use linphone_account_params_get_identity_address() instead.
 **/
LINPHONE_DEPRECATED LINPHONE_PUBLIC const char *
linphone_account_params_get_identity(const LinphoneAccountParams *params);

/**
 * Tell if the PUBLISH is enabled.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return TRUE if PUBLISH request is enabled for this proxy.
 * @deprecated 16/12/2021 Use linphone_account_params_publish_enabled() instead.
 **/
LINPHONE_DEPRECATED LINPHONE_PUBLIC bool_t
linphone_account_params_get_publish_enabled(const LinphoneAccountParams *params);

/**
 * Tell if the PUBLISH is enabled.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return TRUE if PUBLISH request is enabled for this proxy.
 **/
LINPHONE_PUBLIC bool_t linphone_account_params_publish_enabled(const LinphoneAccountParams *params);

/**
 * Get the account params SIP proxy or registrar address.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The proxy's SIP #LinphoneAddress. @maybenil
 **/
LINPHONE_PUBLIC const LinphoneAddress *linphone_account_params_get_server_address(const LinphoneAccountParams *params);

/**
 * Get the account params proxy address.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The proxy's SIP address. @maybenil
 * @deprecated 01/03/2021 Use linphone_account_params_get_server_address() instead.
 **/
LINPHONE_DEPRECATED LINPHONE_PUBLIC const char *
linphone_account_params_get_server_addr(const LinphoneAccountParams *params);

/**
 * Get the account params expires.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The duration of registration.
 **/
LINPHONE_PUBLIC int linphone_account_params_get_expires(const LinphoneAccountParams *params);

/**
 * Returns whether the account params is enabled or not.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return TRUE if registration to the proxy is enabled.
 * @deprecated 16/12/2021 Use linphone_account_params_register_enabled() instead.
 **/
LINPHONE_DEPRECATED LINPHONE_PUBLIC bool_t
linphone_account_params_get_register_enabled(const LinphoneAccountParams *params);

/**
 * Returns whether the account params is enabled or not.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return TRUE if registration to the proxy is enabled.
 **/
LINPHONE_PUBLIC bool_t linphone_account_params_register_enabled(const LinphoneAccountParams *params);

/**
 * Returns the contact parameters.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The previously set contact parameters. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_account_params_get_contact_parameters(const LinphoneAccountParams *params);

/**
 * Set optional contact parameters that will be added to the contact information sent in the registration.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param contact_params A string contaning the additional parameters in text form, like
 *"myparam=something;myparam2=something_else" @maybenil
 *
 * The main use case for this function is provide the proxy additional information regarding the user agent, like for
 *example unique identifier or apple push id. As an example, the contact address in the SIP register sent will look like
 *<sip:joe@15.128.128.93:50421>;apple-push-id=43143-DFE23F-2323-FA2232.
 **/
LINPHONE_PUBLIC void linphone_account_params_set_contact_parameters(LinphoneAccountParams *params,
                                                                    const char *contact_params);

/**
 * Set optional contact parameters that will be added to the contact information sent in the registration, inside the
 *URI.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param contact_uri_params A string containing the additional parameters in text form, like
 *"myparam=something;myparam2=something_else" @maybenil
 *
 * The main use case for this function is provide the proxy additional information regarding the user agent, like for
 *example unique identifier or apple push id. As an example, the contact address in the SIP register sent will look like
 *<sip:joe@15.128.128.93:50421;apple-push-id=43143-DFE23F-2323-FA2232>.
 **/
LINPHONE_PUBLIC void linphone_account_params_set_contact_uri_parameters(LinphoneAccountParams *params,
                                                                        const char *contact_uri_params);

/**
 * Return the contact URI parameters.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The previously set contact URI parameters. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_account_params_get_contact_uri_parameters(const LinphoneAccountParams *params);

/**
 * Return whether or not the + should be replaced by 00.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return Whether liblinphone should replace "+" by "00" in dialed numbers (passed to #linphone_core_invite()).
 * @deprecated 16/12/2021 Use linphone_account_params_dial_escape_plus_enabled() instead.
 **/
LINPHONE_DEPRECATED LINPHONE_PUBLIC bool_t
linphone_account_params_get_dial_escape_plus_enabled(const LinphoneAccountParams *params);

/**
 * Return whether or not the + should be replaced by 00.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return Whether liblinphone should replace "+" by "00" in dialed numbers (passed to #linphone_core_invite()).
 **/
LINPHONE_PUBLIC bool_t linphone_account_params_dial_escape_plus_enabled(const LinphoneAccountParams *params);

/**
 * Gets the prefix set for this account params.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The international prefix if set, null otherwise. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_account_params_get_international_prefix(const LinphoneAccountParams *params);

/**
 * Gets the ISO country code set for the international prefix in this account params.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The international prefix ISO country code if set, null otherwise. @maybenil
 **/
LINPHONE_PUBLIC const char *
linphone_account_params_get_international_prefix_iso_country_code(const LinphoneAccountParams *params);

/**
 * Return whether or not the international prefix will automaticaly be used for calls and chats.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return Whether we should use international prefix automatically for calls.
 */
LINPHONE_PUBLIC bool_t
linphone_account_params_get_use_international_prefix_for_calls_and_chats(const LinphoneAccountParams *params);

/**
 * Set default privacy policy for all calls routed through this proxy.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param privacy #LinphonePrivacyMask to configure privacy
 * */
LINPHONE_PUBLIC void linphone_account_params_set_privacy(LinphoneAccountParams *params, LinphonePrivacyMask privacy);

/**
 * Get default privacy policy for all calls routed through this proxy.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return Privacy mode as #LinphonePrivacyMask
 * */
LINPHONE_PUBLIC LinphonePrivacyMask linphone_account_params_get_privacy(const LinphoneAccountParams *params);

/**
 * Set the http file transfer server to be used for content type application/vnd.gsma.rcs-ft-http+xml
 * @param params The #LinphoneAccountParams object. @notnil
 * @param server_url URL of the file server like https://file.linphone.org/upload.php @maybenil
 * @warning That function isn't implemented yet.
 * @donotwrap
 * */
LINPHONE_PUBLIC void linphone_account_params_set_file_transfer_server(LinphoneAccountParams *params,
                                                                      const char *server_url);

/**
 * Get the http file transfer server to be used for content type application/vnd.gsma.rcs-ft-http+xml
 * @param params The #LinphoneAccountParams object. @notnil
 * @return URL of the file server like https://file.linphone.org/upload.php @maybenil
 * @warning That function isn't implemented yet.
 * @donotwrap
 * */
LINPHONE_PUBLIC const char *linphone_account_params_get_file_transfer_server(const LinphoneAccountParams *params);

/**
 * Set the interval between regular RTCP reports when using AVPF/SAVPF.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param interval The interval in seconds (between 0 and 5 seconds).
 */
LINPHONE_PUBLIC void linphone_account_params_set_avpf_rr_interval(LinphoneAccountParams *params, uint8_t interval);

/**
 * Get the interval between regular RTCP reports when using AVPF/SAVPF.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The interval in seconds.
 */
LINPHONE_PUBLIC uint8_t linphone_account_params_get_avpf_rr_interval(const LinphoneAccountParams *params);

/**
 * Get enablement status of RTCP feedback (also known as AVPF profile).
 * @param params The #LinphoneAccountParams object. @notnil
 * @return the enablement mode, which can be #LinphoneAVPFDefault (use LinphoneCore's mode), #LinphoneAVPFEnabled (avpf
 *is enabled), or #LinphoneAVPFDisabled (disabled).
 **/
LINPHONE_PUBLIC LinphoneAVPFMode linphone_account_params_get_avpf_mode(const LinphoneAccountParams *params);

/**
 * Enable the use of RTCP feedback (also known as AVPF profile).
 * @param params The #LinphoneAccountParams object. @notnil
 * @param mode the enablement mode, which can be #LinphoneAVPFDefault (use LinphoneCore's mode), #LinphoneAVPFEnabled
 *(avpf is enabled), or #LinphoneAVPFDisabled (disabled).
 **/
LINPHONE_PUBLIC void linphone_account_params_set_avpf_mode(LinphoneAccountParams *params, LinphoneAVPFMode mode);

/**
 * Get the persistent reference key associated to the account params.
 *
 * The reference key can be for example an id to an external database.
 * It is stored in the config file, thus can survive to process exits/restarts.
 *
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The reference key string that has been associated to the account params, or NULL if none has been associated.
 *@maybenil
 **/
LINPHONE_PUBLIC const char *linphone_account_params_get_ref_key(const LinphoneAccountParams *params);

/**
 * Associate a persistent reference key to the account params.
 *
 * The reference key can be for example an id to an external database.
 * It is stored in the config file, thus can survive to process exits/restarts.
 *
 * @param params The #LinphoneAccountParams object. @notnil
 * @param refkey The reference key string to associate to the account params. @maybenil
 **/
LINPHONE_PUBLIC void linphone_account_params_set_ref_key(LinphoneAccountParams *params, const char *refkey);

/**
 * Get the idkey property of a #LinphoneAccountParams.
 *
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The idkey string, or NULL. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_account_params_get_idkey(const LinphoneAccountParams *params);

/**
 * Set the idkey property on the given account params.
 * This property can the be referenced by another account params 'depends_on' to create a dependency relation between
 *them.
 *
 * @param params The #LinphoneAccountParams object. @notnil
 * @param idkey The idkey string to associate to the given #LinphoneAccountParams. @maybenil
 **/
LINPHONE_PUBLIC void linphone_account_params_set_idkey(LinphoneAccountParams *params, const char *idkey);

/**
 * Get The policy that is used to pass through NATs/firewalls when using this account params.
 * If it is set to NULL, the default NAT policy from the core will be used instead.
 * @param params The #LinphoneAccountParams object @notnil
 * @return The #LinphoneNatPolicy object in use. @maybenil
 * @see linphone_core_get_nat_policy()
 */
LINPHONE_PUBLIC LinphoneNatPolicy *linphone_account_params_get_nat_policy(const LinphoneAccountParams *params);

/**
 * Set the policy to use to pass through NATs/firewalls when using this account params.
 * If it is set to NULL, the default NAT policy from the core will be used instead.
 * @param params The #LinphoneAccountParams object @notnil
 * @param policy The #LinphoneNatPolicy object. @maybenil
 * @see linphone_core_set_nat_policy()
 */
LINPHONE_PUBLIC void linphone_account_params_set_nat_policy(LinphoneAccountParams *params, LinphoneNatPolicy *policy);

/**
 * Set the conference factory uri.
 * @param params The #LinphoneAccountParams object @notnil
 * @param uri The uri of the conference factory. @maybenil
 * @deprecated 16/08/2023 Use linphone_account_params_set_conference_factory_address() instead.
 */
LINPHONE_DEPRECATED LINPHONE_PUBLIC void
linphone_account_params_set_conference_factory_uri(LinphoneAccountParams *params, const char *uri);

/**
 * Set the conference factory uri.
 * @param params The #LinphoneAccountParams object @notnil
 * @param address The #LinphoneAddress to set. @maybenil
 */
LINPHONE_PUBLIC void linphone_account_params_set_conference_factory_address(LinphoneAccountParams *params,
                                                                            const LinphoneAddress *address);

/**
 * Set the audio video conference factory uri.
 * @param params The #LinphoneAccountParams object @notnil
 * @param address The #LinphoneAddress to set. @maybenil
 */
LINPHONE_PUBLIC void linphone_account_params_set_audio_video_conference_factory_address(LinphoneAccountParams *params,
                                                                                        const LinphoneAddress *address);

/**
 * Set the CCMP server address.
 * @param params The #LinphoneAccountParams object @notnil
 * @param url The URL of the CCMP server. @maybenil
 */
LINPHONE_PUBLIC void linphone_account_params_set_ccmp_server_url(LinphoneAccountParams *params, const char *url);

/**
 * If enabled, the proxy will be used as the only route.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param enable TRUE to enable, FALSE otherwise.
 * @deprecated 16/12/2021 Use linphone_account_params_enable_outbound_proxy() instead.
 */
LINPHONE_DEPRECATED LINPHONE_PUBLIC void
linphone_account_params_set_outbound_proxy_enabled(LinphoneAccountParams *params, bool_t enable);

/**
 * If enabled, the proxy will be used as the only route.
 * @warning This function will replace or remove any routes previously set with
 * linphone_account_params_set_routes_addresses() if any. For that reason
 * linphone_account_params_enable_outbound_proxy() shall not be used with
 * linphone_account_params_set_routes_addresses(). Application shall consider either using the restrictive notion of
 * outbound proxy (a single route identical to the registrar URI), either the notion of predefined route-set where
 * routes can be multiple and different from registrar URI.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param enable TRUE to enable, FALSE otherwise.
 */
LINPHONE_PUBLIC void linphone_account_params_enable_outbound_proxy(LinphoneAccountParams *params, bool_t enable);

/**
 * Tell if the proxy is used as the only route.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return enable TRUE if enabled, FALSE otherwise.
 * @deprecated 16/12/2021 Use linphone_account_params_outbound_proxy_enabled() instead.
 */
LINPHONE_DEPRECATED LINPHONE_PUBLIC bool_t
linphone_account_params_get_outbound_proxy_enabled(const LinphoneAccountParams *params);

/**
 * Tell if the proxy is used as the only route.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return enable TRUE if enabled, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_account_params_outbound_proxy_enabled(const LinphoneAccountParams *params);

/**
 * Get the conference factory uri.
 * @param params The #LinphoneAccountParams object @notnil
 * @return The uri of the conference factory. @maybenil
 */
LINPHONE_DEPRECATED LINPHONE_PUBLIC const char *
linphone_account_params_get_conference_factory_uri(const LinphoneAccountParams *params);

/**
 * Get the conference factory uri.
 * @param params The #LinphoneAccountParams object @notnil
 * @return The #LinphoneAddress of the conference factory. @maybenil
 */
LINPHONE_PUBLIC const LinphoneAddress *
linphone_account_params_get_conference_factory_address(const LinphoneAccountParams *params);

/**
 * Get the audio video conference factory uri.
 * @param params The #LinphoneAccountParams object @notnil
 * @return The #LinphoneAddress of the audio video conference factory. @maybenil
 */
LINPHONE_PUBLIC const LinphoneAddress *
linphone_account_params_get_audio_video_conference_factory_address(const LinphoneAccountParams *params);

/**
 * Get the CCMP user ID.
 * @param params The #LinphoneAccountParams object @notnil
 * @return The ID of the CCMP user. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_account_params_get_ccmp_user_id(const LinphoneAccountParams *params);

/**
 * Get the CCMP server address.
 * @param params The #LinphoneAccountParams object @notnil
 * @return The URL of the CCMP server. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_account_params_get_ccmp_server_url(const LinphoneAccountParams *params);

/**
 * Indicates whether to add to the contact parameters the push notification information. For IOS, it indicates for VOIP
 * push notification.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param allow TRUE to allow push notification information, FALSE otherwise.
 */
LINPHONE_PUBLIC void linphone_account_params_set_push_notification_allowed(LinphoneAccountParams *params, bool_t allow);

/**
 * Indicates whether to add to the contact parameters the push notification information. For IOS, it indicates for VOIP
 * push notification.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return TRUE if push notification informations should be added, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_account_params_get_push_notification_allowed(const LinphoneAccountParams *params);

/**
 * Indicates whether to add to the contact parameters the push notification information.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param allow TRUE to allow remote push notification information, FALSE otherwise.
 */
LINPHONE_PUBLIC void linphone_account_params_set_remote_push_notification_allowed(LinphoneAccountParams *params,
                                                                                  bool_t allow);

/**
 * Indicates whether to add to the contact parameters the push notification information.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return TRUE if remote push notification informations should be added, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t
linphone_account_params_get_remote_push_notification_allowed(const LinphoneAccountParams *params);

/**
 * Gets whether push notifications are available or not (Android & iOS only).
 * @param params The #LinphoneAccountParams object. @notnil
 * @return TRUE if push notifications are available, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_account_params_is_push_notification_available(const LinphoneAccountParams *params);

/**
 * Sets the push notification configuration.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param config The #LinphonePushNotificationConfig object. @notnil
 */
LINPHONE_PUBLIC void linphone_account_params_set_push_notification_config(LinphoneAccountParams *params,
                                                                          LinphonePushNotificationConfig *config);

/**
 * Returns the push notification configuration.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The #LinphonePushNotificationConfig object. @notnil
 */
LINPHONE_PUBLIC LinphonePushNotificationConfig *
linphone_account_params_get_push_notification_config(const LinphoneAccountParams *params);

/**
 * Sets whether the account will unREGISTER when the core stops but only if the push notifications are not allowed for
 * the account.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param unregister TRUE to unregister the account, FALSE otherwise.
 */
LINPHONE_PUBLIC void linphone_account_params_enable_unregister_at_stop(LinphoneAccountParams *params,
                                                                       bool_t unregister);

/**
 * Gets whether the account will unREGISTER when the core stops but only if the push notifications are not allowed for
 * the account.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return TRUE if the account will unREGISTER at stop, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_account_params_unregister_at_stop_enabled(const LinphoneAccountParams *params);

/**
 * Sets the transport type of the server address.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param transport The #LinphoneTransportType to set.
 */
LINPHONE_PUBLIC void linphone_account_params_set_transport(LinphoneAccountParams *params,
                                                           LinphoneTransportType transport);

/**
 * Returns the transport type of the server address.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The #LinphoneTransportType of the server address.
 */
LINPHONE_PUBLIC LinphoneTransportType linphone_account_params_get_transport(const LinphoneAccountParams *params);

/**
 * Enables or disables RTP bundle mode (Media Multiplexing).
 * See https://datatracker.ietf.org/doc/html/rfc8843 for more information about the feature.
 * When enabled, liblinphone will try to negociate the use of a single port for all streams when doing an outgoing call.
 * It automatically enables rtcp-mux.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param value a boolean to indicate whether the feature is to be enabled.
 *
 */
LINPHONE_PUBLIC void linphone_account_params_enable_rtp_bundle(LinphoneAccountParams *params, bool_t value);

/**
 * Returns whether RTP bundle mode (also known as Media Multiplexing) is enabled.
 * See https://datatracker.ietf.org/doc/html/rfc8843 for more information.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return a boolean indicating the enablement of rtp bundle mode.
 */
LINPHONE_PUBLIC bool_t linphone_account_params_rtp_bundle_enabled(const LinphoneAccountParams *params);

/**
 * Indicates whether support of rtp bundle is assumed.
 * See linphone_account_params_enable_rtp_bundle() for background information about rtp bundle.
 * Assumption that RTP bundling support allows interesting optimizations, such as
 * not gathering RTCP candidates, and not gathering candidates for video stream when making
 * an outgoing call.
 * This setting is meaningful only if rtp bundle is enabled.
 * See https://datatracker.ietf.org/doc/html/rfc8843 for more information about the feature.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param value a boolean to indicate whether RTP bundle support can be assumed.
 *
 */
LINPHONE_PUBLIC void linphone_account_params_enable_rtp_bundle_assumption(LinphoneAccountParams *params, bool_t value);

/**
 * Returns whether RTP bundle mode is assumed.
 * See https://datatracker.ietf.org/doc/html/rfc8843 for more information.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return a boolean indicating when rtp bundle support is assumed.
 */
LINPHONE_PUBLIC bool_t linphone_account_params_rtp_bundle_assumption_enabled(const LinphoneAccountParams *params);

/**
 * Indicates whether chat messages sent by this account in a #LinphoneChatRoomBackendBasic chat room will be using CPIM
 * format or not. By default SIP SIMPLE format is used for "basic" chat rooms, CPIM is only used for
 * #LinphoneChatRoomBackendFlexisipChat chat rooms.
 * @see
 * https://wiki.linphone.org/xwiki/wiki/public/view/Lib/Features/Instant%20Messaging/Reply%20to%20a%20specific%20message/
 * @param params The #LinphoneAccountParams object. @notnil
 * @return TRUE if chat messages will be sent out in CPIM format, FALSE if chat messages will be sent out as SIP SIMPLE.
 */
LINPHONE_PUBLIC bool_t linphone_account_params_cpim_in_basic_chat_room_enabled(const LinphoneAccountParams *params);

/**
 * Indicates whether chat messages sent by this account in a #LinphoneChatRoomBackendBasic chat room will be using CPIM
 * format or not.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param enable TRUE to send messages in CPIM format, FALSE to keep using the SIP SIMPLE format.
 */
LINPHONE_PUBLIC void linphone_account_params_enable_cpim_in_basic_chat_room(LinphoneAccountParams *params,
                                                                            bool_t enable);

/**
 * Set one custom parameter to this #LinphoneAccountParams.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param key key of the searched parameter. @notnil
 * @param value value of the searched parameter. @notnil
 */
LINPHONE_PUBLIC void
linphone_account_params_add_custom_param(LinphoneAccountParams *params, const char *key, const char *value);

/**
 * Get the custom parameter with key to this #LinphoneAccountParams.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param key key of the searched parameter. @notnil
 * @return The value of the parameter with key if found or an empty string otherwise. @notnil
 */
LINPHONE_PUBLIC const char *linphone_account_params_get_custom_param(const LinphoneAccountParams *params,
                                                                     const char *key);

/**
 * Set a an optional custom contact address to be used when registering to the SIP server.
 * This is typically used to supply an alternative SIP address to call in case the main one is not reachable.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param contact a #LinphoneAddress the optional alternative contact address. @maybenil
 */
LINPHONE_PUBLIC void linphone_account_params_set_custom_contact(LinphoneAccountParams *params,
                                                                LinphoneAddress *contact);

/**
 * Get the custom contact address previously used when registering to the SIP server.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return a #LinphoneAddress @maybenil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_account_params_get_custom_contact(const LinphoneAccountParams *params);

/**
 * Set the x3dh server url.
 * If empty, this function will disable LIME X3DH from core.
 * Otherwise, or if different from the existing value, this will (re-)initialize the LIME X3DH engine.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param url The x3dh server url. @maybenil
 **/
LINPHONE_PUBLIC void linphone_account_params_set_lime_server_url(LinphoneAccountParams *params, const char *url);

/**
 * Get the x3dh server url.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The x3dh server url. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_account_params_get_lime_server_url(const LinphoneAccountParams *params);

/**
 * Set the base(s) x3dh algorithm.
 * accept an ordered comma separated list (without space) of lime base algorithms
 * accepted values are a combinaison of : c25519, c448 and c25519k512
 * NULL is also valid, it will unset the value
 * @param params The #LinphoneAccountParams object. @notnil
 * @param algo The x3dh base algorithm. @maybenil
 **/
LINPHONE_PUBLIC void linphone_account_params_set_lime_algo(LinphoneAccountParams *params, const char *algo);

/**
 * Get the base x3dh algorithm.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The x3dh base algorithm. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_account_params_get_lime_algo(const LinphoneAccountParams *params);

/**
 * Sets an URI for the account picture.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param uri The account picture URI. @maybenil
 **/
LINPHONE_PUBLIC void linphone_account_params_set_picture_uri(LinphoneAccountParams *params, const char *uri);

/**
 * Gets the account picture URI if set, NULL otherwise.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The account picture URI. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_account_params_get_picture_uri(const LinphoneAccountParams *params);

/**
 * Sets the Message Waiting Indication server address.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param address The Message Waiting Indication server address. @maybenil
 */
LINPHONE_PUBLIC void linphone_account_params_set_mwi_server_address(LinphoneAccountParams *params,
                                                                    LinphoneAddress *address);

/**
 * Gets the Message Waiting Indication server address.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The Message Waiting Indication server address. @maybenil
 */
LINPHONE_PUBLIC const LinphoneAddress *
linphone_account_params_get_mwi_server_address(const LinphoneAccountParams *params);

/**
 * Sets the Voicemail address.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param address The Voicemail address. @maybenil
 */
LINPHONE_PUBLIC void linphone_account_params_set_voicemail_address(LinphoneAccountParams *params,
                                                                   LinphoneAddress *address);

/**
 * Gets the Voicemail address.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The Voicemail address. @maybenil
 */
LINPHONE_PUBLIC const LinphoneAddress *
linphone_account_params_get_voicemail_address(const LinphoneAccountParams *params);

/**
 * Checks if encryption is mandatory for instant messages or not.
 * @param params #LinphoneAccountParams object. @notnil
 * @return TRUE if encryption is mandatory; FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t
linphone_account_params_get_instant_messaging_encryption_mandatory(const LinphoneAccountParams *params);

/**
 * Defines whether the encryption of instant messages is mandatory. If it is, clear text messages will be denied.
 * @param params #LinphoneAccountParams object. @notnil
 * @param mandatory TRUE to set it mandatory; FALSE otherwise.
 */
LINPHONE_PUBLIC void linphone_account_params_set_instant_messaging_encryption_mandatory(LinphoneAccountParams *params,
                                                                                        bool_t mandatory);

/**
 * Gets the list of supported tags.
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The list of supported tags \bctbx_list{char *}. @maybenil
 **/
LINPHONE_PUBLIC const bctbx_list_t *
linphone_account_params_get_supported_tags_list(const LinphoneAccountParams *params);

/**
 * Sets the list of supported tags.
 * @param params The #LinphoneAccountParams object. @notnil
 * @param supported_tags The list of supported tags \bctbx_list{char *}. @maybenil
 */
LINPHONE_PUBLIC void linphone_account_params_set_supported_tags_list(LinphoneAccountParams *params,
                                                                     const bctbx_list_t *supported_tags);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_ACCOUNT_PARAMS_H */
