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

#ifndef _L_C_CONFERENCE_H_
#define _L_C_CONFERENCE_H_

#include "linphone/api/c-types.h"

// =============================================================================

#ifdef __cplusplus
	extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup conferencing
 * @{
 */

/**
 * Add a listener in order to be notified of #LinphoneConference events. Once an event is received, registred #LinphoneConferenceCbs are
 * invoked sequencially.
 * @param conference #LinphoneConference object. @notnil
 * @param cbs A #LinphoneConferenceCbs object holding the callbacks you need. A reference is taken by the #LinphoneConference until you invoke linphone_conference_remove_callbacks(). @notnil

 */
LINPHONE_PUBLIC void linphone_conference_add_callbacks(LinphoneConference *conference, LinphoneConferenceCbs *cbs);

/**
 * Remove a listener from a LinphoneConference
 * @param conference #LinphoneConference object. @notnil
 * @param cbs #LinphoneConferenceCbs object to remove. @notnil
 */
LINPHONE_PUBLIC void linphone_conference_remove_callbacks(LinphoneConference *conference, LinphoneConferenceCbs *cbs);

/**
 * Gets the current LinphoneConferenceCbs.
 * This is meant only to be called from a callback to be able to get the user_data associated with the LinphoneConferenceCbs that is calling the callback.
 * @param conference #LinphoneConference object. @notnil
 * @return The LinphoneConferenceCbs that has called the last callback. @notnil
 */
LINPHONE_PUBLIC LinphoneConferenceCbs *linphone_conference_get_current_callbacks(const LinphoneConference *conference);

/**
 * Returns core for a #LinphoneConference
 * @param conference #LinphoneConference object. @notnil
 * @return back pointer to #LinphoneCore object. @notnil
 * Returns back pointer to #LinphoneCore object.
**/
LINPHONE_PUBLIC LinphoneCore* linphone_conference_get_core(const LinphoneConference *conference);

/**
 * Get the conference address of the conference.
 * @param conference A #LinphoneConference object. @notnil
 * @return The conference address of the conference or NULL if it is unknown. @maybenil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_conference_get_conference_address (const LinphoneConference *conference);
LINPHONE_PUBLIC char *linphone_conference_get_conference_address_as_string (const LinphoneConference *conference);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_CONFERENCE_H_
