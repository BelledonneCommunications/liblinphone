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

#include "call-player.h"

// TODO: From coreapi. Remove me later.
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

CallPlayer::CallPlayer(std::shared_ptr<Core> core, AudioStream *audioStream) : Player(core) {
	mAudioStream = audioStream;
}

// -----------------------------------------------------------------------------

LinphonePlayerState CallPlayer::getState() const {
	MSPlayerState state = MSPlayerClosed;
	if (!checkState(true)) return LinphonePlayerClosed;
	ms_filter_call_method(mAudioStream->av_player.player, MS_PLAYER_GET_STATE, &state);
	return linphoneStateFromMs2State(state);
}

// -----------------------------------------------------------------------------

void CallPlayer::close() {
	if (!checkState(true)) return;
	audio_stream_close_remote_play(mAudioStream);
}

LinphoneStatus CallPlayer::open(const std::string &filename) {
	if (!checkState(false)) return -1;
	MSFilter *filter = audio_stream_open_remote_play(mAudioStream, L_STRING_TO_C(filename));
	if (!filter) return -1;
	ms_filter_add_notify_callback(filter, &onEof, toC(), FALSE);
	return 0;
}

LinphoneStatus CallPlayer::pause() {
	if (!checkState(true)) return -1;
	return ms_filter_call_method_noarg(mAudioStream->av_player.player, MS_PLAYER_PAUSE);
}

LinphoneStatus CallPlayer::seek(int time_ms) {
	if (!checkState(true)) return -1;
	return ms_filter_call_method(mAudioStream->av_player.player, MS_PLAYER_SEEK_MS, &time_ms);
}

LinphoneStatus CallPlayer::start() {
	if (!checkState(true)) return -1;
	return ms_filter_call_method_noarg(mAudioStream->av_player.player, MS_PLAYER_START);
}

// -----------------------------------------------------------------------------

bool CallPlayer::checkState(bool checkPlayer) const {
	if (media_stream_get_state(&mAudioStream->ms) != MSStreamStarted) {
		lWarning() << "In-call player not usable with audio stream that is not started.";
		return false;
	}
	if (checkPlayer && !mAudioStream->av_player.player) {
		lError() << "CallPlayer::checkState(): no player.";
		return false;
	}
	return true;
}

void CallPlayer::onEof(void *userData, BCTBX_UNUSED(MSFilter *f), unsigned int eventId, BCTBX_UNUSED(void *arg)) {
	LinphonePlayer *player = static_cast<LinphonePlayer *>(userData);
	Player *callPlayer = Player::toCpp(player);
	switch (eventId) {
		case MS_PLAYER_EOF:
			LINPHONE_HYBRID_OBJECT_INVOKE_CBS_NO_ARG(Player, callPlayer, linphone_player_cbs_get_eof_reached);
			break;
	}
}

LINPHONE_END_NAMESPACE
