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
#if TARGET_OS_MAC

#include <Foundation/Foundation.h>
#include <CoreLocation/CoreLocation.h>
#include <belr/grammarbuilder.h>
#include "c-wrapper/c-wrapper.h"

#include "logger/logger.h"
#include "mac-platform-helpers.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// =============================================================================

const string MacPlatformHelpers::Framework = "org.linphone.linphone";

MacPlatformHelpers::MacPlatformHelpers (std::shared_ptr<LinphonePrivate::Core> core) : GenericPlatformHelpers(core){

	string cpimPath = getResourceDirPath(Framework, "cpim_grammar");
	if (!cpimPath.empty())
		belr::GrammarLoader::get().addPath(cpimPath);
	else
		lError() << "MacPlatformHelpers did not find cpim grammar resource directory...";

	string icsPath = getResourceDirPath(Framework, "ics_grammar");
	if (!icsPath.empty())
		belr::GrammarLoader::get().addPath(icsPath);
	else
		lError() << "MacPlatformHelpers did not find ics grammar resource directory...";

	string identityPath = getResourceDirPath(Framework, "identity_grammar");
	if (!identityPath.empty())
		belr::GrammarLoader::get().addPath(identityPath);
	else
		lError() << "MacPlatformHelpers did not find identity grammar resource directory...";

#ifdef VCARD_ENABLED
	string vcardPath = getResourceDirPath("org.linphone.belcard", "vcard_grammar");
	if (!vcardPath.empty())
		belr::GrammarLoader::get().addPath(vcardPath);
	else
		lInfo() << "MacPlatformHelpers did not find vcard grammar resource directory...";
#endif

	string sdpPath = getResourceDirPath("org.linphone.belle-sip", "sdp_grammar");
	if (!sdpPath.empty())
		belr::GrammarLoader::get().addPath(sdpPath);
	else
		lError() << "MacPlatformHelpers did not find sdp grammar resource directory...";
	
	lInfo() << "MacPlatformHelpers is fully started";
}

MacPlatformHelpers::~MacPlatformHelpers () {

}
	
// -----------------------------------------------------------------------------

string MacPlatformHelpers::getDataResource (const string &filename) const {
	return getResourcePath(Framework, filename);
}

string MacPlatformHelpers::getImageResource (const string &filename) const {
	return getResourcePath(Framework, filename);
}

string MacPlatformHelpers::getRingResource (const string &filename) const {
	return getResourcePath(Framework, filename);
}

string MacPlatformHelpers::getSoundResource (const string &filename) const {
	return getResourcePath(Framework, filename);
}

// -----------------------------------------------------------------------------

string MacPlatformHelpers::getResourceDirPath (const string &framework, const string &resource) {
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

string MacPlatformHelpers::getResourcePath (const string &framework, const string &resource) {
	return getResourceDirPath(framework, resource) + "/" + resource;
}


//Set proxy settings on core
void MacPlatformHelpers::setHttpProxy (const string &host, int port) {
	if (linphone_core_get_global_state(getCore()->getCCore()) == LinphoneGlobalOff)
		return;
	linphone_core_set_http_proxy_host(getCore()->getCCore(), host.c_str());
	linphone_core_set_http_proxy_port(getCore()->getCCore(), port);
}

//Get global proxy settings from system and set variables mHttpProxy{Host,Port,Enabled}.
void MacPlatformHelpers::getHttpProxySettings(void) {
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

//Safely get an UTF-8 string from the given CFStringRef
string MacPlatformHelpers::toUTF8String(CFStringRef str) {
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

PlatformHelpers *createMacPlatformHelpers(std::shared_ptr<LinphonePrivate::Core> core, UNUSED(void *systemContext)) {
	return new MacPlatformHelpers(core);
}

LINPHONE_END_NAMESPACE

#endif
