/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#include <bctoolbox/defs.h>

#include "message-waiting-indication-summary.h"
#include "message-waiting-indication.h"
#include "parser/mwi-parser.h"

// =============================================================================

using namespace std;

using namespace LinphonePrivate;

LINPHONE_BEGIN_NAMESPACE

namespace Mwi {

MessageWaitingIndication::MessageWaitingIndication() {
}

MessageWaitingIndication::~MessageWaitingIndication() {
	if (mBctbxSummaries) bctbx_list_free(mBctbxSummaries);
}

MessageWaitingIndication *MessageWaitingIndication::clone() const {
	return nullptr;
}

// -----------------------------------------------------------------------------

bool MessageWaitingIndication::hasMessageWaiting() const {
	return mHasMessageWaiting;
}

std::shared_ptr<Address> MessageWaitingIndication::getAccountAddress() const {
	return mAccountAddress;
}

std::list<std::shared_ptr<MessageWaitingIndicationSummary>> MessageWaitingIndication::getSummaries() const {
	return mSummaries;
}

std::shared_ptr<MessageWaitingIndicationSummary>
MessageWaitingIndication::getSummary(LinphoneMessageWaitingIndicationContextClass contextClass) const {
	const auto found = std::find_if(std::begin(mSummaries), std::end(mSummaries),
	                                [&](const auto &elem) { return elem->getContextClass() == contextClass; });
	return (found == std::end(mSummaries)) ? nullptr : *found;
}

// -----------------------------------------------------------------------------

void MessageWaitingIndication::setMessageWaiting(bool messageWaiting) {
	mHasMessageWaiting = messageWaiting;
}

void MessageWaitingIndication::setAccountAddress(std::shared_ptr<Address> accountAddress) {
	mAccountAddress = accountAddress;
}

// -----------------------------------------------------------------------------

void MessageWaitingIndication::addSummary(const std::shared_ptr<MessageWaitingIndicationSummary> summary) {
	mSummaries.push_back(summary);
	mBctbxSummaries = bctbx_list_append(mBctbxSummaries, summary->toC());
}

// -----------------------------------------------------------------------------

std::shared_ptr<MessageWaitingIndication> MessageWaitingIndication::parse(const Content &content) {
	if (content.getContentType() != ContentType::Mwi) return nullptr;

	return Mwi::Parser::getInstance()->parseMessageSummary(content.getBodyAsString());
}

} // namespace Mwi

LINPHONE_END_NAMESPACE
