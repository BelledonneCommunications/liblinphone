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

#ifndef LINPHONE_INFO_MESSAGE_H_
#define LINPHONE_INFO_MESSAGE_H_


#include "linphone/types.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup misc
 * @{
 */


/**
 * Add a header to an info message to be sent.
 * @param info_message the #LinphoneInfoMessage object @notnil
 * @param name the header'name @notnil
 * @param value the header's value @maybenil
**/
LINPHONE_PUBLIC void linphone_info_message_add_header(LinphoneInfoMessage *info_message, const char *name, const char *value);

/**
 * Obtain a header value from a received info message.
 * @param info_message the #LinphoneInfoMessage object @notnil
 * @param name the header'name @notnil
 * @return the corresponding header's value, or NULL if not exists. @maybenil
**/
LINPHONE_PUBLIC const char *linphone_info_message_get_header(const LinphoneInfoMessage *info_message, const char *name);

/**
 * Assign a content to the info message.
 * 
 * All fields of the #LinphoneContent are copied, thus the application can destroy/modify/recycloe the content object freely ater the function returns.
 * @param info_message the #LinphoneInfoMessage object @notnil
 * @param content the content described as a #LinphoneContent structure. @maybenil
**/
LINPHONE_PUBLIC void linphone_info_message_set_content(LinphoneInfoMessage *info_message, const LinphoneContent *content);

/**
 * Returns the info message's content as a #LinphoneContent structure.
 * @param info_message the #LinphoneInfoMessage object @notnil
 * @return the #LinphoneContent object. @maybenil
**/
LINPHONE_PUBLIC const LinphoneContent * linphone_info_message_get_content(const LinphoneInfoMessage *info_message);

/**
 * Take a reference on a #LinphoneInfoMessage.
 * @param info_message the #LinphoneInfoMessage object @notnil
 * @return the same #LinphoneInfoMessage object @notnil
 */
LINPHONE_PUBLIC LinphoneInfoMessage *linphone_info_message_ref(LinphoneInfoMessage *info_message);

/**
 * Release a reference on a #LinphoneInfoMessage.
 * @param info_message the linphone info message @notnil
 */
LINPHONE_PUBLIC void linphone_info_message_unref(LinphoneInfoMessage *info_message);

LINPHONE_PUBLIC LinphoneInfoMessage *linphone_info_message_copy(const LinphoneInfoMessage *info_message);


/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_INFO_MESSAGE_H_ */
