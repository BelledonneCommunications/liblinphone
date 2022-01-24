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
#include "linphone/api/c-types.h"
#include "linphone/api/c-chat-room.h"
#include "linphone/api/c-chat-room-params.h"
#include "tester_utils.h"
#include "shared_tester_functions.h"
#include "linphone/wrapper_utils.h"
#include "liblinphone_tester.h"
#include "bctoolbox/crypto.h"
#include <map>
#include "address/identity-address.h"
#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "chat/chat-room/chat-room.h"
#include "core/core.h"
#include "conference/participant.h"
#include "address/identity-address.h"
#include "chat/chat-room/server-group-chat-room-p.h"

#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic push
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

using namespace LinphonePrivate;
using namespace std;
namespace LinphoneTest {

class BcAssert {
public:
	void addCustomIterate(const std::function<void ()> &iterate) {
		mIterateFuncs.push_back(iterate);
	}
	bool waitUntil( std::chrono::duration<double> timeout ,const std::function<bool ()> &condition) {
		auto start = std::chrono::steady_clock::now();

		bool_t result;
		while (!(result = condition()) && (std::chrono::steady_clock::now() - start < timeout)) {
			for (const std::function<void ()>& iterate:mIterateFuncs) {
				iterate();
			}
			ms_usleep(100);
		}
		return result;
	}
	bool wait(const std::function<bool ()> &condition) {
		return waitUntil(std::chrono::seconds(10),condition);
	}
private:
	std::list<std::function<void ()>> mIterateFuncs;
};

class CoreAssert : public BcAssert {
public:
	CoreAssert(std::initializer_list<std::shared_ptr<Core>> cores) {
		for (shared_ptr<Core> core: cores) {
			addCustomIterate([core] {
				linphone_core_iterate(L_GET_C_BACK_PTR(core));
			});
		}
	}
};

class ClientConference;

class CoreManager {
public:
	CoreManager(std::string rc) {
		mMgr = linphone_core_manager_new(rc.c_str());
		mMgr->user_info = this;
	}

	CoreManager(std::string rc,const std::function<void ()> &preStart):mPreStart(preStart) {
		mMgr = linphone_core_manager_create(rc.c_str());
		mMgr->user_info = this;
		mPreStart();
		linphone_core_manager_start(mMgr,TRUE);
	}

	~CoreManager() {
		IdentityAddress id = getIdentity();
		linphone_core_manager_destroy(mMgr);
		lInfo() << "Core manager for [" << id << "] destroyed";
	}
	LinphoneCore * getLc() const {
		return mMgr->lc;
	}
	Core & getCore() const {
		return *L_GET_CPP_PTR_FROM_C_OBJECT(mMgr->lc);
	}
	IdentityAddress getIdentity() {
		return *L_GET_CPP_PTR_FROM_C_OBJECT(mMgr->identity);
	}
	stats & getStats() {
		return mMgr->stat;
	}
	LinphoneCoreManager * getCMgr() {
		return mMgr;
	}
	void reStart() {
		linphone_core_manager_reinit(mMgr);
		mPreStart();
		linphone_core_manager_start(mMgr, TRUE);
	}
protected:
	LinphoneCoreManager * mMgr;
private:
	CoreManager(const CoreManager&) {};
	const std::function<void ()> mPreStart = []{return;};
};


class CoreManagerAssert : public BcAssert {
public:
	CoreManagerAssert(std::initializer_list<std::reference_wrapper<CoreManager>> coreMgrs): BcAssert(){
		for (CoreManager &coreMgr: coreMgrs) {
			addCustomIterate([&coreMgr] {
				linphone_core_iterate(coreMgr.getLc());
			});
		}
	}
};
class Focus;

/*Core manager acting as a client*/
class ClientConference :public CoreManager {
public:
	ClientConference(std::string rc,Address factoryUri):CoreManager(rc,[this,factoryUri] {
		_configure_core_for_conference(mMgr,L_GET_C_BACK_PTR(&factoryUri));
		setup_mgr_for_conference(getCMgr());
		LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
		linphone_core_cbs_set_chat_room_state_changed(cbs, core_chat_room_state_changed);
		linphone_core_cbs_set_chat_room_subject_changed(cbs, core_chat_room_subject_changed);
		linphone_core_add_callbacks(getLc(),cbs);
		linphone_core_cbs_unref(cbs);

	}),mFocus(nullptr) {

	}

	void deleteChatRoomSync(AbstractChatRoom& chatroom) {
		linphone_core_delete_chat_room(getLc(), L_GET_C_BACK_PTR(&chatroom));
		CoreManagerAssert({*mFocus,*this}).wait([&chatroom] {
			return chatroom.getState() == ChatRoom::State::Deleted;
		});
	}
	~ClientConference() {
		for (auto chatRoom :getCore().getChatRooms()) {
			deleteChatRoomSync(*chatRoom);
		}
		if (mMgr->user_info) {
			ms_free(mMgr->user_info);
		}
	}
	friend Focus;
protected:
	void setFocus(CoreManager* myFocus) {
		mFocus = myFocus;
	}
private:
	CoreManager* mFocus;
};


/* Core manager acting as a focus*/
class Focus : public CoreManager {
public:
	Focus(std::string rc): CoreManager(rc, [this] {
		linphone_core_enable_conference_server(getLc(),TRUE);
	}) {
		configureFocus();
	}
	~Focus(){
		CoreManagerAssert({*this}).waitUntil(chrono::seconds(1),[] {
			return false;
		});
	}

	void registerAsParticipantDevice(ClientConference &otherMgr) {
		const LinphoneAddress *cAddr = linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(otherMgr.getLc()));
		IdentityAddress participantDevice(*L_GET_CPP_PTR_FROM_C_OBJECT(cAddr));
		IdentityAddress participant = participantDevice.getAddressWithoutGruu();
		mParticipantDevices.insert({participant,participantDevice});
		//to allow client conference to delete chatroom in its destructor
		otherMgr.setFocus(this);

	}

	void subscribeParticipantDevice(const LinphoneAddress *conferenceAddress, const LinphoneAddress *participantDevice){
		LinphoneChatRoom *cr = linphone_core_search_chat_room(getLc(), NULL, conferenceAddress, conferenceAddress, NULL);
		BC_ASSERT_PTR_NOT_NULL(cr);
	//	CALL_CHAT_ROOM_CBS(cr, ParticipantRegistrationSubscriptionRequested, participant_registration_subscription_requested, cr, participantDevice)
		_linphone_chat_room_notify_participant_registration_subscription_requested(cr, participantDevice);
	}

	void notifyParticipantDeviceRegistration(const LinphoneAddress *conferenceAddress, const LinphoneAddress *participantDevice){

		LinphoneChatRoom *cr = linphone_core_search_chat_room(getLc(), NULL, conferenceAddress, conferenceAddress, NULL);
		BC_ASSERT_PTR_NOT_NULL(cr);
		linphone_chat_room_notify_participant_device_registration(cr, participantDevice);
	}

	void reStart() {
		CoreManager::reStart();
		configureFocus();
	}
private:
	static void server_core_chat_room_conference_address_generation (LinphoneChatRoom *cr) {
		Focus *focus = (Focus*)(((LinphoneCoreManager *)linphone_core_get_user_data(linphone_chat_room_get_core(cr)))->user_info);
		char config_id[6];
		belle_sip_random_token(config_id,sizeof(config_id));
		const LinphoneAddress *cAddr = linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(focus->getLc()));
		Address conference_address = (*L_GET_CPP_PTR_FROM_C_OBJECT(cAddr));
		conference_address.setUriParam("conf-id",config_id);
		linphone_chat_room_set_conference_address(cr, L_GET_C_BACK_PTR(&conference_address));
	}

	static void server_core_chat_room_state_changed (LinphoneCore *core, LinphoneChatRoom *cr, LinphoneChatRoomState state) {
		Focus *focus = (Focus*)(((LinphoneCoreManager *)linphone_core_get_user_data(core))->user_info);
		switch (state) {
			case LinphoneChatRoomStateInstantiated: {
				LinphoneChatRoomCbs *cbs = linphone_factory_create_chat_room_cbs(linphone_factory_get());
				linphone_chat_room_cbs_set_participant_registration_subscription_requested(cbs, chat_room_participant_registration_subscription_requested);
				linphone_chat_room_cbs_set_conference_address_generation(cbs, server_core_chat_room_conference_address_generation);
				linphone_chat_room_add_callbacks(cr, cbs);
				linphone_chat_room_cbs_set_user_data(cbs, focus);
				linphone_chat_room_cbs_unref(cbs);
				break;
		}
			default:
				break;
		}
	}

	static void chat_room_participant_registration_subscription_requested(LinphoneChatRoom *cr, const LinphoneAddress *participantAddr) {
		BC_ASSERT_PTR_NOT_NULL(participantAddr);
		if (participantAddr) {
			const IdentityAddress participant = *L_GET_CPP_PTR_FROM_C_OBJECT(participantAddr);
			BC_ASSERT_TRUE(participant.isValid());
			if (participant.isValid()) {
				Focus *focus = (Focus*)(linphone_chat_room_cbs_get_user_data(linphone_chat_room_get_current_callbacks(cr)));
				bctbx_list_t * devices = NULL;
				auto participantRange = focus->mParticipantDevices.equal_range(participant);
				for (auto participantIt = participantRange.first; participantIt != participantRange.second; participantIt++ ) {
					LinphoneAddress *deviceAddr = linphone_address_new(participantIt->second.asString().c_str());
					LinphoneParticipantDeviceIdentity * identity = linphone_factory_create_participant_device_identity(linphone_factory_get(),deviceAddr,"");
					linphone_participant_device_identity_set_capability_descriptor(identity, linphone_core_get_linphone_specs(linphone_chat_room_get_core(cr)));
					devices = bctbx_list_append(devices, identity);
					linphone_address_unref(deviceAddr);
				}
				Address participantAddress(participant.asAddress().asString());
				linphone_chat_room_set_participant_devices(cr,L_GET_C_BACK_PTR(&participantAddress),devices);
				bctbx_list_free_with_data(devices,(bctbx_list_free_func)belle_sip_object_unref);
			}
		}
	}

	void configureFocus() {
		LinphoneCoreCbs * cbs = linphone_core_get_first_callbacks(getLc());
		linphone_config_set_int(linphone_core_get_config(getLc()), "misc", "hide_empty_chat_rooms", 0);
		linphone_config_set_int(linphone_core_get_config(getLc()), "sip", "reject_duplicated_calls", 0);
		linphone_config_set_int(linphone_core_get_config(getLc()), "misc", "hide_chat_rooms_from_removed_proxies", 0);
		linphone_core_cbs_set_subscription_state_changed(cbs, linphone_subscription_state_change);
		linphone_core_cbs_set_chat_room_state_changed(cbs, server_core_chat_room_state_changed);
//		linphone_core_cbs_set_refer_received(cbs, linphone_conference_server_refer_received);
		LinphoneProxyConfig *config = linphone_core_get_default_proxy_config(getLc());
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(linphone_core_get_default_proxy_config(getLc()), getIdentity().asString().c_str());
		linphone_proxy_config_done(config);
	}

	std::multimap<IdentityAddress,IdentityAddress,std::less<IdentityAddress>> mParticipantDevices;
};

void sendEphemeralMessageInAdminMode(Focus & focus, ClientConference & sender, ClientConference & recipient, LinphoneChatRoom * senderCr, LinphoneChatRoom * recipientCr, const std::string basicText, const int noMsg) {

	bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
	coresList = bctbx_list_append(coresList, sender.getLc());
	coresList = bctbx_list_append(coresList, recipient.getLc());

	bctbx_list_t *senderHistory = linphone_chat_room_get_history(senderCr, 0);
	auto initialSenderMessages = (int)bctbx_list_size(senderHistory);
	bctbx_list_free_with_data(senderHistory, (bctbx_list_free_func)linphone_chat_message_unref);

	bctbx_list_t *recipientHistory = linphone_chat_room_get_history(recipientCr, 0);
	auto initialRecipientMessages = (int)bctbx_list_size(recipientHistory);
	bctbx_list_free_with_data(recipientHistory, (bctbx_list_free_func)linphone_chat_message_unref);

	int initialUnreadMessages = linphone_chat_room_get_unread_messages_count(recipientCr);

	auto sender_stat=sender.getStats();
	auto recipient_stat=recipient.getStats();

	std::list<LinphoneChatMessage *>messages;
	// Marie sends messages
	for (int i=0; i<noMsg; i++) {
		const std::string text = basicText + std::to_string(i);
		messages.push_back(_send_message_ephemeral(senderCr, text.c_str(), TRUE));
	}

	senderHistory = linphone_chat_room_get_history(senderCr, 0);
	BC_ASSERT_EQUAL((int)bctbx_list_size(senderHistory), (noMsg + initialSenderMessages), int, "%i");
	set_ephemeral_cbs(senderHistory);

	BC_ASSERT_TRUE(wait_for_list(coresList, &recipient.getStats().number_of_LinphoneMessageReceived, recipient_stat.number_of_LinphoneMessageReceived + noMsg,11000));

	// Check that the message has been delivered to Pauline
	for (const auto & msg : messages) {
		BC_ASSERT_TRUE(CoreManagerAssert({focus,sender,recipient}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));
	}

	BC_ASSERT_TRUE(CoreManagerAssert({focus,sender,recipient}).wait([&, recipientCr] {
		return linphone_chat_room_get_unread_messages_count(recipientCr) == (noMsg + initialUnreadMessages);
	}));

	recipientHistory = linphone_chat_room_get_history(recipientCr, 0);
	BC_ASSERT_EQUAL((int)bctbx_list_size(recipientHistory), (noMsg + initialRecipientMessages), int, "%i");
	set_ephemeral_cbs(recipientHistory);

	BC_ASSERT_TRUE(wait_for_list(coresList, &sender.getStats().number_of_LinphoneMessageDeliveredToUser, sender_stat.number_of_LinphoneMessageDeliveredToUser + noMsg, 10000));

	// Pauline marks the message as read, check that the state is now displayed on Marie's side
	linphone_chat_room_mark_as_read(recipientCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &sender.getStats().number_of_LinphoneMessageDisplayed, sender_stat.number_of_LinphoneMessageDisplayed + noMsg, 10000));

	BC_ASSERT_TRUE(wait_for_list(coresList, &sender.getStats().number_of_LinphoneChatRoomEphemeralTimerStarted, sender_stat.number_of_LinphoneChatRoomEphemeralTimerStarted + noMsg, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &recipient.getStats().number_of_LinphoneChatRoomEphemeralTimerStarted, recipient_stat.number_of_LinphoneChatRoomEphemeralTimerStarted + noMsg, 10000));

	BC_ASSERT_TRUE(wait_for_list(coresList, &sender.getStats().number_of_LinphoneMessageEphemeralTimerStarted, sender_stat.number_of_LinphoneMessageEphemeralTimerStarted + noMsg, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &recipient.getStats().number_of_LinphoneMessageEphemeralTimerStarted, recipient_stat.number_of_LinphoneMessageEphemeralTimerStarted + noMsg, 10000));

	BC_ASSERT_TRUE(wait_for_list(coresList, &sender.getStats().number_of_LinphoneChatRoomEphemeralDeleted, sender_stat.number_of_LinphoneChatRoomEphemeralDeleted + noMsg, 15000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &recipient.getStats().number_of_LinphoneChatRoomEphemeralDeleted, recipient_stat.number_of_LinphoneChatRoomEphemeralDeleted + noMsg, 15000));

	BC_ASSERT_TRUE(wait_for_list(coresList, &sender.getStats().number_of_LinphoneMessageEphemeralDeleted, sender_stat.number_of_LinphoneMessageEphemeralDeleted + noMsg, 15000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &recipient.getStats().number_of_LinphoneMessageEphemeralDeleted, recipient_stat.number_of_LinphoneMessageEphemeralDeleted + noMsg, 15000));

	bctbx_list_free_with_data(recipientHistory, (bctbx_list_free_func)linphone_chat_message_unref);
	bctbx_list_free_with_data(senderHistory, (bctbx_list_free_func)linphone_chat_message_unref);

	//wait bit more to detect side effect if any
	CoreManagerAssert({focus,sender,recipient}).waitUntil(chrono::seconds(2),[] {
		return false;
	});

	recipientHistory = linphone_chat_room_get_history(recipientCr, 0);
	BC_ASSERT_EQUAL((int)bctbx_list_size(recipientHistory), initialRecipientMessages, int, "%i");
	senderHistory = linphone_chat_room_get_history(senderCr, 0);
	BC_ASSERT_EQUAL((int)bctbx_list_size(senderHistory), initialSenderMessages, int, "%i");

	for (auto & msg : messages) {
		linphone_chat_message_unref(msg);
	}

	bctbx_list_free_with_data(recipientHistory, (bctbx_list_free_func)linphone_chat_message_unref);
	bctbx_list_free_with_data(senderHistory, (bctbx_list_free_func)linphone_chat_message_unref);
	bctbx_list_free(coresList);
}

static void group_chat_room_creation_server (void) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focus.getIdentity().asAddress());
		ClientConference laure("laure_tcp_rc", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);

		bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, laure.getLc());

		Address paulineAddr(pauline.getIdentity().asAddress());
		Address laureAddr(laure.getIdentity().asAddress());
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(L_GET_C_BACK_PTR(&paulineAddr)));
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(L_GET_C_BACK_PTR(&laureAddr)));

		stats initialMarieStats = marie.getStats();
		stats initialPaulineStats = pauline.getStats();
		stats initialLaureStats = laure.getStats();

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues";
		LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie.getCMgr(), &initialMarieStats, participantsAddresses, initialSubject, FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
		const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &initialPaulineStats, confAddr, initialSubject, 2, FALSE);
		LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure.getCMgr(), &initialLaureStats, confAddr, initialSubject, 2, FALSE);

		// Marie now changes the subject
		const char *newSubject = "Let's go drink a beer";
		linphone_chat_room_set_subject(marieCr, newSubject);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_subject_changed, initialMarieStats.number_of_subject_changed + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_subject_changed, initialLaureStats.number_of_subject_changed + 1, 5000));
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr), newSubject);

		// Bring Laure offline and remove her from the chat room
		// As Laure is offline, she is not notified of the removal
		linphone_core_set_network_reachable(laure.getLc(), FALSE);
		BC_ASSERT_FALSE(linphone_core_is_network_reachable(laure.getLc()));
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(std::chrono::seconds(1),[ ]{return false;});
		LinphoneParticipant *laureParticipant = linphone_chat_room_find_participant(marieCr, L_GET_C_BACK_PTR(&laureAddr));
		BC_ASSERT_PTR_NOT_NULL(laureParticipant);
		linphone_chat_room_remove_participant(marieCr, laureParticipant);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed, initialMarieStats.number_of_participants_removed + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, 5000));

		// Check that Laure's conference is not destroyed
		BC_ASSERT_EQUAL(laure.getStats().number_of_LinphoneConferenceStateTerminated, initialLaureStats.number_of_LinphoneConferenceStateTerminated, int, "%d");

		coresList = bctbx_list_remove(coresList, focus.getLc());
		//Restart flexisip
		focus.reStart();
		coresList = bctbx_list_append(coresList, focus.getLc());

		for (auto chatRoom :focus.getCore().getChatRooms()) {

			BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(L_GET_C_BACK_PTR(chatRoom)), 3, int, "%d");
		}

		BC_ASSERT_FALSE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateTerminated, initialLaureStats.number_of_LinphoneConferenceStateTerminated + 1, 3000));

		// Laure comes back online and its chatroom is expected to be deleted
		linphone_core_set_network_reachable(laure.getLc(), TRUE);
		LinphoneAddress *laureDeviceAddress =  linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(laure.getLc())));
		// Notify chat room that a participant has registered
		focus.notifyParticipantDeviceRegistration(linphone_chat_room_get_conference_address(marieCr), laureDeviceAddress);
		linphone_address_unref(laureDeviceAddress);
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateTerminated, initialLaureStats.number_of_LinphoneConferenceStateTerminated + 1, 5000));

		for (auto chatRoom :focus.getCore().getChatRooms()) {

			BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(L_GET_C_BACK_PTR(chatRoom)), 3, int, "%d");
		}

		linphone_chat_room_leave(paulineCr);
	}
}


static void group_chat_room_server_deletion (void) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);

		bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		Address paulineAddr(pauline.getIdentity().asAddress());
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(L_GET_C_BACK_PTR(&paulineAddr)));

		stats initialMarieStats = marie.getStats();
		stats initialPaulineStats = pauline.getStats();

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues";
		LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie.getCMgr(), &initialMarieStats, participantsAddresses, initialSubject, FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
		const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &initialPaulineStats, confAddr, initialSubject, 1, FALSE);

		/*BC_ASSERT_TRUE(*/CoreManagerAssert({focus,marie,pauline}).wait([&focus] {
			for (auto chatRoom :focus.getCore().getChatRooms()) {
				for (auto participant: chatRoom->getParticipants()) {
					for (auto device: participant->getDevices())
						if (device->getState() != ParticipantDevice::State::Present) {
							return false;
						}
				}
			}
			return true;
		})/*)*/;


		LinphoneChatMessage *msg = linphone_chat_room_create_message_from_utf8(marieCr, "message blabla");
		linphone_chat_message_send(msg);
		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));
		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([paulineCr] {
			return linphone_chat_room_get_unread_messages_count(paulineCr) == 1;
		}));


		for (auto chatRoom :focus.getCore().getChatRooms()) {
			for (auto participant: chatRoom->getParticipants()) {
				//  force deletion by removing devices
				bctbx_list_t *empty = bctbx_list_new(NULL);
				Address participantAddress(participant->getAddress().asAddress());
				linphone_chat_room_set_participant_devices(  L_GET_C_BACK_PTR(chatRoom)
														   , L_GET_C_BACK_PTR(&participantAddress)
														   , NULL);
				bctbx_list_free(empty);
			}
		}

		//wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		// Clean db from chat room
		linphone_chat_message_unref(msg);

		bctbx_list_free(coresList);
	}

}

static void group_chat_room_server_deletion_with_rmt_lst_event_handler (void) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);

		bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		Address paulineAddr(pauline.getIdentity().asAddress());
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(L_GET_C_BACK_PTR(&paulineAddr)));

		stats initialMarieStats = marie.getStats();
		stats initialPaulineStats = pauline.getStats();

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues";
		LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie.getCMgr(), &initialMarieStats, participantsAddresses, initialSubject, FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
		const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &initialPaulineStats, confAddr, initialSubject, 1, FALSE);

		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([&focus] {
			for (auto chatRoom :focus.getCore().getChatRooms()) {
				for (auto participant: chatRoom->getParticipants()) {
					for (auto device: participant->getDevices())
						if (device->getState() != ParticipantDevice::State::Present) {
							return false;
						}
				}
			}
			return true;
		}));


		LinphoneChatMessage *msg = linphone_chat_room_create_message_from_utf8(marieCr, "message blabla");
		linphone_chat_message_send(msg);
		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));
		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([paulineCr] {
			return linphone_chat_room_get_unread_messages_count(paulineCr) == 1;
		}));


		//now with simulate foregound/backgroud switch to get a remote event handler list instead of a simple remote event handler
		linphone_core_enter_background(pauline.getLc());
		linphone_config_set_bool(linphone_core_get_config(pauline.getLc()), "misc", "conference_event_package_force_full_state",TRUE);
		CoreManagerAssert({focus, marie, pauline}).waitUntil(std::chrono::seconds(1),[ ]{return false;});

		unsigned int paulineCrNo = (unsigned int)bctbx_list_size(linphone_core_get_chat_rooms (pauline.getLc()));
		coresList = bctbx_list_remove(coresList, pauline.getLc());
		pauline.reStart();
		coresList = bctbx_list_append(coresList, pauline.getLc());

		// Wait for chat rooms to be recovered from the main DB
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateCreated, paulineCrNo, 5000));
		char *paulineDeviceIdentity = linphone_core_get_device_identity(pauline.getLc());
		LinphoneAddress *paulineLocalAddr = linphone_address_new(paulineDeviceIdentity);
		bctbx_free(paulineDeviceIdentity);
		paulineCr = linphone_core_search_chat_room(pauline.getLc(), NULL, paulineLocalAddr, confAddr, NULL);
		linphone_address_unref(paulineLocalAddr);
		BC_ASSERT_PTR_NOT_NULL(paulineCr);

		CoreManagerAssert({focus, marie, pauline}).waitUntil(std::chrono::seconds(1),[ ]{return false;});

		CoreManagerAssert({focus,marie}).waitUntil(std::chrono::seconds(2),[ ]{return false;});

		for (auto chatRoom :focus.getCore().getChatRooms()) {
			for (auto participant: chatRoom->getParticipants()) {
				//  force deletion by removing devices
				bctbx_list_t *empty = bctbx_list_new(NULL);
				Address participantAddress(participant->getAddress().asAddress());
				linphone_chat_room_set_participant_devices(  L_GET_C_BACK_PTR(chatRoom)
														   , L_GET_C_BACK_PTR(&participantAddress)
														   , NULL);
				bctbx_list_free(empty);
			}
		}

		//wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		//to avoid creation attempt of a new chatroom
		LinphoneProxyConfig *config = linphone_core_get_default_proxy_config(focus.getLc());
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		linphone_chat_message_unref(msg);

		bctbx_list_free(coresList);
	}
}

static void group_chat_room_server_admin_managed_messages_base (bool_t encrypted) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);

		linphone_core_set_default_ephemeral_lifetime(marie.getLc(), 25);

		// Enable IMDN
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline.getLc()));

		bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		Address paulineAddr(pauline.getIdentity().asAddress());
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(L_GET_C_BACK_PTR(&paulineAddr)));

		stats chloe_stat=focus.getStats();
		stats marie_stat=marie.getStats();
		stats pauline_stat=pauline.getStats();

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues";
		const LinphoneChatRoomEphemeralMode adminMode = LinphoneChatRoomEphemeralModeAdminManaged;
		LinphoneChatRoomParams *params = linphone_core_create_default_chat_room_params(marie.getLc());

		linphone_chat_room_params_enable_group(params, FALSE);
		linphone_chat_room_params_enable_encryption(params, FALSE);
		linphone_chat_room_params_set_ephemeral_mode(params, adminMode);
		linphone_chat_room_params_set_ephemeral_lifetime(params, 5);
		linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendFlexisipChat);

		LinphoneChatRoom *marieCr = create_chat_room_client_side_with_params(coresList, marie.getCMgr(), &marie_stat, participantsAddresses, initialSubject, params);
		linphone_chat_room_params_unref(params);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &pauline_stat, confAddr, initialSubject, 1, FALSE);

		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 5, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(paulineCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(paulineCr), 5, int, "%d");

		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([&focus] {
			for (auto chatRoom :focus.getCore().getChatRooms()) {
				for (auto participant: chatRoom->getParticipants()) {
					for (auto device: participant->getDevices())
						if (device->getState() != ParticipantDevice::State::Present) {
							return false;
						}
				}
			}
			return true;
		}));

		chloe_stat=focus.getStats();
		marie_stat=marie.getStats();
		pauline_stat=pauline.getStats();

		constexpr int noMsg = 10;
		sendEphemeralMessageInAdminMode(focus, marie, pauline, marieCr, paulineCr, "Hello ", noMsg);

		for (auto chatRoom :focus.getCore().getChatRooms()) {
			for (auto participant: chatRoom->getParticipants()) {
				//  force deletion by removing devices
				bctbx_list_t *empty = bctbx_list_new(NULL);
				Address participantAddress(participant->getAddress().asAddress());
				linphone_chat_room_set_participant_devices(  L_GET_C_BACK_PTR(chatRoom)
														   , L_GET_C_BACK_PTR(&participantAddress)
														   , NULL);
				bctbx_list_free(empty);
			}
		}

		//wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		//to avoid creation attempt of a new chatroom
		LinphoneProxyConfig *config = linphone_core_get_default_proxy_config(focus.getLc());
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		bctbx_list_free(coresList);
	}
}

static void group_chat_room_server_admin_managed_messages_unencrypted (void) {
	group_chat_room_server_admin_managed_messages_base (FALSE);
}

static void group_chat_room_server_admin_managed_messages_ephemeral_enabled_after_creation (void) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);

		// Enable IMDN
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline.getLc()));

		bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		Address paulineAddr(pauline.getIdentity().asAddress());
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(L_GET_C_BACK_PTR(&paulineAddr)));

		stats chloe_stat=focus.getStats();
		stats marie_stat=marie.getStats();
		stats pauline_stat=pauline.getStats();

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues";
		const LinphoneChatRoomEphemeralMode adminMode = LinphoneChatRoomEphemeralModeAdminManaged;
		LinphoneChatRoomParams *params = linphone_core_create_default_chat_room_params(marie.getLc());

		linphone_chat_room_params_enable_group(params, FALSE);
		linphone_chat_room_params_enable_encryption(params, FALSE);
		linphone_chat_room_params_set_ephemeral_mode(params, adminMode);
		linphone_chat_room_params_set_ephemeral_lifetime(params, 0);
		linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendFlexisipChat);

		LinphoneChatRoom *marieCr = create_chat_room_client_side_with_params(coresList, marie.getCMgr(), &marie_stat, participantsAddresses, initialSubject, params);
		linphone_chat_room_params_unref(params);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &pauline_stat, confAddr, initialSubject, 1, FALSE);

		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(paulineCr));

		pauline_stat=pauline.getStats();
		linphone_chat_room_set_ephemeral_lifetime(marieCr, 10);

		BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getCMgr()->stat.number_of_NotifyReceived,pauline_stat.number_of_NotifyReceived + 1,5000));

		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 10, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(paulineCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(paulineCr), 10, int, "%d");

		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([&focus] {
			for (auto chatRoom :focus.getCore().getChatRooms()) {
				for (auto participant: chatRoom->getParticipants()) {
					for (auto device: participant->getDevices())
						if (device->getState() != ParticipantDevice::State::Present) {
							return false;
						}
				}
			}
			return true;
		}));

		chloe_stat=focus.getStats();
		marie_stat=marie.getStats();
		pauline_stat=pauline.getStats();

		constexpr int noMsg = 10;
		sendEphemeralMessageInAdminMode(focus, marie, pauline, marieCr, paulineCr, "Hello ", noMsg);

		coresList = bctbx_list_remove(coresList, marie.getLc());
		marie.reStart();
		setup_mgr_for_conference(marie.getCMgr());
		coresList = bctbx_list_append(coresList, marie.getLc());
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));

		// Retrieve chat room
		LinphoneAddress *marieDeviceAddr =  linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(marie.getLc())));
		marieCr = linphone_core_search_chat_room(marie.getLc(), NULL, marieDeviceAddr, confAddr, NULL);
		BC_ASSERT_PTR_NOT_NULL(marieCr);

		// Wait for chat rooms to be recovered from the main DB
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 1, 5000));

		if (marieCr) {
			BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 10, int, "%d");

			marie_stat=marie.getStats();
			pauline_stat=pauline.getStats();

			linphone_chat_room_set_ephemeral_lifetime(marieCr, 5);

			BC_ASSERT_TRUE(wait_for_list(coresList,&marie.getStats().number_of_LinphoneSubscriptionActive,marie_stat.number_of_LinphoneSubscriptionActive + 1,5000));

			BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 5, int, "%d");

			sendEphemeralMessageInAdminMode(focus, marie, pauline, marieCr, paulineCr, "Back online ", noMsg);
		}

		for (auto chatRoom :focus.getCore().getChatRooms()) {
			for (auto participant: chatRoom->getParticipants()) {
				//  force deletion by removing devices
				bctbx_list_t *empty = bctbx_list_new(NULL);
				Address participantAddress(participant->getAddress().asAddress());
				linphone_chat_room_set_participant_devices(  L_GET_C_BACK_PTR(chatRoom)
														   , L_GET_C_BACK_PTR(&participantAddress)
														   , NULL);
				bctbx_list_free(empty);
			}
		}

		//wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		//to avoid creation attempt of a new chatroom
		LinphoneProxyConfig *config = linphone_core_get_default_proxy_config(focus.getLc());
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void group_chat_room_server_admin_managed_messages_ephemeral_disabled_after_creation (void) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);

		// Enable IMDN
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline.getLc()));

		linphone_core_set_default_ephemeral_lifetime(marie.getLc(), 1);

		bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		Address paulineAddr(pauline.getIdentity().asAddress());
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(L_GET_C_BACK_PTR(&paulineAddr)));

		stats chloe_stat=focus.getStats();
		stats marie_stat=marie.getStats();
		stats pauline_stat=pauline.getStats();

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues";
		const LinphoneChatRoomEphemeralMode adminMode = LinphoneChatRoomEphemeralModeAdminManaged;
		LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie.getCMgr(), &marie_stat, participantsAddresses, initialSubject, FALSE, adminMode);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &pauline_stat, confAddr, initialSubject, 1, FALSE);

		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 1, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(paulineCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(paulineCr), 1, int, "%d");

		pauline_stat=pauline.getStats();
		linphone_chat_room_set_ephemeral_lifetime(marieCr, 0);

		BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getCMgr()->stat.number_of_NotifyReceived,pauline_stat.number_of_NotifyReceived + 1,5000));

		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(paulineCr));

		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([&focus] {
			for (auto chatRoom :focus.getCore().getChatRooms()) {
				for (auto participant: chatRoom->getParticipants()) {
					for (auto device: participant->getDevices())
						if (device->getState() != ParticipantDevice::State::Present) {
							return false;
						}
				}
			}
			return true;
		}));

		chloe_stat=focus.getStats();
		marie_stat=marie.getStats();
		pauline_stat=pauline.getStats();

		constexpr int noMsg = 10;
		LinphoneChatMessage *message[noMsg];
		// Marie sends messages
		for (int i=0; i<noMsg; i++) {
			const std::string text = std::string("Hello ") + std::to_string(i);
			message[i] = _send_message_ephemeral(marieCr, text.c_str(), TRUE);
		}

		bctbx_list_t *marieHistory = linphone_chat_room_get_history(marieCr, 0);
		BC_ASSERT_EQUAL((int)bctbx_list_size(marieHistory), noMsg, int, "%i");

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageReceived, pauline_stat.number_of_LinphoneMessageReceived + noMsg,11000));

		// Check that the message has been delivered to Pauline
		for (int i=0; i<noMsg; i++) {
			const auto msg = message[i];
			BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([msg] {
				return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
			}));
		}

		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([&, paulineCr] {
			return linphone_chat_room_get_unread_messages_count(paulineCr) == noMsg;
		}));

		bctbx_list_t *paulineHistory = linphone_chat_room_get_history(paulineCr, 0);
		BC_ASSERT_EQUAL((int)bctbx_list_size(paulineHistory), noMsg, int, "%i");

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDeliveredToUser, marie_stat.number_of_LinphoneMessageDeliveredToUser + noMsg, 10000));

		// Pauline marks the message as read, check that the state is now displayed on Marie's side
		linphone_chat_room_mark_as_read(paulineCr);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDisplayed, marie_stat.number_of_LinphoneMessageDisplayed + noMsg, 10000));

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		bctbx_list_free_with_data(paulineHistory, (bctbx_list_free_func)linphone_chat_message_unref);
		bctbx_list_free_with_data(marieHistory, (bctbx_list_free_func)linphone_chat_message_unref);
		paulineHistory = linphone_chat_room_get_history(paulineCr, 0);
		BC_ASSERT_EQUAL((int)bctbx_list_size(paulineHistory), noMsg, int, "%i");
		marieHistory = linphone_chat_room_get_history(marieCr, 0);
		BC_ASSERT_EQUAL((int)bctbx_list_size(marieHistory), noMsg, int, "%i");

		bctbx_list_free_with_data(paulineHistory, (bctbx_list_free_func)linphone_chat_message_unref);
		bctbx_list_free_with_data(marieHistory, (bctbx_list_free_func)linphone_chat_message_unref);

		for (int i=0; i<noMsg; i++) {
			linphone_chat_message_unref(message[i]);
		}

		coresList = bctbx_list_remove(coresList, marie.getLc());
		marie.reStart();
		setup_mgr_for_conference(marie.getCMgr());
		coresList = bctbx_list_append(coresList, marie.getLc());
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));

		// Retrieve chat room
		LinphoneAddress *marieDeviceAddr =  linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(marie.getLc())));
		marieCr = linphone_core_search_chat_room(marie.getLc(), NULL, marieDeviceAddr, confAddr, NULL);
		BC_ASSERT_PTR_NOT_NULL(marieCr);

		// Wait for chat rooms to be recovered from the main DB
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 1, 5000));

		if (marieCr) {
			BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(marieCr));

			marie_stat=marie.getStats();
			pauline_stat=pauline.getStats();

			linphone_chat_room_set_ephemeral_lifetime(marieCr, 5);

			BC_ASSERT_TRUE(wait_for_list(coresList,&marie.getStats().number_of_LinphoneSubscriptionActive,marie_stat.number_of_LinphoneSubscriptionActive + 1,5000));
			BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getStats().number_of_LinphoneChatRoomEphemeralMessageEnabled,pauline_stat.number_of_LinphoneChatRoomEphemeralMessageEnabled + 1,5000));
			BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getStats().number_of_LinphoneChatRoomEphemeralLifetimeChanged,pauline_stat.number_of_LinphoneChatRoomEphemeralLifetimeChanged + 1,5000));

			BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 5, int, "%d");
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
			BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(paulineCr));
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(paulineCr), 5, int, "%d");

			sendEphemeralMessageInAdminMode(focus, marie, pauline, marieCr, paulineCr, "Back online ", noMsg);
		}

		for (auto chatRoom :focus.getCore().getChatRooms()) {
			for (auto participant: chatRoom->getParticipants()) {
				//  force deletion by removing devices
				bctbx_list_t *empty = bctbx_list_new(NULL);
				Address participantAddress(participant->getAddress().asAddress());
				linphone_chat_room_set_participant_devices(  L_GET_C_BACK_PTR(chatRoom)
														   , L_GET_C_BACK_PTR(&participantAddress)
														   , NULL);
				bctbx_list_free(empty);
			}
		}

		//wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		//to avoid creation attempt of a new chatroom
		LinphoneProxyConfig *config = linphone_core_get_default_proxy_config(focus.getLc());
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void group_chat_room_server_admin_managed_messages_ephemeral_lifetime_update (void) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focus.getIdentity().asAddress());

		linphone_core_set_default_ephemeral_lifetime(marie.getLc(), 5);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);

		// Enable IMDN
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline.getLc()));

		bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		Address paulineAddr(pauline.getIdentity().asAddress());
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(L_GET_C_BACK_PTR(&paulineAddr)));

		stats marie_stat=marie.getStats();
		stats pauline_stat=pauline.getStats();

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues";
		const LinphoneChatRoomEphemeralMode adminMode = LinphoneChatRoomEphemeralModeAdminManaged;
		LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie.getCMgr(), &marie_stat, participantsAddresses, initialSubject, FALSE, adminMode);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &pauline_stat, confAddr, initialSubject, 1, FALSE);

		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([&focus] {
			for (auto chatRoom :focus.getCore().getChatRooms()) {
				for (auto participant: chatRoom->getParticipants()) {
					for (auto device: participant->getDevices())
						if (device->getState() != ParticipantDevice::State::Present) {
							return false;
						}
				}
			}
			return true;
		}));

		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 5, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(paulineCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(paulineCr), 5, int, "%d");

		coresList = bctbx_list_remove(coresList, marie.getLc());
		marie.reStart();
		setup_mgr_for_conference(marie.getCMgr());
		coresList = bctbx_list_append(coresList, marie.getLc());
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));

		// Retrieve chat room
		LinphoneAddress *marieDeviceAddr =  linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(marie.getLc())));
		marieCr = linphone_core_search_chat_room(marie.getLc(), NULL, marieDeviceAddr, confAddr, NULL);
		BC_ASSERT_PTR_NOT_NULL(marieCr);

		// Wait for chat rooms to be recovered from the main DB
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 1, 5000));

		if (marieCr) {
			BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 5, int, "%d");

			marie_stat=marie.getStats();
			pauline_stat=pauline.getStats();
			linphone_chat_room_set_ephemeral_lifetime(marieCr, 10);

			BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getStats().number_of_LinphoneChatRoomEphemeralLifetimeChanged,pauline_stat.number_of_LinphoneChatRoomEphemeralLifetimeChanged + 1,5000));

			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
			BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 10, int, "%d");
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
			BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(paulineCr));
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(paulineCr), 10, int, "%d");

			constexpr int noMsg = 10;
			sendEphemeralMessageInAdminMode(focus, marie, pauline, marieCr, paulineCr, "Hello ", noMsg);
		}

		for (auto chatRoom :focus.getCore().getChatRooms()) {
			for (auto participant: chatRoom->getParticipants()) {
				//  force deletion by removing devices
				bctbx_list_t *empty = bctbx_list_new(NULL);
				Address participantAddress(participant->getAddress().asAddress());
				linphone_chat_room_set_participant_devices(  L_GET_C_BACK_PTR(chatRoom)
														   , L_GET_C_BACK_PTR(&participantAddress)
														   , NULL);
				bctbx_list_free(empty);
			}
		}

		//wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		//to avoid creation attempt of a new chatroom
		LinphoneProxyConfig *config = linphone_core_get_default_proxy_config(focus.getLc());
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void group_chat_room_server_admin_managed_messages_ephemeral_lifetime_toggle_using_different_methods (void) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);

		// Enable IMDN
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline.getLc()));

		linphone_core_set_default_ephemeral_lifetime(marie.getLc(), 5);

		bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		Address paulineAddr(pauline.getIdentity().asAddress());
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(L_GET_C_BACK_PTR(&paulineAddr)));

		stats chloe_stat=focus.getStats();
		stats marie_stat=marie.getStats();
		stats pauline_stat=pauline.getStats();

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues";
		const LinphoneChatRoomEphemeralMode adminMode = LinphoneChatRoomEphemeralModeAdminManaged;
		LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie.getCMgr(), &marie_stat, participantsAddresses, initialSubject, FALSE, adminMode);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &pauline_stat, confAddr, initialSubject, 1, FALSE);

		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([&focus] {
			for (auto chatRoom :focus.getCore().getChatRooms()) {
				for (auto participant: chatRoom->getParticipants()) {
					for (auto device: participant->getDevices())
						if (device->getState() != ParticipantDevice::State::Present) {
							return false;
						}
				}
			}
			return true;
		}));

		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 5, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(paulineCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(paulineCr), 5, int, "%d");

		pauline_stat=pauline.getStats();
		linphone_chat_room_set_ephemeral_lifetime(marieCr, 10);

		BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getStats().number_of_LinphoneChatRoomEphemeralLifetimeChanged,pauline_stat.number_of_LinphoneChatRoomEphemeralLifetimeChanged + 1,5000));

		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 10, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(paulineCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(paulineCr), 10, int, "%d");

		chloe_stat=focus.getStats();
		marie_stat=marie.getStats();
		pauline_stat=pauline.getStats();

		constexpr int noMsg = 10;
		sendEphemeralMessageInAdminMode(focus, marie, pauline, marieCr, paulineCr, "Hello ", noMsg);

		pauline_stat=pauline.getStats();
		// Disable ephemeral
		linphone_chat_room_set_ephemeral_lifetime(marieCr, 0);

		BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getStats().number_of_LinphoneChatRoomEphemeralMessageDisabled,pauline_stat.number_of_LinphoneChatRoomEphemeralMessageDisabled + 1,5000));

		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 0, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(paulineCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(paulineCr), 0, int, "%d");

		chloe_stat=focus.getStats();
		marie_stat=marie.getStats();
		pauline_stat=pauline.getStats();

		LinphoneChatMessage *non_ephemeral_message;
		// Marie sends messages
		const std::string non_ephemeral_text = std::string("Not an ephemeral message");
		non_ephemeral_message = _send_message_ephemeral(marieCr, non_ephemeral_text.c_str(), TRUE);

		auto marieHistory = linphone_chat_room_get_history(marieCr, 0);
		BC_ASSERT_EQUAL((int)bctbx_list_size(marieHistory), 1, int, "%i");

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageReceived, pauline_stat.number_of_LinphoneMessageReceived + 1,11000));

		// Check that the message has been delivered to Pauline
		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([non_ephemeral_message] {
			return (linphone_chat_message_get_state(non_ephemeral_message) == LinphoneChatMessageStateDelivered);
		}));

		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([&, paulineCr] {
			return linphone_chat_room_get_unread_messages_count(paulineCr) == 1;
		}));

		auto paulineHistory = linphone_chat_room_get_history(paulineCr, 0);
		BC_ASSERT_EQUAL((int)bctbx_list_size(paulineHistory), 1, int, "%i");

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDeliveredToUser, marie_stat.number_of_LinphoneMessageDeliveredToUser + 1, 10000));

		// Pauline marks the message as read, check that the state is now displayed on Marie's side
		linphone_chat_room_mark_as_read(paulineCr);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDisplayed, marie_stat.number_of_LinphoneMessageDisplayed + 1, 10000));


		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		bctbx_list_free_with_data(paulineHistory, (bctbx_list_free_func)linphone_chat_message_unref);
		bctbx_list_free_with_data(marieHistory, (bctbx_list_free_func)linphone_chat_message_unref);

		pauline_stat=pauline.getStats();
		linphone_chat_room_enable_ephemeral(marieCr, TRUE);

		BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getStats().number_of_LinphoneChatRoomEphemeralMessageEnabled,pauline_stat.number_of_LinphoneChatRoomEphemeralMessageEnabled + 1,5000));

		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), linphone_core_get_default_ephemeral_lifetime(marie.getLc()), int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(paulineCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(paulineCr), linphone_core_get_default_ephemeral_lifetime(marie.getLc()), int, "%d");

		chloe_stat=focus.getStats();
		marie_stat=marie.getStats();
		pauline_stat=pauline.getStats();

		constexpr int noShortMsg = 10;
		sendEphemeralMessageInAdminMode(focus, marie, pauline, marieCr, paulineCr, "Test ephemeral message #", noShortMsg);

		linphone_chat_message_unref(non_ephemeral_message);

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		for (auto chatRoom :focus.getCore().getChatRooms()) {
			for (auto participant: chatRoom->getParticipants()) {
				//  force deletion by removing devices
				bctbx_list_t *empty = bctbx_list_new(NULL);
				Address participantAddress(participant->getAddress().asAddress());
				linphone_chat_room_set_participant_devices(  L_GET_C_BACK_PTR(chatRoom)
														   , L_GET_C_BACK_PTR(&participantAddress)
														   , NULL);
				bctbx_list_free(empty);
			}
		}

		//wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		//to avoid creation attempt of a new chatroom
		LinphoneProxyConfig *config = linphone_core_get_default_proxy_config(focus.getLc());
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		bctbx_list_free(coresList);
	}
}

static void group_chat_room_bulk_notify_to_participant (void) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focus.getIdentity().asAddress());
		ClientConference michelle("michelle_rc_udp", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(michelle);

		bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, michelle.getLc());
		Address paulineAddr(pauline.getIdentity().asAddress());
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(L_GET_C_BACK_PTR(&paulineAddr)));
		Address michelleAddr(michelle.getIdentity().asAddress());
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(L_GET_C_BACK_PTR(&michelleAddr)));

		stats initialMarieStats = marie.getStats();
		stats initialPaulineStats = pauline.getStats();
		stats initialMichelleStats = michelle.getStats();

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues";
		LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie.getCMgr(), &initialMarieStats, participantsAddresses, initialSubject, FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
		const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &initialPaulineStats, confAddr, initialSubject, 2, FALSE);
		LinphoneChatRoom *michelleCr = check_creation_chat_room_client_side(coresList, michelle.getCMgr(), &initialMichelleStats, confAddr, initialSubject, 2, FALSE);

		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline,michelle}).wait([&focus] {
			for (auto chatRoom :focus.getCore().getChatRooms()) {
				for (auto participant: chatRoom->getParticipants()) {
					for (auto device: participant->getDevices())
						if (device->getState() != ParticipantDevice::State::Present) {
							return false;
						}
				}
			}
			return true;
		}));

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateCreated, initialPaulineStats.number_of_LinphoneConferenceStateCreated + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateCreated, initialMichelleStats.number_of_LinphoneConferenceStateCreated + 1, 5000));

		// Marie now changes the subject
		const char *newSubject = "Let's go drink a beer";
		linphone_chat_room_set_subject(marieCr, newSubject);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_subject_changed, initialMarieStats.number_of_subject_changed + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_subject_changed, initialMichelleStats.number_of_subject_changed + 1, 5000));
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(michelleCr), newSubject);

		// Pauline goes offline
		linphone_core_set_network_reachable(pauline.getLc(), FALSE);

		// Adding Laure
		ClientConference laure("laure_tcp_rc", focus.getIdentity().asAddress());
		coresList = bctbx_list_append(coresList, laure.getLc());
		focus.registerAsParticipantDevice(laure);

		initialMarieStats = marie.getStats();
		initialPaulineStats = pauline.getStats();
		initialMichelleStats = michelle.getStats();
		stats initialLaureStats = laure.getStats();

		Address laureAddr(laure.getIdentity().asAddress());
		participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(L_GET_C_BACK_PTR(&laureAddr)));
		linphone_chat_room_add_participants(marieCr, participantsAddresses);

		LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure.getCMgr(), &initialLaureStats, confAddr, newSubject, 3, FALSE);
		BC_ASSERT_PTR_NOT_NULL(laureCr);

		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_LinphoneConferenceStateCreationPending, initialLaureStats.number_of_LinphoneConferenceStateCreationPending + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_LinphoneConferenceStateCreated, initialLaureStats.number_of_LinphoneConferenceStateCreated + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_LinphoneChatRoomConferenceJoined, initialLaureStats.number_of_LinphoneChatRoomConferenceJoined + 1, 5000));


		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_participant_devices_added, initialMarieStats.number_of_participant_devices_added + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participant_devices_added, initialPaulineStats.number_of_participant_devices_added + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getCMgr()->stat.number_of_participant_devices_added, initialMichelleStats.number_of_participant_devices_added + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_participants_added, initialMarieStats.number_of_participants_added + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participants_added, initialPaulineStats.number_of_participants_added + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getCMgr()->stat.number_of_participants_added, initialMichelleStats.number_of_participants_added + 1, 5000));

		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 3, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 2, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(michelleCr), 3, int, "%d");

		// Wait a little bit to detect side effects
		CoreManagerAssert({focus,marie,laure,pauline,michelle}).waitUntil(std::chrono::seconds(2),[ ]{return false;});

		initialMarieStats = marie.getStats();
		initialPaulineStats = pauline.getStats();
		initialMichelleStats = michelle.getStats();
		initialLaureStats = laure.getStats();

		// Marie now changes the subject again
		const char *newSubject2 = "Seriously, ladies... Tonight we go out";
		linphone_chat_room_set_subject(marieCr, newSubject2);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_subject_changed, initialMarieStats.number_of_subject_changed + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline.getStats().number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_subject_changed, initialLaureStats.number_of_subject_changed + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_subject_changed, initialMichelleStats.number_of_subject_changed + 1, 5000));
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject2);
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(michelleCr), newSubject2);
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr), newSubject2);

		initialMarieStats = marie.getStats();
		initialPaulineStats = pauline.getStats();
		initialMichelleStats = michelle.getStats();
		initialLaureStats = laure.getStats();

		char *laureDeviceIdentity = linphone_core_get_device_identity(laure.getLc());
		LinphoneAddress *laureLocalAddr = linphone_address_new(laureDeviceIdentity);
		BC_ASSERT_PTR_NOT_NULL(laureLocalAddr);
		bctbx_free(laureDeviceIdentity);

		// Marisip deletes Laure's device
		for (auto chatRoom :focus.getCore().getChatRooms()) {
			std::shared_ptr<Participant> participant = chatRoom->findParticipant(*L_GET_CPP_PTR_FROM_C_OBJECT(laureLocalAddr));
			BC_ASSERT_PTR_NOT_NULL(participant);
			if (participant) {
				//  force deletion by removing devices
				bctbx_list_t *empty = bctbx_list_new(NULL);
				Address participantAddress(participant->getAddress().asAddress());
				// Do not use laureLocalAddr because it has a GRUU
				linphone_chat_room_set_participant_devices( L_GET_C_BACK_PTR(chatRoom), L_GET_C_BACK_PTR(&participantAddress), NULL);
				bctbx_list_free(empty);
			}
		}

		// Marie removes Laure from the chat room
		LinphoneParticipant *laureParticipant = linphone_chat_room_find_participant(marieCr, laureLocalAddr);
		BC_ASSERT_PTR_NOT_NULL(laureParticipant);
		linphone_chat_room_remove_participant(marieCr, laureParticipant);

		linphone_address_unref(laureLocalAddr);

		BC_ASSERT_FALSE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_LinphoneConferenceStateTerminated, initialLaureStats.number_of_LinphoneConferenceStateTerminated + 1, 3000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_participant_devices_removed, initialMarieStats.number_of_participant_devices_removed + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participant_devices_removed, initialPaulineStats.number_of_participant_devices_removed + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getCMgr()->stat.number_of_participant_devices_removed, initialMichelleStats.number_of_participant_devices_removed + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_participants_removed, initialMarieStats.number_of_participants_removed + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getCMgr()->stat.number_of_participants_removed, initialMichelleStats.number_of_participants_removed + 1, 5000));

		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 2, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 2, int, "%d");

		// Wait a little bit to detect side effects
		CoreManagerAssert({focus,marie,pauline,michelle}).waitUntil(std::chrono::seconds(2),[ ]{return false;});

		initialPaulineStats = pauline.getStats();
		// Pauline comes up online
		linphone_core_set_network_reachable(pauline.getLc(), TRUE);

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_LinphoneRegistrationOk, initialPaulineStats.number_of_LinphoneRegistrationOk + 1, 10000));

		// Check that Pauline receives the backlog of events occurred while she was offline
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participant_devices_added, initialPaulineStats.number_of_participant_devices_added + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participants_added, initialPaulineStats.number_of_participants_added + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participant_devices_removed, initialPaulineStats.number_of_participant_devices_removed + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, 5000));

		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 2, int, "%d");
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject2);

		CoreManagerAssert({focus, marie, pauline,michelle}).waitUntil(std::chrono::seconds(1),[ ]{return false;});

		CoreManagerAssert({focus,marie}).waitUntil(std::chrono::seconds(2),[ ]{return false;});

		for (auto chatRoom :focus.getCore().getChatRooms()) {
			for (auto participant: chatRoom->getParticipants()) {
				//  force deletion by removing devices
				bctbx_list_t *empty = bctbx_list_new(NULL);
				Address participantAddress(participant->getAddress().asAddress());
				linphone_chat_room_set_participant_devices(  L_GET_C_BACK_PTR(chatRoom)
														   , L_GET_C_BACK_PTR(&participantAddress)
														   , NULL);
				bctbx_list_free(empty);
			}
		}

		//wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		//to avoid creation attempt of a new chatroom
		LinphoneProxyConfig *config = linphone_core_get_default_proxy_config(focus.getLc());
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		bctbx_list_free(coresList);
	}
}

static void group_chat_room_add_participant_with_invalid_address (void) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focus.getIdentity().asAddress());
		ClientConference michelle("michelle_rc_udp", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(michelle);

		bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, michelle.getLc());
		Address paulineAddr(pauline.getIdentity().asAddress());
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(L_GET_C_BACK_PTR(&paulineAddr)));
		Address invalidAddr;
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(L_GET_C_BACK_PTR(&invalidAddr)));
		Address michelleAddr(michelle.getIdentity().asAddress());
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(L_GET_C_BACK_PTR(&michelleAddr)));

		stats initialMarieStats = marie.getStats();
		stats initialPaulineStats = pauline.getStats();
		stats initialMichelleStats = michelle.getStats();

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues";
		LinphoneChatRoom *marieCr = create_chat_room_client_side_with_expected_number_of_participants(coresList, marie.getCMgr(), &initialMarieStats, participantsAddresses, initialSubject, 2, FALSE,LinphoneChatRoomEphemeralModeDeviceManaged);
		const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &initialPaulineStats, confAddr, initialSubject, 2, FALSE);
		LinphoneChatRoom *michelleCr = check_creation_chat_room_client_side(coresList, michelle.getCMgr(), &initialMichelleStats, confAddr, initialSubject, 2, FALSE);

		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline,michelle}).wait([&focus] {
			for (auto chatRoom :focus.getCore().getChatRooms()) {
				for (auto participant: chatRoom->getParticipants()) {
					for (auto device: participant->getDevices())
						if (device->getState() != ParticipantDevice::State::Present) {
							return false;
						}
				}
			}
			return true;
		}));

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateCreated, initialPaulineStats.number_of_LinphoneConferenceStateCreated + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateCreated, initialMichelleStats.number_of_LinphoneConferenceStateCreated + 1, 5000));

		// Marie now changes the subject
		const char *newSubject = "Let's go drink a beer";
		linphone_chat_room_set_subject(marieCr, newSubject);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_subject_changed, initialMarieStats.number_of_subject_changed + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_subject_changed, initialMichelleStats.number_of_subject_changed + 1, 5000));
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);
		BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(michelleCr), newSubject);

		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 2, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 2, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(michelleCr), 2, int, "%d");

		initialMarieStats = marie.getStats();
		initialPaulineStats = pauline.getStats();
		initialMichelleStats = michelle.getStats();

		linphone_chat_room_add_participant(marieCr, linphone_address_ref(L_GET_C_BACK_PTR(&invalidAddr)));

		BC_ASSERT_FALSE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_participant_devices_added, initialMarieStats.number_of_participant_devices_added + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_participants_added, initialMarieStats.number_of_participants_added + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participant_devices_added, initialPaulineStats.number_of_participant_devices_added + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participants_added, initialPaulineStats.number_of_participants_added + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &michelle.getCMgr()->stat.number_of_participant_devices_added, initialMichelleStats.number_of_participant_devices_added + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &michelle.getCMgr()->stat.number_of_participants_added, initialMichelleStats.number_of_participants_added + 1, 5000));

		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 2, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 2, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(michelleCr), 2, int, "%d");

		CoreManagerAssert({focus, marie, pauline,michelle}).waitUntil(std::chrono::seconds(1),[ ]{return false;});

		CoreManagerAssert({focus,marie}).waitUntil(std::chrono::seconds(2),[ ]{return false;});

		for (auto chatRoom :focus.getCore().getChatRooms()) {
			for (auto participant: chatRoom->getParticipants()) {
				//  force deletion by removing devices
				bctbx_list_t *empty = bctbx_list_new(NULL);
				Address participantAddress(participant->getAddress().asAddress());
				linphone_chat_room_set_participant_devices(  L_GET_C_BACK_PTR(chatRoom)
														   , L_GET_C_BACK_PTR(&participantAddress)
														   , NULL);
				bctbx_list_free(empty);
			}
		}

		//wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		//to avoid creation attempt of a new chatroom
		LinphoneProxyConfig *config = linphone_core_get_default_proxy_config(focus.getLc());
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		linphone_address_unref(L_GET_C_BACK_PTR(&invalidAddr));
		bctbx_list_free(coresList);
	}
}

static void group_chat_room_with_only_participant_with_invalid_address (void) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);

		bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		Address invalidAddr;
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(L_GET_C_BACK_PTR(&invalidAddr)));

		stats initialMarieStats = marie.getStats();

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues";

		LinphoneChatRoomParams *chatRoomParams = linphone_core_create_default_chat_room_params(marie.getLc());
		linphone_chat_room_params_enable_encryption(chatRoomParams, FALSE);
		linphone_chat_room_params_set_backend(chatRoomParams, LinphoneChatRoomBackendFlexisipChat);
		linphone_chat_room_params_enable_group(chatRoomParams, TRUE);
		LinphoneChatRoom *marieCr = create_chat_room_client_side_with_expected_number_of_participants(coresList, marie.getCMgr(), &initialMarieStats, participantsAddresses, initialSubject, 0, FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
		linphone_chat_room_params_unref(chatRoomParams);
		BC_ASSERT_PTR_NOT_NULL(marieCr);

		// Check that the chat room has not been created
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_LinphoneConferenceStateCreated, initialMarieStats.number_of_LinphoneConferenceStateCreated + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_LinphoneChatRoomConferenceJoined, initialMarieStats.number_of_LinphoneChatRoomConferenceJoined + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_LinphoneConferenceStateCreationFailed, initialMarieStats.number_of_LinphoneConferenceStateCreationFailed + 1, 5000));

		//to avoid creation attempt of a new chatroom
		LinphoneProxyConfig *config = linphone_core_get_default_proxy_config(focus.getLc());
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		bctbx_list_free(coresList);
	}
}

static void one_to_one_chatroom_exhumed_while_offline (void) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);

		bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		Address paulineAddr(pauline.getIdentity().asAddress());
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(L_GET_C_BACK_PTR(&paulineAddr)));

		stats initialMarieStats = marie.getStats();
		stats initialPaulineStats = pauline.getStats();

		// Marie creates a new one to one chat room
		const char *initialSubject = "one to one with Pauline";
		LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie.getCMgr(), &initialMarieStats, participantsAddresses, initialSubject, FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &initialPaulineStats, confAddr, initialSubject, 1, FALSE);

		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([&focus] {
			for (auto chatRoom :focus.getCore().getChatRooms()) {
				for (auto participant: chatRoom->getParticipants()) {
					for (auto device: participant->getDevices())
						if (device->getState() != ParticipantDevice::State::Present) {
							return false;
						}
				}
			}
			return true;
		}));

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateCreated, initialPaulineStats.number_of_LinphoneConferenceStateCreated + 1, 5000));

		// Pauline goes offline
		linphone_core_set_network_reachable(pauline.getLc(), FALSE);

		LinphoneChatMessage *marieMsg1 = linphone_chat_room_create_message_from_utf8(marieCr, "Long live the C++ !");
		linphone_chat_message_send(marieMsg1);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageSent, initialMarieStats.number_of_LinphoneMessageSent + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 5000));
		linphone_chat_message_unref(marieMsg1);

		int marieMsgs = linphone_chat_room_get_history_size(marieCr);
		BC_ASSERT_EQUAL(marieMsgs, 1, int , "%d");
		// Pauline didn't received the message as she was offline
		int paulineMsgs = linphone_chat_room_get_history_size(paulineCr);
		BC_ASSERT_EQUAL(paulineMsgs, 0, int , "%d");

		// Wait a little bit to detect side effects
		CoreManagerAssert({focus,marie,pauline}).waitUntil(std::chrono::seconds(2),[ ]{return false;});

		// Marie deletes the chat room
		// Pauline cannot now this because she is offline
		linphone_core_manager_delete_chat_room(marie.getCMgr(), marieCr, coresList);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminated, initialMarieStats.number_of_LinphoneConferenceStateTerminated + 1, 5000));

		initialMarieStats = marie.getStats();
		initialPaulineStats = pauline.getStats();

		paulineAddr = pauline.getIdentity().asAddress();
		participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(L_GET_C_BACK_PTR(&paulineAddr)));

		marieCr = create_chat_room_client_side(coresList, marie.getCMgr(), &initialMarieStats, participantsAddresses, initialSubject, FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		LinphoneAddress *exhumedConfAddrPtr = (LinphoneAddress *)linphone_chat_room_get_conference_address(marieCr);
		BC_ASSERT_PTR_NOT_NULL(exhumedConfAddrPtr);
		LinphoneAddress *exhumedConfAddr = NULL;
		if (exhumedConfAddrPtr) {
			exhumedConfAddr = linphone_address_clone((LinphoneAddress *)linphone_chat_room_get_conference_address(marieCr));
			BC_ASSERT_PTR_NOT_NULL(exhumedConfAddr);
			if (exhumedConfAddr) {
				BC_ASSERT_FALSE(linphone_address_equal(confAddr, exhumedConfAddr));
			}
		}


		BC_ASSERT_EQUAL((int)marie.getCore().getChatRooms().size(), 1, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 1, int, "%d");

		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 1, int, "%d");

		// Wait a little bit to detect side effects
		CoreManagerAssert({focus,marie,pauline}).waitUntil(std::chrono::seconds(2),[ ]{return false;});

		initialPaulineStats = pauline.getStats();
		// Pauline comes up online
		linphone_core_set_network_reachable(pauline.getLc(), TRUE);

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_LinphoneRegistrationOk, initialPaulineStats.number_of_LinphoneRegistrationOk + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneChatRoomConferenceJoined, initialPaulineStats.number_of_LinphoneChatRoomConferenceJoined + 1, 5000));
		BC_ASSERT_EQUAL((int)pauline.getCore().getChatRooms().size(), 1, int, "%d");

		char *paulineDeviceIdentity = linphone_core_get_device_identity(pauline.getLc());
		LinphoneAddress *paulineDeviceAddr = linphone_address_new(paulineDeviceIdentity);
		bctbx_free(paulineDeviceIdentity);
		LinphoneChatRoom *newPaulineCr = linphone_core_search_chat_room(pauline.getLc(), NULL, paulineDeviceAddr, confAddr, NULL);
		linphone_address_unref(paulineDeviceAddr);
		BC_ASSERT_PTR_NOT_NULL(newPaulineCr);
		BC_ASSERT_PTR_EQUAL(newPaulineCr, paulineCr);

		if (newPaulineCr) {
			LinphoneAddress *paulineNewConfAddr = linphone_address_ref((LinphoneAddress *)linphone_chat_room_get_conference_address(newPaulineCr));
			BC_ASSERT_PTR_NOT_NULL(paulineNewConfAddr);
			if (paulineNewConfAddr) {
				BC_ASSERT_FALSE(linphone_address_equal(confAddr, paulineNewConfAddr));
				if (exhumedConfAddr) {
					BC_ASSERT_TRUE(linphone_address_equal(exhumedConfAddr, paulineNewConfAddr));
				}
			}
			linphone_address_unref(paulineNewConfAddr);

			BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(newPaulineCr), 1, int, "%d");
			BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(newPaulineCr), initialSubject);

			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 5000));
			paulineMsgs = linphone_chat_room_get_history_size(newPaulineCr);
			BC_ASSERT_EQUAL(paulineMsgs, 1, int , "%d");

			LinphoneChatMessage *paulineMsg = linphone_chat_room_create_message_from_utf8(newPaulineCr, "Sorry I was offline :(");
			linphone_chat_message_send(paulineMsg);
			BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([paulineMsg] {
				return (linphone_chat_message_get_state(paulineMsg) == LinphoneChatMessageStateDelivered);
			}));
			BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([marieCr] {
				return linphone_chat_room_get_unread_messages_count(marieCr) == 1;
			}));

			// Since Marie has deleted the chat room, she lost all messages she sent before deleting it
			marieMsgs = linphone_chat_room_get_history_size(marieCr);
			BC_ASSERT_EQUAL(marieMsgs, 1, int , "%d");
			paulineMsgs = linphone_chat_room_get_history_size(newPaulineCr);
			BC_ASSERT_EQUAL(paulineMsgs, 2, int , "%d");

			LinphoneChatMessage *marieMsg = linphone_chat_room_create_message_from_utf8(marieCr, "exhumed!!");
			linphone_chat_message_send(marieMsg);
			BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([marieMsg] {
				return (linphone_chat_message_get_state(marieMsg) == LinphoneChatMessageStateDelivered);
			}));
			BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([newPaulineCr] {
				return linphone_chat_room_get_unread_messages_count(newPaulineCr) == 2;
			}));
			linphone_chat_message_unref(marieMsg);

			marieMsgs = linphone_chat_room_get_history_size(marieCr);
			BC_ASSERT_EQUAL(marieMsgs, 2, int , "%d");
			paulineMsgs = linphone_chat_room_get_history_size(newPaulineCr);
			BC_ASSERT_EQUAL(paulineMsgs, 3, int , "%d");
		}

		linphone_address_unref(exhumedConfAddr);

		CoreManagerAssert({focus, marie, pauline}).waitUntil(std::chrono::seconds(1),[ ]{return false;});

		CoreManagerAssert({focus,marie}).waitUntil(std::chrono::seconds(2),[ ]{return false;});

		initialMarieStats = marie.getStats();
		initialPaulineStats = pauline.getStats();

		linphone_core_manager_delete_chat_room(marie.getCMgr(), marieCr, coresList);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminated, initialMarieStats.number_of_LinphoneConferenceStateTerminated + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateTerminated, initialPaulineStats.number_of_LinphoneConferenceStateTerminated + 1, 5000));

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		//to avoid creation attempt of a new chatroom
		LinphoneProxyConfig *config = linphone_core_get_default_proxy_config(focus.getLc());
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		bctbx_list_free(coresList);
	}
}

static void multidomain_group_chat_room (void) {
	Focus focusExampleDotOrg("chloe_rc");
	Focus focusAuth1DotExampleDotOrg("arthur_rc");
	{ //to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focusExampleDotOrg.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focusExampleDotOrg.getIdentity().asAddress());
		ClientConference michelle("michelle_rc_udp", focusExampleDotOrg.getIdentity().asAddress());

		focusExampleDotOrg.registerAsParticipantDevice(marie);
		focusExampleDotOrg.registerAsParticipantDevice(pauline);
		focusExampleDotOrg.registerAsParticipantDevice(michelle);

		bctbx_list_t * coresList = bctbx_list_append(NULL, focusExampleDotOrg.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, michelle.getLc());
		Address paulineAddr(pauline.getIdentity().asAddress());
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(L_GET_C_BACK_PTR(&paulineAddr)));
		Address michelleAddr(michelle.getIdentity().asAddress());
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(L_GET_C_BACK_PTR(&michelleAddr)));

		stats initialMarieStats = marie.getStats();
		stats initialPaulineStats = pauline.getStats();
		stats initialMichelleStats = michelle.getStats();

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues";
		LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie.getCMgr(), &initialMarieStats, participantsAddresses, initialSubject, FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
		LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

		LinphoneChatRoom *michelleCr = check_creation_chat_room_client_side(coresList, michelle.getCMgr(), &initialMichelleStats, confAddr, initialSubject, 2, FALSE);

		if (paulineCr || michelleCr) {
			//throw BCTBX_EXCEPTION << "Cannot create chatroom giving up";
			//goto end;
		}
		BC_ASSERT_TRUE(CoreManagerAssert({focusExampleDotOrg,marie,pauline,michelle}).wait([&focusExampleDotOrg] {
			for (auto chatRoom :focusExampleDotOrg.getCore().getChatRooms()) {
				for (auto participant: chatRoom->getParticipants()) {
					for (auto device: participant->getDevices())
						if (device->getState() != ParticipantDevice::State::Present) {
							return false;
						}
				}
			}
			return true;
		}));


		LinphoneChatMessage *msg = linphone_chat_room_create_message_from_utf8(marieCr, "message blabla");
		linphone_chat_message_send(msg);
		BC_ASSERT_TRUE(CoreManagerAssert({focusExampleDotOrg,marie,pauline,michelle}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));
		BC_ASSERT_TRUE(CoreManagerAssert({focusExampleDotOrg,marie,pauline,michelle}).wait([paulineCr] {
			return linphone_chat_room_get_unread_messages_count(paulineCr) == 1;
		}));
		BC_ASSERT_TRUE(CoreManagerAssert({focusExampleDotOrg,marie,pauline,michelle}).wait([michelleCr] {
			return linphone_chat_room_get_unread_messages_count(michelleCr) == 1;
		}));
		linphone_chat_message_unref(msg);


		// now change focus in order to get conference with multiple domain.


		focusAuth1DotExampleDotOrg.registerAsParticipantDevice(marie);
		focusAuth1DotExampleDotOrg.registerAsParticipantDevice(pauline);
		focusAuth1DotExampleDotOrg.registerAsParticipantDevice(michelle);

		//change conference factory uri
		Address focusAuth1DotExampleDotOrgFactoryAddress = focusAuth1DotExampleDotOrg.getIdentity().asAddress();
		_configure_core_for_conference(marie.getCMgr(),L_GET_C_BACK_PTR(&focusAuth1DotExampleDotOrgFactoryAddress));
		_configure_core_for_conference(pauline.getCMgr(),L_GET_C_BACK_PTR(&focusAuth1DotExampleDotOrgFactoryAddress));
		_configure_core_for_conference(michelle.getCMgr(),L_GET_C_BACK_PTR(&focusAuth1DotExampleDotOrgFactoryAddress));

		coresList = bctbx_list_append(coresList, focusAuth1DotExampleDotOrg.getLc());
		initialMarieStats = marie.getStats();
		initialPaulineStats = pauline.getStats();
		initialMichelleStats = michelle.getStats();
		participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(L_GET_C_BACK_PTR(&paulineAddr)));
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(L_GET_C_BACK_PTR(&michelleAddr)));

;		LinphoneChatRoom *marieCrfocusAuth1DotExampleDotOrg = create_chat_room_client_side(coresList, marie.getCMgr(), &initialMarieStats, participantsAddresses, initialSubject, FALSE, LinphoneChatRoomEphemeralModeDeviceManaged);
		LinphoneAddress *confAddrfocusAuth1DotExampleDotOrg = linphone_address_clone(linphone_chat_room_get_conference_address(marieCrfocusAuth1DotExampleDotOrg));

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCrfocusAuth1DotExampleDotOrg = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &initialPaulineStats, confAddrfocusAuth1DotExampleDotOrg, initialSubject, 2, FALSE);
		BC_ASSERT_PTR_NOT_NULL(paulineCrfocusAuth1DotExampleDotOrg);
		LinphoneChatRoom *michelleCrfocusAuth1DotExampleDotOrg = check_creation_chat_room_client_side(coresList, michelle.getCMgr(), &initialMichelleStats, confAddrfocusAuth1DotExampleDotOrg, initialSubject, 2, FALSE);
		BC_ASSERT_PTR_NOT_NULL(michelleCrfocusAuth1DotExampleDotOrg);

		BC_ASSERT_TRUE(CoreManagerAssert({focusAuth1DotExampleDotOrg,marie,pauline,michelle}).wait([&focusAuth1DotExampleDotOrg] {
			for (auto chatRoom :focusAuth1DotExampleDotOrg.getCore().getChatRooms()) {
				for (auto participant: chatRoom->getParticipants()) {
					for (auto device: participant->getDevices())
						if (device->getState() != ParticipantDevice::State::Present) {
							return false;
						}
				}
			}
			return true;
		}));

		msg = linphone_chat_room_create_message_from_utf8(marieCrfocusAuth1DotExampleDotOrg, "message blabla");
		linphone_chat_message_send(msg);
		BC_ASSERT_TRUE(CoreManagerAssert({focusAuth1DotExampleDotOrg,marie,pauline,michelle}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));

		//great, now I want to see what happened if marie restart.
		coresList = bctbx_list_remove(coresList, marie.getLc());
		marie.reStart();
		setup_mgr_for_conference(marie.getCMgr());
		coresList = bctbx_list_append(coresList, marie.getLc());

		// Retrieve chat room
		LinphoneAddress *marieDeviceAddr =  linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(marie.getLc())));
		marieCr = linphone_core_search_chat_room(marie.getLc(), NULL, marieDeviceAddr, confAddr, NULL);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		marieCrfocusAuth1DotExampleDotOrg = linphone_core_search_chat_room(marie.getLc(), NULL, marieDeviceAddr, confAddrfocusAuth1DotExampleDotOrg, NULL);
		BC_ASSERT_PTR_NOT_NULL(marieCrfocusAuth1DotExampleDotOrg);

		CoreManagerAssert({focusExampleDotOrg,marie,pauline,michelle}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		BC_ASSERT_TRUE(wait_for_list(coresList,&marie.getCMgr()->stat.number_of_LinphoneSubscriptionActive,2,10000));

		ClientConference laure("laure_tcp_rc", focusExampleDotOrg.getIdentity().asAddress());
		coresList = bctbx_list_append(coresList, laure.getLc());
		Address laureAddr(laure.getIdentity().asAddress());
		focusExampleDotOrg.registerAsParticipantDevice(laure);

		initialMarieStats = marie.getStats();
		initialPaulineStats = pauline.getStats();
		initialMichelleStats = michelle.getStats();
		stats initialLaureStats = laure.getStats();

		participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(L_GET_C_BACK_PTR(&laureAddr)));
		linphone_chat_room_add_participants(marieCr, participantsAddresses);

		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_LinphoneConferenceStateCreationPending, initialLaureStats.number_of_LinphoneConferenceStateCreationPending + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_LinphoneConferenceStateCreated, initialLaureStats.number_of_LinphoneConferenceStateCreated + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_LinphoneChatRoomConferenceJoined, initialLaureStats.number_of_LinphoneChatRoomConferenceJoined + 1, 5000));

		LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure.getCMgr(), &initialLaureStats, confAddr, initialSubject, 3, FALSE);
		BC_ASSERT_PTR_NOT_NULL(laureCr);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_participants_added, initialMarieStats.number_of_participants_added + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participants_added, initialPaulineStats.number_of_participants_added + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getCMgr()->stat.number_of_participants_added, initialMichelleStats.number_of_participants_added + 1, 5000));

		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 3, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 3, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(michelleCr), 3, int, "%d");

		BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getCMgr()->stat.number_of_NotifyReceived,initialPaulineStats.number_of_NotifyReceived + 1,5000));
		BC_ASSERT_TRUE(wait_for_list(coresList,&michelle.getCMgr()->stat.number_of_NotifyReceived,initialMichelleStats.number_of_NotifyReceived + 1,5000));

		focusAuth1DotExampleDotOrg.registerAsParticipantDevice(laure);
		_configure_core_for_conference(laure.getCMgr(),L_GET_C_BACK_PTR(&focusAuth1DotExampleDotOrgFactoryAddress));

		participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(L_GET_C_BACK_PTR(&laureAddr)));
		linphone_chat_room_add_participants(marieCrfocusAuth1DotExampleDotOrg, participantsAddresses);
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_LinphoneConferenceStateCreationPending, initialLaureStats.number_of_LinphoneConferenceStateCreationPending + 2, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_LinphoneConferenceStateCreated, initialLaureStats.number_of_LinphoneConferenceStateCreated + 2, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_LinphoneChatRoomConferenceJoined, initialLaureStats.number_of_LinphoneChatRoomConferenceJoined + 2, 5000));

		LinphoneChatRoom *laureCrfocusAuth1DotExampleDotOrg = check_creation_chat_room_client_side(coresList, laure.getCMgr(), &initialLaureStats, confAddrfocusAuth1DotExampleDotOrg, initialSubject, 3, FALSE);
		BC_ASSERT_PTR_NOT_NULL(laureCrfocusAuth1DotExampleDotOrg);

		BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getCMgr()->stat.number_of_NotifyReceived,initialPaulineStats.number_of_NotifyReceived + 2,5000));
		BC_ASSERT_TRUE(wait_for_list(coresList,&michelle.getCMgr()->stat.number_of_NotifyReceived,initialMichelleStats.number_of_NotifyReceived + 2,5000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_participants_added, initialMarieStats.number_of_participants_added + 2, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participants_added, initialPaulineStats.number_of_participants_added + 2, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getCMgr()->stat.number_of_participants_added, initialMichelleStats.number_of_participants_added + 2, 5000));

		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCrfocusAuth1DotExampleDotOrg), 3, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCrfocusAuth1DotExampleDotOrg), 3, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(michelleCrfocusAuth1DotExampleDotOrg), 3, int, "%d");

		linphone_chat_message_unref(msg);

		bctbx_list_free(coresList);
	}

}

static void group_chat_room_server_ephemeral_mode_changed (void) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);

		// Enable IMDN
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline.getLc()));

		bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		Address paulineAddr(pauline.getIdentity().asAddress());
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(L_GET_C_BACK_PTR(&paulineAddr)));

		stats chloe_stat=focus.getStats();
		stats marie_stat=marie.getStats();
		stats pauline_stat=pauline.getStats();

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues";
		const LinphoneChatRoomEphemeralMode adminMode = LinphoneChatRoomEphemeralModeAdminManaged;
		LinphoneChatRoomParams *params = linphone_core_create_default_chat_room_params(marie.getLc());

		linphone_chat_room_params_enable_group(params, FALSE);
		linphone_chat_room_params_enable_encryption(params, FALSE);
		linphone_chat_room_params_set_ephemeral_mode(params, adminMode);
		linphone_chat_room_params_set_ephemeral_lifetime(params, 0);
		linphone_chat_room_params_set_backend(params, LinphoneChatRoomBackendFlexisipChat);

		LinphoneChatRoom *marieCr = create_chat_room_client_side_with_params(coresList, marie.getCMgr(), &marie_stat, participantsAddresses, initialSubject, params);
		linphone_chat_room_params_unref(params);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &pauline_stat, confAddr, initialSubject, 1, FALSE);

		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(paulineCr));

		pauline_stat=pauline.getStats();
		linphone_chat_room_set_ephemeral_lifetime(marieCr, 10);

		BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getCMgr()->stat.number_of_NotifyReceived,pauline_stat.number_of_NotifyReceived + 1,5000));

		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 10, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(paulineCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(paulineCr), 10, int, "%d");

		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([&focus] {
			for (auto chatRoom :focus.getCore().getChatRooms()) {
				for (auto participant: chatRoom->getParticipants()) {
					for (auto device: participant->getDevices())
						if (device->getState() != ParticipantDevice::State::Present) {
							return false;
						}
				}
			}
			return true;
		}));

		chloe_stat=focus.getStats();
		marie_stat=marie.getStats();
		pauline_stat=pauline.getStats();

		constexpr int noMsg = 10;
		sendEphemeralMessageInAdminMode(focus, marie, pauline, marieCr, paulineCr, "Hello ", noMsg);

		pauline_stat=pauline.getStats();
		linphone_chat_room_set_ephemeral_lifetime(marieCr, 0);

		BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getCMgr()->stat.number_of_NotifyReceived,pauline_stat.number_of_NotifyReceived + 1,5000));

		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(paulineCr));

		pauline_stat=pauline.getStats();
		const LinphoneChatRoomEphemeralMode deviceMode = LinphoneChatRoomEphemeralModeDeviceManaged;
		linphone_chat_room_set_ephemeral_mode(marieCr, deviceMode);

		BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getCMgr()->stat.number_of_NotifyReceived,pauline_stat.number_of_NotifyReceived + 1,5000));

		pauline_stat=pauline.getStats();
		marie_stat=marie.getStats();

		linphone_chat_room_enable_ephemeral(paulineCr, TRUE);
		linphone_chat_room_set_ephemeral_lifetime(paulineCr, 5);

		wait_for_list(coresList, NULL, 1, 2000);

		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), deviceMode, int, "%d");
		BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), deviceMode, int, "%d");
		BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(paulineCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(paulineCr), 5, int, "%d");

		LinphoneChatMessage *nonEphemeralMessage = _send_message(marieCr, "I have disabled ephemeral messages");

		auto marieHistory = linphone_chat_room_get_history(marieCr, 0);
		BC_ASSERT_EQUAL((int)bctbx_list_size(marieHistory), 1, int, "%i");
		bctbx_list_free_with_data(marieHistory, (bctbx_list_free_func)linphone_chat_message_unref);

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageReceived, pauline_stat.number_of_LinphoneMessageReceived + 1,11000));

		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([nonEphemeralMessage] {
			return (linphone_chat_message_get_state(nonEphemeralMessage) == LinphoneChatMessageStateDelivered);
		}));

		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([&, paulineCr] {
			return linphone_chat_room_get_unread_messages_count(paulineCr) == 1;
		}));

		auto paulineHistory = linphone_chat_room_get_history(paulineCr, 0);
		BC_ASSERT_EQUAL((int)bctbx_list_size(paulineHistory), 1, int, "%i");
		bctbx_list_free_with_data(paulineHistory, (bctbx_list_free_func)linphone_chat_message_unref);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDeliveredToUser, marie_stat.number_of_LinphoneMessageDeliveredToUser + 1, 10000));

		// Pauline marks the message as read, check that the state is now displayed on Marie's side
		linphone_chat_room_mark_as_read(paulineCr);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDisplayed, marie_stat.number_of_LinphoneMessageDisplayed + 1, 10000));

		sendEphemeralMessageInAdminMode(focus,pauline, marie, paulineCr, marieCr, "Test ephemeral message ", noMsg);

		if (nonEphemeralMessage) {
			linphone_chat_message_unref(nonEphemeralMessage);
		}

		for (auto chatRoom :focus.getCore().getChatRooms()) {
			for (auto participant: chatRoom->getParticipants()) {
				//  force deletion by removing devices
				bctbx_list_t *empty = bctbx_list_new(NULL);
				Address participantAddress(participant->getAddress().asAddress());
				linphone_chat_room_set_participant_devices(  L_GET_C_BACK_PTR(chatRoom)
														   , L_GET_C_BACK_PTR(&participantAddress)
														   , NULL);
				bctbx_list_free(empty);
			}
		}

		//wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		//to avoid creation attempt of a new chatroom
		LinphoneProxyConfig *config = linphone_core_get_default_proxy_config(focus.getLc());
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		bctbx_list_free(coresList);
	}
}

static void conference_scheduler_state_changed(LinphoneConferenceScheduler *scheduler, LinphoneConferenceSchedulerState state) {
	stats *stat = get_stats(linphone_scheduler_get_core(scheduler));
	if (state == LinphoneConferenceSchedulerStateReady) {
		stat->number_of_ConferenceSchedulerStateReady++;
	}
}

static void conference_scheduler_invitations_sent(LinphoneConferenceScheduler *scheduler, const bctbx_list_t *failed_addresses) {
	stats *stat = get_stats(linphone_scheduler_get_core(scheduler));
	stat->number_of_ConferenceSchedulerInvitationsSent++;
}

static LinphoneAddress * create_conference_on_server(Focus & focus, ClientConference & organizer, std::list<LinphoneCoreManager*> participants, time_t start_time, time_t end_time, const char * subject, const char * description) {
	bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
	coresList = bctbx_list_append(coresList, organizer.getLc());
	std::list<stats> participant_stats;
	for (auto & p : participants) {
		coresList = bctbx_list_append(coresList, p->lc);
		participant_stats.push_back(p->stat);
	}

	stats organizer_stat=organizer.getStats();

	// Marie creates a new group chat room
	LinphoneConferenceScheduler *conference_scheduler = linphone_core_create_conference_scheduler(organizer.getLc());
	LinphoneConferenceSchedulerCbs *cbs = linphone_factory_create_conference_scheduler_cbs(linphone_factory_get());
	linphone_conference_scheduler_cbs_set_state_changed(cbs, conference_scheduler_state_changed);
	linphone_conference_scheduler_cbs_set_invitations_sent(cbs, conference_scheduler_invitations_sent);
	linphone_conference_scheduler_add_callbacks(conference_scheduler, cbs);
	linphone_conference_scheduler_cbs_unref(cbs);

	LinphoneConferenceInfo *conf_info = linphone_conference_info_new();
	linphone_conference_info_set_organizer(conf_info, organizer.getCMgr()->identity);
	for (auto & p : participants) {
		linphone_conference_info_add_participant(conf_info, p->identity);
	}
	if ((end_time >= 0) && (start_time >= 0) && (end_time > start_time)) {
		linphone_conference_info_set_duration(conf_info, (int)((end_time - start_time) / 60)); // duration is expected to be set in minutes
	}
	linphone_conference_info_set_date_time(conf_info, start_time);
	linphone_conference_info_set_subject(conf_info, subject);
	linphone_conference_info_set_description(conf_info, description);

	linphone_conference_scheduler_set_info(conference_scheduler, conf_info);
	linphone_conference_info_unref(conf_info);

	BC_ASSERT_TRUE(wait_for_list(coresList, &organizer.getStats().number_of_ConferenceSchedulerStateReady, organizer_stat.number_of_ConferenceSchedulerStateReady + 1, 10000));

	LinphoneChatRoomParams *chat_room_params = linphone_core_create_default_chat_room_params(organizer.getLc());
	linphone_chat_room_params_set_backend(chat_room_params, LinphoneChatRoomBackendBasic);
	linphone_conference_scheduler_send_invitations(conference_scheduler, chat_room_params);
	linphone_chat_room_params_unref(chat_room_params);

	BC_ASSERT_TRUE(wait_for_list(coresList, &organizer.getStats().number_of_ConferenceSchedulerInvitationsSent, organizer_stat.number_of_ConferenceSchedulerInvitationsSent + 1, 10000));

	linphone_conference_scheduler_unref(conference_scheduler);

	LinphoneAddress * conference_address = NULL;

	for (auto & mgr : participants) {

		auto old_stats = participant_stats.front();
		// chat room in created state
		BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceived, old_stats.number_of_LinphoneMessageReceived + 1, 10000));
		if (!linphone_core_conference_ics_in_message_body_enabled(organizer.getLc())) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceivedWithFile, old_stats.number_of_LinphoneMessageReceivedWithFile + 1, 10000));
		}

		BC_ASSERT_PTR_NOT_NULL(mgr->stat.last_received_chat_message);
		if (mgr->stat.last_received_chat_message != NULL) {
			const string expected = ContentType::Icalendar.getMediaType();
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(mgr->stat.last_received_chat_message), expected.c_str());
		}

		bctbx_list_t *chat_room_participants = bctbx_list_append(NULL, mgr->identity);
		LinphoneChatRoom *cr = linphone_core_search_chat_room(organizer.getLc(), NULL, organizer.getCMgr()->identity, NULL, chat_room_participants);
		bctbx_list_free(chat_room_participants);

		LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(cr);
		linphone_chat_room_unref(cr);

		const bctbx_list_t* original_contents = linphone_chat_message_get_contents(msg);
		BC_ASSERT_EQUAL((int)bctbx_list_size(original_contents), 1, int, "%d");
		LinphoneContent *original_content = (LinphoneContent *) bctbx_list_get_data(original_contents);
		BC_ASSERT_PTR_NOT_NULL(original_content);

		LinphoneConferenceInfo *conf_info_from_original_content = linphone_factory_create_conference_info_from_icalendar_content(linphone_factory_get(), original_content);
		if (BC_ASSERT_PTR_NOT_NULL(conf_info_from_original_content)) {
			if (!conference_address) {
				conference_address = linphone_address_clone(linphone_conference_info_get_uri(conf_info_from_original_content));
			}
			LinphoneConferenceInfo * conf_info_in_db = linphone_core_find_conference_information_from_uri(mgr->lc, conference_address);
			if(!BC_ASSERT_PTR_NOT_NULL(conf_info_in_db)) {
				return conference_address;
			}
			BC_ASSERT_TRUE(linphone_address_weak_equal(organizer.getCMgr()->identity, linphone_conference_info_get_organizer(conf_info_from_original_content)));
			BC_ASSERT_TRUE(linphone_address_equal(linphone_conference_info_get_organizer(conf_info_in_db), linphone_conference_info_get_organizer(conf_info_from_original_content)));
			BC_ASSERT_TRUE(linphone_address_weak_equal(conference_address, linphone_conference_info_get_uri(conf_info_from_original_content)));
			BC_ASSERT_TRUE(linphone_address_equal(linphone_conference_info_get_uri(conf_info_in_db), linphone_conference_info_get_uri(conf_info_from_original_content)));

			const bctbx_list_t * ics_participants = linphone_conference_info_get_participants(conf_info_from_original_content);
			BC_ASSERT_EQUAL(bctbx_list_size(ics_participants), participants.size(), size_t, "%zu");
			bctbx_list_free((bctbx_list_t *)ics_participants);

			const bctbx_list_t * ics_participants_db = linphone_conference_info_get_participants(conf_info_in_db);
			BC_ASSERT_EQUAL(bctbx_list_size(ics_participants_db), participants.size(), size_t, "%zu");
			bctbx_list_free((bctbx_list_t *)ics_participants_db);

			if (start_time > 0) {
				BC_ASSERT_EQUAL((long long)linphone_conference_info_get_date_time(conf_info_from_original_content), (long long)start_time, long long, "%lld");
				BC_ASSERT_EQUAL((long long)linphone_conference_info_get_date_time(conf_info_in_db), (long long)linphone_conference_info_get_date_time(conf_info_from_original_content), long long, "%lld");
				if (end_time > 0) {
					const int duration_s = linphone_conference_info_get_duration(conf_info_from_original_content) * 60;
					BC_ASSERT_EQUAL(duration_s, (int)(end_time - start_time), int, "%d");
					BC_ASSERT_EQUAL((int)linphone_conference_info_get_duration(conf_info_in_db), (int)linphone_conference_info_get_duration(conf_info_from_original_content), int, "%0d");
				}
			}
			if (subject) {
				BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_subject(conf_info_from_original_content), subject);
				BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_subject(conf_info_in_db), linphone_conference_info_get_subject(conf_info_from_original_content));
			} else {
				BC_ASSERT_PTR_NULL(linphone_conference_info_get_subject(conf_info_from_original_content));
				BC_ASSERT_PTR_NULL(linphone_conference_info_get_subject(conf_info_in_db));
			}
			if (description) {
				BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_description(conf_info_from_original_content), description);
				BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_description(conf_info_from_original_content), linphone_conference_info_get_description(conf_info_in_db));
			} else {
				BC_ASSERT_PTR_NULL(linphone_conference_info_get_description(conf_info_from_original_content));
				BC_ASSERT_PTR_NULL(linphone_conference_info_get_description(conf_info_in_db));
			}
			linphone_conference_info_unref(conf_info_from_original_content);
			linphone_conference_info_unref(conf_info_in_db);
		}

		linphone_chat_message_unref(msg);
		participant_stats.pop_front();
	}

	LinphoneConferenceInfo * conf_info_in_db = linphone_core_find_conference_information_from_uri(organizer.getLc(), conference_address);
	if(BC_ASSERT_PTR_NOT_NULL(conf_info_in_db)) {
		BC_ASSERT_TRUE(linphone_address_weak_equal(organizer.getCMgr()->identity, linphone_conference_info_get_organizer(conf_info_in_db)));
		BC_ASSERT_TRUE(linphone_address_weak_equal(conference_address, linphone_conference_info_get_uri(conf_info_in_db)));
		const bctbx_list_t * ics_participants = linphone_conference_info_get_participants(conf_info_in_db);
		BC_ASSERT_EQUAL(bctbx_list_size(ics_participants), participants.size(), size_t, "%zu");
		bctbx_list_free((bctbx_list_t *)ics_participants);
		if (start_time > 0) {
			BC_ASSERT_EQUAL((long long)linphone_conference_info_get_date_time(conf_info_in_db), (long long)start_time, long long, "%lld");
			if (end_time > 0) {
				const int duration_s = linphone_conference_info_get_duration(conf_info_in_db) * 60;
				BC_ASSERT_EQUAL(duration_s, (int)(end_time - start_time), int, "%d");
			}
		}
		if (subject) {
			BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_subject(conf_info_in_db), subject);
		} else {
			BC_ASSERT_PTR_NULL(linphone_conference_info_get_subject(conf_info_in_db));
		}
		if (description) {
			BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_description(conf_info_in_db), description);
		} else {
			BC_ASSERT_PTR_NULL(linphone_conference_info_get_description(conf_info_in_db));
		}
	}
	linphone_conference_info_unref(conf_info_in_db);


	BC_ASSERT_EQUAL(organizer.getStats().number_of_LinphoneConferenceStateTerminationPending, organizer_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
	BC_ASSERT_EQUAL(organizer.getStats().number_of_LinphoneConferenceStateTerminated, organizer_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
	BC_ASSERT_EQUAL(organizer.getStats().number_of_LinphoneConferenceStateDeleted, organizer_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

	bctbx_list_free(coresList);

	char * conference_address_str = (conference_address) ? linphone_address_as_string(conference_address) : ms_strdup("<unknown>");
	ms_message("%s is creating conference %s on server %s", linphone_core_get_identity(organizer.getLc()), conference_address_str, linphone_core_get_identity(focus.getLc()));
	ms_free(conference_address_str);

	return conference_address;
}

static void create_conference_base (time_t start_time, int duration, bool_t add_uninvited_participant, LinphoneConferenceParticipantListType participant_list_type, bool_t remove_participant, const LinphoneMediaEncryption encryption, bool_t enable_video, LinphoneConferenceLayout layout, bool_t enable_ice, bool_t enable_stun, bool_t audio_only_participant, bool_t server_restart) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focus.getIdentity().asAddress());
		ClientConference laure("laure_tcp_rc", focus.getIdentity().asAddress());
		ClientConference michelle("michelle_rc", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);

		setup_conference_info_cbs(marie.getCMgr());

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr()}) {
			if (enable_video) {
				if (!audio_only_participant || (mgr != pauline.getCMgr())) {
					LinphoneVideoActivationPolicy * pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
					linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
					linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
					linphone_core_set_video_activation_policy(mgr->lc, pol);
					linphone_video_activation_policy_unref(pol);
				}

				linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);
				linphone_core_enable_video_capture(mgr->lc, TRUE);
				linphone_core_enable_video_display(mgr->lc, TRUE);
			}

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, layout);
			}

			enable_stun_in_core(mgr, enable_stun, enable_ice);
			linphone_core_manager_wait_for_stun_resolution(mgr);
		}

		linphone_core_set_media_encryption(marie.getLc(),encryption);
		linphone_core_set_media_encryption(pauline.getLc(),encryption);
		linphone_core_set_media_encryption(laure.getLc(),encryption);
		linphone_core_set_media_encryption(michelle.getLc(),encryption);
		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), participant_list_type);

		bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, laure.getLc());
		coresList = bctbx_list_append(coresList, michelle.getLc());

		stats marie_stat=marie.getStats();
		stats focus_stat=focus.getStats();
		stats pauline_stat=pauline.getStats();

		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr()};

		time_t end_time = (duration <= 0) ? -1 : (start_time + duration * 60);
		const char *initialSubject = "Test characters: ^ :) ¤ çà @";
		const char *description = "Paris Baker";

		LinphoneAddress * confAddr = create_conference_on_server(focus, marie, participants, start_time, end_time, initialSubject, description);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 2, 10000));

		if (server_restart) {
			coresList = bctbx_list_remove(coresList, focus.getLc());
			//Restart flexisip
			focus.reStart();

			if (enable_video) {
				LinphoneVideoActivationPolicy * pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
				linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
				linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
				linphone_core_set_video_activation_policy(focus.getLc(), pol);
				linphone_video_activation_policy_unref(pol);

				linphone_core_enable_video_capture(focus.getLc(), TRUE);
				linphone_core_enable_video_display(focus.getLc(), TRUE);
			}

			linphone_core_set_conference_participant_list_type(focus.getLc(), participant_list_type);
			linphone_core_set_default_conference_layout(focus.getLc(), layout);
			coresList = bctbx_list_append(coresList, focus.getLc());
		}

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			// Multistream ZRTP is not supported yet
			if (encryption == LinphoneMediaEncryptionZRTP) {
				linphone_call_params_set_media_encryption (new_params, LinphoneMediaEncryptionSRTP);
			} else {
				linphone_call_params_set_media_encryption (new_params, encryption);
			}
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
		}

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1, 10000));
			int no_streams_running = ((enable_ice) ? 3 : 2);
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating, (no_streams_running - 1), 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, no_streams_running, 10000));
			// Update to add to conference.
			// If ICE is enabled, the addition to a conference may go through a resume of the call
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, ((mgr == marie.getCMgr()) ? 3 : 2), 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyReceived, 1, 10000));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived, focus_stat.number_of_LinphoneCallIncomingReceived + 3, 10000));
		int focus_no_streams_running = ((enable_ice) ? 9 : 6);
		// Update to end ICE negotiations
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote, focus_stat.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running - 3), 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running, 10000));
		// Update to add to conference.
		// If ICE is enabled, the addition to a conference may go through a resume of the call
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated, focus_stat.number_of_LinphoneConferenceStateCreated + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived, focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 3, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive, focus_stat.number_of_LinphoneSubscriptionActive + 3, 5000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added, focus_stat.number_of_participants_added + 3, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added, focus_stat.number_of_participant_devices_added + 3, 10000));

		LinphoneAddress *focus_uri = linphone_address_new(linphone_core_get_identity(focus.getLc()));
		LinphoneConference * fconference = linphone_core_search_conference(focus.getLc(), NULL, focus_uri, confAddr, NULL);
		linphone_address_unref(focus_uri);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure,michelle}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
			linphone_address_unref(uri);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				const LinphoneConferenceParams * conference_params = linphone_conference_get_current_params(pconference);
				int no_participants = 0;
				if (start_time >= 0) {
					BC_ASSERT_EQUAL((long long)linphone_conference_params_get_start_time(conference_params), (long long)start_time, long long, "%lld");
				}
				BC_ASSERT_EQUAL((long long)linphone_conference_params_get_end_time(conference_params), (long long)end_time, long long, "%lld");
				if (mgr == focus.getCMgr()) {
					no_participants = 3;
					BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
				} else {
					no_participants = 2;
					BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
					LinphoneCall * current_call = linphone_core_get_current_call(mgr->lc);
					BC_ASSERT_PTR_NOT_NULL(current_call);
					if (current_call) {
						BC_ASSERT_EQUAL((int)linphone_call_get_state(current_call), (int)LinphoneCallStateStreamsRunning, int, "%0d");
					}
					if (enable_ice) {
						BC_ASSERT_TRUE(check_ice(mgr, focus.getCMgr(), LinphoneIceStateHostConnection));
					}

					LinphoneVideoActivationPolicy * pol = linphone_core_get_video_activation_policy(mgr->lc);
					bool_t enabled = !!linphone_video_activation_policy_get_automatically_initiate(pol);
					LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						const LinphoneCallParams* call_lparams = linphone_call_get_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
						const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
						const LinphoneCallParams* call_params = linphone_call_get_current_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_params), enabled, int, "%0d");
					}
					LinphoneCall * ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
					BC_ASSERT_PTR_NOT_NULL(ccall);
					if (ccall) {
						const LinphoneCallParams* call_lparams = linphone_call_get_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
						const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
						const LinphoneCallParams* call_cparams = linphone_call_get_current_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
					}
					linphone_video_activation_policy_unref(pol);
				}
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int, "%0d");
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
				LinphoneParticipant * me = linphone_conference_get_me (pconference);
				BC_ASSERT_TRUE(linphone_participant_is_admin(me) == ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())));
				BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_participant_get_address(me), mgr->identity));
				bctbx_list_t *participants = linphone_conference_get_participant_list(pconference);
				for (bctbx_list_t *itp = participants; itp; itp = bctbx_list_next(itp)) {
					LinphoneParticipant * p = (LinphoneParticipant *)bctbx_list_get_data(itp);
					BC_ASSERT_TRUE(linphone_participant_is_admin(p) == linphone_address_weak_equal(linphone_participant_get_address(p), marie.getCMgr()->identity));
				}
				bctbx_list_free_with_data(participants, (void(*)(void *))linphone_participant_unref);

				if (mgr != focus.getCMgr()) {
					check_conference_ssrc(fconference, pconference);
				}
			}
		}

		// Wait a little bit
		wait_for_list(coresList, NULL, 0, 3000);

		if (enable_video) {
			LinphoneCall * pauline_call = linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(pauline_call);

			LinphoneVideoActivationPolicy * pol = linphone_core_get_video_activation_policy(pauline.getLc());
			bool_t enable = !!!linphone_video_activation_policy_get_automatically_initiate(pol);
			linphone_video_activation_policy_unref(pol);

			for (int i = 0; i < 4; i++) {
				stats focus_stat2=focus.getStats();
				stats marie_stat2=marie.getStats();
				stats pauline_stat2=pauline.getStats();
				stats laure_stat2=laure.getStats();

				if (pauline_call) {
					LinphoneCallParams * new_params = linphone_core_create_call_params(pauline.getLc(), pauline_call);
					linphone_call_params_enable_video (new_params, enable);
					linphone_call_update(pauline_call, new_params);
					linphone_call_params_unref (new_params);
				}

				int focus_no_streams_running = (3 + ((enable_ice) ? 3 : 2));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived, focus_stat.number_of_LinphoneCallIncomingReceived + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote, focus_stat.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running - 1), 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived, focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 1, 5000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive, focus_stat.number_of_LinphoneSubscriptionActive + 1, 5000));

				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallUpdating, pauline_stat2.number_of_LinphoneCallUpdating + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning, pauline_stat2.number_of_LinphoneCallStreamsRunning + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating, marie_stat2.number_of_LinphoneCallUpdating + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning, marie_stat2.number_of_LinphoneCallStreamsRunning + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallUpdating, laure_stat2.number_of_LinphoneCallUpdating + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallStreamsRunning, laure_stat2.number_of_LinphoneCallStreamsRunning + 1, 10000));

				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_device_media_capability_changed, focus_stat2.number_of_participant_device_media_capability_changed + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_device_media_capability_changed, marie_stat2.number_of_participant_device_media_capability_changed + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_device_media_capability_changed, pauline_stat2.number_of_participant_device_media_capability_changed + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_device_media_capability_changed, laure_stat2.number_of_participant_device_media_capability_changed + 1, 10000));

				for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
					LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
					LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
					linphone_address_unref(uri);
					BC_ASSERT_PTR_NOT_NULL(pconference);
					if (pconference) {
						LinphoneParticipant * p = (mgr == pauline.getCMgr()) ? linphone_conference_get_me(pconference) : linphone_conference_find_participant(pconference, pauline.getCMgr()->identity);
						BC_ASSERT_PTR_NOT_NULL(p);
						if (p) {
							bctbx_list_t *devices = linphone_participant_get_devices (p);
							for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
								LinphoneParticipantDevice * d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
								if (enable == TRUE) {
									BC_ASSERT_TRUE(linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) == LinphoneMediaDirectionSendRecv);
								} else {
									BC_ASSERT_TRUE(linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) == LinphoneMediaDirectionInactive);
								}
							}
							if (devices) {
								bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
							}
						}
					}
				}

				LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
				BC_ASSERT_PTR_NOT_NULL(pcall);
				if (pcall) {
					const LinphoneCallParams* call_lparams = linphone_call_get_params(pcall);
					BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enable, int, "%0d");
					const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(pcall);
					BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enable, int, "%0d");
					const LinphoneCallParams* call_params = linphone_call_get_current_params(pcall);
					BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_params), enable, int, "%0d");
				}
				Address paulineAddr(pauline.getIdentity().asAddress());
				LinphoneCall * ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), L_GET_C_BACK_PTR(&paulineAddr));
				BC_ASSERT_PTR_NOT_NULL(ccall);
				if (ccall) {
					const LinphoneCallParams* call_lparams = linphone_call_get_params(ccall);
					BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enable, int, "%0d");
					const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(ccall);
					BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enable, int, "%0d");
					const LinphoneCallParams* call_cparams = linphone_call_get_current_params(ccall);
					BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enable, int, "%0d");
				}

				LinphoneAddress *paulineUri = linphone_address_new(linphone_core_get_identity(pauline.getLc()));
				LinphoneConference * paulineConference = linphone_core_search_conference(pauline.getLc(), NULL, paulineUri, confAddr, NULL);
				linphone_address_unref(paulineUri);
				BC_ASSERT_PTR_NOT_NULL(paulineConference);

				if (paulineConference) {
					bctbx_list_t *devices = linphone_conference_get_participant_device_list(paulineConference);
					for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
						LinphoneParticipantDevice * d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
						BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) == enable);
					}

					if (devices) {
						bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
					}
				}

				// Wait a little bit
				wait_for_list(coresList, NULL, 0, 1000);

				enable = !enable;
			}
		}

		int no_local_participants = 3;
		if (add_uninvited_participant) {
			stats marie_stat2=marie.getStats();
			stats focus_stat2=focus.getStats();
			stats pauline_stat2=pauline.getStats();
			stats laure_stat2=laure.getStats();

			LinphoneCallParams *params = linphone_core_create_call_params(michelle.getLc(), nullptr);
			LinphoneCall * michelle_call = linphone_core_invite_address_with_params(michelle.getLc(), confAddr, params);
			BC_ASSERT_PTR_NOT_NULL(michelle_call);
			linphone_call_params_unref(params);
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallOutgoingProgress, 1, 10000));
			int extra_participants = 0;
			if (participant_list_type == LinphoneConferenceParticipantListTypeOpen) {
				extra_participants = 1;

				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallStreamsRunning, 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallUpdating, 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallStreamsRunning, 2, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateCreated, 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneSubscriptionActive, 1, 5000));

				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added, focus_stat2.number_of_participants_added + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added, focus_stat2.number_of_participant_devices_added + 1, 10000));

				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_added, marie_stat2.number_of_participants_added + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_added, marie_stat2.number_of_participant_devices_added + 1, 10000));

				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_added, pauline_stat2.number_of_participants_added + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_added, pauline_stat2.number_of_participant_devices_added + 1, 10000));

				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participants_added, laure_stat2.number_of_participants_added + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_added, laure_stat2.number_of_participant_devices_added + 1, 10000));
			} else if (participant_list_type == LinphoneConferenceParticipantListTypeClosed) {
				extra_participants = 0;

				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallError, 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallReleased, 1, 10000));

				//wait bit more to detect side effect if any
				CoreManagerAssert({focus,marie,pauline,laure}).waitUntil(chrono::seconds(2),[] {
					return false;
				});

				BC_ASSERT_EQUAL(michelle.getStats().number_of_LinphoneConferenceStateCreated, 0, int, "%0d");
				BC_ASSERT_EQUAL(michelle.getStats().number_of_LinphoneSubscriptionOutgoingProgress, 0, int, "%0d");
				BC_ASSERT_EQUAL(michelle.getStats().number_of_LinphoneSubscriptionActive, 0, int, "%0d");
				BC_ASSERT_EQUAL(focus.getStats().number_of_participants_added, focus_stat2.number_of_participants_added, int, "%0d");
				BC_ASSERT_EQUAL(focus.getStats().number_of_participant_devices_added, focus_stat2.number_of_participant_devices_added, int, "%0d");
				BC_ASSERT_EQUAL(marie.getStats().number_of_participants_added, marie_stat2.number_of_participants_added, int, "%0d");
				BC_ASSERT_EQUAL(marie.getStats().number_of_participant_devices_added, marie_stat2.number_of_participant_devices_added, int, "%0d");
				BC_ASSERT_EQUAL(pauline.getStats().number_of_participants_added, pauline_stat2.number_of_participants_added, int, "%0d");
				BC_ASSERT_EQUAL(pauline.getStats().number_of_participant_devices_added, pauline_stat2.number_of_participant_devices_added, int, "%0d");
				BC_ASSERT_EQUAL(laure.getStats().number_of_participants_added, laure_stat2.number_of_participants_added, int, "%0d");
				BC_ASSERT_EQUAL(laure.getStats().number_of_participant_devices_added, laure_stat2.number_of_participant_devices_added, int, "%0d");
			}

			for (auto mgr : {focus.getCMgr(), marie.getCMgr(), michelle.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
				LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
				LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
				linphone_address_unref(uri);
				if (participant_list_type == LinphoneConferenceParticipantListTypeOpen) {
					BC_ASSERT_PTR_NOT_NULL(pconference);
				} else if (mgr == michelle.getCMgr()) {
					BC_ASSERT_PTR_NULL(pconference);
				}
				if (pconference) {
					const LinphoneConferenceParams * conference_params = linphone_conference_get_current_params(pconference);
					int no_participants = 0;
					if (start_time >= 0) {
						BC_ASSERT_EQUAL((long long)linphone_conference_params_get_start_time(conference_params), (long long)start_time, long long, "%lld");
					}
					BC_ASSERT_EQUAL((long long)linphone_conference_params_get_end_time(conference_params), (long long)end_time, long long, "%lld");
					if (mgr == focus.getCMgr()) {
						no_participants = no_local_participants + extra_participants;
						BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
					} else {
						// Substracting one because we conference server is not in the conference
						no_participants = (no_local_participants - 1) + extra_participants;
						BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
					}
					BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int, "%0d");
					BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
				}
			}

		}

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		std::list <LinphoneCoreManager*> mgrsToRemove {pauline.getCMgr()};
		if (remove_participant) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(marie.getLc()));
			LinphoneConference * pconference = linphone_core_search_conference(marie.getLc(), NULL, uri, confAddr, NULL);
			linphone_address_unref(uri);

			LinphoneAddress *puri = linphone_address_new(linphone_core_get_identity(laure.getLc()));
			LinphoneParticipant * participant = linphone_conference_find_participant(pconference, puri);
			BC_ASSERT_PTR_NOT_NULL(participant);
			linphone_conference_remove_participant_2(pconference, participant);

			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallEnd, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallReleased, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneSubscriptionTerminated, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateTerminationPending, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateTerminated, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateDeleted, 1, 10000));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed, focus_stat.number_of_participants_removed + 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed, focus_stat.number_of_participant_devices_removed + 1, 10000));

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed, marie_stat.number_of_participants_removed + 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_removed, marie_stat.number_of_participant_devices_removed + 1, 10000));

			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_removed, pauline_stat.number_of_participants_removed + 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_removed, pauline_stat.number_of_participant_devices_removed + 1, 10000));

			if ((add_uninvited_participant) && (participant_list_type == LinphoneConferenceParticipantListTypeOpen)) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participants_removed, 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_removed, 1, 10000));
			}

			LinphoneConference * conference = linphone_core_search_conference(laure.getLc(), NULL, puri, confAddr, NULL);
			BC_ASSERT_PTR_NULL(conference);
			linphone_address_unref(puri);
		} else {
			mgrsToRemove.push_back(laure.getCMgr());
		}
		for (auto mgr : mgrsToRemove) {
			LinphoneCall * call = linphone_core_get_current_call(mgr->lc);
			BC_ASSERT_PTR_NOT_NULL(call);
			if (call) {
				linphone_call_terminate(call);
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated, 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated, 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1, 10000));

				LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
				LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
				BC_ASSERT_PTR_NULL(pconference);
				linphone_address_unref(uri);
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed, focus_stat.number_of_participants_removed + 2, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed, focus_stat.number_of_participant_devices_removed + 2, 10000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed, marie_stat.number_of_participants_removed + 2, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_removed, marie_stat.number_of_participant_devices_removed + 2, 10000));

		if (add_uninvited_participant) {
			if (participant_list_type == LinphoneConferenceParticipantListTypeOpen) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participants_removed, 2, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_removed, 2, 10000));

				for (auto mgr : {focus.getCMgr(), marie.getCMgr(), michelle.getCMgr()}) {
					LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
					LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
					BC_ASSERT_PTR_NOT_NULL(pconference);
					linphone_address_unref(uri);
					if (pconference) {
						BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), ((mgr == focus.getCMgr()) ? 2 : 1), int, "%0d");
						BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
					}
				}

				stats marie_stat2=marie.getStats();
				stats focus_stat2=focus.getStats();

				LinphoneCall * call = linphone_core_get_current_call(michelle.getLc());
				BC_ASSERT_PTR_NOT_NULL(call);
				if (call) {
					linphone_call_terminate(call);
					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallEnd, 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallReleased, 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneSubscriptionTerminated, 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateTerminationPending, 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateTerminated, 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateDeleted, 1, 10000));
				}

				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed, focus_stat2.number_of_participants_removed + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed, focus_stat2.number_of_participant_devices_removed + 1, 10000));

				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed, marie_stat2.number_of_participants_removed + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_removed, marie_stat2.number_of_participant_devices_removed + 1, 10000));
			} else if (participant_list_type == LinphoneConferenceParticipantListTypeClosed) {
				LinphoneCall * call = linphone_core_get_current_call(michelle.getLc());
				BC_ASSERT_PTR_NULL(call);

				LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(michelle.getLc()));
				LinphoneConference * pconference = linphone_core_search_conference(michelle.getLc(), NULL, uri, confAddr, NULL);
				BC_ASSERT_PTR_NULL(pconference);
				linphone_address_unref(uri);
			}
		}

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		BC_ASSERT_EQUAL(marie.getStats().number_of_LinphoneConferenceStateTerminationPending, marie_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(marie.getStats().number_of_LinphoneConferenceStateTerminated, marie_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(marie.getStats().number_of_LinphoneConferenceStateDeleted, marie_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminationPending, focus_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminated, focus_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateDeleted, focus_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		for (auto mgr : {focus.getCMgr(), marie.getCMgr()}) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			linphone_address_unref(uri);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), ((mgr == focus.getCMgr()) ? 1 : 0), int, "%0d");
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
			}
		}

		LinphoneCall * call = linphone_core_get_current_call(marie.getLc());
		BC_ASSERT_PTR_NOT_NULL(call);
		if (call) {
			linphone_call_terminate(call);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallEnd, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallReleased, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionTerminated, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminationPending, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminated, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateDeleted, 1, 10000));

			// Explicitely terminate conference as those on server are static by default
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(focus.getLc()));
			LinphoneConference * pconference = linphone_core_search_conference(focus.getLc(), NULL, uri, confAddr, NULL);
			linphone_address_unref(uri);
			linphone_conference_terminate(pconference);
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1, 10000));
		}

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			const bctbx_list_t* call_logs = linphone_core_get_call_logs(mgr->lc);
			BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(call_logs), 1, unsigned int, "%u");

			bctbx_list_t * mgr_focus_call_log = linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
			if (mgr_focus_call_log) {
				BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(mgr_focus_call_log), 1, unsigned int, "%u");
				for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
					LinphoneCallLog *call_log=(LinphoneCallLog *)it->data;
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				}
				bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func) linphone_call_log_unref);
			}
		}

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure,michelle}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void create_simple_conference (void) {
	create_conference_base (-1, -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionNone, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE);
}

static void create_simple_conference_with_server_restart (void) {
	create_conference_base (-1, -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionNone, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, TRUE);
}

static void create_simple_ice_conference (void) {
	create_conference_base (-1, -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, TRUE, FALSE, FALSE, FALSE);
}

static void create_simple_stun_ice_conference (void) {
	create_conference_base (-1, -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, TRUE, TRUE, FALSE, FALSE);
}

static void create_simple_srtp_conference (void) {
	create_conference_base (-1, -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionSRTP, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE);
}

static void create_simple_ice_srtp_conference (void) {
	create_conference_base (-1, -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionSRTP, TRUE, LinphoneConferenceLayoutGrid, TRUE, FALSE, FALSE, FALSE);
}

static void create_simple_stun_ice_srtp_conference (void) {
	create_conference_base (-1, -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionSRTP, TRUE, LinphoneConferenceLayoutGrid, TRUE, TRUE, FALSE, FALSE);
}

static void create_conference_with_uninvited_participant (void) {
	create_conference_base (-1, -1, TRUE, LinphoneConferenceParticipantListTypeOpen, TRUE, LinphoneMediaEncryptionNone, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE);
}

static void create_conference_with_uninvited_participant_not_allowed (void) {
	create_conference_base (-1, -1, TRUE, LinphoneConferenceParticipantListTypeClosed, FALSE, LinphoneMediaEncryptionNone, FALSE, LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE, FALSE, FALSE);
}

static void create_conference_starting_immediately (void) {
	create_conference_base (ms_time(NULL), 0, FALSE, LinphoneConferenceParticipantListTypeClosed, FALSE, LinphoneMediaEncryptionNone, FALSE, LinphoneConferenceLayoutLegacy, FALSE, FALSE, FALSE, FALSE);
}

static void create_conference_starting_in_the_past (void) {
	create_conference_base (ms_time(NULL) - 600, 900, FALSE, LinphoneConferenceParticipantListTypeClosed, TRUE, LinphoneMediaEncryptionNone, FALSE, LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE, FALSE, FALSE);
}

static void create_simple_conference_with_audio_only_participant (void) {
	create_conference_base (-1, -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, TRUE, FALSE);
}

static void create_simple_ice_conference_with_audio_only_participant (void) {
	create_conference_base (-1, -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, TRUE, TRUE, TRUE, FALSE);
}

static void create_simple_stun_ice_conference_with_audio_only_participant (void) {
	create_conference_base (-1, -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, TRUE, TRUE, TRUE, FALSE);
}

static void create_simple_stun_ice_srtp_conference_with_audio_only_participant (void) {
	create_conference_base (-1, -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionSRTP, TRUE, LinphoneConferenceLayoutGrid, TRUE, TRUE, TRUE, FALSE);
}

static void create_conference_with_audio_only_and_uninvited_participant (void) {
	create_conference_base (-1, -1, TRUE, LinphoneConferenceParticipantListTypeOpen, TRUE, LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, TRUE, FALSE);
}

static void create_simple_conference_with_audio_only_participant_enabling_video (void) {
	create_conference_base (-1, -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, TRUE, FALSE);
}

#if 0
static void conference_with_participant_added_outside_valid_time_slot (bool_t before_start) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focus.getIdentity().asAddress());
		ClientConference laure("laure_tcp_rc", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);

		setup_conference_info_cbs(marie.getCMgr());

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), LinphoneConferenceParticipantListTypeOpen);

		bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, laure.getLc());

		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr()};
		time_t start_time = (time_t)-1;
		time_t end_time = (time_t)-1;

		if (before_start) {
			start_time = ms_time(NULL) + 600;
		} else {
			start_time = ms_time(NULL) - 60;
		}
		end_time = start_time + 60;
		const char *initialSubject = "Colleagues";
		const char *description = "Tom Black";

		LinphoneAddress* confAddr = create_conference_on_server(focus, marie, participants, start_time, end_time, initialSubject, description);
		BC_ASSERT_PTR_NOT_NULL(confAddr);
		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 2, 10000));

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
		}

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1, 10000));
		}

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);

	}
}

static void conference_with_participants_added_after_end (void) {
	conference_with_participant_added_outside_valid_time_slot(FALSE);
}

static void conference_with_participants_added_before_start (void) {
	conference_with_participant_added_outside_valid_time_slot(TRUE);
}
#endif

static void two_overlapping_conferences_base (bool_t same_organizer) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focus.getIdentity().asAddress());
		ClientConference laure("laure_tcp_rc", focus.getIdentity().asAddress());
		ClientConference michelle("michelle_rc", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		setup_conference_info_cbs(marie.getCMgr());
		if (!same_organizer) {
			linphone_core_set_file_transfer_server(michelle.getLc(), file_transfer_url);
			setup_conference_info_cbs(michelle.getCMgr());
		}

		linphone_core_set_conference_participant_list_type(focus.getLc(), LinphoneConferenceParticipantListTypeOpen);
		linphone_core_set_default_conference_layout(focus.getLc(), LinphoneConferenceLayoutLegacy);

		stats focus_stat = focus.getStats();

		bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, laure.getLc());
		coresList = bctbx_list_append(coresList, michelle.getLc());

		std::list<LinphoneCoreManager *> participants1{pauline.getCMgr(), laure.getCMgr()};
		time_t start_time1 = ms_time(NULL);
		time_t end_time1 = start_time1 + 600;
		const char *subject1 = "Colleagues";
		const char *description1 = NULL;
		LinphoneAddress * confAddr1 = create_conference_on_server(focus, marie, participants1, start_time1, end_time1, subject1, description1);
		BC_ASSERT_PTR_NOT_NULL(confAddr1);
		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 2, 10000));

		time_t start_time2 = ms_time(NULL);
		time_t end_time2 = start_time2 + 600;
		const char *subject2 = "All Hands Q3 FY2021 - Attendance Mandatory";
		const char *description2 = "Financial result - Internal only - Strictly confidential";
		LinphoneAddress * confAddr2 = NULL;
		if (same_organizer) {
			std::list<LinphoneCoreManager *> participants2{pauline.getCMgr(), michelle.getCMgr()};
			confAddr2 = create_conference_on_server(focus, marie, participants2, start_time2, end_time2, subject2, description2);

			// Chat room creation to send ICS
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 3, 10000));

		} else {
			std::list<LinphoneCoreManager *> participants2{pauline.getCMgr(), marie.getCMgr()};
			confAddr2 = create_conference_on_server(focus, michelle, participants2, start_time2, end_time2, subject2, description2);

			// Chat room creation to send ICS
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateCreated, 2, 10000));
		}
		BC_ASSERT_PTR_NOT_NULL(confAddr2);

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			char * conference_address_str = (confAddr1) ? linphone_address_as_string(confAddr1) : ms_strdup("<unknown>");
			ms_message("%s is entering conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
			ms_free(conference_address_str);
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr1, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
		}

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 2, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
			LinphoneCall * currentCall = linphone_core_get_current_call(mgr->lc);
			BC_ASSERT_PTR_NOT_NULL(currentCall);
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr1, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			linphone_address_unref(uri);
			if (currentCall && pconference) {
				BC_ASSERT_PTR_EQUAL(linphone_call_get_conference(currentCall), pconference);
			}
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyReceived, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived, focus_stat.number_of_LinphoneCallIncomingReceived + 3, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote, focus_stat.number_of_LinphoneCallUpdatedByRemote + 3, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + 6, 10000));
		// Update to add to conference.
		// If ICE is enabled, the addition to a conference may go through a resume of the call
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated, focus_stat.number_of_LinphoneConferenceStateCreated + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived, focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 3, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive, focus_stat.number_of_LinphoneSubscriptionActive + 3, 5000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added, focus_stat.number_of_participants_added + 3, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added, focus_stat.number_of_participant_devices_added + 3, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + 3, 10000));

		LinphoneAddress *focus_uri1 = linphone_address_new(linphone_core_get_identity(focus.getLc()));
		LinphoneConference * fconference1 = linphone_core_search_conference(focus.getLc(), NULL, focus_uri1, confAddr1, NULL);
		linphone_address_unref(focus_uri1);
		BC_ASSERT_PTR_NOT_NULL(fconference1);

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure,michelle}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr1, NULL);
			linphone_address_unref(uri);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				const LinphoneConferenceParams * conference_params = linphone_conference_get_current_params(pconference);
				int no_participants = 0;
				if (start_time1 >= 0) {
					BC_ASSERT_EQUAL((long long)linphone_conference_params_get_start_time(conference_params), (long long)start_time1, long long, "%lld");
				}
				BC_ASSERT_EQUAL((long long)linphone_conference_params_get_end_time(conference_params), (long long)end_time1, long long, "%lld");
				if (mgr == focus.getCMgr()) {
					no_participants = 3;
					BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
				} else {
					no_participants = 2;
					BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
				}
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int, "%0d");
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), subject1);
				LinphoneParticipant * me = linphone_conference_get_me (pconference);
				BC_ASSERT_TRUE(linphone_participant_is_admin(me) == ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())));
				BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_participant_get_address(me), mgr->identity));
				bctbx_list_t *participants = linphone_conference_get_participant_list(pconference);
				for (bctbx_list_t *itp = participants; itp; itp = bctbx_list_next(itp)) {
					LinphoneParticipant * p = (LinphoneParticipant *)bctbx_list_get_data(itp);
					BC_ASSERT_TRUE(linphone_participant_is_admin(p) == linphone_address_weak_equal(linphone_participant_get_address(p), marie.getCMgr()->identity));
				}
				bctbx_list_free_with_data(participants, (void(*)(void *))linphone_participant_unref);

				if (mgr != focus.getCMgr()) {
					check_conference_ssrc(fconference1, pconference);
				}
			}
		}

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), michelle.getCMgr()}) {
			char * conference_address_str = (confAddr1) ? linphone_address_as_string(confAddr2) : ms_strdup("<unknown>");
			ms_message("%s is entering conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
			ms_free(conference_address_str);
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr2, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
		}

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr()}) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 2, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 3, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallPaused, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, ((mgr == marie.getCMgr()) ? 4 : 3), 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 2, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 2, 5000));
			LinphoneCall * currentCall = linphone_core_get_current_call(mgr->lc);
			BC_ASSERT_PTR_NOT_NULL(currentCall);
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr2, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			linphone_address_unref(uri);
			if (currentCall && pconference) {
				BC_ASSERT_PTR_EQUAL(linphone_call_get_conference(currentCall), pconference);
			}
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating, 2, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 4, 10000));
		}

		for (auto mgr : {michelle.getCMgr()}) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, ((same_organizer) ? 2 : 3), 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
			LinphoneCall * currentCall = linphone_core_get_current_call(mgr->lc);
			BC_ASSERT_PTR_NOT_NULL(currentCall);
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr2, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			linphone_address_unref(uri);
			if (currentCall && pconference) {
				BC_ASSERT_PTR_EQUAL(linphone_call_get_conference(currentCall), pconference);
			}
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyReceived, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived, focus_stat.number_of_LinphoneCallIncomingReceived + 6, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote, focus_stat.number_of_LinphoneCallUpdatedByRemote + 6, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + 12, 10000));
		// Update to add to conference.
		// If ICE is enabled, the addition to a conference may go through a resume of the call
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated, focus_stat.number_of_LinphoneConferenceStateCreated + 2, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived, focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 6, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive, focus_stat.number_of_LinphoneSubscriptionActive + 6, 5000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added, focus_stat.number_of_participants_added + 6, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added, focus_stat.number_of_participant_devices_added + 6, 10000));

		// Marie and Pauline leave conference1
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_device_left, focus_stat.number_of_participant_device_left + 2, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_device_media_capability_changed, 2, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_device_left, 2, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_device_media_capability_changed, 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_device_left, 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_device_media_capability_changed, 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_device_left, 1, 10000));

		LinphoneAddress *focus_uri2 = linphone_address_new(linphone_core_get_identity(focus.getLc()));
		LinphoneConference * fconference2 = linphone_core_search_conference(focus.getLc(), NULL, focus_uri2, confAddr2, NULL);
		linphone_address_unref(focus_uri2);
		BC_ASSERT_PTR_NOT_NULL(fconference2);

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr1, NULL);
			linphone_address_unref(uri);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				if (mgr == focus.getCMgr()) {
					BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), 3, int, "%0d");
				} else {
					BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), 2, int, "%0d");
				}
				if (mgr == laure.getCMgr()) {
					BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
				} else {
					BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
				}
			}
		}

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure,michelle}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		auto & organizer2 = (same_organizer) ? marie : michelle;
		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), michelle.getCMgr()}) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference_2(mgr->lc, confAddr2);
			linphone_address_unref(uri);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				const LinphoneConferenceParams * conference_params = linphone_conference_get_current_params(pconference);
				int no_participants = 0;
				if (start_time2 >= 0) {
					BC_ASSERT_EQUAL((long long)linphone_conference_params_get_start_time(conference_params), (long long)start_time2, long long, "%lld");
				}
				BC_ASSERT_EQUAL((long long)linphone_conference_params_get_end_time(conference_params), (long long)end_time2, long long, "%lld");
				if (mgr == focus.getCMgr()) {
					no_participants = 3;
					BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
				} else {
					no_participants = 2;
					BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
				}
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int, "%0d");
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), subject2);

				LinphoneParticipant * me = linphone_conference_get_me (pconference);

				BC_ASSERT_TRUE(linphone_participant_is_admin(me) == ((mgr == organizer2.getCMgr()) || (mgr == focus.getCMgr())));
				BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_participant_get_address(me), mgr->identity));
				bctbx_list_t *participants = linphone_conference_get_participant_list(pconference);
				for (bctbx_list_t *itp = participants; itp; itp = bctbx_list_next(itp)) {
					LinphoneParticipant * p = (LinphoneParticipant *)bctbx_list_get_data(itp);
					BC_ASSERT_TRUE(linphone_participant_is_admin(p) == linphone_address_weak_equal(linphone_participant_get_address(p), organizer2.getCMgr()->identity));
				}
				bctbx_list_free_with_data(participants, (void(*)(void *))linphone_participant_unref);

				if (mgr != focus.getCMgr()) {
					check_conference_ssrc(fconference2, pconference);
				}
			}
		}

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		focus_stat = focus.getStats();
		stats pauline_stat = pauline.getStats();

		// Marie and Michelle leave conference2
		for (auto mgr : {marie.getCMgr(), michelle.getCMgr()}) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr2, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				char * conference_address_str = (confAddr2) ? linphone_address_as_string(confAddr2) : ms_strdup("<unknown>");
				ms_message("%s is terminating conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
				ms_free(conference_address_str);
				linphone_conference_terminate(pconference);
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated, 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated, 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1, 10000));

				LinphoneConference * pconference1 = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr2, NULL);
				BC_ASSERT_PTR_NULL(pconference1);
			}
			linphone_address_unref(uri);
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed, focus_stat.number_of_participants_removed + 2, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed, focus_stat.number_of_participant_devices_removed + 2, 10000));
 
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_removed, pauline_stat.number_of_participants_removed + 2, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_removed, pauline_stat.number_of_participant_devices_removed + 2, 10000));

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneConferenceStateTerminationPending, pauline_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneConferenceStateTerminated, pauline_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneConferenceStateDeleted, pauline_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminationPending, focus_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminated, focus_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateDeleted, focus_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		for (auto mgr : {focus.getCMgr(), pauline.getCMgr()}) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr2, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			linphone_address_unref(uri);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), ((mgr == focus.getCMgr()) ? 1 : 0), int, "%0d");
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), subject2);
			}
		}

		// Marie and Pauline rejoin conference1 (Pauline leaves conference2)
		focus_stat = focus.getStats();
		pauline_stat = pauline.getStats();
		stats marie_stat = marie.getStats();
		for (auto mgr : {marie.getCMgr(), pauline.getCMgr()}) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr1, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			linphone_address_unref(uri);
			if (pconference) {
				char * conference_address_str = (confAddr1) ? linphone_address_as_string(confAddr1) : ms_strdup("<unknown>");
				ms_message("%s is joining conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
				ms_free(conference_address_str);
				linphone_conference_enter(pconference);
			}
		}

		LinphoneAddress *focusUri = linphone_address_new(linphone_core_get_identity(focus.getLc()));
		LinphoneConference * conference1 = linphone_core_search_conference(focus.getLc(), NULL, focusUri, confAddr1, NULL);

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallPaused, pauline_stat.number_of_LinphoneCallPaused + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning, pauline_stat.number_of_LinphoneCallStreamsRunning + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning, marie_stat.number_of_LinphoneCallStreamsRunning + 1, 10000));

		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneConferenceStateTerminationPending, pauline_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneConferenceStateTerminated, pauline_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneConferenceStateDeleted, pauline_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminationPending, focus_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminated, focus_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateDeleted, focus_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		// Pauline leaves conference2
		// Pauline and Marie enter conference1
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_device_left, focus_stat.number_of_participant_device_left + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_device_joined, focus_stat.number_of_participant_device_joined + 2, 10000));

		for (auto mgr : {focus.getCMgr(), pauline.getCMgr()}) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr2, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			linphone_address_unref(uri);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), ((mgr == focus.getCMgr()) ? 1 : 0), int, "%0d");
				BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), subject2);
			}
		}

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure,michelle}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		// Laure and Pauline are removed from conference1
		focus_stat = focus.getStats();
		marie_stat = marie.getStats();
		pauline_stat = pauline.getStats();
		stats laure_stat = laure.getStats();
		int cnt = 0;
		for (auto mgr : {laure.getCMgr(), pauline.getCMgr()}) {
			cnt++;
			LinphoneParticipant  * participant = linphone_conference_find_participant(conference1, mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(participant);
			if (participant) {
				char * conference_address_str = (confAddr1) ? linphone_address_as_string(confAddr1) : ms_strdup("<unknown>");
				char * conference1_me = linphone_address_as_string(linphone_participant_get_address(linphone_conference_get_me(conference1)));
				ms_message("%s is removing participant %s from conference %s", conference1_me, linphone_core_get_identity(mgr->lc), conference_address_str);
				ms_free(conference_address_str);
				ms_free(conference1_me);
				linphone_conference_remove_participant_2(conference1, participant);
			}
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd, focus_stat.number_of_LinphoneCallEnd + cnt, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased, focus_stat.number_of_LinphoneCallReleased + cnt, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed, focus_stat.number_of_participants_removed + cnt, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed, focus_stat.number_of_participant_devices_removed + cnt, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed, marie_stat.number_of_participants_removed + cnt, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_removed, marie_stat.number_of_participant_devices_removed + cnt, 10000));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallEnd, laure_stat.number_of_LinphoneCallEnd + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallReleased, laure_stat.number_of_LinphoneCallReleased + 1, 10000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallEnd, pauline_stat.number_of_LinphoneCallEnd + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallReleased, pauline_stat.number_of_LinphoneCallReleased + 1, 10000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneSubscriptionTerminated, laure_stat.number_of_LinphoneSubscriptionTerminated + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateTerminationPending, laure_stat.number_of_LinphoneConferenceStateTerminationPending + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateTerminated, laure_stat.number_of_LinphoneConferenceStateTerminated + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateDeleted, laure_stat.number_of_LinphoneConferenceStateDeleted + 1, 10000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneSubscriptionTerminated, pauline_stat.number_of_LinphoneSubscriptionTerminated + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateTerminationPending, pauline_stat.number_of_LinphoneConferenceStateTerminationPending + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateTerminated, pauline_stat.number_of_LinphoneConferenceStateTerminated + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateDeleted, pauline_stat.number_of_LinphoneConferenceStateDeleted + 1, 10000));

		BC_ASSERT_EQUAL(marie.getStats().number_of_LinphoneConferenceStateTerminationPending, marie_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(marie.getStats().number_of_LinphoneConferenceStateTerminated, marie_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(marie.getStats().number_of_LinphoneConferenceStateDeleted, marie_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");


		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure,michelle}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		// Pauline rejoins and leaves conference2 (conference2 is destroyed on the server)
		focus_stat = focus.getStats();
		pauline_stat = pauline.getStats();
		for (auto mgr : {pauline.getCMgr()}) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr2, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			linphone_address_unref(uri);
			if (pconference) {
				char * conference_address_str = (confAddr2) ? linphone_address_as_string(confAddr2) : ms_strdup("<unknown>");
				ms_message("%s is entering conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
				ms_free(conference_address_str);
				linphone_conference_enter(pconference);
			}
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning, pauline_stat.number_of_LinphoneCallStreamsRunning + 1, 10000));

		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneConferenceStateTerminationPending, pauline_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneConferenceStateTerminated, pauline_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneConferenceStateDeleted, pauline_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		// Pauline enters conference2
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_device_joined, focus_stat.number_of_participant_device_joined + 1, 10000));

		for (auto mgr : {focus.getCMgr(), pauline.getCMgr()}) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr2, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			linphone_address_unref(uri);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), ((mgr == focus.getCMgr()) ? 1 : 0), int, "%0d");
				BC_ASSERT_TRUE(linphone_conference_is_in(pconference) == (mgr != focus.getCMgr()));
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), subject2);
			}
		}

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure,michelle}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		// Pauline leaves conference2
		focus_stat = focus.getStats();
		pauline_stat = pauline.getStats();
		for (auto mgr : {pauline.getCMgr()}) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr2, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				char * conference_address_str = (confAddr2) ? linphone_address_as_string(confAddr2) : ms_strdup("<unknown>");
				ms_message("%s is terminating conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
				ms_free(conference_address_str);
				linphone_conference_terminate(pconference);
			}
			linphone_address_unref(uri);
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallEnd, pauline_stat.number_of_LinphoneCallEnd + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallReleased, pauline_stat.number_of_LinphoneCallReleased + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneSubscriptionTerminated, pauline_stat.number_of_LinphoneSubscriptionTerminated + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateTerminationPending, pauline_stat.number_of_LinphoneConferenceStateTerminationPending + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateTerminated, pauline_stat.number_of_LinphoneConferenceStateTerminated + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateDeleted, pauline_stat.number_of_LinphoneConferenceStateDeleted + 1, 10000));

		// Explicitely terminate conference as those on server are static by default
		LinphoneConference * conference2 = linphone_core_search_conference(focus.getLc(), NULL, focusUri, confAddr2, NULL);
		linphone_conference_terminate(conference2);
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending, focus_stat.number_of_LinphoneConferenceStateTerminationPending + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, focus_stat.number_of_LinphoneConferenceStateTerminated + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, focus_stat.number_of_LinphoneConferenceStateDeleted + 1, 10000));

		// Marie terminates conference1 (conference1 is destroyed on the server)
		focus_stat = focus.getStats();
		marie_stat = marie.getStats();
		for (auto mgr : {marie.getCMgr()}) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr1, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				char * conference_address_str = (confAddr1) ? linphone_address_as_string(confAddr1) : ms_strdup("<unknown>");
				ms_message("%s is terminating conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
				ms_free(conference_address_str);
				linphone_conference_terminate(pconference);
			}
			linphone_address_unref(uri);
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallEnd, marie_stat.number_of_LinphoneCallEnd + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallReleased, marie_stat.number_of_LinphoneCallReleased + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionTerminated, marie_stat.number_of_LinphoneSubscriptionTerminated + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminationPending, marie_stat.number_of_LinphoneConferenceStateTerminationPending + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminated, marie_stat.number_of_LinphoneConferenceStateTerminated + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateDeleted, marie_stat.number_of_LinphoneConferenceStateDeleted + 1, 10000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd, focus_stat.number_of_LinphoneCallEnd + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased, focus_stat.number_of_LinphoneCallReleased + 1, 10000));

		// Explicitely terminate conference as those on server are static by default
		linphone_conference_terminate(conference1);
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending, focus_stat.number_of_LinphoneConferenceStateTerminationPending + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, focus_stat.number_of_LinphoneConferenceStateTerminated + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, focus_stat.number_of_LinphoneConferenceStateDeleted + 1, 10000));

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure,michelle}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		linphone_address_unref(focusUri);
		linphone_address_unref(confAddr1);
		linphone_address_unref(confAddr2);
		bctbx_list_free(coresList);

	}
}

static void organizer_schedule_two_conferences (void) {
	two_overlapping_conferences_base(TRUE);
}

static void two_overlapping_conferences_from_difference_organizers (void) {
	two_overlapping_conferences_base(FALSE);
}

static void create_simple_conference_merging_calls_base(bool_t enable_ice) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focus.getIdentity().asAddress());
		ClientConference laure("laure_tcp_rc", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);

		setup_conference_info_cbs(marie.getCMgr());

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);

		LinphoneProxyConfig *focus_proxy_config = linphone_core_get_default_proxy_config(focus.getLc());
		const char *focus_uri = linphone_proxy_config_get_identity(focus_proxy_config);

		LinphoneProxyConfig *marie_proxy = linphone_core_get_default_proxy_config(marie.getLc());
		linphone_proxy_config_edit(marie_proxy);
		linphone_proxy_config_set_conference_factory_uri(marie_proxy, focus_uri);
		linphone_proxy_config_done(marie_proxy);

		bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, laure.getLc());

		BC_ASSERT_TRUE(call(marie.getCMgr(),pauline.getCMgr()));

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			enable_stun_in_core(mgr, enable_ice, enable_ice);
		}

		LinphoneCall* marie_call_pauline=linphone_core_get_current_call(marie.getLc());
		BC_ASSERT_PTR_NOT_NULL(marie_call_pauline);
		LinphoneCall* pauline_called_by_marie=linphone_core_get_current_call(pauline.getLc());
		BC_ASSERT_PTR_NOT_NULL(pauline_called_by_marie);
		BC_ASSERT_TRUE(pause_call_1(marie.getCMgr(),marie_call_pauline,pauline.getCMgr(),pauline_called_by_marie));

		BC_ASSERT_TRUE(call(marie.getCMgr(),laure.getCMgr()));
		LinphoneCall* marie_call_laure=linphone_core_get_current_call(marie.getLc());
		BC_ASSERT_PTR_NOT_NULL(marie_call_laure);

		//marie creates the conference
		LinphoneConferenceParams *conf_params = linphone_core_create_conference_params_2(marie.getLc(), NULL);
		const char *initialSubject = "Test characters: ^ :) ¤ çà @";
		linphone_conference_params_set_subject(conf_params, initialSubject);
		LinphoneConference *conf = linphone_core_create_conference_with_params(marie.getLc(), conf_params);
		linphone_conference_params_unref(conf_params);
		BC_ASSERT_PTR_NOT_NULL(conf);


		if (conf) {
			const bctbx_list_t * calls = linphone_core_get_calls(marie.getLc());
			for (const bctbx_list_t *it = calls; it; it = bctbx_list_next(it)) {
				LinphoneCall *call=(LinphoneCall *)it->data;
				linphone_conference_add_participant(conf, call);
			}
		}


		BC_ASSERT_TRUE(wait_for_list(coresList,&marie.getStats().number_of_LinphoneConferenceStateCreationPending, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList,&marie.getStats().number_of_LinphoneConferenceStateCreated, 1, 5000));

		LinphoneAddress *confAddr = conf ? linphone_address_clone(linphone_conference_get_conference_address(conf)) : NULL;
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		int counter = 0;
		for (auto mgr : {pauline.getCMgr(), laure.getCMgr()}) {
			counter++;
			BC_ASSERT_TRUE(wait_for_list(coresList,&mgr->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList,&mgr->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));

			BC_ASSERT_TRUE(wait_for_list(coresList,&mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));

			BC_ASSERT_TRUE(wait_for_list(coresList,&mgr->stat.number_of_LinphoneSubscriptionActive, 1, 3000));

			BC_ASSERT_TRUE(wait_for_list(coresList,&focus.getStats().number_of_LinphoneCallStreamsRunning,counter + 1,5000));
			BC_ASSERT_TRUE(wait_for_list(coresList,&marie.getStats().number_of_LinphoneTransferCallConnected,counter,5000));

			// End of call between conference and participant
			BC_ASSERT_TRUE(wait_for_list(coresList,&marie.getStats().number_of_LinphoneCallEnd,counter,5000));
			BC_ASSERT_TRUE(wait_for_list(coresList,&mgr->stat.number_of_LinphoneCallEnd,1,5000));

			BC_ASSERT_TRUE(wait_for_list(coresList,&marie.getStats().number_of_LinphoneCallReleased,counter,5000));
			BC_ASSERT_TRUE(wait_for_list(coresList,&mgr->stat.number_of_LinphoneCallReleased,1,5000));

			BC_ASSERT_TRUE(wait_for_list(coresList,&mgr->stat.number_of_NotifyReceived, 1,5000));
			// Local conference
			LinphoneCall * focus_call = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(focus_call);
			if (focus_call) {
				BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(focus_call));
				BC_ASSERT_TRUE(linphone_call_is_in_conference(focus_call));
			}

			// Remote  conference
			LinphoneCall * participant_call = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
			BC_ASSERT_PTR_NOT_NULL(participant_call);
			if (participant_call) {
				BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(participant_call));
				BC_ASSERT_FALSE(linphone_call_is_in_conference(participant_call));
			}

			if (confAddr) {
				LinphoneConferenceInfo * conf_info_in_db = linphone_core_find_conference_information_from_uri(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(conf_info_in_db);

				if(conf_info_in_db) {
					BC_ASSERT_TRUE(linphone_address_weak_equal(marie.getCMgr()->identity, linphone_conference_info_get_organizer(conf_info_in_db)));
					BC_ASSERT_TRUE(linphone_address_weak_equal(confAddr, linphone_conference_info_get_uri(conf_info_in_db)));

					const bctbx_list_t * participants_db = linphone_conference_info_get_participants(conf_info_in_db);
					BC_ASSERT_EQUAL(bctbx_list_size(participants_db), 3, size_t, "%zu");
					bctbx_list_free((bctbx_list_t *)participants_db);

					BC_ASSERT_NOT_EQUAL((long long)linphone_conference_info_get_date_time(conf_info_in_db), 0, long long, "%lld");
					const int duration_s = linphone_conference_info_get_duration(conf_info_in_db) * 60;
					BC_ASSERT_EQUAL(duration_s, 0, int, "%d");
					BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_subject(conf_info_in_db), initialSubject);
					BC_ASSERT_PTR_NULL(linphone_conference_info_get_description(conf_info_in_db));
					linphone_conference_info_unref(conf_info_in_db);
				}
			}

		}

		const char *newSubject = "Let's go drink a beer";
		linphone_conference_set_subject(conf, newSubject);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_subject_changed, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_subject_changed, 1, 5000));

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			if (confAddr) {
				LinphoneConferenceInfo * conf_info_in_db = linphone_core_find_conference_information_from_uri(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(conf_info_in_db);

				if(conf_info_in_db) {
					BC_ASSERT_TRUE(linphone_address_weak_equal(marie.getCMgr()->identity, linphone_conference_info_get_organizer(conf_info_in_db)));
					BC_ASSERT_TRUE(linphone_address_weak_equal(confAddr, linphone_conference_info_get_uri(conf_info_in_db)));

					const bctbx_list_t * ics_participants_db = linphone_conference_info_get_participants(conf_info_in_db);
					BC_ASSERT_EQUAL(bctbx_list_size(ics_participants_db), 3, size_t, "%zu");
					bctbx_list_free((bctbx_list_t *)ics_participants_db);

					BC_ASSERT_NOT_EQUAL((long long)linphone_conference_info_get_date_time(conf_info_in_db), 0, long long, "%lld");
					const int duration_s = linphone_conference_info_get_duration(conf_info_in_db) * 60;
					BC_ASSERT_EQUAL(duration_s, 0, int, "%d");
					BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_subject(conf_info_in_db), newSubject);
					BC_ASSERT_PTR_NULL(linphone_conference_info_get_description(conf_info_in_db));
					linphone_conference_info_unref(conf_info_in_db);
				}
			}

		}
		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		LinphoneAddress *focus_addr = linphone_address_new(linphone_core_get_identity(focus.getLc()));
		LinphoneConference * fconference = linphone_core_search_conference(focus.getLc(), NULL, focus_addr, confAddr, NULL);
		linphone_address_unref(focus_addr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			// Explicitely terminate conference as those on server are static by default
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
			linphone_address_unref(uri);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			check_conference_ssrc(fconference, pconference);
		}

		const int total_marie_calls = marie.getStats().number_of_LinphoneCallEnd + (int)bctbx_list_size(linphone_core_get_calls(marie.getLc()));
		const int total_focus_calls = focus.getStats().number_of_LinphoneCallEnd + (int)bctbx_list_size(linphone_core_get_calls(focus.getLc()));
		const int total_pauline_calls = pauline.getStats().number_of_LinphoneCallEnd + (int)bctbx_list_size(linphone_core_get_calls(pauline.getLc()));
		const int total_laure_calls = laure.getStats().number_of_LinphoneCallEnd + (int)bctbx_list_size(linphone_core_get_calls(laure.getLc()));

		linphone_core_terminate_all_calls(pauline.getLc());
		linphone_core_terminate_all_calls(laure.getLc());
		linphone_core_terminate_all_calls(marie.getLc());

		// Wait for calls to be terminated
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallEnd, total_marie_calls, 30000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallEnd, total_pauline_calls, 30000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallEnd, total_laure_calls, 30000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd, total_focus_calls, 30000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallReleased, total_marie_calls, 30000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallReleased, total_pauline_calls, 30000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallReleased, total_laure_calls, 30000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased, total_focus_calls, 30000));

		if (confAddr && fconference) {
			linphone_conference_terminate(fconference);
		}

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {

			// Wait for all conferences to be terminated
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1, 5000));

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated, (mgr == focus.getCMgr()) ? 3 : 1,10000));

			if (mgr && (mgr != focus.getCMgr())) {
				LinphoneCall * participant_call = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
				BC_ASSERT_PTR_NULL(participant_call);
				LinphoneCall * conference_call = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
				BC_ASSERT_PTR_NULL(conference_call);

				const bctbx_list_t* call_logs = linphone_core_get_call_logs(mgr->lc);
				BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(call_logs), ((mgr == marie.getCMgr()) ? 3 : 2), unsigned int, "%u");

				bctbx_list_t * mgr_focus_call_log = linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
				BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
				if (mgr_focus_call_log) {
					BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(mgr_focus_call_log), 1, unsigned int, "%u");
					for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
						LinphoneCallLog *call_log=(LinphoneCallLog *)it->data;
						BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
					}
					bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func) linphone_call_log_unref);
				}
			}
		}

		linphone_conference_unref(conf);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void create_simple_conference_merging_calls (void) {
	create_simple_conference_merging_calls_base(FALSE);
}

static void create_simple_ice_conference_merging_calls (void) {
	create_simple_conference_merging_calls_base(TRUE);
}

}

static test_t local_conference_chat_tests[] = {
	TEST_ONE_TAG("Group chat room creation local server", LinphoneTest::group_chat_room_creation_server,"LeaksMemory"), /* beacause of coreMgr restart*/
	TEST_NO_TAG("Group chat Server chat room deletion", LinphoneTest::group_chat_room_server_deletion),
	TEST_NO_TAG("Group chat Add participant with invalid address", LinphoneTest::group_chat_room_add_participant_with_invalid_address),
	TEST_NO_TAG("Group chat Only participant with invalid address", LinphoneTest::group_chat_room_with_only_participant_with_invalid_address),
	TEST_ONE_TAG("Group chat room bulk notify to participant", LinphoneTest::group_chat_room_bulk_notify_to_participant,"LeaksMemory"), /* because of network up and down*/
	TEST_ONE_TAG("One to one chatroom exhumed while participant is offline", LinphoneTest::one_to_one_chatroom_exhumed_while_offline,"LeaksMemory"), /* because of network up and down*/
	TEST_ONE_TAG("Group chat Server chat room deletion with remote list event handler", LinphoneTest::group_chat_room_server_deletion_with_rmt_lst_event_handler,"LeaksMemory"), /* because of coreMgr restart*/
	TEST_NO_TAG("Unencrypted group chat server chat room with admin managed ephemeral messages", LinphoneTest::group_chat_room_server_admin_managed_messages_unencrypted),
	TEST_ONE_TAG("Group chat Server chat room with admin managed ephemeral messages disabled after creation", LinphoneTest::group_chat_room_server_admin_managed_messages_ephemeral_disabled_after_creation, "LeaksMemory"), /* because of coreMgr restart*/
	TEST_ONE_TAG("Group chat Server chat room with admin managed ephemeral messages enabled after creation", LinphoneTest::group_chat_room_server_admin_managed_messages_ephemeral_enabled_after_creation, "LeaksMemory"), /* because of coreMgr restart*/
	TEST_ONE_TAG("Group chat Server chat room with admin managed ephemeral messages with lifetime update", LinphoneTest::group_chat_room_server_admin_managed_messages_ephemeral_lifetime_update, "LeaksMemory"), /* because of coreMgr restart*/
	TEST_NO_TAG("Group chat Server chat room with admin managed ephemeral messages with lifetime toggle", LinphoneTest::group_chat_room_server_admin_managed_messages_ephemeral_lifetime_toggle_using_different_methods),
	TEST_NO_TAG("Group chat Server chat room with ephemeral message mode changed", LinphoneTest::group_chat_room_server_ephemeral_mode_changed),
	TEST_ONE_TAG("Multi domain chatroom", LinphoneTest::multidomain_group_chat_room,"LeaksMemory") /* because of coreMgr restart*/
};

static test_t local_conference_conference_tests[] = {
	TEST_NO_TAG("Create simple conference", LinphoneTest::create_simple_conference),
	TEST_NO_TAG("Create simple conference with server restart", LinphoneTest::create_simple_conference_with_server_restart),
	TEST_NO_TAG("Create simple ICE conference", LinphoneTest::create_simple_ice_conference),
	TEST_NO_TAG("Create simple STUN+ICE conference", LinphoneTest::create_simple_stun_ice_conference),
	TEST_NO_TAG("Create simple SRTP conference", LinphoneTest::create_simple_srtp_conference),
	TEST_NO_TAG("Create simple ICE SRTP conference", LinphoneTest::create_simple_ice_srtp_conference),
	TEST_NO_TAG("Create simple STUN+ICE SRTP conference", LinphoneTest::create_simple_stun_ice_srtp_conference),
	TEST_NO_TAG("Create conference with uninvited participant", LinphoneTest::create_conference_with_uninvited_participant),
	TEST_NO_TAG("Create conference with uninvited participant not allowed", LinphoneTest::create_conference_with_uninvited_participant_not_allowed),
	TEST_NO_TAG("Create conference starting immediately", LinphoneTest::create_conference_starting_immediately),
	TEST_NO_TAG("Create conference starting in the past", LinphoneTest::create_conference_starting_in_the_past),
	TEST_NO_TAG("Create simple conference with audio only participant", LinphoneTest::create_simple_conference_with_audio_only_participant),
	TEST_NO_TAG("Create simple ICE conference with audio only participant", LinphoneTest::create_simple_ice_conference_with_audio_only_participant),
	TEST_NO_TAG("Create simple STUN+ICE conference with audio only participant", LinphoneTest::create_simple_stun_ice_conference_with_audio_only_participant),
	TEST_NO_TAG("Create simple STUN+ICE SRTP conference with audio only participant", LinphoneTest::create_simple_stun_ice_srtp_conference_with_audio_only_participant),
	TEST_NO_TAG("Create conference with audio only and uninvited participant", LinphoneTest::create_conference_with_audio_only_and_uninvited_participant),
	TEST_NO_TAG("Create simple conference with audio only participant enabling video", LinphoneTest::create_simple_conference_with_audio_only_participant_enabling_video),
#if 0
	TEST_NO_TAG("Conference with participants added after conference end", LinphoneTest::conference_with_participants_added_after_end),
	TEST_NO_TAG("Conference with participants added before conference start", LinphoneTest::conference_with_participants_added_before_start),
#endif
	TEST_NO_TAG("Organizer schedules 2 conferences", LinphoneTest::organizer_schedule_two_conferences),
	TEST_NO_TAG("2 overlapping conferences from difference organizers", LinphoneTest::two_overlapping_conferences_from_difference_organizers),
	TEST_NO_TAG("Create simple conference by merging calls", LinphoneTest::create_simple_conference_merging_calls),
	TEST_NO_TAG("Create simple ICE conference by merging calls", LinphoneTest::create_simple_ice_conference_merging_calls)
};

test_suite_t local_conference_test_suite_chat = {
	"Local conference tester (Chat)",
	NULL,
	NULL,
	liblinphone_tester_before_each,
	liblinphone_tester_after_each,
	sizeof(local_conference_chat_tests) / sizeof(local_conference_chat_tests[0]), local_conference_chat_tests
};

test_suite_t local_conference_test_suite_conference = {
	"Local conference tester (Conference)",
	NULL,
	NULL,
	liblinphone_tester_before_each,
	liblinphone_tester_after_each,
	sizeof(local_conference_conference_tests) / sizeof(local_conference_conference_tests[0]), local_conference_conference_tests
};

#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic pop
#endif
