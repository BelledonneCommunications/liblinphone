/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
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

#include "call-pause.h"

using namespace std;

CallPauseCommand::CallPauseCommand()
    : DaemonCommand("call-pause", "call-pause [<call_id>]", "Pause a call (pause current if no id is specified).") {
	addExample(make_unique<DaemonCommandExample>("call-pause 1", "Status: Ok\n\n"
	                                                             "Call was paused"));

	addExample(make_unique<DaemonCommandExample>("call-pause 2", "Status: Error\n"
	                                                             "Reason: No call with such id."));

	addExample(make_unique<DaemonCommandExample>("call-pause", "Status: Error\n"
	                                                           "Reason: No current call available."));
}

void CallPauseCommand::exec(Daemon *app, const string &args) {
	LinphoneCore *lc = app->getCore();
	int cid;
	LinphoneCall *call = NULL;
	bool current = false;
	istringstream ist(args);
	ist >> cid;
	if (ist.fail()) {
		call = linphone_core_get_current_call(lc);
		current = true;
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

	if (linphone_call_pause(call) == 0) {
		app->sendResponse(Response(current ? "Current call was paused" : "Call was paused", Response::Ok));
	} else {
		app->sendResponse(Response("Error pausing call"));
	}
}
