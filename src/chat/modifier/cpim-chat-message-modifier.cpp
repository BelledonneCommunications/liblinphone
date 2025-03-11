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

#include "bctoolbox/utils.hh"
#include <bctoolbox/defs.h>

#include "linphone/utils/utils.h"

#include "address/address.h"
#include "chat/chat-message/chat-message-p.h"
#include "chat/chat-room/chat-room.h"
#include "chat/cpim/cpim.h"
#include "conference/conference-params.h"
#include "content/content-disposition.h"
#include "content/content-manager.h"
#include "content/content-type.h"
#include "content/content.h"
#include "logger/logger.h"

#include "cpim-chat-message-modifier.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// Following https://tools.ietf.org/html/rfc4151
const string linphoneNamespaceTag = "tag:linphone.org,2020:params:groupchat";
const string linphoneNamespace = "linphone";
const string linphoneEphemeralHeader = "Ephemeral-Time";
const string linphoneReplyingToMessageIdHeader = "Replying-To-Message-ID";
const string linphoneReplyingToMessageSenderHeader = "Replying-To-Sender";
const string linphoneReactionToMessageIdHeader = "In-Reply-To";

const string imdnNamespaceUrn = "urn:ietf:params:imdn";
const string imdnNamespace = "imdn";
const string imdnMessageIdHeader = "Message-ID";
const string imdnForwardInfoHeader = "Forward-Info";
const string imdnDispositionNotificationHeader = "Disposition-Notification";

ChatMessageModifier::Result CpimChatMessageModifier::encode(const shared_ptr<ChatMessage> &message,
                                                            BCTBX_UNUSED(int &errorCode)) {
	Cpim::Message cpimMessage;
	shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();
	const auto &account = chatRoom->getAccount();
	if (!account) {
		lWarning() << "Unable to encode CPIM because the account attached to the message is unknown";
		return ChatMessageModifier::Result::Error;
	}
	const bool isBasicChatRoom =
	    (chatRoom->getCurrentParams()->getChatParams()->getBackend() == ChatParams::Backend::Basic);
	std::shared_ptr<Address> localDevice;
	// For non-basic tchat, prefer using the account contact address in case the message from address is the account's
	// identity address. It will help the server to correctly dispatch requests to the other participants and sender
	// devices
	if (!isBasicChatRoom) {
		localDevice = account->getContactAddress();
	}
	if (!localDevice) {
		localDevice = message->getFromAddress();
	}
	localDevice = localDevice->clone()->toSharedPtr();
	localDevice->setDisplayName("");
	cpimMessage.addMessageHeader(Cpim::FromHeader(cpimAddressUri(localDevice), cpimAddressDisplayName(localDevice)));
	const auto &to = message->getToAddress();
	cpimMessage.addMessageHeader(Cpim::ToHeader(cpimAddressUri(to), cpimAddressDisplayName(to)));
	cpimMessage.addMessageHeader(Cpim::DateTimeHeader(message->getTime()));

	bool linphoneNamespaceHeaderSet = false;
	if (message->getPrivate()->getPositiveDeliveryNotificationRequired() ||
	    message->getPrivate()->getNegativeDeliveryNotificationRequired() ||
	    message->getPrivate()->getDisplayNotificationRequired()) {
		if (message->isEphemeral()) {
			long time = message->getEphemeralLifetime();
			const string &buf = Utils::toString(time);
			cpimMessage.addMessageHeader(Cpim::NsHeader(linphoneNamespaceTag, linphoneNamespace));
			cpimMessage.addMessageHeader(Cpim::GenericHeader(linphoneNamespace + "." + linphoneEphemeralHeader, buf));
			linphoneNamespaceHeaderSet = true;
		}

		cpimMessage.addMessageHeader(Cpim::NsHeader(imdnNamespaceUrn, imdnNamespace));

		const string &previousToken = message->getImdnMessageId();
		if (previousToken.empty()) {
			char token[13];
			belle_sip_random_token(token, sizeof(token));
			cpimMessage.addMessageHeader(Cpim::GenericHeader(imdnNamespace + "." + imdnMessageIdHeader, token));
			message->getPrivate()->setImdnMessageId(token);
		} else {
			cpimMessage.addMessageHeader(Cpim::GenericHeader(imdnNamespace + "." + imdnMessageIdHeader, previousToken));
		}

		const string &forwardInfo = message->getForwardInfo();
		if (!forwardInfo.empty()) {
			cpimMessage.addMessageHeader(Cpim::GenericHeader(imdnNamespace + "." + imdnForwardInfoHeader, forwardInfo));
		}

		const string &replyToMessageId = message->getReplyToMessageId();
		if (!replyToMessageId.empty()) {
			if (!linphoneNamespaceHeaderSet) {
				cpimMessage.addMessageHeader(Cpim::NsHeader(linphoneNamespaceTag, linphoneNamespace));
				linphoneNamespaceHeaderSet = true;
			}
			cpimMessage.addMessageHeader(
			    Cpim::GenericHeader(linphoneNamespace + "." + linphoneReplyingToMessageIdHeader, replyToMessageId));
			const std::shared_ptr<Address> &senderAddress = message->getReplyToSenderAddress();
			string address = senderAddress->toString();
			cpimMessage.addMessageHeader(
			    Cpim::GenericHeader(linphoneNamespace + "." + linphoneReplyingToMessageSenderHeader, address));
		}

		vector<string> dispositionNotificationValues;
		if (message->getPrivate()->getPositiveDeliveryNotificationRequired())
			dispositionNotificationValues.emplace_back("positive-delivery");
		if (message->getPrivate()->getNegativeDeliveryNotificationRequired())
			dispositionNotificationValues.emplace_back("negative-delivery");
		if (message->getPrivate()->getDisplayNotificationRequired())
			dispositionNotificationValues.emplace_back("display");
		cpimMessage.addMessageHeader(Cpim::GenericHeader(imdnNamespace + "." + imdnDispositionNotificationHeader,
		                                                 Utils::join(dispositionNotificationValues, ", ")));
	}

	const string &reactionToMessageId = message->getReactionToMessageId();
	if (!reactionToMessageId.empty()) {
		if (!linphoneNamespaceHeaderSet) { // If message is ephemeral linphone namespace has already been set
			cpimMessage.addMessageHeader(Cpim::NsHeader(linphoneNamespaceTag, linphoneNamespace));
			linphoneNamespaceHeaderSet = true;
		}
		cpimMessage.addMessageHeader(
		    Cpim::GenericHeader(linphoneNamespace + "." + linphoneReactionToMessageIdHeader, reactionToMessageId));
		cpimMessage.addContentHeader(Cpim::GenericHeader("Content-Disposition", "Reaction"));
	}

	const Content *content;
	if (!message->getInternalContent().isEmpty()) {
		// Another ChatMessageModifier was called before this one, we apply our changes on the private content
		content = &(message->getInternalContent());
	} else {
		// We're the first ChatMessageModifier to be called, we'll create the private content from the public one
		// We take the first one because if there is more of them, the multipart modifier should have been called first
		// So we should not be in this block
		content = message->getContents().front().get();
	}

	const string contentBody = content->getBodyAsUtf8String();
	if (reactionToMessageId.empty()) {
		if (content->getContentDisposition().isValid()) {
			cpimMessage.addContentHeader(
			    Cpim::GenericHeader("Content-Disposition", content->getContentDisposition().asString()));
		}
	}
	cpimMessage.addContentHeader(Cpim::GenericHeader("Content-Type", content->getContentType().getMediaType()));
	cpimMessage.addContentHeader(Cpim::GenericHeader("Content-Length", Utils::toString(contentBody.size())));
	cpimMessage.setContent(contentBody);

	Content newContent;
	newContent.setContentType(ContentType::Cpim);
	newContent.setBodyFromUtf8(cpimMessage.asString());
	message->setInternalContent(newContent);

	return ChatMessageModifier::Result::Done;
}

ChatMessageModifier::Result CpimChatMessageModifier::decode(const shared_ptr<ChatMessage> &message, int &errorCode) {
	const Content *content = nullptr;
	if (!message->getInternalContent().isEmpty()) content = &(message->getInternalContent());
	else if (message->getContents().size() > 0) content = message->getContents().front().get();

	if (content == nullptr) {
		lError() << "[CPIM] Couldn't find a valid content in the message";
		errorCode = 503; // IO error
		return ChatMessageModifier::Result::Error;
	}

	if (content->getContentType() != ContentType::Cpim) {
		lError() << "[CPIM] Message is not CPIM but " << content->getContentType();
		return ChatMessageModifier::Result::Skipped;
	}

	const string contentBody = content->getBodyAsUtf8String();
	const shared_ptr<const Cpim::Message> cpimMessage = Cpim::Message::createFromString(contentBody);
	if (!cpimMessage || !cpimMessage->getMessageHeader("From") || !cpimMessage->getMessageHeader("To")) {
		lError() << "[CPIM] Message is invalid: " << contentBody;
		errorCode = 488; // Not Acceptable
		return ChatMessageModifier::Result::Error;
	}

	Content newContent;
	auto contentTypeHeader = cpimMessage->getContentHeader("Content-Type");
	if (!contentTypeHeader) {
		lError() << "[CPIM] No Content-type for the content of the message";
		errorCode = 488; // Not Acceptable
		return ChatMessageModifier::Result::Error;
	}
	newContent.setContentType(ContentType(contentTypeHeader->getValue()));
	auto contentDispositionHeader = cpimMessage->getContentHeader("Content-Disposition");
	if (contentDispositionHeader)
		newContent.setContentDisposition(ContentDisposition(contentDispositionHeader->getValue()));
	newContent.setBodyFromUtf8(cpimMessage->getContent());
	newContent.setContentEncoding(content->getContentEncoding());

	message->getPrivate()->setPositiveDeliveryNotificationRequired(false);
	message->getPrivate()->setNegativeDeliveryNotificationRequired(false);
	message->getPrivate()->setDisplayNotificationRequired(false);

	string imdnNsName = "";
	string linphoneNsName = "";
	auto messageHeaders = cpimMessage->getMessageHeaders();
	if (messageHeaders) {
		for (const auto &header : *messageHeaders.get()) {
			if (header->getName() != "NS") continue;
			auto nsHeader = static_pointer_cast<const Cpim::NsHeader>(header);
			if (nsHeader->getUri() == imdnNamespaceUrn) {
				imdnNsName = nsHeader->getPrefixName();
			} else if (nsHeader->getUri() == linphoneNamespaceTag) {
				linphoneNsName = nsHeader->getPrefixName();
			}
		}
	}

	auto fromHeader = static_pointer_cast<const Cpim::FromHeader>(cpimMessage->getMessageHeader("From"));
	std::shared_ptr<Address> cpimFromAddress = Address::create(fromHeader->getValue());
	auto dateTimeHeader = static_pointer_cast<const Cpim::DateTimeHeader>(cpimMessage->getMessageHeader("DateTime"));
	if (dateTimeHeader) message->getPrivate()->setTime(dateTimeHeader->getTime());

	auto messageIdHeader = cpimMessage->getMessageHeader(
	    imdnMessageIdHeader); // TODO: For compatibility when imdn namespace wasn't set, to remove
	if (!imdnNsName.empty()) {
		if (!messageIdHeader) messageIdHeader = cpimMessage->getMessageHeader(imdnMessageIdHeader, imdnNsName);
		auto dispositionNotificationHeader =
		    cpimMessage->getMessageHeader(imdnDispositionNotificationHeader, imdnNsName);
		if (dispositionNotificationHeader) {
			vector<string> values = bctoolbox::Utils::split(dispositionNotificationHeader->getValue(), ", ");
			for (const auto &value : values) {
				string trimmedValue = Utils::trim(
				    value); // Might be better to have a Disposition-Notification parser from the CPIM parser
				if (trimmedValue == "positive-delivery")
					message->getPrivate()->setPositiveDeliveryNotificationRequired(true);
				else if (trimmedValue == "negative-delivery")
					message->getPrivate()->setNegativeDeliveryNotificationRequired(true);
				else if (trimmedValue == "display") message->getPrivate()->setDisplayNotificationRequired(true);
				else lError() << "Unknown Disposition-Notification value [" << trimmedValue << "]";
			}
		}
		auto forwardInfoHeader = cpimMessage->getMessageHeader(imdnForwardInfoHeader, imdnNsName);
		if (forwardInfoHeader) {
			message->getPrivate()->setForwardInfo(forwardInfoHeader->getValue());
		}
	}

	if (!linphoneNsName.empty()) {
		auto timeHeader = cpimMessage->getMessageHeader(linphoneEphemeralHeader, linphoneNsName);
		if (timeHeader) {
			long time = (long)Utils::stod(timeHeader->getValue());
			message->getPrivate()->enableEphemeralWithTime(time);
		}

		auto replyToMessageIdHeader = cpimMessage->getMessageHeader(linphoneReplyingToMessageIdHeader, linphoneNsName);
		auto replyToSenderHeader = cpimMessage->getMessageHeader(linphoneReplyingToMessageSenderHeader, linphoneNsName);
		if (replyToMessageIdHeader && replyToSenderHeader) {
			message->getPrivate()->setReplyToMessageIdAndSenderAddress(
			    replyToMessageIdHeader->getValue(), Address::create(replyToSenderHeader->getValue()));
		}

		auto reactionToMessageIdHeader =
		    cpimMessage->getMessageHeader(linphoneReactionToMessageIdHeader, linphoneNsName);
		const char *expected_disposition_header = "Reaction";
		if (reactionToMessageIdHeader && contentDispositionHeader) {
			string dispositionHeader = contentDispositionHeader->getValue();
			if (!dispositionHeader.empty() && strcasecmp(expected_disposition_header, dispositionHeader.c_str()) == 0) {
				message->getPrivate()->setReactionToMessageId(reactionToMessageIdHeader->getValue());
			}
		}
	}

	if (messageIdHeader) message->getPrivate()->setImdnMessageId(messageIdHeader->getValue());

	// Discard message if sender authentication is enabled and failed
	if (message->getPrivate()->senderAuthenticationEnabled) {
		if (*cpimFromAddress == message->getAuthenticatedFromAddress()) {
			lInfo() << "[CPIM] Sender authentication successful";
		} else {
			lWarning() << "[CPIM] Sender authentication failed";
			errorCode = 488;
			return ChatMessageModifier::Result::Error;
		}
	}

	// Modify the initial message since there was no error
	message->setInternalContent(newContent);
	if (cpimFromAddress && cpimFromAddress->isValid()) message->getPrivate()->forceFromAddress(cpimFromAddress);

	return ChatMessageModifier::Result::Done;
}

string CpimChatMessageModifier::cpimAddressDisplayName(const std::shared_ptr<Address> &addr) const {
	return addr->getDisplayName();
}

string CpimChatMessageModifier::cpimAddressUri(const std::shared_ptr<Address> &addr) const {
	return addr->asStringUriOnly();
}

std::shared_ptr<Content>
CpimChatMessageModifier::createMinimalCpimContentForLimeMessage(const shared_ptr<ChatMessage> &message) const {
	shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();
	const auto &account = chatRoom->getAccount();
	if (!account) {
		lWarning() << "Unable to create CPIM because the account attached to the message is unknown";
		return Content::create();
	}
	const auto &accountContactAddress = account->getContactAddress();
	const auto &localDevice =
	    accountContactAddress ? accountContactAddress : chatRoom->getConferenceId().getLocalAddress();
	if (!localDevice) {
		lWarning() << "Unable to create CPIM because " << *account
		           << " has not registered yet and the chatroom local address is unknown";
		return Content::create();
	}
	Cpim::Message cpimMessage;
	const string &localDeviceId = localDevice->asStringUriOnly();
	cpimMessage.addMessageHeader(Cpim::FromHeader(localDeviceId, cpimAddressDisplayName(message->getToAddress())));
	cpimMessage.addMessageHeader(Cpim::NsHeader(imdnNamespaceUrn, imdnNamespace));
	cpimMessage.addMessageHeader(
	    Cpim::GenericHeader(imdnNamespace + "." + imdnMessageIdHeader, message->getImdnMessageId()));
	cpimMessage.addContentHeader(Cpim::GenericHeader("Content-Type", ContentType::PlainText.getMediaType()));

	auto cpimContent = Content::create();
	cpimContent->setContentType(ContentType::Cpim);
	cpimContent->setBodyFromLocale(cpimMessage.asString());

	return cpimContent;
}

std::shared_ptr<LinphonePrivate::Address>
CpimChatMessageModifier::parseFromHeaderCpimContentInLimeMessage(const std::shared_ptr<ChatMessage> &message) const {
	const Content *content = nullptr;
	if (!message->getInternalContent().isEmpty()) content = &(message->getInternalContent());
	else if (message->getContents().size() > 0) content = message->getContents().front().get();

	if (content == nullptr) {
		return nullptr;
	}
	list<Content> contentList = ContentManager::multipartToContentList(*content);

	for (const auto &c : contentList) {
		if (c.getContentType() != ContentType::Cpim) continue;
		const string contentBody = c.getBodyAsString();
		const shared_ptr<const Cpim::Message> cpimMessage = Cpim::Message::createFromString(contentBody);
		if (cpimMessage && cpimMessage->getMessageHeader("From")) {
			return Address::create(
			    static_pointer_cast<const Cpim::FromHeader>(cpimMessage->getMessageHeader("From"))->getValue());
		}
	}
	return nullptr;
}
std::string CpimChatMessageModifier::parseMinimalCpimContentInLimeMessage(const std::shared_ptr<ChatMessage> &message,
                                                                          const Content &content) const {
	const string contentBody = content.getBodyAsString();
	const shared_ptr<const Cpim::Message> cpimMessage = Cpim::Message::createFromString(contentBody);
	if (!cpimMessage || !cpimMessage->getMessageHeader("From")) {
		lError() << "[CPIM] Message is invalid: " << contentBody;
		return "";
	}

	string imdnNsName = "";
	auto messageHeaders = cpimMessage->getMessageHeaders();
	if (messageHeaders) {
		for (const auto &header : *messageHeaders.get()) {
			if (header->getName() != "NS") continue;
			auto nsHeader = static_pointer_cast<const Cpim::NsHeader>(header);
			if (nsHeader->getUri() == imdnNamespaceUrn) {
				imdnNsName = nsHeader->getPrefixName();
			}
		}
	}

	auto messageIdHeader = cpimMessage->getMessageHeader(imdnMessageIdHeader);
	if (!imdnNsName.empty()) {
		if (!messageIdHeader) {
			messageIdHeader = cpimMessage->getMessageHeader(imdnMessageIdHeader, imdnNsName);
		}
	}

	if (messageIdHeader) {
		lInfo() << "[CPIM] Found Message ID: " << messageIdHeader->getValue();
		message->getPrivate()->setImdnMessageId(messageIdHeader->getValue());
	}

	auto fromHeader = static_pointer_cast<const Cpim::FromHeader>(cpimMessage->getMessageHeader("From"));
	return fromHeader->getValue();
}

LINPHONE_END_NAMESPACE
