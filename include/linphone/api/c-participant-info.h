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

#ifndef LINPHONE_PARTICIPANT_INFO_H
#define LINPHONE_PARTICIPANT_INFO_H

#include "linphone/api/c-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup conference
 * @{
 */

/**
 * Create a new #LinphoneParticipantInfo object.
 * @return The newly created #LinphoneParticipantInfo object. @notnil
 */
LINPHONE_PUBLIC LinphoneParticipantInfo *linphone_participant_info_new(const LinphoneAddress *address);

/**
 * Take a reference on a #LinphoneParticipantInfo.
 * @param participant_info The #LinphoneParticipantInfo object. @notnil
 * @return the same #LinphoneParticipantInfo object. @notnil
 */
LINPHONE_PUBLIC LinphoneParticipantInfo *linphone_participant_info_ref(LinphoneParticipantInfo *participant_info);

/**
 * Clone an object #LinphoneParticipantInfo.
 * @param participant_info The #LinphoneParticipantInfo object. @notnil
 * @return the cloned #LinphoneParticipantInfo object. @notnil
 */
LINPHONE_PUBLIC LinphoneParticipantInfo *
linphone_participant_info_clone(const LinphoneParticipantInfo *participant_info);

/**
 * Release a #LinphoneParticipantInfo.
 * @param participant_info The #LinphoneParticipantInfo object. @notnil
 */
LINPHONE_PUBLIC void linphone_participant_info_unref(LinphoneParticipantInfo *participant_info);

/**
 * Get the address of the object #LinphoneParticipantInfo.
 * @param participant_info The #LinphoneParticipantInfo object. @notnil
 * @return the #LinphoneAddress of the #LinphoneParticipantInfo object. @notnil
 */
LINPHONE_PUBLIC const LinphoneAddress *
linphone_participant_info_get_address(const LinphoneParticipantInfo *participant_info);

/**
 * Set the role of the object #LinphoneParticipantInfo.
 * @param participant_info The #LinphoneParticipantInfo object. @notnil
 * @param role the #LinphoneParticipantRole of the #LinphoneParticipantInfo object. @notnil
 */
LINPHONE_PUBLIC void linphone_participant_info_set_role(LinphoneParticipantInfo *participant_info,
                                                        LinphoneParticipantRole role);

/**
 * Get the role of the object #LinphoneParticipantInfo.
 * @param participant_info The #LinphoneParticipantInfo object. @notnil
 * @return the #LinphoneParticipantRole of the #LinphoneParticipantInfo object. @notnil
 */
LINPHONE_PUBLIC LinphoneParticipantRole
linphone_participant_info_get_role(const LinphoneParticipantInfo *participant_info);

/**
 * Set the a custom parameter to object #LinphoneParticipantInfo.
 * @param participant_info The #LinphoneParticipantInfo object. @notnil
 * @param name the name of the parameter. @notnil
 * @param value the value of the parameter. @notnil
 */
LINPHONE_PUBLIC void
linphone_participant_info_add_parameter(LinphoneParticipantInfo *participant_info, const char *name, const char *value);

/**
 * Get the value of a custom parameter of the object #LinphoneParticipantInfo.
 * @param participant_info The #LinphoneParticipantInfo object. @notnil
 * @param name the name of the parameter. @notnil
 * @return value the value of the parameter. @notnil
 */
LINPHONE_PUBLIC const char *
linphone_participant_info_get_parameter_value(const LinphoneParticipantInfo *participant_info, const char *name);

/**
 * Find whether a #LinphoneParticipantInfo has a parameter
 * @param participant_info The #LinphoneParticipantInfo object. @notnil
 * @param name the name of the parameter. @notnil
 * @return TRUE if the parameter is present, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_participant_info_has_parameter(const LinphoneParticipantInfo *participant_info,
                                                               const char *name);

/**
 * Find the value of a custom parameter of the object #LinphoneParticipantInfo.
 * @param participant_info The #LinphoneParticipantInfo object. @notnil
 * @param name the name of the parameter. @notnil
 */
LINPHONE_PUBLIC void linphone_participant_info_remove_parameter(LinphoneParticipantInfo *participant_info,
                                                                const char *name);

/**
 * Get the CCMP uri of the object #LinphoneParticipantInfo.
 * @param participant_info The #LinphoneParticipantInfo object. @notnil
 * @return the CCMP uri of the #LinphoneParticipantInfo or NULL. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_participant_info_get_ccmp_uri(const LinphoneParticipantInfo *participant_info);
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif // LINPHONE_PARTICIPANT_INFO_H
