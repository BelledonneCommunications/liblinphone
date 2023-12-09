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

#include "player.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

Player::Player(std::shared_ptr<Core> core) : CoreAccessor(core) {
}

Player *Player::clone() const {
	return nullptr;
}

// -----------------------------------------------------------------------------

void Player::setVolumeGain(BCTBX_UNUSED(float gain)) {
}

void Player::setWindowId(BCTBX_UNUSED(void *windowId)) {
}

// -----------------------------------------------------------------------------

int Player::getCurrentPosition() const {
	return 0;
}

int Player::getDuration() const {
	return 0;
}

LinphonePlayerState Player::getState() const {
	return LinphonePlayerClosed;
}

float Player::getVolumeGain() const {
	return 0.0f;
}

bool Player::isVideoAvailable() const {
	return false;
}

// -----------------------------------------------------------------------------

void Player::close() {
}

void *Player::createWindowId() {
	return nullptr;
}

LinphoneStatus Player::open(BCTBX_UNUSED(const std::string &filename)) {
	return -1;
}

LinphoneStatus Player::pause() {
	return -1;
}

LinphoneStatus Player::seek(BCTBX_UNUSED(int time_ms)) {
	return -1;
}

LinphoneStatus Player::start() {
	return -1;
}

// -----------------------------------------------------------------------------

LinphonePlayerState Player::linphoneStateFromMs2State(MSPlayerState state) {
	switch (state) {
		case MSPlayerClosed:
		default:
			return LinphonePlayerClosed;
		case MSPlayerPaused:
			return LinphonePlayerPaused;
		case MSPlayerPlaying:
			return LinphonePlayerPlaying;
	}
}

// -----------------------------------------------------------------------------

LinphonePlayerCbsEofReachedCb PlayerCbs::getEofReached() const {
	return mEofReachedCb;
}

void PlayerCbs::setEofReached(LinphonePlayerCbsEofReachedCb cb) {
	mEofReachedCb = cb;
}

LINPHONE_END_NAMESPACE
