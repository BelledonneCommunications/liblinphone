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

#ifndef LINPHONE_ACCOUNT_CREATOR_H_
#define LINPHONE_ACCOUNT_CREATOR_H_

#include "linphone/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Utils
 */
const char *_get_domain(LinphoneAccountCreator *creator);
char* _get_identity(const LinphoneAccountCreator *creator);
unsigned int validate_uri(const char* username, const char* domain, const char* display_name);
void reset_field (char **field);
void fill_domain_and_algorithm_if_needed(LinphoneAccountCreator *creator);
char *generate_random_password(void);

/**
 * @addtogroup account_creator
 * @{
 */

/**
 * Callback to notify a response of server.
 * @param creator #LinphoneAccountCreator object @notnil
 * @param status The status of the #LinphoneAccountCreator test existence operation that has just finished
 * @param response The response has a string @maybenil
**/
typedef void (*LinphoneAccountCreatorCbsStatusCb)(LinphoneAccountCreator *creator, LinphoneAccountCreatorStatus status, const char* response);

/************************** Start Account Creator data **************************/

/**
 * Create a #LinphoneAccountCreator and set Linphone Request callbacks.
 * @param core The #LinphoneCore used for the XML-RPC communication @notnil
 * @return The new #LinphoneAccountCreator object. @notnil
**/
LINPHONE_PUBLIC LinphoneAccountCreator* linphone_account_creator_create(LinphoneCore *core);

/**
 * Create a #LinphoneAccountCreator and set Linphone Request callbacks.
 * @param core The #LinphoneCore used for the XML-RPC communication @notnil
 * @param xmlrpc_url The URL to the XML-RPC server. @maybenil
 * @return The new #LinphoneAccountCreator object. @notnil
 * @deprecated 13/04/2020 Use linphone_account_creator_create() instead
**/
LINPHONE_PUBLIC LinphoneAccountCreator* linphone_account_creator_new(LinphoneCore *core, const char *xmlrpc_url);

/**
 * Reset the account creator entries like username, password, phone number...
 * @param creator #LinphoneAccountCreator object @notnil
**/
LINPHONE_PUBLIC void linphone_account_creator_reset(LinphoneAccountCreator *creator);

/**
 * Send a request to know the existence of account on server.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return #LinphoneAccountCreatorStatusRequestOk if the request has been sent, #LinphoneAccountCreatorStatusRequestFailed otherwise
**/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus linphone_account_creator_is_account_exist(LinphoneAccountCreator *creator);

/**
 * Send a request to create an account on server.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return #LinphoneAccountCreatorStatusRequestOk if the request has been sent, #LinphoneAccountCreatorStatusRequestFailed otherwise
**/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus linphone_account_creator_create_account(LinphoneAccountCreator *creator);

/**
 * Send a request to know if an account is activated on server.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return #LinphoneAccountCreatorStatusRequestOk if the request has been sent, #LinphoneAccountCreatorStatusRequestFailed otherwise
**/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus linphone_account_creator_is_account_activated(LinphoneAccountCreator *creator);

/**
 * Send a request to activate an account on server.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return #LinphoneAccountCreatorStatusRequestOk if the request has been sent, #LinphoneAccountCreatorStatusRequestFailed otherwise
**/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus linphone_account_creator_activate_account(LinphoneAccountCreator *creator);

/**
 * Send a request to link an account to an alias.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return #LinphoneAccountCreatorStatusRequestOk if the request has been sent, #LinphoneAccountCreatorStatusRequestFailed otherwise
**/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus linphone_account_creator_link_account(LinphoneAccountCreator *creator);

/**
 * Send a request to activate an alias.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return #LinphoneAccountCreatorStatusRequestOk if the request has been sent, #LinphoneAccountCreatorStatusRequestFailed otherwise
**/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus linphone_account_creator_activate_alias(LinphoneAccountCreator *creator);

/**
 * Send a request to know if an alias is used.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return #LinphoneAccountCreatorStatusRequestOk if the request has been sent, #LinphoneAccountCreatorStatusRequestFailed otherwise
**/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus linphone_account_creator_is_alias_used(LinphoneAccountCreator *creator);

/**
 * Send a request to know if an account is linked.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return #LinphoneAccountCreatorStatusRequestOk if the request has been sent, #LinphoneAccountCreatorStatusRequestFailed otherwise
**/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus linphone_account_creator_is_account_linked(LinphoneAccountCreator *creator);

/**
 * Send a request to recover an account.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return #LinphoneAccountCreatorStatusRequestOk if the request has been sent, #LinphoneAccountCreatorStatusRequestFailed otherwise
**/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus linphone_account_creator_recover_account(LinphoneAccountCreator *creator);

/**
 * Send a request to update an account.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return #LinphoneAccountCreatorStatusRequestOk if the request has been sent, #LinphoneAccountCreatorStatusRequestFailed otherwise
**/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus linphone_account_creator_update_account(LinphoneAccountCreator *creator);

/**
 * Send a request to get the password & algorithm of an account using the confirmation key
 * @param creator #LinphoneAccountCreator object @notnil
 * @return #LinphoneAccountCreatorStatusRequestOk if everything is OK, or a specific error otherwise.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus linphone_account_creator_login_linphone_account(LinphoneAccountCreator *creator);

/**
 * Request an auth token to be send by the backend by push notification.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return #LinphoneAccountCreatorStatusRequestOk if everything is OK, or a specific error otherwise.
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_request_auth_token(LinphoneAccountCreator *creator);

/**
 * Request an account creation "request_token" to be used on account creations. The request_token is retrieved from the callback linphone_account_creator_cbs_get_account_creation_request_token()
 * @param creator #LinphoneAccountCreator object @notnil
 * @return #LinphoneAccountCreatorStatusRequestOk if everything is OK, or a specific error otherwise.
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_request_account_creation_request_token(LinphoneAccountCreator *creator);

/**
 * Send a request to get a token to be used for account creation from a request_token. The token is retrieved from the callback linphone_account_creator_cbs_get_account_creation_token_using_request_token()
 * @param creator #LinphoneAccountCreator object @notnil
 * @param token the request token to check. It comes from linphone_account_creator_cbs_get_account_creation_request_token()
 * @return #LinphoneAccountCreatorStatusRequestOk if the request has been sent, #LinphoneAccountCreatorStatusRequestFailed otherwise
**/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus linphone_account_creator_request_account_creation_token_using_request_token(LinphoneAccountCreator *creator);

/**
 * Acquire a reference to the LinphoneAccountCreator.
 * @param creator #LinphoneAccountCreator object. @notnil
 * @return The same #LinphoneAccountCreator object. @notnil
**/
LINPHONE_PUBLIC LinphoneAccountCreator * linphone_account_creator_ref(LinphoneAccountCreator *creator);

/**
 * Release reference to the LinphoneAccountCreator.
 * @param creator #LinphoneAccountCreator object. @notnil
**/
LINPHONE_PUBLIC void linphone_account_creator_unref(LinphoneAccountCreator *creator);

/**
 * Retrieve the user pointer associated with the LinphoneAccountCreator.
 * @param creator #LinphoneAccountCreator object. @notnil
 * @return The user pointer associated with the LinphoneAccountCreator. @maybenil
**/
LINPHONE_PUBLIC void *linphone_account_creator_get_user_data(const LinphoneAccountCreator *creator);

/**
 * Assign a user pointer to the LinphoneAccountCreator.
 * @param creator #LinphoneAccountCreator object. @notnil
 * @param user_data The user pointer to associate with the LinphoneAccountCreator. @maybenil
**/
LINPHONE_PUBLIC void linphone_account_creator_set_user_data(LinphoneAccountCreator *creator, void *user_data);

/**
 * Assign a proxy config pointer to the LinphoneAccountCreator.
 * @param creator LinphoneAccountCreator object. @notnil
 * @param cfg The LinphoneProxyConfig to associate with the LinphoneAccountCreator. @maybenil
**/
LINPHONE_PUBLIC void linphone_account_creator_set_proxy_config(LinphoneAccountCreator *creator, LinphoneProxyConfig *cfg);

/**
 * Set the username.
 * @param creator #LinphoneAccountCreator object @notnil
 * @param username The username to set @maybenil
 * @return #LinphoneAccountCreatorUsernameStatusOk if everything is OK, or a specific error otherwise.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorUsernameStatus linphone_account_creator_set_username(LinphoneAccountCreator *creator, const char *username);

/**
 * Get the username.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return The username of the #LinphoneAccountCreator. @maybenil
**/
LINPHONE_PUBLIC const char * linphone_account_creator_get_username(const LinphoneAccountCreator *creator);

/**
 * Set the provider to be used by the backend to send the push notification to the device asking for an auth token.
 * @param creator #LinphoneAccountCreator object @notnil
 * @param pn_provider The pn_provider to set @maybenil
**/
LINPHONE_PUBLIC void linphone_account_creator_set_pn_provider(LinphoneAccountCreator *creator, const char* pn_provider);

/**
 * Get the provider to be used by the backend to send the push notification to the device asking for an auth token.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return The pn_provider set, if any @maybenil
**/
LINPHONE_PUBLIC const char* linphone_account_creator_get_pn_provider(const LinphoneAccountCreator *creator);

/**
 * Set the param to be used by the backend to send the push notification to the device asking for an auth token.
 * @param creator #LinphoneAccountCreator object @notnil
 * @param pn_param The pn_param to set @maybenil
**/
LINPHONE_PUBLIC void linphone_account_creator_set_pn_param(LinphoneAccountCreator *creator, const char* pn_param);

/**
 * Get the param to be used by the backend to send the push notification to the device asking for an auth token.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return The pn_param set, if any @maybenil
**/
LINPHONE_PUBLIC const char* linphone_account_creator_get_pn_param(const LinphoneAccountCreator *creator);

/**
 * Set the prid to be used by the backend to send the push notification to the device asking for an auth token.
 * @param creator #LinphoneAccountCreator object @notnil
 * @param pn_prid The pn_prid to set @maybenil
**/
LINPHONE_PUBLIC void linphone_account_creator_set_pn_prid(LinphoneAccountCreator *creator, const char* pn_prid);

/**
 * Get the prid to be used by the backend to send the push notification to the device asking for an auth token.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return The pn_prid set, if any @maybenil
**/
LINPHONE_PUBLIC const char* linphone_account_creator_get_pn_prid(const LinphoneAccountCreator *creator);

/**
 * Set the authentication token received by push notification to be used to authenticate next queries, if required.
 * @param creator #LinphoneAccountCreator object @notnil
 * @param token The token to set @maybenil
**/
LINPHONE_PUBLIC void linphone_account_creator_set_token(LinphoneAccountCreator *creator, const char* token);

/**
 * Get the authentication token set (if any) to be used to authenticate next queries, if required.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return The token set, if any @maybenil
**/
LINPHONE_PUBLIC const char* linphone_account_creator_get_token(const LinphoneAccountCreator *creator);

/**
 * Set the account creation request token received to be used to check user validation.
 * @param creator #LinphoneAccountCreator object @notnil
 * @param token The token to set @maybenil
**/
LINPHONE_PUBLIC void linphone_account_creator_set_account_creation_request_token(LinphoneAccountCreator *creator, const char* token);

/**
 * Get the account creation request token received to be used to check user validation.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return The token set, if any @maybenil
**/
LINPHONE_PUBLIC const char* linphone_account_creator_get_account_creation_request_token(const LinphoneAccountCreator *creator);

/**
 * Set the phone number normalized.
 * @param creator #LinphoneAccountCreator object @notnil
 * @param phone_number The phone number to set @maybenil
 * @param country_code Country code to associate phone number with @maybenil
 * @return #LinphoneAccountCreatorPhoneNumberStatusOk if everything is OK, or specific(s) error(s) otherwise.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorPhoneNumberStatusMask linphone_account_creator_set_phone_number(LinphoneAccountCreator *creator, const char *phone_number, const char *country_code);

/**
 * Get the RFC 3966 normalized phone number.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return The phone number of the #LinphoneAccountCreator. @maybenil
**/
LINPHONE_PUBLIC const char * linphone_account_creator_get_phone_number(const LinphoneAccountCreator *creator);

/**
 * Get the international prefix.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return The international prefix (or phone country code) of the #LinphoneAccountCreator. @maybenil
**/
LINPHONE_PUBLIC const char * linphone_account_creator_get_phone_country_code(const LinphoneAccountCreator *creator);

/**
 * Set the password.
 * @param creator #LinphoneAccountCreator object @notnil
 * @param password The password to set @maybenil
 * @return #LinphoneAccountCreatorPasswordStatusOk if everything is OK, or specific(s) error(s) otherwise.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorPasswordStatus linphone_account_creator_set_password(LinphoneAccountCreator *creator, const char *password);

/**
 * Get the password.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return The password of the #LinphoneAccountCreator. @maybenil
**/
LINPHONE_PUBLIC const char * linphone_account_creator_get_password(const LinphoneAccountCreator *creator);

/**
 * Set the ha1.
 * @param creator #LinphoneAccountCreator object @notnil
 * @param ha1 The ha1 to set @maybenil
 * @return #LinphoneAccountCreatorPasswordStatusOk if everything is OK, or a specific error otherwise.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorPasswordStatus linphone_account_creator_set_ha1(LinphoneAccountCreator *creator, const char *ha1);

/**
 * Get the ha1.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return The ha1 of the #LinphoneAccountCreator. @maybenil
**/
LINPHONE_PUBLIC const char * linphone_account_creator_get_ha1(const LinphoneAccountCreator *creator);

/**
 * Set the activation code.
 * @param creator #LinphoneAccountCreator object @notnil
 * @param activation_code The activation code to set @maybenil
 * @return #LinphoneAccountCreatorActivationCodeStatusOk if everything is OK, or a specific error otherwise.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorActivationCodeStatus linphone_account_creator_set_activation_code(LinphoneAccountCreator *creator, const char *activation_code);

/**
 * Get the activation code.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return The activation code of the #LinphoneAccountCreator. @maybenil
**/
LINPHONE_PUBLIC const char * linphone_account_creator_get_activation_code(const LinphoneAccountCreator *creator);

/**
 * Set the language to use in email or SMS if supported.
 * @param creator #LinphoneAccountCreator object @notnil
 * @param lang The language to use @maybenil
 * @return #LinphoneAccountCreatorLanguageStatusOk if everything is OK, or a specific error otherwise.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorLanguageStatus linphone_account_creator_set_language(LinphoneAccountCreator *creator, const char *lang);

/**
 * Get the language use in email of SMS.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return The language of the #LinphoneAccountCreator. @maybenil
**/
LINPHONE_PUBLIC const char * linphone_account_creator_get_language(const LinphoneAccountCreator *creator);

/**
* Set the supported algorithm.
* @param creator LinphoneAccountCreator object @notnil
* @param algorithm The algorithm to use @maybenil
* @return LinphoneAccountCreatorAlgoStatusOk if everything is OK, or a specific error otherwise.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorAlgoStatus linphone_account_creator_set_algorithm(LinphoneAccountCreator *creator, const char *algorithm);

/**
 * Get the algorithm configured in the account creator.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return The algorithm of the #LinphoneAccountCreator. @maybenil
**/
LINPHONE_PUBLIC const char * linphone_account_creator_get_algorithm(const LinphoneAccountCreator *creator);

/**
 * Set the display name.
 * @param creator #LinphoneAccountCreator object @notnil
 * @param display_name The display name to set @maybenil
 * @return #LinphoneAccountCreatorUsernameStatusOk if everything is OK, or a specific error otherwise.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorUsernameStatus linphone_account_creator_set_display_name(LinphoneAccountCreator *creator, const char *display_name);

/**
 * Get the display name.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return The display name of the #LinphoneAccountCreator. @maybenil
**/
LINPHONE_PUBLIC const char * linphone_account_creator_get_display_name(const LinphoneAccountCreator *creator);

/**
 * Set the email.
 * @param creator #LinphoneAccountCreator object @notnil
 * @param email The email to set @maybenil
 * @return #LinphoneAccountCreatorEmailStatusOk if everything is OK, or a specific error otherwise.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorEmailStatus linphone_account_creator_set_email(LinphoneAccountCreator *creator, const char *email);

/**
 * Get the email.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return The email of the #LinphoneAccountCreator. @maybenil
**/
LINPHONE_PUBLIC const char * linphone_account_creator_get_email(const LinphoneAccountCreator *creator);

/**
 * Set the domain.
 * @param creator #LinphoneAccountCreator object @notnil
 * @param domain The domain to set @maybenil
 * @return #LinphoneAccountCreatorDomainOk if everything is OK, or a specific error otherwise.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorDomainStatus linphone_account_creator_set_domain(LinphoneAccountCreator *creator, const char *domain);

/**
 * Get the domain.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return The domain of the #LinphoneAccountCreator. @maybenil
**/
LINPHONE_PUBLIC const char * linphone_account_creator_get_domain(const LinphoneAccountCreator *creator);

/**
 * Set Transport
 * @param creator #LinphoneAccountCreator object @notnil
 * @param transport The #LinphoneTransportType to set
 * @return #LinphoneAccountCreatorTransportOk if everything is OK, or a specific error otherwise.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorTransportStatus linphone_account_creator_set_transport(LinphoneAccountCreator *creator, LinphoneTransportType transport);

/**
 * Get Transport
 * @param creator #LinphoneAccountCreator object @notnil
 * @return The #LinphoneTransportType of the creator.
**/
LINPHONE_PUBLIC LinphoneTransportType linphone_account_creator_get_transport(const LinphoneAccountCreator *creator);

/**
 * Set the set_as_default property.
 * @param creator #LinphoneAccountCreator object @notnil
 * @param set_as_default TRUE for the created proxy config to be set as default in #LinphoneCore, FALSE otherwise
 * @return #LinphoneAccountCreatorStatusRequestOk if everything is OK, or a specific error otherwise.
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus linphone_account_creator_set_as_default(LinphoneAccountCreator *creator, bool_t set_as_default);

/**
 * Get the set_as_default property.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return TRUE if account will be set as default, FALSE otherwise.
 **/
LINPHONE_PUBLIC bool_t linphone_account_creator_get_set_as_default(const LinphoneAccountCreator *creator);

/**
 * Add the #LinphoneAccountCreatorCbs object to a LinphoneAccountCreator.
 * @param creator #LinphoneAccountCreator object @notnil
 * @param cbs The #LinphoneAccountCreatorCbs object to add to the LinphoneAccountCreator. @notnil
**/
LINPHONE_PUBLIC void linphone_account_creator_add_callbacks(LinphoneAccountCreator *creator, LinphoneAccountCreatorCbs *cbs);

/**
 * Removes the #LinphoneAccountCreatorCbs object from a LinphoneAccountCreator.
 * @param creator #LinphoneAccountCreator object @notnil
 * @param cbs The #LinphoneAccountCreatorCbs object to remove from the LinphoneAccountCreator. @notnil
**/
LINPHONE_PUBLIC void linphone_account_creator_remove_callbacks(LinphoneAccountCreator *creator, LinphoneAccountCreatorCbs *cbs);

/**
 * Get the current #LinphoneAccountCreatorCbs object associated with a LinphoneAccountCreator.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return The current #LinphoneAccountCreatorCbs object associated with the LinphoneAccountCreator. @maybenil
**/
LINPHONE_PUBLIC LinphoneAccountCreatorCbs *linphone_account_creator_get_current_callbacks(const LinphoneAccountCreator *creator);

/**
 * Get the #LinphoneAccountCreatorService object associated with a LinphoneAccountCreator.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return The #LinphoneAccountCreatorService object associated with the LinphoneAccountCreator.
 * @donotwrap
**/
LINPHONE_PUBLIC LinphoneAccountCreatorService * linphone_account_creator_get_service(const LinphoneAccountCreator *creator);

/************************** End Account Creator data **************************/

/************************** Start Account Creator Cbs **************************/

/**
 * Acquire a reference to a #LinphoneAccountCreatorCbs object.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @return The same #LinphoneAccountCreatorCbs object. @notnil
**/
LINPHONE_PUBLIC LinphoneAccountCreatorCbs * linphone_account_creator_cbs_ref(LinphoneAccountCreatorCbs *cbs);

/**
 * Release a reference to a #LinphoneAccountCreatorCbs object.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
**/
LINPHONE_PUBLIC void linphone_account_creator_cbs_unref(LinphoneAccountCreatorCbs *cbs);

/**
 * Retrieve the user pointer associated with a #LinphoneAccountCreatorCbs object.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @return The user pointer associated with the #LinphoneAccountCreatorCbs object. @maybenil
**/
LINPHONE_PUBLIC void *linphone_account_creator_cbs_get_user_data(const LinphoneAccountCreatorCbs *cbs);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorCbs object.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @param user_data The user pointer to associate with the #LinphoneAccountCreatorCbs object. @maybenil
**/
LINPHONE_PUBLIC void linphone_account_creator_cbs_set_user_data(LinphoneAccountCreatorCbs *cbs, void *user_data);

/**
 * Get the create account request.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @return The current create account request.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_create_account(const LinphoneAccountCreatorCbs *cbs);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorCbs object.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @param cb The create account request to be used.
**/
LINPHONE_PUBLIC void linphone_account_creator_cbs_set_create_account(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb);

/**
 * Get the is account exist request.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @return The current is account exist request.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_is_account_exist(const LinphoneAccountCreatorCbs *cbs);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorCbs object.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @param cb The is account exist request to be used.
**/
LINPHONE_PUBLIC void linphone_account_creator_cbs_set_is_account_exist(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb);

/**
 * Get the activate account request.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @return The current activate account request.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_activate_account(const LinphoneAccountCreatorCbs *cbs);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorCbs object.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @param cb The activate account request to be used.
**/
LINPHONE_PUBLIC void linphone_account_creator_cbs_set_activate_account(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb);

/**
 * Get the activate account request.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @return The current activate account request.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_send_token(const LinphoneAccountCreatorCbs *cbs);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorCbs object.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @param cb The send token request used.
**/
LINPHONE_PUBLIC void linphone_account_creator_cbs_set_send_token(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb);

/**
 * Get the callback on account creation request token.
 * In response:
 *   - "token" is the request token to used with linphone_account_creator_request_account_creation_token_using_request_token().
 *   - "validation_url" is a URL to redirect the user into a browser for validation.
 * In status:
 *   - LinphoneAccountCreatorStatusRequestOk: the request token should be in response with the validation url.
 *
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @return The current request token request.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_account_creation_request_token(const LinphoneAccountCreatorCbs *cbs);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorCbs object.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @param cb The request token callback used.
**/
LINPHONE_PUBLIC void linphone_account_creator_cbs_set_account_creation_request_token(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb);

/**
 * Get the callback on account creation token.
 * In response, "token" is the token to pass to linphone_account_creator_set_token(). It is used for linphone_account_creator_create_account()
 *
 * In status:
 *   - LinphoneAccountCreatorStatusRequestOk: token can be retrieved from the "token" field in response.
 *   - LinphoneAccountCreatorStatusRequestFailed: request token has not been validated. Recall linphone_account_creator_request_account_creation_token_using_request_token() after some time.
 *   - LinphoneAccountCreatorStatusMissingArguments: request_token has not been set from linphone_account_creator_set_token().
 *   - LinphoneAccountCreatorStatusServerError: URL is not reachable.
 *
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @return The current request token request.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_account_creation_token_using_request_token(const LinphoneAccountCreatorCbs *cbs);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorCbs object.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @param cb The token callback used.
**/
LINPHONE_PUBLIC void linphone_account_creator_cbs_set_account_creation_token_using_request_token(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb);

/**
 * Get the is account activated request.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @return The current is account activated request.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_is_account_activated(const LinphoneAccountCreatorCbs *cbs);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorCbs object.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @param cb The is account activated request to be used.
**/
LINPHONE_PUBLIC void linphone_account_creator_cbs_set_is_account_activated(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb);

/**
 * Get the link account request.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @return The current link account request.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_link_account(const LinphoneAccountCreatorCbs *cbs);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorCbs object.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @param cb The link account request to be used.
**/
LINPHONE_PUBLIC void linphone_account_creator_cbs_set_link_account(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb);

/**
 * Get the activate alias request.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @return The current link account request.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_activate_alias(const LinphoneAccountCreatorCbs *cbs);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorCbs object.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @param cb The activate alias request to be used.
**/
LINPHONE_PUBLIC void linphone_account_creator_cbs_set_activate_alias(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb);

/**
 * Get the is alias used request.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @return The current is alias used request.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_is_alias_used(const LinphoneAccountCreatorCbs *cbs);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorCbs object.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @param cb The is alias used request to be used.
**/
LINPHONE_PUBLIC void linphone_account_creator_cbs_set_is_alias_used(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb);

/**
 * Get the is account linked request.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @return The current is account linked request.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_is_account_linked(const LinphoneAccountCreatorCbs *cbs);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorCbs object.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @param cb The is account linked request to be used.
**/
LINPHONE_PUBLIC void linphone_account_creator_cbs_set_is_account_linked(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb);

/**
 * Get the recover account request.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @return The current recover account request.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_recover_account(const LinphoneAccountCreatorCbs *cbs);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorCbs object.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @param cb The recover account request to be used.
**/
LINPHONE_PUBLIC void linphone_account_creator_cbs_set_recover_account(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb);

/**
 * Get the update account request.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @return The current update account request.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_update_account(const LinphoneAccountCreatorCbs *cbs);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorCbs object.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @param cb The update account request to be used.
**/
LINPHONE_PUBLIC void linphone_account_creator_cbs_set_update_account(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb);

/**
 * Get the login linphone account request.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @return The current login linphone account request.
**/
LINPHONE_PUBLIC LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_login_linphone_account(const LinphoneAccountCreatorCbs *cbs);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorCbs object.
 * @param cbs #LinphoneAccountCreatorCbs object. @notnil
 * @param cb The login linphone account request to be used.
**/
LINPHONE_PUBLIC void linphone_account_creator_cbs_set_login_linphone_account(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb);

/************************** End Account Creator Cbs **************************/

/**
 * Create and configure a proxy config and a authentication info for an account creator
 * @param creator #LinphoneAccountCreator object @notnil
 * @return A #LinphoneProxyConfig object if successful, NULL otherwise. @maybenil
 * @deprecated 05/05/2023 Use linphone_account_creator_create_account_in_core() instead.
 **/
LINPHONE_PUBLIC LinphoneProxyConfig *
linphone_account_creator_create_proxy_config(const LinphoneAccountCreator *creator);

/**
 * Create and configure a #LinphoneAccount and a #LinphoneAuthInfo from informations set in the #LinphoneAccountCreator.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return A #LinphoneAccount object if successful, NULL otherwise. @maybenil @tobefreed
 **/
LINPHONE_PUBLIC LinphoneAccount *linphone_account_creator_create_account_in_core(const LinphoneAccountCreator *creator);


/**
 * Require the account creator to use special "test admin account".
 * @warning The "test admin account" is a special feature required for automated test, and requires the APP_EVERYONE_IS_ADMIN
 * property to be enabled on the remote Flexisip Account Manager (FlexiAPI).
 * This feature must never be turned on for a production-stage app.
 * @param creator #LinphoneAccountCreator object @notnil
 **/
LINPHONE_PUBLIC void linphone_account_creator_use_test_admin_account(LinphoneAccountCreator *creator);

/**************/
/* DEPRECATED */
/**************/

/**
 * Get the #LinphoneAccountCreatorCbs object associated with a LinphoneAccountCreator.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return The #LinphoneAccountCreatorCbs object associated with the LinphoneAccountCreator. @maybenil
 * @deprecated 19/02/2019 use add_callbacks / remove_callbacks instead
 * @donotwrap
**/
LINPHONE_DEPRECATED LINPHONE_PUBLIC LinphoneAccountCreatorCbs * linphone_account_creator_get_callbacks(const LinphoneAccountCreator *creator);

/**
 * Set the route.
 * @param creator #LinphoneAccountCreator object @notnil
 * @param route The route to set @maybenil
 * @return #LinphoneAccountCreatorStatusRequestOk if everything is OK, or a specific error otherwise.
**/
LINPHONE_DEPRECATED LINPHONE_PUBLIC LinphoneAccountCreatorStatus linphone_account_creator_set_route(LinphoneAccountCreator *creator, const char *route);

/**
 * Get the route.
 * @param creator #LinphoneAccountCreator object @notnil
 * @return The route of the #LinphoneAccountCreator. @maybenil
**/
LINPHONE_DEPRECATED LINPHONE_PUBLIC const char * linphone_account_creator_get_route(const LinphoneAccountCreator *creator);

/**
 * Configure an account (create a proxy config and authentication info for it).
 * @param creator #LinphoneAccountCreator object @notnil
 * @return A #LinphoneProxyConfig object if successful, NULL otherwise @maybenil
 * @deprecated 13/04/2017 Use linphone_account_creator_create_proxy_config() instead
 * @donotwrap
**/
LINPHONE_DEPRECATED LINPHONE_PUBLIC LinphoneProxyConfig * linphone_account_creator_configure(const LinphoneAccountCreator *creator);

/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_ACCOUNT_CREATOR_H_ */
