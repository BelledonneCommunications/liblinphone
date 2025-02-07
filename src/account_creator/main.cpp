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
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-auth-info.h"
#include "linphone/core.h"
#include "private_functions.h"

// TODO: From coreapi. Remove me later.
#include "private.h"

using namespace LinphonePrivate;

/**
 * Utility functions
 */

static void reset_field(char **field) {
	if (*field) {
		bctbx_free(*field);
		*field = nullptr;
	}
}

static unsigned int validate_uri(LinphoneCore *lc, const char *username, const char *domain, const char *display_name) {
	LinphoneAddress *addr;
	unsigned int status = 0;
	LinphoneAccountParams *params = linphone_account_params_new(lc, TRUE);
	addr = linphone_address_new("sip:?@domain.com");
	linphone_account_params_set_identity_address(params, addr);
	if (addr) linphone_address_unref(addr);
	LinphoneAccount *account = linphone_core_create_account(lc, params);
	linphone_account_params_unref(params);

	if (username) {
		addr = linphone_account_normalize_sip_uri(account, username);
	} else {
		addr =
		    linphone_address_clone(linphone_account_params_get_identity_address(linphone_account_get_params(account)));
	}

	if (addr == NULL) {
		status = 1;
		goto end;
	}

	if (domain && linphone_address_set_domain(addr, domain) != 0) {
		status = 1;
	}

	if (display_name && (!strlen(display_name) || linphone_address_set_display_name(addr, display_name) != 0)) {
		status = 1;
	}
	linphone_address_unref(addr);
end:
	linphone_account_unref(account);
	return status;
}

LinphoneProxyConfig *linphone_account_creator_create_proxy_config(const LinphoneAccountCreator *creator) {
	LinphoneAccount *account = linphone_account_creator_create_account_in_core(creator);
	if (account != nullptr) {
		auto cfg = linphone_proxy_config_ref(linphone_account_get_proxy_config(account));
		linphone_account_unref(account);
		return cfg;
	}
	return nullptr;
}

LinphoneAccount *linphone_account_creator_create_account_in_core(const LinphoneAccountCreator *creator) {
	LinphoneAuthInfo *info;
	LinphoneAccountParams *params = linphone_account_params_new(creator->core, TRUE);
	char *identity_str = linphone_account_creator_get_identity(creator);
	LinphoneAddress *identity = linphone_address_new(identity_str);
	ms_free(identity_str);
	if (creator->display_name) {
		linphone_address_set_display_name(identity, creator->display_name);
	}
	linphone_account_params_set_identity_address(params, identity);
	if (creator->phone_country_code) {
		linphone_account_params_set_international_prefix(params, creator->phone_country_code);
	} else if (creator->phone_number) {
		int dial_prefix_number = DialPlan::lookupCccFromE164(creator->phone_number);
		char buff[4];
		snprintf(buff, sizeof(buff), "%d", dial_prefix_number);
		linphone_account_params_set_international_prefix(params, buff);
	}
	if (linphone_account_params_get_server_addr(params) == NULL && creator->domain != NULL) {
		char *url = ms_strdup_printf("sip:%s", creator->domain);
		LinphoneAddress *proxy_addr = linphone_address_new(url);
		if (proxy_addr) {
			linphone_address_set_transport(proxy_addr, creator->transport);
			char *proxy_addr_str = linphone_address_as_string_uri_only(proxy_addr);
			linphone_account_params_set_server_addr(params, proxy_addr_str);
			bctbx_free(proxy_addr_str);
			linphone_address_unref(proxy_addr);
		} else {
			linphone_account_params_set_server_addr(params, creator->domain);
		}
		ms_free(url);
	}

	linphone_account_params_enable_register(params, TRUE);

	info = linphone_auth_info_new_for_algorithm(
	    linphone_address_get_username(identity), // username
	    NULL,                                    // user id
	    creator->password,                       // passwd
	    creator->password ? NULL : creator->ha1, // ha1
	    !creator->password && creator->ha1 ? linphone_address_get_domain(identity)
	                                       : NULL, // realm - assumed to be domain
	    linphone_address_get_domain(identity),     // domain
	    creator->password ? NULL
	                      : creator->algorithm // if clear text password is given, allow its usage with all algorithms.
	);
	linphone_core_add_auth_info(creator->core, info);
	linphone_address_unref(identity);

	LinphoneAccount *account = linphone_core_create_account(creator->core, params);
	linphone_account_params_unref(params);

	if (linphone_core_add_account(creator->core, account) != -1) {
		if (creator->set_as_default) {
			linphone_core_set_default_account(creator->core, account);
		}
		linphone_auth_info_unref(info);
		return account;
	}

	linphone_core_remove_auth_info(creator->core, info);
	linphone_auth_info_unref(info);
	return NULL;
}

LinphoneProxyConfig *linphone_account_creator_configure(const LinphoneAccountCreator *creator) {
	LinphoneAccount *account = linphone_account_creator_create_account_in_core(creator);
	if (account) {
		return linphone_account_get_proxy_config(account);
	}
	return NULL;
}

LinphoneAccountCreatorUsernameStatus linphone_account_creator_set_username(LinphoneAccountCreator *creator,
                                                                           const char *username) {
	int min_length =
	    linphone_config_get_int(linphone_core_get_config(creator->core), "assistant", "username_min_length", -1);
	int max_length =
	    linphone_config_get_int(linphone_core_get_config(creator->core), "assistant", "username_max_length", -1);
	bool_t use_phone_number =
	    !!linphone_config_get_int(linphone_core_get_config(creator->core), "assistant", "use_phone_number", 0);
	const char *regex =
	    linphone_config_get_string(linphone_core_get_config(creator->core), "assistant", "username_regex", 0);
	if (!username) {
		reset_field(&creator->username);
		return LinphoneAccountCreatorUsernameStatusOk;
	} else if (min_length > 0 && strlen(username) < (size_t)min_length) {
		return LinphoneAccountCreatorUsernameStatusTooShort;
	} else if (max_length > 0 && strlen(username) > (size_t)max_length) {
		return LinphoneAccountCreatorUsernameStatusTooLong;
	} else if (use_phone_number && !linphone_account_is_phone_number(NULL, username)) {
		return LinphoneAccountCreatorUsernameStatusInvalid;
	} else if (regex && !bctbx_is_matching_regex(username, regex)) {
		return LinphoneAccountCreatorUsernameStatusInvalidCharacters;
	} else if (validate_uri(creator->core, username, NULL, NULL) != 0) {
		return LinphoneAccountCreatorUsernameStatusInvalid;
	}

	set_string(&creator->username, username, FALSE);
	return LinphoneAccountCreatorUsernameStatusOk;
}

const char *linphone_account_creator_get_username(const LinphoneAccountCreator *creator) {
	return creator->username;
}

void linphone_account_creator_set_pn_provider(LinphoneAccountCreator *creator, const char *pn_provider) {
	set_string(&creator->pn_provider, pn_provider, FALSE);
}

const char *linphone_account_creator_get_pn_provider(const LinphoneAccountCreator *creator) {
	return creator->pn_provider;
}

void linphone_account_creator_set_pn_param(LinphoneAccountCreator *creator, const char *pn_param) {
	set_string(&creator->pn_param, pn_param, FALSE);
}

const char *linphone_account_creator_get_pn_param(const LinphoneAccountCreator *creator) {
	return creator->pn_param;
}

void linphone_account_creator_set_pn_prid(LinphoneAccountCreator *creator, const char *pn_prid) {
	set_string(&creator->pn_prid, pn_prid, FALSE);
}

const char *linphone_account_creator_get_pn_prid(const LinphoneAccountCreator *creator) {
	return creator->pn_prid;
}

void linphone_account_creator_set_token(LinphoneAccountCreator *creator, const char *token) {
	set_string(&creator->token, token, FALSE);
}

const char *linphone_account_creator_get_token(const LinphoneAccountCreator *creator) {
	return creator->token;
}

void linphone_account_creator_set_account_creation_request_token(LinphoneAccountCreator *creator, const char *token) {
	set_string(&creator->account_creation_request_token, token, FALSE);
}

const char *linphone_account_creator_get_account_creation_request_token(const LinphoneAccountCreator *creator) {
	return creator->account_creation_request_token;
}

LinphoneAccountCreatorPhoneNumberStatusMask linphone_account_creator_set_phone_number(LinphoneAccountCreator *creator,
                                                                                      const char *phone_number,
                                                                                      const char *country_code) {
	char *normalized_phone_number;
	LinphoneAccountCreatorPhoneNumberStatusMask return_status = 0;
	if (!phone_number || !country_code) {
		if (!phone_number && !country_code) {
			creator->phone_number = NULL;
			creator->phone_country_code = NULL;
			return (LinphoneAccountCreatorPhoneNumberStatusMask)LinphoneAccountCreatorPhoneNumberStatusOk;
		} else {
			return (LinphoneAccountCreatorPhoneNumberStatusMask)LinphoneAccountCreatorPhoneNumberStatusInvalid;
		}
	} else {
		if (strlen(country_code) == 0 || strcmp(country_code, "+") == 0)
			return LinphoneAccountCreatorPhoneNumberStatusInvalidCountryCode;

		if (!creator->account) {
			LinphoneAccountParams *params = linphone_account_params_new(creator->core, TRUE);
			creator->account = linphone_core_create_account(creator->core, params);
			linphone_account_params_unref(params);
		}
		LinphoneAccount *numCfg = creator->account;

		creator->phone_country_code = ms_strdup(country_code[0] == '+' ? &country_code[1] : country_code);
		LinphoneAccountParams *account_params = linphone_account_params_clone(linphone_account_get_params(numCfg));
		linphone_account_params_set_international_prefix(account_params, creator->phone_country_code);
		linphone_account_set_params(numCfg, account_params);
		linphone_account_params_unref(account_params);
		normalized_phone_number = linphone_account_normalize_phone_number(numCfg, phone_number);
		if (!normalized_phone_number) {
			return LinphoneAccountCreatorPhoneNumberStatusInvalid;
		}

		// if phone is valid, we lastly want to check that length is OK in case phone_number was normalized
		if (strcmp(normalized_phone_number, phone_number) != 0 || phone_number[0] != '+') {
			std::shared_ptr<DialPlan> plan = DialPlan::findByCcc(creator->phone_country_code);
			int size = (int)strlen(phone_number);
			if (plan->isGeneric()) {
				return_status = LinphoneAccountCreatorPhoneNumberStatusInvalidCountryCode;
				goto end;
			}
			if (phone_number[0] == '+') {
				// + 1 because of the '+' sign
				size -= (int)(strlen(country_code) + 1);
			} else if (strncmp("00", phone_number, 2) == 0) {
				// + 2 because of the starting 00<country_code>
				size -= (int)(strlen(country_code) + 2);
			}
			// DO NOT NOTIFY ABOUT PHONE NUMBER BEING TOO SHORT,
			// OUR DIAL PLAN IMPLEMENTATION ISNT PRECISE ENOUGH TO GARANTY
			// THE PHONE NUMBER IS INVALID
			/*if (size < plan->getMaxNationalNumberLength() - 1) {
			    return_status = LinphoneAccountCreatorPhoneNumberStatusTooShort;
			    goto end;
			} else*/
			if (size > plan->getMaxNationalNumberLength() + 1) {
				return_status = LinphoneAccountCreatorPhoneNumberStatusTooLong;
				goto end;
			} else if (return_status & LinphoneAccountCreatorPhoneNumberStatusInvalidCountryCode) {
				goto end;
			}
		}
	}
	set_string(&creator->phone_number, normalized_phone_number, FALSE);
	return_status = LinphoneAccountCreatorPhoneNumberStatusOk;
end:
	ms_free(normalized_phone_number);
	return return_status;
}

const char *linphone_account_creator_get_phone_number(const LinphoneAccountCreator *creator) {
	return creator->phone_number;
}

const char *linphone_account_creator_get_phone_country_code(const LinphoneAccountCreator *creator) {
	return creator->phone_country_code;
}

LinphoneAccountCreatorPasswordStatus linphone_account_creator_set_password(LinphoneAccountCreator *creator,
                                                                           const char *password) {
	int min_length =
	    linphone_config_get_int(linphone_core_get_config(creator->core), "assistant", "password_min_length", -1);
	int max_length =
	    linphone_config_get_int(linphone_core_get_config(creator->core), "assistant", "password_max_length", -1);
	if (!password) {
		reset_field(&creator->password);
		return LinphoneAccountCreatorPasswordStatusTooShort;
	}
	if (min_length > 0 && strlen(password) < (size_t)min_length) {
		return LinphoneAccountCreatorPasswordStatusTooShort;
	} else if (max_length > 0 && strlen(password) > (size_t)max_length) {
		return LinphoneAccountCreatorPasswordStatusTooLong;
	}
	set_string(&creator->password, password, FALSE);
	return LinphoneAccountCreatorPasswordStatusOk;
}

const char *linphone_account_creator_get_password(const LinphoneAccountCreator *creator) {
	return creator->password;
}

LinphoneAccountCreatorAlgoStatus linphone_account_creator_set_algorithm(LinphoneAccountCreator *creator,
                                                                        const char *algorithm) {
	set_string(&creator->algorithm, algorithm, FALSE);
	if (algorithm && strcmp(algorithm, "MD5") && strcmp(algorithm, "SHA-256")) {
		return LinphoneAccountCreatorAlgoStatusNotSupported;
	}
	return LinphoneAccountCreatorAlgoStatusOk;
}

const char *linphone_account_creator_get_algorithm(const LinphoneAccountCreator *creator) {
	return creator->algorithm;
}

LinphoneAccountCreatorPasswordStatus linphone_account_creator_set_ha1(LinphoneAccountCreator *creator,
                                                                      const char *ha1) {
	set_string(&creator->ha1, ha1, FALSE);
	return LinphoneAccountCreatorPasswordStatusOk;
}

const char *linphone_account_creator_get_ha1(const LinphoneAccountCreator *creator) {
	return creator->ha1;
}

LinphoneAccountCreatorActivationCodeStatus linphone_account_creator_set_activation_code(LinphoneAccountCreator *creator,
                                                                                        const char *activation_code) {
	set_string(&creator->activation_code, activation_code, FALSE);
	return LinphoneAccountCreatorActivationCodeStatusOk;
}

const char *linphone_account_creator_get_activation_code(const LinphoneAccountCreator *creator) {
	return creator->activation_code;
}

LinphoneAccountCreatorLanguageStatus linphone_account_creator_set_language(LinphoneAccountCreator *creator,
                                                                           const char *lang) {
	set_string(&creator->language, lang, FALSE);
	return LinphoneAccountCreatorLanguageStatusOk;
}

const char *linphone_account_creator_get_language(const LinphoneAccountCreator *creator) {
	return creator->language;
}

LinphoneAccountCreatorUsernameStatus linphone_account_creator_set_display_name(LinphoneAccountCreator *creator,
                                                                               const char *display_name) {
	if (validate_uri(creator->core, NULL, display_name, NULL) != 0) {
		return LinphoneAccountCreatorUsernameStatusInvalid;
	}
	set_string(&creator->display_name, display_name, FALSE);
	return LinphoneAccountCreatorUsernameStatusOk;
}

const char *linphone_account_creator_get_display_name(const LinphoneAccountCreator *creator) {
	return creator->display_name;
}

LinphoneAccountCreatorEmailStatus linphone_account_creator_set_email(LinphoneAccountCreator *creator,
                                                                     const char *email) {
	if (!email || !bctbx_is_matching_regex(email, "^.+@.+\\..*$")) {
		return LinphoneAccountCreatorEmailStatusMalformed;
	}
	if (!bctbx_is_matching_regex(email, "^.+@.+\\.[A-Za-z]{2}[A-Za-z]*$")) {
		return LinphoneAccountCreatorEmailStatusInvalidCharacters;
	}
	set_string(&creator->email, email, TRUE);
	return LinphoneAccountCreatorEmailStatusOk;
}

const char *linphone_account_creator_get_email(const LinphoneAccountCreator *creator) {
	return creator->email;
}

LinphoneAccountCreatorDomainStatus linphone_account_creator_set_domain(LinphoneAccountCreator *creator,
                                                                       const char *domain) {
	if (domain && validate_uri(creator->core, NULL, domain, NULL) != 0) {
		return LinphoneAccountCreatorDomainInvalid;
	}

	set_string(&creator->domain, domain, TRUE);
	return LinphoneAccountCreatorDomainOk;
}

const char *linphone_account_creator_get_domain(const LinphoneAccountCreator *creator) {
	return creator->domain;
}

LinphoneAccountCreatorTransportStatus linphone_account_creator_set_transport(LinphoneAccountCreator *creator,
                                                                             LinphoneTransportType transport) {
	if (!linphone_core_sip_transport_supported(creator->core, transport)) {
		return LinphoneAccountCreatorTransportUnsupported;
	}
	creator->transport = transport;
	return LinphoneAccountCreatorTransportOk;
}

LinphoneTransportType linphone_account_creator_get_transport(const LinphoneAccountCreator *creator) {
	return creator->transport;
}

LinphoneAccountCreatorStatus linphone_account_creator_set_route(LinphoneAccountCreator *creator, const char *route) {
	if (!creator->account) {
		LinphoneAccountParams *params = linphone_account_params_new(creator->core, TRUE);
		creator->account = linphone_core_create_account(creator->core, params);
		linphone_account_params_unref(params);
	}
	bool set_route_ok = true;
	if (route != NULL && route[0] != '\0') {
		bctbx_list_t *list = NULL;
		std::string tmp;
		/*try to prepend 'sip:' */
		if (strstr(route, "sip:") == NULL && strstr(route, "sips:") == NULL) {
			tmp.append("sip:");
		}
		tmp.append(route);

		auto *addr = linphone_address_new(tmp.c_str());
		if (addr != NULL) {
			list = bctbx_list_append(list, addr);
		}
		LinphoneAccountParams *account_params =
		    linphone_account_params_clone(linphone_account_get_params(creator->account));
		set_route_ok = (linphone_account_params_set_routes_addresses(account_params, list) == 0);
		linphone_account_set_params(creator->account, account_params);
		linphone_account_params_unref(account_params);
		if (list) {
			bctbx_list_free_with_data(list, (bctbx_list_free_func)linphone_address_unref);
		}
	}

	if (!route || !set_route_ok) {
		return LinphoneAccountCreatorStatusRequestFailed;
	}

	set_string(&creator->route, route, TRUE);
	return LinphoneAccountCreatorStatusRequestOk;
}

const char *linphone_account_creator_get_route(const LinphoneAccountCreator *creator) {
	return creator->route;
}

LinphoneAccountCreatorStatus linphone_account_creator_set_as_default(LinphoneAccountCreator *creator,
                                                                     bool_t set_as_default) {
	creator->set_as_default = set_as_default;
	return LinphoneAccountCreatorStatusRequestOk;
}

bool_t linphone_account_creator_get_set_as_default(const LinphoneAccountCreator *creator) {
	return creator->set_as_default;
}

void linphone_account_creator_use_test_admin_account(LinphoneAccountCreator *creator) {
	if (linphone_core_get_account_creator_backend(creator->core) != LinphoneAccountCreatorBackendFlexiAPI) {
		lWarning() << "linphone_account_creator_use_test_admin_account() is only meaningful for FlexiAPI backend.";
		return;
	}
#ifdef HAVE_FLEXIAPI
	linphone_account_creator_service_set_create_account_cb(linphone_account_creator_get_service(creator),
	                                                       linphone_account_creator_admin_create_account_flexiapi);
#else
	lError() << "linphone_account_creator_use_test_admin_account(): FlexiAPI not compiled in this version.";
#endif
}

/************************** Start Account Creator data **************************/
static void _linphone_account_creator_destroy(LinphoneAccountCreator *creator) {
	/*this will drop all pending requests if any*/
	LinphoneXmlRpcSession *xmlrpc =
	    (LinphoneXmlRpcSession *)belle_sip_object_data_get(BELLE_SIP_OBJECT(creator), "xmlrpc_session");
	if (xmlrpc) linphone_xml_rpc_session_release(xmlrpc);

	if (creator->service != NULL) {
		if (linphone_account_creator_service_get_destructor_cb(creator->service) != NULL)
			linphone_account_creator_service_get_destructor_cb(creator->service)(creator);
		linphone_account_creator_service_unref(creator->service);
	}

	linphone_account_creator_cbs_unref(creator->cbs);
	bctbx_list_free_with_data(creator->callbacks, (bctbx_list_free_func)linphone_account_creator_cbs_unref);
	creator->callbacks = nullptr;

	linphone_account_creator_reset(creator);
}

BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneAccountCreator);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneAccountCreator,
                           belle_sip_object_t,
                           (belle_sip_object_destroy_t)_linphone_account_creator_destroy,
                           NULL, // clone
                           NULL, // marshal
                           FALSE);

LinphoneAccountCreator *linphone_account_creator_new(LinphoneCore *core, const char *xmlrpc_url) {
	LinphoneAccountCreator *creator;

	creator = belle_sip_object_new(LinphoneAccountCreator);
	creator->service = linphone_core_get_account_creator_service(core);
	linphone_account_creator_service_ref(creator->service);
	creator->cbs = linphone_account_creator_cbs_new();
	creator->core = core;
	creator->transport = LinphoneTransportTcp;

	if (xmlrpc_url) {
		belle_sip_object_data_set(BELLE_SIP_OBJECT(creator), "xmlrpc_session",
		                          linphone_xml_rpc_session_new(core, xmlrpc_url), NULL);
	}

	creator->set_as_default = TRUE;

	LinphoneAccountParams *params = linphone_account_params_new(core, TRUE);
	creator->account = linphone_core_create_account(core, params);
	linphone_account_params_unref(params);

	if (creator->service != NULL && linphone_account_creator_service_get_constructor_cb(creator->service) != NULL)
		linphone_account_creator_service_get_constructor_cb(creator->service)(creator);

	return creator;
}

LinphoneAccountCreator *linphone_account_creator_create(LinphoneCore *core) {
	return linphone_account_creator_new(core, NULL);
}

void linphone_account_creator_reset(LinphoneAccountCreator *creator) {
	reset_field(&creator->username);
	reset_field(&creator->display_name);
	reset_field(&creator->password);
	reset_field(&creator->ha1);
	reset_field(&creator->phone_number);
	reset_field(&creator->phone_country_code);
	reset_field(&creator->email);
	reset_field(&creator->language);
	reset_field(&creator->activation_code);
	reset_field(&creator->domain);
	reset_field(&creator->route);
	reset_field(&creator->algorithm);
	reset_field(&creator->token);
	reset_field(&creator->pn_prid);
	reset_field(&creator->pn_param);
	reset_field(&creator->pn_provider);

	if (creator->account) {
		linphone_account_unref(creator->account);
		creator->account = nullptr;
	}
}

LinphoneAccountCreator *linphone_core_create_account_creator(LinphoneCore *core, const char *xmlrpc_url) {
	return linphone_account_creator_new(core, xmlrpc_url);
}

LinphoneAccountCreator *linphone_account_creator_ref(LinphoneAccountCreator *creator) {
	belle_sip_object_ref(creator);
	return creator;
}

void linphone_account_creator_unref(LinphoneAccountCreator *creator) {
	belle_sip_object_unref(creator);
}

void *linphone_account_creator_get_user_data(const LinphoneAccountCreator *creator) {
	return creator->user_data;
}

void linphone_account_creator_set_user_data(LinphoneAccountCreator *creator, void *ud) {
	creator->user_data = ud;
}

void linphone_account_creator_set_proxy_config(LinphoneAccountCreator *creator, LinphoneProxyConfig *cfg) {
	linphone_account_creator_set_account(creator, cfg ? linphone_proxy_config_get_account(cfg) : nullptr);
}

void linphone_account_creator_set_account(LinphoneAccountCreator *creator, LinphoneAccount *account) {
	LinphoneAccount *old_account = creator->account;

	creator->account = account ? linphone_account_ref(account) : NULL;

	if (old_account) {
		linphone_account_unref(old_account);
	}
}

LinphoneAccountCreatorCbs *linphone_account_creator_get_callbacks(const LinphoneAccountCreator *creator) {
	return creator->cbs;
}

void linphone_account_creator_add_callbacks(LinphoneAccountCreator *creator, LinphoneAccountCreatorCbs *cbs) {
	creator->callbacks = bctbx_list_append(creator->callbacks, linphone_account_creator_cbs_ref(cbs));
}

void linphone_account_creator_remove_callbacks(LinphoneAccountCreator *creator, LinphoneAccountCreatorCbs *cbs) {
	creator->callbacks = bctbx_list_remove(creator->callbacks, cbs);
	linphone_account_creator_cbs_unref(cbs);
}

LinphoneAccountCreatorCbs *linphone_account_creator_get_current_callbacks(const LinphoneAccountCreator *creator) {
	return creator->currentCbs;
}

void linphone_account_creator_set_current_callbacks(LinphoneAccountCreator *creator, LinphoneAccountCreatorCbs *cbs) {
	creator->currentCbs = cbs;
}

const bctbx_list_t *linphone_account_creator_get_callbacks_list(const LinphoneAccountCreator *creator) {
	return creator->callbacks;
}

LinphoneAccountCreatorService *linphone_account_creator_get_service(const LinphoneAccountCreator *creator) {
	return creator->service;
}

LinphoneAccountCreatorStatus linphone_account_creator_is_account_exist(LinphoneAccountCreator *creator) {
	if (creator->service->is_account_exist_request_cb) {
		return creator->service->is_account_exist_request_cb(creator);
	}
	return LinphoneAccountCreatorStatusNotImplementedError;
}

LinphoneAccountCreatorStatus linphone_account_creator_create_account(LinphoneAccountCreator *creator) {
	if (creator->service->create_account_request_cb) {
		return creator->service->create_account_request_cb(creator);
	}
	return LinphoneAccountCreatorStatusNotImplementedError;
}

LinphoneAccountCreatorStatus linphone_account_creator_create_push_account(LinphoneAccountCreator *creator) {
	if (creator->service->create_push_account_request_cb) {
		return creator->service->create_push_account_request_cb(creator);
	}
	return LinphoneAccountCreatorStatusNotImplementedError;
}

LinphoneAccountCreatorStatus linphone_account_creator_delete_account(LinphoneAccountCreator *creator) {
	if (creator->service->delete_account_request_cb) {
		return creator->service->delete_account_request_cb(creator);
	}
	return LinphoneAccountCreatorStatusNotImplementedError;
}

LinphoneAccountCreatorStatus linphone_account_creator_get_confirmation_key(LinphoneAccountCreator *creator) {
	if (creator->service->confirmation_key_request_cb) {
		return creator->service->confirmation_key_request_cb(creator);
	}
	return LinphoneAccountCreatorStatusNotImplementedError;
}

LinphoneAccountCreatorStatus linphone_account_creator_is_account_activated(LinphoneAccountCreator *creator) {
	if (creator->service->is_account_activated_request_cb) {
		return creator->service->is_account_activated_request_cb(creator);
	}
	return LinphoneAccountCreatorStatusNotImplementedError;
}

LinphoneAccountCreatorStatus linphone_account_creator_activate_account(LinphoneAccountCreator *creator) {
	if (creator->service->activate_account_request_cb) {
		return creator->service->activate_account_request_cb(creator);
	}
	return LinphoneAccountCreatorStatusNotImplementedError;
}

LinphoneAccountCreatorStatus linphone_account_creator_link_account(LinphoneAccountCreator *creator) {
	if (creator->service->link_account_request_cb) {
		return creator->service->link_account_request_cb(creator);
	}
	return LinphoneAccountCreatorStatusNotImplementedError;
}

LinphoneAccountCreatorStatus linphone_account_creator_activate_alias(LinphoneAccountCreator *creator) {
	if (creator->service->activate_alias_request_cb) {
		return creator->service->activate_alias_request_cb(creator);
	}
	return LinphoneAccountCreatorStatusNotImplementedError;
}

LinphoneAccountCreatorStatus linphone_account_creator_is_alias_used(LinphoneAccountCreator *creator) {
	if (creator->service->is_alias_used_request_cb) {
		return creator->service->is_alias_used_request_cb(creator);
	}
	return LinphoneAccountCreatorStatusNotImplementedError;
}

LinphoneAccountCreatorStatus linphone_account_creator_is_account_linked(LinphoneAccountCreator *creator) {
	if (creator->service->is_account_linked_request_cb) {
		return creator->service->is_account_linked_request_cb(creator);
	}
	return LinphoneAccountCreatorStatusNotImplementedError;
}

LinphoneAccountCreatorStatus linphone_account_creator_recover_account(LinphoneAccountCreator *creator) {
	if (creator->service->recover_account_request_cb) {
		return creator->service->recover_account_request_cb(creator);
	}
	return LinphoneAccountCreatorStatusNotImplementedError;
}

LinphoneAccountCreatorStatus linphone_account_creator_update_account(LinphoneAccountCreator *creator) {
	if (creator->service->update_account_request_cb) {
		return creator->service->update_account_request_cb(creator);
	}
	return LinphoneAccountCreatorStatusNotImplementedError;
}

LinphoneAccountCreatorStatus linphone_account_creator_login_linphone_account(LinphoneAccountCreator *creator) {
	if (creator->service->login_linphone_account_request_cb) {
		return creator->service->login_linphone_account_request_cb(creator);
	}
	return LinphoneAccountCreatorStatusNotImplementedError;
}

LinphoneAccountCreatorStatus linphone_account_creator_request_auth_token(LinphoneAccountCreator *creator) {
	if (creator->service->send_token_request_cb) {
		return creator->service->send_token_request_cb(creator);
	}
	return LinphoneAccountCreatorStatusNotImplementedError;
}

LinphoneAccountCreatorStatus
linphone_account_creator_request_account_creation_request_token(LinphoneAccountCreator *creator) {
	if (creator->service->account_creation_request_token_request_cb) {
		return creator->service->account_creation_request_token_request_cb(creator);
	}
	return LinphoneAccountCreatorStatusNotImplementedError;
}

LinphoneAccountCreatorStatus
linphone_account_creator_request_account_creation_token_using_request_token(LinphoneAccountCreator *creator) {
	if (creator->service->account_creation_token_using_request_token_request_cb) {
		return creator->service->account_creation_token_using_request_token_request_cb(creator);
	}
	return LinphoneAccountCreatorStatusNotImplementedError;
}

/************************** End Account Creator data **************************/
