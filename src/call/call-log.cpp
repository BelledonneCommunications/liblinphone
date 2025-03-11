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

#include "call-log.h"

#include <ctime>
#include <sstream>

#include "address/address.h"
#include "c-wrapper/internal/c-tools.h"
#include "chat/chat-room/abstract-chat-room.h"
#include "core/core-p.h"
#include "linphone/types.h"
#include "linphone/utils/utils.h"
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

CallLog::CallLog(shared_ptr<Core> core,
                 LinphoneCallDir direction,
                 const std::shared_ptr<const Address> &from,
                 const std::shared_ptr<const Address> &to)
    : CoreAccessor(core) {
	mDirection = direction;
	setFromAddress(from);
	setToAddress(to);

	mStartTime = std::time(nullptr);
	mStartDate = Utils::getTimeAsString("%c", mStartTime);

	mReporting.reports[LINPHONE_CALL_STATS_AUDIO] = linphone_reporting_new();
	mReporting.reports[LINPHONE_CALL_STATS_VIDEO] = linphone_reporting_new();
	mReporting.reports[LINPHONE_CALL_STATS_TEXT] = linphone_reporting_new();
}

CallLog::~CallLog() {
	if (mReporting.reports[LINPHONE_CALL_STATS_AUDIO] != nullptr)
		linphone_reporting_destroy(mReporting.reports[LINPHONE_CALL_STATS_AUDIO]);
	if (mReporting.reports[LINPHONE_CALL_STATS_VIDEO] != nullptr)
		linphone_reporting_destroy(mReporting.reports[LINPHONE_CALL_STATS_VIDEO]);
	if (mReporting.reports[LINPHONE_CALL_STATS_TEXT] != nullptr)
		linphone_reporting_destroy(mReporting.reports[LINPHONE_CALL_STATS_TEXT]);
	if (mErrorInfo != nullptr) linphone_error_info_unref(mErrorInfo);
}

// =============================================================================

LinphoneCallDir CallLog::getDirection() const {
	return mDirection;
}

void CallLog::setDirection(LinphoneCallDir direction) {
	mDirection = direction;
}

int CallLog::getDuration() const {
	return mDuration;
}

void CallLog::setDuration(int duration) {
	mDuration = duration;
}

float CallLog::getQuality() const {
	return mQuality;
}

void CallLog::setQuality(float quality) {
	mQuality = quality;
}

const std::shared_ptr<Address> &CallLog::getFromAddress() const {
	return mFrom;
}

void CallLog::setFromAddress(const std::shared_ptr<const Address> &address) {
	mFrom = address->clone()->toSharedPtr();
}

const std::shared_ptr<Address> &CallLog::getToAddress() const {
	return mTo;
}

void CallLog::setToAddress(const std::shared_ptr<const Address> &address) {
	mTo = address->clone()->toSharedPtr();
}

const string &CallLog::getCallId() const {
	return mCallId;
}

void CallLog::setCallId(const string &callId) {
	mCallId = callId;
}

const string &CallLog::getRefKey() const {
	return mRefKey;
}

void CallLog::setRefKey(const string &refKey) {
	mRefKey = refKey;
}

time_t CallLog::getStartTime() const {
	return mStartTime;
}

void CallLog::setStartTime(time_t startTime) {
	mStartTime = startTime;
	mStartDate = Utils::getTimeAsString("%c", mStartTime);
}

time_t CallLog::getConnectedTime() const {
	return mConnectedTime;
}

void CallLog::setConnectedTime(time_t connectedTime) {
	mConnectedTime = connectedTime;
}

LinphoneCallStatus CallLog::getStatus() const {
	return mStatus;
}

void CallLog::setStatus(LinphoneCallStatus status) {
	mStatus = status;
}

bool CallLog::isVideoEnabled() const {
	return mVideoEnabled;
}

void CallLog::setVideoEnabled(bool enabled) {
	mVideoEnabled = enabled;
}

bool CallLog::wasConference() {
	return getConferenceInfo() != nullptr;
}

const LinphoneErrorInfo *CallLog::getErrorInfo() const {
	return mErrorInfo;
}

void CallLog::setErrorInfo(LinphoneErrorInfo *errorInfo) {
	if (mErrorInfo) linphone_error_info_unref(mErrorInfo);
	mErrorInfo = linphone_error_info_clone(errorInfo);
}

const std::shared_ptr<Address> &CallLog::getRemoteAddress() const {
	return (mDirection == LinphoneCallIncoming) ? mFrom : mTo;
}

void CallLog::setRemoteAddress(const std::shared_ptr<Address> &remoteAddress) {
	if (mDirection == LinphoneCallIncoming) {
		setFromAddress(remoteAddress);
	} else {
		setToAddress(remoteAddress);
	}
}

void *CallLog::getUserData() const {
	return mUserData;
}

void CallLog::setUserData(void *userData) {
	mUserData = userData;
}

const std::shared_ptr<Address> &CallLog::getLocalAddress() const {
	return (mDirection == LinphoneCallIncoming) ? mTo : mFrom;
}

const std::string &CallLog::getStartTimeString() const {
	return mStartDate;
}

LinphoneQualityReporting *CallLog::getQualityReporting() {
	return &mReporting;
}

void CallLog::setConferenceInfoId(long long conferenceInfoId) {
	mConferenceInfoId = conferenceInfoId;
}

// =============================================================================

void CallLog::setConferenceInfo(std::shared_ptr<ConferenceInfo> conferenceInfo) {
	mConferenceInfo = conferenceInfo;
}

std::shared_ptr<ConferenceInfo> CallLog::getConferenceInfo() const {
	// The conference info stored in the database is always up to date, therefore try to update the cache all the time
	// if there id is valid Nonetheless, the cache variable is required if the core disables the storage of information
	// in the database.
#ifdef HAVE_DB_STORAGE
	auto &db = L_GET_PRIVATE(getCore())->mainDb;
	if (mConferenceInfoId != -1) {
		mConferenceInfo = db->getConferenceInfo(mConferenceInfoId);
	} else if (mTo && !mConferenceInfo) {
		// Try to find the conference info based on the to address
		// We enter this branch of the if-else statement only if the call cannot be started right away, for example when
		// ICE candidates need to be gathered first
		mConferenceInfo = db->getConferenceInfoFromURI(getRemoteAddress());
	}
#endif // HAVE_DB_STORAGE

	return mConferenceInfo;
}

const std::shared_ptr<AbstractChatRoom> CallLog::getChatRoom() const {
	auto conferenceInfo = getConferenceInfo();
	if (conferenceInfo && conferenceInfo->getCapability(LinphoneStreamTypeText)) {
		return getCore()->getPrivate()->searchChatRoom(nullptr, nullptr, conferenceInfo->getUri(), {});
	}
	return nullptr;
}

// =============================================================================

string CallLog::toString() const {
	ostringstream os;

	os << (mDirection == LinphoneCallIncoming ? "Incoming call" : "Outgoing call") << " with call-id: " << mCallId
	   << " at " << mStartDate << "\n";

	os << "From: " << *mFrom << "\nTo: " << *mTo << "\n";

	string status;
	switch (mStatus) {
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

	os << "Status: " << status << "\nDuration: " << (mDuration / 60) << " mn " << (mDuration % 60) << " sec\n";

	return os.str();
}

LINPHONE_END_NAMESPACE
