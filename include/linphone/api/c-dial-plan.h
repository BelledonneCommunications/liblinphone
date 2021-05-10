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

#ifndef _L_C_DIAL_PLAN_H_
#define _L_C_DIAL_PLAN_H_

#include "linphone/api/c-types.h"

// =============================================================================

#ifdef __cplusplus
	extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup misc
 * @{
 */

/**
 * Increases the reference counter of #LinphoneDialPlan objects.
 * @param dial_plan the #LinphoneDialPlan object @notnil
 * @return the same #LinphoneDialPlan object @notnil
 */
LINPHONE_PUBLIC LinphoneDialPlan *linphone_dial_plan_ref(LinphoneDialPlan *dial_plan);

/**
 * Decreases the reference counter of #LinphoneDialPaln objects.
 * @param dial_plan the #LinphoneDialPlan object @notnil
 */
LINPHONE_PUBLIC void linphone_dial_plan_unref(LinphoneDialPlan *dial_plan);

/**
 * Returns the country name of the dialplan
 * @param dial_plan the #LinphoneDialPlan object @notnil
 * @return the country name
 */
LINPHONE_PUBLIC const char * linphone_dial_plan_get_country(const LinphoneDialPlan *dial_plan);

/**
 * Returns the iso country code of the dialplan
 * @param dial_plan the #LinphoneDialPlan object @notnil
 * @return the iso country code @notnil
 */
LINPHONE_PUBLIC const char * linphone_dial_plan_get_iso_country_code(const LinphoneDialPlan *dial_plan);

/**
 * Returns the country calling code of the dialplan
 * @param dial_plan the #LinphoneDialPlan object @notnil
 * @return the country calling code @notnil
 */
LINPHONE_PUBLIC const char * linphone_dial_plan_get_country_calling_code(const LinphoneDialPlan *dial_plan);

/**
 * Returns the national number length of the dialplan
 * @param dial_plan the #LinphoneDialPlan object @notnil
 * @return the national number length
 */
LINPHONE_PUBLIC int linphone_dial_plan_get_national_number_length(const LinphoneDialPlan *dial_plan);

/**
 * Returns the international call prefix of the dialplan
 * @param dial_plan the #LinphoneDialPlan object @notnil
 * @return the international call prefix @notnil
 */
LINPHONE_PUBLIC const char * linphone_dial_plan_get_international_call_prefix(const LinphoneDialPlan *dial_plan);

/**
 * Function to get  call country code from  ISO 3166-1 alpha-2 code, ex: FR returns 33
 * @param iso country code alpha2 @notnil
 * @return call country code or -1 if not found
 */
LINPHONE_PUBLIC	int linphone_dial_plan_lookup_ccc_from_iso(const char* iso);

/**
 * Function to get  call country code from  an e164 number, ex: +33952650121 will return 33
 * @param e164 phone number @notnil
 * @return call country code or -1 if not found
 */
LINPHONE_PUBLIC	int linphone_dial_plan_lookup_ccc_from_e164(const char* e164);

/**
 * Returns a list of all known dial plans
 * @return The list of all known dial plans. \bctbx_list{LinphoneDialPlan} @notnil @tobefreed
**/
LINPHONE_PUBLIC bctbx_list_t * linphone_dial_plan_get_all_list(void);

/**
 * Find best match for given CCC
 * @param ccc The country calling code @notnil
 * @return the matching dial plan, or a generic one if none found @notnil
**/
LINPHONE_PUBLIC const LinphoneDialPlan* linphone_dial_plan_by_ccc(const char *ccc);
/**
 * Find best match for given CCC
 * @param ccc the country calling code @notnil
 * @return the matching dial plan, or a generic one if none found @notnil
 **/
LINPHONE_PUBLIC const LinphoneDialPlan* linphone_dial_plan_by_ccc_as_int(int ccc);

/**
 * Return if given plan is generic
 * @param ccc the country calling code @notnil
 * @return TRUE if generic, FALSE otherwise
**/
LINPHONE_PUBLIC bool_t linphone_dial_plan_is_generic(const LinphoneDialPlan *ccc);

/************ */
/* DEPRECATED */
/* ********** */

/**
 * Return NULL-terminated array of all known dial plans
 * @deprecated 16/10/2017 use linphone_dial_plan_get_all_list instead, this method will always return NULL
 * @donotwrap
**/
LINPHONE_PUBLIC LINPHONE_DEPRECATED const LinphoneDialPlan* linphone_dial_plan_get_all(void);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_DIAL_PLAN_H_
