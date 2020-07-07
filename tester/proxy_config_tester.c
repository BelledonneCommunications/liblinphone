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

#include "liblinphone_tester.h"
#include "tester_utils.h"

void linphone_proxy_config_stop_refreshing(LinphoneProxyConfig *obj);

#include <stdlib.h>

const char* phone_normalization(LinphoneProxyConfig *proxy, const char* in) {
	static char result[255];
	char * output = linphone_proxy_config_normalize_phone_number(proxy, in);
	if (output) {
		memcpy(result, output, strlen(output)+1);
		ms_free(output);
		return result;
	} else {
		return NULL;
	}
}

static void phone_normalization_without_proxy(void) {
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "012 345 6789"), "0123456789");
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "+33123456789"), "+33123456789");
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "+33012345678"), "+33012345678");
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "+33 0012345678"), "+33012345678");
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "+33012345678"), "+33012345678");
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "+3301234567891"), "+33234567891");
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "+33 01234567891"), "+33234567891");
	BC_ASSERT_PTR_NULL(phone_normalization(NULL, "I_AM_NOT_A_NUMBER")); // invalid phone number

	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "0"), "0");
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "01"), "01");
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "012"), "012");
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "0123"), "0123");
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "01234"), "01234");
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "012345"), "012345");
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "0123456"), "0123456");
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "01234567"), "01234567");
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "012345678"), "012345678");
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "0123456789"), "0123456789");
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "01234567890"), "01234567890");
}

static void phone_normalization_with_proxy(void) {
	LinphoneProxyConfig *proxy = linphone_core_create_proxy_config(NULL);
	linphone_proxy_config_set_dial_prefix(proxy, "33");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "012 3456 789"), "+33123456789");
	linphone_proxy_config_set_dial_prefix(proxy, NULL);
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+33123456789"), "+33123456789");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+33 0123456789"), "+33123456789");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+330012345678"), "+33012345678");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+3301 2345678"), "+33012345678");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+3301234567891"), "+33234567891");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+33 (0) 1 23 45 67 89"), "+33123456789");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+90 (903) 1234567"), "+909031234567");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0033123456789"), "0033123456789");

	linphone_proxy_config_set_dial_prefix(proxy, "33");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "123456789"), "+33123456789");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, " 0123456789"), "+33123456789");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0012345678"), "+12345678");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "01 2345678"), "+33012345678");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "01234567891"), "+33234567891"); // invalid phone number (too long)
	BC_ASSERT_PTR_NULL(phone_normalization(proxy, "I_AM_NOT_A_NUMBER")); // invalid phone number

	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+990012345678"), "+990012345678");

	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0952636505"), "+33952636505");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "09 52 63 65 05"), "+33952636505");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "09-52-63-65-05"), "+33952636505");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+31952636505"), "+31952636505");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0033952636505"), "+33952636505");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0033952636505"), "+33952636505");
	BC_ASSERT_PTR_NULL(phone_normalization(proxy, "toto"));


	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0"), "+330");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "01"), "+3301");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "012"), "+33012");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0123"), "+330123");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "01234"), "+3301234");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "012345"), "+33012345");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0123456"), "+330123456");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "01234567"), "+3301234567");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "012345678"), "+33012345678");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0123456789"), "+33123456789");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "01234567890"), "+33234567890");

	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+330"), "+330");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+3301"), "+3301");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+33012"), "+33012");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+330123"), "+330123");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+3301234"), "+3301234");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+33012345"), "+33012345");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+330123456"), "+330123456");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+3301234567"), "+3301234567");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+33012345678"), "+33012345678");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+330123456789"), "+33123456789");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+3301234567890"), "+33234567890");

	// invalid prefix - use a generic dialplan with 10 max length
	linphone_proxy_config_set_dial_prefix(proxy, "99");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0012345678"), "+12345678");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0"), "+990");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "01"), "+9901");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "012"), "+99012");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0123"), "+990123");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "01234"), "+9901234");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "012345"), "+99012345");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0123456"), "+990123456");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "01234567"), "+9901234567");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "012345678"), "+99012345678");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0123456789"), "+990123456789");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "01234567890"), "+991234567890");

	// Phone normalization for mexican dial plans
	linphone_proxy_config_set_dial_prefix(proxy, "52");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+5217227718184"), "+5217227718184"); /*this is a mobile phone number */
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+528127718184"), "+528127718184"); /*this is a landline phone number from Monterrey*/
	
	BC_ASSERT_EQUAL(linphone_dial_plan_lookup_ccc_from_e164("+522824713146"), 52, int, "%i"); /*this is a landline phone number*/
	

	// Phone normalization for myanmar dial plans
	linphone_proxy_config_set_dial_prefix(proxy, "95");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "9965066691"), "+959965066691");

	// Phone normalization for cameroon dial plans
	linphone_proxy_config_set_dial_prefix(proxy, "237");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "674788175"), "+237674788175");

	linphone_proxy_config_unref(proxy);
}

static void phone_normalization_with_dial_escape_plus(void){
	LinphoneProxyConfig *proxy = linphone_core_create_proxy_config(NULL);
	linphone_proxy_config_set_dial_prefix(proxy, "33");
	linphone_proxy_config_set_dial_escape_plus(proxy, TRUE);
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0033952636505"), "0033952636505");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0952636505"), "0033952636505");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+34952636505"), "0034952636505");

	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0"), "00330");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "01"), "003301");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "012"), "0033012");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0123"), "00330123");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "01234"), "003301234");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "012345"), "0033012345");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0123456"), "00330123456");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "01234567"), "003301234567");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "012345678"), "0033012345678");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0123456789"), "0033123456789");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "01234567890"), "0033234567890");

	linphone_proxy_config_set_dial_escape_plus(proxy, FALSE);
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+34952636505"), "+34952636505");

	linphone_proxy_config_unref(proxy);
}

#define SIP_URI_CHECK(actual, expected) { \
		LinphoneProxyConfig *proxy = linphone_core_create_proxy_config(NULL); \
		LinphoneAddress* res;\
		char* actual_str;\
		LinphoneAddress *addr = linphone_address_new("sip:username@linphone.org"); \
		linphone_proxy_config_set_identity_address(proxy, addr); \
		if (addr) linphone_address_unref(addr); \
		res  = linphone_proxy_config_normalize_sip_uri(proxy, actual); \
		actual_str = linphone_address_as_string_uri_only(res); \
		BC_ASSERT_STRING_EQUAL(actual_str, expected); \
		ms_free(actual_str); \
		linphone_address_unref(res); \
		linphone_proxy_config_unref(proxy); \
	}


static void sip_uri_normalization(void) {
	char* expected ="sip:%d9%a1@linphone.org";
	BC_ASSERT_PTR_NULL(linphone_proxy_config_normalize_sip_uri(NULL, "test"));
	SIP_URI_CHECK("test@linphone.org", "sip:test@linphone.org");
	SIP_URI_CHECK("test@linphone.org;transport=tls", "sip:test@linphone.org;transport=tls");

	SIP_URI_CHECK("١", expected); //test that no more invalid memory writes are made (valgrind only)
}

static void load_dynamic_proxy_config(void) {
	LinphoneCoreManager *lauriane = linphone_core_manager_new(NULL);
	LinphoneProxyConfig *proxy;
	LinphoneAddress *read, *expected;
	LinphoneNatPolicy *nat_policy;
	const char* config =	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
							"<config xmlns=\"http://www.linphone.org/xsds/lpconfig.xsd\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.linphone.org/xsds/lpconfig.xsd lpconfig.xsd\">\r\n"
							"<section name=\"proxy_default_values\">\r\n"
								"<entry name=\"avpf\" overwrite=\"true\">1</entry>\r\n"
								"<entry name=\"dial_escape_plus\" overwrite=\"true\">0</entry>\r\n"
								"<entry name=\"publish\" overwrite=\"true\">0</entry>\r\n"
								"<entry name=\"quality_reporting_collector\" overwrite=\"true\">sip:voip-metrics@sip.linphone.org;transport=tls</entry>\r\n"
								"<entry name=\"quality_reporting_enabled\" overwrite=\"true\">1</entry>\r\n"
								"<entry name=\"quality_reporting_interval\" overwrite=\"true\">180</entry>\r\n"
								"<entry name=\"reg_expires\" overwrite=\"true\">31536000</entry>\r\n"
								"<entry name=\"reg_identity\" overwrite=\"true\">sip:?@sip.linphone.org</entry>\r\n"
								"<entry name=\"reg_proxy\" overwrite=\"true\">&lt;sip:sip.linphone.org;transport=tls&gt;</entry>\r\n"
								"<entry name=\"reg_sendregister\" overwrite=\"true\">1</entry>\r\n"
								"<entry name=\"nat_policy_ref\" overwrite=\"true\">nat_policy_default_values</entry>\r\n"
								"<entry name=\"realm\" overwrite=\"true\">sip.linphone.org</entry>\r\n"
							"</section>\r\n"
							"<section name=\"nat_policy_default_values\">\r\n"
								"<entry name=\"stun_server\" overwrite=\"true\">stun.linphone.org</entry>\r\n"
								"<entry name=\"protocols\" overwrite=\"true\">stun,ice</entry>\r\n"
							"</section>\r\n"
							"</config>";
	BC_ASSERT_FALSE(linphone_config_load_from_xml_string(linphone_core_get_config(lauriane->lc),config));
	proxy = linphone_core_create_proxy_config(lauriane->lc);

	read = linphone_address_new(linphone_proxy_config_get_server_addr(proxy));
	expected = linphone_address_new("sip:sip.linphone.org;transport=tls");

	BC_ASSERT_TRUE(linphone_address_equal(read,expected));
	linphone_address_unref(read);
	linphone_address_unref(expected);

	nat_policy = linphone_proxy_config_get_nat_policy(proxy);

	if (BC_ASSERT_PTR_NOT_NULL(nat_policy)) {
		BC_ASSERT_TRUE(linphone_nat_policy_ice_enabled(nat_policy));
		BC_ASSERT_TRUE(linphone_nat_policy_stun_enabled(nat_policy));
		BC_ASSERT_FALSE(linphone_nat_policy_turn_enabled(nat_policy));
	}
	linphone_proxy_config_unref(proxy);
	linphone_core_manager_destroy(lauriane);

	//BC_ASSERT_STRING_EQUAL(linphone_proxy_config_get(proxy), "sip:sip.linphone.org;transport=tls");

}

static void single_route(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneProxyConfig *marie_cfg = linphone_core_get_default_proxy_config(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_cfg);

	const bctbx_list_t *routes = linphone_proxy_config_get_routes(marie_cfg);
	BC_ASSERT_PTR_NOT_NULL(routes);
	BC_ASSERT_EQUAL(bctbx_list_size(routes), 1, int, "%d");
	const char *route = (const char *)bctbx_list_get_data(routes);
	BC_ASSERT_STRING_EQUAL(linphone_proxy_config_get_route(marie_cfg), "<sip:sip.example.org;transport=tcp>");
	BC_ASSERT_STRING_EQUAL(route, "<sip:sip.example.org;transport=tcp>");

	linphone_proxy_config_set_route(marie_cfg, "sip.linphone.org");
	routes = linphone_proxy_config_get_routes(marie_cfg);
	BC_ASSERT_PTR_NOT_NULL(routes);
	BC_ASSERT_EQUAL(bctbx_list_size(routes), 1, int, "%d");
	route = (const char *)bctbx_list_get_data(routes);
	BC_ASSERT_STRING_EQUAL(linphone_proxy_config_get_route(marie_cfg), "sip:sip.linphone.org");
	BC_ASSERT_STRING_EQUAL(route, "sip:sip.linphone.org");

	linphone_core_manager_destroy(marie);
}

/*
 * Dependent proxy config scenario: pauline@example.org, marie@example.org and marie@sip2.linphone.org
 * marie@sip2.linphone.org is marked 'Dependent' on marie@example.org.
 * Once all registered, we cut the marie@sip2.linphone.org connection.
 * A call from pauline@example.org to marie@sip2.linphone.org should work (and go through example.org instead of sip2.linphone.org)
 */
static void dependent_proxy_config(void) {
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_external_rc");
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_dependent_proxy_rc");
	LinphoneProxyConfig *marie_cfg = linphone_core_get_default_proxy_config(marie->lc);
	LinphoneProxyConfig *marie_dependent_cfg = (LinphoneProxyConfig *) linphone_core_get_proxy_config_list(marie->lc)->next->data;
	BC_ASSERT_PTR_NOT_NULL(marie_cfg);
	BC_ASSERT_PTR_NOT_NULL(marie_dependent_cfg);

	BC_ASSERT_PTR_EQUAL(marie_cfg, linphone_proxy_config_get_dependency(marie_dependent_cfg));

	BC_ASSERT_TRUE(wait_for(pauline->lc, NULL, &pauline->stat.number_of_LinphoneRegistrationOk, 1));

	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 2));

	const LinphoneAddress *marie_cfg_contact = linphone_proxy_config_get_contact(marie_cfg);
	const LinphoneAddress *marie_dependent_cfg_contact = linphone_proxy_config_get_contact(marie_dependent_cfg);

	BC_ASSERT_TRUE(linphone_proxy_config_address_equal(marie_cfg_contact, marie_dependent_cfg_contact) == LinphoneProxyConfigAddressEqual);

	//Cut link for dependent proxy config, then call its identity address and check that we receive the call
	//(which would be received through the 'master' proxy config server)
	linphone_core_set_network_reachable(marie->lc, FALSE);
	linphone_proxy_config_edit(marie_dependent_cfg);
	linphone_proxy_config_enable_register(marie_dependent_cfg, FALSE);
	linphone_proxy_config_done(marie_dependent_cfg);
 	linphone_core_set_network_reachable(marie->lc, TRUE);

	wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneRegistrationOk, 3); //One more time for 'master' proxy config

	LinphoneAddress *marie_dependent_addr = linphone_address_new(linphone_proxy_config_get_identity(marie_dependent_cfg));

	linphone_core_invite_address(pauline->lc, marie_dependent_addr);

	if (BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallIncomingReceived, 1, 10000))) {
		linphone_call_accept(linphone_core_get_current_call(marie->lc));
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));
		end_call(pauline, marie);
	}

	linphone_address_unref(marie_dependent_addr);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

//Dependent proxy config should not register if its dependency is not in a LinphoneRegistrationOk state
static void proxy_config_dependent_register(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_dependent_proxy_rc");
	const bctbx_list_t *proxyConfigs = linphone_core_get_proxy_config_list(marie->lc);
	LinphoneProxyConfig *master = (LinphoneProxyConfig *) proxyConfigs->data;

	linphone_proxy_config_edit(master);
	linphone_proxy_config_set_server_addr(master, "sip:cannotberesol.ved");
	linphone_proxy_config_done(master);

	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 0));

	linphone_core_manager_destroy(marie);
}

//Dependent proxy config should	mirror the state of its 'parent'
static void proxy_config_dependent_register_state_changed(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_dependent_proxy_rc");
	const bctbx_list_t *proxyConfigs = linphone_core_get_proxy_config_list(marie->lc);
	LinphoneProxyConfig *master = (LinphoneProxyConfig *) proxyConfigs->data;

	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 2));

	linphone_proxy_config_edit(master);
	char *keep_server_addr = bctbx_strdup(linphone_proxy_config_get_server_addr(master));
	linphone_proxy_config_set_server_addr(master, "sip:cannotberesol.ved");
	linphone_proxy_config_done(master);

	//Both configs should now be in 'LinphoneRegistrationFailed' state
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationFailed, 2));

	marie->stat.number_of_LinphoneRegistrationOk = 0;
	linphone_proxy_config_edit(master);
	linphone_proxy_config_set_server_addr(master, keep_server_addr);
	linphone_proxy_config_done(master);

	//Ok again.
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 2));

	linphone_proxy_config_edit(master);
	linphone_proxy_config_enable_register(master, FALSE);
	linphone_proxy_config_done(master);

	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationCleared, 2));

	bctbx_free(keep_server_addr);
	linphone_core_manager_destroy(marie);
}


//A dependent proxy config should behave as a normal one after removal of dependency
static void dependent_proxy_dependency_removal(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_dependent_proxy_rc");
	const bctbx_list_t *proxyConfigs = linphone_core_get_proxy_config_list(marie->lc);
	LinphoneProxyConfig *master = (LinphoneProxyConfig *) proxyConfigs->data;

	linphone_core_set_network_reachable(marie->lc, FALSE);

	linphone_core_remove_proxy_config(marie->lc, master);

	proxyConfigs = linphone_core_get_proxy_config_list(marie->lc);

	BC_ASSERT_EQUAL(bctbx_list_size(proxyConfigs), 1, int, "%d");

	linphone_core_set_network_reachable(marie->lc, TRUE);

	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 1));

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRegistrationOk, 1, int, "%d");

	linphone_core_manager_destroy(marie);
}

static void dependent_proxy_dependency_with_core_reloaded(void){
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_external_rc");
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_dependent_proxy_rc");
	LinphoneProxyConfig *marie_cfg = linphone_core_get_default_proxy_config(marie->lc);
	LinphoneProxyConfig *marie_dependent_cfg = (LinphoneProxyConfig *) linphone_core_get_proxy_config_list(marie->lc)->next->data;
	LinphoneAddress *marie_master_address, *marie_secondary_address;
	
	BC_ASSERT_PTR_NOT_NULL(marie_cfg);
	BC_ASSERT_PTR_NOT_NULL(marie_dependent_cfg);
	
	BC_ASSERT_TRUE(wait_for(pauline->lc, NULL, &pauline->stat.number_of_LinphoneRegistrationOk, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 2));
	BC_ASSERT_PTR_EQUAL(marie_cfg, linphone_proxy_config_get_dependency(marie_dependent_cfg));
	
	marie_master_address = linphone_address_clone(linphone_proxy_config_get_identity_address(marie_cfg));
	marie_secondary_address = linphone_address_clone(linphone_proxy_config_get_identity_address(marie_dependent_cfg));
	
	/* Clear all proxy config, wait for unregistration*/
	linphone_core_clear_proxy_config(marie->lc);
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationCleared, 2));

	/* Now re-enter the proxy config in reverse order: the dependant first, the master second. */
	
	marie_dependent_cfg = linphone_core_create_proxy_config(marie->lc);
	linphone_proxy_config_set_identity_address(marie_dependent_cfg, marie_secondary_address);
	linphone_proxy_config_set_server_addr(marie_dependent_cfg, "sip:external.example.org:5068;transport=tcp");
	linphone_proxy_config_set_route(marie_dependent_cfg, "sip:external.example.org:5068;transport=tcp");
	linphone_proxy_config_enable_register(marie_dependent_cfg, TRUE);
	
	marie_cfg = linphone_core_create_proxy_config(marie->lc);
	linphone_proxy_config_set_identity_address(marie_cfg, marie_master_address);
	linphone_proxy_config_set_server_addr(marie_cfg, "sip:sipopen.example.org;transport=tls");
	linphone_proxy_config_enable_register(marie_cfg, TRUE);

	/* Now add them to the core: */
	linphone_core_add_proxy_config(marie->lc, marie_dependent_cfg);
	linphone_core_add_proxy_config(marie->lc, marie_cfg);
	linphone_core_set_default_proxy_config(marie->lc, marie_cfg);
	
	/* Setup their dependency: */
	linphone_proxy_config_set_dependency(marie_dependent_cfg, marie_cfg);
	
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 4));
	BC_ASSERT_PTR_EQUAL(marie_cfg, linphone_proxy_config_get_dependency(marie_dependent_cfg));
	
	/* Then everything should work as if order was the same as before. */

	const LinphoneAddress *marie_cfg_contact = linphone_proxy_config_get_contact(marie_cfg);
	const LinphoneAddress *marie_dependent_cfg_contact = linphone_proxy_config_get_contact(marie_dependent_cfg);

	BC_ASSERT_TRUE(linphone_proxy_config_address_equal(marie_cfg_contact, marie_dependent_cfg_contact) == LinphoneProxyConfigAddressEqual);

	//Cut link for dependent proxy config, then call its identity address and check that we receive the call
	//(which would be received through the 'master' proxy config server)
	linphone_core_set_network_reachable(marie->lc, FALSE);
	linphone_proxy_config_edit(marie_dependent_cfg);
	linphone_proxy_config_enable_register(marie_dependent_cfg, FALSE);
	linphone_proxy_config_done(marie_dependent_cfg);
 	linphone_core_set_network_reachable(marie->lc, TRUE);

	wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneRegistrationOk, 5); //One more time for 'master' proxy config

	LinphoneAddress *marie_dependent_addr = linphone_address_new(linphone_proxy_config_get_identity(marie_dependent_cfg));

	linphone_core_invite_address(pauline->lc, marie_dependent_addr);

	if (BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallIncomingReceived, 1, 10000))) {
		linphone_call_accept(linphone_core_get_current_call(marie->lc));
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));
		end_call(pauline, marie);
	}
	linphone_address_unref(marie_dependent_addr);
	
	/* re-enable marie's registration on dependent proxy config*/
	linphone_proxy_config_edit(marie_dependent_cfg);
	linphone_proxy_config_enable_register(marie_dependent_cfg, TRUE);
	linphone_proxy_config_done(marie_dependent_cfg);
	
	wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneRegistrationOk, 6);
	
	/* Now marie's core stops and restarts. */
	linphone_core_stop(marie->lc);
	linphone_core_start(marie->lc);
	linphone_core_manager_setup_dns(marie);
	
	/* Check that configuration could be reloaded correctly */
	BC_ASSERT_EQUAL(bctbx_list_size(linphone_core_get_proxy_config_list(marie->lc)), 2, int, "%i");
	
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 8));
	
	/* And make new call attempt */
	
	marie_cfg = (LinphoneProxyConfig *) linphone_core_get_proxy_config_list(marie->lc)->next->data;
	marie_dependent_cfg = (LinphoneProxyConfig *) linphone_core_get_proxy_config_list(marie->lc)->data;
	BC_ASSERT_PTR_EQUAL(marie_cfg, linphone_proxy_config_get_dependency(marie_dependent_cfg));

	marie_cfg_contact = linphone_proxy_config_get_contact(marie_cfg);
	marie_dependent_cfg_contact = linphone_proxy_config_get_contact(marie_dependent_cfg);

	BC_ASSERT_TRUE(linphone_proxy_config_address_equal(marie_cfg_contact, marie_dependent_cfg_contact) == LinphoneProxyConfigAddressEqual);

	//Cut link for dependent proxy config, then call its identity address and check that we receive the call
	//(which would be received through the 'master' proxy config server)
	linphone_core_set_network_reachable(marie->lc, FALSE);
	linphone_proxy_config_edit(marie_dependent_cfg);
	linphone_proxy_config_enable_register(marie_dependent_cfg, FALSE);
	linphone_proxy_config_done(marie_dependent_cfg);
 	linphone_core_set_network_reachable(marie->lc, TRUE);

	wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneRegistrationOk, 9); //One more time for 'master' proxy config

	marie_dependent_addr = linphone_address_new(linphone_proxy_config_get_identity(marie_dependent_cfg));

	linphone_core_invite_address(pauline->lc, marie_dependent_addr);

	if (BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallIncomingReceived, 2, 10000))) {
		linphone_call_accept(linphone_core_get_current_call(marie->lc));
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
		end_call(pauline, marie);
	}

	linphone_address_unref(marie_dependent_addr);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

extern void linphone_core_update_push_notification_information(LinphoneCore *core, const char *param, const char *prid);

static void proxy_config_push_notification_scenario_1(bool_t use_legacy_format, bool_t multi_config, bool_t both_push) {
	const char *rc = "marie_rc";
	if (multi_config) 
		rc = "marie_dual_proxy_rc";
		
	LinphoneCoreManager *marie = linphone_core_manager_new(rc);
	LinphoneProxyConfig *marie_cfg = linphone_core_get_default_proxy_config(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_cfg);
	const bctbx_list_t *proxies = linphone_core_get_proxy_config_list(marie->lc);
	int proxy_config_count = bctbx_list_size(proxies);
	LinphoneProxyConfig *marie_cfg_2 = NULL;
	
	if (multi_config) {
		BC_ASSERT_EQUAL(proxy_config_count, 2, int, "%d");
		marie_cfg_2 = (LinphoneProxyConfig *)proxies->next->data;
		BC_ASSERT_PTR_NOT_NULL(marie_cfg_2);
	} else {
		BC_ASSERT_EQUAL(proxy_config_count, 1, int, "%d");
		BC_ASSERT_PTR_NULL(marie_cfg_2);
	}

	if (use_legacy_format) {
		lp_config_set_int(linphone_core_get_config(marie->lc), "net", "use_legacy_push_notification_params", TRUE);
	}

	BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg));
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
	BC_ASSERT_FALSE(linphone_core_is_push_notification_enabled(marie->lc));
	BC_ASSERT_FALSE(linphone_core_is_push_notification_available(marie->lc));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count, int, "%i");
	if (multi_config) {
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
		BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
	}

	// First: enable push
	linphone_core_set_push_notification_enabled(marie->lc, TRUE);
	// Nothing should happen, push information aren't set
	BC_ASSERT_TRUE(linphone_core_is_push_notification_enabled(marie->lc));
	BC_ASSERT_FALSE(linphone_core_is_push_notification_available(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg));
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count, int, "%i");
	if (multi_config) {
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
		BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
	}

	// Second: configure push informations
	linphone_core_update_push_notification_information(marie->lc, "test-app-id", "test-push-token");
	// Nothing should happen, push aren't allowed on proxy config
	BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg));
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
	BC_ASSERT_TRUE(linphone_core_is_push_notification_available(marie->lc));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count, int, "%i");
	if (multi_config) {
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
		BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
	}

	// Third: allow push notification on proxy config
	linphone_proxy_config_set_push_notification_allowed(marie_cfg, TRUE);
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
	const char *uriParams = linphone_proxy_config_get_contact_uri_parameters(marie_cfg);
	BC_ASSERT_PTR_NOT_NULL(uriParams);
	if (uriParams) {
#ifdef __ANDROID__
		const char *expected = "pn-provider=fcm;pn-param=test-app-id;pn-prid=test-push-token;pn-timeout=0;pn-silent=1";
#elif TARGET_OS_IPHONE
		const char *expected = "pn-provider=apple;pn-param=test-app-id;pn-prid=test-push-token;pn-timeout=0;pn-silent=1";
#else
		const char *expected = "pn-provider=liblinphone_tester;pn-param=test-app-id;pn-prid=test-push-token;pn-timeout=0;pn-silent=1";
#endif
		if (use_legacy_format) {
#ifdef __ANDROID__
			expected = "pn-type=firebase;app-id=test-app-id;pn-tok=test-push-token;pn-timeout=0;pn-silent=1";
#elif TARGET_OS_IPHONE
			expected = "pn-type=apple;app-id=test-app-id;pn-tok=test-push-token;pn-timeout=0;pn-silent=1";
#else
			expected = "pn-type=liblinphone_tester;app-id=test-app-id;pn-tok=test-push-token;pn-timeout=0;pn-silent=1";
#endif
		}
		BC_ASSERT_STRING_EQUAL(uriParams, expected);
	}

	if (both_push) {
		linphone_proxy_config_set_push_notification_allowed(marie_cfg_2, TRUE);
		BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
		BC_ASSERT_PTR_NOT_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
		proxy_config_count++;
	}

	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count+1, 10000));
	if (multi_config && !both_push) {
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
		BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
	}

	// Fourth: disallow push notification on proxy config
	linphone_proxy_config_set_push_notification_allowed(marie_cfg, FALSE);
	if (both_push) {
		linphone_proxy_config_set_push_notification_allowed(marie_cfg_2, FALSE);
		proxy_config_count++;
	}
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
	BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg));
	BC_ASSERT_TRUE(linphone_core_is_push_notification_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_is_push_notification_available(marie->lc));
	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count+2, 10000));
	if (multi_config) {
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
		BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
	}

	linphone_core_manager_destroy(marie);
}

static void proxy_config_push_notification_scenario_2(bool_t use_legacy_format, bool_t multi_config, bool_t both_push) {
	const char *rc = "marie_rc";
	if (multi_config) 
		rc = "marie_dual_proxy_rc";
		
	LinphoneCoreManager *marie = linphone_core_manager_new(rc);
	LinphoneProxyConfig *marie_cfg = linphone_core_get_default_proxy_config(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_cfg);
	const bctbx_list_t *proxies = linphone_core_get_proxy_config_list(marie->lc);
	int proxy_config_count = bctbx_list_size(proxies);
	LinphoneProxyConfig *marie_cfg_2 = NULL;
	
	if (multi_config) {
		BC_ASSERT_EQUAL(proxy_config_count, 2, int, "%d");
		marie_cfg_2 = (LinphoneProxyConfig *)proxies->next->data;
		BC_ASSERT_PTR_NOT_NULL(marie_cfg_2);
	} else {
		BC_ASSERT_EQUAL(proxy_config_count, 1, int, "%d");
		BC_ASSERT_PTR_NULL(marie_cfg_2);
	}
	
	if (use_legacy_format) {
		lp_config_set_int(linphone_core_get_config(marie->lc), "net", "use_legacy_push_notification_params", TRUE);
	}

	BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg));
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
	BC_ASSERT_FALSE(linphone_core_is_push_notification_enabled(marie->lc));
	BC_ASSERT_FALSE(linphone_core_is_push_notification_available(marie->lc));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count, int, "%i");
	if (multi_config) {
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
		BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
	}

	// First: enable push
	linphone_core_set_push_notification_enabled(marie->lc, TRUE);
	// Nothing should happen, push information aren't set
	BC_ASSERT_TRUE(linphone_core_is_push_notification_enabled(marie->lc));
	BC_ASSERT_FALSE(linphone_core_is_push_notification_available(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg));
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count, int, "%i");
	if (multi_config) {
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
		BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
	}

	// Second: allow push notification on proxy config
	linphone_proxy_config_set_push_notification_allowed(marie_cfg, TRUE);
	if (both_push) {
		linphone_proxy_config_set_push_notification_allowed(marie_cfg_2, TRUE);
		BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
	}
	// Nothing should happen, push informations aren't supplied
	BC_ASSERT_FALSE(linphone_core_is_push_notification_available(marie->lc));
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
	BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count, int, "%i");
	if (multi_config) {
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
	}

	// Third: configure push informations
	linphone_core_update_push_notification_information(marie->lc, "test-app-id", "test-push-token");
	BC_ASSERT_TRUE(linphone_core_is_push_notification_available(marie->lc));
	const char *uriParams = linphone_proxy_config_get_contact_uri_parameters(marie_cfg);
	BC_ASSERT_PTR_NOT_NULL(uriParams);
	if (uriParams) {
#ifdef __ANDROID__
		const char *expected = "pn-provider=fcm;pn-param=test-app-id;pn-prid=test-push-token;pn-timeout=0;pn-silent=1";
#elif TARGET_OS_IPHONE
		const char *expected = "pn-provider=apple;pn-param=test-app-id;pn-prid=test-push-token;pn-timeout=0;pn-silent=1";
#else
		const char *expected = "pn-provider=liblinphone_tester;pn-param=test-app-id;pn-prid=test-push-token;pn-timeout=0;pn-silent=1";
#endif
		if (use_legacy_format) {
#ifdef __ANDROID__
			expected = "pn-type=firebase;app-id=test-app-id;pn-tok=test-push-token;pn-timeout=0;pn-silent=1";
#elif TARGET_OS_IPHONE
			expected = "pn-type=apple;app-id=test-app-id;pn-tok=test-push-token;pn-timeout=0;pn-silent=1";
#else
			expected = "pn-type=liblinphone_tester;app-id=test-app-id;pn-tok=test-push-token;pn-timeout=0;pn-silent=1";
#endif
		}
		BC_ASSERT_STRING_EQUAL(uriParams, expected);
		BC_ASSERT_STRING_EQUAL(uriParams, expected);
	}

	if (both_push) {
		BC_ASSERT_PTR_NOT_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
		proxy_config_count++;
	}

	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count+1, 10000));
	if (multi_config && !both_push) {
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
		BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
	}

	// Fourth: disable push notification on core
	linphone_core_set_push_notification_enabled(marie->lc, FALSE);
	BC_ASSERT_FALSE(linphone_core_is_push_notification_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_is_push_notification_available(marie->lc));
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
	BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg));
	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count+2, 10000));
	if (multi_config) {
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
		if (both_push)
			BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
		else
			BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
	}
	
	linphone_core_manager_destroy(marie);
}

static void proxy_config_push_notification_scenario_3(bool_t use_legacy_format, bool_t multi_config, bool_t both_push) {
	const char *rc = "marie_rc";
	if (multi_config) 
		rc = "marie_dual_proxy_rc";
		
	LinphoneCoreManager *marie = linphone_core_manager_new(rc);
	LinphoneProxyConfig *marie_cfg = linphone_core_get_default_proxy_config(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_cfg);
	const bctbx_list_t *proxies = linphone_core_get_proxy_config_list(marie->lc);
	int proxy_config_count = bctbx_list_size(proxies);
	LinphoneProxyConfig *marie_cfg_2 = NULL;
	
	if (multi_config) {
		BC_ASSERT_EQUAL(proxy_config_count, 2, int, "%d");
		marie_cfg_2 = (LinphoneProxyConfig *)proxies->next->data;
		BC_ASSERT_PTR_NOT_NULL(marie_cfg_2);
	} else {
		BC_ASSERT_EQUAL(proxy_config_count, 1, int, "%d");
		BC_ASSERT_PTR_NULL(marie_cfg_2);
	}
	
	if (use_legacy_format) {
		lp_config_set_int(linphone_core_get_config(marie->lc), "net", "use_legacy_push_notification_params", TRUE);
	}

	BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg));
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
	if (multi_config) {
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
		BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
	}
	BC_ASSERT_FALSE(linphone_core_is_push_notification_enabled(marie->lc));
	BC_ASSERT_FALSE(linphone_core_is_push_notification_available(marie->lc));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count, int, "%i");

	// First: allow push notification on proxy config
	linphone_proxy_config_set_push_notification_allowed(marie_cfg, TRUE);
	if (both_push) {
		linphone_proxy_config_set_push_notification_allowed(marie_cfg_2, TRUE);
		BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
	}
	// Nothing should happen, push aren't enabled at core level
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
	BC_ASSERT_FALSE(linphone_core_is_push_notification_enabled(marie->lc));
	BC_ASSERT_FALSE(linphone_core_is_push_notification_available(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count, int, "%i");
	if (multi_config) {
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
		if (both_push)
			BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
		else
			BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
	}

	// Second: enable push
	linphone_core_set_push_notification_enabled(marie->lc, TRUE);
	// Nothing should happen, push informations aren't supplied
	BC_ASSERT_TRUE(linphone_core_is_push_notification_enabled(marie->lc));
	BC_ASSERT_FALSE(linphone_core_is_push_notification_available(marie->lc));
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
	BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count, int, "%i");
	if (multi_config) {
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
		if (both_push)
			BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
		else
			BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
	}

	// Third: configure push informations
	linphone_core_update_push_notification_information(marie->lc, "test-app-id", "test-push-token");
	BC_ASSERT_TRUE(linphone_core_is_push_notification_available(marie->lc));
	const char *uriParams = linphone_proxy_config_get_contact_uri_parameters(marie_cfg);
	BC_ASSERT_PTR_NOT_NULL(uriParams);
	if (uriParams) {
#ifdef __ANDROID__
		const char *expected = "pn-provider=fcm;pn-param=test-app-id;pn-prid=test-push-token;pn-timeout=0;pn-silent=1";
#elif TARGET_OS_IPHONE
		const char *expected = "pn-provider=apple;pn-param=test-app-id;pn-prid=test-push-token;pn-timeout=0;pn-silent=1";
#else
		const char *expected = "pn-provider=liblinphone_tester;pn-param=test-app-id;pn-prid=test-push-token;pn-timeout=0;pn-silent=1";
#endif
		if (use_legacy_format) {
#ifdef __ANDROID__
			expected = "pn-type=firebase;app-id=test-app-id;pn-tok=test-push-token;pn-timeout=0;pn-silent=1";
#elif TARGET_OS_IPHONE
			expected = "pn-type=apple;app-id=test-app-id;pn-tok=test-push-token;pn-timeout=0;pn-silent=1";
#else
			expected = "pn-type=liblinphone_tester;app-id=test-app-id;pn-tok=test-push-token;pn-timeout=0;pn-silent=1";
#endif
		}
		BC_ASSERT_STRING_EQUAL(uriParams, expected);
	}
	if (both_push) {
		BC_ASSERT_PTR_NOT_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
		proxy_config_count++;
	}

	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count+1, 10000));
	if (multi_config && !both_push) {
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
		BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
	}

	// Fourth: update push information with null
	linphone_core_update_push_notification_information(marie->lc, NULL, NULL);
	BC_ASSERT_TRUE(linphone_core_is_push_notification_enabled(marie->lc));
	BC_ASSERT_FALSE(linphone_core_is_push_notification_available(marie->lc));
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
	BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg));
	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count+2, 10000));
	if (multi_config) {
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
		if (both_push)
			BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
		else
			BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
	}
	
	linphone_core_manager_destroy(marie);
}

static void proxy_config_push_notification_params(void) {
	proxy_config_push_notification_scenario_1(FALSE, FALSE, FALSE);
}

static void proxy_config_push_notification_params_2(void) {
	proxy_config_push_notification_scenario_2(FALSE, FALSE, FALSE);
}

static void proxy_config_push_notification_params_3(void) {
	proxy_config_push_notification_scenario_3(FALSE, FALSE, FALSE);
}

static void proxy_config_push_notification_legacy_params(void) {
	proxy_config_push_notification_scenario_1(TRUE, FALSE, FALSE);
}

static void proxy_config_push_notification_legacy_params_2(void) {
	proxy_config_push_notification_scenario_2(TRUE, FALSE, FALSE);
}

static void proxy_config_push_notification_legacy_params_3(void) {
	proxy_config_push_notification_scenario_3(TRUE, FALSE, FALSE);
}

static void proxy_config_push_notification_params_two_proxies_one_push(void) {
	proxy_config_push_notification_scenario_1(FALSE, TRUE, FALSE);
}

static void proxy_config_push_notification_params_two_proxies_one_push_2(void) {
	proxy_config_push_notification_scenario_2(FALSE, TRUE, FALSE);
}

static void proxy_config_push_notification_params_two_proxies_one_push_3(void) {
	proxy_config_push_notification_scenario_3(FALSE, TRUE, FALSE);
}

static void proxy_config_push_notification_params_two_proxies_both_push(void) {
	proxy_config_push_notification_scenario_1(FALSE, TRUE, TRUE);
}

static void proxy_config_push_notification_params_two_proxies_both_push_2(void) {
	proxy_config_push_notification_scenario_2(FALSE, TRUE, TRUE);
}

static void proxy_config_push_notification_params_two_proxies_both_push_3(void) {
	proxy_config_push_notification_scenario_3(FALSE, TRUE, TRUE);
}

test_t proxy_config_tests[] = {
	TEST_NO_TAG("Phone normalization without proxy", phone_normalization_without_proxy),
	TEST_NO_TAG("Phone normalization with proxy", phone_normalization_with_proxy),
	TEST_NO_TAG("Phone normalization with dial escape plus", phone_normalization_with_dial_escape_plus),
	TEST_NO_TAG("SIP URI normalization", sip_uri_normalization),
	TEST_NO_TAG("Load new default value for proxy config", load_dynamic_proxy_config),
	TEST_NO_TAG("Single route", single_route),
	TEST_NO_TAG("Proxy dependency", dependent_proxy_config),
	TEST_NO_TAG("Dependent proxy dependency register", proxy_config_dependent_register),
	TEST_NO_TAG("Dependent proxy state changed", proxy_config_dependent_register_state_changed),
	TEST_NO_TAG("Dependent proxy dependency removal", dependent_proxy_dependency_removal),
	TEST_ONE_TAG("Dependent proxy dependency with core reloaded", dependent_proxy_dependency_with_core_reloaded, "LeaksMemory"),
	TEST_ONE_TAG("Push notification params", proxy_config_push_notification_params, "Push Notification"),
	TEST_ONE_TAG("Push notification params 2", proxy_config_push_notification_params_2, "Push Notification"),
	TEST_ONE_TAG("Push notification params 3", proxy_config_push_notification_params_3, "Push Notification"),
	TEST_ONE_TAG("Push notification legacy params", proxy_config_push_notification_legacy_params, "Push Notification"),
	TEST_ONE_TAG("Push notification legacy params 2", proxy_config_push_notification_legacy_params_2, "Push Notification"),
	TEST_ONE_TAG("Push notification legacy params 3", proxy_config_push_notification_legacy_params_3, "Push Notification"),
	TEST_ONE_TAG("Push notification params two proxies, one push", proxy_config_push_notification_params_two_proxies_one_push, "Push Notification"),
	TEST_ONE_TAG("Push notification params two proxies, one push 2", proxy_config_push_notification_params_two_proxies_one_push_2, "Push Notification"),
	TEST_ONE_TAG("Push notification params two proxies, one push 3", proxy_config_push_notification_params_two_proxies_one_push_3, "Push Notification"),
	TEST_ONE_TAG("Push notification params two proxies, both push", proxy_config_push_notification_params_two_proxies_both_push, "Push Notification"),
	TEST_ONE_TAG("Push notification params two proxies, both push 2", proxy_config_push_notification_params_two_proxies_both_push_2, "Push Notification"),
	TEST_ONE_TAG("Push notification params two proxies, both push 3", proxy_config_push_notification_params_two_proxies_both_push_3, "Push Notification")
};

test_suite_t proxy_config_test_suite = {"Proxy config", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
										sizeof(proxy_config_tests) / sizeof(proxy_config_tests[0]), proxy_config_tests};
