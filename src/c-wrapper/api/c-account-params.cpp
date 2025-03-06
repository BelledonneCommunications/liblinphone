/*
 * Copyright (c) 2010-2025 Belledonne Communications SARL.
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

#include "linphone/api/c-account-params.h"

#include "account/account-params.h"
#include "c-wrapper/internal/c-tools.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"
#include "linphone/utils/utils.h"
#include "nat/nat-policy.h"
#include "push-notification/push-notification-config.h"

// =============================================================================

using namespace LinphonePrivate;

LinphoneAccountParams *linphone_account_params_new(LinphoneCore *lc, bool_t use_default_values) {
	return AccountParams::createCObject(lc, !!use_default_values);
}

LinphoneAccountParams *linphone_account_params_new_with_config(LinphoneCore *lc, int index) {
	char key[50];
	snprintf(key, sizeof(key), "proxy_%i", index); // TODO: change to account

	if (!linphone_config_has_section(linphone_core_get_config(lc), key)) {
		return nullptr;
	}

	return AccountParams::createCObject(lc, index);
}

LinphoneAccountParams *linphone_account_params_clone(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->clone()->toC();
}

LinphoneAccountParams *linphone_account_params_ref(LinphoneAccountParams *params) {
	AccountParams::toCpp(params)->ref();
	return params;
}

void linphone_account_params_unref(LinphoneAccountParams *params) {
	AccountParams::toCpp(params)->unref();
}

void linphone_account_params_set_user_data(LinphoneAccountParams *params, void *user_data) {
	AccountParams::toCpp(params)->setUserData(user_data);
}

void *linphone_account_params_get_user_data(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getUserData();
}

LinphoneStatus linphone_account_params_set_server_address(LinphoneAccountParams *params,
                                                          const LinphoneAddress *server_address) {
	return AccountParams::toCpp(params)->setServerAddress(bellesip::getSharedPtr<Address>(server_address));
}

LinphoneStatus linphone_account_params_set_server_addr(LinphoneAccountParams *params, const char *server_address) {
	return AccountParams::toCpp(params)->setServerAddressAsString(L_C_TO_STRING(server_address));
}

LinphoneStatus linphone_account_params_set_identity_address(LinphoneAccountParams *params,
                                                            const LinphoneAddress *identity) {
	return AccountParams::toCpp(params)->setIdentityAddress(bellesip::getSharedPtr<Address>(identity));
}

LinphoneStatus linphone_account_params_set_routes_addresses(LinphoneAccountParams *params, const bctbx_list_t *routes) {
	const std::list<std::shared_ptr<LinphonePrivate::Address>> routeList =
	    LinphonePrivate::Utils::bctbxListToCppSharedPtrList<LinphoneAddress, LinphonePrivate::Address>(routes);
	return AccountParams::toCpp(params)->setRoutes(routeList);
}

void linphone_account_params_set_expires(LinphoneAccountParams *params, int expires) {
	AccountParams::toCpp(params)->setExpires(expires);
}

void linphone_account_params_set_register_enabled(LinphoneAccountParams *params, bool_t enable) {
	linphone_account_params_enable_register(params, enable);
}

void linphone_account_params_enable_register(LinphoneAccountParams *params, bool_t enable) {
	AccountParams::toCpp(params)->setRegisterEnabled(enable);
}

void linphone_account_params_set_publish_enabled(LinphoneAccountParams *params, bool_t enable) {
	linphone_account_params_enable_publish(params, enable);
}

void linphone_account_params_enable_publish(LinphoneAccountParams *params, bool_t enable) {
	AccountParams::toCpp(params)->setPublishEnabled(enable);
}

void linphone_account_params_set_publish_expires(LinphoneAccountParams *params, int expires) {
	AccountParams::toCpp(params)->setPublishExpires(expires);
}

int linphone_account_params_get_publish_expires(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getPublishExpires();
}

void linphone_account_params_set_dial_escape_plus_enabled(LinphoneAccountParams *params, bool_t enable) {
	linphone_account_params_enable_dial_escape_plus(params, enable);
}

void linphone_account_params_enable_dial_escape_plus(LinphoneAccountParams *params, bool_t enable) {
	AccountParams::toCpp(params)->setDialEscapePlusEnabled(enable);
}

void linphone_account_params_set_international_prefix(LinphoneAccountParams *params, const char *prefix) {
	AccountParams::toCpp(params)->setInternationalPrefix(L_C_TO_STRING(prefix));
}

void linphone_account_params_set_international_prefix_iso_country_code(LinphoneAccountParams *params,
                                                                       const char *prefix_iso_country_code) {
	AccountParams::toCpp(params)->setInternationalPrefixIsoCountryCode(L_C_TO_STRING(prefix_iso_country_code));
}

void linphone_account_params_set_use_international_prefix_for_calls_and_chats(LinphoneAccountParams *params,
                                                                              bool_t enable) {
	AccountParams::toCpp(params)->setUseInternationalPrefixForCallsAndChats(enable);
}

void linphone_account_params_set_quality_reporting_enabled(LinphoneAccountParams *params, bool_t enable) {
	linphone_account_params_enable_quality_reporting(params, enable);
}

void linphone_account_params_enable_quality_reporting(LinphoneAccountParams *params, bool_t enable) {
	AccountParams::toCpp(params)->setQualityReportingEnabled(enable);
}

bool_t linphone_account_params_get_quality_reporting_enabled(const LinphoneAccountParams *params) {
	return linphone_account_params_quality_reporting_enabled(params);
}

bool_t linphone_account_params_quality_reporting_enabled(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getQualityReportingEnabled();
}

void linphone_account_params_set_quality_reporting_collector(LinphoneAccountParams *params, const char *collector) {
	AccountParams::toCpp(params)->setQualityReportingCollector(L_C_TO_STRING(collector));
}

const char *linphone_account_params_get_quality_reporting_collector(const LinphoneAccountParams *params) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getQualityReportingCollector());
}

void linphone_account_params_set_quality_reporting_interval(LinphoneAccountParams *params, int interval) {
	AccountParams::toCpp(params)->setQualityReportingInterval(interval);
}

int linphone_account_params_get_quality_reporting_interval(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getQualityReportingInterval();
}

const char *linphone_account_params_get_domain(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getDomainCstr();
}

const char *linphone_account_params_get_realm(const LinphoneAccountParams *params) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getRealm());
}

void linphone_account_params_set_realm(LinphoneAccountParams *params, const char *realm) {
	AccountParams::toCpp(params)->setRealm(L_C_TO_STRING(realm));
}

bctbx_list_t *linphone_account_params_get_routes_addresses(const LinphoneAccountParams *params) {
	bctbx_list_t *route_list = nullptr;
	for (const auto &route : AccountParams::toCpp(params)->getRoutes()) {
		route_list = bctbx_list_append(route_list, route->toC());
	}
	return route_list;
}

const LinphoneAddress *linphone_account_params_get_identity_address(const LinphoneAccountParams *params) {
	return toC(AccountParams::toCpp(params)->getIdentityAddress());
}

const char *linphone_account_params_get_identity(const LinphoneAccountParams *params) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getIdentity());
}

bool_t linphone_account_params_get_publish_enabled(const LinphoneAccountParams *params) {
	return linphone_account_params_publish_enabled(params);
}

bool_t linphone_account_params_publish_enabled(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getPublishEnabled();
}

const LinphoneAddress *linphone_account_params_get_server_address(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getServerAddress()->toC();
}

const char *linphone_account_params_get_server_addr(const LinphoneAccountParams *params) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getServerAddressAsString());
}

int linphone_account_params_get_expires(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getExpires();
}

bool_t linphone_account_params_get_register_enabled(const LinphoneAccountParams *params) {
	return linphone_account_params_register_enabled(params);
}

bool_t linphone_account_params_register_enabled(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getRegisterEnabled();
}

const char *linphone_account_params_get_contact_parameters(const LinphoneAccountParams *params) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getContactParameters());
}

void linphone_account_params_set_contact_parameters(LinphoneAccountParams *params, const char *contact_params) {
	AccountParams::toCpp(params)->setContactParameters(L_C_TO_STRING(contact_params));
}

void linphone_account_params_set_contact_uri_parameters(LinphoneAccountParams *params, const char *contact_uri_params) {
	AccountParams::toCpp(params)->setContactUriParameters(L_C_TO_STRING(contact_uri_params));
}

const char *linphone_account_params_get_contact_uri_parameters(const LinphoneAccountParams *params) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getContactUriParameters());
}

bool_t linphone_account_params_get_dial_escape_plus_enabled(const LinphoneAccountParams *params) {
	return linphone_account_params_dial_escape_plus_enabled(params);
}

bool_t linphone_account_params_dial_escape_plus_enabled(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getDialEscapePlusEnabled();
}

const char *linphone_account_params_get_international_prefix(const LinphoneAccountParams *params) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getInternationalPrefix());
}

const char *linphone_account_params_get_international_prefix_iso_country_code(const LinphoneAccountParams *params) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getInternationalPrefixIsoCountryCode());
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
	AccountParams::toCpp(params)->setFileTransferServer(std::string(server_url));
}

const char *linphone_account_params_get_file_transfer_server(const LinphoneAccountParams *params) {
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

const char *linphone_account_params_get_ref_key(const LinphoneAccountParams *params) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getRefKey());
}

void linphone_account_params_set_ref_key(LinphoneAccountParams *params, const char *refkey) {
	AccountParams::toCpp(params)->setRefKey(L_C_TO_STRING(refkey));
}

const char *linphone_account_params_get_idkey(const LinphoneAccountParams *params) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getIdKey());
}

void linphone_account_params_set_idkey(LinphoneAccountParams *params, const char *idkey) {
	AccountParams::toCpp(params)->setIdKey(L_C_TO_STRING(idkey));
}

LinphoneNatPolicy *linphone_account_params_get_nat_policy(const LinphoneAccountParams *params) {
	auto pol = AccountParams::toCpp(params)->getNatPolicy();
	return pol ? pol->toC() : nullptr;
}

void linphone_account_params_set_nat_policy(LinphoneAccountParams *params, LinphoneNatPolicy *policy) {
	AccountParams::toCpp(params)->setNatPolicy(policy ? NatPolicy::toCpp(policy)->getSharedFromThis() : nullptr);
}

void linphone_account_params_set_conference_factory_uri(LinphoneAccountParams *params, const char *uri) {
	AccountParams::toCpp(params)->setConferenceFactoryUri(L_C_TO_STRING(uri));
}

const char *linphone_account_params_get_conference_factory_uri(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getConferenceFactoryCstr();
}

void linphone_account_params_set_conference_factory_address(LinphoneAccountParams *params,
                                                            const LinphoneAddress *address) {
	AccountParams::toCpp(params)->setConferenceFactoryAddress(address ? Address::toCpp(address)->getSharedFromThis()
	                                                                  : nullptr);
}

const LinphoneAddress *linphone_account_params_get_conference_factory_address(const LinphoneAccountParams *params) {
	const auto &address = AccountParams::toCpp(params)->getConferenceFactoryAddress();
	return address != nullptr ? address->toC() : nullptr;
}

void linphone_account_params_set_audio_video_conference_factory_address(LinphoneAccountParams *params,
                                                                        const LinphoneAddress *address) {
	AccountParams::toCpp(params)->setAudioVideoConferenceFactoryAddress(
	    address ? Address::toCpp(address)->getSharedFromThis() : nullptr);
}

const LinphoneAddress *
linphone_account_params_get_audio_video_conference_factory_address(const LinphoneAccountParams *params) {
	const auto &address = AccountParams::toCpp(params)->getAudioVideoConferenceFactoryAddress();
	return address != nullptr ? address->toC() : nullptr;
}

const char *linphone_account_params_get_ccmp_user_id(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getCcmpUserIdCstr();
}

void linphone_account_params_set_ccmp_server_url(LinphoneAccountParams *params, const char *url) {
	AccountParams::toCpp(params)->setCcmpServerUrl(L_C_TO_STRING(url));
}

const char *linphone_account_params_get_ccmp_server_url(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getCcmpServerUrlCstr();
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

void linphone_account_params_set_push_notification_config(LinphoneAccountParams *params,
                                                          LinphonePushNotificationConfig *config) {
	AccountParams::toCpp(params)->setPushNotificationConfig(PushNotificationConfig::toCpp(config));
}

LinphonePushNotificationConfig *
linphone_account_params_get_push_notification_config(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getPushNotificationConfig()->toC();
}

void linphone_account_params_enable_unregister_at_stop(LinphoneAccountParams *params, bool_t enable) {
	AccountParams::toCpp(params)->setUnregisterAtStop(enable);
}

bool_t linphone_account_params_unregister_at_stop_enabled(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getUnregisterAtStop();
}

void linphone_account_params_set_outbound_proxy_enabled(LinphoneAccountParams *params, bool_t enable) {
	linphone_account_params_enable_outbound_proxy(params, enable);
}

void linphone_account_params_enable_outbound_proxy(LinphoneAccountParams *params, bool_t enable) {
	AccountParams::toCpp(params)->setOutboundProxyEnabled(enable);
}

bool_t linphone_account_params_get_outbound_proxy_enabled(const LinphoneAccountParams *params) {
	return linphone_account_params_outbound_proxy_enabled(params);
}

bool_t linphone_account_params_outbound_proxy_enabled(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getOutboundProxyEnabled();
}

void linphone_account_params_set_transport(LinphoneAccountParams *params, LinphoneTransportType transport) {
	AccountParams::toCpp(params)->setTransport(static_cast<LinphonePrivate::Transport>(transport));
}

LinphoneTransportType linphone_account_params_get_transport(const LinphoneAccountParams *params) {
	return static_cast<LinphoneTransportType>(AccountParams::toCpp(params)->getTransport());
}

void linphone_account_params_enable_rtp_bundle(LinphoneAccountParams *params, bool_t value) {
	AccountParams::toCpp(params)->enableRtpBundle(!!value);
}

bool_t linphone_account_params_rtp_bundle_enabled(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->rtpBundleEnabled();
}

void linphone_account_params_enable_rtp_bundle_assumption(LinphoneAccountParams *params, bool_t value) {
	AccountParams::toCpp(params)->enableRtpBundleAssumption(!!value);
}

bool_t linphone_account_params_rtp_bundle_assumption_enabled(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->rtpBundleAssumptionEnabled();
}

bool_t linphone_account_params_cpim_in_basic_chat_room_enabled(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->isCpimMessagesAllowedInBasicChatRooms();
}

void linphone_account_params_enable_cpim_in_basic_chat_room(LinphoneAccountParams *params, bool_t enable) {
	AccountParams::toCpp(params)->setCpimMessagesAllowedInBasicChatRooms(enable);
}

void linphone_account_params_add_custom_param(LinphoneAccountParams *params, const char *key, const char *value) {
	AccountParams::toCpp(params)->addCustomParam(L_C_TO_STRING(key), L_C_TO_STRING(value));
}

const char *linphone_account_params_get_custom_param(const LinphoneAccountParams *params, const char *key) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getCustomParam(L_C_TO_STRING(key)));
}

void linphone_account_params_set_custom_contact(LinphoneAccountParams *params, LinphoneAddress *contact) {
	AccountParams::toCpp(params)->setCustomContact(contact != nullptr ? Address::toCpp(contact)->getSharedFromThis()
	                                                                  : nullptr);
}

const LinphoneAddress *linphone_account_params_get_custom_contact(const LinphoneAccountParams *params) {
	const auto customContact = AccountParams::toCpp(params)->getCustomContact();
	return customContact != nullptr ? customContact->toC() : nullptr;
}

void linphone_account_params_set_lime_server_url(LinphoneAccountParams *params, const char *url) {
	AccountParams::toCpp(params)->setLimeServerUrl(L_C_TO_STRING(url));
}

const char *linphone_account_params_get_lime_server_url(const LinphoneAccountParams *params) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getLimeServerUrl());
}

void linphone_account_params_set_lime_algo(LinphoneAccountParams *params, const char *algo) {
	AccountParams::toCpp(params)->setLimeAlgo(L_C_TO_STRING(algo));
}

const char *linphone_account_params_get_lime_algo(const LinphoneAccountParams *params) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getLimeAlgo());
}

void linphone_account_params_set_picture_uri(LinphoneAccountParams *params, const char *uri) {
	AccountParams::toCpp(params)->setPictureUri(L_C_TO_STRING(uri));
}

const char *linphone_account_params_get_picture_uri(const LinphoneAccountParams *params) {
	return L_STRING_TO_C(AccountParams::toCpp(params)->getPictureUri());
}

void linphone_account_params_set_mwi_server_address(LinphoneAccountParams *params, LinphoneAddress *address) {
	AccountParams::toCpp(params)->setMwiServerAddress(address ? Address::toCpp(address)->getSharedFromThis() : nullptr);
}

const LinphoneAddress *linphone_account_params_get_mwi_server_address(const LinphoneAccountParams *params) {
	const std::shared_ptr<const Address> addr = AccountParams::toCpp(params)->getMwiServerAddress();
	return addr ? addr->toC() : nullptr;
}

void linphone_account_params_set_voicemail_address(LinphoneAccountParams *params, LinphoneAddress *address) {
	AccountParams::toCpp(params)->setVoicemailAddress(address ? Address::toCpp(address)->getSharedFromThis() : nullptr);
}

const LinphoneAddress *linphone_account_params_get_voicemail_address(const LinphoneAccountParams *params) {
	const std::shared_ptr<const Address> addr = AccountParams::toCpp(params)->getVoicemailAddress();
	return addr ? addr->toC() : nullptr;
}

bool_t linphone_account_params_get_instant_messaging_encryption_mandatory(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->isInstantMessagingEncryptionMandatory();
}

void linphone_account_params_set_instant_messaging_encryption_mandatory(LinphoneAccountParams *params,
                                                                        bool_t mandatory) {
	AccountParams::toCpp(params)->setInstantMessagingEncryptionMandatory(mandatory);
}

const bctbx_list_t *linphone_account_params_get_supported_tags_list(const LinphoneAccountParams *params) {
	return AccountParams::toCpp(params)->getSupportedTagsCList();
}

void linphone_account_params_set_supported_tags_list(LinphoneAccountParams *params,
                                                     const bctbx_list_t *supported_tags) {
	list<string> supportedTagsList = {};
	for (auto tag = supported_tags; tag != nullptr; tag = tag->next) {
		supportedTagsList.push_back(string((char *)tag->data));
	}
	AccountParams::toCpp(params)->setSupportedTagsList(supportedTagsList);
}
