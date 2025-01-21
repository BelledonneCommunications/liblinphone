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
#include <bctoolbox/vfs.h>

using namespace std;

using namespace LinphonePrivate;

// This test verifies the database access configuration.
static void db_access_test(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneConfig *config = linphone_factory_create_config_with_factory(linphone_factory_get(), marie->rc_local, NULL);

	/* Define a database file name for the LIME X3DH database.
	 * Set the database path in the configuration under the "lime" section for "x3dh_db_path".
	 */
	std::string limeDb("my_file.db");
	linphone_config_set_string(config, "lime", "x3dh_db_path", limeDb.c_str());
	auto lc = configure_lc_from(marie->cbs, bc_tester_get_resource_dir_prefix(), config, marie);
	auto core = shared_ptr<Core>(L_GET_CPP_PTR_FROM_C_OBJECT(lc));

	/* Retrieve the actual database path as determined by core->getX3dhDbPath().
	 * Define the expected database path by appending the limeDb name to the data path.
	 * Assert that the actual database path matches the expected database path.
	 */
	auto actualDbPath = core->getX3dhDbPath();
	auto expectedDbPath = core->getDataPath() + limeDb;
	BC_ASSERT_STRING_EQUAL(actualDbPath.c_str(), expectedDbPath.c_str());

	linphone_config_unref(config);
	linphone_core_unref(lc);
	linphone_core_manager_destroy(marie);
}

// This test verifies that the database can be correctly configured to be stored in memory.
static void db_access_store_db_in_memory_test(void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneConfig *config = linphone_factory_create_config_with_factory(linphone_factory_get(), marie->rc_local, NULL);

	/* Define the database path to be stored in memory using ":memory:".
	 * Set the database path in the configuration under the "lime" section for "x3dh_db_path".
	 */
	std::string limeDb(":memory:");
	linphone_config_set_string(config, "lime", "x3dh_db_path", limeDb.c_str());
	auto lc = configure_lc_from(marie->cbs, bc_tester_get_resource_dir_prefix(), config, marie);
	auto core = shared_ptr<Core>(L_GET_CPP_PTR_FROM_C_OBJECT(lc));

	/* Retrieve the actual database path as determined by core->getX3dhDbPath().
	 * Assert that the actual database path matches the expected in-memory database path ":memory:".
	 */
	auto actualDbPath = core->getX3dhDbPath();
	BC_ASSERT_STRING_EQUAL(actualDbPath.c_str(), limeDb.c_str());

	linphone_config_unref(config);
	linphone_core_unref(lc);
	linphone_core_manager_destroy(marie);
}

/**
 * Scenario:
 * - create a lime user
 * - stop the manager and corrupt the db so it cannot be opened by sqlite3
 * - restart the manager -> the imee should not be created
 */
static void corrupted_db(void) {
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	set_lime_server_and_curve_list(C25519, coresManagerList);
	stats initialPaulineStats = pauline->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	// Wait for lime user creation
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess,
	                             initialPaulineStats.number_of_X3dhUserCreationSuccess + 1,
	                             x3dhServer_creationTimeout));
	// Check imee is not null
	BC_ASSERT_PTR_NOT_NULL(L_GET_CPP_PTR_FROM_C_OBJECT(pauline->lc)->getEncryptionEngine());

	// Restart Pauline core, so the encryption engine is stopped and started and looses his cache
	linphone_core_set_network_reachable(pauline->lc, FALSE);
	coresList = bctbx_list_remove(coresList, pauline->lc);
	// Corrupt Pauline database so it is not a valid sqlite3 file anymore
	bctbx_vfs_file_t *dbFile = bctbx_file_open2(bctbx_vfs_get_default(), pauline->lime_database_path, O_RDWR);
	const uint8_t buf[16] = {0};
	bctbx_file_write2(dbFile, buf, 16);
	bctbx_file_close(dbFile);

	linphone_core_manager_reinit(pauline);
	bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, pauline);
	set_lime_server_and_curve_list(C25519, tmpCoresManagerList);
	bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	linphone_core_manager_start(pauline, TRUE);
	wait_for_list(coresList, 0, 1, 2000); // Make sure Pauline's core restart is all done
	// Check imee is null
	BC_ASSERT_PTR_NULL(L_GET_CPP_PTR_FROM_C_OBJECT(pauline->lc)->getEncryptionEngine());

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(pauline);
}

test_t lime_db_tests[] = {
    TEST_NO_TAG("Data base access", db_access_test),
    TEST_NO_TAG("Data base access : Store BD in memory", db_access_store_db_in_memory_test),
    TEST_ONE_TAG("Corrupted db", corrupted_db, "LimeX3DH"),
};

test_suite_t lime_db_test_suite = {"Lime data bases",
                                   NULL,
                                   NULL,
                                   liblinphone_tester_before_each,
                                   liblinphone_tester_after_each,
                                   sizeof(lime_db_tests) / sizeof(lime_db_tests[0]),
                                   lime_db_tests,
                                   0};
