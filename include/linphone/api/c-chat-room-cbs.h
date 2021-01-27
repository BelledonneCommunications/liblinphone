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

#ifndef _L_C_CHAT_ROOM_CBS_H_
#define _L_C_CHAT_ROOM_CBS_H_

#include "linphone/api/c-callbacks.h"
#include "linphone/api/c-types.h"

// =============================================================================

#ifdef __cplusplus
	extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup chatroom
 * @{
 */

/**
 * Acquire a reference to the chat room callbacks object.
 * @param cbs The #LinphoneChatRoomCbs object @notnil
 * @return The same #LinphoneChatRoomCbs object @notnil
**/
LINPHONE_PUBLIC LinphoneChatRoomCbs * linphone_chat_room_cbs_ref (LinphoneChatRoomCbs *cbs);

/**
 * Release reference to the chat room callbacks object.
 * @param cbs The chat room callbacks object @notnil
**/
LINPHONE_PUBLIC void linphone_chat_room_cbs_unref (LinphoneChatRoomCbs *cbs);

/**
 * Retrieve the user pointer associated with the chat room callbacks object.
 * @param cbs The chat room callbacks object @notnil
 * @return The user pointer associated with the chat room callbacks object. @maybenil
**/
LINPHONE_PUBLIC void * linphone_chat_room_cbs_get_user_data (const LinphoneChatRoomCbs *cbs);

/**
 * Assign a user pointer to the chat room callbacks object.
 * @param cbs The chat room callbacks object @notnil
 * @param user_data The user pointer to associate with the chat room callbacks object. @maybenil
**/
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_user_data (LinphoneChatRoomCbs *cbs, void *user_data);

/**
 * Get the is-composing received callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @return The current is-composing received callback.
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsIsComposingReceivedCb linphone_chat_room_cbs_get_is_composing_received (const LinphoneChatRoomCbs *cbs);

/**
 * Set the is-composing received callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @param cb The is-composing received callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_is_composing_received (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsIsComposingReceivedCb cb);

/**
 * Get the message received callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @return The current message received callback.
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsMessageReceivedCb linphone_chat_room_cbs_get_message_received (const LinphoneChatRoomCbs *cbs);

/**
 * Set the message received callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @param cb The message received callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_message_received (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsMessageReceivedCb cb);

/**
 * Get the new event log callback.
 * This callback will be called before every other #LinphoneEventLog related callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @return The current event log created callback.
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsNewEventCb linphone_chat_room_cbs_get_new_event (const LinphoneChatRoomCbs *cbs);

/**
 * Set the new event log callback.
 * This callback will be called before every other #LinphoneEventLog related callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @param cb The event log created callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_new_event (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsNewEventCb cb);

/**
 * Get the chat message received callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @return The current chat message received callback.
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsChatMessageReceivedCb linphone_chat_room_cbs_get_chat_message_received (const LinphoneChatRoomCbs *cbs);

/**
 * Set the chat message received callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @param cb The chat message received callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_chat_message_received (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsChatMessageReceivedCb cb);

/**
 * Get the chat message sending callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @return The current chat message being sent callback.
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsChatMessageSendingCb linphone_chat_room_cbs_get_chat_message_sending (const LinphoneChatRoomCbs *cbs);

/**
 * Set the chat message sending callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @param cb The chat message being sent callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_chat_message_sending (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsChatMessageSendingCb cb);

/**
 * Get the chat message sent callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @return The current chat message sent callback.
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsChatMessageSentCb linphone_chat_room_cbs_get_chat_message_sent (const LinphoneChatRoomCbs *cbs);

/**
 * Set the chat message sent callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @param cb The chat message sent callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_chat_message_sent (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsChatMessageSentCb cb);

/**
 * Get the participant added callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @return The current participant added callback.
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsParticipantAddedCb linphone_chat_room_cbs_get_participant_added (const LinphoneChatRoomCbs *cbs);

/**
 * Set the participant added callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @param cb The participant added callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_participant_added (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsParticipantAddedCb cb);

/**
 * Get the participant removed callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @return The current participant removed callback.
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsParticipantRemovedCb linphone_chat_room_cbs_get_participant_removed (const LinphoneChatRoomCbs *cbs);

/**
 * Set the participant removed callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @param cb The participant removed callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_participant_removed (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsParticipantRemovedCb cb);

/**
 * Get the participant admin status changed callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @return The current participant admin status changed callback.
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsParticipantAdminStatusChangedCb linphone_chat_room_cbs_get_participant_admin_status_changed (const LinphoneChatRoomCbs *cbs);

/**
 * Set the participant admin status changed callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @param cb The participant admin status changed callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_participant_admin_status_changed (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsParticipantAdminStatusChangedCb cb);

/**
 * Get the state changed callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @return The current state changed callback.
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsStateChangedCb linphone_chat_room_cbs_get_state_changed (const LinphoneChatRoomCbs *cbs);

/**
 * Set the state changed callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @param cb The state changed callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_state_changed (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsStateChangedCb cb);

/**
 * Get the security event callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @return The security event callback to be used.
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsSecurityEventCb linphone_chat_room_cbs_get_security_event (const LinphoneChatRoomCbs *cbs);

/**
 * Set the security event callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @param cb The security event callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_security_event (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsSecurityEventCb cb);

/**
 * Get the subject changed callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @return The current subject changed callback.
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsSubjectChangedCb linphone_chat_room_cbs_get_subject_changed (const LinphoneChatRoomCbs *cbs);

/**
 * Set the subject changed callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @param cb The subject changed callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_subject_changed (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsSubjectChangedCb cb);

/**
 * Get the undecryptable message received callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @return The current undecryptable message received callback.
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsUndecryptableMessageReceivedCb linphone_chat_room_cbs_get_undecryptable_message_received (const LinphoneChatRoomCbs *cbs);

/**
 * Set the undecryptable message received callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @param cb The undecryptable message received callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_undecryptable_message_received (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsUndecryptableMessageReceivedCb cb);

/**
 * Get the participant device added callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @return The current participant device added callback.
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsParticipantDeviceAddedCb linphone_chat_room_cbs_get_participant_device_added (const LinphoneChatRoomCbs *cbs);

/**
 * Set the participant device added callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @param cb The participant device added callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_participant_device_added (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsParticipantDeviceAddedCb cb);

/**
 * Get the participant device removed callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @return The current participant device removed callback.
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsParticipantDeviceRemovedCb linphone_chat_room_cbs_get_participant_device_removed (const LinphoneChatRoomCbs *cbs);

/**
 * Set the participant device removed callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @param cb The participant device removed callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_participant_device_removed (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsParticipantDeviceRemovedCb cb);

/**
 * Get the conference joined callback.
 * @param cbs LinphoneChatRoomCbs object. @notnil
 * @return The current conference joined callback.
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsConferenceJoinedCb linphone_chat_room_cbs_get_conference_joined (const LinphoneChatRoomCbs *cbs);

/**
 * Set the conference joined callback.
 * @param cbs LinphoneChatRoomCbs object. @notnil
 * @param cb The conference joined callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_conference_joined (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsConferenceJoinedCb cb);

/**
 * Get the conference left callback.
 * @param cbs LinphoneChatRoomCbs object. @notnil
 * @return The current conference left callback.
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsConferenceLeftCb linphone_chat_room_cbs_get_conference_left (const LinphoneChatRoomCbs *cbs);

/**
 * Set the conference left callback.
 * @param cbs LinphoneChatRoomCbs object. @notnil
 * @param cb The conference left callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_conference_left (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsConferenceLeftCb cb);

/**
 * Get the ephemeral event callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @return The ephemeral event callback to be used.
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsEphemeralEventCb linphone_chat_room_cbs_get_ephemeral_event (const LinphoneChatRoomCbs *cbs);

/**
 * Set the ephemeral event callback.
 * @param cbs #LinphoneChatRoomCbs object. @notnil
 * @param cb The ephemeral event callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_ephemeral_event (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsEphemeralEventCb cb);

/**
 * Get the current "ephemeral message timer started" callback. This callback is called when a message deletion timer starts (the message has been viewed).
 * @param cbs LinphoneChatRoomCbs object. @notnil
 * @return The current ephemeral message "timer started" callback.
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsEphemeralMessageTimerStartedCb linphone_chat_room_cbs_get_ephemeral_message_timer_started (const LinphoneChatRoomCbs *cbs);

/**
 * Set the ephemeral message timer started callback. This callback will be used when new message deletion timer starts (the message has been viewed).
 * @param cbs LinphoneChatRoomCbs object. @notnil
 * @param cb The ephemeral message timer started callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_ephemeral_message_timer_started (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsEphemeralMessageTimerStartedCb cb);

/**
 * Get the ephemeral message deleted callback. This callback is used when a message deletion timer runs out (message is deleted).
 * @param cbs LinphoneChatRoomCbs object. @notnil
 * @return The current ephemeral message deleted callback.
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsEphemeralMessageDeletedCb linphone_chat_room_cbs_get_ephemeral_message_deleted (const LinphoneChatRoomCbs *cbs);

/**
 * Set the ephemeral message deleted callback. This callback is used when new message deletion timer runs out (message is deleted).
 * @param cbs LinphoneChatRoomCbs object. @notnil
 * @param cb The ephemeral message deleted callback to be used.
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_ephemeral_message_deleted (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsEphemeralMessageDeletedCb cb);

/**
 * Get the conference address generation callback.
 * @param cbs #LinphoneChatRoomCbs object @notnil
 * @return The current conference address generation callback
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsConferenceAddressGenerationCb linphone_chat_room_cbs_get_conference_address_generation (const LinphoneChatRoomCbs *cbs);

/**
 * Set the conference address generation callback.
 * @param cbs #LinphoneChatRoomCbs object @notnil
 * @param cb The conference address generation callback to be used
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_conference_address_generation (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsConferenceAddressGenerationCb cb);

/**
 * Get the participant registration subscription callback.
 * @param cbs LinphoneChatRoomCbs object @notnil
 * @return The participant registration subscription callback
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsParticipantRegistrationSubscriptionRequestedCb linphone_chat_room_cbs_get_participant_registration_subscription_requested (const LinphoneChatRoomCbs *cbs);

/**
 * Set the participant registration subscription callback.
 * @param cbs LinphoneChatRoomCbs object @notnil
 * @param cb The participant registration subscription callback to be used
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_participant_registration_subscription_requested (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsParticipantRegistrationSubscriptionRequestedCb cb);

/**
 * Get the participant registration unsubscription callback.
 * @param cbs LinphoneChatRoomCbs object @notnil
 * @return The participant registration unsubscription callback
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsParticipantRegistrationUnsubscriptionRequestedCb linphone_chat_room_cbs_get_participant_registration_unsubscription_requested (const LinphoneChatRoomCbs *cbs);

/**
 * Set the participant registration unsubscription callback.
 * @param cbs LinphoneChatRoomCbs object @notnil
 * @param cb The participant registration unsubscription callback to be used
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_participant_registration_unsubscription_requested (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsParticipantRegistrationUnsubscriptionRequestedCb cb);

/**
 * Get the message should be stored callback.
 * @param cbs LinphoneChatRoomCbs object @notnil
 * @return The message should be stored callback
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsShouldChatMessageBeStoredCb linphone_chat_room_cbs_get_chat_message_should_be_stored (LinphoneChatRoomCbs *cbs);

/**
 * Set the message should be stored callback.
 * @param cbs LinphoneChatRoomCbs object @notnil
 * @param cb The message should be stored callback to be used
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_chat_message_should_be_stored (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsShouldChatMessageBeStoredCb cb);

/**
 * Get the message's participant state changed callback.
 * @param cbs LinphoneChatRoomCbs object @notnil
 * @return The message's participant state changed callback callback
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsChatMessageParticipantImdnStateChangedCb linphone_chat_room_cbs_get_chat_message_participant_imdn_state_changed (LinphoneChatRoomCbs *cbs);

/**
 * Set the message's participant state changed callback callback.
 * @param cbs LinphoneChatRoomCbs object @notnil
 * @param cb The message's participant state changed callback to be used
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_chat_message_participant_imdn_state_changed (LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsChatMessageParticipantImdnStateChangedCb cb);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_CHAT_ROOM_CBS_H_
