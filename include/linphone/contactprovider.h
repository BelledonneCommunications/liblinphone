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

#ifndef LINPHONE_CONTACTPROVIDER_H_
#define LINPHONE_CONTACTPROVIDER_H_


#include "linphone/core.h"


#ifdef __cplusplus
extern "C" {
#endif

/* LinphoneContactSearchRequest */

void linphone_contact_search_init(LinphoneContactSearch *obj, const char *predicate, ContactSearchCallback cb, void *cb_data);
LinphoneContactSearchID linphone_contact_search_get_id(LinphoneContactSearch *obj);
const char* linphone_contact_search_get_predicate(LinphoneContactSearch *obj);
void linphone_contact_search_invoke_cb(LinphoneContactSearch *req, MSList *friends);
LINPHONE_PUBLIC LinphoneContactSearch* linphone_contact_search_ref(void *obj);
void linphone_contact_search_unref(void *obj);
LinphoneContactSearch* linphone_contact_search_cast(void *obj);


/* LinphoneContactProvider */

void linphone_contact_provider_init(LinphoneContactProvider *obj, LinphoneCore *lc);
LinphoneCore * linphone_contact_provider_get_core(LinphoneContactProvider *obj);
const char * linphone_contact_provider_get_name(LinphoneContactProvider *obj);
LinphoneContactProvider* linphone_contact_provider_ref(void *obj);
LINPHONE_PUBLIC void linphone_contact_provider_unref(void *obj);
LINPHONE_PUBLIC LinphoneContactProvider * linphone_contact_provider_cast(void *obj);

LINPHONE_PUBLIC LinphoneContactSearch * linphone_contact_provider_begin_search(LinphoneContactProvider *obj, const char *predicate, ContactSearchCallback cb, void *data);
unsigned int linphone_contact_provider_cancel_search(LinphoneContactProvider *obj, LinphoneContactSearch *request);

#ifdef __cplusplus
}
#endif


#endif /* LINPHONE_CONTACTPROVIDER_H_ */
