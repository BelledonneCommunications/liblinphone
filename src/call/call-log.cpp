/*
 * Copyright (c) 2010-2021 Belledonne Communications SARL.
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

#include "call-log.h"

#include <ctime>
#include <sstream>

#include "c-wrapper/internal/c-tools.h"
#include "core/core-p.h"
#include "linphone/utils/utils.h"
#include "linphone/types.h"
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

CallLog::CallLog (shared_ptr<Core> core, LinphoneCallDir direction, LinphoneAddress *from, LinphoneAddress *to) : CoreAccessor(core) {
	mDirection = direction;
	mFrom = from;
	mTo = to;

	mStartTime = std::time(nullptr);
	mStartDate = Utils::getTimeAsString("%c", mStartTime);

	mReporting.reports[LINPHONE_CALL_STATS_AUDIO] = linphone_reporting_new();
	mReporting.reports[LINPHONE_CALL_STATS_VIDEO] = linphone_reporting_new();
	mReporting.reports[LINPHONE_CALL_STATS_TEXT] = linphone_reporting_new();
}

CallLog::~CallLog () {
	if (mFrom != nullptr) linphone_address_unref(mFrom);
	if (mTo != nullptr) linphone_address_unref(mTo);
	if (mReporting.reports[LINPHONE_CALL_STATS_AUDIO] != nullptr) linphone_reporting_destroy(mReporting.reports[LINPHONE_CALL_STATS_AUDIO]);
	if (mReporting.reports[LINPHONE_CALL_STATS_VIDEO] != nullptr) linphone_reporting_destroy(mReporting.reports[LINPHONE_CALL_STATS_VIDEO]);
	if (mReporting.reports[LINPHONE_CALL_STATS_TEXT] != nullptr) linphone_reporting_destroy(mReporting.reports[LINPHONE_CALL_STATS_TEXT]);
	if (mErrorInfo != nullptr) linphone_error_info_unref(mErrorInfo);
}

// =============================================================================

LinphoneCallDir CallLog::getDirection () const {
	return mDirection;
}

void CallLog::setDirection (LinphoneCallDir direction) {
	mDirection = direction;
}

int CallLog::getDuration () const {
	return mDuration;
}

void CallLog::setDuration (int duration) {
	mDuration = duration;
}

float CallLog::getQuality () const {
	return mQuality;
}

void CallLog::setQuality (float quality) {
	mQuality = quality;
}

const LinphoneAddress *CallLog::getFromAddress () const {
	return mFrom;
}

void CallLog::setFromAddress (LinphoneAddress *address) {
	if (mFrom) linphone_address_unref(mFrom);
	mFrom = address;
}

const LinphoneAddress *CallLog::getToAddress () const {
	return mTo;
}

void CallLog::setToAddress (LinphoneAddress *address) {
	if (mTo) linphone_address_unref(mTo);
	mTo = address;
}

const string &CallLog::getCallId () const {
	return mCallId;
}

void CallLog::setCallId (const string &callId) {
	mCallId = callId;
}

const string &CallLog::getRefKey () const {
	return mRefKey;
}

void CallLog::setRefKey (const string &refKey) {
	mRefKey = refKey;
}

time_t CallLog::getStartTime () const {
	return mStartTime;
}

void CallLog::setStartTime (time_t startTime) {
	mStartTime = startTime;
	mStartDate = Utils::getTimeAsString("%c", mStartTime);
}

time_t CallLog::getConnectedTime () const {
	return mConnectedTime;
}

void CallLog::setConnectedTime (time_t connectedTime) {
	mConnectedTime = connectedTime;
}

LinphoneCallStatus CallLog::getStatus () const {
	return mStatus;
}

void CallLog::setStatus (LinphoneCallStatus status) {
	mStatus = status;
}

bool CallLog::isVideoEnabled () const {
	return mVideoEnabled;
}

void CallLog::setVideoEnabled (bool enabled) {
	mVideoEnabled = enabled;
}

bool CallLog::wasConference () {
	return getConferenceInfo() != nullptr;
}

const LinphoneErrorInfo *CallLog::getErrorInfo () const {
	return mErrorInfo;
}

void CallLog::setErrorInfo (LinphoneErrorInfo *errorInfo) {
	if (mErrorInfo) linphone_error_info_unref(mErrorInfo);
	mErrorInfo = errorInfo;
}

const LinphoneAddress *CallLog::getRemoteAddress () const {
	return (mDirection == LinphoneCallIncoming) ? mFrom : mTo;
}

void CallLog::setRemoteAddress (const LinphoneAddress *remoteAddress) {
	if (mDirection == LinphoneCallIncoming) {
		if (mFrom) linphone_address_unref(mFrom);
		mFrom = linphone_address_clone(remoteAddress);
	} else {
		if (mTo) linphone_address_unref(mTo);
		mTo = linphone_address_clone(mTo);
	}
}

void *CallLog::getUserData () const {
	return mUserData;
}

void CallLog::setUserData (void* userData) {
	mUserData = userData;
}

const LinphoneAddress *CallLog::getLocalAddress () const {
	return (mDirection == LinphoneCallIncoming) ? mTo : mFrom;
}

const std::string &CallLog::getStartTimeString () const {
	return mStartDate;
}

LinphoneQualityReporting *CallLog::getQualityReporting () {
	return &mReporting;
}

void CallLog::setConferenceInfoId (long long conferenceInfoId) {
	mConferenceInfoId = conferenceInfoId;
}

// =============================================================================

std::shared_ptr<ConferenceInfo> CallLog::getConferenceInfo () {
	if (mConferenceInfo != nullptr) return mConferenceInfo;

	if (mConferenceInfoId != -1) {
		mConferenceInfo = L_GET_PRIVATE(getCore())->mainDb->getConferenceInfo(mConferenceInfoId);
	}

	return nullptr;
}

// =============================================================================

string CallLog::toString() const {
	ostringstream os;

	os << (mDirection == LinphoneCallIncoming ? "Incoming call" : "Outgoing call") << " at " << mStartDate << "\n";

	char *from = linphone_address_as_string(mFrom);
	char *to = linphone_address_as_string(mTo);

	os << "From: " << from << "\nTo: " << to << "\n";

	bctbx_free(from);
	bctbx_free(to);

	string status;
	switch(mStatus) {
		case LinphoneCallAborted:
			status = "aborted";
			break;
		case LinphoneCallSuccess:
			status = "completed";
			break;
		case LinphoneCallMissed:
			status = "missed";
			break;
		case LinphoneCallAcceptedElsewhere:
			status = "answered elsewhere";
			break;
		case LinphoneCallDeclinedElsewhere:
			status = "declined elsewhere";
			break;
		default:
			status = "unknown";
	}

	os << "Status: " << status << "\nDuration: " << (mDuration/60) << " mn " << (mDuration%60) << " sec\n";

	return os.str();
}

LINPHONE_END_NAMESPACE
