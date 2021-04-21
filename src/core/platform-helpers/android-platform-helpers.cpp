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

#include <jni.h>

#include "platform-helpers.h"
#include "logger/logger.h"
#include "c-wrapper/c-wrapper.h"
#include "core/paths/paths.h"

// TODO: Remove me later.
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

class AndroidPlatformHelpers : public GenericPlatformHelpers {
public:
	AndroidPlatformHelpers (std::shared_ptr<LinphonePrivate::Core> core, void *systemContext);
	~AndroidPlatformHelpers ();

	void acquireWifiLock () override;
	void releaseWifiLock () override;
	void acquireMcastLock () override;
	void releaseMcastLock () override;
	void acquireCpuLock () override;
	void releaseCpuLock () override;

	string getConfigPath () const override;
	string getDataPath () const override;
	string getDataResource (const string &filename) const override;
	string getImageResource (const string &filename) const override;
	string getRingResource (const string &filename) const override;
	string getSoundResource (const string &filename) const override;
	void *getPathContext () override;

	void setVideoPreviewWindow (void *windowId) override;
	void setVideoWindow (void *windowId) override;
	void resizeVideoPreview (int width, int height) override;

	bool isNetworkReachable () override;
	bool isActiveNetworkWifiOnlyCompliant () const override;
	void onWifiOnlyEnabled (bool enabled) override;
	void setDnsServers () override;
	void setNetworkReachable (bool reachable) override;
	void setHttpProxy (const string &host, int port) override;

	void onLinphoneCoreStart (bool monitoringEnabled) override;
	void onLinphoneCoreStop () override;

	void startAudioForEchoTestOrCalibration () override;
	void stopAudioForEchoTestOrCalibration () override;

	void _setPreviewVideoWindow(jobject window);
	void _setVideoWindow(jobject window);
	string getDownloadPath() override;

private:
	int callVoidMethod (jmethodID id);
	static jmethodID getMethodId (JNIEnv *env, jclass klass, const char *method, const char *signature);
	string getNativeLibraryDir ();
	void createCoreManager (std::shared_ptr<LinphonePrivate::Core> core, void *systemContext);
	void destroyCoreManager ();

	jobject mJavaHelper = nullptr;
	jobject mSystemContext = nullptr;
	jobject mJavaCoreManager = nullptr;
	jobject mPreviewVideoWindow = nullptr;
	jobject mVideoWindow = nullptr;

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
	jmethodID mResizeVideoPreviewId = nullptr;
	jmethodID mOnLinphoneCoreStartId = nullptr;
	jmethodID mOnLinphoneCoreStopId = nullptr;
	jmethodID mOnWifiOnlyEnabledId = nullptr;
	jmethodID mIsActiveNetworkWifiOnlyCompliantId = nullptr;

	// CoreManager methods
	jmethodID mCoreManagerDestroyId = nullptr;
	jmethodID mCoreManagerOnLinphoneCoreStartId = nullptr;
	jmethodID mCoreManagerOnLinphoneCoreStopId = nullptr;
	jmethodID mStartAudioForEchoTestOrCalibrationId = nullptr;
	jmethodID mStopAudioForEchoTestOrCalibrationId = nullptr;

	bool mNetworkReachable = false;
};

static const char *GetStringUTFChars (JNIEnv *env, jstring string) {
	const char *cstring = string ? env->GetStringUTFChars(string, nullptr) : nullptr;
	return cstring;
}

static void ReleaseStringUTFChars (JNIEnv *env, jstring string, const char *cstring) {
	if (string) env->ReleaseStringUTFChars(string, cstring);
}

jmethodID AndroidPlatformHelpers::getMethodId (JNIEnv *env, jclass klass, const char *method, const char *signature) {
	jmethodID id = env->GetMethodID(klass, method, signature);
	if (id == 0)
		lFatal() << "[Android Platform Helper] Could not find java method: `" << method << ", " << signature << "`.";
	return id;
}

// -----------------------------------------------------------------------------

extern "C" jobject getCore(JNIEnv *env, LinphoneCore *cptr, bool_t takeref);

void AndroidPlatformHelpers::createCoreManager (std::shared_ptr<LinphonePrivate::Core> core, void *systemContext) {
	JNIEnv *env = ms_get_jni_env();
	jclass klass = env->FindClass("org/linphone/core/tools/service/CoreManager");
	if (!klass) {
		lError() << "[Android Platform Helper] Could not find java CoreManager class.";
		return;
	}

	jmethodID ctor = env->GetMethodID(klass, "<init>", "(Ljava/lang/Object;Lorg/linphone/core/Core;)V");
	LinphoneCore *lc = L_GET_C_BACK_PTR(core);
	jobject javaCore = ::LinphonePrivate::getCore(env, lc, FALSE);
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

	lInfo() << "[Android Platform Helper] CoreManager is fully initialised.";
}

void AndroidPlatformHelpers::destroyCoreManager () {
	if (mJavaCoreManager) {
		JNIEnv *env = ms_get_jni_env();
		env->CallVoidMethod(mJavaCoreManager, mCoreManagerDestroyId);
		env->DeleteGlobalRef(mJavaCoreManager);
		mJavaCoreManager = nullptr;
		lInfo() << "[Android Platform Helper] CoreManager has been destroyed.";
	}
}

// -----------------------------------------------------------------------------

AndroidPlatformHelpers::AndroidPlatformHelpers (std::shared_ptr<LinphonePrivate::Core> core, void *systemContext) : GenericPlatformHelpers(core) {
	createCoreManager(core, systemContext);

	JNIEnv *env = ms_get_jni_env();
	jclass klass = env->FindClass("org/linphone/core/tools/AndroidPlatformHelper");
	if (!klass)
		lFatal() << "[Android Platform Helper] Could not find java AndroidPlatformHelper class.";

	jmethodID ctor = env->GetMethodID(klass, "<init>", "(JLjava/lang/Object;Z)V");
	mJavaHelper = env->NewObject(klass, ctor, (jlong)this, (jobject)systemContext, (jboolean)linphone_core_wifi_only_enabled(getCore()->getCCore()));
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
	mResizeVideoPreviewId = getMethodId(env, klass, "resizeVideoPreview", "(II)V");
	mOnLinphoneCoreStartId = getMethodId(env, klass, "onLinphoneCoreStart", "(Z)V");
	mOnLinphoneCoreStopId = getMethodId(env, klass, "onLinphoneCoreStop", "()V");
	mOnWifiOnlyEnabledId = getMethodId(env, klass, "onWifiOnlyEnabled", "(Z)V");
	mIsActiveNetworkWifiOnlyCompliantId = getMethodId(env, klass, "isActiveNetworkWifiOnlyCompliant", "()Z");

	jobject pm = env->CallObjectMethod(mJavaHelper, mGetPowerManagerId);
	belle_sip_wake_lock_init(env, pm);

	linphone_factory_set_top_resources_dir(linphone_factory_get() , getDataPath().append("share").c_str());
	linphone_factory_set_msplugins_dir(linphone_factory_get(), getNativeLibraryDir().c_str());
	lInfo() << "[Android Platform Helper] AndroidPlatformHelper is fully initialised.";

	mPreviewVideoWindow = nullptr;
	mVideoWindow = nullptr;
	mNetworkReachable = false;
}

AndroidPlatformHelpers::~AndroidPlatformHelpers () {
	destroyCoreManager();
	if (mJavaHelper) {
		JNIEnv *env = ms_get_jni_env();
		belle_sip_wake_lock_uninit(env);
		env->DeleteGlobalRef(mJavaHelper);
		mJavaHelper = nullptr;
	}
	lInfo() << "[Android Platform Helper] AndroidPlatformHelper has been destroyed.";
}

// -----------------------------------------------------------------------------

void AndroidPlatformHelpers::acquireWifiLock () {
	callVoidMethod(mWifiLockAcquireId);
}

void AndroidPlatformHelpers::releaseWifiLock () {
	callVoidMethod(mWifiLockReleaseId);
}

void AndroidPlatformHelpers::acquireMcastLock () {
	callVoidMethod(mMcastLockAcquireId);
}

void AndroidPlatformHelpers::releaseMcastLock () {
	callVoidMethod(mMcastLockReleaseId);
}

void AndroidPlatformHelpers::acquireCpuLock () {
	callVoidMethod(mCpuLockAcquireId);
}

void AndroidPlatformHelpers::releaseCpuLock () {
	callVoidMethod(mCpuLockReleaseId);
}

// -----------------------------------------------------------------------------

string AndroidPlatformHelpers::getConfigPath () const {
	return Paths::getPath(Paths::Config, mSystemContext);
}

string AndroidPlatformHelpers::getDownloadPath () {
	return Paths::getPath(Paths::Download, mSystemContext);
}

string AndroidPlatformHelpers::getDataPath () const {
	return Paths::getPath(Paths::Data, mSystemContext);
}

string AndroidPlatformHelpers::getDataResource (const string &filename) const {
	return getFilePath(
		linphone_factory_get_data_resources_dir(linphone_factory_get()),
		filename
	);
}

string AndroidPlatformHelpers::getImageResource (const string &filename) const {
	return getFilePath(
		linphone_factory_get_image_resources_dir(linphone_factory_get()),
		filename
	);
}

string AndroidPlatformHelpers::getRingResource (const string &filename) const {
	return getFilePath(
		linphone_factory_get_ring_resources_dir(linphone_factory_get()),
		filename
	);
}

string AndroidPlatformHelpers::getSoundResource (const string &filename) const {
	return getFilePath(
		linphone_factory_get_sound_resources_dir(linphone_factory_get()),
		filename
	);
}

void *AndroidPlatformHelpers::getPathContext () {
	return mSystemContext;
}

// -----------------------------------------------------------------------------

void AndroidPlatformHelpers::setVideoPreviewWindow (void *windowId) {
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

void AndroidPlatformHelpers::setVideoWindow (void *windowId) {
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

void AndroidPlatformHelpers::resizeVideoPreview (int width, int height) {
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

void AndroidPlatformHelpers::setDnsServers () {
	if (!mJavaHelper) {
		lError() << "[Android Platform Helper] mJavaHelper is null.";
		return;
	}

	if (linphone_core_get_dns_set_by_app(getCore()->getCCore())) {
		lWarning() << "[Android Platform Helper] Detected DNS servers have been overriden by app.";
		return;
	}

	JNIEnv *env = ms_get_jni_env();
	if (env) {
		jobjectArray jservers = (jobjectArray)env->CallObjectMethod(mJavaHelper, mGetDnsServersId);
		bctbx_list_t *l = nullptr;
		if (env->ExceptionCheck()) {
			env->ExceptionClear();
			lError() << "[Android Platform Helper] setDnsServers() exception.";
			return;
		}

		if (jservers != nullptr) {
			int count = env->GetArrayLength(jservers);

			for (int i = 0; i < count; i++) {
				jstring jserver = (jstring)env->GetObjectArrayElement(jservers, i);
				const char *str = GetStringUTFChars(env, jserver);
				if (str) {
					lInfo() << "[Android Platform Helper] Found DNS server " << str;
					l = bctbx_list_append(l, ms_strdup(str));
					ReleaseStringUTFChars(env, jserver, str);
				}
			}
		} else {
			lError() << "[Android Platform Helper] setDnsServers() failed to get DNS servers list";
			return;
		}
		
		linphone_core_set_dns_servers(getCore()->getCCore(), l);
		bctbx_list_free_with_data(l, ms_free);
	}
}

void AndroidPlatformHelpers::setNetworkReachable(bool reachable) {
	mNetworkReachable = reachable;
	linphone_core_set_network_reachable_internal(getCore()->getCCore(), reachable ? 1 : 0);
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

void AndroidPlatformHelpers::startAudioForEchoTestOrCalibration () {
	JNIEnv *env = ms_get_jni_env();
	if (env) {
		if (mJavaCoreManager) {
			env->CallVoidMethod(mJavaCoreManager, mStartAudioForEchoTestOrCalibrationId);
		}
	}
}

void AndroidPlatformHelpers::stopAudioForEchoTestOrCalibration () {
	JNIEnv *env = ms_get_jni_env();
	if (env) {
		if (mJavaCoreManager) {
			env->CallVoidMethod(mJavaCoreManager, mStopAudioForEchoTestOrCalibrationId);
		}
	}
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

// -----------------------------------------------------------------------------

int AndroidPlatformHelpers::callVoidMethod (jmethodID id) {
	JNIEnv *env = ms_get_jni_env();
	if (env && mJavaHelper) {
		env->CallVoidMethod(mJavaHelper, id);
		if (env->ExceptionCheck()) {
			env->ExceptionClear();
			return -1;
		} else
			return 0;
	} else
		return -1;
}

string AndroidPlatformHelpers::getNativeLibraryDir () {
	JNIEnv *env = ms_get_jni_env();
	string libPath;
	jstring jlib_path = (jstring)env->CallObjectMethod(mJavaHelper, mGetNativeLibraryDirId);
	if (jlib_path){
		const char *lib_path = GetStringUTFChars(env, jlib_path);
		libPath = lib_path;
		ReleaseStringUTFChars(env, jlib_path, lib_path);
	}
	return libPath;
}

PlatformHelpers *createAndroidPlatformHelpers (std::shared_ptr<LinphonePrivate::Core> core, void *systemContext) {
	return new AndroidPlatformHelpers(core, systemContext);
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_AndroidPlatformHelper_setNativePreviewWindowId(JNIEnv *env, jobject thiz, jlong ptr, jobject id) {
	AndroidPlatformHelpers *androidPlatformHelper = static_cast<AndroidPlatformHelpers *>((void *)ptr);
	androidPlatformHelper->_setPreviewVideoWindow(id);
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_AndroidPlatformHelper_setNativeVideoWindowId(JNIEnv *env, jobject thiz, jlong ptr, jobject id) {
	AndroidPlatformHelpers *androidPlatformHelper = static_cast<AndroidPlatformHelpers *>((void *)ptr);
	androidPlatformHelper->_setVideoWindow(id);
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_AndroidPlatformHelper_setNetworkReachable(JNIEnv* env, jobject thiz, jlong ptr, jboolean reachable) {
	AndroidPlatformHelpers *androidPlatformHelper = static_cast<AndroidPlatformHelpers *>((void *)ptr);
	const std::function<void ()> fun = [androidPlatformHelper, reachable]() {
		androidPlatformHelper->setNetworkReachable(reachable);
	};
	androidPlatformHelper->getCore()->doLater(fun);
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_AndroidPlatformHelper_setHttpProxy(JNIEnv* env, jobject thiz, jlong ptr, jstring host, jint port) {
	AndroidPlatformHelpers *androidPlatformHelper = static_cast<AndroidPlatformHelpers *>((void *)ptr);
	const char *hostC = GetStringUTFChars(env, host);
	char * httpProxyHost = ms_strdup(hostC);
	ReleaseStringUTFChars(env, host, hostC);

	const std::function<void ()> fun = [androidPlatformHelper, httpProxyHost, port]() {
		androidPlatformHelper->setHttpProxy(httpProxyHost, port);
		ms_free(httpProxyHost);
	};
	androidPlatformHelper->getCore()->doLater(fun);
}

extern "C" JNIEXPORT jboolean JNICALL Java_org_linphone_core_tools_AndroidPlatformHelper_useSystemHttpProxy(JNIEnv* env, jobject thiz, jlong ptr) {
	AndroidPlatformHelpers *androidPlatformHelper = static_cast<AndroidPlatformHelpers *>((void *)ptr);
	LpConfig *config = linphone_core_get_config(androidPlatformHelper->getCore()->getCCore());
	return !!linphone_config_get_int(config, "sip", "use_system_http_proxy", 0);
}

extern "C" JNIEXPORT jboolean JNICALL Java_org_linphone_core_tools_AndroidPlatformHelper_isInBackground(JNIEnv *env, jobject thiz, jlong ptr) {
	AndroidPlatformHelpers *androidPlatformHelper = static_cast<AndroidPlatformHelpers *>((void *)ptr);
	return androidPlatformHelper->getCore()->isInBackground();
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_AndroidPlatformHelper_enableKeepAlive(JNIEnv *env, jobject thiz, jlong ptr, jboolean enable) {
	AndroidPlatformHelpers *androidPlatformHelper = static_cast<AndroidPlatformHelpers *>((void *)ptr);
	linphone_core_enable_keep_alive(androidPlatformHelper->getCore()->getCCore(), enable ? TRUE : FALSE);
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_service_CoreManager_updatePushNotificationInformation(JNIEnv *env, jobject thiz, jlong ptr, jstring param, jstring prid) {
	LinphoneCore *core = static_cast<LinphoneCore *>((void *)ptr);
	const char *paramC = GetStringUTFChars(env, param);
	const char *pridC = GetStringUTFChars(env, prid);
	linphone_core_update_push_notification_information(core, paramC, pridC);
	ReleaseStringUTFChars(env, prid, pridC);
	ReleaseStringUTFChars(env, param, paramC);
}

LINPHONE_END_NAMESPACE
