/*
	vcard_tester.c
	Copyright (C) 2015  Belledonne Communications SARL

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "linphone/core.h"


#ifdef VCARD_ENABLED


#include "liblinphone_tester.h"
#include "tester_utils.h"
#include "carddav.h"
#include <bctoolbox/map.h>

#include <time.h>
#define CARDDAV_SERVER "http://dav.linphone.org/card.php/addressbooks/tester/default"
#define CARDDAV_SYNC_TIMEOUT 15000

static void linphone_vcard_import_export_friends_test(void) {
	LinphoneCoreManager* manager = linphone_core_manager_new2("empty_rc", FALSE);
	LinphoneFriendList *lfl = linphone_core_get_default_friend_list(manager->lc);
	const bctbx_list_t *friends = linphone_friend_list_get_friends(lfl);
	char *import_filepath = bc_tester_res("vcards/vcards.vcf");
	char *export_filepath = bc_tester_file("export_vcards.vcf");
	int count = 0;
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends), 0, unsigned int, "%u");

	count = linphone_friend_list_import_friends_from_vcard4_file(lfl, import_filepath);
	BC_ASSERT_EQUAL(count, 3, int, "%d");
	friends = linphone_friend_list_get_friends(lfl);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends), 3, unsigned int, "%u");

	linphone_friend_list_export_friends_as_vcard4_file(lfl, export_filepath);

	lfl = linphone_core_create_friend_list(manager->lc);
	count = linphone_friend_list_import_friends_from_vcard4_file(lfl, export_filepath);
	BC_ASSERT_EQUAL(count, 3, int, "%d");
	friends = linphone_friend_list_get_friends(lfl);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends), 3, unsigned int, "%u");
	linphone_friend_list_unref(lfl);

	remove(export_filepath);
	bc_free(import_filepath);
	bc_free(export_filepath);
	linphone_core_manager_destroy(manager);
}

static void linphone_vcard_import_a_lot_of_friends_test(void) {
	LinphoneCoreManager* manager = linphone_core_manager_new2("empty_rc", FALSE);
	LinphoneFriendList *lfl = linphone_core_get_default_friend_list(manager->lc);
	char *import_filepath = bc_tester_res("vcards/thousand_vcards.vcf");
	clock_t start, end;
	double elapsed = 0;
	const bctbx_list_t *friends = NULL;
	FILE    *infile = NULL;
	char    *buffer = NULL;
	long    numbytes = 0;
	size_t readbytes;

	start = clock();
	linphone_friend_list_import_friends_from_vcard4_file(lfl, import_filepath);
	end = clock();

	friends = linphone_friend_list_get_friends(lfl);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends), 1000, unsigned int, "%u"); // Now that we accept Friends without a SIP URI, the result must be equal to 1000

	elapsed = (double)(end - start);
	ms_message("Imported a thousand of vCards from file in %f seconds", elapsed / CLOCKS_PER_SEC);

	lfl = linphone_core_create_friend_list(manager->lc);
	infile = fopen(import_filepath, "rb");
	BC_ASSERT_PTR_NOT_NULL(infile);
	if (infile) {
		fseek(infile, 0L, SEEK_END);
		numbytes = ftell(infile);
		fseek(infile, 0L, SEEK_SET);
		buffer = (char*)ms_malloc((numbytes + 1) * sizeof(char));
		readbytes = fread(buffer, sizeof(char), numbytes, infile);
		fclose(infile);
		buffer[readbytes] = '\0';

		start = clock();
		linphone_friend_list_import_friends_from_vcard4_buffer(lfl, buffer);
		end = clock();
		ms_free(buffer);
	}

	friends = linphone_friend_list_get_friends(lfl);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends), 1000, unsigned int, "%u"); // Now that we accept Friends without a SIP URI, the result must be equal to 1000

	elapsed = (double)(end - start);
	ms_message("Imported a thousand of vCards from buffer in %f seconds", elapsed / CLOCKS_PER_SEC);

	linphone_friend_list_unref(lfl);

	bc_free(import_filepath);
	linphone_core_manager_destroy(manager);
}

#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic push
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4996)
#else
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

static void linphone_vcard_update_existing_friends_test(void) {
	LinphoneFriend *lf = linphone_friend_new_with_addr("sip:oldfriend@sip.linphone.org");

	BC_ASSERT_PTR_NOT_NULL(lf);
	if (linphone_core_vcard_supported()) {
		BC_ASSERT_PTR_NOT_NULL(linphone_friend_get_vcard(lf));
	} else {
		BC_ASSERT_PTR_NULL(linphone_friend_get_vcard(lf));
	}

	linphone_friend_edit(lf);
	linphone_friend_set_name(lf, "Old Friend");
	linphone_friend_done(lf);

	BC_ASSERT_PTR_NOT_NULL(linphone_friend_get_vcard(lf));
	BC_ASSERT_STRING_EQUAL(linphone_vcard_get_full_name(linphone_friend_get_vcard(lf)), "Old Friend");
	linphone_friend_unref(lf);
	lf = NULL;
}

#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic pop
#endif

static void linphone_vcard_phone_numbers_and_sip_addresses(void) {
	LinphoneCoreManager* manager = linphone_core_manager_new2("empty_rc", FALSE);
	LinphoneVcard *lvc = linphone_vcard_context_get_vcard_from_buffer(linphone_core_get_vcard_context(manager->lc), "BEGIN:VCARD\r\nVERSION:4.0\r\nFN:Sylvain Berfini\r\nIMPP:sip:sberfini@sip.linphone.org\r\nIMPP;TYPE=home:sip:sylvain@sip.linphone.org\r\nTEL;TYPE=work:0952636505\r\nEND:VCARD\r\n");
	LinphoneFriend *lf = linphone_friend_new_from_vcard(lvc);
	linphone_vcard_unref(lvc);
	const bctbx_list_t *sip_addresses = linphone_friend_get_addresses(lf);
	bctbx_list_t *phone_numbers = linphone_friend_get_phone_numbers(lf);
	LinphoneAddress *addr = NULL;

	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(sip_addresses), 2, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(phone_numbers), 1, unsigned int, "%u");
	if (phone_numbers) bctbx_list_free(phone_numbers);
	linphone_friend_unref(lf);

	lvc = linphone_vcard_context_get_vcard_from_buffer(linphone_core_get_vcard_context(manager->lc), "BEGIN:VCARD\r\nVERSION:4.0\r\nFN:Sylvain Berfini\r\nTEL;TYPE=work:0952636505\r\nTEL:0476010203\r\nEND:VCARD\r\n");
	lf = linphone_friend_new_from_vcard(lvc);
	linphone_vcard_unref(lvc);
	linphone_friend_set_core(lf, manager->lc);
	sip_addresses = linphone_friend_get_addresses(lf);
	phone_numbers = linphone_friend_get_phone_numbers(lf);

	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(sip_addresses), 0, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(phone_numbers), 2, unsigned int, "%u");
	if (phone_numbers) bctbx_list_free(phone_numbers);

	addr = linphone_address_new("sip:sylvain@sip.linphone.org");
	linphone_friend_add_address(lf, addr);
	sip_addresses = linphone_friend_get_addresses(lf);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(sip_addresses), 1, unsigned int, "%u");

	linphone_friend_remove_phone_number(lf, "0952636505");
	phone_numbers = linphone_friend_get_phone_numbers(lf);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(phone_numbers), 1, unsigned int, "%u");
	if (phone_numbers) bctbx_list_free(phone_numbers);

	linphone_friend_remove_phone_number(lf, "0476010203");
	phone_numbers = linphone_friend_get_phone_numbers(lf);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(phone_numbers), 0, unsigned int, "%u");
	if (phone_numbers) bctbx_list_free(phone_numbers);

	linphone_friend_edit(lf);
	linphone_friend_remove_address(lf, addr);
	linphone_friend_done(lf);
	sip_addresses = linphone_friend_get_addresses(lf);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(sip_addresses), 0, unsigned int, "%u");

	linphone_friend_add_phone_number(lf, "+33952636505");
	phone_numbers = linphone_friend_get_phone_numbers(lf);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(phone_numbers), 1, unsigned int, "%u");
	if (phone_numbers) bctbx_list_free(phone_numbers);

	linphone_address_unref(addr);
	linphone_friend_unref(lf);
	lf = NULL;
	lvc = NULL;
	linphone_core_manager_destroy(manager);
}

static void friends_if_no_db_set(void) {
	LinphoneCoreManager* manager = linphone_core_manager_new2("empty_rc", FALSE);
	LinphoneFriend *lf = linphone_core_create_friend(manager->lc);
	LinphoneAddress *addr = linphone_address_new("sip:sylvain@sip.linphone.org");
	const bctbx_list_t *friends = NULL;
	LinphoneFriendList *lfl = linphone_core_create_friend_list(manager->lc);

	linphone_friend_set_address(lf, addr);
	linphone_friend_set_name(lf, "Sylvain");
	linphone_friend_list_add_friend(lfl, lf);
	friends = linphone_friend_list_get_friends(lfl);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends), 1, unsigned int, "%u");

	linphone_friend_list_remove_friend(lfl, lf);
	linphone_friend_unref(lf);
	friends = linphone_friend_list_get_friends(lfl);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends), 0, unsigned int, "%u");

	linphone_friend_list_unref(lfl);
	linphone_address_unref(addr);
	linphone_core_manager_destroy(manager);
}

typedef struct _LinphoneFriendListStats {
	int new_list_count;
	int removed_list_count;
} LinphoneFriendListStats;

static void friend_list_created_cb(LinphoneCore *lc, LinphoneFriendList *list) {
	LinphoneFriendListStats *stats = (LinphoneFriendListStats *)linphone_friend_list_get_user_data(list);
	if (stats) {
		stats->new_list_count++;
	}
}

static void friend_list_removed_cb(LinphoneCore *lc, LinphoneFriendList *list) {
	LinphoneFriendListStats *stats = (LinphoneFriendListStats *)linphone_friend_list_get_user_data(list);
	if (stats) {
		stats->removed_list_count++;
	}
}

static void friends_sqlite_storage(void) {
	LinphoneCore* lc = NULL;
	LinphoneCoreCbs *cbs;
	LinphoneFriendList *lfl = NULL;
	LinphoneFriend *lf = NULL;
	LinphoneFriend *lf2 = NULL;
	LinphoneVcard *lvc = linphone_factory_create_vcard(linphone_factory_get());
	LinphoneAddress *addr = linphone_address_new("sip:sylvain@sip.linphone.org");
	const bctbx_list_t *friends = NULL;
	bctbx_list_t *friends_from_db = NULL;
	bctbx_list_t *friends_lists_from_db = NULL;
	char *friends_db = bc_tester_file("friends.db");
	LinphoneFriendListStats *stats = (LinphoneFriendListStats *)ms_new0(LinphoneFriendListStats, 1);
	const LinphoneAddress *laddress = NULL, *laddress2 = NULL;
	char *address = NULL, *address2 = NULL;

	cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_friend_list_created(cbs, friend_list_created_cb);
	linphone_core_cbs_set_friend_list_removed(cbs, friend_list_removed_cb);
	lc = linphone_factory_create_core_2(linphone_factory_get(), cbs, NULL, NULL, NULL, system_context);
	linphone_core_cbs_unref(cbs);
	friends = linphone_friend_list_get_friends(linphone_core_get_default_friend_list(lc));
	lfl = linphone_core_create_friend_list(lc);
	linphone_friend_list_set_user_data(lfl, stats);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends), 0, unsigned int, "%u");

	unlink(friends_db);
	linphone_core_set_friends_database_path(lc, friends_db);
	friends_from_db = linphone_core_fetch_friends_from_db(lc, linphone_core_get_default_friend_list(lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends_from_db), 0, unsigned int, "%u");

	linphone_vcard_set_etag(lvc, "\"123-456789\"");
	linphone_vcard_set_url(lvc, "http://dav.somewhere.fr/addressbook/me/someone.vcf");
	lf = linphone_friend_new_from_vcard(lvc);
	linphone_vcard_unref(lvc);
	linphone_friend_set_address(lf, addr);
	linphone_friend_set_name(lf, "Sylvain");

	linphone_core_add_friend_list(lc, lfl);
	wait_for_until(lc, NULL, &stats->new_list_count, 1, 1000);
	BC_ASSERT_EQUAL(stats->new_list_count, 1, int, "%i");
	linphone_friend_list_unref(lfl);
	linphone_friend_list_set_display_name(lfl, "Test");
	BC_ASSERT_EQUAL(linphone_friend_list_add_friend(lfl, lf), LinphoneFriendListOK, int, "%i");
	linphone_friend_unref(lf);
	BC_ASSERT_EQUAL(linphone_friend_list_get_storage_id(lfl), 1, unsigned int, "%u");
	BC_ASSERT_EQUAL(linphone_friend_get_storage_id(lf), 1, unsigned int, "%u");

	friends = linphone_friend_list_get_friends(linphone_core_get_default_friend_list(lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends), 0, unsigned int, "%u");

	friends_lists_from_db = linphone_core_fetch_friends_lists_from_db(lc);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends_lists_from_db), 1, unsigned int, "%u");
	friends_from_db = *linphone_friend_list_get_friends_attribute((LinphoneFriendList *)bctbx_list_get_data(friends_lists_from_db));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends_from_db), 1, unsigned int, "%u");
	lf2 = (LinphoneFriend *)friends_from_db->data;
	BC_ASSERT_PTR_NOT_NULL(linphone_friend_get_core(lf2));
	BC_ASSERT_PTR_NOT_NULL(linphone_friend_get_friend_list(lf2));
	friends_lists_from_db = bctbx_list_free_with_data(friends_lists_from_db, (void (*)(void *))linphone_friend_list_unref);

	friends_from_db = linphone_core_fetch_friends_from_db(lc, lfl);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends_from_db), 1, unsigned int, "%u");
	if (bctbx_list_size(friends_from_db) < 1) {
		goto end;
	}
	lf2 = (LinphoneFriend *)friends_from_db->data;
	BC_ASSERT_STRING_EQUAL(linphone_friend_get_name(lf2), linphone_friend_get_name(lf));
	BC_ASSERT_EQUAL(linphone_friend_get_storage_id(lf2), linphone_friend_get_storage_id(lf), unsigned int, "%u");
	BC_ASSERT_STRING_EQUAL(linphone_vcard_get_etag(linphone_friend_get_vcard(lf2)), linphone_vcard_get_etag(linphone_friend_get_vcard(lf)));
	BC_ASSERT_STRING_EQUAL(linphone_vcard_get_url(linphone_friend_get_vcard(lf2)), linphone_vcard_get_url(linphone_friend_get_vcard(lf)));
	laddress = linphone_friend_get_address(lf);
	address = linphone_address_as_string(laddress);
	laddress2 = linphone_friend_get_address(lf2);
	address2 = linphone_address_as_string(laddress2);
	BC_ASSERT_STRING_EQUAL(address2, address);

	ms_free(address);
	ms_free(address2);

	linphone_friend_edit(lf);
	linphone_friend_set_name(lf, "Margaux");
	linphone_friend_done(lf);
	friends_from_db = bctbx_list_free_with_data(friends_from_db, (void (*)(void *))linphone_friend_unref);
	friends_from_db = linphone_core_fetch_friends_from_db(lc, lfl);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends_from_db), 1, unsigned int, "%u");
	if (bctbx_list_size(friends_from_db) < 1) {
		goto end;
	}
	lf2 = (LinphoneFriend *)friends_from_db->data;
	BC_ASSERT_STRING_EQUAL(linphone_friend_get_name(lf2), "Margaux");
	friends_from_db = bctbx_list_free_with_data(friends_from_db, (void (*)(void *))linphone_friend_unref);

	linphone_friend_list_remove_friend(lfl, lf);
	friends = linphone_friend_list_get_friends(linphone_core_get_default_friend_list(lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends), 0, unsigned int, "%u");
	friends_from_db = linphone_core_fetch_friends_from_db(lc, lfl);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends_from_db), 0, unsigned int, "%u");
	friends_from_db = bctbx_list_free_with_data(friends_from_db, (void (*)(void *))linphone_friend_unref);

	linphone_core_remove_friend_list(lc, lfl);
	wait_for_until(lc, NULL, &stats->removed_list_count, 1, 1000);
	BC_ASSERT_EQUAL(stats->removed_list_count, 1, int, "%i");

end:
	ms_free(stats);
	linphone_address_unref(addr);
	linphone_core_unref(lc);
	unlink(friends_db);
	bc_free(friends_db);
}

static void friends_sqlite_store_lot_of_friends(void) {
	LinphoneCore* lc = linphone_factory_create_core_2(linphone_factory_get(), NULL, NULL, NULL, NULL, system_context);
	sqlite3 *db;
	int i;
	char* errmsg = NULL;
	int ret;
	char *buf;
	char *friends_db = bc_tester_file("friends.db");

	unlink(friends_db);
	ret = sqlite3_open(friends_db, &db);
	bc_free(friends_db);

	BC_ASSERT_TRUE(ret ==SQLITE_OK);
	ret = sqlite3_exec(db,"BEGIN",0,0,&errmsg);
	BC_ASSERT_TRUE(ret ==SQLITE_OK);

	ret = sqlite3_exec(db,
					   "CREATE TABLE IF NOT EXISTS friends ("
					   "id                INTEGER PRIMARY KEY AUTOINCREMENT,"
					   "friend_list_id    INTEGER,"
					   "sip_uri           TEXT,"
					   "subscribe_policy  INTEGER,"
					   "send_subscribe    INTEGER,"
					   "ref_key           TEXT,"
					   "vCard             TEXT,"
					   "vCard_etag        TEXT,"
					   "vCard_url         TEXT,"
					   "presence_received INTEGER"
					   ");", 0, 0, &errmsg);
	BC_ASSERT_TRUE(ret ==SQLITE_OK);
	ret = sqlite3_exec(db,"END",0,0,&errmsg);
	BC_ASSERT_TRUE(ret ==SQLITE_OK);

	ms_message("Start :\n");
	ret = sqlite3_exec(db,"BEGIN",0,0,&errmsg);
	BC_ASSERT_TRUE(ret ==SQLITE_OK);
	for (i = 0; i < 20000; i++) {
		buf = sqlite3_mprintf("INSERT INTO friends VALUES(NULL,%u,%Q,%i,%i,'key_%i',%Q,%Q,%Q,%i);",
							  i,
							  "dummy_addr",
							  0,
							  0,
							  i,
							  NULL,
							  NULL,
							  NULL,
							  0
							  );

		ret = sqlite3_exec(db,buf,0,0,&errmsg);
		BC_ASSERT_TRUE(ret ==SQLITE_OK);
		sqlite3_free(buf);
	}

	ret = sqlite3_exec(db,"END",0,0,&errmsg);
	BC_ASSERT_TRUE(ret ==SQLITE_OK);

	ms_message("End :\n");

	ret = sqlite3_exec(db,"BEGIN",0,0,&errmsg);
	BC_ASSERT_TRUE(ret ==SQLITE_OK);
	ret = sqlite3_exec(db, "DELETE FROM friends;",0,0,&errmsg);
	BC_ASSERT_TRUE(ret ==SQLITE_OK);
	ret = sqlite3_exec(db,"END",0,0,&errmsg);
	BC_ASSERT_TRUE(ret ==SQLITE_OK);
	sqlite3_close(db);
	linphone_core_unref(lc);
}

static void friends_sqlite_find_friend_in_lot_of_friends(void) {
	LinphoneCore* lc = linphone_factory_create_core_2(linphone_factory_get(), NULL, NULL, NULL, NULL, system_context);
	sqlite3 *db;
	int i;
	char* errmsg = NULL;
	int ret;
	char *buf;
	bctoolboxTimeSpec t1;
	bctoolboxTimeSpec t2;
	char *friends_db = bc_tester_file("friends.db");

	unlink(friends_db);
	ret = sqlite3_open(friends_db, &db);
	bc_free(friends_db);

	BC_ASSERT_TRUE(ret ==SQLITE_OK);
	ret = sqlite3_exec(db,"BEGIN",0,0,&errmsg);
	BC_ASSERT_TRUE(ret ==SQLITE_OK);

	ret = sqlite3_exec(db,
					   "CREATE TABLE IF NOT EXISTS friends ("
					   "id                INTEGER PRIMARY KEY AUTOINCREMENT,"
					   "friend_list_id    INTEGER,"
					   "sip_uri           TEXT,"
					   "subscribe_policy  INTEGER,"
					   "send_subscribe    INTEGER,"
					   "ref_key           TEXT,"
					   "vCard             TEXT,"
					   "vCard_etag        TEXT,"
					   "vCard_url         TEXT,"
					   "presence_received INTEGER"
					   ");", 0, 0, &errmsg);
	BC_ASSERT_TRUE(ret ==SQLITE_OK);
	ret = sqlite3_exec(db,"END",0,0,&errmsg);
	BC_ASSERT_TRUE(ret ==SQLITE_OK);

	ret = sqlite3_exec(db,"BEGIN",0,0,&errmsg);
	BC_ASSERT_TRUE(ret ==SQLITE_OK);
	for (i = 0; i < 20000; i++) {
		buf = sqlite3_mprintf("INSERT INTO friends VALUES(NULL,%u,%Q,%i,%i,'key_%i',%Q,%Q,%Q,%i);",
							  i,
							  "dummy_addr",
							  0,
							  0,
							  i,
							  NULL,
							  NULL,
							  NULL,
							  0
							  );

		ret = sqlite3_exec(db,buf,0,0,&errmsg);
		BC_ASSERT_TRUE(ret ==SQLITE_OK);
		sqlite3_free(buf);
		//ms_message("%i",i);
	}

	ret = sqlite3_exec(db,"END",0,0,&errmsg);
	BC_ASSERT_TRUE(ret ==SQLITE_OK);

	bctbx_get_cur_time(&t1);
	ms_message("Start : %li : %li\n", (long int)t1.tv_sec, (long int)t1.tv_nsec);
	for (i = 0; i < 20000; i++) {
		buf = sqlite3_mprintf("SELECT * FROM friends WHERE ref_key LIKE 'key_%i';",
							  i);

		ret = sqlite3_exec(db,buf,0,0,&errmsg);
		BC_ASSERT_TRUE(ret ==SQLITE_OK);
		sqlite3_free(buf);
	}

	bctbx_get_cur_time(&t2);
	ms_message("End : %li : %li\n", (long int)t2.tv_sec, (long int)t2.tv_nsec);

	ret = sqlite3_exec(db,"BEGIN",0,0,&errmsg);
	BC_ASSERT_TRUE(ret ==SQLITE_OK);
	ret = sqlite3_exec(db, "DELETE FROM friends;",0,0,&errmsg);
	BC_ASSERT_TRUE(ret ==SQLITE_OK);
	ret = sqlite3_exec(db,"END",0,0,&errmsg);
	BC_ASSERT_TRUE(ret ==SQLITE_OK);
	sqlite3_close(db);
	linphone_core_unref(lc);
}

typedef struct _LinphoneCardDAVStats {
	int sync_done_count;
	int new_contact_count;
	int removed_contact_count;
	int updated_contact_count;
} LinphoneCardDAVStats;

static void carddav_sync_done(LinphoneCardDavContext *c, bool_t success, const char *message) {
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)linphone_carddav_get_user_data(c);
	BC_ASSERT_TRUE(success);
	stats->sync_done_count++;
}

static void carddav_new_contact(LinphoneCardDavContext *c, LinphoneFriend *lf) {
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)linphone_carddav_get_user_data(c);
	BC_ASSERT_PTR_NOT_NULL(lf);
	stats->new_contact_count++;
}

static void carddav_removed_contact(LinphoneCardDavContext *c, LinphoneFriend *lf) {
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)linphone_carddav_get_user_data(c);
	BC_ASSERT_PTR_NOT_NULL(lf);
	stats->removed_contact_count++;
}

static void carddav_updated_contact(LinphoneCardDavContext *c, LinphoneFriend *new_lf, LinphoneFriend *old_lf) {
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)linphone_carddav_get_user_data(c);
	BC_ASSERT_PTR_NOT_NULL(new_lf);
	BC_ASSERT_PTR_NOT_NULL(old_lf);
	stats->updated_contact_count++;
}

static void carddav_sync(void) {
	LinphoneCoreManager *manager = linphone_core_manager_new2("carddav_rc", FALSE);
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)ms_new0(LinphoneCardDAVStats, 1);
	LinphoneFriendList *lfl = linphone_core_create_friend_list(manager->lc);
	LinphoneCardDavContext *c = NULL;

	linphone_friend_list_set_uri(lfl, CARDDAV_SERVER);
	linphone_core_add_friend_list(manager->lc, lfl);
	linphone_friend_list_unref(lfl);
	c = linphone_carddav_context_new(lfl);
	BC_ASSERT_PTR_NOT_NULL(c);

	linphone_carddav_set_user_data(c, stats);
	linphone_carddav_set_synchronization_done_callback(c, carddav_sync_done);
	linphone_carddav_set_new_contact_callback(c, carddav_new_contact);
	linphone_carddav_set_removed_contact_callback(c, carddav_removed_contact);
	linphone_carddav_set_updated_contact_callback(c, carddav_updated_contact);
	linphone_carddav_synchronize(c);

	wait_for_until(manager->lc, NULL, &stats->new_contact_count, 1, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->new_contact_count, 1, int, "%i");
	wait_for_until(manager->lc, NULL, &stats->sync_done_count, 1, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->sync_done_count, 1, int, "%i");

	ms_free(stats);
	linphone_carddav_context_destroy(c);
	linphone_core_manager_destroy(manager);
}

static void carddav_sync_2(void) {
	LinphoneCoreManager *manager = linphone_core_manager_new2("carddav_rc", FALSE);
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)ms_new0(LinphoneCardDAVStats, 1);
	LinphoneFriend *lf = linphone_core_create_friend_with_address(manager->lc, "\"Sylvain\" <sip:sylvain@sip.linphone.org>");
	char *friends_db = bc_tester_file("friends.db");
	LinphoneFriendList *lfl = linphone_core_create_friend_list(manager->lc);
	LinphoneCardDavContext *c = NULL;

	linphone_friend_list_set_uri(lfl, CARDDAV_SERVER);
	linphone_core_add_friend_list(manager->lc, lfl);
	linphone_friend_list_unref(lfl);
	c = linphone_carddav_context_new(lfl);
	BC_ASSERT_PTR_NOT_NULL(c);

	unlink(friends_db);
	linphone_core_set_friends_database_path(manager->lc, friends_db);
	BC_ASSERT_EQUAL(linphone_friend_list_add_local_friend(lfl, lf), LinphoneFriendListOK, int, "%d");
	linphone_friend_unref(lf);

	linphone_carddav_set_user_data(c, stats);
	linphone_carddav_set_synchronization_done_callback(c, carddav_sync_done);
	linphone_carddav_set_new_contact_callback(c, carddav_new_contact);
	linphone_carddav_set_removed_contact_callback(c, carddav_removed_contact);
	linphone_carddav_set_updated_contact_callback(c, carddav_updated_contact);

	linphone_carddav_synchronize(c);

	wait_for_until(manager->lc, NULL, &stats->new_contact_count, 1, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->new_contact_count, 1, int, "%i");
	wait_for_until(manager->lc, NULL, &stats->removed_contact_count, 1, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->removed_contact_count, 1, int, "%i");
	wait_for_until(manager->lc, NULL, &stats->sync_done_count, 1, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->sync_done_count, 1, int, "%i");

	ms_free(stats);
	linphone_carddav_context_destroy(c);
	linphone_core_manager_destroy(manager);
	unlink(friends_db);
	bc_free(friends_db);
}

static void carddav_sync_3(void) {
	LinphoneCoreManager *manager = linphone_core_manager_new2("carddav_rc", FALSE);
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)ms_new0(LinphoneCardDAVStats, 1);
	LinphoneVcard *lvc = linphone_vcard_context_get_vcard_from_buffer(linphone_core_get_vcard_context(manager->lc), "BEGIN:VCARD\r\nVERSION:4.0\r\nUID:1f08dd48-29ac-4097-8e48-8596d7776283\r\nFN:Sylvain Berfini\r\nIMPP;TYPE=work:sip:sylvain@sip.linphone.org\r\nEND:VCARD\r\n");
	LinphoneFriend *lf = linphone_friend_new_from_vcard(lvc);
	char *friends_db = bc_tester_file("friends.db");
	LinphoneFriendList *lfl = linphone_core_create_friend_list(manager->lc);
	LinphoneCardDavContext *c = NULL;

	linphone_vcard_unref(lvc);
	linphone_friend_list_set_uri(lfl, CARDDAV_SERVER);
	linphone_core_add_friend_list(manager->lc, lfl);
	linphone_friend_list_unref(lfl);
	c = linphone_carddav_context_new(lfl);
	BC_ASSERT_PTR_NOT_NULL(c);

	unlink(friends_db);
	linphone_core_set_friends_database_path(manager->lc, friends_db);
	BC_ASSERT_EQUAL(linphone_friend_list_add_local_friend(lfl, lf), LinphoneFriendListOK, int, "%d");
	linphone_friend_unref(lf);

	linphone_carddav_set_user_data(c, stats);
	linphone_carddav_set_synchronization_done_callback(c, carddav_sync_done);
	linphone_carddav_set_new_contact_callback(c, carddav_new_contact);
	linphone_carddav_set_removed_contact_callback(c, carddav_removed_contact);
	linphone_carddav_set_updated_contact_callback(c, carddav_updated_contact);

	linphone_carddav_synchronize(c);

	wait_for_until(manager->lc, NULL, &stats->updated_contact_count, 1, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->updated_contact_count, 1, int, "%i");
	wait_for_until(manager->lc, NULL, &stats->sync_done_count, 1, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->sync_done_count, 1, int, "%i");

	ms_free(stats);
	linphone_carddav_context_destroy(c);
	linphone_core_manager_destroy(manager);
	unlink(friends_db);
	bc_free(friends_db);
}

static void carddav_sync_4(void) {
	LinphoneCoreManager *manager = linphone_core_manager_new2("carddav_rc", FALSE);
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)ms_new0(LinphoneCardDAVStats, 1);
	LinphoneVcard *lvc = linphone_vcard_context_get_vcard_from_buffer(linphone_core_get_vcard_context(manager->lc), "BEGIN:VCARD\r\nVERSION:4.0\r\nFN:Margaux Clerc\r\nIMPP;TYPE=work:sip:margaux@sip.linphone.org\r\nEND:VCARD\r\n");
	LinphoneFriend *lf = linphone_friend_new_from_vcard(lvc);
	LinphoneFriendList *lfl = linphone_core_create_friend_list(manager->lc);
	LinphoneCardDavContext *c = NULL;

	linphone_vcard_unref(lvc);
	linphone_friend_list_set_uri(lfl, CARDDAV_SERVER);
	linphone_core_add_friend_list(manager->lc, lfl);
	linphone_friend_list_unref(lfl);
	c = linphone_carddav_context_new(lfl);
	BC_ASSERT_PTR_NOT_NULL(c);

	linphone_carddav_set_user_data(c, stats);
	linphone_carddav_set_synchronization_done_callback(c, carddav_sync_done);
	linphone_carddav_set_new_contact_callback(c, carddav_new_contact);
	linphone_carddav_set_removed_contact_callback(c, carddav_removed_contact);
	linphone_carddav_set_updated_contact_callback(c, carddav_updated_contact);

	BC_ASSERT_PTR_NULL(linphone_vcard_get_uid(lvc));
	linphone_carddav_put_vcard(c, lf);
	BC_ASSERT_PTR_NOT_NULL(linphone_vcard_get_uid(lvc));
	wait_for_until(manager->lc, NULL, &stats->sync_done_count, 1, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->sync_done_count, 1, int, "%i");

	linphone_carddav_delete_vcard(c, lf);
	wait_for_until(manager->lc, NULL, &stats->sync_done_count, 2, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->sync_done_count, 2, int, "%i");

	linphone_friend_unref(lf);
	ms_free(stats);
	linphone_carddav_context_destroy(c);
	linphone_core_manager_destroy(manager);
}

static void carddav_contact_created(LinphoneFriendList *list, LinphoneFriend *lf) {
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)linphone_friend_list_cbs_get_user_data(linphone_friend_list_get_callbacks(list));
	stats->new_contact_count++;
}

static void carddav_contact_deleted(LinphoneFriendList *list, LinphoneFriend *lf) {
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)linphone_friend_list_cbs_get_user_data(linphone_friend_list_get_callbacks(list));
	stats->removed_contact_count++;
}

static void carddav_contact_updated(LinphoneFriendList *list, LinphoneFriend *new_friend, LinphoneFriend *old_friend) {
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)linphone_friend_list_cbs_get_user_data(linphone_friend_list_get_callbacks(list));
	BC_ASSERT_STRING_EQUAL(linphone_vcard_get_full_name(linphone_friend_get_vcard(new_friend)), linphone_vcard_get_full_name(linphone_friend_get_vcard(old_friend)));
	stats->updated_contact_count++;
}

static void carddav_sync_status_changed(LinphoneFriendList *list, LinphoneFriendListSyncStatus status, const char *msg) {
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)linphone_friend_list_cbs_get_user_data(linphone_friend_list_get_callbacks(list));
	char *state = status == LinphoneFriendListSyncStarted ? "Sync started" : (status == LinphoneFriendListSyncFailure ? "Sync failure" : "Sync successful");
	ms_message("[CardDAV] %s : %s", state, msg);
	if (status == LinphoneFriendListSyncFailure || status == LinphoneFriendListSyncSuccessful) {
		stats->sync_done_count++;
	}
}

static void carddav_integration(void) {
	LinphoneCoreManager *manager = linphone_core_manager_new2("carddav_rc", FALSE);
	LinphoneFriendList *lfl = linphone_core_create_friend_list(manager->lc);
	LinphoneVcard *lvc = linphone_vcard_context_get_vcard_from_buffer(linphone_core_get_vcard_context(manager->lc), "BEGIN:VCARD\r\nVERSION:4.0\r\nFN:Margaux Clerc\r\nIMPP;TYPE=work:sip:margaux@sip.linphone.org\r\nEND:VCARD\r\n");
	LinphoneFriend *lf = linphone_friend_new_from_vcard(lvc);
	LinphoneVcard *lvc2 = NULL;
	LinphoneFriend *lf2 = NULL;
	LinphoneFriendListCbs *cbs = NULL;
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)ms_new0(LinphoneCardDAVStats, 1);
	const char *refkey = "toto";
	char *address = NULL;
	const LinphoneAddress *addr;

	linphone_vcard_unref(lvc);
	linphone_friend_list_set_uri(lfl, CARDDAV_SERVER);
	cbs = linphone_friend_list_get_callbacks(lfl);
	linphone_friend_list_cbs_set_user_data(cbs, stats);
	linphone_friend_list_cbs_set_contact_created(cbs, carddav_contact_created);
	linphone_friend_list_cbs_set_contact_deleted(cbs, carddav_contact_deleted);
	linphone_friend_list_cbs_set_contact_updated(cbs, carddav_contact_updated);
	linphone_friend_list_cbs_set_sync_status_changed(cbs, carddav_sync_status_changed);
	linphone_core_add_friend_list(manager->lc, lfl);

	BC_ASSERT_PTR_NULL(linphone_vcard_get_uid(lvc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_friend_list_get_dirty_friends_to_update(lfl)), 0, unsigned int, "%u");
	BC_ASSERT_EQUAL(linphone_friend_list_add_friend(lfl, lf), LinphoneFriendListOK, int, "%d");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_friend_list_get_dirty_friends_to_update(lfl)), 1, unsigned int, "%u");
	wait_for_until(manager->lc, NULL, &stats->sync_done_count, 1, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->sync_done_count, 1, int, "%i");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_friend_list_get_dirty_friends_to_update(lfl)), 0, unsigned int, "%u");
	BC_ASSERT_PTR_NOT_NULL(linphone_vcard_get_uid(lvc));
	linphone_friend_list_remove_friend(lfl, lf);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_friend_list_get_friends(lfl)), 0, unsigned int, "%u");
	wait_for_until(manager->lc, NULL, &stats->sync_done_count, 2, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->sync_done_count, 2, int, "%i");
	linphone_friend_unref(lf);
	lf = NULL;

	lvc = linphone_vcard_context_get_vcard_from_buffer(linphone_core_get_vcard_context(manager->lc), "BEGIN:VCARD\r\nVERSION:4.0\r\nFN:Ghislain Mary\r\nIMPP;TYPE=work:sip:ghislain@sip.linphone.org\r\nEND:VCARD\r\n");
	lf = linphone_friend_new_from_vcard(lvc);
	linphone_vcard_unref(lvc);
	BC_ASSERT_EQUAL(linphone_friend_list_add_local_friend(lfl, lf), LinphoneFriendListOK, int, "%d");
	linphone_friend_unref(lf);

	lvc2 = linphone_vcard_context_get_vcard_from_buffer(linphone_core_get_vcard_context(manager->lc), "BEGIN:VCARD\r\nVERSION:4.0\r\nFN:Sylvain Berfini\r\nIMPP:sip:sberfini@sip.linphone.org\r\nUID:1f08dd48-29ac-4097-8e48-8596d7776283\r\nEND:VCARD\r\n");
	linphone_vcard_set_url(lvc2, "/card.php/addressbooks/tester/default/me.vcf");
	lf2 = linphone_friend_new_from_vcard(lvc2);
	linphone_vcard_unref(lvc2);
	linphone_friend_set_ref_key(lf2, refkey);
	BC_ASSERT_EQUAL(linphone_friend_list_add_local_friend(lfl, lf2), LinphoneFriendListOK, int, "%d");

	BC_ASSERT_EQUAL(linphone_friend_list_get_revision(lfl), 0, int, "%i");
	linphone_friend_list_synchronize_friends_from_server(lfl);
	wait_for_until(manager->lc, NULL, &stats->new_contact_count, 0, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->new_contact_count, 0, int, "%i");
	wait_for_until(manager->lc, NULL, &stats->removed_contact_count, 1, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->removed_contact_count, 1, int, "%i");
	wait_for_until(manager->lc, NULL, &stats->updated_contact_count, 1, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->updated_contact_count, 1, int, "%i");
	BC_ASSERT_NOT_EQUAL(linphone_friend_list_get_revision(lfl), 0, int, "%i");
	wait_for_until(manager->lc, NULL, &stats->sync_done_count, 3, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->sync_done_count, 3, int, "%i");

	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_friend_list_get_friends(lfl)), 1, unsigned int, "%u");
	lf = (LinphoneFriend *)bctbx_list_get_data((linphone_friend_list_get_friends(lfl)));
	BC_ASSERT_STRING_EQUAL(linphone_friend_get_ref_key(lf), refkey);
	BC_ASSERT_EQUAL(linphone_friend_get_storage_id(lf), linphone_friend_get_storage_id(lf2), unsigned int, "%u");
	linphone_friend_unref(lf2);
	addr = linphone_friend_get_address(lf);
	BC_ASSERT_PTR_NOT_NULL(addr);
	address = linphone_address_as_string_uri_only(addr);
	BC_ASSERT_STRING_EQUAL(address, "sip:sylvain@sip.linphone.org");
	ms_free(address);


	linphone_friend_edit(lf);
	linphone_friend_done(lf);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_friend_list_get_dirty_friends_to_update(linphone_friend_get_friend_list(lf))), 0, unsigned int, "%u");

	linphone_core_set_network_reachable(manager->lc, FALSE); //To prevent the CardDAV update
	linphone_friend_edit(lf);
	linphone_friend_set_name(lf, "François Grisez");
	linphone_friend_done(lf);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_friend_list_get_dirty_friends_to_update(linphone_friend_get_friend_list(lf))), 1, unsigned int, "%u");

	linphone_friend_list_unref(lfl);
	linphone_core_manager_destroy(manager);
	ms_free(stats);
}

static void carddav_clean(void) {  // This is to ensure the content of the test addressbook is in the correct state for the following tests
	LinphoneCoreManager *manager = linphone_core_manager_new2("carddav_rc", FALSE);
	LinphoneFriendList *lfl = linphone_core_create_friend_list(manager->lc);
	LinphoneFriendListCbs *cbs = linphone_friend_list_get_callbacks(lfl);
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)ms_new0(LinphoneCardDAVStats, 1);
	bctbx_list_t *friends = NULL;
	bctbx_list_t *friends_iterator = NULL;
	LinphoneFriend *lf = NULL;
	LinphoneVcard *lvc = NULL;

	linphone_friend_list_cbs_set_user_data(cbs, stats);
	linphone_friend_list_cbs_set_contact_created(cbs, carddav_contact_created);
	linphone_friend_list_cbs_set_contact_deleted(cbs, carddav_contact_deleted);
	linphone_friend_list_cbs_set_contact_updated(cbs, carddav_contact_updated);
	linphone_friend_list_cbs_set_sync_status_changed(cbs, carddav_sync_status_changed);
	linphone_core_add_friend_list(manager->lc, lfl);
	linphone_friend_list_set_uri(lfl, CARDDAV_SERVER);

	linphone_friend_list_synchronize_friends_from_server(lfl);
	wait_for_until(manager->lc, NULL, &stats->sync_done_count, 1, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->sync_done_count, 1, int, "%i");
	stats->sync_done_count = 0;

	friends = bctbx_list_copy(linphone_friend_list_get_friends(lfl));
	friends_iterator = friends;
	while (friends_iterator) {
		LinphoneFriend *lf = (LinphoneFriend *)friends_iterator->data;
		linphone_friend_list_remove_friend(lfl, lf);
		wait_for_until(manager->lc, NULL, &stats->sync_done_count, 1, CARDDAV_SYNC_TIMEOUT);
		BC_ASSERT_EQUAL(stats->sync_done_count, 1, int, "%i");
		stats->sync_done_count = 0;
		stats->removed_contact_count = 0;
		friends_iterator = bctbx_list_next(friends_iterator);
	}
	bctbx_list_free(friends);

	lvc = linphone_vcard_context_get_vcard_from_buffer(linphone_core_get_vcard_context(manager->lc), "BEGIN:VCARD\r\nVERSION:4.0\r\nFN:Sylvain Berfini\r\nIMPP:sip:sylvain@sip.linphone.org\r\nUID:1f08dd48-29ac-4097-8e48-8596d7776283\r\nEND:VCARD\r\n");
	linphone_vcard_set_url(lvc, "http://dav.linphone.org/card.php/addressbooks/tester/default/me.vcf");
	lf = linphone_friend_new_from_vcard(lvc);
	linphone_vcard_unref(lvc);
	linphone_friend_list_add_friend(lfl, lf);
	wait_for_until(manager->lc, NULL, &stats->sync_done_count, 1, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->sync_done_count, 1, int, "%i");

	ms_free(stats);
	linphone_friend_unref(lf);
	linphone_friend_list_unref(lfl);
	linphone_core_manager_destroy(manager);
}

static void carddav_multiple_sync(void) {
	LinphoneCoreManager *manager = linphone_core_manager_new2("carddav_rc", FALSE);
	LinphoneFriendList *lfl = linphone_core_create_friend_list(manager->lc);
	LinphoneFriendListCbs *cbs = linphone_friend_list_get_callbacks(lfl);
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)ms_new0(LinphoneCardDAVStats, 1);

	linphone_friend_list_cbs_set_user_data(cbs, stats);
	linphone_friend_list_cbs_set_contact_created(cbs, carddav_contact_created);
	linphone_friend_list_cbs_set_contact_deleted(cbs, carddav_contact_deleted);
	linphone_friend_list_cbs_set_contact_updated(cbs, carddav_contact_updated);
	linphone_friend_list_cbs_set_sync_status_changed(cbs, carddav_sync_status_changed);
	linphone_core_add_friend_list(manager->lc, lfl);
	linphone_friend_list_set_uri(lfl, CARDDAV_SERVER);

	linphone_friend_list_synchronize_friends_from_server(lfl);
	wait_for_until(manager->lc, NULL, &stats->sync_done_count, 1, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->sync_done_count, 1, int, "%i");
	linphone_friend_list_synchronize_friends_from_server(lfl);
	wait_for_until(manager->lc, NULL, &stats->sync_done_count, 2, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->sync_done_count, 2, int, "%i");
	linphone_friend_list_synchronize_friends_from_server(lfl);
	wait_for_until(manager->lc, NULL, &stats->sync_done_count, 3, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->sync_done_count, 3, int, "%i");
	BC_ASSERT_EQUAL(stats->removed_contact_count, 0, int, "%i");

	ms_free(stats);
	linphone_friend_list_unref(lfl);
	linphone_core_manager_destroy(manager);
}

static void carddav_server_to_client_and_client_to_sever_sync(void) {
	LinphoneCoreManager *manager = linphone_core_manager_new2("carddav_rc", FALSE);
	LinphoneFriendList *lfl = linphone_core_create_friend_list(manager->lc);
	LinphoneFriendListCbs *cbs = linphone_friend_list_get_callbacks(lfl);
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)ms_new0(LinphoneCardDAVStats, 1);
	LinphoneVcard *lvc1 = linphone_vcard_context_get_vcard_from_buffer(linphone_core_get_vcard_context(manager->lc), "BEGIN:VCARD\r\nVERSION:4.0\r\nFN:Margaux Clerc\r\nIMPP;TYPE=work:sip:margaux@sip.linphone.org\r\nEND:VCARD\r\n");
	LinphoneFriend *lf1 = linphone_friend_new_from_vcard(lvc1);
	LinphoneVcard *lvc2 = linphone_vcard_context_get_vcard_from_buffer(linphone_core_get_vcard_context(manager->lc), "BEGIN:VCARD\r\nVERSION:4.0\r\nFN:Ghislain Mary\r\nIMPP;TYPE=work:sip:ghislain@sip.linphone.org\r\nEND:VCARD\r\n");
	LinphoneFriend *lf2 = linphone_friend_new_from_vcard(lvc2);
	bctbx_list_t *friends = NULL, *friends_iterator = NULL;

	linphone_vcard_unref(lvc1);
	linphone_vcard_unref(lvc2);
	linphone_friend_list_cbs_set_user_data(cbs, stats);
	linphone_friend_list_cbs_set_contact_created(cbs, carddav_contact_created);
	linphone_friend_list_cbs_set_contact_deleted(cbs, carddav_contact_deleted);
	linphone_friend_list_cbs_set_contact_updated(cbs, carddav_contact_updated);
	linphone_friend_list_cbs_set_sync_status_changed(cbs, carddav_sync_status_changed);
	linphone_core_add_friend_list(manager->lc, lfl);
	linphone_friend_list_set_uri(lfl, CARDDAV_SERVER);

	linphone_friend_list_add_friend(lfl, lf1);
	linphone_friend_unref(lf1);
	linphone_friend_list_synchronize_friends_from_server(lfl);
	linphone_friend_list_add_friend(lfl, lf2);
	linphone_friend_unref(lf2);
	wait_for_until(manager->lc, NULL, &stats->sync_done_count, 3, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->sync_done_count, 3, int, "%i");

	stats->sync_done_count = 0;
	friends = bctbx_list_copy(linphone_friend_list_get_friends(lfl));
	friends_iterator = friends;
	while (friends_iterator) {
		LinphoneFriend *lf = (LinphoneFriend *)friends_iterator->data;
		if (lf && strcmp(linphone_friend_get_name(lf), "Sylvain Berfini") != 0) {
			linphone_friend_list_remove_friend(lfl, lf);
			wait_for_until(manager->lc, NULL, &stats->sync_done_count, 1, CARDDAV_SYNC_TIMEOUT);
			BC_ASSERT_EQUAL(stats->sync_done_count, 1, int, "%i");
			stats->sync_done_count = 0;
		}
		friends_iterator = bctbx_list_next(friends_iterator);
	}
	bctbx_list_free(friends);

	ms_free(stats);
	linphone_friend_list_unref(lfl);
	linphone_core_manager_destroy(manager);
}

static void find_friend_by_ref_key_test(void) {
	LinphoneCoreManager* manager = linphone_core_manager_new2("empty_rc", FALSE);
	LinphoneFriendList *lfl = linphone_core_get_default_friend_list(manager->lc);
	LinphoneFriend *lf = linphone_core_create_friend_with_address(manager->lc, "sip:toto@sip.linphone.org");
	LinphoneFriend *lf2 = NULL;
	const LinphoneAddress *addr = NULL;
	linphone_friend_set_ref_key(lf, "totorefkey");
	linphone_friend_list_add_friend(lfl, lf);
	lf2 = linphone_friend_list_find_friend_by_ref_key(lfl, "totorefkey");
	BC_ASSERT_PTR_NOT_NULL(lf2);
	if (!lf2) {
		goto end;
	}
	addr = linphone_friend_get_address(lf2);
	char *uri_addr = linphone_address_as_string_uri_only(addr);
	BC_ASSERT_STRING_EQUAL(uri_addr, "sip:toto@sip.linphone.org");
	bctbx_free(uri_addr);
	BC_ASSERT_EQUAL(lf2, lf, void*, "%p");
end:
	linphone_friend_unref(lf);
	linphone_core_manager_destroy(manager);
}

static void insert_lot_of_friends_map_test(void) {
	int i;
	bctbx_map_t *friends_map = bctbx_mmap_cchar_new();
	bctbx_pair_cchar_t *pair;

	char key[64];
	ms_message("Start\n");
	for(i = 0; i < 20000; i++) {
		snprintf(key, sizeof(key),"key_%i",i);
		pair = bctbx_pair_cchar_new(key,(void*)(uintptr_t)i);
		bctbx_map_cchar_insert_and_delete(friends_map, (bctbx_pair_t*)pair);
	}
	ms_message("End\n");
	bctbx_mmap_cchar_delete(friends_map);
}

static void find_friend_by_ref_key_in_lot_of_friends_test(void) {
	int i;
	int j;
	bctbx_map_t *friends_map = bctbx_mmap_cchar_new();
	bctbx_pair_cchar_t *pair;
	bctbx_iterator_t *it;
	bctoolboxTimeSpec t1;
	bctoolboxTimeSpec t2;
	char key[64];
	for(i = 0; i < 20000; i++) {
		snprintf(key, sizeof(key),"key_%i",i);
		pair = bctbx_pair_cchar_new(key,(void*)(uintptr_t)i);
		bctbx_map_cchar_insert_and_delete(friends_map, (bctbx_pair_t*)pair);
	}
	bctbx_get_cur_time(&t1);
	ms_message("Start : %li : %li\n", (long int)t1.tv_sec, (long int)t1.tv_nsec);
	for(i = 0; i < 20000; i++) {
		snprintf(key, sizeof(key),"key_%i",i);
		it = bctbx_map_cchar_find_key(friends_map, key);
		j = (int)(uintptr_t)bctbx_pair_cchar_get_second(bctbx_iterator_cchar_get_pair(it));
		BC_ASSERT_TRUE(i == j);
		bctbx_iterator_cchar_delete(it);
	}
	bctbx_get_cur_time(&t2);
	ms_message("End : %li : %li\n", (long int)t2.tv_sec, (long int)t2.tv_nsec);
	bctbx_mmap_cchar_delete(friends_map);
}

static void find_friend_by_ref_key_empty_list_test(void) {
	LinphoneCoreManager* manager = linphone_core_manager_new2("empty_rc", FALSE);
	LinphoneFriendList *lfl = linphone_core_get_default_friend_list(manager->lc);
	LinphoneFriend *lf2;
	lf2 = linphone_friend_list_find_friend_by_ref_key(lfl, "totorefkey");
	BC_ASSERT_PTR_NULL(lf2);
	if (lf2) {
		goto end;
	}
end:
	linphone_core_manager_destroy(manager);
}

test_t vcard_tests[] = {
	TEST_NO_TAG("Import / Export friends from vCards", linphone_vcard_import_export_friends_test),
	TEST_NO_TAG("Import a lot of friends from vCards", linphone_vcard_import_a_lot_of_friends_test),
	TEST_NO_TAG("vCard creation for existing friends", linphone_vcard_update_existing_friends_test),
	TEST_NO_TAG("vCard phone numbers and SIP addresses", linphone_vcard_phone_numbers_and_sip_addresses),
	TEST_NO_TAG("Friends working if no db set", friends_if_no_db_set),
	TEST_NO_TAG("Friends storage in sqlite database", friends_sqlite_storage),
	TEST_NO_TAG("20000 Friends storage in sqlite database", friends_sqlite_store_lot_of_friends),
	TEST_NO_TAG("Find friend in database of 20000 objects", friends_sqlite_find_friend_in_lot_of_friends),
	TEST_NO_TAG("CardDAV clean", carddav_clean), // This is to ensure the content of the test addressbook is in the correct state for the following tests
	TEST_NO_TAG("CardDAV synchronization", carddav_sync),
	TEST_NO_TAG("CardDAV synchronization 2", carddav_sync_2),
	TEST_NO_TAG("CardDAV synchronization 3", carddav_sync_3),
	TEST_NO_TAG("CardDAV synchronization 4", carddav_sync_4),
	TEST_NO_TAG("CardDAV integration", carddav_integration),
	TEST_NO_TAG("CardDAV multiple synchronizations", carddav_multiple_sync),
	TEST_NO_TAG("CardDAV client to server and server to client sync", carddav_server_to_client_and_client_to_sever_sync),
	TEST_NO_TAG("Find friend by ref key", find_friend_by_ref_key_test),
	TEST_NO_TAG("create a map and insert 20000 objects", insert_lot_of_friends_map_test),
	TEST_NO_TAG("Find ref key in 20000 objects map", find_friend_by_ref_key_in_lot_of_friends_test),
	TEST_NO_TAG("Find friend by ref key in empty list", find_friend_by_ref_key_empty_list_test)
};

test_suite_t vcard_test_suite = {
	"VCard", NULL, NULL,
	liblinphone_tester_before_each, liblinphone_tester_after_each,
	sizeof(vcard_tests) / sizeof(vcard_tests[0]), vcard_tests
};

#endif
