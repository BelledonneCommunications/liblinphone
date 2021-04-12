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

#ifndef LINPHONE_PLAYER_H_
#define LINPHONE_PLAYER_H_


#include "linphone/types.h"
#include "mediastreamer2/msinterfaces.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup call_control
 * @{
 */

/**
 * Acquire a reference to the player.
 * @param player #LinphonePlayer object. @notnil
 * @return The same #LinphonePlayer object. @notnil
**/
LINPHONE_PUBLIC LinphonePlayer * linphone_player_ref(LinphonePlayer *player);

/**
 * Release reference to the player.
 * @param player #LinphonePlayer object. @notnil
**/
LINPHONE_PUBLIC void linphone_player_unref(LinphonePlayer *player);

/**
 * Retrieve the user pointer associated with the player.
 * @param player #LinphonePlayer object. @notnil
 * @return The user pointer associated with the player. @maybenil
**/
LINPHONE_PUBLIC void *linphone_player_get_user_data(const LinphonePlayer *player);

/**
 * Assign a user pointer to the player.
 * @param player #LinphonePlayer object. @notnil
 * @param user_data The user pointer to associate with the player. @maybenil
**/
LINPHONE_PUBLIC void linphone_player_set_user_data(LinphonePlayer *player, void *user_data);

/**
 * Adds a #LinphonePlayerCbs object associated to the LinphonePlayer.
 * @param player #LinphonePlayer object @notnil
 * @param cbs The #LinphonePlayerCbs object to be added to the LinphonePlayer. @notnil
 */
LINPHONE_PUBLIC void linphone_player_add_callbacks(LinphonePlayer *player, LinphonePlayerCbs *cbs);

/**
 * Removes a #LinphonePlayerCbs object associated to the LinphonePlayer.
 * @param player #LinphonePlayer object @notnil
 * @param cbs The #LinphonePlayerCbs object to be removed from the LinphonePlayer. @notnil
 */
LINPHONE_PUBLIC void linphone_player_remove_callbacks(LinphonePlayer *player, LinphonePlayerCbs *cbs);

/**
 * Returns the current LinphonePlayerCbsCbs object
 * @param player #LinphonePlayer object @notnil
 * @return The current #LinphonePlayerCbs object @maybenil
 */
LINPHONE_PUBLIC LinphonePlayerCbs *linphone_player_get_current_callbacks(const LinphonePlayer *player);

/**
 * Open a file for playing.
 * @param player #LinphonePlayer object @notnil
 * @param filename The path to the file to open @notnil
 */
LINPHONE_PUBLIC LinphoneStatus linphone_player_open(LinphonePlayer *player, const char *filename);

/**
 * Start playing a file that has been opened with linphone_player_open().
 * @param player #LinphonePlayer object @notnil
 * @return 0 on success, a negative value otherwise
 */
LINPHONE_PUBLIC LinphoneStatus linphone_player_start(LinphonePlayer *player);

/**
 * Pause the playing of a file.
 * @param player #LinphonePlayer object @notnil
 * @return 0 on success, a negative value otherwise
 */
LINPHONE_PUBLIC LinphoneStatus linphone_player_pause(LinphonePlayer *player);

/**
 * Seek in an opened file.
 * @param player #LinphonePlayer object @notnil
 * @param time_ms The time we want to go to in the file (in milliseconds).
 * @return 0 on success, a negative value otherwise.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_player_seek(LinphonePlayer *player, int time_ms);

/**
 * Get the current state of a player.
 * @param player #LinphonePlayer object @notnil
 * @return The current #LinphonePlayerState of the player.
 */
LINPHONE_PUBLIC LinphonePlayerState linphone_player_get_state(LinphonePlayer *player);

/**
 * Get the duration of the opened file.
 * @param player #LinphonePlayer object @notnil
 * @return The duration of the opened file
 */
LINPHONE_PUBLIC int linphone_player_get_duration(LinphonePlayer *player);

/**
 * Get the current position in the opened file.
 * @param player #LinphonePlayer object @notnil
 * @return The current position in the opened file
 */
LINPHONE_PUBLIC int linphone_player_get_current_position(LinphonePlayer *player);

/**
 * Close the opened file.
 * @param player #LinphonePlayer object @notnil
 */
LINPHONE_PUBLIC void linphone_player_close(LinphonePlayer *player);

/**
 * Sets a window id to be used to display video if any.
 * @param player #LinphonePlayer object @notnil
 * @param window_id The window id pointer to use. @maybenil
 */
LINPHONE_PUBLIC void linphone_player_set_window_id(LinphonePlayer *player, void *window_id);

/**
 * Returns whether the file has video and if it can be displayed
 * @param player #LinphonePlayer object @notnil
 * @return TRUE if file has video and it can be displayed, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_player_get_is_video_available(LinphonePlayer *player);

/**
 * Returns the #LinphoneCore object managing this player's call, if any.
 * @param player #LinphonePlayer object @notnil
 * @return the #LinphoneCore object associated @notnil
 */
LINPHONE_PUBLIC LinphoneCore *linphone_player_get_core(const LinphonePlayer *player);

/**
 * Acquire a reference to the #LinphonePlayerCbs object.
 * @param cbs #LinphonePlayerCbs object. @notnil
 * @return The same #LinphonePlayerCbs object. @notnil
 */
LINPHONE_PUBLIC LinphonePlayerCbs * linphone_player_cbs_ref(LinphonePlayerCbs *cbs);

/**
 * Release reference to the #LinphonePlayerCbs object.
 * @param cbs #LinphonePlayerCbs object. @notnil
 */
LINPHONE_PUBLIC void linphone_player_cbs_unref(LinphonePlayerCbs *cbs);

/**
 * Retrieve the user pointer associated with the #LinphonePlayerCbs object.
 * @param cbs #LinphonePlayerCbs object. @notnil
 * @return The user pointer associated with the #LinphonePlayerCbs object. @maybenil
 */
LINPHONE_PUBLIC void *linphone_player_cbs_get_user_data(const LinphonePlayerCbs *cbs);

/**
 * Assign a user pointer to the #LinphonePlayerCbs object.
 * @param cbs #LinphonePlayerCbs object. @notnil
 * @param user_data The user pointer to associate with the #LinphonePlayerCbs object. @maybenil
 */
LINPHONE_PUBLIC void linphone_player_cbs_set_user_data(LinphonePlayerCbs *cbs, void *user_data);

/**
 * Get the end-of-file reached callback.
 * @param cbs #LinphonePlayerCbs object. @notnil
 * @return The current end-of-file reached callback.
 */
LINPHONE_PUBLIC LinphonePlayerCbsEofReachedCb linphone_player_cbs_get_eof_reached(const LinphonePlayerCbs *cbs);

/**
 * Set the end-of-file reached callback.
 * @param cbs #LinphonePlayerCbs object. @notnil
 * @param cb The end-of-file reached callback to be used.
 */
LINPHONE_PUBLIC void linphone_player_cbs_set_eof_reached(LinphonePlayerCbs *cbs, LinphonePlayerCbsEofReachedCb cb);

/************ */
/* DEPRECATED */
/* ********** */

/**
 * Get the #LinphonePlayerCbs object associated with the LinphonePlayer.
 * @param player #LinphonePlayer object @notnil
 * @return The #LinphonePlayerCbs object associated with the LinphonePlayer.
 * @deprecated 19/02/2019 use add_callbacks / remove_callbacks instead @notnil
 * @donotwrap
 */
LINPHONE_PUBLIC LINPHONE_DEPRECATED LinphonePlayerCbs * linphone_player_get_callbacks(const LinphonePlayer *player);

/**
 * @}
**/


#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_PLAYER_H_ */
