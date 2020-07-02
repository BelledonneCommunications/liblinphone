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

#include "logger/logger.h"
#include "platform-helpers.h"

// TODO: Remove me.
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

GenericPlatformHelpers::GenericPlatformHelpers (std::shared_ptr<LinphonePrivate::Core> core) : PlatformHelpers(core), mMonitorTimer(nullptr) {
	mSharedCoreHelpers = make_shared<GenericSharedCoreHelpers>(core);
}

GenericPlatformHelpers::~GenericPlatformHelpers () {
	if (mMonitorTimer) {
		if (getCore()->getCCore() && getCore()->getCCore()->sal) getCore()->getCCore()->sal->cancelTimer(mMonitorTimer);
		belle_sip_object_unref(mMonitorTimer);
		mMonitorTimer = nullptr;
	}
}


void GenericPlatformHelpers::acquireWifiLock () {}

void GenericPlatformHelpers::releaseWifiLock () {}

void GenericPlatformHelpers::acquireMcastLock () {}

void GenericPlatformHelpers::releaseMcastLock () {}

void GenericPlatformHelpers::acquireCpuLock () {}

void GenericPlatformHelpers::releaseCpuLock () {}


string GenericPlatformHelpers::getConfigPath () const {
	return "";
}

string GenericPlatformHelpers::getDataPath () const {
	return "";
}

string GenericPlatformHelpers::getDataResource (const string &filename) const {
	return getFilePath(
		linphone_factory_get_data_resources_dir(linphone_factory_get()),
		filename
	);
}

string GenericPlatformHelpers::getImageResource (const string &filename) const {
	return getFilePath(
		linphone_factory_get_image_resources_dir(linphone_factory_get()),
		filename
	);
}

string GenericPlatformHelpers::getRingResource (const string &filename) const {
	return getFilePath(
		linphone_factory_get_ring_resources_dir(linphone_factory_get()),
		filename
	);
}

string GenericPlatformHelpers::getSoundResource (const string &filename) const {
	return getFilePath(
		linphone_factory_get_sound_resources_dir(linphone_factory_get()),
		filename
	);
}

void *GenericPlatformHelpers::getPathContext () {
	return nullptr;
}

void GenericPlatformHelpers::setVideoPreviewWindow (void *windowId) {}

void GenericPlatformHelpers::setVideoWindow (void *windowId) {}

void GenericPlatformHelpers::resizeVideoPreview (int width, int height) {}

bool GenericPlatformHelpers::isNetworkReachable () {
	return mNetworkReachable;
}

void GenericPlatformHelpers::onWifiOnlyEnabled (bool enabled) {}

void GenericPlatformHelpers::setDnsServers () {}

void GenericPlatformHelpers::setHttpProxy (const string &host, int port) {}

string GenericPlatformHelpers::getWifiSSID() { return mCurrentSSID; }

void GenericPlatformHelpers::setWifiSSID(const string &ssid) { mCurrentSSID = ssid; }

void GenericPlatformHelpers::setNetworkReachable (bool reachable) {
	mNetworkReachable = reachable;
	linphone_core_set_network_reachable_internal(getCore()->getCCore(), reachable);
}

bool GenericPlatformHelpers::startNetworkMonitoring() { return true; }

void GenericPlatformHelpers::stopNetworkMonitoring() {}

void GenericPlatformHelpers::onLinphoneCoreStart (bool monitoringEnabled) {
	if (!monitoringEnabled) return;

	if (!mMonitorTimer) {
		mMonitorTimer = getCore()->getCCore()->sal->createTimer(
			monitorTimerExpired,
			this,
			DefaultMonitorTimeout * 1000,
			"monitor network timeout"
		);
	} else {
		belle_sip_source_set_timeout(mMonitorTimer, DefaultMonitorTimeout * 1000);
	}

	// Get ip right now to avoid waiting for 5s
	monitorTimerExpired(this, 0);
}

void GenericPlatformHelpers::onLinphoneCoreStop () {}

void GenericPlatformHelpers::startAudioForEchoTestOrCalibration () { }

void GenericPlatformHelpers::stopAudioForEchoTestOrCalibration () { }

int GenericPlatformHelpers::monitorTimerExpired (void *data, unsigned int revents) {
	GenericPlatformHelpers *helper = static_cast<GenericPlatformHelpers *>(data);
	LinphoneCore *core = helper->getCore()->getCCore();
	bool ipv6Enabled = linphone_core_ipv6_enabled(core);

	char newIp4[LINPHONE_IPADDR_SIZE];
	char newIp6[LINPHONE_IPADDR_SIZE];
	linphone_core_get_local_ip(core, AF_INET, nullptr, newIp4);
	if (ipv6Enabled)
		linphone_core_get_local_ip(core, AF_INET6, nullptr, newIp6);

	bool status = strcmp(newIp6,"::1") != 0 || strcmp(newIp4,"127.0.0.1") != 0;
	if (status && core->network_last_status){
		bool ipChanged = false;
		// Check for IP address changes:
		if (strcmp(newIp4, core->localip4) != 0) {
			lInfo() << "IPv4 address change detected";
			ipChanged = true;
		}
		if (ipv6Enabled && strcmp(newIp6, core->localip6) != 0) {
			lInfo() << "IPv6 address change detected";
			ipChanged = true;
		}
		if (ipChanged){
			helper->setNetworkReachable(false);
			core->network_last_status = FALSE;
		}
	}

	strncpy(core->localip4, newIp4, sizeof core->localip4);
	if (ipv6Enabled) strncpy(core->localip6, newIp6, sizeof core->localip6);

	if (bool_t(status) != core->network_last_status) {
		if (status) {
			lInfo() << "Default local ipv4 address is " << core->localip4;
			if (ipv6Enabled) lInfo() << "Default local ipv6 address is " << core->localip6;
		}
		helper->setNetworkReachable(status);
		core->network_last_status = status;
	}

	return BELLE_SIP_CONTINUE;
}

string GenericPlatformHelpers::getDownloadPath () {
	return "";
}

std::shared_ptr<SharedCoreHelpers> GenericPlatformHelpers::getSharedCoreHelpers() {
	return mSharedCoreHelpers;
}

void GenericPlatformHelpers::createAppDelegate() {
}

void GenericPlatformHelpers::destroyAppDelegate() {
}

void GenericPlatformHelpers::dummyConfigAudioSession() {
}

LINPHONE_END_NAMESPACE
