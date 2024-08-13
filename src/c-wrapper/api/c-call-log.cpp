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

#include "linphone/api/c-call-log.h"
#include "c-wrapper/c-wrapper.h"
#include "call/call-log.h"
#include "core/core-p.h"
#include "db/main-db.h"
#include "linphone/utils/utils.h"
#include "linphone/wrapper_utils.h"
#include "private.h"

// =============================================================================

using namespace LinphonePrivate;

LinphoneCallLog *
linphone_call_log_new(LinphoneCore *core, LinphoneCallDir dir, const LinphoneAddress *from, const LinphoneAddress *to) {
	const auto cppFrom = Address::toCpp(from)->getSharedFromThis();
	const auto cppTo = Address::toCpp(to)->getSharedFromThis();
	return CallLog::createCObject(L_GET_CPP_PTR_FROM_C_OBJECT(core)->getSharedFromThis(), dir, cppFrom, cppTo);
}

LinphoneCallLog *linphone_call_log_ref(LinphoneCallLog *call_log) {
	CallLog::toCpp(call_log)->ref();
	return call_log;
}

void linphone_call_log_unref(LinphoneCallLog *call_log) {
	CallLog::toCpp(call_log)->unref();
}

const char *linphone_call_log_get_call_id(const LinphoneCallLog *call_log) {
	return L_STRING_TO_C(CallLog::toCpp(call_log)->getCallId());
}

void linphone_call_log_set_call_id(LinphoneCallLog *call_log, const char *call_id) {
	CallLog::toCpp(call_log)->setCallId(L_C_TO_STRING(call_id));
}

LinphoneCallDir linphone_call_log_get_dir(const LinphoneCallLog *call_log) {
	return CallLog::toCpp(call_log)->getDirection();
}

int linphone_call_log_get_duration(const LinphoneCallLog *call_log) {
	return CallLog::toCpp(call_log)->getDuration();
}

const LinphoneAddress *linphone_call_log_get_from_address(const LinphoneCallLog *call_log) {
	return CallLog::toCpp(call_log)->getFromAddress()->toC();
}

float linphone_call_log_get_quality(const LinphoneCallLog *call_log) {
	return CallLog::toCpp(call_log)->getQuality();
}

const char *linphone_call_log_get_ref_key(const LinphoneCallLog *call_log) {
	return L_STRING_TO_C(CallLog::toCpp(call_log)->getRefKey());
}

const LinphoneAddress *linphone_call_log_get_local_address(const LinphoneCallLog *call_log) {
	return CallLog::toCpp(call_log)->getLocalAddress()->toC();
}

const LinphoneAddress *linphone_call_log_get_remote_address(const LinphoneCallLog *call_log) {
	return CallLog::toCpp(call_log)->getRemoteAddress()->toC();
}

void linphone_call_log_set_remote_address(LinphoneCallLog *call_log, LinphoneAddress *address) {
	const auto cppAddress = Address::toCpp(address)->getSharedFromThis();
	CallLog::toCpp(call_log)->setRemoteAddress(cppAddress);
}

time_t linphone_call_log_get_start_date(const LinphoneCallLog *call_log) {
	return CallLog::toCpp(call_log)->getStartTime();
}

LinphoneCallStatus linphone_call_log_get_status(const LinphoneCallLog *call_log) {
	return CallLog::toCpp(call_log)->getStatus();
}

const LinphoneAddress *linphone_call_log_get_to_address(const LinphoneCallLog *call_log) {
	return CallLog::toCpp(call_log)->getToAddress()->toC();
}

void linphone_call_log_set_ref_key(LinphoneCallLog *call_log, const char *refkey) {
	CallLog::toCpp(call_log)->setRefKey(L_C_TO_STRING(refkey));
}

bool_t linphone_call_log_video_enabled(const LinphoneCallLog *call_log) {
	return CallLog::toCpp(call_log)->isVideoEnabled();
}

char *linphone_call_log_to_str(const LinphoneCallLog *call_log) {
	std::string s = CallLog::toCpp(call_log)->toString();
	return s.empty() ? NULL : bctbx_strdup(s.c_str());
}

bool_t linphone_call_log_was_conference(LinphoneCallLog *call_log) {
	return CallLog::toCpp(call_log)->wasConference();
}

const LinphoneErrorInfo *linphone_call_log_get_error_info(const LinphoneCallLog *call_log) {
	return CallLog::toCpp(call_log)->getErrorInfo();
}

void *linphone_call_log_get_user_data(const LinphoneCallLog *call_log) {
	return CallLog::toCpp(call_log)->getUserData();
}

void linphone_call_log_set_user_data(LinphoneCallLog *call_log, void *user_data) {
	CallLog::toCpp(call_log)->setUserData(user_data);
}

LinphoneQualityReporting *linphone_call_log_get_quality_reporting(LinphoneCallLog *call_log) {
	return CallLog::toCpp(call_log)->getQualityReporting();
}

LinphoneConferenceInfo *linphone_call_log_get_conference_info(LinphoneCallLog *call_log) {
	auto confInfo = CallLog::toCpp(call_log)->getConferenceInfo();
	if (confInfo) return confInfo->toC();
	return nullptr;
}

LinphoneChatRoom *linphone_call_log_get_chat_room(LinphoneCallLog *call_log) {
	auto &chatRoom = CallLog::toCpp(call_log)->getChatRoom();
	if (chatRoom) return chatRoom->toC();
	return nullptr;
}

LinphoneCallLog *linphone_core_create_call_log(LinphoneCore *core,
                                               LinphoneAddress *from,
                                               LinphoneAddress *to,
                                               LinphoneCallDir dir,
                                               int duration,
                                               time_t start_time,
                                               time_t connected_time,
                                               LinphoneCallStatus status,
                                               bool_t video_enabled,
                                               float quality) {

	const auto cppFrom = Address::toCpp(from)->getSharedFromThis();
	const auto cppTo = Address::toCpp(to)->getSharedFromThis();
	auto log = CallLog::create(L_GET_CPP_PTR_FROM_C_OBJECT(core), dir, cppFrom, cppTo);

	log->setDuration(duration);
	log->setStartTime(start_time);
	log->setConnectedTime(connected_time);
	log->setStatus(status);
	log->setVideoEnabled(video_enabled);
	log->setQuality(quality);

	linphone_core_store_call_log(core, log->toC());

	return linphone_call_log_ref(log->toC());
}

// =============================================================================

void call_logs_write_to_config_file(LinphoneCore *lc) {
	bctbx_list_t *elem;
	char logsection[32];
	int i;
	char *tmp;
	LpConfig *cfg = lc->config;

	if (linphone_core_get_global_state(lc) == LinphoneGlobalStartup) return;

	if (lc->max_call_logs == LINPHONE_MAX_CALL_HISTORY_UNLIMITED) return;

	for (i = 0, elem = lc->call_logs; elem != NULL; elem = elem->next, ++i) {
		LinphoneCallLog *cl = (LinphoneCallLog *)elem->data;
		auto log = CallLog::toCpp(cl);

		snprintf(logsection, sizeof(logsection), "call_log_%i", i);
		linphone_config_clean_section(cfg, logsection);
		linphone_config_set_int(cfg, logsection, "dir", log->getDirection());
		linphone_config_set_int(cfg, logsection, "status", log->getStatus());

		tmp = ms_strdup(L_STRING_TO_C(log->getFromAddress()->toString()));
		linphone_config_set_string(cfg, logsection, "from", tmp);
		ms_free(tmp);
		tmp = ms_strdup(L_STRING_TO_C(log->getToAddress()->toString()));
		linphone_config_set_string(cfg, logsection, "to", tmp);
		ms_free(tmp);
		if (log->getStartTime())
			linphone_config_set_int64(cfg, logsection, "start_date_time", (int64_t)log->getStartTime());
		else linphone_config_set_string(cfg, logsection, "start_date", log->getStartTimeString().c_str());
		linphone_config_set_int(cfg, logsection, "duration", log->getDuration());
		if (!log->getRefKey().empty()) linphone_config_set_string(cfg, logsection, "refkey", log->getRefKey().c_str());
		linphone_config_set_float(cfg, logsection, "quality", log->getQuality());
		linphone_config_set_int(cfg, logsection, "video_enabled", log->isVideoEnabled() ? 1 : 0);
		linphone_config_set_string(cfg, logsection, "call_id", log->getCallId().c_str());
	}

	for (; i < lc->max_call_logs; ++i) {
		snprintf(logsection, sizeof(logsection), "call_log_%i", i);
		linphone_config_clean_section(cfg, logsection);
	}
}

bctbx_list_t *linphone_core_read_call_logs_from_config_file(LinphoneCore *lc) {
	char logsection[32];
	int i;
	const char *tmp;
	uint64_t sec;
	LpConfig *cfg = lc->config;
	bctbx_list_t *call_logs = NULL;

	for (i = 0;; ++i) {
		snprintf(logsection, sizeof(logsection), "call_log_%i", i);
		if (linphone_config_has_section(cfg, logsection)) {
			tmp = linphone_config_get_string(cfg, logsection, "from", NULL);
			const auto from = (tmp) ? Address::create(tmp) : nullptr;
			tmp = linphone_config_get_string(cfg, logsection, "to", NULL);
			const auto to = (tmp) ? Address::create(tmp) : nullptr;
			if (!from || !from->isValid() || !to || !to->isValid()) {
				continue;
			}
			auto cl = CallLog::create(L_GET_CPP_PTR_FROM_C_OBJECT(lc),
			                          static_cast<LinphoneCallDir>(linphone_config_get_int(cfg, logsection, "dir", 0)),
			                          from, to);
			cl->setStatus(static_cast<LinphoneCallStatus>(linphone_config_get_int(cfg, logsection, "status", 0)));
			sec = (uint64_t)linphone_config_get_int64(cfg, logsection, "start_date_time", 0);
			if (sec) {
				/*new call log format with date expressed in seconds */
				cl->setStartTime((time_t)sec);
			} else {
				tmp = linphone_config_get_string(cfg, logsection, "start_date", NULL);
				if (tmp) {
					cl->setStartTime(Utils::getStringToTime("%c", tmp));
				}
			}
			cl->setDuration(linphone_config_get_int(cfg, logsection, "duration", 0));
			tmp = linphone_config_get_string(cfg, logsection, "refkey", NULL);
			if (tmp) cl->setRefKey(tmp);
			cl->setQuality(linphone_config_get_float(cfg, logsection, "quality", -1));
			cl->setVideoEnabled(!!linphone_config_get_int(cfg, logsection, "video_enabled", 0));
			tmp = linphone_config_get_string(cfg, logsection, "call_id", NULL);
			if (tmp) cl->setCallId(tmp);

			call_logs = bctbx_list_append(call_logs, linphone_call_log_ref(cl->toC()));
		} else break;
	}
	return call_logs;
}

// =============================================================================

void linphone_core_store_call_log(LinphoneCore *lc, LinphoneCallLog *log) {
	if (!lc) return;

#ifdef HAVE_DB_STORAGE
	std::unique_ptr<MainDb> &mainDb = L_GET_PRIVATE_FROM_C_OBJECT(lc)->mainDb;
	if (mainDb) mainDb->insertCallLog(CallLog::toCpp(log)->getSharedFromThis());
#endif

	lc->call_logs = bctbx_list_prepend(lc->call_logs, linphone_call_log_ref(log));
}

const bctbx_list_t *linphone_core_get_call_history(LinphoneCore *lc) {
	if (!lc) return NULL;

	if (linphone_core_get_global_state(lc) != LinphoneGlobalOn) {
		bctbx_warning("Cannot get call history as the core is not running");
		return NULL;
	}

#ifdef HAVE_DB_STORAGE
	std::unique_ptr<MainDb> &mainDb = L_GET_PRIVATE_FROM_C_OBJECT(lc)->mainDb;
	if (!mainDb) return lc->call_logs;

	if (lc->call_logs != NULL) {
		size_t callLogsDatabaseSize = (size_t)mainDb->getCallHistorySize();
		if (bctbx_list_size(lc->call_logs) >= callLogsDatabaseSize) return lc->call_logs;
		// If some call logs were added to the Core before the full history was loaded from database,
		// clean memory cache and reload everything from database
		bctbx_list_free_with_data(lc->call_logs, (bctbx_list_free_func)linphone_call_log_unref);
		lc->call_logs = NULL;
	}

	auto list = mainDb->getCallHistory(lc->max_call_logs);
	if (!list.empty()) {
		for (auto &log : list) {
			lc->call_logs = bctbx_list_append(lc->call_logs, linphone_call_log_ref(log->toC()));
		}
	}
#endif

	return lc->call_logs;
}

void linphone_core_delete_call_history(LinphoneCore *lc) {
	if (!lc) return;

#ifdef HAVE_DB_STORAGE
	std::unique_ptr<MainDb> &mainDb = L_GET_PRIVATE_FROM_C_OBJECT(lc)->mainDb;
	if (!mainDb) return;

	mainDb->deleteCallHistory();
#endif

	if (lc->call_logs) {
		bctbx_list_free_with_data(lc->call_logs, (bctbx_list_free_func)linphone_call_log_unref);
		lc->call_logs = NULL;
	}
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void linphone_core_delete_call_log(LinphoneCore *lc, LinphoneCallLog *log) {
	if (!lc) return;

#ifdef HAVE_DB_STORAGE
	std::unique_ptr<MainDb> &mainDb = L_GET_PRIVATE_FROM_C_OBJECT(lc)->mainDb;
	if (!mainDb) return;

	mainDb->deleteCallLog(CallLog::toCpp(log)->getSharedFromThis());
#endif

	if (lc->call_logs) {
		bctbx_list_free_with_data(lc->call_logs, (bctbx_list_free_func)linphone_call_log_unref);
		lc->call_logs = NULL;
	}
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

int linphone_core_get_call_history_size(LinphoneCore *lc) {
	if (!lc) return 0;

#ifdef HAVE_DB_STORAGE
	std::unique_ptr<MainDb> &mainDb = L_GET_PRIVATE_FROM_C_OBJECT(lc)->mainDb;
	if (!mainDb) return 0;

	return mainDb->getCallHistorySize();
#else
	return (int)bctbx_list_size(lc->call_logs);
#endif
}

bctbx_list_t *linphone_core_get_call_history_2(LinphoneCore *lc,
                                               const LinphoneAddress *peer_addr,
                                               const LinphoneAddress *local_addr) {
	if (!lc || !peer_addr || !local_addr) return NULL;

#ifdef HAVE_DB_STORAGE
	std::unique_ptr<MainDb> &mainDb = L_GET_PRIVATE_FROM_C_OBJECT(lc)->mainDb;
	if (!mainDb) return NULL;

	const auto peerAddr = Address::toCpp(peer_addr)->getSharedFromThis();
	const auto localAddr = Address::toCpp(local_addr)->getSharedFromThis();
	auto list = mainDb->getCallHistory(peerAddr, localAddr);

	bctbx_list_t *results = NULL;
	if (!list.empty()) {
		for (auto &log : list) {
			results = bctbx_list_append(results, linphone_call_log_ref(log->toC()));
		}
	}

	return results;
#else
	bctbx_fatal("This function requires ENABLE_DB_STORAGE in order to work!");
	return NULL;
#endif
}

LinphoneCallLog *linphone_core_get_last_outgoing_call_log(LinphoneCore *lc) {
	if (!lc) return NULL;

#ifdef HAVE_DB_STORAGE
	std::unique_ptr<MainDb> &mainDb = L_GET_PRIVATE_FROM_C_OBJECT(lc)->mainDb;
	if (!mainDb) return NULL;

	auto log = mainDb->getLastOutgoingCall();

	return log != nullptr ? linphone_call_log_ref(log->toC()) : NULL;
#else
	bctbx_fatal("This function requires ENABLE_DB_STORAGE in order to work!");
	return NULL;
#endif
}

LinphoneCallLog *linphone_core_find_call_log(LinphoneCore *lc, const char *call_id, int limit) {
	if (!lc) return NULL;

#ifdef HAVE_DB_STORAGE
	std::unique_ptr<MainDb> &mainDb = L_GET_PRIVATE_FROM_C_OBJECT(lc)->mainDb;
	if (!mainDb) return NULL;

	auto log = mainDb->getCallLog(L_C_TO_STRING(call_id), limit);

	return log != nullptr ? linphone_call_log_ref(log->toC()) : NULL;
#else
	long i;
	bctbx_list_t *item;

	for (item = lc->call_logs, i = 0; item != NULL && (limit < 1 || i < limit); item = bctbx_list_next(item), i++) {
		LinphoneCallLog *call_log = reinterpret_cast<LinphoneCallLog *>(bctbx_list_get_data(item));
		if ((strcmp(linphone_call_log_get_call_id(call_log), call_id) == 0)) return linphone_call_log_ref(call_log);
	}

	return NULL;
#endif
}

LinphoneCallLog *linphone_core_find_call_log_from_call_id(LinphoneCore *lc, const char *call_id) {
	return linphone_core_find_call_log(lc, call_id, -1);
}
