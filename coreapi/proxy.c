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
#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "linphone/api/c-dial-plan.h"

#include "dial-plan/dial-plan.h"

using namespace LinphonePrivate;

static char *generate_proxy_config_id() {
	char id[17] = {0};
	belle_sip_random_token(id, 16);
	return bctbx_concat("proxy_config_", id, NULL);
}

/*store current config related to server location*/
static void linphone_proxy_config_store_server_config(LinphoneProxyConfig* proxy_config) {
	if (proxy_config->saved_identity) linphone_address_unref(proxy_config->saved_identity);
	if (proxy_config->identity_address)
		proxy_config->saved_identity = linphone_address_clone(proxy_config->identity_address);
	else
		proxy_config->saved_identity = NULL;

	if (proxy_config->saved_proxy) linphone_address_unref(proxy_config->saved_proxy);
	if (proxy_config->reg_proxy)
		proxy_config->saved_proxy = linphone_address_new(proxy_config->reg_proxy);
	else
		proxy_config->saved_proxy = NULL;
}

LinphoneProxyConfigAddressComparisonResult linphone_proxy_config_address_equal(const LinphoneAddress *a, const LinphoneAddress *b) {
	if (a == NULL && b == NULL)
		return LinphoneProxyConfigAddressEqual;
	else if (!a || !b)
		return LinphoneProxyConfigAddressDifferent;

	if (linphone_address_equal(a,b))
		return LinphoneProxyConfigAddressEqual;
	if (linphone_address_weak_equal(a,b)) {
		/*also check both transport and uri */
		if (linphone_address_get_secure(a) == linphone_address_get_secure(b) && linphone_address_get_transport(a) == linphone_address_get_transport(b))
			return LinphoneProxyConfigAddressWeakEqual;
		else
			return LinphoneProxyConfigAddressDifferent;
	}
	return LinphoneProxyConfigAddressDifferent; /*either username, domain or port ar not equals*/
}

LinphoneProxyConfigAddressComparisonResult linphone_proxy_config_is_server_config_changed(const LinphoneProxyConfig* proxy_config) {
	LinphoneAddress *current_proxy=proxy_config->reg_proxy?linphone_address_new(proxy_config->reg_proxy):NULL;
	LinphoneProxyConfigAddressComparisonResult result_identity;
	LinphoneProxyConfigAddressComparisonResult result;

	result = linphone_proxy_config_address_equal(proxy_config->saved_identity,proxy_config->identity_address);
	if (result == LinphoneProxyConfigAddressDifferent) goto end;
	result_identity = result;

	result = linphone_proxy_config_address_equal(proxy_config->saved_proxy,current_proxy);
	if (result == LinphoneProxyConfigAddressDifferent) goto end;
	/** If the proxies are equal use the result of the difference between the identities,
	  * otherwise the result is weak-equal and so weak-equal must be returned even if the
	  * identities were equal.
	  */
	if (result == LinphoneProxyConfigAddressEqual) result = result_identity;

	end:
	if (current_proxy) linphone_address_unref(current_proxy);
	ms_message("linphone_proxy_config_is_server_config_changed : %i", result);
	return result;
}

void linphone_proxy_config_write_all_to_config_file(LinphoneCore *lc){
	bctbx_list_t *elem;
	int i;
	if (!linphone_core_ready(lc)) return;

	for(elem=lc->sip_conf.proxies,i=0;elem!=NULL;elem=bctbx_list_next(elem),i++){
		LinphoneProxyConfig *proxy_config=(LinphoneProxyConfig*)elem->data;
		linphone_proxy_config_write_to_config_file(lc->config,proxy_config,i);
	}
	/*to ensure removed configs are erased:*/
	linphone_proxy_config_write_to_config_file(lc->config,NULL,i);
	linphone_config_set_int(lc->config,"sip","default_proxy",linphone_core_get_default_proxy_config_index(lc));
}

static void linphone_proxy_config_init(LinphoneCore* lc, LinphoneProxyConfig *proxy_config) {
	const char *dial_prefix = lc ? linphone_config_get_default_string(lc->config,"proxy","dial_prefix",NULL) : NULL;
  	const char *identity = lc ? linphone_config_get_default_string(lc->config, "proxy", "reg_identity", NULL) : NULL;
	const char *proxy = lc ? linphone_config_get_default_string(lc->config, "proxy", "reg_proxy", NULL) : NULL;
	const char *route = lc ? linphone_config_get_default_string(lc->config, "proxy", "reg_route", NULL) : NULL; //TODO return list instead of string
	const char *realm = lc ? linphone_config_get_default_string(lc->config, "proxy", "realm", NULL) : NULL;
	const char *quality_reporting_collector = lc ? linphone_config_get_default_string(lc->config, "proxy", "quality_reporting_collector", NULL) : NULL;
	const char *contact_params = lc ? linphone_config_get_default_string(lc->config, "proxy", "contact_parameters", NULL) : NULL;
	const char *contact_uri_params = lc ? linphone_config_get_default_string(lc->config, "proxy", "contact_uri_parameters", NULL) : NULL;
	const char *refkey = lc ? linphone_config_get_default_string(lc->config, "proxy", "refkey", NULL) : NULL;
	const char *idkey = lc ? linphone_config_get_default_string(lc->config, "proxy", "idkey", NULL) : NULL;
	const char *depends_on = lc ? linphone_config_get_default_string(lc->config, "proxy", "depends_on", NULL) : NULL;
	const char *nat_policy_ref = lc ? linphone_config_get_default_string(lc->config, "proxy", "nat_policy_ref", NULL):NULL;
	const char *conference_factory_uri = lc ? linphone_config_get_default_string(lc->config, "proxy", "conference_factory_uri", NULL):NULL;
	proxy_config->lc = lc;
	proxy_config->expires = lc ? linphone_config_get_default_int(lc->config, "proxy", "reg_expires", 3600) : 3600;
	proxy_config->reg_sendregister = lc ? !!linphone_config_get_default_int(lc->config, "proxy", "reg_sendregister", 1) : 1;
	proxy_config->dial_prefix = dial_prefix ? ms_strdup(dial_prefix) : NULL;
	proxy_config->dial_escape_plus = lc ? !!linphone_config_get_default_int(lc->config, "proxy", "dial_escape_plus", 0) : 0;
	proxy_config->privacy = lc ? (LinphonePrivacyMask)linphone_config_get_default_int(lc->config, "proxy", "privacy", LinphonePrivacyDefault) : (LinphonePrivacyMask)LinphonePrivacyDefault;
	proxy_config->identity_address = identity ? linphone_address_new(identity) : NULL;
	proxy_config->reg_identity = proxy_config->identity_address ? linphone_address_to_string(proxy_config->identity_address) : NULL;
	proxy_config->reg_proxy = proxy ? ms_strdup(proxy) : NULL;
	proxy_config->reg_routes = route ? bctbx_list_append(proxy_config->reg_routes, ms_strdup(route)) : NULL; //TODO get list directly
	proxy_config->realm = realm ? ms_strdup(realm) : NULL;
	proxy_config->quality_reporting_enabled = lc ? !!linphone_config_get_default_int(lc->config, "proxy", "quality_reporting_enabled", 0) : 0;
	proxy_config->quality_reporting_collector = quality_reporting_collector ? ms_strdup(quality_reporting_collector) : NULL;
	proxy_config->quality_reporting_interval = lc ? linphone_config_get_default_int(lc->config, "proxy", "quality_reporting_interval", 0) : 0;
	proxy_config->contact_params = contact_params ? ms_strdup(contact_params) : NULL;
	proxy_config->contact_uri_params = contact_uri_params ? ms_strdup(contact_uri_params) : NULL;
	proxy_config->avpf_mode = lc ? static_cast<LinphoneAVPFMode>(linphone_config_get_default_int(lc->config, "proxy", "avpf", LinphoneAVPFDefault)) : LinphoneAVPFDefault;
	proxy_config->avpf_rr_interval = lc ? !!linphone_config_get_default_int(lc->config, "proxy", "avpf_rr_interval", 5) : 5;
	proxy_config->publish_expires= lc ? linphone_config_get_default_int(lc->config, "proxy", "publish_expires", -1) : -1;
	proxy_config->publish = lc ? !!linphone_config_get_default_int(lc->config, "proxy", "publish", FALSE) : FALSE;

	bool_t push_allowed_default = FALSE;
#if __ANDROID__ || TARGET_OS_IPHONE
	push_allowed_default = TRUE;
#endif
	proxy_config->push_notification_allowed = lc ? !!linphone_config_get_default_int(lc->config, "proxy", "push_notification_allowed", push_allowed_default) : push_allowed_default;
	proxy_config->refkey = refkey ? ms_strdup(refkey) : NULL;
	if (nat_policy_ref) {
		LinphoneNatPolicy *policy = linphone_config_create_nat_policy_from_section(lc->config,nat_policy_ref);
		linphone_proxy_config_set_nat_policy(proxy_config, policy);
		if (policy) {
			linphone_nat_policy_unref(policy);
		} else {
			ms_error("Cannot create default nat policy with ref [%s] for proxy config [%p]",nat_policy_ref,proxy_config);
		}
	}
	proxy_config->depends_on = depends_on ? ms_strdup(depends_on) : NULL;
	if (idkey) {
		proxy_config->idkey = ms_strdup(idkey);
	} else {
		proxy_config->idkey = generate_proxy_config_id();
	}

	linphone_proxy_config_set_conference_factory_uri(proxy_config, conference_factory_uri ? ms_strdup(conference_factory_uri) : NULL);
}

static char * append_linphone_address(LinphoneAddress *addr,char *out) {
	char *res = out;
	if (addr) {
		char *tmp;
		tmp = linphone_address_to_string(addr);
		res = ms_strcat_printf(out, "%s",tmp);
		ms_free(tmp);
	}
	return res;
};
static char * append_string(const char * string,char *out) {
	char *res = out;
	if (string) {
		res = ms_strcat_printf(out, "%s",string);
	}
	return res;
}
/*
 * return true if computed value has changed
 */
bool_t linphone_proxy_config_compute_publish_params_hash(LinphoneProxyConfig * proxy_config) {
	char * source = NULL;
	char hash[33];
	char saved;
	unsigned long long previous_hash[2];
	bctbx_list_t *routes_iterator = proxy_config->reg_routes;
	previous_hash[0] = proxy_config->previous_publish_config_hash[0];
	previous_hash[1] = proxy_config->previous_publish_config_hash[1];

	source = ms_strcat_printf(source, "%i",proxy_config->privacy);
	source=append_linphone_address(proxy_config->identity_address, source);
	source=append_string(proxy_config->reg_proxy,source);
	while (routes_iterator) {
		const char *route = (const char *)bctbx_list_get_data(routes_iterator);
		source=append_string(route,source);
		routes_iterator = bctbx_list_next(routes_iterator);
	}
	source=append_string(proxy_config->realm,source);
	source = ms_strcat_printf(source, "%i",proxy_config->publish_expires);
	source = ms_strcat_printf(source, "%i",proxy_config->publish);
	belle_sip_auth_helper_compute_ha1(source, "dummy", "dummy", hash);
	ms_free(source);
	saved = hash[16];
	hash[16] = '\0';
	proxy_config->previous_publish_config_hash[0] = strtoull(hash, (char **)NULL, 16);
	hash[16] = saved;
	proxy_config->previous_publish_config_hash[1] = strtoull(&hash[16], (char **)NULL, 16);
	return previous_hash[0] != proxy_config->previous_publish_config_hash[0] || previous_hash[1] != proxy_config->previous_publish_config_hash[1];
}
static void _linphone_proxy_config_destroy(LinphoneProxyConfig *proxy_config);

BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneProxyConfig);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneProxyConfig, belle_sip_object_t,
	(belle_sip_object_destroy_t)_linphone_proxy_config_destroy,
	NULL, // clone
	NULL, // marshal
	FALSE
);

LinphoneProxyConfig * linphone_core_create_proxy_config(LinphoneCore *lc) {
	LinphoneProxyConfig *proxy_config = belle_sip_object_new(LinphoneProxyConfig);
	linphone_proxy_config_init(lc,proxy_config);
	return proxy_config;
}

void _linphone_proxy_config_release_ops(LinphoneProxyConfig *proxy_config){
	if (proxy_config->op) {
		proxy_config->op->release();
		proxy_config->op=NULL;
	}
	if (proxy_config->presence_publish_event){
		linphone_event_terminate(proxy_config->presence_publish_event);
		linphone_event_unref(proxy_config->presence_publish_event);
		proxy_config->presence_publish_event=NULL;
	}
	if (proxy_config->dependency){
		linphone_proxy_config_unref(proxy_config->dependency);
		proxy_config->dependency = NULL;
	}
}

void _linphone_proxy_config_destroy(LinphoneProxyConfig *proxy_config){
	if (proxy_config->reg_proxy != NULL) ms_free(proxy_config->reg_proxy);
	if (proxy_config->reg_identity != NULL) ms_free(proxy_config->reg_identity);
	if (proxy_config->identity_address != NULL) linphone_address_unref(proxy_config->identity_address);
	if (proxy_config->reg_routes != NULL) bctbx_list_free_with_data(proxy_config->reg_routes, ms_free);
	if (proxy_config->quality_reporting_collector != NULL) ms_free(proxy_config->quality_reporting_collector);
	if (proxy_config->ssctx != NULL) sip_setup_context_free(proxy_config->ssctx);
	if (proxy_config->realm != NULL) ms_free(proxy_config->realm);
	if (proxy_config->type != NULL) ms_free(proxy_config->type);
	if (proxy_config->dial_prefix != NULL) ms_free(proxy_config->dial_prefix);
	if (proxy_config->contact_params) ms_free(proxy_config->contact_params);
	if (proxy_config->contact_uri_params) ms_free(proxy_config->contact_uri_params);
	if (proxy_config->saved_proxy != NULL) linphone_address_unref(proxy_config->saved_proxy);
	if (proxy_config->saved_identity != NULL) linphone_address_unref(proxy_config->saved_identity);
	if (proxy_config->sent_headers != NULL) sal_custom_header_free(proxy_config->sent_headers);
	if (proxy_config->pending_contact) linphone_address_unref(proxy_config->pending_contact);
	if (proxy_config->refkey) ms_free(proxy_config->refkey);
	if (proxy_config->idkey) ms_free(proxy_config->idkey);
	linphone_proxy_config_set_dependency(proxy_config, NULL);
	if (proxy_config->nat_policy != NULL) {
		linphone_nat_policy_unref(proxy_config->nat_policy);
	}
	if (proxy_config->conference_factory_uri)
		bctbx_free(proxy_config->conference_factory_uri);
	if (proxy_config->ei){
		linphone_error_info_unref(proxy_config->ei);
	}

	if (proxy_config->service_route) linphone_address_unref(proxy_config->service_route);
	if (proxy_config->contact_address) linphone_address_unref(proxy_config->contact_address);
	if (proxy_config->contact_address_without_params)
		linphone_address_unref(proxy_config->contact_address_without_params);

	_linphone_proxy_config_release_ops(proxy_config);
}

void _linphone_proxy_config_release(LinphoneProxyConfig *proxy_config) {
	_linphone_proxy_config_release_ops(proxy_config);
	belle_sip_object_unref(proxy_config);
}

LinphoneProxyConfig *linphone_proxy_config_ref(LinphoneProxyConfig *proxy_config) {
	belle_sip_object_ref(proxy_config);
	return proxy_config;
}

void linphone_proxy_config_unref(LinphoneProxyConfig *proxy_config) {
	belle_sip_object_unref(proxy_config);
}

LinphoneStatus linphone_proxy_config_set_server_addr(LinphoneProxyConfig *proxy_config, const char *server_addr){
	LinphoneAddress *addr=NULL;
	char *modified=NULL;

	if (proxy_config->reg_proxy!=NULL) ms_free(proxy_config->reg_proxy);
	proxy_config->reg_proxy=NULL;

	if (server_addr!=NULL && strlen(server_addr)>0){
		if (strstr(server_addr,"sip:")==NULL && strstr(server_addr,"sips:")==NULL){
			modified=ms_strdup_printf("sip:%s",server_addr);
			addr=linphone_address_new(modified);
			ms_free(modified);
		}
		if (addr==NULL)
			addr=linphone_address_new(server_addr);
		if (addr){
			proxy_config->reg_proxy=linphone_address_to_string(addr);
			linphone_address_unref(addr);
		}else{
			ms_warning("Could not parse %s",server_addr);
			return -1;
		}
	}
	return 0;
}

LinphoneStatus linphone_proxy_config_set_identity_address(LinphoneProxyConfig *proxy_config, const LinphoneAddress *addr){
	if (!addr || linphone_address_get_username(addr)==NULL){
		char* as_string = addr ? linphone_address_to_string(addr) : ms_strdup("NULL");
		ms_warning("Invalid sip identity: %s", as_string);
		ms_free(as_string);
		return -1;
	}
	if (proxy_config->identity_address != NULL) {
		linphone_address_unref(proxy_config->identity_address);
	}
	proxy_config->identity_address=linphone_address_clone(addr);

	if (proxy_config->reg_identity!=NULL) {
		ms_free(proxy_config->reg_identity);
	}
	proxy_config->reg_identity= linphone_address_to_string(proxy_config->identity_address);
	return 0;
}

const char *linphone_proxy_config_get_domain(const LinphoneProxyConfig *proxy_config){
	return proxy_config->identity_address ? linphone_address_get_domain(proxy_config->identity_address) : NULL;
}

LinphoneStatus linphone_proxy_config_set_route(LinphoneProxyConfig *proxy_config, const char *route)
{
	if (proxy_config->reg_routes != NULL) {
		bctbx_list_free_with_data(proxy_config->reg_routes, ms_free);
		proxy_config->reg_routes = NULL;
	}
	if (route!=NULL && route[0] !='\0'){
		SalAddress *addr;
		char *tmp;
		/*try to prepend 'sip:' */
		if (strstr(route,"sip:")==NULL && strstr(route,"sips:")==NULL){
			tmp=ms_strdup_printf("sip:%s",route);
		}else tmp=ms_strdup(route);
		addr=sal_address_new(tmp);
		if (addr!=NULL){
			sal_address_unref(addr);
			proxy_config->reg_routes = bctbx_list_append(proxy_config->reg_routes, tmp);
			return 0;
		}else{
			ms_free(tmp);
			return -1;
		}
	} else {
		return 0;
	}
}

LinphoneStatus linphone_proxy_config_set_routes(LinphoneProxyConfig *proxy_config, const bctbx_list_t *routes) {
	if (proxy_config->reg_routes != NULL) {
		bctbx_list_free_with_data(proxy_config->reg_routes, ms_free);
		proxy_config->reg_routes = NULL;
	}
	bctbx_list_t *iterator = (bctbx_list_t *)routes;
	while (iterator != NULL) {
		char *route = (char *)bctbx_list_get_data(iterator);
		if (route != NULL && route[0] !='\0') {
			SalAddress *addr;
			char *tmp;
			/*try to prepend 'sip:' */
			if (strstr(route,"sip:") == NULL && strstr(route,"sips:") == NULL) {
				tmp = ms_strdup_printf("sip:%s",route);
			} else {
				tmp = ms_strdup(route);
			}
			addr = sal_address_new(tmp);
			if (addr != NULL) {
				sal_address_unref(addr);
				proxy_config->reg_routes = bctbx_list_append(proxy_config->reg_routes, tmp);
			} else {
				ms_free(tmp);
				return -1;
			}
		}
		iterator = bctbx_list_next(iterator);
	}
	return 0;
}

static void _linphone_core_resolve_proxy_config_dependencies(LinphoneCore *lc){
	LinphoneProxyConfig *proxy_config;
	const bctbx_list_t *elem;
	for(elem = lc->sip_conf.proxies; elem != NULL; elem = elem->next){
		proxy_config = (LinphoneProxyConfig*)elem->data;
		
		if (proxy_config->dependency != NULL && proxy_config->depends_on != NULL) {
			LinphoneProxyConfig *master_proxy_config = linphone_core_get_proxy_config_by_idkey(lc, proxy_config->depends_on);
			if (master_proxy_config != NULL && master_proxy_config != proxy_config->dependency) {
				ms_error("LinphoneProxyConfig has a dependency but idkeys do not match: [%s] != [%s], breaking dependency now.", proxy_config->depends_on, proxy_config->dependency->idkey);
				linphone_proxy_config_unref(proxy_config->dependency);
				proxy_config->dependency = NULL;
				return;
			}else if (master_proxy_config == NULL){
				ms_warning("LinphoneProxyConfig [%p] depends on proxy config [%p], which is not currently in the list.", proxy_config, proxy_config->dependency);
			}
		}
		if (proxy_config->depends_on != NULL && proxy_config->dependency == NULL) {
			LinphoneProxyConfig *dependency = linphone_core_get_proxy_config_by_idkey(lc, proxy_config->depends_on);

			if (dependency == NULL) {
				ms_warning("LinphoneProxyConfig marked as dependent but no proxy configuration found for idkey [%s]", proxy_config->depends_on);
				return;
			} else {
				ms_message("LinphoneProxyConfig [%p] now depends on master LinphoneProxyConfig [%p]", proxy_config, dependency);
				proxy_config->dependency = linphone_proxy_config_ref(dependency);
			}
		}
	}
}

bool_t linphone_proxy_config_check(LinphoneCore *lc, LinphoneProxyConfig *proxy_config){
	if (proxy_config->reg_proxy==NULL)
		return FALSE;
	if (proxy_config->identity_address==NULL)
		return FALSE;
	_linphone_core_resolve_proxy_config_dependencies(lc);
	return TRUE;
}

void linphone_proxy_config_enableregister(LinphoneProxyConfig *proxy_config, bool_t val){
	if (val != proxy_config->reg_sendregister) proxy_config->register_changed = TRUE;
	proxy_config->reg_sendregister = val;
}

void linphone_proxy_config_set_expires(LinphoneProxyConfig *proxy_config, int val){
	if (val<0) val=600;
	if (val != proxy_config->expires) proxy_config->register_changed = TRUE;
	proxy_config->expires=val;
}

void linphone_proxy_config_enable_publish(LinphoneProxyConfig *proxy_config, bool_t val){
	proxy_config->publish=val;
}

void linphone_proxy_config_pause_register(LinphoneProxyConfig *proxy_config){
	if (proxy_config->op) proxy_config->op->stopRefreshing();
}

void linphone_proxy_config_edit(LinphoneProxyConfig *proxy_config){
	/*store current config related to server location*/
	linphone_proxy_config_store_server_config(proxy_config);
	linphone_proxy_config_compute_publish_params_hash(proxy_config);

	if (proxy_config->publish && proxy_config->presence_publish_event){
		linphone_event_pause_publish(proxy_config->presence_publish_event);
	}
	/*Don't stop refresher*/
}

void linphone_proxy_config_apply(LinphoneProxyConfig *proxy_config, LinphoneCore *lc){
	proxy_config->lc=lc;
	if (proxy_config->dependency != NULL) {
		//disable register if master proxy_config is not yet registered
		if (proxy_config->dependency->state != LinphoneRegistrationOk) {
			if (proxy_config->reg_sendregister != FALSE) {
				proxy_config->register_changed = TRUE;
			}
			//We do not call linphone_proxy_config_enable_register on purpose here
			//Explicitely disabling register on a dependent config puts it in a disabled state (see proxy_config->reg_dependent_disabled) to avoid automatic re-enable if masterCfg reach LinphoneRegistrationOk
		}
	}
	linphone_proxy_config_done(proxy_config);
}

void linphone_proxy_config_stop_refreshing(LinphoneProxyConfig * proxy_config){
	LinphoneAddress *contact_addr = NULL;
	const SalAddress *sal_addr = proxy_config->op && proxy_config->state == LinphoneRegistrationOk ? proxy_config->op->getContactAddress() : NULL;
	if (sal_addr) {
		char *buf = sal_address_to_string(sal_addr);
		contact_addr = buf ? linphone_address_new(buf) : NULL;
		ms_free(buf);
	}

	/*with udp, there is a risk of port reuse, so I prefer to not do anything for now*/
	if (contact_addr) {
		if (linphone_address_get_transport(contact_addr) != LinphoneTransportUdp && linphone_config_get_int(proxy_config->lc->config, "sip", "unregister_previous_contact", 0)) {
			if (proxy_config->pending_contact)
				linphone_address_unref(proxy_config->pending_contact);
			proxy_config->pending_contact=contact_addr;
		} else
			linphone_address_unref(contact_addr);
	}
	if (proxy_config->presence_publish_event){ /*might probably do better*/
		linphone_event_set_publish_state(proxy_config->presence_publish_event,LinphonePublishNone);
		linphone_event_unref(proxy_config->presence_publish_event); /*probably useless as proxy_config->long_term_event is already unref in linphone_proxy_config_notify_publish_state_changed. To be check with Ghislain*/
		proxy_config->presence_publish_event=NULL;
	}

	if (proxy_config->op){
		proxy_config->op->release();
		proxy_config->op=NULL;
	}
}

static LinphoneAddress *guess_contact_for_register (LinphoneProxyConfig *proxy_config) {
	LinphoneProxyConfig *ref = proxy_config;
	LinphoneAddress *result = nullptr;

	if (proxy_config->dependency) {
		//In case of dependent proxy config, force contact of 'master' proxy config, but only after a successful register
		return linphone_address_clone(proxy_config->dependency->contact_address);
	}
	LinphoneAddress *proxy = linphone_address_new(ref->reg_proxy);
	if (!proxy)
		return nullptr;
	const char *host = linphone_address_get_domain(proxy);
	if (host) {
		result = linphone_address_clone(ref->identity_address);
		if (ref->contact_params) {
			// We want to add a list of contacts params to the linphone address
			linphone_address_set_params(result, ref->contact_params);
		}
		if (ref->contact_uri_params)
			linphone_address_set_uri_params(result, ref->contact_uri_params);
	}
	linphone_address_unref(proxy);
	return result;
}

void _linphone_proxy_config_unregister(LinphoneProxyConfig *obj) {
	if (obj->op && (obj->state == LinphoneRegistrationOk ||
					(obj->state == LinphoneRegistrationProgress && obj->expires != 0))) {
		obj->op->unregister();
	}
}

static void linphone_proxy_config_register(LinphoneProxyConfig *proxy_config){
	if (proxy_config->reg_sendregister) {
		LinphoneAddress* proxy = linphone_address_new(proxy_config->reg_proxy);
		char *proxy_string;
		char *from = linphone_address_to_string(proxy_config->identity_address);
		ms_message("LinphoneProxyConfig [%p] about to register (LinphoneCore version: %s)",proxy_config,linphone_core_get_version());
		proxy_string=linphone_address_to_string_uri_only(proxy);
		linphone_address_unref(proxy);
		if (proxy_config->op)
			proxy_config->op->release();
		proxy_config->op=new SalRegisterOp(proxy_config->lc->sal);

		linphone_configure_op(proxy_config->lc, proxy_config->op, proxy_config->identity_address, proxy_config->sent_headers, FALSE);

		LinphoneAddress *contactAddress = guess_contact_for_register(proxy_config);
		if (contactAddress) {
			proxy_config->op->setContactAddress(L_GET_CPP_PTR_FROM_C_OBJECT(contactAddress)->getInternalAddress());
			if (!proxy_config->contact_address) {
				proxy_config->contact_address = linphone_address_clone(contactAddress);
			}
			linphone_address_unref(contactAddress);
		}
		proxy_config->op->setUserPointer(proxy_config);

		if (proxy_config->op->sendRegister(
			proxy_string,
			proxy_config->reg_identity,
			proxy_config->expires,
			proxy_config->pending_contact ? L_GET_CPP_PTR_FROM_C_OBJECT(proxy_config->pending_contact)->getInternalAddress() : NULL
		)==0) {
			if (proxy_config->pending_contact) {
				linphone_address_unref(proxy_config->pending_contact);
				proxy_config->pending_contact=NULL;
			}
			linphone_proxy_config_set_state(proxy_config,LinphoneRegistrationProgress,"Registration in progress");
		} else {
			linphone_proxy_config_set_state(proxy_config,LinphoneRegistrationFailed,"Registration failed");
		}
		ms_free(proxy_string);
		ms_free(from);
	} else {
		/* unregister if registered*/
		_linphone_proxy_config_unregister(proxy_config);
		if (proxy_config->state == LinphoneRegistrationProgress) {
			linphone_proxy_config_set_state(proxy_config,LinphoneRegistrationCleared,"Registration cleared");
		}
	}
}

void linphone_proxy_config_refresh_register(LinphoneProxyConfig *proxy_config){
	if (proxy_config->reg_sendregister && proxy_config->op && proxy_config->state!=LinphoneRegistrationProgress){
		if (proxy_config->op->refreshRegister(proxy_config->expires) == 0) {
			linphone_proxy_config_set_state(proxy_config,LinphoneRegistrationProgress, "Refresh registration");
		}
	}
}


void linphone_proxy_config_set_dial_prefix(LinphoneProxyConfig *proxy_config, const char *prefix){
	if (proxy_config->dial_prefix!=NULL){
		ms_free(proxy_config->dial_prefix);
		proxy_config->dial_prefix=NULL;
	}
	if (prefix && prefix[0]!='\0') proxy_config->dial_prefix=ms_strdup(prefix);
	
	/* Ensure there is a default proxy config otherwise after invalidating friends maps we won't be able to recompute phone numbers */
	/* Also it is useless to do it if the proxy config being edited isn't the default one */
	if (proxy_config->lc && proxy_config == linphone_core_get_default_proxy_config(proxy_config->lc)) {
		linphone_core_invalidate_friends_maps(proxy_config->lc);
	}
}

const char *linphone_proxy_config_get_dial_prefix(const LinphoneProxyConfig *proxy_config){
	return proxy_config->dial_prefix;
}

void linphone_proxy_config_set_dial_escape_plus(LinphoneProxyConfig *proxy_config, bool_t val){
	proxy_config->dial_escape_plus=val;
}

bool_t linphone_proxy_config_get_dial_escape_plus(const LinphoneProxyConfig *proxy_config){
	return proxy_config->dial_escape_plus;
}

void linphone_proxy_config_enable_quality_reporting(LinphoneProxyConfig *proxy_config, bool_t val){
	proxy_config->quality_reporting_enabled = val;
}

bool_t linphone_proxy_config_quality_reporting_enabled(LinphoneProxyConfig *proxy_config){
	return proxy_config->quality_reporting_enabled;
}

void linphone_proxy_config_set_quality_reporting_interval(LinphoneProxyConfig *proxy_config, int interval) {
	proxy_config->quality_reporting_interval = interval;
}

int linphone_proxy_config_get_quality_reporting_interval(LinphoneProxyConfig *proxy_config) {
	return proxy_config->quality_reporting_interval;
}

void linphone_proxy_config_set_quality_reporting_collector(LinphoneProxyConfig *proxy_config, const char *collector){
	if (collector!=NULL && strlen(collector)>0){
		LinphoneAddress *addr=linphone_address_new(collector);
		if (!addr){
			ms_error("Invalid SIP collector URI: %s. Quality reporting will be DISABLED.",collector);
		} else {
			if (proxy_config->quality_reporting_collector != NULL){
				ms_free(proxy_config->quality_reporting_collector);
			}
			proxy_config->quality_reporting_collector = ms_strdup(collector);
		}

		if (addr){
			linphone_address_unref(addr);
		}
	}
}

const char *linphone_proxy_config_get_quality_reporting_collector(const LinphoneProxyConfig *proxy_config){
	return proxy_config->quality_reporting_collector;
}


bool_t linphone_proxy_config_is_phone_number(LinphoneProxyConfig *proxy, const char *username){
	const char *p;
	if (!username) return FALSE;
	for(p=username;*p!='\0';++p){
		if (isdigit(*p) ||
			*p==' ' ||
			*p=='.' ||
			*p=='-' ||
			*p==')' ||
			*p=='(' ||
			*p=='/' ||
			*p=='+' ||
			(unsigned char)*p==0xca || (unsigned char)*p==0xc2 || (unsigned char)*p==0xa0 // non-breakable space (iOS uses it to format contacts phone number)
			) {
			continue;
		}
		return FALSE;
	}
	return TRUE;
}

//remove anything but [0-9] and +
static char *linphone_proxy_config_flatten_phone_number(LinphoneProxyConfig *proxy, const char *number) {
	char *result=reinterpret_cast<char *>(ms_malloc0(strlen(number)+1));
	char *w=result;
	const char *r;
	for(r=number;*r!='\0';++r){
		if (*r=='+' || isdigit(*r)){
			*w++=*r;
		}
	}
	*w++='\0';
	return result;
}

/*static char* replace_plus_with_icp(char *phone, const char* icp){
	return (icp && phone[0]=='+') ? ms_strdup_printf("%s%s", icp, phone+1) : ms_strdup(phone);
}*/

static char* replace_icp_with_plus(char *phone, const char *icp){
	return (strstr(phone, icp) == phone) ?  ms_strdup_printf("+%s", phone+strlen(icp)) : ms_strdup(phone);
}

bool_t linphone_proxy_config_normalize_number(LinphoneProxyConfig *proxy, const char *username, char *result, size_t result_len){
	char * normalized_phone = linphone_proxy_config_normalize_phone_number(proxy, username);
	const char * output = normalized_phone ? normalized_phone : username;
	memset(result, 0, result_len);
	memcpy(result, output, MIN(strlen(output) + 1, result_len));
	ms_free(normalized_phone);
	return output != username;
}

char* linphone_proxy_config_normalize_phone_number(LinphoneProxyConfig *proxy, const char *username) {
	LinphoneProxyConfig *tmpproxy = proxy ? proxy : linphone_core_create_proxy_config(NULL);
	char* result = NULL;
	std::shared_ptr<DialPlan> dialplan;
	char * nationnal_significant_number = NULL;
	int ccc = -1;

	if (linphone_proxy_config_is_phone_number(tmpproxy, username)){
		char * flatten = linphone_proxy_config_flatten_phone_number(proxy, username);
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
			if (tmpproxy->dial_prefix) {
				dialplan = DialPlan::findByCcc(tmpproxy->dial_prefix); //copy dial plan;
			} else {
				dialplan = DialPlan::MostCommon;
			}
			if (tmpproxy->dial_prefix){
				if (strcmp(tmpproxy->dial_prefix,dialplan->getCountryCallingCode().c_str()) != 0){
					//probably generic dialplan, preserving proxy dial prefix
					dialplan->setCountryCallingCode(tmpproxy->dial_prefix);
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
										, tmpproxy->dial_escape_plus ? dialplan->getInternationalCallPrefix().c_str() : "+"
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
				linphone_address_set_username(uri,username);
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

void linphone_proxy_config_set_etag(LinphoneProxyConfig *proxy_config,const char* sip_etag) {
	if (proxy_config->sip_etag) ms_free(proxy_config->sip_etag);
	if (sip_etag)
		proxy_config->sip_etag = ms_strdup(sip_etag);
	else
		proxy_config->sip_etag = NULL;
}
/**
 * Commits modification made to the proxy configuration.
**/
LinphoneStatus linphone_proxy_config_done(LinphoneProxyConfig *proxy_config)
{
	LinphoneProxyConfigAddressComparisonResult res;

	if (!linphone_proxy_config_check(proxy_config->lc,proxy_config))
		return -1;

	/*check if server address has changed*/
	res = linphone_proxy_config_is_server_config_changed(proxy_config);
	if (res != LinphoneProxyConfigAddressEqual) {
		/* server config has changed, need to unregister from previous first*/
		if (proxy_config->op) {
			if (res == LinphoneProxyConfigAddressDifferent) {
				_linphone_proxy_config_unregister(proxy_config);
			}
			proxy_config->op->setUserPointer(NULL); /*we don't want to receive status for this un register*/
			proxy_config->op->unref(); /*but we keep refresher to handle authentication if needed*/
			proxy_config->op=NULL;
		}
		if (proxy_config->presence_publish_event) {
			if (res == LinphoneProxyConfigAddressDifferent) {
				_linphone_proxy_config_unpublish(proxy_config);
			}
		}
		proxy_config->commit = TRUE;
	}

	if (proxy_config->register_changed){
		proxy_config->commit = TRUE;
		proxy_config->register_changed = FALSE;
	}
	if (proxy_config->commit){
		linphone_proxy_config_pause_register(proxy_config);
	}

	if (linphone_proxy_config_compute_publish_params_hash(proxy_config)) {
		ms_message("Publish params have changed on proxy config [%p]",proxy_config);
		if (proxy_config->presence_publish_event) {
			/*publish is terminated*/
			linphone_event_terminate(proxy_config->presence_publish_event);
		}
		if (proxy_config->publish) proxy_config->send_publish=TRUE;
	} else {
		ms_message("Publish params have not changed on proxy config [%p]",proxy_config);
	}
	linphone_proxy_config_write_all_to_config_file(proxy_config->lc);
	return 0;
}

const char* linphone_proxy_config_get_realm(const LinphoneProxyConfig *proxy_config)
{
	return proxy_config?proxy_config->realm:NULL;
}
void linphone_proxy_config_set_realm(LinphoneProxyConfig *proxy_config, const char *realm)
{
	if (proxy_config->realm!=NULL) {
		ms_free(proxy_config->realm);
	}
	proxy_config->realm=ms_strdup(realm);
}

int linphone_proxy_config_send_publish(LinphoneProxyConfig *proxy, LinphonePresenceModel *presence){
	int err=0;
	LinphoneAddress *presentity_address = NULL;
	char* contact = NULL;

	if (proxy->state==LinphoneRegistrationOk || proxy->state==LinphoneRegistrationCleared){
		LinphoneContent *content;
		char *presence_body;
		if (proxy->presence_publish_event==NULL){
			proxy->presence_publish_event = linphone_proxy_config_create_publish(proxy
										 , "presence"
										 , linphone_proxy_config_get_publish_expires(proxy));
			linphone_event_ref(proxy->presence_publish_event);
		}
		proxy->presence_publish_event->internal = TRUE;

		if (linphone_presence_model_get_presentity(presence) == NULL) {
			ms_message("No presentity set for model [%p], using identity from proxy config [%p]", presence, proxy);
			linphone_presence_model_set_presentity(presence,linphone_proxy_config_get_identity_address(proxy));
		}

		if (!linphone_address_equal(linphone_presence_model_get_presentity(presence), linphone_proxy_config_get_identity_address(proxy))) {
			ms_message("Presentity for model [%p] differ proxy config [%p], using proxy", presence, proxy);
			presentity_address = linphone_address_clone(linphone_presence_model_get_presentity(presence)); /*saved, just in case*/
			if (linphone_presence_model_get_contact(presence)) {
				contact = bctbx_strdup(linphone_presence_model_get_contact(presence));
			}
			linphone_presence_model_set_presentity(presence,linphone_proxy_config_get_identity_address(proxy));
			linphone_presence_model_set_contact(presence,NULL); /*it will be automatically computed*/

		}
		if (!(presence_body = linphone_presence_model_to_xml(presence))) {
			ms_error("Cannot publish presence model [%p] for proxy config [%p] because of xml serialization error",presence,proxy);
			return -1;
		}

		content = linphone_content_new();
		linphone_content_set_buffer(content, (const uint8_t *)presence_body,strlen(presence_body));
		linphone_content_set_type(content, "application");
		linphone_content_set_subtype(content,"pidf+xml");
		if (proxy->sip_etag) {
			linphone_event_add_custom_header(proxy->presence_publish_event, "SIP-If-Match", proxy->sip_etag);
			ms_free(proxy->sip_etag);
			proxy->sip_etag=NULL;
		}
		err = linphone_event_send_publish(proxy->presence_publish_event, content);
		linphone_content_unref(content);
		ms_free(presence_body);
		if (presentity_address) {
			linphone_presence_model_set_presentity(presence,presentity_address);
			linphone_address_unref(presentity_address);
		}
		if (contact) {
			linphone_presence_model_set_contact(presence,contact);
			bctbx_free(contact);
		}

	}else proxy->send_publish=TRUE; /*otherwise do not send publish if registration is in progress, this will be done later*/
	return err;
}

void _linphone_proxy_config_unpublish(LinphoneProxyConfig *obj) {
	if (obj->presence_publish_event
		&& (linphone_event_get_publish_state(obj->presence_publish_event) == LinphonePublishOk ||
					(linphone_event_get_publish_state(obj->presence_publish_event)  == LinphonePublishProgress && obj->publish_expires != 0))) {
		linphone_event_unpublish(obj->presence_publish_event);
	}
	if (obj->sip_etag) {
		ms_free(obj->sip_etag);
		obj->sip_etag=NULL;
	}
}

const char *linphone_proxy_config_get_route(const LinphoneProxyConfig *proxy_config) {
	if (proxy_config->reg_routes) return (const char *)bctbx_list_get_data(proxy_config->reg_routes);
	return NULL;
}

const bctbx_list_t* linphone_proxy_config_get_routes(const LinphoneProxyConfig *proxy_config) {
	return proxy_config->reg_routes;
}

const LinphoneAddress *linphone_proxy_config_get_identity_address(const LinphoneProxyConfig *proxy_config){
	return proxy_config->identity_address;
}

const char *linphone_proxy_config_get_identity(const LinphoneProxyConfig *proxy_config){
	return proxy_config->reg_identity;
}

bool_t linphone_proxy_config_publish_enabled(const LinphoneProxyConfig *proxy_config){
	return proxy_config->publish;
}

const char *linphone_proxy_config_get_server_addr(const LinphoneProxyConfig *proxy_config){
	return proxy_config->reg_proxy;
}

/**
 * @return the duration of registration.
**/
int linphone_proxy_config_get_expires(const LinphoneProxyConfig *proxy_config){
	return proxy_config->expires;
}

bool_t linphone_proxy_config_register_enabled(const LinphoneProxyConfig *proxy_config){
	return proxy_config->reg_sendregister;
}

void linphone_proxy_config_set_contact_parameters(LinphoneProxyConfig *proxy_config, const char *contact_params){
	if (proxy_config->contact_params) {
		ms_free(proxy_config->contact_params);
		proxy_config->contact_params=NULL;
	}
	if (contact_params){
		proxy_config->contact_params=ms_strdup(contact_params);
	}
	proxy_config->register_changed = TRUE;
}

void linphone_proxy_config_set_contact_uri_parameters(LinphoneProxyConfig *proxy_config, const char *contact_uri_params){
	if (proxy_config->contact_uri_params) {
		ms_free(proxy_config->contact_uri_params);
		proxy_config->contact_uri_params=NULL;
	}
	if (contact_uri_params){
		proxy_config->contact_uri_params=ms_strdup(contact_uri_params);
	}
	proxy_config->register_changed = TRUE;
}

const char *linphone_proxy_config_get_contact_parameters(const LinphoneProxyConfig *proxy_config){
	return proxy_config->contact_params;
}

const char *linphone_proxy_config_get_contact_uri_parameters(const LinphoneProxyConfig *proxy_config){
	return proxy_config->contact_uri_params;
}

struct _LinphoneCore * linphone_proxy_config_get_core(const LinphoneProxyConfig *proxy_config){
	return proxy_config->lc;
}

const char *linphone_proxy_config_get_custom_header(LinphoneProxyConfig *proxy_config, const char *header_name){
	const SalCustomHeader *ch;
	if (!proxy_config->op) return NULL;
	ch = proxy_config->op->getRecvCustomHeaders();
	return sal_custom_header_find(ch, header_name);
}

void linphone_proxy_config_set_custom_header(LinphoneProxyConfig *proxy_config, const char *header_name, const char *header_value){
	proxy_config->sent_headers=sal_custom_header_append(proxy_config->sent_headers, header_name, header_value);
	proxy_config->register_changed = TRUE;
}

LinphoneProxyConfig *linphone_proxy_config_get_dependency(LinphoneProxyConfig *proxy_config) {
	return proxy_config->dependency;
}

void linphone_proxy_config_set_dependency(LinphoneProxyConfig *proxy_config, LinphoneProxyConfig *dependency) {
	if (proxy_config) {
		if (proxy_config->depends_on) {
			ms_free(proxy_config->depends_on);
		}
		if (proxy_config->dependency) {
			linphone_proxy_config_unref(proxy_config->dependency);
		}
		if (dependency) {
			proxy_config->dependency = linphone_proxy_config_ref(dependency);
			proxy_config->depends_on = ms_strdup(dependency->idkey);
		} else {
			proxy_config->dependency = NULL;
			proxy_config->depends_on = NULL;
		}
	}
}

const char *linphone_proxy_config_get_idkey(LinphoneProxyConfig *proxy_config) {
	return proxy_config->idkey;
}

void linphone_proxy_config_set_idkey(LinphoneProxyConfig *proxy_config, const char *idkey) {
	if (proxy_config) {
		if (proxy_config->idkey) ms_free(proxy_config->idkey);
		proxy_config->idkey = ms_strdup(idkey);
	}
}


LinphoneStatus linphone_core_add_proxy_config(LinphoneCore *lc, LinphoneProxyConfig *proxy_config) {
	if (!linphone_proxy_config_check(lc,proxy_config)) {
		return -1;
	}
	if (bctbx_list_find(lc->sip_conf.proxies,proxy_config)!=NULL){
		ms_warning("ProxyConfig already entered, ignored.");
		return 0;
	}
	lc->sip_conf.proxies=bctbx_list_append(lc->sip_conf.proxies,(void *)linphone_proxy_config_ref(proxy_config));
	linphone_proxy_config_apply(proxy_config,lc);
	return 0;
}

//If a proxy config dependency is removed, restore 'normal' behavior for previously dependent configs
void linphone_core_remove_dependent_proxy_config(LinphoneCore *lc, LinphoneProxyConfig *proxy_config) {
	bctbx_list_t *it = lc->sip_conf.proxies;

	for (;it;it = it->next) {
		LinphoneProxyConfig *tmp = reinterpret_cast<LinphoneProxyConfig *>(it->data);
		if (tmp != proxy_config && tmp->dependency == proxy_config) {
		 	ms_message("Updating dependent proxy config [%p] caused by removal of 'master' proxy config idkey[%s]", tmp, proxy_config->idkey);
			linphone_proxy_config_set_dependency(tmp, NULL);
			proxy_config->commit = TRUE;
			linphone_proxy_config_update(tmp);
		}
	}
}

void linphone_core_remove_proxy_config(LinphoneCore *lc, LinphoneProxyConfig *proxy_config){
	/* check this proxy config is in the list before doing more*/
	if (bctbx_list_find(lc->sip_conf.proxies,proxy_config)==NULL) {
		ms_error("linphone_core_remove_proxy_config: LinphoneProxyConfig [%p] is not known by LinphoneCore (programming error?)",proxy_config);
		return;
	}
	lc->sip_conf.proxies=bctbx_list_remove(lc->sip_conf.proxies,proxy_config);
	linphone_core_remove_dependent_proxy_config(lc, proxy_config);
	/* add to the list of destroyed proxies, so that the possible unREGISTER request can succeed authentication */
	lc->sip_conf.deleted_proxies=bctbx_list_append(lc->sip_conf.deleted_proxies,proxy_config);

	if (lc->default_proxy==proxy_config){
		lc->default_proxy=NULL;
	}

	proxy_config->deletion_date=ms_time(NULL);
	if (proxy_config->state==LinphoneRegistrationOk){
		/* UNREGISTER */
		linphone_proxy_config_edit(proxy_config);
		linphone_proxy_config_enable_register(proxy_config,FALSE);
		linphone_proxy_config_done(proxy_config);
		linphone_proxy_config_update(proxy_config);
	} else if (proxy_config->state != LinphoneRegistrationNone) {
		linphone_proxy_config_set_state(proxy_config, LinphoneRegistrationNone,"Registration disabled");
	}
	linphone_proxy_config_write_all_to_config_file(lc);

	//Update the associated linphone specs on the core
	linphone_proxy_config_set_conference_factory_uri(proxy_config, NULL);
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
		if (tmp && tmp->idkey && strcmp(idkey, tmp->idkey) == 0) {
			return tmp;
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

void linphone_proxy_config_write_to_config_file(LpConfig *config, LinphoneProxyConfig *proxy_config, int index)
{
	char key[50];

	sprintf(key, "proxy_%i", index);
	linphone_config_clean_section(config, key);
	if (proxy_config == NULL){
		return;
	}
	if (proxy_config->type != NULL){
		linphone_config_set_string(config, key, "type", proxy_config->type);
	}
	if (proxy_config->reg_proxy != NULL){
		linphone_config_set_string(config, key, "reg_proxy", proxy_config->reg_proxy);
	}
	if (proxy_config->reg_routes != NULL) {
		linphone_config_set_string_list(config, key, "reg_route", proxy_config->reg_routes);
	}
	if (proxy_config->reg_identity != NULL){
		linphone_config_set_string(config, key, "reg_identity", proxy_config->reg_identity);
	}
	if (proxy_config->realm != NULL){
		linphone_config_set_string(config, key, "realm", proxy_config->realm);
	}
	if (proxy_config->contact_params != NULL){
		linphone_config_set_string(config, key, "contact_parameters", proxy_config->contact_params);
	}
	if (proxy_config->contact_uri_params != NULL){
		linphone_config_set_string(config, key, "contact_uri_parameters", proxy_config->contact_uri_params);
	}
	if (proxy_config->quality_reporting_collector != NULL){
		linphone_config_set_string(config, key, "quality_reporting_collector", proxy_config->quality_reporting_collector);
	}
	linphone_config_set_int(config, key, "quality_reporting_enabled", proxy_config->quality_reporting_enabled);
	linphone_config_set_int(config, key, "quality_reporting_interval", proxy_config->quality_reporting_interval);
	linphone_config_set_int(config, key, "reg_expires", proxy_config->expires);
	linphone_config_set_int(config, key, "reg_sendregister", proxy_config->reg_sendregister);
	linphone_config_set_int(config, key, "publish", proxy_config->publish);
	linphone_config_set_int(config, key, "avpf", proxy_config->avpf_mode);
	linphone_config_set_int(config, key, "avpf_rr_interval", proxy_config->avpf_rr_interval);
	linphone_config_set_int(config, key, "dial_escape_plus", proxy_config->dial_escape_plus);
	linphone_config_set_string(config,key,"dial_prefix", proxy_config->dial_prefix);
	linphone_config_set_int(config, key, "privacy", (int)proxy_config->privacy);
	linphone_config_set_int(config, key, "push_notification_allowed", (int)proxy_config->push_notification_allowed);
	if (proxy_config->refkey) linphone_config_set_string(config, key, "refkey", proxy_config->refkey);
	if (proxy_config->depends_on) linphone_config_set_string(config, key, "depends_on", proxy_config->depends_on);
	if (proxy_config->idkey) linphone_config_set_string(config, key, "idkey", proxy_config->idkey);
	linphone_config_set_int(config, key, "publish_expires", proxy_config->publish_expires);

	if (proxy_config->nat_policy != NULL) {
		linphone_config_set_string(config, key, "nat_policy_ref", proxy_config->nat_policy->ref);
		linphone_nat_policy_save_to_config(proxy_config->nat_policy);
	}

	linphone_config_set_string(config, key, "conference_factory_uri", proxy_config->conference_factory_uri);
}


#define CONFIGURE_STRING_VALUE(proxy_config,config,key,param,param_name) \
	{\
	char* default_value = linphone_proxy_config_get_##param(proxy_config)?ms_strdup(linphone_proxy_config_get_##param(proxy_config)):NULL;\
	linphone_proxy_config_set_##param(proxy_config,linphone_config_get_string(config,key,param_name,default_value)); \
	if ( default_value) ms_free(default_value); \
	}

#define CONFIGURE_BOOL_VALUE(proxy_config,config,key,param,param_name) \
	linphone_proxy_config_enable_##param(proxy_config, !!linphone_config_get_int(config,key,param_name,linphone_proxy_config_##param##_enabled(proxy_config)));

#define CONFIGURE_INT_VALUE(proxy_config,config,key,param,param_name, param_type) \
		linphone_proxy_config_set_##param(proxy_config, (param_type)linphone_config_get_int(config,key,param_name,(int)linphone_proxy_config_get_##param(proxy_config)));

LinphoneProxyConfig *linphone_proxy_config_new_from_config_file(LinphoneCore* lc, int index)
{
	const char *tmp;
	LinphoneProxyConfig *proxy_config;
	char key[50];
	LpConfig *config=lc->config;
	const char *nat_policy_ref;

	sprintf(key,"proxy_%i",index);

	if (!linphone_config_has_section(config,key)){
		return NULL;
	}

	proxy_config=linphone_core_create_proxy_config(lc);


	char* default_value = linphone_proxy_config_get_identity(proxy_config) ? ms_strdup(linphone_proxy_config_get_identity(proxy_config)) : NULL;
	const char *identity = linphone_config_get_string(config, key, "reg_identity", default_value);
	LinphoneAddress *identity_address = linphone_address_new(identity);
	linphone_proxy_config_set_identity_address(proxy_config, identity_address);
	if (identity_address) linphone_address_unref(identity_address);
	if (default_value) ms_free(default_value);

	CONFIGURE_STRING_VALUE(proxy_config, config, key, server_addr, "reg_proxy")
	bctbx_list_t *routes = linphone_config_get_string_list(config, key, "reg_route", NULL);
	linphone_proxy_config_set_routes(proxy_config, routes);
	if (routes)
		bctbx_list_free_with_data(routes, (bctbx_list_free_func)bctbx_free);

	CONFIGURE_STRING_VALUE(proxy_config, config, key, realm, "realm")

	CONFIGURE_BOOL_VALUE(proxy_config, config, key, quality_reporting, "quality_reporting_enabled")
	CONFIGURE_STRING_VALUE(proxy_config, config, key, quality_reporting_collector, "quality_reporting_collector")
	CONFIGURE_INT_VALUE(proxy_config, config, key, quality_reporting_interval, "quality_reporting_interval", int)

	CONFIGURE_STRING_VALUE(proxy_config, config, key, contact_parameters, "contact_parameters")
	CONFIGURE_STRING_VALUE(proxy_config, config, key, contact_uri_parameters, "contact_uri_parameters")

	CONFIGURE_INT_VALUE(proxy_config,config, key, expires, "reg_expires", int)
	CONFIGURE_BOOL_VALUE(proxy_config, config, key, register, "reg_sendregister")
	CONFIGURE_BOOL_VALUE(proxy_config, config, key, publish, "publish")
	linphone_proxy_config_set_push_notification_allowed(proxy_config, !!linphone_config_get_int(config, key, "push_notification_allowed", linphone_proxy_config_is_push_notification_allowed(proxy_config)));
	linphone_proxy_config_set_avpf_mode(proxy_config, static_cast<LinphoneAVPFMode>(linphone_config_get_int(config, key, "avpf", linphone_proxy_config_get_avpf_mode(proxy_config))));
	CONFIGURE_INT_VALUE(proxy_config, config, key, avpf_rr_interval, "avpf_rr_interval", uint8_t)
	CONFIGURE_INT_VALUE(proxy_config, config,key, dial_escape_plus, "dial_escape_plus", bool_t)
	CONFIGURE_STRING_VALUE(proxy_config, config, key, dial_prefix, "dial_prefix")

	tmp = linphone_config_get_string(config, key, "type", NULL);
	if (tmp != NULL && strlen(tmp) > 0)
		linphone_proxy_config_set_sip_setup(proxy_config, tmp);
	CONFIGURE_INT_VALUE(proxy_config, config, key, privacy, "privacy", LinphonePrivacyMask)

	CONFIGURE_STRING_VALUE(proxy_config, config, key, ref_key, "refkey")
	CONFIGURE_STRING_VALUE(proxy_config, config, key, idkey, "idkey")
	if (proxy_config->idkey == NULL) {
		proxy_config->idkey = generate_proxy_config_id();
		ms_warning("generated proxyconfig idkey = [%s]", proxy_config->idkey);
	}
	const char *depends_on = linphone_config_get_string(config, key, "depends_on", NULL);
	if (proxy_config->depends_on) {
		ms_free(proxy_config->depends_on);
	}
	proxy_config->depends_on = ms_strdup(depends_on);

	CONFIGURE_INT_VALUE(proxy_config, config, key, publish_expires, "publish_expires", int)

	nat_policy_ref = linphone_config_get_string(config, key, "nat_policy_ref", NULL);
	if (nat_policy_ref != NULL) {
		proxy_config->nat_policy = linphone_core_create_nat_policy_from_config(lc, nat_policy_ref);
	}

	CONFIGURE_STRING_VALUE(proxy_config, config, key, conference_factory_uri, "conference_factory_uri");

	return proxy_config;
}

static void linphone_proxy_config_activate_sip_setup(LinphoneProxyConfig *proxy_config){
	SipSetupContext *ssc;
	SipSetup *ss=sip_setup_lookup(proxy_config->type);
	unsigned int caps;
	if (!ss) return ;
	ssc=sip_setup_context_new(ss,proxy_config);
	proxy_config->ssctx=ssc;
	if (proxy_config->reg_identity==NULL){
		ms_error("Invalid identity for this proxy configuration.");
		return;
	}
	caps=(unsigned int)sip_setup_context_get_capabilities(ssc);
	if (caps & SIP_SETUP_CAP_ACCOUNT_MANAGER){
		if (sip_setup_context_login_account(ssc,proxy_config->reg_identity,NULL,NULL)!=0)
			return;
	}
	if (caps & SIP_SETUP_CAP_PROXY_PROVIDER){
		char proxy[256];
		if (sip_setup_context_get_proxy(ssc,NULL,proxy,sizeof(proxy))==0){
			linphone_proxy_config_set_server_addr(proxy_config,proxy);
		}else{
			ms_error("Could not retrieve proxy uri !");
		}
	}
}

SipSetup *linphone_proxy_config_get_sip_setup(LinphoneProxyConfig *proxy_config){
	if (proxy_config->ssctx!=NULL) return proxy_config->ssctx->funcs;
	if (proxy_config->type!=NULL){
		return sip_setup_lookup(proxy_config->type);
	}
	return NULL;
}

static bool_t can_register(LinphoneProxyConfig *proxy_config){
	LinphoneCore *lc=proxy_config->lc;

	if (lc->sip_conf.register_only_when_network_is_up && !lc->sip_network_state.global_state) {
		return FALSE;
	}
	if (proxy_config->dependency) {
		return linphone_proxy_config_get_state(proxy_config->dependency) == LinphoneRegistrationOk;
	}
	return TRUE;
}

void linphone_proxy_config_update(LinphoneProxyConfig *proxy_config){
	LinphoneCore *lc=proxy_config->lc;
	if (proxy_config->commit){
		if (proxy_config->type && proxy_config->ssctx==NULL){
			linphone_proxy_config_activate_sip_setup(proxy_config);
		}
		if (can_register(proxy_config)){
			linphone_proxy_config_register(proxy_config);
			proxy_config->commit=FALSE;
		}
	}
	if (proxy_config->send_publish && (proxy_config->state==LinphoneRegistrationOk || proxy_config->state==LinphoneRegistrationCleared)){
		linphone_proxy_config_send_publish(proxy_config,lc->presence_model);
		proxy_config->send_publish=FALSE;
	}
}

void linphone_proxy_config_set_sip_setup(LinphoneProxyConfig *proxy_config, const char *type){
	if (proxy_config->type)
		ms_free(proxy_config->type);
	proxy_config->type=ms_strdup(type);
	if (linphone_proxy_config_get_addr(proxy_config)==NULL){
		/*put a placeholder so that the sip setup gets saved into the config */
		linphone_proxy_config_set_server_addr(proxy_config,"sip:undefined");
	}
}

SipSetupContext *linphone_proxy_config_get_sip_setup_context(LinphoneProxyConfig *proxy_config){
	return proxy_config->ssctx;
}

void linphone_proxy_config_set_user_data(LinphoneProxyConfig *proxy_config, void *ud) {
	proxy_config->user_data = ud;
}

void *linphone_proxy_config_get_user_data(const LinphoneProxyConfig *proxy_config) {
	return proxy_config->user_data;
}

//Enable register on proxy config dependent on the given one (if any).
//Also force contact address of dependent proxy	config to the given one
void _linphone_update_dependent_proxy_config(LinphoneProxyConfig *proxy_config, LinphoneRegistrationState state, const char *message) {
	if (!proxy_config || !proxy_config->lc) return;
	bctbx_list_t *it = proxy_config->lc->sip_conf.proxies;

	for (;it;it = it->next) {
		LinphoneProxyConfig *tmp = reinterpret_cast<LinphoneProxyConfig *>(it->data);
		ms_message("_linphone_update_dependent_proxy_config(): %p is registered, checking for [%p] ->dependency=%p", proxy_config, tmp, tmp->dependency);
		
		if (tmp != proxy_config && tmp->dependency == proxy_config) {
			if (!tmp->reg_sendregister) {
				ms_message("Dependant proxy config [%p] has registration disabled, so it will not register.", tmp);
				continue;
			}
			linphone_proxy_config_edit(tmp);
			if (state == LinphoneRegistrationOk) {
				// Force dependent proxy config to re-register
				tmp->reg_sendregister = FALSE;
				linphone_proxy_config_enable_register(tmp, TRUE);
				const SalAddress *salAddr = proxy_config->op->getContactAddress();

				if (salAddr) {
					if (tmp->contact_address) {
						linphone_address_unref(tmp->contact_address);
					}
					char *sal_addr = sal_address_to_string(salAddr);
					tmp->contact_address = linphone_address_new(sal_addr);
					bctbx_free(sal_addr);
				}
			} else if (state == LinphoneRegistrationCleared || state == LinphoneRegistrationFailed) {
				linphone_proxy_config_pause_register(tmp);
				linphone_proxy_config_set_state(tmp, state, message);
			}
			linphone_proxy_config_done(tmp);
			linphone_proxy_config_update(tmp);
		}
	}
}

void linphone_proxy_config_set_state(LinphoneProxyConfig *proxy_config, LinphoneRegistrationState state, const char *message){
	LinphoneCore *lc = proxy_config->lc;

	if (proxy_config->state!=state || state==LinphoneRegistrationOk) { /*allow multiple notification of LinphoneRegistrationOk for refreshing*/
		ms_message("Proxy config [%p] for identity [%s] moving from state [%s] to [%s] on core [%p]"	,
					proxy_config,
					linphone_proxy_config_get_identity(proxy_config),
					linphone_registration_state_to_string(proxy_config->state),
					linphone_registration_state_to_string(state),
					proxy_config->lc);
		if (state == LinphoneRegistrationOk) {
			const SalAddress *salAddr = proxy_config->op->getContactAddress();
			if (salAddr)
				L_GET_CPP_PTR_FROM_C_OBJECT(proxy_config->contact_address)->setInternalAddress(salAddr);
		}
		if (linphone_core_should_subscribe_friends_only_when_registered(lc) && proxy_config->state!=state && state == LinphoneRegistrationOk) {
			ms_message("Updating friends for identity [%s] on core [%p]",proxy_config->reg_identity,proxy_config->lc);
			/* state must be updated before calling linphone_core_update_friends_subscriptions*/
			proxy_config->state=state;
			linphone_core_update_friends_subscriptions(lc);
		} else {
			/*at this point state must be updated*/
			proxy_config->state = state;
		}
		if (!proxy_config->dependency) {
			_linphone_update_dependent_proxy_config(proxy_config, state, message);
		}
		if (lc) {
			linphone_core_notify_registration_state_changed(lc,proxy_config,state,message);
		}
	} else {
		/*state already reported*/
	}
}

LinphoneRegistrationState linphone_proxy_config_get_state(const LinphoneProxyConfig *proxy_config){
	return proxy_config->state;
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

LinphoneReason linphone_proxy_config_get_error(const LinphoneProxyConfig *proxy_config) {
	return linphone_error_info_get_reason(linphone_proxy_config_get_error_info(proxy_config));
}

const LinphoneErrorInfo *linphone_proxy_config_get_error_info(const LinphoneProxyConfig *proxy_config){
	if (!proxy_config->ei) ((LinphoneProxyConfig*)proxy_config)->ei = linphone_error_info_new();
	linphone_error_info_from_sal_op(proxy_config->ei, proxy_config->op);
	return proxy_config->ei;
}

const LinphoneAddress* linphone_proxy_config_get_service_route(const LinphoneProxyConfig* proxy_config) {
	if (!proxy_config->op)
		return NULL;
	const SalAddress *salAddr = proxy_config->op->getServiceRoute();
	if (!salAddr)
		return NULL;
	if (proxy_config->service_route)
		L_GET_CPP_PTR_FROM_C_OBJECT(proxy_config->service_route)->setInternalAddress(const_cast<SalAddress *>(salAddr));
	else {
		char *buf = sal_address_to_string(salAddr);
		const_cast<LinphoneProxyConfig *>(proxy_config)->service_route = linphone_address_new(buf);
		ms_free(buf);
	}

	return proxy_config->service_route;
}

const char* linphone_proxy_config_get_transport(const LinphoneProxyConfig *proxy_config) {
	const char* addr=NULL;
	const char* ret="udp"; /*default value*/
	const SalAddress* route_addr=NULL;
	bool_t destroy_route_addr = FALSE;

	if (linphone_proxy_config_get_service_route(proxy_config)) {
		route_addr = L_GET_CPP_PTR_FROM_C_OBJECT(linphone_proxy_config_get_service_route(proxy_config))->getInternalAddress();
	} else if (linphone_proxy_config_get_route(proxy_config)) {
		addr=linphone_proxy_config_get_route(proxy_config);
	} else if(linphone_proxy_config_get_addr(proxy_config)) {
		addr=linphone_proxy_config_get_addr(proxy_config);
	} else {
		ms_error("Cannot guess transport for proxy with identity [%s]", proxy_config->reg_identity);
		return NULL;
	}

	if (!route_addr) {
		if (!((*(SalAddress **)&route_addr) = sal_address_new(addr)))
			return NULL;
		destroy_route_addr = TRUE;
	}

	ret=sal_transport_to_string(sal_address_get_transport(route_addr));
	if (destroy_route_addr)
		sal_address_unref((SalAddress *)route_addr);

	return ret;
}
void linphone_proxy_config_set_privacy(LinphoneProxyConfig *params, LinphonePrivacyMask privacy) {
	params->privacy=privacy;
}
LinphonePrivacyMask linphone_proxy_config_get_privacy(const LinphoneProxyConfig *params) {
	return params->privacy;
}
void linphone_proxy_config_set_publish_expires(LinphoneProxyConfig *proxy_config, int expires) {
	proxy_config->publish_expires=expires;
}
int linphone_proxy_config_get_publish_expires(const LinphoneProxyConfig *proxy_config) {
	if (proxy_config->publish_expires<0) {
		return proxy_config->expires; /*default value is same as register*/
	} else {
		return proxy_config->publish_expires;
	}
}

bool_t linphone_proxy_config_avpf_enabled(LinphoneProxyConfig *proxy_config) {
	if (proxy_config->avpf_mode==LinphoneAVPFDefault && proxy_config->lc){
		return linphone_core_get_avpf_mode(proxy_config->lc)==LinphoneAVPFEnabled;
	}
	return proxy_config->avpf_mode == LinphoneAVPFEnabled;
}

LinphoneAVPFMode linphone_proxy_config_get_avpf_mode(const LinphoneProxyConfig *proxy_config){
	return proxy_config->avpf_mode;
}

void linphone_proxy_config_set_avpf_mode(LinphoneProxyConfig *proxy_config, LinphoneAVPFMode mode){
	proxy_config->avpf_mode=mode;
}

void linphone_proxy_config_set_avpf_rr_interval(LinphoneProxyConfig *proxy_config, uint8_t interval) {
	if (interval > 5) interval = 5;
	proxy_config->avpf_rr_interval = interval;
}

uint8_t linphone_proxy_config_get_avpf_rr_interval(const LinphoneProxyConfig *proxy_config) {
	return proxy_config->avpf_rr_interval;
}

const LinphoneAddress *linphone_proxy_config_get_contact (const LinphoneProxyConfig *proxy_config) {
	return proxy_config->contact_address;
}

LinphoneAddress * linphone_proxy_config_get_transport_contact(LinphoneProxyConfig *proxy_config){
	LinphoneAddress *addr;
	int port = 0;
	const char *public_ip;

	if (!proxy_config->contact_address || !proxy_config->op) return NULL;
	public_ip = proxy_config->op->getPublicAddress(&port);
	if (!public_ip) return NULL;
	
	addr = linphone_address_clone(proxy_config->contact_address);
	linphone_address_set_domain(addr, public_ip);
	if (port > 0) linphone_address_set_port(addr, port);
	return addr;
}


const LinphoneAddress *_linphone_proxy_config_get_contact_without_params (const LinphoneProxyConfig *proxy_config) {
	if (proxy_config->contact_address_without_params) {
		linphone_address_unref(proxy_config->contact_address_without_params);
		const_cast<LinphoneProxyConfig *>(proxy_config)->contact_address_without_params = nullptr;
	}
	if (proxy_config->contact_address) {
		const_cast<LinphoneProxyConfig *>(proxy_config)->contact_address_without_params = linphone_address_clone(proxy_config->contact_address);
		linphone_address_clean(proxy_config->contact_address_without_params);
		linphone_address_set_port(proxy_config->contact_address_without_params, -1);
		linphone_address_set_domain(proxy_config->contact_address_without_params, nullptr);
		linphone_address_set_display_name(proxy_config->contact_address_without_params, nullptr);
	}
	return proxy_config->contact_address_without_params;
}

const struct _LinphoneAuthInfo* linphone_proxy_config_find_auth_info(const LinphoneProxyConfig *proxy_config) {
	const char* username = proxy_config->identity_address ? linphone_address_get_username(proxy_config->identity_address) : NULL;
	const char* domain =  proxy_config->identity_address ? linphone_address_get_domain(proxy_config->identity_address) : NULL;
	return linphone_core_find_auth_info(proxy_config->lc, proxy_config->realm, username, domain);
}

const char * linphone_proxy_config_get_ref_key(const LinphoneProxyConfig *proxy_config) {
	return proxy_config->refkey;
}

void linphone_proxy_config_set_ref_key(LinphoneProxyConfig *proxy_config, const char *refkey) {
	if (proxy_config->refkey!=NULL){
		ms_free(proxy_config->refkey);
		proxy_config->refkey=NULL;
	}
	if (refkey) proxy_config->refkey=ms_strdup(refkey);
}

LinphoneNatPolicy * linphone_proxy_config_get_nat_policy(const LinphoneProxyConfig *proxy_config) {
	return proxy_config->nat_policy;
}

void linphone_proxy_config_set_nat_policy(LinphoneProxyConfig *proxy_config, LinphoneNatPolicy *policy) {
	if (policy != NULL) {
		linphone_nat_policy_ref(policy); /* Prevent object destruction if the same policy is used */
		policy->lc = proxy_config->lc;
	}
	if (proxy_config->nat_policy != NULL) linphone_nat_policy_unref(proxy_config->nat_policy);
	proxy_config->nat_policy = policy;
}

void linphone_proxy_config_notify_publish_state_changed(LinphoneProxyConfig *proxy_config, LinphonePublishState state) {

	if (proxy_config->presence_publish_event != NULL) {
		switch (state) {
			case LinphonePublishCleared:
				linphone_proxy_config_set_etag(proxy_config,NULL);
				BCTBX_NO_BREAK;
			case LinphonePublishError:
				linphone_event_unref(proxy_config->presence_publish_event);
				proxy_config->presence_publish_event = NULL;
				break;
			case LinphonePublishOk:
				linphone_proxy_config_set_etag(proxy_config,linphone_event_get_custom_header(proxy_config->presence_publish_event, "SIP-ETag"));
				break;
			default:
				break;
		}
	}
}

void linphone_proxy_config_set_conference_factory_uri(LinphoneProxyConfig *proxy_config, const char *uri) {
	if (proxy_config->conference_factory_uri) {
		bctbx_free(proxy_config->conference_factory_uri);
		proxy_config->conference_factory_uri = nullptr;
	}
	if (uri && strlen(uri) > 0) {
		proxy_config->conference_factory_uri = bctbx_strdup(uri);
		if (proxy_config->lc) {
			linphone_core_add_linphone_spec(proxy_config->lc, "groupchat");
			linphone_core_add_linphone_spec(proxy_config->lc, "ephemeral");
		}
	} else if (proxy_config->lc) {
		bool_t remove = TRUE;
		//Check that no other proxy config needs the specs before removing it
		bctbx_list_t *it = NULL;
		for (it = proxy_config->lc->sip_conf.proxies; it; it = it->next) {
			if (it->data != proxy_config) {
				const char *confUri = linphone_proxy_config_get_conference_factory_uri((LinphoneProxyConfig *) it->data);
				if (confUri && strlen(confUri)) {
					remove = FALSE;
					break;
				}
			}
		}
		if (remove) {
			linphone_core_remove_linphone_spec(proxy_config->lc, "groupchat");
			linphone_core_remove_linphone_spec(proxy_config->lc, "ephemeral");
		}
	}
}

const char * linphone_proxy_config_get_conference_factory_uri(const LinphoneProxyConfig *proxy_config) {
	return proxy_config->conference_factory_uri;
}

bool_t linphone_proxy_config_is_push_notification_allowed(const LinphoneProxyConfig *proxy_config) {
	return proxy_config->push_notification_allowed;
}

void linphone_proxy_config_set_push_notification_allowed(LinphoneProxyConfig *proxy_config, bool_t is_allowed) {
	proxy_config->push_notification_allowed = is_allowed;

	if (is_allowed) {
		char *computedPushParams = linphone_core_get_push_notification_contact_uri_parameters(proxy_config->lc);
		if (computedPushParams) {
			linphone_proxy_config_edit(proxy_config);
			linphone_proxy_config_set_contact_uri_parameters(proxy_config, computedPushParams);
			linphone_proxy_config_done(proxy_config);
			ms_message("Push notification information [%s] added to proxy config [%p]", computedPushParams, proxy_config);
			ms_free(computedPushParams);
		}
	} else {
		linphone_proxy_config_edit(proxy_config);
		linphone_proxy_config_set_contact_uri_parameters(proxy_config, NULL);
		linphone_proxy_config_done(proxy_config);
		ms_message("Push notification information removed from proxy config [%p]", proxy_config);
	}
}

int linphone_proxy_config_get_unread_chat_message_count (const LinphoneProxyConfig *proxy_config) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(proxy_config->lc)->getUnreadChatMessageCount(
		LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(proxy_config->identity_address))
	);
}

LinphoneEvent *linphone_proxy_config_create_publish(LinphoneProxyConfig *proxy_config, const char *event, int expires) {
	if (!proxy_config->lc){
		ms_error("Cannot create publish from proxy config [%p] not attached to any core",proxy_config);
		return NULL;
	}
	return _linphone_core_create_publish(proxy_config->lc, proxy_config, NULL, event, expires);
}

char * linphone_proxy_config_to_string(const LinphoneProxyConfig *proxy_config){
	char *tmp;
	tmp=ms_strdup_printf("Proxy config %p\n", proxy_config);
	return tmp;
}
