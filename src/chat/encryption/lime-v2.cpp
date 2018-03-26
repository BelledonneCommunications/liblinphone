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

#include "chat/chat-room/abstract-chat-room.h"
#include "content/content-manager.h"
#include "conference/participant-p.h"
#include "lime-v2.h"
#include "private.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

static vector<uint8_t> encodeBase64 (const vector<uint8_t> &input) {
	const unsigned char *inputBuffer = input.data();
	size_t inputLength = input.size();
	size_t encodedLength = 0;
	bctbx_base64_encode(NULL, &encodedLength, inputBuffer, inputLength);			// set encodedLength to the correct value
	unsigned char* encodedBuffer = new unsigned char[encodedLength];				// allocate encoded buffer with correct length
	bctbx_base64_encode(encodedBuffer, &encodedLength, inputBuffer, inputLength);	// real encoding
	vector<uint8_t> output(encodedBuffer, encodedBuffer + encodedLength);
	delete[] encodedBuffer;
	return output;
}

static vector<uint8_t> decodeBase64 (const vector<uint8_t> &input) {
	const unsigned char *inputBuffer = input.data();
	size_t inputLength = input.size();
	size_t decodedLength = 0;
	bctbx_base64_decode(NULL, &decodedLength, inputBuffer, inputLength);			// set decodedLength to the correct value
	unsigned char* decodedBuffer = new unsigned char[decodedLength];				// allocate decoded buffer with correct length
	bctbx_base64_decode(decodedBuffer, &decodedLength, inputBuffer, inputLength);	// real decoding
	vector<uint8_t> output(decodedBuffer, decodedBuffer + decodedLength);
	delete[] decodedBuffer;
	return output;
}

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

void BelleSipLimeManager::processResponse (void *data, const belle_http_response_event_t *event) noexcept {
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

void BelleSipLimeManager::processAuthRequestedFromCarddavRequest (void *data, belle_sip_auth_event_t *event) noexcept {
	// In real life situation, get the real username and password of user for authentication
	belle_sip_auth_event_set_username(event, "alice");
	belle_sip_auth_event_set_passwd(event, "you see the problem is this");
}

BelleSipLimeManager::BelleSipLimeManager (const string &db_access, belle_http_provider_t *prov) : LimeManager(db_access, [prov](const string &url, const string &from, const vector<uint8_t> &message, const lime::limeX3DHServerResponseProcess &responseProcess) {
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

LimeV2::LimeV2(const std::__cxx11::string &db_access, belle_http_provider_t *prov, LinphoneCore *lc) {
	// TODO get x3dhServerUrl and curve from application level
	x3dhServerUrl = "https://localhost:25519"; // 25520
	curve = lime::CurveId::c25519; // c448
	belleSipLimeManager = unique_ptr<BelleSipLimeManager>(new BelleSipLimeManager(db_access, prov));
	lastLimeUpdate = linphone_config_get_int(lc->config, "misc", "last_lime_update_time", 0);
}

ChatMessageModifier::Result LimeV2::processOutgoingMessage (const shared_ptr<ChatMessage> &message, int &errorCode) {

	ChatMessageModifier::Result result = ChatMessageModifier::Result::Suspended;

    shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();
	const string &localDeviceId = chatRoom->getMe()->getPrivate()->getDevices().front()->getAddress().getGruu(); // front() is approximative
	const IdentityAddress &peerAddress = chatRoom->getPeerAddress();
	shared_ptr<const string> recipientUserId = make_shared<const string>(peerAddress.getAddressWithoutGruu().asString());

	auto recipients = make_shared<vector<lime::recipientData>>();
	const list<shared_ptr<Participant>> participants = chatRoom->getParticipants();
	for (const shared_ptr<Participant> &p : participants) {
		const list<shared_ptr<ParticipantDevice>> devices = p->getPrivate()->getDevices();
		for (const shared_ptr<ParticipantDevice> &pd : devices) {
			recipients->emplace_back(pd->getAddress().getGruu());
		}
	}

	const string &plainStringMessage = message->getInternalContent().getBodyAsUtf8String();
	shared_ptr<const vector<uint8_t>> plainMessage = make_shared<const vector<uint8_t>>(plainStringMessage.begin(), plainStringMessage.end());
	shared_ptr<vector<uint8_t>> cipherMessage = make_shared<vector<uint8_t>>();

	belleSipLimeManager->encrypt(localDeviceId, recipientUserId, recipients, plainMessage, cipherMessage, [recipients, cipherMessage, message, &result] (lime::callbackReturn returnCode, string errorMessage) {
		if (returnCode == lime::callbackReturn::success) {

			list<Content> contents;

			// ---------------------------------------------- HEADER

			for (const auto &recipient : *recipients) {

				vector<uint8_t> encodedCipher = encodeBase64(recipient.cipherHeader);
				vector<char> cipherHeaderB64(encodedCipher.begin(), encodedCipher.end());

				Content cipherHeaderContent;
				cipherHeaderContent.setBody(cipherHeaderB64);
				cipherHeaderContent.setContentType(ContentType::LimeKey);
				cipherHeaderContent.addHeader("Content-Id", recipient.deviceId);
				stringstream contentDescription;
				contentDescription << "Key for " << message->getToAddress().asString();
				cipherHeaderContent.addHeader("Content-Description", contentDescription.str());
				contents.push_back(move(cipherHeaderContent));
			}

			// ---------------------------------------------- MESSAGE

			const vector<uint8_t> *binaryCipherMessage = cipherMessage.get();
			vector<uint8_t> encodedMessage = encodeBase64(*binaryCipherMessage);
			vector<char> cipherMessageB64(encodedMessage.begin(), encodedMessage.end());

			Content cipherMessageContent;
			cipherMessageContent.setBody(cipherMessageB64);
			cipherMessageContent.setContentType(ContentType::OctetStream);
			cipherMessageContent.addHeader("Content-Description", "Encrypted Message");
			contents.push_back(move(cipherMessageContent));

			Content finalContent = ContentManager::contentListToMultipart(contents);

			message->setInternalContent(finalContent);
			message->send();
			result = ChatMessageModifier::Result::Done;
		} else {
			BCTBX_SLOGE << "Lime operation failed: " << errorMessage;
			result = ChatMessageModifier::Result::Error;
		}
	});

	return result;
}

ChatMessageModifier::Result LimeV2::processIncomingMessage (const shared_ptr<ChatMessage> &message, int &errorCode) {

	const shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();

	const string &localDeviceId = chatRoom->getMe()->getPrivate()->getDevices().front()->getAddress().getGruu();
	const string &recipientUserId = chatRoom->getPeerAddress().getAddressWithoutGruu().asString();
	const string &senderDeviceId = chatRoom->getParticipants().front()->getPrivate()->getDevices().front()->getAddress().getGruu();

	Content content;
	ContentType contentType = ContentType::Multipart;
	contentType.setParameter("boundary=-----------------------------14737809831466499882746641449");
	content.setContentType(contentType);

	if (message->getInternalContent().isEmpty()) {
		BCTBX_SLOGE << "LIMEv2: no internal content";
		if (message->getContents().front()->isEmpty()) {
			BCTBX_SLOGE << "LIMEv2: no content in received message";
		}
		vector<char> cipherBody(message->getContents().front()->getBody());
	}
	vector<char> cipherBody(message->getInternalContent().getBody());
	content.setBody(cipherBody);

	list<Content> contentList = ContentManager::multipartToContentList(content);

	// ---------------------------------------------- HEADER

	const vector<uint8_t> &cipherHeader = [&]() {
		for (const auto &content : contentList) {
			if (content.getContentType() != ContentType::LimeKey)
				continue;

			for (const auto &header : content.getHeaders()) {
				if (header.second == localDeviceId) {
					const vector<uint8_t> &cipherHeader = vector<uint8_t>(content.getBody().begin(), content.getBody().end());
					return cipherHeader;
				}
			}
		}
		BCTBX_SLOGE << "LIMEv2: unexpected content-type: " << content.getContentType().asString();
		//TODO return nothing or null value
		const vector<uint8_t> &cipherHeader = vector<uint8_t>(content.getBody().begin(), content.getBody().end());
		return cipherHeader;
	}();

	// ---------------------------------------------- MESSAGE

	const vector<uint8_t> &cipherMessage = [&]() {
		for (const auto &content : contentList) {
			if (content.getContentType() == ContentType::OctetStream) {
				const vector<uint8_t> &cipherMessage = vector<uint8_t>(content.getBody().begin(), content.getBody().end());
				return cipherMessage;
			}
		}
		BCTBX_SLOGE << "LIMEv2: unexpected content-type: " << content.getContentType().asString();
		//TODO return nothing or null value
		const vector<uint8_t> &cipherMessage = vector<uint8_t>(content.getBody().begin(), content.getBody().end());
		return cipherMessage;
	}();

	vector<uint8_t> decodedCipherHeader = decodeBase64(cipherHeader);
	vector<uint8_t> decodedCipherMessage = decodeBase64(cipherMessage);
	vector<uint8_t> plainMessage{};

	bool decryptResult = false;
	try {
		decryptResult = belleSipLimeManager->decrypt(localDeviceId, recipientUserId, senderDeviceId, decodedCipherHeader, decodedCipherMessage, plainMessage);
	} catch (const exception e) {
		ms_message("%s while decrypting message\n", e.what());
	}

	if (!decryptResult)
		return ChatMessageModifier::Result::Error;

	string plainMessageString(plainMessage.begin(), plainMessage.end());

	Content finalContent;
	ContentType finalContentType = ContentType::Cpim; // TODO should be the content-type of the decrypted message
	finalContent.setContentType(finalContentType);
	finalContent.setBodyFromUtf8(plainMessageString);
	message->setInternalContent(finalContent);

	// Test errorCode
	return ChatMessageModifier::Result::Done;
}

void LimeV2::update (LinphoneConfig *lpconfig) {

	lime::limeCallback callback = setLimeCallback("Keys update");
	belleSipLimeManager->update(callback);
	lp_config_set_int(lpconfig, "misc", "last_lime_update_time", (int)lastLimeUpdate);
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

std::shared_ptr<BelleSipLimeManager> LimeV2::getLimeManager () {
	return belleSipLimeManager;
}

lime::limeCallback LimeV2::setLimeCallback (string operation) {
	lime::limeCallback callback([operation](lime::callbackReturn returnCode, string anythingToSay) {
		if (returnCode == lime::callbackReturn::success) {
			BCTBX_SLOGI << "Lime operation successful: " << operation;
		} else {
			BCTBX_SLOGE << "Lime operation failed: " << anythingToSay;
		}
	});
	return callback;
}

void LimeV2::onRegistrationStateChanged (LinphoneProxyConfig *cfg, LinphoneRegistrationState state, const string &message) {
	if (state == LinphoneRegistrationState::LinphoneRegistrationOk) {

		string localDeviceId = IdentityAddress(linphone_address_as_string_uri_only(linphone_proxy_config_get_contact(cfg))).getGruu();

		if (localDeviceId == "")
		return;

		stringstream operation;
		operation << "create user " << localDeviceId;
		lime::limeCallback callback = setLimeCallback(operation.str());

		LinphoneConfig *lpconfig = linphone_core_get_config(linphone_proxy_config_get_core(cfg)); // difference with lc->config ?
		lastLimeUpdate = linphone_config_get_int(lpconfig, "misc", "last_lime_update_time", -1); // TODO should be done by the tester

		try {
			// create user if not exist
			belleSipLimeManager->create_user(localDeviceId, x3dhServerUrl, curve, callback);
			lastLimeUpdate = ms_time(NULL);
			lp_config_set_int(lpconfig, "misc", "last_lime_update_time", (int)lastLimeUpdate);
		} catch (const exception e) {
			ms_message("%s while creating lime user\n", e.what());

			// update keys if necessary
			if (ms_time(NULL) - lastLimeUpdate > 86400) { // 24 hours = 86400 ms
				update(lpconfig);
				lastLimeUpdate = ms_time(NULL);
			} else {
			}
		}
	}
}

LINPHONE_END_NAMESPACE
