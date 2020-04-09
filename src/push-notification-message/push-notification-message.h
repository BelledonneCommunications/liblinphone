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

#pragma once

#include <belle-sip/object++.hh>
#include "linphone/api/c-types.h"
#include "address/address.h"

LINPHONE_BEGIN_NAMESPACE

class PushNotificationMessage : public bellesip::HybridObject<LinphonePushNotificationMessage, PushNotificationMessage> {
    public:
    PushNotificationMessage(bool isUsingUserDefaults, const std::string &callId, bool isText, const std::string &textContent, const std::string &subject, const std::string &fromAddr, const std::string &localAddr, const std::string &peerAddr);

    void init(bool isUsingUserDefaults, const std::string &callId, bool isText, const std::string &textContent, const std::string &subject, const std::string &fromAddr, const std::string &localAddr, const std::string &peerAddr);

    bool isUsingUserDefaults() const;
    const std::string& getCallId() const;
    bool isText() const;
    const std::string& getTextContent() const;
    const std::string& getSubject() const;
    std::shared_ptr<Address> getFromAddr() const;
    std::shared_ptr<Address> getLocalAddr() const;
    std::shared_ptr<Address> getPeerAddr() const;

    std::string toString() const override;

    private:
        bool mIsUsingUserDefaults;
        std::string mCallId;
        bool mIsText;
        std::string mTextContent;
        std::string mSubject;
        std::string mFromAddr;
        std::string mLocalAddr;
        std::string mPeerAddr;
};

LINPHONE_END_NAMESPACE
