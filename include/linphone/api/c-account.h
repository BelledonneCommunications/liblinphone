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

#ifndef LINPHONE_ACCOUNT_H
#define LINPHONE_ACCOUNT_H

#include "linphone/api/c-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup account
 * @{
 */

/**
 * Create a new #LinphoneAccount.
 * @param lc The #LinphoneCore object. @maybenil
 * @param params The #LinphoneAccountParams object. @notnil
 * @return The newly created #LinphoneAccount object. @notnil
 */
LINPHONE_PUBLIC LinphoneAccount *linphone_account_new(LinphoneCore *lc, LinphoneAccountParams *params);

/**
 * Create a new #LinphoneAccount with a Proxy config backpointer.
 * This is only intended to be used while keeping a backward compatibility with proxy config.
 * @param lc The #LinphoneCore object. @maybenil
 * @param params The #LinphoneAccountParams object. @notnil
 * @param config The #LinphoneProxyConfig object. @maybenil
 * @return The newly created #LinphoneAccount object. @notnil
 */
LINPHONE_PUBLIC LinphoneAccount *
linphone_account_new_with_config(LinphoneCore *lc, LinphoneAccountParams *params, LinphoneProxyConfig *config);

/**
 * Instantiate a new account with values from source.
 * @param account The #LinphoneAccount object to be cloned. @notnil
 * @return The newly created #LinphoneAccount object. @notnil
 */
LINPHONE_PUBLIC LinphoneAccount *linphone_account_clone(const LinphoneAccount *account);

/**
 * Take a reference on a #LinphoneAccount.
 * @param account The #LinphoneAccount object. @notnil
 * @return the same #LinphoneAccount object. @notnil
 */
LINPHONE_PUBLIC LinphoneAccount *linphone_account_ref(LinphoneAccount *account);

/**
 * Release a #LinphoneAccount.
 * @param account The #LinphoneAccount object. @notnil
 */
LINPHONE_PUBLIC void linphone_account_unref(LinphoneAccount *account);

/**
 * Set the #LinphoneAccountParams used by this #LinphoneAccount.
 * @param account The #LinphoneAccount object. @notnil
 * @param params The #LinphoneAccountParams object. @notnil
 */
LINPHONE_PUBLIC int linphone_account_set_params(LinphoneAccount *account, LinphoneAccountParams *params);

/**
 * Get the #LinphoneAccountParams as read-only object.
 * To make changes, clone the returned object using linphone_account_params_clone() method,
 * make your changes on it and apply them using with linphone_account_set_params().
 * @param account The #LinphoneAccount object. @notnil
 * @return The #LinphoneAccountParams attached to this account. @notnil
 */
LINPHONE_PUBLIC const LinphoneAccountParams *linphone_account_get_params(const LinphoneAccount *account);

/**
 * Set one custom parameter to this #LinphoneAccount.
 * @param account The #LinphoneAccount object. @notnil
 * @param key key of the searched parameter. @notnil
 * @param value value of the searched parameter. @notnil
 */
LINPHONE_PUBLIC void linphone_account_add_custom_param(LinphoneAccount *account, const char *key, const char *value);

/**
 * Get the custom parameter with key to this #LinphoneAccount.
 * @param account The #LinphoneAccount object. @notnil
 * @param key key of the searched parameter. @notnil
 * @return The value of the parameter with key if found or an empty string otherwise. @notnil
 */
LINPHONE_PUBLIC const char *linphone_account_get_custom_param(const LinphoneAccount *account, const char *key);

/**
 * Assign a user pointer to the account.
 * @param account The #LinphoneAccount object. @notnil
 * @param user_data The user data. @maybenil
 */
LINPHONE_PUBLIC void linphone_account_set_user_data(LinphoneAccount *account, void *user_data);

/**
 * Retrieve the user pointer associated with the account.
 * @param account The #LinphoneAccount object. @notnil
 * @return The user data. @maybenil
 */
LINPHONE_PUBLIC void *linphone_account_get_user_data(LinphoneAccount *account);

/**
 * Set the value of a custom header sent to the server in REGISTERs request.
 * @param account The #LinphoneAccount object. @notnil
 * @param header_name The header name. @notnil
 * @param header_value The header value. @maybenil
 */
LINPHONE_PUBLIC void
linphone_account_set_custom_header(LinphoneAccount *account, const char *header_name, const char *header_value);

/**
 * Obtain the value of a header sent by the server in last answer to REGISTER.
 * @param account The #LinphoneAccount object. @notnil
 * @param header_name The header name for which to fetch corresponding value. @notnil
 * @return The value of the queried header. @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_account_get_custom_header(LinphoneAccount *account, const char *header_name);

/**
 * Mark this account as being dependent on the given one.
 * The dependency must refer to an account previously added to the core and which idkey property is defined.
 *
 * @see linphone_account_params_set_idkey()
 *
 * The account marked as dependent will wait for successful registration on its dependency before triggering its own.
 *
 * Once registered, both accounts will share the same contact address (the 'dependency' one).
 *
 * This mecanism must be enabled before the account is added to the core
 *
 * @param account The #LinphoneAccount object. @notnil
 * @param depends_on The #LinphoneAccount this one shall be depending on. @maybenil
 **/
LINPHONE_PUBLIC void linphone_account_set_dependency(LinphoneAccount *account, LinphoneAccount *depends_on);

/**
 * Get the dependency of a #LinphoneAccount.
 *
 * @param account The #LinphoneAccount object. @notnil
 * @return The account this one is dependent upon, or NULL if not marked dependent. @maybenil
 **/
LINPHONE_PUBLIC LinphoneAccount *linphone_account_get_dependency(LinphoneAccount *account);

/**
 * Get the #LinphoneCore object to which is associated the #LinphoneAccount.
 * @param account The #LinphoneAccount object. @notnil
 * @return The #LinphoneCore object to which is associated the #LinphoneAccount. @notnil
 **/
LINPHONE_PUBLIC LinphoneCore *linphone_account_get_core(LinphoneAccount *account);

/**
 * Get detailed information why registration failed when the account state is LinphoneRegistrationFailed.
 * @param account The #LinphoneAccount object. @notnil
 * @return The #LinphoneErrorInfo explaining why registration failed for this account. @notnil
 **/
LINPHONE_PUBLIC const LinphoneErrorInfo *linphone_account_get_error_info(LinphoneAccount *account);

/**
 * Return the contact address of the account.
 * @param account The #LinphoneAccount object. @notnil
 * @return a #LinphoneAddress correspong to the contact address of the account. @maybenil
 **/
LINPHONE_PUBLIC LinphoneAddress *linphone_account_get_contact_address(LinphoneAccount *account);

/**
 * Set the contact address for the account.
 * A client application should not use this function, as the Contact address is provided by the registrar
 * in the 200 Ok. This function is mainly intended for server applications.
 * @param account The #LinphoneAccount object. @notnil
 * @param addr a #LinphoneAddress to use as contact. @maybenil
 */
LINPHONE_PUBLIC void linphone_account_set_contact_address(LinphoneAccount *account, const LinphoneAddress *addr);

/**
 * Get the registration state of the given account.
 * @param account The #LinphoneAccount object. @notnil
 * @return The #LinphoneRegistrationState of the account.
 **/
LINPHONE_PUBLIC LinphoneRegistrationState linphone_account_get_state(const LinphoneAccount *account);

/**
 * Refresh a proxy registration.
 * This is useful if for example you resuming from suspend, thus IP address may have changed.
 * @param account #LinphoneAccount object. @notnil
 **/
LINPHONE_PUBLIC void linphone_account_refresh_register(LinphoneAccount *account);

/**
 * Prevent an account from refreshing its registration.
 * This is useful to let registrations to expire naturally (or) when the application wants to keep control on when
 * refreshes are sent.
 * However, linphone_core_set_network_reachable(lc,TRUE) will always request the accounts to refresh their
 *registrations. The refreshing operations can be resumed with linphone_account_refresh_register().
 * @param account The #LinphoneAccount object. @notnil
 **/
LINPHONE_PUBLIC void linphone_account_pause_register(LinphoneAccount *account);

/**
 * Get the reason why registration failed when the account state is LinphoneRegistrationFailed.
 * @param account #LinphoneAccount object. @notnil
 * @return The #LinphoneReason why registration failed for this account.
 **/
LINPHONE_PUBLIC LinphoneReason linphone_account_get_error(LinphoneAccount *account);

/**
 * Get the transport from either service route, route or addr.
 * @param account The #LinphoneAccount object. @notnil
 * @return The transport as a string (I.E udp, tcp, tls, dtls).
 * @deprecated 01/03/2021 Use Linphone_account_params_get_transport() instead.
 **/
LINPHONE_PUBLIC LinphoneTransportType linphone_account_get_transport(LinphoneAccount *account);

/**
 * Indicates whether AVPF/SAVPF is being used for calls using this account.
 * @param account The #LinphoneAccount object. @notnil
 * @return TRUE if AVPF/SAVPF is enabled, FALSE otherwise.
 * @deprecated 16/12/2021 Use linphone_account_avpf_enabled() instead.
 */
LINPHONE_PUBLIC bool_t linphone_account_is_avpf_enabled(LinphoneAccount *account);

/**
 * Indicates whether AVPF/SAVPF is being used for calls using this account.
 * @param account The #LinphoneAccount object. @notnil
 * @return TRUE if AVPF/SAVPF is enabled, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_account_avpf_enabled(LinphoneAccount *account);

/**
 * Find authentication info matching account, if any, similarly to linphone_core_find_auth_info.
 * @param account The #LinphoneAccount object. @notnil
 * @return a #LinphoneAuthInfo matching account criteria if possible, NULL if nothing can be found. @maybenil
 **/
LINPHONE_PUBLIC const LinphoneAuthInfo *linphone_account_find_auth_info(LinphoneAccount *account);

/**
 * Returns the unread chat message count for a given account.
 * @param account The #LinphoneAccount object. @notnil
 * @return The unread chat message count.
 **/
LINPHONE_PUBLIC int linphone_account_get_unread_chat_message_count(const LinphoneAccount *account);

/**
 * Returns the list of chat rooms for a given account.
 * @param account The #LinphoneAccount object. @notnil
 * @return The list of chat rooms \bctbx_list{LinphoneChatRoom}. @maybenil
 **/
LINPHONE_PUBLIC const bctbx_list_t *linphone_account_get_chat_rooms(const LinphoneAccount *account);

/**
 * Returns a filtered list of chat rooms for a given account.
 * @param account The #LinphoneAccount object. @notnil
 * @param filter the criteria a chat room must meet somehow (subject, participant address, friend's name). @notnil
 * @return The list of chat rooms \bctbx_list{LinphoneChatRoom}. @maybenil @tobefreed
 **/
LINPHONE_PUBLIC bctbx_list_t *linphone_account_filter_chat_rooms(const LinphoneAccount *account, const char *filter);

/**
 * Returns the missed calls count for a given account.
 * @param account The #LinphoneAccount object. @notnil
 * @return The missed calls count.
 **/
LINPHONE_PUBLIC int linphone_account_get_missed_calls_count(const LinphoneAccount *account);

/**
 * Re-sets the number of missed calls for this account to 0.
 * @param account The #LinphoneAccount object. @notnil
 **/
LINPHONE_PUBLIC void linphone_account_reset_missed_calls_count(LinphoneAccount *account);

/**
 * Returns the list of call logs for a given account.
 * This list must be freed after use.
 * @param account The #LinphoneAccount object. @notnil
 * @return The list of call logs \bctbx_list{LinphoneCallLog}. @tobefreed @maybenil
 **/
LINPHONE_PUBLIC bctbx_list_t *linphone_account_get_call_logs(const LinphoneAccount *account);

/**
 * Returns the list of call logs for a given account.
 * This list must be freed after use.
 * @param account The #LinphoneAccount object. @notnil
 * @param remote_address the #LinphoneAddress object to filter call logs. @notnil
 * @return The list of filtered call logs \bctbx_list{LinphoneCallLog}. @tobefreed @maybenil
 **/
LINPHONE_PUBLIC bctbx_list_t *linphone_account_get_call_logs_for_address(const LinphoneAccount *account,
                                                                         const LinphoneAddress *remote_address);

/**
 * Deletes all the call logs related to this account from the database.
 * @param account The #LinphoneAccount object. @notnil
 **/
LINPHONE_PUBLIC void linphone_account_clear_call_logs(const LinphoneAccount *account);

/**
 * Returns the list of conference information stored locally for a given account.
 * This list must be freed after use.
 * @param account The #LinphoneAccount object. @notnil
 * @return The list of call logs \bctbx_list{LinphoneConferenceInfo}. @maybenil
 * @warning this method also start the synchronization with the CCMP server, should it be defined in the #AccountParams.
 *The application may want to wait for the callback conference_information_updated to get a up-to-date list of
 *conferences
 **/
LINPHONE_PUBLIC bctbx_list_t *linphone_account_get_conference_information_list(const LinphoneAccount *account);

/**
 * Returns the list of conference information stored locally for a given account.
 * This list must be freed after use.
 * @param account The #LinphoneAccount object. @notnil
 * @param capabilities the list of conference capabilities that the conference information must has set
 *\bctbx_list{LinphoneStreamType}. @tobefreed @maybenil
 * @return The list of call logs \bctbx_list{LinphoneConferenceInfo}. @tobefreed @maybenil
 * @warning this method also start the synchronization with the CCMP server, should it be defined in the #AccountParams.
 *The application may want to wait for the callback conference_information_updated to get a up-to-date list of
 *conferences
 * @donotwrap
 **/
LINPHONE_PUBLIC bctbx_list_t *linphone_account_get_conference_information_list_2(const LinphoneAccount *account,
                                                                                 bctbx_list_t *capabilities);

/**
 * Detect if the given input is a phone number or not.
 * @param account The #LinphoneAccount object, unused yet but may contain useful data. Can be NULL. @maybenil
 * @param username The string to parse. @notnil
 * @return TRUE if input is a phone number, FALSE otherwise.
 **/
LINPHONE_PUBLIC bool_t linphone_account_is_phone_number(const LinphoneAccount *account, const char *username);

/**
 * Normalize a human readable phone number into a basic string. 888-444-222 becomes 888444222
 * or +33888444222 depending on the #LinphoneAccount object.
 * This function will always generate a normalized username if input is a phone number.
 * @param account The #LinphoneAccount object containing country code and/or escape symbol. If NULL passed, will use
 * default configuration. @maybenil
 * @param username The string to parse. @notnil
 * @return NULL if input is an invalid phone number, normalized phone number from username input otherwise. @maybenil
 * @tobefreed
 */
LINPHONE_PUBLIC char *linphone_account_normalize_phone_number(const LinphoneAccount *account, const char *username);

/**
 * Normalize a human readable sip uri into a fully qualified LinphoneAddress.
 * A sip address should look like DisplayName \<sip:username\@domain:port\> .
 * Basically this function performs the following tasks
 * - if a phone number is entered, prepend country prefix and eventually escape the '+' by 00 of the proxy config.
 * - if no domain part is supplied, append the domain name of the proxy config. Returns NULL if no proxy is provided at
 * this point.
 * - if no sip: is present, prepend it.
 *
 * The result is a syntactically correct SIP address.
 * @param account The #LinphoneAccount object containing country code, escape symbol and/or domain name. Can be NULL if
 * domain is already provided. @maybenil
 * @param username The string to parse. @notnil
 * @return NULL if invalid input, normalized sip address otherwise. @maybenil @tobefreed
 */
LINPHONE_PUBLIC LinphoneAddress *linphone_account_normalize_sip_uri(LinphoneAccount *account, const char *username);

/**
 * Add a listener in order to be notified of #LinphoneAccount events.
 * @param account The #LinphoneAccount object to monitor. @notnil
 * @param cbs A #LinphoneAccountCbs object holding the callbacks you need. @notnil
 */
LINPHONE_PUBLIC void linphone_account_add_callbacks(LinphoneAccount *account, LinphoneAccountCbs *cbs);

/**
 * Remove a listener from a #LinphoneAccount.
 * @param account The #LinphoneAccount object. @notnil
 * @param cbs #LinphoneAccountCbs object to remove. @notnil
 */
LINPHONE_PUBLIC void linphone_account_remove_callbacks(LinphoneAccount *account, LinphoneAccountCbs *cbs);

/**
 * Gets the current LinphoneAccountCbs.
 * This is meant only to be called from a callback to be able to get the user_data associated with the
 * #LinphoneAccountCbs that is calling the callback.
 * @param account The #LinphoneAccount object. @notnil
 * @return The #LinphoneAccountCbs that has called the last callback. @maybenil
 */
LINPHONE_PUBLIC LinphoneAccountCbs *linphone_account_get_current_callbacks(const LinphoneAccount *account);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_ACCOUNT_H */
