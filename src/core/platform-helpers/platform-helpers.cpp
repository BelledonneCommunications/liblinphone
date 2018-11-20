/*
 * platform-helpers.cpp
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

#include "logger/logger.h"
#include "platform-helpers.h"

// TODO: Remove me.
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

GenericPlatformHelpers::GenericPlatformHelpers (LinphoneCore *lc) : PlatformHelpers(lc), mMonitorTimer(nullptr) {}

GenericPlatformHelpers::~GenericPlatformHelpers () {
	if (mMonitorTimer) {
		if (mCore && mCore->sal) mCore->sal->cancelTimer(mMonitorTimer);
		belle_sip_object_unref(mMonitorTimer);
		mMonitorTimer = nullptr;
	}
}

void GenericPlatformHelpers::setDnsServers () {}

void GenericPlatformHelpers::acquireWifiLock () {}

void GenericPlatformHelpers::releaseWifiLock () {}

void GenericPlatformHelpers::acquireMcastLock () {}

void GenericPlatformHelpers::releaseMcastLock () {}

void GenericPlatformHelpers::acquireCpuLock () {}

void GenericPlatformHelpers::releaseCpuLock () {}

string GenericPlatformHelpers::getDataPath () {
	return "";
}

string GenericPlatformHelpers::getConfigPath () {
	return "";
}

void GenericPlatformHelpers::setVideoPreviewWindow (void *windowId) {}

void GenericPlatformHelpers::setVideoWindow (void *windowId) {}

void GenericPlatformHelpers::setNetworkReachable (bool reachable) {
	mNetworkReachable = reachable;
	linphone_core_set_network_reachable_internal(mCore, reachable);
}

bool GenericPlatformHelpers::isNetworkReachable () {
	return mNetworkReachable;
}

void GenericPlatformHelpers::onLinphoneCoreReady (bool monitoringEnabled) {
	if (!monitoringEnabled) return;

	if (!mMonitorTimer) {
		mMonitorTimer = mCore->sal->createTimer(monitorTimerExpired, this,
			mDefaultMonitorTimeout * 1000, "monitor network timeout");
	} else {
		belle_sip_source_set_timeout(mMonitorTimer, mDefaultMonitorTimeout * 1000);
	}
}

void GenericPlatformHelpers::onWifiOnlyEnabled (bool enabled) {}

void GenericPlatformHelpers::setHttpProxy (string host, int port) {}

int GenericPlatformHelpers::monitorTimerExpired (void *data, unsigned int revents) {
	GenericPlatformHelpers *helper = static_cast<GenericPlatformHelpers *>(data);
	LinphoneCore *core = helper->getCore();

	char newIp[LINPHONE_IPADDR_SIZE];
	linphone_core_get_local_ip(core, AF_UNSPEC, nullptr, newIp);

	bool status = strcmp(newIp,"::1") != 0 && strcmp(newIp,"127.0.0.1") != 0;
	if (status && core->network_last_status && strcmp(newIp, core->localip) != 0) {
		lInfo() << "IP address change detected";
		helper->setNetworkReachable(false);
		core->network_last_status = FALSE;
	}

	strncpy(core->localip, newIp, sizeof core->localip);

	if (bool_t(status) != core->network_last_status) {
		if (status) {
			lInfo() << "New local ip address is " << core->localip;
		}
		helper->setNetworkReachable(status);
		core->network_last_status = status;
	}

	return BELLE_SIP_CONTINUE;
}

LINPHONE_END_NAMESPACE
