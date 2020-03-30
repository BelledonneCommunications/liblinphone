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

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif
#if TARGET_OS_IPHONE

#include <Foundation/Foundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/CaptiveNetwork.h>
#include <CoreLocation/CoreLocation.h>
#include <notify_keys.h>
#include <mutex>
#include <set>
#include <belr/grammarbuilder.h>

#include "linphone/utils/general.h"
#include "linphone/utils/utils.h"
#include "chat/chat-room/chat-room.h"
#include "c-wrapper/c-wrapper.h"

#include "logger/logger.h"
#include "platform-helpers.h"

// TODO: Remove me
#include "private.h"

#define ACTIVE_SHARED_CORE "ACTIVE_SHARED_CORE"
#define LAST_UPDATE_TIME_SHARED_CORE "LAST_UPDATE_TIME_SHARED_CORE"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

typedef enum {
	noCoreStarted,
	mainCoreStarted,
	executorCoreStarted
} SharedCoreState;

string sharedStateToString(int state) {
   switch(state) {
      case noCoreStarted:
         return "noCoreStarted";
      case mainCoreStarted:
         return "mainCoreStarted";
      case executorCoreStarted:
         return "executorCoreStarted";
		default:
         return "Invalid state";
   }
}

void on_core_must_stop(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo);

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

	std::shared_ptr<ChatMessage> getPushNotificationMessage(const string &callId) override;
	std::shared_ptr<ChatMessage> processPushNotificationMessage(const string &callId);
	std::shared_ptr<ChatRoom> getPushNotificationChatRoom(const string &chatRoomAddr) override;
	std::shared_ptr<ChatRoom> processPushNotificationChatRoom(const string &chatRoomAddr);

	// shared core
	bool isCoreShared() override;
	bool canCoreStart() override;
	void onCoreMustStop();
	void resetSharedCoreState() override;
	void unlockSharedCoreIfNeeded() override;

	// push notif
	void clearCallIdList();
	void addNewCallIdToList(string callId);
	void removeCallIdFromList(string callId);
	void setChatRoomInvite(std::shared_ptr<ChatRoom> chatRoom);
	std::string getChatRoomAddr();
	void reinitTimer();

	//IosHelper specific
	bool isReachable(SCNetworkReachabilityFlags flags);
	void networkChangeCallback(void);
	void onNetworkChanged(bool reachable, bool force);

private:
	string toUTF8String(CFStringRef str);
	void kickOffConnectivity();
	void getHttpProxySettings(void);

	void bgTaskTimeout ();
	static void sBgTaskTimeout (void *data);
	static string getResourceDirPath (const string &framework, const string &resource);
	static string getResourcePath (const string &framework, const string &resource);

	// shared core
	void setupSharedCore(struct _LpConfig *config);
	bool isSharedCoreStarted();
	SharedCoreState getSharedCoreState();
	void setSharedCoreState(SharedCoreState sharedCoreState);
	void reloadConfig();
	void resetSharedCoreLastUpdateTime();
	NSInteger getSharedCoreLastUpdateTime();

	// shared core : executor
	bool canExecutorCoreStart();
	void subscribeToMainCoreNotifs();

	// shared core : main
	bool canMainCoreStart();
	void stopSharedCores();

	//push notif
	std::shared_ptr<ChatMessage> getChatMsgAndUpdateList(const string &callId);

	long int mCpuLockTaskId;
	int mCpuLockCount;
	SCNetworkReachabilityRef reachabilityRef = NULL;
	SCNetworkReachabilityFlags mCurrentFlags = 0;
	bool mNetworkMonitoringEnabled = false;
	static const string Framework;
	std::string mAppGroupId = "";

	// push notif attributes
	static set<string> callIdList;
	static std::mutex callIdListMutex;
	static std::mutex executorCoreMutex;
	std::shared_ptr<ChatRoom> mChatRoomInvite = nullptr;
	std::string mChatRoomAddr;
	uint64_t mTimer = 0;
	CFRunLoopTimerRef mUnlockTimer;
	uint64_t mMaxIterationTimeMs;
};

static void sNetworkChangeCallback(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo);

// =============================================================================

const string IosPlatformHelpers::Framework = "org.linphone.linphone";
set<string> IosPlatformHelpers::callIdList = set<string>();
std::mutex IosPlatformHelpers::callIdListMutex;
std::mutex IosPlatformHelpers::executorCoreMutex;

IosPlatformHelpers::IosPlatformHelpers (std::shared_ptr<LinphonePrivate::Core> core, void *systemContext) : GenericPlatformHelpers(core) {
	mCpuLockCount = 0;
	mCpuLockTaskId = 0;
	mNetworkReachable = 0; // wait until monitor to give a status;

	string cpimPath = getResourceDirPath(Framework, "cpim_grammar");
	if (!cpimPath.empty())
		belr::GrammarLoader::get().addPath(cpimPath);
	else
		ms_error("IosPlatformHelpers did not find cpim grammar resource directory...");

	setupSharedCore(core->getCCore()->config);

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
	return mAppGroupId.empty() ? NULL : ms_strdup(mAppGroupId.c_str());
}

void IosPlatformHelpers::onLinphoneCoreStart(bool monitoringEnabled) {
	mNetworkMonitoringEnabled = monitoringEnabled;
	if (monitoringEnabled) {
		startNetworkMonitoring();
	}
}

void IosPlatformHelpers::onLinphoneCoreStop() {
	if (mNetworkMonitoringEnabled) {
		stopNetworkMonitoring();
	}
	if (isCoreShared()) {
		CFRunLoopTimerInvalidate(mUnlockTimer);
		ms_message("[SHARED] stop timer");

		bool needUnlock = (getSharedCoreState() == SharedCoreState::executorCoreStarted);

		bool needStateReset = (getCore()->getCCore()->is_main_core && getSharedCoreState() == SharedCoreState::mainCoreStarted) ||
							  (!getCore()->getCCore()->is_main_core && getSharedCoreState() == SharedCoreState::executorCoreStarted);
		if (needStateReset) {
			setSharedCoreState(SharedCoreState::noCoreStarted);
		}
		if (needUnlock) {
			ms_message("[push] unlock executorCoreMutex");
			IosPlatformHelpers::executorCoreMutex.unlock();
		}
	}
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
	CFNotificationCenterRemoveObserver(CFNotificationCenterGetDarwinNotifyCenter(), (void *) this, CFSTR("com.apple.system.config.network_change"), NULL);
}

//This callback keeps tracks of wifi SSID changes
static void sNetworkChangeCallback(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo) {
	if (!observer) {
		return;
	}
	IosPlatformHelpers *iosHelper = (IosPlatformHelpers *) observer;
	//Important: actual actions on core and possibly on UI are to be taken from main thread
	dispatch_async(dispatch_get_main_queue(), ^{
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

std::shared_ptr<ChatMessage> IosPlatformHelpers::getPushNotificationMessage(const string &callId) {
	ms_message("[push] getPushNotificationMessage");
	addNewCallIdToList(callId);
	switch(getSharedCoreState()) {
		case SharedCoreState::mainCoreStarted:
			clearCallIdList();
			IosPlatformHelpers::executorCoreMutex.unlock();
			ms_message("[push] mainCoreStarted");
			return nullptr;
		case SharedCoreState::executorCoreStarted:
			ms_message("[push] executorCoreStarted");
		case SharedCoreState::noCoreStarted:
			IosPlatformHelpers::executorCoreMutex.lock();
			ms_message("[push] noCoreStarted");
			std::shared_ptr<ChatMessage> msg = processPushNotificationMessage(callId);
			return msg;
	}
}

std::shared_ptr<ChatRoom> IosPlatformHelpers::getPushNotificationChatRoom(const string &chatRoomAddr) {
	ms_message("[push] getPushNotificationInvite");
	switch(getSharedCoreState()) {
		case SharedCoreState::mainCoreStarted:
			IosPlatformHelpers::executorCoreMutex.unlock();
			ms_message("[push] mainCoreStarted");
			return nullptr;
		case SharedCoreState::executorCoreStarted:
			ms_message("[push] executorCoreStarted");
		case SharedCoreState::noCoreStarted:
			IosPlatformHelpers::executorCoreMutex.lock();
			ms_message("[push] noCoreStarted");
			std::shared_ptr<ChatRoom> chatRoom = processPushNotificationChatRoom(chatRoomAddr);
			return chatRoom;
	}
}

void IosPlatformHelpers::clearCallIdList() {
	IosPlatformHelpers::callIdListMutex.lock();
	IosPlatformHelpers::callIdList.clear();
	IosPlatformHelpers::callIdListMutex.unlock();
	ms_debug("[push] clear callIdList");
}

void IosPlatformHelpers::addNewCallIdToList(string callId) {
	IosPlatformHelpers::callIdListMutex.lock();
	IosPlatformHelpers::callIdList.insert(callId);
	IosPlatformHelpers::callIdListMutex.unlock();
	ms_debug("[push] add %s to callIdList if not already present", callId.c_str());
}

void IosPlatformHelpers::removeCallIdFromList(string callId) {
	IosPlatformHelpers::callIdListMutex.lock();
	if (IosPlatformHelpers::callIdList.erase(callId)) {
		ms_message("[push] removed %s from callIdList", callId.c_str());
	} else {
		ms_message("[push] unable to remove %s from callIdList: not found", callId.c_str());
	}
	IosPlatformHelpers::callIdListMutex.unlock();
}

static void on_push_notification_message_received(LinphoneCore *lc, LinphoneChatRoom *room, LinphoneChatMessage *message) {
	const char *contentType = linphone_chat_message_get_content_type(message);
	if (strcmp(contentType, "text/plain") == 0 || strcmp(contentType, "image/jpeg") == 0) {
		ms_message("[push] msg [%p] received from chat room [%p]", message, room);
		const char *callId = linphone_chat_message_get_call_id(message);
		static_cast<LinphonePrivate::IosPlatformHelpers*>(lc->platform_helper)->removeCallIdFromList(callId);
	}
}

std::shared_ptr<ChatMessage> IosPlatformHelpers::getChatMsgAndUpdateList(const string &callId) {
	std::shared_ptr<ChatMessage> msg = getCore()->findChatMessageFromCallId(callId);
	if (msg) removeCallIdFromList(callId);
	return msg;
}

std::shared_ptr<ChatMessage> IosPlatformHelpers::processPushNotificationMessage(const string &callId) {
	std::shared_ptr<ChatMessage> chatMessage;
	ms_message("[push] processPushNotificationMessage for callid [%s]", callId.c_str());

	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
 	linphone_core_cbs_set_message_received(cbs, on_push_notification_message_received);
	linphone_core_add_callbacks(getCore()->getCCore(), cbs);

	if (linphone_core_get_global_state(getCore()->getCCore()) != LinphoneGlobalOn && linphone_core_start(getCore()->getCCore()) != 0) {
		linphone_core_cbs_unref(cbs);
		return nullptr;
	}
	ms_message("[push] core started");

	chatMessage = getChatMsgAndUpdateList(callId);
	ms_message("[push] message already in db? %s", chatMessage? "yes" : "no");
	if (chatMessage) {
		linphone_core_cbs_unref(cbs);
		return chatMessage;
	}

	/* init with 0 to look for the msg when newMsgNb <= 0 and then try to get the msg every seconds */
	uint64_t secondsTimer = 0;
	reinitTimer();

	while (ms_get_cur_time_ms() - mTimer < mMaxIterationTimeMs && (!IosPlatformHelpers::callIdList.empty() || !chatMessage)) {
		ms_message("[push] wait for msg. callIdList size = %lu", IosPlatformHelpers::callIdList.size());
		linphone_core_iterate(getCore()->getCCore());
		ms_usleep(50000);
		if (IosPlatformHelpers::callIdList.empty() && ms_get_cur_time_ms() - secondsTimer >= 1000) {
			chatMessage = getChatMsgAndUpdateList(callId);
			secondsTimer = ms_get_cur_time_ms();
		}
	}

	/* In case we wait for 25 seconds, there is probably a problem. So we reset the msg
	counter to prevent each push to wait 25 sec for messages that won't be received */
	if (ms_get_cur_time_ms() - mTimer >= 25000) clearCallIdList();

	chatMessage = getChatMsgAndUpdateList(callId);
	ms_message("[push] message received? %s", chatMessage? "yes" : "no");

	linphone_core_cbs_unref(cbs);
	return chatMessage;
}

void IosPlatformHelpers::setChatRoomInvite(std::shared_ptr<ChatRoom> chatRoom) {
	mChatRoomInvite = chatRoom;
}

std::string IosPlatformHelpers::getChatRoomAddr() {
	return mChatRoomAddr;
}

void IosPlatformHelpers::reinitTimer() {
	mTimer = ms_get_cur_time_ms();
}

static void on_push_notification_chat_room_invite_received(LinphoneCore *lc, LinphoneChatRoom *cr, LinphoneChatRoomState state) {
	if (state == LinphoneChatRoomStateCreated) {
		IosPlatformHelpers *platform_helper = static_cast<LinphonePrivate::IosPlatformHelpers*>(lc->platform_helper);
		platform_helper->reinitTimer();

		const char *cr_peer_addr = linphone_address_get_username(linphone_chat_room_get_peer_address(cr));
		ms_message("[push] we are added to the chat room %s", cr_peer_addr);

		if (strcmp(cr_peer_addr, platform_helper->getChatRoomAddr().c_str()) == 0) {
			ms_message("[push] the chat room associated with the push is found");
			platform_helper->setChatRoomInvite(std::static_pointer_cast<ChatRoom>(L_GET_CPP_PTR_FROM_C_OBJECT(cr)));
		}
	}
}

std::shared_ptr<ChatRoom> IosPlatformHelpers::processPushNotificationChatRoom(const string &crAddr) {
	ms_message("[push] processPushNotificationInvite. looking for chatroom %s", mChatRoomAddr.c_str());
	mChatRoomAddr = crAddr;
	mChatRoomInvite = nullptr;

	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_chat_room_state_changed(cbs, on_push_notification_chat_room_invite_received);
	linphone_core_add_callbacks(getCore()->getCCore(), cbs);

	if (linphone_core_get_global_state(getCore()->getCCore()) != LinphoneGlobalOn && linphone_core_start(getCore()->getCCore()) != 0) {
		linphone_core_cbs_unref(cbs);
		return nullptr;
	}
	ms_message("[push] core started");

	reinitTimer();
	uint64_t iterationTimer = ms_get_cur_time_ms();

	/* if the chatroom is received, iterate for 2 sec otherwise iterate mMaxIterationTimeMs seconds*/
	while ((ms_get_cur_time_ms() - iterationTimer < mMaxIterationTimeMs && !mChatRoomInvite) || (mChatRoomInvite && ms_get_cur_time_ms() - mTimer < 2000)) {
		ms_message("[push] wait chatRoom");
		linphone_core_iterate(getCore()->getCCore());
		ms_usleep(50000);
	}

	linphone_core_cbs_unref(cbs);
	return mChatRoomInvite;
}

// -----------------------------------------------------------------------------
// shared core
// -----------------------------------------------------------------------------

static void unlock_shared_core_if_needed(CFRunLoopTimerRef timer, void *info) {
	IosPlatformHelpers *platform_helper = (IosPlatformHelpers *) info;
	platform_helper->unlockSharedCoreIfNeeded();
}

void IosPlatformHelpers::setupSharedCore(struct _LpConfig *config) {
	ms_message("[SHARED] setupSharedCore");
	mAppGroupId = linphone_config_get_string(config, "shared_core", "app_group_id", "");

	if (isCoreShared()) {
		/* by default iterate for 25 sec max. After 30 sec the iOS app extension can be killed by the system */
		mMaxIterationTimeMs = (uint64_t)linphone_config_get_int(config, "shared_core", "max_iteration_time_ms", 25000);

		CFRunLoopTimerContext timerContext;

		timerContext.version = 0;
		timerContext.info = this;
		timerContext.retain = NULL;
		timerContext.release = NULL;
		timerContext.copyDescription = NULL;

		CFTimeInterval interval = 10.0f; // 10 sec
		mUnlockTimer = CFRunLoopTimerCreate (NULL,
			CFAbsoluteTimeGetCurrent() + interval,
			interval,
			0,
			0,
			unlock_shared_core_if_needed,
			&timerContext
		);

		/* use an iOS timer instead of belle sip timer to unlock
		potentially stuck processes that won't be able to call iterate() */
		CFRunLoopAddTimer (CFRunLoopGetCurrent(), mUnlockTimer, kCFRunLoopCommonModes);
		ms_message("[SHARED] launch timer");
		unlockSharedCoreIfNeeded();
	}
}

bool IosPlatformHelpers::isCoreShared() {
	return !mAppGroupId.empty();
}

bool IosPlatformHelpers::canCoreStart() {
	ms_message("[SHARED] canCoreStart");
	if (!isCoreShared()) return true;

	if (getCore()->getCCore()->is_main_core) {
		return canMainCoreStart();
	} else {
		return canExecutorCoreStart();
	}
}

bool IosPlatformHelpers::isSharedCoreStarted() {
    NSUserDefaults *defaults = [[NSUserDefaults alloc] initWithSuiteName:@(mAppGroupId.c_str())];
    NSInteger state = [defaults integerForKey:@ACTIVE_SHARED_CORE];
	ms_message("[SHARED] get shared core state : %s", sharedStateToString((int)state).c_str());
	[defaults release];
	if (state == SharedCoreState::noCoreStarted) {
		return false;
	}
	return true;
}

SharedCoreState IosPlatformHelpers::getSharedCoreState() {
	NSUserDefaults *defaults = [[NSUserDefaults alloc] initWithSuiteName:@(mAppGroupId.c_str())];
    NSInteger state = [defaults integerForKey:@ACTIVE_SHARED_CORE];
	[defaults release];
	return (SharedCoreState) state;
}

/**
 * Set to false in onLinphoneCoreStop() (called in linphone_core_stop)
 */
void IosPlatformHelpers::setSharedCoreState(SharedCoreState sharedCoreState) {
	ms_message("[SHARED] setSharedCoreState state: %s", sharedStateToString(sharedCoreState).c_str());
    NSUserDefaults *defaults = [[NSUserDefaults alloc] initWithSuiteName:@(mAppGroupId.c_str())];
    [defaults setInteger:sharedCoreState forKey:@ACTIVE_SHARED_CORE];
	[defaults release];
}

void IosPlatformHelpers::resetSharedCoreState() {
	setSharedCoreState(SharedCoreState::noCoreStarted);
}

void IosPlatformHelpers::resetSharedCoreLastUpdateTime() {
	ms_debug("[SHARED] resetSharedCoreLastUpdateTime");
    NSUserDefaults *defaults = [[NSUserDefaults alloc] initWithSuiteName:@(mAppGroupId.c_str())];
    [defaults setInteger:(NSInteger)ms_get_cur_time_ms() forKey:@LAST_UPDATE_TIME_SHARED_CORE];
	[defaults release];
}

NSInteger IosPlatformHelpers::getSharedCoreLastUpdateTime() {
    NSUserDefaults *defaults = [[NSUserDefaults alloc] initWithSuiteName:@(mAppGroupId.c_str())];
    NSInteger lastUpdateTime = [defaults integerForKey:@LAST_UPDATE_TIME_SHARED_CORE];
	[defaults release];
	return lastUpdateTime;
}

void IosPlatformHelpers::unlockSharedCoreIfNeeded() {
	if ((NSInteger)ms_get_cur_time_ms() - getSharedCoreLastUpdateTime() > 30000) {
		ms_message("[SHARED] unlockSharedCoreIfNeeded : no update during last 30 sec");
		resetSharedCoreState();
		IosPlatformHelpers::callIdListMutex.unlock();
		IosPlatformHelpers::executorCoreMutex.unlock();
	}
	resetSharedCoreLastUpdateTime();
}

// we need to reload the config from file at each start tp get the changes made by the other cores
void IosPlatformHelpers::reloadConfig() {
	// if we just created the core, we don't need to reload the config
	if (getCore()->getCCore()->has_already_started_once) {
		linphone_config_reload(getCore()->getCCore()->config);
	} else {
		getCore()->getCCore()->has_already_started_once = true;
	}

}

// -----------------------------------------------------------------------------
// shared core : executor
// -----------------------------------------------------------------------------

bool IosPlatformHelpers::canExecutorCoreStart() {
	ms_message("[SHARED] canExecutorCoreStart");
	if (isSharedCoreStarted()) {
		ms_message("[SHARED] executor core can't start, another one is started");
		return false;
	}

	subscribeToMainCoreNotifs();
	setSharedCoreState(SharedCoreState::executorCoreStarted);
	resetSharedCoreLastUpdateTime();
	reloadConfig();
	return true;
}


void IosPlatformHelpers::subscribeToMainCoreNotifs() {
	ms_message("[SHARED] subscribeToMainCoreNotifs");
   	CFNotificationCenterRef notification = CFNotificationCenterGetDarwinNotifyCenter();
   	CFNotificationCenterAddObserver(notification, (__bridge const void *)(this), on_core_must_stop, CFSTR(ACTIVE_SHARED_CORE), NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
}

void on_core_must_stop(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo) {
   	ms_message("[SHARED] on_core_must_stop");
	if (observer) {
		IosPlatformHelpers *myself = (IosPlatformHelpers *) observer;
		myself->onCoreMustStop();
	}
}

void IosPlatformHelpers::onCoreMustStop() {
	ms_message("[SHARED] onCoreMustStop");
	linphone_shared_core_must_stop(getCore()->getCCore());
}

// -----------------------------------------------------------------------------
// shared core : main
// -----------------------------------------------------------------------------

bool IosPlatformHelpers::canMainCoreStart() {
	ms_message("[SHARED] canMainCoreStart");
	if (isSharedCoreStarted()) {
		try {
			stopSharedCores();
		} catch (std::exception &e) {
			ms_error("[SHARED] %s", e.what());
			return false;
		}
	}
	setSharedCoreState(SharedCoreState::mainCoreStarted);
	resetSharedCoreLastUpdateTime();
	reloadConfig();
	return true;
}

void IosPlatformHelpers::stopSharedCores() {
    ms_message("[SHARED] stopping shared cores");
	CFNotificationCenterRef notification = CFNotificationCenterGetDarwinNotifyCenter();
    CFNotificationCenterPostNotification(notification, CFSTR(ACTIVE_SHARED_CORE), NULL, NULL, YES);

    for(int i=0; isSharedCoreStarted() && i<30; i++) {
        ms_message("[SHARED] wait");
        usleep(100000);
    }
	if (isSharedCoreStarted()) {
		setSharedCoreState(SharedCoreState::noCoreStarted);
	}
    ms_message("[SHARED] shared cores stopped");
}

// -----------------------------------------------------------------------------

PlatformHelpers *createIosPlatformHelpers(std::shared_ptr<LinphonePrivate::Core> core, void *systemContext) {
	return new IosPlatformHelpers(core, systemContext);
}

LINPHONE_END_NAMESPACE

#endif
