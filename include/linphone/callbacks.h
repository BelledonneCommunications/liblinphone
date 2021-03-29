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

#ifndef LINPHONE_CALLBACKS_H_
#define LINPHONE_CALLBACKS_H_


#include "linphone/types.h"
#include "linphone/api/c-callbacks.h"


/**
 * @addtogroup chatroom
 * @{
 */

/**
 * @}
**/

/**
 * @addtogroup initializing
 * @{
**/

/**
 * Callback notifying that a new #LinphoneCall (either incoming or outgoing) has been created.
 * @param core #LinphoneCore object that has created the call @notnil
 * @param call The newly created #LinphoneCall object @notnil
 */
typedef void (*LinphoneCoreCbsCallCreatedCb)(LinphoneCore *core, LinphoneCall *call);

/**
 * Global state notification callback.
 * @param core the #LinphoneCore. @notnil
 * @param state the #LinphoneGlobalState
 * @param message informational message. @notnil
 */
typedef void (*LinphoneCoreCbsGlobalStateChangedCb)(LinphoneCore *core, LinphoneGlobalState state, const char *message);

/**
 * Old name of #LinphoneCoreCbsGlobalStateChangedCb.
 */
typedef LinphoneCoreCbsGlobalStateChangedCb LinphoneCoreGlobalStateChangedCb;

/**
 * Call state notification callback.
 * @param core the #LinphoneCore @notnil
 * @param call the #LinphoneCall object whose state is changed. @notnil
 * @param state the new #LinphoneCallState of the call
 * @param message a non NULL informational message about the state. @notnil
 */
typedef void (*LinphoneCoreCbsCallStateChangedCb)(LinphoneCore *core, LinphoneCall *call, LinphoneCallState state, const char *message);

/**
 * Old name of #LinphoneCoreCbsCallStateChangedCb.
 */
typedef LinphoneCoreCbsCallStateChangedCb LinphoneCoreCallStateChangedCb;

/**
 * Call encryption changed callback.
 * @param core the #LinphoneCore @notnil
 * @param call the #LinphoneCall on which encryption is changed. @notnil
 * @param media_encryption_enabled whether encryption is activated.
 * @param authentication_token an authentication_token, currently set for ZRTP kind of encryption only. @maybenil
 */
typedef void (*LinphoneCoreCbsCallEncryptionChangedCb)(LinphoneCore *core, LinphoneCall *call, bool_t media_encryption_enabled, const char *authentication_token);

/**
 * Old name of #LinphoneCoreCbsCallEncryptionChangedCb.
 */
typedef LinphoneCoreCbsCallEncryptionChangedCb LinphoneCoreCallEncryptionChangedCb;

/**
 * Registration state notification callback prototype
 * @param core the #LinphoneCore @notnil
 * @param proxy_config the #LinphoneProxyConfig which state has changed @notnil
 * @param state the current #LinphoneRegistrationState
 * @param message a non NULL informational message about the state @notnil
 * @ingroup Proxies
 */
typedef void (*LinphoneCoreCbsRegistrationStateChangedCb)(LinphoneCore *core, LinphoneProxyConfig *proxy_config, LinphoneRegistrationState state, const char *message);

/**
 * Old name of #LinphoneCoreCbsRegistrationStateChangedCb.
 */
typedef LinphoneCoreCbsRegistrationStateChangedCb LinphoneCoreRegistrationStateChangedCb;

/**
 * Report status change for a friend previously added to the #LinphoneCore with linphone_core_add_friend().
 * @param core #LinphoneCore object @notnil
 * @param linphone_friend Updated #LinphoneFriend @notnil
 */
typedef void (*LinphoneCoreCbsNotifyPresenceReceivedCb)(LinphoneCore *core, LinphoneFriend * linphone_friend);

/**
 * Old name of #LinphoneCoreCbsNotifyPresenceReceivedCb.
 */
typedef LinphoneCoreCbsNotifyPresenceReceivedCb LinphoneCoreNotifyPresenceReceivedCb;

/**
 * Reports presence model change for a specific URI or phone number of a friend
 * @param core #LinphoneCore object @notnil
 * @param linphone_friend #LinphoneFriend object @notnil
 * @param uri_or_tel The URI or phone number for which the presence model has changed @notnil
 * @param presence_model The new #LinphonePresenceModel @notnil
 */
typedef void (*LinphoneCoreCbsNotifyPresenceReceivedForUriOrTelCb)(LinphoneCore *core, LinphoneFriend *linphone_friend, const char *uri_or_tel, const LinphonePresenceModel *presence_model);

/**
 * Old name of #LinphoneCoreCbsNotifyPresenceReceivedForUriOrTelCb.
 */
typedef LinphoneCoreCbsNotifyPresenceReceivedForUriOrTelCb LinphoneCoreNotifyPresenceReceivedForUriOrTelCb;

/**
 * Reports that a new subscription request has been received and wait for a decision.
 * @note A subscription request is notified by this function only if the #LinphoneSubscribePolicy for the
 * given #LinphoneFriend has been set to #LinphoneSPWait. See linphone_friend_set_inc_subscribe_policy().
 * @param core #LinphoneCore object @notnil
 * @param linphone_friend The #LinphoneFriend aimed by the subscription. @notnil
 * @param url URI of the subscriber @notnil
 */
typedef void (*LinphoneCoreCbsNewSubscriptionRequestedCb)(LinphoneCore *core, LinphoneFriend *linphone_friend, const char *url);

/**
 * Old name of #LinphoneCoreCbsNewSubscriptionRequestedCb.
 */
typedef LinphoneCoreCbsNewSubscriptionRequestedCb LinphoneCoreNewSubscriptionRequestedCb;

/**
 * Callback for requesting authentication information to application or user.
 * @param core the #LinphoneCore @notnil
 * @param auth_info a #LinphoneAuthInfo pre-filled with username, realm and domain values as much as possible @notnil
 * @param method the type of authentication requested as #LinphoneAuthMethod enum @notnil
 * Application shall reply to this callback using linphone_core_add_auth_info().
 */
typedef void (*LinphoneCoreCbsAuthenticationRequestedCb)(LinphoneCore *core, LinphoneAuthInfo *auth_info, LinphoneAuthMethod method);

/**
 * Old name of #LinphoneCoreCbsAuthenticationRequestedCb.
 */
typedef LinphoneCoreCbsAuthenticationRequestedCb LinphoneCoreAuthenticationRequestedCb;

/**
 * Callback to notify a new call-log entry has been added.
 * This is done typically when a call terminates.
 * @param core the #LinphoneCore @notnil
 * @param call_log the new #LinphoneCallLog entry added. @notnil
 */
typedef void (*LinphoneCoreCbsCallLogUpdatedCb)(LinphoneCore *core, LinphoneCallLog *call_log);

/**
 * Old name of #LinphoneCoreCbsCallLogUpdatedCb.
 */
typedef LinphoneCoreCbsCallLogUpdatedCb LinphoneCoreCallLogUpdatedCb;

/**
 * Callback to notify the callid of a call has been updated.
 * This is done typically when a call retry.
 * @param core the #LinphoneCore @notnil
 * @param previous_call_id the previous callid. @notnil
 * @param current_call_id the new callid. @notnil
 */
typedef void (*LinphoneCoreCbsCallIdUpdatedCb)(LinphoneCore *core, const char *previous_call_id, const char *current_call_id);

/**
 * Old name of #LinphoneCoreCbsCallIdUpdatedCb.
 */
typedef LinphoneCoreCbsCallIdUpdatedCb LinphoneCoreCallIdUpdatedCb;

/**
 * Chat message callback prototype.
 * @param core #LinphoneCore object @notnil
 * @param chat_room #LinphoneChatRoom involved in this conversation. Can be created by the framework in case the From-URI is not present in any chat room. @notnil
 * @param message #LinphoneChatMessage incoming message @notnil
 */
typedef void (*LinphoneCoreCbsMessageReceivedCb)(LinphoneCore *core, LinphoneChatRoom *chat_room, LinphoneChatMessage *message);

/**
 * Called after the #send method of the #LinphoneChatMessage was called.
 * The message will be in state InProgress.
 * In case of resend this callback won't be called.
 * @param core #LinphoneCore object @notnil
 * @param chat_room #LinphoneChatRoom involved in this conversation. Can be be created by the framework in case the From-URI is not present in any chat room. @notnil
 * @param message #LinphoneChatMessage outgoing message @notnil
 */
typedef void (*LinphoneCoreCbsMessageSentCb)(LinphoneCore *core, LinphoneChatRoom *chat_room, LinphoneChatMessage *message);

/**
 * Old name of #LinphoneCoreCbsMessageReceivedCb.
 */
typedef LinphoneCoreCbsMessageReceivedCb LinphoneCoreMessageReceivedCb;

/**
 * Chat room marked as read callback
 * @param core #LinphoneCore object @notnil
 * @param chat_room #LinphoneChatRoom that has been marked as read. @notnil
 */
typedef void (*LinphoneCoreCbsChatRoomReadCb)(LinphoneCore *core, LinphoneChatRoom *chat_room);

/**
 * Chat message not decrypted callback prototype
 * @param core #LinphoneCore object @notnil
 * @param chat_room #LinphoneChatRoom involved in this conversation. Can be be created by the framework in case the from-URI is not present in any chat room. @notnil
 * @param message #LinphoneChatMessage incoming message @notnil
 */
typedef void (*LinphoneCoreCbsMessageReceivedUnableDecryptCb)(LinphoneCore *core, LinphoneChatRoom *chat_room, LinphoneChatMessage *message);

/**
 * File transfer receive callback prototype. This function is called by the core upon an incoming File transfer is started. This function may be call several time for the same file in case of large file.
 * @param core #LinphoneCore object @notnil
 * @param message #LinphoneChatMessage message from which the body is received. @notnil
 * @param content #LinphoneContent incoming content information @notnil
 * @param buffer pointer to the received data @maybenil
 * @param size number of bytes to be read from buff. 0 means end of file.
 */
typedef void (*LinphoneCoreFileTransferRecvCb)(LinphoneCore *core, LinphoneChatMessage *message, LinphoneContent* content, const char* buffer, size_t size);

/**
 * File transfer send callback prototype. This function is called by the core upon an outgoing file transfer is started. This function is called until size is set to 0.
 * @param core #LinphoneCore object @notnil
 * @param message #LinphoneChatMessage message from which the body is received. @notnil
 * @param content #LinphoneContent outgoing content @notnil
 * @param buffer pointer to the buffer where data chunk shall be written by the app @notnil
 * @param size as input value, it represents the number of bytes expected by the framework. As output value, it means the number of bytes wrote by the application in the buffer. 0 means end of file.
 *
 */
typedef void (*LinphoneCoreFileTransferSendCb)(LinphoneCore *core, LinphoneChatMessage *message, LinphoneContent* content, char* buffer, size_t* size);

/**
 * File transfer progress indication callback prototype.
 * @param core #LinphoneCore object @notnil
 * @param message #LinphoneChatMessage message from which the body is received. @notnil
 * @param content #LinphoneContent incoming content information @notnil
 * @param offset The number of bytes sent/received since the beginning of the transfer.
 * @param total The total number of bytes to be sent/received.
 */
typedef void (*LinphoneCoreFileTransferProgressIndicationCb)(LinphoneCore *core, LinphoneChatMessage *message, LinphoneContent* content, size_t offset, size_t total);

/**
 * Is composing notification callback prototype.
 * @param core #LinphoneCore object @notnil
 * @param chat_room #LinphoneChatRoom involved in the conversation. @notnil
 */
typedef void (*LinphoneCoreCbsIsComposingReceivedCb)(LinphoneCore *core, LinphoneChatRoom *chat_room);

/**
 * Old name of #LinphoneCoreCbsIsComposingReceivedCb.
 */
typedef LinphoneCoreCbsIsComposingReceivedCb LinphoneCoreIsComposingReceivedCb;

/**
 * Callback for being notified of DTMFs received.
 * @param core the #LinphoneCore @notnil
 * @param call the #LinphoneCall that received the dtmf @notnil
 * @param dtmf the ascii code of the dtmf
 */
typedef void (*LinphoneCoreCbsDtmfReceivedCb)(LinphoneCore* core, LinphoneCall *call, int dtmf);

/**
 * Old name of #LinphoneCoreCbsDtmfReceivedCb.
 */
typedef LinphoneCoreCbsDtmfReceivedCb LinphoneCoreDtmfReceivedCb;

/** 
 * Callback prototype for when a refer is received
 * @param core the #LinphoneCore @notnil
 * @param refer_to the address of the refer @notnil
 */
typedef void (*LinphoneCoreCbsReferReceivedCb)(LinphoneCore *core, const char *refer_to);

/**
 * Old name of #LinphoneCoreCbsReferReceivedCb.
 */
typedef LinphoneCoreCbsReferReceivedCb LinphoneCoreReferReceivedCb;

/** 
 * Callback prototype when using the buddy plugin
 * @param core the #LinphoneCore @notnil
 * @param linphone_friend the #LinphoneFriend that has been updated @notnil
 */ 
typedef void (*LinphoneCoreCbsBuddyInfoUpdatedCb)(LinphoneCore *core, LinphoneFriend *linphone_friend);

/**
 * Old name of #LinphoneCoreCbsBuddyInfoUpdatedCb.
 */
typedef LinphoneCoreCbsBuddyInfoUpdatedCb LinphoneCoreBuddyInfoUpdatedCb;

/**
 * Callback for notifying progresses of transfers.
 * @param core the #LinphoneCore @notnil
 * @param transfered the #LinphoneCall that was transfered @notnil
 * @param call_state the #LinphoneCallState of the call to transfer target at the far end.
 */
typedef void (*LinphoneCoreCbsTransferStateChangedCb)(LinphoneCore *core, LinphoneCall *transfered, LinphoneCallState call_state);

/**
 * Old name of LinphoneCoreCbsTransferStateChangedCb.
 */
typedef LinphoneCoreCbsTransferStateChangedCb LinphoneCoreTransferStateChangedCb;

/**
 * Callback for receiving quality statistics for calls.
 * @param core the #LinphoneCore @notnil
 * @param call the call @notnil
 * @param call_stats the call statistics. @notnil
 */
typedef void (*LinphoneCoreCbsCallStatsUpdatedCb)(LinphoneCore *core, LinphoneCall *call, const LinphoneCallStats *call_stats);

/**
 * Old name of #LinphoneCoreCbsCallStatsUpdatedCb.
 */
typedef LinphoneCoreCbsCallStatsUpdatedCb LinphoneCoreCallStatsUpdatedCb;

/**
 * Callback prototype for receiving info messages.
 * @param core the #LinphoneCore @notnil
 * @param call the call whose info message belongs to. @notnil
 * @param message the info message. @notnil
 */
typedef void (*LinphoneCoreCbsInfoReceivedCb)(LinphoneCore *core, LinphoneCall *call, const LinphoneInfoMessage *message);

/**
 * Old name of #LinphoneCoreCbsInfoReceivedCb.
 */
typedef LinphoneCoreCbsInfoReceivedCb LinphoneCoreInfoReceivedCb;

/**
 * Callback prototype for configuring status changes notification
 * @param core the #LinphoneCore @notnil
 * @param status the current #LinphoneConfiguringState
 * @param message informational message. @maybenil
 */
typedef void (*LinphoneCoreCbsConfiguringStatusCb)(LinphoneCore *core, LinphoneConfiguringState status, const char *message);

/**
 * Old name of #LinphoneCoreCbsConfiguringStatusCb.
 */
typedef LinphoneCoreCbsConfiguringStatusCb LinphoneCoreConfiguringStatusCb;

/**
 * Callback prototype for reporting network change either automatically detected or notified by #linphone_core_set_network_reachable().
 * @param core the #LinphoneCore @notnil
 * @param reachable true if network is reachable.
 */
typedef void (*LinphoneCoreCbsNetworkReachableCb)(LinphoneCore *core, bool_t reachable);

/**
 * Old name of #LinphoneCoreCbsNetworkReachableCb.
 */
typedef LinphoneCoreCbsNetworkReachableCb LinphoneCoreNetworkReachableCb;

/**
 * Callback prototype for reporting log collection upload state change.
 * @param core #LinphoneCore object @notnil
 * @param state The state of the log collection upload
 * @param info Additional information: error message in case of error state, URL of uploaded file in case of success. @notnil
 */
typedef void (*LinphoneCoreCbsLogCollectionUploadStateChangedCb)(LinphoneCore *core, LinphoneCoreLogCollectionUploadState state, const char *info);

/**
 * Old name of #LinphoneCoreCbsLogCollectionUploadStateChangedCb.
 */
typedef LinphoneCoreCbsLogCollectionUploadStateChangedCb LinphoneCoreLogCollectionUploadStateChangedCb;

/**
 * Callback prototype for reporting log collection upload progress indication.
 * @param core #LinphoneCore object @notnil
 * @param offset the number of bytes sent since the start of the upload
 * @param total the total number of bytes to upload
 */
typedef void (*LinphoneCoreCbsLogCollectionUploadProgressIndicationCb)(LinphoneCore *core, size_t offset, size_t total);

/**
 * Old name of #LinphoneCoreCbsLogCollectionUploadProgressIndicationCb.
 */
typedef LinphoneCoreCbsLogCollectionUploadProgressIndicationCb LinphoneCoreLogCollectionUploadProgressIndicationCb;

/**
 * Callback prototype for reporting when a friend list has been added to the core friends list.
 * @param core #LinphoneCore object @notnil
 * @param friend_list #LinphoneFriendList object @notnil
 */
typedef void (*LinphoneCoreCbsFriendListCreatedCb) (LinphoneCore *core, LinphoneFriendList *friend_list);

/**
 * Old name of #LinphoneCoreCbsFriendListCreatedCb.
 */
typedef LinphoneCoreCbsFriendListCreatedCb LinphoneCoreFriendListCreatedCb;

/**
 * Callback prototype for reporting when a friend list has been removed from the core friends list.
 * @param core #LinphoneCore object @notnil
 * @param friend_list #LinphoneFriendList object @notnil
 */
typedef void (*LinphoneCoreCbsFriendListRemovedCb) (LinphoneCore *core, LinphoneFriendList *friend_list);

/**
 * Old name of #LinphoneCoreCbsFriendListRemovedCb.
 */
typedef LinphoneCoreCbsFriendListRemovedCb LinphoneCoreFriendListRemovedCb;

/**
 * Callback prototype for reporting the result of a version update check.
 * @param core #LinphoneCore object @notnil
 * @param result The result of the version update check @notnil
 * @param url The url where to download the new version if the result is #LinphoneVersionUpdateCheckNewVersionAvailable @maybenil
 */
typedef void (*LinphoneCoreCbsVersionUpdateCheckResultReceivedCb) (LinphoneCore *core, LinphoneVersionUpdateCheckResult result, const char *version, const char *url);

/**
 * Callback prototype telling that a #LinphoneConference state has changed.
 * @param[in] core #LinphoneCore object @notnil
 * @param[in] conference The #LinphoneConference object for which the state has changed @notnil
 * @param[in] state the current #LinphoneChatRoomState
 */
typedef void (*LinphoneCoreCbsConferenceStateChangedCb) (LinphoneCore * core, LinphoneConference *conference, LinphoneConferenceState state);

/**
 * Callback prototype telling that a #LinphoneChatRoom state has changed.
 * @param core #LinphoneCore object @notnil
 * @param chat_room The #LinphoneChatRoom object for which the state has changed @notnil
 * @param state the current #LinphoneChatRoomState
 */
typedef void (*LinphoneCoreCbsChatRoomStateChangedCb) (LinphoneCore *core, LinphoneChatRoom *chat_room, LinphoneChatRoomState state);

/**
 * Callback prototype telling that a #LinphoneChatRoom subject has changed.
 * @param core #LinphoneCore object @notnil
 * @param chat_room The #LinphoneChatRoom object for which the subject has changed @notnil
 */
typedef void (*LinphoneCoreCbsChatRoomSubjectChangedCb) (LinphoneCore *core, LinphoneChatRoom *chat_room);

/**
 * Callback prototype telling that a #LinphoneChatRoom ephemeral message has expired.
 * @param core #LinphoneCore object @notnil
 * @param chat_room The #LinphoneChatRoom object for which a message has expired. @notnil
 */
typedef void (*LinphoneCoreCbsChatRoomEphemeralMessageDeleteCb) (LinphoneCore *core, LinphoneChatRoom *chat_room);

/**
 * Callback prototype telling that an Instant Message Encryption Engine user registered on the server with or without success.
 * @param core #LinphoneCore object @notnil
 * @param status the return status of the registration action.
 * @param user_id the userId published on the encryption engine server @notnil
 * @param info information about failure @notnil
 */
typedef void (*LinphoneCoreCbsImeeUserRegistrationCb) (LinphoneCore *core, const bool_t status, const char *user_id, const char *info);

/**
 * Callback prototype telling the result of decoded qrcode
 * @param core #LinphoneCore object @notnil
 * @param result The result of the decoded qrcode @maybenil
 */
typedef void (*LinphoneCoreCbsQrcodeFoundCb)(LinphoneCore *core, const char *result);

/**
 * Callback prototype telling a call has started (incoming or outgoing) while there was no other call.
 * @param core #LinphoneCore object @notnil
 */
typedef void (*LinphoneCoreCbsFirstCallStartedCb)(LinphoneCore *core);

/**
 * Callback prototype telling the last call has ended (#LinphoneCore.get_calls_nb() returns 0)
 * @param core #LinphoneCore object @notnil
 */
typedef void (*LinphoneCoreCbsLastCallEndedCb)(LinphoneCore *core);

/**
 * Callback prototype telling that the audio device for at least one call has changed
 * @param core #LinphoneCore object @notnil
 * @param audio_device the newly used #LinphoneAudioDevice object @notnil
 */
typedef void (*LinphoneCoreCbsAudioDeviceChangedCb)(LinphoneCore *core, LinphoneAudioDevice *audio_device);

/**
 * Callback prototype telling the audio devices list has been updated.
 * Either a new device is available or a previously available device isn't anymore.
 * You can call linphone_core_get_audio_devices() to get the new list.
 * @param core #LinphoneCore object @notnil
 */
typedef void (*LinphoneCoreCbsAudioDevicesListUpdatedCb)(LinphoneCore *core);

/**
 * @}
**/

/**
 * @addtogroup event_api
 * @{
**/

/**
 * Callback prototype for notifying the application about notification received from the network.
 * @param core #LinphoneCore object @notnil
 * @param linphone_event the #LinphoneEvent received @notnil
 * @param notified_event The event as string @notnil
 * @param body the #LinphoneContent of the event @notnil
 */
typedef void (*LinphoneCoreCbsNotifyReceivedCb)(LinphoneCore *core, LinphoneEvent *linphone_event, const char *notified_event, const LinphoneContent *body);

/**
 * Old name of #LinphoneCoreCbsNotifyReceivedCb.
 */
typedef LinphoneCoreCbsNotifyReceivedCb LinphoneCoreNotifyReceivedCb;

/**
 * Callback prototype for notifying the application about subscription received from the network.
 * @param core #LinphoneCore object @notnil
 * @param linphone_event the #LinphoneEvent received @notnil
 * @param subscribe_event The event as string @notnil
 * @param body the #LinphoneContent of the event @notnil
 */
typedef void (*LinphoneCoreCbsSubscribeReceivedCb)(LinphoneCore *core, LinphoneEvent *linphone_event, const char *subscribe_event, const LinphoneContent *body);

/**
 * Old name of #LinphoneCoreCbsSubscribeReceivedCb.
 */
typedef LinphoneCoreCbsSubscribeReceivedCb LinphoneCoreSubscribeReceivedCb;

/**
 * Callback prototype for notifying the application about changes of subscription states, including arrival of new subscriptions.
 * @param core #LinphoneCore object @notnil
 * @param linphone_event the #LinphoneEvent @notnil
 * @param state the new #LinphoneSubscriptionState
 */
typedef void (*LinphoneCoreCbsSubscriptionStateChangedCb)(LinphoneCore *core, LinphoneEvent *linphone_event, LinphoneSubscriptionState state);

/**
 * Old name of #LinphoneCoreCbsSubscriptionStateChangedCb.
 */
typedef LinphoneCoreCbsSubscriptionStateChangedCb LinphoneCoreSubscriptionStateChangedCb;

/**
 * Callback prototype for notifying the application about changes of publish states.
 * @param core #LinphoneCore object @notnil
 * @param linphone_event the #LinphoneEvent @notnil
 * @param state the new #LinphonePublishState
 */
typedef void (*LinphoneCoreCbsPublishStateChangedCb)(LinphoneCore *core, LinphoneEvent *linphone_event, LinphonePublishState state);

/**
 * Old name of LinphoneCoreCbsPublishStateChangedCb.
 */
typedef LinphoneCoreCbsPublishStateChangedCb LinphoneCorePublishStateChangedCb;

/**
 * @}
**/

/**
 * @addtogroup event_api
 * @{
 */

/**
 * Callback used to notify the response to a sent NOTIFY
 * @param linphone_event The #LinphoneEvent object that has sent the NOTIFY and for which we received a response @notnil
**/
typedef void (*LinphoneEventCbsNotifyResponseCb)(const LinphoneEvent *linphone_event);

/**
 * @}
 */

/**
 * @addtogroup buddy_list
 * @{
 */

/**
 * Callback used to notify a new contact has been created on the CardDAV server and downloaded locally
 * @param friend_list The #LinphoneFriendList object the new contact is added to @notnil
 * @param linphone_friend The #LinphoneFriend object that has been created @notnil
**/
typedef void (*LinphoneFriendListCbsContactCreatedCb)(LinphoneFriendList *friend_list, LinphoneFriend *linphone_friend);

/**
 * Callback used to notify a contact has been deleted on the CardDAV server
 * @param friend_list The #LinphoneFriendList object a contact has been removed from @notnil
 * @param linphone_friend The #LinphoneFriend object that has been deleted @notnil
**/
typedef void (*LinphoneFriendListCbsContactDeletedCb)(LinphoneFriendList *friend_list, LinphoneFriend *linphone_friend);

/**
 * Callback used to notify a contact has been updated on the CardDAV server
 * @param friend_list The #LinphoneFriendList object in which a contact has been updated @notnil
 * @param new_friend The new #LinphoneFriend object corresponding to the updated contact @notnil
 * @param old_friend The old #LinphoneFriend object before update @notnil
**/
typedef void (*LinphoneFriendListCbsContactUpdatedCb)(LinphoneFriendList *friend_list, LinphoneFriend *new_friend, LinphoneFriend *old_friend);

/**
 * Callback used to notify the status of the synchronization has changed
 * @param friend_list The #LinphoneFriendList object for which the status has changed @notnil
 * @param status The new #LinphoneFriendListSyncStatus
 * @param message An additional information on the status update @notnil
**/
typedef void (*LinphoneFriendListCbsSyncStateChangedCb)(LinphoneFriendList *friend_list, LinphoneFriendListSyncStatus status, const char *message);

/**
 * Callback used to notify a list with all friends that have received presence information.
 * @param friend_list The #LinphoneFriendList object for which the status has changed @notnil
 * @param friends A \bctbx_list{LinphoneFriend} of the relevant friends @notnil
**/
typedef void (*LinphoneFriendListCbsPresenceReceivedCb)(LinphoneFriendList *friend_list, const bctbx_list_t *friends);

/**
 * @}
**/

/**
 * @addtogroup misc
 * @{
 */

/**
 * @brief Function prototype used by #linphone_core_cbs_set_ec_calibrator_result().
 * @param core The #LinphoneCore. @notnil
 * @param status The #LinphoneEcCalibratorStatus of the calibrator.
 * @param delay_ms The measured delay if available.
 */
typedef void (*LinphoneCoreCbsEcCalibrationResultCb)(LinphoneCore *core, LinphoneEcCalibratorStatus status, int delay_ms);

/**
 * @brief Function prototype used by #linphone_core_cbs_set_ec_calibrator_audio_init().
 * @param core The #LinphoneCore. @notnil
 */
typedef void (*LinphoneCoreCbsEcCalibrationAudioInitCb)(LinphoneCore *core);

/**
 * @brief Function prototype used by #linphone_core_cbs_set_ec_calibrator_audio_uninit().
 * @param core The #LinphoneCore. @notnil
 */
typedef void (*LinphoneCoreCbsEcCalibrationAudioUninitCb)(LinphoneCore *core);

/**
 * Callback to decrypt incoming #LinphoneChatMessage
 * @param engine The #LinphoneImEncryptionEngine object @notnil
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param message #LinphoneChatMessage object @notnil
 * @return -1 if nothing to be done, 0 on success or an integer > 0 for error
*/
typedef int (*LinphoneImEncryptionEngineCbsIncomingMessageCb)(LinphoneImEncryptionEngine *engine, LinphoneChatRoom *chat_room, LinphoneChatMessage *message);

/**
 * Callback to encrypt outgoing #LinphoneChatMessage
 * @param engine #LinphoneImEncryptionEngine object @notnil
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param message #LinphoneChatMessage object @notnil
 * @return -1 if nothing to be done, 0 on success or an integer > 0 for error
*/
typedef int (*LinphoneImEncryptionEngineCbsOutgoingMessageCb)(LinphoneImEncryptionEngine *engine, LinphoneChatRoom *chat_room, LinphoneChatMessage *message);

/**
 * Callback to know whether or not the engine will encrypt files before uploading them
 * @param engine #LinphoneImEncryptionEngine object @notnil
 * @param chat_room #LinphoneChatRoom object @notnil
 * @return TRUE if files will be encrypted, FALSE otherwise
*/
typedef bool_t (*LinphoneImEncryptionEngineCbsIsEncryptionEnabledForFileTransferCb)(LinphoneImEncryptionEngine *engine, LinphoneChatRoom *chat_room);

/**
 * Callback to generate the key used to encrypt the files before uploading them
 * Key can be stored in the #LinphoneContent object inside the #LinphoneChatMessage using linphone_content_set_key
 * @param engine #LinphoneImEncryptionEngine object @notnil
 * @param chat_room #LinphoneChatRoom object @notnil
 * @param message #LinphoneChatMessage object @notnil
*/
typedef void (*LinphoneImEncryptionEngineCbsGenerateFileTransferKeyCb)(LinphoneImEncryptionEngine *engine, LinphoneChatRoom *chat_room, LinphoneChatMessage *message);

/**
 * Callback to decrypt downloading file
 * @param engine #LinphoneImEncryptionEngine object @notnil
 * @param message #LinphoneChatMessage object @notnil
 * @param offset The current offset of the upload
 * @param buffer Encrypted data buffer @maybenil
 * @param size Size of the encrypted data buffer and maximum size of the decrypted data buffer
 * @param[out] decrypted_buffer Buffer in which to write the decrypted data which maximum size is size @notnil
 * @return -1 if nothing to be done, 0 on success or an integer > 0 for error
*/
typedef int (*LinphoneImEncryptionEngineCbsDownloadingFileCb)(LinphoneImEncryptionEngine *engine, LinphoneChatMessage *message, size_t offset, const uint8_t *buffer, size_t size, uint8_t *decrypted_buffer);

/**
 * Callback to encrypt uploading file
 * @param engine #LinphoneImEncryptionEngine object @notnil
 * @param message #LinphoneChatMessage object @notnil
 * @param offset The current offset of the upload
 * @param buffer Encrypted data buffer @maybenil
 * @param[in,out] size Size of the plain data buffer and the size of the encrypted data buffer once encryption is done
 * @param[out] encrypted_buffer Buffer in which to write the encrypted data which maxmimum size is size @notnil
 * @return -1 if nothing to be done, 0 on success or an integer > 0 for error
*/
typedef int (*LinphoneImEncryptionEngineCbsUploadingFileCb)(LinphoneImEncryptionEngine *engine, LinphoneChatMessage *message, size_t offset, const uint8_t *buffer, size_t *size, uint8_t *encrypted_buffer);

/**
 * Callback used to notify the response to an XML-RPC request.
 * @param request #LinphoneXmlRpcRequest object @notnil
**/
typedef void (*LinphoneXmlRpcRequestCbsResponseCb)(LinphoneXmlRpcRequest *request);

/**
 * Callback used to notify a chat room has been exhumed. Internal use only!
 * @param core #LinphoneCore object @notnil
 * @param chat_room #LinphoneChatRoom object @notnil
 * @donotwrap
**/
typedef void (*LinphoneCoreCbsChatRoomExhumedCb)(LinphoneCore *core, LinphoneChatRoom *chat_room);

/**
 * @}
**/

/**
 * @addtogroup call_control
 * @{
 */

/**
 * Callback for notifying end of play (file).
 * @param player The #LinphonePlayer object @notnil
**/
typedef void (*LinphonePlayerCbsEofReachedCb)(LinphonePlayer *player);

/************ */
/* DEPRECATED */
/* ********** */

/**
 * Callback prototype
 * @param core #LinphoneCore object @notnil
 * @param chat_room #LinphoneChatRoom involved in this conversation. Can be be created by the framework in case the From-URI is not present in any chat room. @notnil
 * @param from #LinphoneAddress from @notnil
 * @param message incoming message @notnil
 * @deprecated 30/03/2017 use #LinphoneCoreMessageReceivedCb instead.
 * @donotwrap
 */
typedef void (*LinphoneCoreTextMessageReceivedCb)(LinphoneCore *core, LinphoneChatRoom *chat_room, const LinphoneAddress *from, const char *message);

/**
 * Callback for requesting authentication information to application or user.
 * @param core the #LinphoneCore @notnil
 * @param realm the realm (domain) on which authentication is required. @notnil
 * @param username the username that needs to be authenticated. @notnil
 * @param domain the domain on which authentication is required. @notnil
 * Application shall reply to this callback using linphone_core_add_auth_info().
 * @deprecated 08/07/2020 use #LinphoneCoreCbsAuthenticationRequestedCb instead
 */
typedef void (*LinphoneCoreAuthInfoRequestedCb)(LinphoneCore *core, const char *realm, const char *username, const char *domain);


/**
 * @}
**/

#endif /* LINPHONE_CALLBACKS_H_ */
