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
#include <fstream>
#include <iostream>
#include <list>

#include <linphone++/linphone.hh>

#include "bctoolbox/logging.h"

#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "liblinphone_tester.h"
#include "linphone/api/c-chat-room.h"
#include "linphone/core.h"
#include "linphone/wrapper_utils.h"
#include "payload-type/payload-type.h"
#include "tester_utils.h"

//--------------------------------------------------------------------------------------------
//					Class handlers
//--------------------------------------------------------------------------------------------
class AutoAcceptHandler : public linphone::CoreListener {
public:
	virtual void onCallStateChanged(const std::shared_ptr<linphone::Core> &,
	                                const std::shared_ptr<linphone::Call> &call,
	                                linphone::Call::State state,
	                                const std::string &message) override {
		lDebug() << "[AutoAcceptHandler] Call state changed : (" << (int)state << ") " << message;
		if (state == linphone::Call::State::IncomingReceived) call->accept();
	}
};

//--------------------------------------------------------------------------------------------

static void create_account() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");

	// Get C++ and start working from it.
	auto core = linphone::Object::cPtrToSharedPtr<linphone::Core>(marie->lc, TRUE);

	auto accountCreator = core->createAccountCreator("");
	accountCreator->setUsername("toto");
	accountCreator->setDomain("sip.example.org");
	auto account = accountCreator->createAccountInCore();
	account = nullptr; // Clean account

	core = nullptr; // C++ Core deletion
	wait_for_until(marie->lc, NULL, NULL, 0, 500);

	// C clean
	linphone_core_manager_destroy(marie);
}

static void account_freed_after_core_destroyed() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");

	// Get C++ and start working from it.
	auto core = linphone::Object::cPtrToSharedPtr<linphone::Core>(marie->lc, TRUE);

	auto account = core->getDefaultAccount();
	auto accountParams = account->getParams()->clone();
	accountParams->enableRegister(false);
	account->setParams(accountParams);
	core->clearAccounts();
	core = nullptr; // C++ Core deletion
	// C clean
	linphone_core_manager_destroy(marie);

	// Release last reference after the core is destroyed
	account = nullptr;
}

static void create_chat_room() {
	// Init from C
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	// Get C++ and start working from it.
	auto core = linphone::Object::cPtrToSharedPtr<linphone::Core>(marie->lc, TRUE);

	// Chat room parameters
	auto params = core->createDefaultChatRoomParams();
	std::list<std::shared_ptr<linphone::Address>> participants;
	std::shared_ptr<const linphone::Address> localAddress;
	participants.push_back(linphone::Object::cPtrToSharedPtr<linphone::Address>(pauline->identity));
	params->setBackend(linphone::ChatRoom::Backend::Basic);

	// Creation, store the result inside a variable to test variable scope.
	auto chatRoom = core->createChatRoom(params, localAddress, participants);

	auto cChatRoom = (LinphoneChatRoom *)linphone::Object::sharedPtrToCPtr(chatRoom);
	linphone_chat_room_ref(cChatRoom);
	linphone_chat_room_unref(cChatRoom); // Should not delete chat room. Refs are : Core + chatRoom

	auto chatRooms = core->getChatRooms();
	BC_ASSERT_EQUAL((int)chatRooms.size(), 1, int, "%d");
	auto cr = chatRooms.front(); // Use only one item.
	chatRooms.clear();

	auto cCr = (LinphoneChatRoom *)linphone::Object::sharedPtrToCPtr(cr);
	linphone_chat_room_ref(cCr);
	linphone_chat_room_unref(cCr); // Refs are : Core + chatRoom + cr
	cr = nullptr;                  // Refs are : Core + chatRoom

	wait_for_until(marie->lc, pauline->lc, NULL, 0, 300);
	core->deleteChatRoom(chatRoom);
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 500);

	cChatRoom = (LinphoneChatRoom *)linphone::Object::sharedPtrToCPtr(chatRoom);
	linphone_chat_room_ref(cChatRoom);
	linphone_chat_room_unref(cChatRoom); // Ref is : chatRoom

	chatRoom = nullptr; // Delete chat room.

	core = nullptr; // C++ Core deletion
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 500);

	// C clean
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void create_conference() {
	std::shared_ptr<AutoAcceptHandler> handler = std::make_shared<AutoAcceptHandler>();
	// Init from C
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	// Get C++ and start working from it.
	auto marieCore = linphone::Object::cPtrToSharedPtr<linphone::Core>(marie->lc, TRUE);
	auto paulineCore = linphone::Object::cPtrToSharedPtr<linphone::Core>(pauline->lc, TRUE);

	paulineCore->addListener(handler);

	auto confParams = marieCore->createConferenceParams(nullptr);
	BC_ASSERT_PTR_NOT_NULL(confParams);
	confParams->enableVideo(false);
	confParams->enableLocalParticipant(true);
	confParams->enableChat(false);

	auto conference = marieCore->createConferenceWithParams(confParams);
	BC_ASSERT_PTR_NOT_NULL(conference);

	std::list<std::shared_ptr<linphone::Address>> participants;

	participants.push_back(
	    marieCore->createAddress(linphone::Object::cPtrToSharedPtr<linphone::Address>(pauline->identity)->asString()));

	auto callParams = marieCore->createCallParams(nullptr);
	callParams->enableVideo(false);
	callParams->setAudioDirection(linphone::MediaDirection::SendRecv);

	conference->inviteParticipants(participants, callParams);

	wait_for_until(marie->lc, pauline->lc, NULL, 0, 1000);

	if (marieCore->getCurrentCall()) marieCore->getCurrentCall()->terminate();
	if (paulineCore->getCurrentCall()) paulineCore->getCurrentCall()->terminate();
	conference->terminate();
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 500);

	callParams = nullptr;
	participants.clear();
	conference = nullptr;
	confParams = nullptr;

	marieCore->stop();
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 100);
	marieCore->start();
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 100);

	paulineCore = nullptr; // C++ Core deletion
	marieCore = nullptr;
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 500);

	// C clean
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void various_api_checks(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");

	auto core = linphone::Object::cPtrToSharedPtr<linphone::Core>(marie->lc, TRUE);

	/* check a few accessors */
	auto defaultAccount = core->getDefaultAccount();
	BC_ASSERT_PTR_NOT_NULL(defaultAccount);
	auto contactAddress = defaultAccount->getContactAddress();
	BC_ASSERT_PTR_NOT_NULL(contactAddress);
	auto newAddress = contactAddress->clone();
	BC_ASSERT_PTR_NOT_NULL(newAddress);

	std::list<std::shared_ptr<linphone::Address>> participants;
	auto address = linphone::Factory::get()->createAddress("sip:toto@sip.linphone.org");
	participants.push_back(address);
	auto conferenceInfo = linphone::Factory::get()->createConferenceInfo();
	conferenceInfo->setParticipants(participants);
	auto testList = conferenceInfo->getParticipants();
	BC_ASSERT_EQUAL(participants.size(), testList.size(), size_t, "%zu");

	linphone_core_manager_destroy(marie);
}

static void displaying_payload_type(void) {
	// Init from C
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	// Get C++ and start working from it.
	auto core = linphone::Object::cPtrToSharedPtr<linphone::Core>(marie->lc, TRUE);

	auto payloads = core->getAudioPayloadTypes();
	for (auto it : payloads) {
		lInfo() << "Get mime type : " << it->getMimeType();
		lInfo() << "Encoder description : " << it->getEncoderDescription();
	}

	payloads.clear();

	core = nullptr; // C++ Core deletion

	// C clean
	linphone_core_manager_destroy(marie);
}

test_t wrapper_cpp_tests[] = {TEST_NO_TAG("Create account", create_account),
                              TEST_NO_TAG("Account freed after core destroyed", account_freed_after_core_destroyed),
                              TEST_NO_TAG("Create chat room", create_chat_room),
                              TEST_NO_TAG("Create conference", create_conference),
                              TEST_NO_TAG("Various API checks", various_api_checks),
                              TEST_NO_TAG("Displaying PayloadType", displaying_payload_type)};

test_suite_t wrapper_cpp_test_suite = {"Wrapper Cpp",
                                       NULL,
                                       NULL,
                                       liblinphone_tester_before_each,
                                       liblinphone_tester_after_each,
                                       sizeof(wrapper_cpp_tests) / sizeof(wrapper_cpp_tests[0]),
                                       wrapper_cpp_tests,
                                       0};
