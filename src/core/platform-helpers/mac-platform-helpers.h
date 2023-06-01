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

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif
#if TARGET_OS_MAC

#include "platform-helpers.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE
/*
 Common helper for mac plaform including IOS and OSX
 */
class MacPlatformHelpers : public GenericPlatformHelpers {
public:
	MacPlatformHelpers(std::shared_ptr<LinphonePrivate::Core> core);
	virtual ~MacPlatformHelpers();
	string getDataResource(const string &filename) const override;
	string getImageResource(const string &filename) const override;
	string getRingResource(const string &filename) const override;
	string getSoundResource(const string &filename) const override;
	string getPluginsDir() const override;
	void setHttpProxy(const string &host, int port) override;

protected:
	static const string Framework;
	void getHttpProxySettings(void);
	static string toString(CFStringRef str, CFStringEncoding encodingMethod);
	static string toUTF8String(CFStringRef str);
private:
	static string getBundleResourceDirPath(const string &framework, const string &resource);
	static string getResourceDirPath(const string &framework, const string &resource);
	static string getResourcePath(const string &framework, const string &resource);
};

LINPHONE_END_NAMESPACE

#endif
