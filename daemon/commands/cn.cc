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

#include "cn.h"

using namespace std;

class CNResponse : public Response {
public:
	CNResponse(LinphoneCore *core);
};

CNResponse::CNResponse(LinphoneCore *core) : Response() {
	ostringstream ost;
	bool cn_enabled = linphone_core_generic_comfort_noise_enabled(core) == TRUE ? true : false;
	ost << "State: ";
	if (cn_enabled) {
		ost << "enabled\n";
	} else {
		ost << "disabled\n";
	}
	setBody(ost.str());
}


CNCommand::CNCommand() :
		DaemonCommand("cn", "cn [enable|disable]",
				"Enable or disable generic comfort noice (CN payload type) with the 'enable' and 'disable' parameters, return the status of the use of comfort noise without parameter.") {
	addExample(new DaemonCommandExample("cn enable",
						"Status: Ok\n\n"
						"State: enabled"));
	addExample(new DaemonCommandExample("cn disable",
						"Status: Ok\n\n"
						"State: disabled"));
	addExample(new DaemonCommandExample("cn",
						"Status: Ok\n\n"
						"State: disabled"));
}

void CNCommand::exec(Daemon *app, const string& args) {
	string status;
	istringstream ist(args);
	ist >> status;
	if (ist.fail()) {
		app->sendResponse(CNResponse(app->getCore()));
		return;
	}

	if (status.compare("enable") == 0) {
		linphone_core_enable_generic_comfort_noise(app->getCore(), TRUE);
	} else if (status.compare("disable") == 0) {
		linphone_core_enable_generic_comfort_noise(app->getCore(), FALSE);
	} else {
		app->sendResponse(Response("Incorrect parameter.", Response::Error));
		return;
	}
	app->sendResponse(CNResponse(app->getCore()));
}
