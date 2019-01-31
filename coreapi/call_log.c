/*
linphone
Copyright (C) 2010-2014  Belledonne Communications SARL

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

#ifndef __APPLE__ /*XOPEN_SOURCE icompilation issue  with xcode9 https://github.com/eclipse/omr/pull/1721*/
#define _XOPEN_SOURCE 700 /*required for strptime of GNU libc*/
#endif

#include <time.h>

#if !defined(_WIN32) && !defined(__ANDROID__) && !defined(__QNXNTO__)
	#include <langinfo.h>
	#include <iconv.h>
	#include <string.h>
#endif // if !defined(_WIN32) && !defined(__ANDROID__) && !defined(__QNXNTO__)

#define MAX_PATH_SIZE 1024

#include "c-wrapper/c-wrapper.h"

// TODO: From coreapi. Remove me later.
#include "private.h"

typedef struct _CallLogStorageResult {
	LinphoneCore *core;
	bctbx_list_t *result;
} CallLogStorageResult;

/*******************************************************************************
 * Internal functions                                                          *
 ******************************************************************************/

/*prevent a gcc bug with %c*/
static size_t my_strftime(char *s, size_t max, const char  *fmt,  const struct tm *tm){
	return strftime(s, max, fmt, tm);
}

static time_t string_to_time(const char *date){
#ifndef _WIN32
	struct tm tmtime={0};
	strptime(date,"%c",&tmtime);
	return mktime(&tmtime);
#else
	return 0;
#endif
}

static void set_call_log_date(LinphoneCallLog *cl, time_t start_time){
	struct tm loctime;
#ifdef _WIN32
#if !defined(_WIN32_WCE)
	loctime=*localtime(&start_time);
	/*FIXME*/
#endif /*_WIN32_WCE*/
#else
	localtime_r(&start_time,&loctime);
#endif
	my_strftime(cl->start_date,sizeof(cl->start_date),"%c",&loctime);
}

/*******************************************************************************
 * Private functions                                                           *
 ******************************************************************************/

void call_logs_write_to_config_file(LinphoneCore *lc){
	bctbx_list_t *elem;
	char logsection[32];
	int i;
	char *tmp;
	LpConfig *cfg=lc->config;

	if (linphone_core_get_global_state (lc)==LinphoneGlobalStartup) return;

	if (lc->max_call_logs == LINPHONE_MAX_CALL_HISTORY_UNLIMITED) return;

	for(i=0,elem=lc->call_logs;elem!=NULL;elem=elem->next,++i){
		LinphoneCallLog *cl=(LinphoneCallLog*)elem->data;
		snprintf(logsection,sizeof(logsection),"call_log_%i",i);
		lp_config_clean_section(cfg,logsection);
		lp_config_set_int(cfg,logsection,"dir",cl->dir);
		lp_config_set_int(cfg,logsection,"status",cl->status);
		tmp=linphone_address_as_string(cl->from);
		lp_config_set_string(cfg,logsection,"from",tmp);
		ms_free(tmp);
		tmp=linphone_address_as_string(cl->to);
		lp_config_set_string(cfg,logsection,"to",tmp);
		ms_free(tmp);
		if (cl->start_date_time)
			lp_config_set_int64(cfg,logsection,"start_date_time",(int64_t)cl->start_date_time);
		else lp_config_set_string(cfg,logsection,"start_date",cl->start_date);
		lp_config_set_int(cfg,logsection,"duration",cl->duration);
		if (cl->refkey) lp_config_set_string(cfg,logsection,"refkey",cl->refkey);
		lp_config_set_float(cfg,logsection,"quality",cl->quality);
		lp_config_set_int(cfg,logsection,"video_enabled", cl->video_enabled);
		lp_config_set_string(cfg,logsection,"call_id",cl->call_id);
	}
	for(;i<lc->max_call_logs;++i){
		snprintf(logsection,sizeof(logsection),"call_log_%i",i);
		lp_config_clean_section(cfg,logsection);
	}
}

bctbx_list_t * linphone_core_read_call_logs_from_config_file(LinphoneCore *lc){
	char logsection[32];
	int i;
	const char *tmp;
	uint64_t sec;
	LpConfig *cfg=lc->config;
	bctbx_list_t *call_logs = NULL;

	for(i=0;;++i){
		snprintf(logsection,sizeof(logsection),"call_log_%i",i);
		if (lp_config_has_section(cfg,logsection)){
			LinphoneCallLog *cl;
			LinphoneAddress *from=NULL,*to=NULL;
			tmp=lp_config_get_string(cfg,logsection,"from",NULL);
			if (tmp) from=linphone_address_new(tmp);
			tmp=lp_config_get_string(cfg,logsection,"to",NULL);
			if (tmp) to=linphone_address_new(tmp);
			if (!from || !to)
				continue;
			cl=linphone_call_log_new(static_cast<LinphoneCallDir>(lp_config_get_int(cfg,logsection,"dir",0)),from,to);
			cl->status=static_cast<LinphoneCallStatus>(lp_config_get_int(cfg,logsection,"status",0));
			sec=(uint64_t)lp_config_get_int64(cfg,logsection,"start_date_time",0);
			if (sec) {
				/*new call log format with date expressed in seconds */
				cl->start_date_time=(time_t)sec;
				set_call_log_date(cl,cl->start_date_time);
			}else{
				tmp=lp_config_get_string(cfg,logsection,"start_date",NULL);
				if (tmp) {
					strncpy(cl->start_date,tmp,sizeof(cl->start_date));
					cl->start_date[sizeof(cl->start_date) - 1] = '\0';
					cl->start_date_time=string_to_time(cl->start_date);
				}
			}
			cl->duration=lp_config_get_int(cfg,logsection,"duration",0);
			tmp=lp_config_get_string(cfg,logsection,"refkey",NULL);
			if (tmp) cl->refkey=ms_strdup(tmp);
			cl->quality=lp_config_get_float(cfg,logsection,"quality",-1);
			cl->video_enabled=!!lp_config_get_int(cfg,logsection,"video_enabled",0);
			tmp=lp_config_get_string(cfg,logsection,"call_id",NULL);
			if (tmp) cl->call_id=ms_strdup(tmp);
			call_logs=bctbx_list_append(call_logs,cl);
		}else break;
	}
	return call_logs;
}


/*******************************************************************************
 * Public functions                                                            *
 ******************************************************************************/

LinphoneCallLog *linphone_core_create_call_log(LinphoneCore *lc, LinphoneAddress *from, LinphoneAddress *to, LinphoneCallDir dir,
		int duration, time_t start_time, time_t connected_time, LinphoneCallStatus status, bool_t video_enabled, float quality) {
	LinphoneCallLog *log = linphone_call_log_new(dir, linphone_address_ref(from), linphone_address_ref(to));

	log->duration = duration;
	log->start_date_time = start_time;
	set_call_log_date(log,log->start_date_time);
	log->connected_date_time = connected_time;
	log->status = status;
	log->video_enabled = video_enabled;
	log->quality = quality;

	linphone_core_store_call_log(lc, log);

	return log;
}

const char *linphone_call_log_get_call_id(const LinphoneCallLog *cl){
	return cl->call_id;
}

void linphone_call_log_set_call_id(LinphoneCallLog *cl, const char *call_id) {
	if (cl->call_id) bctbx_free(cl->call_id);
	cl->call_id = call_id ? bctbx_strdup(call_id) : NULL;
}

LinphoneCallDir linphone_call_log_get_dir(const LinphoneCallLog *cl){
	return cl->dir;
}

int linphone_call_log_get_duration(const LinphoneCallLog *cl){
	return cl->duration;
}

const LinphoneAddress *linphone_call_log_get_from_address(const LinphoneCallLog *cl){
	return cl->from;
}

const rtp_stats_t *linphone_call_log_get_local_stats(const LinphoneCallLog *cl){
	return &cl->local_stats;
}

float linphone_call_log_get_quality(const LinphoneCallLog *cl){
	return cl->quality;
}

const char *linphone_call_log_get_ref_key(const LinphoneCallLog *cl){
	return cl->refkey;
}

const LinphoneAddress *linphone_call_log_get_local_address(const LinphoneCallLog *cl) {
	return (cl->dir == LinphoneCallIncoming) ? cl->to : cl->from;
}

const LinphoneAddress *linphone_call_log_get_remote_address(const LinphoneCallLog *cl){
	return (cl->dir == LinphoneCallIncoming) ? cl->from : cl->to;
}

const rtp_stats_t *linphone_call_log_get_remote_stats(const LinphoneCallLog *cl){
	return &cl->remote_stats;
}

time_t linphone_call_log_get_start_date(const LinphoneCallLog *cl){
	return cl->start_date_time;
}

LinphoneCallStatus linphone_call_log_get_status(const LinphoneCallLog *cl){
	return cl->status;
}

const LinphoneAddress *linphone_call_log_get_to_address(const LinphoneCallLog *cl){
	return cl->to;
}

void linphone_call_log_set_ref_key(LinphoneCallLog *cl, const char *refkey){
	if (cl->refkey!=NULL){
		ms_free(cl->refkey);
		cl->refkey=NULL;
	}
	if (refkey) cl->refkey=ms_strdup(refkey);
}

char * linphone_call_log_to_str(const LinphoneCallLog *cl){
	const char *status;
	char *tmp;
	char *from=linphone_address_as_string (cl->from);
	char *to=linphone_address_as_string (cl->to);
	switch(cl->status){
		case LinphoneCallAborted:
			status="aborted";
			break;
		case LinphoneCallSuccess:
			status="completed";
			break;
		case LinphoneCallMissed:
			status="missed";
			break;
		case LinphoneCallAcceptedElsewhere:
			status="answered elsewhere";
			break;
		case LinphoneCallDeclinedElsewhere:
			status="declined elsewhere";
			break;
		default:
			status="unknown";
	}
	tmp=ms_strdup_printf("%s at %s\nFrom: %s\nTo: %s\nStatus: %s\nDuration: %i mn %i sec\n",
			(cl->dir==LinphoneCallIncoming) ? "Incoming call" : "Outgoing call",
			cl->start_date,
			from,
			to,
			status,
			cl->duration/60,
			cl->duration%60);
	ms_free(from);
	ms_free(to);
	return tmp;
}

bool_t linphone_call_log_video_enabled(const LinphoneCallLog *cl) {
	return cl->video_enabled;
}

bool_t linphone_call_log_was_conference(const LinphoneCallLog *cl) {
	return cl->was_conference;
}

const LinphoneErrorInfo *linphone_call_log_get_error_info(const LinphoneCallLog *cl){
	return cl->error_info;
}


/*******************************************************************************
 * Reference and user data handling functions                                  *
 ******************************************************************************/

void *linphone_call_log_get_user_data(const LinphoneCallLog *cl) {
	return cl->user_data;
}

void linphone_call_log_set_user_data(LinphoneCallLog *cl, void *ud) {
	cl->user_data = ud;
}

LinphoneCallLog * linphone_call_log_ref(LinphoneCallLog *cl) {
	belle_sip_object_ref(cl);
	return cl;
}

void linphone_call_log_unref(LinphoneCallLog *cl) {
	belle_sip_object_unref(cl);
}

/*******************************************************************************
 * Constructor and destructor functions                                        *
 ******************************************************************************/

static void _linphone_call_log_destroy(LinphoneCallLog *cl) {
	if (cl->from!=NULL) linphone_address_unref(cl->from);
	if (cl->to!=NULL) linphone_address_unref(cl->to);
	if (cl->refkey!=NULL) ms_free(cl->refkey);
	if (cl->call_id) ms_free(cl->call_id);
	if (cl->reporting.reports[LINPHONE_CALL_STATS_AUDIO]!=NULL) linphone_reporting_destroy(cl->reporting.reports[LINPHONE_CALL_STATS_AUDIO]);
	if (cl->reporting.reports[LINPHONE_CALL_STATS_VIDEO]!=NULL) linphone_reporting_destroy(cl->reporting.reports[LINPHONE_CALL_STATS_VIDEO]);
	if (cl->reporting.reports[LINPHONE_CALL_STATS_TEXT]!=NULL) linphone_reporting_destroy(cl->reporting.reports[LINPHONE_CALL_STATS_TEXT]);
	if (cl->error_info) linphone_error_info_unref(cl->error_info);
}

LinphoneCallLog * linphone_call_log_new(LinphoneCallDir dir, LinphoneAddress *from, LinphoneAddress *to) {
	LinphoneCallLog *cl=belle_sip_object_new(LinphoneCallLog);
	cl->dir=dir;
	cl->start_date_time=time(NULL);
	set_call_log_date(cl,cl->start_date_time);
	cl->from=from;

	cl->to=to;

	cl->status=LinphoneCallAborted; /*default status*/
	cl->quality=-1;
	cl->storage_id=0;

	cl->reporting.reports[LINPHONE_CALL_STATS_AUDIO]=linphone_reporting_new();
	cl->reporting.reports[LINPHONE_CALL_STATS_VIDEO]=linphone_reporting_new();
	cl->reporting.reports[LINPHONE_CALL_STATS_TEXT]=linphone_reporting_new();
	cl->connected_date_time=0;
	return cl;
}


/* DEPRECATED */
void linphone_call_log_destroy(LinphoneCallLog *cl) {
	belle_sip_object_unref(cl);
}

BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneCallLog);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneCallLog, belle_sip_object_t,
	(belle_sip_object_destroy_t)_linphone_call_log_destroy,
	NULL, // clone
	NULL, // marshal
	FALSE
);


/*******************************************************************************
 * SQL storage related functions                                               *
 ******************************************************************************/

static void linphone_create_call_log_table(sqlite3* db) {
	char* errmsg=NULL;
	int ret;
	ret=sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS call_history ("
							 "id             INTEGER PRIMARY KEY AUTOINCREMENT,"
							 "caller         TEXT NOT NULL," // Can't name a field "from"...
							 "callee         TEXT NOT NULL,"
							 "direction      INTEGER,"
							 "duration       INTEGER,"
							 "start_time     TEXT NOT NULL,"
							 "connected_time TEXT NOT NULL,"
							 "status         INTEGER,"
							 "videoEnabled   INTEGER,"
							 "quality        REAL"
						");",
			0,0,&errmsg);
	if(ret != SQLITE_OK) {
		ms_error("Error in creation: %s.\n", errmsg);
		sqlite3_free(errmsg);
	}
}

static void linphone_update_call_log_table(sqlite3* db) {
	char* errmsg=NULL;
	int ret;

	// for image url storage
	ret=sqlite3_exec(db,"ALTER TABLE call_history ADD COLUMN call_id TEXT;",NULL,NULL,&errmsg);
	if(ret != SQLITE_OK) {
		ms_message("Table already up to date: %s.", errmsg);
		sqlite3_free(errmsg);
	} else {
		ret=sqlite3_exec(db,"ALTER TABLE call_history ADD COLUMN refkey TEXT;",NULL,NULL,&errmsg);
		if(ret != SQLITE_OK) {
			ms_message("Table already up to date: %s.", errmsg);
			sqlite3_free(errmsg);
		} else {
			ms_debug("Table call_history updated successfully for call_id and refkey.");
		}
	}
}

void linphone_core_call_log_storage_init(LinphoneCore *lc) {
	int ret;
	const char *errmsg;
	sqlite3 *db;

	linphone_core_call_log_storage_close(lc);

	ret=_linphone_sqlite3_open(lc->logs_db_file, &db);
	if(ret != SQLITE_OK) {
		errmsg = sqlite3_errmsg(db);
		ms_error("Error in the opening: %s.\n", errmsg);
		sqlite3_close(db);
		return;
	}

	linphone_create_call_log_table(db);
	linphone_update_call_log_table(db);
	lc->logs_db = db;

	// Load the existing call logs
	linphone_core_get_call_history(lc);
}

void linphone_core_call_log_storage_close(LinphoneCore *lc) {
	if (lc->logs_db){
		sqlite3_close(lc->logs_db);
		lc->logs_db = NULL;
	}
}

static LinphoneCallLog * find_call_log_by_storage_id(bctbx_list_t *call_logs, unsigned int storage_id) {
	bctbx_list_t *item;
	for (item = call_logs; item != NULL; item = bctbx_list_next(item)) {
		LinphoneCallLog *call_log = reinterpret_cast<LinphoneCallLog *>(bctbx_list_get_data(item));
		if (call_log->storage_id == storage_id) return call_log;
	}
	return NULL;
}

/* DB layout:
 * | 0  | storage_id
 * | 1  | from
 * | 2  | to
 * | 3  | direction flag
 * | 4  | duration
 * | 5  | start date time (time_t)
 * | 6  | connected date time (time_t)
 * | 7  | status
 * | 8  | video enabled (1 or 0)
 * | 9  | quality
 * | 10 | call_id
 * | 11 | refkey
 */
static int create_call_log(void *data, int argc, char **argv, char **colName) {
	CallLogStorageResult *clsres = (CallLogStorageResult *)data;
	LinphoneAddress *from;
	LinphoneAddress *to;
	LinphoneCallDir dir;
	LinphoneCallLog *log;

	unsigned int storage_id = (unsigned int)atoi(argv[0]);

	log = find_call_log_by_storage_id(clsres->core->call_logs, storage_id);
	if (log != NULL) {
		clsres->result = bctbx_list_append(clsres->result, linphone_call_log_ref(log));
		return 0;
	}

	from = linphone_address_new(argv[1]);
	to = linphone_address_new(argv[2]);

	if (from == NULL || to == NULL) goto error;

	dir = (LinphoneCallDir) atoi(argv[3]);
	log = linphone_call_log_new(dir, from, to);

	log->storage_id = storage_id;
	log->duration = atoi(argv[4]);
	log->start_date_time = (time_t)atol(argv[5]);
	set_call_log_date(log,log->start_date_time);
	log->connected_date_time = (time_t)atol(argv[6]);
	log->status = (LinphoneCallStatus) atoi(argv[7]);
	log->video_enabled = atoi(argv[8]) == 1;
	log->quality = (float)atof(argv[9]);

	if (argc > 10) {
		if (argv[10] != NULL) {
			log->call_id = ms_strdup(argv[10]);
		}
		if (argv[10] != NULL) {
			log->refkey = ms_strdup(argv[11]);
		}
	}

	clsres->result = bctbx_list_append(clsres->result, log);
	return 0;

error:
	if (from){
		linphone_address_unref(from);
	}
	if (to){
		linphone_address_unref(to);
	}
	ms_error("Bad call log at storage_id %u", storage_id);
	return 0;
}

static void linphone_sql_request_call_log(sqlite3 *db, const char *stmt, CallLogStorageResult *clsres) {
	char* errmsg = NULL;
	int ret;
	ret = sqlite3_exec(db, stmt, create_call_log, clsres, &errmsg);
	if (ret != SQLITE_OK) {
		ms_error("linphone_sql_request: statement %s -> error sqlite3_exec(): %s.", stmt, errmsg);
		sqlite3_free(errmsg);
	}
}

static int linphone_sql_request_generic(sqlite3* db, const char *stmt) {
	char* errmsg = NULL;
	int ret;
	ret = sqlite3_exec(db, stmt, NULL, NULL, &errmsg);
	if (ret != SQLITE_OK) {
		ms_error("linphone_sql_request: statement %s -> error sqlite3_exec(): %s.", stmt, errmsg);
		sqlite3_free(errmsg);
	}
	return ret;
}

void linphone_core_store_call_log(LinphoneCore *lc, LinphoneCallLog *log) {
	if (lc && lc->logs_db){
		char *from, *to;
		char *buf;

		from = linphone_address_as_string(log->from);
		to = linphone_address_as_string(log->to);
		buf = sqlite3_mprintf("INSERT INTO call_history VALUES(NULL,%Q,%Q,%i,%i,%lld,%lld,%i,%i,%f,%Q,%Q);",
						from,
						to,
						log->dir,
						log->duration,
						(int64_t)log->start_date_time,
						(int64_t)log->connected_date_time,
						log->status,
						log->video_enabled ? 1 : 0,
						log->quality,
						log->call_id,
						log->refkey
					);
		linphone_sql_request_generic(lc->logs_db, buf);
		sqlite3_free(buf);
		ms_free(from);
		ms_free(to);

		log->storage_id = (unsigned int)sqlite3_last_insert_rowid(lc->logs_db);
	}

	if (lc) {
		lc->call_logs = bctbx_list_prepend(lc->call_logs, linphone_call_log_ref(log));
	}
}

const bctbx_list_t *linphone_core_get_call_history(LinphoneCore *lc) {
	char *buf;
	uint64_t begin,end;
	CallLogStorageResult clsres;

	if (!lc || lc->logs_db == NULL) return NULL;
		if (lc->call_logs != NULL) return lc->call_logs;

	if (lc->max_call_logs != LINPHONE_MAX_CALL_HISTORY_UNLIMITED){
		buf = sqlite3_mprintf("SELECT * FROM call_history ORDER BY id DESC LIMIT %i", lc->max_call_logs);
	}else{
		buf = sqlite3_mprintf("SELECT * FROM call_history ORDER BY id DESC");
	}

	clsres.core = lc;
	clsres.result = NULL;
	begin = ortp_get_cur_time_ms();
	linphone_sql_request_call_log(lc->logs_db, buf, &clsres);
	end = ortp_get_cur_time_ms();
	ms_message("%s(): completed in %i ms",__FUNCTION__, (int)(end-begin));
	sqlite3_free(buf);

	lc->call_logs = clsres.result;
	return lc->call_logs;
}

void linphone_core_delete_call_history(LinphoneCore *lc) {
	char *buf;

	if (!lc || lc->logs_db == NULL) return ;

	buf = sqlite3_mprintf("DELETE FROM call_history");
	linphone_sql_request_generic(lc->logs_db, buf);
	sqlite3_free(buf);
}

void linphone_core_delete_call_log(LinphoneCore *lc, LinphoneCallLog *log) {
	char *buf;

	if (!lc || lc->logs_db == NULL) return ;

	buf = sqlite3_mprintf("DELETE FROM call_history WHERE id = %u", log->storage_id);
	linphone_sql_request_generic(lc->logs_db, buf);
	sqlite3_free(buf);
}

int linphone_core_get_call_history_size(LinphoneCore *lc) {
	int numrows = 0;
	char *buf;
	sqlite3_stmt *selectStatement;
	int returnValue;

	if (!lc)
		return 0;
	if (!lc->logs_db)
		return (int)bctbx_list_size(lc->call_logs);

	buf = sqlite3_mprintf("SELECT count(*) FROM call_history");
	returnValue = sqlite3_prepare_v2(lc->logs_db, buf, -1, &selectStatement, NULL);
	if (returnValue == SQLITE_OK){
		if(sqlite3_step(selectStatement) == SQLITE_ROW){
			numrows = sqlite3_column_int(selectStatement, 0);
		}
	}
	sqlite3_finalize(selectStatement);
	sqlite3_free(buf);

	return numrows;
}

bctbx_list_t * linphone_core_get_call_history_for_address(LinphoneCore *lc, const LinphoneAddress *addr) {
	char *buf;
	char *sipAddress;
	uint64_t begin,end;
	CallLogStorageResult clsres;

	if (!lc || lc->logs_db == NULL || addr == NULL) return NULL;

	/*since we want to append query parameters depending on arguments given, we use malloc instead of sqlite3_mprintf*/
	sipAddress = linphone_address_as_string_uri_only(addr);
	buf = sqlite3_mprintf("SELECT * FROM call_history WHERE caller LIKE '%%%q%%' OR callee LIKE '%%%q%%' ORDER BY id DESC", sipAddress, sipAddress); // The '%%%q%%' takes care of the eventual presence of a display name

	clsres.core = lc;
	clsres.result = NULL;
	begin = ortp_get_cur_time_ms();
	linphone_sql_request_call_log(lc->logs_db, buf, &clsres);
	end = ortp_get_cur_time_ms();
	ms_message("%s(): completed in %i ms",__FUNCTION__, (int)(end-begin));
	sqlite3_free(buf);
	ms_free(sipAddress);

	return clsres.result;
}

bctbx_list_t *linphone_core_get_call_history_2(
	LinphoneCore *lc,
	const LinphoneAddress *peer_addr,
	const LinphoneAddress *local_addr
) {
	char *buf;
	char *peer_addr_str;
	char *local_addr_str;
	uint64_t begin, end;
	CallLogStorageResult clsres;

	if (!lc || !lc->logs_db || !peer_addr || !local_addr) return NULL;

	peer_addr_str = bctbx_strdup(L_GET_CPP_PTR_FROM_C_OBJECT(peer_addr)->asStringUriOnly().c_str());
	local_addr_str = bctbx_strdup(L_GET_CPP_PTR_FROM_C_OBJECT(local_addr)->asStringUriOnly().c_str());
	buf = sqlite3_mprintf(
		"SELECT * FROM call_history WHERE "
		"(caller LIKE '%%%q%%' AND callee LIKE '%%%q%%' AND direction = 0) OR "
		"(caller LIKE '%%%q%%' AND callee LIKE '%%%q%%' AND direction = 1) "
		"ORDER BY id DESC",
		local_addr_str,
		peer_addr_str,
		peer_addr_str,
		local_addr_str
	);

	clsres.core = lc;
	clsres.result = NULL;
	begin = ortp_get_cur_time_ms();
	linphone_sql_request_call_log(lc->logs_db, buf, &clsres);
	end = ortp_get_cur_time_ms();
	bctbx_message("%s(): completed in %i ms", __FUNCTION__, (int)(end - begin));
	sqlite3_free(buf);
	bctbx_free(peer_addr_str);
	bctbx_free(local_addr_str);

	return clsres.result;
}

LinphoneCallLog * linphone_core_get_last_outgoing_call_log(LinphoneCore *lc) {
	char *buf;
	uint64_t begin,end;
	CallLogStorageResult clsres;
	LinphoneCallLog *result = NULL;

	if (!lc || lc->logs_db == NULL) return NULL;

	/*since we want to append query parameters depending on arguments given, we use malloc instead of sqlite3_mprintf*/
	buf = sqlite3_mprintf("SELECT * FROM call_history WHERE direction = 0 ORDER BY id DESC LIMIT 1");

	clsres.core = lc;
	clsres.result = NULL;
	begin = ortp_get_cur_time_ms();
	linphone_sql_request_call_log(lc->logs_db, buf, &clsres);
	end = ortp_get_cur_time_ms();
	ms_message("%s(): completed in %i ms",__FUNCTION__, (int)(end-begin));
	sqlite3_free(buf);

	if (clsres.result != NULL) {
		result = (LinphoneCallLog *)bctbx_list_get_data(clsres.result);
	}

	return result;
}

LinphoneCallLog * linphone_core_find_call_log_from_call_id(LinphoneCore *lc, const char *call_id) {
	char *buf;
	uint64_t begin,end;
	CallLogStorageResult clsres;
	LinphoneCallLog* result = NULL;

	if (!lc || lc->logs_db == NULL) return NULL;

	/*since we want to append query parameters depending on arguments given, we use malloc instead of sqlite3_mprintf*/
	buf = sqlite3_mprintf("SELECT * FROM call_history WHERE call_id = '%q' ORDER BY id DESC LIMIT 1", call_id);

	clsres.core = lc;
	clsres.result = NULL;
	begin = ortp_get_cur_time_ms();
	linphone_sql_request_call_log(lc->logs_db, buf, &clsres);
	end = ortp_get_cur_time_ms();
	ms_message("%s(): completed in %i ms",__FUNCTION__, (int)(end-begin));
	sqlite3_free(buf);

	if (clsres.result != NULL) {
		result = (LinphoneCallLog *)bctbx_list_get_data(clsres.result);
	}

	return result;
}
