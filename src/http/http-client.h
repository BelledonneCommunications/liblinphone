/*
 * Copyright (c) 2024-2025 Belledonne Communications SARL.
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

#ifndef http_client_h
#define http_client_h

#include "content/content.h"
#include "core/core.h"
#include "json/json.h"

#include <functional>

LINPHONE_BEGIN_NAMESPACE

class HttpClient;
class HttpRequest;

class JsonDocument {
public:
	JsonDocument(const Content &content);
	const Json::Value &getRoot() const {
		return mValue;
	}
	// TODO:
	Content toContent() const;

private:
	Json::Value mValue;
	static Json::CharReader &getReader();
	static std::unique_ptr<Json::CharReader> sReader;
};

class LINPHONE_PUBLIC HttpResponse {
	friend class HttpRequest;

public:
	enum Status { InvalidRequest, Valid, IOError, Timeout };
	HttpResponse(const HttpResponse &) = delete;
	HttpResponse(HttpResponse &&) = delete;
	// Overall status of the response, which may be a non-response in case of IOError, Timeout, or
	// invalid request (case where it could even not be sent).
	Status getStatus() const {
		return mStatus;
	}
	// Return the http response code.
	int getHttpStatusCode() const;
	std::string getReason() const;
	std::string getHeaderValue(const std::string &headerName) const;
	const Content &getBody() const;

private:
	HttpResponse(Status status, belle_http_response_t *response = nullptr);
	belle_http_response_t *mResponse;
	Status mStatus = Timeout;
	mutable Content mBody;
};

class LINPHONE_PUBLIC HttpRequest {
	friend class HttpClient;

public:
	using ResponseHandler = std::function<void(const HttpResponse &)>;
	HttpRequest(const HttpRequest &) = delete;
	HttpRequest(HttpRequest &&) = delete;
	/* Add a header */
	HttpRequest &addHeader(const std::string &headerName, const std::string &headerValue);
	HttpRequest &setBody(const Content &content);
	/* Execute the request, ie send it and upon response execute the responseHandler lambda.*/
	void execute(const ResponseHandler &responseHandler);
	void cancel();
	void setAuthInfo(const std::string &username, const std::string &domain);

private:
	void abortAuthentication();
	void send();
	void restart();
	~HttpRequest();
	void processResponse(const belle_http_response_event_t *event);
	void processResponseHeaders(const belle_http_response_event_t *event);
	void processTimeout(const belle_sip_timeout_event_t *event);
	void processIOError(const belle_sip_io_error_event_t *event);
	void processAuthRequested(belle_sip_auth_event_t *event);
	static void process_response_headers(void *user_ctx, const belle_http_response_event_t *event);
	static void process_response(void *user_ctx, const belle_http_response_event_t *event);
	static void process_io_error(void *user_ctx, const belle_sip_io_error_event_t *event);
	static void process_timeout(void *user_ctx, const belle_sip_timeout_event_t *event);
	static void process_auth_requested(void *user_ctx, belle_sip_auth_event_t *event);
	HttpRequest(HttpClient &client, const std::string &method, const std::string &uri);
	HttpClient &mClient;
	ResponseHandler mResponseHandler;
	belle_http_request_t *mRequest;
	belle_http_request_listener_t *mListener = nullptr;
	bool mAuthPending = false;
	std::string mAuthUsername;
	std::string mAuthDomain;
};

class LINPHONE_PUBLIC HttpClient : public CoreAccessor {
	friend class HttpRequest;

public:
	HttpClient(const std::shared_ptr<Core> &core);
	HttpClient(const HttpClient &other) = delete;
	~HttpClient();
	/* Create a http request, throws exception if it cannot be constructed. */
	HttpRequest &createRequest(const std::string &method, const std::string &uri);
	/*
	 * This method is provided for backward compatibility with code written before the HttpClient
	 * in order to send their belle_http_request_t directly.
	 * New code should use the HttpClient/HttpRequest only.
	 */
	belle_http_provider_t *getProvider() const {
		return mProvider;
	};
	belle_tls_crypto_config_t *getCryptoConfig() {
		return mCryptoConfig;
	}
	/*
	 * Restarts http requests that were awaiting authentication information.
	 * Returns the number of requests that were re-started.
	 */
	size_t retryPendingRequests();
	size_t abortPendingRequests();

private:
	void postpone(HttpRequest &req);
	std::list<HttpRequest *> mRequestsAwaitingAuth;
	belle_http_provider_t *mProvider = nullptr;
	belle_tls_crypto_config_t *mCryptoConfig = nullptr;
};

LINPHONE_END_NAMESPACE

#endif
