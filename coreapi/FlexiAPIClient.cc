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

#include "linphone/account_creator.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"

#include "FlexiAPIClient.hh"

#include "c-wrapper/c-wrapper.h"
#include "dial-plan/dial-plan.h"

#include "bctoolbox/crypto.h"
#include "bctoolbox/regex.h"

#include "private.h"

#include <jsoncpp/json/json.h>
#include <functional>

using namespace LinphonePrivate;
using namespace std;

FlexiAPIClient::FlexiAPIClient(LinphoneCore *lc) {
    mCore = lc;
    apiKey = nullptr;

    // Assign the core there as well to keep it in the callback contexts
    mRequestCallbacks.core = lc;
}

/**
 * Endpoints
 */

FlexiAPIClient* FlexiAPIClient::ping() {
    prepareRequest("ping");
    return this;
}

FlexiAPIClient* FlexiAPIClient::emailChange(string email) {
    JsonParams params;
    params.push("email", email);
    prepareRequest("accounts/email/request", "POST", params);
    return this;
}

/**
 * Change the account password
 * @param [in] algorithm can be SHA-256 or MD5
 * @param [in] the new password
 * @param [in] the old password if already set
 */

FlexiAPIClient* FlexiAPIClient::passwordChange(string algorithm, string password) {
    return passwordChange(algorithm, password, "");
}

FlexiAPIClient* FlexiAPIClient::passwordChange(string algorithm, string password, string oldPassword) {
    JsonParams params;
    params.push("algorithm", algorithm);
    params.push("password", password);

    if (!oldPassword.empty()) {
        params.push("old_password", oldPassword);
    }
    prepareRequest("accounts/password", "POST", params);
    return this;
}

FlexiAPIClient* FlexiAPIClient::me() {
    prepareRequest("accounts/me");
    return this;
}

/**
 * Admin endpoints
 */

FlexiAPIClient* FlexiAPIClient::createAccount(
    string username,
    string password,
    string algorithm
) {
    return createAccount(username, password, algorithm, "", false);
}

FlexiAPIClient* FlexiAPIClient::createAccount(
    string username,
    string password,
    string algorithm,
    string domain
) {
    return createAccount(username, password, algorithm, domain, false);
}

FlexiAPIClient* FlexiAPIClient::createAccount(
    string username,
    string password,
    string algorithm,
    bool activated
) {
    return createAccount(username, password, algorithm, "", activated);
}

FlexiAPIClient* FlexiAPIClient::createAccount(
    string username,
    string password,
    string algorithm,
    string domain,
    bool activated
) {
    JsonParams params;
    params.push("username", username);
    params.push("password", password);
    params.push("algorithm", algorithm);
    params.push("activated", to_string(activated));

    if (!domain.empty()) {
        params.push("domain", domain);
    }
    prepareRequest("accounts", "POST", params);
    return this;
}

FlexiAPIClient* FlexiAPIClient::accounts() {
    prepareRequest("accounts");
    return this;
}

FlexiAPIClient* FlexiAPIClient::accountDelete(int id) {
    prepareRequest(string("accounts/").append(to_string(id)).c_str(), "DELETE");
    return this;
}

FlexiAPIClient* FlexiAPIClient::account(int id) {
    prepareRequest(string("accounts/").append(to_string(id)).c_str());
    return this;
}

FlexiAPIClient* FlexiAPIClient::accountActivate(int id) {
    prepareRequest(string("accounts/").append(to_string(id)).append("/activate").c_str());
    return this;
}

FlexiAPIClient* FlexiAPIClient::accountDeactivate(int id) {
    prepareRequest(string("accounts/").append(to_string(id)).append("/deactivate").c_str());
    return this;
}

/**
 * Authentication
 */

FlexiAPIClient* FlexiAPIClient::setApiKey(const char* key) {
    apiKey = key;
    return this;
}

/**
 * Callback requests
 */

FlexiAPIClient* FlexiAPIClient::then(function<void (FlexiAPIClient::Response)> success) {
    mRequestCallbacks.success = success;
    return this;
}

FlexiAPIClient* FlexiAPIClient::error(function<void (FlexiAPIClient::Response)> error) {
    mRequestCallbacks.error = error;
    return this;
}

void FlexiAPIClient::prepareRequest(const char* path) {
    JsonParams params;
    prepareRequest(path, "GET", params);
}

void FlexiAPIClient::prepareRequest(const char* path, string type) {
    JsonParams params;
    prepareRequest(path, type, params);
}

void FlexiAPIClient::prepareRequest(const char* path, string type, JsonParams params) {
    belle_http_request_listener_callbacks_t internalCallbacks = {};
    belle_http_request_listener_t *listener;
    belle_http_request_t *req;

    string uri = linphone_config_get_string(mCore->config, "sip", "flexiapi_url", NULL);

    LinphoneProxyConfig *cfg = linphone_core_get_default_proxy_config(mCore);
    char *addr = linphone_address_as_string_uri_only(linphone_proxy_config_get_identity_address(cfg));

    req = belle_http_request_create(
        type.c_str(),
        belle_generic_uri_parse(uri.append(path).c_str()),
        belle_sip_header_content_type_create("application", "json"),
        belle_sip_header_accept_create("application", "json"),
        belle_http_header_create("From", addr),
    NULL);

    if (!params.empty()) {
        string body = params.json();
        belle_sip_message_set_body(BELLE_SIP_MESSAGE(req), body.c_str(), body.size());
    }

    if (apiKey != nullptr) {
        belle_sip_message_add_header(
            BELLE_SIP_MESSAGE(req),
            belle_http_header_create("x-api-key", apiKey)
        );
    }

    internalCallbacks.process_response = processResponse;
    internalCallbacks.process_auth_requested = processAuthRequested;
    listener = belle_http_request_listener_create_from_callbacks(&internalCallbacks, &mRequestCallbacks);

    belle_http_provider_send_request(mCore->http_provider, req, listener);
    belle_sip_object_data_set(BELLE_SIP_OBJECT(req), "listener", listener, belle_sip_object_unref);
}

void FlexiAPIClient::processResponse(void *ctx, const belle_http_response_event_t *event) {
    auto cb = (Callbacks *)ctx;
    FlexiAPIClient::Response response;

    if (event->response){
        int code = belle_http_response_get_status_code(event->response);
        response.code = code;

        if (code >= 200 && code < 300) {
            belle_sip_body_handler_t *body = belle_sip_message_get_body_handler(BELLE_SIP_MESSAGE(event->response));
            const char *content = belle_sip_object_to_string(body);

            response.body = content;
            if (cb->success) {
                cb->success(response);
            }
        } else if(cb->error) {
            cb->error(response);
        }
    }
}

void FlexiAPIClient::processAuthRequested(void *ctx, belle_sip_auth_event_t *event) {
    auto cb = (Callbacks *)ctx;
    const char *username = belle_sip_auth_event_get_username(event);
    const char *domain = belle_sip_auth_event_get_domain(event);

    linphone_core_fill_belle_sip_auth_event(cb->core, event, username, domain);
}
