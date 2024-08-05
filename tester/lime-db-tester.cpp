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

#include "liblinphone_tester.h"
#include "linphone/core.h"
#include "private.h"

using namespace std;

using namespace LinphonePrivate;

static void db_access_test(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneConfig *config = linphone_factory_create_config_with_factory(linphone_factory_get(), marie->rc_local, NULL);
	std::string limeDb("my_file.db");
	linphone_config_set_string(config, "lime", "x3dh_db_path", limeDb.c_str());
	auto lc = configure_lc_from(marie->cbs, bc_tester_get_resource_dir_prefix(), config, marie);
	auto core = shared_ptr<Core>(L_GET_CPP_PTR_FROM_C_OBJECT(lc));
	auto actualDbPath = core->getX3dhDbPath();
	auto expectedDbPath = core->getDataPath() + limeDb;
	BC_ASSERT_STRING_EQUAL(actualDbPath.c_str(), expectedDbPath.c_str());
	linphone_config_unref(config);
	linphone_core_unref(lc);
	linphone_core_manager_destroy(marie);
}

test_t lime_db_tests[] = {
    TEST_ONE_TAG("Data base access", db_access_test, "LimeX3DH"),
};

test_suite_t lime_db_test_suite = {"Lime data base",
                                   NULL,
                                   NULL,
                                   liblinphone_tester_before_each,
                                   liblinphone_tester_after_each,
                                   sizeof(lime_db_tests) / sizeof(lime_db_tests[0]),
                                   lime_db_tests,
                                   0};
