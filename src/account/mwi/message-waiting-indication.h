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

#ifndef _L_MESSAGE_WAITING_INDICATION_H_
#define _L_MESSAGE_WAITING_INDICATION_H_

#include "c-wrapper/c-wrapper.h"
#include "linphone/api/c-message-waiting-indication.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

namespace Mwi {

class MessageWaitingIndicationSummary;

class LINPHONE_PUBLIC MessageWaitingIndication
    : public bellesip::HybridObject<LinphoneMessageWaitingIndication, MessageWaitingIndication> {
public:
	MessageWaitingIndication();
	MessageWaitingIndication(const MessageWaitingIndication &other);
	virtual ~MessageWaitingIndication();

	MessageWaitingIndication *clone() const override;

	// Friends
	friend const bctbx_list_t * ::linphone_message_waiting_indication_get_summaries(
	    const LinphoneMessageWaitingIndication *mwi);

	// Getters
	bool hasMessageWaiting() const;
	std::shared_ptr<Address> getAccountAddress() const;
	std::list<std::shared_ptr<MessageWaitingIndicationSummary>> getSummaries() const;
	std::shared_ptr<MessageWaitingIndicationSummary>
	getSummary(LinphoneMessageWaitingIndicationContextClass contextClass) const;

	// Setters
	void setMessageWaiting(bool messageWaiting);
	void setAccountAddress(std::shared_ptr<Address> accountAddress);

	// Other
	void addSummary(const std::shared_ptr<MessageWaitingIndicationSummary> summary);
	std::shared_ptr<Content> toContent() const;
	static std::shared_ptr<MessageWaitingIndication> parse(const Content &content);

private:
	bool mHasMessageWaiting;
	std::shared_ptr<Address> mAccountAddress;
	std::list<std::shared_ptr<MessageWaitingIndicationSummary>> mSummaries;
	mutable bctbx_list_t *mBctbxSummaries = nullptr;
};

} // namespace Mwi

LINPHONE_END_NAMESPACE

#endif // ifndef _L_MESSAGE_WAITING_INDICATION_H_
