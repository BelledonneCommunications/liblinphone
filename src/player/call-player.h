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

#ifndef _L_CALL_PLAYER_H_
#define _L_CALL_PLAYER_H_

#include <mediastreamer2/mediastream.h>
#include <mediastreamer2/msconference.h>

#include "c-wrapper/c-wrapper.h"
#include "player/player.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC CallPlayer : public Player {
public:
	CallPlayer(std::shared_ptr<Core> core, AudioStream *audioStream);
	CallPlayer(std::shared_ptr<Core> core, MSAudioConference *conference);
	CallPlayer(const CallPlayer &other) = delete;
	virtual ~CallPlayer();

	// Getters
	LinphonePlayerState getState() const override;

	// Others
	void close() override;
	LinphoneStatus open(const std::string &filename) override;
	LinphoneStatus pause() override;
	LinphoneStatus seek(int time_ms) override;
	LinphoneStatus start() override;

private:
	bool checkState(bool checkPlayer) const;
	static void onEof(void *userData, MSFilter *f, unsigned int eventId, void *arg);

	AudioStream *mAudioStream = nullptr;
	MSAudioEndpoint *mAudioEndpoint = nullptr;
	MSAudioConference *mAudioConference = nullptr;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CALL_PLAYER_H_
