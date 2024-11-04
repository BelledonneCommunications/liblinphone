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

#ifndef _CONFERENCE_CBS_H_
#define _CONFERENCE_CBS_H_

#include "belle-sip/object++.hh"

#include "c-wrapper/c-wrapper.h"
#include "linphone/api/c-callbacks.h"
#include "linphone/api/c-types.h"
#include "linphone/utils/general.h"

LINPHONE_BEGIN_NAMESPACE

class ConferenceCbs : public bellesip::HybridObject<LinphoneConferenceCbs, ConferenceCbs>, public Callbacks {
public:
	LinphoneConferenceCbsAllowedParticipantListChangedCb allowedParticipantListChangedCb;
	LinphoneConferenceCbsParticipantAddedCb participantAddedCb;
	LinphoneConferenceCbsParticipantRemovedCb participantRemovedCb;
	LinphoneConferenceCbsParticipantDeviceAddedCb participantDeviceAddedCb;
	LinphoneConferenceCbsParticipantDeviceRemovedCb participantDeviceRemovedCb;
	LinphoneConferenceCbsParticipantDeviceJoiningRequestCb participantDeviceJoiningRequestCb;
	LinphoneConferenceCbsParticipantRoleChangedCb participantRoleChangedCb;
	LinphoneConferenceCbsParticipantAdminStatusChangedCb participantAdminStatusChangedCb;
	LinphoneConferenceCbsParticipantDeviceMediaCapabilityChangedCb participantDeviceMediaCapabilityChangedCb;
	LinphoneConferenceCbsParticipantDeviceMediaAvailabilityChangedCb participantDeviceMediaAvailabilityChangedCb;
	LinphoneConferenceCbsParticipantDeviceStateChangedCb participantDeviceStateChangedCb;
	LinphoneConferenceCbsParticipantDeviceScreenSharingChangedCb participantDeviceScreenSharingChangedCb;
	LinphoneConferenceCbsStateChangedCb stateChangedCb;
	LinphoneConferenceCbsSubjectChangedCb subjectChangedCb;
	LinphoneConferenceCbsAvailableMediaChangedCb availableMediaChangedCb;
	LinphoneConferenceCbsAudioDeviceChangedCb audioDeviceChangedCb;
	LinphoneConferenceCbsParticipantDeviceIsSpeakingChangedCb participantDeviceIsSpeakingChangedCb;
	LinphoneConferenceCbsParticipantDeviceIsMutedCb participantDeviceIsMutedCb;
	LinphoneConferenceCbsActiveSpeakerParticipantDeviceCb activeSpeakerParticipantDeviceCb;
	LinphoneConferenceCbsFullStateReceivedCb fullStateReceivedCb;
};

LINPHONE_END_NAMESPACE

#endif // _CONFERENCE_CBS_H_
