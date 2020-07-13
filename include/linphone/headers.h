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

#ifndef LINPHONE_HEADERS_H_
#define LINPHONE_HEADERS_H_


#include "linphone/types.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup misc
 * @{
 */

/**
 * Increments ref count.
**/
LINPHONE_PUBLIC LinphoneHeaders * linphone_headers_ref(LinphoneHeaders *obj);

/**
 * Decrements ref count.
**/
LINPHONE_PUBLIC void linphone_headers_unref(LinphoneHeaders *obj);

/**
 * Search for a given header name and return its value.
 * @param headers the #LinphoneHeaders object @notnil
 * @param name the header's name @notnil
 * @return the header's value or NULL if not found. @maybenil
**/

LINPHONE_PUBLIC const char* linphone_headers_get_value(LinphoneHeaders *headers, const char *header_name);

/**
 * Add given header name and corresponding value.
 * @param headers the #LinphoneHeaders object @notnil
 * @param name the header's name @notnil
 * @param value the header's value @maybenil
**/
LINPHONE_PUBLIC void linphone_headers_add(LinphoneHeaders *headers, const char *name, const char *value);

/**
 * Add given header name and corresponding value.
 * @param headers the #LinphoneHeaders object @notnil
 * @param name the header's name @notnil
**/
LINPHONE_PUBLIC void linphone_headers_remove(LinphoneHeaders *headers, const char *name);


/**
 * @}
**/

#ifdef __cplusplus
}
#endif

#endif
