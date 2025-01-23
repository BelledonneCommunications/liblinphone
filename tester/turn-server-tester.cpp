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

#include "bctoolbox/defs.h"
#include "belle_sip_tester_utils.h"

#include "http-server-utils.h"
#include "liblinphone_tester++.h"
#include "liblinphone_tester.h"
#include "shared_tester_functions.h"

static void configure_nat_policy_with_turn(LinphoneCoreManager *core_manager, const std::string &rootUrl) {
	LinphoneNatPolicy *nat_policy = linphone_core_create_nat_policy(core_manager->lc);
	linphone_nat_policy_enable_turn(nat_policy, TRUE);
	linphone_nat_policy_enable_ice(nat_policy, TRUE);
	linphone_nat_policy_enable_stun(nat_policy, TRUE);
	linphone_nat_policy_set_stun_server(nat_policy, "sip1.linphone.org:3479");
	linphone_nat_policy_set_stun_server_username(nat_policy, "liblinphone-tester");
	linphone_nat_policy_set_turn_configuration_endpoint(nat_policy, (rootUrl + "/services/turn").c_str());

	auto default_account = linphone_core_get_default_account(core_manager->lc);
	const LinphoneAccountParams *account_params = linphone_account_get_params(default_account);
	LinphoneAccountParams *new_account_params = linphone_account_params_clone(account_params);
	linphone_account_params_set_nat_policy(new_account_params, nat_policy);
	linphone_account_set_params(default_account, new_account_params);

	linphone_account_params_unref(new_account_params);
	linphone_nat_policy_unref(nat_policy);
}

static void check_turn_username_password(LinphoneCoreManager *core_manager,
                                         const std::string &username,
                                         const std::string &password) {
	auto auth_infos = linphone_core_get_auth_info_list(core_manager->lc);
	BC_ASSERT_PTR_NOT_NULL(auth_infos);
	if (BC_ASSERT_PTR_NOT_NULL(auth_infos)) {
		bool usernameFound = false;
		bool passwordFound = false;
		for (auto auth_info = auth_infos; auth_info != nullptr; auth_info = auth_info->next) {
			if (strcmp((const char *)(linphone_auth_info_get_username((const LinphoneAuthInfo *)auth_info->data)),
			           username.c_str()) == 0) {
				usernameFound = true;
			}
			if (strcmp((const char *)(linphone_auth_info_get_password((const LinphoneAuthInfo *)auth_info->data)),
			           password.c_str()) == 0) {
				passwordFound = true;
			}
		}
		BC_ASSERT_TRUE(usernameFound);
		BC_ASSERT_TRUE(passwordFound);
	}
}

void update_turn_configuration_test() {
	bellesip::HttpServer httpServer;
	std::string username = "liblinphone-tester";
	std::string password = "retset-enohpnilbil";

	httpServer.Get("/services/turn", [](BCTBX_UNUSED(const httplib::Request &req), httplib::Response &res) {
		res.status = 200;
		constexpr const char *body = R"(
{
	"username" : "liblinphone-tester",
	"password" : "retset-enohpnilbil",
	"ttl" : 10,
	"uris" : [
		"turn:sip1.linphone.org:3479?transport=udp"
	]
}
						)";
		res.set_content(body, "application/json");
	});

	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	configure_nat_policy_with_turn(marie, httpServer.mRootUrl);
	configure_nat_policy_with_turn(pauline, httpServer.mRootUrl);

	linphone_core_enable_forced_ice_relay(marie->lc, TRUE);
	linphone_core_enable_forced_ice_relay(pauline->lc, TRUE);

	if (!BC_ASSERT_TRUE(call(pauline, marie))) return;

	/*wait for the ICE reINVITE to complete*/
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));

	BC_ASSERT_TRUE(check_ice(pauline, marie, LinphoneIceStateRelayConnection));

	check_nb_media_starts(AUDIO_START, pauline, marie, 1, 1);

	liblinphone_tester_check_rtcp(marie, pauline);

	check_turn_username_password(marie, username, password);
	check_turn_username_password(pauline, username, password);

	/*then close the call*/
	end_call(pauline, marie);

	wait_for_until(marie->lc, pauline->lc, nullptr, 1, 10000);
	BC_ASSERT_PTR_NULL(linphone_core_find_auth_info(marie->lc, nullptr, "liblinphone-tester", nullptr));
	BC_ASSERT_PTR_NULL(linphone_core_find_auth_info(pauline->lc, nullptr, "liblinphone-tester", nullptr));

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

test_t turn_server_tests[] = {
    TEST_NO_TAG("Update TURN configuration", update_turn_configuration_test),
};

test_suite_t turn_server_test_suite = {"Turn",
                                       nullptr,
                                       nullptr,
                                       liblinphone_tester_before_each,
                                       liblinphone_tester_after_each,
                                       sizeof(turn_server_tests) / sizeof(turn_server_tests[0]),
                                       turn_server_tests,
                                       0};