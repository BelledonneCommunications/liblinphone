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

- (void)setCore:(std::shared_ptr<LinphonePrivate::Core>)core {
	pcore = core;
}

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

@end



