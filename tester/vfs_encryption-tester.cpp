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



#include "linphone/core.h"
#include "tester_utils.h"
#include "linphone/wrapper_utils.h"
#include "liblinphone_tester.h"
#include <iostream>
#include <fstream>

/* encrypted files starts with text : bcEncryptedFs
 * read the first 13 char of the file and check them */
static bool is_encrypted(const char *filepath) {
	BCTBX_SLOGD<<"JOHAN: is encrypted ? "<<filepath;
	bool ret = false;
	uint8_t evfs_magicNumber[13] = {0x62, 0x63, 0x45, 0x6e, 0x63, 0x72, 0x79, 0x70, 0x74, 0x65, 0x64, 0x46, 0x73};
	std::ifstream file (filepath, std::ios::in | std::ios::binary);
	if (file.is_open()) {
		char readBuf[13];
		file.seekg(0, std::ios::beg);
		auto sizeRead = file.read(readBuf, 13).gcount();
		BCTBX_SLOGD<<"JOHAN: is encrypted ? "<<filepath<<" got "<<sizeRead<<" bytes";

		if (sizeRead == 13) {
			ret = std::equal(evfs_magicNumber, evfs_magicNumber+13, readBuf);
		}
	}
	file.close();
	return ret;
}

static void register_user(uint16_t encryptionModule) {
	// enable encryption
	if (encryptionModule == LINPHONE_VFS_ENCRYPTION_PLAIN) {
		linphone_factory_set_vfs_encryption(linphone_factory_get(), LINPHONE_VFS_ENCRYPTION_PLAIN, NULL, 0);
	} else if (encryptionModule == LINPHONE_VFS_ENCRYPTION_DUMMY) {
		uint8_t evfs_key[16] = {0xaa, 0x55, 0xFF, 0xFF, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x11, 0x22, 0x33, 0x44};
		linphone_factory_set_vfs_encryption(linphone_factory_get(), LINPHONE_VFS_ENCRYPTION_DUMMY, evfs_key, 16);
	}

	// create a user
	LinphoneCoreManager* marie;
	char random_id[8];
	belle_sip_random_token(random_id, sizeof random_id);
	auto localRc_filename = bctbx_strdup_printf("evfs_register_marie_rc_%s", random_id);
	auto localRc = bc_tester_file(localRc_filename);
	bctbx_free(localRc_filename);

	unlink(localRc); // make sure we're not reusing a rc file

	// use a local rc file built from marie_rc to check it is encrypted
	marie = linphone_core_manager_new_localrc("marie_rc", localRc);

	// check it registers ok
	wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 1);

	// check the linphone db and local_rc are encrypted or not
	if (encryptionModule == LINPHONE_VFS_ENCRYPTION_PLAIN) {
		BC_ASSERT_FALSE(is_encrypted(marie->database_path));
		BC_ASSERT_FALSE(is_encrypted(localRc));
	} else {
		BC_ASSERT_TRUE(is_encrypted(marie->database_path));
		BC_ASSERT_TRUE(is_encrypted(localRc));
	}

	// cleaning
	linphone_core_manager_destroy(marie);
	unlink(localRc);
	bctbx_free(localRc);
}

static void register_test(void) {
	register_user(LINPHONE_VFS_ENCRYPTION_PLAIN);
	register_user(LINPHONE_VFS_ENCRYPTION_DUMMY);
}

test_t vfs_encryption_tests[] = {
	TEST_NO_TAG("Register", register_test)
};

test_suite_t vfs_encryption_test_suite = {
	"VFS encryption",
	NULL,
	NULL,
	liblinphone_tester_before_each,
	liblinphone_tester_after_each,
	sizeof(vfs_encryption_tests) / sizeof(vfs_encryption_tests[0]), vfs_encryption_tests
};


