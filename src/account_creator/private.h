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

#ifndef _ACCOUNT_CREATOR_PRIVATE_H_
#define _ACCOUNT_CREATOR_PRIVATE_H_

#include "linphone/account_creator.h"
#include "linphone/account_creator_service.h"
#include "linphone/defs.h"
#include <belle-sip/object.h>

#define NOTIFY_IF_EXIST_ACCOUNT_CREATOR(functionName, ...)                                                             \
	bctbx_list_t *callbacksCopy = bctbx_list_copy_with_data(linphone_account_creator_get_callbacks_list(creator),      \
	                                                        (bctbx_list_copy_func)belle_sip_object_ref);               \
	for (bctbx_list_t *it = callbacksCopy; it; it = bctbx_list_next(it)) {                                             \
		linphone_account_creator_set_current_callbacks(                                                                \
		    creator, static_cast<LinphoneAccountCreatorCbs *>(bctbx_list_get_data(it)));                               \
		LinphoneAccountCreatorCbsStatusCb cb =                                                                         \
		    linphone_account_creator_cbs_get_##functionName(linphone_account_creator_get_current_callbacks(creator));  \
		if (cb) cb(__VA_ARGS__);                                                                                       \
	}                                                                                                                  \
	linphone_account_creator_set_current_callbacks(creator, nullptr);                                                  \
	bctbx_list_free_with_data(callbacksCopy, (bctbx_list_free_func)belle_sip_object_unref);

struct _LinphoneAccountCreatorService {
	belle_sip_object_t base;
	void *user_data;

	LinphoneAccountCreatorRequestFunc account_creator_service_constructor_cb; /**< Constructor */
	LinphoneAccountCreatorRequestFunc account_creator_service_destructor_cb;  /**< Destructor */

	LinphoneAccountCreatorRequestFunc create_account_request_cb;   /**< Request to create account */
	LinphoneAccountCreatorRequestFunc delete_account_request_cb;   /**< Request to delete account */
	LinphoneAccountCreatorRequestFunc is_account_exist_request_cb; /**< Request to know if account exist */
	LinphoneAccountCreatorRequestFunc confirmation_key_request_cb; /**< Request to get the confirmation key */

	LinphoneAccountCreatorRequestFunc activate_account_request_cb;     /**< Request to activate account */
	LinphoneAccountCreatorRequestFunc is_account_activated_request_cb; /**< Request to know if account is activated */

	LinphoneAccountCreatorRequestFunc link_account_request_cb;   /**< Request to link account with an alias */
	LinphoneAccountCreatorRequestFunc activate_alias_request_cb; /**< Request to activate the link of alias */
	LinphoneAccountCreatorRequestFunc is_alias_used_request_cb;  /**< Request to know if alias is used */
	LinphoneAccountCreatorRequestFunc
	    is_account_linked_request_cb; /**< Request to know if account is linked with an alias */

	LinphoneAccountCreatorRequestFunc recover_account_request_cb; /**< Request to recover account */
	LinphoneAccountCreatorRequestFunc update_account_request_cb;  /**< Request to update account */
	LinphoneAccountCreatorRequestFunc
	    login_linphone_account_request_cb; /**< Request to get password & algorithm from confirmation key */

	LinphoneAccountCreatorRequestFunc send_token_request_cb; /**< Generate and send a token via push notification */
	/**< Generate and get a request token */
	LinphoneAccountCreatorRequestFunc account_creation_request_token_request_cb;
	/**< Generate and get a token for account creation */
	LinphoneAccountCreatorRequestFunc account_creation_token_using_request_token_request_cb;
	/**< Use a received token to create an account */
	LinphoneAccountCreatorRequestFunc create_account_with_token_request_cb;
	/**< Request to create a push account */
	LinphoneAccountCreatorRequestFunc create_push_account_request_cb;
};

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneAccountCreatorService);

struct _LinphoneAccountCreatorCbs {
	belle_sip_object_t base;
	void *user_data;

	LinphoneAccountCreatorCbsStatusCb create_account_response_cb;   /**< Response of create_account request */
	LinphoneAccountCreatorCbsStatusCb delete_account_response_cb;   /**< Response of delete_account request */
	LinphoneAccountCreatorCbsStatusCb is_account_exist_response_cb; /**< Response of is_account_exist request */
	LinphoneAccountCreatorCbsStatusCb confirmation_key_response_cb; /**< Response of get_confirmation_key request */

	LinphoneAccountCreatorCbsStatusCb activate_account_response_cb;     /**< Response of activate_account request */
	LinphoneAccountCreatorCbsStatusCb is_account_activated_response_cb; /**< Response of is_account_activated request */

	LinphoneAccountCreatorCbsStatusCb link_account_response_cb;      /**< Response of link_account request */
	LinphoneAccountCreatorCbsStatusCb activate_alias_response_cb;    /**< Response of activation alias */
	LinphoneAccountCreatorCbsStatusCb is_alias_used_response_cb;     /**< Response of is_alias_used request */
	LinphoneAccountCreatorCbsStatusCb is_account_linked_response_cb; /**< Response of is_account_linked request */

	LinphoneAccountCreatorCbsStatusCb recover_account_response_cb; /**< Response of recover_account request */
	LinphoneAccountCreatorCbsStatusCb update_account_response_cb;  /**< Response of update_account request */
	LinphoneAccountCreatorCbsStatusCb
	    login_linphone_account_response_cb; /** < Response of login_linphone_account request */

	LinphoneAccountCreatorCbsStatusCb send_token_cb; /** < Response of login_linphone_account request */
	LinphoneAccountCreatorCbsStatusCb account_creation_request_token_cb; /** < Response of request token request */
	LinphoneAccountCreatorCbsStatusCb account_creation_token_using_request_token_cb; /** < Response of token request */
};

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneAccountCreatorCbs);

struct _LinphoneAccountCreator {
	belle_sip_object_t base;
	void *user_data;
	LinphoneCore *core;

	/* AccountCreator */
	LinphoneAccountCreatorService *service; /**< Account creator service */

	LinphoneAccountCreatorCbs *cbs; /**< Account creator cbs, deprecated, use the list of callbacks */

	bctbx_list_t *callbacks;
	LinphoneAccountCreatorCbs *currentCbs; /** Used during the callbacks list iteration */

	LinphoneAccount *account; /**< Default account */

	/* User */
	char *username;     /**< Username */
	char *display_name; /**< Display name */
	/* Password */
	char *password; /**< Plain text password */
	char *ha1;      /**< Hash password */
	/* Phone Number(Alias) */
	char *phone_number;       /**< User phone number*/
	char *phone_country_code; /**< User phone number country code */
	/* Email(Alias) */
	char *email; /**< User email */
	/* Misc */
	char *language;                  /**< User language */
	char *activation_code;           /**< Account validation code */
	char *domain;                    /**< Domain */
	char *algorithm;                 /**< Digest authentication algorithm */
	LinphoneTransportType transport; /**< Transport used */

	/* Push Notification */
	char *pn_provider;
	char *pn_param;
	char *pn_prid;
	char *token;

	/* account creation tokens */
	char *account_creation_request_token;

	bool_t set_as_default; /**< Set account as the default one */

	/* Deprecated */
	char *route;
};

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneAccountCreator);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Account creator custom to set Linphone default values
 * @param[in] creator LinphoneAccountCreator object
 * @return LinphoneAccountCreatorStatusRequestOk if the request has been sent, LinphoneAccountCreatorStatusRequestFailed
 *otherwise
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_constructor_linphone_xmlrpc(LinphoneAccountCreator *creator);

/**
 * Send an XML-RPC request to test the existence of a Linphone account.
 * @param[in] creator LinphoneAccountCreator object
 * @return LinphoneAccountCreatorStatusRequestOk if the request has been sent, LinphoneAccountCreatorStatusRequestFailed
 *otherwise
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_is_account_exist_linphone_xmlrpc(LinphoneAccountCreator *creator);
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_is_account_exist_flexiapi(LinphoneAccountCreator *creator);

/**
 * Send an XML-RPC request to create a Linphone account.
 * @param[in] creator LinphoneAccountCreator object
 * @return LinphoneAccountCreatorStatusRequestOk if the request has been sent, LinphoneAccountCreatorStatusRequestFailed
 *otherwise
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_create_account_linphone_xmlrpc(LinphoneAccountCreator *creator);
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_create_account_flexiapi(LinphoneAccountCreator *creator); // unsecure

/**
 * Send an XML-RPC request to delete a Linphone account.
 * @param[in] creator LinphoneAccountCreator object
 * @return LinphoneAccountCreatorStatusRequestOk if the request has been sent, LinphoneAccountCreatorStatusRequestFailed
 *otherwise
 * @donotwrap Exists for tests purposes only
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_delete_account_linphone_xmlrpc(LinphoneAccountCreator *creator);
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_delete_account_flexiapi(LinphoneAccountCreator *creator);

/**
 * Send an XML-RPC request to get the confirmation key of a Linphone account.
 * @param[in] creator LinphoneAccountCreator object
 * @return LinphoneAccountCreatorStatusRequestOk if the request has been sent, LinphoneAccountCreatorStatusRequestFailed
 *otherwise
 * @donotwrap Exists for tests purposes only
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_get_confirmation_key_linphone_xmlrpc(LinphoneAccountCreator *creator); // for test purpose only

/**
 * Send an XML-RPC request to activate a Linphone account with phone number.
 * @param[in] creator LinphoneAccountCreator object
 * @return LinphoneAccountCreatorStatusRequestOk if the request has been sent, LinphoneAccountCreatorStatusRequestFailed
 *otherwise
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_activate_phone_account_linphone_xmlrpc(LinphoneAccountCreator *creator);
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_activate_phone_account_flexiapi(LinphoneAccountCreator *creator);

/**
 * Send an XML-RPC request to activate a Linphone account with email.
 * @param[in] creator LinphoneAccountCreator object
 * @return LinphoneAccountCreatorStatusRequestOk if the request has been sent, LinphoneAccountCreatorStatusRequestFailed
 *otherwise
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_activate_email_account_linphone_xmlrpc(LinphoneAccountCreator *creator);
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_activate_email_account_flexiapi(LinphoneAccountCreator *creator);

/**
 * Send an XML-RPC request to test the validation of a Linphone account.
 * @param[in] creator LinphoneAccountCreator object
 * @return LinphoneAccountCreatorStatusRequestOk if the request has been sent, LinphoneAccountCreatorStatusRequestFailed
 *otherwise
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_is_account_activated_linphone_xmlrpc(LinphoneAccountCreator *creator);
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_is_account_activated_flexiapi(LinphoneAccountCreator *creator);

/**
 * Send an XML-RPC request to test the existence a phone number with a Linphone account.
 * @param[in] creator LinphoneAccountCreator object
 * @return LinphoneAccountCreatorStatusRequestOk if the request has been sent, LinphoneAccountCreatorStatusRequestFailed
 *otherwise
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_is_phone_number_used_linphone_xmlrpc(LinphoneAccountCreator *creator);
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_is_phone_number_used_flexiapi(LinphoneAccountCreator *creator); // unsecure

/**
 * Send an XML-RPC request to link a phone number with a Linphone account.
 * @param[in] creator LinphoneAccountCreator object
 * @return LinphoneAccountCreatorStatusRequestOk if the request has been sent, LinphoneAccountCreatorStatusRequestFailed
 *otherwise
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_link_phone_number_with_account_linphone_xmlrpc(LinphoneAccountCreator *creator);
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_link_phone_number_with_account_flexiapi(LinphoneAccountCreator *creator);

/**
 * Send an XML-RPC request to activate the link of a phone number with a Linphone account.
 * @param[in] creator LinphoneAccountCreator object
 * @return LinphoneAccountCreatorStatusRequestOk if the request has been sent, LinphoneAccountCreatorStatusRequestFailed
 *otherwise
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_activate_phone_number_link_linphone_xmlrpc(LinphoneAccountCreator *creator);
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_activate_phone_number_link_flexiapi(LinphoneAccountCreator *creator);

/**
 * Send an XML-RPC request to a Linphone account with the phone number.
 * @param[in] creator LinphoneAccountCreator object
 * @return LinphoneAccountCreatorStatusRequestOk if the request has been sent, LinphoneAccountCreatorStatusRequestFailed
 *otherwise
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_recover_phone_account_linphone_xmlrpc(LinphoneAccountCreator *creator);
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_recover_phone_account_flexiapi(LinphoneAccountCreator *creator); // unsecure

/**
 * Send an XML-RPC request to ask if an account is linked with a phone number
 * @param[in] creator LinphoneAccountCreator object
 * @return if this account is linked with a phone number
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_is_account_linked_linphone_xmlrpc(LinphoneAccountCreator *creator);
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_is_account_linked_flexiapi(LinphoneAccountCreator *creator);

/**
 * Send an XML-RPC request to ask if an account is linked with a phone number
 * @param[in] creator LinphoneAccountCreator object
 * @return LinphoneAccountCreatorStatusRequestOk if everything is OK, or a specific error otherwise.
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_update_password_linphone_xmlrpc(LinphoneAccountCreator *creator);
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_update_password_flexiapi(LinphoneAccountCreator *creator);

/**
 * Send an XML-RPC request to get the password & algorithm of an account using the confirmation key
 * @param[in] creator LinphoneAccountCreator object
 * @return LinphoneAccountCreatorStatusRequestOk if everything is OK, or a specific error otherwise.
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_login_linphone_account_linphone_xmlrpc(LinphoneAccountCreator *creator);
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_login_linphone_account_flexiapi(LinphoneAccountCreator *creator);

/**
 * Send an FlexiAPI request to generate a token and send it through push notification
 * @param[in] creator LinphoneAccountCreator object
 * @return LinphoneAccountCreatorStatusRequestOk if everything is OK, or a specific error otherwise.
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_send_token_flexiapi(LinphoneAccountCreator *creator);

/**
 * Send a FlexiAPI request to generate a request token with a waiting url. Check callbacks.
 * @param[in] creator LinphoneAccountCreator object
 * @return LinphoneAccountCreatorStatusRequestOk if everything is OK, or a specific error otherwise.
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_account_creation_request_token_flexiapi(LinphoneAccountCreator *creator);

/**
 * Send a FlexiAPI request to generate a token from a request token. Check callbacks.
 * @param[in] creator LinphoneAccountCreator object
 * @return LinphoneAccountCreatorStatusRequestOk if everything is OK, or a specific error otherwise.
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_account_creation_token_using_request_token_flexiapi(LinphoneAccountCreator *creator);

/**
 * Send an FlexiAPI request to create an account using a received token
 * @param[in] creator LinphoneAccountCreator object
 * @return LinphoneAccountCreatorStatusRequestOk if everything is OK, or a specific error otherwise.
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_create_account_with_token_flexiapi(LinphoneAccountCreator *creator);

/**
 * Send an FlexiAPI request to create an account as an admin
 * @param[in] creator LinphoneAccountCreator object
 * @return LinphoneAccountCreatorStatusRequestOk if everything is OK, or a specific error otherwise.
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_admin_create_account_flexiapi(LinphoneAccountCreator *creator);

/**
 * Send an FlexiAPI request to create a push account
 * @param[in] creator LinphoneAccountCreator object
 * @return LinphoneAccountCreatorStatusRequestOk if everything is OK, or a specific error otherwise.
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_create_push_account_with_token_flexiapi(LinphoneAccountCreator *creator);

char *linphone_account_creator_get_identity(const LinphoneAccountCreator *creator);
void linphone_account_creator_fill_domain_and_algorithm_if_needed(LinphoneAccountCreator *creator);
const char *linphone_account_creator_get_domain_with_fallback_to_proxy_domain(LinphoneAccountCreator *creator);

#ifdef __cplusplus
}
#endif

#endif // _ACCOUNT_CREATOR_PRIVATE_H_
