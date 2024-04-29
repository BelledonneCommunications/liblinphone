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

#include <UIKit/UIKit.h>
#import <PushKit/PushKit.h>
#include "private.h"
#import <UserNotifications/UserNotifications.h>

@interface IosObject : NSObject {
	std::weak_ptr<LinphonePrivate::Core> pcore;
}

- (id)initWithCore:(std::shared_ptr<LinphonePrivate::Core>)core;
- (std::shared_ptr<LinphonePrivate::Core>)getCore;

@end

/*
 Used only by main core.
 IosAppDelegate is an object taking care of all application delegate's notifications and iteartion:
 UIApplicationDidEnterBackgroundNotification
 UIApplicationWillEnterForegroundNotification
 AVAudioSessionRouteChangeNotification
 iteration
 Its lifecicle is the same as the one from linphonecore init to destroy.
 */
@interface IosAppDelegate : IosObject <PKPushRegistryDelegate> {
	BOOL mStopAsyncEnd;
	dispatch_queue_t pushAndAppDelegateDispatchQueue;
	PKPushRegistry* voipRegistry;
}

- (id)initWithCore:(std::shared_ptr<LinphonePrivate::Core>)core;
- (void)onStopAsyncEnd: (BOOL)stop;
- (void)didRegisterForRemotePush:(NSData *)token;
- (void)didRegisterForRemotePushWithStringifiedToken:(const char *)tokenStr;
- (void)setPushAndAppDelegateDispatchQueue:(void *)dispatchQueue;
- (void)registerForPush;

@end
