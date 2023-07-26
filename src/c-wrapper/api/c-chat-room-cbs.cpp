/* * Copyright (c) 2010-2022 Belledonne Communications SARL.  * * This file is part of Liblinphone
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

#include "linphone/api/c-chat-room-cbs.h"
#include "chat/chat-room/chat-room-cbs.h"
#include "private_functions.h"

// =============================================================================

LinphoneChatRoomCbs *_linphone_chat_room_cbs_new(void) {
	return LinphonePrivate::ChatRoomCbs::createCObject();
}

LinphoneChatRoomCbs *linphone_chat_room_cbs_ref(LinphoneChatRoomCbs *cbs) {
	belle_sip_object_ref(cbs);
	return cbs;
}

void linphone_chat_room_cbs_unref(LinphoneChatRoomCbs *cbs) {
	belle_sip_object_unref(cbs);
}

void *linphone_chat_room_cbs_get_user_data(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->getUserData();
}

void linphone_chat_room_cbs_set_user_data(LinphoneChatRoomCbs *cbs, void *ud) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->setUserData(ud);
}

void linphone_chat_room_cbs_set_session_state_changed(LinphoneChatRoomCbs *cbs,
                                                      LinphoneChatRoomCbsSessionStateChangedCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->sessionStateChangedCb = cb;
}

LinphoneChatRoomCbsSessionStateChangedCb
linphone_chat_room_cbs_get_session_state_changed(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->sessionStateChangedCb;
}

LinphoneChatRoomCbsIsComposingReceivedCb
linphone_chat_room_cbs_get_is_composing_received(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->isComposingReceivedCb;
}

void linphone_chat_room_cbs_set_is_composing_received(LinphoneChatRoomCbs *cbs,
                                                      LinphoneChatRoomCbsIsComposingReceivedCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->isComposingReceivedCb = cb;
}

LinphoneChatRoomCbsMessageReceivedCb linphone_chat_room_cbs_get_message_received(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->messageReceivedCb;
}

void linphone_chat_room_cbs_set_message_received(LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsMessageReceivedCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->messageReceivedCb = cb;
}

void linphone_chat_room_cbs_set_chat_messages_received(LinphoneChatRoomCbs *cbs,
                                                       LinphoneChatRoomCbsChatMessagesReceivedCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->chatMessagesReceivedCb = cb;
}

LinphoneChatRoomCbsMessagesReceivedCb linphone_chat_room_cbs_get_messages_received(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->messagesReceivedCb;
}

LinphoneChatRoomCbsNewEventCb linphone_chat_room_cbs_get_new_event(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->newEventCb;
}

void linphone_chat_room_cbs_set_new_event(LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsNewEventCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->newEventCb = cb;
}

LinphoneChatRoomCbsNewEventsCb linphone_chat_room_cbs_get_new_events(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->newEventsCb;
}

void linphone_chat_room_cbs_set_new_events(LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsNewEventsCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->newEventsCb = cb;
}

LinphoneChatRoomCbsChatMessageReceivedCb
linphone_chat_room_cbs_get_chat_message_received(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->chatMessageReceivedCb;
}

void linphone_chat_room_cbs_set_chat_message_received(LinphoneChatRoomCbs *cbs,
                                                      LinphoneChatRoomCbsChatMessageReceivedCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->chatMessageReceivedCb = cb;
}

LinphoneChatRoomCbsChatMessagesReceivedCb
linphone_chat_room_cbs_get_chat_messages_received(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->chatMessagesReceivedCb;
}

void linphone_chat_room_cbs_set_messages_received(LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsMessagesReceivedCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->messagesReceivedCb = cb;
}

LinphoneChatRoomCbsChatMessageSendingCb
linphone_chat_room_cbs_get_chat_message_sending(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->chatMessageSendingCb;
}

void linphone_chat_room_cbs_set_chat_message_sending(LinphoneChatRoomCbs *cbs,
                                                     LinphoneChatRoomCbsChatMessageSendingCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->chatMessageSendingCb = cb;
}

LinphoneChatRoomCbsChatMessageSentCb linphone_chat_room_cbs_get_chat_message_sent(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->chatMessageSentCb;
}

void linphone_chat_room_cbs_set_chat_message_sent(LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsChatMessageSentCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->chatMessageSentCb = cb;
}

LinphoneChatRoomCbsParticipantAddedCb linphone_chat_room_cbs_get_participant_added(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->participantAddedCb;
}

void linphone_chat_room_cbs_set_participant_added(LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsParticipantAddedCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->participantAddedCb = cb;
}

LinphoneChatRoomCbsParticipantRemovedCb linphone_chat_room_cbs_get_participant_removed(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->participantRemovedCb;
}

void linphone_chat_room_cbs_set_participant_removed(LinphoneChatRoomCbs *cbs,
                                                    LinphoneChatRoomCbsParticipantRemovedCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->participantRemovedCb = cb;
}

LinphoneChatRoomCbsParticipantAdminStatusChangedCb
linphone_chat_room_cbs_get_participant_admin_status_changed(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->participantAdminStatusChangedCb;
}

void linphone_chat_room_cbs_set_participant_admin_status_changed(
    LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsParticipantAdminStatusChangedCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->participantAdminStatusChangedCb = cb;
}

LinphoneChatRoomCbsStateChangedCb linphone_chat_room_cbs_get_state_changed(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->stateChangedCb;
}

void linphone_chat_room_cbs_set_state_changed(LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsStateChangedCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->stateChangedCb = cb;
}

LinphoneChatRoomCbsSecurityEventCb linphone_chat_room_cbs_get_security_event(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->securityEventCb;
}

void linphone_chat_room_cbs_set_security_event(LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsSecurityEventCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->securityEventCb = cb;
}

LinphoneChatRoomCbsSubjectChangedCb linphone_chat_room_cbs_get_subject_changed(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->subjectChangedCb;
}

void linphone_chat_room_cbs_set_subject_changed(LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsSubjectChangedCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->subjectChangedCb = cb;
}

LinphoneChatRoomCbsUndecryptableMessageReceivedCb
linphone_chat_room_cbs_get_undecryptable_message_received(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->undecryptableMessageReceivedCb;
}

void linphone_chat_room_cbs_set_undecryptable_message_received(LinphoneChatRoomCbs *cbs,
                                                               LinphoneChatRoomCbsUndecryptableMessageReceivedCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->undecryptableMessageReceivedCb = cb;
}

LinphoneChatRoomCbsParticipantDeviceAddedCb
linphone_chat_room_cbs_get_participant_device_added(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->participantDeviceAddedCb;
}

void linphone_chat_room_cbs_set_participant_device_added(LinphoneChatRoomCbs *cbs,
                                                         LinphoneChatRoomCbsParticipantDeviceAddedCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->participantDeviceAddedCb = cb;
}

LinphoneChatRoomCbsParticipantDeviceRemovedCb
linphone_chat_room_cbs_get_participant_device_removed(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->participantDeviceRemovedCb;
}

void linphone_chat_room_cbs_set_participant_device_removed(LinphoneChatRoomCbs *cbs,
                                                           LinphoneChatRoomCbsParticipantDeviceRemovedCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->participantDeviceRemovedCb = cb;
}

LinphoneChatRoomCbsParticipantDeviceMediaAvailabilityChangedCb
linphone_chat_room_cbs_get_participant_device_media_availability_changed(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->participantDeviceMediaAvailabilityChangedCb;
}

void linphone_chat_room_cbs_set_participant_device_media_availability_changed(
    LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsParticipantDeviceMediaAvailabilityChangedCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->participantDeviceMediaAvailabilityChangedCb = cb;
}

LinphoneChatRoomCbsParticipantDeviceStateChangedCb
linphone_chat_room_cbs_get_participant_device_state_changed(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->participantDeviceStateChangedCb;
}

void linphone_chat_room_cbs_set_participant_device_state_changed(
    LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsParticipantDeviceStateChangedCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->participantDeviceStateChangedCb = cb;
}

LinphoneChatRoomCbsConferenceJoinedCb linphone_chat_room_cbs_get_conference_joined(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->conferenceJoinedCb;
}

void linphone_chat_room_cbs_set_conference_joined(LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsConferenceJoinedCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->conferenceJoinedCb = cb;
}

LinphoneChatRoomCbsConferenceLeftCb linphone_chat_room_cbs_get_conference_left(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->conferenceLeftCb;
}

void linphone_chat_room_cbs_set_conference_left(LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsConferenceLeftCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->conferenceLeftCb = cb;
}

LinphoneChatRoomCbsEphemeralEventCb linphone_chat_room_cbs_get_ephemeral_event(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->ephemeralEventCb;
}

void linphone_chat_room_cbs_set_ephemeral_event(LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsEphemeralEventCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->ephemeralEventCb = cb;
}

LinphoneChatRoomCbsEphemeralMessageTimerStartedCb
linphone_chat_room_cbs_get_ephemeral_message_timer_started(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->ephemeralMessageTimerStartedCb;
}

void linphone_chat_room_cbs_set_ephemeral_message_timer_started(LinphoneChatRoomCbs *cbs,
                                                                LinphoneChatRoomCbsEphemeralMessageTimerStartedCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->ephemeralMessageTimerStartedCb = cb;
}

LinphoneChatRoomCbsEphemeralMessageDeletedCb
linphone_chat_room_cbs_get_ephemeral_message_deleted(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->ephemeralMessageDeletedCb;
}

void linphone_chat_room_cbs_set_ephemeral_message_deleted(LinphoneChatRoomCbs *cbs,
                                                          LinphoneChatRoomCbsEphemeralMessageDeletedCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->ephemeralMessageDeletedCb = cb;
}

LinphoneChatRoomCbsConferenceAddressGenerationCb
linphone_chat_room_cbs_get_conference_address_generation(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->conferenceAddressGenerationCb;
}

void linphone_chat_room_cbs_set_conference_address_generation(LinphoneChatRoomCbs *cbs,
                                                              LinphoneChatRoomCbsConferenceAddressGenerationCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->conferenceAddressGenerationCb = cb;
}

LinphoneChatRoomCbsParticipantRegistrationSubscriptionRequestedCb
linphone_chat_room_cbs_get_participant_registration_subscription_requested(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->participantRegistrationSubscriptionRequestedCb;
}

void linphone_chat_room_cbs_set_participant_registration_subscription_requested(
    LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsParticipantRegistrationSubscriptionRequestedCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->participantRegistrationSubscriptionRequestedCb = cb;
}

LinphoneChatRoomCbsParticipantRegistrationUnsubscriptionRequestedCb
linphone_chat_room_cbs_get_participant_registration_unsubscription_requested(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->participantRegistrationUnsubscriptionRequestedCb;
}

void linphone_chat_room_cbs_set_participant_registration_unsubscription_requested(
    LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsParticipantRegistrationUnsubscriptionRequestedCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->participantRegistrationUnsubscriptionRequestedCb = cb;
}

LinphoneChatRoomCbsShouldChatMessageBeStoredCb
linphone_chat_room_cbs_get_chat_message_should_be_stored(LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->shouldMessageBeStoredCb;
}

void linphone_chat_room_cbs_set_chat_message_should_be_stored(LinphoneChatRoomCbs *cbs,
                                                              LinphoneChatRoomCbsShouldChatMessageBeStoredCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->shouldMessageBeStoredCb = cb;
}

LinphoneChatRoomCbsChatMessageParticipantImdnStateChangedCb
linphone_chat_room_cbs_get_chat_message_participant_imdn_state_changed(LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->chatMessageParticipantImdnStateChangedCb;
}

void linphone_chat_room_cbs_set_chat_message_participant_imdn_state_changed(
    LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsChatMessageParticipantImdnStateChangedCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->chatMessageParticipantImdnStateChangedCb = cb;
}

LinphoneChatRoomCbsChatRoomReadCb linphone_chat_room_cbs_get_chat_room_read(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->chatRoomReadCb;
}

void linphone_chat_room_cbs_set_chat_room_read(LinphoneChatRoomCbs *cbs, LinphoneChatRoomCbsChatRoomReadCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->chatRoomReadCb = cb;
}

LinphoneChatRoomCbsNewMessageReactionCb
linphone_chat_room_cbs_get_new_message_reaction(const LinphoneChatRoomCbs *cbs) {
	return LinphonePrivate::ChatRoomCbs::toCpp(cbs)->newMessageReactionCb;
}

void linphone_chat_room_cbs_set_new_message_reaction(LinphoneChatRoomCbs *cbs,
                                                     LinphoneChatRoomCbsNewMessageReactionCb cb) {
	LinphonePrivate::ChatRoomCbs::toCpp(cbs)->newMessageReactionCb = cb;
}
