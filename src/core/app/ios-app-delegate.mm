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

@implementation IosObject

- (id)initWithCore:(std::shared_ptr<LinphonePrivate::Core>)core {
	pcore = core;
	return self;
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

@end

@implementation IosHandler

- (id)initWithCore:(std::shared_ptr<LinphonePrivate::Core>)core {
	self = [super initWithCore:core];
	if (self != nil) {
		[NSNotificationCenter.defaultCenter addObserver:self
		selector:@selector(reloadDeviceOnRouteChangeCallback:)
			name:AVAudioSessionRouteChangeNotification
		  object:nil];
	}

	return self;
}

- (void)dealloc {
	[super dealloc];
	[NSNotificationCenter.defaultCenter removeObserver:self];
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
	bool currentOutputIsSpeaker = false, currentOutputIsReceiver = false;
	
	if (previousRoute.inputs.count > 0)
		previousInputPort = std::string([previousRoute.inputs[0].portName UTF8String]);
	if (previousRoute.outputs.count > 0)
		previousOutputPort = std::string([previousRoute.outputs[0].portName UTF8String]);
	if (currentRoute.inputs.count > 0)
		currentInputPort = std::string([currentRoute.inputs[0].portName UTF8String]);
	if (currentRoute.outputs.count > 0) {
		currentOutputPort = std::string([currentRoute.outputs[0].portName UTF8String]);
		currentOutputIsReceiver = (strcmp(currentRoute.outputs[0].portType.UTF8String, AVAudioSessionPortBuiltInReceiver.UTF8String) == 0);
		currentOutputIsSpeaker = (strcmp(currentRoute.outputs[0].portType.UTF8String, AVAudioSessionPortBuiltInSpeaker.UTF8String) == 0);
	}

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
	
	pcore->doLater([=]() {
		
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
		
		
		auto deviceMatchCurrentInput = [currentInputPort](std::string const& deviceName) -> bool {
				return (deviceName == currentInputPort);
		};
		auto deviceMatchCurrentOutput = [currentOutputIsSpeaker, currentOutputPort](std::string const& deviceName) -> bool {
			if (currentOutputIsSpeaker)
				return (deviceName == "Speaker");
			else
				return (deviceName == currentOutputPort);
		};
		
		const LinphoneAudioDevice * inputDevice = linphone_core_get_input_audio_device(pcore->getCCore());
		const LinphoneAudioDevice * outputDevice = linphone_core_get_output_audio_device(pcore->getCCore());
		std::string currentInputDeviceInCore( (inputDevice == NULL) ? "" : linphone_audio_device_get_device_name(inputDevice) );
		std::string currentOutputDeviceInCore( (outputDevice == NULL) ? "" : linphone_audio_device_get_device_name(outputDevice) );
		
		// Make sure that the current device the core is using match the reality of the IOS audio route. If not, set it properly
		if (deviceMatchCurrentInput(currentInputDeviceInCore) && deviceMatchCurrentOutput(currentOutputDeviceInCore) )
			return;
		
		bctbx_list_t * deviceIt = linphone_core_get_audio_devices(pcore->getCCore());
		bool inputUpdated = false, outputUpdated = false;
		while ( deviceIt != NULL && !(inputUpdated && outputUpdated) ) {
			LinphoneAudioDevice * pDevice = (LinphoneAudioDevice *) deviceIt->data;
			std::string deviceName(linphone_audio_device_get_device_name(pDevice));
			
			if (deviceMatchCurrentInput(deviceName) && deviceName != currentInputDeviceInCore && !inputUpdated) {
				linphone_core_set_input_audio_device(pcore->getCCore(), pDevice);
				inputUpdated = true;
				
				// Special case : the LinphoneAudioDevice matching the IPhoneMicrophone and the IPhoneReceiver is the same.
				// They are built using the AVAudioSession.availableInputs function, which is why the name will not match
				// We make the assumption that if the output is the IPhone Receiver, then the input is always the IPhoneMicrophone,
				// so also set the output here.
				if (currentOutputIsReceiver) {
					linphone_core_set_output_audio_device(pcore->getCCore(), pDevice);
					outputUpdated = true;
				}
			}
			if (deviceMatchCurrentOutput(deviceName) && deviceName != currentOutputDeviceInCore && !outputUpdated) {
				linphone_core_set_output_audio_device(pcore->getCCore(), pDevice);
				outputUpdated = true;
			}
			
			deviceIt = deviceIt->next;
		}
		
		// Notify the filter that the audio route changed
		pcore->soundcardAudioRouteChanged();
	});
}

@end



