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

#include <bctoolbox/defs.h>

#include "c-wrapper/internal/c-tools.h"
#include "ldap/ldap-params.h"
#include "linphone/api/c-ldap-params.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"

// =============================================================================

using namespace LinphonePrivate;

LinphoneLdapParams *linphone_ldap_params_clone(const LinphoneLdapParams *params) {
	return LdapParams::toCpp(params)->clone()->toC();
}

LinphoneLdapParams *linphone_ldap_params_ref(LinphoneLdapParams *params) {
	LdapParams::toCpp(params)->ref();
	return params;
}

void linphone_ldap_params_unref(LinphoneLdapParams *params) {
	LdapParams::toCpp(params)->unref();
}

/*****************************************************************************************************************/

void linphone_ldap_params_set_custom_value(LinphoneLdapParams *params, const char *key, const char *value) {
	LdapParams::toCpp(params)->setCustomValue(L_C_TO_STRING(key), L_C_TO_STRING(value));
}

const char *linphone_ldap_params_get_custom_value(const LinphoneLdapParams *params, const char *key) {
	return L_STRING_TO_C(LdapParams::toCpp(params)->getCustomValue(L_C_TO_STRING(key)));
}

/*****************************************************************************************************************/

void linphone_ldap_params_set_server(LinphoneLdapParams *params, const char *server) {
	LdapParams::toCpp(params)->setServer(L_C_TO_STRING(server));
}

const char *linphone_ldap_params_get_server(const LinphoneLdapParams *params) {
	return L_STRING_TO_C(LdapParams::toCpp(params)->getServer());
}

/*************************************************************************************/
void linphone_ldap_params_set_bind_dn(LinphoneLdapParams *params, const char *bind_dn) {
	LdapParams::toCpp(params)->setBindDn(L_C_TO_STRING(bind_dn));
}

const char *linphone_ldap_params_get_bind_dn(const LinphoneLdapParams *params) {
	return L_STRING_TO_C(LdapParams::toCpp(params)->getBindDn());
}

/*************************************************************************************/

void linphone_ldap_params_set_base_object(LinphoneLdapParams *params, const char *base_object) {
	LdapParams::toCpp(params)->setBaseObject(L_C_TO_STRING(base_object));
}

const char *linphone_ldap_params_get_base_object(const LinphoneLdapParams *params) {
	return L_STRING_TO_C(LdapParams::toCpp(params)->getBaseObject());
}

/*************************************************************************************/

void linphone_ldap_params_set_timeout(LinphoneLdapParams *params, int timeout) {
	LdapParams::toCpp(params)->setTimeout(timeout);
}

int linphone_ldap_params_get_timeout(const LinphoneLdapParams *params) {
	return LdapParams::toCpp(params)->getTimeout();
}

void linphone_ldap_params_set_timeout_tls_ms(LinphoneLdapParams *params, int timeout) {
	LdapParams::toCpp(params)->setTimeoutTlsMs(timeout);
}

int linphone_ldap_params_get_timeout_tls_ms(const LinphoneLdapParams *params) {
	return LdapParams::toCpp(params)->getTimeoutTlsMs();
}
/*************************************************************************************/

void linphone_ldap_params_set_max_results(LinphoneLdapParams *params, int max_results) {
	LdapParams::toCpp(params)->setMaxResults(max_results);
}

int linphone_ldap_params_get_max_results(const LinphoneLdapParams *params) {
	return LdapParams::toCpp(params)->getMaxResults();
}

/*************************************************************************************/

void linphone_ldap_params_set_min_chars(LinphoneLdapParams *params, int min_chars) {
	LdapParams::toCpp(params)->setMinChars(min_chars);
}

int linphone_ldap_params_get_min_chars(const LinphoneLdapParams *params) {
	return LdapParams::toCpp(params)->getMinChars();
}

/*************************************************************************************/

void linphone_ldap_params_set_delay(LinphoneLdapParams *params, int delay) {
	LdapParams::toCpp(params)->setDelay(delay);
}

int linphone_ldap_params_get_delay(const LinphoneLdapParams *params) {
	return LdapParams::toCpp(params)->getDelay();
}

/*************************************************************************************/

void linphone_ldap_params_set_auth_method(LinphoneLdapParams *params, LinphoneLdapAuthMethod auth_method) {
	LdapParams::toCpp(params)->setAuthMethod(auth_method);
}

LinphoneLdapAuthMethod linphone_ldap_params_get_auth_method(const LinphoneLdapParams *params) {
	return LdapParams::toCpp(params)->getAuthMethod();
}
/*************************************************************************************/

void linphone_ldap_params_set_password(LinphoneLdapParams *params, const char *password) {
	LdapParams::toCpp(params)->setPassword(L_C_TO_STRING(password));
}

const char *linphone_ldap_params_get_password(const LinphoneLdapParams *params) {
	return L_STRING_TO_C(LdapParams::toCpp(params)->getPassword());
}
/*************************************************************************************/

void linphone_ldap_params_set_filter(LinphoneLdapParams *params, const char *filter) {
	LdapParams::toCpp(params)->setFilter(L_C_TO_STRING(filter));
}

const char *linphone_ldap_params_get_filter(const LinphoneLdapParams *params) {
	return L_STRING_TO_C(LdapParams::toCpp(params)->getFilter());
}

/*************************************************************************************/

void linphone_ldap_params_set_name_attribute(LinphoneLdapParams *params, const char *name_attribute) {
	LdapParams::toCpp(params)->setNameAttribute(L_C_TO_STRING(name_attribute));
}

const char *linphone_ldap_params_get_name_attribute(const LinphoneLdapParams *params) {
	return L_STRING_TO_C(LdapParams::toCpp(params)->getNameAttribute());
}

/*************************************************************************************/

void linphone_ldap_params_set_sip_attribute(LinphoneLdapParams *params, const char *sip_attribute) {
	LdapParams::toCpp(params)->setSipAttribute(L_C_TO_STRING(sip_attribute));
}

const char *linphone_ldap_params_get_sip_attribute(const LinphoneLdapParams *params) {
	return L_STRING_TO_C(LdapParams::toCpp(params)->getSipAttribute());
}

/*************************************************************************************/

void linphone_ldap_params_set_sip_domain(LinphoneLdapParams *params, const char *sip_domain) {
	LdapParams::toCpp(params)->setSipDomain(L_C_TO_STRING(sip_domain));
}

const char *linphone_ldap_params_get_sip_domain(const LinphoneLdapParams *params) {
	return L_STRING_TO_C(LdapParams::toCpp(params)->getSipDomain());
}

/*************************************************************************************/

void linphone_ldap_params_set_enabled(LinphoneLdapParams *params, bool_t enable) {
	LdapParams::toCpp(params)->setEnabled(enable);
}

bool_t linphone_ldap_params_get_enabled(const LinphoneLdapParams *params) {
	return LdapParams::toCpp(params)->getEnabled();
}

/*************************************************************************************/

void linphone_ldap_params_enable_sal(LinphoneLdapParams *params, bool_t enable) {
	LdapParams::toCpp(params)->enableSal(enable);
}

bool_t linphone_ldap_params_sal_enabled(const LinphoneLdapParams *params) {
	return LdapParams::toCpp(params)->salEnabled();
}

/*************************************************************************************/

void linphone_ldap_params_enable_tls(LinphoneLdapParams *params, bool_t enable) {
	LdapParams::toCpp(params)->enableTls(enable);
}

bool_t linphone_ldap_params_tls_enabled(const LinphoneLdapParams *params) {
	return LdapParams::toCpp(params)->tlsEnabled();
}

/*************************************************************************************/

void linphone_ldap_params_set_debug_level(LinphoneLdapParams *params, LinphoneLdapDebugLevel level) {
	LdapParams::toCpp(params)->setDebugLevel(level);
}

LinphoneLdapDebugLevel linphone_ldap_params_get_debug_level(const LinphoneLdapParams *params) {
	return LdapParams::toCpp(params)->getDebugLevel();
}

/*************************************************************************************/

void linphone_ldap_params_set_server_certificates_verification_mode(
    LinphoneLdapParams *params, LinphoneLdapCertVerificationMode verify_server_certificates) {
	LdapParams::toCpp(params)->setServerCertificatesVerificationMode(verify_server_certificates);
}

LinphoneLdapCertVerificationMode
linphone_ldap_params_get_server_certificates_verification_mode(const LinphoneLdapParams *params) {
	return LdapParams::toCpp(params)->getServerCertificatesVerificationMode();
}

/*************************************************************************************/

int linphone_ldap_params_check(const LinphoneLdapParams *params) {
	return LdapParams::toCpp(params)->check();
}
