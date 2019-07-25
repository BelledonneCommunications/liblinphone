/*
linphone
Copyright (C) 2010-2018 Belledonne Communications SARL

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "linphone/account_creator.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"


#include "c-wrapper/c-wrapper.h"
#include "dial-plan/dial-plan.h"


#include "bctoolbox/crypto.h"
#include "bctoolbox/regex.h"

// TODO: From coreapi. Remove me later.
#include "private.h"

// =============================================================================

using namespace LinphonePrivate;

BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneAccountCreatorCbs);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneAccountCreatorCbs, belle_sip_object_t,
	NULL, // destroy
	NULL, // clone
	NULL, // marshal
	FALSE
);

/************************** Start Misc **************************/
static const char *_get_domain(LinphoneAccountCreator *creator) {
	if (creator->domain)
		return creator->domain;
	return linphone_proxy_config_get_domain(creator->proxy_cfg);
}

static const char* ha1_for_passwd(const char* username, const char* realm, const char* passwd, const char* algo) {
	if(algo==NULL || strcmp(algo, "MD5")==0) {
		static char ha1[33];
		sal_auth_compute_ha1(username, realm, passwd, ha1);
		return ha1;
	}
	else if(strcmp(algo, "SHA-256")==0) {
		static char ha1[65];
		sal_auth_compute_ha1_for_algorithm(username, realm, passwd, ha1, 65, algo);
		return ha1;
	}
	else {
		return NULL;
	}
}

static unsigned int validate_uri(const char* username, const char* domain, const char* display_name) {
	LinphoneAddress* addr;
	unsigned int status = 0;
	LinphoneProxyConfig* proxy = linphone_proxy_config_new();
	linphone_proxy_config_set_identity(proxy, "sip:?@domain.com");

	if (username) {
		addr = linphone_proxy_config_normalize_sip_uri(proxy, username);
	} else {
		addr = linphone_address_clone(linphone_proxy_config_get_identity_address(proxy));
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
	linphone_proxy_config_destroy(proxy);
	return status;
}

static char* _get_identity(const LinphoneAccountCreator *creator) {
	char *identity = NULL;
	if ((creator->username || creator->phone_number)) {
		//we must escape username
		LinphoneProxyConfig* proxy = linphone_core_create_proxy_config(creator->core);
		LinphoneAddress* addr;

		addr = linphone_proxy_config_normalize_sip_uri(proxy, creator->username ? creator->username : creator->phone_number);
		if (addr == NULL || (creator->domain && strcmp(linphone_address_get_domain(addr), creator->domain) != 0)) {
			if ((creator->username || creator->phone_number) && creator->domain) {
                char *url = ms_strdup_printf("sip:%s@%s", creator->username ? creator->username : creator->phone_number, creator->domain);
				addr = linphone_address_new(url);
				ms_free(url);
				if (addr == NULL) {
					goto end;
				}
			} else {
				goto end;
			}
		}

		identity = linphone_address_as_string(addr);
		linphone_address_unref(addr);
		end:
		linphone_proxy_config_destroy(proxy);
	}
	return identity;
}

static inline void resetField (char **field) {
	if (*field) {
		bctbx_free(*field);
		*field = nullptr;
	}
}

LinphoneProxyConfig * linphone_account_creator_create_proxy_config(const LinphoneAccountCreator *creator) {
	LinphoneAuthInfo *info;
	LinphoneProxyConfig *cfg = linphone_core_create_proxy_config(creator->core);
	char *identity_str = _get_identity(creator);
	LinphoneAddress *identity = linphone_address_new(identity_str);

	ms_free(identity_str);
	if (creator->display_name) {
		linphone_address_set_display_name(identity, creator->display_name);
	}
	linphone_proxy_config_set_identity_address(cfg, identity);
	if (creator->phone_country_code) {
		linphone_proxy_config_set_dial_prefix(cfg, creator->phone_country_code);
	} else if (creator->phone_number) {
		int dial_prefix_number = DialPlan::lookupCccFromE164(creator->phone_number);
		char buff[4];
		snprintf(buff, sizeof(buff), "%d", dial_prefix_number);
		linphone_proxy_config_set_dial_prefix(cfg, buff);
	}
	if (linphone_proxy_config_get_server_addr(cfg) == NULL && creator->domain != NULL) {
		char *url = ms_strdup_printf("sip:%s", creator->domain);
		LinphoneAddress *proxy_addr = linphone_address_new(url);
		if (proxy_addr) {
			linphone_address_set_transport(proxy_addr, creator->transport);
			linphone_proxy_config_set_server_addr(cfg, linphone_address_as_string_uri_only(proxy_addr));
			linphone_address_unref(proxy_addr);
		} else {
			linphone_proxy_config_set_server_addr(cfg, creator->domain);
		}
		ms_free(url);
	}

	linphone_proxy_config_enable_register(cfg, TRUE);

	info = linphone_auth_info_new_for_algorithm(linphone_address_get_username(identity), // username
								NULL, //user id
								creator->password, // passwd
								creator->password ? NULL : creator->ha1,  // ha1
								!creator->password && creator->ha1 ? linphone_address_get_domain(identity) : NULL,  // realm - assumed to be domain
								linphone_address_get_domain(identity), // domain
								creator->algorithm
	);
	linphone_core_add_auth_info(creator->core, info);
	linphone_address_unref(identity);

	if (linphone_core_add_proxy_config(creator->core, cfg) != -1) {
		if (creator->set_as_default) {
			linphone_core_set_default_proxy(creator->core, cfg);
		}
		return cfg;
	}

	linphone_core_remove_auth_info(creator->core, info);
	linphone_auth_info_unref(info);
	return NULL;
}

LinphoneProxyConfig * linphone_account_creator_configure(const LinphoneAccountCreator *creator) {
	return linphone_account_creator_create_proxy_config(creator);
}
/************************** End Misc **************************/

/************************** Start Account Creator Cbs **************************/

LinphoneAccountCreatorCbs * linphone_account_creator_cbs_new(void) {
	return belle_sip_object_new(LinphoneAccountCreatorCbs);
}

LinphoneAccountCreatorCbs * linphone_account_creator_cbs_ref(LinphoneAccountCreatorCbs *cbs) {
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

LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_create_account(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->create_account_response_cb;
}

void linphone_account_creator_cbs_set_create_account(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->create_account_response_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_delete_account(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->delete_account_response_cb;
}

void linphone_account_creator_cbs_set_delete_account(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->delete_account_response_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_is_account_exist(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->is_account_exist_response_cb;
}

void linphone_account_creator_cbs_set_is_account_exist(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->is_account_exist_response_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_confirmation_key(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->confirmation_key_response_cb;
}

void linphone_account_creator_cbs_set_confirmation_key(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->confirmation_key_response_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_activate_account(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->activate_account_response_cb;
}

void linphone_account_creator_cbs_set_activate_account(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->activate_account_response_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_is_account_activated(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->is_account_activated_response_cb;
}

void linphone_account_creator_cbs_set_is_account_activated(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->is_account_activated_response_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_link_account(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->link_account_response_cb;
}

void linphone_account_creator_cbs_set_link_account(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->link_account_response_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_activate_alias(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->activate_alias_response_cb;
}

void linphone_account_creator_cbs_set_activate_alias(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->activate_alias_response_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_is_alias_used(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->is_alias_used_response_cb;
}

void linphone_account_creator_cbs_set_is_alias_used(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->is_alias_used_response_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_is_account_linked(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->is_account_linked_response_cb;
}

void linphone_account_creator_cbs_set_is_account_linked(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->is_account_linked_response_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_recover_account(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->recover_account_response_cb;
}

void linphone_account_creator_cbs_set_recover_account(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->recover_account_response_cb = cb;
}

LinphoneAccountCreatorCbsStatusCb linphone_account_creator_cbs_get_update_account(const LinphoneAccountCreatorCbs *cbs) {
	return cbs->update_account_response_cb;
}

void linphone_account_creator_cbs_set_update_account(LinphoneAccountCreatorCbs *cbs, LinphoneAccountCreatorCbsStatusCb cb) {
	cbs->update_account_response_cb = cb;
}
/************************** End Account Creator Cbs **************************/

/************************** Start Account Creator data **************************/
static void _linphone_account_creator_destroy(LinphoneAccountCreator *creator) {
	/*this will drop all pending requests if any*/
	if (creator->xmlrpc_session) linphone_xml_rpc_session_release(creator->xmlrpc_session);
	if (creator->service != NULL ) {
			if (linphone_account_creator_service_get_destructor_cb(creator->service) != NULL)
				linphone_account_creator_service_get_destructor_cb(creator->service)(creator);
			linphone_account_creator_service_unref(creator->service);
	}

	linphone_account_creator_cbs_unref(creator->cbs);
	bctbx_list_free_with_data(creator->callbacks, (bctbx_list_free_func)linphone_account_creator_cbs_unref);
	creator->callbacks = nullptr;
	if (creator->proxy_cfg) {
		linphone_proxy_config_unref(creator->proxy_cfg);
	}
	linphone_account_creator_reset(creator);
}

BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneAccountCreator);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneAccountCreator, belle_sip_object_t,
	(belle_sip_object_destroy_t)_linphone_account_creator_destroy,
	NULL, // clone
	NULL, // marshal
	FALSE
);

LinphoneAccountCreator * _linphone_account_creator_new(LinphoneCore *core, const char *xmlrpc_url) {
	LinphoneAccountCreator *creator;
	const char* domain = lp_config_get_string(core->config, "assistant", "domain", NULL);
	creator = belle_sip_object_new(LinphoneAccountCreator);
	creator->service = linphone_core_get_account_creator_service(core);
	linphone_account_creator_service_ref(creator->service);
	creator->cbs = linphone_account_creator_cbs_new();
	creator->core = core;
	creator->transport = LinphoneTransportTcp;
	creator->xmlrpc_session = (xmlrpc_url) ? linphone_xml_rpc_session_new(core, xmlrpc_url) : NULL;
	if (domain) {
		linphone_account_creator_set_domain(creator, domain);
	}
	creator->set_as_default = TRUE;
	creator->proxy_cfg = linphone_core_create_proxy_config(core);

	if (creator->service != NULL && linphone_account_creator_service_get_constructor_cb(creator->service) != NULL)
		linphone_account_creator_service_get_constructor_cb(creator->service)(creator);

	return creator;
}

LinphoneAccountCreator * linphone_account_creator_new(LinphoneCore *core, const char *xmlrpc_url) {
	return _linphone_account_creator_new(core, xmlrpc_url);
}

void linphone_account_creator_reset(LinphoneAccountCreator *creator) {
	resetField(&creator->username);
	resetField(&creator->display_name);
	resetField(&creator->password);
	resetField(&creator->ha1);
	resetField(&creator->phone_number);
	resetField(&creator->phone_country_code);
	resetField(&creator->email);
	resetField(&creator->language);
	resetField(&creator->activation_code);
	resetField(&creator->domain);
	resetField(&creator->route);
}

LinphoneAccountCreator * linphone_core_create_account_creator(LinphoneCore *core, const char *xmlrpc_url) {
	return _linphone_account_creator_new(core, xmlrpc_url);
}

LinphoneAccountCreator * linphone_account_creator_ref(LinphoneAccountCreator *creator) {
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
	LinphoneProxyConfig *old_cfg = creator->proxy_cfg;

	creator->proxy_cfg = cfg ? linphone_proxy_config_ref(cfg) : NULL;

	if (old_cfg) {
		linphone_proxy_config_unref(old_cfg);
	}
}

LinphoneAccountCreatorUsernameStatus linphone_account_creator_set_username(LinphoneAccountCreator *creator, const char *username) {
	int min_length = lp_config_get_int(creator->core->config, "assistant", "username_min_length", -1);
	int max_length = lp_config_get_int(creator->core->config, "assistant", "username_max_length", -1);
	bool_t use_phone_number = !!lp_config_get_int(creator->core->config, "assistant", "use_phone_number", 0);
	const char* regex = lp_config_get_string(creator->core->config, "assistant", "username_regex", 0);
	if (!username) {
		resetField(&creator->username);
		return LinphoneAccountCreatorUsernameStatusOk;
	} else if (min_length > 0 && strlen(username) < (size_t)min_length) {
		return LinphoneAccountCreatorUsernameStatusTooShort;
	} else if (max_length > 0 && strlen(username) > (size_t)max_length) {
		return LinphoneAccountCreatorUsernameStatusTooLong;
	} else if (use_phone_number && !linphone_proxy_config_is_phone_number(NULL, username)) {
		return LinphoneAccountCreatorUsernameStatusInvalid;
	} else if (regex && !bctbx_is_matching_regex(username, regex)) {
		return LinphoneAccountCreatorUsernameStatusInvalidCharacters;
	} else if (validate_uri(username, NULL, NULL) != 0) {
		return LinphoneAccountCreatorUsernameStatusInvalid;
	}

	set_string(&creator->username, username, FALSE);
	return LinphoneAccountCreatorUsernameStatusOk;
}

const char * linphone_account_creator_get_username(const LinphoneAccountCreator *creator) {
	return creator->username;
}

LinphoneAccountCreatorPhoneNumberStatusMask linphone_account_creator_set_phone_number(LinphoneAccountCreator *creator, const char *phone_number, const char *country_code) {
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
		LinphoneProxyConfig *numCfg = creator->proxy_cfg;
		creator->phone_country_code = ms_strdup(country_code[0] == '+' ? &country_code[1] : country_code);
		linphone_proxy_config_set_dial_prefix(numCfg, creator->phone_country_code);
		normalized_phone_number = linphone_proxy_config_normalize_phone_number(numCfg, phone_number);
		if (!normalized_phone_number) {
			return LinphoneAccountCreatorPhoneNumberStatusInvalid;
		}
		// if phone is valid, we lastly want to check that length is OK in case phone_nunber was normilized
		if (strcmp(normalized_phone_number,phone_number) != 0 || phone_number[0] != '+') {
			std::shared_ptr<DialPlan> plan = DialPlan::findByCcc(creator->phone_country_code);
			int size = (int)strlen(phone_number);
			if (plan->isGeneric()) {
				return_status = LinphoneAccountCreatorPhoneNumberStatusInvalidCountryCode;
			}
			if (size < plan->getNationalNumberLength() - 1) {
				return_status += LinphoneAccountCreatorPhoneNumberStatusTooShort;
				goto end;
			} else if (size > plan->getNationalNumberLength() + 1) {
				return_status += LinphoneAccountCreatorPhoneNumberStatusTooLong;
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

const char * linphone_account_creator_get_phone_number(const LinphoneAccountCreator *creator) {
	return creator->phone_number;
}

LinphoneAccountCreatorPasswordStatus linphone_account_creator_set_password(LinphoneAccountCreator *creator, const char *password) {
	int min_length = lp_config_get_int(creator->core->config, "assistant", "password_min_length", -1);
	int max_length = lp_config_get_int(creator->core->config, "assistant", "password_max_length", -1);
	if (!password) {
		resetField(&creator->password);
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

const char * linphone_account_creator_get_password(const LinphoneAccountCreator *creator) {
	return creator->password;
}

LinphoneAccountCreatorAlgoStatus linphone_account_creator_set_algorithm(LinphoneAccountCreator *creator, const char *algorithm) {
	set_string(&creator->algorithm, algorithm, FALSE);
	if(algorithm && strcmp(algorithm,"MD5") && strcmp(algorithm, "SHA-256")) {
		return LinphoneAccountCreatorAlgoStatusNotSupported;
	}
	return LinphoneAccountCreatorAlgoStatusOk;
}

const char * linphone_account_creator_get_algorithm(const LinphoneAccountCreator *creator) {
	return creator->algorithm;
}

LinphoneAccountCreatorPasswordStatus linphone_account_creator_set_ha1(LinphoneAccountCreator *creator, const char *ha1){
	set_string(&creator->ha1, ha1, FALSE);
	return LinphoneAccountCreatorPasswordStatusOk;
}

const char * linphone_account_creator_get_ha1(const LinphoneAccountCreator *creator) {
	return creator->ha1;
}

LinphoneAccountCreatorActivationCodeStatus linphone_account_creator_set_activation_code(LinphoneAccountCreator *creator, const char *activation_code){
	set_string(&creator->activation_code, activation_code, FALSE);
	return LinphoneAccountCreatorActivationCodeStatusOk;
}

const char * linphone_account_creator_get_activation_code(const LinphoneAccountCreator *creator) {
	return creator->activation_code;
}

LinphoneAccountCreatorLanguageStatus linphone_account_creator_set_language(LinphoneAccountCreator *creator, const char *lang) {
	set_string(&creator->language, lang, FALSE);
	return LinphoneAccountCreatorLanguageStatusOk;
}

const char * linphone_account_creator_get_language(const LinphoneAccountCreator *creator) {
	return creator->language;
}

LinphoneAccountCreatorUsernameStatus linphone_account_creator_set_display_name(LinphoneAccountCreator *creator, const char *display_name) {
	if (validate_uri(NULL, display_name, NULL) != 0) {
		return LinphoneAccountCreatorUsernameStatusInvalid;
	}
	set_string(&creator->display_name, display_name, FALSE);
	return LinphoneAccountCreatorUsernameStatusOk;
}

const char * linphone_account_creator_get_display_name(const LinphoneAccountCreator *creator) {
	return creator->display_name;
}

LinphoneAccountCreatorEmailStatus linphone_account_creator_set_email(LinphoneAccountCreator *creator, const char *email) {
	if (!email || !bctbx_is_matching_regex(email, "^.+@.+\\..*$")) {
		return LinphoneAccountCreatorEmailStatusMalformed;
	}
	if (!bctbx_is_matching_regex(email, "^.+@.+\\.[A-Za-z]{2}[A-Za-z]*$")) {
		return LinphoneAccountCreatorEmailStatusInvalidCharacters;
	}
	set_string(&creator->email, email, TRUE);
	return LinphoneAccountCreatorEmailStatusOk;
}

const char * linphone_account_creator_get_email(const LinphoneAccountCreator *creator) {
	return creator->email;
}

LinphoneAccountCreatorDomainStatus linphone_account_creator_set_domain(LinphoneAccountCreator *creator, const char *domain) {
	if (domain && validate_uri(NULL, domain, NULL) != 0) {
		return LinphoneAccountCreatorDomainInvalid;
	}

	set_string(&creator->domain, domain, TRUE);
	return LinphoneAccountCreatorDomainOk;
 }

const char * linphone_account_creator_get_domain(const LinphoneAccountCreator *creator) {
	return creator->domain;
}

LinphoneAccountCreatorTransportStatus linphone_account_creator_set_transport(LinphoneAccountCreator *creator, LinphoneTransportType transport) {
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
	if (!route || linphone_proxy_config_set_route(creator->proxy_cfg, route) != 0)
		return LinphoneAccountCreatorStatusRequestFailed;

	set_string(&creator->route, route, TRUE);
	return LinphoneAccountCreatorStatusRequestOk;
}

const char * linphone_account_creator_get_route(const LinphoneAccountCreator *creator) {
	return creator->route;
}

LinphoneAccountCreatorStatus linphone_account_creator_set_as_default(LinphoneAccountCreator *creator, bool_t set_as_default) {
	creator->set_as_default = set_as_default;
	return LinphoneAccountCreatorStatusRequestOk;
}

bool_t linphone_account_creator_get_set_as_default(const LinphoneAccountCreator *creator) {
	return creator->set_as_default;
}

LinphoneAccountCreatorCbs * linphone_account_creator_get_callbacks(const LinphoneAccountCreator *creator) {
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

#define NOTIFY_IF_EXIST(cbName, functionName, ...) \
	bctbx_list_t *callbacksCopy = bctbx_list_copy(linphone_account_creator_get_callbacks_list(creator)); \
	for (bctbx_list_t *it = callbacksCopy; it; it = bctbx_list_next(it)) { \
		linphone_account_creator_set_current_callbacks(creator, reinterpret_cast<LinphoneAccountCreatorCbs *>(bctbx_list_get_data(it))); \
		LinphoneAccountCreatorCbs ## cbName ## Cb cb = linphone_account_creator_cbs_get_ ## functionName (linphone_account_creator_get_current_callbacks(creator)); \
		if (cb) \
			cb(__VA_ARGS__); \
	} \
	linphone_account_creator_set_current_callbacks(creator, nullptr); \
	bctbx_list_free(callbacksCopy);

LinphoneAccountCreatorService * linphone_account_creator_get_service(const LinphoneAccountCreator *creator) {
	return creator->service;
}

LinphoneAccountCreatorStatus linphone_account_creator_is_account_exist(LinphoneAccountCreator *creator) {
	return creator->service->is_account_exist_request_cb(creator);
}

LinphoneAccountCreatorStatus linphone_account_creator_create_account(LinphoneAccountCreator *creator) {
	return creator->service->create_account_request_cb(creator);
}

LinphoneAccountCreatorStatus linphone_account_creator_delete_account(LinphoneAccountCreator *creator) {
	return creator->service->delete_account_request_cb(creator);
}

LinphoneAccountCreatorStatus linphone_account_creator_get_confirmation_key(LinphoneAccountCreator *creator) {
	return creator->service->confirmation_key_request_cb(creator);
}

LinphoneAccountCreatorStatus linphone_account_creator_is_account_activated(LinphoneAccountCreator *creator) {
	return creator->service->is_account_activated_request_cb(creator);
}

LinphoneAccountCreatorStatus linphone_account_creator_activate_account(LinphoneAccountCreator *creator) {
	return creator->service->activate_account_request_cb(creator);
}

LinphoneAccountCreatorStatus linphone_account_creator_link_account(LinphoneAccountCreator *creator) {
	return creator->service->link_account_request_cb(creator);
}

LinphoneAccountCreatorStatus linphone_account_creator_activate_alias(LinphoneAccountCreator *creator) {
	return creator->service->activate_alias_request_cb(creator);
}

LinphoneAccountCreatorStatus linphone_account_creator_is_alias_used(LinphoneAccountCreator *creator) {
	return creator->service->is_alias_used_request_cb(creator);
}

LinphoneAccountCreatorStatus linphone_account_creator_is_account_linked(LinphoneAccountCreator *creator) {
	return creator->service->is_account_linked_request_cb(creator);
}

LinphoneAccountCreatorStatus linphone_account_creator_recover_account(LinphoneAccountCreator *creator) {
	return creator->service->recover_account_request_cb(creator);
}

LinphoneAccountCreatorStatus linphone_account_creator_update_account(LinphoneAccountCreator *creator) {
	return creator->service->update_account_request_cb(creator);
}
/************************** End Account Creator data **************************/

/************************** Start Account Creator Linphone **************************/

LinphoneAccountCreatorStatus linphone_account_creator_constructor_linphone(LinphoneAccountCreator *creator) {
	LinphoneAddress *addr;
	const char *identity = lp_config_get_default_string(creator->core->config, "proxy", "reg_identity", NULL);
	const char *proxy = lp_config_get_default_string(creator->core->config, "proxy", "reg_proxy", NULL);
	const char *route = lp_config_get_default_string(creator->core->config, "proxy", "reg_route", NULL);
	const char *realm = lp_config_get_default_string(creator->core->config, "proxy", "realm", NULL);
	linphone_proxy_config_set_realm(creator->proxy_cfg, realm ? realm : "sip.linphone.org");
	linphone_proxy_config_set_route(creator->proxy_cfg, route ? route : "sip.linphone.org");
	linphone_proxy_config_set_server_addr(creator->proxy_cfg, proxy ? proxy : "sip.linphone.org");
	addr = linphone_address_new(identity ? identity : "sip:username@sip.linphone.org");
	linphone_proxy_config_set_identity_address(creator->proxy_cfg, addr);
	linphone_address_unref(addr);
	return LinphoneAccountCreatorStatusRequestOk;
}

/****************** START OF ACCOUNT USED SECTION *****************************/
static void _is_account_exist_response_cb(LinphoneXmlRpcRequest *request) {
	LinphoneAccountCreator *creator = (LinphoneAccountCreator *)linphone_xml_rpc_request_get_user_data(request);
	LinphoneAccountCreatorStatus status = LinphoneAccountCreatorStatusRequestFailed;
	const char* resp = linphone_xml_rpc_request_get_string_response(request);
	if (linphone_xml_rpc_request_get_status(request) == LinphoneXmlRpcStatusOk) {
		status = (strcmp(resp, "ERROR_ACCOUNT_DOESNT_EXIST") == 0) ? LinphoneAccountCreatorStatusAccountNotExist : (
						(strcmp(resp, "ERROR_ALIAS_DOESNT_EXIST") == 0) ? LinphoneAccountCreatorStatusAccountExist :
													LinphoneAccountCreatorStatusAccountExistWithAlias);
		if (status == LinphoneAccountCreatorStatusAccountExistWithAlias) {
			set_string(&creator->phone_number, resp, FALSE);
		}
	}
	if (creator->cbs->is_account_exist_response_cb != NULL) {
		creator->cbs->is_account_exist_response_cb(creator, status, resp);
	}
	NOTIFY_IF_EXIST(Status, is_account_exist, creator, status, resp)
}

LinphoneAccountCreatorStatus linphone_account_creator_is_account_exist_linphone(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request = NULL;
	if (!creator->username && !creator->phone_number) {
		if (creator->cbs->is_account_exist_response_cb != NULL) {
			creator->cbs->is_account_exist_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters");
		}
		NOTIFY_IF_EXIST(Status, is_account_exist, creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	if (creator->xmlrpc_session) {
		ms_debug("Account creator: is_account_exist (%s=%s, domain=%s)",
			(creator->username) ? "username" : "phone number",
			(creator->username) ? creator->username : creator->phone_number,
			_get_domain(creator));
		request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "get_phone_number_for_account");
		linphone_xml_rpc_request_add_string_arg(request, creator->username ? creator->username : creator->phone_number);
		linphone_xml_rpc_request_add_string_arg(request, _get_domain(creator));
		linphone_xml_rpc_request_set_user_data(request, creator);
		linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request), _is_account_exist_response_cb);
		linphone_xml_rpc_session_send_request(creator->xmlrpc_session, request);
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
	const char* resp = linphone_xml_rpc_request_get_string_response(request);
	if (linphone_xml_rpc_request_get_status(request) == LinphoneXmlRpcStatusOk) {
		status = (strcmp(resp, "OK") == 0) ? LinphoneAccountCreatorStatusAccountCreated
		: (strcmp(resp, "ERROR_CANNOT_SEND_SMS") == 0) ? LinphoneAccountCreatorStatusPhoneNumberInvalid
		: (strcmp(resp, "ERROR_MAX_SMS_EXCEEDED") == 0) ? LinphoneAccountCreatorStatusPhoneNumberOverused
		: (strcmp(resp, "ERROR_ACCOUNT_ALREADY_IN_USE") == 0) ? LinphoneAccountCreatorStatusAccountExist
		: (strcmp(resp, "ERROR_ALIAS_ALREADY_IN_USE") == 0) ? LinphoneAccountCreatorStatusAccountExistWithAlias
		: (strcmp(resp, "ERROR_ALGO_NOT_SUPPORTED") == 0) ? LinphoneAccountCreatorStatusAlgoNotSupported
		: LinphoneAccountCreatorStatusAccountNotCreated;
	}
	if (creator->cbs->create_account_response_cb != NULL) {
		creator->cbs->create_account_response_cb(creator, status, resp);
	}
	NOTIFY_IF_EXIST(Status, create_account, creator, status, resp)
}

static LinphoneXmlRpcRequest * _create_account_with_phone_custom(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request;
	if (!creator->phone_number) {
		return NULL;
	}
	ms_debug("Account creator: create_account_with_phone (phone number=%s, username=%s, domain=%s, language=%s)",
		creator->phone_number,
		(creator->username) ? creator->username : creator->phone_number,
		_get_domain(creator),
		creator->language);

	request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "create_phone_account");
	linphone_xml_rpc_request_add_string_arg(request, creator->phone_number);
	linphone_xml_rpc_request_add_string_arg(request, creator->username ? creator->username : creator->phone_number);
	linphone_xml_rpc_request_add_string_arg(request, creator->password ?
		ha1_for_passwd(creator->username ? creator->username : creator->phone_number, _get_domain(creator), creator->password, creator->algorithm) : "");
	linphone_xml_rpc_request_add_string_arg(request, linphone_core_get_user_agent(creator->core));
	linphone_xml_rpc_request_add_string_arg(request, _get_domain(creator));
	linphone_xml_rpc_request_add_string_arg(request, creator->language);
	linphone_xml_rpc_request_add_string_arg(request, creator->algorithm);
	return request;
}

static LinphoneXmlRpcRequest * _create_account_with_email_custom(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request;
	if (!creator->username || !creator->email || !creator->password) {
		return NULL;
	}
	ms_debug("Account creator: create_account_with_email (username=%s, email=%s, domain=%s)",
		creator->username,
		creator->email,
		_get_domain(creator));

	request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "create_email_account");
	linphone_xml_rpc_request_add_string_arg(request, creator->username);
	linphone_xml_rpc_request_add_string_arg(request, creator->email);
	linphone_xml_rpc_request_add_string_arg(request,
		ha1_for_passwd(creator->username ? creator->username : creator->phone_number, _get_domain(creator), creator->password, creator->algorithm));
	linphone_xml_rpc_request_add_string_arg(request, linphone_core_get_user_agent(creator->core));
	linphone_xml_rpc_request_add_string_arg(request, _get_domain(creator));
	linphone_xml_rpc_request_add_string_arg(request, creator->algorithm);
	return request;
}

LinphoneAccountCreatorStatus linphone_account_creator_create_account_linphone(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request = NULL;
	char *identity = _get_identity(creator);
	if (!identity || (!(request = _create_account_with_phone_custom(creator))
		&& !(request = _create_account_with_email_custom(creator)))) {
		if (creator->cbs->create_account_response_cb != NULL) {
			creator->cbs->create_account_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters");
		}
		NOTIFY_IF_EXIST(Status, create_account, creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		if (identity) ms_free(identity);
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	if (creator->xmlrpc_session) {
		linphone_xml_rpc_request_set_user_data(request, creator);
		linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request), _create_account_cb_custom);
		linphone_xml_rpc_session_send_request(creator->xmlrpc_session, request);
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
	const char* resp = linphone_xml_rpc_request_get_string_response(request);
	if (linphone_xml_rpc_request_get_status(request) == LinphoneXmlRpcStatusOk) {
		status = (strcmp(resp, "OK") == 0) ? LinphoneAccountCreatorStatusAccountCreated
		: (strcmp(resp, "ERROR_USERNAME_PARAMETER_NOT_FOUND") == 0) ? LinphoneAccountCreatorStatusMissingArguments
		: (strcmp(resp, "ERROR_ACCOUNT_DOESNT_EXIST") == 0) ? LinphoneAccountCreatorStatusAccountNotExist
		: (strcmp(resp, "ERROR_ALGO_NOT_SUPPORTED") == 0) ? LinphoneAccountCreatorStatusAlgoNotSupported
		: (strstr(resp, "ERROR_") == 0) ? LinphoneAccountCreatorStatusRequestFailed
		: LinphoneAccountCreatorStatusAccountNotCreated;
	}
	if (creator->cbs->delete_account_response_cb != NULL) {
		creator->cbs->delete_account_response_cb(creator, status, resp);
	}
	NOTIFY_IF_EXIST(Status, delete_account, creator, status, resp)
}

LinphoneAccountCreatorStatus linphone_account_creator_delete_account_linphone(LinphoneAccountCreator *creator) {
	if ((!creator->username && !creator->phone_number) || !creator->password) {
		if (creator->cbs->delete_account_response_cb != NULL) {
			creator->cbs->delete_account_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters");
		}
		NOTIFY_IF_EXIST(Status, delete_account, creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	ms_debug("Account creator: delete_account (username=%s, password=%s, domain=%s)",
		creator->username,
		creator->password,
		linphone_proxy_config_get_domain(creator->proxy_cfg));

	LinphoneXmlRpcRequest *request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "delete_account");
	linphone_xml_rpc_request_add_string_arg(request, creator->username ? creator->username : creator->phone_number);
	linphone_xml_rpc_request_add_string_arg(request, creator->password);
	linphone_xml_rpc_request_add_string_arg(request, linphone_proxy_config_get_domain(creator->proxy_cfg));
	linphone_xml_rpc_request_set_user_data(request, creator);
	linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request), _delete_linphone_account_response_cb);
	linphone_xml_rpc_session_send_request(creator->xmlrpc_session, request);
	linphone_xml_rpc_request_unref(request);

	return LinphoneAccountCreatorStatusRequestOk;
}

/****************** END OF CREATE ACCOUNT SECTION *****************************/

/****************** START OF VALIDATE ACCOUNT SECTION *************************/
static void _activate_account_cb_custom(LinphoneXmlRpcRequest *request) {
	LinphoneAccountCreator *creator = (LinphoneAccountCreator *)linphone_xml_rpc_request_get_user_data(request);
	LinphoneAccountCreatorStatus status = LinphoneAccountCreatorStatusRequestFailed;
	const char* resp = linphone_xml_rpc_request_get_string_response(request);
	if (linphone_xml_rpc_request_get_status(request) == LinphoneXmlRpcStatusOk) {
		if (strcmp(resp, "ERROR_ACCOUNT_ALREADY_ACTIVATED") == 0) {
			status = LinphoneAccountCreatorStatusAccountAlreadyActivated;
		} else if (strcmp(resp, "ERROR_KEY_DOESNT_MATCH") == 0) {
			status = LinphoneAccountCreatorStatusWrongActivationCode;
		} else if (strcmp(resp, "ERROR_ALGO_NOT_SUPPORTED") == 0) {
			status = LinphoneAccountCreatorStatusAlgoNotSupported;
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
	NOTIFY_IF_EXIST(Status, activate_account, creator, status, resp)
}

LinphoneAccountCreatorStatus linphone_account_creator_activate_account_linphone(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request = NULL;
	if (!creator->phone_number || !creator->activation_code) {
		if (creator->cbs->is_account_activated_response_cb != NULL) {
			creator->cbs->is_account_activated_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters");
		}
		NOTIFY_IF_EXIST(Status, activate_account, creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	if (creator->xmlrpc_session) {
		ms_debug("Account creator: activate_account_phone (phone number=%s, username=%s, activation code=%s, domain=%s)",
			creator->phone_number,
			creator->username ? creator->username : creator->phone_number,
			creator->activation_code,
			_get_domain(creator));

		request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "activate_phone_account");
		linphone_xml_rpc_request_add_string_arg(request, creator->phone_number);
		linphone_xml_rpc_request_add_string_arg(request, creator->username ? creator->username : creator->phone_number);
		linphone_xml_rpc_request_add_string_arg(request, creator->activation_code);
		linphone_xml_rpc_request_add_string_arg(request, _get_domain(creator));
		linphone_xml_rpc_request_add_string_arg(request, creator->algorithm);
		linphone_xml_rpc_request_set_user_data(request, creator);
		linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request), _activate_account_cb_custom);
		linphone_xml_rpc_session_send_request(creator->xmlrpc_session, request);
		linphone_xml_rpc_request_unref(request);
		return LinphoneAccountCreatorStatusRequestOk;
	}
	return LinphoneAccountCreatorStatusRequestFailed;
}

LinphoneAccountCreatorStatus linphone_account_creator_activate_email_account_linphone(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request = NULL;
	if (!creator->activation_code || !creator->username) {
		if (creator->cbs->is_account_activated_response_cb != NULL) {
			creator->cbs->is_account_activated_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters");
		}
		NOTIFY_IF_EXIST(Status, activate_account, creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	if (creator->xmlrpc_session) {
		ms_debug("Account creator: activate_account_email (username=%s, activation code=%s, domain=%s)",
			creator->username,
			creator->activation_code,
			_get_domain(creator));

		request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "activate_email_account");
		linphone_xml_rpc_request_add_string_arg(request, creator->username);
		linphone_xml_rpc_request_add_string_arg(request, creator->activation_code);
		linphone_xml_rpc_request_add_string_arg(request, _get_domain(creator));
        linphone_xml_rpc_request_add_string_arg(request, creator->algorithm);
		linphone_xml_rpc_request_set_user_data(request, creator);
		linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request), _activate_account_cb_custom);
		linphone_xml_rpc_session_send_request(creator->xmlrpc_session, request);
		linphone_xml_rpc_request_unref(request);
		return LinphoneAccountCreatorStatusRequestOk;
	}
	return LinphoneAccountCreatorStatusRequestFailed;
}

static void get_linphone_confirmation_key_response_cb(LinphoneXmlRpcRequest *request) {
	LinphoneAccountCreator *creator = (LinphoneAccountCreator *)linphone_xml_rpc_request_get_user_data(request);
	LinphoneAccountCreatorStatus status = LinphoneAccountCreatorStatusRequestFailed;
	const char* resp = linphone_xml_rpc_request_get_string_response(request);
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
	NOTIFY_IF_EXIST(Status, confirmation_key, creator, status, resp)
}

LinphoneAccountCreatorStatus linphone_account_creator_get_confirmation_key_linphone(LinphoneAccountCreator *creator) {
	if (!creator->username || !creator->password) {
		if (creator->cbs->confirmation_key_response_cb != NULL) {
			creator->cbs->confirmation_key_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters");
		}
		NOTIFY_IF_EXIST(Status, confirmation_key, creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	ms_debug("Account creator: confirmation_key (username=%s, password=%s, domain=%s)",
		creator->username,
		creator->password,
		linphone_proxy_config_get_domain(creator->proxy_cfg));

	LinphoneXmlRpcRequest *request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "get_confirmation_key");
	linphone_xml_rpc_request_add_string_arg(request, creator->username);
	linphone_xml_rpc_request_add_string_arg(request, ha1_for_passwd(creator->username, linphone_proxy_config_get_domain(creator->proxy_cfg), creator->password, creator->algorithm));
	linphone_xml_rpc_request_add_string_arg(request, linphone_proxy_config_get_domain(creator->proxy_cfg));
	linphone_xml_rpc_request_add_string_arg(request, creator->algorithm);
	linphone_xml_rpc_request_set_user_data(request, creator);
	linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request), get_linphone_confirmation_key_response_cb);
	linphone_xml_rpc_session_send_request(creator->xmlrpc_session, request);
	linphone_xml_rpc_request_unref(request);
	return LinphoneAccountCreatorStatusRequestOk;
}
/****************** END OF CREATE VALIDATE ACCOUNT SECTION ********************/

/****************** START OF ACCOUNT VALIDATED SECTION ************************/
static void _is_account_activated_cb_custom(LinphoneXmlRpcRequest *request) {
	LinphoneAccountCreator *creator = (LinphoneAccountCreator *)linphone_xml_rpc_request_get_user_data(request);
	LinphoneAccountCreatorStatus status = LinphoneAccountCreatorStatusRequestFailed;
	const char* resp = linphone_xml_rpc_request_get_string_response(request);
	if (linphone_xml_rpc_request_get_status(request) == LinphoneXmlRpcStatusOk) {
		status = (strcmp(resp, "OK") == 0) ? LinphoneAccountCreatorStatusAccountActivated : LinphoneAccountCreatorStatusAccountNotActivated;
	}
	if (creator->cbs->is_account_activated_response_cb != NULL) {
		creator->cbs->is_account_activated_response_cb(creator, status, resp);
	}
	NOTIFY_IF_EXIST(Status, is_account_activated, creator, status, resp)
}

LinphoneAccountCreatorStatus linphone_account_creator_is_account_activated_linphone(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request = NULL;
	char *identity = _get_identity(creator);
	if (!identity) {
		if (creator->cbs->is_account_activated_response_cb != NULL) {
			creator->cbs->is_account_activated_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters");
		}
		NOTIFY_IF_EXIST(Status, is_account_activated, creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	if (creator->xmlrpc_session) {
		ms_debug("Account creator: is_account_activated (username=%s, domain=%s)",
			creator->username ? creator->username : creator->phone_number,
			_get_domain(creator));

		request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "is_account_activated");
		linphone_xml_rpc_request_add_string_arg(request, creator->username ? creator->username : creator->phone_number);
		linphone_xml_rpc_request_add_string_arg(request, _get_domain(creator));
        linphone_xml_rpc_request_add_string_arg(request, creator->algorithm);
		linphone_xml_rpc_request_set_user_data(request, creator);
		linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request), _is_account_activated_cb_custom);
		linphone_xml_rpc_session_send_request(creator->xmlrpc_session, request);
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
	const char* resp = linphone_xml_rpc_request_get_string_response(request);
	if (linphone_xml_rpc_request_get_status(request) == LinphoneXmlRpcStatusOk) {
		status = (strcmp(resp, "OK_ACCOUNT") == 0) ? LinphoneAccountCreatorStatusAliasIsAccount
		: (strcmp(resp, "OK_ALIAS") == 0) ? LinphoneAccountCreatorStatusAliasExist
		: LinphoneAccountCreatorStatusAliasNotExist;
	}
	if (creator->cbs->is_alias_used_response_cb != NULL) {
		creator->cbs->is_alias_used_response_cb(creator, status, resp);
	}
	NOTIFY_IF_EXIST(Status, is_alias_used, creator, status, resp)
}

LinphoneAccountCreatorStatus linphone_account_creator_is_phone_number_used_linphone(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request = NULL;
	char *identity = _get_identity(creator);
	if (!identity) {
		if (creator->cbs->is_alias_used_response_cb != NULL) {
			creator->cbs->is_alias_used_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters");
		}
		NOTIFY_IF_EXIST(Status, is_alias_used, creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	if (creator->xmlrpc_session) {
		ms_debug("Account creator: is_phone_number_used (phone number=%s, domain=%s)",
			creator->phone_number,
			_get_domain(creator));

		request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "is_phone_number_used");
		linphone_xml_rpc_request_add_string_arg(request, creator->phone_number);
		linphone_xml_rpc_request_add_string_arg(request, _get_domain(creator));
		linphone_xml_rpc_request_set_user_data(request, creator);
		linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request), _is_phone_number_used_cb_custom);
		linphone_xml_rpc_session_send_request(creator->xmlrpc_session, request);
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
	const char* resp = linphone_xml_rpc_request_get_string_response(request);

	status = (strcmp(resp, "OK") == 0) ? LinphoneAccountCreatorStatusRequestOk
		: (strcmp(resp, "ERROR_CANNOT_SEND_SMS") == 0) ? LinphoneAccountCreatorStatusPhoneNumberInvalid
		: (strcmp(resp, "ERROR_MAX_SMS_EXCEEDED") == 0) ? LinphoneAccountCreatorStatusPhoneNumberOverused
		: LinphoneAccountCreatorStatusAccountNotLinked;

	if (creator->cbs->link_account_response_cb != NULL) {
		creator->cbs->link_account_response_cb(creator, status, resp);
	}
	NOTIFY_IF_EXIST(Status, link_account, creator, status, resp)
}

LinphoneAccountCreatorStatus linphone_account_creator_link_phone_number_with_account_linphone(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request = NULL;
	if (!creator->phone_number || !creator->username) {
		if (creator->cbs->link_account_response_cb != NULL) {
			creator->cbs->link_account_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters");
		}
		NOTIFY_IF_EXIST(Status, link_account, creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	if (creator->xmlrpc_session) {
		ms_debug("Account creator: link_phone_number_with_account (phone number=%s, username=%s, domain=%s, language=%s)",
			creator->phone_number,
			creator->username,
			_get_domain(creator),
			creator->language);

		request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "link_phone_number_with_account");
		linphone_xml_rpc_request_add_string_arg(request, creator->phone_number);
		linphone_xml_rpc_request_add_string_arg(request, creator->username);
		linphone_xml_rpc_request_add_string_arg(request, _get_domain(creator));
		linphone_xml_rpc_request_add_string_arg(request, creator->language);
		linphone_xml_rpc_request_set_user_data(request, creator);
		linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request), _link_phone_number_with_account_cb_custom);
		linphone_xml_rpc_session_send_request(creator->xmlrpc_session, request);
		linphone_xml_rpc_request_unref(request);
		return LinphoneAccountCreatorStatusRequestOk;
	}
	return LinphoneAccountCreatorStatusRequestFailed;
}

static void _get_phone_number_for_account_cb_custom(LinphoneXmlRpcRequest *request) {
	LinphoneAccountCreator *creator = (LinphoneAccountCreator *)linphone_xml_rpc_request_get_user_data(request);
	LinphoneAccountCreatorStatus status = LinphoneAccountCreatorStatusRequestFailed;
	const char* resp = linphone_xml_rpc_request_get_string_response(request);
	if (linphone_xml_rpc_request_get_status(request) == LinphoneXmlRpcStatusOk) {
		status = (strcmp(resp, "ERROR_USERNAME_PARAMETER_NOT_FOUND") == 0
			|| strcmp(resp, "ERROR_ACCOUNT_DOESNT_EXIST") == 0
			|| strcmp(resp, "ERROR_ALIAS_DOESNT_EXIST") == 0) ? LinphoneAccountCreatorStatusAccountNotLinked : LinphoneAccountCreatorStatusAccountLinked;
	}
	if (creator->cbs->is_account_linked_response_cb != NULL) {
		creator->cbs->is_account_linked_response_cb(creator, status, resp);
	}
	NOTIFY_IF_EXIST(Status, is_account_linked, creator, status, resp)
}

LinphoneAccountCreatorStatus linphone_account_creator_is_account_linked_linphone(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request = NULL;
	if (!creator->username || !_get_domain(creator)) {
		if (creator->cbs->is_account_linked_response_cb != NULL) {
			creator->cbs->is_account_linked_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters");
		}
		NOTIFY_IF_EXIST(Status, is_account_linked, creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	if (creator->xmlrpc_session) {
		ms_debug("Account creator: is_account_linked (username=%s, domain=%s)",
			creator->username,
			_get_domain(creator));

		request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "get_phone_number_for_account");
		linphone_xml_rpc_request_add_string_arg(request, creator->username);
		linphone_xml_rpc_request_add_string_arg(request, _get_domain(creator));
		linphone_xml_rpc_request_set_user_data(request, creator);
		linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request), _get_phone_number_for_account_cb_custom);
		linphone_xml_rpc_session_send_request(creator->xmlrpc_session, request);
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
	const char* resp = linphone_xml_rpc_request_get_string_response(request);
	if (linphone_xml_rpc_request_get_status(request) == LinphoneXmlRpcStatusOk) {
		status = (strstr(resp, "ERROR_") == resp) ? LinphoneAccountCreatorStatusAccountNotActivated : LinphoneAccountCreatorStatusAccountActivated;
	}
	if (creator->cbs->activate_alias_response_cb != NULL) {
		creator->cbs->activate_alias_response_cb(creator, status, resp);
	}
	NOTIFY_IF_EXIST(Status, activate_alias, creator, status, resp)
}

LinphoneAccountCreatorStatus linphone_account_creator_activate_phone_number_link_linphone(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request = NULL;
	if (!creator->phone_number || !creator->username || !creator->activation_code || (!creator->password && !creator->ha1) || !_get_domain(creator)) {
		if (creator->cbs->activate_alias_response_cb != NULL) {
			creator->cbs->activate_alias_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters");
		}
		NOTIFY_IF_EXIST(Status, activate_alias, creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	if (creator->xmlrpc_session) {
		ms_debug("Account creator: activate_phone_number_link (phone number=%s, username=%s, activation code=%s, domain=%s)",
			creator->phone_number,
			creator->username,
			creator->activation_code,
			_get_domain(creator));

		request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "activate_phone_number_link");
		linphone_xml_rpc_request_add_string_arg(request, creator->phone_number);
		linphone_xml_rpc_request_add_string_arg(request, creator->username);
		linphone_xml_rpc_request_add_string_arg(request, creator->activation_code);
		linphone_xml_rpc_request_add_string_arg(request, creator->ha1 ? creator->ha1 : ha1_for_passwd(creator->username, _get_domain(creator), creator->password, creator->algorithm));
		linphone_xml_rpc_request_add_string_arg(request, _get_domain(creator));
		linphone_xml_rpc_request_add_string_arg(request, creator->algorithm);
		linphone_xml_rpc_request_set_user_data(request, creator);
		linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request), _activate_phone_number_link_cb_custom);
		linphone_xml_rpc_session_send_request(creator->xmlrpc_session, request);
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
	const char* resp = linphone_xml_rpc_request_get_string_response(request);
	if (linphone_xml_rpc_request_get_status(request) == LinphoneXmlRpcStatusOk) {
		if (strstr(resp, "ERROR_") == resp) {
			status = (strcmp(resp, "ERROR_CANNOT_SEND_SMS") == 0) ? LinphoneAccountCreatorStatusPhoneNumberInvalid
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
	NOTIFY_IF_EXIST(Status, recover_account, creator, status, resp)
}

LinphoneAccountCreatorStatus linphone_account_creator_recover_phone_account_linphone(LinphoneAccountCreator *creator) {
	LinphoneXmlRpcRequest *request = NULL;
	if (!creator->phone_number) {
		if (creator->cbs->recover_account_response_cb != NULL) {
			creator->cbs->recover_account_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters");
		}
		NOTIFY_IF_EXIST(Status, recover_account, creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}

	if (creator->xmlrpc_session) {
		ms_debug("Account creator: recover_phone_account (phone number=%s, domain=%s, language=%s)",
			creator->phone_number,
			_get_domain(creator),
			creator->language);

		request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "recover_phone_account");
		linphone_xml_rpc_request_add_string_arg(request, creator->phone_number);
		linphone_xml_rpc_request_add_string_arg(request, _get_domain(creator));
		linphone_xml_rpc_request_add_string_arg(request, creator->language);
		linphone_xml_rpc_request_set_user_data(request, creator);
		linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request), _recover_phone_account_cb_custom);
		linphone_xml_rpc_session_send_request(creator->xmlrpc_session, request);
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
	const char* resp = linphone_xml_rpc_request_get_string_response(request);
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
	NOTIFY_IF_EXIST(Status, update_account, creator, status, resp)
}

LinphoneAccountCreatorStatus linphone_account_creator_update_password_linphone(LinphoneAccountCreator *creator){
	LinphoneXmlRpcRequest *request = NULL;
	char *identity = _get_identity(creator);
	const char* new_pwd = (const char*)linphone_account_creator_get_user_data(creator);
	if (!identity ||
			((!creator->username && !creator->phone_number)
				|| !_get_domain(creator)
				|| (!creator->password && !creator->ha1) || !new_pwd
			)
		) {
		if (creator->cbs->update_account_response_cb != NULL) {
			creator->cbs->update_account_response_cb(creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters");
		}
		NOTIFY_IF_EXIST(Status, update_account, creator, LinphoneAccountCreatorStatusMissingArguments, "Missing required parameters")
		return LinphoneAccountCreatorStatusMissingArguments;
	}
	bctbx_free(identity);

	if (creator->xmlrpc_session) {
		const char *username = creator->username ? creator->username : creator->phone_number;
		char *ha1 = bctbx_strdup(creator->ha1 ? creator->ha1 : ha1_for_passwd(username, _get_domain(creator), creator->password, creator->algorithm));
		char *new_ha1 = bctbx_strdup(ha1_for_passwd(username, _get_domain(creator), new_pwd, creator->algorithm));

		ms_debug("Account creator: update_password (username=%s, domain=%s)",
			creator->username,
			_get_domain(creator));

		request = linphone_xml_rpc_request_new(LinphoneXmlRpcArgString, "update_hash");
		linphone_xml_rpc_request_add_string_arg(request, username);
		linphone_xml_rpc_request_add_string_arg(request, ha1);
		linphone_xml_rpc_request_add_string_arg(request, new_ha1);
		linphone_xml_rpc_request_add_string_arg(request, _get_domain(creator));
		linphone_xml_rpc_request_add_string_arg(request, creator->algorithm);
		linphone_xml_rpc_request_set_user_data(request, creator);
		linphone_xml_rpc_request_cbs_set_response(linphone_xml_rpc_request_get_callbacks(request), _password_updated_cb_custom);
		linphone_xml_rpc_session_send_request(creator->xmlrpc_session, request);
		linphone_xml_rpc_request_unref(request);

		bctbx_free(ha1);
		bctbx_free(new_ha1);

		return LinphoneAccountCreatorStatusRequestOk;
	}
	return LinphoneAccountCreatorStatusRequestFailed;
}
/****************** END OF UPDATE ACCOUNT **************************/

/************************** End Account Creator Linphone **************************/
