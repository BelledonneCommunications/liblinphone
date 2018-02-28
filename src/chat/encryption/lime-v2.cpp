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

#include "bctoolbox/crypto.h"
#include "chat/chat-message/chat-message.h"
#include "chat/chat-room/abstract-chat-room.h"
#include "content/content.h"
#include "content/content-manager.h"
#include "content/content-type.h"
#include "conference/participant.h"
#include "conference/participant-p.h"
#include "lime-v2.h"
#include "private.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

struct X3DHServerPostContext {
	const lime::limeX3DHServerResponseProcess responseProcess;
	const string username;
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
	X3DHServerPostContext *userData = new X3DHServerPostContext(responseProcess, from);
	l=belle_http_request_listener_create_from_callbacks(&cbs, userData);
	belle_sip_object_data_set(BELLE_SIP_OBJECT(req), "http_request_listener", l, belle_sip_object_unref);
	belle_http_provider_send_request(prov,req,l);
}) {
}

LimeV2::LimeV2 (const string &db_access, belle_http_provider_t *prov) {
	belleSipLimeManager = unique_ptr<BelleSipLimeManager>(new BelleSipLimeManager(db_access, prov));
}

ChatMessageModifier::Result LimeV2::processOutgoingMessage (const shared_ptr<ChatMessage> &message, int &errorCode) {

    shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();

	// localDeviceId
	const string &localDeviceId = chatRoom->getMe()->getPrivate()->getDevices().front()->getAddress().getGruu();

	// recipientUserId
	const IdentityAddress &peerAddress = chatRoom->getPeerAddress();
	shared_ptr<const string> recipientUserId = make_shared<const string>(peerAddress.getAddressWithoutGruu().asString());

	// recipients
	auto recipients = make_shared<vector<lime::recipientData>>();
	const list<shared_ptr<Participant>> participants = chatRoom->getParticipants();
	for (const shared_ptr<Participant> &p : participants) {
		const list<shared_ptr<ParticipantDevice>> devices = p->getPrivate()->getDevices();
		for (const shared_ptr<ParticipantDevice> &pd : devices) {
			recipients->emplace_back(pd->getAddress().getGruu());
		}
	}

	// plainMessage
	const string &plainStringMessage = message->getInternalContent().getBodyAsUtf8String();
	shared_ptr<const vector<uint8_t>> plainMessage = make_shared<const vector<uint8_t>>(plainStringMessage.begin(), plainStringMessage.end());

	// cipherMessage
	shared_ptr<vector<uint8_t>> cipherMessage = make_shared<vector<uint8_t>>();

	belleSipLimeManager->encrypt(localDeviceId, recipientUserId, recipients, plainMessage, cipherMessage, [recipients, cipherMessage, message] (lime::callbackReturn returnCode, string errorMessage) {
		if (returnCode == lime::callbackReturn::success) {

			list<Content> contents;

			// ---------------------------------------------- HEADER

			for (const auto &recipient : *recipients) {

				// base64 encode header
				vector<char> cipherHeader(recipient.cipherHeader.begin(), recipient.cipherHeader.end());
				const unsigned char *input_buffer = recipient.cipherHeader.data();
				size_t input_length = cipherHeader.size();
				size_t encoded_length = 0;
				bctbx_base64_encode(NULL, &encoded_length, input_buffer, input_length);				// set encoded_length to the correct value
				unsigned char *encoded_buffer = new unsigned char[encoded_length];					// allocate encoded buffer with correct length
				bctbx_base64_encode(encoded_buffer, &encoded_length, input_buffer, input_length);	// real encoding
				vector<uint8_t> encodedCipher(encoded_buffer, encoded_buffer + encoded_length);
				vector<char> cipherHeaderB64(encodedCipher.begin(), encodedCipher.end());

				Content cipherHeaderContent;
				cipherHeaderContent.setBody(cipherHeaderB64);
				cipherHeaderContent.setContentType("application/lime");
				cipherHeaderContent.addHeader("Content-Id", recipient.deviceId);
				contents.push_back(move(cipherHeaderContent));

				delete[] encoded_buffer;
			}

			// ---------------------------------------------- MESSAGE

			// base64 encode message
			const unsigned char *input_buffer = cipherMessage->data();
			size_t input_length = cipherMessage->size();
			size_t encoded_length = 0;
			bctbx_base64_encode(NULL, &encoded_length, input_buffer, input_length);					// set encoded_length to the correct value
			unsigned char *encoded_buffer = new unsigned char[encoded_length];						// allocate encoded buffer with correct length
			bctbx_base64_encode(encoded_buffer, &encoded_length, input_buffer, input_length);		// real encoding
			vector<uint8_t> encodedCipher(encoded_buffer, encoded_buffer + encoded_length);
			vector<char> cipherMessageB64(encodedCipher.begin(), encodedCipher.end());

			Content cipherMessageContent;
			cipherMessageContent.setBody(cipherMessageB64);
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

	// localDeviceId
	const string &localDeviceId = chatRoom->getMe()->getPrivate()->getDevices().front()->getAddress().getGruu();

	// recipientUserId
	const string &recipientUserId = chatRoom->getPeerAddress().getAddressWithoutGruu().asString();

	// senderDeviceId
	const string &senderDeviceId = chatRoom->getParticipants().front()->getPrivate()->getDevices().front()->getAddress().getGruu();

	Content content;
	ContentType contentType = ContentType::Multipart;
	string boundary = "boundary=-----------------------------14737809831466499882746641449";
	contentType.setParameter(boundary);
	content.setContentType(contentType);

	// cipherMessage
	if (message->getInternalContent().isEmpty()) {
		cout << "LIMEv2 ERROR : no internal content" << endl;
		if (message->getContents().front()->isEmpty()) {
			BCTBX_SLOGE << "LIMEv2 : no content in received message";
		}
	}

	vector<char> cipherBody(message->getInternalContent().getBody());
	content.setBody(cipherBody);

	list<Content> contentList = ContentManager::multipartToContentList(content);

	// With lambdas
	const vector<uint8_t> &cipherHeader = [&]() {
		for (const auto &content : contentList) {
			if (content.getContentType().getSubType() == "lime") {
				const vector<uint8_t> &cipherHeader = vector<uint8_t>(content.getBody().begin(), content.getBody().end());
				return cipherHeader;
			}
		}
		BCTBX_SLOGE << "LIMEv2 : unexpected subtype : " << content.getContentType().getSubType();
		//TODO return nothing or null value
		const vector<uint8_t> &cipherHeader = vector<uint8_t>(content.getBody().begin(), content.getBody().end());
		return cipherHeader;
	}();

	const vector<uint8_t> &cipherMessage = [&]() {
		for (const auto &content : contentList) {
			if (content.getContentType().getSubType() == "octet-stream") {
				const vector<uint8_t> &cipherMessage = vector<uint8_t>(content.getBody().begin(), content.getBody().end());
				return cipherMessage;
			}
		}
		BCTBX_SLOGE << "LIMEv2 : unexpected subtype : " << content.getContentType().getSubType();
		//TODO return nothing or null value
		const vector<uint8_t> &cipherMessage = vector<uint8_t>(content.getBody().begin(), content.getBody().end());
		return cipherMessage;
	}();

	// base64 decode header
	const unsigned char* encodedHeader = cipherHeader.data();
	size_t encodedLength = cipherHeader.size();
	size_t decodedLength = 0;
	bctbx_base64_decode(NULL, &decodedLength, encodedHeader, encodedLength);				// set decodedLength to the correct value
	unsigned char *decodedHeader = new unsigned char[decodedLength];						// allocate decoded buffer with correct length
	bctbx_base64_decode(decodedHeader, &decodedLength, encodedHeader, encodedLength);		// real decoding
	vector<uint8_t> decodedCipherHeader(decodedHeader, decodedHeader + decodedLength);
	delete[] decodedHeader;

	// base64 decode message
	const unsigned char* encodedMessage = cipherMessage.data();
	encodedLength = cipherMessage.size();
	decodedLength = 0;
	bctbx_base64_decode(NULL, &decodedLength, encodedMessage, encodedLength);				// set decodedLength to the correct value
	unsigned char *decodedMessage = new unsigned char[decodedLength];						// allocate decoded buffer with correct length
	bctbx_base64_decode(decodedMessage, &decodedLength, encodedMessage, encodedLength);		// real decoding
	vector<uint8_t> decodedCipherMessage(decodedMessage, decodedMessage + decodedLength);
	delete[] decodedMessage;

	// plainMessage
	vector<uint8_t> plainMessage{};

	belleSipLimeManager->decrypt(localDeviceId, recipientUserId, senderDeviceId, decodedCipherHeader, decodedCipherMessage, plainMessage);

	string plainMessageString(plainMessage.begin(), plainMessage.end());

	Content finalContent;
	ContentType finalContentType = ContentType::Cpim;
	finalContent.setContentType(finalContentType);
	finalContent.setBodyFromUtf8(plainMessageString);
	message->setInternalContent(finalContent);

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
