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

#ifndef LINPHONE_PROXY_CONFIG_H
#define LINPHONE_PROXY_CONFIG_H

#include "linphone/types.h"
#include "linphone/sipsetup.h"
#include "linphone/api/c-push-notification-config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup proxies
 * @{
**/

/**
 * Acquire a reference to the proxy config.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return The same proxy config. @notnil
**/
LINPHONE_PUBLIC LinphoneProxyConfig *linphone_proxy_config_ref(LinphoneProxyConfig *proxy_config);

/**
 * Release reference to the proxy config.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
**/
LINPHONE_PUBLIC void linphone_proxy_config_unref(LinphoneProxyConfig *proxy_config);

/**
 * Retrieve the user pointer associated with the proxy config.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return The user pointer associated with the proxy config. @maybenil
**/
LINPHONE_PUBLIC void *linphone_proxy_config_get_user_data(const LinphoneProxyConfig *proxy_config);

/**
 * Assign a user pointer to the proxy config.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param user_data The user pointer to associate with the proxy config. @maybenil
**/
LINPHONE_PUBLIC void linphone_proxy_config_set_user_data(LinphoneProxyConfig *proxy_config, void *user_data);

/**
 * Sets the proxy address
 *
 * Examples of valid sip proxy address are:
 * - IP address: sip:87.98.157.38
 * - IP address with port: sip:87.98.157.38:5062
 * - hostnames : sip:sip.example.net
 * 
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param server_address the proxy address to set. @maybenil
**/
LINPHONE_PUBLIC LinphoneStatus linphone_proxy_config_set_server_addr(LinphoneProxyConfig *proxy_config, const char *server_address);

/**
 * Sets the user identity as a SIP address.
 *
 * This identity is normally formed with display name, username and domain, such
 * as: Alice <sip:alice@example.net>
 * The REGISTER messages will have from and to set to this identity.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param identity the #LinphoneAddress of the identity to set @maybenil
**/
LINPHONE_PUBLIC LinphoneStatus linphone_proxy_config_set_identity_address(LinphoneProxyConfig *proxy_config, const LinphoneAddress *identity);

/**
 * Sets a list of SIP route.
 * When a route is set, all outgoing calls will go to the route's destination if this proxy
 * is the default one (see linphone_core_set_default_proxy_config() ).
 * @param proxy_config the #LinphoneProxyConfig @notnil
 * @param routes A \bctbx_list{const char *} of routes @maybenil
 * @return -1 if routes are invalid, 0 otherwise.
**/
LINPHONE_PUBLIC LinphoneStatus linphone_proxy_config_set_routes(LinphoneProxyConfig *proxy_config, const bctbx_list_t *routes);

/**
 * Sets the registration expiration time in seconds.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param expires the expiration time to set
**/
LINPHONE_PUBLIC void linphone_proxy_config_set_expires(LinphoneProxyConfig *proxy_config, int expires);

#define linphone_proxy_config_expires linphone_proxy_config_set_expires

/**
 * @brief Indicates  either or not, REGISTRATION must be issued for this #LinphoneProxyConfig.
 *
 * In case this #LinphoneProxyConfig has been added to #LinphoneCore, follows the linphone_proxy_config_edit() rule.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param enable if true, registration will be engaged
 */
LINPHONE_PUBLIC void linphone_proxy_config_enable_register(LinphoneProxyConfig *proxy_config, bool_t enable);

#define linphone_proxy_config_enableregister linphone_proxy_config_enable_register

/**
 * Starts editing a proxy configuration.
 *
 * Because proxy configuration must be consistent, applications MUST
 * call linphone_proxy_config_edit() before doing any attempts to modify
 * proxy configuration (such as identity, proxy address and so on).
 * Once the modifications are done, then the application must call
 * linphone_proxy_config_done() to commit the changes.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
**/
LINPHONE_PUBLIC void linphone_proxy_config_edit(LinphoneProxyConfig *proxy_config);

/**
 * Commits modification made to the proxy configuration.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return 0 if successful, -1 otherwise
**/
LINPHONE_PUBLIC LinphoneStatus linphone_proxy_config_done(LinphoneProxyConfig *proxy_config);

/**
 * @brief Indicates either or not, PUBLISH must be issued for this #LinphoneProxyConfig.
 *
 * In case this #LinphoneProxyConfig has been added to #LinphoneCore, follows the #linphone_proxy_config_edit() rule.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param enable if TRUE, publish will be engaged
 */
LINPHONE_PUBLIC void linphone_proxy_config_enable_publish(LinphoneProxyConfig *proxy_config, bool_t enable);

/**
 * Set the publish expiration time in second.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param expires in second
 * */
LINPHONE_PUBLIC void linphone_proxy_config_set_publish_expires(LinphoneProxyConfig *proxy_config, int expires);

/**
 * get the publish expiration time in second. Default value is the registration expiration value.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return expires in second
 * */
LINPHONE_PUBLIC int linphone_proxy_config_get_publish_expires(const LinphoneProxyConfig *proxy_config);

/**
 * Sets whether liblinphone should replace "+" by international calling prefix in dialed numbers (passed to
 * #linphone_core_invite() ).
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param enable TRUE to replace + by the international prefix, FALSE otherwise.
**/
LINPHONE_PUBLIC void linphone_proxy_config_set_dial_escape_plus(LinphoneProxyConfig *proxy_config, bool_t enable);

/**
 * Sets a dialing prefix to be automatically prepended when inviting a number with
 * linphone_core_invite();
 * This dialing prefix shall usually be the country code of the country where the user is living, without "+".
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param prefix the prefix to set (withouth the +) @maybenil
**/
LINPHONE_PUBLIC void linphone_proxy_config_set_dial_prefix(LinphoneProxyConfig *proxy_config, const char *prefix);

 /**
 * Indicates whether quality statistics during call should be stored and sent to a collector according to RFC 6035.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param enable True to store quality statistics and send them to the collector, false to disable it.
 */
LINPHONE_PUBLIC void linphone_proxy_config_enable_quality_reporting(LinphoneProxyConfig *proxy_config, bool_t enable);

/**
 * Indicates whether quality statistics during call should be stored and sent to a collector according to RFC 6035.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return True if quality repotring is enabled, false otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_proxy_config_quality_reporting_enabled(LinphoneProxyConfig *proxy_config);

 /**
 * Set the route of the collector end-point when using quality reporting. This SIP address
 * should be used on server-side to process packets directly before discarding packets. Collector address
 * should be a non existing account and will not receive any messages.
 * If NULL, reports will be send to the proxy domain.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param collector route of the collector end-point, if NULL PUBLISH will be sent to the proxy domain. @maybenil
 */
LINPHONE_PUBLIC void linphone_proxy_config_set_quality_reporting_collector(LinphoneProxyConfig *proxy_config, const char *collector);

 /**
 * Get the route of the collector end-point when using quality reporting. This SIP address
 * should be used on server-side to process packets directly before discarding packets. Collector address
 * should be a non existing account and will not receive any messages.
 * If NULL, reports will be send to the proxy domain.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return The SIP address of the collector end-point. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_proxy_config_get_quality_reporting_collector(const LinphoneProxyConfig *proxy_config);

/**
 * Set the interval between 2 interval reports sending when using quality reporting. If call exceed interval size, an
 * interval report will be sent to the collector. On call termination, a session report will be sent
 * for the remaining period. Value must be 0 (disabled) or positive.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param interval The interval in seconds, 0 means interval reports are disabled.
 */
LINPHONE_PUBLIC void linphone_proxy_config_set_quality_reporting_interval(LinphoneProxyConfig *proxy_config, int interval);

/**
 * Get the interval between interval reports when using quality reporting.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return The interval in seconds, 0 means interval reports are disabled.
 */
LINPHONE_PUBLIC int linphone_proxy_config_get_quality_reporting_interval(LinphoneProxyConfig *proxy_config);

/**
 * Get the registration state of the given proxy config.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return The #LinphoneRegistrationState of the proxy config.
**/
LINPHONE_PUBLIC LinphoneRegistrationState linphone_proxy_config_get_state(const LinphoneProxyConfig *proxy_config);

/**
 * Get the domain name of the given proxy config.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return The domain name of the proxy config. @maybenil
**/
LINPHONE_PUBLIC const char *linphone_proxy_config_get_domain(const LinphoneProxyConfig *proxy_config);

/**
 * Get the realm of the given proxy config.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return The realm of the proxy config. @maybenil
**/
LINPHONE_PUBLIC const char *linphone_proxy_config_get_realm(const LinphoneProxyConfig *proxy_config);

/**
 * Set the realm of the given proxy config.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param realm New realm value. @maybenil
**/
LINPHONE_PUBLIC void linphone_proxy_config_set_realm(LinphoneProxyConfig *proxy_config, const char * realm);

/**
 * Gets the list of the routes set for this proxy config.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return The list of routes as string. \bctbx_list{const char *} @maybenil
 */
LINPHONE_PUBLIC const bctbx_list_t* linphone_proxy_config_get_routes(const LinphoneProxyConfig *proxy_config);

/**
 * Gets the identity addres of the proxy config.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return the SIP identity that belongs to this proxy configuration. @maybenil
**/
LINPHONE_PUBLIC const LinphoneAddress *linphone_proxy_config_get_identity_address(const LinphoneProxyConfig *proxy_config);

/**
 * Gets if the PUBLISH is enabled.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return TRUE if PUBLISH request is enabled for this proxy.
**/
LINPHONE_PUBLIC bool_t linphone_proxy_config_publish_enabled(const LinphoneProxyConfig *proxy_config);

/**
 * Gets the proxy config proxy address.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return the proxy's SIP address. @maybenil
**/
LINPHONE_PUBLIC	const char *linphone_proxy_config_get_server_addr(const LinphoneProxyConfig *proxy_config);

#define linphone_proxy_config_get_addr linphone_proxy_config_get_server_addr

/**
 * Gets the proxy config expires
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return the duration of registration.
**/
LINPHONE_PUBLIC int linphone_proxy_config_get_expires(const LinphoneProxyConfig *proxy_config);

/**
 * Returns whether the proxy config is enabled or not.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return TRUE if registration to the proxy is enabled.
**/
LINPHONE_PUBLIC bool_t linphone_proxy_config_register_enabled(const LinphoneProxyConfig *proxy_config);

/**
 * Refresh a proxy registration.
 * This is useful if for example you resuming from suspend, thus IP address may have changed.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
**/
LINPHONE_PUBLIC void linphone_proxy_config_refresh_register(LinphoneProxyConfig *proxy_config);

/**
 * Prevent a proxy config from refreshing its registration.
 * This is useful to let registrations to expire naturally (or) when the application wants to keep control on when
 * refreshes are sent.
 * However, linphone_core_set_network_reachable(lc,TRUE) will always request the proxy configs to refresh their registrations.
 * The refreshing operations can be resumed with linphone_proxy_config_refresh_register().
 * @param proxy_config #LinphoneProxyConfig object. @notnil
**/
LINPHONE_PUBLIC void linphone_proxy_config_pause_register(LinphoneProxyConfig *proxy_config);

/**
 * Return the contact address of the proxy config.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return a #LinphoneAddress correspong to the contact address of the proxy config. @maybenil
**/
LINPHONE_PUBLIC const LinphoneAddress* linphone_proxy_config_get_contact(const LinphoneProxyConfig *proxy_config);

/**
 * Returns the contact parameters.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return previously set contact parameters. @maybenil
**/
LINPHONE_PUBLIC const char *linphone_proxy_config_get_contact_parameters(const LinphoneProxyConfig *proxy_config);

/**
 * Set optional contact parameters that will be added to the contact information sent in the registration.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param contact_params a string contaning the additional parameters in text form, like "myparam=something;myparam2=something_else" @maybenil
 *
 * The main use case for this function is provide the proxy additional information regarding the user agent, like for example unique identifier or apple push id.
 * As an example, the contact address in the SIP register sent will look like <sip:joe@15.128.128.93:50421>;apple-push-id=43143-DFE23F-2323-FA2232.
**/
LINPHONE_PUBLIC void linphone_proxy_config_set_contact_parameters(LinphoneProxyConfig *proxy_config, const char *contact_params);

/**
 * Set optional contact parameters that will be added to the contact information sent in the registration, inside the URI.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param contact_uri_params a string containing the additional parameters in text form, like "myparam=something;myparam2=something_else" @maybenil
 *
 * The main use case for this function is provide the proxy additional information regarding the user agent, like for example unique identifier or apple push id.
 * As an example, the contact address in the SIP register sent will look like <sip:joe@15.128.128.93:50421;apple-push-id=43143-DFE23F-2323-FA2232>.
**/
LINPHONE_PUBLIC void linphone_proxy_config_set_contact_uri_parameters(LinphoneProxyConfig *proxy_config, const char *contact_uri_params);

/**
 * Returns the contact URI parameters.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return previously set contact URI parameters. @maybenil
**/
LINPHONE_PUBLIC const char* linphone_proxy_config_get_contact_uri_parameters(const LinphoneProxyConfig *proxy_config);

/**
 * Get the #LinphoneCore object to which is associated the #LinphoneProxyConfig.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return The #LinphoneCore object to which is associated the #LinphoneProxyConfig. @notnil
**/
LINPHONE_PUBLIC LinphoneCore * linphone_proxy_config_get_core(const LinphoneProxyConfig *proxy_config);

/**
 * Returns whether or not the + should be replaced by 00.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return whether liblinphone should replace "+" by "00" in dialed numbers (passed to #linphone_core_invite()).
**/
LINPHONE_PUBLIC bool_t linphone_proxy_config_get_dial_escape_plus(const LinphoneProxyConfig *proxy_config);

/**
 * Gets the prefix set for this proxy config.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return dialing prefix. @maybenil
**/
LINPHONE_PUBLIC	const char * linphone_proxy_config_get_dial_prefix(const LinphoneProxyConfig *proxy_config);

/**
 * Get the reason why registration failed when the proxy config state is LinphoneRegistrationFailed.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return The #LinphoneReason why registration failed for this proxy config.
**/
LINPHONE_PUBLIC LinphoneReason linphone_proxy_config_get_error(const LinphoneProxyConfig *proxy_config);

/**
 * Get detailed information why registration failed when the proxy config state is LinphoneRegistrationFailed.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return The #LinphoneErrorInfo explaining why registration failed for this proxy config. @notnil
**/
LINPHONE_PUBLIC const LinphoneErrorInfo *linphone_proxy_config_get_error_info(const LinphoneProxyConfig *proxy_config);

/**
 * Get the transport from either service route, route or addr.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return The transport as a string (I.E udp, tcp, tls, dtls) @notnil
**/
LINPHONE_PUBLIC const char* linphone_proxy_config_get_transport(const LinphoneProxyConfig *proxy_config);

LINPHONE_PUBLIC void linphone_proxy_config_set_sip_setup(LinphoneProxyConfig *proxy_config, const char *type);

LINPHONE_PUBLIC SipSetupContext *linphone_proxy_config_get_sip_setup_context(LinphoneProxyConfig *proxy_config);

LINPHONE_PUBLIC SipSetup *linphone_proxy_config_get_sip_setup(LinphoneProxyConfig *proxy_config);

/**
 * Normalize a human readable phone number into a basic string. 888-444-222 becomes 888444222
 * or +33888444222 depending on the #LinphoneProxyConfig object.
 * This function will always generate a normalized username if input is a phone number.
 * @param proxy_config #LinphoneProxyConfig object containing country code and/or escape symbol. If NULL passed, will use default configuration. @maybenil
 * @param username the string to parse @notnil
 * @return NULL if input is an invalid phone number, normalized phone number from username input otherwise. @maybenil
*/
LINPHONE_PUBLIC char* linphone_proxy_config_normalize_phone_number(LinphoneProxyConfig *proxy_config, const char *username);

/**
 * Normalize a human readable sip uri into a fully qualified LinphoneAddress.
 * A sip address should look like DisplayName \<sip:username\@domain:port\> .
 * Basically this function performs the following tasks
 * - if a phone number is entered, prepend country prefix and eventually escape the '+' by 00 of the proxy config.
 * - if no domain part is supplied, append the domain name of the proxy config. Returns NULL if no proxy is provided at this point.
 * - if no sip: is present, prepend it.
 *
 * The result is a syntactically correct SIP address.
 * @param proxy_config #LinphoneProxyConfig object containing country code, escape symbol and/or domain name. Can be NULL if domain is already provided. @maybenil
 * @param username the string to parse @notnil
 * @return NULL if invalid input, normalized sip address otherwise. @maybenil
*/
LINPHONE_PUBLIC LinphoneAddress* linphone_proxy_config_normalize_sip_uri(LinphoneProxyConfig *proxy_config, const char *username);

/**
 * Set default privacy policy for all calls routed through this proxy.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param privacy #LinphonePrivacy to configure privacy
 * */
LINPHONE_PUBLIC void linphone_proxy_config_set_privacy(LinphoneProxyConfig *proxy_config, LinphonePrivacyMask privacy);

/**
 * Get default privacy policy for all calls routed through this proxy.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return Privacy mode as #LinphonePrivacyMask
 * */
LINPHONE_PUBLIC LinphonePrivacyMask linphone_proxy_config_get_privacy(const LinphoneProxyConfig *proxy_config);

/**
 * Set the http file transfer server to be used for content type application/vnd.gsma.rcs-ft-http+xml
 * Url may be like: "https://file.linphone.org/upload.php".
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param server_url URL of the file server. @maybenil
 * @warning That function isn't implemented yet.
 * @donotwrap
 * */
LINPHONE_PUBLIC void linphone_proxy_config_set_file_transfer_server(LinphoneProxyConfig *proxy_config, const char * server_url);

/**
 * Get the http file transfer server to be used for content type application/vnd.gsma.rcs-ft-http+xml
 * Url may be like: "https://file.linphone.org/upload.php".
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return URL of the file server. @maybenil
 * @warning That function isn't implemented yet.
 * @donotwrap
 * */
LINPHONE_PUBLIC const char* linphone_proxy_config_get_file_transfer_server(const LinphoneProxyConfig *proxy_config);

/**
 * Indicates whether AVPF/SAVPF is being used for calls using this proxy config.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return True if AVPF/SAVPF is enabled, false otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_proxy_config_avpf_enabled(LinphoneProxyConfig *proxy_config);

/**
 * Set the interval between regular RTCP reports when using AVPF/SAVPF.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param interval The interval in seconds (between 0 and 5 seconds).
 */
LINPHONE_PUBLIC void linphone_proxy_config_set_avpf_rr_interval(LinphoneProxyConfig *proxy_config, uint8_t interval);

/**
 * Get the interval between regular RTCP reports when using AVPF/SAVPF.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return The interval in seconds.
 */
LINPHONE_PUBLIC uint8_t linphone_proxy_config_get_avpf_rr_interval(const LinphoneProxyConfig *proxy_config);

/**
 * Get enablement status of RTCP feedback (also known as AVPF profile).
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return the enablement mode, which can be #LinphoneAVPFDefault (use LinphoneCore's mode), #LinphoneAVPFEnabled (avpf is enabled), or #LinphoneAVPFDisabled (disabled).
**/
LINPHONE_PUBLIC LinphoneAVPFMode linphone_proxy_config_get_avpf_mode(const LinphoneProxyConfig *proxy_config);

/**
 * Enable the use of RTCP feedback (also known as AVPF profile).
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param mode the enablement mode, which can be #LinphoneAVPFDefault (use LinphoneCore's mode), #LinphoneAVPFEnabled (avpf is enabled), or #LinphoneAVPFDisabled (disabled).
**/
LINPHONE_PUBLIC void linphone_proxy_config_set_avpf_mode(LinphoneProxyConfig *proxy_config, LinphoneAVPFMode mode);

/**
 * Obtain the value of a header sent by the server in last answer to REGISTER.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param header_name the header name for which to fetch corresponding value @notnil
 * @return the value of the queried header. @maybenil
**/
LINPHONE_PUBLIC const char *linphone_proxy_config_get_custom_header(LinphoneProxyConfig *proxy_config, const char *header_name);

/**
 * Set the value of a custom header sent to the server in REGISTERs request.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param header_name the header name @notnil
 * @param header_value the header's value @maybenil
**/
LINPHONE_PUBLIC void linphone_proxy_config_set_custom_header(LinphoneProxyConfig *proxy_config, const char *header_name, const char *header_value);

/**
 * Find authentication info matching proxy config, if any, similarly to linphone_core_find_auth_info.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return a #LinphoneAuthInfo matching proxy config criteria if possible, NULL if nothing can be found. @maybenil
**/
LINPHONE_PUBLIC const LinphoneAuthInfo* linphone_proxy_config_find_auth_info(const LinphoneProxyConfig *proxy_config);

/**
 * Get the persistent reference key associated to the proxy config.
 *
 * The reference key can be for example an id to an external database.
 * It is stored in the config file, thus can survive to process exits/restarts.
 *
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return The reference key string that has been associated to the proxy config, or NULL if none has been associated. @maybenil
**/
LINPHONE_PUBLIC const char * linphone_proxy_config_get_ref_key(const LinphoneProxyConfig *proxy_config);

/**
 * Associate a persistent reference key to the proxy config.
 *
 * The reference key can be for example an id to an external database.
 * It is stored in the config file, thus can survive to process exits/restarts.
 *
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param refkey The reference key string to associate to the proxy config. @maybenil
**/
LINPHONE_PUBLIC void linphone_proxy_config_set_ref_key(LinphoneProxyConfig *proxy_config, const char *refkey);

/**
 * Get the dependency of a #LinphoneProxyConfig.
 *
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return The proxy config this one is dependent upon, or NULL if not marked dependent. @maybenil
 **/
LINPHONE_PUBLIC LinphoneProxyConfig *linphone_proxy_config_get_dependency(LinphoneProxyConfig *proxy_config);

/**
 * Mark	this proxy configuration as being dependent on the given one.
 * The dependency must refer to a proxy config previously added to the core and which idkey property is defined.
 *
 * @see linphone_proxy_config_set_idkey()
 *
 * The proxy configuration marked as dependent will wait for successful registration on its dependency before triggering its own.
 *
 * Once registered, both proxy configurations will share the same contact address (the 'dependency' one).
 *
 * This mecanism must be enabled before the proxy configuration is added to the core
 *
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param depends_on The #LinphoneProxyConfig this one shall be depend on. @maybenil
 **/
LINPHONE_PUBLIC void linphone_proxy_config_set_dependency(LinphoneProxyConfig *proxy_config, LinphoneProxyConfig *depends_on);

/**
 * Get the idkey property of a #LinphoneProxyConfig.
 *
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return The idkey string, or NULL. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_proxy_config_get_idkey(LinphoneProxyConfig *proxy_config);

/**
 * Set the idkey property on the given proxy configuration.
 * This property can the be referenced by another proxy config 'depends_on' to create a dependency relation between them.
 * @see linphone_proxy_config_set_depends_on()
 *
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param idkey The idkey string to associate to the given #LinphoneProxyConfig. @maybenil
 **/
LINPHONE_PUBLIC void linphone_proxy_config_set_idkey(LinphoneProxyConfig *proxy_config, const char *idkey);

/**
 * Get The policy that is used to pass through NATs/firewalls when using this proxy config.
 * If it is set to NULL, the default NAT policy from the core will be used instead.
 * @param proxy_config #LinphoneProxyConfig object @notnil
 * @return #LinphoneNatPolicy object in use. @maybenil
 * @see linphone_core_get_nat_policy()
 */
LINPHONE_PUBLIC LinphoneNatPolicy * linphone_proxy_config_get_nat_policy(const LinphoneProxyConfig *proxy_config);

/**
 * Set the policy to use to pass through NATs/firewalls when using this proxy config.
 * If it is set to NULL, the default NAT policy from the core will be used instead.
 * @param proxy_config #LinphoneProxyConfig object @notnil
 * @param policy #LinphoneNatPolicy object. @maybenil
 * @see linphone_core_set_nat_policy()
 */
LINPHONE_PUBLIC void linphone_proxy_config_set_nat_policy(LinphoneProxyConfig *proxy_config, LinphoneNatPolicy *policy);

/**
 * Set the conference factory uri.
 * @param proxy_config A #LinphoneProxyConfig object @notnil
 * @param uri The uri of the conference factory. @maybenil
 */
LINPHONE_PUBLIC void linphone_proxy_config_set_conference_factory_uri(LinphoneProxyConfig *proxy_config, const char *uri);

/**
 * Get the conference factory uri.
 * @param proxy_config A #LinphoneProxyConfig object @notnil
 * @return The uri of the conference factory. @maybenil
 */
LINPHONE_PUBLIC const char * linphone_proxy_config_get_conference_factory_uri(const LinphoneProxyConfig *proxy_config);

/**
 * Indicates whether to add to the contact parameters the push notification information. For IOS, it indicates to VOIP push notification.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param allow TRUE to allow push notification information, FALSE otherwise.
 */
LINPHONE_PUBLIC void linphone_proxy_config_set_push_notification_allowed(LinphoneProxyConfig *proxy_config, bool_t allow);

/**
 * Indicates whether to add to the contact parameters the push notification information. For IOS, it indicates to VOIP push notification.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return TRUE if push notification informations should be added, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_proxy_config_is_push_notification_allowed(const LinphoneProxyConfig *proxy_config);

/**
* Indicates whether to add to the contact parameters the remote push notification information (IOS only). Default value is FALSE.
* @param proxy_config #LinphoneProxyConfig object. @notnil
* @return TRUE if remote push notification informations should be added, FALSE otherwise.
*/
LINPHONE_PUBLIC bool_t linphone_proxy_config_is_remote_push_notification_allowed(const LinphoneProxyConfig *proxy_config);

/**
* Indicates whether to add to the contact parameters the  remote push notification information (IOS only).
* @param proxy_config #LinphoneProxyConfig object. @notnil
* @param allow TRUE to allow remote push notification information, FALSE otherwise.
*/
LINPHONE_PUBLIC void linphone_proxy_config_set_remote_push_notification_allowed(LinphoneProxyConfig *proxy_config, bool_t allow);

/**
* Gets whether push notifications are available or not (Android & iOS only).
* @param proxy_config #LinphoneProxyConfig object. @notnil
* @return TRUE if push notifications are available, FALSE otherwise
*/
LINPHONE_PUBLIC bool_t linphone_proxy_config_is_push_notification_available(const LinphoneProxyConfig *proxy_config);

/**
 * Return the unread chat message count for a given proxy config.
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @return The unread chat message count.
 */
LINPHONE_PUBLIC int linphone_proxy_config_get_unread_chat_message_count (const LinphoneProxyConfig *proxy_config);

/**
 * Sets the push notification configuration.
 * @param cfg #LinphoneProxyConfig object. @notnil
 * @param push_cfg #LinphonePushNotificationConfig to set. @notnil
 */
LINPHONE_PUBLIC void linphone_proxy_config_set_push_notification_config(LinphoneProxyConfig *cfg, LinphonePushNotificationConfig *push_cfg);

/**
 * Retrieves the push notification configuration.
 * @param cfg #LinphoneProxyConfig object. @notnil
 * @return The #LinphonePushNotificationConfig. @notnil
 */
LINPHONE_PUBLIC LinphonePushNotificationConfig *linphone_proxy_config_get_push_notification_config(const LinphoneProxyConfig *cfg);

/************ */
/* DEPRECATED */
/* ********** */

/**
 * Sets a SIP route.
 * When a route is set, all outgoing calls will go to the route's destination if this proxy
 * is the default one (see linphone_core_set_default_proxy_config() ).
 * @param proxy_config #LinphoneProxyConfig object. @notnil
 * @param route the SIP route to set @maybenil
 * @return -1 if route is invalid, 0 otherwise.
 * @deprecated 08/07/2020 use linphone_proxy_config_set_routes() instead
**/
LINPHONE_PUBLIC LINPHONE_DEPRECATED LinphoneStatus linphone_proxy_config_set_route(LinphoneProxyConfig *proxy_config, const char *route);

/**
 * @return the route set for this proxy configuration.
 * @deprecated 09/03/2018 Use linphone_proxy_config_get_routes() instead.
 * @donotwrap
**/
LINPHONE_PUBLIC LINPHONE_DEPRECATED const char *linphone_proxy_config_get_route(const LinphoneProxyConfig *proxy_config);

/**
 * @deprecated 29/07/2015 use linphone_proxy_config_get_identity_address()
 * @donotwrap
**/
LINPHONE_PUBLIC LINPHONE_DEPRECATED const char *linphone_proxy_config_get_identity(const LinphoneProxyConfig *cfg);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
