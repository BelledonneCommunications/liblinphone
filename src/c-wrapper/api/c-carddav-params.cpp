/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#include "bctoolbox/defs.h"

#include "c-wrapper/internal/c-tools.h"
#include "linphone/api/c-carddav-params.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"
#include "vcard/carddav-params.h"

// =============================================================================

using namespace LinphonePrivate;

LinphoneCardDavParams *linphone_card_dav_params_clone(const LinphoneCardDavParams *params) {
	return CardDavParams::toCpp(params)->clone()->toC();
}

LinphoneCardDavParams *linphone_card_dav_params_ref(LinphoneCardDavParams *params) {
	CardDavParams::toCpp(params)->ref();
	return params;
}

void linphone_card_dav_params_unref(LinphoneCardDavParams *params) {
	CardDavParams::toCpp(params)->unref();
}

// =============================================================================

bctbx_list_t *linphone_card_dav_params_get_user_input_fields(const LinphoneCardDavParams *params) {
	bctbx_list_t *results = nullptr;
	for (auto field : CardDavParams::toCpp(params)->getFieldsToFilterUserInputOn()) {
		results = bctbx_list_append(results, bctbx_strdup(L_STRING_TO_C(field)));
	}
	return results;
}

void linphone_card_dav_params_set_user_input_fields(LinphoneCardDavParams *params, bctbx_list_t *list) {
	std::list<std::string> fields;
	for (bctbx_list_t *it = list; it != nullptr; it = bctbx_list_next(it)) {
		fields.push_back(L_C_TO_STRING(static_cast<const char *>(bctbx_list_get_data(it))));
	}
	CardDavParams::toCpp(params)->setFieldsToFilterUserInputOn(fields);
}

bctbx_list_t *linphone_card_dav_params_get_domain_fields(const LinphoneCardDavParams *params) {
	bctbx_list_t *results = nullptr;
	for (auto field : CardDavParams::toCpp(params)->getFieldsToFilterDomainOn()) {
		results = bctbx_list_append(results, bctbx_strdup(L_STRING_TO_C(field)));
	}
	return results;
}

void linphone_card_dav_params_set_domain_fields(LinphoneCardDavParams *params, const bctbx_list_t *list) {
	std::list<std::string> fields;
	for (const bctbx_list_t *it = list; it != nullptr; it = bctbx_list_next(it)) {
		fields.push_back(L_C_TO_STRING(static_cast<const char *>(bctbx_list_get_data(it))));
	}
	CardDavParams::toCpp(params)->setFieldsToFilterDomainOn(fields);
}

bool_t linphone_card_dav_params_get_use_exact_match_policy(const LinphoneCardDavParams *params) {
	return CardDavParams::toCpp(params)->getUseExactMatchPolicy();
}

void linphone_card_dav_params_set_use_exact_match_policy(LinphoneCardDavParams *params, bool_t exact_match) {
	CardDavParams::toCpp(params)->setUseExactMatchPolicy(!!exact_match);
}