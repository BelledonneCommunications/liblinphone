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

- (void)configure:(std::shared_ptr<LinphonePrivate::Core>)core{
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

@end



