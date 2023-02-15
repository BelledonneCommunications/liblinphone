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

#include "linphone/api/c-magic-search-cbs.h"

#include "c-wrapper/c-wrapper.h"

// =============================================================================

struct _LinphoneMagicSearchCbs {
	belle_sip_object_t base;
	void *userData;
	LinphoneMagicSearchCbsSearchResultsReceivedCb search_results_received;
	LinphoneMagicSearchCbsLdapHaveMoreResultsCb ldap_have_more_results;
};

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneMagicSearchCbs);

BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneMagicSearchCbs);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneMagicSearchCbs,
                           belle_sip_object_t,
                           NULL, // destroy
                           NULL, // clone
                           NULL, // marshal
                           FALSE);

// =============================================================================

LinphoneMagicSearchCbs *linphone_magic_search_cbs_new(void) {
	return belle_sip_object_new(LinphoneMagicSearchCbs);
}

LinphoneMagicSearchCbs *linphone_magic_search_cbs_ref(LinphoneMagicSearchCbs *cbs) {
	belle_sip_object_ref(cbs);
	return cbs;
}

void linphone_magic_search_cbs_unref(LinphoneMagicSearchCbs *cbs) {
	belle_sip_object_unref(cbs);
}

void *linphone_magic_search_cbs_get_user_data(const LinphoneMagicSearchCbs *cbs) {
	return cbs->userData;
}

void linphone_magic_search_cbs_set_user_data(LinphoneMagicSearchCbs *cbs, void *ud) {
	cbs->userData = ud;
}

LinphoneMagicSearchCbsSearchResultsReceivedCb
linphone_magic_search_cbs_get_search_results_received(const LinphoneMagicSearchCbs *cbs) {
	return cbs->search_results_received;
}
void linphone_magic_search_cbs_set_search_results_received(LinphoneMagicSearchCbs *cbs,
                                                           LinphoneMagicSearchCbsSearchResultsReceivedCb cb) {
	cbs->search_results_received = cb;
}

LinphoneMagicSearchCbsLdapHaveMoreResultsCb
linphone_magic_search_cbs_get_ldap_have_more_results(const LinphoneMagicSearchCbs *cbs) {
	return cbs->ldap_have_more_results;
}
void linphone_magic_search_cbs_set_ldap_have_more_results(LinphoneMagicSearchCbs *cbs,
                                                          LinphoneMagicSearchCbsLdapHaveMoreResultsCb cb) {
	cbs->ldap_have_more_results = cb;
}
