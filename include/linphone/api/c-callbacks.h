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

#ifndef _L_C_CALLBACKS_H_
#define _L_C_CALLBACKS_H_

// TODO: Remove me in the future.
#include "linphone/callbacks.h"
#include "linphone/api/c-types.h"

// =============================================================================

#ifdef __cplusplus
	extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup account
 * @{
**/

/**
 * Callback for notifying when a registration state has changed for the account.
 * @param account #LinphoneAccount object whose registration state changed. @notnil
 * @param state The current #LinphoneRegistrationState.
 * @param message A non NULL informational message about the state. @notnil
 */
typedef void (*LinphoneAccountCbsRegistrationStateChangedCb)(LinphoneAccount *account, LinphoneRegistrationState state, const char *message);

/**
 * @}
**/

/**
 * @addtogroup call_control
 * @{
**/

/**
 * Callback for being notified of received DTMFs.
 * @param call #LinphoneCall object that received the dtmf @notnil
 * @param dtmf The ascii code of the dtmf
 */
typedef void (*LinphoneCallCbsDtmfReceivedCb)(LinphoneCall *call, int dtmf);

/**
 * GoClear ACK sent callback.
 * @param call the #LinphoneCall on which the GoClear ACK was sent. @notnil
 */
typedef void (*LinphoneCallCbsGoClearAckSentCb)(LinphoneCall *call);

/**
 * Call encryption changed callback.
 * @param call #LinphoneCall object whose encryption is changed. @notnil
 * @param on Whether encryption is activated.
 * @param authentication_token An authentication_token, currently set for ZRTP kind of encryption only. @maybenil
 */
typedef void (*LinphoneCallCbsEncryptionChangedCb)(LinphoneCall *call, bool_t on, const char *authentication_token);

/**
 * Call send master key changed callback.
 * @param call #LinphoneCall object whose encryption is changed. @notnil
 * @param master_key The send master key of the SRTP session. @maybenil
 */
typedef void (*LinphoneCallCbsSendMasterKeyChangedCb)(LinphoneCall *call, const char *send_master_key);

/**
 * Call receive master key changed callback.
 * @param call #LinphoneCall object whose encryption is changed. @notnil
 * @param master_key The receive master key of the SRTP session. @maybenil
 */
typedef void (*LinphoneCallCbsReceiveMasterKeyChangedCb)(LinphoneCall *call, const char *receive_master_key);

/**
 * Callback for receiving info messages.
 * @param call #LinphoneCall whose info message belongs to. @notnil
 * @param message #LinphoneInfoMessage object. @notnil
 */
typedef void (*LinphoneCallCbsInfoMessageReceivedCb)(LinphoneCall *call, const LinphoneInfoMessage *message);

/**
 * Call state notification callback.
 * @param call #LinphoneCall whose state is changed. @notnil
 * @param state The new #LinphoneCallState of the call
 * @param message An informational message about the state. @notnil
 */
typedef void (*LinphoneCallCbsStateChangedCb)(LinphoneCall *call, LinphoneCallState state, const char *message);

/**
 * Callback for receiving quality statistics for calls.
 * @param call #LinphoneCall object whose statistics are notified @notnil
 * @param stats #LinphoneCallStats object @notnil
 */
typedef void (*LinphoneCallCbsStatsUpdatedCb)(LinphoneCall *call, const LinphoneCallStats *stats);

/**
 * Callback for notifying progresses of transfers.
 * @param call #LinphoneCall that was transfered @notnil
 * @param state The #LinphoneCallState of the call to transfer target at the far end.
 */
typedef void (*LinphoneCallCbsTransferStateChangedCb)(LinphoneCall *call, LinphoneCallState state);

/**
 * Callback for notifying the processing SIP ACK messages.
 * @param call #LinphoneCall for which an ACK is being received or sent @notnil
 * @param ack the ACK #LinphoneHeaders @notnil
 * @param is_received if TRUE this ACK is an incoming one, otherwise it is an ACK about to be sent.
 */
typedef void (*LinphoneCallCbsAckProcessingCb)(LinphoneCall *call, LinphoneHeaders *ack, bool_t is_received);

/**
 * Callback for notifying a received TMMBR.
 * @param call LinphoneCall for which the TMMBR has changed @notnil
 * @param stream_index the index of the current stream
 * @param tmmbr the value of the received TMMBR
 */
typedef void (*LinphoneCallCbsTmmbrReceivedCb)(LinphoneCall *call, int stream_index, int tmmbr);

/**
 * Callback for notifying a snapshot taken.
 * @param call LinphoneCall for which the snapshot was taken @notnil
 * @param file_path the name of the saved file @notnil
 */
typedef void (*LinphoneCallCbsSnapshotTakenCb)(LinphoneCall *call, const char *file_path);

/**
 * Callback to notify a next video frame has been decoded
 * @param call LinphoneCall for which the next video frame has been decoded @notnil
 */
typedef void (*LinphoneCallCbsNextVideoFrameDecodedCb)(LinphoneCall *call);

/**
 * Callback to notify that the camera is not working and has been changed to "No Webcam".
 *
 * A camera is detected as mis-functionning as soon as it outputs no frames at all during
 * a period of 5 seconds.
 * This check is only performed on desktop platforms, in the purpose of notifying camera
 * failures, for example if when a usb cable gets disconnected.
 *
 * @param call LinphoneCall for which the next video frame has been decoded @notnil
 * @param camera_name the name of the non-working camera @notnil
 */
typedef void (*LinphoneCallCbsCameraNotWorkingCb)(LinphoneCall *call, const char *camera_name);

/**
 * Callback to notify that there are errors from the video rendering. The error code depends of the implementation.
 * - If using OpenGL then the errors comes from eglGetError() : https://registry.khronos.org/EGL/sdk/docs/man/html/eglGetError.xhtml
 *   On `EGL_CONTEXT_LOST`, it is recommanded to restart the Window ID with **_create_native_**_video_window_id() and **_set_native_**_video_window_id() functions.
 *
 * @param call LinphoneCall @notnil
 * @param error_code error code from render. It depends of the renderer.
 */
typedef void (*LinphoneCallCbsVideoDisplayErrorOccurredCb)(LinphoneCall *call, int error_code);

/**
 * Callback to notify that the audio device has been changed.
 *
 * @param call LinphoneCall for which the audio device has changed @notnil
 * @param audio_device the new audio device used for this call @notnil
 */
typedef void (*LinphoneCallCbsAudioDeviceChangedCb)(LinphoneCall *call, LinphoneAudioDevice *audio_device);

/**
 * Callback to notify that the call is being recorded by the remote.
 *
 * @param call LinphoneCall for which the audio is recorded @notnil
 * @param recording TRUE if the call is being recorded by the remote, FALSE otherwise
 */
typedef void (*LinphoneCallCbsRemoteRecordingCb)(LinphoneCall *call, bool_t recording);

/**
 * @}
**/


/**
 * @addtogroup chatroom
 * @{
 */

/**
 * Call back used to notify message delivery status
 * @param message #LinphoneChatMessage object @notnil
 * @param state #LinphoneChatMessageState
 */
typedef void (*LinphoneChatMessageCbsMsgStateChangedCb)(LinphoneChatMessage* message, LinphoneChatMessageState state);

/**
 * Call back used to notify participant IMDN state
 * @param message #LinphoneChatMessage object @notnil
 * @param state #LinphoneParticipantImdnState @notnil
 */
typedef void (*LinphoneChatMessageCbsParticipantImdnStateChangedCb)(LinphoneChatMessage* message, const LinphoneParticipantImdnState *state);

/**
 * File transfer receive callback prototype. This function is called by the core upon an incoming File transfer is started. This function may be call several time for the same file in case of large file.
 * @param message #LinphoneChatMessage message from which the body is received. @notnil
 * @param content #LinphoneContent incoming content information @notnil
 * @param buffer #LinphoneBuffer holding the received data. Empty buffer means end of file. @notnil
 */
typedef void (*LinphoneChatMessageCbsFileTransferRecvCb)(LinphoneChatMessage *message, LinphoneContent* content, const LinphoneBuffer *buffer);

/**
 * File transfer send callback prototype. This function is called by the core when an outgoing file transfer is started. This function is called until size is set to 0.
 * @param message #LinphoneChatMessage message from which the body is received. @notnil
 * @param content #LinphoneContent outgoing content @notnil
 * @param offset the offset in the file from where to get the data to be sent
 * @param size the number of bytes expected by the framework
 * @return A #LinphoneBuffer object holding the data written by the application. An empty buffer means end of file. @maybenil @warning The returned value isn't used, hence the deprecation!
 * @deprecated 17/08/2020 Use #LinphoneChatMessageCbsFileTransferSendChunkCb instead.
 */
typedef LinphoneBuffer * (*LinphoneChatMessageCbsFileTransferSendCb)(LinphoneChatMessage *message, LinphoneContent* content, size_t offset, size_t size);

/**
 * File transfer send callback prototype. This function is called by the core when an outgoing file transfer is started. This function is called until size is set to 0.
 * @param message #LinphoneChatMessage message from which the body is received. @notnil
 * @param content #LinphoneContent outgoing content @notnil
 * @param offset the offset in the file from where to get the data to be sent
 * @param size the number of bytes expected by the framework
 * @param buffer A #LinphoneBuffer to be filled. Leave it empty when end of file has been reached. @notnil
 */
typedef void (*LinphoneChatMessageCbsFileTransferSendChunkCb)(LinphoneChatMessage *message, LinphoneContent* content, size_t offset, size_t size, LinphoneBuffer *buffer);

/**
 * File transfer progress indication callback prototype.
 * @param message #LinphoneChatMessage message from which the body is received. @notnil
 * @param content #LinphoneContent incoming content information @notnil
 * @param offset The number of bytes sent/received since the beginning of the transfer.
 * @param total The total number of bytes to be sent/received.
 */
typedef void (*LinphoneChatMessageCbsFileTransferProgressIndicationCb)(LinphoneChatMessage *message, LinphoneContent* content, size_t offset, size_t total);

/**
 * Callback used to notify an ephemeral message that its lifespan before disappearing has started to decrease.
 * This callback is called when the ephemeral message is read by the receiver.
 * @param message #LinphoneChatMessage object @notnil
 */
typedef void (*LinphoneChatMessageCbsEphemeralMessageTimerStartedCb)(LinphoneChatMessage* message);

/**
 * Call back used to notify ephemeral message is deleted.
 * @param message #LinphoneChatMessage object @notnil
 */
typedef void (*LinphoneChatMessageCbsEphemeralMessageDeletedCb)(LinphoneChatMessage* message);

/**
 * Is composing notification callback prototype.
 * @param chat_room #LinphoneChatRoom involved in the conversation @notnil
 * @param remote_address The #LinphoneAddress that has sent the is-composing notification @notnil
 * @param is_composing A boolean value telling whether the remote is composing or not
 */
typedef void (*LinphoneChatRoomCbsIsComposingReceivedCb) (LinphoneChatRoom *chat_room, const LinphoneAddress *remote_address, bool_t is_composing);

/**
 * Callback used to notify a chat room that a message has been received.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param message The #LinphoneChatMessage that has been received @notnil
 */
typedef void (*LinphoneChatRoomCbsMessageReceivedCb) (LinphoneChatRoom *chat_room, LinphoneChatMessage *message);

/**
 * Callback used to notify a chat room that many chat messages have been received.
 * Only called when aggregation is enabled (aka [sip] chat_messages_aggregation == 1 or using linphone_core_set_chat_messages_aggregation_enabled()),
 * it replaces the single message received callback.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param chat_messages The \bctbx_list{LinphoneChatMessage} list of events to be notified @notnil
 */
typedef void (*LinphoneChatRoomCbsMessagesReceivedCb) (LinphoneChatRoom *chat_room, const bctbx_list_t *chat_messages);

/**
 * Callback used to notify a chat room that an event log has been created.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param event_log #LinphoneEventLog The event to be notified @notnil
 */
typedef void (*LinphoneChatRoomCbsNewEventCb) (LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log);

/**
 * Callback used to notify a chat room that many event logs have been created.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param event_logs The \bctbx_list{LinphoneEventLog} list of events to be notified @notnil
 */
typedef void (*LinphoneChatRoomCbsNewEventsCb) (LinphoneChatRoom *chat_room, const bctbx_list_t *event_logs);

/**
 * Callback used to notify a chat room that a chat message has been received.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param event_log #LinphoneEventLog The event to be notified @notnil
 */
typedef void (*LinphoneChatRoomCbsChatMessageReceivedCb) (LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log);

/**
 * Callback used to notify a chat room that one or many chat messages have been received.
 * Only called when aggregation is enabled (aka [sip] chat_messages_aggregation == 1 or using linphone_core_set_chat_messages_aggregation_enabled()),
 * it replaces the single chat message received callback.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param event_logs The \bctbx_list{LinphoneEventLog} list of events to be notified @notnil
 */
typedef void (*LinphoneChatRoomCbsChatMessagesReceivedCb) (LinphoneChatRoom *chat_room, const bctbx_list_t *event_logs);

/**
 * Callback used to notify a chat room that a chat message is being sent.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param event_log #LinphoneEventLog The event to be notified @notnil
 */
typedef void (*LinphoneChatRoomCbsChatMessageSendingCb) (LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log);

/**
 * Callback used to notify a chat room that a chat message has been sent.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param event_log #LinphoneEventLog The event to be notified @notnil
 */
typedef void (*LinphoneChatRoomCbsChatMessageSentCb) (LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log);

/**
 * Callback used to notify a chat room that a participant has been added.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param event_log #LinphoneEventLog The event to be notified @notnil
 */
typedef void (*LinphoneChatRoomCbsParticipantAddedCb) (LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log);

/**
 * Callback used to notify a chat room that a participant has been removed.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param event_log #LinphoneEventLog The event to be notified @notnil
 */
typedef void (*LinphoneChatRoomCbsParticipantRemovedCb) (LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log);

/**
 * Callback used to notify a chat room that the admin status of a participant has been changed.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param event_log #LinphoneEventLog The event to be notified @notnil
 */
typedef void (*LinphoneChatRoomCbsParticipantAdminStatusChangedCb) (LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log);

/**
 * Callback used to notify a chat room state has changed.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param newState The new #LinphoneChatRoomState of the chat room
 */
typedef void (*LinphoneChatRoomCbsStateChangedCb) (LinphoneChatRoom *chat_room, LinphoneChatRoomState newState);

/**
 * Callback used to notify a security event in the chat room.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param event_log #LinphoneEventLog The event to be notified @notnil
 */
typedef void (*LinphoneChatRoomCbsSecurityEventCb) (LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log);

/**
 * Callback used to notify that the subject of a chat room has changed.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param event_log #LinphoneEventLog The event to be notified @notnil
 */
typedef void (*LinphoneChatRoomCbsSubjectChangedCb) (LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log);

/**
 * Callback used to notify a chat room that a message has been received but we were unable to decrypt it
 * @param chat_room #LinphoneChatRoom involved in this conversation @notnil
 * @param message The #LinphoneChatMessage that has been received @notnil
 */
typedef void (*LinphoneChatRoomCbsUndecryptableMessageReceivedCb) (LinphoneChatRoom *chat_room, LinphoneChatMessage *message);

/**
 * Callback used to notify a chat room that a participant has been added.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param event_log #LinphoneEventLog The event to be notified @notnil
 */
typedef void (*LinphoneChatRoomCbsParticipantDeviceAddedCb) (LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log);

/**
 * Callback used to notify a chat room that a participant has been removed.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param event_log #LinphoneEventLog The event to be notified @notnil
 */
typedef void (*LinphoneChatRoomCbsParticipantDeviceRemovedCb) (LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log);

/**
 * Callback used to notify a conference that a participant device has changed state
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param event_log #LinphoneEventLog The event to be notified @notnil
 * @param state new participant device state
 */
typedef void (*LinphoneChatRoomCbsParticipantDeviceStateChangedCb) (LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log, const LinphoneParticipantDeviceState state);

/**
 * Callback used to notify a conference that the media availability of a participant device has been changed.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param event_log #LinphoneEventLog The event to be notified @notnil
 */
typedef void (*LinphoneChatRoomCbsParticipantDeviceMediaAvailabilityChangedCb) (LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log);

/**
 * Callback used to notify a chat room has been joined.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param event_log #LinphoneEventLog The event to be notified @notnil
 */
typedef void (*LinphoneChatRoomCbsConferenceJoinedCb) (LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log);

/**
 * Callback used to notify a chat room has been left.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param event_log #LinphoneEventLog The event to be notified @notnil
 */
typedef void (*LinphoneChatRoomCbsConferenceLeftCb) (LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log);

/**
 * Callback used to notify a chat room that an ephemeral related event has been generated.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param event_log #LinphoneEventLog The event to be notified @notnil
 */
typedef void (*LinphoneChatRoomCbsEphemeralEventCb) (LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log);

/**
 * Callback used to notify a chat room that the lifespan of an ephemeral message before disappearing has started to decrease.
 * This callback is called when the ephemeral message is read by the receiver.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param event_log #LinphoneEventLog The event to be notified @notnil
 */
typedef void (*LinphoneChatRoomCbsEphemeralMessageTimerStartedCb) (LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log);

/**
 * Callback used to notify a chat room that an ephemeral message has been deleted.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param event_log #LinphoneEventLog The event to be notified @notnil
 */
typedef void (*LinphoneChatRoomCbsEphemeralMessageDeletedCb) (LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log);

/**
 * Callback used when a group chat room is created server-side to generate the address of the chat room.
 * The function linphone_chat_room_set_conference_address() needs to be called by this callback.
 * @param chat_room #LinphoneChatRoom object @notnil
 */
typedef void (*LinphoneChatRoomCbsConferenceAddressGenerationCb) (LinphoneChatRoom *chat_room);

/**
 * Callback used when a group chat room server is subscribing to registration state of a participant.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param participant_address #LinphoneAddress object @notnil
 */
typedef void (*LinphoneChatRoomCbsParticipantRegistrationSubscriptionRequestedCb) (LinphoneChatRoom *chat_room, const LinphoneAddress *participant_address);

/**
 * Callback used when a group chat room server is unsubscribing to registration state of a participant.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param participant_address #LinphoneAddress object @notnil
 */
typedef void (*LinphoneChatRoomCbsParticipantRegistrationUnsubscriptionRequestedCb) (LinphoneChatRoom *chat_room, const LinphoneAddress *participant_address);

/**
 * Callback used to tell the core whether or not to store the incoming message in db or not using linphone_chat_message_set_to_be_stored().
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param message The #LinphoneChatMessage that is being received @notnil
 */
typedef void (*LinphoneChatRoomCbsShouldChatMessageBeStoredCb) (LinphoneChatRoom *chat_room, LinphoneChatMessage *message);

/**
 * Callback used to notify a participant state has changed in a message of this chat room.
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param message The #LinphoneChatMessage for which a participant has it's state changed @notnil
 * @param state The #LinphoneParticipantImdnState @notnil
 */
typedef void (*LinphoneChatRoomCbsChatMessageParticipantImdnStateChangedCb) (LinphoneChatRoom *chat_room, LinphoneChatMessage *message, const LinphoneParticipantImdnState *state);

/**
* @}
**/	
	
/**
 * @addtogroup misc
 * @{
 */

/**
 * Callback used to notify when results are received.
 * @param magic_search #LinphoneMagicSearch object @notnil
 */
typedef void (*LinphoneMagicSearchCbsSearchResultsReceivedCb)(LinphoneMagicSearch* magic_search);

/**
 * Callback used to notify when LDAP have more results available.
 * @param magic_search #LinphoneMagicSearch object @notnil
 * @param ldap #LinphoneLdap object @notnil
 */
typedef void (*LinphoneMagicSearchCbsLdapHaveMoreResultsCb)(LinphoneMagicSearch* magic_search, LinphoneLdap * ldap);

/**
* @}
**/	
	
/************ */
/* DEPRECATED */
/* ********** */
/**
 * @addtogroup chatroom
 * @{
 */
 
 /**
 * Call back used to notify message delivery status
 * @param message #LinphoneChatMessage object @notnil
 * @param state #LinphoneChatMessageState
 * @param user_data application user data
 * @deprecated 03/07/2018 Use #LinphoneChatMessageCbsMsgStateChangedCb instead.
 * @donotwrap
 */
typedef void (*LinphoneChatMessageStateChangedCb)(LinphoneChatMessage* message, LinphoneChatMessageState state, void* user_data);

/**
 * @}
**/


/**
 * @addtogroup conference
 * @{
 */

/**
 * Callback used to notify a conference that a participant has been added.
 * @param[in] conference #LinphoneConference object @notnil
 * @param[in] participant #LinphoneParticipant that has been added to the conference @notnil
 */
typedef void (*LinphoneConferenceCbsParticipantAddedCb) (LinphoneConference *conference, const LinphoneParticipant *participant);

/**
 * Callback used to notify a conference that a participant has been removed.
 * @param[in] conference #LinphoneConference object @notnil
 * @param[in] participant #LinphoneParticipant that has been removed to the conference @notnil
 */
typedef void (*LinphoneConferenceCbsParticipantRemovedCb) (LinphoneConference *conference, const LinphoneParticipant *participant);

/**
 * Callback used to notify a conference that a participant device has changed state
 * @param[in] conference #LinphoneConference object @notnil
 * @param[in] device #LinphoneParticipantDevice who change state @notnil
 * @param[in] state new participant device state
 */
typedef void (*LinphoneConferenceCbsParticipantDeviceStateChangedCb) (LinphoneConference *conference, const LinphoneParticipantDevice *device, const LinphoneParticipantDeviceState state);

/**
 * Callback used to notify a conference that the media availability of a participant device has been changed.
 * @param[in] conference #LinphoneConference object @notnil
 * @param[in] device #LinphoneParticipantDevice whose media availability changed has changed @notnil
 */
typedef void (*LinphoneConferenceCbsParticipantDeviceMediaAvailabilityChangedCb) (LinphoneConference *conference, const LinphoneParticipantDevice *device);

/**
 * Callback used to notify a conference that the media capability of a participant device has been changed.
 * @param[in] conference #LinphoneConference object @notnil
 * @param[in] device #LinphoneParticipantDevice whose media capability changed has changed @notnil
 */
typedef void (*LinphoneConferenceCbsParticipantDeviceMediaCapabilityChangedCb) (LinphoneConference *conference, const LinphoneParticipantDevice *device);

/**
 * Callback used to notify a conference that the admin status of a participant has been changed.
 * @param[in] conference #LinphoneConference object @notnil
 * @param[in] participant #LinphoneParticipant whose admin status has changed @notnil
 */
typedef void (*LinphoneConferenceCbsParticipantAdminStatusChangedCb) (LinphoneConference *conference, const LinphoneParticipant *participant);

/**
 * Callback used to notify a conference state has changed.
 * @param[in] conference #LinphoneConference object @notnil
 * @param[in] newState The new state of the conference
 */
typedef void (*LinphoneConferenceCbsStateChangedCb) (LinphoneConference *conference, LinphoneConferenceState newState);

/**
 * Callback used to notify that the available media of a conference has changed.
 * @param[in] conference #LinphoneConference object @notnil
 */
typedef void (*LinphoneConferenceCbsAvailableMediaChangedCb) (LinphoneConference *conference);

/**
 * Callback used to notify that the subject of a conference has changed.
 * @param[in] conference #LinphoneConference object @notnil
 * @param[in] subject subject of the conference @notnil
 */
typedef void (*LinphoneConferenceCbsSubjectChangedCb) (LinphoneConference *conference, const char *subject);

/**
 * Callback used to notify that a participant device is speaking or isn't speaking anymore.
 * @param[in] conference #LinphoneConference object @notnil
 * @param[in] participant_device the participant device @notnil
 * @param[in] is_speaking TRUE if is speaking, FALSE otherwise
 */
typedef void (*LinphoneConferenceCbsParticipantDeviceIsSpeakingChangedCb) (LinphoneConference *conference, const LinphoneParticipantDevice *participant_device, bool_t is_speaking);

/**
 * Callback used to notify that a participant device is muted or is no longer muted.
 * @param[in] conference #LinphoneConference object @notnil
 * @param[in] participant_device the participant device @notnil
 * @param[in] is_muted TRUE if is muted, FALSE otherwise
 */
typedef void (*LinphoneConferenceCbsParticipantDeviceIsMutedCb) (LinphoneConference *conference, const LinphoneParticipantDevice *participant_device, bool_t is_muted);

/**
 * Callback used to notify that the audio device of a conference has changed.
 * @param[in] conference #LinphoneConference object @notnil
 * @param[in] audio_device audio device of the conference @notnil
 */
typedef void (*LinphoneConferenceCbsAudioDeviceChangedCb) (LinphoneConference *conference, const  LinphoneAudioDevice *audio_device);

/**
 * Callback used to notify a conference that a participant has been added.
 * @param[in] conference #LinphoneConference object @notnil
 * @param[in] participant_device #LinphoneParticipantDevice that has been added to the conference @notnil
 */
typedef void (*LinphoneConferenceCbsParticipantDeviceAddedCb) (LinphoneConference *conference, const LinphoneParticipantDevice *participant_device);

/**
 * Callback used to notify a conference that a participant has been removed.
 * @param[in] conference #LinphoneConference object @notnil
 * @param[in] participant_device #LinphoneParticipantDevice that has been removed to the conference @notnil
 */
typedef void (*LinphoneConferenceCbsParticipantDeviceRemovedCb) (LinphoneConference *conference, const LinphoneParticipantDevice *participant_device);

/**
 * Callback used to notify which participant device video is being displayed as "actively speaking".
 * @param[in] conference #LinphoneConference object @notnil
 * @param[in] participant_device the participant device currently displayed as active speaker @notnil
 */
typedef void (*LinphoneConferenceCbsActiveSpeakerParticipantDeviceCb) (LinphoneConference *conference, const LinphoneParticipantDevice *participant_device);

/**
 * Callback used to notify that is this participant device speaking has changed.
 * @param[in] participant_device #LinphoneParticipantDevice object @notnil
 * @param[in] is_speaking is this participant device speaking
 */
typedef void (*LinphoneParticipantDeviceCbsIsSpeakingChangedCb) (LinphoneParticipantDevice *participant_device, bool_t is_speaking);

/**
 * Callback used to notify that this participant device is muted or is no longer muted.
 * @param[in] participant_device #LinphoneParticipantDevice object @notnil
 * @param[in] is_muted is this participant device muted
 */
typedef void (*LinphoneParticipantDeviceCbsIsMutedCb) (LinphoneParticipantDevice *participant_device, bool_t is_muted);

/**
 * Callback used to notify that participant device changed state
 * @param[in] participant_device #LinphoneParticipantDevice object @notnil
 */
typedef void (*LinphoneParticipantDeviceCbsStateChangedCb) (LinphoneParticipantDevice *participant_device, const LinphoneParticipantDeviceState state);

/**
 * Callback used to notify that participant device stream capability has changed.
 * @param[in] participant_device #LinphoneParticipantDevice object @notnil
 * @param[in] direction  participant device's audio direction
 */
typedef void (*LinphoneParticipantDeviceCbsStreamCapabilityChangedCb) (LinphoneParticipantDevice *participant_device, LinphoneMediaDirection direction, const LinphoneStreamType stream_type);

/**
 * Callback used to notify that participant device stream availability has changed.
 * @param[in] participant_device #LinphoneParticipantDevice object @notnil
 * @param[in] direction  participant device's video direction
 */
typedef void (*LinphoneParticipantDeviceCbsStreamAvailabilityChangedCb) (LinphoneParticipantDevice *participant_device, bool_t available, const LinphoneStreamType stream_type);

/**
 * Callback to notify that there are errors from the video rendering of the participant device.
 * Check #LinphoneCallCbsVideoDisplayErrorOccurredCb for more details.
 *
 * @param[in] participant_device #LinphoneParticipantDevice object @notnil
 * @param[in] error_code the error code coming from the display render.
 */
typedef void (*LinphoneParticipantDeviceCbsVideoDisplayErrorOccurredCb)(LinphoneParticipantDevice *participant_device,
                                                                        int error_code);


/**
 * Callback for notifying when a registration state has changed for the conference scheduler.
 * @param conference_scheduler #LinphoneConferenceScheduler object whose state has changed. @notnil
 * @param state The current #LinphoneConferenceSchedulerState.
 */
typedef void (*LinphoneConferenceSchedulerCbsStateChangedCb) (LinphoneConferenceScheduler *conference_scheduler, LinphoneConferenceSchedulerState state);

/**
 * Callback for notifying when conference invitations have been sent. 
 * In case of error for some participants, their addresses will be given as parameter.
 * @param conference_scheduler #LinphoneConferenceScheduler object whose state has changed. @notnil
 * @param failed_invitations a list of addresses for which invitation couldn't be sent. \bctbx_list{LinphoneAddress} @maybenil
 */
typedef void (*LinphoneConferenceSchedulerCbsInvitationsSentCb) (LinphoneConferenceScheduler *conference_scheduler, const bctbx_list_t *failed_invitations);

/**
 * @}
**/


	
	
#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_CALLBACKS_H_
