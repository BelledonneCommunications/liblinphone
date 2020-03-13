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

#include "linphone/utils/general.h"
#include "core/core-accessor.h"
#include "core/core.h"
#include "push-notification-message/push-notification-message.h"

LINPHONE_BEGIN_NAMESPACE

/**
 * This interface aims at abstracting some features offered by the platform, most often mobile platforms.
 * A per platform implementation is to be made to implement these features, if available on the platform.
 */
class SharedCoreHelpers: public CoreAccessor {
public:
	virtual ~SharedCoreHelpers () = default;

	virtual	void onLinphoneCoreStop () = 0;

	virtual	bool isCoreShared() = 0;
	virtual bool canCoreStart() = 0;
	virtual void registerMainCoreMsgCallback() = 0;
	virtual std::shared_ptr<PushNotificationMessage> getPushNotificationMessage(const std::string &callId) = 0;
	virtual	std::shared_ptr<ChatRoom> getPushNotificationChatRoom(const std::string &chatRoomAddr) = 0;
	virtual void resetSharedCoreState() = 0;
	virtual void unlockSharedCoreIfNeeded() = 0;
	virtual bool isCoreStopRequired() = 0;
	virtual void onMsgWrittenInUserDefaults() = 0;

    virtual void *getPathContext () = 0;

protected:
	inline explicit SharedCoreHelpers (std::shared_ptr<LinphonePrivate::Core> core) : CoreAccessor(core) {}
};

class GenericSharedCoreHelpers : public SharedCoreHelpers {
public:
	explicit GenericSharedCoreHelpers (std::shared_ptr<LinphonePrivate::Core> core);
	~GenericSharedCoreHelpers () = default;

	void onLinphoneCoreStop () override;

	bool isCoreShared() override;
	bool canCoreStart() override;
	void registerMainCoreMsgCallback() override;
	std::shared_ptr<PushNotificationMessage> getPushNotificationMessage(const std::string &callId) override;
	std::shared_ptr<ChatRoom> getPushNotificationChatRoom(const std::string &chatRoomAddr) override;
	void resetSharedCoreState() override;
	void unlockSharedCoreIfNeeded() override;
	bool isCoreStopRequired() override;
	void onMsgWrittenInUserDefaults() override;

    void *getPathContext () override;
};

std::shared_ptr<SharedCoreHelpers> createIosSharedCoreHelpers (std::shared_ptr<LinphonePrivate::Core> core);
void uninitSharedCore(LinphoneCore *lc);

LINPHONE_END_NAMESPACE
