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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "linphonecore.h"
#include "sipsetup.h"
#include "lpconfig.h"
#include "private.h"
#include "quality_reporting.h"
#include "lime.h"

#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ortp/telephonyevents.h>
#include <mediastreamer2/zrtp.h>
#include <mediastreamer2/dtls_srtp.h>
#include "mediastreamer2/mediastream.h"
#include "mediastreamer2/mseventqueue.h"
#include "mediastreamer2/msvolume.h"
#include "mediastreamer2/msequalizer.h"
#include "mediastreamer2/dtmfgen.h"

#ifdef INET6
#ifndef _WIN32
#include <netdb.h>
#endif
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#ifndef ANDROID /*on Android LIBLINPHONE version is passed from root Makefile*/
	#include "liblinphone_gitversion.h"
#endif
#else
#ifndef LIBLINPHONE_GIT_VERSION
#define LIBLINPHONE_GIT_VERSION "unknown"
#endif
#endif

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

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

#define ROOT_CA_FILE PACKAGE_DATA_DIR "/linphone/rootca.pem"

static const char *liblinphone_version=
#ifdef LIBLINPHONE_GIT_VERSION
	LIBLINPHONE_GIT_VERSION
#else
	LIBLINPHONE_VERSION
#endif
;
static OrtpLogFunc liblinphone_log_func = NULL;
static LinphoneLogCollectionState liblinphone_log_collection_state = LinphoneLogCollectionDisabled;
static char * liblinphone_log_collection_path = NULL;
static char * liblinphone_log_collection_prefix = NULL;
static int liblinphone_log_collection_max_file_size = LOG_COLLECTION_DEFAULT_MAX_FILE_SIZE;
static ortp_mutex_t liblinphone_log_collection_mutex;
static bool_t liblinphone_serialize_logs = FALSE;
static void set_network_reachable(LinphoneCore* lc,bool_t isReachable, time_t curtime);
static void linphone_core_run_hooks(LinphoneCore *lc);
static void linphone_core_free_hooks(LinphoneCore *lc);

#include "enum.h"
#include "contact_providers_priv.h"


const char *linphone_core_get_nat_address_resolved(LinphoneCore *lc);
static void toggle_video_preview(LinphoneCore *lc, bool_t val);

#if defined(LINPHONE_WINDOWS_PHONE) || defined(LINPHONE_WINDOWS_UNIVERSAL)
#define SOUNDS_PREFIX "Assets/Sounds/"
#else
#define SOUNDS_PREFIX
#endif
/* relative path where is stored local ring*/
#define LOCAL_RING SOUNDS_PREFIX "rings/oldphone.wav"
/* same for remote ring (ringback)*/
#define REMOTE_RING SOUNDS_PREFIX "ringback.wav"
#define HOLD_MUSIC SOUNDS_PREFIX "rings/toy-mono.wav"


extern SalCallbacks linphone_sal_callbacks;

void lc_callback_obj_init(LCCallbackObj *obj,LinphoneCoreCbFunc func,void* ud)
{
  obj->_func=func;
  obj->_user_data=ud;
}

int lc_callback_obj_invoke(LCCallbackObj *obj, LinphoneCore *lc){
	if (obj->_func!=NULL) obj->_func(lc,obj->_user_data);
	return 0;
}


/**
 * Returns TRUE if the LinphoneCall asked to autoanswer
 *
**/
bool_t linphone_call_asked_to_autoanswer(LinphoneCall *call){
	//return TRUE if the unique(for the moment) incoming call asked to be autoanswered
	if(call)
		return sal_call_autoanswer_asked(call->op);
	else
		return FALSE;
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

static void linphone_core_log_collection_handler(OrtpLogLevel level, const char *fmt, va_list args);

void linphone_core_set_log_handler(OrtpLogFunc logfunc) {
	if (ortp_logv_out == linphone_core_log_collection_handler) {
		ms_message("There is already a log collection handler, keep it");
		liblinphone_log_func = logfunc;
	} else
		ortp_set_log_handler(logfunc);
}

void linphone_core_set_log_file(FILE *file) {
	if (file == NULL) file = stdout;
	ortp_set_log_file(file);
}

void linphone_core_set_log_level(OrtpLogLevel loglevel) {
	OrtpLogLevel mask = loglevel;
	switch (loglevel) {
		case ORTP_TRACE:
		case ORTP_DEBUG:
			mask |= ORTP_DEBUG;
		case ORTP_MESSAGE:
			mask |= ORTP_MESSAGE;
		case ORTP_WARNING:
			mask |= ORTP_WARNING;
		case ORTP_ERROR:
			mask |= ORTP_ERROR;
		case ORTP_FATAL:
			mask |= ORTP_FATAL;
			break;
		case ORTP_LOGLEV_END:
			break;
	}
	linphone_core_set_log_level_mask(mask);
}

void linphone_core_set_log_level_mask(OrtpLogLevel loglevel) {
	ortp_set_log_level_mask(loglevel);
	if (loglevel == 0) {
		sal_disable_log();
	} else {
		sal_enable_log();
	}
}

static void linphone_core_log_collection_handler(OrtpLogLevel level, const char *fmt, va_list args) {
	const char *lname="undef";
	char *msg;
	char *log_filename1;
	char *log_filename2;
	FILE *log_file;
	struct timeval tp;
	struct tm *lt;
	time_t tt;
	struct stat statbuf;

	if (liblinphone_log_func != NULL) {
#ifndef _WIN32
		va_list args_copy;
		va_copy(args_copy, args);
		liblinphone_log_func(level, fmt, args_copy);
		va_end(args_copy);
#else
		/* This works on 32 bits, luckily. */
		/* TODO: va_copy is available in Visual Studio 2013. */
		liblinphone_log_func(level, fmt, args);
#endif
	}

	ortp_gettimeofday(&tp, NULL);
	tt = (time_t)tp.tv_sec;
	lt = localtime((const time_t*)&tt);
	switch(level){
		case ORTP_DEBUG:
			lname = "DEBUG";
			break;
		case ORTP_MESSAGE:
			lname = "MESSAGE";
			break;
		case ORTP_WARNING:
			lname = "WARNING";
			break;
		case ORTP_ERROR:
			lname = "ERROR";
			break;
		case ORTP_FATAL:
			lname = "FATAL";
			break;
		default:
			ortp_fatal("Bad level !");
	}
	msg = ortp_strdup_vprintf(fmt, args);

	log_filename1 = ortp_strdup_printf("%s/%s1.log",
		liblinphone_log_collection_path ? liblinphone_log_collection_path : LOG_COLLECTION_DEFAULT_PATH,
		liblinphone_log_collection_prefix ? liblinphone_log_collection_prefix : LOG_COLLECTION_DEFAULT_PREFIX);
	log_filename2 = ortp_strdup_printf("%s/%s2.log",
		liblinphone_log_collection_path ? liblinphone_log_collection_path : LOG_COLLECTION_DEFAULT_PATH,
		liblinphone_log_collection_prefix ? liblinphone_log_collection_prefix : LOG_COLLECTION_DEFAULT_PREFIX);
	ortp_mutex_lock(&liblinphone_log_collection_mutex);
	log_file = fopen(log_filename1, "a");
	fstat(fileno(log_file), &statbuf);
	if (statbuf.st_size > liblinphone_log_collection_max_file_size) {
		fclose(log_file);
		log_file = fopen(log_filename2, "a");
		fstat(fileno(log_file), &statbuf);
		if (statbuf.st_size > liblinphone_log_collection_max_file_size) {
			fclose(log_file);
			unlink(log_filename1);
			rename(log_filename2, log_filename1);
			log_file = fopen(log_filename2, "a");
		}
	}
	fprintf(log_file,"%i-%.2i-%.2i %.2i:%.2i:%.2i:%.3i %s %s\n",
		1900 + lt->tm_year, lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec, (int)(tp.tv_usec / 1000), lname, msg);
	fflush(log_file);
	fclose(log_file);
	ortp_mutex_unlock(&liblinphone_log_collection_mutex);

	ortp_free(log_filename1);
	ortp_free(log_filename2);
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

int linphone_core_get_log_collection_max_file_size(void) {
	return liblinphone_log_collection_max_file_size;
}

void linphone_core_set_log_collection_max_file_size(int size) {
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

	/* at first call of this function, set liblinphone_log_func to the current
	 * ortp log function */
	if( liblinphone_log_func == NULL ){
		liblinphone_log_func = ortp_logv_out;
	}
	liblinphone_log_collection_state = state;
	if (state != LinphoneLogCollectionDisabled) {
		ortp_mutex_init(&liblinphone_log_collection_mutex, NULL);
		if (state == LinphoneLogCollectionEnabledWithoutPreviousLogHandler) {
			liblinphone_log_func = NULL;
		} else {
			liblinphone_log_func = ortp_logv_out;
		}
		ortp_set_log_handler(linphone_core_log_collection_handler);
	} else {
		ortp_set_log_handler(liblinphone_log_func);
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
		ms_free(lc->log_collection_upload_information);
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
 *
 * @param[in] bh The body handler
 * @param[in] msg The belle sip message
 * @param[in] data The user data associated with the handler, contains the LinphoneCore object
 * @param[in] offset The current position in the input buffer
 * @param[in] buffer The ouput buffer where to copy the data to be uploaded
 * @param[in,out] size The size in byte of the data requested, as output it will contain the effective copied size
 *
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
		if (fseek(log_file, offset, SEEK_SET)) {
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
			first_part_bh = belle_sip_user_body_handler_new(linphone_content_get_size(core->log_collection_upload_information), NULL, NULL, log_collection_upload_on_send_body, core);
			belle_sip_body_handler_add_header((belle_sip_body_handler_t *)first_part_bh, belle_sip_header_create("Content-disposition", first_part_header));
			belle_sip_free(first_part_header);
			belle_sip_body_handler_add_header((belle_sip_body_handler_t *)first_part_bh,
				(belle_sip_header_t *)belle_sip_header_content_type_create(linphone_content_get_type(core->log_collection_upload_information), linphone_content_get_subtype(core->log_collection_upload_information)));

			/* Insert it in a multipart body handler which will manage the boundaries of multipart message */
			bh = belle_sip_multipart_body_handler_new(log_collection_upload_on_progress, core, (belle_sip_body_handler_t *)first_part_bh);
			ua = ms_strdup_printf("%s/%s", linphone_core_get_user_agent_name(), linphone_core_get_user_agent_version());
			uri = belle_generic_uri_parse(linphone_core_get_log_collection_upload_server_url(core));
			req = belle_http_request_create("POST", uri, belle_sip_header_create("User-Agent", ua), NULL);
			ms_free(ua);
			belle_sip_message_set_body_handler(BELLE_SIP_MESSAGE(req), BELLE_SIP_BODY_HANDLER(bh));
			cbs.process_response = process_response_from_post_file_log_collection;
			cbs.process_io_error = process_io_error_upload_log_collection;
			cbs.process_auth_requested = process_auth_requested_upload_log_collection;
			l = belle_http_request_listener_create_from_callbacks(&cbs, core);
			belle_http_provider_send_request(core->http_provider, req, l);
		}
		if (code == 200) { /* The file has been uploaded correctly, get the server reply */
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
									file_url = 	xmlGetProp(cur, (const xmlChar *)"url");
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
			}
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
	int bytes;

	while ((bytes = fread(buffer, 1, sizeof(buffer), input_file)) > 0) {
		if (bytes < 0) return bytes;
#ifdef HAVE_ZLIB
		bytes = gzwrite(output_file, buffer, bytes);
#else
		bytes = fwrite(buffer, 1, bytes, output_file);
#endif
		if (bytes < 0) return bytes;
	}
	return 0;
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
	output_file = COMPRESS_OPEN(output_filename, "w");
	if (output_file == NULL) goto error;
	input_filename = ms_strdup_printf("%s/%s1.log",
		liblinphone_log_collection_path ? liblinphone_log_collection_path : LOG_COLLECTION_DEFAULT_PATH,
		liblinphone_log_collection_prefix ? liblinphone_log_collection_prefix : LOG_COLLECTION_DEFAULT_PREFIX);
	input_file = fopen(input_filename, "r");
	if (input_file == NULL) goto error;
	ret = compress_file(input_file, output_file);
	if (ret < 0) goto error;
	fclose(input_file);
	ms_free(input_filename);
	input_filename = ms_strdup_printf("%s/%s2.log",
		liblinphone_log_collection_path ? liblinphone_log_collection_path : LOG_COLLECTION_DEFAULT_PATH,
		liblinphone_log_collection_prefix ? liblinphone_log_collection_prefix : LOG_COLLECTION_DEFAULT_PREFIX);
	input_file = fopen(input_filename, "r");
	if (input_file != NULL) {
		ret = compress_file(input_file, output_file);
		if (ret < 0) goto error;
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
	return statbuf.st_size;
}

void linphone_core_upload_log_collection(LinphoneCore *core) {
	if ((core->log_collection_upload_information == NULL) && (linphone_core_get_log_collection_upload_server_url(core) != NULL) && (liblinphone_log_collection_state != LinphoneLogCollectionDisabled)) {
		/* open a transaction with the server and send an empty request(RCS5.1 section 3.5.4.8.3.1) */
		belle_http_request_listener_callbacks_t cbs = { 0 };
		belle_http_request_listener_t *l;
		belle_generic_uri_t *uri;
		belle_http_request_t *req;
		char *name;

		core->log_collection_upload_information = (LinphoneContent *)malloc(sizeof(LinphoneContent));
		memset(core->log_collection_upload_information, 0, sizeof(LinphoneContent));
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
		if (prepare_log_collection_file_to_upload(name) < 0) return;
		linphone_content_set_size(core->log_collection_upload_information, get_size_of_file_to_upload(name));
		uri = belle_generic_uri_parse(linphone_core_get_log_collection_upload_server_url(core));
		req = belle_http_request_create("POST", uri, NULL, NULL, NULL);
		cbs.process_response = process_response_from_post_file_log_collection;
		cbs.process_io_error = process_io_error_upload_log_collection;
		cbs.process_auth_requested = process_auth_requested_upload_log_collection;
		l = belle_http_request_listener_create_from_callbacks(&cbs, core);
		belle_http_provider_send_request(core->http_provider, req, l);
		ms_free(name);
	}
}

char * linphone_core_compress_log_collection() {
	char *filename = NULL;
	if (liblinphone_log_collection_state == LinphoneLogCollectionDisabled) return NULL;
	filename = ms_strdup_printf("%s_log.%s",
		liblinphone_log_collection_prefix ? liblinphone_log_collection_prefix : LOG_COLLECTION_DEFAULT_PREFIX,
		COMPRESSED_LOG_COLLECTION_EXTENSION);
	if (prepare_log_collection_file_to_upload(filename) < 0) {
		ms_free(filename);
		return NULL;
	}
	ms_free(filename);
	return ms_strdup_printf("%s/%s_log.%s",
		liblinphone_log_collection_path ? liblinphone_log_collection_path : LOG_COLLECTION_DEFAULT_PATH,
		liblinphone_log_collection_prefix ? liblinphone_log_collection_prefix : LOG_COLLECTION_DEFAULT_PREFIX,
		COMPRESSED_LOG_COLLECTION_EXTENSION);
}

void linphone_core_reset_log_collection() {
	char *filename;
	ortp_mutex_lock(&liblinphone_log_collection_mutex);
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
	ortp_mutex_unlock(&liblinphone_log_collection_mutex);
}

/**
 * Enable logs in supplied FILE*.
 *
 * @ingroup misc
 * @deprecated Use #linphone_core_set_log_file and #linphone_core_set_log_level instead.
 *
 * @param file a C FILE* where to fprintf logs. If null stdout is used.
 *
**/
void linphone_core_enable_logs(FILE *file){
	if (file==NULL) file=stdout;
	ortp_set_log_file(file);
	linphone_core_set_log_level(ORTP_MESSAGE);
}

/**
 * Enable logs through the user's supplied log callback.
 *
 * @ingroup misc
 * @deprecated Use #linphone_core_set_log_handler and #linphone_core_set_log_level instead.
 *
 * @param logfunc The address of a OrtpLogFunc callback whose protoype is
 *            	  typedef void (*OrtpLogFunc)(OrtpLogLevel lev, const char *fmt, va_list args);
 *
**/
void linphone_core_enable_logs_with_cb(OrtpLogFunc logfunc){
	linphone_core_set_log_level(ORTP_MESSAGE);
	linphone_core_set_log_handler(logfunc);
}

/**
 * Entirely disable logging.
 *
 * @ingroup misc
 * @deprecated Use #linphone_core_set_log_level instead.
**/
void linphone_core_disable_logs(void){
	linphone_core_set_log_level(ORTP_ERROR);
}

void linphone_core_serialize_logs(void) {
	liblinphone_serialize_logs = TRUE;
}


static void net_config_read (LinphoneCore *lc)
{
	int tmp;
	const char *tmpstr;
	LpConfig *config=lc->config;

	lc->net_conf.nat_address_ip = NULL;
	tmp=lp_config_get_int(config,"net","download_bw",0);
	linphone_core_set_download_bandwidth(lc,tmp);
	tmp=lp_config_get_int(config,"net","upload_bw",0);
	linphone_core_set_upload_bandwidth(lc,tmp);
	linphone_core_set_stun_server(lc,lp_config_get_string(config,"net","stun_server",NULL));
	tmpstr=lp_config_get_string(lc->config,"net","nat_address",NULL);
	if (tmpstr!=NULL && (strlen(tmpstr)<1)) tmpstr=NULL;
	linphone_core_set_nat_address(lc,tmpstr);
	tmp=lp_config_get_int(lc->config,"net","nat_sdp_only",0);
	lc->net_conf.nat_sdp_only=tmp;
	tmp=lp_config_get_int(lc->config,"net","mtu",1300);
	linphone_core_set_mtu(lc,tmp);
	tmp=lp_config_get_int(lc->config,"net","download_ptime",-1);
	if (tmp !=-1 && linphone_core_get_download_ptime(lc) !=0) {
		/*legacy parameter*/
		linphone_core_set_download_ptime(lc,tmp);
	}
	tmp = lp_config_get_int(lc->config, "net", "dns_srv_enabled", 1);
	linphone_core_enable_dns_srv(lc, tmp);

	/* This is to filter out unsupported firewall policies */
	linphone_core_set_firewall_policy(lc, linphone_core_get_firewall_policy(lc));
}

static void build_sound_devices_table(LinphoneCore *lc){
	const char **devices;
	const char **old;
	int ndev;
	int i;
	const MSList *elem=ms_snd_card_manager_get_list(ms_snd_card_manager_get());
	ndev=ms_list_size(elem);
	devices=ms_malloc((ndev+1)*sizeof(const char *));
	for (i=0;elem!=NULL;elem=elem->next,i++){
		devices[i]=ms_snd_card_get_string_id((MSSndCard *)elem->data);
	}
	devices[ndev]=NULL;
	old=lc->sound_conf.cards;
	lc->sound_conf.cards=devices;
	if (old!=NULL) ms_free(old);
}

static void sound_config_read(LinphoneCore *lc)
{
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
		char* d=malloc(l+1);
		char* i;
		memcpy(d,devid,l+1);
		for (l=0,i=strpbrk(d+l,delim);i;i=strpbrk(d+l,delim)){
			char s=*i;
			*i='\0';
			card=ms_alsa_card_new_custom(d+l,d+l);
			ms_snd_card_manager_add_card(ms_snd_card_manager_get(),card);
			*i=s;
			l=i-d+1;
		}
		if(d[l]!='\0') {
			card=ms_alsa_card_new_custom(d+l,d+l);
			ms_snd_card_manager_add_card(ms_snd_card_manager_get(),card);
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

	tmpbuf=PACKAGE_SOUND_DIR "/" LOCAL_RING;
	tmpbuf=lp_config_get_string(lc->config,"sound","local_ring",tmpbuf);
	if (ortp_file_exist(tmpbuf)==-1) {
		ms_warning("%s does not exist",tmpbuf);
		tmpbuf=PACKAGE_SOUND_DIR "/" LOCAL_RING;
	}
	if (strstr(tmpbuf,".wav")==NULL){
		/* it currently uses old sound files, so replace them */
		tmpbuf=PACKAGE_SOUND_DIR "/" LOCAL_RING;
	}
	linphone_core_set_ring(lc,tmpbuf);

	tmpbuf=PACKAGE_SOUND_DIR "/" REMOTE_RING;
	tmpbuf=lp_config_get_string(lc->config,"sound","remote_ring",tmpbuf);
	if (ortp_file_exist(tmpbuf)==-1){
		tmpbuf=PACKAGE_SOUND_DIR "/" REMOTE_RING;
	}
	if (strstr(tmpbuf,".wav")==NULL){
		/* it currently uses old sound files, so replace them */
		tmpbuf=PACKAGE_SOUND_DIR "/" REMOTE_RING;
	}
	linphone_core_set_ringback(lc,tmpbuf);

	linphone_core_set_play_file(lc,lp_config_get_string(lc->config,"sound","hold_music",PACKAGE_SOUND_DIR "/" HOLD_MUSIC));
	lc->sound_conf.latency=0;
#ifndef __ios
	tmp=TRUE;
#else
	tmp=FALSE; /* on iOS we have builtin echo cancellation.*/
#endif
	tmp=lp_config_get_int(lc->config,"sound","echocancellation",tmp);
	linphone_core_enable_echo_cancellation(lc,tmp);
	linphone_core_enable_echo_limiter(lc,
		lp_config_get_int(lc->config,"sound","echolimiter",0));
	linphone_core_enable_agc(lc,
		lp_config_get_int(lc->config,"sound","agc",0));

	linphone_core_set_playback_gain_db (lc,lp_config_get_float(lc->config,"sound","playback_gain_db",0));
	linphone_core_set_mic_gain_db (lc,lp_config_get_float(lc->config,"sound","mic_gain_db",0));

	linphone_core_set_remote_ringback_tone (lc,lp_config_get_string(lc->config,"sound","ringback_tone",NULL));

	/*just parse requested stream feature once at start to print out eventual errors*/
	linphone_core_get_audio_features(lc);

	_linphone_core_set_tone(lc,LinphoneReasonBusy,LinphoneToneBusy,NULL);
}

static void certificates_config_read(LinphoneCore *lc)
{
	const char *rootca;
#ifdef __linux
	struct stat sb;
	rootca=lp_config_get_string(lc->config,"sip","root_ca", "/etc/ssl/certs");
	if (stat("/etc/ssl/certs", &sb) != 0 || !S_ISDIR(sb.st_mode))
	{
		ms_warning("/etc/ssl/certs not found, using %s instead", ROOT_CA_FILE);
		rootca=lp_config_get_string(lc->config,"sip","root_ca", ROOT_CA_FILE);
	}
#else
	rootca=lp_config_get_string(lc->config,"sip","root_ca", ROOT_CA_FILE);
#endif
	linphone_core_set_root_ca(lc,rootca);
	linphone_core_verify_server_certificates(lc,lp_config_get_int(lc->config,"sip","verify_server_certs",TRUE));
	linphone_core_verify_server_cn(lc,lp_config_get_int(lc->config,"sip","verify_server_cn",TRUE));
}

static void sip_config_read(LinphoneCore *lc)
{
	char *contact;
	const char *tmpstr;
	LCSipTransports tr;
	int i,tmp;
	int ipv6;

	if (lp_config_get_int(lc->config,"sip","use_session_timers",0)==1){
		sal_use_session_timers(lc->sal,200);
	}

	sal_use_no_initial_route(lc->sal,lp_config_get_int(lc->config,"sip","use_no_initial_route",0));
	sal_use_rport(lc->sal,lp_config_get_int(lc->config,"sip","use_rport",1));

	ipv6=lp_config_get_int(lc->config,"sip","use_ipv6",-1);
	if (ipv6==-1){
		ipv6=0;
	}
	linphone_core_enable_ipv6(lc,ipv6);
	memset(&tr,0,sizeof(tr));

	tr.udp_port=lp_config_get_int(lc->config,"sip","sip_port",5060);
	tr.tcp_port=lp_config_get_int(lc->config,"sip","sip_tcp_port",5060);
	/*we are not listening inbound connection for tls, port has no meaning*/
	tr.tls_port=lp_config_get_int(lc->config,"sip","sip_tls_port",LC_SIP_TRANSPORT_RANDOM);

	certificates_config_read(lc);
	/*setting the dscp must be done before starting the transports, otherwise it is not taken into effect*/
	sal_set_dscp(lc->sal,linphone_core_get_sip_dscp(lc));
	/*start listening on ports*/
	linphone_core_set_sip_transports(lc,&tr);

	tmpstr=lp_config_get_string(lc->config,"sip","contact",NULL);
	if (tmpstr==NULL || linphone_core_set_primary_contact(lc,tmpstr)==-1) {
		const char *hostname=NULL;
		const char *username=NULL;
#ifdef HAVE_GETENV
		hostname=getenv("HOST");
		username=getenv("USER");
		if (hostname==NULL) hostname=getenv("HOSTNAME");
#endif /*HAVE_GETENV*/
		if (hostname==NULL)
			hostname="unknown-host";
		if (username==NULL){
			username="toto";
		}
		contact=ortp_strdup_printf("sip:%s@%s",username,hostname);
		linphone_core_set_primary_contact(lc,contact);
		ms_free(contact);
	}

	tmp=lp_config_get_int(lc->config,"sip","guess_hostname",1);
	linphone_core_set_guess_hostname(lc,tmp);

	tmp=lp_config_get_int(lc->config,"sip","lime",FALSE);
	linphone_core_enable_lime(lc,tmp);

	tmp=lp_config_get_int(lc->config,"sip","inc_timeout",30);
	linphone_core_set_inc_timeout(lc,tmp);

	tmp=lp_config_get_int(lc->config,"sip","in_call_timeout",0);
	linphone_core_set_in_call_timeout(lc,tmp);

	tmp=lp_config_get_int(lc->config,"sip","delayed_timeout",4);
	linphone_core_set_delayed_timeout(lc,tmp);

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

	/* read authentication information */
	for(i=0;; i++){
		LinphoneAuthInfo *ai=linphone_auth_info_new_from_config_file(lc->config,i);
		if (ai!=NULL){
			linphone_core_add_auth_info(lc,ai);
			linphone_auth_info_destroy(ai);
		}else{
			break;
		}
	}
	/*this is to filter out unsupported encryption schemes*/
	linphone_core_set_media_encryption(lc,linphone_core_get_media_encryption(lc));

	/*for tuning or test*/
	lc->sip_conf.sdp_200_ack=lp_config_get_int(lc->config,"sip","sdp_200_ack",0);
	lc->sip_conf.register_only_when_network_is_up=
		lp_config_get_int(lc->config,"sip","register_only_when_network_is_up",1);
	lc->sip_conf.register_only_when_upnp_is_ok=
		lp_config_get_int(lc->config,"sip","register_only_when_upnp_is_ok",1);
	lc->sip_conf.ping_with_options=lp_config_get_int(lc->config,"sip","ping_with_options",0);
	lc->sip_conf.auto_net_state_mon=lp_config_get_int(lc->config,"sip","auto_net_state_mon",1);
	lc->sip_conf.keepalive_period=lp_config_get_int(lc->config,"sip","keepalive_period",10000);
	lc->sip_conf.tcp_tls_keepalive=lp_config_get_int(lc->config,"sip","tcp_tls_keepalive",0);
	linphone_core_enable_keep_alive(lc, (lc->sip_conf.keepalive_period > 0));
	sal_use_one_matching_codec_policy(lc->sal,lp_config_get_int(lc->config,"sip","only_one_codec",0));
	sal_use_dates(lc->sal,lp_config_get_int(lc->config,"sip","put_date",0));
	sal_enable_sip_update_method(lc->sal,lp_config_get_int(lc->config,"sip","sip_update",1));
	lc->sip_conf.vfu_with_info=lp_config_get_int(lc->config,"sip","vfu_with_info",1);
	linphone_core_set_sip_transport_timeout(lc, lp_config_get_int(lc->config, "sip", "transport_timeout", 63000));
	sal_set_supported_tags(lc->sal,lp_config_get_string(lc->config,"sip","supported","replaces, outbound"));
	lc->sip_conf.save_auth_info = lp_config_get_int(lc->config, "sip", "save_auth_info", 1);
}

static void rtp_config_read(LinphoneCore *lc)
{
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

	jitt_comp=lp_config_get_int(lc->config,"rtp","audio_jitt_comp",60);
	linphone_core_set_audio_jittcomp(lc,jitt_comp);
	jitt_comp=lp_config_get_int(lc->config,"rtp","video_jitt_comp",60);
	if (jitt_comp==0) jitt_comp=60;
	linphone_core_set_video_jittcomp(lc,jitt_comp);
	nortp_timeout=lp_config_get_int(lc->config,"rtp","nortp_timeout",30);
	linphone_core_set_nortp_timeout(lc,nortp_timeout);
	rtp_no_xmit_on_audio_mute=lp_config_get_int(lc->config,"rtp","rtp_no_xmit_on_audio_mute",FALSE);
	linphone_core_set_rtp_no_xmit_on_audio_mute(lc,rtp_no_xmit_on_audio_mute);
	adaptive_jitt_comp_enabled = lp_config_get_int(lc->config, "rtp", "audio_adaptive_jitt_comp_enabled", TRUE);
	linphone_core_enable_audio_adaptive_jittcomp(lc, adaptive_jitt_comp_enabled);
	adaptive_jitt_comp_enabled = lp_config_get_int(lc->config, "rtp", "video_adaptive_jitt_comp_enabled", TRUE);
	linphone_core_enable_video_adaptive_jittcomp(lc, adaptive_jitt_comp_enabled);
	lc->rtp_conf.disable_upnp = lp_config_get_int(lc->config, "rtp", "disable_upnp", FALSE);
	linphone_core_set_avpf_mode(lc,lp_config_get_int(lc->config,"rtp","avpf",0));
	if ((tmp=lp_config_get_string(lc->config,"rtp","audio_multicast_addr",NULL)))
		linphone_core_set_audio_multicast_addr(lc,tmp);
	else
		lc->rtp_conf.audio_multicast_addr=ms_strdup("224.1.2.3");
	if ((tmp_int=lp_config_get_int(lc->config,"rtp","audio_multicast_enabled",-1)) >-1)
		linphone_core_enable_audio_multicast(lc,tmp_int);
	if ((tmp_int=lp_config_get_int(lc->config,"rtp","audio_multicast_ttl",-1))>0)
			linphone_core_set_audio_multicast_ttl(lc,tmp_int);
	else
		lc->rtp_conf.audio_multicast_ttl=1;/*local network*/
	if ((tmp=lp_config_get_string(lc->config,"rtp","video_multicast_addr",NULL)))
		linphone_core_set_video_multicast_addr(lc,tmp);
	else
		lc->rtp_conf.video_multicast_addr=ms_strdup("224.1.2.3");
	if ((tmp_int=lp_config_get_int(lc->config,"rtp","video_multicast_ttl",-1))>-1)
		linphone_core_set_video_multicast_ttl(lc,tmp_int);
	else
		lc->rtp_conf.video_multicast_ttl=1;/*local network*/
	if ((tmp_int=lp_config_get_int(lc->config,"rtp","video_multicast_enabled",-1)) >0)
		linphone_core_enable_video_multicast(lc,tmp_int);
}

static PayloadType * find_payload(const MSList *default_list, const char *mime_type, int clock_rate, int channels, const char *recv_fmtp){
	PayloadType *candidate=NULL;
	PayloadType *it;
	const MSList *elem;

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

static PayloadType* find_payload_type_from_list(const char* type, int rate, int channels, const MSList* from) {
	const MSList *elem;
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
	}
	return ms_filter_codec_supported(mime);
}


static bool_t get_codec(LinphoneCore *lc, SalStreamType type, int index, PayloadType **ret){
	char codeckey[50];
	const char *mime,*fmtp;
	int rate,channels,enabled;
	PayloadType *pt;
	LpConfig *config=lc->config;

	*ret=NULL;
	snprintf(codeckey,50,"%s_codec_%i",type==SalAudio ? "audio" : "video", index);
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
	pt=find_payload(type==SalAudio ? lc->default_audio_codecs : lc->default_video_codecs,mime,rate,channels,fmtp);
	if (!pt){
		MSList **default_list=(type==SalAudio) ? &lc->default_audio_codecs :  &lc->default_video_codecs;
		if (type==SalAudio)
			ms_warning("Codec %s/%i/%i read from conf is not in the default list.",mime,rate,channels);
		else
			ms_warning("Codec %s/%i read from conf is not in the default list.",mime,rate);
		pt=payload_type_new();
		pt->type=(type==SalAudio) ? PAYLOAD_AUDIO_PACKETIZED :  PAYLOAD_VIDEO;
		pt->mime_type=ortp_strdup(mime);
		pt->clock_rate=rate;
		pt->channels=channels;
		payload_type_set_number(pt,-1); /*dynamic assignment*/
		payload_type_set_recv_fmtp(pt,fmtp);
		*default_list=ms_list_append(*default_list, pt);
	}
	if (enabled ) pt->flags|=PAYLOAD_TYPE_ENABLED;
	else pt->flags&=~PAYLOAD_TYPE_ENABLED;
	*ret=pt;
	return TRUE;
}

/*this function merges the payload types from the codec default list with the list read from configuration file.
 * If a new codec becomes supported in Liblinphone or if the list from configuration file is empty or incomplete, all the supported codecs are added
 * automatically. This 'l' list is entirely destroyed and rewritten.*/
static MSList *add_missing_codecs(const MSList *default_list, MSList *l){
	const MSList *elem;
	MSList *newlist;

	for(elem=default_list; elem!=NULL; elem=elem->next){
		MSList *elem2=ms_list_find(l,elem->data);
		if (!elem2){
			PayloadType *pt=(PayloadType*)elem->data;
			/*this codec from default list should be inserted in the list*/
			if (!elem->prev){
				l=ms_list_prepend(l,pt);
			}else{
				const MSList *after=ms_list_find(l,elem->prev->data);
				l=ms_list_insert(l, after->next, pt);
			}
			ms_message("Supported codec %s/%i fmtp=%s automatically added to codec list.", pt->mime_type,
				   pt->clock_rate, pt->recv_fmtp ? pt->recv_fmtp : "");
		}
	}
	newlist=ms_list_copy_with_data(l,(void *(*)(void*))payload_type_clone);
	ms_list_free(l);
	return newlist;
}

static MSList *codec_append_if_new(MSList *l, PayloadType *pt){
	MSList *elem;
	for (elem=l;elem!=NULL;elem=elem->next){
		PayloadType *ept=(PayloadType*)elem->data;
		if (pt==ept)
			return l;
	}
	l=ms_list_append(l,pt);
	return l;
}

static void codecs_config_read(LinphoneCore *lc)
{
	int i;
	PayloadType *pt;
	MSList *audio_codecs=NULL;
	MSList *video_codecs=NULL;

	lc->codecs_conf.dyn_pt=96;
	lc->codecs_conf.telephone_event_pt=lp_config_get_int(lc->config,"misc","telephone_event_pt",101);

	for (i=0;get_codec(lc,SalAudio,i,&pt);i++){
		if (pt){
			audio_codecs=codec_append_if_new(audio_codecs, pt);
		}
	}
	if( lp_config_get_int(lc->config, "misc", "add_missing_audio_codecs", 1) == 1 ){
		audio_codecs=add_missing_codecs(lc->default_audio_codecs,audio_codecs);
	}

	for (i=0;get_codec(lc,SalVideo,i,&pt);i++){
		if (pt){
			video_codecs=codec_append_if_new(video_codecs, pt);
		}
	}
	if( lp_config_get_int(lc->config, "misc", "add_missing_video_codecs", 1) == 1 ){
		video_codecs=add_missing_codecs(lc->default_video_codecs,video_codecs);
	}
	linphone_core_set_audio_codecs(lc,audio_codecs);
	linphone_core_set_video_codecs(lc,video_codecs);
	linphone_core_update_allocated_audio_bandwidth(lc);
}

static void build_video_devices_table(LinphoneCore *lc){
	const MSList *elem;
	int i;
	int ndev;
	const char **devices;
	if (lc->video_conf.cams)
		ms_free(lc->video_conf.cams);
	/* retrieve all video devices */
	elem=ms_web_cam_manager_get_list(ms_web_cam_manager_get());
	ndev=ms_list_size(elem);
	devices=ms_malloc((ndev+1)*sizeof(const char *));
	for (i=0;elem!=NULL;elem=elem->next,i++){
		devices[i]=ms_web_cam_get_string_id((MSWebCam *)elem->data);
	}
	devices[ndev]=NULL;
	lc->video_conf.cams=devices;
}

static void video_config_read(LinphoneCore *lc){
#ifdef VIDEO_ENABLED
	int capture, display, self_view, reuse_source;
	int automatic_video=1;
#endif
	const char *str;
#ifdef VIDEO_ENABLED
	LinphoneVideoPolicy vpol;
	memset(&vpol, 0, sizeof(LinphoneVideoPolicy));
#endif
	build_video_devices_table(lc);

	str=lp_config_get_string(lc->config,"video","device",NULL);
	if (str && str[0]==0) str=NULL;
	linphone_core_set_video_device(lc,str);

	linphone_core_set_preferred_video_size_by_name(lc,
		lp_config_get_string(lc->config,"video","size","cif"));

	linphone_core_set_preview_video_size_by_name(lc,
		lp_config_get_string(lc->config,"video","preview_size",NULL));

	linphone_core_set_preferred_framerate(lc,lp_config_get_float(lc->config,"video","framerate",0));

#ifdef VIDEO_ENABLED
#if defined(ANDROID) || defined(__ios)
	automatic_video=0;
#endif
	capture=lp_config_get_int(lc->config,"video","capture",1);
	display=lp_config_get_int(lc->config,"video","display",1);
	self_view=lp_config_get_int(lc->config,"video","self_view",1);
	reuse_source=lp_config_get_int(lc->config,"video","reuse_source",0);
	vpol.automatically_initiate=lp_config_get_int(lc->config,"video","automatically_initiate",automatic_video);
	vpol.automatically_accept=lp_config_get_int(lc->config,"video","automatically_accept",automatic_video);
	linphone_core_enable_video_capture(lc, capture);
	linphone_core_enable_video_display(lc, display);
	linphone_core_enable_video_preview(lc,lp_config_get_int(lc->config,"video","show_local",0));
	linphone_core_enable_self_view(lc,self_view);
	linphone_core_enable_video_source_reuse(lc, reuse_source);
	linphone_core_set_video_policy(lc,&vpol);
#endif
}

static void ui_config_read(LinphoneCore *lc)
{
	LinphoneFriend *lf;
	int i;
	for (i=0;(lf=linphone_friend_new_from_config_file(lc,i))!=NULL;i++){
		linphone_core_add_friend(lc,lf);
		linphone_friend_unref(lf);
	}
	call_logs_read_from_config_file(lc);
}

/*
static void autoreplier_config_init(LinphoneCore *lc)
{
	autoreplier_config_t *config=&lc->autoreplier_conf;
	config->enabled=lp_config_get_int(lc->config,"autoreplier","enabled",0);
	config->after_seconds=lp_config_get_int(lc->config,"autoreplier","after_seconds",6);
	config->max_users=lp_config_get_int(lc->config,"autoreplier","max_users",1);
	config->max_rec_time=lp_config_get_int(lc->config,"autoreplier","max_rec_time",60);
	config->max_rec_msg=lp_config_get_int(lc->config,"autoreplier","max_rec_msg",10);
	config->message=lp_config_get_string(lc->config,"autoreplier","message",NULL);
}
*/

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
	return lp_config_get_int(lc->config,"net","adaptive_rate_control",TRUE);
}

void linphone_core_set_adaptive_rate_algorithm(LinphoneCore *lc, const char* algorithm){
	lp_config_set_string(lc->config,"net","adaptive_rate_algorithm",algorithm);
}

const char * linphone_core_get_adaptive_rate_algorithm(const LinphoneCore *lc){
	return lp_config_get_string(lc->config, "net", "adaptive_rate_algorithm", "Simple");
}

bool_t linphone_core_rtcp_enabled(const LinphoneCore *lc){
	return lp_config_get_int(lc->config,"rtp","rtcp_enabled",TRUE);
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

void linphone_core_set_sip_transport_timeout(LinphoneCore *lc, int timeout_ms) {
	sal_set_transport_timeout(lc->sal, timeout_ms);
	if (linphone_core_ready(lc))
		lp_config_set_int(lc->config, "sip", "transport_timeout", timeout_ms);
}

int linphone_core_get_sip_transport_timeout(LinphoneCore *lc) {
	return sal_get_transport_timeout(lc->sal);
}

void linphone_core_enable_dns_srv(LinphoneCore *lc, bool_t enable) {
	sal_enable_dns_srv(lc->sal, enable);
	if (linphone_core_ready(lc))
		lp_config_set_int(lc->config, "net", "dns_srv_enabled", enable ? 1 : 0);
}

bool_t linphone_core_dns_srv_enabled(const LinphoneCore *lc) {
	return sal_dns_srv_enabled(lc->sal);
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
	MSList **codec_list=const_pt->type==PAYLOAD_VIDEO ? &lc->default_video_codecs : &lc->default_audio_codecs;
	if (linphone_core_codec_supported(lc, (const_pt->type == PAYLOAD_VIDEO) ? SalVideo : SalAudio, const_pt->mime_type)){
		PayloadType *pt=payload_type_clone(const_pt);
		int number=-1;
		payload_type_set_enable(pt,enabled);
		if (recv_fmtp!=NULL) payload_type_set_recv_fmtp(pt,recv_fmtp);
		/*Set a number to the payload type from the statically defined (RFC3551) profile, if not static, -1 is returned
		 and the payload type number will be determined dynamically later, at call time.*/
		payload_type_set_number(pt,
			(number=rtp_profile_find_payload_number(&av_profile, pt->mime_type, pt->clock_rate, pt->channels))
		);
		ms_message("Codec %s/%i fmtp=[%s] number=%i, enabled=%i) added to default capabilities.", pt->mime_type, pt->clock_rate,
			   pt->recv_fmtp ? pt->recv_fmtp : "", number, (int)payload_type_enabled(pt));
		*codec_list=ms_list_append(*codec_list,pt);
	}
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
				pt->mime_type, pt->clock_rate, pt->type!=PAYLOAD_VIDEO ? pt->channels : LINPHONE_FIND_PAYLOAD_IGNORE_CHANNELS,
				pt->type==PAYLOAD_VIDEO ? lc->default_video_codecs : lc->default_audio_codecs)==NULL){
				linphone_core_register_payload_type(lc,pt,NULL,FALSE);
			}
		}
	}
}

static void linphone_core_free_payload_types(LinphoneCore *lc){
	ms_list_free_with_data(lc->default_audio_codecs, (void (*)(void*))payload_type_destroy);
	ms_list_free_with_data(lc->default_video_codecs, (void (*)(void*))payload_type_destroy);
}

void linphone_core_set_state(LinphoneCore *lc, LinphoneGlobalState gstate, const char *message){
	lc->state=gstate;
	linphone_core_notify_global_state_changed(lc,gstate,message);
}

static void misc_config_read(LinphoneCore *lc) {
	LpConfig *config=lc->config;
	const char *uuid;

	lc->max_call_logs=lp_config_get_int(config,"misc","history_max_size",30);
	lc->max_calls=lp_config_get_int(config,"misc","max_calls",NB_MAX_CALLS);

	uuid=lp_config_get_string(config,"misc","uuid",NULL);
	if (!uuid){
		char tmp[64];
		sal_create_uuid(lc->sal,tmp,sizeof(tmp));
		lp_config_set_string(config,"misc","uuid",tmp);
	}else if (strcmp(uuid,"0")!=0) /*to allow to disable sip.instance*/
		sal_set_uuid(lc->sal, uuid);

	lc->user_certificates_path=ms_strdup(lp_config_get_string(config,"misc","user_certificates_path","."));
}

static void linphone_core_start(LinphoneCore * lc) {
	sip_setup_register_all();
	sound_config_read(lc);
	net_config_read(lc);
	rtp_config_read(lc);
	codecs_config_read(lc);
	sip_config_read(lc);
	video_config_read(lc);
	//autoreplier_config_init(&lc->autoreplier_conf);
	lc->presence_model=linphone_presence_model_new_with_activity(LinphonePresenceActivityOnline, NULL);
	misc_config_read(lc);
	ui_config_read(lc);
#ifdef TUNNEL_ENABLED
	if (lc->tunnel) {
		linphone_tunnel_configure(lc->tunnel);
	}
#endif

	linphone_core_notify_display_status(lc,_("Ready"));
	lc->auto_net_state_mon=lc->sip_conf.auto_net_state_mon;
	linphone_core_set_state(lc,LinphoneGlobalOn,"Ready");
}

void linphone_configuring_terminated(LinphoneCore *lc, LinphoneConfiguringState state, const char *message) {
	linphone_core_notify_configuring_status(lc, state, message);

	if (state == LinphoneConfiguringSuccessful) {
		if (linphone_core_is_provisioning_transient(lc) == TRUE)
			linphone_core_set_provisioning_uri(lc, NULL);
	}

	linphone_core_start(lc);
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
	if (ms_get_cpu_count()==1) opus_enabled=FALSE;
#endif
	linphone_core_register_payload_type(lc,&payload_type_opus,"useinbandfec=1",opus_enabled);
	linphone_core_register_payload_type(lc,&payload_type_silk_wb,NULL,TRUE);
	linphone_core_register_payload_type(lc,&payload_type_speex_wb,"vbr=on",TRUE);
	linphone_core_register_payload_type(lc,&payload_type_speex_nb,"vbr=on",TRUE);
	linphone_core_register_payload_type(lc,&payload_type_pcmu8000,NULL,TRUE);
	linphone_core_register_payload_type(lc,&payload_type_pcma8000,NULL,TRUE);

	/*other audio codecs, not enabled by default, in order of preference*/
	linphone_core_register_payload_type(lc,&payload_type_gsm,NULL,FALSE);
	linphone_core_register_payload_type(lc,&payload_type_g722,NULL,FALSE);
	linphone_core_register_payload_type(lc,&payload_type_ilbc,"mode=30",FALSE);
	linphone_core_register_payload_type(lc,&payload_type_amr,"octet-align=1",FALSE);
	linphone_core_register_payload_type(lc,&payload_type_amrwb,"octet-align=1",FALSE);
	linphone_core_register_payload_type(lc,&payload_type_g729,"annexb=no",FALSE);
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



#ifdef VIDEO_ENABLED
	/*default enabled video codecs, in order of preference*/
	linphone_core_register_payload_type(lc,&payload_type_vp8,NULL,TRUE);
	linphone_core_register_payload_type(lc,&payload_type_h264,"profile-level-id=42801F",TRUE);
	linphone_core_register_payload_type(lc,&payload_type_mp4v,"profile-level-id=3",TRUE);
	linphone_core_register_payload_type(lc,&payload_type_h263_1998,"CIF=1;QCIF=1",FALSE);
	linphone_core_register_payload_type(lc,&payload_type_h263,NULL,FALSE);
#endif
	/*register all static payload types declared in av_profile of oRTP, if not already declared above*/
	linphone_core_register_static_payloads(lc);
}

static void linphone_core_init(LinphoneCore * lc, const LinphoneCoreVTable *vtable, LpConfig *config, void * userdata){
	const char *remote_provisioning_uri = NULL;
	LinphoneCoreVTable* local_vtable= linphone_core_v_table_new();
	ms_message("Initializing LinphoneCore %s", linphone_core_get_version());

	lc->config=lp_config_ref(config);
	lc->data=userdata;
	lc->ringstream_autorelease=TRUE;


	memcpy(local_vtable,vtable,sizeof(LinphoneCoreVTable));
	_linphone_core_add_listener(lc, local_vtable, TRUE);

	linphone_core_set_state(lc,LinphoneGlobalStartup,"Starting up");
	ortp_init();
	linphone_core_activate_log_serialization_if_needed();

	ms_init();

	linphone_core_register_default_codecs(lc);
	/* Get the mediastreamer2 event queue */
	/* This allows to run event's callback in linphone_core_iterate() */
	lc->msevq=ms_factory_get_event_queue(ms_factory_get_fallback());

	lc->sal=sal_init();

	sal_set_user_pointer(lc->sal,lc);
	sal_set_callbacks(lc->sal,&linphone_sal_callbacks);

#ifdef TUNNEL_ENABLED
	lc->tunnel=linphone_core_tunnel_new(lc);
#endif

	lc->network_last_check = 0;
	lc->network_last_status = FALSE;

	lc->http_provider = belle_sip_stack_create_http_provider(sal_get_belle_sip_stack(lc->sal), "0.0.0.0");
	lc->http_verify_policy = belle_tls_verify_policy_new();
	belle_http_provider_set_tls_verify_policy(lc->http_provider,lc->http_verify_policy);

	certificates_config_read(lc);

	remote_provisioning_uri = linphone_core_get_provisioning_uri(lc);
	if (remote_provisioning_uri == NULL) {
		linphone_configuring_terminated(lc, LinphoneConfiguringSkipped, NULL);
	} // else linphone_core_start will be called after the remote provisioning (see linphone_core_iterate)
}

LinphoneCore *linphone_core_new(const LinphoneCoreVTable *vtable,
						const char *config_path, const char *factory_config_path, void * userdata)
{
	LinphoneCore *lc;
	LpConfig *config = lp_config_new_with_factory(config_path, factory_config_path);
	lc=linphone_core_new_with_config(vtable, config, userdata);
	lp_config_unref(config);
	return lc;
}

LinphoneCore *linphone_core_new_with_config(const LinphoneCoreVTable *vtable, struct _LpConfig *config, void *userdata)
{
	LinphoneCore *core = ms_new0(LinphoneCore, 1);
	linphone_core_init(core, vtable, config, userdata);
	return core;
}

const MSList *linphone_core_get_audio_codecs(const LinphoneCore *lc)
{
	return lc->codecs_conf.audio_codecs;
}

const MSList *linphone_core_get_video_codecs(const LinphoneCore *lc)
{
	return lc->codecs_conf.video_codecs;
}

int linphone_core_set_primary_contact(LinphoneCore *lc, const char *contact)
{
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
	linphone_address_destroy(ctt);
	return 0;
}


static void update_primary_contact(LinphoneCore *lc){
	char *guessed=NULL;
	char tmp[LINPHONE_IPADDR_SIZE];

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
	linphone_address_set_port(url,linphone_core_get_sip_port(lc));
	guessed=linphone_address_as_string(url);
	lc->sip_conf.guessed_contact=guessed;
	linphone_address_destroy(url);
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

void linphone_core_enable_lime(LinphoneCore *lc, bool_t val){
	if (linphone_core_ready(lc)){
		lp_config_set_int(lc->config,"sip","lime",val);
	}
}

bool_t linphone_core_lime_enabled(const LinphoneCore *lc){
	return (lp_config_get_int(lc->config,"sip", "lime", FALSE) && lime_is_available());
}

bool_t linphone_core_lime_for_file_sharing_enabled(const LinphoneCore *lc){
	return linphone_core_lime_enabled(lc) && (lp_config_get_int(lc->config,"sip", "lime_for_file_sharing", TRUE) && lime_is_available());
}

LinphoneAddress *linphone_core_get_primary_contact_parsed(LinphoneCore *lc){
	return linphone_address_new(linphone_core_get_primary_contact(lc));
}

/**
 * Sets the list of audio codecs.
 * @param[in] lc The LinphoneCore object
 * @param[in] codecs \mslist{PayloadType}
 * @return 0
 *
 * @ingroup media_parameters
 * The list is taken by the LinphoneCore thus the application should not free it.
 * This list is made of struct PayloadType describing the codec parameters.
**/
int linphone_core_set_audio_codecs(LinphoneCore *lc, MSList *codecs){
	if (lc->codecs_conf.audio_codecs!=NULL) ms_list_free(lc->codecs_conf.audio_codecs);
	lc->codecs_conf.audio_codecs=codecs;
	_linphone_core_codec_config_write(lc);
	linphone_core_update_allocated_audio_bandwidth(lc);
	return 0;
}

/**
 * Sets the list of video codecs.
 * @param[in] lc The LinphoneCore object
 * @param[in] codecs \mslist{PayloadType}
 * @return 0
 *
 * @ingroup media_parameters
 * The list is taken by the LinphoneCore thus the application should not free it.
 * This list is made of struct PayloadType describing the codec parameters.
**/
int linphone_core_set_video_codecs(LinphoneCore *lc, MSList *codecs){
	if (lc->codecs_conf.video_codecs!=NULL) ms_list_free(lc->codecs_conf.video_codecs);
	lc->codecs_conf.video_codecs=codecs;
	_linphone_core_codec_config_write(lc);
	return 0;
}

/**
 * Enable RFC3389 generic confort noise algorithm (CN payload type).
 * It is disabled by default, because this algorithm is only relevant for legacy codecs (PCMU, PCMA, G722).
 * @param lc the LinphoneCore
 * @param enabled TRUE if enabled, FALSE otherwise.
**/
void linphone_core_enable_generic_confort_noise(LinphoneCore *lc, bool_t enabled){
	lp_config_set_int(lc->config, "misc", "use_cn", enabled);
}

/**
 * Returns enablement of RFC3389 generic confort noise algorithm.
 * @param lc the LinphoneCore
 * @return TRUE or FALSE.
**/
bool_t linphone_core_generic_confort_noise_enabled(const LinphoneCore *lc){
	return lp_config_get_int(lc->config, "misc", "use_cn", FALSE);
}

const MSList * linphone_core_get_friend_list(const LinphoneCore *lc)
{
	return lc->friends;
}

void linphone_core_enable_audio_adaptive_jittcomp(LinphoneCore* lc, bool_t val)
{
	lc->rtp_conf.audio_adaptive_jitt_comp_enabled = val;
}

bool_t linphone_core_audio_adaptive_jittcomp_enabled(LinphoneCore* lc)
{
	return lc->rtp_conf.audio_adaptive_jitt_comp_enabled;
}

/**
 * Returns the nominal audio jitter buffer size in milliseconds.
 *
 * @ingroup media_parameters
**/
int linphone_core_get_audio_jittcomp(LinphoneCore *lc)
{
	return lc->rtp_conf.audio_jitt_comp;
}

void linphone_core_enable_video_adaptive_jittcomp(LinphoneCore* lc, bool_t val)
{
	lc->rtp_conf.video_adaptive_jitt_comp_enabled = val;
}

bool_t linphone_core_video_adaptive_jittcomp_enabled(LinphoneCore* lc)
{
	return lc->rtp_conf.video_adaptive_jitt_comp_enabled;
}

/**
 * Returns the nominal video jitter buffer size in milliseconds.
 *
 * @ingroup media_parameters
**/
int linphone_core_get_video_jittcomp(LinphoneCore *lc)
{
	return lc->rtp_conf.video_jitt_comp;
}

/**
 * Returns the UDP port used for audio streaming.
 *
 * @ingroup network_parameters
**/
int linphone_core_get_audio_port(const LinphoneCore *lc)
{
	return lc->rtp_conf.audio_rtp_min_port;
}

/**
 * Get the audio port range from which is randomly chosen the UDP port used for audio streaming.
 *
 * @ingroup network_parameters
 */
void linphone_core_get_audio_port_range(const LinphoneCore *lc, int *min_port, int *max_port)
{
	*min_port = lc->rtp_conf.audio_rtp_min_port;
	*max_port = lc->rtp_conf.audio_rtp_max_port;
}

/**
 * Returns the UDP port used for video streaming.
 *
 * @ingroup network_parameters
**/
int linphone_core_get_video_port(const LinphoneCore *lc){
	return lc->rtp_conf.video_rtp_min_port;
}

/**
 * Get the video port range from which is randomly chosen the UDP port used for video streaming.
 *
 * @ingroup network_parameters
 */
void linphone_core_get_video_port_range(const LinphoneCore *lc, int *min_port, int *max_port)
{
	*min_port = lc->rtp_conf.video_rtp_min_port;
	*max_port = lc->rtp_conf.video_rtp_max_port;
}


/**
 * Returns the value in seconds of the no-rtp timeout.
 *
 * @ingroup media_parameters
 * When no RTP or RTCP packets have been received for a while
 * LinphoneCore will consider the call is broken (remote end crashed or
 * disconnected from the network), and thus will terminate the call.
 * The no-rtp timeout is the duration above which the call is considered broken.
**/
int linphone_core_get_nortp_timeout(const LinphoneCore *lc){
	return lc->rtp_conf.nortp_timeout;
}

bool_t linphone_core_get_rtp_no_xmit_on_audio_mute(const LinphoneCore *lc){
	return lc->rtp_conf.rtp_no_xmit_on_audio_mute;
}

static void apply_jitter_value(LinphoneCore *lc, int value, MSFormatType stype){
	LinphoneCall *call;
	MSList *it;
	for (it=lc->calls;it!=NULL;it=it->next){
		MediaStream *ms;
		call=(LinphoneCall*)it->data;
		ms = stype==MSAudio ? (MediaStream*)call->audiostream : (MediaStream*)call->videostream;
		if (ms){
			RtpSession *s=ms->sessions.rtp_session;
			if (s){
				if (value>0){
					ms_message("Jitter buffer size set to [%i] ms on call [%p]",value,call);
					rtp_session_set_jitter_compensation(s,value);
					rtp_session_enable_jitter_buffer(s,TRUE);
				}else if (value==0){
					ms_warning("Jitter buffer is disabled per application request on call [%p]",call);
					rtp_session_enable_jitter_buffer(s,FALSE);
				}
			}
		}
	}
}

void linphone_core_set_audio_jittcomp(LinphoneCore *lc, int milliseconds)
{
	lc->rtp_conf.audio_jitt_comp=milliseconds;
	apply_jitter_value(lc, milliseconds, MSAudio);
}

void linphone_core_set_video_jittcomp(LinphoneCore *lc, int milliseconds)
{
	lc->rtp_conf.video_jitt_comp=milliseconds;
	apply_jitter_value(lc, milliseconds, MSVideo);
}

void linphone_core_set_rtp_no_xmit_on_audio_mute(LinphoneCore *lc,bool_t rtp_no_xmit_on_audio_mute){
	lc->rtp_conf.rtp_no_xmit_on_audio_mute=rtp_no_xmit_on_audio_mute;
}

/**
 * Sets the UDP port used for audio streaming.
 * A value if -1 will request the system to allocate the local port randomly.
 * This is recommended in order to avoid firewall warnings.
 *
 * @ingroup network_parameters
**/
void linphone_core_set_audio_port(LinphoneCore *lc, int port)
{
	lc->rtp_conf.audio_rtp_min_port=lc->rtp_conf.audio_rtp_max_port=port;
}

/**
 * Sets the UDP port range from which to randomly select the port used for audio streaming.
 * @ingroup media_parameters
 */
void linphone_core_set_audio_port_range(LinphoneCore *lc, int min_port, int max_port)
{
	lc->rtp_conf.audio_rtp_min_port=min_port;
	lc->rtp_conf.audio_rtp_max_port=max_port;
}

/**
 * Sets the UDP port used for video streaming.
 * A value if -1 will request the system to allocate the local port randomly.
 * This is recommended in order to avoid firewall warnings.
 *
 * @ingroup network_parameters
**/
void linphone_core_set_video_port(LinphoneCore *lc, int port){
	lc->rtp_conf.video_rtp_min_port=lc->rtp_conf.video_rtp_max_port=port;
}

/**
 * Sets the UDP port range from which to randomly select the port used for video streaming.
 * @ingroup media_parameters
 */
void linphone_core_set_video_port_range(LinphoneCore *lc, int min_port, int max_port)
{
	lc->rtp_conf.video_rtp_min_port=min_port;
	lc->rtp_conf.video_rtp_max_port=max_port;
}

/**
 * Sets the no-rtp timeout value in seconds.
 *
 * @ingroup media_parameters
 * See linphone_core_get_nortp_timeout() for details.
**/
void linphone_core_set_nortp_timeout(LinphoneCore *lc, int nortp_timeout){
	lc->rtp_conf.nortp_timeout=nortp_timeout;
}

/**
 * Indicates whether SIP INFO is used for sending digits.
 *
 * @ingroup media_parameters
**/
bool_t linphone_core_get_use_info_for_dtmf(LinphoneCore *lc)
{
	return lp_config_get_int(lc->config, "sip", "use_info", 0);
}

/**
 * Sets whether SIP INFO is to be used for sending digits.
 *
 * @ingroup media_parameters
**/
void linphone_core_set_use_info_for_dtmf(LinphoneCore *lc,bool_t use_info)
{
	if (linphone_core_ready(lc)) {
		lp_config_set_int(lc->config, "sip", "use_info", use_info);
	}
}

/**
 * Indicates whether RFC2833 is used for sending digits.
 *
 * @ingroup media_parameters
**/
bool_t linphone_core_get_use_rfc2833_for_dtmf(LinphoneCore *lc)
{
	return lp_config_get_int(lc->config, "sip", "use_rfc2833", 1);
}

/**
 * Sets whether RFC2833 is to be used for sending digits.
 *
 * @ingroup media_parameters
**/
void linphone_core_set_use_rfc2833_for_dtmf(LinphoneCore *lc,bool_t use_rfc2833)
{
	if (linphone_core_ready(lc)) {
		lp_config_set_int(lc->config, "sip", "use_rfc2833", use_rfc2833);
	}
}

/**
 * Returns the UDP port used by SIP.
 *
 * @deprecated use linphone_core_get_sip_transports() instead.
 * @ingroup network_parameters
**/
int linphone_core_get_sip_port(LinphoneCore *lc){
	LCSipTransports tr;
	linphone_core_get_sip_transports_used(lc,&tr);
	return tr.udp_port>0 ? tr.udp_port : (tr.tcp_port > 0 ? tr.tcp_port : tr.tls_port);
}

static char _ua_name[64]="Linphone";
static char _ua_version[64]=LIBLINPHONE_VERSION;

/**
 * Sets the user agent string used in SIP messages.
 *
 * @ingroup misc
**/
void linphone_core_set_user_agent(LinphoneCore *lc, const char *name, const char *ver){
	char ua_string[256];
	snprintf(ua_string, sizeof(ua_string) - 1, "%s/%s", name?name:"", ver?ver:"");
	if (lc->sal) {
		sal_set_user_agent(lc->sal, ua_string);
		sal_append_stack_string_to_user_agent(lc->sal);
	}
}
const char *linphone_core_get_user_agent(LinphoneCore *lc){
	return sal_get_user_agent(lc->sal);
}

const char *linphone_core_get_user_agent_name(void){
	return _ua_name;
}

const char *linphone_core_get_user_agent_version(void){
	return _ua_version;
}

static void transport_error(LinphoneCore *lc, const char* transport, int port){
	char *msg=ortp_strdup_printf("Could not start %s transport on port %i, maybe this port is already used.",transport,port);
	ms_warning("%s",msg);
	linphone_core_notify_display_warning(lc,msg);
	ms_free(msg);
}

static bool_t transports_unchanged(const LCSipTransports * tr1, const LCSipTransports * tr2){
	return
		tr2->udp_port==tr1->udp_port &&
		tr2->tcp_port==tr1->tcp_port &&
		tr2->dtls_port==tr1->dtls_port &&
		tr2->tls_port==tr1->tls_port;
}

int _linphone_core_apply_transports(LinphoneCore *lc){
	Sal *sal=lc->sal;
	const char *anyaddr;
	LCSipTransports *tr=&lc->sip_conf.transports;
	const char* listening_address;
	/*first of all invalidate all current registrations so that we can register again with new transports*/
	__linphone_core_invalidate_registers(lc);

	if (lc->sip_conf.ipv6_enabled)
		anyaddr="::0";
	else
		anyaddr="0.0.0.0";

	sal_unlisten_ports(sal);

	listening_address = lp_config_get_string(lc->config,"sip","bind_address",anyaddr);

	if (lc->tunnel && linphone_tunnel_sip_enabled(lc->tunnel) && linphone_tunnel_get_activated(lc->tunnel)){
		if (sal_listen_port(sal,anyaddr,tr->udp_port,SalTransportUDP,TRUE)!=0){
			transport_error(lc,"udp+tunnel",tr->udp_port);
		}
	}else{
		if (tr->udp_port!=0){
			if (sal_listen_port(sal,listening_address,tr->udp_port,SalTransportUDP,FALSE)!=0){
				transport_error(lc,"udp",tr->udp_port);
			}
		}
		if (tr->tcp_port!=0){
			if (sal_listen_port (sal,listening_address,tr->tcp_port,SalTransportTCP,FALSE)!=0){
				transport_error(lc,"tcp",tr->tcp_port);
			}
		}
		if (linphone_core_sip_transport_supported(lc,LinphoneTransportTls)){
			if (tr->tls_port!=0){
				if (sal_listen_port (sal,listening_address,tr->tls_port,SalTransportTLS,FALSE)!=0){
					transport_error(lc,"tls",tr->tls_port);
				}
			}
		}
	}
	return 0;
}

/**
 * Returns TRUE if given transport type is supported by the library, FALSE otherwise.
**/
bool_t linphone_core_sip_transport_supported(const LinphoneCore *lc, LinphoneTransportType tp){
	return sal_transport_available(lc->sal,(SalTransport)tp);
}

/**
 * Sets the ports to be used for each of transport (UDP or TCP)
 *
 * A zero value port for a given transport means the transport
 * is not used. A value of LC_SIP_TRANSPORT_RANDOM (-1) means the port is to be choosen randomly by the system.
 *
 * @ingroup network_parameters
**/
int linphone_core_set_sip_transports(LinphoneCore *lc, const LCSipTransports * tr_config /*config to be saved*/){
	LCSipTransports tr=*tr_config;

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

/**
 * Retrieves the port configuration used for each transport (udp, tcp, tls).
 * A zero value port for a given transport means the transport
 * is not used. A value of LC_SIP_TRANSPORT_RANDOM (-1) means the port is to be chosen randomly by the system.
 * @ingroup network_parameters
**/
int linphone_core_get_sip_transports(LinphoneCore *lc, LCSipTransports *tr){
	memcpy(tr,&lc->sip_conf.transports,sizeof(*tr));
	return 0;
}

/**
 * Retrieves the real port number assigned for each sip transport (udp, tcp, tls).
 * A zero value means that the transport is not activated.
 * If LC_SIP_TRANSPORT_RANDOM was passed to linphone_core_set_sip_transports(), the random port choosed by the system is returned.
 * @ingroup network_parameters
 * @param lc the LinphoneCore
 * @param tr a LCSipTransports structure.
**/
void linphone_core_get_sip_transports_used(LinphoneCore *lc, LCSipTransports *tr){
	tr->udp_port=sal_get_listening_port(lc->sal,SalTransportUDP);
	tr->tcp_port=sal_get_listening_port(lc->sal,SalTransportTCP);
	tr->tls_port=sal_get_listening_port(lc->sal,SalTransportTLS);
}
/**
 * Sets the UDP port to be used by SIP.
 *
 * @deprecated use linphone_core_set_sip_transports() instead.
 * @ingroup network_parameters
**/
void linphone_core_set_sip_port(LinphoneCore *lc,int port)
{
	LCSipTransports tr;
	memset(&tr,0,sizeof(tr));
	tr.udp_port=port;
	linphone_core_set_sip_transports (lc,&tr);
}

/**
 * Returns TRUE if IPv6 is enabled.
 *
 * @ingroup network_parameters
 * See linphone_core_enable_ipv6() for more details on how IPv6 is supported in liblinphone.
**/
bool_t linphone_core_ipv6_enabled(LinphoneCore *lc){
	return lc->sip_conf.ipv6_enabled;
}

/**
 * Turns IPv6 support on or off.
 *
 * @ingroup network_parameters
 *
 * @note IPv6 support is exclusive with IPv4 in liblinphone:
 * when IPv6 is turned on, IPv4 calls won't be possible anymore.
 * By default IPv6 support is off.
**/
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


static void monitor_network_state(LinphoneCore *lc, time_t curtime){
	bool_t new_status=lc->network_last_status;
	char newip[LINPHONE_IPADDR_SIZE];

	/* only do the network up checking every five seconds */
	if (lc->network_last_check==0 || (curtime-lc->network_last_check)>=5){
		linphone_core_get_local_ip(lc,AF_UNSPEC,NULL,newip);
		if (strcmp(newip,"::1")!=0 && strcmp(newip,"127.0.0.1")!=0){
			new_status=TRUE;
		}else new_status=FALSE; /*no network*/

		if (new_status==lc->network_last_status && new_status==TRUE && strcmp(newip,lc->localip)!=0){
			/*IP address change detected*/
			ms_message("IP address change detected.");
			set_network_reachable(lc,FALSE,curtime);
			lc->network_last_status=FALSE;
		}
		strncpy(lc->localip,newip,sizeof(lc->localip));

		if (new_status!=lc->network_last_status) {
			if (new_status){
				ms_message("New local ip address is %s",lc->localip);
			}
			set_network_reachable(lc,new_status, curtime);
			lc->network_last_status=new_status;
		}
		lc->network_last_check=curtime;
	}
}

static void proxy_update(LinphoneCore *lc){
	MSList *elem,*next;
	ms_list_for_each(lc->sip_conf.proxies,(void (*)(void*))&linphone_proxy_config_update);
	for(elem=lc->sip_conf.deleted_proxies;elem!=NULL;elem=next){
		LinphoneProxyConfig* cfg = (LinphoneProxyConfig*)elem->data;
		next=elem->next;
		if (ms_time(NULL) - cfg->deletion_date > 32) {
			lc->sip_conf.deleted_proxies =ms_list_remove_link(lc->sip_conf.deleted_proxies,elem);
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
	MSList *elem;
	SipSetupContext *ctx=linphone_proxy_config_get_sip_setup_context(cfg);
	for (elem=lc->bl_reqs;elem!=NULL;elem=ms_list_next(elem)){
		BuddyLookupRequest *req=(BuddyLookupRequest *)elem->data;
		if (req->status==BuddyLookupDone || req->status==BuddyLookupFailure){
			if (req->results!=NULL){
				BuddyInfo *i=(BuddyInfo*)req->results->data;
				ms_list_free(req->results);
				req->results=NULL;
				assign_buddy_info(lc,i);
			}
			sip_setup_context_buddy_lookup_free(ctx,req);
			elem->data=NULL;
		}
	}
	/*purge completed requests */
	while((elem=ms_list_find(lc->bl_reqs,NULL))!=NULL){
		lc->bl_reqs=ms_list_remove_link(lc->bl_reqs,elem);
	}
}

static void linphone_core_grab_buddy_infos(LinphoneCore *lc, LinphoneProxyConfig *cfg){
	const MSList *elem;
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
					lc->bl_reqs=ms_list_append(lc->bl_reqs,req);
					ms_free(tmp);
				}
			}
		}
	}
}

static void linphone_core_do_plugin_tasks(LinphoneCore *lc){
	LinphoneProxyConfig *cfg=NULL;
	linphone_core_get_default_proxy(lc,&cfg);
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

/**
 * Main loop function. It is crucial that your application call it periodically.
 *
 * @ingroup initializing
 * linphone_core_iterate() performs various backgrounds tasks:
 * - receiving of SIP messages
 * - handles timers and timeout
 * - performs registration to proxies
 * - authentication retries
 * The application MUST call this function periodically, in its main loop.
 * Be careful that this function must be called from the same thread as
 * other liblinphone methods. If it is not the case make sure all liblinphone calls are
 * serialized with a mutex.
**/
void linphone_core_iterate(LinphoneCore *lc){
	MSList *calls;
	LinphoneCall *call;
	time_t curtime=time(NULL);
	int elapsed;
	bool_t one_second_elapsed=FALSE;
	const char *remote_provisioning_uri = NULL;
	if (lc->network_reachable_to_be_notified) {
		lc->network_reachable_to_be_notified=FALSE;
		linphone_core_notify_network_reachable(lc,lc->network_reachable);
	}
	if (linphone_core_get_global_state(lc) == LinphoneGlobalStartup) {
		if (sal_get_root_ca(lc->sal)) {
			belle_tls_verify_policy_t *tls_policy = belle_tls_verify_policy_new();
			belle_tls_verify_policy_set_root_ca(tls_policy, sal_get_root_ca(lc->sal));
			belle_http_provider_set_tls_verify_policy(lc->http_provider, tls_policy);
		}

		linphone_core_notify_display_status(lc, _("Configuring"));
		linphone_core_set_state(lc, LinphoneGlobalConfiguring, "Configuring");

		remote_provisioning_uri = linphone_core_get_provisioning_uri(lc);
		if (remote_provisioning_uri) {
			int err = linphone_remote_provisioning_download_and_apply(lc, remote_provisioning_uri);
			if (err == -1) {
				linphone_configuring_terminated(lc, LinphoneConfiguringFailed, "Bad URI");
			}
		} // else linphone_configuring_terminated has already been called in linphone_core_init
	}

	if (curtime-lc->prevtime>=1){
		lc->prevtime=curtime;
		one_second_elapsed=TRUE;
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
		ring_stop(lc->ringstream);
		lc->ringstream=NULL;
		lc_callback_obj_invoke(&lc->preview_finished_cb,lc);
	}

	if (lc->ringstream && lc->ringstream_autorelease && lc->dmfs_playing_start_time!=0
		&& (curtime-lc->dmfs_playing_start_time)>5){
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

	sal_iterate(lc->sal);
	if (lc->msevq) ms_event_queue_pump(lc->msevq);
	if (lc->auto_net_state_mon) monitor_network_state(lc,curtime);

	proxy_update(lc);

	//we have to iterate for each call
	calls= lc->calls;
	while(calls!= NULL){
		call = (LinphoneCall *)calls->data;
		elapsed = curtime-call->log->start_date_time;
		 /* get immediately a reference to next one in case the one
		 we are going to examine is destroy and removed during
		 linphone_core_start_invite() */
		calls=calls->next;
		linphone_call_background_tasks(call,one_second_elapsed);
		if (call->state==LinphoneCallOutgoingInit && (elapsed>=lc->sip_conf.delayed_timeout)){
			/*start the call even if the OPTIONS reply did not arrive*/
			if (call->ice_session != NULL) {
				ms_warning("ICE candidates gathering from [%s] has not finished yet, proceed with the call without ICE anyway."
						,linphone_core_get_stun_server(lc));
				linphone_call_delete_ice_session(call);
				linphone_call_stop_media_streams_for_ice_gathering(call);
			}
#ifdef BUILD_UPNP
			if (call->upnp_session != NULL) {
				ms_warning("uPnP mapping has not finished yet, proceeded with the call without uPnP anyway.");
				linphone_call_delete_upnp_session(call);
			}
#endif //BUILD_UPNP
			linphone_core_start_invite(lc,call, NULL);
		}
		if (call->state==LinphoneCallIncomingReceived || call->state==LinphoneCallIncomingEarlyMedia){
			if (one_second_elapsed) ms_message("incoming call ringing for %i seconds",elapsed);
			if (elapsed>lc->sip_conf.inc_timeout){
				LinphoneReason decline_reason;
				ms_message("incoming call timeout (%i)",lc->sip_conf.inc_timeout);
				decline_reason=lc->current_call ? LinphoneReasonBusy : LinphoneReasonDeclined;
				call->log->status=LinphoneCallMissed;
				sal_error_info_set(&call->non_op_error,SalReasonRequestTimeout,408,"Not answered",NULL);
				linphone_core_decline_call(lc,call,decline_reason);
			}
		}
		if ( (lc->sip_conf.in_call_timeout > 0)
			 && (call->log->connected_date_time != 0)
			 && ((curtime - call->log->connected_date_time) > lc->sip_conf.in_call_timeout))
		{
			ms_message("in call timeout (%i)",lc->sip_conf.in_call_timeout);
			linphone_core_terminate_call(lc,call);
		}
	}

	if (linphone_core_video_preview_enabled(lc)){
		if (lc->previewstream==NULL && lc->calls==NULL)
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

	if (lc->network_reachable && lc->netup_time!=0 && (curtime-lc->netup_time)>3){
		/*not do that immediately, take your time.*/
		linphone_core_send_initial_subscribes(lc);
	}

	if (one_second_elapsed) {
		if (lp_config_needs_commit(lc->config)) {
			lp_config_sync(lc->config);
		}
	}

	if (liblinphone_serialize_logs == TRUE) {
		ortp_logv_flush();
	}
}

LinphoneAddress * linphone_core_interpret_url(LinphoneCore *lc, const char *url){
	LinphoneProxyConfig *proxy = linphone_core_get_default_proxy_config(lc);
	return linphone_proxy_config_normalize_sip_uri(proxy, url);
}

/**
 * Returns the default identity SIP address.
 *
 * @ingroup proxies
 * This is an helper function:
 *
 * If no default proxy is set, this will return the primary contact (
 * see linphone_core_get_primary_contact() ). If a default proxy is set
 * it returns the registered identity on the proxy.
**/
const char * linphone_core_get_identity(LinphoneCore *lc){
	LinphoneProxyConfig *proxy=NULL;
	const char *from;
	linphone_core_get_default_proxy(lc,&proxy);
	if (proxy!=NULL) {
		from=linphone_proxy_config_get_identity(proxy);
	}else from=linphone_core_get_primary_contact(lc);
	return from;
}

const char * linphone_core_get_route(LinphoneCore *lc){
	LinphoneProxyConfig *proxy=NULL;
	const char *route=NULL;
	linphone_core_get_default_proxy(lc,&proxy);
	if (proxy!=NULL) {
		route=linphone_proxy_config_get_route(proxy);
	}
	return route;
}

/**
 * Start a new call as a consequence of a transfer request received from a call.
 * This function is for advanced usage: the execution of transfers is automatically managed by the LinphoneCore. However if an application
 * wants to have control over the call parameters for the new call, it should call this function immediately during the LinphoneCallRefered notification.
 * @see LinphoneCoreVTable::call_state_changed
 * @param lc the LinphoneCore
 * @param call a call that has just been notified about LinphoneCallRefered state event.
 * @param params the call parameters to be applied to the new call.
 * @return a LinphoneCall corresponding to the new call that is attempted to the transfer destination.
**/
LinphoneCall * linphone_core_start_refered_call(LinphoneCore *lc, LinphoneCall *call, const LinphoneCallParams *params){
	LinphoneCallParams *cp=params ? linphone_call_params_copy(params) : linphone_core_create_default_call_parameters(lc);
	LinphoneCall *newcall;

	if (call->state!=LinphoneCallPaused){
		ms_message("Automatically pausing current call to accept transfer.");
		_linphone_core_pause_call(lc,call);
		call->was_automatically_paused=TRUE;
	}

	if (!params){
		cp->has_video = call->current_params->has_video; /*start the call to refer-target with video enabled if original call had video*/
	}
	cp->referer=call;
	ms_message("Starting new call to refered address %s",call->refer_to);
	call->refer_pending=FALSE;
	newcall=linphone_core_invite_with_params(lc,call->refer_to,cp);
	linphone_call_params_destroy(cp);
	if (newcall) {
		call->transfer_target=linphone_call_ref(newcall);
		linphone_core_notify_refer_state(lc,call,newcall);
	}
	return newcall;
}

void linphone_core_notify_refer_state(LinphoneCore *lc, LinphoneCall *referer, LinphoneCall *newcall){
	if (referer->op!=NULL){
		sal_call_notify_refer_state(referer->op,newcall ? newcall->op : NULL);
	}
}

/* returns the ideal route set for making an operation through this proxy.
 * The list must be freed as well as the SalAddress content*/

	/*
* rfc3608
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

static MSList *make_routes_for_proxy(LinphoneProxyConfig *proxy, const LinphoneAddress *dest){
	MSList *ret=NULL;
	const char *local_route=linphone_proxy_config_get_route(proxy);
	const LinphoneAddress *srv_route=linphone_proxy_config_get_service_route(proxy);
	if (local_route){
		ret=ms_list_append(ret,sal_address_new(local_route));
	}
	if (srv_route){
		ret=ms_list_append(ret,sal_address_clone((SalAddress*)srv_route));
	}
	if (ret==NULL){
		/*if the proxy address matches the domain part of the destination, then use the same transport
		 * as the one used for registration. This is done by forcing a route to this proxy.*/
		SalAddress *proxy_addr=sal_address_new(linphone_proxy_config_get_addr(proxy));
		if (strcmp(sal_address_get_domain(proxy_addr),linphone_address_get_domain(dest))==0){
			ret=ms_list_append(ret,proxy_addr);
		}else sal_address_destroy(proxy_addr);
	}
	return ret;
}

LinphoneProxyConfig * linphone_core_lookup_known_proxy(LinphoneCore *lc, const LinphoneAddress *uri){
	const MSList *elem;
	LinphoneProxyConfig *found_cfg=NULL;
	LinphoneProxyConfig *found_reg_cfg=NULL;
	LinphoneProxyConfig *found_noreg_cfg=NULL;
	LinphoneProxyConfig *default_cfg=lc->default_proxy;

	if (linphone_address_get_domain(uri) == NULL) {
		ms_message("cannot seach for proxy for uri [%p] if no domain set. returning default",uri);
		return default_cfg;
	}
	/*return default proxy if it is matching the destination uri*/
	if (default_cfg){
		const char *domain=linphone_proxy_config_get_domain(default_cfg);
		if (strcmp(domain,linphone_address_get_domain(uri))==0){
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


int linphone_core_proceed_with_invite_if_ready(LinphoneCore *lc, LinphoneCall *call, LinphoneProxyConfig *dest_proxy){
	bool_t ice_ready = FALSE;
	bool_t upnp_ready = FALSE;
	bool_t ping_ready = FALSE;

	if (call->ice_session != NULL) {
		if (ice_session_candidates_gathered(call->ice_session)) ice_ready = TRUE;
	} else {
		ice_ready = TRUE;
	}
#ifdef BUILD_UPNP
	if (call->upnp_session != NULL) {
		if (linphone_upnp_session_get_state(call->upnp_session) == LinphoneUpnpStateOk) upnp_ready = TRUE;
	} else {
		upnp_ready = TRUE;
	}
#else
	upnp_ready=TRUE;
#endif //BUILD_UPNP
	if (call->ping_op != NULL) {
		if (call->ping_replied == TRUE) ping_ready = TRUE;
	} else {
		ping_ready = TRUE;
	}

	if ((ice_ready == TRUE) && (upnp_ready == TRUE) && (ping_ready == TRUE)) {
		return linphone_core_start_invite(lc, call, NULL);
	}
	return 0;
}

int linphone_core_restart_invite(LinphoneCore *lc, LinphoneCall *call){
	linphone_call_create_op(call);
	linphone_call_stop_media_streams(call);
	ms_media_stream_sessions_uninit(&call->sessions[0]);
	ms_media_stream_sessions_uninit(&call->sessions[1]);
	linphone_call_init_media_streams(call);
	return linphone_core_start_invite(lc,call, NULL);
}

int linphone_core_start_invite(LinphoneCore *lc, LinphoneCall *call, const LinphoneAddress* destination /* = NULL if to be taken from the call log */){
	int err;
	char *real_url,*barmsg;
	char *from;
	/*try to be best-effort in giving real local or routable contact address */
	linphone_call_set_contact_op(call);

	linphone_core_stop_dtmf_stream(lc);
	linphone_call_make_local_media_description(lc,call);

	if (lc->ringstream==NULL) {
		if (lc->sound_conf.play_sndcard && lc->sound_conf.capt_sndcard){
			/*give a chance a set card prefered sampling frequency*/
			if (call->localdesc->streams[0].max_rate>0) {
				ms_snd_card_set_preferred_sample_rate(lc->sound_conf.play_sndcard, call->localdesc->streams[0].max_rate);
			}
			if (!lc->use_files)
				audio_stream_prepare_sound(call->audiostream,lc->sound_conf.play_sndcard,lc->sound_conf.capt_sndcard);
		}
	}
	real_url=linphone_address_as_string( destination ? destination : call->log->to);
	from=linphone_address_as_string(call->log->from);

	if (!lc->sip_conf.sdp_200_ack){
		/*we are offering, set local media description before sending the call*/
		sal_call_set_local_media_description(call->op,call->localdesc);
	}
	err=sal_call(call->op,from,real_url);
	if (lc->sip_conf.sdp_200_ack){
		/*we are NOT offering, set local media description after sending the call so that we are ready to
		 process the remote offer when it will arrive*/
		sal_call_set_local_media_description(call->op,call->localdesc);
	}

	call->log->call_id=ms_strdup(sal_op_get_call_id(call->op)); /*must be known at that time*/

	barmsg=ortp_strdup_printf("%s %s", _("Contacting"), real_url);
	linphone_core_notify_display_status(lc,barmsg);
	ms_free(barmsg);

	if (err<0){
		linphone_core_notify_display_status(lc,_("Could not call"));
		linphone_call_stop_media_streams(call);
		linphone_call_set_state(call,LinphoneCallError,"Call failed");
	}else {
		linphone_call_set_state(call,LinphoneCallOutgoingProgress,"Outgoing call in progress");
	}
	ms_free(real_url);
	ms_free(from);
	return err;
}

/**
 * Initiates an outgoing call
 *
 * @ingroup call_control
 * @param lc the LinphoneCore object
 * @param url the destination of the call (sip address, or phone number).
 *
 * The application doesn't own a reference to the returned LinphoneCall object.
 * Use linphone_call_ref() to safely keep the LinphoneCall pointer valid within your application.
 *
 * @return a LinphoneCall object or NULL in case of failure
**/
LinphoneCall * linphone_core_invite(LinphoneCore *lc, const char *url){
	LinphoneCall *call;
	LinphoneCallParams *p=linphone_core_create_default_call_parameters (lc);
	p->has_video &= !!lc->video_policy.automatically_initiate;
	call=linphone_core_invite_with_params(lc,url,p);
	linphone_call_params_destroy(p);
	return call;
}


/**
 * Initiates an outgoing call according to supplied call parameters
 *
 * @ingroup call_control
 * @param lc the LinphoneCore object
 * @param url the destination of the call (sip address, or phone number).
 * @param p call parameters
 *
 * The application doesn't own a reference to the returned LinphoneCall object.
 * Use linphone_call_ref() to safely keep the LinphoneCall pointer valid within your application.
 *
 * @return a LinphoneCall object or NULL in case of failure
**/
LinphoneCall * linphone_core_invite_with_params(LinphoneCore *lc, const char *url, const LinphoneCallParams *p){
	LinphoneAddress *addr=linphone_core_interpret_url(lc,url);
	if (addr){
		LinphoneCall *call;
		call=linphone_core_invite_address_with_params(lc,addr,p);
		linphone_address_destroy(addr);
		return call;
	}
	return NULL;
}

/**
 * Initiates an outgoing call given a destination LinphoneAddress
 *
 * @ingroup call_control
 * @param lc the LinphoneCore object
 * @param addr the destination of the call (sip address).
 *
 * The LinphoneAddress can be constructed directly using linphone_address_new(), or
 * created by linphone_core_interpret_url().
 * The application doesn't own a reference to the returned LinphoneCall object.
 * Use linphone_call_ref() to safely keep the LinphoneCall pointer valid within your application.
 *
 * @return a LinphoneCall object or NULL in case of failure
**/
LinphoneCall * linphone_core_invite_address(LinphoneCore *lc, const LinphoneAddress *addr){
	LinphoneCall *call;
	LinphoneCallParams *p=linphone_core_create_default_call_parameters(lc);
	p->has_video &= !!lc->video_policy.automatically_initiate;
	call=linphone_core_invite_address_with_params (lc,addr,p);
	linphone_call_params_destroy(p);
	return call;
}

static void linphone_transfer_routes_to_op(MSList *routes, SalOp *op){
	MSList *it;
	for(it=routes;it!=NULL;it=it->next){
		SalAddress *addr=(SalAddress*)it->data;
		sal_op_add_route_address(op,addr);
		sal_address_destroy(addr);
	}
	ms_list_free(routes);
}

void linphone_configure_op(LinphoneCore *lc, SalOp *op, const LinphoneAddress *dest, SalCustomHeader *headers, bool_t with_contact){
	MSList *routes=NULL;
	LinphoneProxyConfig *proxy=linphone_core_lookup_known_proxy(lc,dest);
	const char *identity;
	if (proxy){
		identity=linphone_proxy_config_get_identity(proxy);
		if (linphone_proxy_config_get_privacy(proxy)!=LinphonePrivacyDefault) {
			sal_op_set_privacy(op,linphone_proxy_config_get_privacy(proxy));
		}
	}else identity=linphone_core_get_primary_contact(lc);
	/*sending out of calls*/
	if (proxy){
		routes=make_routes_for_proxy(proxy,dest);
		linphone_transfer_routes_to_op(routes,op);
	}
	sal_op_set_to_address(op,dest);
	sal_op_set_from(op,identity);
	sal_op_set_sent_custom_header(op,headers);
	sal_op_set_realm(op,linphone_proxy_config_get_realm(proxy));
	if (with_contact && proxy && proxy->op){
		const SalAddress *contact;
		if ((contact=sal_op_get_contact_address(proxy->op))){
			SalTransport tport=sal_address_get_transport((SalAddress*)contact);
			SalAddress *new_contact=sal_address_clone(contact);
			sal_address_clean(new_contact); /* clean out contact_params that come from proxy config*/
			sal_address_set_transport(new_contact,tport);
			sal_op_set_contact_address(op,new_contact);
			sal_address_destroy(new_contact);
		}
	}
	sal_op_cnx_ip_to_0000_if_sendonly_enable(op,lp_config_get_default_int(lc->config,"sip","cnx_ip_to_0000_if_sendonly_enabled",0)); /*also set in linphone_call_new_incoming*/
}

/**
 * Initiates an outgoing call given a destination LinphoneAddress
 *
 * @ingroup call_control
 * @param lc the LinphoneCore object
 * @param addr the destination of the call (sip address).
	@param params call parameters
 *
 * The LinphoneAddress can be constructed directly using linphone_address_new(), or
 * created by linphone_core_interpret_url().
 * The application doesn't own a reference to the returned LinphoneCall object.
 * Use linphone_call_ref() to safely keep the LinphoneCall pointer valid within your application.
 *
 * @return a LinphoneCall object or NULL in case of failure
**/
LinphoneCall * linphone_core_invite_address_with_params(LinphoneCore *lc, const LinphoneAddress *addr, const LinphoneCallParams *params)
{
	const char *from=NULL;
	LinphoneProxyConfig *proxy=NULL;
	LinphoneAddress *parsed_url2=NULL;
	char *real_url=NULL;
	LinphoneCall *call;
	bool_t defer = FALSE;
	LinphoneCallParams *cp = linphone_call_params_copy(params);

	linphone_core_preempt_sound_resources(lc);

	if(!linphone_core_can_we_add_call(lc)){
		linphone_core_notify_display_warning(lc,_("Sorry, we have reached the maximum number of simultaneous calls"));
		return NULL;
	}

	real_url=linphone_address_as_string(addr);
	proxy=linphone_core_lookup_known_proxy(lc,addr);

	if (proxy!=NULL) {
		from=linphone_proxy_config_get_identity(proxy);
		cp->avpf_enabled = linphone_proxy_config_avpf_enabled(proxy);
		cp->avpf_rr_interval = linphone_proxy_config_get_avpf_rr_interval(proxy) * 1000;
	}else{
		cp->avpf_enabled=linphone_core_get_avpf_mode(lc)==LinphoneAVPFEnabled;
		if (cp->avpf_enabled) cp->avpf_rr_interval=linphone_core_get_avpf_rr_interval(lc) * 1000;
	}

	/* if no proxy or no identity defined for this proxy, default to primary contact*/
	if (from==NULL) from=linphone_core_get_primary_contact(lc);

	parsed_url2=linphone_address_new(from);

	call=linphone_call_new_outgoing(lc,parsed_url2,linphone_address_clone(addr),cp,proxy);

	if(linphone_core_add_call(lc,call)!= 0)
	{
		ms_warning("we had a problem in adding the call into the invite ... weird");
		linphone_call_unref(call);
		linphone_call_params_destroy(cp);
		return NULL;
	}

	/* this call becomes now the current one*/
	lc->current_call=call;
	linphone_call_set_state (call,LinphoneCallOutgoingInit,"Starting outgoing call");
	call->log->start_date_time=ms_time(NULL);
	linphone_call_init_media_streams(call);

	if (linphone_core_get_firewall_policy(call->core) == LinphonePolicyUseIce) {
		/* Defer the start of the call after the ICE gathering process. */
		if (linphone_call_prepare_ice(call,FALSE)==1)
			defer=TRUE;
	}
	else if (linphone_core_get_firewall_policy(call->core) == LinphonePolicyUseUpnp) {
#ifdef BUILD_UPNP
		if (linphone_core_update_upnp(lc,call)<0) {
			/* uPnP port mappings failed, proceed with the call anyway. */
			linphone_call_delete_upnp_session(call);
		} else {
			defer = TRUE;
		}
#endif //BUILD_UPNP
	}

	if (call->dest_proxy==NULL && lc->sip_conf.ping_with_options==TRUE){
#ifdef BUILD_UPNP
		if (lc->upnp != NULL && linphone_core_get_firewall_policy(lc)==LinphonePolicyUseUpnp &&
			linphone_upnp_context_get_state(lc->upnp) == LinphoneUpnpStateOk) {
#else //BUILD_UPNP
		{
#endif //BUILD_UPNP
			/*defer the start of the call after the OPTIONS ping*/
			call->ping_replied=FALSE;
			call->ping_op=sal_op_new(lc->sal);
			sal_ping(call->ping_op,from,real_url);
			sal_op_set_user_pointer(call->ping_op,call);
			defer = TRUE;
		}
	}

	if (defer==FALSE) linphone_core_start_invite(lc,call,NULL);

	if (real_url!=NULL) ms_free(real_url);
	linphone_call_params_destroy(cp);
	return call;
}

/**
 * Performs a simple call transfer to the specified destination.
 *
 * @ingroup call_control
 * The remote endpoint is expected to issue a new call to the specified destination.
 * The current call remains active and thus can be later paused or terminated.
 *
 * It is possible to follow the progress of the transfer provided that transferee sends notification about it.
 * In this case, the transfer_state_changed callback of the #LinphoneCoreVTable is invoked to notify of the state of the new call at the other party.
 * The notified states are #LinphoneCallOutgoingInit , #LinphoneCallOutgoingProgress, #LinphoneCallOutgoingRinging and #LinphoneCallConnected.
**/
int linphone_core_transfer_call(LinphoneCore *lc, LinphoneCall *call, const char *url)
{
	char *real_url=NULL;
	LinphoneAddress *real_parsed_url=linphone_core_interpret_url(lc,url);

	if (!real_parsed_url){
		/* bad url */
		return -1;
	}
	if (call==NULL){
		ms_warning("No established call to refer.");
		return -1;
	}
	//lc->call=NULL; //Do not do that you will lose the call afterward . . .
	real_url=linphone_address_as_string (real_parsed_url);
	sal_call_refer(call->op,real_url);
	ms_free(real_url);
	linphone_address_destroy(real_parsed_url);
	linphone_call_set_transfer_state(call, LinphoneCallOutgoingInit);
	return 0;
}

/**
 * Transfer a call to destination of another running call. This is used for "attended transfer" scenarios.
 * @param lc linphone core object
 * @param call a running call you want to transfer
 * @param dest a running call whose remote person will receive the transfer
 *
 * @ingroup call_control
 *
 * The transfered call is supposed to be in paused state, so that it is able to accept the transfer immediately.
 * The destination call is a call previously established to introduce the transfered person.
 * This method will send a transfer request to the transfered person. The phone of the transfered is then
 * expected to automatically call to the destination of the transfer. The receiver of the transfer will then automatically
 * close the call with us (the 'dest' call).
 *
 * It is possible to follow the progress of the transfer provided that transferee sends notification about it.
 * In this case, the transfer_state_changed callback of the #LinphoneCoreVTable is invoked to notify of the state of the new call at the other party.
 * The notified states are #LinphoneCallOutgoingInit , #LinphoneCallOutgoingProgress, #LinphoneCallOutgoingRinging and #LinphoneCallConnected.
**/
int linphone_core_transfer_call_to_another(LinphoneCore *lc, LinphoneCall *call, LinphoneCall *dest){
	int result = sal_call_refer_with_replaces (call->op,dest->op);
	linphone_call_set_transfer_state(call, LinphoneCallOutgoingInit);
	return result;
}

bool_t linphone_core_is_incoming_invite_pending(LinphoneCore*lc){
	LinphoneCall *call = linphone_core_get_current_call(lc);
	if(call != NULL)
	{
		if(call->dir==LinphoneCallIncoming
			&& (call->state == LinphoneCallIncomingReceived || call->state ==  LinphoneCallIncomingEarlyMedia))
			return TRUE;
	}
	return FALSE;
}

bool_t linphone_core_incompatible_security(LinphoneCore *lc, SalMediaDescription *md){
	return linphone_core_is_media_encryption_mandatory(lc) && linphone_core_get_media_encryption(lc)==LinphoneMediaEncryptionSRTP && !sal_media_description_has_srtp(md);
}

void linphone_core_notify_incoming_call(LinphoneCore *lc, LinphoneCall *call){
	char *barmesg;
	char *tmp;
	LinphoneAddress *from_parsed;
	bool_t propose_early_media=lp_config_get_int(lc->config,"sip","incoming_calls_early_media",FALSE);

	from_parsed=linphone_address_new(sal_op_get_from(call->op));
	linphone_address_clean(from_parsed);
	tmp=linphone_address_as_string(from_parsed);
	linphone_address_destroy(from_parsed);
	barmesg=ortp_strdup_printf("%s %s%s",tmp,_("is contacting you"),
		(sal_call_autoanswer_asked(call->op)) ?_(" and asked autoanswer."):".");
	linphone_core_notify_show_interface(lc);
	linphone_core_notify_display_status(lc,barmesg);

	/* play the ring if this is the only call*/
	if (ms_list_size(lc->calls)==1){
		lc->current_call=call;
		if (lc->ringstream && lc->dmfs_playing_start_time!=0){
			linphone_core_stop_dtmf_stream(lc);
		}
		if (lc->sound_conf.ring_sndcard!=NULL){
			if(lc->ringstream==NULL && lc->sound_conf.local_ring){
				MSSndCard *ringcard=lc->sound_conf.lsd_card ?lc->sound_conf.lsd_card : lc->sound_conf.ring_sndcard;
				ms_message("Starting local ring...");
				lc->ringstream=ring_start(lc->sound_conf.local_ring,2000,ringcard);
			}
			else
			{
				ms_message("the local ring is already started");
			}
		}
	}else{
		/* else play a tone within the context of the current call */
		call->ringing_beep=TRUE;
		linphone_core_play_named_tone(lc,LinphoneToneCallWaiting);
	}

	linphone_call_set_state(call,LinphoneCallIncomingReceived,"Incoming call");
	/*from now on, the application is aware of the call and supposed to take background task or already submitted notification to the user.
	We can then drop our background task.*/
	if (call->bg_task_id!=0) {
		sal_end_background_task(call->bg_task_id);
		call->bg_task_id=0;
	}

	if (call->state==LinphoneCallIncomingReceived){
		/*try to be best-effort in giving real local or routable contact address for 100Rel case*/
		linphone_call_set_contact_op(call);

		if (propose_early_media){
			linphone_core_accept_early_media(lc,call);
		}else sal_call_notify_ringing(call->op,FALSE);

		if (sal_call_get_replaces(call->op)!=NULL && lp_config_get_int(lc->config,"sip","auto_answer_replacing_calls",1)){
			linphone_core_accept_call(lc,call);
		}
	}
	linphone_call_unref(call);

	ms_free(barmesg);
	ms_free(tmp);
}


/**
 * When receiving an incoming, accept to start a media session as early-media.
 * This means the call is not accepted but audio & video streams can be established if the remote party supports early media.
 * However, unlike after call acceptance, mic and camera input are not sent during early-media, though received audio & video are played normally.
 * The call can then later be fully accepted using linphone_core_accept_call() or linphone_core_accept_call_with_params().
 * @param lc the linphonecore
 * @param call the call
 * @param params the call params, can be NULL.
 * @return 0 if successful, -1 otherwise.
 * @ingroup call_control
**/
int linphone_core_accept_early_media_with_params(LinphoneCore* lc, LinphoneCall* call, const LinphoneCallParams* params) {
	if (call->state==LinphoneCallIncomingReceived){
		SalMediaDescription* md;

		/*try to be best-effort in giving real local or routable contact address for 100Rel case*/
		linphone_call_set_contact_op(call);

		// if parameters are passed, update the media description
		if ( params ) {
			linphone_call_set_new_params(call,params);
			linphone_call_make_local_media_description ( lc,call );
			sal_call_set_local_media_description ( call->op,call->localdesc );
			sal_op_set_sent_custom_header ( call->op,params->custom_headers );
		}

		sal_call_notify_ringing(call->op,TRUE);

		linphone_call_set_state(call,LinphoneCallIncomingEarlyMedia,"Incoming call early media");
		md=sal_call_get_final_media_description(call->op);
		if (md) linphone_core_update_streams(lc,call,md);
		return 0;
	}else{
		ms_error("Bad state %s for linphone_core_accept_early_media_with_params()", linphone_call_state_to_string(call->state));
	}

	return -1;
}

/**
 * Accept an early media session for an incoming call.
 * This is identical as calling linphone_core_accept_early_media_with_params() with NULL call parameters.
 * @see linphone_core_accept_early_media_with_params()
 * @param lc the core
 * @param call the incoming call
 * @return 0 if successful, -1 otherwise.
 * @ingroup call_control
**/
int linphone_core_accept_early_media(LinphoneCore* lc, LinphoneCall* call){
	return linphone_core_accept_early_media_with_params(lc, call, NULL);
}

int linphone_core_start_update_call(LinphoneCore *lc, LinphoneCall *call){
	const char *subject;
	int err;
	bool_t no_user_consent=call->params->no_user_consent;

	linphone_call_fill_media_multicast_addr(call);

	if (!no_user_consent) linphone_call_make_local_media_description(lc,call);
#ifdef BUILD_UPNP
	if(call->upnp_session != NULL) {
		linphone_core_update_local_media_description_from_upnp(call->localdesc, call->upnp_session);
	}
#endif //BUILD_UPNP
	if (call->params->in_conference){
		subject="Conference";
	}else if (!no_user_consent){
		subject="Media change";
	}else{
		subject="Refreshing";
	}
	linphone_core_notify_display_status(lc,_("Modifying call parameters..."));
	if (!lc->sip_conf.sdp_200_ack){
		sal_call_set_local_media_description (call->op,call->localdesc);
	} else {
		sal_call_set_local_media_description (call->op,NULL);
	}
	if (call->dest_proxy && call->dest_proxy->op){
		/*give a chance to update the contact address if connectivity has changed*/
		sal_op_set_contact_address(call->op,sal_op_get_contact_address(call->dest_proxy->op));
	}else sal_op_set_contact_address(call->op,NULL);
	err= sal_call_update(call->op,subject,no_user_consent);
	if (lc->sip_conf.sdp_200_ack){
		/*we are NOT offering, set local media description after sending the call so that we are ready to
		 process the remote offer when it will arrive*/
		sal_call_set_local_media_description(call->op,call->localdesc);
	}
	return err;
}

/**
 * @ingroup call_control
 * Updates a running call according to supplied call parameters or parameters changed in the LinphoneCore.
 *
 * In this version this is limited to the following use cases:
 * - setting up/down the video stream according to the video parameter of the LinphoneCallParams (see linphone_call_params_enable_video() ).
 * - changing the size of the transmitted video after calling linphone_core_set_preferred_video_size()
 *
 * In case no changes are requested through the LinphoneCallParams argument, then this argument can be omitted and set to NULL.
 * @param lc the core
 * @param call the call to be updated
 * @param params the new call parameters to use. (may be NULL)
 * @return 0 if successful, -1 otherwise.
**/
int linphone_core_update_call(LinphoneCore *lc, LinphoneCall *call, const LinphoneCallParams *params){
	int err=0;
	LinphoneCallState nextstate, initial_state;

#if defined(VIDEO_ENABLED) && defined(BUILD_UPNP)
	bool_t has_video = FALSE;
#endif

	switch(initial_state=call->state){
		case LinphoneCallIncomingReceived:
		case LinphoneCallIncomingEarlyMedia:
		case LinphoneCallOutgoingRinging:
		case LinphoneCallOutgoingEarlyMedia:
			nextstate=LinphoneCallEarlyUpdating;
			break;
		case LinphoneCallStreamsRunning:
			nextstate=LinphoneCallUpdating;
			break;
		default:
		ms_error("linphone_core_update_call() is not allowed in [%s] state",linphone_call_state_to_string(call->state));
		return -1;
	}

	if (params!=NULL){
		linphone_call_set_state(call,nextstate,"Updating call");
#if defined(VIDEO_ENABLED) && defined(BUILD_UPNP)
		has_video = call->params->has_video;

		// Video removing
		if((call->videostream != NULL) && !params->has_video) {
			if(call->upnp_session != NULL) {
				if (linphone_core_update_upnp(lc, call)<0) {
					/* uPnP port mappings failed, proceed with the call anyway. */
					linphone_call_delete_upnp_session(call);
				}
			}

		}
#endif /* defined(VIDEO_ENABLED) && defined(BUILD_UPNP) */
		linphone_call_set_new_params(call,params);
		err=linphone_call_prepare_ice(call,FALSE);
		if (err==1) {
			ms_message("Defer call update to gather ICE candidates");
			return 0;
		}

#if defined(VIDEO_ENABLED) && defined(BUILD_UPNP)
		// Video adding
		if (!has_video && call->params->has_video) {
			if(call->upnp_session != NULL) {
				ms_message("Defer call update to add uPnP port mappings");
				video_stream_prepare_video(call->videostream);
				if (linphone_core_update_upnp(lc, call)<0) {
					/* uPnP port mappings failed, proceed with the call anyway. */
					linphone_call_delete_upnp_session(call);
				} else {
					return err;
				}
			}
		}
#endif //defined(VIDEO_ENABLED) && defined(BUILD_UPNP)
		if ((err = linphone_core_start_update_call(lc, call)) && call->state!=initial_state) {
			/*Restore initial state*/
			linphone_call_set_state(call,initial_state,NULL);
		}

	}else{
#ifdef VIDEO_ENABLED
		if ((call->videostream != NULL) && (call->state == LinphoneCallStreamsRunning)) {
			video_stream_set_sent_video_size(call->videostream,linphone_core_get_preferred_video_size(lc));
			video_stream_set_fps(call->videostream, linphone_core_get_preferred_framerate(lc));
			if (call->camera_enabled && call->videostream->cam!=lc->video_conf.device){
				video_stream_change_camera(call->videostream,call->cam = lc->video_conf.device);
			}else video_stream_update_video_params(call->videostream);
		}
#endif
	}

	return err;
}

/**
 * @ingroup call_control
 * When receiving a #LinphoneCallUpdatedByRemote state notification, prevent LinphoneCore from performing an automatic answer.
 *
 * When receiving a #LinphoneCallUpdatedByRemote state notification (ie an incoming reINVITE), the default behaviour of
 * LinphoneCore is defined by the "defer_update_default" option of the "sip" section of the config. If this option is 0 (the default)
 * then the LinphoneCore automatically answers the reINIVTE with call parameters unchanged.
 * However when for example when the remote party updated the call to propose a video stream, it can be useful
 * to prompt the user before answering. This can be achieved by calling linphone_core_defer_call_update() during
 * the call state notification, to deactivate the automatic answer that would just confirm the audio but reject the video.
 * Then, when the user responds to dialog prompt, it becomes possible to call linphone_core_accept_call_update() to answer
 * the reINVITE, with eventually video enabled in the LinphoneCallParams argument.
 *
 * @return 0 if successful, -1 if the linphone_core_defer_call_update() was done outside a #LinphoneCallUpdatedByRemote notification, which is illegal.
**/
int linphone_core_defer_call_update(LinphoneCore *lc, LinphoneCall *call){
	if (call->state==LinphoneCallUpdatedByRemote){
		call->defer_update=TRUE;
		return 0;
	}
	ms_error("linphone_core_defer_call_update() not done in state LinphoneCallUpdatedByRemote");
	return -1;
}

int linphone_core_start_accept_call_update(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState next_state, const char *state_info){
	SalMediaDescription *md;
	if (call->ice_session != NULL) {
		if (ice_session_nb_losing_pairs(call->ice_session) > 0) {
			/* Defer the sending of the answer until there are no losing pairs left. */
			return 0;
		}
	}
	linphone_call_make_local_media_description(lc,call);

	linphone_call_update_remote_session_id_and_ver(call);
	linphone_call_stop_ice_for_inactive_streams(call);
	sal_call_set_local_media_description(call->op,call->localdesc);
	sal_call_accept(call->op);
	md=sal_call_get_final_media_description(call->op);
	if (md && !sal_media_description_empty(md)){
		linphone_core_update_streams (lc,call,md);
		linphone_call_fix_call_parameters(call);
	}
	linphone_call_set_state(call,next_state,state_info);
	return 0;
}

/**
 * @ingroup call_control
 * Accept call modifications initiated by other end.
 *
 * This call may be performed in response to a #LinphoneCallUpdatedByRemote state notification.
 * When such notification arrives, the application can decide to call linphone_core_defer_update_call() so that it can
 * have the time to prompt the user. linphone_call_get_remote_params() can be used to get information about the call parameters
 * requested by the other party, such as whether a video stream is requested.
 *
 * When the user accepts or refuse the change, linphone_core_accept_call_update() can be done to answer to the other party.
 * If params is NULL, then the same call parameters established before the update request will continue to be used (no change).
 * If params is not NULL, then the update will be accepted according to the parameters passed.
 * Typical example is when a user accepts to start video, then params should indicate that video stream should be used
 * (see linphone_call_params_enable_video()).
 * @param lc the linphone core object.
 * @param call the LinphoneCall object
 * @param params a LinphoneCallParams object describing the call parameters to accept.
 * @return 0 if successful, -1 otherwise (actually when this function call is performed outside ot #LinphoneCallUpdatedByRemote state).
**/
int linphone_core_accept_call_update(LinphoneCore *lc, LinphoneCall *call, const LinphoneCallParams *params){
	if (call->state!=LinphoneCallUpdatedByRemote){
		ms_error("linphone_core_accept_update(): invalid state %s to call this function.",
				 linphone_call_state_to_string(call->state));
		return -1;
	}
	return _linphone_core_accept_call_update(lc, call, params, call->prevstate, linphone_call_state_to_string(call->prevstate));
}

int _linphone_core_accept_call_update(LinphoneCore *lc, LinphoneCall *call, const LinphoneCallParams *params, LinphoneCallState next_state, const char *state_info){
	SalMediaDescription *remote_desc;
	bool_t keep_sdp_version;
#if defined(VIDEO_ENABLED) && defined(BUILD_UPNP)
	bool_t old_has_video = call->params->has_video;
#endif

	remote_desc = sal_call_get_remote_media_description(call->op);
	keep_sdp_version = lp_config_get_int(lc->config, "sip", "keep_sdp_version", 0);
	if (keep_sdp_version &&(remote_desc->session_id == call->remote_session_id) && (remote_desc->session_ver == call->remote_session_ver)) {
		/* Remote has sent an INVITE with the same SDP as before, so send a 200 OK with the same SDP as before. */
		ms_warning("SDP version has not changed, send same SDP as before.");
		sal_call_accept(call->op);
		linphone_call_set_state(call,next_state,state_info);
		return 0;
	}
	if (params==NULL){
		linphone_call_params_enable_video(call->params, lc->video_policy.automatically_accept || call->current_params->has_video);
		if (!sal_call_is_offerer(call->op)) {
			/*reset call param for multicast because this param is only relevant when offering*/
			linphone_call_params_enable_audio_multicast(call->params,FALSE);
			linphone_call_params_enable_video_multicast(call->params,FALSE);
		}
	}else
		linphone_call_set_new_params(call,params);

	if (call->params->has_video && !linphone_core_video_enabled(lc)){
		ms_warning("linphone_core_accept_call_update(): requested video but video support is globally disabled. Refusing video.");
		call->params->has_video=FALSE;
	}
	if (call->current_params->in_conference) {
		ms_warning("Video isn't supported in conference");
		call->params->has_video = FALSE;
	}
	/*update multicast params according to call params*/
	linphone_call_fill_media_multicast_addr(call);

	linphone_call_init_media_streams(call); /*so that video stream is initialized if necessary*/
	if (call->ice_session != NULL) {
		if (linphone_call_prepare_ice(call,TRUE)==1)
			return 0;/*deferred to completion of ICE gathering*/
	}

#ifdef BUILD_UPNP
	if(call->upnp_session != NULL) {
		linphone_core_update_upnp_from_remote_media_description(call, sal_call_get_remote_media_description(call->op));
#ifdef VIDEO_ENABLED
		if ((call->params->has_video) && (call->params->has_video != old_has_video)) {
			video_stream_prepare_video(call->videostream);
			if (linphone_core_update_upnp(lc, call)<0) {
				/* uPnP update failed, proceed with the call anyway. */
				linphone_call_delete_upnp_session(call);
			} else return 0;
		}
#endif //VIDEO_ENABLED
	}
#endif //BUILD_UPNP

	linphone_core_start_accept_call_update(lc, call, next_state, state_info);
	return 0;
}

/**
 * Accept an incoming call.
 *
 * @ingroup call_control
 * Basically the application is notified of incoming calls within the
 * call_state_changed callback of the #LinphoneCoreVTable structure, where it will receive
 * a LinphoneCallIncoming event with the associated LinphoneCall object.
 * The application can later accept the call using this method.
 * @param lc the LinphoneCore object
 * @param call the LinphoneCall object representing the call to be answered.
 *
**/
int linphone_core_accept_call(LinphoneCore *lc, LinphoneCall *call){
	return linphone_core_accept_call_with_params(lc,call,NULL);
}

/**
 * Accept an incoming call, with parameters.
 *
 * @ingroup call_control
 * Basically the application is notified of incoming calls within the
 * call_state_changed callback of the #LinphoneCoreVTable structure, where it will receive
 * a LinphoneCallIncoming event with the associated LinphoneCall object.
 * The application can later accept the call using
 * this method.
 * @param lc the LinphoneCore object
 * @param call the LinphoneCall object representing the call to be answered.
 * @param params the specific parameters for this call, for example whether video is accepted or not. Use NULL to use default parameters.
 *
**/
int linphone_core_accept_call_with_params(LinphoneCore *lc, LinphoneCall *call, const LinphoneCallParams *params){
	SalOp *replaced;
	SalMediaDescription *new_md;
	bool_t was_ringing=FALSE;
	MSList * iterator, *copy;

	if (call==NULL){
		//if just one call is present answer the only one ...
		if(linphone_core_get_calls_nb (lc) != 1)
			return -1;
		else
			call = (LinphoneCall*)linphone_core_get_calls(lc)->data;
	}

	switch(call->state){
		case LinphoneCallIncomingReceived:
		case LinphoneCallIncomingEarlyMedia:
		break;
		default:
			ms_error("linphone_core_accept_call_with_params() call [%p] is in state [%s], operation not permitted.",
				 call, linphone_call_state_to_string(call->state));
			return -1;
			break;
	}


	for (iterator=copy=ms_list_copy(linphone_core_get_calls(lc));iterator!=NULL;iterator=iterator->next) {
		LinphoneCall *a_call=(LinphoneCall*)iterator->data;
		if (a_call==call) continue;
		switch(a_call->state){
			case LinphoneCallOutgoingInit:
			case LinphoneCallOutgoingProgress:
			case LinphoneCallOutgoingRinging:
			case LinphoneCallOutgoingEarlyMedia:
				ms_message("Already existing call [%p] in state [%s], canceling it before accepting new call [%p]",a_call
						,linphone_call_state_to_string(a_call->state)
						,call);
				linphone_core_terminate_call(lc,a_call);
				break;
			default:
				break; /*nothing to do*/
		}
	}
	ms_list_free(copy);

	/* check if this call is supposed to replace an already running one*/
	replaced=sal_call_get_replaces(call->op);
	if (replaced){
		LinphoneCall *rc=(LinphoneCall*)sal_op_get_user_pointer (replaced);
		if (rc){
			ms_message("Call %p replaces call %p. This last one is going to be terminated automatically.",
					   call,rc);
			linphone_core_terminate_call(lc,rc);
		}
	}

	if (lc->current_call!=call){
		linphone_core_preempt_sound_resources(lc);
	}

	/*stop ringing */
	if (lc->ringstream!=NULL) {
		ms_message("stop ringing");
		linphone_core_stop_ringing(lc);
		was_ringing=TRUE;
	}
	if (call->ringing_beep){
		linphone_core_stop_dtmf(lc);
		call->ringing_beep=FALSE;
	}

	/*try to be best-effort in giving real local or routable contact address */
	linphone_call_set_contact_op(call);
	if (params){
		const SalMediaDescription *md = sal_call_get_remote_media_description(call->op);
		linphone_call_set_new_params(call,params);
		// There might not be a md if the INVITE was lacking an SDP
		// In this case we use the parameters as is.
		if (md) {
			linphone_call_set_compatible_incoming_call_parameters(call, md);
		}
		linphone_call_prepare_ice(call,TRUE);
		linphone_call_make_local_media_description(lc,call);
		sal_call_set_local_media_description(call->op,call->localdesc);
		sal_op_set_sent_custom_header(call->op,params->custom_headers);
	}

	/*give a chance a set card prefered sampling frequency*/
	if (call->localdesc->streams[0].max_rate>0) {
		ms_message ("configuring prefered card sampling rate to [%i]",call->localdesc->streams[0].max_rate);
		if (lc->sound_conf.play_sndcard)
			ms_snd_card_set_preferred_sample_rate(lc->sound_conf.play_sndcard, call->localdesc->streams[0].max_rate);
		if (lc->sound_conf.capt_sndcard)
			ms_snd_card_set_preferred_sample_rate(lc->sound_conf.capt_sndcard, call->localdesc->streams[0].max_rate);
	}

	if (!was_ringing && call->audiostream->ms.state==MSStreamInitialized && !lc->use_files){
		audio_stream_prepare_sound(call->audiostream,lc->sound_conf.play_sndcard,lc->sound_conf.capt_sndcard);
	}

	linphone_call_update_remote_session_id_and_ver(call);
	linphone_call_stop_ice_for_inactive_streams(call);
	sal_call_accept(call->op);
	linphone_core_notify_display_status(lc,_("Connected."));
	lc->current_call=call;
	linphone_call_set_state(call,LinphoneCallConnected,"Connected");
	new_md=sal_call_get_final_media_description(call->op);
	if (new_md){
		linphone_core_update_streams(lc, call, new_md);
		linphone_call_fix_call_parameters(call);
		linphone_call_set_state(call,LinphoneCallStreamsRunning,"Connected (streams running)");
	}else call->expect_media_in_ack=TRUE;

	ms_message("call answered.");
	return 0;
}

int linphone_core_abort_call(LinphoneCore *lc, LinphoneCall *call, const char *error){
	sal_call_terminate(call->op);

	/*stop ringing*/
	linphone_core_stop_ringing(lc);
	linphone_call_stop_media_streams(call);

#ifdef BUILD_UPNP
	linphone_call_delete_upnp_session(call);
#endif //BUILD_UPNP

	linphone_core_notify_display_status(lc,_("Call aborted") );
	linphone_call_set_state(call,LinphoneCallError,error);
	return 0;
}

static void terminate_call(LinphoneCore *lc, LinphoneCall *call){
	if (call->state==LinphoneCallIncomingReceived){
		if (call->non_op_error.reason!=SalReasonRequestTimeout)
			call->non_op_error.reason=SalReasonDeclined;
	}
	/*stop ringing*/
	linphone_core_stop_ringing(lc);

	linphone_call_stop_media_streams(call);

#ifdef BUILD_UPNP
	linphone_call_delete_upnp_session(call);
#endif //BUILD_UPNP

	linphone_core_notify_display_status(lc,_("Call ended") );
	linphone_call_set_state(call,LinphoneCallEnd,"Call terminated");
}

int linphone_core_redirect_call(LinphoneCore *lc, LinphoneCall *call, const char *redirect_uri){
	if (call->state==LinphoneCallIncomingReceived){
		sal_call_decline(call->op,SalReasonRedirect,redirect_uri);
		sal_error_info_set(&call->non_op_error,SalReasonRedirect,603,"Call redirected",NULL);
		terminate_call(lc,call);
	}else{
		ms_error("Bad state for call redirection.");
		return -1;
	}
	return 0;
}


/**
 * Terminates a call.
 *
 * @ingroup call_control
 * @param lc the LinphoneCore
 * @param the_call the LinphoneCall object representing the call to be terminated.
**/
int linphone_core_terminate_call(LinphoneCore *lc, LinphoneCall *the_call)
{
	LinphoneCall *call;
	if (the_call == NULL){
		call = linphone_core_get_current_call(lc);
		if (ms_list_size(lc->calls)==1){
			call=(LinphoneCall*)lc->calls->data;
		}else{
			ms_warning("No unique call to terminate !");
			return -1;
		}
	}
	else
	{
		call = the_call;
	}
	switch (call->state) {
		case LinphoneCallReleased:
		case LinphoneCallEnd:
			ms_warning("No need to terminate a call [%p] in state [%s]",call,linphone_call_state_to_string(call->state));
			return -1;
		case LinphoneCallIncomingReceived:
		case LinphoneCallIncomingEarlyMedia:
			return linphone_core_decline_call(lc,call,LinphoneReasonDeclined);
		case LinphoneCallOutgoingInit: {
				/* In state OutgoingInit, op has to be destroyed */
				sal_op_release(call->op);
				call->op = NULL;
				break;
			}
		default:
			sal_call_terminate(call->op);
			break;
	}
	terminate_call(lc,call);
	return 0;
}

/**
 * Decline a pending incoming call, with a reason.
 *
 * @ingroup call_control
 *
 * @param lc the linphone core
 * @param call the LinphoneCall, must be in the IncomingReceived state.
 * @param reason the reason for rejecting the call: LinphoneReasonDeclined or LinphoneReasonBusy
**/
int linphone_core_decline_call(LinphoneCore *lc, LinphoneCall * call, LinphoneReason reason){
	if (call->state!=LinphoneCallIncomingReceived && call->state!=LinphoneCallIncomingEarlyMedia){
		ms_error("linphone_core_decline_call(): Cannot decline a call that is in state %s",linphone_call_state_to_string(call->state));
		return -1;
	}

	sal_call_decline(call->op,linphone_reason_to_sal(reason),NULL);
	terminate_call(lc,call);
	return 0;
}

/**
 * Terminates all the calls.
 *
 * @ingroup call_control
 * @param lc The LinphoneCore
**/
int linphone_core_terminate_all_calls(LinphoneCore *lc){
	MSList *calls=lc->calls;
	while(calls) {
		LinphoneCall *c=(LinphoneCall*)calls->data;
		calls=calls->next;
		linphone_core_terminate_call(lc,c);
	}
	return 0;
}

/**
 * Returns the current list of calls.
 * @param[in] lc The LinphoneCore object
 * @return \mslist{LinphoneCall}
 *
 * Note that this list is read-only and might be changed by the core after a function call to linphone_core_iterate().
 * Similarly the LinphoneCall objects inside it might be destroyed without prior notice.
 * To hold references to LinphoneCall object into your program, you must use linphone_call_ref().
 *
 * @ingroup call_control
**/
const MSList *linphone_core_get_calls(LinphoneCore *lc)
{
	return lc->calls;
}

/**
 * Returns TRUE if there is a call running.
 *
 * @ingroup call_control
**/
bool_t linphone_core_in_call(const LinphoneCore *lc){
	return linphone_core_get_current_call((LinphoneCore *)lc)!=NULL || linphone_core_is_in_conference(lc);
}

/**
 * Returns The _LinphoneCall struct of the current call if one is in call
 *
 * @ingroup call_control
**/
LinphoneCall *linphone_core_get_current_call(const LinphoneCore *lc){
	return lc->current_call;
}

/**
 * Pauses the call. If a music file has been setup using linphone_core_set_play_file(),
 * this file will be played to the remote user.
 *
 * @ingroup call_control
**/
int linphone_core_pause_call(LinphoneCore *lc, LinphoneCall *call){
	int err=_linphone_core_pause_call(lc,call);
	if (err==0)  call->paused_by_app=TRUE;
	return err;
}

/* Internal version that does not play tone indication*/
int _linphone_core_pause_call(LinphoneCore *lc, LinphoneCall *call)
{
	const char *subject=NULL;
	LinphoneCallParams *params;

	if (call->state!=LinphoneCallStreamsRunning && call->state!=LinphoneCallPausedByRemote){
		ms_warning("Cannot pause this call, it is not active.");
		return -1;
	}
	if (sal_media_description_has_dir(call->resultdesc, SalStreamSendRecv)) {
		subject = "Call on hold";
	} else if (sal_media_description_has_dir(call->resultdesc, SalStreamRecvOnly)) {
		subject = "Call on hold for me too";
	} else {
		ms_error("No reason to pause this call, it is already paused or inactive.");
		return -1;
	}
	params = linphone_call_params_copy(call->params);
	linphone_call_params_set_audio_direction(params, LinphoneMediaDirectionSendOnly);
	if (lp_config_get_int(lc->config, "sip", "inactive_video_on_pause", 0)) {
		linphone_call_params_set_video_direction(params, LinphoneMediaDirectionInactive);
	} else {
		linphone_call_params_set_video_direction(params, LinphoneMediaDirectionSendOnly);
	}
	linphone_call_make_local_media_description_with_params(lc, call, params);
	linphone_call_params_unref(params);
#ifdef BUILD_UPNP
	if(call->upnp_session != NULL) {
		linphone_core_update_local_media_description_from_upnp(call->localdesc, call->upnp_session);
	}
#endif //BUILD_UPNP
	sal_call_set_local_media_description(call->op,call->localdesc);
	if (sal_call_update(call->op,subject,FALSE) != 0){
		linphone_core_notify_display_warning(lc,_("Could not pause the call"));
	}
	lc->current_call=NULL;
	linphone_core_notify_display_status(lc,_("Pausing the current call..."));
	if (call->audiostream || call->videostream)
		linphone_call_stop_media_streams (call);
	linphone_call_set_state(call,LinphoneCallPausing,"Pausing call");
	call->paused_by_app=FALSE;
	return 0;
}

/**
 * Pause all currently running calls.
 * @ingroup call_control
**/
int linphone_core_pause_all_calls(LinphoneCore *lc){
	const MSList *elem;
	for(elem=lc->calls;elem!=NULL;elem=elem->next){
		LinphoneCall *call=(LinphoneCall *)elem->data;
		LinphoneCallState cs=linphone_call_get_state(call);
		if (cs==LinphoneCallStreamsRunning || cs==LinphoneCallPausedByRemote){
			_linphone_core_pause_call(lc,call);
		}
	}
	return 0;
}

void linphone_core_preempt_sound_resources(LinphoneCore *lc){
	LinphoneCall *current_call;

	if (linphone_core_is_in_conference(lc)){
		linphone_core_leave_conference(lc);
		return;
	}

	current_call=linphone_core_get_current_call(lc);
	if(current_call != NULL){
		ms_message("Pausing automatically the current call.");
		_linphone_core_pause_call(lc,current_call);
	}
	if (lc->ringstream){
		linphone_core_stop_ringing(lc);
	}
}

/**
 * Resumes the call.
 *
 * @ingroup call_control
**/
int linphone_core_resume_call(LinphoneCore *lc, LinphoneCall *call){
	char temp[255]={0};
	const char *subject="Call resuming";

	if(call->state!=LinphoneCallPaused ){
		ms_warning("we cannot resume a call that has not been established and paused before");
		return -1;
	}
	if (call->params->in_conference==FALSE){
		if (linphone_core_sound_resources_locked(lc)){
			ms_warning("Cannot resume call %p because another call is locking the sound resources.",call);
			return -1;
		}
		linphone_core_preempt_sound_resources(lc);
		ms_message("Resuming call %p",call);
	}

	call->was_automatically_paused=FALSE;

	/* Stop playing music immediately. If remote side is a conference it
	 prevents the participants to hear it while the 200OK comes back.*/
	if (call->audiostream) audio_stream_play(call->audiostream, NULL);

	linphone_call_make_local_media_description(lc,call);
#ifdef BUILD_UPNP
	if(call->upnp_session != NULL) {
		linphone_core_update_local_media_description_from_upnp(call->localdesc, call->upnp_session);
	}
#endif //BUILD_UPNP
	if (!lc->sip_conf.sdp_200_ack){
		sal_call_set_local_media_description(call->op,call->localdesc);
	} else {
		sal_call_set_local_media_description(call->op,NULL);
	}
	sal_media_description_set_dir(call->localdesc,SalStreamSendRecv);
	if (call->params->in_conference && !call->current_params->in_conference) subject="Conference";
	if ( sal_call_update(call->op,subject,FALSE) != 0){
		return -1;
	}
	linphone_call_set_state(call,LinphoneCallResuming,"Resuming");
	if (call->params->in_conference==FALSE)
		lc->current_call=call;
	snprintf(temp,sizeof(temp)-1,"Resuming the call with %s",linphone_call_get_remote_address_as_string(call));
	linphone_core_notify_display_status(lc,temp);

	if (lc->sip_conf.sdp_200_ack){
		/*we are NOT offering, set local media description after sending the call so that we are ready to
		 process the remote offer when it will arrive*/
		sal_call_set_local_media_description(call->op,call->localdesc);
	}
	return 0;
}

static int remote_address_compare(LinphoneCall *call, const LinphoneAddress *raddr){
	const LinphoneAddress *addr=linphone_call_get_remote_address (call);
	return !linphone_address_weak_equal (addr,raddr);
}

/**
 * Get the call with the remote_address specified
 * @param lc
 * @param remote_address
 * @return the LinphoneCall of the call if found
 *
 * @ingroup call_control
 */
LinphoneCall *linphone_core_get_call_by_remote_address(LinphoneCore *lc, const char *remote_address){
	LinphoneCall *call=NULL;
	LinphoneAddress *raddr=linphone_address_new(remote_address);
	if (raddr) {
		call=linphone_core_get_call_by_remote_address2(lc, raddr);
		linphone_address_unref(raddr);
	}
	return call;
}
LinphoneCall *linphone_core_get_call_by_remote_address2(LinphoneCore *lc, const LinphoneAddress *raddr){
	MSList *elem=ms_list_find_custom(lc->calls,(int (*)(const void*,const void *))remote_address_compare,raddr);

	if (elem) return (LinphoneCall*) elem->data;
	return NULL;
}

int linphone_core_send_publish(LinphoneCore *lc,
				   LinphonePresenceModel *presence)
{
	const MSList *elem;
	for (elem=linphone_core_get_proxy_config_list(lc);elem!=NULL;elem=ms_list_next(elem)){
		LinphoneProxyConfig *cfg=(LinphoneProxyConfig*)elem->data;
		if (cfg->publish) linphone_proxy_config_send_publish(cfg,presence);
	}
	return 0;
}

/**
 * Set the incoming call timeout in seconds.
 *
 * @ingroup call_control
 * If an incoming call isn't answered for this timeout period, it is
 * automatically declined.
**/
void linphone_core_set_inc_timeout(LinphoneCore *lc, int seconds){
	lc->sip_conf.inc_timeout=seconds;
	if (linphone_core_ready(lc)){
		lp_config_set_int(lc->config,"sip","inc_timeout",seconds);
	}
}

/**
 * Returns the incoming call timeout
 *
 * @ingroup call_control
 * See linphone_core_set_inc_timeout() for details.
**/
int linphone_core_get_inc_timeout(LinphoneCore *lc){
	return lc->sip_conf.inc_timeout;
}

/**
 * Set the in call timeout in seconds.
 *
 * @ingroup call_control
 * After this timeout period, the call is automatically hangup.
**/
void linphone_core_set_in_call_timeout(LinphoneCore *lc, int seconds){
	lc->sip_conf.in_call_timeout=seconds;
	if( linphone_core_ready(lc)){
		lp_config_set_int(lc->config, "sip", "in_call_timeout", seconds);
	}
}

/**
 * Returns the in call timeout
 *
 * @ingroup call_control
 * See linphone_core_set_in_call_timeout() for details.
**/
int linphone_core_get_in_call_timeout(LinphoneCore *lc){
	return lc->sip_conf.in_call_timeout;
}

/**
 * Returns the delayed timeout
 *
 * @ingroup call_control
 * See linphone_core_set_delayed_timeout() for details.
**/
int linphone_core_get_delayed_timeout(LinphoneCore *lc){
	return lc->sip_conf.delayed_timeout;
}

/**
 * Set the in delayed timeout in seconds.
 *
 * @ingroup call_control
 * After this timeout period, a delayed call (internal call initialisation or resolution) is resumed.
**/
void linphone_core_set_delayed_timeout(LinphoneCore *lc, int seconds){
	lc->sip_conf.delayed_timeout=seconds;
}

void linphone_core_set_presence_info(LinphoneCore *lc, int minutes_away, const char *contact, LinphoneOnlineStatus os) {
	LinphonePresenceModel *presence = NULL;
	char *description = NULL;
	LinphonePresenceActivityType acttype = LinphonePresenceActivityUnknown;

	if (minutes_away>0) lc->minutes_away=minutes_away;

	switch (os) {
		case LinphoneStatusOffline:
			acttype = LinphonePresenceActivityOffline;
			break;
		case LinphoneStatusOnline:
			acttype = LinphonePresenceActivityOnline;
			break;
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
	presence = linphone_presence_model_new_with_activity(acttype, description);
	linphone_presence_model_set_contact(presence, contact);
	linphone_core_set_presence_model(lc, presence);
}

void linphone_core_send_presence(LinphoneCore *lc, LinphonePresenceModel *presence){
	linphone_core_notify_all_friends(lc,presence);
	linphone_core_send_publish(lc,presence);
}

void linphone_core_set_presence_model(LinphoneCore *lc, LinphonePresenceModel *presence) {
	linphone_core_send_presence(lc,presence);

	if ((lc->presence_model != NULL) && (lc->presence_model != presence)) {
		linphone_presence_model_unref(lc->presence_model);
		lc->presence_model = presence;
	}
}

LinphoneOnlineStatus linphone_core_get_presence_info(const LinphoneCore *lc){
	LinphonePresenceActivity *activity = NULL;
	const char *description = NULL;

	activity = linphone_presence_model_get_activity(lc->presence_model);
	description = linphone_presence_activity_get_description(activity);
	switch (linphone_presence_activity_get_type(activity)) {
		case LinphonePresenceActivityOffline:
			return LinphoneStatusOffline;
		case LinphonePresenceActivityOnline:
			return LinphoneStatusOnline;
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
}

LinphonePresenceModel * linphone_core_get_presence_model(const LinphoneCore *lc) {
	return lc->presence_model;
}

/**
 * Get playback sound level in 0-100 scale.
 *
 * @ingroup media_parameters
**/
int linphone_core_get_play_level(LinphoneCore *lc)
{
	return lc->sound_conf.play_lev;
}

/**
 * Get ring sound level in 0-100 scale
 *
 * @ingroup media_parameters
**/
int linphone_core_get_ring_level(LinphoneCore *lc)
{
	return lc->sound_conf.ring_lev;
}

/**
 * Get sound capture level in 0-100 scale
 *
 * @ingroup media_parameters
**/
int linphone_core_get_rec_level(LinphoneCore *lc){
	return lc->sound_conf.rec_lev;
}

/**
 * Set sound ring level in 0-100 scale
 *
 * @ingroup media_parameters
**/
void linphone_core_set_ring_level(LinphoneCore *lc, int level){
	MSSndCard *sndcard;
	lc->sound_conf.ring_lev=level;
	sndcard=lc->sound_conf.ring_sndcard;
	if (sndcard) ms_snd_card_set_level(sndcard,MS_SND_CARD_PLAYBACK,level);
}

/**
 * Allow to control microphone level:  gain in db
 *
 * @ingroup media_parameters
**/
void linphone_core_set_mic_gain_db (LinphoneCore *lc, float gaindb){
	float gain=gaindb;
	LinphoneCall *call=linphone_core_get_current_call (lc);
	AudioStream *st;

	lc->sound_conf.soft_mic_lev=gaindb;

	if (linphone_core_ready(lc)){
		lp_config_set_float(lc->config,"sound","mic_gain_db",lc->sound_conf.soft_mic_lev);
	}

	if (call==NULL || (st=call->audiostream)==NULL){
		ms_message("linphone_core_set_mic_gain_db(): no active call.");
		return;
	}
	set_mic_gain_db(st,gain);
}

/**
 * Get microphone gain in db.
 *
 * @ingroup media_parameters
**/
float linphone_core_get_mic_gain_db(LinphoneCore *lc) {
	return lc->sound_conf.soft_mic_lev;
}

/**
 * Allow to control play level before entering sound card:  gain in db
 *
 * @ingroup media_parameters
**/
void linphone_core_set_playback_gain_db (LinphoneCore *lc, float gaindb){
	float gain=gaindb;
	LinphoneCall *call=linphone_core_get_current_call (lc);
	AudioStream *st;

	lc->sound_conf.soft_play_lev=gaindb;
	if (linphone_core_ready(lc)){
		lp_config_set_float(lc->config,"sound","playback_gain_db",lc->sound_conf.soft_play_lev);
	}

	if (call==NULL || (st=call->audiostream)==NULL){
		ms_message("linphone_core_set_playback_gain_db(): no active call.");
		return;
	}
	set_playback_gain_db(st,gain);
}

/**
 * Get playback gain in db before entering  sound card.
 *
 * @ingroup media_parameters
**/
float linphone_core_get_playback_gain_db(LinphoneCore *lc) {
	return lc->sound_conf.soft_play_lev;
}

/**
 * Set sound playback level in 0-100 scale
 *
 * @ingroup media_parameters
**/
void linphone_core_set_play_level(LinphoneCore *lc, int level){
	MSSndCard *sndcard;
	lc->sound_conf.play_lev=level;
	sndcard=lc->sound_conf.play_sndcard;
	if (sndcard) ms_snd_card_set_level(sndcard,MS_SND_CARD_PLAYBACK,level);
}

/**
 * Set sound capture level in 0-100 scale
 *
 * @ingroup media_parameters
**/
void linphone_core_set_rec_level(LinphoneCore *lc, int level)
{
	MSSndCard *sndcard;
	lc->sound_conf.rec_lev=level;
	sndcard=lc->sound_conf.capt_sndcard;
	if (sndcard) ms_snd_card_set_level(sndcard,MS_SND_CARD_CAPTURE,level);
}

static MSSndCard *get_card_from_string_id(const char *devid, unsigned int cap){
	MSSndCard *sndcard=NULL;
	if (devid!=NULL){
		sndcard=ms_snd_card_manager_get_card(ms_snd_card_manager_get(),devid);
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
			sndcard=ms_snd_card_manager_get_default_card(ms_snd_card_manager_get());
		}else if (cap & MS_SND_CARD_CAP_CAPTURE){
			sndcard=ms_snd_card_manager_get_default_capture_card(ms_snd_card_manager_get());
		}
		else if (cap & MS_SND_CARD_CAP_PLAYBACK){
			sndcard=ms_snd_card_manager_get_default_playback_card(ms_snd_card_manager_get());
		}
		if (sndcard==NULL){/*looks like a bug! take the first one !*/
			const MSList *elem=ms_snd_card_manager_get_list(ms_snd_card_manager_get());
			if (elem) sndcard=(MSSndCard*)elem->data;
		}
	}
	if (sndcard==NULL) ms_error("Could not find a suitable soundcard !");
	return sndcard;
}

/**
 * Returns true if the specified sound device can capture sound.
 *
 * @ingroup media_parameters
 * @param lc The LinphoneCore object
 * @param devid the device name as returned by linphone_core_get_sound_devices()
**/
bool_t linphone_core_sound_device_can_capture(LinphoneCore *lc, const char *devid){
	MSSndCard *sndcard;
	sndcard=ms_snd_card_manager_get_card(ms_snd_card_manager_get(),devid);
	if (sndcard!=NULL && (ms_snd_card_get_capabilities(sndcard) & MS_SND_CARD_CAP_CAPTURE)) return TRUE;
	return FALSE;
}

/**
 * Returns true if the specified sound device can play sound.
 *
 * @ingroup media_parameters
 * @param lc The LinphoneCore object
 * @param devid the device name as returned by linphone_core_get_sound_devices()
**/
bool_t linphone_core_sound_device_can_playback(LinphoneCore *lc, const char *devid){
	MSSndCard *sndcard;
	sndcard=ms_snd_card_manager_get_card(ms_snd_card_manager_get(),devid);
	if (sndcard!=NULL && (ms_snd_card_get_capabilities(sndcard) & MS_SND_CARD_CAP_PLAYBACK)) return TRUE;
	return FALSE;
}

/**
 * Sets the sound device used for ringing.
 *
 * @ingroup media_parameters
 * @param lc The LinphoneCore object
 * @param devid the device name as returned by linphone_core_get_sound_devices()
**/
int linphone_core_set_ringer_device(LinphoneCore *lc, const char * devid){
	MSSndCard *card=get_card_from_string_id(devid,MS_SND_CARD_CAP_PLAYBACK);
	lc->sound_conf.ring_sndcard=card;
	if (card && linphone_core_ready(lc))
		lp_config_set_string(lc->config,"sound","ringer_dev_id",ms_snd_card_get_string_id(card));
	return 0;
}

/**
 * Sets the sound device used for playback.
 *
 * @ingroup media_parameters
 * @param lc The LinphoneCore object
 * @param devid the device name as returned by linphone_core_get_sound_devices()
**/
int linphone_core_set_playback_device(LinphoneCore *lc, const char * devid){
	MSSndCard *card=get_card_from_string_id(devid,MS_SND_CARD_CAP_PLAYBACK);
	lc->sound_conf.play_sndcard=card;
	if (card &&  linphone_core_ready(lc))
		lp_config_set_string(lc->config,"sound","playback_dev_id",ms_snd_card_get_string_id(card));
	return 0;
}

/**
 * Sets the sound device used for capture.
 *
 * @ingroup media_parameters
 * @param lc The LinphoneCore object
 * @param devid the device name as returned by linphone_core_get_sound_devices()
**/
int linphone_core_set_capture_device(LinphoneCore *lc, const char * devid){
	MSSndCard *card=get_card_from_string_id(devid,MS_SND_CARD_CAP_CAPTURE);
	lc->sound_conf.capt_sndcard=card;
	if (card &&  linphone_core_ready(lc))
		lp_config_set_string(lc->config,"sound","capture_dev_id",ms_snd_card_get_string_id(card));
	return 0;
}

/**
 * Returns the name of the currently assigned sound device for ringing.
 *
 * @ingroup media_parameters
 * @param lc The LinphoneCore object
**/
const char * linphone_core_get_ringer_device(LinphoneCore *lc)
{
	if (lc->sound_conf.ring_sndcard) return ms_snd_card_get_string_id(lc->sound_conf.ring_sndcard);
	return NULL;
}

/**
 * Returns the name of the currently assigned sound device for playback.
 *
 * @ingroup media_parameters
 * @param lc The LinphoneCore object
**/
const char * linphone_core_get_playback_device(LinphoneCore *lc)
{
	return lc->sound_conf.play_sndcard ? ms_snd_card_get_string_id(lc->sound_conf.play_sndcard) : NULL;
}

/**
 * Returns the name of the currently assigned sound device for capture.
 *
 * @ingroup media_parameters
 * @param lc The LinphoneCore object
**/
const char * linphone_core_get_capture_device(LinphoneCore *lc)
{
	return lc->sound_conf.capt_sndcard ? ms_snd_card_get_string_id(lc->sound_conf.capt_sndcard) : NULL;
}

/**
 * Returns an unmodifiable array of available sound devices.
 *
 * The array is NULL terminated.
 *
 * @ingroup media_parameters
 * @param lc The LinphoneCore object
**/
const char**  linphone_core_get_sound_devices(LinphoneCore *lc){
	return lc->sound_conf.cards;
}

/**
 * Returns an unmodifiable array of available video capture devices.
 *
 * @ingroup media_parameters
 * The array is NULL terminated.
**/
const char**  linphone_core_get_video_devices(const LinphoneCore *lc){
	return lc->video_conf.cams;
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
	ms_snd_card_manager_reload(ms_snd_card_manager_get());
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
	ms_web_cam_manager_reload(ms_web_cam_manager_get());
	build_video_devices_table(lc);
	if (devid_copy != NULL) {
		linphone_core_set_video_device(lc, devid_copy);
		ms_free(devid_copy);
	}
}

char linphone_core_get_sound_source(LinphoneCore *lc)
{
	return lc->sound_conf.source;
}

void linphone_core_set_sound_source(LinphoneCore *lc, char source)
{
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


/**
 * Sets the path to a wav file used for ringing.
 *
 * @param path The file must be a wav 16bit linear. Local ring is disabled if null
 * @param lc The LinphoneCore object
 *
 * @ingroup media_parameters
**/
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

/**
 * Returns the path to the wav file used for ringing.
 *
 * @param lc The LinphoneCore object
 * @ingroup media_parameters
**/
const char *linphone_core_get_ring(const LinphoneCore *lc){
	return lc->sound_conf.local_ring;
}

/**
 * Sets the path to a file or folder containing trusted root CAs (PEM format)
 *
 * @param path
 * @param lc The LinphoneCore object
 *
 * @ingroup initializing
**/
void linphone_core_set_root_ca(LinphoneCore *lc,const char *path){
	sal_set_root_ca(lc->sal, path);
	if (lc->http_verify_policy){
		belle_tls_verify_policy_set_root_ca(lc->http_verify_policy,path);
	}
	lp_config_set_string(lc->config,"sip","root_ca",path);
}

/**
 * Gets the path to a file or folder containing the trusted root CAs (PEM format)
 *
 * @param lc The LinphoneCore object
 *
 * @ingroup initializing
**/
const char *linphone_core_get_root_ca(LinphoneCore *lc){
	return lp_config_get_string(lc->config,"sip","root_ca",NULL);
}

/**
 * Specify whether the tls server certificate must be verified when connecting to a SIP/TLS server.
 *
 * @ingroup initializing
**/
void linphone_core_verify_server_certificates(LinphoneCore *lc, bool_t yesno){
	sal_verify_server_certificates(lc->sal,yesno);
	if (lc->http_verify_policy){
		belle_tls_verify_policy_set_exceptions(lc->http_verify_policy, yesno ? 0 : BELLE_TLS_VERIFY_ANY_REASON);
	}
	lp_config_set_int(lc->config,"sip","verify_server_certs",yesno);
}

/**
 * Specify whether the tls server certificate common name must be verified when connecting to a SIP/TLS server.
 * @ingroup initializing
**/
void linphone_core_verify_server_cn(LinphoneCore *lc, bool_t yesno){
	sal_verify_server_cn(lc->sal,yesno);
	if (lc->http_verify_policy){
		belle_tls_verify_policy_set_exceptions(lc->http_verify_policy, yesno ? 0 : BELLE_TLS_VERIFY_CN_MISMATCH);
	}
	lp_config_set_int(lc->config,"sip","verify_server_cn",yesno);
}

static void notify_end_of_ring(void *ud, MSFilter *f, unsigned int event, void *arg){
	LinphoneCore *lc=(LinphoneCore*)ud;
	if (event==MS_PLAYER_EOF){
		lc->preview_finished=1;
	}
}

int linphone_core_preview_ring(LinphoneCore *lc, const char *ring,LinphoneCoreCbFunc func,void * userdata)
{
	if (lc->ringstream!=0){
		ms_warning("Cannot start ring now,there's already a ring being played");
		return -1;
	}
	lc_callback_obj_init(&lc->preview_finished_cb,func,userdata);
	lc->preview_finished=0;
	if (lc->sound_conf.ring_sndcard!=NULL){
		MSSndCard *ringcard=lc->sound_conf.lsd_card ? lc->sound_conf.lsd_card : lc->sound_conf.ring_sndcard;
		lc->ringstream=ring_start_with_cb(ring,2000,ringcard,notify_end_of_ring,(void *)lc);
	}
	return 0;
}

/**
 * Sets the path to a wav file used for ringing back.
 *
 * Ringback means the ring that is heard when it's ringing at the remote party.
 * The file must be a wav 16bit linear.
 *
 * @ingroup media_parameters
**/
void linphone_core_set_ringback(LinphoneCore *lc, const char *path){
	if (lc->sound_conf.remote_ring!=0){
		ms_free(lc->sound_conf.remote_ring);
	}
	lc->sound_conf.remote_ring=path?ms_strdup(path):NULL;
}

/**
 * Returns the path to the wav file used for ringing back.
 *
 * @ingroup media_parameters
**/
const char * linphone_core_get_ringback(const LinphoneCore *lc){
	return lc->sound_conf.remote_ring;
}

/**
 * Enables or disable echo cancellation. Value is saved and used for subsequent calls.
 * This actually controls software echo cancellation. If hardware echo cancellation is available, it will be always used and activated for calls, regardless
 * of the value passed to this function.
 * When hardware echo cancellation is available, the software one is of course not activated.
 *
 * @ingroup media_parameters
**/
void linphone_core_enable_echo_cancellation(LinphoneCore *lc, bool_t val){
	lc->sound_conf.ec=val;
	if ( linphone_core_ready(lc))
		lp_config_set_int(lc->config,"sound","echocancellation",val);
}


/**
 * Returns TRUE if echo cancellation is enabled.
 *
 * @ingroup media_parameters
**/
bool_t linphone_core_echo_cancellation_enabled(LinphoneCore *lc){
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
	LinphoneCall *call;
	const MSList *list;
	const MSList *elem;

	if (linphone_core_is_in_conference(lc)){
		lc->conf_ctx.local_muted=val;
		linphone_core_mute_audio_stream(lc, lc->conf_ctx.local_participant, val);
	}
	list = linphone_core_get_calls(lc);
	for (elem = list; elem != NULL; elem = elem->next) {
		call = (LinphoneCall *)elem->data;
		call->audio_muted = val;
		linphone_core_mute_audio_stream(lc, call->audiostream, val);
	}
}

bool_t linphone_core_is_mic_muted(LinphoneCore *lc) {
	LinphoneCall *call=linphone_core_get_current_call(lc);
	if (linphone_core_is_in_conference(lc)){
		return lc->conf_ctx.local_muted;
	}else if (call==NULL){
		ms_warning("linphone_core_is_mic_muted(): No current call !");
		return FALSE;
	}
	return call->audio_muted;
}

void linphone_core_enable_mic(LinphoneCore *lc, bool_t enable) {
	linphone_core_mute_mic(lc, (enable == TRUE) ? FALSE : TRUE);
}

bool_t linphone_core_mic_enabled(LinphoneCore *lc) {
	return (linphone_core_is_mic_muted(lc) == TRUE) ? FALSE : TRUE;
}

// returns rtp transmission status for an active stream
// if audio is muted and config parameter rtp_no_xmit_on_audio_mute
// was set on then rtp transmission is also muted
bool_t linphone_core_is_rtp_muted(LinphoneCore *lc){
	LinphoneCall *call=linphone_core_get_current_call(lc);
	if (call==NULL){
		ms_warning("linphone_core_is_rtp_muted(): No current call !");
		return FALSE;
	}
	if( linphone_core_get_rtp_no_xmit_on_audio_mute(lc)){
		return call->audio_muted;
	}
	return FALSE;
}

void linphone_core_enable_agc(LinphoneCore *lc, bool_t val){
	lc->sound_conf.agc=val;
}

bool_t linphone_core_agc_enabled(const LinphoneCore *lc){
	return lc->sound_conf.agc;
}

void linphone_core_send_dtmf(LinphoneCore *lc, char dtmf)
{
	LinphoneCall *call=linphone_core_get_current_call(lc);
	linphone_call_send_dtmf(call, dtmf);
}

void linphone_core_set_stun_server(LinphoneCore *lc, const char *server){
	if (lc->net_conf.stun_server!=NULL)
		ms_free(lc->net_conf.stun_server);
	if (server)
		lc->net_conf.stun_server=ms_strdup(server);
	else lc->net_conf.stun_server=NULL;

	/* each time the stun server is changed, we must clean the resolved cached addrinfo*/
	if (lc->net_conf.stun_addrinfo){
		freeaddrinfo(lc->net_conf.stun_addrinfo);
		lc->net_conf.stun_addrinfo=NULL;
	}
	/*if a stun server is set, we must request asynchronous resolution immediately to be ready for call*/
	if (lc->net_conf.stun_server){
		linphone_core_resolve_stun_server(lc);
	}

	if (linphone_core_ready(lc))
		lp_config_set_string(lc->config,"net","stun_server",lc->net_conf.stun_server);
}

const char * linphone_core_get_stun_server(const LinphoneCore *lc){
	return lc->net_conf.stun_server;
}


bool_t linphone_core_upnp_available(){
#ifdef BUILD_UPNP
	return TRUE;
#else
	return FALSE;
#endif //BUILD_UPNP
}

LinphoneUpnpState linphone_core_get_upnp_state(const LinphoneCore *lc){
#ifdef BUILD_UPNP
	return linphone_upnp_context_get_state(lc->upnp);
#else
	return LinphoneUpnpStateNotAvailable;
#endif //BUILD_UPNP
}

const char * linphone_core_get_upnp_external_ipaddress(const LinphoneCore *lc){
#ifdef BUILD_UPNP
	return linphone_upnp_context_get_external_ipaddress(lc->upnp);
#else
	return NULL;
#endif //BUILD_UPNP
}


void linphone_core_set_nat_address(LinphoneCore *lc, const char *addr)
{
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

const char *linphone_core_get_nat_address_resolved(LinphoneCore *lc)
{
	struct sockaddr_storage ss;
	socklen_t ss_len;
	int error;
	char ipstring [INET6_ADDRSTRLEN];

	if (lc->net_conf.nat_address==NULL) return NULL;

	if (parse_hostname_to_addr (lc->net_conf.nat_address, &ss, &ss_len, 5060)<0) {
		return lc->net_conf.nat_address;
	}

	error = getnameinfo((struct sockaddr *)&ss, ss_len,
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

void linphone_core_set_firewall_policy(LinphoneCore *lc, LinphoneFirewallPolicy pol){
	const char *policy = "none";

	switch (pol) {
		default:
		case LinphonePolicyNoFirewall:
			policy = "none";
			break;
		case LinphonePolicyUseNatAddress:
			policy = "nat_address";
			break;
		case LinphonePolicyUseStun:
			policy = "stun";
			break;
		case LinphonePolicyUseIce:
			policy = "ice";
			break;
		case LinphonePolicyUseUpnp:
#ifdef BUILD_UPNP
			policy = "upnp";
#else
			ms_warning("UPNP is not available, reset firewall policy to no firewall");
			pol = LinphonePolicyNoFirewall;
			policy = "none";
#endif //BUILD_UPNP
			break;
	}
#ifdef BUILD_UPNP
	if(pol == LinphonePolicyUseUpnp) {
		if(lc->upnp == NULL) {
			lc->upnp = linphone_upnp_context_new(lc);
		}
	} else {
		if(lc->upnp != NULL) {
			linphone_upnp_context_destroy(lc->upnp);
			lc->upnp = NULL;
		}
	}
	linphone_core_enable_keep_alive(lc, (lc->sip_conf.keepalive_period > 0));
#endif //BUILD_UPNP
	switch(pol) {
	case LinphonePolicyUseUpnp:
		sal_nat_helper_enable(lc->sal, FALSE);
		sal_enable_auto_contacts(lc->sal,FALSE);
		sal_use_rport(lc->sal, FALSE);
		break;
	default:
		sal_nat_helper_enable(lc->sal, lp_config_get_int(lc->config,"net","enable_nat_helper",1));
		sal_enable_auto_contacts(lc->sal,TRUE);
		sal_use_rport(lc->sal, lp_config_get_int(lc->config,"sip","use_rport",1));
		break;
	}
	if (lc->sip_conf.contact) update_primary_contact(lc);
	if (linphone_core_ready(lc))
		lp_config_set_string(lc->config,"net","firewall_policy",policy);
}
LinphoneFirewallPolicy linphone_core_get_firewall_policy(const LinphoneCore *lc){
	const char *policy;
	policy = lp_config_get_string(lc->config, "net", "firewall_policy", NULL);
	if ((policy == NULL) || (strcmp(policy, "0") == 0))
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



/*******************************************************************************
 * Call log related functions                                                  *
 ******************************************************************************/

const MSList * linphone_core_get_call_logs(LinphoneCore *lc){
	return lc->call_logs;
}

void linphone_core_clear_call_logs(LinphoneCore *lc){
	lc->missed_calls=0;
	ms_list_for_each(lc->call_logs,(void (*)(void*))linphone_call_log_unref);
	lc->call_logs=ms_list_free(lc->call_logs);
	call_logs_write_to_config_file(lc);
}

int linphone_core_get_missed_calls_count(LinphoneCore *lc) {
	return lc->missed_calls;
}

void linphone_core_reset_missed_calls_count(LinphoneCore *lc) {
	lc->missed_calls=0;
}

void linphone_core_remove_call_log(LinphoneCore *lc, LinphoneCallLog *cl){
	lc->call_logs = ms_list_remove(lc->call_logs, cl);
	call_logs_write_to_config_file(lc);
	linphone_call_log_unref(cl);
}




static void toggle_video_preview(LinphoneCore *lc, bool_t val){
#ifdef VIDEO_ENABLED
	if (val){
		if (lc->previewstream==NULL){
			const char *display_filter=linphone_core_get_video_display_filter(lc);
			MSVideoSize vsize=lc->video_conf.preview_vsize.width!=0 ? lc->video_conf.preview_vsize : lc->video_conf.vsize;
			lc->previewstream=video_preview_new();
			video_preview_set_size(lc->previewstream,vsize);
			if (display_filter)
				video_preview_set_display_filter_name(lc->previewstream,display_filter);
			if (lc->preview_window_id != NULL)
				video_preview_set_native_window_id(lc->previewstream,lc->preview_window_id);
			video_preview_set_fps(lc->previewstream,linphone_core_get_preferred_framerate(lc));
			video_preview_start(lc->previewstream,lc->video_conf.device);
		}
	}else{
		if (lc->previewstream!=NULL){
			video_preview_stop(lc->previewstream);
			lc->previewstream=NULL;
		}
	}
#endif
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

/**
 * Sets the default policy for video.
 * This policy defines whether:
 * - video shall be initiated by default for outgoing calls
 * - video shall be accepter by default for incoming calls
 * @ingroup media_parameters
**/
void linphone_core_set_video_policy(LinphoneCore *lc, const LinphoneVideoPolicy *policy){
	lc->video_policy=*policy;
	if (linphone_core_ready(lc)){
		lp_config_set_int(lc->config,"video","automatically_initiate",policy->automatically_initiate);
		lp_config_set_int(lc->config,"video","automatically_accept",policy->automatically_accept);
	}
}

/**
 * Get the default policy for video.
 * See linphone_core_set_video_policy() for more details.
 * @ingroup media_parameters
**/
const LinphoneVideoPolicy *linphone_core_get_video_policy(LinphoneCore *lc){
	return &lc->video_policy;
}

/**
 * Controls video preview enablement.
 *
 * @ingroup media_parameters
 * Video preview refers to the action of displaying the local webcam image
 * to the user while not in call.
**/
void linphone_core_enable_video_preview(LinphoneCore *lc, bool_t val){
	lc->video_conf.show_local=val;
	if (linphone_core_ready(lc))
		lp_config_set_int(lc->config,"video","show_local",val);
}

/**
 * Returns TRUE if video previewing is enabled.
 * @ingroup media_parameters
**/
bool_t linphone_core_video_preview_enabled(const LinphoneCore *lc){
	return lc->video_conf.show_local;
}

/**
 * Enables or disable self view during calls.
 *
 * @ingroup media_parameters
 * Self-view refers to having local webcam image inserted in corner
 * of the video window during calls.
 * This function works at any time, including during calls.
**/
void linphone_core_enable_self_view(LinphoneCore *lc, bool_t val){
#ifdef VIDEO_ENABLED
	LinphoneCall *call=linphone_core_get_current_call (lc);
	lc->video_conf.selfview=val;
	if (linphone_core_ready(lc)) {
		lp_config_set_int(lc->config,"video","self_view",linphone_core_self_view_enabled(lc));
	}
	if (call && call->videostream){
		video_stream_enable_self_view(call->videostream,val);
	}
	if (linphone_core_ready(lc)){
		lp_config_set_int(lc->config,"video","self_view",val);
	}
#endif
}

/**
 * Returns TRUE if self-view is enabled, FALSE otherwise.
 *
 * @ingroup media_parameters
 *
 * Refer to linphone_core_enable_self_view() for details.
**/
bool_t linphone_core_self_view_enabled(const LinphoneCore *lc){
	return lc->video_conf.selfview;
}

/**
 * Sets the active video device.
 *
 * @ingroup media_parameters
 * @param lc The LinphoneCore object
 * @param id the name of the video device as returned by linphone_core_get_video_devices()
**/
int linphone_core_set_video_device(LinphoneCore *lc, const char *id){
	MSWebCam *olddev=lc->video_conf.device;
	const char *vd;
	if (id!=NULL){
		lc->video_conf.device=ms_web_cam_manager_get_cam(ms_web_cam_manager_get(),id);
		if (lc->video_conf.device==NULL){
			ms_warning("Could not find video device %s",id);
		}
	}
	if (lc->video_conf.device==NULL)
		lc->video_conf.device=ms_web_cam_manager_get_default_cam(ms_web_cam_manager_get());
	if (olddev!=NULL && olddev!=lc->video_conf.device){
		toggle_video_preview(lc,FALSE);/*restart the video local preview*/
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

/**
 * Returns the name of the currently active video device.
 *
 * @param lc The LinphoneCore object
 * @ingroup media_parameters
**/
const char *linphone_core_get_video_device(const LinphoneCore *lc){
	if (lc->video_conf.device) return ms_web_cam_get_string_id(lc->video_conf.device);
	return NULL;
}

#ifdef VIDEO_ENABLED
static VideoStream * get_active_video_stream(LinphoneCore *lc){
	VideoStream *vs = NULL;
	LinphoneCall *call=linphone_core_get_current_call (lc);
	/* Select the video stream from the call in the first place */
	if (call && call->videostream) {
		vs = call->videostream;
	}
	/* If not in call, select the video stream from the preview */
	if (vs == NULL && lc->previewstream) {
		vs = lc->previewstream;
	}
	return vs;
}
#endif

int linphone_core_set_static_picture(LinphoneCore *lc, const char *path) {
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

int linphone_core_set_static_picture_fps(LinphoneCore *lc, float fps) {
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

/**
 * Returns the native window handle of the video window, casted as an unsigned long.
 *
 * @ingroup media_parameters
**/
void * linphone_core_get_native_video_window_id(const LinphoneCore *lc){
	if (lc->video_window_id) {
		/* case where the video id was previously set by the app*/
		return lc->video_window_id;
	}else{
#ifdef VIDEO_ENABLED
		/*case where it was not set but we want to get the one automatically created by mediastreamer2 (desktop versions only)*/
		LinphoneCall *call=linphone_core_get_current_call (lc);
		if (call && call->videostream)
			return video_stream_get_native_window_id(call->videostream);
#endif
	}
	return 0;
}

/* unsets the video id for all calls (indeed it may be kept by filters or videostream object itself by paused calls)*/
static void unset_video_window_id(LinphoneCore *lc, bool_t preview, void *id){
#ifdef VIDEO_ENABLED
	LinphoneCall *call;
	MSList *elem;
#endif

	if ((id != NULL)
#ifndef _WIN32
		&& ((unsigned long)id != (unsigned long)-1)
#endif
	){
		ms_error("Invalid use of unset_video_window_id()");
		return;
	}
#ifdef VIDEO_ENABLED
	for(elem=lc->calls;elem!=NULL;elem=elem->next){
		call=(LinphoneCall *) elem->data;
		if (call->videostream){
			if (preview)
				video_stream_set_native_preview_window_id(call->videostream,id);
			else
				video_stream_set_native_window_id(call->videostream,id);
		}
	}
#endif
}

void linphone_core_set_native_video_window_id(LinphoneCore *lc, void *id){
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
		if (call!=NULL && call->videostream){
			video_stream_set_native_window_id(call->videostream,id);
		}
	}
#endif
}

/**
 * Returns the native window handle of the video preview window, casted as an unsigned long.
 *
 * @ingroup media_parameters
**/
void * linphone_core_get_native_preview_window_id(const LinphoneCore *lc){
	if (lc->preview_window_id){
		/*case where the id was set by the app previously*/
		return lc->preview_window_id;
	}else{
		/*case where we want the id automatically created by mediastreamer2 (desktop versions only)*/
#ifdef VIDEO_ENABLED
		LinphoneCall *call=linphone_core_get_current_call(lc);
		if (call && call->videostream)
			return video_stream_get_native_preview_window_id(call->videostream);
		if (lc->previewstream)
			return video_preview_get_native_window_id(lc->previewstream);
#endif
	}
	return 0;
}

/**
 * @ingroup media_parameters
 * Set the native window id where the preview video (local camera) is to be displayed.
 * This has to be used in conjonction with linphone_core_use_preview_window().
 * MacOS, Linux, Windows: if not set or zero the core will create its own window, unless the special id -1 is given.
**/
void linphone_core_set_native_preview_window_id(LinphoneCore *lc, void *id){
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
		if (call!=NULL && call->videostream){
			video_stream_set_native_preview_window_id(call->videostream,id);
		}else if (lc->previewstream){
			video_preview_set_native_window_id(lc->previewstream,id);
		}
	}
#endif
}

/**
 * Can be used to disable video showing to free XV port
**/
void linphone_core_show_video(LinphoneCore *lc, bool_t show){
#ifdef VIDEO_ENABLED
	LinphoneCall *call=linphone_core_get_current_call(lc);
	ms_error("linphone_core_show_video %d", show);
	if (call!=NULL && call->videostream){
		video_stream_show_video(call->videostream,show);
	}
#endif
}

void linphone_core_use_preview_window(LinphoneCore *lc, bool_t yesno){
	lc->use_preview_window=yesno;
}
/**
 * @ingroup media_parameters
 *returns current device orientation
 */
int linphone_core_get_device_rotation(LinphoneCore *lc ) {
	return lc->device_rotation;
}
/**
 * @ingroup media_parameters
 * Tells the core the device current orientation. This can be used by capture filters
 * on mobile devices to select between portrait/landscape mode and to produce properly
 * oriented images. The exact meaning of the value in rotation if left to each device
 * specific implementations.
 *@param lc  object.
 *@param rotation . IOS supported values are 0 for UIInterfaceOrientationPortrait and 270 for UIInterfaceOrientationLandscapeRight.
 *
**/
void linphone_core_set_device_rotation(LinphoneCore *lc, int rotation) {
	if (rotation!=lc->device_rotation) ms_message("%s : rotation=%d\n", __FUNCTION__, rotation);
	lc->device_rotation = rotation;
#ifdef VIDEO_ENABLED
	{
		LinphoneCall *call=linphone_core_get_current_call(lc);
		if (call!=NULL && call->videostream){
			video_stream_set_device_rotation(call->videostream,rotation);
		}
	}
#endif
}

int linphone_core_get_camera_sensor_rotation(LinphoneCore *lc) {
#ifdef VIDEO_ENABLED
	LinphoneCall *call = linphone_core_get_current_call(lc);
	if ((call != NULL) && (call->videostream != NULL)) {
		return video_stream_get_camera_sensor_rotation(call->videostream);
	}
#endif
	return -1;
}

static MSVideoSizeDef supported_resolutions[]={
#if !ANDROID && !TARGET_OS_IPHONE
	{	{ MS_VIDEO_SIZE_1080P_W, MS_VIDEO_SIZE_1080P_H }	,	"1080p"	},
#endif
#if !ANDROID && !TARGET_OS_MAC /*limit to most common sizes because mac video API cannot list supported resolutions*/
	{	{ MS_VIDEO_SIZE_UXGA_W, MS_VIDEO_SIZE_UXGA_H }	,	"uxga"	},
	{	{ MS_VIDEO_SIZE_SXGA_MINUS_W, MS_VIDEO_SIZE_SXGA_MINUS_H }	,	"sxga-"	},
#endif
	{	{ MS_VIDEO_SIZE_720P_W, MS_VIDEO_SIZE_720P_H }	,	"720p"	},
#if !ANDROID && !TARGET_OS_MAC
	{	{ MS_VIDEO_SIZE_XGA_W, MS_VIDEO_SIZE_XGA_H }	,	"xga"	},
#endif
#if !ANDROID && !TARGET_OS_IPHONE
	{	{ MS_VIDEO_SIZE_SVGA_W, MS_VIDEO_SIZE_SVGA_H }	,	"svga"	},
	{	{ MS_VIDEO_SIZE_4CIF_W, MS_VIDEO_SIZE_4CIF_H }	,	"4cif"	},
#endif

	{	{ MS_VIDEO_SIZE_VGA_W, MS_VIDEO_SIZE_VGA_H }	,	"vga"	},
#if TARGET_OS_IPHONE
	{	{ MS_VIDEO_SIZE_IOS_MEDIUM_H, MS_VIDEO_SIZE_IOS_MEDIUM_W }	,	"ios-medium"	},
#endif
	{	{ MS_VIDEO_SIZE_CIF_W, MS_VIDEO_SIZE_CIF_H }	,	"cif"	},
#if !TARGET_OS_MAC || TARGET_OS_IPHONE /* OS_MAC is 1 for iPhone, but we need QVGA */
	{	{ MS_VIDEO_SIZE_QVGA_W, MS_VIDEO_SIZE_QVGA_H }	,	"qvga"	},
#endif
	{	{ MS_VIDEO_SIZE_QCIF_W, MS_VIDEO_SIZE_QCIF_H }	,	"qcif"	},
	{	{ 0,0 }			,	NULL	}
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

/* warning: function not reentrant*/
static const char *video_size_get_name(MSVideoSize vsize){
	MSVideoSizeDef *pdef=supported_resolutions;
	static char customsize[64]={0};
	for(;pdef->name!=NULL;pdef++){
		if (pdef->vsize.width==vsize.width && pdef->vsize.height==vsize.height){
			return pdef->name;
		}
	}
	if (vsize.width && vsize.height){
		snprintf(customsize,sizeof(customsize)-1,"%ix%i",vsize.width,vsize.height);
		return customsize;
	}
	return NULL;
}

static bool_t video_size_supported(MSVideoSize vsize){
	if (video_size_get_name(vsize)) return TRUE;
	ms_warning("Video resolution %ix%i is not supported in linphone.",vsize.width,vsize.height);
	return FALSE;
}

static void update_preview_size(LinphoneCore *lc, MSVideoSize oldvsize, MSVideoSize vsize){
	if (!ms_video_size_equal(oldvsize,vsize) && lc->previewstream!=NULL){
		toggle_video_preview(lc,FALSE);
		toggle_video_preview(lc,TRUE);
	}
}

void linphone_core_set_preferred_video_size(LinphoneCore *lc, MSVideoSize vsize){
	if (video_size_supported(vsize)){
		MSVideoSize oldvsize=lc->video_conf.preview_vsize;

		if (oldvsize.width==0){
			oldvsize=lc->video_conf.vsize;
		}
		lc->video_conf.vsize=vsize;
		update_preview_size(lc,oldvsize,vsize);

		if (linphone_core_ready(lc))
			lp_config_set_string(lc->config,"video","size",video_size_get_name(vsize));
	}
}

void linphone_core_set_preview_video_size(LinphoneCore *lc, MSVideoSize vsize){
	MSVideoSize oldvsize;
	if (vsize.width==0 && vsize.height==0){
		/*special case to reset the forced preview size mode*/
		lc->video_conf.preview_vsize=vsize;
		if (linphone_core_ready(lc))
			lp_config_set_string(lc->config,"video","preview_size",NULL);
		return;
	}
	oldvsize=lc->video_conf.preview_vsize;
	lc->video_conf.preview_vsize=vsize;
	if (!ms_video_size_equal(oldvsize,vsize) && lc->previewstream!=NULL){
		toggle_video_preview(lc,FALSE);
		toggle_video_preview(lc,TRUE);
	}
	if (linphone_core_ready(lc))
		lp_config_set_string(lc->config,"video","preview_size",video_size_get_name(vsize));
}

MSVideoSize linphone_core_get_preview_video_size(const LinphoneCore *lc){
	return lc->video_conf.preview_vsize;
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

void linphone_core_set_preview_video_size_by_name(LinphoneCore *lc, const char *name){
	MSVideoSize vsize=video_size_get_by_name(name);
	linphone_core_set_preview_video_size(lc,vsize);
}

void linphone_core_set_preferred_video_size_by_name(LinphoneCore *lc, const char *name){
	MSVideoSize vsize=video_size_get_by_name(name);
	MSVideoSize default_vsize={MS_VIDEO_SIZE_CIF_W,MS_VIDEO_SIZE_CIF_H};
	if (vsize.width!=0)	linphone_core_set_preferred_video_size(lc,vsize);
	else linphone_core_set_preferred_video_size(lc,default_vsize);
}

MSVideoSize linphone_core_get_preferred_video_size(const LinphoneCore *lc){
	return lc->video_conf.vsize;
}

char * linphone_core_get_preferred_video_size_name(const LinphoneCore *lc) {
	return ms_strdup(video_size_get_name(lc->video_conf.vsize));
}

void linphone_core_set_preferred_framerate(LinphoneCore *lc, float fps){
	lc->video_conf.fps=fps;
	if (linphone_core_ready(lc))
		lp_config_set_float(lc->config,"video","framerate",fps);
}

float linphone_core_get_preferred_framerate(LinphoneCore *lc){
	return lc->video_conf.fps;
}


void linphone_core_set_use_files(LinphoneCore *lc, bool_t yesno){
	lc->use_files=yesno;
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
		if (call && call->audiostream && call->audiostream->ms.state==MSStreamStarted)
			audio_stream_play(call->audiostream,file);
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
		if (call && call->audiostream)
			audio_stream_record(call->audiostream,file);
	}
}

typedef enum{
	LinphoneToneGenerator,
	LinphoneLocalPlayer
}LinphoneAudioResourceType;

static MSFilter *get_audio_resource(LinphoneCore *lc, LinphoneAudioResourceType rtype){
	LinphoneCall *call=linphone_core_get_current_call(lc);
	AudioStream *stream=NULL;
	RingStream *ringstream;
	if (call){
		stream=call->audiostream;
	}else if (linphone_core_is_in_conference(lc)){
		stream=lc->conf_ctx.local_participant;
	}
	if (stream){
		if (rtype==LinphoneToneGenerator) return stream->dtmfgen;
		if (rtype==LinphoneLocalPlayer) return stream->local_player;
		return NULL;
	}
	if (lc->ringstream==NULL){
		float amp=lp_config_get_float(lc->config,"sound","dtmf_player_amp",0.1);
		MSSndCard *ringcard=lc->sound_conf.lsd_card ?lc->sound_conf.lsd_card : lc->sound_conf.ring_sndcard;
		if (ringcard == NULL)
			return NULL;

		ringstream=lc->ringstream=ring_start(NULL,0,ringcard);
		ms_filter_call_method(lc->ringstream->gendtmf,MS_DTMF_GEN_SET_DEFAULT_AMPLITUDE,&amp);
		lc->dmfs_playing_start_time=time(NULL);
	}else{
		ringstream=lc->ringstream;
		if (lc->dmfs_playing_start_time!=0)
			lc->dmfs_playing_start_time=time(NULL);
	}
	if (rtype==LinphoneToneGenerator) return ringstream->gendtmf;
	if (rtype==LinphoneLocalPlayer) return ringstream->source;
	return NULL;
}

static MSFilter *get_dtmf_gen(LinphoneCore *lc){
	return get_audio_resource(lc,LinphoneToneGenerator);
}

/**
 * @ingroup media_parameters
 * Plays a dtmf sound to the local user.
 * @param lc #LinphoneCore
 * @param dtmf DTMF to play ['0'..'16'] | '#' | '#'
 * @param duration_ms duration in ms, -1 means play until next further call to #linphone_core_stop_dtmf()
**/
void linphone_core_play_dtmf(LinphoneCore *lc, char dtmf, int duration_ms){
	MSFilter *f=get_dtmf_gen(lc);
	if (f==NULL){
		ms_error("No dtmf generator at this time !");
		return;
	}

	if (duration_ms > 0)
		ms_filter_call_method(f, MS_DTMF_GEN_PLAY, &dtmf);
	else ms_filter_call_method(f, MS_DTMF_GEN_START, &dtmf);
}

/**
 * Plays an audio file to the local user.
 * This function works at any time, during calls, or when no calls are running.
 * It doesn't request the underlying audio system to support multiple playback streams.
 * @param lc the linphone core
 * @param audiofile path to audio file in wav PCM 16 bit format.
 * @ingroup misc
**/
int linphone_core_play_local(LinphoneCore *lc, const char *audiofile){
	MSFilter *f=get_audio_resource(lc,LinphoneLocalPlayer);
	int loopms=-1;
	if (!f) return -1;
	ms_filter_call_method(f,MS_PLAYER_SET_LOOP,&loopms);
	if (ms_filter_call_method(f,MS_PLAYER_OPEN,(void*)audiofile)!=0){
		return -1;
	}
	ms_filter_call_method_noarg(f,MS_PLAYER_START);
	return 0;
}

void linphone_core_play_named_tone(LinphoneCore *lc, LinphoneToneID toneid){
	if (linphone_core_tone_indications_enabled(lc)){
		const char *audiofile=linphone_core_get_tone_file(lc,toneid);
		if (!audiofile){
			MSFilter *f=get_dtmf_gen(lc);
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
			linphone_core_play_local(lc,audiofile);
		}
	}
}

void linphone_core_play_call_error_tone(LinphoneCore *lc, LinphoneReason reason){
	if (linphone_core_tone_indications_enabled(lc)){
		LinphoneToneDescription *tone=linphone_core_get_call_error_tone(lc,reason);
		if (tone){
			if (tone->audiofile){
				linphone_core_play_local(lc,tone->audiofile);
			}else if (tone->toneid!=LinphoneToneUndefined){
				linphone_core_play_named_tone(lc,tone->toneid);
			}
		}
	}
}

/**
 * @ingroup media_parameters
 *
 * Stops playing a dtmf started by linphone_core_play_dtmf().
**/
void linphone_core_stop_dtmf(LinphoneCore *lc){
	MSFilter *f=get_dtmf_gen(lc);
	if (f!=NULL)
		ms_filter_call_method_noarg (f, MS_DTMF_GEN_STOP);
}



/**
 * Retrieves the user pointer that was given to linphone_core_new()
 *
 * @ingroup initializing
**/
void *linphone_core_get_user_data(const LinphoneCore *lc){
	return lc->data;
}


/**
 * Associate a user pointer to the linphone core.
 *
 * @ingroup initializing
**/
void linphone_core_set_user_data(LinphoneCore *lc, void *userdata){
	lc->data=userdata;
}

int linphone_core_get_mtu(const LinphoneCore *lc){
	return lc->net_conf.mtu;
}

/**
 * Sets the maximum transmission unit size in bytes.
 * This information is useful for sending RTP packets.
 * Default value is 1500.
 *
 * @ingroup media_parameters
**/
void linphone_core_set_mtu(LinphoneCore *lc, int mtu){
	lc->net_conf.mtu=mtu;
	if (mtu>0){
		if (mtu<500){
			ms_error("MTU too small !");
			mtu=500;
		}
		ms_set_mtu(mtu);
		ms_message("MTU is supposed to be %i, rtp payload max size will be %i",mtu, ms_get_payload_max_size());
	}else ms_set_mtu(0);//use mediastreamer2 default value
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

/**
 * Retrieve RTP statistics regarding current call.
 * @param lc the LinphoneCore
 * @param local RTP statistics computed locally.
 * @param remote RTP statistics computed by far end (obtained via RTCP feedback).
 *
 * @note Remote RTP statistics is not implemented yet.
 *
 * @return 0 or -1 if no call is running.
**/

int linphone_core_get_current_call_stats(LinphoneCore *lc, rtp_stats_t *local, rtp_stats_t *remote){
	LinphoneCall *call=linphone_core_get_current_call (lc);
	if (call!=NULL){
		if (call->audiostream!=NULL){
			memset(remote,0,sizeof(*remote));
			audio_stream_get_local_rtp_stats (call->audiostream,local);
			return 0;
		}
	}
	return -1;
}

void net_config_uninit(LinphoneCore *lc)
{
	net_config_t *config=&lc->net_conf;

	if (config->stun_server!=NULL){
		ms_free(config->stun_server);
	}
	if (config->stun_addrinfo){
		freeaddrinfo(config->stun_addrinfo);
		config->stun_addrinfo=NULL;
	}
	if (config->nat_address!=NULL){
		lp_config_set_string(lc->config,"net","nat_address",config->nat_address);
		ms_free(lc->net_conf.nat_address);
	}
	if (lc->net_conf.nat_address_ip !=NULL){
		ms_free(lc->net_conf.nat_address_ip);
	}
	lp_config_set_int(lc->config,"net","mtu",config->mtu);
}


void sip_config_uninit(LinphoneCore *lc)
{
	MSList *elem;
	int i;
	sip_config_t *config=&lc->sip_conf;
	bool_t still_registered=TRUE;

	lp_config_set_int(lc->config,"sip","guess_hostname",config->guess_hostname);
	lp_config_set_string(lc->config,"sip","contact",config->contact);
	lp_config_set_int(lc->config,"sip","inc_timeout",config->inc_timeout);
	lp_config_set_int(lc->config,"sip","in_call_timeout",config->in_call_timeout);
	lp_config_set_int(lc->config,"sip","delayed_timeout",config->delayed_timeout);
	lp_config_set_int(lc->config,"sip","use_ipv6",config->ipv6_enabled);
	lp_config_set_int(lc->config,"sip","register_only_when_network_is_up",config->register_only_when_network_is_up);
	lp_config_set_int(lc->config,"sip","register_only_when_upnp_is_ok",config->register_only_when_upnp_is_ok);

	if (lc->network_reachable) {
		for(elem=config->proxies;elem!=NULL;elem=ms_list_next(elem)){
			LinphoneProxyConfig *cfg=(LinphoneProxyConfig*)(elem->data);
			_linphone_proxy_config_unregister(cfg);	/* to unregister without changing the stored flag enable_register */
		}

		ms_message("Unregistration started.");

		for (i=0;i<20&&still_registered;i++){
			still_registered=FALSE;
			sal_iterate(lc->sal);
			for(elem=config->proxies;elem!=NULL;elem=ms_list_next(elem)){
				LinphoneProxyConfig *cfg=(LinphoneProxyConfig*)(elem->data);
				LinphoneRegistrationState state = linphone_proxy_config_get_state(cfg);
				still_registered|=(state==LinphoneRegistrationOk||state==LinphoneRegistrationProgress);
			}
			ms_usleep(100000);
		}
		if (i>=20) ms_warning("Cannot complete unregistration, giving up");
	}
	config->proxies=ms_list_free_with_data(config->proxies,(void (*)(void*)) _linphone_proxy_config_release);

	config->deleted_proxies=ms_list_free_with_data(config->deleted_proxies,(void (*)(void*)) _linphone_proxy_config_release);

	/*no longuer need to write proxy config if not changedlinphone_proxy_config_write_to_config_file(lc->config,NULL,i);*/	/*mark the end */

	lc->auth_info=ms_list_free_with_data(lc->auth_info,(void (*)(void*))linphone_auth_info_destroy);

	/*now that we are unregisted, we no longer need the tunnel.*/
#ifdef TUNNEL_ENABLED
	if (lc->tunnel) {
		linphone_tunnel_destroy(lc->tunnel);
		lc->tunnel=NULL;
		ms_message("Tunnel destroyed.");
	}
#endif

	sal_reset_transports(lc->sal);
	sal_unlisten_ports(lc->sal); /*to make sure no new messages are received*/
	if (lc->http_provider) {
		belle_sip_object_unref(lc->http_provider);
		lc->http_provider=NULL;
	}
	if (lc->http_verify_policy){
		belle_sip_object_unref(lc->http_verify_policy);
		lc->http_verify_policy=NULL;
	}
	sal_iterate(lc->sal); /*make sure event are purged*/
	sal_uninit(lc->sal);
	lc->sal=NULL;

	if (lc->sip_conf.guessed_contact)
		ms_free(lc->sip_conf.guessed_contact);
	if (config->contact)
		ms_free(config->contact);

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
	lp_config_set_int(lc->config,"rtp","audio_jitt_comp",config->audio_jitt_comp);
	lp_config_set_int(lc->config,"rtp","video_jitt_comp",config->video_jitt_comp);
	lp_config_set_int(lc->config,"rtp","nortp_timeout",config->nortp_timeout);
	lp_config_set_int(lc->config,"rtp","audio_adaptive_jitt_comp_enabled",config->audio_adaptive_jitt_comp_enabled);
	lp_config_set_int(lc->config,"rtp","video_adaptive_jitt_comp_enabled",config->video_adaptive_jitt_comp_enabled);
	ms_free(lc->rtp_conf.audio_multicast_addr);
	ms_free(lc->rtp_conf.video_multicast_addr);
	ms_free(config->srtp_suites);
}

static void sound_config_uninit(LinphoneCore *lc)
{
	sound_config_t *config=&lc->sound_conf;
	ms_free(config->cards);

	lp_config_set_string(lc->config,"sound","remote_ring",config->remote_ring);
	lp_config_set_float(lc->config,"sound","playback_gain_db",config->soft_play_lev);
	lp_config_set_float(lc->config,"sound","mic_gain_db",config->soft_mic_lev);

	if (config->local_ring) ms_free(config->local_ring);
	if (config->remote_ring) ms_free(config->remote_ring);
	lc->tones=ms_list_free_with_data(lc->tones, (void (*)(void*))linphone_tone_description_destroy);
}

static void video_config_uninit(LinphoneCore *lc)
{
	lp_config_set_string(lc->config,"video","size",video_size_get_name(linphone_core_get_preferred_video_size(lc)));
	lp_config_set_int(lc->config,"video","display",lc->video_conf.display);
	lp_config_set_int(lc->config,"video","capture",lc->video_conf.capture);
	if (lc->video_conf.cams)
		ms_free(lc->video_conf.cams);
}

void _linphone_core_codec_config_write(LinphoneCore *lc){
	if (linphone_core_ready(lc)){
		PayloadType *pt;
		codecs_config_t *config=&lc->codecs_conf;
		MSList *node;
		char key[50];
		int index;
		index=0;
		for(node=config->audio_codecs;node!=NULL;node=ms_list_next(node)){
			pt=(PayloadType*)(node->data);
			sprintf(key,"audio_codec_%i",index);
			lp_config_set_string(lc->config,key,"mime",pt->mime_type);
			lp_config_set_int(lc->config,key,"rate",pt->clock_rate);
			lp_config_set_int(lc->config,key,"channels",pt->channels);
			lp_config_set_int(lc->config,key,"enabled",linphone_core_payload_type_enabled(lc,pt));
			index++;
		}
		sprintf(key,"audio_codec_%i",index);
		lp_config_clean_section (lc->config,key);

		index=0;
		for(node=config->video_codecs;node!=NULL;node=ms_list_next(node)){
			pt=(PayloadType*)(node->data);
			sprintf(key,"video_codec_%i",index);
			lp_config_set_string(lc->config,key,"mime",pt->mime_type);
			lp_config_set_int(lc->config,key,"rate",pt->clock_rate);
			lp_config_set_int(lc->config,key,"enabled",linphone_core_payload_type_enabled(lc,pt));
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
	ms_list_free_with_data(lc->codecs_conf.audio_codecs, (void (*)(void*))payload_type_destroy);
	ms_list_free_with_data(lc->codecs_conf.video_codecs, (void (*)(void*))payload_type_destroy);
}

void ui_config_uninit(LinphoneCore* lc)
{
	ms_message("Destroying friends.");
	if (lc->friends){
		ms_list_for_each(lc->friends,(void (*)(void *))linphone_friend_unref);
		ms_list_free(lc->friends);
		lc->friends=NULL;
	}
	if (lc->presence_model) {
		linphone_presence_model_unref(lc->presence_model);
		lc->presence_model = NULL;
	}
	ms_message("Destroying friends done.");
}

/**
 * Returns the LpConfig object used to manage the storage (config) file.
 *
 * @ingroup misc
 * The application can use the LpConfig object to insert its own private
 * sections and pairs of key=value in the configuration file.
 *
**/
LpConfig * linphone_core_get_config(LinphoneCore *lc){
	return lc->config;
}

LpConfig * linphone_core_create_lp_config(LinphoneCore *lc, const char *filename) {
	return lp_config_new(filename);
}

static void linphone_core_uninit(LinphoneCore *lc)
{
	linphone_core_free_hooks(lc);
	lc->video_conf.show_local = FALSE;

	while(lc->calls)
	{
		LinphoneCall *the_call = lc->calls->data;
		linphone_core_terminate_call(lc,the_call);
		linphone_core_iterate(lc);
		ms_usleep(50000);
	}

	if (lc->friends) /* FIXME we should wait until subscription to complete*/
		ms_list_for_each(lc->friends,(void (*)(void *))linphone_friend_close_subscriptions);
	linphone_core_set_state(lc,LinphoneGlobalShutdown,"Shutting down");
#ifdef VIDEO_ENABLED
	if (lc->previewstream!=NULL){
		video_preview_stop(lc->previewstream);
		lc->previewstream=NULL;
	}
#endif

	lc->msevq=NULL;
	/* save all config */
	ui_config_uninit(lc);
	sip_config_uninit(lc);
	net_config_uninit(lc);
	rtp_config_uninit(lc);
	linphone_core_stop_ringing(lc);
	sound_config_uninit(lc);
	video_config_uninit(lc);
	codecs_config_uninit(lc);

	sip_setup_unregister_all();

#ifdef BUILD_UPNP
	if(lc->upnp != NULL) {
		linphone_upnp_context_destroy(lc->upnp);
		lc->upnp = NULL;
	}
#endif //BUILD_UPNP

	ms_list_for_each(lc->chatrooms, (MSIterateFunc)linphone_chat_room_release);
	lc->chatrooms = ms_list_free(lc->chatrooms);

	if (lp_config_needs_commit(lc->config)) lp_config_sync(lc->config);
	lp_config_destroy(lc->config);
	lc->config = NULL; /* Mark the config as NULL to block further calls */

	ms_list_for_each(lc->call_logs,(void (*)(void*))linphone_call_log_unref);
	lc->call_logs=ms_list_free(lc->call_logs);

	ms_list_for_each(lc->last_recv_msg_ids,ms_free);
	lc->last_recv_msg_ids=ms_list_free(lc->last_recv_msg_ids);

	if(lc->zrtp_secrets_cache != NULL) {
		ms_free(lc->zrtp_secrets_cache);
	}
	if(lc->user_certificates_path != NULL) {
		ms_free(lc->user_certificates_path);
	}
	if(lc->play_file!=NULL){
		ms_free(lc->play_file);
	}
	if(lc->rec_file!=NULL){
		ms_free(lc->rec_file);
	}
	if (lc->chat_db_file){
		ms_free(lc->chat_db_file);
	}
	if(lc->presence_model){
		linphone_presence_model_unref(lc->presence_model);
	}
	linphone_core_free_payload_types(lc);
	if (lc->supported_formats) ms_free(lc->supported_formats);
	linphone_core_message_storage_close(lc);
	ms_exit();
	linphone_core_set_state(lc,LinphoneGlobalOff,"Off");
	linphone_core_deactivate_log_serialization_if_needed();
	ms_list_free_with_data(lc->vtable_refs,(void (*)(void *))v_table_reference_destroy);
}

static void set_network_reachable(LinphoneCore* lc,bool_t isReachable, time_t curtime){
	// second get the list of available proxies
	const MSList *elem=linphone_core_get_proxy_config_list(lc);

	if (lc->network_reachable==isReachable) return; // no change, ignore.
	lc->network_reachable_to_be_notified=TRUE;
	ms_message("Network state is now [%s]",isReachable?"UP":"DOWN");
	for(;elem!=NULL;elem=elem->next){
		LinphoneProxyConfig *cfg=(LinphoneProxyConfig*)elem->data;
		if (linphone_proxy_config_register_enabled(cfg) ) {
			if (!isReachable) {
				linphone_proxy_config_stop_refreshing(cfg);
				linphone_proxy_config_set_state(cfg, LinphoneRegistrationNone,"Registration impossible (network down)");
			}else{
				cfg->commit=TRUE;
			}
		}
	}
	lc->netup_time=curtime;
	lc->network_reachable=isReachable;

	if (!lc->network_reachable){
		linphone_core_invalidate_friend_subscriptions(lc);
		sal_reset_transports(lc->sal);
	}else{
		linphone_core_resolve_stun_server(lc);
	}
#ifdef BUILD_UPNP
	if(lc->upnp == NULL) {
		if(isReachable && linphone_core_get_firewall_policy(lc) == LinphonePolicyUseUpnp) {
			lc->upnp = linphone_upnp_context_new(lc);
		}
	} else {
		if(!isReachable && linphone_core_get_firewall_policy(lc) == LinphonePolicyUseUpnp) {
			linphone_upnp_context_destroy(lc->upnp);
			lc->upnp = NULL;
		}
	}
#endif
}

void linphone_core_refresh_registers(LinphoneCore* lc) {
	const MSList *elem;
	if (!lc->network_reachable) {
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

void __linphone_core_invalidate_registers(LinphoneCore* lc){
	const MSList *elem=linphone_core_get_proxy_config_list(lc);
	for(;elem!=NULL;elem=elem->next){
		LinphoneProxyConfig *cfg=(LinphoneProxyConfig*)elem->data;
		if (linphone_proxy_config_register_enabled(cfg)) {
			linphone_proxy_config_edit(cfg);
			linphone_proxy_config_done(cfg);
		}
	}
}

void linphone_core_set_network_reachable(LinphoneCore* lc,bool_t isReachable) {
	//first disable automatic mode
	if (lc->auto_net_state_mon) {
		ms_message("Disabling automatic network state monitoring");
		lc->auto_net_state_mon=FALSE;
	}
	set_network_reachable(lc,isReachable, ms_time(NULL));
}

bool_t linphone_core_is_network_reachable(LinphoneCore* lc) {
	return lc->network_reachable;
}
ortp_socket_t linphone_core_get_sip_socket(LinphoneCore *lc){
	return sal_get_socket(lc->sal);
}
/**
 * Destroys a LinphoneCore
 *
 * @ingroup initializing
**/
void linphone_core_destroy(LinphoneCore *lc){
	linphone_core_uninit(lc);
	ms_free(lc);
}
/**
 * Get the number of Call
 *
 * @ingroup call_control
**/
int linphone_core_get_calls_nb(const LinphoneCore *lc){
	return  ms_list_size(lc->calls);;
}

/**
 * Check if we do not have exceed the number of simultaneous call
 *
 * @ingroup call_control
**/
bool_t linphone_core_can_we_add_call(LinphoneCore *lc)
{
	if(linphone_core_get_calls_nb(lc) < lc->max_calls)
		return TRUE;
	ms_message("Maximum amount of simultaneous calls reached !");
	return FALSE;
}

static void notify_soundcard_usage(LinphoneCore *lc, bool_t used){
	MSSndCard *card=lc->sound_conf.capt_sndcard;
	if (card && ms_snd_card_get_capabilities(card) & MS_SND_CARD_CAP_IS_SLOW){
		ms_snd_card_set_usage_hint(card,used);
	}
}

void linphone_core_soundcard_hint_check( LinphoneCore* lc){
	MSList* the_calls = lc->calls;
	LinphoneCall* call = NULL;
	bool_t dont_need_sound = TRUE;
	bool_t use_rtp_io = lp_config_get_int(lc->config, "sound", "rtp_io", FALSE);

	/* check if the remaining calls are paused */
	while( the_calls ){
		call = the_calls->data;
		if( call->state != LinphoneCallPausing && call->state != LinphoneCallPaused ){
			dont_need_sound = FALSE;
			break;
		}
		the_calls = the_calls->next;
	}

	/* if no more calls or all calls are paused, we can free the soundcard */
	if ( (lc->calls==NULL || dont_need_sound) && !lc->use_files && !use_rtp_io){
		ms_message("Notifying soundcard that we don't need it anymore for calls.");
		notify_soundcard_usage(lc,FALSE);
	}
}

int linphone_core_add_call( LinphoneCore *lc, LinphoneCall *call)
{
	if (linphone_core_can_we_add_call(lc)){
		if (lc->calls==NULL) notify_soundcard_usage(lc,TRUE);
		lc->calls = ms_list_append(lc->calls,call);
		return 0;
	}
	return -1;
}

int linphone_core_del_call( LinphoneCore *lc, LinphoneCall *call)
{
	MSList *it;
	MSList *the_calls = lc->calls;

	it=ms_list_find(the_calls,call);
	if (it)
	{
		the_calls = ms_list_remove_link(the_calls,it);
	}
	else
	{
		ms_warning("could not find the call into the list\n");
		return -1;
	}
	lc->calls = the_calls;

	return 0;
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

LinphonePayloadType* linphone_core_find_payload_type(LinphoneCore* lc, const char* type, int rate, int channels) {
	LinphonePayloadType* result = find_payload_type_from_list(type, rate, channels, linphone_core_get_audio_codecs(lc));
	if (result)  {
		return result;
	} else {
		result = find_payload_type_from_list(type, rate, 0, linphone_core_get_video_codecs(lc));
		if (result) {
			return result;
		}
	}
	/*not found*/
	return NULL;
}

const char* linphone_configuring_state_to_string(LinphoneConfiguringState cs){
	switch(cs){
		case LinphoneConfiguringSuccessful:
			return "LinphoneConfiguringSuccessful";
		break;
		case LinphoneConfiguringFailed:
			return "LinphoneConfiguringFailed";
		break;
		case LinphoneConfiguringSkipped:
			return "LinphoneConfiguringSkipped";
		break;
	}
	return NULL;
}

const char *linphone_global_state_to_string(LinphoneGlobalState gs){
	switch(gs){
		case LinphoneGlobalOff:
			return "LinphoneGlobalOff";
		break;
		case LinphoneGlobalOn:
			return "LinphoneGlobalOn";
		break;
		case LinphoneGlobalStartup:
			return "LinphoneGlobalStartup";
		break;
		case LinphoneGlobalShutdown:
			return "LinphoneGlobalShutdown";
		case LinphoneGlobalConfiguring:
			return "LinphoneGlobalConfiguring";
		break;
	}
	return NULL;
}

LinphoneGlobalState linphone_core_get_global_state(const LinphoneCore *lc){
	return lc->state;
}

LinphoneCallParams *linphone_core_create_default_call_parameters(LinphoneCore *lc){
	LinphoneCallParams *p=linphone_call_params_new();
	linphone_core_init_default_params(lc, p);
	return p;
}

/**
 * Create a LinphoneCallParams suitable for linphone_core_invite_with_params(), linphone_core_accept_call_with_params(), linphone_core_accept_early_media_with_params(),
 * linphone_core_accept_call_update().
 * The parameters are initialized according to the current LinphoneCore configuration and the current state of the LinphoneCall.
 * @param lc the LinphoneCore
 * @param call the call for which the parameters are to be build, or NULL in the case where the parameters are to be used for a new outgoing call.
 * @return a new LinphoneCallParams
 * @ingroup call_control
 */
LinphoneCallParams *linphone_core_create_call_params(LinphoneCore *lc, LinphoneCall *call){
	if (!call) return linphone_core_create_default_call_parameters(lc);
	return linphone_call_params_copy(call->params);
}

const char *linphone_reason_to_string(LinphoneReason err){
	switch(err){
		case LinphoneReasonNone:
			return "No error";
		case LinphoneReasonNoResponse:
			return "No response";
		case LinphoneReasonForbidden:
			return "Bad credentials";
		case LinphoneReasonDeclined:
			return "Call declined";
		case LinphoneReasonNotFound:
			return "User not found";
		case LinphoneReasonNotAnswered:
			return "Not answered";
		case LinphoneReasonBusy:
			return "Busy";
		case LinphoneReasonMedia:
			return "Incompatible media capabilities";
		case LinphoneReasonIOError:
			return "IO error";
		case LinphoneReasonDoNotDisturb:
			return "Do not distrub";
		case LinphoneReasonUnauthorized:
			return "Unauthorized";
		case LinphoneReasonNotAcceptable:
			return "Not acceptable here";
		case LinphoneReasonNoMatch:
			return "No match";
		case LinphoneReasonMovedPermanently:
			return "Moved permanently";
		case LinphoneReasonGone:
			return "Gone";
		case LinphoneReasonTemporarilyUnavailable:
			return "Temporarily unavailable";
		case LinphoneReasonAddressIncomplete:
			return "Address incomplete";
		case LinphoneReasonNotImplemented:
			return "Not implemented";
		case LinphoneReasonBadGateway:
			return "Bad gateway";
		case LinphoneReasonServerTimeout:
			return "Server timeout";
		case LinphoneReasonUnknown:
			return "Unknown error";
	}
	return "unknown error";
}

const char *linphone_error_to_string(LinphoneReason err){
	return linphone_reason_to_string(err);
}
/**
 * Enables signaling keep alive
 */
void linphone_core_enable_keep_alive(LinphoneCore* lc,bool_t enable) {
#ifdef BUILD_UPNP
	if (linphone_core_get_firewall_policy(lc)==LinphonePolicyUseUpnp) {
		enable = FALSE;
	}
#endif //BUILD_UPNP
	if (enable > 0) {
		sal_use_tcp_tls_keepalive(lc->sal,lc->sip_conf.tcp_tls_keepalive);
		sal_set_keepalive_period(lc->sal,lc->sip_conf.keepalive_period);
	} else {
		sal_set_keepalive_period(lc->sal,0);
	}
}
/**
 * Is signaling keep alive enabled
 */
bool_t linphone_core_keep_alive_enabled(LinphoneCore* lc) {
	return sal_get_keepalive_period(lc->sal) > 0;
}

void linphone_core_start_dtmf_stream(LinphoneCore* lc) {
	get_dtmf_gen(lc); /*make sure ring stream is started*/
	lc->ringstream_autorelease=FALSE; /*disable autorelease mode*/
}

/**
 * Whenever the liblinphone is playing a ring to advertise an incoming call or ringback of an outgoing call, this function stops
 * the ringing. Typical use is to stop ringing when the user requests to ignore the call.
 *
 * @param lc The LinphoneCore object
 *
 * @ingroup media_parameters
**/
void linphone_core_stop_ringing(LinphoneCore* lc) {
	LinphoneCall *call=linphone_core_get_current_call(lc);
	if (lc->ringstream) {
		ring_stop(lc->ringstream);
		lc->ringstream=NULL;
		lc->dmfs_playing_start_time=0;
		lc->ringstream_autorelease=TRUE;
	}
	if (call && call->ringing_beep){
		linphone_core_stop_dtmf(lc);
		call->ringing_beep=FALSE;
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

typedef struct Hook{
	LinphoneCoreIterateHook fun;
	void *data;
}Hook;

static Hook *hook_new(LinphoneCoreIterateHook hook, void *hook_data){
	Hook *h=ms_new0(Hook,1);
	h->fun=hook;
	h->data=hook_data;
	return h;
}

static void hook_invoke(Hook *h){
	h->fun(h->data);
}

void linphone_core_add_iterate_hook(LinphoneCore *lc, LinphoneCoreIterateHook hook, void *hook_data){
	lc->hooks=ms_list_append(lc->hooks,hook_new(hook,hook_data));
}

static void linphone_core_run_hooks(LinphoneCore *lc){
	ms_list_for_each(lc->hooks,(void (*)(void*))hook_invoke);
}

static void linphone_core_free_hooks(LinphoneCore *lc){
	ms_list_for_each(lc->hooks,(void (*)(void*))ms_free);
	ms_list_free(lc->hooks);
	lc->hooks=NULL;
}

void linphone_core_remove_iterate_hook(LinphoneCore *lc, LinphoneCoreIterateHook hook, void *hook_data){
	MSList *elem;
	for(elem=lc->hooks;elem!=NULL;elem=elem->next){
		Hook *h=(Hook*)elem->data;
		if (h->fun==hook && h->data==hook_data){
			lc->hooks = ms_list_remove_link(lc->hooks,elem);
			ms_free(h);
			return;
		}
	}
	ms_error("linphone_core_remove_iterate_hook(): No such hook found.");
}

void linphone_core_set_zrtp_secrets_file(LinphoneCore *lc, const char* file){
	if (lc->zrtp_secrets_cache != NULL) {
		ms_free(lc->zrtp_secrets_cache);
	}
	lc->zrtp_secrets_cache=file ? ms_strdup(file) : NULL;
}

const char *linphone_core_get_zrtp_secrets_file(LinphoneCore *lc){
	return lc->zrtp_secrets_cache;
}

void linphone_core_set_user_certificates_path(LinphoneCore *lc, const char* path){
	char* new_value;
	new_value = path?ms_strdup(path):NULL;
	if (lc->user_certificates_path) ms_free(lc->user_certificates_path);
	lp_config_set_string(lc->config,"misc","user_certificates_path",lc->user_certificates_path=new_value);
	return ;
}

const char *linphone_core_get_user_certificates_path(LinphoneCore *lc){
	return lc->user_certificates_path;
}

LinphoneCall* linphone_core_find_call_from_uri(const LinphoneCore *lc, const char *uri) {
	MSList *calls;
	LinphoneCall *c;
	const LinphoneAddress *address;
	char *current_uri;

	if (uri == NULL) return NULL;
	calls=lc->calls;
	while(calls) {
		c=(LinphoneCall*)calls->data;
		calls=calls->next;
		address = linphone_call_get_remote_address(c);
		current_uri=linphone_address_as_string_uri_only(address);
		if (strcmp(uri,current_uri)==0) {
			ms_free(current_uri);
			return c;
		} else {
			ms_free(current_uri);
		}
	}
	return NULL;
}


/**
 * Check if a call will need the sound resources.
 *
 * @ingroup call_control
 * @param lc The LinphoneCore
**/
bool_t linphone_core_sound_resources_locked(LinphoneCore *lc){
	MSList *elem;
	for(elem=lc->calls;elem!=NULL;elem=elem->next) {
		LinphoneCall *c=(LinphoneCall*)elem->data;
		switch (c->state) {
			case LinphoneCallOutgoingInit:
			case LinphoneCallOutgoingProgress:
			case LinphoneCallOutgoingRinging:
			case LinphoneCallOutgoingEarlyMedia:
			case LinphoneCallConnected:
			case LinphoneCallRefered:
			case LinphoneCallIncomingEarlyMedia:
			case LinphoneCallUpdating:
				ms_message("Call %p is locking sound resources.",c);
				return TRUE;
			default:
				break;
		}
	}
	return FALSE;
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
			return ms_zrtp_available();
		case LinphoneMediaEncryptionNone:
			return TRUE;
	}
	return FALSE;
}

int linphone_core_set_media_encryption(LinphoneCore *lc, LinphoneMediaEncryption menc) {
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
			if (!ms_zrtp_available()){
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
	params->has_video=linphone_core_video_enabled(lc) && lc->video_policy.automatically_initiate;
	params->media_encryption=linphone_core_get_media_encryption(lc);
	params->in_conference=FALSE;
	params->privacy=LinphonePrivacyDefault;
	params->avpf_enabled=FALSE;
	params->audio_dir=LinphoneMediaDirectionSendRecv;
	params->video_dir=LinphoneMediaDirectionSendRecv;
	params->real_early_media=lp_config_get_int(lc->config,"misc","real_early_media",FALSE);
	params->audio_multicast_enabled=linphone_core_audio_multicast_enabled(lc);
	params->video_multicast_enabled=linphone_core_video_multicast_enabled(lc);
}

void linphone_core_set_device_identifier(LinphoneCore *lc,const char* device_id) {
	if (lc->device_id) ms_free(lc->device_id);
	lc->device_id=ms_strdup(device_id);
}
const char*  linphone_core_get_device_identifier(const LinphoneCore *lc) {
	return lc->device_id;
}

/**
 * Set the DSCP field for SIP signaling channel.
 *
 * @ingroup network_parameters
 * * The DSCP defines the quality of service in IP packets.
 *
**/
void linphone_core_set_sip_dscp(LinphoneCore *lc, int dscp){
	sal_set_dscp(lc->sal,dscp);
	if (linphone_core_ready(lc)){
		lp_config_set_int_hex(lc->config,"sip","dscp",dscp);
		_linphone_core_apply_transports(lc);
	}
}

/**
 * Get the DSCP field for SIP signaling channel.
 *
 * @ingroup network_parameters
 * * The DSCP defines the quality of service in IP packets.
 *
**/
int linphone_core_get_sip_dscp(const LinphoneCore *lc){
	return lp_config_get_int(lc->config,"sip","dscp",0x1a);
}

/**
 * Set the DSCP field for outgoing audio streams.
 *
 * @ingroup network_parameters
 * The DSCP defines the quality of service in IP packets.
 *
**/
void linphone_core_set_audio_dscp(LinphoneCore *lc, int dscp){
	if (linphone_core_ready(lc))
		lp_config_set_int_hex(lc->config,"rtp","audio_dscp",dscp);
}

/**
 * Get the DSCP field for outgoing audio streams.
 *
 * @ingroup network_parameters
 * The DSCP defines the quality of service in IP packets.
 *
**/
int linphone_core_get_audio_dscp(const LinphoneCore *lc){
	return lp_config_get_int(lc->config,"rtp","audio_dscp",0x2e);
}

/**
 * Set the DSCP field for outgoing video streams.
 *
 * @ingroup network_parameters
 * The DSCP defines the quality of service in IP packets.
 *
**/
void linphone_core_set_video_dscp(LinphoneCore *lc, int dscp){
	if (linphone_core_ready(lc))
		lp_config_set_int_hex(lc->config,"rtp","video_dscp",dscp);

}

/**
 * Get the DSCP field for outgoing video streams.
 *
 * @ingroup network_parameters
 * The DSCP defines the quality of service in IP packets.
 *
**/
int linphone_core_get_video_dscp(const LinphoneCore *lc){
	return lp_config_get_int(lc->config,"rtp","video_dscp",0);
}


/**
 * Sets the database filename where chat messages will be stored.
 * If the file does not exist, it will be created.
 * @ingroup initializing
 * @param lc the linphone core
 * @param path filesystem path
**/
void linphone_core_set_chat_database_path(LinphoneCore *lc, const char *path){
	if (lc->chat_db_file){
		ms_free(lc->chat_db_file);
		lc->chat_db_file=NULL;
	}
	if (path) {
		lc->chat_db_file=ms_strdup(path);
		linphone_core_message_storage_init(lc);
	}
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

/**
 * This function controls signaling features supported by the core.
 * They are typically included in a SIP Supported header.
 * @param lc the LinphoneCore
 * @param tag the feature tag name
 * @ingroup initializing
**/
void linphone_core_add_supported_tag(LinphoneCore *lc, const char *tag){
	sal_add_supported_tag(lc->sal,tag);
	lp_config_set_string(lc->config,"sip","supported",sal_get_supported_tags(lc->sal));
}

/**
 * Remove a supported tag. @see linphone_core_add_supported_tag()
 * @param lc the LinphoneCore
 * @param tag the tag to remove
 * @ingroup initializing
**/
void linphone_core_remove_supported_tag(LinphoneCore *lc, const char *tag){
	sal_remove_supported_tag(lc->sal,tag);
	lp_config_set_string(lc->config,"sip","supported",sal_get_supported_tags(lc->sal));
}

/**
 * Enable RTCP feedback (also known as RTP/AVPF profile).
 * Setting LinphoneAVPFDefault is equivalent to LinphoneAVPFDisabled.
 * This setting can be overriden per LinphoneProxyConfig with linphone_proxy_config_set_avpf_mode().
 * The value set here is used for calls placed or received out of any proxy configured, or if the proxy config is configured with LinphoneAVPFDefault.
 * @param lc the LinphoneCore
 * @param mode the mode.
 * @ingroup media_parameters
**/
void linphone_core_set_avpf_mode(LinphoneCore *lc, LinphoneAVPFMode mode){
	if (mode==LinphoneAVPFDefault) mode=LinphoneAVPFDisabled;
	lc->rtp_conf.avpf_mode=mode;
	if (linphone_core_ready(lc)) lp_config_set_int(lc->config,"rtp","avpf",mode);
}

/**
 * Return AVPF enablement. See linphone_core_set_avpf_mode() .
 * @param lc the core
 * @return the avpf enablement mode.
 * @ingroup media_parameters
**/
LinphoneAVPFMode linphone_core_get_avpf_mode(const LinphoneCore *lc){
	return lc->rtp_conf.avpf_mode;
}

/**
 * Return the avpf report interval in seconds.
 * @param lc the LinphoneCore
 * @return the avpf report interval in seconds.
 * @ingroup media_parameters
**/
int linphone_core_get_avpf_rr_interval(const LinphoneCore *lc){
	return lp_config_get_int(lc->config,"rtp","avpf_rr_interval",5);
}

/**
 * Set the avpf report interval in seconds.
 * This value can be overriden by the proxy config using linphone_proxy_config_set_avpf_rr_interval().
 * @param lc the core
 * @param interval interval in seconds.
 * @ingroup media_parameters
**/
void linphone_core_set_avpf_rr_interval(LinphoneCore *lc, int interval){
	lp_config_set_int(lc->config,"rtp","avpf_rr_interval",interval);
}

int linphone_payload_type_get_type(const LinphonePayloadType *pt) {
	return pt->type;
}

int linphone_payload_type_get_normal_bitrate(const LinphonePayloadType *pt) {
	return pt->normal_bitrate;
}

const char * linphone_payload_type_get_mime_type(const LinphonePayloadType *pt) {
	return pt->mime_type;
}

int linphone_payload_type_get_channels(const LinphonePayloadType *pt) {
	return pt->channels;
}

int linphone_core_set_audio_multicast_addr(LinphoneCore *lc, const char* ip) {
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

int linphone_core_set_video_multicast_addr(LinphoneCore *lc, const char* ip) {
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

int linphone_core_set_audio_multicast_ttl(LinphoneCore *lc, int ttl) {
	if (ttl>255) {
		ms_error("Cannot set multicast audio ttl to core [%p] to [%i] value must be <256",lc,ttl);
		return -1;
	}

	lp_config_set_int(lc->config,"rtp","audio_multicast_ttl",lc->rtp_conf.audio_multicast_ttl=ttl);
	return 0;
}

int linphone_core_set_video_multicast_ttl(LinphoneCore *lc, int ttl) {
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

#ifdef ANDROID
static int linphone_core_call_void_method(jobject obj, jmethodID id) {
	JNIEnv *env=ms_get_jni_env();
		if (env && obj) {
			(*env)->CallVoidMethod(env,obj,id);
			if ((*env)->ExceptionCheck(env)) {
				(*env)->ExceptionClear(env);
				return -1;
			} else
				return 0;
		} else
			return -1;
}

void linphone_core_wifi_lock_acquire(LinphoneCore *lc) {
	if (linphone_core_call_void_method(lc->wifi_lock,lc->wifi_lock_acquire_id))
		ms_warning("No wifi lock configured or not usable for core [%p]",lc);
}
void linphone_core_wifi_lock_release(LinphoneCore *lc) {
	if (linphone_core_call_void_method(lc->wifi_lock,lc->wifi_lock_release_id))
		ms_warning("No wifi lock configured or not usable for core [%p]",lc);
}
void linphone_core_multicast_lock_acquire(LinphoneCore *lc) {
	if (linphone_core_call_void_method(lc->multicast_lock,lc->multicast_lock_acquire_id))
			ms_warning("No multicast lock configured or not usable for core [%p]",lc);
}
void linphone_core_multicast_lock_release(LinphoneCore *lc) {
	if (linphone_core_call_void_method(lc->multicast_lock,lc->multicast_lock_release_id))
			ms_warning("No wifi lock configured or not usable for core [%p]",lc);
}
#endif


LINPHONE_PUBLIC const char *linphone_core_log_collection_upload_state_to_string(const LinphoneCoreLogCollectionUploadState lcus) {
	switch (lcus) {
	case LinphoneCoreLogCollectionUploadStateInProgress : return "LinphoneCoreLogCollectionUploadStateInProgress";
	case LinphoneCoreLogCollectionUploadStateDelivered : return "LinphoneCoreLogCollectionUploadStateDelivered";
	case LinphoneCoreLogCollectionUploadStateNotDelivered : return "LinphoneCoreLogCollectionUploadStateNotDelivered";
	}
	return "UNKNOWN";
}
