/*
 * Copyright (c) 2010-2021 Belledonne Communications SARL.
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

#include "linphone/account_creator.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"

#include "FlexiAPIClient.hh"

#include "c-wrapper/c-wrapper.h"
#include "dial-plan/dial-plan.h"

#include "bctoolbox/crypto.h"
#include "bctoolbox/regex.h"

#include "private.h"

#include <functional>
#include <json/json.h>
#include <algorithm>
#include <string>

using namespace LinphonePrivate;
using namespace std;

FlexiAPIClient::FlexiAPIClient(LinphoneCore *lc) {
	mCore = lc;
	mApiKey = nullptr;
	mTest = false;

	// Assign the core there as well to keep it in the callback contexts
	mRequestCallbacks.core = lc;
}

/**
 * Public endpoints
 */

FlexiAPIClient *FlexiAPIClient::ping() {
	prepareRequest("ping");
	return this;
}

FlexiAPIClient *FlexiAPIClient::sendToken(string pnProvider, string pnParam, string pnPrid) {
	JsonParams params;
	params.push("pn_provider", pnProvider);
	params.push("pn_param", pnParam);
	params.push("pn_prid", pnPrid);
	prepareRequest("tokens", "POST", params);
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountCreate(string username, string password, string algorithm, string token) {
	JsonParams params;
	params.push("username", username);
	params.push("password", password);
	params.push("algorithm", algorithm);
	params.push("token", token);
	prepareRequest("accounts/with-token", "POST", params);
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountCreate(string username, string domain, string password, string algorithm,
											  string token) {
	JsonParams params;
	params.push("username", username);
	params.push("domain", domain);
	params.push("password", password);
	params.push("algorithm", algorithm);
	params.push("token", token);
	prepareRequest("accounts/with-token", "POST", params);
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountInfo(string sip) {
	prepareRequest(string("accounts/").append(urlEncode(sip)).append("/info"));
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountActivateEmail(string sip, string code) {
	JsonParams params;
	params.push("code", code);
	prepareRequest(string("accounts/").append(urlEncode(sip)).append("/activate/email"), "POST", params);
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountActivatePhone(string sip, string code) {
	JsonParams params;
	params.push("code", code);
	prepareRequest(string("accounts/").append(urlEncode(sip)).append("/activate/email"), "POST", params);
	return this;
}

/**
 * Authenticated endpoints
 */

/**
 * Change the account password
 * @param [in] algorithm can be SHA-256 or MD5
 * @param [in] the new password
 * @param [in] the old password if already set
 */

FlexiAPIClient *FlexiAPIClient::me() {
	prepareRequest("accounts/me");
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountDelete() {
	prepareRequest("accounts/me", "DELETE");
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountPasswordChange(string algorithm, string password) {
	return accountPasswordChange(algorithm, password, "");
}

FlexiAPIClient *FlexiAPIClient::accountPasswordChange(string algorithm, string password, string oldPassword) {
	JsonParams params;
	params.push("algorithm", algorithm);
	params.push("password", password);

	if (!oldPassword.empty()) {
		params.push("old_password", oldPassword);
	}

	prepareRequest("accounts/me/password", "POST", params);
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountEmailChangeRequest(string email) {
	JsonParams params;
	params.push("email", email);
	prepareRequest("accounts/me/email/request", "POST", params);
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountDevices() {
	prepareRequest("accounts/me/devices");
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountDevice(string uuid) {
	prepareRequest(string("accounts/me/devices/").append(uuid));
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountPhoneChangeRequest(string phone) {
	JsonParams params;
	params.push("phone", phone);
	prepareRequest("accounts/me/phone/request", "POST", params);
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountPhoneChange(string code) {
	JsonParams params;
	params.push("code", code);
	prepareRequest("accounts/me/phone", "POST", params);
	return this;
}

/**
 * Admin endpoints
 */

FlexiAPIClient *FlexiAPIClient::adminAccountCreate(string username, string password, string algorithm) {
	return adminAccountCreate(username, password, algorithm, "", false);
}

FlexiAPIClient *FlexiAPIClient::adminAccountCreate(string username, string password, string algorithm, string domain) {
	return adminAccountCreate(username, password, algorithm, domain, false);
}

FlexiAPIClient *FlexiAPIClient::adminAccountCreate(string username, string password, string algorithm, bool activated) {
	return adminAccountCreate(username, password, algorithm, "", activated);
}

FlexiAPIClient *FlexiAPIClient::adminAccountCreate(string username, string password, string algorithm, string domain,
								   bool activated) {
	return adminAccountCreate(username, password, algorithm, domain, activated, "");
}

FlexiAPIClient *FlexiAPIClient::adminAccountCreate(string username, string password, string algorithm, string domain,
												   bool activated, string email) {
	return adminAccountCreate(username, password, algorithm, domain, activated, email, "");
}

FlexiAPIClient *FlexiAPIClient::adminAccountCreate(string username, string password, string algorithm, string domain,
												   bool activated, string email, string phone) {
	JsonParams params;
	params.push("username", username);
	params.push("password", password);
	params.push("algorithm", algorithm);
	params.push("activated", to_string(activated));

	if (!email.empty()) {
		params.push("email", email);
	}
	if (!phone.empty()) {
		params.push("phone", phone);
	}
	if (!domain.empty()) {
		params.push("domain", domain);
	}
	prepareRequest("accounts", "POST", params);
	return this;
}

FlexiAPIClient *FlexiAPIClient::adminAccounts() {
	prepareRequest("accounts");
	return this;
}

FlexiAPIClient *FlexiAPIClient::adminAccount(int id) {
	prepareRequest(string("accounts/").append(to_string(id)));
	return this;
}

FlexiAPIClient *FlexiAPIClient::adminAccountDelete(int id) {
	prepareRequest(string("accounts/").append(to_string(id)), "DELETE");
	return this;
}

FlexiAPIClient *FlexiAPIClient::adminAccountActivate(int id) {
	prepareRequest(string("accounts/").append(to_string(id)).append("/activate"));
	return this;
}

FlexiAPIClient *FlexiAPIClient::adminAccountDeactivate(int id) {
	prepareRequest(string("accounts/").append(to_string(id)).append("/deactivate"));
	return this;
}

/**
 * Authentication
 */

FlexiAPIClient *FlexiAPIClient::setApiKey(const char *key) {
	mApiKey = key;
	return this;
}

FlexiAPIClient *FlexiAPIClient::setTest(bool test) {
	mTest = test;
	return this;
}

/**
 * Callback requests
 */

FlexiAPIClient *FlexiAPIClient::then(function<void(FlexiAPIClient::Response)> success) {
	mRequestCallbacks.success = success;
	return this;
}

FlexiAPIClient *FlexiAPIClient::error(function<void(FlexiAPIClient::Response)> error) {
	mRequestCallbacks.error = error;
	return this;
}

void FlexiAPIClient::prepareRequest(string path) {
	JsonParams params;
	prepareRequest(path, "GET", params);
}

void FlexiAPIClient::prepareRequest(string path, string type) {
	JsonParams params;
	prepareRequest(path, type, params);
}

void FlexiAPIClient::prepareRequest(string path, string type, JsonParams params) {
	mRequestCallbacks.mSelf = shared_from_this();
	belle_http_request_listener_callbacks_t internalCallbacks = {};
	belle_http_request_listener_t *listener;
	belle_http_request_t *req;

	string uri = linphone_config_get_string(mCore->config, "account_creator", "url", NULL);

	req = belle_http_request_create(type.c_str(), belle_generic_uri_parse(uri.append(path).c_str()),
									belle_sip_header_content_type_create("application", "json"),
									belle_sip_header_accept_create("application", "json"), NULL);

	LinphoneProxyConfig *cfg = linphone_core_get_default_proxy_config(mCore);
	if (cfg != nullptr) {
		char *addr = linphone_address_as_string_uri_only(linphone_proxy_config_get_identity_address(cfg));
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(req), belle_http_header_create("From", addr));

		ms_free(addr);
	} else if (mTest) {
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(req), belle_http_header_create("From", "sip:admin_test@sip.example.org"));
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(req), belle_http_header_create("x-api-key", "no_secret_at_all"));
	}

	if (!params.empty()) {
		string body = params.json();
		belle_sip_message_set_body(BELLE_SIP_MESSAGE(req), body.c_str(), body.size());
	}

	if (mApiKey != nullptr) {
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(req), belle_http_header_create("x-api-key", mApiKey));
	}

	internalCallbacks.process_response = processResponse;
	internalCallbacks.process_auth_requested = processAuthRequested;
	listener = belle_http_request_listener_create_from_callbacks(&internalCallbacks, &mRequestCallbacks);

	belle_http_provider_send_request(mCore->http_provider, req, listener);
	belle_sip_object_data_set(BELLE_SIP_OBJECT(req), "listener", listener, belle_sip_object_unref);
}

void FlexiAPIClient::processResponse(void *ctx, const belle_http_response_event_t *event) noexcept {
	auto cb = static_cast<Callbacks *>(ctx);

	try {
		FlexiAPIClient::Response response;

		if (event->response) {
			int code = belle_http_response_get_status_code(event->response);
			response.code = code;

			if (code >= 200 && code < 300) {
				belle_sip_body_handler_t *body = belle_sip_message_get_body_handler(BELLE_SIP_MESSAGE(event->response));
				char *content = belle_sip_object_to_string(body);
				response.body = content;
				ms_free(content);

				if (cb->success) {
					cb->success(response);
				}
			} else if (cb->error) {
				cb->error(response);
			}
		}

	} catch (const std::exception &e) {
		lError() << e.what();
	}

	cb->mSelf = nullptr;
}

void FlexiAPIClient::processAuthRequested(void *ctx, belle_sip_auth_event_t *event) noexcept {
	auto cb = static_cast<Callbacks *>(ctx);

	try {
		const char *username = belle_sip_auth_event_get_username(event);
		const char *domain = belle_sip_auth_event_get_domain(event);

		linphone_core_fill_belle_sip_auth_event(cb->core, event, username, domain);
	} catch (const std::exception &e) {
		lError() << e.what();
	}
}

string FlexiAPIClient::urlEncode(const string &value) {
	ostringstream escaped;
	escaped.fill('0');
	escaped << hex;

	for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
		string::value_type c = (*i);

		// Keep alphanumeric and other accepted characters intact
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
			escaped << c;
			continue;
		}

		// Any other characters are percent-encoded
		escaped << uppercase;
		escaped << '%' << int((unsigned char)c);
		escaped << nouppercase;
	}

	return escaped.str();
}
