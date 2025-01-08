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

#include "liblinphone_tester.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-dial-plan.h"
#include "linphone/api/c-nat-policy.h"
#include "tester_utils.h"

void linphone_proxy_config_stop_refreshing(LinphoneProxyConfig *obj);

#include <stdlib.h>

const char *phone_normalization(LinphoneProxyConfig *proxy, const char *in) {
	static char result[255];
	char *output = linphone_proxy_config_normalize_phone_number(proxy, in);
	if (output) {
		memcpy(result, output, strlen(output) + 1);
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

	// generic dial plan used, do no detect short phone number
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

	// no dial plan given, do no detect short phone numbers, add international prefix
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "003"), "003");   // generic dial plan
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "0033"), "0033"); // short number in France's dial plan
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "00331"), "00331");
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "0033 1 2345678"),
	                       "003312345678"); // not normal nor short number in France's dial plan, do nothing
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "0033 1 23456789"),
	                       "+33123456789"); // normal number in France's dial plan
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "0033 1 234567891"), "+33234567891");
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "001"), "001"); // short number in American Samoa's dial plan

	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "0039"), "0039"); // short number in Italy's dial plan
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "00391"), "00391");
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "0039 1 23456"),
	                       "+39123456"); // normal number in Italy's dial plan
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "0039123456789"), "+39123456789");
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "0039 1 2345678901"), "+3912345678901");
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "0039 1 23456789012"), "+3923456789012");

	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "003212345678"), "+3212345678"); // Belgium's dial plan
	BC_ASSERT_STRING_EQUAL(phone_normalization(NULL, "0032123456789"), "+32123456789");
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
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0033123456789"), "+33123456789");

	linphone_proxy_config_set_dial_prefix(proxy, "33");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "123456789"), "+33123456789");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, " 0123456789"), "+33123456789");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "01 23456789"), "+33123456789");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "01234567891"),
	                       "+33234567891");                              // invalid phone number (too long)
	BC_ASSERT_PTR_NULL(phone_normalization(proxy, "I_AM_NOT_A_NUMBER")); // invalid phone number

	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+990012345678"), "+990012345678");

	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0952636505"), "+33952636505");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "09 52 63 65 05"), "+33952636505");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "09-52-63-65-05"), "+33952636505");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+31952636505"), "+31952636505");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0033952636505"), "+33952636505");
	BC_ASSERT_PTR_NULL(phone_normalization(proxy, "toto"));

	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0"), "0"); // short numbers, France's dial plan
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "01"), "01");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "012"), "012");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0123"), "0123");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "01234"), "01234");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "012345"), "012345");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0123456"), "0123456");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "01234567"), "01234567");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "012345678"), "+33012345678"); // not short numbers, add prefix
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0123456789"), "+33123456789");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "01234567890"), "+33234567890");

	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "1"), "1"); // short numbers, France's dial plan
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "12"), "12");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "1 2"), "12");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "123"), "123");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "1234"), "1234");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "12345"), "12345");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "123456"), "123456");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "1234567"), "1234567");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "12345678"), "12345678");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "123456 78"), "12345678");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "123456789"), "+33123456789"); // not short numbers, add prefix
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "1234567890"), "+33234567890");

	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "003"), "003"); // short numbers, France's dial plan
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0033"), "0033");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "00331"), "00331");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "003312"), "003312");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0033123"), "0033123");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "00331234"), "00331234");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "003312345"),
	                       "003312345"); // not short numbers, but not long enough
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0033123456"), "0033123456");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "00331234567"), "00331234567");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "003312345678"), "003312345678");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0033123456789"), "+33123456789"); // valid french number

	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "001"), "001"); // short numbers, France's dial plan
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "00123456"), "00123456");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "00123456789"),
	                       "00123456789"); // not short numbers, but not long enough in American Samoa's dial plan
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "001234567890"), "001234567890");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0012345678902"),
	                       "+12345678902"); // valid american samoan number

	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+330"), "+330"); // start with "+", not short numbers
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

	// invalid prefix - use a generic dialplan with 10 max length and no min length
	linphone_proxy_config_set_dial_prefix(proxy, "99");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0011234567890"), "+11234567890");
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
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+5217227718184"),
	                       "+5217227718184"); /*this is a mobile phone number */
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+528127718184"),
	                       "+528127718184"); /*this is a landline phone number from Monterrey*/

	BC_ASSERT_EQUAL(linphone_dial_plan_lookup_ccc_from_e164("+522824713146"), 52, int,
	                "%i"); /*this is a landline phone number*/

	// Phone normalization for Myanmar dial plans
	linphone_proxy_config_set_dial_prefix(proxy, "95");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "9965066691"), "+959965066691");

	// Phone normalization for Cameroon dial plans
	linphone_proxy_config_set_dial_prefix(proxy, "237");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "674788175"), "+237674788175");

	// Phone normalization for Finland dial plan
	linphone_proxy_config_set_dial_prefix(proxy, "358");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+3584012345678"), "+3584012345678");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "112"), "112");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+6723 21234"), "+672321234");

	// Phone normalization for Norfolk Island dial plan
	linphone_proxy_config_set_dial_prefix(proxy, "672");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "672 3 21234"), "+672321234");

	// Phone normalization for Belgium dial plan, with 8 or 9 digits and no 0 after international prefix
	linphone_proxy_config_set_dial_prefix(proxy, "32");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0123456"), "0123456");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "01234567"), "+3201234567");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "012345678"), "+3212345678");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0123456789"), "+32123456789");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "003212345678"), "+3212345678");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0032123456789"), "+32123456789");

	linphone_proxy_config_unref(proxy);
}

static void phone_normalization_with_dial_escape_plus(void) {
	LinphoneProxyConfig *proxy = linphone_core_create_proxy_config(NULL);
	linphone_proxy_config_set_dial_prefix(proxy, "33");
	linphone_proxy_config_set_dial_escape_plus(proxy, TRUE);
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0033952636505"), "0033952636505");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0952636505"), "0033952636505");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+34952636505"), "0034952636505");

	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0"), "0"); // France's dial plan, short number
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "01"), "01");
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "012345678"), "0033012345678"); // not short number
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "0123456789"), "0033123456789");

	linphone_proxy_config_set_dial_escape_plus(proxy, FALSE);
	BC_ASSERT_STRING_EQUAL(phone_normalization(proxy, "+34952636505"), "+34952636505");

	linphone_proxy_config_unref(proxy);
}

#define SIP_URI_CHECK(actual, expected)                                                                                \
	{                                                                                                                  \
		LinphoneProxyConfig *proxy = linphone_core_create_proxy_config(NULL);                                          \
		LinphoneAddress *res;                                                                                          \
		char *actual_str;                                                                                              \
		LinphoneAddress *addr = linphone_address_new("sip:username@linphone.org");                                     \
		linphone_proxy_config_set_identity_address(proxy, addr);                                                       \
		if (addr) linphone_address_unref(addr);                                                                        \
		res = linphone_proxy_config_normalize_sip_uri(proxy, actual);                                                  \
		actual_str = linphone_address_as_string_uri_only(res);                                                         \
		BC_ASSERT_STRING_EQUAL(actual_str, expected);                                                                  \
		ms_free(actual_str);                                                                                           \
		linphone_address_unref(res);                                                                                   \
		linphone_proxy_config_unref(proxy);                                                                            \
	}

static void sip_uri_normalization(void) {
	char *expected = "sip:%d9%a1@linphone.org";
	BC_ASSERT_PTR_NULL(linphone_proxy_config_normalize_sip_uri(NULL, "test"));
	SIP_URI_CHECK("test@linphone.org", "sip:test@linphone.org");
	SIP_URI_CHECK("test@linphone.org;transport=tls", "sip:test@linphone.org;transport=tls");

	SIP_URI_CHECK("١", expected); // test that no more invalid memory writes are made (valgrind only)
}

static void load_dynamic_proxy_config(void) {
	LinphoneCoreManager *lauriane = linphone_core_manager_new("empty_rc");
	LinphoneProxyConfig *proxy;
	LinphoneAddress *read, *expected;
	LinphoneNatPolicy *nat_policy;
	const char *config =
	    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
	    "<config xmlns=\"http://www.linphone.org/xsds/lpconfig.xsd\" "
	    "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
	    "xsi:schemaLocation=\"http://www.linphone.org/xsds/lpconfig.xsd lpconfig.xsd\">\r\n"
	    "<section name=\"proxy_default_values\">\r\n"
	    "<entry name=\"avpf\" overwrite=\"true\">1</entry>\r\n"
	    "<entry name=\"dial_escape_plus\" overwrite=\"true\">0</entry>\r\n"
	    "<entry name=\"publish\" overwrite=\"true\">0</entry>\r\n"
	    "<entry name=\"quality_reporting_collector\" "
	    "overwrite=\"true\">sip:voip-metrics@sip.linphone.org;transport=tls</entry>\r\n"
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
	BC_ASSERT_FALSE(linphone_config_load_from_xml_string(linphone_core_get_config(lauriane->lc), config));
	proxy = linphone_core_create_proxy_config(lauriane->lc);

	read = linphone_address_new(linphone_proxy_config_get_server_addr(proxy));
	expected = linphone_address_new("sip:sip.linphone.org;transport=tls");

	BC_ASSERT_TRUE(linphone_address_equal(read, expected));
	linphone_address_unref(read);
	linphone_address_unref(expected);

	nat_policy = linphone_proxy_config_get_nat_policy(proxy);

	if (BC_ASSERT_PTR_NOT_NULL(nat_policy)) {
		BC_ASSERT_STRING_EQUAL(linphone_nat_policy_get_stun_server(nat_policy), "stun.linphone.org");
		BC_ASSERT_TRUE(linphone_nat_policy_ice_enabled(nat_policy));
		BC_ASSERT_TRUE(linphone_nat_policy_stun_enabled(nat_policy));
		BC_ASSERT_FALSE(linphone_nat_policy_turn_enabled(nat_policy));
	}
	linphone_proxy_config_unref(proxy);
	linphone_core_manager_destroy(lauriane);

	// BC_ASSERT_STRING_EQUAL(linphone_proxy_config_get(proxy), "sip:sip.linphone.org;transport=tls");
}

static void single_route(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneProxyConfig *marie_cfg = linphone_core_get_default_proxy_config(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_cfg);

	const bctbx_list_t *routes = linphone_proxy_config_get_routes(marie_cfg);
	BC_ASSERT_PTR_NOT_NULL(routes);
	BC_ASSERT_EQUAL((int)bctbx_list_size(routes), 1, int, "%d");
	const char *route = (const char *)bctbx_list_get_data(routes);
	const char *marie_route = linphone_proxy_config_get_route(marie_cfg);
	BC_ASSERT_STRING_EQUAL(marie_route, "<sip:sip.example.org;transport=tcp>");
	BC_ASSERT_STRING_EQUAL(route, "<sip:sip.example.org;transport=tcp>");
	routes = NULL;

	linphone_proxy_config_set_route(marie_cfg, "sip.linphone.org");
	routes = linphone_proxy_config_get_routes(marie_cfg);
	BC_ASSERT_PTR_NOT_NULL(routes);
	BC_ASSERT_EQUAL((int)bctbx_list_size(routes), 1, int, "%d");
	route = (const char *)bctbx_list_get_data(routes);
	marie_route = linphone_proxy_config_get_route(marie_cfg);
	BC_ASSERT_STRING_EQUAL(marie_route, "sip:sip.linphone.org");
	BC_ASSERT_STRING_EQUAL(route, "sip:sip.linphone.org");
	routes = NULL;

	linphone_core_manager_destroy(marie);
}

static void multiple_route(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneProxyConfig *marie_cfg = linphone_core_get_default_proxy_config(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_cfg);

	linphone_proxy_config_set_routes(marie_cfg, NULL); // Clear routes
	const bctbx_list_t *empty_routes = linphone_proxy_config_get_routes(marie_cfg);
	BC_ASSERT_EQUAL((int)bctbx_list_size(empty_routes), 0, int, "%d");
	empty_routes = NULL;

	bctbx_list_t *new_routes = NULL;
	new_routes = bctbx_list_append(new_routes, ms_strdup("<sip:sip.example.org;transport=tcp>"));
	new_routes = bctbx_list_append(new_routes, ms_strdup("sip:sip.linphone.org"));
	new_routes = bctbx_list_append(new_routes, ms_strdup("s:sip.linphone.org"));
	new_routes = bctbx_list_append(new_routes, ms_strdup("false_ero\nbad\\url::"));

	linphone_proxy_config_set_routes(marie_cfg, new_routes);

	bctbx_list_free_with_data(new_routes, bctbx_free);

	const bctbx_list_t *routes = linphone_proxy_config_get_routes(marie_cfg);
	BC_ASSERT_PTR_NOT_NULL(routes);
	BC_ASSERT_EQUAL((int)bctbx_list_size(routes), 2, int, "%d"); // 2 are good, 2 are bad

	const char *route = (const char *)bctbx_list_get_data(routes);
	const char *marie_route = linphone_proxy_config_get_route(marie_cfg);
	BC_ASSERT_STRING_EQUAL(marie_route, "<sip:sip.example.org;transport=tcp>");
	BC_ASSERT_STRING_EQUAL(route, "<sip:sip.example.org;transport=tcp>");
	route = (const char *)bctbx_list_get_data(bctbx_list_next(routes));
	BC_ASSERT_STRING_EQUAL(route, "sip:sip.linphone.org");
	routes = NULL;

	linphone_core_manager_destroy(marie);
}

static LinphoneAddress *get_raw_contact_address(LinphoneProxyConfig *cfg) {
	const char *raw_contact = linphone_proxy_config_get_custom_header(cfg, "Contact");
	if (!raw_contact) {
		return NULL;
	}
	return linphone_address_new(raw_contact);
}

/*
 * Dependent proxy config scenario: pauline@example.org, marie@example.org and marie@sip2.linphone.org
 * marie@sip2.linphone.org is marked 'Dependent' on marie@example.org.
 * Once all registered, we cut the marie@sip2.linphone.org connection.
 * A call from pauline@example.org to marie@sip2.linphone.org should work (and go through example.org instead of
 * sip2.linphone.org)
 */
static void dependent_proxy_config(void) {
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_external_rc");
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_dependent_proxy_rc");
	LinphoneProxyConfig *marie_cfg = linphone_core_get_default_proxy_config(marie->lc);
	LinphoneProxyConfig *marie_dependent_cfg =
	    (LinphoneProxyConfig *)linphone_core_get_proxy_config_list(marie->lc)->next->data;
	BC_ASSERT_PTR_NOT_NULL(marie_cfg);
	BC_ASSERT_PTR_NOT_NULL(marie_dependent_cfg);

	BC_ASSERT_PTR_EQUAL(marie_cfg, linphone_proxy_config_get_dependency(marie_dependent_cfg));

	BC_ASSERT_TRUE(wait_for(pauline->lc, NULL, &pauline->stat.number_of_LinphoneRegistrationOk, 1));

	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 2));

	const LinphoneAddress *marie_cfg_contact = linphone_proxy_config_get_contact(marie_cfg);
	LinphoneAddress *marie_dependent_cfg_contact = get_raw_contact_address(marie_dependent_cfg);
	if (BC_ASSERT_PTR_NOT_NULL(marie_dependent_cfg_contact)) {
		if (!BC_ASSERT_TRUE(linphone_proxy_config_address_equal(marie_cfg_contact, marie_dependent_cfg_contact) ==
		                    LinphoneProxyConfigAddressEqual)) {
			char *tmp1, *tmp2;
			tmp1 = linphone_address_as_string(marie_cfg_contact);
			tmp2 = linphone_address_as_string(marie_dependent_cfg_contact);
			bctbx_error("Contact addresses do differ: %s <> %s", tmp1, tmp2);
			bctbx_free(tmp1), bctbx_free(tmp2);
		}
		linphone_address_unref(marie_dependent_cfg_contact);
	}

	// Cut link for dependent proxy config, then call its identity address and check that we receive the call
	//(which would be received through the 'master' proxy config server)
	linphone_core_set_network_reachable(marie->lc, FALSE);
	linphone_proxy_config_edit(marie_dependent_cfg);
	linphone_proxy_config_enable_register(marie_dependent_cfg, FALSE);
	linphone_proxy_config_done(marie_dependent_cfg);
	linphone_core_set_network_reachable(marie->lc, TRUE);

	wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneRegistrationOk,
	         3); // One more time for 'master' proxy config

	LinphoneAddress *marie_dependent_addr =
	    linphone_address_new(linphone_proxy_config_get_identity(marie_dependent_cfg));

	linphone_core_invite_address(pauline->lc, marie_dependent_addr);

	if (BC_ASSERT_TRUE(
	        wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallIncomingReceived, 1, 10000))) {
		linphone_call_accept(linphone_core_get_current_call(marie->lc));
		BC_ASSERT_TRUE(
		    wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));
		BC_ASSERT_TRUE(
		    wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));
		end_call(pauline, marie);
	}

	linphone_address_unref(marie_dependent_addr);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

// Dependent proxy config should not register if its dependency is not in a LinphoneRegistrationOk state
static void proxy_config_dependent_register(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_dependent_proxy_rc");
	const bctbx_list_t *proxyConfigs = linphone_core_get_proxy_config_list(marie->lc);
	LinphoneProxyConfig *master = (LinphoneProxyConfig *)proxyConfigs->data;

	linphone_proxy_config_edit(master);
	linphone_proxy_config_set_server_addr(master, "sip:cannotberesol.ved");
	linphone_proxy_config_done(master);

	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 0));

	linphone_core_manager_destroy(marie);
}

// Dependent proxy config should	mirror the state of its 'parent'
static void proxy_config_dependent_register_state_changed(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_dependent_proxy_rc");
	const bctbx_list_t *proxyConfigs = linphone_core_get_proxy_config_list(marie->lc);
	LinphoneProxyConfig *master = (LinphoneProxyConfig *)proxyConfigs->data;

	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 2));

	linphone_proxy_config_edit(master);
	char *keep_server_addr = bctbx_strdup(linphone_proxy_config_get_server_addr(master));
	linphone_proxy_config_set_server_addr(master, "sip:cannotberesol.ved");
	linphone_proxy_config_done(master);

	// Both configs should now be in 'LinphoneRegistrationFailed' state
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationFailed, 2));

	marie->stat.number_of_LinphoneRegistrationOk = 0;
	linphone_proxy_config_edit(master);
	linphone_proxy_config_set_server_addr(master, keep_server_addr);
	linphone_proxy_config_done(master);

	// Ok again.
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 2));

	linphone_proxy_config_edit(master);
	linphone_proxy_config_enable_register(master, FALSE);
	linphone_proxy_config_done(master);

	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationCleared, 2));

	bctbx_free(keep_server_addr);
	linphone_core_manager_destroy(marie);
}

// A dependent proxy config should behave as a normal one after removal of dependency
static void dependent_proxy_dependency_removal(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_dependent_proxy_rc");
	const bctbx_list_t *proxyConfigs = linphone_core_get_proxy_config_list(marie->lc);
	LinphoneProxyConfig *master = (LinphoneProxyConfig *)proxyConfigs->data;

	linphone_core_manager_start(marie, FALSE);
	linphone_core_set_network_reachable(marie->lc, FALSE);

	linphone_core_remove_proxy_config(marie->lc, master);
	const bctbx_list_t *accounts = linphone_core_get_account_list(marie->lc);
	BC_ASSERT_EQUAL(bctbx_list_size(accounts), 1, size_t, "%zu");

	proxyConfigs = linphone_core_get_proxy_config_list(marie->lc);
	BC_ASSERT_EQUAL(bctbx_list_size(proxyConfigs), 1, size_t, "%zu");

	linphone_core_set_network_reachable(marie->lc, TRUE);

	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 1));

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRegistrationOk, 1, int, "%d");

	linphone_core_manager_destroy(marie);
}

static void dependent_proxy_dependency_with_core_reloaded(void) {
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_external_rc");
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_dependent_proxy_rc");
	LinphoneProxyConfig *marie_cfg = linphone_core_get_default_proxy_config(marie->lc);
	LinphoneProxyConfig *marie_dependent_cfg =
	    (LinphoneProxyConfig *)linphone_core_get_proxy_config_list(marie->lc)->next->data;
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
	linphone_address_unref(marie_secondary_address);

	marie_cfg = linphone_core_create_proxy_config(marie->lc);
	linphone_proxy_config_set_identity_address(marie_cfg, marie_master_address);
	linphone_proxy_config_set_server_addr(marie_cfg, "sip:sipopen.example.org;transport=tls");
	linphone_proxy_config_enable_register(marie_cfg, TRUE);
	linphone_address_unref(marie_master_address);

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
	LinphoneAddress *marie_dependent_cfg_contact = get_raw_contact_address(marie_dependent_cfg);

	linphone_proxy_config_unref(marie_cfg);
	linphone_proxy_config_unref(marie_dependent_cfg);

	if (BC_ASSERT_PTR_NOT_NULL(marie_dependent_cfg_contact)) {
		BC_ASSERT_TRUE(linphone_proxy_config_address_equal(marie_cfg_contact, marie_dependent_cfg_contact) ==
		               LinphoneProxyConfigAddressEqual);
		linphone_address_unref(marie_dependent_cfg_contact);
	}

	// Cut link for dependent proxy config, then call its identity address and check that we receive the call
	//(which would be received through the 'master' proxy config server)
	linphone_core_set_network_reachable(marie->lc, FALSE);
	linphone_proxy_config_edit(marie_dependent_cfg);
	linphone_proxy_config_enable_register(marie_dependent_cfg, FALSE);
	linphone_proxy_config_done(marie_dependent_cfg);
	linphone_core_set_network_reachable(marie->lc, TRUE);

	wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneRegistrationOk,
	         5); // One more time for 'master' proxy config

	LinphoneAddress *marie_dependent_addr =
	    linphone_address_new(linphone_proxy_config_get_identity(marie_dependent_cfg));

	linphone_core_invite_address(pauline->lc, marie_dependent_addr);

	if (BC_ASSERT_TRUE(
	        wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallIncomingReceived, 1, 10000))) {
		linphone_call_accept(linphone_core_get_current_call(marie->lc));
		BC_ASSERT_TRUE(
		    wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));
		BC_ASSERT_TRUE(
		    wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));
		end_call(pauline, marie);
	}
	linphone_address_unref(marie_dependent_addr);

	/* re-enable marie's registration on dependent proxy config*/
	linphone_proxy_config_edit(marie_dependent_cfg);
	linphone_proxy_config_enable_register(marie_dependent_cfg, TRUE);
	linphone_proxy_config_done(marie_dependent_cfg);

	wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneRegistrationOk, 6);

	/* Now marie's core stops and restarts. */
	ms_message("%s stops and restarts its core", linphone_core_get_identity(marie->lc));
	linphone_core_stop(marie->lc);
	linphone_core_start(marie->lc);
	linphone_core_manager_setup_dns(marie);

	/* Check that configuration could be reloaded correctly */
	BC_ASSERT_EQUAL((int)bctbx_list_size(linphone_core_get_proxy_config_list(marie->lc)), 2, int, "%i");

	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 8));

	/* And make new call attempt */
	marie_cfg = (LinphoneProxyConfig *)linphone_core_get_proxy_config_list(marie->lc)->next->data;
	marie_dependent_cfg = (LinphoneProxyConfig *)linphone_core_get_proxy_config_list(marie->lc)->data;
	BC_ASSERT_PTR_EQUAL(marie_cfg, linphone_proxy_config_get_dependency(marie_dependent_cfg));

	marie_cfg_contact = linphone_proxy_config_get_contact(marie_cfg);
	marie_dependent_cfg_contact = get_raw_contact_address(marie_dependent_cfg);

	if (BC_ASSERT_PTR_NOT_NULL(marie_dependent_cfg_contact)) {
		BC_ASSERT_TRUE(linphone_proxy_config_address_equal(marie_cfg_contact, marie_dependent_cfg_contact) ==
		               LinphoneProxyConfigAddressEqual);
		linphone_address_unref(marie_dependent_cfg_contact);
	}

	// Cut link for dependent proxy config, then call its identity address and check that we receive the call
	//(which would be received through the 'master' proxy config server)
	linphone_core_set_network_reachable(marie->lc, FALSE);
	linphone_proxy_config_edit(marie_dependent_cfg);
	linphone_proxy_config_enable_register(marie_dependent_cfg, FALSE);
	linphone_proxy_config_done(marie_dependent_cfg);
	linphone_core_set_network_reachable(marie->lc, TRUE);

	wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneRegistrationOk,
	         9); // One more time for 'master' proxy config

	marie_dependent_addr = linphone_address_new(linphone_proxy_config_get_identity(marie_dependent_cfg));

	linphone_core_invite_address(pauline->lc, marie_dependent_addr);

	if (BC_ASSERT_TRUE(
	        wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallIncomingReceived, 2, 10000))) {
		linphone_call_accept(linphone_core_get_current_call(marie->lc));
		BC_ASSERT_TRUE(
		    wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
		BC_ASSERT_TRUE(
		    wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
		end_call(pauline, marie);
	}

	linphone_address_unref(marie_dependent_addr);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

extern void linphone_core_update_push_notification_information(LinphoneCore *core, const char *param, const char *prid);

static void proxy_config_push_notification_scenario_1(bool_t multi_config, bool_t both_push) {
	const char *rc = "marie_rc";
	if (multi_config) rc = "marie_dual_proxy_rc";

	LinphoneCoreManager *marie = linphone_core_manager_new(rc);
	LinphoneProxyConfig *marie_cfg = linphone_core_get_default_proxy_config(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_cfg);
	const bctbx_list_t *proxies = linphone_core_get_proxy_config_list(marie->lc);
	int proxy_config_count = (int)bctbx_list_size(proxies);
	LinphoneProxyConfig *marie_cfg_2 = NULL;

	if (multi_config) {
		BC_ASSERT_EQUAL(proxy_config_count, 2, int, "%d");
		marie_cfg_2 = (LinphoneProxyConfig *)proxies->next->data;
		BC_ASSERT_PTR_NOT_NULL(marie_cfg_2);
	} else {
		BC_ASSERT_EQUAL(proxy_config_count, 1, int, "%d");
		BC_ASSERT_PTR_NULL(marie_cfg_2);
	}

	BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg));
	BC_ASSERT_PTR_NULL(
	    linphone_push_notification_config_get_prid(linphone_proxy_config_get_push_notification_config(marie_cfg)));
	BC_ASSERT_PTR_NULL(
	    linphone_push_notification_config_get_param(linphone_proxy_config_get_push_notification_config(marie_cfg)));
	BC_ASSERT_PTR_NULL(
	    linphone_config_get_string(linphone_core_get_config(marie->lc), "proxy_0", "push_parameters", NULL));

#if __ANDROID__ || TARGET_OS_IPHONE
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
	BC_ASSERT_TRUE(linphone_core_is_push_notification_enabled(marie->lc));
#else
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
	BC_ASSERT_FALSE(linphone_core_is_push_notification_enabled(marie->lc));
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_available(marie_cfg));
#endif
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count, int, "%i");
	if (multi_config) {
		BC_ASSERT_PTR_NULL(
		    linphone_config_get_string(linphone_core_get_config(marie->lc), "proxy_1", "push_parameters", NULL));
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
#if __ANDROID__ || TARGET_OS_IPHONE
		BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#else
		BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#endif
	}

	// First: enable push
	linphone_core_set_push_notification_enabled(marie->lc, TRUE);
	// Nothing should happen, push information aren't set
	BC_ASSERT_TRUE(linphone_core_is_push_notification_enabled(marie->lc));
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_available(marie_cfg));
	BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg));
#if __ANDROID__ || TARGET_OS_IPHONE
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
#else
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
#endif
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count, int, "%i");
	if (multi_config) {
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
#if __ANDROID__ || TARGET_OS_IPHONE
		BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#else
		BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#endif
	}

	// Second: configure push informations
	linphone_core_update_push_notification_information(marie->lc, "test-app-id", "test-push-token");
	BC_ASSERT_PTR_NOT_NULL(
	    linphone_push_notification_config_get_prid(linphone_proxy_config_get_push_notification_config(marie_cfg)));
	BC_ASSERT_PTR_NOT_NULL(
	    linphone_push_notification_config_get_param(linphone_proxy_config_get_push_notification_config(marie_cfg)));
	// Push auto set fot android and ios. Otherwise nothing should happen, push aren't allowed on proxy config
#if __ANDROID__ || TARGET_OS_IPHONE
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
#else
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
#endif
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_available(marie_cfg));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count, int, "%i");
	if (multi_config) {
#if __ANDROID__ || TARGET_OS_IPHONE
		BC_ASSERT_PTR_NOT_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
		BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#else
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
		BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#endif
	}

	// Third: allow push notification on proxy config
	linphone_proxy_config_edit(marie_cfg);
	linphone_proxy_config_set_push_notification_allowed(marie_cfg, TRUE);
	linphone_proxy_config_done(marie_cfg);

	if (both_push) {
		BC_ASSERT_PTR_NULL(
		    linphone_config_get_string(linphone_core_get_config(marie->lc), "proxy_1", "push_parameters", NULL));
		linphone_proxy_config_edit(marie_cfg_2);
		linphone_proxy_config_set_push_notification_allowed(marie_cfg_2, TRUE);
		linphone_proxy_config_done(marie_cfg_2);
		BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
		proxy_config_count++;
	}

	BC_ASSERT_TRUE(
	    wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count + 1, 10000));

	const char *savedPushParameters =
	    linphone_config_get_string(linphone_core_get_config(marie->lc), "proxy_0", "push_parameters", NULL);
	BC_ASSERT_PTR_NOT_NULL(savedPushParameters);

#ifdef __ANDROID__
	char *expectedSavedPushParameters =
	    "pn-prid=test-push-token;pn-provider=fcm;pn-param=test-app-id;pn-silent=1;pn-timeout=0;";
#elif TARGET_OS_IPHONE
	char *expectedSavedPushParameters =
	    "pn-prid=test-push-token;pn-provider=apns.dev;pn-param=test-app-id;pn-silent=1;pn-timeout=0;";
#else
	char *expectedSavedPushParameters =
	    "pn-prid=test-push-token;pn-provider=liblinphone_tester;pn-param=test-app-id;pn-silent=1;pn-timeout=0;";
#endif

	BC_ASSERT_STRING_EQUAL(savedPushParameters, expectedSavedPushParameters);
	if (both_push) {
		BC_ASSERT_PTR_NOT_NULL(
		    linphone_config_get_string(linphone_core_get_config(marie->lc), "proxy_1", "push_parameters", NULL));
	} else if (multi_config) {
#if __ANDROID__ || TARGET_OS_IPHONE
		BC_ASSERT_PTR_NOT_NULL(
		    linphone_config_get_string(linphone_core_get_config(marie->lc), "proxy_1", "push_parameters", NULL));
		BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#else
		BC_ASSERT_PTR_NULL(
		    linphone_config_get_string(linphone_core_get_config(marie->lc), "proxy_1", "push_parameters", NULL));
		BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#endif
	}

	// Fourth: disallow push notification on proxy config
	linphone_proxy_config_edit(marie_cfg);
	linphone_proxy_config_set_push_notification_allowed(marie_cfg, FALSE);
	linphone_proxy_config_done(marie_cfg);
	if (both_push) {
		linphone_proxy_config_edit(marie_cfg_2);
		linphone_proxy_config_set_push_notification_allowed(marie_cfg_2, FALSE);
		linphone_proxy_config_done(marie_cfg_2);
		proxy_config_count++;
	}
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
	BC_ASSERT_TRUE(linphone_core_is_push_notification_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_available(marie_cfg));
	BC_ASSERT_TRUE(
	    wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count + 2, 10000));
	BC_ASSERT_PTR_NULL(
	    linphone_config_get_string(linphone_core_get_config(marie->lc), "proxy_0", "push_parameters", NULL));
	if (multi_config) {
		if (!both_push) {
#if __ANDROID__ || TARGET_OS_IPHONE
			BC_ASSERT_PTR_NOT_NULL(
			    linphone_config_get_string(linphone_core_get_config(marie->lc), "proxy_1", "push_parameters", NULL));
			BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#else
			BC_ASSERT_PTR_NULL(
			    linphone_config_get_string(linphone_core_get_config(marie->lc), "proxy_1", "push_parameters", NULL));
			BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#endif
		} else {
			BC_ASSERT_PTR_NULL(
			    linphone_config_get_string(linphone_core_get_config(marie->lc), "proxy_1", "push_parameters", NULL));
			BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
		}
	}

	linphone_core_manager_destroy(marie);
}

static void proxy_config_push_notification_scenario_2(bool_t multi_config, bool_t both_push) {
	const char *rc = "marie_rc";
	if (multi_config) rc = "marie_dual_proxy_rc";

	LinphoneCoreManager *marie = linphone_core_manager_new(rc);
	LinphoneProxyConfig *marie_cfg = linphone_core_get_default_proxy_config(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_cfg);
	const bctbx_list_t *proxies = linphone_core_get_proxy_config_list(marie->lc);
	int proxy_config_count = (int)bctbx_list_size(proxies);
	LinphoneProxyConfig *marie_cfg_2 = NULL;

	if (multi_config) {
		BC_ASSERT_EQUAL(proxy_config_count, 2, int, "%d");
		marie_cfg_2 = (LinphoneProxyConfig *)proxies->next->data;
		BC_ASSERT_PTR_NOT_NULL(marie_cfg_2);
	} else {
		BC_ASSERT_EQUAL(proxy_config_count, 1, int, "%d");
		BC_ASSERT_PTR_NULL(marie_cfg_2);
	}

	BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg));
#if __ANDROID__ || TARGET_OS_IPHONE
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
	BC_ASSERT_TRUE(linphone_core_is_push_notification_enabled(marie->lc));
#else
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
	BC_ASSERT_FALSE(linphone_core_is_push_notification_enabled(marie->lc));
#endif
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_available(marie_cfg));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count, int, "%i");
	if (multi_config) {
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
#if __ANDROID__ || TARGET_OS_IPHONE
		BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#else
		BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#endif
	}

	// First: enable push
	linphone_core_set_push_notification_enabled(marie->lc, TRUE);
	// Nothing should happen, push information aren't set
	BC_ASSERT_TRUE(linphone_core_is_push_notification_enabled(marie->lc));
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_available(marie_cfg));
	BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg));
#if __ANDROID__ || TARGET_OS_IPHONE
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
#else
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
#endif
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count, int, "%i");
	if (multi_config) {
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
#if __ANDROID__ || TARGET_OS_IPHONE
		BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#else
		BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#endif
	}

	// Second: allow push notification on proxy config
	linphone_proxy_config_edit(marie_cfg);
	linphone_proxy_config_set_push_notification_allowed(marie_cfg, TRUE);
	linphone_proxy_config_done(marie_cfg);
	if (both_push) {
		linphone_proxy_config_edit(marie_cfg_2);
		linphone_proxy_config_set_push_notification_allowed(marie_cfg_2, TRUE);
		linphone_proxy_config_done(marie_cfg_2);
		BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
	}
	// Nothing should happen, push informations aren't supplied
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_available(marie_cfg));
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
	BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count, int, "%i");
	if (multi_config) {
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
	}

	// Third: configure push informations
	linphone_core_update_push_notification_information(marie->lc, "test-app-id", "test-push-token");
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_available(marie_cfg));
	if (both_push) proxy_config_count++;

	BC_ASSERT_TRUE(
	    wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count + 1, 10000));

	const char *savedPushParameters =
	    linphone_config_get_string(linphone_core_get_config(marie->lc), "proxy_0", "push_parameters", NULL);
	BC_ASSERT_PTR_NOT_NULL(savedPushParameters);

#ifdef __ANDROID__
	char *expectedSavedPushParameters =
	    "pn-prid=test-push-token;pn-provider=fcm;pn-param=test-app-id;pn-silent=1;pn-timeout=0;";
#elif TARGET_OS_IPHONE
	char *expectedSavedPushParameters =
	    "pn-prid=test-push-token;pn-provider=apns.dev;pn-param=test-app-id;pn-silent=1;pn-timeout=0;";
#else
	char *expectedSavedPushParameters =
	    "pn-prid=test-push-token;pn-provider=liblinphone_tester;pn-param=test-app-id;pn-silent=1;pn-timeout=0;";
#endif

	BC_ASSERT_STRING_EQUAL(savedPushParameters, expectedSavedPushParameters);
	if (both_push) {
		BC_ASSERT_PTR_NOT_NULL(
		    linphone_config_get_string(linphone_core_get_config(marie->lc), "proxy_1", "push_parameters", NULL));
	} else if (multi_config) {
#if __ANDROID__ || TARGET_OS_IPHONE
		BC_ASSERT_PTR_NOT_NULL(
		    linphone_config_get_string(linphone_core_get_config(marie->lc), "proxy_1", "push_parameters", NULL));
		BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#else
		BC_ASSERT_PTR_NULL(
		    linphone_config_get_string(linphone_core_get_config(marie->lc), "proxy_1", "push_parameters", NULL));
		BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#endif
	}

	// Fourth: disable push notification on core
	linphone_core_set_push_notification_enabled(marie->lc, FALSE);
	BC_ASSERT_FALSE(linphone_core_is_push_notification_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_available(marie_cfg));
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
	BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg));
	BC_ASSERT_TRUE(
	    wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count + 2, 10000));
	if (multi_config) {
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
		if (both_push) BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
		else
#if __ANDROID__ || TARGET_OS_IPHONE
			BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#else
			BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#endif
	}

	linphone_core_manager_destroy(marie);
}

static void proxy_config_push_notification_scenario_3(bool_t multi_config, bool_t both_push) {
	const char *rc = "marie_rc";
	if (multi_config) rc = "marie_dual_proxy_rc";

	LinphoneCoreManager *marie = linphone_core_manager_new(rc);
	LinphoneProxyConfig *marie_cfg = linphone_core_get_default_proxy_config(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_cfg);
	const bctbx_list_t *proxies = linphone_core_get_proxy_config_list(marie->lc);
	int proxy_config_count = (int)bctbx_list_size(proxies);
	LinphoneProxyConfig *marie_cfg_2 = NULL;

	if (multi_config) {
		BC_ASSERT_EQUAL(proxy_config_count, 2, int, "%d");
		marie_cfg_2 = (LinphoneProxyConfig *)proxies->next->data;
		BC_ASSERT_PTR_NOT_NULL(marie_cfg_2);
	} else {
		BC_ASSERT_EQUAL(proxy_config_count, 1, int, "%d");
		BC_ASSERT_PTR_NULL(marie_cfg_2);
	}

	BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg));
#if __ANDROID__ || TARGET_OS_IPHONE
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
#else
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
#endif
	if (multi_config) {
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
#if __ANDROID__ || TARGET_OS_IPHONE
		BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
	}
	BC_ASSERT_TRUE(linphone_core_is_push_notification_enabled(marie->lc));
#else
		BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
	}
	BC_ASSERT_FALSE(linphone_core_is_push_notification_enabled(marie->lc));
#endif
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_available(marie_cfg));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count, int, "%i");

	// First: allow push notification on proxy config
	linphone_proxy_config_edit(marie_cfg);
	linphone_proxy_config_set_push_notification_allowed(marie_cfg, TRUE);
	linphone_proxy_config_done(marie_cfg);
	if (both_push) {
		linphone_proxy_config_edit(marie_cfg_2);
		linphone_proxy_config_set_push_notification_allowed(marie_cfg_2, TRUE);
		linphone_proxy_config_done(marie_cfg_2);
		BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
	}
	// Nothing should happen, push aren't enabled at core level
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
#if __ANDROID__ || TARGET_OS_IPHONE
	BC_ASSERT_TRUE(linphone_core_is_push_notification_enabled(marie->lc));
#else
	BC_ASSERT_FALSE(linphone_core_is_push_notification_enabled(marie->lc));
#endif
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_available(marie_cfg));
	BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count, int, "%i");
	if (multi_config) {
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
		if (both_push) BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
		else
#if __ANDROID__ || TARGET_OS_IPHONE
			BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#else
			BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#endif
	}

	// Second: enable push
	linphone_core_set_push_notification_enabled(marie->lc, TRUE);
	// Nothing should happen, push informations aren't supplied
	BC_ASSERT_TRUE(linphone_core_is_push_notification_enabled(marie->lc));
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_available(marie_cfg));
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
	BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count, int, "%i");
	if (multi_config) {
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
		if (both_push) BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
		else
#if __ANDROID__ || TARGET_OS_IPHONE
			BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#else
			BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#endif
	}

	// Third: configure push informations
	linphone_core_update_push_notification_information(marie->lc, "test-app-id", "test-push-token");
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_available(marie_cfg));

	if (both_push) proxy_config_count++;

	BC_ASSERT_TRUE(
	    wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count + 1, 10000));

	const char *savedPushParameters =
	    linphone_config_get_string(linphone_core_get_config(marie->lc), "proxy_0", "push_parameters", NULL);
	BC_ASSERT_PTR_NOT_NULL(savedPushParameters);

#ifdef __ANDROID__
	char *expectedSavedPushParameters =
	    "pn-prid=test-push-token;pn-provider=fcm;pn-param=test-app-id;pn-silent=1;pn-timeout=0;";
#elif TARGET_OS_IPHONE
	char *expectedSavedPushParameters =
	    "pn-prid=test-push-token;pn-provider=apns.dev;pn-param=test-app-id;pn-silent=1;pn-timeout=0;";
#else
		char *expectedSavedPushParameters =
		    "pn-prid=test-push-token;pn-provider=liblinphone_tester;pn-param=test-app-id;pn-silent=1;pn-timeout=0;";
#endif

	BC_ASSERT_STRING_EQUAL(savedPushParameters, expectedSavedPushParameters);
	if (both_push) {
		BC_ASSERT_PTR_NOT_NULL(
		    linphone_config_get_string(linphone_core_get_config(marie->lc), "proxy_1", "push_parameters", NULL));
	} else if (multi_config) {
#if __ANDROID__ || TARGET_OS_IPHONE
		BC_ASSERT_PTR_NOT_NULL(
		    linphone_config_get_string(linphone_core_get_config(marie->lc), "proxy_1", "push_parameters", NULL));
		BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#else
		BC_ASSERT_PTR_NULL(
		    linphone_config_get_string(linphone_core_get_config(marie->lc), "proxy_1", "push_parameters", NULL));
		BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#endif
	}

	// Fourth: update push information with null
	linphone_core_update_push_notification_information(marie->lc, NULL, NULL);
	BC_ASSERT_TRUE(linphone_core_is_push_notification_enabled(marie->lc));
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_available(marie_cfg));
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
	BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg));
	BC_ASSERT_TRUE(
	    wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count + 2, 10000));
	if (multi_config) {
		BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg_2));
		if (both_push) BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
		else
#if __ANDROID__ || TARGET_OS_IPHONE
			BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#else
			BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg_2));
#endif
	}

	linphone_core_manager_destroy(marie);
}

static void proxy_config_push_notification_params(void) {
	proxy_config_push_notification_scenario_1(FALSE, FALSE);
}

static void proxy_config_push_notification_params_2(void) {
	proxy_config_push_notification_scenario_2(FALSE, FALSE);
}

static void proxy_config_push_notification_params_3(void) {
	proxy_config_push_notification_scenario_3(FALSE, FALSE);
}

static void proxy_config_push_notification_params_two_proxies_one_push(void) {
	proxy_config_push_notification_scenario_1(TRUE, FALSE);
}

static void proxy_config_push_notification_params_two_proxies_one_push_2(void) {
	proxy_config_push_notification_scenario_2(TRUE, FALSE);
}

static void proxy_config_push_notification_params_two_proxies_one_push_3(void) {
	proxy_config_push_notification_scenario_3(TRUE, FALSE);
}

static void proxy_config_push_notification_params_two_proxies_both_push(void) {
	proxy_config_push_notification_scenario_1(TRUE, TRUE);
}

static void proxy_config_push_notification_params_two_proxies_both_push_2(void) {
	proxy_config_push_notification_scenario_2(TRUE, TRUE);
}

static void proxy_config_push_notification_params_two_proxies_both_push_3(void) {
	proxy_config_push_notification_scenario_3(TRUE, TRUE);
}

static void proxy_config_push_notification_core_restart(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneProxyConfig *marie_cfg = linphone_core_get_default_proxy_config(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_cfg);
	int proxy_config_count = (int)bctbx_list_size(linphone_core_get_proxy_config_list(marie->lc));
	BC_ASSERT_EQUAL(proxy_config_count, 1, size_t, "%zu");

#if __ANDROID__ || TARGET_OS_IPHONE
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
#else
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
#endif
	BC_ASSERT_FALSE(linphone_core_is_push_notification_enabled(marie->lc));
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_available(marie_cfg));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count, int, "%i");

	// First: allow push notification on proxy config
	linphone_proxy_config_edit(marie_cfg);
	linphone_proxy_config_set_push_notification_allowed(marie_cfg, TRUE);
	linphone_proxy_config_done(marie_cfg);

	// Second: enable push
	linphone_core_set_push_notification_enabled(marie->lc, TRUE);
	// Nothing should happen, push informations aren't supplied
	BC_ASSERT_TRUE(linphone_core_is_push_notification_enabled(marie->lc));
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_available(marie_cfg));
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
	BC_ASSERT_PTR_NULL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count, int, "%i");

	// Third: configure push informations
	linphone_core_update_push_notification_information(marie->lc, "test-app-id", "test-push-token");
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_available(marie_cfg));

	BC_ASSERT_TRUE(
	    wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count + 1, 10000));

	const char *savedPushParameters =
	    linphone_config_get_string(linphone_core_get_config(marie->lc), "proxy_0", "push_parameters", NULL);
	BC_ASSERT_PTR_NOT_NULL(savedPushParameters);

#ifdef __ANDROID__
	const char *expectedSavedPushParameters =
	    "pn-prid=test-push-token;pn-provider=fcm;pn-param=test-app-id;pn-silent=1;pn-timeout=0;";
#elif TARGET_OS_IPHONE
	const char *expectedSavedPushParameters =
	    "pn-prid=test-push-token;pn-provider=apns.dev;pn-param=test-app-id;pn-silent=1;pn-timeout=0;";
#else
		const char *expectedSavedPushParameters =
		    "pn-prid=test-push-token;pn-provider=liblinphone_tester;pn-param=test-app-id;pn-silent=1;pn-timeout=0;";
#endif

	BC_ASSERT_STRING_EQUAL(savedPushParameters, expectedSavedPushParameters);

	linphone_core_stop(marie->lc);
	BC_ASSERT_EQUAL(linphone_core_get_global_state(marie->lc), LinphoneGlobalOff, int, "%i");
	proxy_config_count = (int)bctbx_list_size(linphone_core_get_proxy_config_list(marie->lc));
	BC_ASSERT_EQUAL(proxy_config_count, 1, size_t, "%zu");
	linphone_core_start(marie->lc);
	linphone_core_manager_setup_dns(marie);
	BC_ASSERT_EQUAL(linphone_core_get_global_state(marie->lc), LinphoneGlobalOn, int, "%i");
	BC_ASSERT_TRUE(
	    wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count + 2, 10000));
	savedPushParameters =
	    linphone_config_get_string(linphone_core_get_config(marie->lc), "proxy_0", "push_parameters", NULL);
	BC_ASSERT_TRUE(linphone_core_is_push_notification_enabled(marie->lc));
	BC_ASSERT_STRING_EQUAL(savedPushParameters, expectedSavedPushParameters);

	linphone_core_manager_destroy(marie);
}

static void proxy_config_remove_push_info_from_contact_uri_parameters(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneProxyConfig *marie_cfg = linphone_core_get_default_proxy_config(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_cfg);
	int proxy_config_count = (int)bctbx_list_size(linphone_core_get_proxy_config_list(marie->lc));
	BC_ASSERT_EQUAL(proxy_config_count, 1, int, "%d");

#if __ANDROID__ || TARGET_OS_IPHONE
	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
#else
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_allowed(marie_cfg));
#endif
	BC_ASSERT_FALSE(linphone_core_is_push_notification_enabled(marie->lc));
	BC_ASSERT_FALSE(linphone_proxy_config_is_push_notification_available(marie_cfg));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count, int, "%i");

	linphone_proxy_config_edit(marie_cfg);
	linphone_proxy_config_set_push_notification_allowed(marie_cfg, TRUE);
	linphone_proxy_config_set_contact_uri_parameters(marie_cfg,
	                                                 "pn-param=other-id;pn-prid=other-token;otherparam=othervalue;");
	BC_ASSERT_STRING_EQUAL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg),
	                       "pn-param=other-id;pn-prid=other-token;otherparam=othervalue;");
	linphone_proxy_config_done(marie_cfg);

	// Second: enable push
	linphone_core_set_push_notification_enabled(marie->lc, TRUE);

	// Third: configure push informations
	linphone_core_update_push_notification_information(marie->lc, "test-app-id", "test-push-token");

	BC_ASSERT_TRUE(linphone_proxy_config_is_push_notification_available(marie_cfg));

	BC_ASSERT_TRUE(
	    wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, proxy_config_count + 1, 10000));

	const char *savedPushParameters =
	    linphone_config_get_string(linphone_core_get_config(marie->lc), "proxy_0", "push_parameters", NULL);
	BC_ASSERT_PTR_NOT_NULL(savedPushParameters);

#ifdef __ANDROID__
	const char *expectedSavedPushParameters =
	    "pn-prid=test-push-token;pn-provider=fcm;pn-param=test-app-id;pn-silent=1;pn-timeout=0;";
#elif TARGET_OS_IPHONE
	const char *expectedSavedPushParameters =
	    "pn-prid=test-push-token;pn-provider=apns.dev;pn-param=test-app-id;pn-silent=1;pn-timeout=0;";
#else
		const char *expectedSavedPushParameters =
		    "pn-prid=test-push-token;pn-provider=liblinphone_tester;pn-param=test-app-id;pn-silent=1;pn-timeout=0;";
#endif

	BC_ASSERT_STRING_EQUAL(savedPushParameters, expectedSavedPushParameters);
	// Check that pn-param and pn-prid have been removed from contact uri parameters
	BC_ASSERT_STRING_EQUAL(linphone_proxy_config_get_contact_uri_parameters(marie_cfg), "otherparam=othervalue;");

	linphone_core_manager_destroy(marie);
}

test_t proxy_config_tests[] = {
    TEST_NO_TAG("Phone normalization without proxy", phone_normalization_without_proxy),
    TEST_NO_TAG("Phone normalization with proxy", phone_normalization_with_proxy),
    TEST_NO_TAG("Phone normalization with dial escape plus", phone_normalization_with_dial_escape_plus),
    TEST_NO_TAG("SIP URI normalization", sip_uri_normalization),
    TEST_NO_TAG("Load new default value for proxy config", load_dynamic_proxy_config),
    TEST_NO_TAG("Single route", single_route),
    TEST_NO_TAG("Multiple routes", multiple_route),
    TEST_NO_TAG("Proxy dependency", dependent_proxy_config),
    TEST_NO_TAG("Dependent proxy dependency register", proxy_config_dependent_register),
    TEST_NO_TAG("Dependent proxy state changed", proxy_config_dependent_register_state_changed),
    TEST_NO_TAG("Dependent proxy dependency removal", dependent_proxy_dependency_removal),
    TEST_NO_TAG("Dependent proxy dependency with core reloaded", dependent_proxy_dependency_with_core_reloaded),
    TEST_ONE_TAG("Push notification params", proxy_config_push_notification_params, "Push Notification"),
    TEST_ONE_TAG("Push notification params 2", proxy_config_push_notification_params_2, "Push Notification"),
    TEST_ONE_TAG("Push notification params 3", proxy_config_push_notification_params_3, "Push Notification"),
    TEST_ONE_TAG("Push notification params two proxies, one push",
                 proxy_config_push_notification_params_two_proxies_one_push,
                 "Push Notification"),
    TEST_ONE_TAG("Push notification params two proxies, one push 2",
                 proxy_config_push_notification_params_two_proxies_one_push_2,
                 "Push Notification"),
    TEST_ONE_TAG("Push notification params two proxies, one push 3",
                 proxy_config_push_notification_params_two_proxies_one_push_3,
                 "Push Notification"),
    TEST_ONE_TAG("Push notification params two proxies, both push",
                 proxy_config_push_notification_params_two_proxies_both_push,
                 "Push Notification"),
    TEST_ONE_TAG("Push notification params two proxies, both push 2",
                 proxy_config_push_notification_params_two_proxies_both_push_2,
                 "Push Notification"),
    TEST_ONE_TAG("Push notification params two proxies, both push 3",
                 proxy_config_push_notification_params_two_proxies_both_push_3,
                 "Push Notification"),
    TEST_ONE_TAG("Push notification params saved after core restart",
                 proxy_config_push_notification_core_restart,
                 "Push Notification"),
    TEST_ONE_TAG("Remove push informations from contact uri parameters if core push are enabled",
                 proxy_config_remove_push_info_from_contact_uri_parameters,
                 "Push Notification")};

test_suite_t proxy_config_test_suite = {"Proxy config",
                                        NULL,
                                        NULL,
                                        liblinphone_tester_before_each,
                                        liblinphone_tester_after_each,
                                        sizeof(proxy_config_tests) / sizeof(proxy_config_tests[0]),
                                        proxy_config_tests,
                                        0};
