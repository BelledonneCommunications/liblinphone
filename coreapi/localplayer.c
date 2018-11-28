/*
linphone
Copyright (C) 2000 - 2010 Simon MORLAT (simon.morlat@linphone.org)

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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "private.h"
#include <mediastreamer2/msmediaplayer.h>
#include <mediastreamer2/mssndcard.h>

static int _local_player_open(LinphonePlayer *obj, const char *filename);
static int _local_player_start(LinphonePlayer *obj);
static int _local_player_pause(LinphonePlayer *obj);
static int _local_player_seek(LinphonePlayer *obj, int time_ms);
static MSPlayerState _local_player_get_state(LinphonePlayer *obj);
static int _local_player_get_duration(LinphonePlayer *obj);
static int _local_player_get_current_position(LinphonePlayer *obj);
static void _local_player_close(LinphonePlayer *obj);
static void _local_player_destroy(LinphonePlayer *obj);
static void _local_player_eof_callback(void *user_data);

LinphonePlayer *linphone_core_create_local_player(LinphoneCore *lc, const char *sound_card_name, const char *video_display_name, void *window_id) {
	LinphonePlayer *obj = linphone_player_new(lc);
	MSSndCard *snd_card;
	MSSndCardManager *snd_card_manager = ms_factory_get_snd_card_manager(lc->factory);
	if (sound_card_name == NULL) sound_card_name = linphone_core_get_ringer_device(lc);
	snd_card = ms_snd_card_manager_get_card(snd_card_manager, sound_card_name);
	if (video_display_name == NULL) video_display_name = linphone_core_get_video_display_filter(lc);
	obj->impl = ms_media_player_new(lc->factory, snd_card, video_display_name, window_id);
	obj->open = _local_player_open;
	obj->start = _local_player_start;
	obj->pause = _local_player_pause;
	obj->seek = _local_player_seek;
	obj->get_state = _local_player_get_state;
	obj->get_duration = _local_player_get_duration;
	obj->get_position = _local_player_get_current_position;
	obj->close = _local_player_close;
	obj->destroy = _local_player_destroy;
	ms_media_player_set_eof_callback((MSMediaPlayer *)obj->impl, _local_player_eof_callback, obj);
	return obj;
}

bool_t linphone_local_player_matroska_supported(void) {
	return ms_media_player_matroska_supported();
}

static int _local_player_open(LinphonePlayer *obj, const char *filename) {
	return ms_media_player_open((MSMediaPlayer *)obj->impl, filename) ? 0 : -1;
}

static int _local_player_start(LinphonePlayer *obj) {
	return ms_media_player_start((MSMediaPlayer *)obj->impl) ? 0 : -1;
}

static int _local_player_pause(LinphonePlayer *obj) {
	ms_media_player_pause((MSMediaPlayer *)obj->impl);
	return 0;
}

static int _local_player_seek(LinphonePlayer *obj, int time_ms) {
	return ms_media_player_seek((MSMediaPlayer *)obj->impl, time_ms) ? 0 : -1;
}

static MSPlayerState _local_player_get_state(LinphonePlayer *obj) {
	return ms_media_player_get_state((MSMediaPlayer *)obj->impl);
}

static int _local_player_get_duration(LinphonePlayer *obj) {
	return ms_media_player_get_duration((MSMediaPlayer *)obj->impl);
}

static int _local_player_get_current_position(LinphonePlayer *obj) {
	return ms_media_player_get_current_position((MSMediaPlayer *)obj->impl);
}

static void _local_player_destroy(LinphonePlayer *obj) {
	ms_media_player_free((MSMediaPlayer *)obj->impl);
}

static void _local_player_close(LinphonePlayer *obj) {
	ms_media_player_close((MSMediaPlayer *)obj->impl);
}

static void _local_player_eof_callback(void *user_data) {
	LinphonePlayer *obj = (LinphonePlayer *)user_data;
	LinphonePlayerCbs *cbs = linphone_player_get_callbacks(obj);
	LinphonePlayerCbsEofReachedCb cb = linphone_player_cbs_get_eof_reached(cbs);
	if (cb) cb(obj);
}
