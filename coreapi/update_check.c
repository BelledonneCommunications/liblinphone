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

#include "bctoolbox/defs.h"

#include "linphone/api/c-content.h"
#include "linphone/core.h"
#include "linphone/core_utils.h"
#include "linphone/logging.h"
#include "linphone/lpconfig.h"
#include "linphone/utils/utils.h"

#include "http/http-client.h"
#include "logging-private.h"
#include "private.h"

using namespace LinphonePrivate;

LinphoneUpdateCheck *linphone_update_check_new(LinphoneCore *lc, const char *version) {
	LinphoneUpdateCheck *update = ms_new0(LinphoneUpdateCheck, 1);

	update->lc = lc;
	if (version) {
		update->current_version = bctbx_strdup(version);
	}

	return update;
}

static void linphone_update_check_destroy(LinphoneUpdateCheck *update) {
	if (update->current_version) bctbx_free(update->current_version);
	ms_free(update);
}

static void update_check_process_terminated(LinphoneUpdateCheck *update,
                                            LinphoneVersionUpdateCheckResult result,
                                            const char *version,
                                            const char *url) {
	linphone_core_notify_version_update_check_result_received(update->lc, result, version, url);
	linphone_update_check_destroy(update);
}

static bool_t update_is_available(const char *current_version, const char *last_version) {
	return Utils::Version(current_version) < Utils::Version(last_version);
}

static void update_check_process_response_event(LinphoneUpdateCheck *update, const HttpResponse &response) {

	if (response.getHttpStatusCode() == 200) {
		const Content &content = response.getBody();
		if (!content.isEmpty()) {
			char *body = bctbx_strdup(content.getBodyAsUtf8String().c_str());
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
				update_check_process_terminated(update, LinphoneVersionUpdateCheckNewVersionAvailable, last_version,
				                                url);
			} else {
				update_check_process_terminated(update, LinphoneVersionUpdateCheckUpToDate, NULL, NULL);
			}
			bctbx_free(body);
		}
	} else {
		update_check_process_terminated(update, LinphoneVersionUpdateCheckError, NULL, NULL);
	}
}

void linphone_core_check_for_update(LinphoneCore *lc, const char *current_version) {
	bool_t is_desktop = FALSE;
	const char *platform = NULL;
	const char *mobilePlatform = NULL;
	const char *version_check_url_root = linphone_config_get_string(lc->config, "misc", "version_check_url_root", NULL);

	if (current_version == NULL || strlen(current_version) == 0) {
		bctbx_error("Can't check for a version newer than null or empty !");
		return;
	}

	if (version_check_url_root != NULL) {
		char *version_check_url;
		LinphoneUpdateCheck *update;
		bctbx_list_t *item;
		bctbx_list_t *platform_tags = ms_factory_get_platform_tags(linphone_core_get_ms_factory(lc));

		for (item = platform_tags; item != NULL; item = item->next) {
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
		ms_message("Checking for new version at: %s", version_check_url);

		update = linphone_update_check_new(lc, current_version);
		L_GET_CPP_PTR_FROM_C_OBJECT(lc)
		    ->getHttpClient()
		    .createRequest("GET", version_check_url)
		    .addHeader("User-Agent", linphone_core_get_user_agent(lc))
		    .execute([update](const HttpResponse &response) { update_check_process_response_event(update, response); });
		bctbx_free(version_check_url);
	}
}
