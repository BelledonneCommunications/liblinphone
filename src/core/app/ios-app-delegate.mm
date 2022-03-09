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

#include "ios-app-delegate.h"
#include "call/call.h"
#import <AVFoundation/AVFoundation.h>

@implementation IosObject

- (id)initWithCore:(std::shared_ptr<LinphonePrivate::Core>)core {
	pcore = core;
	return self;
}

-(std::shared_ptr<LinphonePrivate::Core>)getCore {
	std::shared_ptr<LinphonePrivate::Core> core = pcore.lock();
	if (!core) {
		lWarning() << "Unable to get valid core instance.";
		return nullptr;
	}
	return core;
}

@end

@implementation IosAppDelegate

- (id)initWithCore:(std::shared_ptr<LinphonePrivate::Core>)core {
	self = [super initWithCore:core];
	if (self != nil) {
		[NSNotificationCenter.defaultCenter addObserver:self
		selector:@selector(didEnterBackground:)
			name:UIApplicationDidEnterBackgroundNotification
		  object:nil];

		[NSNotificationCenter.defaultCenter addObserver:self
		selector:@selector(didEnterForeground:)
			name:UIApplicationWillEnterForegroundNotification
		  object:nil];
		mStopAsyncEnd = true;
	}

	return self;
}

- (void)dealloc {
	[NSNotificationCenter.defaultCenter removeObserver:self];
	[super dealloc];
}

- (void)didEnterBackground:(NSNotification *)notif {
	ms_message("[Ios App] didEnterBackground");
	if ([self getCore])
		[self getCore]->enterBackground();
}

- (void)didEnterForeground:(NSNotification *)notif {
	ms_message("[Ios App] didEnterForeground");
	if ([self getCore])
		[self getCore]->enterForeground();
}

- (void)iterate {
	if ([self getCore])
		linphone_core_iterate([self getCore]->getCCore());
}

- (void)onStopAsyncEnd: (BOOL)stop {
	mStopAsyncEnd = stop;
}

- (NSMutableString *)stringFromToken:(NSData *)token forType:(NSString *)type {
	NSMutableString *tokenString = nil;
	const unsigned char *tokenBuffer = (const unsigned char *)[token bytes];
	tokenString = [NSMutableString stringWithCapacity:[token length] * 2];
	for (unsigned long i = 0; i < [token length]; ++i) {
		[tokenString appendFormat:@"%02X", (unsigned int)tokenBuffer[i]];
	}
	[tokenString appendFormat:@":%@",type];
	return tokenString;
}

// push notifications

- (void)registerForPush {
	std::shared_ptr<LinphonePrivate::Core> core = [self getCore];
	if (!core) return;

	ms_message("[PushKit] Connecting for push notifications");
	voipRegistry = [[PKPushRegistry alloc] initWithQueue:dispatch_get_main_queue()];
	voipRegistry.delegate = self;
	voipRegistry.desiredPushTypes = [NSSet setWithObject:PKPushTypeVoIP];
		
	ms_message("[APNs] register for push notif");
	[[UIApplication sharedApplication] registerForRemoteNotifications];

	linphone_push_notification_config_set_bundle_identifier(core->getCCore()->push_config, [[NSBundle mainBundle] bundleIdentifier].UTF8String);
	bctbx_list_t* accounts = (bctbx_list_t*)linphone_core_get_account_list(core->getCCore());
	for (; accounts != NULL; accounts = accounts->next) {
		LinphoneAccount *account = (LinphoneAccount *)accounts->data;
		LinphoneAccountParams *newParams = linphone_account_params_clone(linphone_account_get_params(account));
		LinphonePushNotificationConfig *push_cfg = linphone_account_params_get_push_notification_config(newParams);
		linphone_push_notification_config_set_bundle_identifier(push_cfg, [[NSBundle mainBundle] bundleIdentifier].UTF8String);
		linphone_account_set_params(account, newParams);
		linphone_account_params_unref(newParams);
	}
}


- (void)didRegisterForRemotePushWithStringifiedToken:(const char *)tokenStr {
	std::shared_ptr<LinphonePrivate::Core> core = [self getCore];
	if (!core) return;
	
	if (tokenStr) {
		linphone_push_notification_config_set_remote_token(core->getCCore()->push_config, tokenStr);
	} else {
		linphone_push_notification_config_set_remote_token(core->getCCore()->push_config, nullptr);
	}
	bctbx_list_t* accounts = (bctbx_list_t*)linphone_core_get_account_list(core->getCCore());
	for (; accounts != NULL; accounts = accounts->next) {
		LinphoneAccount *account = (LinphoneAccount *)accounts->data;
		
		LinphoneAccountParams *newParams = linphone_account_params_clone(linphone_account_get_params(account));
		LinphonePushNotificationConfig *push_cfg = linphone_account_params_get_push_notification_config(newParams);
		if (tokenStr) {
			linphone_push_notification_config_set_remote_token(push_cfg, tokenStr);
		} else {
			linphone_push_notification_config_set_remote_token(push_cfg, nullptr);
		}
		linphone_account_set_params(account, newParams);
		linphone_account_params_unref(newParams);
		
	}

}
- (void)didRegisterForRemotePush:(NSData *)token {
	if (token) {
		[self didRegisterForRemotePushWithStringifiedToken: [self stringFromToken:token forType:@"remote"].UTF8String];
	} else {
		[self didRegisterForRemotePushWithStringifiedToken: nullptr];
	}
}

//  PushKit Functions
- (void)pushRegistry:(PKPushRegistry *)registry didUpdatePushCredentials:(PKPushCredentials *)credentials forType:(PKPushType)type {
	ms_message("[PushKit] credentials updated with voip token: %s", [credentials.token.description UTF8String]);
	std::shared_ptr<LinphonePrivate::Core> core = [self getCore];
	if (!core) return;
	NSData *pushToken = credentials.token;

	linphone_push_notification_config_set_voip_token(core->getCCore()->push_config, [self stringFromToken:pushToken forType:@"voip"].UTF8String);
	bctbx_list_t* accounts = (bctbx_list_t*)linphone_core_get_account_list(core->getCCore());
	for (; accounts != NULL; accounts = accounts->next) {
		LinphoneAccount *account = (LinphoneAccount *)accounts->data;
		LinphoneAccountParams *newParams = linphone_account_params_clone(linphone_account_get_params(account));
		LinphonePushNotificationConfig *push_cfg = linphone_account_params_get_push_notification_config(newParams);
		linphone_push_notification_config_set_voip_token(push_cfg, [self stringFromToken:pushToken forType:@"voip"].UTF8String);
		linphone_account_set_params(account, newParams);
		linphone_account_params_unref(newParams);
	}
}

- (void)pushRegistry:(PKPushRegistry *)registry didInvalidatePushTokenForType:(NSString *)type {
    ms_message("[PushKit] Token invalidated");
	std::shared_ptr<LinphonePrivate::Core> core = [self getCore];
	if (!core) return;
	linphone_push_notification_config_set_voip_token(core->getCCore()->push_config, nullptr);
	bctbx_list_t* accounts = (bctbx_list_t*)linphone_core_get_account_list(core->getCCore());
	for (; accounts != NULL; accounts = accounts->next) {
		LinphoneAccount *account = (LinphoneAccount *)accounts->data;
		LinphonePushNotificationConfig *push_cfg = linphone_account_params_get_push_notification_config(linphone_account_get_params(account));
		linphone_push_notification_config_set_voip_token(push_cfg, nullptr);
	}
}

- (void)pushRegistry:(PKPushRegistry *)registry didReceiveIncomingPushWithPayload:(PKPushPayload *)payload forType:(PKPushType)type withCompletionHandler:(void (^)(void))completion {
	NSDictionary *userInfo = payload.dictionaryPayload;
	[self processRemoteNotification:userInfo];
	dispatch_async(dispatch_get_main_queue(), ^{completion();});
}

- (void)pushRegistry:(PKPushRegistry *)registry didReceiveIncomingPushWithPayload:(PKPushPayload *)payload forType:(NSString *)type {
	NSDictionary *userInfo = payload.dictionaryPayload;
	[self processRemoteNotification:userInfo];
}

- (BOOL)callkitEnabled {
#if !TARGET_IPHONE_SIMULATOR
	if ([self getCore] && linphone_core_callkit_enabled([self getCore]->getCCore())) {
		return true;
	}
#endif
	return false;
}

- (void)processRemoteNotification:(NSDictionary *)userInfo {
	ms_message("[PushKit] Notification [%p] received with payload : %s", userInfo, [userInfo.description UTF8String]);
	std::shared_ptr<LinphonePrivate::Core> core = [self getCore];
	if (!core) return;
	LinphoneCore *lc = core->getCCore();
	linphone_core_start(lc);
	// support only for calls
	NSDictionary *aps = [userInfo objectForKey:@"aps"];
	NSString *callId = [aps objectForKey:@"call-id"] ?: @"";

	// Since ios13, a new Incoming call must be displayed when the callkit is enabled and app is in background.
	// Otherwise it will cause a crash.
	LinphoneCall *incomingCall = linphone_core_get_call_by_callid(lc, [callId UTF8String]);
	if (!incomingCall) {
		ms_message("[pushkit] create new call");
		incomingCall = linphone_call_new_incoming_with_callid(lc, [callId UTF8String]);
		linphone_call_start_basic_incoming_notification(incomingCall);
		linphone_call_start_push_incoming_notification(incomingCall);
		LinphoneCallLog *calllog = linphone_core_find_call_log(lc,[callId UTF8String], linphone_config_get_int(linphone_core_get_config(lc), "misc", "call_logs_search_limit", 5));
		if (calllog) {
			/* After display a new callkit call, check if the call log with the same callid is created.
			   If yes, that means the call is already aborted. */
			linphone_call_terminate(incomingCall);
			linphone_call_log_unref(calllog);
		}
	}

    ms_message("Notification [%p] processed", userInfo);
	// Tell the core to make sure that we are registered.
	// It will initiate socket connections, which seems to be required.
	// Indeed it is observed that if no network action is done in the notification handler, then
	// iOS kills us.
	linphone_core_process_push_notification(lc, [callId UTF8String]);
}

@end
