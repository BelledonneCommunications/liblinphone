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

#include "linphone/api/c-nat-policy.h"
#include "core/core.h"
#include "nat/nat-policy.h"
#include "private_functions.h"

using namespace ::LinphonePrivate;

LinphoneNatPolicy *linphone_core_create_nat_policy(LinphoneCore *lc) {
	return (new NatPolicy(L_GET_CPP_PTR_FROM_C_OBJECT(lc)))->toC();
}

LinphoneNatPolicy *linphone_core_create_nat_policy_from_ref(LinphoneCore *core, const char *ref) {
	return (new NatPolicy(L_GET_CPP_PTR_FROM_C_OBJECT(core), NatPolicy::ConstructionMethod::FromRefName, ref))->toC();
}

LinphoneNatPolicy *linphone_core_create_nat_policy_from_config(LinphoneCore *core, const char *section) {
	return (new NatPolicy(L_GET_CPP_PTR_FROM_C_OBJECT(core), NatPolicy::ConstructionMethod::FromSectionName, section))
	    ->toC();
}

LinphoneNatPolicy *linphone_nat_policy_clone(const LinphoneNatPolicy *other) {
	return (new NatPolicy(*NatPolicy::toCpp(other)))->toC();
}

LinphoneNatPolicy *linphone_nat_policy_ref(LinphoneNatPolicy *policy) {
	belle_sip_object_ref(policy);
	return policy;
}

void linphone_nat_policy_unref(LinphoneNatPolicy *policy) {
	belle_sip_object_unref(policy);
}

void *linphone_nat_policy_get_user_data(const LinphoneNatPolicy *policy) {
	return NatPolicy::toCpp(policy)->getUserData();
}

void linphone_nat_policy_set_user_data(LinphoneNatPolicy *policy, void *ud) {
	NatPolicy::toCpp(policy)->setUserData(ud);
}

void linphone_nat_policy_clear(LinphoneNatPolicy *policy) {
	NatPolicy::toCpp(policy)->clear();
}

bool_t linphone_nat_policy_stun_enabled(const LinphoneNatPolicy *policy) {
	return NatPolicy::toCpp(policy)->stunEnabled() ? TRUE : FALSE;
}

void linphone_nat_policy_enable_stun(LinphoneNatPolicy *policy, bool_t enable) {
	NatPolicy::toCpp(policy)->enableStun(!!enable);
}

bool_t linphone_nat_policy_turn_enabled(const LinphoneNatPolicy *policy) {
	return NatPolicy::toCpp(policy)->turnEnabled() ? TRUE : FALSE;
}

void linphone_nat_policy_enable_turn(LinphoneNatPolicy *policy, bool_t enable) {
	NatPolicy::toCpp(policy)->enableTurn(!!enable);
}

bool_t linphone_nat_policy_ice_enabled(const LinphoneNatPolicy *policy) {
	return NatPolicy::toCpp(policy)->iceEnabled() ? TRUE : FALSE;
}

void linphone_nat_policy_enable_ice(LinphoneNatPolicy *policy, bool_t enable) {
	NatPolicy::toCpp(policy)->enableIce(!!enable);
}

bool_t linphone_nat_policy_upnp_enabled(const LinphoneNatPolicy *policy) {
	return NatPolicy::toCpp(policy)->upnpEnabled() ? TRUE : FALSE;
}

void linphone_nat_policy_enable_upnp(LinphoneNatPolicy *policy, bool_t enable) {
	if (enable) {
		ms_warning("uPnP NAT policy is no longer supported");
	}
	NatPolicy::toCpp(policy)->enableUpnp(!!enable);
}

const char *linphone_nat_policy_get_stun_server(const LinphoneNatPolicy *policy) {
	return L_STRING_TO_C(NatPolicy::toCpp(policy)->getStunServer());
}

void linphone_nat_policy_resolve_stun_server(LinphoneNatPolicy *policy) {
	NatPolicy::toCpp(policy)->resolveStunServer();
}

const struct addrinfo *linphone_nat_policy_get_stun_server_addrinfo(LinphoneNatPolicy *policy) {
	return NatPolicy::toCpp(policy)->getStunServerAddrinfo();
}

void linphone_nat_policy_set_stun_server(LinphoneNatPolicy *policy, const char *stun_server) {
	NatPolicy::toCpp(policy)->setStunServer(L_C_TO_STRING(stun_server));
}

const char *linphone_nat_policy_get_stun_server_username(const LinphoneNatPolicy *policy) {
	return L_STRING_TO_C(NatPolicy::toCpp(policy)->getStunServerUsername());
}

void linphone_nat_policy_set_stun_server_username(LinphoneNatPolicy *policy, const char *username) {
	NatPolicy::toCpp(policy)->setStunServerUsername(L_C_TO_STRING(username));
}

void linphone_nat_policy_enable_udp_turn_transport(LinphoneNatPolicy *policy, bool_t enable) {
	NatPolicy::toCpp(policy)->enableTurnUdp(!!enable);
}

bool_t linphone_nat_policy_udp_turn_transport_enabled(const LinphoneNatPolicy *policy) {
	return NatPolicy::toCpp(policy)->turnUdpEnabled() ? TRUE : FALSE;
}

void linphone_nat_policy_enable_tcp_turn_transport(LinphoneNatPolicy *policy, bool_t enable) {
	NatPolicy::toCpp(policy)->enableTurnTcp(!!enable);
}

bool_t linphone_nat_policy_tcp_turn_transport_enabled(const LinphoneNatPolicy *policy) {
	return NatPolicy::toCpp(policy)->turnTcpEnabled() ? TRUE : FALSE;
}

void linphone_nat_policy_enable_tls_turn_transport(LinphoneNatPolicy *policy, bool_t enable) {
	NatPolicy::toCpp(policy)->enableTurnTls(!!enable);
}

bool_t linphone_nat_policy_tls_turn_transport_enabled(const LinphoneNatPolicy *policy) {
	return NatPolicy::toCpp(policy)->turnTlsEnabled() ? TRUE : FALSE;
}

LinphoneCore *linphone_nat_policy_get_core(const LinphoneNatPolicy *policy) {
	return NatPolicy::toCpp(policy)->getCore()->getCCore();
}

const char *linphone_nat_policy_get_nat_v4_address(const LinphoneNatPolicy *policy) {
	return L_STRING_TO_C(NatPolicy::toCpp(policy)->getNatV4Address());
}

void linphone_nat_policy_set_nat_v4_address(LinphoneNatPolicy *policy, const char *v4_address) {
	NatPolicy::toCpp(policy)->setNatV4Address(L_C_TO_STRING(v4_address));
}

const char *linphone_nat_policy_get_nat_v6_address(const LinphoneNatPolicy *policy) {
	return L_STRING_TO_C(NatPolicy::toCpp(policy)->getNatV6Address());
}

void linphone_nat_policy_set_nat_v6_address(LinphoneNatPolicy *policy, const char *v6_address) {
	NatPolicy::toCpp(policy)->setNatV6Address(L_C_TO_STRING(v6_address));
}

const char *linphone_nat_policy_get_turn_configuration_endpoint(const LinphoneNatPolicy *policy) {
	return L_STRING_TO_C(NatPolicy::toCpp(policy)->getTurnConfigurationEndpoint());
}

void linphone_nat_policy_set_turn_configuration_endpoint(LinphoneNatPolicy *policy, const char *endpoint) {
	NatPolicy::toCpp(policy)->setTurnConfigurationEndpoint(L_C_TO_STRING(endpoint));
}