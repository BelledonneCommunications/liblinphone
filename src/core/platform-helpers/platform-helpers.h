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

#ifndef _L_PLATFORM_HELPERS_H_
#define _L_PLATFORM_HELPERS_H_

#include <sstream>
#include <string>

#include <bctoolbox/defs.h>

#include "chat/chat-message/chat-message.h"
#include "chat/chat-room/chat-room.h"
#include "core/core-accessor.h"
#include "core/core.h"
#include "core/shared-core-helpers/shared-core-helpers.h"
#include "linphone/api/c-participant-device.h"
#include "linphone/utils/general.h"

// =============================================================================

typedef struct belle_sip_source belle_sip_source_t;

L_DECL_C_STRUCT(LinphoneCore);

LINPHONE_BEGIN_NAMESPACE

class Core;
class SignalInformation;

/**
 * This interface aims at abstracting some features offered by the platform, most often mobile platforms.
 * A per platform implementation is to be made to implement these features, if available on the platform.
 */
class PlatformHelpers : public CoreAccessor {
public:
	virtual ~PlatformHelpers() = default;

	virtual void acquireWifiLock() = 0;
	virtual void releaseWifiLock() = 0;
	virtual void acquireMcastLock() = 0;
	virtual void releaseMcastLock() = 0;
	virtual void acquireCpuLock() = 0;
	virtual void releaseCpuLock() = 0;

	virtual std::string getConfigPath() const = 0;
	virtual std::string getDataPath() const = 0;
	virtual std::string getPluginsDir() const = 0;
	virtual std::string getDataResource(const std::string &filename) const = 0;
	virtual std::string getImageResource(const std::string &filename) const = 0;
	virtual std::string getRingResource(const std::string &filename) const = 0;
	virtual std::string getSoundResource(const std::string &filename) const = 0;
	virtual void *getPathContext() = 0;

	enum NetworkType { Unknown, Wifi, MobileData };
	virtual NetworkType getNetworkType() const = 0;
	virtual std::string getWifiSSID() = 0;
	virtual void setWifiSSID(const std::string &ssid) = 0;
	virtual void setSignalInformation(std::shared_ptr<SignalInformation> &signalInformation) = 0;
	virtual std::shared_ptr<SignalInformation> getSignalInformation() = 0;

	virtual void setVideoPreviewWindow(void *windowId) = 0;
	virtual std::string getDownloadPath() = 0;
	virtual void setVideoWindow(void *windowId) = 0;
	virtual void setParticipantDeviceVideoWindow(LinphoneParticipantDevice *participantDevice, void *windowId) = 0;
	virtual void resizeVideoPreview(int width, int height) = 0;

	// This method shall retrieve DNS server list from the platform and assign it to the core.
	virtual bool isNetworkReachable() = 0;
	virtual void updateNetworkReachability() = 0;
	virtual bool isActiveNetworkWifiOnlyCompliant() const = 0;
	virtual void onWifiOnlyEnabled(bool enabled) = 0;
	virtual void setDnsServers() = 0;
	virtual void setHttpProxy(const std::string &host, int port) = 0;
	virtual void setNetworkReachable(bool reachable) = 0;

	virtual bool startNetworkMonitoring() = 0;
	virtual void stopNetworkMonitoring() = 0;
	virtual void startPushService() = 0;
	virtual void stopPushService() = 0;
	virtual void startFileTransferService() = 0;
	virtual void stopFileTransferService() = 0;

	virtual void onLinphoneCoreStart(bool monitoringEnabled) = 0;
	virtual void onLinphoneCoreStop() = 0;

	virtual std::shared_ptr<SharedCoreHelpers> getSharedCoreHelpers() = 0;

	virtual void startAudioForEchoTestOrCalibration() = 0;
	virtual void stopAudioForEchoTestOrCalibration() = 0;
	virtual void routeAudioToSpeaker() = 0;
	virtual void restorePreviousAudioRoute() = 0;

	virtual void start(std::shared_ptr<LinphonePrivate::Core> core) = 0;
	virtual void stop(void) = 0;

	virtual void didRegisterForRemotePush(void *token) = 0;
	virtual void didRegisterForRemotePushWithStringifiedToken(const char *tokenStr) = 0;
	virtual void setPushAndAppDelegateDispatchQueue(void *dispatch_queue) = 0;
	virtual void enableAutoIterate(bool autoIterateEnabled) = 0;

	virtual void onRecordingStarted() const = 0;
	virtual void onRecordingPaused() const = 0;
	virtual bool isRingingAllowed() const = 0;
	virtual void stopRinging() const = 0;

	virtual void setDeviceRotation(int orientation) const = 0;

protected:
	inline explicit PlatformHelpers(std::shared_ptr<LinphonePrivate::Core> core) : CoreAccessor(core) {
	}

	inline std::string getFilePath(const std::string &directory, const std::string &filename) const {
		std::ostringstream oss;
		oss << directory << "/" << filename;
		return oss.str();
	}
};

class GenericPlatformHelpers : public PlatformHelpers {
public:
	explicit GenericPlatformHelpers(std::shared_ptr<LinphonePrivate::Core> core);
	~GenericPlatformHelpers();

	void acquireWifiLock() override;
	void releaseWifiLock() override;
	void acquireMcastLock() override;
	void releaseMcastLock() override;
	void acquireCpuLock() override;
	void releaseCpuLock() override;

	std::string getConfigPath() const override;
	std::string getDataPath() const override;
	std::string getPluginsDir() const override;
	std::string getDataResource(const std::string &filename) const override;
	std::string getImageResource(const std::string &filename) const override;
	std::string getRingResource(const std::string &filename) const override;
	std::string getSoundResource(const std::string &filename) const override;
	void *getPathContext() override;

	NetworkType getNetworkType() const override;
	std::string getWifiSSID() override;
	void setWifiSSID(const std::string &ssid) override;
	void setSignalInformation(std::shared_ptr<SignalInformation> &signalInformation) override;
	std::shared_ptr<SignalInformation> getSignalInformation() override;
	void setVideoPreviewWindow(void *windowId) override;
	void setVideoWindow(void *windowId) override;
	void setParticipantDeviceVideoWindow(LinphoneParticipantDevice *participantDevice, void *windowId) override;
	void resizeVideoPreview(int width, int height) override;

	bool isNetworkReachable() override;
	void updateNetworkReachability() override;
	bool isActiveNetworkWifiOnlyCompliant() const override;
	void onWifiOnlyEnabled(bool enabled) override;
	void setDnsServers() override;
	void setHttpProxy(const std::string &host, int port) override;
	void setNetworkReachable(bool reachable) override;

	bool startNetworkMonitoring() override;
	void stopNetworkMonitoring() override;
	void startPushService() override;
	void stopPushService() override;
	void startFileTransferService() override;
	void stopFileTransferService() override;

	void onLinphoneCoreStart(bool monitoringEnabled) override;
	void onLinphoneCoreStop() override;

	std::shared_ptr<SharedCoreHelpers> getSharedCoreHelpers() override;

	void startAudioForEchoTestOrCalibration() override;
	void stopAudioForEchoTestOrCalibration() override;
	void routeAudioToSpeaker() override;
	void restorePreviousAudioRoute() override;

	void start(BCTBX_UNUSED(std::shared_ptr<LinphonePrivate::Core> core)) override{};
	void stop(void) override{};

	void didRegisterForRemotePush(BCTBX_UNUSED(void *token)) override{};
	void didRegisterForRemotePushWithStringifiedToken(BCTBX_UNUSED(const char *tokenStr)) override{};
	void setPushAndAppDelegateDispatchQueue(BCTBX_UNUSED(void *dispatch_queue)) override{};
	void enableAutoIterate(BCTBX_UNUSED(bool autoIterateEnabled)) override{};

	void onRecordingStarted() const override{};
	void onRecordingPaused() const override{};
	bool isRingingAllowed() const override {
		return true;
	};
	void stopRinging() const override{};
	void setDeviceRotation(BCTBX_UNUSED(int orientation)) const override{};

protected:
	bool checkIpAddressChanged();
	std::shared_ptr<SharedCoreHelpers> mSharedCoreHelpers;
	std::string mCurrentSSID;
	std::string mHttpProxyHost;
	int mHttpProxyPort;
	bool mNetworkReachable = true;
	bool mWifiOnly = false;
	bool mHttpProxyEnabled = false;

private:
	static int monitorTimerExpired(void *data, unsigned int revents);
	static constexpr int DefaultMonitorTimeout = 5;
	belle_sip_source_t *mMonitorTimer;
	std::string getDownloadPath() override;
};

PlatformHelpers *createAndroidPlatformHelpers(std::shared_ptr<LinphonePrivate::Core> core, void *systemContext);
PlatformHelpers *createIosPlatformHelpers(std::shared_ptr<LinphonePrivate::Core> core, void *systemContext);
PlatformHelpers *createMacPlatformHelpers(std::shared_ptr<LinphonePrivate::Core> core, void *systemContext);

LINPHONE_END_NAMESPACE

#endif // indef _L_PLATFORM_HELPERS_H_
