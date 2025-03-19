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
 * @param chat_room The #LinphoneChatRoom object. @notnil
 * @return The same #LinphoneChatRoom object. @notnil
 **/
LINPHONE_PUBLIC LinphoneChatRoom *linphone_chat_room_ref(LinphoneChatRoom *chat_room);

/**
 * Release reference to the chat room.
 * @param chat_room The #LinphoneChatRoom object. @notnil
 **/
LINPHONE_PUBLIC void linphone_chat_room_unref(LinphoneChatRoom *chat_room);

/**
 * Retrieve the user pointer associated with the chat room.
 * @param chat_room the #LinphoneChatRoom object. @notnil
 * @return The user pointer associated with the chat room. @maybenil
 **/
LINPHONE_PUBLIC void *linphone_chat_room_get_user_data(const LinphoneChatRoom *chat_room);

/**
 * Assign a user pointer to the chat room.
 * @param chat_room the #LinphoneChatRoom object. @notnil
 * @param user_data The user pointer to associate with the chat room. @maybenil
 **/
LINPHONE_PUBLIC void linphone_chat_room_set_user_data(LinphoneChatRoom *chat_room, void *user_data);

/**
 * Creates an empty message attached to the given chat room.
 * @param chat_room the #LinphoneChatRoom object. @notnil
 * @return a new #LinphoneChatMessage @notnil
 */
LINPHONE_PUBLIC LinphoneChatMessage *linphone_chat_room_create_empty_message(LinphoneChatRoom *chat_room);

/**
 * Creates a message attached to the given chat room with a plain text content filled with the given message. Introduced
 * in 01/07/2020
 * @param chat_room the #LinphoneChatRoom object. @notnil
 * @param message text message in UTF8, NULL if absent. @maybenil
 * @return a new #LinphoneChatMessage @notnil
 */
LINPHONE_PUBLIC LinphoneChatMessage *linphone_chat_room_create_message_from_utf8(LinphoneChatRoom *chat_room,
                                                                                 const char *message);

/**
 * Creates a message attached to the given chat room with a particular content.
 * Use linphone_chat_message_send() to initiate the transfer
 * @param chat_room the #LinphoneChatRoom object. @notnil
 * @param initial_content #LinphoneContent initial content. @notnil
 * @return a new #LinphoneChatMessage @notnil
 */
LINPHONE_PUBLIC LinphoneChatMessage *linphone_chat_room_create_file_transfer_message(LinphoneChatRoom *chat_room,
                                                                                     LinphoneContent *initial_content);

/**
 * Creates a forward message attached to the given chat room with a particular message.
 * @param chat_room the #LinphoneChatRoom object. @notnil
 * @param message #LinphoneChatMessage message to be forwarded. @notnil
 * @return a new #LinphoneChatMessage @notnil
 */
LINPHONE_PUBLIC LinphoneChatMessage *linphone_chat_room_create_forward_message(LinphoneChatRoom *chat_room,
                                                                               LinphoneChatMessage *message);

/**
 * Creates a reply message attached to the given chat room with a particular message.
 * @param chat_room the #LinphoneChatRoom object. @notnil
 * @param message #LinphoneChatMessage message to reply to. @notnil
 * @return a new #LinphoneChatMessage @notnil
 */
LINPHONE_PUBLIC LinphoneChatMessage *linphone_chat_room_create_reply_message(LinphoneChatRoom *chat_room,
                                                                             LinphoneChatMessage *message);

/**
 * Creates a chat message with a voice recording attached to the given chat room.
 * @warning If the recorder isn't in Closed state, it will return an empty message!
 * @param chat_room the #LinphoneChatRoom object. @notnil
 * @param recorder the #LinphoneRecorder object used to record the voice message. @notnil
 * @return a new #LinphoneChatMessage @notnil
 */
LINPHONE_PUBLIC LinphoneChatMessage *linphone_chat_room_create_voice_recording_message(LinphoneChatRoom *chat_room,
                                                                                       LinphoneRecorder *recorder);

/**
 * Get the peer address associated to this chat room.
 * @warning This method returns an invalid address if the ChatRoom is in the Instantiated state
 * @param chat_room #LinphoneChatRoom object. @notnil
 * @return The peer address. @notnil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_chat_room_get_peer_address(LinphoneChatRoom *chat_room);

/**
 * Get the local address associated to this chat room.
 * @warning This method returns a guessed address based on the me participant if the ChatRoom is in the Instantiated
 * state
 * @param chat_room #LinphoneChatRoom object. @notnil
 * @return The local address. @notnil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_chat_room_get_local_address(LinphoneChatRoom *chat_room);

/**
 * Returns the chat room identifier
 * @warning This method returns a NULL pointer if the ChatRoom is in the Instantiated state
 * @param chat_room The #LinphoneChatRoom object. @notnil
 * @return the conference identifier. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_chat_room_get_identifier(const LinphoneChatRoom *chat_room);

/**
 * Returns the local account to which this chat room is related.
 * @param chat_room The #LinphoneChatRoom object. @notnil
 * @return the related #LinphoneAccount object if any, NULL otherwise. @maybenil
 */
LINPHONE_PUBLIC LinphoneAccount *linphone_chat_room_get_account(LinphoneChatRoom *chat_room);

/**
 * Used to receive a chat message when using async mechanism with IM enchat_roomyption engine
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param message #LinphoneChatMessage object @notnil
 */
LINPHONE_PUBLIC void linphone_chat_room_receive_chat_message(LinphoneChatRoom *chat_room, LinphoneChatMessage *message);

/**
 * Mark all messages of the conversation as read
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation. @notnil
 */
LINPHONE_PUBLIC void linphone_chat_room_mark_as_read(LinphoneChatRoom *chat_room);

/**
 * Enable or disable the ephemeral message feature in the chat room. Works only for flexisip-based chat room.
 * An ephemeral message will automatically disappear from the sender and recipient's chatrooms after a specified
 * timeout configurable with linphone_chat_room_set_ephemeral_lifetime().
 * The timer starts when the message has been displayed at the recipent, which means:
 * - at recipient side when linphone_chat_room_mark_as_read() is called.
 * - at sender side, when the message enters the state LinphoneChatMessageStateDisplayed (when receiving the displayed
 * IMDN).
 *
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param enable TRUE if the ephemeral message feature is enabled, FALSE otherwise.
 */
LINPHONE_PUBLIC void linphone_chat_room_enable_ephemeral(LinphoneChatRoom *chat_room, bool_t enable);

/**
 * Returns whether or not the ephemeral message feature is enabled in the chat room.
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation @notnil
 * @return TRUE if ephemeral is enabled, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_chat_room_ephemeral_enabled(const LinphoneChatRoom *chat_room);

/**
 * Sets lifetime (in seconds) for all new ephemeral messages in the chat room.
 * After the message is read, it will be deleted after "time" seconds.
 * @see linphone_chat_room_ephemeral_enabled()
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param time The ephemeral lifetime, default is 0 (disabled)
 * @warning A value of "time" equal to 0 disables ephemeral messages
 */
LINPHONE_PUBLIC void linphone_chat_room_set_ephemeral_lifetime(LinphoneChatRoom *chat_room, long time);

/**
 * Gets lifetime (in seconds) for all new ephemeral messages in the chat room.
 * After the message is read, it will be deleted after "time" seconds.
 * @see linphone_chat_room_ephemeral_enabled()
 * @param chat_room #LinphoneChatRoom object @notnil
 * @return the ephemeral lifetime (in secoonds)
 */
LINPHONE_PUBLIC long linphone_chat_room_get_ephemeral_lifetime(const LinphoneChatRoom *chat_room);

/**
 * Sets the ephemeral mode of the chat room
 * @see linphone_chat_room_ephemeral_enabled()
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param mode The ephemeral mode #LinphoneChatRoomEphemeralMode
 * @warning This function only changes the mode of ephemeral messages #LinphoneChatRoomEphemeralMode. It is required to
 * manually enable ephemeral messages after setting the mode by calling linphone_chat_room_enable_ephemeral()
 */
LINPHONE_PUBLIC void linphone_chat_room_set_ephemeral_mode(LinphoneChatRoom *chat_room,
                                                           LinphoneChatRoomEphemeralMode mode);

/**
 * Gets the ephemeral mode of the chat room.
 * @see linphone_chat_room_ephemeral_enabled()
 * @param chat_room #LinphoneChatRoom object @notnil
 * @return the ephemeral mode #LinphoneChatRoomEphemeralMode
 */
LINPHONE_PUBLIC LinphoneChatRoomEphemeralMode linphone_chat_room_get_ephemeral_mode(const LinphoneChatRoom *chat_room);

/**
 * Check if all participants support ephemeral messages.
 * It doesn't prevent to send ephemeral messages in the room but those who don't support it
 * won't delete messages after lifetime has expired.
 * The check is done by verifying the participant's advertised capabilities (+org.linphone.specs parameter).
 * @see linphone_chat_room_ephemeral_enabled()
 * @param chat_room #LinphoneChatRoom object @notnil
 * @return TRUE if all participants in the chat room support ephemeral messages, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_chat_room_ephemeral_supported_by_all_participants(const LinphoneChatRoom *chat_room);

/**
 * Delete a message from the chat room history.
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation. @notnil
 * @param message The #LinphoneChatMessage object to remove. @notnil
 */

LINPHONE_PUBLIC void linphone_chat_room_delete_message(LinphoneChatRoom *chat_room, LinphoneChatMessage *message);

/**
 * Delete all messages from the history
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation. @notnil
 */
LINPHONE_PUBLIC void linphone_chat_room_delete_history(LinphoneChatRoom *chat_room);

/**
 * Gets the number of messages in a chat room.
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation for which size has to be computed
 * @param filters The #LinphoneChatRoomHistoryFilterMask mask to filter the result with #LinphoneChatRoomHistoryFilter
 * @notnil
 * @return the number of messages.
 */
LINPHONE_PUBLIC int linphone_chat_room_get_history_size_2(LinphoneChatRoom *chat_room,
                                                          LinphoneChatRoomHistoryFilterMask filters);

/**
 * Returns whether or not a #LinphoneChatRoom has at least one #LinphoneChatMessage or not.
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation @notnil
 * @return TRUE if there are no #LinphoneChatMessage, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_chat_room_is_empty(LinphoneChatRoom *chat_room);

/**
 * Gets all contents for which content-type starts with either video/, audio/ or image/.
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation for which matching contents should be
 * retrieved. @notnil
 * @return A list of contents considered as "media". \bctbx_list{LinphoneContent} @tobefreed
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_chat_room_get_media_contents(LinphoneChatRoom *chat_room);

/**
 * Gets all contents for which content-type starts with either text/ or application/.
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation for which matching contents should be
 * retrieved. @notnil
 * @return A list of contents considered as "document". \bctbx_list{LinphoneContent} @tobefreed
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_chat_room_get_document_contents(LinphoneChatRoom *chat_room);

/**
 * Gets nb_message most recent events from chat_room chat room, sorted from oldest to most recent.
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation for which events should be
 * retrieved @notnil
 * @param nb_message Number of events to retrieve. 0 means everything.
 * @param filters The #LinphoneChatRoomHistoryFilterMask mask to filter the results with #LinphoneChatRoomHistoryFilter
 * @return A list of \bctbx_list{LinphoneEventLog} @tobefreed
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_chat_room_get_history_2(LinphoneChatRoom *chat_room,
                                                               int nb_message,
                                                               LinphoneChatRoomHistoryFilterMask filters);

/**
 * Gets the partial list of events in the given range, sorted from oldest to most recent.
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation for which messages should be
 * retrieved @notnil
 * @param begin The first event of the range to be retrieved. History most recent message has index 0.
 * @param end The last event of the range to be retrieved. History oldest message has index of history size - 1 (use
 * #linphone_chat_room_get_history_size_2() to retrieve history size)
 * @return A list of \bctbx_list{LinphoneEventLog} @tobefreed
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_chat_room_get_history_range_2(LinphoneChatRoom *chat_room,
                                                                     int begin,
                                                                     int end,
                                                                     LinphoneChatRoomHistoryFilterMask filters);

/**
 * Gets the partial list of messages in the given range near the #LinphoneEventLog provided, sorted from oldest to most
 * recent.
 * If before and after are both set to 0, the returned list is empty.
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation for which messages should be
 * retrieved @notnil
 * @param before The number of messages to retrieve before the event provided.
 * @param after The number of messages to retrieve after the event provided.
 * @param event The #LinphoneEventLog object corresponding to the event. @maybenil
 * @param filters The #LinphoneChatRoomHistoryFilterMask mask to filter the results with #LinphoneChatRoomHistoryFilter
 * @return A list of \bctbx_list{LinphoneEventLog} near the event provided included. @tobefreed
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_chat_room_get_history_range_near(LinphoneChatRoom *chat_room,
                                                                        unsigned int before,
                                                                        unsigned int after,
                                                                        LinphoneEventLog *event,
                                                                        LinphoneChatRoomHistoryFilterMask filters);
/**
 * Gets the partial list of messages between two given #LinphoneEventLog, sorted from oldest to most
 * recent.
 * If either first_event or last_event is null, then nothing is returned.
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation for which messages should be
 * retrieved @notnil
 * @param first_event The #LinphoneEventLog object corresponding to the event. @maybenil
 * @param last_event The #LinphoneEventLog object corresponding to the event. @maybenil
 * @param filters The #LinphoneChatRoomHistoryFilterMask mask to filter the results with #LinphoneChatRoomHistoryFilter
 * @return A list of \bctbx_list{LinphoneEventLog} between the two provided events, if any. @tobefreed
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_chat_room_get_history_range_between(LinphoneChatRoom *chat_room,
                                                                           LinphoneEventLog *first_event,
                                                                           LinphoneEventLog *last_event,
                                                                           LinphoneChatRoomHistoryFilterMask filters);

/**
 * Gets all unread messages for this chat room, sorted from oldest to most recent.
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation for which messages should be
 * retrieved @notnil
 * @return A list of unread chat messages. \bctbx_list{LinphoneChatMessage} @tobefreed
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_chat_room_get_unread_history(LinphoneChatRoom *chat_room);

/**
 * Gets the last chat message sent or received in this chat room
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation for which last message should be
 * retrieved @notnil
 * @return the latest #LinphoneChatMessage or NULL if no message. @maybenil
 */
LINPHONE_PUBLIC LinphoneChatMessage *linphone_chat_room_get_last_message_in_history(LinphoneChatRoom *chat_room);

/**
 * Gets the chat message sent or received in this chat room that matches the message_id
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation for which the message should be
 * retrieved @notnil
 * @param message_id The id of the message to find @notnil
 * @return the #LinphoneChatMessage if found or NULL. @maybenil
 */
LINPHONE_PUBLIC LinphoneChatMessage *linphone_chat_room_find_message(LinphoneChatRoom *chat_room,
                                                                     const char *message_id);

/**
 * Gets the event log sent or received in this chat room that matches a chat message with the given message_id
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation for which the message should be
 * retrieved @notnil
 * @param message_id The id of the message to find @notnil
 * @return the #LinphoneEventLog if found or NULL. @maybenil
 */
LINPHONE_PUBLIC LinphoneEventLog *linphone_chat_room_find_event_log(LinphoneChatRoom *chat_room,
                                                                    const char *message_id);

/**
 * Searches chat messages by text.
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation that will be searched @notnil
 * @param text The text to search in messages @notnil
 * @param from The #LinphoneEventLog object corresponding to the event where to start the search @maybenil
 * @param direction The #LinphoneSearchDirection where to search, LinphoneSearchDirectionUp will search older messages
 * while LinphoneSearchDirectionDown will search newer messages
 * @return the #LinphoneEventLog corresponding to the message if found or NULL. @maybenil
 */
LINPHONE_PUBLIC LinphoneEventLog *linphone_chat_room_search_chat_message_by_text(LinphoneChatRoom *chat_room,
                                                                                 const char *text,
                                                                                 const LinphoneEventLog *from,
                                                                                 LinphoneSearchDirection direction);

/**
 * Notifies the destination of the chat message being composed that the user is typing a new message.
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation for which a new message is being
 * typed. @notnil
 */
LINPHONE_PUBLIC void linphone_chat_room_compose(LinphoneChatRoom *chat_room);

/**
 * Tells whether the remote is currently composing a message.
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation. @notnil
 * @return TRUE if the remote is currently composing a message, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_chat_room_is_remote_composing(const LinphoneChatRoom *chat_room);

/**
 * Gets the number of unread messages in the chatroom.
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation. @notnil
 * @return the number of unread messages.
 */
LINPHONE_PUBLIC int linphone_chat_room_get_unread_messages_count(LinphoneChatRoom *chat_room);

/**
 * Returns back pointer to #LinphoneCore object.
 * @param chat_room the #LinphoneChatRoom object. @notnil
 * @return the #LinphoneCore object this chat room is attached to. @notnil
 **/
LINPHONE_PUBLIC LinphoneCore *linphone_chat_room_get_core(const LinphoneChatRoom *chat_room);

/**
 * When realtime text is enabled in a #LinphoneCall (see linphone_call_params_realtime_text_enabled()),
 * #LinphoneCoreIsComposingReceivedCb is called everytime a Real-Time Text character is received from peer.
 * linphone_chat_room_get_char() pops a character previously received from the receive queue, and returns it.
 * When a new line character is received, a #LinphoneChatMessage is automatically
 * created with received characters and notified to the application through the
 * #LinphoneCoreCbs or #LinphoneChatRoomCbs interfaces.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @return  RFC 4103/T.140 char
 */
LINPHONE_PUBLIC uint32_t linphone_chat_room_get_char(LinphoneChatRoom *chat_room);

/**
 * Gets the current call associated to this chatroom if any
 * To commit a message, use linphone_chat_message_send()
 * @param chat_room the #LinphoneChatRoom object @notnil
 * @return #LinphoneCall or NULL. @maybenil
 */
LINPHONE_PUBLIC LinphoneCall *linphone_chat_room_get_call(const LinphoneChatRoom *chat_room);

/**
 * Adds a listener in order to be notified of #LinphoneChatRoom events. Once an event is received, registred
 * #LinphoneChatRoomCbs are invoked sequencially.
 * @param chat_room #LinphoneChatRoom object to monitor. @notnil
 * @param cbs A #LinphoneChatRoomCbs object holding the callbacks you need. A reference is taken by the
 * #LinphoneChatRoom until you invoke linphone_call_remove_callbacks(). @notnil
 */
LINPHONE_PUBLIC void linphone_chat_room_add_callbacks(LinphoneChatRoom *chat_room, LinphoneChatRoomCbs *cbs);

/**
 * Removes a listener from a LinphoneChatRoom
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param cbs LinphoneChatRoomCbs object to remove. @notnil
 */
LINPHONE_PUBLIC void linphone_chat_room_remove_callbacks(LinphoneChatRoom *chat_room, LinphoneChatRoomCbs *cbs);

/**
 * Gets the current LinphoneChatRoomCbs.
 * This is meant only to be called from a callback to be able to get the user_data associated with the
 * LinphoneChatRoomCbs that is calling the callback.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @return The LinphoneChatRoomCbs that has called the last callback @maybenil
 */
LINPHONE_PUBLIC LinphoneChatRoomCbs *linphone_chat_room_get_current_callbacks(const LinphoneChatRoom *chat_room);

/**
 * Gets the state of the chat room.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @return The current #LinphoneChatRoomState of the chat room
 */
LINPHONE_PUBLIC LinphoneChatRoomState linphone_chat_room_get_state(const LinphoneChatRoom *chat_room);

/**
 * Returns whether or not the chat room has been left.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @return TRUE if the chat room has been left, FALSE otherwise.
 * @deprecated 16/03/2022 use linphone_chat_room_is_read_only() instead.
 */
LINPHONE_PUBLIC bool_t linphone_chat_room_has_been_left(const LinphoneChatRoom *chat_room);

/**
 * Returns whether or not a message can be sent using this chat room.
 * A chat room may be read only until it's created, or when it's a group you have left.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @return TRUE if a chat message can't be sent in it, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_chat_room_is_read_only(const LinphoneChatRoom *chat_room);

/**
 * Returns the creation time for the chat room
 * @param chat_room #LinphoneChatRoom object @notnil
 * @return the time at which the chat room was created
 */
LINPHONE_PUBLIC time_t linphone_chat_room_get_creation_time(const LinphoneChatRoom *chat_room);

/**
 * Returns the last updated time for the chat room
 * @param chat_room #LinphoneChatRoom object @notnil
 * @return the last updated time
 */
LINPHONE_PUBLIC time_t linphone_chat_room_get_last_update_time(const LinphoneChatRoom *chat_room);

/**
 * Adds a participant to a chat room. This may fail if this type of chat room does not handle participants.
 * Use linphone_chat_room_can_handle_participants() to know if this chat room handles participants.
 * @param chat_room A #LinphoneChatRoom object @notnil
 * @param addr The address of the participant to add to the chat room @notnil
 */
LINPHONE_PUBLIC void linphone_chat_room_add_participant(LinphoneChatRoom *chat_room, LinphoneAddress *addr);

/**
 * Adds several participants to a chat room at once. This may fail if this type of chat room does not handle
 * participants. Use linphone_chat_room_can_handle_participants() to know if this chat room handles participants.
 * @param chat_room A #LinphoneChatRoom object @notnil
 * @param addresses The participants to add. \bctbx_list{LinphoneAddress} @notnil
 * @return TRUE if everything is OK, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_chat_room_add_participants(LinphoneChatRoom *chat_room, const bctbx_list_t *addresses);

/**
 * Tells whether a chat room is able to handle participants.
 * @param chat_room A #LinphoneChatRoom object @notnil
 * @return TRUE if the chat room can handle participants, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_chat_room_can_handle_participants(const LinphoneChatRoom *chat_room);

/**
 * Finds a participant of a chat room from its address.
 * @param chat_room A #LinphoneChatRoom object @notnil
 * @param address The #LinphoneAddress to search in the list of participants of the chat room @notnil
 * @return The participant if found, NULL otherwise. @maybenil
 */
LINPHONE_PUBLIC LinphoneParticipant *linphone_chat_room_find_participant(const LinphoneChatRoom *chat_room,
                                                                         LinphoneAddress *address);

/**
 * Gets the capabilities of a chat room.
 * @param chat_room A #LinphoneChatRoom object @notnil
 * @return The capabilities of the chat room (as a #LinphoneChatRoomCapabilitiesMask)
 */
LINPHONE_PUBLIC LinphoneChatRoomCapabilitiesMask linphone_chat_room_get_capabilities(const LinphoneChatRoom *chat_room);

/**
 * Checks if a chat room has given capabilities.
 * @param chat_room A #LinphoneChatRoom object @notnil
 * @param mask a #LinphoneChatRoomCapabilitiesMask mask
 * @return TRUE if the mask matches, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_chat_room_has_capability(const LinphoneChatRoom *chat_room, int mask);

/**
 * Gets the conference address of the chat room.
 * @param chat_room A #LinphoneChatRoom object @notnil
 * @return The conference address of the chat room or NULL if this type of chat room is not conference based. @maybenil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_chat_room_get_conference_address(const LinphoneChatRoom *chat_room);

/**
 * Gets the participant representing myself in the chat room.
 * @param chat_room A #LinphoneChatRoom object @notnil
 * @return The participant representing myself in the conference or NULL if me is not set. @maybenil
 */
LINPHONE_PUBLIC LinphoneParticipant *linphone_chat_room_get_me(const LinphoneChatRoom *chat_room);

/**
 * Gets the number of participants in the chat room (that is without ourselves).
 * @param chat_room A #LinphoneChatRoom object @notnil
 * @return The number of participants in the chat room
 */
LINPHONE_PUBLIC int linphone_chat_room_get_nb_participants(const LinphoneChatRoom *chat_room);

/**
 * Gets the list of participants of a chat room.
 * @param chat_room A #LinphoneChatRoom object @notnil
 * @return A \bctbx_list{LinphoneParticipant} of the participants @maybenil @tobefreed
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_chat_room_get_participants(const LinphoneChatRoom *chat_room);

/**
 * Gets the subject of a chat room.
 * @param chat_room A #LinphoneChatRoom object @notnil
 * @return The subject of the chat room. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_chat_room_get_subject(const LinphoneChatRoom *chat_room);

/**
 * Gets the subject of a chat room (as an UTF8 string)
 * @param chat_room A #LinphoneChatRoom object @notnil
 * @return The subject of the chat room. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_chat_room_get_subject_utf8(const LinphoneChatRoom *chat_room);

/**
 * Gets the security level of a chat room.
 * @param chat_room A #LinphoneChatRoom object @notnil
 * @return The current #LinphoneChatRoomSecurityLevel of the chat room
 */
LINPHONE_PUBLIC LinphoneChatRoomSecurityLevel linphone_chat_room_get_security_level(LinphoneChatRoom *chat_room);

/**
 * Leaves a chat room.
 * @param chat_room A #LinphoneChatRoom object @notnil
 */
LINPHONE_PUBLIC void linphone_chat_room_leave(LinphoneChatRoom *chat_room);

/**
 * Removes a participant of a chat room.
 * @param chat_room A #LinphoneChatRoom object @notnil
 * @param participant The participant to remove from the chat room @notnil
 */
LINPHONE_PUBLIC void linphone_chat_room_remove_participant(LinphoneChatRoom *chat_room,
                                                           LinphoneParticipant *participant);

/**
 * Removes several participants of a chat room at once.
 * @param chat_room A #LinphoneChatRoom object @notnil
 * @param participants The participants to remove. \bctbx_list{LinphoneParticipant} @notnil
 */
LINPHONE_PUBLIC void linphone_chat_room_remove_participants(LinphoneChatRoom *chat_room,
                                                            const bctbx_list_t *participants);

/**
 * Changes the admin status of a participant of a chat room (you need to be an admin yourself to do this).
 * @param chat_room A #LinphoneChatRoom object @notnil
 * @param participant The Participant for which to change the admin status @notnil
 * @param is_admin A boolean value telling whether the participant should now be an admin or not
 */
LINPHONE_PUBLIC void linphone_chat_room_set_participant_admin_status(LinphoneChatRoom *chat_room,
                                                                     LinphoneParticipant *participant,
                                                                     bool_t is_admin);

/**
 * Sets the subject of a chat room.
 * @param chat_room A #LinphoneChatRoom object @notnil
 * @param subject The new subject to set for the chat room @maybenil
 */
LINPHONE_PUBLIC void linphone_chat_room_set_subject(LinphoneChatRoom *chat_room, const char *subject);

/**
 * Sets the subject of a chat room (utf-8 string).
 * @param chat_room A #LinphoneChatRoom object @notnil
 * @param subject The new subject to set for the chat room @maybenil
 */
LINPHONE_PUBLIC void linphone_chat_room_set_subject_utf8(LinphoneChatRoom *chat_room, const char *subject);

/**
 * Gets the list of participants that are currently composing
 * @param chat_room A #LinphoneChatRoom object @notnil
 * @return List of addresses that are in the is_composing state. \bctbx_list{LinphoneAddress}
 */
LINPHONE_PUBLIC const bctbx_list_t *linphone_chat_room_get_composing_addresses(LinphoneChatRoom *chat_room);

/**
 * Sets the conference address of a group chat room. This function needs to be called from the
 * #LinphoneChatRoomCbsConferenceAddressGenerationCb callback and only there.
 * This function is meaningful only for server implementation of chatroom, and shall not by used by client applications.
 * @param chat_room A #LinphoneChatRoom object @notnil
 * @param conference_address The conference #LinphoneAddress to be used by the group chat room @maybenil
 */
LINPHONE_PUBLIC void linphone_chat_room_set_conference_address(LinphoneChatRoom *chat_room,
                                                               LinphoneAddress *conference_address);

/**
 * Sets the list of participant devices in the form of SIP URIs with GRUUs for a given participant.
 * This function is meaningful only for server implementation of chatroom, and shall not by used by client applications.
 * @param chat_room A #LinphoneChatRoom object @notnil
 * @param participant_address The participant address @notnil
 * @param device_identities List of the participant devices to be used by the group chat room
 * \bctbx_list{LinphoneParticipantDeviceIdentity} @notnil
 */
LINPHONE_PUBLIC void linphone_chat_room_set_participant_devices(LinphoneChatRoom *chat_room,
                                                                LinphoneAddress *participant_address,
                                                                const bctbx_list_t *device_identities);

/**
 * Notifies the chatroom that a participant device has just registered.
 * This function is meaningful only for server implementation of chatroom, and shall not by used by client applications.
 * @param chat_room A #LinphoneChatRoom object @notnil
 * @param participant_device list of the participant devices to be used by the group chat room @notnil
 */
LINPHONE_PUBLIC void
linphone_chat_room_notify_participant_device_registration(LinphoneChatRoom *chat_room,
                                                          const LinphoneAddress *participant_device);

/**
 * Returns current parameters associated with the chat room.
 * This is typically the parameters passed at chat room chat_roomeation to linphone_core_chat_roomeate_chat_room() or
 *some default parameters if no #LinphoneChatRoomParams was explicitely passed during chat room chat_roomeation.
 * @param chat_room the #LinphoneChatRoom object @notnil
 * @return the current #LinphoneChatRoomParams parameters. @notnil
 **/
LINPHONE_PUBLIC const LinphoneChatRoomParams *linphone_chat_room_get_current_params(const LinphoneChatRoom *chat_room);

/**
 * Gets if a chat room has been flagged as muted (not by default).
 * A muted chat room isn't used to compute unread messages total count.
 * @param chat_room the #LinphoneChatRoom object @notnil
 * @return TRUE if the chat room is muted, FALSE otherwise.
 **/
LINPHONE_PUBLIC bool_t linphone_chat_room_get_muted(const LinphoneChatRoom *chat_room);

/**
 * Sets if a chat room should be considered as muted or not.
 * A muted chat room isn't used to compute unread messages total count.
 * @param chat_room the #LinphoneChatRoom object @notnil
 * @param muted TRUE to flag it as muted, FALSE to un-mute it.
 **/
LINPHONE_PUBLIC void linphone_chat_room_set_muted(LinphoneChatRoom *chat_room, bool_t muted);

/**
 * Gets the conference information associated to the chatroom.
 * @param chat_room The #LinphoneChatRoom object. @notnil
 * @return the #LinphoneConferenceInfo. @maybenil
 */
LINPHONE_PUBLIC const LinphoneConferenceInfo *linphone_chat_room_get_conference_info(LinphoneChatRoom *chat_room);

/**
 * Converts a #LinphoneChatRoomState enum to a  string.
 * @param state a #LinphoneChatRoomState to convert to string
 * @return the string representation of the #LinphoneChatRoomState @notnil
 */
LINPHONE_PUBLIC const char *linphone_chat_room_state_to_string(const LinphoneChatRoomState state);

/**
 * Gets the number of messages in a chat room.
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation for which size has to be computed
 * @notnil
 * @return the number of messages.
 * @deprecated 30/07/2024. Use linphone_chat_room_get_history_size_2() instead.
 */
LINPHONE_PUBLIC int linphone_chat_room_get_history_size(LinphoneChatRoom *chat_room);

/**
 * Gets nb_message most recent messages from chat_room chat room, sorted from oldest to most recent.
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation for which messages should be
 * retrieved @notnil
 * @param nb_message Number of message to retrieve. 0 means everything.
 * @return A list of \bctbx_list{LinphoneChatMessage} @tobefreed
 * @deprecated 30/07/2024. Use linphone_chat_room_get_history_2() instead.
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_chat_room_get_history(LinphoneChatRoom *chat_room, int nb_message);

/**
 * Gets the partial list of messages in the given range, sorted from oldest to most recent.
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation for which messages should be
 * retrieved @notnil
 * @param begin The first message of the range to be retrieved. History most recent message has index 0.
 * @param end The last message of the range to be retrieved. History oldest message has index of history size - 1 (use
 * #linphone_chat_room_get_history_size() to retrieve history size)
 * @return A list of chat messages. \bctbx_list{LinphoneChatMessage} @tobefreed
 * @deprecated 30/07/2024. Use linphone_chat_room_get_history_range_2() instead.
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_chat_room_get_history_range(LinphoneChatRoom *chat_room, int begin, int end);

/**
 * Gets nb_events most recent chat message events from chat_room chat room, sorted from oldest to most recent.
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation for which events should be retrieved
 * @notnil
 * @param nb_events Number of events to retrieve. 0 means everything.
 * @return A list \bctbx_list{LinphoneEventLog} @tobefreed
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_chat_room_get_history_message_events(LinphoneChatRoom *chat_room, int nb_events);

/**
 * Gets the partial list of chat message events in the given range, sorted from oldest to most recent.
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation for which events should be retrieved
 * @notnil
 * @param begin The first event of the range to be retrieved. History most recent event has index 0.
 * @param end The last event of the range to be retrieved. History oldest event has index of history size - 1
 * @return The list of chat message events. \bctbx_list{LinphoneEventLog} @tobefreed
 */
LINPHONE_PUBLIC bctbx_list_t *
linphone_chat_room_get_history_range_message_events(LinphoneChatRoom *chat_room, int begin, int end);

/**
 * Gets nb_events most recent events from chat_room chat room, sorted from oldest to most recent.
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation for which events should be retrieved
 * @notnil
 * @param nb_events Number of events to retrieve. 0 means everything.
 * @return The list of the most recent events. \bctbx_list{LinphoneEventLog} @tobefreed
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_chat_room_get_history_events(LinphoneChatRoom *chat_room, int nb_events);

/**
 * Gets the partial list of events in the given range, sorted from oldest to most recent.
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation for which events should be retrieved
 * @notnil
 * @param begin The first event of the range to be retrieved. History most recent event has index 0.
 * @param end The last event of the range to be retrieved. History oldest event has index of history size - 1
 * @return The list of the found events. \bctbx_list{LinphoneEventLog} @tobefreed
 */
LINPHONE_PUBLIC bctbx_list_t *
linphone_chat_room_get_history_range_events(LinphoneChatRoom *chat_room, int begin, int end);

/**
 * Gets the number of events in a chat room.
 * @param chat_room The #LinphoneChatRoom object corresponding to the conversation for which size has to be computed
 * @notnil
 * @return the number of events.
 */
LINPHONE_PUBLIC int linphone_chat_room_get_history_events_size(LinphoneChatRoom *chat_room);

/************ */
/* DEPRECATED */
/* ********** */

/**
 * Creates a message attached to the given chat room.
 * @param chat_room the #LinphoneChatRoom object.
 * @param message text message, NULL if absent.
 * @param external_body_url the URL given in external body or NULL.
 * @param state the LinphoneChatMessage. State of the message.
 * @param time the time_t at which the message has been received/sent.
 * @param is_read TRUE if the message should be flagged as read, FALSE otherwise.
 * @param is_incoming TRUE if the message has been received, FALSE otherwise.
 * @return a new #LinphoneChatMessage
 * @deprecated 14/11/2017 Use #linphone_chat_room_create_message_from_utf8() instead.
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED LinphoneChatMessage *
linphone_chat_room_create_message_2(LinphoneChatRoom *chat_room,
                                    const char *message,
                                    const char *external_body_url,
                                    LinphoneChatMessageState state,
                                    time_t time,
                                    bool_t is_read,
                                    bool_t is_incoming);

/**
 * Returns back pointer to #LinphoneCore object.
 * @deprecated 15/09/2017 use linphone_chat_room_get_core()
 * @donotwrap
 **/
LINPHONE_PUBLIC LINPHONE_DEPRECATED LinphoneCore *linphone_chat_room_get_lc(const LinphoneChatRoom *chat_room);

/**
 * Send a message to peer member of this chat room.
 * @param chat_room #LinphoneChatRoom object
 * @param message message to be sent
 * @deprecated 13/10/2017 Use linphone_chat_message_send() instead.
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED void linphone_chat_room_send_message(LinphoneChatRoom *chat_room,
                                                                         const char *message);

/**
 * Send a message to peer member of this chat room.
 * @param chat_room #LinphoneChatRoom object
 * @param message #LinphoneChatMessage object
 * The state of the message sending will be notified via the callbacks defined in the #LinphoneChatMessageCbs object
 * that can be obtained by calling linphone_chat_message_get_callbacks(). The #LinphoneChatMessage reference is
 * transferred to the function and thus doesn't need to be unref'd by the application.
 * @deprecated 13/10/2017 Use linphone_chat_message_send() instead.
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED void linphone_chat_room_send_chat_message(LinphoneChatRoom *chat_room,
                                                                              LinphoneChatMessage *message);

/**
 * Creates a message attached to the given chat room with a plain text content filled with the given message.
 * @param chat_room the #LinphoneChatRoom object. @notnil
 * @param message text message, NULL if absent. @maybenil
 * @return a new #LinphoneChatMessage @notnil
 * @deprecated 01/07/2020. Use linphone_chat_room_create_message_from_utf8() instead.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED LinphoneChatMessage *linphone_chat_room_create_message(LinphoneChatRoom *chat_room,
                                                                                           const char *message);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_CHAT_ROOM_H_
