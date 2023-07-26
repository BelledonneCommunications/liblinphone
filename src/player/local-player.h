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

#ifndef _L_LOCAL_PLAYER_H_
#define _L_LOCAL_PLAYER_H_

#include "mediastreamer2/msmediaplayer.h"

#include "c-wrapper/c-wrapper.h"
#include "player/player.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC LocalPlayer : public Player {
public:
	LocalPlayer(std::shared_ptr<Core> core, MSSndCard *sndCard, const std::string &videoDisplayName, void *windowId);
	LocalPlayer(const LocalPlayer &other) = delete;
	~LocalPlayer();

	// Setters
	void setVolumeGain(float gain) override;
	void setWindowId(void *windowId) override;

	// Getters
	int getCurrentPosition() const override;
	int getDuration() const override;
	LinphonePlayerState getState() const override;
	float getVolumeGain() const override;
	bool isVideoAvailable() const override;

	// Others
	void close() override;
	void *createWindowId() override;
	LinphoneStatus open(const std::string &filename) override;
	LinphoneStatus pause() override;
	LinphoneStatus seek(int time_ms) override;
	LinphoneStatus start() override;

private:
	static void onEof(void *userData);

	MSMediaPlayer *mMediaPlayer = nullptr;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_LOCAL_PLAYER_H_
