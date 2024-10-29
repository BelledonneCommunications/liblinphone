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

#include <ctype.h>

#include "c-wrapper/c-wrapper.h"
#include "ldap/ldap-params.h"
#include "ldap/ldap.h"
#include "linphone/api/c-ldap-params.h"
#include "linphone/api/c-ldap.h"
#include "linphone/wrapper_utils.h"
#include "utils/enum.h"

// =============================================================================

using namespace LinphonePrivate;

LinphoneLdap *linphone_ldap_new(LinphoneCore *core) {
	return Ldap::createCObject(L_GET_CPP_PTR_FROM_C_OBJECT(core));
}

LinphoneLdap *linphone_ldap_new_with_params(LinphoneCore *core, LinphoneLdapParams *params) {
	return Ldap::createCObject(L_GET_CPP_PTR_FROM_C_OBJECT(core), LdapParams::toCpp(params)->getSharedFromThis());
}

LinphoneLdap *linphone_ldap_ref(LinphoneLdap *ldap) {
	Ldap::toCpp(ldap)->ref();
	return ldap;
}

void linphone_ldap_unref(LinphoneLdap *ldap) {
	Ldap::toCpp(ldap)->unref();
}

void linphone_ldap_set_params(LinphoneLdap *ldap, LinphoneLdapParams *params) {
	Ldap::toCpp(ldap)->setLdapParams(LdapParams::toCpp(params)->getSharedFromThis());
}

LinphoneLdapParams *linphone_ldap_get_params(LinphoneLdap *ldap) {
	return Ldap::toCpp(ldap)->getLdapParams()->toC();
}

LinphoneCore *linphone_ldap_get_core(LinphoneLdap *ldap) {
	return Ldap::toCpp(ldap)->getCore()->getCCore();
}

void linphone_ldap_set_index(LinphoneLdap *ldap, int index) {
	return Ldap::toCpp(ldap)->setIndex(index);
}

int linphone_ldap_get_index(const LinphoneLdap *ldap) {
	return Ldap::toCpp(ldap)->getIndex();
}
