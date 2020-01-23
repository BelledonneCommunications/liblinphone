/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "linphone/utils/utils.h"

#include "address/address.h"
#include "chat/chat-message/chat-message-p.h"
#include "chat/cpim/cpim.h"
#include "content/content.h"
#include "content/content-disposition.h"
#include "content/content-type.h"
#include "logger/logger.h"

#include "cpim-chat-message-modifier.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// Following https://tools.ietf.org/html/rfc4151
const string linphoneNamespaceTag = "tag:linphone.org,2020:groupchat";
const string linphoneNamespace = "linphone";
const string linphoneEphemeralHeader = "Ephemeral-Time";

const string imdnNamespaceUrn = "urn:ietf:params:imdn";
const string imdnNamespace = "imdn";
const string imdnMessageIdHeader = "Message-ID";
const string imdnForwardInfoHeader = "Forward-Info";
const string imdnDispositionNotificationHeader = "Disposition-Notification";

ChatMessageModifier::Result CpimChatMessageModifier::encode (const shared_ptr<ChatMessage> &message, int &errorCode) {

	Cpim::Message cpimMessage;

	cpimMessage.addMessageHeader(
		Cpim::FromHeader(cpimAddressUri(message->getFromAddress()), cpimAddressDisplayName(message->getFromAddress()))
	);
	cpimMessage.addMessageHeader(
		Cpim::ToHeader(cpimAddressUri(message->getToAddress()), cpimAddressDisplayName(message->getToAddress()))
	);
	cpimMessage.addMessageHeader(
		Cpim::DateTimeHeader(message->getTime())
	);

	if (message->getPrivate()->getPositiveDeliveryNotificationRequired()
		|| message->getPrivate()->getNegativeDeliveryNotificationRequired()
		|| message->getPrivate()->getDisplayNotificationRequired()
	) {
		if (message->isEphemeral()) {
			long time = message->getEphemeralLifetime();
			const string &buf = Utils::toString(time);
			cpimMessage.addMessageHeader(Cpim::NsHeader(linphoneNamespaceTag, linphoneNamespace));
			cpimMessage.addMessageHeader(Cpim::GenericHeader(linphoneNamespace + "." + linphoneEphemeralHeader, buf));
		}

		cpimMessage.addMessageHeader(Cpim::NsHeader(imdnNamespaceUrn, imdnNamespace));

		const string &previousToken = message->getImdnMessageId();
		if (previousToken.empty()) {
			char token[13];
			belle_sip_random_token(token, sizeof(token));
			cpimMessage.addMessageHeader(
				Cpim::GenericHeader(imdnNamespace + "." + imdnMessageIdHeader, token)
			);
			message->getPrivate()->setImdnMessageId(token);
		} else {
			cpimMessage.addMessageHeader(
				Cpim::GenericHeader(imdnNamespace + "." + imdnMessageIdHeader, previousToken)
			);
		}
		
		const string &forwardInfo = message->getForwardInfo();
		if (!forwardInfo.empty()) {
			cpimMessage.addMessageHeader(
				Cpim::GenericHeader(imdnNamespace + "." + imdnForwardInfoHeader, forwardInfo)
			);
		}

		vector<string> dispositionNotificationValues;
		if (message->getPrivate()->getPositiveDeliveryNotificationRequired())
			dispositionNotificationValues.emplace_back("positive-delivery");
		if (message->getPrivate()->getNegativeDeliveryNotificationRequired())
			dispositionNotificationValues.emplace_back("negative-delivery");
		if (message->getPrivate()->getDisplayNotificationRequired())
			dispositionNotificationValues.emplace_back("display");
		cpimMessage.addMessageHeader(
			Cpim::GenericHeader(
				imdnNamespace + "." + imdnDispositionNotificationHeader,
				Utils::join(dispositionNotificationValues, ", ")
			)
		);
	}

	const Content *content;
	if (!message->getInternalContent().isEmpty()) {
		// Another ChatMessageModifier was called before this one, we apply our changes on the private content
		content = &(message->getInternalContent());
	} else {
		// We're the first ChatMessageModifier to be called, we'll create the private content from the public one
		// We take the first one because if there is more of them, the multipart modifier should have been called first
		// So we should not be in this block
		content = message->getContents().front();
	}

	const string contentBody = content->getBodyAsString();
	if (content->getContentDisposition().isValid()) {
		cpimMessage.addContentHeader(
			Cpim::GenericHeader("Content-Disposition", content->getContentDisposition().asString())
		);
	}
	cpimMessage.addContentHeader(
		Cpim::GenericHeader("Content-Type", content->getContentType().asString())
	);
	cpimMessage.addContentHeader(
		Cpim::GenericHeader("Content-Length", Utils::toString(contentBody.size()))
	);
	cpimMessage.setContent(contentBody);

	Content newContent;
	newContent.setContentType(ContentType::Cpim);
	newContent.setBody(cpimMessage.asString());
	message->setInternalContent(newContent);

	return ChatMessageModifier::Result::Done;
}

ChatMessageModifier::Result CpimChatMessageModifier::decode (const shared_ptr<ChatMessage> &message, int &errorCode) {
	const Content *content;
	if (!message->getInternalContent().isEmpty())
		content = &(message->getInternalContent());
	else
		content = message->getContents().front();

	if (content->getContentType() != ContentType::Cpim) {
		lError() << "[CPIM] Message is not CPIM but " << content->getContentType();
		return ChatMessageModifier::Result::Skipped;
	}

	const string contentBody = content->getBodyAsString();
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
	newContent.setBody(cpimMessage->getContent());

	message->getPrivate()->setPositiveDeliveryNotificationRequired(false);
	message->getPrivate()->setNegativeDeliveryNotificationRequired(false);
	message->getPrivate()->setDisplayNotificationRequired(false);

	string imdnNamespace = "";
	string targetNamespace = "";
	auto messageHeaders = cpimMessage->getMessageHeaders();
	int namespaceNum = 0;
	if (messageHeaders) {
		for (const auto &header : *messageHeaders.get()) {
			if (header->getName() != "NS")
				continue;
			auto nsHeader = static_pointer_cast<const Cpim::NsHeader>(header);
			if (nsHeader->getUri() == imdnNamespaceUrn) {
				imdnNamespace = nsHeader->getPrefixName();
				namespaceNum ++;
			} else if (nsHeader->getUri() == linphoneNamespaceTag) {
				targetNamespace = nsHeader->getPrefixName();
				namespaceNum ++;
			}
			if (namespaceNum > 1) {
				break;
			}
		}
	}

	auto fromHeader = static_pointer_cast<const Cpim::FromHeader>(cpimMessage->getMessageHeader("From"));
	Address cpimFromAddress(fromHeader->getValue());
	auto toHeader = static_pointer_cast<const Cpim::ToHeader>(cpimMessage->getMessageHeader("To"));
	Address cpimToAddress(toHeader->getValue());
	auto dateTimeHeader = static_pointer_cast<const Cpim::DateTimeHeader>(cpimMessage->getMessageHeader("DateTime"));
	if (dateTimeHeader)
		message->getPrivate()->setTime(dateTimeHeader->getTime());

	auto messageIdHeader = cpimMessage->getMessageHeader(imdnMessageIdHeader); // TODO: For compatibility when imdn namespace wasn't set, to remove
	if (!imdnNamespace.empty()) {
		if (!messageIdHeader)
			messageIdHeader = cpimMessage->getMessageHeader(imdnMessageIdHeader, imdnNamespace);
		auto dispositionNotificationHeader = cpimMessage->getMessageHeader(imdnDispositionNotificationHeader, imdnNamespace);
		if (dispositionNotificationHeader) {
			vector<string> values = Utils::split(dispositionNotificationHeader->getValue(), ", ");
			for (const auto &value : values) {
				string trimmedValue = Utils::trim(value); // Might be better to have a Disposition-Notification parser from the CPIM parser
				if (trimmedValue == "positive-delivery")
					message->getPrivate()->setPositiveDeliveryNotificationRequired(true);
				else if (trimmedValue == "negative-delivery")
					message->getPrivate()->setNegativeDeliveryNotificationRequired(true);
				else if (trimmedValue == "display")
					message->getPrivate()->setDisplayNotificationRequired(true);
				else
					lError() << "Unknown Disposition-Notification value [" << trimmedValue << "]";
			}
		}
		auto forwardInfoHeader = cpimMessage->getMessageHeader(imdnForwardInfoHeader, imdnNamespace);
		if (forwardInfoHeader) {
			message->getPrivate()->setForwardInfo(forwardInfoHeader->getValue());
		}
	}

	if (!targetNamespace.empty()) {
		auto timeHeader = cpimMessage->getMessageHeader(linphoneEphemeralHeader, targetNamespace);
		long time = (long)Utils::stod(timeHeader->getValue());
		message->getPrivate()->enableEphemeralWithTime(time);
	}

	if (messageIdHeader)
		message->getPrivate()->setImdnMessageId(messageIdHeader->getValue());

	// Discard message if sender authentication is enabled and failed
	if (message->getPrivate()->senderAuthenticationEnabled) {
		if (cpimFromAddress == message->getAuthenticatedFromAddress()) {
			lInfo() << "[CPIM] Sender authentication successful";
		} else {
			lWarning() << "[CPIM] Sender authentication failed";
			errorCode = 488;
			return ChatMessageModifier::Result::Error;
		}
	}

	// Modify the initial message since there was no error
	message->setInternalContent(newContent);
	if (cpimFromAddress.isValid())
		message->getPrivate()->forceFromAddress(cpimFromAddress);

	return ChatMessageModifier::Result::Done;
}

string CpimChatMessageModifier::cpimAddressDisplayName (const Address &addr) const {
	return addr.getDisplayName();
}

string CpimChatMessageModifier::cpimAddressUri (const Address &addr) const {
	return addr.asStringUriOnly();
}

LINPHONE_END_NAMESPACE
