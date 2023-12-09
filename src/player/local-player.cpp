/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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

#include "local-player.h"

// TODO: From coreapi. Remove me later.
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

LocalPlayer::LocalPlayer(std::shared_ptr<Core> core,
                         MSSndCard *sndCard,
                         const std::string &videoDisplayName,
                         void *windowId)
    : Player(core) {
	mMediaPlayer =
	    ms_media_player_new(getCore()->getCCore()->factory, sndCard, L_STRING_TO_C(videoDisplayName), windowId);
	ms_media_player_set_eof_callback(mMediaPlayer, onEof, toC());
}

LocalPlayer::~LocalPlayer() {
	ms_media_player_free(mMediaPlayer);
}

// -----------------------------------------------------------------------------

void LocalPlayer::setVolumeGain(float gain) {
	ms_media_player_set_volume_gain(mMediaPlayer, gain);
}

void LocalPlayer::setWindowId(void *windowId) {
	ms_media_player_set_window_id(mMediaPlayer, windowId);
}

// -----------------------------------------------------------------------------

int LocalPlayer::getCurrentPosition() const {
	return ms_media_player_get_current_position(mMediaPlayer);
}

int LocalPlayer::getDuration() const {
	return ms_media_player_get_duration(mMediaPlayer);
}

LinphonePlayerState LocalPlayer::getState() const {
	return linphoneStateFromMs2State(ms_media_player_get_state(mMediaPlayer));
}

float LocalPlayer::getVolumeGain() const {
	return ms_media_player_get_volume_gain(mMediaPlayer);
}

bool LocalPlayer::isVideoAvailable() const {
	return ms_media_player_get_is_video_available(mMediaPlayer);
}

// -----------------------------------------------------------------------------

void LocalPlayer::close() {
	ms_media_player_close(mMediaPlayer);
}

void *LocalPlayer::createWindowId() {
	return ms_media_player_create_window_id(mMediaPlayer);
}

LinphoneStatus LocalPlayer::open(const std::string &filename) {
	return ms_media_player_open(mMediaPlayer, L_STRING_TO_C(filename)) ? 0 : -1;
}

LinphoneStatus LocalPlayer::pause() {
	ms_media_player_pause(mMediaPlayer);
	return 0;
}

LinphoneStatus LocalPlayer::seek(int timeMs) {
	return ms_media_player_seek(mMediaPlayer, timeMs) ? 0 : -1;
}

LinphoneStatus LocalPlayer::start() {
	return ms_media_player_start(mMediaPlayer) ? 0 : -1;
}

// -----------------------------------------------------------------------------

void LocalPlayer::onEof(void *userData) {
	LinphonePlayer *player = static_cast<LinphonePlayer *>(userData);
	Player *localPlayer = Player::toCpp(player);
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS_NO_ARG(Player, localPlayer, linphone_player_cbs_get_eof_reached);
}

LINPHONE_END_NAMESPACE
