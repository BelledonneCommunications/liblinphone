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

#include "search/search-result.h"
#include "c-wrapper/c-wrapper.h"

L_DECLARE_C_CLONABLE_OBJECT_IMPL(SearchResult);

LinphoneSearchResult *linphone_search_result_ref (LinphoneSearchResult *searchResult) {
	belle_sip_object_ref(searchResult);
	return searchResult;
}

void linphone_search_result_unref (LinphoneSearchResult *searchResult) {
	belle_sip_object_unref(searchResult);
}

const LinphoneFriend *linphone_search_result_get_friend (const LinphoneSearchResult *searchResult) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(searchResult)->getFriend();
}

const LinphoneAddress *linphone_search_result_get_address (const LinphoneSearchResult *searchResult) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(searchResult)->getAddress();
}

const char *linphone_search_result_get_phone_number (const LinphoneSearchResult *searchResult) {
	return L_STRING_TO_C(L_GET_CPP_PTR_FROM_C_OBJECT(searchResult)->getPhoneNumber());
}

int linphone_search_result_get_capabilities (const LinphoneSearchResult *searchResult) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(searchResult)->getCapabilities();
}

bool_t linphone_search_result_has_capability (const LinphoneSearchResult *searchResult, const LinphoneFriendCapability capability) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(searchResult)->hasCapability(capability);
}

unsigned int linphone_search_result_get_weight (const LinphoneSearchResult *searchResult) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(searchResult)->getWeight();
}
