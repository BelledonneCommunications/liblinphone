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
#include "bctoolbox/vfs_encrypted.hh" // included for testing purpose, we could use encryption without it from a C file
#include "bctoolbox/logging.h"
#include <iostream>
#include <fstream>

static const int x3dhServer_creationTimeout = 5000;
static bool is_encrypted(const char *filepath) {
	bool ret = false;
	auto fp = bctbx_file_open(&bctoolbox::bcEncryptedVfs, filepath, "r");
	if (fp != NULL) {
		ret = (bctbx_file_isEncrypted(fp)==TRUE);
		bctbx_file_close(fp);
	}
	return ret;
}

static void enable_encryption(const uint16_t encryptionModule, const bool encryptDbJournal=true) {
	// enable encryption. The call to linphone_factory_set_vfs_encryption will set the VfsEncryption class callback
	if (encryptionModule == LINPHONE_VFS_ENCRYPTION_PLAIN) {
		linphone_factory_set_vfs_encryption(linphone_factory_get(), LINPHONE_VFS_ENCRYPTION_PLAIN, NULL, 0);
	} else if (encryptionModule == LINPHONE_VFS_ENCRYPTION_DUMMY) {
		uint8_t evfs_key[16] = {0xaa, 0x55, 0xFF, 0xFF, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x11, 0x22, 0x33, 0x44};
		linphone_factory_set_vfs_encryption(linphone_factory_get(), LINPHONE_VFS_ENCRYPTION_DUMMY, evfs_key, 16);
	} else if (encryptionModule == LINPHONE_VFS_ENCRYPTION_AES256GCM128_SHA256) {
		uint8_t evfs_key[32] = {0xaa, 0x55, 0xFF, 0xFF, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x11, 0x22, 0x33, 0x44,
					0x5a, 0xa5, 0x5F, 0xaF, 0x52, 0xa4, 0xa6, 0x58, 0xaa, 0x5c, 0xae, 0x50, 0xa1, 0x52, 0xa3, 0x54};
		linphone_factory_set_vfs_encryption(linphone_factory_get(), LINPHONE_VFS_ENCRYPTION_AES256GCM128_SHA256, evfs_key, 32);
	}


	// For testing purpose and usage demo: directly access the evfs open callback to filter some files
	// The previous call to linphone_factory_set_vfs_encryption set a callback but we can still modify it
	auto currentVfsCb = bctoolbox::VfsEncryption::openCallbackGet();
	bctoolbox::VfsEncryption::openCallbackSet([currentVfsCb, encryptDbJournal](bctoolbox::VfsEncryption &settings){
			// prevent the encryption of any file with filename ending with -journal
			auto filename = settings.filenameGet();
			if (encryptDbJournal == false && (filename.size() > 8) && ( filename.compare (filename.size()-8, 8, std::string{"-journal"}) == 0)) { // This is a plain transfered file
				BCTBX_SLOGD<<"Encryption test: skip encryption for -journal file";
				settings.encryptionSuiteSet(bctoolbox::EncryptionSuite::plain);
			} else { // just call the registered cb (the one registered by the linphone_factory_set_vfs_encryption call)
				currentVfsCb(settings);
			}
			});
}

// when call with create users set to false, unlink all config files at the end of the test
static void register_user(const uint16_t encryptionModule, const char* random_id, const bool createUsers) {
	enable_encryption(encryptionModule, false);

	// create a user
	LinphoneCoreManager* marie;
	auto filename = bctbx_strdup_printf("evfs_register_marie_rc_%s", random_id);
	auto localRc = bc_tester_file(filename);
	bctbx_free(filename);
	filename = bctbx_strdup_printf("linphone_%s.db", random_id);
	auto linphone_db = bc_tester_file(filename);
	bctbx_free(filename);
	filename = bctbx_strdup_printf("lime_%s.db", random_id);
	auto lime_db = bc_tester_file(filename);
	bctbx_free(filename);

	/* make sure we use fresh files */
	if (createUsers == true) {
		unlink(localRc);
		unlink(linphone_db);
		unlink(lime_db);
	}

	// use a local files(rc and db) to check it is encrypted, at user creation, use a factory rc otherwise we do not need it
	marie = linphone_core_manager_new_local(createUsers?"marie_lime_x3dh_rc":NULL, localRc, linphone_db, lime_db);

	// check it registers ok and lime user is created
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 1));
	if (createUsers == true) { // when not creating the user, there is no reason to get a creation success event
		BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_X3dhUserCreationSuccess, 1, x3dhServer_creationTimeout));
	}

	// cleaning
	if (marie->lc && linphone_core_get_global_state(marie->lc) != LinphoneGlobalOff && !linphone_core_is_network_reachable(marie->lc)) {
		int previousNbRegistrationOk = marie->stat.number_of_LinphoneRegistrationOk;
		linphone_core_set_network_reachable(marie->lc, TRUE);
		wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, previousNbRegistrationOk + 1, 2000);
	}

	linphone_core_manager_stop(marie);
	linphone_core_manager_uninit2(marie, FALSE); // uinit but do not unlink the db files
	ms_free(marie);

	// check the linphone dbs and local_rc are encrypted or not
	if (encryptionModule == LINPHONE_VFS_ENCRYPTION_PLAIN) {
		BC_ASSERT_FALSE(is_encrypted(linphone_db));
		BC_ASSERT_FALSE(is_encrypted(lime_db));
		BC_ASSERT_FALSE(is_encrypted(localRc));
	} else {
		BC_ASSERT_TRUE(is_encrypted(linphone_db));
		BC_ASSERT_TRUE(is_encrypted(lime_db));
		BC_ASSERT_TRUE(is_encrypted(localRc));
	}

	if (createUsers == false) {
		unlink(localRc);
		unlink(linphone_db);
		unlink(lime_db);
	}
	bctbx_free(localRc);
	bctbx_free(linphone_db);
	bctbx_free(lime_db);

	// reset VFS encryption
	linphone_factory_set_vfs_encryption(linphone_factory_get(), LINPHONE_VFS_ENCRYPTION_UNSET, NULL, 0);
}

static void register_user_test(void) {
	char random_id[8];
	belle_sip_random_token(random_id, sizeof random_id);
	char *id = bctbx_strdup(random_id);
	// run the test once creating the user and then reusing the local files
	register_user(LINPHONE_VFS_ENCRYPTION_PLAIN, id, true);
	register_user(LINPHONE_VFS_ENCRYPTION_PLAIN, id, false);
	bctbx_free(id);
	// generate a new random id to be sure to use fresh local files
	belle_sip_random_token(random_id, sizeof random_id);
	id = bctbx_strdup(random_id);
	register_user(LINPHONE_VFS_ENCRYPTION_DUMMY, id, true);
	register_user(LINPHONE_VFS_ENCRYPTION_DUMMY, id, false);
	bctbx_free(id);
	// generate a new random id to be sure to use fresh local files
	belle_sip_random_token(random_id, sizeof random_id);
	id = bctbx_strdup(random_id);
	register_user(LINPHONE_VFS_ENCRYPTION_AES256GCM128_SHA256, id, true);
	register_user(LINPHONE_VFS_ENCRYPTION_AES256GCM128_SHA256, id, false);
	bctbx_free(id);
}

static void zrtp_call(const uint16_t encryptionModule, const char *random_id, const bool createUsers, const std::string basename="evfs_zrtp_call_") {
	enable_encryption(encryptionModule);

	// create a user Marie
	LinphoneCoreManager* marie;
	auto local_filename = bctbx_strdup_printf("%s_marie_rc_%s", basename.data(), random_id);
	auto marie_rc = bc_tester_file(local_filename);
	bctbx_free(local_filename);
	local_filename = bctbx_strdup_printf("%s_marie_ZIDcache_%s", basename.data(), random_id);
	auto marie_zidCache = bc_tester_file(local_filename);
	bctbx_free(local_filename);
	local_filename = bctbx_strdup_printf("%s_marie_linphone_%s.db", basename.data(), random_id);
	auto marie_linphone_db = bc_tester_file(local_filename);
	bctbx_free(local_filename);
	local_filename = bctbx_strdup_printf("%s_marie_lime_%s.db", basename.data(), random_id);
	auto marie_lime_db = bc_tester_file(local_filename);
	bctbx_free(local_filename);
	if (createUsers == true) { // creating users, make sure we do not reuse local files
		unlink(marie_rc);
		unlink(marie_zidCache);
		unlink(marie_linphone_db);
		unlink(marie_lime_db);
	}
	// use a local rc file built from marie_rc to check it is encrypted
	marie = linphone_core_manager_new_local(createUsers?"marie_lime_x3dh_rc":NULL, marie_rc, marie_linphone_db, marie_lime_db);
	linphone_core_set_media_encryption(marie->lc,LinphoneMediaEncryptionZRTP);
	linphone_core_set_zrtp_secrets_file(marie->lc, marie_zidCache);


	// create user Pauline
	LinphoneCoreManager* pauline;
	local_filename = bctbx_strdup_printf("%s_pauline_rc_%s", basename.data(), random_id);
	auto pauline_rc = bc_tester_file(local_filename);
	bctbx_free(local_filename);
	local_filename = bctbx_strdup_printf("%s_pauline_ZIDcache_%s", basename.data(), random_id);
	auto pauline_zidCache = bc_tester_file(local_filename);
	bctbx_free(local_filename);
	local_filename = bctbx_strdup_printf("%s_pauline_linphone_%s.db", basename.data(), random_id);
	auto pauline_linphone_db = bc_tester_file(local_filename);
	bctbx_free(local_filename);
	local_filename = bctbx_strdup_printf("%s_pauline_lime_%s.db", basename.data(), random_id);
	auto pauline_lime_db = bc_tester_file(local_filename);
	bctbx_free(local_filename);
	if (createUsers == true) { // creating users, make sure we do not reuse local files
		unlink(pauline_rc); // make sure we're not reusing a rc file
		unlink(pauline_zidCache); // make sure we're not reusing a zidCache file
		unlink(pauline_linphone_db);
		unlink(pauline_lime_db);
	}
	// use a local rc file built from pauline_rc to check it is encrypted
	pauline = linphone_core_manager_new_local(createUsers?"pauline_lime_x3dh_rc":NULL, pauline_rc, pauline_linphone_db, pauline_lime_db);
	linphone_core_set_media_encryption(pauline->lc,LinphoneMediaEncryptionZRTP);
	linphone_core_set_zrtp_secrets_file(pauline->lc, pauline_zidCache);


	// check it registers ok and lime user is created
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneRegistrationOk, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, NULL, &pauline->stat.number_of_LinphoneRegistrationOk, 1));
	if (createUsers == true) {
		BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_X3dhUserCreationSuccess, 1, x3dhServer_creationTimeout));
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, NULL, &pauline->stat.number_of_X3dhUserCreationSuccess, 1, x3dhServer_creationTimeout));
	}

	// make the ZRTP call, confirm SAS
	BC_ASSERT_TRUE(call(pauline,marie));
	if (createUsers == true) { // on the first run, set the SAS to verified, it shall still be set this way on the second run
		linphone_call_set_authentication_token_verified(linphone_core_get_current_call(marie->lc), TRUE);
		linphone_call_set_authentication_token_verified(linphone_core_get_current_call(pauline->lc), TRUE);
	}
	BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(marie->lc)));
	BC_ASSERT_TRUE(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(pauline->lc)));
	end_call(marie, pauline);

	if (encryptionModule == LINPHONE_VFS_ENCRYPTION_PLAIN) {
		BC_ASSERT_FALSE(is_encrypted(marie_zidCache));
		BC_ASSERT_FALSE(is_encrypted(pauline_zidCache));
	} else {
		BC_ASSERT_TRUE(is_encrypted(marie_zidCache));
		BC_ASSERT_TRUE(is_encrypted(pauline_zidCache));
	}

	// cleaning
	linphone_core_manager_stop(marie);
	linphone_core_manager_uninit2(marie, FALSE); // uinit but do not unlink the db files
	ms_free(marie);
	linphone_core_manager_stop(pauline);
	linphone_core_manager_uninit2(pauline, FALSE); // uinit but do not unlink the db files
	ms_free(pauline);

	// check the linphone dbs and local_rc are encrypted or not
	if (encryptionModule == LINPHONE_VFS_ENCRYPTION_PLAIN) {
		BC_ASSERT_FALSE(is_encrypted(marie_linphone_db));
		BC_ASSERT_FALSE(is_encrypted(marie_lime_db));
		BC_ASSERT_FALSE(is_encrypted(marie_rc));
		BC_ASSERT_FALSE(is_encrypted(pauline_linphone_db));
		BC_ASSERT_FALSE(is_encrypted(pauline_lime_db));
		BC_ASSERT_FALSE(is_encrypted(pauline_rc));
	} else {
		BC_ASSERT_TRUE(is_encrypted(marie_linphone_db));
		BC_ASSERT_TRUE(is_encrypted(marie_lime_db));
		BC_ASSERT_TRUE(is_encrypted(marie_rc));
		BC_ASSERT_TRUE(is_encrypted(pauline_linphone_db));
		BC_ASSERT_TRUE(is_encrypted(pauline_lime_db));
		BC_ASSERT_TRUE(is_encrypted(pauline_rc));
	}

	if (createUsers == false) { // we reused users, now clean the local files
		unlink(marie_rc);
		unlink(marie_zidCache);
		unlink(marie_linphone_db);
		unlink(marie_lime_db);
		unlink(pauline_rc);
		unlink(pauline_zidCache);
		unlink(pauline_linphone_db);
		unlink(pauline_lime_db);
	}
	bctbx_free(marie_rc);
	bctbx_free(marie_zidCache);
	bctbx_free(marie_linphone_db);
	bctbx_free(marie_lime_db);
	bctbx_free(pauline_rc);
	bctbx_free(pauline_zidCache);
	bctbx_free(pauline_linphone_db);
	bctbx_free(pauline_lime_db);

	// reset VFS encryption
	linphone_factory_set_vfs_encryption(linphone_factory_get(), LINPHONE_VFS_ENCRYPTION_UNSET, NULL, 0);
}

static void zrtp_call_test(void) {
	char random_id[8];
	char *id;
	belle_sip_random_token(random_id, sizeof random_id);
	id = bctbx_strdup(random_id);
	zrtp_call(LINPHONE_VFS_ENCRYPTION_PLAIN, id, true);
	zrtp_call(LINPHONE_VFS_ENCRYPTION_PLAIN, id, false);
	bctbx_free(id);

	belle_sip_random_token(random_id, sizeof random_id);
	id = bctbx_strdup(random_id);
	zrtp_call(LINPHONE_VFS_ENCRYPTION_DUMMY, id, true);
	zrtp_call(LINPHONE_VFS_ENCRYPTION_DUMMY, id, false);
	bctbx_free(id);

	belle_sip_random_token(random_id, sizeof random_id);
	id = bctbx_strdup(random_id);
	zrtp_call(LINPHONE_VFS_ENCRYPTION_AES256GCM128_SHA256, id, true);
	zrtp_call(LINPHONE_VFS_ENCRYPTION_AES256GCM128_SHA256, id, false);
	bctbx_free(id);
}


// run a ZRTP call, first using plain files
// then run again but using VFS encrypted: files should automatically migrate
static void migration_test(void) {
	char random_id[8];
	char *id;
	belle_sip_random_token(random_id, sizeof random_id);
	id = bctbx_strdup(random_id);
	zrtp_call(LINPHONE_VFS_ENCRYPTION_PLAIN, id, true, "evfs_migration");
	zrtp_call(LINPHONE_VFS_ENCRYPTION_DUMMY, id, false, "evfs_migration");
	bctbx_free(id);

	belle_sip_random_token(random_id, sizeof random_id);
	id = bctbx_strdup(random_id);
	zrtp_call(LINPHONE_VFS_ENCRYPTION_PLAIN, id, true, "evfs_migration");
	zrtp_call(LINPHONE_VFS_ENCRYPTION_AES256GCM128_SHA256, id, false, "evfs_migration");
	bctbx_free(id);
}

static void file_transfer_test(const uint16_t encryptionModule, const char *random_id, const bool createUsers, const std::string basename="evfs_file_transfer_") {
	enable_encryption(encryptionModule);

	// create a user Marie
	LinphoneCoreManager* marie;
	auto local_filename = bctbx_strdup_printf("%s_marie_rc_%s", basename.data(), random_id);
	auto marie_rc = bc_tester_file(local_filename);
	bctbx_free(local_filename);
	local_filename = bctbx_strdup_printf("%s_marie_linphone_%s.db", basename.data(), random_id);
	auto marie_linphone_db = bc_tester_file(local_filename);
	bctbx_free(local_filename);
	local_filename = bctbx_strdup_printf("%s_marie_lime_%s.db", basename.data(), random_id);
	auto marie_lime_db = bc_tester_file(local_filename);
	bctbx_free(local_filename);
	if (createUsers == true) { // creating users, make sure we do not reuse local files
		unlink(marie_rc);
		unlink(marie_linphone_db);
		unlink(marie_lime_db);
	}
	// use a local rc file built from marie_rc to check it is encrypted
	marie = linphone_core_manager_create_local(createUsers?"marie_lime_x3dh_rc":NULL, marie_rc, marie_linphone_db, marie_lime_db);
	linphone_core_set_media_encryption(marie->lc,LinphoneMediaEncryptionZRTP);
	// set file transfer
	linphone_core_set_file_transfer_server(marie->lc, file_transfer_url);

	// create user Pauline
	LinphoneCoreManager* pauline;
	local_filename = bctbx_strdup_printf("%s_pauline_rc_%s", basename.data(), random_id);
	auto pauline_rc = bc_tester_file(local_filename);
	bctbx_free(local_filename);
	local_filename = bctbx_strdup_printf("%s_pauline_linphone_%s.db", basename.data(), random_id);
	auto pauline_linphone_db = bc_tester_file(local_filename);
	bctbx_free(local_filename);
	local_filename = bctbx_strdup_printf("%s_pauline_lime_%s.db", basename.data(), random_id);
	auto pauline_lime_db = bc_tester_file(local_filename);
	bctbx_free(local_filename);
	local_filename = bctbx_strdup_printf("%s_pauline_receive_file_%s.dump", basename.data(), random_id);
	auto receivePaulineFilepath = bc_tester_file(local_filename);
	bctbx_free(local_filename);
	if (createUsers == true) { // creating users, make sure we do not reuse local files
		unlink(pauline_rc); // make sure we're not reusing a rc file
		unlink(pauline_linphone_db);
		unlink(pauline_lime_db);
	}
	// use a local rc file built from pauline_rc to check it is encrypted
	pauline = linphone_core_manager_create_local(createUsers?"pauline_lime_x3dh_rc":NULL, pauline_rc, pauline_linphone_db, pauline_lime_db);
	linphone_core_set_media_encryption(pauline->lc,LinphoneMediaEncryptionZRTP);
	// set file transfer
	linphone_core_set_file_transfer_server(pauline->lc, file_transfer_url);

	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);

	// set lime serveur - fixed to curve25519 we're not testing lime here
	set_lime_curve_list(25519,coresManagerList);
	
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	// start cores
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	// Wait for lime users to be created on X3DH server
	if (createUsers == true) {
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
	}

	// Marie creates a new group chat room
	bctbx_list_t *participantsAddresses = NULL;
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, FALSE);

	// send file
	char *sendFilepath = bc_tester_res("sounds/sintel_trailer_opus_vp8.mkv");
	_send_file(marieCr, sendFilepath, nullptr, false);

	// check pauline got it
	remove(receivePaulineFilepath);
	_receive_file(coresList, pauline, &initialPaulineStats, receivePaulineFilepath, sendFilepath, nullptr, false);

	// It shall be encrypted
	if (encryptionModule == LINPHONE_VFS_ENCRYPTION_PLAIN) {
		BC_ASSERT_FALSE(is_encrypted(receivePaulineFilepath));
	} else {
		BC_ASSERT_TRUE(is_encrypted(receivePaulineFilepath));
	}
	// but not the original file sent
	BC_ASSERT_FALSE(is_encrypted(sendFilepath));

	// Get the file using the linphone content API
	LinphoneChatMessage *msg = pauline->stat.last_received_chat_message;
	auto contents = linphone_chat_message_get_contents(msg);
	BC_ASSERT_PTR_NOT_NULL(contents);
	BC_ASSERT_EQUAL(1, bctbx_list_size(contents), int, "%d");
	auto content = (LinphoneContent *)bctbx_list_get_data(contents);
	BC_ASSERT_PTR_NOT_NULL(content);
	BC_ASSERT_NSTRING_EQUAL(linphone_content_get_file_path(content), receivePaulineFilepath, strlen(receivePaulineFilepath)); // just to check we're on the correct path
	if (encryptionModule == LINPHONE_VFS_ENCRYPTION_PLAIN) {
		BC_ASSERT_FALSE(linphone_content_is_file_encrypted(content));
	} else {
		BC_ASSERT_TRUE(linphone_content_is_file_encrypted(content));
		// get a plain version of the file
		auto plainFilePath = linphone_content_get_plain_file_path(content);
		// check it is plain and match the sent one
		BC_ASSERT_FALSE(is_encrypted(plainFilePath));
		compare_files(plainFilePath, sendFilepath);
		std::remove(plainFilePath);
		bctbx_free(plainFilePath);
	}

	// cleaning
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	remove(receivePaulineFilepath);
	bc_free(sendFilepath);
	bc_free(receivePaulineFilepath);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);

	linphone_core_manager_stop(marie);
	linphone_core_manager_uninit2(marie, FALSE); // uinit but do not unlink the db files
	ms_free(marie);
	linphone_core_manager_stop(pauline);
	linphone_core_manager_uninit2(pauline, FALSE); // uinit but do not unlink the db files
	ms_free(pauline);

	// check the linphone dbs and local_rc are encrypted or not
	if (encryptionModule == LINPHONE_VFS_ENCRYPTION_PLAIN) {
		BC_ASSERT_FALSE(is_encrypted(marie_linphone_db));
		BC_ASSERT_FALSE(is_encrypted(marie_lime_db));
		BC_ASSERT_FALSE(is_encrypted(marie_rc));
		BC_ASSERT_FALSE(is_encrypted(pauline_linphone_db));
		BC_ASSERT_FALSE(is_encrypted(pauline_lime_db));
		BC_ASSERT_FALSE(is_encrypted(pauline_rc));
	} else {
		BC_ASSERT_TRUE(is_encrypted(marie_linphone_db));
		BC_ASSERT_TRUE(is_encrypted(marie_lime_db));
		BC_ASSERT_TRUE(is_encrypted(marie_rc));
		BC_ASSERT_TRUE(is_encrypted(pauline_linphone_db));
		BC_ASSERT_TRUE(is_encrypted(pauline_lime_db));
		BC_ASSERT_TRUE(is_encrypted(pauline_rc));
	}

	if (createUsers == false) { // we reused users, now clean the local files
		unlink(marie_rc);
		unlink(marie_linphone_db);
		unlink(marie_lime_db);
		unlink(pauline_rc);
		unlink(pauline_linphone_db);
		unlink(pauline_lime_db);
	}
	bctbx_free(marie_rc);
	bctbx_free(marie_linphone_db);
	bctbx_free(marie_lime_db);
	bctbx_free(pauline_rc);
	bctbx_free(pauline_linphone_db);
	bctbx_free(pauline_lime_db);

	// reset VFS encryption
	linphone_factory_set_vfs_encryption(linphone_factory_get(), LINPHONE_VFS_ENCRYPTION_UNSET, NULL, 0);
}

static void file_transfer_test() {
	char random_id[8];
	char *id;
	belle_sip_random_token(random_id, sizeof random_id);
	id = bctbx_strdup(random_id);
	file_transfer_test(LINPHONE_VFS_ENCRYPTION_DUMMY, id, true, "evfs_file_transfer");
	file_transfer_test(LINPHONE_VFS_ENCRYPTION_DUMMY, id, false, "evfs_file_transfer");
	bctbx_free(id);

	belle_sip_random_token(random_id, sizeof random_id);
	id = bctbx_strdup(random_id);
	file_transfer_test(LINPHONE_VFS_ENCRYPTION_AES256GCM128_SHA256, id, true, "evfs_file_transfer");
	file_transfer_test(LINPHONE_VFS_ENCRYPTION_AES256GCM128_SHA256, id, false, "evfs_file_transfer");
	bctbx_free(id);

}
test_t vfs_encryption_tests[] = {
	TEST_NO_TAG("Register user", register_user_test),
	TEST_NO_TAG("ZRTP call", zrtp_call_test),
	TEST_NO_TAG("Migration", migration_test),
	TEST_NO_TAG("File transfer", file_transfer_test)
};

test_suite_t vfs_encryption_test_suite = {
	"VFS encryption",
	NULL,
	NULL,
	liblinphone_tester_before_each,
	liblinphone_tester_after_each,
	sizeof(vfs_encryption_tests) / sizeof(vfs_encryption_tests[0]), vfs_encryption_tests
};


