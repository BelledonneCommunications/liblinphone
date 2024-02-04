/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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
#include <mediastreamer2/msjava.h>

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

class JavaPlatformHelpers : public GenericPlatformHelpers {
public:
	JavaPlatformHelpers(std::shared_ptr<LinphonePrivate::Core> core);
	~JavaPlatformHelpers();

	void setVideoPreviewWindow(void *windowId) override;
	void setVideoWindow(void *windowId) override;
	void setParticipantDeviceVideoWindow(LinphoneParticipantDevice *participantDevice, void *windowId) override;
	void resizeVideoPreview(int width, int height) override;

private:
	int callVoidMethod(jmethodID id);
	static jmethodID getMethodId(JNIEnv *env, jclass klass, const char *method, const char *signature);
	string getResourcesDir();
	string getMsPluginsDir();

	jobject mJavaHelper = nullptr;
	jobject mPreviewVideoWindow = nullptr;
	jobject mVideoWindow = nullptr;
	unordered_map<long, jobject> mParticipantDeviceVideoWindows;

	// PlatformHelper methods
	jmethodID mGetResourcesDirId = nullptr;
	jmethodID mGetMsPluginsDirId = nullptr;
};

static const char *GetStringUTFChars(JNIEnv *env, jstring string) {
	const char *cstring = string ? env->GetStringUTFChars(string, nullptr) : nullptr;
	return cstring;
}

static void ReleaseStringUTFChars(JNIEnv *env, jstring string, const char *cstring) {
	if (string) env->ReleaseStringUTFChars(string, cstring);
}

jmethodID JavaPlatformHelpers::getMethodId(JNIEnv *env, jclass klass, const char *method, const char *signature) {
	jmethodID id = env->GetMethodID(klass, method, signature);
	if (id == 0)
		lFatal() << "[Java Platform Helper] Could not find java method: `" << method << ", " << signature << "`.";
	return id;
}

// -----------------------------------------------------------------------------

JavaPlatformHelpers::JavaPlatformHelpers(std::shared_ptr<LinphonePrivate::Core> core) : GenericPlatformHelpers(core) {
	JNIEnv *env = ms_get_jni_env();
	jclass klass = env->FindClass("org/linphone/core/tools/java/JavaPlatformHelper");
	if (!klass) lFatal() << "[Java Platform Helper] Could not find java JavaPlatformHelper class.";

	jmethodID ctor = env->GetMethodID(klass, "<init>", "(J)V");
	mJavaHelper = env->NewObject(klass, ctor, (jlong)this);
	if (!mJavaHelper) {
		lError() << "[Java Platform Helper] Could not instanciate JavaPlatformHelper object.";
		return;
	}
	mJavaHelper = (jobject)env->NewGlobalRef(mJavaHelper);

	mGetResourcesDirId = getMethodId(env, klass, "getResourcesDir", "()Ljava/lang/String;");
	mGetMsPluginsDirId = getMethodId(env, klass, "getMsPluginsDir", "()Ljava/lang/String;");

	linphone_factory_set_top_resources_dir(linphone_factory_get(), getResourcesDir().c_str());
	linphone_factory_set_msplugins_dir(linphone_factory_get(), getMsPluginsDir().c_str());
	lInfo() << "[Java Platform Helper] JavaPlatformHelper is fully initialised.";

	mPreviewVideoWindow = nullptr;
	mVideoWindow = nullptr;
}

JavaPlatformHelpers::~JavaPlatformHelpers() {
	if (mJavaHelper) {
		JNIEnv *env = ms_get_jni_env();
		env->DeleteGlobalRef(mJavaHelper);
		mJavaHelper = nullptr;
	}
	lInfo() << "[Java Platform Helper] JavaPlatformHelper has been destroyed.";
}

// -----------------------------------------------------------------------------

void JavaPlatformHelpers::setVideoPreviewWindow(BCTBX_UNUSED(void *windowId)) {
	// TODO
}

void JavaPlatformHelpers::setVideoWindow(BCTBX_UNUSED(void *windowId)) {
	// TODO
}

void JavaPlatformHelpers::setParticipantDeviceVideoWindow(BCTBX_UNUSED(LinphoneParticipantDevice *participantDevice),
                                                          BCTBX_UNUSED(void *windowId)) {
	// TODO
}

void JavaPlatformHelpers::resizeVideoPreview(BCTBX_UNUSED(int width), BCTBX_UNUSED(int height)) {
	// TODO
}

// -----------------------------------------------------------------------------

int JavaPlatformHelpers::callVoidMethod(jmethodID id) {
	JNIEnv *env = ms_get_jni_env();
	if (env && mJavaHelper) {
		env->CallVoidMethod(mJavaHelper, id);
		if (env->ExceptionCheck()) {
			env->ExceptionClear();
			return -1;
		} else return 0;
	} else return -1;
}

string JavaPlatformHelpers::getResourcesDir() {
	JNIEnv *env = ms_get_jni_env();
	string resourcesPath;
	jstring jresourcesPath = (jstring)env->CallObjectMethod(mJavaHelper, mGetResourcesDirId);
	if (jresourcesPath) {
		const char *cresourcesPath = GetStringUTFChars(env, jresourcesPath);
		resourcesPath = cresourcesPath;
		ReleaseStringUTFChars(env, jresourcesPath, cresourcesPath);
	}
	return resourcesPath;
}

string JavaPlatformHelpers::getMsPluginsDir() {
	JNIEnv *env = ms_get_jni_env();
	string pluginsPath;
	jstring jpluginsPath = (jstring)env->CallObjectMethod(mJavaHelper, mGetMsPluginsDirId);
	if (jpluginsPath) {
		const char *cpluginsPath = GetStringUTFChars(env, jpluginsPath);
		pluginsPath = cpluginsPath;
		ReleaseStringUTFChars(env, jpluginsPath, cpluginsPath);
	}
	return pluginsPath;
}

PlatformHelpers *createJavaPlatformHelpers(std::shared_ptr<LinphonePrivate::Core> core) {
	return new JavaPlatformHelpers(core);
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_JavaPlatformHelper_setNativePreviewWindowId(
    BCTBX_UNUSED(JNIEnv *env), BCTBX_UNUSED(jobject thiz), BCTBX_UNUSED(jlong ptr), BCTBX_UNUSED(jobject id)) {
	// TODO
}

extern "C" JNIEXPORT void JNICALL Java_org_linphone_core_tools_JavaPlatformHelper_setNativeVideoWindowId(
    BCTBX_UNUSED(JNIEnv *env), BCTBX_UNUSED(jobject thiz), BCTBX_UNUSED(jlong ptr), BCTBX_UNUSED(jobject id)) {
	// TODO
}

extern "C" JNIEXPORT void JNICALL
Java_org_linphone_core_tools_JavaPlatformHelper_setParticipantDeviceNativeVideoWindowId(
    BCTBX_UNUSED(JNIEnv *env),
    BCTBX_UNUSED(jobject thiz),
    BCTBX_UNUSED(jlong ptr),
    BCTBX_UNUSED(jlong participantDevicePtr),
    BCTBX_UNUSED(jobject id)) {
	// TODO
}

LINPHONE_END_NAMESPACE
