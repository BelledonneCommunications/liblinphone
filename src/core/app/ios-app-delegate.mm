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
		
		[NSNotificationCenter.defaultCenter addObserver:self
		selector:@selector(LinphoneRegisterForRemoteNotifications:)
			name:@"didRegisterForRemoteNotificationsWithDeviceToken"
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

- (void)didRegisterForRemoteNotificationsWithDeviceToken:(NSNotification *)notif {
	if (!notif.userInfo) {
		remoteNotificationToken = nil;
	} else {
		NSDictionary *dict = notif.userInfo;
		NSData *token = dict[@"token"];
		ms_message("[APNs : didRegisterForRemoteNotificationsWithDeviceToken");
		remoteNotificationToken = token;
		[self updatePushNotificationInformation];
	}
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

// push notifications

- (void)registerForVoipPush {
	ms_message("[PushKit] Connecting for push notifications");
	voipRegistry = [[PKPushRegistry alloc] initWithQueue:dispatch_get_main_queue()];
	voipRegistry.delegate = self;
	voipRegistry.desiredPushTypes = [NSSet setWithObject:PKPushTypeVoIP];
}

-(void)registerForRemotePush {
	ms_message("[APNs] register for push notif");
	[[UIApplication sharedApplication] registerForRemoteNotifications];
}

- (void)registerForPush {
	const char *type = linphone_config_get_string(pcore->getCCore()->config, "net", "push_notifications_type", nil);
	if (linphone_core_is_push_notification_enabled(pcore->getCCore()) && type) {
		if (!strcmp(type, "remote&voip")) {
			[self registerForVoipPush];
			[self registerForRemotePush];
		} else if (!strcmp(type, "voip")){
			[self registerForVoipPush];
		} else if (!strcmp(type, "remote")) {
			[self registerForRemotePush];
		}
	}
}

- (void)updatePushNotificationInformation {
	const char *type = linphone_config_get_string(pcore->getCCore()->config, "net", "push_notifications_type", nil);
	if (linphone_core_is_push_notification_enabled(pcore->getCCore()) && type) {
		NSString *services = [NSString stringWithUTF8String:type];
		BOOL voipSupported = false;
		BOOL remoteSupported = false;
		if (!strcmp(type, "remote&voip")) {
			voipSupported = true;
			remoteSupported = true;
		} else if (!strcmp(type, "voip")){
			voipSupported = true;
		} else if (!strcmp(type, "remote")) {
			remoteSupported = true;
		} else {
			return;
		}

		// dummy value, for later use
		NSString *teamId = @"ABCD1234";

		NSString *ring = [self bundleFile: [NSString stringWithUTF8String:linphone_config_get_string(pcore->getCCore()->config, "sound", "local_ring", "notes_of_the_optimistic.caf")]].lastPathComponent;
		NSString *param = [NSString stringWithFormat:@"%@.%@.%@;pn-msg-str=IM_MSG;pn-call-str=IC_MSG;pn-groupchat-str=GC_MSG;pn-"
				 @"call-snd=%@;pn-msg-snd=msg.caf",teamId,[[NSBundle mainBundle] bundleIdentifier],services, ring];

		NSMutableString *PKTokenString = nil;
		NSMutableString *remoteTokenString = nil;
		if (voipSupported && pushKitToken) {
			const unsigned char *PKTokenBuffer = (const unsigned char *)[pushKitToken bytes];
			PKTokenString = [NSMutableString stringWithCapacity:[pushKitToken length] * 2];
			for (unsigned long i = 0; i < [pushKitToken length]; ++i) {
				[PKTokenString appendFormat:@"%02X", (unsigned int)PKTokenBuffer[i]];
			}
		}
		
		if (remoteSupported && remoteNotificationToken) {
			const unsigned char *remoteTokenBuffer = (const unsigned char *)[remoteNotificationToken bytes];
			remoteTokenString = [NSMutableString stringWithCapacity:[remoteNotificationToken length] * 2];
			for (unsigned long i = 0; i < [remoteNotificationToken length]; ++i) {
				[remoteTokenString appendFormat:@"%02X", (unsigned int)remoteTokenBuffer[i]];
			}
		}
		
		NSString *token;
		if (remoteSupported && voipSupported) {
			if (!pushKitToken || !remoteNotificationToken) {
				return;
			}
			token = [NSString stringWithFormat:@"%@:remote&%@:voip", remoteTokenString, PKTokenString];
		} else if (remoteSupported) {
			token = [NSString stringWithFormat:@"%@:remote", remoteTokenString];
		} else {
			token = [NSString stringWithFormat:@"%@:voip", PKTokenString];
		}

		linphone_core_update_push_notification_information(pcore->getCCore(), param.UTF8String, token.UTF8String);
	}
}

//  PushKit Functions

- (void)pushRegistry:(PKPushRegistry *)registry didUpdatePushCredentials:(PKPushCredentials *)credentials forType:(PKPushType)type {
	ms_message("[PushKit] credentials updated with voip token: %s", [credentials.token.description UTF8String]);
	pushKitToken = credentials.token;
	[self updatePushNotificationInformation];
}

- (void)pushRegistry:(PKPushRegistry *)registry didInvalidatePushTokenForType:(NSString *)type {
    ms_message("[PushKit] Token invalidated");
	pushKitToken = nil;
}

- (void)processPush:(NSDictionary *)userInfo {
	ms_message("[PushKit] Notification [%p] received with payload : %s", userInfo, [userInfo.description UTF8String]);
	//to avoid IOS to suspend the app before being able to launch long running task
	[self processRemoteNotification:userInfo];
}

- (void)pushRegistry:(PKPushRegistry *)registry didReceiveIncomingPushWithPayload:(PKPushPayload *)payload forType:(PKPushType)type withCompletionHandler:(void (^)(void))completion {
	[self processPush:payload.dictionaryPayload];
	dispatch_async(dispatch_get_main_queue(), ^{completion();});
}

- (void)pushRegistry:(PKPushRegistry *)registry didReceiveIncomingPushWithPayload:(PKPushPayload *)payload forType:(NSString *)type {
	[self processPush:payload.dictionaryPayload];
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



