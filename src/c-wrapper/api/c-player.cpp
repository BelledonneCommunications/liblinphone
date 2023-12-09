
/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone
 * (see https://gitlab.linphone.org/BC/public/liblinphone).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <bctoolbox/defs.h>

#include "c-wrapper/c-wrapper.h"
#include "player/call-player.h"
#include "player/local-player.h"
#include "player/player.h"

// TODO: From coreapi. Remove me later.
#include "private.h"

using namespace LinphonePrivate;

LinphonePlayer *linphone_player_ref(LinphonePlayer *player) {
	Player::toCpp(player)->ref();
	return player;
}

void linphone_player_unref(LinphonePlayer *player) {
	Player::toCpp(player)->unref();
}

void *linphone_player_get_user_data(const LinphonePlayer *player) {
	return Player::toCpp(player)->getUserData();
}

void linphone_player_set_user_data(LinphonePlayer *player, void *ud) {
	Player::toCpp(player)->setUserData(ud);
}

void linphone_player_add_callbacks(LinphonePlayer *player, LinphonePlayerCbs *cbs) {
	Player::toCpp(player)->addCallbacks(PlayerCbs::getSharedFromThis(cbs));
}

void linphone_player_remove_callbacks(LinphonePlayer *player, LinphonePlayerCbs *cbs) {
	Player::toCpp(player)->removeCallbacks(PlayerCbs::getSharedFromThis(cbs));
}

LinphonePlayerCbs *linphone_player_get_current_callbacks(const LinphonePlayer *player) {
	return Player::toCpp(player)->getCurrentCallbacks()->toC();
}

const bctbx_list_t *linphone_player_get_callbacks_list(const LinphonePlayer *player) {
	return Player::toCpp(player)->getCCallbacksList();
}

LinphoneCore *linphone_player_get_core(const LinphonePlayer *player) {
	return L_GET_C_BACK_PTR(Player::toCpp(player)->getCore());
}

LinphoneStatus linphone_player_open(LinphonePlayer *player, const char *filename) {
	return Player::toCpp(player)->open(L_C_TO_STRING(filename));
}

LinphoneStatus linphone_player_start(LinphonePlayer *player) {
	return Player::toCpp(player)->start();
}

LinphoneStatus linphone_player_pause(LinphonePlayer *player) {
	return Player::toCpp(player)->pause();
}

LinphoneStatus linphone_player_seek(LinphonePlayer *player, int time_ms) {
	return Player::toCpp(player)->seek(time_ms);
}

LinphonePlayerState linphone_player_get_state(LinphonePlayer *player) {
	return Player::toCpp(player)->getState();
}

int linphone_player_get_duration(LinphonePlayer *player) {
	return Player::toCpp(player)->getDuration();
}

int linphone_player_get_current_position(LinphonePlayer *player) {
	return Player::toCpp(player)->getCurrentPosition();
}

void linphone_player_close(LinphonePlayer *player) {
	Player::toCpp(player)->close();
}

void *linphone_player_create_window_id(LinphonePlayer *player) {
	return Player::toCpp(player)->createWindowId();
}

void linphone_player_set_window_id(LinphonePlayer *player, void *window_id) {
	Player::toCpp(player)->setWindowId(window_id);
}

bool_t linphone_player_get_is_video_available(LinphonePlayer *player) {
	return Player::toCpp(player)->isVideoAvailable();
}

void linphone_player_set_volume_gain(LinphonePlayer *player, float gain) {
	Player::toCpp(player)->setVolumeGain(gain);
}

float linphone_player_get_volume_gain(LinphonePlayer *player) {
	return Player::toCpp(player)->getVolumeGain();
}

LinphonePlayerCbs *linphone_player_cbs_new(void) {
	return PlayerCbs::createCObject();
}

LinphonePlayerCbs *linphone_player_cbs_ref(LinphonePlayerCbs *cbs) {
	PlayerCbs::toCpp(cbs)->ref();
	return cbs;
}

void linphone_player_cbs_unref(LinphonePlayerCbs *cbs) {
	PlayerCbs::toCpp(cbs)->unref();
}

void *linphone_player_cbs_get_user_data(const LinphonePlayerCbs *cbs) {
	return PlayerCbs::toCpp(cbs)->getUserData();
}

void linphone_player_cbs_set_user_data(LinphonePlayerCbs *cbs, void *ud) {
	PlayerCbs::toCpp(cbs)->setUserData(ud);
}

LinphonePlayerCbsEofReachedCb linphone_player_cbs_get_eof_reached(const LinphonePlayerCbs *cbs) {
	return PlayerCbs::toCpp(cbs)->getEofReached();
}

void linphone_player_cbs_set_eof_reached(LinphonePlayerCbs *cbs, LinphonePlayerCbsEofReachedCb cb) {
	PlayerCbs::toCpp(cbs)->setEofReached(cb);
}

bool_t linphone_local_player_matroska_supported(void) {
	return ms_media_player_matroska_supported();
}

LinphonePlayer *linphone_core_create_local_player(LinphoneCore *lc,
                                                  const char *soundCardName,
                                                  const char *videoDisplayName,
                                                  void *windowId) {
	if (!soundCardName) soundCardName = linphone_core_get_media_device(lc);
	MSSndCardManager *sndCardManager = ms_factory_get_snd_card_manager(lc->factory);
	MSSndCard *sndCard = ms_snd_card_manager_get_card(sndCardManager, soundCardName);
	if (!sndCard) {
		lError() << "linphone_core_create_local_player(): no sound card.";
		return nullptr;
	}
	if (!videoDisplayName) videoDisplayName = linphone_core_get_video_display_filter(lc);
	return LocalPlayer::createCObject<LocalPlayer>(L_GET_CPP_PTR_FROM_C_OBJECT(lc), sndCard,
	                                               L_C_TO_STRING(videoDisplayName), windowId);
}
