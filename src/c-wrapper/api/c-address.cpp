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

#include "linphone/api/c-address.h"
#include "address/address.h"
#include "c-wrapper/c-wrapper.h"

using namespace LinphonePrivate;
using namespace std;

// =============================================================================

LinphoneAddress *linphone_address_new(const char *address) {
	LinphoneAddress *addr = Address::createCObject(L_C_TO_STRING(address));
	if (address && address[0] != '\0' && !Address::toCpp(addr)->isValid()) {
		linphone_address_unref(addr);
		addr = NULL;
	}
	return addr;
}

LinphoneAddress *linphone_address_clone(const LinphoneAddress *address) {
	return Address::toCpp(address)->clone()->toC();
}

LinphoneAddress *linphone_address_ref(LinphoneAddress *address) {
	belle_sip_object_ref(address);
	return address;
}

void linphone_address_unref(LinphoneAddress *address) {
	belle_sip_object_unref(address);
}

bool_t linphone_address_is_valid(const LinphoneAddress *address) {
	return address && Address::toCpp(address)->isValid();
}

const char *linphone_address_get_scheme(const LinphoneAddress *address) {
	return Address::toCpp(address)->getSchemeCstr();
}

const char *linphone_address_get_display_name(const LinphoneAddress *address) {
	return Address::toCpp(address)->getDisplayNameCstr();
}

LinphoneStatus linphone_address_set_display_name(LinphoneAddress *address, const char *display_name) {
	return Address::toCpp(address)->setDisplayName(L_C_TO_STRING(display_name)) ? 0 : -1;
}

const char *linphone_address_get_username(const LinphoneAddress *address) {
	return Address::toCpp(address)->getUsernameCstr();
}

LinphoneStatus linphone_address_set_username(LinphoneAddress *address, const char *username) {
	return Address::toCpp(address)->setUsername(L_C_TO_STRING(username)) ? 0 : -1;
}

const char *linphone_address_get_domain(const LinphoneAddress *address) {
	return Address::toCpp(address)->getDomainCstr();
}

LinphoneStatus linphone_address_set_domain(LinphoneAddress *address, const char *domain) {
	return Address::toCpp(address)->setDomain(L_C_TO_STRING(domain)) ? 0 : -1;
}

int linphone_address_get_port(const LinphoneAddress *address) {
	return Address::toCpp(address)->getPort();
}

LinphoneStatus linphone_address_set_port(LinphoneAddress *address, int port) {
	return Address::toCpp(address)->setPort(port) ? 0 : -1;
}

LinphoneTransportType linphone_address_get_transport(const LinphoneAddress *address) {
	return static_cast<LinphoneTransportType>(Address::toCpp(address)->getTransport());
}

LinphoneStatus linphone_address_set_transport(LinphoneAddress *address, LinphoneTransportType transport) {
	return Address::toCpp(address)->setTransport(static_cast<LinphonePrivate::Transport>(transport)) ? 0 : -1;
}

bool_t linphone_address_get_secure(const LinphoneAddress *address) {
	return Address::toCpp(address)->getSecure();
}

void linphone_address_set_secure(LinphoneAddress *address, bool_t enabled) {
	Address::toCpp(address)->setSecure(!!enabled);
}

bool_t linphone_address_is_sip(const LinphoneAddress *address) {
	return Address::toCpp(address)->isSip() ? TRUE : FALSE;
}

const char *linphone_address_get_method_param(const LinphoneAddress *address) {
	return Address::toCpp(address)->getMethodParamCstr();
}

void linphone_address_set_method_param(LinphoneAddress *address, const char *method_param) {
	Address::toCpp(address)->setMethodParam(L_C_TO_STRING(method_param));
}

const char *linphone_address_get_password(const LinphoneAddress *address) {
	return Address::toCpp(address)->getPasswordCstr();
}

void linphone_address_set_password(LinphoneAddress *address, const char *password) {
	Address::toCpp(address)->setPassword(L_C_TO_STRING(password));
}

void linphone_address_clean(LinphoneAddress *address) {
	Address::toCpp(address)->clean();
}

char *linphone_address_as_string(const LinphoneAddress *address) {
	return Address::toCpp(address)->toStringCstr();
}

char *linphone_address_as_string_uri_only(const LinphoneAddress *address) {
	return Address::toCpp(address)->asStringUriOnlyCstr();
}

char *linphone_address_as_string_uri_only_ordered(const LinphoneAddress *address) {
	return Address::toCpp(address)->toStringUriOnlyOrderedCstr();
}

bool_t linphone_address_weak_equal(const LinphoneAddress *address1, const LinphoneAddress *address2) {
	return (address1 && address2 && Address::toCpp(address1)->weakEqual(*Address::toCpp(address2))) ? TRUE : FALSE;
}

bool_t linphone_address_equal(const LinphoneAddress *address1, const LinphoneAddress *address2) {
	return address1 && address2 && *Address::toCpp(address1) == *Address::toCpp(address2);
}

const char *linphone_address_get_header(const LinphoneAddress *address, const char *header_name) {
	return Address::toCpp(address)->getHeaderValueCstr(header_name);
}

void linphone_address_set_header(LinphoneAddress *address, const char *header_name, const char *header_value) {
	Address::toCpp(address)->setHeader(header_name, L_C_TO_STRING(header_value));
}

bool_t linphone_address_has_param(const LinphoneAddress *address, const char *param_name) {
	return Address::toCpp(address)->hasParam(param_name) ? TRUE : FALSE;
}

const char *linphone_address_get_param(const LinphoneAddress *address, const char *param_name) {
	return Address::toCpp(address)->getParamValueCstr(param_name);
}

void linphone_address_set_param(LinphoneAddress *address, const char *param_name, const char *param_value) {
	Address::toCpp(address)->setParam(param_name, L_C_TO_STRING(param_value));
}

void linphone_address_set_params(LinphoneAddress *address, const char *params) {
	Address::toCpp(address)->setParams(L_C_TO_STRING(params));
}

bool_t linphone_address_has_uri_param(const LinphoneAddress *address, const char *uri_param_name) {
	return Address::toCpp(address)->hasUriParam(uri_param_name) ? TRUE : FALSE;
}

const char *linphone_address_get_uri_param(const LinphoneAddress *address, const char *uri_param_name) {
	return Address::toCpp(address)->getUriParamValueCstr(uri_param_name);
}

void linphone_address_set_uri_param(LinphoneAddress *address, const char *uri_param_name, const char *uri_param_value) {
	Address::toCpp(address)->setUriParam(uri_param_name, L_C_TO_STRING(uri_param_value));
}

void linphone_address_set_uri_params(LinphoneAddress *address, const char *params) {
	Address::toCpp(address)->setUriParams(L_C_TO_STRING(params));
}

void linphone_address_remove_uri_param(LinphoneAddress *address, const char *uri_param_name) {
	Address::toCpp(address)->removeUriParam(L_C_TO_STRING(uri_param_name));
}

bool_t linphone_address_lesser(const LinphoneAddress *address, const LinphoneAddress *other) {
	auto othercpp = Address::toCpp(other);
	return Address::toCpp(address)->operator<(*othercpp);
}

void linphone_address_destroy(LinphoneAddress *address) {
	belle_sip_object_unref(address);
}

bool_t linphone_address_is_secure(const LinphoneAddress *address) {
	return Address::toCpp(address)->getSecure() ? TRUE : FALSE;
}
