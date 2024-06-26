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
#include "tester_utils.h"
#include <ctype.h>

static LinphoneAccountCreator *init(LinphoneCore *lc) {
	LinphoneAccountCreatorService *service = linphone_account_creator_service_new();
	linphone_account_creator_service_set_constructor_cb(service, NULL);
	linphone_account_creator_service_set_destructor_cb(service, NULL);
	linphone_account_creator_service_set_create_account_cb(service,
	                                                       linphone_account_creator_create_account_linphone_xmlrpc);
	linphone_account_creator_service_set_is_account_exist_cb(service,
	                                                         linphone_account_creator_is_account_exist_linphone_xmlrpc);
	linphone_account_creator_service_set_activate_account_cb(
	    service, linphone_account_creator_activate_phone_account_linphone_xmlrpc);
	linphone_account_creator_service_set_is_account_activated_cb(
	    service, linphone_account_creator_is_account_activated_linphone_xmlrpc);
	linphone_account_creator_service_set_link_account_cb(
	    service, linphone_account_creator_link_phone_number_with_account_linphone_xmlrpc);
	linphone_account_creator_service_set_activate_alias_cb(
	    service, linphone_account_creator_activate_phone_number_link_linphone_xmlrpc);
	linphone_account_creator_service_set_is_alias_used_cb(
	    service, linphone_account_creator_is_phone_number_used_linphone_xmlrpc);
	linphone_account_creator_service_set_is_account_linked_cb(
	    service, linphone_account_creator_is_account_linked_linphone_xmlrpc);
	linphone_account_creator_service_set_recover_account_cb(
	    service, linphone_account_creator_recover_phone_account_linphone_xmlrpc);
	linphone_account_creator_service_set_update_account_cb(service,
	                                                       linphone_account_creator_update_password_linphone_xmlrpc);
	linphone_account_creator_service_set_login_linphone_account_cb(
	    service, linphone_account_creator_login_linphone_account_linphone_xmlrpc);
	linphone_core_set_account_creator_service(lc, service);

	return linphone_account_creator_create(lc);
}

////// USERNAME //////
static void local_username_too_short(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);

	BC_ASSERT_EQUAL(linphone_account_creator_set_username(creator, ""), LinphoneAccountCreatorUsernameStatusTooShort,
	                LinphoneAccountCreatorUsernameStatus, "%i");

	linphone_core_manager_destroy(marie);
	linphone_account_creator_unref(creator);
}

static void local_username_too_long(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);

	BC_ASSERT_EQUAL(linphone_account_creator_set_username(creator, "usernametoolongforyoutoobadmorelucknexttime"),
	                LinphoneAccountCreatorUsernameStatusTooLong, LinphoneAccountCreatorUsernameStatus, "%i");

	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
}

static void local_username_invalid_character(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);

	BC_ASSERT_EQUAL(linphone_account_creator_set_username(creator, "use!"),
	                LinphoneAccountCreatorUsernameStatusInvalidCharacters, LinphoneAccountCreatorUsernameStatus, "%i");

	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
}

static void local_username_ok(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);

	BC_ASSERT_EQUAL(linphone_account_creator_set_username(creator, "xxxtestuser_1"),
	                LinphoneAccountCreatorUsernameStatusOk, LinphoneAccountCreatorUsernameStatus, "%i");

	BC_ASSERT_STRING_EQUAL(linphone_account_creator_get_username(creator), "xxxtestuser_1");

	BC_ASSERT_EQUAL(linphone_account_creator_set_username(creator, "XXXTESTuser_1"),
	                LinphoneAccountCreatorUsernameStatusOk, LinphoneAccountCreatorUsernameStatus, "%i");

	BC_ASSERT_STRING_EQUAL(linphone_account_creator_get_username(creator), "XXXTESTuser_1");

	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
}

////// PASSWORD //////

static void local_password_too_short(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);

	BC_ASSERT_EQUAL(linphone_account_creator_set_password(creator, ""), LinphoneAccountCreatorPasswordStatusTooShort,
	                LinphoneAccountCreatorPasswordStatus, "%i");

	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
}

static void local_password_too_long(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);

	BC_ASSERT_EQUAL(linphone_account_creator_set_password(creator, "passwordtoolong"),
	                LinphoneAccountCreatorPasswordStatusTooLong, LinphoneAccountCreatorPasswordStatus, "%i");

	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
}

static void local_password_ok(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);

	BC_ASSERT_EQUAL(linphone_account_creator_set_password(creator, "pass"), LinphoneAccountCreatorPasswordStatusOk,
	                LinphoneAccountCreatorPasswordStatus, "%i");

	BC_ASSERT_STRING_EQUAL(linphone_account_creator_get_password(creator), "pass");

	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
}

////// EMAIL //////

static void local_email_malformed(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);

	BC_ASSERT_EQUAL(linphone_account_creator_set_email(creator, "test.linphone.org"),
	                LinphoneAccountCreatorEmailStatusMalformed, LinphoneAccountCreatorEmailStatus, "%i");

	BC_ASSERT_EQUAL(linphone_account_creator_set_email(creator, "test@linphone"),
	                LinphoneAccountCreatorEmailStatusMalformed, LinphoneAccountCreatorEmailStatus, "%i");

	BC_ASSERT_EQUAL(linphone_account_creator_set_email(creator, "@linphone.org"),
	                LinphoneAccountCreatorEmailStatusMalformed, LinphoneAccountCreatorEmailStatus, "%i");

	BC_ASSERT_EQUAL(linphone_account_creator_set_email(creator, "linphone@.org"),
	                LinphoneAccountCreatorEmailStatusMalformed, LinphoneAccountCreatorEmailStatus, "%i");

	BC_ASSERT_EQUAL(linphone_account_creator_set_email(creator, ".linphone@.org"),
	                LinphoneAccountCreatorEmailStatusMalformed, LinphoneAccountCreatorEmailStatus, "%i");

	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
}

static void local_email_invalid_character(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);

	BC_ASSERT_EQUAL(linphone_account_creator_set_email(creator, "test@linphone.org$"),
	                LinphoneAccountCreatorEmailStatusInvalidCharacters, LinphoneAccountCreatorEmailStatus, "%i");

	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
}

static void local_email_ok(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);

	BC_ASSERT_EQUAL(linphone_account_creator_set_email(creator, "test@linphone.org"),
	                LinphoneAccountCreatorEmailStatusOk, LinphoneAccountCreatorEmailStatus, "%i");
	BC_ASSERT_STRING_EQUAL(linphone_account_creator_get_email(creator), "test@linphone.org");

	BC_ASSERT_EQUAL(linphone_account_creator_set_email(creator, "test02@linphone5252.org"),
	                LinphoneAccountCreatorEmailStatusOk, LinphoneAccountCreatorEmailStatus, "%i");
	BC_ASSERT_STRING_EQUAL(linphone_account_creator_get_email(creator), "test02@linphone5252.org");

	BC_ASSERT_EQUAL(linphone_account_creator_set_email(creator, "9053test@50255linphone.org"),
	                LinphoneAccountCreatorEmailStatusOk, LinphoneAccountCreatorEmailStatus, "%i");
	BC_ASSERT_STRING_EQUAL(linphone_account_creator_get_email(creator), "9053test@50255linphone.org");

	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
}

////// PHONE NUMBER //////

/*static void local_phone_number_too_short(void) {
    LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
    LinphoneAccountCreator *creator = init(marie->lc);

    BC_ASSERT_EQUAL(
        linphone_account_creator_set_phone_number(creator, "0123",
"33")&LinphoneAccountCreatorPhoneNumberStatusTooShort, LinphoneAccountCreatorPhoneNumberStatusTooShort,
        LinphoneAccountCreatorPhoneNumberStatus,
        "%i");

    linphone_account_creator_unref(creator);
    linphone_core_manager_destroy(marie);
}*/

static void local_phone_number_too_long(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);

	BC_ASSERT_EQUAL(linphone_account_creator_set_phone_number(creator, "01234567891011", "33") &
	                    LinphoneAccountCreatorPhoneNumberStatusTooLong,
	                LinphoneAccountCreatorPhoneNumberStatusTooLong, LinphoneAccountCreatorPhoneNumberStatus, "%i");

	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
}

static void local_phone_number_invalid(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);

	BC_ASSERT_EQUAL(linphone_account_creator_set_phone_number(creator, NULL, "33") &
	                    LinphoneAccountCreatorPhoneNumberStatusInvalid,
	                LinphoneAccountCreatorPhoneNumberStatusInvalid, LinphoneAccountCreatorPhoneNumberStatus, "%i");

	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
}

static void local_country_code_invalid(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);

	BC_ASSERT_EQUAL(linphone_account_creator_set_phone_number(creator, "0123", "") &
	                    LinphoneAccountCreatorPhoneNumberStatusInvalidCountryCode,
	                LinphoneAccountCreatorPhoneNumberStatusInvalidCountryCode, LinphoneAccountCreatorPhoneNumberStatus,
	                "%i");

	BC_ASSERT_EQUAL(linphone_account_creator_set_phone_number(creator, "0123", "+") &
	                    LinphoneAccountCreatorPhoneNumberStatusInvalidCountryCode,
	                LinphoneAccountCreatorPhoneNumberStatusInvalidCountryCode, LinphoneAccountCreatorPhoneNumberStatus,
	                "%i");

	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
}

static void local_phone_number_ok(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("account_creator_rc", FALSE);
	LinphoneAccountCreator *creator = init(marie->lc);

	BC_ASSERT_EQUAL(linphone_account_creator_set_phone_number(creator, "0005554559", "1") &
	                    LinphoneAccountCreatorPhoneNumberStatusOk,
	                LinphoneAccountCreatorPhoneNumberStatusOk, LinphoneAccountCreatorPhoneNumberStatus, "%i");
	BC_ASSERT_STRING_EQUAL(linphone_account_creator_get_phone_number(creator), "+10005554559");

	linphone_account_creator_unref(creator);
	linphone_core_manager_destroy(marie);
}

/****************** End Update Account ************************/

test_t account_creator_local_tests[] = {
    TEST_ONE_TAG("Local - Username too short", local_username_too_short, "Local"),
    TEST_ONE_TAG("Local - Username too long", local_username_too_long, "Local"),
    TEST_ONE_TAG("Local - Username invalid character", local_username_invalid_character, "Local"),
    TEST_ONE_TAG("Local - Username Ok", local_username_ok, "Local"),
    TEST_ONE_TAG("Local - Password too short", local_password_too_short, "Local"),
    TEST_ONE_TAG("Local - Password too long", local_password_too_long, "Local"),
    TEST_ONE_TAG("Local - Password Ok", local_password_ok, "Local"),
    TEST_ONE_TAG("Local - Email malformed", local_email_malformed, "Local"),
    TEST_ONE_TAG("Local - Email invalid character", local_email_invalid_character, "Local"),
    TEST_ONE_TAG("Local - Email Ok", local_email_ok, "Local"),
    /*TEST_ONE_TAG(
        "Local - Phone number too short",
        local_phone_number_too_short,
        "Local"),*/
    TEST_ONE_TAG("Local - Phone number too long", local_phone_number_too_long, "Local"),
    TEST_ONE_TAG("Local - Phone number invalid", local_phone_number_invalid, "Local"),
    TEST_ONE_TAG("Local - Country code invalid", local_country_code_invalid, "Local"),
    TEST_ONE_TAG("Local - Phone number ok", local_phone_number_ok, "Local"),
};

test_suite_t account_creator_local_test_suite = {"Account creator local",
                                                 NULL,
                                                 NULL,
                                                 liblinphone_tester_before_each,
                                                 liblinphone_tester_after_each,
                                                 sizeof(account_creator_local_tests) /
                                                     sizeof(account_creator_local_tests[0]),
                                                 account_creator_local_tests,
                                                 0};
