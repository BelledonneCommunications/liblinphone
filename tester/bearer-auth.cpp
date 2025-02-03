/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#include "belle_sip_tester_utils.h"

#include "http-server-utils.h"
#include "liblinphone_tester.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-auth-info.h"
#include "linphone/api/c-bearer-token.h"

static LinphoneAuthInfo *proposedAuthInfo = nullptr;

static void bearer_auth_info_requested(LinphoneCore *lc, LinphoneAuthInfo *info, LinphoneAuthMethod method) {
	bctbx_message("bearer_auth_info_requested()");
	BC_ASSERT_PTR_NOT_NULL(lc);
	BC_ASSERT_PTR_NOT_NULL(info);
	BC_ASSERT_TRUE(method == LinphoneAuthBearer);
	BC_ASSERT_STRING_EQUAL(linphone_auth_info_get_authorization_server(info), "auth.example.org");
	if (!BC_ASSERT_PTR_NULL(proposedAuthInfo)) return;

	proposedAuthInfo = linphone_auth_info_ref(info);
}

static void test_bearer_auth(void) {

	bellesip::AuthenticatedRegistrar registrar({"sip:bob@sip.example.com;access_token=abcdefgh"}, "sip.example.com",
	                                           "tcp");
	registrar.addAuthMode(BELLE_SIP_AUTH_MODE_HTTP_BEARER);
	registrar.setAuthzServer("auth.example.org");

	LinphoneCoreManager *lcm = linphone_core_manager_new("empty_rc");
	LinphoneAccountParams *account_params = linphone_core_create_account_params(lcm->lc);
	LinphoneAddress *identity = linphone_factory_create_address(linphone_factory_get(), "sip:bob@sip.example.com");
	LinphoneAddress *server_addr =
	    linphone_factory_create_address(linphone_factory_get(), registrar.getAgent().getListeningUriAsString().c_str());
	linphone_account_params_set_identity_address(account_params, identity);
	linphone_account_params_set_server_address(account_params, server_addr);
	linphone_account_params_enable_register(account_params, TRUE);
	linphone_address_unref(identity);
	linphone_address_unref(server_addr);

	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_authentication_requested(cbs, bearer_auth_info_requested);
	linphone_core_add_callbacks(lcm->lc, cbs);

	LinphoneAccount *account = linphone_core_create_account(lcm->lc, account_params);
	linphone_core_add_account(lcm->lc, account);
	linphone_account_params_unref(account_params);
	linphone_account_unref(account);

	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_authentication_info_requested, 1));
	if (BC_ASSERT_PTR_NOT_NULL(proposedAuthInfo)) {
		LinphoneBearerToken *token =
		    linphone_factory_create_bearer_token(linphone_factory_get(), "abcdefgh", time(NULL) + 30);
		linphone_auth_info_set_access_token(proposedAuthInfo, token);
		linphone_bearer_token_unref(token);
		linphone_core_add_auth_info(lcm->lc, proposedAuthInfo);
		bctbx_message("Added AuthInfo.");
		BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_LinphoneRegistrationOk, 1));
		linphone_auth_info_unref(proposedAuthInfo);
		proposedAuthInfo = nullptr;
	}

	linphone_core_manager_destroy(lcm);
	linphone_core_cbs_unref(cbs);
}

static void test_bearer_auth_set_before(void) {

	bellesip::AuthenticatedRegistrar registrar({"sip:bob@sip.example.com;access_token=abcdefgh"}, "sip.example.com",
	                                           "tcp");
	registrar.addAuthMode(BELLE_SIP_AUTH_MODE_HTTP_BEARER);

	LinphoneCoreManager *lcm = linphone_core_manager_new("empty_rc");
	LinphoneAccountParams *account_params = linphone_core_create_account_params(lcm->lc);
	LinphoneAddress *identity = linphone_factory_create_address(linphone_factory_get(), "sip:bob@sip.example.com");
	LinphoneAddress *server_addr =
	    linphone_factory_create_address(linphone_factory_get(), registrar.getAgent().getListeningUriAsString().c_str());
	linphone_account_params_set_identity_address(account_params, identity);
	linphone_account_params_set_server_address(account_params, server_addr);
	linphone_account_params_enable_register(account_params, TRUE);
	linphone_address_unref(server_addr);

	LinphoneAccount *account = linphone_core_create_account(lcm->lc, account_params);
	linphone_core_add_account(lcm->lc, account);
	linphone_account_params_unref(account_params);
	linphone_account_unref(account);

	LinphoneBearerToken *token =
	    linphone_factory_create_bearer_token(linphone_factory_get(), "abcdefgh", time(NULL) + 30);
	LinphoneAuthInfo *ai = linphone_factory_create_auth_info_3(
	    linphone_factory_get(), linphone_address_get_username(identity), token, "sip.example.com");
	linphone_bearer_token_unref(token);
	linphone_core_add_auth_info(lcm->lc, ai);
	linphone_auth_info_unref(ai);
	bctbx_message("Added AuthInfo.");
	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_LinphoneRegistrationOk, 1));
	BC_ASSERT_EQUAL(lcm->stat.number_of_authentication_info_requested, 0, int, "%i");

	linphone_address_unref(identity);
	linphone_core_manager_destroy(lcm);
}

static void provisioning_with_http_bearer_auth(void) {
	HttpServerWithBearerAuth httpServer("sip.example.org", "abcdefgh");
	httpServer.addResource("/provisioning", "application/xml", "rcfiles/marie_xml");

	bellesip::AuthenticatedRegistrar registrar({"sip:marie@sip.example.com;access_token=abcdefgh"}, "sip.example.org",
	                                           "tcp");
	registrar.addAuthMode(BELLE_SIP_AUTH_MODE_HTTP_BEARER);

	LinphoneCoreManager *lcm = linphone_core_manager_new("empty_rc");

	std::string provisioningUri = httpServer.getUrl() + "/provisioning";

	/* set access token and provisioning uri */
	LinphoneBearerToken *token =
	    linphone_factory_create_bearer_token(linphone_factory_get(), "abcdefgh", time(NULL) + 30);
	LinphoneAuthInfo *ai =
	    linphone_factory_create_auth_info_3(linphone_factory_get(), "marie", token, "sip.example.org");
	linphone_auth_info_set_access_token(ai, token);
	linphone_bearer_token_unref(token);
	linphone_core_add_auth_info(lcm->lc, ai);
	linphone_auth_info_unref(ai);

	linphone_core_set_provisioning_uri(lcm->lc, provisioningUri.c_str());
	linphone_core_stop(lcm->lc);
	linphone_core_start(lcm->lc);
	linphone_core_manager_setup_dns(lcm);
	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_LinphoneGlobalOn, 2));

	/* modify the registrar uri to setup correct port number */
	LinphoneAccount *account = linphone_core_get_default_account(lcm->lc);
	if (BC_ASSERT_PTR_NOT_NULL(account)) {
		LinphoneAccountParams *params = linphone_account_params_clone(linphone_account_get_params(account));
		LinphoneAddress *addr = linphone_address_clone(linphone_account_params_get_server_address(params));
		linphone_address_set_port(addr, belle_sip_uri_get_port(registrar.getAgent().getListeningUri()));
		linphone_account_params_set_server_address(params, addr);
		linphone_account_params_enable_register(params, TRUE);
		linphone_address_unref(addr);
		linphone_account_set_params(account, params);
		linphone_account_params_unref(params);
	}
	BC_ASSERT_TRUE(wait_for(lcm->lc, nullptr, &lcm->stat.number_of_LinphoneRegistrationOk, 1));
	BC_ASSERT_EQUAL(lcm->stat.number_of_authentication_info_requested, 0, int, "%i");
	linphone_core_manager_destroy(lcm);
}

static void bearer_auth_info_requested_2(LinphoneCore *lc, LinphoneAuthInfo *info, LinphoneAuthMethod method) {
	bctbx_message("bearer_auth_info_requested_2()");
	BC_ASSERT_PTR_NOT_NULL(lc);
	BC_ASSERT_PTR_NOT_NULL(info);
	BC_ASSERT_TRUE(method == LinphoneAuthBearer);
	BC_ASSERT_STRING_EQUAL(linphone_auth_info_get_authorization_server(info), "auth.example.org");
}

static void provisioning_with_http_bearer_auth_requested_on_demand(void) {
	HttpServerWithBearerAuth httpServer("sip.example.org", "abcdefgh");
	httpServer.addResource("/provisioning", "application/xml", "rcfiles/marie_xml");

	bellesip::AuthenticatedRegistrar registrar({"sip:marie@sip.example.com;access_token=abcdefgh"}, "sip.example.org",
	                                           "tcp");
	registrar.addAuthMode(BELLE_SIP_AUTH_MODE_HTTP_BEARER);

	LinphoneCoreManager *lcm = linphone_core_manager_new("empty_rc");

	std::string provisioningUri = httpServer.getUrl() + "/provisioning";

	/* set provisioning uri and attempt to load it */
	linphone_core_set_provisioning_uri(lcm->lc, provisioningUri.c_str());

	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_authentication_requested(cbs, bearer_auth_info_requested_2);
	linphone_core_add_callbacks(lcm->lc, cbs);

	linphone_core_stop(lcm->lc);
	linphone_core_start(lcm->lc);
	linphone_core_manager_setup_dns(lcm);
	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_authentication_info_requested, 1));
	BC_ASSERT_EQUAL(lcm->stat.number_of_LinphoneConfiguringFailed, 0, int, "%i");
	BC_ASSERT_EQUAL(lcm->stat.number_of_LinphoneConfiguringSuccessful, 0, int, "%i");

	/* set access token */
	LinphoneBearerToken *token =
	    linphone_factory_create_bearer_token(linphone_factory_get(), "abcdefgh", time(NULL) + 30);
	LinphoneAuthInfo *ai =
	    linphone_factory_create_auth_info_3(linphone_factory_get(), "marie", token, "sip.example.org");
	linphone_auth_info_set_access_token(ai, token);
	linphone_bearer_token_unref(token);
	linphone_core_add_auth_info(lcm->lc, ai);
	linphone_auth_info_unref(ai);

	/* the provisioning shall resume */
	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_LinphoneConfiguringSuccessful, 1));
	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_LinphoneGlobalOn, 2));

	/* modify the registrar uri to setup correct port number */
	LinphoneAccount *account = linphone_core_get_default_account(lcm->lc);
	if (BC_ASSERT_PTR_NOT_NULL(account)) {
		LinphoneAccountParams *params = linphone_account_params_clone(linphone_account_get_params(account));
		LinphoneAddress *addr = linphone_address_clone(linphone_account_params_get_server_address(params));
		linphone_address_set_port(addr, belle_sip_uri_get_port(registrar.getAgent().getListeningUri()));
		linphone_account_params_set_server_address(params, addr);
		linphone_account_params_enable_register(params, TRUE);
		linphone_address_unref(addr);
		linphone_account_set_params(account, params);
		linphone_account_params_unref(params);
	}
	BC_ASSERT_TRUE(wait_for(lcm->lc, nullptr, &lcm->stat.number_of_LinphoneRegistrationOk, 1));
	BC_ASSERT_EQUAL(lcm->stat.number_of_authentication_info_requested, 1, int, "%i");
	linphone_core_cbs_unref(cbs);
	linphone_core_manager_destroy(lcm);
}

static void provisioning_with_http_bearer_auth_requested_on_demand_aborted(void) {
	HttpServerWithBearerAuth httpServer("sip.example.org", "abcdefgh");
	httpServer.addResource("/provisioning", "application/xml", "rcfiles/marie_xml");

	bellesip::AuthenticatedRegistrar registrar({"sip:marie@sip.example.com;access_token=abcdefgh"}, "sip.example.org",
	                                           "tcp");
	registrar.addAuthMode(BELLE_SIP_AUTH_MODE_HTTP_BEARER);

	LinphoneCoreManager *lcm = linphone_core_manager_new("empty_rc");

	std::string provisioningUri = httpServer.getUrl() + "/provisioning";

	/* set provisioning uri and attempt to load it */
	linphone_core_set_provisioning_uri(lcm->lc, provisioningUri.c_str());

	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_authentication_requested(cbs, bearer_auth_info_requested_2);
	linphone_core_add_callbacks(lcm->lc, cbs);

	linphone_core_stop(lcm->lc);
	linphone_core_start(lcm->lc);
	linphone_core_manager_setup_dns(lcm);
	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_authentication_info_requested, 1));
	BC_ASSERT_EQUAL(lcm->stat.number_of_LinphoneConfiguringFailed, 0, int, "%i");
	BC_ASSERT_EQUAL(lcm->stat.number_of_LinphoneConfiguringSuccessful, 0, int, "%i");

	/* abort authentication */
	linphone_core_abort_authentication(lcm->lc, NULL);

	/* the provisioning shall resume anyway*/
	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_LinphoneConfiguringFailed, 1));
	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_LinphoneGlobalOn, 2));

	BC_ASSERT_EQUAL(lcm->stat.number_of_authentication_info_requested, 1, int, "%i");
	linphone_core_cbs_unref(cbs);
	linphone_core_manager_destroy(lcm);
}

static void provisioning_with_http_bearer_expired_token() {
	bellesip::HttpServer authorizationServer;
	HttpServerWithBearerAuth httpServer("sip.example.org", "abcdefgh1");
	httpServer.addResource("/provisioning", "application/xml", "rcfiles/marie_xml");

	bellesip::AuthenticatedRegistrar registrar({"sip:marie@sip.example.com;access_token=abcdefgh1"}, "sip.example.org",
	                                           "tcp");
	registrar.addAuthMode(BELLE_SIP_AUTH_MODE_HTTP_BEARER);

	authorizationServer.Post("/oauth/token", [](BCTBX_UNUSED(const httplib::Request &req), httplib::Response &res) {
		res.status = 200;
		constexpr const char *body = R"(
{
  "access_token":"abcdefgh1",
  "token_type":"Bearer",
  "expires_in":3600,
  "refresh_token":"xyz2",
  "scope":"create"
}
						)";
		res.set_content(body, "application/json");
	});

	LinphoneCoreManager *lcm = linphone_core_manager_new("empty_rc");

	std::string provisioningUri = httpServer.getUrl() + "/provisioning";

	/* set access token and provisioning uri */
	LinphoneBearerToken *token =
	    linphone_factory_create_bearer_token(linphone_factory_get(), "expiredtoken", time(NULL) - 30);
	LinphoneBearerToken *refresh_token =
	    linphone_factory_create_bearer_token(linphone_factory_get(), "xyz", time(NULL) + 3600);
	LinphoneAuthInfo *ai =
	    linphone_factory_create_auth_info_3(linphone_factory_get(), "marie", token, "sip.example.org");
	linphone_auth_info_set_access_token(ai, token);
	linphone_auth_info_set_refresh_token(ai, refresh_token);
	linphone_auth_info_set_token_endpoint_uri(ai, (authorizationServer.mRootUrl + "/oauth/token").c_str());
	linphone_bearer_token_unref(token);
	linphone_bearer_token_unref(refresh_token);
	linphone_core_add_auth_info(lcm->lc, ai);
	linphone_auth_info_unref(ai);

	/* set provisioning uri and attempt to load it */
	linphone_core_set_provisioning_uri(lcm->lc, provisioningUri.c_str());

	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_authentication_requested(cbs, bearer_auth_info_requested_2);
	linphone_core_add_callbacks(lcm->lc, cbs);

	/* now restart the provisioning */
	linphone_core_stop(lcm->lc);
	linphone_core_start(lcm->lc);

	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_LinphoneConfiguringSuccessful, 1));
	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_LinphoneGlobalOn, 2));
	/* Make sure that bearer token is updated */
	const bctbx_list_t *ai_list = linphone_core_get_auth_info_list(lcm->lc);
	if (BC_ASSERT_PTR_NOT_NULL(ai_list)) {
		LinphoneAuthInfo *bai = (LinphoneAuthInfo *)ai_list->data;
		const LinphoneBearerToken *access_token = linphone_auth_info_get_access_token(bai);
		BC_ASSERT_STRING_EQUAL(linphone_bearer_token_get_token(access_token), "abcdefgh1");
	}
	BC_ASSERT_EQUAL(lcm->stat.number_of_authentication_info_requested, 0, int, "%i");
	linphone_core_manager_setup_dns(lcm);

	/* modify the registrar uri to setup correct port number */
	LinphoneAccount *account = linphone_core_get_default_account(lcm->lc);
	if (BC_ASSERT_PTR_NOT_NULL(account)) {
		LinphoneAccountParams *params = linphone_account_params_clone(linphone_account_get_params(account));
		LinphoneAddress *addr = linphone_address_clone(linphone_account_params_get_server_address(params));
		linphone_address_set_port(addr, belle_sip_uri_get_port(registrar.getAgent().getListeningUri()));
		linphone_account_params_set_server_address(params, addr);
		linphone_account_params_enable_register(params, TRUE);
		linphone_address_unref(addr);
		linphone_account_set_params(account, params);
		linphone_account_params_unref(params);
	}
	BC_ASSERT_TRUE(wait_for(lcm->lc, nullptr, &lcm->stat.number_of_LinphoneRegistrationOk, 1));
	BC_ASSERT_EQUAL(lcm->stat.number_of_authentication_info_requested, 0, int, "%i");
	ai_list = linphone_core_get_auth_info_list(lcm->lc);
	BC_ASSERT_EQUAL((int)bctbx_list_size(ai_list), 1, int, "%i");
	linphone_core_cbs_unref(cbs);
	linphone_core_manager_destroy(lcm);
}

static void provisioning_with_http_bearer_expired_token_invalid_refresh_token() {
	bellesip::HttpServer authorizationServer;
	HttpServerWithBearerAuth httpServer("sip.example.org", "abcdefgh1");
	httpServer.addResource("/provisioning", "application/xml", "rcfiles/marie_xml");

	bellesip::AuthenticatedRegistrar registrar({"sip:marie@sip.example.com;access_token=abcdefgh1"}, "sip.example.org",
	                                           "tcp");
	registrar.addAuthMode(BELLE_SIP_AUTH_MODE_HTTP_BEARER);

	authorizationServer.Post("/oauth/token", [](BCTBX_UNUSED(const httplib::Request &req), httplib::Response &res) {
		res.status = 400;
		constexpr const char *body = R"(
{
  "error":"Invalid refresh token",
}
						)";
		res.set_content(body, "application/json");
	});

	LinphoneCoreManager *lcm = linphone_core_manager_new("empty_rc");

	std::string provisioningUri = httpServer.getUrl() + "/provisioning";

	/* set access token and provisioning uri */
	LinphoneBearerToken *token =
	    linphone_factory_create_bearer_token(linphone_factory_get(), "expiredtoken", time(NULL) - 30);
	LinphoneBearerToken *refresh_token =
	    linphone_factory_create_bearer_token(linphone_factory_get(), "xyz", time(NULL) + 3600);
	LinphoneAuthInfo *ai =
	    linphone_factory_create_auth_info_3(linphone_factory_get(), "marie", token, "sip.example.org");
	linphone_auth_info_set_access_token(ai, token);
	linphone_auth_info_set_refresh_token(ai, refresh_token);
	linphone_auth_info_set_authorization_server(ai, "auth.example.org");
	linphone_auth_info_set_token_endpoint_uri(ai, (authorizationServer.mRootUrl + "/oauth/token").c_str());
	linphone_bearer_token_unref(token);
	linphone_bearer_token_unref(refresh_token);
	linphone_core_add_auth_info(lcm->lc, ai);
	linphone_auth_info_unref(ai);

	/* set provisioning uri and attempt to load it */
	linphone_core_set_provisioning_uri(lcm->lc, provisioningUri.c_str());

	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_authentication_requested(cbs, bearer_auth_info_requested_2);
	linphone_core_add_callbacks(lcm->lc, cbs);

	/* now restart the provisioning */
	linphone_core_stop(lcm->lc);
	linphone_core_start(lcm->lc);

	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_authentication_info_requested, 1));
	/* resubmit new access token */
	token = linphone_factory_create_bearer_token(linphone_factory_get(), "abcdefgh1", time(NULL) + 30);
	refresh_token = linphone_factory_create_bearer_token(linphone_factory_get(), "xyz2", time(NULL) + 3600);
	ai = linphone_factory_create_auth_info_3(linphone_factory_get(), "marie", token, "sip.example.org");
	linphone_auth_info_set_access_token(ai, token);
	linphone_auth_info_set_refresh_token(ai, refresh_token);
	linphone_auth_info_set_token_endpoint_uri(ai, (authorizationServer.mRootUrl + "/oauth/token").c_str());
	linphone_bearer_token_unref(token);
	linphone_bearer_token_unref(refresh_token);
	linphone_core_add_auth_info(lcm->lc, ai);
	linphone_auth_info_unref(ai);

	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_LinphoneConfiguringSuccessful, 1));
	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_LinphoneGlobalOn, 2));

	BC_ASSERT_EQUAL(lcm->stat.number_of_authentication_info_requested, 1, int, "%i");
	linphone_core_manager_setup_dns(lcm);

	/* modify the registrar uri to setup correct port number */
	LinphoneAccount *account = linphone_core_get_default_account(lcm->lc);
	if (BC_ASSERT_PTR_NOT_NULL(account)) {
		LinphoneAccountParams *params = linphone_account_params_clone(linphone_account_get_params(account));
		LinphoneAddress *addr = linphone_address_clone(linphone_account_params_get_server_address(params));
		linphone_address_set_port(addr, belle_sip_uri_get_port(registrar.getAgent().getListeningUri()));
		linphone_account_params_set_server_address(params, addr);
		linphone_account_params_enable_register(params, TRUE);
		linphone_address_unref(addr);
		linphone_account_set_params(account, params);
		linphone_account_params_unref(params);
	}
	BC_ASSERT_TRUE(wait_for(lcm->lc, nullptr, &lcm->stat.number_of_LinphoneRegistrationOk, 1));
	BC_ASSERT_EQUAL(lcm->stat.number_of_authentication_info_requested, 1, int, "%i");

	linphone_core_cbs_unref(cbs);
	linphone_core_manager_destroy(lcm);
}

static void test_bearer_auth_refresh_token(void) {
	bellesip::HttpServer authorizationServer;
	bellesip::AuthenticatedRegistrar registrar({"sip:bob@sip.example.com;access_token=abcdefgh1"}, "sip.example.com",
	                                           "tcp");
	registrar.addAuthMode(BELLE_SIP_AUTH_MODE_HTTP_BEARER);

	authorizationServer.Post("/oauth/token", [](BCTBX_UNUSED(const httplib::Request &req), httplib::Response &res) {
		res.status = 200;
		constexpr const char *body = R"(
{
  "access_token":"abcdefgh1",
  "token_type":"Bearer",
  "expires_in":3600,
  "refresh_token":"xyz2",
  "scope":"create"
}
						)";
		res.set_content(body, "application/json");
	});

	LinphoneCoreManager *lcm = linphone_core_manager_new("empty_rc");
	LinphoneAccountParams *account_params = linphone_core_create_account_params(lcm->lc);
	LinphoneAddress *identity = linphone_factory_create_address(linphone_factory_get(), "sip:bob@sip.example.com");
	LinphoneAddress *server_addr =
	    linphone_factory_create_address(linphone_factory_get(), registrar.getAgent().getListeningUriAsString().c_str());
	linphone_account_params_set_identity_address(account_params, identity);
	linphone_account_params_set_server_address(account_params, server_addr);
	linphone_account_params_enable_register(account_params, TRUE);
	linphone_address_unref(server_addr);

	LinphoneAccount *account = linphone_core_create_account(lcm->lc, account_params);
	linphone_core_add_account(lcm->lc, account);
	linphone_account_params_unref(account_params);
	linphone_account_unref(account);

	/* create expired access token */
	LinphoneBearerToken *access_token =
	    linphone_factory_create_bearer_token(linphone_factory_get(), "abcdefgh0", time(NULL) - 20);
	LinphoneBearerToken *refresh_token =
	    linphone_factory_create_bearer_token(linphone_factory_get(), "xyz", time(NULL) + 3600);

	LinphoneAuthInfo *ai = linphone_factory_create_auth_info_3(
	    linphone_factory_get(), linphone_address_get_username(identity), access_token, "sip.example.com");
	linphone_auth_info_set_refresh_token(ai, refresh_token);
	linphone_auth_info_set_token_endpoint_uri(ai, (authorizationServer.mRootUrl + "/oauth/token").c_str());
	linphone_bearer_token_unref(access_token);
	linphone_bearer_token_unref(refresh_token);
	linphone_core_add_auth_info(lcm->lc, ai);
	linphone_auth_info_unref(ai);
	bctbx_message("Added AuthInfo.");
	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_LinphoneRegistrationOk, 1));
	BC_ASSERT_EQUAL(lcm->stat.number_of_authentication_info_requested, 0, int, "%i");
	const bctbx_list_t *ai_list = linphone_core_get_auth_info_list(lcm->lc);
	BC_ASSERT_EQUAL((int)bctbx_list_size(ai_list), 1, int, "%i");
	if (ai_list) {
		/* Make sure that the refresh token was updated*/
		const LinphoneAuthInfo *updated_ai = (const LinphoneAuthInfo *)ai_list->data;
		const LinphoneBearerToken *new_refresh_token = linphone_auth_info_get_refresh_token(updated_ai);
		BC_ASSERT_STRING_EQUAL(linphone_bearer_token_get_token(new_refresh_token), "xyz2");
	}

	linphone_address_unref(identity);
	linphone_core_manager_destroy(lcm);
}

static void test_bearer_auth_refresh_token_non_working() {
	bellesip::HttpServer authorizationServer;
	bellesip::AuthenticatedRegistrar registrar({"sip:bob@sip.example.com;access_token=abcdefgh1"}, "sip.example.com",
	                                           "tcp");
	registrar.addAuthMode(BELLE_SIP_AUTH_MODE_HTTP_BEARER);

	authorizationServer.Post("/oauth/token", [](BCTBX_UNUSED(const httplib::Request &req), httplib::Response &res) {
		res.status = 400;
		constexpr const char *body = R"(
{
  "error": "invalid bearer token"
}
						)";
		res.set_content(body, "application/json");
	});

	LinphoneCoreManager *lcm = linphone_core_manager_new("empty_rc");
	LinphoneAccountParams *account_params = linphone_core_create_account_params(lcm->lc);
	LinphoneAddress *identity = linphone_factory_create_address(linphone_factory_get(), "sip:bob@sip.example.com");
	LinphoneAddress *server_addr =
	    linphone_factory_create_address(linphone_factory_get(), registrar.getAgent().getListeningUriAsString().c_str());
	linphone_account_params_set_identity_address(account_params, identity);
	linphone_account_params_set_server_address(account_params, server_addr);
	linphone_account_params_enable_register(account_params, TRUE);
	linphone_address_unref(server_addr);

	LinphoneAccount *account = linphone_core_create_account(lcm->lc, account_params);
	linphone_core_add_account(lcm->lc, account);
	linphone_account_params_unref(account_params);
	linphone_account_unref(account);

	/* create expired access token */
	LinphoneBearerToken *access_token =
	    linphone_factory_create_bearer_token(linphone_factory_get(), "abcdefgh0", time(NULL) - 20);
	LinphoneBearerToken *refresh_token =
	    linphone_factory_create_bearer_token(linphone_factory_get(), "xyz", time(NULL) + 3600);

	LinphoneAuthInfo *ai = linphone_factory_create_auth_info_3(
	    linphone_factory_get(), linphone_address_get_username(identity), access_token, "sip.example.com");
	linphone_auth_info_set_refresh_token(ai, refresh_token);
	linphone_auth_info_set_token_endpoint_uri(ai, (authorizationServer.mRootUrl + "/oauth/token").c_str());
	linphone_bearer_token_unref(access_token);
	linphone_bearer_token_unref(refresh_token);
	linphone_core_add_auth_info(lcm->lc, ai);
	linphone_auth_info_unref(ai);
	bctbx_message("Added AuthInfo.");

	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_authentication_info_requested, 1));
	access_token = linphone_factory_create_bearer_token(linphone_factory_get(), "abcdefgh1", time(NULL) + 30);
	refresh_token = linphone_factory_create_bearer_token(linphone_factory_get(), "xyz2", time(NULL) + 3600);

	ai = linphone_factory_create_auth_info_3(linphone_factory_get(), linphone_address_get_username(identity),
	                                         access_token, "sip.example.com");
	linphone_auth_info_set_refresh_token(ai, refresh_token);
	linphone_auth_info_set_token_endpoint_uri(ai, (authorizationServer.mRootUrl + "/oauth/token").c_str());
	linphone_auth_info_set_authorization_server(ai, "auth.example.org");
	linphone_bearer_token_unref(access_token);
	linphone_bearer_token_unref(refresh_token);
	linphone_core_add_auth_info(lcm->lc, ai);
	linphone_auth_info_unref(ai);

	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_LinphoneRegistrationOk, 1));
	BC_ASSERT_EQUAL(lcm->stat.number_of_authentication_info_requested, 1, int, "%i");
	const bctbx_list_t *ai_list = linphone_core_get_auth_info_list(lcm->lc);
	BC_ASSERT_EQUAL((int)bctbx_list_size(ai_list), 1, int, "%i");
	if (ai_list) {
		/* Make sure that the refresh token was updated*/
		const LinphoneAuthInfo *updated_ai = (const LinphoneAuthInfo *)ai_list->data;
		const LinphoneBearerToken *new_refresh_token = linphone_auth_info_get_refresh_token(updated_ai);
		BC_ASSERT_STRING_EQUAL(linphone_bearer_token_get_token(new_refresh_token), "xyz2");
	}

	linphone_address_unref(identity);
	linphone_core_manager_destroy(lcm);
}

static void provisioning_with_http_bearer_then_register_with_digest_base(bool with_username_in_auth_info) {
	HttpServerWithBearerAuth httpServer("sip.example.org", "abcdefgh");
	httpServer.addResource("/provisioning", "application/xml", "rcfiles/marie_digest_auth_xml");

	bellesip::AuthenticatedRegistrar registrar({"sip:marie:toto@sip.example.com"}, "sip.example.org", "tcp");
	registrar.addAuthMode(BELLE_SIP_AUTH_MODE_HTTP_DIGEST);

	LinphoneCoreManager *lcm = linphone_core_manager_new("empty_rc");

	std::string provisioningUri = httpServer.getUrl() + "/provisioning";

	/* set provisioning uri and attempt to load it */
	linphone_core_set_provisioning_uri(lcm->lc, provisioningUri.c_str());

	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_authentication_requested(cbs, bearer_auth_info_requested_2);
	linphone_core_add_callbacks(lcm->lc, cbs);

	linphone_core_stop(lcm->lc);
	linphone_core_start(lcm->lc);
	linphone_core_manager_setup_dns(lcm);
	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_authentication_info_requested, 1));
	BC_ASSERT_EQUAL(lcm->stat.number_of_LinphoneConfiguringFailed, 0, int, "%i");
	BC_ASSERT_EQUAL(lcm->stat.number_of_LinphoneConfiguringSuccessful, 0, int, "%i");

	/* set access token */
	LinphoneBearerToken *token =
	    linphone_factory_create_bearer_token(linphone_factory_get(), "abcdefgh", time(NULL) + 30);
	LinphoneAuthInfo *ai = linphone_factory_create_auth_info_3(
	    linphone_factory_get(), with_username_in_auth_info ? "marie" : NULL, token, "sip.example.org");
	linphone_auth_info_set_access_token(ai, token);
	linphone_bearer_token_unref(token);
	linphone_core_add_auth_info(lcm->lc, ai);
	linphone_auth_info_unref(ai);

	/* the provisioning shall resume */
	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_LinphoneConfiguringSuccessful, 1));
	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_LinphoneGlobalOn, 2));

	/* modify the registrar uri to setup correct port number */
	LinphoneAccount *account = linphone_core_get_default_account(lcm->lc);
	if (BC_ASSERT_PTR_NOT_NULL(account)) {
		LinphoneAccountParams *params = linphone_account_params_clone(linphone_account_get_params(account));
		LinphoneAddress *addr = linphone_address_clone(linphone_account_params_get_server_address(params));
		linphone_address_set_port(addr, belle_sip_uri_get_port(registrar.getAgent().getListeningUri()));
		linphone_account_params_set_server_address(params, addr);
		linphone_account_params_enable_register(params, TRUE);
		linphone_address_unref(addr);
		linphone_account_set_params(account, params);
		linphone_account_params_unref(params);
	}
	BC_ASSERT_TRUE(wait_for(lcm->lc, nullptr, &lcm->stat.number_of_LinphoneRegistrationOk, 1));
	BC_ASSERT_EQUAL(lcm->stat.number_of_authentication_info_requested, 1, int, "%i");
	const bctbx_list_t *ai_list = linphone_core_get_auth_info_list(lcm->lc);
	BC_ASSERT_EQUAL((int)bctbx_list_size(ai_list), 3, int, "%i");

	// ms_message("dump of config:\n %s", linphone_config_dump(linphone_core_get_config(lcm->lc)));
	/* check that restarting works */
	linphone_core_stop(lcm->lc);
	linphone_core_start(lcm->lc);

	linphone_core_manager_setup_dns(lcm);
	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_LinphoneGlobalOn, 3));
	BC_ASSERT_EQUAL(lcm->stat.number_of_LinphoneConfiguringFailed, 0, int, "%i");
	BC_ASSERT_EQUAL(lcm->stat.number_of_LinphoneConfiguringSuccessful, 2, int, "%i");
	BC_ASSERT_TRUE(wait_for(lcm->lc, nullptr, &lcm->stat.number_of_LinphoneRegistrationOk, 2));
	/* there should still be 3 auth infos */
	BC_ASSERT_EQUAL((int)bctbx_list_size(linphone_core_get_auth_info_list(lcm->lc)), 3, int, "%i");
	char *dump = linphone_config_dump(linphone_core_get_config(lcm->lc));
	ms_message("dump of config after start \n: %s", dump);
	bctbx_free(dump);

	linphone_core_cbs_unref(cbs);
	linphone_core_manager_destroy(lcm);
}

static void provisioning_with_http_bearer_then_register_with_digest(void) {
	provisioning_with_http_bearer_then_register_with_digest_base(false);
}

static void provisioning_with_http_bearer_then_register_with_digest_with_username_in_auth_info(void) {
	provisioning_with_http_bearer_then_register_with_digest_base(true);
}
static test_t bearer_auth_tests[] = {
    {"Bearer auth requested", test_bearer_auth},
    {"Bearer auth set before", test_bearer_auth_set_before},
    {"Provisioning with http bearer auth", provisioning_with_http_bearer_auth},
    {"Provisioning with http bearer auth requested on demand", provisioning_with_http_bearer_auth_requested_on_demand},
    {"Provisioning with http bearer auth but expired access token", provisioning_with_http_bearer_expired_token},
    {"Provisioning with http bearer auth but expired access token and invalid refresh token",
     provisioning_with_http_bearer_expired_token_invalid_refresh_token},
    {"Provisioning with http bearer auth requested aborted",
     provisioning_with_http_bearer_auth_requested_on_demand_aborted},
    {"Bearer auth with refresh", test_bearer_auth_refresh_token},
    {"Bearer auth with refresh non working", test_bearer_auth_refresh_token_non_working},
    {"Provisioning with http bearer providing SIP account using digest auth",
     provisioning_with_http_bearer_then_register_with_digest},
    {"Provisioning with http bearer providing SIP account using digest auth 2",
     provisioning_with_http_bearer_then_register_with_digest_with_username_in_auth_info}};

/*
 * TODO: future tests
 * - core restart with tokens (to make sure that they are saved in linphonerc)
 * - auth token not sent over UDP or TCP (unless with special property)
 * - auth token bound to specific hosts (with wildcard ?), not sent to any other hosts
 */

test_suite_t bearer_auth_test_suite = {"Bearer authentication",
                                       NULL,
                                       NULL,
                                       liblinphone_tester_before_each,
                                       liblinphone_tester_after_each,
                                       sizeof(bearer_auth_tests) / sizeof(bearer_auth_tests[0]),
                                       bearer_auth_tests,
                                       10,
                                       0};
