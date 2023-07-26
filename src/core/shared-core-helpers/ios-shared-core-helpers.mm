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
#include <bctoolbox/defs.h>

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif
#if TARGET_OS_IPHONE

#include <Foundation/Foundation.h>
#include <mutex>
#include <set>

#include "core/core.h"
#include "linphone/core.h"
#include "linphone/api/c-api.h"
#include "linphone/utils/general.h"
#include "linphone/utils/utils.h"
#include "chat/chat-room/chat-room.h"
#include "push-notification-message/push-notification-message.h"
#include "c-wrapper/c-wrapper.h"

#include "logger/logger.h"
#include "shared-core-helpers.h"

// TODO: Remove me
#include "private.h"

#define ACTIVE_SHARED_CORE "ACTIVE_SHARED_CORE"
#define LAST_UPDATE_TIME_SHARED_CORE "LAST_UPDATE_TIME_SHARED_CORE"
#define TEST_GROUP_ID "test group id"
#define TEST_CALL_ID "dummy_call_id"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

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
	IosSharedCoreHelpers (shared_ptr<LinphonePrivate::Core> core);
	~IosSharedCoreHelpers () = default;

	void onLinphoneCoreStop () override;
    void *getPathContext () override;

	void registerSharedCoreMsgCallback() override;
	shared_ptr<PushNotificationMessage> getPushNotificationMessage(const string &callId) override;
	shared_ptr<ChatRoom> getPushNotificationChatRoom(const string &chatRoomAddr) override;

	// shared core
	bool isCoreShared() override;
	bool canCoreStart() override;
	void onCoreMustStop();
	static void uninitSharedCore(LinphoneCore *lc);

	void resetSharedCoreState() override;
	void unlockSharedCoreIfNeeded() override;
	bool isCoreStopRequired() override;

	// push notif
	void clearCallIdList();
	void addNewCallIdToList(string callId);
	void removeCallIdFromList(string callId);
	void setChatRoomInvite(shared_ptr<ChatRoom> chatRoom) override;
	string getChatRoomAddr();
	void reinitTimer();

	void putMsgInUserDefaults(LinphoneChatMessage *msg);
	void putReactionInUserDefaults(LinphoneCore *lc, LinphoneChatMessage *msg, const LinphoneChatMessageReaction *reaction, const LinphoneAddress *address, const char *callIdNoReaction);
	void onMsgWrittenInUserDefaults() override;

	SharedCoreState getSharedCoreState() override;

private:
	// shared core
	void setupSharedCore(struct _LpConfig *config);
	bool isSharedCoreStarted();
	static SharedCoreState getSharedCoreState(const string &appGroupId);
	void setSharedCoreState(SharedCoreState sharedCoreState);
	static void setSharedCoreState(SharedCoreState sharedCoreState, const string &appGroupId);
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
	shared_ptr<PushNotificationMessage> getMsgFromExecutorCore(const string &callId);
	shared_ptr<PushNotificationMessage> getMsgFromMainCore(const string &callId);
	shared_ptr<ChatRoom> getChatRoomFromAddr(const string &chatRoomAddr);
	shared_ptr<PushNotificationMessage> getChatMsgAndUpdateList(const string &callId);
	shared_ptr<PushNotificationMessage> fetchUserDefaultsMsg(const string &callId);
	void cleanUserDefaultsMessages();
	void notifyMessageReceived(const string &callId);
	LinphoneConferenceInfo *getConferenceInfo(LinphoneChatMessage *msg);

	string mAppGroupId = "";
	static set<string> callIdList;
	static mutex callIdListMutex;
	static mutex executorCoreMutex;
	static mutex userDefaultMutex;
	shared_ptr<ChatRoom> mChatRoomInvite = nullptr;
	string mChatRoomAddr;
	uint64_t mTimer = 0;
	CFRunLoopTimerRef mUnlockTimer;
	uint64_t mMaxIterationTimeMs;
	bool mMsgWrittenInUserDefaults = false;
};

set<string> IosSharedCoreHelpers::callIdList = set<string>();
mutex IosSharedCoreHelpers::callIdListMutex;
mutex IosSharedCoreHelpers::executorCoreMutex;
mutex IosSharedCoreHelpers::userDefaultMutex;


// =============================================================================

IosSharedCoreHelpers::IosSharedCoreHelpers (shared_ptr<LinphonePrivate::Core> core) : GenericSharedCoreHelpers(core) {
    struct _LpConfig *config = core->getCCore()->config;
	lInfo() << "[SHARED] setup IosSharedCoreHelpers";
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
		CFRunLoopAddTimer (CFRunLoopGetMain(), mUnlockTimer, kCFRunLoopCommonModes);
		lInfo() << "[SHARED] launch timer";
		unlockSharedCoreIfNeeded();

		cleanUserDefaultsMessages();
	}
}

static void on_push_notification_message_received(LinphoneCore *lc, LinphoneChatRoom *room, LinphoneChatMessage *message) {
	if (linphone_chat_message_has_text_content(message) || linphone_chat_message_get_file_transfer_information(message) != NULL ||
		linphone_chat_message_has_conference_invitation_content(message)) {
		lInfo() << "[push] msg [" << message << "] received from chat room [" << room << "]";

		PlatformHelpers *platform_helper = static_cast<LinphonePrivate::PlatformHelpers*>(lc->platform_helper);
		IosSharedCoreHelpers *shared_core_helper = static_cast<LinphonePrivate::IosSharedCoreHelpers*>(platform_helper->getSharedCoreHelpers().get());

		shared_core_helper->putMsgInUserDefaults(message);
		const char *callId = linphone_chat_message_get_call_id(message);
		static_cast<LinphonePrivate::IosSharedCoreHelpers*>(lc->platform_helper)->removeCallIdFromList(callId);
	} else {
		lInfo() << "[push] don't put msg [" << message << "] in UserDefaults because its content type is " << linphone_chat_message_get_content_type(message) ;
	}
}

static void on_push_notification_messages_received(LinphoneCore *lc, LinphoneChatRoom *room, const bctbx_list_t *messages) {
	for (const bctbx_list_t* msg = messages; msg != NULL; msg = msg->next) {
		on_push_notification_message_received(lc, room, (LinphoneChatMessage *)msg->data);
	}
}

static void on_push_notification_reaction_to_message_received(LinphoneCore *lc, LinphoneChatRoom *room, LinphoneChatMessage *message, const LinphoneChatMessageReaction *reaction) {
	const LinphoneAddress *from = NULL;
	LinphoneAccount *account = NULL;
	account = linphone_core_get_default_account(lc);

	if ((account != NULL)) {
		const LinphoneAccountParams *account_params = linphone_account_get_params(account);
		from = linphone_account_params_get_identity_address(account_params);
	}
	
	if (linphone_chat_message_reaction_get_body(reaction) && from != NULL && !(Address::toCpp(from)->weakEqual(*Address::toCpp(linphone_chat_message_reaction_get_from_address(reaction)))) ){
		lInfo() << "[push] reaction [" << reaction << "] received from msg [" << message << "]";
		PlatformHelpers *platform_helper = static_cast<LinphonePrivate::PlatformHelpers*>(lc->platform_helper);
		IosSharedCoreHelpers *shared_core_helper = static_cast<LinphonePrivate::IosSharedCoreHelpers*>(platform_helper->getSharedCoreHelpers().get());

		shared_core_helper->putReactionInUserDefaults(lc, message, reaction, NULL, NULL);
		const char *callId = linphone_chat_message_reaction_get_call_id(reaction);
		static_cast<LinphonePrivate::IosSharedCoreHelpers*>(lc->platform_helper)->removeCallIdFromList(callId);
	} else {
		lInfo() << "[push] don't put msg [" << message << "] in UserDefaults because the reaction is empty or the user is the sender (no notification for himself)";
	}
}

static void on_push_notification_reaction_to_message_removed(LinphoneCore *lc, LinphoneChatRoom *room, LinphoneChatMessage *message, const LinphoneAddress *address, const char *callId) {
	const LinphoneAddress *from = NULL;
	LinphoneAccount *account = NULL;
	account = linphone_core_get_default_account(lc);

	if ((account != NULL)) {
		const LinphoneAccountParams *account_params = linphone_account_get_params(account);
		from = linphone_account_params_get_identity_address(account_params);
	}
	
	if (from != NULL && !(Address::toCpp(from)->weakEqual(*Address::toCpp(address))) ){
		lInfo() << "[push] callId [" << callId << "] received from msg [" << message << "]";
		PlatformHelpers *platform_helper = static_cast<LinphonePrivate::PlatformHelpers*>(lc->platform_helper);
		IosSharedCoreHelpers *shared_core_helper = static_cast<LinphonePrivate::IosSharedCoreHelpers*>(platform_helper->getSharedCoreHelpers().get());

		shared_core_helper->putReactionInUserDefaults(lc, message, NULL, address, callId);
		static_cast<LinphonePrivate::IosSharedCoreHelpers*>(lc->platform_helper)->removeCallIdFromList(callId);
	} else {
		lInfo() << "[push] don't put msg [" << message << "] in UserDefaults because the user is the sender (no notification for himself)";
	}
}

void IosSharedCoreHelpers::registerSharedCoreMsgCallback() {
	if (isCoreShared()) {
		lInfo() << "[push] register shared core msg callback";
		LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
		linphone_core_cbs_set_message_received(cbs, on_push_notification_message_received);
		linphone_core_cbs_set_messages_received(cbs, on_push_notification_messages_received);
		linphone_core_cbs_set_new_message_reaction(cbs, on_push_notification_reaction_to_message_received);
		linphone_core_cbs_set_reaction_removed_private(cbs, on_push_notification_reaction_to_message_removed);
		linphone_core_add_callbacks(getCore()->getCCore(), cbs);
		linphone_core_cbs_unref(cbs);
	}
}

void IosSharedCoreHelpers::onLinphoneCoreStop() {
	lInfo() << "[SHARED] " << __FUNCTION__;
   	CFNotificationCenterRef notification = CFNotificationCenterGetDarwinNotifyCenter();
	CFNotificationCenterRemoveObserver(notification, (__bridge const void *)(this), CFSTR(ACTIVE_SHARED_CORE), NULL);

	if (isCoreShared()) {
		CFRunLoopTimerInvalidate(mUnlockTimer);
		lInfo() << "[SHARED] stop timer";
	}
}

void IosSharedCoreHelpers::uninitSharedCore(LinphoneCore *lc) {
	lInfo() << "[SHARED] " << __FUNCTION__;
	struct _LpConfig *config = lc->config;
	string appGroupId = linphone_config_get_string(config, "shared_core", "app_group_id", "");

	bool needUnlock = (getSharedCoreState(appGroupId) == SharedCoreState::executorCoreStarted || getSharedCoreState(appGroupId) == SharedCoreState::executorCoreStopping);

	if ((lc->is_main_core && getSharedCoreState(appGroupId) == SharedCoreState::mainCoreStarted) ||
		(!lc->is_main_core && getSharedCoreState(appGroupId) == SharedCoreState::executorCoreStarted)) {
		setSharedCoreState(SharedCoreState::noCoreStarted, appGroupId);
	}

	if (!lc->is_main_core && getSharedCoreState(appGroupId) == SharedCoreState::executorCoreStopping) {
		setSharedCoreState(SharedCoreState::executorCoreStopped, appGroupId);
	}

	if (needUnlock) {
		lInfo() << "[push] unlock executorCoreMutex [" << &IosSharedCoreHelpers::executorCoreMutex << "]";
		IosSharedCoreHelpers::executorCoreMutex.unlock();
	}
}

void *IosSharedCoreHelpers::getPathContext() {
	/* FIXME: not really good idea to use a const char * as an opaque context. An abstract PathContext class 
	 * would be safer and clearer. */
	return mAppGroupId.empty() ? NULL : const_cast<void*>(static_cast<const void*>(mAppGroupId.c_str()));
}

// -----------------------------------------------------------------------------
// push notification: new message
// -----------------------------------------------------------------------------

shared_ptr<PushNotificationMessage> IosSharedCoreHelpers::getPushNotificationMessage(const string &callId) {
	lInfo() << "[push] " << __FUNCTION__;
	lInfo() << "[push] current shared core state: " << sharedStateToString(getSharedCoreState()).c_str();

	switch(getSharedCoreState()) {
		case SharedCoreState::mainCoreStarted:
			lInfo() << "[push] unlock executorCoreMutex [" << &IosSharedCoreHelpers::executorCoreMutex << "]";
			IosSharedCoreHelpers::executorCoreMutex.unlock();
		case SharedCoreState::executorCoreStopping:
		case SharedCoreState::executorCoreStopped:
			clearCallIdList();
			return getMsgFromMainCore(callId);
		case SharedCoreState::executorCoreStarted:
		case SharedCoreState::noCoreStarted:
			addNewCallIdToList(callId);
			IosSharedCoreHelpers::executorCoreMutex.lock();
			lInfo() << "[push] lock executorCoreMutex [" << &IosSharedCoreHelpers::executorCoreMutex << "]";
			if (getSharedCoreState() == executorCoreStopped ||
				getSharedCoreState() == mainCoreStarted ||
				getSharedCoreState() == executorCoreStopping) {
				/* this executor core was waiting on the mutex.
				when unlocked is can't start because a main core is starting */
				return getMsgFromMainCore(callId);
			}
			return getMsgFromExecutorCore(callId);
	}
}

void IosSharedCoreHelpers::cleanUserDefaultsMessages() {
	lInfo() << "[push] " << __FUNCTION__;
	NSUserDefaults *defaults = [[NSUserDefaults alloc] initWithSuiteName:@(mAppGroupId.c_str())];

	NSDictionary *msgDictionary = [defaults dictionaryForKey:@"messages"];
	if (msgDictionary == nil) return;
	NSMutableDictionary *messages = [[NSMutableDictionary alloc] initWithDictionary:msgDictionary];

	NSArray<NSString *> *callIds = [messages allKeys];
	NSString *callId;
	NSDictionary *msg;
	NSNumber *ttl;

	for (callId in callIds) {
		msg = [messages valueForKey:callId];
		ttl = msg[@"ttl"];
		if (ttl && ms_get_cur_time_ms() - ttl.unsignedLongLongValue > 300000) { // 5 minutes
			[messages removeObjectForKey:callId];
			lInfo() << "[push] ttl expired: removed " << callId.UTF8String << " from UserDefaults[messages]. nb of msg: " << [messages count];
		}
	}

	[defaults setObject:messages forKey:@"messages"];
	[defaults release];
	[messages release];
}

LinphoneConferenceInfo *IosSharedCoreHelpers::getConferenceInfo(LinphoneChatMessage *msg) {
	for (const bctbx_list_t *contents = linphone_chat_message_get_contents(msg); contents != nil; contents = contents->next) {
		if (linphone_content_is_icalendar((const LinphoneContent *)contents->data)) {
			return linphone_factory_create_conference_info_from_icalendar_content(linphone_factory_get(), (LinphoneContent *)contents->data);
		}
	}
	return nil;
}

void IosSharedCoreHelpers::putMsgInUserDefaults(LinphoneChatMessage *msg) {
   	lInfo() << "[push] " << __FUNCTION__ << " msg: [" << msg << "]";

	NSUserDefaults *defaults = [[NSUserDefaults alloc] initWithSuiteName:@(mAppGroupId.c_str())];
	NSMutableDictionary *messages;

	NSDictionary *msgDictionary = [defaults dictionaryForKey:@"messages"];
	if (msgDictionary == nil) {
		messages = [[NSMutableDictionary alloc] init];
	} else {
		messages = [[NSMutableDictionary alloc] initWithDictionary:msgDictionary];
	}

	NSString *callId = [NSString stringWithUTF8String:linphone_chat_message_get_call_id(msg)];

	NSNumber *isText = [NSNumber numberWithBool:((BOOL) linphone_chat_message_is_text(msg))];
	const char *cTextContent = linphone_chat_message_get_utf8_text(msg);
	const char *cReactionContent = nil;
	NSString *textContent = [NSString stringWithUTF8String: cTextContent ? cTextContent : ""];
	NSString *reactionContent = [NSString stringWithUTF8String: cReactionContent ? cReactionContent : ""];
	NSString *subject;
	LinphoneChatRoomCapabilitiesMask capabilities = linphone_chat_room_get_capabilities(linphone_chat_message_get_chat_room(msg));
	if (capabilities & LinphoneChatRoomCapabilitiesOneToOne) {
		subject = @"";
	} else {
		const char *cSubject = linphone_chat_room_get_subject(linphone_chat_message_get_chat_room(msg));
		subject = [NSString stringWithUTF8String: cSubject ? cSubject : ""];
	}

	const LinphoneAddress *cFromAddr = linphone_chat_message_get_from_address(msg);
	const LinphoneAddress *cLocalAddr = linphone_chat_message_get_local_address(msg);
	const LinphoneAddress *cPeerAddr = linphone_chat_message_get_peer_address(msg);
	NSString *fromAddr = [NSString stringWithUTF8String:linphone_address_as_string(cFromAddr)];
	NSString *localAddr = [NSString stringWithUTF8String:linphone_address_as_string(cLocalAddr)];
	NSString *peerAddr = [NSString stringWithUTF8String:linphone_address_as_string(cPeerAddr)];
	NSNumber *ttl = [NSNumber numberWithUnsignedLongLong:ms_get_cur_time_ms()];
	NSNumber *isIcalendar = [NSNumber numberWithBool: (BOOL)linphone_chat_message_has_conference_invitation_content(msg)];
	LinphoneConferenceInfo *conferenceInfo = isIcalendar ? getConferenceInfo(msg) : nil;
	NSNumber *isConferenceInvitationNew = isIcalendar && conferenceInfo ? [NSNumber numberWithBool:linphone_conference_info_get_state(conferenceInfo) == LinphoneConferenceInfoStateNew] : [NSNumber numberWithBool:false];
	NSNumber *isConferenceInvitationUpdate = isIcalendar && conferenceInfo ? [NSNumber numberWithBool:linphone_conference_info_get_state(conferenceInfo) == LinphoneConferenceInfoStateUpdated] : [NSNumber numberWithBool:false];
	NSNumber *isConferenceInvitationCancellation = isIcalendar && conferenceInfo ? [NSNumber numberWithBool:linphone_conference_info_get_state(conferenceInfo) == LinphoneConferenceInfoStateCancelled] : [NSNumber numberWithBool:false];
	if (isIcalendar && conferenceInfo &&  linphone_conference_info_get_subject(conferenceInfo)) {
		subject = [NSString stringWithUTF8String:linphone_conference_info_get_subject(conferenceInfo)];
	}

	NSDictionary *newMsg = [NSDictionary dictionaryWithObjects:[NSArray arrayWithObjects:isText, textContent, subject, fromAddr, localAddr, peerAddr, ttl, reactionContent, isIcalendar, isConferenceInvitationNew, isConferenceInvitationUpdate, isConferenceInvitationCancellation, nil]
							forKeys:[NSArray arrayWithObjects:@"isText", @"textContent", @"subject", @"fromAddr", @"localAddr", @"peerAddr", @"ttl", @"reactionContent", @"isIcalendar", @"isConferenceInvitationNew", @"isConferenceInvitationUpdate", @"isConferenceInvitationCancellation", nil]];

	[messages setObject:newMsg forKey:callId];
	[defaults setObject:messages forKey:@"messages"];
	lInfo() << "[push] add " << linphone_chat_message_get_call_id(msg) << " into UserDefaults[messages]. nb of msg:" << [messages count];
	[defaults release];


	if (getSharedCoreState() == mainCoreStarted) notifyMessageReceived(linphone_chat_message_get_call_id(msg));
}

void IosSharedCoreHelpers::putReactionInUserDefaults(LinphoneCore *lc, LinphoneChatMessage *msg, const LinphoneChatMessageReaction *reaction, const LinphoneAddress *address, const char *callIdNoReaction) {
	lInfo() << "[push] " << __FUNCTION__ << " reaction: [" << reaction << "]";

	NSUserDefaults *defaults = [[NSUserDefaults alloc] initWithSuiteName:@(mAppGroupId.c_str())];
	NSMutableDictionary *messages;

	NSDictionary *msgDictionary = [defaults dictionaryForKey:@"messages"];
	if (msgDictionary == nil) {
		messages = [[NSMutableDictionary alloc] init];
	} else {
		messages = [[NSMutableDictionary alloc] initWithDictionary:msgDictionary];
	}

	NSString *callId = [NSString stringWithUTF8String: reaction != nil ? linphone_chat_message_reaction_get_call_id(reaction) : callIdNoReaction];

	NSNumber *isText = [NSNumber numberWithBool:((BOOL) linphone_chat_message_is_text(msg))];
	const char *cTextContent = linphone_chat_message_get_utf8_text(msg);
	const char *cReactionContent = reaction != nil ? linphone_chat_message_reaction_get_body(reaction) : nil;
	NSString *textContent = [NSString stringWithUTF8String: cTextContent ? cTextContent : ""];
	NSString *reactionContent = [NSString stringWithUTF8String: cReactionContent ? cReactionContent : " "];
	NSString *subject;
	LinphoneChatRoomCapabilitiesMask capabilities = linphone_chat_room_get_capabilities(linphone_chat_message_get_chat_room(msg));
	if (capabilities & LinphoneChatRoomCapabilitiesOneToOne) {
		subject = @"";
	} else {
		const char *cSubject = linphone_chat_room_get_subject(linphone_chat_message_get_chat_room(msg));
		subject = [NSString stringWithUTF8String: cSubject ? cSubject : ""];
	}
	const LinphoneAddress *cFromAddr = reaction != nil ? linphone_chat_message_reaction_get_from_address(reaction) : address;
	const LinphoneAddress *cLocalAddr = linphone_chat_message_get_local_address(msg);
	const LinphoneAddress *cPeerAddr = linphone_chat_message_get_peer_address(msg);
	NSString *fromAddr = [NSString stringWithUTF8String:linphone_address_as_string(cFromAddr)];
	NSString *localAddr = [NSString stringWithUTF8String:linphone_address_as_string(cLocalAddr)];
	NSString *peerAddr = [NSString stringWithUTF8String:linphone_address_as_string(cPeerAddr)];
	NSNumber *ttl = [NSNumber numberWithUnsignedLongLong:ms_get_cur_time_ms()];
	NSNumber *isIcalendar = [NSNumber numberWithBool: (BOOL)linphone_chat_message_has_conference_invitation_content(msg)];
	LinphoneConferenceInfo *conferenceInfo = isIcalendar ? getConferenceInfo(msg) : nil;
	NSNumber *isConferenceInvitationNew = isIcalendar && conferenceInfo ? [NSNumber numberWithBool:linphone_conference_info_get_state(conferenceInfo) == LinphoneConferenceInfoStateNew] : [NSNumber numberWithBool:false];
	NSNumber *isConferenceInvitationUpdate = isIcalendar && conferenceInfo ? [NSNumber numberWithBool:linphone_conference_info_get_state(conferenceInfo) == LinphoneConferenceInfoStateUpdated] : [NSNumber numberWithBool:false];
	NSNumber *isConferenceInvitationCancellation = isIcalendar && conferenceInfo ? [NSNumber numberWithBool:linphone_conference_info_get_state(conferenceInfo) == LinphoneConferenceInfoStateCancelled] : [NSNumber numberWithBool:false];
	if (isIcalendar && conferenceInfo &&  linphone_conference_info_get_subject(conferenceInfo)) {
		subject = [NSString stringWithUTF8String:linphone_conference_info_get_subject(conferenceInfo)];
	}

	NSDictionary *newMsg = [NSDictionary dictionaryWithObjects:[NSArray arrayWithObjects:isText, textContent, subject, fromAddr, localAddr, peerAddr, ttl, reactionContent, isIcalendar, isConferenceInvitationNew, isConferenceInvitationUpdate, isConferenceInvitationCancellation, nil]
							forKeys:[NSArray arrayWithObjects:@"isText", @"textContent", @"subject", @"fromAddr", @"localAddr", @"peerAddr", @"ttl", @"reactionContent", @"isIcalendar", @"isConferenceInvitationNew", @"isConferenceInvitationUpdate", @"isConferenceInvitationCancellation", nil]];

	[messages setObject:newMsg forKey:callId];
	[defaults setObject:messages forKey:@"messages"];
	if (reaction != nil) {
		lInfo() << "[push] add " << linphone_chat_message_reaction_get_call_id(reaction) << " into UserDefaults[messages]. nb of msg:" << [messages count];
	} else {
		lInfo() << "[push] add " << callIdNoReaction << " into UserDefaults[messages]. nb of msg:" << [messages count];
	}
	
	[defaults release];

	if (getSharedCoreState() == mainCoreStarted) notifyMessageReceived(reaction != nil ? linphone_chat_message_reaction_get_call_id(reaction) : callIdNoReaction);
}

shared_ptr<PushNotificationMessage> IosSharedCoreHelpers::fetchUserDefaultsMsg(const string &callId) {
	bool isText;
	string textContent;
	string subject;
	string fromAddr;
	string localAddr;
	string peerAddr;
	string reactionContent;
	bool isIcalendar;
	bool isConferenceInvitationNew;
	bool isConferenceInvitationUpdate;
	bool isConferenceInvitationCancellation;


	NSUserDefaults *defaults = [[NSUserDefaults alloc] initWithSuiteName:@(mAppGroupId.c_str())];
	NSString *objcCallId = [NSString stringWithUTF8String:callId.c_str()];
	NSMutableDictionary *messages = [[NSMutableDictionary alloc] initWithDictionary:[defaults dictionaryForKey:@"messages"]];
	lInfo() << "[push] " << __FUNCTION__ << " userDefaults messages: [" << messages << "]";

	NSDictionary *msgData = NULL;
	msgData = [messages objectForKey:objcCallId];

	if (msgData == nil) {
		lInfo() << "[push] message data not found for callId : [" << callId.c_str() << "]";
		return nullptr;
	} else {
		lInfo() << "[push] found message data for callId : [" << callId.c_str() << "]";
	}

	isText = [msgData[@"isText"] boolValue];
	textContent = [[NSString stringWithString:msgData[@"textContent"]] UTF8String];
	subject = [[NSString stringWithString:msgData[@"subject"]] UTF8String];
	fromAddr = [[NSString stringWithString:msgData[@"fromAddr"]] UTF8String];
	localAddr = [[NSString stringWithString:msgData[@"localAddr"]] UTF8String];
	peerAddr = [[NSString stringWithString:msgData[@"peerAddr"]] UTF8String];
	reactionContent = [[NSString stringWithString:msgData[@"reactionContent"]] UTF8String];
	isIcalendar = [msgData[@"isIcalendar"] boolValue];
	isConferenceInvitationNew = [msgData[@"isConferenceInvitationNew"] boolValue];
	isConferenceInvitationUpdate = [msgData[@"isConferenceInvitationUpdate"] boolValue];
	isConferenceInvitationCancellation = [msgData[@"isConferenceInvitationCancellation"] boolValue];

	[messages removeObjectForKey:objcCallId];
	lInfo() << "[push] push received: removed " << callId.c_str() << " from UserDefaults[messages]. nb of msg: " << [messages count];
	[defaults setObject:messages forKey:@"messages"];
	[defaults release];

	shared_ptr<PushNotificationMessage> msg = PushNotificationMessage::create(callId, isText, textContent, subject, fromAddr, localAddr, peerAddr, reactionContent, isIcalendar, isConferenceInvitationNew, isConferenceInvitationUpdate, isConferenceInvitationCancellation);
	return msg;
}

// -----------------------------------------------------------------------------
// push notification: get new message already received by the main core
// -----------------------------------------------------------------------------

void on_msg_written_in_user_defaults(BCTBX_UNUSED(CFNotificationCenterRef center), void *observer, BCTBX_UNUSED(CFStringRef name), BCTBX_UNUSED(const void *object), BCTBX_UNUSED(CFDictionaryRef userInfo)) {
	lInfo() << "[push] " << __FUNCTION__;
	if (observer) {
		IosSharedCoreHelpers *myself = (IosSharedCoreHelpers *) observer;
		myself->onMsgWrittenInUserDefaults();
	}
}


// The message has been received by the main core and we will just fetch it into the user defaults
shared_ptr<PushNotificationMessage> IosSharedCoreHelpers::getMsgFromMainCore(const string &callId) {
	
	shared_ptr<PushNotificationMessage> msg = fetchUserDefaultsMsg(callId);
	if (msg) {
		lInfo() << "[push] message was already found in user defaults, no need to subscribe to notifications";
		return msg;
	}
	
	lInfo() << "[push] subscribe to main core notif: receive a notif when msg is written in user defaults";
	
	CFNotificationCenterRef notification = CFNotificationCenterGetDarwinNotifyCenter();
	CFStringRef notificationName;

	notificationName = CFStringCreateWithCString(NULL, callId.c_str(), kCFStringEncodingUTF8);

	CFNotificationCenterAddObserver(notification, (__bridge const void *)(this), on_msg_written_in_user_defaults, notificationName, NULL, CFNotificationSuspensionBehaviorDeliverImmediately);

	reinitTimer();
	while (ms_get_cur_time_ms() - mTimer < mMaxIterationTimeMs && !mMsgWrittenInUserDefaults) {
		lInfo() << "[push] wait for msg to be written in user defaults";
		ms_usleep(50000);
	}

	CFNotificationCenterRemoveObserver(notification, (__bridge const void *)(this), notificationName, NULL);

	msg = fetchUserDefaultsMsg(callId);
	return msg;
}

void IosSharedCoreHelpers::onMsgWrittenInUserDefaults() {
   	lInfo() << "[SHARED] " << __FUNCTION__;
	mMsgWrittenInUserDefaults = true;
}

void IosSharedCoreHelpers::notifyMessageReceived(const string &callId) {
	string notifName = callId;

	// post notif
	CFNotificationCenterRef notification = CFNotificationCenterGetDarwinNotifyCenter();
	CFNotificationCenterPostNotification(notification, CFStringCreateWithCString(NULL, notifName.c_str(), kCFStringEncodingUTF8), NULL, NULL, YES);
	lInfo() << "[push] Darwin notif id : [" << notifName << "] sent";
}

// -----------------------------------------------------------------------------
// push notification: get new message from by starting an executor core
// -----------------------------------------------------------------------------

// We have to start a linphone core to get the message
shared_ptr<PushNotificationMessage> IosSharedCoreHelpers::getMsgFromExecutorCore(const string &callId) {
	shared_ptr<PushNotificationMessage> chatMessage;
	lInfo() << "[push] " << __FUNCTION__ << " for callid [" << callId << "]";

	chatMessage = getChatMsgAndUpdateList(callId);
	lInfo() << "[push] message found? " << (chatMessage ? "yes" : "no");
	if (chatMessage) {
		/* if we are here, the mutex is locked but won't be unlocked during
		linphone_core_stop because this executor core doesn't need to be started */
		lInfo() << "[push] unlock executorCoreMutex [" << &IosSharedCoreHelpers::executorCoreMutex << "]";
		IosSharedCoreHelpers::executorCoreMutex.unlock();
		return chatMessage;
	}

	if (linphone_core_get_global_state(getCore()->getCCore()) != LinphoneGlobalOn && linphone_core_start(getCore()->getCCore()) != 0) {
		return nullptr;
	}
	lInfo() << "[push] core started";

	/* init with 0 to look for the msg when newMsgNb <= 0 and then try to get the msg every seconds */
	uint64_t secondsTimer = 0;
	reinitTimer();

	while (ms_get_cur_time_ms() - mTimer < mMaxIterationTimeMs && (!IosSharedCoreHelpers::callIdList.empty() || !chatMessage)) {
		lInfo() << "[push] wait for msg. callIdList size = "  << IosSharedCoreHelpers::callIdList.size();
		linphone_core_iterate(getCore()->getCCore());

		if (getSharedCoreState() == SharedCoreState::executorCoreStopping) {
			lInfo() << "[SHARED] executor core stopping";
			chatMessage = getChatMsgAndUpdateList(callId);
			lInfo() << "[push] last chance to get msg in userDefaults. message found? " << (chatMessage ? "yes" : "no");
			return chatMessage;
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

	if (!chatMessage) chatMessage = getChatMsgAndUpdateList(callId);
	lInfo() << "[push] message received? " << (chatMessage ? "yes" : "no");

	return chatMessage;
}

shared_ptr<PushNotificationMessage> IosSharedCoreHelpers::getChatMsgAndUpdateList(const string &callId) {
	shared_ptr<PushNotificationMessage> msg = fetchUserDefaultsMsg(callId);
	if (msg) removeCallIdFromList(callId);
	return msg;
}

void IosSharedCoreHelpers::clearCallIdList() {
	IosSharedCoreHelpers::callIdListMutex.lock();
	IosSharedCoreHelpers::callIdList.clear();
	IosSharedCoreHelpers::callIdListMutex.unlock();
	lInfo() << "[push] clear callIdList";
}

void IosSharedCoreHelpers::addNewCallIdToList(string callId) {
	IosSharedCoreHelpers::callIdListMutex.lock();
	IosSharedCoreHelpers::callIdList.insert(callId);
	IosSharedCoreHelpers::callIdListMutex.unlock();
	lInfo() << "[push] add " << callId.c_str() << " to callIdList if not already present";
}

void IosSharedCoreHelpers::removeCallIdFromList(string callId) {
	IosSharedCoreHelpers::callIdListMutex.lock();
	if (IosSharedCoreHelpers::callIdList.erase(callId)) {
		lInfo() << "[push] removed " << callId.c_str() << " from callIdList";
	} else {
		lInfo() << "[push] unable to remove " << callId.c_str() << " from callIdList: not found";
	}
	IosSharedCoreHelpers::callIdListMutex.unlock();
}

// -----------------------------------------------------------------------------
// push notification: new chat room
// -----------------------------------------------------------------------------

shared_ptr<ChatRoom> IosSharedCoreHelpers::getPushNotificationChatRoom(const string &chatRoomAddr) {
	lInfo() << "[push] " << __FUNCTION__;
	lInfo() << "[push] current shared core state: " << sharedStateToString(getSharedCoreState()).c_str();

	switch(getSharedCoreState()) {
		case SharedCoreState::mainCoreStarted:
			lInfo() << "[push] unlock executorCoreMutex [" << &IosSharedCoreHelpers::executorCoreMutex << "]";
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
				/* this executor core was waiting on the mutex.
				when unlocked is can't start because a main core is starting */
				return nullptr;
			}
			shared_ptr<ChatRoom> chatRoom = getChatRoomFromAddr(chatRoomAddr);
			return chatRoom;
	}
}

void IosSharedCoreHelpers::setChatRoomInvite(shared_ptr<ChatRoom> chatRoom) {
	mChatRoomInvite = chatRoom;
}

string IosSharedCoreHelpers::getChatRoomAddr() {
	return mChatRoomAddr;
}

void IosSharedCoreHelpers::reinitTimer() {
	mTimer = ms_get_cur_time_ms();
}

static void on_push_notification_chat_room_joined(LinphoneChatRoom *cr, BCTBX_UNUSED(const LinphoneEventLog *evlog)){
	LinphoneCore *lc = linphone_chat_room_get_core(cr);
	PlatformHelpers *platform_helper = static_cast<LinphonePrivate::PlatformHelpers*>(lc->platform_helper);
	IosSharedCoreHelpers *shared_core_helper = static_cast<LinphonePrivate::IosSharedCoreHelpers*>(platform_helper->getSharedCoreHelpers().get());
	lInfo() << "[push] the chat room associated with the push is joined.";
	shared_core_helper->setChatRoomInvite(static_pointer_cast<ChatRoom>(AbstractChatRoom::toCpp(cr)->getSharedFromThis()));
}

static void on_push_notification_chat_room_invite_received(LinphoneCore *lc, LinphoneChatRoom *cr, LinphoneChatRoomState state) {
	if (state == LinphoneChatRoomStateCreated) {
		PlatformHelpers *platform_helper = static_cast<LinphonePrivate::PlatformHelpers*>(lc->platform_helper);
		IosSharedCoreHelpers *shared_core_helper = static_cast<LinphonePrivate::IosSharedCoreHelpers*>(platform_helper->getSharedCoreHelpers().get());
		shared_core_helper->reinitTimer();

		const char *cr_peer_addr = linphone_address_get_username(linphone_chat_room_get_peer_address(cr));
		lInfo() << "[push] we are added to the chat room " << cr_peer_addr;

		if (cr_peer_addr && strcmp(cr_peer_addr, shared_core_helper->getChatRoomAddr().c_str()) == 0) {
			lInfo() << "[push] the chat room associated with the push is found, now waiting for being joined.";
			LinphoneChatRoomCbs *cbs = linphone_factory_create_chat_room_cbs(linphone_factory_get());
			linphone_chat_room_cbs_set_conference_joined(cbs, on_push_notification_chat_room_joined);
			linphone_chat_room_add_callbacks(cr, cbs);
			linphone_chat_room_cbs_unref(cbs);
		}
	}
}

shared_ptr<ChatRoom> IosSharedCoreHelpers::getChatRoomFromAddr(const string &crAddr) {
	lInfo() << "[push] " << __FUNCTION__ << ". looking for chatroom " << mChatRoomAddr;
	mChatRoomAddr = crAddr;
	mChatRoomInvite = nullptr;


	if (linphone_core_get_global_state(getCore()->getCCore()) != LinphoneGlobalOn && linphone_core_start(getCore()->getCCore()) != 0) {
		return nullptr;
	}
	lInfo() << "[push] core started";
	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_chat_room_state_changed(cbs, on_push_notification_chat_room_invite_received);
	linphone_core_add_callbacks(getCore()->getCCore(), cbs);
	linphone_core_cbs_unref(cbs);

	reinitTimer();
	uint64_t iterationTimer = ms_get_cur_time_ms();

	/* if the chatroom is received, iterate for 2 sec otherwise iterate mMaxIterationTimeMs seconds*/
	while ((ms_get_cur_time_ms() - iterationTimer < mMaxIterationTimeMs) && !mChatRoomInvite) {
		lInfo() << "[push] wait chatRoom";
		linphone_core_iterate(getCore()->getCCore());

		if (getSharedCoreState() == SharedCoreState::executorCoreStopping) {
			lInfo() << "[SHARED] executor core stopping";
			return nullptr;
		}

		ms_usleep(20000);
	}

	return mChatRoomInvite;
}

// -----------------------------------------------------------------------------
// shared core
// -----------------------------------------------------------------------------

void unlock_shared_core_if_needed(BCTBX_UNUSED(CFRunLoopTimerRef timer), void *info) {
	IosSharedCoreHelpers *platform_helper = (IosSharedCoreHelpers *) info;
	platform_helper->unlockSharedCoreIfNeeded();
}

bool IosSharedCoreHelpers::isCoreShared() {
	return !mAppGroupId.empty();
}

bool IosSharedCoreHelpers::canCoreStart() {
	lInfo() << "[SHARED] " << __FUNCTION__;
	if (!isCoreShared()) return true;

	if (getCore()->getCCore()->is_main_core) {
		return canMainCoreStart();
	} else {
		return canExecutorCoreStart();
	}
}

bool IosSharedCoreHelpers::isSharedCoreStarted() {
    SharedCoreState state = getSharedCoreState();
	lInfo() << "[SHARED] get shared core state: " << sharedStateToString((int)state);
	if (getCore()->getCCore()->is_main_core && state == SharedCoreState::executorCoreStopped) {
		return false;
	}
	if (state == SharedCoreState::noCoreStarted) {
		return false;
	}
	return true;
}

SharedCoreState IosSharedCoreHelpers::getSharedCoreState() {
	return IosSharedCoreHelpers::getSharedCoreState(mAppGroupId);
}

SharedCoreState IosSharedCoreHelpers::getSharedCoreState(const string &appGroupId) {
	userDefaultMutex.lock();
	NSUserDefaults *defaults = [[NSUserDefaults alloc] initWithSuiteName:@(appGroupId.c_str())];
    NSInteger state = [defaults integerForKey:@ACTIVE_SHARED_CORE];
	[defaults release];
	userDefaultMutex.unlock();
	return (SharedCoreState) state;
}

void IosSharedCoreHelpers::setSharedCoreState(SharedCoreState sharedCoreState) {
	IosSharedCoreHelpers::setSharedCoreState(sharedCoreState, mAppGroupId);
}

void IosSharedCoreHelpers::setSharedCoreState(SharedCoreState sharedCoreState, const string &appGroupId) {
	lInfo() << "[SHARED] setSharedCoreState state: " << sharedStateToString(sharedCoreState);
	if (appGroupId.empty()) {
		lInfo() << "[SHARED] unable to setSharedCoreState state: appGroupId empty";
		return;
	}

	userDefaultMutex.lock();
    NSUserDefaults *defaults = [[NSUserDefaults alloc] initWithSuiteName:@(appGroupId.c_str())];
    [defaults setInteger:sharedCoreState forKey:@ACTIVE_SHARED_CORE];
	[defaults release];
	userDefaultMutex.unlock();
}

void IosSharedCoreHelpers::resetSharedCoreState() {
	setSharedCoreState(SharedCoreState::noCoreStarted);
}

void IosSharedCoreHelpers::resetSharedCoreLastUpdateTime() {
	userDefaultMutex.lock();
	lInfo() <<"[SHARED] " << __FUNCTION__;
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
		lInfo() << "[SHARED] " << __FUNCTION__ << " : no update during last 30 sec";
		resetSharedCoreState();
		IosSharedCoreHelpers::callIdListMutex.unlock();
		lInfo() << "[push] unlock executorCoreMutex [" << &IosSharedCoreHelpers::executorCoreMutex << "]";
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
	lInfo() << "[SHARED] " << __FUNCTION__;
	if (isSharedCoreStarted()) {
		lInfo() << "[SHARED] executor core can't start, another one is started";
		return false;
	}

	subscribeToMainCoreNotifs();
	setSharedCoreState(SharedCoreState::executorCoreStarted);
	resetSharedCoreLastUpdateTime();
	reloadConfig();
	return true;
}

void IosSharedCoreHelpers::subscribeToMainCoreNotifs() {
	lInfo() << "[SHARED] " << __FUNCTION__;
   	CFNotificationCenterRef notification = CFNotificationCenterGetDarwinNotifyCenter();
   	CFNotificationCenterAddObserver(notification, (__bridge const void *)(this), on_core_must_stop, CFSTR(ACTIVE_SHARED_CORE), NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
}

void on_core_must_stop(BCTBX_UNUSED(CFNotificationCenterRef center), void *observer, BCTBX_UNUSED(CFStringRef name), BCTBX_UNUSED(const void *object), BCTBX_UNUSED(CFDictionaryRef userInfo)) {
   	lInfo() << "[SHARED] " << __FUNCTION__;
	if (observer) {
		IosSharedCoreHelpers *myself = (IosSharedCoreHelpers *) observer;
		myself->onCoreMustStop();
	}
}

void IosSharedCoreHelpers::onCoreMustStop() {
	lInfo() << "[SHARED] " << __FUNCTION__;
	if (getSharedCoreState() != SharedCoreState::executorCoreStopping) {
		setSharedCoreState(SharedCoreState::executorCoreStopping);
	}
}

// -----------------------------------------------------------------------------
// shared core : main
// -----------------------------------------------------------------------------

bool IosSharedCoreHelpers::canMainCoreStart() {
	lInfo() << "[SHARED] " << __FUNCTION__;
	if (isSharedCoreStarted()) {
		try {
			stopSharedCores();
		} catch (exception &e) {
			lError() << "[SHARED] " << e.what();
			return false;
		}
	}
	setSharedCoreState(SharedCoreState::mainCoreStarted);
	resetSharedCoreLastUpdateTime();
	reloadConfig();
	return true;
}

void IosSharedCoreHelpers::stopSharedCores() {
    lInfo() << "[SHARED] stopping shared cores";
	CFNotificationCenterRef notification = CFNotificationCenterGetDarwinNotifyCenter();
    CFNotificationCenterPostNotification(notification, CFSTR(ACTIVE_SHARED_CORE), NULL, NULL, YES);

    for(int i=0; isSharedCoreStarted() && i<30; i++) {
        lInfo() << "[SHARED] wait";
        usleep(100000);
    }
	if (isSharedCoreStarted()) {
		setSharedCoreState(SharedCoreState::noCoreStarted);
	}
    lInfo() << "[SHARED] shared cores stopped";
}

// -----------------------------------------------------------------------------

shared_ptr<SharedCoreHelpers> createIosSharedCoreHelpers (shared_ptr<LinphonePrivate::Core> core) {
	return dynamic_pointer_cast<SharedCoreHelpers>(make_shared<IosSharedCoreHelpers>(core));
}

void uninitSharedCore(LinphoneCore *lc) {
	if (getPlatformHelpers(lc)->getSharedCoreHelpers()->isCoreShared()) {
		getPlatformHelpers(lc)->getSharedCoreHelpers()->setChatRoomInvite(nullptr);
		IosSharedCoreHelpers::uninitSharedCore(lc);
	}
}

LINPHONE_END_NAMESPACE

#endif
