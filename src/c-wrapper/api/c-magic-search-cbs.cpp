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
#include "search/magic-search.h"

using namespace LinphonePrivate;

// =============================================================================

LinphoneMagicSearchCbs *linphone_magic_search_cbs_new(void) {
	return MagicSearchCbs::createCObject();
}

LinphoneMagicSearchCbs *linphone_magic_search_cbs_ref(LinphoneMagicSearchCbs *cbs) {
	MagicSearchCbs::toCpp(cbs)->ref();
	return cbs;
}

void linphone_magic_search_cbs_unref(LinphoneMagicSearchCbs *cbs) {
	MagicSearchCbs::toCpp(cbs)->unref();
}

void *linphone_magic_search_cbs_get_user_data(const LinphoneMagicSearchCbs *cbs) {
	return MagicSearchCbs::toCpp(cbs)->getUserData();
}

void linphone_magic_search_cbs_set_user_data(LinphoneMagicSearchCbs *cbs, void *ud) {
	MagicSearchCbs::toCpp(cbs)->setUserData(ud);
}

LinphoneMagicSearchCbsSearchResultsReceivedCb
linphone_magic_search_cbs_get_search_results_received(const LinphoneMagicSearchCbs *cbs) {
	return MagicSearchCbs::toCpp(cbs)->getResultsReceived();
}
void linphone_magic_search_cbs_set_search_results_received(LinphoneMagicSearchCbs *cbs,
                                                           LinphoneMagicSearchCbsSearchResultsReceivedCb cb) {
	MagicSearchCbs::toCpp(cbs)->setResultsReceived(cb);
}

LinphoneMagicSearchCbsLdapHaveMoreResultsCb
linphone_magic_search_cbs_get_ldap_have_more_results(const LinphoneMagicSearchCbs *cbs) {
	return MagicSearchCbs::toCpp(cbs)->getLdapMoreResultsAvailable();
}
void linphone_magic_search_cbs_set_ldap_have_more_results(LinphoneMagicSearchCbs *cbs,
                                                          LinphoneMagicSearchCbsLdapHaveMoreResultsCb cb) {
	MagicSearchCbs::toCpp(cbs)->setLdapMoreResultsAvailable(cb);
}

LinphoneMagicSearchCbsMoreResultsAvailableCb
linphone_magic_search_cbs_get_more_results_available(const LinphoneMagicSearchCbs *cbs) {
	return MagicSearchCbs::toCpp(cbs)->getMoreResultsAvailable();
}

void linphone_magic_search_cbs_set_more_results_available(LinphoneMagicSearchCbs *cbs,
                                                          LinphoneMagicSearchCbsMoreResultsAvailableCb cb) {
	MagicSearchCbs::toCpp(cbs)->setMoreResultsAvailable(cb);
}