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

#include "http/http-client.h"
#include "liblinphone_tester.h"

#include "http-server-utils.h"

// =============================================================================

using namespace std;

using namespace LinphonePrivate;

std::unique_ptr<HttpServerWithBearerAuth> httpServer;

static const char *serverRealm = "subscribe.example.org";
static const char *validToken = "xyzzyx";

static int before_suite() {
	httpServer.reset(new HttpServerWithBearerAuth(serverRealm, validToken));
	httpServer->addResource("/hello", "text/plain", "rcfiles/marie_sips_rc");
	return 0;
}

static int after_suite() {
	httpServer.reset();
	return 0;
}

static void http_get() {
	LinphoneCoreManager *lcm = linphone_core_manager_new("empty_rc");
	int responseCode = 0;
	int gotResponse = 0;
	LinphoneBearerToken *token =
	    linphone_factory_create_bearer_token(linphone_factory_get(), validToken, time(NULL) + 10);
	LinphoneAuthInfo *ai = linphone_factory_create_auth_info_3(linphone_factory_get(), nullptr, token, serverRealm);

	linphone_core_add_auth_info(lcm->lc, ai);
	linphone_auth_info_unref(ai);
	linphone_bearer_token_unref(token);

	HttpClient &httpClient = L_GET_CPP_PTR_FROM_C_OBJECT(lcm->lc)->getHttpClient();
	auto &httpRequest = httpClient.createRequest("GET", httpServer->getUrl() + "/hello");
	httpRequest.execute([&](const HttpResponse &response) {
		responseCode = response.getHttpStatusCode();
		gotResponse = 1;
	});
	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &gotResponse, 1));
	BC_ASSERT_EQUAL(responseCode, 200, int, "%i");

	linphone_core_manager_destroy(lcm);
}

static void http_bad_uri() {
	LinphoneCoreManager *lcm = linphone_core_manager_new("empty_rc");
	int gotException = 0;

	HttpClient &httpClient = L_GET_CPP_PTR_FROM_C_OBJECT(lcm->lc)->getHttpClient();

	try {
		auto &httpRequest = httpClient.createRequest("GET", "bob@example.net");
		(void)httpRequest;
	} catch (const std::exception &) {
		gotException = 1;
	}
	BC_ASSERT_TRUE(gotException == 1);

	linphone_core_manager_destroy(lcm);
}

static void _http_needs_token_refresh(bool expiration_is_known) {
	bellesip::HttpServer authorizationServer;

	authorizationServer.Post("/oauth/token", [](BCTBX_UNUSED(const httplib::Request &req), httplib::Response &res) {
		res.status = 200;
		constexpr const char *body = R"(
{
  "access_token":"xyzzyx",
  "token_type":"Bearer",
  "expires_in":3600,
  "refresh_token":"xyz2",
  "scope":"create"
}
						)";
		res.set_content(body, "application/json");
	});
	LinphoneCoreManager *lcm = linphone_core_manager_new("empty_rc");
	int responseCode = 0;
	int gotResponse = 0;
	LinphoneBearerToken *token = linphone_factory_create_bearer_token(linphone_factory_get(), "expired",
	                                                                  expiration_is_known ? time(NULL) - 10 : 0);
	LinphoneBearerToken *refreshToken = linphone_factory_create_bearer_token(linphone_factory_get(), "abcdefgh", 0);
	LinphoneAuthInfo *ai = linphone_factory_create_auth_info_3(linphone_factory_get(), nullptr, token, serverRealm);
	linphone_auth_info_set_refresh_token(ai, refreshToken);
	linphone_auth_info_set_token_endpoint_uri(ai, (authorizationServer.mRootUrl + "/oauth/token").c_str());

	linphone_core_add_auth_info(lcm->lc, ai);
	linphone_auth_info_unref(ai);
	linphone_bearer_token_unref(token);
	linphone_bearer_token_unref(refreshToken);

	HttpClient &httpClient = L_GET_CPP_PTR_FROM_C_OBJECT(lcm->lc)->getHttpClient();
	auto &httpRequest = httpClient.createRequest("GET", httpServer->getUrl() + "/hello");
	httpRequest.execute([&](const HttpResponse &response) {
		responseCode = response.getHttpStatusCode();
		gotResponse = 1;
	});
	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &gotResponse, 1));
	BC_ASSERT_EQUAL(responseCode, 200, int, "%i");

	linphone_core_manager_destroy(lcm);
}

static void http_needs_token_refresh() {
	_http_needs_token_refresh(true);
}

static void http_needs_token_refresh_but_expiration_is_not_known() {
	_http_needs_token_refresh(false);
}

// Disabled functionality and test: on-demand authentication for http is not supported, unless in Configuring global
// state. This may change in the future.
#if 0
static int auth_requested = 0;
static void on_auth_requested(LinphoneCore *lc, LinphoneAuthInfo *ai, LinphoneAuthMethod method){
    auth_requested++;
    (void)lc;
    (void)ai;
    (void)method;
}

static void challenged_requested_with_late_credentials() {
    LinphoneCoreManager *lcm = linphone_core_manager_new("empty_rc");
    LinphoneCoreCbs *coreCbs = linphone_factory_create_core_cbs(linphone_factory_get());
    int responseCode = 0;
    int gotResponse = 0;
    auth_requested = 0;

    linphone_core_cbs_set_authentication_requested(coreCbs, on_auth_requested);

    linphone_core_add_callbacks(lcm->lc, coreCbs);
    linphone_core_cbs_unref(coreCbs);

	HttpClient &httpClient = L_GET_CPP_PTR_FROM_C_OBJECT(lcm->lc)->getHttpClient();
    auto & httpRequest = httpClient.createRequest("GET", httpServer->getUrl() + "/hello");
    httpRequest.execute([&](const HttpResponse & response){
        responseCode = response.getHttpStatusCode();
        gotResponse = 1;
    });
	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &auth_requested, 1));
	BC_ASSERT_EQUAL(gotResponse, 0, int, "%i");
	LinphoneBearerToken *token =
	    linphone_factory_create_bearer_token(linphone_factory_get(), validToken, time(NULL) + 10);
	LinphoneAuthInfo *ai = linphone_factory_create_auth_info_3(linphone_factory_get(), nullptr, token, serverRealm);

	linphone_core_add_auth_info(lcm->lc, ai);
	linphone_auth_info_unref(ai);


	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &gotResponse, 1));
	BC_ASSERT_EQUAL(responseCode, 200, int, "%i");
	BC_ASSERT_EQUAL(auth_requested, 1, int, "%i");

	linphone_core_manager_destroy(lcm);
}
#endif

static void http_request_cancelled() {
	LinphoneCoreManager *lcm = linphone_core_manager_new("empty_rc");
	int responseCode = 0;
	int gotResponse = 0;
	LinphoneBearerToken *token =
	    linphone_factory_create_bearer_token(linphone_factory_get(), validToken, time(NULL) + 10);
	LinphoneAuthInfo *ai = linphone_factory_create_auth_info_3(linphone_factory_get(), nullptr, token, serverRealm);

	linphone_core_add_auth_info(lcm->lc, ai);
	linphone_auth_info_unref(ai);
	linphone_bearer_token_unref(token);

	HttpClient &httpClient = L_GET_CPP_PTR_FROM_C_OBJECT(lcm->lc)->getHttpClient();
	auto &httpRequest = httpClient.createRequest("GET", httpServer->getUrl() + "/hello");
	httpRequest.execute([&](const HttpResponse &response) {
		responseCode = response.getHttpStatusCode();
		gotResponse = 1;
	});
	for (int i = 0; i < 2; ++i)
		linphone_core_iterate(lcm->lc);
	BC_ASSERT_TRUE(gotResponse == 0);
	if (gotResponse == 0) {
		httpRequest.cancel();
		BC_ASSERT_FALSE(wait_for_until(lcm->lc, NULL, &gotResponse, 1, 1000));
		BC_ASSERT_EQUAL(responseCode, 0, int, "%i");
	}
	linphone_core_manager_destroy(lcm);
}

static void one_over_two_http_request_cancelled() {
	LinphoneCoreManager *lcm = linphone_core_manager_new("empty_rc");
	int responseCode1 = 0;
	int gotResponse1 = 0;
	int responseCode2 = 0;
	int gotResponse2 = 0;
	LinphoneBearerToken *token =
	    linphone_factory_create_bearer_token(linphone_factory_get(), validToken, time(NULL) + 10);
	LinphoneAuthInfo *ai = linphone_factory_create_auth_info_3(linphone_factory_get(), nullptr, token, serverRealm);

	linphone_core_add_auth_info(lcm->lc, ai);
	linphone_auth_info_unref(ai);
	linphone_bearer_token_unref(token);

	HttpClient &httpClient = L_GET_CPP_PTR_FROM_C_OBJECT(lcm->lc)->getHttpClient();
	auto &httpRequest1 = httpClient.createRequest("GET", httpServer->getUrl() + "/hello");
	httpRequest1.execute([&](const HttpResponse &response) {
		responseCode1 = response.getHttpStatusCode();
		gotResponse1 = 1;
	});

	auto &httpRequest2 = httpClient.createRequest("GET", httpServer->getUrl() + "/hello");
	httpRequest2.execute([&](const HttpResponse &response) {
		responseCode2 = response.getHttpStatusCode();
		gotResponse2 = 1;
	});

	for (int i = 0; i < 2; ++i)
		linphone_core_iterate(lcm->lc);
	BC_ASSERT_TRUE(gotResponse1 == 0); // normally it should not have enough time to get answered.
	if (gotResponse1 == 0) {
		httpRequest1.cancel(); // Cancel is possible only if not answered.
		BC_ASSERT_FALSE(wait_for_until(lcm->lc, NULL, &gotResponse1, 1, 1000));
		BC_ASSERT_EQUAL(responseCode1, 0, int, "%i");
	}

	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &gotResponse2, 1));
	BC_ASSERT_EQUAL(responseCode2, 200, int, "%i");

	linphone_core_manager_destroy(lcm);
}

static test_t http_client_tests[] = {
    {"Http get", http_get},
    {"Bad http uri", http_bad_uri},
    {"Http request needs a token refresh", http_needs_token_refresh},
    {"Http request needs a token refresh but expiration was not known",
     http_needs_token_refresh_but_expiration_is_not_known},
    //{"Challenged request, credentials comes later", challenged_requested_with_late_credentials},
    {"Http request cancelled", http_request_cancelled},
    {"One over two http request cancelled", one_over_two_http_request_cancelled}};

test_suite_t http_client_test_suite = {"HTTP Client",
                                       before_suite,
                                       after_suite,
                                       liblinphone_tester_before_each,
                                       liblinphone_tester_after_each,
                                       sizeof(http_client_tests) / sizeof(http_client_tests[0]),
                                       http_client_tests,
                                       3,
                                       0};
