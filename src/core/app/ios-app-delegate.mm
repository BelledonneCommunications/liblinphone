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

@implementation IosAppDelegate

- (id)init {
	self = [super init];
	if (self != nil) {
		[NSNotificationCenter.defaultCenter addObserver:self
		selector:@selector(didEnterBackground:)
			name:UIApplicationDidEnterBackgroundNotification
		  object:nil];

		[NSNotificationCenter.defaultCenter addObserver:self
		selector:@selector(didEnterForeground:)
			name:UIApplicationWillEnterForegroundNotification
		  object:nil];
	}
	return self;
}

- (void)dealloc {
	[super dealloc];
	[NSNotificationCenter.defaultCenter removeObserver:self];
}

// callbacks

- (void)didEnterBackground:(NSNotification *)notif {
	ms_message("[Ios App] didEnterBackground");
	pcore->enterBackground();
}

- (void)didEnterForeground:(NSNotification *)notif {
	ms_message("[Ios App] didEnterForeground");
	pcore->enterForeground();
}

- (void)iterate {
	linphone_core_iterate(pcore->getCCore());
}

- (void)setCore:(std::shared_ptr<LinphonePrivate::Core>)core {
	pcore = core;
}

- (void)onLinphoneCoreStart {
	if (linphone_core_is_auto_iterate_enabled(pcore->getCCore())) {
		if (mIterateTimer.valid) {
			ms_message("[Ios App] core.iterate() is already scheduled");
			return;
		}
		mIterateTimer = [NSTimer timerWithTimeInterval:0.02 target:self selector:@selector(iterate) userInfo:nil repeats:YES];
		// NSTimer runs only in the main thread correctly. Since there may not be a current thread loop.
		[[NSRunLoop mainRunLoop] addTimer:mIterateTimer forMode:NSDefaultRunLoopMode];
		ms_message("[Ios App] Call to core.iterate() scheduled every 20ms");
	} else {
		ms_warning("[Ios App] Auto core.iterate() isn't enabled, ensure you do it in your application!");
	}
}

- (void)onLinphoneCoreStop {
	if (linphone_core_is_auto_iterate_enabled(pcore->getCCore())) {
		if (mIterateTimer) {
			[mIterateTimer invalidate];
			mIterateTimer = nil;
		}
		ms_message("[Ios App] Auto core.iterate() stopped");
	}
}

- (NSString *)bundleFile:(NSString *)file {
	return [[NSBundle mainBundle] pathForResource:[file stringByDeletingPathExtension] ofType:[file pathExtension]];
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
	if (linphone_core_is_push_notification_enabled(pcore->getCCore())) {
		ms_message("[PushKit] Connecting for push notifications");
		voipRegistry = [[PKPushRegistry alloc] initWithQueue:dispatch_get_main_queue()];
		voipRegistry.delegate = self;
		voipRegistry.desiredPushTypes = [NSSet setWithObject:PKPushTypeVoIP];
		
		ms_message("[APNs] register for push notif");
		[[UIApplication sharedApplication] registerForRemoteNotifications];
		
		LinphonePushNotificationConfig *cfg = linphone_core_get_push_notification_config(pcore->getCCore());
		linphone_push_notification_config_set_bundle_identifier(cfg, [[NSBundle mainBundle] bundleIdentifier].UTF8String);
	}
}

- (void)didRegisterForRemotePush:(NSData *)token {
	LinphonePushNotificationConfig *cfg = linphone_core_get_push_notification_config(pcore->getCCore());
	if (token) {
		linphone_push_notification_config_set_remote_token(cfg, [self stringFromToken:token forType:@"remote"].UTF8String);
		linphone_core_update_proxy_config_push_params(pcore->getCCore());
	} else {
		linphone_push_notification_config_set_remote_token(cfg, nullptr);
	}
}

//  PushKit Functions
- (void)pushRegistry:(PKPushRegistry *)registry didUpdatePushCredentials:(PKPushCredentials *)credentials forType:(PKPushType)type {
	ms_message("[PushKit] credentials updated with voip token: %s", [credentials.token.description UTF8String]);
	NSData *pushToken = credentials.token;
	LinphonePushNotificationConfig *cfg = linphone_core_get_push_notification_config(pcore->getCCore());
	linphone_push_notification_config_set_voip_token(cfg, [self stringFromToken:pushToken forType:@"voip"].UTF8String);
	linphone_core_update_proxy_config_push_params(pcore->getCCore());
}

- (void)pushRegistry:(PKPushRegistry *)registry didInvalidatePushTokenForType:(NSString *)type {
    ms_message("[PushKit] Token invalidated");
	LinphonePushNotificationConfig *cfg = linphone_core_get_push_notification_config(pcore->getCCore());
	linphone_push_notification_config_set_voip_token(cfg, nullptr);
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
	if (linphone_core_callkit_enabled(pcore->getCCore())) {
		return true;
	}
#endif
	return false;
}

- (void)processRemoteNotification:(NSDictionary *)userInfo {
	ms_message("[PushKit] Notification [%p] received with payload : %s", userInfo, [userInfo.description UTF8String]);
	LinphoneCore *lc = pcore->getCCore();
	linphone_core_start(lc);
	// support only for calls
	NSDictionary *aps = [userInfo objectForKey:@"aps"];
	NSString *callId = [aps objectForKey:@"call-id"] ?: @"";

	if([self callkitEnabled]) {
		// Since ios13, a new Incoming call must be displayed when the callkit is enabled and app is in background.
		// Otherwise it will cause a crash.
		LinphoneCall *incomingCall = linphone_core_get_call_by_callid(lc, [callId UTF8String]);
		if (!incomingCall) {
			ms_message("[pushkit] create new call");
			incomingCall = linphone_call_new_incoming_with_callid(lc, [callId UTF8String]);
			linphone_call_start_basic_incoming_notification(incomingCall);
			linphone_call_start_push_incoming_notification(incomingCall);
		}
	}

    ms_message("Notification [%p] processed", userInfo);
	// Tell the core to make sure that we are registered.
	// It will initiate socket connections, which seems to be required.
	// Indeed it is observed that if no network action is done in the notification handler, then
	// iOS kills us.
	linphone_core_ensure_registered(lc);
}


@end



