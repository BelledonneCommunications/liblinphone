/*
 * c-dial-plan.cpp
 * Copyright (C) 2010-2018 Belledonne Communications SARL
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "linphone/api/c-dial-plan.h"
#include "linphone/wrapper_utils.h"

#include "c-wrapper/c-wrapper.h"
#include "dial-plan/dial-plan.h"

// =============================================================================

using namespace std;
using namespace LinphonePrivate;

LinphoneDialPlan *linphone_dial_plan_ref (LinphoneDialPlan *dp) {
	DialPlan::toCpp(dp)->ref();
	return dp;
}

void linphone_dial_plan_unref (LinphoneDialPlan *dp) {
	DialPlan::toCpp(dp)->unref();
}

const char *linphone_dial_plan_get_country (const LinphoneDialPlan *dp) {
	return L_STRING_TO_C(DialPlan::toCpp(dp)->getCountry());
}

const char *linphone_dial_plan_get_iso_country_code (const LinphoneDialPlan *dp) {
	return L_STRING_TO_C(DialPlan::toCpp(dp)->getIsoCountryCode());
}

const char *linphone_dial_plan_get_country_calling_code (const LinphoneDialPlan *dp) {
	return L_STRING_TO_C(DialPlan::toCpp(dp)->getCountryCallingCode());
}

int linphone_dial_plan_get_national_number_length (const LinphoneDialPlan *dp) {
	return DialPlan::toCpp(dp)->getNationalNumberLength();
}

const char *linphone_dial_plan_get_international_call_prefix (const LinphoneDialPlan *dp) {
	return L_STRING_TO_C(DialPlan::toCpp(dp)->getInternationalCallPrefix());
}

int linphone_dial_plan_lookup_ccc_from_e164 (const char *e164) {
	return DialPlan::lookupCccFromE164(L_C_TO_STRING(e164));
}

int linphone_dial_plan_lookup_ccc_from_iso (const char *iso) {
	return DialPlan::lookupCccFromIso(L_C_TO_STRING(iso));
}

const LinphoneDialPlan *linphone_dial_plan_by_ccc_as_int (int ccc) {
	shared_ptr<DialPlan> dp = DialPlan::findByCcc(ccc);
	return dp->toC();
}

const LinphoneDialPlan *linphone_dial_plan_by_ccc (const char *ccc) {
	shared_ptr<DialPlan> dp = DialPlan::findByCcc(L_C_TO_STRING(ccc));
	return dp->toC();
}

bctbx_list_t *linphone_dial_plan_get_all_list () {
	return DialPlan::getCListFromCppList(DialPlan::getAllDialPlans());
}

bool_t linphone_dial_plan_is_generic (const LinphoneDialPlan *ccc) {
	return DialPlan::toCpp(ccc)->isGeneric();
}
