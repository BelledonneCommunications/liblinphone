/*
 * platform-helpers.h
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

#ifndef _L_PLATFORM_HELPERS_H_
#define _L_PLATFORM_HELPERS_H_

#include <string>
#include <sstream>

#include "linphone/utils/general.h"

// =============================================================================

typedef struct belle_sip_source belle_sip_source_t;

L_DECL_C_STRUCT(LinphoneCore);

LINPHONE_BEGIN_NAMESPACE

/**
 * This interface aims at abstracting some features offered by the platform, most often mobile platforms.
 * A per platform implementation is to be made to implement these features, if available on the platform.
 */
class PlatformHelpers {
public:
	virtual ~PlatformHelpers () = default;

	LinphoneCore *getCore() const { return mCore; }

	virtual void acquireWifiLock () = 0;
	virtual void releaseWifiLock () = 0;
	virtual void acquireMcastLock () = 0;
	virtual void releaseMcastLock () = 0;
	virtual void acquireCpuLock () = 0;
	virtual void releaseCpuLock () = 0;

	virtual std::string getConfigPath () const = 0;
	virtual std::string getDataPath () const = 0;
	virtual std::string getDataResource (const std::string &filename) const = 0;
	virtual std::string getImageResource (const std::string &filename) const = 0;
	virtual std::string getRingResource (const std::string &filename) const = 0;
	virtual std::string getSoundResource (const std::string &filename) const = 0;

	virtual void setVideoPreviewWindow (void *windowId) = 0;
	virtual std::string getDownloadPath () = 0;
	virtual void setVideoWindow (void *windowId) = 0;

	// This method shall retrieve DNS server list from the platform and assign it to the core.
	virtual bool isNetworkReachable () = 0;
	virtual void onWifiOnlyEnabled (bool enabled) = 0;
	virtual void setDnsServers () = 0;
	virtual void setHttpProxy (std::string host, int port) = 0;
	virtual void setNetworkReachable (bool reachable) = 0;

	virtual void onLinphoneCoreStart (bool monitoringEnabled) = 0;

protected:
	inline explicit PlatformHelpers (LinphoneCore *lc) : mCore(lc) {}

	inline std::string getFilePath (const std::string &directory, const std::string &filename) const {
		std::ostringstream oss;
		oss << directory << "/" << filename;
		return oss.str();
	}

	LinphoneCore *mCore;
};

class GenericPlatformHelpers : public PlatformHelpers {
public:
	explicit GenericPlatformHelpers (LinphoneCore *lc);
	~GenericPlatformHelpers ();

	void acquireWifiLock () override;
	void releaseWifiLock () override;
	void acquireMcastLock () override;
	void releaseMcastLock () override;
	void acquireCpuLock () override;
	void releaseCpuLock () override;

	std::string getConfigPath () const override;
	std::string getDataPath () const override;
	std::string getDataResource (const std::string &filename) const override;
	std::string getImageResource (const std::string &filename) const override;
	std::string getRingResource (const std::string &filename) const override;
	std::string getSoundResource (const std::string &filename) const override;

	void setVideoPreviewWindow (void *windowId) override;
	void setVideoWindow (void *windowId) override;

	bool isNetworkReachable () override;
	void onWifiOnlyEnabled (bool enabled) override;
	void setDnsServers () override;
	void setHttpProxy (std::string host, int port) override;
	void setNetworkReachable (bool reachable) override;

	void onLinphoneCoreStart (bool monitoringEnabled) override;

private:
	static int monitorTimerExpired (void *data, unsigned int revents);

private:
	static constexpr int DefaultMonitorTimeout = 5;

	belle_sip_source_t *mMonitorTimer;
	bool mNetworkReachable = false;
	std::string getDownloadPath () override;
};

PlatformHelpers *createAndroidPlatformHelpers (LinphoneCore *lc, void *systemContext);
PlatformHelpers *createIosPlatformHelpers (LinphoneCore *lc, void *systemContext);

LINPHONE_END_NAMESPACE

#endif // indef _L_PLATFORM_HELPERS_H_
