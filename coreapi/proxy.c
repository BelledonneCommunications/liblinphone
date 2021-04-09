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

#include <bctoolbox/defs.h>
#include "linphone/core_utils.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"
#include "linphone/sipsetup.h"

#include "mediastreamer2/mediastream.h"

#include "core/core.h"
#include "enum.h"
#include "private.h"

// For migration purpose.
#include "account/account.h"
#include "account/account-params.h"
#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "linphone/api/c-dial-plan.h"

#include "dial-plan/dial-plan.h"

using namespace LinphonePrivate;

LinphoneProxyConfigAddressComparisonResult linphone_proxy_config_address_equal(const LinphoneAddress *a, const LinphoneAddress *b) {
	return (LinphoneProxyConfigAddressComparisonResult) Account::compareLinphoneAddresses(a, b);
}

LinphoneProxyConfigAddressComparisonResult linphone_proxy_config_is_server_config_changed(const LinphoneProxyConfig* cfg) {
	return (LinphoneProxyConfigAddressComparisonResult) Account::toCpp(cfg->account)->isServerConfigChanged();
}

void linphone_proxy_config_write_all_to_config_file(LinphoneCore *lc){
	bctbx_list_t *elem;
	int i;
	if (!linphone_core_ready(lc)) return;

	for(elem=lc->sip_conf.proxies,i=0;elem!=NULL;elem=bctbx_list_next(elem),i++){
		LinphoneProxyConfig *cfg=(LinphoneProxyConfig*)elem->data;
		linphone_proxy_config_write_to_config_file(lc->config,cfg,i);
	}
	/*to ensure removed configs are erased:*/
	linphone_proxy_config_write_to_config_file(lc->config,NULL,i);
	linphone_config_set_int(lc->config,"sip","default_proxy",linphone_core_get_default_proxy_config_index(lc));
}

static void linphone_proxy_config_init(LinphoneCore* lc, LinphoneProxyConfig *cfg) {
	LinphoneAccountParams *params = linphone_account_params_new(lc);
	cfg->account = linphone_account_new_with_config(lc, params, cfg);
	linphone_account_params_unref(params);
	cfg->edit = NULL;
}

static void _linphone_proxy_config_destroy(LinphoneProxyConfig *cfg);

BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneProxyConfig);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneProxyConfig, belle_sip_object_t,
	(belle_sip_object_destroy_t)_linphone_proxy_config_destroy,
	NULL, // clone
	NULL, // marshal
	FALSE
);

LinphoneProxyConfig * linphone_core_create_proxy_config(LinphoneCore *lc) {
	LinphoneProxyConfig *cfg = belle_sip_object_new(LinphoneProxyConfig);
	linphone_proxy_config_init(lc,cfg);
	return cfg;
}

void _linphone_proxy_config_destroy(LinphoneProxyConfig *cfg){
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

LinphoneStatus linphone_proxy_config_set_server_addr(LinphoneProxyConfig *cfg, const char *server_addr){
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	return linphone_account_params_set_server_addr(cfg->edit, server_addr);
}

LinphoneStatus linphone_proxy_config_set_identity_address(LinphoneProxyConfig *cfg, const LinphoneAddress *addr){
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	return linphone_account_params_set_identity_address(cfg->edit, addr);
}

const char *linphone_proxy_config_get_domain(const LinphoneProxyConfig *cfg){
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

		SalAddress *addr = sal_address_new(tmp.c_str());
		if (addr != NULL) {
			sal_address_unref(addr);
			list = bctbx_list_append(list, linphone_address_new(tmp.c_str()));
		}
	}

	ret = linphone_account_params_set_routes_addresses(cfg->edit, list);

	bctbx_list_free_with_data(list, (bctbx_list_free_func) linphone_address_unref);
	return ret;
}

LinphoneStatus linphone_proxy_config_set_routes(LinphoneProxyConfig *cfg, const bctbx_list_t *routes) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	return AccountParams::toCpp(cfg->edit)->setRoutesFromStringList(routes);
}

void linphone_proxy_config_enableregister(LinphoneProxyConfig *cfg, bool_t val){
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_register_enabled(cfg->edit, val);
}

void linphone_proxy_config_set_expires(LinphoneProxyConfig *cfg, int val){
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_expires(cfg->edit, val);
}

void linphone_proxy_config_enable_publish(LinphoneProxyConfig *cfg, bool_t val){
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_publish_enabled(cfg->edit, val);
}

void linphone_proxy_config_pause_register(LinphoneProxyConfig *cfg){
	linphone_account_pause_register(cfg->account);
}

void linphone_proxy_config_edit(LinphoneProxyConfig *cfg){
	if (cfg->edit) {
		ms_message("Proxy config [%p] is already on edit mode", cfg);
		return;
	}

	cfg->edit = linphone_account_params_clone(linphone_account_get_params(cfg->account));
}

void linphone_proxy_config_apply(LinphoneProxyConfig *cfg, LinphoneCore *lc){
	Account::toCpp(cfg->account)->apply(lc);
}

void linphone_proxy_config_stop_refreshing(LinphoneProxyConfig * cfg){
	Account::toCpp(cfg->account)->stopRefreshing();
}

void _linphone_proxy_config_unregister(LinphoneProxyConfig *obj) {
	Account::toCpp(obj->account)->unregister();
}

void linphone_proxy_config_refresh_register(LinphoneProxyConfig *cfg){
	linphone_account_refresh_register(cfg->account);
}

void linphone_proxy_config_set_dial_prefix(LinphoneProxyConfig *cfg, const char *prefix){
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_international_prefix(cfg->edit, prefix);
}

const char *linphone_proxy_config_get_dial_prefix(const LinphoneProxyConfig *cfg){
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_international_prefix(params);
}

void linphone_proxy_config_set_dial_escape_plus(LinphoneProxyConfig *cfg, bool_t val){
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_dial_escape_plus_enabled(cfg->edit, val);
}

bool_t linphone_proxy_config_get_dial_escape_plus(const LinphoneProxyConfig *cfg){
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_dial_escape_plus_enabled(params);
}

void linphone_proxy_config_enable_quality_reporting(LinphoneProxyConfig *cfg, bool_t val){
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_quality_reporting_enabled(cfg->edit, val);
}

bool_t linphone_proxy_config_quality_reporting_enabled(LinphoneProxyConfig *cfg){
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_quality_reporting_enabled(params);
}

void linphone_proxy_config_set_quality_reporting_interval(LinphoneProxyConfig *cfg, int interval) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_quality_reporting_interval(cfg->edit, interval);
}

int linphone_proxy_config_get_quality_reporting_interval(LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_quality_reporting_interval(params);
}

void linphone_proxy_config_set_quality_reporting_collector(LinphoneProxyConfig *cfg, const char *collector){
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_quality_reporting_collector(cfg->edit, collector);
}

const char *linphone_proxy_config_get_quality_reporting_collector(const LinphoneProxyConfig *cfg){
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_quality_reporting_collector(params);
}

bool_t linphone_proxy_config_normalize_number(LinphoneProxyConfig *proxy, const char *username, char *result, size_t result_len){
	char * normalized_phone = linphone_proxy_config_normalize_phone_number(proxy, username);
	const char * output = normalized_phone ? normalized_phone : username;
	memset(result, 0, result_len);
	memcpy(result, output, MIN(strlen(output) + 1, result_len));
	ms_free(normalized_phone);
	return output != username;
} // TODO: not used but blacklisted in wrapper

static char *linphone_account_flatten_phone_number(const char *number) {
	char *unescaped_phone_number = belle_sip_username_unescape_unnecessary_characters(number);
	char *result = reinterpret_cast<char *>(ms_malloc0(strlen(unescaped_phone_number) + 1));
	char *w = result;
	const char *r;

	for (r = unescaped_phone_number; *r != '\0'; ++r) {
		if (*r == '+' || isdigit(*r)) {
			*w++ = *r;
		}
	}
	
	*w++ = '\0';
	belle_sip_free(unescaped_phone_number);
	return result;
}

static char* replace_icp_with_plus(char *phone, const char *icp){
	return (strstr(phone, icp) == phone) ?  ms_strdup_printf("+%s", phone+strlen(icp)) : ms_strdup(phone);
}

char* linphone_proxy_config_normalize_phone_number(LinphoneProxyConfig *proxy, const char *username) {
	//return linphone_account_normalize_phone_number(proxy ? proxy->account : NULL, username);
	LinphoneProxyConfig *tmpproxy = proxy ? proxy : linphone_core_create_proxy_config(NULL);
	char* result = NULL;
	std::shared_ptr<DialPlan> dialplan;
	char * nationnal_significant_number = NULL;
	int ccc = -1;

	if (linphone_account_is_phone_number(NULL, username)){
		char * flatten = linphone_account_flatten_phone_number(username);
		ms_debug("Flattened number is '%s' for '%s'",flatten, username);

		ccc = DialPlan::lookupCccFromE164(flatten);
		if (ccc>-1) { /*e164 like phone number*/
			dialplan = DialPlan::findByCcc(ccc);
			nationnal_significant_number = strstr(flatten, dialplan->getCountryCallingCode().c_str());
			if (nationnal_significant_number) {
				nationnal_significant_number +=strlen(dialplan->getCountryCallingCode().c_str());
			}
		} else if (flatten[0] =='+') {
			ms_message ("Unknown ccc for e164 like number [%s]", flatten);
			goto end;
		} else {
			if (linphone_proxy_config_get_dial_prefix(tmpproxy)) {
				dialplan = DialPlan::findByCcc(linphone_proxy_config_get_dial_prefix(tmpproxy)); //copy dial plan;
			} else {
				dialplan = DialPlan::MostCommon;
			}
			if (linphone_proxy_config_get_dial_prefix(tmpproxy)){
				if (strcmp(linphone_proxy_config_get_dial_prefix(tmpproxy),dialplan->getCountryCallingCode().c_str()) != 0){
					//probably generic dialplan, preserving proxy dial prefix
					dialplan->setCountryCallingCode(linphone_proxy_config_get_dial_prefix(tmpproxy));
				}

				/*it does not make sens to try replace icp with + if we are not sure from the country we are (I.E tmpproxy->dial_prefix==NULL)*/
				if (strstr(flatten, dialplan->getInternationalCallPrefix().c_str()) == flatten) {
					char *e164 = replace_icp_with_plus(flatten, dialplan->getInternationalCallPrefix().c_str());
					result = linphone_proxy_config_normalize_phone_number(tmpproxy, e164);
					ms_free(e164);
					goto end;
				}
			}
			nationnal_significant_number=flatten;
		}
		ms_debug("Using dial plan '%s'",dialplan->getCountry().c_str());

		/*if proxy has a dial prefix, modify phonenumber accordingly*/
		if (dialplan->getCountryCallingCode().c_str()[0]!='\0') {
			/* the number already starts with + or international prefix*/
			/*0. keep at most national number significant digits */
			char* nationnal_significant_number_start = nationnal_significant_number
														+ MAX(0, (int)strlen(nationnal_significant_number)
														- (int)dialplan->getNationalNumberLength());
			ms_debug("Prefix not present. Keeping at most %d digits: %s", dialplan->getNationalNumberLength(), nationnal_significant_number_start);

			/*1. First prepend international calling prefix or +*/
			/*2. Second add prefix*/
			/*3. Finally add user digits */
			result = ms_strdup_printf("%s%s%s"
										, linphone_proxy_config_get_dial_escape_plus(tmpproxy) ? dialplan->getInternationalCallPrefix().c_str() : "+"
										, dialplan->getCountryCallingCode().c_str()
										, nationnal_significant_number_start);
			ms_debug("Prepended prefix resulted in %s", result);
		}

	end:
		if (result==NULL) {
			result = flatten;
		} else {
			ms_free(flatten);
		}
	}
	if (proxy==NULL) linphone_proxy_config_unref(tmpproxy);
	return result;
}

static LinphoneAddress* _linphone_core_destroy_addr_if_not_sip( LinphoneAddress* addr ){
	if( linphone_address_is_sip(addr) ) {
		return addr;
	} else {
		linphone_address_unref(addr);
		return NULL;
	}
}

LinphoneAddress* linphone_proxy_config_normalize_sip_uri(LinphoneProxyConfig *proxy, const char *username) {
	//return linphone_account_normalize_sip_uri(proxy ? proxy->account : NULL, username);
	enum_lookup_res_t *enumres=NULL;
	char *enum_domain=NULL;
	char *tmpurl;
	LinphoneAddress *uri;

	if (!username || *username=='\0') return NULL;

	if (is_enum(username,&enum_domain)){
		if (enum_lookup(enum_domain,&enumres)<0){
			ms_free(enum_domain);
			return NULL;
		}
		ms_free(enum_domain);
		tmpurl=enumres->sip_address[0];
		uri=linphone_address_new(tmpurl);
		enum_lookup_res_free(enumres);
		return _linphone_core_destroy_addr_if_not_sip(uri);
	}
	/* check if we have a "sip:" or a "sips:" */
	if ( (strstr(username,"sip:")==NULL) && (strstr(username,"sips:")==NULL) ){
		/* this doesn't look like a true sip uri */
		if (strchr(username,'@')!=NULL){
			/* seems like sip: is missing !*/
			tmpurl=ms_strdup_printf("sip:%s",username);
			uri=linphone_address_new(tmpurl);
			ms_free(tmpurl);
			if (uri){
				return _linphone_core_destroy_addr_if_not_sip(uri);
			}
		}

		if (proxy!=NULL && linphone_proxy_config_get_identity_address(proxy)!=NULL){
			/* append the proxy domain suffix but remove any custom parameters/headers */
			LinphoneAddress *uri=linphone_address_clone(linphone_proxy_config_get_identity_address(proxy));
			if (uri==NULL){
				return NULL;
			} else {
				linphone_address_clean(uri);
				linphone_address_set_display_name(uri,NULL);
				// Unescape character if possible
				char *unescaped_username = belle_sip_username_unescape_unnecessary_characters(username);
				linphone_address_set_username(uri,unescaped_username);
				belle_sip_free(unescaped_username);
				return _linphone_core_destroy_addr_if_not_sip(uri);
			}
		} else {
			return NULL;
		}
	}
	uri=linphone_address_new(username);
	if (uri!=NULL){
		return _linphone_core_destroy_addr_if_not_sip(uri);
	}

	return NULL;
}

void linphone_proxy_config_set_etag(LinphoneProxyConfig *cfg,const char* sip_etag) {
	Account::toCpp(cfg->account)->setSipEtag(sip_etag);
}

/**
 * Commits modification made to the proxy configuration.
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

const char* linphone_proxy_config_get_realm(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_realm(params);
}

void linphone_proxy_config_set_realm(LinphoneProxyConfig *cfg, const char *realm) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_realm(cfg->edit, realm);
}

int linphone_proxy_config_send_publish(LinphoneProxyConfig *proxy, LinphonePresenceModel *presence){
	return Account::toCpp(proxy->account)->sendPublish(presence);
}

void _linphone_proxy_config_unpublish(LinphoneProxyConfig *obj) {
	Account::toCpp(obj->account)->unpublish();
}

const char *linphone_proxy_config_get_route(const LinphoneProxyConfig *cfg) {
	const bctbx_list_t *list = linphone_proxy_config_get_routes(cfg);
	if (list != NULL) {
		return (const char *)bctbx_list_get_data(list);
	}

	return NULL;
}

const bctbx_list_t* linphone_proxy_config_get_routes(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return AccountParams::toCpp(params)->getRoutesString();
}

const LinphoneAddress *linphone_proxy_config_get_identity_address(const LinphoneProxyConfig *cfg){
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_identity_address(params);
}

const char *linphone_proxy_config_get_identity(const LinphoneProxyConfig *cfg){
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_identity(params);
}

bool_t linphone_proxy_config_publish_enabled(const LinphoneProxyConfig *cfg){
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_publish_enabled(params);
}

const char *linphone_proxy_config_get_server_addr(const LinphoneProxyConfig *cfg){
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_server_addr(params);
}

/**
 * @return the duration of registration.
**/
int linphone_proxy_config_get_expires(const LinphoneProxyConfig *cfg){
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_expires(params);
}

bool_t linphone_proxy_config_register_enabled(const LinphoneProxyConfig *cfg){
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_register_enabled(params);
}

void linphone_proxy_config_set_contact_parameters(LinphoneProxyConfig *cfg, const char *contact_params){
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_contact_parameters(cfg->edit, contact_params);
}

void linphone_proxy_config_set_contact_uri_parameters(LinphoneProxyConfig *cfg, const char *contact_uri_params){
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_contact_uri_parameters(cfg->edit, contact_uri_params);
}

const char *linphone_proxy_config_get_contact_parameters(const LinphoneProxyConfig *cfg){
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_contact_parameters(params);
}

const char *linphone_proxy_config_get_contact_uri_parameters(const LinphoneProxyConfig *cfg){
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_contact_uri_parameters(params);
}

struct _LinphoneCore * linphone_proxy_config_get_core(const LinphoneProxyConfig *cfg){
	return linphone_account_get_core(cfg->account);
}

const char *linphone_proxy_config_get_custom_header(LinphoneProxyConfig *cfg, const char *header_name){
	return linphone_account_get_custom_header(cfg->account, header_name);
}

void linphone_proxy_config_set_custom_header(LinphoneProxyConfig *cfg, const char *header_name, const char *header_value){
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
}

LinphoneStatus linphone_core_add_proxy_config(LinphoneCore *lc, LinphoneProxyConfig *cfg) {
	if (cfg->edit) {
		// Use done here to be sure this will use updated parameters
		linphone_proxy_config_done(cfg);
	}

	if (!Account::toCpp(cfg->account)->check()) {
		return -1;
	}
	if (bctbx_list_find(lc->sip_conf.proxies,cfg)!=NULL){
		ms_warning("ProxyConfig already entered, ignored.");
		return 0;
	}
	lc->sip_conf.proxies = bctbx_list_append(lc->sip_conf.proxies,(void *)linphone_proxy_config_ref(cfg));
	lc->sip_conf.accounts = bctbx_list_append(lc->sip_conf.accounts,(void *)linphone_account_ref(cfg->account));
	linphone_proxy_config_apply(cfg,lc);
	return 0;
}

//If an account dependency is removed, restore 'normal' behavior for previously dependent accounts
void linphone_core_remove_dependent_account(LinphoneCore *lc, LinphoneAccount *account) {
	bctbx_list_t *it = lc->sip_conf.accounts;

	for (;it;it = it->next) {
		LinphoneAccount *tmp = reinterpret_cast<LinphoneAccount *>(it->data);
		if (tmp != account && linphone_account_get_dependency(tmp) == account) {
		 	ms_message("Updating dependent account [%p] caused by removal of 'master' account idkey[%s]"
			 	, tmp
				, linphone_account_params_get_idkey(linphone_account_get_params(account)));
			linphone_account_set_dependency(tmp, NULL);
			Account::toCpp(account)->setNeedToRegister(true);
			Account::toCpp(tmp)->update();
		}
	}
}

void linphone_core_remove_proxy_config(LinphoneCore *lc, LinphoneProxyConfig *cfg){
	/* check this proxy config is in the list before doing more*/
	if (bctbx_list_find(lc->sip_conf.proxies,cfg)==NULL) {
		ms_error("linphone_core_remove_proxy_config: LinphoneProxyConfig [%p] is not known by LinphoneCore (programming error?)",cfg);
		return;
	}
	lc->sip_conf.proxies = bctbx_list_remove(lc->sip_conf.proxies,cfg);
	/* add to the list of destroyed proxies, so that the possible unREGISTER request can succeed authentication */
	lc->sip_conf.deleted_proxies=bctbx_list_append(lc->sip_conf.deleted_proxies,cfg);

	if (lc->default_proxy==cfg){
		lc->default_proxy=NULL;
	}

	/* we also need to update the accounts list */
	lc->sip_conf.accounts = bctbx_list_remove(lc->sip_conf.accounts,cfg->account);
	linphone_core_remove_dependent_account(lc, cfg->account);
	/* add to the list of destroyed proxies, so that the possible unREGISTER request can succeed authentication */
	lc->sip_conf.deleted_accounts=bctbx_list_append(lc->sip_conf.deleted_accounts,cfg->account);

	if (lc->default_account == cfg->account) {
		lc->default_account = NULL;
	}

	Account::toCpp(cfg->account)->setDeletionDate(ms_time(NULL));
	if (linphone_proxy_config_get_state(cfg)==LinphoneRegistrationOk){
		/* UNREGISTER */
		linphone_proxy_config_edit(cfg);
		linphone_proxy_config_enable_register(cfg,FALSE);
		linphone_proxy_config_done(cfg);
		linphone_proxy_config_update(cfg);
	} else if (linphone_proxy_config_get_state(cfg) != LinphoneRegistrationNone) {
		linphone_proxy_config_set_state(cfg, LinphoneRegistrationNone,"Registration disabled");
	}
	linphone_proxy_config_write_all_to_config_file(lc);

	//Update the associated linphone specs on the core
	linphone_proxy_config_edit(cfg);
	linphone_proxy_config_set_conference_factory_uri(cfg, NULL);
	linphone_proxy_config_done(cfg);
}

void linphone_core_clear_proxy_config(LinphoneCore *lc) {
	bctbx_list_t* list=bctbx_list_copy(linphone_core_get_proxy_config_list((const LinphoneCore*)lc));
	bctbx_list_t* copy=list;
	for(;list!=NULL;list=list->next){
		linphone_core_remove_proxy_config(lc,(LinphoneProxyConfig *)list->data);
	}
	bctbx_list_free(copy);
	linphone_proxy_config_write_all_to_config_file(lc);
}

int linphone_core_get_default_proxy_config_index(LinphoneCore *lc) {
	int pos = -1;
	if (lc->default_proxy != NULL) {
		pos = bctbx_list_position(lc->sip_conf.proxies, bctbx_list_find(lc->sip_conf.proxies, (void *)lc->default_proxy));
	}
	return pos;
}

LinphoneProxyConfig *linphone_core_get_proxy_config_by_idkey(LinphoneCore *lc, const char *idkey) {
	if (idkey == NULL || lc == NULL) return NULL;
	bctbx_list_t *list = lc->sip_conf.proxies;
	for (;list!=NULL;list=list->next) {
		LinphoneProxyConfig *tmp = reinterpret_cast<LinphoneProxyConfig *>(list->data);
		if (tmp) {
			const char *proxy_idkey = linphone_proxy_config_get_idkey(tmp);
			if (proxy_idkey && strcmp(idkey, proxy_idkey) == 0) return tmp;
		}
	}
	return NULL;
}

void linphone_core_set_default_proxy_config(LinphoneCore *lc, LinphoneProxyConfig *config){
	/* check if this proxy is in our list */
	if (config!=NULL){
		if (bctbx_list_find(lc->sip_conf.proxies,config)==NULL){
			ms_warning("Bad proxy address: it is not in the list !");
			lc->default_proxy=NULL;
			return ;
		}
	}
	lc->default_proxy=config;
	lc->default_account= config ? config->account : NULL;
	if (linphone_core_ready(lc)) {
		linphone_config_set_int(lc->config,"sip","default_proxy",linphone_core_get_default_proxy_config_index(lc));
		/* Invalidate phone numbers in friends maps when default proxy config changes because the new one may have a different dial prefix */
		linphone_core_invalidate_friends_maps(lc);
	}
}

void linphone_core_set_default_proxy_index(LinphoneCore *lc, int index){
	if (index<0) linphone_core_set_default_proxy_config(lc,NULL);
	else linphone_core_set_default_proxy_config(lc,reinterpret_cast<LinphoneProxyConfig *>(bctbx_list_nth_data(lc->sip_conf.proxies,index)));
}

int linphone_core_get_default_proxy(LinphoneCore *lc, LinphoneProxyConfig **config){
	if (config!=NULL) *config=lc->default_proxy;
	return linphone_core_get_default_proxy_config_index(lc);
}

LinphoneProxyConfig * linphone_core_get_default_proxy_config(const LinphoneCore *lc) {
	return lc->default_proxy;
}

const bctbx_list_t *linphone_core_get_proxy_config_list(const LinphoneCore *lc){
	return lc->sip_conf.proxies;
}

LinphoneAccountParams * linphone_core_create_account_params(LinphoneCore *core) {
	return linphone_account_params_new(core);
}

LinphoneAccount * linphone_core_create_account(LinphoneCore *core, LinphoneAccountParams *params) {
	return linphone_account_new(core, params);
}

LinphoneStatus linphone_core_add_account(LinphoneCore *lc, LinphoneAccount *account) {
	if (!Account::toCpp(account)->check()) {
		return -1;
	}
	if (bctbx_list_find(lc->sip_conf.accounts,account)!=NULL){
		ms_warning("Account already entered, ignored.");
		return 0;
	}
	lc->sip_conf.accounts=bctbx_list_append(lc->sip_conf.accounts,(void *)linphone_account_ref(account));

	// If there is no back pointer to a proxy config then create a proxy config that will depend on this account
	// to ensure backward compatibility when using only proxy configs
	LinphoneProxyConfig *cfg = Account::toCpp(account)->getConfig();
	if (cfg == NULL) {
		cfg = belle_sip_object_new(LinphoneProxyConfig);
		cfg->account = linphone_account_ref(account);
		Account::toCpp(account)->setConfig(cfg);
		lc->sip_conf.proxies=bctbx_list_append(lc->sip_conf.proxies,(void *)cfg);
	} else {
		lc->sip_conf.proxies=bctbx_list_append(lc->sip_conf.proxies,(void *)linphone_proxy_config_ref(cfg));
	}

	Account::toCpp(account)->apply(lc);
	return 0;
}

void linphone_core_clear_accounts(LinphoneCore *core) {
	bctbx_list_t* list=bctbx_list_copy(linphone_core_get_account_list((const LinphoneCore*)core));
	bctbx_list_t* copy=list;
	for(;list!=NULL;list=list->next){
		linphone_core_remove_account(core,(LinphoneAccount *)list->data);
	}
	bctbx_list_free(copy);
	linphone_proxy_config_write_all_to_config_file(core);
}

void linphone_core_remove_account(LinphoneCore *core, LinphoneAccount *account) {
	/* check this proxy config is in the list before doing more*/
	if (bctbx_list_find(core->sip_conf.accounts,account)==NULL) {
		ms_error("linphone_core_remove_account: LinphoneAccount [%p] is not known by LinphoneCore (programming error?)",account);
		return;
	}

	/* we also need to update the accounts list */
	core->sip_conf.accounts = bctbx_list_remove(core->sip_conf.accounts,account);
	linphone_core_remove_dependent_account(core, account);
	/* add to the list of destroyed accounts, so that the possible unREGISTER request can succeed authentication */
	core->sip_conf.deleted_accounts=bctbx_list_append(core->sip_conf.deleted_accounts,account);

	if (core->default_account == account) {
		core->default_account = NULL;
	}

	LinphoneProxyConfig *cfg = Account::toCpp(account)->getConfig();
	core->sip_conf.proxies = bctbx_list_remove(core->sip_conf.proxies,cfg);
	/* add to the list of destroyed proxies, so that the possible unREGISTER request can succeed authentication */
	core->sip_conf.deleted_proxies=bctbx_list_append(core->sip_conf.deleted_proxies,cfg);

	if (core->default_proxy==cfg){
		core->default_proxy=NULL;
	}

	Account::toCpp(account)->setDeletionDate(ms_time(NULL));
	if (linphone_account_get_state(account)==LinphoneRegistrationOk){
		LinphoneAccountParams *params = linphone_account_params_clone(linphone_account_get_params(account));
		linphone_account_params_set_register_enabled(params, FALSE);
		linphone_account_set_params(account, params);
		linphone_account_params_unref(params);
		Account::toCpp(account)->update();
	} else if (linphone_account_get_state(account) != LinphoneRegistrationNone) {
		Account::toCpp(account)->setState(LinphoneRegistrationNone,"Registration disabled");
	}
	linphone_proxy_config_write_all_to_config_file(core);

	//Update the associated linphone specs on the core
	LinphoneAccountParams *params = linphone_account_params_clone(linphone_account_get_params(account));
	linphone_account_params_set_conference_factory_uri(params, NULL);
	linphone_account_set_params(account, params);
	linphone_account_params_unref(params);
}

const bctbx_list_t *linphone_core_get_account_list(const LinphoneCore *lc){
	return lc->sip_conf.accounts;
}

void linphone_core_set_default_account_index(LinphoneCore *lc, int index){
	if (index<0) linphone_core_set_default_account(lc,NULL);
	else linphone_core_set_default_account(lc,reinterpret_cast<LinphoneAccount *>(bctbx_list_nth_data(lc->sip_conf.accounts,index)));
}

LinphoneAccount *linphone_core_get_account_by_idkey(LinphoneCore *lc, const char *idkey) {
	if (idkey == NULL || lc == NULL) return NULL;
	bctbx_list_t *list = lc->sip_conf.accounts;
	for (;list!=NULL;list=list->next) {
		LinphoneAccount *tmp = reinterpret_cast<LinphoneAccount *>(list->data);
		if (tmp) {
			const char *account_idkey = linphone_account_params_get_idkey(linphone_account_get_params(tmp));
			if (account_idkey && strcmp(idkey, account_idkey) == 0) return tmp;
		}
	}
	return NULL;
}

LinphoneAccount * linphone_core_get_default_account(const LinphoneCore *lc) {
	return lc->default_account;
}

int linphone_core_get_default_account_index(LinphoneCore *lc) {
	int pos = -1;
	if (lc->default_account != NULL) {
		pos = bctbx_list_position(lc->sip_conf.accounts, bctbx_list_find(lc->sip_conf.accounts, (void *)lc->default_account));
	}
	return pos;
}

void linphone_core_set_default_account(LinphoneCore *lc, LinphoneAccount *account){
	/* check if this account is in our list */
	if (account!=NULL){
		if (bctbx_list_find(lc->sip_conf.accounts,account)==NULL){
			ms_warning("Bad account address: it is not in the list !");
			lc->default_account=NULL;
			return ;
		}
	}
	lc->default_account=account;
	lc->default_proxy=Account::toCpp(account)->getConfig();
	if (linphone_core_ready(lc)) {
		linphone_config_set_int(lc->config,"sip","default_proxy",linphone_core_get_default_account_index(lc));
		/* Invalidate phone numbers in friends maps when default account changes because the new one may have a different dial prefix */
		linphone_core_invalidate_friends_maps(lc);
	}
}

void linphone_proxy_config_write_to_config_file(LpConfig *config, LinphoneProxyConfig *cfg, int index)
{
	char key[50];

	sprintf(key, "proxy_%i", index);
	linphone_config_clean_section(config, key);
	if (cfg == NULL){
		return;
	}

	Account::toCpp(cfg->account)->writeToConfigFile(index);
}

LinphoneProxyConfig *linphone_proxy_config_new_from_config_file(LinphoneCore* lc, int index)
{
	LinphoneAccountParams *params = linphone_account_params_new_with_config(lc, index);

	// The config do not have the section refered by index if params is NULL
	if (params == NULL) {
		return NULL;
	}

	LinphoneProxyConfig *cfg = belle_sip_object_new(LinphoneProxyConfig);
	cfg->account = linphone_account_new_with_config(lc, params, cfg);
	linphone_account_params_unref(params);
	cfg->edit = NULL;

	return cfg;
}

SipSetup *linphone_proxy_config_get_sip_setup(LinphoneProxyConfig *cfg){
	return NULL;
} // TODO: remove this

void linphone_proxy_config_update(LinphoneProxyConfig *cfg){
	Account::toCpp(cfg->account)->update();
}

void linphone_account_update(LinphoneAccount *account){
	Account::toCpp(account)->update();
}

void linphone_proxy_config_set_sip_setup(LinphoneProxyConfig *cfg, const char *type){

} // TODO: remove this

SipSetupContext *linphone_proxy_config_get_sip_setup_context(LinphoneProxyConfig *cfg){
	return NULL;
} // TODO: remove this

void linphone_proxy_config_set_user_data(LinphoneProxyConfig *cfg, void *ud) {
	linphone_account_set_user_data(cfg->account, ud);
}

void *linphone_proxy_config_get_user_data(const LinphoneProxyConfig *cfg) {
	return linphone_account_get_user_data(cfg->account);
}

void linphone_proxy_config_set_state(LinphoneProxyConfig *cfg, LinphoneRegistrationState state, const char *message){
	Account::toCpp(cfg->account)->setState(state, message);
}

LinphoneRegistrationState linphone_proxy_config_get_state(const LinphoneProxyConfig *cfg){
	return linphone_account_get_state(cfg->account);
}

LinphoneReason linphone_proxy_config_get_error(const LinphoneProxyConfig *cfg) {
	return linphone_account_get_error(cfg->account);
}

const LinphoneErrorInfo *linphone_proxy_config_get_error_info(const LinphoneProxyConfig *cfg){
	return linphone_account_get_error_info(cfg->account);
}

const LinphoneAddress* linphone_proxy_config_get_service_route(const LinphoneProxyConfig* cfg) {
	return Account::toCpp(cfg->account)->getServiceRouteAddress();
}

const char* linphone_proxy_config_get_transport(const LinphoneProxyConfig *cfg) {
	return linphone_transport_to_string(linphone_account_get_transport(cfg->account));
}

void linphone_proxy_config_set_privacy(LinphoneProxyConfig *params, LinphonePrivacyMask privacy) {
	if (!params->edit) {
		linphone_proxy_config_edit(params);
	}

	return linphone_account_params_set_privacy(params->edit, privacy);
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
}

int linphone_proxy_config_get_publish_expires(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_publish_expires(params);
}

bool_t linphone_proxy_config_avpf_enabled(LinphoneProxyConfig *cfg) {
	return linphone_account_is_avpf_enabled(cfg->account);
}

LinphoneAVPFMode linphone_proxy_config_get_avpf_mode(const LinphoneProxyConfig *cfg){
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_avpf_mode(params);
}

void linphone_proxy_config_set_avpf_mode(LinphoneProxyConfig *cfg, LinphoneAVPFMode mode){
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_avpf_mode(cfg->edit, mode);
}

void linphone_proxy_config_set_avpf_rr_interval(LinphoneProxyConfig *cfg, uint8_t interval) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_avpf_rr_interval(cfg->edit, interval);
}

uint8_t linphone_proxy_config_get_avpf_rr_interval(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_avpf_rr_interval(params);
}

const LinphoneAddress *linphone_proxy_config_get_contact (const LinphoneProxyConfig *cfg) {
	return linphone_account_get_contact_address(cfg->account);
}

LinphoneAddress * linphone_proxy_config_get_transport_contact(LinphoneProxyConfig *cfg){
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


const LinphoneAddress *_linphone_proxy_config_get_contact_without_params (const LinphoneProxyConfig *cfg) {
	return Account::toCpp(cfg->account)->getContactAddressWithoutParams();
}

const struct _LinphoneAuthInfo* linphone_proxy_config_find_auth_info(const LinphoneProxyConfig *cfg) {
	return linphone_account_find_auth_info(cfg->account);
}

const char * linphone_proxy_config_get_ref_key(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_ref_key(params);
}

void linphone_proxy_config_set_ref_key(LinphoneProxyConfig *cfg, const char *refkey) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_ref_key(cfg->edit, refkey);
}

LinphoneNatPolicy * linphone_proxy_config_get_nat_policy(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_nat_policy(params);
}

void linphone_proxy_config_set_nat_policy(LinphoneProxyConfig *cfg, LinphoneNatPolicy *policy) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_nat_policy(cfg->edit, policy);
}

void linphone_proxy_config_set_conference_factory_uri(LinphoneProxyConfig *cfg, const char *uri) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_conference_factory_uri(cfg->edit, uri);
}

const char * linphone_proxy_config_get_conference_factory_uri(const LinphoneProxyConfig *cfg) {
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
}

bool_t linphone_proxy_config_is_push_notification_available(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_is_push_notification_available(params);
}

void linphone_proxy_config_set_push_notification_config(LinphoneProxyConfig *cfg, LinphonePushNotificationConfig *push_cfg) {
	if (!cfg->edit) {
		linphone_proxy_config_edit(cfg);
	}

	linphone_account_params_set_push_notification_config(cfg->edit, push_cfg);
}

LinphonePushNotificationConfig *linphone_proxy_config_get_push_notification_config(const LinphoneProxyConfig *cfg) {
	const LinphoneAccountParams *params = cfg->edit ? cfg->edit : linphone_account_get_params(cfg->account);
	return linphone_account_params_get_push_notification_config(params);
}

int linphone_proxy_config_get_unread_chat_message_count (const LinphoneProxyConfig *cfg) {
	return linphone_account_get_unread_chat_message_count(cfg->account);
}

LinphoneEvent *linphone_proxy_config_create_publish(LinphoneProxyConfig *cfg, const char *event, int expires) {
	return Account::toCpp(cfg->account)->createPublish(event, expires);
}

SalRegisterOp *linphone_proxy_config_get_op(const LinphoneProxyConfig *cfg) {
	return Account::toCpp(cfg->account)->getOp();
}

const char *linphone_registration_state_to_string(LinphoneRegistrationState cs){
	 switch(cs){
		case LinphoneRegistrationCleared:
			 return "LinphoneRegistrationCleared";
		break;
		case LinphoneRegistrationNone:
			 return "LinphoneRegistrationNone";
		break;
		case LinphoneRegistrationProgress:
			return "LinphoneRegistrationProgress";
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
