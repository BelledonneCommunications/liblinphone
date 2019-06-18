/*
 * c-content.h
 * Copyright (C) 2010-2018 Belledonne Communications SARL
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
 * @param[in] content #LinphoneContent object.
 * @return The same #LinphoneContent object.
**/
LINPHONE_PUBLIC LinphoneContent *linphone_content_ref (LinphoneContent *content);

/**
 * Release reference to the content.
 * @param[in] content #LinphoneContent object.
**/
LINPHONE_PUBLIC void linphone_content_unref (LinphoneContent *content);

/**
 * Retrieve the user pointer associated with the content.
 * @param[in] content #LinphoneContent object.
 * @return The user pointer associated with the content.
**/
LINPHONE_PUBLIC void *linphone_content_get_user_data (const LinphoneContent *content);

/**
 * Assign a user pointer to the content.
 * @param[in] content #LinphoneContent object.
 * @param[in] ud The user pointer to associate with the content.
**/
LINPHONE_PUBLIC void linphone_content_set_user_data (LinphoneContent *content, void *user_data);

/**
 * Get the mime type of the content data.
 * @param[in] content #LinphoneContent object.
 * @return The mime type of the content data, for example "application".
 */
LINPHONE_PUBLIC const char *linphone_content_get_type (const LinphoneContent *content);

/**
 * Set the mime type of the content data.
 * @param[in] content #LinphoneContent object.
 * @param[in] type The mime type of the content data, for example "application".
 */
LINPHONE_PUBLIC void linphone_content_set_type (LinphoneContent *content, const char *type);

/**
 * Get the mime subtype of the content data.
 * @param[in] content #LinphoneContent object.
 * @return The mime subtype of the content data, for example "html".
 */
LINPHONE_PUBLIC const char *linphone_content_get_subtype (const LinphoneContent *content);

/**
 * Set the mime subtype of the content data.
 * @param[in] content #LinphoneContent object.
 * @param[in] subtype The mime subtype of the content data, for example "html".
 */
LINPHONE_PUBLIC void linphone_content_set_subtype (LinphoneContent *content, const char *subtype);

/**
 * Adds a parameter to the ContentType header.
 * @param[in] content LinphoneContent object.
 * @param[in] name the name of the parameter to add.
 * @param[in] value the value of the parameter to add.
 */
LINPHONE_PUBLIC void linphone_content_add_content_type_parameter (
	LinphoneContent *content,
	const char *name,
	const char *value
);

/**
 * Get the content data buffer, usually a string.
 * @param[in] content #LinphoneContent object.
 * @return The content data buffer.
 */
LINPHONE_PUBLIC const uint8_t *linphone_content_get_buffer (const LinphoneContent *content);

/**
 * Set the content data buffer, usually a string.
 * @param[in] content #LinphoneContent object.
 * @param[in] buffer The content data buffer.
 * @param[in] size The size of the content data buffer.
 */
LINPHONE_PUBLIC void linphone_content_set_buffer (LinphoneContent *content, const uint8_t *buffer, size_t size);

/**
 * Get the string content data buffer.
 * @param[in] content #LinphoneContent object
 * @return The string content data buffer.
 */
LINPHONE_PUBLIC const char *linphone_content_get_string_buffer (const LinphoneContent *content);

/**
 * Set the string content data buffer.
 * @param[in] content #LinphoneContent object.
 * @param[in] buffer The string content data buffer.
 */
LINPHONE_PUBLIC void linphone_content_set_string_buffer (LinphoneContent *content, const char *buffer);

/**
 * Get the content data buffer size, excluding null character despite null character is always set for convenience.
 * @param[in] content #LinphoneContent object.
 * @return The content data buffer size.
 */
LINPHONE_PUBLIC size_t linphone_content_get_size (const LinphoneContent *content);

/**
 * Get the file size if content is either a FileContent or a FileTransferContent.
 * @param[in] content #LinphoneContent object.
 * @return The represented file size.
 */
LINPHONE_PUBLIC size_t linphone_content_get_file_size(const LinphoneContent *content);

/**
 * Set the content data size, excluding null character despite null character is always set for convenience.
 * @param[in] content #LinphoneContent object
 * @param[in] size The content data buffer size.
 */
LINPHONE_PUBLIC void linphone_content_set_size (LinphoneContent *content, size_t size);

/**
 * Get the encoding of the data buffer, for example "gzip".
 * @param[in] content #LinphoneContent object.
 * @return The encoding of the data buffer.
 */
LINPHONE_PUBLIC const char *linphone_content_get_encoding (const LinphoneContent *content);

/**
 * Set the encoding of the data buffer, for example "gzip".
 * @param[in] content #LinphoneContent object.
 * @param[in] encoding The encoding of the data buffer.
 */
LINPHONE_PUBLIC void linphone_content_set_encoding (LinphoneContent *content, const char *encoding);

/**
 * Get the name associated with a RCS file transfer message. It is used to store the original filename of the file to be downloaded from server.
 * @param[in] content #LinphoneContent object.
 * @return The name of the content.
 */
LINPHONE_PUBLIC const char *linphone_content_get_name (const LinphoneContent *content);

/**
 * Set the name associated with a RCS file transfer message. It is used to store the original filename of the file to be downloaded from server.
 * @param[in] content #LinphoneContent object.
 * @param[in] name The name of the content.
 */
LINPHONE_PUBLIC void linphone_content_set_name (LinphoneContent *content, const char *name);

/**
 * Tell whether a content is a multipart content.
 * @param[in] content #LinphoneContent object.
 * @return A boolean value telling whether the content is multipart or not.
 */
LINPHONE_PUBLIC bool_t linphone_content_is_multipart (const LinphoneContent *content);

/**
 * Get all the parts from a multipart content.
 * @param[in] content #LinphoneContent object.
 * @return A \bctbx_list{LinphoneContent} \onTheFlyList object holding the part if found, NULL otherwise.
 */
LINPHONE_PUBLIC 
bctbx_list_t *linphone_content_get_parts (const LinphoneContent *content);

/**
 * Get a part from a multipart content according to its index.
 * @param[in] content #LinphoneContent object.
 * @param[in] idx The index of the part to get.
 * @return A #LinphoneContent object holding the part if found, NULL otherwise.
 */
LINPHONE_PUBLIC LinphoneContent *linphone_content_get_part (const LinphoneContent *content, int idx);

/**
 * Find a part from a multipart content looking for a part header with a specified value.
 * @param[in] content #LinphoneContent object.
 * @param[in] header_name The name of the header to look for.
 * @param[in] header_value The value of the header to look for.
 * @return A #LinphoneContent object object the part if found, NULL otherwise.
 */
LINPHONE_PUBLIC LinphoneContent *linphone_content_find_part_by_header (
	const LinphoneContent *content,
	const char *header_name,
	const char *header_value
);

/**
 * Get a custom header value of a content.
 * @param[in] content #LinphoneContent object.
 * @param[in] header_name The name of the header to get the value from.
 * @return The value of the header if found, NULL otherwise.
 */
LINPHONE_PUBLIC const char *linphone_content_get_custom_header (const LinphoneContent *content, const char *header_name);

/**
 * Get the key associated with a RCS file transfer message if encrypted
 * @param[in] content #LinphoneContent object.
 * @return The key to encrypt/decrypt the file associated to this content.
 */
LINPHONE_PUBLIC const char *linphone_content_get_key (const LinphoneContent *content);

/**
 * Get the size of key associated with a RCS file transfer message if encrypted
 * @param[in] content #LinphoneContent object.
 * @return The key size in bytes
 */
LINPHONE_PUBLIC size_t linphone_content_get_key_size (const LinphoneContent *content);

/**
 * Set the key associated with a RCS file transfer message if encrypted
 * @param[in] content #LinphoneContent object.
 * @param[in] key The key to be used to encrypt/decrypt file associated to this content.
 * @param[in] key_length The lengh of the key.
 */
LINPHONE_PUBLIC void linphone_content_set_key (LinphoneContent *content, const char *key, const size_t key_length);

/**
 * Get the file transfer filepath set for this content (replace linphone_chat_message_get_file_transfer_filepath).
 * @param[in] content #LinphoneContent object.
 * @return The file path set for this content if it has been set, NULL otherwise.
 */
LINPHONE_PUBLIC const char *linphone_content_get_file_path (const LinphoneContent *content);

/**
 * Set the file transfer filepath for this content (replace linphone_chat_message_set_file_transfer_filepath).
 * @param[in] content #LinphoneContent object.
 * @param[in] file_path the file transfer filepath.
 */
LINPHONE_PUBLIC void linphone_content_set_file_path (LinphoneContent *content, const char *file_path);

/**
 * Tells whether or not this content contains text.
 * @param[in] content #LinphoneContent object.
 * @return True if this content contains plain text, false otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_content_is_text (const LinphoneContent *content);

/**
 * Tells whether or not this content contains a file.
 * @param[in] content #LinphoneContent object.
 * @return True if this content contains a file, false otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_content_is_file (const LinphoneContent *content);

/**
 * Tells whether or not this content is a file transfer.
 * @param[in] content #LinphoneContent object.
 * @return True if this content is a file transfer, false otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_content_is_file_transfer (const LinphoneContent *content);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_CONTENT_H_
