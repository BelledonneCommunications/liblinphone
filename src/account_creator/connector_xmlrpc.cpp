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

#include "bctoolbox/crypto.h"
#include "bctoolbox/regex.h"

#include "c-wrapper/c-wrapper.h"
#include "dial-plan/dial-plan.h"
#include "linphone/account_creator.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"
#ifdef HAVE_FLEXIAPI
#include "linphone/flexi-api-client.h"
#endif
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/utils/utils.h"
#include "private_functions.h"

// TODO: From coreapi. Remove me later.
#include "private.h"

// =============================================================================

using namespace LinphonePrivate;

BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneAccountCreatorCbs);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneAccountCreatorCbs,
                           belle_sip_object_t,
                           NULL, // destroy
                           NULL, // clone
                           NULL, // marshal
                           FALSE);

/************************** Start Account Creator Cbs **************************/

LinphoneAccountCreatorCbs *linphone_account_creator_cbs_new(void) {
	return belle_sip_object_new(LinphoneAccountCreatorCbs);
}

LinphoneAccountCreatorCbs *linphone_account_creator_cbs_ref(LinphoneAccountCreatorCbs *cbs) {
	belle_sip_object_ref(cbs);
	return cbs;
}

void linphone_account_creator_cbs_unref(LinphoneAccountCreatorCbs *cbs) {
	belle_sip_object_unref(cbs);
}

void *linphone_account_creator_cbs_get_user_data(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->user_data;
}

void linphone_account_creator_cbs_set_user_data(LinphoneAccountCreatorCbs *cbs, void *ud) {
	cbs->user_data = ud;
}

LinphoneAccountCreatorCbsStatusCb
linphone_account_creator_cbs_get_create_account(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->create_account_response_cb;
}

void linphone_account_creator_cbs_set_create_account(LinphoneAccountCreatorCbs *cbs,
                                                     LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->create_account_response_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb
linphone_account_creator_cbs_get_delete_account(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->delete_account_response_cb;
}

void linphone_account_creator_cbs_set_delete_account(LinphoneAccountCreatorCbs *cbs,
                                                     LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->delete_account_response_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb
linphone_account_creator_cbs_get_is_account_exist(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->is_account_exist_response_cb;
}

void linphone_account_creator_cbs_set_is_account_exist(LinphoneAccountCreatorCbs *cbs,
                                                       LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->is_account_exist_response_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb
linphone_account_creator_cbs_get_confirmation_key(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->confirmation_key_response_cb;
}

void linphone_account_creator_cbs_set_confirmation_key(LinphoneAccountCreatorCbs *cbs,
                                                       LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->confirmation_key_response_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb
linphone_account_creator_cbs_get_activate_account(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->activate_account_response_cb;
}

void linphone_account_creator_cbs_set_activate_account(LinphoneAccountCreatorCbs *cbs,
                                                       LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->activate_account_response_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_send_token(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->send_token_cb;
}

void linphone_account_creator_cbs_set_send_token(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->send_token_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb
linphone_account_creator_cbs_get_account_creation_request_token(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->account_creation_request_token_cb;
}

void linphone_account_creator_cbs_set_account_creation_request_token(LinphoneAccountCreatorCbs *cbs,
                                                                     LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->account_creation_request_token_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb
linphone_account_creator_cbs_get_account_creation_token_using_request_token(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->account_creation_token_using_request_token_cb;
}

void linphone_account_creator_cbs_set_account_creation_token_using_request_token(LinphoneAccountCreatorCbs *cbs,
                                                                                 LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->account_creation_token_using_request_token_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb
linphone_account_creator_cbs_get_is_account_activated(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->is_account_activated_response_cb;
}

void linphone_account_creator_cbs_set_is_account_activated(LinphoneAccountCreatorCbs *cbs,
                                                           LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->is_account_activated_response_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_link_account(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->link_account_response_cb;
}

void linphone_account_creator_cbs_set_link_account(LinphoneAccountCreatorCbs *cbs,
                                                   LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->link_account_response_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb
linphone_account_creator_cbs_get_activate_alias(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->activate_alias_response_cb;
}

void linphone_account_creator_cbs_set_activate_alias(LinphoneAccountCreatorCbs *cbs,
                                                     LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->activate_alias_response_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_is_alias_used(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->is_alias_used_response_cb;
}

void linphone_account_creator_cbs_set_is_alias_used(LinphoneAccountCreatorCbs *cbs,
                                                    LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->is_alias_used_response_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb
linphone_account_creator_cbs_get_is_account_linked(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->is_account_linked_response_cb;
}

void linphone_account_creator_cbs_set_is_account_linked(LinphoneAccountCreatorCbs *cbs,
                                                        LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->is_account_linked_response_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb
linphone_account_creator_cbs_get_recover_account(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->recover_account_response_cb;
}

void linphone_account_creator_cbs_set_recover_account(LinphoneAccountCreatorCbs *cbs,
                                                      LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->recover_account_response_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb
linphone_account_creator_cbs_get_update_account(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->update_account_response_cb;
}

void linphone_account_creator_cbs_set_update_account(LinphoneAccountCreatorCbs *cbs,
                                                     LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->update_account_response_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb
linphone_account_creator_cbs_get_login_linphone_account(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->login_linphone_account_response_cb;
}

void linphone_account_creator_cbs_set_login_linphone_account(LinphoneAccountCreatorCbs *cbs,
                                                             LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->login_linphone_account_response_cb = cb;
}

/************************** End Account Creator Cbs **************************/

/************************** Start Account Creator Linphone **************************/

LinphoneAccountCreatorStatus linphone_account_creator_constructor_linphone_xmlrpc(LinphoneAccountCreator *creator) {
	LinphoneAddress *addr;
	const char *identity =
	    linphone_config_get_default_string(linphone_core_get_config(creator->core), "proxy", "reg_identity", NULL);
	const char *proxy =
	    linphone_config_get_default_string(linphone_core_get_config(creator->core), "proxy", "reg_proxy", NULL);
	const char *route =
	    linphone_config_get_default_string(linphone_core_get_config(creator->core), "proxy", "reg_route", NULL);
	const char *realm =
	    linphone_config_get_default_string(linphone_core_get_config(creator->core), "proxy", "realm", NULL);

	if (!creator->account) {
		LinphoneAccountParams *account_params = linphone_core_create_account_params(creator->core);
		linphone_account_params_set_realm(account_params, realm ? realm : "sip.linphone.org");
		bctbx_list_t *route_list = nullptr;
		LinphoneAddress *route_address = linphone_address_new(route ? route : "sip.linphone.org");
		if (route_address != NULL) {
			route_list = bctbx_list_append(route_list, route_address);
		}
		linphone_account_params_set_routes_addresses(account_params, route_list);
		if (route_list) {
			bctbx_list_free_with_data(route_list, (bctbx_list_free_func)linphone_address_unref);
		}
		linphone_account_params_set_server_addr(account_params, proxy ? proxy : "sip.linphone.org");
		addr = linphone_address_new(identity ? identity : "sip:username@sip.linphone.org");
		linphone_account_params_set_identity_address(account_params, addr);
		linphone_address_unref(addr);
		creator->account = linphone_core_create_account(creator->core, account_params);
		linphone_account_params_unref(account_params);
	}

	return LinphoneAccountCreatorStatusRequestOk;
}

/****************** START OF ACCOUNT USED SECTION *****************************/
static void _is_account_exist_response_cb(LinphoneXmlRpcRequest *request) {
	LinphoneAccountCreator *creator = (LinphoneAccountCreator *)linphone_xml_rpc_request_get_user_data(request);
	LinphoneAccountCreatorStatus status = LinphoneAccountCreatorStatusRequestFailed;

	const char *resp = linphone_xml_rpc_request_get_string_response(request);
	if (linphone_xml_rpc_request_get_status(request) == LinphoneXmlRpcStatusOk) {
		status =
		    (strcmp(resp, "ERROR_ACCOUNT_DOESNT_EXIST") == 0)
		        ? LinphoneAccountCreatorStatusAccountNotExist
		        : ((strcmp(resp, "ERROR_ALIAS_DOESNT_EXIST") == 0) ? LinphoneAccountCreatorStatusAccountExist
		                                                           : LinphoneAccountCreatorStatusAccountExistWithAlias);
		if (status == LinphoneAccountCreatorStatusAccountExistWithAlias) {
			set_string(&creator->phone_number, resp, FALSE);
		}
	}

	if (creator->cbs->is_account_exist_response_cb != NULL) {
		creator->cbs->is_account_exist_response_cb(creator, status, resp);
	}
	NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_exist, creator, status, resp)
}

LinphoneAccountCreatorStatus
linphone_account_creator_is_account_exist_linphone_xmlrpc(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request = NULL;
	if (!creator->username && !creator->phone_number) {
		if (creator->cbs->is_account_exist_response_cb != NULL) {
			creator->cbs->is_account_exist_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments,
			                                           "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_exist, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);

	LinphoneXmlRpcSession *xmlrpc_session =
	    (LinphoneXmlRpcSession *)belle_sip_object_data_get(BELLE_SIP_OBJECT(creator), "xmlrpc_session");

	if (xmlrpc_session) {
		ms_debug("Account creator: is_account_exist (%s=%s, domain=%s)",
		         (creator->username) ? "username" : "phone number",
		         (creator->username) ? creator->username : creator->phone_number,
		         linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator));

		request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "get_phone_number_for_account");
		linphone_xml_rpc_request_add_string_arg(request, creator->username ? creator->username : creator->phone_number);
		linphone_xml_rpc_request_add_string_arg(
		    request, linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator));
		linphone_xml_rpc_request_set_user_data(request, creator);
		linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request),
		                                          _is_account_exist_response_cb);
		linphone_xml_rpc_session_send_request(xmlrpc_session, request);
		linphone_xml_rpc_request_unref(request);

		return LinphoneAccountCreatorStatusRequestOk;
	}
	return LinphoneAccountCreatorStatusRequestFailed;
}
/****************** END OF CREATE ACCOUNT USED SECTION ************************/

/****************** START OF CREATE ACCOUNT SECTION ***************************/
static void _create_account_cb_custom(LinphoneXmlRpcRequest *request) {
	LinphoneAccountCreator *creator = (LinphoneAccountCreator *)linphone_xml_rpc_request_get_user_data(request);
	LinphoneAccountCreatorStatus status = LinphoneAccountCreatorStatusRequestFailed;

	const char *resp = linphone_xml_rpc_request_get_string_response(request);
	if (linphone_xml_rpc_request_get_status(request) == LinphoneXmlRpcStatusOk) {
		status = (strcmp(resp, "OK") == 0)                             ? LinphoneAccountCreatorStatusAccountCreated
		         : (strcmp(resp, "ERROR_CANNOT_SEND_SMS") == 0)        ? LinphoneAccountCreatorStatusPhoneNumberInvalid
		         : (strcmp(resp, "ERROR_MAX_SMS_EXCEEDED") == 0)       ? LinphoneAccountCreatorStatusPhoneNumberOverused
		         : (strcmp(resp, "ERROR_ACCOUNT_ALREADY_IN_USE") == 0) ? LinphoneAccountCreatorStatusAccountExist
		         : (strcmp(resp, "ERROR_ALIAS_ALREADY_IN_USE") == 0) ? LinphoneAccountCreatorStatusAccountExistWithAlias
		         : (strcmp(resp, "ERROR_ALGO_NOT_SUPPORTED") == 0)   ? LinphoneAccountCreatorStatusAlgoNotSupported
		                                                             : LinphoneAccountCreatorStatusAccountNotCreated;
	}

	if (creator->cbs->create_account_response_cb != NULL) {
		creator->cbs->create_account_response_cb(creator, status, resp);
	}
	NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator, status, resp)
}

static LinphoneXmlRpcRequest *_create_account_with_phone_custom(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request;
	if (!creator->phone_number) {
		return NULL;
	}
	ms_debug(
	    "Account creator: create_account_with_phone (phone number=%s, username=%s, domain=%s, language=%s, algo=%s)",
	    creator->phone_number, (creator->username) ? creator->username : creator->phone_number,
	    linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator), creator->language,
	    creator->algorithm);

	request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "create_phone_account");
	linphone_xml_rpc_request_add_string_arg(request, creator->phone_number);
	linphone_xml_rpc_request_add_string_arg(request, creator->username ? creator->username : creator->phone_number);
	std::string ha1 = Utils::computeHa1ForAlgorithm(
	    L_C_TO_STRING(creator->username ? creator->username : creator->phone_number), L_C_TO_STRING(creator->password),
	    L_C_TO_STRING(linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator)),
	    L_C_TO_STRING(creator->algorithm));
	linphone_xml_rpc_request_add_string_arg(request, creator->password ? ha1.c_str() : "");
	linphone_xml_rpc_request_add_string_arg(request, linphone_core_get_user_agent(creator->core));
	linphone_xml_rpc_request_add_string_arg(request,
	                                        linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator));
	linphone_xml_rpc_request_add_string_arg(request, creator->language);
	linphone_xml_rpc_request_add_string_arg(request, creator->algorithm);
	return request;
}

static LinphoneXmlRpcRequest *_create_account_with_email_custom(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request;
	if (!creator->username || !creator->email || !creator->password) {
		return NULL;
	}
	ms_debug("Account creator: create_account_with_email (username=%s, email=%s, domain=%s, algo=%s)",
	         creator->username, creator->email,
	         linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator), creator->algorithm);

	request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "create_email_account");
	linphone_xml_rpc_request_add_string_arg(request, creator->username);
	linphone_xml_rpc_request_add_string_arg(request, creator->email);
	std::string ha1 = Utils::computeHa1ForAlgorithm(
	    L_C_TO_STRING(creator->username ? creator->username : creator->phone_number), L_C_TO_STRING(creator->password),
	    L_C_TO_STRING(linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator)),
	    L_C_TO_STRING(creator->algorithm));
	linphone_xml_rpc_request_add_string_arg(request, ha1.c_str());
	linphone_xml_rpc_request_add_string_arg(request, linphone_core_get_user_agent(creator->core));
	linphone_xml_rpc_request_add_string_arg(request,
	                                        linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator));
	linphone_xml_rpc_request_add_string_arg(request, creator->algorithm);
	return request;
}

LinphoneAccountCreatorStatus linphone_account_creator_create_account_linphone_xmlrpc(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request = NULL;
	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);
	char *identity = linphone_account_creator_get_identity(creator);

	if (!identity || (!(request = _create_account_with_phone_custom(creator)) &&
	                  !(request = _create_account_with_email_custom(creator)))) {
		if (creator->cbs->create_account_response_cb != NULL) {
			creator->cbs->create_account_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments,
			                                         "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(create_account, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		if (identity) ms_free(identity);
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	LinphoneXmlRpcSession *xmlrpc_session =
	    (LinphoneXmlRpcSession *)belle_sip_object_data_get(BELLE_SIP_OBJECT(creator), "xmlrpc_session");

	if (xmlrpc_session) {
		linphone_xml_rpc_request_set_user_data(request, creator);
		linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request),
		                                          _create_account_cb_custom);
		linphone_xml_rpc_session_send_request(xmlrpc_session, request);
		linphone_xml_rpc_request_unref(request);
		ms_free(identity);
		return LinphoneAccountCreatorStatusRequestOk;
	}

	ms_free(identity);
	return LinphoneAccountCreatorStatusRequestFailed;
}

static void _delete_linphone_account_response_cb(LinphoneXmlRpcRequest *request) {
	LinphoneAccountCreator *creator = (LinphoneAccountCreator *)linphone_xml_rpc_request_get_user_data(request);
	LinphoneAccountCreatorStatus status = LinphoneAccountCreatorStatusRequestFailed;

	const char *resp = linphone_xml_rpc_request_get_string_response(request);
	if (linphone_xml_rpc_request_get_status(request) == LinphoneXmlRpcStatusOk) {
		status = (strcmp(resp, "OK") == 0) ? LinphoneAccountCreatorStatusAccountCreated
		         : (strcmp(resp, "ERROR_USERNAME_PARAMETER_NOT_FOUND") == 0)
		             ? LinphoneAccountCreatorStatusMissingArguments
		         : (strcmp(resp, "ERROR_ACCOUNT_DOESNT_EXIST") == 0) ? LinphoneAccountCreatorStatusAccountNotExist
		         : (strcmp(resp, "ERROR_ALGO_NOT_SUPPORTED") == 0)   ? LinphoneAccountCreatorStatusAlgoNotSupported
		         : (strstr(resp, "ERROR_") == 0)                     ? LinphoneAccountCreatorStatusRequestFailed
		                                                             : LinphoneAccountCreatorStatusAccountNotCreated;
	}

	if (creator->cbs->delete_account_response_cb != NULL) {
		creator->cbs->delete_account_response_cb(creator, status, resp);
	}
	NOTIFY_IF_EXIST_ACCOUNT_CREATOR(delete_account, creator, status, resp)
}

LinphoneAccountCreatorStatus linphone_account_creator_delete_account_linphone_xmlrpc(LinphoneAccountCreator *creator) {
	if ((!creator->username && !creator->phone_number) || !creator->password || !creator->account) {
		if (creator->cbs->delete_account_response_cb != NULL) {
			creator->cbs->delete_account_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments,
			                                         "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(delete_account, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);
	const LinphoneAccountParams *account_params = linphone_account_get_params(creator->account);
	ms_debug("Account creator: delete_account (username=%s, password=%s, domain=%s, algo=%s)", creator->username,
	         creator->password, linphone_account_params_get_domain(account_params), creator->algorithm);

	LinphoneXmlRpcSession *xmlrpc_session =
	    (LinphoneXmlRpcSession *)belle_sip_object_data_get(BELLE_SIP_OBJECT(creator), "xmlrpc_session");

	LinphoneXmlRpcRequest *request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "delete_account");
	linphone_xml_rpc_request_add_string_arg(request, creator->username ? creator->username : creator->phone_number);
	linphone_xml_rpc_request_add_string_arg(request, creator->password);
	linphone_xml_rpc_request_add_string_arg(request, linphone_account_params_get_domain(account_params));
	linphone_xml_rpc_request_add_string_arg(request, creator->algorithm);
	linphone_xml_rpc_request_set_user_data(request, creator);
	linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request),
	                                          _delete_linphone_account_response_cb);
	linphone_xml_rpc_session_send_request(xmlrpc_session, request);
	linphone_xml_rpc_request_unref(request);

	return LinphoneAccountCreatorStatusRequestOk;
}

/****************** END OF CREATE ACCOUNT SECTION *****************************/

/****************** START OF VALIDATE ACCOUNT SECTION *************************/

static void _login_account_confirmation_key_cb_custom(LinphoneXmlRpcRequest *request) {
	LinphoneAccountCreator *creator = (LinphoneAccountCreator *)linphone_xml_rpc_request_get_user_data(request);
	LinphoneAccountCreatorStatus status = LinphoneAccountCreatorStatusRequestFailed;

	if (linphone_xml_rpc_request_get_status(request) == LinphoneXmlRpcStatusFailed) {
		const char *resp = linphone_xml_rpc_request_get_string_response(request);
		if (resp != NULL) {
			if (strcmp(resp, "ERROR_ACCOUNT_DOESNT_EXIST") == 0) {
				status = LinphoneAccountCreatorStatusAccountNotExist;
			} else if (strcmp(resp, "ERROR_KEY_DOESNT_MATCH") == 0) {
				status = LinphoneAccountCreatorStatusWrongActivationCode;
			} else if (strcmp(resp, "ERROR_ALGO_NOT_SUPPORTED") == 0) {
				status = LinphoneAccountCreatorStatusAlgoNotSupported;
			} else {
				status = LinphoneAccountCreatorStatusUnexpectedError;
			}
		}
	} else if (linphone_xml_rpc_request_get_status(request) == LinphoneXmlRpcStatusOk) {
		status = LinphoneAccountCreatorStatusRequestOk;
		/*const bctbx_map_t *resp = linphone_xml_rpc_request_get_string_struct_response(request);
		const auto &it = bctbx_map_cchar_find_key(resp, "password");
		if (!bctbx_iterator_equals(it, bctbx_map_cchar_end(resp))) {
		    const char *ha1 = (const char *)bctbx_pair_cchar_get_second(bctbx_iterator_cchar_get_pair(it));
		    set_string(&creator->ha1, ha1, FALSE);
		}
		const auto &it2 = bctbx_map_cchar_find_key(resp, "algorithm");
		if (!bctbx_iterator_equals(it2, bctbx_map_cchar_end(resp))) {
		    const char *algo = (const char *)bctbx_pair_cchar_get_second(bctbx_iterator_cchar_get_pair(it2));
		    if (algo) {
		        if (creator->algorithm) {
		            if (strcmp(algo, creator->algorithm) != 0) {
		                ms_warning("Asked for password hashed using %s, got algorithm %s", creator->algorithm, algo);
		            } else {
		                ms_debug("Got password hashed using %s that we requested", algo);
		            }
		        } else {
		            ms_debug("Account creator wasn't configured for a specific alogithm, got %s", algo);
		        }
		        set_string(&creator->algorithm, algo, FALSE);
		    } else {
		        ms_error("Couldn't get algorithm from struct response !");
		    }
		}*/
		const bctbx_list_t *list = linphone_xml_rpc_request_get_list_response(request);
		const char *ha1 = (char *)bctbx_list_get_data(list);
		set_string(&creator->ha1, ha1, FALSE);
		if (bctbx_list_size(list) > 1) {
			const char *algo = (char *)bctbx_list_get_data(bctbx_list_next(list));
			if (algo) {
				if (creator->algorithm) {
					if (strcmp(algo, creator->algorithm) != 0) {
						ms_warning("Asked for password hashed using %s, got algorithm %s", creator->algorithm, algo);
					} else {
						ms_debug("Got password hashed using %s that we requested", algo);
					}
				} else {
					ms_debug("Account creator wasn't configured for a specific alogithm, got %s", algo);
				}
				set_string(&creator->algorithm, algo, FALSE);
			} else {
				ms_error("Couldn't get algorithm from struct response !");
			}
		}
	}

	const char *content = linphone_xml_rpc_request_get_content(request);
	if (creator->cbs->login_linphone_account_response_cb != NULL) {
		creator->cbs->login_linphone_account_response_cb(creator, status, content);
	}
	NOTIFY_IF_EXIST_ACCOUNT_CREATOR(login_linphone_account, creator, status, content)
}

LinphoneAccountCreatorStatus
linphone_account_creator_login_linphone_account_linphone_xmlrpc(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request = NULL;
	if ((!creator->username && !creator->phone_number) || !creator->activation_code) {
		if (creator->cbs->login_linphone_account_response_cb != NULL) {
			creator->cbs->login_linphone_account_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments,
			                                                 "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(login_linphone_account, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	LinphoneXmlRpcSession *xmlrpc_session =
	    (LinphoneXmlRpcSession *)belle_sip_object_data_get(BELLE_SIP_OBJECT(creator), "xmlrpc_session");

	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);
	if (xmlrpc_session) {
		ms_debug("Account creator: recover_account_from_confirmation_key (username=%s, activation code=%s, domain=%s, "
		         "algo=%s)",
		         creator->username ? creator->username : creator->phone_number, creator->activation_code,
		         linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator), creator->algorithm);

		request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgStringStruct, "recover_account_from_confirmation_key");
		linphone_xml_rpc_request_add_string_arg(request, creator->username ? creator->username : creator->phone_number);
		linphone_xml_rpc_request_add_string_arg(request, creator->activation_code);
		linphone_xml_rpc_request_add_string_arg(
		    request, linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator));
		linphone_xml_rpc_request_add_string_arg(request, creator->algorithm);
		linphone_xml_rpc_request_set_user_data(request, creator);
		linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request),
		                                          _login_account_confirmation_key_cb_custom);
		linphone_xml_rpc_session_send_request(xmlrpc_session, request);
		linphone_xml_rpc_request_unref(request);
		return LinphoneAccountCreatorStatusRequestOk;
	}

	return LinphoneAccountCreatorStatusRequestFailed;
}

static void _activate_account_cb_custom(LinphoneXmlRpcRequest *request) {
	LinphoneAccountCreator *creator = (LinphoneAccountCreator *)linphone_xml_rpc_request_get_user_data(request);
	LinphoneAccountCreatorStatus status = LinphoneAccountCreatorStatusRequestFailed;

	const char *resp = linphone_xml_rpc_request_get_string_response(request);
	if (linphone_xml_rpc_request_get_status(request) == LinphoneXmlRpcStatusOk) {
		if (strcmp(resp, "ERROR_ACCOUNT_ALREADY_ACTIVATED") == 0) {
			status = LinphoneAccountCreatorStatusAccountAlreadyActivated;
		} else if (strcmp(resp, "ERROR_KEY_DOESNT_MATCH") == 0) {
			status = LinphoneAccountCreatorStatusWrongActivationCode;
		} else if (strcmp(resp, "ERROR_ALGO_NOT_SUPPORTED") == 0) {
			status = LinphoneAccountCreatorStatusAlgoNotSupported;
		} else if (strcmp(resp, "ERROR_ACCOUNT_DOESNT_EXIST") == 0) {
			status = LinphoneAccountCreatorStatusAccountNotExist;
		} else if (strstr(resp, "ERROR_") == resp) {
			status = LinphoneAccountCreatorStatusAccountNotActivated;
		} else {
			status = LinphoneAccountCreatorStatusAccountActivated;
			set_string(&creator->ha1, resp, FALSE);
		}
	}

	if (creator->cbs->activate_account_response_cb != NULL) {
		creator->cbs->activate_account_response_cb(creator, status, resp);
	}
	NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_account, creator, status, resp)
}

LinphoneAccountCreatorStatus
linphone_account_creator_activate_phone_account_linphone_xmlrpc(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request = NULL;

	if (!creator->phone_number || !creator->activation_code) {
		if (creator->cbs->is_account_activated_response_cb != NULL) {
			creator->cbs->is_account_activated_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments,
			                                               "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_account, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	LinphoneXmlRpcSession *xmlrpc_session =
	    (LinphoneXmlRpcSession *)belle_sip_object_data_get(BELLE_SIP_OBJECT(creator), "xmlrpc_session");

	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);
	if (xmlrpc_session) {
		ms_debug("Account creator: activate_account_phone (phone number=%s, username=%s, activation code=%s, "
		         "domain=%s, algo=%s)",
		         creator->phone_number, creator->username ? creator->username : creator->phone_number,
		         creator->activation_code, linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator),
		         creator->algorithm);

		request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "activate_phone_account");
		linphone_xml_rpc_request_add_string_arg(request, creator->phone_number);
		linphone_xml_rpc_request_add_string_arg(request, creator->username ? creator->username : creator->phone_number);
		linphone_xml_rpc_request_add_string_arg(request, creator->activation_code);
		linphone_xml_rpc_request_add_string_arg(
		    request, linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator));
		linphone_xml_rpc_request_add_string_arg(request, creator->algorithm);
		linphone_xml_rpc_request_set_user_data(request, creator);
		linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request),
		                                          _activate_account_cb_custom);
		linphone_xml_rpc_session_send_request(xmlrpc_session, request);
		linphone_xml_rpc_request_unref(request);
		return LinphoneAccountCreatorStatusRequestOk;
	}

	return LinphoneAccountCreatorStatusRequestFailed;
}

LinphoneAccountCreatorStatus
linphone_account_creator_activate_email_account_linphone_xmlrpc(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request = NULL;
	if (!creator->activation_code || !creator->username) {
		if (creator->cbs->is_account_activated_response_cb != NULL) {
			creator->cbs->is_account_activated_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments,
			                                               "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_account, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	LinphoneXmlRpcSession *xmlrpc_session =
	    (LinphoneXmlRpcSession *)belle_sip_object_data_get(BELLE_SIP_OBJECT(creator), "xmlrpc_session");

	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);
	if (xmlrpc_session) {
		ms_debug("Account creator: activate_account_email (username=%s, activation code=%s, domain=%s, algo=%s)",
		         creator->username, creator->activation_code,
		         linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator), creator->algorithm);

		request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "activate_email_account");
		linphone_xml_rpc_request_add_string_arg(request, creator->username);
		linphone_xml_rpc_request_add_string_arg(request, creator->activation_code);
		linphone_xml_rpc_request_add_string_arg(
		    request, linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator));
		linphone_xml_rpc_request_add_string_arg(request, creator->algorithm);
		linphone_xml_rpc_request_set_user_data(request, creator);
		linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request),
		                                          _activate_account_cb_custom);
		linphone_xml_rpc_session_send_request(xmlrpc_session, request);
		linphone_xml_rpc_request_unref(request);
		return LinphoneAccountCreatorStatusRequestOk;
	}

	return LinphoneAccountCreatorStatusRequestFailed;
}

static void get_linphone_confirmation_key_response_cb(LinphoneXmlRpcRequest *request) {
	LinphoneAccountCreator *creator = (LinphoneAccountCreator *)linphone_xml_rpc_request_get_user_data(request);
	LinphoneAccountCreatorStatus status = LinphoneAccountCreatorStatusRequestFailed;

	const char *resp = linphone_xml_rpc_request_get_string_response(request);
	if (linphone_xml_rpc_request_get_status(request) == LinphoneXmlRpcStatusOk) {
		if (strcmp(resp, "ERROR_USERNAME_PARAMETER_NOT_FOUND") == 0) {
			status = LinphoneAccountCreatorStatusMissingArguments;
		} else if (strcmp(resp, "ERROR_ACCOUNT_DOESNT_EXIST") == 0) {
			status = LinphoneAccountCreatorStatusAccountNotExist;
		} else if (strcmp(resp, "ERROR_ALGO_NOT_SUPPORTED") == 0) {
			status = LinphoneAccountCreatorStatusAlgoNotSupported;
		} else if (strstr(resp, "ERROR_") == resp) {
			status = LinphoneAccountCreatorStatusRequestFailed;
		} else {
			status = LinphoneAccountCreatorStatusRequestOk;
			set_string(&creator->activation_code, resp, FALSE);
		}
	}

	if (creator->cbs->confirmation_key_response_cb != NULL) {
		creator->cbs->confirmation_key_response_cb(creator, status, resp);
	}
	NOTIFY_IF_EXIST_ACCOUNT_CREATOR(confirmation_key, creator, status, resp)
}

LinphoneAccountCreatorStatus
linphone_account_creator_get_confirmation_key_linphone_xmlrpc(LinphoneAccountCreator *creator) {
	if (!creator->username || !creator->password || !creator->account) {
		if (creator->cbs->confirmation_key_response_cb != NULL) {
			creator->cbs->confirmation_key_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments,
			                                           "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(confirmation_key, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);
	const LinphoneAccountParams *account_params = linphone_account_get_params(creator->account);
	ms_debug("Account creator: confirmation_key (username=%s, password=%s, domain=%s, algo=%s)", creator->username,
	         creator->password, linphone_account_params_get_domain(account_params), creator->algorithm);

	LinphoneXmlRpcSession *xmlrpc_session =
	    (LinphoneXmlRpcSession *)belle_sip_object_data_get(BELLE_SIP_OBJECT(creator), "xmlrpc_session");

	LinphoneXmlRpcRequest *request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "get_confirmation_key");
	linphone_xml_rpc_request_add_string_arg(request, creator->username);
	std::string ha1 = Utils::computeHa1ForAlgorithm(L_C_TO_STRING(creator->username), L_C_TO_STRING(creator->password),
	                                                L_C_TO_STRING(linphone_account_params_get_domain(account_params)),
	                                                L_C_TO_STRING(creator->algorithm));
	linphone_xml_rpc_request_add_string_arg(request, ha1.c_str());
	linphone_xml_rpc_request_add_string_arg(request, linphone_account_params_get_domain(account_params));
	linphone_xml_rpc_request_add_string_arg(request, creator->algorithm);
	linphone_xml_rpc_request_set_user_data(request, creator);
	linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request),
	                                          get_linphone_confirmation_key_response_cb);
	linphone_xml_rpc_session_send_request(xmlrpc_session, request);
	linphone_xml_rpc_request_unref(request);
	return LinphoneAccountCreatorStatusRequestOk;
}
/****************** END OF CREATE VALIDATE ACCOUNT SECTION ********************/

/****************** START OF ACCOUNT VALIDATED SECTION ************************/
static void _is_account_activated_cb_custom(LinphoneXmlRpcRequest *request) {
	LinphoneAccountCreator *creator = (LinphoneAccountCreator *)linphone_xml_rpc_request_get_user_data(request);
	LinphoneAccountCreatorStatus status = LinphoneAccountCreatorStatusRequestFailed;

	const char *resp = linphone_xml_rpc_request_get_string_response(request);
	if (linphone_xml_rpc_request_get_status(request) == LinphoneXmlRpcStatusOk) {
		status = (strcmp(resp, "OK") == 0) ? LinphoneAccountCreatorStatusAccountActivated
		                                   : LinphoneAccountCreatorStatusAccountNotActivated;
	}

	if (creator->cbs->is_account_activated_response_cb != NULL) {
		creator->cbs->is_account_activated_response_cb(creator, status, resp);
	}
	NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_activated, creator, status, resp)
}

LinphoneAccountCreatorStatus
linphone_account_creator_is_account_activated_linphone_xmlrpc(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request = NULL;
	char *identity = linphone_account_creator_get_identity(creator);
	if (!identity) {
		if (creator->cbs->is_account_activated_response_cb != NULL) {
			creator->cbs->is_account_activated_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments,
			                                               "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_activated, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	LinphoneXmlRpcSession *xmlrpc_session =
	    (LinphoneXmlRpcSession *)belle_sip_object_data_get(BELLE_SIP_OBJECT(creator), "xmlrpc_session");

	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);
	if (xmlrpc_session) {
		ms_debug("Account creator: is_account_activated (username=%s, domain=%s, algo=%s)",
		         creator->username ? creator->username : creator->phone_number,
		         linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator), creator->algorithm);

		request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "is_account_activated");
		linphone_xml_rpc_request_add_string_arg(request, creator->username ? creator->username : creator->phone_number);
		linphone_xml_rpc_request_add_string_arg(
		    request, linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator));
		linphone_xml_rpc_request_add_string_arg(request, creator->algorithm);
		linphone_xml_rpc_request_set_user_data(request, creator);
		linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request),
		                                          _is_account_activated_cb_custom);
		linphone_xml_rpc_session_send_request(xmlrpc_session, request);
		linphone_xml_rpc_request_unref(request);
		ms_free(identity);
		return LinphoneAccountCreatorStatusRequestOk;
	}

	ms_free(identity);
	return LinphoneAccountCreatorStatusRequestFailed;
}
/****************** END OF CREATE ACCOUNT VALIDATED SECTION********************/

/****************** START OF PHONE NUMBER VALIDATED SECTION *******************/

static void _is_phone_number_used_cb_custom(LinphoneXmlRpcRequest *request) {
	LinphoneAccountCreator *creator = (LinphoneAccountCreator *)linphone_xml_rpc_request_get_user_data(request);
	LinphoneAccountCreatorStatus status = LinphoneAccountCreatorStatusRequestFailed;

	const char *resp = linphone_xml_rpc_request_get_string_response(request);
	if (linphone_xml_rpc_request_get_status(request) == LinphoneXmlRpcStatusOk) {
		status = (strcmp(resp, "OK_ACCOUNT") == 0) ? LinphoneAccountCreatorStatusAliasIsAccount
		         : (strcmp(resp, "OK_ALIAS") == 0) ? LinphoneAccountCreatorStatusAliasExist
		                                           : LinphoneAccountCreatorStatusAliasNotExist;
	}

	if (creator->cbs->is_alias_used_response_cb != NULL) {
		creator->cbs->is_alias_used_response_cb(creator, status, resp);
	}
	NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_alias_used, creator, status, resp)
}

LinphoneAccountCreatorStatus
linphone_account_creator_is_phone_number_used_linphone_xmlrpc(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request = NULL;
	char *identity = linphone_account_creator_get_identity(creator);
	if (!identity) {
		if (creator->cbs->is_alias_used_response_cb != NULL) {
			creator->cbs->is_alias_used_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments,
			                                        "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_alias_used, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	LinphoneXmlRpcSession *xmlrpc_session =
	    (LinphoneXmlRpcSession *)belle_sip_object_data_get(BELLE_SIP_OBJECT(creator), "xmlrpc_session");

	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);
	if (xmlrpc_session) {
		ms_debug("Account creator: is_phone_number_used (phone number=%s, domain=%s)", creator->phone_number,
		         linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator));

		request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "is_phone_number_used");
		linphone_xml_rpc_request_add_string_arg(request, creator->phone_number);
		linphone_xml_rpc_request_add_string_arg(
		    request, linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator));
		linphone_xml_rpc_request_set_user_data(request, creator);
		linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request),
		                                          _is_phone_number_used_cb_custom);
		linphone_xml_rpc_session_send_request(xmlrpc_session, request);
		linphone_xml_rpc_request_unref(request);
		ms_free(identity);
		return LinphoneAccountCreatorStatusRequestOk;
	}

	ms_free(identity);
	return LinphoneAccountCreatorStatusRequestFailed;
}

/****************** END OF PHONE NUMBER VALIDATED SECTION *********************/

/****************** START OF LINK PHONE NUMBER WITH ACCOUNT SECTION ***********/
static void _link_phone_number_with_account_cb_custom(LinphoneXmlRpcRequest *request) {
	LinphoneAccountCreator *creator = (LinphoneAccountCreator *)linphone_xml_rpc_request_get_user_data(request);
	LinphoneAccountCreatorStatus status = LinphoneAccountCreatorStatusRequestFailed;

	const char *resp = linphone_xml_rpc_request_get_string_response(request);
	if (linphone_xml_rpc_request_get_status(request) == LinphoneXmlRpcStatusOk) {
		status = (strcmp(resp, "OK") == 0)                       ? LinphoneAccountCreatorStatusRequestOk
		         : (strcmp(resp, "ERROR_CANNOT_SEND_SMS") == 0)  ? LinphoneAccountCreatorStatusPhoneNumberInvalid
		         : (strcmp(resp, "ERROR_MAX_SMS_EXCEEDED") == 0) ? LinphoneAccountCreatorStatusPhoneNumberOverused
		                                                         : LinphoneAccountCreatorStatusAccountNotLinked;
	}

	if (creator->cbs->link_account_response_cb != NULL) {
		creator->cbs->link_account_response_cb(creator, status, resp);
	}
	NOTIFY_IF_EXIST_ACCOUNT_CREATOR(link_account, creator, status, resp)
}

LinphoneAccountCreatorStatus
linphone_account_creator_link_phone_number_with_account_linphone_xmlrpc(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request = NULL;
	if (!creator->phone_number || !creator->username) {
		if (creator->cbs->link_account_response_cb != NULL) {
			creator->cbs->link_account_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments,
			                                       "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(link_account, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	LinphoneXmlRpcSession *xmlrpc_session =
	    (LinphoneXmlRpcSession *)belle_sip_object_data_get(BELLE_SIP_OBJECT(creator), "xmlrpc_session");

	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);
	if (xmlrpc_session) {
		ms_debug(
		    "Account creator: link_phone_number_with_account (phone number=%s, username=%s, domain=%s, language=%s)",
		    creator->phone_number, creator->username,
		    linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator), creator->language);

		request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "link_phone_number_with_account");
		linphone_xml_rpc_request_add_string_arg(request, creator->phone_number);
		linphone_xml_rpc_request_add_string_arg(request, creator->username);
		linphone_xml_rpc_request_add_string_arg(
		    request, linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator));
		linphone_xml_rpc_request_add_string_arg(request, creator->language);
		linphone_xml_rpc_request_set_user_data(request, creator);
		linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request),
		                                          _link_phone_number_with_account_cb_custom);
		linphone_xml_rpc_session_send_request(xmlrpc_session, request);
		linphone_xml_rpc_request_unref(request);
		return LinphoneAccountCreatorStatusRequestOk;
	}

	return LinphoneAccountCreatorStatusRequestFailed;
}

static void _get_phone_number_for_account_cb_custom(LinphoneXmlRpcRequest *request) {
	LinphoneAccountCreator *creator = (LinphoneAccountCreator *)linphone_xml_rpc_request_get_user_data(request);
	LinphoneAccountCreatorStatus status = LinphoneAccountCreatorStatusRequestFailed;

	const char *resp = linphone_xml_rpc_request_get_string_response(request);
	if (linphone_xml_rpc_request_get_status(request) == LinphoneXmlRpcStatusOk) {
		status = (strcmp(resp, "ERROR_USERNAME_PARAMETER_NOT_FOUND") == 0 ||
		          strcmp(resp, "ERROR_ACCOUNT_DOESNT_EXIST") == 0 || strcmp(resp, "ERROR_ALIAS_DOESNT_EXIST") == 0)
		             ? LinphoneAccountCreatorStatusAccountNotLinked
		             : LinphoneAccountCreatorStatusAccountLinked;
	}

	if (creator->cbs->is_account_linked_response_cb != NULL) {
		creator->cbs->is_account_linked_response_cb(creator, status, resp);
	}
	NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_linked, creator, status, resp)
}

LinphoneAccountCreatorStatus
linphone_account_creator_is_account_linked_linphone_xmlrpc(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request = NULL;
	if (!creator->username || !linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator)) {
		if (creator->cbs->is_account_linked_response_cb != NULL) {
			creator->cbs->is_account_linked_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments,
			                                            "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(is_account_linked, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	LinphoneXmlRpcSession *xmlrpc_session =
	    (LinphoneXmlRpcSession *)belle_sip_object_data_get(BELLE_SIP_OBJECT(creator), "xmlrpc_session");

	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);
	if (xmlrpc_session) {
		ms_debug("Account creator: is_account_linked (username=%s, domain=%s)", creator->username,
		         linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator));

		request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "get_phone_number_for_account");
		linphone_xml_rpc_request_add_string_arg(request, creator->username);
		linphone_xml_rpc_request_add_string_arg(
		    request, linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator));
		linphone_xml_rpc_request_set_user_data(request, creator);
		linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request),
		                                          _get_phone_number_for_account_cb_custom);
		linphone_xml_rpc_session_send_request(xmlrpc_session, request);
		linphone_xml_rpc_request_unref(request);
		return LinphoneAccountCreatorStatusRequestOk;
	}

	return LinphoneAccountCreatorStatusRequestFailed;
}
/****************** END OF LINK PHONE NUMBER WITH ACCOUNT SECTION *************/

/****************** START OF ACTIVE PHONE NUMBER LINK **************************/
static void _activate_phone_number_link_cb_custom(LinphoneXmlRpcRequest *request) {
	LinphoneAccountCreator *creator = (LinphoneAccountCreator *)linphone_xml_rpc_request_get_user_data(request);
	LinphoneAccountCreatorStatus status = LinphoneAccountCreatorStatusRequestFailed;

	const char *resp = linphone_xml_rpc_request_get_string_response(request);
	if (linphone_xml_rpc_request_get_status(request) == LinphoneXmlRpcStatusOk) {
		status = (strstr(resp, "ERROR_") == resp) ? LinphoneAccountCreatorStatusAccountNotActivated
		                                          : LinphoneAccountCreatorStatusAccountActivated;
	}

	if (creator->cbs->activate_alias_response_cb != NULL) {
		creator->cbs->activate_alias_response_cb(creator, status, resp);
	}
	NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_alias, creator, status, resp)
}

LinphoneAccountCreatorStatus
linphone_account_creator_activate_phone_number_link_linphone_xmlrpc(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request = NULL;
	if (!creator->phone_number || !creator->username || !creator->activation_code ||
	    (!creator->password && !creator->ha1) ||
	    !linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator)) {
		if (creator->cbs->activate_alias_response_cb != NULL) {
			creator->cbs->activate_alias_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments,
			                                         "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(activate_alias, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	LinphoneXmlRpcSession *xmlrpc_session =
	    (LinphoneXmlRpcSession *)belle_sip_object_data_get(BELLE_SIP_OBJECT(creator), "xmlrpc_session");

	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);
	if (xmlrpc_session) {
		ms_debug("Account creator: activate_phone_number_link (phone number=%s, username=%s, activation code=%s, "
		         "domain=%s, algo=%s)",
		         creator->phone_number, creator->username, creator->activation_code,
		         linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator), creator->algorithm);

		request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "activate_phone_number_link");
		linphone_xml_rpc_request_add_string_arg(request, creator->phone_number);
		linphone_xml_rpc_request_add_string_arg(request, creator->username);
		linphone_xml_rpc_request_add_string_arg(request, creator->activation_code);
		std::string ha1 = Utils::computeHa1ForAlgorithm(
		    L_C_TO_STRING(creator->username), L_C_TO_STRING(creator->password),
		    L_C_TO_STRING(linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator)),
		    L_C_TO_STRING(creator->algorithm));
		linphone_xml_rpc_request_add_string_arg(request, creator->ha1 ? creator->ha1 : ha1.c_str());
		linphone_xml_rpc_request_add_string_arg(
		    request, linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator));
		linphone_xml_rpc_request_add_string_arg(request, creator->algorithm);
		linphone_xml_rpc_request_set_user_data(request, creator);
		linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request),
		                                          _activate_phone_number_link_cb_custom);
		linphone_xml_rpc_session_send_request(xmlrpc_session, request);
		linphone_xml_rpc_request_unref(request);
		return LinphoneAccountCreatorStatusRequestOk;
	}

	return LinphoneAccountCreatorStatusRequestFailed;
}
/****************** END OF ACTIVE PHONE NUMBER LINK **************************/

/****************** START OF ACTIVE PHONE NUMBER LINK **************************/
static void _recover_phone_account_cb_custom(LinphoneXmlRpcRequest *request) {
	LinphoneAccountCreator *creator = (LinphoneAccountCreator *)linphone_xml_rpc_request_get_user_data(request);
	LinphoneAccountCreatorStatus status = LinphoneAccountCreatorStatusRequestFailed;

	const char *resp = linphone_xml_rpc_request_get_string_response(request);
	if (linphone_xml_rpc_request_get_status(request) == LinphoneXmlRpcStatusOk) {
		if (strstr(resp, "ERROR_") == resp) {
			status = (strcmp(resp, "ERROR_CANNOT_SEND_SMS") == 0)    ? LinphoneAccountCreatorStatusPhoneNumberInvalid
			         : (strcmp(resp, "ERROR_MAX_SMS_EXCEEDED") == 0) ? LinphoneAccountCreatorStatusPhoneNumberOverused
			         : (strcmp(resp, "ERROR_ACCOUNT_DOESNT_EXIST") == 0) ? LinphoneAccountCreatorStatusAccountNotExist
			                                                             : LinphoneAccountCreatorStatusRequestFailed;
		} else {
			status = LinphoneAccountCreatorStatusRequestOk;
			set_string(&creator->username, resp, FALSE);
		}
	}

	if (creator->cbs->recover_account_response_cb != NULL) {
		creator->cbs->recover_account_response_cb(creator, status, resp);
	}
	NOTIFY_IF_EXIST_ACCOUNT_CREATOR(recover_account, creator, status, resp)
}

LinphoneAccountCreatorStatus
linphone_account_creator_recover_phone_account_linphone_xmlrpc(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request = NULL;
	if (!creator->phone_number) {
		if (creator->cbs->recover_account_response_cb != NULL) {
			creator->cbs->recover_account_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments,
			                                          "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(recover_account, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	LinphoneXmlRpcSession *xmlrpc_session =
	    (LinphoneXmlRpcSession *)belle_sip_object_data_get(BELLE_SIP_OBJECT(creator), "xmlrpc_session");

	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);
	if (xmlrpc_session) {
		ms_debug("Account creator: recover_phone_account (phone number=%s, domain=%s, language=%s)",
		         creator->phone_number, linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator),
		         creator->language);

		request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "recover_phone_account");
		linphone_xml_rpc_request_add_string_arg(request, creator->phone_number);
		linphone_xml_rpc_request_add_string_arg(
		    request, linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator));
		linphone_xml_rpc_request_add_string_arg(request, creator->language);
		linphone_xml_rpc_request_set_user_data(request, creator);
		linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request),
		                                          _recover_phone_account_cb_custom);
		linphone_xml_rpc_session_send_request(xmlrpc_session, request);
		linphone_xml_rpc_request_unref(request);
		return LinphoneAccountCreatorStatusRequestOk;
	}

	return LinphoneAccountCreatorStatusRequestFailed;
}
/****************** END OF ACTIVE PHONE NUMBER LINK **************************/

/****************** START OF UPDATE ACCOUNT **************************/
static void _password_updated_cb_custom(LinphoneXmlRpcRequest *request) {
	LinphoneAccountCreator *creator = (LinphoneAccountCreator *)linphone_xml_rpc_request_get_user_data(request);
	LinphoneAccountCreatorStatus status = LinphoneAccountCreatorStatusRequestFailed;

	const char *resp = linphone_xml_rpc_request_get_string_response(request);
	if (linphone_xml_rpc_request_get_status(request) == LinphoneXmlRpcStatusOk) {
		if (strcmp(resp, "OK") == 0) {
			status = LinphoneAccountCreatorStatusRequestOk;
		} else if (strcmp(resp, "ERROR_ALGO_NOT_SUPPORTED") == 0) {
			status = LinphoneAccountCreatorStatusAlgoNotSupported;
		} else if (strcmp(resp, "ERROR_PASSWORD_DOESNT_MATCH") == 0) {
			status = LinphoneAccountCreatorStatusAccountNotExist;
		} else {
			status = LinphoneAccountCreatorStatusServerError;
		}
	}

	if (creator->cbs->update_account_response_cb != NULL) {
		creator->cbs->update_account_response_cb(creator, status, resp);
	}
	NOTIFY_IF_EXIST_ACCOUNT_CREATOR(update_account, creator, status, resp)
}

LinphoneAccountCreatorStatus linphone_account_creator_update_password_linphone_xmlrpc(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request = NULL;
	char *identity = linphone_account_creator_get_identity(creator);
	const char *new_pwd = (const char *)linphone_account_creator_get_user_data(creator);
	if (!identity || ((!creator->username && !creator->phone_number) ||
	                  !linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator) ||
	                  (!creator->password && !creator->ha1) || !new_pwd)) {
		if (creator->cbs->update_account_response_cb != NULL) {
			creator->cbs->update_account_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments,
			                                         "Missing required parameters");
		}
		NOTIFY_IF_EXIST_ACCOUNT_CREATOR(update_account, creator, LinphoneAccountCreatorStatusMissingArguments,
		                                "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}
	bctbx_free(identity);

	LinphoneXmlRpcSession *xmlrpc_session =
	    (LinphoneXmlRpcSession *)belle_sip_object_data_get(BELLE_SIP_OBJECT(creator), "xmlrpc_session");

	linphone_account_creator_fill_domain_and_algorithm_if_needed(creator);
	if (xmlrpc_session) {
		const char *username = creator->username ? creator->username : creator->phone_number;
		std::string generated_ha1 = Utils::computeHa1ForAlgorithm(
		    L_C_TO_STRING(username), L_C_TO_STRING(creator->password),
		    L_C_TO_STRING(linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator)),
		    L_C_TO_STRING(creator->algorithm));
		char *ha1 = bctbx_strdup(creator->ha1 ? creator->ha1 : generated_ha1.c_str());
		std::string new_generated_ha1 = Utils::computeHa1ForAlgorithm(
		    L_C_TO_STRING(username), L_C_TO_STRING(new_pwd),
		    L_C_TO_STRING(linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator)),
		    L_C_TO_STRING(creator->algorithm));
		char *new_ha1 = bctbx_strdup(new_generated_ha1.c_str());

		ms_debug("Account creator: update_password (username=%s, domain=%s, algo=%s)", creator->username,
		         linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator), creator->algorithm);

		request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "update_hash");
		linphone_xml_rpc_request_add_string_arg(request, username);
		linphone_xml_rpc_request_add_string_arg(request, ha1);
		linphone_xml_rpc_request_add_string_arg(request, new_ha1);
		linphone_xml_rpc_request_add_string_arg(
		    request, linphone_account_creator_get_domain_with_fallback_to_proxy_domain(creator));
		linphone_xml_rpc_request_add_string_arg(request, creator->algorithm);
		linphone_xml_rpc_request_set_user_data(request, creator);
		linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request),
		                                          _password_updated_cb_custom);
		linphone_xml_rpc_session_send_request(xmlrpc_session, request);
		linphone_xml_rpc_request_unref(request);

		bctbx_free(ha1);
		bctbx_free(new_ha1);

		return LinphoneAccountCreatorStatusRequestOk;
	}

	return LinphoneAccountCreatorStatusRequestFailed;
}
/****************** END OF UPDATE ACCOUNT **************************/

/************************** End Account Creator Linphone **************************/
