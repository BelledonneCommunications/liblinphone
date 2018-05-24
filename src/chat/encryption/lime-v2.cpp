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
	LinphoneCore *lc;
	X3DHServerPostContext(const lime::limeX3DHServerResponseProcess &response, const string &username, LinphoneCore *lc) : responseProcess(response), username{username}, lc{lc} {};
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

void BelleSipLimeManager::processAuthRequested (void *data, belle_sip_auth_event_t *event) noexcept {
	X3DHServerPostContext *userData = static_cast<X3DHServerPostContext *>(data);
	LinphoneCore *lc = userData->lc;

	const char *realm = belle_sip_auth_event_get_realm(event);
	const char *username = belle_sip_auth_event_get_username(event);
	const char *domain = belle_sip_auth_event_get_domain(event);

// 	printf("[LIMEv2] finding auth_info for:\nrealm = %s\nusername = %s\ndomain = %s\n", realm, username, domain);
	const LinphoneAuthInfo *auth_info = linphone_core_find_auth_info(lc, realm, username, domain);

	if (auth_info) {
		const char *auth_username = linphone_auth_info_get_username(auth_info);
		const char *auth_password = linphone_auth_info_get_password(auth_info);
// 		printf("setting auth event username = %s\n", auth_username);
// 		printf("setting auth event password = %s\n", auth_password);
		belle_sip_auth_event_set_username(event, auth_username);
		belle_sip_auth_event_set_passwd(event, auth_password);
	}
}

BelleSipLimeManager::BelleSipLimeManager (const string &db_access, belle_http_provider_t *prov, LinphoneCore *lc) : LimeManager(db_access, [prov, lc](const string &url, const string &from, const vector<uint8_t> &message, const lime::limeX3DHServerResponseProcess &responseProcess) {
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
	cbs.process_auth_requested = processAuthRequested;
	X3DHServerPostContext *userData = new X3DHServerPostContext(responseProcess, from, lc);
	l=belle_http_request_listener_create_from_callbacks(&cbs, userData);
	belle_sip_object_data_set(BELLE_SIP_OBJECT(req), "http_request_listener", l, belle_sip_object_unref);
	belle_http_provider_send_request(prov,req,l);
}) {
}

LimeV2::LimeV2 (const std::__cxx11::string &db_access, belle_http_provider_t *prov, LinphoneCore *lc) {
	x3dhServerUrl = linphone_config_get_string(linphone_core_get_config(lc), "misc", "x3dh_server_url", "");
	curve = lime::CurveId::c25519; // c448
	belleSipLimeManager = unique_ptr<BelleSipLimeManager>(new BelleSipLimeManager(db_access, prov, lc));
	lastLimeUpdate = linphone_config_get_int(lc->config, "misc", "last_lime_update_time", 0);
}

string LimeV2::getX3dhServerUrl () const {
	return x3dhServerUrl;
}

lime::CurveId LimeV2::getCurveId () const {
	return curve;
}

ChatMessageModifier::Result LimeV2::processOutgoingMessage (const shared_ptr<ChatMessage> &message, int &errorCode) {
	ChatMessageModifier::Result result = ChatMessageModifier::Result::Suspended;

    shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();
	const string &localDeviceId = chatRoom->getMe()->getPrivate()->getDevices().front()->getAddress().asString();
	const IdentityAddress &peerAddress = chatRoom->getPeerAddress();
	shared_ptr<const string> recipientUserId = make_shared<const string>(peerAddress.getAddressWithoutGruu().asString());

	auto recipients = make_shared<vector<lime::RecipientData>>();
	const list<shared_ptr<Participant>> participants = chatRoom->getParticipants();
	for (const shared_ptr<Participant> &p : participants) {
		const list<shared_ptr<ParticipantDevice>> devices = p->getPrivate()->getDevices();
		for (const shared_ptr<ParticipantDevice> &pd : devices) {
			recipients->emplace_back(pd->getAddress().asString());
		}
	}

	const string &plainStringMessage = message->getInternalContent().getBodyAsUtf8String();
	shared_ptr<const vector<uint8_t>> plainMessage = make_shared<const vector<uint8_t>>(plainStringMessage.begin(), plainStringMessage.end());
	shared_ptr<vector<uint8_t>> cipherMessage = make_shared<vector<uint8_t>>();

	belleSipLimeManager->encrypt(localDeviceId, recipientUserId, recipients, plainMessage, cipherMessage, [recipients, cipherMessage, message, &result] (lime::CallbackReturn returnCode, string errorMessage) {
		if (returnCode == lime::CallbackReturn::success) {

			list<Content> contents;

			// ---------------------------------------------- HEADER

			for (const auto &recipient : *recipients) {

				vector<uint8_t> encodedCipher = encodeBase64(recipient.DRmessage);
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
	}, lime::EncryptionPolicy::cipherMessage);

	return result;
}

ChatMessageModifier::Result LimeV2::processIncomingMessage (const shared_ptr<ChatMessage> &message, int &errorCode) {
	const shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();

	const string &localDeviceId = chatRoom->getMe()->getPrivate()->getDevices().front()->getAddress().asString(); // TODO cfg->contact_address
	const string &recipientUserId = chatRoom->getPeerAddress().getAddressWithoutGruu().asString();
	const string &senderDeviceId = chatRoom->getParticipants().front()->getPrivate()->getDevices().front()->getAddress().asString(); // TODO find the correct participant

	// debug
	auto participants = chatRoom->getParticipants();
	list<shared_ptr<ParticipantDevice>> devices;
	cout << endl << "[" << chatRoom->getMe()->getPrivate()->getDevices().front()->getAddress().getUsername() << "]" << endl;
	cout << "list of participants:" << endl;
	for (const auto &participant : participants) {
		devices = participant->getPrivate()->getDevices();
		for (const auto &device : devices) {
			cout << device->getAddress().asString() << endl;
		}
	}
	const string &from_address = message->getFromAddress().asString();

	cout << "From address = " << from_address << endl;
	cout << "senderDeviceId = " << senderDeviceId << endl;

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
	} catch (const exception &e) {
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
	// TODO Work in progress

	// get the callback telling whether or not to encrypt the files ?

	// default value ?

	// expected behaviour ?
	return false;
}

void LimeV2::generateFileTransferKeyCb (const shared_ptr<AbstractChatRoom> &chatRoom, const shared_ptr<ChatMessage> &message) {
	// TODO Work in progress

	/** in LIMEv1
	 *
	char keyBuffer [FILE_TRANSFER_KEY_SIZE]; // temporary storage of generated key: 192 bits of key + 64 bits of initial vector
	// generate a random 192 bits key + 64 bits of initial vector and store it into the file_transfer_information->key field of the msg
	sal_get_random_bytes((unsigned char *)keyBuffer, FILE_TRANSFER_KEY_SIZE);
	LinphoneContent *content = linphone_chat_message_get_file_transfer_information(msg);
	if (!content)
		return;
	linphone_content_set_key(content, keyBuffer, FILE_TRANSFER_KEY_SIZE); // key is duplicated in the content private structure
	linphone_content_unref(content);
	*/
}

int LimeV2::downloadingFileCb (const shared_ptr<ChatMessage> &message, size_t offset, const uint8_t *buffer, size_t size, uint8_t *decrypted_buffer) {
	// TODO Work in progress

	// get encrypted buffer from content

	// check file transfer key

	// decrypt encrypted buffer

	// return result code

	//message->isFileTransferInProgress();
	//message->downloadFile();
	//message->cancelFileTransfer();

	// see lime_im_encryption_engine_process_downloading_file_cb in lime.c

	return 0;
}

int LimeV2::uploadingFileCb (const shared_ptr<ChatMessage> &message, size_t offset, const uint8_t *buffer, size_t size, uint8_t *encrypted_buffer) {
	// TODO Work in progress

	// get file transfer key

	// encrypt content

	// check encrypted content size

	// return result code

	// see lime_im_encryption_engine_process_downloading_file_cb in lime.c

	return 0;
}

void LimeV2::onNetworkReachable (bool sipNetworkReachable, bool mediaNetworkReachable) {
	// work in progress
}

std::shared_ptr<BelleSipLimeManager> LimeV2::getLimeManager () {
	return belleSipLimeManager;
}

lime::limeCallback LimeV2::setLimeCallback (string operation) {
	lime::limeCallback callback([operation](lime::CallbackReturn returnCode, string anythingToSay) {
		if (returnCode == lime::CallbackReturn::success) {
			BCTBX_SLOGI << "Lime operation successful: " << operation;
		} else {
			BCTBX_SLOGE << "Lime operation failed: " << anythingToSay;
		}
	});
	return callback;
}

void LimeV2::onRegistrationStateChanged (LinphoneProxyConfig *cfg, LinphoneRegistrationState state, const string &message) {
	if (state == LinphoneRegistrationState::LinphoneRegistrationOk) {

		IdentityAddress ia = IdentityAddress(linphone_address_as_string_uri_only(linphone_proxy_config_get_contact(cfg)));
		string localDeviceId = ia.asString();

// 		const LinphoneAddress *addr = linphone_proxy_config_get_identity_address(cfg);
// 		cout << "[Register] domain = " << linphone_address_get_domain(addr) << endl;
// 		localDeviceId = linphone_address_as_string(addr);

		// TODO may not be necessary
// 		if (localDeviceId == "")
// 			return;

		stringstream operation;
		operation << "create user " << localDeviceId;
		lime::limeCallback callback = setLimeCallback(operation.str());

		LinphoneConfig *lpconfig = linphone_core_get_config(linphone_proxy_config_get_core(cfg));
		lastLimeUpdate = linphone_config_get_int(lpconfig, "misc", "last_lime_update_time", -1); // TODO should be done by the tester

		try {
			// create user if not exist
			cout << "[Register] trying to create lime user = " << localDeviceId << endl;
			belleSipLimeManager->create_user(localDeviceId, x3dhServerUrl, curve, callback);
			lastLimeUpdate = ms_time(NULL);
			lp_config_set_int(lpconfig, "misc", "last_lime_update_time", (int)lastLimeUpdate);
		} catch (const exception &e) {
			cout << "[Register] user already exists" << endl;
			ms_message("%s while creating lime user\n", e.what());

			// update keys if necessary
			int limeUpdateThreshold = lp_config_get_int(lpconfig, "misc", "lime_update_threshold", 86400);
			if (ms_time(NULL) - lastLimeUpdate > limeUpdateThreshold) { // 24 hours = 86400 ms
				update(lpconfig);
				lastLimeUpdate = ms_time(NULL);
			} else {
			}
		}
	}
}

LINPHONE_END_NAMESPACE
