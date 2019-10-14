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

#ifndef CONTACT_PROVIDERS_PRIV_H
#define CONTACT_PROVIDERS_PRIV_H

#include "linphone/core.h"

#include "c-wrapper/c-wrapper.h"

/* Base for contact search and contact provider */

struct _LinphoneContactSearch{
	belle_sip_object_t base;
	LinphoneContactSearchID id;
	char* predicate;
	ContactSearchCallback cb;
	void* data;
};

#define LINPHONE_CONTACT_SEARCH(obj) BELLE_SIP_CAST(obj,LinphoneContactSearch)
BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneContactSearch)


struct _LinphoneContactProvider {
	belle_sip_object_t base;
	LinphoneCore* lc;
};

#define LINPHONE_CONTACT_PROVIDER(obj) BELLE_SIP_CAST(obj,LinphoneContactProvider)

typedef LinphoneContactSearch* (*LinphoneContactProviderStartSearchMethod)( LinphoneContactProvider* thiz, const char* predicate, ContactSearchCallback cb, void* data );
typedef unsigned int           (*LinphoneContactProviderCancelSearchMethod)( LinphoneContactProvider* thiz, LinphoneContactSearch *request );

BELLE_SIP_DECLARE_CUSTOM_VPTR_BEGIN_NO_EXPORT(LinphoneContactProvider,belle_sip_object_t)
	const char* name; /*!< Name of the contact provider (LDAP, Google, ...) */

	/* pure virtual methods: inheriting objects must implement these */
	LinphoneContactProviderStartSearchMethod  begin_search;
	LinphoneContactProviderCancelSearchMethod cancel_search;
BELLE_SIP_DECLARE_CUSTOM_VPTR_END

/* LDAP search and contact providers */


#define LINPHONE_LDAP_CONTACT_SEARCH(obj) BELLE_SIP_CAST(obj,LinphoneLDAPContactSearch)
BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneLDAPContactSearch)

#define LINPHONE_LDAP_CONTACT_PROVIDER(obj) BELLE_SIP_CAST(obj,LinphoneLDAPContactProvider)

BELLE_SIP_DECLARE_CUSTOM_VPTR_BEGIN_NO_EXPORT(LinphoneLDAPContactProvider,LinphoneContactProvider)
BELLE_SIP_DECLARE_CUSTOM_VPTR_END


#endif // CONTACT_PROVIDERS_PRIV_H
