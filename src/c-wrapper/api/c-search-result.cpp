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

#include "linphone/api/c-search-result.h"
#include "c-wrapper/c-wrapper.h"
#include "friend/friend.h"
#include "search/search-result.h"

using namespace LinphonePrivate;

LinphoneSearchResult *linphone_search_result_ref(LinphoneSearchResult *searchResult) {
	belle_sip_object_ref(searchResult);
	return searchResult;
}

void linphone_search_result_unref(LinphoneSearchResult *searchResult) {
	belle_sip_object_unref(searchResult);
}

LinphoneFriend *linphone_search_result_get_friend(const LinphoneSearchResult *searchResult) {
	auto lf = SearchResult::toCpp(searchResult)->getFriend();
	return lf ? lf->toC() : nullptr;
}

const LinphoneAddress *linphone_search_result_get_address(const LinphoneSearchResult *searchResult) {
	auto cppAddr = SearchResult::toCpp(searchResult)->getAddress();
	return cppAddr ? cppAddr->toC() : nullptr;
}

const char *linphone_search_result_get_phone_number(const LinphoneSearchResult *searchResult) {
	return L_STRING_TO_C(SearchResult::toCpp(searchResult)->getPhoneNumber());
}

int linphone_search_result_get_capabilities(const LinphoneSearchResult *searchResult) {
	return SearchResult::toCpp(searchResult)->getCapabilities();
}

bool_t linphone_search_result_has_capability(const LinphoneSearchResult *searchResult,
                                             const LinphoneFriendCapability capability) {
	return SearchResult::toCpp(searchResult)->hasCapability(capability);
}

unsigned int linphone_search_result_get_weight(const LinphoneSearchResult *searchResult) {
	return SearchResult::toCpp(searchResult)->getWeight();
}

int linphone_search_result_get_source_flags(const LinphoneSearchResult *searchResult) {
	return SearchResult::toCpp(searchResult)->getSourceFlags();
}

bool_t linphone_search_result_has_source_flag(const LinphoneSearchResult *searchResult,
                                              const LinphoneMagicSearchSource source) {
	return SearchResult::toCpp(searchResult)->hasSourceFlag(source);
}
