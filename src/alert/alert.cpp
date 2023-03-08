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
#include "signal-information/signal-information.h"
// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

static string linphone_alert_type_to_string(LinphoneAlertType type) {
	switch (type) {
		case LinphoneAlertQoSCameraMisfunction:
			return "LinphoneAlertQoSCameraMisfunction";
		case LinphoneAlertQoSCameraLowFramerate:
			return "LinphoneAlertQoSCameraLowFramerate";
		case LinphoneAlertQoSVideoStalled:
			return "LinphoneAlertQoSVideoStalled";
		case LinphoneAlertQoSHighLossLateRate:
			return "LinphoneAlertQoSHighLossLateRate";
		case LinphoneAlertQoSHighRemoteLossRate:
			return "LinphoneAlertQoSHighRemoteLossRate";
		case LinphoneAlertQoSRetransmissionFailures:
			return "LinphoneAlertQoSRetransmissionFailures";
		case LinphoneAlertQoSLowDownloadBandwidthEstimation:
			return "LinphoneAlertQoSLowDownloadBandwidthEstimation";
		case LinphoneAlertQoSLowQualityReceivedVideo:
			return "LinphoneAlertQoSLowQualityReceivedVideo";
		case LinphoneAlertQoSLowQualitySentVideo:
			return "LinphoneAlertQoSLowQualitySentVideo";
		case LinphoneAlertQoSLowSignal:
			return "LinphoneAlertQoSLowSignal";
		case LinphoneAlertQoSLostSignal:
			return "LinphoneAlertQoSLostSignal";
		case LinphoneAlertQoSBurstOccured:
			return "LinphoneAlertQoSBurstOccured";
	}
	return "LinphoneAlertType not found";
}
// -----------------------------------------------------------------------------
Alert::Alert(const Alert &other) : HybridObject(other) {
}
Alert::Alert(std::shared_ptr<Call> &call, LinphoneAlertType type) : mType(type) {

	mCall = call;
	mStartTime = time(NULL);
	mInformations = nullptr;
	mState = true;
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
bool Alert::getState() const {
	return mState;
}
void Alert::setState(const bool state) {
	mState = state;
}
std::ostream &Alert::toStream(std::ostream &stream) const {
	auto call = mCall.lock();
	stream << linphone_alert_type_to_string(mType) << " | ";
	stream << "Call-id :" << call->getOp()->getCallId() << " | ";
	stream << "From " << call->getToAddress()->asString() << " | ";
	stream << "To " << call->getLocalAddress()->asString() << endl;
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

AlertMonitor::AlertMonitor(const std::shared_ptr<Core> &core) : CoreAccessor(core) {
	mAlertsEnabled = linphone_core_alerts_enabled(getCore()->getCCore());
}

void AlertMonitor::notify(const std::shared_ptr<Dictionary> &properties, LinphoneAlertType type) {
	auto call = getCore()->getCurrentCall();
	auto alert = (new Alert(call, type))->toSharedPtr();
	alert->mInformations = properties;
	mRunningAlerts[type] = alert;
	linphone_core_notify_alert(getCore()->getCCore(), alert->toC());
	lWarning() << *alert;
}
void AlertMonitor::handleAlert(LinphoneAlertType type,
                               const std::shared_ptr<Dictionary> &properties,
                               bool triggerCondition) {
	if (!mTimers[type].isTimeout()) return;
	if (!alreadyRunning(type) && triggerCondition) {
		notify(properties, type);
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
void AlertMonitor::getTimer(LinphoneAlertType type, string section, string key, int defaultDelay) {
	LinphoneConfig *config = linphone_core_get_config(getCore()->getCCore());
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
VideoQualityAlertMonitor::VideoQualityAlertMonitor(const std::shared_ptr<Core> &core)
    : AlertMonitor(core), mStalled(false) {
	getTimer(LinphoneAlertQoSLowQualitySentVideo, "camera"s, "quality_sent_interval"s, 1000);
	getTimer(LinphoneAlertQoSCameraMisfunction, "camera"s, "camera_misfunction_interval"s, 1000);
	getTimer(LinphoneAlertQoSCameraLowFramerate, "camera"s, "low_framerate_interval"s, 1000);
	getTimer(LinphoneAlertQoSVideoStalled, "camera"s, "video_stalled_interval"s, 1000);
}
void VideoQualityAlertMonitor::check(const VideoControlInterface::VideoStats *sendStats,
                                     const VideoControlInterface::VideoStats *recvStats,
                                     const float fps) {
	if (!mAlertsEnabled) return;
	checkSendingLowQuality(sendStats);
	videoStalledCheck(recvStats->fps);
	checkCameraMisfunction(fps);
	checkCameraLowFramerate(fps);
}
float VideoQualityAlertMonitor::getFpsThreshold() {
	auto config = linphone_core_get_config(getCore()->getCCore());
	return linphone_config_get_float(config, "alerts::camera", "fps_threshold", 10.0);
}
void VideoQualityAlertMonitor::videoStalledCheck(float fps) {
	mStalled = (fps <= 1.0f);
	handleAlert(LinphoneAlertQoSVideoStalled, nullptr, mStalled);
}
void VideoQualityAlertMonitor::checkCameraMisfunction(float fps) {
	bool condition = (fps == 0);
	handleAlert(LinphoneAlertQoSCameraMisfunction, nullptr, condition);
}
void VideoQualityAlertMonitor::checkCameraLowFramerate(float fps) {
	bool condition = ((fps > 0) && (fps <= getFpsThreshold()));
	auto properties = (new Dictionary())->toSharedPtr();
	properties->setProperty("fps", fps);
	handleAlert(LinphoneAlertQoSCameraLowFramerate, properties, condition);
}
void VideoQualityAlertMonitor::checkSendingLowQuality(const VideoControlInterface::VideoStats *stats) {
	bool condition = (stats->width <= 320 && stats->height <= 240);
	auto properties = (new Dictionary())->toSharedPtr();
	properties->setProperty("fps", stats->fps);
	properties->setProperty("width", stats->width);
	properties->setProperty("height", stats->height);
	handleAlert(LinphoneAlertQoSLowQualitySentVideo, properties, condition);
}

VideoQualityAlertMonitor::~VideoQualityAlertMonitor() {
}
VideoBandwidthAlertMonitor::VideoBandwidthAlertMonitor(const std::shared_ptr<Core> &core) : AlertMonitor(core) {
	getTimer(LinphoneAlertQoSLowQualityReceivedVideo, "video"s, "low_quality_recieved_interval"s, 1000);
	getTimer(LinphoneAlertQoSLowDownloadBandwidthEstimation, "video"s, "download_bandwidth_interval"s, 1000);
}
void VideoBandwidthAlertMonitor::check(LinphoneCallStats *callStats) {
	if (!mAlertsEnabled) return;
	float bandwidth = linphone_call_stats_get_download_bandwidth(callStats);
	checkVideoBandwidth(bandwidth);
	float estimatedBandwidth = linphone_call_stats_get_estimated_download_bandwidth(callStats);
	checkBandwidthEstimation(estimatedBandwidth * 1000.0f);
}
float VideoBandwidthAlertMonitor::getBandwidthThreshold() {
	auto config = linphone_core_get_config(getCore()->getCCore());
	return linphone_config_get_float(config, "alerts::video", "bandwidth_threshold", 150000.0);
}
void VideoBandwidthAlertMonitor::checkVideoBandwidth(float bandwidth) {
	bool condition = (bandwidth > 0) && (bandwidth * 1000.0f <= getBandwidthThreshold());
	auto properties = (new Dictionary())->toSharedPtr();
	properties->setProperty("bandwidth", bandwidth);
	handleAlert(LinphoneAlertQoSLowQualityReceivedVideo, properties, condition);
}
void VideoBandwidthAlertMonitor::checkBandwidthEstimation(float bandwidth) {
	bool condition = bandwidth <= getBandwidthThreshold();
	auto properties = (new Dictionary())->toSharedPtr();
	properties->setProperty("bandwidth", bandwidth);
	handleAlert(LinphoneAlertQoSLowDownloadBandwidthEstimation, properties, condition);
}
NetworkQualityAlertMonitor::NetworkQualityAlertMonitor(const std::shared_ptr<Core> &core)
    : AlertMonitor(core), mNackSent(false), mBurstCount(0), mFirstMeasureNonZero(false), mLastNackLoss(0),
      mLastTotalLoss(0), mNackIndicator(0.0) {
	getTimer(LinphoneAlertQoSHighLossLateRate, "network"s, "loss_rate_interval"s, 5000);
	getTimer(LinphoneAlertQoSHighRemoteLossRate, "network"s, "remote_loss_rate_interval"s, 5000);
	getTimer(LinphoneAlertQoSLostSignal, "network"s, "lost_signal_interval"s, 1000);
	getTimer(LinphoneAlertQoSBurstOccured, "network"s, "burst_occured_interval"s, 1000);
	getTimer(LinphoneAlertQoSRetransmissionFailures, "network"s, "nack_check_interval"s, 2000);
	getTimer(LinphoneAlertQoSLowSignal, "network"s, "low_signal_interval"s, 1000);
}
float NetworkQualityAlertMonitor::getLossRateThreshold() {
	auto config = linphone_core_get_config(getCore()->getCCore());
	return linphone_config_get_float(config, "alerts::network", "loss_rate_threshold", 5.0);
}
void NetworkQualityAlertMonitor::check(LinphoneCallStats *callStats, bool burstOccured) {
	if (!mAlertsEnabled) return;
	float localLossRate = linphone_call_stats_get_local_loss_rate(callStats);
	float localLateRate = linphone_call_stats_get_local_late_rate(callStats);

	float remoteLossRate = linphone_call_stats_get_receiver_loss_rate(callStats);
	checkRemoteLossRate(remoteLossRate);
	checkLocalLossRate(localLossRate, localLateRate, LinphoneStreamTypeAudio);
	checkLostSignal();
	checkBurstOccurence(burstOccured);
	checkSignalQuality();
}
void NetworkQualityAlertMonitor::checkLocalLossRate(float lossRate, float lateRate, LinphoneStreamType streamType) {
	bool condition = (lossRate >= getLossRateThreshold());
	auto properties = (new Dictionary())->toSharedPtr();
	properties->setProperty("loss rate", lossRate);
	properties->setProperty("late rate", lateRate);
	properties->setProperty("media type", streamType);
	handleAlert(LinphoneAlertQoSHighLossLateRate, properties, condition);
}

void NetworkQualityAlertMonitor::checkRemoteLossRate(float recievedLossRate) {
	bool condition = (recievedLossRate >= getLossRateThreshold());
	auto properties = (new Dictionary())->toSharedPtr();
	properties->setProperty("loss rate", recievedLossRate);
	handleAlert(LinphoneAlertQoSHighRemoteLossRate, properties, condition);
}
void NetworkQualityAlertMonitor::checkLostSignal() {
	bool condition = !linphone_core_is_network_reachable(getCore()->getCCore());
	handleAlert(LinphoneAlertQoSLostSignal, nullptr, condition);
}
void NetworkQualityAlertMonitor::reset() {
	mBurstCount = 0;
}
void NetworkQualityAlertMonitor::checkBurstOccurence(const bool burstOccured) {
	mBurstCount += burstOccured;
	bool condition = (mBurstCount > 0);
	handleAlert(LinphoneAlertQoSBurstOccured, nullptr, condition);
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
		auto properties = (new Dictionary())->toSharedPtr();
		float threshold = linphone_config_get_float(linphone_core_get_config(getCore()->getCCore()), "alerts::network",
		                                            "nack_threshold", 0.5f);
		properties->setProperty("nack indicator", mNackIndicator);
		mLastNackLoss = currentNackLoss;
		mLastTotalLoss = currentTotalLoss;
		handleAlert(LinphoneAlertQoSRetransmissionFailures, properties, mNackIndicator <= threshold);
	}
}

void NetworkQualityAlertMonitor::checkSignalQuality() {
	bool condition = false;
	float value = 0.0f;
	auto properties = (new Dictionary())->toSharedPtr();
	auto threshold = linphone_config_get_float(linphone_core_get_config(getCore()->getCCore()), "alerts::network",
	                                           "signal_threshold", -70.0f);

	auto information = getCore()->getSignalInformation();
	if (information) {
		value = information->getStrength();
		condition = (value <= threshold);
	}
	properties->setProperty("Rssi value", value);
	handleAlert(LinphoneAlertQoSLowSignal, properties, condition);
}

LINPHONE_END_NAMESPACE