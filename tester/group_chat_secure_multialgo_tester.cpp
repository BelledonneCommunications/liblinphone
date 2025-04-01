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
#include "bctoolbox/exception.hh"

#include "liblinphone_tester++.h"
#include "liblinphone_tester.h"

#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic push
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

/*
 * Hard migration scenario
 * - create 4 lime users on curve25519, they all register on the server
 * - they exchange messages (Marie writes a message to the group, IMDN are on so everyone writes to everyone)
 * - one user stops its core, restart and switch to curve25519k512, change lime db
 * local path
 * - check this user is registered on the lime server
 * - Exchange messages(each try to send to all) : the 3 users on curve 25519 can do it, the migrated user cannot
 * communicate anymore
 * - switch another user
 * - Exchange messages(each trying to send to all) : user can communicate only 2 by 2 with the one on the same curve
 * - switch the last two users to curve25519k512
 * - Exchange messages: everyone can communicate
 */
static void group_chat_lime_x3dh_hard_migration(void) {
	if (!liblinphone_tester_is_lime_PQ_available()) {
		bctbx_warning("Skip lime hard migration test as lime PQ is not supported");
		return;
	}

	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	coresManagerList = bctbx_list_append(coresManagerList, chloe);
	set_lime_server_and_curve_list(C25519, coresManagerList); // all clients start on curve 25519 only
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;
	stats initialChloeStats = chloe->stat;
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));

	// Enable IMDN
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(laure->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(chloe->lc));

	// Wait for lime user creation
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess,
	                             initialMarieStats.number_of_X3dhUserCreationSuccess + 1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess,
	                             initialPaulineStats.number_of_X3dhUserCreationSuccess + 1,
	                             x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_X3dhUserCreationSuccess,
	                             initialLaureStats.number_of_X3dhUserCreationSuccess + 1, x3dhServer_creationTimeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_X3dhUserCreationSuccess,
	                             initialChloeStats.number_of_X3dhUserCreationSuccess + 1, x3dhServer_creationTimeout));

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE,
	                                 LinphoneChatRoomEphemeralModeDeviceManaged);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's and Laure's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 3, FALSE);
	LinphoneChatRoom *laureCr =
	    check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 3, FALSE);
	LinphoneChatRoom *chloeCr =
	    check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 3, FALSE);

	try {
		if (!BC_ASSERT_PTR_NOT_NULL(marieCr) || !BC_ASSERT_PTR_NOT_NULL(paulineCr) ||
		    !BC_ASSERT_PTR_NOT_NULL(laureCr) || !BC_ASSERT_PTR_NOT_NULL(chloeCr))
			throw BCTBX_EXCEPTION;

		// Marie send a message to the group
		const char *marieTextMessage = "Hello";
		LinphoneChatMessage *marieMessage = _send_message(marieCr, marieTextMessage);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived,
		                             initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived,
		                             initialLaureStats.number_of_LinphoneMessageReceived + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageReceived,
		                             initialChloeStats.number_of_LinphoneMessageReceived + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageSent, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered,
		                             initialMarieStats.number_of_LinphoneMessageDelivered + 1, 10000));
		LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
		if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg)) throw BCTBX_EXCEPTION;
		LinphoneChatMessage *laureLastMsg = laure->stat.last_received_chat_message;
		if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg)) throw BCTBX_EXCEPTION;
		LinphoneChatMessage *chloeLastMsg = chloe->stat.last_received_chat_message;
		if (!BC_ASSERT_PTR_NOT_NULL(chloeLastMsg)) throw BCTBX_EXCEPTION;
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieTextMessage);
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieTextMessage);
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(chloeLastMsg), marieTextMessage);
		LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
		BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
		BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(laureLastMsg)));
		BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(chloeLastMsg)));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDeliveredToUser,
		                             initialMarieStats.number_of_LinphoneMessageDeliveredToUser + 1,
		                             10000)); // make sure the IMDN is back to marie
		wait_for_list(coresList, 0, 1, 4000); // Just to be sure all imdn message finally reach their recipients (Laure,
		                                      // Pauline and Chloe receive each others IMDN)
		linphone_chat_message_unref(marieMessage);

		// Stop Pauline core and switch to c25519k512
		linphone_core_set_network_reachable(pauline->lc, FALSE);
		LinphoneAddress *paulineAddr = linphone_address_clone(linphone_chat_room_get_peer_address(paulineCr));
		coresList = bctbx_list_remove(coresList, pauline->lc);
		linphone_core_manager_reinit(pauline);
		bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, pauline);
		// Set pauline curve to c25519k512, and delete its lime DB - TODO: do it via file remote provisioning
		set_lime_server_and_curve_list(C25519K512, tmpCoresManagerList);
		unlink(pauline->lime_database_path);
		bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
		bctbx_list_free(tmpCoresManagerList);
		coresList = bctbx_list_concat(coresList, tmpCoresList);
		linphone_core_manager_start(pauline, TRUE);
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));
		paulineCr = linphone_core_search_chat_room(pauline->lc, NULL, NULL, paulineAddr, NULL);
		wait_for_list(coresList, 0, 1, 4000); // Make sure Pauline's core restart can complete lime update
		linphone_address_unref(paulineAddr);
		if (!BC_ASSERT_PTR_NOT_NULL(paulineCr)) throw BCTBX_EXCEPTION;

		// Marie sends a new message: Pauline will not be able to get it but the others two yes
		// Marie send a message to the group
		const char *marieTextMessage2 = "Pauline do you hear me?";
		initialMarieStats = marie->stat;
		initialPaulineStats = pauline->stat;
		initialLaureStats = laure->stat;
		initialChloeStats = chloe->stat;
		marieMessage = _send_message(marieCr, marieTextMessage2);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered,
		                             initialMarieStats.number_of_LinphoneMessageDelivered + 1,
		                             10000)); // Delivered to the server
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceivedFailedToDecrypt,
		                             initialPaulineStats.number_of_LinphoneMessageReceivedFailedToDecrypt + 1,
		                             10000)); // Pauline fails to decrypt
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived,
		                             initialLaureStats.number_of_LinphoneMessageReceived + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageReceived,
		                             initialChloeStats.number_of_LinphoneMessageReceived + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageNotDelivered,
		                             initialMarieStats.number_of_LinphoneMessageNotDelivered + 1,
		                             10000)); // Not delivered to pauline
		laureLastMsg = laure->stat.last_received_chat_message;
		if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg)) throw BCTBX_EXCEPTION;
		chloeLastMsg = chloe->stat.last_received_chat_message;
		if (!BC_ASSERT_PTR_NOT_NULL(chloeLastMsg)) throw BCTBX_EXCEPTION;
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieTextMessage2);
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(chloeLastMsg), marieTextMessage2);
		BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(laureLastMsg)));
		BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(chloeLastMsg)));
		wait_for_list(coresList, 0, 1, 4000); // Just to be sure all imdn message finally reach their recipients (Laure,
		                                      // Pauline and Chloe receive each others IMDN)

		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageReceived,
		                initialPaulineStats.number_of_LinphoneMessageReceived, int, "%d");
		linphone_chat_message_unref(marieMessage);

		// Now Pauline sends a message, no one should be able to get it: it is not even sent
		const char *paulineTextMessage = "Girls? Where are you?";
		initialMarieStats = marie->stat;
		initialPaulineStats = pauline->stat;
		initialLaureStats = laure->stat;
		initialChloeStats = chloe->stat;
		LinphoneChatMessage *paulineMessage = _send_message(paulineCr, paulineTextMessage);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageNotDelivered,
		                             initialPaulineStats.number_of_LinphoneMessageNotDelivered + 1,
		                             10000)); // Unable to encrypt to any participant
		linphone_chat_message_unref(paulineMessage);

		// Stop Laure core and switch to c25519k512
		linphone_core_set_network_reachable(laure->lc, FALSE);
		LinphoneAddress *laureAddr = linphone_address_clone(linphone_chat_room_get_peer_address(laureCr));
		coresList = bctbx_list_remove(coresList, laure->lc);
		linphone_core_manager_reinit(laure);
		tmpCoresManagerList = bctbx_list_append(NULL, laure);
		// Set laure curve to c25519k512, and delete its lime DB - TODO: do it via file remote provisioning
		set_lime_server_and_curve_list(C25519K512, tmpCoresManagerList);
		unlink(laure->lime_database_path);
		tmpCoresList = init_core_for_conference(tmpCoresManagerList);
		bctbx_list_free(tmpCoresManagerList);
		coresList = bctbx_list_concat(coresList, tmpCoresList);
		linphone_core_manager_start(laure, TRUE);
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(laure->lc));
		laureCr = linphone_core_search_chat_room(laure->lc, NULL, NULL, laureAddr, NULL);
		linphone_address_unref(laureAddr);
		wait_for_list(coresList, 0, 1, 4000); // Make sure Laure's core restart can complete lime update
		if (!BC_ASSERT_PTR_NOT_NULL(laureCr)) throw BCTBX_EXCEPTION;

		// Marie Sends a message, only chloe can get it but she still encrypts to Laure and Pauline as they have keys on
		// the server or an active session
		const char *marieTextMessage3 = "Chloe, do you copy?";
		initialMarieStats = marie->stat;
		initialPaulineStats = pauline->stat;
		initialLaureStats = laure->stat;
		initialChloeStats = chloe->stat;
		marieMessage = _send_message(marieCr, marieTextMessage3);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered,
		                             initialMarieStats.number_of_LinphoneMessageDelivered + 1,
		                             10000)); // Delivered to the server
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceivedFailedToDecrypt,
		                             initialPaulineStats.number_of_LinphoneMessageReceivedFailedToDecrypt + 1,
		                             10000)); // Pauline fails to decrypt
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceivedFailedToDecrypt,
		                             initialLaureStats.number_of_LinphoneMessageReceivedFailedToDecrypt + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageReceived,
		                             initialChloeStats.number_of_LinphoneMessageReceived + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageNotDelivered,
		                             initialMarieStats.number_of_LinphoneMessageNotDelivered + 1,
		                             10000)); // Not delivered to pauline and laure
		chloeLastMsg = chloe->stat.last_received_chat_message;
		if (!BC_ASSERT_PTR_NOT_NULL(chloeLastMsg)) throw BCTBX_EXCEPTION;
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(chloeLastMsg), marieTextMessage3);
		BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(chloeLastMsg)));
		wait_for_list(coresList, 0, 1, 4000); // Just to be sure all imdn message finally reach their recipients (Laure,
		                                      // Pauline and Chloe receive each others IMDN)

		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageReceived,
		                initialPaulineStats.number_of_LinphoneMessageReceived, int, "%d");
		BC_ASSERT_EQUAL(laure->stat.number_of_LinphoneMessageReceived,
		                initialLaureStats.number_of_LinphoneMessageReceived, int, "%d");
		linphone_chat_message_unref(marieMessage);

		// pauline sends a message, only laure can get it, No keys for Marie and Chloe on the server
		// only laure receives the message and it is not marked as delivered to users as pauline could not encrypt to
		// Marie and Chloe
		paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
		initialMarieStats = marie->stat;
		initialPaulineStats = pauline->stat;
		initialLaureStats = laure->stat;
		initialChloeStats = chloe->stat;
		paulineMessage = _send_message(paulineCr, paulineTextMessage);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDelivered,
		                             initialPaulineStats.number_of_LinphoneMessageDelivered + 1,
		                             10000)); // Delivered to the server
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived,
		                             initialLaureStats.number_of_LinphoneMessageReceived + 1, 10000));
		laureLastMsg = laure->stat.last_received_chat_message;
		if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg)) throw BCTBX_EXCEPTION;
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), paulineTextMessage);
		BC_ASSERT_TRUE(linphone_address_weak_equal(paulineAddr, linphone_chat_message_get_from_address(laureLastMsg)));
		wait_for_list(coresList, 0, 1, 4000); // Just to be sure all imdn message finally reach their recipients

		// Check that Marie and Chloe did not received the message
		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneMessageReceived,
		                initialMarieStats.number_of_LinphoneMessageReceived, int, "%d");
		BC_ASSERT_EQUAL(chloe->stat.number_of_LinphoneMessageReceived,
		                initialChloeStats.number_of_LinphoneMessageReceived, int, "%d");
		linphone_chat_message_unref(paulineMessage);

		// Stop marie and Chloe core and switch to c25519k512
		linphone_core_set_network_reachable(chloe->lc, FALSE);
		LinphoneAddress *chloeAddr = linphone_address_clone(linphone_chat_room_get_peer_address(chloeCr));
		coresList = bctbx_list_remove(coresList, chloe->lc);
		linphone_core_manager_reinit(chloe);
		tmpCoresManagerList = bctbx_list_append(NULL, chloe);
		// Set chloe curve to c25519k512, and delete its lime DB - TODO: do it via file remote provisioning
		set_lime_server_and_curve_list(C25519K512, tmpCoresManagerList);
		unlink(chloe->lime_database_path);
		tmpCoresList = init_core_for_conference(tmpCoresManagerList);
		bctbx_list_free(tmpCoresManagerList);
		coresList = bctbx_list_concat(coresList, tmpCoresList);
		linphone_core_manager_start(chloe, TRUE);
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(chloe->lc));
		chloeCr = linphone_core_search_chat_room(chloe->lc, NULL, NULL, chloeAddr, NULL);
		linphone_address_unref(chloeAddr);
		wait_for_list(coresList, 0, 1, 4000); // Make sure Chloe's core restart can complete lime update
		if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) throw BCTBX_EXCEPTION;

		linphone_address_unref(marieAddr);
		linphone_core_set_network_reachable(marie->lc, FALSE);
		marieAddr = linphone_address_clone(linphone_chat_room_get_peer_address(marieCr));
		coresList = bctbx_list_remove(coresList, marie->lc);
		linphone_core_manager_reinit(marie);
		tmpCoresManagerList = bctbx_list_append(NULL, marie);
		// Set marie curve to c25519k512, and delete its lime DB - TODO: do it via file remote provisioning
		set_lime_server_and_curve_list(C25519K512, tmpCoresManagerList);
		unlink(marie->lime_database_path);
		tmpCoresList = init_core_for_conference(tmpCoresManagerList);
		bctbx_list_free(tmpCoresManagerList);
		coresList = bctbx_list_concat(coresList, tmpCoresList);
		linphone_core_manager_start(marie, TRUE);
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
		marieCr = linphone_core_search_chat_room(marie->lc, NULL, NULL, marieAddr, NULL);
		linphone_address_unref(marieAddr);
		wait_for_list(coresList, 0, 1, 4000); // Make sure Marie's core restart can complete lime update
		if (!BC_ASSERT_PTR_NOT_NULL(marieCr)) throw BCTBX_EXCEPTION;

		// pauline sends a message, everybody should get it
		initialMarieStats = marie->stat;
		initialPaulineStats = pauline->stat;
		initialLaureStats = laure->stat;
		initialChloeStats = chloe->stat;
		const char *paulineTextMessage2 = "Hey! You're back!";
		paulineMessage = _send_message(paulineCr, paulineTextMessage2);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDelivered,
		                             initialPaulineStats.number_of_LinphoneMessageDelivered + 1,
		                             10000)); // Delivered to the server
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived,
		                             initialLaureStats.number_of_LinphoneMessageReceived + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived,
		                             initialMarieStats.number_of_LinphoneMessageReceived + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageReceived,
		                             initialChloeStats.number_of_LinphoneMessageReceived + 1, 10000));
		laureLastMsg = laure->stat.last_received_chat_message;
		if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg)) throw BCTBX_EXCEPTION;
		LinphoneChatMessage *marieLastMsg = marie->stat.last_received_chat_message;
		if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg)) throw BCTBX_EXCEPTION;
		chloeLastMsg = chloe->stat.last_received_chat_message;
		if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg)) throw BCTBX_EXCEPTION;
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), paulineTextMessage2);
		BC_ASSERT_TRUE(linphone_address_weak_equal(paulineAddr, linphone_chat_message_get_from_address(laureLastMsg)));
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marieLastMsg), paulineTextMessage2);
		BC_ASSERT_TRUE(linphone_address_weak_equal(paulineAddr, linphone_chat_message_get_from_address(marieLastMsg)));
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(chloeLastMsg), paulineTextMessage2);
		BC_ASSERT_TRUE(linphone_address_weak_equal(paulineAddr, linphone_chat_message_get_from_address(chloeLastMsg)));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDeliveredToUser,
		                             initialPaulineStats.number_of_LinphoneMessageDeliveredToUser + 1,
		                             10000)); // make sure the IMDN is back to marie
		wait_for_list(coresList, 0, 1, 4000); // Just to be sure all imdn message finally reach their recipients
		linphone_chat_message_unref(paulineMessage);

		linphone_address_unref(paulineAddr);
	} catch (BctbxException const &e) {
		lError() << e.what();
		BC_FAIL("");
	}

	wait_for_list(coresList, 0, 1, 4000); // Just to be sure all imdn message finally reach their recipients (Laure and
	                                      // Pauline receive each others IMDN)
	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(chloe);
}
/*
 * Soft migration scenario
 * - create 4 lime users on curve25519, they all register on the server
 * - they exchange messages
 * - one user stops its core, restart and switch to curve25519k512,c25519 in its settings (keep curve25519 as base)
 * - check this new lime user is registered on the lime server
 * - Exchange messages : the swithched user is able to receive the message from the non switched one
 * - switch another two users to c25519k512, c25519
 * - Exchange messages : they all an communicate
 * - switch the last users and use it to encrypt a message to everyone
 * - Exchange messages: everyone can communicate
 */
static void group_chat_lime_x3dh_soft_migration(void) {
	if (!liblinphone_tester_is_lime_PQ_available()) {
		bctbx_warning("Skip lime hard migration test as lime PQ is not supported");
		return;
	}
	const auto confFactoryUri = linphone_address_new(sFactoryUri);
	Linphone::Tester::ClientCoreManager marie("marie_rc", *(LinphonePrivate::Address::toCpp(confFactoryUri)),
	                                          {lime::CurveId::c25519});
	Linphone::Tester::ClientCoreManager pauline("pauline_rc", *(LinphonePrivate::Address::toCpp(confFactoryUri)),
	                                            {lime::CurveId::c25519});
	Linphone::Tester::ClientCoreManager laure("laure_tcp_rc", *(LinphonePrivate::Address::toCpp(confFactoryUri)),
	                                          {lime::CurveId::c25519});
	Linphone::Tester::ClientCoreManager chloe("chloe_rc", *(LinphonePrivate::Address::toCpp(confFactoryUri)),
	                                          {lime::CurveId::c25519});
	std::list<std::reference_wrapper<Linphone::Tester::CoreManager>> allCoreMgrs{marie, pauline, laure, chloe};
	std::list<std::reference_wrapper<Linphone::Tester::CoreManager>> recipients{pauline, laure, chloe};
	linphone_address_unref(confFactoryUri);
	auto marieStats = marie.getStats();
	auto paulineStats = pauline.getStats();
	auto laureStats = laure.getStats();
	auto chloeStats = chloe.getStats();
	BC_ASSERT_TRUE(marie.assertPtrValue(&(marieStats.number_of_X3dhUserCreationSuccess), 1));
	BC_ASSERT_TRUE(pauline.assertPtrValue(&(paulineStats.number_of_X3dhUserCreationSuccess), 1));
	BC_ASSERT_TRUE(laure.assertPtrValue(&(laureStats.number_of_X3dhUserCreationSuccess), 1));
	BC_ASSERT_TRUE(chloe.assertPtrValue(&(chloeStats.number_of_X3dhUserCreationSuccess), 1));

	bctbx_list_t *coresList = bctbx_list_append(NULL, marie.getLc());
	coresList = bctbx_list_append(coresList, laure.getLc());
	coresList = bctbx_list_append(coresList, pauline.getLc());
	coresList = bctbx_list_append(coresList, chloe.getLc());

	bctbx_list_t *participantsAddresses = NULL;
	auto laureAddr = laure.getIdentity();
	auto paulineAddr = pauline.getIdentity();
	auto chloeAddr = chloe.getIdentity();
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(laureAddr.toC()));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(paulineAddr.toC()));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(chloeAddr.toC()));

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie.getCMgr(), &marieStats, participantsAddresses, initialSubject,
	                                 TRUE, LinphoneChatRoomEphemeralModeDeviceManaged);
	LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));
	if (!marieCr) {
		BC_FAIL("failed to create chatroom");
		return;
	}

	// Check that the chat room is correctly created on other participants side
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &paulineStats,
	                                                                   confAddr, initialSubject, 3, FALSE);
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure.getCMgr(), &laureStats, confAddr,
	                                                                 initialSubject, 3, FALSE);
	LinphoneChatRoom *chloeCr = check_creation_chat_room_client_side(coresList, chloe.getCMgr(), &chloeStats, confAddr,
	                                                                 initialSubject, 3, FALSE);

	if (!paulineCr || !laureCr || !chloeCr) {
		BC_FAIL("failed to create chatroom");
		return;
	}

	// Marie send a message to the group
	std::string msgText("Hello");
	LinphoneChatMessage *msg = Linphone::Tester::ConfCoreManager::sendTextMsg(marieCr, msgText);

	// Message is delivered
	BC_ASSERT_TRUE(Linphone::Tester::CoreManagerAssert(allCoreMgrs).wait([msg] {
		return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
	}));

	linphone_chat_message_unref(msg);
	msg = nullptr;

	// everyone get it
	for (Linphone::Tester::CoreManager &client : recipients) {
		for (auto chatRoom : client.getCore().getChatRooms()) {
			BC_ASSERT_TRUE(Linphone::Tester::CoreManagerAssert(allCoreMgrs).wait([chatRoom] {
				return chatRoom->getUnreadChatMessageCount() == 1;
			}));
			LinphoneChatMessage *lastMsg = client.getStats().last_received_chat_message;
			BC_ASSERT_PTR_NOT_NULL(lastMsg);
			if (lastMsg) {
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(lastMsg), msgText.c_str());
			}
			chatRoom->markAsRead();
		}
	}
	// wait a bit longer to allow imdn exchanges
	Linphone::Tester::CoreManagerAssert(allCoreMgrs).waitUntil(std::chrono::seconds(3), [] { return false; });

	// ReStart Pauline core with c25519k512,c25519
	pauline.set_limeAlgoList({lime::CurveId::c25519k512, lime::CurveId::c25519});
	coresList = bctbx_list_remove(coresList, pauline.getLc());
	pauline.reStart();
	coresList = bctbx_list_append(coresList, pauline.getLc());
	paulineStats = pauline.getStats();
	BC_ASSERT_TRUE(pauline.assertPtrValue(&(paulineStats.number_of_X3dhUserCreationSuccess), 1));

	// Marie send a message to the group
	msgText = std::string("Pauline are you there?");
	msg = Linphone::Tester::ConfCoreManager::sendTextMsg(marieCr, msgText);

	// Message is delivered
	BC_ASSERT_TRUE(Linphone::Tester::CoreManagerAssert(allCoreMgrs).wait([msg] {
		return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
	}));

	linphone_chat_message_unref(msg);
	msg = nullptr;

	// everyone get it
	for (Linphone::Tester::CoreManager &client : recipients) {
		for (auto chatRoom : client.getCore().getChatRooms()) {
			BC_ASSERT_TRUE(Linphone::Tester::CoreManagerAssert(allCoreMgrs).wait([chatRoom] {
				return chatRoom->getUnreadChatMessageCount() == 1;
			}));
			LinphoneChatMessage *lastMsg = client.getStats().last_received_chat_message;
			BC_ASSERT_PTR_NOT_NULL(lastMsg);
			if (lastMsg) {
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(lastMsg), msgText.c_str());
			}
			chatRoom->markAsRead();
		}
	}
	// wait a bit longer to allow imdn exchanges
	Linphone::Tester::CoreManagerAssert(allCoreMgrs).waitUntil(std::chrono::seconds(3), [] { return false; });

	// ReStart Laure and Chloe core with c25519k512,c25519
	laure.set_limeAlgoList({lime::CurveId::c25519k512, lime::CurveId::c25519});
	coresList = bctbx_list_remove(coresList, laure.getLc());
	laure.reStart();
	coresList = bctbx_list_append(coresList, laure.getLc());
	laureStats = laure.getStats();
	BC_ASSERT_TRUE(laure.assertPtrValue(&(laureStats.number_of_X3dhUserCreationSuccess), 1));

	chloe.set_limeAlgoList({lime::CurveId::c25519k512, lime::CurveId::c25519});
	coresList = bctbx_list_remove(coresList, chloe.getLc());
	chloe.reStart();
	coresList = bctbx_list_append(coresList, chloe.getLc());
	chloeStats = chloe.getStats();
	BC_ASSERT_TRUE(chloe.assertPtrValue(&(chloeStats.number_of_X3dhUserCreationSuccess), 1));

	// Marie send a message to the group
	msgText = std::string("Girls shall I migrate too?");
	msg = Linphone::Tester::ConfCoreManager::sendTextMsg(marieCr, msgText);

	// Message is delivered
	BC_ASSERT_TRUE(Linphone::Tester::CoreManagerAssert(allCoreMgrs).wait([msg] {
		return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
	}));

	linphone_chat_message_unref(msg);
	msg = nullptr;

	// everyone get it
	for (Linphone::Tester::CoreManager &client : recipients) {
		for (auto chatRoom : client.getCore().getChatRooms()) {
			BC_ASSERT_TRUE(Linphone::Tester::CoreManagerAssert(allCoreMgrs).wait([chatRoom] {
				return chatRoom->getUnreadChatMessageCount() == 1;
			}));
			LinphoneChatMessage *lastMsg = client.getStats().last_received_chat_message;
			BC_ASSERT_PTR_NOT_NULL(lastMsg);
			if (lastMsg) {
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(lastMsg), msgText.c_str());
			}
			chatRoom->markAsRead();
		}
	}
	// wait a bit longer to allow imdn exchanges
	Linphone::Tester::CoreManagerAssert(allCoreMgrs).waitUntil(std::chrono::seconds(3), [] { return false; });

	// ReStart Marie core with c25519k512,c25519
	marie.set_limeAlgoList({lime::CurveId::c25519k512, lime::CurveId::c25519});
	coresList = bctbx_list_remove(coresList, marie.getLc());
	marie.reStart();
	coresList = bctbx_list_append(coresList, marie.getLc());
	marieStats = marie.getStats();
	BC_ASSERT_TRUE(marie.assertPtrValue(&(marieStats.number_of_X3dhUserCreationSuccess), 1));
	// get back marieCr
	LinphoneAddress *marieDeviceAddr =
	    linphone_address_clone(linphone_proxy_config_get_contact(marie.getDefaultProxyConfig()));
	marieCr = marie.searchChatRoom(marieDeviceAddr, confAddr);
	BC_ASSERT_PTR_NOT_NULL(marieCr);
	linphone_address_unref(marieDeviceAddr);
	// Marie send a message to the group
	msgText = std::string("Here I am");
	msg = Linphone::Tester::ConfCoreManager::sendTextMsg(marieCr, msgText);

	// Message is delivered
	BC_ASSERT_TRUE(Linphone::Tester::CoreManagerAssert(allCoreMgrs).wait([msg] {
		return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
	}));

	linphone_chat_message_unref(msg);
	msg = nullptr;

	// everyone get it
	for (Linphone::Tester::CoreManager &client : recipients) {
		for (auto chatRoom : client.getCore().getChatRooms()) {
			BC_ASSERT_TRUE(Linphone::Tester::CoreManagerAssert(allCoreMgrs).wait([chatRoom] {
				return chatRoom->getUnreadChatMessageCount() == 1;
			}));
			LinphoneChatMessage *lastMsg = client.getStats().last_received_chat_message;
			BC_ASSERT_PTR_NOT_NULL(lastMsg);
			if (lastMsg) {
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(lastMsg), msgText.c_str());
			}
			chatRoom->markAsRead();
		}
	}

	// wait a bit longer to allow imdn exchanges
	Linphone::Tester::CoreManagerAssert(allCoreMgrs).waitUntil(std::chrono::seconds(3), [] { return false; });

	// ReStart Marie, Laure and Chloe core with c25519mlk512,c25519k512 (drop c25519)
	laure.set_limeAlgoList({lime::CurveId::c25519mlk512, lime::CurveId::c25519k512});
	coresList = bctbx_list_remove(coresList, laure.getLc());
	laure.reStart();
	coresList = bctbx_list_append(coresList, laure.getLc());
	laureStats = laure.getStats();
	BC_ASSERT_TRUE(laure.assertPtrValue(&(laureStats.number_of_X3dhUserCreationSuccess), 1));

	chloe.set_limeAlgoList({lime::CurveId::c25519mlk512, lime::CurveId::c25519k512});
	coresList = bctbx_list_remove(coresList, chloe.getLc());
	chloe.reStart();
	coresList = bctbx_list_append(coresList, chloe.getLc());
	chloeStats = chloe.getStats();
	BC_ASSERT_TRUE(chloe.assertPtrValue(&(chloeStats.number_of_X3dhUserCreationSuccess), 1));

	marie.set_limeAlgoList({lime::CurveId::c25519mlk512, lime::CurveId::c25519k512});
	coresList = bctbx_list_remove(coresList, marie.getLc());
	marie.reStart();
	coresList = bctbx_list_append(coresList, marie.getLc());
	marieStats = marie.getStats();
	BC_ASSERT_TRUE(marie.assertPtrValue(&(marieStats.number_of_X3dhUserCreationSuccess), 1));

	// get back marieCr
	marieDeviceAddr = linphone_address_clone(linphone_proxy_config_get_contact(marie.getDefaultProxyConfig()));
	marieCr = marie.searchChatRoom(marieDeviceAddr, confAddr);
	BC_ASSERT_PTR_NOT_NULL(marieCr);
	linphone_address_unref(marieDeviceAddr);

	// Marie send a message to the group
	msgText = std::string("To infinity and beyond");
	msg = Linphone::Tester::ConfCoreManager::sendTextMsg(marieCr, msgText);

	// Message is delivered
	BC_ASSERT_TRUE(Linphone::Tester::CoreManagerAssert(allCoreMgrs).wait([msg] {
		return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
	}));

	linphone_chat_message_unref(msg);
	msg = nullptr;

	// everyone get it
	for (Linphone::Tester::CoreManager &client : recipients) {
		for (auto chatRoom : client.getCore().getChatRooms()) {
			BC_ASSERT_TRUE(Linphone::Tester::CoreManagerAssert(allCoreMgrs).wait([chatRoom] {
				return chatRoom->getUnreadChatMessageCount() == 1;
			}));
			LinphoneChatMessage *lastMsg = client.getStats().last_received_chat_message;
			BC_ASSERT_PTR_NOT_NULL(lastMsg);
			if (lastMsg) {
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(lastMsg), msgText.c_str());
			}
			chatRoom->markAsRead();
		}
	}

	// wait a bit longer to allow imdn exchanges
	Linphone::Tester::CoreManagerAssert(allCoreMgrs).waitUntil(std::chrono::seconds(3), [] { return false; });

	// ReStart Marie, Laure and Chloe core with c25519mlk512 only (drop c25519k512) -> pauline won't be able to get the
	// message
	laure.set_limeAlgoList({lime::CurveId::c25519mlk512});
	coresList = bctbx_list_remove(coresList, laure.getLc());
	laure.reStart();
	coresList = bctbx_list_append(coresList, laure.getLc());
	laureStats = laure.getStats();
	BC_ASSERT_TRUE(laure.assertPtrValue(&(laureStats.number_of_X3dhUserCreationSuccess), 1));

	chloe.set_limeAlgoList({lime::CurveId::c25519mlk512});
	coresList = bctbx_list_remove(coresList, chloe.getLc());
	chloe.reStart();
	coresList = bctbx_list_append(coresList, chloe.getLc());
	chloeStats = chloe.getStats();
	BC_ASSERT_TRUE(chloe.assertPtrValue(&(chloeStats.number_of_X3dhUserCreationSuccess), 1));

	marie.set_limeAlgoList({lime::CurveId::c25519mlk512});
	coresList = bctbx_list_remove(coresList, marie.getLc());
	marie.reStart();
	coresList = bctbx_list_append(coresList, marie.getLc());
	marieStats = marie.getStats();
	BC_ASSERT_TRUE(marie.assertPtrValue(&(marieStats.number_of_X3dhUserCreationSuccess), 1));

	// get back marieCr
	marieDeviceAddr = linphone_address_clone(linphone_proxy_config_get_contact(marie.getDefaultProxyConfig()));
	marieCr = marie.searchChatRoom(marieDeviceAddr, confAddr);
	BC_ASSERT_PTR_NOT_NULL(marieCr);
	linphone_address_unref(marieDeviceAddr);

	// Marie send a message to the group
	msgText = std::string("Bye Pauline");
	msg = Linphone::Tester::ConfCoreManager::sendTextMsg(marieCr, msgText);

	// Message is delivered
	BC_ASSERT_TRUE(Linphone::Tester::CoreManagerAssert(allCoreMgrs).wait([msg] {
		return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
	}));

	linphone_chat_message_unref(msg);
	msg = nullptr;

	// Laure and Chloe get it
	for (Linphone::Tester::CoreManager &client :
	     std::list<std::reference_wrapper<Linphone::Tester::CoreManager>>{laure, chloe}) {
		for (auto chatRoom : client.getCore().getChatRooms()) {
			BC_ASSERT_TRUE(Linphone::Tester::CoreManagerAssert(allCoreMgrs).wait([chatRoom] {
				return chatRoom->getUnreadChatMessageCount() == 1;
			}));
			LinphoneChatMessage *lastMsg = client.getStats().last_received_chat_message;
			BC_ASSERT_PTR_NOT_NULL(lastMsg);
			if (lastMsg) {
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(lastMsg), msgText.c_str());
			}
			chatRoom->markAsRead();
		}
	}

	// wait a bit longer to allow imdn exchanges
	Linphone::Tester::CoreManagerAssert(allCoreMgrs).waitUntil(std::chrono::seconds(3), [] { return false; });

	// Pauline does not
	BC_ASSERT_EQUAL(pauline.getCore().getChatRooms().front()->getUnreadChatMessageCount(), 0, int, "%d");

	linphone_address_unref(confAddr);
	bctbx_list_free(coresList);
}

/**
 * - Create 4 users, marie supporting c25519k512, c448 and c25519, the other three only one of them (all support also
 * c25519 as a fallback as it is the default core setting)
 * - marie encrypts to all of them
 * - check they can all retrieve the message
 * - check their status : they all should be untrusted
 * - marie place calls to each of them and verify SAS
 * - from Marie's perspective, the chatroom is trusted now
 * - chloe migrates to c25519k512 -> status is still trusted as Marie doesn't know it
 * - Marie sends a new message, she now knows about Chloe 'new' device -> back to untrusted
 * - place a call with SAS
 * - chatroom is back to trusted
 */
static void group_chat_lime_x3dh_multialgo(void) {
	if (!liblinphone_tester_is_lime_PQ_available()) {
		bctbx_warning("Skip lime multialgo test as lime PQ is not supported");
		return;
	}
	const auto confFactoryUri = linphone_address_new(sFactoryUri);
	Linphone::Tester::ClientCoreManager marie("marie_rc", *(LinphonePrivate::Address::toCpp(confFactoryUri)),
	                                          {lime::CurveId::c25519k512, lime::CurveId::c448, lime::CurveId::c25519});
	Linphone::Tester::ClientCoreManager pauline("pauline_rc", *(LinphonePrivate::Address::toCpp(confFactoryUri)),
	                                            {lime::CurveId::c25519k512, lime::CurveId::c25519});
	Linphone::Tester::ClientCoreManager laure("laure_tcp_rc", *(LinphonePrivate::Address::toCpp(confFactoryUri)),
	                                          {lime::CurveId::c448, lime::CurveId::c25519});
	Linphone::Tester::ClientCoreManager chloe("chloe_rc", *(LinphonePrivate::Address::toCpp(confFactoryUri)),
	                                          {lime::CurveId::c25519});
	std::list<std::reference_wrapper<Linphone::Tester::CoreManager>> allCoreMgrs{marie, pauline, laure, chloe};
	std::list<std::reference_wrapper<Linphone::Tester::CoreManager>> recipients{pauline, laure, chloe};
	linphone_address_unref(confFactoryUri);
	auto marieStats = marie.getStats();
	auto paulineStats = pauline.getStats();
	auto laureStats = laure.getStats();
	auto chloeStats = chloe.getStats();
	BC_ASSERT_TRUE(marie.assertPtrValue(&(marieStats.number_of_X3dhUserCreationSuccess), 1));
	BC_ASSERT_TRUE(pauline.assertPtrValue(&(paulineStats.number_of_X3dhUserCreationSuccess), 1));
	BC_ASSERT_TRUE(laure.assertPtrValue(&(laureStats.number_of_X3dhUserCreationSuccess), 1));
	BC_ASSERT_TRUE(chloe.assertPtrValue(&(chloeStats.number_of_X3dhUserCreationSuccess), 1));

	bctbx_list_t *coresList = bctbx_list_append(NULL, marie.getLc());
	coresList = bctbx_list_append(coresList, laure.getLc());
	coresList = bctbx_list_append(coresList, pauline.getLc());
	coresList = bctbx_list_append(coresList, chloe.getLc());

	bctbx_list_t *participantsAddresses = NULL;
	auto laureAddr = laure.getIdentity();
	auto paulineAddr = pauline.getIdentity();
	auto chloeAddr = chloe.getIdentity();
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(laureAddr.toC()));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(paulineAddr.toC()));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(chloeAddr.toC()));

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie.getCMgr(), &marieStats, participantsAddresses, initialSubject,
	                                 TRUE, LinphoneChatRoomEphemeralModeDeviceManaged);
	LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));
	if (!marieCr) {
		BC_FAIL("failed to create chatroom");
		return;
	}

	// Check that the chat room is correctly created on other participants side
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &paulineStats,
	                                                                   confAddr, initialSubject, 3, FALSE);
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure.getCMgr(), &laureStats, confAddr,
	                                                                 initialSubject, 3, FALSE);
	LinphoneChatRoom *chloeCr = check_creation_chat_room_client_side(coresList, chloe.getCMgr(), &chloeStats, confAddr,
	                                                                 initialSubject, 3, FALSE);

	if (!paulineCr || !laureCr || !chloeCr) {
		BC_FAIL("failed to create chatroom");
		return;
	}

	// Marie send a message to the group
	std::string msgText("Hello");
	LinphoneChatMessage *msg = Linphone::Tester::ConfCoreManager::sendTextMsg(marieCr, msgText);

	// Message is delivered
	BC_ASSERT_TRUE(Linphone::Tester::CoreManagerAssert(allCoreMgrs).wait([msg] {
		return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
	}));

	linphone_chat_message_unref(msg);
	msg = nullptr;

	// everyone get it
	for (Linphone::Tester::CoreManager &client : recipients) {
		for (auto chatRoom : client.getCore().getChatRooms()) {
			BC_ASSERT_TRUE(Linphone::Tester::CoreManagerAssert(allCoreMgrs).wait([chatRoom] {
				return chatRoom->getUnreadChatMessageCount() == 1;
			}));
			LinphoneChatMessage *lastMsg = client.getStats().last_received_chat_message;
			BC_ASSERT_PTR_NOT_NULL(lastMsg);
			if (lastMsg) {
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(lastMsg), msgText.c_str());
			}
			chatRoom->markAsRead();
		}
	}
	// wait a bit longer to allow imdn exchanges
	Linphone::Tester::CoreManagerAssert(allCoreMgrs).waitUntil(std::chrono::seconds(3), [] { return false; });

	// Check chatroom status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

	// Call each other with SAS validation
	BC_ASSERT_TRUE(marie.callWithSASvalidation(pauline));
	BC_ASSERT_TRUE(marie.callWithSASvalidation(laure));
	BC_ASSERT_TRUE(marie.callWithSASvalidation(chloe));

	// Check chatroom status -> safe now
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");

	// Chloe update to c25519k512/c25519
	chloe.set_limeAlgoList({lime::CurveId::c25519k512, lime::CurveId::c25519});
	coresList = bctbx_list_remove(coresList, chloe.getLc());
	chloe.reStart();
	coresList = bctbx_list_append(coresList, chloe.getLc());
	chloeStats = chloe.getStats();
	BC_ASSERT_TRUE(chloe.assertPtrValue(&(chloeStats.number_of_X3dhUserCreationSuccess), 1));

	// Check chatroom status -> still safe as marie doesn't know chloe has a new device
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");

	// Marie sends a message to the group
	msgText = std::string("Hello again");
	msg = Linphone::Tester::ConfCoreManager::sendTextMsg(marieCr, msgText);

	// Message is delivered
	BC_ASSERT_TRUE(Linphone::Tester::CoreManagerAssert(allCoreMgrs).wait([msg] {
		return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
	}));

	linphone_chat_message_unref(msg);
	msg = nullptr;

	// everyone get it
	for (Linphone::Tester::CoreManager &client : recipients) {
		for (auto chatRoom : client.getCore().getChatRooms()) {
			BC_ASSERT_TRUE(Linphone::Tester::CoreManagerAssert(allCoreMgrs).wait([chatRoom] {
				return chatRoom->getUnreadChatMessageCount() == 1;
			}));
			LinphoneChatMessage *lastMsg = client.getStats().last_received_chat_message;
			BC_ASSERT_PTR_NOT_NULL(lastMsg);
			if (lastMsg) {
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(lastMsg), msgText.c_str());
			}
			chatRoom->markAsRead();
		}
	}

	// wait a bit longer to allow imdn exchanges
	Linphone::Tester::CoreManagerAssert(allCoreMgrs).waitUntil(std::chrono::seconds(3), [] { return false; });

	// Check chatroom status -> now marie has the new chloe device as active -> chatroom is back to encrypted
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

	// marie calls chloe so she validates her new key
	BC_ASSERT_TRUE(marie.callWithSASvalidation(chloe));

	// Check chatroom status -> chatroom is back to safe
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");

	linphone_address_unref(confAddr);
	bctbx_list_free(coresList);
}

test_t secure_group_chat_multialgos_tests[] = {
    TEST_TWO_TAGS("LIME X3DH multialgo support",
                  group_chat_lime_x3dh_multialgo,
                  "LimeX3DH",
                  "LeaksMemory" /*due to core restart*/),
};

test_suite_t secure_group_chat_multialgos_test_suite = {"Secure group chat (Lime multialgos)",
                                                        NULL,
                                                        NULL,
                                                        liblinphone_tester_before_each,
                                                        liblinphone_tester_after_each,
                                                        sizeof(secure_group_chat_multialgos_tests) /
                                                            sizeof(secure_group_chat_multialgos_tests[0]),
                                                        secure_group_chat_multialgos_tests,
                                                        0};

test_t secure_group_chat_migration_tests[] = {
    TEST_TWO_TAGS("LIME X3DH hard migration x25519 to x25519k512",
                  group_chat_lime_x3dh_hard_migration,
                  "LimeX3DH",
                  "LeaksMemory" /*due to core restart*/),
    TEST_TWO_TAGS("LIME X3DH soft migration x25519 to x25519k512",
                  group_chat_lime_x3dh_soft_migration,
                  "LimeX3DH",
                  "LeaksMemory" /*due to core restart*/),
};

test_suite_t secure_group_chat_migration_test_suite = {"Secure group chat (Lime migration)",
                                                       NULL,
                                                       NULL,
                                                       liblinphone_tester_before_each,
                                                       liblinphone_tester_after_each,
                                                       sizeof(secure_group_chat_migration_tests) /
                                                           sizeof(secure_group_chat_migration_tests[0]),
                                                       secure_group_chat_migration_tests,
                                                       0};
#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic pop
#endif
