/*
 * lime-x3dh-encryption-engine.cpp
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
#include "chat/chat-message/chat-message-p.h"
#include "chat/chat-room/chat-room-p.h"
#include "chat/chat-room/client-group-chat-room.h"
#include "content/content-manager.h"
#include "content/header/header-param.h"
#include "conference/participant-p.h"
#include "conference/participant-device.h"
#include "core/core.h"
#include "c-wrapper/c-wrapper.h"
#include "event-log/conference/conference-security-event.h"
#include "lime-x3dh-encryption-engine.h"
#include "private.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

struct X3dhServerPostContext {
	const lime::limeX3DHServerResponseProcess responseProcess;
	const string username;
	shared_ptr<Core> core;
	X3dhServerPostContext (
		const lime::limeX3DHServerResponseProcess &response,
		const string &username,
		shared_ptr<Core> core
	) : responseProcess(response), username{username}, core{core} {};
};

void LimeManager::processIoError (void *data, const belle_sip_io_error_event_t *event) noexcept {
	X3dhServerPostContext *userData = static_cast<X3dhServerPostContext *>(data);
	(userData->responseProcess)(0, vector<uint8_t>{});
	delete(userData);
}

void LimeManager::processResponse (void *data, const belle_http_response_event_t *event) noexcept {
	X3dhServerPostContext *userData = static_cast<X3dhServerPostContext *>(data);
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

void LimeManager::processAuthRequested (void *data, belle_sip_auth_event_t *event) noexcept {
	X3dhServerPostContext *userData = static_cast<X3dhServerPostContext *>(data);
	shared_ptr<Core> core = userData->core;

	const char *realm = belle_sip_auth_event_get_realm(event);
	const char *username = belle_sip_auth_event_get_username(event);
	const char *domain = belle_sip_auth_event_get_domain(event);

	const LinphoneAuthInfo *auth_info = linphone_core_find_auth_info(core->getCCore(), realm, username, domain);

	if (auth_info) {
		const char *auth_username = linphone_auth_info_get_username(auth_info);
		const char *auth_password = linphone_auth_info_get_password(auth_info);
		const char *auth_ha1 = linphone_auth_info_get_ha1(auth_info);
		belle_sip_auth_event_set_username(event, auth_username);
		belle_sip_auth_event_set_passwd(event, auth_password);
		belle_sip_auth_event_set_ha1(event, auth_ha1);
	}
}

LimeManager::LimeManager (
	const string &dbAccess,
	belle_http_provider_t *prov,
	shared_ptr<Core> core
) : lime::LimeManager(dbAccess, [prov, core](const string &url, const string &from, const vector<uint8_t> &message, const lime::limeX3DHServerResponseProcess &responseProcess) {
	belle_http_request_listener_callbacks_t cbs= {};
	belle_http_request_listener_t *l;
	belle_generic_uri_t *uri;
	belle_http_request_t *req;
	belle_sip_memory_body_handler_t *bh;

	stringstream userAgent;
	userAgent << "Linphone/" << linphone_core_get_version() << " (Lime)" << " Belle-sip/" << belle_sip_version_to_string();

	bh = belle_sip_memory_body_handler_new_copy_from_buffer(message.data(), message.size(), NULL, NULL);
	uri=belle_generic_uri_parse(url.data());
	req=belle_http_request_create("POST", uri,
			belle_http_header_create("User-Agent", userAgent.str().c_str()),
			belle_http_header_create("Content-type", "x3dh/octet-stream"),
			belle_http_header_create("From", from.data()),
			NULL);

	belle_sip_message_set_body_handler(BELLE_SIP_MESSAGE(req),BELLE_SIP_BODY_HANDLER(bh));
	cbs.process_response = processResponse;
	cbs.process_io_error = processIoError;
	cbs.process_auth_requested = processAuthRequested;
	X3dhServerPostContext *userData = new X3dhServerPostContext(responseProcess, from, core);
	l=belle_http_request_listener_create_from_callbacks(&cbs, userData);
	belle_sip_object_data_set(BELLE_SIP_OBJECT(req), "http_request_listener", l, belle_sip_object_unref);
	belle_http_provider_send_request(prov,req,l);
}) {}

LimeX3dhEncryptionEngine::LimeX3dhEncryptionEngine (
	const std::string &dbAccess,
	belle_http_provider_t *prov,
	const shared_ptr<Core> core
) : EncryptionEngine(core) {
	engineType = EncryptionEngine::EngineType::LimeX3dh;
	curve = lime::CurveId::c25519; // c448
	_dbAccess = dbAccess;
	auto cCore = core->getCCore();
	limeManager = unique_ptr<LimeManager>(new LimeManager(dbAccess, prov, core));
	lastLimeUpdate = linphone_config_get_int(cCore->config, "lime", "last_update_time", 0);
	x3dhServerUrl = linphone_config_get_string(
		linphone_core_get_config(cCore),
		"lime",
		"x3dh_server_url",
		""
	);
	if (x3dhServerUrl.empty())
		lError() << "LIME X3DH server URL unavailable for encryption engine";
}

string LimeX3dhEncryptionEngine::getX3dhServerUrl () const {
	return x3dhServerUrl;
}

lime::CurveId LimeX3dhEncryptionEngine::getCurveId () const {
	return curve;
}

ChatMessageModifier::Result LimeX3dhEncryptionEngine::processOutgoingMessage (
	const shared_ptr<ChatMessage> &message,
	int &errorCode
) {
	// We use a shared_ptr here due to non synchronism with the lambda in the encrypt method
	shared_ptr<ChatMessageModifier::Result> result =  make_shared<ChatMessageModifier::Result>(ChatMessageModifier::Result::Suspended);
	shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();
	const string &localDeviceId = chatRoom->getLocalAddress().asString();
	const IdentityAddress &peerAddress = chatRoom->getPeerAddress();
	shared_ptr<const string> recipientUserId = make_shared<const string>(peerAddress.getAddressWithoutGruu().asString());

	// Check if chatroom is encrypted or not
	shared_ptr<ClientGroupChatRoom> cgcr = static_pointer_cast<ClientGroupChatRoom>(chatRoom);
	if (cgcr->getCapabilities() & ChatRoom::Capabilities::Encrypted) {
		lInfo() << "LIME X3DH this chatroom is encrypted, proceed to encrypt outgoing message";
	} else {
		lWarning() << "LIME X3DH this chatroom is not encrypted, no need encrypt outgoing message";
		return ChatMessageModifier::Result::Skipped;
	}

	// Reject message in unsafe chatroom if not allowed
	if (linphone_config_get_int(linphone_core_get_config(chatRoom->getCore()->getCCore()), "lime", "allow_message_in_unsafe_chatroom", 0) == 0) {
		if (chatRoom->getSecurityLevel() == ClientGroupChatRoom::SecurityLevel::Unsafe) {
			lWarning() << "Sending encrypted message in an unsafe chatroom";
			errorCode = 488; // Not Acceptable
			return ChatMessageModifier::Result::Error;
		}
	}

	// Add participants to the recipient list
	bool tooManyDevices = FALSE;
	int maxNbDevicePerParticipant = linphone_config_get_int(linphone_core_get_config(chatRoom->getCore()->getCCore()), "lime", "max_nb_device_per_participant", 1);
	auto recipients = make_shared<vector<lime::RecipientData>>();
	const list<shared_ptr<Participant>> participants = chatRoom->getParticipants();
	for (const shared_ptr<Participant> &participant : participants) {
		int nbDevice = 0;
		const list<shared_ptr<ParticipantDevice>> devices = participant->getPrivate()->getDevices();
		for (const shared_ptr<ParticipantDevice> &device : devices) {
			recipients->emplace_back(device->getAddress().asString());
			nbDevice++;
		}
		if (nbDevice > maxNbDevicePerParticipant) tooManyDevices = TRUE;
	}

	// Add potential other devices of the sender participant
	int nbDevice = 0;
	const list<shared_ptr<ParticipantDevice>> senderDevices = chatRoom->getMe()->getPrivate()->getDevices();
	for (const auto &senderDevice : senderDevices) {
		if (senderDevice->getAddress() != chatRoom->getLocalAddress()) {
			recipients->emplace_back(senderDevice->getAddress().asString());
			nbDevice++;
		}
	}
	if (nbDevice > maxNbDevicePerParticipant) tooManyDevices = TRUE;

	// Check if there is at least one recipient
	if (recipients->empty()) {
		lError() << "LIME X3DH encrypting message with no recipient";
		errorCode = 488;
		return ChatMessageModifier::Result::Error;
	}

	// If too many devices for a participant, throw a local security alert event
	if (tooManyDevices) {
		lWarning() << "LIME X3DH encrypting message for excessive number of devices, message discarded";

		// Check the last 2 events for security alerts before sending a new security event
		bool recentSecurityAlert = false;
		shared_ptr<ClientGroupChatRoom> confListener = static_pointer_cast<ClientGroupChatRoom>(chatRoom);
		list<shared_ptr<EventLog>> eventList = chatRoom->getHistory(2);

		// If there is at least one security alert don't send a new one
		for (const auto &event : eventList) {
			if (event->getType() == ConferenceEvent::Type::ConferenceSecurityEvent) {
				auto securityEvent = static_pointer_cast<ConferenceSecurityEvent>(event);
				if (securityEvent->getSecurityEventType() == ConferenceSecurityEvent::SecurityEventType::ParticipantMaxDeviceCountExceeded) {
					recentSecurityAlert = true;
				}
			}
		}

		// If there is no recent security alert send a new one
		if (!recentSecurityAlert) {
			ConferenceSecurityEvent::SecurityEventType securityEventType = ConferenceSecurityEvent::SecurityEventType::ParticipantMaxDeviceCountExceeded;
			shared_ptr<ConferenceSecurityEvent> securityEvent = make_shared<ConferenceSecurityEvent>(time(nullptr), chatRoom->getConferenceId(), securityEventType);
			confListener->onSecurityEvent(securityEvent);
		}
		errorCode = 488; // Not Acceptable
		return ChatMessageModifier::Result::Error;
	}

	const string &plainStringMessage = message->getInternalContent().getBodyAsUtf8String();
	shared_ptr<const vector<uint8_t>> plainMessage = make_shared<const vector<uint8_t>>(plainStringMessage.begin(), plainStringMessage.end());
	shared_ptr<vector<uint8_t>> cipherMessage = make_shared<vector<uint8_t>>();

	try {
		limeManager->encrypt(localDeviceId, recipientUserId, recipients, plainMessage, cipherMessage, [localDeviceId, recipients, cipherMessage, message, result, &errorCode] (lime::CallbackReturn returnCode, string errorMessage) {
			if (returnCode == lime::CallbackReturn::success) {

				// Ignore devices which do not have keys on the X3DH server
				// The message will still be sent to them but they will not be able to decrypt it
				vector<lime::RecipientData> filteredRecipients;
				filteredRecipients.reserve(recipients->size());
				for (const lime::RecipientData &recipient : *recipients) {
					if (recipient.peerStatus != lime::PeerDeviceStatus::fail) {
						filteredRecipients.push_back(recipient);
					}
				}

				list<Content *> contents;

				// ---------------------------------------------- SIPFRAG

				Content *sipfrag = new Content();
				sipfrag->setBody("From: <" + localDeviceId + ">");
				sipfrag->setContentType(ContentType::SipFrag);
				contents.push_back(move(sipfrag));

				// ---------------------------------------------- HEADERS

				for (const lime::RecipientData &recipient : filteredRecipients) {
					string cipherHeaderB64 = encodeBase64(recipient.DRmessage);
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
				string cipherMessageB64 = encodeBase64(*binaryCipherMessage);
				Content *cipherMessage = new Content();
				cipherMessage->setBody(cipherMessageB64);
				cipherMessage->setContentType(ContentType::OctetStream);
				cipherMessage->addHeader("Content-Description", "Encrypted message");
				contents.push_back(move(cipherMessage));

				Content finalContent = ContentManager::contentListToMultipart(contents, MultipartBoundary, true);

				// Insert protocol param before boundary for flexisip
				ContentType contentType(finalContent.getContentType());
				contentType.removeParameter("boundary");
				contentType.addParameter("protocol", "\"application/lime\"");
				contentType.addParameter("boundary", MultipartBoundary);
				finalContent.setContentType(contentType);

				message->setInternalContent(finalContent);
				message->getPrivate()->send(); // seems to leak when called for the second time
				*result = ChatMessageModifier::Result::Done;

				// TODO can be improved
				for (const auto &content : contents) {
					delete content;
				}
			} else {
				lError() << "LIME X3DH operation failed: " << errorMessage;
				errorCode = 503; // IO Error
				*result = ChatMessageModifier::Result::Error;
			}
		}, lime::EncryptionPolicy::cipherMessage);
	} catch (const exception &e) {
		lError() << e.what() << " while encrypting message";
		errorCode = 503; // IO Error
		*result = ChatMessageModifier::Result::Error;
	}
	return *result;
}

ChatMessageModifier::Result LimeX3dhEncryptionEngine::processIncomingMessage (
	const shared_ptr<ChatMessage> &message,
	int &errorCode
) {
	const shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();
	const string &localDeviceId = chatRoom->getLocalAddress().asString();
	const string &recipientUserId = chatRoom->getPeerAddress().getAddressWithoutGruu().asString();

	const Content *internalContent;
	if (!message->getInternalContent().isEmpty())
		internalContent = &(message->getInternalContent());
	else
		internalContent = message->getContents().front();

	// Check if the message is encrypted and unwrap the multipart
	ContentType incomingContentType = internalContent->getContentType();
	ContentType expectedContentType = ContentType::Encrypted;
	expectedContentType.addParameter("protocol", "\"application/lime\"");
	expectedContentType.addParameter("boundary", MultipartBoundary);

	if (incomingContentType != expectedContentType) {
		lError() << "LIME X3DH unexpected content-type: " << incomingContentType;
		// Set unencrypted content warning flag because incoming message type is unexpected
		message->getPrivate()->setUnencryptedContentWarning(true);
		// Disable sender authentication otherwise the unexpected message will always be discarded
		message->getPrivate()->enableSenderAuthentication(false);
		return ChatMessageModifier::Result::Skipped;
	}
	list<Content> contentList = ContentManager::multipartToContentList(*internalContent);

	// ---------------------------------------------- SIPFRAG

	string senderDeviceId;
	for (const auto &content : contentList) {
		if (content.getContentType() != ContentType::SipFrag)
			continue;

		// Extract Contact header from sipfrag content
		senderDeviceId = content.getBodyAsUtf8String();
		string toErase = "From: ";
		size_t contactPosition = senderDeviceId.find(toErase);
		if (contactPosition != string::npos) senderDeviceId.erase(contactPosition, toErase.length());
		IdentityAddress tmpIdentityAddress(senderDeviceId);
		senderDeviceId = tmpIdentityAddress.asString();
	}

	// Discard incoming messages from unsafe peer devices
	lime::PeerDeviceStatus peerDeviceStatus = limeManager->get_peerDeviceStatus(senderDeviceId);
	if (peerDeviceStatus == lime::PeerDeviceStatus::unsafe) {
		lWarning() << "LIME X3DH discard incoming message from unsafe sender device " << senderDeviceId;
		errorCode = 488; // Not Acceptable
		return ChatMessageModifier::Result::Error;
	}

	// ---------------------------------------------- HEADERS

	string cipherHeader;
	for (const auto &content : contentList) {
		if (content.getContentType() != ContentType::LimeKey)
			continue;

		Header headerDeviceId = content.getHeader("Content-Id");
		if (headerDeviceId.getValueWithParams() == localDeviceId) {
			cipherHeader = content.getBodyAsUtf8String();
		}
	}

	// ---------------------------------------------- MESSAGE

	string cipherMessage;
	for (const auto &content : contentList) {
		if (content.getContentType() == ContentType::OctetStream) {
			cipherMessage = content.getBodyAsUtf8String();
		}
	}

	vector<uint8_t> decodedCipherHeader = decodeBase64(cipherHeader);
	vector<uint8_t> decodedCipherMessage = decodeBase64(cipherMessage);
	vector<uint8_t> plainMessage{};

	try {
		 peerDeviceStatus = limeManager->decrypt(localDeviceId, recipientUserId, senderDeviceId, decodedCipherHeader, decodedCipherMessage, plainMessage);
	} catch (const exception &e) {
		lError() << e.what() << " while decrypting message";
	}

	if (peerDeviceStatus == lime::PeerDeviceStatus::fail) {
		lError() << "Failed to decrypt message from " << senderDeviceId;
		errorCode = 488; // Not Acceptable
		return ChatMessageModifier::Result::Done;
	}

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

	return ChatMessageModifier::Result::Done;
}

void LimeX3dhEncryptionEngine::update () {
	lime::limeCallback callback = setLimeCallback("Keys update");

	LinphoneConfig *lpconfig = linphone_core_get_config(getCore()->getCCore());
	limeManager->update(callback);
	lp_config_set_int(lpconfig, "lime", "last_update_time", (int)lastLimeUpdate);
}

bool LimeX3dhEncryptionEngine::isEncryptionEnabledForFileTransfer (const shared_ptr<AbstractChatRoom> &chatRoom) {
	return true;
}

void LimeX3dhEncryptionEngine::generateFileTransferKey (
	const shared_ptr<AbstractChatRoom> &chatRoom,
	const shared_ptr<ChatMessage> &message
) {
#define FILE_TRANSFER_KEY_SIZE 32
	char keyBuffer [FILE_TRANSFER_KEY_SIZE];// temporary storage of generated key: 192 bits of key + 64 bits of initial vector
	// generate a random 192 bits key + 64 bits of initial vector and store it into the file_transfer_information->key field of the msg
    sal_get_random_bytes((unsigned char *)keyBuffer, FILE_TRANSFER_KEY_SIZE);

	for (Content *content : message->getContents()) {
		if (content->isFileTransfer()) {
			FileTransferContent *fileTransferContent = static_cast<FileTransferContent *>(content);
			fileTransferContent->setFileKey(keyBuffer, FILE_TRANSFER_KEY_SIZE);
			return;
		}
	}
}

int LimeX3dhEncryptionEngine::downloadingFile (
	const shared_ptr<ChatMessage> &message,
	size_t offset,
	const uint8_t *buffer,
	size_t size,
	uint8_t *decrypted_buffer
) {
	const Content *content = message->getPrivate()->getFileTransferContent();
	if (!content)
		return -1;

	const FileTransferContent *fileTransferContent = static_cast<const FileTransferContent *>(content);
	const char *fileKey = fileTransferContent->getFileKey().data();

	if (!fileKey)
		return -1;

	if (!buffer || size == 0)
		return bctbx_aes_gcm_decryptFile(linphone_content_get_cryptoContext_address(L_GET_C_BACK_PTR(content)), NULL, 0, NULL, NULL);

	return bctbx_aes_gcm_decryptFile(
		linphone_content_get_cryptoContext_address(L_GET_C_BACK_PTR(content)),
		(unsigned char *)fileKey,
		size,
		(char *)decrypted_buffer,
		(char *)buffer
	);

	return 0;
}

int LimeX3dhEncryptionEngine::uploadingFile (
	const shared_ptr<ChatMessage> &message,
	size_t offset,
	const uint8_t *buffer,
	size_t *size,
	uint8_t *encrypted_buffer
) {
	const Content *content = message->getPrivate()->getFileTransferContent();
	if (!content)
		return -1;

	const FileTransferContent *fileTransferContent = dynamic_cast<const FileTransferContent *>(content);
	const char *fileKey = fileTransferContent->getFileKey().data();

	if (!fileKey)
		return -1;

	if (!buffer || *size == 0)
		return bctbx_aes_gcm_encryptFile(linphone_content_get_cryptoContext_address(L_GET_C_BACK_PTR(content)), NULL, 0, NULL, NULL);

	size_t file_size = fileTransferContent->getFileSize();
	if (file_size == 0) {
		lWarning() << "File size has not been set, encryption will fail if not done in one step (if file is larger than 16K)";
	} else if (offset + *size < file_size) {
		*size -= (*size % 16);
	}

	return bctbx_aes_gcm_encryptFile(
		linphone_content_get_cryptoContext_address(L_GET_C_BACK_PTR(content)),
		(unsigned char *)fileKey,
		*size,
		(char *)buffer,
		(char *)encrypted_buffer
	);

	return 0;
}

EncryptionEngine::EngineType LimeX3dhEncryptionEngine::getEngineType () {
	return engineType;
}

AbstractChatRoom::SecurityLevel LimeX3dhEncryptionEngine::getSecurityLevel (const string &deviceId) const {
	lime::PeerDeviceStatus status = limeManager->get_peerDeviceStatus(deviceId);
	switch (status) {
		case lime::PeerDeviceStatus::unknown:
		case lime::PeerDeviceStatus::untrusted:
			return AbstractChatRoom::SecurityLevel::Encrypted;
		case lime::PeerDeviceStatus::trusted:
			return AbstractChatRoom::SecurityLevel::Safe;
		case lime::PeerDeviceStatus::unsafe:
			return AbstractChatRoom::SecurityLevel::Unsafe;
		default:
			return AbstractChatRoom::SecurityLevel::Unsafe;
	}
}

list<EncryptionParameter> LimeX3dhEncryptionEngine::getEncryptionParameters () {
	// Get proxy config
	LinphoneProxyConfig *proxy = linphone_core_get_default_proxy_config(getCore()->getCCore());
	if (!proxy) {
		lWarning() << "No proxy config available, unable to setup LIME X3DH identity key for ZRTP auxiliary shared secret";
		return {};
	}

	// Get local device Id from local contact address
	const LinphoneAddress *contactAddress = linphone_proxy_config_get_contact(proxy);
	if (!contactAddress) {
		lWarning() << "No contactAddress available, unable to setup LIMEv2 identity key for ZRTP auxiliary shared secret";
		return {};
	}
	IdentityAddress identityAddress = IdentityAddress(linphone_address_as_string(contactAddress));
	string localDeviceId = identityAddress.asString();
	vector<uint8_t> Ik;

	try {
		limeManager->get_selfIdentityKey(localDeviceId, Ik);
	} catch (const exception &e) {
		lInfo() << e.what() << " while setting up lime identity key for ZRTP auxiliary secret";
		return {};
	}

	if (Ik.empty()) {
		lWarning() << "No identity key available, unable to setup lime identity key for ZRTP auxiliary shared secret";
		return {};
	}

	// Encode to base64 and append to the parameter list
	list<pair<string,string>> paramList;
	string IkB64 = encodeBase64(Ik);
	paramList.push_back(make_pair("Ik", IkB64));
	return paramList;
}

void LimeX3dhEncryptionEngine::mutualAuthentication (
	MSZrtpContext *zrtpContext,
	SalMediaDescription *localMediaDescription,
	SalMediaDescription *remoteMediaDescription,
	LinphoneCallDir direction
) {
	// Get local and remote identity keys from sdp attributes
	const char *charLocalIk = sal_custom_sdp_attribute_find(localMediaDescription->custom_sdp_attributes, "Ik");
	const char *charRemoteIk = sal_custom_sdp_attribute_find(remoteMediaDescription->custom_sdp_attributes, "Ik");

	// If LIME X3DH is disabled there might not be identity keys
	if (!charLocalIk || !charRemoteIk) {
		lError() << "Missing identity keys for mutual authentication";
		return;
	}

	const string LocalIkB64(charLocalIk);
	const string RemoteIkB64(charRemoteIk);

	// Convert to vectors and decode base64
	vector<uint8_t> localIk = decodeBase64(LocalIkB64);
	vector<uint8_t> remoteIk = decodeBase64(RemoteIkB64);

	// Concatenate identity keys in the right order
	vector<uint8_t> vectorAuxSharedSecret;
	if (direction == LinphoneCallDir::LinphoneCallOutgoing) {
		localIk.insert(localIk.end(), remoteIk.begin(), remoteIk.end());
		vectorAuxSharedSecret = localIk;
	} else if (direction == LinphoneCallDir::LinphoneCallIncoming) {
		remoteIk.insert(remoteIk.end(), localIk.begin(), localIk.end());
		vectorAuxSharedSecret = remoteIk;
	} else {
		lError() << "Unknown call direction for mutual authentication";
		return;
	}

	if (vectorAuxSharedSecret.empty()) {
		lError() << "Empty auxiliary shared secret for mutual authentication";
		return;
	}

	// Set the auxiliary shared secret in ZRTP
	size_t auxSharedSecretLength = vectorAuxSharedSecret.size();
	const uint8_t *auxSharedSecret = vectorAuxSharedSecret.data();
	lInfo() << "Setting ZRTP auxiliary shared secret after identity key concatenation";
	int retval = ms_zrtp_setAuxiliarySharedSecret(zrtpContext, auxSharedSecret, auxSharedSecretLength);
	if (retval != 0)
		lError() << "ZRTP auxiliary shared secret mismatch 0x" << hex << retval;
}

void LimeX3dhEncryptionEngine::authenticationVerified (
	MSZrtpContext *zrtpContext,
	SalMediaDescription *remoteMediaDescription,
	const char *peerDeviceId
) {
	// Get peer's Ik
	string remoteIkB64;
	const char *sdpRemoteIk = sal_custom_sdp_attribute_find(remoteMediaDescription->custom_sdp_attributes, "Ik");
	if (sdpRemoteIk)
		remoteIkB64 = sdpRemoteIk;

	vector<uint8_t> remoteIk = decodeBase64(remoteIkB64);
	const IdentityAddress peerDeviceAddr = IdentityAddress(peerDeviceId);

	if (ms_zrtp_getAuxiliarySharedSecretMismatch(zrtpContext) == 2) {
		lInfo() << "No auxiliary shared secret exchange because LIME X3DH disabled";
	}
	// SAS is verified and the auxiliary secret matches so we can trust this peer device
	else if (ms_zrtp_getAuxiliarySharedSecretMismatch(zrtpContext) == 0) {
		try {
			lInfo() << "SAS verified and Ik exchange successful";
			limeManager->set_peerDeviceStatus(peerDeviceId, remoteIk, lime::PeerDeviceStatus::trusted);
		} catch (const exception &e) {
			lInfo() << "LIME X3DH exception" << e.what();
			// Ik error occured, the stored Ik is different from this Ik
			lime::PeerDeviceStatus status = limeManager->get_peerDeviceStatus(peerDeviceId);
			switch (status) {
				case lime::PeerDeviceStatus::unsafe:
					lWarning() << "LIME X3DH peer device " << peerDeviceId << " is unsafe and its lime identity key has changed";
					break;
				case lime::PeerDeviceStatus::untrusted:
					lWarning() << "LIME X3DH peer device " << peerDeviceId << " is untrusted and its lime identity key has changed";
					// TODO specific alert to warn the user that previous messages are compromised
					addSecurityEventInChatrooms(peerDeviceAddr, ConferenceSecurityEvent::SecurityEventType::EncryptionIdentityKeyChanged);
					break;
				case lime::PeerDeviceStatus::trusted:
					lError() << "LIME X3DH peer device " << peerDeviceId << " is already trusted but its lime identity key has changed";
					break;
				case lime::PeerDeviceStatus::unknown:
				case lime::PeerDeviceStatus::fail:
					lError() << "LIME X3DH peer device " << peerDeviceId << " is unknown but its lime identity key has changed";
					break;
			}

			// Delete current peer device data and replace it with the new Ik and a trusted status
			limeManager->delete_peerDevice(peerDeviceId);
			limeManager->set_peerDeviceStatus(peerDeviceId, remoteIk, lime::PeerDeviceStatus::trusted);
		}
	}
	// SAS is verified but the auxiliary secret mismatches
	else {
		ms_zrtp_sas_reset_verified(zrtpContext);
		limeManager->set_peerDeviceStatus(peerDeviceId, lime::PeerDeviceStatus::unsafe);
		addSecurityEventInChatrooms(peerDeviceAddr, ConferenceSecurityEvent::SecurityEventType::ManInTheMiddleDetected);
	}
}

void LimeX3dhEncryptionEngine::authenticationRejected (
	SalMediaDescription *remoteMediaDescription,
	const char *peerDeviceId
) {
	// Get peer's Ik
	string remoteIkB64;
	const char *sdpRemoteIk = sal_custom_sdp_attribute_find(remoteMediaDescription->custom_sdp_attributes, "Ik");
	if (sdpRemoteIk)
		remoteIkB64 = sdpRemoteIk;
	vector<uint8_t> remoteIk = decodeBase64(remoteIkB64);

	// Warn the user that rejecting the SAS reveals a man-in-the-middle
	const IdentityAddress peerDeviceAddr = IdentityAddress(peerDeviceId);
	addSecurityEventInChatrooms(peerDeviceAddr, ConferenceSecurityEvent::SecurityEventType::ManInTheMiddleDetected);

	// Set peer device to untrusted or unsafe depending on configuration
	LinphoneConfig *lp_config = linphone_core_get_config(getCore()->getCCore());
	lime::PeerDeviceStatus statusIfSASrefused = lp_config_get_int(lp_config, "lime", "unsafe_if_sas_refused", 1) ? lime::PeerDeviceStatus::unsafe : lime::PeerDeviceStatus::untrusted;
	limeManager->set_peerDeviceStatus(peerDeviceId, remoteIk, statusIfSASrefused);
}

void LimeX3dhEncryptionEngine::addSecurityEventInChatrooms (
	const IdentityAddress &peerDeviceAddr,
	ConferenceSecurityEvent::SecurityEventType securityEventType
) {
	const list<shared_ptr<AbstractChatRoom>> chatRooms = getCore()->getChatRooms();
	for (const auto &chatRoom : chatRooms) {
		if (chatRoom->findParticipant(peerDeviceAddr)) {
			shared_ptr<ConferenceSecurityEvent> securityEvent = make_shared<ConferenceSecurityEvent>(
				time(nullptr),
				chatRoom->getConferenceId(),
				securityEventType,
				peerDeviceAddr
			);
			shared_ptr<ClientGroupChatRoom> confListener = static_pointer_cast<ClientGroupChatRoom>(chatRoom);
			confListener->onSecurityEvent(securityEvent);
		}
	}
}

shared_ptr<ConferenceSecurityEvent> LimeX3dhEncryptionEngine::onDeviceAdded (
	const IdentityAddress &newDeviceAddr,
	shared_ptr<Participant> participant,
	const shared_ptr<AbstractChatRoom> &chatRoom,
	ChatRoom::SecurityLevel currentSecurityLevel
) {
	lime::PeerDeviceStatus newDeviceStatus = limeManager->get_peerDeviceStatus(newDeviceAddr.asString());
	int maxNbDevicesPerParticipant = linphone_config_get_int(linphone_core_get_config(L_GET_C_BACK_PTR(getCore())), "lime", "max_nb_device_per_participant", 1);
	int nbDevice = int(participant->getPrivate()->getDevices().size());
	shared_ptr<ConferenceSecurityEvent> securityEvent = nullptr;

	// Check if the new participant device is unexpected in which case a security alert is created
	if (nbDevice > maxNbDevicesPerParticipant) {
		lWarning() << "LIME X3DH maximum number of devices exceeded for " << participant->getAddress();
		securityEvent = make_shared<ConferenceSecurityEvent>(
			time(nullptr),
			chatRoom->getConferenceId(),
			ConferenceSecurityEvent::SecurityEventType::ParticipantMaxDeviceCountExceeded,
			newDeviceAddr
		);
		limeManager->set_peerDeviceStatus(newDeviceAddr.asString(), lime::PeerDeviceStatus::unsafe);
	}

	// Otherwise if the chatroom security level was degraded a corresponding security event is created
	else {
		if ((currentSecurityLevel == ChatRoom::SecurityLevel::Safe) && (newDeviceStatus != lime::PeerDeviceStatus::trusted)) {
			lInfo() << "LIME X3DH chat room security level degraded by " << newDeviceAddr.asString();
			securityEvent = make_shared<ConferenceSecurityEvent>(
				time(nullptr),
				chatRoom->getConferenceId(),
				ConferenceSecurityEvent::SecurityEventType::SecurityLevelDowngraded,
				newDeviceAddr
			);
		}
	}
	return securityEvent;
}

void LimeX3dhEncryptionEngine::cleanDb () {
	remove(_dbAccess.c_str());
}

std::shared_ptr<LimeManager> LimeX3dhEncryptionEngine::getLimeManager () {
	return limeManager;
}

lime::limeCallback LimeX3dhEncryptionEngine::setLimeCallback (string operation) {
	lime::limeCallback callback([operation](lime::CallbackReturn returnCode, string anythingToSay) {
		if (returnCode == lime::CallbackReturn::success) {
			lInfo() << "LIME X3DH operation successful: " << operation;
		} else {
			lInfo() << "LIME X3DH operation failed: " << operation;
		}
	});
	return callback;
}

void LimeX3dhEncryptionEngine::onNetworkReachable (bool sipNetworkReachable, bool mediaNetworkReachable) {}

void LimeX3dhEncryptionEngine::onRegistrationStateChanged (
	LinphoneProxyConfig *cfg,
	LinphoneRegistrationState state,
	const string &message
) {
	if (state != LinphoneRegistrationState::LinphoneRegistrationOk)
		return;

	if (x3dhServerUrl.empty()) {
		lError() << "LIME X3DH server URL unavailable for encryption engine: can't create lime user";
		return;
	}

	char *contactAddress = linphone_address_as_string_uri_only(linphone_proxy_config_get_contact(cfg));
	IdentityAddress identityAddress = IdentityAddress(contactAddress);
	string localDeviceId = identityAddress.asString();
	if (contactAddress)
		ms_free(contactAddress);

	stringstream operation;
	operation << "create user " << localDeviceId;
	lime::limeCallback callback = setLimeCallback(operation.str());

	LinphoneConfig *lpconfig = linphone_core_get_config(linphone_proxy_config_get_core(cfg));
	lastLimeUpdate = linphone_config_get_int(lpconfig, "lime", "last_update_time", -1);

	try {
		// create user if not exist
		limeManager->create_user(localDeviceId, x3dhServerUrl, curve, callback);
		lastLimeUpdate = ms_time(NULL);
		lp_config_set_int(lpconfig, "lime", "last_update_time", (int)lastLimeUpdate);
	} catch (const exception &e) {
		lInfo() << "LIME X3DH user already exists or the creation went wrong" << e.what();

		// update keys if necessary
		int limeUpdateThreshold = lp_config_get_int(lpconfig, "lime", "lime_update_threshold", 86400); // 24 hours = 86400 s
		if (ms_time(NULL) - lastLimeUpdate > limeUpdateThreshold) {
			update();
			lastLimeUpdate = ms_time(NULL);
		}
	}
}

LINPHONE_END_NAMESPACE
