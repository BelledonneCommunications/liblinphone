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

#include "friend/friend-list.h"
#include "friend/friend.h"
#ifdef VCARD_ENABLED
#include <time.h>

#include <bctoolbox/defs.h>
#include <bctoolbox/map.h>

#include "liblinphone_tester.h"
#include "linphone/api/c-address.h"
#include "linphone/core.h"
#include "tester_utils.h"
#include "vcard/carddav-context.h"

#define CARDDAV_SERVER "http://dav.example.org/baikal/html/card.php"
#define CARDDAV_SERVER_WITH_PORT "http://dav.example.org:80/baikal/html/card.php"
#define ME_VCF "http://dav.example.org/baikal/html/card.php/addressbooks/tester/default/me.vcf"
#define ME_VCF_2 "/baikal/html/card.php/addressbooks/tester/default/me.vcf"
#define ME_VCF_3 "/baikal/html/card.php/addressbooks/tester/default/unknown.vcf"
#define CARDDAV_SYNC_TIMEOUT 15000

using namespace LinphonePrivate;

const char *vcard_friends_db_file = "vcard-friends.db";

static void linphone_vcard_import_export_friends_test(void) {
	LinphoneCoreManager *manager = linphone_core_manager_new_with_proxies_check("empty_rc", FALSE);
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
	const bctbx_list_t *it;
	for (it = friends; it != NULL; it = it->next) {
		LinphoneFriend *lfriend = (LinphoneFriend *)it->data;
		if (strcmp(linphone_friend_get_name(lfriend), "Sylvain Berfini") == 0) {
			BC_ASSERT_TRUE(linphone_friend_get_starred(lfriend));
			linphone_friend_set_starred(lfriend, FALSE);
		} else {
			BC_ASSERT_FALSE(linphone_friend_get_starred(lfriend));
		}
	}

	linphone_friend_list_export_friends_as_vcard4_file(lfl, export_filepath);

	lfl = linphone_core_create_friend_list(manager->lc);
	count = linphone_friend_list_import_friends_from_vcard4_file(lfl, export_filepath);
	BC_ASSERT_EQUAL(count, 3, int, "%d");
	friends = linphone_friend_list_get_friends(lfl);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends), 3, unsigned int, "%u");
	for (it = friends; it != NULL; it = it->next) {
		const LinphoneFriend *lfriend = (LinphoneFriend *)it->data;
		BC_ASSERT_FALSE(linphone_friend_get_starred(lfriend));
	}
	linphone_friend_list_unref(lfl);

	remove(export_filepath);
	bc_free(import_filepath);
	bc_free(export_filepath);
	linphone_core_manager_destroy(manager);
}

static void linphone_vcard_import_a_lot_of_friends_test(void) {
	LinphoneCoreManager *manager = linphone_core_manager_new_with_proxies_check("empty_rc", FALSE);
	LinphoneFriendList *lfl = linphone_core_get_default_friend_list(manager->lc);
	char *import_filepath = bc_tester_res("vcards/thousand_vcards.vcf");
	clock_t start, end;
	double elapsed = 0;
	const bctbx_list_t *friends = NULL;
	FILE *infile = NULL;
	char *buffer = NULL;
	long numbytes = 0;
	size_t readbytes;

	start = clock();
	linphone_friend_list_import_friends_from_vcard4_file(lfl, import_filepath);
	end = clock();

	friends = linphone_friend_list_get_friends(lfl);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends), 1000, unsigned int,
	                "%u"); // Now that we accept Friends without a SIP URI, the result must be equal to 1000

	elapsed = (double)(end - start);
	ms_message("Imported a thousand of vCards from file in %f seconds", elapsed / CLOCKS_PER_SEC);

	lfl = linphone_core_create_friend_list(manager->lc);
	infile = fopen(import_filepath, "rb");
	BC_ASSERT_PTR_NOT_NULL(infile);
	if (infile) {
		fseek(infile, 0L, SEEK_END);
		numbytes = ftell(infile);
		fseek(infile, 0L, SEEK_SET);
		buffer = (char *)ms_malloc((numbytes + 1) * sizeof(char));
		readbytes = fread(buffer, sizeof(char), numbytes, infile);
		fclose(infile);
		buffer[readbytes] = '\0';

		start = clock();
		linphone_friend_list_import_friends_from_vcard4_buffer(lfl, buffer);
		end = clock();
		ms_free(buffer);
	}

	friends = linphone_friend_list_get_friends(lfl);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends), 1000, unsigned int,
	                "%u"); // Now that we accept Friends without a SIP URI, the result must be equal to 1000

	elapsed = (double)(end - start);
	ms_message("Imported a thousand of vCards from buffer in %f seconds", elapsed / CLOCKS_PER_SEC);

	linphone_friend_list_unref(lfl);

	bc_free(import_filepath);
	linphone_core_manager_destroy(manager);
}

static void linphone_vcard_update_existing_friends_test(void) {
	LinphoneCoreManager *manager = linphone_core_manager_new_with_proxies_check("empty_rc", FALSE);
	LinphoneFriend *lf = linphone_core_create_friend_with_address(manager->lc, "sip:oldfriend@sip.linphone.org");

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
	linphone_core_manager_destroy(manager);
}

static void linphone_vcard_phone_numbers_and_sip_addresses(void) {
	LinphoneCoreManager *manager = linphone_core_manager_new_with_proxies_check("empty_rc", FALSE);
	LinphoneVcard *lvc = linphone_vcard_context_get_vcard_from_buffer(
	    linphone_core_get_vcard_context(manager->lc),
	    "BEGIN:VCARD\r\nVERSION:4.0\r\nFN:Sylvain "
	    "Berfini\r\nIMPP:sip:sberfini@sip.linphone.org\r\nIMPP;TYPE=home:sip:sylvain@sip.linphone.org\r\nTEL;TYPE=work:"
	    "0952636505\r\nEND:VCARD\r\n");
	LinphoneFriend *lf = linphone_core_create_friend_from_vcard(manager->lc, lvc);
	linphone_vcard_unref(lvc);
	const bctbx_list_t *sip_addresses = linphone_friend_get_addresses(lf);
	bctbx_list_t *phone_numbers = linphone_friend_get_phone_numbers(lf);
	LinphoneAddress *addr = NULL;

	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(sip_addresses), 2, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(phone_numbers), 1, unsigned int, "%u");
	if (phone_numbers) bctbx_list_free_with_data(phone_numbers, (bctbx_list_free_func)bctbx_free);
	linphone_friend_unref(lf);

	lvc = linphone_vcard_context_get_vcard_from_buffer(
	    linphone_core_get_vcard_context(manager->lc),
	    "BEGIN:VCARD\r\nVERSION:4.0\r\nFN:Sylvain "
	    "Berfini\r\nTEL;TYPE=work:0952636505\r\nTEL:0476010203\r\nEND:VCARD\r\n");
	lf = linphone_core_create_friend_from_vcard(manager->lc, lvc);
	linphone_vcard_unref(lvc);
	sip_addresses = linphone_friend_get_addresses(lf);
	phone_numbers = linphone_friend_get_phone_numbers(lf);

	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(sip_addresses), 0, unsigned int, "%u");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(phone_numbers), 2, unsigned int, "%u");
	if (phone_numbers) bctbx_list_free_with_data(phone_numbers, (bctbx_list_free_func)bctbx_free);

	addr = linphone_address_new("sip:sylvain@sip.linphone.org");
	linphone_friend_add_address(lf, addr);
	LinphoneAddress *addr2 = linphone_address_new("sip:sylvain@sip.linphone.org");
	linphone_friend_add_address(lf, addr2);
	linphone_address_unref(addr2);
	sip_addresses = linphone_friend_get_addresses(lf);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(sip_addresses), 1, unsigned int, "%u");

	linphone_friend_remove_phone_number(lf, "0952636505");
	phone_numbers = linphone_friend_get_phone_numbers(lf);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(phone_numbers), 1, unsigned int, "%u");
	if (phone_numbers) bctbx_list_free_with_data(phone_numbers, (bctbx_list_free_func)bctbx_free);

	linphone_friend_remove_phone_number(lf, "0476010203");
	phone_numbers = linphone_friend_get_phone_numbers(lf);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(phone_numbers), 0, unsigned int, "%u");
	if (phone_numbers) bctbx_list_free_with_data(phone_numbers, (bctbx_list_free_func)bctbx_free);

	linphone_friend_edit(lf);
	linphone_friend_remove_address(lf, addr);
	linphone_friend_done(lf);
	sip_addresses = linphone_friend_get_addresses(lf);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(sip_addresses), 0, unsigned int, "%u");

	linphone_friend_add_phone_number(lf, "+33952636505");
	linphone_friend_add_phone_number(lf, "+33952636505");
	phone_numbers = linphone_friend_get_phone_numbers(lf);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(phone_numbers), 1, unsigned int, "%u");
	if (phone_numbers) bctbx_list_free_with_data(phone_numbers, (bctbx_list_free_func)bctbx_free);

	linphone_address_unref(addr);
	linphone_friend_unref(lf);
	lf = NULL;
	lvc = NULL;
	linphone_core_manager_destroy(manager);
}

static void linphone_vcard_local_photo_to_base_64(void) {
	LinphoneCoreManager *manager = linphone_core_manager_new_with_proxies_check("empty_rc", FALSE);
	LinphoneVcard *lvc = linphone_vcard_context_get_vcard_from_buffer(
	    linphone_core_get_vcard_context(manager->lc),
	    "BEGIN:VCARD\r\nVERSION:4.0\r\nFN:Sylvain Berfini\r\nTEL;TYPE=work:"
	    "0952636505\r\nPHOTO:https://gitlab.linphone.org/uploads/-/system/appearance/header_logo/1/"
	    "logo-BC.png\r\nEND:VCARD\r\n");
	BC_ASSERT_PTR_NOT_NULL(lvc);
	if (lvc == nullptr) return;

	char *asString = ms_strdup(linphone_vcard_as_vcard4_string(lvc));
	char *asStringWithBase64Picture = ms_strdup(linphone_vcard_as_vcard4_string_with_base_64_picture(lvc));
	BC_ASSERT_STRING_EQUAL(asString, asStringWithBase64Picture);
	linphone_vcard_unref(lvc);
	bctbx_free(asString);
	bctbx_free(asStringWithBase64Picture);

	lvc = linphone_vcard_context_get_vcard_from_buffer(
	    linphone_core_get_vcard_context(manager->lc),
	    "BEGIN:VCARD\r\nVERSION:4.0\r\nFN:Sylvain Berfini\r\nTEL;TYPE=work:"
	    "0952636505\r\nEND:VCARD\r\n");
	BC_ASSERT_PTR_NOT_NULL(lvc);
	if (lvc == nullptr) return;

	const char *base64Image =
	    "data:image/jpeg;base64,/9j/4AAQSkZJRgABAQIAdgB2AAD//gATQ3JlYXRlZCB3aXRoIEdJTVD/"
	    "4gKwSUNDX1BST0ZJTEUAAQEAAAKgbGNtcwRAAABtbnRyUkdCIFhZWiAH6QABABcACwAeABxhY3NwQVBQTAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
	    "AAAAAA9tYAAQAAAADTLWxjbXMAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA1kZXNjAAABIAAAAEBjcHJ0"
	    "AAABYAAAADZ3dHB0AAABmAAAABRjaGFkAAABrAAAACxyWFlaAAAB2AAAABRiWFlaAAAB7AAAABRnWFlaAAACAAAAABRyVFJDAAACFAAAACBnVF"
	    "JDAAACFAAAACBiVFJDAAACFAAAACBjaHJtAAACNAAAACRkbW5kAAACWAAAACRkbWRkAAACfAAAACRtbHVjAAAAAAAAAAEAAAAMZW5VUwAAACQA"
	    "AAAcAEcASQBNAFAAIABiAHUAaQBsAHQALQBpAG4AIABzAFIARwBCbWx1YwAAAAAAAAABAAAADGVuVVMAAAAaAAAAHABQAHUAYgBsAGkAYwAgAE"
	    "QAbwBtAGEAaQBuAABYWVogAAAAAAAA9tYAAQAAAADTLXNmMzIAAAAAAAEMQgAABd7///MlAAAHkwAA/ZD///uh///"
	    "9ogAAA9wAAMBuWFlaIAAAAAAAAG+gAAA49QAAA5BYWVogAAAAAAAAJJ8AAA+"
	    "EAAC2xFhZWiAAAAAAAABilwAAt4cAABjZcGFyYQAAAAAAAwAAAAJmZgAA8qcAAA1ZAAAT0AAACltjaHJtAAAAAAADAAAAAKPXAABUfAAATM0AA"
	    "JmaAAAmZwAAD1xtbHVjAAAAAAAAAAEAAAAMZW5VUwAAAAgAAAAcAEcASQBNAFBtbHVjAAAAAAAAAAEAAAAMZW5VUwAAAAgAAAAcAHMAUgBHAEL"
	    "/2wBDAAEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQH/"
	    "2wBDAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQH/"
	    "wgARCAAoACgDAREAAhEBAxEB/8QAGAAAAwEBAAAAAAAAAAAAAAAABwgJBQb/xAAaAQADAQEBAQAAAAAAAAAAAAAFBgcIBAID/"
	    "9oADAMBAAIQAxAAAAG4RIGjtNg2tzECsEZGrRa/"
	    "OmvZoJ4ZiVh9i4oZJYziJdcP6+3ylWjOKJAua7wkzLljF4ZRqM3rT4Qg7Sm9Jz0FWdAqPCNnzusWWQO2ylnES3PTLdGf/"
	    "8QAHhAAAgICAwEBAAAAAAAAAAAABQYCBAMHAAETFBH/2gAIAQEAAQUCPn6i9TtMB9j7+NfBcCbBV/"
	    "SEupxaSMGdqaaVFUy5h8rEiYzx5qR0sVSAQb+"
	    "kWQaDtSmlDLvL6qh2o4uvmPNdG6tWG0VbbB6uMzhhBLWTVaya5WLRdyZV6k0CLAbaGvuX2RqKEQuHb7VjWloarDf/"
	    "xAAsEQACAgEEAAUDAwUAAAAAAAACAwEEBQYREhMABxQhMRUiQSNSYSQyM0JR/9oACAEDAQE/"
	    "Ac9nqmApxZsA6w97PT0MfVGGXcjbISMa1VcyMSXADa5rCBFZANs2GLQozi3qnOaillzI5mcbg1PBDFYO5Yp4ZdqSYI4tWaornUWs84ZEkZxumg"
	    "oUCYBI+oSc8GMs6S0n05HUk4zC2JqgCGZoQu6lsCLZDvDE1gy+eGq/YHiV/"
	    "UNiwqC42kVmg3xgfNzQN54Ua+q60NP2X9QpZHF1vmI4i++HSJTM7ABW1zP4Etp8DMEMFExMFETEjO4zE/ExP5ifxPjWWXVq/"
	    "V2Qwy7Y1tN6fWqnqrMJdxsNXbsghelcWzfZTMzkVqr3yWSjuEr07y6cUC7essfj9G26N/Ere/"
	    "LupsxmmwtRUPE6Ox9cFBdbg6KqqhXeud6hZZtTasN2nssdK/TOt6f9ebrV2Tt2bBSb7Fkic5xzHuTGs5Gc/wAkUzt/HjUumvpvK1VHiA+7Fx/"
	    "bx/cP/NvzHxt/Pz5BeY9uLidE5ewT6llZ/"
	    "QnNIiZUsJCWTjuUxP8ASuSLCrwRDFdq4QHIbACrF4Y2ah0vpTIGERWO75pa9NTJgX3OyWYqpxLmfCoE0oNE9a2DfY+"
	    "I7dpnVGK09cKjd1DZ6EU/UJQBP6Ftba6SnfjHawxGtuALKPbmRQUR7M0Hib3prGKuyNBs7s4kNjdfv71W/"
	    "u5RwKG8+"
	    "MzJfIdZZjRPlrk03sW7Jrq3EV3lYeOVCH1wQMlYaxb5KrMJiJmwPVHWMFv17bxp1QUtZ4pOMuxkF1dTUF0MgtJoi4KcoqKtoUM3NXdAgzqKSkO"
	    "XHkW3KdcUclo5vmlq+BUz6uvRa8W2Y+axXQq5SiU8Y4z1VACeEz+kaGTG/"
	    "t41vhL2tMZgctpmzXtLWDLK1E+FpuVcgquYOS33VLFdIxAnIRINZ98EPA9I4m1gsHXx+Rer1jWvZIrZyBZsjl0oIoGWSC1yw+I7c+"
	    "yY3COU6i8mdb5dmQrpt4saoLeyvaZYZLMgcbyiv0wveuT/AGhzGlIJmZ4y+Pu8eTWhbmS10Hr6xKRpK36rKCc/"
	    "4sjSeYUqckstpb69EtKIklmqo4S3EogtU6ax+"
	    "rcJcwWT7Yq24CexB8HIcooYh659xklMET4GJLPbiYkMzHi5pnzm8sltqaXyN7I4LsliTxlStkwCWHO++IuIvWKJkUyx/"
	    "pVsrci7CsGXKYy2d8yc5nKbslf1G7OY79WihKH07FOfuGX1sfRRXFJnBytrgribQnqaZhED401T8+dXK9Nk87f0/"
	    "hnAS33r2Px+Pv8AV7gUVUop1MtLZ/0aTaoTH3jZ+N9LaWxWkMSrE4lRQuCl1my6YO3ftnEdty43aOxzNo/EAsIFShBYCMf/"
	    "xAAuEQACAgIBAwIEBgIDAAAAAAABAgMEBRESAAYTITEUIjJBByNCQ1FhFTNSU3H/2gAIAQIBAT8BxeLsZWwYYikUUa+W1amPGCrACAZZW/"
	    "skLHGu3lkKxopY9QYjGYgLBVoC1kGjMgkvwRT33iHHds0rDDHYKiNMVtZIz2AD61jrrnmcvzr4w2r0YkPMUGavjI/"
	    "lB8bXpmp4+SWP1UirikjbW45JFK9ZLs7uBEaw+HkKr6t8LYq2pj/ZSDg7a/UVgf8A9XogqSCCCCQQRogj7EfY/"
	    "wAjrBU3weCq2zD5Mpk2axi6ci/"
	    "IrxxGRsrcX3KUaxLwKeQhDc4wJrrGLBWbOeisQXWjSnHMlnJmHyrczViUuYVvTtKxNeHxtqKHwxqOKpHvTpFeEASKELFFGOKRxgIiKPsqqAqj+"
	    "h1j8h59I52T7H+/4PX4idsQy1Xz9KMJZgK/"
	    "5BEAAnhY8fiCB+9ExXyHW3jJZj+V65G7xoZbL1gSZfD2p2+"
	    "rD6IhpLUyn0Xcsvn042R8HGhPHY6w93KQCxXxcPkkn8ckhEfkZEh5ge54KCZfVnB9eIGj7r3JdreaK7X3aT0TYMXzbH+5P40eQKa5eg9m5Cnn+"
	    "667QW1pmavLJGscZqN45DIQI1Vk/"
	    "OHMkCMljyJH1extHz4a01uHwebGzmzXZ1fw86z+aJpF+V+GyvMaDa3oe3Xb9mrm4u0MOCR8Cc692PftMkBepY1v15NZZwT+4JFHt12/"
	    "fr4C5kqWWjkhZmWJnEZZoZazSqyOo+fg/"
	    "Pe1Deqr6EHYzl2HJZGWzUjf4eNI05MmiyqdeSQDfEMz8F5HeuAOmOusf3xgKyVndLXlZo1liWEarA/XIX5cZEj/"
	    "AEiP53A+lfbrvXPQUe3HMEqtLmYfh6ZX15wWIwZpwCPoFZ9BtbEksf36w2Xs4PIQ5GpwMkXJWSQbSSJxqSNvuOS/"
	    "qUhlOiD1Dk+xe7ik2TgrVshxCulyV6kh0B6C5DJBHZUfTHzcSaGvEo9OqeI7coUpo6tbHx0LA4zszJNHMP8AjLPO8hkUa2qvIQp9VAPWTl/"
	    "DrBP5q9Grkr6HlHWrWJ7UPP3HlZ5paSID7rxkZf8ApPWZzN3O3Wu3WG9cIYU2Ia0IJ4wwrs6Ub2T9TsSzEk9f/"
	    "8QAMRAAAgIBAwIEBAMJAAAAAAAAAgMBBAUREhMAIQYUIjEjMkJRQUNSBxUWJDM0RGFi/9oACAEBAAY/"
	    "Aos2BY97j4KNCvG61fs7ZKEpH7QMSbnFotColjJ00iWWb2VmhigbCpTirLqmKW6ddtGL9QSy3ibIz6YOniYRVmfUNwBLpdrMDSxrCWMr/"
	    "eqxtZhvfs4cVXXkMqhTPmGb2eNg+xrUUF0NQfEIDJekIvU7tKqP+hZYN4B/zvtJGPsXt0JjIlBRBRIzuGYmNYkSj5on8J/"
	    "GOshU8zKsHhRGpmL6S9ZKJ8JjC44vlhuTuxC7BxtmwQ7W76mNHmp2MaDmZB1dlPDRY4CoeHaiYCLB4ysKQiLb+"
	    "UNbFjncZb2MbM6gxjrEm5zZk2taUsYwp9yMy1Ipn7z1JhHb8Y6V4VyLpZSuboxZsKZmpaiJLyozP+PZiC4x10W+BEI+"
	    "MWmDwV2dAQLvHHison52TEuo1zj5tqqs1ogO3qybyj1aT1TuZyzwJq8ykjy8QsZY4yn2iWmQwnWBXp23SWsR2r2MZd0ot7s9UP8AR3/"
	    "t2fq3RsIW7ts6zrqPHNqhOTivbQppNd58ZYrhGZaZgz4BcURMuARiRiC+TTWKcULHmJRl6/"
	    "k7QAS+biuB5d4rP1hLNAOALuOuk69ftBzxxExlP4aVjX6aRNaxa/namumkcQ48UkMT/"
	    "SlJzHqjrDZPAtVZWIMcKpcKxem4CCFgGXw+"
	    "RUq2kByPzF3gg2kmjfcvzb2uZsBmsARjrwpmdN5AtctLbGkFyTGoxum4tZUpSsGmh5WO9yY1lahXpJqY36pdtWE/WUerpIWEEFfw/"
	    "Z85kt35dio2fL1S07ch3FxqGvdSX+8D1YxF6WApu1i3JnRiHr7qcP0ntn3WeomMzHbtMNRiLF25id8kt2NQGQr95n1HQeqyymc+7ZhXFu/"
	    "PZ79Iddu5R2TqzurCMMS2sX6kV64KhJT9RLUMl7FM9RWfkr+IxTI2OvXayKL+L2LhgK6Mi8yjXQoMFl9Tx16DHY4J9+"
	    "WzZZpNi5YmNDe8o95nTQAj0LCIEY+//8QAGxABAQEBAQEBAQAAAAAAAAAAAREAITFBcVH/"
	    "2gAIAQEAAT8hnS2p1BsIiUyMHK9d6ld6WKDUKx6AZmDRLMZ3S+AW1OabePvMB4nvGuACafIJh0BK21D6AHEllG7AZoe5M0bHulWNWpReur/"
	    "cBkj859Q00HNJmxAkUK9DP2OK7JUtLZKlSEBUJCPcm0YgqwhjUQUj9AJ1nDl9hJIU6cvnRywrZfQJRqXDIqdp1uSJAqlENI5382AwYNieHY0PF"
	    "CCVvTRMjD0qcE/nBTFQp7gKpiUl2KhXIx0C+PbDtMfTZFHTmJGDQDH7T+i+OlCjV875x3G4ABQMucX7ciinUXbR5d8HHEJBGBc//"
	    "9oADAMBAAIAAwAAABBy2/TFZwMtDhsf/8QAGREBAQEBAQEAAAAAAAAAAAAAAREAITFB/9oACAEDAQE/"
	    "EA1JNDbdMCHOcPksS1XAX9l4XKMhozFxAPHuPkxDAKiWqEeAKrjpQQ+ERDiInMVBjHubY5BV0O+"
	    "2c5IAolCjaEAww0FjIAgABATpBHxH0XhPQkFCNi0JdAwKK2M6ZyslPqYK7w4LAVsJbEahRBWEHaemmpgIWO4lmuIayVjYCLwOkrbloAdV5yKyJ"
	    "xgJ6/ZIehGBkahcZyGI7RINMNwgle/EbOYgLezUs1wuJPPH9g2j/O1NcTK/"
	    "ukD8GUTpAOBdPZXx06qw5iUVTpBKnM+YOmxADDIg2VCs9UP6Z0IirU0l3lzIyv8A/8QAGxEBAQEBAQEBAQAAAAAAAAAAAREhADFBUXH/"
	    "2gAIAQIBAT8Quj2QZSFxIGlTgkK6YUqRjcrSSXIfGmxgoRcODqiqtFjkgpU9wec8oJwirQSJo0eEW5bIrGd9fwxbb9xFeKKaMj5hWR47g4PwAG"
	    "vuc6GAb76NJ7P6P2cknQN+GjBCh6Q9NTD4yGMOxEgAWeXbAYCnFhRTh44TB2kp+"
	    "bZAhxYecmlvUKaiy8LcKkKdEAVjQNIXZmtJSpJKY10T1wpJWoOFmYqT3EKcqlngSgI4B1T0ooGBQUSBXet6bEjKJMUHYCoVnxxeqhsgYMRGttx"
	    "cIRE4UYRcThCPuI8KLTmCrQIzBO4dRvUCDLiUErYzpyFQZPf/xAAZEAEBAQEBAQAAAAAAAAAAAAABEQAhMUH/"
	    "2gAIAQEAAT8QeS4bSfF0mZlNhd8DVz6s6Ox5SjHqQXo8WooNfEmBSdDXesmEoouQcVFihAzLJgK7SFbMDYUXVYvp7pg5np1aph33fX7OiR0z1A"
	    "48LeJ6hxLy+"
	    "053abRtnYUGI1lzwRKDoYgzFSnHclKAiLOsaeqmZx0H3k3rgrGcanW5e0SthRjpxbZZkZV6ez0yNyIbWjuC1g6clxRIDuEKKJxSZ3l9Pc8S7pK"
	    "kYC2fLgVyu288Fqt1HVdMk8OyoszyXYgVGt1GIwQbApYC9YmhQgjZY/7IxkFrkNeHybZ1cGwsWAS47//Z";
	linphone_vcard_set_photo(lvc, base64Image);

	asString = ms_strdup(linphone_vcard_as_vcard4_string(lvc));
	asStringWithBase64Picture = ms_strdup(linphone_vcard_as_vcard4_string_with_base_64_picture(lvc));
	BC_ASSERT_STRING_EQUAL(asString, asStringWithBase64Picture);
	char *base64vCard = ms_strdup(asStringWithBase64Picture);
	linphone_vcard_unref(lvc);
	bctbx_free(asString);
	bctbx_free(asStringWithBase64Picture);

	lvc = linphone_vcard_context_get_vcard_from_buffer(
	    linphone_core_get_vcard_context(manager->lc),
	    "BEGIN:VCARD\r\nVERSION:4.0\r\nFN:Sylvain Berfini\r\nTEL;TYPE=work:"
	    "0952636505\r\n\r\nEND:VCARD\r\n");
	BC_ASSERT_PTR_NOT_NULL(lvc);
	if (lvc == nullptr) return;

	char *picture_path = bc_tester_res("images/bc.jpeg");
	std::string picture = "file:";
	picture.append(picture_path);
	linphone_vcard_set_photo(lvc, picture.c_str());
	bctbx_free(picture_path);

	asString = ms_strdup(linphone_vcard_as_vcard4_string(lvc));
	asStringWithBase64Picture = ms_strdup(linphone_vcard_as_vcard4_string_with_base_64_picture(lvc));
	BC_ASSERT_STRING_NOT_EQUAL(asString, asStringWithBase64Picture);
	BC_ASSERT_STRING_EQUAL(base64vCard, asStringWithBase64Picture);
	linphone_vcard_unref(lvc);
	bctbx_free(base64vCard);
	bctbx_free(asString);
	bctbx_free(asStringWithBase64Picture);

	linphone_core_manager_destroy(manager);
}

static void friends_if_no_db_set(void) {
	LinphoneCoreManager *manager = linphone_core_manager_new_with_proxies_check("empty_rc", FALSE);
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

static void friend_list_created_cb(BCTBX_UNUSED(LinphoneCore *lc), LinphoneFriendList *list) {
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

	// Verify that the removed list is not in the core anymore
	const bctbx_list_t *friends_list = linphone_core_get_friends_lists(lc);
	bctbx_list_t *copy = bctbx_list_copy(friends_list);

	for (bctbx_list_t *it = copy; it != nullptr; it = it->next) {
		LinphoneFriendList *core_list = (LinphoneFriendList *)bctbx_list_get_data(it);
		if (core_list) BC_ASSERT_PTR_NOT_EQUAL(core_list, list);
	}

	bctbx_list_free(copy);
}

static void friends_sqlite_storage(void) {
	LinphoneCore *lc = NULL;
	LinphoneCoreCbs *cbs;
	LinphoneFriendList *lfl = NULL;
	LinphoneFriend *lf = NULL;
	LinphoneFriend *lf2 = NULL;
	LinphoneVcard *lvc = NULL;
	LinphoneAddress *addr = linphone_address_new("sip:sylvain@sip.linphone.org");
	LinphoneCoreManager *lcm;
	const bctbx_list_t *friends = NULL;
	bctbx_list_t *friends_from_db = NULL;
	const bctbx_list_t *friends_from_friendlist = NULL;
	bctbx_list_t *friends_lists_from_db = NULL;
	char *friends_db = bc_tester_file(vcard_friends_db_file);
	LinphoneFriendListStats *stats = (LinphoneFriendListStats *)ms_new0(LinphoneFriendListStats, 1);
	const LinphoneAddress *laddress = NULL, *laddress2 = NULL;
	char *address = NULL, *address2 = NULL;

	cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_friend_list_created(cbs, friend_list_created_cb);
	linphone_core_cbs_set_friend_list_removed(cbs, friend_list_removed_cb);
	lcm = linphone_core_manager_new_with_proxies_check("stun_rc", FALSE);
	lc = lcm->lc;
	linphone_core_add_callbacks(lc, cbs);
	linphone_core_cbs_unref(cbs);
	friends = linphone_friend_list_get_friends(linphone_core_get_default_friend_list(lc));
	lfl = linphone_core_create_friend_list(lc);
	linphone_friend_list_set_user_data(lfl, stats);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends), 0, unsigned int, "%u");

	unlink(friends_db);
	linphone_core_set_friends_database_path(lc, friends_db);
	friends_from_db = linphone_core_fetch_friends_from_db(lc, linphone_core_get_default_friend_list(lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends_from_db), 0, unsigned int, "%u");

	lf = linphone_core_create_friend(lc);
	linphone_friend_set_address(lf, addr);
	linphone_friend_set_name(lf, "Sylvain");
	lvc = linphone_friend_get_vcard(lf);
	linphone_vcard_set_etag(lvc, "\"123-456789\"");
	linphone_vcard_set_url(lvc, "http://dav.somewhere.fr/addressbook/me/someone.vcf");

	linphone_core_add_friend_list(lc, lfl);
	wait_for_until(lc, NULL, &stats->new_list_count, 1, 1000);
	BC_ASSERT_EQUAL(stats->new_list_count, 1, int, "%i");
	linphone_friend_list_unref(lfl);
	linphone_friend_list_set_display_name(lfl, "Test");
	BC_ASSERT_EQUAL(linphone_friend_list_add_friend(lfl, lf), LinphoneFriendListOK, int, "%i");
	linphone_friend_unref(lf);
	BC_ASSERT_EQUAL(linphone_friend_list_get_storage_id(lfl), 1, long long, "%lld");
	BC_ASSERT_EQUAL(linphone_friend_get_storage_id(lf), 1, long long, "%lld");

	friends = linphone_friend_list_get_friends(linphone_core_get_default_friend_list(lc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends), 0, unsigned int, "%u");

	friends_lists_from_db = linphone_core_fetch_friends_lists_from_db(lc);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends_lists_from_db), 1, unsigned int, "%u");
	friends_from_friendlist =
	    linphone_friend_list_get_friends((LinphoneFriendList *)bctbx_list_get_data(friends_lists_from_db));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends_from_friendlist), 1, unsigned int, "%u");
	lf2 = (LinphoneFriend *)friends_from_friendlist->data;
	BC_ASSERT_PTR_NOT_NULL(linphone_friend_get_core(lf2));
	BC_ASSERT_PTR_NOT_NULL(linphone_friend_get_friend_list(lf2));
	friends_lists_from_db =
	    bctbx_list_free_with_data(friends_lists_from_db, (void (*)(void *))linphone_friend_list_unref);

	friends_from_db = linphone_core_fetch_friends_from_db(lc, lfl);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(friends_from_db), 1, unsigned int, "%u");
	if (bctbx_list_size(friends_from_db) < 1) {
		goto end;
	}
	lf2 = (LinphoneFriend *)friends_from_db->data;
	BC_ASSERT_STRING_EQUAL(linphone_friend_get_name(lf2), linphone_friend_get_name(lf));
	BC_ASSERT_EQUAL(linphone_friend_get_storage_id(lf2), linphone_friend_get_storage_id(lf), long long int, "%lld");
	BC_ASSERT_STRING_EQUAL(linphone_vcard_get_etag(linphone_friend_get_vcard(lf2)),
	                       linphone_vcard_get_etag(linphone_friend_get_vcard(lf)));
	BC_ASSERT_STRING_EQUAL(linphone_vcard_get_url(linphone_friend_get_vcard(lf2)),
	                       linphone_vcard_get_url(linphone_friend_get_vcard(lf)));
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
	unlink(friends_db);
	bc_free(friends_db);
	linphone_core_manager_destroy(lcm);
}

typedef struct _LinphoneCardDAVStats {
	int sync_done_count;
	int new_contact_count;
	int removed_contact_count;
	int updated_contact_count;
} LinphoneCardDAVStats;

static void carddav_contact_created(LinphoneFriendList *list, BCTBX_UNUSED(LinphoneFriend *lf)) {
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)linphone_friend_list_cbs_get_user_data(
	    linphone_friend_list_get_current_callbacks(list));
	stats->new_contact_count++;
}

static void carddav_contact_deleted(LinphoneFriendList *list, BCTBX_UNUSED(LinphoneFriend *lf)) {
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)linphone_friend_list_cbs_get_user_data(
	    linphone_friend_list_get_current_callbacks(list));
	stats->removed_contact_count++;
}

static void carddav_contact_updated(LinphoneFriendList *list, LinphoneFriend *new_friend, LinphoneFriend *old_friend) {
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)linphone_friend_list_cbs_get_user_data(
	    linphone_friend_list_get_current_callbacks(list));
	BC_ASSERT_STRING_EQUAL(linphone_vcard_get_full_name(linphone_friend_get_vcard(new_friend)),
	                       linphone_vcard_get_full_name(linphone_friend_get_vcard(old_friend)));
	stats->updated_contact_count++;
}

static void
carddav_sync_status_changed(LinphoneFriendList *list, LinphoneFriendListSyncStatus status, const char *msg) {
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)linphone_friend_list_cbs_get_user_data(
	    linphone_friend_list_get_current_callbacks(list));
	const char *state = status == LinphoneFriendListSyncStarted
	                        ? "Sync started"
	                        : (status == LinphoneFriendListSyncFailure ? "Sync failure" : "Sync successful");
	ms_message("[CardDAV] %s : %s", state, msg);
	if (status == LinphoneFriendListSyncFailure || status == LinphoneFriendListSyncSuccessful) {
		stats->sync_done_count++;
	}
}

static void carddav_operation_before_sync(void) {
	LinphoneCoreManager *manager = linphone_core_manager_new_with_proxies_check("carddav_rc", FALSE);
	LinphoneFriendList *lfl = linphone_core_create_friend_list(manager->lc);
	LinphoneFriendListCbs *cbs = NULL;
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)ms_new0(LinphoneCardDAVStats, 1);
	LinphoneFriend *lf = NULL;
	LinphoneVcard *lvc = NULL;

	cbs = linphone_factory_create_friend_list_cbs(linphone_factory_get());
	linphone_friend_list_cbs_set_user_data(cbs, stats);
	linphone_friend_list_cbs_set_contact_created(cbs, carddav_contact_created);
	linphone_friend_list_cbs_set_contact_deleted(cbs, carddav_contact_deleted);
	linphone_friend_list_cbs_set_contact_updated(cbs, carddav_contact_updated);
	linphone_friend_list_cbs_set_sync_status_changed(cbs, carddav_sync_status_changed);
	linphone_friend_list_add_callbacks(lfl, cbs);
	linphone_friend_list_cbs_unref(cbs);
	linphone_friend_list_set_display_name(lfl, "CardDAV friend list");
	linphone_friend_list_set_uri(lfl, CARDDAV_SERVER_WITH_PORT);
	linphone_friend_list_set_type(lfl, LinphoneFriendListTypeCardDAV);
	linphone_core_add_friend_list(manager->lc, lfl);
	linphone_friend_list_unref(lfl);

	BC_ASSERT_STRING_EQUAL(linphone_friend_list_get_revision(lfl), "");
	// DO NOT SYNCHRONIZE THE NEWLY CREATE FRIEND LIST
	// linphone_friend_list_synchronize_friends_from_server(lfl);

	lvc = linphone_vcard_context_get_vcard_from_buffer(
	    linphone_core_get_vcard_context(manager->lc),
	    "BEGIN:VCARD\r\nVERSION:4.0\r\nFN:Sylvain "
	    "Berfini\r\nIMPP:sip:sylvain@sip.linphone.org\r\nUID:1f08dd48-29ac-4097-8e48-8596d7776283\r\nEND:VCARD\r\n");
	linphone_vcard_set_url(lvc, ME_VCF);
	lf = linphone_core_create_friend_from_vcard(manager->lc, lvc);
	linphone_vcard_unref(lvc);
	linphone_friend_list_add_friend(lfl, lf);
	linphone_friend_unref(lf);
	
	wait_for_until(manager->lc, NULL, &stats->sync_done_count, 1, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->sync_done_count, 1, int, "%i");

	ms_free(stats);
	linphone_core_manager_destroy(manager);
}

static void carddav_clean(void) { 
	// This is to ensure the content of the test addressbook is in the correct state for the following tests
	LinphoneCoreManager *manager = linphone_core_manager_new_with_proxies_check("carddav_rc", FALSE);
	LinphoneFriendList *lfl = linphone_core_create_friend_list(manager->lc);
	LinphoneFriendListCbs *cbs = NULL;
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)ms_new0(LinphoneCardDAVStats, 1);
	bctbx_list_t *friends = NULL;
	bctbx_list_t *friends_iterator = NULL;
	LinphoneFriend *lf = NULL;
	LinphoneVcard *lvc = NULL;

	cbs = linphone_factory_create_friend_list_cbs(linphone_factory_get());
	linphone_friend_list_cbs_set_user_data(cbs, stats);
	linphone_friend_list_cbs_set_contact_created(cbs, carddav_contact_created);
	linphone_friend_list_cbs_set_contact_deleted(cbs, carddav_contact_deleted);
	linphone_friend_list_cbs_set_contact_updated(cbs, carddav_contact_updated);
	linphone_friend_list_cbs_set_sync_status_changed(cbs, carddav_sync_status_changed);
	linphone_friend_list_add_callbacks(lfl, cbs);
	linphone_friend_list_cbs_unref(cbs);
	linphone_friend_list_set_display_name(lfl, "CardDAV friend list");
	linphone_friend_list_set_uri(lfl, CARDDAV_SERVER_WITH_PORT);
	linphone_friend_list_set_type(lfl, LinphoneFriendListTypeCardDAV);
	linphone_core_add_friend_list(manager->lc, lfl);
	linphone_friend_list_unref(lfl);

	BC_ASSERT_STRING_EQUAL(linphone_friend_list_get_revision(lfl), "");
	linphone_friend_list_synchronize_friends_from_server(lfl);
	wait_for_until(manager->lc, NULL, &stats->sync_done_count, 1, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->sync_done_count, 1, int, "%i");
	BC_ASSERT_STRING_NOT_EQUAL(linphone_friend_list_get_revision(lfl), "");
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

	lvc = linphone_vcard_context_get_vcard_from_buffer(
	    linphone_core_get_vcard_context(manager->lc),
	    "BEGIN:VCARD\r\nVERSION:4.0\r\nFN:Sylvain "
	    "Berfini\r\nIMPP:sip:sylvain@sip.linphone.org\r\nUID:1f08dd48-29ac-4097-8e48-8596d7776283\r\nEND:VCARD\r\n");
	linphone_vcard_set_url(lvc, ME_VCF);
	lf = linphone_core_create_friend_from_vcard(manager->lc, lvc);
	linphone_vcard_unref(lvc);
	linphone_friend_list_add_friend(lfl, lf);
	linphone_friend_unref(lf);
	wait_for_until(manager->lc, NULL, &stats->sync_done_count, 1, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->sync_done_count, 1, int, "%i");

	ms_free(stats);
	linphone_core_manager_destroy(manager);
}

static void carddav_integration(void) {
	LinphoneCoreManager *manager = linphone_core_manager_new_with_proxies_check("carddav_rc", FALSE);
	LinphoneFriendList *lfl = linphone_core_create_friend_list(manager->lc);
	LinphoneVcard *lvc = linphone_vcard_context_get_vcard_from_buffer(
	    linphone_core_get_vcard_context(manager->lc),
	    "BEGIN:VCARD\r\nVERSION:4.0\r\nFN:Margaux "
	    "Clerc\r\nIMPP;TYPE=work:sip:margaux@sip.linphone.org\r\nEND:VCARD\r\n");
	LinphoneFriend *lf = linphone_core_create_friend_from_vcard(manager->lc, lvc);
	LinphoneVcard *lvc2 = NULL;
	LinphoneFriend *lf2 = NULL;
	LinphoneFriendListCbs *cbs = NULL;
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)ms_new0(LinphoneCardDAVStats, 1);
	const char *refkey = "toto";
	char *address = NULL;
	const LinphoneAddress *addr;

	linphone_vcard_unref(lvc);
	linphone_friend_list_set_uri(lfl, CARDDAV_SERVER);
	linphone_friend_list_set_type(lfl, LinphoneFriendListTypeCardDAV);
	cbs = linphone_factory_create_friend_list_cbs(linphone_factory_get());
	linphone_friend_list_add_callbacks(lfl, cbs);
	linphone_friend_list_cbs_set_user_data(cbs, stats);
	linphone_friend_list_cbs_set_contact_created(cbs, carddav_contact_created);
	linphone_friend_list_cbs_set_contact_deleted(cbs, carddav_contact_deleted);
	linphone_friend_list_cbs_set_contact_updated(cbs, carddav_contact_updated);
	linphone_friend_list_cbs_set_sync_status_changed(cbs, carddav_sync_status_changed);
	linphone_core_add_friend_list(manager->lc, lfl);

	BC_ASSERT_STRING_EQUAL(linphone_friend_list_get_revision(lfl), "");
	linphone_friend_list_synchronize_friends_from_server(lfl);
	wait_for_until(manager->lc, NULL, &stats->sync_done_count, 1, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->sync_done_count, 1, int, "%i");

	BC_ASSERT_PTR_NULL(linphone_vcard_get_uid(lvc));
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_friend_list_get_dirty_friends_to_update(lfl)), 0,
	                unsigned int, "%u");

	BC_ASSERT_EQUAL(linphone_friend_list_add_friend(lfl, lf), LinphoneFriendListOK, int, "%d");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_friend_list_get_dirty_friends_to_update(lfl)), 1,
	                unsigned int, "%u");
	wait_for_until(manager->lc, NULL, &stats->sync_done_count, 2, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->sync_done_count, 2, int, "%i");
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_friend_list_get_dirty_friends_to_update(lfl)), 0,
	                unsigned int, "%u");
	BC_ASSERT_PTR_NOT_NULL(linphone_vcard_get_uid(lvc));

	linphone_friend_list_remove_friend(lfl, lf);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(linphone_friend_list_get_friends(lfl)), 1, unsigned int,
	                "%u"); // a local Sylvain friend is there
	LinphoneFriend *sylvain = (LinphoneFriend *)bctbx_list_get_data(linphone_friend_list_get_friends(lfl));
	BC_ASSERT_PTR_NOT_NULL(sylvain);
	wait_for_until(manager->lc, NULL, &stats->sync_done_count, 3, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->sync_done_count, 3, int, "%i");

	linphone_friend_unref(lf);
	lf = NULL;

	lvc = linphone_vcard_context_get_vcard_from_buffer(
	    linphone_core_get_vcard_context(manager->lc),
	    "BEGIN:VCARD\r\nVERSION:4.0\r\nFN:Ghislain "
	    "Mary\r\nIMPP;TYPE=work:sip:ghislain@sip.linphone.org\r\nEND:VCARD\r\n");
	lf = linphone_core_create_friend_from_vcard(manager->lc, lvc);
	linphone_vcard_unref(lvc);
	BC_ASSERT_EQUAL(linphone_friend_list_add_local_friend(lfl, lf), LinphoneFriendListOK, int, "%d");
	linphone_friend_unref(lf);

	lvc2 = linphone_vcard_context_get_vcard_from_buffer(
	    linphone_core_get_vcard_context(manager->lc),
	    "BEGIN:VCARD\r\nVERSION:4.0\r\nFN:Sylvain "
	    "Berfini\r\nIMPP:sip:sberfini@sip.linphone.org\r\nUID:1f08dd48-29ac-4097-8e48-8596d7776283\r\nEND:VCARD\r\n");
	linphone_vcard_set_url(lvc2, ME_VCF_3);
	lf2 = linphone_core_create_friend_from_vcard(manager->lc, lvc2);
	linphone_vcard_unref(lvc2);
	linphone_friend_set_ref_key(lf2, refkey);
	BC_ASSERT_EQUAL(linphone_friend_list_add_local_friend(lfl, lf2), LinphoneFriendListOK, int, "%d");
	linphone_friend_unref(lf2);

	// To force update
	linphone_vcard_set_etag(linphone_friend_get_vcard(sylvain), "wrong");

	stats->new_contact_count = 0;
	stats->removed_contact_count = 0;
	stats->updated_contact_count = 0;
	BC_ASSERT_STRING_NOT_EQUAL(linphone_friend_list_get_revision(lfl), "0");
	linphone_friend_list_synchronize_friends_from_server(lfl);
	wait_for_until(manager->lc, NULL, &stats->sync_done_count, 4, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->sync_done_count, 4, int, "%i");
	wait_for_until(manager->lc, NULL, &stats->new_contact_count, 0, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->new_contact_count, 0, int, "%i");
	wait_for_until(manager->lc, NULL, &stats->removed_contact_count, 2, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->removed_contact_count, 2, int, "%i");
	wait_for_until(manager->lc, NULL, &stats->updated_contact_count, 1, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->updated_contact_count, 1, int, "%i");
	BC_ASSERT_STRING_NOT_EQUAL(linphone_friend_list_get_revision(lfl), "0");

	BC_ASSERT_EQUAL(bctbx_list_size(linphone_friend_list_get_friends(lfl)), 1, size_t, "%zu");
	lf = (LinphoneFriend *)bctbx_list_get_data((linphone_friend_list_get_friends(lfl)));
	addr = linphone_friend_get_address(lf);
	BC_ASSERT_PTR_NOT_NULL(addr);
	address = linphone_address_as_string_uri_only(addr);
	BC_ASSERT_STRING_EQUAL(address, "sip:sylvain@sip.linphone.org");
	ms_free(address);

	linphone_friend_edit(lf);
	linphone_friend_done(lf);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(
	                    linphone_friend_list_get_dirty_friends_to_update(linphone_friend_get_friend_list(lf))),
	                0, unsigned int, "%u");

	linphone_core_set_network_reachable(manager->lc, FALSE); // To prevent the CardDAV update
	linphone_friend_edit(lf);
	linphone_friend_set_name(lf, "François Grisez");
	linphone_friend_done(lf);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(
	                    linphone_friend_list_get_dirty_friends_to_update(linphone_friend_get_friend_list(lf))),
	                1, unsigned int, "%u");

	linphone_friend_list_unref(lfl);
	linphone_friend_list_cbs_unref(cbs);
	linphone_core_manager_destroy(manager);
	ms_free(stats);
}

void _onMagicSearchResultsReceived(LinphoneMagicSearch *magic_search) {
	stats *stat =
	    (stats *)linphone_magic_search_cbs_get_user_data(linphone_magic_search_get_current_callbacks(magic_search));
	++stat->number_of_LinphoneMagicSearchResultReceived;
}

static void magic_search_carddav_query_from_api(void) {
	LinphoneCoreManager *manager = linphone_core_manager_new("marie_rc");

	// Init Magic search
	LinphoneMagicSearch *magicSearch = NULL;
	LinphoneMagicSearchCbs *searchHandler = linphone_factory_create_magic_search_cbs(linphone_factory_get());
	linphone_magic_search_cbs_set_search_results_received(searchHandler, _onMagicSearchResultsReceived);
	magicSearch = linphone_magic_search_new(manager->lc);
	linphone_magic_search_add_callbacks(magicSearch, searchHandler);

	bctbx_list_t *resultList = NULL;
	stats *stat = get_stats(manager->lc);
	linphone_magic_search_cbs_set_user_data(searchHandler, stat);
	LinphoneSearchResult *result = nullptr;

	bctbx_list_t *remoteContactDirectoriesList = linphone_core_get_remote_contact_directories(manager->lc);
	BC_ASSERT_EQUAL((int)bctbx_list_size(remoteContactDirectoriesList), 0, int, "%d");

	//------------------------------------------------------------------------

	linphone_magic_search_get_contacts_list_async(magicSearch, "", "", LinphoneMagicSearchSourceRemoteCardDAV,
	                                              LinphoneMagicSearchAggregationNone);
	BC_ASSERT_TRUE(wait_for(manager->lc, NULL, &stat->number_of_LinphoneMagicSearchResultReceived, 1));
	stat->number_of_LinphoneMagicSearchResultReceived = 0;
	resultList = linphone_magic_search_get_last_search(magicSearch);
	// no result expected as no CardDAV server is configured
	BC_ASSERT_EQUAL((int)bctbx_list_size(resultList), 0, int, "%d");

	LinphoneCardDavParams *carddavParams = linphone_core_create_card_dav_params(manager->lc);
	bctbx_list_t *fields = bctbx_list_append(NULL, (void *)"FN");
	fields = bctbx_list_append(fields, (void *)"N");
	fields = bctbx_list_append(fields, (void *)"IMPP");
	linphone_card_dav_params_set_user_input_fields(carddavParams, fields);
	bctbx_list_free(fields);
	fields = bctbx_list_append(NULL, (void *)"IMPP");
	linphone_card_dav_params_set_domain_fields(carddavParams, fields);
	bctbx_list_free(fields);
	linphone_card_dav_params_set_use_exact_match_policy(carddavParams, FALSE);

	LinphoneRemoteContactDirectory *rcd =
	    linphone_core_create_card_dav_remote_contact_directory(manager->lc, carddavParams);
	linphone_remote_contact_directory_set_server_url(rcd, CARDDAV_SERVER);
	linphone_remote_contact_directory_set_limit(rcd, 0);
	linphone_remote_contact_directory_set_min_characters(rcd, 0);
	linphone_remote_contact_directory_set_timeout(rcd, 5);
	linphone_core_add_remote_contact_directory(manager->lc, rcd);
	linphone_card_dav_params_unref(carddavParams);

	remoteContactDirectoriesList = linphone_core_get_remote_contact_directories(manager->lc);
	BC_ASSERT_EQUAL((int)bctbx_list_size(remoteContactDirectoriesList), 1, int, "%d");
	if (remoteContactDirectoriesList) {
		bctbx_list_free_with_data(remoteContactDirectoriesList,
		                          (bctbx_list_free_func)linphone_remote_contact_directory_unref);
	}

	if (linphone_core_vcard_supported()) {
		linphone_magic_search_get_contacts_list_async(magicSearch, "", "", LinphoneMagicSearchSourceRemoteCardDAV,
		                                              LinphoneMagicSearchAggregationNone);
		BC_ASSERT_TRUE(wait_for(manager->lc, NULL, &stat->number_of_LinphoneMagicSearchResultReceived, 1));
		stat->number_of_LinphoneMagicSearchResultReceived = 0;
		resultList = linphone_magic_search_get_last_search(magicSearch);
		// 0 expected as no AuthInfo was provided to do the authentication
		BC_ASSERT_EQUAL((int)bctbx_list_size(resultList), 0, int, "%d");

		LinphoneAuthInfo *authInfo = linphone_core_create_auth_info(
		    manager->lc, "tester", NULL, NULL, "40aa5ad3b300405c20c5e4eda2da9751", "BaikalDAV", "dav.example.org");
		linphone_core_add_auth_info(manager->lc, authInfo);
		linphone_auth_info_unref(authInfo);

		linphone_magic_search_get_contacts_list_async(magicSearch, "", "", LinphoneMagicSearchSourceRemoteCardDAV,
		                                              LinphoneMagicSearchAggregationNone);
		BC_ASSERT_TRUE(wait_for(manager->lc, NULL, &stat->number_of_LinphoneMagicSearchResultReceived, 1));
		stat->number_of_LinphoneMagicSearchResultReceived = 0;
		resultList = linphone_magic_search_get_last_search(magicSearch);
		// no filter so 1 expected, the whole address book
		BC_ASSERT_EQUAL((int)bctbx_list_size(resultList), 1, int, "%d");
		if (resultList) {
			result = (LinphoneSearchResult *)bctbx_list_get_data(resultList);
			BC_ASSERT_STRING_EQUAL(linphone_friend_get_name(linphone_search_result_get_friend(result)),
			                       "François Grisez");
			bctbx_list_free_with_data(resultList, (bctbx_list_free_func)linphone_search_result_unref);
		}

		linphone_magic_search_get_contacts_list_async(magicSearch, "çois", "", LinphoneMagicSearchSourceRemoteCardDAV,
		                                              LinphoneMagicSearchAggregationNone);
		BC_ASSERT_TRUE(wait_for(manager->lc, NULL, &stat->number_of_LinphoneMagicSearchResultReceived, 1));
		stat->number_of_LinphoneMagicSearchResultReceived = 0;
		resultList = linphone_magic_search_get_last_search(magicSearch);
		// 1 result expected: François Grisez from FN & N
		BC_ASSERT_EQUAL((int)bctbx_list_size(resultList), 1, int, "%d");
		if (resultList) {
			result = (LinphoneSearchResult *)bctbx_list_get_data(resultList);
			BC_ASSERT_STRING_EQUAL(linphone_friend_get_name(linphone_search_result_get_friend(result)),
			                       "François Grisez");
			bctbx_list_free_with_data(resultList, (bctbx_list_free_func)linphone_search_result_unref);
		}

		linphone_magic_search_get_contacts_list_async(magicSearch, "sylv", "sip.linphone.org",
		                                              LinphoneMagicSearchSourceRemoteCardDAV,
		                                              LinphoneMagicSearchAggregationNone);
		BC_ASSERT_TRUE(wait_for(manager->lc, NULL, &stat->number_of_LinphoneMagicSearchResultReceived, 1));
		stat->number_of_LinphoneMagicSearchResultReceived = 0;
		resultList = linphone_magic_search_get_last_search(magicSearch);
		// 1 result expected: François Grisez from IMPP
		BC_ASSERT_EQUAL((int)bctbx_list_size(resultList), 1, int, "%d");
		if (resultList) {
			result = (LinphoneSearchResult *)bctbx_list_get_data(resultList);
			BC_ASSERT_STRING_EQUAL(linphone_friend_get_name(linphone_search_result_get_friend(result)),
			                       "François Grisez");
			bctbx_list_free_with_data(resultList, (bctbx_list_free_func)linphone_search_result_unref);
		}
	}

	//------------------------------------------------------------------------

	linphone_core_remove_remote_contact_directory(manager->lc, rcd);
	remoteContactDirectoriesList = linphone_core_get_remote_contact_directories(manager->lc);
	BC_ASSERT_EQUAL((int)bctbx_list_size(remoteContactDirectoriesList), 0, int, "%d");
	linphone_remote_contact_directory_unref(rcd);

	linphone_magic_search_cbs_unref(searchHandler);
	linphone_magic_search_unref(magicSearch);

	linphone_core_manager_destroy(manager);
}

static void carddav_multiple_sync(void) {
	LinphoneCoreManager *manager = linphone_core_manager_new_with_proxies_check("carddav_rc", FALSE);
	LinphoneFriendList *lfl = linphone_core_create_friend_list(manager->lc);
	LinphoneFriendListCbs *cbs = NULL;
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)ms_new0(LinphoneCardDAVStats, 1);

	cbs = linphone_factory_create_friend_list_cbs(linphone_factory_get());
	linphone_friend_list_add_callbacks(lfl, cbs);
	linphone_friend_list_cbs_set_user_data(cbs, stats);
	linphone_friend_list_cbs_set_contact_created(cbs, carddav_contact_created);
	linphone_friend_list_cbs_set_contact_deleted(cbs, carddav_contact_deleted);
	linphone_friend_list_cbs_set_contact_updated(cbs, carddav_contact_updated);
	linphone_friend_list_cbs_set_sync_status_changed(cbs, carddav_sync_status_changed);
	linphone_core_add_friend_list(manager->lc, lfl);
	linphone_friend_list_set_uri(lfl, CARDDAV_SERVER);
	linphone_friend_list_set_type(lfl, LinphoneFriendListTypeCardDAV);

	linphone_friend_list_synchronize_friends_from_server(lfl);
	wait_for_until(manager->lc, NULL, &stats->sync_done_count, 1, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->sync_done_count, 1, int, "%i");

	stats->new_contact_count = 0;
	stats->updated_contact_count = 0;
	stats->removed_contact_count = 0;

	linphone_friend_list_synchronize_friends_from_server(lfl);
	wait_for_until(manager->lc, NULL, &stats->sync_done_count, 2, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->sync_done_count, 2, int, "%i");

	linphone_friend_list_synchronize_friends_from_server(lfl);
	wait_for_until(manager->lc, NULL, &stats->sync_done_count, 3, CARDDAV_SYNC_TIMEOUT);
	BC_ASSERT_EQUAL(stats->sync_done_count, 3, int, "%i");

	BC_ASSERT_EQUAL(stats->new_contact_count, 0, int, "%i");
	BC_ASSERT_EQUAL(stats->updated_contact_count, 0, int, "%i");
	BC_ASSERT_EQUAL(stats->removed_contact_count, 0, int, "%i");

	ms_free(stats);
	linphone_friend_list_unref(lfl);
	linphone_friend_list_cbs_unref(cbs);
	linphone_core_manager_destroy(manager);
}

static void carddav_server_to_client_and_client_to_sever_sync(void) {
	LinphoneCoreManager *manager = linphone_core_manager_new_with_proxies_check("carddav_rc", FALSE);
	LinphoneFriendList *lfl = linphone_core_create_friend_list(manager->lc);
	LinphoneFriendListCbs *cbs = NULL;
	LinphoneCardDAVStats *stats = (LinphoneCardDAVStats *)ms_new0(LinphoneCardDAVStats, 1);
	LinphoneVcard *lvc1 = linphone_vcard_context_get_vcard_from_buffer(
	    linphone_core_get_vcard_context(manager->lc),
	    "BEGIN:VCARD\r\nVERSION:4.0\r\nFN:Margaux "
	    "Clerc\r\nIMPP;TYPE=work:sip:margaux@sip.linphone.org\r\nEND:VCARD\r\n");
	LinphoneFriend *lf1 = linphone_core_create_friend_from_vcard(manager->lc, lvc1);
	LinphoneVcard *lvc2 = linphone_vcard_context_get_vcard_from_buffer(
	    linphone_core_get_vcard_context(manager->lc),
	    "BEGIN:VCARD\r\nVERSION:4.0\r\nFN:Ghislain "
	    "Mary\r\nIMPP;TYPE=work:sip:ghislain@sip.linphone.org\r\nEND:VCARD\r\n");
	LinphoneFriend *lf2 = linphone_core_create_friend_from_vcard(manager->lc, lvc2);
	bctbx_list_t *friends = NULL, *friends_iterator = NULL;

	linphone_vcard_unref(lvc1);
	linphone_vcard_unref(lvc2);
	cbs = linphone_factory_create_friend_list_cbs(linphone_factory_get());
	linphone_friend_list_add_callbacks(lfl, cbs);
	linphone_friend_list_cbs_set_user_data(cbs, stats);
	linphone_friend_list_cbs_set_contact_created(cbs, carddav_contact_created);
	linphone_friend_list_cbs_set_contact_deleted(cbs, carddav_contact_deleted);
	linphone_friend_list_cbs_set_contact_updated(cbs, carddav_contact_updated);
	linphone_friend_list_cbs_set_sync_status_changed(cbs, carddav_sync_status_changed);
	linphone_core_add_friend_list(manager->lc, lfl);
	linphone_friend_list_set_uri(lfl, CARDDAV_SERVER);
	linphone_friend_list_set_type(lfl, LinphoneFriendListTypeCardDAV);

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
	linphone_friend_list_cbs_unref(cbs);
	linphone_core_manager_destroy(manager);
}

static void magic_search_carddav_query_from_config(void) {
	LinphoneCoreManager *manager = linphone_core_manager_new("marie_carddav_rc");

	// Init Magic search
	LinphoneMagicSearch *magicSearch = NULL;
	LinphoneMagicSearchCbs *searchHandler = linphone_factory_create_magic_search_cbs(linphone_factory_get());
	linphone_magic_search_cbs_set_search_results_received(searchHandler, _onMagicSearchResultsReceived);
	magicSearch = linphone_magic_search_new(manager->lc);
	linphone_magic_search_add_callbacks(magicSearch, searchHandler);

	bctbx_list_t *resultList = NULL;
	stats *stat = get_stats(manager->lc);
	linphone_magic_search_cbs_set_user_data(searchHandler, stat);
	LinphoneSearchResult *result = nullptr;

	bctbx_list_t *remoteContactDirectoriesList = linphone_core_get_remote_contact_directories(manager->lc);
	BC_ASSERT_EQUAL((int)bctbx_list_size(remoteContactDirectoriesList), 1, int,
	                "%d"); // There is one CardDavParams configured in marie_carddav_rc
	if (remoteContactDirectoriesList) {
		bctbx_list_free_with_data(remoteContactDirectoriesList,
		                          (bctbx_list_free_func)linphone_remote_contact_directory_unref);
	}

	//------------------------------------------------------------------------

	if (linphone_core_vcard_supported()) {
		linphone_magic_search_get_contacts_list_async(magicSearch, "", "", LinphoneMagicSearchSourceRemoteCardDAV,
		                                              LinphoneMagicSearchAggregationNone);
		BC_ASSERT_TRUE(wait_for(manager->lc, NULL, &stat->number_of_LinphoneMagicSearchResultReceived, 1));
		stat->number_of_LinphoneMagicSearchResultReceived = 0;
		resultList = linphone_magic_search_get_last_search(magicSearch);
		// no filter so 2 expected : margaux & ghislain, the whole address book
		BC_ASSERT_EQUAL((int)bctbx_list_size(resultList), 2, int, "%d");
		if (resultList) {
			result = (LinphoneSearchResult *)bctbx_list_get_data(resultList);
			BC_ASSERT_STRING_EQUAL(linphone_friend_get_name(linphone_search_result_get_friend(result)),
			                       "Ghislain Mary");
		}
		bctbx_list_free_with_data(resultList, (bctbx_list_free_func)linphone_search_result_unref);

		linphone_magic_search_get_contacts_list_async(magicSearch, "gh", "", LinphoneMagicSearchSourceRemoteCardDAV,
		                                              LinphoneMagicSearchAggregationNone);
		BC_ASSERT_TRUE(wait_for(manager->lc, NULL, &stat->number_of_LinphoneMagicSearchResultReceived, 1));
		stat->number_of_LinphoneMagicSearchResultReceived = 0;
		resultList = linphone_magic_search_get_last_search(magicSearch);
		// 1 expected : ghislain (FN, N & IMPP will match)
		BC_ASSERT_EQUAL((int)bctbx_list_size(resultList), 1, int, "%d");
		if (resultList) {
			result = (LinphoneSearchResult *)bctbx_list_get_data(resultList);
			BC_ASSERT_STRING_EQUAL(linphone_friend_get_name(linphone_search_result_get_friend(result)),
			                       "Ghislain Mary");
		}
		bctbx_list_free_with_data(resultList, (bctbx_list_free_func)linphone_search_result_unref);

		linphone_magic_search_get_contacts_list_async(magicSearch, "mar", "", LinphoneMagicSearchSourceRemoteCardDAV,
		                                              LinphoneMagicSearchAggregationNone);
		BC_ASSERT_TRUE(wait_for(manager->lc, NULL, &stat->number_of_LinphoneMagicSearchResultReceived, 1));
		stat->number_of_LinphoneMagicSearchResultReceived = 0;
		resultList = linphone_magic_search_get_last_search(magicSearch);
		// 2 expected : margaux (FN, N & IMPP will match) & Ghislain Mary (FN & N) will match
		BC_ASSERT_EQUAL((int)bctbx_list_size(resultList), 2, int, "%d");
		bctbx_list_free_with_data(resultList, (bctbx_list_free_func)linphone_search_result_unref);

		linphone_magic_search_get_contacts_list_async(magicSearch, "marg", "", LinphoneMagicSearchSourceRemoteCardDAV,
		                                              LinphoneMagicSearchAggregationNone);
		linphone_magic_search_get_contacts_list_async(magicSearch, "marga", "", LinphoneMagicSearchSourceRemoteCardDAV,
		                                              LinphoneMagicSearchAggregationNone);
		linphone_magic_search_get_contacts_list_async(magicSearch, "margau", "", LinphoneMagicSearchSourceRemoteCardDAV,
		                                              LinphoneMagicSearchAggregationNone);
		linphone_magic_search_get_contacts_list_async(
		    magicSearch, "margaux", "", LinphoneMagicSearchSourceRemoteCardDAV, LinphoneMagicSearchAggregationNone);
		BC_ASSERT_TRUE(wait_for(manager->lc, NULL, &stat->number_of_LinphoneMagicSearchResultReceived, 1));
		stat->number_of_LinphoneMagicSearchResultReceived = 0;
		resultList = linphone_magic_search_get_last_search(magicSearch);
		// 1 expected : margaux
		BC_ASSERT_EQUAL((int)bctbx_list_size(resultList), 1, int, "%d");
		if (resultList) {
			result = (LinphoneSearchResult *)bctbx_list_get_data(resultList);
			BC_ASSERT_STRING_EQUAL(linphone_friend_get_name(linphone_search_result_get_friend(result)),
			                       "Margaux Clerc");
		}
		bctbx_list_free_with_data(resultList, (bctbx_list_free_func)linphone_search_result_unref);

		linphone_magic_search_get_contacts_list_async(magicSearch, "margaux", "sip.test.org",
		                                              LinphoneMagicSearchSourceRemoteCardDAV,
		                                              LinphoneMagicSearchAggregationNone);
		BC_ASSERT_TRUE(wait_for(manager->lc, NULL, &stat->number_of_LinphoneMagicSearchResultReceived, 1));
		stat->number_of_LinphoneMagicSearchResultReceived = 0;
		resultList = linphone_magic_search_get_last_search(magicSearch);
		// O expected as margaux SIP URI isn't on sip.test.org domain
		BC_ASSERT_EQUAL((int)bctbx_list_size(resultList), 0, int, "%d");
		bctbx_list_free_with_data(resultList, (bctbx_list_free_func)linphone_search_result_unref);

		linphone_magic_search_get_contacts_list_async(magicSearch, "", "sip.test.org",
		                                              LinphoneMagicSearchSourceRemoteCardDAV,
		                                              LinphoneMagicSearchAggregationNone);
		BC_ASSERT_TRUE(wait_for(manager->lc, NULL, &stat->number_of_LinphoneMagicSearchResultReceived, 1));
		stat->number_of_LinphoneMagicSearchResultReceived = 0;
		resultList = linphone_magic_search_get_last_search(magicSearch);
		// O expected as margaux SIP URI isn't on sip.test.org domain
		BC_ASSERT_EQUAL((int)bctbx_list_size(resultList), 0, int, "%d");
		bctbx_list_free_with_data(resultList, (bctbx_list_free_func)linphone_search_result_unref);

		linphone_magic_search_get_contacts_list_async(magicSearch, "", "sip.linphone.org",
		                                              LinphoneMagicSearchSourceRemoteCardDAV,
		                                              LinphoneMagicSearchAggregationNone);
		BC_ASSERT_TRUE(wait_for(manager->lc, NULL, &stat->number_of_LinphoneMagicSearchResultReceived, 1));
		stat->number_of_LinphoneMagicSearchResultReceived = 0;
		resultList = linphone_magic_search_get_last_search(magicSearch);
		// 2 expected : margaux & ghislain (IMPP will match)
		BC_ASSERT_EQUAL((int)bctbx_list_size(resultList), 2, int, "%d");
		if (resultList) {
			result = (LinphoneSearchResult *)bctbx_list_get_data(resultList);
			BC_ASSERT_STRING_EQUAL(linphone_friend_get_name(linphone_search_result_get_friend(result)),
			                       "Ghislain Mary");
		}
		bctbx_list_free_with_data(resultList, (bctbx_list_free_func)linphone_search_result_unref);

		linphone_magic_search_get_contacts_list_async(magicSearch, "robert", "sip.linphone.org",
		                                              LinphoneMagicSearchSourceRemoteCardDAV,
		                                              LinphoneMagicSearchAggregationNone);
		BC_ASSERT_TRUE(wait_for(manager->lc, NULL, &stat->number_of_LinphoneMagicSearchResultReceived, 1));
		stat->number_of_LinphoneMagicSearchResultReceived = 0;
		resultList = linphone_magic_search_get_last_search(magicSearch);
		// 0 expected as no contact is named robert
		BC_ASSERT_EQUAL((int)bctbx_list_size(resultList), 0, int, "%d");
		bctbx_list_free_with_data(resultList, (bctbx_list_free_func)linphone_search_result_unref);
	}

	//------------------------------------------------------------------------

	linphone_magic_search_cbs_unref(searchHandler);
	linphone_magic_search_unref(magicSearch);

	linphone_core_manager_destroy(manager);
}

static void find_friend_by_ref_key_test(void) {
	LinphoneCoreManager *manager = linphone_core_manager_new_with_proxies_check("empty_rc", FALSE);
	LinphoneFriendList *lfl = linphone_core_get_default_friend_list(manager->lc);
	LinphoneFriend *lf = linphone_core_create_friend_with_address(manager->lc, "sip:toto@sip.linphone.org");
	LinphoneFriend *lf2 = NULL;
	const LinphoneAddress *addr = NULL;
	char *uri_addr = nullptr;
	linphone_friend_set_ref_key(lf, "totorefkey");
	linphone_friend_list_add_friend(lfl, lf);
	lf2 = linphone_friend_list_find_friend_by_ref_key(lfl, "totorefkey");
	BC_ASSERT_PTR_NOT_NULL(lf2);
	if (!lf2) goto end;
	addr = linphone_friend_get_address(lf2);
	uri_addr = linphone_address_as_string_uri_only(addr);
	BC_ASSERT_STRING_EQUAL(uri_addr, "sip:toto@sip.linphone.org");
	bctbx_free(uri_addr);
	BC_ASSERT_EQUAL(lf2, lf, void *, "%p");
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
	for (i = 0; i < 20000; i++) {
		snprintf(key, sizeof(key), "key_%i", i);
		pair = bctbx_pair_cchar_new(key, (void *)(uintptr_t)i);
		bctbx_map_cchar_insert_and_delete(friends_map, (bctbx_pair_t *)pair);
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
	for (i = 0; i < 20000; i++) {
		snprintf(key, sizeof(key), "key_%i", i);
		pair = bctbx_pair_cchar_new(key, (void *)(uintptr_t)i);
		bctbx_map_cchar_insert_and_delete(friends_map, (bctbx_pair_t *)pair);
	}
	bctbx_get_cur_time(&t1);
	ms_message("Start : %li : %li\n", (long int)t1.tv_sec, (long int)t1.tv_nsec);
	for (i = 0; i < 20000; i++) {
		snprintf(key, sizeof(key), "key_%i", i);
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
	LinphoneCoreManager *manager = linphone_core_manager_new_with_proxies_check("empty_rc", FALSE);
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
    TEST_NO_TAG("vCard with local photo file to base64", linphone_vcard_local_photo_to_base_64),
    TEST_NO_TAG("Friends working if no db set", friends_if_no_db_set),
    TEST_NO_TAG("Friends storage in sqlite database", friends_sqlite_storage),
    TEST_ONE_TAG("CardDAV operation before client sync", carddav_operation_before_sync, "CardDAV"),
    TEST_ONE_TAG("CardDAV clean", carddav_clean, "CardDAV"), // This is to ensure the content of the test addressbook is
                                                             // in the correct state for the following tests
    TEST_ONE_TAG("CardDAV integration", carddav_integration, "CardDAV"),
    TEST_TWO_TAGS("Search friend in remote CardDAV server from API",
                  magic_search_carddav_query_from_api,
                  "CardDAV",
                  "MagicSearch"),
    TEST_ONE_TAG("CardDAV multiple synchronizations", carddav_multiple_sync, "CardDAV"),
    TEST_ONE_TAG("CardDAV client to server and server to client sync",
                 carddav_server_to_client_and_client_to_sever_sync,
                 "CardDAV"),
    TEST_TWO_TAGS("Search friend in remote CardDAV server from config",
                  magic_search_carddav_query_from_config,
                  "CardDAV",
                  "MagicSearch"),
    TEST_NO_TAG("Find friend by ref key", find_friend_by_ref_key_test),
    TEST_NO_TAG("create a map and insert 20000 objects", insert_lot_of_friends_map_test),
    TEST_NO_TAG("Find ref key in 20000 objects map", find_friend_by_ref_key_in_lot_of_friends_test),
    TEST_NO_TAG("Find friend by ref key in empty list", find_friend_by_ref_key_empty_list_test)};

test_suite_t vcard_test_suite = {"VCard",
                                 NULL,
                                 NULL,
                                 liblinphone_tester_before_each,
                                 liblinphone_tester_after_each,
                                 sizeof(vcard_tests) / sizeof(vcard_tests[0]),
                                 vcard_tests,
                                 0};

#endif
