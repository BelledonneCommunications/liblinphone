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

#ifndef _L_C_CHAT_ROOM_H_
#define _L_C_CHAT_ROOM_H_

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
 * Acquire a reference to the chat room.
 * @param[in] chat_room The #LinphoneChatRoom object.
 * @return The same chat room.
**/
LINPHONE_PUBLIC LinphoneChatRoom *linphone_chat_room_ref(LinphoneChatRoom *chat_room);

/**
 * Release reference to the chat room.
 * @param[in] chat_room The #LinphoneChatRoom object.
**/
LINPHONE_PUBLIC void linphone_chat_room_unref(LinphoneChatRoom *chat_room);

/**
 * Retrieve the user pointer associated with the chat room.
 * @param chat_room the #LinphoneChatRoom object.
 * @return The user pointer associated with the chat room. @maybenil
**/
LINPHONE_PUBLIC void *linphone_chat_room_get_user_data(const LinphoneChatRoom *chat_room);

/**
 * Assign a user pointer to the chat room.
 * @param chat_room the #LinphoneChatRoom object.
 * @param[in] user_data The user pointer to associate with the chat room. @maybenil
**/
LINPHONE_PUBLIC void linphone_chat_room_set_user_data(LinphoneChatRoom *chat_room, void *user_data);

/**
 * Creates an empty message attached to the given chat room.
 * @param chat_room the #LinphoneChatRoom object.
 * @return a new #LinphoneChatMessage
 */
LINPHONE_PUBLIC LinphoneChatMessage *linphone_chat_room_create_empty_message (LinphoneChatRoom *chat_room);

/**
 * Creates a message attached to the given chat room with a plain text content filled with the given message.
 * @param chat_room the #LinphoneChatRoom object.
 * @param message text message, NULL if absent. @maybenil
 * @return a new #LinphoneChatMessage
 */
LINPHONE_PUBLIC LinphoneChatMessage* linphone_chat_room_create_message(LinphoneChatRoom *chat_room, const char* message);

/**
 * Creates a message attached to the given chat room.
 * @param chat_room the #LinphoneChatRoom object.
 * @param message text message, NULL if absent.
 * @param external_body_url the URL given in external body or NULL.
 * @param state the LinphoneChatMessage.State of the message.
 * @param time the time_t at which the message has been received/sent.
 * @param is_read TRUE if the message should be flagged as read, FALSE otherwise.
 * @param is_incoming TRUE if the message has been received, FALSE otherwise.
 * @return a new #LinphoneChatMessage
 * @deprecated 14/11/2017 Use #linphone_chat_room_chat_roomeate_message() instead.
 * @donotwrap
 */
LINPHONE_PUBLIC LinphoneChatMessage* linphone_chat_room_create_message_2(LinphoneChatRoom *chat_room, const char* message, const char* external_body_url, LinphoneChatMessageState state, time_t time, bool_t is_read, bool_t is_incoming);

 /**
 * Creates a message attached to the given chat room with a particular content.
 * Use #linphone_chat_room_send_message to initiate the transfer
 * @param chat_room the #LinphoneChatRoom object.
 * @param initial_content #LinphoneContent initial content.
 * @return a new #LinphoneChatMessage
 */
LINPHONE_PUBLIC LinphoneChatMessage* linphone_chat_room_create_file_transfer_message(LinphoneChatRoom *chat_room, LinphoneContent* initial_content);

 /**
 * Creates a forward message attached to the given chat room with a particular message.
 * @param chat_room the #LinphoneChatRoom object.
 * @param msg #LinphoneChatMessage message to be forwarded.
 * @return a new #LinphoneChatMessage
 */
LINPHONE_PUBLIC LinphoneChatMessage *linphone_chat_room_create_forward_message (LinphoneChatRoom *chat_room, LinphoneChatMessage *msg);

/**
 * Gets the peer address \link linphone_core_get_chat_room() associated to \endlink this #LinphoneChatRoom
 * @param chat_room #LinphoneChatRoom object
 * @return #LinphoneAddress peer address
 */
LINPHONE_PUBLIC const LinphoneAddress* linphone_chat_room_get_peer_address(LinphoneChatRoom *chat_room);

/**
 * Gets the local address \link linphone_core_get_chat_room() associated to \endlink this #LinphoneChatRoom
 * @param chat_room #LinphoneChatRoom object
 * @return #LinphoneAddress local address
 */
LINPHONE_PUBLIC const LinphoneAddress* linphone_chat_room_get_local_address(LinphoneChatRoom *chat_room);


/**
 * Send a message to peer member of this chat room.
 * @deprecated 13/10/2017 Use linphone_chat_message_send() instead.
 * @param chat_room #LinphoneChatRoom object
 * @param msg message to be sent
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED void linphone_chat_room_send_message(LinphoneChatRoom *chat_room, const char *msg);

/**
 * Send a message to peer member of this chat room.
 * @param[in] chat_room #LinphoneChatRoom object
 * @param[in] msg #LinphoneChatMessage object
 * The state of the message sending will be notified via the callbacks defined in the #LinphoneChatMessageCbs object that can be obtained
 * by calling linphone_chat_message_get_callbacks().
 * The #LinphoneChatMessage reference is transfered to the function and thus doesn't need to be unref'd by the application.
 * @deprecated 13/10/2017 Use linphone_chat_message_send() instead.
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED void linphone_chat_room_send_chat_message(LinphoneChatRoom *chat_room, LinphoneChatMessage *msg);

/**
 * Used to receive a chat message when using async mechanism with IM enchat_roomyption engine
 * @param[in] chat_room #LinphoneChatRoom object
 * @param[in] msg #LinphoneChatMessage object
 */
LINPHONE_PUBLIC void linphone_chat_room_receive_chat_message (LinphoneChatRoom *chat_room, LinphoneChatMessage *msg);

/**
 * Mark all messages of the conversation as read
 * @param[in] chat_room The #LinphoneChatRoom object corresponding to the conversation.
 */
LINPHONE_PUBLIC void linphone_chat_room_mark_as_read(LinphoneChatRoom *chat_room);

/**
 * Enable or disable the ephemeral message feature in the chat room. Works only for conference based chat room.
 * An ephemeral message will automatically disappear from the recipient's schat_roomeen after the message has been viewed.
 * @param[in] chat_room #LinphoneChatRoom object
 * @param[in] enable TRUE if the ephemeral message feature is enabled, FALSE otherwise.
 */
LINPHONE_PUBLIC void linphone_chat_room_enable_ephemeral (LinphoneChatRoom *chat_room, bool_t enable);

/**
 * Returns whether or not the ephemeral message feature is enabled in the chat room.
 * @param[in] chat_room The #LinphoneChatRoom object corresponding to the conversation
 * @return TRUE if ephemeral is enabled, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_chat_room_ephemeral_enabled (const LinphoneChatRoom *chat_room);

/**
 * Set lifetime (in seconds) for all new ephemral messages in the chat room.
 * After the message is read, it will be deleted after "time" seconds.
 * @see linphone_chat_room_ephemeral_enabled()
 * @param[in] chat_room #LinphoneChatRoom object
 * @param[in] time The ephemral lifetime, default 24h, 86400s
 */
LINPHONE_PUBLIC void linphone_chat_room_set_ephemeral_lifetime (LinphoneChatRoom *chat_room, long time);

/**
 * Get lifetime (in seconds) for all new ephemeral messages in the chat room.
 * After the message is read, it will be deleted after "time" seconds.
 * @see linphone_chat_room_ephemeral_enabled()
 * @param[in] chat_room #LinphoneChatRoom object
 * @return the ephemeral lifetime (in secoonds)
 */
LINPHONE_PUBLIC long linphone_chat_room_get_ephemeral_lifetime (const LinphoneChatRoom *chat_room);

/**
 * Uses linphone spec to check if all participants support ephemeral messages.
 * It doesn't prevent to send ephemeral messages in the room but those who don't support it
 * won't delete messages after lifetime has expired.
 * @see linphone_chat_room_ephemeral_enabled()
 * @param[in] chat_room #LinphoneChatRoom object
 * @return TRUE if all participants in the chat room support ephemeral messages, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_chat_room_ephemeral_supported_by_all_participants (const LinphoneChatRoom *chat_room);

/**
 * Delete a message from the chat room history.
 * @param[in] chat_room The #LinphoneChatRoom object corresponding to the conversation.
 * @param[in] message The #LinphoneChatMessage object to remove.
 */

LINPHONE_PUBLIC void linphone_chat_room_delete_message(LinphoneChatRoom *chat_room, LinphoneChatMessage *message);

/**
 * Delete all messages from the history
 * @param[in] chat_room The #LinphoneChatRoom object corresponding to the conversation.
 */
LINPHONE_PUBLIC void linphone_chat_room_delete_history(LinphoneChatRoom *chat_room);

/**
 * Gets the number of messages in a chat room.
 * @param[in] chat_room The #LinphoneChatRoom object corresponding to the conversation for which size has to be computed
 * @return the number of messages.
 */
LINPHONE_PUBLIC int linphone_chat_room_get_history_size(LinphoneChatRoom *chat_room);

/**
 * Returns whether or not a #LinphoneChatRoom has at least one #LinphoneChatMessage or not.
 * @param[in] chat_room The #LinphoneChatRoom object corresponding to the conversation
 * @return TRUE if there are no #LinphoneChatMessage, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_chat_room_is_empty (LinphoneChatRoom *chat_room);

/**
 * Gets nb_message most recent messages from chat_room chat room, sorted from oldest to most recent.
 * @param[in] chat_room The #LinphoneChatRoom object corresponding to the conversation for which messages should be retrieved
 * @param[in] nb_message Number of message to retrieve. 0 means everything.
 * @return \bctbx_list{LinphoneChatMessage} \onTheFlyList
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_chat_room_get_history (LinphoneChatRoom *chat_room, int nb_message);

/**
 * Gets the partial list of messages in the given range, sorted from oldest to most recent.
 * @param[in] chat_room The #LinphoneChatRoom object corresponding to the conversation for which messages should be retrieved
 * @param[in] begin The first message of the range to be retrieved. History most recent message has index 0.
 * @param[in] end The last message of the range to be retrieved. History oldest message has index of history size - 1 (use #linphone_chat_room_get_history_size to retrieve history size)
 * @return \bctbx_list{LinphoneChatMessage} \onTheFlyList
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_chat_room_get_history_range (LinphoneChatRoom *chat_room, int begin, int end);

/**
 * Gets nb_events most recent chat message events from chat_room chat room, sorted from oldest to most recent.
 * @param[in] chat_room The #LinphoneChatRoom object corresponding to the conversation for which events should be retrieved
 * @param[in] nb_events Number of events to retrieve. 0 means everything.
 * @return \bctbx_list{LinphoneEventLog} \onTheFlyList
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_chat_room_get_history_message_events (LinphoneChatRoom *chat_room, int nb_events);

/**
 * Gets the partial list of chat message events in the given range, sorted from oldest to most recent.
 * @param[in] chat_room The #LinphoneChatRoom object corresponding to the conversation for which events should be retrieved
 * @param[in] begin The first event of the range to be retrieved. History most recent event has index 0.
 * @param[in] end The last event of the range to be retrieved. History oldest event has index of history size - 1
 * @return \bctbx_list{LinphoneEventLog} \onTheFlyList
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_chat_room_get_history_range_message_events (LinphoneChatRoom *chat_room, int begin, int end);

/**
 * Gets nb_events most recent events from chat_room chat room, sorted from oldest to most recent.
 * @param[in] chat_room The #LinphoneChatRoom object corresponding to the conversation for which events should be retrieved
 * @param[in] nb_events Number of events to retrieve. 0 means everything.
 * @return \bctbx_list{LinphoneEventLog} \onTheFlyList
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_chat_room_get_history_events (LinphoneChatRoom *chat_room, int nb_events);

/**
 * Gets the partial list of events in the given range, sorted from oldest to most recent.
 * @param[in] chat_room The #LinphoneChatRoom object corresponding to the conversation for which events should be retrieved
 * @param[in] begin The first event of the range to be retrieved. History most recent event has index 0.
 * @param[in] end The last event of the range to be retrieved. History oldest event has index of history size - 1
 * @return \bctbx_list{LinphoneEventLog} \onTheFlyList
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_chat_room_get_history_range_events (LinphoneChatRoom *chat_room, int begin, int end);

/**
 * Gets the number of events in a chat room.
 * @param[in] chat_room The #LinphoneChatRoom object corresponding to the conversation for which size has to be computed
 * @return the number of events.
 */
LINPHONE_PUBLIC int linphone_chat_room_get_history_events_size(LinphoneChatRoom *chat_room);

/**
 * Gets the last chat message sent or received in this chat room
 * @param[in] chat_room The #LinphoneChatRoom object corresponding to the conversation for which last message should be retrieved
 * @return the latest #LinphoneChatMessage or NULL if no message. @maybenil
 */
LINPHONE_PUBLIC LinphoneChatMessage *linphone_chat_room_get_last_message_in_history(LinphoneChatRoom *chat_room);

/**
 * Gets the chat message sent or received in this chat room that matches the message_id
 * @param[in] chat_room The #LinphoneChatRoom object corresponding to the conversation for which the message should be retrieved
 * @param[in] message_id The id of the message to find
 * @return the #LinphoneChatMessage if found or NULL. @maybenil
 */
LINPHONE_PUBLIC LinphoneChatMessage * linphone_chat_room_find_message(LinphoneChatRoom *chat_room, const char *message_id);

/**
 * Notifies the destination of the chat message being composed that the user is typing a new message.
 * @param[in] chat_room The #LinphoneChatRoom object corresponding to the conversation for which a new message is being typed.
 */
LINPHONE_PUBLIC void linphone_chat_room_compose(LinphoneChatRoom *chat_room);

/**
 * Tells whether the remote is currently composing a message.
 * @param[in] chat_room The #LinphoneChatRoom object corresponding to the conversation.
 * @return TRUE if the remote is currently composing a message, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_chat_room_is_remote_composing(const LinphoneChatRoom *chat_room);

/**
 * Gets the number of unread messages in the chatroom.
 * @param[in] chat_room The #LinphoneChatRoom object corresponding to the conversation.
 * @return the number of unread messages.
 */
LINPHONE_PUBLIC int linphone_chat_room_get_unread_messages_count(LinphoneChatRoom *chat_room);

/**
 * Returns back pointer to #LinphoneCore object.
 * @param[in] chat_room the #LinphoneChatRoom object.
 * @return the #LinphoneCore object this chat room is attached to.
**/
LINPHONE_PUBLIC LinphoneCore* linphone_chat_room_get_core(const LinphoneChatRoom *chat_room);

/**
 * When realtime text is enabled #linphone_call_params_realtime_text_enabled, #LinphoneCoreIsComposingReceivedCb is call everytime a char is received from peer.
 * At the end of remote typing a regular #LinphoneChatMessage is received with committed data from #LinphoneCoreMessageReceivedCb.
 * @param[in] chat_room #LinphoneChatRoom object
 * @return  RFC 4103/T.140 char
 */
LINPHONE_PUBLIC uint32_t linphone_chat_room_get_char(const LinphoneChatRoom *chat_room);

/**
 * Returns wether lime is available for given peer or not. *
 * @return TRUE if zrtp secrets have already been shared and ready to use, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_chat_room_lime_available(LinphoneChatRoom *chat_room);

/**
 * Gets the current call associated to this chatroom if any
 * To commit a message, use #linphone_chat_room_send_message
 * @param[in] chat_room the #LinphoneChatRomm object
 * @return #LinphoneCall or NULL. @maybenil
 */
LINPHONE_PUBLIC LinphoneCall *linphone_chat_room_get_call(const LinphoneChatRoom *chat_room);

/**
 * Add a listener in order to be notified of #LinphoneChatRoom events. Once an event is received, registred #LinphoneChatRoomCbs are
 * invoked sequencially.
 * @param[in] call #LinphoneChatRoom object to monitor.
 * @param[in] cbs A #LinphoneChatRoomCbs object holding the callbacks you need. A reference is taken by the #LinphoneChatRoom until you invoke linphone_call_remove_callbacks().
 */
LINPHONE_PUBLIC void linphone_chat_room_add_callbacks(LinphoneChatRoom *chat_room, LinphoneChatRoomCbs *cbs);

/**
 * Remove a listener from a LinphoneChatRoom
 * @param[in] call LinphoneChatRoom object
 * @param[in] cbs LinphoneChatRoomCbs object to remove.
 */
LINPHONE_PUBLIC void linphone_chat_room_remove_callbacks(LinphoneChatRoom *chat_room, LinphoneChatRoomCbs *cbs);

/**
 * Gets the current LinphoneChatRoomCbs.
 * This is meant only to be called from a callback to be able to get the user_data associated with the LinphoneChatRoomCbs that is calling the callback.
 * @param[in] call LinphoneChatRoom object
 * @return The LinphoneChatRoomCbs that has called the last callback
 */
LINPHONE_PUBLIC LinphoneChatRoomCbs *linphone_chat_room_get_current_callbacks(const LinphoneChatRoom *chat_room);

/**
 * Get the state of the chat room.
 * @param[in] chat_room #LinphoneChatRoom object
 * @return The current #LinphoneChatRoomState of the chat room
 */
LINPHONE_PUBLIC LinphoneChatRoomState linphone_chat_room_get_state (const LinphoneChatRoom *chat_room);

/**
 * Return whether or not the chat room has been left.
 * @param[in] chat_room #LinphoneChatRoom object
 * @return TRUE if the chat room has been left, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_chat_room_has_been_left (const LinphoneChatRoom *chat_room);

/**
 * Return the last updated time for the chat room
 * @param[in] chat_room LinphoneChatRoom object
 * @return the last updated time
 */
LINPHONE_PUBLIC time_t linphone_chat_room_get_last_update_time(const LinphoneChatRoom *chat_room);

/**
 * Add a participant to a chat room. This may fail if this type of chat room does not handle participants.
 * Use linphone_chat_room_can_handle_participants() to know if this chat room handles participants.
 * @param[in] chat_room A #LinphoneChatRoom object
 * @param[in] addr The address of the participant to add to the chat room
 */
LINPHONE_PUBLIC void linphone_chat_room_add_participant (LinphoneChatRoom *chat_room, const LinphoneAddress *addr);

/**
 * Add several participants to a chat room at once. This may fail if this type of chat room does not handle participants.
 * Use linphone_chat_room_can_handle_participants() to know if this chat room handles participants.
 * @param[in] chat_room A #LinphoneChatRoom object
 * @param[in] addresses \bctbx_list{LinphoneAddress}
 * @return TRUE if everything is OK, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_chat_room_add_participants (LinphoneChatRoom *chat_room, const bctbx_list_t *addresses);

/**
 * Tells whether a chat room is able to handle participants.
 * @param[in] chat_room A #LinphoneChatRoom object
 * @return TRUE if the chat room can handle participants, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_chat_room_can_handle_participants (const LinphoneChatRoom *chat_room);

/**
 * Find a participant of a chat room from its address.
 * @param[in] chat_room A #LinphoneChatRoom object
 * @param[in] addr The #LinphoneAddress to search in the list of participants of the chat room
 * @return The participant if found, NULL otherwise. @maybenil
 */
LINPHONE_PUBLIC LinphoneParticipant *linphone_chat_room_find_participant (const LinphoneChatRoom *chat_room, const LinphoneAddress *addr);

/**
 * Get the capabilities of a chat room.
 * @param[in] chat_room A #LinphoneChatRoom object
 * @return The capabilities of the chat room (as a #LinphoneChatRoomCapabilitiesMask)
 */
LINPHONE_PUBLIC LinphoneChatRoomCapabilitiesMask linphone_chat_room_get_capabilities (const LinphoneChatRoom *chat_room);

/**
 * Check if a chat room has given capabilities.
 * @param[in] chat_room A #LinphoneChatRoom object
 * @param[in] mask a #LinphoneChatRoomCapabilitiesMask mask
 * @return TRUE if the mask matches, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_chat_room_has_capability(const LinphoneChatRoom *chat_room, int mask);

/**
 * Get the conference address of the chat room.
 * @param[in] chat_room A #LinphoneChatRoom object
 * @return The conference address of the chat room or NULL if this type of chat room is not conference based. @maybenil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_chat_room_get_conference_address (const LinphoneChatRoom *chat_room);

/**
 * Get the participant representing myself in the chat room.
 * @param[in] chat_room A #LinphoneChatRoom object
 * @return The participant representing myself in the conference.
 */
LINPHONE_PUBLIC LinphoneParticipant *linphone_chat_room_get_me (const LinphoneChatRoom *chat_room);

/**
 * Get the number of participants in the chat room (that is without ourselves).
 * @param[in] chat_room A #LinphoneChatRoom object
 * @return The number of participants in the chat room
 */
LINPHONE_PUBLIC int linphone_chat_room_get_nb_participants (const LinphoneChatRoom *chat_room);

/**
 * Get the list of participants of a chat room.
 * @param[in] chat_room A #LinphoneChatRoom object
 * @return A \bctbx_list{LinphoneParticipant} of the participants
 */
LINPHONE_PUBLIC bctbx_list_t * linphone_chat_room_get_participants (const LinphoneChatRoom *chat_room);

/**
 * Get the subject of a chat room.
 * @param[in] chat_room A #LinphoneChatRoom object
 * @return The subject of the chat room. @maybenil
 */
LINPHONE_PUBLIC const char * linphone_chat_room_get_subject (const LinphoneChatRoom *chat_room);

/**
 * Get the security level of a chat room.
 * @param[in] chat_room A #LinphoneChatRoom object
 * @return The current #LinphoneChatRoomSecurityLevel of the chat room
 */
LINPHONE_PUBLIC LinphoneChatRoomSecurityLevel linphone_chat_room_get_security_level (LinphoneChatRoom *chat_room);

/**
 * Leave a chat room.
 * @param[in] chat_room A #LinphoneChatRoom object
 */
LINPHONE_PUBLIC void linphone_chat_room_leave (LinphoneChatRoom *chat_room);

/**
 * Remove a participant of a chat room.
 * @param[in] chat_room A #LinphoneChatRoom object
 * @param[in] participant The participant to remove from the chat room
 */
LINPHONE_PUBLIC void linphone_chat_room_remove_participant (LinphoneChatRoom *chat_room, LinphoneParticipant *participant);

/**
 * Remove several participants of a chat room at once.
 * @param[in] chat_room A #LinphoneChatRoom object
 * @param[in] participants \bctbx_list{LinphoneParticipant}
 */
LINPHONE_PUBLIC void linphone_chat_room_remove_participants (LinphoneChatRoom *chat_room, const bctbx_list_t *participants);

/**
 * Change the admin status of a participant of a chat room (you need to be an admin yourself to do this).
 * @param[in] chat_room A #LinphoneChatRoom object
 * @param[in] participant The Participant for which to change the admin status
 * @param[in] isAdmin A boolean value telling whether the participant should now be an admin or not
 */
LINPHONE_PUBLIC void linphone_chat_room_set_participant_admin_status (LinphoneChatRoom *chat_room, LinphoneParticipant *participant, bool_t isAdmin);

/**
 * Set the subject of a chat room.
 * @param[in] chat_room A #LinphoneChatRoom object
 * @param[in] subject The new subject to set for the chat room
 */
LINPHONE_PUBLIC void linphone_chat_room_set_subject (LinphoneChatRoom *chat_room, const char *subject);

/**
 * Gets the list of participants that are currently composing
 * @param[in] chat_room A #LinphoneChatRoom object
 * @return \bctbx_list{LinphoneAddress} list of addresses that are in the is_composing state
 */
LINPHONE_PUBLIC const bctbx_list_t * linphone_chat_room_get_composing_addresses(LinphoneChatRoom *chat_room);

/**
 * Set the conference address of a group chat room. This function needs to be called from the
 * #LinphoneChatRoomCbsConferenceAddressGenerationCb callback and only there.
 * This function is meaningful only for server implementation of chatroom, and shall not by used by client applications.
 * @param[in] chat_room A #LinphoneChatRoom object
 * @param[in] confAddr The conference address to be used by the group chat room
 */
LINPHONE_PUBLIC void linphone_chat_room_set_conference_address (LinphoneChatRoom *chat_room, const LinphoneAddress *confAddr);

/**
 * Set the list of participant devices in the form of SIP URIs with GRUUs for a given participant.
 * This function is meaningful only for server implementation of chatroom, and shall not by used by client applications.
 * @param[in] chat_room A #LinphoneChatRoom object
 * @param[in] partAddr The participant address
 * @param[in] deviceIdentities \bctbx_list{LinphoneParticipantDeviceIdentity} list of the participant devices to be used by the group chat room
 */
LINPHONE_PUBLIC void linphone_chat_room_set_participant_devices(LinphoneChatRoom *chat_room, const LinphoneAddress *partAddr, const bctbx_list_t *deviceIdentities);


/**
 * Notify the chatroom that a participant device has just registered.
 * This function is meaningful only for server implementation of chatroom, and shall not by used by client applications.
 * @param[in] chat_room A #LinphoneChatRoom object
 * @param[in] partDevice list of the participant devices to be used by the group chat room
 */
LINPHONE_PUBLIC void linphone_chat_room_notify_participant_device_registration(LinphoneChatRoom *chat_room, const LinphoneAddress *participant_device);

/**
 * Returns back pointer to #LinphoneCore object.
 * @deprecated 15/09/2017 use linphone_chat_room_get_core()
 * @donotwrap
**/
LINPHONE_PUBLIC LINPHONE_DEPRECATED LinphoneCore* linphone_chat_room_get_lc(const LinphoneChatRoom *chat_room);

/**
 * Returns current parameters associated with the chat room.
 * This is typically the parameters passed at chat room chat_roomeation to linphone_core_chat_roomeate_chat_room() or some default
 * parameters if no #LinphoneChatRoomParams was explicitely passed during chat room chat_roomeation.
 * @param chat_room the #LinphoneChatRoom object
 * @return the current #LinphoneChatRoomParams parameters.
**/
LINPHONE_PUBLIC const LinphoneChatRoomParams *linphone_chat_room_get_current_params(const LinphoneChatRoom *chat_room);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_CHAT_ROOM_H_
