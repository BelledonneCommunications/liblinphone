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

#include "alert.h"
#include "conference/session/media-session-p.h"
#include "linphone/api/c-alert.h"
#include "signal-information/signal-information.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------
Alert::Alert(LinphoneAlertType type) : mType(type), mStartTime(time(nullptr)) {
}

Alert::Alert(const Alert &other) : HybridObject(other) {
}

Alert::~Alert() {
}

Alert *Alert::clone() const {
	return new Alert(*this);
}

time_t Alert::getStartTime() const {
	return mStartTime;
}

time_t Alert::getEndTime() const {
	return mEndTime;
}

LinphoneAlertType Alert::getType() const {
	return mType;
}

std::shared_ptr<Dictionary> Alert::getInformations() const {
	return mInformations;
}

std::weak_ptr<Call> Alert::getCall() const {
	return mCall;
}

void Alert::setCall(const std::shared_ptr<Call> &call) {
	mCall = call;
}

bool Alert::getState() const {
	return mState;
}

void Alert::setState(bool state) {
	mState = state;
}

std::ostream &Alert::toStream(std::ostream &stream) const {
	try {
		stream << linphone_alert_type_to_string(mType) << " | ";
		std::shared_ptr<Call> call(mCall);
		auto op = call->getOp();
		string callId = op ? op->getCallId() : "<unknown>";
		stream << "Call-id :" << callId << " | ";
		stream << "From " << *call->getToAddress() << " | ";
		stream << "To " << *call->getLocalAddress();
	} catch (const bad_weak_ptr &) {
		stream << "Unknown call";
	}
	stream << endl;

	if (mInformations) {
		mInformations->toStream(stream);
	}

	return stream;
}

AlertTimer::AlertTimer(uint64_t delay) : mDelay(delay), mLastCheck(bctbx_get_cur_time_ms()) {
}

bool AlertTimer::isTimeout(bool autoreset) {

	uint64_t currentTime = bctbx_get_cur_time_ms();
	if (currentTime >= mDelay + mLastCheck) {
		if (autoreset) mLastCheck = currentTime;
		return true;
	}
	return false;
}

AlertMonitor::AlertMonitor(MediaSession &mediaSession) : mMediaSession(mediaSession) {
	mAlertsEnabled = linphone_core_alerts_enabled(mMediaSession.getCore()->getCCore());
}

void AlertMonitor::notify(const std::shared_ptr<Dictionary> &properties, LinphoneAlertType type) {
	auto alert = Alert::create(type);

	alert->mInformations = properties;
	mRunningAlerts[type] = alert;

	mMediaSession.notifyAlert(alert);

	linphone_core_notify_alert(mMediaSession.getCore()->getCCore(), alert->toC());

	lWarning() << *alert;
}

void AlertMonitor::handleAlert(LinphoneAlertType type,
                               bool triggerCondition,
                               const std::function<std::shared_ptr<Dictionary>()> &getInformationFunction) {
	if (!mTimers[type].isTimeout()) return;
	if (!alreadyRunning(type) && triggerCondition) {
		notify(getInformationFunction ? getInformationFunction() : nullptr, type);
		reset();
		return;
	}
	if (alreadyRunning(type) && !triggerCondition) {
		auto alert = mRunningAlerts[type];
		alert->setState(false);
		linphone_alert_notify_on_terminated(alert->toC());
		mRunningAlerts.erase(type);
		return;
	}
}

void AlertMonitor::getTimer(LinphoneAlertType type, const string &section, const string &key, int defaultDelay) {
	LinphoneConfig *config = linphone_core_get_config(mMediaSession.getCore()->getCCore());
	string completeSection = "alerts"s + "::" + section;
	int delay = linphone_config_get_int(config, completeSection.c_str(), key.c_str(), defaultDelay);
	mTimers[type] = AlertTimer((uint64_t)delay);
}

bool AlertMonitor::alreadyRunning(LinphoneAlertType type) {
	auto it = mRunningAlerts.find(type);
	return (it != mRunningAlerts.end());
}

bool AlertMonitor::getAlertsEnabled() {
	return mAlertsEnabled;
}

VideoQualityAlertMonitor::VideoQualityAlertMonitor(MediaSession &mediaSession)
    : AlertMonitor(mediaSession), mStalled(false) {
	getTimer(LinphoneAlertQoSLowQualitySentVideo, "camera"s, "quality_sent_interval"s, 1000);
	getTimer(LinphoneAlertQoSCameraMisfunction, "camera"s, "camera_misfunction_interval"s, 1000);
	getTimer(LinphoneAlertQoSCameraLowFramerate, "camera"s, "low_framerate_interval"s, 1000);
	getTimer(LinphoneAlertQoSVideoStalled, "camera"s, "video_stalled_interval"s, 1000);
	auto config = linphone_core_get_config(mMediaSession.getCore()->getCCore());
	mFpsThreshold = linphone_config_get_float(config, "alerts::camera", "fps_threshold", 10.0);
}

void VideoQualityAlertMonitor::check(const VideoControlInterface::VideoStats *sendStats,
                                     const VideoControlInterface::VideoStats *recvStats,
                                     const float fps) {
	if (!mAlertsEnabled) return;
	checkSendingLowQuality(sendStats);
	videoStalledCheck(recvStats->fps);
	if (fps >= 0) {
		checkCameraMisfunction(fps);
		checkCameraLowFramerate(fps);
	}
}

float VideoQualityAlertMonitor::getFpsThreshold() {
	return mFpsThreshold;
}

void VideoQualityAlertMonitor::videoStalledCheck(float fps) {
	mStalled = (fps <= 1.0f);
	handleAlert(LinphoneAlertQoSVideoStalled, mStalled);
}

void VideoQualityAlertMonitor::checkCameraMisfunction(float fps) {
	bool condition = (fps == 0);
	handleAlert(LinphoneAlertQoSCameraMisfunction, condition);
}

void VideoQualityAlertMonitor::checkCameraLowFramerate(float fps) {
	bool condition = ((fps > 0) && (fps <= getFpsThreshold()));
	handleAlert(LinphoneAlertQoSCameraLowFramerate, condition, [fps]() {
		auto properties = (new Dictionary())->toSharedPtr();
		properties->setProperty("fps", fps);
		return properties;
	});
}

void VideoQualityAlertMonitor::checkSendingLowQuality(const VideoControlInterface::VideoStats *stats) {
	bool condition = (stats->width <= 320 && stats->height <= 240);

	handleAlert(LinphoneAlertQoSLowQualitySentVideo, condition, [stats]() {
		auto properties = (new Dictionary())->toSharedPtr();
		properties->setProperty("fps", stats->fps);
		properties->setProperty("width", stats->width);
		properties->setProperty("height", stats->height);
		return properties;
	});
}

VideoQualityAlertMonitor::~VideoQualityAlertMonitor() {
}

VideoBandwidthAlertMonitor::VideoBandwidthAlertMonitor(MediaSession &mediaSession) : AlertMonitor(mediaSession) {
	auto config = linphone_core_get_config(mMediaSession.getCore()->getCCore());
	mThreshold = linphone_config_get_float(config, "alerts::video", "bandwidth_threshold", 150000.0);
	getTimer(LinphoneAlertQoSLowQualityReceivedVideo, "video"s, "low_quality_received_interval"s, 1000);
	getTimer(LinphoneAlertQoSLowDownloadBandwidthEstimation, "video"s, "download_bandwidth_interval"s, 1000);
}

void VideoBandwidthAlertMonitor::check(const shared_ptr<CallStats> &callStats) {
	if (!mAlertsEnabled) return;
	float bandwidth = callStats->getDownloadBandwidth();
	checkVideoBandwidth(bandwidth);
	float estimatedBandwidth = callStats->getEstimatedDownloadBandwidth();
	if (estimatedBandwidth != 0) {
		lInfo() << "Got video bandwidth estimation: " << estimatedBandwidth;
		checkBandwidthEstimation(estimatedBandwidth);
	}
}

float VideoBandwidthAlertMonitor::getBandwidthThreshold() {
	return mThreshold;
}

void VideoBandwidthAlertMonitor::checkVideoBandwidth(float bandwidth) {
	bool condition = (bandwidth > 0) && (bandwidth * 1000.0f <= getBandwidthThreshold());

	handleAlert(LinphoneAlertQoSLowQualityReceivedVideo, condition, [bandwidth]() {
		auto properties = (new Dictionary())->toSharedPtr();
		properties->setProperty("bandwidth", bandwidth);
		return properties;
	});
}

void VideoBandwidthAlertMonitor::checkBandwidthEstimation(float bandwidth) {
	bool condition = bandwidth * 1000.0f <= getBandwidthThreshold();

	handleAlert(LinphoneAlertQoSLowDownloadBandwidthEstimation, condition, [bandwidth]() {
		auto properties = (new Dictionary())->toSharedPtr();
		properties->setProperty("bandwidth", bandwidth);
		return properties;
	});
}

NetworkQualityAlertMonitor::NetworkQualityAlertMonitor(MediaSession &mediaSession) : AlertMonitor(mediaSession) {
	getTimer(LinphoneAlertQoSHighLossLateRate, "network"s, "loss_rate_interval"s, 5000);
	getTimer(LinphoneAlertQoSHighRemoteLossRate, "network"s, "remote_loss_rate_interval"s, 5000);
	getTimer(LinphoneAlertQoSLostSignal, "network"s, "lost_signal_interval"s, 1000);
	getTimer(LinphoneAlertQoSBurstOccured, "network"s, "burst_occured_interval"s, 1000);
	getTimer(LinphoneAlertQoSRetransmissionFailures, "network"s, "nack_check_interval"s, 2000);
	getTimer(LinphoneAlertQoSLowSignal, "network"s, "low_signal_interval"s, 1000);
	auto config = linphone_core_get_config(mMediaSession.getCore()->getCCore());
	mLossRateThreshold = linphone_config_get_float(config, "alerts::network", "loss_rate_threshold", 5.0);
	mNackPerformanceThreshold = linphone_config_get_float(config, "alerts::network", "nack_threshold", 0.5f);
	mSignalThreshold = linphone_config_get_float(config, "alerts::network", "signal_threshold", -70.0f);
}

float NetworkQualityAlertMonitor::getLossRateThreshold() {
	return mLossRateThreshold;
}

void NetworkQualityAlertMonitor::check(const shared_ptr<CallStats> &callStats, bool burstOccured) {
	if (!mAlertsEnabled) return;
	float localLossRate = callStats->getLocalLossRate();
	float localLateRate = callStats->getLocalLateRate();

	float remoteLossRate = callStats->getReceiverLossRate();
	checkRemoteLossRate(remoteLossRate);
	checkLocalLossRate(localLossRate, localLateRate, LinphoneStreamTypeAudio);
	checkLostSignal();
	checkBurstOccurence(burstOccured);
	checkSignalQuality();
}

void NetworkQualityAlertMonitor::checkLocalLossRate(float lossRate, float lateRate, LinphoneStreamType streamType) {
	bool condition = (lossRate >= getLossRateThreshold());

	handleAlert(LinphoneAlertQoSHighLossLateRate, condition, [lossRate, lateRate, streamType]() {
		auto properties = (new Dictionary())->toSharedPtr();
		properties->setProperty("loss-rate", lossRate);
		properties->setProperty("late-rate", lateRate);
		properties->setProperty("media-type", streamType);
		return properties;
	});
}

void NetworkQualityAlertMonitor::checkRemoteLossRate(float receivedLossRate) {
	bool condition = (receivedLossRate >= getLossRateThreshold());

	handleAlert(LinphoneAlertQoSHighRemoteLossRate, condition, [receivedLossRate]() {
		auto properties = (new Dictionary())->toSharedPtr();
		properties->setProperty("loss-rate", receivedLossRate);
		return properties;
	});
}

void NetworkQualityAlertMonitor::checkLostSignal() {
	bool condition = !linphone_core_is_network_reachable(mMediaSession.getCore()->getCCore());
	handleAlert(LinphoneAlertQoSLostSignal, condition);
}

void NetworkQualityAlertMonitor::reset() {
	mBurstCount = 0;
}

void NetworkQualityAlertMonitor::checkBurstOccurence(const bool burstOccured) {
	mBurstCount += burstOccured;
	bool condition = (mBurstCount > 0);
	handleAlert(LinphoneAlertQoSBurstOccured, condition);
}

float NetworkQualityAlertMonitor::computeNackIndicator(uint64_t lostBeforeNack, uint64_t cumPacketLoss) {
	if (cumPacketLoss > lostBeforeNack) return 0.0f;
	if (lostBeforeNack == 0U) return 1.0f;
	return (float)(lostBeforeNack - cumPacketLoss) / (float)lostBeforeNack;
}

void NetworkQualityAlertMonitor::checkNackQuality(RtpSession *session) {

	if (!mNackSent) return;
	uint64_t currentNackLoss = session->stats.loss_before_nack;
	uint64_t currentTotalLoss = (uint64_t)session->stats.cum_packet_loss;
	if (currentNackLoss != 0 && !mFirstMeasureNonZero) {
		mFirstMeasureNonZero = true;
		mLastNackLoss = currentNackLoss;
		mLastTotalLoss = currentTotalLoss;
	}
	if (mFirstMeasureNonZero && mTimers[LinphoneAlertQoSRetransmissionFailures].isTimeout(false)) {
		mNackIndicator = computeNackIndicator(currentNackLoss - mLastNackLoss, currentTotalLoss - mLastTotalLoss);
		mLastNackLoss = currentNackLoss;
		mLastTotalLoss = currentTotalLoss;
		handleAlert(LinphoneAlertQoSRetransmissionFailures, mNackIndicator <= mNackPerformanceThreshold, [this]() {
			auto properties = (new Dictionary())->toSharedPtr();
			properties->setProperty("nack-performance", mNackIndicator);
			return properties;
		});
	}
}

void NetworkQualityAlertMonitor::checkSignalQuality() {
	bool condition = false;
	float value = 0.0f;

	auto information = mMediaSession.getCore()->getSignalInformation();
	if (information) {
		value = information->getStrength();
		condition = (value <= mSignalThreshold);
	}
	handleAlert(LinphoneAlertQoSLowSignal, condition, [value, information]() {
		auto properties = (new Dictionary())->toSharedPtr();
		properties->setProperty("rssi-value", value);
		properties->setProperty("network-type", SignalInformation::signalTypeToString(information->getSignalType()));
		return properties;
	});
}

LINPHONE_END_NAMESPACE
