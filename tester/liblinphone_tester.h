/*
 * Copyright (c) 2010-2025 Belledonne Communications SARL.
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

#ifndef LIBLINPHONE_TESTER_H_
#define LIBLINPHONE_TESTER_H_

#include <stdbool.h>

#include "linphone/core.h"
#include <bctoolbox/tester.h>
#include <mediastreamer2/msutils.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef _MSC_VER
#define popen _popen
#define pclose _pclose
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __ANDROID__
extern jobject system_context;
#else
extern void *system_context;
#endif

extern test_suite_t account_creator_local_test_suite;
extern test_suite_t account_creator_flexiapi_test_suite;
// extern test_suite_t account_creator_xmlrpc_test_suite;
extern test_suite_t account_manager_services_test_suite;
extern test_suite_t call_test_suite;
extern test_suite_t call2_test_suite;
extern test_suite_t call_not_established_test_suite;
extern test_suite_t push_incoming_call_test_suite;

#if VIDEO_ENABLED
extern test_suite_t call_video_test_suite;
extern test_suite_t call_video_msogl_test_suite;
extern test_suite_t call_video_quality_test_suite;
extern test_suite_t alerts_test_suite;
extern test_suite_t call_flexfec_suite;
extern test_suite_t call_video_advanced_scenarios_test_suite;
#endif // if VIDEO_ENABLED

#ifdef HAVE_EKT_SERVER_PLUGIN
extern test_suite_t local_conference_test_suite_end_to_end_encryption_scheduled_conference;
extern test_suite_t local_conference_test_suite_end_to_end_encryption_scheduled_conference_audio_only_participant;
extern test_suite_t local_conference_test_suite_end_to_end_encryption_scheduled_conference_with_chat;
extern test_suite_t local_conference_test_suite_end_to_end_encryption_impromptu_conference;
#endif // HAVE_EKT_SERVER_PLUGIN
extern test_suite_t clonable_object_test_suite;
extern test_suite_t conference_event_test_suite;
extern test_suite_t conference_test_suite;
extern test_suite_t conference_info_tester;
extern test_suite_t contents_test_suite;
extern test_suite_t cpim_test_suite;
extern test_suite_t ics_test_suite;
extern test_suite_t event_test_suite;
extern test_suite_t main_db_test_suite;
extern test_suite_t flexisip_test_suite;
extern test_suite_t lime_db_test_suite;
extern test_suite_t group_chat_test_suite;
extern test_suite_t group_chat2_test_suite;
extern test_suite_t group_chat3_test_suite;
extern test_suite_t group_chat4_test_suite;
extern test_suite_t secure_group_chat_test_suite;
extern test_suite_t secure_group_chat2_test_suite;
extern test_suite_t secure_group_chat_exhume_test_suite;
extern test_suite_t secure_message_test_suite;
extern test_suite_t secure_message2_test_suite;
extern test_suite_t secure_group_chat_migration_test_suite;
extern test_suite_t secure_group_chat_multialgos_test_suite;
extern test_suite_t ephemeral_group_chat_test_suite;
extern test_suite_t ephemeral_group_chat_basic_test_suite;
extern test_suite_t log_collection_test_suite;
extern test_suite_t message_test_suite;
extern test_suite_t rtt_message_test_suite;
#ifdef HAVE_BAUDOT
extern test_suite_t baudot_message_test_suite;
#endif /* HAVE_BAUDOT */
extern test_suite_t session_timers_test_suite;
extern test_suite_t multi_call_test_suite;
extern test_suite_t multicast_call_test_suite;
extern test_suite_t audio_video_conference_basic_test_suite;
extern test_suite_t audio_video_conference_basic2_test_suite;
extern test_suite_t audio_conference_test_suite;
extern test_suite_t audio_conference_local_participant_test_suite;
extern test_suite_t audio_conference_remote_participant_test_suite;
extern test_suite_t video_conference_test_suite;
extern test_suite_t video_conference_layout_test_suite;
extern test_suite_t ice_conference_test_suite;
extern test_suite_t multipart_test_suite;
extern test_suite_t flexiapiclient_suite;
extern test_suite_t offeranswer_test_suite;
extern test_suite_t player_test_suite;
extern test_suite_t presence_server_test_suite;
extern test_suite_t presence_test_suite;
extern test_suite_t property_container_test_suite;
extern test_suite_t proxy_config_test_suite;
extern test_suite_t account_test_suite;
extern test_suite_t quality_reporting_test_suite;
extern test_suite_t recorder_test_suite;
extern test_suite_t register_test_suite;
extern test_suite_t remote_provisioning_test_suite;
extern test_suite_t setup_test_suite;
extern test_suite_t log_file_test_suite;
extern test_suite_t stun_test_suite;
extern test_suite_t tunnel_test_suite;
extern test_suite_t upnp_test_suite;
extern test_suite_t utils_test_suite;
extern test_suite_t video_test_suite;
extern test_suite_t capability_negotiation_test_suite;
extern test_suite_t capability_negotiation_parameters_test_suite;
extern test_suite_t capability_negotiation_no_sdp_test_suite;
extern test_suite_t srtp_capability_negotiation_test_suite;
extern test_suite_t srtp_capability_negotiation_basic_test_suite;
extern test_suite_t zrtp_capability_negotiation_test_suite;
extern test_suite_t zrtp_capability_negotiation_basic_test_suite;
extern test_suite_t dtls_srtp_capability_negotiation_test_suite;
extern test_suite_t dtls_srtp_capability_negotiation_basic_test_suite;
extern test_suite_t ice_capability_negotiation_test_suite;
extern test_suite_t srtp_ice_capability_negotiation_test_suite;
extern test_suite_t zrtp_ice_capability_negotiation_test_suite;
extern test_suite_t dtls_srtp_ice_capability_negotiation_test_suite;
extern test_suite_t call_recovery_test_suite;
extern test_suite_t call_with_ice_test_suite;
extern test_suite_t call_secure_test_suite;
extern test_suite_t call_secure2_test_suite;
extern test_suite_t call_with_rtp_bundle_test_suite;
extern test_suite_t shared_core_test_suite;
extern test_suite_t lime_server_auth_test_suite;
extern test_suite_t vfs_encryption_test_suite;
extern test_suite_t local_conference_test_suite_chat_basic;
extern test_suite_t local_conference_test_suite_chat_advanced;
extern test_suite_t local_conference_test_suite_chat_error;
extern test_suite_t local_conference_test_suite_chat_imdn;
extern test_suite_t local_conference_test_suite_ephemeral_chat;
extern test_suite_t local_conference_test_suite_secure_chat;
extern test_suite_t local_conference_test_suite_conference_edition;
extern test_suite_t local_conference_test_suite_scheduled_conference_basic;
extern test_suite_t local_conference_test_suite_scheduled_conference_advanced;
extern test_suite_t local_conference_test_suite_scheduled_conference_audio_only_participant;
extern test_suite_t local_conference_test_suite_scheduled_conference_with_screen_sharing;
extern test_suite_t local_conference_test_suite_scheduled_conference_with_chat;
extern test_suite_t local_conference_test_suite_scheduled_ice_conference;
extern test_suite_t local_conference_test_suite_impromptu_conference;
extern test_suite_t local_conference_test_suite_encrypted_impromptu_conference;
extern test_suite_t local_conference_test_suite_impromptu_mismatch_conference;
extern test_suite_t local_conference_test_suite_transferred_conference_basic;
extern test_suite_t external_domain_test_suite;
extern test_suite_t potential_configuration_graph_test_suite;
extern test_suite_t call_race_conditions_suite;
extern test_suite_t mwi_test_suite;
extern test_suite_t bearer_auth_test_suite;
extern test_suite_t call_twisted_cases_suite;
extern test_suite_t http_client_test_suite;
extern test_suite_t turn_server_test_suite;
extern test_suite_t refer_test_suite;

#ifdef VCARD_ENABLED
extern test_suite_t vcard_test_suite;
#endif

#ifdef CXX_WRAPPER_ENABLED
extern test_suite_t wrapper_cpp_test_suite;
#endif

extern test_suite_t audio_bypass_suite;
extern test_suite_t audio_routes_test_suite;
extern test_suite_t audio_quality_test_suite;
#if HAVE_SIPP
extern test_suite_t complex_sip_call_test_suite;
#endif

extern test_t dtmf_tests[10];

extern int manager_count;

extern const char *liblinphone_tester_ipv6_probing_address;
extern int liblinphone_tester_ipv6_available(void);
extern int liblinphone_tester_ipv4_available(void);
extern const int liblinphone_tester_sip_timeout;
extern const int x3dhServer_creationTimeout;

/**
 * @brief Tells the tester whether or not to clean the accounts it has created between runs.
 * @details Setting this to 1 will not clear the list of created accounts between successive
 * calls to liblinphone_run_tests(). Some testing APIs call this function for *each* test,
 * in which case we should keep the accounts that were created for further testing.
 *
 * You are supposed to manually call liblinphone_tester_clear_account when all the tests are
 * finished.
 *
 * @param keep 1 to keep the accounts in-between runs, 0 to clear them after each run.
 */
extern void liblinphone_tester_keep_accounts(int keep);

/**
 * @brief Tells the test whether to not remove recorded audio/video files after the tests.
 * @details By default recorded files are erased after the test, unless the test is failed.
 **/
void liblinphone_tester_keep_recorded_files(int keep);

/**
 * @brief Disable the automatic object leak detection. This is useful because the object leak detector prevents valgrind
 *from seeing the leaks.
 * @details By default object leak detector is enabled.
 **/
void liblinphone_tester_disable_leak_detector(int disabled);

/**
 * @brief Clears the created accounts during the testing session.
 */
extern void liblinphone_tester_clear_accounts(void);

extern const char *flexisip_tester_dns_server;
extern bctbx_list_t *flexisip_tester_dns_ip_addresses;
extern const char *ccmp_server_url;
extern const char *test_domain;
extern const char *auth_domain;
extern const char *test_username;
extern const char *test_sha_username;
extern const char *pure_sha256_user;
extern const char *test_password;
extern const char *test_route;
extern const char *userhostsfile;
extern const char *file_transfer_url;
extern const char *file_transfer_url_tls_client_auth;
extern const char *file_transfer_url_digest_auth;
extern const char *file_transfer_url_digest_auth_external_domain;
extern const char *file_transfer_url_digest_auth_any_domain;
extern const char *file_transfer_url_small_files;
extern const char *file_transfer_get_proxy;
extern const char *file_transfer_get_proxy_external_domain;

extern const char *lime_server_url;
extern const char *lime_server_any_domain_url;
extern const char *lime_server_tlsauth_opt_url;
extern const char *lime_server_tlsauth_req_url;
extern const char *lime_server_dual_auth_url;
extern const char *lime_server_external_url;
extern const char *lime_server_external_dual_auth_url;

extern bool_t liblinphone_tester_keep_uuid;
extern bool_t liblinphone_tester_tls_support_disabled;
extern const MSAudioDiffParams audio_cmp_params;
extern const char *liblinphone_tester_mire_id;
extern const char *liblinphone_tester_static_image_id;
extern bool_t liblinphonetester_ipv6;
extern bool_t liblinphonetester_show_account_manager_logs;
extern bool_t liblinphonetester_no_account_creator;
extern unsigned int liblinphone_tester_max_cpu_count;

typedef struct _stats {
	int number_of_LinphoneRegistrationNone;
	int number_of_LinphoneRegistrationProgress;
	int number_of_LinphoneRegistrationRefreshing;
	int number_of_LinphoneRegistrationOk;
	int number_of_LinphoneRegistrationCleared;
	int number_of_LinphoneRegistrationFailed;
	int number_of_auth_info_requested; /* obsolete callback */
	int number_of_authentication_info_requested;
	int number_of_LinphoneChatRoomExhumed;
	int number_of_LinphoneAccountAdded;
	int number_of_LinphoneDefaultAccountChanged;
	int number_of_LinphoneAccountRemoved;

	int number_of_LinphoneCallCreated;
	int number_of_LinphoneCallIncomingReceived;
	int number_of_LinphoneCallPushIncomingReceived;
	int number_of_LinphoneCallOutgoingInit;
	int number_of_LinphoneCallOutgoingProgress;
	int number_of_LinphoneCallOutgoingRinging;
	int number_of_LinphoneCallOutgoingEarlyMedia;
	int number_of_LinphoneCallConnected;
	int number_of_LinphoneCallStreamsRunning;
	int number_of_LinphoneCallPausing;
	int number_of_LinphoneCallPaused;
	int number_of_LinphoneCallResuming;
	int number_of_LinphoneCallRefered;
	int number_of_LinphoneCallError;
	int number_of_LinphoneCallEnd;
	int number_of_LinphoneCallPausedByRemote;
	int number_of_LinphoneCallUpdatedByRemote;
	int number_of_LinphoneCallIncomingEarlyMedia;
	int number_of_LinphoneCallUpdating;
	int number_of_LinphoneCallReleased;
	int number_of_LinphoneCallEarlyUpdatedByRemote;
	int number_of_LinphoneCallEarlyUpdating;

	int number_of_LinphoneChatRoomSessionCreated;
	int number_of_LinphoneChatRoomSessionIncomingReceived;
	int number_of_LinphoneChatRoomSessionPushIncomingReceived;
	int number_of_LinphoneChatRoomSessionOutgoingInit;
	int number_of_LinphoneChatRoomSessionOutgoingProgress;
	int number_of_LinphoneChatRoomSessionOutgoingRinging;
	int number_of_LinphoneChatRoomSessionOutgoingEarlyMedia;
	int number_of_LinphoneChatRoomSessionConnected;
	int number_of_LinphoneChatRoomSessionStreamsRunning;
	int number_of_LinphoneChatRoomSessionPausing;
	int number_of_LinphoneChatRoomSessionPaused;
	int number_of_LinphoneChatRoomSessionResuming;
	int number_of_LinphoneChatRoomSessionRefered;
	int number_of_LinphoneChatRoomSessionError;
	int number_of_LinphoneChatRoomSessionEnd;
	int number_of_LinphoneChatRoomSessionPausedByRemote;
	int number_of_LinphoneChatRoomSessionUpdatedByRemote;
	int number_of_LinphoneChatRoomSessionIncomingEarlyMedia;
	int number_of_LinphoneChatRoomSessionUpdating;
	int number_of_LinphoneChatRoomSessionReleased;
	int number_of_LinphoneChatRoomSessionEarlyUpdatedByRemote;
	int number_of_LinphoneChatRoomSessionEarlyUpdating;

	int number_of_LinphoneChatRoomStateInstantiated;
	int number_of_LinphoneChatRoomStateCreationPending;
	int number_of_LinphoneChatRoomStateCreated;
	int number_of_LinphoneChatRoomStateCreationFailed;
	int number_of_LinphoneChatRoomStateTerminationPending;
	int number_of_LinphoneChatRoomStateTerminated;
	int number_of_LinphoneChatRoomStateTerminationFailed;
	int number_of_LinphoneChatRoomStateDeleted;

	int number_of_LinphoneConferenceStateInstantiated;
	int number_of_LinphoneConferenceStateCreationPending;
	int number_of_LinphoneConferenceStateCreated;
	int number_of_LinphoneConferenceStateCreationFailed;
	int number_of_LinphoneConferenceStateTerminationPending;
	int number_of_LinphoneConferenceStateTerminated;
	int number_of_LinphoneConferenceStateTerminationFailed;
	int number_of_LinphoneConferenceStateDeleted;

	int number_of_LinphoneTransferCallOutgoingInit;
	int number_of_LinphoneTransferCallOutgoingProgress;
	int number_of_LinphoneTransferCallOutgoingRinging;
	int number_of_LinphoneTransferCallOutgoingEarlyMedia;
	int number_of_LinphoneTransferCallConnected;
	int number_of_LinphoneTransferCallStreamsRunning;
	int number_of_LinphoneTransferCallError;

	int number_of_LinphoneMessageReceived;
	int number_of_LinphoneMessageReceivedWithFile;
	int number_of_LinphoneMessageReceivedFailedToDecrypt;
	int number_of_LinphoneAggregatedMessagesReceived;
	int number_of_LinphoneMessageExtBodyReceived;
	int number_of_LinphoneMessageInProgress;
	int number_of_LinphoneMessageDelivered;
	int number_of_LinphoneMessagePendingDelivery;
	int number_of_LinphoneMessageNotDelivered;
	int number_of_LinphoneMessageUndecryptable;
	int number_of_LinphoneMessageFileTransferDone;
	int number_of_LinphoneMessageFileTransferError;
	int number_of_LinphoneMessageFileTransferCancelling;
	int number_of_LinphoneMessageFileTransferInProgress;
	int number_of_LinphoneMessageDeliveredToUser;
	int number_of_LinphoneMessageDisplayed;
	int number_of_LinphoneRemainingNumberOfFileTransferChanged;
	int number_of_LinphoneMessageFileTransferTerminated;
	int number_of_LinphoneMessageSent;
	int number_of_LinphoneMessageEphemeralTimerStarted;
	int number_of_LinphoneMessageEphemeralDeleted;
	int number_of_LinphoneIsComposingActiveReceived;
	int number_of_LinphoneIsComposingIdleReceived;
	int progress_of_LinphoneFileTransfer;
	int number_of_LinphoneFileTransfer;
	int number_of_LinphoneReactionSentOrReceived;
	int number_of_LinphoneReactionRemoved;

	int number_of_LinphoneChatRoomConferenceJoined;
	int number_of_LinphoneChatRoomEphemeralLifetimeChanged;
	int number_of_LinphoneChatRoomEphemeralMessageEnabled;
	int number_of_LinphoneChatRoomEphemeralMessageDisabled;
	int number_of_LinphoneChatRoomEphemeralTimerStarted;
	int number_of_LinphoneChatRoomEphemeralDeleted;
	int number_of_X3dhUserCreationSuccess;
	int number_of_X3dhUserCreationFailure;

	int number_of_IframeDecoded;

	int number_of_NewSubscriptionRequest;
	int number_of_NotifySent;
	int number_of_NotifyReceived;
	int number_of_NotifyFullStateReceived;
	int number_of_NotifyPresenceReceived;
	int number_of_NotifyPresenceReceivedForUriOrTel;
	int number_of_NotifyFriendPresenceReceived;
	int number_of_NotifyEktSent;
	int number_of_NotifyEktReceived;
	int number_of_LinphonePresenceActivityOffline;
	int number_of_LinphonePresenceActivityOnline;
	int number_of_LinphonePresenceActivityAppointment;
	int number_of_LinphonePresenceActivityAway;
	int number_of_LinphonePresenceActivityBreakfast;
	int number_of_LinphonePresenceActivityBusy;
	int number_of_LinphonePresenceActivityDinner;
	int number_of_LinphonePresenceActivityHoliday;
	int number_of_LinphonePresenceActivityInTransit;
	int number_of_LinphonePresenceActivityLookingForWork;
	int number_of_LinphonePresenceActivityLunch;
	int number_of_LinphonePresenceActivityMeal;
	int number_of_LinphonePresenceActivityMeeting;
	int number_of_LinphonePresenceActivityOnThePhone;
	int number_of_LinphonePresenceActivityOther;
	int number_of_LinphonePresenceActivityPerformance;
	int number_of_LinphonePresenceActivityPermanentAbsence;
	int number_of_LinphonePresenceActivityPlaying;
	int number_of_LinphonePresenceActivityPresentation;
	int number_of_LinphonePresenceActivityShopping;
	int number_of_LinphonePresenceActivitySleeping;
	int number_of_LinphonePresenceActivitySpectator;
	int number_of_LinphonePresenceActivitySteering;
	int number_of_LinphonePresenceActivityTravel;
	int number_of_LinphonePresenceActivityTV;
	int number_of_LinphonePresenceActivityUnknown;
	int number_of_LinphonePresenceActivityVacation;
	int number_of_LinphonePresenceActivityWorking;
	int number_of_LinphonePresenceActivityWorship;
	const LinphonePresenceModel *last_received_presence;

	int number_of_LinphonePresenceBasicStatusOpen;
	int number_of_LinphonePresenceBasicStatusClosed;

	int number_of_LinphoneConsolidatedPresenceOnline;
	int number_of_LinphoneConsolidatedPresenceBusy;
	int number_of_LinphoneConsolidatedPresenceDoNotDisturb;
	int number_of_LinphoneConsolidatedPresenceOffline;

	int number_of_InfoReceived;
	LinphoneInfoMessage *last_received_info_message;

	int number_of_LinphoneSubscriptionIncomingReceived;
	int number_of_LinphoneSubscriptionOutgoingProgress;
	int number_of_LinphoneSubscriptionPending;
	int number_of_LinphoneSubscriptionActive;
	int number_of_LinphoneSubscriptionTerminated;
	int number_of_LinphoneSubscriptionError;
	int number_of_LinphoneSubscriptionExpiring;

	int number_of_LinphonePublishOutgoingProgress;
	int number_of_LinphonePublishIncomingReceived;
	int number_of_LinphonePublishRefreshing;
	int number_of_LinphonePublishOk;
	int number_of_LinphonePublishExpiring;
	int number_of_LinphonePublishError;
	int number_of_LinphonePublishCleared;
	int number_of_LinphonePublishTerminating;

	int number_of_LinphoneConfiguringSkipped;
	int number_of_LinphoneConfiguringFailed;
	int number_of_LinphoneConfiguringSuccessful;

	int number_of_LinphoneCallReferRequested;
	int number_of_LinphoneCallGoClearAckSent;
	int number_of_LinphoneCallSecurityLevelDowngraded;
	int number_of_LinphoneCallEncryptedOn;
	int number_of_LinphoneCallEncryptedOff;
	int number_of_LinphoneCallAuthenticationTokenVerified;
	int number_of_LinphoneCallIncorrectAuthenticationTokenSelected;
	int number_of_LinphoneCallSendMasterKeyChanged;
	int number_of_LinphoneCallReceiveMasterKeyChanged;
	int number_of_NetworkReachableTrue;
	int number_of_NetworkReachableFalse;
	int number_of_player_eof;
	LinphoneChatMessage *last_received_chat_message;
	LinphoneChatMessage *last_fail_to_decrypt_received_chat_message;

	char *dtmf_list_received;
	int dtmf_count;

	int number_of_LinphoneCallStatsUpdated;
	int number_of_rtcp_sent;
	int number_of_rtcp_received;         /*total number of rtcp packet received */
	int number_of_rtcp_received_via_mux; /*number of rtcp packet received in rtcp-mux mode*/

	int number_of_video_windows_created;

	int number_of_LinphoneFileTransferDownloadSuccessful;
	int number_of_LinphoneCoreLogCollectionUploadStateDelivered;
	int number_of_LinphoneCoreLogCollectionUploadStateNotDelivered;
	int number_of_LinphoneCoreLogCollectionUploadStateInProgress;
	int audio_download_bandwidth[3];
	int audio_upload_bandwidth[3];

	int video_download_bandwidth[3];
	int video_upload_bandwidth[3];
	int current_bandwidth_index[2] /*audio and video only*/;
	int number_of_LinphoneCallCameraNotWorking;

	int number_of_rtcp_generic_nack;
	int number_of_tmmbr_received;
	int last_tmmbr_value_received;
	int tmmbr_received_from_cb;

	int number_of_chat_room_participants_added;
	int number_of_chat_room_participants_removed;
	int number_of_chat_room_participant_admin_statuses_changed;

	int number_of_chat_room_participant_devices_added;
	int number_of_chat_room_participant_devices_removed;

	int number_of_chat_room_subject_changed;

	int number_of_active_speaker_participant_device_changed;
	int number_of_allowed_participant_list_changed;
	int number_of_participants_added;
	int number_of_participant_role_changed;
	int number_of_participant_admin_statuses_changed;
	int number_of_participants_removed;
	int number_of_subject_changed;
	int number_of_available_media_changed;
	int number_of_core_chat_room_subject_changed;

	int number_of_participant_devices_added;
	int number_of_participant_devices_removed;
	int number_of_participant_devices_screen_sharing_enabled;
	int number_of_participant_devices_screen_sharing_disabled;
	int number_of_participant_devices_joining_request;
	int number_of_conference_full_state_received;
	int number_of_participant_devices_media_capability_changed;
	int number_of_conference_participant_devices_scheduled_for_joining;
	int number_of_conference_participant_devices_pending;
	int number_of_conference_participant_devices_on_hold;
	int number_of_conference_participant_devices_requesting_to_join;
	int number_of_conference_participant_devices_alerting;
	int number_of_conference_participant_devices_present;
	int number_of_conference_participant_devices_scheduled_for_leaving;
	int number_of_conference_participant_devices_leaving;
	int number_of_conference_participant_devices_left;
	int number_of_conference_participant_devices_muted_by_focus;
	int number_of_participant_state_changed;

	int number_of_participant_devices_scheduled_for_joining;
	int number_of_participant_devices_pending;
	int number_of_participant_devices_on_hold;
	int number_of_participant_devices_requesting_to_join;
	int number_of_participant_devices_alerting;
	int number_of_participant_devices_present;
	int number_of_participant_devices_scheduled_for_leaving;
	int number_of_participant_devices_leaving;
	int number_of_participant_devices_left;
	int number_of_participant_devices_muted_by_focus;

	int number_of_SecurityLevelDowngraded;
	int number_of_ParticipantMaxDeviceCountExceeded;
	int number_of_EncryptionIdentityKeyChanged;
	int number_of_ManInTheMiddleDetected;

	int number_of_snapshot_taken;

	int number_of_LinphoneGlobalOn;
	int number_of_LinphoneGlobalReady;
	int number_of_LinphoneGlobalOff;
	int number_of_LinphoneGlobalShutdown;
	int number_of_LinphoneGlobalStartup;
	int number_of_LinphoneGlobalConfiguring;

	int number_of_LinphoneCoreFirstCallStarted;
	int number_of_LinphoneCoreLastCallEnded;
	int number_of_LinphoneCoreAudioDeviceChanged;
	int number_of_LinphoneCoreAudioDevicesListUpdated;
	int number_of_LinphoneCoreVersionUpdateCheck;

	int number_of_LinphoneRemoteRecordingEnabled;
	int number_of_LinphoneRemoteRecordingDisabled;

	int number_of_LinphoneParticipantDeviceStartSpeaking;
	int number_of_LinphoneParticipantDeviceStopSpeaking;

	int number_of_LinphoneParticipantDeviceMuted;
	int number_of_LinphoneParticipantDeviceUnmuted;

	int number_of_ConferenceSchedulerStateIdle;
	int number_of_ConferenceSchedulerStateAllocationPending;
	int number_of_ConferenceSchedulerStateReady;
	int number_of_ConferenceSchedulerStateUpdating;
	int number_of_ConferenceSchedulerStateError;
	int number_of_ConferenceSchedulerInvitationsSent;

	int number_of_ConferenceInformationUpdated;

	int number_of_LinphoneMagicSearchResultReceived;
	int number_of_LinphoneMagicSearchLdapHaveMoreResults;

	int number_of_mwi;
	int number_of_new_LinphoneMessageWaitingIndicationVoice;
	int number_of_old_LinphoneMessageWaitingIndicationVoice;
	int number_of_new_urgent_LinphoneMessageWaitingIndicationVoice;
	int number_of_old_urgent_LinphoneMessageWaitingIndicationVoice;

#ifdef HAVE_BAUDOT
	int number_of_LinphoneBaudotDetected;
	int number_of_LinphoneBaudotEuropeDetected;
	int number_of_LinphoneBaudotUsDetected;
#endif /* HAVE_BAUDOT */

	int number_of_out_of_dialog_refer_received;
} stats;

typedef enum _LinphoneCoreManagerSubscribePolicy {
	AcceptSubscription,
	DenySubscription,
	RetainSubscription,
	DoNothingWithSubscription
} LinphoneCoreManagerSubscribePolicy;

typedef enum _LinphoneCoreManagerPublishPolicy {
	AcceptPublish,
	DenyPublish,
	DoNothingWithPublish
} LinphoneCoreManagerPublishPolicy;

typedef struct _LinphoneCoreManager {
	LinphoneCoreCbs *cbs;
	LinphoneCore *lc;
	stats stat;
	LinphoneAddress *identity;
	LinphoneEvent *lev;
	int number_of_bcunit_error_at_creation;
	char *phone_alias;
	char *rc_path;
	char *rc_local;
	char *database_path;
	char *lime_database_path;
	char *zrtp_secrets_database_path;
	char *app_group_id;
	void *user_info;
	bool_t main_core;
	bool_t skip_lime_user_creation_asserts;
	bool_t lime_failure;
	bool_t registration_failure;
	LinphoneCoreManagerSubscribePolicy subscribe_policy;
	LinphoneCoreManagerPublishPolicy publish_policy;
	int subscription_received;
} LinphoneCoreManager;

typedef struct _LinphoneConferenceServer {
	LinphoneCoreManager base;
	LinphoneCall *first_call;
	LinphoneCoreCbs *cbs;
	LinphoneRegistrationState reg_state;
} LinphoneConferenceServer;

typedef struct _LinphoneCallTestParams {
	LinphoneCallParams *base;
	bool_t sdp_removal;
	bool_t sdp_simulate_error;
} LinphoneCallTestParams;

void liblinphone_tester_add_suites(void);

void linphone_core_manager_init(LinphoneCoreManager *mgr, const char *rc_file, const char *phone_alias);
void linphone_core_manager_init2(LinphoneCoreManager *mgr, const char *rc_file, const char *phone_alias);
void linphone_core_manager_init_shared(LinphoneCoreManager *mgr,
                                       const char *rc_file,
                                       const char *phone_alias,
                                       LinphoneCoreManager *mgr_to_copy);
LinphoneCore *linphone_core_manager_configure_lc(LinphoneCoreManager *mgr);
void linphone_core_manager_configure(LinphoneCoreManager *mgr);
void linphone_core_manager_start(LinphoneCoreManager *mgr, bool_t check_for_proxies);
LinphoneCoreManager *linphone_core_manager_create2(const char *rc_file, const char *phone_alias);
LinphoneCoreManager *linphone_core_manager_create(const char *rc_file);
LinphoneCoreManager *linphone_core_manager_new4(
    const char *rc_file, int check_for_proxies, const char *phone_aliasconst, const char *contact_params, int expires);
LinphoneCoreManager *linphone_core_manager_new3(const char *rc_file, bool_t check_for_proxies, const char *phone_alias);
LinphoneCoreManager *linphone_core_manager_new_with_proxies_check(const char *rc_file, bool_t check_for_proxies);
LinphoneCoreManager *linphone_core_manager_new(const char *rc_file);
LinphoneCoreManager *linphone_core_manager_create_local(const char *rc_factory,
                                                        const char *rc_local,
                                                        const char *linphone_db,
                                                        const char *lime_db,
                                                        const char *zrtp_secrets_db);
LinphoneCoreManager *linphone_core_manager_new_local(const char *rc_factory,
                                                     const char *rc_local,
                                                     const char *linphone_db,
                                                     const char *lime_db,
                                                     const char *zrtp_secrets_db);
LinphoneCoreManager *linphone_core_manager_create_shared(const char *rc_file,
                                                         const char *app_group_id,
                                                         bool_t main_core,
                                                         LinphoneCoreManager *mgr_to_copy);
void linphone_core_manager_skip_lime_user_creation_asserts(LinphoneCoreManager *mgr, bool_t value);
void linphone_core_manager_expect_lime_failure(LinphoneCoreManager *mgr, bool_t value);
void linphone_core_manager_stop(LinphoneCoreManager *mgr);
void linphone_core_manager_uninit_after_stop_async(LinphoneCoreManager *mgr);
void linphone_core_manager_reinit(LinphoneCoreManager *mgr);
void linphone_core_manager_restart(LinphoneCoreManager *mgr, bool_t check_for_proxies);
/* This function is used to restore the fake DNS which is lost after a linphone_core_stop() */
void linphone_core_manager_setup_dns(LinphoneCoreManager *mgr);
void linphone_core_manager_uninit(LinphoneCoreManager *mgr);
void linphone_core_manager_uninit2(LinphoneCoreManager *mgr, bool_t unlinkDb, bool_t unlinkRc);
void linphone_core_manager_wait_for_stun_resolution(LinphoneCoreManager *mgr);
void linphone_core_manager_destroy(LinphoneCoreManager *mgr);
void linphone_core_manager_destroy_after_stop_async(LinphoneCoreManager *mgr);
void linphone_core_manager_delete_chat_room(LinphoneCoreManager *mgr, LinphoneChatRoom *cr, bctbx_list_t *coresList);
bctbx_list_t *init_core_for_conference(bctbx_list_t *coreManagerList);
bctbx_list_t *init_core_for_conference_with_factori_uri(bctbx_list_t *coreManagerList, const char *factoryUri);
void start_core_for_conference(bctbx_list_t *coreManagerList);
bctbx_list_t *init_core_for_conference_with_factory_uri(bctbx_list_t *coreManagerList, const char *factoryUri);
bctbx_list_t *init_core_for_conference_with_groupchat_version(bctbx_list_t *coreManagerList,
                                                              const char *groupchat_version);

// void linphone_conference_server_refer_received(LinphoneCore *core, const char *refer_to);

void reset_counters(stats *counters);

void registration_state_changed(struct _LinphoneCore *lc,
                                LinphoneProxyConfig *cfg,
                                LinphoneRegistrationState cstate,
                                const char *message);
void call_state_changed(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *msg);
void linphone_transfer_state_changed(LinphoneCore *lc, LinphoneCall *transferred, LinphoneCallState new_call_state);
void notify_presence_received(LinphoneCore *lc, LinphoneFriend *lf);
void notify_friend_presence_received(LinphoneFriend *lf);
void notify_presence_received_for_uri_or_tel(LinphoneCore *lc,
                                             LinphoneFriend *lf,
                                             const char *uri_or_tel,
                                             const LinphonePresenceModel *presence);
void messages_received(LinphoneCore *lc, LinphoneChatRoom *room, const bctbx_list_t *messages);
void message_received(LinphoneCore *lc, LinphoneChatRoom *room, LinphoneChatMessage *message);
void message_received_fail_to_decrypt(LinphoneCore *lc, LinphoneChatRoom *room, LinphoneChatMessage *message);
void reaction_received(LinphoneCore *lc,
                       LinphoneChatRoom *room,
                       LinphoneChatMessage *msg,
                       const LinphoneChatMessageReaction *reaction);
void reaction_removed(LinphoneCore *lc,
                      LinphoneChatRoom *room,
                      LinphoneChatMessage *msg,
                      const LinphoneAddress *address);
char *random_filename(char *prefix, char *extension);
char *random_filepath(char *prefix, char *extension);
void file_transfer_received(LinphoneChatMessage *message, LinphoneContent *content, const LinphoneBuffer *buffer);
LinphoneBuffer *
tester_file_transfer_send(LinphoneChatMessage *message, LinphoneContent *content, size_t offset, size_t size);
void tester_file_transfer_send_2(
    LinphoneChatMessage *message, LinphoneContent *content, size_t offset, size_t size, LinphoneBuffer *buffer);
LinphoneChatMessage *_send_message_ephemeral(LinphoneChatRoom *chatRoom, const char *message, bool_t ephemeral);
LinphoneChatMessage *_send_message(LinphoneChatRoom *chatRoom, const char *message);
void set_ephemeral_cbs(bctbx_list_t *history);
void _send_file_plus_text(
    LinphoneChatRoom *cr, const char *sendFilepath, const char *sendFilepath2, const char *text, bool_t use_buffer);
void _send_file(LinphoneChatRoom *cr, const char *sendFilepath, const char *sendFilepath2, bool_t use_buffer);
void _receive_file(bctbx_list_t *coresList,
                   LinphoneCoreManager *lcm,
                   stats *receiverStats,
                   const char *receive_filepath,
                   const char *sendFilepath,
                   const char *sendFilepath2,
                   bool_t use_buffer);
void _receive_file_plus_text(bctbx_list_t *coresList,
                             LinphoneCoreManager *lcm,
                             stats *receiverStats,
                             const char *receive_filepath,
                             const char *sendFilepath,
                             const char *sendFilepath2,
                             const char *text,
                             bool_t use_buffer);

LinphoneBuffer *
tester_memory_file_transfer_send(LinphoneChatMessage *message, LinphoneContent *content, size_t offset, size_t size);
void file_transfer_progress_indication(LinphoneChatMessage *message,
                                       LinphoneContent *content,
                                       size_t offset,
                                       size_t total);
void file_transfer_progress_indication_2(LinphoneChatMessage *message,
                                         LinphoneContent *content,
                                         size_t offset,
                                         size_t total);
void is_composing_received(LinphoneCore *lc, LinphoneChatRoom *room);
void info_message_received(LinphoneCore *lc, LinphoneCall *call, const LinphoneInfoMessage *msg);
void new_subscription_requested(LinphoneCore *lc, LinphoneFriend *lf, const char *url);
void linphone_subscription_state_change(LinphoneCore *lc, LinphoneEvent *ev, LinphoneSubscriptionState state);
void linphone_publish_state_changed(LinphoneCore *lc, LinphoneEvent *ev, LinphonePublishState state);
void linphone_notify_sent(LinphoneCore *lc, LinphoneEvent *lev, const LinphoneContent *content);
void linphone_notify_received(LinphoneCore *lc,
                              LinphoneEvent *lev,
                              const char *eventname,
                              const LinphoneContent *content);
void linphone_subscribe_received(LinphoneCore *lc,
                                 LinphoneEvent *lev,
                                 const char *eventname,
                                 const LinphoneContent *content);
void linphone_publish_received(LinphoneCore *lc,
                               LinphoneEvent *lev,
                               const char *eventname,
                               const LinphoneContent *content);
void linphone_configuration_status(LinphoneCore *lc, LinphoneConfiguringState status, const char *message);
void linphone_refer_asked(LinphoneCore *lc);
void linphone_call_goclear_ack_sent(LinphoneCore *lc, LinphoneCall *call);
void linphone_call_create_cbs_security_level_downgraded(LinphoneCall *call);
void linphone_call_encryption_changed(LinphoneCore *lc,
                                      LinphoneCall *call,
                                      bool_t on,
                                      const char *authentication_token);
void linphone_call_authentication_token_verified(LinphoneCore *lc, LinphoneCall *call, bool_t verified);
void linphone_call_send_master_key_changed(LinphoneCore *lc, LinphoneCall *call, const char *master_key);
void linphone_call_receive_master_key_changed(LinphoneCore *lc, LinphoneCall *call, const char *master_key);
void dtmf_received(LinphoneCore *lc, LinphoneCall *call, int dtmf);
void call_stats_updated(LinphoneCore *lc, LinphoneCall *call, const LinphoneCallStats *stats);
void global_state_changed(LinphoneCore *lc, LinphoneGlobalState gstate, const char *message);
void first_call_started(LinphoneCore *lc);
void last_call_ended(LinphoneCore *lc);
void audio_device_changed(LinphoneCore *lc, LinphoneAudioDevice *device);
void audio_devices_list_updated(LinphoneCore *lc);

LinphoneAddress *create_linphone_address(const char *domain);
LinphoneAddress *create_linphone_address_for_algo(const char *domain, const char *username);
bool_t wait_for(LinphoneCore *lc_1, LinphoneCore *lc_2, int *counter, int value);
bool_t wait_for_list(MSList *lcs, const int *counter, int value, int timeout_ms);
bool_t wait_for_list_for_uint64(MSList *lcs, const uint64_t *counter, uint64_t value, int timeout_ms);
bool_t wait_for_list_interval(MSList *lcs, int *counter, int min, int max, int timeout_ms);
bool_t wait_for_until(LinphoneCore *lc_1, LinphoneCore *lc_2, const int *counter, int value, int timeout_ms);
bool_t wait_for_until_for_uint64(
    LinphoneCore *lc_1, LinphoneCore *lc_2, const uint64_t *counter, uint64_t value, int timeout_ms);
bool_t wait_for_until_interval(LinphoneCore *lc_1, LinphoneCore *lc_2, int *counter, int min, int max, int timeout_ms);

bool_t call_with_params(LinphoneCoreManager *caller_mgr,
                        LinphoneCoreManager *callee_mgr,
                        const LinphoneCallParams *caller_params,
                        const LinphoneCallParams *callee_params);
bool_t call_with_test_params(LinphoneCoreManager *caller_mgr,
                             LinphoneCoreManager *callee_mgr,
                             const LinphoneCallTestParams *caller_test_params,
                             const LinphoneCallTestParams *callee_test_params);
bool_t call_with_params2(LinphoneCoreManager *caller_mgr,
                         LinphoneCoreManager *callee_mgr,
                         const LinphoneCallTestParams *caller_test_params,
                         const LinphoneCallTestParams *callee_test_params,
                         bool_t build_callee_params);

bool_t call(LinphoneCoreManager *caller_mgr, LinphoneCoreManager *callee_mgr);
bool_t request_video(LinphoneCoreManager *caller, LinphoneCoreManager *callee, bool_t use_accept_call_update);
bool_t remove_video(LinphoneCoreManager *caller, LinphoneCoreManager *callee);
void end_call(LinphoneCoreManager *m1, LinphoneCoreManager *m2);
void disable_all_audio_codecs_except_one(LinphoneCore *lc, const char *mime, int rate);
void disable_all_video_codecs_except_one(LinphoneCore *lc, const char *mime);
void disable_all_codecs(const MSList *elem, LinphoneCoreManager *call);
stats *get_stats(LinphoneCore *lc);
bool_t transport_supported(LinphoneTransportType transport);
LinphoneCoreManager *get_manager(LinphoneCore *lc);
const char *liblinphone_tester_get_subscribe_content(void);
const char *liblinphone_tester_get_notify_content(void);
void liblinphone_tester_chat_message_state_change(LinphoneChatMessage *msg, LinphoneChatMessageState state, void *ud);
bool_t liblinphone_tester_chat_message_msg_update_stats(stats *counters, LinphoneChatMessageState state);
void liblinphone_tester_chat_message_file_transfer_terminated(LinphoneChatMessage *msg, LinphoneContent *content);
void liblinphone_tester_chat_message_msg_state_changed(LinphoneChatMessage *msg, LinphoneChatMessageState state);
void liblinphone_tester_chat_message_reaction_received(LinphoneChatMessage *msg,
                                                       const LinphoneChatMessageReaction *reaction);
void liblinphone_tester_remaining_number_of_file_transfer_changed(LinphoneCore *lc,
                                                                  unsigned int download_count,
                                                                  unsigned int upload_count);
void liblinphone_tester_chat_room_msg_sent(LinphoneCore *lc, LinphoneChatRoom *room, LinphoneChatMessage *msg);
void liblinphone_tester_chat_message_ephemeral_timer_started(LinphoneChatMessage *msg);
void liblinphone_tester_chat_message_ephemeral_deleted(LinphoneChatMessage *msg);
void core_chat_room_state_changed(LinphoneCore *core, LinphoneChatRoom *cr, LinphoneChatRoomState state);
void setup_chat_room_callbacks(LinphoneChatRoomCbs *cbs);
void liblinphone_tester_x3dh_user_created(LinphoneCore *lc, const bool_t status, const char *userId, const char *info);
void core_chat_room_subject_changed(LinphoneCore *core, LinphoneChatRoom *cr);
bctbx_list_t *liblinphone_tester_get_messages_and_states(
    LinphoneChatRoom *cr, int *messageCount, stats *stats); // Return all LinphoneChatMessage and count states

void liblinphone_tester_check_rtcp(LinphoneCoreManager *caller, LinphoneCoreManager *callee);
void liblinphone_tester_check_rtcp_2(LinphoneCoreManager *caller, LinphoneCoreManager *callee);
void liblinphone_tester_clock_start(MSTimeSpec *start);
bool_t liblinphone_tester_clock_elapsed(const MSTimeSpec *start, int value_ms);

void linphone_core_manager_check_accounts(LinphoneCoreManager *m);
void account_manager_destroy(void);
LinphoneCore *configure_lc_from(LinphoneCoreCbs *cbs, const char *path, LinphoneConfig *config, void *user_data);
void configure_core_for_callbacks(LinphoneCoreManager *lcm, LinphoneCoreCbs *cbs);

void liblinphone_tester_set_next_video_frame_decoded_cb(LinphoneCall *call);
void call_paused_resumed_base(bool_t multicast, bool_t with_losses, bool_t accept_video);
void simple_call_base(bool_t enable_multicast_recv_side, bool_t disable_soundcard, bool_t use_multipart_invite_body);
void simple_call_base_with_rcs(const char *caller_rc,
                               const char *callee_rc,
                               bool_t enable_multicast_recv_side,
                               bool_t disable_soundcard,
                               bool_t use_multipart_invite_body,
                               bool_t double_call);
void _call_with_rtcp_mux(bool_t caller_rtcp_mux, bool_t callee_rtcp_mux, bool_t with_ice, bool_t with_ice_reinvite);
void call_base_with_configfile(LinphoneMediaEncryption mode,
                               bool_t enable_video,
                               bool_t enable_relay,
                               LinphoneFirewallPolicy policy,
                               bool_t enable_tunnel,
                               const char *marie_rc,
                               const char *pauline_rc);
void call_base_with_configfile_play_nothing(LinphoneMediaEncryption mode,
                                            bool_t enable_video,
                                            bool_t enable_relay,
                                            LinphoneFirewallPolicy policy,
                                            bool_t enable_tunnel,
                                            const char *marie_rc,
                                            const char *pauline_rc);
void call_base(LinphoneMediaEncryption mode,
               bool_t enable_video,
               bool_t enable_relay,
               LinphoneFirewallPolicy policy,
               bool_t enable_tunnel);
void call_with_several_video_switches_base(const LinphoneMediaEncryption caller_encryption,
                                           const LinphoneMediaEncryption callee_encryption);
bool_t call_with_caller_params(LinphoneCoreManager *caller_mgr,
                               LinphoneCoreManager *callee_mgr,
                               const LinphoneCallParams *params);
bool_t pause_call_1(LinphoneCoreManager *mgr_1, LinphoneCall *call_1, LinphoneCoreManager *mgr_2, LinphoneCall *call_2);
LinphoneAudioDevice *change_device(bool_t enable,
                                   LinphoneCoreManager *mgr,
                                   LinphoneAudioDevice *current_dev,
                                   LinphoneAudioDevice *dev0,
                                   LinphoneAudioDevice *dev1);
LinphoneAudioDevice *pause_call_changing_device(bool_t enable,
                                                bctbx_list_t *lcs,
                                                LinphoneCall *call,
                                                LinphoneCoreManager *mgr_pausing,
                                                LinphoneCoreManager *mgr_paused,
                                                LinphoneCoreManager *mgr_change_device,
                                                LinphoneAudioDevice *current_dev,
                                                LinphoneAudioDevice *dev0,
                                                LinphoneAudioDevice *dev1);
void compare_files(const char *path1, const char *path2);
void check_media_direction(LinphoneCoreManager *mgr,
                           LinphoneCall *call,
                           MSList *lcs,
                           LinphoneMediaDirection audio_dir,
                           LinphoneMediaDirection video_dir);
void _call_with_ice_base(LinphoneCoreManager *pauline,
                         LinphoneCoreManager *marie,
                         bool_t caller_with_ice,
                         bool_t callee_with_ice,
                         bool_t random_ports,
                         bool_t forced_relay,
                         bool_t quick_cancel);
void record_call(const char *filename, bool_t enableVideo, const char *video_codec);
void on_muted_notified(LinphoneParticipantDevice *participant_device, bool_t is_muted);

#define AUDIO_START 0
#define VIDEO_START 1
#define TEXT_START 2
int check_nb_media_starts(unsigned int media_type,
                          LinphoneCoreManager *caller,
                          LinphoneCoreManager *callee,
                          unsigned int caller_nb_media_starts,
                          unsigned int callee_nb_media_starts);

void setup_sdp_handling(const LinphoneCallTestParams *params, LinphoneCoreManager *mgr);
void check_stream_encryption(LinphoneCall *call);
int get_stream_stop_count(LinphoneCall *call);
bool_t search_matching_srtp_suite(LinphoneCoreManager *caller_mgr, LinphoneCoreManager *callee_mgr);

void group_chat_with_imdn_sent_only_to_sender_base(bool_t add_participant,
                                                   bool_t enable_lime,
                                                   bool_t participant_goes_offline);
LinphoneChatRoom *create_chat_room_client_side(bctbx_list_t *lcs,
                                               LinphoneCoreManager *lcm,
                                               stats *initialStats,
                                               bctbx_list_t *participantsAddresses,
                                               const char *initialSubject,
                                               bool_t encrypted,
                                               LinphoneChatRoomEphemeralMode mode);
LinphoneChatRoom *create_chat_room_client_side_with_params(bctbx_list_t *lcs,
                                                           LinphoneCoreManager *lcm,
                                                           stats *initialStats,
                                                           bctbx_list_t *participantsAddresses,
                                                           const char *initialSubject,
                                                           LinphoneChatRoomParams *params);
LinphoneChatRoom *create_chat_room_client_side_with_expected_number_of_participants(bctbx_list_t *lcs,
                                                                                    LinphoneCoreManager *lcm,
                                                                                    stats *initialStats,
                                                                                    bctbx_list_t *participantsAddresses,
                                                                                    const char *initialSubject,
                                                                                    int expectedParticipantSize,
                                                                                    bool_t encrypted,
                                                                                    LinphoneChatRoomEphemeralMode mode);
LinphoneChatRoom *check_creation_chat_room_client_side(bctbx_list_t *lcs,
                                                       LinphoneCoreManager *lcm,
                                                       stats *initialStats,
                                                       const LinphoneAddress *confAddr,
                                                       const char *subject,
                                                       int participantNumber,
                                                       bool_t isAdmin);
void check_create_chat_room_client_side(bctbx_list_t *lcs,
                                        LinphoneCoreManager *lcm,
                                        LinphoneChatRoom *chatRoom,
                                        stats *initialStats,
                                        bctbx_list_t *participantsAddresses,
                                        const char *initialSubject,
                                        int expectedParticipantSize);
void configure_core_for_conference(LinphoneCore *core,
                                   const char *username,
                                   const LinphoneAddress *factoryAddr,
                                   bool_t server);
void _configure_core_for_conference(LinphoneCoreManager *lcm, const LinphoneAddress *factoryAddr);
void _configure_core_for_audio_video_conference(LinphoneCoreManager *lcm, const LinphoneAddress *factoryAddr);
LinphoneParticipantInfo *add_participant_info_to_list(bctbx_list_t **participants_info,
                                                      const LinphoneAddress *address,
                                                      LinphoneParticipantRole role,
                                                      int sequence);
void conference_scheduler_state_changed(LinphoneConferenceScheduler *scheduler, LinphoneConferenceSchedulerState state);
void conference_scheduler_invitations_sent(LinphoneConferenceScheduler *scheduler,
                                           const bctbx_list_t *failed_addresses);
int find_matching_participant_info(const LinphoneParticipantInfo *info1, const LinphoneParticipantInfo *info2);
void check_conference_info_against_db(LinphoneCoreManager *mgr,
                                      LinphoneAddress *confAddr,
                                      const LinphoneConferenceInfo *info1,
                                      bool_t skip_participant_info);

void check_conference_info_in_db(LinphoneCoreManager *mgr,
                                 const char *uid,
                                 LinphoneAddress *confAddr,
                                 LinphoneAddress *organizer,
                                 bctbx_list_t *participantList,
                                 long long start_time,
                                 int duration,
                                 const char *subject,
                                 const char *description,
                                 unsigned int sequence,
                                 LinphoneConferenceInfoState state,
                                 LinphoneConferenceSecurityLevel security_level,
                                 bool_t skip_participant_info,
                                 bool_t audio_enabled,
                                 bool_t video_enabled,
                                 bool_t chat_enabled);

void check_conference_info_members(const LinphoneConferenceInfo *info,
                                   const char *uid,
                                   LinphoneAddress *confAddr,
                                   LinphoneAddress *organizer,
                                   bctbx_list_t *participantList,
                                   long long start_time,
                                   int duration,
                                   const char *subject,
                                   const char *description,
                                   unsigned int sequence,
                                   LinphoneConferenceInfoState state,
                                   LinphoneConferenceSecurityLevel security_level,
                                   bool_t skip_participant_info,
                                   bool_t audio_enabled,
                                   bool_t video_enabled,
                                   bool_t chat_enabled);

void compare_conference_infos(const LinphoneConferenceInfo *info1,
                              const LinphoneConferenceInfo *info2,
                              bool_t skip_participant_info);

void _start_core(LinphoneCoreManager *lcm);
extern const char *sFactoryUri;

void check_reactions(LinphoneChatMessage *message,
                     size_t expected_reactions_count,
                     const bctbx_list_t *expected_reactions,
                     const bctbx_list_t *expected_reactions_from);

/*
 * this function return max value in the last 3 seconds*/
int linphone_core_manager_get_max_audio_down_bw(const LinphoneCoreManager *mgr);
int linphone_core_manager_get_max_audio_up_bw(const LinphoneCoreManager *mgr);
int linphone_core_manager_get_mean_audio_down_bw(const LinphoneCoreManager *mgr);
int linphone_core_manager_get_mean_audio_up_bw(const LinphoneCoreManager *mgr);

void video_call_base_2(LinphoneCoreManager *pauline,
                       LinphoneCoreManager *marie,
                       bool_t using_policy,
                       LinphoneMediaEncryption mode,
                       bool_t callee_video_enabled,
                       bool_t caller_video_enabled);

void liblinphone_tester_before_each(void);
void liblinphone_tester_after_each(void);
void liblinphone_tester_init(void (*ftester_printf)(int level, const char *fmt, va_list args));
void liblinphone_tester_uninit(void);
int liblinphone_tester_set_log_file(const char *filename);

// Add internal callback for subscriptions and notifications
LinphoneCoreManager *
create_mgr_for_conference(const char *rc_file, bool_t check_for_proxies, const char *conference_version);
void setup_mgr_for_conference(LinphoneCoreManager *mgr, const char *conference_version);
void setup_conference_info_cbs(LinphoneCoreManager *mgr);
void destroy_mgr_in_conference(LinphoneCoreManager *mgr);
bool check_conference_ssrc(LinphoneConference *local_conference, LinphoneConference *remote_conference);
void check_conference_medias(LinphoneConference *local_conference, LinphoneConference *remote_conference);
LinphoneStatus add_participant_to_local_conference_through_invite(bctbx_list_t *lcs,
                                                                  LinphoneCoreManager *conf_mgr,
                                                                  bctbx_list_t *participants,
                                                                  const LinphoneCallParams *params);
LinphoneStatus add_calls_to_local_conference(bctbx_list_t *lcs,
                                             LinphoneCoreManager *conf_mgr,
                                             LinphoneConference *conference,
                                             bctbx_list_t *new_participants,
                                             bool_t one_by_one);
LinphoneStatus add_calls_to_remote_conference(bctbx_list_t *lcs,
                                              LinphoneCoreManager *focus_mgr,
                                              LinphoneCoreManager *conf_mgr,
                                              bctbx_list_t *new_participants,
                                              LinphoneConference *conference,
                                              bool_t one_by_one);
LinphoneStatus remove_participant_from_local_conference(bctbx_list_t *lcs,
                                                        LinphoneCoreManager *conf_mgr,
                                                        LinphoneCoreManager *participant_mgr,
                                                        LinphoneConference *conf);
LinphoneStatus terminate_conference(bctbx_list_t *lcs,
                                    LinphoneCoreManager *conf_mgr,
                                    LinphoneConference *conference,
                                    LinphoneCoreManager *focus_mgr,
                                    bool_t participants_exit_conference);
bctbx_list_t *terminate_participant_call(bctbx_list_t *participants,
                                         LinphoneCoreManager *conf_mgr,
                                         LinphoneCoreManager *participant_mgr);
LinphoneConferenceServer *linphone_conference_server_new(const char *rc_file, bool_t do_registration);
void initiate_calls(bctbx_list_t *caller, LinphoneCoreManager *callee);
void linphone_conference_server_destroy(LinphoneConferenceServer *conf_srv);
void check_nb_streams(LinphoneCoreManager *m1,
                      LinphoneCoreManager *m2,
                      const int nb_audio_streams,
                      const int nb_video_streams,
                      const int nb_text_streams);

LinphoneAddress *linphone_core_manager_resolve(LinphoneCoreManager *mgr, const LinphoneAddress *source);
FILE *sip_start(const char *senario, const char *dest_username, const char *passwd, LinphoneAddress *dest_addres);

void early_media_without_sdp_in_200_base(bool_t use_video, bool_t use_ice);
LinphoneNatPolicy *get_nat_policy_for_call(LinphoneCoreManager *mgr, LinphoneCall *call);
void enable_stun_in_mgr(LinphoneCoreManager *mgr,
                        const bool_t account_enable_stun,
                        const bool_t account_enable_ice,
                        const bool_t core_enable_stun,
                        const bool_t core_enable_ice);
void enable_stun_in_account(LinphoneCoreManager *mgr,
                            LinphoneAccount *account,
                            const bool_t enable_stun,
                            const bool_t enable_ice);
void enable_stun_in_core(LinphoneCoreManager *mgr, const bool_t enable_stun, const bool_t enable_ice);
void on_player_eof(LinphonePlayer *player);
void linphone_conf_event_notify(LinphoneEvent *lev);
void _check_friend_result_list(
    LinphoneCore *lc, const bctbx_list_t *resultList, const unsigned int index, const char *uri, const char *phone);
void _check_friend_result_list_2(LinphoneCore *lc,
                                 const bctbx_list_t *resultList,
                                 const unsigned int index,
                                 const char *uri,
                                 const char *phone,
                                 const char *name,
                                 int expected_flags);
void _check_friend_result_list_3(LinphoneCore *lc,
                                 const bctbx_list_t *resultList,
                                 const unsigned int index,
                                 const char *uri,
                                 const char *phone,
                                 const char *name);

/*Convenience function providing the path to the "empty_rc" config file*/
const char *liblinphone_tester_get_empty_rc(void);

int liblinphone_tester_copy_file(const char *from, const char *to);
size_t liblinphone_tester_load_text_file_in_buffer(const char *filePath, char **buffer);
char *generate_random_e164_phone_from_dial_plan(const LinphoneDialPlan *dialPlan);

void linphone_core_start_process_remote_notification(LinphoneCoreManager *mgr, const char *callid);
extern MSSndCardDesc dummy_test_snd_card_desc;
#define DUMMY_TEST_SOUNDCARD "dummy test sound card"

extern MSSndCardDesc dummy2_test_snd_card_desc;
#define DUMMY2_TEST_SOUNDCARD "dummy2 test sound card"

extern MSSndCardDesc dummy3_test_snd_card_desc;
#define DUMMY3_TEST_SOUNDCARD "dummy3 test sound card"

extern MSSndCardDesc dummy_playback_test_snd_card_desc;
#define DUMMY_PLAYBACK_TEST_SOUNDCARD "dummy playback test sound card"

extern MSSndCardDesc dummy_capture_test_snd_card_desc;
#define DUMMY_CAPTURE_TEST_SOUNDCARD "dummy capture test sound card"

/**
 * Set the requested curve and matching lime server url in the given core manager
 * WARNING: uses a dirty trick: the linphone_core_set_lime_x3dh_server_url will actually restart
 * the encryption engine (only if the given url is different than the current one). It will thus parse
 * again the curve setting that is changed BEFORE.
 */
typedef enum _LinphoneTesterLimeAlgo {
	UNSET = 0,
	C25519,
	C448,
	C25519K512,
	C25519MLK512,
	C448MLK1024
} LinphoneTesterLimeAlgo;
const char *limeAlgoEnum2String(const LinphoneTesterLimeAlgo curveId);
void set_lime_server_and_curve(const LinphoneTesterLimeAlgo curveId, LinphoneCoreManager *manager);
void legacy_set_lime_server_and_curve(const LinphoneTesterLimeAlgo curveId,
                                      LinphoneCoreManager *manager); // Set the lime server url in the [lime] section so
                                                                     // it is setup at core level not account
void set_lime_server_and_curve_list(const LinphoneTesterLimeAlgo curveId, bctbx_list_t *managerList);
void set_lime_server_and_curve_list_tls(const LinphoneTesterLimeAlgo curveId,
                                        bctbx_list_t *managerList,
                                        bool_t tls_auth_server,
                                        bool_t required);

void aggregated_imdns_in_group_chat_base(const LinphoneTesterLimeAlgo curveId);

bool is_filepath_encrypted(const char *filepath);
typedef struct _LinphoneAccountCreatorStats {
	int cb_done;
} LinphoneAccountCreatorStats;

LinphoneAccountCreatorStats *new_linphone_account_creator_stats(void);
void account_creator_set_cb_done(LinphoneAccountCreatorCbs *cbs);
void account_creator_reset_cb_done(LinphoneAccountCreatorCbs *cbs);

void lime_delete_DRSessions(const char *limedb, const char *requestOption);
void lime_setback_usersUpdateTs(const char *limedb, int days);
uint64_t lime_get_userUpdateTs(const char *limedb);
char *lime_get_userIk(LinphoneCoreManager *mgr, char *gruu, uint8_t curveId);
bool_t liblinphone_tester_is_lime_PQ_available(void);
void delete_all_in_zrtp_table(const char *zrtpdb);

void liblinphone_tester_simulate_mire_defunct(
    MSFilter *filter,
    bool_t defunct,
    float fps); // if defunct : Set fps to 0 and keep it on updates. if false : remove fps protection.
bctbx_list_t *liblinphone_tester_resolve_name_to_ip_address(const char *name);
bctbx_list_t *liblinphone_tester_remove_v6_addr(bctbx_list_t *l);

bool_t liblinphone_tester_is_executable_installed(const char *executable, const char *resource);
void liblinphone_tester_add_grammar_loader_path(const char *path);
#ifdef HAVE_SOCI
void liblinphone_tester_add_soci_search_path(const char *path);
#endif

/* Returns a unique path, useful for tests that need temporary files, so that they can have unique path
 * to be robust to parallel execution*/
char *liblinphone_tester_make_unique_file_path(const char *name, const char *extension);

int liblinphone_tester_check_recorded_audio(const char *hellopath, const char *recordpath);

/* returns a CPU bogomips indication. Only supported for linux.*/
float liblinphone_tester_get_cpu_bogomips(void);

/**
 * @brief liblinphone_tester_audio_device_name_match compare device name with string
 *
 * @param audio_device the audio device to compare @notnil
 * @param name the name to find in device name
 * @return the result of strcmp
 */
int liblinphone_tester_audio_device_name_match(const LinphoneAudioDevice *audio_device, const char *name);

/**
 * @brief liblinphone_tester_audio_device_match compare device ids between 2 devices
 *
 * @param a the first device
 * @param b the second device
 * @return the result of strcmp
 */
int liblinphone_tester_audio_device_match(const LinphoneAudioDevice *a, LinphoneAudioDevice *b);

/**
 * @brief liblinphone_tester_find_changing_devices Find the device that is missed/new into 2 lists.
 *
 * @param a first list to check. @maybenil
 * @param b second list to check. @maybenil
 * @param is_new TRUE if the changed device is new.
 * @return the device that changed or NULL if no change. Set is_new if the device has been
 * connected. @tobefreed @maybenil
 */
bctbx_list_t *liblinphone_tester_find_changing_devices(bctbx_list_t *a, bctbx_list_t *b, bool_t *is_new);

/**
 * @brief liblinphone_tester_sound_detection Check sounds between 2 Core on 200ms and exit the function on detection.
 *
 * @param a the first core manager to retrieve current call and make wait loop @notnil
 * @param b the second core manager to retrieve current call and make wait loop @notnil
 * @param timeout_ms timeout in ms
 * @param log_tag Use this tag to monitor level in logs. If NULL, log is switched off @maybenil
 * @return -1 if timed out else 0.
 */
int liblinphone_tester_sound_detection(LinphoneCoreManager *a,
                                       LinphoneCoreManager *b,
                                       int timeout_ms,
                                       const char *log_tag);

#ifdef __cplusplus
};
#endif

#endif /* LIBLINPHONE_TESTER_H_ */
