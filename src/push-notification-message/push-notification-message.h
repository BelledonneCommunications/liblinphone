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

#pragma once

#include "address/address.h"
#include "belle-sip/object++.hh"
#include "linphone/api/c-types.h"

LINPHONE_BEGIN_NAMESPACE

class PushNotificationMessage
    : public bellesip::HybridObject<LinphonePushNotificationMessage, PushNotificationMessage> {
public:
	PushNotificationMessage(const std::string &callId,
	                        bool isText,
	                        const std::string &textContent,
	                        const std::string &subject,
	                        const std::string &fromAddr,
	                        const std::string &localAddr,
	                        const std::string &peerAddr,
	                        const std::string &reactionContent,
	                        bool isIcalendar,
	                        bool isConferenceInvitationNew,
	                        bool isConferenceInvitationUpdate,
	                        bool isConferenceInvitationCancellation);

	void init(const std::string &callId,
	          bool isText,
	          const std::string &textContent,
	          const std::string &subject,
	          const std::string &fromAddr,
	          const std::string &localAddr,
	          const std::string &peerAddr,
	          const std::string &reactionContent,
	          bool isIcalendar,
	          bool isConferenceInvitationNew,
	          bool isConferenceInvitationUpdate,
	          bool isConferenceInvitationCancellation);

	const std::string &getCallId() const;
	bool isText() const;
	const std::string &getTextContent() const;
	const std::string &getSubject() const;
	std::shared_ptr<const Address> getFromAddr() const;
	std::shared_ptr<const Address> getLocalAddr() const;
	std::shared_ptr<const Address> getPeerAddr() const;
	const std::string &getReactionContent() const;
	bool isIcalendar() const;
	bool isConferenceInvitationNew() const;
	bool isConferenceInvitationUpdate() const;
	bool isConferenceInvitationCancellation() const;

	std::string toString() const override;

private:
	std::string mCallId;
	bool mIsText = false;
	std::string mTextContent;
	std::string mSubject;
	std::shared_ptr<Address> mFromAddr;
	std::shared_ptr<Address> mLocalAddr;
	std::shared_ptr<Address> mPeerAddr;
	std::string mReactionContent;
	bool mIsIcalendar = false;
	bool mIsConferenceInvitationNew = false;
	bool mIsConferenceInvitationUpdate = false;
	bool mIsConferenceInvitationCancellation = false;
};

LINPHONE_END_NAMESPACE
