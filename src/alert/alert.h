/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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
#ifndef ALERT_H
#define ALERT_H

#include "call/call.h"
#include "dictionary/dictionary.h"
#include "linphone/api/c-types.h"
#include <belle-sip/object++.hh>
#include <memory>
#include <unordered_map>

using namespace std;

LINPHONE_BEGIN_NAMESPACE
class AlertCbs;

class LINPHONE_PUBLIC Alert : public bellesip::HybridObject<LinphoneAlert, Alert>, public CallbacksHolder<AlertCbs> {

public:
	Alert(){};
	Alert(const Alert &other);
	virtual ~Alert();
	Alert *clone() const override;

	Alert(std::shared_ptr<Call> &call, LinphoneAlertType type);
	time_t getStartTime() const;
	time_t getEndTime() const;
	LinphoneAlertType getType() const;
	shared_ptr<Dictionary> getInformations() const;
	weak_ptr<Call> getCall() const;
	bool getState() const;
	void setState(const bool state);
	std::ostream &toStream(std::ostream &stream) const;
	shared_ptr<Dictionary> mInformations;

private:
	std::weak_ptr<Call> mCall;
	LinphoneAlertType mType;
	time_t mStartTime;
	time_t mEndTime;
	bool mState;
};
inline std::ostream &operator<<(ostream &stream, const Alert &alert) {
	return alert.toStream(stream);
}

class AlertTimer {
public:
	AlertTimer(){};
	AlertTimer(uint64_t delay);
	bool isTimeout(bool autoreset = true);

private:
	uint64_t mDelay;
	uint64_t mLastCheck;
};
class AlertMonitor : public CoreAccessor {

public:
	AlertMonitor(const std::shared_ptr<Core> &core);
	void notify(const std::shared_ptr<Dictionary> &properties, LinphoneAlertType);
	bool alreadyRunning(LinphoneAlertType type);
	void handleAlert(LinphoneAlertType type, const std::shared_ptr<Dictionary> &properties, bool triggerCondition);
	void getTimer(LinphoneAlertType type, string section, string key, int delay);
	virtual void reset(){};
	bool getAlertsEnabled();

protected:
	bool mAlertsEnabled;
	std::unordered_map<LinphoneAlertType, AlertTimer> mTimers;
	std::unordered_map<LinphoneAlertType, std::shared_ptr<Alert>> mRunningAlerts;
};

class VideoQualityAlertMonitor : public AlertMonitor {
public:
	VideoQualityAlertMonitor(const std::shared_ptr<Core> &core);
	float getFpsThreshold();
	void checkSendingLowQuality(const VideoControlInterface::VideoStats *stats);
	void videoStalledCheck(float fps);
	void checkCameraMisfunction(float fps);
	void checkCameraLowFramerate(float fps);
	void check(const VideoControlInterface::VideoStats *, const VideoControlInterface::VideoStats *, const float);
	~VideoQualityAlertMonitor();

private:
	bool mStalled;
};
class VideoBandwidthAlertMonitor : public AlertMonitor {

public:
	VideoBandwidthAlertMonitor(const std::shared_ptr<Core> &core);
	float getBandwidthThreshold();
	void check(LinphoneCallStats *callStats);
	void checkVideoBandwidth(float bandwidth);
	void checkBandwidthEstimation(float bandwidth);
};

class NetworkQualityAlertMonitor : public AlertMonitor {

public:
	NetworkQualityAlertMonitor(const std::shared_ptr<Core> &core);
	float getLossRateThreshold();
	void checkRemoteLossRate(float);
	void checkLocalLossRate(float lossRate, float lateRate, LinphoneStreamType streamType);
	void checkLostSignal();
	void checkBurstOccurence(const bool burstOccured);
	void checkNackQuality(RtpSession *session);
	void check(LinphoneCallStats *, bool);
	void reset() override;
	float computeNackIndicator(uint64_t lostBeforeNack, uint64_t cumPacketLoss);
	void checkSignalQuality();
	bool mNackSent;

private:
	int mBurstCount;
	bool mFirstMeasureNonZero;
	uint64_t mLastNackLoss;
	uint64_t mLastTotalLoss;
	float mNackIndicator;
};

class AlertCbs : public bellesip::HybridObject<LinphoneAlertCbs, AlertCbs>, public Callbacks {
public:
	LinphoneAlertCbsOnTerminatedCb getOnTerminated() const {
		return mOnTerminated;
	};
	void setOnTerminated(LinphoneAlertCbsOnTerminatedCb onTerminated) {
		mOnTerminated = onTerminated;
	};

private:
	LinphoneAlertCbsOnTerminatedCb mOnTerminated = nullptr;
};
LINPHONE_END_NAMESPACE

#endif