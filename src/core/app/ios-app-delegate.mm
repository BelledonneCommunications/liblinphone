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
#import <AVFoundation/AVFoundation.h>

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

- (void)iterate {
	linphone_core_iterate(pcore->getCCore());
}

- (void)configure:(std::shared_ptr<LinphonePrivate::Core>)core{
	pcore = core;
	[self registerForPush];
}

// callbacks

- (void)didEnterBackground:(NSNotification *)notif {
	ms_message("[Ios App] didEnterBackground");
	pcore->enterBackground();
}

- (void)reloadDeviceOnRouteChangeCallback: (NSNotification *) notif
{
	AVAudioSession * audioSession = [AVAudioSession sharedInstance];
		   
	NSDictionary * userInfo = [notif userInfo];
	NSInteger changeReason = [[userInfo valueForKey:AVAudioSessionRouteChangeReasonKey] integerValue];

	AVAudioSessionRouteDescription *previousRoute = [userInfo valueForKey:AVAudioSessionRouteChangePreviousRouteKey];
	AVAudioSessionRouteDescription *currentRoute = [audioSession currentRoute];
	
	std::string previousInputPort("No input");
	std::string currentInputPort("No input");
	std::string previousOutputPort("No output");
	std::string currentOutputPort("No output");

	if (previousRoute.inputs.count > 0)
		previousInputPort = std::string([previousRoute.inputs[0].portName UTF8String]);
	if (previousRoute.outputs.count > 0)
		previousOutputPort = std::string([previousRoute.outputs[0].portName UTF8String]);
	if (currentRoute.inputs.count > 0)
		currentInputPort = std::string([currentRoute.inputs[0].portName UTF8String]);
	if (currentRoute.outputs.count > 0)
		currentOutputPort = std::string([currentRoute.outputs[0].portName UTF8String]);

	ms_message("Previous audio route: input=%s, output=%s, New audio route: input=%s, output=%s"
			  , previousInputPort.c_str(), previousOutputPort.c_str()
			  , currentInputPort.c_str(), currentOutputPort.c_str());

	AVAudioSessionCategory currentCategory = [audioSession category];
	// Relevant audio devices will not be detected by [AVAudioSession availableinputs] if not in PlayAndRecord category
	if (currentCategory != AVAudioSessionCategoryPlayAndRecord)
	   return;

	// Reload only if there was an effective change in the route ports
	if (currentInputPort == previousInputPort && currentOutputPort == previousOutputPort)
	   return;
	
	pcore->doLater([currentOutputPort, currentInputPort, changeReason, self]() {
		
		switch (changeReason)
		{
			case AVAudioSessionRouteChangeReasonNewDeviceAvailable:
			case AVAudioSessionRouteChangeReasonOldDeviceUnavailable:
			case AVAudioSessionRouteChangeReasonCategoryChange:
			{
				// We need to reload for these 3 category, because the first list of AudioDevices available may not be up to date
				// For example, bluetooth devices would possibly not be detected before a call Start, as the AudioSession may be in a category other than AVAudioSessionCategoryPlayAndRecord
				linphone_core_reload_sound_devices(pcore->getCCore());
			}
			default: {}
		}
		
		// Make sure that the current device the core is using match the reality of the IOS audio route. If not, set it properly
		const char * currentDeviceInCore = linphone_audio_device_get_device_name(linphone_core_get_output_audio_device(pcore->getCCore()));
		if (currentDeviceInCore && strcmp(currentOutputPort.c_str(),  currentDeviceInCore) == 0) {
			return;
		}

		bctbx_list_t * deviceIt = linphone_core_get_audio_devices(pcore->getCCore());
		while (deviceIt != NULL) {
			LinphoneAudioDevice * pDevice = (LinphoneAudioDevice *) deviceIt->data;
			if (strcmp(currentInputPort.c_str(), linphone_audio_device_get_device_name(pDevice)) == 0)
			{
				linphone_core_set_output_audio_device(pcore->getCCore(), pDevice);
				break;
			}
			deviceIt = deviceIt->next;
		}
		
		// Notify the filter that the audio route changed
		pcore->soundcardAudioRouteChanged();
	});
}

- (NSString *)bundleFile:(NSString *)file {
	return [[NSBundle mainBundle] pathForResource:[file stringByDeletingPathExtension] ofType:[file pathExtension]];
}

- (void)didEnterForeground:(NSNotification *)notif {
	ms_message("[Ios App] didEnterForeground");
	pcore->enterForeground();
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
