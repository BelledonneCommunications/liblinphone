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
		[NSNotificationCenter.defaultCenter addObserver:self
		selector:@selector(reloadDeviceOnRouteChangeCallback:)
			name:AVAudioSessionRouteChangeNotification
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

- (void)reloadDeviceOnRouteChangeCallback: (NSNotification *) notif
{
	if (mStopAsyncEnd) {
		ms_warning("[reloadDeviceOnRouteChangeCallback] linphone_stop_async_end is already called, skipping");
		return;
	}
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
	
	std::shared_ptr<LinphonePrivate::Core> core = [self getCore];
	if (!core) return;
	core->doLater([=]() {
		
		switch (changeReason)
		{
			case AVAudioSessionRouteChangeReasonNewDeviceAvailable:
			case AVAudioSessionRouteChangeReasonOldDeviceUnavailable:
			case AVAudioSessionRouteChangeReasonCategoryChange:
			{
				// We need to reload for these 3 category, because the first list of AudioDevices available may not be up to date
				// For example, bluetooth devices would possibly not be detected before a call Start, as the AudioSession may be in a category other than AVAudioSessionCategoryPlayAndRecord
				linphone_core_reload_sound_devices([self getCore]->getCCore());
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
		
		const LinphoneAudioDevice * inputDevice = linphone_core_get_input_audio_device(core->getCCore());
		const LinphoneAudioDevice * outputDevice = linphone_core_get_output_audio_device(core->getCCore());
		std::string currentInputDeviceInCore( (inputDevice == NULL) ? "" : linphone_audio_device_get_device_name(inputDevice) );
		std::string currentOutputDeviceInCore( (outputDevice == NULL) ? "" : linphone_audio_device_get_device_name(outputDevice) );
		
		// Make sure that the current device the core is using match the reality of the IOS audio route. If not, set it properly
		bool inputRequiresUpdate = !deviceMatchCurrentInput(currentInputDeviceInCore);
		bool outputRequiresUpdate = !deviceMatchCurrentOutput(currentOutputDeviceInCore);
		
		bctbx_list_t * deviceIt = linphone_core_get_extended_audio_devices(core->getCCore());
		LinphoneAudioDevice * pLastDeviceSet = NULL;
		while ( deviceIt != NULL && (inputRequiresUpdate || outputRequiresUpdate) ) {
			LinphoneAudioDevice * pDevice = (LinphoneAudioDevice *) deviceIt->data;
			std::string deviceName(linphone_audio_device_get_device_name(pDevice));
			
			bool inputMatch = deviceMatchCurrentInput(deviceName);
			if (inputRequiresUpdate && inputMatch) {
				linphone_core_set_input_audio_device(core->getCCore(), pDevice);
				pLastDeviceSet = pDevice;
				inputRequiresUpdate = false;
			}
			if (outputRequiresUpdate) {
				// Special case : the LinphoneAudioDevice matching the IPhoneMicrophone and the IPhoneReceiver is the same.
				// They are built using the AVAudioSession.availableInputs function, which is why the name will not match
				// We make the assumption that if the output is the IPhone Receiver, then the input is always the IPhoneMicrophone,
				// so also set the output here.
				bool isReceiverSpecialCase = currentOutputIsReceiver && inputMatch;
				
				if (isReceiverSpecialCase || deviceMatchCurrentOutput(deviceName)) {
					linphone_core_set_output_audio_device(core->getCCore(), pDevice);
					pLastDeviceSet = pDevice;
					outputRequiresUpdate = false;
				}
			}
			deviceIt = deviceIt->next;
		}
		bctbx_list_free_with_data(deviceIt, (void (*)(void *))linphone_audio_device_unref);
		
		if (inputRequiresUpdate) {
			ms_warning("Current audio route input is '%s', but we could not find the matching device in the linphone devices list", currentInputPort.c_str());
			if (!outputRequiresUpdate && pLastDeviceSet != NULL) {
				ms_warning("Setting input device to match the current output device by default");
				linphone_core_set_input_audio_device(core->getCCore(), pLastDeviceSet);
			}
		}
		if (outputRequiresUpdate) {
			ms_warning("Current audio route output is '%s', but we could not find the matching device in the linphone devices list", currentOutputPort.c_str());
			if (!inputRequiresUpdate && pLastDeviceSet != NULL) {
				ms_warning("Setting output device to match the current input device by default");
				linphone_core_set_output_audio_device(core->getCCore(), pLastDeviceSet);
			}
		}
		
		// Notify the filter that the audio route changed
		core->soundcardAudioRouteChanged();
		
	});
}

- (void)onStopAsyncEnd: (BOOL)stop {
	mStopAsyncEnd = stop;
}

@end



