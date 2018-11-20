/*
 * android-platform-helpers.h
 * Copyright (C) 2010-2018 Belledonne Communications SARL
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <jni.h>

#include "platform-helpers.h"
#include "logger/logger.h"
#include "c-wrapper/c-wrapper.h"

// TODO: Remove me later.
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

class AndroidPlatformHelpers : public PlatformHelpers {
public:
	AndroidPlatformHelpers (LinphoneCore *lc, void *systemContext);
	~AndroidPlatformHelpers ();

	void setDnsServers () override;
	void acquireWifiLock () override;
	void releaseWifiLock () override;
	void acquireMcastLock () override;
	void releaseMcastLock () override;
	void acquireCpuLock () override;
	void releaseCpuLock () override;
	string getDataPath () override;
	string getConfigPath () override;
	void setVideoWindow (void *windowId) override;
	void setVideoPreviewWindow (void *windowId) override;
	void setNetworkReachable (bool reachable) override;
	void onLinphoneCoreReady () override;

	void _setPreviewVideoWindow(jobject window);
	void _setVideoWindow(jobject window);

private:
	int callVoidMethod (jmethodID id);
	static jmethodID getMethodId (JNIEnv *env, jclass klass, const char *method, const char *signature);
	string getNativeLibraryDir();
	jobject mJavaHelper;
	jmethodID mWifiLockAcquireId;
	jmethodID mWifiLockReleaseId;
	jmethodID mMcastLockAcquireId;
	jmethodID mMcastLockReleaseId;
	jmethodID mCpuLockAcquireId;
	jmethodID mCpuLockReleaseId;
	jmethodID mGetDnsServersId;
	jmethodID mGetPowerManagerId;
	jmethodID mGetDataPathId;
	jmethodID mGetConfigPathId;
	jmethodID mGetNativeLibraryDirId;
	jmethodID mSetNativeVideoWindowId;
	jmethodID mSetNativePreviewVideoWindowId;
	jmethodID mUpdateNetworkReachabilityId;
	jmethodID mOnLinphoneCoreReadyId;
	jobject mPreviewVideoWindow;
	jobject mVideoWindow;
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
		lFatal() << "Could not find java method: `" << method << ", " << signature << "`.";
	return id;
}

AndroidPlatformHelpers::AndroidPlatformHelpers (LinphoneCore *lc, void *systemContext) : PlatformHelpers(lc) {
	JNIEnv *env = ms_get_jni_env();
	jclass klass = env->FindClass("org/linphone/core/tools/AndroidPlatformHelper");
	if (!klass)
		lFatal() << "Could not find java AndroidPlatformHelper class.";

	jmethodID ctor = env->GetMethodID(klass, "<init>", "(JLjava/lang/Object;)V");
	mJavaHelper = env->NewObject(klass, ctor, this, (jobject)systemContext);
	if (!mJavaHelper) {
		lError() << "Could not instanciate AndroidPlatformHelper object.";
		return;
	}
	mJavaHelper = (jobject)env->NewGlobalRef(mJavaHelper);

	mWifiLockAcquireId = getMethodId(env, klass, "acquireWifiLock", "()V");
	mWifiLockReleaseId = getMethodId(env, klass, "releaseWifiLock", "()V");
	mMcastLockAcquireId = getMethodId(env, klass, "acquireMcastLock", "()V");
	mMcastLockReleaseId = getMethodId(env, klass, "releaseMcastLock", "()V");
	mCpuLockAcquireId = getMethodId(env, klass, "acquireCpuLock", "()V");
	mCpuLockReleaseId = getMethodId(env, klass, "releaseCpuLock", "()V");
	mGetDnsServersId = getMethodId(env, klass, "getDnsServers", "()[Ljava/lang/String;");
	mGetPowerManagerId = getMethodId(env, klass, "getPowerManager", "()Ljava/lang/Object;");
	mGetDataPathId = getMethodId(env, klass, "getDataPath", "()Ljava/lang/String;");
	mGetConfigPathId = getMethodId(env, klass, "getConfigPath", "()Ljava/lang/String;");
	mGetNativeLibraryDirId = getMethodId(env, klass, "getNativeLibraryDir", "()Ljava/lang/String;");
	mSetNativeVideoWindowId = getMethodId(env, klass, "setVideoRenderingView", "(Ljava/lang/Object;)V");
	mSetNativePreviewVideoWindowId = getMethodId(env, klass, "setVideoPreviewView", "(Ljava/lang/Object;)V");
	mUpdateNetworkReachabilityId = getMethodId(env, klass, "updateNetworkReachability", "()V");
	mOnLinphoneCoreReadyId = getMethodId(env, klass, "onLinphoneCoreReady", "()V");

	jobject pm = env->CallObjectMethod(mJavaHelper, mGetPowerManagerId);
	belle_sip_wake_lock_init(env, pm);

	linphone_factory_set_top_resources_dir(linphone_factory_get() , getDataPath().append("share").c_str());
	linphone_factory_set_msplugins_dir(linphone_factory_get(), getNativeLibraryDir().c_str());
	lInfo() << "AndroidPlatformHelpers is fully initialised.";

	mPreviewVideoWindow = nullptr;
	mVideoWindow = nullptr;
}

AndroidPlatformHelpers::~AndroidPlatformHelpers () {
	if (mJavaHelper) {
		JNIEnv *env = ms_get_jni_env();
		belle_sip_wake_lock_uninit(env);
		env->DeleteGlobalRef(mJavaHelper);
		mJavaHelper = nullptr;
	}
	lInfo() << "AndroidPlatformHelpers has been destroyed.";
}

void AndroidPlatformHelpers::setDnsServers () {
	if (!mJavaHelper || linphone_core_get_dns_set_by_app(mCore)) {
		lError() << "AndroidPlatformHelpers' mJavaHelper is null.";
		return;
	}
	JNIEnv *env = ms_get_jni_env();
	if (env && mJavaHelper) {
		jobjectArray jservers = (jobjectArray)env->CallObjectMethod(mJavaHelper, mGetDnsServersId);
		bctbx_list_t *l = nullptr;
		if (env->ExceptionCheck()) {
			env->ExceptionClear();
			lError() << "AndroidPlatformHelpers::setDnsServers() exception.";
			return;
		}
		if (jservers != nullptr) {
			int count = env->GetArrayLength(jservers);

			for (int i = 0; i < count; i++) {
				jstring jserver = (jstring)env->GetObjectArrayElement(jservers, i);
				const char *str = env->GetStringUTFChars(jserver, nullptr);
				if (str) {
					lInfo() << "AndroidPlatformHelpers found DNS server " << str;
					l = bctbx_list_append(l, ms_strdup(str));
					env->ReleaseStringUTFChars(jserver, str);
				}
			}
		} else {
			lError() << "AndroidPlatformHelpers::setDnsServers() failed to get DNS servers list";
		}
		linphone_core_set_dns_servers(mCore, l);
		bctbx_list_free_with_data(l, ms_free);
	}
}

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

string AndroidPlatformHelpers::getDataPath () {
	JNIEnv *env = ms_get_jni_env();
	jstring jdata_path = (jstring)env->CallObjectMethod(mJavaHelper, mGetDataPathId);
	const char *data_path = GetStringUTFChars(env, jdata_path);
	string dataPath = data_path;
	ReleaseStringUTFChars(env, jdata_path, data_path);
	return dataPath + "/";
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

string AndroidPlatformHelpers::getConfigPath () {
	JNIEnv *env = ms_get_jni_env();
	jstring jconfig_path = (jstring)env->CallObjectMethod(mJavaHelper, mGetConfigPathId);
	const char *config_path = GetStringUTFChars(env, jconfig_path);
	string configPath = config_path;
	ReleaseStringUTFChars(env, jconfig_path, config_path);
	return configPath + "/";
}

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

void AndroidPlatformHelpers::setVideoPreviewWindow (void *windowId) {
	JNIEnv *env = ms_get_jni_env();
	if (env && mJavaHelper) {
		string displayFilter = L_C_TO_STRING(linphone_core_get_video_display_filter(getCore()));
		if (windowId && (displayFilter.empty() || displayFilter == "MSAndroidTextureDisplay")) {
			env->CallVoidMethod(mJavaHelper, mSetNativePreviewVideoWindowId, (jobject)windowId);
		} else {
			_setPreviewVideoWindow((jobject)windowId);
		}
	}
}

void AndroidPlatformHelpers::setVideoWindow (void *windowId) {
	JNIEnv *env = ms_get_jni_env();
	if (env && mJavaHelper) {
		string displayFilter = L_C_TO_STRING(linphone_core_get_video_display_filter(getCore()));
		if (windowId && (displayFilter.empty() || displayFilter == "MSAndroidTextureDisplay")) {
			env->CallVoidMethod(mJavaHelper, mSetNativeVideoWindowId, (jobject)windowId);
		} else {
			_setVideoWindow((jobject)windowId);
		}
	}
}

void AndroidPlatformHelpers::_setPreviewVideoWindow(jobject window) {
	JNIEnv *env = ms_get_jni_env();
	LinphoneCore *lc = getCore();
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
	LinphoneCore *lc = getCore();
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

void AndroidPlatformHelpers::setNetworkReachable(bool reachable) {
	linphone_core_set_network_reachable(mCore, reachable);
}

void AndroidPlatformHelpers::onLinphoneCoreReady() {
	JNIEnv *env = ms_get_jni_env();
	if (env && mJavaHelper) {
		env->CallVoidMethod(mJavaHelper, mOnLinphoneCoreReadyId);
	}
}

PlatformHelpers *createAndroidPlatformHelpers (LinphoneCore *lc, void *systemContext) {
	return new AndroidPlatformHelpers(lc, systemContext);
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_AndroidPlatformHelper_setNativePreviewWindowId(JNIEnv *env, jobject thiz, jlong ptr, jobject id) {
	AndroidPlatformHelpers *androidPlatformHelper = (AndroidPlatformHelpers *)ptr;
	androidPlatformHelper->_setPreviewVideoWindow(id);
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_AndroidPlatformHelper_setNativeVideoWindowId(JNIEnv *env, jobject thiz, jlong ptr, jobject id) {
	AndroidPlatformHelpers *androidPlatformHelper = (AndroidPlatformHelpers *)ptr;
	androidPlatformHelper->_setVideoWindow(id);
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_AndroidPlatformHelper_setNetworkReachable(JNIEnv* env, jobject thiz, jlong ptr, jboolean reachable) {
	AndroidPlatformHelpers *androidPlatformHelper = (AndroidPlatformHelpers *)ptr;
	androidPlatformHelper->setNetworkReachable(reachable);
}

LINPHONE_END_NAMESPACE
