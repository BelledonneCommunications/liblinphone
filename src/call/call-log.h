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

#ifndef _L_CALL_LOG_H_
#define _L_CALL_LOG_H_

#include <belle-sip/object++.hh>
#include "linphone/api/c-types.h"
#include "linphone/types.h"
#include "core/core-accessor.h"
#include "conference/conference-info.h"

#include "quality_reporting.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC CallLog : public bellesip::HybridObject<LinphoneCallLog, CallLog>, public CoreAccessor {
public:
	CallLog (std::shared_ptr<Core> core, LinphoneCallDir direction, LinphoneAddress *from, LinphoneAddress *to);
	~CallLog ();

	LinphoneCallDir getDirection () const;
	void setDirection (LinphoneCallDir direction);

	int getDuration () const;
	void setDuration (int duration);

	float getQuality () const;
	void setQuality (float quality);

	const LinphoneAddress *getFromAddress () const;
	void setFromAddress (LinphoneAddress *address);

	const LinphoneAddress *getToAddress () const;
	void setToAddress (LinphoneAddress *address);

	const std::string &getCallId () const;
	void setCallId (const std::string &callId);

	const std::string &getRefKey () const;
	void setRefKey (const std::string &refKey);

	time_t getStartTime () const;
	void setStartTime (time_t startTime);

	time_t getConnectedTime () const;
	void setConnectedTime (time_t connectedTime);

	LinphoneCallStatus getStatus () const;
	void setStatus (LinphoneCallStatus status);

	bool isVideoEnabled () const;
	void setVideoEnabled (bool enabled);

	bool wasConference ();

	const LinphoneErrorInfo *getErrorInfo () const;
	void setErrorInfo (LinphoneErrorInfo *errorInfo);

	const LinphoneAddress *getRemoteAddress () const;
	void setRemoteAddress (const LinphoneAddress *remoteAddress);

	void *getUserData () const;
	void setUserData (void* userData);

	const LinphoneAddress *getLocalAddress () const;
	const std::string &getStartTimeString () const;

	LinphoneQualityReporting *getQualityReporting ();

	void setConferenceInfoId (long long conferenceInfoId);

	std::shared_ptr<ConferenceInfo> getConferenceInfo ();

	std::string toString () const override;
private:
	void *mUserData = nullptr;

	LinphoneCallDir mDirection; /**< The direction of the call*/
	LinphoneCallStatus mStatus = LinphoneCallAborted; /**< The status of the call*/
	LinphoneAddress *mFrom = nullptr; /**<Originator of the call as a LinphoneAddress object*/
	LinphoneAddress *mTo = nullptr; /**<Destination of the call as a LinphoneAddress object*/
	LinphoneQualityReporting mReporting = {};
	LinphoneErrorInfo *mErrorInfo = nullptr;

	std::string mStartDate; /**<Human readable string containing the start date*/
	std::string mRefKey;
	std::string mCallId; /**unique id of a call*/

	int mDuration = -1; /**<Duration of the call starting in connected state in seconds*/
	float mQuality = -1.0;
	time_t mStartTime = 0; /**Start date of the call in seconds as expressed in a time_t */
	time_t mConnectedTime = 0; /**Connecting date of the call in seconds as expressed in a time_t */
	bool mVideoEnabled = false;

	long long mConferenceInfoId = -1;
	std::shared_ptr<ConferenceInfo> mConferenceInfo = nullptr;
};

LINPHONE_END_NAMESPACE

#endif /* _L_CALL_LOG_H_ */
