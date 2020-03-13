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

#include "logger/logger.h"
#include "paths-android.h"
#include "c-wrapper/c-wrapper.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

static jmethodID getStaticMethodId (JNIEnv *env, jclass klass, const char *method, const char *signature) {
	jmethodID id = env->GetStaticMethodID(klass, method, signature);
	if (id == 0)
		lFatal() << "Could not find static java method: `" << method << ", " << signature << "`.";
	return id;
}

static const char *GetStringUTFChars (JNIEnv *env, jstring string) {
	const char *cstring = string ? env->GetStringUTFChars(string, nullptr) : nullptr;
	return cstring;
}

static void ReleaseStringUTFChars (JNIEnv *env, jstring string, const char *cstring) {
	if (string) env->ReleaseStringUTFChars(string, cstring);
}

static string getPath (void *context, const char *jMethodName) {
	if (!context) {
		lError() << "context is null.";
		return "";
	}

	JNIEnv *env = ms_get_jni_env();
	jobject jContext = (jobject)context;

	jclass klass = env->FindClass("org/linphone/core/tools/AndroidPlatformHelper");
	if (!klass)
		lFatal() << "Could not find java AndroidPlatformHelper class.";

	jmethodID jMethodId = getStaticMethodId(env, klass, jMethodName, "(Landroid/content/Context;)Ljava/lang/String;");
	jstring jPath = (jstring)env->CallStaticObjectMethod(klass, jMethodId, jContext);

	const char *cPath = GetStringUTFChars(env, jPath);
	string path = L_C_TO_STRING(cPath);
	ReleaseStringUTFChars(env, jPath, cPath);
	return path;
}

string SysPaths::getDataPath (void *context) {
	return getPath(context, "getDataPath");
}

string SysPaths::getConfigPath (void *context) {
	return getPath(context, "getConfigPath");
}

string SysPaths::getDownloadPath (void *context) {
	return getPath(context, "getDownloadPath");
}

LINPHONE_END_NAMESPACE
