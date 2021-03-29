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

#ifndef _L_C_CHAT_MESSAGE_H_
#define _L_C_CHAT_MESSAGE_H_

#include "linphone/api/c-chat-message-cbs.h"
#include "linphone/api/c-types.h"

// =============================================================================

typedef enum _LinphoneChatMessageDir{
	LinphoneChatMessageIncoming,
	LinphoneChatMessageOutgoing
} LinphoneChatMessageDir;

// =============================================================================

#ifdef __cplusplus
	extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup chatmessage
 * @{
 */

/**
 * Acquire a reference to the chat message.
 * @param message #LinphoneChatMessage object. @notnil
 * @return The same #LinphoneChatMessage object. @notnil
 */
LINPHONE_PUBLIC LinphoneChatMessage *linphone_chat_message_ref (LinphoneChatMessage *message);

/**
 * Release reference to the chat message.
 * @param message #LinphoneChatMessage object. @notnil
 */
LINPHONE_PUBLIC void linphone_chat_message_unref (LinphoneChatMessage *message);

/**
 * Retrieve the user pointer associated with the chat message.
 * @param message #LinphoneChatMessage object. @notnil
 * @return The user pointer associated with the chat message. @maybenil
 */
LINPHONE_PUBLIC void *linphone_chat_message_get_user_data (const LinphoneChatMessage *message);

/**
 * Assign a user pointer to the chat message.
 * @param message #LinphoneChatMessage object. @notnil
 * @param user_data The user pointer to associate with the chat message. @maybenil
 */
LINPHONE_PUBLIC void linphone_chat_message_set_user_data (LinphoneChatMessage *message, void *user_data);

// =============================================================================

/**
 * Returns back pointer to #LinphoneCore object.
 * @param message #LinphoneChatMessage object. @notnil
 * @return the #LinphoneCore object associated with this message. @notnil
**/
LINPHONE_PUBLIC LinphoneCore *linphone_chat_message_get_core (const LinphoneChatMessage *message);

/**
 * Linphone message can carry external body as defined by rfc2017
 * @param message #LinphoneChatMessage object. @notnil
 * @return external body url or NULL if not present. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_chat_message_get_external_body_url (const LinphoneChatMessage *message);

/**
 * Linphone message can carry external body as defined by rfc2017
 *
 * @param message #LinphoneChatMessage object. @notnil
 * @param external_body_url ex: access-type=URL; URL="http://www.foo.com/file" @maybenil
 */
LINPHONE_PUBLIC void linphone_chat_message_set_external_body_url (LinphoneChatMessage *message, const char *external_body_url);

/**
 * Get the time the message was sent.
 * @param message #LinphoneChatMessage object. @notnil
 * @return the timestamp of when the message was sent.
 */
LINPHONE_PUBLIC time_t linphone_chat_message_get_time (const LinphoneChatMessage *message);

/**
 * Returns wehther the message has been sent or received.
 * @param message #LinphoneChatMessage object. @notnil
 * @return TRUE if message has been sent, FALSE if it has been received.
 */
LINPHONE_PUBLIC bool_t linphone_chat_message_is_outgoing (const LinphoneChatMessage *message);

/**
 * Get origin of the message
 * @param message #LinphoneChatMessage object. @notnil
 * @return the #LinphoneAddress of the sender. @notnil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_chat_message_get_from_address (const LinphoneChatMessage *message);

/**
 * Get destination of the message
 * @param message #LinphoneChatMessage object. @notnil
 * @return the #LinphoneAddress of the recipient. @notnil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_chat_message_get_to_address (const LinphoneChatMessage *message);

/**
 * Get the content type of a chat message.
 * @param message #LinphoneChatMessage object. @notnil
 * @return The content type of the chat message @notnil
 */
LINPHONE_PUBLIC const char *linphone_chat_message_get_content_type (const LinphoneChatMessage *message);

/**
 * Set the content type of a chat message.
 * This content type must match a content that is text representable, such as text/plain, text/html or image/svg+xml.
 * @param message #LinphoneChatMessage object. @notnil
 * @param content_type The new content type of the chat message @notnil
 */
LINPHONE_PUBLIC void linphone_chat_message_set_content_type (LinphoneChatMessage *message, const char *content_type);

/**
 * Get text part of this message. Introduced in 01/07/2020
 * @param message #LinphoneChatMessage object. @notnil
 * @return The text in UTF8 or NULL if no text. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_chat_message_get_utf8_text (const LinphoneChatMessage* message);

/** Set a chat message text to be sent by linphone_chat_room_send_message(). Introduced in 01/07/2020
 * @param message #LinphoneChatMessage @notnil
 * @param text The text in UTF8 to set. @maybenil
 * @return 0 if succeed.
*/
LINPHONE_PUBLIC int linphone_chat_message_set_utf8_text(LinphoneChatMessage *message , const char* text);

/**
 * Get the message identifier.
 * It is used to identify a message so that it can be notified as delivered and/or displayed.
 * @param message #LinphoneChatMessage object. @notnil
 * @return The message identifier. @notnil
 */
LINPHONE_PUBLIC const char *linphone_chat_message_get_message_id (const LinphoneChatMessage *message);

/**
 * Linphone message has an app-specific field that can store a text. The application might want
 * to use it for keeping data over restarts, like thumbnail image path.
 * @param message #LinphoneChatMessage object. @notnil
 * @return the application-specific data or NULL if none has been stored. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_chat_message_get_appdata (const LinphoneChatMessage *message);

/**
 * Linphone message has an app-specific field that can store a text. The application might want
 * to use it for keeping data over restarts, like thumbnail image path.
 *
 * Invoking this function will attempt to update the message storage to reflect the change if it is
 * enabled.
 *
 * @param message #LinphoneChatMessage object. @notnil
 * @param data the data to store into the message. @maybenil
 */
LINPHONE_PUBLIC void linphone_chat_message_set_appdata (LinphoneChatMessage *message, const char *data);

/**
 * Returns the chatroom this message belongs to.
 * @param message #LinphoneChatMessage object. @notnil
 * @return the #LinphoneChatRoom in which this message has been sent or received. @notnil
 */
LINPHONE_PUBLIC LinphoneChatRoom *linphone_chat_message_get_chat_room (const LinphoneChatMessage *message);

// =============================================================================

/**
 * Get if a chat message is to be stored.
 * @param message #LinphoneChatMessage object. @notnil
 * @return Whether or not the message is to be stored
 */
LINPHONE_PUBLIC bool_t linphone_chat_message_get_to_be_stored (const LinphoneChatMessage *message);

/**
 * Set if a chat message is to be stored.
 * This content type must match a content that is text representable, such as text/plain, text/html or image/svg+xml.
 * @param message #LinphoneChatMessage object. @notnil
 * @param to_be_stored Whether or not the chat message is to be stored
 */
LINPHONE_PUBLIC void linphone_chat_message_set_to_be_stored (LinphoneChatMessage *message, bool_t to_be_stored);

LINPHONE_PUBLIC unsigned int linphone_chat_message_store (LinphoneChatMessage *message);

/**
 * Get the state of the message
 * @param message #LinphoneChatMessage object. @notnil
 * @return the current #LinphoneChatMessageState of the message.
 */
LINPHONE_PUBLIC LinphoneChatMessageState linphone_chat_message_get_state (const LinphoneChatMessage *message);

/**
 * Get if the message was encrypted when transfered
 * @param message #LinphoneChatMessage object. @notnil
 * @return TRUE if the message was encrypted when transfered, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_chat_message_is_secured (const LinphoneChatMessage *message);

/**
 * Get the file_transfer_information (used by call backs to recover informations during a rcs file transfer)
 *
 * @param message #LinphoneChatMessage object. @notnil
 * @return a pointer to the #LinphoneContent structure or NULL if not present. @maybenil
 */
LINPHONE_PUBLIC LinphoneContent *linphone_chat_message_get_file_transfer_information (const LinphoneChatMessage *message);

/**
 * Start the download of the #LinphoneContent referenced in the #LinphoneChatMessage from remote server.
 * @param message #LinphoneChatMessage object. @notnil
 * @param content the #LinphoneContent object to download (must have the linphone_content_is_file_transfer() method return TRUE). @notnil
 * @return FALSE if there is an error, TRUE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_chat_message_download_content(LinphoneChatMessage *message, LinphoneContent *content);

/**
 * Cancel an ongoing file transfer attached to this message.(upload or download)
 * @param message #LinphoneChatMessage object. @notnil
 */
LINPHONE_PUBLIC void linphone_chat_message_cancel_file_transfer (LinphoneChatMessage *message);

/**
 * Send a chat message.
 * @param message #LinphoneChatMessage object. @notnil
 */
LINPHONE_PUBLIC void linphone_chat_message_send (LinphoneChatMessage *message);

LINPHONE_PUBLIC const LinphoneAddress *linphone_chat_message_get_peer_address (const LinphoneChatMessage *message);

/**
 * Returns the local address the message was sent or received with.
 * @param message #LinphoneChatMessage object. @notnil
 * @return the #LinphoneAddress of the local address used to send/receive this message. @notnil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_chat_message_get_local_address (const LinphoneChatMessage *message);

/**
 * Add custom headers to the message.
 * @param message #LinphoneChatMessage object. @notnil
 * @param header_name name of the header @notnil
 * @param header_value header value @maybenil
 */
LINPHONE_PUBLIC void linphone_chat_message_add_custom_header (
	LinphoneChatMessage *message,
	const char *header_name,
	const char *header_value
);

/**
 * Retrieve a custom header value given its name.
 * @param message #LinphoneChatMessage object. @notnil
 * @param header_name header name searched @notnil
 * @return the custom header value or NULL if not found. @maybenil
 */
LINPHONE_PUBLIC const char * linphone_chat_message_get_custom_header (const LinphoneChatMessage *message, const char *header_name);

/**
 * Removes a custom header from the message.
 * @param message #LinphoneChatMessage object. @notnil
 * @param header_name name of the header to remove @notnil
 */
LINPHONE_PUBLIC void linphone_chat_message_remove_custom_header (LinphoneChatMessage *message, const char *header_name);

/**
 * Returns wether the message has been read or not.
 * @param message #LinphoneChatMessage object. @notnil
 * @return TRUE if message has been marked as read, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_chat_message_is_read (const LinphoneChatMessage *message);

LINPHONE_PUBLIC LinphoneReason linphone_chat_message_get_reason (const LinphoneChatMessage *message);

/**
 * Get full details about delivery error of a chat message.
 * @param message #LinphoneChatMessage object. @notnil
 * @return a #LinphoneErrorInfo describing the details. @notnil
 */
LINPHONE_PUBLIC const LinphoneErrorInfo *linphone_chat_message_get_error_info (const LinphoneChatMessage *message);

/**
 * Returns wether the chat message is a forward message or not.
 * @param message #LinphoneChatMessage object. @notnil
 * @return TRUE if it is a forward message, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_chat_message_is_forward (LinphoneChatMessage *message);
		
/**
 * Gets the forward info if available as a string
 * @param message #LinphoneChatMessage object. @notnil
 * @return the original sender of the message if it has been forwarded, NULL otherwise. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_chat_message_get_forward_info (const LinphoneChatMessage *message);

/**
 * Returns wether the chat message is an ephemeral message or not.
 * An ephemeral message will automatically disappear from the recipient's screen after the message has been viewed.
 * @param message #LinphoneChatMessage object. @notnil
 * @return TRUE if it is an ephemeral message, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_chat_message_is_ephemeral(const LinphoneChatMessage *message);

/**
 * Returns lifetime of an ephemeral message.
 * The lifetime is the duration after which the ephemeral message will disappear once viewed.
 * @see linphone_chat_message_is_ephemeral()
 * @param message #LinphoneChatMessage object. @notnil
 * @return the lifetime of an ephemeral message, by default 86400s.
 */
LINPHONE_PUBLIC long linphone_chat_message_get_ephemeral_lifetime (const LinphoneChatMessage *message);

/**
 * Returns the real time at which an ephemeral message expires and will be deleted.
 * @see linphone_chat_message_is_ephemeral()
 * @param message #LinphoneChatMessage object. @notnil
 * @return the time at which an ephemeral message expires. 0 means the message has not been read.
 */
LINPHONE_PUBLIC time_t linphone_chat_message_get_ephemeral_expire_time (const LinphoneChatMessage *message);

/**
 * Fulfill a chat message char by char. 
 * Message linked to a Real Time Text Call send char in realtime following RFC 4103/T.140
 * To commit a message, use linphone_chat_room_send_message()
 * @param message #LinphoneChatMessage object. @notnil
 * @param character T.140 char
 * @return 0 if succeed.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_chat_message_put_char (LinphoneChatMessage *message, uint32_t character);

/**
 * Add a listener in order to be notified of #LinphoneChatMessage events.
 * @param message #LinphoneChatMessage object to monitor. @notnil
 * @param cbs A #LinphoneChatMessageCbs object holding the callbacks you need. @notnil
 */
LINPHONE_PUBLIC void linphone_chat_message_add_callbacks(LinphoneChatMessage *message, LinphoneChatMessageCbs *cbs);

/**
 * Remove a listener from a #LinphoneChatMessage
 * @param message #LinphoneChatMessage object @notnil
 * @param cbs #LinphoneChatMessageCbs object to remove. @notnil
 */
LINPHONE_PUBLIC void linphone_chat_message_remove_callbacks(LinphoneChatMessage *message, LinphoneChatMessageCbs *cbs);

/**
 * Gets the current LinphoneChatMessageCbs.
 * This is meant only to be called from a callback to be able to get the user_data associated with the #LinphoneChatMessageCbs that is calling the callback.
 * @param message #LinphoneChatMessage object @notnil
 * @return The #LinphoneChatMessageCbs that has called the last callback. @maybenil
 */
LINPHONE_PUBLIC LinphoneChatMessageCbs *linphone_chat_message_get_current_callbacks(const LinphoneChatMessage *message);

/**
 * Adds a file content to the ChatMessage.
 * @param message #LinphoneChatMessage object. @notnil
 * @param content the #LinphoneContent object to add. @notnil
 */
LINPHONE_PUBLIC void linphone_chat_message_add_file_content (LinphoneChatMessage *message, LinphoneContent *content);

/**
 * Creates a #LinphoneContent of type PlainText with the given text as body. Introduced in 01/07/2020
 * @param message #LinphoneChatMessage object. @notnil
 * @param text The text in UTF8 to add to the message. @notnil
 */
LINPHONE_PUBLIC void linphone_chat_message_add_utf8_text_content (LinphoneChatMessage *message, const char *text);

/**
 * Adds a content to the ChatMessage.
 * @param message #LinphoneChatMessage object. @notnil
 * @param content the #LinphoneContent object to add. @notnil
 */
LINPHONE_PUBLIC void linphone_chat_message_add_content (LinphoneChatMessage *message, LinphoneContent *content);

/**
 * Removes a content from the ChatMessage.
 * @param message #LinphoneChatMessage object. @notnil
 * @param content the #LinphoneContent object to remove. @notnil
 */
LINPHONE_PUBLIC void linphone_chat_message_remove_content (LinphoneChatMessage *message, LinphoneContent *content);

/**
 * Returns the list of contents in the message.
 * @param message #LinphoneChatMessage object. @notnil
 * @return The list of #LinphoneContent. @bctbx_list{LinphoneContent} @notnil
 */
LINPHONE_PUBLIC const bctbx_list_t *linphone_chat_message_get_contents(const LinphoneChatMessage *message);

/**
 * Gets whether or not a file is currently being downloaded or uploaded
 * @param message #LinphoneChatMessage object. @notnil
 * @return TRUE if download or upload is in progress, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_chat_message_is_file_transfer_in_progress (const LinphoneChatMessage *message);

/**
 * Gets the list of participants for which the imdn state has reached the specified state and the time at which they did.
 * @param message #LinphoneChatMessage object. @notnil
 * @param state The LinphoneChatMessageState the imdn have reached (only use LinphoneChatMessageStateDelivered,
 * LinphoneChatMessageStateDeliveredToUser, LinphoneChatMessageStateDisplayed and LinphoneChatMessageStateNotDelivered)
 * @return The list of participants. \bctbx_list{LinphoneParticipantImdnState} @tobefreed @notnil
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_chat_message_get_participants_by_imdn_state (
	const LinphoneChatMessage *message,
	LinphoneChatMessageState state
);

/**
 * Gets the callId accociated with the message
 * @param message #LinphoneChatMessage object. @notnil
 * @return the call Id @notnil
 */
LINPHONE_PUBLIC const char *linphone_chat_message_get_call_id(const LinphoneChatMessage *message);

/************ */
/* DEPRECATED */
/* ********** */

/**
 * Returns wether the chat message has a text content or not.
 * @param message #LinphoneChatMessage object. @notnil
 * @return TRUE if it has one, FALSE otherwise.
 * @deprecated 27/10/2020. Check if linphone_chat_message_get_contents() contains a #LinphoneContent for which it's content type is PlainText.
 */
LINPHONE_PUBLIC bool_t linphone_chat_message_has_text_content (const LinphoneChatMessage *message);

/**
 * Get text part of this message
 * @param message #LinphoneChatMessage object.
 * @return text or NULL if no text.
 * @deprecated 07/11/2017 use linphone_chat_message_get_utf8_text() instead
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED const char *linphone_chat_message_get_text (const LinphoneChatMessage* message);

/**
 * Get the path to the file to read from or write to during the file transfer.
 * @param message #LinphoneChatMessage object
 * @return The path to the file to use for the file transfer.
 * @deprecated 12/07/2018 use linphone_content_get_file_path() instead.
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED const char *linphone_chat_message_get_file_transfer_filepath (const LinphoneChatMessage *message);


/**
 * Start the download of the file from remote server
 *
 * @param message #LinphoneChatMessage object. @notnil
 * @param status_cb #LinphoneChatMessageStateChangeCb status callback invoked when file is downloaded or could not be downloaded
 * @param user_data user data
 * @deprecated 21/09/2017 Use linphone_chat_message_download_file() instead.
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED void linphone_chat_message_start_file_download (
	LinphoneChatMessage *message,
	LinphoneChatMessageStateChangedCb status_cb,
	void *user_data
);

/**
 * Start the download of the file referenced in a #LinphoneChatMessage from remote server.
 * @param message #LinphoneChatMessage object. @notnil
 * @deprecated 12/07/2018 Use linphone_chat_message_download_content() instead
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED bool_t linphone_chat_message_download_file (LinphoneChatMessage *message);

/**
 * Resend a chat message if it is in the 'not delivered' state for whatever reason.
 * @param message #LinphoneChatMessage object. @notnil
 * @deprecated 13/10/2017 Use linphone_chat_message_send instead.
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED void linphone_chat_message_resend (LinphoneChatMessage *message);

/**
 * Set the path to the file to read from or write to during the file transfer.
 * @param message #LinphoneChatMessage object. @notnil
 * @param filepath The path to the file to use for the file transfer. @notnil
 * @deprecated 12/07/2018 use linphone_content_set_file_path() instead.
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED void linphone_chat_message_set_file_transfer_filepath (LinphoneChatMessage *message, const char *filepath);

/**
 * Get the #LinphoneChatMessageCbs object associated with the LinphoneChatMessage.
 * @param message #LinphoneChatMessage object. @notnil
 * @return The #LinphoneChatMessageCbs object associated with the LinphoneChatMessage. @notnil
 * @deprecated 19/02/2019
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED LinphoneChatMessageCbs *linphone_chat_message_get_callbacks (const LinphoneChatMessage *message);

/** Set a chat message text to be sent by linphone_chat_room_send_message()
 * @param message #LinphoneChatMessage @notnil
 * @param text The text is in System Locale. @maybenil
 * @return 0 if succeed.
 * @deprecated 01/07/2020. Use linphone_chat_message_set_utf8_text() instead.
 * @donotwrap
*/
LINPHONE_PUBLIC LINPHONE_DEPRECATED int linphone_chat_message_set_text(LinphoneChatMessage *message , const char* text);

/**
 * Gets the text content if available as a string
 * @param message #LinphoneChatMessage object. @notnil
 * @return the #LinphoneContent buffer if available in System Locale, null otherwise. @maybenil
 * @deprecated 01/07/2020. Use linphone_chat_message_get_utf8_text() instead.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED const char *linphone_chat_message_get_text_content (const LinphoneChatMessage *message );

/**
 * Creates a #LinphoneContent of type PlainText with the given text as body.
 * @param message #LinphoneChatMessage object. @notnil
 * @param text The text in System Locale to add to the message. @notnil
 * @deprecated 01/07/2020. Use linphone_chat_message_add_utf8_text_content() instead.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED void linphone_chat_message_add_text_content (LinphoneChatMessage *message, const char *text);

/**
 * Return whether or not a chat message is a file transfer.
 * @param message #LinphoneChatMessage object @notnil
 * @return Whether or not the message is a file transfer
 * @deprecated 06/07/2020 check if linphone_chat_message_get_contents() contains a #LinphoneContent for which linphone_content_is_file_transfer() returns TRUE.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED bool_t linphone_chat_message_is_file_transfer (const LinphoneChatMessage *message);

/**
 * Return whether or not a chat message is a text.
 * @param message #LinphoneChatMessage object. @notnil
 * @return Whether or not the message is a text
 * @deprecated 06/07/2020 check if linphone_chat_message_get_contents() contains a #LinphoneContent with a PlainText content type.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED bool_t linphone_chat_message_is_text (const LinphoneChatMessage *message);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_CHAT_MESSAGE_H_
