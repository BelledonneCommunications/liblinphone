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

#include <climits>

#include "bctoolbox/crypto.h"
#include "bctoolbox/crypto.hh"
#include "bctoolbox/defs.h"
#include "bctoolbox/exception.hh"

#include "account/account.h"
#include "c-wrapper/c-wrapper.h"
#include "chat/chat-message/chat-message-p.h"
#include "chat/chat-room/chat-room.h"
#ifdef HAVE_ADVANCED_IM
#include "chat/chat-room/client-chat-room.h"
#endif // HAVE_ADVANCED_IM
#include "chat/modifier/cpim-chat-message-modifier.h"
#include "conference/participant-device.h"
#include "conference/participant.h"
#include "content/content-manager.h"
#include "content/header/header-param.h"
#include "core/core.h"
#include "event-log/conference/conference-security-event.h"
#include "factory/factory.h"
#include "lime-x3dh-encryption-engine.h"
#include "private.h"
#include "sqlite3_bctbx_vfs.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE
namespace { // local function
lime::CurveId string2CurveId(const std::string &algo) {
	lime::CurveId curve = lime::CurveId::unset; // default to unset
	if (algo.compare("c448") == 0) {
		curve = lime::CurveId::c448;
	} else if (algo.compare("c25519k512") == 0) {
		curve = lime::CurveId::c25519k512;
	} else if (algo.compare("c25519") == 0) {
		curve = lime::CurveId::c25519;
	}
	return curve;
}

const std::string CurveId2String(const lime::CurveId algo) {
	switch (algo) {
		case lime::CurveId::c25519:
			return "c25519";
		case lime::CurveId::c448:
			return "c448";
		case lime::CurveId::c25519k512:
			return "c25519k512";
		case lime::CurveId::unset:
		default:
			return "unset";
	}
}
} // namespace

struct X3dhServerPostContext {
	const lime::limeX3DHServerResponseProcess responseProcess;
	const string username;
	shared_ptr<Core> core;
	X3dhServerPostContext(const lime::limeX3DHServerResponseProcess &response,
	                      const string &username,
	                      shared_ptr<Core> core)
	    : responseProcess(response), username{username}, core{core} {};
};

void LimeManager::processIoError(void *data, BCTBX_UNUSED(const belle_sip_io_error_event_t *event)) noexcept {
	X3dhServerPostContext *userData = static_cast<X3dhServerPostContext *>(data);
	try {
		(userData->responseProcess)(0, vector<uint8_t>{});
	} catch (const exception &e) {
		lError() << "Processing IoError on lime server request triggered an exception: " << e.what();
	}
	delete (userData);
}

void LimeManager::processResponse(void *data, const belle_http_response_event_t *event) noexcept {
	X3dhServerPostContext *userData = static_cast<X3dhServerPostContext *>(data);

	if (event->response) {
		auto code = belle_http_response_get_status_code(event->response);
		belle_sip_message_t *message = BELLE_SIP_MESSAGE(event->response);
		auto body = reinterpret_cast<const uint8_t *>(belle_sip_message_get_body(message));
		auto bodySize = belle_sip_message_get_body_size(message);
		try { // the response processing might generate an exception, make sure it will not flow up to belle-sip
			  // otherwise it will cause an abort
			(userData->responseProcess)(code, vector<uint8_t>{body, body + bodySize});
		} catch (const exception &e) {
			lError() << "Processing lime server response triggered an exception: " << e.what();
		}
	} else {
		try {
			(userData->responseProcess)(0, vector<uint8_t>{});
		} catch (const exception &e) {
			lError() << "Processing empty response event on lime server request triggered an exception: " << e.what();
		}
	}
	delete (userData);
}

void LimeManager::processAuthRequested(void *data, belle_sip_auth_event_t *event) noexcept {
	X3dhServerPostContext *userData = static_cast<X3dhServerPostContext *>(data);
	shared_ptr<Core> core = userData->core;

	/* extract username and domain from the GRUU stored in userData->username */
	auto address = Address::create(userData->username);
	/* we are not using a From field in the request so our event username and domain is empty,
	 * load it with the ones stored in userData */
	belle_sip_auth_event_set_username(event, address->getUsername().c_str());
	belle_sip_auth_event_set_domain(event, address->getDomain().c_str());

	/* Notes: when registering on the Lime server, the user is already registered on the flexisip server
	 * the requested auth info shall thus be present in linphone core (except if registering methods are differents on
	 * flexisip and lime server - very unlikely) This request will thus not use the auth requested callback to get the
	 * information
	 * - Stored auth information in linphone core are indexed by username/domain */
	linphone_core_fill_belle_sip_auth_event(core->getCCore(), event, address->getUsername().data(),
	                                        address->getDomain().data());
}

LimeManager::LimeManager(const string &dbAccess, belle_http_provider_t *prov, shared_ptr<Core> core)
    : lime::LimeManager(
          dbAccess,
          [prov, core](const string &url,
                       const string &from,
                       const vector<uint8_t> &message,
                       const lime::limeX3DHServerResponseProcess &responseProcess) {
	          belle_http_request_listener_callbacks_t cbs = {};
	          belle_http_request_listener_t *l;
	          belle_generic_uri_t *uri;
	          belle_http_request_t *req;
	          belle_sip_memory_body_handler_t *bh;

	          stringstream userAgent;
	          userAgent << "Linphone/" << linphone_core_get_version() << " (Lime)"
	                    << " Belle-sip/" << belle_sip_version_to_string();

	          bh = belle_sip_memory_body_handler_new_copy_from_buffer(message.data(), message.size(), NULL, NULL);
	          uri = belle_generic_uri_parse(url.data());
	          req = belle_http_request_create("POST", uri,
	                                          belle_http_header_create("User-Agent", userAgent.str().c_str()),
	                                          belle_http_header_create("Content-type", "x3dh/octet-stream"),
	                                          belle_http_header_create("From", from.data()), NULL);

	          belle_sip_message_set_body_handler(BELLE_SIP_MESSAGE(req), BELLE_SIP_BODY_HANDLER(bh));
	          cbs.process_response = processResponse;
	          cbs.process_io_error = processIoError;
	          cbs.process_auth_requested = processAuthRequested;
	          X3dhServerPostContext *userData = new X3dhServerPostContext(responseProcess, from, core);
	          l = belle_http_request_listener_create_from_callbacks(&cbs, userData);
	          belle_sip_object_data_set(BELLE_SIP_OBJECT(req), "http_request_listener", l, belle_sip_object_unref);
	          belle_http_provider_send_request(prov, req, l);
          }) {
}

LimeX3dhEncryptionEngine::LimeX3dhEncryptionEngine(const std::string &dbAccess,
                                                   belle_http_provider_t *prov,
                                                   const shared_ptr<Core> core)
    : EncryptionEngine(core) {
	engineType = EncryptionEngine::EngineType::LimeX3dh;
	auto cCore = core->getCCore();
	// Default curve core config is c25519 - keep it for retrocompatibility [june 2024] but when the move
	// to account based setting for this feature is completed, it should default to unset
	const std::string curveConfig = linphone_config_get_string(cCore->config, "lime", "curve", "c25519");
	coreCurve = string2CurveId(curveConfig);
	lInfo() << "[LIME] instanciate a LimeX3dhEncryption engine " << this << " - default server is ["
	        << core->getX3dhServerUrl() << "] and default curve [" << curveConfig << "] DB path: " << dbAccess;
	_dbAccess = dbAccess;
	std::string dbAccessWithParam = std::string("db=\"").append(dbAccess).append("\" vfs=").append(
	    BCTBX_SQLITE3_VFS); // force sqlite3 to use the bctbx_sqlite3_vfs
	try {
		limeManager = std::make_shared<LimeManager>(dbAccessWithParam, prov, core);
	} catch (const BctbxException &e) {
		lInfo() << "[LIME] exception at Encryption engine instanciation" << e.what();
	}
}

LimeX3dhEncryptionEngine::~LimeX3dhEncryptionEngine() {
	lInfo() << "[LIME] destroy LimeX3dhEncryption engine " << this;
}

void LimeX3dhEncryptionEngine::rawEncrypt(
    const std::string &localDeviceId,
    const std::list<std::string> &recipientDevices,
    std::shared_ptr<const std::vector<uint8_t>> plainMessage,
    std::shared_ptr<const std::vector<uint8_t>> associatedData,
    const std::function<void(const bool status, std::unordered_map<std::string, std::vector<uint8_t>> cipherTexts)>
        &callback) const {

	// build the recipient list
	auto recipients = make_shared<vector<lime::RecipientData>>();
	for (const auto &recipient : recipientDevices) {
		recipients->emplace_back(recipient);
	}

	// encrypt
	try {
		// this buffer will not be used as we stick to DRMessage encryption policy
		shared_ptr<vector<uint8_t>> cipherMessage = make_shared<vector<uint8_t>>();
		limeManager->encrypt(
		    localDeviceId, associatedData, recipients, plainMessage, cipherMessage,
		    [localDeviceId, recipients, callback](lime::CallbackReturn returnCode, string errorMessage) {
			    std::unordered_map<std::string, std::vector<uint8_t>> cipherTexts{};
			    if (returnCode == lime::CallbackReturn::success) {
				    for (const auto &recipient : *recipients) {
					    if (recipient.peerStatus != lime::PeerDeviceStatus::fail) {
						    cipherTexts[recipient.deviceId] = recipient.DRmessage;
					    } else {
						    lError() << "[LIME] No cipher message generated for " << recipient.deviceId;
						    cipherTexts[recipient.deviceId] = std::vector<uint8_t>{};
					    }
				    }
				    callback(true, cipherTexts);
			    } else {
				    lError() << "Raw encrypt from " << localDeviceId << " failed: " << errorMessage;
				    callback(false, std::unordered_map<std::string, std::vector<uint8_t>>{});
			    }
		    },
		    lime::EncryptionPolicy::DRMessage);
	} catch (const exception &e) {
		lError() << e.what() << " while raw encrypting message";
		callback(false, std::unordered_map<std::string, std::vector<uint8_t>>{});
	}
}

bool LimeX3dhEncryptionEngine::rawDecrypt(const std::string &localDeviceId,
                                          const std::string &senderDeviceId,
                                          const std::vector<uint8_t> &associatedData,
                                          const std::vector<uint8_t> &cipherText,
                                          BCTBX_UNUSED(std::vector<uint8_t> &plainText)) const {
	auto peerDeviceStatus = lime::PeerDeviceStatus::fail;

	try {
		peerDeviceStatus = limeManager->decrypt(localDeviceId, associatedData, senderDeviceId, cipherText, plainText);
	} catch (const exception &e) {
		lError() << e.what() << " while decrypting message";
		peerDeviceStatus = lime::PeerDeviceStatus::fail;
	}

	if (peerDeviceStatus == lime::PeerDeviceStatus::fail) {
		lError() << "Failed to decrypt message from " << senderDeviceId;
		return false;
	}

	return true;
}

ChatMessageModifier::Result LimeX3dhEncryptionEngine::processOutgoingMessage(const shared_ptr<ChatMessage> &message,
                                                                             int &errorCode) {
	// We use a shared_ptr here due to non synchronism with the lambda in the encrypt method
	shared_ptr<ChatMessageModifier::Result> result =
	    make_shared<ChatMessageModifier::Result>(ChatMessageModifier::Result::Suspended);
	shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();
	if (!chatRoom) {
		lWarning() << "Sending encrypted message on an unknown chatroom";
		errorCode = 488; // Not Acceptable
		return ChatMessageModifier::Result::Error;
	}

	const auto &chatRoomParams = chatRoom->getCurrentParams();
	auto conferenceAddress = chatRoom->getConferenceAddress();
	auto conferenceAddressStr = conferenceAddress ? conferenceAddress->asString() : std::string("sip:unknown");

	// Check if chatroom is encrypted or not
	if (chatRoomParams->getChatParams()->isEncrypted()) {
		lInfo() << "[LIME] chatroom " << chatRoom << " (address " << conferenceAddressStr
		        << ") is encrypted, proceed to encrypt outgoing message";
	} else {
		lInfo() << "[LIME] chatroom " << chatRoom << " (address " << conferenceAddressStr
		        << ") is not encrypted, no need to encrypt outgoing message";
		return ChatMessageModifier::Result::Skipped;
	}

	// Reject message in unsafe chatroom if not allowed
	if (linphone_config_get_int(linphone_core_get_config(chatRoom->getCore()->getCCore()), "lime",
	                            "allow_message_in_unsafe_chatroom", 0) == 0) {
		if (chatRoom->getSecurityLevel() == AbstractChatRoom::SecurityLevel::Unsafe) {
			lWarning() << "Sending encrypted message in an unsafe chatroom";
			errorCode = 488; // Not Acceptable
			return ChatMessageModifier::Result::Error;
		}
	}

	const auto &account = chatRoom->getAccount();
	if (!account) {
		lWarning() << "Sending encrypted message with unknown account";
		errorCode = 488; // Not Acceptable
		return ChatMessageModifier::Result::Error;
	}

	// Add participants to the recipient list
	bool tooManyDevices = FALSE;
	int maxNbDevicePerParticipant = linphone_config_get_int(linphone_core_get_config(chatRoom->getCore()->getCCore()),
	                                                        "lime", "max_nb_device_per_participant", INT_MAX);
	bool requestFullState = false;
	// A set allow doesn't allow to have two identical keys
	std::set<Address> recipientAddresses;
	const list<shared_ptr<Participant>> participants = chatRoom->getParticipants();
	for (const shared_ptr<Participant> &participant : participants) {
		int nbDevice = 0;
		const list<shared_ptr<ParticipantDevice>> devices = participant->getDevices();
		for (const shared_ptr<ParticipantDevice> &device : devices) {
			Address address(*device->getAddress());
			auto [it, inserted] = recipientAddresses.insert(address);
			if (inserted) {
				nbDevice++;
			} else {
				lWarning() << "Multiple instances of participant device with address " << address
				           << " have been found in chat room " << conferenceAddressStr;
				requestFullState = true;
			}
		}
		if (nbDevice > maxNbDevicePerParticipant) tooManyDevices = TRUE;
	}

	// Add potential other devices of the sender participant
	int nbDevice = 0;
	const auto &localAddress = account->getContactAddress();
	const list<shared_ptr<ParticipantDevice>> senderDevices = chatRoom->getMe()->getDevices();
	for (const auto &senderDevice : senderDevices) {
		if (*senderDevice->getAddress() != *localAddress) {
			Address address(*senderDevice->getAddress());
			auto [it, inserted] = recipientAddresses.insert(address);
			if (inserted) {
				nbDevice++;
			} else {
				lWarning() << "Multiple instances of me participant device with address " << address
				           << " have been found in chat room " << conferenceAddressStr;
				requestFullState = true;
			}
		}
	}
	if (nbDevice > maxNbDevicePerParticipant) tooManyDevices = TRUE;

	// Check if there is at least one recipient
	if (recipientAddresses.empty()) {
		lError() << "[LIME] encrypting message on chatroom " << chatRoom << " (address " << conferenceAddressStr
		         << ") with no recipient";
		errorCode = 488;
		return ChatMessageModifier::Result::Error;
	}

	shared_ptr<ClientConference> conference = dynamic_pointer_cast<ClientConference>(chatRoom->getConference());
	if (requestFullState && conference) {
		// It looks like at least one participant device has multiple instances in the chat room therefore request a
		// full state to make sure the client and the server are on the same page
		conference->requestFullState();
	}

	// If too many devices for a participant, throw a local security alert event
	if (tooManyDevices) {
		lWarning() << "[LIME] encrypting message for excessive number of devices, message discarded";

		// Check the last 2 events for security alerts before sending a new security event
		bool recentSecurityAlert = false;
		list<shared_ptr<EventLog>> eventList = chatRoom->getHistory(2);

		// If there is at least one security alert don't send a new one
		for (const auto &event : eventList) {
			if (event->getType() == ConferenceEvent::Type::ConferenceSecurityEvent) {
				auto securityEvent = static_pointer_cast<ConferenceSecurityEvent>(event);
				if (securityEvent->getSecurityEventType() ==
				    ConferenceSecurityEvent::SecurityEventType::ParticipantMaxDeviceCountExceeded) {
					recentSecurityAlert = true;
				}
			}
		}

#ifdef HAVE_ADVANCED_IM
		// If there is no recent security alert send a new one
		if (!recentSecurityAlert) {
			ConferenceSecurityEvent::SecurityEventType securityEventType =
			    ConferenceSecurityEvent::SecurityEventType::ParticipantMaxDeviceCountExceeded;
			shared_ptr<ConferenceSecurityEvent> securityEvent =
			    make_shared<ConferenceSecurityEvent>(time(nullptr), chatRoom->getConferenceId(), securityEventType);
			if (conference) {
				conference->onSecurityEvent(securityEvent);
			}
		}
#endif                   // HAVE_ADVANCED_IM
		errorCode = 488; // Not Acceptable
		return ChatMessageModifier::Result::Error;
	}

	// Compress plain text message - compression after encryption is much less efficient
	// To keep compatibility with version not supporting the encryption at this stage, do it (for now: may 2024)
	// only when the lime base algorithm is c25519k512
	bool compressedPlain = false;
	std::shared_ptr<vector<uint8_t>> plainMessage = nullptr;
	const std::string curveConfig =
	    linphone_config_get_string(message->getChatRoom()->getCore()->getCCore()->config, "lime", "curve", "c25519");
	Content plainContent(message->getInternalContent());
	if (curveConfig.compare("c25519k512") == 0) {
		compressedPlain = plainContent.deflateBody();
	}
	auto &contentBody = plainContent.getBody();
	plainMessage = make_shared<vector<uint8_t>>(contentBody.cbegin(), contentBody.cend());

	shared_ptr<vector<uint8_t>> cipherMessage = make_shared<vector<uint8_t>>();

	try {
		const string localDeviceId = localAddress->asStringUriOnly();
		auto peerAddress = chatRoom->getPeerAddress()->getUriWithoutGruu();
		shared_ptr<const string> recipientUserId = make_shared<const string>(peerAddress.asStringUriOnly());
		errorCode = 0; // no need to specify error code because not used later
		auto recipients = make_shared<vector<lime::RecipientData>>();
		for (const auto &address : recipientAddresses) {
			recipients->emplace_back(address.asStringUriOnly());
		}
		limeManager->encrypt(
		    localDeviceId, recipientUserId, recipients, plainMessage, cipherMessage,
		    [localDeviceId, recipients, cipherMessage, message, result,
		     compressedPlain](lime::CallbackReturn returnCode, string errorMessage) {
			    if (returnCode == lime::CallbackReturn::success) {

				    // Ignore devices which do not have keys on the X3DH server
				    // The message will still be sent to them but they will not be able to decrypt it
				    vector<lime::RecipientData> filteredRecipients;
				    filteredRecipients.reserve(recipients->size());
				    for (const lime::RecipientData &recipient : *recipients) {
					    if (recipient.peerStatus != lime::PeerDeviceStatus::fail) {
						    filteredRecipients.push_back(recipient);
					    } else {
						    lError() << "[LIME] No cipher key generated for " << recipient.deviceId;
					    }
				    }

				    list<shared_ptr<Content>> contents;

				    // ---------------------------------------------- CPIM

				    // Replaces SIPFRAG since version 4.4.0
				    CpimChatMessageModifier ccmm;
				    auto cpimContent = ccmm.createMinimalCpimContentForLimeMessage(message);
				    contents.push_back(std::move(cpimContent));

				    // ---------------------------------------------- SIPFRAG

				    // For backward compatibility only since 4.4.0
				    auto sipfrag = Content::create();
				    sipfrag->setBodyFromLocale("From: <" + localDeviceId + ">");
				    sipfrag->setContentType(ContentType::SipFrag);
				    contents.push_back(std::move(sipfrag));

				    // ---------------------------------------------- HEADERS

				    for (const lime::RecipientData &recipient : filteredRecipients) {
					    string cipherHeaderB64 = bctoolbox::encodeBase64(recipient.DRmessage);
					    auto cipherHeader = Content::create();
					    cipherHeader->setBodyFromLocale(cipherHeaderB64);
					    cipherHeader->setContentType(ContentType::LimeKey);
					    cipherHeader->addHeader("Content-Id", recipient.deviceId);
					    Header contentDescription("Content-Description", "Cipher key");
					    cipherHeader->addHeader(contentDescription);
					    contents.push_back(std::move(cipherHeader));
				    }

				    // ---------------------------------------------- MESSAGE

				    const vector<uint8_t> *binaryCipherMessage = cipherMessage.get();
				    string cipherMessageB64 = bctoolbox::encodeBase64(*binaryCipherMessage);
				    auto cipherMessageC = Content::create();
				    cipherMessageC->setBodyFromLocale(cipherMessageB64);
				    cipherMessageC->setContentType(ContentType::OctetStream);
				    if (compressedPlain) {
					    cipherMessageC->setContentEncoding("deflate");
				    }
				    cipherMessageC->addHeader("Content-Description", "Encrypted message");
				    contents.push_back(std::move(cipherMessageC));

				    auto finalContent = ContentManager::contentListToMultipart(contents, true);

				    /* Septembre 2022 note:
				     * Because of a scandalous ancient bug in belle-sip, we are forced to set
				     * the boundary as the last parameter of the content-type header.
				     * After this is fixed, only the line that adds the protocol parameter is necessary.
				     */
				    ContentType &contentType = finalContent.getContentType();
				    string boundary = contentType.getParameter("boundary").getValue();
				    contentType.removeParameter("boundary");
				    contentType.addParameter("protocol", "\"application/lime\"");
				    contentType.addParameter("boundary", boundary);

				    if (linphone_core_content_encoding_supported(message->getChatRoom()->getCore()->getCCore(),
				                                                 "deflate")) {
					    finalContent.setContentEncoding("deflate");
				    }

				    message->setInternalContent(finalContent);
				    message->getPrivate()->send();
				    *result = ChatMessageModifier::Result::Done;

			    } else {
				    lError() << "[LIME] operation failed: " << errorMessage;
				    message->getPrivate()->setParticipantState(
				        message->getChatRoom()->getConference()->getMe()->getAddress(),
				        ChatMessage::State::NotDelivered, ::ms_time(nullptr));
				    *result = ChatMessageModifier::Result::Error;
			    }
		    },
		    lime::EncryptionPolicy::cipherMessage);
	} catch (const exception &e) {
		lError() << e.what() << " while encrypting message";
		*result = ChatMessageModifier::Result::Error;
	}
	return *result;
}

ChatMessageModifier::Result LimeX3dhEncryptionEngine::processIncomingMessage(const shared_ptr<ChatMessage> &message,
                                                                             int &errorCode) {
	const shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();

	// Check if chatroom is encrypted or not
	const auto &chatRoomParams = chatRoom->getCurrentParams();
	if (chatRoomParams->getChatParams()->isEncrypted()) {
		lInfo() << "[LIME] this chatroom is encrypted, proceed to decrypt incoming message";
	} else {
		lInfo() << "[LIME] this chatroom is not encrypted, no need to decrypt incoming message";
		return ChatMessageModifier::Result::Skipped;
	}

	const Content *internalContent;
	if (!message->getInternalContent().isEmpty()) internalContent = &(message->getInternalContent());
	else internalContent = message->getContents().front().get();

	// Check if the message is encrypted and unwrap the multipart
	if (!isMessageEncrypted(*internalContent)) {
		lError() << "[LIME] unexpected content-type: " << internalContent->getContentType();
		// Set unencrypted content warning flag because incoming message type is unexpected
		message->getPrivate()->setUnencryptedContentWarning(true);
		// Disable sender authentication otherwise the unexpected message will always be discarded
		message->getPrivate()->enableSenderAuthentication(false);
		return ChatMessageModifier::Result::Skipped;
	}
	list<Content> contentList = ContentManager::multipartToContentList(*internalContent);

	const auto &account = chatRoom->getAccount();
	if (!account) {
		lWarning() << "Receiving encrypted message with unknown account";
		errorCode = 488; // Not Acceptable
		return ChatMessageModifier::Result::Error;
	}
	const string &localDeviceId = account->getContactAddress()->asStringUriOnly();

	// ---------------------------------------------- CPIM

	string senderDeviceId;
	bool cpimFound = false;
	for (const auto &content : contentList) { // 4.4.x new behavior
		if (content.getContentType() != ContentType::Cpim) continue;

		CpimChatMessageModifier ccmm;
		senderDeviceId = ccmm.parseMinimalCpimContentInLimeMessage(message, content);
		if (senderDeviceId != "") {
			const Address tmpIdentityAddress(senderDeviceId);
			senderDeviceId = tmpIdentityAddress.asStringUriOnly();
			cpimFound = true;
			break;
		}
	}

	// ---------------------------------------------- SIPFRAG

	if (!cpimFound) { // Legacy behavior (< 4.4.x)
		for (const auto &content : contentList) {
			if (content.getContentType() != ContentType::SipFrag) continue;
			senderDeviceId = Utils::getSipFragAddress(content);
		}
	}

	// Early discard of malformed incoming message: we must have a sender Id to decrypt the message
	if (senderDeviceId.empty()) {
		lWarning() << "[LIME] discard malformed incoming message [" << message << "] for [" << localDeviceId
		           << "]: no sender Device Id found ";
		errorCode = 488; // Not Acceptable
		return ChatMessageModifier::Result::Error;
	}

	// Discard incoming messages from unsafe peer devices
	lime::PeerDeviceStatus peerDeviceStatus = limeManager->get_peerDeviceStatus(senderDeviceId);
	if (linphone_config_get_int(linphone_core_get_config(chatRoom->getCore()->getCCore()), "lime",
	                            "allow_message_in_unsafe_chatroom", 0) == 0) {
		if (peerDeviceStatus == lime::PeerDeviceStatus::unsafe) {
			lWarning() << "[LIME] discard incoming message from unsafe sender device " << senderDeviceId;
			errorCode = 488; // Not Acceptable
			return ChatMessageModifier::Result::Error;
		}
	}

	// ---------------------------------------------- HEADERS

	string cipherHeader;
	for (const auto &content : contentList) {
		if (content.getContentType() != ContentType::LimeKey) continue;

		Header headerDeviceId = content.getHeader("Content-Id");
		if (headerDeviceId.getValueWithParams() == localDeviceId) {
			cipherHeader = content.getBodyAsUtf8String();
		}
	}

	// ---------------------------------------------- MESSAGE

	string cipherMessage;
	bool compressedPlain = false;
	for (const auto &content : contentList) {
		if (content.getContentType() == ContentType::OctetStream) {
			cipherMessage = content.getBodyAsUtf8String();
			if (content.getContentEncoding() == std::string("deflate")) {
				compressedPlain = true;
			}
			break;
		}
	}

	if (forceFailure) {
		lError() << "No key found (on purpose for tests) for [" << localDeviceId << "] for message [" << message << "]";
		errorCode = 488; // Not Acceptable
		return ChatMessageModifier::Result::Error;
	}

	if (cipherHeader.empty()) {
		lError() << "No key found for [" << localDeviceId << "] for message [" << message << "]";
		errorCode = 488; // Not Acceptable
		return ChatMessageModifier::Result::Error;
	}

	vector<uint8_t> decodedCipherHeader = bctoolbox::decodeBase64(cipherHeader);
	vector<uint8_t> decodedCipherMessage = bctoolbox::decodeBase64(cipherMessage);
	vector<uint8_t> plainMessage{};

	try {
		auto peerAddress = chatRoom->getPeerAddress()->getUriWithoutGruu();
		const string &recipientUserId = peerAddress.asStringUriOnly();
		peerDeviceStatus = limeManager->decrypt(localDeviceId, recipientUserId, senderDeviceId, decodedCipherHeader,
		                                        decodedCipherMessage, plainMessage);
	} catch (const exception &e) {
		lError() << e.what() << " while decrypting message";
		peerDeviceStatus = lime::PeerDeviceStatus::fail;
	}

	if (peerDeviceStatus == lime::PeerDeviceStatus::fail) {
		lError() << "Failed to decrypt message from " << senderDeviceId;
		errorCode = 488; // Not Acceptable
		return ChatMessageModifier::Result::Error;
	}

	// Prepare decrypted message for next modifier
	Content finalContent;
	ContentType finalContentType = ContentType::Cpim; // TODO should be the content-type of the decrypted message
	finalContent.setContentType(finalContentType);
	finalContent.setBody(std::move(plainMessage));
	if (compressedPlain) {
		finalContent.inflateBody();
	}
	finalContent.setContentEncoding(internalContent->getContentEncoding());
	message->setInternalContent(finalContent);

	// Set the contact in sipfrag as the authenticatedFromAddress for sender authentication
	const Address sipfragAddress(senderDeviceId);
	message->getPrivate()->setAuthenticatedFromAddress(sipfragAddress);

	return ChatMessageModifier::Result::Done;
}

void LimeX3dhEncryptionEngine::update(const std::string localDeviceId) {
	lime::limeCallback callback = setLimeCallback("Keys update");
	limeManager->update(localDeviceId, callback);
}

bool LimeX3dhEncryptionEngine::isEncryptionEnabledForFileTransfer(const shared_ptr<AbstractChatRoom> &chatRoom) {

	const auto &chatRoomParams = chatRoom->getCurrentParams()->getChatParams();
	return chatRoomParams->isEncrypted();
}

#define FILE_TRANSFER_AUTH_TAG_SIZE 16
#define FILE_TRANSFER_KEY_SIZE 32

void LimeX3dhEncryptionEngine::generateFileTransferKey(
    BCTBX_UNUSED(const shared_ptr<AbstractChatRoom> &chatRoom),
    BCTBX_UNUSED(const shared_ptr<ChatMessage> &message),
    const std::shared_ptr<FileTransferContent> &fileTransferContent) {
	char keyBuffer[FILE_TRANSFER_KEY_SIZE]; // temporary storage of generated key: 192 bits of key + 64 bits of initial
	                                        // vector
	// generate a random 192 bits key + 64 bits of initial vector and store it into the file_transfer_information->key
	// field of the msg
	sal_get_random_bytes((unsigned char *)keyBuffer, FILE_TRANSFER_KEY_SIZE);
	fileTransferContent->setFileKey(keyBuffer, FILE_TRANSFER_KEY_SIZE);
	bctbx_clean(keyBuffer, FILE_TRANSFER_KEY_SIZE);
}

int LimeX3dhEncryptionEngine::downloadingFile(BCTBX_UNUSED(const shared_ptr<ChatMessage> &message),
                                              BCTBX_UNUSED(size_t offset),
                                              const uint8_t *buffer,
                                              size_t size,
                                              uint8_t *decrypted_buffer,
                                              const std::shared_ptr<FileTransferContent> &fileTransferContent) {
	if (fileTransferContent == nullptr) return -1;

	auto content = static_pointer_cast<Content>(fileTransferContent);
	const char *fileKey = fileTransferContent->getFileKey().data();
	if (!fileKey) return -1;

	if (!buffer) {
		// compare auth tag if we have one
		const size_t fileAuthTagSize = fileTransferContent->getFileAuthTagSize();
		if (fileAuthTagSize == FILE_TRANSFER_AUTH_TAG_SIZE) {
			int ret = bctbx_aes_gcm_decryptFile(content->getCryptoContextAddress(), NULL, fileAuthTagSize,
			                                    (char *)fileTransferContent->getFileAuthTag().data(), NULL);
			if (ret != 0) {
				lError() << "download encrypted file : authentication failure";
				return ERROR_FILE_TRANFER_AUTHENTICATION_FAILED;
			} else {
				return ret;
			}
		} else {
			lWarning() << "download encrypted file : no authentication Tag";
			return 0;
		}
	}

	return bctbx_aes_gcm_decryptFile(content->getCryptoContextAddress(), (unsigned char *)fileKey, size,
	                                 (char *)decrypted_buffer, (char *)buffer);
}

int LimeX3dhEncryptionEngine::uploadingFile(BCTBX_UNUSED(const shared_ptr<ChatMessage> &message),
                                            size_t offset,
                                            const uint8_t *buffer,
                                            size_t *size,
                                            uint8_t *encrypted_buffer,
                                            const std::shared_ptr<FileTransferContent> &fileTransferContent) {
	if (fileTransferContent == nullptr) return -1;

	auto content = static_pointer_cast<Content>(fileTransferContent);
	const char *fileKey = fileTransferContent->getFileKey().data();
	if (!fileKey) return -1;

	/* This is the final call, get an auth tag and insert it in the fileTransferContent*/
	if (!buffer || *size == 0) {
		char authTag[FILE_TRANSFER_AUTH_TAG_SIZE]; // store the authentication tag generated at the end of encryption,
		                                           // size is fixed at 16 bytes
		int ret = bctbx_aes_gcm_encryptFile(content->getCryptoContextAddress(), NULL, FILE_TRANSFER_AUTH_TAG_SIZE, NULL,
		                                    authTag);
		fileTransferContent->setFileAuthTag(authTag, 16);
		return ret;
	}

	size_t file_size = fileTransferContent->getFileSize();
	if (file_size == 0) {
		lWarning()
		    << "File size has not been set, encryption will fail if not done in one step (if file is larger than 16K)";
	} else if (offset + *size < file_size) {
		*size -= (*size % 16);
	}

	return bctbx_aes_gcm_encryptFile(content->getCryptoContextAddress(), (unsigned char *)fileKey, *size,
	                                 (char *)buffer, (char *)encrypted_buffer);

	return 0;
}

int LimeX3dhEncryptionEngine::cancelFileTransfer(const std::shared_ptr<FileTransferContent> &fileTransferContent) {
	// calling decrypt with no data and no buffer to write the tag will simply release the encryption context and delete
	// it
	return bctbx_aes_gcm_decryptFile(fileTransferContent->getCryptoContextAddress(), NULL, 0, NULL, NULL);
}

EncryptionEngine::EngineType LimeX3dhEncryptionEngine::getEngineType() {
	return engineType;
}

namespace {
AbstractChatRoom::SecurityLevel limeStatus2ChatRoomSecLevel(const lime::PeerDeviceStatus status) {
	switch (status) {
		case lime::PeerDeviceStatus::unknown:
			return AbstractChatRoom::SecurityLevel::Encrypted;
		case lime::PeerDeviceStatus::untrusted:
			return AbstractChatRoom::SecurityLevel::Encrypted;
		case lime::PeerDeviceStatus::trusted:
			return AbstractChatRoom::SecurityLevel::Safe;
		case lime::PeerDeviceStatus::unsafe:
		default:
			return AbstractChatRoom::SecurityLevel::Unsafe;
	}
}
} // namespace

AbstractChatRoom::SecurityLevel LimeX3dhEncryptionEngine::getSecurityLevel(const string &deviceId) const {
	return limeStatus2ChatRoomSecLevel(limeManager->get_peerDeviceStatus(deviceId));
}
AbstractChatRoom::SecurityLevel LimeX3dhEncryptionEngine::getSecurityLevel(const std::list<string> &deviceId) const {
	return limeStatus2ChatRoomSecLevel(limeManager->get_peerDeviceStatus(deviceId));
}

list<EncryptionParameter> LimeX3dhEncryptionEngine::getEncryptionParameters(const std::shared_ptr<Account> &account) {
	// Sanity checks on the lime manager
	if (!limeManager) {
		lWarning() << "[LIME] Lime manager has not been created for encryption engine " << this
		           << ", unable to setup identity key for ZRTP auxiliary shared secret";
		return {};
	}

	// Sanity checks on the account
	if (!account) {
		lWarning() << "[LIME] No account available, unable to setup identity key for ZRTP auxiliary shared secret";
		return {};
	}

	// Get local device Id from local contact address
	const auto &contactAddress = account->getContactAddress();
	if (!contactAddress) {
		lWarning() << "[LIME] No contactAddress available for account " << account
		           << ", unable to setup identity key for ZRTP auxiliary shared secret";
		return {};
	}

	string localDeviceId = contactAddress->asStringUriOnly();
	vector<uint8_t> Ik;
	try {
		limeManager->get_selfIdentityKey(localDeviceId, Ik);
	} catch (const exception &e) {
		lInfo() << "[LIME] " << e.what() << " while setting up identity key for ZRTP auxiliary secret";
		return {};
	}

	if (Ik.empty()) {
		lWarning() << "[LIME] No identity key available, unable to setup identity key for ZRTP auxiliary shared secret";
		return {};
	}

	// Encode to base64 and append to the parameter list
	list<pair<string, string>> paramList;
	string IkB64 = bctoolbox::encodeBase64(Ik);
	// "Ik" is deprecated, use "lime-Ik" instead. "lime-Ik" parsing is supported since 01/03/2020.
	// Switch here to lime-Ik to start publishing lime-Ik instead of Ik
	paramList.push_back(make_pair("Ik", IkB64));
	return paramList;
}

void LimeX3dhEncryptionEngine::mutualAuthentication(MSZrtpContext *zrtpContext,
                                                    const std::shared_ptr<SalMediaDescription> &localMediaDescription,
                                                    const std::shared_ptr<SalMediaDescription> &remoteMediaDescription,
                                                    LinphoneCallDir direction) {
	// Check we have remote and local media description (remote could be null when a call without SDP is received)
	if (!localMediaDescription || !remoteMediaDescription) {
		lInfo() << "[LIME] Missing media description to get identity keys for mutual authentication, do not set "
		           "auxiliary secret from identity keys";
		return;
	}

	// Get local and remote identity keys from sdp attributes
	std::string LocalIkB64;
	const char *charLocalLimeIk =
	    sal_custom_sdp_attribute_find(localMediaDescription->custom_sdp_attributes, "lime-Ik");
	// "Ik" is deprecated, use "lime-Ik" instead. "lime-Ik" parsing is supported since 01/03/2020.
	if (!charLocalLimeIk) {
		const char *charLocalIk = sal_custom_sdp_attribute_find(localMediaDescription->custom_sdp_attributes, "Ik");
		if (charLocalIk) {
			LocalIkB64 = charLocalIk;
		}
	} else {
		LocalIkB64 = charLocalLimeIk;
	}

	std::string RemoteIkB64;
	const char *charRemoteLimeIk =
	    sal_custom_sdp_attribute_find(remoteMediaDescription->custom_sdp_attributes, "lime-Ik");
	// "Ik" is deprecated, use "lime-Ik" instead. "lime-Ik" parsing is supported since 01/03/2020.
	if (!charRemoteLimeIk) {
		const char *charRemoteIk = sal_custom_sdp_attribute_find(remoteMediaDescription->custom_sdp_attributes, "Ik");
		if (charRemoteIk) {
			RemoteIkB64 = charRemoteIk;
		}
	} else {
		RemoteIkB64 = charRemoteLimeIk;
	}

	// This sdp might be from a non lime aware device
	if (LocalIkB64.size() == 0 || RemoteIkB64.size() == 0) {
		lInfo()
		    << "[LIME] Missing identity keys for mutual authentication, do not set auxiliary secret from identity keys";
		return;
	}

	// Convert to vectors and decode base64
	vector<uint8_t> localIk = bctoolbox::decodeBase64(LocalIkB64);
	vector<uint8_t> remoteIk = bctoolbox::decodeBase64(RemoteIkB64);

	// Concatenate identity keys in the right order
	vector<uint8_t> vectorAuxSharedSecret;
	if (direction == LinphoneCallDir::LinphoneCallOutgoing) {
		localIk.insert(localIk.end(), remoteIk.begin(), remoteIk.end());
		vectorAuxSharedSecret = localIk;
	} else if (direction == LinphoneCallDir::LinphoneCallIncoming) {
		remoteIk.insert(remoteIk.end(), localIk.begin(), localIk.end());
		vectorAuxSharedSecret = remoteIk;
	} else {
		lError() << "[LIME] Unknown call direction for mutual authentication";
		return;
	}

	if (vectorAuxSharedSecret.empty()) {
		lError() << "[LIME] Empty auxiliary shared secret for mutual authentication";
		return;
	}

	// Set the auxiliary shared secret in ZRTP
	size_t auxSharedSecretLength = vectorAuxSharedSecret.size();
	const uint8_t *auxSharedSecret = vectorAuxSharedSecret.data();
	lInfo() << "[LIME] Setting ZRTP auxiliary shared secret after identity key concatenation";
	int retval = ms_zrtp_setAuxiliarySharedSecret(zrtpContext, auxSharedSecret, auxSharedSecretLength);
	if (retval != 0) // not an error as most of the time reason is already set (I.E re-invite case)
		lWarning() << "[LIME] ZRTP auxiliary shared secret cannot be set 0x" << hex << retval;
}

void LimeX3dhEncryptionEngine::authenticationVerified(
    MSZrtpContext *zrtpContext,
    const std::shared_ptr<SalMediaDescription> &remoteMediaDescription,
    const char *peerDeviceId) {
	// Get peer's Ik
	string remoteIkB64;
	const char *sdpRemoteLimeIk =
	    sal_custom_sdp_attribute_find(remoteMediaDescription->custom_sdp_attributes, "lime-Ik");
	if (sdpRemoteLimeIk) {
		remoteIkB64 = sdpRemoteLimeIk;
	} else { /* legacy: check deprecated Ik attribute */
		const char *sdpRemoteIk = sal_custom_sdp_attribute_find(remoteMediaDescription->custom_sdp_attributes, "Ik");
		if (sdpRemoteIk) {
			remoteIkB64 = sdpRemoteIk;
		}
	}

	vector<uint8_t> remoteIk = bctoolbox::decodeBase64(remoteIkB64);
	const std::shared_ptr<Address> peerDeviceAddr = Address::create(peerDeviceId);

	if (ms_zrtp_getAuxiliarySharedSecretMismatch(zrtpContext) == 2 /*BZRTP_AUXSECRET_UNSET*/) {
		lInfo() << "[LIME] No auxiliary shared secret exchanged check from SDP if Ik were exchanged";
	}
	// SAS is verified and the auxiliary secret matches so we can trust this peer device
	else if (ms_zrtp_getAuxiliarySharedSecretMismatch(zrtpContext) == 0 /*BZRTP_AUXSECRET_MATCH*/) {
		try {
			lInfo() << "[LIME] SAS verified and Ik exchange successful";
			limeManager->set_peerDeviceStatus(peerDeviceId, remoteIk, lime::PeerDeviceStatus::trusted);
		} catch (const BctbxException &e) {
			lInfo() << "[LIME] exception" << e.what();
			// Ik error occured, the stored Ik is different from this Ik
			lime::PeerDeviceStatus status = limeManager->get_peerDeviceStatus(peerDeviceId);
			switch (status) {
				case lime::PeerDeviceStatus::unsafe:
					lWarning() << "[LIME] peer device " << peerDeviceId
					           << " is unsafe and its identity key has changed";
					break;
				case lime::PeerDeviceStatus::untrusted:
					lWarning() << "[LIME] peer device " << peerDeviceId
					           << " is untrusted and its identity key has changed";
					// TODO specific alert to warn the user that previous messages are compromised
					addSecurityEventInChatrooms(
					    peerDeviceAddr, ConferenceSecurityEvent::SecurityEventType::EncryptionIdentityKeyChanged);
					break;
				case lime::PeerDeviceStatus::trusted:
					lError() << "[LIME] peer device " << peerDeviceId
					         << " is already trusted but its identity key has changed";
					addSecurityEventInChatrooms(
					    peerDeviceAddr, ConferenceSecurityEvent::SecurityEventType::EncryptionIdentityKeyChanged);
					break;
				case lime::PeerDeviceStatus::unknown:
				case lime::PeerDeviceStatus::fail:
					lError() << "[LIME] peer device " << peerDeviceId << " is unknown but its identity key has changed";
					break;
			}
			// Delete current peer device data and replace it with the new Ik and a trusted status
			limeManager->delete_peerDevice(peerDeviceId);
			limeManager->set_peerDeviceStatus(peerDeviceId, remoteIk, lime::PeerDeviceStatus::trusted);
		} catch (const exception &e) {
			lError() << "[LIME] exception" << e.what();
			return;
		}
	}
	// SAS is verified but the auxiliary secret mismatches
	else /*BZRTP_AUXSECRET_MISMATCH*/ {
		lError() << "[LIME] SAS is verified but the auxiliary secret mismatches, removing trust";
		ms_zrtp_sas_reset_verified(zrtpContext);
		limeManager->set_peerDeviceStatus(peerDeviceId, lime::PeerDeviceStatus::unsafe);
		addSecurityEventInChatrooms(peerDeviceAddr, ConferenceSecurityEvent::SecurityEventType::ManInTheMiddleDetected);
	}
}

void LimeX3dhEncryptionEngine::authenticationRejected(const char *peerDeviceId) {
	// Get peer's Ik
	// Warn the user that rejecting the SAS reveals a man-in-the-middle
	const std::shared_ptr<Address> peerDeviceAddr = Address::create(peerDeviceId);

	if (limeManager->get_peerDeviceStatus(peerDeviceId) == lime::PeerDeviceStatus::trusted) {
		addSecurityEventInChatrooms(peerDeviceAddr,
		                            ConferenceSecurityEvent::SecurityEventType::SecurityLevelDowngraded);
	}

	// Set peer device to untrusted or unsafe depending on configuration
	LinphoneConfig *lp_config = linphone_core_get_config(getCore()->getCCore());
	lime::PeerDeviceStatus statusIfSASrefused = linphone_config_get_int(lp_config, "lime", "unsafe_if_sas_refused", 0)
	                                                ? lime::PeerDeviceStatus::unsafe
	                                                : lime::PeerDeviceStatus::untrusted;
	if (statusIfSASrefused == lime::PeerDeviceStatus::unsafe) {
		addSecurityEventInChatrooms(peerDeviceAddr, ConferenceSecurityEvent::SecurityEventType::ManInTheMiddleDetected);
	}

	limeManager->set_peerDeviceStatus(peerDeviceId, statusIfSASrefused);
}

void LimeX3dhEncryptionEngine::addSecurityEventInChatrooms(
    const std::shared_ptr<Address> &peerDeviceAddr, ConferenceSecurityEvent::SecurityEventType securityEventType) {
	const list<shared_ptr<AbstractChatRoom>> chatRooms = getCore()->getChatRooms();
	for (const auto &chatRoom : chatRooms) {
		const auto &chatRoomParams = chatRoom->getCurrentParams()->getChatParams();
		if (chatRoom->findParticipant(peerDeviceAddr) && chatRoomParams->isEncrypted()) {
			shared_ptr<ConferenceSecurityEvent> securityEvent = make_shared<ConferenceSecurityEvent>(
			    time(nullptr), chatRoom->getConferenceId(), securityEventType, peerDeviceAddr);
			shared_ptr<ClientConference> confListener =
			    dynamic_pointer_cast<ClientConference>(chatRoom->getConference());
			if (confListener) {
				confListener->onSecurityEvent(securityEvent);
			}
		}
	}
}

shared_ptr<ConferenceSecurityEvent>
LimeX3dhEncryptionEngine::onDeviceAdded(const std::shared_ptr<Address> &newDeviceAddr,
                                        shared_ptr<Participant> participant,
                                        const shared_ptr<AbstractChatRoom> &chatRoom,
                                        ChatRoom::SecurityLevel currentSecurityLevel) {
	const auto deviceId = newDeviceAddr->asStringUriOnly();
	lime::PeerDeviceStatus newDeviceStatus = limeManager->get_peerDeviceStatus(deviceId);
	int maxNbDevicesPerParticipant = linphone_config_get_int(linphone_core_get_config(L_GET_C_BACK_PTR(getCore())),
	                                                         "lime", "max_nb_device_per_participant", INT_MAX);
	int nbDevice = int(participant->getDevices().size());
	shared_ptr<ConferenceSecurityEvent> securityEvent = nullptr;

	// Check if the new participant device is unexpected in which case a security alert is created
	if (nbDevice > maxNbDevicesPerParticipant) {
		lWarning() << "[LIME] maximum number of devices exceeded for " << participant->getAddress();
		securityEvent = make_shared<ConferenceSecurityEvent>(
		    time(nullptr), chatRoom->getConferenceId(),
		    ConferenceSecurityEvent::SecurityEventType::ParticipantMaxDeviceCountExceeded, newDeviceAddr);
		limeManager->set_peerDeviceStatus(deviceId, lime::PeerDeviceStatus::unsafe);
	}

	// Otherwise if the chatroom security level was degraded a corresponding security event is created
	else {
		if ((currentSecurityLevel == ChatRoom::SecurityLevel::Safe) &&
		    (newDeviceStatus != lime::PeerDeviceStatus::trusted)) {
			lInfo() << "[LIME] chat room security level degraded by " << deviceId;
			securityEvent = make_shared<ConferenceSecurityEvent>(
			    time(nullptr), chatRoom->getConferenceId(),
			    ConferenceSecurityEvent::SecurityEventType::SecurityLevelDowngraded, newDeviceAddr);
		}
	}
	return securityEvent;
}

void LimeX3dhEncryptionEngine::cleanDb() {
	remove(_dbAccess.c_str());
}

std::shared_ptr<LimeManager> LimeX3dhEncryptionEngine::getLimeManager() {
	return limeManager;
}

void LimeX3dhEncryptionEngine::staleSession(const std::string localDeviceId, const std::string peerDeviceId) {
	try {
		limeManager->stale_sessions(localDeviceId, peerDeviceId);
	} catch (const BctbxException &e) {
		lError() << "[LIME] fail to stale session between local [" << localDeviceId << "] and "
		         << " remote [" << peerDeviceId << "]. lime says: " << e.what();
	}
}

lime::limeCallback LimeX3dhEncryptionEngine::setLimeCallback(string operation) {
	lime::limeCallback callback([operation](lime::CallbackReturn returnCode, string anythingToSay) {
		if (returnCode == lime::CallbackReturn::success) {
			lInfo() << "[LIME] operation successful: " << operation << " : " << anythingToSay;
		} else {
			lInfo() << "[LIME] operation failed: " << operation << " : " << anythingToSay;
		}
	});
	return callback;
}

lime::limeCallback LimeX3dhEncryptionEngine::setLimeUserCreationCallback(LinphoneCore *lc,
                                                                         const std::string localDeviceId,
                                                                         shared_ptr<Account> &account) {
	lime::limeCallback callback([lc, localDeviceId, account](lime::CallbackReturn returnCode, string info) {
		if (returnCode == lime::CallbackReturn::success) {
			lInfo() << "[LIME] user " << localDeviceId << " creation successful";
			account->setLimeUserAccountStatus(LimeUserAccountStatus::LimeUserAccountCreated);
		} else {
			lWarning() << "[LIME] user " << localDeviceId << " creation failed with error [" << info << "]";
			/* mLimeUserAccountStatus set to LimeUserAccountCreationSkiped in order to send the register even if Lime
			 * user creation failed */
			account->setLimeUserAccountStatus(LimeUserAccountStatus::LimeUserAccountCreationSkiped);
		}
		linphone_core_notify_imee_user_registration(lc, returnCode == lime::CallbackReturn::success,
		                                            localDeviceId.data(), info.data());
	});

	return callback;
}

void LimeX3dhEncryptionEngine::onNetworkReachable(BCTBX_UNUSED(bool sipNetworkReachable),
                                                  BCTBX_UNUSED(bool mediaNetworkReachable)) {
}

void LimeX3dhEncryptionEngine::onAccountRegistrationStateChanged(std::shared_ptr<Account> account,
                                                                 LinphoneRegistrationState state,
                                                                 BCTBX_UNUSED(const string &message)) {
	if (state != LinphoneRegistrationState::LinphoneRegistrationOk) return;

	auto accountParams = account->getAccountParams();
	// The LIME server URL set in the account parameters is preferred to that set in the core parameters
	string accountLimeServerUrl = accountParams->getLimeServerUrl();
	if (accountLimeServerUrl.empty()) {
		accountLimeServerUrl = getCore()->getX3dhServerUrl();
		lWarning()
		    << "[LIME] No LIME server URL in account params, trying to fallback on Core's default LIME server URL ["
		    << accountLimeServerUrl << "]";
	}
	if (accountLimeServerUrl.empty()) {
		lWarning() << "[LIME] Server URL unavailable for encryption engine: can't create user";
		return;
	}

	const std::shared_ptr<Address> &identityAddress = account->getContactAddress();
	string localDeviceId = identityAddress->asStringUriOnly();

	lInfo() << "[LIME] Load lime user for device " << localDeviceId << " with server URL [" << accountLimeServerUrl
	        << "]";

	try {
		if (limeManager->is_user(localDeviceId)) {
			limeManager->set_x3dhServerUrl(localDeviceId, accountLimeServerUrl);
			update(localDeviceId);
		} else {
			lError() << "[LIME] Lime user isn't created for device" << localDeviceId << "with server URL ["
			         << accountLimeServerUrl << "]";
		}
	} catch (const exception &e) {
		lError() << "[LIME] user for id [" << localDeviceId << "] cannot be created" << e.what();
	}
}

void LimeX3dhEncryptionEngine::onServerUrlChanged(std::shared_ptr<Account> &account, const std::string &limeServerUrl) {

	auto accountParams = account->getAccountParams();
	// The LIME server URL set in the account parameters is preferred to that set in the core parameters
	string accountLimeServerUrl = limeServerUrl;
	if (accountLimeServerUrl.empty()) {
		accountLimeServerUrl = getCore()->getX3dhServerUrl();
		lWarning()
		    << "[LIME] No LIME server URL in account params, trying to fallback on Core's default LIME server URL ["
		    << accountLimeServerUrl << "]";
	}
	// The LIME base algorithm set in the account parameters is preferred to that set in the core parameters
	lime::CurveId accountLimeAlgo = string2CurveId(accountParams->getLimeAlgo());
	if (accountLimeAlgo == lime::CurveId::unset) {
		accountLimeAlgo = coreCurve;
		lWarning() << "[LIME] No LIME algo in account params. Fallback on Core's default LIME algo ["
		           << CurveId2String(accountLimeAlgo) << "]";
	}

	// Can take into account LIME server changed when the contact address is not known
	auto contactAddress = account->getContactAddress();
	if (!contactAddress) {
		return;
	}
	string localDeviceId = contactAddress->asStringUriOnly();

	LinphoneCore *lc = L_GET_C_BACK_PTR(account->getCore());

	lInfo() << "[LIME] Trying to update lime user for device " << localDeviceId << " with server URL ["
	        << accountLimeServerUrl << "] on base algorithm [" << CurveId2String(accountLimeAlgo) << "]";

	try {
		if (!accountLimeServerUrl.empty()) {
			if (!limeManager->is_user(localDeviceId)) {
				if (accountLimeAlgo != lime::CurveId::unset) {
					lime::limeCallback callback = setLimeUserCreationCallback(lc, localDeviceId, account);
					// create user if not exist
					limeManager->create_user(localDeviceId, accountLimeServerUrl, accountLimeAlgo, callback);
				} else {
					lError() << "[LIME] user for id [" << localDeviceId
					         << "] cannot be created as base algorithm is unset";
					return;
				}
			} else {
				limeManager->set_x3dhServerUrl(localDeviceId, accountLimeServerUrl);
				update(localDeviceId);
			}
		}
	} catch (const exception &e) {
		lError() << "[LIME] user for id [" << localDeviceId << "] cannot be created" << e.what();
	}
}

void LimeX3dhEncryptionEngine::setTestForceDecryptionFailureFlag(bool flag) {
	forceFailure = flag;
}

void LimeX3dhEncryptionEngine::createLimeUser(shared_ptr<Account> &account, const string &gruu) {
	LinphoneCore *lc = L_GET_C_BACK_PTR(account->getCore());
	string accountLimeServerUrl = account->getAccountParams()->getLimeServerUrl();
	if (accountLimeServerUrl.empty()) {
		accountLimeServerUrl = getCore()->getX3dhServerUrl();
		lWarning()
		    << "[LIME] No LIME server URL in account params, trying to fallback on Core's default LIME server URL ["
		    << accountLimeServerUrl << "]";
	}
	if (accountLimeServerUrl.empty()) {
		lWarning() << "[LIME] Server URL unavailable for encryption engine: can't create user";
		account->setLimeUserAccountStatus(LimeUserAccountStatus::LimeUserAccountCreationSkiped);
		return;
	}

	lime::CurveId accountLimeAlgo = string2CurveId(account->getAccountParams()->getLimeAlgo());
	if (accountLimeAlgo == lime::CurveId::unset) {
		accountLimeAlgo = coreCurve;
		lWarning() << "[LIME] No LIME algo in account params. Fallback on Core's default LIME algo ["
		           << CurveId2String(accountLimeAlgo) << "]";
	}
	if (accountLimeAlgo == lime::CurveId::unset) {
		lWarning() << "[LIME] Algo unavailable for encryption engine: can't create user";
		account->setLimeUserAccountStatus(LimeUserAccountStatus::LimeUserAccountCreationSkiped);
		return;
	}

	try {
		if (!limeManager->is_user(gruu)) {
			lInfo() << "[LIME] Try to create lime user for device " << gruu << " with server URL ["
			        << accountLimeServerUrl << "] on base algorithm [" << CurveId2String(accountLimeAlgo) << "]";

			lime::limeCallback callback = setLimeUserCreationCallback(lc, gruu, account);
			// create user if not exist
			limeManager->create_user(gruu, accountLimeServerUrl, accountLimeAlgo, callback);
			account->setLimeUserAccountStatus(LimeUserAccountStatus::LimeUserAccountIsCreating);
		} else {
			string info = "";
			account->setLimeUserAccountStatus(LimeUserAccountStatus::LimeUserAccountCreated);
			linphone_core_notify_imee_user_registration(lc, TRUE, gruu.data(), info.data());
		}
	} catch (const exception &e) {
		lError() << "[LIME] user for id [" << gruu << "] cannot be created" << e.what();
	}
}

bool LimeX3dhEncryptionEngine::participantListRequired() const {
	return true;
}
LINPHONE_END_NAMESPACE
