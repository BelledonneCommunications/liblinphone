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

#include "call-stats.h"

using namespace std;

CallStatsCommand::CallStatsCommand() :
		DaemonCommand("call-stats", "call-stats [<call_id>]", "Return all stats of a call.") {
	addExample(new DaemonCommandExample("call-stats 1",
						"Status: Ok\n\n"
						"Audio-ICE state: Not activated\n"
						"Audio-RoundTripDelay: 0.0859833\n"
						"Audio-Jitter: 296\n"
						"Audio-JitterBufferSizeMs: 47.7778\n"
						"Audio-Received-InterarrivalJitter: 154\n"
						"Audio-Received-FractionLost: 0\n"
						"Audio-Sent-InterarrivalJitter: 296\n"
						"Audio-Sent-FractionLost: 0\n"
						"Audio-Payload-type-number: 111\n"
						"Audio-Clock-rate: 16000\n"
						"Audio-Bitrate: 44000\n"
						"Audio-Mime: speex\n"
						"Audio-Channels: 1\n"
						"Audio-Recv-fmtp: vbr=on\n"
						"Audio-Send-fmtp: vbr=on\n\n"
						"Video-ICE state: Not activated\n"
						"Video-RoundTripDelay: 0\n"
						"Video-Jitter: 0\n"
						"Video-JitterBufferSizeMs: 0State: disabled"));
	addExample(new DaemonCommandExample("call-stats 2",
						"Status: Error\n"
						"Reason: No call with such id."));
	addExample(new DaemonCommandExample("call-stats",
						"Status: Error\n"
						"Reason: No current call available."));
}

void CallStatsCommand::exec(Daemon *app, const string& args) {
	LinphoneCore *lc = app->getCore();
	int cid;
	LinphoneCall *call = NULL;
	istringstream ist(args);
	ist >> cid;
	if (ist.fail()) {
		call = linphone_core_get_current_call(lc);
		if (call == NULL) {
			app->sendResponse(Response("No current call available."));
			return;
		}
	} else {
		call = app->findCall(cid);
		if (call == NULL) {
			app->sendResponse(Response("No call with such id."));
			return;
		}
	}

	ostringstream ostr;
	ostr << CallStatsEvent(app, call, linphone_call_get_audio_stats(call)).getBody();
	ostr << CallStatsEvent(app, call, linphone_call_get_video_stats(call)).getBody();
	app->sendResponse(Response(ostr.str(), Response::Ok));
}
