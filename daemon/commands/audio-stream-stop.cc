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

#include "audio-stream-stop.h"

using namespace std;

AudioStreamStopCommand::AudioStreamStopCommand() :
		DaemonCommand("audio-stream-stop", "audio-stream-stop <stream_id>", "Stop an audio stream.") {
	addExample(new DaemonCommandExample("audio-stream-stop 1",
						"Status: Ok"));
	addExample(new DaemonCommandExample("audio-stream-stop 2",
						"Status: Error\n"
						"Reason: No Audio Stream with such id."));
}

void AudioStreamStopCommand::exec(Daemon *app, const string& args) {
	int id;
	istringstream ist(args);
	ist >> id;
	if (ist.fail()) {
		app->sendResponse(Response("Missing/Incorrect parameter(s)."));
		return;
	}

	AudioStream *stream = app->findAudioStream(id);
	if (stream == NULL) {
		app->sendResponse(Response("No Audio Stream with such id."));
		return;
	}
	app->removeAudioStream(id);
	RtpProfile *prof=rtp_session_get_profile(stream->ms.sessions.rtp_session);
	audio_stream_stop(stream);
	rtp_profile_destroy(prof);
	app->sendResponse(Response());
}
