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

#include "c-wrapper/c-wrapper.h"
#include "core/core-p.h"
#include "linphone/wrapper_utils.h"
#include "private.h"

LinphoneCoreVTable *linphone_core_v_table_new() {
	return ms_new0(LinphoneCoreVTable, 1);
}

void linphone_core_v_table_set_user_data(LinphoneCoreVTable *table, void *data) {
	table->user_data = data;
}

void *linphone_core_v_table_get_user_data(const LinphoneCoreVTable *table) {
	return table->user_data;
}

void linphone_core_v_table_destroy(LinphoneCoreVTable *table) {
	ms_free(table);
}

LinphoneCoreVTable *linphone_core_get_current_vtable(LinphoneCore *lc) {
	if (lc->current_cbs != NULL) return lc->current_cbs->vtable;
	else return NULL;
}

static void cleanup_dead_vtable_refs(LinphoneCore *lc) {
	bctbx_list_t *it, *next_it;

	if (lc->vtable_notify_recursion > 0) return; /*don't cleanup vtable if we are iterating through a listener list.*/
	for (it = lc->vtable_refs; it != NULL;) {
		VTableReference *ref = (VTableReference *)it->data;
		next_it = it->next;
		if (ref->valid == 0) {
			lc->vtable_refs = bctbx_list_erase_link(lc->vtable_refs, it);
			belle_sip_object_unref(ref->cbs);
			ms_free(ref);
		}
		it = next_it;
	}
}

#define NOTIFY_IF_EXIST(function_name, ...)                                                                            \
	if (lc->is_unreffing)                                                                                              \
		return; /* This is to prevent someone from taking a ref in a callback called while the Core is being destroyed \
		           after last unref */                                                                                 \
	bctbx_list_t *iterator;                                                                                            \
	VTableReference *ref;                                                                                              \
	bool_t has_cb = FALSE;                                                                                             \
	lc->vtable_notify_recursion++;                                                                                     \
	for (iterator = lc->vtable_refs; iterator != NULL; iterator = iterator->next) {                                    \
		if ((ref = (VTableReference *)iterator->data)->valid && (lc->current_cbs = ref->cbs)->vtable->function_name) { \
			lc->current_cbs->vtable->function_name(__VA_ARGS__);                                                       \
			has_cb = TRUE;                                                                                             \
		}                                                                                                              \
	}                                                                                                                  \
	lc->vtable_notify_recursion--;                                                                                     \
	if (has_cb) {                                                                                                      \
		if (linphone_core_get_global_state(lc) == LinphoneGlobalStartup) {                                             \
			ms_debug("Linphone core [%p] notified [%s]", lc, #function_name);                                          \
		} else {                                                                                                       \
			ms_message("Linphone core [%p] notified [%s]", lc, #function_name);                                        \
		}                                                                                                              \
	}

#define NOTIFY_IF_EXIST_INTERNAL(function_name, internal_val, ...)                                                     \
	bctbx_list_t *iterator;                                                                                            \
	VTableReference *ref;                                                                                              \
	lc->vtable_notify_recursion++;                                                                                     \
	bool_t internal_val_evaluation = (internal_val);                                                                   \
	for (iterator = lc->vtable_refs; iterator != NULL; iterator = iterator->next) {                                    \
		if ((ref = (VTableReference *)iterator->data)->valid && (lc->current_cbs = ref->cbs)->vtable->function_name && \
		    (ref->internal == internal_val_evaluation)) {                                                              \
			lc->current_cbs->vtable->function_name(__VA_ARGS__);                                                       \
		}                                                                                                              \
	}                                                                                                                  \
	lc->vtable_notify_recursion--;

void linphone_core_notify_global_state_changed(LinphoneCore *lc, LinphoneGlobalState gstate, const char *message) {
	L_GET_PRIVATE_FROM_C_OBJECT(lc)->notifyGlobalStateChanged(gstate);
	NOTIFY_IF_EXIST(global_state_changed, lc, gstate, message);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_call_state_changed(LinphoneCore *lc,
                                             LinphoneCall *call,
                                             LinphoneCallState cstate,
                                             const char *message) {
	L_GET_PRIVATE_FROM_C_OBJECT(lc)->notifyCallStateChanged(call, cstate, message);
	NOTIFY_IF_EXIST(call_state_changed, lc, call, cstate, message);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_first_call_started(LinphoneCore *lc) {
	NOTIFY_IF_EXIST(first_call_started, lc);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_last_call_ended(LinphoneCore *lc) {
	NOTIFY_IF_EXIST(last_call_ended, lc);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_audio_device_changed(LinphoneCore *lc, LinphoneAudioDevice *audioDevice) {
	NOTIFY_IF_EXIST(audio_device_changed, lc, audioDevice);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_audio_devices_list_updated(LinphoneCore *lc) {
	NOTIFY_IF_EXIST(audio_devices_list_updated, lc);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_call_goclear_ack_sent(LinphoneCore *lc, LinphoneCall *call) {
	NOTIFY_IF_EXIST(call_goclear_ack_sent, lc, call);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_call_encryption_changed(LinphoneCore *lc,
                                                  LinphoneCall *call,
                                                  bool_t on,
                                                  const char *authentication_token) {
	NOTIFY_IF_EXIST(call_encryption_changed, lc, call, on, authentication_token);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_call_send_master_key_changed(LinphoneCore *lc, LinphoneCall *call, const char *master_key) {
	NOTIFY_IF_EXIST(call_send_master_key_changed, lc, call, master_key);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_call_receive_master_key_changed(LinphoneCore *lc,
                                                          LinphoneCall *call,
                                                          const char *master_key) {
	NOTIFY_IF_EXIST(call_receive_master_key_changed, lc, call, master_key);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_registration_state_changed(LinphoneCore *lc,
                                                     LinphoneProxyConfig *cfg,
                                                     LinphoneRegistrationState cstate,
                                                     const char *message) {
	L_GET_PRIVATE_FROM_C_OBJECT(lc)->notifyRegistrationStateChanged(cfg, cstate, message);
	NOTIFY_IF_EXIST(registration_state_changed, lc, cfg, cstate, message);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_account_registration_state_changed(LinphoneCore *lc,
                                                             LinphoneAccount *account,
                                                             LinphoneRegistrationState state,
                                                             const char *message) {
	L_GET_PRIVATE_FROM_C_OBJECT(lc)->notifyRegistrationStateChanged(
	    LinphonePrivate::Account::toCpp(account)->getSharedFromThis(), state, message);
	NOTIFY_IF_EXIST(account_registration_state_changed, lc, account, state, message);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_notify_presence_received(LinphoneCore *lc, LinphoneFriend *lf) {
	if (linphone_config_get_int(lc->config, "misc", "notify_each_friend_individually_when_presence_received", 1)) {
		NOTIFY_IF_EXIST(notify_presence_received, lc, lf);
		cleanup_dead_vtable_refs(lc);
	}
}

void linphone_core_notify_notify_presence_received_for_uri_or_tel(LinphoneCore *lc,
                                                                  LinphoneFriend *lf,
                                                                  const char *uri_or_tel,
                                                                  const LinphonePresenceModel *presence_model) {
	if (linphone_config_get_int(lc->config, "misc", "notify_each_friend_individually_when_presence_received", 1)) {
		NOTIFY_IF_EXIST(notify_presence_received_for_uri_or_tel, lc, lf, uri_or_tel, presence_model);
		cleanup_dead_vtable_refs(lc);
	}
}

void linphone_core_notify_new_subscription_requested(LinphoneCore *lc, LinphoneFriend *lf, const char *url) {
	NOTIFY_IF_EXIST(new_subscription_requested, lc, lf, url);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_authentication_requested(LinphoneCore *lc, LinphoneAuthInfo *ai, LinphoneAuthMethod method) {
	NOTIFY_IF_EXIST(authentication_requested, lc, ai, method);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_call_log_updated(LinphoneCore *lc, LinphoneCallLog *newcl) {
	NOTIFY_IF_EXIST(call_log_updated, lc, newcl);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_call_id_updated(LinphoneCore *lc, const char *previous, const char *current) {
	NOTIFY_IF_EXIST(call_id_updated, lc, previous, current);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_message_waiting_indication_changed(LinphoneCore *lc,
                                                             LinphoneEvent *lev,
                                                             const LinphoneMessageWaitingIndication *mwi) {
	NOTIFY_IF_EXIST(message_waiting_indication_changed, lc, lev, mwi);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_snapshot_taken(LinphoneCore *lc, const char *file_path) {
	NOTIFY_IF_EXIST(snapshot_taken, lc, file_path);
	cleanup_dead_vtable_refs(lc);
}

#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic push
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4996)
#else
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

void linphone_core_notify_auth_info_requested(LinphoneCore *lc,
                                              const char *realm,
                                              const char *username,
                                              const char *domain) {
	NOTIFY_IF_EXIST(auth_info_requested, lc, realm, username, domain);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_text_message_received(LinphoneCore *lc,
                                                LinphoneChatRoom *room,
                                                const LinphoneAddress *from,
                                                const char *message) {
	NOTIFY_IF_EXIST(text_received, lc, room, from, message);
	cleanup_dead_vtable_refs(lc);
}
#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic pop
#endif

void linphone_core_notify_message_received(LinphoneCore *lc, LinphoneChatRoom *room, LinphoneChatMessage *message) {
	NOTIFY_IF_EXIST(message_received, lc, room, message);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_remaining_number_of_file_transfer_changed(LinphoneCore *lc,
                                                                    unsigned int download_count,
                                                                    unsigned int upload_count) {
	NOTIFY_IF_EXIST(remaining_number_of_file_transfer_changed, lc, download_count, upload_count);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_new_message_reaction(LinphoneCore *lc,
                                               LinphoneChatRoom *room,
                                               LinphoneChatMessage *message,
                                               const LinphoneChatMessageReaction *reaction) {
	NOTIFY_IF_EXIST(new_message_reaction, lc, room, message, reaction);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_message_reaction_removed(LinphoneCore *lc,
                                                   LinphoneChatRoom *room,
                                                   LinphoneChatMessage *message,
                                                   const LinphoneAddress *address) {
	NOTIFY_IF_EXIST(reaction_removed, lc, room, message, address);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_message_reaction_removed_private(LinphoneCore *lc,
                                                           LinphoneChatRoom *room,
                                                           LinphoneChatMessage *message,
                                                           const LinphoneAddress *address,
                                                           const char *callId) {
	NOTIFY_IF_EXIST(reaction_removed_private, lc, room, message, address, callId);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_messages_received(LinphoneCore *lc, LinphoneChatRoom *room, const bctbx_list_t *messages) {
	NOTIFY_IF_EXIST(messages_received, lc, room, messages);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_message_sent(LinphoneCore *lc, LinphoneChatRoom *room, LinphoneChatMessage *message) {
	NOTIFY_IF_EXIST(message_sent, lc, room, message);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_chat_room_session_state_changed(LinphoneCore *lc,
                                                          LinphoneChatRoom *cr,
                                                          LinphoneCallState cstate,
                                                          const char *message) {
	NOTIFY_IF_EXIST(chat_room_session_state_changed, lc, cr, cstate, message);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_chat_room_read(LinphoneCore *lc, LinphoneChatRoom *room) {
	NOTIFY_IF_EXIST(chat_room_read, lc, room);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_message_received_unable_decrypt(LinphoneCore *lc,
                                                          LinphoneChatRoom *room,
                                                          LinphoneChatMessage *message) {
	NOTIFY_IF_EXIST(message_received_unable_decrypt, lc, room, message);
	cleanup_dead_vtable_refs(lc);
}
#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic push
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4996)
#else
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
void linphone_core_notify_file_transfer_recv(
    LinphoneCore *lc, LinphoneChatMessage *message, LinphoneContent *content, const char *buff, size_t size) {
	NOTIFY_IF_EXIST(file_transfer_recv, lc, message, content, buff, size);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_file_transfer_send(
    LinphoneCore *lc, LinphoneChatMessage *message, LinphoneContent *content, char *buff, size_t *size) {
	NOTIFY_IF_EXIST(file_transfer_send, lc, message, content, buff, size);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_file_transfer_progress_indication(
    LinphoneCore *lc, LinphoneChatMessage *message, LinphoneContent *content, size_t offset, size_t total) {
	NOTIFY_IF_EXIST(file_transfer_progress_indication, lc, message, content, offset, total);
	cleanup_dead_vtable_refs(lc);
}
#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic pop
#endif
void linphone_core_notify_is_composing_received(LinphoneCore *lc, LinphoneChatRoom *room) {
	LinphoneImNotifPolicy *policy = linphone_core_get_im_notif_policy(lc);
	if (linphone_im_notif_policy_get_recv_is_composing(policy) == TRUE) {
		NOTIFY_IF_EXIST(is_composing_received, lc, room);
		cleanup_dead_vtable_refs(lc);
	}
}

void linphone_core_notify_dtmf_received(LinphoneCore *lc, LinphoneCall *call, int dtmf) {
	NOTIFY_IF_EXIST(dtmf_received, lc, call, dtmf);
	cleanup_dead_vtable_refs(lc);
}

bool_t linphone_core_dtmf_received_has_listener(const LinphoneCore *lc) {
	bctbx_list_t *iterator;
	for (iterator = lc->vtable_refs; iterator != NULL; iterator = iterator->next) {
		VTableReference *ref = (VTableReference *)iterator->data;
		if (ref->valid && ref->cbs->vtable->dtmf_received) return TRUE;
	}
	return FALSE;
}

void linphone_core_notify_refer_received(LinphoneCore *lc,
                                         const LinphoneAddress *referToAddr,
                                         const LinphoneHeaders *customHeaders,
                                         const LinphoneContent *content) {
	NOTIFY_IF_EXIST(refer_received, lc, referToAddr, customHeaders, content);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_buddy_info_updated(LinphoneCore *lc, LinphoneFriend *lf) {
	NOTIFY_IF_EXIST(buddy_info_updated, lc, lf);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_transfer_state_changed(LinphoneCore *lc,
                                                 LinphoneCall *transferred,
                                                 LinphoneCallState new_call_state) {
	NOTIFY_IF_EXIST(transfer_state_changed, lc, transferred, new_call_state);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_call_stats_updated(LinphoneCore *lc, LinphoneCall *call, const LinphoneCallStats *stats) {
	NOTIFY_IF_EXIST(call_stats_updated, lc, call, stats);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_info_received(LinphoneCore *lc, LinphoneCall *call, const LinphoneInfoMessage *msg) {
	NOTIFY_IF_EXIST(info_received, lc, call, msg);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_configuring_status(LinphoneCore *lc, LinphoneConfiguringState status, const char *message) {
	NOTIFY_IF_EXIST(configuring_status, lc, status, message);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_network_reachable(LinphoneCore *lc, bool_t reachable) {
	L_GET_PRIVATE_FROM_C_OBJECT(lc)->notifyNetworkReachable(!!lc->sip_network_state.global_state,
	                                                        !!lc->media_network_state.global_state);
	NOTIFY_IF_EXIST(network_reachable, lc, reachable);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_notify_sent(LinphoneCore *lc, LinphoneEvent *lev, const LinphoneContent *body) {
	NOTIFY_IF_EXIST(notify_sent, lc, lev, body);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_notify_received(LinphoneCore *lc,
                                          LinphoneEvent *lev,
                                          const char *notified_event,
                                          const LinphoneContent *body) {
	NOTIFY_IF_EXIST_INTERNAL(notify_received, linphone_event_is_internal(lev), lc, lev, notified_event, body);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_subscribe_received(LinphoneCore *lc,
                                             LinphoneEvent *lev,
                                             const char *subscribe_event,
                                             const LinphoneContent *body) {
	NOTIFY_IF_EXIST_INTERNAL(subscribe_received, linphone_event_is_internal(lev), lc, lev, subscribe_event, body);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_subscription_state_changed(LinphoneCore *lc,
                                                     LinphoneEvent *lev,
                                                     LinphoneSubscriptionState state) {
	NOTIFY_IF_EXIST_INTERNAL(subscription_state_changed, linphone_event_is_internal(lev), lc, lev, state);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_publish_state_changed(LinphoneCore *lc, LinphoneEvent *lev, LinphonePublishState state) {
	NOTIFY_IF_EXIST_INTERNAL(publish_state_changed, linphone_event_is_internal(lev), lc, lev, state);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_publish_received(LinphoneCore *lc,
                                           LinphoneEvent *lev,
                                           const char *publish_event,
                                           const LinphoneContent *body) {
	NOTIFY_IF_EXIST_INTERNAL(publish_received, linphone_event_is_internal(lev), lc, lev, publish_event, body);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_log_collection_upload_state_changed(LinphoneCore *lc,
                                                              LinphoneCoreLogCollectionUploadState state,
                                                              const char *info) {
	NOTIFY_IF_EXIST(log_collection_upload_state_changed, lc, state, info);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_log_collection_upload_progress_indication(LinphoneCore *lc, size_t offset, size_t total) {
	NOTIFY_IF_EXIST(log_collection_upload_progress_indication, lc, offset, total);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_friend_list_created(LinphoneCore *lc, LinphoneFriendList *list) {
	NOTIFY_IF_EXIST(friend_list_created, lc, list);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_friend_list_removed(LinphoneCore *lc, LinphoneFriendList *list) {
	NOTIFY_IF_EXIST(friend_list_removed, lc, list);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_call_created(LinphoneCore *lc, LinphoneCall *call) {
	NOTIFY_IF_EXIST(call_created, lc, call);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_version_update_check_result_received(LinphoneCore *lc,
                                                               LinphoneVersionUpdateCheckResult result,
                                                               const char *version,
                                                               const char *url) {
	NOTIFY_IF_EXIST(version_update_check_result_received, lc, result, version, url);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_chat_room_state_changed(LinphoneCore *lc, LinphoneChatRoom *cr, LinphoneChatRoomState state) {
	NOTIFY_IF_EXIST(chat_room_state_changed, lc, cr, state);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_conference_state_changed(LinphoneCore *lc,
                                                   LinphoneConference *conference,
                                                   LinphoneConferenceState cstate) {
	NOTIFY_IF_EXIST(conference_state_changed, lc, conference, cstate);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_chat_room_subject_changed(LinphoneCore *lc, LinphoneChatRoom *cr) {
	NOTIFY_IF_EXIST(chat_room_subject_changed, lc, cr);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_chat_room_ephemeral_message_deleted(LinphoneCore *lc, LinphoneChatRoom *cr) {
	NOTIFY_IF_EXIST(chat_room_ephemeral_message_deleted, lc, cr);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_imee_user_registration(LinphoneCore *lc,
                                                 bool_t status,
                                                 const char *userId,
                                                 const char *info) {
	NOTIFY_IF_EXIST(imee_user_registration, lc, status, userId, info);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_qrcode_found(LinphoneCore *lc, const char *result) {
	NOTIFY_IF_EXIST(qrcode_found, lc, result);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_ec_calibration_result(LinphoneCore *lc, LinphoneEcCalibratorStatus status, int delay_ms) {
	NOTIFY_IF_EXIST(ec_calibration_result, lc, status, delay_ms);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_ec_calibration_audio_init(LinphoneCore *lc) {
	NOTIFY_IF_EXIST(ec_calibration_audio_init, lc);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_ec_calibration_audio_uninit(LinphoneCore *lc) {
	NOTIFY_IF_EXIST(ec_calibration_audio_uninit, lc);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_chat_room_exhumed(LinphoneCore *lc, LinphoneChatRoom *chat_room) {
	NOTIFY_IF_EXIST(chat_room_exhumed, lc, chat_room);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_preview_display_error_occurred(LinphoneCore *lc, int error_code) {
	NOTIFY_IF_EXIST(preview_display_error_occurred, lc, error_code);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_conference_info_received(LinphoneCore *lc, const LinphoneConferenceInfo *conference_info) {
	NOTIFY_IF_EXIST(conference_info_received, lc, conference_info);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_push_notification_received(LinphoneCore *lc, const char *payload) {
	NOTIFY_IF_EXIST(push_notification_received, lc, payload);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_default_account_changed(LinphoneCore *lc, LinphoneAccount *account) {
	NOTIFY_IF_EXIST(default_account_changed, lc, account);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_account_added(LinphoneCore *lc, LinphoneAccount *account) {
	NOTIFY_IF_EXIST(account_added, lc, account);
	cleanup_dead_vtable_refs(lc);
}

void linphone_core_notify_account_removed(LinphoneCore *lc, LinphoneAccount *account) {
	NOTIFY_IF_EXIST(account_removed, lc, account);
	cleanup_dead_vtable_refs(lc);
}

static VTableReference *v_table_reference_new(LinphoneCoreCbs *cbs, bool_t internal) {
	VTableReference *ref = ms_new0(VTableReference, 1);
	ref->valid = TRUE;
	ref->internal = internal;
	ref->cbs = linphone_core_cbs_ref(cbs);
	return ref;
}

void v_table_reference_destroy(VTableReference *ref) {
	linphone_core_cbs_unref(ref->cbs);
	ms_free(ref);
}

void _linphone_core_add_callbacks(LinphoneCore *lc, LinphoneCoreCbs *vtable, bool_t internal) {
	ms_message("Core callbacks [%p] registered on core [%p]", vtable, lc);
	lc->vtable_refs = bctbx_list_append(lc->vtable_refs, v_table_reference_new(vtable, internal));
}

void linphone_core_add_listener(LinphoneCore *lc, LinphoneCoreVTable *vtable) {
	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	_linphone_core_cbs_set_v_table(cbs, vtable, FALSE);
	_linphone_core_add_callbacks(lc, cbs, FALSE);
	linphone_core_cbs_unref(cbs);
}

void linphone_core_add_callbacks(LinphoneCore *lc, LinphoneCoreCbs *cbs) {
	_linphone_core_add_callbacks(lc, cbs, FALSE);
}

void linphone_core_remove_listener(LinphoneCore *lc, const LinphoneCoreVTable *vtable) {
	bctbx_list_t *it;
	ms_message("Vtable [%p] unregistered on core [%p]", vtable, lc);
	for (it = lc->vtable_refs; it != NULL; it = it->next) {
		VTableReference *ref = (VTableReference *)it->data;
		if (ref->cbs->vtable == vtable) {
			ref->valid = FALSE;
		}
	}
}

bctbx_list_t *linphone_core_get_callbacks_list(const LinphoneCore *lc) {
	bctbx_list_t *result = NULL;
	bctbx_list_t *it;
	for (it = lc->vtable_refs; it != NULL; it = it->next) {
		VTableReference *ref = (VTableReference *)it->data;
		if (!ref->internal) {
			// DO NOT RETURN INTERNAL LISTENERS HERE TO PREVENT ISSUES IN WRAPPERS
			result = bctbx_list_append(result, ref->cbs);
		}
	}
	return result;
}

void linphone_core_remove_callbacks(LinphoneCore *lc, const LinphoneCoreCbs *cbs) {
	bctbx_list_t *it;
	ms_message("Callbacks [%p] unregistered on core [%p]", cbs, lc);
	for (it = lc->vtable_refs; it != NULL; it = it->next) {
		VTableReference *ref = (VTableReference *)it->data;
		if (ref->cbs == cbs) {
			ref->valid = FALSE;
		}
	}
}

void linphone_core_notify_alert(LinphoneCore *lc, LinphoneAlert *alert) {
	NOTIFY_IF_EXIST(new_alert_triggered, lc, alert);
	cleanup_dead_vtable_refs(lc);
}
