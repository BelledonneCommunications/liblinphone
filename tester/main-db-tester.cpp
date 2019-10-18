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

#include "address/address.h"
#include "core/core-p.h"
#include "db/main-db.h"
#include "event-log/events.h"

// TODO: Remove me. <3
#include "private.h"

#include "liblinphone_tester.h"
#include "tools/tester.h"

// =============================================================================

using namespace std;

using namespace LinphonePrivate;

// -----------------------------------------------------------------------------

class MainDbProvider {
public:
	MainDbProvider () : MainDbProvider("db/linphone.db") { }

	MainDbProvider (const char *db_file) {
		mCoreManager = linphone_core_manager_create("marie_rc");
		char *roDbPath = bc_tester_res(db_file);
		char *rwDbPath = bc_tester_file("linphone.db");
		BC_ASSERT_FALSE(liblinphone_tester_copy_file(roDbPath, rwDbPath));
		linphone_config_set_string(linphone_core_get_config(mCoreManager->lc), "storage", "uri", rwDbPath);
		bc_free(roDbPath);
		bc_free(rwDbPath);
		linphone_core_manager_start(mCoreManager, false);
	}

	~MainDbProvider () {
		linphone_core_manager_destroy(mCoreManager);
	}

	const MainDb &getMainDb () {
		return *L_GET_PRIVATE(mCoreManager->lc->cppPtr)->mainDb;
	}

private:
	LinphoneCoreManager *mCoreManager;
};

// -----------------------------------------------------------------------------

static void get_events_count (void) {
	MainDbProvider provider;
	const MainDb &mainDb = provider.getMainDb();
	BC_ASSERT_EQUAL(mainDb.getEventCount(), 5175, int, "%d");
	BC_ASSERT_EQUAL(mainDb.getEventCount(MainDb::ConferenceCallFilter), 0, int, "%d");
	BC_ASSERT_EQUAL(mainDb.getEventCount(MainDb::ConferenceInfoFilter), 18, int, "%d");
	BC_ASSERT_EQUAL(mainDb.getEventCount(MainDb::ConferenceChatMessageFilter), 5157, int, "%d");
	BC_ASSERT_EQUAL(mainDb.getEventCount(MainDb::NoFilter), 5175, int, "%d");
}

static void get_messages_count (void) {
	MainDbProvider provider;
	const MainDb &mainDb = provider.getMainDb();
	BC_ASSERT_EQUAL(mainDb.getChatMessageCount(), 5157, int, "%d");
	BC_ASSERT_EQUAL(
		mainDb.getChatMessageCount(
			ConferenceId(IdentityAddress("sip:test-3@sip.linphone.org"), IdentityAddress("sip:test-1@sip.linphone.org"))
		),
		861, int, "%d"
	);
}

static void get_unread_messages_count (void) {
	MainDbProvider provider;
	const MainDb &mainDb = provider.getMainDb();
	BC_ASSERT_EQUAL(mainDb.getUnreadChatMessageCount(), 2, int, "%d");
	BC_ASSERT_EQUAL(
		mainDb.getUnreadChatMessageCount(
			ConferenceId(IdentityAddress("sip:test-3@sip.linphone.org"), IdentityAddress("sip:test-1@sip.linphone.org"))
		),
		0, int, "%d"
	);
}

static void get_history (void) {
	MainDbProvider provider;
	const MainDb &mainDb = provider.getMainDb();
	BC_ASSERT_EQUAL(
		mainDb.getHistoryRange(
			ConferenceId(IdentityAddress("sip:test-4@sip.linphone.org"), IdentityAddress("sip:test-1@sip.linphone.org")),
			0, -1, MainDb::Filter::ConferenceChatMessageFilter
		).size(),
		54,
		int,
		"%d"
	);
	BC_ASSERT_EQUAL(
		mainDb.getHistoryRange(
			ConferenceId(IdentityAddress("sip:test-7@sip.linphone.org"), IdentityAddress("sip:test-7@sip.linphone.org")),
			0, -1, MainDb::Filter::ConferenceCallFilter
		).size(),
		0,
		int,
		"%d"
	);
	BC_ASSERT_EQUAL(
		mainDb.getHistoryRange(
			ConferenceId(IdentityAddress("sip:test-1@sip.linphone.org"), IdentityAddress("sip:test-1@sip.linphone.org")),
			0, -1, MainDb::Filter::ConferenceChatMessageFilter
		).size(),
		804,
		int,
		"%d"
	);
	BC_ASSERT_EQUAL(
		mainDb.getHistory(
			ConferenceId(IdentityAddress("sip:test-1@sip.linphone.org"), IdentityAddress("sip:test-1@sip.linphone.org")),
			100, MainDb::Filter::ConferenceChatMessageFilter
		).size(),
		100,
		int,
		"%d"
	);
}

static void get_conference_notified_events (void) {
	MainDbProvider provider;
	const MainDb &mainDb = provider.getMainDb();
	list<shared_ptr<EventLog>> events = mainDb.getConferenceNotifiedEvents(
		ConferenceId(IdentityAddress("sip:test-44@sip.linphone.org"), IdentityAddress("sip:test-1@sip.linphone.org")),
		1
	);
	BC_ASSERT_EQUAL(events.size(), 3, int, "%d");
	if (events.size() != 3)
		return;

	shared_ptr<EventLog> event;
	auto it = events.cbegin();

	event = *it;
	if (!BC_ASSERT_TRUE(event->getType() == EventLog::Type::ConferenceParticipantRemoved)) return;
	{
		shared_ptr<ConferenceParticipantEvent> participantEvent = static_pointer_cast<ConferenceParticipantEvent>(event);
		BC_ASSERT_TRUE(participantEvent->getConferenceId().getPeerAddress().asString() == "sip:test-44@sip.linphone.org");
		BC_ASSERT_TRUE(participantEvent->getParticipantAddress().asString() == "sip:test-11@sip.linphone.org");
		BC_ASSERT_TRUE(participantEvent->getNotifyId() == 2);
	}

	event = *++it;
	if (!BC_ASSERT_TRUE(event->getType() == EventLog::Type::ConferenceParticipantDeviceAdded)) return;
	{
		shared_ptr<ConferenceParticipantDeviceEvent> deviceEvent = static_pointer_cast<
			ConferenceParticipantDeviceEvent
		>(event);
		BC_ASSERT_TRUE(deviceEvent->getConferenceId().getPeerAddress().asString() == "sip:test-44@sip.linphone.org");
		BC_ASSERT_TRUE(deviceEvent->getParticipantAddress().asString() == "sip:test-11@sip.linphone.org");
		BC_ASSERT_TRUE(deviceEvent->getNotifyId() == 3);
		BC_ASSERT_TRUE(deviceEvent->getDeviceAddress().asString() == "sip:test-47@sip.linphone.org");
	}

	event = *++it;
	if (!BC_ASSERT_TRUE(event->getType() == EventLog::Type::ConferenceParticipantDeviceRemoved)) return;
	{
		shared_ptr<ConferenceParticipantDeviceEvent> deviceEvent = static_pointer_cast<
			ConferenceParticipantDeviceEvent
		>(event);
		BC_ASSERT_TRUE(deviceEvent->getConferenceId().getPeerAddress().asString() == "sip:test-44@sip.linphone.org");
		BC_ASSERT_TRUE(deviceEvent->getParticipantAddress().asString() == "sip:test-11@sip.linphone.org");
		BC_ASSERT_TRUE(deviceEvent->getNotifyId() == 4);
		BC_ASSERT_TRUE(deviceEvent->getDeviceAddress().asString() == "sip:test-47@sip.linphone.org");
	}
}

static void get_chat_rooms() {
	MainDbProvider provider;
	const MainDb &mainDb = provider.getMainDb();
	list<shared_ptr<AbstractChatRoom>> chatRooms = mainDb.getChatRooms();
	BC_ASSERT_EQUAL(chatRooms.size(), 86, int, "%d");
	
	list<shared_ptr<AbstractChatRoom>> emptyChatRooms;
	shared_ptr<AbstractChatRoom> emptyMessageRoom = nullptr;
	shared_ptr<AbstractChatRoom> oneMessageRoom = nullptr;
	shared_ptr<AbstractChatRoom> multiMessageRoom = nullptr;
	for (const auto chatRoom : chatRooms) {
		if (emptyMessageRoom == nullptr && chatRoom->getMessageHistorySize() == 0) {
			emptyMessageRoom = chatRoom;
		}
		if (oneMessageRoom == nullptr && chatRoom->getMessageHistorySize() == 1) {
			oneMessageRoom = chatRoom;
		}
		if (multiMessageRoom == nullptr && chatRoom->getMessageHistorySize() > 1) {
			multiMessageRoom = chatRoom;
		}

		shared_ptr<ChatMessage> lastMessage = chatRoom->getLastChatMessageInHistory();
		if (chatRoom->isEmpty()) {
			emptyChatRooms.push_back(chatRoom);
			BC_ASSERT_PTR_NULL(lastMessage);
		} else {
			BC_ASSERT_PTR_NOT_NULL(lastMessage);
		}
	}
	BC_ASSERT_EQUAL(emptyChatRooms.size(), 4, int, "%d");
	
	// Check an empty chat room last_message_id is updated after adding a message into it
	BC_ASSERT_PTR_NOT_NULL(emptyMessageRoom);
	if (emptyMessageRoom != nullptr) {
		shared_ptr<ChatMessage> lastMessage = emptyMessageRoom->getLastChatMessageInHistory();
		BC_ASSERT_PTR_NULL(lastMessage);
		shared_ptr<ChatMessage> newMessage = emptyMessageRoom->createChatMessage("Hello world !");
		newMessage->send();
		lastMessage = emptyMessageRoom->getLastChatMessageInHistory();
		BC_ASSERT_PTR_NOT_NULL(lastMessage);
		BC_ASSERT_PTR_EQUAL(lastMessage, newMessage);
	}

	// Check last_message_id is updated to 0 if we remove the last message from a chat room with exactly 1 message
	BC_ASSERT_PTR_NOT_NULL(oneMessageRoom);
	if (oneMessageRoom != nullptr) {
		shared_ptr<ChatMessage> lastMessage = oneMessageRoom->getLastChatMessageInHistory();
		BC_ASSERT_PTR_NOT_NULL(lastMessage);
		oneMessageRoom->deleteHistory();
		lastMessage = oneMessageRoom->getLastChatMessageInHistory();
		BC_ASSERT_PTR_NULL(lastMessage);
	}

	// Check last_message_id is updated to previous message id if we remove the last message from a chat room with more than 1 message
	BC_ASSERT_PTR_NOT_NULL(multiMessageRoom);
	if (multiMessageRoom != nullptr) {
		shared_ptr<ChatMessage> lastMessage = multiMessageRoom->getLastChatMessageInHistory();
		BC_ASSERT_PTR_NOT_NULL(lastMessage);
		multiMessageRoom->deleteMessageFromHistory(lastMessage);
		shared_ptr<ChatMessage> lastMessage2 = multiMessageRoom->getLastChatMessageInHistory();
		BC_ASSERT_PTR_NOT_NULL(lastMessage2);
		BC_ASSERT_PTR_NOT_EQUAL(lastMessage, lastMessage2);

		// Check a non empty chat room last_message_id is updated after adding a message into it
		shared_ptr<ChatMessage> newMessage = multiMessageRoom->createChatMessage("Hello world !");
		newMessage->send();
		lastMessage = multiMessageRoom->getLastChatMessageInHistory();
		BC_ASSERT_PTR_NOT_NULL(lastMessage);
		BC_ASSERT_PTR_EQUAL(lastMessage, newMessage);
		BC_ASSERT_PTR_NOT_EQUAL(lastMessage, lastMessage2);
	}
}

static void load_a_lot_of_chatrooms(void) {
	chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();
	MainDbProvider provider("db/chatrooms.db");
	chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
	long ms = (long) chrono::duration_cast<chrono::milliseconds>(end - start).count();
#if __APPLE__
	BC_ASSERT_LOWER(ms, 2000, long, "%li");
#else
	BC_ASSERT_LOWER(ms, 1000, long, "%li");
#endif
}

test_t main_db_tests[] = {
	TEST_NO_TAG("Get events count", get_events_count),
	TEST_NO_TAG("Get messages count", get_messages_count),
	TEST_NO_TAG("Get unread messages count", get_unread_messages_count),
	TEST_NO_TAG("Get history", get_history),
	TEST_NO_TAG("Get conference events", get_conference_notified_events),
	TEST_NO_TAG("Get chat rooms", get_chat_rooms),
	TEST_NO_TAG("Load a lot of chatrooms", load_a_lot_of_chatrooms)
};

test_suite_t main_db_test_suite = {
	"MainDb", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
	sizeof(main_db_tests) / sizeof(main_db_tests[0]), main_db_tests
};
