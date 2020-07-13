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

#ifndef LINPHONE_BUFFER_H_
#define LINPHONE_BUFFER_H_


#include "linphone/types.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup misc
 * @{
 */

/**
 * Create a new empty #LinphoneBuffer object.
 * @return A new #LinphoneBuffer object. @notnil
 */
LINPHONE_PUBLIC LinphoneBuffer * linphone_buffer_new(void);

/**
 * Create a new #LinphoneBuffer object from existing data.
 * @param data The initial data to store in the LinphoneBuffer. @notnil
 * @param size The size of the initial data to stroe in the LinphoneBuffer.
 * @return A new #LinphoneBuffer object. @notnil
 */
LINPHONE_PUBLIC LinphoneBuffer * linphone_buffer_new_from_data(const uint8_t *data, size_t size);

/**
 * Create a new #LinphoneBuffer object from a string.
 * @param data The initial string content of the LinphoneBuffer. @notnil
 * @return A new #LinphoneBuffer object. @notnil
 */
LINPHONE_PUBLIC LinphoneBuffer * linphone_buffer_new_from_string(const char *data);

/**
 * Acquire a reference to the buffer.
 * @param buffer #LinphoneBuffer object. @notnil
 * @return The same #LinphoneBuffer object. @notnil
**/
LINPHONE_PUBLIC LinphoneBuffer * linphone_buffer_ref(LinphoneBuffer *buffer);

/**
 * Release reference to the buffer.
 * @param buffer #LinphoneBuffer object. @notnil
**/
LINPHONE_PUBLIC void linphone_buffer_unref(LinphoneBuffer *buffer);

/**
 * Retrieve the user pointer associated with the buffer.
 * @param buffer #LinphoneBuffer object. @notnil
 * @return The user pointer associated with the buffer. @maybenil
**/
LINPHONE_PUBLIC void *linphone_buffer_get_user_data(const LinphoneBuffer *buffer);

/**
 * Assign a user pointer to the buffer.
 * @param buffer #LinphoneBuffer object. @notnil
 * @param user_data The user pointer to associate with the buffer. @maybenil
**/
LINPHONE_PUBLIC void linphone_buffer_set_user_data(LinphoneBuffer *buffer, void *user_data);

/**
 * Get the content of the data buffer.
 * @param buffer #LinphoneBuffer object. @notnil
 * @return The content of the data buffer.  @notnil
 */
LINPHONE_PUBLIC const uint8_t * linphone_buffer_get_content(const LinphoneBuffer *buffer);

/**
 * Set the content of the data buffer.
 * @param buffer #LinphoneBuffer object. @notnil
 * @param content The content of the data buffer. @notnil
 * @param size The size of the content of the data buffer.
 */
LINPHONE_PUBLIC void linphone_buffer_set_content(LinphoneBuffer *buffer, const uint8_t *content, size_t size);

/**
 * Get the string content of the data buffer.
 * @param buffer #LinphoneBuffer object
 * @return The string content of the data buffer. @notnil
 */
LINPHONE_PUBLIC const char * linphone_buffer_get_string_content(const LinphoneBuffer *buffer);

/**
 * Set the string content of the data buffer.
 * @param buffer #LinphoneBuffer object. @notnil
 * @param content The string content of the data buffer. @notnil
 */
LINPHONE_PUBLIC void linphone_buffer_set_string_content(LinphoneBuffer *buffer, const char *content);

/**
 * Get the size of the content of the data buffer.
 * @param buffer #LinphoneBuffer object. @notnil
 * @return The size of the content of the data buffer.
 */
LINPHONE_PUBLIC size_t linphone_buffer_get_size(const LinphoneBuffer *buffer);

/**
 * Set the size of the content of the data buffer.
 * @param buffer #LinphoneBuffer object @notnil
 * @param size The size of the content of the data buffer.
 */
LINPHONE_PUBLIC void linphone_buffer_set_size(LinphoneBuffer *buffer, size_t size);

/**
 * Tell whether the #LinphoneBuffer is empty.
 * @param buffer #LinphoneBuffer object @notnil
 * @return A boolean value telling whether the #LinphoneBuffer is empty or not.
 */
LINPHONE_PUBLIC bool_t linphone_buffer_is_empty(const LinphoneBuffer *buffer);

/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_BUFFER_H_ */
