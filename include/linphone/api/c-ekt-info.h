/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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

#ifndef _L_C_EKT_INFO_H_
#define _L_C_EKT_INFO_H_

#include "linphone/api/c-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup ekt_api
 * @{
 **/

/**
 * Take a reference on a #LinphoneEktInfo.
 * @param linphone_ekt_info The #LinphoneEktInfo object. @notnil
 * @return the same #LinphoneEktInfo object.
 */
LINPHONE_PUBLIC LinphoneEktInfo *linphone_ekt_info_ref(LinphoneEktInfo *linphone_ekt_info);

/**
 * Release a #LinphoneEktInfo.
 * @param linphone_ekt_info The #LinphoneEktInfo object. @notnil
 */
LINPHONE_PUBLIC void linphone_ekt_info_unref(LinphoneEktInfo *linphone_ekt_info);

/**
 * Get from address.
 * @param linphone_ekt_info the #LinphoneEktInfo. @notnil
 * @return a #LinphoneAddress object. @maybenil
 **/
LINPHONE_PUBLIC const LinphoneAddress *linphone_ekt_info_get_from_address(const LinphoneEktInfo *linphone_ekt_info);

/**
 * Set from address.
 * @param linphone_ekt_info the #LinphoneEktInfo. @notnil
 * @param from the address to set. @maybenil
 **/
LINPHONE_PUBLIC void linphone_ekt_info_set_from_address(LinphoneEktInfo *linphone_ekt_info,
                                                        const LinphoneAddress *from);

/**
 * Get sSPI.
 * @param linphone_ekt_info the #LinphoneEktInfo. @notnil
 * @return a 16-bits unsigned int representing the sSPI.
 **/
LINPHONE_PUBLIC uint16_t linphone_ekt_info_get_sspi(const LinphoneEktInfo *linphone_ekt_info);

/**
 * Set sSPI.
 * @param linphone_ekt_info the #LinphoneEktInfo. @notnil
 * @param sspi the sspi to set.
 **/
LINPHONE_PUBLIC void linphone_ekt_info_set_sspi(LinphoneEktInfo *linphone_ekt_info, uint16_t sspi);

/**
 * Get cSPI.
 * @param linphone_ekt_info the #LinphoneEktInfo. @notnil
 * @return a #LinphoneAddress object. @maybenil @tobefreed
 **/
LINPHONE_PUBLIC LinphoneBuffer *linphone_ekt_info_get_cspi(const LinphoneEktInfo *linphone_ekt_info);

/**
 * Set cSPI.
 * @param linphone_ekt_info the #LinphoneEktInfo. @notnil
 * @param cspi the address to set. @maybenil
 **/
LINPHONE_PUBLIC void linphone_ekt_info_set_cspi(LinphoneEktInfo *linphone_ekt_info, const LinphoneBuffer *cspi);

/**
 * Get ciphers.
 * @param linphone_ekt_info the #LinphoneEktInfo. @notnil
 * @return a #LinphoneDictionary object. @maybenil
 */
LINPHONE_PUBLIC LinphoneDictionary *linphone_ekt_info_get_ciphers(const LinphoneEktInfo *linphone_ekt_info);

/**
 * Set ciphers.
 * @param linphone_ekt_info the #LinphoneEktInfo. @notnil
 * @param ciphers the ciphers to set. @maybenil
 */
LINPHONE_PUBLIC void linphone_ekt_info_set_ciphers(LinphoneEktInfo *linphone_ekt_info, LinphoneDictionary *ciphers);

/**
 * Add a cipher to the chipher map.
 * @param linphone_ekt_info the #LinphoneEktInfo. @notnil
 * @param to the address to set. @maybenil
 * @param cipher the cipher to set. @maybenil
 */
LINPHONE_PUBLIC void
linphone_ekt_info_add_cipher(LinphoneEktInfo *linphone_ekt_info, const char *to, const LinphoneBuffer *cipher);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif // _L_C_EKT_INFO_H_
