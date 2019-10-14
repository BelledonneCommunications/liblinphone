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


#include "daemon.h"
#include "message.h"

using namespace std;

MessageCommand::MessageCommand() : DaemonCommand("message", "message <sip_address> <text>", "Send a SIP MESSAGE request with specified text."){
	addExample(new DaemonCommandExample("message sip:userxxx@sip.linphone.org Hi man !",
						"Status: Ok\n"));
}

void MessageCommand::exec(Daemon *app, const std::string &args){
	istringstream istr(args);
	string uri;
	
	istr>>uri;
	
	if (uri.empty()){
		app->sendResponse(Response("Missing uri parameter.", Response::Error));
		return;
	}
	LinphoneAddress *addr = linphone_factory_create_address(linphone_factory_get(), uri.c_str());
	if (!addr){
		app->sendResponse(Response("Bad sip uri.", Response::Error));
		return;
	}
	LinphoneChatRoom *cr = linphone_core_get_chat_room(app->getCore(), addr);
	linphone_address_unref(addr);
	if (!cr){
		app->sendResponse(Response("Internal error creating chat room.", Response::Error));
		return;
	}
	string text(args.substr(uri.size() + 1, string::npos));
	LinphoneChatMessage *msg = linphone_chat_room_create_message(cr, text.c_str());
	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, sMsgStateChanged);
	linphone_chat_message_set_user_data(msg, app);
	linphone_chat_message_send(msg);
	ostringstream ostr;
	ostr << "Id: "<< linphone_chat_message_get_message_id(msg) << "\n";
	app->sendResponse(Response(ostr.str(), Response::Ok));
}

void MessageCommand::sMsgStateChanged(LinphoneChatMessage *msg, LinphoneChatMessageState state){
	Daemon *app = (Daemon*) linphone_chat_message_get_user_data(msg);
	app->queueEvent(new OutgoingMessageEvent(msg));
}


OutgoingMessageEvent::OutgoingMessageEvent(LinphoneChatMessage *msg) : Event("message-state-changed"){
	ostringstream ostr;
	ostr << "Id: "<< linphone_chat_message_get_message_id(msg) << "\n";
	ostr << "State: " << linphone_chat_message_state_to_string(linphone_chat_message_get_state(msg)) << "\n";
	setBody(ostr.str());
	
	switch(linphone_chat_message_get_state(msg)){
		case LinphoneChatMessageStateDelivered:
		case LinphoneChatMessageStateDeliveredToUser:
		case LinphoneChatMessageStateNotDelivered:
			/*whichever of this state is reach, we delete the message, which means that we won't receive Displayed state*/
			linphone_chat_message_unref(msg);
		break;
		default:
		break;
	}
}

IncomingMessageEvent::IncomingMessageEvent(LinphoneChatMessage *msg) : Event("message-received"){
	ostringstream ostr;
	const LinphoneAddress *from = linphone_chat_message_get_from_address(msg);
	const LinphoneAddress *to = linphone_chat_message_get_to_address(msg);
	char *fromstr = linphone_address_as_string(from);
	char *tostr = linphone_address_as_string(to);
	ostr << "From: "<< fromstr << "\n";
	ostr << "To: "<< tostr << "\n";
	ostr << "Id: "<< linphone_chat_message_get_message_id(msg) << "\n";
	ostr << "Content: \"" << linphone_chat_message_get_text(msg) << "\"\n";
	setBody(ostr.str());
	ms_free(fromstr);
	ms_free(tostr);
}


