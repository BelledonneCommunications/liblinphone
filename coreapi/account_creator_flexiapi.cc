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

#include "account_creator_flexiapi.hh"

#include "c-wrapper/c-wrapper.h"
#include "dial-plan/dial-plan.h"

#include "bctoolbox/crypto.h"
#include "bctoolbox/regex.h"

// TODO: From coreapi. Remove me later.
#include "private.h"

#include <functional>

using namespace LinphonePrivate;
using namespace std;

AccountCreatorFlexiAPI::AccountCreatorFlexiAPI(LinphoneCore *lc) {
    core = lc;
    counter = 0;

    /*LinphoneAccountCreatorService *service = linphone_account_creator_service_new();
    linphone_account_creator_service_set_is_account_exist_cb(service, AccountCreatorFlexiAPI::account_creator_flexiapi_ping);

    linphone_core_set_account_creator_service(core, service);

    LinphoneAccountCreator *creator = linphone_account_creator_new(core, "");

    if (linphone_account_creator_is_account_exist(creator)) {
        ms_message("Error with the request is_account_exist");
    }*/
}

void AccountCreatorFlexiAPI::ping(function<void (const char *)> success) {
    callbacks requestCallbacks;
    requestCallbacks.success = success;

    belle_http_request_listener_callbacks_t internalCallbacks = {};
    belle_http_request_listener_t *listener;
    //belle_generic_uri_t *uri;
    belle_http_request_t *req;

    //belle_sip_memory_body_handler_t *bh;
    //const char *data;

    LinphoneProxyConfig *cfg = linphone_core_get_default_proxy_config(core);
    char *addr = linphone_address_as_string_uri_only(linphone_proxy_config_get_identity_address(cfg));

    req = belle_http_request_create("GET", belle_generic_uri_parse("https://subscribe.linphone.org/api/ping"),
    belle_sip_header_content_type_create("application", "json"),
        belle_sip_header_accept_create("application", "json"),
        belle_http_header_create("From", addr),
    NULL);
    bctbx_free(addr);

    /*data = linphone_xml_rpc_request_get_content(request);
    bh = belle_sip_memory_body_handler_new_copy_from_buffer(data, strlen(data), NULL, NULL);
    belle_sip_message_set_body_handler(BELLE_SIP_MESSAGE(req), BELLE_SIP_BODY_HANDLER(bh));*/
    internalCallbacks.process_response = processResponse;
    listener = belle_http_request_listener_create_from_callbacks(&internalCallbacks, &requestCallbacks);

    belle_http_provider_send_request(core->http_provider, req, listener);

    /*ensure that the listener object will be destroyed with the request*/
    belle_sip_object_data_set(BELLE_SIP_OBJECT(req), "listener", listener, belle_sip_object_unref);
};

void AccountCreatorFlexiAPI::processResponse(void *ctx, const belle_http_response_event_t *event) {
    auto cb = (callbacks_t *)ctx;

    if (event->response){
        int code = belle_http_response_get_status_code(event->response);
        belle_sip_body_handler_t *body = belle_sip_message_get_body_handler(BELLE_SIP_MESSAGE(event->response));
        const char *content = belle_sip_object_to_string(body);

        if (code >= 200 && code < 300) {
            cb->success(content);
        }
    }
}