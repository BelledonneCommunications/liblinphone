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

#include "linphone/api/c-content.h"
#include "linphone/core_utils.h"
#include "linphone/core.h"
#include "linphone/logging.h"
#include "linphone/lpconfig.h"

#include "private.h"
#include "logging-private.h"

LinphoneUpdateCheck * linphone_update_check_new(LinphoneCore *lc, const char *version, belle_http_request_listener_t *listener) {
	LinphoneUpdateCheck *update = ms_new0(LinphoneUpdateCheck, 1);

	update->lc = lc;
	if (version) {
		update->current_version = bctbx_strdup(version);
	}
	update->http_listener = listener;

	return update;
}

static void linphone_update_check_destroy(LinphoneUpdateCheck *update) {
	if (update->current_version) bctbx_free(update->current_version);
	if (update->http_listener) belle_sip_object_unref(update->http_listener);
	ms_free(update);
}

static void update_check_process_terminated(LinphoneUpdateCheck *update, LinphoneVersionUpdateCheckResult result, const char *version, const char *url) {
	linphone_core_notify_version_update_check_result_received(update->lc, result, version, url);
	linphone_update_check_destroy(update);
}

typedef struct _parsed_version_st {
	int major;
	int minor;
	int patch;
} parsed_version_t;

static int compare_parsed_versions(parsed_version_t current_version, parsed_version_t last_version) {
	bool same_major = (last_version.major == current_version.major);
	bool same_minor = (last_version.minor == current_version.minor);
	if (last_version.major > current_version.major) return 1;
	if (same_major && last_version.minor > current_version.minor) return 1;
	if (same_minor && last_version.patch > current_version.patch) return 1;
	return -1;
}

static void parse_version(const char *version, parsed_version_t *parsed_version) {
	char *copy = bctbx_strdup(version);
	char *ptr = copy;
	char *next;
	memset(parsed_version, 0, sizeof(parsed_version_t));
	next = strchr(ptr, '.');
	parsed_version->major = atoi(ptr);
	ptr = next + 1;
	next = strchr(ptr, '.');
	parsed_version->minor = atoi(ptr);
	if (next != NULL) {
		ptr = next + 1;
		parsed_version->patch = atoi(ptr);
	}
	bctbx_free(copy);
}

static bool_t update_is_available(const char *current_version, const char *last_version) {
	parsed_version_t current_parsed_version;
	parsed_version_t last_parsed_version;
	parse_version(current_version, &current_parsed_version);
	parse_version(last_version, &last_parsed_version);
	return (compare_parsed_versions(current_parsed_version, last_parsed_version) > 0) ? TRUE : FALSE;
}

static void update_check_process_response_event(void *ctx, const belle_http_response_event_t *event) {
	LinphoneUpdateCheck *update = (LinphoneUpdateCheck *)ctx;

	if (belle_http_response_get_status_code(event->response) == 200) {
		belle_sip_message_t *message = BELLE_SIP_MESSAGE(event->response);
		char *body = bctbx_strdup(belle_sip_message_get_body(message));
		char *last_version = body;
		char *url = strchr(body, '\t');
		char *ptr;
		if (url == NULL) {
			ms_error("Bad format for update check answer, cannot find TAB between version and URL");
			update_check_process_terminated(update, LinphoneVersionUpdateCheckError, NULL, NULL);
			bctbx_free(body);
			return;
		}
		*url = '\0';
		url++;
		ptr = strrchr(url, '\r');
		if (ptr != NULL) *ptr = '\0';
		ptr = strrchr(url, '\n');
		if (ptr != NULL) *ptr = '\0';
		if (update_is_available(update->current_version, last_version)) {
			update_check_process_terminated(update, LinphoneVersionUpdateCheckNewVersionAvailable, last_version, url);
		} else {
			update_check_process_terminated(update, LinphoneVersionUpdateCheckUpToDate, NULL, NULL);
		}
		bctbx_free(body);
	} else {
		update_check_process_terminated(update, LinphoneVersionUpdateCheckError, NULL, NULL);
	}
}

static void update_check_process_io_error(void *ctx, const belle_sip_io_error_event_t *event) {
	LinphoneUpdateCheck *update = (LinphoneUpdateCheck *)ctx;
	update_check_process_terminated(update, LinphoneVersionUpdateCheckError, NULL, NULL);
}

static void update_check_process_timeout(void *ctx, const belle_sip_timeout_event_t *event) {
	LinphoneUpdateCheck *update = (LinphoneUpdateCheck *)ctx;
	update_check_process_terminated(update, LinphoneVersionUpdateCheckError, NULL, NULL);
}

static void update_check_process_auth_requested(void *ctx, belle_sip_auth_event_t *event) {
	LinphoneUpdateCheck *update = (LinphoneUpdateCheck *)ctx;
	update_check_process_terminated(update, LinphoneVersionUpdateCheckError, NULL, NULL);
}

void linphone_core_check_for_update(LinphoneCore *lc, const char *current_version) {
	bool_t is_desktop = FALSE;
	const char *platform = NULL;
	const char *mobilePlatform = NULL;
	const char *version_check_url_root = lp_config_get_string(lc->config, "misc", "version_check_url_root", NULL);

	if (current_version == NULL || strlen(current_version) == 0) {
		bctbx_error("Can't check for a version newer than null or empty !");
		return;
	}

	if (version_check_url_root != NULL) {
		belle_http_request_listener_callbacks_t belle_request_listener = { 0 };
		belle_http_request_t *request;
		belle_generic_uri_t *uri;
		char *version_check_url;
		LinphoneUpdateCheck *update;
		MSList *item;
		MSList *platform_tags = ms_factory_get_platform_tags(linphone_core_get_ms_factory(lc));

		for (item = platform_tags; item != NULL; item = ms_list_next(item)) {
			const char *tag = (const char *)item->data;
			if (strcmp(tag, "win32") == 0) platform = "windows";
			else if (strcmp(tag, "apple") == 0) platform = "macosx";
			else if (strcmp(tag, "linux") == 0) platform = "linux";
			else if (strcmp(tag, "ios") == 0) mobilePlatform = "ios";
			else if (strcmp(tag, "android") == 0) mobilePlatform = "android";
			else if (strcmp(tag, "desktop") == 0) is_desktop = TRUE;
		}
		if (!is_desktop) {
			platform = mobilePlatform;
		}
		if (platform == NULL) {
			ms_warning("Update checking is not supported on this platform");
			return;
		}
		version_check_url = bctbx_strdup_printf("%s/%s/RELEASE", version_check_url_root, platform);
		uri = belle_generic_uri_parse(version_check_url);
		ms_message("Checking for new version at: %s", version_check_url);
		bctbx_free(version_check_url);

		belle_request_listener.process_response = update_check_process_response_event;
		belle_request_listener.process_auth_requested = update_check_process_auth_requested;
		belle_request_listener.process_io_error = update_check_process_io_error;
		belle_request_listener.process_timeout = update_check_process_timeout;

		update = linphone_update_check_new(lc, current_version, NULL);
		update->http_listener = belle_http_request_listener_create_from_callbacks(&belle_request_listener, update);
		request = belle_http_request_create("GET", uri, belle_sip_header_create("User-Agent", linphone_core_get_user_agent(lc)), NULL);
		belle_http_provider_send_request(lc->http_provider, request, update->http_listener);
	}
}