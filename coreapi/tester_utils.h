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

#ifndef _TESTER_UTILS_H_
#define _TESTER_UTILS_H_

#ifdef HAVE_SQLITE
#include <sqlite3.h>
#else
typedef struct _sqlite3 sqlite3;
#endif

#include "account_creator/private.h"
#include "c-wrapper/internal/c-sal.h"
#include "linphone/core.h"
#include "linphone/tunnel.h"
#include "mediastreamer2/msmire.h"
#include "quality_reporting.h"

#ifndef __cplusplus
typedef struct _Sal Sal;
typedef struct _SalOp SalOp;
#endif

#ifdef __cplusplus
LINPHONE_BEGIN_NAMESPACE
class SalMediaDescription;
class SalEventOp;
class SalSubscribeOp;
class Sal;
LINPHONE_END_NAMESPACE
LINPHONE_PUBLIC LinphonePrivate::SalMediaDescription *_linphone_call_get_local_desc(const LinphoneCall *call);
LINPHONE_PUBLIC LinphonePrivate::SalMediaDescription *_linphone_call_get_remote_desc(const LinphoneCall *call);
LINPHONE_PUBLIC LinphonePrivate::SalMediaDescription *_linphone_call_get_result_desc(const LinphoneCall *call);
extern "C" {
LINPHONE_PUBLIC LinphoneEvent *linphone_event_new_subscribe_with_op(LinphoneCore *lc,
                                                                    LinphonePrivate::SalSubscribeOp *op,
                                                                    LinphoneSubscriptionDir dir,
                                                                    const char *name);
}
#endif

typedef enum _LinphoneProxyConfigAddressComparisonResult {
	LinphoneProxyConfigAddressDifferent,
	LinphoneProxyConfigAddressEqual,
	LinphoneProxyConfigAddressWeakEqual
} LinphoneProxyConfigAddressComparisonResult;

typedef struct _LinphoneCoreToneManagerStats {
	int number_of_startRingbackTone;
	int number_of_startRingtone;
	int number_of_startNamedTone;
	int number_of_stopRingbackTone;
	int number_of_stopRingtone;
	int number_of_stopTone;
} LinphoneCoreToneManagerStats;

typedef struct _LinphoneStreamInternalStats {
	unsigned int number_of_starts;
	unsigned int number_of_stops;
	unsigned int number_of_ice_check_list_relay_pair_verified;
	unsigned int number_of_ice_check_list_processing_finished;
	unsigned int number_of_dtls_starts;
} LinphoneStreamInternalStats;

/**
 * Chat room session state changed callback
 * @param chat_room #LinphoneChatRoom that has been marked as read. @notnil
 * @param state the new #LinphoneCallState of the call
 * @param message a non NULL informational message about the state. @notnil
 */
typedef void (*LinphoneChatRoomCbsSessionStateChangedCb)(LinphoneChatRoom *chat_room,
                                                         LinphoneCallState state,
                                                         const char *message);

/**
 * The LinphoneVcardContext object.
 */
typedef struct _LinphoneVcardContext LinphoneVcardContext;

#ifdef __cplusplus
extern "C" {
#endif

// Required to check SRTP crypto suite match during call
LINPHONE_PUBLIC const MSCryptoSuite *linphone_core_get_srtp_crypto_suites_array(LinphoneCore *lc);

LINPHONE_PUBLIC LinphoneVcardContext *linphone_core_get_vcard_context(const LinphoneCore *lc);
LINPHONE_PUBLIC bool_t linphone_core_rtcp_enabled(const LinphoneCore *lc);
LINPHONE_PUBLIC void linphone_core_get_local_ip(LinphoneCore *lc, int af, const char *dest, char *result);
LINPHONE_PUBLIC int linphone_core_get_local_ip_for(int type, const char *dest, char *result);
LINPHONE_PUBLIC void linphone_core_enable_forced_ice_relay(LinphoneCore *lc, bool_t enable);
LINPHONE_PUBLIC void linphone_core_set_zrtp_not_available_simulation(LinphoneCore *lc, bool_t enabled);
LINPHONE_PUBLIC belle_http_provider_t *linphone_core_get_http_provider(LinphoneCore *lc);
LINPHONE_PUBLIC IceSession *linphone_call_get_ice_session(const LinphoneCall *call);
LINPHONE_PUBLIC const struct addrinfo *linphone_core_get_stun_server_addrinfo(LinphoneCore *lc);
LINPHONE_PUBLIC void linphone_core_enable_send_call_stats_periodical_updates(LinphoneCore *lc, bool_t enabled);
LINPHONE_PUBLIC LinphoneAccount *linphone_core_lookup_known_account(LinphoneCore *lc, const LinphoneAddress *uri);
LINPHONE_PUBLIC const bctbx_list_t *linphone_core_get_deleted_account_list(const LinphoneCore *lc);

LINPHONE_PUBLIC int linphone_run_stun_tests(LinphoneCore *lc,
                                            int audioPort,
                                            int videoPort,
                                            int textPort,
                                            char *audioCandidateAddr,
                                            int *audioCandidatePort,
                                            char *videoCandidateAddr,
                                            int *videoCandidatePort,
                                            char *textCandidateAddr,
                                            int *textCandidatePort);
LINPHONE_PUBLIC void linphone_core_enable_short_turn_refresh(LinphoneCore *lc, bool_t enable);

LINPHONE_PUBLIC void linphone_core_set_zrtp_cache_db(LinphoneCore *lc, sqlite3 *cache_db);

LINPHONE_PUBLIC LinphoneCoreCbs *linphone_core_get_first_callbacks(const LinphoneCore *lc);
LINPHONE_PUBLIC void _linphone_core_add_callbacks(LinphoneCore *lc, LinphoneCoreCbs *vtable, bool_t internal);

LINPHONE_PUBLIC bctbx_list_t *linphone_core_read_call_logs_from_config_file(LinphoneCore *lc);
LINPHONE_PUBLIC bctbx_list_t **linphone_core_get_call_logs_attribute(LinphoneCore *lc);
LINPHONE_PUBLIC void linphone_core_delete_call_log(LinphoneCore *lc, LinphoneCallLog *log);

LINPHONE_PUBLIC const MSList *linphone_core_get_call_history(LinphoneCore *lc);
LINPHONE_PUBLIC void linphone_core_delete_call_history(LinphoneCore *lc);
LINPHONE_PUBLIC int linphone_core_get_call_history_size(LinphoneCore *lc);

LINPHONE_PUBLIC void linphone_core_set_keep_stream_direction_for_rejected_stream(LinphoneCore *lc, bool_t yesno);
LINPHONE_PUBLIC bool_t linphone_core_get_keep_stream_direction_for_rejected_stream(LinphoneCore *lc);

LINPHONE_DEPRECATED LINPHONE_PUBLIC void linphone_core_cbs_set_auth_info_requested(LinphoneCoreCbs *cbs,
                                                                                   LinphoneCoreAuthInfoRequestedCb cb);

LINPHONE_PUBLIC LinphoneProxyConfigAddressComparisonResult
linphone_proxy_config_is_server_config_changed(const LinphoneProxyConfig *obj);
LINPHONE_PUBLIC LinphoneProxyConfigAddressComparisonResult
linphone_proxy_config_address_equal(const LinphoneAddress *a, const LinphoneAddress *b);

LINPHONE_PUBLIC MediaStream *linphone_call_get_stream(LinphoneCall *call, LinphoneStreamType type);
LINPHONE_PUBLIC VideoStream *linphone_core_get_preview_stream(LinphoneCore *call);
LINPHONE_PUBLIC bool_t linphone_call_get_all_muted(const LinphoneCall *call);
LINPHONE_PUBLIC LinphoneAccount *linphone_call_get_dest_account(const LinphoneCall *call);
LINPHONE_PUBLIC unsigned int _linphone_call_get_nb_audio_starts(const LinphoneCall *call);
LINPHONE_PUBLIC unsigned int _linphone_call_get_nb_audio_stops(const LinphoneCall *call);
LINPHONE_PUBLIC unsigned int _linphone_call_get_nb_video_starts(const LinphoneCall *call);
LINPHONE_PUBLIC unsigned int _linphone_call_get_nb_text_starts(const LinphoneCall *call);
LINPHONE_PUBLIC const LinphoneStreamInternalStats *_linphone_call_get_stream_internal_stats(const LinphoneCall *call,
                                                                                            LinphoneStreamType type);
LINPHONE_PUBLIC belle_sip_source_t *_linphone_call_get_dtmf_timer(const LinphoneCall *call);
LINPHONE_PUBLIC bool_t _linphone_call_has_dtmf_sequence(const LinphoneCall *call);

LINPHONE_PUBLIC MSWebCam *_linphone_call_get_video_device(const LinphoneCall *call);
LINPHONE_PUBLIC void _linphone_call_add_local_desc_changed_flag(LinphoneCall *call, int flag);
LINPHONE_PUBLIC int _linphone_call_get_main_audio_stream_index(const LinphoneCall *call);
LINPHONE_PUBLIC int _linphone_call_get_main_text_stream_index(const LinphoneCall *call);
LINPHONE_PUBLIC int _linphone_call_get_main_video_stream_index(const LinphoneCall *call);

LINPHONE_PUBLIC void linphone_call_params_set_no_user_consent(LinphoneCallParams *params, bool_t value);
LINPHONE_PUBLIC bool_t linphone_call_params_get_update_call_when_ice_completed(const LinphoneCallParams *params);
LINPHONE_PUBLIC void linphone_call_params_set_video_download_bandwidth(LinphoneCallParams *params, int bw);

LINPHONE_PUBLIC int _linphone_call_stats_get_updated(const LinphoneCallStats *stats);
LINPHONE_PUBLIC bool_t _linphone_call_stats_rtcp_received_via_mux(const LinphoneCallStats *stats);
LINPHONE_PUBLIC mblk_t *_linphone_call_stats_get_received_rtcp(const LinphoneCallStats *stats);
LINPHONE_PUBLIC bool_t _linphone_call_stats_has_received_rtcp(const LinphoneCallStats *stats);
LINPHONE_PUBLIC bool_t _linphone_call_stats_has_sent_rtcp(const LinphoneCallStats *stats);

LINPHONE_PUBLIC LinphoneQualityReporting *linphone_call_log_get_quality_reporting(LinphoneCallLog *call_log);
LINPHONE_PUBLIC reporting_session_report_t **
linphone_quality_reporting_get_reports(LinphoneQualityReporting *qreporting);

LINPHONE_PUBLIC bool_t linphone_call_compare_video_color(LinphoneCall *call,
                                                         MSMireControl cl,
                                                         MediaStreamDir dir,
                                                         const char *label);
LINPHONE_PUBLIC bool_t linphone_call_check_rtp_sessions(LinphoneCall *call);

LINPHONE_PUBLIC int _linphone_chat_room_get_transient_message_count(const LinphoneChatRoom *cr);
LINPHONE_PUBLIC LinphoneChatMessage *_linphone_chat_room_get_first_transient_message(const LinphoneChatRoom *cr);
LINPHONE_PUBLIC bctbx_list_t *linphone_core_fetch_friends_from_db(LinphoneCore *lc, LinphoneFriendList *list);
LINPHONE_PUBLIC bctbx_list_t *linphone_core_fetch_friends_lists_from_db(LinphoneCore *lc);
LINPHONE_PUBLIC void linphone_friend_invalidate_subscription(LinphoneFriend *lf);
LINPHONE_PUBLIC void linphone_friend_update_subscribes(LinphoneFriend *fr, bool_t only_when_registered);
LINPHONE_PUBLIC bctbx_list_t *linphone_friend_get_insubs(const LinphoneFriend *fr);
LINPHONE_PUBLIC int linphone_friend_list_get_expected_notification_version(const LinphoneFriendList *list);
LINPHONE_PUBLIC long long linphone_friend_list_get_storage_id(const LinphoneFriendList *list);
LINPHONE_PUBLIC long long linphone_friend_get_storage_id(const LinphoneFriend *lf);
LINPHONE_PUBLIC const bctbx_list_t *linphone_friend_list_get_dirty_friends_to_update(const LinphoneFriendList *lfl);
LINPHONE_PUBLIC const char *linphone_friend_list_get_revision(const LinphoneFriendList *lfl);

LINPHONE_PUBLIC int linphone_remote_provisioning_load_file(LinphoneCore *lc, const char *file_path);

LINPHONE_PUBLIC char *linphone_core_get_device_identity(LinphoneCore *lc);

LINPHONE_PUBLIC const LinphoneCoreToneManagerStats *linphone_core_get_tone_manager_stats(LinphoneCore *lc);
LINPHONE_PUBLIC void linphone_core_reset_tone_manager_stats(LinphoneCore *lc);
LINPHONE_PUBLIC const char *linphone_core_get_tone_file(LinphoneCore *lc, LinphoneToneID id);

/**
 * set the EKT to all audio and video streams active in the call
 *
 * @param[in] ekt_params	All data needed to set the EKT
 */
LINPHONE_PUBLIC void linphone_call_set_ekt(const LinphoneCall *call, const MSEKTParametersSet *ekt_params);
/**
 * Send a request to delete an account on server.
 * @param[in] creator LinphoneAccountCreator object
 * @return LinphoneAccountCreatorStatusRequestOk if the request has been sent, LinphoneAccountCreatorStatusRequestFailed
 *otherwise
 * @donotwrap Exists for tests purposes only
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus linphone_account_creator_delete_account(LinphoneAccountCreator *creator);

/**
 * Send a request to get the account confirmation key on server.
 * @param[in] creator LinphoneAccountCreator object
 * @return LinphoneAccountCreatorStatusRequestOk if the request has been sent, LinphoneAccountCreatorStatusRequestFailed
 *otherwise
 * @donotwrap Exists for tests purposes only
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorStatus
linphone_account_creator_get_confirmation_key(LinphoneAccountCreator *creator);

/**
 * Get the delete account request.
 * @param[in] cbs LinphoneAccountCreatorCbs object.
 * @return The current delete account request.
 * @donotwrap Exists for tests purposes only
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorCbsStatusCb
linphone_account_creator_cbs_get_delete_account(const LinphoneAccountCreatorCbs *cbs);

/**
 * Assign a user pointer to a LinphoneAccountCreatorCbs object.
 * @param[in] cbs LinphoneAccountCreatorCbs object.
 * @param[in] cb The delete account request to be used.
 * @donotwrap Exists for tests purposes only
 **/
LINPHONE_PUBLIC void linphone_account_creator_cbs_set_delete_account(LinphoneAccountCreatorCbs *cbs,
                                                                     LinphoneAccountCreatorCbsStatusCb cb);

/**
 * Get the get confirmation key request.
 * @param[in] cbs LinphoneAccountCreatorCbs object.
 * @return The current is account exist request.
 * @donotwrap Exists for tests purposes only
 **/
LINPHONE_PUBLIC LinphoneAccountCreatorCbsStatusCb
linphone_account_creator_cbs_get_get_confirmation_key(const LinphoneAccountCreatorCbs *cbs);

/**
 * Assign a user pointer to a LinphoneAccountCreatorCbs object.
 * @param[in] cbs LinphoneAccountCreatorCbs object.
 * @param[in] cb The get confirmation key request to be used.
 * @donotwrap Exists for tests purposes only
 **/
LINPHONE_PUBLIC void linphone_account_creator_cbs_set_confirmation_key(LinphoneAccountCreatorCbs *cbs,
                                                                       LinphoneAccountCreatorCbsStatusCb cb);

/**
 * Set the #LinphoneChatRoomCbsSessionStateChangedCb callback.
 * @param cbs A #LinphoneChatRoomCbs. @notnil
 * @param cb The callback.
 */
LINPHONE_PUBLIC void linphone_chat_room_cbs_set_session_state_changed(LinphoneChatRoomCbs *cbs,
                                                                      LinphoneChatRoomCbsSessionStateChangedCb cb);

/**
 * Get the #LinphoneChatRoomCbsSessionStateChangedCb callback.
 * @param cbs A #LinphoneChatRoomCbs. @notnil
 * @return The callback.
 */
LINPHONE_PUBLIC LinphoneChatRoomCbsSessionStateChangedCb
linphone_chat_room_cbs_get_session_state_changed(const LinphoneChatRoomCbs *cbs);

/**
 * Uses belcard to parse the content of a buffer and returns one vCard if possible, or NULL otherwise.
 * @param[in] context the vCard context to use (speed up the process by not creating a Belcard parser each time)
 * @param[in] buffer the buffer to parse
 * @return a LinphoneVcard if one could be parsed, or NULL otherwise
 */
LINPHONE_PUBLIC LinphoneVcard *linphone_vcard_context_get_vcard_from_buffer(LinphoneVcardContext *context,
                                                                            const char *buffer);
/**
 * Set the force LIME X3DH decryption failure flag
 * @param[in] lc LinphoneCore object
 * @donotwrap Exists for tests purposes only
 **/
LINPHONE_PUBLIC void linphone_core_lime_x3dh_set_test_decryption_failure_flag(const LinphoneCore *lc, bool_t flag);
LINPHONE_PUBLIC bool_t linphone_core_lime_x3dh_is_user_active(LinphoneCore *lc, const char *userId);
LINPHONE_PUBLIC void linphone_core_set_network_reachable_internal(LinphoneCore *lc, bool_t is_reachable);

LINPHONE_PUBLIC bctbx_list_t *linphone_fetch_local_addresses(void);
LINPHONE_PUBLIC void linphone_core_reset_shared_core_state(LinphoneCore *lc);
LINPHONE_PUBLIC char *linphone_core_get_download_path(LinphoneCore *lc);

LINPHONE_PUBLIC const char *linphone_core_get_conference_version(const LinphoneCore *lc);
LINPHONE_PUBLIC const char *linphone_core_get_groupchat_version(const LinphoneCore *lc);
LINPHONE_PUBLIC const char *linphone_core_get_ephemeral_version(const LinphoneCore *lc);

LINPHONE_PUBLIC size_t linphone_chat_room_get_previouses_conference_ids_count(LinphoneChatRoom *cr);
LINPHONE_PUBLIC void linphone_conference_info_set_uri(LinphoneConferenceInfo *conference_info,
                                                      const LinphoneAddress *uri);
LINPHONE_PUBLIC void linphone_conference_info_set_state(LinphoneConferenceInfo *conference_info,
                                                        LinphoneConferenceInfoState state);
LINPHONE_PUBLIC int linphone_participant_info_get_sequence_number(const LinphoneParticipantInfo *participant_info);
LINPHONE_PUBLIC void linphone_conference_info_set_ics_sequence(LinphoneConferenceInfo *conference_info,
                                                               unsigned int sequence);
LINPHONE_PUBLIC unsigned int linphone_conference_info_get_ics_sequence(const LinphoneConferenceInfo *conference_info);
LINPHONE_PUBLIC time_t linphone_conference_info_get_earlier_joining_time(const LinphoneConferenceInfo *conference_info);
LINPHONE_PUBLIC time_t linphone_conference_info_get_expiry_time(const LinphoneConferenceInfo *conference_info);
LINPHONE_PUBLIC void linphone_participant_info_set_sequence_number(LinphoneParticipantInfo *participant_info,
                                                                   int sequence);
LINPHONE_PUBLIC bool_t linphone_participant_preserve_session(const LinphoneParticipant *participant);

LINPHONE_PUBLIC time_t linphone_conference_params_get_earlier_joining_time(const LinphoneConferenceParams *params);
LINPHONE_PUBLIC time_t linphone_conference_params_get_expiry_time(const LinphoneConferenceParams *params);

LINPHONE_PUBLIC char *sal_get_random_token_lowercase(int size);

LINPHONE_PUBLIC void linphone_core_set_answer_with_own_numbering_policy(LinphoneCore *lc, bool_t value);

#ifndef __cplusplus
LINPHONE_PUBLIC Sal *linphone_core_get_sal(const LinphoneCore *lc);
LINPHONE_PUBLIC SalOp *linphone_proxy_config_get_sal_op(const LinphoneProxyConfig *cfg);
LINPHONE_PUBLIC SalOp *linphone_call_get_op_as_sal_op(const LinphoneCall *call);

LINPHONE_PUBLIC Sal *sal_init(MSFactory *factory);
LINPHONE_PUBLIC void sal_uninit(Sal *sal);

LINPHONE_PUBLIC int sal_create_uuid(Sal *ctx, char *uuid, size_t len);
LINPHONE_PUBLIC char *sal_get_random_token(int size);
LINPHONE_PUBLIC void sal_set_uuid(Sal *ctx, const char *uuid);
LINPHONE_PUBLIC const char *sal_get_uuid(const Sal *ctx);

LINPHONE_PUBLIC void sal_default_set_sdp_handling(Sal *h, SalOpSDPHandling handling_method);

LINPHONE_PUBLIC void sal_set_send_error(Sal *sal, int value);
LINPHONE_PUBLIC void sal_set_recv_error(Sal *sal, int value);
LINPHONE_PUBLIC void sal_set_client_bind_port(Sal *sal, int port);
LINPHONE_PUBLIC int sal_enable_pending_trans_checking(Sal *sal, bool_t value);
LINPHONE_PUBLIC void sal_enable_unconditional_answer(Sal *sal, bool_t value);
LINPHONE_PUBLIC void sal_set_unconditional_answer(Sal *sal, unsigned short value);
LINPHONE_PUBLIC void sal_set_dns_timeout(Sal *sal, int timeout);
LINPHONE_PUBLIC void sal_set_dns_user_hosts_file(Sal *sal, const char *hosts_file);
LINPHONE_PUBLIC const char *sal_get_dns_user_hosts_file(const Sal *sal);
LINPHONE_PUBLIC void *sal_get_stack_impl(Sal *sal);
LINPHONE_PUBLIC void sal_set_refresher_retry_after(Sal *sal, int value);
LINPHONE_PUBLIC int sal_get_refresher_retry_after(const Sal *sal);
LINPHONE_PUBLIC void sal_set_transport_timeout(Sal *sal, int timeout);
LINPHONE_PUBLIC void sal_enable_test_features(Sal *ctx, bool_t value);
LINPHONE_PUBLIC bool_t sal_transport_available(Sal *ctx, SalTransport t);

LINPHONE_PUBLIC const SalErrorInfo *sal_op_get_error_info(const SalOp *op);
LINPHONE_PUBLIC bool_t sal_call_dialog_request_pending(const SalOp *op);
LINPHONE_PUBLIC void sal_call_set_sdp_handling(SalOp *h, SalOpSDPHandling handling);
LINPHONE_PUBLIC const char *sal_call_get_local_tag(SalOp *op);
LINPHONE_PUBLIC const char *sal_call_get_remote_tag(SalOp *op);
LINPHONE_PUBLIC void sal_call_set_replaces(SalOp *op, const char *callId, const char *fromTag, const char *toTag);

LINPHONE_PUBLIC belle_sip_resolver_context_t *
sal_resolve_a(Sal *sal, const char *name, int port, int family, belle_sip_resolver_callback_t cb, void *data);

LINPHONE_PUBLIC Sal *sal_op_get_sal(SalOp *op);
LINPHONE_PUBLIC SalOp *sal_create_refer_op(Sal *sal);
LINPHONE_PUBLIC void sal_release_op(SalOp *op);
LINPHONE_PUBLIC void sal_op_set_from(SalOp *sal_refer_op, const char *from);
LINPHONE_PUBLIC void sal_op_set_to(SalOp *sal_refer_op, const char *to);
LINPHONE_PUBLIC void sal_op_send_refer(SalOp *sal_refer_op, SalAddress *refer_to);
LINPHONE_PUBLIC void sal_set_user_pointer(Sal *sal, void *user_pointer);
LINPHONE_PUBLIC void *sal_get_user_pointer(Sal *sal);
LINPHONE_PUBLIC void sal_set_call_refer_callback(Sal *sal, void (*OnReferCb)(SalOp *op, const SalAddress *referto));

LINPHONE_PUBLIC LinphoneAddress *linphone_proxy_config_get_transport_contact(LinphoneProxyConfig *cfg);

LINPHONE_PUBLIC void linphone_call_start_basic_incoming_notification(LinphoneCall *call);
LINPHONE_PUBLIC void linphone_call_start_push_incoming_notification(LinphoneCall *call);
LINPHONE_PUBLIC LinphoneCall *linphone_call_new_incoming_with_callid(LinphoneCore *lc, const char *callid);
LINPHONE_PUBLIC bool_t linphone_call_is_op_configured(const LinphoneCall *call);

LINPHONE_PUBLIC LinphoneContent *linphone_content_copy(const LinphoneContent *ref);

LINPHONE_PUBLIC void sal_custom_header_unref(SalCustomHeader *ch);
LINPHONE_PUBLIC SalCustomHeader *sal_custom_header_append(SalCustomHeader *ch, const char *name, const char *value);

LINPHONE_PUBLIC SalAddress *sal_address_clone(const SalAddress *addr);
LINPHONE_PUBLIC void sal_address_unref(SalAddress *addr);

LINPHONE_PUBLIC void linphone_event_set_state(LinphoneEvent *lev, LinphoneSubscriptionState state);

LINPHONE_PUBLIC void linphone_participant_device_set_state(LinphoneParticipantDevice *participant_device,
                                                           LinphoneParticipantDeviceState state);
LINPHONE_PUBLIC LinphoneCore *linphone_participant_device_get_core(const LinphoneParticipantDevice *participant_device);

#endif // !defined(__cplusplus)

LINPHONE_PUBLIC bool_t linphone_tunnel_is_tunnel_rtp_transport(const LinphoneTunnel *tunnel, const RtpTransport *tp);

LINPHONE_PUBLIC void linphone_config_simulate_crash_during_sync(LinphoneConfig *lpconfig, bool_t value);
LINPHONE_PUBLIC void linphone_config_simulate_read_failure(bool_t value);

LINPHONE_PUBLIC void linphone_payload_type_set_priority_bonus(LinphonePayloadType *pt, bool_t value);

LINPHONE_PUBLIC bool_t linphone_account_lime_enabled(LinphoneAccount *account);

LINPHONE_PUBLIC void linphone_call_restart_main_audio_stream(LinphoneCall *call);

LINPHONE_PUBLIC int linphone_core_get_number_of_duplicated_messages(const LinphoneCore *core);

LINPHONE_PUBLIC void linphone_core_set_account_deletion_timeout(LinphoneCore *core, unsigned int seconds);

LINPHONE_PUBLIC void linphone_core_set_push_notification_config(LinphoneCore *core,
                                                                LinphonePushNotificationConfig *config);

LINPHONE_PUBLIC LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_get_account_creation_token_as_admin_request(
    LinphoneAccountManagerServices *ams);
LINPHONE_PUBLIC LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_get_account_info_as_admin_request(LinphoneAccountManagerServices *ams,
                                                                           int account_id);
LINPHONE_PUBLIC LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_create_delete_account_as_admin_request(LinphoneAccountManagerServices *ams,
                                                                         int account_id);

LINPHONE_PUBLIC void linphone_core_enable_goog_remb(LinphoneCore *core, bool_t enable);

/**
 * Do not prune gr parameter in conference address
 * @param core #LinphoneCore object @notnil
 * @param enabled TRUE if enabled, FALSE otherwise.
 **/
LINPHONE_PUBLIC void linphone_core_enable_gruu_in_conference_address(LinphoneCore *core, bool_t enabled);

/**
 * Return whether the gr parameter is kept in the conference address
 * Returns enablement of text sending via Baudot tones in the audio stream.
 * @ingroup media_parameters
 * @param core #LinphoneCore object @notnil
 * @return TRUE if the "gr" parameter is kept in the conference address, FALSE otherwise.
 **/
LINPHONE_PUBLIC bool_t linphone_core_gruu_in_conference_address_enabled(const LinphoneCore *core);

LINPHONE_PUBLIC LinphoneChatRoom *
linphone_core_create_basic_chat_room(LinphoneCore *core, const char *localSipUri, const char *remoteSipUri);

#ifdef __cplusplus
}
#endif

#endif // _TESTER_UTILS_H_
