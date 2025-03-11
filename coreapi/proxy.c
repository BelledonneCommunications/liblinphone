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
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <ctype.h>
#include <string>

#include "linphone/core.h"
#include "linphone/core_utils.h"
#include "linphone/lpconfig.h"
#include "linphone/sipsetup.h"

#include <bctoolbox/defs.h>

#include "mediastreamer2/mediastream.h"

#include "account/account-params.h"
#include "account/account.h"
#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "core/core-p.h"
#include "core/core.h"
#include "dial-plan/dial-plan.h"
#include "enum.h"
#include "event/event-publish.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-dial-plan.h"
#include "private.h"

using namespace LinphonePrivate;

LinphoneProxyConfigAddressComparisonResult linphone_proxy_config_address_equal(const LinphoneAddress *a,
                                                                               const LinphoneAddress *b) {
	std::shared_ptr<const Address> aAddr = a ? Address::toCpp(a)->getSharedFromThis() : nullptr;
	std::shared_ptr<const Address> bAddr = b ? Address::toCpp(b)->getSharedFromThis() : nullptr;
	return (LinphoneProxyConfigAddressComparisonResult)Account::compareLinphoneAddresses(aAddr, bAddr);
}

LinphoneProxyConfigAddressComparisonResult
linphone_proxy_config_is_server_config_changed(const LinphoneProxyConfig *cfg) {
	return (LinphoneProxyConfigAddressComparisonResult)Account::toCpp(cfg->account)->isServerConfigChanged();
}

static void linphone_proxy_config_init(LinphoneCore *lc, LinphoneProxyConfig *cfg) {
	LinphoneAccountParams *params = linphone_account_params_new(lc, TRUE);
	cfg->account = linphone_account_new_with_config(lc, params, cfg);
	linphone_account_params_unref(params);
	cfg->edit = NULL;
}

static void _linphone_proxy_config_destroy(LinphoneProxyConfig *cfg);

BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneProxyConfig);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneProxyConfig,
                           belle_sip_object_t,
                           (belle_sip_object_destroy_t)_linphone_proxy_config_destroy,
                           NULL, // clone
                           NULL, // marshal
                           FALSE);

LinphoneProxyConfig *linphone_core_create_proxy_config(LinphoneCore *lc) {
	LinphoneProxyConfig *cfg = belle_sip_object_new(LinphoneProxyConfig);
	linphone_proxy_config_init(lc, cfg);
	return cfg;
}

void _linphone_proxy_config_destroy(LinphoneProxyConfig *cfg) {
	if (cfg->account) linphone_account_unref(cfg->account);
	if (cfg->edit) linphone_account_params_unref(cfg->edit);
}

void _linphone_proxy_config_release(LinphoneProxyConfig *cfg) {
	Account::toCpp(cfg->account)->releaseOps();
	belle_sip_object_unref(cfg);
}

LinphoneProxyConfig *linphone_proxy_config_ref(LinphoneProxyConfig *cfg) {
	belle_sip_object_ref(cfg);
	return cfg;
}

void linphone_proxy_config_unref(LinphoneProxyConfig *cfg) {
	belle_sip_object_unref(cfg);
}

LinphoneStatus linphone_proxy_config_set_server_addr(LinphoneProxyConfig *cfg, const char *server_addr) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	LinphoneStatus status = linphone_account_params_set_server_addr(cfg->edit, server_addr);
	linphone_proxy_config_done(cfg);
	return status;
}

LinphoneStatus linphone_proxy_config_set_identity_address(LinphoneProxyConfig *cfg, LinphoneAddress *addr) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	LinphoneStatus status = linphone_account_params_set_identity_address(cfg->edit, addr);
	linphone_proxy_config_done(cfg);
	return status;
}

const char *linphone_proxy_config_get_domain(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);

	const LinphoneAddress *identity_address = linphone_account_params_get_identity_address(params);
	return identity_address ? linphone_address_get_domain(identity_address) : NULL;
}

LinphoneStatus linphone_proxy_config_set_route(LinphoneProxyConfig *cfg, const char *route) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	bctbx_list_t *list = NULL;
	LinphoneStatus ret;

	if (route != NULL && route[0] != '\0') {
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
	}

	ret = linphone_account_params_set_routes_addresses(cfg->edit, list);

	bctbx_list_free_with_data(list, (bctbx_list_free_func)linphone_address_unref);
	linphone_proxy_config_done(cfg);
	return ret;
}

LinphoneStatus linphone_proxy_config_set_routes(LinphoneProxyConfig *cfg, const bctbx_list_t *routes) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	LinphoneStatus status = AccountParams::toCpp(cfg->edit)->setRoutesFromStringList(routes);
	linphone_proxy_config_done(cfg);
	return status;
}

void linphone_proxy_config_enableregister(LinphoneProxyConfig *cfg, bool_t val) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_register_enabled(cfg->edit, val);
	linphone_proxy_config_done(cfg);
}

void linphone_proxy_config_set_expires(LinphoneProxyConfig *cfg, int val) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_expires(cfg->edit, val);
	linphone_proxy_config_done(cfg);
}

void linphone_proxy_config_enable_publish(LinphoneProxyConfig *cfg, bool_t val) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_publish_enabled(cfg->edit, val);
	linphone_proxy_config_done(cfg);
}

void linphone_proxy_config_pause_register(LinphoneProxyConfig *cfg) {
	linphone_account_pause_register(cfg->account);
}

void linphone_proxy_config_edit(LinphoneProxyConfig *cfg) {
	if (cfg->edit) {
		ms_message("Proxy config [%p] is already on edit mode", cfg);
		return;
	}

	cfg->edit = linphone_account_params_clone(linphone_account_get_params(cfg->account));
}

void linphone_proxy_config_apply(LinphoneProxyConfig *cfg, LinphoneCore *lc) {
	Account::toCpp(cfg->account)->apply(lc);
}

void linphone_proxy_config_stop_refreshing(LinphoneProxyConfig *cfg) {
	Account::toCpp(cfg->account)->stopRefreshing();
}

void _linphone_proxy_config_unregister(LinphoneProxyConfig *obj) {
	Account::toCpp(obj->account)->unregister();
}

void linphone_proxy_config_refresh_register(LinphoneProxyConfig *cfg) {
	linphone_account_refresh_register(cfg->account);
}

void linphone_proxy_config_set_dial_prefix(LinphoneProxyConfig *cfg, const char *prefix) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_international_prefix(cfg->edit, prefix);
	linphone_proxy_config_done(cfg);
}

const char *linphone_proxy_config_get_dial_prefix(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_international_prefix(params);
}

void linphone_proxy_config_set_dial_escape_plus(LinphoneProxyConfig *cfg, bool_t val) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_dial_escape_plus_enabled(cfg->edit, val);
	linphone_proxy_config_done(cfg);
}

bool_t linphone_proxy_config_get_dial_escape_plus(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_dial_escape_plus_enabled(params);
}

void linphone_proxy_config_enable_quality_reporting(LinphoneProxyConfig *cfg, bool_t val) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_quality_reporting_enabled(cfg->edit, val);
	linphone_proxy_config_done(cfg);
}

bool_t linphone_proxy_config_quality_reporting_enabled(LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params =
	    cfg->edit ? cfg->edit : ((cfg->account) ? linphone_account_get_params(cfg->account) : NULL);
	return (params ? linphone_account_params_get_quality_reporting_enabled(params)
	               : !!linphone_config_get_default_int(linphone_core_get_config(linphone_proxy_config_get_core(cfg)),
	                                                   "proxy", "quality_reporting_enabled", false));
}

void linphone_proxy_config_set_quality_reporting_interval(LinphoneProxyConfig *cfg, int interval) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_quality_reporting_interval(cfg->edit, interval);
	linphone_proxy_config_done(cfg);
}

int linphone_proxy_config_get_quality_reporting_interval(LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_quality_reporting_interval(params);
}

void linphone_proxy_config_set_quality_reporting_collector(LinphoneProxyConfig *cfg, const char *collector) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_quality_reporting_collector(cfg->edit, collector);
	linphone_proxy_config_done(cfg);
}

const char *linphone_proxy_config_get_quality_reporting_collector(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_quality_reporting_collector(params);
}

bool_t linphone_proxy_config_normalize_number(LinphoneProxyConfig *proxy,
                                              const char *username,
                                              char *result,
                                              size_t result_len) {
	char *normalized_phone = linphone_proxy_config_normalize_phone_number(proxy, username);
	const char *output = normalized_phone ? normalized_phone : username;
	memset(result, 0, result_len);
	memcpy(result, output, MIN(strlen(output) + 1, result_len));
	ms_free(normalized_phone);
	return output != username;
} // TODO: not used but blacklisted in wrapper

char *linphone_proxy_config_normalize_phone_number(LinphoneProxyConfig *proxy, const char *username) {
	return linphone_account_normalize_phone_number(proxy ? proxy->account : NULL, username);
}

LinphoneAddress *linphone_proxy_config_normalize_sip_uri(LinphoneProxyConfig *proxy, const char *username) {
	return linphone_account_normalize_sip_uri(proxy ? proxy->account : NULL, username);
}

void linphone_proxy_config_set_etag(LinphoneProxyConfig *cfg, const char *sip_etag) {
	Account::toCpp(cfg->account)->setSipEtag(sip_etag);
}

/**
 * Commits modification made to the proxy configuration.
 * This function is called every time the proxy configuration is edited in order to propagate the changes to the account
 *parameters as well. In fact as we moved towards deprecating the ProxyConfig object, the account must always be up to
 *date
 **/
LinphoneStatus linphone_proxy_config_done(LinphoneProxyConfig *cfg) {
	if (!cfg->edit) {
		ms_message("Done called before edit on proxy config [%p]", cfg);
		return -1;
	}

	LinphoneStatus ret = linphone_account_set_params(cfg->account, cfg->edit);
	linphone_account_params_unref(cfg->edit);
	cfg->edit = NULL;

	return ret;
}

const char *linphone_proxy_config_get_realm(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_realm(params);
}

void linphone_proxy_config_set_realm(LinphoneProxyConfig *cfg, const char *realm) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_realm(cfg->edit, realm);
	linphone_proxy_config_done(cfg);
}

int linphone_proxy_config_send_publish(LinphoneProxyConfig *proxy, LinphonePresenceModel *presence) {
	Account::toCpp(proxy->account)->setPresenceModel(presence);
	return Account::toCpp(proxy->account)->sendPublish();
}

void _linphone_proxy_config_unpublish(LinphoneProxyConfig *obj) {
	Account::toCpp(obj->account)->unpublish();
}

const char *linphone_proxy_config_get_route(const LinphoneProxyConfig *cfg) {
	const bctbx_list_t *list = linphone_proxy_config_get_routes(cfg);
	if (list != NULL) {
		const char *route = (const char *)bctbx_list_get_data(list);
		return route;
	}

	return NULL;
}

const bctbx_list_t *linphone_proxy_config_get_routes(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return AccountParams::toCpp(params)->getRoutesCString();
}

const LinphoneAddress *linphone_proxy_config_get_identity_address(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_identity_address(params);
}

const char *linphone_proxy_config_get_identity(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_identity(params);
}

bool_t linphone_proxy_config_publish_enabled(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_publish_enabled(params);
}

const char *linphone_proxy_config_get_server_addr(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_server_addr(params);
}

/**
 * @return the duration of registration.
 **/
int linphone_proxy_config_get_expires(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_expires(params);
}

bool_t linphone_proxy_config_register_enabled(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_register_enabled(params);
}

void linphone_proxy_config_set_contact_parameters(LinphoneProxyConfig *cfg, const char *contact_params) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_contact_parameters(cfg->edit, contact_params);
	linphone_proxy_config_done(cfg);
}

void linphone_proxy_config_set_contact_uri_parameters(LinphoneProxyConfig *cfg, const char *contact_uri_params) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_contact_uri_parameters(cfg->edit, contact_uri_params);
	linphone_proxy_config_done(cfg);
}

const char *linphone_proxy_config_get_contact_parameters(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_contact_parameters(params);
}

const char *linphone_proxy_config_get_contact_uri_parameters(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_contact_uri_parameters(params);
}

struct _LinphoneCore *linphone_proxy_config_get_core(const LinphoneProxyConfig *cfg) {
	return linphone_account_get_core(cfg->account);
}

const char *linphone_proxy_config_get_custom_header(LinphoneProxyConfig *cfg, const char *header_name) {
	return linphone_account_get_custom_header(cfg->account, header_name);
}

void linphone_proxy_config_set_custom_header(LinphoneProxyConfig *cfg,
                                             const char *header_name,
                                             const char *header_value) {
	linphone_account_set_custom_header(cfg->account, header_name, header_value);
}

LinphoneProxyConfig *linphone_proxy_config_get_dependency(LinphoneProxyConfig *cfg) {
	LinphoneAccount *dependency = linphone_account_get_dependency(cfg->account);
	return dependency ? Account::toCpp(dependency)->getConfig() : NULL;
}

void linphone_proxy_config_set_dependency(LinphoneProxyConfig *cfg, LinphoneProxyConfig *dependency) {
	linphone_account_set_dependency(cfg->account, dependency->account);
}

const char *linphone_proxy_config_get_idkey(LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_idkey(params);
}

void linphone_proxy_config_set_idkey(LinphoneProxyConfig *cfg, const char *idkey) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_idkey(cfg->edit, idkey);
	linphone_proxy_config_done(cfg);
}

LinphoneAccount *linphone_proxy_config_get_account(LinphoneProxyConfig *cfg) {
	return cfg->account;
}

LinphoneStatus linphone_core_add_proxy_config(LinphoneCore *lc, LinphoneProxyConfig *cfg) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(lc)->addAccount(Account::toCpp(cfg->account)->getSharedFromThis());
}

// If an account dependency is removed, restore 'normal' behavior for previously dependent accounts
void linphone_core_remove_dependent_account(LinphoneCore *lc, LinphoneAccount *account) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->removeDependentAccount(Account::toCpp(account)->getSharedFromThis());
}

void linphone_core_remove_proxy_config(LinphoneCore *lc, LinphoneProxyConfig *cfg) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->removeAccount(Account::toCpp(cfg->account)->getSharedFromThis());
}

void linphone_core_clear_proxy_config(LinphoneCore *lc) {
	linphone_core_clear_accounts(lc);
}

int linphone_core_get_default_proxy_config_index(LinphoneCore *lc) {
	return linphone_core_get_default_account_index(lc);
}

LinphoneProxyConfig *linphone_core_get_proxy_config_by_idkey(LinphoneCore *lc, const char *idkey) {
	LinphoneAccount *account = linphone_core_get_account_by_idkey(lc, idkey);
	if (account) {
		return Account::toCpp(account)->getConfig();
	}
	return NULL;
}

void linphone_core_set_default_proxy_config(LinphoneCore *lc, LinphoneProxyConfig *config) {
	linphone_core_set_default_account(lc, config ? config->account : NULL);
}

void linphone_core_set_default_proxy_index(LinphoneCore *lc, int index) {
	linphone_core_set_default_account_index(lc, index);
}

int linphone_core_get_default_proxy(LinphoneCore *lc, LinphoneProxyConfig **config) {
	LinphoneAccount *account = linphone_core_get_default_account(lc);
	if (account && config) {
		*config = Account::toCpp(account)->getConfig();
	}
	return linphone_core_get_default_account_index(lc);
}

LinphoneProxyConfig *linphone_core_get_default_proxy_config(const LinphoneCore *lc) {
	LinphoneAccount *account = linphone_core_get_default_account(lc);
	if (account) {
		return Account::toCpp(account)->getConfig();
	}
	return NULL;
}

const bctbx_list_t *linphone_core_get_proxy_config_list(const LinphoneCore *lc) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getProxyConfigList();
}

LinphoneAccountParams *linphone_core_create_account_params(LinphoneCore *core) {
	return linphone_account_params_new(core, TRUE);
}

LinphoneAccount *linphone_core_create_account(LinphoneCore *core, LinphoneAccountParams *params) {
	return linphone_account_new(core, params);
}

LinphoneStatus linphone_core_add_account(LinphoneCore *lc, LinphoneAccount *account) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(lc)->addAccount(Account::toCpp(account)->getSharedFromThis());
}

void linphone_core_clear_accounts(LinphoneCore *core) {
	L_GET_CPP_PTR_FROM_C_OBJECT(core)->clearAccounts();
}

void linphone_core_remove_deleted_account(LinphoneCore *core, LinphoneAccount *account) {
	L_GET_CPP_PTR_FROM_C_OBJECT(core)->removeDeletedAccount(Account::toCpp(account)->getSharedFromThis());
}

void linphone_core_remove_account(LinphoneCore *core, LinphoneAccount *account) {
	L_GET_CPP_PTR_FROM_C_OBJECT(core)->removeAccount(Account::toCpp(account)->getSharedFromThis());
}

const bctbx_list_t *linphone_core_get_deleted_account_list(const LinphoneCore *lc) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getDeletedAccountsCList();
}

const bctbx_list_t *linphone_core_get_account_list(const LinphoneCore *lc) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getAccountsCList();
}

void linphone_core_set_default_account_index(LinphoneCore *lc, int index) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->setDefaultAccountIndex(index);
}

LinphoneAccount *linphone_core_get_account_by_idkey(LinphoneCore *lc, const char *idkey) {
	auto account = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getAccountByIdKey(L_C_TO_STRING(idkey));
	return (account) ? account->toC() : nullptr;
}

LinphoneAccount *linphone_core_get_default_account(const LinphoneCore *lc) {
	auto defaultAccount = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getDefaultAccount();
	return (defaultAccount) ? defaultAccount->toC() : nullptr;
}

int linphone_core_get_default_account_index(LinphoneCore *lc) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getDefaultAccountIndex();
}

void linphone_core_set_default_account(LinphoneCore *lc, LinphoneAccount *account) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->setDefaultAccount(account ? Account::toCpp(account)->getSharedFromThis()
	                                                           : nullptr);
}

SipSetup *linphone_proxy_config_get_sip_setup(BCTBX_UNUSED(LinphoneProxyConfig *cfg)) {
	return NULL;
} // TODO: remove this

void linphone_proxy_config_update(LinphoneProxyConfig *cfg) {
	Account::toCpp(cfg->account)->update();
}

void linphone_account_update(LinphoneAccount *account) {
	Account::toCpp(account)->update();
}

void linphone_proxy_config_set_sip_setup(BCTBX_UNUSED(LinphoneProxyConfig *cfg), BCTBX_UNUSED(const char *type)) {

} // TODO: remove this

SipSetupContext *linphone_proxy_config_get_sip_setup_context(BCTBX_UNUSED(LinphoneProxyConfig *cfg)) {
	return NULL;
} // TODO: remove this

void linphone_proxy_config_set_user_data(LinphoneProxyConfig *cfg, void *ud) {
	linphone_account_set_user_data(cfg->account, ud);
}

void *linphone_proxy_config_get_user_data(const LinphoneProxyConfig *cfg) {
	return linphone_account_get_user_data(cfg->account);
}

void linphone_proxy_config_set_state(LinphoneProxyConfig *cfg, LinphoneRegistrationState state, const char *message) {
	Account::toCpp(cfg->account)->setState(state, message);
}

LinphoneRegistrationState linphone_proxy_config_get_state(const LinphoneProxyConfig *cfg) {
	return linphone_account_get_state(cfg->account);
}

LinphoneReason linphone_proxy_config_get_error(const LinphoneProxyConfig *cfg) {
	return linphone_account_get_error(cfg->account);
}

const LinphoneErrorInfo *linphone_proxy_config_get_error_info(const LinphoneProxyConfig *cfg) {
	return linphone_account_get_error_info(cfg->account);
}

const LinphoneAddress *linphone_proxy_config_get_service_route(const LinphoneProxyConfig *cfg) {
	const auto route = Account::toCpp(cfg->account)->getServiceRouteAddress();
	return route ? route->toC() : nullptr;
}

const char *linphone_proxy_config_get_transport(const LinphoneProxyConfig *cfg) {
	return linphone_transport_to_string(linphone_account_get_transport(cfg->account));
}

void linphone_proxy_config_set_privacy(LinphoneProxyConfig *cfg, LinphonePrivacyMask privacy) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_privacy(cfg->edit, privacy);
	linphone_proxy_config_done(cfg);
}

LinphonePrivacyMask linphone_proxy_config_get_privacy(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_privacy(params);
}

void linphone_proxy_config_set_publish_expires(LinphoneProxyConfig *cfg, int expires) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_publish_expires(cfg->edit, expires);
	linphone_proxy_config_done(cfg);
}

int linphone_proxy_config_get_publish_expires(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_publish_expires(params);
}

bool_t linphone_proxy_config_avpf_enabled(LinphoneProxyConfig *cfg) {
	return linphone_account_is_avpf_enabled(cfg->account);
}

LinphoneAVPFMode linphone_proxy_config_get_avpf_mode(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_avpf_mode(params);
}

void linphone_proxy_config_set_avpf_mode(LinphoneProxyConfig *cfg, LinphoneAVPFMode mode) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_avpf_mode(cfg->edit, mode);
	linphone_proxy_config_done(cfg);
}

void linphone_proxy_config_set_avpf_rr_interval(LinphoneProxyConfig *cfg, uint8_t interval) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_avpf_rr_interval(cfg->edit, interval);
	linphone_proxy_config_done(cfg);
}

uint8_t linphone_proxy_config_get_avpf_rr_interval(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_avpf_rr_interval(params);
}

const LinphoneAddress *linphone_proxy_config_get_contact(const LinphoneProxyConfig *cfg) {
	return linphone_account_get_contact_address(cfg->account);
}

LinphoneAddress *linphone_proxy_config_get_transport_contact(LinphoneProxyConfig *cfg) {
	LinphoneAddress *addr;
	int port = 0;
	const char *public_ip;

	if (!linphone_account_get_contact_address(cfg->account) || !Account::toCpp(cfg->account)->getOp()) return NULL;
	public_ip = Account::toCpp(cfg->account)->getOp()->getPublicAddress(&port);
	if (!public_ip) return NULL;

	addr = linphone_address_clone(linphone_account_get_contact_address(cfg->account));
	linphone_address_set_domain(addr, public_ip);
	if (port > 0) linphone_address_set_port(addr, port);
	return addr;
}

const LinphoneAddress *_linphone_proxy_config_get_contact_without_params(const LinphoneProxyConfig *cfg) {
	return Account::toCpp(cfg->account)->getContactAddressWithoutParams()->toC();
}

const struct _LinphoneAuthInfo *linphone_proxy_config_find_auth_info(const LinphoneProxyConfig *cfg) {
	return linphone_account_find_auth_info(cfg->account);
}

const char *linphone_proxy_config_get_ref_key(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_ref_key(params);
}

void linphone_proxy_config_set_ref_key(LinphoneProxyConfig *cfg, const char *refkey) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_ref_key(cfg->edit, refkey);
	linphone_proxy_config_done(cfg);
}

LinphoneNatPolicy *linphone_proxy_config_get_nat_policy(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_nat_policy(params);
}

void linphone_proxy_config_set_nat_policy(LinphoneProxyConfig *cfg, LinphoneNatPolicy *policy) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_nat_policy(cfg->edit, policy);
	linphone_proxy_config_done(cfg);
}

void linphone_proxy_config_set_conference_factory_uri(LinphoneProxyConfig *cfg, const char *uri) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_conference_factory_uri(cfg->edit, uri);
	linphone_proxy_config_done(cfg);
}

const char *linphone_proxy_config_get_conference_factory_uri(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_conference_factory_uri(params);
}

bool_t linphone_proxy_config_is_push_notification_allowed(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_push_notification_allowed(params);
}

void linphone_proxy_config_set_push_notification_allowed(LinphoneProxyConfig *cfg, bool_t is_allowed) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_push_notification_allowed(cfg->edit, is_allowed);
	linphone_proxy_config_done(cfg);
}

bool_t linphone_proxy_config_is_remote_push_notification_allowed(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_remote_push_notification_allowed(params);
}

void linphone_proxy_config_set_remote_push_notification_allowed(LinphoneProxyConfig *cfg, bool_t allow) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_remote_push_notification_allowed(cfg->edit, allow);
	linphone_proxy_config_done(cfg);
}

bool_t linphone_proxy_config_is_push_notification_available(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_is_push_notification_available(params);
}

void linphone_proxy_config_set_push_notification_config(LinphoneProxyConfig *cfg,
                                                        LinphonePushNotificationConfig *push_cfg) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_push_notification_config(cfg->edit, push_cfg);
	linphone_proxy_config_done(cfg);
}

LinphonePushNotificationConfig *linphone_proxy_config_get_push_notification_config(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_push_notification_config(params);
}

int linphone_proxy_config_get_unread_chat_message_count(const LinphoneProxyConfig *cfg) {
	return linphone_account_get_unread_chat_message_count(cfg->account);
}

LinphoneEvent *linphone_proxy_config_create_publish(LinphoneProxyConfig *cfg, const char *event, int expires) {
	return Account::toCpp(cfg->account)->createPublish(event, expires)->toC();
}

SalRegisterOp *linphone_proxy_config_get_op(const LinphoneProxyConfig *cfg) {
	return Account::toCpp(cfg->account)->getOp();
}

const char *linphone_registration_state_to_string(LinphoneRegistrationState cs) {
	switch (cs) {
		case LinphoneRegistrationCleared:
			return "LinphoneRegistrationCleared";
			break;
		case LinphoneRegistrationNone:
			return "LinphoneRegistrationNone";
			break;
		case LinphoneRegistrationProgress:
			return "LinphoneRegistrationProgress";
			break;
		case LinphoneRegistrationRefreshing:
			return "LinphoneRegistrationRefreshing";
			break;
		case LinphoneRegistrationOk:
			return "LinphoneRegistrationOk";
			break;
		case LinphoneRegistrationFailed:
			return "LinphoneRegistrationFailed";
			break;
	}
	return NULL;
}
