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

// clean
#include "chat/chat-message/chat-message.h"
#include "chat/chat-room/abstract-chat-room.h"
#include "content/content.h"
#include "conference/participant.h"
#include "conference/participant-p.h"
#include "lime-v2.h"
#include "private.h"
#include "content/content-manager.h"

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
	belle_sip_auth_event_set_username(event, "alice");
	belle_sip_auth_event_set_passwd(event, "you see the problem is this");
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
	belleSipLimeManager = unique_ptr<BelleSipLimeManager>(new BelleSipLimeManager(db_access, prov));
}

ChatMessageModifier::Result LimeV2::processOutgoingMessage (const shared_ptr<ChatMessage> &message, int &errorCode) {

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

	belleSipLimeManager->encrypt(localDeviceId, recipientUserId, recipients, plainMessage, cipherMessage, [recipients, cipherMessage, message] (lime::callbackReturn returnCode, string errorMessage) {
		cout << "INSIDE ENCRYPT SUCCESS LAMBDA FUNCTION" << endl;
		if (returnCode == lime::callbackReturn::success) {

			list<Content> contents;

			for (const auto &recipient : *recipients) {
				string cipherHeaderString(recipient.cipherHeader.begin(), recipient.cipherHeader.end());
				Content cipherHeaderContent;
				cipherHeaderContent.setBodyFromUtf8(cipherHeaderString);
				cipherHeaderContent.setContentType("application/lime");
				cipherHeaderContent.addHeader("Content-Id", recipient.deviceId);
				contents.push_back(move(cipherHeaderContent));
			}

			string cipherMessageString(cipherMessage->begin(), cipherMessage->end());
			Content cipherMessageContent;
			cipherMessageContent.setBodyFromUtf8(cipherMessageString);
			cipherMessageContent.setContentType("application/octet-stream");
			cipherMessageContent.addHeader("Content-Description", "Encrypted Message");
			contents.push_back(move(cipherMessageContent));

			Content finalContent = ContentManager::contentListToMultipart(contents);

			message->setInternalContent(finalContent);
			message->send();

			return ChatMessageModifier::Result::Done;
		} else {
			BCTBX_SLOGE << "Lime operation failed : " << errorMessage;
			return ChatMessageModifier::Result::Error;
		}
	});

	// Test errorCode
	return ChatMessageModifier::Result::Suspended;
}

ChatMessageModifier::Result LimeV2::processIncomingMessage (const shared_ptr<ChatMessage> &message, int &errorCode) {
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
			BCTBX_SLOGE << "LIME v2 : no content in received message";
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

bool LimeV2::encryptionEnabledForFileTransferCb (const shared_ptr<AbstractChatRoom> &chatRoom) {
	// Work in progress
	return false;
}

void LimeV2::generateFileTransferKeyCb (const shared_ptr<AbstractChatRoom> &chatRoom, const shared_ptr<ChatMessage> &message) {
	// Work in progress
}

int LimeV2::downloadingFileCb (const shared_ptr<ChatMessage> &message, size_t offset, const uint8_t *buffer, size_t size, uint8_t *decrypted_buffer) {
	// Work in progress
	return 0;
}

int LimeV2::uploadingFileCb (const shared_ptr<ChatMessage> &message, size_t offset, const uint8_t *buffer, size_t size, uint8_t *encrypted_buffer) {
	// Work in progress
	return 0;
}

void LimeV2::onNetworkReachable (bool sipNetworkReachable, bool mediaNetworkReachable) {
	// work in progress
}

void LimeV2::onRegistrationStateChanged (LinphoneProxyConfig *cfg, LinphoneRegistrationState state, const string &message) {
	if (state == LinphoneRegistrationState::LinphoneRegistrationOk) {
		// Create user
		const LinphoneAddress *contactAddr = linphone_proxy_config_get_contact(cfg);
		char *contactAddrStr = linphone_address_as_string_uri_only(contactAddr);
		IdentityAddress ia = IdentityAddress(contactAddrStr);
		bctbx_free(contactAddrStr);
		string localDeviceId = ia.getGruu();
		string x3dhServerUrl = "https://localhost:25519";
		lime::CurveId curve = lime::CurveId::c25519;
		lime::limeCallback callback([](lime::callbackReturn returnCode, std::string anythingToSay) {
				if (returnCode == lime::callbackReturn::success) {
					BCTBX_SLOGI << "Lime create user operation successful";
				} else {
					BCTBX_SLOGE << "Lime operation failed : " << anythingToSay;
				}
			});
		belleSipLimeManager->create_user(localDeviceId, x3dhServerUrl, curve, callback);
	}
}

LINPHONE_END_NAMESPACE
