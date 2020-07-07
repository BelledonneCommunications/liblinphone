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

#ifndef LINPHONE_VIDEO_DEFINITION_H_
#define LINPHONE_VIDEO_DEFINITION_H_


#include "linphone/types.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup media_parameters
 * @{
 */

/**
 * Acquire a reference to the video definition.
 * @param vdef #LinphoneVideoDefinition object.
 * @return The same #LinphoneVideoDefinition object.
**/
LINPHONE_PUBLIC LinphoneVideoDefinition * linphone_video_definition_ref(LinphoneVideoDefinition *vdef);

/**
 * Release reference to the video definition.
 * @param vdef #LinphoneVideoDefinition object.
**/
LINPHONE_PUBLIC void linphone_video_definition_unref(LinphoneVideoDefinition *vdef);

/**
 * Retrieve the user pointer associated with the video definition.
 * @param vdef #LinphoneVideoDefinition object.
 * @return The user pointer associated with the video definition. @maybenil
**/
LINPHONE_PUBLIC void *linphone_video_definition_get_user_data(const LinphoneVideoDefinition *vdef);

/**
 * Assign a user pointer to the video definition.
 * @param vdef #LinphoneVideoDefinition object.
 * @param user_data The user pointer to associate with the video definition. @maybenil
**/
LINPHONE_PUBLIC void linphone_video_definition_set_user_data(LinphoneVideoDefinition *vdef, void *user_data);

/**
 * Clone a video definition.
 * @param vdef #LinphoneVideoDefinition object to be cloned
 * @return The new clone of the video definition
 */
LINPHONE_PUBLIC LinphoneVideoDefinition * linphone_video_definition_clone(const LinphoneVideoDefinition *vdef);

/**
 * Get the width of the video definition.
 * @param vdef #LinphoneVideoDefinition object
 * @return The width of the video definition
 */
LINPHONE_PUBLIC unsigned int linphone_video_definition_get_width(const LinphoneVideoDefinition *vdef);

/**
 * Set the width of the video definition.
 * @param vdef #LinphoneVideoDefinition object
 * @param width The width of the video definition
 */
LINPHONE_PUBLIC void linphone_video_definition_set_width(LinphoneVideoDefinition *vdef, unsigned int width);

/**
 * Get the height of the video definition.
 * @param vdef #LinphoneVideoDefinition object
 * @return The height of the video definition
 */
LINPHONE_PUBLIC unsigned int linphone_video_definition_get_height(const LinphoneVideoDefinition *vdef);

/**
 * Set the height of the video definition.
 * @param vdef #LinphoneVideoDefinition object
 * @param height The height of the video definition
 */
LINPHONE_PUBLIC void linphone_video_definition_set_height(LinphoneVideoDefinition *vdef, unsigned int height);

/**
 * Set the width and the height of the video definition.
 * @param vdef #LinphoneVideoDefinition object
 * @param width The width of the video definition
 * @param height The height of the video definition
 */
LINPHONE_PUBLIC void linphone_video_definition_set_definition(LinphoneVideoDefinition *vdef, unsigned int width, unsigned int height);

/**
 * Get the name of the video definition.
 * @param vdef #LinphoneVideoDefinition object
 * @return The name of the video definition @maybenil
 */
LINPHONE_PUBLIC const char * linphone_video_definition_get_name(const LinphoneVideoDefinition *vdef);

/**
 * Set the name of the video definition.
 * @param vdef #LinphoneVideoDefinition object
 * @param name The name of the video definition
 */
LINPHONE_PUBLIC void linphone_video_definition_set_name(LinphoneVideoDefinition *vdef, const char *name);

/**
 * Tells whether two #LinphoneVideoDefinition objects are equal (the widths and the heights are the same but can be switched).
 * @param vdef1 #LinphoneVideoDefinition object
 * @param vdef2 #LinphoneVideoDefinition object
 * @return A boolean value telling whether the two #LinphoneVideoDefinition objects are equal.
 */
LINPHONE_PUBLIC bool_t linphone_video_definition_equals(const LinphoneVideoDefinition *vdef1, const LinphoneVideoDefinition *vdef2);

/**
 * Tells whether two #LinphoneVideoDefinition objects are strictly equal (the widths are the same and the heights are the same).
 * @param vdef1 #LinphoneVideoDefinition object
 * @param vdef2 #LinphoneVideoDefinition object
 * @return A boolean value telling whether the two #LinphoneVideoDefinition objects are strictly equal.
 */
LINPHONE_PUBLIC bool_t linphone_video_definition_strict_equals(const LinphoneVideoDefinition *vdef1, const LinphoneVideoDefinition *vdef2);

/**
 * Tells whether a #LinphoneVideoDefinition is undefined.
 * @param vdef #LinphoneVideoDefinition object
 * @return A boolean value telling whether the #LinphoneVideoDefinition is undefined.
 */
LINPHONE_PUBLIC bool_t linphone_video_definition_is_undefined(const LinphoneVideoDefinition *vdef);

/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_VIDEO_DEFINITION_H_ */
