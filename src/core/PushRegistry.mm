/*
 linphone
 Copyright (C) 2017 Belledonne Communications SARL

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "PushRegistry.h"
// TODO: Remove me
#include "private.h"

@implementation RegistryDelegate

- (void)setCore:(std::shared_ptr<LinphonePrivate::Core> )core {
	pcore = core;
}

- (void)setPushKitToken:(NSData *)pushKitToken {
	if (pkToken == pushKitToken) {
		return;
	}
	pkToken = pushKitToken;

    [self configurePushTokenForProxyConfigs];
}

- (void)setRemoteNotificationToken:(NSData *)remoteNotificationToken {
    if (rmToken == remoteNotificationToken) {
        return;
    }
    rmToken = remoteNotificationToken;

    [self configurePushTokenForProxyConfigs];
}

- (void)configurePushTokenForProxyConfigs {
    // we register only when the second token is set ?

	if (pkToken != nil || rmToken != nil) {
		const unsigned char *remoteTokenBuffer = (const unsigned char *)[rmToken bytes];
        NSMutableString *remoteTokenString = [NSMutableString stringWithCapacity:[rmToken length] * 2];
		for (int i = 0; i < int([rmToken length]); ++i) {
			[remoteTokenString appendFormat:@"%02X", (unsigned int)remoteTokenBuffer[i]];
		}

		const unsigned char *PKTokenBuffer = (const unsigned char *)[pkToken bytes];
		NSMutableString *PKTokenString = [NSMutableString stringWithCapacity:[pkToken length] * 2];
		for (int i = 0; i < int([pkToken length]); ++i) {
			[PKTokenString appendFormat:@"%02X", (unsigned int)PKTokenBuffer[i]];
		}

		NSString *token;
		NSString *services;
		if (pkToken && rmToken) {
            token = [NSString stringWithFormat:@"%@:remote&%@:voip", remoteTokenString, PKTokenString];
            services = @"remote&voip";
        } else if (rmToken) {
            token = [NSString stringWithFormat:@"%@:remote", remoteTokenString];
            services = @"remote";
        } else {
            token = [NSString stringWithFormat:@"%@:voip", PKTokenString];
            services = @"voip";
        }
	
		
		// dummy value, for later use
		NSString *teamId = @"ABCD1234";
		NSString *params = [NSString stringWithFormat:@"%@.%@.%@",
						   teamId, [[NSBundle mainBundle] bundleIdentifier], services];
		NSString *rid = token;
	
		linphone_core_update_push_notification_information(pcore->getCCore(), [params UTF8String], [rid UTF8String]);
		
	} else {
		linphone_core_update_push_notification_information(pcore->getCCore(), NULL, NULL);
	}
}


- (void)pushRegistry:(PKPushRegistry *)registry didUpdatePushCredentials:(PKPushCredentials *)credentials forType:(PKPushType)type {
	dispatch_async(dispatch_get_main_queue(), ^{
		[self setPushKitToken:credentials.token];
	});
}

- (void)pushRegistry:(PKPushRegistry *)registry didInvalidatePushTokenForType:(NSString *)type {
	ms_message("[PushKit] Token invalidated");
	dispatch_async(dispatch_get_main_queue(), ^{[self setPushKitToken:NULL];});
}

- (void)processPush:(NSDictionary *)userInfo {
	ms_message("[PushKit] Notification [%p] received with pay load : %s", userInfo, userInfo.description.UTF8String);
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

- (void)processRemoteNotification:(NSDictionary *)userInfo {
	// support only for calls
	NSDictionary *aps = [userInfo objectForKey:@"aps"];
	//NSString *loc_key = [aps objectForKey:@"loc-key"] ?: [[aps objectForKey:@"alert"] objectForKey:@"loc-key"];
	NSString *callId = [aps objectForKey:@"call-id"] ?: @"";

	if(linphone_config_get_int(pcore->getCCore()->config, "app", "use_callkit", 0)) {
		// Since ios13, a new Incoming call must be displayed when the callkit is enabled and app is in background.
		// Otherwise it will cause a crash.
		LinphoneCall *incomingCall = linphone_core_get_call_by_callid(pcore->getCCore(), callId.UTF8String);
		if (!incomingCall) {
			incomingCall = linphone_call_new_incoming_with_callid(pcore->getCCore(), callId.UTF8String);
			linphone_call_start_basic_incoming_notification(incomingCall);
		}
		linphone_call_start_push_incoming_notification(incomingCall);
	} else {
		if (linphone_core_get_calls(pcore->getCCore())) {
			// if there are calls, obviously our TCP socket shall be working
			ms_message("Notification [%p] has no need to be processed because there already is an active call.", userInfo);
			return;
		}

		if ([callId isEqualToString:@""]) {
			// Present apn pusher notifications for info
			ms_message("Notification [%p] came from flexisip-pusher.", userInfo);
			if (floor(NSFoundationVersionNumber) > NSFoundationVersionNumber_iOS_9_x_Max) {
				UNMutableNotificationContent* content = [[UNMutableNotificationContent alloc] init];
				content.title = @"APN Pusher";
				content.body = @"Push notification received !";

				UNNotificationRequest *req = [UNNotificationRequest requestWithIdentifier:@"call_request" content:content trigger:NULL];
				[[UNUserNotificationCenter currentNotificationCenter] addNotificationRequest:req withCompletionHandler:^(NSError * _Nullable error) {
					// Enable or disable features based on authorization.
					if (error) {
						ms_error("Error while adding notification request : %s",error.description.UTF8String);
					}
				}];
			}
		}
	}

    ms_message("Notification [%p] processed", userInfo);
	// Tell the core to make sure that we are registered.
	// It will initiate socket connections, which seems to be required.
	// Indeed it is observed that if no network action is done in the notification handler, then
	// iOS kills us.
	linphone_core_ensure_registered(pcore->getCCore());
}

- (void)application:(UIApplication *)application
	didRegisterForRemoteNotificationsWithDeviceToken:(NSData *)deviceToken {
	ms_message("[APNs] %s : %s", NSStringFromSelector(_cmd).UTF8String, deviceToken.description.UTF8String);
	dispatch_async(dispatch_get_main_queue(), ^{
		[self setRemoteNotificationToken:deviceToken];
	});
}

- (void)application:(UIApplication *)application didFailToRegisterForRemoteNotificationsWithError:(NSError *)error {
	ms_message("[APNs] %s : %s", NSStringFromSelector(_cmd).UTF8String, [error localizedDescription].UTF8String);
	dispatch_async(dispatch_get_main_queue(), ^{
		[self setRemoteNotificationToken:NULL];
	});
}

@end
