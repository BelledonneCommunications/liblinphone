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

#include "bctoolbox/defs.h"

#ifdef HAVE_FLEXIAPI
#include <json/json.h>
using namespace Json;
#endif

#include "c-wrapper/c-wrapper.h"
#include "liblinphone_tester.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-friend.h"
#include "linphone/core.h"
#ifdef HAVE_FLEXIAPI
#include "linphone/flexi-api-client.h"
#endif
#include "private_functions.h"
#include "tester_utils.h"

typedef struct _LinphoneFriendListStats {
	int new_list_count;
} LinphoneFriendListStats;

static void remote_provisioning_skipped(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("marie_rc", FALSE);
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneConfiguringSkipped, 1));
	linphone_core_manager_destroy(marie);
}

static void remote_provisioning_http(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_remote_rc");
	LinphoneFriendList *list = linphone_core_get_friend_list_by_name(marie->lc, "_default");
	BC_ASSERT_TRUE(linphone_friend_list_subscriptions_enabled(list));

	linphone_core_manager_start(marie, FALSE);
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneConfiguringSuccessful, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 1));

	/*make sure proxy config is not added in double, one time at core init, next time at configuring successful*/
	BC_ASSERT_EQUAL((int)bctbx_list_size(linphone_core_get_proxy_config_list(marie->lc)), 1, int, "%i");
	BC_ASSERT_FALSE(linphone_friend_list_subscriptions_enabled(list));

	linphone_core_manager_destroy(marie);
}

static void remote_provisioning_contact_list_fetch_at_startup(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_remote_rc");
	const char *vcard_file = "http://provisioning.example.org/vcards.vcf";
	linphone_config_set_string(linphone_core_get_config(marie->lc), "misc", "contacts-vcard-list", vcard_file);

	// do not get vcard contact list from remote provisioning at startup
	bool_t fetch_contact_list_at_startup = FALSE;
	linphone_config_set_bool(linphone_core_get_config(marie->lc), "misc", "fetch_contacts_vcard_list_at_startup",
	                         fetch_contact_list_at_startup);

	linphone_core_manager_start(marie, FALSE);
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneConfiguringSuccessful, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 1));

	std::string url =
	    linphone_config_get_string(linphone_core_get_config(marie->lc), "misc", "contacts-vcard-list", NULL);
	LinphoneFriendList *friendList = linphone_core_get_friend_list_by_name(marie->lc, url.c_str());
	BC_ASSERT_PTR_NOT_NULL(friendList);
	if (friendList) {
		unsigned int friends_list_size = (unsigned int)bctbx_list_size(linphone_friend_list_get_friends(friendList));
		BC_ASSERT_EQUAL(friends_list_size, 0, unsigned int, "%u");
	}

	// check that the vcards file is not missing nor empty
	linphone_friend_list_synchronize_friends_from_server(friendList);
	wait_for_until(marie->lc, NULL, NULL, 1, 5000);
	BC_ASSERT_PTR_NOT_NULL(friendList);
	if (friendList) {
		unsigned int friends_list_size = (unsigned int)bctbx_list_size(linphone_friend_list_get_friends(friendList));
		BC_ASSERT_EQUAL(friends_list_size, 3, unsigned int, "%u");
	}

	linphone_core_manager_destroy(marie);
}

static void remote_provisioning_transient(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("marie_transient_remote_rc", FALSE);
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneConfiguringSuccessful, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 1));
	BC_ASSERT_TRUE(linphone_core_is_provisioning_transient(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_provisioning_uri(marie->lc));
	linphone_core_manager_destroy(marie);
}

static void remote_provisioning_https(void) {
	if (transport_supported(LinphoneTransportTls)) {
		LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("marie_remote_https_rc", FALSE);
		BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneConfiguringSuccessful, 1));
		BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 1));
		linphone_core_manager_destroy(marie);
	}
}

static void aborted_provisioning_https(void) {
	if (transport_supported(LinphoneTransportTls)) {
		LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("marie_remote_https_rc", FALSE);
		/** immediately call linphohne_core_stop() without letting the remote provisioning to complete */
		linphone_core_stop_async(marie->lc);
		BC_ASSERT_EQUAL(linphone_core_get_global_state(marie->lc), LinphoneGlobalShutdown, int, "%i");
		BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneConfiguringSkipped, 1));
		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConfiguringFailed, 0, int, "%i");
		/* now should move to Off state */
		BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneGlobalOff, 1));
		linphone_core_manager_destroy(marie);
	}
}

static void remote_provisioning_not_found(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("marie_remote_404_rc", FALSE);
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneConfiguringFailed, 1));
	linphone_core_manager_destroy(marie);
}

static void remote_provisioning_invalid(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("marie_remote_invalid_rc", FALSE);
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneConfiguringFailed, 1));
	linphone_core_manager_destroy(marie);
}

static void remote_provisioning_invalid_uri(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("marie_remote_invalid_uri_rc", FALSE);
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneConfiguringFailed, 1));
	linphone_core_manager_destroy(marie);
}

static void remote_provisioning_default_values(void) {
	LinphoneProxyConfig *lpc;
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("marie_remote_default_values_rc", FALSE);
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneConfiguringSuccessful, 1));
	lpc = linphone_core_create_proxy_config(marie->lc);
	BC_ASSERT_TRUE(linphone_proxy_config_register_enabled(lpc));
	BC_ASSERT_EQUAL(linphone_proxy_config_get_expires(lpc), 604800, int, "%d");
	BC_ASSERT_STRING_EQUAL(linphone_proxy_config_get_server_addr(lpc), "<sip:sip.linphone.org:5223;transport=tls>");
	const char *route = linphone_proxy_config_get_route(lpc);
	BC_ASSERT_STRING_EQUAL(route, "<sip:sip.linphone.org:5223;transport=tls>");
	BC_ASSERT_STRING_EQUAL(linphone_proxy_config_get_identity(lpc), "sip:?@sip.linphone.org");
	{
		LpConfig *lp = linphone_core_get_config(marie->lc);
		BC_ASSERT_STRING_EQUAL(linphone_config_get_string(lp, "app", "toto", "empty"), "titi");
	}
	linphone_proxy_config_unref(lpc);
	linphone_core_manager_destroy(marie);
}

static void remote_provisioning_file(void) {
	LinphoneCoreManager *marie;
	const LpConfig *conf;
#if TARGET_OS_IPHONE
	ms_message("Skipping remote provisioning from file on iOS");
	return;
#elif defined(__ANDROID__)
	marie = linphone_core_manager_new_with_proxies_check("marie_remote_localfile_android_rc", FALSE);
#elif defined(LINPHONE_WINDOWS_UNIVERSAL)
	marie = linphone_core_manager_new_with_proxies_check("marie_remote_localfile_win10_rc", FALSE);
#else
	marie = ms_new0(LinphoneCoreManager, 1);
	linphone_core_manager_init(marie, "marie_remote_localfile_rc", NULL);

	// fix relative path to absolute path
	{
		char *path = bc_tester_res("rcfiles/marie_remote_localfile2_rc");
		char *abspath = ms_strdup_printf("file://%s", path);
		linphone_config_set_string(linphone_core_get_config(marie->lc), "misc", "config-uri", abspath);
		linphone_core_manager_start(marie, 1);
		ms_free(path);
		ms_free(abspath);
	}
#endif
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneConfiguringSuccessful, 1));

	conf = linphone_core_get_config(marie->lc);
	BC_ASSERT_EQUAL(linphone_config_get_int(conf, "misc", "tester_file_ok", 0), 1, int, "%d");
	linphone_core_manager_destroy(marie);
}

#ifdef HAVE_FLEXIAPI
static void flexiapi_remote_provisioning_flow(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("pauline_rc");
	linphone_config_set_string(linphone_core_get_config(marie->lc), "misc", "config-uri",
	                           "http://provisioning.example.org:10080/flexiapi/provisioning");

	auto flexiAPIClient = make_shared<LinphonePrivate::FlexiAPIClient>(marie->lc);
	flexiAPIClient->useTestAdminAccount(true);

	int code = 0;
	int fetched = 0;
	std::string remoteProvisioningURI = linphone_core_get_provisioning_uri(marie->lc);

	// Create a test account
	char *token = sal_get_random_token_lowercase(12);
	string username = string("test_").append(token);
	bctbx_free(token);
	bool activated = false; // Required to get a confirmation key
	std::string confirmationKey;
	int id;

	flexiAPIClient->adminAccountCreate(username, "1234", "MD5", activated)
	    ->then([&code, &fetched, &confirmationKey, &id](LinphonePrivate::FlexiAPIClient::Response response) {
		    code = response.code;
		    fetched = 1;
		    confirmationKey = response.json()["provisioning_token"].asString();
		    id = response.json()["id"].asInt();
	    });

	wait_for_until(marie->lc, NULL, &fetched, 1, liblinphone_tester_sip_timeout);
	BC_ASSERT_EQUAL(code, 200, int, "%d");

	// Provision it
	std::string remoteProvisioningURIWithConfirmationKey = remoteProvisioningURI;
	remoteProvisioningURIWithConfirmationKey.append("/").append(confirmationKey);

	linphone_core_manager_reinit(marie);
	linphone_core_set_provisioning_uri(marie->lc, remoteProvisioningURIWithConfirmationKey.c_str());
	linphone_core_manager_start(marie, FALSE);

	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneConfiguringSuccessful, 1,
	                              liblinphone_tester_sip_timeout));

	// Re-provision it, without the confirmationKey
	std::string remoteProvisioningURIAuthenticated = remoteProvisioningURI;
	remoteProvisioningURIAuthenticated.append("/me");

	linphone_core_manager_reinit(marie);
	linphone_core_set_provisioning_uri(marie->lc, remoteProvisioningURIAuthenticated.c_str());
	linphone_core_manager_start(marie, FALSE);

	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneConfiguringSuccessful, 1,
	                              liblinphone_tester_sip_timeout));

	flexiAPIClient = make_shared<LinphonePrivate::FlexiAPIClient>(marie->lc);

	// Clean up
	fetched = 0;
	flexiAPIClient->adminAccountDelete(id)->then([&code, &fetched](LinphonePrivate::FlexiAPIClient::Response response) {
		code = response.code;
		fetched = 1;
	});

	wait_for_until(marie->lc, NULL, &fetched, 1, liblinphone_tester_sip_timeout);
	BC_ASSERT_EQUAL(code, 200, int, "%d");

	linphone_core_manager_destroy(marie);
}

static void flexiapi_remote_provisioning_contacts_list_flow(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("pauline_rc");
	linphone_config_set_string(linphone_core_get_config(marie->lc), "misc", "config-uri",
	                           "http://provisioning.example.org:10080/flexiapi/provisioning");

	auto flexiAPIClient = make_shared<LinphonePrivate::FlexiAPIClient>(marie->lc);
	flexiAPIClient->useTestAdminAccount(true);

	int code = 0;
	int fetched = 0;
	std::string remoteProvisioningURI = linphone_core_get_provisioning_uri(marie->lc);

	// Create a test account
	char *token = sal_get_random_token_lowercase(12);
	string usernameContact1 = string("test_").append(token);
	bctbx_free(token);
	token = sal_get_random_token_lowercase(12);
	string usernameContact2 = string("test_").append(token);
	bctbx_free(token);
	int contactId0;
	int contactId1;
	int contactId2;

	char *addr = linphone_address_as_string_uri_only(
	    linphone_proxy_config_get_identity_address(linphone_core_get_default_proxy_config(marie->lc)));

	// Get the contact account id
	flexiAPIClient->adminAccountSearch(addr)->then(
	    [&code, &fetched, &contactId0](LinphonePrivate::FlexiAPIClient::Response response) {
		    code = response.code;
		    fetched = 1;
		    contactId0 = response.json()["id"].asInt();
	    });

	bctbx_free(addr);

	wait_for_until(marie->lc, NULL, &fetched, 1, liblinphone_tester_sip_timeout);

	// Create the contacts accounts
	fetched = code = 0;
	flexiAPIClient->adminAccountCreate(usernameContact1, "1234", "MD5", "", true, "", "", "sipinfo")
	    ->then([&code, &fetched, &contactId1](LinphonePrivate::FlexiAPIClient::Response response) {
		    code = response.code;
		    BC_ASSERT_EQUAL(code, 200, int, "%d");
		    fetched = 1;
		    contactId1 = response.json()["id"].asInt();
	    });

	wait_for_until(marie->lc, NULL, &fetched, 1, liblinphone_tester_sip_timeout);

	fetched = code = 0;
	flexiAPIClient->adminAccountCreate(usernameContact2, "1234", "MD5", "", true, "", "", "rfc2833")
	    ->then([&code, &fetched, &contactId2](LinphonePrivate::FlexiAPIClient::Response response) {
		    code = response.code;
		    BC_ASSERT_EQUAL(code, 200, int, "%d");
		    fetched = 1;
		    contactId2 = response.json()["id"].asInt();
	    });

	wait_for_until(marie->lc, NULL, &fetched, 1, liblinphone_tester_sip_timeout);
	BC_ASSERT_EQUAL(code, 200, int, "%d");

	auto responseCb = [&code, &fetched](const LinphonePrivate::FlexiAPIClient::Response &response) {
		code = response.code;
		BC_ASSERT_EQUAL(code, 200, int, "%d");
		fetched = 1;
	};

	// Link the contacts
	fetched = code = 0;
	flexiAPIClient->adminAccountContactAdd(contactId0, contactId1)->then(responseCb);
	wait_for_until(marie->lc, NULL, &fetched, 1, liblinphone_tester_sip_timeout);
	fetched = code = 0;
	flexiAPIClient->adminAccountContactAdd(contactId0, contactId2)->then(responseCb);
	wait_for_until(marie->lc, NULL, &fetched, 1, liblinphone_tester_sip_timeout);

	// Provision it
	std::string remoteProvisioningURIWithConfirmationKey = remoteProvisioningURI;
	remoteProvisioningURIWithConfirmationKey.append("/me");

	linphone_core_manager_reinit(marie);

	// Registering the callbacks
	LinphoneFriendListStats *stats = (LinphoneFriendListStats *)ms_new0(LinphoneFriendListStats, 1);
	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_friend_list_created(cbs, [](LinphoneCore *lc, BCTBX_UNUSED(LinphoneFriendList * list)) {
		LinphoneFriendListStats *stats =
		    (LinphoneFriendListStats *)linphone_core_cbs_get_user_data(linphone_core_get_current_callbacks(lc));
		stats->new_list_count++;
	});

	linphone_core_add_callbacks(marie->lc, cbs);
	linphone_core_cbs_set_user_data(cbs, stats);

	linphone_core_set_provisioning_uri(marie->lc, remoteProvisioningURIWithConfirmationKey.c_str());
	linphone_core_manager_start(marie, FALSE);

	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneConfiguringSuccessful, 1,
	                              liblinphone_tester_sip_timeout));

	BC_ASSERT_TRUE(
	    wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneGlobalOn, 1, liblinphone_tester_sip_timeout));

	std::string url =
	    linphone_config_get_string(linphone_core_get_config(marie->lc), "misc", "contacts-vcard-list", NULL);
	LinphoneFriendList *friendList = linphone_core_get_friend_list_by_name(marie->lc, url.c_str());
	BC_ASSERT_PTR_NOT_NULL(friendList);
	if (friendList) {
		wait_for_until(marie->lc, NULL, NULL, 1, 5000);

		unsigned int friends_list_size = (unsigned int)bctbx_list_size(linphone_friend_list_get_friends(friendList));
		BC_ASSERT_EQUAL(friends_list_size, 2, unsigned int, "%u");

		if (friends_list_size > 0) {
			// Check if the vCard has been properly parsed
			LinphoneFriend *lf = (LinphoneFriend *)bctbx_list_get_data(linphone_friend_list_get_friends(friendList));
			LinphoneVcard *vcard = linphone_friend_get_vcard(lf);

			string fullName = string(linphone_vcard_get_full_name(vcard)).substr(0, usernameContact2.length());
			BC_ASSERT_STRING_EQUAL(fullName.c_str(), usernameContact2.c_str());

			bctbx_list_t *extended_properties =
			    linphone_vcard_get_extended_properties_values_by_name(vcard, "X-LINPHONE-ACCOUNT-DTMF-PROTOCOL");
			BC_ASSERT_STRING_EQUAL((const char *)bctbx_list_get_data(extended_properties), "rfc2833");
			bctbx_list_free_with_data(extended_properties, (bctbx_list_free_func)bctbx_free);

			linphone_vcard_remove_extented_properties_by_name(vcard, "X-LINPHONE-ACCOUNT-DTMF-PROTOCOL");
			linphone_vcard_add_extended_property(vcard, "X-LINPHONE-ACCOUNT-DTMF-PROTOCOL", "test");

			extended_properties =
			    linphone_vcard_get_extended_properties_values_by_name(vcard, "X-LINPHONE-ACCOUNT-DTMF-PROTOCOL");
			BC_ASSERT_STRING_EQUAL((const char *)bctbx_list_get_data(extended_properties), "test");
			bctbx_list_free_with_data(extended_properties, (bctbx_list_free_func)bctbx_free);
		}
		BC_ASSERT_EQUAL((int)bctbx_list_size(linphone_core_get_friends_lists(marie->lc)), 2, int, "%i");
		linphone_core_remove_friend_list(marie->lc, friendList);
		// Only _default remains
		BC_ASSERT_EQUAL((int)bctbx_list_size(linphone_core_get_friends_lists(marie->lc)), 1, int, "%i");

		// Reparse it with one less friend
		fetched = 0;

		flexiAPIClient = make_shared<LinphonePrivate::FlexiAPIClient>(marie->lc);
		flexiAPIClient->useTestAdminAccount(true);

		flexiAPIClient->adminAccountContactDelete(contactId0, contactId2)
		    ->then([&code, &fetched](const LinphonePrivate::FlexiAPIClient::Response &response) {
			    code = response.code;
			    fetched = 1;
		    });

		wait_for_until(marie->lc, NULL, &fetched, 1, liblinphone_tester_sip_timeout);

		linphone_friend_list_synchronize_friends_from_server(friendList);
		wait_for_until(marie->lc, NULL, NULL, 1, 5000);

		BC_ASSERT_EQUAL((int)bctbx_list_size(linphone_friend_list_get_friends(friendList)), 1, int, "%i");

		BC_ASSERT_EQUAL(linphone_core_friends_storage_resync_friends_lists(marie->lc), 1, int, "%i");

		BC_ASSERT_EQUAL((int)bctbx_list_size(linphone_friend_list_get_friends(friendList)), 1, int, "%i");
	}

	// Clean up
	flexiAPIClient = make_shared<LinphonePrivate::FlexiAPIClient>(marie->lc);

	fetched = 0;
	flexiAPIClient->adminAccountDelete(contactId1)->then(responseCb);
	wait_for_until(marie->lc, NULL, &fetched, 1, liblinphone_tester_sip_timeout);
	fetched = 0;
	flexiAPIClient->adminAccountDelete(contactId2)->then(responseCb);
	wait_for_until(marie->lc, NULL, &fetched, 1, liblinphone_tester_sip_timeout);

	linphone_core_cbs_unref(cbs);
	ms_free(stats);
	linphone_core_manager_destroy(marie);
}

#endif // HAVE_FLEXIAPI

static void remote_provisioning_check_push_params(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_remote_rc");

	LinphonePushNotificationConfig *marie_push_cfg =
	    linphone_push_notification_config_clone(linphone_core_get_push_notification_config(marie->lc));
	linphone_push_notification_config_set_voip_token(marie_push_cfg, "token:voip");
	linphone_push_notification_config_set_bundle_identifier(marie_push_cfg, "linphone-tester");
	linphone_push_notification_config_set_param(marie_push_cfg, "param");
	linphone_core_set_push_notification_config(marie->lc, marie_push_cfg);
	linphone_push_notification_config_unref(marie_push_cfg);

	linphone_core_manager_start(marie, FALSE);
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneConfiguringSuccessful, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 1));

	/*make sure proxy config is not added in double, one time at core init, next time at configuring successful*/
	BC_ASSERT_EQUAL((int)bctbx_list_size(linphone_core_get_proxy_config_list(marie->lc)), 1, int, "%i");

	LinphoneAccount *marie_account = linphone_core_get_default_account(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_account);

	LinphoneAccountParams *marie_params = linphone_account_params_clone(linphone_account_get_params(marie_account));

	BC_ASSERT_STRING_EQUAL(linphone_push_notification_config_get_voip_token(
	                           linphone_account_params_get_push_notification_config(marie_params)),
	                       "token:voip");
	BC_ASSERT_STRING_EQUAL(linphone_push_notification_config_get_bundle_identifier(
	                           linphone_account_params_get_push_notification_config(marie_params)),
	                       "linphone-tester");
	BC_ASSERT_STRING_EQUAL(
	    linphone_push_notification_config_get_param(linphone_account_params_get_push_notification_config(marie_params)),
	    "param");

	linphone_account_params_set_push_notification_allowed(marie_params, TRUE);
	linphone_account_set_params(marie_account, marie_params);
	linphone_account_params_unref(marie_params);

	linphone_core_set_push_notification_enabled(marie->lc, TRUE);
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 2));

	const LinphoneAccountParams *updated_marie_params = linphone_account_get_params(marie_account);
	const char *prid = linphone_push_notification_config_get_prid(
	    linphone_account_params_get_push_notification_config(updated_marie_params));
	BC_ASSERT_STRING_EQUAL(prid, "token:voip"); // Ensure that push parameters have been generated for the new register

	linphone_core_manager_destroy(marie);
}

test_t remote_provisioning_tests[] = {
    TEST_NO_TAG("Remote provisioning skipped", remote_provisioning_skipped),
    TEST_NO_TAG("Remote provisioning successful behind http", remote_provisioning_http),
    TEST_NO_TAG("Remote provisioning successful behind https", remote_provisioning_https),
    TEST_NO_TAG("Remote provisioning aborted", aborted_provisioning_https),
    TEST_NO_TAG("Remote provisioning 404 not found", remote_provisioning_not_found),
    TEST_NO_TAG("Remote provisioning invalid", remote_provisioning_invalid),
    TEST_NO_TAG("Remote provisioning transient successful", remote_provisioning_transient),
    TEST_NO_TAG("Remote provisioning default values", remote_provisioning_default_values),
    TEST_NO_TAG("Remote provisioning from file", remote_provisioning_file),
    TEST_NO_TAG("Remote provisioning invalid URI", remote_provisioning_invalid_uri),
    TEST_NO_TAG("Remote provisioning check if push tokens are not lost", remote_provisioning_check_push_params),
    TEST_NO_TAG("Remote Provisioning Contacts List fetch at startup", remote_provisioning_contact_list_fetch_at_startup)
#ifdef HAVE_FLEXIAPI
        ,
    TEST_NO_TAG("Remote Provisioning Flow", flexiapi_remote_provisioning_flow),
    TEST_NO_TAG("Remote Provisioning Contacts List Flow", flexiapi_remote_provisioning_contacts_list_flow)
#endif
};

test_suite_t remote_provisioning_test_suite = {"RemoteProvisioning",
                                               NULL,
                                               NULL,
                                               liblinphone_tester_before_each,
                                               liblinphone_tester_after_each,
                                               sizeof(remote_provisioning_tests) / sizeof(remote_provisioning_tests[0]),
                                               remote_provisioning_tests,
                                               0};
