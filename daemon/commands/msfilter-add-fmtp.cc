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

#include "msfilter-add-fmtp.h"
#include <mediastreamer2/msfilter.h>
#include <private.h>

using namespace std;

MSFilterAddFmtpCommand::MSFilterAddFmtpCommand() :
	DaemonCommand("msfilter-add-fmtp", "msfilter-add-fmtp call|stream <id> <fmtp>", "Add fmtp to the encoder of a call or a stream") {
	addExample(new DaemonCommandExample("msfilter-add-fmtp call 1 vbr=on",
						"Status: Ok"));
	addExample(new DaemonCommandExample("msfilter-add-fmtp call 2 vbr=on",
						"Status: Error\n"
						"Reason: No Call with such id."));
	addExample(new DaemonCommandExample("msfilter-add-fmtp stream 7 vbr=on",
						"Status: Error\n"
						"Reason: No Audio Stream with such id."));
}

void MSFilterAddFmtpCommand::exec(Daemon *app, const string& args) {
	string type;
	int id;
	string fmtp;

	istringstream ist(args);
	ist >> type;
	ist >> id;
	ist >> fmtp;
	if (ist.fail()) {
		app->sendResponse(Response("Missing/Incorrect parameter(s)."));
		return;
	}
	if (type.compare("call") == 0) {
		LinphoneCall *call = app->findCall(id);
		if (call == NULL) {
			app->sendResponse(Response("No Call with such id."));
			return;
		}
		AudioStream *astream = reinterpret_cast<AudioStream *>(linphone_call_get_stream(call, LinphoneStreamTypeAudio));
		if (astream == NULL || astream->ms.encoder == NULL) {
			app->sendResponse(Response("This call doesn't have an active audio stream."));
			return;
		}
		ms_filter_call_method(astream->ms.encoder, MS_FILTER_ADD_FMTP, (void *)fmtp.c_str());
	} else if (type.compare("stream") == 0) {
		AudioStream *stream = app->findAudioStream(id);
		if (stream == NULL) {
			app->sendResponse(Response("No Audio Stream with such id."));
			return;
		}
		ms_filter_call_method(stream->ms.encoder, MS_FILTER_ADD_FMTP, (void *)fmtp.c_str());
	} else {
		app->sendResponse(Response("Incorrect parameter(s)."));
		return;
	}
	app->sendResponse(Response());
}
