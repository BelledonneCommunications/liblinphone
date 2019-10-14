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

#ifndef LINPHONE_LDAPPROVIDER_H_
#define LINPHONE_LDAPPROVIDER_H_


#include "linphone/contactprovider.h"


#ifdef __cplusplus
extern "C" {
#endif

/* LinphoneLDAPContactSearch */

LinphoneLDAPContactSearch * linphone_ldap_contact_search_create(LinphoneLDAPContactProvider *ld, const char *predicate, ContactSearchCallback cb, void *cb_data);

LINPHONE_PUBLIC unsigned int linphone_ldap_contact_search_result_count(LinphoneLDAPContactSearch *obj);

LINPHONE_PUBLIC LinphoneLDAPContactSearch * linphone_ldap_contact_search_cast(void *obj);


/* LinphoneLDAPContactProvider */

LINPHONE_PUBLIC LinphoneLDAPContactProvider * linphone_ldap_contact_provider_create(LinphoneCore *lc, const LinphoneDictionary *config);

LINPHONE_PUBLIC unsigned int linphone_ldap_contact_provider_get_max_result(const LinphoneLDAPContactProvider *obj);

LINPHONE_PUBLIC LinphoneLDAPContactProvider * linphone_ldap_contact_provider_ref(void *obj);

void linphone_ldap_contact_provider_unref(void *obj);

LinphoneLDAPContactProvider * linphone_ldap_contact_provider_cast(void *obj);

LINPHONE_PUBLIC int linphone_ldap_contact_provider_available(void);

#ifdef __cplusplus
}
#endif


#endif /* LINPHONE_LDAPPROVIDER_H_ */
