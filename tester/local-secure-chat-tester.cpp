/*
 * copyright (c) 2010-2023 belledonne communications sarl.
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

#ifdef HAVE_SOCI
#include <soci/soci.h>
#endif // HAVE_SOCI

#include "conference/conference.h"
#include "conference/participant.h"
#include "core/core-p.h"
#include "liblinphone_tester.h"
#include "linphone/api/c-chat-room.h"
#include "linphone/chat.h"
#include "local-conference-tester-functions.h"

namespace LinphoneTest {

static void secure_group_chat_room_with_client_restart(void) {
	group_chat_room_with_client_restart_base(true);
}

static void secure_group_chat_room_with_invite_error(void) {
	group_chat_room_with_sip_errors_base(true, false, true);
}

static void secure_group_chat_room_with_subscribe_error(void) {
	group_chat_room_with_sip_errors_base(false, true, true);
}

static void secure_group_chat_room_with_chat_room_deleted_before_server_restart(void) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), true);
		ClientConference marie2("marie_rc", focus.getConferenceFactoryAddress(), true);
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress(), true);
		ClientConference michelle2("michelle_rc", focus.getConferenceFactoryAddress(), true);

		stats initialFocusStats = focus.getStats();
		stats initialMarieStats = marie.getStats();
		stats initialMarie2Stats = marie2.getStats();
		stats initialMichelleStats = michelle.getStats();
		stats initialMichelle2Stats = michelle2.getStats();

		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, marie2.getLc());
		coresList = bctbx_list_append(coresList, michelle.getLc());
		coresList = bctbx_list_append(coresList, michelle2.getLc());

		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie2.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle2.getLc()));

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(marie2);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(michelle2);

		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie2.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(michelle.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(michelle2.getLc()));

		bctbx_list_t *participantsAddresses = NULL;
		Address michelleAddr = michelle.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(michelleAddr.toC()));
		Address michelle2Addr = michelle2.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(michelle2Addr.toC()));

		const char *initialSubject = "Colleagues (characters: $ £ çà)";

		LinphoneChatRoomParams *params = linphone_core_create_default_chat_room_params(marie.getLc());
		linphone_chat_room_params_enable_encryption(params, TRUE);
		linphone_chat_room_params_set_ephemeral_mode(params, LinphoneChatRoomEphemeralModeDeviceManaged);
		linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendFlexisipChat);
		linphone_chat_room_params_enable_group(params, FALSE);
		LinphoneChatRoom *marieCr =
		    linphone_core_create_chat_room_2(marie.getLc(), params, initialSubject, participantsAddresses);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		linphone_chat_room_params_unref(params);
		if (!marieCr) {
			return;
		}

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 1;
		}));

		for (auto chatRoom : focus.getCore().getChatRooms()) {
			linphone_chat_room_set_user_data(chatRoom->toC(), marie.getCMgr());
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_chat_room_participants_added,
		                             initialFocusStats.number_of_chat_room_participants_added + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_chat_room_participant_devices_added,
		                             initialFocusStats.number_of_chat_room_participant_devices_added + 2, 5000));

		check_create_chat_room_client_side(coresList, marie.getCMgr(), marieCr, &initialMarieStats,
		                                   participantsAddresses, initialSubject, 1);

		const LinphoneAddress *confAddr = marieCr ? linphone_chat_room_get_conference_address(marieCr) : NULL;
		// Check that the chat room is correctly created on Pauline's and Michelle's side and that the participants are
		// added
		LinphoneChatRoom *marie2Cr = check_creation_chat_room_client_side(
		    coresList, marie2.getCMgr(), &initialMarie2Stats, confAddr, initialSubject, 1, FALSE);
		LinphoneChatRoom *michelleCr = check_creation_chat_room_client_side(
		    coresList, michelle.getCMgr(), &initialMichelleStats, confAddr, initialSubject, 1, FALSE);
		LinphoneChatRoom *michelle2Cr = check_creation_chat_room_client_side(
		    coresList, michelle2.getCMgr(), &initialMichelle2Stats, confAddr, initialSubject, 1, FALSE);

		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialMichelleStats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialMichelle2Stats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialMarieStats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie2.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialMarie2Stats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));

		// Send a few messages
		std::string msg_text = "message marie2 blabla";
		LinphoneChatMessage *msg = ClientConference::sendTextMsg(marie2Cr, msg_text);
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([michelleCr] {
			return linphone_chat_room_get_unread_messages_count(michelleCr) == 1;
		}));
		LinphoneChatMessage *michelleLastMsg = michelle.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(michelleLastMsg);
		if (michelleLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(michelleLastMsg), msg_text.c_str());
		}
		linphone_chat_room_mark_as_read(michelleCr);

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([michelle2Cr] {
			return linphone_chat_room_get_unread_messages_count(michelle2Cr) == 1;
		}));
		LinphoneChatMessage *michelle2LastMsg = michelle2.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(michelle2LastMsg);
		if (michelle2LastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(michelle2LastMsg), msg_text.c_str());
		}
		linphone_chat_room_mark_as_read(michelle2Cr);
		linphone_chat_room_mark_as_read(marieCr);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie2.getStats().number_of_LinphoneMessageDisplayed,
		                             initialMarie2Stats.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));

		linphone_chat_message_unref(msg);
		msg = nullptr;

		msg_text = "message marie blabla";
		msg = ClientConference::sendTextMsg(marieCr, msg_text);
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([michelleCr] {
			return linphone_chat_room_get_unread_messages_count(michelleCr) == 1;
		}));
		michelleLastMsg = michelle.getStats().last_received_chat_message;
		if (michelleLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(michelleLastMsg), msg_text.c_str());
		}

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([michelle2Cr] {
			return linphone_chat_room_get_unread_messages_count(michelle2Cr) == 1;
		}));
		michelle2LastMsg = michelle2.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(michelle2LastMsg);
		if (michelle2LastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(michelle2LastMsg), msg_text.c_str());
		}

		linphone_chat_room_mark_as_read(marie2Cr);
		linphone_chat_room_mark_as_read(michelleCr);
		linphone_chat_room_mark_as_read(michelle2Cr);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDisplayed,
		                             initialMarieStats.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));
		linphone_chat_message_unref(msg);
		msg = nullptr;

		msg_text = "message michelle2 blabla";
		msg = ClientConference::sendTextMsg(michelle2Cr, msg_text);
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([marieCr] {
			return linphone_chat_room_get_unread_messages_count(marieCr) == 1;
		}));
		LinphoneChatMessage *marieLastMsg = marie.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(marieLastMsg);
		if (marieLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marieLastMsg), msg_text.c_str());
		}

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([marie2Cr] {
			return linphone_chat_room_get_unread_messages_count(marie2Cr) == 1;
		}));
		LinphoneChatMessage *marie2LastMsg = marie2.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(marie2LastMsg);
		if (marie2LastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marie2LastMsg), msg_text.c_str());
		}

		linphone_chat_room_mark_as_read(michelleCr);
		linphone_chat_room_mark_as_read(marieCr);
		linphone_chat_room_mark_as_read(marie2Cr);

		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_LinphoneMessageDisplayed,
		                             initialMichelle2Stats.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));
		linphone_chat_message_unref(msg);
		msg = nullptr;

		// Marie deletes the chat room
		char *confAddrStr = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:");
		ms_message("%s deletes chat room %s", linphone_core_get_identity(marie.getLc()), confAddrStr);
		ms_free(confAddrStr);

		linphone_core_manager_delete_chat_room(marie.getCMgr(), marieCr, coresList);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminated,
		                             initialMarieStats.number_of_LinphoneConferenceStateTerminated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateTerminated,
		                             initialMichelleStats.number_of_LinphoneConferenceStateTerminated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_LinphoneConferenceStateTerminated,
		                             initialMichelle2Stats.number_of_LinphoneConferenceStateTerminated + 1,
		                             liblinphone_tester_sip_timeout));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).waitUntil(chrono::seconds(5), [] {
			return false;
		});

		ms_message("%s is restarting its core", linphone_core_get_identity(focus.getLc()));
		coresList = bctbx_list_remove(coresList, focus.getLc());
		// Restart flexisip
		focus.reStart();
		coresList = bctbx_list_append(coresList, focus.getLc());

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).waitUntil(chrono::seconds(5), [] {
			return false;
		});

		initialMarieStats = marie.getStats();
		initialMichelleStats = michelle.getStats();
		msg_text = "Cou cou Marieeee.....";
		msg = ClientConference::sendTextMsg(michelle2Cr, msg_text);

		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialMichelleStats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialMichelle2Stats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));

		confAddr = linphone_chat_room_get_conference_address(michelleCr);
		marieCr = check_creation_chat_room_client_side(coresList, marie.getCMgr(), &initialMarieStats, confAddr,
		                                               initialSubject, 1, FALSE);
		BC_ASSERT_PTR_NOT_NULL(marieCr);

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));

		if (marieCr) {
			BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([marieCr] {
				return linphone_chat_room_get_unread_messages_count(marieCr) == 1;
			}));
		}
		marieLastMsg = marie.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(marieLastMsg);
		if (marieLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marieLastMsg), msg_text.c_str());
		}

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([marie2Cr] {
			return linphone_chat_room_get_unread_messages_count(marie2Cr) == 1;
		}));
		marie2LastMsg = marie2.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(marie2LastMsg);
		if (marie2LastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marie2LastMsg), msg_text.c_str());
		}

		if (michelleCr) {
			linphone_chat_room_mark_as_read(michelleCr);
		}
		if (marieCr) {
			linphone_chat_room_mark_as_read(marieCr);
		}
		if (marie2Cr) {
			linphone_chat_room_mark_as_read(marie2Cr);
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_LinphoneMessageDisplayed,
		                             initialMichelle2Stats.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));

		for (auto chatRoom : focus.getCore().getChatRooms()) {
			for (auto participant : chatRoom->getParticipants()) {
				//  force deletion by removing devices
				std::shared_ptr<Address> participantAddress = participant->getAddress();
				linphone_chat_room_set_participant_devices(chatRoom->toC(), participantAddress->toC(), NULL);
			}
		}

		// wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, marie2, michelle, michelle2}).waitUntil(chrono::seconds(2), [] {
			return false;
		});

		// to avoid creation attempt of a new chatroom
		LinphoneProxyConfig *config = linphone_core_get_default_proxy_config(focus.getLc());
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		bctbx_list_free(coresList);
	}
}

static void group_chat_room_lime_server_encrypted_message(void) {
	group_chat_room_lime_server_message(TRUE);
}

static void secure_one_to_one_group_chat_room_deletion_by_server_client(void) {
	one_to_one_group_chat_room_deletion_by_server_client_base(TRUE);
}

static void secure_group_chat_room_with_client_with_uppercase_username(void) {
#ifdef HAVE_SOCI
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), true);
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress(), true);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), true);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(pauline);

		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(michelle.getLc()));

		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, michelle.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		bctbx_list_t *participantsAddresses = NULL;
		Address michelleAddr = michelle.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(michelleAddr.toC()));
		Address paulineAddr = pauline.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(paulineAddr.toC()));

		stats initialMarieStats = marie.getStats();
		stats initialMichelleStats = michelle.getStats();
		stats initialPaulineStats = pauline.getStats();

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues (characters: $ £ çà)";
		LinphoneChatRoom *marieCr = create_chat_room_client_side_with_expected_number_of_participants(
		    coresList, marie.getCMgr(), &initialMarieStats, participantsAddresses, initialSubject, 2, TRUE,
		    LinphoneChatRoomEphemeralModeDeviceManaged);
		const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

		// Check that the chat room is correctly created on Michelle's side and that the participants are added
		LinphoneChatRoom *michelleCr = check_creation_chat_room_client_side(
		    coresList, michelle.getCMgr(), &initialMichelleStats, confAddr, initialSubject, 2, FALSE);

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(
		    coresList, pauline.getCMgr(), &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle}).wait([&focus] {
			for (auto chatRoom : focus.getCore().getChatRooms()) {
				for (auto participant : chatRoom->getParticipants()) {
					for (auto device : participant->getDevices())
						if (device->getState() != ParticipantDevice::State::Present) {
							return false;
						}
				}
			}
			return true;
		}));

		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneChatRoomStateCreated,
		                             initialMichelleStats.number_of_LinphoneChatRoomStateCreated + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneChatRoomStateCreated,
		                             initialPaulineStats.number_of_LinphoneChatRoomStateCreated + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated,
		                             initialMarieStats.number_of_LinphoneChatRoomStateCreated + 1,
		                             liblinphone_tester_sip_timeout));

		// Uppercase some characters of Pauline's username
		std::shared_ptr<Address> paulineUppercase = paulineAddr.clone()->toSharedPtr();
		std::string paulineUpperUsername = paulineUppercase->getUsername();
		std::transform(paulineUpperUsername.begin(), paulineUpperUsername.begin() + 2, paulineUpperUsername.begin(),
		               [](unsigned char c) { return toupper(c); });
		paulineUppercase->setUsername(paulineUpperUsername);

		auto michelleCppCr = AbstractChatRoom::toCpp(michelleCr)->getSharedFromThis();
		std::shared_ptr<Participant> paulineUppercaseParticipant =
		    Participant::create(michelleCppCr->getConference(), paulineUppercase);

		// Add PAuline's address to the DB. It is not possible to add it through insertSipAddress as it checks there is
		// a similar address using case insensitive comparison
		try {
			soci::session sql("sqlite3", michelle.getCMgr()->database_path); // open the DB
			const string sipAddress = paulineUppercase->toStringUriOnlyOrdered();
			const string displayName = paulineUppercase->getDisplayName();
			soci::indicator displayNameInd = displayName.empty() ? soci::i_null : soci::i_ok;
			sql << "INSERT INTO sip_address (value, display_name) VALUES (:sipAddress, :displayName)",
			    soci::use(sipAddress), soci::use(displayName, displayNameInd);
		} catch (std::exception &e) { // swallow any error on DB
			lWarning() << "Cannot insert address " << *paulineUppercase << " to the database "
			           << michelle.getCMgr()->database_path << ". Error is " << e.what();
		}

		auto &michelleMainDb = L_GET_PRIVATE_FROM_C_OBJECT(michelle.getLc())->mainDb;
		ms_message("%s is adding participant with address %s to chatroom %s to the database",
		           linphone_core_get_identity(michelle.getLc()),
		           paulineUppercaseParticipant->getAddress()->toString().c_str(),
		           michelleCppCr->getConferenceAddress()->toString().c_str());
		michelleMainDb->insertChatRoomParticipant(michelleCppCr, paulineUppercaseParticipant);
		for (const auto &deviceCr : michelleCppCr->getConference()
		                                ->findParticipant(Address::toCpp(paulineAddr.toC())->getSharedFromThis())
		                                ->getDevices()) {
			auto device =
			    ParticipantDevice::create(paulineUppercaseParticipant, deviceCr->getAddress(), deviceCr->getName());
			ms_message("%s is adding device with address %s to participant %s of chatroom %s to the database",
			           linphone_core_get_identity(michelle.getLc()), device->getAddress()->toString().c_str(),
			           paulineUppercaseParticipant->getAddress()->toString().c_str(),
			           michelleCppCr->getConferenceAddress()->toString().c_str());
			michelleMainDb->insertChatRoomParticipantDevice(michelleCppCr, device);
		}

		LinphoneAddress *michelleContact = linphone_account_get_contact_address(michelle.getDefaultAccount());
		char *michelleContactString = linphone_address_as_string(michelleContact);
		ms_message("%s is restarting its core", michelleContactString);
		ms_free(michelleContactString);
		coresList = bctbx_list_remove(coresList, michelle.getLc());
		michelle.reStart();
		coresList = bctbx_list_append(coresList, michelle.getLc());

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle}).wait([&michelle] {
			return checkChatroomCreation(michelle, 1);
		}));

		LinphoneAddress *michelleDeviceAddr =
		    linphone_address_clone(linphone_account_get_contact_address(michelle.getDefaultAccount()));
		michelleCr = michelle.searchChatRoom(michelleDeviceAddr, confAddr);
		BC_ASSERT_PTR_NOT_NULL(michelleCr);

		// Michelle has 3 participants: marie, pauline and a fake participant with marie's username having some
		// uppercase letter
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(michelleCr), 3, int, "%0d");
		LinphoneChatMessage *msg = linphone_chat_room_create_message_from_utf8(michelleCr, "back with you");
		linphone_chat_message_send(msg);
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, michelle, pauline}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, michelle, pauline}).wait([marieCr] {
			return linphone_chat_room_get_unread_messages_count(marieCr) == 1;
		}));
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, michelle, pauline}).wait([paulineCr] {
			return linphone_chat_room_get_unread_messages_count(paulineCr) == 1;
		}));
		linphone_chat_message_unref(msg);
		msg = NULL;

		linphone_chat_room_mark_as_read(marieCr);
		linphone_chat_room_mark_as_read(paulineCr);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDisplayed, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageDisplayed, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_NotifyFullStateReceived, 1,
		                             liblinphone_tester_sip_timeout));
		// Client and server are on the same page now. The only participants of the chat room are Marie and Pauline
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(michelleCr), 2, int, "%0d");

		for (auto chatRoom : focus.getCore().getChatRooms()) {
			for (auto participant : chatRoom->getParticipants()) {
				//  force deletion by removing devices
				std::shared_ptr<Address> participantAddress = participant->getAddress();
				linphone_chat_room_set_participant_devices(chatRoom->toC(), participantAddress->toC(), NULL);
			}
		}

		// wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, michelle}).waitUntil(chrono::seconds(2), [] { return false; });

		// to avoid creation attempt of a new chatroom
		auto focus_account = focus.getDefaultAccount();
		LinphoneAccountParams *params = linphone_account_params_clone(linphone_account_get_params(focus_account));
		linphone_account_params_set_conference_factory_uri(params, NULL);
		linphone_account_set_params(focus_account, params);
		linphone_account_params_unref(params);

		bctbx_list_free(coresList);
	}
#else  // HAVE_SOCI
	BC_PASS("Test requires to compile the core with SOCI");
#endif // HAVE_SOCI
}

static void secure_group_chat_room_with_multi_account_client(void) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), true);
		ClientConference multi_account("multi_account2_rc", focus.getConferenceFactoryAddress(), true);
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress(), true);
		ClientConference berthe("berthe_rc", focus.getConferenceFactoryAddress(), true);

		stats initialFocusStats = focus.getStats();
		stats initialMarieStats = marie.getStats();
		stats initialMultiAccountStats = multi_account.getStats();
		stats initialMichelleStats = michelle.getStats();
		stats initialBertheStats = berthe.getStats();

		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, multi_account.getLc());
		coresList = bctbx_list_append(coresList, michelle.getLc());
		coresList = bctbx_list_append(coresList, berthe.getLc());

		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(multi_account.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(berthe.getLc()));

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(multi_account);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(berthe);

		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(multi_account.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(michelle.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(berthe.getLc()));

		const bctbx_list_t *accounts = linphone_core_get_account_list(multi_account.getLc());
		BC_ASSERT_GREATER(bctbx_list_size(accounts), 1, size_t, "%zu");
		LinphoneAccount *default_account = (LinphoneAccount *)bctbx_list_get_data(accounts);
		linphone_core_set_default_account(multi_account.getLc(), default_account);
		LinphoneAccount *chatroom_account = (LinphoneAccount *)bctbx_list_get_data(bctbx_list_next(accounts));
		BC_ASSERT_PTR_NOT_NULL(chatroom_account);
		BC_ASSERT_PTR_NOT_EQUAL(chatroom_account, default_account);
		LinphoneAddress *chatroom_account_identity = linphone_address_clone(
		    linphone_account_params_get_identity_address(linphone_account_get_params(chatroom_account)));

		bctbx_list_t *participantsAddresses = NULL;
		Address michelleAddr = michelle.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(michelleAddr.toC()));
		Address bertheAddr = berthe.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(bertheAddr.toC()));
		participantsAddresses =
		    bctbx_list_append(participantsAddresses, linphone_address_ref(chatroom_account_identity));

		const char *initialSubject = "Colleagues (characters: $ £ çà)";

		LinphoneChatRoomParams *params = linphone_core_create_default_chat_room_params(marie.getLc());
		linphone_chat_room_params_enable_encryption(params, TRUE);
		linphone_chat_room_params_set_ephemeral_mode(params, LinphoneChatRoomEphemeralModeDeviceManaged);
		linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendFlexisipChat);
		linphone_chat_room_params_enable_group(params, TRUE);
		LinphoneChatRoom *marieCr =
		    linphone_core_create_chat_room_2(marie.getLc(), params, initialSubject, participantsAddresses);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		linphone_chat_room_params_unref(params);
		if (!marieCr) {
			return;
		}

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, multi_account, michelle, berthe}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 1;
		}));

		for (auto chatRoom : focus.getCore().getChatRooms()) {
			linphone_chat_room_set_user_data(chatRoom->toC(), marie.getCMgr());
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_chat_room_participants_added,
		                             initialFocusStats.number_of_chat_room_participants_added + 3, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_chat_room_participant_devices_added,
		                             initialFocusStats.number_of_chat_room_participant_devices_added + 3, 5000));

		check_create_chat_room_client_side(coresList, marie.getCMgr(), marieCr, &initialMarieStats,
		                                   participantsAddresses, initialSubject, 3);

		const LinphoneAddress *confAddr = marieCr ? linphone_chat_room_get_conference_address(marieCr) : NULL;
		// Check that the chat room is correctly created on MultiAccount's and Michelle's side and that the participants
		// are added
		LinphoneChatRoom *michelleCr = check_creation_chat_room_client_side(
		    coresList, michelle.getCMgr(), &initialMichelleStats, confAddr, initialSubject, 3, FALSE);
		LinphoneChatRoom *bertheCr = check_creation_chat_room_client_side(
		    coresList, berthe.getCMgr(), &initialBertheStats, confAddr, initialSubject, 3, FALSE);

		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialMichelleStats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialBertheStats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialMarieStats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &multi_account.getStats().number_of_LinphoneConferenceStateCreated,
		                             initialMultiAccountStats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));

		LinphoneAddress *multiAccountDeviceAddr = linphone_account_get_contact_address(chatroom_account);
		LinphoneChatRoom *multiAccountCr = multi_account.searchChatRoom(multiAccountDeviceAddr, confAddr);
		BC_ASSERT_PTR_NOT_NULL(multiAccountCr);

		// Verify that the chatroom is associated to the right account
		for (auto chatRoom : multi_account.getCore().getChatRooms()) {
			const auto &conference = chatRoom->getConference();
			const LinphoneAddress *conference_params_accont_identity =
			    conference->getAccount()->getAccountParams()->getIdentityAddress()->toC();
			BC_ASSERT_TRUE(linphone_address_equal(chatroom_account_identity, conference_params_accont_identity));
		}

		ms_message("Multi account is restarting its core");
		coresList = bctbx_list_remove(coresList, multi_account.getLc());
		multi_account.reStart();
		coresList = bctbx_list_append(coresList, multi_account.getLc());

		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(multi_account.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(multi_account.getLc()));

		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneChatRoomStateCreated, 1,
		                             liblinphone_tester_sip_timeout));

		LinphoneAccount *new_chatroom_account =
		    linphone_core_lookup_known_account(multi_account.getLc(), chatroom_account_identity);
		BC_ASSERT_PTR_NOT_NULL(new_chatroom_account);

		multiAccountDeviceAddr = linphone_account_get_contact_address(new_chatroom_account);
		multiAccountCr = multi_account.searchChatRoom(multiAccountDeviceAddr, confAddr);
		BC_ASSERT_PTR_NOT_NULL(multiAccountCr);

		// Verify that the chatroom is associated to the right account after restart
		for (auto chatRoom : multi_account.getCore().getChatRooms()) {
			const auto &conference = chatRoom->getConference();
			const LinphoneAddress *conference_params_accont_identity =
			    conference->getAccount()->getAccountParams()->getIdentityAddress()->toC();
			BC_ASSERT_TRUE(linphone_address_equal(chatroom_account_identity, conference_params_accont_identity));
		}

		// Send a few messages
		std::string msg_text = "message multi_account blabla";
		LinphoneChatMessage *msg = ClientConference::sendTextMsg(marieCr, msg_text);
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, multi_account, michelle, berthe}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, multi_account, michelle, berthe}).wait([michelleCr] {
			return linphone_chat_room_get_unread_messages_count(michelleCr) == 1;
		}));
		LinphoneChatMessage *michelleLastMsg = michelle.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(michelleLastMsg);
		if (michelleLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(michelleLastMsg), msg_text.c_str());
		}
		linphone_chat_room_mark_as_read(michelleCr);

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, multi_account, michelle, berthe}).wait([bertheCr] {
			return linphone_chat_room_get_unread_messages_count(bertheCr) == 1;
		}));
		LinphoneChatMessage *bertheLastMsg = berthe.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(bertheLastMsg);
		if (bertheLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(bertheLastMsg), msg_text.c_str());
		}
		linphone_chat_room_mark_as_read(bertheCr);

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, multi_account, michelle, berthe}).wait([multiAccountCr] {
			return linphone_chat_room_get_unread_messages_count(multiAccountCr) == 1;
		}));
		LinphoneChatMessage *multiAccountLastMsg = multi_account.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(multiAccountLastMsg);
		if (multiAccountLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(multiAccountLastMsg), msg_text.c_str());
		}
		linphone_chat_room_mark_as_read(multiAccountCr);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDisplayed,
		                             initialMarieStats.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));

		linphone_chat_message_unref(msg);
		for (auto chatRoom : focus.getCore().getChatRooms()) {
			for (auto participant : chatRoom->getParticipants()) {
				//  force deletion by removing devices
				std::shared_ptr<Address> participantAddress = participant->getAddress();
				linphone_chat_room_set_participant_devices(chatRoom->toC(), participantAddress->toC(), NULL);
			}
		}

		// wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, multi_account, michelle, berthe}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, multi_account, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
			return false;
		});

		// to avoid creation attempt of a new chatroom
		LinphoneProxyConfig *config = linphone_core_get_default_proxy_config(focus.getLc());
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		linphone_address_unref(chatroom_account_identity);
		bctbx_list_free(coresList);
	}
}

static void secure_one_to_one_chat_room_with_client_removed_from_database(void) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		bool_t encrypted = TRUE;
		linphone_core_enable_lime_x3dh(focus.getLc(), encrypted);
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), encrypted);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);

		stats initialMarieStats = marie.getStats();
		stats initialPaulineStats = pauline.getStats();
		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());

		if (encrypted) {
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline.getLc()));
		}

		Address paulineAddr = pauline.getIdentity();
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(paulineAddr.toC()));

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues";
		LinphoneChatRoom *marieCr =
		    create_chat_room_client_side(coresList, marie.getCMgr(), &initialMarieStats, participantsAddresses,
		                                 initialSubject, encrypted, LinphoneChatRoomEphemeralModeDeviceManaged);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(
		    coresList, pauline.getCMgr(), &initialPaulineStats, confAddr, initialSubject, 1, FALSE);
		BC_ASSERT_PTR_NOT_NULL(paulineCr);

		initialMarieStats = marie.getStats();
		initialPaulineStats = pauline.getStats();

		LinphoneAddress *paulineContact =
		    linphone_account_get_contact_address(linphone_core_get_default_account(pauline.getLc()));

		char *paulineContactStr = linphone_address_as_string(paulineContact);
		ms_message("All %s's devices are removed", paulineContactStr);
		ms_free(paulineContactStr);
		for (auto chatRoom : focus.getCore().getChatRooms()) {
			auto participant =
			    chatRoom->findParticipant(Address::toCpp(pauline.getCMgr()->identity)->getSharedFromThis());
			BC_ASSERT_PTR_NOT_NULL(participant);
			if (participant) {
				auto conference = chatRoom->getConference();
				BC_ASSERT_PTR_NOT_NULL(conference);
				if (conference) {
					conference->Conference::removeParticipant(participant);
				}
			}
		}

		LinphoneChatMessage *msg = linphone_chat_room_create_message_from_utf8(paulineCr, "Hello everybody");
		linphone_chat_message_send(msg);

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateNotDelivered);
		}));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneChatRoomStateTerminated,
		                             initialPaulineStats.number_of_LinphoneChatRoomStateTerminated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneChatRoomStateCreated,
		                             initialPaulineStats.number_of_LinphoneChatRoomStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));
		linphone_chat_message_unref(msg);
		msg = NULL;

		CoreManagerAssert({focus, marie, pauline}).waitUntil(std::chrono::seconds(2), [] { return false; });

		for (auto chatRoom : focus.getCore().getChatRooms()) {
			for (auto participant : chatRoom->getParticipants()) {
				//  force deletion by removing devices
				auto participantAddress = participant->getAddress();
				linphone_chat_room_set_participant_devices(chatRoom->toC(), participantAddress->toC(), NULL);
			}
		}

		// wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline}).waitUntil(chrono::seconds(2), [] { return false; });

		// to avoid creation attempt of a new chatroom
		LinphoneProxyConfig *config = linphone_core_get_default_proxy_config(focus.getLc());
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);
		bctbx_list_free(coresList);
	}
}

} // namespace LinphoneTest

static test_t local_conference_secure_chat_tests[] = {
    TEST_ONE_TAG("Secure Group chat with client restart",
                 LinphoneTest::secure_group_chat_room_with_client_restart,
                 "LeaksMemory"), /* beacause of coreMgr restart*/
    TEST_NO_TAG("Secure group chat with INVITE session error", LinphoneTest::secure_group_chat_room_with_invite_error),
    TEST_NO_TAG("Secure group chat with SUBSCRIBE session error",
                LinphoneTest::secure_group_chat_room_with_subscribe_error),
    TEST_ONE_TAG("Secure group chat with chat room deleted before server restart",
                 LinphoneTest::secure_group_chat_room_with_chat_room_deleted_before_server_restart,
                 "LeaksMemory"), /* because of network up and down */
    TEST_ONE_TAG("Secure one to one group chat deletion initiated by server and client",
                 LinphoneTest::secure_one_to_one_group_chat_room_deletion_by_server_client,
                 "LeaksMemory"), /* because of network up and down */
    TEST_ONE_TAG("Secure group chat room with client with uppercase username",
                 LinphoneTest::secure_group_chat_room_with_client_with_uppercase_username,
                 "LeaksMemory"),
    TEST_ONE_TAG("Secure group chat room with multi account client",
                 LinphoneTest::secure_group_chat_room_with_multi_account_client,
                 "LeaksMemory"),
    TEST_NO_TAG("Group chat Lime Server chat room encrypted message",
                LinphoneTest::group_chat_room_lime_server_encrypted_message),
    TEST_ONE_TAG("Secure one-to-one chat with client removed from database",
                 LinphoneTest::secure_one_to_one_chat_room_with_client_removed_from_database,
                 "LeaksMemory")};

test_suite_t local_conference_test_suite_secure_chat = {"Local conference tester (Secure Chat)",
                                                        NULL,
                                                        NULL,
                                                        liblinphone_tester_before_each,
                                                        liblinphone_tester_after_each,
                                                        sizeof(local_conference_secure_chat_tests) /
                                                            sizeof(local_conference_secure_chat_tests[0]),
                                                        local_conference_secure_chat_tests,
                                                        0};
