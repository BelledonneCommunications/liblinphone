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

#include <jni.h>

#include <bctoolbox/defs.h>

#include "c-wrapper/c-wrapper.h"
#include "conference/participant-device.h"
#include "core/paths/paths.h"
#include "logger/logger.h"
#include "platform-helpers.h"
#include "signal-information/signal-information.h"

// TODO: Remove me later.
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

class AndroidPlatformHelpers : public GenericPlatformHelpers {
public:
	AndroidPlatformHelpers(std::shared_ptr<LinphonePrivate::Core> core, void *systemContext);
	~AndroidPlatformHelpers();

	void acquireWifiLock() override;
	void releaseWifiLock() override;
	void acquireMcastLock() override;
	void releaseMcastLock() override;
	void acquireCpuLock() override;
	void releaseCpuLock() override;

	string getConfigPath() const override;
	string getDataPath() const override;
	string getDataResource(const string &filename) const override;
	string getImageResource(const string &filename) const override;
	string getRingResource(const string &filename) const override;
	string getSoundResource(const string &filename) const override;
	void *getPathContext() override;

	void setVideoPreviewWindow(void *windowId) override;
	void setVideoWindow(void *windowId) override;
	void setParticipantDeviceVideoWindow(LinphoneParticipantDevice *participantDevice, void *windowId) override;
	void resizeVideoPreview(int width, int height) override;

	bool isNetworkReachable() override;
	void updateNetworkReachability() override;
	bool isActiveNetworkWifiOnlyCompliant() const override;
	void onWifiOnlyEnabled(bool enabled) override;
	void setDnsServers() override;
	void updateDnsServers();
	void setNetworkReachable(bool reachable) override;
	void setHttpProxy(const string &host, int port) override;
	void startPushService() override;
	void stopPushService() override;
	void startFileTransferService() override;
	void stopFileTransferService() override;

	void onLinphoneCoreStart(bool monitoringEnabled) override;
	void onLinphoneCoreStop() override;

	void startAudioForEchoTestOrCalibration() override;
	void stopAudioForEchoTestOrCalibration() override;
	void routeAudioToSpeaker() override;
	void restorePreviousAudioRoute() override;

	void enableAutoIterate(bool autoIterateEnabled) override;

	void onRecordingStarted() const override;
	void onRecordingPaused() const override;
	bool isRingingAllowed() const override;
	void stopRinging() const override;

	void setDeviceRotation(int orientation) const override;

	void _setPreviewVideoWindow(jobject window);
	void _setVideoWindow(jobject window);
	void _setParticipantDeviceVideoWindow(LinphoneParticipantDevice *participantDevice, jobject windowId);
	string getDownloadPath() override;

	void disableAudioRouteChanges(bool disable);

private:
	int callVoidMethod(jmethodID id);
	static jmethodID getMethodId(JNIEnv *env, jclass klass, const char *method, const char *signature);
	string getNativeLibraryDir();
	void createCoreManager(std::shared_ptr<LinphonePrivate::Core> core, void *systemContext);
	void destroyCoreManager();

	jobject mJavaHelper = nullptr;
	jobject mSystemContext = nullptr;
	jobject mJavaCoreManager = nullptr;
	jobject mPreviewVideoWindow = nullptr;
	jobject mVideoWindow = nullptr;
	unordered_map<long, jobject> mParticipantDeviceVideoWindows;

	// PlatformHelper methods
	jmethodID mWifiLockAcquireId = nullptr;
	jmethodID mWifiLockReleaseId = nullptr;
	jmethodID mMcastLockAcquireId = nullptr;
	jmethodID mMcastLockReleaseId = nullptr;
	jmethodID mCpuLockAcquireId = nullptr;
	jmethodID mCpuLockReleaseId = nullptr;
	jmethodID mGetDnsServersId = nullptr;
	jmethodID mGetPowerManagerId = nullptr;
	jmethodID mGetNativeLibraryDirId = nullptr;
	jmethodID mSetNativeVideoWindowId = nullptr;
	jmethodID mSetNativePreviewVideoWindowId = nullptr;
	jmethodID mSetParticipantDeviceNativeVideoWindowId = nullptr;
	jmethodID mResizeVideoPreviewId = nullptr;
	jmethodID mOnLinphoneCoreStartId = nullptr;
	jmethodID mOnLinphoneCoreStopId = nullptr;
	jmethodID mOnWifiOnlyEnabledId = nullptr;
	jmethodID mIsActiveNetworkWifiOnlyCompliantId = nullptr;
	jmethodID mUpdateNetworkReachabilityId = nullptr;
	jmethodID mDisableAudioRouteChangesId = nullptr;
	jmethodID mStartPushService = nullptr;
	jmethodID mStopPushService = nullptr;
	jmethodID mStartFileTransferService = nullptr;
	jmethodID mStopFileTransferService = nullptr;

	// CoreManager methods
	jmethodID mCoreManagerDestroyId = nullptr;
	jmethodID mCoreManagerOnLinphoneCoreStartId = nullptr;
	jmethodID mCoreManagerOnLinphoneCoreStopId = nullptr;
	jmethodID mStartAudioForEchoTestOrCalibrationId = nullptr;
	jmethodID mStopAudioForEchoTestOrCalibrationId = nullptr;
	jmethodID mRouteAudioToSpeakerId = nullptr;
	jmethodID mRestorePreviousAudioRouteId = nullptr;
	jmethodID mStartAutoIterateId = nullptr;
	jmethodID mStopAutoIterateId = nullptr;
	jmethodID mSetAudioManagerCommunicationMode = nullptr;
	jmethodID mSetAudioManagerNormalMode = nullptr;
	jmethodID mIsRingingAllowed = nullptr;
	jmethodID mStopRingingId = nullptr;

	bool mNetworkReachable = false;
	string mDownloadPath = "";
	bctbx_list_t *mDnsServersList = nullptr;
};

static const char *GetStringUTFChars(JNIEnv *env, jstring string) {
	const char *cstring = string ? env->GetStringUTFChars(string, nullptr) : nullptr;
	return cstring;
}

static void ReleaseStringUTFChars(JNIEnv *env, jstring string, const char *cstring) {
	if (string) env->ReleaseStringUTFChars(string, cstring);
}

jmethodID AndroidPlatformHelpers::getMethodId(JNIEnv *env, jclass klass, const char *method, const char *signature) {
	jmethodID id = env->GetMethodID(klass, method, signature);
	if (id == 0)
		lFatal() << "[Android Platform Helper] Could not find java method: `" << method << ", " << signature << "`.";
	return id;
}

// -----------------------------------------------------------------------------

extern "C" jobject getCore(JNIEnv *env, LinphoneCore *cptr, bool_t takeref, bool_t is_const);

void AndroidPlatformHelpers::createCoreManager(std::shared_ptr<LinphonePrivate::Core> core, void *systemContext) {
	JNIEnv *env = ms_get_jni_env();
	jclass klass = env->FindClass("org/linphone/core/tools/service/CoreManager");
	if (!klass) {
		lError() << "[Android Platform Helper] Could not find java CoreManager class.";
		return;
	}

	jmethodID ctor = env->GetMethodID(klass, "<init>", "(Ljava/lang/Object;Lorg/linphone/core/Core;)V");
	LinphoneCore *lc = L_GET_C_BACK_PTR(core);
	jobject javaCore = ::LinphonePrivate::getCore(env, lc, TRUE, FALSE);
	mJavaCoreManager = env->NewObject(klass, ctor, (jobject)systemContext, (jobject)javaCore);
	if (!mJavaCoreManager) {
		lError() << "[Android Platform Helper] Could not instanciate CoreManager object.";
		return;
	}
	mJavaCoreManager = (jobject)env->NewGlobalRef(mJavaCoreManager);

	mCoreManagerDestroyId = getMethodId(env, klass, "destroy", "()V");
	mCoreManagerOnLinphoneCoreStartId = getMethodId(env, klass, "onLinphoneCoreStart", "()V");
	mCoreManagerOnLinphoneCoreStopId = getMethodId(env, klass, "onLinphoneCoreStop", "()V");

	mStartAudioForEchoTestOrCalibrationId = getMethodId(env, klass, "startAudioForEchoTestOrCalibration", "()V");
	mStopAudioForEchoTestOrCalibrationId = getMethodId(env, klass, "stopAudioForEchoTestOrCalibration", "()V");
	mRouteAudioToSpeakerId = getMethodId(env, klass, "routeAudioToSpeaker", "()V");
	mRestorePreviousAudioRouteId = getMethodId(env, klass, "restorePreviousAudioRoute", "()V");
	mStartAutoIterateId = getMethodId(env, klass, "startAutoIterate", "()V");
	mStopAutoIterateId = getMethodId(env, klass, "stopAutoIterate", "()V");
	mSetAudioManagerCommunicationMode = getMethodId(env, klass, "setAudioManagerInCommunicationMode", "()V");
	mSetAudioManagerNormalMode = getMethodId(env, klass, "setAudioManagerInNormalMode", "()V");
	mIsRingingAllowed = getMethodId(env, klass, "isRingingAllowed", "()Z");
	mStopRingingId = getMethodId(env, klass, "stopRinging", "()V");

	lInfo() << "[Android Platform Helper] CoreManager is fully initialised.";
}

void AndroidPlatformHelpers::destroyCoreManager() {
	if (mJavaCoreManager) {
		JNIEnv *env = ms_get_jni_env();
		env->CallVoidMethod(mJavaCoreManager, mCoreManagerDestroyId);
		env->DeleteGlobalRef(mJavaCoreManager);
		mJavaCoreManager = nullptr;
		lInfo() << "[Android Platform Helper] CoreManager has been destroyed.";
	}
}

// -----------------------------------------------------------------------------

AndroidPlatformHelpers::AndroidPlatformHelpers(std::shared_ptr<LinphonePrivate::Core> core, void *systemContext)
    : GenericPlatformHelpers(core) {
	createCoreManager(core, systemContext);

	JNIEnv *env = ms_get_jni_env();
	jclass klass = env->FindClass("org/linphone/core/tools/AndroidPlatformHelper");
	if (!klass) lFatal() << "[Android Platform Helper] Could not find java AndroidPlatformHelper class.";

	jmethodID ctor = env->GetMethodID(klass, "<init>", "(JLjava/lang/Object;Z)V");
	mJavaHelper = env->NewObject(klass, ctor, (jlong)this, (jobject)systemContext,
	                             (jboolean)linphone_core_wifi_only_enabled(getCore()->getCCore()));
	if (!mJavaHelper) {
		lError() << "[Android Platform Helper] Could not instanciate AndroidPlatformHelper object.";
		return;
	}
	mJavaHelper = (jobject)env->NewGlobalRef(mJavaHelper);
	mSystemContext = (jobject)systemContext;

	mWifiLockAcquireId = getMethodId(env, klass, "acquireWifiLock", "()V");
	mWifiLockReleaseId = getMethodId(env, klass, "releaseWifiLock", "()V");
	mMcastLockAcquireId = getMethodId(env, klass, "acquireMcastLock", "()V");
	mMcastLockReleaseId = getMethodId(env, klass, "releaseMcastLock", "()V");
	mCpuLockAcquireId = getMethodId(env, klass, "acquireCpuLock", "()V");
	mCpuLockReleaseId = getMethodId(env, klass, "releaseCpuLock", "()V");
	mGetDnsServersId = getMethodId(env, klass, "getDnsServers", "()[Ljava/lang/String;");
	mGetPowerManagerId = getMethodId(env, klass, "getPowerManager", "()Ljava/lang/Object;");
	mGetNativeLibraryDirId = getMethodId(env, klass, "getNativeLibraryDir", "()Ljava/lang/String;");
	mSetNativeVideoWindowId = getMethodId(env, klass, "setVideoRenderingView", "(Ljava/lang/Object;)V");
	mSetNativePreviewVideoWindowId = getMethodId(env, klass, "setVideoPreviewView", "(Ljava/lang/Object;)V");
	mSetParticipantDeviceNativeVideoWindowId =
	    getMethodId(env, klass, "setParticipantDeviceVideoRenderingView", "(JLjava/lang/Object;)V");
	mResizeVideoPreviewId = getMethodId(env, klass, "resizeVideoPreview", "(II)V");
	mOnLinphoneCoreStartId = getMethodId(env, klass, "onLinphoneCoreStart", "(Z)V");
	mOnLinphoneCoreStopId = getMethodId(env, klass, "onLinphoneCoreStop", "()V");
	mOnWifiOnlyEnabledId = getMethodId(env, klass, "onWifiOnlyEnabled", "(Z)V");
	mIsActiveNetworkWifiOnlyCompliantId = getMethodId(env, klass, "isActiveNetworkWifiOnlyCompliant", "()Z");
	mUpdateNetworkReachabilityId = getMethodId(env, klass, "updateNetworkReachability", "()V");
	mDisableAudioRouteChangesId = getMethodId(env, klass, "disableAudioRouteChanges", "(Z)V");
	mStartPushService = getMethodId(env, klass, "startPushService", "()V");
	mStopPushService = getMethodId(env, klass, "stopPushService", "()V");
	mStartFileTransferService = getMethodId(env, klass, "startFileTransferService", "()V");
	mStopFileTransferService = getMethodId(env, klass, "stopFileTransferService", "()V");

	jobject pm = env->CallObjectMethod(mJavaHelper, mGetPowerManagerId);
	belle_sip_wake_lock_init(env, pm);

	linphone_factory_set_top_resources_dir(linphone_factory_get(), getDataPath().append("share").c_str());
	linphone_factory_set_msplugins_dir(linphone_factory_get(), getNativeLibraryDir().c_str());
	lInfo() << "[Android Platform Helper] AndroidPlatformHelper is fully initialised.";

	mPreviewVideoWindow = nullptr;
	mVideoWindow = nullptr;
	mNetworkReachable = false;

	LinphoneConfig *config = linphone_core_get_config(getCore()->getCCore());
	if (linphone_config_get_bool(config, "sound", "android_disable_audio_route_changes", FALSE) == TRUE) {
		disableAudioRouteChanges(true);
	}
}

AndroidPlatformHelpers::~AndroidPlatformHelpers() {
	destroyCoreManager();
	if (mJavaHelper) {
		JNIEnv *env = ms_get_jni_env();
		belle_sip_wake_lock_uninit(env);
		env->DeleteGlobalRef(mJavaHelper);
		mJavaHelper = nullptr;
		if (mDnsServersList) {
			bctbx_list_free_with_data(mDnsServersList, ms_free);
			mDnsServersList = nullptr;
		}
	}
	lInfo() << "[Android Platform Helper] AndroidPlatformHelper has been destroyed.";
}

// -----------------------------------------------------------------------------

void AndroidPlatformHelpers::acquireWifiLock() {
	callVoidMethod(mWifiLockAcquireId);
}

void AndroidPlatformHelpers::releaseWifiLock() {
	callVoidMethod(mWifiLockReleaseId);
}

void AndroidPlatformHelpers::acquireMcastLock() {
	callVoidMethod(mMcastLockAcquireId);
}

void AndroidPlatformHelpers::releaseMcastLock() {
	callVoidMethod(mMcastLockReleaseId);
}

void AndroidPlatformHelpers::acquireCpuLock() {
	callVoidMethod(mCpuLockAcquireId);
}

void AndroidPlatformHelpers::releaseCpuLock() {
	callVoidMethod(mCpuLockReleaseId);
}

// -----------------------------------------------------------------------------

string AndroidPlatformHelpers::getConfigPath() const {
	return Paths::getPath(Paths::Config, mSystemContext);
}

string AndroidPlatformHelpers::getDownloadPath() {
	if (mDownloadPath.empty()) {
		mDownloadPath = Paths::getPath(Paths::Download, mSystemContext);
	}
	return mDownloadPath;
}

string AndroidPlatformHelpers::getDataPath() const {
	return Paths::getPath(Paths::Data, mSystemContext);
}

string AndroidPlatformHelpers::getDataResource(const string &filename) const {
	return getFilePath(linphone_factory_get_data_resources_dir(linphone_factory_get()), filename);
}

string AndroidPlatformHelpers::getImageResource(const string &filename) const {
	return getFilePath(linphone_factory_get_image_resources_dir(linphone_factory_get()), filename);
}

string AndroidPlatformHelpers::getRingResource(const string &filename) const {
	return getFilePath(linphone_factory_get_ring_resources_dir(linphone_factory_get()), filename);
}

string AndroidPlatformHelpers::getSoundResource(const string &filename) const {
	return getFilePath(linphone_factory_get_sound_resources_dir(linphone_factory_get()), filename);
}

void *AndroidPlatformHelpers::getPathContext() {
	return mSystemContext;
}

// -----------------------------------------------------------------------------

void AndroidPlatformHelpers::setVideoPreviewWindow(void *windowId) {
	JNIEnv *env = ms_get_jni_env();
	if (env && mJavaHelper) {
		string displayFilter = L_C_TO_STRING(linphone_core_get_video_display_filter(getCore()->getCCore()));
		if ((displayFilter.empty() || displayFilter == "MSAndroidTextureDisplay")) {
			env->CallVoidMethod(mJavaHelper, mSetNativePreviewVideoWindowId, (jobject)windowId);
		} else {
			_setPreviewVideoWindow((jobject)windowId);
		}
	}
}

void AndroidPlatformHelpers::setVideoWindow(void *windowId) {
	JNIEnv *env = ms_get_jni_env();
	if (env && mJavaHelper) {
		string displayFilter = L_C_TO_STRING(linphone_core_get_video_display_filter(getCore()->getCCore()));
		if ((displayFilter.empty() || displayFilter == "MSAndroidTextureDisplay")) {
			env->CallVoidMethod(mJavaHelper, mSetNativeVideoWindowId, (jobject)windowId);
		} else {
			_setVideoWindow((jobject)windowId);
		}
	}
}

void AndroidPlatformHelpers::setParticipantDeviceVideoWindow(LinphoneParticipantDevice *participantDevice,
                                                             void *windowId) {
	JNIEnv *env = ms_get_jni_env();
	if (env && mJavaHelper) {
		string displayFilter = L_C_TO_STRING(linphone_core_get_video_display_filter(getCore()->getCCore()));
		if ((displayFilter.empty() || displayFilter == "MSAndroidTextureDisplay")) {
			lInfo() << "[Android Platform Helper] Sending window ID [" << windowId
			        << "] through Java platform helper for participant device [" << participantDevice << "]";
			env->CallVoidMethod(mJavaHelper, mSetParticipantDeviceNativeVideoWindowId, (jlong)participantDevice,
			                    (jobject)windowId);
		} else {
			lInfo() << "[Android Platform Helper] Directly using window ID [" << windowId
			        << "] without going through Java platform helper for participant device [" << participantDevice
			        << "]";
			_setParticipantDeviceVideoWindow(participantDevice, (jobject)windowId);
		}
	}
}

void AndroidPlatformHelpers::resizeVideoPreview(int width, int height) {
	JNIEnv *env = ms_get_jni_env();
	if (env && mJavaHelper) {
		string displayFilter = L_C_TO_STRING(linphone_core_get_video_display_filter(getCore()->getCCore()));
		if ((displayFilter.empty() || displayFilter == "MSAndroidTextureDisplay")) {
			env->CallVoidMethod(mJavaHelper, mResizeVideoPreviewId, width, height);
		}
	}
}

// -----------------------------------------------------------------------------

bool AndroidPlatformHelpers::isNetworkReachable() {
	return mNetworkReachable;
}

void AndroidPlatformHelpers::updateNetworkReachability() {
	JNIEnv *env = ms_get_jni_env();
	if (env && mJavaHelper) {
		env->CallVoidMethod(mJavaHelper, mUpdateNetworkReachabilityId);
	}
}

bool AndroidPlatformHelpers::isActiveNetworkWifiOnlyCompliant() const {
	JNIEnv *env = ms_get_jni_env();
	if (env && mJavaHelper) {
		return env->CallBooleanMethod(mJavaHelper, mIsActiveNetworkWifiOnlyCompliantId);
	}
	return false;
}

void AndroidPlatformHelpers::onWifiOnlyEnabled(bool enabled) {
	JNIEnv *env = ms_get_jni_env();
	if (env && mJavaHelper) {
		env->CallVoidMethod(mJavaHelper, mOnWifiOnlyEnabledId, (jboolean)enabled);
	}
}

void AndroidPlatformHelpers::setHttpProxy(const string &host, int port) {
	linphone_core_set_http_proxy_host(getCore()->getCCore(), host.c_str());
	linphone_core_set_http_proxy_port(getCore()->getCCore(), port);
}

void AndroidPlatformHelpers::setDnsServers() {
	if (linphone_core_get_dns_set_by_app(getCore()->getCCore())) {
		lWarning() << "[Android Platform Helper] DNS servers have been overriden by app";
		return;
	}

	if (!mDnsServersList) {
		updateDnsServers();
	}

	if (mDnsServersList) {
		linphone_core_set_dns_servers(getCore()->getCCore(), mDnsServersList);
	} else {
		lError() << "[Android Platform Helper] No DNS server available!";
	}
}

void AndroidPlatformHelpers::updateDnsServers() {
	if (!mJavaHelper) {
		lError() << "[Android Platform Helper] mJavaHelper is null.";
		return;
	}

	JNIEnv *env = ms_get_jni_env();
	if (env) {
		jobjectArray jservers = (jobjectArray)env->CallObjectMethod(mJavaHelper, mGetDnsServersId);
		bctbx_list_t *dns_servers_list = nullptr;
		if (env->ExceptionCheck()) {
			env->ExceptionClear();
			lError() << "[Android Platform Helper] setDnsServers() exception.";
			return;
		}

		if (jservers != nullptr) {
			int count = env->GetArrayLength(jservers);
			ostringstream ostr;

			for (int i = 0; i < count; i++) {
				jstring jserver = (jstring)env->GetObjectArrayElement(jservers, i);
				const char *str = GetStringUTFChars(env, jserver);
				if (str) {
					if (i != 0) ostr << ", ";
					ostr << str;
					dns_servers_list = bctbx_list_append(dns_servers_list, ms_strdup(str));
					ReleaseStringUTFChars(env, jserver, str);
				}
			}

			lInfo() << "[Android Platform Helper] Known DNS servers are: " << ostr.str();
		} else {
			lError() << "[Android Platform Helper] setDnsServers() failed to get DNS servers list";
			return;
		}

		if (mDnsServersList) {
			bctbx_list_free_with_data(mDnsServersList, ms_free);
			mDnsServersList = nullptr;
		}
		mDnsServersList = dns_servers_list;
	}
}

void AndroidPlatformHelpers::setNetworkReachable(bool reachable) {
	mNetworkReachable = reachable;
	linphone_core_set_network_reachable_internal(getCore()->getCCore(), reachable ? 1 : 0);
}

void AndroidPlatformHelpers::startPushService() {
	JNIEnv *env = ms_get_jni_env();
	if (env && mJavaHelper) {
		env->CallVoidMethod(mJavaHelper, mStartPushService);
	}
}

void AndroidPlatformHelpers::stopPushService() {
	JNIEnv *env = ms_get_jni_env();
	if (env && mJavaHelper) {
		env->CallVoidMethod(mJavaHelper, mStopPushService);
	}
}

void AndroidPlatformHelpers::startFileTransferService() {
	JNIEnv *env = ms_get_jni_env();
	if (env && mJavaHelper) {
		env->CallVoidMethod(mJavaHelper, mStartFileTransferService);
	}
}

void AndroidPlatformHelpers::stopFileTransferService() {
	JNIEnv *env = ms_get_jni_env();
	if (env && mJavaHelper) {
		env->CallVoidMethod(mJavaHelper, mStopFileTransferService);
	}
}

// -----------------------------------------------------------------------------

void AndroidPlatformHelpers::onLinphoneCoreStart(bool monitoringEnabled) {
	JNIEnv *env = ms_get_jni_env();
	if (env) {
		if (mJavaCoreManager) {
			env->CallVoidMethod(mJavaCoreManager, mCoreManagerOnLinphoneCoreStartId);
		}
		if (mJavaHelper) {
			env->CallVoidMethod(mJavaHelper, mOnLinphoneCoreStartId, (jboolean)monitoringEnabled);
		}
	}
}

void AndroidPlatformHelpers::onLinphoneCoreStop() {
	JNIEnv *env = ms_get_jni_env();
	if (env) {
		if (mJavaCoreManager) {
			env->CallVoidMethod(mJavaCoreManager, mCoreManagerOnLinphoneCoreStopId);
		}
		if (mJavaHelper) {
			env->CallVoidMethod(mJavaHelper, mOnLinphoneCoreStopId);
		}
	}
}

void AndroidPlatformHelpers::startAudioForEchoTestOrCalibration() {
	JNIEnv *env = ms_get_jni_env();
	if (env) {
		if (mJavaCoreManager) {
			env->CallVoidMethod(mJavaCoreManager, mStartAudioForEchoTestOrCalibrationId);
		}
	}
}

void AndroidPlatformHelpers::stopAudioForEchoTestOrCalibration() {
	JNIEnv *env = ms_get_jni_env();
	if (env) {
		if (mJavaCoreManager) {
			env->CallVoidMethod(mJavaCoreManager, mStopAudioForEchoTestOrCalibrationId);
		}
	}
}

void AndroidPlatformHelpers::routeAudioToSpeaker() {
	JNIEnv *env = ms_get_jni_env();
	if (env) {
		if (mJavaCoreManager) {
			env->CallVoidMethod(mJavaCoreManager, mRouteAudioToSpeakerId);
		}
	}
}

void AndroidPlatformHelpers::restorePreviousAudioRoute() {
	JNIEnv *env = ms_get_jni_env();
	if (env) {
		if (mJavaCoreManager) {
			env->CallVoidMethod(mJavaCoreManager, mRestorePreviousAudioRouteId);
		}
	}
}

void AndroidPlatformHelpers::enableAutoIterate(bool autoIterateEnabled) {
	JNIEnv *env = ms_get_jni_env();
	if (env) {
		if (mJavaCoreManager) {
			if (autoIterateEnabled) {
				env->CallVoidMethod(mJavaCoreManager, mStartAutoIterateId);
			} else {
				env->CallVoidMethod(mJavaCoreManager, mStopAutoIterateId);
			}
		}
	}
}

void AndroidPlatformHelpers::onRecordingStarted() const {
}

void AndroidPlatformHelpers::onRecordingPaused() const {
}

bool AndroidPlatformHelpers::isRingingAllowed() const {
	JNIEnv *env = ms_get_jni_env();
	if (env) {
		if (mJavaCoreManager) {
			return env->CallBooleanMethod(mJavaCoreManager, mIsRingingAllowed);
		}
	}
	return false;
}

void AndroidPlatformHelpers::stopRinging() const {
	LinphoneCore *lc = getCore()->getCCore();
	if (linphone_core_is_native_ringing_enabled(lc) == TRUE) {
		JNIEnv *env = ms_get_jni_env();
		if (env) {
			if (mJavaCoreManager) {
				env->CallVoidMethod(mJavaCoreManager, mStopRingingId);
			}
		}
	}
}

void AndroidPlatformHelpers::setDeviceRotation(BCTBX_UNUSED(int orientation)) const {
}

// -----------------------------------------------------------------------------

void AndroidPlatformHelpers::_setPreviewVideoWindow(jobject window) {
	JNIEnv *env = ms_get_jni_env();
	LinphoneCore *lc = getCore()->getCCore();
	if (window != nullptr && window != mPreviewVideoWindow) {
		if (mPreviewVideoWindow != nullptr) {
			env->DeleteGlobalRef(mPreviewVideoWindow);
		}
		mPreviewVideoWindow = env->NewGlobalRef(window);
	} else if (window == nullptr && mPreviewVideoWindow != nullptr) {
		env->DeleteGlobalRef(mPreviewVideoWindow);
		mPreviewVideoWindow = nullptr;
	}
	_linphone_core_set_native_preview_window_id(lc, (void *)mPreviewVideoWindow);
}

void AndroidPlatformHelpers::_setVideoWindow(jobject window) {
	JNIEnv *env = ms_get_jni_env();
	LinphoneCore *lc = getCore()->getCCore();
	if (window != nullptr && window != mVideoWindow) {
		if (mVideoWindow != nullptr) {
			env->DeleteGlobalRef(mVideoWindow);
		}
		mVideoWindow = env->NewGlobalRef(window);
	} else if (window == nullptr && mVideoWindow != nullptr) {
		env->DeleteGlobalRef(mVideoWindow);
		mVideoWindow = nullptr;
	}
	_linphone_core_set_native_video_window_id(lc, (void *)mVideoWindow);
}

void AndroidPlatformHelpers::_setParticipantDeviceVideoWindow(LinphoneParticipantDevice *participantDevice,
                                                              jobject window) {
	JNIEnv *env = ms_get_jni_env();

	long key = (long)participantDevice;
	auto it = mParticipantDeviceVideoWindows.find(key);
	jobject currentWindow = it == mParticipantDeviceVideoWindows.cend() ? nullptr : it->second;

	if (window != nullptr && window != currentWindow) {
		lInfo() << "[Android Platform Helper] New not null window ID [" << window
		        << "] for participant device, previously was [" << currentWindow << "]";
		if (currentWindow != nullptr) {
			env->DeleteGlobalRef(currentWindow);
		}
		currentWindow = env->NewGlobalRef(window);
		mParticipantDeviceVideoWindows[key] = currentWindow;
	} else if (window == nullptr && currentWindow != nullptr) {
		lInfo() << "[Android Platform Helper] New null window ID for participant device, previously was ["
		        << currentWindow << "]";
		env->DeleteGlobalRef(currentWindow);
		currentWindow = nullptr;
		mParticipantDeviceVideoWindows[key] = nullptr;
	}

	lInfo() << "[Android Platform Helper] New window ID is [" << currentWindow << "]";
	LinphonePrivate::ParticipantDevice::toCpp(participantDevice)->setWindowId((void *)currentWindow);
}

void AndroidPlatformHelpers::disableAudioRouteChanges(bool disable) {
	JNIEnv *env = ms_get_jni_env();
	if (env && mJavaHelper) {
		env->CallVoidMethod(mJavaHelper, mDisableAudioRouteChangesId, disable);
	}
}

// -----------------------------------------------------------------------------

int AndroidPlatformHelpers::callVoidMethod(jmethodID id) {
	JNIEnv *env = ms_get_jni_env();
	if (env && mJavaHelper) {
		env->CallVoidMethod(mJavaHelper, id);
		if (env->ExceptionCheck()) {
			env->ExceptionClear();
			return -1;
		} else return 0;
	} else return -1;
}

string AndroidPlatformHelpers::getNativeLibraryDir() {
	JNIEnv *env = ms_get_jni_env();
	string libPath;
	jstring jlib_path = (jstring)env->CallObjectMethod(mJavaHelper, mGetNativeLibraryDirId);
	if (jlib_path) {
		const char *lib_path = GetStringUTFChars(env, jlib_path);
		libPath = lib_path;
		ReleaseStringUTFChars(env, jlib_path, lib_path);
	}
	return libPath;
}

PlatformHelpers *createAndroidPlatformHelpers(std::shared_ptr<LinphonePrivate::Core> core, void *systemContext) {
	return new AndroidPlatformHelpers(core, systemContext);
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_AndroidPlatformHelper_setNativePreviewWindowId(
    BCTBX_UNUSED(JNIEnv *env), BCTBX_UNUSED(jobject thiz), jlong ptr, jobject id) {
	AndroidPlatformHelpers *androidPlatformHelper = static_cast<AndroidPlatformHelpers *>((void *)ptr);
	androidPlatformHelper->_setPreviewVideoWindow(id);
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_AndroidPlatformHelper_setNativeVideoWindowId(
    BCTBX_UNUSED(JNIEnv *env), BCTBX_UNUSED(jobject thiz), jlong ptr, jobject id) {
	AndroidPlatformHelpers *androidPlatformHelper = static_cast<AndroidPlatformHelpers *>((void *)ptr);
	androidPlatformHelper->_setVideoWindow(id);
}

extern "C" JNIEXPORT void JNICALL
Java_org_linphone_core_tools_AndroidPlatformHelper_setParticipantDeviceNativeVideoWindowId(
    BCTBX_UNUSED(JNIEnv *env), BCTBX_UNUSED(jobject thiz), jlong ptr, jlong participantDevicePtr, jobject id) {
	AndroidPlatformHelpers *androidPlatformHelper = static_cast<AndroidPlatformHelpers *>((void *)ptr);
	LinphoneParticipantDevice *participantDevice =
	    static_cast<LinphoneParticipantDevice *>((void *)participantDevicePtr);
	androidPlatformHelper->_setParticipantDeviceVideoWindow(participantDevice, id);
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_AndroidPlatformHelper_setNetworkReachable(
    BCTBX_UNUSED(JNIEnv *env), BCTBX_UNUSED(jobject thiz), jlong ptr, jboolean reachable) {
	AndroidPlatformHelpers *androidPlatformHelper = static_cast<AndroidPlatformHelpers *>((void *)ptr);
	const std::function<void()> fun = [androidPlatformHelper, reachable]() {
		androidPlatformHelper->setNetworkReachable(reachable);
	};
	androidPlatformHelper->getCore()->doLater(fun);
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_AndroidPlatformHelper_setDnsServers(
    BCTBX_UNUSED(JNIEnv *env), BCTBX_UNUSED(jobject thiz), jlong ptr) {
	AndroidPlatformHelpers *androidPlatformHelper = static_cast<AndroidPlatformHelpers *>((void *)ptr);
	const std::function<void()> fun = [androidPlatformHelper]() {
		androidPlatformHelper->updateDnsServers();
		androidPlatformHelper->setDnsServers();
	};
	androidPlatformHelper->getCore()->doLater(fun);
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_AndroidPlatformHelper_setHttpProxy(
    JNIEnv *env, BCTBX_UNUSED(jobject thiz), jlong ptr, jstring host, jint port) {
	AndroidPlatformHelpers *androidPlatformHelper = static_cast<AndroidPlatformHelpers *>((void *)ptr);
	const char *hostC = GetStringUTFChars(env, host);
	char *httpProxyHost = ms_strdup(hostC);
	ReleaseStringUTFChars(env, host, hostC);

	const std::function<void()> fun = [androidPlatformHelper, httpProxyHost, port]() {
		androidPlatformHelper->setHttpProxy(httpProxyHost, port);
		ms_free(httpProxyHost);
	};
	androidPlatformHelper->getCore()->doLater(fun);
}

extern "C" JNIEXPORT jboolean JNICALL Java_org_linphone_core_tools_AndroidPlatformHelper_useSystemHttpProxy(
    BCTBX_UNUSED(JNIEnv *env), BCTBX_UNUSED(jobject thiz), jlong ptr) {
	AndroidPlatformHelpers *androidPlatformHelper = static_cast<AndroidPlatformHelpers *>((void *)ptr);
	LpConfig *config = linphone_core_get_config(androidPlatformHelper->getCore()->getCCore());
	return !!linphone_config_get_int(config, "sip", "use_system_http_proxy", 0);
}

extern "C" JNIEXPORT jboolean JNICALL Java_org_linphone_core_tools_AndroidPlatformHelper_isInBackground(
    BCTBX_UNUSED(JNIEnv *env), BCTBX_UNUSED(jobject thiz), jlong ptr) {
	AndroidPlatformHelpers *androidPlatformHelper = static_cast<AndroidPlatformHelpers *>((void *)ptr);
	return androidPlatformHelper->getCore()->isInBackground();
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_AndroidPlatformHelper_enableKeepAlive(
    BCTBX_UNUSED(JNIEnv *env), BCTBX_UNUSED(jobject thiz), jlong ptr, jboolean enable) {
	AndroidPlatformHelpers *androidPlatformHelper = static_cast<AndroidPlatformHelpers *>((void *)ptr);
	linphone_core_enable_keep_alive(androidPlatformHelper->getCore()->getCCore(), enable ? TRUE : FALSE);
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_service_CoreManager_updatePushNotificationInformation(
    JNIEnv *env, BCTBX_UNUSED(jobject thiz), jlong ptr, jstring jparam, jstring jprid) {
	LinphoneCore *core = static_cast<LinphoneCore *>((void *)ptr);
	const char *paramC = GetStringUTFChars(env, jparam);
	const char *pridC = GetStringUTFChars(env, jprid);
	std::string param = paramC;
	std::string prid = pridC;

	const std::function<void()> fun = [core, param, prid]() {
		linphone_core_update_push_notification_information(core, param.c_str(), prid.c_str());
	};
	L_GET_CPP_PTR_FROM_C_OBJECT(core)->performOnIterateThread(fun);

	ReleaseStringUTFChars(env, jprid, pridC);
	ReleaseStringUTFChars(env, jparam, paramC);
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_service_CoreManager_stopCore(BCTBX_UNUSED(JNIEnv *env),
                                                                                            BCTBX_UNUSED(jobject thiz),
                                                                                            jlong ptr) {
	LinphoneCore *core = static_cast<LinphoneCore *>((void *)ptr);

	const std::function<void()> fun = [core]() { linphone_core_stop(core); };
	L_GET_CPP_PTR_FROM_C_OBJECT(core)->performOnIterateThread(fun);
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_service_CoreManager_leaveConference(
    BCTBX_UNUSED(JNIEnv *env), BCTBX_UNUSED(jobject thiz), jlong ptr) {
	LinphoneCore *core = static_cast<LinphoneCore *>((void *)ptr);

	const std::function<void()> fun = [core]() { linphone_core_leave_conference(core); };
	L_GET_CPP_PTR_FROM_C_OBJECT(core)->performOnIterateThread(fun);
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_service_CoreManager_pauseAllCalls(
    BCTBX_UNUSED(JNIEnv *env), BCTBX_UNUSED(jobject thiz), jlong ptr) {
	LinphoneCore *core = static_cast<LinphoneCore *>((void *)ptr);

	const std::function<void()> fun = [core]() { linphone_core_pause_all_calls(core); };
	L_GET_CPP_PTR_FROM_C_OBJECT(core)->performOnIterateThread(fun);
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_service_CoreManager_reloadSoundDevices(
    BCTBX_UNUSED(JNIEnv *env), BCTBX_UNUSED(jobject thiz), jlong ptr) {
	LinphoneCore *core = static_cast<LinphoneCore *>((void *)ptr);

	const std::function<void()> fun = [core]() { linphone_core_reload_sound_devices(core); };
	L_GET_CPP_PTR_FROM_C_OBJECT(core)->performOnIterateThread(fun);
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_service_CoreManager_enterBackground(
    BCTBX_UNUSED(JNIEnv *env), BCTBX_UNUSED(jobject thiz), jlong ptr) {
	LinphoneCore *core = static_cast<LinphoneCore *>((void *)ptr);

	const std::function<void()> fun = [core]() { linphone_core_enter_background(core); };
	L_GET_CPP_PTR_FROM_C_OBJECT(core)->performOnIterateThread(fun);
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_service_CoreManager_enterForeground(
    BCTBX_UNUSED(JNIEnv *env), BCTBX_UNUSED(jobject thiz), jlong ptr) {
	LinphoneCore *core = static_cast<LinphoneCore *>((void *)ptr);

	const std::function<void()> fun = [core]() { linphone_core_enter_foreground(core); };
	L_GET_CPP_PTR_FROM_C_OBJECT(core)->performOnIterateThread(fun);
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_service_CoreManager_processPushNotification(
    JNIEnv *env, BCTBX_UNUSED(jobject thiz), jlong ptr, jstring callId, jstring payload, jboolean isCoreStarting) {
	LinphoneCore *core = static_cast<LinphoneCore *>((void *)ptr);
	const char *c_callId = GetStringUTFChars(env, callId);
	const char *c_payload = GetStringUTFChars(env, payload);
	bool_t is_core_starting = isCoreStarting ? TRUE : FALSE;
	linphone_core_push_notification_received_2(core, c_payload, c_callId, is_core_starting);
	ReleaseStringUTFChars(env, callId, c_callId);
	ReleaseStringUTFChars(env, payload, c_payload);
}

extern "C" JNIEXPORT void JNICALL
Java_org_linphone_core_tools_AndroidPlatformHelper_setSignalInfo(BCTBX_UNUSED(JNIEnv *env),
                                                                 BCTBX_UNUSED(jobject thiz),
                                                                 jlong ptr,
                                                                 jint jtype,
                                                                 jint junit,
                                                                 jint jvalue,
                                                                 jstring details) {
	AndroidPlatformHelpers *androidPlatformHelper = static_cast<AndroidPlatformHelpers *>((void *)ptr);

	LinphoneSignalType type = (LinphoneSignalType)jtype;
	LinphoneSignalStrengthUnit unit = (LinphoneSignalStrengthUnit)junit;
	const char *c_details = GetStringUTFChars(env, details);
	float value = (float)jvalue;
	auto info = (new SignalInformation(type, unit, value, c_details))->toSharedPtr();
	androidPlatformHelper->setSignalInformation(info);
	ReleaseStringUTFChars(env, details, c_details);
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_service_CoreManager_healNetworkConnections(
    BCTBX_UNUSED(JNIEnv *env), BCTBX_UNUSED(jobject thiz), jlong ptr) {
	LinphoneCore *core = static_cast<LinphoneCore *>((void *)ptr);
	L_GET_CPP_PTR_FROM_C_OBJECT(core)->healNetworkConnections();
}

LINPHONE_END_NAMESPACE
