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

#ifndef _CHAT_ROOM_CBS_H_
#define _CHAT_ROOM_CBS_H_

#include "belle-sip/object++.hh"

#include "c-wrapper/c-wrapper.h"
#include "linphone/api/c-callbacks.h"
#include "tester_utils.h"

LINPHONE_BEGIN_NAMESPACE

class ChatRoomCbs : public bellesip::HybridObject<LinphoneChatRoomCbs, ChatRoomCbs>, public Callbacks {
public:
	LinphoneChatRoomCbsSessionStateChangedCb sessionStateChangedCb;
	LinphoneChatRoomCbsIsComposingReceivedCb isComposingReceivedCb;
	LinphoneChatRoomCbsMessageReceivedCb messageReceivedCb;
	LinphoneChatRoomCbsMessagesReceivedCb messagesReceivedCb;
	LinphoneChatRoomCbsParticipantAddedCb participantAddedCb;
	LinphoneChatRoomCbsParticipantRemovedCb participantRemovedCb;
	LinphoneChatRoomCbsParticipantDeviceAddedCb participantDeviceAddedCb;
	LinphoneChatRoomCbsParticipantDeviceRemovedCb participantDeviceRemovedCb;
	LinphoneChatRoomCbsParticipantDeviceMediaAvailabilityChangedCb participantDeviceMediaAvailabilityChangedCb;
	LinphoneChatRoomCbsParticipantDeviceStateChangedCb participantDeviceStateChangedCb;
	LinphoneChatRoomCbsParticipantAdminStatusChangedCb participantAdminStatusChangedCb;
	LinphoneChatRoomCbsStateChangedCb stateChangedCb;
	LinphoneChatRoomCbsSecurityEventCb securityEventCb;
	LinphoneChatRoomCbsSubjectChangedCb subjectChangedCb;
	LinphoneChatRoomCbsConferenceJoinedCb conferenceJoinedCb;
	LinphoneChatRoomCbsConferenceLeftCb conferenceLeftCb;
	LinphoneChatRoomCbsUndecryptableMessageReceivedCb undecryptableMessageReceivedCb;
	LinphoneChatRoomCbsChatMessageReceivedCb chatMessageReceivedCb;
	LinphoneChatRoomCbsChatMessagesReceivedCb chatMessagesReceivedCb;
	LinphoneChatRoomCbsChatMessageSendingCb chatMessageSendingCb;
	LinphoneChatRoomCbsChatMessageSentCb chatMessageSentCb;
	LinphoneChatRoomCbsConferenceAddressGenerationCb conferenceAddressGenerationCb;
	LinphoneChatRoomCbsParticipantRegistrationSubscriptionRequestedCb participantRegistrationSubscriptionRequestedCb;
	LinphoneChatRoomCbsParticipantRegistrationUnsubscriptionRequestedCb
	    participantRegistrationUnsubscriptionRequestedCb;
	LinphoneChatRoomCbsShouldChatMessageBeStoredCb shouldMessageBeStoredCb;
	LinphoneChatRoomCbsEphemeralEventCb ephemeralEventCb;
	LinphoneChatRoomCbsEphemeralMessageTimerStartedCb ephemeralMessageTimerStartedCb;
	LinphoneChatRoomCbsEphemeralMessageDeletedCb ephemeralMessageDeletedCb;
	LinphoneChatRoomCbsChatMessageParticipantImdnStateChangedCb chatMessageParticipantImdnStateChangedCb;
	LinphoneChatRoomCbsNewEventCb newEventCb;
	LinphoneChatRoomCbsNewEventsCb newEventsCb;
	LinphoneChatRoomCbsChatRoomReadCb chatRoomReadCb;
	LinphoneChatRoomCbsNewMessageReactionCb newMessageReactionCb;
};

LINPHONE_END_NAMESPACE

#endif // _CHAT_ROOM_CBS_H_
