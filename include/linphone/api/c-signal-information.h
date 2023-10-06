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
#ifndef _L_C_SIGNAL_INFORMATION_H_
#define _L_C_SIGNAL_INFORMATION_H_

#include "linphone/api/c-types.h"
#include "linphone/enums/c-enums.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @addtogroup signalInformation
 * @{
 */

/**
 * Clone the given signalInformation
 * @param signalInformation The given signalInformation. @notnil
 * @return A new signalInformation with exactly same informations that param.
 * @notnil
 */
LINPHONE_PUBLIC LinphoneSignalInformation *
linphone_signal_information_clone(const LinphoneSignalInformation *signalInformation);
/**
 * Take a reference on a #LinphoneSignalInformation.
 * @param signalInformation The #LinphoneSignalInformation object. @notnil
 * @return the same #LinphoneSignalInformation object. @notnil
 */
LINPHONE_PUBLIC LinphoneSignalInformation *
linphone_signal_information_ref(LinphoneSignalInformation *signalInformation);

/**
 * Release a #LinphoneSignalInformation.
 * @param signalInformation The #LinphoneSignalInformation object. @notnil
 */
LINPHONE_PUBLIC void linphone_signal_information_unref(LinphoneSignalInformation *signalInformation);
/**
 * Get the value of the #LinphoneSignalInformation.
 * @param signalInformation The #LinphoneSignalInformation object. @notnil
 * @return A float containing the value.
 */
LINPHONE_PUBLIC float linphone_signal_information_get_strength(LinphoneSignalInformation *signalInformation);
/**
 * Set a new value to a #LinphoneSignalInformation.
 * @param signalInformation The #LinphoneSignalInformation object. @notnil
 * @param value a float containing the new value to set.
 */
LINPHONE_PUBLIC void linphone_signal_information_set_value(LinphoneSignalInformation *signalInformation, float value);
/**
 * Get the #LinphoneSignalType of the #LinphoneSignalInformation
 * @param signalInformation The #LinphoneSignalInformation object. @notnil
 * @return A #LinphoneSignalType.
 */
LINPHONE_PUBLIC LinphoneSignalType
linphone_signal_information_get_signal_type(LinphoneSignalInformation *signalInformation);
/**
 * Set a new #LinphoneSignalType to a #LinphoneSignalInformation
 * @param signalInformation The #LinphoneSignalInformation object. @notnil
 * @param type The new #LinphoneSignalType to set.
 */
LINPHONE_PUBLIC void linphone_signal_information_set_signal_type(LinphoneSignalInformation *signalInformation,
                                                                 LinphoneSignalType type);
/**
 * Get the #LinphoneSignalStrengthUnit value of the #LinphoneSignalInformation
 * @param signalInformation The #LinphoneSignalInformation object. @notnil
 * @return A #LinphoneSignalStrengthUnit.
 */
LINPHONE_PUBLIC LinphoneSignalStrengthUnit
linphone_signal_information_get_signal_unit(LinphoneSignalInformation *signalInformation);
/**
 * Set a new #LinphoneSignalStrengthUnit to a #LinphoneSignalInformation
 * @param signalInformation The #LinphoneSignalInformation object. @notnil
 * @param unit The new #LinphoneSignalStrengthUnit to set.
 */
LINPHONE_PUBLIC void linphone_signal_information_set_signal_unit(LinphoneSignalInformation *signalInformation,
                                                                 LinphoneSignalStrengthUnit unit);
/**
 * @}
 */
#ifdef __cplusplus
}
#endif

#endif //_L_C_SIGNAL_INFORMATION_H_
