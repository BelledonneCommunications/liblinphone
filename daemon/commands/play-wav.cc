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

#include "play-wav.h"

using namespace std;

PlayWavCommand::PlayWavCommand() :
		DaemonCommand("play-wav", "play-wav <filename>",
				"Play an WAV audio file (needs to have enabled the linphone sound daemon (LSD).\n"
				"<filename> is the WAV file to be played.") {
	addExample(new DaemonCommandExample("play-wav /usr/local/share/sounds/linphone/hello8000.wav",
						"Status: Ok"));
}

void playWavFinished(LsdPlayer *p) {
	linphone_sound_daemon_release_player(lsd_player_get_daemon(p), p);
}

void PlayWavCommand::exec(Daemon *app, const string& args) {
	LinphoneSoundDaemon *lsd = app->getLSD();
	if (!lsd) {
		app->sendResponse(Response("The linphone sound daemon (LSD) is not enabled.", Response::Error));
		return;
	}

	string filename;
	istringstream ist(args);
	ist >> filename;
	if (ist.eof() && (filename.length() == 0)) {
		app->sendResponse(Response("Missing filename parameter.", Response::Error));
		return;
	}
	if (ist.fail()) {
		app->sendResponse(Response("Incorrect filename parameter.", Response::Error));
		return;
	}
	LsdPlayer *p = linphone_sound_daemon_get_player(lsd);
	lsd_player_set_callback(p, playWavFinished);
	lsd_player_play(p, filename.c_str());
	app->sendResponse(Response());
}
