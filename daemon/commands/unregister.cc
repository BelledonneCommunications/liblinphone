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

#include "unregister.h"

using namespace std;

UnregisterCommand::UnregisterCommand()
    : DaemonCommand("unregister",
                    "unregister <register_id>|ALL",
                    "Unregister the daemon from the specified proxy or from all proxies.") {
	addExample(make_unique<DaemonCommandExample>("unregister 3", "Status: Error\n"
	                                                             "Reason: No register with such id."));
	addExample(make_unique<DaemonCommandExample>("unregister 2", "Status: Ok"));
	addExample(make_unique<DaemonCommandExample>("unregister ALL", "Status: Ok"));
}

void UnregisterCommand::exec(Daemon *app, const string &args) {
	LinphoneAccount *account = NULL;
	string param;
	int pid;

	istringstream ist(args);
	ist >> param;
	if (ist.fail()) {
		app->sendResponse(Response("Missing parameter.", Response::Error));
		return;
	}
	if (param.compare("ALL") == 0) {
		for (int i = 1; i <= app->maxProxyId(); i++) {
			account = app->findProxy(i);
			if (account != NULL) {
				linphone_core_remove_account(app->getCore(), account);
			}
		}
	} else {
		ist.clear();
		ist.str(param);
		ist >> pid;
		if (ist.fail()) {
			app->sendResponse(Response("Incorrect parameter.", Response::Error));
			return;
		}
		account = app->findProxy(pid);
		if (account == NULL) {
			app->sendResponse(Response("No register with such id.", Response::Error));
			return;
		}
		linphone_core_remove_account(app->getCore(), account);
	}
	app->sendResponse(Response());
}
