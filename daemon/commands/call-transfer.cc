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

#include "call-transfer.h"

using namespace std;

CallTransferCommand::CallTransferCommand() :
	DaemonCommand("call-transfer",
				  "call-transfer <call_to_transfer_id> <call_to_transfer_to_id>|<sip_url_to_transfer_to>",
				  "Transfer a call that you aswered to another party")
{
	addExample(new DaemonCommandExample("call-transfer 1 sip:john",
										"Status: Ok\n\n"
										"Call ID: 1\n"
										"Transfer to: sip:john"));

	addExample(new DaemonCommandExample("call-transfer 2 sip:john",
										"Status: Error\n"
										"Reason: No call with such id."));

	addExample(new DaemonCommandExample("call-transfer 1 2",
										"Status: Ok\n\n"
										"Call ID: 1\n"
										"Transfer to: 2"));
}

void CallTransferCommand::exec(Daemon* app, const string& args)
{
	LinphoneCall *call_to_transfer = NULL;
	LinphoneCall *call_to_transfer_to = NULL;
	istringstream ist(args);

	if (ist.peek() == EOF) {
		app->sendResponse(Response("Missing call_to_transfer_id parameter.", Response::Error));
		return;
	}

	int call_to_transfer_id;
	int call_to_transfer_to_id;
	string sip_uri_to_transfer_to;
	ist >> call_to_transfer_id;
	if (ist.fail()) {
		app->sendResponse(Response("Invalid command format (wrong call_to_transfer_id parameter)."));
		return;
	}
	if (ist.peek() == EOF) {
		app->sendResponse(Response("Missing call_to_transfer_to_id or sip_uri_to_transfer_to parameter.", Response::Error));
	}
	ist >> call_to_transfer_to_id;
	if (ist.fail()) {
		ist.clear();
		ist >> sip_uri_to_transfer_to;
		if (ist.fail()) {
			app->sendResponse(Response("Invalid command format (wrong call_to_transfer_to_id or sip_uri_to_transfer_to parameter."));
			return;
		}
	}

	call_to_transfer = app->findCall(call_to_transfer_id);
	if (call_to_transfer == NULL) {
		app->sendResponse(Response("No call with such id."));
		return;
	}
	if (sip_uri_to_transfer_to.empty()) {
		call_to_transfer_to = app->findCall(call_to_transfer_to_id);
		if (call_to_transfer_to == NULL) {
			app->sendResponse(Response("No call with such id."));
			return;
		}
		if (linphone_call_transfer_to_another(call_to_transfer, call_to_transfer_to) == 0) {
			ostringstream ostr;
			ostr << "Call ID: " << call_to_transfer_id << "\n";
			ostr << "Transfer to: " << call_to_transfer_to_id << "\n";
			app->sendResponse(Response(ostr.str(), Response::Ok));
			return;
		}
	} else {
		if (linphone_call_transfer(call_to_transfer, sip_uri_to_transfer_to.c_str()) == 0) {
			ostringstream ostr;
			ostr << "Call ID: " << call_to_transfer_id << "\n";
			ostr << "Transfer to: " << sip_uri_to_transfer_to << "\n";
			app->sendResponse(Response(ostr.str(), Response::Ok));
			return;
		}
	}

	app->sendResponse(Response("Error transferring call."));
}
