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
#include <iostream>
#include <sstream>
#include <unordered_set>

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
#include "http/http-client.h"
#include "lime-x3dh-encryption-engine.h"
#include "private.h"
#include "sqlite3_bctbx_vfs.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE
namespace { // local function

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

/**
 * Parse a CSV list of base algorithms
 */
std::vector<lime::CurveId> parseBaseAlgo(const std::string &csv) {
	std::vector<lime::CurveId> algos;
	std::stringstream lineStream(csv);
	std::string algoString;

	// Split line by commas
	while (std::getline(lineStream, algoString, ',')) {
		auto algo = lime::string2CurveId(algoString);
		// check the algo is valid
		if (algo == lime::CurveId::unset) {
			lWarning() << "Invalid lime algo[" << algoString << "] skip it";
		} else {
			algos.push_back(algo);
		}
	}
	return algos;
}
} // anonymous namespace

LimeX3dhEncryptionEngine::LimeX3dhEncryptionEngine(const std::string &dbAccess, const shared_ptr<Core> core)
    : EncryptionEngine(core) {
	engineType = EncryptionEngine::EngineType::LimeX3dh;
	// Default curve core config is c25519 - keep it for retrocompatibility
	// it is still in use for the transition period: use it to produce the a=ik SDP parameter
	// the lime-Iks parameter, allowing support for several curves is used by updated [oct 2024] clients
	// when the transition of all client is completed, it should default to unset
	// This change should also be applied in shared_tester_function / check_lime_ik
	const std::string curveConfig = linphone_config_get_string(core->getCCore()->config, "lime", "curve", "c25519");
	coreCurve = lime::string2CurveId(curveConfig);
	lInfo() << "[LIME] instanciate a LimeX3dhEncryption engine " << this << " - default server is ["
	        << core->getX3dhServerUrl() << "] and default curve [" << curveConfig << "] DB path: " << dbAccess;
	std::string dbAccessWithParam = std::string("db=\"").append(dbAccess).append("\" vfs=").append(
	    BCTBX_SQLITE3_VFS); // force sqlite3 to use the bctbx_sqlite3_vfs
	try {
		limeManager = std::make_shared<lime::LimeManager>(
		    dbAccessWithParam, [core](const string &url, const string &from, vector<uint8_t> &&message,
		                              const lime::limeX3DHServerResponseProcess &responseProcess) {
			    HttpClient &httpClient = core->getHttpClient();
			    HttpRequest &request = httpClient.createRequest("POST", url);
			    request.addHeader("From", from);
			    // We should not use "From" header but X-Lime-user-identity, switch to it when
			    // lime-server 1.2.1(released 2024/05) or above is massively deployed
			    // request.addHeader("X-Lime-user-identity", from);

			    /* extract username and domain from the GRUU given in from parameter */
			    auto address = Address::create(from);
			    request.setAuthInfo(address->getUsername(), address->getDomain());

			    request.setBody(Content(ContentType("x3dh/octet-stream"), std::move(message)));
			    request.execute([responseProcess](const HttpResponse &resp) {
				    switch (resp.getStatus()) {
					    case HttpResponse::Status::Valid:
						    try {
							    (responseProcess(resp.getHttpStatusCode(), resp.getBody().getBody()));
						    } catch (const exception &e) {
							    lError() << "Processing lime server response triggered an exception: " << e.what();
						    }
						    break;
					    case HttpResponse::Status::InvalidRequest:
					    case HttpResponse::Status::IOError:
					    case HttpResponse::Status::Timeout:
						    try {
							    (responseProcess)(0, vector<uint8_t>{});
						    } catch (const exception &e) {
							    lError() << "Processing communication error response with lime server "
							                "request triggered an exception: "
							             << e.what();
						    }
						    break;
				    }
			    });
		    });
	} catch (const BctbxException &e) {
		lInfo() << "[LIME] exception at Encryption engine instanciation" << e.what();
		engineType = EncryptionEngine::EngineType::Undefined;
	}
}

LimeX3dhEncryptionEngine::~LimeX3dhEncryptionEngine() {
	lInfo() << "[LIME] destroy LimeX3dhEncryption engine " << this;
}

void LimeX3dhEncryptionEngine::rawEncrypt(
    const std::string &localDeviceId,
    const std::list<std::string> &recipientDevices,
    const std::vector<uint8_t> &plainMessage,
    const std::vector<uint8_t> &associatedData,
    const std::function<void(const bool status, std::unordered_map<std::string, std::vector<uint8_t>> cipherTexts)>
        &callback) const {

	// build the encryption context, stick to DRMessage encryption policy
	auto encryptionContext =
	    make_shared<lime::EncryptionContext>(associatedData, plainMessage, lime::EncryptionPolicy::DRMessage);
	// add the recipient list
	for (const auto &recipient : recipientDevices) {
		encryptionContext->addRecipient(recipient);
	}

	// encrypt
	try {
		lime::limeCallback encryptionCallback = [localDeviceId, encryptionContext,
		                                         callback](lime::CallbackReturn returnCode, string errorMessage) {
			std::unordered_map<std::string, std::vector<uint8_t>> cipherTexts{};
			if (returnCode == lime::CallbackReturn::success) {
				for (const auto &recipient : encryptionContext->m_recipients) {
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
		};
		limeManager->encrypt(localDeviceId, usersAlgos.at(localDeviceId), std::move(encryptionContext),
		                     std::move(encryptionCallback));
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

	// Check if chatroom is encrypted or not
	if (chatRoomParams->getChatParams()->isEncrypted()) {
		lInfo() << "[LIME] " << *chatRoom << " is encrypted, proceed to encrypt outgoing message";
	} else {
		lInfo() << "[LIME] " << *chatRoom << " is not encrypted, no need to encrypt outgoing message";
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
				           << " have been found in " << *chatRoom;
				requestFullState = true;
			}
		}
		if (nbDevice > maxNbDevicePerParticipant) tooManyDevices = TRUE;
	}

	// Add potential other devices of the sender participant
	int nbDevice = 0;
	const auto &accountContactAddress = account->getContactAddress();
	const auto &localDevice =
	    accountContactAddress ? accountContactAddress : chatRoom->getConferenceId().getLocalAddress();
	if (!localDevice) {
		lWarning() << "Sending an encrypted message but the local device address is unknown";
		errorCode = 488; // Not Acceptable
		return ChatMessageModifier::Result::Error;
	}

	const list<shared_ptr<ParticipantDevice>> senderDevices = chatRoom->getMe()->getDevices();
	for (const auto &senderDevice : senderDevices) {
		if (*senderDevice->getAddress() != *localDevice) {
			Address address(*senderDevice->getAddress());
			auto [it, inserted] = recipientAddresses.insert(address);
			if (inserted) {
				nbDevice++;
			} else {
				lWarning() << "Multiple instances of me participant device with address " << address
				           << " have been found in " << *chatRoom;
				requestFullState = true;
			}
		}
	}
	if (nbDevice > maxNbDevicePerParticipant) tooManyDevices = TRUE;

	// Check if there is at least one recipient
	if (recipientAddresses.empty()) {
		lError() << "[LIME] encrypting message on " << *chatRoom << " with no recipient";
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
	// only when all the supported algorithm are in c25519k512, c25519mlk512, c448mlk1024
	bool compressedPlain = false;
	Content plainContent(message->getInternalContent());
	const string localDeviceId = localDevice->asStringUriOnly();
	if (usersAlgos.find(localDeviceId) != usersAlgos.end()) {
		bool skipDeflate = false;
		for (const auto &algo : usersAlgos[localDeviceId]) {
			if (algo <
			    lime::CurveId::c25519k512) { // enum class is ordered, values under c2519k512 are the older base algo
				skipDeflate = true;
				break;
			}
		}
		if (!skipDeflate) {
			compressedPlain = plainContent.deflateBody();
		}
	}

	try {
		auto peerAddress = chatRoom->getPeerAddress()->getUriWithoutGruu();
		errorCode = 0; // no need to specify error code because not used later
		auto encryptionContext = make_shared<lime::EncryptionContext>(
		    peerAddress.asStringUriOnly(), plainContent.getBody(), lime::EncryptionPolicy::cipherMessage);
		for (const auto &address : recipientAddresses) {
			encryptionContext->addRecipient(address.asStringUriOnly());
		}

		lime::limeCallback encryptionCallback = [localDeviceId, encryptionContext, message, result, compressedPlain](
		                                            lime::CallbackReturn returnCode, string errorMessage) {
			if (returnCode == lime::CallbackReturn::success) {

				// Ignore devices which do not have keys on the X3DH server
				// The message will still be sent to them but they will not be able to decrypt it
				vector<lime::RecipientData> filteredRecipients;
				filteredRecipients.reserve(encryptionContext->m_recipients.size());
				for (const auto &recipient : encryptionContext->m_recipients) {
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

				string cipherMessageB64 = bctoolbox::encodeBase64(encryptionContext->m_cipherMessage);
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
				    message->getChatRoom()->getConference()->getMe()->getAddress(), ChatMessage::State::NotDelivered,
				    ::ms_time(nullptr));
				*result = ChatMessageModifier::Result::Error;
			}
		};

		limeManager->encrypt(localDeviceId, usersAlgos.at(localDeviceId), std::move(encryptionContext),
		                     std::move(encryptionCallback));
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
		lInfo() << "[LIME] " << *chatRoom << " is encrypted, proceed to decrypt incoming message";
	} else {
		lInfo() << "[LIME] " << *chatRoom << " is not encrypted, no need to decrypt incoming message";
		return ChatMessageModifier::Result::Skipped;
	}

	const Content *internalContent;
	if (!message->getInternalContent().isEmpty()) internalContent = &(message->getInternalContent());
	else internalContent = message->getContents().front().get();

	// Check if the message is encrypted and unwrap the multipart
	if (!isMessageEncrypted(*internalContent)) {
		lError() << "[LIME] " << *chatRoom << ": unexpected content-type: " << internalContent->getContentType();
		// Set unencrypted content warning flag because incoming message type is unexpected
		message->getPrivate()->setUnencryptedContentWarning(true);
		// Disable sender authentication otherwise the unexpected message will always be discarded
		message->getPrivate()->enableSenderAuthentication(false);
		return ChatMessageModifier::Result::Skipped;
	}
	list<Content> contentList = ContentManager::multipartToContentList(*internalContent);

	const auto &account = chatRoom->getAccount();
	if (!account) {
		lWarning() << "[LIME] " << *chatRoom << ": Receiving encrypted message with unknown account";
		errorCode = 488; // Not Acceptable
		return ChatMessageModifier::Result::Error;
	}
	const auto &accountContactAddress = account->getContactAddress();
	const auto &localDevice =
	    accountContactAddress ? accountContactAddress : chatRoom->getConferenceId().getLocalAddress();
	if (!localDevice) {
		lWarning() << "[LIME] " << *chatRoom << ": Receiving encrypted message but the local device address is unknown";
		errorCode = 488; // Not Acceptable
		return ChatMessageModifier::Result::Error;
	}
	const string &localDeviceId = localDevice->asStringUriOnly();

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
		lWarning() << "[LIME] " << *chatRoom << " for [" << localDeviceId << "]: no sender Device Id found ";
		errorCode = 488; // Not Acceptable
		return ChatMessageModifier::Result::Error;
	}

	// Discard incoming messages from unsafe peer devices
	lime::PeerDeviceStatus peerDeviceStatus = limeManager->get_peerDeviceStatus(senderDeviceId);
	if (linphone_config_get_int(linphone_core_get_config(chatRoom->getCore()->getCCore()), "lime",
	                            "allow_message_in_unsafe_chatroom", 0) == 0) {
		if (peerDeviceStatus == lime::PeerDeviceStatus::unsafe) {
			lWarning() << "[LIME]  " << *chatRoom << ": discard incoming message from unsafe sender device "
			           << senderDeviceId;
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
		lError() << "[LIME] " << *chatRoom << ": No key found (on purpose for tests) for [" << localDeviceId
		         << "] for message [" << message << "]";
		errorCode = 488; // Not Acceptable
		return ChatMessageModifier::Result::Error;
	}

	if (cipherHeader.empty()) {
		lError() << "[LIME] " << *chatRoom << ": No key found for [" << localDeviceId << "] for message [" << message
		         << "]";
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
		lError() << "[LIME] " << *chatRoom << ": " << e.what() << " while decrypting message";
		peerDeviceStatus = lime::PeerDeviceStatus::fail;
	}

	if (peerDeviceStatus == lime::PeerDeviceStatus::fail) {
		lError() << "[LIME] " << *chatRoom << ": Failed to decrypt message from " << senderDeviceId;
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
	lime::limeCallback callback = [localDeviceId](lime::CallbackReturn returnCode, string anythingToSay) {
		if (returnCode == lime::CallbackReturn::success) {
			lInfo() << "[LIME] Update successful for " << localDeviceId << " : " << anythingToSay;
		} else {
			lInfo() << "[LIME] Update failed for " << localDeviceId << " : " << anythingToSay;
		}
	};
	try {
		limeManager->update(localDeviceId, usersAlgos.at(localDeviceId), std::move(callback));
	} catch (const exception &e) {
		lInfo() << "[LIME] Update failed for " << localDeviceId << " : " << e.what();
	}
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
	std::map<lime::CurveId, std::vector<uint8_t>> Iks;
	if (usersAlgos.find(localDeviceId) != usersAlgos.end()) {
		try {
			limeManager->get_selfIdentityKey(localDeviceId, usersAlgos.at(localDeviceId), Iks);
		} catch (const exception &e) {
			lInfo() << "[LIME] " << e.what() << " while getting identity key for ZRTP auxiliary secret";
			return {};
		}
	}

	if (Iks.empty()) {
		lWarning() << "[LIME] No identity key available, unable to setup identity key for ZRTP auxiliary shared secret";
		return {};
	}

	// Build an attribute lime-Iks:ALGO1:base64key;ALGO2:base64Key
	// If we have a coreCurve (deprecated setting), use it to produce a Ik:base64Ik for compatibility with old version
	std::string limeIksAttr{};
	list<pair<string, string>> paramList;
	for (const auto &pair : Iks) {
		// Encode to base64
		string IkB64 = bctoolbox::encodeBase64(pair.second);
		// Do we need to use it to produce the deprecated Ik attribute?
		// Note: when not present in account, the usersAlgos[localDeviceId] is populated with coreCurve
		// so if the setting was not updated, this produce a Ik and lime-Iks with the same keys.
		if (pair.first == coreCurve) {
			paramList.push_back(make_pair("Ik", IkB64));
		}
		limeIksAttr.append(lime::CurveId2String(pair.first)).append(":").append(IkB64).append(";");
	}
	if (!limeIksAttr.empty()) {
		paramList.push_back(make_pair("lime-Iks", limeIksAttr));
	}
	return paramList;
}

namespace {
// parse an attribute in the form: AlgoName:base64Ik;AlgoName:base64Ik;
// return a vector of curveId/Ik(vector form)
std::vector<std::pair<lime::CurveId, std::vector<uint8_t>>> parseLimeIks(const char *attr) {
	std::vector<std::pair<lime::CurveId, std::vector<uint8_t>>> algoIks;
	std::stringstream lineStream{std::string(attr)};
	std::string algoIkString;

	// Split line by semi colon fields
	while (std::getline(lineStream, algoIkString, ';')) {
		// split the field found by colon
		std::string algoString, Ikb64;
		std::stringstream algoIkStream(algoIkString);
		std::getline(algoIkStream, algoString, ':');
		std::getline(algoIkStream, Ikb64);

		auto algo = lime::string2CurveId(algoString);
		// check the algo is valid
		if (algo == lime::CurveId::unset) {
			lWarning() << "Invalid lime algo[" << algoString << "] skip it";
		} else {
			// convert b64 Ik to actual vector
			vector<uint8_t> Ik = bctoolbox::decodeBase64(Ikb64);
			algoIks.emplace_back(algo, Ik);
		}
	}
	return algoIks;
}
} // anonymous namespace

// return a vector with all supported algorithm - from all users, in no specific order
std::vector<lime::CurveId> LimeX3dhEncryptionEngine::getAllConfiguredAlgos() {
	std::unordered_set<lime::CurveId> allSupportedBaseAlgo{};
	for (const auto &userAlgos : usersAlgos) {          // loop all registered users
		for (const auto &userAlgo : userAlgos.second) { // loop all algos registered to them
			allSupportedBaseAlgo.insert(userAlgo);      // unordered_set will filter out duplicates
		}
	}
	return std::vector<lime::CurveId>(allSupportedBaseAlgo.begin(), allSupportedBaseAlgo.end());
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
	// Store local and remote parts in vectors, then they are concatenated to create the aux secret
	vector<uint8_t> localPart{};
	vector<uint8_t> remotePart{};

	// Historic version pre-multialgo support are using an attribute called Ik
	// since oct 2024, lime multi algo support produce an attribute called lime-Iks
	// First check if both ends support the lime-Iks attribute. In that case we can ignore the Ik one
	const char *charLocalLimeIks =
	    sal_custom_sdp_attribute_find(localMediaDescription->custom_sdp_attributes, "lime-Iks");
	const char *charRemoteLimeIks =
	    sal_custom_sdp_attribute_find(remoteMediaDescription->custom_sdp_attributes, "lime-Iks");

	if (charLocalLimeIks && charRemoteLimeIks) {
		// both ends support the lime multi algo attribute, use it
		auto local = parseLimeIks(charLocalLimeIks);
		auto remote = parseLimeIks(charRemoteLimeIks);

		// serialize them
		for (const auto &algoIk : local) {
			localPart.push_back(static_cast<uint8_t>(algoIk.first));
			localPart.insert(localPart.end(), algoIk.second.cbegin(), algoIk.second.cend());
		}
		for (const auto &algoIk : remote) {
			remotePart.push_back(static_cast<uint8_t>(algoIk.first));
			remotePart.insert(remotePart.end(), algoIk.second.cbegin(), algoIk.second.cend());
		}
	} else {
		// at least one end does not support the lime-Iks attribute
		// check they still both use the Ik one (and hope is was for the same base algo)
		const char *charLocalIk = sal_custom_sdp_attribute_find(localMediaDescription->custom_sdp_attributes, "Ik");
		const char *charRemoteIk = sal_custom_sdp_attribute_find(remoteMediaDescription->custom_sdp_attributes, "Ik");
		if (charLocalIk && charRemoteIk) {
			// both ends support the Ik attribute(but not the lime-Iks one), use it
			localPart = bctoolbox::decodeBase64(std::string(charLocalIk));
			remotePart = bctoolbox::decodeBase64(std::string(charRemoteIk));
		}
	}

	// This sdp might be from a non lime aware device
	if (localPart.size() == 0 || remotePart.size() == 0) {
		lInfo()
		    << "[LIME] Missing identity keys for mutual authentication, do not set auxiliary secret from identity keys";
		return;
	}

	// Concatenate parts in the right order
	vector<uint8_t> vectorAuxSharedSecret;
	if (direction == LinphoneCallDir::LinphoneCallOutgoing) {
		localPart.insert(localPart.end(), remotePart.cbegin(), remotePart.cend());
		vectorAuxSharedSecret = std::move(localPart);
	} else if (direction == LinphoneCallDir::LinphoneCallIncoming) {
		remotePart.insert(remotePart.end(), localPart.cbegin(), localPart.cend());
		vectorAuxSharedSecret = std::move(remotePart);
	} else {
		lError() << "[LIME] Unknown call direction for mutual authentication";
		return;
	}

	if (vectorAuxSharedSecret.empty()) {
		lError() << "[LIME] Empty auxiliary shared secret for mutual authentication";
		return;
	}

	// Set the auxiliary shared secret in ZRTP
	lInfo() << "[LIME] Setting ZRTP auxiliary shared secret after identity key concatenation ";
	int retval =
	    ms_zrtp_setAuxiliarySharedSecret(zrtpContext, vectorAuxSharedSecret.data(), vectorAuxSharedSecret.size());
	if (retval != 0) // not an error as most of the time reason is already set (I.E re-invite case)
		lWarning() << "[LIME] ZRTP auxiliary shared secret cannot be set 0x" << hex << retval;
}

void LimeX3dhEncryptionEngine::authenticationVerified(
    MSZrtpContext *zrtpContext,
    const std::shared_ptr<SalMediaDescription> &remoteMediaDescription,
    const char *peerDeviceId) {
	if (ms_zrtp_getAuxiliarySharedSecretMismatch(zrtpContext) == 2 /*BZRTP_AUXSECRET_UNSET*/) {
		lInfo() << "[LIME] No auxiliary shared secret exchanged check from SDP if Ik were exchanged";
		return;
	}

	// SAS is verified and the auxiliary secret matches so we can trust this peer device
	if (ms_zrtp_getAuxiliarySharedSecretMismatch(zrtpContext) == 0 /*BZRTP_AUXSECRET_MATCH*/) {
		// Get peer's Ik(s)
		std::vector<std::pair<lime::CurveId, std::vector<uint8_t>>> remoteIks;

		// This version uses lime-Iks always. So if peer has lime-Iks, we built the aux secret with both
		// of them, so peer's lime-Iks is verified
		const char *charRemoteLimeIks =
		    sal_custom_sdp_attribute_find(remoteMediaDescription->custom_sdp_attributes, "lime-Iks");
		if (charRemoteLimeIks) {
			remoteIks = parseLimeIks(charRemoteLimeIks);
		} else { /* legacy: peer does not have lime-Iks. check deprecated Ik attribute, and hope peer uses the same
			        legacy base algorithm we do */
			const char *charRemoteIk =
			    sal_custom_sdp_attribute_find(remoteMediaDescription->custom_sdp_attributes, "Ik");
			if (charRemoteIk && coreCurve != lime::CurveId::unset) {
				remoteIks.emplace_back(coreCurve, bctoolbox::decodeBase64(charRemoteIk));
			}
		}

		try {
			lInfo() << "[LIME] SAS verified and Ik exchange successful";
			for (const auto &remoteIk : remoteIks) {
				limeManager->set_peerDeviceStatus(peerDeviceId, remoteIk.first, remoteIk.second,
				                                  lime::PeerDeviceStatus::trusted);
			}
		} catch (const BctbxException &e) {
			lInfo() << "[LIME] exception" << e.what();
			// Ik error occured, the stored Ik is different from this Ik
			lime::PeerDeviceStatus status = limeManager->get_peerDeviceStatus(peerDeviceId);
			const std::shared_ptr<Address> peerDeviceAddr = Address::create(peerDeviceId);
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
			// Delete current peer device data(all known base algo variants) and replace it with the new Ik and a
			// trusted status
			limeManager->delete_peerDevice(peerDeviceId);
			for (const auto &remoteIk : remoteIks) {
				limeManager->set_peerDeviceStatus(peerDeviceId, remoteIk.first, remoteIk.second,
				                                  lime::PeerDeviceStatus::trusted);
			}
		} catch (const exception &e) {
			lError() << "[LIME] exception" << e.what();
			return;
		}
	}
	// SAS is verified but the auxiliary secret mismatches
	else /*BZRTP_AUXSECRET_MISMATCH*/ {
		lError() << "[LIME] SAS is verified but the auxiliary secret mismatches, removing trust for " << peerDeviceId;
		ms_zrtp_sas_reset_verified(zrtpContext);
		// We do not know wich local device was involved and we cannot trust the lime-Iks content :
		// remove trust for all device with any supported algo from any user on our side
		limeManager->set_peerDeviceStatus(peerDeviceId, getAllConfiguredAlgos(), lime::PeerDeviceStatus::unsafe);
		addSecurityEventInChatrooms(Address::create(peerDeviceId),
		                            ConferenceSecurityEvent::SecurityEventType::ManInTheMiddleDetected);
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

	// We do not know wich local device was involved and we cannot trust the lime-Iks content :
	// remove trust for all device with any supported algo from any user on our side
	limeManager->set_peerDeviceStatus(peerDeviceId, getAllConfiguredAlgos(), statusIfSASrefused);
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
		lWarning() << "[LIME] maximum number of devices exceeded for " << *participant->getAddress();
		securityEvent = make_shared<ConferenceSecurityEvent>(
		    time(nullptr), chatRoom->getConferenceId(),
		    ConferenceSecurityEvent::SecurityEventType::ParticipantMaxDeviceCountExceeded, newDeviceAddr);
		limeManager->set_peerDeviceStatus(deviceId, getAllConfiguredAlgos(), lime::PeerDeviceStatus::unsafe);
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

void LimeX3dhEncryptionEngine::staleSession(const std::string localDeviceId, const std::string peerDeviceId) {
	try {
		auto curveIds = usersAlgos.at(localDeviceId);
		limeManager->stale_sessions(localDeviceId, curveIds, peerDeviceId);
	} catch (const BctbxException &e) {
		lError() << "[LIME] fail to stale session between local [" << localDeviceId << "] and "
		         << " remote [" << peerDeviceId << "]. lime says: " << e.what();
	} catch (std::out_of_range &) {
		lError() << "Unable to find curve IDs for device " << localDeviceId;
	}
}

lime::limeCallback LimeX3dhEncryptionEngine::setLimeUserCreationCallback(const std::string &localDeviceId,
                                                                         shared_ptr<Account> &account) {
	LinphoneCore *lc = L_GET_C_BACK_PTR(account->getCore());
	std::string baseAlgos = lime::CurveId2String(usersAlgos[localDeviceId]);
	lime::limeCallback callback = [lc, localDeviceId, baseAlgos, account](lime::CallbackReturn returnCode,
	                                                                      string info) {
		if (returnCode == lime::CallbackReturn::success) {
			lInfo() << "[LIME] user " << localDeviceId << " creation successful on base algo " << baseAlgos;
			account->setLimeUserAccountStatus(LimeUserAccountStatus::LimeUserAccountCreated);
		} else {
			lWarning() << "[LIME] user " << localDeviceId << " on base algo " << baseAlgos
			           << " creation failed with error [" << info << "]";
			/* mLimeUserAccountStatus set to LimeUserAccountCreationSkiped in order to send the register even if
			 * Lime user creation failed */
			account->setLimeUserAccountStatus(LimeUserAccountStatus::LimeUserAccountCreationSkiped);
		}
		linphone_core_notify_imee_user_registration(lc, returnCode == lime::CallbackReturn::success,
		                                            localDeviceId.data(), info.data());
	};
	return callback;
}

void LimeX3dhEncryptionEngine::onNetworkReachable(BCTBX_UNUSED(bool sipNetworkReachable),
                                                  BCTBX_UNUSED(bool mediaNetworkReachable)) {
}

void LimeX3dhEncryptionEngine::onAccountRegistrationStateChanged(std::shared_ptr<Account> account,
                                                                 LinphoneRegistrationState state,
                                                                 BCTBX_UNUSED(const string &message)) {
	if (state != LinphoneRegistrationState::LinphoneRegistrationOk) return;

	const std::shared_ptr<Address> &contactAddress = account->getContactAddress();
	if (!contactAddress) {
		lWarning() << "[LIME] Unable to load LIME user because the contact address of account [" << account
		           << "] has not been defined yet";
		return;
	}
	string localDeviceId = contactAddress->asStringUriOnly();

	if (usersAlgos.find(localDeviceId) != usersAlgos.end()) {
		try {
			if (limeManager->is_user(localDeviceId, usersAlgos[localDeviceId])) {
				update(localDeviceId);
			} else {
				lError() << "[LIME] Lime user isn't created for device" << localDeviceId << " and algos ["
				         << lime::CurveId2String(usersAlgos[localDeviceId]) << "]";
				createLimeUser(account, localDeviceId);
			}

		} catch (const exception &e) {
			lError() << "[LIME] user for id [" << localDeviceId << "] cannot be updated " << e.what();
		}
	}
}

void LimeX3dhEncryptionEngine::onServerUrlChanged(std::shared_ptr<Account> &account, const std::string &limeServerUrl) {
	// Can't take into account LIME server changed when the contact address is not known
	auto contactAddress = account->getContactAddress();
	if (!contactAddress) {
		lWarning() << "[LIME] Unable to update LIME user because contact address of account [" << account
		           << "] has not been defined yet";
		return;
	}
	string localDeviceId = contactAddress->asStringUriOnly();

	string accountLimeServerUrl = limeServerUrl;
	if (accountLimeServerUrl.empty()) {
		accountLimeServerUrl = getCore()->getX3dhServerUrl();
		lWarning()
		    << "[LIME] No LIME server URL in account params, trying to fallback on Core's default LIME server URL ["
		    << accountLimeServerUrl << "]";
	}
	if (accountLimeServerUrl.empty()) {
		lWarning() << "[LIME] Server URL unavailable for encryption engine: can't update user [" << localDeviceId
		           << "]";
		return;
	}
	// Also reload the user's algo setting (as there is no callback for this change)
	auto accountLimeAlgo = parseBaseAlgo(account->getAccountParams()->getLimeAlgo());
	if (accountLimeAlgo.empty() && (coreCurve != lime::CurveId::unset)) {
		accountLimeAlgo = std::vector<lime::CurveId>{coreCurve};
		lWarning() << "[LIME] No LIME algo in account params. Fallback on Core's default LIME algo ["
		           << CurveId2String(coreCurve) << "]";
	}
	if (accountLimeAlgo.empty()) {
		lWarning() << "[LIME] Algo unavailable for encryption engine: can't update user server URL for user ["
		           << localDeviceId << "]";
		return;
	}
	/* update this user prefered algo list */
	usersAlgos[localDeviceId] = accountLimeAlgo;

	lInfo() << "[LIME] Trying to update lime user for device " << localDeviceId << " with server URL ["
	        << accountLimeServerUrl << "] on base algorithm [" << CurveId2String(accountLimeAlgo) << "]";

	try {
		if (!limeManager->is_user(localDeviceId, accountLimeAlgo)) {
			auto callback = setLimeUserCreationCallback(localDeviceId, account);
			// create user if not exist
			limeManager->create_user(localDeviceId, accountLimeAlgo, accountLimeServerUrl, std::move(callback));
			account->setLimeUserAccountStatus(LimeUserAccountStatus::LimeUserAccountIsCreating);
		} else {
			limeManager->set_x3dhServerUrl(localDeviceId, accountLimeAlgo, accountLimeServerUrl);
			update(localDeviceId);
		}
	} catch (const exception &e) {
		lError() << "[LIME] user for id [" << localDeviceId << "] cannot update its server url : " << e.what();
	}
}

void LimeX3dhEncryptionEngine::setTestForceDecryptionFailureFlag(bool flag) {
	forceFailure = flag;
}

void LimeX3dhEncryptionEngine::createLimeUser(shared_ptr<Account> &account, const string &gruu) {
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

	auto accountLimeAlgo = parseBaseAlgo(account->getAccountParams()->getLimeAlgo());
	if (accountLimeAlgo.empty() && (coreCurve != lime::CurveId::unset)) {
		accountLimeAlgo = std::vector<lime::CurveId>{coreCurve};
		lWarning() << "[LIME] No LIME algo in account params. Fallback on Core's default LIME algo ["
		           << CurveId2String(coreCurve) << "]";
	}
	if (accountLimeAlgo.empty()) {
		lWarning() << "[LIME] Algo unavailable for encryption engine: can't create user";
		account->setLimeUserAccountStatus(LimeUserAccountStatus::LimeUserAccountCreationSkiped);
		return;
	}
	/* store this users prefered algo list */
	usersAlgos[gruu] = accountLimeAlgo;

	try {
		if (!limeManager->is_user(gruu, accountLimeAlgo)) {
			lInfo() << "[LIME] Try to create lime user for device " << gruu << " with server URL ["
			        << accountLimeServerUrl << "] on base algorithm [" << CurveId2String(accountLimeAlgo) << "]";
			auto callback = setLimeUserCreationCallback(gruu, account);
			// create user if not exist
			limeManager->create_user(gruu, accountLimeAlgo, accountLimeServerUrl, std::move(callback));
			account->setLimeUserAccountStatus(LimeUserAccountStatus::LimeUserAccountIsCreating);
		} else {
			LinphoneCore *lc = L_GET_C_BACK_PTR(account->getCore());
			string info = "";
			account->setLimeUserAccountStatus(LimeUserAccountStatus::LimeUserAccountCreated);
			linphone_core_notify_imee_user_registration(lc, TRUE, gruu.data(), info.data());
		}
	} catch (const exception &e) {
		LinphoneCore *lc = L_GET_C_BACK_PTR(account->getCore());
		lError() << "[LIME] user for id [" << gruu << "] cannot be created: " << e.what();
		account->setLimeUserAccountStatus(LimeUserAccountStatus::LimeUserAccountCreationSkiped);
		linphone_core_notify_imee_user_registration(lc, false, gruu.data(), NULL);
		return;
	}
}

bool LimeX3dhEncryptionEngine::participantListRequired() const {
	return true;
}
LINPHONE_END_NAMESPACE
