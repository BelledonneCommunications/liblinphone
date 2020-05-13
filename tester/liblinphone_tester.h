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


#ifndef LIBLINPHONE_TESTER_H_
#define LIBLINPHONE_TESTER_H_



#include <bctoolbox/tester.h>
#include "linphone/core.h"
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

extern test_suite_t account_creator_test_suite;
extern test_suite_t call_test_suite;
extern test_suite_t push_incoming_call_test_suite;

#if VIDEO_ENABLED
	extern test_suite_t call_video_test_suite;
	extern test_suite_t call_video_quality_test_suite;
#endif // if VIDEO_ENABLED

extern test_suite_t clonable_object_test_suite;
extern test_suite_t conference_event_test_suite;
extern test_suite_t conference_test_suite;
extern test_suite_t contents_test_suite;
extern test_suite_t cpim_test_suite;
extern test_suite_t dtmf_test_suite;
extern test_suite_t event_test_suite;
extern test_suite_t main_db_test_suite;
extern test_suite_t flexisip_test_suite;
extern test_suite_t group_chat_test_suite;
extern test_suite_t secure_group_chat_test_suite;
extern test_suite_t ephemeral_group_chat_test_suite;
extern test_suite_t log_collection_test_suite;
extern test_suite_t message_test_suite;
extern test_suite_t session_timers_test_suite;
extern test_suite_t multi_call_test_suite;
extern test_suite_t multicast_call_test_suite;
extern test_suite_t multipart_test_suite;
extern test_suite_t offeranswer_test_suite;
extern test_suite_t player_test_suite;
extern test_suite_t presence_server_test_suite;
extern test_suite_t presence_test_suite;
extern test_suite_t property_container_test_suite;
extern test_suite_t proxy_config_test_suite;
extern test_suite_t quality_reporting_test_suite;
extern test_suite_t register_test_suite;
extern test_suite_t remote_provisioning_test_suite;
extern test_suite_t setup_test_suite;
extern test_suite_t stun_test_suite;
extern test_suite_t tunnel_test_suite;
extern test_suite_t upnp_test_suite;
extern test_suite_t utils_test_suite;
extern test_suite_t video_test_suite;
extern test_suite_t call_recovery_test_suite;
extern test_suite_t call_with_ice_test_suite;
extern test_suite_t call_secure_test_suite;
extern test_suite_t call_with_rtp_bundle_test_suite;
extern test_suite_t shared_core_test_suite;
extern test_suite_t lime_server_auth_test_suite;
extern test_suite_t vfs_encryption_test_suite;
extern test_suite_t local_conference_test_suite;

#ifdef VCARD_ENABLED
	extern test_suite_t vcard_test_suite;
#endif

extern test_suite_t audio_bypass_suite;
#if HAVE_SIPP
	extern test_suite_t complex_sip_call_test_suite;
#endif

extern int manager_count;

extern int liblinphone_tester_ipv6_available(void);
extern int liblinphone_tester_ipv4_available(void);

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
extern void liblinphone_tester_keep_accounts( int keep );

/**
 * @brief Tells the test whether to not remove recorded audio/video files after the tests.
 * @details By default recorded files are erased after the test, unless the test is failed.
**/
void liblinphone_tester_keep_recorded_files(int keep);

/**
 * @brief Disable the automatic object leak detection. This is useful because the object leak detector prevents valgrind from seeing the leaks.
 * @details By default object leak detector is enabled.
**/
void liblinphone_tester_disable_leak_detector(int disabled);

/**
 * @brief Clears the created accounts during the testing session.
 */
extern void liblinphone_tester_clear_accounts(void);


extern const char* test_domain;
extern const char* auth_domain;
extern const char* test_username;
extern const char* test_sha_username;
extern const char* pure_sha256_user;
extern const char* test_password;
extern const char* test_route;
extern const char* userhostsfile;
extern const char* file_transfer_url;
extern const char* lime_server_c25519_url;
extern const char* lime_server_c448_url;
extern bool_t liblinphone_tester_keep_uuid;
extern bool_t liblinphone_tester_tls_support_disabled;
extern const MSAudioDiffParams audio_cmp_params;
extern const char *liblinphone_tester_mire_id;
extern const char *liblinphone_tester_static_image_id;
extern bool_t liblinphonetester_ipv6;
extern bool_t liblinphonetester_show_account_manager_logs;
extern bool_t liblinphonetester_no_account_creator;

typedef struct _stats {
	int number_of_LinphoneRegistrationNone;
	int number_of_LinphoneRegistrationProgress ;
	int number_of_LinphoneRegistrationOk ;
	int number_of_LinphoneRegistrationCleared ;
	int number_of_LinphoneRegistrationFailed ;
	int number_of_auth_info_requested ;

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

	int number_of_LinphoneTransferCallOutgoingInit;
	int number_of_LinphoneTransferCallOutgoingProgress;
	int number_of_LinphoneTransferCallOutgoingRinging;
	int number_of_LinphoneTransferCallOutgoingEarlyMedia;
	int number_of_LinphoneTransferCallConnected;
	int number_of_LinphoneTransferCallStreamsRunning;
	int number_of_LinphoneTransferCallError;

	int number_of_LinphoneMessageReceived;
	int number_of_LinphoneMessageReceivedWithFile;
	int number_of_LinphoneMessageExtBodyReceived;
	int number_of_LinphoneMessageInProgress;
	int number_of_LinphoneMessageDelivered;
	int number_of_LinphoneMessageNotDelivered;
	int number_of_LinphoneMessageUndecryptable;
	int number_of_LinphoneMessageFileTransferDone;
	int number_of_LinphoneMessageFileTransferError;
	int number_of_LinphoneMessageFileTransferInProgress;
	int number_of_LinphoneMessageDeliveredToUser;
	int number_of_LinphoneMessageDisplayed;
	int number_of_LinphoneMessageSent;
	int number_of_LinphoneMessageEphemeralTimerStarted;
	int number_of_LinphoneMessageEphemeralDeleted;
	int number_of_LinphoneIsComposingActiveReceived;
	int number_of_LinphoneIsComposingIdleReceived;
	int progress_of_LinphoneFileTransfer;

	int number_of_LinphoneChatRoomConferenceJoined;
	int number_of_LinphoneChatRoomStateInstantiated;
	int number_of_LinphoneChatRoomStateCreationPending;
	int number_of_LinphoneChatRoomStateCreated;
	int number_of_LinphoneChatRoomStateCreationFailed;
	int number_of_LinphoneChatRoomStateTerminationPending;
	int number_of_LinphoneChatRoomStateTerminated;
	int number_of_LinphoneChatRoomStateTerminationFailed;
	int number_of_LinphoneChatRoomStateDeleted;
	int number_of_LinphoneChatRoomEphemeralTimerStarted;
	int number_of_LinphoneChatRoomEphemeralDeleted;
	int number_of_X3dhUserCreationSuccess;
	int number_of_X3dhUserCreationFailure;

	int number_of_IframeDecoded;

	int number_of_NewSubscriptionRequest;
	int number_of_NotifyReceived;
	int number_of_NotifyPresenceReceived;
	int number_of_NotifyPresenceReceivedForUriOrTel;
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

	int number_of_inforeceived;
	LinphoneInfoMessage* last_received_info_message;

	int number_of_LinphoneSubscriptionIncomingReceived;
	int number_of_LinphoneSubscriptionOutgoingProgress;
	int number_of_LinphoneSubscriptionPending;
	int number_of_LinphoneSubscriptionActive;
	int number_of_LinphoneSubscriptionTerminated;
	int number_of_LinphoneSubscriptionError;
	int number_of_LinphoneSubscriptionExpiring;

	int number_of_LinphonePublishProgress;
	int number_of_LinphonePublishOk;
	int number_of_LinphonePublishExpiring;
	int number_of_LinphonePublishError;
	int number_of_LinphonePublishCleared;

	int number_of_LinphoneConfiguringSkipped;
	int number_of_LinphoneConfiguringFailed;
	int number_of_LinphoneConfiguringSuccessful;

	int number_of_LinphoneCallEncryptedOn;
	int number_of_LinphoneCallEncryptedOff;
	int number_of_NetworkReachableTrue;
	int number_of_NetworkReachableFalse;
	int number_of_player_eof;
	LinphoneChatMessage* last_received_chat_message;

	char * dtmf_list_received;
	int dtmf_count;

	int number_of_LinphoneCallStatsUpdated;
	int number_of_rtcp_sent;
	int number_of_rtcp_received; /*total number of rtcp packet received */
	int number_of_rtcp_received_via_mux;/*number of rtcp packet received in rtcp-mux mode*/

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

	int number_of_participants_added;
	int number_of_participant_admin_statuses_changed;
	int number_of_participants_removed;
	int number_of_subject_changed;
	int number_of_core_chat_room_subject_changed;
	int number_of_participant_devices_added;
	int number_of_participant_devices_removed;

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
}stats;


typedef struct _LinphoneCoreManager {
	LinphoneCoreCbs *cbs;
	LinphoneCore *lc;
	stats stat;
	LinphoneAddress *identity;
	LinphoneEvent *lev;
	bool_t decline_subscribe;
	int number_of_bcunit_error_at_creation;
	char *phone_alias;
	char *rc_path;
	char *rc_local;
	char *database_path;
	char *lime_database_path;
	char *app_group_id;
	bool_t main_core;
	void * user_info;
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

void linphone_core_manager_init(LinphoneCoreManager *mgr, const char* rc_file, const char* phone_alias);
void linphone_core_manager_init2(LinphoneCoreManager *mgr, const char* rc_file, const char* phone_alias);
void linphone_core_manager_init_shared(LinphoneCoreManager *mgr, const char* rc_file, const char* phone_alias, LinphoneCoreManager *mgr_to_copy);
LinphoneCore *linphone_core_manager_configure_lc(LinphoneCoreManager *mgr);
void linphone_core_manager_configure (LinphoneCoreManager *mgr);
void linphone_core_manager_start(LinphoneCoreManager *mgr, bool_t check_for_proxies);
LinphoneCoreManager* linphone_core_manager_create2(const char* rc_file, const char* phone_alias);
LinphoneCoreManager* linphone_core_manager_create(const char* rc_file);
LinphoneCoreManager* linphone_core_manager_new4(const char* rc_file, int check_for_proxies, const char* phone_aliasconst, const char* contact_params, int expires);
LinphoneCoreManager* linphone_core_manager_new3(const char* rc_file, bool_t check_for_proxies, const char* phone_alias);
LinphoneCoreManager* linphone_core_manager_new2(const char* rc_file, bool_t check_for_proxies);
LinphoneCoreManager* linphone_core_manager_new(const char* rc_file);
LinphoneCoreManager* linphone_core_manager_create_local(const char* rc_factory, const char* rc_local, const char *linphone_db, const char *lime_db);
LinphoneCoreManager* linphone_core_manager_new_local(const char* rc_factory, const char* rc_local, const char *linphone_db, const char *lime_db);
LinphoneCoreManager* linphone_core_manager_create_shared(const char *rc_file, const char *app_group_id, bool_t main_core, LinphoneCoreManager *mgr_to_copy);
void linphone_core_manager_stop(LinphoneCoreManager *mgr);
void linphone_core_manager_uninit_after_stop_async(LinphoneCoreManager *mgr);
void linphone_core_manager_reinit(LinphoneCoreManager *mgr);
void linphone_core_manager_restart(LinphoneCoreManager *mgr, bool_t check_for_proxies);
/* This function is used to restore the fake DNS which is lost after a linphone_core_stop() */
void linphone_core_manager_setup_dns(LinphoneCoreManager *mgr);
void linphone_core_manager_uninit(LinphoneCoreManager *mgr);
void linphone_core_manager_uninit2(LinphoneCoreManager *mgr, bool_t unlinkDb);
void linphone_core_manager_wait_for_stun_resolution(LinphoneCoreManager *mgr);
void linphone_core_manager_destroy(LinphoneCoreManager* mgr);
void linphone_core_manager_destroy_after_stop_async(LinphoneCoreManager* mgr);
void linphone_core_manager_delete_chat_room (LinphoneCoreManager *mgr, LinphoneChatRoom *cr, bctbx_list_t *coresList);
bctbx_list_t * init_core_for_conference(bctbx_list_t *coreManagerList);
void start_core_for_conference(bctbx_list_t *coreManagerList);
bctbx_list_t * init_core_for_conference_with_factory_uri(bctbx_list_t *coreManagerList, const char* factoryUri);

void reset_counters(stats* counters);

void registration_state_changed(struct _LinphoneCore *lc, LinphoneProxyConfig *cfg, LinphoneRegistrationState cstate, const char *message);
void call_state_changed(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *msg);
void linphone_transfer_state_changed(LinphoneCore *lc, LinphoneCall *transfered, LinphoneCallState new_call_state);
void notify_presence_received(LinphoneCore *lc, LinphoneFriend * lf);
void notify_presence_received_for_uri_or_tel(LinphoneCore *lc, LinphoneFriend *lf, const char *uri_or_tel, const LinphonePresenceModel *presence);
void message_received(LinphoneCore *lc, LinphoneChatRoom *room, LinphoneChatMessage* message);
void file_transfer_received(LinphoneChatMessage *message, LinphoneContent* content, const LinphoneBuffer *buffer);
LinphoneBuffer * tester_file_transfer_send(LinphoneChatMessage *message, LinphoneContent* content, size_t offset, size_t size);
LinphoneChatMessage *_send_message_ephemeral(LinphoneChatRoom *chatRoom, const char *message, bool_t ephemeral);
LinphoneChatMessage *_send_message(LinphoneChatRoom *chatRoom, const char *message);
void _send_file_plus_text(LinphoneChatRoom* cr, const char *sendFilepath, const char *sendFilepath2, const char *text, bool_t use_buffer);
void _send_file(LinphoneChatRoom* cr, const char *sendFilepath, const char *sendFilepath2, bool_t use_buffer);
void _receive_file(bctbx_list_t *coresList, LinphoneCoreManager *lcm, stats *receiverStats, const char *receive_filepath, const char *sendFilepath, const char *sendFilepath2, bool_t use_buffer);
void _receive_file_plus_text(bctbx_list_t *coresList, LinphoneCoreManager *lcm, stats *receiverStats, const char *receive_filepath, const char *sendFilepath, const char *sendFilepath2, const char *text, bool_t use_buffer);

LinphoneBuffer * tester_memory_file_transfer_send(LinphoneChatMessage *message, LinphoneContent* content, size_t offset, size_t size);
void file_transfer_progress_indication(LinphoneChatMessage *message, LinphoneContent* content, size_t offset, size_t total);
void is_composing_received(LinphoneCore *lc, LinphoneChatRoom *room);
void info_message_received(LinphoneCore *lc, LinphoneCall *call, const LinphoneInfoMessage *msg);
void new_subscription_requested(LinphoneCore *lc, LinphoneFriend *lf, const char *url);
void linphone_subscription_state_change(LinphoneCore *lc, LinphoneEvent *ev, LinphoneSubscriptionState state);
void linphone_publish_state_changed(LinphoneCore *lc, LinphoneEvent *ev, LinphonePublishState state);
void linphone_notify_received(LinphoneCore *lc, LinphoneEvent *lev, const char *eventname, const LinphoneContent *content);
void linphone_subscribe_received(LinphoneCore *lc, LinphoneEvent *lev, const char *eventname, const LinphoneContent *content);
void linphone_configuration_status(LinphoneCore *lc, LinphoneConfiguringState status, const char *message);
void linphone_call_encryption_changed(LinphoneCore *lc, LinphoneCall *call, bool_t on, const char *authentication_token);
void dtmf_received(LinphoneCore *lc, LinphoneCall *call, int dtmf);
void call_stats_updated(LinphoneCore *lc, LinphoneCall *call, const LinphoneCallStats *stats);
void global_state_changed(LinphoneCore *lc, LinphoneGlobalState gstate, const char *message);
void first_call_started(LinphoneCore *lc);
void last_call_ended(LinphoneCore *lc);
void audio_device_changed(LinphoneCore *lc, LinphoneAudioDevice *device);
void audio_devices_list_updated(LinphoneCore *lc);
	
LinphoneAddress * create_linphone_address(const char * domain);
LinphoneAddress * create_linphone_address_for_algo(const char * domain, const char * username);
bool_t wait_for(LinphoneCore* lc_1, LinphoneCore* lc_2,int* counter,int value);
bool_t wait_for_list(MSList* lcs,int* counter,int value,int timeout_ms);
bool_t wait_for_list_interval(MSList* lcs,int* counter,int min,int max,int timeout_ms);
bool_t wait_for_until(LinphoneCore* lc_1, LinphoneCore* lc_2,int* counter,int value,int timout_ms);
bool_t wait_for_until_interval(LinphoneCore* lc_1, LinphoneCore* lc_2,int* counter,int min,int max,int timout_ms);

bool_t call_with_params(LinphoneCoreManager* caller_mgr
						,LinphoneCoreManager* callee_mgr
						, const LinphoneCallParams *caller_params
						, const LinphoneCallParams *callee_params);
bool_t call_with_test_params(LinphoneCoreManager* caller_mgr
				,LinphoneCoreManager* callee_mgr
				,const LinphoneCallTestParams *caller_test_params
				,const LinphoneCallTestParams *callee_test_params);
bool_t call_with_params2(LinphoneCoreManager* caller_mgr
						,LinphoneCoreManager* callee_mgr
						, const LinphoneCallTestParams *caller_test_params
						, const LinphoneCallTestParams *callee_test_params
						, bool_t build_callee_params);

bool_t call(LinphoneCoreManager* caller_mgr,LinphoneCoreManager* callee_mgr);
bool_t request_video(LinphoneCoreManager* caller,LinphoneCoreManager* callee, bool_t use_accept_call_update);
void end_call(LinphoneCoreManager *m1, LinphoneCoreManager *m2);
void disable_all_audio_codecs_except_one(LinphoneCore *lc, const char *mime, int rate);
void disable_all_video_codecs_except_one(LinphoneCore *lc, const char *mime);
void disable_all_codecs(const MSList* elem, LinphoneCoreManager* call);
stats * get_stats(LinphoneCore *lc);
bool_t transport_supported(LinphoneTransportType transport);
LinphoneCoreManager *get_manager(LinphoneCore *lc);
const char *liblinphone_tester_get_subscribe_content(void);
const char *liblinphone_tester_get_notify_content(void);
void liblinphone_tester_chat_message_state_change(LinphoneChatMessage* msg,LinphoneChatMessageState state,void* ud);
void liblinphone_tester_chat_message_msg_state_changed(LinphoneChatMessage *msg, LinphoneChatMessageState state);
void liblinphone_tester_chat_room_msg_sent(LinphoneCore *lc, LinphoneChatRoom *room, LinphoneChatMessage *msg);
void liblinphone_tester_chat_message_ephemeral_timer_started(LinphoneChatMessage *msg);
void liblinphone_tester_chat_message_ephemeral_deleted(LinphoneChatMessage *msg);
void core_chat_room_state_changed (LinphoneCore *core, LinphoneChatRoom *cr, LinphoneChatRoomState state);
void liblinphone_tester_x3dh_user_created(LinphoneCore *lc, const bool_t status, const char* userId, const char *info);
void core_chat_room_subject_changed (LinphoneCore *core, LinphoneChatRoom *cr);

void liblinphone_tester_check_rtcp(LinphoneCoreManager* caller, LinphoneCoreManager* callee);
void liblinphone_tester_clock_start(MSTimeSpec *start);
bool_t liblinphone_tester_clock_elapsed(const MSTimeSpec *start, int value_ms);
void linphone_core_manager_check_accounts(LinphoneCoreManager *m);
void account_manager_destroy(void);
LinphoneAddress *account_manager_get_identity_with_modified_identity(const LinphoneAddress *modified_identity);
LinphoneCore *configure_lc_from(LinphoneCoreCbs *cbs, const char *path, LinphoneConfig *config, void *user_data);
void configure_core_for_callbacks(LinphoneCoreManager *lcm, LinphoneCoreCbs *cbs);

void liblinphone_tester_set_next_video_frame_decoded_cb(LinphoneCall *call);
void call_paused_resumed_base(bool_t multicast,bool_t with_losses);
void simple_call_base(bool_t enable_multicast_recv_side, bool_t disable_soundcard, bool_t use_multipart_invite_body);
void _call_with_rtcp_mux(bool_t caller_rtcp_mux, bool_t callee_rtcp_mux, bool_t with_ice,bool_t with_ice_reinvite);
void call_base_with_configfile(LinphoneMediaEncryption mode, bool_t enable_video,bool_t enable_relay,LinphoneFirewallPolicy policy,bool_t enable_tunnel, const char *marie_rc, const char *pauline_rc);
void call_base_with_configfile_play_nothing(LinphoneMediaEncryption mode, bool_t enable_video,bool_t enable_relay,LinphoneFirewallPolicy policy,bool_t enable_tunnel, const char *marie_rc, const char *pauline_rc);
void call_base(LinphoneMediaEncryption mode, bool_t enable_video,bool_t enable_relay,LinphoneFirewallPolicy policy,bool_t enable_tunnel);
bool_t call_with_caller_params(LinphoneCoreManager* caller_mgr,LinphoneCoreManager* callee_mgr, const LinphoneCallParams *params);
bool_t pause_call_1(LinphoneCoreManager* mgr_1,LinphoneCall* call_1,LinphoneCoreManager* mgr_2,LinphoneCall* call_2);
LinphoneAudioDevice * change_device(bool_t enable, LinphoneCoreManager* mgr, LinphoneAudioDevice *current_dev, LinphoneAudioDevice *dev0, LinphoneAudioDevice *dev1);
LinphoneAudioDevice* pause_call_changing_device(bool_t enable, bctbx_list_t *lcs, LinphoneCall *call, LinphoneCoreManager* mgr_pausing, LinphoneCoreManager* mgr_paused, LinphoneCoreManager* mgr_change_device, LinphoneAudioDevice *current_dev, LinphoneAudioDevice *dev0, LinphoneAudioDevice *dev1);
void compare_files(const char *path1, const char *path2);
void check_media_direction(LinphoneCoreManager* mgr, LinphoneCall *call, MSList* lcs,LinphoneMediaDirection audio_dir, LinphoneMediaDirection video_dir);
void _call_with_ice_base(LinphoneCoreManager* pauline,LinphoneCoreManager* marie, bool_t caller_with_ice, bool_t callee_with_ice, bool_t random_ports, bool_t forced_relay);
void record_call(const char *filename, bool_t enableVideo, const char *video_codec);

#define AUDIO_START	0
#define VIDEO_START	1
#define TEXT_START	2
int check_nb_media_starts(unsigned int media_type, LinphoneCoreManager *caller, LinphoneCoreManager *callee, unsigned int caller_nb_media_starts, unsigned int callee_nb_media_starts);

void setup_sdp_handling(const LinphoneCallTestParams* params, LinphoneCoreManager* mgr);

LinphoneChatRoom * create_chat_room_client_side(bctbx_list_t *lcs, LinphoneCoreManager *lcm, stats *initialStats, bctbx_list_t *participantsAddresses, const char* initialSubject, bool_t encrypted);
LinphoneChatRoom * check_creation_chat_room_client_side(bctbx_list_t *lcs, LinphoneCoreManager *lcm, stats *initialStats, const LinphoneAddress *confAddr, const char* subject, int participantNumber, bool_t isAdmin);
void configure_core_for_conference (LinphoneCore *core, const char* username, const LinphoneAddress *factoryAddr, bool_t server);
void _configure_core_for_conference (LinphoneCoreManager *lcm, LinphoneAddress *factoryAddr);
void _start_core(LinphoneCoreManager *lcm);
extern const char *sFactoryUri;

/*
 * this function return max value in the last 3 seconds*/
int linphone_core_manager_get_max_audio_down_bw(const LinphoneCoreManager *mgr);
int linphone_core_manager_get_max_audio_up_bw(const LinphoneCoreManager *mgr);
int linphone_core_manager_get_mean_audio_down_bw(const LinphoneCoreManager *mgr);
int linphone_core_manager_get_mean_audio_up_bw(const LinphoneCoreManager *mgr);

void video_call_base_2(LinphoneCoreManager* pauline,LinphoneCoreManager* marie, bool_t using_policy,LinphoneMediaEncryption mode, bool_t callee_video_enabled, bool_t caller_video_enabled);

void liblinphone_tester_before_each(void);
void liblinphone_tester_after_each(void);
void liblinphone_tester_init(void(*ftester_printf)(int level, const char *fmt, va_list args));
void liblinphone_tester_uninit(void);
int liblinphone_tester_set_log_file(const char *filename);
bool_t check_ice(LinphoneCoreManager* caller, LinphoneCoreManager* callee, LinphoneIceState state);


LinphoneConferenceServer* linphone_conference_server_new(const char *rc_file, bool_t do_registration);
void linphone_conference_server_destroy(LinphoneConferenceServer *conf_srv);

LinphoneAddress * linphone_core_manager_resolve(LinphoneCoreManager *mgr, const LinphoneAddress *source);
FILE *sip_start(const char *senario, const char* dest_username, const char *passwd, LinphoneAddress* dest_addres);

void early_media_without_sdp_in_200_base( bool_t use_video, bool_t use_ice );
void linphone_conf_event_notify(LinphoneEvent *lev);
void _check_friend_result_list(LinphoneCore *lc, const bctbx_list_t *resultList, const unsigned int index, const char* uri, const char* phone);

/*Convenience function providing the path to the "empty_rc" config file*/
const char *liblinphone_tester_get_empty_rc(void);

int liblinphone_tester_copy_file(const char *from, const char *to);
size_t liblinphone_tester_load_text_file_in_buffer(const char *filePath, char **buffer);
char * generate_random_e164_phone_from_dial_plan(const LinphoneDialPlan *dialPlan);


void linphone_core_start_process_remote_notification (LinphoneCoreManager *mgr, const char *callid);
extern MSSndCardDesc dummy_test_snd_card_desc;
#define DUMMY_TEST_SOUNDCARD "dummy test sound card"

extern MSSndCardDesc dummy2_test_snd_card_desc;
#define DUMMY2_TEST_SOUNDCARD "dummy2 test sound card"

extern MSSndCardDesc dummy3_test_snd_card_desc;
#define DUMMY3_TEST_SOUNDCARD "dummy3 test sound card"

/**
 * Set the requested curve and matching lime server url in the given core manager
 * WARNING: uses a dirty trick: the linphone_core_set_lime_x3dh_server_url will actually restart
 * the encryption engine (only if the given url is different than the current one). It will thus parse
 * again the curve setting that is changed BEFORE.
 */
void set_lime_curve(const int curveId, LinphoneCoreManager *manager);
void set_lime_curve_list(const int curveId, bctbx_list_t *managerList);
void set_lime_curve_list_tls(const int curveId, bctbx_list_t *managerList, bool_t tls_auth_server, bool_t required);


#ifdef __cplusplus
};
#endif

#endif /* LIBLINPHONE_TESTER_H_ */
