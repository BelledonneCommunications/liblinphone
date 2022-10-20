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

#include "linphone/account_creator.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"

#include "linphone/FlexiAPIClient.hh"

#include "c-wrapper/c-wrapper.h"
#include "dial-plan/dial-plan.h"

#include "bctoolbox/crypto.h"
#include "bctoolbox/regex.h"

#include "private.h"

#include <algorithm>
#include <functional>
#include <json/json.h>
#include <string>

using namespace LinphonePrivate;
using namespace std;

Json::Value FlexiAPIClient::Response::json() {
	JSONCPP_STRING err;
	Json::CharReaderBuilder builder;
	Json::Value obj;

	const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

	if (!reader->parse(body.c_str(), body.c_str() + body.length(), &obj, &err)) {
		lError() << err;
	}

	return obj;
}

FlexiAPIClient::FlexiAPIClient(LinphoneCore *lc) {
	mCore = lc;
	mApiKey = nullptr;
	mUseTestAdminAccount = false;

	// Assign the core there as well to keep it in the callback contexts
	mRequestCallbacks.core = lc;
}

/**
 * Public endpoints
 */

FlexiAPIClient *FlexiAPIClient::ping() {
	prepareAndSendRequest("ping");
	return this;
}

FlexiAPIClient *FlexiAPIClient::sendAccountCreationTokenByPush(string pnProvider, string pnParam, string pnPrid) {
	JsonParams params;
	params.push("pn_provider", pnProvider);
	params.push("pn_param", pnParam);
	params.push("pn_prid", pnPrid);
	prepareAndSendRequest("account_creation_tokens/send-by-push", "POST", params);
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountCreateWithAccountCreationToken(string username, string password,
																	  string algorithm, string accountCreationToken) {
	JsonParams params;
	params.push("username", username);
	params.push("password", password);
	params.push("algorithm", algorithm);
	params.push("account_creation_token", accountCreationToken);
	prepareAndSendRequest("accounts/with-account-creation-token", "POST", params);
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountCreateWithAccountCreationToken(string username, string domain, string password,
																	  string algorithm, string accountCreationToken) {
	JsonParams params;
	params.push("username", username);
	params.push("domain", domain);
	params.push("password", password);
	params.push("algorithm", algorithm);
	params.push("account_creation_token", accountCreationToken);
	prepareAndSendRequest("accounts/with-account-creation-token", "POST", params);
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountInfo(string sip) {
	prepareAndSendRequest(string("accounts/").append(urlEncode(sip)).append("/info"));
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountActivateEmail(string sip, string code) {
	JsonParams params;
	params.push("code", code);
	prepareAndSendRequest(string("accounts/").append(urlEncode(sip)).append("/activate/email"), "POST", params);
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountActivatePhone(string sip, string code) {
	JsonParams params;
	params.push("code", code);
	prepareAndSendRequest(string("accounts/").append(urlEncode(sip)).append("/activate/email"), "POST", params);
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountAuthTokenCreate() {
	prepareAndSendRequest("accounts/auth_token", "POST");
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountApiKeyFromAuthTokenGenerate(string authToken) {
	prepareAndSendRequest(string("accounts/me/api_key/").append(authToken));
	return this;
}

/**
 * Public unsecure endpoint
 */
FlexiAPIClient *FlexiAPIClient::accountCreate(string username, string password, string email) {
	return accountCreate(username, password, "", "", email, "");
}

FlexiAPIClient *FlexiAPIClient::accountCreate(string username, string password, string algorithm, string domain,
											  string email, string phone) {
	JsonParams params;

	if (!username.empty()) {
		params.push("username", username);
	}

	params.push("password", password);
	params.push("algorithm", (!algorithm.empty()) ? algorithm : "MD5");

	if (!email.empty()) {
		params.push("email", email);
	}
	if (!phone.empty()) {
		params.push("phone", phone);
	}
	if (!domain.empty()) {
		params.push("domain", domain);
	}

	prepareAndSendRequest("accounts/public", "POST", params);
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountInfoByPhone(string phone) {
	prepareAndSendRequest(string("accounts/").append(phone).append("/info-by-phone"));
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountRecoverByPhone(string phone) {
	JsonParams params;
	params.push("phone", phone);
	prepareAndSendRequest(string("accounts/recover-by-phone"), "POST", params);
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountRecoverUsingRecoverKey(string sip, string recoverKey) {
	prepareAndSendRequest(string("accounts/").append(urlEncode(sip)).append("/recover/").append(recoverKey));
	return this;
}

/**
 * Authenticated endpoints
 */

FlexiAPIClient *FlexiAPIClient::me() {
	prepareAndSendRequest("accounts/me");
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountDelete() {
	prepareAndSendRequest("accounts/me", "DELETE");
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

	prepareAndSendRequest("accounts/me/password", "POST", params);
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountEmailChangeRequest(string email) {
	JsonParams params;
	params.push("email", email);
	prepareAndSendRequest("accounts/me/email/request", "POST", params);
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountDevices() {
	prepareAndSendRequest("accounts/me/devices");
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountDevice(string uuid) {
	prepareAndSendRequest(string("accounts/me/devices/").append(uuid));
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountContacts() {
	prepareAndSendRequest("accounts/me/contacts");
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountContact(string sip) {
	prepareAndSendRequest(string("accounts/me/contacts/").append(urlEncode(sip)));
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountPhoneChangeRequest(string phone) {
	JsonParams params;
	params.push("phone", phone);
	prepareAndSendRequest("accounts/me/phone/request", "POST", params);
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountPhoneChange(string code) {
	JsonParams params;
	params.push("code", code);
	prepareAndSendRequest("accounts/me/phone", "POST", params);
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountAuthTokenAttach(string authToken) {
	prepareAndSendRequest(string("accounts/auth_token/").append(authToken).append("/attach"));
	return this;
}

FlexiAPIClient *FlexiAPIClient::accountProvision() {
	prepareAndSendRequest("accounts/me/provision");
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
	return adminAccountCreate(username, password, algorithm, domain, activated, email, phone, "");
}

FlexiAPIClient *FlexiAPIClient::adminAccountCreate(string username, string password, string algorithm, string domain,
												   bool activated, string email, string phone, string dtmfProtocol) {
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
	if (!dtmfProtocol.empty()) {
		params.push("dtmf_protocol", dtmfProtocol);
	}
	prepareAndSendRequest("accounts", "POST", params);
	return this;
}

FlexiAPIClient *FlexiAPIClient::adminAccounts() {
	prepareAndSendRequest("accounts");
	return this;
}

FlexiAPIClient *FlexiAPIClient::adminAccountSearch(string sip) {
	prepareAndSendRequest(string("accounts/").append(urlEncode(sip).substr(6)).append("/search"));
	return this;
}

FlexiAPIClient *FlexiAPIClient::adminAccount(int id) {
	prepareAndSendRequest(string("accounts/").append(to_string(id)));
	return this;
}

FlexiAPIClient *FlexiAPIClient::adminAccountDelete(int id) {
	prepareAndSendRequest(string("accounts/").append(to_string(id)), "DELETE");
	return this;
}

FlexiAPIClient *FlexiAPIClient::adminAccountActivate(int id) {
	prepareAndSendRequest(string("accounts/").append(to_string(id)).append("/activate"));
	return this;
}

FlexiAPIClient *FlexiAPIClient::adminAccountDeactivate(int id) {
	prepareAndSendRequest(string("accounts/").append(to_string(id)).append("/deactivate"));
	return this;
}

FlexiAPIClient *FlexiAPIClient::adminAccountContacts(int id) {
	prepareAndSendRequest(string("accounts/").append(to_string(id)).append("/contacts"));
	return this;
}

FlexiAPIClient *FlexiAPIClient::adminAccountContactAdd(int id, int contactId) {
	prepareAndSendRequest(string("accounts/").append(to_string(id)).append("/contacts/").append(to_string(contactId)),
						  "POST");
	return this;
}

FlexiAPIClient *FlexiAPIClient::adminAccountContactDelete(int id, int contactId) {
	prepareAndSendRequest(string("accounts/").append(to_string(id)).append("/contacts/").append(to_string(contactId)),
						  "DELETE");
	return this;
}

/**
 * Authentication
 */

FlexiAPIClient *FlexiAPIClient::setApiKey(const char *key) {
	mApiKey = key;
	return this;
}

FlexiAPIClient *FlexiAPIClient::useTestAdminAccount(bool test) {
	mUseTestAdminAccount = test;
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

void FlexiAPIClient::prepareAndSendRequest(string path) {
	JsonParams params;
	prepareAndSendRequest(path, "GET", params);
}

void FlexiAPIClient::prepareAndSendRequest(string path, string type) {
	JsonParams params;
	prepareAndSendRequest(path, type, params);
}

void FlexiAPIClient::prepareAndSendRequest(string path, string type, JsonParams params) {
	mRequestCallbacks.mSelf = shared_from_this();
	belle_http_request_listener_callbacks_t internalCallbacks = {};
	belle_http_request_listener_t *listener;
	belle_http_request_t *req;

	const char *accountCreatorUrl = linphone_core_get_account_creator_url(mCore);
	string uri = accountCreatorUrl ? accountCreatorUrl : "";

	req = belle_http_request_create(type.c_str(), belle_generic_uri_parse(uri.append(path).c_str()),
									belle_sip_header_content_type_create("application", "json"),
									belle_sip_header_accept_create("application", "json"), NULL);
	if (!req) {
		lError() << "FlexiAPIClient cannot create a http request from [" << path << "] and config url [" << uri << "]";
		return;
	}
	LinphoneProxyConfig *cfg = linphone_core_get_default_proxy_config(mCore);
	if (mUseTestAdminAccount) {
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(req),
									 belle_http_header_create("From", "sip:admin_test@sip.example.org"));
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(req), belle_http_header_create("x-api-key", "no_secret_at_all"));
	} else if (cfg != nullptr) {
		char *addr = linphone_address_as_string_uri_only(linphone_proxy_config_get_identity_address(cfg));
		belle_sip_message_add_header(BELLE_SIP_MESSAGE(req), belle_http_header_create("From", addr));

		ms_free(addr);
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
	auto self = cb->mSelf; // Keep an instance of self.
	cb->mSelf = nullptr;   // Allow callbacks to reset cb if needed.

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
