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

#include "dtmf.h"

using namespace std;

DtmfCommand::DtmfCommand()
    : DaemonCommand(
          "dtmf", "dtmf <digit>", "Generate a DTMF (one of 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, *, #).") {
	addExample(make_unique<DaemonCommandExample>("dtmf 4", "Status: Ok"));
	addExample(make_unique<DaemonCommandExample>("dtmf B", "Status: Ok"));
	addExample(make_unique<DaemonCommandExample>("dtmf #", "Status: Ok"));
}

void DtmfCommand::exec(Daemon *app, const string &args) {
	string digit_str;
	char digit;
	istringstream ist(args);
	ist >> digit_str;
	if (ist.fail()) {
		app->sendResponse(Response("Missing digit parameter.", Response::Error));
		return;
	}

	digit = digit_str.at(0);
	if (isdigit(digit) || (digit == 'A') || (digit == 'B') || (digit == 'C') || (digit == 'D') || (digit == '*') ||
	    (digit == '#')) {
		LinphoneCall *call = linphone_core_get_current_call(app->getCore());
		linphone_core_play_dtmf(app->getCore(), digit, 200);
		if (call != NULL) {
			linphone_call_send_dtmf(call, digit);
		}
		app->sendResponse(Response());
	} else {
		app->sendResponse(Response("Incorrect digit parameter.", Response::Error));
	}
}
