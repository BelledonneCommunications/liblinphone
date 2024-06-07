/*
 * Copyright (c) 2024 Belledonne Communications SARL.
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

#include "http-client.h"
#include "private.h"

LINPHONE_BEGIN_NAMESPACE

std::unique_ptr<Json::CharReader> JsonDocument::sReader;

JsonDocument::JsonDocument(const Content &content) {
	Json::String err;
	std::string body = content.getBodyAsUtf8String();

	if (!getReader().parse(body.c_str(), body.c_str() + body.size(), &mValue, &err)) {
		lError() << "JsonDocument parse error: " << err;
	}
}

Json::CharReader &JsonDocument::getReader() {
	if (!sReader) {
		Json::CharReaderBuilder builder;
		sReader.reset(builder.newCharReader());
	}
	return *sReader;
}

HttpClient::HttpClient(const std::shared_ptr<Core> &core) : CoreAccessor(core) {
	LinphoneCore *lc = core->getCCore();
	/* Create the http provider in dual stack mode  if ipv6 enabled for sip (ipv4 and ipv6.
	 * If this creates problem, we may need to implement parallel ipv6/ ipv4 http requests in belle-sip.
	 * ipv6 config value is read later in fonction sip_config_read*/
	int use_ipv6_for_sip = linphone_config_get_int(lc->config, "sip", "use_ipv6", TRUE);
	/* TLS transports is always enabled, TCP can be disabled using the https_only flag in the configuration */
	uint8_t transports = BELLE_SIP_HTTP_TRANSPORT_TLS;
	if (linphone_config_get_bool(lc->config, "sip", "https_only", FALSE) == FALSE) {
		transports |= BELLE_SIP_HTTP_TRANSPORT_TCP;
	}
	mProvider = belle_sip_stack_create_http_provider_with_transports(
	    reinterpret_cast<belle_sip_stack_t *>(lc->sal->getStackImpl()), (use_ipv6_for_sip ? "::0" : "0.0.0.0"),
	    transports);
	mCryptoConfig = belle_tls_crypto_config_new();
	belle_http_provider_set_tls_crypto_config(mProvider, mCryptoConfig);
}

HttpClient::~HttpClient() {
	belle_sip_object_unref(mProvider);
	belle_sip_object_unref(mCryptoConfig);
}

HttpRequest &HttpClient::createRequest(const std::string &method, const std::string &uri) {
	HttpRequest *req = new HttpRequest(*this, method, uri);
	return *req;
}

HttpRequest::HttpRequest(HttpClient &client, const std::string &method, const std::string &uri) : mClient(client) {
	auto uriParsed = belle_generic_uri_parse(uri.c_str());
	if (!uriParsed) return;
	mRequest = belle_http_request_create(method.c_str(), uriParsed, nullptr);

	const char *core_user_agent = linphone_core_get_user_agent(client.getCore()->getCCore());
	belle_sip_message_add_header(BELLE_SIP_MESSAGE(mRequest), belle_sip_header_create("User-Agent", core_user_agent));

	belle_sip_object_ref(mRequest);
}

HttpRequest &HttpRequest::addHeader(const std::string &headerName, const std::string &headerValue) {
	belle_sip_message_add_header(BELLE_SIP_MESSAGE(mRequest),
	                             belle_sip_header_create(headerName.c_str(), L_STRING_TO_C(headerValue)));
	return *this;
}

HttpRequest &HttpRequest::setBody(const Content &content) {
	belle_sip_body_handler_t *bh = (belle_sip_body_handler_t *)Content::getBodyHandlerFromContent(content, false);
	belle_sip_message_set_body_handler(BELLE_SIP_MESSAGE(mRequest), bh);
	return *this;
}

HttpResponse::HttpResponse(Status status, belle_http_response_t *response) {
	mStatus = status;
	mResponse = response;
}

int HttpResponse::getStatusCode() const {
	return mResponse ? belle_http_response_get_status_code(mResponse) : 0;
}

std::string HttpResponse::getReason() const {
	if (mResponse) {
		return belle_http_response_get_reason_phrase(mResponse);
	}
	return std::string();
}

std::string HttpResponse::getHeaderValue(const std::string &headerName) const {
	if (mResponse) {
		auto header = belle_sip_message_get_header(BELLE_SIP_MESSAGE(mResponse), headerName.c_str());
		if (header) return belle_sip_header_get_unparsed_value(header);
	}
	return std::string();
}

const Content &HttpResponse::getBody() const {
	if (mBody.isEmpty()) {
		auto bh = belle_sip_message_get_body_handler(BELLE_SIP_MESSAGE(mResponse));
		if (bh) {
			mBody = Content((SalBodyHandler *)bh, false);
		}
		belle_sip_header_content_type_t *ct =
		    belle_sip_message_get_header_by_type(BELLE_SIP_MESSAGE(mResponse), belle_sip_header_content_type_t);
		if (ct) {
			mBody.setContentType(
			    ContentType(belle_sip_header_content_type_get_type(ct), belle_sip_header_content_type_get_subtype(ct)));
		}
	}
	return mBody;
}

HttpRequest::~HttpRequest() {
	belle_sip_object_unref(mRequest);
	if (mListener) belle_sip_object_unref(mListener);
}

void HttpRequest::execute(const ResponseHandler &responseHandler) {
	mResponseHandler = responseHandler;
	belle_http_request_listener_callbacks_t cbs{};
	cbs.process_auth_requested = HttpRequest::process_auth_requested;
	cbs.process_io_error = HttpRequest::process_io_error;
	cbs.process_timeout = HttpRequest::process_timeout;
	cbs.process_response = HttpRequest::process_response;
	cbs.process_response_headers = HttpRequest::process_response_headers;
	mListener = belle_http_request_listener_create_from_callbacks(&cbs, this);
	if (belle_http_provider_send_request(mClient.getProvider(), mRequest, mListener) != 0) {
		HttpResponse response(HttpResponse::InvalidRequest);
		if (mResponseHandler) mResponseHandler(response);
		delete this;
	}
}

void HttpRequest::processResponseHeaders(BCTBX_UNUSED(const belle_http_response_event_t *event)) {
	// TODO
}

void HttpRequest::processResponse(const belle_http_response_event_t *event) {
	HttpResponse response(HttpResponse::Valid, event->response);
	if (mResponseHandler) mResponseHandler(response);
	delete this;
}

void HttpRequest::processTimeout(BCTBX_UNUSED(const belle_sip_timeout_event_t *event)) {
	HttpResponse response(HttpResponse::Timeout);
	if (mResponseHandler) mResponseHandler(response);
	delete this;
}

void HttpRequest::processIOError(BCTBX_UNUSED(const belle_sip_io_error_event_t *event)) {
	HttpResponse response(HttpResponse::IOError);
	if (mResponseHandler) mResponseHandler(response);
	delete this;
}

void HttpRequest::processAuthRequested(belle_sip_auth_event_t *event) {
	try {
		auto core = mClient.getCore();
		if (linphone_core_fill_belle_sip_auth_event(core->getCCore(), event, NULL, NULL)) return;
	} catch (...) {
	}
	// TODO check what happens if auth info is not supplied.
}

void HttpRequest::process_response_headers(void *user_ctx, const belle_http_response_event_t *event) {
	static_cast<HttpRequest *>(user_ctx)->processResponseHeaders(event);
}

void HttpRequest::process_response(void *user_ctx, const belle_http_response_event_t *event) {
	static_cast<HttpRequest *>(user_ctx)->processResponse(event);
}

void HttpRequest::process_io_error(void *user_ctx, const belle_sip_io_error_event_t *event) {
	static_cast<HttpRequest *>(user_ctx)->processIOError(event);
}

void HttpRequest::process_timeout(void *user_ctx, const belle_sip_timeout_event_t *event) {
	static_cast<HttpRequest *>(user_ctx)->processTimeout(event);
}

void HttpRequest::process_auth_requested(void *user_ctx, belle_sip_auth_event_t *event) {
	static_cast<HttpRequest *>(user_ctx)->processAuthRequested(event);
}

LINPHONE_END_NAMESPACE
