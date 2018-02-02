/*
 * encryption-chat-message-modifier.cpp
 * Copyright (C) 2010-2018 Belledonne Communications SARL
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "chat/chat-message/chat-message.h"
#include "chat/chat-room/abstract-chat-room.h"
#include "content/content.h"
#include "conference/participant.h"
#include "conference/participant-p.h"
#include "lime-v2.h"
#include "private.h"

using namespace std;
//using namespace lime;

LINPHONE_BEGIN_NAMESPACE

// =============================== BELLE SIP LIME MANAGER

struct X3DHServerPostContext {
	const lime::limeX3DHServerResponseProcess responseProcess; // a callback to forward the response to lib lime
	const string username; // the username to provide corresponding credentials, not really in use in this test as the test server let us access any record with the same credentials
	X3DHServerPostContext(const lime::limeX3DHServerResponseProcess &response, const string &username) : responseProcess(response), username{username} {};
};

void BelleSipLimeManager::processIoError (void *data, const belle_sip_io_error_event_t *event) noexcept {
	X3DHServerPostContext *userData = static_cast<X3DHServerPostContext *>(data);
	(userData->responseProcess)(0, vector<uint8_t>{});
	delete(userData);
}

void BelleSipLimeManager::processResponse(void *data, const belle_http_response_event_t *event) noexcept {
	X3DHServerPostContext *userData = static_cast<X3DHServerPostContext *>(data);
	if (event->response){
		auto code=belle_http_response_get_status_code(event->response);
		belle_sip_message_t *message = BELLE_SIP_MESSAGE(event->response);
		// all raw data access functions in lime use uint8_t *, so safely cast the body pointer to it, it's just a data stream pointer anyway
		auto body = reinterpret_cast<const uint8_t *>(belle_sip_message_get_body(message));
		auto bodySize = belle_sip_message_get_body_size(message);
		(userData->responseProcess)(code, vector<uint8_t>{body, body+bodySize});
	} else {
		(userData->responseProcess)(0, vector<uint8_t>{});
	}
	delete(userData);
}

void BelleSipLimeManager::processAuthRequestedFromCarddavRequest(void *data, belle_sip_auth_event_t *event) noexcept {
	LinphoneCardDavQuery *query = (LinphoneCardDavQuery *)data;
	LinphoneCardDavContext *cdc = query->context;
	const char *realm = belle_sip_auth_event_get_realm(event);
	belle_generic_uri_t *uri = belle_generic_uri_parse(query->url);
	const char *domain = belle_generic_uri_get_host(uri);

	if (cdc->auth_info) {
		belle_sip_auth_event_set_username(event, cdc->auth_info->username);
		belle_sip_auth_event_set_passwd(event, cdc->auth_info->passwd);
		belle_sip_auth_event_set_ha1(event, cdc->auth_info->ha1);
	} else {
		BCTBX_SLOGE << "Could not get authentication info from CardDAV context";

		LinphoneCore *lc = cdc->friend_list->lc;
		const bctbx_list_t *auth_infos = linphone_core_get_auth_info_list(lc);

		ms_debug("Looking for auth info for domain %s and realm %s", domain, realm);
		while (auth_infos) {
			LinphoneAuthInfo *auth_info = (LinphoneAuthInfo *)auth_infos->data;
			if (auth_info->domain && strcmp(domain, auth_info->domain) == 0) {
				if (!auth_info->realm || strcmp(realm, auth_info->realm) == 0) {
					belle_sip_auth_event_set_username(event, auth_info->username);
					belle_sip_auth_event_set_passwd(event, auth_info->passwd);
					belle_sip_auth_event_set_ha1(event, auth_info->ha1);
					cdc->auth_info = linphone_auth_info_clone(auth_info);
					break;
				}
			}
			auth_infos = bctbx_list_next(auth_infos);
		}

		if (!auth_infos) {
			BCTBX_SLOGE << "CardDAV authentication error in lime v2";

// 			ms_error("[carddav] Authentication requested during CardDAV request sending, and username/password weren't provided");
// 			if (is_query_client_to_server_sync(query)) {
// 				linphone_carddav_client_to_server_sync_done(query->context, FALSE, "Authentication requested during CardDAV request sending, and username/password weren't provided");
// 			} else {
// 				linphone_carddav_server_to_client_sync_done(query->context, FALSE, "Authentication requested during CardDAV request sending, and username/password weren't provided");
// 			}
// 			linphone_carddav_query_free(query);
		}
	}
}

BelleSipLimeManager::BelleSipLimeManager(const string &db_access, belle_http_provider_t *prov) : LimeManager(db_access, [prov](const string &url, const string &from, const vector<uint8_t> &message, const lime::limeX3DHServerResponseProcess &responseProcess) {
	belle_http_request_listener_callbacks_t cbs= {};
	belle_http_request_listener_t *l;
	belle_generic_uri_t *uri;
	belle_http_request_t *req;
	belle_sip_memory_body_handler_t *bh;

	bh = belle_sip_memory_body_handler_new_copy_from_buffer(message.data(), message.size(), NULL, NULL);

	uri=belle_generic_uri_parse(url.data());

	req=belle_http_request_create("POST", uri,
			belle_http_header_create("User-Agent", "lime"),
			belle_http_header_create("Content-type", "x3dh/octet-stream"),
			belle_http_header_create("From", from.data()),
			NULL);

	belle_sip_message_set_body_handler(BELLE_SIP_MESSAGE(req),BELLE_SIP_BODY_HANDLER(bh));
	cbs.process_response = processResponse;
	cbs.process_io_error = processIoError;
	cbs.process_auth_requested = processAuthRequestedFromCarddavRequest;
	// store a reference to the responseProcess function in a wrapper as belle-sip request C-style callbacks with a void * user data parameter, C++ implementation shall
	// use lambda and capture the function.
	// this new creates on the heap a copy of the responseProcess closure, so we have access to it when called back by belle-sip
	// We also provide the username to be used to retrieve credentials when server ask for it
	X3DHServerPostContext *userData = new X3DHServerPostContext(responseProcess, from);
	l=belle_http_request_listener_create_from_callbacks(&cbs, userData);
	belle_sip_object_data_set(BELLE_SIP_OBJECT(req), "http_request_listener", l, belle_sip_object_unref); // Ensure the listener object is destroyed when the request is destroyed
	belle_http_provider_send_request(prov,req,l);
}) {
}

// =============================== LIME V2

LimeV2::LimeV2 (const string &db_access, belle_http_provider_t *prov) {
	//limeManager = unique_ptr<LimeManager>(new LimeManager(db_access, X3DHServerPost));
	belleSipLimeManager = unique_ptr<BelleSipLimeManager>(new BelleSipLimeManager(db_access, prov));
}

ChatMessageModifier::Result LimeV2::processOutgoingMessage (
	const shared_ptr<ChatMessage> &message,
	int &errorCode
) {
    shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();

	// Use the LimeManager to cipher the message

	/* const string &localDeviceId 						--> local GRUU
	 * shared_ptr<const string> recipientUserId 		--> sip:uri of user/conference
	 * shared_ptr<vector<recipientData>> recipients 	--> recipient device Id (GRUU) and an empty buffer to store the cipherHeader
	 * shared_ptr<const vector<uint8_t>> plainMessage 	--> message to be encrypted
	 * shared_ptr<vector<uint8_t>> cipherMessage 		--> buffer to store encrypted message
	 * const limeCallback &callback 					--> gives the exit status and an error message in case of failure
	 */

	// localDeviceId
	const string &localDeviceId = chatRoom->getMe()->getPrivate()->getDevices().front()->getAddress().getGruu(); // How to get the device we are using to send the message ?

	// recipientUserId
	const IdentityAddress &peerAddress = chatRoom->getPeerAddress(); // Check if it is correct to use IdentityAddress for recipientUserId
	shared_ptr<const string> recipientUserId = make_shared<const string>(peerAddress.getAddressWithoutGruu().asString()); // get the recipient user id (user or conference, not device id)

	// recipients
	auto recipients = make_shared<vector<lime::recipientData>>(); // create the recipients vector
	const list<shared_ptr<Participant>> participants = chatRoom->getParticipants(); // get the participants list from chatroom
	for (const shared_ptr<Participant> &p : participants) { // For each participant
		const list<shared_ptr<ParticipantDevice>> devices = p->getPrivate()->getDevices(); // get the device list from participant
		for (const shared_ptr<ParticipantDevice> &pd : devices) { // For each device
			recipients->emplace_back(pd->getAddress().getGruu()); // get the GRUU and add it to the recipients list
		}
	}

	// plainMessage
	const string &plainStringMessage = message->getInternalContent().getBodyAsUtf8String(); // get utf8 string but we need a shared_ptr<const vector<uint8_t>>
	shared_ptr<const vector<uint8_t>> plainMessage = make_shared<const vector<uint8_t>>(plainStringMessage.begin(), plainStringMessage.end()); // obtain the message to be encrypted

	// cipherMessage
	auto cipherMessage = make_shared<vector<uint8_t>>(); // an empty buffer to get the encrypted message

	belleSipLimeManager->encrypt(localDeviceId, recipientUserId, recipients, plainMessage, cipherMessage,
						 // callback lambda --> create a real callback that can be applied everywhere
						 [recipients, cipherMessage] (lime::callbackReturn returnCode, string errorMessage) {
						// counters is related to this test environment only, not to be considered for real usage
						if (returnCode == lime::callbackReturn::success) {
							// here is the code processing the output when all went well.
							// Send the message to recipient
							// that function must, before returning, send or copy the data to send them later
							// recipients and cipherMessage are likely to be be destroyed as soon as we get out of this closure
							// In this exanple we know that bodDevice is in recipients[0], real code shall loop on recipients vector
							//sendMessageTo("bob", (*recipients)[0].cipherHeader, *cipherMessage);
							return ChatMessageModifier::Result::Done;
						} else {
							// The encryption failed.
							BCTBX_SLOGE << "Lime operation failed : " << errorMessage;
							return ChatMessageModifier::Result::Error;
						}
					});

	// Test errorCode
	return ChatMessageModifier::Result::Skipped;
}

ChatMessageModifier::Result LimeV2::processIncomingMessage (
	const shared_ptr<ChatMessage> &message,
	int &errorCode
) {
	const shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();

	// Use the LimeManager to decipher the message

	/* const string &localDeviceId 				--> local GRUU
	 * const string &recipientUserId 			--> sip:uri of user/conference
	 * const string &senderDeviceId 			--> "sender's GRUU"
	 * const vector<uint8_t> &cipherHeader 		--> the part of cipher which is targeted to current device
	 * const vector<uint8_t> &cipherMessage 	--> part of cipher routed to all recipient devices
	 * vector<uint8_t> &plainMessage 			--> output buffer
	 */

	// localDeviceId
	const string &localDeviceId = chatRoom->getMe()->getPrivate()->getDevices().front()->getAddress().getGruu();

	// recipientUserId
	const string &recipientUserId = chatRoom->getLocalAddress().getAddressWithoutGruu().asString(); // Get local user id

	// senderDeviceId
	const string &senderDeviceId = chatRoom->getParticipants().front()->getPrivate()->getDevices().front()->getAddress().getGruu(); // Get sender user id

	// cipherHeader
	const vector<uint8_t> cipherHeader;

	// cipherMessage
	if (message->getInternalContent().isEmpty()) {
		if (message->getContents().front()->isEmpty()) {
			// return ChatMessageModifier::Result::Error;
			// BC_FAIL() ?
			// errorCode ?
			// throw exception ?
		}
		const string &cipherStringMessage = message->getContents().front()->getBodyAsUtf8String(); // utf8_string to vector<uint8_t>
		const vector<uint8_t> &cipherMessage = vector<uint8_t>(cipherStringMessage.begin(), cipherStringMessage.end());
	}
	const string &cipherStringMessage = message->getInternalContent().getBodyAsUtf8String(); // utf8_string to vector<uint8_t>
	const vector<uint8_t> &cipherMessage = vector<uint8_t>(cipherStringMessage.begin(), cipherStringMessage.end());

	// plainMessage
	vector<uint8_t> plainMessage{};

	belleSipLimeManager->decrypt(localDeviceId, recipientUserId, senderDeviceId, cipherHeader, cipherMessage, plainMessage);

	// Test errorCode
	return ChatMessageModifier::Result::Skipped;
}

bool LimeV2::encryptionEnabledForFileTransferCb (
	const shared_ptr<AbstractChatRoom> &chatRoom
) {
	// Work in progress
	return false;
}

void LimeV2::generateFileTransferKeyCb (
	const shared_ptr<AbstractChatRoom> &chatRoom,
	const shared_ptr<ChatMessage> &message
) {
	// Work in progress
}

int LimeV2::downloadingFileCb (
	const shared_ptr<ChatMessage> &message,
	size_t offset,
	const uint8_t *buffer,
	size_t size,
	uint8_t *decrypted_buffer
) {
	// Work in progress
	return 0;
}

int LimeV2::uploadingFileCb (
	const shared_ptr<ChatMessage> &message,
	size_t offset,
	const uint8_t *buffer,
	size_t size,
	uint8_t *encrypted_buffer
) {
	// Work in progress
	return 0;
}

LINPHONE_END_NAMESPACE
