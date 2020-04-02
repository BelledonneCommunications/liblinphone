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

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif
#if TARGET_OS_IPHONE

#include <Foundation/Foundation.h>
#include <mutex>
#include <set>

#include "linphone/utils/general.h"
#include "linphone/utils/utils.h"
#include "chat/chat-room/chat-room.h"
#include "c-wrapper/c-wrapper.h"

#include "logger/logger.h"
#include "shared-core-helpers.h"

// TODO: Remove me
#include "private.h"

#define ACTIVE_SHARED_CORE "ACTIVE_SHARED_CORE"
#define LAST_UPDATE_TIME_SHARED_CORE "LAST_UPDATE_TIME_SHARED_CORE"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

typedef enum {
	noCoreStarted,
	mainCoreStarted,
	executorCoreStarted,
	executorCoreStopping, // A Main Core needs to start. Executor Cores have to stop.
	executorCoreStopped // A Main Core has stopped the Executor Cores. Only a Main Core can start here.
} SharedCoreState;

string sharedStateToString(int state) {
   switch(state) {
      case noCoreStarted:
         return "noCoreStarted";
      case mainCoreStarted:
         return "mainCoreStarted";
      case executorCoreStarted:
         return "executorCoreStarted";
	case executorCoreStopping:
		 return "executorCoreStopping";
	case executorCoreStopped:
		 return "executorCoreStopped";
		default:
         return "Invalid state";
   }
}

void on_core_must_stop(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo);
static void unlock_shared_core_if_needed(CFRunLoopTimerRef timer, void *info);

class IosSharedCoreHelpers : public GenericSharedCoreHelpers {
public:
	IosSharedCoreHelpers (std::shared_ptr<LinphonePrivate::Core> core);
	~IosSharedCoreHelpers () = default;

	void onLinphoneCoreStop () override;
    void *getPathContext () override;

	std::shared_ptr<ChatMessage> getPushNotificationMessage(const string &callId) override;
	std::shared_ptr<ChatMessage> processPushNotificationMessage(const string &callId);
	std::shared_ptr<ChatRoom> getPushNotificationChatRoom(const string &chatRoomAddr) override;
	std::shared_ptr<ChatRoom> processPushNotificationChatRoom(const string &chatRoomAddr);

	// shared core
	bool isCoreShared() override;
	bool canCoreStart() override;
	void onCoreMustStop();
	void resetSharedCoreState() override;
	void unlockSharedCoreIfNeeded() override;
	bool isCoreStopRequired() override;

	// push notif
	void clearCallIdList();
	void addNewCallIdToList(string callId);
	void removeCallIdFromList(string callId);
	void setChatRoomInvite(std::shared_ptr<ChatRoom> chatRoom);
	std::string getChatRoomAddr();
	void reinitTimer();

private:
	// shared core
	void setupSharedCore(struct _LpConfig *config);
	bool isSharedCoreStarted();
	SharedCoreState getSharedCoreState();
	void setSharedCoreState(SharedCoreState sharedCoreState);
	void reloadConfig();
	void resetSharedCoreLastUpdateTime();
	NSInteger getSharedCoreLastUpdateTime();

	// shared core : executor
	bool canExecutorCoreStart();
	void subscribeToMainCoreNotifs();

	// shared core : main
	bool canMainCoreStart();
	void stopSharedCores();

	//push notif
	std::shared_ptr<ChatMessage> getChatMsgAndUpdateList(const string &callId);

	std::string mAppGroupId = "";
	static set<string> callIdList;
	static std::mutex callIdListMutex;
	static std::mutex executorCoreMutex;
	static std::mutex userDefaultMutex;
	std::shared_ptr<ChatRoom> mChatRoomInvite = nullptr;
	std::string mChatRoomAddr;
	uint64_t mTimer = 0;
	CFRunLoopTimerRef mUnlockTimer;
	uint64_t mMaxIterationTimeMs;

};

set<string> IosSharedCoreHelpers::callIdList = set<string>();
std::mutex IosSharedCoreHelpers::callIdListMutex;
std::mutex IosSharedCoreHelpers::executorCoreMutex;
std::mutex IosSharedCoreHelpers::userDefaultMutex;


// =============================================================================


IosSharedCoreHelpers::IosSharedCoreHelpers (std::shared_ptr<LinphonePrivate::Core> core) : GenericSharedCoreHelpers(core) {
    struct _LpConfig *config = core->getCCore()->config;
	ms_message("[SHARED] setup IosSharedCoreHelpers");
	mAppGroupId = linphone_config_get_string(config, "shared_core", "app_group_id", "");

	if (isCoreShared()) {
		/* by default iterate for 25 sec max. After 30 sec the iOS app extension can be killed by the system */
		mMaxIterationTimeMs = (uint64_t)linphone_config_get_int(config, "shared_core", "max_iteration_time_ms", 25000);

		CFRunLoopTimerContext timerContext;

		timerContext.version = 0;
		timerContext.info = this;
		timerContext.retain = NULL;
		timerContext.release = NULL;
		timerContext.copyDescription = NULL;

		CFTimeInterval interval = 10.0f; // 10 sec
		mUnlockTimer = CFRunLoopTimerCreate (NULL,
			CFAbsoluteTimeGetCurrent() + interval,
			interval,
			0,
			0,
			unlock_shared_core_if_needed,
			&timerContext
		);

		/* use an iOS timer instead of belle sip timer to unlock
		potentially stuck processes that won't be able to call iterate() */
		CFRunLoopAddTimer (CFRunLoopGetCurrent(), mUnlockTimer, kCFRunLoopCommonModes);
		ms_message("[SHARED] launch timer");
		unlockSharedCoreIfNeeded();
	}
}

void IosSharedCoreHelpers::onLinphoneCoreStop () {
	if (isCoreShared()) {
		CFRunLoopTimerInvalidate(mUnlockTimer);
		ms_message("[SHARED] stop timer");

		bool needUnlock = (getSharedCoreState() == SharedCoreState::executorCoreStarted || getSharedCoreState() == SharedCoreState::executorCoreStopping);

		if ((getCore()->getCCore()->is_main_core && getSharedCoreState() == SharedCoreState::mainCoreStarted) ||
			(!getCore()->getCCore()->is_main_core && getSharedCoreState() == SharedCoreState::executorCoreStarted)) {
			setSharedCoreState(SharedCoreState::noCoreStarted);
		}

		if (!getCore()->getCCore()->is_main_core && getSharedCoreState() == SharedCoreState::executorCoreStopping) {
			setSharedCoreState(SharedCoreState::executorCoreStopped);
		}

		if (needUnlock) {
			ms_message("[push] unlock executorCoreMutex");
			IosSharedCoreHelpers::executorCoreMutex.unlock();
		}
	}
}

void *IosSharedCoreHelpers::getPathContext() {
	return mAppGroupId.empty() ? NULL : ms_strdup(mAppGroupId.c_str());
}

// -----------------------------------------------------------------------------
// push notification
// -----------------------------------------------------------------------------

std::shared_ptr<ChatMessage> IosSharedCoreHelpers::getPushNotificationMessage(const string &callId) {
	ms_message("[push] getPushNotificationMessage");
	ms_message("[push] current shared core state : %s", sharedStateToString(getSharedCoreState()).c_str());

	addNewCallIdToList(callId);
	switch(getSharedCoreState()) {
		case SharedCoreState::mainCoreStarted:
			IosSharedCoreHelpers::executorCoreMutex.unlock();
		case SharedCoreState::executorCoreStopping:
		case SharedCoreState::executorCoreStopped:
			clearCallIdList();
			return nullptr;
		case SharedCoreState::executorCoreStarted:
		case SharedCoreState::noCoreStarted:
			IosSharedCoreHelpers::executorCoreMutex.lock();
			if (getSharedCoreState() == executorCoreStopped ||
				getSharedCoreState() == mainCoreStarted ||
				getSharedCoreState() == executorCoreStopping) {
				/* this executor core can start because a main core is starting */
				return nullptr;
			}
			std::shared_ptr<ChatMessage> msg = processPushNotificationMessage(callId);
			return msg;
	}
}

std::shared_ptr<ChatRoom> IosSharedCoreHelpers::getPushNotificationChatRoom(const string &chatRoomAddr) {
	ms_message("[push] getPushNotificationInvite");
	ms_message("[push] current shared core state : %s", sharedStateToString(getSharedCoreState()).c_str());

	switch(getSharedCoreState()) {
		case SharedCoreState::mainCoreStarted:
			IosSharedCoreHelpers::executorCoreMutex.unlock();
		case SharedCoreState::executorCoreStopping:
		case SharedCoreState::executorCoreStopped:
			return nullptr;
		case SharedCoreState::executorCoreStarted:
		case SharedCoreState::noCoreStarted:
			IosSharedCoreHelpers::executorCoreMutex.lock();
			if (getSharedCoreState() == executorCoreStopped ||
				getSharedCoreState() == mainCoreStarted ||
				getSharedCoreState() == executorCoreStopping) {
				/* this executor core can start because a main core is starting */
				return nullptr;
			}
			std::shared_ptr<ChatRoom> chatRoom = processPushNotificationChatRoom(chatRoomAddr);
			return chatRoom;
	}
}

void IosSharedCoreHelpers::clearCallIdList() {
	IosSharedCoreHelpers::callIdListMutex.lock();
	IosSharedCoreHelpers::callIdList.clear();
	IosSharedCoreHelpers::callIdListMutex.unlock();
	ms_debug("[push] clear callIdList");
}

void IosSharedCoreHelpers::addNewCallIdToList(string callId) {
	IosSharedCoreHelpers::callIdListMutex.lock();
	IosSharedCoreHelpers::callIdList.insert(callId);
	IosSharedCoreHelpers::callIdListMutex.unlock();
	ms_debug("[push] add %s to callIdList if not already present", callId.c_str());
}

void IosSharedCoreHelpers::removeCallIdFromList(string callId) {
	IosSharedCoreHelpers::callIdListMutex.lock();
	if (IosSharedCoreHelpers::callIdList.erase(callId)) {
		ms_message("[push] removed %s from callIdList", callId.c_str());
	} else {
		ms_message("[push] unable to remove %s from callIdList: not found", callId.c_str());
	}
	IosSharedCoreHelpers::callIdListMutex.unlock();
}

static void on_push_notification_message_received(LinphoneCore *lc, LinphoneChatRoom *room, LinphoneChatMessage *message) {
	if (linphone_chat_message_is_text(message) || linphone_chat_message_is_file_transfer(message)) {
		ms_message("[push] msg [%p] received from chat room [%p]", message, room);
		const char *callId = linphone_chat_message_get_call_id(message);
		static_cast<LinphonePrivate::IosSharedCoreHelpers*>(lc->platform_helper)->removeCallIdFromList(callId);
	}
}

std::shared_ptr<ChatMessage> IosSharedCoreHelpers::getChatMsgAndUpdateList(const string &callId) {
	std::shared_ptr<ChatMessage> msg = getCore()->findChatMessageFromCallId(callId);
	if (msg) removeCallIdFromList(callId);
	return msg;
}

std::shared_ptr<ChatMessage> IosSharedCoreHelpers::processPushNotificationMessage(const string &callId) {
	std::shared_ptr<ChatMessage> chatMessage;
	ms_message("[push] processPushNotificationMessage for callid [%s]", callId.c_str());

	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
 	linphone_core_cbs_set_message_received(cbs, on_push_notification_message_received);
	linphone_core_add_callbacks(getCore()->getCCore(), cbs);

	if (linphone_core_get_global_state(getCore()->getCCore()) != LinphoneGlobalOn && linphone_core_start(getCore()->getCCore()) != 0) {
		linphone_core_cbs_unref(cbs);
		return nullptr;
	}
	ms_message("[push] core started");

	chatMessage = getChatMsgAndUpdateList(callId);
	ms_message("[push] message already in db? %s", chatMessage? "yes" : "no");
	if (chatMessage) {
		linphone_core_cbs_unref(cbs);
		return chatMessage;
	}

	/* init with 0 to look for the msg when newMsgNb <= 0 and then try to get the msg every seconds */
	uint64_t secondsTimer = 0;
	reinitTimer();

	while (ms_get_cur_time_ms() - mTimer < mMaxIterationTimeMs && (!IosSharedCoreHelpers::callIdList.empty() || !chatMessage)) {
		ms_message("[push] wait for msg. callIdList size = %lu", IosSharedCoreHelpers::callIdList.size());
		linphone_core_iterate(getCore()->getCCore());

		if (getSharedCoreState() == SharedCoreState::executorCoreStopping) {
			ms_message("[SHARED] executor core stopping");
			linphone_core_cbs_unref(cbs);
			return nullptr;
		}

		ms_usleep(50000);
		if (IosSharedCoreHelpers::callIdList.empty() && ms_get_cur_time_ms() - secondsTimer >= 1000) {
			chatMessage = getChatMsgAndUpdateList(callId);
			secondsTimer = ms_get_cur_time_ms();
		}
	}

	/* In case we wait for 25 seconds, there is probably a problem. So we reset the msg
	counter to prevent each push to wait 25 sec for messages that won't be received */
	if (ms_get_cur_time_ms() - mTimer >= 25000) clearCallIdList();

	chatMessage = getChatMsgAndUpdateList(callId);
	ms_message("[push] message received? %s", chatMessage? "yes" : "no");

	linphone_core_cbs_unref(cbs);
	return chatMessage;
}

void IosSharedCoreHelpers::setChatRoomInvite(std::shared_ptr<ChatRoom> chatRoom) {
	mChatRoomInvite = chatRoom;
}

std::string IosSharedCoreHelpers::getChatRoomAddr() {
	return mChatRoomAddr;
}

void IosSharedCoreHelpers::reinitTimer() {
	mTimer = ms_get_cur_time_ms();
}

static void on_push_notification_chat_room_invite_received(LinphoneCore *lc, LinphoneChatRoom *cr, LinphoneChatRoomState state) {
	if (state == LinphoneChatRoomStateCreated) {
		PlatformHelpers *platform_helper = static_cast<LinphonePrivate::PlatformHelpers*>(lc->platform_helper);
		IosSharedCoreHelpers *shared_core_helper = static_cast<LinphonePrivate::IosSharedCoreHelpers*>(platform_helper->getSharedCoreHelpers().get());
		shared_core_helper->reinitTimer();

		const char *cr_peer_addr = linphone_address_get_username(linphone_chat_room_get_peer_address(cr));
		ms_message("[push] we are added to the chat room %s", cr_peer_addr);

		if (cr_peer_addr && strcmp(cr_peer_addr, shared_core_helper->getChatRoomAddr().c_str()) == 0) {
			ms_message("[push] the chat room associated with the push is found");
			shared_core_helper->setChatRoomInvite(std::static_pointer_cast<ChatRoom>(L_GET_CPP_PTR_FROM_C_OBJECT(cr)));
		}
	}
}

std::shared_ptr<ChatRoom> IosSharedCoreHelpers::processPushNotificationChatRoom(const string &crAddr) {
	ms_message("[push] processPushNotificationInvite. looking for chatroom %s", mChatRoomAddr.c_str());
	mChatRoomAddr = crAddr;
	mChatRoomInvite = nullptr;


	if (linphone_core_get_global_state(getCore()->getCCore()) != LinphoneGlobalOn && linphone_core_start(getCore()->getCCore()) != 0) {
		return nullptr;
	}
	ms_message("[push] core started");
	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_chat_room_state_changed(cbs, on_push_notification_chat_room_invite_received);
	linphone_core_add_callbacks(getCore()->getCCore(), cbs);


	reinitTimer();
	uint64_t iterationTimer = ms_get_cur_time_ms();

	/* if the chatroom is received, iterate for 2 sec otherwise iterate mMaxIterationTimeMs seconds*/
	while ((ms_get_cur_time_ms() - iterationTimer < mMaxIterationTimeMs && !mChatRoomInvite) || (mChatRoomInvite && ms_get_cur_time_ms() - mTimer < 2000)) {
		ms_message("[push] wait chatRoom");
		linphone_core_iterate(getCore()->getCCore());

		if (getSharedCoreState() == SharedCoreState::executorCoreStopping) {
			ms_message("[SHARED] executor core stopping");
			linphone_core_cbs_unref(cbs);
			return nullptr;
		}

		ms_usleep(50000);
	}

	linphone_core_cbs_unref(cbs);
	return mChatRoomInvite;
}

// -----------------------------------------------------------------------------
// shared core
// -----------------------------------------------------------------------------

void unlock_shared_core_if_needed(CFRunLoopTimerRef timer, void *info) {
	IosSharedCoreHelpers *platform_helper = (IosSharedCoreHelpers *) info;
	platform_helper->unlockSharedCoreIfNeeded();
}

bool IosSharedCoreHelpers::isCoreShared() {
	return !mAppGroupId.empty();
}

bool IosSharedCoreHelpers::canCoreStart() {
	ms_message("[SHARED] canCoreStart");
	if (!isCoreShared()) return true;

	if (getCore()->getCCore()->is_main_core) {
		return canMainCoreStart();
	} else {
		return canExecutorCoreStart();
	}
}

bool IosSharedCoreHelpers::isSharedCoreStarted() {
    SharedCoreState state = getSharedCoreState();
	ms_message("[SHARED] get shared core state : %s", sharedStateToString((int)state).c_str());
	if (getCore()->getCCore()->is_main_core && state == SharedCoreState::executorCoreStopped) {
		return false;
	}
	if (state == SharedCoreState::noCoreStarted) {
		return false;
	}
	return true;
}

SharedCoreState IosSharedCoreHelpers::getSharedCoreState() {
	userDefaultMutex.lock();
	NSUserDefaults *defaults = [[NSUserDefaults alloc] initWithSuiteName:@(mAppGroupId.c_str())];
    NSInteger state = [defaults integerForKey:@ACTIVE_SHARED_CORE];
	[defaults release];
	userDefaultMutex.unlock();
	return (SharedCoreState) state;
}

/**
 * Set to false in onLinphoneCoreStop() (called in linphone_core_stop)
 */
void IosSharedCoreHelpers::setSharedCoreState(SharedCoreState sharedCoreState) {
	userDefaultMutex.lock();
	ms_message("[SHARED] setSharedCoreState state: %s", sharedStateToString(sharedCoreState).c_str());
    NSUserDefaults *defaults = [[NSUserDefaults alloc] initWithSuiteName:@(mAppGroupId.c_str())];
    [defaults setInteger:sharedCoreState forKey:@ACTIVE_SHARED_CORE];
	[defaults release];
	userDefaultMutex.unlock();
}

void IosSharedCoreHelpers::resetSharedCoreState() {
	setSharedCoreState(SharedCoreState::noCoreStarted);
}

void IosSharedCoreHelpers::resetSharedCoreLastUpdateTime() {
	userDefaultMutex.lock();
	ms_debug("[SHARED] resetSharedCoreLastUpdateTime");
    NSUserDefaults *defaults = [[NSUserDefaults alloc] initWithSuiteName:@(mAppGroupId.c_str())];
    [defaults setInteger:(NSInteger)ms_get_cur_time_ms() forKey:@LAST_UPDATE_TIME_SHARED_CORE];
	[defaults release];
	userDefaultMutex.unlock();
}

NSInteger IosSharedCoreHelpers::getSharedCoreLastUpdateTime() {
	userDefaultMutex.lock();
    NSUserDefaults *defaults = [[NSUserDefaults alloc] initWithSuiteName:@(mAppGroupId.c_str())];
    NSInteger lastUpdateTime = [defaults integerForKey:@LAST_UPDATE_TIME_SHARED_CORE];
	[defaults release];
	userDefaultMutex.unlock();
	return lastUpdateTime;
}

void IosSharedCoreHelpers::unlockSharedCoreIfNeeded() {
	if ((NSInteger)ms_get_cur_time_ms() - getSharedCoreLastUpdateTime() > 30000) {
		ms_message("[SHARED] unlockSharedCoreIfNeeded : no update during last 30 sec");
		resetSharedCoreState();
		IosSharedCoreHelpers::callIdListMutex.unlock();
		IosSharedCoreHelpers::executorCoreMutex.unlock();
		IosSharedCoreHelpers::userDefaultMutex.unlock();
	}
	resetSharedCoreLastUpdateTime();
}

bool IosSharedCoreHelpers::isCoreStopRequired() {
	return getSharedCoreState() == executorCoreStopping;
}

// we need to reload the config from file at each start tp get the changes made by the other cores
void IosSharedCoreHelpers::reloadConfig() {
	// if we just created the core, we don't need to reload the config
	if (getCore()->getCCore()->has_already_started_once) {
		linphone_config_reload(getCore()->getCCore()->config);
	} else {
		getCore()->getCCore()->has_already_started_once = true;
	}

}

// -----------------------------------------------------------------------------
// shared core : executor
// -----------------------------------------------------------------------------

bool IosSharedCoreHelpers::canExecutorCoreStart() {
	ms_message("[SHARED] canExecutorCoreStart");
	if (isSharedCoreStarted()) {
		ms_message("[SHARED] executor core can't start, another one is started");
		return false;
	}

	subscribeToMainCoreNotifs();
	setSharedCoreState(SharedCoreState::executorCoreStarted);
	resetSharedCoreLastUpdateTime();
	reloadConfig();
	return true;
}


void IosSharedCoreHelpers::subscribeToMainCoreNotifs() {
	ms_message("[SHARED] subscribeToMainCoreNotifs");
   	CFNotificationCenterRef notification = CFNotificationCenterGetDarwinNotifyCenter();
   	CFNotificationCenterAddObserver(notification, (__bridge const void *)(this), on_core_must_stop, CFSTR(ACTIVE_SHARED_CORE), NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
}

void on_core_must_stop(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo) {
   	ms_message("[SHARED] on_core_must_stop");
	if (observer) {
		IosSharedCoreHelpers *myself = (IosSharedCoreHelpers *) observer;
		myself->onCoreMustStop();
	}
}

void IosSharedCoreHelpers::onCoreMustStop() {
	ms_message("[SHARED] onCoreMustStop");
	setSharedCoreState(SharedCoreState::executorCoreStopping);
}

// -----------------------------------------------------------------------------
// shared core : main
// -----------------------------------------------------------------------------

bool IosSharedCoreHelpers::canMainCoreStart() {
	ms_message("[SHARED] canMainCoreStart");
	if (isSharedCoreStarted()) {
		try {
			stopSharedCores();
		} catch (std::exception &e) {
			ms_error("[SHARED] %s", e.what());
			return false;
		}
	}
	setSharedCoreState(SharedCoreState::mainCoreStarted);
	resetSharedCoreLastUpdateTime();
	reloadConfig();
	return true;
}

void IosSharedCoreHelpers::stopSharedCores() {
    ms_message("[SHARED] stopping shared cores");
	CFNotificationCenterRef notification = CFNotificationCenterGetDarwinNotifyCenter();
    CFNotificationCenterPostNotification(notification, CFSTR(ACTIVE_SHARED_CORE), NULL, NULL, YES);

    for(int i=0; isSharedCoreStarted() && i<30; i++) {
        ms_message("[SHARED] wait");
        usleep(100000);
    }
	if (isSharedCoreStarted()) {
		setSharedCoreState(SharedCoreState::noCoreStarted);
	}
    ms_message("[SHARED] shared cores stopped");
}

// -----------------------------------------------------------------------------

std::shared_ptr<SharedCoreHelpers> createIosSharedCoreHelpers (std::shared_ptr<LinphonePrivate::Core> core) {
	return std::dynamic_pointer_cast<SharedCoreHelpers>(make_shared<IosSharedCoreHelpers>(core));
}

LINPHONE_END_NAMESPACE

#endif
