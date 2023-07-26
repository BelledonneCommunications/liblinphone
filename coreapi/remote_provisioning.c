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

#include <bctoolbox/defs.h>

#include "http/http-client.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/lpconfig.h"
#include "private.h"
#include "xml2lpc.h"

#define XML2LPC_CALLBACK_BUFFER_SIZE 1024

static void belle_request_process_io_error(void *ctx, BCTBX_UNUSED(const belle_sip_io_error_event_t *event)) {
	LinphoneCore *lc = (LinphoneCore *)ctx;
	linphone_configuring_terminated(lc, LinphoneConfiguringFailed, "http io error");
}

static void belle_request_process_timeout(void *ctx, BCTBX_UNUSED(const belle_sip_timeout_event_t *event)) {
	LinphoneCore *lc = (LinphoneCore *)ctx;
	linphone_configuring_terminated(lc, LinphoneConfiguringFailed, "http timeout");
}

static void belle_request_process_auth_requested(void *ctx, belle_sip_auth_event_t *event) {
	LinphoneCore *lc = (LinphoneCore *)ctx;
	linphone_core_fill_belle_sip_auth_event(lc, event, NULL, NULL);
}

static void linphone_remote_provisioning_apply(LinphoneCore *lc, const char *xml) {
	LinphoneConfig *config = linphone_core_get_config(lc);
	const char *error_msg = _linphone_config_load_from_xml_string(config, xml);

	_linphone_config_apply_factory_config(config);
	linphone_configuring_terminated(lc, error_msg ? LinphoneConfiguringFailed : LinphoneConfiguringSuccessful,
	                                error_msg);
}

int linphone_remote_provisioning_load_file(LinphoneCore *lc, const char *file_path) {
	int status = -1;
	char *provisioning = ms_load_path_content(file_path, NULL);

	if (provisioning) {
		linphone_remote_provisioning_apply(lc, provisioning);
		status = 0;
		ms_free(provisioning);
	}
	return status;
}

static void belle_request_process_response_event(void *ctx, const belle_http_response_event_t *event) {
	LinphoneCore *lc = (LinphoneCore *)ctx;
	belle_sip_message_t *message = BELLE_SIP_MESSAGE(event->response);
	const char *body = belle_sip_message_get_body(message);

	int statusCode = belle_http_response_get_status_code(event->response);
	if (statusCode == 200) {
		linphone_remote_provisioning_apply(lc, body);
	} else if (statusCode == 401) {
		linphone_configuring_terminated(lc, LinphoneConfiguringFailed, "http auth requested");
	} else {
		linphone_configuring_terminated(lc, LinphoneConfiguringFailed, "http error");
	}
}

int linphone_remote_provisioning_download_and_apply(LinphoneCore *lc,
                                                    const char *remote_provisioning_uri,
                                                    const bctbx_list_t *remote_provisioning_headers) {
	belle_generic_uri_t *uri = belle_generic_uri_parse(remote_provisioning_uri);
	const char *scheme = uri ? belle_generic_uri_get_scheme(uri) : NULL;
	const char *host = uri ? belle_generic_uri_get_host(uri) : NULL;

	if (scheme && (strcmp(scheme, "file") == 0)) {
		// We allow for 'local remote-provisioning' in case the file is to be opened from the hard drive.
		const char *file_path = remote_provisioning_uri + strlen("file://"); // skip scheme

		if (uri) {
			belle_sip_object_unref(uri);
		}

		return linphone_remote_provisioning_load_file(lc, file_path);
	} else if (scheme && strncmp(scheme, "http", 4) == 0 && host && strlen(host) > 0) {
		belle_http_request_listener_callbacks_t belle_request_listener = {0};
		belle_http_request_t *request;

		belle_request_listener.process_response = belle_request_process_response_event;
		belle_request_listener.process_auth_requested = belle_request_process_auth_requested;
		belle_request_listener.process_io_error = belle_request_process_io_error;
		belle_request_listener.process_timeout = belle_request_process_timeout;

		lc->provisioning_http_listener = belle_http_request_listener_create_from_callbacks(&belle_request_listener, lc);

		request = belle_http_request_create("GET", uri,
		                                    belle_sip_header_create("User-Agent", linphone_core_get_user_agent(lc)),
		                                    belle_sip_header_create("X-Linphone-Provisioning", "1"), NULL);

		const bctbx_list_t *header_it = remote_provisioning_headers;
		while (header_it) {
			const bctbx_list_t *pair_value = (const bctbx_list_t *)bctbx_list_get_data(header_it);
			const char *field = (const char *)bctbx_list_get_data(pair_value);
			pair_value = bctbx_list_next(pair_value);
			belle_sip_header_t *header = belle_http_header_create(field, (const char *)bctbx_list_get_data(pair_value));
			if (header) belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), header);
			header_it = bctbx_list_next(header_it);
		}

		LinphoneAccount *account = linphone_core_get_default_account(lc);
		if (account != nullptr) {
			char *addr = linphone_address_as_string_uri_only(
			    linphone_account_params_get_identity_address(linphone_account_get_params(account)));
			belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), belle_http_header_create("From", addr));
			ms_free(addr);
		} else if (linphone_config_get_string(lc->config, "misc", "remote_provisioning_from_address", NULL) != NULL) {
			const char *addr = linphone_config_get_string(lc->config, "misc", "remote_provisioning_from_address", NULL);
			belle_sip_message_add_header(BELLE_SIP_MESSAGE(request), belle_http_header_create("From", addr));
		}

		return belle_http_provider_send_request(L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getHttpClient().getProvider(), request,
		                                        lc->provisioning_http_listener);
	} else {
		ms_error("Invalid provisioning URI [%s] (missing scheme or host ?)", remote_provisioning_uri);
		if (uri) {
			belle_sip_object_unref(uri);
		}
		return -1;
	}
}

// Remove all "[misc] config-uri-header_X"
void linphone_core_clear_provisioning_headers(LinphoneCore *lc) {
	char config_field[40];
	int count = 0;
	snprintf(config_field, 40, "config-uri-header_%d", count);
	while (linphone_config_has_entry(lc->config, "misc", config_field)) {
		linphone_config_clean_entry(lc->config, "misc", config_field);
		snprintf(config_field, 40, "config-uri-header_%d", ++count);
	}
}

// Add a pair "config-uri-header_<N+1> ; header" where header will be added to the remote provisioning download. N is
// the current header index (which is '-1' if no headers have been defined)
void linphone_core_add_provisioning_header(LinphoneCore *lc, const char *header_name, const char *value) {
	char config_field[40];
	// 1. get last index
	int count = 0;
	snprintf(config_field, 40, "config-uri-header_%d", count);
	while (linphone_config_has_entry(lc->config, "misc", config_field)) {
		snprintf(config_field, 40, "config-uri-header_%d", ++count);
	}
	// 2. Build value
	char *header = bctbx_strdup_printf("%s:%s", header_name, value);
	// 3. Write new header
	linphone_config_set_string(lc->config, "misc", config_field, header);
	bctbx_free(header);
}

// Split "<header name>:<value>". Note : we are not using stringlist because it will split other ',' in value. And by
// keeping ':', we match the header syntax for config readability.
bctbx_list_t *linphone_remote_provisioning_split_header(const char *serialized_header) {
	bctbx_list_t *header = NULL;
	int index = 0;
	while (serialized_header[index] != ':' && serialized_header[index] != '\0')
		++index;
	if (serialized_header[index] != '\0') { // Check null in case of malformed configuration
		char *header_name = bctbx_strndup(serialized_header, index);
		char *value = bctbx_strdup(serialized_header + index + 1);
		header = bctbx_list_append(header, header_name);
		header = bctbx_list_append(header, value);
	}
	return header;
}

bctbx_list_t *linphone_remote_provisioning_split_headers(bctbx_list_t *headers) {
	bctbx_list_t *splitted_headers = NULL;

	while (headers) { // Allow headers to be NULL
		bctbx_list_t *header = linphone_remote_provisioning_split_header((const char *)bctbx_list_get_data(headers));
		if (header) splitted_headers = bctbx_list_append(splitted_headers, header);
		headers = bctbx_list_next(headers);
	}

	return splitted_headers;
}

// Get an array of serialized pair (header name/value). Note : deserialized is not supported by wrappers (aka
// bctbx_list<bctbx_list<char*>>)
bctbx_list_t *linphone_core_get_provisioning_headers(const LinphoneCore *lc) {
	bctbx_list_t *headers = NULL;
	char config_field[40];
	int count = 0;
	const char *header_name = NULL;
	snprintf(config_field, 40, "config-uri-header_%d", count);
	while ((header_name = linphone_config_get_string(lc->config, "misc", config_field, NULL))) { // Should be never NULL
		headers = bctbx_list_append(headers, bctbx_strdup(header_name));
		snprintf(config_field, 40, "config-uri-header_%d", ++count);
	}
	return headers;
}

LinphoneStatus linphone_core_set_provisioning_uri(LinphoneCore *lc, const char *remote_provisioning_uri) {
	belle_generic_uri_t *uri = remote_provisioning_uri ? belle_generic_uri_parse(remote_provisioning_uri) : NULL;
	if (!remote_provisioning_uri || uri) {
		linphone_config_set_string(lc->config, "misc", "config-uri", remote_provisioning_uri);
		if (!remote_provisioning_uri) linphone_core_clear_provisioning_headers(lc);
		if (uri) {
			belle_sip_object_unref(uri);
		}
		return 0;
	}
	ms_error("Invalid provisioning URI [%s] (could not be parsed)", remote_provisioning_uri);
	return -1;
}

const char *linphone_core_get_provisioning_uri(const LinphoneCore *lc) {
	return linphone_config_get_string(lc->config, "misc", "config-uri", NULL);
}

bool_t linphone_core_is_provisioning_transient(LinphoneCore *lc) {
	return linphone_config_get_int(lc->config, "misc", "transient_provisioning", 0) == 1;
}
