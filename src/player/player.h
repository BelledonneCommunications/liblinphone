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

#ifndef _L_PLAYER_H_
#define _L_PLAYER_H_

#include "c-wrapper/c-wrapper.h"
#include "core/core-accessor.h"
#include "linphone/callbacks.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class PlayerCbs;

class LINPHONE_PUBLIC Player : public bellesip::HybridObject<LinphonePlayer, Player>,
                               public UserDataAccessor,
                               public CallbacksHolder<PlayerCbs>,
                               public CoreAccessor {
public:
	Player(std::shared_ptr<Core> core);
	Player(const Player &other) = delete;
	virtual ~Player() = default;

	Player *clone() const override;

	// Setters
	virtual void setVolumeGain(float gain);
	virtual void setWindowId(void *windowId);

	// Getters
	virtual int getCurrentPosition() const;
	virtual int getDuration() const;
	virtual LinphonePlayerState getState() const;
	virtual float getVolumeGain() const;
	virtual bool isVideoAvailable() const;

	// Others
	virtual void close();
	virtual void *createWindowId();
	virtual LinphoneStatus open(const std::string &filename);
	virtual LinphoneStatus pause();
	virtual LinphoneStatus seek(int time_ms);
	virtual LinphoneStatus start();

protected:
	static LinphonePlayerState linphoneStateFromMs2State(MSPlayerState state);
};

class PlayerCbs : public bellesip::HybridObject<LinphonePlayerCbs, PlayerCbs>, public Callbacks {
public:
	LinphonePlayerCbsEofReachedCb getEofReached() const;
	void setEofReached(LinphonePlayerCbsEofReachedCb cb);

private:
	LinphonePlayerCbsEofReachedCb mEofReachedCb = nullptr;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_PLAYER_H_
