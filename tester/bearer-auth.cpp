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

#include "belle_sip_tester_utils.h"

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

class HttpServerWithBearerAuth : public bellesip::HttpServer {
public:
	HttpServerWithBearerAuth(const std::string &realm) {
		mToken = "abcdefgh";
		mBaseUrl = "http://localhost:" + mListeningPort;
		mRealm = realm;
		BCTBX_SLOGI << " Waiting for http request on " << mBaseUrl;
	}
	void addResource(const std::string &urlPath, const std::string &contentType, const std::string &resourceName) {
		Get(urlPath, [this, contentType, resourceName](const httplib::Request &req, httplib::Response &res) {
			if (req.has_header("Authorization")) {
				// ok
				belle_sip_header_authorization_t *authorisation = belle_sip_header_authorization_parse(
				    ("Authorization: " + req.get_header_value("Authorization")).c_str());
				BC_ASSERT_STRING_EQUAL(belle_sip_header_authorization_get_scheme(authorisation), "Bearer");
				belle_sip_param_pair_t *paramPair = (belle_sip_param_pair_t *)bctbx_list_get_data(
				    belle_sip_parameters_get_parameters(BELLE_SIP_PARAMETERS(authorisation)));
				std::string token = paramPair ? paramPair->name : "";
				belle_sip_object_unref(authorisation);
				if (token == mToken) {
					res.set_content(loadFile(resourceName), contentType);
				} else {
					res.status = 403;
				}
			} else {
				res.status = 401;
				std::ostringstream headerValue;
				headerValue << "Bearer realm=\"" << mRealm << "\""
				            << ", authz_server=\"auth.example.org\"";
				res.set_header("WWW-Authenticate", headerValue.str());
			}
		});
	}
	const std::string &getUrl() const {
		return mBaseUrl;
	}

private:
	std::string loadFile(const std::string &resource) {
		char *file = bc_tester_res(resource.c_str());
		if (file) {
			std::ifstream ifst(file);
			if (!ifst.is_open()) bctbx_error("Cannot open [%s]", file);
			bc_free(file);
			std::ostringstream ostr;
			ostr << ifst.rdbuf();
			bctbx_message("File content is %s", ostr.str().c_str());
			return ostr.str();
		} else {
			bctbx_error("No resource file [%s]", resource.c_str());
		}
		return "";
	}
	std::string mToken;
	std::string mBaseUrl;
	std::string mRealm;
};

static void provisioning_with_http_bearer_auth(void) {
	HttpServerWithBearerAuth httpServer("sip.example.org");
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
	HttpServerWithBearerAuth httpServer("sip.example.org");
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
	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_LinphoneConfiguringFailed, 1));
	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_LinphoneGlobalOn, 2));
	BC_ASSERT_EQUAL(lcm->stat.number_of_authentication_info_requested, 1, int, "%i");
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

	/* now restart the provisioning */
	linphone_core_stop(lcm->lc);
	linphone_core_start(lcm->lc);
	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_LinphoneConfiguringSuccessful, 1));
	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_LinphoneGlobalOn, 3));
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

static test_t bearer_auth_tests[] = {
    {"Bearer auth requested", test_bearer_auth},
    {"Bearer auth set before", test_bearer_auth_set_before},
    {"Provisioning with http bearer auth", provisioning_with_http_bearer_auth},
    {"Provisioning with http bearer auth requested on demand", provisioning_with_http_bearer_auth_requested_on_demand},
    {"Bearer auth with refresh", test_bearer_auth_refresh_token}};

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
