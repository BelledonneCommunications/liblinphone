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

#ifndef _L_C_CONTENT_H_
#define _L_C_CONTENT_H_

#include "linphone/api/c-types.h"

// =============================================================================

#ifdef __cplusplus
	extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup misc
 * @{
 */

/**
 * Acquire a reference to the content.
 * @param content #LinphoneContent object. @notnil
 * @return The same #LinphoneContent object. @notnil
**/
LINPHONE_PUBLIC LinphoneContent *linphone_content_ref (LinphoneContent *content);

/**
 * Release reference to the content.
 * @param content #LinphoneContent object. @notnil
**/
LINPHONE_PUBLIC void linphone_content_unref (LinphoneContent *content);

/**
 * Retrieve the user pointer associated with the content.
 * @param content #LinphoneContent object. @notnil
 * @return The user pointer associated with the content. @maybenil
**/
LINPHONE_PUBLIC void *linphone_content_get_user_data (const LinphoneContent *content);

/**
 * Assign a user pointer to the content.
 * @param content #LinphoneContent object. @notnil
 * @param user_data The user pointer to associate with the content. @maybenil
**/
LINPHONE_PUBLIC void linphone_content_set_user_data (LinphoneContent *content, void *user_data);

/**
 * Get the mime type of the content data.
 * @param content #LinphoneContent object. @notnil
 * @return The mime type of the content data, for example "application". @notnil
 */
LINPHONE_PUBLIC const char *linphone_content_get_type (const LinphoneContent *content);

/**
 * Set the mime type of the content data.
 * @param content #LinphoneContent object. @notnil
 * @param type The mime type of the content data, for example "application". @notnil
 */
LINPHONE_PUBLIC void linphone_content_set_type (LinphoneContent *content, const char *type);

/**
 * Get the mime subtype of the content data.
 * @param content #LinphoneContent object. @notnil
 * @return The mime subtype of the content data, for example "html". @notnil
 */
LINPHONE_PUBLIC const char *linphone_content_get_subtype (const LinphoneContent *content);

/**
 * Set the mime subtype of the content data.
 * @param content #LinphoneContent object. @notnil
 * @param subtype The mime subtype of the content data, for example "html". @notnil
 */
LINPHONE_PUBLIC void linphone_content_set_subtype (LinphoneContent *content, const char *subtype);

/**
 * Adds a parameter to the ContentType header.
 * @param content #LinphoneContent object. @notnil
 * @param name the name of the parameter to add. @notnil
 * @param value the value of the parameter to add. @maybenil
 */
LINPHONE_PUBLIC void linphone_content_add_content_type_parameter (
	LinphoneContent *content,
	const char *name,
	const char *value
);

/**
 * Get the content data buffer, usually a string.
 * @param content #LinphoneContent object. @notnil
 * @return The content data buffer. @notnil
 */
LINPHONE_PUBLIC const uint8_t *linphone_content_get_buffer (const LinphoneContent *content);

/**
 * Set the content data buffer, usually a string.
 * @param content #LinphoneContent object. @notnil
 * @param buffer The content data buffer. @notnil
 * @param size The size of the content data buffer.
 */
LINPHONE_PUBLIC void linphone_content_set_buffer (LinphoneContent *content, const uint8_t *buffer, size_t size);

/**
 * Get the string content data buffer. Introduced in 01/07/2020
 * @param content #LinphoneContent object. @notnil
 * @return The string content data buffer in UTF8. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_content_get_utf8_text(const LinphoneContent *content);

/**
 * Get the string content data buffer. Introduced in 01/07/2020
 * @param content #LinphoneContent object. @notnil
 * @param buffer The string content data buffer in UTF8. @maybenil
 */
LINPHONE_PUBLIC void linphone_content_set_utf8_text (LinphoneContent *content, const char *buffer);

/**
 * Get the content data buffer size, excluding null character despite null character is always set for convenience.
 * @param content #LinphoneContent object. @notnil
 * @return The content data buffer size.
 */
LINPHONE_PUBLIC size_t linphone_content_get_size (const LinphoneContent *content);

/**
 * Get the file size if content is either a FileContent or a FileTransferContent.
 * @param content #LinphoneContent object. @notnil
 * @return The represented file size.
 */
LINPHONE_PUBLIC size_t linphone_content_get_file_size(const LinphoneContent *content);

/**
 * Set the content data size, excluding null character despite null character is always set for convenience.
 * @param content #LinphoneContent object @notnil
 * @param size The content data buffer size.
 */
LINPHONE_PUBLIC void linphone_content_set_size (LinphoneContent *content, size_t size);

/**
 * Get the encoding of the data buffer, for example "gzip".
 * @param content #LinphoneContent object. @notnil
 * @return The encoding of the data buffer. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_content_get_encoding (const LinphoneContent *content);

/**
 * Set the encoding of the data buffer, for example "gzip".
 * @param content #LinphoneContent object. @notnil
 * @param encoding The encoding of the data buffer. @maybenil
 */
LINPHONE_PUBLIC void linphone_content_set_encoding (LinphoneContent *content, const char *encoding);

/**
 * Get the name associated with a RCS file transfer message. It is used to store the original filename of the file to be downloaded from server.
 * @param content #LinphoneContent object. @notnil
 * @return The name of the content. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_content_get_name (const LinphoneContent *content);

/**
 * Set the name associated with a RCS file transfer message. It is used to store the original filename of the file to be downloaded from server.
 * @param content #LinphoneContent object. @notnil
 * @param name The name of the content. @maybenil
 */
LINPHONE_PUBLIC void linphone_content_set_name (LinphoneContent *content, const char *name);

/**
 * Tell whether a content is a multipart content.
 * @param content #LinphoneContent object. @notnil
 * @return A boolean value telling whether the content is multipart or not.
 */
LINPHONE_PUBLIC bool_t linphone_content_is_multipart (const LinphoneContent *content);

/**
 * Get all the parts from a multipart content.
 * @param content #LinphoneContent object. @notnil
 * @return A \bctbx_list{LinphoneContent} object holding the part if found, NULL otherwise. @tobefreed @maybenil
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_content_get_parts (const LinphoneContent *content);

/**
 * Get a part from a multipart content according to its index.
 * @param content #LinphoneContent object. @notnil
 * @param index The index of the part to get.
 * @return A #LinphoneContent object holding the part if found, NULL otherwise. @maybenil
 */
LINPHONE_PUBLIC LinphoneContent *linphone_content_get_part (const LinphoneContent *content, int index);

/**
 * Find a part from a multipart content looking for a part header with a specified value.
 * @param content #LinphoneContent object. @notnil
 * @param header_name The name of the header to look for. @notnil
 * @param header_value The value of the header to look for. @notnil
 * @return A #LinphoneContent object object the part if found, NULL otherwise. @maybenil
 */
LINPHONE_PUBLIC LinphoneContent *linphone_content_find_part_by_header (
	const LinphoneContent *content,
	const char *header_name,
	const char *header_value
);

/**
 * Get a custom header value of a content.
 * @param content #LinphoneContent object. @notnil
 * @param header_name The name of the header to get the value from. @notnil
 * @return The value of the header if found, NULL otherwise. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_content_get_custom_header (const LinphoneContent *content, const char *header_name);

/**
 * Get the key associated with a RCS file transfer message if encrypted
 * @param content #LinphoneContent object. @notnil
 * @return The key to encrypt/decrypt the file associated to this content. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_content_get_key (const LinphoneContent *content);

/**
 * Get the size of key associated with a RCS file transfer message if encrypted
 * @param content #LinphoneContent object. @notnil
 * @return The key size in bytes
 */
LINPHONE_PUBLIC size_t linphone_content_get_key_size (const LinphoneContent *content);

/**
 * Set the key associated with a RCS file transfer message if encrypted
 * @param content #LinphoneContent object. @notnil
 * @param key The key to be used to encrypt/decrypt file associated to this content. @notnil
 * @param key_length The lengh of the key.
 */
LINPHONE_PUBLIC void linphone_content_set_key (LinphoneContent *content, const char *key, const size_t key_length);

/**
 * Get the file transfer filepath set for this content (replace linphone_chat_message_get_file_transfer_filepath).
 * @param content #LinphoneContent object. @notnil
 * @return The file path set for this content if it has been set, NULL otherwise. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_content_get_file_path (const LinphoneContent *content);

/**
 * If the content is an encrypted file, generate a temporary plain copy of the file and returns its paths
 * The caller is responsible to then delete this temporary copy and the returned string
 * @param[in] content #LinphoneContent object.
 * @return The file path set for this content if it has been set, NULL otherwise.
 */
LINPHONE_PUBLIC char *linphone_content_get_plain_file_path (const LinphoneContent *content);

/**
 * Set the file transfer filepath for this content (replace linphone_chat_message_set_file_transfer_filepath).
 * @param content #LinphoneContent object. @notnil
 * @param file_path the file transfer filepath. @maybenil
 */
LINPHONE_PUBLIC void linphone_content_set_file_path (LinphoneContent *content, const char *file_path);

/**
 * Tells whether or not this content contains text.
 * @param content #LinphoneContent object. @notnil
 * @return TRUE if this content contains plain text, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_content_is_text (const LinphoneContent *content);

/**
 * Tells whether or not this content contains a file.
 * @param content #LinphoneContent object. @notnil
 * @return TRUE if this content contains a file, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_content_is_file (const LinphoneContent *content);

/**
 * Tells whether or not this content is a file transfer.
 * @param content #LinphoneContent object. @notnil
 * @return TRUE if this content is a file transfer, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_content_is_file_transfer (const LinphoneContent *content);

/************ */
/* DEPRECATED */
/* ********** */

/**
 * Get the string content data buffer.
 * @param content #LinphoneContent object @notnil
 * @return The string content data buffer. @notnil
 * @deprecated 2020-07-01. Use linphone_content_get_utf8_text() instead.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED const char *linphone_content_get_string_buffer (const LinphoneContent *content);

/**
 * Set the string content data buffer.
 * @param content #LinphoneContent object. @notnil
 * @param buffer The string content data buffer in UTF8. @notnil
 * @deprecated 2020-07-01. Use linphone_content_set_utf8_text() instead.
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED void linphone_content_set_string_buffer (LinphoneContent *content, const char *buffer);

/**
 * Tells whether or not this content contains an encrypted file
 * @return True is this content contains a file and this file is encrypted, false otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_content_is_file_encrypted (const LinphoneContent *content);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_CONTENT_H_
