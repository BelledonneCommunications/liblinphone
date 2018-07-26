/*
 * lime-v2.cpp
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

#include "chat/chat-message/chat-message-p.h"
#include "chat/chat-room/chat-room.h"
#include "content/content-manager.h"
#include "content/header/header-param.h"
#include "conference/participant-p.h"
#include "conference/participant-device.h"
#include "c-wrapper/c-wrapper.h"
#include "lime-v2.h"
#include "private.h"

// TODO remove me
#include "lime.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

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

	const LinphoneAuthInfo *auth_info = linphone_core_find_auth_info(lc, realm, username, domain);

	if (auth_info) {
		const char *auth_username = linphone_auth_info_get_username(auth_info);
		const char *auth_password = linphone_auth_info_get_password(auth_info);
		const char *auth_ha1 = linphone_auth_info_get_ha1(auth_info);
		belle_sip_auth_event_set_username(event, auth_username);
		belle_sip_auth_event_set_passwd(event, auth_password);
		belle_sip_auth_event_set_ha1(event, auth_ha1);
	}
}

BelleSipLimeManager::BelleSipLimeManager (const string &dbAccess, belle_http_provider_t *prov, LinphoneCore *lc) : LimeManager(dbAccess, [prov, lc](const string &url, const string &from, const vector<uint8_t> &message, const lime::limeX3DHServerResponseProcess &responseProcess) {
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

LimeV2::LimeV2 (const std::string &dbAccess, belle_http_provider_t *prov, LinphoneCore *lc) {
	engineType = EncryptionEngineListener::EngineType::LimeV2;
	curve = lime::CurveId::c25519; // c448
	x3dhServerUrl = linphone_config_get_string(linphone_core_get_config(lc), "misc", "x3dh_server_url", "");
	_dbAccess = dbAccess;
	belleSipLimeManager = unique_ptr<BelleSipLimeManager>(new BelleSipLimeManager(dbAccess, prov, lc));
	lastLimeUpdate = linphone_config_get_int(lc->config, "misc", "last_lime_update_time", 0);
}

string LimeV2::getX3dhServerUrl () const {
	return x3dhServerUrl;
}

lime::CurveId LimeV2::getCurveId () const {
	return curve;
}

ChatMessageModifier::Result LimeV2::processOutgoingMessage (const shared_ptr<ChatMessage> &message, int &errorCode) {
	// We use a shared ptr here due to non synchronism with the lambda in the encrypt method
	shared_ptr<ChatMessageModifier::Result> result =  make_shared<ChatMessageModifier::Result>(ChatMessageModifier::Result::Suspended);
    shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();
	const string &localDeviceId = chatRoom->getLocalAddress().asString();
	const IdentityAddress &peerAddress = chatRoom->getPeerAddress();
	shared_ptr<const string> recipientUserId = make_shared<const string>(peerAddress.getAddressWithoutGruu().asString());

	// Add participants to the recipient list
	bool isMultidevice = FALSE;
	auto recipients = make_shared<vector<lime::RecipientData>>();
	const list<shared_ptr<Participant>> participants = chatRoom->getParticipants();
	for (const shared_ptr<Participant> &participant : participants) {
		int nbDevice = 0;
		const list<shared_ptr<ParticipantDevice>> devices = participant->getPrivate()->getDevices();
		for (const shared_ptr<ParticipantDevice> &device : devices) {
			nbDevice++;
			recipients->emplace_back(device->getAddress().asString());
		}
		if (nbDevice > 1) isMultidevice = TRUE;
	}

	// Add potential other devices of the sender
	const list<shared_ptr<ParticipantDevice>> senderDevices = chatRoom->getMe()->getPrivate()->getDevices();
	for (const auto &senderDevice : senderDevices) {
		if (senderDevice->getAddress() != chatRoom->getLocalAddress()) {
			recipients->emplace_back(senderDevice->getAddress().asString());
			isMultidevice = TRUE;
		}
	}

	// TODO warning when multiple devices for the same participant
	if (isMultidevice) {
		// TODO add policies to adapt behaviour when multiple devices
		lWarning() << "Sending encrypted message to multidevice participant";
	}

	const string &plainStringMessage = message->getInternalContent().getBodyAsUtf8String();
	shared_ptr<const vector<uint8_t>> plainMessage = make_shared<const vector<uint8_t>>(plainStringMessage.begin(), plainStringMessage.end());
	shared_ptr<vector<uint8_t>> cipherMessage = make_shared<vector<uint8_t>>();

	try {
		belleSipLimeManager->encrypt(localDeviceId, recipientUserId, recipients, plainMessage, cipherMessage, [localDeviceId, recipients, cipherMessage, message, result] (lime::CallbackReturn returnCode, string errorMessage) {
			if (returnCode == lime::CallbackReturn::success) {
				list<Content *> contents;

				// ---------------------------------------------- SIPFRAG

				Content *sipfrag = new Content();
				sipfrag->setBody("Contact: " + localDeviceId);
				sipfrag->setContentType(ContentType::SipFrag);
				contents.push_back(move(sipfrag));

				// ---------------------------------------------- HEADERS

				for (const auto &recipient : *recipients) {
					vector<uint8_t> encodedCipher = encodeBase64(recipient.DRmessage);
					vector<char> cipherHeaderB64(encodedCipher.begin(), encodedCipher.end());
					Content *cipherHeader = new Content();
					cipherHeader->setBody(cipherHeaderB64);
					cipherHeader->setContentType(ContentType::LimeKey);
					cipherHeader->addHeader("Content-Id", recipient.deviceId);
					Header contentDescription("Content-Description", "Cipher key");
					cipherHeader->addHeader(contentDescription);
					contents.push_back(move(cipherHeader));
				}

				// ---------------------------------------------- MESSAGE

				const vector<uint8_t> *binaryCipherMessage = cipherMessage.get();
				vector<uint8_t> encodedMessage = encodeBase64(*binaryCipherMessage);
				vector<char> cipherMessageB64(encodedMessage.begin(), encodedMessage.end());
				Content *cipherMessage = new Content();
				cipherMessage->setBody(cipherMessageB64);
				cipherMessage->setContentType(ContentType::OctetStream);
				cipherMessage->addHeader("Content-Description", "Encrypted message");
				contents.push_back(move(cipherMessage));

				Content finalContent = ContentManager::contentListToMultipart(contents, MultipartBoundary, true);

				message->setInternalContent(finalContent);
				message->getPrivate()->send(); // seems to leak when called for the second time
				*result = ChatMessageModifier::Result::Done;

				// TODO can be improved
				for (const auto &content : contents) {
					delete content;
				}
			} else {
				lError() << "Lime operation failed: " << errorMessage;
				*result = ChatMessageModifier::Result::Error;
			}
		}, lime::EncryptionPolicy::cipherMessage);
	} catch (const exception &e) {
		lError() << "test" << " while encrypting message";
		*result = ChatMessageModifier::Result::Error;
	}

	return *result;
}

ChatMessageModifier::Result LimeV2::processIncomingMessage (const shared_ptr<ChatMessage> &message, int &errorCode) {
	const shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();
	const string &localDeviceId = chatRoom->getLocalAddress().asString();
	const string &recipientUserId = chatRoom->getPeerAddress().getAddressWithoutGruu().asString();

	// Get message internal content if there is one
	Content internalContent;
	message->getContents();
	if (message->getInternalContent().isEmpty()) {
		lError() << "LIMEv2 no internal content";
		if (message->getContents().front()->isEmpty()) {
			lError() << "LIMEv2 no content in received message";
		}
		internalContent = *message->getContents().front();
	}
	internalContent = message->getInternalContent();

	// Check if message if encrypted and unwrap the multipart
	ContentType expectedContentType = ContentType::Encrypted;
	expectedContentType.addParameter("boundary", MultipartBoundary);
	if (internalContent.getContentType() != expectedContentType) {
		lError() << "LIMEv2 unexpected content-type: " << internalContent.getContentType();
		return ChatMessageModifier::Result::Error;
	}
	list<Content> contentList = ContentManager::multipartToContentList(internalContent);

	// ---------------------------------------------- SIPFRAG

	const string &senderDeviceId = [contentList]() {
		string senderDeviceId;
		for (const auto &content : contentList) {
			if (content.getContentType() != ContentType::SipFrag)
				continue;
			senderDeviceId = content.getBodyAsUtf8String();

			// Extract Contact header from sipfrag content
			string toErase = "Contact: ";
			size_t contactPosition = senderDeviceId.find(toErase);
			if (contactPosition != string::npos) senderDeviceId.erase(contactPosition, toErase.length());
			const string &result = senderDeviceId;
			return result;
		}
		// TODO return nothing or null value
		lError() << "LIMEv2 no sipfrag found";
		const string &result = senderDeviceId;
		return result;
	}();

	// ---------------------------------------------- HEADERS

	const vector<uint8_t> &cipherHeader = [contentList, localDeviceId]() {
		for (const auto &content : contentList) {
			if (content.getContentType() != ContentType::LimeKey)
				continue;

			// TODO workaround because GRUU is parsed as a parameter by content-manager
			Header headerDeviceId = content.getHeader("Content-Id");
			list<HeaderParam> params = headerDeviceId.getParameters();
			HeaderParam gruuParam;
			for (const auto &param : params) {
				if (param.getName() == "gr")
					gruuParam = param;
			}

			const string &recomposedGruu = headerDeviceId.getValue() + gruuParam.asString();
			if (recomposedGruu == localDeviceId) {
				const vector<uint8_t> &cipherHeader = vector<uint8_t>(content.getBody().begin(), content.getBody().end());
				return cipherHeader;
			}
		}
		// TODO return nothing or null value
		lError() << "LIMEv2 no cipher header found";
		const vector<uint8_t> cipherHeader;
		return cipherHeader;
	}();

	// ---------------------------------------------- MESSAGE

	const vector<uint8_t> &cipherMessage = [contentList]() {
		for (const auto &content : contentList) {
			if (content.getContentType() == ContentType::OctetStream) {
				const vector<uint8_t> &cipherMessage = vector<uint8_t>(content.getBody().begin(), content.getBody().end());
				return cipherMessage;
			}
		}
		// TODO return nothing or null value
		lError() << "LIMEv2 no cipher message found";
		const vector<uint8_t> cipherMessage;
		return cipherMessage;
	}();

	vector<uint8_t> decodedCipherHeader = decodeBase64(cipherHeader);
	vector<uint8_t> decodedCipherMessage = decodeBase64(cipherMessage);
	vector<uint8_t> plainMessage{};

	lime::PeerDeviceStatus peerDeviceStatus = lime::PeerDeviceStatus::fail;
	try {
		 peerDeviceStatus = belleSipLimeManager->decrypt(localDeviceId, recipientUserId, senderDeviceId, decodedCipherHeader, decodedCipherMessage, plainMessage);
	} catch (const exception &e) {
		lError() << e.what() << " while decrypting message";
	}

	if (peerDeviceStatus == lime::PeerDeviceStatus::fail) lError() << "Failed to decrypt message from " << senderDeviceId;

	// Prepare decrypted message for next modifier
	string plainMessageString(plainMessage.begin(), plainMessage.end());
	Content finalContent;
	ContentType finalContentType = ContentType::Cpim; // TODO should be the content-type of the decrypted message
	finalContent.setContentType(finalContentType);
	finalContent.setBodyFromUtf8(plainMessageString);
	message->setInternalContent(finalContent);

	// Set the contact in sipfrag as the authenticatedFromAddress for sender authentication
	IdentityAddress sipfragAddress(senderDeviceId);
	message->getPrivate()->setAuthenticatedFromAddress(sipfragAddress);

	// Test errorCode
	return ChatMessageModifier::Result::Done;
}

void LimeV2::update (LinphoneConfig *lpconfig) {
	lime::limeCallback callback = setLimeCallback("Keys update");
	belleSipLimeManager->update(callback);
	lp_config_set_int(lpconfig, "misc", "last_lime_update_time", (int)lastLimeUpdate);
}

bool LimeV2::encryptionEnabledForFileTransfer (const shared_ptr<AbstractChatRoom> &chatRoom) {
	return true;
}

void LimeV2::generateFileTransferKey (const shared_ptr<AbstractChatRoom> &chatRoom, const shared_ptr<ChatMessage> &message) {
	size_t FILE_TRANSFER_KEY_SIZE = 32;
	char keyBuffer [FILE_TRANSFER_KEY_SIZE];// temporary storage of generated key: 192 bits of key + 64 bits of initial vector
	// generate a random 192 bits key + 64 bits of initial vector and store it into the file_transfer_information->key field of the msg
    sal_get_random_bytes((unsigned char *)keyBuffer, FILE_TRANSFER_KEY_SIZE);

	for (Content *content : message->getContents()) {
		if (content->isFileTransfer()) {
			FileTransferContent *fileTransferContent = dynamic_cast<FileTransferContent *>(content); // TODO could static_cast
			fileTransferContent->setFileKey(keyBuffer, FILE_TRANSFER_KEY_SIZE);
			return;
		}
	}
}

int LimeV2::downloadingFile (const shared_ptr<ChatMessage> &message, size_t offset, const uint8_t *buffer, size_t size, uint8_t *decrypted_buffer) {
	const Content *content = message->getPrivate()->getFileTransferContent();
	if (!content)
		return -1;

	const FileTransferContent *fileTransferContent = dynamic_cast<const FileTransferContent *>(content); // TODO could static_cast
	const char *fileKey = fileTransferContent->getFileKey().data();

	if (!fileKey)
		return -1;

	if (!buffer || size == 0)
		return lime_decryptFile(linphone_content_get_cryptoContext_address(L_GET_C_BACK_PTR(content)), NULL, 0, NULL, NULL);

	return lime_decryptFile(
		linphone_content_get_cryptoContext_address(L_GET_C_BACK_PTR(content)),
		(unsigned char *)fileKey,
		size,
		(char *)decrypted_buffer,
		(char *)buffer
	);

	return 0;
}

int LimeV2::uploadingFile (const shared_ptr<ChatMessage> &message, size_t offset, const uint8_t *buffer, size_t *size, uint8_t *encrypted_buffer) {
	const Content *content = message->getPrivate()->getFileTransferContent();
	if (!content)
		return -1;

	const FileTransferContent *fileTransferContent = dynamic_cast<const FileTransferContent *>(content);
	const char *fileKey = fileTransferContent->getFileKey().data();

	if (!fileKey)
		return -1;

	if (!buffer || *size == 0)
		return lime_encryptFile(linphone_content_get_cryptoContext_address(L_GET_C_BACK_PTR(content)), NULL, 0, NULL, NULL);

	size_t file_size = fileTransferContent->getFileSize();
	if (file_size == 0) {
		lWarning() << "File size has not been set, encryption will fail if not done in one step (if file is larger than 16K)";
	} else if (offset + *size < file_size) {
		*size -= (*size % 16);
	}

	return lime_encryptFile(
		linphone_content_get_cryptoContext_address(L_GET_C_BACK_PTR(content)),
		(unsigned char *)fileKey,
		*size,
		(char *)buffer,
		(char *)encrypted_buffer
	);

	return 0;
}

EncryptionEngineListener::EngineType LimeV2::getEngineType () {
	return engineType;
}

AbstractChatRoom::SecurityLevel LimeV2::getSecurityLevel (string deviceId) const {
	lime::PeerDeviceStatus status = belleSipLimeManager->get_peerDeviceStatus(deviceId);
	switch (status) {
		case lime::PeerDeviceStatus::unknown:
			return AbstractChatRoom::SecurityLevel::Encrypted;
		case lime::PeerDeviceStatus::untrusted:
			return AbstractChatRoom::SecurityLevel::Encrypted;
		case lime::PeerDeviceStatus::trusted:
			return AbstractChatRoom::SecurityLevel::Safe;
		default:
			return AbstractChatRoom::SecurityLevel::Unsafe;
	}
}

void LimeV2::cleanDb () {
	remove(_dbAccess.c_str());
}

void LimeV2::onNetworkReachable (bool sipNetworkReachable, bool mediaNetworkReachable) {
	// TODO Work in progress
}

std::shared_ptr<BelleSipLimeManager> LimeV2::getLimeManager () {
	return belleSipLimeManager;
}

lime::limeCallback LimeV2::setLimeCallback (string operation) {
	lime::limeCallback callback([operation](lime::CallbackReturn returnCode, string anythingToSay) {
		if (returnCode == lime::CallbackReturn::success) {
			lInfo() << "Lime operation successful: " << operation;
		} else {
			lInfo() << "Lime operation failed: " << operation;
		}
	});
	return callback;
}

void LimeV2::onRegistrationStateChanged (LinphoneProxyConfig *cfg, LinphoneRegistrationState state, const string &message) {
	if (state == LinphoneRegistrationState::LinphoneRegistrationOk) {

		char *contactAddress = linphone_address_as_string_uri_only(linphone_proxy_config_get_contact(cfg));
		IdentityAddress identityAddress = IdentityAddress(contactAddress);
		string localDeviceId = identityAddress.asString();
		if (contactAddress)
			ms_free(contactAddress);

		stringstream operation;
		operation << "create user " << localDeviceId;
		lime::limeCallback callback = setLimeCallback(operation.str());

		LinphoneConfig *lpconfig = linphone_core_get_config(linphone_proxy_config_get_core(cfg));
		lastLimeUpdate = linphone_config_get_int(lpconfig, "misc", "last_lime_update_time", -1); // TODO should be done by the tester

		try {
			// create user if not exist
			belleSipLimeManager->create_user(localDeviceId, x3dhServerUrl, curve, callback);
			lastLimeUpdate = ms_time(NULL);
			lp_config_set_int(lpconfig, "misc", "last_lime_update_time", (int)lastLimeUpdate);
		} catch (const exception &e) {
			lInfo() << e.what() << " while creating lime user";

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
