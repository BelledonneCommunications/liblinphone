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

#include "register-status.h"

#include "linphone/api/c-account.h"

using namespace std;

class RegisterStatusResponse : public Response {
public:
	RegisterStatusResponse();
	RegisterStatusResponse(int id, const LinphoneAccount *account);
	void append(int id, const LinphoneAccount *account);
};

RegisterStatusResponse::RegisterStatusResponse() {
}

RegisterStatusResponse::RegisterStatusResponse(int id, const LinphoneAccount *account) {
	append(id, account);
}

void RegisterStatusResponse::append(int id, const LinphoneAccount *account) {
	ostringstream ost;
	ost << getBody();
	if (ost.tellp() > 0) {
		ost << "\n";
	}
	ost << "Id: " << id << "\n";
	ost << "State: " << linphone_registration_state_to_string(linphone_account_get_state(account)) << "\n";
	setBody(ost.str());
}

RegisterStatusCommand::RegisterStatusCommand()
    : DaemonCommand("register-status",
                    "register-status <register_id>|ALL",
                    "Return status of a registration or of all registrations.") {
	addExample(make_unique<DaemonCommandExample>("register-status 1", "Status: Ok\n\n"
	                                                                  "Id: 1\n"
	                                                                  "State: LinphoneRegistrationOk"));
	addExample(make_unique<DaemonCommandExample>("register-status ALL", "Status: Ok\n\n"
	                                                                    "Id: 1\n"
	                                                                    "State: LinphoneRegistrationOk\n\n"
	                                                                    "Id: 2\n"
	                                                                    "State: LinphoneRegistrationFailed"));
	addExample(make_unique<DaemonCommandExample>("register-status 3", "Status: Error\n"
	                                                                  "Reason: No register with such id."));
}

void RegisterStatusCommand::exec(Daemon *app, const string &args) {
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
		RegisterStatusResponse response;
		for (int i = 1; i <= app->maxProxyId(); i++) {
			account = app->findProxy(i);
			if (account != NULL) {
				response.append(i, account);
			}
		}
		app->sendResponse(response);
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
		app->sendResponse(RegisterStatusResponse(pid, account));
	}
}
