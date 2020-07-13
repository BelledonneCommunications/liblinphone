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
 * @param video_definition #LinphoneVideoDefinition object. @notnil
 * @return The same #LinphoneVideoDefinition object. @notnil
**/
LINPHONE_PUBLIC LinphoneVideoDefinition * linphone_video_definition_ref(LinphoneVideoDefinition *video_definition);

/**
 * Release reference to the video definition.
 * @param video_definition #LinphoneVideoDefinition object. @notnil
**/
LINPHONE_PUBLIC void linphone_video_definition_unref(LinphoneVideoDefinition *video_definition);

/**
 * Retrieve the user pointer associated with the video definition.
 * @param video_definition #LinphoneVideoDefinition object. @notnil
 * @return The user pointer associated with the video definition. @maybenil
**/
LINPHONE_PUBLIC void *linphone_video_definition_get_user_data(const LinphoneVideoDefinition *video_definition);

/**
 * Assign a user pointer to the video definition.
 * @param video_definition #LinphoneVideoDefinition object. @notnil
 * @param user_data The user pointer to associate with the video definition. @maybenil
**/
LINPHONE_PUBLIC void linphone_video_definition_set_user_data(LinphoneVideoDefinition *video_definition, void *user_data);

/**
 * Clone a video definition.
 * @param video_definition #LinphoneVideoDefinition object to be cloned @notnil
 * @return The new clone of the video definition @notnil
 */
LINPHONE_PUBLIC LinphoneVideoDefinition * linphone_video_definition_clone(const LinphoneVideoDefinition *video_definition);

/**
 * Get the width of the video definition.
 * @param video_definition #LinphoneVideoDefinition object @notnil
 * @return The width of the video definition
 */
LINPHONE_PUBLIC unsigned int linphone_video_definition_get_width(const LinphoneVideoDefinition *video_definition);

/**
 * Set the width of the video definition.
 * @param video_definition #LinphoneVideoDefinition object @notnil
 * @param width The width of the video definition
 */
LINPHONE_PUBLIC void linphone_video_definition_set_width(LinphoneVideoDefinition *video_definition, unsigned int width);

/**
 * Get the height of the video definition.
 * @param video_definition #LinphoneVideoDefinition object @notnil
 * @return The height of the video definition
 */
LINPHONE_PUBLIC unsigned int linphone_video_definition_get_height(const LinphoneVideoDefinition *video_definition);

/**
 * Set the height of the video definition.
 * @param video_definition #LinphoneVideoDefinition object @notnil
 * @param height The height of the video definition
 */
LINPHONE_PUBLIC void linphone_video_definition_set_height(LinphoneVideoDefinition *video_definition, unsigned int height);

/**
 * Set the width and the height of the video definition.
 * @param video_definition #LinphoneVideoDefinition object @notnil
 * @param width The width of the video definition
 * @param height The height of the video definition
 */
LINPHONE_PUBLIC void linphone_video_definition_set_definition(LinphoneVideoDefinition *video_definition, unsigned int width, unsigned int height);

/**
 * Get the name of the video definition.
 * @param video_definition #LinphoneVideoDefinition object @notnil
 * @return The name of the video definition @maybenil
 */
LINPHONE_PUBLIC const char * linphone_video_definition_get_name(const LinphoneVideoDefinition *video_definition);

/**
 * Set the name of the video definition.
 * @param video_definition #LinphoneVideoDefinition object @notnil
 * @param name The name of the video definition @maybenil
 */
LINPHONE_PUBLIC void linphone_video_definition_set_name(LinphoneVideoDefinition *video_definition, const char *name);

/**
 * Tells whether two #LinphoneVideoDefinition objects are equal (the widths and the heights are the same but can be switched).
 * @param video_definition1 #LinphoneVideoDefinition object @notnil
 * @param video_definition2 #LinphoneVideoDefinition object @notnil
 * @return A boolean value telling whether the two #LinphoneVideoDefinition objects are equal.
 */
LINPHONE_PUBLIC bool_t linphone_video_definition_equals(const LinphoneVideoDefinition *video_definition1, const LinphoneVideoDefinition *video_definition2);

/**
 * Tells whether two #LinphoneVideoDefinition objects are strictly equal (the widths are the same and the heights are the same).
 * @param video_definition1 #LinphoneVideoDefinition object @notnil
 * @param video_definition2 #LinphoneVideoDefinition object @notnil
 * @return A boolean value telling whether the two #LinphoneVideoDefinition objects are strictly equal.
 */
LINPHONE_PUBLIC bool_t linphone_video_definition_strict_equals(const LinphoneVideoDefinition *video_definition1, const LinphoneVideoDefinition *video_definition2);

/**
 * Tells whether a #LinphoneVideoDefinition is undefined.
 * @param video_definition #LinphoneVideoDefinition object @notnil
 * @return A boolean value telling whether the #LinphoneVideoDefinition is undefined.
 */
LINPHONE_PUBLIC bool_t linphone_video_definition_is_undefined(const LinphoneVideoDefinition *video_definition);

/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_VIDEO_DEFINITION_H_ */
