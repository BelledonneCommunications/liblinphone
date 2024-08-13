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

#ifndef LINPHONE_CONFERENCE_SCHEDULER_H
#define LINPHONE_CONFERENCE_SCHEDULER_H

#include "linphone/api/c-callbacks.h"
#include "linphone/api/c-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup conference
 * @{
 */

/**
 * Takes a reference on a #LinphoneConferenceScheduler.
 * @param conference_scheduler The #LinphoneConferenceScheduler object. @notnil
 * @return the same #LinphoneConferenceScheduler object. @notnil
 */
LINPHONE_PUBLIC LinphoneConferenceScheduler *
linphone_conference_scheduler_ref(LinphoneConferenceScheduler *conference_scheduler);

/**
 * Releases a #LinphoneConferenceScheduler.
 * @param conference_scheduler The #LinphoneConferenceScheduler object. @notnil
 */
LINPHONE_PUBLIC void linphone_conference_scheduler_unref(LinphoneConferenceScheduler *conference_scheduler);

/**
 * Gets the #LinphoneCore from a #LinphoneConferenceScheduler object.
 * @param conference_scheduler The #LinphoneConferenceScheduler object. @notnil
 * @return the #LinphoneCore object. @notnil
 */
LINPHONE_PUBLIC LinphoneCore *
linphone_conference_scheduler_get_core(const LinphoneConferenceScheduler *conference_scheduler);

/**
 * Set the #LinphoneAccount to use for the conference scheduler
 *
 * @param conference_scheduler The #LinphoneConferenceScheduler object. @notnil
 * @param account The #LinphoneAccount to use, or NULL if none has been selected. The LinphoneConferenceScheduler keeps
 *a reference to it and removes the previous one, if any. @maybenil
 **/
LINPHONE_PUBLIC void linphone_conference_scheduler_set_account(LinphoneConferenceScheduler *conference_scheduler,
                                                               LinphoneAccount *account);

/**
 * Get the #LinphoneAccount that is used for the conference scheduler
 *
 * @param conference_scheduler The #LinphoneConferenceScheduler object. @notnil
 * @return The selected #LinphoneAccount for the call, or NULL if none has been selected. @maybenil
 **/
LINPHONE_PUBLIC LinphoneAccount *
linphone_conference_scheduler_get_account(const LinphoneConferenceScheduler *conference_scheduler);

/**
 * Returns the #LinphoneConferenceInfo currently set in this scheduler.
 * @param conference_scheduler the #LinphoneConferenceScheduler object. @notnil
 * @return the currently configured #LinphoneConferenceInfo or NULL if none is set. @maybenil
 */
LINPHONE_PUBLIC const LinphoneConferenceInfo *
linphone_conference_scheduler_get_info(const LinphoneConferenceScheduler *conference_scheduler);

/**
 * Cancel the conference linked to the #LinphoneConferenceInfo provided as argument
 * @param conference_scheduler the #LinphoneConferenceScheduler object. @notnil
 * @param conference_info the #LinphoneConferenceInfo object to linked to the conference to cancel. @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_scheduler_cancel_conference(LinphoneConferenceScheduler *conference_scheduler,
                                                                     LinphoneConferenceInfo *conference_info);

/**
 * Sets the #LinphoneConferenceInfo to use to create/update the conference, which will be done right away.
 * @param conference_scheduler the #LinphoneConferenceScheduler object. @notnil
 * @param conference_info the #LinphoneConferenceInfo object to use to start creating/updating the client conference.
 * @maybenil
 */
LINPHONE_PUBLIC void linphone_conference_scheduler_set_info(LinphoneConferenceScheduler *conference_scheduler,
                                                            LinphoneConferenceInfo *conference_info);

/**
 * Sends an invitation to the scheduled conference to each participant by chat, using given chat rooms params to
 * use/create the chat room in which to send it.
 * @param conference_scheduler the #LinphoneConferenceScheduler object. @notnil
 * @param chat_room_params the #LinphoneChatRoomParams object to use to use/create the #LinphoneChatRoom that will be
 * used to send the invite. @notnil
 * @deprecated 28/08/2024 Use linphone_conference_scheduler_send_invitations_2() instead.
 */
LINPHONE_DEPRECATED LINPHONE_PUBLIC void
linphone_conference_scheduler_send_invitations(LinphoneConferenceScheduler *conference_scheduler,
                                               LinphoneChatRoomParams *chat_room_params);

/**
 * Sends an invitation to the scheduled conference to each participant by chat, using given conference params to
 * use/create the chat room in which to send it.
 * @param conference_scheduler the #LinphoneConferenceScheduler object. @notnil
 * @param conference_params the #LinphoneConferenceParams object to use to use/create the #LinphoneChatRoom that will be
 * used to send the invite. @notnil
 */
LINPHONE_PUBLIC void linphone_conference_scheduler_send_invitations_2(LinphoneConferenceScheduler *conference_scheduler,
                                                                      LinphoneConferenceParams *conference_params);

/**
 * Add a listener in order to be notified of #LinphoneConferenceScheduler events.
 * @param conference_scheduler The #LinphoneAccount object to monitor. @notnil
 * @param cbs A #LinphoneConferenceSchedulerCbs object holding the callbacks you need. @notnil
 */
LINPHONE_PUBLIC void linphone_conference_scheduler_add_callbacks(LinphoneConferenceScheduler *conference_scheduler,
                                                                 LinphoneConferenceSchedulerCbs *cbs);

/**
 * Remove a listener from a #LinphoneConferenceScheduler.
 * @param conference_scheduler The #LinphoneConferenceScheduler object. @notnil
 * @param cbs #LinphoneConferenceSchedulerCbs object to remove. @notnil
 */
LINPHONE_PUBLIC void linphone_conference_scheduler_remove_callbacks(LinphoneConferenceScheduler *conference_scheduler,
                                                                    LinphoneConferenceSchedulerCbs *cbs);

/**
 * Gets the current LinphoneConferenceSchedulerCbs.
 * This is meant only to be called from a callback to be able to get the user_data associated with the
 * #LinphoneConferenceSchedulerCbs that is calling the callback.
 * @param conference_scheduler The #LinphoneConferenceScheduler object. @notnil
 * @return The #LinphoneConferenceSchedulerCbs that has called the last callback. @maybenil
 */
LINPHONE_PUBLIC LinphoneConferenceSchedulerCbs *
linphone_conference_scheduler_get_current_callbacks(const LinphoneConferenceScheduler *conference_scheduler);

/**
 * Create a new conference scheduler callbacks object.
 * @return The #LinphoneConferenceSchedulerCbs object. @notnil
 **/
LinphoneConferenceSchedulerCbs *linphone_conference_scheduler_cbs_new(void);

/**
 * Acquire a reference to the conference scheduler callbacks object.
 * @param cbs The #LinphoneConferenceSchedulerCbs object. @notnil
 * @return The same conference scheduler callbacks object. @notnil
 **/
LINPHONE_PUBLIC LinphoneConferenceSchedulerCbs *
linphone_conference_scheduler_cbs_ref(LinphoneConferenceSchedulerCbs *cbs);

/**
 * Release reference to the conference scheduler callbacks object.
 * @param cbs The #LinphoneConferenceSchedulerCbs object. @notnil
 **/
LINPHONE_PUBLIC void linphone_conference_scheduler_cbs_unref(LinphoneConferenceSchedulerCbs *cbs);

/**
 * Retrieve the user pointer associated with the conference scheduler callbacks object.
 * @param cbs The #LinphoneConferenceSchedulerCbs object. @notnil
 * @return The user pointer associated with the conference scheduler callbacks object. @maybenil
 **/
LINPHONE_PUBLIC void *linphone_conference_scheduler_cbs_get_user_data(const LinphoneConferenceSchedulerCbs *cbs);

/**
 * Assign a user pointer to the conference scheduler callbacks object.
 * @param cbs The #LinphoneConferenceSchedulerCbs object. @notnil
 * @param user_data The user pointer to associate with the conference scheduler callbacks object. @maybenil
 **/
LINPHONE_PUBLIC void linphone_conference_scheduler_cbs_set_user_data(LinphoneConferenceSchedulerCbs *cbs,
                                                                     void *user_data);

/**
 * Get the state changed callback.
 * @param cbs #LinphoneConferenceSchedulerCbs object. @notnil
 * @return The current state changed callback.
 */
LINPHONE_PUBLIC LinphoneConferenceSchedulerCbsStateChangedCb
linphone_conference_scheduler_cbs_get_state_changed(const LinphoneConferenceSchedulerCbs *cbs);

/**
 * Set the state changed callback.
 * @param cbs #LinphoneConferenceSchedulerCbs object. @notnil
 * @param cb The state changed callback to be used.
 */
LINPHONE_PUBLIC void
linphone_conference_scheduler_cbs_set_state_changed(LinphoneConferenceSchedulerCbs *cbs,
                                                    LinphoneConferenceSchedulerCbsStateChangedCb cb);

/**
 * Get the invitations sent callback.
 * @param cbs #LinphoneConferenceSchedulerCbs object. @notnil
 * @return The current invitations sent callback.
 */
LINPHONE_PUBLIC LinphoneConferenceSchedulerCbsInvitationsSentCb
linphone_conference_scheduler_cbs_get_invitations_sent(const LinphoneConferenceSchedulerCbs *cbs);

/**
 * Set the invitations sent callback.
 * @param cbs #LinphoneConferenceSchedulerCbs object. @notnil
 * @param cb The invitations sent callback to be used.
 */
LINPHONE_PUBLIC void
linphone_conference_scheduler_cbs_set_invitations_sent(LinphoneConferenceSchedulerCbs *cbs,
                                                       LinphoneConferenceSchedulerCbsInvitationsSentCb cb);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_CONFERENCE_SCHEDULER_H */
