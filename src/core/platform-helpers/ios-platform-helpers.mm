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
#include <bctoolbox/defs.h>

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif
#if TARGET_OS_IPHONE

#include <Foundation/Foundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/CaptiveNetwork.h>
#include <CoreLocation/CoreLocation.h>
#include <notify_keys.h>
#include <belr/grammarbuilder.h>
#include <AVFoundation/AVAudioSession.h>

#include "core/core.h"
#include "core/core-p.h"
#include "linphone/core.h"
#include "linphone/utils/general.h"
#include "linphone/utils/utils.h"
#include "c-wrapper/c-wrapper.h"

#include "logger/logger.h"
#include "mac-platform-helpers.h"

// TODO: Remove me
#include "private.h"

#include "core/app/ios-app-delegate.h"

#import <AVFoundation/AVFoundation.h>
// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

class IosPlatformHelpers : public MacPlatformHelpers, public CoreListener {
public:
	IosPlatformHelpers (std::shared_ptr<LinphonePrivate::Core> core, void *systemContext);
	~IosPlatformHelpers () {
		[mAppDelegate dealloc];
	}

	void acquireCpuLock () override;
	void releaseCpuLock () override;

	void * getPathContext () override;

	NetworkType getNetworkType()const override;
	string getWifiSSID() override;
	void setWifiSSID(const string &ssid) override;

	void setVideoPreviewWindow (BCTBX_UNUSED(void *windowId)) override {}
	string getDownloadPath () override {return Utils::getEmptyConstRefObject<string>();}
	void setVideoWindow (BCTBX_UNUSED(void *windowId)) override {}
	void setParticipantDeviceVideoWindow(BCTBX_UNUSED(LinphoneParticipantDevice *participantDevice), BCTBX_UNUSED(void* windowId)) override {};
	void resizeVideoPreview (BCTBX_UNUSED(int width), BCTBX_UNUSED(int height)) override {}

	void onWifiOnlyEnabled (bool enabled) override;
	bool isActiveNetworkWifiOnlyCompliant () const override;
	void setDnsServers () override;
	bool startNetworkMonitoring() override;
	void stopNetworkMonitoring() override;

	void onLinphoneCoreStart (bool monitoringEnabled) override;
	void onLinphoneCoreStop () override;

	void startAudioForEchoTestOrCalibration () override;
	void stopAudioForEchoTestOrCalibration () override;
	
	void start (std::shared_ptr<LinphonePrivate::Core> core) override;
	void stop (void) override;

	//IosHelper specific
	bool isReachable(SCNetworkReachabilityFlags flags);
	void networkChangeCallback(void);
	void onNetworkChanged(bool reachable, bool force);

	void didRegisterForRemotePush(void *token) override;
	void didRegisterForRemotePushWithStringifiedToken(const char *token) override;
	void setPushAndAppDelegateDispatchQueue(void *dispatchQueue) override;
	void enableAutoIterate (bool autoIterateEnabled) override;

	void onRecordingStarted () const override;
	void onRecordingPaused () const override;
	void stopRinging () const override;

	void setDeviceRotation (int orientation) const override;

private:
	void kickOffConnectivity();
	void bgTaskTimeout ();
	static void sBgTaskTimeout (void *data);
    void onGlobalStateChanged (LinphoneGlobalState state) override;

	long int mCpuLockTaskId;
	int mCpuLockCount;
	SCNetworkReachabilityRef reachabilityRef = NULL;
	SCNetworkReachabilityFlags mCurrentFlags = 0;
	bool mNetworkMonitoringEnabled = false;

	IosAppDelegate *mAppDelegate = NULL; /* auto didEnterBackground/didEnterForeground and other callbacks */
	bool mStart = false; /* generic platformhelper's funcs only work when mStart is true */
	bool mUseAppDelgate = false; /* app delegate is only used by main core*/
	NSTimer* mIterateTimer = NULL;
};

static void sNetworkChangeCallback(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo);

// =============================================================================


IosPlatformHelpers::IosPlatformHelpers (std::shared_ptr<LinphonePrivate::Core> core, BCTBX_UNUSED(void *systemContext)) : MacPlatformHelpers(core) {
	mUseAppDelgate = core->getCCore()->is_main_core;
	if (mUseAppDelgate) {
		mAppDelegate = [[IosAppDelegate alloc] initWithCore:core];
	}
	ms_message("IosPlatformHelpers is fully initialised");
}

void IosPlatformHelpers::start (std::shared_ptr<LinphonePrivate::Core> core) {
	mCpuLockCount = 0;
	mCpuLockTaskId = 0;
	mNetworkReachable = 0; // wait until monitor to give a status;
	mSharedCoreHelpers = createIosSharedCoreHelpers(core);
		
	ms_message("IosPlatformHelpers is fully started");
	mStart = true;
	[mAppDelegate onStopAsyncEnd:false];
}

void IosPlatformHelpers::stop () {
	mStart = false;
	ms_message("IosPlatformHelpers is fully stopped");
}

// Make sure that we register for push notification token after the core is GlobalOn
// in order to avoid push token being erased by remote provisioning
void IosPlatformHelpers::onGlobalStateChanged (LinphoneGlobalState state) {
	if (state == LinphoneGlobalOn) {
	    if (mUseAppDelgate && linphone_core_is_push_notification_enabled(getCore()->getCCore())) {
		    [mAppDelegate registerForPush];
	    }
	}
}

void IosPlatformHelpers::didRegisterForRemotePush(void *token) {
	[mAppDelegate didRegisterForRemotePush:(NSData *)token];
}

void IosPlatformHelpers::didRegisterForRemotePushWithStringifiedToken(const char *tokenStr) {
	[mAppDelegate didRegisterForRemotePushWithStringifiedToken:tokenStr];
}

void IosPlatformHelpers::setPushAndAppDelegateDispatchQueue(void *dispatchQueue) {
	[mAppDelegate setPushAndAppDelegateDispatchQueue:dispatchQueue];
}

void IosPlatformHelpers::enableAutoIterate(bool autoIterateEnabled) {
	if (mUseAppDelgate && mStart) {
		if (autoIterateEnabled) {
			if (mIterateTimer && mIterateTimer.valid) {
				ms_message("[IosPlatformHelpers] core.iterate() is already scheduled");
				return;
			}
			mIterateTimer = [NSTimer timerWithTimeInterval:0.02 target:mAppDelegate selector:@selector(iterate) userInfo:nil repeats:YES];
			// NSTimer runs only in the main thread correctly. Since there may not be a current thread loop.
			[[NSRunLoop mainRunLoop] addTimer:mIterateTimer forMode:NSDefaultRunLoopMode];
			ms_message("[IosPlatformHelpers] Call to core.iterate() scheduled every 20ms");
		} else {
			if (mIterateTimer) {
				[mIterateTimer invalidate];
				mIterateTimer = NULL;
				ms_message("[IosPlatformHelpers] Auto core.iterate() stopped");
			}
		}
	}
}

void IosPlatformHelpers::onRecordingStarted() const {

}

void IosPlatformHelpers::onRecordingPaused() const {

}

void IosPlatformHelpers::stopRinging () const {
	
}

void IosPlatformHelpers::setDeviceRotation (BCTBX_UNUSED(int orientation)) const {

}

// -----------------------------------------------------------------------------

void IosPlatformHelpers::bgTaskTimeout () {
	ms_error("IosPlatformHelpers: the system requests that the cpu lock is released now.");
	if (mCpuLockTaskId != 0) {
		belle_sip_end_background_task(static_cast<unsigned long>(mCpuLockTaskId));
		mCpuLockTaskId = 0;
	}
}

void IosPlatformHelpers::sBgTaskTimeout (void *data) {
	IosPlatformHelpers *zis = static_cast<IosPlatformHelpers *>(data);
	zis->bgTaskTimeout();
}

// -----------------------------------------------------------------------------

void IosPlatformHelpers::acquireCpuLock () {
	if (!mStart) return;

	// on iOS, cpu lock is implemented by a long running task and it is abstracted by belle-sip, so let's use belle-sip directly.
	if (mCpuLockCount == 0)
		mCpuLockTaskId = static_cast<long>(belle_sip_begin_background_task("Liblinphone cpu lock", sBgTaskTimeout, this));

	mCpuLockCount++;
}

void IosPlatformHelpers::releaseCpuLock () {
	if (!mStart) return;

	mCpuLockCount--;
	if (mCpuLockCount != 0)
		return;

	if (mCpuLockTaskId == 0) {
		ms_error("IosPlatformHelpers::releaseCpuLock(): too late, the lock has been released already by the system.");
		return;
	}

	belle_sip_end_background_task(static_cast<unsigned long>(mCpuLockTaskId));
	mCpuLockTaskId = 0;
}

void *IosPlatformHelpers::getPathContext () {
	return getSharedCoreHelpers()->getPathContext();
}

void IosPlatformHelpers::onLinphoneCoreStart(bool monitoringEnabled) {
	if (!mStart) return;

	mNetworkMonitoringEnabled = monitoringEnabled;
	if (monitoringEnabled) {
		startNetworkMonitoring();
	}
	if (mUseAppDelgate && linphone_core_is_auto_iterate_enabled(getCore()->getCCore())) {
		enableAutoIterate(TRUE);
	} else {
		ms_warning("[IosPlatformHelpers] Auto core.iterate() isn't enabled, ensure you do it in your application!");
	}
    
    ms_message("[IosPlatformHelpers] Register onGlobalStateChanged listener to request push notification when finished starting");
    L_GET_PRIVATE(getCore())->registerListener(this);
}

void IosPlatformHelpers::onLinphoneCoreStop() {
	if (!mStart) return;

	if (mNetworkMonitoringEnabled) {
		stopNetworkMonitoring();
	}

	if (mUseAppDelgate && linphone_core_is_auto_iterate_enabled(getCore()->getCCore())) {
		enableAutoIterate(FALSE);
	}
    
    try {
        L_GET_PRIVATE(getCore())->unregisterListener(this);
    } catch (...) {
        
    }
	// To avoid trigger callbacks of mHandler after linphone core stop
	[mAppDelegate onStopAsyncEnd:true];
	getSharedCoreHelpers()->onLinphoneCoreStop();
}

void IosPlatformHelpers::startAudioForEchoTestOrCalibration () {

}

void IosPlatformHelpers::stopAudioForEchoTestOrCalibration () {
	
}

void IosPlatformHelpers::onWifiOnlyEnabled(bool enabled) {
	if (!mStart) return;

	mWifiOnly = enabled;
	if (isNetworkReachable()) {
		//Nothing to do if we have no connection
		if (enabled && (mCurrentFlags & kSCNetworkReachabilityFlagsIsWWAN)) {
			onNetworkChanged(false, true);
		}
	}
}

void IosPlatformHelpers::setDnsServers () {
	//Nothing to do here, already handled by core for IOS platforms
}


static void showNetworkFlags(SCNetworkReachabilityFlags flags) {
	ms_message("Network connection flags:");

	if (flags == 0)
		ms_message("no flags.");
	if (flags & kSCNetworkReachabilityFlagsTransientConnection)
		ms_message("kSCNetworkReachabilityFlagsTransientConnection, ");
	if (flags & kSCNetworkReachabilityFlagsReachable)
		ms_message("kSCNetworkReachabilityFlagsReachable, ");
	if (flags & kSCNetworkReachabilityFlagsConnectionRequired)
		ms_message("kSCNetworkReachabilityFlagsConnectionRequired, ");
	if (flags & kSCNetworkReachabilityFlagsConnectionOnTraffic)
		ms_message("kSCNetworkReachabilityFlagsConnectionOnTraffic, ");
	if (flags & kSCNetworkReachabilityFlagsConnectionOnDemand)
		ms_message("kSCNetworkReachabilityFlagsConnectionOnDemand, ");
	if (flags & kSCNetworkReachabilityFlagsIsLocalAddress)
		ms_message("kSCNetworkReachabilityFlagsIsLocalAddress, ");
	if (flags & kSCNetworkReachabilityFlagsIsDirect)
		ms_message("kSCNetworkReachabilityFlagsIsDirect, ");
	if (flags & kSCNetworkReachabilityFlagsIsWWAN)
		ms_message("kSCNetworkReachabilityFlagsIsWWAN, ");
	ms_message("\n");
}

bool IosPlatformHelpers::startNetworkMonitoring(void) {
	if (!mStart) return false;

	if (reachabilityRef != NULL) {
		stopNetworkMonitoring();
	}
	//Trying to reach captive.apple.com by default.	Apple uses it already to check if wifi is a captive network
	//See /Library/Preferences/SystemConfiguration/CaptiveNetworkSupport/Settings.plist for more URL examples
	string reachabilityTargetHost = "captive.apple.com";
	LinphoneProxyConfig *proxy = linphone_core_get_default_proxy_config(getCore()->getCCore());

	//Try to reach default proxy config server if defined
	if (proxy) {
		reachabilityTargetHost = linphone_proxy_config_get_server_addr(proxy);
	}

	reachabilityRef = SCNetworkReachabilityCreateWithName(NULL, reachabilityTargetHost.c_str());
	if (reachabilityRef == NULL) {
		return false;
	}
	CFNotificationCenterAddObserver(CFNotificationCenterGetDarwinNotifyCenter(), (void *) this, sNetworkChangeCallback, CFSTR(kNotifySCNetworkChange), NULL, CFNotificationSuspensionBehaviorDeliverImmediately /*Ignored*/);

	//Load and trigger initial state
	networkChangeCallback();
	return true;
}

void IosPlatformHelpers::stopNetworkMonitoring(void) {
	if (!mStart) return;

	if (reachabilityRef) {
		CFRelease(reachabilityRef);
		reachabilityRef = NULL;
	}
	CFNotificationCenterRemoveObserver(CFNotificationCenterGetDarwinNotifyCenter(), (void *) this, CFSTR(kNotifySCNetworkChange), NULL);
}

//This callback keeps tracks of wifi SSID changes
static void sNetworkChangeCallback(BCTBX_UNUSED(CFNotificationCenterRef center), void *observer, BCTBX_UNUSED(CFStringRef name), BCTBX_UNUSED(const void *object), BCTBX_UNUSED(CFDictionaryRef userInfo)) {
	if (!observer) {
		return;
	}
	IosPlatformHelpers *iosHelper = (IosPlatformHelpers *) observer;

	iosHelper->getCore()->doLater([iosHelper] () {
		iosHelper->networkChangeCallback();
	});
}

void IosPlatformHelpers::networkChangeCallback() {
	if (!mStart) return;

	//We receive this notification multiple time for possibly unknown reasons
	//So take actions only if state changed of our internal network-related properties

	bool reachable = isNetworkReachable(), changed = false, force = false;

	SCNetworkReachabilityFlags flags;
	if (SCNetworkReachabilityGetFlags(reachabilityRef, &flags)) {
		showNetworkFlags(flags);

		reachable = isReachable(flags);
		if (flags != mCurrentFlags || reachable != isNetworkReachable()) {
			changed = true;
			if (mCurrentFlags == 0
				||
				/*check if moving from Wifi to cellular*/
				(mCurrentFlags & kSCNetworkReachabilityFlagsIsWWAN) != (flags & kSCNetworkReachabilityFlagsIsWWAN)) {
				//Force reinit after network down
				force = true;
			}
		}
		mCurrentFlags = flags;
	}
	if (!(flags & kSCNetworkReachabilityFlagsIsWWAN)) {
/*
 * CHECK_SSID is undefined by default. Previously we are monitoring SSID changes to notify liblinphone of possible network changes.
 * Since iOS 12, it requires an annoying location permission. For that reason, the default algorithm just monitors
 * the default local ip addresses for changes. A bit less reliable since we can change from wifi network but by change obtain the same ip address.
 * But much less annoying because it doesn't require to ask a permission to the user.
 */
#ifdef CHECK_SSID
		//Only check for wifi changes if current connection type is Wifi
		string newSSID = getWifiSSID();
		if (newSSID.empty() || newSSID.compare(mCurrentSSID) != 0) {
			setWifiSSID(newSSID);
			changed = true;
			//We possibly changed network, force reset of transports
			force = true;
			ms_message("New Wifi SSID detected: %s", mCurrentSSID.empty()?"[none]":mCurrentSSID.c_str());
		}
#else
        if (reachable && !force){
            force = checkIpAddressChanged();
        }
#endif
	}
	if (!!linphone_core_automatic_http_proxy_detection_enabled(getCore()->getCCore())) {
		ms_message("Do not try to get HTTP proxy as automatic proxy detection is enabled");
	} else {
		getHttpProxySettings();
	}
	if (mHttpProxyEnabled) {
		ms_message("Update HTTP proxy settings: using [%s:%d]", mHttpProxyHost.c_str(), mHttpProxyPort);
	} else {
		ms_message("Updated HTTP proxy settings: no proxy");
	}
	const char *currentProxyHostCstr = linphone_core_get_http_proxy_host(getCore()->getCCore());
	string currentProxyHost;
	if (currentProxyHostCstr) {
		currentProxyHost = currentProxyHostCstr;
	}
	int currentProxyPort = linphone_core_get_http_proxy_port(getCore()->getCCore());
	if (mHttpProxyEnabled == currentProxyHost.empty() || currentProxyPort != mHttpProxyPort || currentProxyHost.compare(mHttpProxyHost)) {
		//Empty host is considered as no proxy
		changed = true;
		force = true;
	}
	if (changed) {
		onNetworkChanged(reachable, force);
	}
}

//Get reachability state from given flags
bool IosPlatformHelpers::isReachable(SCNetworkReachabilityFlags flags) {
	if (flags) {
		if (flags & kSCNetworkReachabilityFlagsConnectionOnDemand) {
			//Assume unreachable for now. Wait for kickoff and for later network change events
			kickOffConnectivity();
			return false;
		}
		if (flags & (kSCNetworkReachabilityFlagsConnectionRequired | kSCNetworkReachabilityFlagsInterventionRequired)) {
			return false;
		}
		if (!(flags & kSCNetworkReachabilityFlagsReachable)) {
			return false;
		}
		bool isWifiOnly = linphone_core_wifi_only_enabled(getCore()->getCCore());
		if (isWifiOnly && (flags & kSCNetworkReachabilityFlagsIsWWAN)) {
			return false;
		}
	} else {
		return false;
	}
	return true;
}

bool IosPlatformHelpers::isActiveNetworkWifiOnlyCompliant() const {
	return false;
}

//Method called when we detected actual network changes in callbacks
void IosPlatformHelpers::onNetworkChanged(bool reachable, bool force) {
	if (reachable != isNetworkReachable() || force) {
		ms_message("Global network status changed: reachable: [%d].", (int) reachable);
		setHttpProxy(mHttpProxyHost, mHttpProxyPort);
		if (force && reachable){
			//mandatory to  trigger action from the core in case of switch from 3G to wifi (both up)
			setNetworkReachable(FALSE);
		}
		setNetworkReachable(reachable);
	}
}

//In case reachability flag "kSCNetworkReachabilityFlagsConnectionOnDemand" is set, we have to use CFStream APIs for the connection to be opened
//Start asynchronously the connection kickoff and do not change reachability status.
//We assume we will be notified via callbacks if connection changes state afterwards
//192.168.0.200	is just an arbitrary address. It should	still activate the on-demand connection even if not routable
void IosPlatformHelpers::kickOffConnectivity() {
	static bool in_progress = false;
	if (in_progress) {
		return;
	}
	in_progress = true;
	/* start a new thread to avoid blocking the main ui in case of peer host failure */
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
			static useconds_t sleep_us = 10000;
			static int timeout_s = 5;
			bool timeout_reached = FALSE;
			CFWriteStreamRef writeStream;
			CFStreamCreatePairWithSocketToHost(NULL, (CFStringRef) @"192.168.0.200" /*"linphone.org"*/, 15000, nil,
							   &writeStream);
			bool res = CFWriteStreamOpen(writeStream);
			const char *buff = "hello";
			time_t start = time(NULL);
			time_t loop_time;

			if (res == false) {
				ms_error("Could not open write stream, backing off");
				CFRelease(writeStream);
				in_progress = FALSE;
				return;
			}

			// check stream status and handle timeout
			CFStreamStatus status = CFWriteStreamGetStatus(writeStream);
			while (status != kCFStreamStatusOpen && status != kCFStreamStatusError) {
				usleep(sleep_us);
				status = CFWriteStreamGetStatus(writeStream);
				loop_time = time(NULL);
				if (loop_time - start >= timeout_s) {
					timeout_reached = false;
					break;
				}
			}

			if (status == kCFStreamStatusOpen) {
				CFWriteStreamWrite(writeStream, (const UInt8 *)buff, (CFIndex) strlen(buff));
			} else if (!timeout_reached) {
				CFErrorRef cfError = CFWriteStreamCopyError(writeStream);
				if (cfError) {
					NSError *error = (__bridge NSError *)cfError;
					lError() << "CFStream error " << error.localizedDescription;
					CFRelease(cfError);
				}
			} else if (timeout_reached) {
				ms_message("CFStream timeout reached");
			}
			CFWriteStreamClose(writeStream);
			CFRelease(writeStream);
			in_progress = false;
		});
}

PlatformHelpers::NetworkType IosPlatformHelpers::getNetworkType()const{
	return (mCurrentFlags & kSCNetworkReachabilityFlagsIsWWAN) ? NetworkType::MobileData : NetworkType::Wifi;
}

void IosPlatformHelpers::setWifiSSID(const string &ssid) {
	mCurrentSSID = ssid;
}

string IosPlatformHelpers::getWifiSSID(void) {
	#if TARGET_IPHONE_SIMULATOR
		return "Sim_err_SSID_NotSupported";
	#else
		string ssid;
		bool shallGetWifiInfo = true;

		if (@available(iOS 13.0, *)) {
			//Starting from IOS13 we need to check for authorization to get wifi information.
			//User permission is asked in the main app
			CLAuthorizationStatus status = [CLLocationManager authorizationStatus];
			if (status != kCLAuthorizationStatusAuthorizedAlways &&
				status != kCLAuthorizationStatusAuthorizedWhenInUse) {
				shallGetWifiInfo = false;
				ms_warning("User has not given authorization to access Wifi information (Authorization: [%d])", (int) status);
			}
		}
		if (shallGetWifiInfo) {
			CFArrayRef ifaceNames = CNCopySupportedInterfaces();
			if (ifaceNames) {
				CFIndex	i;
				for (i = 0; i < CFArrayGetCount(ifaceNames); ++i) {
					CFStringRef iface = (CFStringRef) CFArrayGetValueAtIndex(ifaceNames, i);
					CFDictionaryRef ifaceInfo = CNCopyCurrentNetworkInfo(iface);

					if (ifaceInfo != NULL && CFDictionaryGetCount(ifaceInfo) > 0) {
						CFStringRef ifaceSSID = (CFStringRef) CFDictionaryGetValue(ifaceInfo, kCNNetworkInfoKeySSID);
						if (ifaceSSID != NULL) {
							ssid = toUTF8String(ifaceSSID);
							if (!ssid.empty()) {
								CFRelease(ifaceInfo);
								break;
							}
						}
						CFRelease(ifaceInfo);
					}
				}
			}
			CFRelease(ifaceNames);
		}
		return ssid;
	#endif
}

// -----------------------------------------------------------------------------

PlatformHelpers *createIosPlatformHelpers(std::shared_ptr<LinphonePrivate::Core> core, void *systemContext) {
	return new IosPlatformHelpers(core, systemContext);
}


LINPHONE_END_NAMESPACE

#endif
