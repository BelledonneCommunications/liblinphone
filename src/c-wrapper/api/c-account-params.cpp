/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
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

#include "account/account-params.h"
#include "c-wrapper/internal/c-tools.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-address.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"

// =============================================================================

using namespace LinphonePrivate;

LinphoneAccountParams* linphone_account_params_new(LinphoneCore *lc) {
	return AccountParams::createCObject(lc);
}

LinphoneAccountParams* linphone_account_params_new_with_config(LinphoneCore *lc, int index) {
	char key[50];
	sprintf(key, "proxy_%i", index); // TODO: change to account

	if (!linphone_config_has_section(linphone_core_get_config(lc), key)){
		return NULL;
	}

	return AccountParams::createCObject(lc, index);
}

LinphoneAccountParams* linphone_account_params_clone(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->clone()->toC();
}

LinphoneAccountParams* linphone_account_params_ref(LinphoneAccountParams *params) {
	AccountParams::toCpp(params)->ref();
	return params;
}

void linphone_account_params_unref(LinphoneAccountParams *params) {
	AccountParams::toCpp(params)->unref();
}

void linphone_account_params_set_user_data(LinphoneAccountParams *params, void *user_data) {
	AccountParams::toCpp(params)->setUserData(user_data);
}

void* linphone_account_params_get_user_data(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getUserData();
}

LinphoneStatus linphone_account_params_set_server_address(LinphoneAccountParams *params, const LinphoneAddress *server_address) {
	return AccountParams::toCpp(params)->setServerAddress(server_address);
}

LinphoneStatus linphone_account_params_set_server_addr(LinphoneAccountParams *params, const char *server_address) {
	return AccountParams::toCpp(params)->setServerAddressAsString(server_address);
}

LinphoneStatus linphone_account_params_set_identity_address(LinphoneAccountParams *params, const LinphoneAddress *identity) {
	return AccountParams::toCpp(params)->setIdentityAddress(identity);
}

LinphoneStatus linphone_account_params_set_routes_addresses(LinphoneAccountParams *params, const bctbx_list_t *routes) {
	return AccountParams::toCpp(params)->setRoutes(routes);
}

void linphone_account_params_set_expires(LinphoneAccountParams *params, int expires) {
	AccountParams::toCpp(params)->setExpires(expires);
}

void linphone_account_params_set_register_enabled(LinphoneAccountParams *params, bool_t enable) {
	AccountParams::toCpp(params)->setRegisterEnabled(enable);
}

void linphone_account_params_set_publish_enabled(LinphoneAccountParams *params, bool_t enable) {
	AccountParams::toCpp(params)->setPublishEnabled(enable);
}

void linphone_account_params_set_publish_expires(LinphoneAccountParams *params, int expires) {
	AccountParams::toCpp(params)->setPublishExpires(expires);
}

int linphone_account_params_get_publish_expires(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getPublishExpires();
}

void linphone_account_params_set_dial_escape_plus_enabled(LinphoneAccountParams *params, bool_t enable) {
	AccountParams::toCpp(params)->setDialEscapePlusEnabled(enable);
}

void linphone_account_params_set_international_prefix(LinphoneAccountParams *params, const char *prefix) {
	AccountParams::toCpp(params)->setInternationalPrefix(L_C_TO_STRING(prefix));
}

void linphone_account_params_set_use_international_prefix_for_calls_and_chats(LinphoneAccountParams* params, bool_t enable) {
	AccountParams::toCpp(params)->setUseInternationalPrefixForCallsAndChats(enable);
}

void linphone_account_params_set_quality_reporting_enabled(LinphoneAccountParams *params, bool_t enable) {
	AccountParams::toCpp(params)->setQualityReportingEnabled(enable);
}

bool_t linphone_account_params_get_quality_reporting_enabled(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getQualityReportingEnabled();
}

void linphone_account_params_set_quality_reporting_collector(LinphoneAccountParams *params, const char *collector) {
	AccountParams::toCpp(params)->setQualityReportingCollector(L_C_TO_STRING(collector));
}

const char* linphone_account_params_get_quality_reporting_collector(const LinphoneAccountParams *params) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getQualityReportingCollector());
}

void linphone_account_params_set_quality_reporting_interval(LinphoneAccountParams *params, int interval) {
	AccountParams::toCpp(params)->setQualityReportingInterval(interval);
}

int linphone_account_params_get_quality_reporting_interval(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getQualityReportingInterval();
}

const char* linphone_account_params_get_domain(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getDomain();
}

const char* linphone_account_params_get_realm(const LinphoneAccountParams *params) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getRealm());
}

void linphone_account_params_set_realm(LinphoneAccountParams *params, const char *realm) {
	AccountParams::toCpp(params)->setRealm(L_C_TO_STRING(realm));
}

const bctbx_list_t* linphone_account_params_get_routes_addresses(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getRoutes();
}

const LinphoneAddress *linphone_account_params_get_identity_address(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getIdentityAddress();
}

const char *linphone_account_params_get_identity(const LinphoneAccountParams *params) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getIdentity());
}

bool_t linphone_account_params_get_publish_enabled(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getPublishEnabled();
}

const LinphoneAddress *linphone_account_params_get_server_address(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getServerAddress();
}

const char* linphone_account_params_get_server_addr(const LinphoneAccountParams *params) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getServerAddressAsString());
}

int linphone_account_params_get_expires(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getExpires();
}

bool_t linphone_account_params_get_register_enabled(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getRegisterEnabled();
}

const char* linphone_account_params_get_contact_parameters(const LinphoneAccountParams *params) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getContactParameters());
}

void linphone_account_params_set_contact_parameters(LinphoneAccountParams *params, const char *contact_params) {
	AccountParams::toCpp(params)->setContactParameters(L_C_TO_STRING(contact_params));
}

void linphone_account_params_set_contact_uri_parameters(LinphoneAccountParams *params, const char *contact_uri_params) {
	AccountParams::toCpp(params)->setContactUriParameters(L_C_TO_STRING(contact_uri_params));
}

const char* linphone_account_params_get_contact_uri_parameters(const LinphoneAccountParams *params) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getContactUriParameters());
}

bool_t linphone_account_params_get_dial_escape_plus_enabled(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getDialEscapePlusEnabled();
}

const char* linphone_account_params_get_international_prefix(const LinphoneAccountParams *params) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getInternationalPrefix());
}

bool_t linphone_account_params_get_use_international_prefix_for_calls_and_chats(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getUseInternationalPrefixForCallsAndChats();
}

void linphone_account_params_set_privacy(LinphoneAccountParams *params, LinphonePrivacyMask privacy) {
	AccountParams::toCpp(params)->setPrivacy(privacy);
}

LinphonePrivacyMask linphone_account_params_get_privacy(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getPrivacy();
}

void linphone_account_params_set_file_transfer_server(LinphoneAccountParams *params, const char *server_url) {
	AccountParams::toCpp(params)->setFileTranferServer(std::string(server_url));
}

const char* linphone_account_params_get_file_transfer_server(const LinphoneAccountParams *params) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getFileTransferServer());
}

void linphone_account_params_set_avpf_rr_interval(LinphoneAccountParams *params, uint8_t interval) {
	AccountParams::toCpp(params)->setAvpfRrInterval(interval);
}

uint8_t linphone_account_params_get_avpf_rr_interval(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getAvpfRrInterval();
}

LinphoneAVPFMode linphone_account_params_get_avpf_mode(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getAvpfMode();
}

void linphone_account_params_set_avpf_mode(LinphoneAccountParams *params, LinphoneAVPFMode mode) {
	AccountParams::toCpp(params)->setAvpfMode(mode);
}

const char* linphone_account_params_get_ref_key(const LinphoneAccountParams *params) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getRefKey());
}

void linphone_account_params_set_ref_key(LinphoneAccountParams *params, const char *refkey) {
	AccountParams::toCpp(params)->setRefKey(L_C_TO_STRING(refkey));
}

const char* linphone_account_params_get_idkey(const LinphoneAccountParams *params) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getIdKey());
}

void linphone_account_params_set_idkey(LinphoneAccountParams *params, const char *idkey) {
	AccountParams::toCpp(params)->setIdKey(L_C_TO_STRING(idkey));
}

LinphoneNatPolicy* linphone_account_params_get_nat_policy(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getNatPolicy();
}

void linphone_account_params_set_nat_policy(LinphoneAccountParams *params, LinphoneNatPolicy *policy) {
	AccountParams::toCpp(params)->setNatPolicy(policy);
}

void linphone_account_params_set_conference_factory_uri(LinphoneAccountParams *params, const char *uri) {
	AccountParams::toCpp(params)->setConferenceFactoryUri(L_C_TO_STRING(uri));
}

const char* linphone_account_params_get_conference_factory_uri(const LinphoneAccountParams *params) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getConferenceFactoryUri());
}

void linphone_account_params_set_push_notification_allowed(LinphoneAccountParams *params, bool_t allow) {
	AccountParams::toCpp(params)->setPushNotificationAllowed(allow);
}

bool_t linphone_account_params_get_push_notification_allowed(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getPushNotificationAllowed();
}

void linphone_account_params_set_remote_push_notification_allowed(LinphoneAccountParams *params, bool_t allow) {
	AccountParams::toCpp(params)->setRemotePushNotificationAllowed(allow);
}

bool_t linphone_account_params_get_remote_push_notification_allowed(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getRemotePushNotificationAllowed();
}

bool_t linphone_account_params_is_push_notification_available(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->isPushNotificationAvailable();
}

void linphone_account_params_set_push_notification_config(LinphoneAccountParams *params, LinphonePushNotificationConfig *config) {
	AccountParams::toCpp(params)->setPushNotificationConfig(config);
}

LinphonePushNotificationConfig *linphone_account_params_get_push_notification_config(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getPushNotificationConfig();
}

void linphone_account_params_set_outbound_proxy_enabled(LinphoneAccountParams *params, bool_t enable) {
	AccountParams::toCpp(params)->setOutboundProxyEnabled(enable);
}

bool_t linphone_account_params_get_outbound_proxy_enabled(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getOutboundProxyEnabled();
}

void linphone_account_params_set_transport(LinphoneAccountParams *params, LinphoneTransportType transport) {
	AccountParams::toCpp(params)->setTransport(transport);
}

LinphoneTransportType linphone_account_params_get_transport(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getTransport();
}
