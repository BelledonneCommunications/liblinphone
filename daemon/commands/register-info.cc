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

#include "register-info.h"

#include <stdexcept>
#include <string>

#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"

using namespace std;

class RegisterInfoResponse : public Response {
public:
	RegisterInfoResponse() : Response() {
	}
	RegisterInfoResponse(int id, const ::LinphoneAccount *account) : Response() {
		append(id, account);
	}
	void append(int id, const ::LinphoneAccount *account) {
		ostringstream ost;
		ost << getBody();
		if (ost.tellp() > 0) ost << endl;
		ost << "Id: " << id << endl;
		const LinphoneAccountParams *params = linphone_account_get_params(account);
		ost << "Identity: " << linphone_account_params_get_identity(params) << endl;
		ost << "Proxy: " << linphone_account_params_get_server_addr(params) << endl;

		bctbx_list_t *routes = linphone_account_params_get_routes_addresses(params);
		char *route = linphone_address_as_string((LinphoneAddress *)bctbx_list_get_data(routes));
		if (route != NULL) {
			ost << "Route: " << route << endl;
		}
		bctbx_free(route);
		bctbx_list_free_with_data(routes, (bctbx_list_free_func)linphone_address_unref);

		ost << "State: " << linphone_registration_state_to_string(linphone_account_get_state(account)) << endl;
		setBody(ost.str());
	}
};

RegisterInfoCommand::RegisterInfoCommand()
    : DaemonCommand(
          "register-info", "register-info <register_id>|ALL", "Get informations about one or more registrations.") {
	addExample(make_unique<DaemonCommandExample>("register-info 1", "Status: Ok\n\n"
	                                                                "Id: 1\n"
	                                                                "Identity: sip:toto@sip.linphone.org\n"
	                                                                "Proxy: <sip:sip.linphone.org;transport=tls>\n"
	                                                                "Route: <sip:sip.linphone.org;transport=tls>\n"
	                                                                "State: LinphoneRegistrationOk"));
	addExample(make_unique<DaemonCommandExample>("register-info ALL", "Status: Ok\n\n"
	                                                                  "Id: 1\n"
	                                                                  "Identity: sip:toto@sip.linphone.org\n"
	                                                                  "Proxy: <sip:sip.linphone.org;transport=tls>\n"
	                                                                  "Route: <sip:sip.linphone.org;transport=tls>\n"
	                                                                  "State: LinphoneRegistrationOk\n\n"
	                                                                  "Id: 2\n"
	                                                                  "Identity: sip:toto2@sip.linphone.org\n"
	                                                                  "Proxy: <sip:sip.linphone.org;transport=udp>\n"
	                                                                  "State: LinphoneRegistrationFailed"));
	addExample(make_unique<DaemonCommandExample>("register-info 3", "Status: Error\n"
	                                                                "Reason: No register with such id."));
}

void RegisterInfoCommand::exec(Daemon *app, const string &args) {
	string param;
	istringstream ist(args);
	ist >> param;
	if (ist.fail()) {
		app->sendResponse(Response("Missing parameter.", Response::Error));
		return;
	}
	if (param == "ALL") {
		RegisterInfoResponse response;
		for (int i = 1; i <= app->maxProxyId(); i++) {
			::LinphoneAccount *account = app->findProxy(i);
			if (account != NULL) {
				response.append(i, account);
			}
		}
		app->sendResponse(response);
	} else {
		int id;
		try {
			id = atoi(param.c_str());
		} catch (invalid_argument &) {
			app->sendResponse(Response("Invalid ID.", Response::Error));
			return;
		} catch (out_of_range &) {
			app->sendResponse(Response("Out of range ID.", Response::Error));
			return;
		}
		::LinphoneAccount *account = app->findProxy(id);
		if (account == NULL) {
			app->sendResponse(Response("No register with such id.", Response::Error));
			return;
		}
		app->sendResponse(RegisterInfoResponse(id, account));
	}
}
