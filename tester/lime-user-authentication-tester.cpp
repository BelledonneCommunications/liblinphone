/*
 * Copyright (c) 2010-2025 Belledonne Communications SARL.
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

#include <array>

#include "bctoolbox/crypto.h"

#include "liblinphone_tester.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-auth-info.h"
#include "linphone/core.h"
#include "tester_utils.h"

// enum the different methods for the client to retrieve the certificate
enum class certProvider {
	config_sip = 0,              /**< in the sip section (client_cert_chain and client_cert_key) of the config file */
	config_auth_info_buffer = 1, /**< in a dedicated auth_info section of the configuration file, set cert and key in a
	                                buffer -> they won't be written in the core config file */
	config_auth_info_path = 2, /**< in a dedicated auth_info section of the configuration file, set path to cert and key
	                              -> these will be written in the core config file */
	callback =
	    3 /**< using a callback adding auth_info into the core :
	NOT IMPLEMENTED, Client certificate for lime user identification shall already be accessible to the core as
   user register to the flexisip server before. THIS IS NOT DONE THIS WAY IN THIS TEST SUITE : user register on
   flexisip user http digest and tls cert on lime server for test purpose, it is very unlikely to proceed this way*/
};

// Helper to loop on all certificate providing methods availables
static std::array<certProvider, 3> availCertProv{
    {certProvider::config_sip, certProvider::config_auth_info_buffer, certProvider::config_auth_info_path}};

// This function will add proxy and auth info to the core config. The proxy is set as the default one
static void add_user_to_core_config(LinphoneCore *lc,
                                    const char *identity,
                                    const char *username,
                                    const char *realm,
                                    const char *server,
                                    const char *password) {
	// Use the user user_1
	LinphoneProxyConfig *cfg;
	cfg = linphone_core_create_proxy_config(lc);
	linphone_proxy_config_set_server_addr(cfg, server);
	linphone_proxy_config_set_route(cfg, server);
	linphone_proxy_config_set_realm(cfg, realm);
	linphone_proxy_config_enable_register(cfg, TRUE);

	LinphoneAddress *addr = linphone_address_new(identity);
	linphone_proxy_config_set_identity_address(cfg, addr);
	if (addr) linphone_address_unref(addr);

	linphone_core_add_proxy_config(lc, cfg);
	linphone_proxy_config_unref(cfg); // linphone_core_add_proxy_config set a ref on it
	linphone_core_set_default_proxy_config(lc, cfg);
	// set its credential
	LinphoneAuthInfo *auth_info = linphone_auth_info_new(username, username, password, NULL, realm, realm);
	linphone_core_add_auth_info(lc, auth_info);
	linphone_auth_info_unref(auth_info);
}

// Add tls information for given user into the linphone core
static void add_tls_client_certificate(LinphoneCore *lc,
                                       const std::string &username,
                                       const std::string &realm,
                                       const std::string &cert,
                                       const std::string &key,
                                       const certProvider method) {
	// set a TLS client certificate
	switch (method) {
		// when using config_sip, no user name is set, we can set only one certificate anyway...
		case certProvider::config_sip:
			if (!cert.empty()) {
				char *cert_path = bc_tester_res(cert.data());
				linphone_config_set_string(linphone_core_get_config(lc), "sip", "client_cert_chain", cert_path);
				bc_free(cert_path);
			}
			if (!key.empty()) {
				char *key_path = bc_tester_res(key.data());
				linphone_config_set_string(linphone_core_get_config(lc), "sip", "client_cert_key", key_path);
				bc_free(key_path);
			}
			break;
		case certProvider::config_auth_info_path: {
			// We shall already have an auth info for this username/realm, add the tls cert in it
			LinphoneAuthInfo *auth_info =
			    linphone_auth_info_clone(linphone_core_find_auth_info(lc, realm.data(), username.data(), realm.data()));
			// otherwise create it
			if (auth_info == NULL) {
				auth_info = linphone_auth_info_new(username.data(), NULL, NULL, NULL, realm.data(), realm.data());
			}
			if (!cert.empty()) {
				char *cert_path = bc_tester_res(cert.data());
				linphone_auth_info_set_tls_cert_path(auth_info, cert_path);
				bc_free(cert_path);
			}
			if (!key.empty()) {
				char *key_path = bc_tester_res(key.data());
				linphone_auth_info_set_tls_key_path(auth_info, key_path);
				bc_free(key_path);
			}
			linphone_core_add_auth_info(lc, auth_info);
			linphone_auth_info_unref(auth_info);
		} break;
		case certProvider::config_auth_info_buffer: {
			// We shall already have an auth info for this username/realm, add the tls cert in it
			LinphoneAuthInfo *auth_info =
			    linphone_auth_info_clone(linphone_core_find_auth_info(lc, realm.data(), username.data(), realm.data()));
			// otherwise create it
			if (auth_info == NULL) {
				auth_info = linphone_auth_info_new(username.data(), NULL, NULL, NULL, realm.data(), realm.data());
			}
			if (!cert.empty()) {
				char *cert_path = bc_tester_res(cert.data());
				char *cert_buffer = NULL;
				liblinphone_tester_load_text_file_in_buffer(cert_path, &cert_buffer);
				linphone_auth_info_set_tls_cert(auth_info, cert_buffer);
				bc_free(cert_path);
				bctbx_free(cert_buffer);
			}
			if (!key.empty()) {
				char *key_path = bc_tester_res(key.data());
				char *key_buffer = NULL;
				liblinphone_tester_load_text_file_in_buffer(key_path, &key_buffer);
				linphone_auth_info_set_tls_key(auth_info, key_buffer);
				bc_free(key_path);
				bctbx_free(key_buffer);
			}
			linphone_core_add_auth_info(lc, auth_info);
			linphone_auth_info_unref(auth_info);
		} break;
		case certProvider::callback:
			break;
	}
}

static void TLS_mandatory_two_users_curve(const LinphoneTesterLimeAlgo curveId) {
	LinphoneCoreManager *lcm = linphone_core_manager_create(NULL);
	add_user_to_core_config(lcm->lc, "sip:user_1@sip.example.org", "user_1", "sip.example.org",
	                        "sip:sip.example.org; transport=tls", "secret");
	add_user_to_core_config(lcm->lc, "sip:user_2@sip.example.org", "user_2", "sip.example.org",
	                        "sip:sip.example.org; transport=tls", "secret");
	add_tls_client_certificate(
	    lcm->lc, "user_1", "sip.example.org", "certificates/client/user1_multiple_aliases_cert.pem",
	    "certificates/client/user1_multiple_aliases_key.pem", certProvider::config_auth_info_path);
	add_tls_client_certificate(lcm->lc, "user_2", "sip.example.org", "certificates/client/user2_cert.pem",
	                           "certificates/client/user2_key.pem", certProvider::config_auth_info_buffer);
	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, lcm);
	set_lime_server_and_curve_list_tls(curveId, coresManagerList, TRUE, TRUE);
	stats initialMarieStats = lcm->stat;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_LinphoneRegistrationOk, 2));
	BC_ASSERT_TRUE(wait_for_list(coresList, &lcm->stat.number_of_X3dhUserCreationSuccess,
	                             initialMarieStats.number_of_X3dhUserCreationSuccess + 2, x3dhServer_creationTimeout));

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(lcm);
}

static void TLS_mandatory_two_users(void) {
	TLS_mandatory_two_users_curve(C25519);
	TLS_mandatory_two_users_curve(C448);
}

// code readability, use name which means something instead of a list of true and false in the args
constexpr bool tls_mandatory = true;
constexpr bool tls_optional = false;

static void create_user_sip_client_cert_chain(const LinphoneTesterLimeAlgo curveId,
                                              const bool use_tls,
                                              const certProvider method,
                                              const std::string &cert,
                                              const std::string &key,
                                              const bool expectFailure,
                                              const std::string &username = "user_1") {
	LinphoneCoreManager *lcm = linphone_core_manager_create(NULL);
	linphone_core_manager_expect_lime_failure(lcm, expectFailure);
	const std::string realm{"sip.example.org"};
	const std::string identity = std::string("sip:").append(username).append("@").append(realm);

	// add user
	add_user_to_core_config(lcm->lc, identity.data(), username.data(), realm.data(),
	                        "sip:sip.example.org; transport=tls", "secret");

	// add client certificate
	add_tls_client_certificate(lcm->lc, username, realm, cert, key, method);

	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, lcm);
	set_lime_server_and_curve_list_tls(curveId, coresManagerList, TRUE, use_tls);

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	BC_ASSERT_TRUE(wait_for(lcm->lc, NULL, &lcm->stat.number_of_LinphoneRegistrationOk, 1));

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(lcm);
}

/**
 * Create a user, successfully identify with the correct certificate holding the sip:uri in altName:DNS (one entry only)
 */
static void identity_in_altName_one_DNS_entry(void) {
	const std::string cert_path{"certificates/client/user1_cert.pem"};
	const std::string key_path{"certificates/client/user1_key.pem"};
	for (const auto &certProv : availCertProv) {
		create_user_sip_client_cert_chain(C25519, tls_mandatory, certProv, cert_path, key_path, false);
		create_user_sip_client_cert_chain(C448, tls_mandatory, certProv, cert_path, key_path, false);
		if (liblinphone_tester_is_lime_PQ_available()) {
			create_user_sip_client_cert_chain(C25519K512, tls_mandatory, certProv, cert_path, key_path, false);
			create_user_sip_client_cert_chain(C25519MLK512, tls_mandatory, certProv, cert_path, key_path, false);
			create_user_sip_client_cert_chain(C448MLK1024, tls_mandatory, certProv, cert_path, key_path, false);
		}
	}
}

/**
 * Create a user, successfully identify with the correct certificate holding the sip:uri in subject CN
 */
static void identity_in_subject_CN(void) {
	const std::string cert_path{"certificates/client/user2_CN_cert.pem"};
	const std::string key_path{"certificates/client/user2_CN_key.pem"};
	for (const auto &certProv : availCertProv) {
		create_user_sip_client_cert_chain(C25519, tls_mandatory, certProv, cert_path, key_path, false, "user_2");
		create_user_sip_client_cert_chain(C448, tls_mandatory, certProv, cert_path, key_path, false, "user_2");
		if (liblinphone_tester_is_lime_PQ_available()) {
			create_user_sip_client_cert_chain(C25519K512, tls_mandatory, certProv, cert_path, key_path, false,
			                                  "user_2");
			create_user_sip_client_cert_chain(C25519MLK512, tls_mandatory, certProv, cert_path, key_path, false,
			                                  "user_2");
			create_user_sip_client_cert_chain(C448MLK1024, tls_mandatory, certProv, cert_path, key_path, false,
			                                  "user_2");
		}
	}
}

/**
 * Create a user, successfully identify with the correct certificate holding the sip:uri in altName:DNS1 (4 DNS entry in
 * the altName)
 */
static void identity_in_altName_multiple_DNS_entry(void) {
	const std::string cert_path{"certificates/client/user1_multiple_aliases_cert.pem"};
	const std::string key_path{"certificates/client/user1_multiple_aliases_key.pem"};
	for (const auto &certProv : availCertProv) {
		create_user_sip_client_cert_chain(C25519, tls_mandatory, certProv, cert_path, key_path, false);
		create_user_sip_client_cert_chain(C448, tls_mandatory, certProv, cert_path, key_path, false);
		if (liblinphone_tester_is_lime_PQ_available()) {
			create_user_sip_client_cert_chain(C25519K512, tls_mandatory, certProv, cert_path, key_path, false);
			create_user_sip_client_cert_chain(C25519MLK512, tls_mandatory, certProv, cert_path, key_path, false);
			create_user_sip_client_cert_chain(C448MLK1024, tls_mandatory, certProv, cert_path, key_path, false);
		}
	}
}

/**
 * Try to create a user with a revoked certificate
 */
static void revoked_certificate(void) {
	const std::string cert_path{"certificates/client/user2_revoked_cert.pem"};
	const std::string key_path{"certificates/client/user2_revoked_key.pem"};
	for (const auto &certProv : availCertProv) {
		create_user_sip_client_cert_chain(C25519, tls_mandatory, certProv, cert_path, key_path, true, "user_2");
		create_user_sip_client_cert_chain(C448, tls_mandatory, certProv, cert_path, key_path, true, "user_2");
		if (liblinphone_tester_is_lime_PQ_available()) {
			create_user_sip_client_cert_chain(C25519K512, tls_mandatory, certProv, cert_path, key_path, true, "user_2");
			create_user_sip_client_cert_chain(C25519MLK512, tls_mandatory, certProv, cert_path, key_path, true,
			                                  "user_2");
			create_user_sip_client_cert_chain(C448MLK1024, tls_mandatory, certProv, cert_path, key_path, true,
			                                  "user_2");
		}
	}
}

/**
 * Create a user but authentify with the wrong certificate (mismatch the lime user id )
 * user creation shall fail
 */
static void TLS_mandatory_CN_UserId_mismatch(void) {
	const std::string cert_path{"certificates/client/cert2.pem"};
	const std::string key_path{"certificates/client/key2.pem"};
	for (const auto &certProv : availCertProv) {
		create_user_sip_client_cert_chain(C25519, tls_mandatory, certProv, cert_path, key_path, true);
		create_user_sip_client_cert_chain(C448, tls_mandatory, certProv, cert_path, key_path, true);
		if (liblinphone_tester_is_lime_PQ_available()) {
			create_user_sip_client_cert_chain(C25519K512, tls_mandatory, certProv, cert_path, key_path, true);
			create_user_sip_client_cert_chain(C25519MLK512, tls_mandatory, certProv, cert_path, key_path, true);
			create_user_sip_client_cert_chain(C448MLK1024, tls_mandatory, certProv, cert_path, key_path, true);
		}
	}
}

/**
 * Create a user but authentify with the wrong certificate (mismatch the lime user id)
 * user creation shall fail, even if user could have been identified with a http digest
 */
static void TLS_optional_CN_UserId_mismatch(void) {
	const std::string cert_path{"certificates/client/cert2.pem"};
	const std::string key_path{"certificates/client/key2.pem"};
	for (const auto &certProv : availCertProv) {
		create_user_sip_client_cert_chain(C25519, tls_optional, certProv, cert_path, key_path, true);
		create_user_sip_client_cert_chain(C448, tls_optional, certProv, cert_path, key_path, true);
		if (liblinphone_tester_is_lime_PQ_available()) {
			create_user_sip_client_cert_chain(C25519K512, tls_optional, certProv, cert_path, key_path, true);
			create_user_sip_client_cert_chain(C25519MLK512, tls_optional, certProv, cert_path, key_path, true);
			create_user_sip_client_cert_chain(C448MLK1024, tls_optional, certProv, cert_path, key_path, true);
		}
	}
}

/**
 * Create a user do not give any certificate, on TLS optional, it shall work
 */
static void TLS_optional_No_certificate(void) {
	const std::string empty{};
	/* just use the config_sip method, each method shall actually just do nothing when given an empty certificate */
	create_user_sip_client_cert_chain(C25519, tls_optional, certProvider::config_sip, empty, empty, false);
	create_user_sip_client_cert_chain(C448, tls_optional, certProvider::config_sip, empty, empty, false);
	if (liblinphone_tester_is_lime_PQ_available()) {
		create_user_sip_client_cert_chain(C25519K512, tls_optional, certProvider::config_sip, empty, empty, false);
		create_user_sip_client_cert_chain(C25519MLK512, tls_optional, certProvider::config_sip, empty, empty, false);
		create_user_sip_client_cert_chain(C448MLK1024, tls_optional, certProvider::config_sip, empty, empty, false);
	}
}

/**
 * Create a user do not give any certificate, on TLS mandatory, it shall fail
 */
static void TLS_mandatory_No_certificate(void) {
	const std::string empty{};
	/* just use the config_sip method, each method shall actually just do nothing when given an empty certificate */
	create_user_sip_client_cert_chain(C25519, tls_mandatory, certProvider::config_sip, empty, empty, true);
	create_user_sip_client_cert_chain(C448, tls_mandatory, certProvider::config_sip, empty, empty, true);
	if (liblinphone_tester_is_lime_PQ_available()) {
		create_user_sip_client_cert_chain(C25519K512, tls_mandatory, certProvider::config_sip, empty, empty, true);
		create_user_sip_client_cert_chain(C25519MLK512, tls_mandatory, certProvider::config_sip, empty, empty, true);
		create_user_sip_client_cert_chain(C448MLK1024, tls_mandatory, certProvider::config_sip, empty, empty, true);
	}
}

static void local_set_lime_server_and_curve(LinphoneCoreManager *manager, const char *curve, const char *server) {
	LinphoneAccount *account = linphone_core_get_default_account(manager->lc);
	const LinphoneAccountParams *account_params = linphone_account_get_params(account);
	LinphoneAccountParams *new_account_params = linphone_account_params_clone(account_params);
	linphone_account_params_set_lime_algo(new_account_params, curve);
	linphone_account_params_set_lime_server_url(new_account_params, server);
	linphone_account_set_params(account, new_account_params);
	linphone_account_params_unref(new_account_params);
}

/**
 * Create a user registered on lime on sip.example.org and auth1.example.org using the same lime server
 */
static void Digest_Auth_multidomains_curve(const LinphoneTesterLimeAlgo curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *libtester = linphone_core_manager_create(NULL);
	add_user_to_core_config(libtester->lc, "sip:liblinphone_tester@auth1.example.org", "liblinphone_tester",
	                        "auth1.example.org", "sip:auth1.example.org; transport=tls", "secret");
	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, libtester);

	// We use a specific server here, so do not use the generic set_lime_server_and_curve function
	local_set_lime_server_and_curve(marie, limeAlgoEnum2String(curveId), lime_server_any_domain_url);
	local_set_lime_server_and_curve(libtester, limeAlgoEnum2String(curveId), lime_server_any_domain_url);

	stats initialMarieStats = marie->stat;
	stats initialLibtesterStats = libtester->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	// Wait for lime users to be created on X3DH server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess,
	                             initialMarieStats.number_of_X3dhUserCreationSuccess + 1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &libtester->stat.number_of_X3dhUserCreationSuccess,
	                             initialLibtesterStats.number_of_X3dhUserCreationSuccess + 1,
	                             x3dhServer_creationTimeout));

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(libtester);
}

static void Digest_Auth_multidomains(void) {
	Digest_Auth_multidomains_curve(C25519);
	Digest_Auth_multidomains_curve(C448);
	if (liblinphone_tester_is_lime_PQ_available()) {
		Digest_Auth_multidomains_curve(C25519K512);
		Digest_Auth_multidomains_curve(C25519MLK512);
		Digest_Auth_multidomains_curve(C448MLK1024);
	}
}

static void Digest_Auth_multiservers_curve(const LinphoneTesterLimeAlgo curveId) {
	LinphoneCoreManager *manager = linphone_core_manager_create("multi_account_rc");
	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, manager);

	LinphoneAccount *marie_account = linphone_core_get_account_by_idkey(manager->lc, "eric");
	BC_ASSERT_PTR_NOT_NULL(marie_account);
	if (marie_account) {
		LinphoneAccountParams *params = linphone_account_params_clone(linphone_account_get_params(marie_account));
		linphone_account_params_set_lime_algo(params, limeAlgoEnum2String(curveId));
		linphone_account_params_set_lime_server_url(params, lime_server_url);
		linphone_account_set_params(marie_account, params);
		linphone_account_params_unref(params);
	}

	LinphoneAccount *pauline_account = linphone_core_get_account_by_idkey(manager->lc, "roger");
	BC_ASSERT_PTR_NOT_NULL(pauline_account);
	if (pauline_account) {
		LinphoneAccountParams *params = linphone_account_params_clone(linphone_account_get_params(pauline_account));
		linphone_account_params_set_lime_algo(params, limeAlgoEnum2String(curveId));
		linphone_account_params_set_lime_server_url(params, lime_server_any_domain_url);
		linphone_account_set_params(pauline_account, params);
		linphone_account_params_unref(params);
	}

	stats initialStats = manager->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	// Wait for lime users to be created on X3DH server, but only for Marie & Pauline
	BC_ASSERT_FALSE(wait_for_list(coresList, &manager->stat.number_of_X3dhUserCreationSuccess,
	                              initialStats.number_of_X3dhUserCreationSuccess + 3, x3dhServer_creationTimeout));
	BC_ASSERT_EQUAL(manager->stat.number_of_X3dhUserCreationSuccess, 2, int, "%d");

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(manager);
}

static void Digest_Auth_multiservers(void) {
	Digest_Auth_multiservers_curve(C25519);
	Digest_Auth_multiservers_curve(C448);
	if (liblinphone_tester_is_lime_PQ_available()) {
		Digest_Auth_multiservers_curve(C25519K512);
		Digest_Auth_multiservers_curve(C25519MLK512);
		Digest_Auth_multiservers_curve(C448MLK1024);
	}
}

static void invalid_lime_server_in_account_curve(const LinphoneTesterLimeAlgo curveId,
                                                 const std::string &wrongLimeServerUrl) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	linphone_core_manager_skip_lime_user_creation_asserts(marie, TRUE);
	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);

	LinphoneAccount *marie_account = linphone_core_get_default_account(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_account);
	if (marie_account) {
		LinphoneAccountParams *params = linphone_account_params_clone(linphone_account_get_params(marie_account));
		linphone_account_params_set_lime_algo(params, limeAlgoEnum2String(curveId));
		linphone_account_params_set_lime_server_url(params, wrongLimeServerUrl.c_str());
		linphone_account_set_params(marie_account, params);
		linphone_account_params_unref(params);
	}

	stats initialStats = marie->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	// Wait for lime user to be created on X3DH server: it shall fail
	BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess,
	                              initialStats.number_of_X3dhUserCreationSuccess + 1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationFailure,
	                             initialStats.number_of_X3dhUserCreationFailure + 2, x3dhServer_creationTimeout));
	BC_ASSERT_EQUAL(marie->stat.number_of_X3dhUserCreationFailure, 2, int, "%d");

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
}

static void invalid_lime_server_in_account() {
	// Server does not exists - unreachable
	const std::string wrongLimeServer = "https://lime.wildcard8.linphone.org:8443/lime-server/lime-server.php";
	// URL is not valid
	const std::string unparsableLimeServer = "ThisIsNotAnURL";

	invalid_lime_server_in_account_curve(C25519, wrongLimeServer);
	invalid_lime_server_in_account_curve(C448, wrongLimeServer);
	invalid_lime_server_in_account_curve(C25519, unparsableLimeServer);
	invalid_lime_server_in_account_curve(C448, unparsableLimeServer);
	if (liblinphone_tester_is_lime_PQ_available()) {
		invalid_lime_server_in_account_curve(C25519K512, wrongLimeServer);
		invalid_lime_server_in_account_curve(C25519MLK512, wrongLimeServer);
		invalid_lime_server_in_account_curve(C448MLK1024, wrongLimeServer);
		invalid_lime_server_in_account_curve(C25519K512, unparsableLimeServer);
		invalid_lime_server_in_account_curve(C25519MLK512, unparsableLimeServer);
		invalid_lime_server_in_account_curve(C448MLK1024, unparsableLimeServer);
	}
}

test_t lime_server_auth_tests[] = {
    TEST_TWO_TAGS("Invalid LIME server in account parameters", invalid_lime_server_in_account, "LimeX3DH", "CRYPTO"),
    TEST_TWO_TAGS("sip:uri in altname DNS", identity_in_altName_one_DNS_entry, "LimeX3DH", "CRYPTO"),
    TEST_TWO_TAGS("sip:uri in subject CN", identity_in_subject_CN, "LimeX3DH", "CRYPTO"),
    TEST_TWO_TAGS(
        "sip:uri in altname DNS with multiple entries", identity_in_altName_multiple_DNS_entry, "LimeX3DH", "CRYPTO"),
    TEST_TWO_TAGS("Try to use a revoked certificate", revoked_certificate, "LimeX3DH", "CRYPTO"),
    TEST_TWO_TAGS("TLS optional server log using digest auth", TLS_optional_No_certificate, "LimeX3DH", "CRYPTO"),
    TEST_TWO_TAGS("TLS mandatory, no certificate", TLS_mandatory_No_certificate, "LimeX3DH", "CRYPTO"),
    TEST_TWO_TAGS(
        "CN and From mismatch on TLS mandatory server", TLS_mandatory_CN_UserId_mismatch, "LimeX3DH", "CRYPTO"),
    TEST_TWO_TAGS("CN and From mismatch on TLS optional server", TLS_optional_CN_UserId_mismatch, "LimeX3DH", "CRYPTO"),
    TEST_TWO_TAGS("Connect two users with client certificate to TLS mandatory server",
                  TLS_mandatory_two_users,
                  "LimeX3DH",
                  "CRYPTO"),
    TEST_TWO_TAGS("Digest Auth - multiple domains", Digest_Auth_multidomains, "LimeX3DH", "CRYPTO"),
    TEST_TWO_TAGS("Digest Auth - multiple servers", Digest_Auth_multiservers, "LimeX3DH", "CRYPTO"),
};

test_suite_t lime_server_auth_test_suite = {"Lime server user authentication",
                                            NULL,
                                            NULL,
                                            liblinphone_tester_before_each,
                                            liblinphone_tester_after_each,
                                            sizeof(lime_server_auth_tests) / sizeof(lime_server_auth_tests[0]),
                                            lime_server_auth_tests,
                                            0};
