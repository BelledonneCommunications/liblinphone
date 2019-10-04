/*
linphone
Copyright (C) 2000  Simon MORLAT (simon.morlat@linphone.org)
Copyright (C) 2010  Belledonne Communications SARL

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <sstream>

#include "linphone/api/c-content.h"
#include "linphone/core_utils.h"
#include "linphone/core.h"
#include "linphone/logging.h"
#include "linphone/lpconfig.h"
#include "linphone/sipsetup.h"

#include "private.h"
#include "logging-private.h"
#include "quality_reporting.h"
#include "lime.h"
#include "conference_private.h"
#include "logger/logger.h"

#include "sqlite3_bctbx_vfs.h"

#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ortp/telephonyevents.h>
#include <mediastreamer2/zrtp.h>
#include <mediastreamer2/dtls_srtp.h>
#include "bctoolbox/defs.h"
#include "bctoolbox/regex.h"
#include "belr/grammarbuilder.h"

#include "mediastreamer2/dtmfgen.h"
#include "mediastreamer2/mediastream.h"
#include "mediastreamer2/msequalizer.h"
#include "mediastreamer2/mseventqueue.h"
#include "mediastreamer2/msfactory.h"
#include "mediastreamer2/msjpegwriter.h"
#include "mediastreamer2/msogl.h"
#include "mediastreamer2/msvolume.h"
#include "mediastreamer2/msqrcodereader.h"

#include "bctoolbox/charconv.h"

#ifdef HAVE_ADVANCED_IM
#include "chat/chat-room/client-group-chat-room-p.h"
#include "chat/chat-room/client-group-to-basic-chat-room.h"
#include "chat/chat-room/server-group-chat-room-p.h"
#include "conference/handlers/local-conference-list-event-handler.h"
#include "conference/handlers/remote-conference-event-handler.h"
#include "conference/handlers/remote-conference-event-handler-p.h"
#include "conference/handlers/remote-conference-list-event-handler.h"
#endif
#include "content/content-manager.h"
#include "content/content-type.h"
#include "core/core-p.h"

// For migration purpose.
#include "address/address-p.h"
#include "c-wrapper/c-wrapper.h"


#ifdef INET6
#ifndef _WIN32
#include <netdb.h>
#endif
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "gitversion.h"
#endif

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

#include "c-wrapper/c-wrapper.h"
#include "call/call-p.h"
#include "conference/params/media-session-params-p.h"

#ifdef HAVE_ZLIB
#define COMPRESSED_LOG_COLLECTION_EXTENSION "gz"
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#ifndef fileno
#define fileno _fileno
#endif
#define unlink _unlink
#define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#define SET_BINARY_MODE(file)
#endif
#include <zlib.h>
#else
#define COMPRESSED_LOG_COLLECTION_EXTENSION "txt"
#endif
#define LOG_COLLECTION_DEFAULT_PATH "."
#define LOG_COLLECTION_DEFAULT_PREFIX "linphone"
#define LOG_COLLECTION_DEFAULT_MAX_FILE_SIZE (10 * 1024 * 1024)


/*#define UNSTANDART_GSM_11K 1*/

static const char *liblinphone_version=
#ifdef LIBLINPHONE_GIT_VERSION
	LIBLINPHONE_GIT_VERSION
#else
	LIBLINPHONE_VERSION
#endif
;

inline OrtpLogLevel operator|=(OrtpLogLevel a, OrtpLogLevel b) {
	int ia = static_cast<int>(a);
	int ib = static_cast<int>(b);
	return static_cast<OrtpLogLevel>(ia |= ib);
}

static OrtpLogFunc liblinphone_user_log_func = bctbx_logv_out; /*by default, user log handler = stdout*/
static OrtpLogFunc liblinphone_current_log_func = NULL; /*can be either logcolection or user_log*/
static LinphoneLogCollectionState liblinphone_log_collection_state = LinphoneLogCollectionDisabled;
static char * liblinphone_log_collection_path = NULL;
static char * liblinphone_log_collection_prefix = NULL;
static size_t liblinphone_log_collection_max_file_size = LOG_COLLECTION_DEFAULT_MAX_FILE_SIZE;
static ortp_mutex_t liblinphone_log_collection_mutex;
static FILE * liblinphone_log_collection_file = NULL;
static size_t liblinphone_log_collection_file_size = 0;
static bool_t liblinphone_serialize_logs = FALSE;
static void set_sip_network_reachable(LinphoneCore* lc,bool_t isReachable, time_t curtime);
static void set_media_network_reachable(LinphoneCore* lc,bool_t isReachable);
static void linphone_core_run_hooks(LinphoneCore *lc);
static void linphone_core_zrtp_cache_close(LinphoneCore *lc);
void linphone_core_zrtp_cache_db_init(LinphoneCore *lc, const char *fileName);

#include "enum.h"
#include "contact_providers_priv.h"

const char *linphone_core_get_nat_address_resolved(LinphoneCore *lc);
static void toggle_video_preview(LinphoneCore *lc, bool_t val);


/* relative path where is stored local ring*/
#define LOCAL_RING_WAV "oldphone-mono.wav"
#define LOCAL_RING_MKV "notes_of_the_optimistic.mkv"
/* same for remote ring (ringback)*/
#define REMOTE_RING_WAV "ringback.wav"

#define HOLD_MUSIC_WAV "toy-mono.wav"
#define HOLD_MUSIC_MKV "dont_wait_too_long.mkv"

using namespace std;

using namespace LinphonePrivate;

extern Sal::Callbacks linphone_sal_callbacks;


static void _linphone_core_cbs_uninit(LinphoneCoreCbs *cbs);

typedef belle_sip_object_t_vptr_t LinphoneCoreCbs_vptr_t;
BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneCoreCbs);
BELLE_SIP_INSTANCIATE_VPTR(LinphoneCoreCbs, belle_sip_object_t,
	_linphone_core_cbs_uninit, // destroy
	NULL, // clone
	NULL, // Marshall
	FALSE
);

LinphoneCoreCbs *_linphone_core_cbs_new(void) {
	LinphoneCoreCbs *obj = belle_sip_object_new(LinphoneCoreCbs);
	obj->vtable = ms_new0(LinphoneCoreVTable, 1);
	obj->autorelease = TRUE;
	return obj;
}

static void _linphone_core_cbs_uninit(LinphoneCoreCbs *cbs) {
	if (cbs->autorelease) ms_free(cbs->vtable);
}

void _linphone_core_cbs_set_v_table(LinphoneCoreCbs *cbs, LinphoneCoreVTable *vtable, bool_t autorelease) {
	ms_free(cbs->vtable);
	cbs->vtable = vtable;
	cbs->autorelease = autorelease;
}

LinphoneCoreCbs *linphone_core_cbs_ref(LinphoneCoreCbs *cbs) {
	return (LinphoneCoreCbs *)belle_sip_object_ref(cbs);
}

void linphone_core_cbs_unref(LinphoneCoreCbs *cbs) {
	belle_sip_object_unref(cbs);
}

void linphone_core_cbs_set_user_data(LinphoneCoreCbs *cbs, void *user_data) {
	cbs->vtable->user_data = user_data;
}

void *linphone_core_cbs_get_user_data(const LinphoneCoreCbs *cbs) {
	return cbs->vtable->user_data;
}

LinphoneCoreCbs *linphone_core_get_current_callbacks(const LinphoneCore *lc) {
	return lc->current_cbs;
}

LinphoneCoreCbsGlobalStateChangedCb linphone_core_cbs_get_global_state_changed(LinphoneCoreCbs *cbs) {
	return cbs->vtable->global_state_changed;
}

void linphone_core_cbs_set_global_state_changed(LinphoneCoreCbs *cbs, LinphoneCoreCbsGlobalStateChangedCb cb) {
	cbs->vtable->global_state_changed = cb;
}

LinphoneCoreCbsRegistrationStateChangedCb linphone_core_cbs_get_registration_state_changed(LinphoneCoreCbs *cbs) {
	return cbs->vtable->registration_state_changed;
}

void linphone_core_cbs_set_registration_state_changed(LinphoneCoreCbs *cbs, LinphoneCoreCbsRegistrationStateChangedCb cb) {
	cbs->vtable->registration_state_changed = cb;
}

LinphoneCoreCbsCallStateChangedCb linphone_core_cbs_get_call_state_changed(LinphoneCoreCbs *cbs) {
	return cbs->vtable->call_state_changed;
}

void linphone_core_cbs_set_call_state_changed(LinphoneCoreCbs *cbs, LinphoneCoreCbsCallStateChangedCb cb) {
	cbs->vtable->call_state_changed = cb;
}

LinphoneCoreCbsNotifyPresenceReceivedCb linphone_core_cbs_get_notify_presence_received(LinphoneCoreCbs *cbs) {
	return cbs->vtable->notify_presence_received;
}

void linphone_core_cbs_set_notify_presence_received(LinphoneCoreCbs *cbs, LinphoneCoreCbsNotifyPresenceReceivedCb cb) {
	cbs->vtable->notify_presence_received = cb;
}

LinphoneCoreCbsNotifyPresenceReceivedForUriOrTelCb linphone_core_cbs_get_notify_presence_received_for_uri_or_tel(LinphoneCoreCbs *cbs) {
	return cbs->vtable->notify_presence_received_for_uri_or_tel;
}

void linphone_core_cbs_set_notify_presence_received_for_uri_or_tel(LinphoneCoreCbs *cbs, LinphoneCoreCbsNotifyPresenceReceivedForUriOrTelCb cb) {
	cbs->vtable->notify_presence_received_for_uri_or_tel = cb;
}

LinphoneCoreCbsNewSubscriptionRequestedCb linphone_core_cbs_get_new_subscription_requested(LinphoneCoreCbs *cbs) {
	return cbs->vtable->new_subscription_requested;
}

void linphone_core_cbs_set_new_subscription_requested(LinphoneCoreCbs *cbs, LinphoneCoreCbsNewSubscriptionRequestedCb cb) {
	cbs->vtable->new_subscription_requested = cb;
}

LinphoneCoreCbsAuthenticationRequestedCb linphone_core_cbs_get_authentication_requested(LinphoneCoreCbs *cbs) {
	return cbs->vtable->authentication_requested;
}

void linphone_core_cbs_set_authentication_requested(LinphoneCoreCbs *cbs, LinphoneCoreCbsAuthenticationRequestedCb cb) {
	cbs->vtable->authentication_requested = cb;
}

LinphoneCoreCbsCallLogUpdatedCb linphone_core_cbs_get_call_log_updated(LinphoneCoreCbs *cbs) {
	return cbs->vtable->call_log_updated;
}

void linphone_core_cbs_set_call_log_updated(LinphoneCoreCbs *cbs, LinphoneCoreCbsCallLogUpdatedCb cb) {
	cbs->vtable->call_log_updated = cb;
}

LinphoneCoreCbsChatRoomReadCb linphone_core_cbs_get_chat_room_read(LinphoneCoreCbs *cbs) {
	return cbs->vtable->chat_room_read;
}

void linphone_core_cbs_set_chat_room_read(LinphoneCoreCbs *cbs, LinphoneCoreCbsChatRoomReadCb cb) {
	cbs->vtable->chat_room_read = cb;
}

LinphoneCoreCbsMessageReceivedCb linphone_core_cbs_get_message_sent(LinphoneCoreCbs *cbs) {
	return cbs->vtable->message_sent;
}

void linphone_core_cbs_set_message_sent(LinphoneCoreCbs *cbs, LinphoneCoreCbsMessageReceivedCb cb) {
	cbs->vtable->message_sent = cb;
}

LinphoneCoreCbsMessageReceivedCb linphone_core_cbs_get_message_received(LinphoneCoreCbs *cbs) {
	return cbs->vtable->message_received;
}

void linphone_core_cbs_set_message_received(LinphoneCoreCbs *cbs, LinphoneCoreCbsMessageReceivedCb cb) {
	cbs->vtable->message_received = cb;
}

LinphoneCoreCbsMessageReceivedUnableDecryptCb linphone_core_cbs_get_message_received_unable_decrypt(LinphoneCoreCbs *cbs) {
	return cbs->vtable->message_received_unable_decrypt;
}

void linphone_core_cbs_set_message_received_unable_decrypt(LinphoneCoreCbs *cbs, LinphoneCoreCbsMessageReceivedUnableDecryptCb cb) {
	cbs->vtable->message_received_unable_decrypt = cb;
}

LinphoneCoreCbsIsComposingReceivedCb linphone_core_cbs_get_is_composing_received(LinphoneCoreCbs *cbs) {
	return cbs->vtable->is_composing_received;
}

void linphone_core_cbs_set_is_composing_received(LinphoneCoreCbs *cbs, LinphoneCoreCbsIsComposingReceivedCb cb) {
	cbs->vtable->is_composing_received = cb;
}

LinphoneCoreCbsDtmfReceivedCb linphone_core_cbs_get_dtmf_received(LinphoneCoreCbs *cbs) {
	return cbs->vtable->dtmf_received;
}

void linphone_core_cbs_set_dtmf_received(LinphoneCoreCbs *cbs, LinphoneCoreCbsDtmfReceivedCb cb) {
	cbs->vtable->dtmf_received = cb;
}

LinphoneCoreCbsReferReceivedCb linphone_core_cbs_get_refer_received(LinphoneCoreCbs *cbs) {
	return cbs->vtable->refer_received;
}

void linphone_core_cbs_set_refer_received(LinphoneCoreCbs *cbs, LinphoneCoreCbsReferReceivedCb cb) {
	cbs->vtable->refer_received = cb;
}

LinphoneCoreCbsCallEncryptionChangedCb linphone_core_cbs_get_call_encryption_changed(LinphoneCoreCbs *cbs) {
	return cbs->vtable->call_encryption_changed;
}

void linphone_core_cbs_set_call_encryption_changed(LinphoneCoreCbs *cbs, LinphoneCoreCbsCallEncryptionChangedCb cb) {
	cbs->vtable->call_encryption_changed = cb;
}

LinphoneCoreCbsTransferStateChangedCb linphone_core_cbs_get_transfer_state_changed(LinphoneCoreCbs *cbs) {
	return cbs->vtable->transfer_state_changed;
}

void linphone_core_cbs_set_transfer_state_changed(LinphoneCoreCbs *cbs, LinphoneCoreCbsTransferStateChangedCb cb) {
	cbs->vtable->transfer_state_changed = cb;
}

LinphoneCoreCbsBuddyInfoUpdatedCb linphone_core_cbs_get_buddy_info_updated(LinphoneCoreCbs *cbs) {
	return cbs->vtable->buddy_info_updated;
}

void linphone_core_cbs_set_buddy_info_updated(LinphoneCoreCbs *cbs, LinphoneCoreCbsBuddyInfoUpdatedCb cb) {
	cbs->vtable->buddy_info_updated = cb;
}

LinphoneCoreCbsCallStatsUpdatedCb linphone_core_cbs_get_call_stats_updated(LinphoneCoreCbs *cbs) {
	return cbs->vtable->call_stats_updated;
}

void linphone_core_cbs_set_call_stats_updated(LinphoneCoreCbs *cbs, LinphoneCoreCbsCallStatsUpdatedCb cb) {
	cbs->vtable->call_stats_updated = cb;
}

LinphoneCoreCbsInfoReceivedCb linphone_core_cbs_get_info_received(LinphoneCoreCbs *cbs) {
	return cbs->vtable->info_received;
}

void linphone_core_cbs_set_info_received(LinphoneCoreCbs *cbs, LinphoneCoreCbsInfoReceivedCb cb) {
	cbs->vtable->info_received = cb;
}

LinphoneCoreCbsSubscriptionStateChangedCb linphone_core_cbs_get_subscription_state_changed(LinphoneCoreCbs *cbs) {
	return cbs->vtable->subscription_state_changed;
}

void linphone_core_cbs_set_subscription_state_changed(LinphoneCoreCbs *cbs, LinphoneCoreCbsSubscriptionStateChangedCb cb) {
	cbs->vtable->subscription_state_changed = cb;
}

LinphoneCoreCbsNotifyReceivedCb linphone_core_cbs_get_notify_received(LinphoneCoreCbs *cbs) {
	return cbs->vtable->notify_received;
}

void linphone_core_cbs_set_notify_received(LinphoneCoreCbs *cbs, LinphoneCoreCbsNotifyReceivedCb cb) {
	cbs->vtable->notify_received = cb;
}

LinphoneCoreCbsSubscribeReceivedCb linphone_core_cbs_get_subscribe_received(LinphoneCoreCbs *cbs) {
	return cbs->vtable->subscribe_received;
}

void linphone_core_cbs_set_subscribe_received(LinphoneCoreCbs *cbs, LinphoneCoreCbsSubscribeReceivedCb cb) {
	cbs->vtable->subscribe_received = cb;
}

LinphoneCoreCbsPublishStateChangedCb linphone_core_cbs_get_rpublish_state_changed(LinphoneCoreCbs *cbs) {
	return cbs->vtable->publish_state_changed;
}

void linphone_core_cbs_set_publish_state_changed(LinphoneCoreCbs *cbs, LinphoneCoreCbsPublishStateChangedCb cb) {
	cbs->vtable->publish_state_changed = cb;
}

LinphoneCoreCbsConfiguringStatusCb linphone_core_cbs_get_configuring_status(LinphoneCoreCbs *cbs) {
	return cbs->vtable->configuring_status;
}

void linphone_core_cbs_set_configuring_status(LinphoneCoreCbs *cbs, LinphoneCoreCbsConfiguringStatusCb cb) {
	cbs->vtable->configuring_status = cb;
}

LinphoneCoreCbsNetworkReachableCb linphone_core_cbs_get_network_reachable(LinphoneCoreCbs *cbs) {
	return cbs->vtable->network_reachable;
}

void linphone_core_cbs_set_network_reachable(LinphoneCoreCbs *cbs, LinphoneCoreCbsNetworkReachableCb cb) {
	cbs->vtable->network_reachable = cb;
}

LinphoneCoreCbsLogCollectionUploadStateChangedCb linphone_core_cbs_log_collection_upload_state_changed(LinphoneCoreCbs *cbs) {
	return cbs->vtable->log_collection_upload_state_changed;
}

void linphone_core_cbs_set_log_collection_upload_state_changed(LinphoneCoreCbs *cbs, LinphoneCoreCbsLogCollectionUploadStateChangedCb cb) {
	cbs->vtable->log_collection_upload_state_changed = cb;
}

LinphoneCoreCbsLogCollectionUploadProgressIndicationCb linphone_core_cbs_get_rlog_collection_upload_progress_indication(LinphoneCoreCbs *cbs) {
	return cbs->vtable->log_collection_upload_progress_indication;
}

void linphone_core_cbs_set_log_collection_upload_progress_indication(LinphoneCoreCbs *cbs, LinphoneCoreCbsLogCollectionUploadProgressIndicationCb cb) {
	cbs->vtable->log_collection_upload_progress_indication = cb;
}

LinphoneCoreCbsFriendListCreatedCb linphone_core_cbs_get_friend_list_created(LinphoneCoreCbs *cbs) {
	return cbs->vtable->friend_list_created;
}

void linphone_core_cbs_set_friend_list_created(LinphoneCoreCbs *cbs, LinphoneCoreCbsFriendListCreatedCb cb) {
	cbs->vtable->friend_list_created = cb;
}

LinphoneCoreCbsFriendListRemovedCb linphone_core_cbs_get_friend_list_removed(LinphoneCoreCbs *cbs) {
	return cbs->vtable->friend_list_removed;
}

void linphone_core_cbs_set_friend_list_removed(LinphoneCoreCbs *cbs, LinphoneCoreCbsFriendListRemovedCb cb) {
	cbs->vtable->friend_list_removed = cb;
}

LinphoneCoreCbsCallCreatedCb linphone_core_cbs_get_call_created(LinphoneCoreCbs *cbs) {
	return cbs->vtable->call_created;
}

void linphone_core_cbs_set_call_created(LinphoneCoreCbs *cbs, LinphoneCoreCbsCallCreatedCb cb) {
	cbs->vtable->call_created = cb;
}

LinphoneCoreCbsVersionUpdateCheckResultReceivedCb linphone_core_cbs_get_version_update_check_result_received(LinphoneCoreCbs *cbs) {
	return cbs->vtable->version_update_check_result_received;
}

void linphone_core_cbs_set_version_update_check_result_received(LinphoneCoreCbs *cbs, LinphoneCoreCbsVersionUpdateCheckResultReceivedCb cb) {
	cbs->vtable->version_update_check_result_received = cb;
}

LinphoneCoreCbsChatRoomStateChangedCb linphone_core_cbs_get_chat_room_state_changed (LinphoneCoreCbs *cbs) {
	return cbs->vtable->chat_room_state_changed;
}

void linphone_core_cbs_set_chat_room_state_changed (LinphoneCoreCbs *cbs, LinphoneCoreCbsChatRoomStateChangedCb cb) {
	cbs->vtable->chat_room_state_changed = cb;
}

LinphoneCoreCbsQrcodeFoundCb linphone_core_cbs_get_qrcode_found(LinphoneCoreCbs *cbs) {
	return cbs->vtable->qrcode_found;
}

void linphone_core_cbs_set_qrcode_found(LinphoneCoreCbs *cbs, LinphoneCoreCbsQrcodeFoundCb cb) {
	cbs->vtable->qrcode_found = cb;
}

void linphone_core_cbs_set_ec_calibration_result(LinphoneCoreCbs *cbs, LinphoneCoreCbsEcCalibrationResultCb cb) {
	cbs->vtable->ec_calibration_result = cb;
}

void linphone_core_cbs_set_ec_calibration_audio_init(LinphoneCoreCbs *cbs, LinphoneCoreCbsEcCalibrationAudioInitCb cb) {
	cbs->vtable->ec_calibration_audio_init = cb;
}

void linphone_core_cbs_set_ec_calibration_audio_uninit(LinphoneCoreCbs *cbs, LinphoneCoreCbsEcCalibrationAudioUninitCb cb) {
	cbs->vtable->ec_calibration_audio_uninit = cb;
}


void lc_callback_obj_init(LCCallbackObj *obj,LinphoneCoreCbFunc func,void* ud) {
	obj->_func=func;
	obj->_user_data=ud;
}

int lc_callback_obj_invoke(LCCallbackObj *obj, LinphoneCore *lc){
	if (obj->_func!=NULL) obj->_func(lc,obj->_user_data);
	return 0;
}

int linphone_core_get_current_call_duration(const LinphoneCore *lc){
	LinphoneCall *call=linphone_core_get_current_call((LinphoneCore *)lc);
	if (call)  return linphone_call_get_duration(call);
	return -1;
}

const LinphoneAddress *linphone_core_get_current_call_remote_address(struct _LinphoneCore *lc){
	LinphoneCall *call=linphone_core_get_current_call(lc);
	if (call==NULL) return NULL;
	return linphone_call_get_remote_address(call);
}

static void linphone_core_log_collection_handler(const char *domain, OrtpLogLevel level, const char *fmt, va_list args);

void _linphone_core_set_log_handler(OrtpLogFunc logfunc) {
	liblinphone_user_log_func = logfunc;
	if (liblinphone_current_log_func == linphone_core_log_collection_handler) {
		ms_message("There is already a log collection handler, keep it");
	} else {
		bctbx_set_log_handler(liblinphone_current_log_func=liblinphone_user_log_func);
	}
}

void linphone_core_set_log_handler(OrtpLogFunc logfunc){
	_linphone_core_set_log_handler(logfunc);
}

void linphone_core_set_log_file(FILE *file) {
	if (file == NULL) file = stdout;
	_linphone_core_set_log_handler(NULL);
	bctbx_set_log_file(file); /*gather everythings*/
}

void linphone_core_set_log_level(OrtpLogLevel loglevel) {
	LinphoneLoggingService *log_service = linphone_logging_service_get();
	linphone_logging_service_set_log_level(log_service, _bctbx_log_level_to_linphone_log_level(loglevel));
}


void linphone_core_set_log_level_mask(unsigned int mask) {
	LinphoneLoggingService *log_service = linphone_logging_service_get();
	linphone_logging_service_set_log_level_mask(log_service, _bctbx_log_mask_to_linphone_log_mask(mask));
}

unsigned int linphone_core_get_log_level_mask(void) {
	LinphoneLoggingService *log_service = linphone_logging_service_get();
	return linphone_logging_service_get_log_level_mask(log_service);
}

static int _open_log_collection_file_with_idx(int idx) {
	struct stat statbuf;
	char *log_filename;

	log_filename = ortp_strdup_printf("%s/%s%d.log",
		liblinphone_log_collection_path ? liblinphone_log_collection_path : LOG_COLLECTION_DEFAULT_PATH,
		liblinphone_log_collection_prefix ? liblinphone_log_collection_prefix : LOG_COLLECTION_DEFAULT_PREFIX,
		idx);
	liblinphone_log_collection_file = fopen(log_filename, "a");
	ortp_free(log_filename);
	if (liblinphone_log_collection_file == NULL) return -1;

	fstat(fileno(liblinphone_log_collection_file), &statbuf);
	if ((size_t)statbuf.st_size > liblinphone_log_collection_max_file_size) {
		fclose(liblinphone_log_collection_file);
		return -1;
	}

	liblinphone_log_collection_file_size = (size_t)statbuf.st_size;
	return 0;
}

static void _rotate_log_collection_files(void) {
	char *log_filename1;
	char *log_filename2;

	log_filename1 = ortp_strdup_printf("%s/%s1.log",
		liblinphone_log_collection_path ? liblinphone_log_collection_path : LOG_COLLECTION_DEFAULT_PATH,
		liblinphone_log_collection_prefix ? liblinphone_log_collection_prefix : LOG_COLLECTION_DEFAULT_PREFIX);
	log_filename2 = ortp_strdup_printf("%s/%s2.log",
		liblinphone_log_collection_path ? liblinphone_log_collection_path : LOG_COLLECTION_DEFAULT_PATH,
		liblinphone_log_collection_prefix ? liblinphone_log_collection_prefix : LOG_COLLECTION_DEFAULT_PREFIX);
	unlink(log_filename1);
	rename(log_filename2, log_filename1);
	ortp_free(log_filename1);
	ortp_free(log_filename2);
}

static void _open_log_collection_file(void) {
	if (_open_log_collection_file_with_idx(1) < 0) {
		if (_open_log_collection_file_with_idx(2) < 0) {
			_rotate_log_collection_files();
			_open_log_collection_file_with_idx(2);
		}
	}
}

static void _close_log_collection_file(void) {
	if (liblinphone_log_collection_file) {
		fclose(liblinphone_log_collection_file);
		liblinphone_log_collection_file = NULL;
		liblinphone_log_collection_file_size = 0;
	}
}

static void linphone_core_log_collection_handler(const char *domain, OrtpLogLevel level, const char *fmt, va_list args) {
	const char *lname="undef";
	char *msg;
	struct timeval tp;
	struct tm *lt;
	time_t tt;
	int ret;

	if (liblinphone_user_log_func != NULL && liblinphone_user_log_func != linphone_core_log_collection_handler) {
#ifndef _WIN32
		va_list args_copy;
		va_copy(args_copy, args);
		liblinphone_user_log_func(domain, level, fmt, args_copy);
		va_end(args_copy);
#else
		/* This works on 32 bits, luckily. */
		/* TODO: va_copy is available in Visual Studio 2013. */
		liblinphone_user_log_func(domain, level, fmt, args);
#endif
	}

	ortp_gettimeofday(&tp, NULL);
	tt = (time_t)tp.tv_sec;
	lt = localtime((const time_t*)&tt);

	if ((level & ORTP_DEBUG) != 0) {
		lname = "DEBUG";
	} else if ((level & ORTP_MESSAGE) != 0) {
		lname = "MESSAGE";
	} else if ((level & ORTP_WARNING) != 0) {
		lname = "WARNING";
	} else if ((level & ORTP_ERROR) != 0) {
		lname = "ERROR";
	} else if ((level & ORTP_FATAL) != 0) {
		lname = "FATAL";
	} else {
		ortp_fatal("Bad level !");
	}
	msg = ortp_strdup_vprintf(fmt, args);

	if (liblinphone_log_collection_file == NULL) {
		ortp_mutex_lock(&liblinphone_log_collection_mutex);
		_open_log_collection_file();
		ortp_mutex_unlock(&liblinphone_log_collection_mutex);
	}
	if (liblinphone_log_collection_file) {
		ortp_mutex_lock(&liblinphone_log_collection_mutex);
		ret = fprintf(liblinphone_log_collection_file,"%i-%.2i-%.2i %.2i:%.2i:%.2i:%.3i [%s] %s %s\n",
			1900 + lt->tm_year, lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec, (int)(tp.tv_usec / 1000), domain, lname, msg);
		fflush(liblinphone_log_collection_file);
		if (ret > 0) {
			liblinphone_log_collection_file_size += (size_t)ret;
			if (liblinphone_log_collection_file_size > liblinphone_log_collection_max_file_size) {
				_close_log_collection_file();
				_open_log_collection_file();
			}
		}
		ortp_mutex_unlock(&liblinphone_log_collection_mutex);
	}

	ortp_free(msg);
}

const char * linphone_core_get_log_collection_path(void) {
	if (liblinphone_log_collection_path != NULL) {
		return liblinphone_log_collection_path;
	}
	return LOG_COLLECTION_DEFAULT_PATH;
}

void linphone_core_set_log_collection_path(const char *path) {
	if (liblinphone_log_collection_path != NULL) {
		ms_free(liblinphone_log_collection_path);
		liblinphone_log_collection_path = NULL;
	}
	if (path != NULL) {
		liblinphone_log_collection_path = ms_strdup(path);
	}
}

const char * linphone_core_get_log_collection_prefix(void) {
	if (liblinphone_log_collection_prefix != NULL) {
		return liblinphone_log_collection_prefix;
	}
	return LOG_COLLECTION_DEFAULT_PREFIX;
}

void linphone_core_set_log_collection_prefix(const char *prefix) {
	if (liblinphone_log_collection_prefix != NULL) {
		ms_free(liblinphone_log_collection_prefix);
		liblinphone_log_collection_prefix = NULL;
	}
	if (prefix != NULL) {
		liblinphone_log_collection_prefix = ms_strdup(prefix);
	}
}

size_t linphone_core_get_log_collection_max_file_size(void) {
	return liblinphone_log_collection_max_file_size;
}

void linphone_core_set_log_collection_max_file_size(size_t size) {
	liblinphone_log_collection_max_file_size = size;
}

const char *linphone_core_get_log_collection_upload_server_url(LinphoneCore *core) {
	return lp_config_get_string(core->config, "misc", "log_collection_upload_server_url", NULL);
}

void linphone_core_set_log_collection_upload_server_url(LinphoneCore *core, const char *server_url) {
	lp_config_set_string(core->config, "misc", "log_collection_upload_server_url", server_url);
}

LinphoneLogCollectionState linphone_core_log_collection_enabled(void) {
	return liblinphone_log_collection_state;
}

void linphone_core_enable_log_collection(LinphoneLogCollectionState state) {
	if (liblinphone_log_collection_state == state) return;

	liblinphone_log_collection_state = state;
	if (state != LinphoneLogCollectionDisabled) {
		ortp_mutex_init(&liblinphone_log_collection_mutex, NULL);
		if (state == LinphoneLogCollectionEnabledWithoutPreviousLogHandler) {
			liblinphone_user_log_func = NULL; /*remove user log handler*/
		}
		bctbx_set_log_handler(liblinphone_current_log_func = linphone_core_log_collection_handler);
	} else {
		bctbx_set_log_handler(liblinphone_user_log_func); /*restaure */
	}
}

static void clean_log_collection_upload_context(LinphoneCore *lc) {
	char *filename = ms_strdup_printf("%s/%s_log.%s",
		liblinphone_log_collection_path ? liblinphone_log_collection_path : LOG_COLLECTION_DEFAULT_PATH,
		liblinphone_log_collection_prefix ? liblinphone_log_collection_prefix : LOG_COLLECTION_DEFAULT_PREFIX,
		COMPRESSED_LOG_COLLECTION_EXTENSION);
	unlink(filename);
	ms_free(filename);
	if (lc && lc->log_collection_upload_information) {
		linphone_content_unref(lc->log_collection_upload_information);
		lc->log_collection_upload_information=NULL;
	}
}

static void process_io_error_upload_log_collection(void *data, const belle_sip_io_error_event_t *event) {
	LinphoneCore *core = (LinphoneCore *)data;
	ms_error("I/O Error during log collection upload to %s", linphone_core_get_log_collection_upload_server_url(core));
	linphone_core_notify_log_collection_upload_state_changed(core, LinphoneCoreLogCollectionUploadStateNotDelivered, "I/O Error");
	clean_log_collection_upload_context(core);
}

static void process_auth_requested_upload_log_collection(void *data, belle_sip_auth_event_t *event) {
	LinphoneCore *core = (LinphoneCore *)data;
	ms_error("Error during log collection upload: auth requested to connect %s", linphone_core_get_log_collection_upload_server_url(core));
	linphone_core_notify_log_collection_upload_state_changed(core, LinphoneCoreLogCollectionUploadStateNotDelivered, "Auth requested");
	clean_log_collection_upload_context(core);
}

/**
 * Callback called when posting a log collection file to server (following rcs5.1 recommendation)
 * @param[in] bh The body handler
 * @param[in] msg The belle sip message
 * @param[in] data The user data associated with the handler, contains the LinphoneCore object
 * @param[in] offset The current position in the input buffer
 * @param[in] buffer The ouput buffer where to copy the data to be uploaded
 * @param[in,out] size The size in byte of the data requested, as output it will contain the effective copied size
 */
static int log_collection_upload_on_send_body(belle_sip_user_body_handler_t *bh, belle_sip_message_t *msg, void *data, size_t offset, uint8_t *buffer, size_t *size) {
	LinphoneCore *core = (LinphoneCore *)data;

	/* If we've not reach the end of file yet, fill the buffer with more data */
	if (offset < linphone_content_get_size(core->log_collection_upload_information)) {
		char *log_filename = ms_strdup_printf("%s/%s_log.%s",
			liblinphone_log_collection_path ? liblinphone_log_collection_path : LOG_COLLECTION_DEFAULT_PATH,
			liblinphone_log_collection_prefix ? liblinphone_log_collection_prefix : LOG_COLLECTION_DEFAULT_PREFIX,
			COMPRESSED_LOG_COLLECTION_EXTENSION);
#ifdef HAVE_ZLIB
		FILE *log_file = fopen(log_filename, "rb");
#else
		FILE *log_file = fopen(log_filename, "r");
#endif
		if (fseek(log_file, (long)offset, SEEK_SET)) {
			ms_error("Cannot seek file [%s] at position [%lu] errno [%s]",log_filename,(unsigned long)offset,strerror(errno));

		} else {
			*size = fread(buffer, 1, *size, log_file);
		}
		fclose(log_file);
		ms_free(log_filename);
		return BELLE_SIP_CONTINUE;
	} else {
		*size=0;
		return BELLE_SIP_STOP;
	}
}

/**
 * Callback called during upload of a log collection to server.
 * It is just forwarding the call and some parameters to the vtable defined callback.
 */
static void log_collection_upload_on_progress(belle_sip_body_handler_t *bh, belle_sip_message_t *msg, void *data, size_t offset, size_t total) {
	LinphoneCore *core = (LinphoneCore *)data;
	linphone_core_notify_log_collection_upload_progress_indication(core, offset, total);
}

/**
 * Callback function called when we have a response from server during the upload of the log collection to the server (rcs5.1 recommandation)
 * Note: The first post is empty and the server shall reply a 204 (No content) message, this will trigger a new post request to the server
 * to upload the file. The server response to this second post is processed by this same function
 *
 * @param[in] data The user-defined pointer associated with the request, it contains the LinphoneCore object
 * @param[in] event The response from server
 */
static void process_response_from_post_file_log_collection(void *data, const belle_http_response_event_t *event) {
	LinphoneCore *core = (LinphoneCore *)data;

	/* Check the answer code */
	if (event->response) {
		int code = belle_http_response_get_status_code(event->response);
		if (code == 204) { /* This is the reply to the first post to the server - an empty file */
			/* Start uploading the file */
			belle_http_request_listener_callbacks_t cbs = { 0 };
			belle_http_request_listener_t *l;
			belle_generic_uri_t *uri;
			belle_http_request_t *req;
			belle_sip_multipart_body_handler_t *bh;
			char* ua;
			char *first_part_header;
			belle_sip_user_body_handler_t *first_part_bh;

			linphone_core_notify_log_collection_upload_state_changed(core, LinphoneCoreLogCollectionUploadStateInProgress, NULL);

			/* Temporary storage for the Content-disposition header value */
			first_part_header = belle_sip_strdup_printf("form-data; name=\"File\"; filename=\"%s\"", linphone_content_get_name(core->log_collection_upload_information));

			/* Create a user body handler to take care of the file and add the content disposition and content-type headers */
			first_part_bh = belle_sip_user_body_handler_new(linphone_content_get_size(core->log_collection_upload_information), NULL, NULL, NULL, log_collection_upload_on_send_body, NULL, core);
			belle_sip_body_handler_add_header((belle_sip_body_handler_t *)first_part_bh, belle_sip_header_create("Content-disposition", first_part_header));
			belle_sip_free(first_part_header);
			belle_sip_body_handler_add_header((belle_sip_body_handler_t *)first_part_bh,
				(belle_sip_header_t *)belle_sip_header_content_type_create(linphone_content_get_type(core->log_collection_upload_information), linphone_content_get_subtype(core->log_collection_upload_information)));

			/* Insert it in a multipart body handler which will manage the boundaries of multipart message */
			bh = belle_sip_multipart_body_handler_new(log_collection_upload_on_progress, core, (belle_sip_body_handler_t *)first_part_bh, NULL);
			ua = ms_strdup_printf("%s/%s", linphone_core_get_user_agent(core), linphone_core_get_version());
			uri = belle_generic_uri_parse(linphone_core_get_log_collection_upload_server_url(core));
			req = belle_http_request_create("POST", uri, belle_sip_header_create("User-Agent", ua), NULL);
			ms_free(ua);
			belle_sip_message_set_body_handler(BELLE_SIP_MESSAGE(req), BELLE_SIP_BODY_HANDLER(bh));
			cbs.process_response = process_response_from_post_file_log_collection;
			cbs.process_io_error = process_io_error_upload_log_collection;
			cbs.process_auth_requested = process_auth_requested_upload_log_collection;
			l = belle_http_request_listener_create_from_callbacks(&cbs, core);
			belle_sip_object_data_set(BELLE_SIP_OBJECT(req), "http_request_listener", l, belle_sip_object_unref); // Ensure the listener object is destroyed when the request is destroyed
			belle_http_provider_send_request(core->http_provider, req, l);
		} else if (code == 200) { /* The file has been uploaded correctly, get the server reply */
			xmlDocPtr xmlMessageBody;
			xmlNodePtr cur;
			xmlChar *file_url = NULL;
			const char *body = belle_sip_message_get_body((belle_sip_message_t *)event->response);
			xmlMessageBody = xmlParseDoc((const xmlChar *)body);
			cur = xmlDocGetRootElement(xmlMessageBody);
			if (cur != NULL) {
				cur = cur->xmlChildrenNode;
				while (cur != NULL) {
					if (!xmlStrcmp(cur->name, (const xmlChar *)"file-info")) { /* we found a file info node, check it has a type="file" attribute */
						xmlChar *typeAttribute = xmlGetProp(cur, (const xmlChar *)"type");
						if (!xmlStrcmp(typeAttribute, (const xmlChar *)"file")) { /* this is the node we are looking for */
							cur = cur->xmlChildrenNode; /* now loop on the content of the file-info node */
							while (cur != NULL) {
								if (!xmlStrcmp(cur->name, (const xmlChar *)"data")) {
									file_url = xmlGetProp(cur, (const xmlChar *)"url");
								}
								cur=cur->next;
							}
							xmlFree(typeAttribute);
							break;
						}
						xmlFree(typeAttribute);
					}
					cur = cur->next;
				}
			}
			if (file_url != NULL) {
				linphone_core_notify_log_collection_upload_state_changed(core, LinphoneCoreLogCollectionUploadStateDelivered, (const char *)file_url);
				xmlFree(file_url);
			}
			xmlFreeDoc(xmlMessageBody);
			clean_log_collection_upload_context(core);
		} else {
			ms_error("Unexpected HTTP response code %i during log collection upload to %s", code, linphone_core_get_log_collection_upload_server_url(core));
			linphone_core_notify_log_collection_upload_state_changed(core, LinphoneCoreLogCollectionUploadStateNotDelivered, "Unexpected HTTP response");
			clean_log_collection_upload_context(core);
		}
	}
}

#ifdef HAVE_ZLIB
#define COMPRESS_FILE_PTR gzFile
#define COMPRESS_OPEN gzopen
#define COMPRESS_CLOSE gzclose
#else
#define COMPRESS_FILE_PTR FILE*
#define COMPRESS_OPEN fopen
#define COMPRESS_CLOSE fclose
#endif

/**
 * If zlib is not available the two log files are simply concatenated.
 */
static int compress_file(FILE *input_file, COMPRESS_FILE_PTR output_file) {
	char buffer[131072]; /* 128kB */
	size_t bytes;
	size_t total_bytes = 0;

	while ((bytes = fread(buffer, 1, sizeof(buffer), input_file)) > 0) {
#ifdef HAVE_ZLIB
		int res = gzwrite(output_file, buffer, (unsigned int)bytes);
		if (res < 0) return 0;
		total_bytes += (size_t)res;
#else
		total_bytes += fwrite(buffer, 1, bytes, output_file);
#endif
	}
	return (int)total_bytes;
}

static int prepare_log_collection_file_to_upload(const char *filename) {
	char *input_filename = NULL;
	char *output_filename = NULL;
	FILE *input_file = NULL;
	COMPRESS_FILE_PTR output_file = NULL;
	int ret = 0;

	ortp_mutex_lock(&liblinphone_log_collection_mutex);
	output_filename = ms_strdup_printf("%s/%s",
		liblinphone_log_collection_path ? liblinphone_log_collection_path : LOG_COLLECTION_DEFAULT_PATH, filename);
	output_file = COMPRESS_OPEN(output_filename, "wb");
	if (output_file == NULL) goto error;
	input_filename = ms_strdup_printf("%s/%s1.log",
		liblinphone_log_collection_path ? liblinphone_log_collection_path : LOG_COLLECTION_DEFAULT_PATH,
		liblinphone_log_collection_prefix ? liblinphone_log_collection_prefix : LOG_COLLECTION_DEFAULT_PREFIX);
	input_file = fopen(input_filename, "rb");
	if (input_file == NULL) goto error;
	ret = compress_file(input_file, output_file);
	if (ret <= 0) goto error;
	fclose(input_file);
	ms_free(input_filename);
	input_filename = ms_strdup_printf("%s/%s2.log",
		liblinphone_log_collection_path ? liblinphone_log_collection_path : LOG_COLLECTION_DEFAULT_PATH,
		liblinphone_log_collection_prefix ? liblinphone_log_collection_prefix : LOG_COLLECTION_DEFAULT_PREFIX);
	input_file = fopen(input_filename, "rb");
	if (input_file != NULL) {
		ret = compress_file(input_file, output_file);
		if (ret <= 0) goto error;
	}

error:
	if (input_file != NULL) fclose(input_file);
	if (output_file != NULL) COMPRESS_CLOSE(output_file);
	if (input_filename != NULL) ms_free(input_filename);
	if (output_filename != NULL) ms_free(output_filename);
	ortp_mutex_unlock(&liblinphone_log_collection_mutex);
	return ret;
}

static size_t get_size_of_file_to_upload(const char *filename) {
	struct stat statbuf;
	char *output_filename = ms_strdup_printf("%s/%s",
		liblinphone_log_collection_path ? liblinphone_log_collection_path : LOG_COLLECTION_DEFAULT_PATH, filename);
	FILE *output_file = fopen(output_filename, "rb");
	fstat(fileno(output_file), &statbuf);
	fclose(output_file);
	ms_free(output_filename);
	return (size_t)statbuf.st_size;
}

void linphone_core_upload_log_collection(LinphoneCore *core) {
	if ((core->log_collection_upload_information == NULL) && (linphone_core_get_log_collection_upload_server_url(core) != NULL) && (liblinphone_log_collection_state != LinphoneLogCollectionDisabled)) {
		/* open a transaction with the server and send an empty request(RCS5.1 section 3.5.4.8.3.1) */
		belle_http_request_listener_callbacks_t cbs = { 0 };
		belle_http_request_listener_t *l;
		belle_generic_uri_t *uri;
		belle_http_request_t *req;
		char *name;

		core->log_collection_upload_information = linphone_core_create_content(core);
#ifdef HAVE_ZLIB
		linphone_content_set_type(core->log_collection_upload_information, "application");
		linphone_content_set_subtype(core->log_collection_upload_information, "gzip");
#else
		linphone_content_set_type(core->log_collection_upload_information, "text");
		linphone_content_set_subtype(core->log_collection_upload_information,"plain");
#endif
		name = ms_strdup_printf("%s_log.%s",
			liblinphone_log_collection_prefix ? liblinphone_log_collection_prefix : LOG_COLLECTION_DEFAULT_PREFIX,
			COMPRESSED_LOG_COLLECTION_EXTENSION);
		linphone_content_set_name(core->log_collection_upload_information, name);
		if (prepare_log_collection_file_to_upload(name) <= 0) {
			linphone_content_unref(core->log_collection_upload_information);
			core->log_collection_upload_information = NULL;
			ms_error("prepare_log_collection_file_to_upload(): error.");
			linphone_core_notify_log_collection_upload_state_changed(core, LinphoneCoreLogCollectionUploadStateNotDelivered, "Error while preparing log collection upload");
			return;
		}
		linphone_content_set_size(core->log_collection_upload_information, get_size_of_file_to_upload(name));
		uri = belle_generic_uri_parse(linphone_core_get_log_collection_upload_server_url(core));
		req = belle_http_request_create("POST", uri, NULL, NULL, NULL);
		cbs.process_response = process_response_from_post_file_log_collection;
		cbs.process_io_error = process_io_error_upload_log_collection;
		cbs.process_auth_requested = process_auth_requested_upload_log_collection;
		l = belle_http_request_listener_create_from_callbacks(&cbs, core);
		belle_sip_object_data_set(BELLE_SIP_OBJECT(req), "http_request_listener", l, belle_sip_object_unref); // Ensure the listener object is destroyed when the request is destroyed
		belle_http_provider_send_request(core->http_provider, req, l);
		ms_free(name);
	} else {
		const char *msg = NULL;
		ms_warning("Could not upload log collection: log_collection_upload_information=%p, server_url=%s, log_collection_state=%d",
			core->log_collection_upload_information, linphone_core_get_log_collection_upload_server_url(core), liblinphone_log_collection_state);
		if (core->log_collection_upload_information != NULL) {
			msg = "Log collection upload already in progress";
		} else if (linphone_core_get_log_collection_upload_server_url(core) == NULL) {
			msg = "Log collection upload server not set";
		} else if (liblinphone_log_collection_state == LinphoneLogCollectionDisabled) {
			msg = "Log collection is disabled";
		}
		linphone_core_notify_log_collection_upload_state_changed(core, LinphoneCoreLogCollectionUploadStateNotDelivered, msg);
	}
}

char * linphone_core_compress_log_collection(void) {
	char *filename = NULL;
	if (liblinphone_log_collection_state == LinphoneLogCollectionDisabled) return NULL;
	filename = ms_strdup_printf("%s_log.%s",
		liblinphone_log_collection_prefix ? liblinphone_log_collection_prefix : LOG_COLLECTION_DEFAULT_PREFIX,
		COMPRESSED_LOG_COLLECTION_EXTENSION);
	if (prepare_log_collection_file_to_upload(filename) <= 0) {
		ms_free(filename);
		return NULL;
	}
	ms_free(filename);
	return ms_strdup_printf("%s/%s_log.%s",
		liblinphone_log_collection_path ? liblinphone_log_collection_path : LOG_COLLECTION_DEFAULT_PATH,
		liblinphone_log_collection_prefix ? liblinphone_log_collection_prefix : LOG_COLLECTION_DEFAULT_PREFIX,
		COMPRESSED_LOG_COLLECTION_EXTENSION);
}

void linphone_core_reset_log_collection(void) {
	char *filename;
	ortp_mutex_lock(&liblinphone_log_collection_mutex);
	_close_log_collection_file();
	clean_log_collection_upload_context(NULL);
	filename = ms_strdup_printf("%s/%s1.log",
			liblinphone_log_collection_path ? liblinphone_log_collection_path : LOG_COLLECTION_DEFAULT_PATH,
			liblinphone_log_collection_prefix ? liblinphone_log_collection_prefix : LOG_COLLECTION_DEFAULT_PREFIX);
	unlink(filename);
	ms_free(filename);
	filename = ms_strdup_printf("%s/%s2.log",
		liblinphone_log_collection_path ? liblinphone_log_collection_path : LOG_COLLECTION_DEFAULT_PATH,
		liblinphone_log_collection_prefix ? liblinphone_log_collection_prefix : LOG_COLLECTION_DEFAULT_PREFIX);
	unlink(filename);
	ms_free(filename);
	liblinphone_log_collection_file = NULL;
	liblinphone_log_collection_file_size = 0;
	liblinphone_log_collection_max_file_size = LOG_COLLECTION_DEFAULT_MAX_FILE_SIZE; /*also reset size*/
	ortp_mutex_unlock(&liblinphone_log_collection_mutex);
}

void linphone_core_enable_logs(FILE *file){
	if (file==NULL) file=stdout;
	linphone_core_set_log_file(file);
	linphone_core_set_log_level(ORTP_MESSAGE);
}

void linphone_core_enable_logs_with_cb(OrtpLogFunc logfunc){
	_linphone_core_set_log_handler(logfunc);
	linphone_core_set_log_level(ORTP_MESSAGE);
}

void linphone_core_disable_logs(void){
	linphone_core_set_log_level(ORTP_ERROR);
}

void linphone_core_serialize_logs(void) {
	liblinphone_serialize_logs = TRUE;
}


static void net_config_read(LinphoneCore *lc) {
	int tmp;
	const char *tmpstr;
	LpConfig *config=lc->config;
	const char *nat_policy_ref;

	nat_policy_ref = lp_config_get_string(lc->config, "net", "nat_policy_ref", NULL);
	if (nat_policy_ref != NULL) {
		LinphoneNatPolicy *nat_policy = linphone_core_create_nat_policy_from_config(lc, nat_policy_ref);
		linphone_core_set_nat_policy(lc, nat_policy);
		linphone_nat_policy_unref(nat_policy);
	}
	if (lc->nat_policy == NULL){
		/*this will create a default nat policy according to deprecated config keys, or an empty nat policy otherwise*/
		linphone_core_set_firewall_policy(lc, linphone_core_get_firewall_policy(lc));
	}

	lc->net_conf.nat_address_ip = NULL;
	tmp=lp_config_get_int(config,"net","download_bw",0);
	linphone_core_set_download_bandwidth(lc,tmp);
	tmp=lp_config_get_int(config,"net","upload_bw",0);
	linphone_core_set_upload_bandwidth(lc,tmp);
	tmp=lp_config_get_int(config, "net", "expected_bw", 0);
	linphone_core_set_expected_bandwidth(lc, tmp);

	tmpstr=lp_config_get_string(lc->config,"net","nat_address",NULL);
	if (tmpstr!=NULL && (strlen(tmpstr)<1)) tmpstr=NULL;
	linphone_core_set_nat_address(lc,tmpstr);
	tmp=lp_config_get_int(lc->config,"net","nat_sdp_only",0);
	lc->net_conf.nat_sdp_only=!!tmp;
	tmp=lp_config_get_int(lc->config,"net","mtu",1300);
	linphone_core_set_mtu(lc,tmp);
	tmp=lp_config_get_int(lc->config,"net","download_ptime",-1);
	if (tmp !=-1 && linphone_core_get_download_ptime(lc) !=0) {
		/*legacy parameter*/
		linphone_core_set_download_ptime(lc,tmp);
	}
	tmp = lp_config_get_int(lc->config, "net", "dns_srv_enabled", 1);
	linphone_core_enable_dns_srv(lc, !!tmp);
	tmp = lp_config_get_int(lc->config, "net", "dns_search_enabled", 1);
	linphone_core_enable_dns_search(lc, !!tmp);
}

static void build_sound_devices_table(LinphoneCore *lc){
	const char **devices;
	const char **old;
	size_t ndev;
	int i;
	const bctbx_list_t *elem=ms_snd_card_manager_get_list(ms_factory_get_snd_card_manager(lc->factory));
	ndev=bctbx_list_size(elem);
	devices=reinterpret_cast<const char **>(ms_malloc((ndev+1)*sizeof(const char *)));
	for (i=0;elem!=NULL;elem=elem->next,i++){
		devices[i]=ms_snd_card_get_string_id((MSSndCard *)elem->data);
	}
	devices[ndev]=NULL;
	old=lc->sound_conf.cards;
	lc->sound_conf.cards=devices;
	if (old!=NULL) ms_free((void *)old);
}

static string get_default_local_ring(LinphoneCore * lc) {
	if (linphone_core_file_format_supported(lc, "mkv")) {
		return static_cast<PlatformHelpers *>(lc->platform_helper)->getRingResource(LOCAL_RING_MKV);
	}
	return static_cast<PlatformHelpers *>(lc->platform_helper)->getRingResource(LOCAL_RING_WAV);
}

static string get_default_onhold_music(LinphoneCore * lc) {
	if (linphone_core_file_format_supported(lc, "mkv")) {
		return static_cast<PlatformHelpers *>(lc->platform_helper)->getSoundResource(HOLD_MUSIC_MKV);
	}
	return static_cast<PlatformHelpers *>(lc->platform_helper)->getSoundResource(HOLD_MUSIC_WAV);
}

static void sound_config_read(LinphoneCore *lc) {
	int tmp;
	const char *tmpbuf;
	const char *devid;

#ifdef __linux
	/*alsadev let the user use custom alsa device within linphone*/
	devid=lp_config_get_string(lc->config,"sound","alsadev",NULL);
	if (devid){
		MSSndCard* card;
		const char* delim=",";
		size_t l=strlen(devid);
		char* d=reinterpret_cast<char *>(malloc(l+1));
		char* i;
		memcpy(d,devid,l+1);
		for (l=0,i=strpbrk(d+l,delim);i;i=strpbrk(d+l,delim)){
			char s=*i;
			*i='\0';
			card=ms_alsa_card_new_custom(d+l,d+l);
			ms_snd_card_manager_add_card(ms_factory_get_snd_card_manager(lc->factory),card);
			*i=s;
			l=(size_t)(i-d)+1;
		}
		if(d[l]!='\0') {
			card=ms_alsa_card_new_custom(d+l,d+l);
			ms_snd_card_manager_add_card(ms_factory_get_snd_card_manager(lc->factory),card);
		}
		free(d);
	}
	tmp=lp_config_get_int(lc->config,"sound","alsa_forced_rate",-1);
	if (tmp>0) ms_alsa_card_set_forced_sample_rate(tmp);
#endif
	/* retrieve all sound devices */
	build_sound_devices_table(lc);

	devid=lp_config_get_string(lc->config,"sound","playback_dev_id",NULL);
	linphone_core_set_playback_device(lc,devid);

	devid=lp_config_get_string(lc->config,"sound","ringer_dev_id",NULL);
	linphone_core_set_ringer_device(lc,devid);

	devid=lp_config_get_string(lc->config,"sound","capture_dev_id",NULL);
	linphone_core_set_capture_device(lc,devid);

	devid=lp_config_get_string(lc->config,"sound","media_dev_id",NULL);
	linphone_core_set_media_device(lc,devid);

/*
	tmp=lp_config_get_int(lc->config,"sound","play_lev",80);
	linphone_core_set_play_level(lc,tmp);
	tmp=lp_config_get_int(lc->config,"sound","ring_lev",80);
	linphone_core_set_ring_level(lc,tmp);
	tmp=lp_config_get_int(lc->config,"sound","rec_lev",80);
	linphone_core_set_rec_level(lc,tmp);
	tmpbuf=lp_config_get_string(lc->config,"sound","source","m");
	linphone_core_set_sound_source(lc,tmpbuf[0]);
*/

	tmpbuf = lp_config_get_string(lc->config, "sound", "local_ring", NULL);
	if (tmpbuf) {
		if (bctbx_file_exist(tmpbuf) == 0) {
			linphone_core_set_ring(lc, tmpbuf);
		} else {
			ms_warning("'%s' ring file does not exist", tmpbuf);
		}
	} else {
		linphone_core_set_ring(lc, get_default_local_ring(lc).c_str());
	}

	string defaultRemoteRing = static_cast<PlatformHelpers *>(lc->platform_helper)->getSoundResource(REMOTE_RING_WAV);
	tmpbuf = lp_config_get_string(lc->config, "sound", "remote_ring", defaultRemoteRing.c_str());
	if (bctbx_file_exist(tmpbuf) == -1){
		tmpbuf = defaultRemoteRing.c_str();
	}
	if (strstr(tmpbuf, ".wav") == NULL) {
		/* It currently uses old sound files, so replace them */
		tmpbuf = defaultRemoteRing.c_str();
	}
	linphone_core_set_ringback(lc, tmpbuf);

	tmpbuf = lp_config_get_string(lc->config, "sound", "hold_music", NULL);
	if (tmpbuf) {
		if (bctbx_file_exist(tmpbuf) == 0) {
			linphone_core_set_play_file(lc, tmpbuf);
		} else {
			ms_warning("'%s' on-hold music file does not exist", tmpbuf);
		}
	} else {
		linphone_core_set_play_file(lc, get_default_onhold_music(lc).c_str());
	}

	lc->sound_conf.latency=0;
#if !TARGET_OS_IPHONE
	tmp=TRUE;
#else
	tmp=FALSE; /* on iOS we have builtin echo cancellation.*/
#endif
	tmp=lp_config_get_int(lc->config,"sound","echocancellation",tmp);
	linphone_core_enable_echo_cancellation(lc, !!tmp);
	linphone_core_set_echo_canceller_filter_name(lc, linphone_core_get_echo_canceller_filter_name(lc));
	linphone_core_enable_echo_limiter(lc, !!lp_config_get_int(lc->config,"sound","echolimiter",0));
	linphone_core_enable_agc(lc, !!lp_config_get_int(lc->config,"sound","agc",0));

	linphone_core_set_playback_gain_db (lc,lp_config_get_float(lc->config,"sound","playback_gain_db",0));
	linphone_core_set_mic_gain_db (lc,lp_config_get_float(lc->config,"sound","mic_gain_db",0));

	linphone_core_set_remote_ringback_tone (lc,lp_config_get_string(lc->config,"sound","ringback_tone",NULL));

	/*just parse requested stream feature once at start to print out eventual errors*/
	linphone_core_get_audio_features(lc);

	_linphone_core_set_tone(lc,LinphoneReasonBusy,LinphoneToneBusy,NULL);
}

static int _linphone_core_tls_postcheck_callback(void *data, const bctbx_x509_certificate_t *peer_cert){
	LinphoneCore *lc = (LinphoneCore *) data;
	const char *tls_certificate_subject_regexp = lp_config_get_string(lc->config,"sip","tls_certificate_subject_regexp", NULL);
	int ret = 0;
	if (tls_certificate_subject_regexp){
		ret = -1;
		/*the purpose of this handling is to a peer certificate for which there is no single subject matching the regexp given
		 * in the "tls_certificate_subject_regexp" property.
		 */
		bctbx_list_t *subjects = bctbx_x509_certificate_get_subjects(peer_cert);
		bctbx_list_t *elem;
		for(elem = subjects; elem != NULL; elem = elem->next){
			const char *subject = (const char *)elem->data;
			ms_message("_linphone_core_tls_postcheck_callback: subject=%s", subject);
			if (bctbx_is_matching_regex(subject, tls_certificate_subject_regexp)){
				ret = 0;
				ms_message("_linphone_core_tls_postcheck_callback(): successful by matching '%s'", subject);
				break;
			}
		}
		bctbx_list_free_with_data(subjects, bctbx_free);
	}
	if (ret == -1){
		ms_message("_linphone_core_tls_postcheck_callback(): postcheck failed, nothing matched.");
	}
	return ret;
}

static void certificates_config_read(LinphoneCore *lc) {
	string rootCaPath = static_cast<PlatformHelpers *>(lc->platform_helper)->getDataResource("rootca.pem");
	const char *rootca = lp_config_get_string(lc->config, "sip", "root_ca", nullptr);

	// If rootca is not existing anymore, we try data_resources_dir/rootca.pem else default from belle-sip
	if (!rootca || ((bctbx_file_exist(rootca) != 0) && !bctbx_directory_exists(rootca))) {
		// Check root_ca_path
		if ((bctbx_file_exist(rootCaPath.c_str()) == 0) || bctbx_directory_exists(rootCaPath.c_str()))
			rootca = rootCaPath.c_str();
		else
			rootca = nullptr;
	}

	if (rootca)
		linphone_core_set_root_ca(lc, rootca);
	// else use default value from belle-sip

	linphone_core_verify_server_certificates(lc, !!lp_config_get_int(lc->config, "sip", "verify_server_certs", TRUE));
	linphone_core_verify_server_cn(lc, !!lp_config_get_int(lc->config, "sip", "verify_server_cn", TRUE));

	lc->sal->setTlsPostcheckCallback(_linphone_core_tls_postcheck_callback, lc);
}

static void bodyless_config_read(LinphoneCore *lc) {
	// Clean previous friend lists
	linphone_core_clear_bodyless_friend_lists(lc);

	bctbx_list_t *bodyless_lists = linphone_config_get_string_list(lc->config, "sip", "bodyless_lists", NULL);
	while (bodyless_lists) {
		char *name = (char *)bctbx_list_get_data(bodyless_lists);
		bodyless_lists = bctbx_list_next(bodyless_lists);
		LinphoneAddress *addr = linphone_address_new(name);
		if(!addr) {
			bctbx_free(name);
			continue;
		}

		ms_message("Found bodyless friendlist %s", name);
		bctbx_free(name);
		LinphoneFriendList *friendList = linphone_core_create_friend_list(lc);
		linphone_friend_list_set_rls_address(friendList, addr);
		linphone_friend_list_set_display_name(
			friendList,
			linphone_address_get_display_name(addr)
				? linphone_address_get_display_name(addr)
				: linphone_address_get_username(addr)
		);
		linphone_address_unref(addr);
		linphone_friend_list_set_subscription_bodyless(friendList, TRUE);
		linphone_core_add_friend_list(lc, friendList);
		linphone_friend_list_unref(friendList);
	}
}

void linphone_core_invalidate_friends_maps(LinphoneCore *lc) {
	bctbx_list_t *elem;
	for (elem = lc->friends_lists; elem != NULL; elem = bctbx_list_next(elem)) {
		LinphoneFriendList *list = (LinphoneFriendList *)bctbx_list_get_data(elem);
		ms_message("Invalidating friends maps for list [%p]", list);
		linphone_friend_list_invalidate_friends_maps(list);
	}
}

static void sip_config_read(LinphoneCore *lc) {
	char *contact;
	const char *tmpstr;
	LinphoneSipTransports tr;
	int i,tmp;
	int ipv6_default = TRUE;

	if (lp_config_get_int(lc->config,"sip","use_session_timers",0)==1){
		lc->sal->useSessionTimers(200);
	}

	lc->sal->useNoInitialRoute(!!lp_config_get_int(lc->config,"sip","use_no_initial_route",0));
	lc->sal->useRport(!!lp_config_get_int(lc->config,"sip","use_rport",1));

	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->setSpecs(linphone_config_get_string(lc->config, "sip", "linphone_specs", ""));

	if (!lp_config_get_int(lc->config,"sip","ipv6_migration_done",FALSE) && lp_config_has_entry(lc->config,"sip","use_ipv6")) {
		lp_config_clean_entry(lc->config,"sip","use_ipv6");
		lp_config_set_int(lc->config, "sip", "ipv6_migration_done", TRUE);
		ms_message("IPV6 settings migration done.");
	}

	lc->sip_conf.ipv6_enabled = !!lp_config_get_int(lc->config,"sip","use_ipv6",ipv6_default);

	memset(&tr,0,sizeof(tr));

	tr.udp_port=lp_config_get_int(lc->config,"sip","sip_port",5060);
	tr.tcp_port=lp_config_get_int(lc->config,"sip","sip_tcp_port",5060);
	/*we are not listening inbound connection for tls, port has no meaning*/
	tr.tls_port=lp_config_get_int(lc->config,"sip","sip_tls_port",LC_SIP_TRANSPORT_RANDOM);

	certificates_config_read(lc);
	/*setting the dscp must be done before starting the transports, otherwise it is not taken into effect*/
	lc->sal->setDscp(linphone_core_get_sip_dscp(lc));
	/*start listening on ports*/
	linphone_core_set_sip_transports(lc,&tr);

	tmpstr=lp_config_get_string(lc->config,"sip","contact",NULL);
	if (tmpstr==NULL || linphone_core_set_primary_contact(lc,tmpstr)==-1) {
		const char *hostname=NULL;
		const char *username=NULL;
#if !defined(LINPHONE_WINDOWS_UNIVERSAL) && !defined(LINPHONE_WINDOWS_PHONE) // Using getenv is forbidden on Windows 10 and Windows Phone
		hostname=getenv("HOST");

		#if defined _WIN32
			username = getenv("USERNAME");
		#else
			username = getenv("USER");
		#endif // if defined _WIN32
		if (hostname==NULL) hostname=getenv("HOSTNAME");
#endif
		if (hostname==NULL)
			hostname="unknown-host";
		if (username==NULL){
			username="linphone";
		}
		contact=ortp_strdup_printf("sip:%s@%s",username,hostname);
		linphone_core_set_primary_contact(lc,contact);
		ms_free(contact);
	}

	tmp=lp_config_get_int(lc->config,"sip","guess_hostname",1);
	linphone_core_set_guess_hostname(lc, !!tmp);

	if (linphone_core_lime_x3dh_available(lc)) {
		//Always try to enable x3dh. Will actually be enabled only if there is a server url configured
		linphone_core_enable_lime_x3dh(lc, true);
	}

	tmp=lp_config_get_int(lc->config,"sip", "lime", LinphoneLimeDisabled);
	LinphoneLimeState limeState = static_cast<LinphoneLimeState>(tmp);
	if (limeState != LinphoneLimeDisabled && linphone_core_lime_x3dh_enabled(lc)) {
		bctbx_fatal("You can't have both LIME and LIME X3DH enabled at the same time !\nConflicting settings are [sip] lime and [lime] lime_server_url");
	}
	linphone_core_enable_lime(lc, limeState);

	tmp=lp_config_get_int(lc->config,"sip","inc_timeout",30);
	linphone_core_set_inc_timeout(lc,tmp);

	tmp=lp_config_get_int(lc->config,"sip","in_call_timeout",0);
	linphone_core_set_in_call_timeout(lc,tmp);

	tmp=lp_config_get_int(lc->config,"sip","delayed_timeout",4);
	linphone_core_set_delayed_timeout(lc,tmp);

	tmp=lp_config_get_int(lc->config,"app","auto_download_incoming_files_max_size",-1);
	linphone_core_set_max_size_for_auto_download_incoming_files(lc, tmp);

	/*In case of remote provisionning, function sip_config_read is initialy called in core_init, then in state ConfiguringSuccessfull*/
	/*Accordingly, to avoid proxy_config to be added twice, it is mandatory to reset proxy config list from LinphoneCore*/
	/*We assume, lc->config contains an accurate list of proxy_config, so no need to keep it from LinphoneCore */
	/*Consequence in case of remote provisionning, linphone_core_add_proxy function should not be called before state GlobalOn*/
	linphone_core_clear_proxy_config(lc);

	/* get proxies config */
	for(i=0;; i++){
		LinphoneProxyConfig *cfg=linphone_proxy_config_new_from_config_file(lc,i);
		if (cfg!=NULL){
			linphone_core_add_proxy_config(lc,cfg);
			linphone_proxy_config_unref(cfg);
		}else{
			break;
		}
	}
	/* get the default proxy */
	tmp=lp_config_get_int(lc->config,"sip","default_proxy",-1);
	linphone_core_set_default_proxy_index(lc,tmp);

	/* In case of remote provisioning, recompute the phone numbers in case the dial prefix of the default proxy config has changed */
	linphone_core_invalidate_friends_maps(lc);

	/* read authentication information */
	for(i=0;; i++){
		LinphoneAuthInfo *ai=linphone_auth_info_new_from_config_file(lc->config,i);
		if (ai!=NULL){
			linphone_core_add_auth_info(lc,ai);
			linphone_auth_info_unref(ai);
		}else{
			break;
		}
	}
	/*this is to filter out unsupported encryption schemes*/
	linphone_core_set_media_encryption(lc,linphone_core_get_media_encryption(lc));

	/*enable the reconnection to the primary server when it is up again asap*/
	lc->sal->enableReconnectToPrimaryAsap(!!lp_config_get_int(lc->config,"sip","reconnect_to_primary_asap",0));

	/*for tuning or test*/
	lc->sip_conf.sdp_200_ack = !!lp_config_get_int(lc->config,"sip","sdp_200_ack",0);
	lc->sip_conf.register_only_when_network_is_up=
		!!lp_config_get_int(lc->config,"sip","register_only_when_network_is_up",1);
	lc->sip_conf.register_only_when_upnp_is_ok=
		!!lp_config_get_int(lc->config,"sip","register_only_when_upnp_is_ok",1);
	lc->sip_conf.ping_with_options= !!lp_config_get_int(lc->config,"sip","ping_with_options",0);
	lc->sip_conf.auto_net_state_mon = !!lp_config_get_int(lc->config,"sip","auto_net_state_mon",1);
	lc->sip_conf.keepalive_period = (unsigned int)lp_config_get_int(lc->config,"sip","keepalive_period",10000);
	lc->sip_conf.tcp_tls_keepalive = !!lp_config_get_int(lc->config,"sip","tcp_tls_keepalive",0);
	linphone_core_enable_keep_alive(lc, (lc->sip_conf.keepalive_period > 0));
	lc->sal->useOneMatchingCodecPolicy(!!lp_config_get_int(lc->config,"sip","only_one_codec",0));
	lc->sal->useDates(!!lp_config_get_int(lc->config,"sip","put_date",0));
	lc->sal->enableSipUpdateMethod(!!lp_config_get_int(lc->config,"sip","sip_update",1));
	lc->sip_conf.vfu_with_info = !!lp_config_get_int(lc->config,"sip","vfu_with_info",1);
	linphone_core_set_sip_transport_timeout(lc, lp_config_get_int(lc->config, "sip", "transport_timeout", 63000));
	lc->sal->setSupportedTags(lp_config_get_string(lc->config,"sip","supported","replaces, outbound, gruu"));
	lc->sip_conf.save_auth_info = !!lp_config_get_int(lc->config, "sip", "save_auth_info", 1);
	linphone_core_create_im_notif_policy(lc);

	bodyless_config_read(lc);
}

static void rtp_config_read(LinphoneCore *lc) {
	int min_port, max_port;
	int jitt_comp;
	int nortp_timeout;
	bool_t rtp_no_xmit_on_audio_mute;
	bool_t adaptive_jitt_comp_enabled;
	const char* tmp;
	int tmp_int;

	if (lp_config_get_range(lc->config, "rtp", "audio_rtp_port", &min_port, &max_port, 7078, 7078) == TRUE) {
		if (min_port <= 0) min_port = 1;
		if (max_port > 65535) max_port = 65535;
		linphone_core_set_audio_port_range(lc, min_port, max_port);
	} else {
		min_port = lp_config_get_int(lc->config, "rtp", "audio_rtp_port", 7078);
		linphone_core_set_audio_port(lc, min_port);
	}

	if (lp_config_get_range(lc->config, "rtp", "video_rtp_port", &min_port, &max_port, 9078, 9078) == TRUE) {
		if (min_port <= 0) min_port = 1;
		if (max_port > 65535) max_port = 65535;
		linphone_core_set_video_port_range(lc, min_port, max_port);
	} else {
		min_port = lp_config_get_int(lc->config, "rtp", "video_rtp_port", 9078);
		linphone_core_set_video_port(lc, min_port);
	}

	if (lp_config_get_range(lc->config, "rtp", "text_rtp_port", &min_port, &max_port, 11078, 11078) == TRUE) {
		if (min_port <= 0) min_port = 1;
		if (max_port > 65535) max_port = 65535;
		linphone_core_set_text_port_range(lc, min_port, max_port);
	} else {
		min_port = lp_config_get_int(lc->config, "rtp", "text_rtp_port", 11078);
		linphone_core_set_text_port(lc, min_port);
	}

	jitt_comp=lp_config_get_int(lc->config,"rtp","audio_jitt_comp",60);
	linphone_core_set_audio_jittcomp(lc,jitt_comp);
	jitt_comp=lp_config_get_int(lc->config,"rtp","video_jitt_comp",60);
	if (jitt_comp==0) jitt_comp=60;
	linphone_core_set_video_jittcomp(lc,jitt_comp);
	nortp_timeout=lp_config_get_int(lc->config,"rtp","nortp_timeout",30);
	linphone_core_set_nortp_timeout(lc,nortp_timeout);
	rtp_no_xmit_on_audio_mute = !!lp_config_get_int(lc->config,"rtp","rtp_no_xmit_on_audio_mute",FALSE);
	linphone_core_set_rtp_no_xmit_on_audio_mute(lc,rtp_no_xmit_on_audio_mute);
	adaptive_jitt_comp_enabled = !!lp_config_get_int(lc->config, "rtp", "audio_adaptive_jitt_comp_enabled", TRUE);
	linphone_core_enable_audio_adaptive_jittcomp(lc, adaptive_jitt_comp_enabled);
	adaptive_jitt_comp_enabled = !!lp_config_get_int(lc->config, "rtp", "video_adaptive_jitt_comp_enabled", TRUE);
	linphone_core_enable_video_adaptive_jittcomp(lc, adaptive_jitt_comp_enabled);
	lc->rtp_conf.disable_upnp = lp_config_get_int(lc->config, "rtp", "disable_upnp", FALSE);
	linphone_core_set_avpf_mode(lc,static_cast<LinphoneAVPFMode>(lp_config_get_int(lc->config,"rtp","avpf",LinphoneAVPFDisabled)));
	if ((tmp=lp_config_get_string(lc->config,"rtp","audio_multicast_addr",NULL)))
		linphone_core_set_audio_multicast_addr(lc,tmp);
	else {
		if (lc->rtp_conf.audio_multicast_addr) bctbx_free(lc->rtp_conf.audio_multicast_addr);
		lc->rtp_conf.audio_multicast_addr=ms_strdup("224.1.2.3");
	}
	if ((tmp_int=lp_config_get_int(lc->config,"rtp","audio_multicast_enabled",-1)) >-1)
		linphone_core_enable_audio_multicast(lc, !!tmp_int);
	if ((tmp_int=lp_config_get_int(lc->config,"rtp","audio_multicast_ttl",-1))>0)
			linphone_core_set_audio_multicast_ttl(lc,tmp_int);
	else
		lc->rtp_conf.audio_multicast_ttl=1;/*local network*/
	if ((tmp=lp_config_get_string(lc->config,"rtp","video_multicast_addr",NULL)))
		linphone_core_set_video_multicast_addr(lc,tmp);
	else {
		if (lc->rtp_conf.video_multicast_addr) bctbx_free(lc->rtp_conf.video_multicast_addr);
		lc->rtp_conf.video_multicast_addr=ms_strdup("224.1.2.3");
	}
	if ((tmp_int=lp_config_get_int(lc->config,"rtp","video_multicast_ttl",-1))>-1)
		linphone_core_set_video_multicast_ttl(lc,tmp_int);
	else
		lc->rtp_conf.video_multicast_ttl=1;/*local network*/
	if ((tmp_int=lp_config_get_int(lc->config,"rtp","video_multicast_enabled",-1)) >0)
		linphone_core_enable_video_multicast(lc, !!tmp_int);
}

static PayloadType * find_payload(const bctbx_list_t *default_list, const char *mime_type, int clock_rate, int channels, const char *recv_fmtp){
	PayloadType *candidate=NULL;
	PayloadType *it;
	const bctbx_list_t *elem;

	for(elem=default_list;elem!=NULL;elem=elem->next){
		it=(PayloadType*)elem->data;
		if (it!=NULL && strcasecmp(mime_type,it->mime_type)==0
			&& (clock_rate==it->clock_rate || clock_rate<=0)
			&& (channels==it->channels || channels<=0) ){
			if ( (recv_fmtp && it->recv_fmtp && strstr(recv_fmtp,it->recv_fmtp)!=NULL) ||
				(recv_fmtp==NULL && it->recv_fmtp==NULL) ){
				/*exact match*/
				if (recv_fmtp) payload_type_set_recv_fmtp(it,recv_fmtp);
				return it;
			}else {
				if (candidate){
					if (it->recv_fmtp==NULL) candidate=it;
				}else candidate=it;
			}
		}
	}
	if (candidate && recv_fmtp){
		payload_type_set_recv_fmtp(candidate,recv_fmtp);
	}
	return candidate;
}

static PayloadType* find_payload_type_from_list(const char* type, int rate, int channels, const bctbx_list_t* from) {
	const bctbx_list_t *elem;
	for(elem=from;elem!=NULL;elem=elem->next){
		PayloadType *pt=(PayloadType*)elem->data;
		if ((strcasecmp(type, payload_type_get_mime(pt)) == 0)
			&& (rate == LINPHONE_FIND_PAYLOAD_IGNORE_RATE || rate==pt->clock_rate)
			&& (channels == LINPHONE_FIND_PAYLOAD_IGNORE_CHANNELS || channels==pt->channels)) {
			return pt;
		}
	}
	return NULL;
}

static bool_t linphone_core_codec_supported(LinphoneCore *lc, SalStreamType type, const char *mime){
	if (type == SalVideo && lp_config_get_int(lc->config, "video", "rtp_io", FALSE)){
		return TRUE; /*in rtp io mode, we don't transcode video, thus we can support a format for which we have no encoder nor decoder.*/
	} else if (type == SalAudio && lp_config_get_int(lc->config, "sound", "rtp_io", FALSE)){
		return TRUE; /*in rtp io mode, we don't transcode video, thus we can support a format for which we have no encoder nor decoder.*/
	} else if (type == SalText) {
		return TRUE;
	}
	if (lc->video_conf.capture && !lc->video_conf.display) {
		return ms_factory_has_encoder(lc->factory, mime);
	}
	if (lc->video_conf.display && !lc->video_conf.capture) {
		return ms_factory_has_decoder(lc->factory, mime);
	}
	return ms_factory_codec_supported(lc->factory, mime);
}


static bool_t get_codec(LinphoneCore *lc, SalStreamType type, int index, PayloadType **ret){
	char codeckey[50];
	const char *mime,*fmtp;
	int rate,channels,enabled;
	PayloadType *pt;
	LpConfig *config=lc->config;

	*ret=NULL;
	snprintf(codeckey,50,"%s_codec_%i",type == SalAudio ? "audio" : type == SalVideo ? "video" : "text", index);
	mime=lp_config_get_string(config,codeckey,"mime",NULL);
	if (mime==NULL || strlen(mime)==0 ) return FALSE;

	rate=lp_config_get_int(config,codeckey,"rate",8000);
	fmtp=lp_config_get_string(config,codeckey,"recv_fmtp",NULL);
	channels=lp_config_get_int(config,codeckey,"channels",0);
	enabled=lp_config_get_int(config,codeckey,"enabled",1);
	if (!linphone_core_codec_supported(lc, type, mime)){
		ms_warning("Codec %s/%i read from conf is not supported by mediastreamer2, ignored.",mime,rate);
		return TRUE;
	}
	pt = find_payload(type == SalAudio ? lc->default_audio_codecs : type == SalVideo ? lc->default_video_codecs : lc->default_text_codecs ,mime,rate,channels,fmtp);
	if (!pt){
		bctbx_list_t **default_list = (type==SalAudio) ? &lc->default_audio_codecs : type == SalVideo ? &lc->default_video_codecs : &lc->default_text_codecs;
		if (type == SalAudio)
			ms_warning("Codec %s/%i/%i read from conf is not in the default list.",mime,rate,channels);
		else if (type == SalVideo)
			ms_warning("Codec %s/%i read from conf is not in the default list.",mime,rate);
		else
			ms_warning("Codec %s read from conf is not in the default list.",mime);
		pt=payload_type_new();
		pt->type=(type==SalAudio) ? PAYLOAD_AUDIO_PACKETIZED : type == SalVideo ? PAYLOAD_VIDEO : PAYLOAD_TEXT;
		pt->mime_type=ortp_strdup(mime);
		pt->clock_rate=rate;
		pt->channels=channels;
		payload_type_set_number(pt,-1); /*dynamic assignment*/
		payload_type_set_recv_fmtp(pt,fmtp);
		*default_list=bctbx_list_append(*default_list, pt);
	}
	if (enabled)
		payload_type_set_enable(pt, TRUE);
	else
		payload_type_set_enable(pt, FALSE);
	*ret=pt;
	return TRUE;
}

static SalStreamType payload_type_get_stream_type(const PayloadType *pt){
	switch(pt->type){
		case PAYLOAD_AUDIO_PACKETIZED:
		case PAYLOAD_AUDIO_CONTINUOUS:
			return SalAudio;
		break;
		case PAYLOAD_VIDEO:
			return SalVideo;
		break;
		case PAYLOAD_TEXT:
			return SalText;
		break;
	}
	return SalOther;
}

/*this function merges the payload types from the codec default list with the list read from configuration file.
 * If a new codec becomes supported in Liblinphone or if the list from configuration file is empty or incomplete, all the supported codecs are added
 * automatically. This 'l' list is entirely rewritten.*/
static bctbx_list_t *add_missing_supported_codecs(LinphoneCore *lc, const bctbx_list_t *default_list, bctbx_list_t *l){
	const bctbx_list_t *elem;
	PayloadType *last_seen = NULL;

	for(elem=default_list; elem!=NULL; elem=elem->next){
		bctbx_list_t *elem2=bctbx_list_find(l,elem->data);
		if (!elem2){
			PayloadType *pt=(PayloadType*)elem->data;
			/*this codec from default list should be inserted in the list, with respect to the default_list order*/

			if (!linphone_core_codec_supported(lc, payload_type_get_stream_type(pt), pt->mime_type)) continue;
			if (!last_seen){
				l=bctbx_list_prepend(l,pt);
			}else{
				const bctbx_list_t *after=bctbx_list_find(l,last_seen);
				l=bctbx_list_insert(l, after->next, pt);
			}
			last_seen = pt;
			ms_message("Supported codec %s/%i fmtp=%s automatically added to codec list.", pt->mime_type,
					 pt->clock_rate, pt->recv_fmtp ? pt->recv_fmtp : "");
		}else{
			last_seen = (PayloadType*)elem2->data;
		}
	}
	return l;
}

/*
 * This function adds missing codecs, if required by configuration.
 * This 'l' list is entirely rewritten if required.
 */
static bctbx_list_t *handle_missing_codecs (LinphoneCore *lc, const bctbx_list_t *default_list, bctbx_list_t *l, MSFormatType ft) {
	const char *name = "unknown";

	switch (ft) {
		case MSAudio:
			name = "add_missing_audio_codecs";
			break;
		case MSVideo:
			name = "add_missing_video_codecs";
			break;
		case MSText:
			name = "add_missing_text_codecs";
			break;
		case MSUnknownMedia:
		break;
	}

	if (lp_config_get_int(lc->config, "misc", name, 1))
		return add_missing_supported_codecs(lc, default_list, l);
	return l;
}

static bctbx_list_t *codec_append_if_new(bctbx_list_t *l, PayloadType *pt){
	bctbx_list_t *elem;
	for (elem=l;elem!=NULL;elem=elem->next){
		PayloadType *ept=(PayloadType*)elem->data;
		if (pt==ept)
			return l;
	}
	l=bctbx_list_append(l,pt);
	return l;
}

static void codecs_config_read(LinphoneCore *lc){
	int i;
	PayloadType *pt;
	bctbx_list_t *audio_codecs=NULL;
	bctbx_list_t *video_codecs=NULL;
	bctbx_list_t *text_codecs=NULL;

	lc->codecs_conf.dyn_pt=96;
	lc->codecs_conf.telephone_event_pt=lp_config_get_int(lc->config,"misc","telephone_event_pt",101);

	for (i=0;get_codec(lc,SalAudio,i,&pt);i++){
		if (pt){
			audio_codecs=codec_append_if_new(audio_codecs, pt);
		}
	}
	audio_codecs = handle_missing_codecs(lc, lc->default_audio_codecs,audio_codecs, MSAudio);

	for (i=0;get_codec(lc,SalVideo,i,&pt);i++){
		if (pt){
			video_codecs=codec_append_if_new(video_codecs, pt);
		}
	}

	video_codecs = handle_missing_codecs(lc, lc->default_video_codecs, video_codecs, MSVideo);

	for (i=0;get_codec(lc,SalText,i,&pt);i++){
		if (pt){
			text_codecs=codec_append_if_new(text_codecs, pt);
		}
	}
	text_codecs = add_missing_supported_codecs(lc, lc->default_text_codecs, text_codecs);

	linphone_core_set_audio_codecs(lc,audio_codecs);
	linphone_core_set_video_codecs(lc,video_codecs);
	linphone_core_set_text_codecs(lc, text_codecs);
	linphone_core_update_allocated_audio_bandwidth(lc);
}

static void build_video_devices_table(LinphoneCore *lc){
	const bctbx_list_t *elem;
	int i;
	size_t ndev;
	const char **devices;
	if (lc->video_conf.cams)
		ms_free((void *)lc->video_conf.cams);
	/* retrieve all video devices */
	elem=ms_web_cam_manager_get_list(ms_factory_get_web_cam_manager(lc->factory));
	ndev=bctbx_list_size(elem);
	devices=reinterpret_cast<const char **>(ms_malloc((ndev+1)*sizeof(const char *)));
	for (i=0;elem!=NULL;elem=elem->next,i++){
		devices[i]=ms_web_cam_get_string_id((MSWebCam *)elem->data);
	}
	devices[ndev]=NULL;
	lc->video_conf.cams=devices;
}

static void video_config_read(LinphoneCore *lc){
#ifdef VIDEO_ENABLED
	int automatic_video=1;
	const char *str;
	LinphoneVideoPolicy vpol;
	memset(&vpol, 0, sizeof(LinphoneVideoPolicy));
	build_video_devices_table(lc);

	str=lp_config_get_string(lc->config,"video","device",NULL);
	if (str && str[0]==0) str=NULL;
	linphone_core_set_video_device(lc,str);

	linphone_core_set_preferred_video_size_by_name(lc,
		lp_config_get_string(lc->config,"video","size","vga"));

	linphone_core_set_preview_video_size_by_name(lc,
		lp_config_get_string(lc->config,"video","preview_size",NULL));

	linphone_core_set_preferred_framerate(lc,lp_config_get_float(lc->config,"video","framerate",0));

#if defined(__ANDROID__) || TARGET_OS_IPHONE
	automatic_video=0;
#endif
	vpol.automatically_initiate = !!lp_config_get_int(lc->config,"video","automatically_initiate",automatic_video);
	vpol.automatically_accept = !!lp_config_get_int(lc->config,"video","automatically_accept",automatic_video);
	linphone_core_enable_video_capture(lc, !!lp_config_get_int(lc->config,"video","capture",1));
	linphone_core_enable_video_display(lc, !!lp_config_get_int(lc->config,"video","display",1));
	linphone_core_enable_video_preview(lc, !!lp_config_get_int(lc->config,"video","show_local",0));
	linphone_core_enable_self_view(lc, !!lp_config_get_int(lc->config,"video","self_view",1));
	linphone_core_enable_video_source_reuse(lc, !!lp_config_get_int(lc->config,"video","reuse_source",0));
	linphone_core_set_video_policy(lc,&vpol);

	lc->video_conf.retransmission_on_nack_enabled = !!lp_config_get_int(lc->config,"video","retransmission_on_nack_enabled",0);
#endif
}

static void read_friends_from_rc(LinphoneCore *lc)
{
	LinphoneFriend *lf = NULL;
	int i;
	for (i = 0; (lf = linphone_friend_new_from_config_file(lc, i)) != NULL; i++) {
		linphone_core_add_friend(lc, lf);
		linphone_friend_unref(lf);
	}
}

static void ui_config_read(LinphoneCore *lc) {
	if (!lc->friends_db) {
		read_friends_from_rc(lc);
	}
	if (!lc->logs_db) {
		lc->call_logs = linphone_core_read_call_logs_from_config_file(lc);
	}
}

bool_t linphone_core_tunnel_available(void){
#ifdef TUNNEL_ENABLED
	return TRUE;
#else
	return FALSE;
#endif
}

void linphone_core_enable_adaptive_rate_control(LinphoneCore *lc, bool_t enabled){
	lp_config_set_int(lc->config,"net","adaptive_rate_control",(int)enabled);
}

bool_t linphone_core_adaptive_rate_control_enabled(const LinphoneCore *lc){
	return !!lp_config_get_int(lc->config,"net","adaptive_rate_control",TRUE);
}

void linphone_core_set_adaptive_rate_algorithm(LinphoneCore *lc, const char* algorithm){
	if (strcasecmp(algorithm, "basic") != 0 && strcasecmp(algorithm, "advanced") != 0) {
		ms_warning("Unsupported adaptive rate algorithm [%s] on core [%p], using advanced",algorithm,lc);
		linphone_core_set_adaptive_rate_algorithm(lc, "advanced");
		return;
	}
	lp_config_set_string(lc->config,"net","adaptive_rate_algorithm",algorithm);
}

const char * linphone_core_get_adaptive_rate_algorithm(const LinphoneCore *lc){
	const char* saved_value = lp_config_get_string(lc->config, "net", "adaptive_rate_algorithm", "advanced");
	if (strcasecmp(saved_value, "basic") != 0 && strcasecmp(saved_value, "advanced") != 0) {
		ms_warning("Unsupported adaptive rate algorithm [%s] on core [%p]",saved_value,lc);
	}
	return saved_value;
}

bool_t linphone_core_rtcp_enabled(const LinphoneCore *lc){
	return !!lp_config_get_int(lc->config,"rtp","rtcp_enabled",TRUE);
}

void linphone_core_set_download_bandwidth(LinphoneCore *lc, int bw){
	lc->net_conf.download_bw=bw;
	linphone_core_update_allocated_audio_bandwidth(lc);
	if (linphone_core_ready(lc)) lp_config_set_int(lc->config,"net","download_bw",bw);
}

void linphone_core_set_upload_bandwidth(LinphoneCore *lc, int bw){
	lc->net_conf.upload_bw=bw;
	linphone_core_update_allocated_audio_bandwidth(lc);
	if (linphone_core_ready(lc)) lp_config_set_int(lc->config,"net","upload_bw",bw);
}

void linphone_core_set_expected_bandwidth(LinphoneCore *lc, int bw){
	ms_factory_set_expected_bandwidth(lc->factory, bw * 1000); // In linphone we use kbits/s, in ms2 bits/s
	if (linphone_core_ready(lc)) lp_config_set_int(lc->config,"net","expected_bw",bw);
}

void linphone_core_set_sip_transport_timeout(LinphoneCore *lc, int timeout_ms) {
	lc->sal->setTransportTimeout(timeout_ms);
	if (linphone_core_ready(lc))
		lp_config_set_int(lc->config, "sip", "transport_timeout", timeout_ms);
}

int linphone_core_get_sip_transport_timeout(LinphoneCore *lc) {
	return lc->sal->getTransportTimeout();
}

bool_t linphone_core_get_dns_set_by_app(LinphoneCore *lc) {
	return lc->dns_set_by_app;
}

void linphone_core_set_dns_servers_app(LinphoneCore *lc, const bctbx_list_t *servers){
	lc->dns_set_by_app = (servers != NULL);
	linphone_core_set_dns_servers(lc, servers);
}

void linphone_core_set_dns_servers(LinphoneCore *lc, const bctbx_list_t *servers){
	lc->sal->setDnsServers(servers);
}

void linphone_core_enable_dns_srv(LinphoneCore *lc, bool_t enable) {
	lc->sal->enableDnsSrv(!!enable);
	if (linphone_core_ready(lc))
		lp_config_set_int(lc->config, "net", "dns_srv_enabled", enable ? 1 : 0);
}

bool_t linphone_core_dns_srv_enabled(const LinphoneCore *lc) {
	return lc->sal->dnsSrvEnabled();
}

void linphone_core_enable_dns_search(LinphoneCore *lc, bool_t enable) {
	lc->sal->enableDnsSearch(!!enable);
	if (linphone_core_ready(lc))
		lp_config_set_int(lc->config, "net", "dns_search_enabled", enable ? 1 : 0);
}

bool_t linphone_core_dns_search_enabled(const LinphoneCore *lc) {
	return lc->sal->dnsSearchEnabled();
}

int linphone_core_get_download_bandwidth(const LinphoneCore *lc){
	return lc->net_conf.download_bw;
}

int linphone_core_get_upload_bandwidth(const LinphoneCore *lc){
	return lc->net_conf.upload_bw;
}

void linphone_core_set_download_ptime(LinphoneCore *lc, int ptime) {
	lp_config_set_int(lc->config,"rtp","download_ptime",ptime);
}

int linphone_core_get_download_ptime(LinphoneCore *lc) {
	return lp_config_get_int(lc->config,"rtp","download_ptime",0);
}

void linphone_core_set_upload_ptime(LinphoneCore *lc, int ptime){
	lp_config_set_int(lc->config,"rtp","upload_ptime",ptime);
}

int linphone_core_get_upload_ptime(LinphoneCore *lc){
	return lp_config_get_int(lc->config,"rtp","upload_ptime",0);
}

const char * linphone_core_get_version(void){
	return liblinphone_version;
}

static void linphone_core_register_payload_type(LinphoneCore *lc, const PayloadType *const_pt, const char *recv_fmtp, bool_t enabled){
	bctbx_list_t **codec_list = const_pt->type==PAYLOAD_VIDEO ? &lc->default_video_codecs : const_pt->type==PAYLOAD_TEXT ? &lc->default_text_codecs : &lc->default_audio_codecs;
	PayloadType *pt=payload_type_clone(const_pt);
	int number=-1;
	payload_type_set_enable(pt,enabled);
	if (recv_fmtp!=NULL) payload_type_set_recv_fmtp(pt,recv_fmtp);
	/*Set a number to the payload type from the statically defined (RFC3551) profile, if not static, -1 is returned
		and the payload type number will be determined dynamically later, at call time.*/
	payload_type_set_number(pt,
		(number=rtp_profile_find_payload_number(&av_profile, pt->mime_type, pt->clock_rate, pt->channels))
	);
	ms_message("Codec %s/%i fmtp=[%s] number=%i, default enablement: %i) added to the list of possible codecs.", pt->mime_type, pt->clock_rate,
			pt->recv_fmtp ? pt->recv_fmtp : "", number, (int)payload_type_enabled(pt));
	*codec_list=bctbx_list_append(*codec_list,pt);
}

static void linphone_core_register_static_payloads(LinphoneCore *lc){
	RtpProfile *prof=&av_profile;
	int i;
	for(i=0;i<RTP_PROFILE_MAX_PAYLOADS;++i){
		PayloadType *pt=rtp_profile_get_payload(prof,i);
		if (pt){
#ifndef VIDEO_ENABLED
			if (pt->type==PAYLOAD_VIDEO) continue;
#endif
			if (find_payload_type_from_list(
				pt->mime_type, pt->clock_rate, pt->type == PAYLOAD_VIDEO || pt->type == PAYLOAD_TEXT ? LINPHONE_FIND_PAYLOAD_IGNORE_CHANNELS : pt->channels,
				pt->type == PAYLOAD_VIDEO ? lc->default_video_codecs : pt->type == PAYLOAD_TEXT ? lc->default_text_codecs : lc->default_audio_codecs)==NULL){
				linphone_core_register_payload_type(lc,pt,NULL,FALSE);
			}
		}
	}
}

static void linphone_core_free_payload_types(LinphoneCore *lc){
	bctbx_list_free_with_data(lc->default_audio_codecs, (void (*)(void*))payload_type_destroy);
	bctbx_list_free_with_data(lc->default_video_codecs, (void (*)(void*))payload_type_destroy);
	bctbx_list_free_with_data(lc->default_text_codecs, (void (*)(void*))payload_type_destroy);
	lc->default_audio_codecs = NULL;
	lc->default_video_codecs = NULL;
	lc->default_text_codecs = NULL;
}

void linphone_core_set_state(LinphoneCore *lc, LinphoneGlobalState gstate, const char *message){
	lc->state=gstate;
	linphone_core_notify_global_state_changed(lc,gstate,message);
}

static void misc_config_read (LinphoneCore *lc) {
	LpConfig *config = lc->config;

	lc->max_call_logs = lp_config_get_int(config,"misc","history_max_size",LINPHONE_MAX_CALL_HISTORY_SIZE);
	lc->max_calls = lp_config_get_int(config,"misc","max_calls",NB_MAX_CALLS);

	if (lc->user_certificates_path) bctbx_free(lc->user_certificates_path);
		lc->user_certificates_path = bctbx_strdup(lp_config_get_string(config, "misc", "user_certificates_path", "."));

	lc->send_call_stats_periodical_updates = !!lp_config_get_int(config, "misc", "send_call_stats_periodical_updates", 0);
}

void linphone_core_reload_ms_plugins(LinphoneCore *lc, const char *path){
	if (path) ms_factory_set_plugins_dir(lc->factory, path);
	ms_factory_init_plugins(lc->factory);
	codecs_config_read(lc);
}

static void _linphone_core_read_config(LinphoneCore * lc) {
	sip_setup_register_all(lc->factory);
	sound_config_read(lc);
	net_config_read(lc);
	rtp_config_read(lc);
	codecs_config_read(lc);
	sip_config_read(lc);
	video_config_read(lc);
	//autoreplier_config_init(&lc->autoreplier_conf);
	misc_config_read(lc);
	ui_config_read(lc);
#ifdef TUNNEL_ENABLED
	if (lc->tunnel) {
		linphone_tunnel_configure(lc->tunnel);
	}
#endif

	lc->auto_net_state_mon=lc->sip_conf.auto_net_state_mon;
}

void linphone_core_load_config_from_xml(LinphoneCore *lc, const char * xml_uri) {
	// As for today assume the URI is a local file
	const char *error = linphone_config_load_from_xml_file(lc->config, xml_uri);
	if (error) {
		bctbx_error("Couldn't load config from xml: %s", error);
		return;
	}

	// We should call _linphone_core_read_config(lc); in the future but currently this is dangerous
	if (linphone_core_lime_x3dh_available(lc)) {
		linphone_core_enable_lime_x3dh(lc, true);
	}
}

void linphone_configuring_terminated(LinphoneCore *lc, LinphoneConfiguringState state, const char *message) {
	linphone_core_notify_configuring_status(lc, state, message);

	if (state == LinphoneConfiguringSuccessful) {
		if (linphone_core_is_provisioning_transient(lc) == TRUE)
			linphone_core_set_provisioning_uri(lc, NULL);
		_linphone_core_read_config(lc);
	}
	if (lc->provisioning_http_listener){
		belle_sip_object_unref(lc->provisioning_http_listener);
		lc->provisioning_http_listener = NULL;
	}

	linphone_core_set_state(lc,LinphoneGlobalOn,"Ready");
}


static int linphone_core_serialization_ref = 0;

static void linphone_core_activate_log_serialization_if_needed(void) {
	if (liblinphone_serialize_logs == TRUE) {
		linphone_core_serialization_ref++;
		if (linphone_core_serialization_ref == 1)
			ortp_set_log_thread_id(ortp_thread_self());
	}
}

static void linphone_core_deactivate_log_serialization_if_needed(void) {
	if (liblinphone_serialize_logs == TRUE) {
		--linphone_core_serialization_ref;
		if (linphone_core_serialization_ref == 0)
			ortp_set_log_thread_id(0);
	}
}

static void linphone_core_register_default_codecs(LinphoneCore *lc){
	const char *aac_fmtp162248, *aac_fmtp3244;
	bool_t opus_enabled=TRUE;
	/*default enabled audio codecs, in order of preference*/
#if defined(__arm__) || defined(_M_ARM)
	/*hack for opus, that needs to be disabed by default on ARM single processor, otherwise there is no cpu left for video processing*/
	//if (ms_get_cpu_count()==1) opus_enabled=FALSE;
	if (ms_factory_get_cpu_count(lc->factory)==1) opus_enabled=FALSE;
#endif
	linphone_core_register_payload_type(lc,&payload_type_opus,"useinbandfec=1",opus_enabled);
	linphone_core_register_payload_type(lc,&payload_type_silk_wb,NULL,TRUE);
	linphone_core_register_payload_type(lc,&payload_type_speex_wb,"vbr=on",TRUE);
	linphone_core_register_payload_type(lc,&payload_type_speex_nb,"vbr=on",TRUE);
	linphone_core_register_payload_type(lc,&payload_type_pcmu8000,NULL,TRUE);
	linphone_core_register_payload_type(lc,&payload_type_pcma8000,NULL,TRUE);

	/* Text codecs in order or preference (RED first (more robust), then T140) */
	linphone_core_register_payload_type(lc, &payload_type_t140_red, NULL, TRUE);
	linphone_core_register_payload_type(lc, &payload_type_t140, NULL, TRUE);

	/*other audio codecs, not enabled by default, in order of preference*/
	linphone_core_register_payload_type(lc,&payload_type_gsm,NULL,FALSE);
	linphone_core_register_payload_type(lc,&payload_type_g722,NULL,FALSE);
	linphone_core_register_payload_type(lc,&payload_type_ilbc,"mode=30",FALSE);
	linphone_core_register_payload_type(lc,&payload_type_amr,"octet-align=1",FALSE);
	linphone_core_register_payload_type(lc,&payload_type_amrwb,"octet-align=1",FALSE);
	linphone_core_register_payload_type(lc,&payload_type_g729,"annexb=yes",TRUE);
	/* For AAC, we use a config value to determine if we ought to support SBR. Since it is not offically supported
	 * for the mpeg4-generic mime type, setting this flag to 1 will break compatibility with other clients. */
	if( lp_config_get_int(lc->config, "misc", "aac_use_sbr", FALSE) ) {
		ms_message("Using SBR for AAC");
		aac_fmtp162248 = "config=F8EE2000; constantDuration=512; indexDeltaLength=3; indexLength=3; mode=AAC-hbr; profile-level-id=76; sizeLength=13; streamType=5; SBR-enabled=1";
		aac_fmtp3244   = "config=F8E82000; constantDuration=512; indexDeltaLength=3; indexLength=3; mode=AAC-hbr; profile-level-id=76; sizeLength=13; streamType=5; SBR-enabled=1";
	} else {
		aac_fmtp162248 = "config=F8EE2000; constantDuration=512; indexDeltaLength=3; indexLength=3; mode=AAC-hbr; profile-level-id=76; sizeLength=13; streamType=5";
		aac_fmtp3244   = "config=F8E82000; constantDuration=512; indexDeltaLength=3; indexLength=3; mode=AAC-hbr; profile-level-id=76; sizeLength=13; streamType=5";
	}
	linphone_core_register_payload_type(lc,&payload_type_aaceld_16k,aac_fmtp162248,FALSE);
	linphone_core_register_payload_type(lc,&payload_type_aaceld_22k,aac_fmtp162248,FALSE);
	linphone_core_register_payload_type(lc,&payload_type_aaceld_32k,aac_fmtp3244,FALSE);
	linphone_core_register_payload_type(lc,&payload_type_aaceld_44k,aac_fmtp3244,FALSE);
	linphone_core_register_payload_type(lc,&payload_type_aaceld_48k,aac_fmtp162248,FALSE);
	linphone_core_register_payload_type(lc,&payload_type_isac,NULL,FALSE);
	linphone_core_register_payload_type(lc,&payload_type_speex_uwb,"vbr=on",FALSE);
	linphone_core_register_payload_type(lc,&payload_type_silk_nb,NULL,FALSE);
	linphone_core_register_payload_type(lc,&payload_type_silk_mb,NULL,FALSE);
	linphone_core_register_payload_type(lc,&payload_type_silk_swb,NULL,FALSE);
	linphone_core_register_payload_type(lc,&payload_type_g726_16,NULL,FALSE);
	linphone_core_register_payload_type(lc,&payload_type_g726_24,NULL,FALSE);
	linphone_core_register_payload_type(lc,&payload_type_g726_32,NULL,FALSE);
	linphone_core_register_payload_type(lc,&payload_type_g726_40,NULL,FALSE);
	linphone_core_register_payload_type(lc,&payload_type_aal2_g726_16,NULL,FALSE);
	linphone_core_register_payload_type(lc,&payload_type_aal2_g726_24,NULL,FALSE);
	linphone_core_register_payload_type(lc,&payload_type_aal2_g726_32,NULL,FALSE);
	linphone_core_register_payload_type(lc,&payload_type_aal2_g726_40,NULL,FALSE);
	linphone_core_register_payload_type(lc,&payload_type_codec2,NULL,FALSE);
	linphone_core_register_payload_type(lc,&payload_type_bv16,NULL,FALSE);


#ifdef VIDEO_ENABLED
	/*default enabled video codecs, in order of preference*/
	linphone_core_register_payload_type(lc,&payload_type_vp8,NULL,TRUE);
	linphone_core_register_payload_type(lc,&payload_type_h264,"profile-level-id=42801F",TRUE);
	linphone_core_register_payload_type(lc,&payload_type_h265,NULL,TRUE);
	linphone_core_register_payload_type(lc,&payload_type_mp4v,"profile-level-id=3",TRUE);
	linphone_core_register_payload_type(lc,&payload_type_h263_1998,"CIF=1;QCIF=1",FALSE);
	linphone_core_register_payload_type(lc,&payload_type_h263,NULL,FALSE);
#endif
	/*register all static payload types declared in av_profile of oRTP, if not already declared above*/
	linphone_core_register_static_payloads(lc);
}

static void linphone_core_internal_notify_received(LinphoneCore *lc, LinphoneEvent *lev, const char *notified_event, const LinphoneContent *body) {
	if (strcmp(notified_event, "Presence") == 0) {
		for (const bctbx_list_t *it = linphone_core_get_friends_lists(lc); it; it = bctbx_list_next(it)) {
			LinphoneFriendList *list = reinterpret_cast<LinphoneFriendList *>(bctbx_list_get_data(it));
			if (list->event != lev) continue;

			ms_message("Notify presence for list %p", list);
			linphone_friend_list_notify_presence_received(list, lev, body);
		}
	} else if (strcmp(notified_event, "conference") == 0) {
#ifdef HAVE_ADVANCED_IM
		const LinphoneAddress *resource = linphone_event_get_resource(lev);
		char *resourceAddrStr = linphone_address_as_string_uri_only(resource);
		const char *factoryUri = linphone_proxy_config_get_conference_factory_uri(linphone_core_get_default_proxy_config(lc));
		if (factoryUri && (strcmp(resourceAddrStr, factoryUri) == 0)) {
			bctbx_free(resourceAddrStr);
			L_GET_PRIVATE_FROM_C_OBJECT(lc)->remoteListEventHandler->notifyReceived(L_GET_CPP_PTR_FROM_C_OBJECT(body));
			return;
		}
		bctbx_free(resourceAddrStr);

		const LinphoneAddress *from = linphone_event_get_from(lev);
		shared_ptr<AbstractChatRoom> chatRoom = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->findChatRoom(LinphonePrivate::ConferenceId(
			IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(resource)),
			IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(from))
		));
		if (!chatRoom)
			return;

		shared_ptr<ClientGroupChatRoom> cgcr;
		if (chatRoom->getCapabilities() & ChatRoom::Capabilities::Proxy)
			cgcr = static_pointer_cast<ClientGroupChatRoom>(
				static_pointer_cast<ClientGroupToBasicChatRoom>(chatRoom)->getProxiedChatRoom());
		else
			cgcr = static_pointer_cast<ClientGroupChatRoom>(chatRoom);

		if (linphone_content_is_multipart(body)) {
			// TODO : migrate to c++ 'Content'.
			int i = 0;
			LinphoneContent *part = nullptr;
			while ((part = linphone_content_get_part(body, i))) {
				i++;
				L_GET_PRIVATE(cgcr)->notifyReceived(linphone_content_get_string_buffer(part));
				linphone_content_unref(part);
			}
		} else
			L_GET_PRIVATE(cgcr)->notifyReceived(linphone_content_get_string_buffer(body));
#else
		ms_message("Advanced IM such as group chat is disabled!");
#endif
	}
}

static void _linphone_core_conference_subscribe_received(LinphoneCore *lc, LinphoneEvent *lev, const LinphoneContent *body) {
#ifdef HAVE_ADVANCED_IM
	if (body && linphone_event_get_custom_header(lev, "Content-Disposition") && strcasecmp(linphone_event_get_custom_header(lev, "Content-Disposition"), "recipient-list") == 0) {
		// List subscription
		L_GET_PRIVATE_FROM_C_OBJECT(lc)->localListEventHandler->subscribeReceived(lev, body);
		return;
	}

	const LinphoneAddress *resource = linphone_event_get_resource(lev);
	shared_ptr<AbstractChatRoom> chatRoom = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->findChatRoom(LinphonePrivate::ConferenceId(
		IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(resource)),
		IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(resource))
	));
	if (chatRoom)
		L_GET_PRIVATE(static_pointer_cast<ServerGroupChatRoom>(chatRoom))->subscribeReceived(lev);
	else
		linphone_event_deny_subscription(lev, LinphoneReasonDeclined);
#else
	ms_warning("Advanced IM such as group chat is disabled!");
#endif
}

static void linphone_core_internal_subscribe_received(LinphoneCore *lc, LinphoneEvent *lev, const char *subscribe_event, const LinphoneContent *body) {
	if (strcmp(linphone_event_get_name(lev), "conference") == 0) {
		_linphone_core_conference_subscribe_received(lc, lev, body);
	}
}

static void _linphone_core_conference_subscription_state_changed (LinphoneCore *lc, LinphoneEvent *lev, LinphoneSubscriptionState state) {
#ifdef HAVE_ADVANCED_IM
	if (!linphone_core_conference_server_enabled(lc)) {
		RemoteConferenceEventHandlerPrivate *thiz = static_cast<RemoteConferenceEventHandlerPrivate *>(linphone_event_get_user_data(lev));
		if (state == LinphoneSubscriptionError)
			thiz->invalidateSubscription();

		return;
	}

	const LinphoneAddress *resource = linphone_event_get_resource(lev);
	shared_ptr<AbstractChatRoom> chatRoom = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->findChatRoom(LinphonePrivate::ConferenceId(
		IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(resource)),
		IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(resource))
	));
	if (chatRoom)
		L_GET_PRIVATE(static_pointer_cast<ServerGroupChatRoom>(chatRoom))->subscriptionStateChanged(lev, state);
#else
	ms_warning("Advanced IM such as group chat is disabled!");
#endif
}

static void linphone_core_internal_subscription_state_changed(LinphoneCore *lc, LinphoneEvent *lev, LinphoneSubscriptionState state) {
	if (strcasecmp(linphone_event_get_name(lev), "Presence") == 0) {
		linphone_friend_list_subscription_state_changed(lc, lev, state);
	} else if (strcmp(linphone_event_get_name(lev), "conference") == 0) {
		_linphone_core_conference_subscription_state_changed(lc, lev, state);
	}
}

static void linphone_core_internal_publish_state_changed(LinphoneCore *lc, LinphoneEvent *lev, LinphonePublishState state) {
	if (strcasecmp(linphone_event_get_name(lev), "Presence") == 0) {
		const bctbx_list_t *cfgs = linphone_core_get_proxy_config_list(lc);
		const bctbx_list_t *item;
		for (item = cfgs; item != NULL; item = bctbx_list_next(item)) {
			LinphoneProxyConfig *cfg = (LinphoneProxyConfig *)bctbx_list_get_data(item);
			if (cfg->presence_publish_event == lev) {
				linphone_proxy_config_notify_publish_state_changed(cfg, state);
				break;
			}
		}
	}
}

static void _linphone_core_init_account_creator_service(LinphoneCore *lc) {
	LinphoneAccountCreatorService *service = linphone_account_creator_service_new();
	service->account_creator_service_constructor_cb = linphone_account_creator_constructor_linphone;
	service->account_creator_service_destructor_cb = NULL;
	service->create_account_request_cb = linphone_account_creator_create_account_linphone;
	service->delete_account_request_cb = linphone_account_creator_delete_account_linphone;
	service->is_account_exist_request_cb = linphone_account_creator_is_account_exist_linphone;
	service->confirmation_key_request_cb = linphone_account_creator_get_confirmation_key_linphone;
	service->activate_account_request_cb = linphone_account_creator_activate_account_linphone;
	service->is_account_activated_request_cb = linphone_account_creator_is_account_activated_linphone;
	service->link_account_request_cb = linphone_account_creator_link_phone_number_with_account_linphone;
	service->activate_alias_request_cb = linphone_account_creator_activate_phone_number_link_linphone;
	service->is_alias_used_request_cb = linphone_account_creator_is_phone_number_used_linphone;
	service->is_account_linked_request_cb = linphone_account_creator_is_account_linked_linphone;
	service->recover_account_request_cb = linphone_account_creator_recover_phone_account_linphone;
	service->update_account_request_cb = linphone_account_creator_update_password_linphone;
	linphone_core_set_account_creator_service(lc, service);
}

static void linphone_core_init(LinphoneCore * lc, LinphoneCoreCbs *cbs, LpConfig *config, void * userdata, void *system_context, bool_t automatically_start) {
	LinphoneFactory *lfactory = linphone_factory_get();
	LinphoneCoreCbs *internal_cbs = _linphone_core_cbs_new();
	const char *msplugins_dir;
	const char *image_resources_dir;

	bctbx_init_logger(FALSE);
	if (liblinphone_user_log_func && liblinphone_current_log_func == NULL)
		bctbx_set_log_handler(liblinphone_current_log_func=liblinphone_user_log_func); /*default value*/

	ms_message("Initializing LinphoneCore %s", linphone_core_get_version());

	lc->is_unreffing = FALSE;
	lc->config=lp_config_ref(config);
	lc->data=userdata;
	lc->ringstream_autorelease=TRUE;

	// We need the Sal on the Android platform helper init
	lc->sal=new Sal(NULL);
	lc->sal->setRefresherRetryAfter(lp_config_get_int(lc->config, "sip", "refresher_retry_after", 60000));
	lc->sal->setHttpProxyHost(L_C_TO_STRING(linphone_core_get_http_proxy_host(lc)));
	lc->sal->setHttpProxyPort(linphone_core_get_http_proxy_port(lc));

	lc->sal->setUserPointer(lc);
	lc->sal->setCallbacks(&linphone_sal_callbacks);

#ifdef __ANDROID__
	if (system_context) {
		JNIEnv *env = ms_get_jni_env();
		if (lc->system_context) {
			env->DeleteGlobalRef((jobject)lc->system_context);
		}
		lc->system_context = (jobject)env->NewGlobalRef((jobject)system_context);
	}
	if (lc->system_context) {
		lc->platform_helper = LinphonePrivate::createAndroidPlatformHelpers(lc->cppPtr, lc->system_context);
	}
	else
		ms_fatal("You must provide the Android's app context when creating the core!");
#elif TARGET_OS_IPHONE
	if (system_context) {
		lc->system_context = system_context;
	}
	lc->platform_helper = LinphonePrivate::createIosPlatformHelpers(lc->cppPtr, lc->system_context);
#endif
	if (lc->platform_helper == NULL)
		lc->platform_helper = new LinphonePrivate::GenericPlatformHelpers(lc->cppPtr);

	msplugins_dir = linphone_factory_get_msplugins_dir(lfactory);
	image_resources_dir = linphone_factory_get_image_resources_dir(lfactory);
	// MS Factory MUST be created after Android has been set, otherwise no camera will be detected !
	lc->factory = ms_factory_new_with_voip_and_directories(msplugins_dir, image_resources_dir);
	lc->sal->setFactory(lc->factory);

	belr::GrammarLoader::get().addPath(std::string(linphone_factory_get_top_resources_dir(lfactory)).append("/belr/grammars"));

	linphone_task_list_init(&lc->hooks);

	_linphone_core_init_account_creator_service(lc);

	linphone_core_cbs_set_notify_received(internal_cbs, linphone_core_internal_notify_received);
	linphone_core_cbs_set_subscribe_received(internal_cbs, linphone_core_internal_subscribe_received);
	linphone_core_cbs_set_subscription_state_changed(internal_cbs, linphone_core_internal_subscription_state_changed);
	linphone_core_cbs_set_publish_state_changed(internal_cbs, linphone_core_internal_publish_state_changed);
	if (lc->vtable_refs == NULL) { // Do not add a new listener upon restart
		_linphone_core_add_callbacks(lc, internal_cbs, TRUE);
	}
	belle_sip_object_unref(internal_cbs);

	if (cbs != NULL) {
		_linphone_core_add_callbacks(lc, cbs, FALSE);
	}

	ortp_init();

	linphone_core_activate_log_serialization_if_needed();

	linphone_core_register_default_codecs(lc);
	linphone_core_register_offer_answer_providers(lc);
	/* Get the mediastreamer2 event queue */
	/* This allows to run event's callback in linphone_core_iterate() */
	lc->msevq=ms_factory_create_event_queue(lc->factory);

#ifdef TUNNEL_ENABLED
	lc->tunnel=linphone_core_tunnel_new(lc);
#endif

	lc->network_last_status = FALSE;

	lc->sip_network_state.global_state = FALSE;
	lc->sip_network_state.user_state = TRUE;
	lc->media_network_state.global_state = FALSE;
	lc->media_network_state.user_state = TRUE;

	/* Create the http provider in dual stack mode (ipv4 and ipv6.
	 * If this creates problem, we may need to implement parallel ipv6/ ipv4 http requests in belle-sip.
	 */
	lc->http_provider = belle_sip_stack_create_http_provider(reinterpret_cast<belle_sip_stack_t *>(lc->sal->getStackImpl()), "::0");
	lc->http_crypto_config = belle_tls_crypto_config_new();
	belle_http_provider_set_tls_crypto_config(lc->http_provider,lc->http_crypto_config);

	//certificates_config_read(lc); // This will be done below in _linphone_core_read_config()

	lc->ringtoneplayer = linphone_ringtoneplayer_new();

	sqlite3_bctbx_vfs_register(0);

	lc->qrcode_rect.h = 0;
	lc->qrcode_rect.w = 0;
	lc->qrcode_rect.x = 0;
	lc->qrcode_rect.y = 0;

	lc->vcard_context = linphone_vcard_context_new();
	linphone_core_initialize_supported_content_types(lc);
	lc->bw_controller = ms_bandwidth_controller_new();

	getPlatformHelpers(lc)->setDnsServers();

	LinphoneFriendList *list = linphone_core_create_friend_list(lc);
	linphone_friend_list_set_display_name(list, "_default");
	linphone_core_add_friend_list(lc, list);
	linphone_friend_list_unref(list);
	lc->presence_model = linphone_presence_model_new();
	linphone_presence_model_set_basic_status(lc->presence_model, LinphonePresenceBasicStatusOpen);

	_linphone_core_read_config(lc);
	linphone_core_set_state(lc, LinphoneGlobalReady, "Ready");

	if (automatically_start) {
		linphone_core_start(lc);
	}
}

LinphoneStatus linphone_core_start (LinphoneCore *lc) {
	try {
		if (lc->state == LinphoneGlobalOn) {
			bctbx_warning("Core is already started, skipping...");
			return -1;
		} else if (lc->state == LinphoneGlobalShutdown) {
			bctbx_error("Can't start a Core that is stopping, wait for Off state");
			return -1;
		} else if (lc->state == LinphoneGlobalOff) {
			bctbx_warning("Core was stopped, before starting it again we need to init it");
			linphone_core_init(lc, NULL, lc->config, lc->data, NULL, FALSE);

			// Decrement refs to avoid leaking
			lp_config_unref(lc->config);
			linphone_core_deactivate_log_serialization_if_needed();
			bctbx_uninit_logger();
		}

		linphone_core_set_state(lc, LinphoneGlobalStartup, "Starting up");

		L_GET_PRIVATE_FROM_C_OBJECT(lc)->init();

		//to give a chance to change uuid before starting
		const char* uuid=lp_config_get_string(lc->config,"misc","uuid",NULL);
		if (!uuid){
			string uuid = lc->sal->createUuid();
			lp_config_set_string(lc->config,"misc","uuid",uuid.c_str());
		}else if (strcmp(uuid,"0")!=0) /*to allow to disable sip.instance*/
			lc->sal->setUuid(uuid);

		if (!lc->sal->getRootCa().empty()) {
			belle_tls_crypto_config_set_root_ca(lc->http_crypto_config, lc->sal->getRootCa().c_str());
			belle_http_provider_set_tls_crypto_config(lc->http_provider, lc->http_crypto_config);
		}

		getPlatformHelpers(lc)->onLinphoneCoreStart(!!lc->auto_net_state_mon);

		linphone_core_set_state(lc, LinphoneGlobalConfiguring, "Configuring");

		const char *remote_provisioning_uri = linphone_core_get_provisioning_uri(lc);
		if (remote_provisioning_uri) {
			if (linphone_remote_provisioning_download_and_apply(lc, remote_provisioning_uri) == -1)
				linphone_configuring_terminated(lc, LinphoneConfiguringFailed, "Bad URI");
		} else {
			linphone_configuring_terminated(lc, LinphoneConfiguringSkipped, NULL);
		}

		return 0;
	} catch (const CorePrivate::DatabaseConnectionFailure &e) {
		bctbx_error("%s", e.what());
		return -2;
	}
}

LinphoneCore *_linphone_core_new_with_config(LinphoneCoreCbs *cbs, struct _LpConfig *config, void *userdata, void *system_context, bool_t automatically_start) {
	LinphoneCore *core = L_INIT(Core);
	Core::create(core);
	linphone_core_init(core, cbs, config, userdata, system_context, automatically_start);
	return core;
}

static LinphoneCore *_linphone_core_new_with_config_and_start (
	const LinphoneCoreVTable *vtable,
	LinphoneConfig *config,
	void *userdata,
	bool_t automatically_start
) {
	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	LinphoneCoreVTable *local_vtable = linphone_core_v_table_new();
	LinphoneCore *core = NULL;
	if (vtable != NULL) *local_vtable = *vtable;
	_linphone_core_cbs_set_v_table(cbs, local_vtable, TRUE);
	core = _linphone_core_new_with_config(cbs, config, userdata, NULL, automatically_start);
	linphone_core_cbs_unref(cbs);
	return core;
}

LinphoneCore *linphone_core_new_with_config(const LinphoneCoreVTable *vtable, struct _LpConfig *config, void *userdata) {
	return _linphone_core_new_with_config_and_start(vtable, config, userdata, TRUE);
}

LinphoneCore *linphone_core_new(const LinphoneCoreVTable *vtable,
						const char *config_path, const char *factory_config_path, void * userdata) {
	LinphoneConfig *config = lp_config_new_with_factory(config_path, factory_config_path);
	LinphoneCore *lc = _linphone_core_new_with_config_and_start(vtable, config, userdata, TRUE);
	linphone_config_unref(config);
	return lc;
}

LinphoneCore *linphone_core_ref(LinphoneCore *lc) {
	belle_sip_object_ref(BELLE_SIP_OBJECT(lc));
	return lc;
}

void linphone_core_unref(LinphoneCore *lc) {
	belle_sip_object_unref(BELLE_SIP_OBJECT(lc));
}

static bctbx_list_t *ortp_payloads_to_linphone_payloads(const bctbx_list_t *ortp_payloads, LinphoneCore *lc) {
	bctbx_list_t *linphone_payloads = NULL;
	for (; ortp_payloads!=NULL; ortp_payloads=bctbx_list_next(ortp_payloads)) {
		LinphonePayloadType *pt = linphone_payload_type_new(lc, (OrtpPayloadType *)ortp_payloads->data);
		linphone_payloads = bctbx_list_append(linphone_payloads, pt);
	}
	return linphone_payloads;
}

static void sort_ortp_pt_list(bctbx_list_t **ortp_pt_list, const bctbx_list_t *linphone_pt_list) {
	bctbx_list_t *new_list = NULL;
	const bctbx_list_t *it;
	for (it=bctbx_list_first_elem(linphone_pt_list); it; it=bctbx_list_next(it)) {
		OrtpPayloadType *ortp_pt = linphone_payload_type_get_ortp_pt((LinphonePayloadType *)it->data);
		bctbx_list_t *elem = bctbx_list_find(*ortp_pt_list, ortp_pt);
		if (elem) {
			*ortp_pt_list = bctbx_list_unlink(*ortp_pt_list, elem);
			new_list = bctbx_list_append_link(new_list, elem);
		}
	}
	*ortp_pt_list = bctbx_list_prepend_link(*ortp_pt_list, new_list);
}

bctbx_list_t *linphone_core_get_audio_payload_types(LinphoneCore *lc) {
	return ortp_payloads_to_linphone_payloads(lc->codecs_conf.audio_codecs, lc);
}

void linphone_core_set_audio_payload_types(LinphoneCore *lc, const bctbx_list_t *payload_types) {
	sort_ortp_pt_list(&lc->codecs_conf.audio_codecs, payload_types);
}

bctbx_list_t *linphone_core_get_video_payload_types(LinphoneCore *lc) {
	return ortp_payloads_to_linphone_payloads(lc->codecs_conf.video_codecs, lc);
}

void linphone_core_set_video_payload_types(LinphoneCore *lc, const bctbx_list_t *payload_types) {
	sort_ortp_pt_list(&lc->codecs_conf.video_codecs, payload_types);
}

bctbx_list_t *linphone_core_get_text_payload_types(LinphoneCore *lc) {
	return ortp_payloads_to_linphone_payloads(lc->codecs_conf.text_codecs, lc);
}

void linphone_core_set_text_payload_types(LinphoneCore *lc, const bctbx_list_t *payload_types) {
	sort_ortp_pt_list(&lc->codecs_conf.text_codecs, payload_types);
}

const bctbx_list_t *linphone_core_get_audio_codecs(const LinphoneCore *lc) {
	return lc->codecs_conf.audio_codecs;
}

const bctbx_list_t *linphone_core_get_video_codecs(const LinphoneCore *lc) {
	return lc->codecs_conf.video_codecs;
}

const bctbx_list_t *linphone_core_get_text_codecs(const LinphoneCore *lc) {
	return lc->codecs_conf.text_codecs;
}

LinphoneStatus linphone_core_set_primary_contact(LinphoneCore *lc, const char *contact) {
	LinphoneAddress *ctt;

	if( lc->sip_conf.contact != NULL && strcmp(lc->sip_conf.contact, contact) == 0){
		/* changing for the same contact: no need to do anything */
		return 0;
	}

	if ((ctt=linphone_address_new(contact))==0) {
		ms_error("Bad contact url: %s",contact);
		return -1;
	}

	if (lc->sip_conf.contact!=NULL) ms_free(lc->sip_conf.contact);
	lc->sip_conf.contact=ms_strdup(contact);
	lp_config_set_string(lc->config, "sip", "contact", lc->sip_conf.contact);

	/* clean the guessed contact, we have to regenerate it */
	if (lc->sip_conf.guessed_contact!=NULL){
		ms_free(lc->sip_conf.guessed_contact);
		lc->sip_conf.guessed_contact=NULL;
	}
	linphone_address_unref(ctt);
	return 0;
}


static void update_primary_contact(LinphoneCore *lc){
	char *guessed=NULL;
	char tmp[LINPHONE_IPADDR_SIZE];
	int port;

	LinphoneAddress *url;
	if (lc->sip_conf.guessed_contact!=NULL){
		ms_free(lc->sip_conf.guessed_contact);
		lc->sip_conf.guessed_contact=NULL;
	}
	url=linphone_address_new(lc->sip_conf.contact);
	if (!url){
		ms_error("Could not parse identity contact !");
		url=linphone_address_new("sip:unknown@unkwownhost");
	}
	linphone_core_get_local_ip(lc, AF_UNSPEC, NULL, tmp);
	if (strcmp(tmp,"127.0.0.1")==0 || strcmp(tmp,"::1")==0 ){
		ms_warning("Local loopback network only !");
		lc->sip_conf.loopback_only=TRUE;
	}else lc->sip_conf.loopback_only=FALSE;
	linphone_address_set_domain(url,tmp);
	port = linphone_core_get_sip_port(lc);
	if (port > 0) linphone_address_set_port(url, port); /*if there is no listening socket the primary contact is somewhat useless,
		it won't work. But we prefer to return something in all cases. It at least shows username and ip address.*/
	guessed=linphone_address_as_string(url);
	lc->sip_conf.guessed_contact=guessed;
	linphone_address_unref(url);
}

const char *linphone_core_get_primary_contact(LinphoneCore *lc){
	char *identity;

	if (lc->sip_conf.guess_hostname){
		if (lc->sip_conf.guessed_contact==NULL || lc->sip_conf.loopback_only){
			update_primary_contact(lc);
		}
		identity=lc->sip_conf.guessed_contact;
	}else{
		identity=lc->sip_conf.contact;
	}
	return identity;
}

void linphone_core_set_guess_hostname(LinphoneCore *lc, bool_t val){
	lc->sip_conf.guess_hostname=val;
}

bool_t linphone_core_get_guess_hostname(LinphoneCore *lc){
	return lc->sip_conf.guess_hostname;
}

//Deprecated
void linphone_core_enable_lime(LinphoneCore *lc, LinphoneLimeState val){
	LinphoneImEncryptionEngine *imee = linphone_im_encryption_engine_new();
	LinphoneImEncryptionEngineCbs *cbs = linphone_im_encryption_engine_get_callbacks(imee);

	if (lime_is_available()) {
		if (linphone_core_ready(lc)) {
			lp_config_set_int(lc->config,"sip","lime",val);
		}

		linphone_im_encryption_engine_cbs_set_process_incoming_message(cbs, lime_im_encryption_engine_process_incoming_message_cb);
		linphone_im_encryption_engine_cbs_set_process_downloading_file(cbs, lime_im_encryption_engine_process_downloading_file_cb);

		if (val != LinphoneLimeDisabled) {
			linphone_im_encryption_engine_cbs_set_process_outgoing_message(cbs, lime_im_encryption_engine_process_outgoing_message_cb);
			linphone_im_encryption_engine_cbs_set_process_uploading_file(cbs, lime_im_encryption_engine_process_uploading_file_cb);
			linphone_im_encryption_engine_cbs_set_is_encryption_enabled_for_file_transfer(cbs, lime_im_encryption_engine_is_file_encryption_enabled_cb);
			linphone_im_encryption_engine_cbs_set_generate_file_transfer_key(cbs, lime_im_encryption_engine_generate_file_transfer_key_cb);
		} else {
			linphone_im_encryption_engine_cbs_set_process_outgoing_message(cbs, NULL);
			linphone_im_encryption_engine_cbs_set_process_uploading_file(cbs, NULL);
			linphone_im_encryption_engine_cbs_set_is_encryption_enabled_for_file_transfer(cbs, NULL);
			linphone_im_encryption_engine_cbs_set_generate_file_transfer_key(cbs, NULL);
		}

		linphone_core_set_im_encryption_engine(lc, imee);
	}
	linphone_im_encryption_engine_unref(imee);
}

//Deprecated
bool_t linphone_core_lime_available(const LinphoneCore *lc){
	return lime_is_available();
}

//Deprecated
LinphoneLimeState linphone_core_lime_enabled(const LinphoneCore *lc){
	return linphone_core_lime_available(lc) ? static_cast<LinphoneLimeState>(lp_config_get_int(lc->config,"sip", "lime", LinphoneLimeDisabled)) : LinphoneLimeDisabled;
}

//Deprecated
LinphoneLimeState linphone_core_lime_for_file_sharing_enabled(const LinphoneCore *lc){
	LinphoneLimeState s = linphone_core_lime_enabled(lc);
	if (s != LinphoneLimeDisabled) {
		s = static_cast<LinphoneLimeState>(lp_config_get_int(lc->config,"sip", "lime_for_file_sharing", LinphoneLimeMandatory));
	}
	return s;
}

LinphoneAddress *linphone_core_get_primary_contact_parsed(LinphoneCore *lc){
	return linphone_address_new(linphone_core_get_primary_contact(lc));
}

LinphoneAddress *linphone_core_create_primary_contact_parsed (LinphoneCore *lc) {
	return linphone_address_new(linphone_core_get_primary_contact(lc));
}

LinphoneStatus linphone_core_set_audio_codecs(LinphoneCore *lc, bctbx_list_t *codecs){
	if (lc->codecs_conf.audio_codecs!=NULL) bctbx_list_free(lc->codecs_conf.audio_codecs);
	lc->codecs_conf.audio_codecs=codecs;
	_linphone_core_codec_config_write(lc);
	linphone_core_update_allocated_audio_bandwidth(lc);
	return 0;
}

LinphoneStatus linphone_core_set_video_codecs(LinphoneCore *lc, bctbx_list_t *codecs){
	if (lc->codecs_conf.video_codecs!=NULL) bctbx_list_free(lc->codecs_conf.video_codecs);
	lc->codecs_conf.video_codecs=codecs;
	_linphone_core_codec_config_write(lc);
	return 0;
}

LinphoneStatus linphone_core_set_text_codecs(LinphoneCore *lc, bctbx_list_t *codecs) {
	if (lc->codecs_conf.text_codecs != NULL)
		bctbx_list_free(lc->codecs_conf.text_codecs);

	lc->codecs_conf.text_codecs = codecs;
	_linphone_core_codec_config_write(lc);
	return 0;
}

void linphone_core_enable_generic_comfort_noise(LinphoneCore *lc, bool_t enabled){
	lp_config_set_int(lc->config, "misc", "use_cn", enabled);
}

bool_t linphone_core_generic_comfort_noise_enabled(const LinphoneCore *lc){
	return !!lp_config_get_int(lc->config, "misc", "use_cn", FALSE);
}

const bctbx_list_t* linphone_core_get_friend_list(const LinphoneCore *lc) {
	bctbx_list_t *lists = lc->friends_lists;
	if (lists) {
		LinphoneFriendList *list = (LinphoneFriendList *)lists->data;
		if (list) {
			return list->friends;
		}
	}
	return NULL;
}

const bctbx_list_t* linphone_core_get_friends_lists(const LinphoneCore *lc) {
	return lc->friends_lists;
}

LinphoneFriendList* linphone_core_get_default_friend_list(const LinphoneCore *lc) {
	if (lc && lc->friends_lists) {
		return (LinphoneFriendList *)lc->friends_lists->data;
	}
	return NULL;
}

LinphoneFriendList *linphone_core_get_friend_list_by_name(const LinphoneCore *lc, const char *name) {
	if (!lc)
		return NULL;

	LinphoneFriendList *ret = NULL;
	bctbx_list_t *list_copy = lc->friends_lists;
	while (list_copy) {
		LinphoneFriendList *list = (LinphoneFriendList *)list_copy->data;
		const char *list_name = linphone_friend_list_get_display_name(list);
		if (list_name && strcmp(name, list_name) == 0) {
			ret = list;
			break;
		}
		list_copy = list_copy->next;
	}

	return ret;
}

void linphone_core_remove_friend_list(LinphoneCore *lc, LinphoneFriendList *list) {
	bctbx_list_t *elem = bctbx_list_find(lc->friends_lists, list);
	if (elem == NULL) return;
	linphone_core_remove_friends_list_from_db(lc, list);
	linphone_core_notify_friend_list_removed(lc, list);
	list->lc = NULL;
	linphone_friend_list_unref(list);
	lc->friends_lists = bctbx_list_erase_link(lc->friends_lists, elem);
}

void linphone_core_clear_bodyless_friend_lists(LinphoneCore *lc) {
	bctbx_list_t *copy = bctbx_list_copy(linphone_core_get_friends_lists((const LinphoneCore *)lc));
	for (auto it = copy; it; it = bctbx_list_next(it)) {
		LinphoneFriendList *friends = (LinphoneFriendList *)bctbx_list_get_data(it);
		if (linphone_friend_list_is_subscription_bodyless(friends))
			linphone_core_remove_friend_list(lc, (LinphoneFriendList *)bctbx_list_get_data(it));
	}
	bctbx_list_free(copy);
}

void linphone_core_add_friend_list(LinphoneCore *lc, LinphoneFriendList *list) {
	if (!list->lc) {
		list->lc = lc;
	}
	lc->friends_lists = bctbx_list_append(lc->friends_lists, linphone_friend_list_ref(list));
	linphone_core_store_friends_list_in_db(lc, list);
	linphone_core_notify_friend_list_created(lc, list);
}

const bctbx_list_t * linphone_core_find_contacts_by_char(LinphoneCore *core, const char *filter, bool_t sip_only) {
	// Get sipuri from filter if possible
	bctbx_list_t *list = NULL, *_list = NULL;
	LinphoneAddress *addr = linphone_core_interpret_url(core, (sip_only) ? filter : "");
	bctbx_list_t* listFriendsList = (bctbx_list_t*)linphone_core_get_friends_lists(core);
	bctbx_list_t* listFriend = (listFriendsList != NULL)
			? (bctbx_list_t*)linphone_friend_list_get_friends((LinphoneFriendList*)listFriendsList->data) : NULL;

	if (addr != NULL)
		list = bctbx_list_new(addr);

	while (listFriend != NULL && listFriend->data != NULL) {
		LinphoneAddress *buff = (LinphoneAddress*)linphone_friend_get_address((LinphoneFriend*)listFriend->data);
		if (buff != NULL) {
			bctbx_list_t *new_list = bctbx_list_new(buff);
			if (list == NULL) {
				_list = list = new_list;
			} else {
				if (_list == NULL) _list = list;
				_list->next = new_list;
				_list = _list->next;
			}
		}
		listFriend = listFriend->next;
	}

	return list;
}

void linphone_core_enable_audio_adaptive_jittcomp(LinphoneCore* lc, bool_t val) {
	lc->rtp_conf.audio_adaptive_jitt_comp_enabled = val;
}

bool_t linphone_core_audio_adaptive_jittcomp_enabled(LinphoneCore* lc) {
	return lc->rtp_conf.audio_adaptive_jitt_comp_enabled;
}

int linphone_core_get_audio_jittcomp(LinphoneCore *lc) {
	return lc->rtp_conf.audio_jitt_comp;
}

void linphone_core_enable_video_adaptive_jittcomp(LinphoneCore* lc, bool_t val) {
	lc->rtp_conf.video_adaptive_jitt_comp_enabled = val;
}

bool_t linphone_core_video_adaptive_jittcomp_enabled(LinphoneCore* lc) {
	return lc->rtp_conf.video_adaptive_jitt_comp_enabled;
}

int linphone_core_get_video_jittcomp(LinphoneCore *lc) {
	return lc->rtp_conf.video_jitt_comp;
}

int linphone_core_get_audio_port(const LinphoneCore *lc) {
	return lc->rtp_conf.audio_rtp_min_port;
}

void linphone_core_get_audio_port_range(const LinphoneCore *lc, int *min_port, int *max_port) {
	*min_port = lc->rtp_conf.audio_rtp_min_port;
	*max_port = lc->rtp_conf.audio_rtp_max_port;
}

LinphoneRange *linphone_core_get_audio_ports_range(const LinphoneCore *lc) {
	LinphoneRange *range = linphone_range_new();
	range->min = lc->rtp_conf.audio_rtp_min_port;
	range->max = lc->rtp_conf.audio_rtp_max_port;
	return range;
}

int linphone_core_get_video_port(const LinphoneCore *lc){
	return lc->rtp_conf.video_rtp_min_port;
}

void linphone_core_get_video_port_range(const LinphoneCore *lc, int *min_port, int *max_port) {
	*min_port = lc->rtp_conf.video_rtp_min_port;
	*max_port = lc->rtp_conf.video_rtp_max_port;
}

LinphoneRange *linphone_core_get_video_ports_range(const LinphoneCore *lc) {
	LinphoneRange *range = linphone_range_new();
	range->min = lc->rtp_conf.video_rtp_min_port;
	range->max = lc->rtp_conf.video_rtp_max_port;
	return range;
}

int linphone_core_get_text_port(const LinphoneCore *lc) {
	return lc->rtp_conf.text_rtp_min_port;
}

void linphone_core_get_text_port_range(const LinphoneCore *lc, int *min_port, int *max_port) {
	*min_port = lc->rtp_conf.text_rtp_min_port;
	*max_port = lc->rtp_conf.text_rtp_max_port;
}

LinphoneRange *linphone_core_get_text_ports_range(const LinphoneCore *lc) {
	LinphoneRange *range = linphone_range_new();
	range->min = lc->rtp_conf.text_rtp_min_port;
	range->max = lc->rtp_conf.text_rtp_max_port;
	return range;
}

int linphone_core_get_nortp_timeout(const LinphoneCore *lc){
	return lc->rtp_conf.nortp_timeout;
}

bool_t linphone_core_get_rtp_no_xmit_on_audio_mute(const LinphoneCore *lc){
	return lc->rtp_conf.rtp_no_xmit_on_audio_mute;
}

static void apply_jitter_value(LinphoneCore *lc, int value, MSFormatType stype){
	for (const auto &call : L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getCalls()) {
		MediaStream *ms = (stype == MSAudio)
			? L_GET_PRIVATE(call)->getMediaStream(LinphoneStreamTypeAudio)
			: L_GET_PRIVATE(call)->getMediaStream(LinphoneStreamTypeVideo);
		if (ms) {
			RtpSession *s=ms->sessions.rtp_session;
			if (s){
				if (value>0){
					ms_message("Jitter buffer size set to [%i] ms on call [%p]", value, call.get());
					rtp_session_set_jitter_compensation(s,value);
					rtp_session_enable_jitter_buffer(s,TRUE);
				}else if (value==0){
					ms_warning("Jitter buffer is disabled per application request on call [%p]", call.get());
					rtp_session_enable_jitter_buffer(s,FALSE);
				}
			}
		}
	}
}

void linphone_core_set_audio_jittcomp(LinphoneCore *lc, int milliseconds) {
	lc->rtp_conf.audio_jitt_comp=milliseconds;
	apply_jitter_value(lc, milliseconds, MSAudio);
}

void linphone_core_set_video_jittcomp(LinphoneCore *lc, int milliseconds) {
	lc->rtp_conf.video_jitt_comp=milliseconds;
	apply_jitter_value(lc, milliseconds, MSVideo);
}

void linphone_core_set_rtp_no_xmit_on_audio_mute(LinphoneCore *lc,bool_t rtp_no_xmit_on_audio_mute){
	lc->rtp_conf.rtp_no_xmit_on_audio_mute=rtp_no_xmit_on_audio_mute;
}

void linphone_core_set_audio_port(LinphoneCore *lc, int port) {
	lc->rtp_conf.audio_rtp_min_port=lc->rtp_conf.audio_rtp_max_port=port;
}

void linphone_core_set_audio_port_range(LinphoneCore *lc, int min_port, int max_port) {
	lc->rtp_conf.audio_rtp_min_port=min_port;
	lc->rtp_conf.audio_rtp_max_port=max_port;
}

void linphone_core_set_video_port(LinphoneCore *lc, int port){
	lc->rtp_conf.video_rtp_min_port=lc->rtp_conf.video_rtp_max_port=port;
}

void linphone_core_set_video_port_range(LinphoneCore *lc, int min_port, int max_port) {
	lc->rtp_conf.video_rtp_min_port=min_port;
	lc->rtp_conf.video_rtp_max_port=max_port;
}

void linphone_core_set_text_port(LinphoneCore *lc, int port) {
	lc->rtp_conf.text_rtp_min_port = lc->rtp_conf.text_rtp_max_port = port;
}

void linphone_core_set_text_port_range(LinphoneCore *lc, int min_port, int max_port) {
	lc->rtp_conf.text_rtp_min_port = min_port;
	lc->rtp_conf.text_rtp_max_port = max_port;
}

void linphone_core_set_nortp_timeout(LinphoneCore *lc, int nortp_timeout){
	lc->rtp_conf.nortp_timeout=nortp_timeout;
}

bool_t linphone_core_get_use_info_for_dtmf(LinphoneCore *lc) {
	return !!lp_config_get_int(lc->config, "sip", "use_info", 0);
}

void linphone_core_set_use_info_for_dtmf(LinphoneCore *lc,bool_t use_info) {
	if (linphone_core_ready(lc)) {
		lp_config_set_int(lc->config, "sip", "use_info", use_info);
	}
}

bool_t linphone_core_get_use_rfc2833_for_dtmf(LinphoneCore *lc) {
	return !!lp_config_get_int(lc->config, "sip", "use_rfc2833", 1);
}

void linphone_core_set_use_rfc2833_for_dtmf(LinphoneCore *lc,bool_t use_rfc2833) {
	if (linphone_core_ready(lc)) {
		lp_config_set_int(lc->config, "sip", "use_rfc2833", use_rfc2833);
	}
}

int linphone_core_get_sip_port(LinphoneCore *lc){
	LinphoneSipTransports tr;
	linphone_core_get_sip_transports_used(lc,&tr);
	return tr.udp_port>0 ? tr.udp_port : (tr.tcp_port > 0 ? tr.tcp_port : tr.tls_port);
}

static char _ua_name[64]="Linphone";
static char _ua_version[64]=LIBLINPHONE_VERSION;

void linphone_core_set_user_agent(LinphoneCore *lc, const char *name, const char *ver){
	ostringstream ua_string;
	ua_string << (name ? name : "");
	if (ver) ua_string << "/" << ver;
	if (lc->sal) {
		lc->sal->setUserAgent(ua_string.str());
	}
}
const char *linphone_core_get_user_agent(LinphoneCore *lc){
	return lc->sal->getUserAgent().c_str();
}

const char *linphone_core_get_user_agent_name(void){
	return _ua_name;
}

const char *linphone_core_get_user_agent_version(void){
	return _ua_version;
}

static bool_t transports_unchanged(const LinphoneSipTransports * tr1, const LinphoneSipTransports * tr2){
	return
		tr2->udp_port==tr1->udp_port &&
		tr2->tcp_port==tr1->tcp_port &&
		tr2->dtls_port==tr1->dtls_port &&
		tr2->tls_port==tr1->tls_port;
}

static void __linphone_core_invalidate_registers(LinphoneCore* lc){
	const bctbx_list_t *elem=linphone_core_get_proxy_config_list(lc);
	for(;elem!=NULL;elem=elem->next){
		LinphoneProxyConfig *cfg=(LinphoneProxyConfig*)elem->data;
		if (linphone_proxy_config_register_enabled(cfg)) {
			/*this will force a re-registration at next iterate*/
			cfg->commit = TRUE;
		}
	}
}

int _linphone_core_apply_transports(LinphoneCore *lc){
	Sal *sal=lc->sal;
	const char *anyaddr;
	LinphoneSipTransports *tr=&lc->sip_conf.transports;
	const char* listening_address;
	/*first of all invalidate all current registrations so that we can register again with new transports*/
	__linphone_core_invalidate_registers(lc);

	if (lc->sip_conf.ipv6_enabled)
		anyaddr="::0";
	else
		anyaddr="0.0.0.0";

	sal->unlistenPorts();

	listening_address = lp_config_get_string(lc->config,"sip","bind_address",anyaddr);
	if (linphone_core_get_http_proxy_host(lc)) {
		sal->setHttpProxyHost(linphone_core_get_http_proxy_host(lc));
		sal->setHttpProxyPort(linphone_core_get_http_proxy_port(lc));
	}
	if (lc->tunnel && linphone_tunnel_sip_enabled(lc->tunnel) && linphone_tunnel_get_activated(lc->tunnel)){
		sal->setListenPort(anyaddr,tr->udp_port,SalTransportUDP,TRUE);
	}else{
		if (tr->udp_port!=0){
			sal->setListenPort(listening_address,tr->udp_port,SalTransportUDP,FALSE);
		}
		if (tr->tcp_port!=0){
			sal->setListenPort (listening_address,tr->tcp_port,SalTransportTCP,FALSE);
		}
		if (linphone_core_sip_transport_supported(lc,LinphoneTransportTls)){
			if (tr->tls_port!=0)
				sal->setListenPort (listening_address,tr->tls_port,SalTransportTLS,FALSE);
		}
	}
	return 0;
}

bool_t linphone_core_sip_transport_supported(const LinphoneCore *lc, LinphoneTransportType tp){
	return !!lc->sal->isTransportAvailable((SalTransport)tp);
}

BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneTransports);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneTransports, belle_sip_object_t,
	NULL, // destroy
	NULL, // clone
	NULL, // marshal
	FALSE
);

LinphoneTransports *linphone_transports_new() {
	LinphoneTransports *transports = belle_sip_object_new(LinphoneTransports);
	transports->udp_port = 0;
	transports->tcp_port = 0;
	transports->tls_port = 0;
	transports->dtls_port = 0;
	return transports;
}

LinphoneTransports* linphone_transports_ref(LinphoneTransports* transports) {
	return (LinphoneTransports*) belle_sip_object_ref(transports);
}

void linphone_transports_unref (LinphoneTransports* transports) {
	belle_sip_object_unref(transports);
}

void *linphone_transports_get_user_data(const LinphoneTransports *transports) {
	return transports->user_data;
}

void linphone_transports_set_user_data(LinphoneTransports *transports, void *data) {
	transports->user_data = data;
}

int linphone_transports_get_udp_port(const LinphoneTransports* transports) {
	return transports->udp_port;
}

int linphone_transports_get_tcp_port(const LinphoneTransports* transports) {
	return transports->tcp_port;
}

int linphone_transports_get_tls_port(const LinphoneTransports* transports) {
	return transports->tls_port;
}

int linphone_transports_get_dtls_port(const LinphoneTransports* transports) {
	return transports->dtls_port;
}

void linphone_transports_set_udp_port(LinphoneTransports *transports, int port) {
	transports->udp_port = port;
}

void linphone_transports_set_tcp_port(LinphoneTransports *transports, int port) {
	transports->tcp_port = port;
}

void linphone_transports_set_tls_port(LinphoneTransports *transports, int port) {
	transports->tls_port = port;
}

void linphone_transports_set_dtls_port(LinphoneTransports *transports, int port) {
	transports->dtls_port = port;
}

LinphoneStatus linphone_core_set_sip_transports(LinphoneCore *lc, const LinphoneSipTransports * tr_config /*config to be saved*/){
	LinphoneSipTransports tr=*tr_config;

	if (lp_config_get_int(lc->config,"sip","sip_random_port",0)==1) {
		/*legacy random mode*/
		if (tr.udp_port>0){
			tr.udp_port=LC_SIP_TRANSPORT_RANDOM;
		}
		if (tr.tcp_port>0){
			tr.tcp_port=LC_SIP_TRANSPORT_RANDOM;
		}
		if (tr.tls_port>0){
			tr.tls_port=LC_SIP_TRANSPORT_RANDOM;
		}
	}

	if (tr.udp_port==0 && tr.tcp_port==0 && tr.tls_port==0){
		tr.udp_port=5060;
	}

	if (transports_unchanged(&tr,&lc->sip_conf.transports))
		return 0;
	memcpy(&lc->sip_conf.transports,&tr,sizeof(tr));

	if (linphone_core_ready(lc)){
		lp_config_set_int(lc->config,"sip","sip_port",tr_config->udp_port);
		lp_config_set_int(lc->config,"sip","sip_tcp_port",tr_config->tcp_port);
		lp_config_set_int(lc->config,"sip","sip_tls_port",tr_config->tls_port);
	}

	if (lc->sal==NULL) return 0;
	return _linphone_core_apply_transports(lc);
}

LinphoneStatus linphone_core_set_transports(LinphoneCore *lc, const LinphoneTransports * transports){
	if (transports->udp_port == lc->sip_conf.transports.udp_port &&
		transports->tcp_port == lc->sip_conf.transports.tcp_port &&
		transports->tls_port == lc->sip_conf.transports.tls_port &&
		transports->dtls_port == lc->sip_conf.transports.dtls_port) {
		return 0;
	}
	lc->sip_conf.transports.udp_port = transports->udp_port;
	lc->sip_conf.transports.tcp_port = transports->tcp_port;
	lc->sip_conf.transports.tls_port = transports->tls_port;
	lc->sip_conf.transports.dtls_port = transports->dtls_port;

	if (linphone_core_ready(lc)) {
		lp_config_set_int(lc->config,"sip", "sip_port", transports->udp_port);
		lp_config_set_int(lc->config,"sip", "sip_tcp_port", transports->tcp_port);
		lp_config_set_int(lc->config,"sip", "sip_tls_port", transports->tls_port);
	}

	if (lc->sal == NULL) return 0;
	return _linphone_core_apply_transports(lc);
}

LinphoneStatus linphone_core_get_sip_transports(LinphoneCore *lc, LinphoneSipTransports *tr){
	memcpy(tr,&lc->sip_conf.transports,sizeof(*tr));
	return 0;
}

LinphoneTransports *linphone_core_get_transports(LinphoneCore *lc){
	LinphoneTransports *transports = linphone_transports_new();
	transports->udp_port = lc->sip_conf.transports.udp_port;
	transports->tcp_port = lc->sip_conf.transports.tcp_port;
	transports->tls_port = lc->sip_conf.transports.tls_port;
	transports->dtls_port = lc->sip_conf.transports.dtls_port;
	return transports;
}

void linphone_core_get_sip_transports_used(LinphoneCore *lc, LinphoneSipTransports *tr){
	tr->udp_port=lc->sal->getListeningPort(SalTransportUDP);
	tr->tcp_port=lc->sal->getListeningPort(SalTransportTCP);
	tr->tls_port=lc->sal->getListeningPort(SalTransportTLS);
}

LinphoneTransports *linphone_core_get_transports_used(LinphoneCore *lc){
	LinphoneTransports *transports = linphone_transports_new();
	transports->udp_port = lc->sal->getListeningPort(SalTransportUDP);
	transports->tcp_port = lc->sal->getListeningPort(SalTransportTCP);
	transports->tls_port = lc->sal->getListeningPort(SalTransportTLS);
	transports->dtls_port = lc->sal->getListeningPort(SalTransportDTLS);
	return transports;
}

void linphone_core_set_sip_port(LinphoneCore *lc,int port) {
	LinphoneSipTransports tr;
	memset(&tr,0,sizeof(tr));
	tr.udp_port=port;
	linphone_core_set_sip_transports (lc,&tr);
}

bool_t linphone_core_ipv6_enabled(LinphoneCore *lc){
	return lc->sip_conf.ipv6_enabled;
}

void linphone_core_enable_ipv6(LinphoneCore *lc, bool_t val){
	if (lc->sip_conf.ipv6_enabled!=val){
		lc->sip_conf.ipv6_enabled=val;
		if (lc->sal){
			/* we need to update the sip stack */
			_linphone_core_apply_transports(lc);
		}
		/*update the localip immediately for the network monitor to avoid to "discover" later that we switched to ipv6*/
		linphone_core_get_local_ip(lc,AF_UNSPEC,NULL,lc->localip);
		if (linphone_core_ready(lc)){
			lp_config_set_int(lc->config,"sip","use_ipv6",(int)val);
		}
	}
}

bool_t linphone_core_retransmission_on_nack_enabled(LinphoneCore *lc) {
	return lc->video_conf.retransmission_on_nack_enabled;
}

void linphone_core_enable_retransmission_on_nack(LinphoneCore *lc, bool_t val) {
	if (lc->video_conf.retransmission_on_nack_enabled != val) {
		lc->video_conf.retransmission_on_nack_enabled = val;

		if (linphone_core_ready(lc)) {
			lp_config_set_int(lc->config, "video", "retransmission_on_nack_enabled", (int)val);
		}
	}
}

bool_t linphone_core_wifi_only_enabled(LinphoneCore *lc) {
	return (bool_t)lp_config_get_int(lc->config, "net", "wifi_only", 0);
}

void linphone_core_enable_wifi_only(LinphoneCore *lc, bool_t val) {
	if (linphone_core_ready(lc)) {
		lp_config_set_int(lc->config, "net", "wifi_only", (int)val);
		getPlatformHelpers(lc)->onWifiOnlyEnabled(!!val);
	}
}

bool_t linphone_core_content_encoding_supported(const LinphoneCore *lc, const char *content_encoding) {
	const char *handle_content_encoding = lp_config_get_string(lc->config, "sip", "handle_content_encoding", "deflate");
	return (strcmp(handle_content_encoding, content_encoding) == 0) && lc->sal->isContentEncodingAvailable(content_encoding);
}

static void notify_network_reachable_change (LinphoneCore *lc) {
	if (!lc->network_reachable_to_be_notified)
		return;

	lc->network_reachable_to_be_notified = FALSE;
	linphone_core_notify_network_reachable(lc, lc->sip_network_state.global_state);
	if (lc->sip_network_state.global_state)
		linphone_core_resolve_stun_server(lc);
}

static void proxy_update(LinphoneCore *lc){
	bctbx_list_t *elem,*next;
	bctbx_list_for_each(lc->sip_conf.proxies,(void (*)(void*))&linphone_proxy_config_update);
	for(elem=lc->sip_conf.deleted_proxies;elem!=NULL;elem=next){
		LinphoneProxyConfig* cfg = (LinphoneProxyConfig*)elem->data;
		next=elem->next;
		if (ms_time(NULL) - cfg->deletion_date > 32) {
			lc->sip_conf.deleted_proxies =bctbx_list_erase_link(lc->sip_conf.deleted_proxies,elem);
			ms_message("Proxy config for [%s] is definitely removed from core.",linphone_proxy_config_get_addr(cfg));
			_linphone_proxy_config_release_ops(cfg);
			linphone_proxy_config_unref(cfg);
		}
	}
}

static void assign_buddy_info(LinphoneCore *lc, BuddyInfo *info){
	LinphoneFriend *lf=linphone_core_get_friend_by_address(lc,info->sip_uri);
	if (lf!=NULL){
		lf->info=info;
		ms_message("%s has a BuddyInfo assigned with image %p",info->sip_uri, info->image_data);
		linphone_core_notify_buddy_info_updated(lc,lf);
	}else{
		ms_warning("Could not any friend with uri %s",info->sip_uri);
	}
}

static void analyze_buddy_lookup_results(LinphoneCore *lc, LinphoneProxyConfig *cfg){
	bctbx_list_t *elem;
	SipSetupContext *ctx=linphone_proxy_config_get_sip_setup_context(cfg);
	for (elem=lc->bl_reqs;elem!=NULL;elem=bctbx_list_next(elem)){
		BuddyLookupRequest *req=(BuddyLookupRequest *)elem->data;
		if (req->status==BuddyLookupDone || req->status==BuddyLookupFailure){
			if (req->results!=NULL){
				BuddyInfo *i=(BuddyInfo*)req->results->data;
				bctbx_list_free(req->results);
				req->results=NULL;
				assign_buddy_info(lc,i);
			}
			sip_setup_context_buddy_lookup_free(ctx,req);
			elem->data=NULL;
		}
	}
	/*purge completed requests */
	while((elem=bctbx_list_find(lc->bl_reqs,NULL))!=NULL){
		lc->bl_reqs=bctbx_list_erase_link(lc->bl_reqs,elem);
	}
}

static void linphone_core_grab_buddy_infos(LinphoneCore *lc, LinphoneProxyConfig *cfg){
	const bctbx_list_t *elem;
	SipSetupContext *ctx=linphone_proxy_config_get_sip_setup_context(cfg);
	for(elem=linphone_core_get_friend_list(lc);elem!=NULL;elem=elem->next){
		LinphoneFriend *lf=(LinphoneFriend*)elem->data;
		if (lf->info==NULL){
			if (linphone_core_lookup_known_proxy(lc,lf->uri)==cfg){
				if (linphone_address_get_username(lf->uri)!=NULL){
					BuddyLookupRequest *req;
					char *tmp=linphone_address_as_string_uri_only(lf->uri);
					req=sip_setup_context_create_buddy_lookup_request(ctx);
					buddy_lookup_request_set_key(req,tmp);
					buddy_lookup_request_set_max_results(req,1);
					sip_setup_context_buddy_lookup_submit(ctx,req);
					lc->bl_reqs=bctbx_list_append(lc->bl_reqs,req);
					ms_free(tmp);
				}
			}
		}
	}
}

static void linphone_core_do_plugin_tasks(LinphoneCore *lc){
	LinphoneProxyConfig *cfg=linphone_core_get_default_proxy_config(lc);
	if (cfg){
		if (lc->bl_refresh){
			SipSetupContext *ctx=linphone_proxy_config_get_sip_setup_context(cfg);
			if (ctx && (sip_setup_context_get_capabilities(ctx) & SIP_SETUP_CAP_BUDDY_LOOKUP)){
				linphone_core_grab_buddy_infos(lc,cfg);
				lc->bl_refresh=FALSE;
			}
		}
		if (lc->bl_reqs) analyze_buddy_lookup_results(lc,cfg);
	}
}

void linphone_core_iterate(LinphoneCore *lc){
	uint64_t curtime_ms = ms_get_cur_time_ms(); /*monotonic time*/
	time_t current_real_time = ms_time(NULL);
	int64_t diff_time;
	bool one_second_elapsed = false;

	if (lc->prevtime_ms == 0){
		lc->prevtime_ms = curtime_ms;
	}
	if ((diff_time=(int64_t)(curtime_ms-lc->prevtime_ms)) >= 1000){
		one_second_elapsed = true;
		if (diff_time>3000){
			/*since monotonic time doesn't increment while machine is sleeping, we don't want to catchup too much*/
			lc->prevtime_ms = curtime_ms;
		}else{
			lc->prevtime_ms += 1000;
		}
	}

	if (lc->ecc!=NULL){
		LinphoneEcCalibratorStatus ecs=ec_calibrator_get_status(lc->ecc);
		if (ecs!=LinphoneEcCalibratorInProgress){
			if (lc->ecc->cb)
				lc->ecc->cb(lc,ecs,lc->ecc->delay,lc->ecc->cb_data);
			if (ecs==LinphoneEcCalibratorDone){
				int len=lp_config_get_int(lc->config,"sound","ec_tail_len",0);
				int margin=len/2;

				lp_config_set_int(lc->config, "sound", "ec_delay",MAX(lc->ecc->delay-margin,0));
			} else if (ecs == LinphoneEcCalibratorFailed) {
				lp_config_set_int(lc->config, "sound", "ec_delay", -1);/*use default value from soundcard*/
			} else if (ecs == LinphoneEcCalibratorDoneNoEcho) {
				linphone_core_enable_echo_cancellation(lc, FALSE);
			}
			ec_calibrator_destroy(lc->ecc);
			lc->ecc=NULL;
		}
	}

	if (lc->preview_finished){
		lc->preview_finished=0;
		linphone_ringtoneplayer_stop(lc->ringtoneplayer);
		lc_callback_obj_invoke(&lc->preview_finished_cb,lc);
	}

	if (lc->ringstream && lc->ringstream_autorelease && lc->dmfs_playing_start_time!=0
		&& (curtime_ms/1000 - (uint64_t)lc->dmfs_playing_start_time)>5){
		MSPlayerState state;
		bool_t stop=TRUE;
		if (lc->ringstream->source && ms_filter_call_method(lc->ringstream->source,MS_PLAYER_GET_STATE,&state)==0){
			if (state==MSPlayerPlaying) stop=FALSE;
		}
		if (stop) {
			ms_message("Releasing inactive tone player.");
			linphone_core_stop_dtmf_stream(lc);
		}
	}

	lc->sal->iterate();
	if (lc->msevq) ms_event_queue_pump(lc->msevq);
	if (linphone_core_get_global_state(lc) == LinphoneGlobalConfiguring)
		// Avoid registration before getting remote configuration results
		return;

	proxy_update(lc);

	/* We have to iterate for each call */
	L_GET_PRIVATE_FROM_C_OBJECT(lc)->iterateCalls(current_real_time, one_second_elapsed);

	if (linphone_core_video_preview_enabled(lc)){
		if (lc->previewstream==NULL && !L_GET_PRIVATE_FROM_C_OBJECT(lc)->hasCalls())
			toggle_video_preview(lc,TRUE);
#ifdef VIDEO_ENABLED
		if (lc->previewstream) video_stream_iterate(lc->previewstream);
#endif
	}else{
		if (lc->previewstream!=NULL)
			toggle_video_preview(lc,FALSE);
	}

	linphone_core_run_hooks(lc);
	linphone_core_do_plugin_tasks(lc);

	if (lc->sip_network_state.global_state && lc->netup_time!=0 && (current_real_time-lc->netup_time)>=2){
		/*not do that immediately, take your time.*/
		linphone_core_send_initial_subscribes(lc);
	}

	if (one_second_elapsed) {
		bctbx_list_t *elem = NULL;
		if (lp_config_needs_commit(lc->config)) {
			lp_config_sync(lc->config);
		}
		for (elem = lc->friends_lists; elem != NULL; elem = bctbx_list_next(elem)) {
			LinphoneFriendList *list = (LinphoneFriendList *)elem->data;
			if (list->dirty_friends_to_update) {
				linphone_friend_list_update_dirty_friends(list);
			}
		}
	}

	if (liblinphone_serialize_logs == TRUE) {
		ortp_logv_flush();
	}
}

LinphoneAddress * linphone_core_interpret_url(LinphoneCore *lc, const char *url){
	LinphoneProxyConfig *proxy = linphone_core_get_default_proxy_config(lc);
	LinphoneAddress *result=NULL;

	if (linphone_proxy_config_is_phone_number(proxy,url)) {
		char *normalized_number = linphone_proxy_config_normalize_phone_number(proxy, url);
		result = linphone_proxy_config_normalize_sip_uri(proxy, normalized_number);
		ms_free(normalized_number);
	} else {
		result = linphone_proxy_config_normalize_sip_uri(proxy, url);
	}
	return result;
}

const char * linphone_core_get_identity(LinphoneCore *lc){
	LinphoneProxyConfig *proxy=linphone_core_get_default_proxy_config(lc);
	const char *from;
	if (proxy!=NULL) {
		from=linphone_proxy_config_get_identity(proxy);
	}else from=linphone_core_get_primary_contact(lc);
	return from;
}

const char * linphone_core_get_route(LinphoneCore *lc){
	LinphoneProxyConfig *proxy=linphone_core_get_default_proxy_config(lc);
	const char *route=NULL;
	if (proxy!=NULL) {
		route=linphone_proxy_config_get_route(proxy);
	}
	return route;
}

LinphoneCall * linphone_core_start_refered_call(LinphoneCore *lc, LinphoneCall *call, const LinphoneCallParams *params) {
	shared_ptr<LinphonePrivate::Call> referredCall = L_GET_PRIVATE_FROM_C_OBJECT(call)->startReferredCall(params
		? L_GET_CPP_PTR_FROM_C_OBJECT(params) : nullptr);
	return L_GET_C_BACK_PTR(referredCall);
}

/*
	 returns the ideal route set for making an operation through this proxy.
	 The list must be freed as well as the SalAddress content

	 rfc3608
	 6.1.  Procedures at the UA

	 /.../
	 For example, some devices will use locally-configured
	 explicit loose routing to reach a next-hop proxy, and others will use
	 a default outbound-proxy routing rule.  However, for the result to
	 function, the combination MUST provide valid routing in the local
	 environment.  In general, the service route set is appended to any
	 locally configured route needed to egress the access proxy chain.
	 Systems designers must match the service routing policy of their
	 nodes with the basic SIP routing policy in order to get a workable
	 system.
*/
static bctbx_list_t *make_routes_for_proxy(LinphoneProxyConfig *proxy, const LinphoneAddress *dest){
	bctbx_list_t *ret = NULL;
	const bctbx_list_t *proxy_routes = linphone_proxy_config_get_routes(proxy);
	bctbx_list_t *proxy_routes_iterator = (bctbx_list_t *)proxy_routes;
	const LinphoneAddress *srv_route = linphone_proxy_config_get_service_route(proxy);
	while (proxy_routes_iterator) {
		const char *local_route = (const char *)bctbx_list_get_data(proxy_routes_iterator);
		if (local_route) {
			ret = bctbx_list_append(ret, sal_address_new(local_route));
		}
		proxy_routes_iterator = bctbx_list_next(proxy_routes_iterator);
	}
	if (srv_route){
		ret=bctbx_list_append(ret,sal_address_clone(L_GET_PRIVATE_FROM_C_OBJECT(srv_route)->getInternalAddress()));
	}
	if (ret==NULL){
		/*if the proxy address matches the domain part of the destination, then use the same transport
		 * as the one used for registration. This is done by forcing a route to this proxy.*/
		SalAddress *proxy_addr=sal_address_new(linphone_proxy_config_get_addr(proxy));
		if (strcmp(sal_address_get_domain(proxy_addr),linphone_address_get_domain(dest))==0){
			ret=bctbx_list_append(ret,proxy_addr);
		}else sal_address_unref(proxy_addr);
	}
	return ret;
}

/*
 * Returns a proxy config matching the given identity address
 * Prefers registered, then first registering matching, otherwise first matching
 */
LinphoneProxyConfig * linphone_core_lookup_proxy_by_identity(LinphoneCore *lc, const LinphoneAddress *uri){
	LinphoneProxyConfig *found_cfg = NULL;
	LinphoneProxyConfig *found_reg_cfg = NULL;
	LinphoneProxyConfig *found_noreg_cfg = NULL;
	LinphoneProxyConfig *default_cfg=lc->default_proxy;
	const bctbx_list_t *elem;

	for (elem=linphone_core_get_proxy_config_list(lc);elem!=NULL;elem=elem->next){
		LinphoneProxyConfig *cfg = (LinphoneProxyConfig*)elem->data;
		if (linphone_address_weak_equal(uri, linphone_proxy_config_get_identity_address(cfg))) {
			if (linphone_proxy_config_get_state(cfg) == LinphoneRegistrationOk) {
				found_cfg=cfg;
				break;
			} else if (!found_reg_cfg && linphone_proxy_config_register_enabled(cfg)) {
				found_reg_cfg=cfg;
			} else if (!found_noreg_cfg) {
				found_noreg_cfg=cfg;
			}
		}
	}
	if (!found_cfg && found_reg_cfg)    found_cfg = found_reg_cfg;
	else if (!found_cfg && found_noreg_cfg) found_cfg = found_noreg_cfg;
	if (!found_cfg) found_cfg=default_cfg; /*when no matching proxy config is found, use the default proxy config*/
	return found_cfg;
}

LinphoneProxyConfig * linphone_core_lookup_known_proxy(LinphoneCore *lc, const LinphoneAddress *uri){
	const bctbx_list_t *elem;
	LinphoneProxyConfig *found_cfg=NULL;
	LinphoneProxyConfig *found_reg_cfg=NULL;
	LinphoneProxyConfig *found_noreg_cfg=NULL;
	LinphoneProxyConfig *default_cfg=lc->default_proxy;

	if (linphone_address_get_domain(uri) == NULL) {
		ms_message("Cannot look for proxy for uri [%p] that has no domain set, returning default", uri);
		return default_cfg;
	}
	/*return default proxy if it is matching the destination uri*/
	if (default_cfg){
		const char *domain=linphone_proxy_config_get_domain(default_cfg);
		if (domain && !strcmp(domain,linphone_address_get_domain(uri))){
			found_cfg=default_cfg;
			goto end;
		}
	}

	/*otherwise return first registered, then first registering matching, otherwise first matching */
	for (elem=linphone_core_get_proxy_config_list(lc);elem!=NULL;elem=elem->next){
		LinphoneProxyConfig *cfg=(LinphoneProxyConfig*)elem->data;
		const char *domain=linphone_proxy_config_get_domain(cfg);
		if (domain!=NULL && strcmp(domain,linphone_address_get_domain(uri))==0){
			if (linphone_proxy_config_get_state(cfg) == LinphoneRegistrationOk ){
				found_cfg=cfg;
				break;
			} else if (!found_reg_cfg && linphone_proxy_config_register_enabled(cfg)) {
				found_reg_cfg=cfg;
			} else if (!found_noreg_cfg){
				found_noreg_cfg=cfg;
			}
		}
	}
end:
	if     ( !found_cfg && found_reg_cfg)    found_cfg = found_reg_cfg;
	else if( !found_cfg && found_noreg_cfg ) found_cfg = found_noreg_cfg;

	if (found_cfg && found_cfg!=default_cfg){
		ms_debug("Overriding default proxy setting for this call/message/subscribe operation.");
	}else if (!found_cfg) found_cfg=default_cfg; /*when no matching proxy config is found, use the default proxy config*/
	return found_cfg;
}

const char *linphone_core_find_best_identity(LinphoneCore *lc, const LinphoneAddress *to){
	LinphoneProxyConfig *cfg=linphone_core_lookup_known_proxy(lc,to);
	if (cfg!=NULL){
		return linphone_proxy_config_get_identity (cfg);
	}
	return linphone_core_get_primary_contact(lc);
}

LinphoneCall * linphone_core_invite(LinphoneCore *lc, const char *url){
	LinphoneCall *call;
	LinphoneCallParams *p=linphone_core_create_call_params(lc, NULL);
	linphone_call_params_enable_video(p, linphone_call_params_video_enabled(p) && !!lc->video_policy.automatically_initiate);
	call=linphone_core_invite_with_params(lc,url,p);
	linphone_call_params_unref(p);
	return call;
}

LinphoneCall * linphone_core_invite_with_params(LinphoneCore *lc, const char *url, const LinphoneCallParams *p){
	LinphoneAddress *addr=linphone_core_interpret_url(lc,url);
	if (addr){
		LinphoneCall *call;
		call=linphone_core_invite_address_with_params(lc,addr,p);
		linphone_address_unref(addr);
		return call;
	}
	return NULL;
}

LinphoneCall * linphone_core_invite_address(LinphoneCore *lc, const LinphoneAddress *addr){
	LinphoneCall *call;
	LinphoneCallParams *p=linphone_core_create_call_params(lc, NULL);
	linphone_call_params_enable_video(p, linphone_call_params_video_enabled(p) && !!lc->video_policy.automatically_initiate);
	call=linphone_core_invite_address_with_params (lc,addr,p);
	linphone_call_params_unref(p);
	return call;
}

static void linphone_transfer_routes_to_op(bctbx_list_t *routes, SalOp *op){
	bctbx_list_t *it;
	for(it=routes;it!=NULL;it=it->next){
		SalAddress *addr=(SalAddress*)it->data;
		op->addRouteAddress(addr);
		sal_address_unref(addr);
	}
	bctbx_list_free(routes);
}

void linphone_configure_op_with_proxy(LinphoneCore *lc, SalOp *op, const LinphoneAddress *dest, SalCustomHeader *headers, bool_t with_contact, LinphoneProxyConfig *proxy){
	bctbx_list_t *routes=NULL;
	const char *identity;

	if (proxy){
		identity=linphone_proxy_config_get_identity(proxy);
		if (linphone_proxy_config_get_privacy(proxy)!=LinphonePrivacyDefault) {
			op->setPrivacy(linphone_proxy_config_get_privacy(proxy));
		}
	}else identity=linphone_core_get_primary_contact(lc);
	/*sending out of calls*/
	if (proxy){
		routes=make_routes_for_proxy(proxy,dest);
		linphone_transfer_routes_to_op(routes,op);
	}

	op->setToAddress(L_GET_PRIVATE_FROM_C_OBJECT(dest)->getInternalAddress());
	op->setFrom(identity);
	op->setSentCustomHeaders(headers);
	op->setRealm(L_C_TO_STRING(linphone_proxy_config_get_realm(proxy)));

	if (with_contact && proxy && proxy->op){
		const LinphoneAddress *contact = linphone_proxy_config_get_contact(proxy);
		SalAddress *salAddress = nullptr;
		if (contact)
			salAddress = sal_address_clone(const_cast<SalAddress *>(L_GET_PRIVATE_FROM_C_OBJECT(contact)->getInternalAddress()));
		op->setContactAddress(salAddress);
		if (salAddress)
			sal_address_unref(salAddress);
	}
	op->enableCnxIpTo0000IfSendOnly(!!lp_config_get_default_int(lc->config,"sip","cnx_ip_to_0000_if_sendonly_enabled",0)); /*also set in linphone_call_new_incoming*/
}

void linphone_configure_op(LinphoneCore *lc, SalOp *op, const LinphoneAddress *dest, SalCustomHeader *headers, bool_t with_contact) {
	linphone_configure_op_with_proxy(lc, op, dest, headers, with_contact, linphone_core_lookup_known_proxy(lc, dest));
}

void linphone_configure_op_2(LinphoneCore *lc, SalOp *op, const LinphoneAddress *local, const LinphoneAddress *dest, SalCustomHeader *headers, bool_t with_contact) {
	linphone_configure_op_with_proxy(lc, op, dest, headers, with_contact, linphone_core_lookup_proxy_by_identity(lc, local));
}

LinphoneCall * linphone_core_invite_address_with_params(LinphoneCore *lc, const LinphoneAddress *addr, const LinphoneCallParams *params){
	const char *from=NULL;
	LinphoneProxyConfig *proxy=NULL;
	LinphoneAddress *parsed_url2=NULL;
	LinphoneCall *call;
	LinphoneCallParams *cp;

	if (!addr) {
		ms_error("Can't invite a NULL address");
		return NULL;
	}

	if (!(!linphone_call_params_audio_enabled(params) ||
		linphone_call_params_get_audio_direction(params) == LinphoneMediaDirectionInactive ||
		linphone_call_params_get_local_conference_mode(params) == TRUE
		)
		&& linphone_core_preempt_sound_resources(lc) == -1) {
		ms_error("linphone_core_invite_address_with_params(): sound is required for this call but another call is already locking the sound resource. Call attempt is rejected.");
		return NULL;
	}

	if (!L_GET_PRIVATE_FROM_C_OBJECT(lc)->canWeAddCall())
		return NULL;

	cp = linphone_call_params_copy(params);
	proxy=linphone_core_lookup_known_proxy(lc,addr);
	if (proxy!=NULL) {
		from=linphone_proxy_config_get_identity(proxy);
		linphone_call_params_enable_avpf(cp, linphone_proxy_config_avpf_enabled(proxy));
		linphone_call_params_set_avpf_rr_interval(cp, (uint16_t)(linphone_proxy_config_get_avpf_rr_interval(proxy) * 1000));
	}else{
		linphone_call_params_enable_avpf(cp, linphone_core_get_avpf_mode(lc)==LinphoneAVPFEnabled);
		if (linphone_call_params_avpf_enabled(cp))
			linphone_call_params_set_avpf_rr_interval(cp, (uint16_t)(linphone_core_get_avpf_rr_interval(lc) * 1000));
	}

	/* if no proxy or no identity defined for this proxy, default to primary contact*/
	if (from==NULL) from=linphone_core_get_primary_contact(lc);

	parsed_url2=linphone_address_new(from);
	call=linphone_call_new_outgoing(lc,parsed_url2,addr,cp,proxy);
	linphone_address_unref(parsed_url2);

	if (L_GET_PRIVATE_FROM_C_OBJECT(lc)->addCall(L_GET_CPP_PTR_FROM_C_OBJECT(call)) != 0) {
		ms_warning("we had a problem in adding the call into the invite ... weird");
		linphone_call_unref(call);
		linphone_call_params_unref(cp);
		return NULL;
	}

	/* Unless this call is for a conference, it becomes now the current one*/
	if (linphone_call_params_get_local_conference_mode(params) ==  FALSE)
		L_GET_PRIVATE_FROM_C_OBJECT(lc)->setCurrentCall(L_GET_CPP_PTR_FROM_C_OBJECT(call));
	bool defer = L_GET_PRIVATE_FROM_C_OBJECT(call)->initiateOutgoing();
	if (!defer) {
		if (L_GET_PRIVATE_FROM_C_OBJECT(call)->startInvite(nullptr) != 0) {
			/* The call has already gone to error and released state, so do not return it */
			call = nullptr;
		}
	}

	linphone_call_params_unref(cp);
	return call;
}

LinphoneStatus linphone_core_transfer_call(LinphoneCore *lc, LinphoneCall *call, const char *url) {
	if (call == NULL) {
		ms_warning("No established call to refer.");
		return -1;
	}
	return linphone_call_transfer(call, url);
}

LinphoneStatus linphone_core_transfer_call_to_another(LinphoneCore *lc, LinphoneCall *call, LinphoneCall *dest) {
	return linphone_call_transfer_to_another(call, dest);
}

bool_t linphone_core_is_incoming_invite_pending(LinphoneCore*lc) {
	LinphoneCall *call = linphone_core_get_current_call(lc);
	if (call) {
		if ((linphone_call_get_dir(call) == LinphoneCallIncoming)
			&& ((linphone_call_get_state(call) == LinphoneCallIncomingReceived) || (linphone_call_get_state(call) ==  LinphoneCallIncomingEarlyMedia)))
			return TRUE;
	}
	return FALSE;
}

bool_t linphone_core_incompatible_security(LinphoneCore *lc, SalMediaDescription *md){
	return linphone_core_is_media_encryption_mandatory(lc) && linphone_core_get_media_encryption(lc)==LinphoneMediaEncryptionSRTP && !sal_media_description_has_srtp(md);
}

void linphone_core_notify_incoming_call(LinphoneCore *lc, LinphoneCall *call){
	/* Play the ring if this is the only call*/
	if (linphone_core_get_calls_nb(lc)==1){
		MSSndCard *ringcard=lc->sound_conf.lsd_card ?lc->sound_conf.lsd_card : lc->sound_conf.ring_sndcard;
		L_GET_PRIVATE_FROM_C_OBJECT(lc)->setCurrentCall(L_GET_CPP_PTR_FROM_C_OBJECT(call));
		if (lc->ringstream && lc->dmfs_playing_start_time!=0){
			linphone_core_stop_dtmf_stream(lc);
		}
		if (ringcard) {
			ms_snd_card_set_stream_type(ringcard, MS_SND_CARD_STREAM_RING);
			linphone_ringtoneplayer_start(lc->factory, lc->ringtoneplayer, ringcard, lc->sound_conf.local_ring, 2000);
		}
	}else{
		/* else play a tone within the context of the current call */
		L_GET_PRIVATE_FROM_C_OBJECT(call)->setRingingBeep(true);
		linphone_core_play_named_tone(lc,LinphoneToneCallWaiting);
	}
}

LinphoneStatus linphone_core_accept_early_media_with_params(LinphoneCore *lc, LinphoneCall *call, const LinphoneCallParams *params) {
	return linphone_call_accept_early_media_with_params(call, params);
}

LinphoneStatus linphone_core_accept_early_media(LinphoneCore *lc, LinphoneCall *call) {
	return linphone_call_accept_early_media(call);
}

LinphoneStatus linphone_core_update_call(LinphoneCore *lc, LinphoneCall *call, const LinphoneCallParams *params) {
	return linphone_call_update(call, params);
}

LinphoneStatus linphone_core_defer_call_update(LinphoneCore *lc, LinphoneCall *call) {
	return linphone_call_defer_update(call);
}

LinphoneStatus linphone_core_accept_call_update(LinphoneCore *lc, LinphoneCall *call, const LinphoneCallParams *params) {
	return linphone_call_accept_update(call, params);
}

static LinphoneCall * get_unique_call(LinphoneCore *lc) {
	LinphoneCall *call = linphone_core_get_current_call(lc);
	if ((call == NULL) && (linphone_core_get_calls_nb(lc) == 1)) {
		call = (LinphoneCall *)bctbx_list_get_data(linphone_core_get_calls(lc));
	}
	return call;
}

static LinphoneStatus _linphone_core_accept_call_with_params(LinphoneCore *lc, LinphoneCall *call, const LinphoneCallParams *params) {
	if (call == NULL) {
		call = get_unique_call(lc);
		if (call == NULL) {
			ms_warning("No unique call to accept!");
			return -1;
		}
	}
	return linphone_call_accept_with_params(call, params);
}

LinphoneStatus linphone_core_accept_call(LinphoneCore *lc, LinphoneCall *call) {
	return _linphone_core_accept_call_with_params(lc, call, NULL);
}

LinphoneStatus linphone_core_accept_call_with_params(LinphoneCore *lc, LinphoneCall *call, const LinphoneCallParams *params) {
	return _linphone_core_accept_call_with_params(lc, call, params);
}

LinphoneStatus linphone_core_redirect_call(LinphoneCore *lc, LinphoneCall *call, const char *redirect_uri) {
	return linphone_call_redirect(call, redirect_uri);
}

LinphoneStatus linphone_core_terminate_call(LinphoneCore *lc, LinphoneCall *call) {
	if (call == NULL) {
		call = get_unique_call(lc);
		if (call == NULL) {
			ms_warning("No unique call to terminate!");
			return -1;
		}
	}
	return linphone_call_terminate(call);
}

LinphoneStatus linphone_core_terminate_all_calls(LinphoneCore *lc) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(lc)->terminateAllCalls();
}

LinphoneStatus linphone_core_decline_call(LinphoneCore *lc, LinphoneCall *call, LinphoneReason reason) {
	return linphone_call_decline(call, reason);
}

const bctbx_list_t *linphone_core_get_calls(LinphoneCore *lc) {
	if (lc->callsCache) {
		bctbx_list_free_with_data(lc->callsCache, (bctbx_list_free_func)linphone_call_unref);
		lc->callsCache = NULL;
	}
	lc->callsCache = L_GET_RESOLVED_C_LIST_FROM_CPP_LIST(L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getCalls());
	return lc->callsCache;
}

bool_t linphone_core_in_call(const LinphoneCore *lc){
	return linphone_core_get_current_call((LinphoneCore *)lc)!=NULL || linphone_core_is_in_conference(lc);
}

LinphoneCall *linphone_core_get_current_call(const LinphoneCore *lc) {
	shared_ptr<LinphonePrivate::Call> call = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getCurrentCall();
	return call ? L_GET_C_BACK_PTR(call) : NULL;
}

LinphoneStatus linphone_core_pause_call(LinphoneCore *lc, LinphoneCall *call) {
	return linphone_call_pause(call);
}

LinphoneStatus linphone_core_pause_all_calls(LinphoneCore *lc) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(lc)->pauseAllCalls();
}

int linphone_core_preempt_sound_resources(LinphoneCore *lc){
	LinphoneCall *current_call;
	int err = 0;

	if (linphone_core_is_in_conference(lc)){
		linphone_core_leave_conference(lc);
		return 0;
	}

	current_call=linphone_core_get_current_call(lc);
	if(current_call != NULL){
		ms_message("Pausing automatically the current call.");
		err = L_GET_CPP_PTR_FROM_C_OBJECT(current_call)->pause();
	}
	if (lc->ringstream){
		linphone_core_stop_ringing(lc);
	}
	return err;
}

LinphoneStatus linphone_core_resume_call(LinphoneCore *lc, LinphoneCall *call) {
	return linphone_call_resume(call);
}

LinphoneCall *linphone_core_get_call_by_remote_address(const LinphoneCore *lc, const char *remote_address){
	LinphoneCall *call=NULL;
	LinphoneAddress *raddr=linphone_address_new(remote_address);
	if (raddr) {
		call=linphone_core_get_call_by_remote_address2(lc, raddr);
		linphone_address_unref(raddr);
	}
	return call;
}

LinphoneCall *linphone_core_find_call_from_uri(const LinphoneCore *lc, const char *remote_address){
	return linphone_core_get_call_by_remote_address(lc, remote_address);
}

LinphoneCall *linphone_core_get_call_by_remote_address2(const LinphoneCore *lc, const LinphoneAddress *raddr) {
	shared_ptr<LinphonePrivate::Call> call = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getCallByRemoteAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(raddr));
	return call ? L_GET_C_BACK_PTR(call) : NULL;
}

int linphone_core_send_publish(LinphoneCore *lc, LinphonePresenceModel *presence) {
	const bctbx_list_t *elem;
	for (elem=linphone_core_get_proxy_config_list(lc);elem!=NULL;elem=bctbx_list_next(elem)){
		LinphoneProxyConfig *cfg=(LinphoneProxyConfig*)elem->data;
		if (cfg->publish) linphone_proxy_config_send_publish(cfg,presence);
	}
	return 0;
}

void linphone_core_set_inc_timeout(LinphoneCore *lc, int seconds){
	lc->sip_conf.inc_timeout=seconds;
	if (linphone_core_ready(lc)){
		lp_config_set_int(lc->config,"sip","inc_timeout",seconds);
	}
}

int linphone_core_get_inc_timeout(LinphoneCore *lc){
	return lc->sip_conf.inc_timeout;
}

void linphone_core_set_in_call_timeout(LinphoneCore *lc, int seconds){
	lc->sip_conf.in_call_timeout=seconds;
	if( linphone_core_ready(lc)){
		lp_config_set_int(lc->config, "sip", "in_call_timeout", seconds);
	}
}

int linphone_core_get_in_call_timeout(LinphoneCore *lc){
	return lc->sip_conf.in_call_timeout;
}

int linphone_core_get_delayed_timeout(LinphoneCore *lc){
	return lc->sip_conf.delayed_timeout;
}

void linphone_core_set_delayed_timeout(LinphoneCore *lc, int seconds){
	lc->sip_conf.delayed_timeout=seconds;
}

int linphone_core_get_max_size_for_auto_download_incoming_files(LinphoneCore *lc) {
	return lc->auto_download_incoming_files_max_size;
}

void linphone_core_set_max_size_for_auto_download_incoming_files(LinphoneCore *lc, int size) {
	lc->auto_download_incoming_files_max_size = size;
	lp_config_set_int(lc->config, "app", "auto_download_incoming_files_max_size", size);
}

void linphone_core_set_presence_info(LinphoneCore *lc, int minutes_away, const char *contact, LinphoneOnlineStatus os) {
	LinphonePresenceModel *presence = NULL;
	LinphonePresenceActivity *activity = NULL;
	const char *description = NULL;
	LinphonePresenceActivityType acttype = LinphonePresenceActivityUnknown;

	if (minutes_away>0) lc->minutes_away=minutes_away;

	presence = linphone_presence_model_new();
	linphone_presence_model_set_basic_status(presence, LinphonePresenceBasicStatusOpen);
	switch (os) {
		case LinphoneStatusOffline:
			linphone_presence_model_set_basic_status(presence, LinphonePresenceBasicStatusClosed);
			goto end;
		case LinphoneStatusOnline:
			goto end;
		case LinphoneStatusBusy:
			acttype = LinphonePresenceActivityBusy;
			break;
		case LinphoneStatusBeRightBack:
			acttype = LinphonePresenceActivityInTransit;
			break;
		case LinphoneStatusAway:
			acttype = LinphonePresenceActivityAway;
			break;
		case LinphoneStatusOnThePhone:
			acttype = LinphonePresenceActivityOnThePhone;
			break;
		case LinphoneStatusOutToLunch:
			acttype = LinphonePresenceActivityLunch;
			break;
		case LinphoneStatusDoNotDisturb:
			acttype = LinphonePresenceActivityBusy;
			description = "Do not disturb";
			linphone_presence_model_set_basic_status(presence, LinphonePresenceBasicStatusClosed);
			break;
		case LinphoneStatusMoved:
			acttype = LinphonePresenceActivityPermanentAbsence;
			break;
		case LinphoneStatusAltService:
			acttype = LinphonePresenceActivityBusy;
			description = "Using another messaging service";
			break;
		case LinphoneStatusPending:
			acttype = LinphonePresenceActivityOther;
			description = "Waiting for user acceptance";
			break;
		case LinphoneStatusVacation:
			acttype = LinphonePresenceActivityVacation;
			break;
		case LinphoneStatusEnd:
			ms_warning("Invalid status LinphoneStatusEnd");
			return;
	}
	activity = linphone_presence_activity_new(acttype, description);
	linphone_presence_model_add_activity(presence, activity);
	linphone_presence_activity_unref(activity);

end:
	linphone_presence_model_set_contact(presence, contact);
	linphone_core_set_presence_model(lc, presence);
	linphone_presence_model_unref(presence);
}

void linphone_core_send_presence(LinphoneCore *lc, LinphonePresenceModel *presence){
	linphone_core_notify_all_friends(lc,presence);
	linphone_core_send_publish(lc,presence);
}

void linphone_core_set_presence_model(LinphoneCore *lc, LinphonePresenceModel *presence) {
	linphone_presence_model_ref(presence);
	linphone_core_send_presence(lc, presence);
	if (lc->presence_model != NULL) linphone_presence_model_unref(lc->presence_model);
	lc->presence_model = presence;
}

LinphoneOnlineStatus linphone_core_get_presence_info(const LinphoneCore *lc){
	LinphonePresenceActivity *activity = NULL;
	const char *description = NULL;

	activity = linphone_presence_model_get_activity(lc->presence_model);
	if (activity) {
		description = linphone_presence_activity_get_description(activity);
		switch (linphone_presence_activity_get_type(activity)) {
			case LinphonePresenceActivityBusy:
				if (description != NULL) {
					if (strcmp(description, "Do not disturb") == 0)
						return LinphoneStatusDoNotDisturb;
					else if (strcmp(description, "Using another messaging service") == 0)
						return LinphoneStatusAltService;
				}
				return LinphoneStatusBusy;
			case LinphonePresenceActivityInTransit:
			case LinphonePresenceActivitySteering:
				return LinphoneStatusBeRightBack;
			case LinphonePresenceActivityAway:
				return LinphoneStatusAway;
			case LinphonePresenceActivityOnThePhone:
				return LinphoneStatusOnThePhone;
			case LinphonePresenceActivityBreakfast:
			case LinphonePresenceActivityDinner:
			case LinphonePresenceActivityLunch:
			case LinphonePresenceActivityMeal:
				return LinphoneStatusOutToLunch;
			case LinphonePresenceActivityPermanentAbsence:
				return LinphoneStatusMoved;
			case LinphonePresenceActivityOther:
				if (description != NULL) {
					if (strcmp(description, "Waiting for user acceptance") == 0)
						return LinphoneStatusPending;
				}
				return LinphoneStatusBusy;
			case LinphonePresenceActivityVacation:
				return LinphoneStatusVacation;
			case LinphonePresenceActivityAppointment:
			case LinphonePresenceActivityMeeting:
			case LinphonePresenceActivityWorship:
				return LinphoneStatusDoNotDisturb;
			default:
				return LinphoneStatusBusy;
		}
	} else {
		if (linphone_presence_model_get_basic_status(lc->presence_model) == LinphonePresenceBasicStatusOpen)
			return LinphoneStatusOnline;
		else
			return LinphoneStatusOffline;
	}
}

LinphonePresenceModel * linphone_core_get_presence_model(const LinphoneCore *lc) {
	return lc->presence_model;
}

LinphoneConsolidatedPresence linphone_core_get_consolidated_presence(const LinphoneCore *lc) {
	LinphoneProxyConfig *cfg = lc->default_proxy;
	LinphonePresenceModel *model = linphone_core_get_presence_model(lc);

	return ((cfg && !linphone_proxy_config_publish_enabled(cfg)) || !model)
		? LinphoneConsolidatedPresenceOffline
		: linphone_presence_model_get_consolidated_presence(model);
}

void linphone_core_set_consolidated_presence(LinphoneCore *lc, LinphoneConsolidatedPresence presence) {
	const bctbx_list_t *cfg_list;
	const bctbx_list_t *item;
	LinphoneProxyConfig *cfg;
	LinphonePresenceModel *model;
	LinphonePresenceActivity *activity = NULL;

	cfg_list = linphone_core_get_proxy_config_list(lc);
	for (item = cfg_list; item != NULL; item = bctbx_list_next(item)) {
		cfg = (LinphoneProxyConfig *)bctbx_list_get_data(item);
		if ((cfg != NULL) && (presence == LinphoneConsolidatedPresenceOffline) && linphone_proxy_config_publish_enabled(cfg)) {
			/* Unpublish when going offline before changing the presence model. */
			linphone_proxy_config_edit(cfg);
			linphone_proxy_config_enable_publish(cfg, FALSE);
			linphone_proxy_config_done(cfg);
		}
	}
	model = linphone_presence_model_new();
	switch (presence) {
		case LinphoneConsolidatedPresenceOnline:
			linphone_presence_model_set_basic_status(model, LinphonePresenceBasicStatusOpen);
			break;
		case LinphoneConsolidatedPresenceBusy:
			linphone_presence_model_set_basic_status(model, LinphonePresenceBasicStatusOpen);
			activity = linphone_presence_activity_new(LinphonePresenceActivityAway, NULL);
			break;
		case LinphoneConsolidatedPresenceDoNotDisturb:
			linphone_presence_model_set_basic_status(model, LinphonePresenceBasicStatusClosed);
			activity = linphone_presence_activity_new(LinphonePresenceActivityAway, NULL);
			break;
		case LinphoneConsolidatedPresenceOffline:
		default:
			linphone_presence_model_set_basic_status(model, LinphonePresenceBasicStatusClosed);
			break;
	}
	if (activity != NULL) linphone_presence_model_add_activity(model, activity);
	linphone_core_set_presence_model(lc, model);
	linphone_presence_model_unref(model);
	for (item = cfg_list; item != NULL; item = bctbx_list_next(item)) {
		cfg = (LinphoneProxyConfig *)bctbx_list_get_data(item);
		if ((cfg != NULL) && (presence != LinphoneConsolidatedPresenceOffline) && !linphone_proxy_config_publish_enabled(cfg)) {
			/* When going online or busy, publish after changing the presence model. */
			linphone_proxy_config_edit(cfg);
			linphone_proxy_config_enable_publish(cfg, TRUE);
			linphone_proxy_config_done(cfg);
		}
	}
}

int linphone_core_get_play_level(LinphoneCore *lc) {
	return lc->sound_conf.play_lev;
}

int linphone_core_get_ring_level(LinphoneCore *lc) {
	return lc->sound_conf.ring_lev;
}

int linphone_core_get_rec_level(LinphoneCore *lc) {
	return lc->sound_conf.rec_lev;
}

int linphone_core_get_media_level(LinphoneCore *lc) {
	return lc->sound_conf.media_lev;
}

void linphone_core_set_ring_level(LinphoneCore *lc, int level){
	MSSndCard *sndcard;
	lc->sound_conf.ring_lev = (char)level;
	sndcard=lc->sound_conf.ring_sndcard;
	if (sndcard) ms_snd_card_set_level(sndcard,MS_SND_CARD_PLAYBACK,level);
}

void linphone_core_set_mic_gain_db (LinphoneCore *lc, float gaindb){
	float gain=gaindb;
	LinphoneCall *call=linphone_core_get_current_call (lc);
	AudioStream *st;

	lc->sound_conf.soft_mic_lev=gaindb;

	if (linphone_core_ready(lc)){
		lp_config_set_float(lc->config,"sound","mic_gain_db",lc->sound_conf.soft_mic_lev);
	}

	if (!call || !(st = reinterpret_cast<AudioStream *>(linphone_call_get_stream(call, LinphoneStreamTypeAudio)))) {
		ms_message("linphone_core_set_mic_gain_db(): no active call.");
		return;
	}
	audio_stream_set_mic_gain_db(st,gain);
}

float linphone_core_get_mic_gain_db(LinphoneCore *lc) {
	return lc->sound_conf.soft_mic_lev;
}

void linphone_core_set_playback_gain_db (LinphoneCore *lc, float gaindb){
	float gain=gaindb;
	LinphoneCall *call=linphone_core_get_current_call (lc);
	AudioStream *st;

	lc->sound_conf.soft_play_lev=gaindb;
	if (linphone_core_ready(lc)){
		lp_config_set_float(lc->config,"sound","playback_gain_db",lc->sound_conf.soft_play_lev);
	}

	if (!call || !(st = reinterpret_cast<AudioStream *>(linphone_call_get_stream(call, LinphoneStreamTypeAudio)))) {
		ms_message("linphone_core_set_playback_gain_db(): no active call.");
		return;
	}
	set_playback_gain_db(st,gain);
}

float linphone_core_get_playback_gain_db(LinphoneCore *lc) {
	return lc->sound_conf.soft_play_lev;
}

void linphone_core_set_play_level(LinphoneCore *lc, int level){
	MSSndCard *sndcard;
	lc->sound_conf.play_lev = (char)level;
	sndcard=lc->sound_conf.play_sndcard;
	if (sndcard) ms_snd_card_set_level(sndcard,MS_SND_CARD_PLAYBACK,level);
}

void linphone_core_set_rec_level(LinphoneCore *lc, int level) {
	MSSndCard *sndcard;
	lc->sound_conf.rec_lev = (char)level;
	sndcard=lc->sound_conf.capt_sndcard;
	if (sndcard) ms_snd_card_set_level(sndcard,MS_SND_CARD_CAPTURE,level);
}

void linphone_core_set_media_level(LinphoneCore *lc, int level) {
	MSSndCard *sndcard;
	lc->sound_conf.media_lev = (char)level;
	sndcard=lc->sound_conf.media_sndcard;
	if (sndcard) ms_snd_card_set_level(sndcard,MS_SND_CARD_PLAYBACK,level);
}

static MSSndCard *get_card_from_string_id(const char *devid, unsigned int cap, MSFactory *f){
	MSSndCard *sndcard=NULL;
	if (devid!=NULL){
		sndcard=ms_snd_card_manager_get_card(ms_factory_get_snd_card_manager(f),devid);
		if (sndcard!=NULL &&
			(ms_snd_card_get_capabilities(sndcard) & cap)==0 ){
			ms_warning("%s card does not have the %s capability, ignoring.",
				devid,
				cap==MS_SND_CARD_CAP_CAPTURE ? "capture" : "playback");
			sndcard=NULL;
		}
	}
	if (sndcard==NULL) {
		if ((cap & MS_SND_CARD_CAP_CAPTURE) && (cap & MS_SND_CARD_CAP_PLAYBACK)){
			sndcard=ms_snd_card_manager_get_default_card(ms_factory_get_snd_card_manager(f));
		}else if (cap & MS_SND_CARD_CAP_CAPTURE){
			sndcard=ms_snd_card_manager_get_default_capture_card(ms_factory_get_snd_card_manager(f));
		}
		else if (cap & MS_SND_CARD_CAP_PLAYBACK){
			sndcard=ms_snd_card_manager_get_default_playback_card(ms_factory_get_snd_card_manager(f));
		}
		if (sndcard==NULL){/*looks like a bug! take the first one !*/
			const bctbx_list_t *elem=ms_snd_card_manager_get_list(ms_factory_get_snd_card_manager(f));
			if (elem) sndcard=(MSSndCard*)elem->data;
		}
	}
	if (sndcard==NULL) ms_error("Could not find a suitable soundcard !");
	return sndcard;
}

bool_t linphone_core_sound_device_can_capture(LinphoneCore *lc, const char *devid){
	return ms_snd_card_manager_get_capture_card(ms_factory_get_snd_card_manager(lc->factory),devid) != NULL;
}

bool_t linphone_core_sound_device_can_playback(LinphoneCore *lc, const char *devid){
	return ms_snd_card_manager_get_playback_card(ms_factory_get_snd_card_manager(lc->factory),devid) != NULL;
}

LinphoneStatus linphone_core_set_ringer_device(LinphoneCore *lc, const char * devid){
	MSSndCard *card=get_card_from_string_id(devid,MS_SND_CARD_CAP_PLAYBACK, lc->factory);
	lc->sound_conf.ring_sndcard=card;
	if (card && linphone_core_ready(lc))
		lp_config_set_string(lc->config,"sound","ringer_dev_id",ms_snd_card_get_string_id(card));
	return 0;
}

LinphoneStatus linphone_core_set_playback_device(LinphoneCore *lc, const char * devid){
	MSSndCard *card=get_card_from_string_id(devid,MS_SND_CARD_CAP_PLAYBACK, lc->factory);
	lc->sound_conf.play_sndcard=card;
	if (card &&  linphone_core_ready(lc))
		lp_config_set_string(lc->config,"sound","playback_dev_id",ms_snd_card_get_string_id(card));
	return 0;
}

LinphoneStatus linphone_core_set_capture_device(LinphoneCore *lc, const char * devid){
	MSSndCard *card=get_card_from_string_id(devid,MS_SND_CARD_CAP_CAPTURE, lc->factory);
	lc->sound_conf.capt_sndcard=card;
	if (card &&  linphone_core_ready(lc))
		lp_config_set_string(lc->config,"sound","capture_dev_id",ms_snd_card_get_string_id(card));
	return 0;
}

LinphoneStatus linphone_core_set_media_device(LinphoneCore *lc, const char * devid){
	MSSndCard *card=get_card_from_string_id(devid,MS_SND_CARD_CAP_PLAYBACK, lc->factory);
	lc->sound_conf.media_sndcard=card;
	if (card &&  linphone_core_ready(lc))
		lp_config_set_string(lc->config,"sound","media_dev_id",ms_snd_card_get_string_id(card));
	return 0;
}

const char * linphone_core_get_ringer_device(LinphoneCore *lc) {
	if (lc->sound_conf.ring_sndcard) return ms_snd_card_get_string_id(lc->sound_conf.ring_sndcard);
	return NULL;
}

const char * linphone_core_get_playback_device(LinphoneCore *lc) {
	return lc->sound_conf.play_sndcard ? ms_snd_card_get_string_id(lc->sound_conf.play_sndcard) : NULL;
}

const char * linphone_core_get_capture_device(LinphoneCore *lc) {
	return lc->sound_conf.capt_sndcard ? ms_snd_card_get_string_id(lc->sound_conf.capt_sndcard) : NULL;
}

const char * linphone_core_get_media_device(LinphoneCore *lc) {
	return lc->sound_conf.media_sndcard ? ms_snd_card_get_string_id(lc->sound_conf.media_sndcard) : NULL;
}

const char**  linphone_core_get_sound_devices(LinphoneCore *lc){
	return lc->sound_conf.cards;
}

const char**  linphone_core_get_video_devices(const LinphoneCore *lc){
	return lc->video_conf.cams;
}

bctbx_list_t * linphone_core_get_sound_devices_list(const LinphoneCore *lc){
	bctbx_list_t *cards_list = NULL;
	const char** cards = lc->sound_conf.cards;
	for (const char* c = *cards; c; c=*++cards) {
		cards_list = bctbx_list_append(cards_list, (char *)c);
	}
	return cards_list;
}

bctbx_list_t * linphone_core_get_video_devices_list(const LinphoneCore *lc){
	bctbx_list_t *cards_list = NULL;
	const char** cards = lc->video_conf.cams;

	if (cards)
		for (const char* c = *cards; c; c=*++cards)
			cards_list = bctbx_list_append(cards_list, (char *)c);

	return cards_list;
}

void linphone_core_set_default_sound_devices(LinphoneCore *lc){
		linphone_core_set_ringer_device(lc, NULL);
		linphone_core_set_playback_device(lc, NULL);
		linphone_core_set_capture_device(lc, NULL);
}

void linphone_core_reload_sound_devices(LinphoneCore *lc){
	const char *ringer;
	const char *playback;
	const char *capture;
	char *ringer_copy = NULL;
	char *playback_copy = NULL;
	char *capture_copy = NULL;

	ringer = linphone_core_get_ringer_device(lc);
	if (ringer != NULL) {
		ringer_copy = ms_strdup(ringer);
	}
	playback = linphone_core_get_playback_device(lc);
	if (playback != NULL) {
		playback_copy = ms_strdup(playback);
	}
	capture = linphone_core_get_capture_device(lc);
	if (capture != NULL) {
		capture_copy = ms_strdup(capture);
	}
	ms_snd_card_manager_reload(ms_factory_get_snd_card_manager(lc->factory));
	build_sound_devices_table(lc);
	if (ringer_copy != NULL) {
		linphone_core_set_ringer_device(lc, ringer_copy);
		ms_free(ringer_copy);
	}
	if (playback_copy != NULL) {
		linphone_core_set_playback_device(lc, playback_copy);
		ms_free(playback_copy);
	}
	if (capture_copy != NULL) {
		linphone_core_set_capture_device(lc, capture_copy);
		ms_free(capture_copy);
	}
}

void linphone_core_reload_video_devices(LinphoneCore *lc){
	char *devid_copy = NULL;
	const char *devid = linphone_core_get_video_device(lc);
	if (devid != NULL) {
		devid_copy = ms_strdup(devid);
	}
	ms_web_cam_manager_reload(ms_factory_get_web_cam_manager(lc->factory));
	build_video_devices_table(lc);
	if (devid_copy != NULL) {
		linphone_core_set_video_device(lc, devid_copy);
		ms_free(devid_copy);
	}
}

char linphone_core_get_sound_source(LinphoneCore *lc) {
	return lc->sound_conf.source;
}

void linphone_core_set_sound_source(LinphoneCore *lc, char source) {
	MSSndCard *sndcard=lc->sound_conf.capt_sndcard;
	lc->sound_conf.source=source;
	if (!sndcard) return;
	switch(source){
		case 'm':
			ms_snd_card_set_capture(sndcard,MS_SND_CARD_MIC);
			break;
		case 'l':
			ms_snd_card_set_capture(sndcard,MS_SND_CARD_LINE);
			break;
	}

}


void linphone_core_set_ring(LinphoneCore *lc,const char *path){
	if (lc->sound_conf.local_ring!=0){
		ms_free(lc->sound_conf.local_ring);
		lc->sound_conf.local_ring=NULL;
	}
	if (path)
		lc->sound_conf.local_ring=ms_strdup(path);
	if ( linphone_core_ready(lc) && lc->sound_conf.local_ring)
		lp_config_set_string(lc->config,"sound","local_ring",lc->sound_conf.local_ring);
}

const char *linphone_core_get_ring(const LinphoneCore *lc){
	return lc->sound_conf.local_ring;
}

void linphone_core_set_root_ca(LinphoneCore *lc, const char *path) {
	lc->sal->setRootCa(L_C_TO_STRING(path));
	if (lc->http_crypto_config) {
		belle_tls_crypto_config_set_root_ca(lc->http_crypto_config, path);
	}
	lp_config_set_string(lc->config,"sip", "root_ca", path);
}

void linphone_core_set_root_ca_data(LinphoneCore *lc, const char *data) {
	lc->sal->setRootCa("");
	lc->sal->setRootCaData(L_C_TO_STRING(data));
	if (lc->http_crypto_config) {
		belle_tls_crypto_config_set_root_ca_data(lc->http_crypto_config, data);
	}
}

const char *linphone_core_get_root_ca(LinphoneCore *lc){
	return lp_config_get_string(lc->config,"sip","root_ca",NULL);
}

void linphone_core_verify_server_certificates(LinphoneCore *lc, bool_t yesno){
	lc->sal->verifyServerCertificates(!!yesno);
	if (lc->http_crypto_config){
		belle_tls_crypto_config_set_verify_exceptions(lc->http_crypto_config, yesno ? 0 : BELLE_TLS_VERIFY_ANY_REASON);
	}
	lp_config_set_int(lc->config,"sip","verify_server_certs",yesno);
}

void linphone_core_verify_server_cn(LinphoneCore *lc, bool_t yesno){
	lc->sal->verifyServerCn(!!yesno);
	if (lc->http_crypto_config){
		belle_tls_crypto_config_set_verify_exceptions(lc->http_crypto_config, yesno ? 0 : BELLE_TLS_VERIFY_CN_MISMATCH);
	}
	lp_config_set_int(lc->config,"sip","verify_server_cn",yesno);
}

void linphone_core_set_ssl_config(LinphoneCore *lc, void *ssl_config) {
	lc->sal->setSslConfig(ssl_config);
	if (lc->http_crypto_config) {
		belle_tls_crypto_config_set_ssl_config(lc->http_crypto_config, ssl_config);
	}
}

static void notify_end_of_ringtone( LinphoneRingtonePlayer* rp, void* user_data, int status) {
	LinphoneCore *lc=(LinphoneCore*)user_data;
	lc->preview_finished=1;
}

LinphoneStatus linphone_core_preview_ring(LinphoneCore *lc, const char *ring,LinphoneCoreCbFunc end_of_ringtone,void * userdata)
{
	int err;
	MSSndCard *ringcard=lc->sound_conf.lsd_card ? lc->sound_conf.lsd_card : lc->sound_conf.ring_sndcard;
	if (linphone_ringtoneplayer_is_started(lc->ringtoneplayer)){
		ms_warning("Cannot start ring now,there's already a ring being played");
		return -1;
	}
	lc_callback_obj_init(&lc->preview_finished_cb,end_of_ringtone,userdata);
	lc->preview_finished=0;
	err = linphone_ringtoneplayer_start_with_cb(lc->factory, lc->ringtoneplayer, ringcard, ring, -1, notify_end_of_ringtone,(void *)lc);
	if (err) {
		lc->preview_finished=1;
	}
	return err;
}

MSFactory *linphone_core_get_ms_factory(LinphoneCore *lc){
	return lc->factory;
}

void linphone_core_set_ringback(LinphoneCore *lc, const char *path){
	if (lc->sound_conf.remote_ring!=0){
		ms_free(lc->sound_conf.remote_ring);
	}
	lc->sound_conf.remote_ring=path?ms_strdup(path):NULL;
}

const char * linphone_core_get_ringback(const LinphoneCore *lc){
	return lc->sound_conf.remote_ring;
}

void linphone_core_enable_echo_cancellation(LinphoneCore *lc, bool_t val){
	lc->sound_conf.ec=val;
	if ( linphone_core_ready(lc))
		lp_config_set_int(lc->config,"sound","echocancellation",val);
}

bool_t linphone_core_echo_cancellation_enabled(const LinphoneCore *lc){
	return lc->sound_conf.ec;
}

void linphone_core_enable_echo_limiter(LinphoneCore *lc, bool_t val){
	lc->sound_conf.ea=val;
}

bool_t linphone_core_echo_limiter_enabled(const LinphoneCore *lc){
	return lc->sound_conf.ea;
}

static void linphone_core_mute_audio_stream(LinphoneCore *lc, AudioStream *st, bool_t val) {
	if (val) {
		audio_stream_set_mic_gain(st, 0);
	} else {
		audio_stream_set_mic_gain_db(st, lc->sound_conf.soft_mic_lev);
	}

	if ( linphone_core_get_rtp_no_xmit_on_audio_mute(lc) ){
		audio_stream_mute_rtp(st,val);
	}
}

void linphone_core_mute_mic(LinphoneCore *lc, bool_t val){
	linphone_core_enable_mic(lc, !val);
}

bool_t linphone_core_is_mic_muted(LinphoneCore *lc) {
	return !linphone_core_mic_enabled(lc);
}

void linphone_core_enable_mic(LinphoneCore *lc, bool_t enable) {
	LinphoneCall *call;
	const bctbx_list_t *list;
	const bctbx_list_t *elem;

	if (linphone_core_is_in_conference(lc)){
		linphone_conference_mute_microphone(lc->conf_ctx, !enable);
	}
	list = linphone_core_get_calls(lc);
	for (elem = list; elem != NULL; elem = elem->next) {
		call = (LinphoneCall *)elem->data;
		linphone_call_set_microphone_muted(call, !enable);
		AudioStream *astream = reinterpret_cast<AudioStream *>(linphone_call_get_stream(call, LinphoneStreamTypeAudio));
		if (astream)
			linphone_core_mute_audio_stream(lc, astream, linphone_call_get_microphone_muted(call));
	}
}

bool_t linphone_core_mic_enabled(LinphoneCore *lc) {
	LinphoneCall *call=linphone_core_get_current_call(lc);
	if (linphone_core_is_in_conference(lc)){
		return !linphone_conference_microphone_is_muted(lc->conf_ctx);
	}else if (call==NULL){
		ms_warning("%s(): No current call!", __FUNCTION__);
		return TRUE;
	}
	return !linphone_call_get_microphone_muted(call);
}

bool_t linphone_core_is_rtp_muted(LinphoneCore *lc){
	LinphoneCall *call=linphone_core_get_current_call(lc);
	if (call==NULL){
		ms_warning("linphone_core_is_rtp_muted(): No current call !");
		return FALSE;
	}
	if( linphone_core_get_rtp_no_xmit_on_audio_mute(lc)){
		return linphone_call_get_microphone_muted(call);
	}
	return FALSE;
}

void linphone_core_enable_agc(LinphoneCore *lc, bool_t val){
	lc->sound_conf.agc=val;
}

bool_t linphone_core_agc_enabled(const LinphoneCore *lc){
	return lc->sound_conf.agc;
}

void linphone_core_send_dtmf(LinphoneCore *lc, char dtmf) {
	LinphoneCall *call=linphone_core_get_current_call(lc);
	linphone_call_send_dtmf(call, dtmf);
}

void linphone_core_set_stun_server(LinphoneCore *lc, const char *server) {
	if (lc->nat_policy != NULL) {
		linphone_nat_policy_set_stun_server(lc->nat_policy, server);
		linphone_nat_policy_save_to_config(lc->nat_policy);
	} else {
		lp_config_set_string(lc->config, "net", "stun_server", server);
	}
}

const char * linphone_core_get_stun_server(const LinphoneCore *lc){
	if (lc->nat_policy != NULL)
		return linphone_nat_policy_get_stun_server(lc->nat_policy);
	else
		return lp_config_get_string(lc->config, "net", "stun_server", NULL);
}


bool_t linphone_core_upnp_available(){
	return FALSE;
}

LinphoneUpnpState linphone_core_get_upnp_state(const LinphoneCore *lc){
	return LinphoneUpnpStateNotAvailable;
}

const char * linphone_core_get_upnp_external_ipaddress(const LinphoneCore *lc){
	return NULL;
}

void linphone_core_set_nat_address(LinphoneCore *lc, const char *addr) {
	if (lc->net_conf.nat_address!=NULL){
		ms_free(lc->net_conf.nat_address);
	}
	if (addr!=NULL) lc->net_conf.nat_address=ms_strdup(addr);
	else lc->net_conf.nat_address=NULL;
	if (lc->sip_conf.contact) update_primary_contact(lc);
}

const char *linphone_core_get_nat_address(const LinphoneCore *lc) {
	return lc->net_conf.nat_address;
}

const char *linphone_core_get_nat_address_resolved(LinphoneCore *lc) {
	struct sockaddr_storage ss;
	socklen_t ss_len;
	int error;
	char ipstring [INET6_ADDRSTRLEN];

	if (lc->net_conf.nat_address==NULL) return NULL;

	if (parse_hostname_to_addr (lc->net_conf.nat_address, &ss, &ss_len, 5060)<0) {
		return lc->net_conf.nat_address;
	}

	error = bctbx_getnameinfo((struct sockaddr *)&ss, ss_len,
		ipstring, sizeof(ipstring), NULL, 0, NI_NUMERICHOST);
	if (error) {
		return lc->net_conf.nat_address;
	}

	if (lc->net_conf.nat_address_ip!=NULL){
		ms_free(lc->net_conf.nat_address_ip);
	}
	lc->net_conf.nat_address_ip = ms_strdup (ipstring);
	return lc->net_conf.nat_address_ip;
}

void linphone_core_set_firewall_policy(LinphoneCore *lc, LinphoneFirewallPolicy pol) {
	LinphoneNatPolicy *nat_policy;
	char *stun_server = NULL;
	char *stun_server_username = NULL;

	if (lc->nat_policy != NULL) {
		nat_policy = linphone_nat_policy_ref(lc->nat_policy);
		stun_server = ms_strdup(linphone_nat_policy_get_stun_server(nat_policy));
		stun_server_username = ms_strdup(linphone_nat_policy_get_stun_server_username(nat_policy));
		linphone_nat_policy_clear(nat_policy);
	} else {
		nat_policy = linphone_core_create_nat_policy(lc);
		stun_server = ms_strdup(linphone_core_get_stun_server(lc));
	}

	switch (pol) {
		default:
		case LinphonePolicyNoFirewall:
		case LinphonePolicyUseNatAddress:
			break;
		case LinphonePolicyUseStun:
			linphone_nat_policy_enable_stun(nat_policy, TRUE);
			break;
		case LinphonePolicyUseIce:
			linphone_nat_policy_enable_ice(nat_policy, TRUE);
			linphone_nat_policy_enable_stun(nat_policy, TRUE);
			break;
		case LinphonePolicyUseUpnp:
			ms_warning("UPNP is no longer supported, reset firewall policy to no firewall");
			break;
	}

	if (stun_server_username != NULL) {
		linphone_nat_policy_set_stun_server_username(nat_policy, stun_server_username);
		ms_free(stun_server_username);
	}
	if (stun_server != NULL) {
		linphone_nat_policy_set_stun_server(nat_policy, stun_server);
		ms_free(stun_server);
	}
	linphone_core_set_nat_policy(lc, nat_policy);
	linphone_nat_policy_unref(nat_policy);

	/* Ensure that the firewall policy is cleared in the config because it has been replaced by the nat_policy. */
	lp_config_set_string(lc->config, "net", "firewall_policy", NULL);
}

LinphoneFirewallPolicy linphone_core_get_firewall_policy(const LinphoneCore *lc) {
	const char *policy;

	policy = lp_config_get_string(lc->config, "net", "firewall_policy", NULL);
	if (policy == NULL) {
		LinphoneNatPolicy *nat_policy = linphone_core_get_nat_policy(lc);
		if (nat_policy == NULL) {
			return LinphonePolicyNoFirewall;
		} else if (linphone_nat_policy_upnp_enabled(nat_policy))
			return LinphonePolicyUseUpnp;
		else if (linphone_nat_policy_ice_enabled(nat_policy))
			return LinphonePolicyUseIce;
		else if (linphone_nat_policy_stun_enabled(nat_policy))
			return LinphonePolicyUseStun;
		else
			return LinphonePolicyNoFirewall;
	} else if (strcmp(policy, "0") == 0)
		return LinphonePolicyNoFirewall;
	else if ((strcmp(policy, "nat_address") == 0) || (strcmp(policy, "1") == 0))
		return LinphonePolicyUseNatAddress;
	else if ((strcmp(policy, "stun") == 0) || (strcmp(policy, "2") == 0))
		return LinphonePolicyUseStun;
	else if ((strcmp(policy, "ice") == 0) || (strcmp(policy, "3") == 0))
		return LinphonePolicyUseIce;
	else if ((strcmp(policy, "upnp") == 0) || (strcmp(policy, "4") == 0))
		return LinphonePolicyUseUpnp;
	else
		return LinphonePolicyNoFirewall;
}

void linphone_core_set_nat_policy(LinphoneCore *lc, LinphoneNatPolicy *policy) {
	if (policy != NULL) policy = linphone_nat_policy_ref(policy); /* Prevent object destruction if the same policy is used */
	else{
		ms_error("linphone_core_set_nat_policy() setting to NULL is not allowed");
		return ;
	}
	if (lc->nat_policy != NULL) {
		linphone_nat_policy_unref(lc->nat_policy);
		lc->nat_policy = NULL;
	}
	if (policy != NULL){
		lc->nat_policy = policy;
		/*start an immediate (but asynchronous) resolution.*/
		linphone_nat_policy_resolve_stun_server(policy);
		lp_config_set_string(lc->config, "net", "nat_policy_ref", lc->nat_policy->ref);
		linphone_nat_policy_save_to_config(lc->nat_policy);
	}

	lc->sal->enableNatHelper(!!lp_config_get_int(lc->config, "net", "enable_nat_helper", 1));
	lc->sal->enableAutoContacts(TRUE);
	lc->sal->useRport(!!lp_config_get_int(lc->config, "sip", "use_rport", 1));
	if (lc->sip_conf.contact) update_primary_contact(lc);
}

LinphoneNatPolicy * linphone_core_get_nat_policy(const LinphoneCore *lc) {
	return lc->nat_policy;
}


/*******************************************************************************
 * Call log related functions                                                  *
 ******************************************************************************/

void linphone_core_set_call_logs_database_path(LinphoneCore *lc, const char *path) {
	if (lc->logs_db_file){
		ms_free(lc->logs_db_file);
		lc->logs_db_file = NULL;
	}
	if (path) {
		lc->logs_db_file = ms_strdup(path);
		linphone_core_call_log_storage_init(lc);

		linphone_core_migrate_logs_from_rc_to_db(lc);
	}
}

const char * linphone_core_get_call_logs_database_path(LinphoneCore *lc) {
	return lc->logs_db_file;
}

const bctbx_list_t* linphone_core_get_call_logs(LinphoneCore *lc) {
	if (lc->logs_db) {
		linphone_core_get_call_history(lc);
	}
	return lc->call_logs;
}

void linphone_core_clear_call_logs(LinphoneCore *lc) {
	bool_t call_logs_sqlite_db_found = FALSE;
	lc->missed_calls=0;
	if (lc->logs_db) {
		call_logs_sqlite_db_found = TRUE;
		linphone_core_delete_call_history(lc);
	}
	if (!call_logs_sqlite_db_found) {
		bctbx_list_for_each(lc->call_logs, (void (*)(void*))linphone_call_log_unref);
		lc->call_logs = bctbx_list_free(lc->call_logs);
		call_logs_write_to_config_file(lc);
	}
}

int linphone_core_get_missed_calls_count(LinphoneCore *lc) {
	return lc->missed_calls;
}

void linphone_core_reset_missed_calls_count(LinphoneCore *lc) {
	lc->missed_calls=0;
}

void linphone_core_remove_call_log(LinphoneCore *lc, LinphoneCallLog *cl) {
	bool_t call_logs_sqlite_db_found = FALSE;
	if (lc->logs_db) {
		call_logs_sqlite_db_found = TRUE;
		linphone_core_delete_call_log(lc, cl);
	}
	lc->call_logs = bctbx_list_remove(lc->call_logs, cl);
	if (!call_logs_sqlite_db_found) {
		call_logs_write_to_config_file(lc);
		linphone_call_log_unref(cl);
	}
}

void linphone_core_migrate_logs_from_rc_to_db(LinphoneCore *lc) {
	bctbx_list_t *logs_to_migrate = NULL;
	LpConfig *lpc = NULL;
	size_t original_logs_count, migrated_logs_count;
	int i;

	if (!lc) {
		return;
	}

	lpc = linphone_core_get_config(lc);
	if (!lpc) {
		ms_warning("this core has been started without a rc file, nothing to migrate");
		return;
	}
	if (lp_config_get_int(lpc, "misc", "call_logs_migration_done", 0) == 1) {
		ms_warning("the call logs migration has already been done, skipping...");
		return;
	}

	logs_to_migrate = linphone_core_read_call_logs_from_config_file(lc);
	if (!logs_to_migrate) {
		ms_warning("nothing to migrate, skipping...");
		return;
	}

	// This is because there must have been a call previously to linphone_core_call_log_storage_init
	lc->call_logs = bctbx_list_free_with_data(lc->call_logs, (void (*)(void*))linphone_call_log_unref);
	lc->call_logs = NULL;

	// We can't use bctbx_list_for_each because logs_to_migrate are listed in the wrong order (latest first), and we want to store the logs latest last
	for (i = (int)bctbx_list_size(logs_to_migrate) - 1; i >= 0; i--) {
		LinphoneCallLog *log = (LinphoneCallLog *) bctbx_list_nth_data(logs_to_migrate, i);
		linphone_core_store_call_log(lc, log);
	}

	original_logs_count = bctbx_list_size(logs_to_migrate);
	migrated_logs_count = bctbx_list_size(lc->call_logs);
	if (original_logs_count == migrated_logs_count) {
		size_t i = 0;
		ms_debug("call logs migration successful: %u logs migrated", (unsigned int)bctbx_list_size(lc->call_logs));
		lp_config_set_int(lpc, "misc", "call_logs_migration_done", 1);

		for (; i < original_logs_count; i++) {
			char logsection[32];
			snprintf(logsection, sizeof(logsection), "call_log_%u", (unsigned int)i);
			lp_config_clean_section(lpc, logsection);
		}
	} else {
		ms_error("not as many logs saved in db has logs read from rc (" FORMAT_SIZE_T " in rc against " FORMAT_SIZE_T " in db)!", original_logs_count, migrated_logs_count);
	}

	bctbx_list_free_with_data(logs_to_migrate, (void (*)(void*))linphone_call_log_unref);
}


/*******************************************************************************
 * Video related functions                                                  *
 ******************************************************************************/

void linphone_core_resize_video_preview(LinphoneCore *lc, int width, int height) {
	bool_t auto_camera_preview_resize = !!lp_config_get_int(lc->config, "video", "auto_resize_preview_to_keep_ratio", 0);
	if (!auto_camera_preview_resize) return;
	bctbx_message("Resizing video preview to: %ix%i", width, height);
#ifdef VIDEO_ENABLED
	getPlatformHelpers(lc)->resizeVideoPreview(width, height);
#endif
}

#ifdef VIDEO_ENABLED
static void video_filter_callback(void *userdata, struct _MSFilter *f, unsigned int id, void *arg) {
	switch(id) {
		case  MS_JPEG_WRITER_SNAPSHOT_TAKEN: {
			LinphoneCore *lc = (LinphoneCore *)userdata;
			linphone_core_enable_video_preview(lc, FALSE);
			break;
		}
		case MS_QRCODE_READER_QRCODE_FOUND: {
			LinphoneCore *lc = (LinphoneCore *)userdata;
			if (linphone_core_cbs_get_qrcode_found(linphone_core_get_current_callbacks(lc)) != NULL) {
				char* result = ms_strdup((const char*)arg);
				linphone_core_notify_qrcode_found(lc, result);
				ms_free(result);
			}
			break;
		}
		case MS_CAMERA_PREVIEW_SIZE_CHANGED: {
			MSVideoSize size = *(MSVideoSize *)arg;
			bctbx_message("Camera preview size changed: %ix%i", size.width, size.height);
			LinphoneCore *lc = (LinphoneCore *)userdata;
			linphone_core_resize_video_preview(lc, size.width, size.height);
			break;
		}
	}
}
#endif

LinphoneStatus linphone_core_take_preview_snapshot(LinphoneCore *lc, const char *file) {
	LinphoneCall *call = linphone_core_get_current_call(lc);

	if (!file) return -1;
	if (call) {
		return linphone_call_take_preview_snapshot(call, file);
	} else {
#ifdef VIDEO_ENABLED
		if (lc->previewstream == NULL) {
			MSVideoSize vsize=lc->video_conf.preview_vsize.width != 0 ? lc->video_conf.preview_vsize : lc->video_conf.vsize;
			lc->previewstream = video_preview_new(lc->factory);
			video_preview_set_size(lc->previewstream, vsize);
			video_preview_set_display_filter_name(lc->previewstream, NULL);
			video_preview_set_fps(lc->previewstream,linphone_core_get_preferred_framerate(lc));
			video_preview_start(lc->previewstream, lc->video_conf.device);
			lc->previewstream->ms.factory = lc->factory;
			linphone_core_enable_video_preview(lc, TRUE);

			ms_filter_add_notify_callback(lc->previewstream->local_jpegwriter, video_filter_callback, lc, TRUE);
			ms_filter_call_method(lc->previewstream->local_jpegwriter, MS_JPEG_WRITER_TAKE_SNAPSHOT, (void*)file);
		} else {
			ms_filter_call_method(lc->previewstream->local_jpegwriter, MS_JPEG_WRITER_TAKE_SNAPSHOT, (void*)file);
		}
		return 0;
#endif
	}
	return -1;
}

static void toggle_video_preview(LinphoneCore *lc, bool_t val){
#ifdef VIDEO_ENABLED
	if (val) {
		if (lc->previewstream == NULL) {
			const char *display_filter = linphone_core_get_video_display_filter(lc);
			MSVideoSize vsize = { 0 };
			const LinphoneVideoDefinition *vdef = linphone_core_get_preview_video_definition(lc);
			if (!vdef || linphone_video_definition_is_undefined(vdef)) {
				vdef = linphone_core_get_preferred_video_definition(lc);
			}
			if (linphone_core_qrcode_video_preview_enabled(lc)) {
				vsize.width = 720;
				vsize.height = 1280;
			} else {
				vsize.width = (int)linphone_video_definition_get_width(vdef);
				vsize.height = (int)linphone_video_definition_get_height(vdef);
			}
			lc->previewstream = video_preview_new(lc->factory);
			video_preview_set_size(lc->previewstream, vsize);
			video_stream_set_device_rotation(lc->previewstream, lc->device_rotation);
			if (display_filter) {
				video_preview_set_display_filter_name(lc->previewstream, display_filter);
			}
			if (lc->preview_window_id != NULL) {
				video_preview_set_native_window_id(lc->previewstream, lc->preview_window_id);
			}
			video_preview_set_fps(lc->previewstream, linphone_core_get_preferred_framerate(lc));
			if (linphone_core_qrcode_video_preview_enabled(lc)) {
				video_preview_enable_qrcode(lc->previewstream, TRUE);
				if (lc->qrcode_rect.w != 0 && lc->qrcode_rect.h != 0) {
					video_preview_set_decode_rect(lc->previewstream, lc->qrcode_rect);
				}
			}
			video_preview_start(lc->previewstream, lc->video_conf.device);
			if (video_preview_qrcode_enabled(lc->previewstream)) {
				ms_filter_add_notify_callback(lc->previewstream->qrcode, video_filter_callback, lc, TRUE);
			}
			MSVideoSize size = video_preview_get_current_size(lc->previewstream);
			linphone_core_resize_video_preview(lc, size.width, size.height);
			ms_filter_add_notify_callback(lc->previewstream->source, video_filter_callback, lc, TRUE);
		}
	} else {
		if (lc->previewstream != NULL) {
			ms_filter_remove_notify_callback(lc->previewstream->source, video_filter_callback, lc);
			video_preview_stop(lc->previewstream);
			lc->previewstream = NULL;
		}
	}
#endif
}

static void relaunch_video_preview(LinphoneCore *lc){
	if (lc->previewstream){
		toggle_video_preview(lc,FALSE);
	}
	/* And nothing else, because linphone_core_iterate() will restart the preview stream if necessary.
	 * This code will need to be revisited when linphone_core_iterate() will no longer be required*/
}

bool_t linphone_core_video_supported(LinphoneCore *lc){
#ifdef VIDEO_ENABLED
	return TRUE;
#else
	return FALSE;
#endif
}

void linphone_core_enable_video(LinphoneCore *lc, bool_t vcap_enabled, bool_t display_enabled) {
	linphone_core_enable_video_capture(lc, vcap_enabled);
	linphone_core_enable_video_display(lc, display_enabled);
}

bool_t linphone_core_video_enabled(LinphoneCore *lc){
	return (lc->video_conf.display || lc->video_conf.capture);
}

static void reapply_network_bandwidth_settings(LinphoneCore *lc) {
	linphone_core_set_download_bandwidth(lc, linphone_core_get_download_bandwidth(lc));
	linphone_core_set_upload_bandwidth(lc, linphone_core_get_upload_bandwidth(lc));
}

void linphone_core_enable_video_capture(LinphoneCore *lc, bool_t enable) {
#ifndef VIDEO_ENABLED
	if (enable == TRUE) {
		ms_warning("Cannot enable video capture, this version of linphone was built without video support.");
	}
#endif
	lc->video_conf.capture = enable;
	if (linphone_core_ready(lc)) {
		lp_config_set_int(lc->config, "video", "capture", lc->video_conf.capture);
	}
	/* Need to re-apply network bandwidth settings. */
	reapply_network_bandwidth_settings(lc);
}

void linphone_core_enable_video_display(LinphoneCore *lc, bool_t enable) {
#ifndef VIDEO_ENABLED
	if (enable == TRUE) {
		ms_warning("Cannot enable video display, this version of linphone was built without video support.");
	}
#endif
	lc->video_conf.display = enable;
	if (linphone_core_ready(lc)) {
		lp_config_set_int(lc->config, "video", "display", lc->video_conf.display);
	}
	/* Need to re-apply network bandwidth settings. */
	reapply_network_bandwidth_settings(lc);
}

void linphone_core_enable_video_source_reuse(LinphoneCore* lc, bool_t enable){
#ifndef VIDEO_ENABLED
	if (enable == TRUE) {
		ms_warning("Cannot enable video display, this version of linphone was built without video support.");
	}
#endif
	lc->video_conf.reuse_preview_source = enable;
	if( linphone_core_ready(lc) ){
		lp_config_set_int(lc->config, "video", "reuse_source", lc->video_conf.reuse_preview_source);
	}
}

bool_t linphone_core_video_capture_enabled(LinphoneCore *lc) {
	return lc->video_conf.capture;
}

bool_t linphone_core_video_display_enabled(LinphoneCore *lc) {
	return lc->video_conf.display;
}

void linphone_core_set_video_policy(LinphoneCore *lc, const LinphoneVideoPolicy *policy){
	lc->video_policy=*policy;
	if (linphone_core_ready(lc)){
		lp_config_set_int(lc->config,"video","automatically_initiate",policy->automatically_initiate);
		lp_config_set_int(lc->config,"video","automatically_accept",policy->automatically_accept);
	}
}

const LinphoneVideoPolicy *linphone_core_get_video_policy(const LinphoneCore *lc){
	return &lc->video_policy;
}

BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneVideoActivationPolicy);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneVideoActivationPolicy, belle_sip_object_t,
	NULL, // destroy
	NULL, // clone
	NULL, // marshal
	FALSE
);

LinphoneVideoActivationPolicy *linphone_video_activation_policy_new() {
	LinphoneVideoActivationPolicy *policy = belle_sip_object_new(LinphoneVideoActivationPolicy);
	policy->automatically_accept = FALSE;
	policy->automatically_initiate = FALSE;
	return policy;
}

LinphoneVideoActivationPolicy* linphone_video_activation_policy_ref(LinphoneVideoActivationPolicy* policy) {
	return (LinphoneVideoActivationPolicy*) belle_sip_object_ref(policy);
}

void linphone_video_activation_policy_unref(LinphoneVideoActivationPolicy* policy) {
	belle_sip_object_unref(policy);
}

void *linphone_video_activation_policy_get_user_data(const LinphoneVideoActivationPolicy *policy) {
	return policy->user_data;
}

void linphone_video_activation_policy_set_user_data(LinphoneVideoActivationPolicy *policy, void *data) {
	policy->user_data = data;
}

bool_t linphone_video_activation_policy_get_automatically_accept(const LinphoneVideoActivationPolicy *policy) {
	return policy->automatically_accept;
}

bool_t linphone_video_activation_policy_get_automatically_initiate(const LinphoneVideoActivationPolicy *policy) {
	return policy->automatically_initiate;
}

void linphone_video_activation_policy_set_automatically_accept(LinphoneVideoActivationPolicy *policy, bool_t enable) {
	policy->automatically_accept = enable;
}

void linphone_video_activation_policy_set_automatically_initiate(LinphoneVideoActivationPolicy *policy, bool_t enable) {
	policy->automatically_initiate = enable;
}

void linphone_core_set_video_activation_policy(LinphoneCore *lc, const LinphoneVideoActivationPolicy *policy) {
	lc->video_policy.automatically_accept = policy->automatically_accept;
	lc->video_policy.automatically_initiate = policy->automatically_initiate;
	if (linphone_core_ready(lc)) {
		lp_config_set_int(lc->config, "video", "automatically_initiate", policy->automatically_initiate);
		lp_config_set_int(lc->config, "video", "automatically_accept", policy->automatically_accept);
	}
}

LinphoneVideoActivationPolicy *linphone_core_get_video_activation_policy(const LinphoneCore *lc){
	LinphoneVideoActivationPolicy *policy = linphone_video_activation_policy_new();
	policy->automatically_accept = lc->video_policy.automatically_accept;
	policy->automatically_initiate = lc->video_policy.automatically_initiate;
	return policy;
}

void linphone_core_enable_video_preview(LinphoneCore *lc, bool_t val){
	lc->video_conf.show_local=val;
	if (linphone_core_ready(lc))
		lp_config_set_int(lc->config,"video","show_local",val);
}

bool_t linphone_core_video_preview_enabled(const LinphoneCore *lc){
	return lc->video_conf.show_local;
}

void linphone_core_enable_qrcode_video_preview(LinphoneCore *lc, bool_t val) {
	lc->video_conf.qrcode_decoder=val;
	if (linphone_core_ready(lc))
		lp_config_set_int(lc->config,"video","qrcode_decoder",val);
}

bool_t linphone_core_qrcode_video_preview_enabled(const LinphoneCore *lc) {
	return lc->video_conf.qrcode_decoder;
}

void linphone_core_set_qrcode_decode_rect(LinphoneCore *lc, const int x, const int y, const int w, const int h) {
	if (lc) {
		MSRect rect;
		rect.x = x;
		rect.y = y;
		rect.w = w;
		rect.h = h;
		lc->qrcode_rect = rect;
	}
}

void linphone_core_enable_self_view(LinphoneCore *lc, bool_t val){
#ifdef VIDEO_ENABLED
	LinphoneCall *call=linphone_core_get_current_call (lc);
	lc->video_conf.selfview=val;
	if (linphone_core_ready(lc)) {
		lp_config_set_int(lc->config,"video","self_view",linphone_core_self_view_enabled(lc));
	}
	if (call) {
		VideoStream *vstream = reinterpret_cast<VideoStream *>(linphone_call_get_stream(call, LinphoneStreamTypeVideo));
		if (vstream)
			video_stream_enable_self_view(vstream,val);
	}
	if (linphone_core_ready(lc)){
		lp_config_set_int(lc->config,"video","self_view",val);
	}
#endif
}

bool_t linphone_core_self_view_enabled(const LinphoneCore *lc){
	return lc->video_conf.selfview;
}

LinphoneStatus linphone_core_set_video_device(LinphoneCore *lc, const char *id){
	MSWebCam *olddev=lc->video_conf.device;
	const char *vd;
	if (id!=NULL){
		lc->video_conf.device=ms_web_cam_manager_get_cam(ms_factory_get_web_cam_manager(lc->factory),id);
		if (lc->video_conf.device==NULL){
			ms_warning("Could not find video device %s",id);
		}
	}
	if (lc->video_conf.device==NULL)
		lc->video_conf.device=ms_web_cam_manager_get_default_cam(ms_factory_get_web_cam_manager(lc->factory));
	if (olddev!=NULL && olddev!=lc->video_conf.device){
		relaunch_video_preview(lc);
	}
	if ( linphone_core_ready(lc) && lc->video_conf.device){
		vd=ms_web_cam_get_string_id(lc->video_conf.device);
		if (vd && strstr(vd,"Static picture")!=NULL){
			vd=NULL;
		}
		lp_config_set_string(lc->config,"video","device",vd);
	}
	return 0;
}

const char *linphone_core_get_video_device(const LinphoneCore *lc){
	if (lc->video_conf.device) return ms_web_cam_get_string_id(lc->video_conf.device);
	return NULL;
}

#ifdef VIDEO_ENABLED
static VideoStream * get_active_video_stream(LinphoneCore *lc){
	VideoStream *vs = NULL;
	LinphoneCall *call=linphone_core_get_current_call (lc);
	/* Select the video stream from the call in the first place */
	if (call) {
		VideoStream *vstream = reinterpret_cast<VideoStream *>(linphone_call_get_stream(call, LinphoneStreamTypeVideo));
		if (vstream)
			vs = vstream;
	}
	/* If not in call, select the video stream from the preview */
	if (vs == NULL && lc->previewstream) {
		vs = lc->previewstream;
	}
	return vs;
}
#endif

LinphoneStatus linphone_core_set_static_picture(LinphoneCore *lc, const char *path) {
#ifdef VIDEO_ENABLED
	VideoStream *vs=get_active_video_stream(lc);
	/* If we have a video stream (either preview, either from call), we
		 have a source and it is using the static picture filter, then
		 force the filter to use that picture. */
	if (vs && vs->source) {
		if (ms_filter_get_id(vs->source) == MS_STATIC_IMAGE_ID) {
			ms_filter_call_method(vs->source, MS_STATIC_IMAGE_SET_IMAGE,
														(void *)path);
		}
	}
	/* Tell the static image filter to use that image from now on so
		 that the image will be used next time it has to be read */
	ms_static_image_set_default_image(path);
#else
	ms_warning("Video support not compiled.");
#endif
	return 0;
}

const char *linphone_core_get_static_picture(LinphoneCore *lc) {
	const char *path=NULL;
#ifdef VIDEO_ENABLED
	path=ms_static_image_get_default_image();
#else
	ms_warning("Video support not compiled.");
#endif
	return path;
}

LinphoneStatus linphone_core_set_static_picture_fps(LinphoneCore *lc, float fps) {
#ifdef VIDEO_ENABLED
	VideoStream *vs = NULL;

	vs=get_active_video_stream(lc);

	/* If we have a video stream (either preview, either from call), we
		 have a source and it is using the static picture filter, then
		 force the filter to use that picture. */
	if (vs && vs->source) {
		if (ms_filter_get_id(vs->source) == MS_STATIC_IMAGE_ID) {
			ms_filter_call_method(vs->source, MS_FILTER_SET_FPS,(void *)&fps);
		}
	}
#else
	ms_warning("Video support not compiled.");
#endif
	return 0;
}

float linphone_core_get_static_picture_fps(LinphoneCore *lc) {
#ifdef VIDEO_ENABLED
	VideoStream *vs = NULL;
	vs=get_active_video_stream(lc);
	/* If we have a video stream (either preview, either from call), we
		 have a source and it is using the static picture filter, then
		 force the filter to use that picture. */
	if (vs && vs->source) {
		if (ms_filter_get_id(vs->source) == MS_STATIC_IMAGE_ID) {

				float fps;

			ms_filter_call_method(vs->source, MS_FILTER_GET_FPS,(void *)&fps);
			return fps;
		}
	}
#else
	ms_warning("Video support not compiled.");
#endif
	return 0;
}

void * linphone_core_get_native_video_window_id(const LinphoneCore *lc){
	if (lc->video_window_id) {
		/* case where the video id was previously set by the app*/
		return lc->video_window_id;
	}else{
#ifdef VIDEO_ENABLED
		/*case where it was not set but we want to get the one automatically created by mediastreamer2 (desktop versions only)*/
		LinphoneCall *call=linphone_core_get_current_call (lc);
		if (call) {
			VideoStream *vstream = reinterpret_cast<VideoStream *>(linphone_call_get_stream(call, LinphoneStreamTypeVideo));
			if (vstream)
				return video_stream_get_native_window_id(vstream);
		}
#endif
	}
	return 0;
}

/* unsets the video id for all calls (indeed it may be kept by filters or videostream object itself by paused calls)*/
static void unset_video_window_id(LinphoneCore *lc, bool_t preview, void *id){
	if ((id != NULL)
#ifndef _WIN32
		&& ((unsigned long)id != (unsigned long)-1)
#endif
	){
		ms_error("Invalid use of unset_video_window_id()");
		return;
	}
	L_GET_PRIVATE_FROM_C_OBJECT(lc)->unsetVideoWindowId(!!preview, id);
}

void _linphone_core_set_native_video_window_id(LinphoneCore *lc, void *id) {
	if ((id == NULL)
#ifndef _WIN32
		|| ((unsigned long)id == (unsigned long)-1)
#endif
	){
		unset_video_window_id(lc,FALSE,id);
	}
	lc->video_window_id=id;
#ifdef VIDEO_ENABLED
	{
		LinphoneCall *call=linphone_core_get_current_call(lc);
		if (call) {
			VideoStream *vstream = reinterpret_cast<VideoStream *>(linphone_call_get_stream(call, LinphoneStreamTypeVideo));
			if (vstream)
				video_stream_set_native_window_id(vstream,id);
		}
	}
#endif
}

void linphone_core_set_native_video_window_id(LinphoneCore *lc, void *id) {
#ifdef ANDROID
	getPlatformHelpers(lc)->setVideoWindow(id);
#else
	_linphone_core_set_native_video_window_id(lc, id);
#endif
}

void * linphone_core_get_native_preview_window_id(const LinphoneCore *lc){
	if (lc->preview_window_id){
		/*case where the id was set by the app previously*/
		return lc->preview_window_id;
	}else{
		/*case where we want the id automatically created by mediastreamer2 (desktop versions only)*/
#ifdef VIDEO_ENABLED
		LinphoneCall *call=linphone_core_get_current_call(lc);
		if (call) {
			VideoStream *vstream = reinterpret_cast<VideoStream *>(linphone_call_get_stream(call, LinphoneStreamTypeVideo));
			if (vstream)
				return video_stream_get_native_preview_window_id(vstream);
		}
		if (lc->previewstream)
			return video_preview_get_native_window_id(lc->previewstream);
#endif
	}
	return 0;
}

void _linphone_core_set_native_preview_window_id(LinphoneCore *lc, void *id) {
	if ((id == NULL)
#ifndef _WIN32
		|| ((unsigned long)id == (unsigned long)-1)
#endif
	) {
		unset_video_window_id(lc,TRUE,id);
	}
	lc->preview_window_id=id;
#ifdef VIDEO_ENABLED
	{
		LinphoneCall *call=linphone_core_get_current_call(lc);
		if (call) {
			VideoStream *vstream = reinterpret_cast<VideoStream *>(linphone_call_get_stream(call, LinphoneStreamTypeVideo));
			if (vstream)
				video_stream_set_native_preview_window_id(vstream,id);
		}else if (lc->previewstream){
			video_preview_set_native_window_id(lc->previewstream,id);
		}
	}
#endif
}

void linphone_core_set_native_preview_window_id(LinphoneCore *lc, void *id) {
#ifdef ANDROID
	getPlatformHelpers(lc)->setVideoPreviewWindow(id);
#else
	_linphone_core_set_native_preview_window_id(lc, id);
#endif
}

void linphone_core_show_video(LinphoneCore *lc, bool_t show){
#ifdef VIDEO_ENABLED
	LinphoneCall *call=linphone_core_get_current_call(lc);
	ms_error("linphone_core_show_video %d", show);
	if (call) {
		VideoStream *vstream = reinterpret_cast<VideoStream *>(linphone_call_get_stream(call, LinphoneStreamTypeVideo));
		if (vstream)
			video_stream_show_video(vstream,show);
	}
#endif
}

void linphone_core_use_preview_window(LinphoneCore *lc, bool_t yesno){
	lc->use_preview_window=yesno;
}

int linphone_core_get_device_rotation(LinphoneCore *lc ) {
	return lc->device_rotation;
}

void linphone_core_set_device_rotation(LinphoneCore *lc, int rotation) {
	if (rotation!=lc->device_rotation) ms_message("%s : rotation=%d\n", __FUNCTION__, rotation);
	lc->device_rotation = rotation;
#ifdef VIDEO_ENABLED
	{
		LinphoneCall *call=linphone_core_get_current_call(lc);
		VideoStream *vstream = NULL;
		if (call) {
			vstream = reinterpret_cast<VideoStream *>(linphone_call_get_stream(call, LinphoneStreamTypeVideo));
			if (vstream) {
				video_stream_set_device_rotation(vstream, rotation);
				video_stream_change_camera_skip_bitrate(vstream, vstream->cam);
			}
		} else if (linphone_core_video_preview_enabled(lc)) {
			vstream = lc->previewstream;
			if (vstream) {
				video_stream_set_device_rotation(vstream, rotation);
				video_preview_update_video_params(vstream);
			}
		}
	}
#endif
}

int linphone_core_get_camera_sensor_rotation(LinphoneCore *lc) {
#ifdef VIDEO_ENABLED
	LinphoneCall *call = linphone_core_get_current_call(lc);
	if (call) {
		VideoStream *vstream = reinterpret_cast<VideoStream *>(linphone_call_get_stream(call, LinphoneStreamTypeVideo));
		if (vstream)
			return video_stream_get_camera_sensor_rotation(vstream);
	}
#endif
	return -1;
}

static MSVideoSizeDef supported_resolutions[] = {
#if !defined(__ANDROID__) && !TARGET_OS_IPHONE
	{ { MS_VIDEO_SIZE_1080P_W, MS_VIDEO_SIZE_1080P_H }, "1080p" },
#endif
#if !defined(__ANDROID__) && !TARGET_OS_MAC /*limit to most common sizes because mac video API cannot list supported resolutions*/
	{ { MS_VIDEO_SIZE_UXGA_W, MS_VIDEO_SIZE_UXGA_H }, "uxga" },
	{ { MS_VIDEO_SIZE_SXGA_MINUS_W, MS_VIDEO_SIZE_SXGA_MINUS_H }, "sxga-" },
#endif
	{ { MS_VIDEO_SIZE_720P_W, MS_VIDEO_SIZE_720P_H }, "720p" },
#if !defined(__ANDROID__) && !TARGET_OS_MAC
	{ { MS_VIDEO_SIZE_XGA_W, MS_VIDEO_SIZE_XGA_H }, "xga" },
#endif
#if !defined(__ANDROID__) && !TARGET_OS_IPHONE
	{ { MS_VIDEO_SIZE_SVGA_W, MS_VIDEO_SIZE_SVGA_H }, "svga" },
	{ { MS_VIDEO_SIZE_4CIF_W, MS_VIDEO_SIZE_4CIF_H }, "4cif" },
#endif
	{ { MS_VIDEO_SIZE_VGA_W, MS_VIDEO_SIZE_VGA_H }, "vga" },
#if TARGET_OS_IPHONE
	{ { MS_VIDEO_SIZE_IOS_MEDIUM_H, MS_VIDEO_SIZE_IOS_MEDIUM_W }, "ios-medium" },
#endif
	{ { MS_VIDEO_SIZE_CIF_W, MS_VIDEO_SIZE_CIF_H }, "cif" },
#if !TARGET_OS_MAC || TARGET_OS_IPHONE /* OS_MAC is 1 for iPhone, but we need QVGA */
	{ { MS_VIDEO_SIZE_QVGA_W, MS_VIDEO_SIZE_QVGA_H } , "qvga" },
#endif
	{ { MS_VIDEO_SIZE_QCIF_W, MS_VIDEO_SIZE_QCIF_H }, "qcif" },
	{ { 0, 0 }, NULL }
};

const MSVideoSizeDef *linphone_core_get_supported_video_sizes(LinphoneCore *lc){
	return supported_resolutions;
}

static MSVideoSize video_size_get_by_name(const char *name){
	MSVideoSizeDef *pdef=supported_resolutions;
	MSVideoSize null_vsize={0,0};
	MSVideoSize parsed;
	if (!name) return null_vsize;
	for(;pdef->name!=NULL;pdef++){
		if (strcasecmp(name,pdef->name)==0){
			return pdef->vsize;
		}
	}
	if (sscanf(name,"%ix%i",&parsed.width,&parsed.height)==2){
		return parsed;
	}
	ms_warning("Video resolution %s is not supported in linphone.",name);
	return null_vsize;
}

static bool_t video_definition_supported(const LinphoneVideoDefinition *vdef) {
	const bctbx_list_t *item;
	const bctbx_list_t *supported_definitions = linphone_factory_get_supported_video_definitions(linphone_factory_get());
	for (item = supported_definitions; item != NULL; item = bctbx_list_next(item)) {
		LinphoneVideoDefinition *supported_vdef = (LinphoneVideoDefinition *)bctbx_list_get_data(item);
		if (linphone_video_definition_equals(vdef, supported_vdef)) return TRUE;
	}
	ms_warning("Video definition %ix%i is not supported", linphone_video_definition_get_width(vdef), linphone_video_definition_get_height(vdef));
	return FALSE;
}

void linphone_core_set_preferred_video_definition(LinphoneCore *lc, LinphoneVideoDefinition *vdef) {
	if (video_definition_supported(vdef)) {
		LinphoneVideoDefinition *oldvdef = lc->video_conf.vdef;
		lc->video_conf.vdef = linphone_video_definition_ref(vdef);

		if ((lc->previewstream != NULL) && (lc->video_conf.preview_vdef == NULL)
			&& (oldvdef != NULL) && !linphone_video_definition_equals(oldvdef, vdef)) {
			relaunch_video_preview(lc);
		}

		if (oldvdef != NULL) linphone_video_definition_unref(oldvdef);
		if (linphone_core_ready(lc)) {
			lp_config_set_string(lc->config, "video", "size", linphone_video_definition_get_name(vdef));
		}
	}
}

void linphone_core_set_preferred_video_size(LinphoneCore *lc, MSVideoSize vsize) {
	linphone_core_set_preferred_video_definition(lc,
		linphone_factory_find_supported_video_definition(linphone_factory_get(), (unsigned int)vsize.width, (unsigned int)vsize.height));
}

void linphone_core_set_preview_video_definition(LinphoneCore *lc, LinphoneVideoDefinition *vdef) {
	if (!vdef || linphone_video_definition_is_undefined(vdef)) {
		/* Reset the forced preview video definition mode */
		if (lc->video_conf.preview_vdef != NULL) linphone_video_definition_unref(lc->video_conf.preview_vdef);
		lc->video_conf.preview_vdef = NULL;
		if (linphone_core_ready(lc)) {
			lp_config_set_string(lc->config, "video", "preview_size", NULL);
		}
		return;
	}

	if (!linphone_video_definition_equals(lc->video_conf.preview_vdef, vdef)) {
		LinphoneVideoDefinition *oldvdef = lc->video_conf.preview_vdef;
		lc->video_conf.preview_vdef = linphone_video_definition_ref(vdef);
		if (oldvdef != NULL) linphone_video_definition_unref(oldvdef);
		if (lc->previewstream != NULL) {
			relaunch_video_preview(lc);
		}
	}
	if (linphone_core_ready(lc)) {
		lp_config_set_string(lc->config, "video", "preview_size", linphone_video_definition_get_name(vdef));
	}
}

void linphone_core_set_preview_video_size(LinphoneCore *lc, MSVideoSize vsize) {
	linphone_core_set_preview_video_definition(lc,
		linphone_factory_find_supported_video_definition(linphone_factory_get(), (unsigned int)vsize.width, (unsigned int)vsize.height));
}

const LinphoneVideoDefinition * linphone_core_get_preview_video_definition(const LinphoneCore *lc) {
	return lc->video_conf.preview_vdef;
}

MSVideoSize linphone_core_get_preview_video_size(const LinphoneCore *lc) {
	MSVideoSize vsize = { 0 };
	vsize.width = (int)linphone_video_definition_get_width(lc->video_conf.preview_vdef);
	vsize.height = (int)linphone_video_definition_get_height(lc->video_conf.preview_vdef);
	return vsize;
}

MSVideoSize linphone_core_get_current_preview_video_size(const LinphoneCore *lc){
	MSVideoSize ret={0};
#ifndef VIDEO_ENABLED
	ms_error("linphone_core_get_current_preview_video_size() fail. Support for video is disabled");
#else
	if (lc->previewstream){
		ret=video_preview_get_current_size(lc->previewstream);
	}
#endif
	return ret;
}

LinphoneVideoDefinition * linphone_core_get_current_preview_video_definition(const LinphoneCore *lc) {
#ifdef VIDEO_ENABLED
	MSVideoSize vsize;
	if (lc->previewstream) {
		vsize = video_preview_get_current_size(lc->previewstream);
	}
	return linphone_factory_find_supported_video_definition(linphone_factory_get(), (unsigned int)vsize.width, (unsigned int)vsize.height);
#else
	ms_error("Video support is disabled");
	return NULL;
#endif
}

void linphone_core_set_preview_video_size_by_name(LinphoneCore *lc, const char *name){
	MSVideoSize vsize=video_size_get_by_name(name);
	linphone_core_set_preview_video_size(lc,vsize);
}

void linphone_core_set_preview_video_definition_by_name(LinphoneCore *lc, const char *name) {
	LinphoneVideoDefinition *vdef = linphone_factory_find_supported_video_definition_by_name(linphone_factory_get(), name);
	if (vdef == NULL) {
		ms_error("Video definition '%s' is not supported", name);
	} else {
		linphone_core_set_preview_video_definition(lc, vdef);
	}
}

void linphone_core_set_preferred_video_size_by_name(LinphoneCore *lc, const char *name){
	MSVideoSize vsize=video_size_get_by_name(name);
	MSVideoSize default_vsize={MS_VIDEO_SIZE_CIF_W,MS_VIDEO_SIZE_CIF_H};
	if (vsize.width!=0)	linphone_core_set_preferred_video_size(lc,vsize);
	else linphone_core_set_preferred_video_size(lc,default_vsize);
}

void linphone_core_set_preferred_video_definition_by_name(LinphoneCore *lc, const char *name) {
	LinphoneVideoDefinition *vdef = linphone_factory_find_supported_video_definition_by_name(linphone_factory_get(), name);
	if (vdef == NULL) {
		ms_error("Video definition '%s' is not supported", name);
	} else {
		linphone_core_set_preferred_video_definition(lc, vdef);
	}
}

const LinphoneVideoDefinition * linphone_core_get_preferred_video_definition(const LinphoneCore *lc) {
	return lc->video_conf.vdef;
}

MSVideoSize linphone_core_get_preferred_video_size(const LinphoneCore *lc) {
	MSVideoSize vsize = { 0 };
	vsize.width = (int)linphone_video_definition_get_width(lc->video_conf.vdef);
	vsize.height = (int)linphone_video_definition_get_height(lc->video_conf.vdef);
	return vsize;
}

char * linphone_core_get_preferred_video_size_name(const LinphoneCore *lc) {
	return ms_strdup(linphone_video_definition_get_name(lc->video_conf.vdef));
}

void linphone_core_set_preferred_framerate(LinphoneCore *lc, float fps){
	lc->video_conf.fps=fps;
	if (linphone_core_ready(lc))
		lp_config_set_float(lc->config,"video","framerate",fps);
}

float linphone_core_get_preferred_framerate(LinphoneCore *lc){
	return lc->video_conf.fps;
}

void linphone_core_preview_ogl_render(const LinphoneCore *lc) {
	#ifdef VIDEO_ENABLED

	LinphoneCall *call = linphone_core_get_current_call(lc);
	VideoStream *stream = call ? reinterpret_cast<VideoStream *>(linphone_call_get_stream(call, LinphoneStreamTypeVideo)) : lc->previewstream;

	if (stream && stream->output2 && ms_filter_get_id(stream->output2) == MS_OGL_ID) {
		int mirroring = TRUE;
		ms_filter_call_method(stream->output2, MS_VIDEO_DISPLAY_ENABLE_MIRRORING, &mirroring);
		ms_filter_call_method(stream->output2, MS_OGL_RENDER, NULL);
	}

	#endif
}

void linphone_core_set_use_files(LinphoneCore *lc, bool_t yesno){
	lc->use_files=yesno;
}

bool_t linphone_core_get_use_files(LinphoneCore *lc) {
	return lc->use_files;
}

const char * linphone_core_get_play_file(const LinphoneCore *lc) {
	return lc->play_file;
}

void linphone_core_set_play_file(LinphoneCore *lc, const char *file){
	LinphoneCall *call=linphone_core_get_current_call(lc);
	if (lc->play_file!=NULL){
		ms_free(lc->play_file);
		lc->play_file=NULL;
	}
	if (file!=NULL) {
		lc->play_file=ms_strdup(file);
		if (call) {
			AudioStream *astream = reinterpret_cast<AudioStream *>(linphone_call_get_stream(call, LinphoneStreamTypeAudio));
			if (astream && astream->ms.state==MSStreamStarted)
				audio_stream_play(astream,file);
		}
	}
}

const char * linphone_core_get_record_file(const LinphoneCore *lc) {
	return lc->rec_file;
}

void linphone_core_set_record_file(LinphoneCore *lc, const char *file){
	LinphoneCall *call=linphone_core_get_current_call(lc);
	if (lc->rec_file!=NULL){
		ms_free(lc->rec_file);
		lc->rec_file=NULL;
	}
	if (file!=NULL) {
		lc->rec_file=ms_strdup(file);
		if (call) {
			AudioStream *astream = reinterpret_cast<AudioStream *>(linphone_call_get_stream(call, LinphoneStreamTypeAudio));
			if (astream)
				audio_stream_record(astream,file);
		}
	}
}

typedef enum{
	LinphoneToneGenerator,
	LinphoneLocalPlayer
}LinphoneAudioResourceType;

static MSFilter *get_audio_resource(LinphoneCore *lc, LinphoneAudioResourceType rtype, MSSndCard *card, bool_t create) {
	LinphoneCall *call=linphone_core_get_current_call(lc);
	AudioStream *stream=NULL;
	RingStream *ringstream;
	if (call){
		stream=reinterpret_cast<AudioStream *>(linphone_call_get_stream(call, LinphoneStreamTypeAudio));
	}else if (linphone_core_is_in_conference(lc)){
		stream=linphone_conference_get_audio_stream(lc->conf_ctx);
	}
	if (stream){
		if (rtype==LinphoneToneGenerator) return stream->dtmfgen;
		if (rtype==LinphoneLocalPlayer) return stream->local_player;
		return NULL;
	}
	if (card && lc->ringstream && card != lc->ringstream->card){
			ring_stop(lc->ringstream);
			lc->ringstream = NULL;
	}
	if (lc->ringstream  == NULL) {
		float amp=lp_config_get_float(lc->config,"sound","dtmf_player_amp",0.1f);
		MSSndCard *ringcard=lc->sound_conf.lsd_card
			? lc->sound_conf.lsd_card
			: card
				? card
				: lc->sound_conf.ring_sndcard;

		if (ringcard == NULL)
			return NULL;
		
		if (!create) return NULL;

		ringstream=lc->ringstream=ring_start(lc->factory, NULL,0,ringcard);
		ms_filter_call_method(lc->ringstream->gendtmf,MS_DTMF_GEN_SET_DEFAULT_AMPLITUDE,&amp);
		lc->dmfs_playing_start_time = (time_t)ms_get_cur_time_ms()/1000;
	}else{
		ringstream=lc->ringstream;
		if (lc->dmfs_playing_start_time!=0)
			lc->dmfs_playing_start_time = (time_t)ms_get_cur_time_ms()/1000;
	}
	if (rtype==LinphoneToneGenerator) return ringstream->gendtmf;
	if (rtype==LinphoneLocalPlayer) return ringstream->source;
	return NULL;
}

static MSFilter *get_dtmf_gen(LinphoneCore *lc, MSSndCard *card) {
	return get_audio_resource(lc,LinphoneToneGenerator, card, TRUE);
}

void linphone_core_play_dtmf(LinphoneCore *lc, char dtmf, int duration_ms){
	MSSndCard *card = linphone_core_in_call(lc)
		? lc->sound_conf.play_sndcard
		: lc->sound_conf.ring_sndcard;
	MSFilter *f=get_dtmf_gen(lc, card);
	if (f==NULL){
		ms_error("No dtmf generator at this time !");
		return;
	}

	if (duration_ms > 0)
		ms_filter_call_method(f, MS_DTMF_GEN_PLAY, &dtmf);
	else ms_filter_call_method(f, MS_DTMF_GEN_START, &dtmf);
}

LinphoneStatus _linphone_core_play_local(LinphoneCore *lc, const char *audiofile, MSSndCard *card) {
	MSFilter *f=get_audio_resource(lc,LinphoneLocalPlayer, card, TRUE);
	int loopms=-1;
	if (!f) return -1;
	ms_filter_call_method(f,MS_PLAYER_SET_LOOP,&loopms);
	if (ms_filter_call_method(f,MS_PLAYER_OPEN,(void*)audiofile)!=0){
		return -1;
	}
	ms_filter_call_method_noarg(f,MS_PLAYER_START);
	return 0;
}

LinphoneStatus linphone_core_play_local(LinphoneCore *lc, const char *audiofile) {
	return _linphone_core_play_local(lc, audiofile, NULL);
}

void linphone_core_play_named_tone(LinphoneCore *lc, LinphoneToneID toneid){
	if (linphone_core_tone_indications_enabled(lc)){
		const char *audiofile=linphone_core_get_tone_file(lc,toneid);
		if (!audiofile){
			MSFilter *f=get_dtmf_gen(lc, lc->sound_conf.play_sndcard);
			MSDtmfGenCustomTone def;
			if (f==NULL){
				ms_error("No dtmf generator at this time !");
				return;
			}
			memset(&def,0,sizeof(def));
			def.amplitude=1;
			/*these are french tones, excepted the failed one, which is USA congestion tone (does not exist in France)*/
			switch(toneid){
				case LinphoneToneCallOnHold:
				case LinphoneToneCallWaiting:
					def.duration=300;
					def.frequencies[0]=440;
					def.interval=2000;
				break;
				case LinphoneToneBusy:
					def.duration=500;
					def.frequencies[0]=440;
					def.interval=500;
					def.repeat_count=3;
				break;
				case LinphoneToneCallLost:
					def.duration=250;
					def.frequencies[0]=480;
					def.frequencies[0]=620;
					def.interval=250;
					def.repeat_count=3;

				break;
				default:
					ms_warning("Unhandled tone id.");
			}
			if (def.duration>0)
				ms_filter_call_method(f, MS_DTMF_GEN_PLAY_CUSTOM,&def);
		}else{
			_linphone_core_play_local(lc,audiofile, lc->sound_conf.play_sndcard);
		}
	}
}

void linphone_core_play_call_error_tone(LinphoneCore *lc, LinphoneReason reason){
	if (linphone_core_tone_indications_enabled(lc)){
		LinphoneToneDescription *tone=linphone_core_get_call_error_tone(lc,reason);
		if (tone){
			if (tone->audiofile){
				_linphone_core_play_local(lc, tone->audiofile, lc->sound_conf.play_sndcard);
			}else if (tone->toneid != LinphoneToneUndefined){
				linphone_core_play_named_tone(lc, tone->toneid);
			}
		}
	}
}

void linphone_core_stop_dtmf(LinphoneCore *lc){
	MSFilter *f=get_audio_resource(lc, LinphoneToneGenerator, NULL, FALSE);
	if (f!=NULL)
		ms_filter_call_method_noarg (f, MS_DTMF_GEN_STOP);
}

void *linphone_core_get_user_data(const LinphoneCore *lc){
	return lc->data;
}

void linphone_core_set_user_data(LinphoneCore *lc, void *userdata){
	lc->data=userdata;
}

int linphone_core_get_mtu(const LinphoneCore *lc){
	return lc->net_conf.mtu;
}

void linphone_core_set_mtu(LinphoneCore *lc, int mtu){
	lc->net_conf.mtu=mtu;
	if (mtu>0){
		if (mtu<500){
			ms_error("MTU too small !");
			mtu=500;
		}
		ms_factory_set_mtu(lc->factory, mtu);
		ms_message("MTU is supposed to be %i, rtp payload max size will be %i",mtu, ms_factory_get_payload_max_size(lc->factory));
	}else ms_factory_set_mtu(lc->factory, 0);//use mediastreamer2 default value
}

void linphone_core_set_waiting_callback(LinphoneCore *lc, LinphoneCoreWaitingCallback cb, void *user_context){
	lc->wait_cb=cb;
	lc->wait_ctx=user_context;
}

void linphone_core_start_waiting(LinphoneCore *lc, const char *purpose){
	if (lc->wait_cb){
		lc->wait_ctx=lc->wait_cb(lc,lc->wait_ctx,LinphoneWaitingStart,purpose,0);
	}
}

void linphone_core_update_progress(LinphoneCore *lc, const char *purpose, float progress){
	if (lc->wait_cb){
		lc->wait_ctx=lc->wait_cb(lc,lc->wait_ctx,LinphoneWaitingProgress,purpose,progress);
	}else{
		ms_usleep(50000);
	}
}

void linphone_core_stop_waiting(LinphoneCore *lc){
	if (lc->wait_cb){
		lc->wait_ctx=lc->wait_cb(lc,lc->wait_ctx,LinphoneWaitingFinished,NULL,0);
	}
}

void linphone_core_set_rtp_transport_factories(LinphoneCore* lc, LinphoneRtpTransportFactories *factories){
	lc->rtptf=factories;
}

int linphone_core_get_current_call_stats(LinphoneCore *lc, rtp_stats_t *local, rtp_stats_t *remote){
	LinphoneCall *call=linphone_core_get_current_call (lc);
	if (call){
		AudioStream *astream = reinterpret_cast<AudioStream *>(linphone_call_get_stream(call, LinphoneStreamTypeAudio));
		if (astream){
			memset(remote,0,sizeof(*remote));
			audio_stream_get_local_rtp_stats (astream,local);
			return 0;
		}
	}
	return -1;
}

void net_config_uninit(LinphoneCore *lc)
{
	net_config_t *config=&lc->net_conf;

	if (config->nat_address!=NULL){
		lp_config_set_string(lc->config,"net","nat_address",config->nat_address);
		ms_free(lc->net_conf.nat_address);
	}
	if (lc->net_conf.nat_address_ip !=NULL){
		ms_free(lc->net_conf.nat_address_ip);
	}
	lp_config_set_int(lc->config,"net","mtu",config->mtu);
	if (lc->nat_policy != NULL) {
		linphone_nat_policy_unref(lc->nat_policy);
		lc->nat_policy = NULL;
	}
	lc->net_conf = {0};
}

void sip_config_uninit(LinphoneCore *lc)
{
	bctbx_list_t *elem;
	int i;
	sip_config_t *config=&lc->sip_conf;
	bool_t still_registered=TRUE;

	lp_config_set_int(lc->config,"sip","guess_hostname",config->guess_hostname);
	lp_config_set_string(lc->config,"sip","contact",config->contact);
	lp_config_set_int(lc->config,"sip","inc_timeout",config->inc_timeout);
	lp_config_set_int(lc->config,"sip","in_call_timeout",config->in_call_timeout);
	lp_config_set_int(lc->config,"sip","delayed_timeout",config->delayed_timeout);
	lp_config_set_int(lc->config,"sip","register_only_when_network_is_up",config->register_only_when_network_is_up);
	lp_config_set_int(lc->config,"sip","register_only_when_upnp_is_ok",config->register_only_when_upnp_is_ok);

	if (lc->sip_network_state.global_state) {
		for(elem=config->proxies;elem!=NULL;elem=bctbx_list_next(elem)){
			LinphoneProxyConfig *cfg=(LinphoneProxyConfig*)(elem->data);
			_linphone_proxy_config_unpublish(cfg);	/* to unpublish without changing the stored flag enable_publish */
			_linphone_proxy_config_unregister(cfg);	/* to unregister without changing the stored flag enable_register */
		}

		ms_message("Unregistration started.");

		for (i=0;i<20&&still_registered;i++){
			still_registered=FALSE;
			lc->sal->iterate();
			for(elem=config->proxies;elem!=NULL;elem=bctbx_list_next(elem)){
				LinphoneProxyConfig *cfg=(LinphoneProxyConfig*)(elem->data);
				LinphoneRegistrationState state = linphone_proxy_config_get_state(cfg);
				still_registered = (state==LinphoneRegistrationOk||state==LinphoneRegistrationProgress);
			}
			ms_usleep(100000);
		}
		if (i>=20) ms_warning("Cannot complete unregistration, giving up");
	}

	elem = config->proxies;
	config->proxies=NULL; /*to make sure proxies cannot be referenced during deletion*/
	bctbx_list_free_with_data(elem,(void (*)(void*)) _linphone_proxy_config_release);

	config->deleted_proxies=bctbx_list_free_with_data(config->deleted_proxies,(void (*)(void*)) _linphone_proxy_config_release);

	/*no longuer need to write proxy config if not changed linphone_proxy_config_write_to_config_file(lc->config,NULL,i);*/	/*mark the end */

	lc->auth_info=bctbx_list_free_with_data(lc->auth_info,(void (*)(void*))linphone_auth_info_unref);
	lc->default_proxy = NULL;

	if (lc->vcard_context) {
		linphone_vcard_context_destroy(lc->vcard_context);
		lc->vcard_context = NULL;
	}

	lc->sal->resetTransports();
	lc->sal->unlistenPorts(); /*to make sure no new messages are received*/
	if (lc->http_provider) {
		belle_sip_object_unref(lc->http_provider);
		lc->http_provider=NULL;
	}
	if (lc->http_crypto_config){
		belle_sip_object_unref(lc->http_crypto_config);
		lc->http_crypto_config=NULL;
	}

	/*now that we are unregisted, there is no more channel using tunnel socket we no longer need the tunnel.*/
#ifdef TUNNEL_ENABLED
	if (lc->tunnel) {
		linphone_tunnel_unref(lc->tunnel);
		lc->tunnel=NULL;
		ms_message("Tunnel destroyed.");
	}
#endif

	lc->sal->iterate(); /*make sure event are purged*/
	delete lc->sal;
	lc->sal=NULL;


	if (lc->sip_conf.guessed_contact) {
		ms_free(lc->sip_conf.guessed_contact);
		lc->sip_conf.guessed_contact = NULL;
	}
	if (config->contact) {
		ms_free(config->contact);
		config->contact = NULL;
	}
	if (lc->default_rls_addr) {
		linphone_address_unref(lc->default_rls_addr);
		lc->default_rls_addr = NULL;
	}

	linphone_im_notif_policy_unref(lc->im_notif_policy);
	lc->im_notif_policy = NULL;
	lc->sip_conf = {0};
}

void rtp_config_uninit(LinphoneCore *lc)
{
	rtp_config_t *config=&lc->rtp_conf;
	if (config->audio_rtp_min_port == config->audio_rtp_max_port) {
		lp_config_set_int(lc->config, "rtp", "audio_rtp_port", config->audio_rtp_min_port);
	} else {
		lp_config_set_range(lc->config, "rtp", "audio_rtp_port", config->audio_rtp_min_port, config->audio_rtp_max_port);
	}
	if (config->video_rtp_min_port == config->video_rtp_max_port) {
		lp_config_set_int(lc->config, "rtp", "video_rtp_port", config->video_rtp_min_port);
	} else {
		lp_config_set_range(lc->config, "rtp", "video_rtp_port", config->video_rtp_min_port, config->video_rtp_max_port);
	}
	if (config->text_rtp_min_port == config->text_rtp_max_port) {
		lp_config_set_int(lc->config, "rtp", "text_rtp_port", config->text_rtp_min_port);
	} else {
		lp_config_set_range(lc->config, "rtp", "text_rtp_port", config->text_rtp_min_port, config->text_rtp_max_port);
	}
	lp_config_set_int(lc->config,"rtp","audio_jitt_comp",config->audio_jitt_comp);
	lp_config_set_int(lc->config,"rtp","video_jitt_comp",config->video_jitt_comp);
	lp_config_set_int(lc->config,"rtp","nortp_timeout",config->nortp_timeout);
	lp_config_set_int(lc->config,"rtp","audio_adaptive_jitt_comp_enabled",config->audio_adaptive_jitt_comp_enabled);
	lp_config_set_int(lc->config,"rtp","video_adaptive_jitt_comp_enabled",config->video_adaptive_jitt_comp_enabled);
	ms_free(lc->rtp_conf.audio_multicast_addr);
	ms_free(lc->rtp_conf.video_multicast_addr);
	ms_free(config->srtp_suites);
	lc->rtp_conf = {0};
}

static void sound_config_uninit(LinphoneCore *lc)
{
	sound_config_t *config=&lc->sound_conf;
	ms_free((void *)config->cards);

	lp_config_set_string(lc->config,"sound","remote_ring",config->remote_ring);
	lp_config_set_float(lc->config,"sound","playback_gain_db",config->soft_play_lev);
	lp_config_set_float(lc->config,"sound","mic_gain_db",config->soft_mic_lev);

	if (config->local_ring) ms_free(config->local_ring);
	if (config->remote_ring) ms_free(config->remote_ring);
	lc->tones=bctbx_list_free_with_data(lc->tones, (void (*)(void*))linphone_tone_description_destroy);
	lc->sound_conf = {0};
}

static void video_config_uninit(LinphoneCore *lc)
{
	const LinphoneVideoDefinition *vdef = linphone_core_get_preferred_video_definition(lc);
	lp_config_set_string(lc->config,"video","size",vdef ? linphone_video_definition_get_name(vdef) : NULL);
	lp_config_set_int(lc->config,"video","display",lc->video_conf.display);
	lp_config_set_int(lc->config,"video","capture",lc->video_conf.capture);
	if (lc->video_conf.cams)
		ms_free((void *)lc->video_conf.cams);
	if (lc->video_conf.vdef) linphone_video_definition_unref(lc->video_conf.vdef);
	if (lc->video_conf.preview_vdef) linphone_video_definition_unref(lc->video_conf.preview_vdef);
	lc->video_conf = {0};
}

void _linphone_core_codec_config_write(LinphoneCore *lc){
	if (linphone_core_ready(lc)){
		PayloadType *pt;
		codecs_config_t *config=&lc->codecs_conf;
		bctbx_list_t *node;
		char key[50];
		int index;
		index=0;
		for(node=config->audio_codecs;node!=NULL;node=bctbx_list_next(node)){
			pt=(PayloadType*)(node->data);
			sprintf(key,"audio_codec_%i",index);
			lp_config_set_string(lc->config,key,"mime",pt->mime_type);
			lp_config_set_int(lc->config,key,"rate",pt->clock_rate);
			lp_config_set_int(lc->config,key,"channels",pt->channels);
			lp_config_set_int(lc->config,key,"enabled",payload_type_enabled(pt));
			index++;
		}
		sprintf(key,"audio_codec_%i",index);
		lp_config_clean_section (lc->config,key);

		index=0;
		for(node=config->video_codecs;node!=NULL;node=bctbx_list_next(node)){
			pt=(PayloadType*)(node->data);
			sprintf(key,"video_codec_%i",index);
			lp_config_set_string(lc->config,key,"mime",pt->mime_type);
			lp_config_set_int(lc->config,key,"rate",pt->clock_rate);
			lp_config_set_int(lc->config,key,"enabled",payload_type_enabled(pt));
			lp_config_set_string(lc->config,key,"recv_fmtp",pt->recv_fmtp);
			index++;
		}
		sprintf(key,"video_codec_%i",index);
		lp_config_clean_section (lc->config,key);
	}
}

static void codecs_config_uninit(LinphoneCore *lc)
{
	_linphone_core_codec_config_write(lc);
	bctbx_list_free(lc->codecs_conf.audio_codecs);
	bctbx_list_free(lc->codecs_conf.video_codecs);
	bctbx_list_free(lc->codecs_conf.text_codecs);
	lc->codecs_conf = {0};
}

void friends_config_uninit(LinphoneCore* lc)
{
	ms_message("Destroying friends.");
	lc->friends_lists = bctbx_list_free_with_data(lc->friends_lists, (void (*)(void*))_linphone_friend_list_release);
	if (lc->subscribers) {
		lc->subscribers = bctbx_list_free_with_data(lc->subscribers, (void (*)(void *))_linphone_friend_release);
	}
	if (lc->presence_model) {
		linphone_presence_model_unref(lc->presence_model);
		lc->presence_model = NULL;
	}
	ms_message("Destroying friends done.");
}

void linphone_core_enter_background(LinphoneCore *lc) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->enterBackground();
}

void linphone_core_enter_foreground(LinphoneCore *lc) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->enterForeground();
}

LpConfig * linphone_core_get_config(const LinphoneCore *lc){
	return lc->config;
}

LpConfig * linphone_core_create_lp_config(LinphoneCore *lc, const char *filename) {
	return lp_config_new(filename);
}

LinphoneConfig * linphone_core_create_config(LinphoneCore *lc, const char *filename) {
	return lp_config_new(filename);
}

LinphoneAddress * linphone_core_create_address(LinphoneCore *lc, const char *address) {
	return linphone_address_new(address);
}

LinphoneXmlRpcSession * linphone_core_create_xml_rpc_session(LinphoneCore *lc, const char *url) {
	return linphone_xml_rpc_session_new(lc, url);
}

static void _linphone_core_stop(LinphoneCore *lc) {
	bctbx_list_t *elem = NULL;
	int i=0;
	bool_t wait_until_unsubscribe = FALSE;
	linphone_task_list_free(&lc->hooks);
	lc->video_conf.show_local = FALSE;

	linphone_core_set_state(lc, LinphoneGlobalShutdown, "Shutdown");

	L_GET_PRIVATE_FROM_C_OBJECT(lc)->uninit();

	for (elem = lc->friends_lists; elem != NULL; elem = bctbx_list_next(elem)) {
		LinphoneFriendList *list = (LinphoneFriendList *)elem->data;
		linphone_friend_list_enable_subscriptions(list,FALSE);
		if (list->event)
			wait_until_unsubscribe =  TRUE;
	}
	/*give a chance to unsubscribe, might be optimized*/
	for (i=0; wait_until_unsubscribe && i<50; i++) {
		linphone_core_iterate(lc);
		ms_usleep(10000);
	}

	lc->chat_rooms = bctbx_list_free_with_data(lc->chat_rooms, (bctbx_list_free_func)linphone_chat_room_unref);

	getPlatformHelpers(lc)->onLinphoneCoreStop();

#ifdef VIDEO_ENABLED
	if (lc->previewstream!=NULL){
		video_preview_stop(lc->previewstream);
		lc->previewstream=NULL;
	}
#endif

	lc->msevq=NULL;

	/* save all config */
	friends_config_uninit(lc);
	sip_config_uninit(lc);
	net_config_uninit(lc);
	rtp_config_uninit(lc);
	linphone_core_stop_ringing(lc);
	sound_config_uninit(lc);
	video_config_uninit(lc);
	codecs_config_uninit(lc);

	sip_setup_unregister_all();

	if (lp_config_needs_commit(lc->config)) lp_config_sync(lc->config);

	bctbx_list_for_each(lc->call_logs,(void (*)(void*))linphone_call_log_unref);
	lc->call_logs=bctbx_list_free(lc->call_logs);

	if(lc->zrtp_secrets_cache != NULL) {
		ms_free(lc->zrtp_secrets_cache);
		lc->zrtp_secrets_cache = NULL;
	}

	if(lc->user_certificates_path != NULL) {
		ms_free(lc->user_certificates_path);
		lc->user_certificates_path = NULL;
	}
	if(lc->play_file!=NULL){
		ms_free(lc->play_file);
		lc->play_file = NULL;
	}
	if(lc->rec_file!=NULL){
		ms_free(lc->rec_file);
		lc->rec_file = NULL;
	}
	if (lc->logs_db_file) {
		ms_free(lc->logs_db_file);
		lc->logs_db_file = NULL;
	}
	if (lc->friends_db_file) {
		ms_free(lc->friends_db_file);
		lc->friends_db_file = NULL;
	}
	if (lc->tls_key){
		ms_free(lc->tls_key);
		lc->tls_key = NULL;
	}
	if (lc->tls_cert){
		ms_free(lc->tls_cert);
		lc->tls_cert = NULL;
	}
	if (lc->ringtoneplayer) {
		linphone_ringtoneplayer_destroy(lc->ringtoneplayer);
		lc->ringtoneplayer = NULL;
	}
	if (lc->im_encryption_engine) {
		linphone_im_encryption_engine_unref(lc->im_encryption_engine);
		lc->im_encryption_engine = NULL;
	}
	if (lc->default_ac_service) {
		linphone_account_creator_service_unref(lc->default_ac_service);
		lc->default_ac_service = NULL;
	}

	linphone_core_free_payload_types(lc);
	if (lc->supported_formats) ms_free((void *)lc->supported_formats);
	lc->supported_formats = NULL;
	linphone_core_call_log_storage_close(lc);
	linphone_core_friends_storage_close(lc);
	linphone_core_zrtp_cache_close(lc);
	ms_bandwidth_controller_destroy(lc->bw_controller);
	lc->bw_controller = NULL;
	ms_factory_destroy(lc->factory);
	lc->factory = NULL;

	if (lc->platform_helper) delete getPlatformHelpers(lc);
	lc->platform_helper = NULL;
	linphone_core_set_state(lc, LinphoneGlobalOff, "Off");
}

void linphone_core_stop(LinphoneCore *lc) {
	_linphone_core_stop(lc);
}

void _linphone_core_uninit(LinphoneCore *lc)
{
	lc->is_unreffing = TRUE;
	if (lc->state != LinphoneGlobalOff) {
		_linphone_core_stop(lc);
	}
	
	lp_config_unref(lc->config);
	lc->config = NULL;
#ifdef __ANDROID__
	if (lc->system_context) {
		JNIEnv *env = ms_get_jni_env();
		env->DeleteGlobalRef((jobject)lc->system_context);
	}
#endif
	lc->system_context = NULL;
	
	linphone_core_deactivate_log_serialization_if_needed();
	bctbx_list_free_with_data(lc->vtable_refs,(void (*)(void *))v_table_reference_destroy);
	bctbx_uninit_logger();
}

static void stop_refreshing_proxy_config(bool_t is_sip_reachable, LinphoneProxyConfig* cfg) {
	if (linphone_proxy_config_register_enabled(cfg) ) {
		if (!is_sip_reachable) {
			linphone_proxy_config_stop_refreshing(cfg);
			linphone_proxy_config_set_state(cfg, LinphoneRegistrationNone,"Registration impossible (network down)");
		}else{
			cfg->commit=TRUE;
			if (linphone_proxy_config_publish_enabled(cfg))
				cfg->send_publish=TRUE; /*not sure if really the best place*/
		}
	}
}

static void set_sip_network_reachable(LinphoneCore* lc,bool_t is_sip_reachable, time_t curtime){
	// second get the list of available proxies
	const bctbx_list_t *elem = NULL;

	if (lc->sip_network_state.global_state==is_sip_reachable) return; // no change, ignore.
	lc->network_reachable_to_be_notified=TRUE;

	if (is_sip_reachable){
		getPlatformHelpers(lc)->setDnsServers();
		if (lc->sip_conf.guess_hostname) update_primary_contact(lc);
	}

	ms_message("SIP network reachability state is now [%s]",is_sip_reachable?"UP":"DOWN");
	for(elem=linphone_core_get_proxy_config_list(lc);elem!=NULL;elem=elem->next){
		LinphoneProxyConfig *cfg=(LinphoneProxyConfig*)elem->data;
		stop_refreshing_proxy_config(is_sip_reachable, cfg);
	}
	for(elem=lc->sip_conf.deleted_proxies;elem!=NULL;elem=elem->next){
		LinphoneProxyConfig *deleted_cfg=(LinphoneProxyConfig*)elem->data;
		stop_refreshing_proxy_config(is_sip_reachable, deleted_cfg);
	}

	lc->netup_time=curtime;
	lc->sip_network_state.global_state=is_sip_reachable;

	if (!lc->sip_network_state.global_state){
		linphone_core_invalidate_friend_subscriptions(lc);
		lc->sal->resetTransports();
	}
}

static void set_media_network_reachable(LinphoneCore* lc, bool_t is_media_reachable){
	if (lc->media_network_state.global_state==is_media_reachable) return; // no change, ignore.
	lc->network_reachable_to_be_notified=TRUE;

	ms_message("Media network reachability state is now [%s]",is_media_reachable?"UP":"DOWN");
	lc->media_network_state.global_state=is_media_reachable;

	if (lc->media_network_state.global_state){
		if (lc->bw_controller){
			ms_bandwidth_controller_reset_state(lc->bw_controller);
		}
	}
}

void linphone_core_refresh_registers(LinphoneCore* lc) {
	const bctbx_list_t *elem;
	if (!lc->sip_network_state.global_state) {
		ms_warning("Refresh register operation not available (network unreachable)");
		return;
	}
	elem=linphone_core_get_proxy_config_list(lc);
	for(;elem!=NULL;elem=elem->next){
		LinphoneProxyConfig *cfg=(LinphoneProxyConfig*)elem->data;
		if (linphone_proxy_config_register_enabled(cfg) && linphone_proxy_config_get_expires(cfg)>0) {
			linphone_proxy_config_refresh_register(cfg);
		}
	}
}

void linphone_core_set_network_reachable_internal(LinphoneCore *lc, bool_t is_reachable) {
	if (lc->auto_net_state_mon) {
		set_sip_network_reachable(lc, lc->sip_network_state.user_state && is_reachable, ms_time(NULL));
		set_media_network_reachable(lc, lc->media_network_state.user_state && is_reachable);
		notify_network_reachable_change(lc);
	}
}

void linphone_core_set_network_reachable(LinphoneCore *lc, bool_t is_reachable) {
	bool_t reachable = is_reachable;

	lc->sip_network_state.user_state = is_reachable;
	lc->media_network_state.user_state = is_reachable;

	if (lc->auto_net_state_mon) reachable = reachable && getPlatformHelpers(lc)->isNetworkReachable();

	set_sip_network_reachable(lc, reachable, ms_time(NULL));
	set_media_network_reachable(lc, reachable);
	notify_network_reachable_change(lc);
}

void linphone_core_set_media_network_reachable(LinphoneCore *lc, bool_t is_reachable){
	bool_t reachable = is_reachable;

	lc->media_network_state.user_state = is_reachable;

	if (lc->auto_net_state_mon) reachable = reachable && getPlatformHelpers(lc)->isNetworkReachable();

	set_media_network_reachable(lc, reachable);
	notify_network_reachable_change(lc);
}

void linphone_core_set_sip_network_reachable(LinphoneCore *lc, bool_t is_reachable){
	bool_t reachable = is_reachable;

	lc->sip_network_state.user_state = is_reachable;

	if (lc->auto_net_state_mon) reachable = reachable && getPlatformHelpers(lc)->isNetworkReachable();

	set_sip_network_reachable(lc, reachable, ms_time(NULL));
	notify_network_reachable_change(lc);
}

bool_t linphone_core_is_network_reachable(LinphoneCore* lc) {
	return lc->sip_network_state.global_state;
}

ortp_socket_t linphone_core_get_sip_socket(LinphoneCore *lc){
	ms_warning("linphone_core_get_sip_socket is deprecated");
	return -1;
}

void linphone_core_destroy(LinphoneCore *lc){
	linphone_core_unref(lc);
}

int linphone_core_get_calls_nb(const LinphoneCore *lc) {
	return (int)L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getCallCount();
}


void linphone_core_soundcard_hint_check(LinphoneCore* lc) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->soundcardHintCheck();
}

void linphone_core_set_remote_ringback_tone(LinphoneCore *lc, const char *file){
	if (lc->sound_conf.ringback_tone){
		ms_free(lc->sound_conf.ringback_tone);
		lc->sound_conf.ringback_tone=NULL;
	}
	if (file)
		lc->sound_conf.ringback_tone=ms_strdup(file);
}

const char *linphone_core_get_remote_ringback_tone(const LinphoneCore *lc){
	return lc->sound_conf.ringback_tone;
}

void linphone_core_set_ring_during_incoming_early_media(LinphoneCore *lc, bool_t enable) {
	lp_config_set_int(lc->config, "sound", "ring_during_incoming_early_media", (int)enable);
}

bool_t linphone_core_get_ring_during_incoming_early_media(const LinphoneCore *lc) {
	return (bool_t)lp_config_get_int(lc->config, "sound", "ring_during_incoming_early_media", 0);
}

static OrtpPayloadType* _linphone_core_find_payload_type(LinphoneCore* lc, const char* type, int rate, int channels) {
	OrtpPayloadType* result = find_payload_type_from_list(type, rate, channels, lc->codecs_conf.audio_codecs);
	if (result)  {
		return result;
	} else {
		result = find_payload_type_from_list(type, rate, 0, lc->codecs_conf.video_codecs);
		if (result) {
			return result;
		} else {
			result = find_payload_type_from_list(type, rate, 0, lc->codecs_conf.text_codecs);
			if (result) {
				return result;
			}
		}
	}
	/*not found*/
	return NULL;
}

OrtpPayloadType* linphone_core_find_payload_type(LinphoneCore* lc, const char* type, int rate, int channels) {
	return _linphone_core_find_payload_type(lc, type, rate, channels);
}

LinphonePayloadType *linphone_core_get_payload_type(LinphoneCore *lc, const char *type, int rate, int channels) {
	OrtpPayloadType *pt = _linphone_core_find_payload_type(lc, type, rate, channels);
	return pt ? linphone_payload_type_new(lc, pt) : NULL;
}

const char* linphone_configuring_state_to_string(LinphoneConfiguringState cs){
	switch(cs){
		case LinphoneConfiguringSuccessful:
			return "LinphoneConfiguringSuccessful";
		case LinphoneConfiguringFailed:
			return "LinphoneConfiguringFailed";
		case LinphoneConfiguringSkipped:
			return "LinphoneConfiguringSkipped";
	}
	return NULL;
}

const char *linphone_global_state_to_string(LinphoneGlobalState gs){
	switch(gs){
		case LinphoneGlobalOff:
			return "LinphoneGlobalOff";
		case LinphoneGlobalOn:
			return "LinphoneGlobalOn";
		case LinphoneGlobalStartup:
			return "LinphoneGlobalStartup";
		case LinphoneGlobalShutdown:
			return "LinphoneGlobalShutdown";
		case LinphoneGlobalConfiguring:
			return "LinphoneGlobalConfiguring";
		case LinphoneGlobalReady:
			return "LinphoneGlobalReady";
		break;
	}
	return NULL;
}

LinphoneGlobalState linphone_core_get_global_state(const LinphoneCore *lc){
	return lc->state;
}


LinphoneCallParams *linphone_core_create_call_params(LinphoneCore *lc, LinphoneCall *call){
	if (!call) return linphone_call_params_new(lc);
	if (linphone_call_get_params(call)){
		return linphone_call_params_copy(linphone_call_get_params(call));
	}
	ms_error("linphone_core_create_call_params(): call [%p] is not in a state where call params can be created or used.", call);
	return NULL;
}

const char *linphone_error_to_string(LinphoneReason err){
	return linphone_reason_to_string(err);
}

void linphone_core_enable_keep_alive(LinphoneCore* lc,bool_t enable) {
	if (enable > 0) {
		lc->sal->useTcpTlsKeepAlive(!!lc->sip_conf.tcp_tls_keepalive);
		lc->sal->setKeepAlivePeriod(lc->sip_conf.keepalive_period);
	} else {
		lc->sal->setKeepAlivePeriod(0);
	}
}

bool_t linphone_core_keep_alive_enabled(LinphoneCore* lc) {
	return lc->sal->getKeepAlivePeriod() > 0;
}

void linphone_core_start_dtmf_stream(LinphoneCore* lc) {
	get_dtmf_gen(lc, lc->sound_conf.ring_sndcard); /*make sure ring stream is started*/
	lc->ringstream_autorelease=FALSE; /*disable autorelease mode*/
}

void linphone_core_stop_ringing(LinphoneCore* lc) {
	LinphoneCall *call=linphone_core_get_current_call(lc);
	if (linphone_ringtoneplayer_is_started(lc->ringtoneplayer)) {
		linphone_ringtoneplayer_stop(lc->ringtoneplayer);
	}
	if (lc->ringstream) {
		ring_stop(lc->ringstream);
		lc->ringstream=NULL;
		lc->dmfs_playing_start_time=0;
		lc->ringstream_autorelease=TRUE;
	}
	if (call && L_GET_PRIVATE_FROM_C_OBJECT(call)->getRingingBeep()) {
		linphone_core_stop_dtmf(lc);
		L_GET_PRIVATE_FROM_C_OBJECT(call)->setRingingBeep(false);
	}
}

void linphone_core_stop_dtmf_stream(LinphoneCore* lc) {
	if (lc->dmfs_playing_start_time!=0) {
		linphone_core_stop_ringing(lc);
	}
}

int linphone_core_get_max_calls(LinphoneCore *lc) {
	return lc->max_calls;
}

void linphone_core_set_max_calls(LinphoneCore *lc, int max) {
	lc->max_calls=max;
}

void linphone_core_add_iterate_hook(LinphoneCore *lc, LinphoneCoreIterateHook hook, void *hook_data){
	linphone_task_list_add(&lc->hooks, hook, hook_data);
}

static void linphone_core_run_hooks(LinphoneCore *lc){
	linphone_task_list_run(&lc->hooks);
}

void linphone_core_remove_iterate_hook(LinphoneCore *lc, LinphoneCoreIterateHook hook, void *hook_data){
	linphone_task_list_remove(&lc->hooks, hook, hook_data);

}

// =============================================================================
// TODO: Remove me later, code found in message_storage.c.
// =============================================================================

int _linphone_sqlite3_open(const char *db_file, sqlite3 **db) {
	char* errmsg = NULL;
	int ret;
	int flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE;

#if TARGET_OS_IPHONE
	/* the secured filesystem of the iPHone doesn't allow writing while the app is in background mode, which is problematic.
	 * We workaround by asking that the open is made with no protection*/
	flags |= SQLITE_OPEN_FILEPROTECTION_NONE;
#endif

	/*since we plug our vfs into sqlite, we convert to UTF-8.
	 * On Windows, the filename has to be converted back to windows native charset.*/
	char *utf8_filename = bctbx_locale_to_utf8(db_file);
	ret = sqlite3_open_v2(utf8_filename, db, flags, LINPHONE_SQLITE3_VFS);
	ms_free(utf8_filename);

	if (ret != SQLITE_OK) return ret;
	// Some platforms do not provide a way to create temporary files which are needed
	// for transactions... so we work in memory only
	// see http ://www.sqlite.org/compile.html#temp_store
	ret = sqlite3_exec(*db, "PRAGMA temp_store=MEMORY", NULL, NULL, &errmsg);
	if (ret != SQLITE_OK) {
		ms_error("Cannot set sqlite3 temporary store to memory: %s.", errmsg);
		sqlite3_free(errmsg);
	}

	return ret;
}

// =============================================================================
//migration code remove in april 2019, 2 years after switching from xml based zrtp cache to sqlite
void linphone_core_set_zrtp_secrets_file(LinphoneCore *lc, const char* file){
	if (lc->zrtp_secrets_cache != NULL) {
		ms_free(lc->zrtp_secrets_cache);
		linphone_core_zrtp_cache_close(lc);
		lc->zrtp_secrets_cache = NULL;
	}
	
	if (file) {
		lc->zrtp_secrets_cache = ms_strdup(file);
		linphone_core_zrtp_cache_db_init(lc, file);
	}
}

const char *linphone_core_get_zrtp_secrets_file(LinphoneCore *lc){
	return lc->zrtp_secrets_cache;
}

zrtpCacheAccess linphone_core_get_zrtp_cache_access(LinphoneCore *lc){
	zrtpCacheAccess ret;
	ret.db = (void *)lc->zrtp_cache_db;
	ret.dbMutex = &(lc->zrtp_cache_db_mutex);
	return ret;
}

void *linphone_core_get_zrtp_cache_db(LinphoneCore *lc){
	return (void *)lc->zrtp_cache_db;
}

LinphoneZrtpPeerStatus linphone_core_get_zrtp_status(LinphoneCore *lc, const char *peerUri) {
	int status = MS_ZRTP_PEER_STATUS_UNKNOWN;
	if (lc->zrtp_cache_db) {
		status = ms_zrtp_get_peer_status(lc->zrtp_cache_db, peerUri, &(lc->zrtp_cache_db_mutex));
	}
	switch (status) {
		case MS_ZRTP_PEER_STATUS_UNKNOWN:
			return LinphoneZrtpPeerStatusUnknown;
		case MS_ZRTP_PEER_STATUS_INVALID:
			return LinphoneZrtpPeerStatusInvalid;
		case MS_ZRTP_PEER_STATUS_VALID:
			return LinphoneZrtpPeerStatusValid;
		default:
			return LinphoneZrtpPeerStatusUnknown;
	}
}

static void linphone_core_zrtp_cache_close(LinphoneCore *lc) {
	if (lc->zrtp_cache_db) {
		sqlite3_close(lc->zrtp_cache_db);
		bctbx_mutex_destroy(&(lc->zrtp_cache_db_mutex));
		lc->zrtp_cache_db = NULL;
	}
}


void linphone_core_zrtp_cache_db_init(LinphoneCore *lc, const char *fileName) {
	int ret;
	const char *errmsg;
	const char *backupExtension = "_backup";
	char *backupName = bctbx_strdup_printf("%s%s", fileName, backupExtension);
	sqlite3 *db;

	linphone_core_zrtp_cache_close(lc);

	bctbx_mutex_init(&(lc->zrtp_cache_db_mutex), NULL);

	ret = _linphone_sqlite3_open(fileName, &db);
	if (ret != SQLITE_OK) {
		errmsg = sqlite3_errmsg(db);
		ms_error("Error in the opening zrtp_cache_db_file(%s): %s.\n", fileName, errmsg);
		sqlite3_close(db);
		unlink(backupName);
		rename(fileName, backupName);
		lc->zrtp_cache_db=NULL;
		goto end;
	}

	ret = ms_zrtp_initCache((void *)db, &(lc->zrtp_cache_db_mutex)); /* this may perform an update, check return value */

	if (ret == MSZRTP_CACHE_SETUP || ret == MSZRTP_CACHE_UPDATE) {
		/* After updating schema, database need to be closed/reopenned */
		sqlite3_close(db);
		_linphone_sqlite3_open(fileName, &db);
	} else if(ret != 0) { /* something went wrong */
		ms_error("Zrtp cache failed to initialise(returned -%x), run cacheless", -ret);
		sqlite3_close(db);
		unlink(backupName);
		rename(fileName, backupName);
		lc->zrtp_cache_db = NULL;
		goto end;
	}

	/* everything ok, set the db pointer into core */
	lc->zrtp_cache_db = db;
end:
	if (backupName) bctbx_free(backupName);
}

void linphone_core_set_user_certificates_path (LinphoneCore *lc, const char *path) {
	char *new_value = path ? bctbx_strdup(path) : NULL;
	if (lc->user_certificates_path) bctbx_free(lc->user_certificates_path);
	lc->user_certificates_path = new_value;
	lp_config_set_string(lc->config, "misc", "user_certificates_path", lc->user_certificates_path);
}

const char *linphone_core_get_user_certificates_path(LinphoneCore *lc){
	return lc->user_certificates_path;
}

bool_t linphone_core_sound_resources_locked(LinphoneCore *lc) {
	return !!L_GET_CPP_PTR_FROM_C_OBJECT(lc)->areSoundResourcesLocked();
}

void linphone_core_set_srtp_enabled(LinphoneCore *lc, bool_t enabled) {
	lp_config_set_int(lc->config,"sip","srtp",(int)enabled);
}

const char *linphone_media_encryption_to_string(LinphoneMediaEncryption menc){
	switch(menc){
		case LinphoneMediaEncryptionSRTP:
			return "LinphoneMediaEncryptionSRTP";
		case LinphoneMediaEncryptionDTLS:
			return "LinphoneMediaEncryptionDTLS";
		case LinphoneMediaEncryptionZRTP:
			return "LinphoneMediaEncryptionZRTP";
		case LinphoneMediaEncryptionNone:
			return "LinphoneMediaEncryptionNone";
	}
	ms_error("Invalid LinphoneMediaEncryption value %i",(int)menc);
	return "INVALID";
}


bool_t linphone_core_media_encryption_supported(const LinphoneCore *lc, LinphoneMediaEncryption menc){
	switch(menc){
		case LinphoneMediaEncryptionSRTP:
			return ms_srtp_supported();
		case LinphoneMediaEncryptionDTLS:
			return ms_dtls_srtp_available();
		case LinphoneMediaEncryptionZRTP:
			return ms_zrtp_available() && !lc->zrtp_not_available_simulation;
		case LinphoneMediaEncryptionNone:
			return TRUE;
	}
	return FALSE;
}

LinphoneStatus linphone_core_set_media_encryption(LinphoneCore *lc, LinphoneMediaEncryption menc) {
	const char *type="none";
	int ret=-1;

	switch(menc){
		case LinphoneMediaEncryptionSRTP:
			if (!ms_srtp_supported()){
				ms_warning("SRTP not supported by library.");
				type="none";
				ret=-1;
			}else{
				type="srtp";
				ret = 0;
			}
		break;
		case LinphoneMediaEncryptionZRTP:
			if (!linphone_core_media_encryption_supported(lc, LinphoneMediaEncryptionZRTP)){
				ms_warning("ZRTP not supported by library.");
				type="none";
				ret=-1;
			}else {
				type="zrtp";
				ret = 0;
			}
		break;
		case LinphoneMediaEncryptionDTLS:
			if (!ms_dtls_srtp_available()){
				ms_warning("DTLS not supported by library.");
				type="none";
				ret=-1;
			}else {
				type="dtls";
				ret = 0;
			}
		break;
		case LinphoneMediaEncryptionNone:
			type = "none";
			ret = 0;
		break;
	}
	lp_config_set_string(lc->config,"sip","media_encryption",type);
	return ret;
}

LinphoneMediaEncryption linphone_core_get_media_encryption(LinphoneCore *lc) {
	const char* menc = lp_config_get_string(lc->config, "sip", "media_encryption", NULL);

	if (menc == NULL)
		return LinphoneMediaEncryptionNone;
	else if (strcmp(menc, "srtp")==0)
		return LinphoneMediaEncryptionSRTP;
	else if (strcmp(menc, "dtls")==0)
		return LinphoneMediaEncryptionDTLS;
	else if (strcmp(menc, "zrtp")==0)
		return LinphoneMediaEncryptionZRTP;
	else
		return LinphoneMediaEncryptionNone;
}

bool_t linphone_core_is_media_encryption_mandatory(LinphoneCore *lc) {
	return (bool_t)lp_config_get_int(lc->config, "sip", "media_encryption_mandatory", 0);
}

void linphone_core_set_media_encryption_mandatory(LinphoneCore *lc, bool_t m) {
	lp_config_set_int(lc->config, "sip", "media_encryption_mandatory", (int)m);
}

void linphone_core_init_default_params(LinphoneCore*lc, LinphoneCallParams *params) {
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->initDefault(L_GET_CPP_PTR_FROM_C_OBJECT(lc));
}

void linphone_core_set_device_identifier(LinphoneCore *lc,const char* device_id) {
	if (lc->device_id) ms_free(lc->device_id);
	lc->device_id=ms_strdup(device_id);
}

const char*  linphone_core_get_device_identifier(const LinphoneCore *lc) {
	return lc->device_id;
}

void linphone_core_set_sip_dscp(LinphoneCore *lc, int dscp){
	lc->sal->setDscp(dscp);
	if (linphone_core_ready(lc)){
		lp_config_set_int_hex(lc->config,"sip","dscp",dscp);
		_linphone_core_apply_transports(lc);
	}
}

int linphone_core_get_sip_dscp(const LinphoneCore *lc){
	return lp_config_get_int(lc->config,"sip","dscp",0x1a);
}

void linphone_core_set_audio_dscp(LinphoneCore *lc, int dscp){
	if (linphone_core_ready(lc))
		lp_config_set_int_hex(lc->config,"rtp","audio_dscp",dscp);
}

int linphone_core_get_audio_dscp(const LinphoneCore *lc){
	return lp_config_get_int(lc->config,"rtp","audio_dscp",0x2e);
}

void linphone_core_set_video_dscp(LinphoneCore *lc, int dscp){
	if (linphone_core_ready(lc))
		lp_config_set_int_hex(lc->config,"rtp","video_dscp",dscp);

}

int linphone_core_get_video_dscp(const LinphoneCore *lc){
	return lp_config_get_int(lc->config,"rtp","video_dscp",0);
}

void linphone_core_set_chat_database_path (LinphoneCore *lc, const char *path) {
	if (!linphone_core_conference_server_enabled(lc)) {
		auto &mainDb = L_GET_PRIVATE_FROM_C_OBJECT(lc)->mainDb;
		if (mainDb) {
			mainDb->import(LinphonePrivate::MainDb::Sqlite3, path);
			L_GET_PRIVATE_FROM_C_OBJECT(lc)->loadChatRooms();
		} else {
			ms_warning("linphone_core_set_chat_database_path() needs to be called once linphone_core_start() has been called");
		}
	}
}

const char *linphone_core_get_chat_database_path (const LinphoneCore *) {
	lError() << "Do not use `linphone_core_get_chat_database_path`. Not necessary.";
	return "";
}

void linphone_core_enable_sdp_200_ack(LinphoneCore *lc, bool_t enable) {
	lp_config_set_int(lc->config,"sip","sdp_200_ack",lc->sip_conf.sdp_200_ack=enable);
}

bool_t linphone_core_sdp_200_ack_enabled(const LinphoneCore *lc) {
	return lc->sip_conf.sdp_200_ack!=0;
}

void linphone_core_set_file_transfer_server(LinphoneCore *core, const char * server_url) {
	lp_config_set_string(core->config, "misc", "file_transfer_server_url", server_url);
}

const char * linphone_core_get_file_transfer_server(LinphoneCore *core) {
	return lp_config_get_string(core->config, "misc", "file_transfer_server_url", NULL);
}

void linphone_core_add_supported_tag(LinphoneCore *lc, const char *tag){
	lc->sal->addSupportedTag(tag);
	lp_config_set_string(lc->config,"sip","supported",lc->sal->getSupportedTags().c_str());
}

void linphone_core_remove_supported_tag(LinphoneCore *lc, const char *tag){
	lc->sal->removeSupportedTag(tag);
	lp_config_set_string(lc->config,"sip","supported",lc->sal->getSupportedTags().c_str());
}

void linphone_core_set_avpf_mode(LinphoneCore *lc, LinphoneAVPFMode mode){
	if (mode==LinphoneAVPFDefault) mode=LinphoneAVPFDisabled;
	lc->rtp_conf.avpf_mode=mode;
	if (linphone_core_ready(lc)) lp_config_set_int(lc->config,"rtp","avpf",mode);
}

LinphoneAVPFMode linphone_core_get_avpf_mode(const LinphoneCore *lc){
	return lc->rtp_conf.avpf_mode;
}

int linphone_core_get_avpf_rr_interval(const LinphoneCore *lc){
	return lp_config_get_int(lc->config,"rtp","avpf_rr_interval",5);
}

void linphone_core_set_avpf_rr_interval(LinphoneCore *lc, int interval){
	lp_config_set_int(lc->config,"rtp","avpf_rr_interval",interval);
}

LinphoneStatus linphone_core_set_audio_multicast_addr(LinphoneCore *lc, const char* ip) {
	char* new_value;
	if (ip && !ms_is_multicast(ip)) {
		ms_error("Cannot set multicast audio addr to core [%p] because [%s] is not multicast",lc,ip);
		return -1;
	}
	new_value = ip?ms_strdup(ip):NULL;
	if (lc->rtp_conf.audio_multicast_addr) ms_free(lc->rtp_conf.audio_multicast_addr);
	lp_config_set_string(lc->config,"rtp","audio_multicast_addr",lc->rtp_conf.audio_multicast_addr=new_value);
	return 0;
}

LinphoneStatus linphone_core_set_video_multicast_addr(LinphoneCore *lc, const char* ip) {
	char* new_value;
	if (ip && !ms_is_multicast(ip)) {
		ms_error("Cannot set multicast video addr to core [%p] because [%s] is not multicast",lc,ip);
		return -1;
	}
	new_value = ip?ms_strdup(ip):NULL;
	if (lc->rtp_conf.video_multicast_addr) ms_free(lc->rtp_conf.video_multicast_addr);
	lp_config_set_string(lc->config,"rtp","video_multicast_addr",lc->rtp_conf.video_multicast_addr=new_value);
	return 0;
}

const char* linphone_core_get_audio_multicast_addr(const LinphoneCore *lc) {
	return lc->rtp_conf.audio_multicast_addr;
}

const char* linphone_core_get_video_multicast_addr(const LinphoneCore *lc){
	return lc->rtp_conf.video_multicast_addr;
}

LinphoneStatus linphone_core_set_audio_multicast_ttl(LinphoneCore *lc, int ttl) {
	if (ttl>255) {
		ms_error("Cannot set multicast audio ttl to core [%p] to [%i] value must be <256",lc,ttl);
		return -1;
	}

	lp_config_set_int(lc->config,"rtp","audio_multicast_ttl",lc->rtp_conf.audio_multicast_ttl=ttl);
	return 0;
}

LinphoneStatus linphone_core_set_video_multicast_ttl(LinphoneCore *lc, int ttl) {
	if (ttl>255) {
		ms_error("Cannot set multicast video ttl to core [%p] to [%i] value must be <256",lc,ttl);
		return -1;
	}

	lp_config_set_int(lc->config,"rtp","video_multicast_ttl",lc->rtp_conf.video_multicast_ttl=ttl);
	return 0;
}

int linphone_core_get_audio_multicast_ttl(const LinphoneCore *lc) {
	return lc->rtp_conf.audio_multicast_ttl;
}


int linphone_core_get_video_multicast_ttl(const LinphoneCore *lc){
	return lc->rtp_conf.video_multicast_ttl;
}

void linphone_core_enable_audio_multicast(LinphoneCore *lc, bool_t yesno) {
	lp_config_set_int(lc->config,"rtp","audio_multicast_enabled",lc->rtp_conf.audio_multicast_enabled=yesno);
}

 bool_t linphone_core_audio_multicast_enabled(const LinphoneCore *lc) {
	return lc->rtp_conf.audio_multicast_enabled;
}

void linphone_core_enable_video_multicast(LinphoneCore *lc, bool_t yesno) {
	lp_config_set_int(lc->config,"rtp","video_multicast_enabled",lc->rtp_conf.video_multicast_enabled=yesno);
}

bool_t linphone_core_video_multicast_enabled(const LinphoneCore *lc) {
	return lc->rtp_conf.video_multicast_enabled;
}

void linphone_core_set_video_preset(LinphoneCore *lc, const char *preset) {
	lp_config_set_string(lc->config, "video", "preset", preset);
}

const char * linphone_core_get_video_preset(const LinphoneCore *lc) {
	return lp_config_get_string(lc->config, "video", "preset", NULL);
}



LINPHONE_PUBLIC const char *linphone_core_log_collection_upload_state_to_string(const LinphoneCoreLogCollectionUploadState lcus) {
	switch (lcus) {
	case LinphoneCoreLogCollectionUploadStateInProgress : return "LinphoneCoreLogCollectionUploadStateInProgress";
	case LinphoneCoreLogCollectionUploadStateDelivered : return "LinphoneCoreLogCollectionUploadStateDelivered";
	case LinphoneCoreLogCollectionUploadStateNotDelivered : return "LinphoneCoreLogCollectionUploadStateNotDelivered";
	}
	return "UNKNOWN";
}

bool_t linphone_core_realtime_text_enabled(LinphoneCore *lc) {
	return lc->text_conf.enabled;
}

void linphone_core_enable_realtime_text(LinphoneCore *lc, bool_t value) {
	lc->text_conf.enabled = value;
}

void linphone_core_set_http_proxy_host(LinphoneCore *lc, const char *host) {
	lp_config_set_string(lc->config,"sip","http_proxy_host",host);
	if (lc->sal) {
		lc->sal->setHttpProxyHost(host);
		lc->sal->setHttpProxyPort(linphone_core_get_http_proxy_port(lc)); /*to make sure default value is set*/
	}
	if (lc->tunnel){
		linphone_tunnel_set_http_proxy(lc->tunnel, host, linphone_core_get_http_proxy_port(lc), NULL, NULL);
	}
}

void linphone_core_set_http_proxy_port(LinphoneCore *lc, int port) {
	lp_config_set_int(lc->config,"sip","http_proxy_port",port);
	if (lc->sal)
		lc->sal->setHttpProxyPort(port);
	if (lc->tunnel){
		linphone_tunnel_set_http_proxy(lc->tunnel, linphone_core_get_http_proxy_host(lc), port, NULL, NULL);
	}
}

const char *linphone_core_get_http_proxy_host(const LinphoneCore *lc) {
	return lp_config_get_string(lc->config,"sip","http_proxy_host",NULL);
}

int linphone_core_get_http_proxy_port(const LinphoneCore *lc) {
	return lp_config_get_int(lc->config,"sip","http_proxy_port",3128);
}

const char* linphone_transport_to_string(LinphoneTransportType transport) {
	return sal_transport_to_string((SalTransport)transport);
}

LinphoneTransportType linphone_transport_parse(const char* transport) {
	return (LinphoneTransportType)sal_transport_parse(transport);
}

const char *linphone_stream_type_to_string(const LinphoneStreamType type) {
	switch (type) {
		case LinphoneStreamTypeAudio: return "LinphoneStreamTypeAudio";
		case LinphoneStreamTypeVideo: return "LinphoneStreamTypeVideo";
		case LinphoneStreamTypeText: return "LinphoneStreamTypeText";
		case LinphoneStreamTypeUnknown: return "LinphoneStreamTypeUnknown";
	}
	return "INVALID";
}

LinphoneRingtonePlayer *linphone_core_get_ringtoneplayer(LinphoneCore *lc) {
	return lc->ringtoneplayer;
}

static int _linphone_core_delayed_conference_destruction_cb(void *user_data, unsigned int event) {
	LinphoneConference *conf = (LinphoneConference *)user_data;
	linphone_conference_unref(conf);
	return 0;
}

static void _linphone_core_conference_state_changed(LinphoneConference *conf, LinphoneConferenceState cstate, void *user_data) {
	LinphoneCore *lc = (LinphoneCore *)user_data;
	if(cstate == LinphoneConferenceStartingFailed || cstate == LinphoneConferenceStopped) {
		linphone_core_queue_task(lc, _linphone_core_delayed_conference_destruction_cb, conf, "Conference destruction task");
		lc->conf_ctx = NULL;
	}
}

/*This function sets the "unique" conference object for the LinphoneCore - which is necessary as long as
 * liblinphone is used in a client app. When liblinphone will be used in a server app, this shall not be done anymore.*/
static void linphone_core_set_conference(LinphoneCore *lc, LinphoneConference *conf){
	lc->conf_ctx = linphone_conference_ref(conf);
}

LinphoneConference *linphone_core_create_conference_with_params(LinphoneCore *lc, const LinphoneConferenceParams *params) {
	const char *conf_method_name;
	LinphoneConference *conf;
	if(lc->conf_ctx == NULL) {
		LinphoneConferenceParams *params2 = linphone_conference_params_clone(params);
		linphone_conference_params_set_state_changed_callback(params2, _linphone_core_conference_state_changed, lc);
		conf_method_name = lp_config_get_string(lc->config, "misc", "conference_type", "local");
		if(strcasecmp(conf_method_name, "local") == 0) {
			conf = linphone_local_conference_new_with_params(lc, params2);
		} else if(strcasecmp(conf_method_name, "remote") == 0) {
			conf = linphone_remote_conference_new_with_params(lc, params2);
		} else {
			ms_error("'%s' is not a valid conference method", conf_method_name);
			linphone_conference_params_unref(params2);
			return NULL;
		}
		linphone_conference_params_unref(params2);
		linphone_core_set_conference(lc, conf);
	} else {
		ms_error("Could not create a conference: a conference instance already exists");
		return NULL;
	}
	return lc->conf_ctx;
}

LinphoneConferenceParams * linphone_core_create_conference_params(LinphoneCore *lc){
	return linphone_conference_params_new(lc);
}

LinphoneStatus linphone_core_add_to_conference(LinphoneCore *lc, LinphoneCall *call) {
	LinphoneConference *conference = linphone_core_get_conference(lc);
	if(conference == NULL) {
		LinphoneConferenceParams *params = linphone_conference_params_new(lc);
		conference = linphone_core_create_conference_with_params(lc, params);
		linphone_conference_params_unref(params);
		linphone_conference_unref(conference); /*actually linphone_core_create_conference_with_params() takes a ref for lc->conf_ctx */
	}
	if(conference) return linphone_conference_add_participant(lc->conf_ctx, call);
	else return -1;
}

LinphoneStatus linphone_core_add_all_to_conference(LinphoneCore *lc) {
	for (const auto &call : L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getCalls()) {
		if (!linphone_call_get_conference(L_GET_C_BACK_PTR(call))) // Prevent the call to the conference server from being added to the conference
			linphone_core_add_to_conference(lc, L_GET_C_BACK_PTR(call));
	}
	if(lc->conf_ctx && linphone_conference_check_class(lc->conf_ctx, LinphoneConferenceClassLocal)) {
		linphone_core_enter_conference(lc);
	}
	return 0;
}

LinphoneStatus linphone_core_remove_from_conference(LinphoneCore *lc, LinphoneCall *call) {
	if(lc->conf_ctx) return linphone_conference_remove_participant_with_call(lc->conf_ctx, call);
	else return -1;
}

int linphone_core_terminate_conference(LinphoneCore *lc) {
	if(lc->conf_ctx == NULL) {
		ms_error("Could not terminate conference: no conference context");
		return -1;
	}
	linphone_conference_terminate(lc->conf_ctx);
	lc->conf_ctx = NULL;
	return 0;
}

LinphoneStatus linphone_core_enter_conference(LinphoneCore *lc) {
	if(lc->conf_ctx) return linphone_conference_enter(lc->conf_ctx);
	else return -1;
}

LinphoneStatus linphone_core_leave_conference(LinphoneCore *lc) {
	if(lc->conf_ctx) return linphone_conference_leave(lc->conf_ctx);
	else return -1;
}

bool_t linphone_core_is_in_conference(const LinphoneCore *lc) {
	if(lc->conf_ctx) return linphone_conference_is_in(lc->conf_ctx);
	else return FALSE;
}

int linphone_core_get_conference_size(LinphoneCore *lc) {
	if(lc->conf_ctx) return linphone_conference_get_size(lc->conf_ctx);
	return 0;
}

float linphone_core_get_conference_local_input_volume(LinphoneCore *lc) {
	if(lc->conf_ctx) return linphone_conference_get_input_volume(lc->conf_ctx);
	else return -1.0;
}

LinphoneStatus linphone_core_start_conference_recording(LinphoneCore *lc, const char *path) {
	if(lc->conf_ctx) return linphone_conference_start_recording(lc->conf_ctx, path);
	return -1;
}

LinphoneStatus linphone_core_stop_conference_recording(LinphoneCore *lc) {
	if(lc->conf_ctx) return linphone_conference_stop_recording(lc->conf_ctx);
	return -1;
}

LinphoneConference *linphone_core_get_conference(LinphoneCore *lc) {
	return lc->conf_ctx;
}

void linphone_core_enable_conference_server (LinphoneCore *lc, bool_t enable) {
	lp_config_set_int(linphone_core_get_config(lc), "misc", "conference_server_enabled", enable);
}

bool_t _linphone_core_is_conference_creation (const LinphoneCore *lc, const LinphoneAddress *addr) {
	LinphoneProxyConfig *proxy = linphone_core_get_default_proxy_config(lc);
	if (!proxy)
		return FALSE;
	const char *uri = linphone_proxy_config_get_conference_factory_uri(proxy);
	if (!uri)
		return FALSE;

	LinphoneAddress *factoryAddr = linphone_address_new(uri);
	if (!factoryAddr)
		return FALSE;
	// Do not compare ports
	linphone_address_set_port(factoryAddr, 0);
	LinphoneAddress *testedAddr = linphone_address_clone(addr);
	linphone_address_set_port(testedAddr, 0);
	bool_t result = linphone_address_weak_equal(factoryAddr, testedAddr);
	linphone_address_unref(factoryAddr);
	linphone_address_unref(testedAddr);
	return result;
}

bool_t linphone_core_conference_server_enabled (const LinphoneCore *lc) {
	return lp_config_get_int(linphone_core_get_config(lc), "misc", "conference_server_enabled", FALSE) ? TRUE : FALSE;
}

void linphone_core_set_tls_cert(LinphoneCore *lc, const char *tls_cert) {
	if (lc->tls_cert) {
		ms_free(lc->tls_cert);
		lc->tls_cert = NULL;
	}
	if (tls_cert && strlen(tls_cert) > 0) lc->tls_cert = ms_strdup(tls_cert);
}

void linphone_core_set_tls_key(LinphoneCore *lc, const char *tls_key) {
	if (lc->tls_key) {
		ms_free(lc->tls_key);
		lc->tls_key = NULL;
	}
	if (tls_key && strlen(tls_key) > 0) lc->tls_key = ms_strdup(tls_key);
}

void linphone_core_set_tls_cert_path(LinphoneCore *lc, const char *tls_cert_path) {
	lp_config_set_string(lc->config, "sip", "client_cert_chain", tls_cert_path);
}

void linphone_core_set_tls_key_path(LinphoneCore *lc, const char *tls_key_path) {
	lp_config_set_string(lc->config, "sip", "client_cert_key", tls_key_path);
}

const char *linphone_core_get_tls_cert(const LinphoneCore *lc) {
	return lc->tls_cert;
}

const char *linphone_core_get_tls_key(const LinphoneCore *lc) {
	return lc->tls_key;
}

const char *linphone_core_get_tls_cert_path(const LinphoneCore *lc) {
	const char *tls_cert_path = lp_config_get_string(lc->config, "sip", "client_cert_chain", NULL);
	return tls_cert_path;
}

const char *linphone_core_get_tls_key_path(const LinphoneCore *lc) {
	const char *tls_key_path = lp_config_get_string(lc->config, "sip", "client_cert_key", NULL);
	return tls_key_path;
}

LinphoneImEncryptionEngine *linphone_core_get_im_encryption_engine(const LinphoneCore *lc) {
	return lc->im_encryption_engine;
}

void linphone_core_initialize_supported_content_types(LinphoneCore *lc) {
	lc->sal->addContentTypeSupport("text/plain");
	lc->sal->addContentTypeSupport("message/external-body");
	lc->sal->addContentTypeSupport("application/vnd.gsma.rcs-ft-http+xml");
	lc->sal->addContentTypeSupport("application/im-iscomposing+xml");
	lc->sal->addContentTypeSupport("message/imdn+xml");
}

bool_t linphone_core_is_content_type_supported(const LinphoneCore *lc, const char *content_type) {
	return lc->sal->isContentTypeSupported(content_type);
}

void linphone_core_add_content_type_support(LinphoneCore *lc, const char *content_type) {
	lc->sal->addContentTypeSupport(content_type);
}

void linphone_core_remove_content_type_support(LinphoneCore *lc, const char *content_type) {
	lc->sal->removeContentTypeSupport(content_type);
}

int linphone_core_get_unread_chat_message_count (const LinphoneCore *lc) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getUnreadChatMessageCount();
}

int linphone_core_get_unread_chat_message_count_from_local (const LinphoneCore *lc, const LinphoneAddress *address) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getUnreadChatMessageCount(
		LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(address))
	);
}

int linphone_core_get_unread_chat_message_count_from_active_locals (const LinphoneCore *lc) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getUnreadChatMessageCountFromActiveLocals();
}

bool_t linphone_core_has_crappy_opengl(LinphoneCore *lc) {
	MSFactory * factory = linphone_core_get_ms_factory(lc);
	MSDevicesInfo *devices = ms_factory_get_devices_info(factory);
	SoundDeviceDescription *sound_description = ms_devices_info_get_sound_device_description(devices);
	if (sound_description == NULL) return FALSE;
	if (sound_description->flags & DEVICE_HAS_CRAPPY_OPENGL) return TRUE;
	return FALSE;
}
