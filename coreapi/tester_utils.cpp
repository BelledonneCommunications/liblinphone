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

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#endif
#include "tester_utils.h"
#include "private.h"

#include "call/call.h"
#include "chat/chat-room/chat-room-p.h"
#include "chat/encryption/encryption-engine.h"
#include "chat/chat-room/client-group-chat-room-p.h"
#include "core/core-p.h"
#include "c-wrapper/c-wrapper.h"
#include "conference/session/media-session-p.h"
#include "event-log/conference/conference-chat-message-event.h"
#include "mediastreamer2/msanalysedisplay.h"

using namespace std;

using namespace LinphonePrivate;

LinphoneVcardContext *linphone_core_get_vcard_context(const LinphoneCore *lc) {
	return lc->vcard_context;
}

void linphone_core_set_zrtp_not_available_simulation(LinphoneCore *lc, bool_t enabled) {
	lc->zrtp_not_available_simulation = enabled;
}

void linphone_core_lime_x3dh_set_test_decryption_failure_flag(const LinphoneCore *lc, bool_t flag) {
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getEncryptionEngine()->setTestForceDecryptionFailureFlag((flag==TRUE));
}

belle_http_provider_t *linphone_core_get_http_provider(const LinphoneCore *lc) {
	return lc->http_provider;
}

void linphone_core_enable_send_call_stats_periodical_updates(LinphoneCore *lc, bool_t enabled) {
	lc->send_call_stats_periodical_updates = enabled;
}

void linphone_core_set_zrtp_cache_db(LinphoneCore *lc, sqlite3 *cache_db) {
	lc->zrtp_cache_db = cache_db;
}

LinphoneCoreCbs *linphone_core_get_first_callbacks(const LinphoneCore *lc) {
	return ((VTableReference *)lc->vtable_refs->data)->cbs;
}

bctbx_list_t **linphone_core_get_call_logs_attribute(LinphoneCore *lc) {
	return &lc->call_logs;
}

void linphone_core_cbs_set_auth_info_requested(LinphoneCoreCbs *cbs, LinphoneCoreAuthInfoRequestedCb cb) {
	cbs->vtable->auth_info_requested = cb;
}

reporting_session_report_t **linphone_quality_reporting_get_reports(LinphoneQualityReporting *qreporting) {
	return &qreporting->reports[0];
}

const bctbx_list_t *linphone_friend_get_insubs(const LinphoneFriend *fr) {
	return fr->insubs;
}

int linphone_friend_list_get_expected_notification_version(const LinphoneFriendList *list) {
	return list->expected_notification_version;
}

unsigned int linphone_friend_list_get_storage_id(const LinphoneFriendList *list) {
	return list->storage_id;
}

unsigned int linphone_friend_get_storage_id(const LinphoneFriend *lf) {
	return lf->storage_id;
}

void linphone_friend_set_core(LinphoneFriend *lf, LinphoneCore *lc) {
	lf->lc = lc;
}

LinphoneFriendList *linphone_friend_get_friend_list(const LinphoneFriend *lf) {
	return lf->friend_list;
}

bctbx_list_t **linphone_friend_list_get_friends_attribute(LinphoneFriendList *lfl) {
	return &lfl->friends;
}

const bctbx_list_t *linphone_friend_list_get_dirty_friends_to_update(const LinphoneFriendList *lfl) {
	return lfl->dirty_friends_to_update;
}

int linphone_friend_list_get_revision(const LinphoneFriendList *lfl) {
	return lfl->revision;
}

unsigned int _linphone_call_get_nb_audio_starts (const LinphoneCall *call) {
	const LinphoneStreamInternalStats * st = _linphone_call_get_stream_internal_stats(call, LinphoneStreamTypeAudio);
	return st ? st->number_of_starts : 0;
}

unsigned int _linphone_call_get_nb_audio_stops (const LinphoneCall *call) {
	const LinphoneStreamInternalStats * st = _linphone_call_get_stream_internal_stats(call, LinphoneStreamTypeAudio);
	return st ? st->number_of_stops : 0;
}

unsigned int _linphone_call_get_nb_video_starts (const LinphoneCall *call) {
	const LinphoneStreamInternalStats * st = _linphone_call_get_stream_internal_stats(call, LinphoneStreamTypeVideo);
	return st ? st->number_of_starts : 0;
}

unsigned int _linphone_call_get_nb_text_starts (const LinphoneCall *call) {
	const LinphoneStreamInternalStats * st = _linphone_call_get_stream_internal_stats(call, LinphoneStreamTypeText);
	return st ? st->number_of_starts : 0;
}

const LinphoneStreamInternalStats *_linphone_call_get_stream_internal_stats(const LinphoneCall *call, LinphoneStreamType stream_type){
	return Call::toCpp(call)->getStreamInternalStats(stream_type);
}

belle_sip_source_t *_linphone_call_get_dtmf_timer (const LinphoneCall *call) {
	return L_GET_PRIVATE(static_pointer_cast<LinphonePrivate::MediaSession>(
		Call::toCpp(call)->getActiveSession()))->getDtmfTimer();
}

bool_t _linphone_call_has_dtmf_sequence (const LinphoneCall *call) {
	return L_GET_PRIVATE(static_pointer_cast<LinphonePrivate::MediaSession>(
		Call::toCpp(call)->getActiveSession()))->getDtmfSequence().empty() ? FALSE : TRUE;
}

SalMediaDescription *_linphone_call_get_local_desc (const LinphoneCall *call) {
	return L_GET_PRIVATE(static_pointer_cast<LinphonePrivate::MediaSession>(
		Call::toCpp(call)->getActiveSession()))->getLocalDesc().get();
}

SalMediaDescription *_linphone_call_get_remote_desc (const LinphoneCall *call) {
	return L_GET_PRIVATE(static_pointer_cast<LinphonePrivate::MediaSession>(
		Call::toCpp(call)->getActiveSession()))->getRemoteDesc().get();
}

SalMediaDescription *_linphone_call_get_result_desc (const LinphoneCall *call) {
	return L_GET_PRIVATE(static_pointer_cast<LinphonePrivate::MediaSession>(
		Call::toCpp(call)->getActiveSession()))->getResultDesc().get();
}

MSWebCam *_linphone_call_get_video_device (const LinphoneCall *call) {
	return L_GET_PRIVATE(static_pointer_cast<LinphonePrivate::MediaSession>(
		Call::toCpp(call)->getActiveSession()))->getVideoDevice();
}

void _linphone_call_add_local_desc_changed_flag (LinphoneCall *call, int flag) {
	L_GET_PRIVATE(static_pointer_cast<LinphonePrivate::MediaSession>(
		Call::toCpp(call)->getActiveSession()))->addLocalDescChangedFlag(flag);
}

int _linphone_call_get_main_audio_stream_index (const LinphoneCall *call) {
	return Call::toCpp(call)->getMediaStreamIndex(LinphoneStreamTypeAudio);
}

int _linphone_call_get_main_text_stream_index (const LinphoneCall *call) {
	return Call::toCpp(call)->getMediaStreamIndex(LinphoneStreamTypeText);
}

int _linphone_call_get_main_video_stream_index (const LinphoneCall *call) {
	return Call::toCpp(call)->getMediaStreamIndex(LinphoneStreamTypeVideo);
}

void _linphone_chat_room_enable_migration(LinphoneChatRoom *cr, bool_t enable) {
	shared_ptr<AbstractChatRoom> acr = L_GET_CPP_PTR_FROM_C_OBJECT(cr);
	L_GET_PRIVATE(acr->getCore())->mainDb->enableChatRoomMigration(acr->getConferenceId(), !!enable);
}

int _linphone_chat_room_get_transient_message_count (const LinphoneChatRoom *cr) {
	shared_ptr<const ChatRoom> chatRoom = static_pointer_cast<const ChatRoom>(L_GET_CPP_PTR_FROM_C_OBJECT(cr));
	return (int)L_GET_PRIVATE(chatRoom)->transientEvents.size();
}

LinphoneChatMessage * _linphone_chat_room_get_first_transient_message (const LinphoneChatRoom *cr) {
	shared_ptr<const ChatRoom> chatRoom = static_pointer_cast<const ChatRoom>(L_GET_CPP_PTR_FROM_C_OBJECT(cr));
	if (L_GET_PRIVATE(chatRoom)->transientEvents.empty())
		return nullptr;
	shared_ptr<ConferenceChatMessageEvent> event = static_pointer_cast<ConferenceChatMessageEvent>(
		L_GET_PRIVATE(chatRoom)->transientEvents.front()
	);
	return L_GET_C_BACK_PTR(event->getChatMessage());
}

char * linphone_core_get_device_identity(LinphoneCore *lc) {
	char *identity = NULL;
	LinphoneProxyConfig *proxy = linphone_core_get_default_proxy_config(lc);
	if (proxy) {
		const LinphoneAddress *contactAddr = linphone_proxy_config_get_contact(proxy);
		if (contactAddr)
			identity = linphone_address_as_string(contactAddr);
		else
			identity = bctbx_strdup(linphone_proxy_config_get_identity(proxy));
	} else {
		identity = bctbx_strdup(linphone_core_get_primary_contact(lc));
	}
	return identity;
}

const LinphoneCoreToneManagerStats *linphone_core_get_tone_manager_stats(LinphoneCore *lc) {
	return L_GET_PRIVATE_FROM_C_OBJECT(lc)->getToneManager().getStats();
}

void linphone_core_reset_tone_manager_stats(LinphoneCore *lc) {
	L_GET_PRIVATE_FROM_C_OBJECT(lc)->getToneManager().resetStats();
}

const char *linphone_core_get_tone_file(LinphoneCore *lc, LinphoneToneID id){
	LinphoneToneDescription *tone = L_GET_PRIVATE_FROM_C_OBJECT(lc)->getToneManager().getToneFromId(id);
	return tone ? tone->audiofile : NULL;
}

void linphone_core_reset_shared_core_state(LinphoneCore *lc) {
	static_cast<PlatformHelpers *>(lc->platform_helper)->getSharedCoreHelpers()->resetSharedCoreState();
}

char * linphone_core_get_download_path(LinphoneCore *lc) {
	return bctbx_strdup(L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getDownloadPath().c_str());
}

size_t linphone_chat_room_get_previouses_conference_ids_count(LinphoneChatRoom *cr) {
	auto abstract = L_GET_CPP_PTR_FROM_C_OBJECT(cr);

	return L_GET_PRIVATE(static_pointer_cast<ClientGroupChatRoom>(abstract))->getPreviousConferenceIds().size();
}

bool_t linphone_call_check_rtp_sessions(LinphoneCall *call) {
	std::shared_ptr<LinphonePrivate::MediaSession> ms = Call::toCpp(call)->getMediaSession();
	if (ms){
		StreamsGroup & sg = L_GET_PRIVATE(ms)->getStreamsGroup();
		for (auto &stream : sg.getStreams()){
			if (!stream) continue;
			MS2Stream *s  =  dynamic_cast<MS2Stream *>(stream.get());
			if(stream->getType() == SalVideo) {
				MediaStream *ms = s->getMediaStream();
				RtpSession *rtp_session = ms->sessions.rtp_session;
				if (!rtp_session) {
					lInfo() << "checkRtpSession(): session empty";
					return false;
				}
				const rtp_stats_t *rtps = rtp_session_get_stats(rtp_session);
				switch (media_stream_get_direction(ms)) {
					case MediaStreamRecvOnly:
						// Can be 0 if it's not attached with filter
						break;
					case MediaStreamSendOnly:
						if (rtps->packet_sent < 5) {
							return false;
						}
						break;
					case MediaStreamSendRecv:
						if (rtps->packet_recv < 5 || rtps->packet_sent < 5) {
							return false;
						}
						break;
					default:
						break;
				}
			}
		}
		return true;
	}
	return false;
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
bool_t linphone_call_compare_video_color(LinphoneCall *call, MSMireControl cl, MediaStreamDir dir, const char  *label) {
#ifdef VIDEO_ENABLED
	auto lambda = [] (Stream *s, MediaStreamDir dir, const string &label, MSMireControl cl) {
		if (s->getType() == SalVideo && (label.empty() ||  label.compare(s->getLabel())==0)) {
			MS2VideoStream *vs  =  dynamic_cast<MS2VideoStream *>(s);
			if (vs && media_stream_get_direction(vs->getMediaStream()) == dir && vs->getVideoStream()->output && ms_filter_get_id(vs->getVideoStream()->output)== MS_ANALYSE_DISPLAY_ID){
				return ms_filter_call_method(vs->getVideoStream()->output, MS_ANALYSE_DISPLAY_COMPARE_COLOR, &cl) == 0;
			}
		}
		return false;
	};
	std::shared_ptr<LinphonePrivate::MediaSession> ms = Call::toCpp(call)->getMediaSession();
	if (ms){
		StreamsGroup & sg = L_GET_PRIVATE(ms)->getStreamsGroup();
		if (sg.lookupStream(lambda, dir, label, cl)) return true;
	}
#endif
	return false;
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

