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

/*
 * That file declares functions that are used by automatic API wrapper generators. These
 * should not be used by C API users.
 */

#ifndef _WRAPPER_UTILS_H
#define _WRAPPER_UTILS_H

#include "linphone/defs.h"
#include "linphone/types.h"
#include <bctoolbox/list.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup wrapper
 * @{
 */

/**
 * @brief Gets the list of listener in the account.
 * @param account #LinphoneAccount object. @notnil
 * @return The list of #LinphoneAccountCbs. @maybenil
 * @donotwrap
 */
LINPHONE_PUBLIC const bctbx_list_t *linphone_account_get_callbacks_list(const LinphoneAccount *account);

/**
 * Sets the current LinphoneAccountCbs.
 * @param account #LinphoneAccount object. @notnil
 * @param cbs The #LinphoneAccountCbs object. @maybenil
 * @donotwrap
 */
LINPHONE_PUBLIC void linphone_account_set_current_callbacks(LinphoneAccount *account, LinphoneAccountCbs *cbs);

/**
 * @brief Gets the list of listener in the account.
 * @param request #LinphoneAccountManagerServicesRequest object. @notnil
 * @return The list of #LinphoneAccountManagerServicesRequestCbs. @maybenil
 * @donotwrap
 */
LINPHONE_PUBLIC const bctbx_list_t *
linphone_account_manager_services_request_get_callbacks_list(const LinphoneAccountManagerServicesRequest *request);

/**
 * Sets the current LinphoneAccountManagerServicesRequestCbs.
 * @param request #LinphoneAccountManagerServicesRequest object. @notnil
 * @param cbs The #LinphoneAccountManagerServicesRequestCbs object. @maybenil
 * @donotwrap
 */
LINPHONE_PUBLIC void
linphone_account_manager_services_set_current_callbacks(LinphoneAccountManagerServicesRequest *request,
                                                        LinphoneAccountManagerServicesRequestCbs *cbs);

/**
 * @brief Gets the list of listener in the core.
 * @param core The #LinphoneCore. @notnil
 * @return The list of #LinphoneCoreCbs. @maybenil
 * @donotwrap
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_core_get_callbacks_list(const LinphoneCore *core);

/**
 * @brief Gets the list of listener in the call.
 * @param call #LinphoneCall object. @notnil
 * @return The list of #LinphoneCallCbs. @maybenil
 * @donotwrap
 */
LINPHONE_PUBLIC const bctbx_list_t *linphone_call_get_callbacks_list(const LinphoneCall *call);

/**
 * @brief Gets the list of listener in the chat room.
 * @param chat_room #LinphoneChatRoom object. @notnil
 * @return The list of #LinphoneChatRoomCbs. @maybenil
 * @donotwrap
 */
LINPHONE_PUBLIC const bctbx_list_t *linphone_chat_room_get_callbacks_list(const LinphoneChatRoom *chat_room);

/**
 * Gets the list of listener in the conference.
 * @param[in] conference LinphoneConference object
 * @return The attached listeners. \bctbx_list{LinphoneConferenceCbs}
 * @donotwrap
 */
LINPHONE_PUBLIC const bctbx_list_t *linphone_conference_get_callbacks_list(const LinphoneConference *conference);

/**
 * Gets the list of listener in the participant device.
 * @param[in] participant_device #LinphoneParticipantDevice object
 * @return The attached listeners. \bctbx_list{LinphoneParticipantDeviceCbs}
 * @donotwrap
 */
LINPHONE_PUBLIC const bctbx_list_t *
linphone_participant_device_get_callbacks_list(const LinphoneParticipantDevice *participant_device);

/**
 * Sets the current LinphoneParticipantDeviceCbs.
 * @param participant_device #LinphoneParticipantDevice object. @notnil
 * @param cbs The #LinphoneParticipantDeviceCbs object. @maybenil
 * @donotwrap
 */
LINPHONE_PUBLIC void linphone_participant_device_set_current_callbacks(LinphoneParticipantDevice *participant_device,
                                                                       LinphoneParticipantDeviceCbs *cbs);

/**
 * Sets the current LinphoneChatRoomCbs.
 * @param chat_room LinphoneChatRoom object
 * @param cbs LinphoneChatRoomCbs object
 * @donotwrap
 */
LINPHONE_PUBLIC void linphone_chat_room_set_current_callbacks(LinphoneChatRoom *chat_room, LinphoneChatRoomCbs *cbs);

/**
 * @brief Gets the list of listener in the chat mesasge.
 * @param message #LinphoneChatMessage object.
 * @return The list of #LinphoneChatMessageCbs.
 * @donotwrap
 */
LINPHONE_PUBLIC const bctbx_list_t *linphone_chat_message_get_callbacks_list(const LinphoneChatMessage *message);

/**
 * Sets the current LinphoneChatMessageCbs.
 * @param message LinphoneChatMessage object
 * @param cbs LinphoneChatMessageCbs object
 * @donotwrap
 */
LINPHONE_PUBLIC void linphone_chat_message_set_current_callbacks(LinphoneChatMessage *message,
                                                                 LinphoneChatMessageCbs *cbs);

/**
 * @brief Gets the list of listener in the account.
 * @param conference_scheduler #LinphoneConferenceScheduler object. @notnil
 * @return The list of #LinphoneConferenceSchedulerCbs. @maybenil
 * @donotwrap
 */
LINPHONE_PUBLIC const bctbx_list_t *
linphone_conference_scheduler_get_callbacks_list(const LinphoneConferenceScheduler *conference_scheduler);

/**
 * Sets the current LinphoneConferenceSchedulerCbs.
 * @param conference_scheduler #LinphoneConferenceScheduler object. @notnil
 * @param cbs The #LinphoneConferenceSchedulerCbs object. @maybenil
 * @donotwrap
 */
LINPHONE_PUBLIC void
linphone_conference_scheduler_set_current_callbacks(LinphoneConferenceScheduler *conference_scheduler,
                                                    LinphoneConferenceSchedulerCbs *cbs);

/**
 * Accessor for the shared_ptr&lt;BelCard&gt; stored by a #LinphoneVcard
 * @param vcard a #LinphoneVcard
 * @return a shared_ptr<BelCard>
 * @donotwrap
 */
LINPHONE_PUBLIC void *linphone_vcard_get_belcard(LinphoneVcard *vcard);

/**
 * Allow multipart on a basic chat room
 * @donotwrap
 */
LINPHONE_PUBLIC void linphone_chat_room_allow_multipart(LinphoneChatRoom *room);

/**
 * Allow cpim on a basic chat room
 * @donotwrap
 */
LINPHONE_PUBLIC void linphone_chat_room_allow_cpim(LinphoneChatRoom *room);

/**
 * Gets the list of listeners from a #LinphoneMagicSearch.
 * @param magic_search #LinphoneMagicSearch object. @notnil
 * @return The list of listeners that have been added. \bctbx_list{LinphoneMagicSearchCbs} @maybenil
 * @donotwrap
 */
LINPHONE_PUBLIC const bctbx_list_t *linphone_magic_search_get_callbacks_list(const LinphoneMagicSearch *magic_search);

/* These 2 functions are needed for C# wrapper, in order to serialize string arrays */
LINPHONE_PUBLIC char *linphone_pointer_to_string(const void *ptr);
/* return value needs to be freed with bctbx_free(). */
LINPHONE_PUBLIC void *linphone_string_to_pointer(const char *ptr);

/* Since C# wrapper can only translate to UTF-8, force the whole process to assume
 * strings are UTF-8 encoded */
LINPHONE_PUBLIC LinphoneStatus linphone_force_utf8(void);

/************ */
/* DEPRECATED */
/* ********** */

/**
 * Send a message to peer member of this chat room.
 *
 * The state of the sending message will be notified via the callbacks defined in the #LinphoneChatMessageCbs object
 * that can be obtained by calling linphone_chat_message_get_callbacks().
 * @note Unlike linphone_chat_room_send_chat_message(), that function only takes a reference on the #LinphoneChatMessage
 * instead of totaly takes ownership on it. Thus, the #LinphoneChatMessage object must be released by the API user after
 * calling that function.
 *
 * @param chat_room A chat room. @notnil
 * @param message The message to send. @notnil
 * @deprecated 08/07/2020 Use linphone_chat_message_send() instead.
 * @donotwrap It doesn't says what the doc says it does
 */
LINPHONE_DEPRECATED LINPHONE_PUBLIC void linphone_chat_room_send_chat_message_2(LinphoneChatRoom *chat_room,
                                                                                LinphoneChatMessage *message);

/**
 * Resend a chat message if it is in the 'not delivered' state for whatever reason.
 * @note Unlike linphone_chat_message_resend(), that function only takes a reference on the #LinphoneChatMessage
 * instead of totaly takes ownership on it. Thus, the #LinphoneChatMessage object must be released by the API user after
 * calling that function.
 *
 * @param message #LinphoneChatMessage object @notnil
 * @deprecated 08/07/2020 Use linphone_chat_message_send instead.
 * @donotwrap It doesn't says what the doc says it does
 */
LINPHONE_DEPRECATED LINPHONE_PUBLIC void linphone_chat_message_resend_2(LinphoneChatMessage *message);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif // _WRAPPER_UTILS_H
