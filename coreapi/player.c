
/*
linphone
Copyright (C) 2014 Belledonne Communications SARL

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "private.h"

/**
 * Open a new source on this player.
 * @param obj the player
 * @param filename file to open.
 * @param cb a callback used to notify end of play.
 * @param user_data a user-data provided in the callback to help the application to retrieve its context.
 * @return 0 if successful, -1 otherwise
**/
int linphone_player_open(LinphonePlayer *obj, const char *filename, LinphonePlayerEofCallback cb, void *user_data){
	obj->user_data=user_data;
	obj->cb=cb;
	return obj->open(obj,filename);
}

/**
 * Start a play operation. The player must have been open previously with linphone_player_open().
 * @param obj the player.
 * @return 0 if successful, -1 otherwise
**/
int linphone_player_start(LinphonePlayer *obj){
	return obj->start(obj);
}

/**
 * Suspend a play operation. The player must have been started previously with linphone_player_start().
 * @param obj the player.
 * @return 0 if successful, -1 otherwise
**/
int linphone_player_pause(LinphonePlayer *obj){
	return obj->pause(obj);
}

/**
 * Seek at a given position given in milliseconds. The player must be in the paused state.
 * @param obj the player.
 * @param time_ms the position to seek to.
 * @return 0 if successful, -1 otherwise
**/
int linphone_player_seek(LinphonePlayer *obj, int time_ms){
	return obj->seek(obj,time_ms);
}

/**
 * Get the state of play operation.
 * @param obj the player.
 * @return the state of the player within MSPlayerClosed, MSPlayerStarted, MSPlayerPaused.
**/
MSPlayerState linphone_player_get_state(LinphonePlayer *obj){
	return obj->get_state(obj);
}

/**
 * Get the duration of the media
 * @param obj the player
 * @return  The duration in milliseconds
 */
int linphone_player_get_duration(LinphonePlayer *obj) {
	return obj->get_duration(obj);
}

/**
 * Get the position of the playback
 * @param obj the player
 * @return Position of the playback in milliseconds
 */
int linphone_player_get_current_position(LinphonePlayer *obj) {
	return obj->get_position(obj);
}

/**
 * Close the player.
 * @param obj the player.
**/
void linphone_player_close(LinphonePlayer *obj){
	obj->close(obj);
}

/**
 * @brief Destroy a player
 * @param obj The player
 */
void linphone_player_destroy(LinphonePlayer *obj) {
	if(obj->destroy) obj->destroy(obj);
}

void _linphone_player_destroy(LinphonePlayer *player) {
	ms_free(player);
}


/*
 * Call player implementation below.
 */


static bool_t call_player_check_state(LinphonePlayer *player, bool_t check_player){
	LinphoneCall *call=(LinphoneCall*)player->impl;
	if (call->state!=LinphoneCallStreamsRunning){
		ms_warning("Call [%p]: in-call player not usable in state [%s]",call,linphone_call_state_to_string(call->state));
		return FALSE;
	}
	if (call->audiostream==NULL) {
		ms_error("call_player_check_state(): no audiostream.");
		return FALSE;
	}
	if (check_player && call->audiostream->av_player.player==NULL){
		ms_error("call_player_check_state(): no player.");
		return FALSE;
	}
	return TRUE;
}

static void on_eof(void *user_data, MSFilter *f, unsigned int event_id, void *arg){
	LinphonePlayer *player=(LinphonePlayer *)user_data;
	if (player->cb) player->cb(player,player->user_data);
}

static int call_player_open(LinphonePlayer* player, const char *filename){
	LinphoneCall *call=(LinphoneCall*)player->impl;
	MSFilter *filter;
	if (!call_player_check_state(player,FALSE)) return -1;
	filter=audio_stream_open_remote_play(call->audiostream,filename);
	if (!filter) return -1;
	ms_filter_add_notify_callback(filter,&on_eof,player,FALSE);
	return 0;
}

static int call_player_start(LinphonePlayer *player){
	LinphoneCall *call=(LinphoneCall*)player->impl;
	if (!call_player_check_state(player,TRUE)) return -1;
	return ms_filter_call_method_noarg(call->audiostream->av_player.player,MS_PLAYER_START);
}

static int call_player_pause(LinphonePlayer *player){
	LinphoneCall *call=(LinphoneCall*)player->impl;
	if (!call_player_check_state(player,TRUE)) return -1;
	return ms_filter_call_method_noarg(call->audiostream->av_player.player,MS_PLAYER_PAUSE);
}

static MSPlayerState call_player_get_state(LinphonePlayer *player){
	LinphoneCall *call=(LinphoneCall*)player->impl;
	MSPlayerState state=MSPlayerClosed;
	if (!call_player_check_state(player,TRUE)) return MSPlayerClosed;
	ms_filter_call_method(call->audiostream->av_player.player,MS_PLAYER_GET_STATE,&state);
	return state;
}

static int call_player_seek(LinphonePlayer *player, int time_ms){
	LinphoneCall *call=(LinphoneCall*)player->impl;
	if (!call_player_check_state(player,TRUE)) return -1;
	return ms_filter_call_method(call->audiostream->av_player.player,MS_PLAYER_SEEK_MS,&time_ms);
}

static void call_player_close(LinphonePlayer *player){
	LinphoneCall *call=(LinphoneCall*)player->impl;
	if (!call_player_check_state(player,TRUE)) return;
	audio_stream_close_remote_play(call->audiostream);
	
}

static void on_call_destroy(void *obj, belle_sip_object_t *call_being_destroyed){
	_linphone_player_destroy(obj);
}

LinphonePlayer *linphone_call_build_player(LinphoneCall *call){
	LinphonePlayer *obj=ms_new0(LinphonePlayer,1);
	obj->open=call_player_open;
	obj->close=call_player_close;
	obj->start=call_player_start;
	obj->seek=call_player_seek;
	obj->pause=call_player_pause;
	obj->get_state=call_player_get_state;
	obj->impl=call;
	belle_sip_object_weak_ref(call,on_call_destroy,obj);
	return obj;
}
