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

#include "linphone/utils/general.h"
#include "linphone/utils/utils.h"
#include "c-wrapper/c-wrapper.h"

#include "logger/logger.h"
#include "platform-helpers.h"

// TODO: Remove me
#include "private.h"

#include "core/app/ios-app-delegate.h"
// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE
static IosAppDelegate *mAppDelegate;

class IosPlatformHelpers : public GenericPlatformHelpers {
public:
	IosPlatformHelpers (std::shared_ptr<LinphonePrivate::Core> core, void *systemContext);
	~IosPlatformHelpers () = default;

	void acquireWifiLock () override {}
	void releaseWifiLock () override {}
	void acquireMcastLock () override {}
	void releaseMcastLock () override {}
	void acquireCpuLock () override;
	void releaseCpuLock () override;

	string getConfigPath () const override { return ""; }
	string getDataPath () const override { return ""; }
	string getDataResource (const string &filename) const override;
	string getImageResource (const string &filename) const override;
	string getRingResource (const string &filename) const override;
	string getSoundResource (const string &filename) const override;
	void * getPathContext () override;

	string getWifiSSID() override;
	void setWifiSSID(const string &ssid) override;

	void setVideoPreviewWindow (void *windowId) override {}
	string getDownloadPath () override {return Utils::getEmptyConstRefObject<string>();}
	void setVideoWindow (void *windowId) override {}
	void resizeVideoPreview (int width, int height) override {}

	void onWifiOnlyEnabled (bool enabled) override;
	void setDnsServers () override;
	void setHttpProxy (const string &host, int port) override;
	bool startNetworkMonitoring() override;
	void stopNetworkMonitoring() override;

	void onLinphoneCoreStart (bool monitoringEnabled) override;
	void onLinphoneCoreStop () override;

	void startAudioForEchoTestOrCalibration () override;
	void stopAudioForEchoTestOrCalibration () override;

	//IosHelper specific
	bool isReachable(SCNetworkReachabilityFlags flags);
	void networkChangeCallback(void);
	void onNetworkChanged(bool reachable, bool force);
	void createAppDelegate() override;
	void destroyAppDelegate() override;
	void dummyConfigAudioSession() override;

private:
	string toUTF8String(CFStringRef str);
	void kickOffConnectivity();
	void getHttpProxySettings(void);

	void bgTaskTimeout ();
	static void sBgTaskTimeout (void *data);
	static string getResourceDirPath (const string &framework, const string &resource);
	static string getResourcePath (const string &framework, const string &resource);

	long int mCpuLockTaskId;
	int mCpuLockCount;
	SCNetworkReachabilityRef reachabilityRef = NULL;
	SCNetworkReachabilityFlags mCurrentFlags = 0;
	bool mNetworkMonitoringEnabled = false;
	static const string Framework;
};

static void sNetworkChangeCallback(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo);

// =============================================================================

const string IosPlatformHelpers::Framework = "org.linphone.linphone";

IosPlatformHelpers::IosPlatformHelpers (std::shared_ptr<LinphonePrivate::Core> core, void *systemContext) : GenericPlatformHelpers(core) {
	mCpuLockCount = 0;
	mCpuLockTaskId = 0;
	mNetworkReachable = 0; // wait until monitor to give a status;
	mSharedCoreHelpers = createIosSharedCoreHelpers(core);

	string cpimPath = getResourceDirPath(Framework, "cpim_grammar");
	if (!cpimPath.empty())
		belr::GrammarLoader::get().addPath(cpimPath);
	else
		ms_error("IosPlatformHelpers did not find cpim grammar resource directory...");

	// mSharedCoreHelpers->setupSharedCore(core->getCCore()->config);

	string identityPath = getResourceDirPath(Framework, "identity_grammar");
	if (!identityPath.empty())
		belr::GrammarLoader::get().addPath(identityPath);
	else
		ms_error("IosPlatformHelpers did not find identity grammar resource directory...");

#ifdef VCARD_ENABLED
	string vcardPath = getResourceDirPath("org.linphone.belcard", "vcard_grammar");
	if (!vcardPath.empty())
		belr::GrammarLoader::get().addPath(vcardPath);
	else
		ms_message("IosPlatformHelpers did not find vcard grammar resource directory...");
#endif
	ms_message("IosPlatformHelpers is fully initialised");
}

void IosPlatformHelpers::createAppDelegate() {
	mAppDelegate = [[IosAppDelegate alloc] init];
	[mAppDelegate setCore:getCore()];
	[mAppDelegate registerForPush];
}

void IosPlatformHelpers::destroyAppDelegate() {
	[mAppDelegate dealloc];
}

void IosPlatformHelpers::dummyConfigAudioSession() {
	NSError *err = nil;
	AVAudioSession *audioSession = [AVAudioSession sharedInstance];
	[audioSession   setCategory:AVAudioSessionCategoryPlayAndRecord
	withOptions:AVAudioSessionCategoryOptionAllowBluetooth| AVAudioSessionCategoryOptionAllowBluetoothA2DP
		  error:&err];
	double sampleRate = 48000; /*let's target the highest sample rate*/
	[audioSession setPreferredSampleRate:sampleRate error:&err];
}

//Safely get an UTF-8 string from the given CFStringRef
string IosPlatformHelpers::toUTF8String(CFStringRef str) {
	string ret;

	if (str == NULL) {
		return ret;
	}
	CFIndex length = CFStringGetLength(str);
	CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
	char *buffer = (char *) malloc((size_t) maxSize);
	if (buffer) {
		if (CFStringGetCString(str, buffer, maxSize, kCFStringEncodingUTF8)) {
			ret = buffer;
		}
		free(buffer);
	}
	return ret;
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
	// on iOS, cpu lock is implemented by a long running task and it is abstracted by belle-sip, so let's use belle-sip directly.
	if (mCpuLockCount == 0)
		mCpuLockTaskId = static_cast<long>(belle_sip_begin_background_task("Liblinphone cpu lock", sBgTaskTimeout, this));

	mCpuLockCount++;
}

void IosPlatformHelpers::releaseCpuLock () {
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

// -----------------------------------------------------------------------------

string IosPlatformHelpers::getDataResource (const string &filename) const {
	return getResourcePath(Framework, filename);
}

string IosPlatformHelpers::getImageResource (const string &filename) const {
	return getResourcePath(Framework, filename);
}

string IosPlatformHelpers::getRingResource (const string &filename) const {
	return getResourcePath(Framework, filename);
}

string IosPlatformHelpers::getSoundResource (const string &filename) const {
	return getResourcePath(Framework, filename);
}

// -----------------------------------------------------------------------------

string IosPlatformHelpers::getResourceDirPath (const string &framework, const string &resource) {
	CFStringEncoding encodingMethod = CFStringGetSystemEncoding();
	CFStringRef cfFramework = CFStringCreateWithCString(nullptr, framework.c_str(), encodingMethod);
	CFStringRef cfResource = CFStringCreateWithCString(nullptr, resource.c_str(), encodingMethod);
	CFBundleRef bundle = CFBundleGetBundleWithIdentifier(cfFramework);
	CFURLRef resourceUrl = CFBundleCopyResourceURL(bundle, cfResource, nullptr, nullptr);
	string path("");
	if (resourceUrl) {
		CFURLRef resourceUrlDirectory = CFURLCreateCopyDeletingLastPathComponent(nullptr, resourceUrl);
		CFStringRef resourcePath = CFURLCopyFileSystemPath(resourceUrlDirectory, kCFURLPOSIXPathStyle);
		path = CFStringGetCStringPtr(resourcePath, encodingMethod);
		CFRelease(resourcePath);
		CFRelease(resourceUrlDirectory);
		CFRelease(resourceUrl);
	}

	CFRelease(cfResource);
	CFRelease(cfFramework);
	return path;
}

string IosPlatformHelpers::getResourcePath (const string &framework, const string &resource) {
	return getResourceDirPath(framework, resource) + "/" + resource;
}

void *IosPlatformHelpers::getPathContext () {
	return getSharedCoreHelpers()->getPathContext();
}

void IosPlatformHelpers::onLinphoneCoreStart(bool monitoringEnabled) {
	mNetworkMonitoringEnabled = monitoringEnabled;
	if (monitoringEnabled) {
		startNetworkMonitoring();
		[mAppDelegate onLinphoneCoreStart];
	}
}

void IosPlatformHelpers::onLinphoneCoreStop() {
	if (mNetworkMonitoringEnabled) {
		stopNetworkMonitoring();
		[mAppDelegate onLinphoneCoreStop];
	}

	getSharedCoreHelpers()->onLinphoneCoreStop();
}

void IosPlatformHelpers::startAudioForEchoTestOrCalibration () {

}

void IosPlatformHelpers::stopAudioForEchoTestOrCalibration () {
	
}

void IosPlatformHelpers::onWifiOnlyEnabled(bool enabled) {
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

//Set proxy settings on core
void IosPlatformHelpers::setHttpProxy (const string &host, int port) {
	linphone_core_set_http_proxy_host(getCore()->getCCore(), host.c_str());
	linphone_core_set_http_proxy_port(getCore()->getCCore(), port);
}

//Get global proxy settings from system and set variables mHttpProxy{Host,Port,Enabled}.
void IosPlatformHelpers::getHttpProxySettings(void) {
	CFDictionaryRef proxySettings = CFNetworkCopySystemProxySettings();

	if (proxySettings) {
		CFNumberRef enabled = (CFNumberRef) CFDictionaryGetValue(proxySettings, kCFNetworkProxiesHTTPEnable);
		if (enabled != NULL) {
			int val = 0;
			CFNumberGetValue(enabled, kCFNumberIntType, &val);
			mHttpProxyEnabled = !!val;
		}
		if (mHttpProxyEnabled) {
			CFStringRef proxyHost = (CFStringRef) CFDictionaryGetValue(proxySettings, kCFNetworkProxiesHTTPProxy);
			if (proxyHost != NULL) {
				mHttpProxyHost = toUTF8String(proxyHost);
				if (mHttpProxyHost.empty()) {
					mHttpProxyEnabled = false;
				}
			} else {
				mHttpProxyEnabled = false;
			}
		}
		if (mHttpProxyEnabled) {
			CFNumberRef proxyPort = (CFNumberRef) CFDictionaryGetValue(proxySettings, kCFNetworkProxiesHTTPPort);
			if (proxyPort != NULL) {
				if (!CFNumberGetValue(proxyPort, kCFNumberIntType, &mHttpProxyPort)) {
					mHttpProxyEnabled = false;
				}
			} else {
				mHttpProxyEnabled = false;
			}
		}
		CFRelease(proxySettings);
	}
	if (!mHttpProxyEnabled) {
		mHttpProxyPort = 0;
		mHttpProxyHost.clear();
	}
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
	if (reachabilityRef) {
		CFRelease(reachabilityRef);
		reachabilityRef = NULL;
	}
	CFNotificationCenterRemoveObserver(CFNotificationCenterGetDarwinNotifyCenter(), (void *) this, CFSTR(kNotifySCNetworkChange), NULL);
}

//This callback keeps tracks of wifi SSID changes
static void sNetworkChangeCallback(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo) {
	if (!observer) {
		return;
	}
	IosPlatformHelpers *iosHelper = (IosPlatformHelpers *) observer;

	iosHelper->getCore()->doLater([iosHelper] () {
		iosHelper->networkChangeCallback();
	});
}

void IosPlatformHelpers::networkChangeCallback() {
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
		//Only check for wifi changes if current connection type is Wifi
		string newSSID = getWifiSSID();
		if (newSSID.empty() || newSSID.compare(mCurrentSSID) != 0) {
			setWifiSSID(newSSID);
			changed = true;
			//We possibly changed network, force reset of transports
			force = true;
			ms_message("New Wifi SSID detected: %s", mCurrentSSID.empty()?"[none]":mCurrentSSID.c_str());
		}
	}
	getHttpProxySettings();
	if (mHttpProxyEnabled) {
		ms_message("Update HTTP proxy settings: using [%s:%d]", mHttpProxyHost.c_str(), mHttpProxyPort);
	} else {
		ms_message("Updated HTTP proxy settings: no proxy");
	}
	const char *currentProxyHostCstr = linphone_core_get_http_proxy_host(getCore()->getCCore());
	string currentProxyHost;
	int currentProxyPort = linphone_core_get_http_proxy_port(getCore()->getCCore());
	if (currentProxyHostCstr) {
		currentProxyHost = currentProxyHostCstr;
	}
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
			int loop = 0;
			CFWriteStreamRef writeStream;
			CFStreamCreatePairWithSocketToHost(NULL, (CFStringRef) "192.168.0.200" /*"linphone.org"*/, 15000, nil,
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
				loop++;
			}

			if (status == kCFStreamStatusOpen) {
				CFWriteStreamWrite(writeStream, (const UInt8 *)buff, (CFIndex) strlen(buff));
			} else if (!timeout_reached) {
				CFErrorRef error = CFWriteStreamCopyError(writeStream);
				CFRelease(error);
			} else if (timeout_reached) {
				ms_message("CFStream timeout reached");
			}
			CFWriteStreamClose(writeStream);
			CFRelease(writeStream);
			in_progress = false;
		});
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
