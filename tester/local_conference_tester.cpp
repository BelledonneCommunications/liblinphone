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
	void reStart(bool check_for_proxies = TRUE) {
		linphone_core_manager_reinit(mMgr);
		mPreStart();
		linphone_core_manager_start(mMgr, check_for_proxies);
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

	CoreManagerAssert(std::list<std::reference_wrapper<CoreManager>> coreMgrs): BcAssert(){
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
		setup_mgr_for_conference(getCMgr(), NULL);
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

	void reStart(bool check_for_proxies = TRUE) {
		CoreManager::reStart(check_for_proxies);
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
		linphone_core_enable_rtp_bundle(getLc(), TRUE);

		LinphoneAccount *account = linphone_core_get_default_account(getLc());
		const LinphoneAccountParams* account_params = linphone_account_get_params(account);
		LinphoneAccountParams* new_account_params = linphone_account_params_clone(account_params);
		linphone_account_params_enable_rtp_bundle(new_account_params, TRUE);
		linphone_account_set_params(account, new_account_params);
		linphone_account_params_unref(new_account_params);
		BC_ASSERT_TRUE(linphone_account_params_rtp_bundle_enabled(linphone_account_get_params(account)));

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

	BC_ASSERT_TRUE(wait_for_list(coresList, &sender.getStats().number_of_LinphoneMessageDeliveredToUser, sender_stat.number_of_LinphoneMessageDeliveredToUser + noMsg, liblinphone_tester_sip_timeout));

	// Pauline marks the message as read, check that the state is now displayed on Marie's side
	linphone_chat_room_mark_as_read(recipientCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &sender.getStats().number_of_LinphoneMessageDisplayed, sender_stat.number_of_LinphoneMessageDisplayed + noMsg, liblinphone_tester_sip_timeout));

	BC_ASSERT_TRUE(wait_for_list(coresList, &sender.getStats().number_of_LinphoneChatRoomEphemeralTimerStarted, sender_stat.number_of_LinphoneChatRoomEphemeralTimerStarted + noMsg, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &recipient.getStats().number_of_LinphoneChatRoomEphemeralTimerStarted, recipient_stat.number_of_LinphoneChatRoomEphemeralTimerStarted + noMsg, liblinphone_tester_sip_timeout));

	BC_ASSERT_TRUE(wait_for_list(coresList, &sender.getStats().number_of_LinphoneMessageEphemeralTimerStarted, sender_stat.number_of_LinphoneMessageEphemeralTimerStarted + noMsg, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &recipient.getStats().number_of_LinphoneMessageEphemeralTimerStarted, recipient_stat.number_of_LinphoneMessageEphemeralTimerStarted + noMsg, liblinphone_tester_sip_timeout));

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
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_subject_changed, initialMarieStats.number_of_subject_changed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_subject_changed, initialLaureStats.number_of_subject_changed + 1, liblinphone_tester_sip_timeout));
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

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed, initialMarieStats.number_of_participants_removed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, liblinphone_tester_sip_timeout));

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
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateTerminated, initialLaureStats.number_of_LinphoneConferenceStateTerminated + 1, liblinphone_tester_sip_timeout));

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
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateCreated, paulineCrNo, liblinphone_tester_sip_timeout));
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

		BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getCMgr()->stat.number_of_NotifyReceived,pauline_stat.number_of_NotifyReceived + 1,liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).waitUntil(chrono::seconds(50),[&pauline] {
			for (auto chatRoom :pauline.getCore().getChatRooms()) {
				if (!chatRoom->ephemeralEnabled() || (chatRoom->getEphemeralLifetime() != 10)) {
					return false;
				}
			}
			return true;
		}));

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
		setup_mgr_for_conference(marie.getCMgr(), NULL);
		coresList = bctbx_list_append(coresList, marie.getLc());
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));

		// Retrieve chat room
		LinphoneAddress *marieDeviceAddr =  linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(marie.getLc())));
		marieCr = linphone_core_search_chat_room(marie.getLc(), NULL, marieDeviceAddr, confAddr, NULL);
		BC_ASSERT_PTR_NOT_NULL(marieCr);

		// Wait for chat rooms to be recovered from the main DB
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 1, liblinphone_tester_sip_timeout));

		if (marieCr) {
			BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 10, int, "%d");

			marie_stat=marie.getStats();
			pauline_stat=pauline.getStats();

			linphone_chat_room_set_ephemeral_lifetime(marieCr, 5);

			BC_ASSERT_TRUE(wait_for_list(coresList,&marie.getStats().number_of_LinphoneSubscriptionActive,marie_stat.number_of_LinphoneSubscriptionActive + 1,liblinphone_tester_sip_timeout));

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

		BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getCMgr()->stat.number_of_NotifyReceived,pauline_stat.number_of_NotifyReceived + 1,liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).waitUntil(chrono::seconds(50),[&pauline] {
			for (auto chatRoom :pauline.getCore().getChatRooms()) {
				if (chatRoom->ephemeralEnabled()) {
					return false;
				}
			}
			return true;
		}));

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

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDeliveredToUser, marie_stat.number_of_LinphoneMessageDeliveredToUser + noMsg, liblinphone_tester_sip_timeout));

		// Pauline marks the message as read, check that the state is now displayed on Marie's side
		linphone_chat_room_mark_as_read(paulineCr);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDisplayed, marie_stat.number_of_LinphoneMessageDisplayed + noMsg, liblinphone_tester_sip_timeout));

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
		setup_mgr_for_conference(marie.getCMgr(), NULL);
		coresList = bctbx_list_append(coresList, marie.getLc());
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));

		// Retrieve chat room
		LinphoneAddress *marieDeviceAddr =  linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(marie.getLc())));
		marieCr = linphone_core_search_chat_room(marie.getLc(), NULL, marieDeviceAddr, confAddr, NULL);
		BC_ASSERT_PTR_NOT_NULL(marieCr);

		// Wait for chat rooms to be recovered from the main DB
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 1, liblinphone_tester_sip_timeout));

		if (marieCr) {
			BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(marieCr));

			marie_stat=marie.getStats();
			pauline_stat=pauline.getStats();

			linphone_chat_room_set_ephemeral_lifetime(marieCr, 5);

			BC_ASSERT_TRUE(wait_for_list(coresList,&marie.getStats().number_of_LinphoneSubscriptionActive,marie_stat.number_of_LinphoneSubscriptionActive + 1,liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getStats().number_of_LinphoneChatRoomEphemeralMessageEnabled,pauline_stat.number_of_LinphoneChatRoomEphemeralMessageEnabled + 1,liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getStats().number_of_LinphoneChatRoomEphemeralLifetimeChanged,pauline_stat.number_of_LinphoneChatRoomEphemeralLifetimeChanged + 1,liblinphone_tester_sip_timeout));

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
		setup_mgr_for_conference(marie.getCMgr(), NULL);
		coresList = bctbx_list_append(coresList, marie.getLc());
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));

		// Retrieve chat room
		LinphoneAddress *marieDeviceAddr =  linphone_address_clone(linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(marie.getLc())));
		marieCr = linphone_core_search_chat_room(marie.getLc(), NULL, marieDeviceAddr, confAddr, NULL);
		BC_ASSERT_PTR_NOT_NULL(marieCr);

		// Wait for chat rooms to be recovered from the main DB
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 1, liblinphone_tester_sip_timeout));

		if (marieCr) {
			BC_ASSERT_TRUE(linphone_chat_room_ephemeral_enabled(marieCr));
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
			BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_lifetime(marieCr), 5, int, "%d");

			marie_stat=marie.getStats();
			pauline_stat=pauline.getStats();
			linphone_chat_room_set_ephemeral_lifetime(marieCr, 10);

			BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getStats().number_of_LinphoneChatRoomEphemeralLifetimeChanged,pauline_stat.number_of_LinphoneChatRoomEphemeralLifetimeChanged + 1,liblinphone_tester_sip_timeout));

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

		BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getStats().number_of_LinphoneChatRoomEphemeralLifetimeChanged,pauline_stat.number_of_LinphoneChatRoomEphemeralLifetimeChanged + 1,liblinphone_tester_sip_timeout));

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

		BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getStats().number_of_LinphoneChatRoomEphemeralMessageDisabled,pauline_stat.number_of_LinphoneChatRoomEphemeralMessageDisabled + 1,liblinphone_tester_sip_timeout));

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

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDeliveredToUser, marie_stat.number_of_LinphoneMessageDeliveredToUser + 1, liblinphone_tester_sip_timeout));

		// Pauline marks the message as read, check that the state is now displayed on Marie's side
		linphone_chat_room_mark_as_read(paulineCr);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDisplayed, marie_stat.number_of_LinphoneMessageDisplayed + 1, liblinphone_tester_sip_timeout));


		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		bctbx_list_free_with_data(paulineHistory, (bctbx_list_free_func)linphone_chat_message_unref);
		bctbx_list_free_with_data(marieHistory, (bctbx_list_free_func)linphone_chat_message_unref);

		pauline_stat=pauline.getStats();
		linphone_chat_room_enable_ephemeral(marieCr, TRUE);

		BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getStats().number_of_LinphoneChatRoomEphemeralMessageEnabled,pauline_stat.number_of_LinphoneChatRoomEphemeralMessageEnabled + 1,liblinphone_tester_sip_timeout));

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

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateCreated, initialPaulineStats.number_of_LinphoneConferenceStateCreated + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateCreated, initialMichelleStats.number_of_LinphoneConferenceStateCreated + 1, liblinphone_tester_sip_timeout));

		// Marie now changes the subject
		const char *newSubject = "Let's go drink a beer";
		linphone_chat_room_set_subject(marieCr, newSubject);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_subject_changed, initialMarieStats.number_of_subject_changed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_subject_changed, initialMichelleStats.number_of_subject_changed + 1, liblinphone_tester_sip_timeout));
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

		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_LinphoneConferenceStateCreationPending, initialLaureStats.number_of_LinphoneConferenceStateCreationPending + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_LinphoneConferenceStateCreated, initialLaureStats.number_of_LinphoneConferenceStateCreated + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_LinphoneChatRoomConferenceJoined, initialLaureStats.number_of_LinphoneChatRoomConferenceJoined + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_participants_added, initialMarieStats.number_of_participants_added + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participants_added, initialPaulineStats.number_of_participants_added + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getCMgr()->stat.number_of_participants_added, initialMichelleStats.number_of_participants_added + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_participant_devices_added, initialMarieStats.number_of_participant_devices_added + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participant_devices_added, initialPaulineStats.number_of_participant_devices_added + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getCMgr()->stat.number_of_participant_devices_added, initialMichelleStats.number_of_participant_devices_added + 1, liblinphone_tester_sip_timeout));
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

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_subject_changed, initialMarieStats.number_of_subject_changed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline.getStats().number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_subject_changed, initialLaureStats.number_of_subject_changed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_subject_changed, initialMichelleStats.number_of_subject_changed + 1, liblinphone_tester_sip_timeout));
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

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_participant_devices_removed, initialMarieStats.number_of_participant_devices_removed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participant_devices_removed, initialPaulineStats.number_of_participant_devices_removed + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getCMgr()->stat.number_of_participant_devices_removed, initialMichelleStats.number_of_participant_devices_removed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_participants_removed, initialMarieStats.number_of_participants_removed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getCMgr()->stat.number_of_participants_removed, initialMichelleStats.number_of_participants_removed + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 2, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 2, int, "%d");

		// Wait a little bit to detect side effects
		CoreManagerAssert({focus,marie,pauline,michelle}).waitUntil(std::chrono::seconds(2),[ ]{return false;});

		initialPaulineStats = pauline.getStats();
		// Pauline comes up online
		linphone_core_set_network_reachable(pauline.getLc(), TRUE);

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_LinphoneRegistrationOk, initialPaulineStats.number_of_LinphoneRegistrationOk + 1, liblinphone_tester_sip_timeout));

		// Check that Pauline receives the backlog of events occurred while she was offline
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participants_added, initialPaulineStats.number_of_participants_added + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participant_devices_added, initialPaulineStats.number_of_participant_devices_added + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participant_devices_removed, initialPaulineStats.number_of_participant_devices_removed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, liblinphone_tester_sip_timeout));

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

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateCreated, initialPaulineStats.number_of_LinphoneConferenceStateCreated + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateCreated, initialMichelleStats.number_of_LinphoneConferenceStateCreated + 1, liblinphone_tester_sip_timeout));

		// Marie now changes the subject
		const char *newSubject = "Let's go drink a beer";
		linphone_chat_room_set_subject(marieCr, newSubject);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_subject_changed, initialMarieStats.number_of_subject_changed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_subject_changed, initialMichelleStats.number_of_subject_changed + 1, liblinphone_tester_sip_timeout));
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

		BC_ASSERT_FALSE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_participants_added, initialMarieStats.number_of_participants_added + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_participant_devices_added, initialMarieStats.number_of_participant_devices_added + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_participant_devices_joined, initialMarieStats.number_of_participant_devices_joined + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participants_added, initialPaulineStats.number_of_participants_added + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participant_devices_added, initialPaulineStats.number_of_participant_devices_added + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participant_devices_joined, initialPaulineStats.number_of_participant_devices_joined + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &michelle.getCMgr()->stat.number_of_participants_added, initialMichelleStats.number_of_participants_added + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &michelle.getCMgr()->stat.number_of_participant_devices_added, initialMichelleStats.number_of_participant_devices_added + 1, 5000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &michelle.getCMgr()->stat.number_of_participant_devices_joined, initialMichelleStats.number_of_participant_devices_joined + 1, 5000));

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
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_LinphoneConferenceStateCreationFailed, initialMarieStats.number_of_LinphoneConferenceStateCreationFailed + 1, liblinphone_tester_sip_timeout));

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

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateCreated, initialPaulineStats.number_of_LinphoneConferenceStateCreated + 1, liblinphone_tester_sip_timeout));

		// Pauline goes offline
		linphone_core_set_network_reachable(pauline.getLc(), FALSE);

		LinphoneChatMessage *marieMsg1 = linphone_chat_room_create_message_from_utf8(marieCr, "Long live the C++ !");
		linphone_chat_message_send(marieMsg1);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageSent, initialMarieStats.number_of_LinphoneMessageSent + 1, liblinphone_tester_sip_timeout));
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
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminated, initialMarieStats.number_of_LinphoneConferenceStateTerminated + 1, liblinphone_tester_sip_timeout));

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

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_LinphoneRegistrationOk, initialPaulineStats.number_of_LinphoneRegistrationOk + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneChatRoomConferenceJoined, initialPaulineStats.number_of_LinphoneChatRoomConferenceJoined + 1, liblinphone_tester_sip_timeout));
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

			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, liblinphone_tester_sip_timeout));
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
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminated, initialMarieStats.number_of_LinphoneConferenceStateTerminated + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateTerminated, initialPaulineStats.number_of_LinphoneConferenceStateTerminated + 1, liblinphone_tester_sip_timeout));

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
		setup_mgr_for_conference(marie.getCMgr(), NULL);
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

		BC_ASSERT_TRUE(wait_for_list(coresList,&marie.getCMgr()->stat.number_of_LinphoneSubscriptionActive,2,liblinphone_tester_sip_timeout));

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

		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_LinphoneConferenceStateCreationPending, initialLaureStats.number_of_LinphoneConferenceStateCreationPending + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_LinphoneConferenceStateCreated, initialLaureStats.number_of_LinphoneConferenceStateCreated + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_LinphoneChatRoomConferenceJoined, initialLaureStats.number_of_LinphoneChatRoomConferenceJoined + 1, liblinphone_tester_sip_timeout));

		LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure.getCMgr(), &initialLaureStats, confAddr, initialSubject, 3, FALSE);
		BC_ASSERT_PTR_NOT_NULL(laureCr);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_participants_added, initialMarieStats.number_of_participants_added + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participants_added, initialPaulineStats.number_of_participants_added + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getCMgr()->stat.number_of_participants_added, initialMichelleStats.number_of_participants_added + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 3, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 3, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(michelleCr), 3, int, "%d");

		BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getCMgr()->stat.number_of_NotifyReceived,initialPaulineStats.number_of_NotifyReceived + 1,liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList,&michelle.getCMgr()->stat.number_of_NotifyReceived,initialMichelleStats.number_of_NotifyReceived + 1,liblinphone_tester_sip_timeout));

		focusAuth1DotExampleDotOrg.registerAsParticipantDevice(laure);
		_configure_core_for_conference(laure.getCMgr(),L_GET_C_BACK_PTR(&focusAuth1DotExampleDotOrgFactoryAddress));

		participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(L_GET_C_BACK_PTR(&laureAddr)));
		linphone_chat_room_add_participants(marieCrfocusAuth1DotExampleDotOrg, participantsAddresses);
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_LinphoneConferenceStateCreationPending, initialLaureStats.number_of_LinphoneConferenceStateCreationPending + 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_LinphoneConferenceStateCreated, initialLaureStats.number_of_LinphoneConferenceStateCreated + 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_LinphoneChatRoomConferenceJoined, initialLaureStats.number_of_LinphoneChatRoomConferenceJoined + 2, liblinphone_tester_sip_timeout));

		LinphoneChatRoom *laureCrfocusAuth1DotExampleDotOrg = check_creation_chat_room_client_side(coresList, laure.getCMgr(), &initialLaureStats, confAddrfocusAuth1DotExampleDotOrg, initialSubject, 3, FALSE);
		BC_ASSERT_PTR_NOT_NULL(laureCrfocusAuth1DotExampleDotOrg);

		BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getCMgr()->stat.number_of_NotifyReceived,initialPaulineStats.number_of_NotifyReceived + 2,liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList,&michelle.getCMgr()->stat.number_of_NotifyReceived,initialMichelleStats.number_of_NotifyReceived + 2,liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_participants_added, initialMarieStats.number_of_participants_added + 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_participants_added, initialPaulineStats.number_of_participants_added + 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getCMgr()->stat.number_of_participants_added, initialMichelleStats.number_of_participants_added + 2, liblinphone_tester_sip_timeout));

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

		BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getCMgr()->stat.number_of_NotifyReceived,pauline_stat.number_of_NotifyReceived + 1,liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline}).waitUntil(chrono::seconds(50),[&pauline] {
			for (auto chatRoom :pauline.getCore().getChatRooms()) {
				if (!chatRoom->ephemeralEnabled() || (chatRoom->getEphemeralLifetime() != 10)) {
					return false;
				}
			}
			return true;
		}));

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

		BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getCMgr()->stat.number_of_NotifyReceived,pauline_stat.number_of_NotifyReceived + 1,liblinphone_tester_sip_timeout));

		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(marieCr), adminMode, int, "%d");
		BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(marieCr));
		BC_ASSERT_EQUAL(linphone_chat_room_get_ephemeral_mode(paulineCr), adminMode, int, "%d");
		BC_ASSERT_FALSE(linphone_chat_room_ephemeral_enabled(paulineCr));

		pauline_stat=pauline.getStats();
		const LinphoneChatRoomEphemeralMode deviceMode = LinphoneChatRoomEphemeralModeDeviceManaged;
		linphone_chat_room_set_ephemeral_mode(marieCr, deviceMode);

		BC_ASSERT_TRUE(wait_for_list(coresList,&pauline.getCMgr()->stat.number_of_NotifyReceived,pauline_stat.number_of_NotifyReceived + 1,liblinphone_tester_sip_timeout));

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

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDeliveredToUser, marie_stat.number_of_LinphoneMessageDeliveredToUser + 1, liblinphone_tester_sip_timeout));

		// Pauline marks the message as read, check that the state is now displayed on Marie's side
		linphone_chat_room_mark_as_read(paulineCr);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDisplayed, marie_stat.number_of_LinphoneMessageDisplayed + 1, liblinphone_tester_sip_timeout));

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

static const int x3dhServer_creationTimeout = 15000;
static void group_chat_room_lime_server_message (bool encrypted) {
	Focus focus("chloe_rc");
	LinphoneChatMessage* msg;
	{//to make sure focus is destroyed after clients.
		linphone_core_enable_lime_x3dh(focus.getLc(), true);
		
		ClientConference marie(encrypted? "marie_lime_x3dh_rc" : "marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline(encrypted? "pauline_lime_x3dh_rc" : "pauline_rc", focus.getIdentity().asAddress());
		ClientConference laure(encrypted? "laure_lime_x3dh_rc" : "laure_tcp_rc", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);

		stats marie_stat=marie.getStats();
		stats pauline_stat=pauline.getStats();
		stats laure_stat=laure.getStats();
		bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, laure.getLc());

		if (encrypted) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_X3dhUserCreationSuccess, marie_stat.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_X3dhUserCreationSuccess, laure_stat.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_X3dhUserCreationSuccess, pauline_stat.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure.getLc()));
		}

		Address paulineAddr(pauline.getIdentity().asAddress());
		Address laureAddr(laure.getIdentity().asAddress());
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(L_GET_C_BACK_PTR(&paulineAddr)));
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(L_GET_C_BACK_PTR(&laureAddr)));

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues";
		LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie.getCMgr(), &marie_stat, participantsAddresses, initialSubject, encrypted, LinphoneChatRoomEphemeralModeDeviceManaged);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &pauline_stat, confAddr, initialSubject, 2, FALSE);
		BC_ASSERT_PTR_NOT_NULL(paulineCr);
		LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure.getCMgr(), &laure_stat, confAddr, initialSubject, 2, FALSE);
		BC_ASSERT_PTR_NOT_NULL(laureCr);
		if (paulineCr && laureCr) {
			// Marie sends the message
			const char *marieMessage = "Hey ! What's up ?";
			msg = _send_message_ephemeral(marieCr, marieMessage, FALSE);
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_LinphoneMessageReceived, pauline_stat.number_of_LinphoneMessageReceived + 1, liblinphone_tester_sip_timeout));
			LinphoneChatMessage *paulineLastMsg = pauline.getCMgr()->stat.last_received_chat_message;
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_LinphoneMessageReceived, laure_stat.number_of_LinphoneMessageReceived + 1, liblinphone_tester_sip_timeout));
			LinphoneChatMessage *laureLastMsg = laure.getCMgr()->stat.last_received_chat_message;
			linphone_chat_message_unref(msg);
			if (paulineLastMsg && laureLastMsg) {
				// Check that the message was correctly decrypted if encrypted
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieMessage);
				LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie.getLc()));
				BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieMessage);
				BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(laureLastMsg)));
				linphone_address_unref(marieAddr);
			}
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
		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline,laure}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure}).waitUntil(chrono::seconds(2),[] {
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

static void group_chat_room_lime_session_corrupted (void) {
	Focus focus("chloe_rc");
	LinphoneChatMessage* msg;
	{//to make sure focus is destroyed after clients.
		linphone_core_enable_lime_x3dh(focus.getLc(), true);
		
		ClientConference marie("marie_lime_x3dh_rc" , focus.getIdentity().asAddress());
		ClientConference pauline("pauline_lime_x3dh_rc", focus.getIdentity().asAddress());
		ClientConference laure("laure_lime_x3dh_rc", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(laure.getLc()));

		stats marie_stat=marie.getStats();
		stats pauline_stat=pauline.getStats();
		stats laure_stat=laure.getStats();
		bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, laure.getLc());

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_X3dhUserCreationSuccess, marie_stat.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_X3dhUserCreationSuccess, laure_stat.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_X3dhUserCreationSuccess, pauline_stat.number_of_X3dhUserCreationSuccess+1, x3dhServer_creationTimeout));

		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure.getLc()));

		Address paulineAddr(pauline.getIdentity().asAddress());
		Address laureAddr(laure.getIdentity().asAddress());
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(L_GET_C_BACK_PTR(&paulineAddr)));
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(L_GET_C_BACK_PTR(&laureAddr)));

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues";
		LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie.getCMgr(), &marie_stat, participantsAddresses, initialSubject, TRUE, LinphoneChatRoomEphemeralModeDeviceManaged);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &pauline_stat, confAddr, initialSubject, 2, FALSE);
		BC_ASSERT_PTR_NOT_NULL(paulineCr);
		LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure.getCMgr(), &laure_stat, confAddr, initialSubject, 2, FALSE);
		BC_ASSERT_PTR_NOT_NULL(laureCr);
		if (paulineCr && laureCr) {
			// Marie sends the message
			const char *marieMessage = "Hey ! What's up ?";
			msg = _send_message(marieCr, marieMessage);
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_LinphoneMessageReceived, pauline_stat.number_of_LinphoneMessageReceived + 1, 10000));
			LinphoneChatMessage *paulineLastMsg = pauline.getCMgr()->stat.last_received_chat_message;
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_LinphoneMessageReceived, laure_stat.number_of_LinphoneMessageReceived + 1, 10000));
			LinphoneChatMessage *laureLastMsg = laure.getCMgr()->stat.last_received_chat_message;
			linphone_chat_message_unref(msg);
			if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
				goto end;
			if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
				goto end;

			// Check that the message was correctly decrypted if encrypted
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieMessage);
			LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie.getLc()));
			BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieMessage);
			BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(laureLastMsg)));
			linphone_address_unref(marieAddr);
		
			
			// Corrupt Pauline sessions in lime database: WARNING: if SOCI is not found, this call does nothing and the test fails
			lime_delete_DRSessions(pauline.getCMgr()->lime_database_path);
			// Trick to force the reloading of the lime engine so the session in cache is cleared
			linphone_core_enable_lime_x3dh(pauline.getLc(), FALSE);
			linphone_core_enable_lime_x3dh(pauline.getLc(), TRUE);

			// Marie send a new message, it shall fail and get a 488 response
			const char *marieTextMessage2 = "Do you copy?";
			msg = _send_message(marieCr, marieTextMessage2);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_LinphoneMessageDelivered, marie_stat.number_of_LinphoneMessageDelivered + 2, 10000)); // Delivered to the server
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_LinphoneMessageReceived, laure_stat.number_of_LinphoneMessageReceived + 2, 10000)); // the message is correctly received by Laure
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_LinphoneMessageNotDelivered, marie_stat.number_of_LinphoneMessageNotDelivered + 1, 10000)); // Not delivered to pauline
			BC_ASSERT_EQUAL(pauline.getCMgr()->stat.number_of_LinphoneMessageReceived, 1, int, "%d");
			linphone_chat_message_unref(msg);
			laureLastMsg = laure.getCMgr()->stat.last_received_chat_message;
			if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
				goto end;
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieTextMessage2);
			marieAddr = linphone_address_new(linphone_core_get_identity(marie.getLc()));
			BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(laureLastMsg)));
			linphone_address_unref(marieAddr);
			
			// Try again, it shall work this time
			const char *marieTextMessage3 = "Hello again";
			marie_stat = marie.getCMgr()->stat;
			msg = _send_message(marieCr, marieTextMessage3);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_LinphoneMessageSent, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_LinphoneMessageDelivered, marie_stat.number_of_LinphoneMessageDelivered + 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getCMgr()->stat.number_of_LinphoneMessageReceived, pauline_stat.number_of_LinphoneMessageReceived + 2, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getCMgr()->stat.number_of_LinphoneMessageReceived, laure_stat.number_of_LinphoneMessageReceived + 3, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getCMgr()->stat.number_of_LinphoneMessageDeliveredToUser, marie_stat.number_of_LinphoneMessageDeliveredToUser + 1, 10000));
			paulineLastMsg = pauline.getCMgr()->stat.last_received_chat_message;
			if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
				goto end;
			laureLastMsg = laure.getCMgr()->stat.last_received_chat_message;
			if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
				goto end;
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieTextMessage3);
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieTextMessage3);
			marieAddr = linphone_address_new(linphone_core_get_identity(marie.getLc()));
			BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
			BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(laureLastMsg)));
			linphone_address_unref(marieAddr);
			linphone_chat_message_unref(msg);
		}

end:
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
		BC_ASSERT_TRUE(CoreManagerAssert({focus,marie,pauline,laure}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure}).waitUntil(chrono::seconds(2),[] {
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

static void group_chat_room_lime_server_encrypted_message (void) {
	group_chat_room_lime_server_message(TRUE);
}

static void group_chat_room_lime_server_clear_message (void) {
	group_chat_room_lime_server_message(FALSE);
}

static void conference_scheduler_state_changed(LinphoneConferenceScheduler *scheduler, LinphoneConferenceSchedulerState state) {
	stats *stat = get_stats(linphone_conference_scheduler_get_core(scheduler));
	if (state == LinphoneConferenceSchedulerStateReady) {
		stat->number_of_ConferenceSchedulerStateReady++;
	} else if (state == LinphoneConferenceSchedulerStateError) {
		stat->number_of_ConferenceSchedulerStateError++;
	} else if (state == LinphoneConferenceSchedulerStateUpdating) {
		stat->number_of_ConferenceSchedulerStateUpdating++;
	}
}

static void conference_scheduler_invitations_sent(LinphoneConferenceScheduler *scheduler, const bctbx_list_t *failed_addresses) {
	stats *stat = get_stats(linphone_conference_scheduler_get_core(scheduler));
	stat->number_of_ConferenceSchedulerInvitationsSent++;
}

static void check_conference_info(LinphoneCoreManager * mgr, LinphoneAddress *confAddr, LinphoneCoreManager * organizer, size_t no_members, long long start_time, int duration, const char * subject, const char *description) {
	LinphoneConferenceInfo * info = linphone_core_find_conference_information_from_uri(mgr->lc, confAddr);
	if (BC_ASSERT_PTR_NOT_NULL(info)) {
		BC_ASSERT_TRUE(linphone_address_weak_equal(organizer->identity, linphone_conference_info_get_organizer(info)));
		BC_ASSERT_TRUE(linphone_address_weak_equal(confAddr, linphone_conference_info_get_uri(info)));

		const bctbx_list_t * info_participants = linphone_conference_info_get_participants(info);
		BC_ASSERT_EQUAL(bctbx_list_size(info_participants), no_members, size_t, "%zu");
		bctbx_list_free((bctbx_list_t *)info_participants);

		if (start_time > 0) {
			BC_ASSERT_EQUAL((long long)linphone_conference_info_get_date_time(info), start_time, long long, "%lld");
		} else {
			BC_ASSERT_GREATER_STRICT((long long)linphone_conference_info_get_date_time(info), 0, long long, "%lld");
		}
		const int duration_m = linphone_conference_info_get_duration(info);
		BC_ASSERT_EQUAL(duration_m, ((duration < 0) ? 0 : duration), int, "%d");
		if (subject) {
			BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_subject(info), subject);
		} else {
			BC_ASSERT_PTR_NULL(linphone_conference_info_get_subject(info));
		}
		if (description) {
			BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_description(info), description);
		} else {
			BC_ASSERT_PTR_NULL(linphone_conference_info_get_description(info));
		}
		linphone_conference_info_unref(info);
	}
}

static LinphoneAddress * create_conference_on_server(Focus & focus, ClientConference & organizer, std::list<LinphoneCoreManager*> participants, time_t start_time, time_t end_time, const char * subject, const char * description, bool_t send_ics) {
	bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
	coresList = bctbx_list_append(coresList, organizer.getLc());
	std::vector<stats> participant_stats;
	for (auto & p : participants) {
		coresList = bctbx_list_append(coresList, p->lc);
		participant_stats.push_back(p->stat);
	}

	stats organizer_stat=organizer.getStats();
	stats focus_stat=focus.getStats();

	// Marie creates a new group chat room
	LinphoneConferenceScheduler *conference_scheduler = linphone_core_create_conference_scheduler(organizer.getLc());
	LinphoneConferenceSchedulerCbs *cbs = linphone_factory_create_conference_scheduler_cbs(linphone_factory_get());
	linphone_conference_scheduler_cbs_set_state_changed(cbs, conference_scheduler_state_changed);
	linphone_conference_scheduler_cbs_set_invitations_sent(cbs, conference_scheduler_invitations_sent);
	linphone_conference_scheduler_add_callbacks(conference_scheduler, cbs);
	linphone_conference_scheduler_cbs_unref(cbs);

	LinphoneConferenceInfo *conf_info = linphone_conference_info_new();

	LinphoneAccount* default_account = linphone_core_get_default_account(organizer.getLc());
	LinphoneAddress * organizer_address =  default_account ? linphone_address_clone(linphone_account_params_get_identity_address(linphone_account_get_params(default_account))) : linphone_address_new(linphone_core_get_identity(organizer.getLc()));
	linphone_conference_info_set_organizer(conf_info, organizer_address);
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

	BC_ASSERT_TRUE(wait_for_list(coresList, &organizer.getStats().number_of_ConferenceSchedulerStateReady, organizer_stat.number_of_ConferenceSchedulerStateReady + 1, liblinphone_tester_sip_timeout));

	const LinphoneConferenceInfo* updated_conf_info = linphone_conference_scheduler_get_info(conference_scheduler);
	LinphoneAddress * conference_address = linphone_address_clone(linphone_conference_info_get_uri(updated_conf_info));

	int idx = 0;
	bool dialout = ((end_time <= 0) && (start_time <= 0));

	if (dialout) {
		// Conference server dials out participants
		for (auto & mgr : participants) {
			auto old_stats = participant_stats[idx];
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallIncomingReceived, old_stats.number_of_LinphoneCallIncomingReceived + 1, liblinphone_tester_sip_timeout));
			idx++;
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &organizer.getStats().number_of_LinphoneCallOutgoingInit, organizer_stat.number_of_LinphoneCallOutgoingInit + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived, focus_stat.number_of_LinphoneCallIncomingReceived + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingProgress, focus_stat.number_of_LinphoneCallOutgoingProgress + static_cast<int>(participants.size()), liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingRinging, focus_stat.number_of_LinphoneCallOutgoingRinging + static_cast<int>(participants.size()), liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_alerting, focus_stat.number_of_participant_devices_alerting + static_cast<int>(participants.size()), liblinphone_tester_sip_timeout));
	}

	if (!!send_ics) {
		LinphoneChatRoomParams *chat_room_params = linphone_core_create_default_chat_room_params(organizer.getLc());
		linphone_chat_room_params_set_backend(chat_room_params, LinphoneChatRoomBackendBasic);
		linphone_conference_scheduler_send_invitations(conference_scheduler, chat_room_params);
		linphone_chat_room_params_unref(chat_room_params);
		BC_ASSERT_TRUE(wait_for_list(coresList, &organizer.getStats().number_of_ConferenceSchedulerInvitationsSent, organizer_stat.number_of_ConferenceSchedulerInvitationsSent + 1, liblinphone_tester_sip_timeout));
	}

	idx = 0;
	for (auto & mgr : participants) {
		auto old_stats = participant_stats[idx];
		if (!!send_ics) {
			// chat room in created state
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceived, old_stats.number_of_LinphoneMessageReceived + 1, liblinphone_tester_sip_timeout));
			if (!linphone_core_conference_ics_in_message_body_enabled(organizer.getLc())) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceivedWithFile, old_stats.number_of_LinphoneMessageReceivedWithFile + 1, liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_PTR_NOT_NULL(mgr->stat.last_received_chat_message);
			if (mgr->stat.last_received_chat_message != NULL) {
				const string expected = ContentType::Icalendar.getMediaType();
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(mgr->stat.last_received_chat_message), expected.c_str());
			}

			bctbx_list_t *chat_room_participants = bctbx_list_append(NULL, mgr->identity);
			LinphoneChatRoom *cr = linphone_core_search_chat_room(organizer.getLc(), NULL, organizer_address, NULL, chat_room_participants);
			bctbx_list_free(chat_room_participants);

			BC_ASSERT_PTR_NOT_NULL(cr);
			if (cr) {
				LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(cr);
				linphone_chat_room_unref(cr);
				BC_ASSERT_PTR_NOT_NULL(msg);

				if (msg) {
					const bctbx_list_t* original_contents = linphone_chat_message_get_contents(msg);
					BC_ASSERT_EQUAL((int)bctbx_list_size(original_contents), 1, int, "%d");
					LinphoneContent *original_content = (LinphoneContent *) bctbx_list_get_data(original_contents);
					BC_ASSERT_PTR_NOT_NULL(original_content);

					LinphoneConferenceInfo * conf_info_in_db = linphone_core_find_conference_information_from_uri(mgr->lc, conference_address);
					if(!BC_ASSERT_PTR_NOT_NULL(conf_info_in_db)) {
						return conference_address;
					}

					LinphoneConferenceInfo *conf_info_from_original_content = linphone_factory_create_conference_info_from_icalendar_content(linphone_factory_get(), original_content);
					if (BC_ASSERT_PTR_NOT_NULL(conf_info_from_original_content)) {
						BC_ASSERT_TRUE(linphone_address_weak_equal(organizer_address, linphone_conference_info_get_organizer(conf_info_from_original_content)));
						BC_ASSERT_TRUE(linphone_address_equal(linphone_conference_info_get_organizer(conf_info_in_db), linphone_conference_info_get_organizer(conf_info_from_original_content)));
						BC_ASSERT_TRUE(linphone_address_weak_equal(conference_address, linphone_conference_info_get_uri(conf_info_from_original_content)));
						BC_ASSERT_TRUE(linphone_address_equal(linphone_conference_info_get_uri(conf_info_in_db), linphone_conference_info_get_uri(conf_info_from_original_content)));

						const bctbx_list_t * ics_participants = linphone_conference_info_get_participants(conf_info_from_original_content);
						BC_ASSERT_EQUAL(bctbx_list_size(ics_participants), participants.size() + ((dialout) ? 1 : 0), size_t, "%zu");
						bctbx_list_free((bctbx_list_t *)ics_participants);

						const bctbx_list_t * ics_participants_db = linphone_conference_info_get_participants(conf_info_in_db);
						BC_ASSERT_EQUAL(bctbx_list_size(ics_participants_db), participants.size() + ((dialout) ? 1 : 0), size_t, "%zu");
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
				}
			}
		}

		if (!!send_ics || ((end_time <= 0) && (start_time <= 0))) {
			LinphoneConferenceInfo * conf_info_in_db = linphone_core_find_conference_information_from_uri(mgr->lc, conference_address);
			if(!BC_ASSERT_PTR_NOT_NULL(conf_info_in_db)) {
				return conference_address;
			}
			check_conference_info(mgr, conference_address, organizer.getCMgr(), participants.size() + ((dialout) ? 1 : 0), start_time, ((start_time > 0) && (end_time > 0)) ? (int)(end_time - start_time)/60 : 0, subject, (!!send_ics) ? description : NULL);
			if (conf_info_in_db) {
				linphone_conference_info_unref(conf_info_in_db);
			}
		}

		idx++;
	}

	if (!!send_ics || ((end_time <= 0) && (start_time <= 0))) {
		if (conference_address) {
			check_conference_info(organizer.getCMgr(), conference_address, organizer.getCMgr(), participants.size() + ((dialout) ? 1 : 0), start_time, (((start_time > 0) && (end_time > 0)) ? (int)(end_time - start_time)/60 : 0), subject, description);
		}
		linphone_address_unref(organizer_address);

		BC_ASSERT_EQUAL(organizer.getStats().number_of_LinphoneConferenceStateTerminationPending, organizer_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(organizer.getStats().number_of_LinphoneConferenceStateTerminated, organizer_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(organizer.getStats().number_of_LinphoneConferenceStateDeleted, organizer_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");
	}

	linphone_conference_scheduler_unref(conference_scheduler);
	bctbx_list_free(coresList);

	char * conference_address_str = (conference_address) ? linphone_address_as_string(conference_address) : ms_strdup("<unknown>");
	ms_message("%s is creating conference %s on server %s", linphone_core_get_identity(organizer.getLc()), conference_address_str, linphone_core_get_identity(focus.getLc()));
	ms_free(conference_address_str);

	return conference_address;
}

static void wait_for_conference_streams(std::initializer_list<std::reference_wrapper<CoreManager>> coreMgrs, std::list<LinphoneCoreManager *> conferenceMgrs, LinphoneCoreManager * focus, std::list<LinphoneCoreManager *> members, const LinphoneAddress *confAddr, bool_t enable_video) {
	for (auto mgr : conferenceMgrs) {
		//wait bit more to detect side effect if any
		BC_ASSERT_TRUE(CoreManagerAssert(coreMgrs).waitUntil(chrono::seconds(50),[mgr, &focus, &members, confAddr, enable_video] {

			size_t nb_audio_streams = 1;
			size_t nb_text_streams = 0;
			std::list<LinphoneCall *> calls;

			bool video_check = false;
			bool device_present = false;
			bool call_ok = false;
			bool_t * call_video_enabled = NULL;
			LinphoneMediaDirection * call_video_dir = NULL;

			if (mgr == focus) {
				int counter = 0;
				for (auto m : members) {
					LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(m->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					// Allocate memory
					call_video_enabled = (bool_t*)realloc(call_video_enabled, (counter+1)*sizeof(bool_t));
					call_video_dir = (LinphoneMediaDirection*)realloc(call_video_dir, (counter+1)*sizeof(LinphoneMediaDirection));
					// Append element
					call_video_enabled[counter] = FALSE;
					call_video_dir[counter] = LinphoneMediaDirectionInactive;
					if (pcall) {
						const LinphoneCallParams * call_params = linphone_call_get_params(pcall);
						call_video_enabled[counter] = linphone_call_params_video_enabled(call_params);
						call_video_dir[counter] = linphone_call_params_get_video_direction(call_params);
					}

					LinphoneCall * call = linphone_core_get_call_by_remote_address2(mgr->lc, m->identity);
					BC_ASSERT_PTR_NOT_NULL(call);
					if (call) {
						calls.push_back(call);
					} else {
						calls.push_back(nullptr);
					}
					// Increment counter
					counter++;
				}
			} else {
				LinphoneCall * call = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(call);
				// Allocate memory
				call_video_enabled = (bool_t*)realloc(call_video_enabled, sizeof(bool_t));
				call_video_dir = (LinphoneMediaDirection*)realloc(call_video_dir, sizeof(LinphoneMediaDirection));
				// Append element
				call_video_enabled[0] = FALSE;
				call_video_dir[0] = LinphoneMediaDirectionInactive;
				if (call) {
					const LinphoneCallParams * call_params = linphone_call_get_params(call);
					call_video_enabled[0] = linphone_call_params_video_enabled(call_params);
					call_video_dir[0] = linphone_call_params_get_video_direction(call_params);
					calls.push_back(call);
				} else {
					calls.push_back(nullptr);
				}
			}

			int counter = 0;
			call_ok = (calls.size() > 0);
			for (auto call : calls) {
				if (call) {
					size_t nb_video_streams = (enable_video && call_video_enabled[counter] && ((call_video_dir[counter] == LinphoneMediaDirectionRecvOnly) || (call_video_dir[counter] == LinphoneMediaDirectionSendRecv))) ? (static_cast<int>(members.size()) + ((call_video_dir[counter] == LinphoneMediaDirectionSendRecv) ? 1 : -1)) : 0;
					const SalMediaDescription * call_local_desc = _linphone_call_get_result_desc(call);
					call_ok &= ((call_local_desc->nbActiveStreamsOfType(SalAudio) == nb_audio_streams) &&
						(call_local_desc->nbActiveStreamsOfType(SalVideo) == nb_video_streams) &&
						(call_local_desc->nbActiveStreamsOfType(SalText) == nb_text_streams) &&
						(linphone_call_get_state(call) == LinphoneCallStateStreamsRunning));
				}
				// Increment counter
				counter++;
			}

			LinphoneConference * conference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(conference);
			if (conference) {
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
				video_check = (bctbx_list_size(devices) > 0);
				device_present = (bctbx_list_size(devices) > 0);
				for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					LinphoneParticipantDevice * d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					if (enable_video && ((mgr == focus) || call_video_enabled[0])) {
						// Video is available if the direction is sendrecv
						video_check &= (linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) == (linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) == LinphoneMediaDirectionSendRecv));
					} else {
						video_check &= !linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo);
					}
					device_present &= (linphone_participant_device_get_state(d) == LinphoneParticipantDeviceStatePresent);
				}

				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
			}
			ms_free(call_video_dir);
			ms_free(call_video_enabled);
			return video_check && device_present && call_ok;
		}));
	}
}

static void set_video_settings_in_conference(LinphoneCoreManager * focus, LinphoneCoreManager * participant, std::list<LinphoneCoreManager *> members, const LinphoneAddress *confAddr, bool_t enable_video, LinphoneMediaDirection video_direction, bool_t answer_enable_video, LinphoneMediaDirection answer_video_direction) {
	std::list<LinphoneCoreManager *> active_cores = members;
	active_cores.push_back(focus);

	bctbx_list_t * coresList = bctbx_list_append(NULL, focus->lc);
	stats * participants_initial_stats = NULL;
	stats focus_stat=focus->stat;
	int counter = 1;

	for (auto mgr : members) {
		coresList = bctbx_list_append(coresList, mgr->lc);
		// Allocate memory
		participants_initial_stats = (stats*)realloc(participants_initial_stats, counter * sizeof(stats));
		// Append element
		participants_initial_stats[counter - 1] = mgr->stat;

		// Increment counter
		counter++;
	}

	bool_t previous_enable_video = FALSE;
	LinphoneMediaDirection previous_video_direction = LinphoneMediaDirectionInvalid;

	LinphoneCall * participant_call = linphone_core_get_call_by_remote_address2(participant->lc, confAddr);
	BC_ASSERT_PTR_NOT_NULL(participant_call);
	if (participant_call) {

		const LinphoneCallParams * call_params = linphone_call_get_current_params(participant_call);
		previous_enable_video = linphone_call_params_video_enabled(call_params);
		previous_video_direction = linphone_call_params_get_video_direction(call_params);

		ms_message("%s %s video with direction %s", linphone_core_get_identity(participant->lc), ((enable_video) ? "enables" : "disables"), linphone_media_direction_to_string(video_direction));

		LinphoneCallParams * new_params = linphone_core_create_call_params(participant->lc, participant_call);
		linphone_call_params_enable_video (new_params, enable_video);
		linphone_call_params_set_video_direction (new_params, video_direction);
		linphone_call_update(participant_call, new_params);
		linphone_call_params_unref (new_params);
	}

	BC_ASSERT_TRUE(wait_for_list(coresList, &focus->stat.number_of_LinphoneCallUpdatedByRemote, focus_stat.number_of_LinphoneCallUpdatedByRemote + 1, liblinphone_tester_sip_timeout));

	int focus_defer_update = !!linphone_config_get_int(linphone_core_get_config(focus->lc), "sip", "defer_update_default", FALSE);
	bool_t enable = enable_video && ((focus_defer_update == TRUE) ? answer_enable_video : TRUE);
	if (focus_defer_update == TRUE) {
		LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(participant->lc));
		LinphoneCall * focus_call = linphone_core_get_call_by_remote_address2(focus->lc, uri);
		linphone_address_unref(uri);
		BC_ASSERT_PTR_NOT_NULL(focus_call);
		if (focus_call) {
			LinphoneCallParams * focus_params = linphone_core_create_call_params(focus->lc, focus_call);
			linphone_call_params_enable_video(focus_params, answer_enable_video);
			linphone_call_params_set_video_direction (focus_params, answer_video_direction);
			linphone_call_accept_update(focus_call, focus_params);
			linphone_call_params_unref(focus_params);
		}
	}

	counter = 0;
	int no_updates = 0;
	for (auto mgr : members) {
		if (((previous_enable_video != enable) && (previous_video_direction == LinphoneMediaDirectionSendRecv)) || (enable && previous_enable_video && (previous_video_direction != video_direction)) || (mgr == participant)) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating, participants_initial_stats[counter].number_of_LinphoneCallUpdating + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, participants_initial_stats[counter].number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));
			if ((previous_enable_video == enable) && (!enable)) {
				BC_ASSERT_FALSE(wait_for_list(coresList, &mgr->stat.number_of_participant_devices_media_capability_changed, participants_initial_stats[counter].number_of_participant_devices_media_capability_changed + 1, 10000));
			} else {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_participant_devices_media_capability_changed, participants_initial_stats[counter].number_of_participant_devices_media_capability_changed + 1, liblinphone_tester_sip_timeout));
			}
			BC_ASSERT_EQUAL(mgr->stat.number_of_participants_added, participants_initial_stats[counter].number_of_participants_added, int, "%0d");
			BC_ASSERT_EQUAL(mgr->stat.number_of_participant_devices_added, participants_initial_stats[counter].number_of_participant_devices_added, int, "%0d");
			BC_ASSERT_EQUAL(mgr->stat.number_of_participant_devices_joined, participants_initial_stats[counter].number_of_participant_devices_joined, int, "%0d");
			no_updates++;
		}
		counter++;
	}
	BC_ASSERT_TRUE(wait_for_list(coresList, &focus->stat.number_of_LinphoneCallUpdatedByRemote, focus_stat.number_of_LinphoneCallUpdatedByRemote + no_updates, liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &focus->stat.number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + no_updates, liblinphone_tester_sip_timeout));
	if ((previous_enable_video == enable) && (!enable)) {
		BC_ASSERT_FALSE(wait_for_list(coresList, &focus->stat.number_of_participant_devices_media_capability_changed, focus_stat.number_of_participant_devices_media_capability_changed + 1, 10000));
	} else {
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus->stat.number_of_participant_devices_media_capability_changed, focus_stat.number_of_participant_devices_media_capability_changed + 1, liblinphone_tester_sip_timeout));
	}
	BC_ASSERT_EQUAL(focus->stat.number_of_participants_added, focus_stat.number_of_participants_added, int, "%0d");
	BC_ASSERT_EQUAL(focus->stat.number_of_participant_devices_added, focus_stat.number_of_participant_devices_added, int, "%0d");
	BC_ASSERT_EQUAL(focus->stat.number_of_participant_devices_joined, focus_stat.number_of_participant_devices_joined, int, "%0d");

	for (auto mgr : active_cores) {
		LinphoneAddress *uri2 = linphone_address_new(linphone_core_get_identity(mgr->lc));
		LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri2, confAddr, NULL);
		linphone_address_unref(uri2);
		BC_ASSERT_PTR_NOT_NULL(pconference);
		if (pconference) {
			LinphoneParticipant * p = (mgr == participant) ? linphone_conference_get_me(pconference) : linphone_conference_find_participant(pconference, participant->identity);
			BC_ASSERT_PTR_NOT_NULL(p);
			if (p) {
				bctbx_list_t *devices = linphone_participant_get_devices (p);
				for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					LinphoneParticipantDevice * d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					if (enable == TRUE) {
						BC_ASSERT_TRUE(linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) == video_direction);
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

	LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(participant->lc, confAddr);
	BC_ASSERT_PTR_NOT_NULL(pcall);
	if (pcall) {
		const LinphoneCallParams* call_lparams = linphone_call_get_params(pcall);
		BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enable_video, int, "%0d");
		const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(pcall);
		BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), (focus_defer_update == TRUE) ? enable : enable_video, int, "%0d");
		const LinphoneCallParams* call_cparams = linphone_call_get_current_params(pcall);
		BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enable, int, "%0d");
	}
	LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(participant->lc));
	LinphoneCall * ccall = linphone_core_get_call_by_remote_address2(focus->lc, uri);
	linphone_address_unref(uri);
	BC_ASSERT_PTR_NOT_NULL(ccall);
	if (ccall) {
		const LinphoneCallParams* call_lparams = linphone_call_get_params(ccall);
		BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), (focus_defer_update == TRUE) ? answer_enable_video : enable_video, int, "%0d");
		const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(ccall);
		BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enable_video, int, "%0d");
		const LinphoneCallParams* call_cparams = linphone_call_get_current_params(ccall);
		BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enable, int, "%0d");
	}
	bctbx_list_free(coresList);
	ms_free(participants_initial_stats);
}

static void call_to_inexisting_conference_address(void) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());

		bctbx_list_t * coresList = NULL;
		coresList = bctbx_list_append(coresList, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());

		focus.registerAsParticipantDevice(marie);

		LinphoneAddress * confAddr = linphone_address_new(L_STRING_TO_C(focus.getIdentity().asString()));
		linphone_address_set_uri_param(confAddr, "conf-id", "xxxxx");

		stats marie_stat=marie.getStats();

		LinphoneCallParams *new_params = linphone_core_create_call_params(marie.getLc(), nullptr);
		linphone_core_invite_address_with_params_2(marie.getLc(), confAddr, new_params, NULL, nullptr);
		linphone_call_params_unref(new_params);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallOutgoingInit, marie_stat.number_of_LinphoneCallOutgoingInit + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallError, marie_stat.number_of_LinphoneCallError + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallReleased, marie_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));

		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void create_conference_base (time_t start_time, int duration, bool_t uninvited_participant_dials, LinphoneConferenceParticipantListType participant_list_type, bool_t remove_participant, const LinphoneMediaEncryption encryption, bool_t enable_video, LinphoneConferenceLayout layout, bool_t enable_ice, bool_t enable_stun, bool_t audio_only_participant, bool_t server_restart, bool_t client_restart, bool_t do_not_use_proxy, LinphoneMediaDirection video_direction) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focus.getIdentity().asAddress());
		ClientConference laure("laure_tcp_rc", focus.getIdentity().asAddress());
		ClientConference michelle("michelle_rc", focus.getIdentity().asAddress());
		ClientConference lise("lise_rc", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(lise);

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t * coresList = NULL;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(), lise.getCMgr()}) {
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

				if (layout == LinphoneConferenceLayoutGrid) {
					linphone_core_set_preferred_video_definition_by_name(mgr->lc, "720p");
					linphone_config_set_string(linphone_core_get_config(mgr->lc), "video", "max_mosaic_size", "vga");
				}
			}

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, layout);
				linphone_core_set_media_encryption(mgr->lc,encryption);
			}

			if (do_not_use_proxy) {
				linphone_core_set_default_proxy_config(mgr->lc,NULL);
			}

			enable_stun_in_core(mgr, enable_stun, enable_ice);
			linphone_core_manager_wait_for_stun_resolution(mgr);

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), participant_list_type);

		stats focus_stat=focus.getStats();

		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr()};
		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()};

		time_t end_time = (duration <= 0) ? -1 : (start_time + duration * 60);
		const char *initialSubject = "Test characters: ^ :) ¤ çà @";
		const char *description = "Paris Baker";

		LinphoneAddress * confAddr = create_conference_on_server(focus, marie, participants, start_time, end_time, initialSubject, description, TRUE);
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

		for (auto mgr : members) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			// Multistream ZRTP is not supported yet
			if (encryption == LinphoneMediaEncryptionZRTP) {
				linphone_call_params_set_media_encryption (new_params, LinphoneMediaEncryptionSRTP);
			} else {
				linphone_call_params_set_media_encryption (new_params, encryption);
			}
			linphone_call_params_set_video_direction (new_params, video_direction);
			if (mgr == laure.getCMgr()) {
				linphone_call_params_enable_mic(new_params, FALSE);
			}
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
		}

		for (auto mgr : members) {
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
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined, focus_stat.number_of_participant_devices_joined + 3, 10000));

		wait_for_conference_streams({focus,marie,pauline,laure,michelle,lise}, conferenceMgrs, focus.getCMgr(), members, confAddr, enable_video);

		LinphoneConference * fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure,michelle,lise}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		for (auto mgr : conferenceMgrs) {
			LinphoneConference * pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				const LinphoneConferenceParams * conference_params = linphone_conference_get_current_params(pconference);
				int no_participants = 0;
				if (start_time >= 0) {
					BC_ASSERT_EQUAL((long long)linphone_conference_params_get_start_time(conference_params), (long long)start_time, long long, "%lld");
				}
				BC_ASSERT_EQUAL((long long)linphone_conference_params_get_end_time(conference_params), (long long)end_time, long long, "%lld");

				bctbx_list_t * participant_device_list = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(participant_device_list), 3, size_t, "%zu");
				for (bctbx_list_t *d_it = participant_device_list; d_it; d_it = bctbx_list_next(d_it)) {
					LinphoneParticipantDevice * d = (LinphoneParticipantDevice *)bctbx_list_get_data(d_it);
					BC_ASSERT_PTR_NOT_NULL(d);
					if (d) {
						BC_ASSERT_TRUE((!!linphone_participant_device_get_is_muted(d)) == (linphone_address_weak_equal(linphone_participant_device_get_address(d), laure.getCMgr()->identity)));
						linphone_participant_device_set_user_data(d, mgr->lc);
						LinphoneParticipantDeviceCbs *cbs = linphone_factory_create_participant_device_cbs(linphone_factory_get());
						linphone_participant_device_cbs_set_is_muted(cbs, on_muted_notified);
						linphone_participant_device_add_callbacks(d, cbs);
						linphone_participant_device_cbs_unref(cbs);
					}
				}
				bctbx_list_free_with_data(participant_device_list, (void(*)(void *))linphone_participant_device_unref);

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
						if ((video_direction == LinphoneMediaDirectionSendRecv) || !enable_video) {
							BC_ASSERT_TRUE(check_ice(mgr, focus.getCMgr(), LinphoneIceStateHostConnection));
						} else {
							LinphoneCall *c1,*c2;
							MSTimeSpec ts;

							c1=linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
							c2=linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
							BC_ASSERT_TRUE(linphone_call_get_microphone_muted(c1) == (mgr == laure.getCMgr()));

							BC_ASSERT_PTR_NOT_NULL(c1);
							BC_ASSERT_PTR_NOT_NULL(c2);
							if (!c1 || !c2) {
								BC_ASSERT_EQUAL(linphone_call_params_video_enabled(linphone_call_get_current_params(c1)),linphone_call_params_video_enabled(linphone_call_get_current_params(c2)), int, "%d");
								BC_ASSERT_EQUAL(linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(c1)),linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(c2)), int, "%d");
								bool_t audio_enabled=linphone_call_params_audio_enabled(linphone_call_get_current_params(c1));
								if (audio_enabled) {
									liblinphone_tester_clock_start(&ts);
									LinphoneCallStats *stats1 = NULL;
									LinphoneCallStats *stats2 = NULL;
									do {
										if ((c1 != NULL) && (c2 != NULL)) {
											stats1 = linphone_call_get_audio_stats(c1);
											stats2 = linphone_call_get_audio_stats(c2);
											if (linphone_call_stats_get_ice_state(stats1)==LinphoneIceStateHostConnection &&
												linphone_call_stats_get_ice_state(stats2)==LinphoneIceStateHostConnection){
												break;
											}
											linphone_core_iterate(mgr->lc);
											linphone_core_iterate(focus.getLc());
											linphone_call_stats_unref(stats1);
											linphone_call_stats_unref(stats2);
											stats1 = stats2 = NULL;
										}
										ms_usleep(20000);
									} while (!liblinphone_tester_clock_elapsed(&ts,10000));
									if (stats1)
										linphone_call_stats_unref(stats1);
									if (stats2)
										linphone_call_stats_unref(stats2);
								}
							}
						}
					}

					LinphoneVideoActivationPolicy * pol = linphone_core_get_video_activation_policy(mgr->lc);
					bool_t enabled = !!linphone_video_activation_policy_get_automatically_initiate(pol);
					linphone_video_activation_policy_unref(pol);

					int no_streams_audio = 1;
					int no_streams_video = (enabled) ? 4 : 0;
					int no_streams_text = 0;

					LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						_linphone_call_check_nb_streams(pcall, no_streams_audio, no_streams_video, no_streams_text);
						const LinphoneCallParams* call_lparams = linphone_call_get_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
						const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
						const LinphoneCallParams* call_cparams = linphone_call_get_current_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");

						if (enabled && layout == LinphoneConferenceLayoutGrid && video_direction == LinphoneMediaDirectionSendRecv) {
							MSVideoSize vsize = linphone_call_params_get_sent_video_size(call_cparams);
							BC_ASSERT_EQUAL(vsize.width, 640, int, "%d");
							BC_ASSERT_EQUAL(vsize.height, 480, int, "%d");
						}
					}
					LinphoneCall * ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
					BC_ASSERT_PTR_NOT_NULL(ccall);
					if (ccall) {
						_linphone_call_check_nb_streams(ccall, no_streams_audio, no_streams_video, no_streams_text);
						const LinphoneCallParams* call_lparams = linphone_call_get_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
						const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
						const LinphoneCallParams* call_cparams = linphone_call_get_current_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
					}
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

		ms_message("Marie mutes its microphone");

		LinphoneConference * marie_conference = linphone_core_search_conference_2(marie.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(marie_conference);
		if (marie_conference) {
			linphone_conference_mute_microphone(marie_conference, TRUE);
		}

		for (auto mgr : conferenceMgrs) {
			if (mgr != marie.getCMgr()) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneParticipantDeviceMuted, 1, 5000));
			}
			if (mgr != focus.getCMgr()) {
				LinphoneCall* c1=linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
				BC_ASSERT_TRUE(linphone_call_get_microphone_muted(c1) == ((mgr == laure.getCMgr()) || (mgr == marie.getCMgr())));
			}

			LinphoneConference * pconference = linphone_core_search_conference_2(mgr->lc, confAddr);

			bctbx_list_t * participant_device_list = linphone_conference_get_participant_device_list(pconference);
			for (bctbx_list_t *d_it = participant_device_list; d_it; d_it = bctbx_list_next(d_it)) {
				LinphoneParticipantDevice * d = (LinphoneParticipantDevice *)bctbx_list_get_data(d_it);
				BC_ASSERT_PTR_NOT_NULL(d);
				if (d) {
					BC_ASSERT_TRUE((!!linphone_participant_device_get_is_muted(d)) == ((linphone_address_weak_equal(linphone_participant_device_get_address(d), laure.getCMgr()->identity)) || (linphone_address_weak_equal(linphone_participant_device_get_address(d), marie.getCMgr()->identity))));
				}
			}
			bctbx_list_free_with_data(participant_device_list, (void(*)(void *))linphone_participant_device_unref);

		}

		if (client_restart) {
			coresList = bctbx_list_remove(coresList, marie.getLc());
			//Restart flexisip
			marie.reStart();

			if (enable_video) {
				LinphoneVideoActivationPolicy * pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
				linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
				linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
				linphone_core_set_video_activation_policy(marie.getLc(), pol);
				linphone_video_activation_policy_unref(pol);

				linphone_core_enable_video_capture(marie.getLc(), TRUE);
				linphone_core_enable_video_display(marie.getLc(), TRUE);
			}

			linphone_core_set_default_conference_layout(marie.getLc(), layout);
			coresList = bctbx_list_append(coresList, marie.getLc());

			stats focus_stat2=focus.getStats();
			stats marie_stat2=marie.getStats();

			LinphoneCallParams *new_params = linphone_core_create_call_params(marie.getLc(), nullptr);
			// Multistream ZRTP is not supported yet
			if (encryption == LinphoneMediaEncryptionZRTP) {
				linphone_call_params_set_media_encryption (new_params, LinphoneMediaEncryptionSRTP);
			} else {
				linphone_call_params_set_media_encryption (new_params, encryption);
			}
			linphone_core_invite_address_with_params_2(marie.getLc(), confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallOutgoingProgress, marie_stat2.number_of_LinphoneCallOutgoingProgress + 1, 10000));
			int no_streams_running = ((enable_ice) ? 3 : 2);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating, marie_stat2.number_of_LinphoneCallUpdating + (no_streams_running - 1), 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning, marie_stat2.number_of_LinphoneCallStreamsRunning + no_streams_running, 10000));
			// Update to add to conference.
			// If ICE is enabled, the addition to a conference may go through a resume of the call
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, marie_stat2.number_of_LinphoneConferenceStateCreated + 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionOutgoingProgress, marie_stat2.number_of_LinphoneSubscriptionOutgoingProgress + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive, marie_stat2.number_of_LinphoneSubscriptionActive + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyReceived, marie_stat2.number_of_NotifyReceived + 1, 10000));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived, focus_stat2.number_of_LinphoneCallIncomingReceived + 1, 10000));
			int focus_no_streams_running2 = ((enable_ice) ? 2 : 1);
			// Update to end ICE negotiations
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote, focus_stat2.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running2 - 1), 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat2.number_of_LinphoneCallStreamsRunning + focus_no_streams_running2, 10000));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived, focus_stat2.number_of_LinphoneSubscriptionIncomingReceived + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive, focus_stat2.number_of_LinphoneSubscriptionActive + 1, 5000));

			//wait bit more to detect side effect if any
			CoreManagerAssert({focus,marie,pauline,laure,michelle,lise}).waitUntil(chrono::seconds(2),[] {
				return false;
			});

			for (auto mgr : conferenceMgrs) {
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

					bctbx_list_t * participant_device_list = linphone_conference_get_participant_device_list(pconference);
					for (bctbx_list_t *d_it = participant_device_list; d_it; d_it = bctbx_list_next(d_it)) {
						LinphoneParticipantDevice * d = (LinphoneParticipantDevice *)bctbx_list_get_data(d_it);
						BC_ASSERT_PTR_NOT_NULL(d);
						if (d) {
							BC_ASSERT_TRUE((!!linphone_participant_device_get_is_muted(d)) == (linphone_address_weak_equal(linphone_participant_device_get_address(d), laure.getCMgr()->identity)));
						}
					}
					bctbx_list_free_with_data(participant_device_list, (void(*)(void *))linphone_participant_device_unref);

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
						linphone_video_activation_policy_unref(pol);

						int no_streams_audio = 1;
						int no_streams_video = (enabled) ? 4 : 0;
						int no_streams_text = 0;

						LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
						BC_ASSERT_PTR_NOT_NULL(pcall);
						if (pcall) {
							_linphone_call_check_nb_streams(pcall, no_streams_audio, no_streams_video, no_streams_text);
							const LinphoneCallParams* call_lparams = linphone_call_get_params(pcall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
							const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(pcall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
							const LinphoneCallParams* call_cparams = linphone_call_get_current_params(pcall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
						}
						LinphoneCall * ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
						BC_ASSERT_PTR_NOT_NULL(ccall);
						if (ccall) {
							_linphone_call_check_nb_streams(ccall, no_streams_audio, no_streams_video, no_streams_text);
							const LinphoneCallParams* call_lparams = linphone_call_get_params(ccall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
							const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(ccall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
							const LinphoneCallParams* call_cparams = linphone_call_get_current_params(ccall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
						}
					}
					BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int, "%0d");
					bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
					BC_ASSERT_EQUAL(bctbx_list_size(devices), 3, size_t, "%zu");
					if (devices) {
						bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
					}
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
		}

		// Wait a little bit
		wait_for_list(coresList, NULL, 0, 3000);

		if (enable_video) {
			LinphoneCall * pauline_call = linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(pauline_call);

			Address paulineAddr(pauline.getIdentity().asAddress());
			LinphoneCall * focus_call = linphone_core_get_call_by_remote_address2(focus.getLc(), L_GET_C_BACK_PTR(&paulineAddr));
			BC_ASSERT_PTR_NOT_NULL(focus_call);

			LinphoneVideoActivationPolicy * pol = linphone_core_get_video_activation_policy(pauline.getLc());
			bool_t enable = !!!linphone_video_activation_policy_get_automatically_initiate(pol);
			linphone_video_activation_policy_unref(pol);

			LinphoneAddress *paulineUri = linphone_address_new(linphone_core_get_identity(pauline.getLc()));
			LinphoneConference * paulineConference = linphone_core_search_conference(pauline.getLc(), NULL, paulineUri, confAddr, NULL);
			linphone_address_unref(paulineUri);
			BC_ASSERT_PTR_NOT_NULL(paulineConference);

			for (int i = 0; i < 4; i++) {
				stats focus_stat2=focus.getStats();
				stats marie_stat2=marie.getStats();
				stats pauline_stat2=pauline.getStats();
				stats laure_stat2=laure.getStats();

				ms_message("Pauline %s video with direction %s", (enable ? "enables" : "disables"), linphone_media_direction_to_string(video_direction));

				if (pauline_call) {
					LinphoneCallParams * new_params = linphone_core_create_call_params(pauline.getLc(), pauline_call);
					linphone_call_params_enable_video (new_params, enable);
					linphone_call_params_set_video_direction (new_params, video_direction);
					linphone_call_update(pauline_call, new_params);
					linphone_call_params_unref (new_params);
				}

				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallUpdating, pauline_stat2.number_of_LinphoneCallUpdating + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning, pauline_stat2.number_of_LinphoneCallStreamsRunning + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote, focus_stat2.number_of_LinphoneCallUpdatedByRemote + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat2.number_of_LinphoneCallStreamsRunning + 1, 10000));

				if (video_direction == LinphoneMediaDirectionSendRecv) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating, marie_stat2.number_of_LinphoneCallUpdating + 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning, marie_stat2.number_of_LinphoneCallStreamsRunning + 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallUpdating, laure_stat2.number_of_LinphoneCallUpdating + 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallStreamsRunning, laure_stat2.number_of_LinphoneCallStreamsRunning + 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote, focus_stat2.number_of_LinphoneCallUpdatedByRemote + 3, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat2.number_of_LinphoneCallStreamsRunning + 3, 10000));
				}

				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_media_capability_changed, focus_stat2.number_of_participant_devices_media_capability_changed + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_media_capability_changed, marie_stat2.number_of_participant_devices_media_capability_changed + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_media_capability_changed, pauline_stat2.number_of_participant_devices_media_capability_changed + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_media_capability_changed, laure_stat2.number_of_participant_devices_media_capability_changed + 1, 10000));
				BC_ASSERT_EQUAL(focus.getStats().number_of_participants_added, focus_stat2.number_of_participants_added, int, "%0d");
				BC_ASSERT_EQUAL(focus.getStats().number_of_participant_devices_added, focus_stat2.number_of_participant_devices_added, int, "%0d");
				BC_ASSERT_EQUAL(focus.getStats().number_of_participant_devices_joined, focus_stat2.number_of_participant_devices_joined, int, "%0d");
				BC_ASSERT_EQUAL(marie.getStats().number_of_participants_added, marie_stat2.number_of_participants_added, int, "%0d");
				BC_ASSERT_EQUAL(marie.getStats().number_of_participant_devices_added, marie_stat2.number_of_participant_devices_added, int, "%0d");
				BC_ASSERT_EQUAL(marie.getStats().number_of_participant_devices_joined, marie_stat2.number_of_participant_devices_joined, int, "%0d");
				BC_ASSERT_EQUAL(pauline.getStats().number_of_participants_added, pauline_stat2.number_of_participants_added, int, "%0d");
				BC_ASSERT_EQUAL(pauline.getStats().number_of_participant_devices_added, pauline_stat2.number_of_participant_devices_added, int, "%0d");
				BC_ASSERT_EQUAL(pauline.getStats().number_of_participant_devices_joined, pauline_stat2.number_of_participant_devices_joined, int, "%0d");
				BC_ASSERT_EQUAL(laure.getStats().number_of_participants_added, laure_stat2.number_of_participants_added, int, "%0d");
				BC_ASSERT_EQUAL(laure.getStats().number_of_participant_devices_added, laure_stat2.number_of_participant_devices_added, int, "%0d");
				BC_ASSERT_EQUAL(laure.getStats().number_of_participant_devices_joined, laure_stat2.number_of_participant_devices_joined, int, "%0d");

				for (auto mgr : conferenceMgrs) {
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
									BC_ASSERT_TRUE(linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) == video_direction);
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

				if (pauline_call) {
					const LinphoneCallParams* call_lparams = linphone_call_get_params(pauline_call);
					BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enable, int, "%0d");
					const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(pauline_call);
					BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enable, int, "%0d");
					const LinphoneCallParams* call_cparams = linphone_call_get_current_params(pauline_call);
					BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enable, int, "%0d");
				}
				if (focus_call) {
					const LinphoneCallParams* call_lparams = linphone_call_get_params(focus_call);
					BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enable, int, "%0d");
					const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(focus_call);
					BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enable, int, "%0d");
					const LinphoneCallParams* call_cparams = linphone_call_get_current_params(focus_call);
					BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enable, int, "%0d");
				}

				if (paulineConference) {
					bctbx_list_t *devices = linphone_conference_get_participant_device_list(paulineConference);
					for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
						LinphoneParticipantDevice * d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
						if (enable) {
							if (linphone_conference_is_me(paulineConference, linphone_participant_device_get_address(d))) {
								BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) == (video_direction == LinphoneMediaDirectionSendRecv));
							} else {
								BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) == (linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) == LinphoneMediaDirectionSendRecv));
							}
						} else {
							BC_ASSERT_FALSE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
						}
					}

					if (devices) {
						bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
					}
				}
				// Wait a little bit
				wait_for_list(coresList, NULL, 0, 1000);

				enable = !enable;
			}

			if (paulineConference) {
				stats focus_stat2=focus.getStats();
				stats marie_stat2=marie.getStats();
				stats pauline_stat2=pauline.getStats();
				stats laure_stat2=laure.getStats();

				LinphoneCall * pauline_call = linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
				BC_ASSERT_PTR_NOT_NULL(pauline_call);

				bool_t video_enabled = FALSE;
				if (pauline_call) {
					const LinphoneCallParams* call_cparams = linphone_call_get_current_params(pauline_call);
					video_enabled = linphone_call_params_video_enabled(call_cparams);
				}

				linphone_conference_leave(paulineConference);

				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallPausing, pauline_stat2.number_of_LinphoneCallPausing + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallPaused, pauline_stat2.number_of_LinphoneCallPaused + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallPausedByRemote, focus_stat2.number_of_LinphoneCallPausedByRemote + 1, 10000));

				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_on_hold, focus_stat2.number_of_participant_devices_on_hold + 1, 10000));

				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_on_hold, laure_stat2.number_of_participant_devices_on_hold + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_media_capability_changed, laure_stat2.number_of_participant_devices_media_capability_changed + 1, 10000));

				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_on_hold, marie_stat2.number_of_participant_devices_on_hold + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_media_capability_changed, marie_stat2.number_of_participant_devices_media_capability_changed + 1, 10000));

				for (auto mgr : conferenceMgrs) {
					LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
					LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
					linphone_address_unref(uri);
					BC_ASSERT_PTR_NOT_NULL(pconference);
					if (pconference) {
						if (mgr == pauline.getCMgr()) {
							BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
						} else {
							LinphoneParticipant * participant = linphone_conference_find_participant(pconference, pauline.getCMgr()->identity);
							BC_ASSERT_PTR_NOT_NULL(participant);
							if (participant) {
								bctbx_list_t *devices = linphone_participant_get_devices(participant);
								for(bctbx_list_t *it_d = devices; it_d != NULL; it_d = it_d->next) {
									LinphoneParticipantDevice *d = (LinphoneParticipantDevice *) it_d->data;
									BC_ASSERT_PTR_NOT_NULL(d);
									if (d) {
										BC_ASSERT_EQUAL(linphone_participant_device_get_state(d), LinphoneParticipantDeviceStateOnHold, int, "%0d");
									}
								}
								bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
							}
						}
					}
				}

				linphone_conference_enter(paulineConference);

				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallResuming, pauline_stat2.number_of_LinphoneCallResuming + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning, pauline_stat2.number_of_LinphoneCallStreamsRunning + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat2.number_of_LinphoneCallStreamsRunning + 1, 10000));

				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined, focus_stat2.number_of_participant_devices_joined + 1, 10000));

				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_joined, laure_stat2.number_of_participant_devices_joined + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_media_capability_changed, laure_stat2.number_of_participant_devices_media_capability_changed + 2, 10000));

				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_joined, marie_stat2.number_of_participant_devices_joined + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_media_capability_changed, marie_stat2.number_of_participant_devices_media_capability_changed + 2, 10000));

				if (pauline_call) {
					const LinphoneCallParams* call_cparams = linphone_call_get_current_params(pauline_call);
					BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), video_enabled, int, "%0d");
				}

				for (auto mgr : conferenceMgrs) {
					LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
					LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
					linphone_address_unref(uri);
					BC_ASSERT_PTR_NOT_NULL(pconference);
					if (pconference) {
						if (mgr == pauline.getCMgr()) {
							BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
						} else {
							LinphoneParticipant * participant = linphone_conference_find_participant(pconference, pauline.getCMgr()->identity);
							BC_ASSERT_PTR_NOT_NULL(participant);
							if (participant) {
								bctbx_list_t *devices = linphone_participant_get_devices(participant);
								for(bctbx_list_t *it_d = devices; it_d != NULL; it_d = it_d->next) {
									LinphoneParticipantDevice *d = (LinphoneParticipantDevice *) it_d->data;
									BC_ASSERT_PTR_NOT_NULL(d);
									if (d) {
										BC_ASSERT_EQUAL(linphone_participant_device_get_state(d), LinphoneParticipantDeviceStatePresent, int, "%0d");
									}
								}
								bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
							}
						}
					}
				}
			}
		}

		std::list <LinphoneCoreManager*> extraParticipantMgrs;
		int no_local_participants = 3;
		if (uninvited_participant_dials) {
			stats marie_stat2=marie.getStats();
			stats focus_stat2=focus.getStats();
			stats pauline_stat2=pauline.getStats();
			stats laure_stat2=laure.getStats();

			extraParticipantMgrs.push_back(michelle.getCMgr());

			char * conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("<unknown>");
			ms_message("%s is entering conference %s", linphone_core_get_identity(michelle.getLc()), conference_address_str);
			ms_free(conference_address_str);

			LinphoneCallParams *params = linphone_core_create_call_params(michelle.getLc(), nullptr);
			LinphoneCall * michelle_call = linphone_core_invite_address_with_params(michelle.getLc(), confAddr, params);
			BC_ASSERT_PTR_NOT_NULL(michelle_call);
			linphone_call_params_unref(params);
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallOutgoingProgress, 1, 10000));
			int extra_participants = 0;
			if (participant_list_type == LinphoneConferenceParticipantListTypeOpen) {
				conferenceMgrs.push_back(michelle.getCMgr());
				members.push_back(michelle.getCMgr());

				extra_participants = 1;

				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallStreamsRunning, 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallUpdating, 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallStreamsRunning, 2, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateCreated, 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneSubscriptionActive, 1, 5000));

				if (!audio_only_participant) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallUpdating, pauline_stat2.number_of_LinphoneCallUpdating + 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning, pauline_stat2.number_of_LinphoneCallStreamsRunning + 1, 10000));
				}
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating, marie_stat2.number_of_LinphoneCallUpdating + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning, marie_stat2.number_of_LinphoneCallStreamsRunning + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallUpdating, laure_stat2.number_of_LinphoneCallUpdating + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallStreamsRunning, laure_stat2.number_of_LinphoneCallStreamsRunning + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote, focus_stat2.number_of_LinphoneCallUpdatedByRemote + 4, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat2.number_of_LinphoneCallStreamsRunning + 5, 10000));

				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added, focus_stat2.number_of_participants_added + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added, focus_stat2.number_of_participant_devices_added + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined, focus_stat2.number_of_participant_devices_joined + 1, 10000));

				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_added, marie_stat2.number_of_participants_added + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_added, marie_stat2.number_of_participant_devices_added + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_joined, marie_stat2.number_of_participant_devices_joined + 1, 10000));

				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_added, pauline_stat2.number_of_participants_added + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_added, pauline_stat2.number_of_participant_devices_added + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_joined, pauline_stat2.number_of_participant_devices_joined + 1, 10000));

				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participants_added, laure_stat2.number_of_participants_added + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_added, laure_stat2.number_of_participant_devices_added + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_joined, laure_stat2.number_of_participant_devices_joined + 1, 10000));

				wait_for_conference_streams({focus,marie,pauline,laure,michelle,lise}, conferenceMgrs, focus.getCMgr(), members, confAddr, enable_video);
			} else if (participant_list_type == LinphoneConferenceParticipantListTypeClosed) {
				extra_participants = 0;

				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallError, 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallReleased, 1, 10000));

				//wait bit more to detect side effect if any
				CoreManagerAssert({focus,marie,pauline,laure,michelle,lise}).waitUntil(chrono::seconds(2),[] {
					return false;
				});

				BC_ASSERT_EQUAL(michelle.getStats().number_of_LinphoneConferenceStateCreated, 0, int, "%0d");
				BC_ASSERT_EQUAL(michelle.getStats().number_of_LinphoneSubscriptionOutgoingProgress, 0, int, "%0d");
				BC_ASSERT_EQUAL(michelle.getStats().number_of_LinphoneSubscriptionActive, 0, int, "%0d");
				BC_ASSERT_EQUAL(focus.getStats().number_of_participants_added, focus_stat2.number_of_participants_added, int, "%0d");
				BC_ASSERT_EQUAL(focus.getStats().number_of_participant_devices_added, focus_stat2.number_of_participant_devices_added, int, "%0d");
				BC_ASSERT_EQUAL(focus.getStats().number_of_participant_devices_joined, focus_stat2.number_of_participant_devices_joined, int, "%0d");
				BC_ASSERT_EQUAL(marie.getStats().number_of_participants_added, marie_stat2.number_of_participants_added, int, "%0d");
				BC_ASSERT_EQUAL(marie.getStats().number_of_participant_devices_added, marie_stat2.number_of_participant_devices_added, int, "%0d");
				BC_ASSERT_EQUAL(marie.getStats().number_of_participant_devices_joined, marie_stat2.number_of_participant_devices_joined, int, "%0d");
				BC_ASSERT_EQUAL(pauline.getStats().number_of_participants_added, pauline_stat2.number_of_participants_added, int, "%0d");
				BC_ASSERT_EQUAL(pauline.getStats().number_of_participant_devices_added, pauline_stat2.number_of_participant_devices_added, int, "%0d");
				BC_ASSERT_EQUAL(pauline.getStats().number_of_participant_devices_joined, pauline_stat2.number_of_participant_devices_joined, int, "%0d");
				BC_ASSERT_EQUAL(laure.getStats().number_of_participants_added, laure_stat2.number_of_participants_added, int, "%0d");
				BC_ASSERT_EQUAL(laure.getStats().number_of_participant_devices_added, laure_stat2.number_of_participant_devices_added, int, "%0d");
				BC_ASSERT_EQUAL(laure.getStats().number_of_participant_devices_joined, laure_stat2.number_of_participant_devices_joined, int, "%0d");
			}

			for (auto mgr : conferenceMgrs) {
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

						LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
						BC_ASSERT_PTR_NOT_NULL(pcall);

						int no_streams_audio = 1;
						int no_streams_video = 0;
						int no_streams_text = 0;
						if (pcall) {
							const LinphoneCallParams* call_cparams = linphone_call_get_current_params(pcall);
							const bool_t enabled = linphone_call_params_video_enabled(call_cparams);
							no_streams_video = (enabled && (participant_list_type == LinphoneConferenceParticipantListTypeOpen)) ? 5 : (enable_video) ? 4 : 0;
							_linphone_call_check_nb_streams(pcall, no_streams_audio, no_streams_video, no_streams_text);
						}

						LinphoneCall * fcall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
						BC_ASSERT_PTR_NOT_NULL(fcall);
						if (fcall) {
							_linphone_call_check_nb_streams(fcall, no_streams_audio, no_streams_video, no_streams_text);
						}

					}
					BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int, "%0d");
					bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
					BC_ASSERT_EQUAL(bctbx_list_size(devices), static_cast<size_t>(no_local_participants + extra_participants), size_t, "%zu");
					if (devices) {
						bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
					}
					BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
				}
			}
		}

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure,michelle,lise}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		stats marie_stat=marie.getStats();

		std::list <LinphoneCoreManager*> mgrsToRemove {pauline.getCMgr()};
		if (remove_participant) {
			stats pauline_stat=pauline.getStats();

			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(marie.getLc()));
			LinphoneConference * pconference = linphone_core_search_conference(marie.getLc(), NULL, uri, confAddr, NULL);
			linphone_address_unref(uri);

			char * conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("<unknown>");
			ms_message("%s is removing %s from conference %s", linphone_core_get_identity(marie.getLc()), linphone_core_get_identity(laure.getLc()), conference_address_str);
			ms_free(conference_address_str);

			BC_ASSERT_PTR_NOT_NULL(pconference);
			LinphoneAddress *puri = linphone_address_new(linphone_core_get_identity(laure.getLc()));
			if (pconference) {
				LinphoneParticipant * participant = linphone_conference_find_participant(pconference, puri);
				BC_ASSERT_PTR_NOT_NULL(participant);
				linphone_conference_remove_participant_2(pconference, participant);
			}

			auto itConferenceMgrs = std::find(conferenceMgrs.begin(), conferenceMgrs.end(), laure.getCMgr());
			if (itConferenceMgrs != conferenceMgrs.end()) {
				conferenceMgrs.erase(itConferenceMgrs);
			}

			auto itMembers = std::find(members.begin(), members.end(), laure.getCMgr());
			if (itMembers != members.end()) {
				members.erase(itMembers);
			}

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

			if ((uninvited_participant_dials) && (participant_list_type == LinphoneConferenceParticipantListTypeOpen)) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participants_removed, 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_removed, 1, 10000));
			}

			LinphoneConference * conference = linphone_core_search_conference(laure.getLc(), NULL, puri, confAddr, NULL);
			BC_ASSERT_PTR_NULL(conference);
			linphone_address_unref(puri);

			no_local_participants = 3;
			if (uninvited_participant_dials) {
				stats marie_stat2=marie.getStats();
				stats focus_stat2=focus.getStats();
				stats pauline_stat2=pauline.getStats();
				stats michelle_stat2=michelle.getStats();

				extraParticipantMgrs.push_back(lise.getCMgr());
				conferenceMgrs.push_back(lise.getCMgr());
				members.push_back(lise.getCMgr());
				char * conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("<unknown>");
				ms_message("%s is entering conference %s", linphone_core_get_identity(lise.getLc()), conference_address_str);
				ms_free(conference_address_str);

				LinphoneCallParams *params = linphone_core_create_call_params(lise.getLc(), nullptr);
				LinphoneCall * lise_call = linphone_core_invite_address_with_params(lise.getLc(), confAddr, params);
				BC_ASSERT_PTR_NOT_NULL(lise_call);
				linphone_call_params_unref(params);
				BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_LinphoneCallOutgoingProgress, 1, 10000));
				int extra_participants = 0;
				if (participant_list_type == LinphoneConferenceParticipantListTypeOpen) {
					extra_participants = 1;

					BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_LinphoneCallStreamsRunning, 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_LinphoneCallUpdating, 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_LinphoneCallStreamsRunning, 2, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_LinphoneConferenceStateCreated, 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_LinphoneSubscriptionActive, 1, 5000));

					if (!audio_only_participant) {
						BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallUpdating, pauline_stat2.number_of_LinphoneCallUpdating + 1, 10000));
						BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning, pauline_stat2.number_of_LinphoneCallStreamsRunning + 1, 10000));
					}
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating, marie_stat2.number_of_LinphoneCallUpdating + 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning, marie_stat2.number_of_LinphoneCallStreamsRunning + 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallUpdating, michelle_stat2.number_of_LinphoneCallUpdating + 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallStreamsRunning, michelle_stat2.number_of_LinphoneCallStreamsRunning + 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote, focus_stat2.number_of_LinphoneCallUpdatedByRemote + (audio_only_participant) ? 3 : 4, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat2.number_of_LinphoneCallStreamsRunning + (audio_only_participant) ? 4 : 5, 10000));

					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added, focus_stat2.number_of_participants_added + 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added, focus_stat2.number_of_participant_devices_added + 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined, focus_stat2.number_of_participant_devices_joined + 1, 10000));

					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_added, marie_stat2.number_of_participants_added + 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_added, marie_stat2.number_of_participant_devices_added + 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_joined, marie_stat2.number_of_participant_devices_joined + 1, 10000));

					BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_added, pauline_stat2.number_of_participants_added + 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_added, pauline_stat2.number_of_participant_devices_added + 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_joined, pauline_stat2.number_of_participant_devices_joined + 1, 10000));

					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participants_added, michelle_stat2.number_of_participants_added + 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_added, michelle_stat2.number_of_participant_devices_added + 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_joined, michelle_stat2.number_of_participant_devices_joined + 1, 10000));
				} else if (participant_list_type == LinphoneConferenceParticipantListTypeClosed) {
					extra_participants = 0;

					BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_LinphoneCallError, 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_LinphoneCallReleased, 1, 10000));

					//wait bit more to detect side effect if any
					CoreManagerAssert({focus,marie,pauline,laure,michelle,lise}).waitUntil(chrono::seconds(2),[] {
						return false;
					});

					BC_ASSERT_EQUAL(lise.getStats().number_of_LinphoneConferenceStateCreated, 0, int, "%0d");
					BC_ASSERT_EQUAL(lise.getStats().number_of_LinphoneSubscriptionOutgoingProgress, 0, int, "%0d");
					BC_ASSERT_EQUAL(lise.getStats().number_of_LinphoneSubscriptionActive, 0, int, "%0d");
					BC_ASSERT_EQUAL(focus.getStats().number_of_participants_added, focus_stat2.number_of_participants_added, int, "%0d");
					BC_ASSERT_EQUAL(focus.getStats().number_of_participant_devices_added, focus_stat2.number_of_participant_devices_added, int, "%0d");
					BC_ASSERT_EQUAL(focus.getStats().number_of_participant_devices_joined, focus_stat2.number_of_participant_devices_joined, int, "%0d");
					BC_ASSERT_EQUAL(marie.getStats().number_of_participants_added, marie_stat2.number_of_participants_added, int, "%0d");
					BC_ASSERT_EQUAL(marie.getStats().number_of_participant_devices_added, marie_stat2.number_of_participant_devices_added, int, "%0d");
					BC_ASSERT_EQUAL(marie.getStats().number_of_participant_devices_joined, marie_stat2.number_of_participant_devices_joined, int, "%0d");
					BC_ASSERT_EQUAL(pauline.getStats().number_of_participants_added, pauline_stat2.number_of_participants_added, int, "%0d");
					BC_ASSERT_EQUAL(pauline.getStats().number_of_participant_devices_added, pauline_stat2.number_of_participant_devices_added, int, "%0d");
					BC_ASSERT_EQUAL(pauline.getStats().number_of_participant_devices_joined, pauline_stat2.number_of_participant_devices_joined, int, "%0d");
					BC_ASSERT_EQUAL(michelle.getStats().number_of_participants_added, michelle_stat2.number_of_participants_added, int, "%0d");
					BC_ASSERT_EQUAL(michelle.getStats().number_of_participant_devices_added, michelle_stat2.number_of_participant_devices_added, int, "%0d");
					BC_ASSERT_EQUAL(michelle.getStats().number_of_participant_devices_joined, michelle_stat2.number_of_participant_devices_joined, int, "%0d");
				}

				for (auto mgr : conferenceMgrs) {
					LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
					LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
					linphone_address_unref(uri);
					if ((participant_list_type == LinphoneConferenceParticipantListTypeOpen) || ((mgr != lise.getCMgr()) && (mgr != michelle.getCMgr()))) {
						BC_ASSERT_PTR_NOT_NULL(pconference);
					} else if ((mgr == lise.getCMgr()) || (mgr == michelle.getCMgr())) {
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
							LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
							BC_ASSERT_PTR_NOT_NULL(pcall);

							int no_streams_audio = 1;
							int no_streams_video = 0;
							int no_streams_text = 0;
							if (pcall) {
								const LinphoneCallParams* call_cparams = linphone_call_get_current_params(pcall);
								const bool_t enabled = linphone_call_params_video_enabled(call_cparams);
								no_streams_video = (enabled && (participant_list_type == LinphoneConferenceParticipantListTypeOpen)) ? 5 : (enable_video) ? 4 : 0;
								_linphone_call_check_nb_streams(pcall, no_streams_audio, no_streams_video, no_streams_text);
							}

							LinphoneCall * fcall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
							BC_ASSERT_PTR_NOT_NULL(fcall);
							if (fcall) {
								_linphone_call_check_nb_streams(fcall, no_streams_audio, no_streams_video, no_streams_text);
							}

							// Substracting one because we conference server is not in the conference
							no_participants = (no_local_participants - 1) + extra_participants;
							BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
						}
						BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int, "%0d");
						bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
						BC_ASSERT_EQUAL(bctbx_list_size(devices), static_cast<size_t>(no_local_participants + extra_participants), size_t, "%zu");
						if (devices) {
							bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
						}
						BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
					}
				}

				//wait bit more to detect side effect if any
				CoreManagerAssert({focus,marie,pauline,laure,michelle,lise}).waitUntil(chrono::seconds(2),[] {
					return false;
				});

			}

		} else {
			mgrsToRemove.push_back(laure.getCMgr());
		}

		LinphoneAddress *paulineUri = linphone_address_new(linphone_core_get_identity(pauline.getLc()));
		LinphoneConference * paulineConference = linphone_core_search_conference(pauline.getLc(), NULL, paulineUri, confAddr, NULL);
		linphone_address_unref(paulineUri);
		BC_ASSERT_PTR_NOT_NULL(paulineConference);

		if (paulineConference) {
			stats focus_stat2=focus.getStats();
			stats marie_stat2=marie.getStats();
			stats pauline_stat2=pauline.getStats();
			stats laure_stat2=laure.getStats();
			stats michelle_stat2=michelle.getStats();

			LinphoneCall * pauline_call = linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(pauline_call);

			bool_t video_enabled = FALSE;
			if (pauline_call) {
				const LinphoneCallParams* call_cparams = linphone_call_get_current_params(pauline_call);
				video_enabled = linphone_call_params_video_enabled(call_cparams);
			}

			linphone_conference_leave(paulineConference);

			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallPausing, pauline_stat2.number_of_LinphoneCallPausing + 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallPaused, pauline_stat2.number_of_LinphoneCallPaused + 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallPausedByRemote, focus_stat2.number_of_LinphoneCallPausedByRemote + 1, 10000));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_on_hold, focus_stat2.number_of_participant_devices_on_hold + 1, 10000));

			if (!remove_participant) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_on_hold, laure_stat2.number_of_participant_devices_on_hold + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_media_capability_changed, laure_stat2.number_of_participant_devices_media_capability_changed + 1, 10000));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_on_hold, marie_stat2.number_of_participant_devices_on_hold + 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_media_capability_changed, marie_stat2.number_of_participant_devices_media_capability_changed + 1, 10000));

			if (uninvited_participant_dials && (participant_list_type == LinphoneConferenceParticipantListTypeOpen)) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_on_hold, michelle_stat2.number_of_participant_devices_on_hold + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_media_capability_changed, michelle_stat2.number_of_participant_devices_media_capability_changed + 1, 10000));
			}

			linphone_conference_enter(paulineConference);

			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallResuming, pauline_stat2.number_of_LinphoneCallResuming + 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning, pauline_stat2.number_of_LinphoneCallStreamsRunning + 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat2.number_of_LinphoneCallStreamsRunning + 1, 10000));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined, focus_stat2.number_of_participant_devices_joined + 1, 10000));

			if (!remove_participant) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_joined, laure_stat2.number_of_participant_devices_joined + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_media_capability_changed, laure_stat2.number_of_participant_devices_media_capability_changed + 2, 10000));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_joined, marie_stat2.number_of_participant_devices_joined + 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_media_capability_changed, marie_stat2.number_of_participant_devices_media_capability_changed + 2, 10000));

			if (uninvited_participant_dials && (participant_list_type == LinphoneConferenceParticipantListTypeOpen)) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_joined, michelle_stat2.number_of_participant_devices_joined + 1, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_media_capability_changed, michelle_stat2.number_of_participant_devices_media_capability_changed + 1, 10000));
			}

			if (pauline_call) {
				const LinphoneCallParams* call_cparams = linphone_call_get_current_params(pauline_call);
				BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), video_enabled, int, "%0d");
			}
		}

		for (auto mgr : mgrsToRemove) {

			auto itConferenceMgrs = std::find(conferenceMgrs.begin(), conferenceMgrs.end(), mgr);
			if (itConferenceMgrs != conferenceMgrs.end()) {
				conferenceMgrs.erase(itConferenceMgrs);
			}

			auto itMembers = std::find(members.begin(), members.end(), mgr);
			if (itMembers != members.end()) {
				members.erase(itMembers);
			}

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

		if (uninvited_participant_dials) {
			if (participant_list_type == LinphoneConferenceParticipantListTypeOpen) {
				int extra_participants = static_cast<int>(extraParticipantMgrs.size());

				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participants_removed, 2, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_removed, 2, 10000));

				if (remove_participant) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_participants_removed, 1, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_participant_devices_removed, 1, 10000));
				}

				for (auto mgr : {focus.getCMgr(), marie.getCMgr(), michelle.getCMgr(), lise.getCMgr()}) {
					LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
					LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
					BC_ASSERT_PTR_NOT_NULL(pconference);
					linphone_address_unref(uri);
					if (pconference) {
						BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), ((mgr == focus.getCMgr()) ? (extra_participants + 1) : extra_participants), int, "%0d");
						bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
						BC_ASSERT_EQUAL(bctbx_list_size(devices), (extra_participants + 1), size_t, "%zu");
						if (devices) {
							bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
						}
						BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
					}
				}

				stats marie_stat2=marie.getStats();
				stats focus_stat2=focus.getStats();

				for (auto mgr : extraParticipantMgrs) {
					LinphoneCall * call = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);

					auto itConferenceMgrs = std::find(conferenceMgrs.begin(), conferenceMgrs.end(), mgr);
					if (itConferenceMgrs != conferenceMgrs.end()) {
						conferenceMgrs.erase(itConferenceMgrs);
					}

					auto itMembers = std::find(members.begin(), members.end(), mgr);
					if (itMembers != members.end()) {
						members.erase(itMembers);
					}

					BC_ASSERT_PTR_NOT_NULL(call);
					if (call) {
						linphone_call_terminate(call);
						BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, 10000));
						BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1, 10000));
						BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated, 1, 10000));
						BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending, 1, 10000));
						BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated, 1, 10000));
						BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1, 10000));
					}
				}

				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed, focus_stat2.number_of_participants_removed + extra_participants, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed, focus_stat2.number_of_participant_devices_removed + extra_participants, 10000));

				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed, marie_stat2.number_of_participants_removed + extra_participants, 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_removed, marie_stat2.number_of_participant_devices_removed + extra_participants, 10000));
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
		CoreManagerAssert({focus,marie,pauline,laure,michelle,lise}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		BC_ASSERT_EQUAL(marie.getStats().number_of_LinphoneConferenceStateTerminationPending, marie_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(marie.getStats().number_of_LinphoneConferenceStateTerminated, marie_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(marie.getStats().number_of_LinphoneConferenceStateDeleted, marie_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminationPending, focus_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminated, focus_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateDeleted, focus_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		for (auto mgr : conferenceMgrs) {
			LinphoneConference * pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), ((mgr == focus.getCMgr()) ? 1 : 0), int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 1, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
			}
		}

		const bctbx_list_t * calls = linphone_core_get_calls(marie.getLc());
		BC_ASSERT_EQUAL(bctbx_list_size(calls), 1, size_t, "%zu");

		LinphoneCall * call = linphone_core_get_call_by_remote_address2(marie.getLc(), focus.getCMgr()->identity);
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
			LinphoneConference * pconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				linphone_conference_terminate(pconference);
			}
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1, 10000));
		}

		std::list<LinphoneCoreManager *> allMembers{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()};
		if ((participant_list_type == LinphoneConferenceParticipantListTypeOpen) && uninvited_participant_dials) {
			allMembers.push_back(michelle.getCMgr());
			if (remove_participant) {
				allMembers.push_back(lise.getCMgr());
			}
		}
		for (auto mgr : allMembers) {
			const bctbx_list_t* call_logs = linphone_core_get_call_logs(mgr->lc);
			BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(call_logs), ((client_restart && (mgr == marie.getCMgr())) ? 2 : 1), unsigned int, "%u");

			bctbx_list_t * mgr_focus_call_log = linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
			if (mgr_focus_call_log) {
				BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(mgr_focus_call_log), ((client_restart && (mgr == marie.getCMgr())) ? 2 : 1), unsigned int, "%u");
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
	create_conference_base (ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionNone, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, LinphoneMediaDirectionRecvOnly);
}

static void create_simple_conference_with_server_restart (void) {
	create_conference_base (ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionNone, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, LinphoneMediaDirectionRecvOnly);
}

static void create_simple_conference_with_client_restart (void) {
	create_conference_base (ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionNone, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, LinphoneMediaDirectionRecvOnly);
}

static void create_simple_ice_conference (void) {
	create_conference_base (ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, LinphoneMediaDirectionRecvOnly);
}

static void create_simple_stun_ice_conference (void) {
	create_conference_base (ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, LinphoneMediaDirectionSendRecv);
}

static void create_simple_srtp_conference (void) {
	create_conference_base (ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionSRTP, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, LinphoneMediaDirectionRecvOnly);
}

static void create_simple_ice_srtp_conference (void) {
	create_conference_base (ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionSRTP, TRUE, LinphoneConferenceLayoutGrid, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, LinphoneMediaDirectionSendRecv);
}

static void create_simple_stun_ice_srtp_conference (void) {
	create_conference_base (ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionSRTP, TRUE, LinphoneConferenceLayoutGrid, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, LinphoneMediaDirectionSendRecv);
}

static void create_conference_with_uninvited_participant (void) {
	create_conference_base (ms_time(NULL), -1, TRUE, LinphoneConferenceParticipantListTypeOpen, TRUE, LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, LinphoneMediaDirectionRecvOnly);
}

static void create_conference_with_uninvited_participant_not_allowed (void) {
	create_conference_base (ms_time(NULL), -1, TRUE, LinphoneConferenceParticipantListTypeClosed, FALSE, LinphoneMediaEncryptionNone, FALSE, LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, LinphoneMediaDirectionSendRecv);
}

static void create_conference_starting_immediately (void) {
	create_conference_base (ms_time(NULL), 0, FALSE, LinphoneConferenceParticipantListTypeClosed, FALSE, LinphoneMediaEncryptionNone, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, LinphoneMediaDirectionRecvOnly);
}

static void create_conference_starting_in_the_past (void) {
	create_conference_base (ms_time(NULL) - 600, 900, FALSE, LinphoneConferenceParticipantListTypeClosed, TRUE, LinphoneMediaEncryptionNone, FALSE, LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, LinphoneMediaDirectionSendRecv);
}

static void create_simple_conference_with_audio_only_participant (void) {
	create_conference_base (ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, LinphoneMediaDirectionSendRecv);
}

static void create_simple_ice_conference_with_audio_only_participant (void) {
	create_conference_base (ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, LinphoneMediaDirectionRecvOnly);
}

static void create_simple_stun_ice_conference_with_audio_only_participant (void) {
	create_conference_base (ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, LinphoneMediaDirectionSendRecv);
}

static void create_simple_stun_ice_srtp_conference_with_audio_only_participant (void) {
	create_conference_base (ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionSRTP, TRUE, LinphoneConferenceLayoutGrid, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, LinphoneMediaDirectionRecvOnly);
}

static void create_conference_with_audio_only_and_uninvited_participant (void) {
	create_conference_base (ms_time(NULL), -1, TRUE, LinphoneConferenceParticipantListTypeOpen, TRUE, LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, LinphoneMediaDirectionSendRecv);
}

static void create_simple_conference_with_audio_only_participant_enabling_video (void) {
	create_conference_base (ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE, LinphoneMediaEncryptionNone, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, LinphoneMediaDirectionSendRecv);
}

static void create_conference_dial_out_base (bool_t send_ics, LinphoneConferenceLayout layout, bool_t enable_video, bool_t enable_stun, bool_t enable_ice, LinphoneConferenceParticipantListType participant_list_type, bool_t accept) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focus.getIdentity().asAddress());
		ClientConference laure("laure_tcp_rc", focus.getIdentity().asAddress());
		ClientConference michelle("michelle_rc", focus.getIdentity().asAddress());
		ClientConference lise("lise_rc", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(lise);

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t * coresList = NULL;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(), lise.getCMgr()}) {
			if (enable_video) {
				LinphoneVideoActivationPolicy * pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
				linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
				linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
				linphone_core_set_video_activation_policy(mgr->lc, pol);
				linphone_video_activation_policy_unref(pol);

				linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);
				linphone_core_enable_video_capture(mgr->lc, TRUE);
				linphone_core_enable_video_display(mgr->lc, TRUE);
			}

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, layout);
			}

			enable_stun_in_core(mgr, enable_stun, enable_ice);
			linphone_core_manager_wait_for_stun_resolution(mgr);

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), participant_list_type);

		stats focus_stat=focus.getStats();
		stats marie_stat=marie.getStats();

		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(), lise.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(), lise.getCMgr()};
		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(), lise.getCMgr()};

		const char *initialSubject = "Schedule of the trip towards the top of Europe";
		const char *description = "To the top of the Mont Blanc!!!! :-)";

		LinphoneAddress * confAddr = create_conference_on_server(focus, marie, participants, -1, -1, initialSubject, description, send_ics);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, (send_ics ? 2 : 1), liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallOutgoingInit, marie_stat.number_of_LinphoneCallOutgoingInit + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingInit, focus_stat.number_of_LinphoneCallOutgoingInit + 4, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived, focus_stat.number_of_LinphoneCallIncomingReceived + 1, liblinphone_tester_sip_timeout));

		for (auto mgr : participants) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallIncomingReceived, 1, liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning, marie_stat.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));
		if (enable_ice) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating, marie_stat.number_of_LinphoneCallUpdating + 1, 20000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote, focus_stat.number_of_LinphoneCallUpdatedByRemote + 1, liblinphone_tester_sip_timeout));
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning, marie_stat.number_of_LinphoneCallStreamsRunning + 2, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionOutgoingProgress, marie_stat.number_of_LinphoneSubscriptionOutgoingProgress + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive, marie_stat.number_of_LinphoneSubscriptionActive + 1, liblinphone_tester_sip_timeout));

		LinphoneConference * oconference = linphone_core_search_conference_2(marie.getLc(), confAddr);
		if (BC_ASSERT_PTR_NOT_NULL(oconference)) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(oconference), 4, int, "%0d");
			bctbx_list_t *devices = linphone_conference_get_participant_device_list(oconference);
			BC_ASSERT_EQUAL(bctbx_list_size(devices), 5, size_t, "%zu");
			if (devices) {
				bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
			}
		}

		if (confAddr) {
			for (auto mgr : participants) {
				check_conference_info(mgr, confAddr, marie.getCMgr(), members.size(), 0, 0, initialSubject, (accept && send_ics) ? description : NULL);

				LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(pcall);
				if (pcall) {
					LinphoneCallLog * call_log = linphone_call_get_call_log(pcall);
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
					if (accept) {
						linphone_call_accept(pcall);
					} else {
						linphone_call_decline(pcall, LinphoneReasonDeclined);
					}
				}
			}
		}

		if (accept) {
			for (auto mgr : participants) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyReceived, 1, liblinphone_tester_sip_timeout));

				check_conference_info(mgr, confAddr, marie.getCMgr(), 5, 0, 0, initialSubject, (send_ics) ? description : NULL);

				LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(pcall);
				if (pcall) {
					LinphoneCallLog * call_log = linphone_call_get_call_log(pcall);
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
					LinphoneConferenceInfo * call_log_info = linphone_call_log_get_conference_info(call_log);
					if (BC_ASSERT_PTR_NOT_NULL(call_log_info)) {
						BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_conference_info_get_organizer(call_log_info), marie.getCMgr()->identity));
						BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_conference_info_get_uri(call_log_info), confAddr));

						const bctbx_list_t * info_participants = linphone_conference_info_get_participants(call_log_info);

						// Original participants + Marie who joined the conference
						BC_ASSERT_EQUAL(bctbx_list_size(info_participants), 5, size_t, "%zu");
						bctbx_list_free((bctbx_list_t *)info_participants);

						BC_ASSERT_GREATER_STRICT((long long)linphone_conference_info_get_date_time(call_log_info), 0, long long, "%lld");
						const int duration_m = linphone_conference_info_get_duration(call_log_info);
						BC_ASSERT_EQUAL(duration_m, 0, int, "%d");
						if (initialSubject) {
							BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_subject(call_log_info), initialSubject);
						} else {
							BC_ASSERT_PTR_NULL(linphone_conference_info_get_subject(call_log_info));
						}
						if (send_ics) {
							BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_description(call_log_info), description);
						} else {
							BC_ASSERT_PTR_NULL(linphone_conference_info_get_description(call_log_info));
						}
					}
				}
			}

			if (enable_ice) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdating, focus_stat.number_of_LinphoneCallUpdating + 4, liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + ((enable_ice) ? 2 : 1) *static_cast<int>(participants.size() + 1), liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, marie_stat.number_of_LinphoneConferenceStateCreated + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionOutgoingProgress, marie_stat.number_of_LinphoneSubscriptionOutgoingProgress + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive, marie_stat.number_of_LinphoneSubscriptionActive + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyReceived, marie_stat.number_of_NotifyReceived + 1, liblinphone_tester_sip_timeout));

			// Update to add to conference.
			// If ICE is enabled, the addition to a conference may go through a resume of the call
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated, focus_stat.number_of_LinphoneConferenceStateCreated + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived, focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 5, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive, focus_stat.number_of_LinphoneSubscriptionActive + 5, liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added, focus_stat.number_of_participants_added + 5, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added, focus_stat.number_of_participant_devices_added + 5, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined, focus_stat.number_of_participant_devices_joined + 5, liblinphone_tester_sip_timeout));

			LinphoneConference * fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(fconference);

			wait_for_conference_streams({focus,marie,pauline,laure,michelle,lise}, conferenceMgrs, focus.getCMgr(), members, confAddr, enable_video);

			//wait bit more to detect side effect if any
			CoreManagerAssert({focus,marie,pauline,laure,michelle,lise}).waitUntil(chrono::seconds(15),[] {
				return false;
			});

			for (auto mgr : conferenceMgrs) {
				LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
				LinphoneConference * pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
				linphone_address_unref(uri);
				BC_ASSERT_PTR_NOT_NULL(pconference);
				if (pconference) {
					int no_participants = 0;
					if (mgr == focus.getCMgr()) {
						no_participants = 5;
						BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
					} else {
						no_participants = 4;
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
						linphone_video_activation_policy_unref(pol);

						int no_streams_audio = 1;
						int no_streams_video = (enabled) ? 6 : 0;
						int no_streams_text = 0;

						LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
						BC_ASSERT_PTR_NOT_NULL(pcall);
						if (pcall) {
							_linphone_call_check_nb_streams(pcall, no_streams_audio, no_streams_video, no_streams_text);
							const LinphoneCallParams* call_lparams = linphone_call_get_params(pcall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
							const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(pcall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
							const LinphoneCallParams* call_cparams = linphone_call_get_current_params(pcall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
						}

						LinphoneCall * ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
						BC_ASSERT_PTR_NOT_NULL(ccall);
						if (ccall) {
							_linphone_call_check_nb_streams(ccall, no_streams_audio, no_streams_video, no_streams_text);
							const LinphoneCallParams* call_lparams = linphone_call_get_params(ccall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
							const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(ccall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
							const LinphoneCallParams* call_cparams = linphone_call_get_current_params(ccall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
						}
					}
					BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int, "%0d");
					bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
					BC_ASSERT_EQUAL(bctbx_list_size(devices), 5, size_t, "%zu");
					if (devices) {
						bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
					}
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

					LinphoneConference * conference = linphone_core_search_conference_2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(conference);
					if (conference) {
						bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
						for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
							LinphoneParticipantDevice * d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
							if (enable_video) {
								if (linphone_conference_is_me(conference, linphone_participant_device_get_address(d))) {
									BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
								} else {
									BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) == (linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) == LinphoneMediaDirectionSendRecv));
								}
							} else {
								BC_ASSERT_FALSE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
							}
						}

						if (devices) {
							bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
						}
					}
				}
			}

			if (enable_video) {
				LinphoneCall * pauline_call = linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
				BC_ASSERT_PTR_NOT_NULL(pauline_call);

				Address paulineAddr(pauline.getIdentity().asAddress());
				LinphoneCall * focus_call = linphone_core_get_call_by_remote_address2(focus.getLc(), L_GET_C_BACK_PTR(&paulineAddr));
				BC_ASSERT_PTR_NOT_NULL(focus_call);

				LinphoneVideoActivationPolicy * pol = linphone_core_get_video_activation_policy(pauline.getLc());
				bool_t enable = !!!linphone_video_activation_policy_get_automatically_initiate(pol);
				linphone_video_activation_policy_unref(pol);

				LinphoneAddress *paulineUri = linphone_address_new(linphone_core_get_identity(pauline.getLc()));
				LinphoneConference * paulineConference = linphone_core_search_conference(pauline.getLc(), NULL, paulineUri, confAddr, NULL);
				linphone_address_unref(paulineUri);
				BC_ASSERT_PTR_NOT_NULL(paulineConference);

				for (int i = 0; i < 4; i++) {
					set_video_settings_in_conference(focus.getCMgr(), pauline.getCMgr(), members, confAddr, enable, LinphoneMediaDirectionSendRecv, enable, LinphoneMediaDirectionSendRecv);

					if (paulineConference) {
						bctbx_list_t *devices = linphone_conference_get_participant_device_list(paulineConference);
						for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
							LinphoneParticipantDevice * d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
							if (enable) {
								if (linphone_conference_is_me(paulineConference, linphone_participant_device_get_address(d))) {
									BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
								} else {
									BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) == (linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) == LinphoneMediaDirectionSendRecv));
								}
							} else {
								BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) == enable);
							}
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

			focus_stat=focus.getStats();
			for (auto mgr : members) {
				LinphoneCall * call = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
				BC_ASSERT_PTR_NOT_NULL(call);
				if (call) {
					ms_message("%s is terminating call with %s", linphone_core_get_identity(mgr->lc), linphone_core_get_identity(focus.getLc()));
					linphone_call_terminate(call);
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated, 1, liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending, 1, liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated, 1, liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1, liblinphone_tester_sip_timeout));

					LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
					LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
					BC_ASSERT_PTR_NULL(pconference);
					linphone_address_unref(uri);
				}
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd, focus_stat.number_of_LinphoneCallEnd + 5, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased, focus_stat.number_of_LinphoneCallReleased + 5, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated, focus_stat.number_of_LinphoneSubscriptionTerminated + 5, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed, focus_stat.number_of_participants_removed + 5, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed, focus_stat.number_of_participant_devices_removed + 5, liblinphone_tester_sip_timeout));

			BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminationPending, focus_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
			BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminated, focus_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
			BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateDeleted, focus_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

			for (auto mgr : {focus.getCMgr()}) {
				LinphoneConference * pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(pconference);
				if (pconference) {
					BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), 0, int, "%0d");
					bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
					BC_ASSERT_EQUAL(bctbx_list_size(devices), 0, size_t, "%zu");
					if (devices) {
						bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
					}
					BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
				}
			}

			focus_stat=focus.getStats();
			const bctbx_list_t * calls = linphone_core_get_calls(focus.getLc());
			BC_ASSERT_EQUAL(bctbx_list_size(calls), 0, size_t, "%zu");

			// Explicitely terminate conference as those on server are static by default
			if (fconference) {
				linphone_conference_terminate(fconference);
			}
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1, liblinphone_tester_sip_timeout));
		} else {
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd, focus_stat.number_of_LinphoneCallEnd + 4, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased, focus_stat.number_of_LinphoneCallReleased + 4, liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed, focus_stat.number_of_participants_removed + 4, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed, focus_stat.number_of_participant_devices_removed + 4, liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed, 4, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_removed, 4, liblinphone_tester_sip_timeout));

			for (auto mgr : participants) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
			}

			ms_message("%s is terminating call with %s", linphone_core_get_identity(marie.getLc()), linphone_core_get_identity(focus.getLc()));
			LinphoneCall * call = linphone_core_get_call_by_remote_address2(marie.getLc(), focus.getCMgr()->identity);
			linphone_call_terminate(call);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd, focus_stat.number_of_LinphoneCallEnd + 5, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased, focus_stat.number_of_LinphoneCallReleased + 5, liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed, focus_stat.number_of_participants_removed + 5, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed, focus_stat.number_of_participant_devices_removed + 5, liblinphone_tester_sip_timeout));
		}

		for (auto mgr : members) {
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
			check_conference_info(mgr, confAddr, marie.getCMgr(), 5, 0, 0, initialSubject, ((accept && send_ics) || (mgr == marie.getCMgr())) ? description : NULL);
		}

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure,michelle,lise}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void create_simple_conference_dial_out (void) {
	create_conference_dial_out_base(FALSE, LinphoneConferenceLayoutActiveSpeaker, TRUE, FALSE, FALSE, LinphoneConferenceParticipantListTypeClosed, TRUE);
}

static void create_simple_conference_dial_out_and_ics (void) {
	create_conference_dial_out_base(TRUE, LinphoneConferenceLayoutGrid, TRUE, TRUE, TRUE, LinphoneConferenceParticipantListTypeOpen, TRUE);
}

static void create_simple_conference_dial_out_with_calls_declined (void) {
	create_conference_dial_out_base(FALSE, LinphoneConferenceLayoutGrid, TRUE, TRUE, TRUE, LinphoneConferenceParticipantListTypeOpen, FALSE);
}

static void create_simple_conference_dial_out_with_some_calls_declined_base (LinphoneReason reason) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focus.getIdentity().asAddress());
		ClientConference laure("laure_tcp_rc", focus.getIdentity().asAddress());
		ClientConference michelle("michelle_rc", focus.getIdentity().asAddress());
		ClientConference lise("lise_rc", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(lise);

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t * coresList = NULL;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(), lise.getCMgr()}) {
			LinphoneVideoActivationPolicy * pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
			linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
			linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
			linphone_core_set_video_activation_policy(mgr->lc, pol);
			linphone_video_activation_policy_unref(pol);

			linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);
			linphone_core_enable_video_capture(mgr->lc, TRUE);
			linphone_core_enable_video_display(mgr->lc, TRUE);

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, LinphoneConferenceLayoutActiveSpeaker);
			}

			enable_stun_in_core(mgr, TRUE, TRUE);
			linphone_core_manager_wait_for_stun_resolution(mgr);

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), LinphoneConferenceParticipantListTypeOpen);

		stats focus_stat=focus.getStats();
		stats marie_stat=marie.getStats();

		std::list<LinphoneCoreManager *> active_participants{pauline.getCMgr(), michelle.getCMgr()};
		std::list<LinphoneCoreManager *> declining_participants{laure.getCMgr(), lise.getCMgr()};
		std::list<LinphoneCoreManager *> participants = active_participants;
		for (auto mgr : declining_participants) {
			participants.push_back(mgr);
		}

		std::list<LinphoneCoreManager *> all_active_participants = active_participants;
		all_active_participants.push_back(marie.getCMgr());

		std::list<LinphoneCoreManager *> conference_members = all_active_participants;
		conference_members.push_back(focus.getCMgr());

		const char *initialSubject = "Team building hike to the mountain hut";
		const char *description = "Having fun!!!! :-)";

		LinphoneAddress * confAddr = create_conference_on_server(focus, marie, participants, -1, -1, initialSubject, description, FALSE);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallOutgoingInit, marie_stat.number_of_LinphoneCallOutgoingInit + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingInit, focus_stat.number_of_LinphoneCallOutgoingInit + 4, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived, focus_stat.number_of_LinphoneCallIncomingReceived + 1, liblinphone_tester_sip_timeout));

		for (auto mgr : participants) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallIncomingReceived, 1, liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning, marie_stat.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating, marie_stat.number_of_LinphoneCallUpdating + 1, 20000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote, focus_stat.number_of_LinphoneCallUpdatedByRemote + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning, marie_stat.number_of_LinphoneCallStreamsRunning + 2, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, marie_stat.number_of_LinphoneConferenceStateCreated + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionOutgoingProgress, marie_stat.number_of_LinphoneSubscriptionOutgoingProgress + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive, marie_stat.number_of_LinphoneSubscriptionActive + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyReceived, marie_stat.number_of_NotifyReceived + 1, liblinphone_tester_sip_timeout));

		LinphoneConference * oconference = linphone_core_search_conference_2(marie.getLc(), confAddr);
		if (BC_ASSERT_PTR_NOT_NULL(oconference)) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(oconference), 4, int, "%0d");
			bctbx_list_t *devices = linphone_conference_get_participant_device_list(oconference);
			BC_ASSERT_EQUAL(bctbx_list_size(devices), 5, size_t, "%zu");
			if (devices) {
				bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
			}
		}

		if (confAddr) {
			for (auto mgr : participants) {
				check_conference_info(mgr, confAddr, marie.getCMgr(), 5, 0, 0, initialSubject, NULL);

				LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(pcall);
				if (pcall) {
					LinphoneCallLog * call_log = linphone_call_get_call_log(pcall);
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
					if (std::find(active_participants.cbegin(), active_participants.cend(), mgr) != active_participants.cend()) {
						linphone_call_accept(pcall);
					} else {
						linphone_call_decline(pcall, reason);
					}
				}
			}
		}

		for (auto mgr : active_participants) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdatedByRemote, 1, 20000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyReceived, 1, liblinphone_tester_sip_timeout));

			check_conference_info(mgr, confAddr, marie.getCMgr(), 5, 0, 0, initialSubject, NULL);

			LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				LinphoneCallLog * call_log = linphone_call_get_call_log(pcall);
				BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				LinphoneConferenceInfo * call_log_info = linphone_call_log_get_conference_info(call_log);
				if (BC_ASSERT_PTR_NOT_NULL(call_log_info)) {
					BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_conference_info_get_organizer(call_log_info), marie.getCMgr()->identity));
					BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_conference_info_get_uri(call_log_info), confAddr));

					const bctbx_list_t * info_participants = linphone_conference_info_get_participants(call_log_info);

					// Original participants + Marie who joined the conference
					BC_ASSERT_EQUAL(bctbx_list_size(info_participants), 5, size_t, "%zu");
					bctbx_list_free((bctbx_list_t *)info_participants);

					BC_ASSERT_GREATER_STRICT((long long)linphone_conference_info_get_date_time(call_log_info), 0, long long, "%lld");
					const int duration_m = linphone_conference_info_get_duration(call_log_info);
					BC_ASSERT_EQUAL(duration_m, 0, int, "%d");
					if (initialSubject) {
						BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_subject(call_log_info), initialSubject);
					} else {
						BC_ASSERT_PTR_NULL(linphone_conference_info_get_subject(call_log_info));
					}
					BC_ASSERT_PTR_NULL(linphone_conference_info_get_description(call_log_info));
				}
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdating, focus_stat.number_of_LinphoneCallUpdating + static_cast<int>(active_participants.size()), liblinphone_tester_sip_timeout) + 1);

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + 2*static_cast<int>(active_participants.size() + 1), liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated, focus_stat.number_of_LinphoneConferenceStateCreated + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived, focus_stat.number_of_LinphoneSubscriptionIncomingReceived + static_cast<int>(active_participants.size()) + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive, focus_stat.number_of_LinphoneSubscriptionActive + static_cast<int>(active_participants.size())+ 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added, focus_stat.number_of_participants_added + 5, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added, focus_stat.number_of_participant_devices_added + 5, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined, focus_stat.number_of_participant_devices_joined + static_cast<int>(all_active_participants.size()), liblinphone_tester_sip_timeout));

		// Participants that declined the call
		for (auto mgr : declining_participants) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
		}

		if (reason == LinphoneReasonBusy) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallError, focus_stat.number_of_LinphoneCallError + static_cast<int>(declining_participants.size()), liblinphone_tester_sip_timeout));
			BC_ASSERT_FALSE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed, marie_stat.number_of_participants_removed + 1, 3000));
			BC_ASSERT_FALSE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed, focus_stat.number_of_participants_removed + 1, 3000));
		} else {
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd, focus_stat.number_of_LinphoneCallEnd + static_cast<int>(declining_participants.size()), liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed, marie_stat.number_of_participants_removed + 2, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed, focus_stat.number_of_participants_removed + 2, liblinphone_tester_sip_timeout));
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_removed, marie_stat.number_of_participant_devices_removed + 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed, focus_stat.number_of_participant_devices_removed + 2, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased, focus_stat.number_of_LinphoneCallReleased + static_cast<int>(declining_participants.size()), liblinphone_tester_sip_timeout));

		LinphoneConference * fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		wait_for_conference_streams({focus,marie,pauline,laure,michelle,lise}, conference_members, focus.getCMgr(), all_active_participants, confAddr, TRUE);

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure,michelle,lise}).waitUntil(chrono::seconds(15),[] {
			return false;
		});

		for (auto mgr : conference_members) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			linphone_address_unref(uri);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				size_t no_participants = 0;
				if (mgr == focus.getCMgr()) {
					no_participants = all_active_participants.size();
					BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
				} else {
					no_participants = active_participants.size();
					BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
					LinphoneCall * current_call = linphone_core_get_current_call(mgr->lc);
					BC_ASSERT_PTR_NOT_NULL(current_call);
					if (current_call) {
						BC_ASSERT_EQUAL((int)linphone_call_get_state(current_call), (int)LinphoneCallStateStreamsRunning, int, "%0d");
					}
					BC_ASSERT_TRUE(check_ice(mgr, focus.getCMgr(), LinphoneIceStateHostConnection));

					LinphoneVideoActivationPolicy * pol = linphone_core_get_video_activation_policy(mgr->lc);
					bool_t enabled = !!linphone_video_activation_policy_get_automatically_initiate(pol);
					linphone_video_activation_policy_unref(pol);

					int no_streams_audio = 1;
					int no_streams_video = (enabled) ? (static_cast<int>(all_active_participants.size()) + 1) : 0;
					int no_streams_text = 0;

					LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						_linphone_call_check_nb_streams(pcall, no_streams_audio, no_streams_video, no_streams_text);
						const LinphoneCallParams* call_lparams = linphone_call_get_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
						const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
						const LinphoneCallParams* call_cparams = linphone_call_get_current_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
					}

					LinphoneCall * ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
					BC_ASSERT_PTR_NOT_NULL(ccall);
					if (ccall) {
						_linphone_call_check_nb_streams(ccall, no_streams_audio, no_streams_video, no_streams_text);
						const LinphoneCallParams* call_lparams = linphone_call_get_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
						const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
						const LinphoneCallParams* call_cparams = linphone_call_get_current_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
					}
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
				LinphoneParticipant * me = linphone_conference_get_me (pconference);
				BC_ASSERT_TRUE(linphone_participant_is_admin(me) == ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())));
				BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_participant_get_address(me), mgr->identity));
				bctbx_list_t *participants = linphone_conference_get_participant_list(pconference);
				if (reason == LinphoneReasonBusy) {
					no_participants += declining_participants.size();
				}
				BC_ASSERT_EQUAL(bctbx_list_size(participants), no_participants, size_t, "%zu");
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), static_cast<int>(no_participants), int, "%0d");
				for (bctbx_list_t *itp = participants; itp; itp = bctbx_list_next(itp)) {
					LinphoneParticipant * p = (LinphoneParticipant *)bctbx_list_get_data(itp);
					BC_ASSERT_TRUE(linphone_participant_is_admin(p) == linphone_address_weak_equal(linphone_participant_get_address(p), marie.getCMgr()->identity));
				}
				bctbx_list_free_with_data(participants, (void(*)(void *))linphone_participant_unref);

				if (mgr != focus.getCMgr()) {
					check_conference_ssrc(fconference, pconference);
				}

				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), all_active_participants.size(), size_t, "%zu");
				for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					LinphoneParticipantDevice * d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					if (linphone_conference_is_me(pconference, linphone_participant_device_get_address(d))) {
						BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
					} else {
						BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) == (linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) == LinphoneMediaDirectionSendRecv));
					}
				}

				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
			}
		}

		LinphoneCall * pauline_call = linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(pauline_call);

		Address paulineAddr(pauline.getIdentity().asAddress());
		LinphoneCall * focus_call = linphone_core_get_call_by_remote_address2(focus.getLc(), L_GET_C_BACK_PTR(&paulineAddr));
		BC_ASSERT_PTR_NOT_NULL(focus_call);

		LinphoneVideoActivationPolicy * pol = linphone_core_get_video_activation_policy(pauline.getLc());
		bool_t enable = !!!linphone_video_activation_policy_get_automatically_initiate(pol);
		linphone_video_activation_policy_unref(pol);

		LinphoneAddress *paulineUri = linphone_address_new(linphone_core_get_identity(pauline.getLc()));
		LinphoneConference * paulineConference = linphone_core_search_conference(pauline.getLc(), NULL, paulineUri, confAddr, NULL);
		linphone_address_unref(paulineUri);
		BC_ASSERT_PTR_NOT_NULL(paulineConference);

		for (int i = 0; i < 4; i++) {
			set_video_settings_in_conference(focus.getCMgr(), pauline.getCMgr(), all_active_participants, confAddr, enable, LinphoneMediaDirectionSendRecv, enable, LinphoneMediaDirectionSendRecv);

			if (paulineConference) {
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(paulineConference);
				for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					LinphoneParticipantDevice * d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					if (enable) {
						if (linphone_conference_is_me(paulineConference, linphone_participant_device_get_address(d))) {
							BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
						} else {
							BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) == (linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) == LinphoneMediaDirectionSendRecv));
						}
					} else {
						BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) == enable);
					}
				}

				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
			}
			// Wait a little bit
			wait_for_list(coresList, NULL, 0, 1000);

			enable = !enable;
		}

		focus_stat=focus.getStats();
		for (auto mgr : all_active_participants) {
			LinphoneCall * call = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
			BC_ASSERT_PTR_NOT_NULL(call);
			if (call) {
				ms_message("%s is terminating call with %s", linphone_core_get_identity(mgr->lc), linphone_core_get_identity(focus.getLc()));
				linphone_call_terminate(call);
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1, liblinphone_tester_sip_timeout));

				LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
				LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
				BC_ASSERT_PTR_NULL(pconference);
				linphone_address_unref(uri);
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd, focus_stat.number_of_LinphoneCallEnd + static_cast<int>(all_active_participants.size()), liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased, focus_stat.number_of_LinphoneCallReleased + static_cast<int>(all_active_participants.size()), liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated, focus_stat.number_of_LinphoneSubscriptionTerminated + static_cast<int>(all_active_participants.size()), liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed, focus_stat.number_of_participants_removed + static_cast<int>(all_active_participants.size()), liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed, focus_stat.number_of_participant_devices_removed + static_cast<int>(all_active_participants.size()), liblinphone_tester_sip_timeout));

		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminationPending, focus_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminated, focus_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateDeleted, focus_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		BC_ASSERT_EQUAL(focus.getStats().number_of_participant_devices_joined, static_cast<int>(all_active_participants.size()), int, "%d");
		BC_ASSERT_EQUAL(marie.getStats().number_of_participant_devices_joined, static_cast<int>(all_active_participants.size()), int, "%d");

		for (auto mgr : {focus.getCMgr()}) {
			LinphoneConference * pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), static_cast<int>((reason == LinphoneReasonBusy) ? declining_participants.size() : 0), int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 0, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
			}
		}

		focus_stat=focus.getStats();
		const bctbx_list_t * calls = linphone_core_get_calls(focus.getLc());
		BC_ASSERT_EQUAL(bctbx_list_size(calls), 0, size_t, "%zu");

		// Explicitely terminate conference as those on server are static by default
		if (fconference) {
			linphone_conference_terminate(fconference);
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending, 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1, liblinphone_tester_sip_timeout));

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(), lise.getCMgr()}) {
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
			check_conference_info(mgr, confAddr, marie.getCMgr(), 5, 0, 0, initialSubject, (mgr == marie.getCMgr()) ? description : NULL);
		}

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure,michelle,lise}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void create_simple_conference_dial_out_with_some_calls_declined (void) {
	create_simple_conference_dial_out_with_some_calls_declined_base(LinphoneReasonDeclined);
}

static void create_simple_conference_dial_out_with_some_calls_busy (void) {
	create_simple_conference_dial_out_with_some_calls_declined_base(LinphoneReasonBusy);
}

static void create_conference_with_late_participant_addition_base (time_t start_time, int duration, LinphoneConferenceLayout layout, LinphoneConferenceParticipantListType participant_list_type, bool_t accept, bool_t one_addition) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focus.getIdentity().asAddress());
		ClientConference laure("laure_tcp_rc", focus.getIdentity().asAddress());
		ClientConference michelle("michelle_rc", focus.getIdentity().asAddress());
		ClientConference lise("lise_rc", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(lise);

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t * coresList = NULL;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(), lise.getCMgr()}) {
			LinphoneVideoActivationPolicy * pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
			linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
			linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
			linphone_core_set_video_activation_policy(mgr->lc, pol);
			linphone_video_activation_policy_unref(pol);

			linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);
			linphone_core_enable_video_capture(mgr->lc, TRUE);
			linphone_core_enable_video_display(mgr->lc, TRUE);

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, layout);
			}

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), participant_list_type);

		stats focus_stat=focus.getStats();
		stats marie_stat=marie.getStats();

		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr()};
		if (one_addition) {
			participants.push_back(michelle.getCMgr());
		}

		time_t end_time = (duration <= 0) ? -1 : (start_time + duration * 60);
		const char *initialSubject = "Weekly recap";
		const char *description = "What happened in the past week";

		LinphoneAddress * confAddr = create_conference_on_server(focus, marie, participants, start_time, end_time, initialSubject, description, TRUE);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 2, liblinphone_tester_sip_timeout));

		auto members = participants;
		members.push_back(marie.getCMgr());
		auto conferenceMgrs = members;
		conferenceMgrs.push_back(focus.getCMgr());

		if (start_time < 0) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallOutgoingInit, marie_stat.number_of_LinphoneCallOutgoingInit + 1, liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingInit, focus_stat.number_of_LinphoneCallOutgoingInit + static_cast<int>(participants.size()), liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived, focus_stat.number_of_LinphoneCallIncomingReceived + 1, liblinphone_tester_sip_timeout));

			for (auto mgr : participants) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallIncomingReceived, 1, liblinphone_tester_sip_timeout));
			}
		} else if (confAddr) {
			for (auto mgr : members) {
				LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
				linphone_call_params_set_video_direction (new_params, LinphoneMediaDirectionSendRecv);
				if (mgr == laure.getCMgr()) {
					linphone_call_params_enable_mic(new_params, FALSE);
				}
				linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
				linphone_call_params_unref(new_params);
			}

			for (auto mgr : members) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1, liblinphone_tester_sip_timeout));
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning, marie_stat.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning, marie_stat.number_of_LinphoneCallStreamsRunning + 2, liblinphone_tester_sip_timeout));

		LinphoneConference * oconference = linphone_core_search_conference_2(marie.getLc(), confAddr);
		if (BC_ASSERT_PTR_NOT_NULL(oconference)) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(oconference), static_cast<int>(participants.size()), int, "%0d");
			bctbx_list_t *devices = linphone_conference_get_participant_device_list(oconference);
			BC_ASSERT_EQUAL(bctbx_list_size(devices), (participants.size() + 1), size_t, "%zu");
			if (devices) {
				bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
			}
		}

		if (confAddr) {
			for (auto mgr : participants) {
				check_conference_info(mgr, confAddr, marie.getCMgr(), members.size(), start_time, duration, initialSubject, description);

				LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(pcall);
				if (pcall) {
					LinphoneCallLog * call_log = linphone_call_get_call_log(pcall);
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
					linphone_call_accept(pcall);
				}
			}
		}

		for (auto mgr : participants) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyReceived, 1, liblinphone_tester_sip_timeout));

			check_conference_info(mgr, confAddr, marie.getCMgr(), members.size(), start_time, duration, initialSubject, description);

			LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				LinphoneCallLog * call_log = linphone_call_get_call_log(pcall);
				BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				LinphoneConferenceInfo * call_log_info = linphone_call_log_get_conference_info(call_log);
				if (BC_ASSERT_PTR_NOT_NULL(call_log_info)) {
					BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_conference_info_get_organizer(call_log_info), marie.getCMgr()->identity));
					BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_conference_info_get_uri(call_log_info), confAddr));

					const bctbx_list_t * info_participants = linphone_conference_info_get_participants(call_log_info);

					// Original participants + Marie who joined the conference
					BC_ASSERT_EQUAL(bctbx_list_size(info_participants), static_cast<int>(members.size()), size_t, "%zu");
					bctbx_list_free((bctbx_list_t *)info_participants);

					if (start_time > 0) {
						BC_ASSERT_EQUAL((long long)linphone_conference_info_get_date_time(call_log_info), start_time, long long, "%lld");
					}
					const int duration_m = linphone_conference_info_get_duration(call_log_info);
					BC_ASSERT_EQUAL(duration_m, ((duration < 0) ? 0 : duration), int, "%d");
					if (initialSubject) {
						BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_subject(call_log_info), initialSubject);
					} else {
						BC_ASSERT_PTR_NULL(linphone_conference_info_get_subject(call_log_info));
					}
					BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_description(call_log_info), description);
				}
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + static_cast<int>(participants.size() + 1), liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, marie_stat.number_of_LinphoneConferenceStateCreated + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionOutgoingProgress, marie_stat.number_of_LinphoneSubscriptionOutgoingProgress + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive, marie_stat.number_of_LinphoneSubscriptionActive + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyReceived, marie_stat.number_of_NotifyReceived + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated, focus_stat.number_of_LinphoneConferenceStateCreated + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived, focus_stat.number_of_LinphoneSubscriptionIncomingReceived + static_cast<int>(members.size()), liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive, focus_stat.number_of_LinphoneSubscriptionActive + static_cast<int>(members.size()), liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added, focus_stat.number_of_participants_added + static_cast<int>(members.size()), liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added, focus_stat.number_of_participant_devices_added + static_cast<int>(members.size()), liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined, focus_stat.number_of_participant_devices_joined + static_cast<int>(members.size()), liblinphone_tester_sip_timeout));

		LinphoneConference * fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		wait_for_conference_streams({focus,marie,pauline,laure,michelle,lise}, conferenceMgrs, focus.getCMgr(), members, confAddr, TRUE);

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure,michelle,lise}).waitUntil(chrono::seconds(15),[] {
			return false;
		});

		for (auto mgr : conferenceMgrs) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			linphone_address_unref(uri);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				int no_participants = 0;
				if (mgr == focus.getCMgr()) {
					no_participants = static_cast<int>(members.size());
					BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
				} else {
					no_participants = static_cast<int>(participants.size());
					BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
					LinphoneCall * current_call = linphone_core_get_current_call(mgr->lc);
					BC_ASSERT_PTR_NOT_NULL(current_call);
					if (current_call) {
						BC_ASSERT_EQUAL((int)linphone_call_get_state(current_call), (int)LinphoneCallStateStreamsRunning, int, "%0d");
					}

					LinphoneVideoActivationPolicy * pol = linphone_core_get_video_activation_policy(mgr->lc);
					bool_t enabled = !!linphone_video_activation_policy_get_automatically_initiate(pol);
					linphone_video_activation_policy_unref(pol);

					int no_streams_audio = 1;
					int no_streams_video = (enabled) ? (static_cast<int>(members.size()) + 1) : 0;
					int no_streams_text = 0;

					LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						_linphone_call_check_nb_streams(pcall, no_streams_audio, no_streams_video, no_streams_text);
						const LinphoneCallParams* call_lparams = linphone_call_get_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
						const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
						const LinphoneCallParams* call_cparams = linphone_call_get_current_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
					}

					LinphoneCall * ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
					BC_ASSERT_PTR_NOT_NULL(ccall);
					if (ccall) {
						_linphone_call_check_nb_streams(ccall, no_streams_audio, no_streams_video, no_streams_text);
						const LinphoneCallParams* call_lparams = linphone_call_get_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
						const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
						const LinphoneCallParams* call_cparams = linphone_call_get_current_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
					}
				}
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), members.size(), size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
				}
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

				LinphoneConference * conference = linphone_core_search_conference_2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(conference);
				if (conference) {
					bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
					for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
						LinphoneParticipantDevice * d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
						if (linphone_conference_is_me(conference, linphone_participant_device_get_address(d))) {
							BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
						} else {
							BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) == (linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) == LinphoneMediaDirectionSendRecv));
						}
					}

					if (devices) {
						bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
					}
				}
			}
		}

		focus_stat=focus.getStats();
		marie_stat=marie.getStats();
		stats michelle_stat=michelle.getStats();
		stats lise_stat=lise.getStats();
		stats pauline_stat=pauline.getStats();
		stats laure_stat=laure.getStats();

		if (one_addition) {
			linphone_conference_add_participant_2(oconference, lise.getCMgr()->identity);
		} else {
			bctbx_list_t * addresses = NULL;
			addresses = bctbx_list_append(addresses, lise.getCMgr()->identity);
			addresses = bctbx_list_append(addresses, michelle.getCMgr()->identity);
			linphone_conference_add_participants_2(oconference, addresses);
			bctbx_list_free(addresses);
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingInit, focus_stat.number_of_LinphoneCallOutgoingInit + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingProgress, focus_stat.number_of_LinphoneCallOutgoingProgress + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_LinphoneCallIncomingReceived, lise_stat.number_of_LinphoneCallIncomingReceived + 1, liblinphone_tester_sip_timeout));

		if (!one_addition) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingInit, focus_stat.number_of_LinphoneCallOutgoingInit + 2, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingProgress, focus_stat.number_of_LinphoneCallOutgoingProgress + 2, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallIncomingReceived, michelle_stat.number_of_LinphoneCallIncomingReceived + 1, liblinphone_tester_sip_timeout));
		}

		LinphoneCall * lise_call = linphone_core_get_call_by_remote_address2(lise.getLc(), focus.getCMgr()->identity);
		BC_ASSERT_PTR_NOT_NULL(lise_call);

		LinphoneCall * michelle_call = linphone_core_get_call_by_remote_address2(michelle.getLc(), focus.getCMgr()->identity);
		BC_ASSERT_PTR_NOT_NULL(michelle_call);

		int participant_added = ((one_addition) ? 1 : 2);

		if (accept) {
			if (lise_call) {
				linphone_call_accept(lise_call);
			}

			conferenceMgrs.push_back(lise.getCMgr());
			members.push_back(lise.getCMgr());

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + 2, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_LinphoneCallStreamsRunning, lise_stat.number_of_LinphoneCallStreamsRunning + 2, liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_LinphoneConferenceStateCreated, lise_stat.number_of_LinphoneConferenceStateCreated + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_LinphoneSubscriptionOutgoingProgress, lise_stat.number_of_LinphoneSubscriptionOutgoingProgress + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_LinphoneSubscriptionActive, lise_stat.number_of_LinphoneSubscriptionActive + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_NotifyReceived, lise_stat.number_of_NotifyReceived + 1, liblinphone_tester_sip_timeout));

			if (!one_addition) {
				if (michelle_call) {
					linphone_call_accept(michelle_call);
				}

				conferenceMgrs.push_back(michelle.getCMgr());
				members.push_back(michelle.getCMgr());

				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + 4, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallStreamsRunning, michelle_stat.number_of_LinphoneCallStreamsRunning + 2, liblinphone_tester_sip_timeout));

				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateCreated, michelle_stat.number_of_LinphoneConferenceStateCreated + 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneSubscriptionOutgoingProgress, michelle_stat.number_of_LinphoneSubscriptionOutgoingProgress + 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneSubscriptionActive, michelle_stat.number_of_LinphoneSubscriptionActive + 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_NotifyReceived, michelle_stat.number_of_NotifyReceived + 1, liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived, focus_stat.number_of_LinphoneSubscriptionIncomingReceived + participant_added, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive, focus_stat.number_of_LinphoneSubscriptionActive + participant_added, liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added, focus_stat.number_of_participants_added + participant_added, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_added, marie_stat.number_of_participants_added + participant_added, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_added, pauline_stat.number_of_participants_added + participant_added, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participants_added, laure_stat.number_of_participants_added + participant_added, liblinphone_tester_sip_timeout));
			if (one_addition) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participants_added, michelle_stat.number_of_participants_added + 1, liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_alerting, focus_stat.number_of_participant_devices_alerting + participant_added, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_alerting, marie_stat.number_of_participant_devices_alerting + participant_added, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_alerting, pauline_stat.number_of_participant_devices_alerting + participant_added, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_alerting, laure_stat.number_of_participant_devices_alerting + participant_added, liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added, focus_stat.number_of_participant_devices_added + participant_added, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_added, marie_stat.number_of_participant_devices_added + participant_added, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_added, pauline_stat.number_of_participant_devices_added + participant_added, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_added, laure_stat.number_of_participant_devices_added + participant_added, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined, focus_stat.number_of_participant_devices_joined + participant_added, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_joined, marie_stat.number_of_participant_devices_joined + participant_added, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_joined, pauline_stat.number_of_participant_devices_joined + participant_added, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_joined, laure_stat.number_of_participant_devices_joined + participant_added, liblinphone_tester_sip_timeout));
			if (one_addition) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_alerting, michelle_stat.number_of_participant_devices_alerting + 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_added, michelle_stat.number_of_participant_devices_added + 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_joined, michelle_stat.number_of_participant_devices_joined + 1, liblinphone_tester_sip_timeout));
			}
		} else {
			if (lise_call) {
				linphone_call_decline(lise_call, LinphoneReasonDeclined);
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_LinphoneCallEnd, lise_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_LinphoneCallReleased, lise_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));

			if (!one_addition) {
				if (michelle_call) {
					linphone_call_decline(michelle_call, LinphoneReasonDeclined);
				}

				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallEnd, michelle_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallReleased, michelle_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd, focus_stat.number_of_LinphoneCallEnd + participant_added, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased, focus_stat.number_of_LinphoneCallReleased + participant_added, liblinphone_tester_sip_timeout));
		}

		wait_for_conference_streams({focus,marie,pauline,laure,michelle,lise}, conferenceMgrs, focus.getCMgr(), members, confAddr, TRUE);

		LinphoneCall * pauline_call = linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(pauline_call);

		Address paulineAddr(pauline.getIdentity().asAddress());
		LinphoneCall * focus_call = linphone_core_get_call_by_remote_address2(focus.getLc(), L_GET_C_BACK_PTR(&paulineAddr));
		BC_ASSERT_PTR_NOT_NULL(focus_call);

		LinphoneVideoActivationPolicy * pol = linphone_core_get_video_activation_policy(pauline.getLc());
		bool_t enable = !!!linphone_video_activation_policy_get_automatically_initiate(pol);
		linphone_video_activation_policy_unref(pol);

		LinphoneAddress *paulineUri = linphone_address_new(linphone_core_get_identity(pauline.getLc()));
		LinphoneConference * paulineConference = linphone_core_search_conference(pauline.getLc(), NULL, paulineUri, confAddr, NULL);
		linphone_address_unref(paulineUri);
		BC_ASSERT_PTR_NOT_NULL(paulineConference);

		for (int i = 0; i < 4; i++) {
			set_video_settings_in_conference(focus.getCMgr(), pauline.getCMgr(), members, confAddr, enable, LinphoneMediaDirectionSendRecv, enable, LinphoneMediaDirectionSendRecv);

			if (paulineConference) {
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(paulineConference);
				for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					LinphoneParticipantDevice * d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					if (enable) {
						if (linphone_conference_is_me(paulineConference, linphone_participant_device_get_address(d))) {
							BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
						} else {
							BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) == (linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) == LinphoneMediaDirectionSendRecv));
						}
					} else {
						BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) == enable);
					}
				}

				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
			}
			// Wait a little bit
			wait_for_list(coresList, NULL, 0, 1000);

			enable = !enable;
		}

		focus_stat=focus.getStats();
		for (auto mgr : members) {
			LinphoneCall * call = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
			BC_ASSERT_PTR_NOT_NULL(call);
			if (call) {
				ms_message("%s is terminating call with %s", linphone_core_get_identity(mgr->lc), linphone_core_get_identity(focus.getLc()));
				linphone_call_terminate(call);
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1, liblinphone_tester_sip_timeout));

				LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
				LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
				BC_ASSERT_PTR_NULL(pconference);
				linphone_address_unref(uri);
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd, focus_stat.number_of_LinphoneCallEnd + ((accept) ? 5 : 4), liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased, focus_stat.number_of_LinphoneCallReleased + ((accept) ? 5 : 4), liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated, focus_stat.number_of_LinphoneSubscriptionTerminated + ((accept) ? 5 : 4), liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed, focus_stat.number_of_participants_removed + ((accept) ? 5 : 4), liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed, focus_stat.number_of_participant_devices_removed + ((accept) ? 5 : 4), liblinphone_tester_sip_timeout));

		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminationPending, focus_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminated, focus_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateDeleted, focus_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		for (auto mgr : {focus.getCMgr()}) {
			LinphoneConference * pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), 0, int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 0, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
			}
		}

		focus_stat=focus.getStats();
		const bctbx_list_t * calls = linphone_core_get_calls(focus.getLc());
		BC_ASSERT_EQUAL(bctbx_list_size(calls), 0, size_t, "%zu");

		// Explicitely terminate conference as those on server are static by default
		if (fconference) {
			linphone_conference_terminate(fconference);
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending, 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1, liblinphone_tester_sip_timeout));

		for (auto mgr : members) {
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

			check_conference_info(mgr, confAddr, marie.getCMgr(), 5, 0, 0, initialSubject, ((!one_addition && (mgr == michelle.getCMgr())) || (mgr == lise.getCMgr())) ? NULL : description);
		}

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure,michelle,lise}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void create_conference_with_late_participant_addition (void) {
	create_conference_with_late_participant_addition_base(ms_time(NULL), -1, LinphoneConferenceLayoutGrid, LinphoneConferenceParticipantListTypeClosed, TRUE, TRUE);
}

static void create_conference_with_late_participant_addition_declined (void) {
	create_conference_with_late_participant_addition_base(ms_time(NULL), -1, LinphoneConferenceLayoutActiveSpeaker, LinphoneConferenceParticipantListTypeClosed, FALSE, TRUE);
}

static void create_simple_conference_dial_out_with_late_participant_addition (void) {
	create_conference_with_late_participant_addition_base(-1, -1, LinphoneConferenceLayoutActiveSpeaker, LinphoneConferenceParticipantListTypeOpen, TRUE, TRUE);
}

static void create_simple_conference_dial_out_with_many_late_participant_additions (void) {
	create_conference_with_late_participant_addition_base(-1, -1, LinphoneConferenceLayoutGrid, LinphoneConferenceParticipantListTypeOpen, TRUE, FALSE);
}

static void abort_call_to_ice_conference (void) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focus.getIdentity().asAddress());
		ClientConference laure("laure_tcp_rc", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);

		setup_conference_info_cbs(marie.getCMgr());

		const LinphoneConferenceLayout layout = LinphoneConferenceLayoutGrid;

		bctbx_list_t * coresList = NULL;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, layout);
			}

			enable_stun_in_core(mgr, TRUE, TRUE);
			linphone_core_manager_wait_for_stun_resolution(mgr);
			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);

		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr()};

		time_t start_time = ms_time(NULL);
		int duration = -1;
		time_t end_time = (duration <= 0) ? -1 : (start_time + duration * 60);
		const char *initialSubject = "Test aborted ICE call";
		const char *description = "Grenoble";

		stats focus_stat=focus.getStats();

		LinphoneAddress * confAddr = create_conference_on_server(focus, marie, participants, start_time, end_time, initialSubject, description, TRUE);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 2, liblinphone_tester_sip_timeout));

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			LinphoneCall * call = linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
			BC_ASSERT_PTR_NOT_NULL(call);
			linphone_call_params_unref(new_params);
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1, liblinphone_tester_sip_timeout));
			if (call) {
				linphone_call_terminate(call);
			}
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived, 3, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd, 3, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased, 3, liblinphone_tester_sip_timeout));

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			reset_counters(&mgr->stat);
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
		}

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1, liblinphone_tester_sip_timeout));
			int no_streams_running = 3;
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating, (no_streams_running - 1), liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, no_streams_running, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyReceived, 1, liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived, focus_stat.number_of_LinphoneCallIncomingReceived + 3, liblinphone_tester_sip_timeout));
		int focus_no_streams_running = 9;
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote, focus_stat.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running - 3), liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated, focus_stat.number_of_LinphoneConferenceStateCreated + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived, focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 3, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive, focus_stat.number_of_LinphoneSubscriptionActive + 3, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added, focus_stat.number_of_participants_added + 3, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added, focus_stat.number_of_participant_devices_added + 3, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined, focus_stat.number_of_participant_devices_joined + 3, liblinphone_tester_sip_timeout));

		LinphoneConference * fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
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
					BC_ASSERT_TRUE(check_ice(mgr, focus.getCMgr(), LinphoneIceStateHostConnection));

					LinphoneVideoActivationPolicy * pol = linphone_core_get_video_activation_policy(mgr->lc);
					bool_t enabled = !!linphone_video_activation_policy_get_automatically_initiate(pol);
					linphone_video_activation_policy_unref(pol);
					LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						const LinphoneCallParams* call_lparams = linphone_call_get_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
						const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
						const LinphoneCallParams* call_cparams = linphone_call_get_current_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
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
				}
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 3, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
				}
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

		std::list <LinphoneCoreManager*> mgrsToRemove {pauline.getCMgr()};
		mgrsToRemove.push_back(laure.getCMgr());

		stats marie_stat=marie.getStats();

		for (auto mgr : mgrsToRemove) {
			LinphoneCall * call = linphone_core_get_current_call(mgr->lc);
			BC_ASSERT_PTR_NOT_NULL(call);
			if (call) {
				linphone_call_terminate(call);
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1, liblinphone_tester_sip_timeout));

				LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
				LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
				BC_ASSERT_PTR_NULL(pconference);
				linphone_address_unref(uri);
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed, focus_stat.number_of_participants_removed + 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed, focus_stat.number_of_participant_devices_removed + 2, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed, marie_stat.number_of_participants_removed + 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_removed, marie_stat.number_of_participant_devices_removed + 2, liblinphone_tester_sip_timeout));

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
			LinphoneConference * pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), ((mgr == focus.getCMgr()) ? 1 : 0), int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 1, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
			}
		}

		const bctbx_list_t * calls = linphone_core_get_calls(marie.getLc());
		BC_ASSERT_EQUAL(bctbx_list_size(calls), 1, size_t, "%zu");

		LinphoneCall * call = linphone_core_get_call_by_remote_address2(marie.getLc(), focus.getCMgr()->identity);
		BC_ASSERT_PTR_NOT_NULL(call);
		if (call) {
			linphone_call_terminate(call);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionTerminated, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminationPending, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminated, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateDeleted, 1, liblinphone_tester_sip_timeout));

			// Explicitely terminate conference as those on server are static by default
			LinphoneConference * pconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				linphone_conference_terminate(pconference);
			}
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1, liblinphone_tester_sip_timeout));
		}

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			const bctbx_list_t* call_logs = linphone_core_get_call_logs(mgr->lc);
			BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(call_logs), 2, unsigned int, "%u");

			bctbx_list_t * mgr_focus_call_log = linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
			if (mgr_focus_call_log) {
				BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(mgr_focus_call_log), 2, unsigned int, "%u");
				for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
					LinphoneCallLog *call_log=(LinphoneCallLog *)it->data;
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				}
				bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func) linphone_call_log_unref);
			}
		}

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void edit_simple_conference_base (bool_t from_organizer, bool_t use_default_account, bool_t enable_bundle_mode) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focus.getIdentity().asAddress());
		ClientConference laure("laure_tcp_rc", focus.getIdentity().asAddress());
		ClientConference michelle("michelle_rc", focus.getIdentity().asAddress());

		LinphoneCoreManager * manager_editing = (from_organizer) ? marie.getCMgr() : laure.getCMgr();
		linphone_core_enable_rtp_bundle(manager_editing->lc, enable_bundle_mode);
		int default_index = linphone_config_get_int(linphone_core_get_config(manager_editing->lc), "sip", "default_proxy", 0);
		LinphoneAccountParams *params = linphone_account_params_new_with_config(manager_editing->lc, default_index);
		LinphoneAddress * alternative_address = linphone_address_new("sip:toto@sip.linphone.org");
		linphone_account_params_set_identity_address(params, alternative_address);
		LinphoneAccount *new_account = linphone_account_new(manager_editing->lc, params);
		linphone_core_add_account(manager_editing->lc, new_account);
		linphone_account_params_unref(params);
		linphone_account_unref(new_account);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);

		setup_conference_info_cbs(marie.getCMgr());
		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);

		bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, laure.getLc());
		coresList = bctbx_list_append(coresList, michelle.getLc());

		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr()};

		time_t start_time = time(NULL) + 600; // Start in 10 minutes
		int duration = 60; // minutes
		time_t end_time = (duration <= 0) ? -1 : (start_time + duration * 60);
		const char *initialSubject = "Test characters: <S-F12><S-F11><S-F6> £$%§";
		const char *description = "Testing characters";

		LinphoneAddress * confAddr = create_conference_on_server(focus, marie, participants, start_time, end_time, initialSubject, description, TRUE);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 2, liblinphone_tester_sip_timeout));

		const char *subject = "Test characters: <S-F12><S-F11><S-F6> £$%§ (+Michelle)";
		const char *description2 = "Testing characters (+Michelle)";

		stats manager_editing_stat=manager_editing->stat;
		LinphoneAccount *editing_account = NULL;
		if (use_default_account) {
			editing_account = linphone_core_get_default_account(manager_editing->lc);
		} else {
			editing_account = linphone_core_lookup_known_account(manager_editing->lc, alternative_address);
		}
		BC_ASSERT_PTR_NOT_NULL(editing_account);
		if (editing_account) {
			LinphoneAccountParams* account_params = linphone_account_params_clone(linphone_account_get_params(editing_account));
			linphone_account_params_enable_rtp_bundle(account_params, enable_bundle_mode);
			linphone_account_set_params(editing_account, account_params);
			linphone_account_params_unref(account_params);
		}

		LinphoneConferenceInfo * conf_info = linphone_core_find_conference_information_from_uri(manager_editing->lc, confAddr);
		linphone_conference_info_add_participant(conf_info, michelle.getCMgr()->identity);
		linphone_conference_info_set_subject(conf_info, subject);
		linphone_conference_info_set_description(conf_info, description2);

		for (int attempt = 0; attempt < 3; attempt++) {
			char * conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("<unknown>");
			ms_message("%s is trying to update conference %s - attempt %0d", linphone_core_get_identity(manager_editing->lc), conference_address_str, attempt);
			ms_free(conference_address_str);

			stats focus_stat=focus.getStats();

			std::list<stats> participant_stats;
			for (auto mgr : {focus.getCMgr(), marie.getCMgr(), laure.getCMgr(), pauline.getCMgr(), michelle.getCMgr()}) {
				participant_stats.push_back(mgr->stat);
			}

			LinphoneConferenceScheduler *conference_scheduler = linphone_core_create_conference_scheduler(manager_editing->lc);
			linphone_conference_scheduler_set_account(conference_scheduler, editing_account);
			LinphoneConferenceSchedulerCbs *cbs = linphone_factory_create_conference_scheduler_cbs(linphone_factory_get());
			linphone_conference_scheduler_cbs_set_state_changed(cbs, conference_scheduler_state_changed);
			linphone_conference_scheduler_cbs_set_invitations_sent(cbs, conference_scheduler_invitations_sent);
			linphone_conference_scheduler_add_callbacks(conference_scheduler, cbs);
			linphone_conference_scheduler_cbs_unref(cbs);
			linphone_conference_scheduler_set_info(conference_scheduler, conf_info);

			if (use_default_account) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &manager_editing->stat.number_of_ConferenceSchedulerStateUpdating, manager_editing_stat.number_of_ConferenceSchedulerStateUpdating + 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &manager_editing->stat.number_of_ConferenceSchedulerStateReady, manager_editing_stat.number_of_ConferenceSchedulerStateReady + 1, liblinphone_tester_sip_timeout));

				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived, focus_stat.number_of_LinphoneCallIncomingReceived + 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd, focus_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased, focus_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));

				LinphoneChatRoomParams *chat_room_params = linphone_core_create_default_chat_room_params(manager_editing->lc);
				linphone_chat_room_params_set_backend(chat_room_params, LinphoneChatRoomBackendBasic);
				linphone_conference_scheduler_send_invitations(conference_scheduler, chat_room_params);
				linphone_chat_room_params_unref(chat_room_params);

				BC_ASSERT_TRUE(wait_for_list(coresList, &manager_editing->stat.number_of_ConferenceSchedulerInvitationsSent, manager_editing_stat.number_of_ConferenceSchedulerInvitationsSent + 1, liblinphone_tester_sip_timeout));

				for (auto mgr : {focus.getCMgr(), marie.getCMgr(), laure.getCMgr(), pauline.getCMgr(), michelle.getCMgr()}) {
					auto old_stats = participant_stats.front();
					if ((mgr != focus.getCMgr()) && (mgr != manager_editing)) {
						BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceived, old_stats.number_of_LinphoneMessageReceived + 1, liblinphone_tester_sip_timeout));
						if (!linphone_core_conference_ics_in_message_body_enabled(manager_editing->lc)) {
							BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceivedWithFile, old_stats.number_of_LinphoneMessageReceivedWithFile + 1, liblinphone_tester_sip_timeout));
						}

						BC_ASSERT_PTR_NOT_NULL(mgr->stat.last_received_chat_message);
						if (mgr->stat.last_received_chat_message != NULL) {
							const string expected = ContentType::Icalendar.getMediaType();
							BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(mgr->stat.last_received_chat_message), expected.c_str());
						}

						bctbx_list_t *participant_chat_room_participants = bctbx_list_append(NULL, manager_editing->identity);
						LinphoneChatRoom *pcr = linphone_core_search_chat_room(mgr->lc, NULL, mgr->identity, NULL, participant_chat_room_participants);
						bctbx_list_free(participant_chat_room_participants);
						BC_ASSERT_PTR_NOT_NULL(pcr);

						bctbx_list_t *chat_room_participants = bctbx_list_append(NULL, mgr->identity);
						LinphoneChatRoom *cr = linphone_core_search_chat_room(manager_editing->lc, NULL, manager_editing->identity, NULL, chat_room_participants);
						bctbx_list_free(chat_room_participants);
						BC_ASSERT_PTR_NOT_NULL(cr);

						if (cr) {
							LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(cr);
							linphone_chat_room_unref(cr);

							const bctbx_list_t* original_contents = linphone_chat_message_get_contents(msg);
							BC_ASSERT_EQUAL((int)bctbx_list_size(original_contents), 1, int, "%d");
							LinphoneContent *original_content = (LinphoneContent *) bctbx_list_get_data(original_contents);
							if (BC_ASSERT_PTR_NOT_NULL(original_content)) {
								LinphoneConferenceInfo *conf_info_from_original_content = linphone_factory_create_conference_info_from_icalendar_content(linphone_factory_get(), original_content);
								if (BC_ASSERT_PTR_NOT_NULL(conf_info_from_original_content)) {
									BC_ASSERT_TRUE(linphone_address_weak_equal(marie.getCMgr()->identity, linphone_conference_info_get_organizer(conf_info_from_original_content)));
									BC_ASSERT_TRUE(linphone_address_weak_equal(confAddr, linphone_conference_info_get_uri(conf_info_from_original_content)));

									const bctbx_list_t * ics_participants = linphone_conference_info_get_participants(conf_info_from_original_content);
									BC_ASSERT_EQUAL(bctbx_list_size(ics_participants), 3, size_t, "%zu");
									bctbx_list_free((bctbx_list_t *)ics_participants);

									if (start_time > 0) {
										BC_ASSERT_EQUAL((long long)linphone_conference_info_get_date_time(conf_info_from_original_content), (long long)start_time, long long, "%lld");
										if (end_time > 0) {
											const int duration_m = linphone_conference_info_get_duration(conf_info_from_original_content);
											const int duration_s = duration_m * 60;
											BC_ASSERT_EQUAL(duration_s, (int)(end_time - start_time), int, "%d");
											BC_ASSERT_EQUAL(duration_m, duration, int, "%d");
										}
									}
									if (subject) {
										BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_subject(conf_info_from_original_content), subject);
									} else {
										BC_ASSERT_PTR_NULL(linphone_conference_info_get_subject(conf_info_from_original_content));
									}
									if (description2) {
										BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_description(conf_info_from_original_content), description2);
									} else {
										BC_ASSERT_PTR_NULL(linphone_conference_info_get_description(conf_info_from_original_content));
									}
									linphone_conference_info_unref(conf_info_from_original_content);
								}
							}
							linphone_chat_message_unref(msg);
						}
					}
					participant_stats.pop_front();
				}
			} else {
				BC_ASSERT_TRUE(wait_for_list(coresList, &manager_editing->stat.number_of_ConferenceSchedulerStateError, manager_editing_stat.number_of_ConferenceSchedulerStateError + 1, liblinphone_tester_sip_timeout));
			}
			linphone_conference_scheduler_unref(conference_scheduler);

			for (auto mgr : {focus.getCMgr(), marie.getCMgr(), laure.getCMgr(), pauline.getCMgr(), michelle.getCMgr()}) {
				LinphoneConferenceInfo * info = linphone_core_find_conference_information_from_uri(mgr->lc, confAddr);
				if (!use_default_account && (mgr == michelle.getCMgr())) {
					BC_ASSERT_PTR_NULL(info);
				} else if (BC_ASSERT_PTR_NOT_NULL(info)) {

					const char * exp_subject = NULL;
					if (use_default_account) {
						exp_subject = subject;
					} else {
						exp_subject = initialSubject;
					}

					const char * exp_description = NULL;
					if (mgr != focus.getCMgr()) {
						if (use_default_account) {
							exp_description = description2;
						} else {
							exp_description = description;
						}
					}
					check_conference_info(mgr, confAddr, marie.getCMgr(), (use_default_account) ? 3 : 2, start_time, duration, exp_subject, exp_description);
				}
				if (info) {
					linphone_conference_info_unref(info);
				}
			}
		}
		linphone_conference_info_unref(conf_info);
		linphone_address_unref(alternative_address);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void organizer_edits_simple_conference (void) {
	edit_simple_conference_base(TRUE, TRUE, FALSE);
}

static void organizer_edits_simple_conference_using_different_account (void) {
	edit_simple_conference_base(TRUE, FALSE, TRUE);
}

static void participant_edits_simple_conference (void) {
	edit_simple_conference_base(FALSE, TRUE, TRUE);
}

static void participant_edits_simple_conference_using_different_account (void) {
	edit_simple_conference_base(FALSE, FALSE, FALSE);
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

		LinphoneAddress* confAddr = create_conference_on_server(focus, marie, participants, start_time, end_time, initialSubject, description, TRUE);
		BC_ASSERT_PTR_NOT_NULL(confAddr);
		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 2, liblinphone_tester_sip_timeout));

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
		}

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
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
		linphone_core_set_default_conference_layout(focus.getLc(), LinphoneConferenceLayoutGrid);

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
		LinphoneAddress * confAddr1 = create_conference_on_server(focus, marie, participants1, start_time1, end_time1, subject1, description1, TRUE);
		BC_ASSERT_PTR_NOT_NULL(confAddr1);
		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 2, liblinphone_tester_sip_timeout));

		time_t start_time2 = ms_time(NULL);
		time_t end_time2 = start_time2 + 600;
		const char *subject2 = "All Hands Q3 FY2021 - Attendance Mandatory";
		const char *description2 = "Financial result - Internal only - Strictly confidential";
		LinphoneAddress * confAddr2 = NULL;
		if (same_organizer) {
			std::list<LinphoneCoreManager *> participants2{pauline.getCMgr(), michelle.getCMgr()};
			confAddr2 = create_conference_on_server(focus, marie, participants2, start_time2, end_time2, subject2, description2, TRUE);

			// Chat room creation to send ICS
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 3, liblinphone_tester_sip_timeout));

		} else {
			std::list<LinphoneCoreManager *> participants2{pauline.getCMgr(), marie.getCMgr()};
			confAddr2 = create_conference_on_server(focus, michelle, participants2, start_time2, end_time2, subject2, description2, TRUE);

			// Chat room creation to send ICS
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateCreated, 2, liblinphone_tester_sip_timeout));
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
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 2, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1, liblinphone_tester_sip_timeout));
			LinphoneCall * currentCall = linphone_core_get_current_call(mgr->lc);
			BC_ASSERT_PTR_NOT_NULL(currentCall);
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr1, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			linphone_address_unref(uri);
			if (currentCall && pconference) {
				BC_ASSERT_PTR_EQUAL(linphone_call_get_conference(currentCall), pconference);
			}
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyReceived, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived, focus_stat.number_of_LinphoneCallIncomingReceived + 3, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote, focus_stat.number_of_LinphoneCallUpdatedByRemote + 3, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + 6, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated, focus_stat.number_of_LinphoneConferenceStateCreated + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived, focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 3, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive, focus_stat.number_of_LinphoneSubscriptionActive + 3, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added, focus_stat.number_of_participants_added + 3, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added, focus_stat.number_of_participant_devices_added + 3, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined, focus_stat.number_of_participant_devices_joined + 3, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + 3, liblinphone_tester_sip_timeout));

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
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 3, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
				}
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
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 2, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 3, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallPaused, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, ((mgr == marie.getCMgr()) ? 4 : 3), liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 2, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 2, liblinphone_tester_sip_timeout));
			LinphoneCall * currentCall = linphone_core_get_current_call(mgr->lc);
			BC_ASSERT_PTR_NOT_NULL(currentCall);
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr2, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			linphone_address_unref(uri);
			if (currentCall && pconference) {
				BC_ASSERT_PTR_EQUAL(linphone_call_get_conference(currentCall), pconference);
			}
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating, 2, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 4, liblinphone_tester_sip_timeout));
		}

		for (auto mgr : {michelle.getCMgr()}) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, ((same_organizer) ? 2 : 3), liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1, liblinphone_tester_sip_timeout));
			LinphoneCall * currentCall = linphone_core_get_current_call(mgr->lc);
			BC_ASSERT_PTR_NOT_NULL(currentCall);
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr2, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			linphone_address_unref(uri);
			if (currentCall && pconference) {
				BC_ASSERT_PTR_EQUAL(linphone_call_get_conference(currentCall), pconference);
			}
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyReceived, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 2, liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived, focus_stat.number_of_LinphoneCallIncomingReceived + 6, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote, focus_stat.number_of_LinphoneCallUpdatedByRemote + 6, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + 12, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated, focus_stat.number_of_LinphoneConferenceStateCreated + 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived, focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 6, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive, focus_stat.number_of_LinphoneSubscriptionActive + 6, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added, focus_stat.number_of_participants_added + 6, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added, focus_stat.number_of_participant_devices_added + 6, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined, focus_stat.number_of_participant_devices_joined + 6, liblinphone_tester_sip_timeout));

		// Marie and Pauline leave conference1
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_on_hold, focus_stat.number_of_participant_devices_on_hold + 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_media_capability_changed, 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_on_hold, 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_media_capability_changed, 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_on_hold, 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_media_capability_changed, 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_on_hold, 1, liblinphone_tester_sip_timeout));

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
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				if (mgr == focus.getCMgr()) {
					BC_ASSERT_EQUAL(bctbx_list_size(devices), 3, size_t, "%zu");
					BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), 3, int, "%0d");
				} else {
					BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), 2, int, "%0d");
					if (mgr == laure.getCMgr()) {
						BC_ASSERT_EQUAL(bctbx_list_size(devices), 3, size_t, "%zu");
						BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
					} else {
						BC_ASSERT_EQUAL(bctbx_list_size(devices), 2, size_t, "%zu");
						BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
					}
				}
				if (devices) {
					bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
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
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 3, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
				}
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
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1, liblinphone_tester_sip_timeout));

				LinphoneConference * pconference1 = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr2, NULL);
				BC_ASSERT_PTR_NULL(pconference1);
			}
			linphone_address_unref(uri);
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed, focus_stat.number_of_participants_removed + 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed, focus_stat.number_of_participant_devices_removed + 2, liblinphone_tester_sip_timeout));
 
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_removed, pauline_stat.number_of_participants_removed + 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_removed, pauline_stat.number_of_participant_devices_removed + 2, liblinphone_tester_sip_timeout));

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
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 1, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
				}
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

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallPaused, pauline_stat.number_of_LinphoneCallPaused + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning, pauline_stat.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning, marie_stat.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneConferenceStateTerminationPending, pauline_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneConferenceStateTerminated, pauline_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneConferenceStateDeleted, pauline_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminationPending, focus_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminated, focus_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateDeleted, focus_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		// Pauline leaves conference2
		// Pauline and Marie enter conference1
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_on_hold, focus_stat.number_of_participant_devices_on_hold + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined, focus_stat.number_of_participant_devices_joined + 2, liblinphone_tester_sip_timeout));

		for (auto mgr : {focus.getCMgr(), pauline.getCMgr()}) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr2, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			linphone_address_unref(uri);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), ((mgr == focus.getCMgr()) ? 1 : 0), int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), ((mgr == focus.getCMgr()) ? 1 : 0), size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
				}
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
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd, focus_stat.number_of_LinphoneCallEnd + cnt, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased, focus_stat.number_of_LinphoneCallReleased + cnt, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed, focus_stat.number_of_participants_removed + cnt, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed, focus_stat.number_of_participant_devices_removed + cnt, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed, marie_stat.number_of_participants_removed + cnt, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_removed, marie_stat.number_of_participant_devices_removed + cnt, liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallEnd, laure_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallReleased, laure_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallEnd, pauline_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallReleased, pauline_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneSubscriptionTerminated, laure_stat.number_of_LinphoneSubscriptionTerminated + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateTerminationPending, laure_stat.number_of_LinphoneConferenceStateTerminationPending + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateTerminated, laure_stat.number_of_LinphoneConferenceStateTerminated + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateDeleted, laure_stat.number_of_LinphoneConferenceStateDeleted + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneSubscriptionTerminated, pauline_stat.number_of_LinphoneSubscriptionTerminated + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateTerminationPending, pauline_stat.number_of_LinphoneConferenceStateTerminationPending + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateTerminated, pauline_stat.number_of_LinphoneConferenceStateTerminated + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateDeleted, pauline_stat.number_of_LinphoneConferenceStateDeleted + 1, liblinphone_tester_sip_timeout));

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
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning, pauline_stat.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneConferenceStateTerminationPending, pauline_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneConferenceStateTerminated, pauline_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneConferenceStateDeleted, pauline_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		// Pauline enters conference2
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined, focus_stat.number_of_participant_devices_joined + 1, liblinphone_tester_sip_timeout));

		for (auto mgr : {focus.getCMgr(), pauline.getCMgr()}) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr2, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			linphone_address_unref(uri);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), ((mgr == focus.getCMgr()) ? 1 : 0), int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 1, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
				}
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

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallEnd, pauline_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallReleased, pauline_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneSubscriptionTerminated, pauline_stat.number_of_LinphoneSubscriptionTerminated + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateTerminationPending, pauline_stat.number_of_LinphoneConferenceStateTerminationPending + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateTerminated, pauline_stat.number_of_LinphoneConferenceStateTerminated + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateDeleted, pauline_stat.number_of_LinphoneConferenceStateDeleted + 1, liblinphone_tester_sip_timeout));

		// Explicitely terminate conference as those on server are static by default
		LinphoneConference * conference2 = linphone_core_search_conference(focus.getLc(), NULL, focusUri, confAddr2, NULL);
		linphone_conference_terminate(conference2);
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending, focus_stat.number_of_LinphoneConferenceStateTerminationPending + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, focus_stat.number_of_LinphoneConferenceStateTerminated + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, focus_stat.number_of_LinphoneConferenceStateDeleted + 1, liblinphone_tester_sip_timeout));

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

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallEnd, marie_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallReleased, marie_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionTerminated, marie_stat.number_of_LinphoneSubscriptionTerminated + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminationPending, marie_stat.number_of_LinphoneConferenceStateTerminationPending + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminated, marie_stat.number_of_LinphoneConferenceStateTerminated + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateDeleted, marie_stat.number_of_LinphoneConferenceStateDeleted + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd, focus_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased, focus_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));

		// Explicitely terminate conference as those on server are static by default
		linphone_conference_terminate(conference1);
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending, focus_stat.number_of_LinphoneConferenceStateTerminationPending + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, focus_stat.number_of_LinphoneConferenceStateTerminated + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, focus_stat.number_of_LinphoneConferenceStateDeleted + 1, liblinphone_tester_sip_timeout));

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

static void create_simple_conference_merging_calls_base(bool_t enable_ice, LinphoneConferenceLayout layout, bool_t toggle_video, bool_t toggle_all_mananger_video, bool_t change_layout) {
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

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			if (toggle_video) {
				LinphoneVideoActivationPolicy * pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
				linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
				linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
				linphone_core_set_video_activation_policy(mgr->lc, pol);
				linphone_video_activation_policy_unref(pol);

				linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);
				linphone_core_enable_video_capture(mgr->lc, TRUE);
				linphone_core_enable_video_display(mgr->lc, TRUE);
			}

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, layout);
			}
		}

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
		//linphone_call_set_microphone_muted (pauline_called_by_marie, TRUE);
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

		std::list<stats> participant_stats;
		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			participant_stats.push_back(mgr->stat);
		}

		if (conf) {
			const bctbx_list_t * calls = linphone_core_get_calls(marie.getLc());
			for (const bctbx_list_t *it = calls; it; it = bctbx_list_next(it)) {
				LinphoneCall *call=(LinphoneCall *)it->data;
				linphone_conference_add_participant(conf, call);
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList,&marie.getStats().number_of_LinphoneConferenceStateCreationPending, 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList,&marie.getStats().number_of_LinphoneConferenceStateCreated, 1, 20000));

		BC_ASSERT_TRUE(wait_for_list(coresList,&marie.getStats().number_of_participants_added, 2,liblinphone_tester_sip_timeout));

		LinphoneAddress *confAddr = conf ? linphone_address_clone(linphone_conference_get_conference_address(conf)) : NULL;
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		int counter = 0;
		for (auto mgr : {pauline.getCMgr(), laure.getCMgr()}) {
			counter++;
			auto old_stats = participant_stats.front();
			BC_ASSERT_TRUE(wait_for_list(coresList,&mgr->stat.number_of_LinphoneConferenceStateCreationPending, old_stats.number_of_LinphoneConferenceStateCreationPending + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,&mgr->stat.number_of_LinphoneConferenceStateCreated, old_stats.number_of_LinphoneConferenceStateCreated + 1, liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList,&mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, old_stats.number_of_LinphoneSubscriptionOutgoingProgress + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,&mgr->stat.number_of_LinphoneSubscriptionActive, old_stats.number_of_LinphoneSubscriptionActive + 1, liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList,&focus.getStats().number_of_LinphoneCallStreamsRunning,counter + 1,liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,&marie.getStats().number_of_LinphoneTransferCallConnected,counter,liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,&mgr->stat.number_of_LinphoneCallStreamsRunning,old_stats.number_of_LinphoneCallStreamsRunning + 1,liblinphone_tester_sip_timeout));

			// End of call between conference and participant
			BC_ASSERT_TRUE(wait_for_list(coresList,&marie.getStats().number_of_LinphoneCallEnd,counter,liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,&mgr->stat.number_of_LinphoneCallEnd,old_stats.number_of_LinphoneCallEnd + 1,liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList,&marie.getStats().number_of_LinphoneCallReleased,counter,liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,&mgr->stat.number_of_LinphoneCallReleased,old_stats.number_of_LinphoneCallReleased + 1,liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList,&mgr->stat.number_of_NotifyReceived, 1,liblinphone_tester_sip_timeout));
			participant_stats.pop_front();
		}

		BC_ASSERT_TRUE(wait_for_list(coresList,&focus.getStats().number_of_participants_added, 3,liblinphone_tester_sip_timeout));

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
			linphone_address_unref(uri);
			BC_ASSERT_PTR_NOT_NULL(pconference);

			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), (mgr == focus.getCMgr()) ? 3 : 2, int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 3, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
			}

			if (mgr != focus.getCMgr()) {
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
					//BC_ASSERT_TRUE(linphone_call_get_microphone_muted(participant_call) == (mgr == pauline.getCMgr()));
				}
			}

			if (confAddr) {
				check_conference_info(mgr, confAddr, marie.getCMgr(), 3, 0, 0, initialSubject, NULL);
			}
		}

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		participant_stats.clear();
		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			participant_stats.push_back(mgr->stat);
		}

		stats focus_stat=focus.getStats();
		stats pauline_stat=pauline.getStats();
		stats laure_stat=laure.getStats();
		const char *newSubject = "Let's go drink a beer";
		linphone_conference_set_subject(conf, newSubject);

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_subject_changed, focus_stat.number_of_subject_changed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_subject_changed, pauline_stat.number_of_subject_changed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_subject_changed, laure_stat.number_of_subject_changed + 1, liblinphone_tester_sip_timeout));

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			auto old_stats = participant_stats.front();
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_subject_changed, old_stats.number_of_subject_changed + 1, liblinphone_tester_sip_timeout));
			if (confAddr) {
				check_conference_info(mgr, confAddr, marie.getCMgr(), 3, 0, 0, newSubject, NULL);
			}
			participant_stats.pop_front();
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

		focus_stat=focus.getStats();
		pauline_stat=pauline.getStats();
		stats marie_stat=marie.getStats();

		// Laure's core suddenly stops
		ms_message("%s core suddently loses network and restarts", linphone_core_get_identity(laure.getLc()));
		linphone_core_set_network_reachable(laure.getLc(), FALSE);
		coresList = bctbx_list_remove(coresList, laure.getLc());

		laure.reStart(false);

		if (toggle_video) {
			LinphoneVideoActivationPolicy * pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
			linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
			linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
			linphone_core_set_video_activation_policy(laure.getLc(), pol);
			linphone_video_activation_policy_unref(pol);

			linphone_core_enable_video_capture(laure.getLc(), TRUE);
			linphone_core_enable_video_display(laure.getLc(), TRUE);
		}

		linphone_core_set_default_conference_layout(laure.getLc(), layout);
		coresList = bctbx_list_append(coresList, laure.getLc());

		linphone_core_set_network_reachable(laure.getLc(), TRUE);

		LinphoneCallParams *laure_params = linphone_core_create_call_params(laure.getLc(), nullptr);
		linphone_core_invite_address_with_params_2(laure.getLc(), confAddr, laure_params, NULL, nullptr);
		linphone_call_params_unref(laure_params);

		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneRegistrationOk, 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList,&laure.getStats().number_of_LinphoneConferenceStateCreationPending,1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList,&laure.getStats().number_of_LinphoneConferenceStateCreated,1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList,&laure.getStats().number_of_LinphoneSubscriptionOutgoingProgress,1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList,&laure.getStats().number_of_LinphoneSubscriptionActive,1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList,&laure.getStats().number_of_NotifyReceived,1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList,&laure.getStats().number_of_LinphoneCallUpdating,1,liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList,&laure.getStats().number_of_LinphoneCallStreamsRunning,2,liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + 2, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added, focus_stat.number_of_participant_devices_added + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_added, marie_stat.number_of_participant_devices_added + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_added, pauline_stat.number_of_participant_devices_added + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined, focus_stat.number_of_participant_devices_joined + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_joined, marie_stat.number_of_participant_devices_joined + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_joined, pauline_stat.number_of_participant_devices_joined + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed, focus_stat.number_of_participant_devices_removed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_removed, marie_stat.number_of_participant_devices_removed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_removed, pauline_stat.number_of_participant_devices_removed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_removed, 1, liblinphone_tester_sip_timeout));

		if (toggle_video) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating, marie_stat.number_of_LinphoneCallUpdating + 2, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallUpdating, pauline_stat.number_of_LinphoneCallUpdating + 2, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallUpdating,2,liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote, focus_stat.number_of_LinphoneCallUpdatedByRemote + 6, liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning, marie_stat.number_of_LinphoneCallStreamsRunning + 2, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning, pauline_stat.number_of_LinphoneCallStreamsRunning + 2, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallStreamsRunning,3,liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + 7, liblinphone_tester_sip_timeout));
		}

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
			linphone_address_unref(uri);
			BC_ASSERT_PTR_NOT_NULL(pconference);

			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), (mgr == focus.getCMgr()) ? 3 : 2, int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 3, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), newSubject);
			}

			if (mgr != focus.getCMgr()) {
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
					//BC_ASSERT_TRUE(linphone_call_get_microphone_muted(participant_call) == (mgr == pauline.getCMgr()));
				}
			}

			if (confAddr) {
				check_conference_info(mgr, confAddr, marie.getCMgr(), 3, 0, 0, newSubject, NULL);
			}
		}

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure}).waitUntil(chrono::seconds(5),[] {
			return false;
		});

		std::list<LinphoneCoreManager *> mgrList = {pauline.getCMgr()};
		if (toggle_all_mananger_video) {
			mgrList.push_back(marie.getCMgr());
			mgrList.push_back(laure.getCMgr());
		}

		if (toggle_video) {
			for (int i = 0; i < 4; i++) {
				for (auto mgr : mgrList) {
					LinphoneMediaDirection video_direction = LinphoneMediaDirectionSendRecv;

					LinphoneCall * participant_call = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(participant_call);
					if (participant_call) {
						const LinphoneCallParams* call_lparams = linphone_call_get_params(participant_call);
						BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
						video_direction = linphone_call_params_get_video_direction(call_lparams);

						if (video_direction == LinphoneMediaDirectionRecvOnly) {
							video_direction = LinphoneMediaDirectionSendRecv;
						} else if (video_direction == LinphoneMediaDirectionSendRecv) {
							video_direction = LinphoneMediaDirectionRecvOnly;
						}

					}

					set_video_settings_in_conference(focus.getCMgr(), mgr, mgrList, confAddr, TRUE, video_direction, TRUE, video_direction);

					LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
					LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
					linphone_address_unref(uri);
					BC_ASSERT_PTR_NOT_NULL(pconference);

					if (pconference) {
						bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
						for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
							LinphoneParticipantDevice * d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
							if (linphone_conference_is_me(pconference, linphone_participant_device_get_address(d))) {
								BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) == (video_direction == LinphoneMediaDirectionSendRecv));
							} else {
								BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) == (linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) == LinphoneMediaDirectionSendRecv));
							}
						}

						if (devices) {
							bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
						}

						if (change_layout) {
							stats mgr_stat2=mgr->stat;
							stats focus_stat2=focus.getStats();

							LinphoneConferenceLayout new_layout = LinphoneConferenceLayoutActiveSpeaker;
							LinphoneCall * pcall2 = linphone_conference_get_call(pconference);
							BC_ASSERT_PTR_NOT_NULL(pcall2);
							if (pcall2) {
								const LinphoneCallParams * pcall2_local_params = linphone_call_get_params(pcall2);
								const LinphoneConferenceLayout conference_layout = linphone_call_params_get_conference_video_layout(pcall2_local_params);

								if (conference_layout == LinphoneConferenceLayoutGrid) {
									new_layout = LinphoneConferenceLayoutActiveSpeaker;
								} else {
									new_layout = LinphoneConferenceLayoutGrid;
								}

								LinphoneCallParams * call_params = linphone_core_create_call_params(mgr->lc, pcall2);
								linphone_call_params_set_conference_video_layout(call_params, new_layout);
								linphone_call_update(pcall2, call_params);
								linphone_call_params_unref(call_params);
							}

							BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating, mgr_stat2.number_of_LinphoneCallUpdating + 1, liblinphone_tester_sip_timeout));
							BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote, focus_stat2.number_of_LinphoneCallUpdatedByRemote + 1, liblinphone_tester_sip_timeout));
							BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, mgr_stat2.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));
							BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat2.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));
							if (pcall2) {
								const LinphoneCallParams * pcall2_local_params = linphone_call_get_params(pcall2);
								const LinphoneConferenceLayout remote_conf_layout = linphone_call_params_get_conference_video_layout(pcall2_local_params);
								BC_ASSERT_EQUAL(new_layout, remote_conf_layout, int, "%d");
							}
							LinphoneConference * fconference = linphone_core_search_conference(focus.getLc(), NULL, NULL, confAddr, NULL);
							LinphoneParticipant * participant = linphone_conference_find_participant(fconference, mgr->identity);
							BC_ASSERT_PTR_NOT_NULL(participant);
							if (participant) {
								bctbx_list_t *devices = linphone_participant_get_devices(participant);

								for(bctbx_list_t *it_d = devices; it_d != NULL; it_d = it_d->next) {
									LinphoneParticipantDevice *d = (LinphoneParticipantDevice *) it_d->data;
									BC_ASSERT_PTR_NOT_NULL(d);
									LinphoneCall * participant_call = linphone_core_get_call_by_remote_address2(focus.getLc(), linphone_participant_device_get_address(d));
									BC_ASSERT_PTR_NOT_NULL(participant_call);
									if (participant_call) {
										const LinphoneCallParams * call_remote_params = linphone_call_get_remote_params(participant_call);
										const LinphoneConferenceLayout device_layout = linphone_call_params_get_conference_video_layout(call_remote_params);
										BC_ASSERT_EQUAL(device_layout, new_layout, int, "%d");
									}

								}
								bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
							}

							LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
							BC_ASSERT_PTR_NOT_NULL(pcall);
							if (pcall) {
								const LinphoneCallParams* call_lparams = linphone_call_get_params(pcall);
								BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
								const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(pcall);
								BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
								const LinphoneCallParams* call_cparams = linphone_call_get_current_params(pcall);
								BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams));
							}

							LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
							LinphoneCall * ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), uri);
							BC_ASSERT_PTR_NOT_NULL(ccall);
							if (ccall) {
								const LinphoneCallParams* call_lparams = linphone_call_get_params(ccall);
								BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
								const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(ccall);
								BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
								const LinphoneCallParams* call_cparams = linphone_call_get_current_params(ccall);
								BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams));
							}

						}

					}

					// Wait a little bit
					wait_for_list(coresList, NULL, 0, 1000);
				}
			}
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
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd, total_focus_calls, 40000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallReleased, total_marie_calls, 30000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallReleased, total_pauline_calls, 30000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallReleased, total_laure_calls, 30000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased, total_focus_calls, 40000));

		if (confAddr && fconference) {
			linphone_conference_terminate(fconference);
		}

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {

			// Wait for all conferences to be terminated
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1, liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated, (mgr == focus.getCMgr()) ? 3 : 1,liblinphone_tester_sip_timeout));

			if (mgr && (mgr != focus.getCMgr())) {
				LinphoneCall * participant_call = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
				BC_ASSERT_PTR_NULL(participant_call);
				LinphoneCall * conference_call = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
				BC_ASSERT_PTR_NULL(conference_call);

				const bctbx_list_t* call_logs = linphone_core_get_call_logs(mgr->lc);
				BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(call_logs), (((mgr == marie.getCMgr()) || (mgr == laure.getCMgr())) ? 3 : 2), unsigned int, "%u");

				bctbx_list_t * mgr_focus_call_log = linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
				BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
				if (mgr_focus_call_log) {
					BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(mgr_focus_call_log),  (mgr == laure.getCMgr()) ? 2 : 1, unsigned int, "%u");
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
	create_simple_conference_merging_calls_base(FALSE, LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE, FALSE);
}

static void create_simple_conference_merging_calls_with_video_toggling (void) {
	create_simple_conference_merging_calls_base(FALSE, LinphoneConferenceLayoutGrid, TRUE, TRUE, TRUE);
}

static void create_simple_ice_conference_merging_calls (void) {
	create_simple_conference_merging_calls_base(TRUE, LinphoneConferenceLayoutActiveSpeaker, TRUE, FALSE, TRUE);
}

static void create_simple_conference_with_update_deferred(void) {
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
			LinphoneVideoActivationPolicy * pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
			linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
			linphone_video_activation_policy_set_automatically_initiate(pol, FALSE);
			linphone_core_set_video_activation_policy(mgr->lc, pol);
			linphone_video_activation_policy_unref(pol);

			linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);
			linphone_core_enable_video_capture(mgr->lc, TRUE);
			linphone_core_enable_video_display(mgr->lc, TRUE);

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, LinphoneConferenceLayoutGrid);
			}
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);

		bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, laure.getLc());
		coresList = bctbx_list_append(coresList, michelle.getLc());

		stats marie_stat=marie.getStats();
		stats focus_stat=focus.getStats();

		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()};

		time_t start_time = ms_time(NULL) + 10;
		time_t end_time = (start_time + 300);
		const char *initialSubject = "Test characters: ^ :) ¤ çà @";
		const char *description = "Paris Baker";

		LinphoneAddress * confAddr = create_conference_on_server(focus, marie, participants, start_time, end_time, initialSubject, description, TRUE);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 2, liblinphone_tester_sip_timeout));

		for (auto mgr : members) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
		}

		for (auto mgr : members) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1, liblinphone_tester_sip_timeout));
			int no_streams_running = 2;
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating, (no_streams_running - 1), liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, no_streams_running, liblinphone_tester_sip_timeout));
			// Update to add to conference.
			// If ICE is enabled, the addition to a conference may go through a resume of the call
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, ((mgr == marie.getCMgr()) ? 3 : 2), liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyReceived, 1, liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived, focus_stat.number_of_LinphoneCallIncomingReceived + 3, liblinphone_tester_sip_timeout));
		int focus_no_streams_running = 6;
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote, focus_stat.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running - 3), liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated, focus_stat.number_of_LinphoneConferenceStateCreated + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived, focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 3, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive, focus_stat.number_of_LinphoneSubscriptionActive + 3, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added, focus_stat.number_of_participants_added + 3, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added, focus_stat.number_of_participant_devices_added + 3, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined, focus_stat.number_of_participant_devices_joined + 3, liblinphone_tester_sip_timeout));

		LinphoneConference * fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure,michelle}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			LinphoneConference * pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
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

					LinphoneVideoActivationPolicy * pol = linphone_core_get_video_activation_policy(mgr->lc);
					bool_t enabled = !!linphone_video_activation_policy_get_automatically_initiate(pol);
					LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						const LinphoneCallParams* call_lparams = linphone_call_get_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
						const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
						const LinphoneCallParams* call_cparams = linphone_call_get_current_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
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
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 3, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
				}
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

		LinphoneCall * pauline_call = linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(pauline_call);

		LinphoneVideoActivationPolicy * pol = linphone_core_get_video_activation_policy(pauline.getLc());
		bool_t enable = !!!linphone_video_activation_policy_get_automatically_initiate(pol);
		linphone_video_activation_policy_unref(pol);

		if (pauline_call) {
			const LinphoneCallParams* call_lparams = linphone_call_get_params(pauline_call);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_lparams));
			const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(pauline_call);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_rparams));
			const LinphoneCallParams* call_cparams = linphone_call_get_current_params(pauline_call);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_cparams));
		}
		Address paulineIdentity(pauline.getIdentity().asAddress());
		LinphoneCall * focus_call = linphone_core_get_call_by_remote_address2(focus.getLc(), L_GET_C_BACK_PTR(&paulineIdentity));
		BC_ASSERT_PTR_NOT_NULL(focus_call);
		if (focus_call) {
			const LinphoneCallParams* call_lparams = linphone_call_get_params(focus_call);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_lparams));
			const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(focus_call);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_rparams));
			const LinphoneCallParams* call_cparams = linphone_call_get_current_params(focus_call);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_cparams));
		}

		LinphoneAddress *paulineUri = linphone_address_new(linphone_core_get_identity(pauline.getLc()));
		LinphoneConference * paulineConference = linphone_core_search_conference(pauline.getLc(), NULL, paulineUri, confAddr, NULL);
		linphone_address_unref(paulineUri);
		BC_ASSERT_PTR_NOT_NULL(paulineConference);

		linphone_config_set_int(linphone_core_get_config(focus.getLc()), "sip", "defer_update_default", TRUE);

		for (int i = 0; i < 4; i++) {
			set_video_settings_in_conference(focus.getCMgr(), pauline.getCMgr(), members, confAddr, enable, LinphoneMediaDirectionSendRecv, !enable, LinphoneMediaDirectionSendRecv);

			// Wait a little bit
			wait_for_list(coresList, NULL, 0, 1000);

			enable = !enable;
		}

		linphone_config_set_int(linphone_core_get_config(focus.getLc()), "sip", "defer_update_default", FALSE);

		if (paulineConference) {

			stats focus_stat2=focus.getStats();
			stats marie_stat2=marie.getStats();
			stats pauline_stat2=pauline.getStats();
			stats laure_stat2=laure.getStats();

			LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			bool_t video_enabled = FALSE;
			if (pcall) {
				const LinphoneCallParams* call_cparams = linphone_call_get_current_params(pcall);
				video_enabled = linphone_call_params_video_enabled(call_cparams);
			}

			linphone_conference_leave(paulineConference);

			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallPausing, pauline_stat2.number_of_LinphoneCallPausing + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallPaused, pauline_stat2.number_of_LinphoneCallPaused + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallPausedByRemote, focus_stat2.number_of_LinphoneCallPausedByRemote + 1, liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_on_hold, focus_stat2.number_of_participant_devices_on_hold + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_on_hold, laure_stat2.number_of_participant_devices_on_hold + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_media_capability_changed, laure_stat2.number_of_participant_devices_media_capability_changed + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_on_hold, marie_stat2.number_of_participant_devices_on_hold + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_media_capability_changed, marie_stat2.number_of_participant_devices_media_capability_changed + 1, liblinphone_tester_sip_timeout));

			linphone_conference_enter(paulineConference);

			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallResuming, pauline_stat2.number_of_LinphoneCallResuming + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning, pauline_stat2.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat2.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined, focus_stat2.number_of_participant_devices_joined + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_joined, laure_stat2.number_of_participant_devices_joined + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_media_capability_changed, laure_stat2.number_of_participant_devices_media_capability_changed + 2, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_joined, marie_stat2.number_of_participant_devices_joined + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_media_capability_changed, marie_stat2.number_of_participant_devices_media_capability_changed + 2, liblinphone_tester_sip_timeout));

			if (pcall) {
				const LinphoneCallParams* call_cparams = linphone_call_get_current_params(pcall);
				BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), video_enabled, int, "%0d");
			}
		}

		std::list <LinphoneCoreManager*> mgrsToRemove {pauline.getCMgr()};
		mgrsToRemove.push_back(laure.getCMgr());

		for (auto mgr : mgrsToRemove) {
			LinphoneCall * call = linphone_core_get_current_call(mgr->lc);
			BC_ASSERT_PTR_NOT_NULL(call);
			if (call) {
				linphone_call_terminate(call);
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1, liblinphone_tester_sip_timeout));

				LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
				LinphoneConference * pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
				BC_ASSERT_PTR_NULL(pconference);
				linphone_address_unref(uri);
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed, focus_stat.number_of_participants_removed + 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed, focus_stat.number_of_participant_devices_removed + 2, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed, marie_stat.number_of_participants_removed + 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_removed, marie_stat.number_of_participant_devices_removed + 2, liblinphone_tester_sip_timeout));

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
			LinphoneConference * pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), ((mgr == focus.getCMgr()) ? 1 : 0), int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 1, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
			}
		}

		const bctbx_list_t * calls = linphone_core_get_calls(marie.getLc());
		BC_ASSERT_EQUAL(bctbx_list_size(calls), 1, size_t, "%zu");

		LinphoneCall * call = linphone_core_get_call_by_remote_address2(marie.getLc(), focus.getCMgr()->identity);
		BC_ASSERT_PTR_NOT_NULL(call);
		if (call) {
			linphone_call_terminate(call);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionTerminated, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminationPending, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminated, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateDeleted, 1, liblinphone_tester_sip_timeout));

			// Explicitely terminate conference as those on server are static by default
			LinphoneConference * pconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				linphone_conference_terminate(pconference);
			}
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1, liblinphone_tester_sip_timeout));
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

static void create_one_participant_conference_toggle_video_base (LinphoneConferenceLayout layout, bool_t enable_ice, bool_t enable_stun) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focus.getIdentity().asAddress());
		ClientConference laure("laure_tcp_rc", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t * coresList = NULL;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			LinphoneVideoActivationPolicy * pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
			linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
			linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
			linphone_core_set_video_activation_policy(mgr->lc, pol);
			linphone_video_activation_policy_unref(pol);

			linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);
			linphone_core_enable_video_capture(mgr->lc, TRUE);
			linphone_core_enable_video_display(mgr->lc, TRUE);

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, layout);
			}

			enable_stun_in_core(mgr, enable_stun, enable_ice);
			linphone_core_manager_wait_for_stun_resolution(mgr);

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);

		stats focus_stat=focus.getStats();
		stats marie_stat=marie.getStats();

		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr()};

		time_t start_time = ms_time(NULL);
		int duration = -1;
		time_t end_time = (duration <= 0) ? -1 : (start_time + duration * 60);
		const char *initialSubject = "Test characters: ^ :) ¤ çà @";
		const char *description = "- <F2><F3>\n\\çà";

		LinphoneAddress * confAddr = create_conference_on_server(focus, marie, participants, start_time, end_time, initialSubject, description, TRUE);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 2, liblinphone_tester_sip_timeout));

		for (auto mgr : {marie.getCMgr()}) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_call_params_set_video_direction (new_params, LinphoneMediaDirectionSendRecv);
			linphone_call_params_enable_video (new_params, TRUE);
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
		}

		for (auto mgr : {marie.getCMgr()}) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1, liblinphone_tester_sip_timeout));
			int no_streams_running = ((enable_ice) ? 3 : 2);
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating, (no_streams_running - 1), liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, no_streams_running, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, ((mgr == marie.getCMgr()) ? 3 : 2), liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyReceived, 1, liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived, focus_stat.number_of_LinphoneCallIncomingReceived + 1, liblinphone_tester_sip_timeout));
		int focus_no_streams_running = ((enable_ice) ? 3 : 2);
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote, focus_stat.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running - 1), liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated, focus_stat.number_of_LinphoneConferenceStateCreated + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived, focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive, focus_stat.number_of_LinphoneSubscriptionActive + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added, focus_stat.number_of_participants_added + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added, focus_stat.number_of_participant_devices_added + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined, focus_stat.number_of_participant_devices_joined + 1, liblinphone_tester_sip_timeout));

		LinphoneConference * fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		int no_streams_audio = 1;
		int no_streams_video = 2;
		int no_streams_text = 0;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr()}) {
			LinphoneConference * pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				const LinphoneConferenceParams * conference_params = linphone_conference_get_current_params(pconference);
				int no_participants = 0;
				if (start_time >= 0) {
					BC_ASSERT_EQUAL((long long)linphone_conference_params_get_start_time(conference_params), (long long)start_time, long long, "%lld");
				}
				BC_ASSERT_EQUAL((long long)linphone_conference_params_get_end_time(conference_params), (long long)end_time, long long, "%lld");
				if (mgr == focus.getCMgr()) {
					no_participants = 1;
					BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
				} else {
					no_participants = 0;
					BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
					LinphoneCall * current_call = linphone_core_get_current_call(mgr->lc);
					BC_ASSERT_PTR_NOT_NULL(current_call);
					if (current_call) {
						BC_ASSERT_EQUAL((int)linphone_call_get_state(current_call), (int)LinphoneCallStateStreamsRunning, int, "%0d");
					}
					if (enable_ice) {
						BC_ASSERT_TRUE(check_ice(mgr, focus.getCMgr(), LinphoneIceStateHostConnection));
					}

					LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						_linphone_call_check_nb_streams(pcall, no_streams_audio, no_streams_video, no_streams_text);
						const LinphoneCallParams* call_lparams = linphone_call_get_params(pcall);
						BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
						const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(pcall);
						BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
						const LinphoneCallParams* call_cparams = linphone_call_get_current_params(pcall);
						BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams));
					}
					LinphoneCall * ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
					BC_ASSERT_PTR_NOT_NULL(ccall);
					if (ccall) {
						_linphone_call_check_nb_streams(ccall, no_streams_audio, no_streams_video, no_streams_text);
						const LinphoneCallParams* call_lparams = linphone_call_get_params(ccall);
						BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
						const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(ccall);
						BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
						const LinphoneCallParams* call_cparams = linphone_call_get_current_params(ccall);
						BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams));
					}
				}
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 1, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
				}
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

		focus_stat=focus.getStats();
		marie_stat=marie.getStats();

		LinphoneCall * marie_calls_focus=linphone_core_get_call_by_remote_address2(marie.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(marie_calls_focus);
		LinphoneCall * focus_called_by_marie=linphone_core_get_call_by_remote_address2(focus.getLc(), marie.getCMgr()->identity);
		BC_ASSERT_PTR_NOT_NULL(focus_called_by_marie);

		LinphoneParticipant * marie_participant = linphone_conference_find_participant(fconference, marie.getCMgr()->identity);
		BC_ASSERT_PTR_NOT_NULL(marie_participant);
		if (marie_participant) {
			bctbx_list_t *devices = linphone_participant_get_devices(marie_participant);
			for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
				LinphoneParticipantDevice * d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
				LinphoneMediaDirection video_direction = (layout == LinphoneConferenceLayoutGrid) ? LinphoneMediaDirectionSendOnly : LinphoneMediaDirectionSendRecv;
				BC_ASSERT_TRUE(linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) == video_direction);
				BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
			}
			if (devices) {
				bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
			}
		}

		if (marie_calls_focus) {
			LinphoneCallParams * new_params = linphone_core_create_call_params(marie.getLc(), marie_calls_focus);
			linphone_call_params_enable_video (new_params, TRUE);
			linphone_call_params_set_video_direction (new_params, LinphoneMediaDirectionRecvOnly);
			linphone_call_update(marie_calls_focus, new_params);
			linphone_call_params_unref (new_params);
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating, marie_stat.number_of_LinphoneCallUpdating + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning, marie_stat.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_media_capability_changed, marie_stat.number_of_participant_devices_media_capability_changed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote, focus_stat.number_of_LinphoneCallUpdatedByRemote + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_media_capability_changed, focus_stat.number_of_participant_devices_media_capability_changed + 1, liblinphone_tester_sip_timeout));

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		if (marie_calls_focus) {
			_linphone_call_check_nb_streams(marie_calls_focus, no_streams_audio, no_streams_video, no_streams_text);
			const LinphoneCallParams* call_lparams = linphone_call_get_params(marie_calls_focus);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
			const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(marie_calls_focus);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams) != (layout == LinphoneConferenceLayoutGrid));
			const LinphoneCallParams* call_cparams = linphone_call_get_current_params(marie_calls_focus);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams) != (layout == LinphoneConferenceLayoutGrid));
		}
		if (focus_called_by_marie) {
			_linphone_call_check_nb_streams(focus_called_by_marie, no_streams_audio, no_streams_video, no_streams_text);
			const LinphoneCallParams* call_lparams = linphone_call_get_params(focus_called_by_marie);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams) != (layout == LinphoneConferenceLayoutGrid));
			const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(focus_called_by_marie);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams) != (layout == LinphoneConferenceLayoutGrid));
			const LinphoneCallParams* call_cparams = linphone_call_get_current_params(focus_called_by_marie);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams) != (layout == LinphoneConferenceLayoutGrid));
		}

		if (marie_participant) {
			bctbx_list_t *devices = linphone_participant_get_devices(marie_participant);
			for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
				LinphoneParticipantDevice * d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
				LinphoneMediaDirection video_direction = (layout == LinphoneConferenceLayoutGrid) ? LinphoneMediaDirectionInactive : LinphoneMediaDirectionRecvOnly;
				BC_ASSERT_TRUE(linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) == video_direction);
				BC_ASSERT_FALSE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
			}
			if (devices) {
				bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
			}
		}

		focus_stat=focus.getStats();
		marie_stat=marie.getStats();

		if (marie_calls_focus) {
			LinphoneCallParams * new_params = linphone_core_create_call_params(marie.getLc(), marie_calls_focus);
			linphone_call_params_enable_video (new_params, TRUE);
			linphone_call_params_set_video_direction (new_params, LinphoneMediaDirectionSendRecv);
			linphone_call_update(marie_calls_focus, new_params);
			linphone_call_params_unref (new_params);
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating, marie_stat.number_of_LinphoneCallUpdating + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning, marie_stat.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_media_capability_changed, marie_stat.number_of_participant_devices_media_capability_changed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote, focus_stat.number_of_LinphoneCallUpdatedByRemote + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_media_capability_changed, focus_stat.number_of_participant_devices_media_capability_changed + 1, liblinphone_tester_sip_timeout));

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		if (marie_calls_focus) {
			_linphone_call_check_nb_streams(marie_calls_focus, no_streams_audio, no_streams_video, no_streams_text);
			const LinphoneCallParams* call_lparams = linphone_call_get_params(marie_calls_focus);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
			const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(marie_calls_focus);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
			const LinphoneCallParams* call_cparams = linphone_call_get_current_params(marie_calls_focus);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams));
		}
		if (focus_called_by_marie) {
			_linphone_call_check_nb_streams(focus_called_by_marie, no_streams_audio, no_streams_video, no_streams_text);
			const LinphoneCallParams* call_lparams = linphone_call_get_params(focus_called_by_marie);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
			const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(focus_called_by_marie);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
			const LinphoneCallParams* call_cparams = linphone_call_get_current_params(focus_called_by_marie);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams));
		}

		if (marie_participant) {
			bctbx_list_t *devices = linphone_participant_get_devices(marie_participant);
			for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
				LinphoneParticipantDevice * d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
				LinphoneMediaDirection video_direction = (layout == LinphoneConferenceLayoutGrid) ? LinphoneMediaDirectionSendOnly : LinphoneMediaDirectionSendRecv;
				BC_ASSERT_TRUE(linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) == video_direction);
				BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
			}
			if (devices) {
				bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
			}
		}

		focus_stat=focus.getStats();
		marie_stat=marie.getStats();

		if (marie_calls_focus) {
			LinphoneCallParams * new_params = linphone_core_create_call_params(marie.getLc(), marie_calls_focus);
			linphone_call_params_enable_video (new_params, FALSE);
			linphone_call_params_set_video_direction (new_params, LinphoneMediaDirectionRecvOnly);
			linphone_call_update(marie_calls_focus, new_params);
			linphone_call_params_unref (new_params);
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating, marie_stat.number_of_LinphoneCallUpdating + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning, marie_stat.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_media_capability_changed, marie_stat.number_of_participant_devices_media_capability_changed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote, focus_stat.number_of_LinphoneCallUpdatedByRemote + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_media_capability_changed, focus_stat.number_of_participant_devices_media_capability_changed + 1, liblinphone_tester_sip_timeout));

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		if (marie_calls_focus) {
			_linphone_call_check_nb_streams(marie_calls_focus, no_streams_audio, no_streams_video, no_streams_text);
			const LinphoneCallParams* call_lparams = linphone_call_get_params(marie_calls_focus);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_lparams));
			const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(marie_calls_focus);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_rparams));
			const LinphoneCallParams* call_cparams = linphone_call_get_current_params(marie_calls_focus);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_cparams));
		}
		if (focus_called_by_marie) {
			_linphone_call_check_nb_streams(focus_called_by_marie, no_streams_audio, no_streams_video, no_streams_text);
			const LinphoneCallParams* call_lparams = linphone_call_get_params(focus_called_by_marie);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_lparams));
			const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(focus_called_by_marie);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_rparams));
			const LinphoneCallParams* call_cparams = linphone_call_get_current_params(focus_called_by_marie);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_cparams));
		}

		if (marie_participant) {
			bctbx_list_t *devices = linphone_participant_get_devices(marie_participant);
			for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
				LinphoneParticipantDevice * d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
				LinphoneMediaDirection video_direction = LinphoneMediaDirectionInactive;
				BC_ASSERT_TRUE(linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) == video_direction);
				BC_ASSERT_FALSE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
			}
			if (devices) {
				bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
			}
		}

		focus_stat=focus.getStats();
		marie_stat=marie.getStats();

		if (marie_calls_focus) {
			LinphoneCallParams * new_params = linphone_core_create_call_params(marie.getLc(), marie_calls_focus);
			linphone_call_params_enable_video (new_params, TRUE);
			linphone_call_update(marie_calls_focus, new_params);
			linphone_call_params_unref (new_params);
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating, marie_stat.number_of_LinphoneCallUpdating + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning, marie_stat.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));
		if (layout == LinphoneConferenceLayoutGrid) {
			BC_ASSERT_FALSE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_media_capability_changed, marie_stat.number_of_participant_devices_media_capability_changed + 1, 2000));
		} else {
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_media_capability_changed, marie_stat.number_of_participant_devices_media_capability_changed + 1, 2000));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote, focus_stat.number_of_LinphoneCallUpdatedByRemote + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));
		if (layout == LinphoneConferenceLayoutGrid) {
			BC_ASSERT_FALSE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_media_capability_changed, focus_stat.number_of_participant_devices_media_capability_changed + 1, 2000));
		} else {
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_media_capability_changed, focus_stat.number_of_participant_devices_media_capability_changed + 1, 2000));
		}

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		if (marie_calls_focus) {
			_linphone_call_check_nb_streams(marie_calls_focus, no_streams_audio, no_streams_video, no_streams_text);
			const LinphoneCallParams* call_lparams = linphone_call_get_params(marie_calls_focus);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
			const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(marie_calls_focus);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams) != (layout == LinphoneConferenceLayoutGrid));
			const LinphoneCallParams* call_cparams = linphone_call_get_current_params(marie_calls_focus);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams) != (layout == LinphoneConferenceLayoutGrid));
		}
		if (focus_called_by_marie) {
			_linphone_call_check_nb_streams(focus_called_by_marie, no_streams_audio, no_streams_video, no_streams_text);
			const LinphoneCallParams* call_lparams = linphone_call_get_params(focus_called_by_marie);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams) != (layout == LinphoneConferenceLayoutGrid));
			const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(focus_called_by_marie);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams) != (layout == LinphoneConferenceLayoutGrid));
			const LinphoneCallParams* call_cparams = linphone_call_get_current_params(focus_called_by_marie);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams) != (layout == LinphoneConferenceLayoutGrid));
		}

		if (marie_participant) {
			bctbx_list_t *devices = linphone_participant_get_devices(marie_participant);
			for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
				LinphoneParticipantDevice * d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
				LinphoneMediaDirection video_direction = (layout == LinphoneConferenceLayoutGrid) ? LinphoneMediaDirectionInactive : LinphoneMediaDirectionRecvOnly;
				BC_ASSERT_TRUE(linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) == video_direction);
				BC_ASSERT_FALSE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
			}
			if (devices) {
				bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
			}
		}

		focus_stat=focus.getStats();
		marie_stat=marie.getStats();

		if (marie_calls_focus) {
			LinphoneCallParams * new_params = linphone_core_create_call_params(marie.getLc(), marie_calls_focus);
			linphone_call_params_set_video_direction (new_params, LinphoneMediaDirectionSendRecv);
			linphone_call_update(marie_calls_focus, new_params);
			linphone_call_params_unref (new_params);
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating, marie_stat.number_of_LinphoneCallUpdating + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning, marie_stat.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_media_capability_changed, marie_stat.number_of_participant_devices_media_capability_changed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote, focus_stat.number_of_LinphoneCallUpdatedByRemote + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_media_capability_changed, focus_stat.number_of_participant_devices_media_capability_changed + 1, liblinphone_tester_sip_timeout));

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		if (marie_calls_focus) {
			_linphone_call_check_nb_streams(marie_calls_focus, no_streams_audio, no_streams_video, no_streams_text);
			const LinphoneCallParams* call_lparams = linphone_call_get_params(marie_calls_focus);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
			const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(marie_calls_focus);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
			const LinphoneCallParams* call_cparams = linphone_call_get_current_params(marie_calls_focus);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams));
		}
		if (focus_called_by_marie) {
			_linphone_call_check_nb_streams(focus_called_by_marie, no_streams_audio, no_streams_video, no_streams_text);
			const LinphoneCallParams* call_lparams = linphone_call_get_params(focus_called_by_marie);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
			const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(focus_called_by_marie);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
			const LinphoneCallParams* call_cparams = linphone_call_get_current_params(focus_called_by_marie);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams));
		}

		if (marie_participant) {
			bctbx_list_t *devices = linphone_participant_get_devices(marie_participant);
			for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
				LinphoneParticipantDevice * d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
				LinphoneMediaDirection video_direction = (layout == LinphoneConferenceLayoutGrid) ? LinphoneMediaDirectionSendOnly : LinphoneMediaDirectionSendRecv;
				BC_ASSERT_TRUE(linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) == video_direction);
				BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
			}
			if (devices) {
				bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
			}
		}

		LinphoneCall * call = linphone_core_get_call_by_remote_address2(marie.getLc(), focus.getCMgr()->identity);
		BC_ASSERT_PTR_NOT_NULL(call);
		if (call) {
			linphone_call_terminate(call);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionTerminated, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminationPending, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminated, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateDeleted, 1, liblinphone_tester_sip_timeout));

			// Explicitely terminate conference as those on server are static by default
			LinphoneConference * pconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				linphone_conference_terminate(pconference);
			}
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1, liblinphone_tester_sip_timeout));
		}

		for (auto mgr : {focus.getCMgr(), marie.getCMgr()}) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated, 1,liblinphone_tester_sip_timeout));

			if (mgr && (mgr != focus.getCMgr())) {
				LinphoneCall * participant_call = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
				BC_ASSERT_PTR_NULL(participant_call);
				LinphoneCall * conference_call = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
				BC_ASSERT_PTR_NULL(conference_call);

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
		}

		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void one_participant_conference_toggles_video_grid(void) {
	create_one_participant_conference_toggle_video_base (LinphoneConferenceLayoutGrid, FALSE, FALSE);
}

static void one_participant_conference_toggles_video_active_speaker(void) {
	create_one_participant_conference_toggle_video_base (LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE);
}

static void create_conference_with_active_call_base (bool_t dialout) {
	Focus focus("chloe_rc");
	{//to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity().asAddress());
		ClientConference pauline("pauline_rc", focus.getIdentity().asAddress());
		ClientConference laure("laure_tcp_rc", focus.getIdentity().asAddress());
		ClientConference michelle("michelle_rc", focus.getIdentity().asAddress());
		ClientConference lise("lise_rc", focus.getIdentity().asAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(lise);

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t * coresList = NULL;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(), lise.getCMgr()}) {
			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);

		stats focus_stat=focus.getStats();
		stats marie_stat=marie.getStats();
		stats lise_stat=lise.getStats();

		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr()};
		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr()};

		const char *initialSubject = "Schedule of the trip towards the top of Europe";
		const char *description = "To the top of the Mont Blanc!!!! :-)";

		time_t start_time = (dialout) ? -1 : (ms_time(NULL) + 10);
		bool_t send_ics = TRUE;

		LinphoneAddress * confAddr = create_conference_on_server(focus, marie, participants, start_time, -1, initialSubject, description, send_ics);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		// the conference server calls Lise  - who knows why......
		// THis is to test
		LinphoneCallParams * new_params = linphone_core_create_call_params(lise.getLc(), NULL);

		LinphoneContent *content = linphone_core_create_content(lise.getLc());
		linphone_content_set_type(content, "application");
		linphone_content_set_subtype(content, "resource-lists+xml");

		static const char *info_content = "<p1:resource-lists xmlns:p1=\"urn:ietf:params:xml:ns:resource-lists\">\n\
<p1:list>\n\
  <p1:entry uri=\"sip:laure@sip.example.org\"/>\n\
  <p1:entry uri=\"sip:michelle@sip.example.org\"/>\n\
  <p1:entry uri=\"sip:pauline@sip.example.org\"/>\n\
</p1:list>\n\
</p1:resource-lists>";
		linphone_content_set_buffer(content, (const uint8_t *)info_content, strlen(info_content));
		linphone_call_params_add_custom_content (new_params, content);

		LinphoneCall * lise_focus_call = linphone_core_invite_address_with_params_2(lise.getLc(), confAddr, new_params, NULL, nullptr);
		BC_ASSERT_PTR_NOT_NULL(lise_focus_call);
		linphone_content_unref(content);

		BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_LinphoneCallOutgoingInit, lise_stat.number_of_LinphoneCallOutgoingInit + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived, focus_stat.number_of_LinphoneCallIncomingReceived + 1, 10000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_LinphoneCallStreamsRunning, lise_stat.number_of_LinphoneCallStreamsRunning + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + 1, 10000));

		if (dialout) {
			BC_ASSERT_PTR_NULL(linphone_core_get_current_call(focus.getLc()));
		} else {
			BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(focus.getLc()));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived, focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated, focus_stat.number_of_LinphoneSubscriptionTerminated + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_LinphoneSubscriptionError, 1, 5000));
		if (dialout) {
			// Chat room creation to send ICS
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, (send_ics ? 2 : 1), 10000));

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallOutgoingInit, marie_stat.number_of_LinphoneCallOutgoingInit + 1, 10000));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingInit, focus_stat.number_of_LinphoneCallOutgoingInit + 3, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived, focus_stat.number_of_LinphoneCallIncomingReceived + 2, 10000));

			for (auto mgr : participants) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallIncomingReceived, 1, 10000));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning, marie_stat.number_of_LinphoneCallStreamsRunning + 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning, marie_stat.number_of_LinphoneCallStreamsRunning + 2, 10000));

			LinphoneConference * oconference = linphone_core_search_conference_2(marie.getLc(), confAddr);
			if (BC_ASSERT_PTR_NOT_NULL(oconference)) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(oconference), 3, int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(oconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 4, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
				}
			}

			if (confAddr) {
				for (auto mgr : participants) {
					LinphoneConferenceInfo * info = linphone_core_find_conference_information_from_uri(mgr->lc, confAddr);
					if (BC_ASSERT_PTR_NOT_NULL(info)) {
						BC_ASSERT_TRUE(linphone_address_weak_equal(marie.getCMgr()->identity, linphone_conference_info_get_organizer(info)));
						BC_ASSERT_TRUE(linphone_address_weak_equal(confAddr, linphone_conference_info_get_uri(info)));

						const bctbx_list_t * info_participants = linphone_conference_info_get_participants(info);
						BC_ASSERT_EQUAL(bctbx_list_size(info_participants), 4, size_t, "%zu");
						bctbx_list_free((bctbx_list_t *)info_participants);

						BC_ASSERT_NOT_EQUAL((long long)linphone_conference_info_get_date_time(info), 0, long long, "%lld");
						const int duration_m = linphone_conference_info_get_duration(info);
						BC_ASSERT_EQUAL(duration_m, 0, int, "%d");
						if (initialSubject) {
							BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_subject(info), initialSubject);
						} else {
							BC_ASSERT_PTR_NULL(linphone_conference_info_get_subject(info));
						}
						if (send_ics) {
							BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_description(info), description);
						} else {
							BC_ASSERT_PTR_NULL(linphone_conference_info_get_description(info));
						}
						linphone_conference_info_unref(info);
					}

					LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						LinphoneCallLog * call_log = linphone_call_get_call_log(pcall);
						BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
						linphone_call_accept(pcall);
					}
				}
			}
		} else {
			for (auto mgr : members) {
				LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
				linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
				linphone_call_params_unref(new_params);
			}

			for (auto mgr : members) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1, 10000));
				int no_streams_running = 2;
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating, (no_streams_running - 1), 10000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, no_streams_running, 10000));
			}
		}
		linphone_call_params_unref (new_params);

		for (auto mgr : participants) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 2, 10000));

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyReceived, 1, 10000));

			LinphoneConferenceInfo * info = linphone_core_find_conference_information_from_uri(mgr->lc, confAddr);
			if (BC_ASSERT_PTR_NOT_NULL(info)) {
				BC_ASSERT_TRUE(linphone_address_weak_equal(marie.getCMgr()->identity, linphone_conference_info_get_organizer(info)));
				BC_ASSERT_TRUE(linphone_address_weak_equal(confAddr, linphone_conference_info_get_uri(info)));

				const bctbx_list_t * info_participants = linphone_conference_info_get_participants(info);

				// Original participants + Marie who joined the conference
				BC_ASSERT_EQUAL(bctbx_list_size(info_participants), 4, size_t, "%zu");
				bctbx_list_free((bctbx_list_t *)info_participants);

				BC_ASSERT_GREATER_STRICT((long long)linphone_conference_info_get_date_time(info), 0, long long, "%lld");
				const int duration_m = linphone_conference_info_get_duration(info);
				BC_ASSERT_EQUAL(duration_m, 0, int, "%d");
				if (initialSubject) {
					BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_subject(info), initialSubject);
				} else {
					BC_ASSERT_PTR_NULL(linphone_conference_info_get_subject(info));
				}
				if (send_ics) {
					BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_description(info), description);
				} else {
					BC_ASSERT_PTR_NULL(linphone_conference_info_get_description(info));
				}
				linphone_conference_info_unref(info);
			}

			LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				LinphoneCallLog * call_log = linphone_call_get_call_log(pcall);
				BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				LinphoneConferenceInfo * call_log_info = linphone_call_log_get_conference_info(call_log);
				if (BC_ASSERT_PTR_NOT_NULL(call_log_info)) {
					BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_conference_info_get_organizer(call_log_info), marie.getCMgr()->identity));
					BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_conference_info_get_uri(call_log_info), confAddr));

					const bctbx_list_t * info_participants = linphone_conference_info_get_participants(call_log_info);

					// Original participants + Marie who joined the conference
					BC_ASSERT_EQUAL(bctbx_list_size(info_participants), 4, size_t, "%zu");
					bctbx_list_free((bctbx_list_t *)info_participants);

					BC_ASSERT_GREATER_STRICT((long long)linphone_conference_info_get_date_time(call_log_info), 0, long long, "%lld");
					const int duration_m = linphone_conference_info_get_duration(call_log_info);
					BC_ASSERT_EQUAL(duration_m, 0, int, "%d");
					if (initialSubject) {
						BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_subject(call_log_info), initialSubject);
					} else {
						BC_ASSERT_PTR_NULL(linphone_conference_info_get_subject(call_log_info));
					}
					if (send_ics) {
						BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_description(call_log_info), description);
					} else {
						BC_ASSERT_PTR_NULL(linphone_conference_info_get_description(call_log_info));
					}
				}
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + static_cast<int>(participants.size() + 1), 10000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, marie_stat.number_of_LinphoneConferenceStateCreated + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionOutgoingProgress, marie_stat.number_of_LinphoneSubscriptionOutgoingProgress + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive, marie_stat.number_of_LinphoneSubscriptionActive + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyReceived, marie_stat.number_of_NotifyReceived + 1, 10000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated, focus_stat.number_of_LinphoneConferenceStateCreated + 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived, focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 5, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive, focus_stat.number_of_LinphoneSubscriptionActive + 4, 5000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added, focus_stat.number_of_participants_added + 4, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added, focus_stat.number_of_participant_devices_added + 4, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined, focus_stat.number_of_participant_devices_joined + 4, 10000));

		LinphoneConference * fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		for (auto mgr : conferenceMgrs) {
			//wait bit more to detect side effect if any
			CoreManagerAssert({focus,marie,pauline,laure,michelle,lise}).waitUntil(chrono::seconds(50),[mgr, &focus, &members, confAddr] {

				size_t nb_audio_streams = 1;
				size_t nb_video_streams = 0;
				size_t nb_text_streams = 0;
				std::list<LinphoneCall *> calls;

				if (mgr == focus.getCMgr()) {
					for (auto m : members) {
						LinphoneCall * call = linphone_core_get_call_by_remote_address2(mgr->lc, m->identity);
						BC_ASSERT_PTR_NOT_NULL(call);
						if (call) {
							calls.push_back(call);
						} else {
							return false;
						}
					}
				} else {
					LinphoneCall * call = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(call);
					if (call) {
						calls.push_back(call);
					} else {
						return false;
					}
				}

				for (auto call : calls) {
					if (call) {
						const SalMediaDescription * call_local_desc = _linphone_call_get_result_desc(call);
						return (call_local_desc->getNbStreams() == nb_audio_streams + nb_video_streams + nb_text_streams) &&
							(call_local_desc->nbStreamsOfType(SalAudio) == nb_audio_streams) &&
							(call_local_desc->nbStreamsOfType(SalVideo) == nb_video_streams) &&
							(call_local_desc->nbStreamsOfType(SalText) == nb_text_streams) &&
							(linphone_call_get_state(call) == LinphoneCallStateStreamsRunning);
					}
				}

				LinphoneConference * conference = linphone_core_search_conference_2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(conference);
				if (conference) {
					bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
					for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
						LinphoneParticipantDevice * d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
						return !linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo);
					}

					if (devices) {
						bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
					}
				}
				return false;
			});
		}

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure,michelle,lise}).waitUntil(chrono::seconds(15),[] {
			return false;
		});

		for (auto mgr : conferenceMgrs) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference * pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			linphone_address_unref(uri);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				int no_participants = 0;
				if (mgr == focus.getCMgr()) {
					no_participants = (dialout) ? 4 : 5;
					BC_ASSERT_FALSE(linphone_conference_is_in(pconference));

					int focus_no_streams_running = 8;
					int focus_no_updating = focus_no_streams_running - 4;
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote, focus_stat.number_of_LinphoneCallUpdatedByRemote + focus_no_updating, 10000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running, 10000));
				} else {
					no_participants = (dialout) ? 3 : 4;
					BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
					LinphoneCall * current_call = linphone_core_get_current_call(mgr->lc);
					BC_ASSERT_PTR_NOT_NULL(current_call);
					if (current_call) {
						BC_ASSERT_EQUAL((int)linphone_call_get_state(current_call), (int)LinphoneCallStateStreamsRunning, int, "%0d");
					}

					bool_t enabled = FALSE;
					bool_t video_strem_enabled = dialout && (mgr->lc == marie.getLc());
					int no_streams_audio = 1;
					int no_streams_video = (video_strem_enabled) ? 1 : 0;
					int no_streams_text = 0;

					LinphoneCall * pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						_linphone_call_check_nb_streams(pcall, no_streams_audio, no_streams_video, no_streams_text);
						const LinphoneCallParams* call_lparams = linphone_call_get_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
						const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
						const LinphoneCallParams* call_cparams = linphone_call_get_current_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
					}

					LinphoneCall * ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
					BC_ASSERT_PTR_NOT_NULL(ccall);
					if (ccall) {
						_linphone_call_check_nb_streams(ccall, no_streams_audio, no_streams_video, no_streams_text);
						const LinphoneCallParams* call_lparams = linphone_call_get_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
						const LinphoneCallParams* call_rparams = linphone_call_get_remote_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
						const LinphoneCallParams* call_cparams = linphone_call_get_current_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
					}
				}
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), (dialout) ? 4 : 5, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
				}
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

				LinphoneConference * conference = linphone_core_search_conference_2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(conference);
				if (conference) {
					bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
					for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
						LinphoneParticipantDevice * d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
						BC_ASSERT_FALSE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
					}

					if (devices) {
						bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
					}
				}
			}
		}

		focus_stat=focus.getStats();
		for (auto mgr : members) {
			LinphoneCall * call = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
			BC_ASSERT_PTR_NOT_NULL(call);
			if (call) {
				ms_message("%s is terminating call with %s", linphone_core_get_identity(mgr->lc), linphone_core_get_identity(focus.getLc()));
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

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd, focus_stat.number_of_LinphoneCallEnd + 4, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased, focus_stat.number_of_LinphoneCallReleased + 4, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated, focus_stat.number_of_LinphoneSubscriptionTerminated + 4, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed, focus_stat.number_of_participants_removed + 4, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed, focus_stat.number_of_participant_devices_removed + 4, 10000));

		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminationPending, focus_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminated, focus_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateDeleted, focus_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		for (auto mgr : {focus.getCMgr()}) {
			LinphoneConference * pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), (dialout) ? 0 : 1, int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), (dialout) ? 0 : 1, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void(*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
			}
		}

		const bctbx_list_t * calls = linphone_core_get_calls(focus.getLc());
		BC_ASSERT_EQUAL(bctbx_list_size(calls), 1, size_t, "%zu");

		LinphoneCall * focus_lise_call=linphone_core_get_call_by_remote_address2(focus.getLc(), lise.getCMgr()->identity);
		BC_ASSERT_PTR_NOT_NULL(focus_lise_call);
		if (focus_lise_call) {
			linphone_call_terminate(focus_lise_call);
		}

		if (!dialout) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed, focus_stat.number_of_participants_removed + 5, 10000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed, focus_stat.number_of_participant_devices_removed + 5, 10000));
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd, focus_stat.number_of_LinphoneCallEnd + 5, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased, focus_stat.number_of_LinphoneCallReleased + 5, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_LinphoneCallEnd, 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &lise.getStats().number_of_LinphoneCallReleased, 1, 10000));

		// Explicitely terminate conference as those on server are static by default
		if (fconference) {
			linphone_conference_terminate(fconference);
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending, 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1, 10000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1, 10000));

		for (auto mgr : members) {
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

			LinphoneConferenceInfo * info = linphone_core_find_conference_information_from_uri(mgr->lc, confAddr);
			if (BC_ASSERT_PTR_NOT_NULL(info)) {
				BC_ASSERT_TRUE(linphone_address_weak_equal(marie.getCMgr()->identity, linphone_conference_info_get_organizer(info)));
				BC_ASSERT_TRUE(linphone_address_weak_equal(confAddr, linphone_conference_info_get_uri(info)));

				const bctbx_list_t * info_participants = linphone_conference_info_get_participants(info);

				// Original participants + Marie who joined the conference
				BC_ASSERT_EQUAL(bctbx_list_size(info_participants), (dialout) ? 4 : 5, size_t, "%zu");
				bctbx_list_free((bctbx_list_t *)info_participants);

				BC_ASSERT_GREATER_STRICT((long long)linphone_conference_info_get_date_time(info), 0, long long, "%lld");
				const int duration_m = linphone_conference_info_get_duration(info);
				BC_ASSERT_EQUAL(duration_m, 0, int, "%d");
				if (initialSubject) {
					BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_subject(info), initialSubject);
				} else {
					BC_ASSERT_PTR_NULL(linphone_conference_info_get_subject(info));
				}
				if (send_ics || (mgr == marie.getCMgr())) {
					BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_description(info), description);
				} else {
					BC_ASSERT_PTR_NULL(linphone_conference_info_get_description(info));
				}
				linphone_conference_info_unref(info);
			}
		}

		//wait bit more to detect side effect if any
		CoreManagerAssert({focus,marie,pauline,laure,michelle,lise}).waitUntil(chrono::seconds(2),[] {
			return false;
		});

		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void create_dial_out_conference_with_active_call (void) {
	create_conference_with_active_call_base(TRUE);
}

static void create_scheduled_conference_with_active_call (void) {
	create_conference_with_active_call_base (FALSE);
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
	TEST_NO_TAG("Group chat Lime Server chat room encrypted message", LinphoneTest::group_chat_room_lime_server_encrypted_message),
	TEST_NO_TAG("Group chat Lime Server chat room send imdn error", LinphoneTest::group_chat_room_lime_session_corrupted),
	TEST_NO_TAG("Group chat Lime Server chat room clear message", LinphoneTest::group_chat_room_lime_server_clear_message),
	TEST_ONE_TAG("Multi domain chatroom", LinphoneTest::multidomain_group_chat_room,"LeaksMemory") /* because of coreMgr restart*/
};

static test_t local_conference_scheduled_conference_tests[] = {
	TEST_NO_TAG("Call to inexisting conference address", LinphoneTest::call_to_inexisting_conference_address),
	TEST_NO_TAG("Create simple conference", LinphoneTest::create_simple_conference),
	TEST_NO_TAG("Organizer edits simple conference", LinphoneTest::organizer_edits_simple_conference),
	TEST_NO_TAG("Organizer edits simple conference using different account", LinphoneTest::organizer_edits_simple_conference_using_different_account),
	TEST_NO_TAG("Participant edits simple conference", LinphoneTest::participant_edits_simple_conference),
	TEST_NO_TAG("Participant edits simple conference using different account", LinphoneTest::participant_edits_simple_conference_using_different_account),
	TEST_NO_TAG("Create simple conference with server restart", LinphoneTest::create_simple_conference_with_server_restart),
	TEST_NO_TAG("Create simple conference with update deferred", LinphoneTest::create_simple_conference_with_update_deferred),
	TEST_ONE_TAG("Create simple conference with client restart", LinphoneTest::create_simple_conference_with_client_restart, "LeaksMemory"), /* because of coreMgr restart*/
	TEST_NO_TAG("Create conference with uninvited participant", LinphoneTest::create_conference_with_uninvited_participant),
	TEST_NO_TAG("Create conference with uninvited participant not allowed", LinphoneTest::create_conference_with_uninvited_participant_not_allowed),
	TEST_NO_TAG("Create conference starting immediately", LinphoneTest::create_conference_starting_immediately),
	TEST_NO_TAG("Create conference starting in the past", LinphoneTest::create_conference_starting_in_the_past),
	TEST_NO_TAG("Create conference with late participant addition", LinphoneTest::create_conference_with_late_participant_addition),
	TEST_NO_TAG("Create conference with late participant addition declined", LinphoneTest::create_conference_with_late_participant_addition_declined),
#if 0
	TEST_NO_TAG("Conference with participants added after conference end", LinphoneTest::conference_with_participants_added_after_end),
	TEST_NO_TAG("Conference with participants added before conference start", LinphoneTest::conference_with_participants_added_before_start),
#endif
	TEST_NO_TAG("Create simple conference with audio only participant", LinphoneTest::create_simple_conference_with_audio_only_participant),
	TEST_NO_TAG("Create conference with audio only and uninvited participant", LinphoneTest::create_conference_with_audio_only_and_uninvited_participant),
	TEST_NO_TAG("Create simple conference with audio only participant enabling video", LinphoneTest::create_simple_conference_with_audio_only_participant_enabling_video),
	TEST_NO_TAG("Create one participant conference toggles video in grid layout", LinphoneTest::one_participant_conference_toggles_video_grid),
	TEST_NO_TAG("Create one participant conference toggles video in active speaker layout", LinphoneTest::one_participant_conference_toggles_video_active_speaker),
	TEST_NO_TAG("Organizer schedules 2 conferences", LinphoneTest::organizer_schedule_two_conferences),
	TEST_NO_TAG("2 overlapping conferences from difference organizers", LinphoneTest::two_overlapping_conferences_from_difference_organizers),
	TEST_NO_TAG("Create scheduled conference with active call", LinphoneTest::create_scheduled_conference_with_active_call)
};

static test_t local_conference_scheduled_ice_conference_tests[] = {
	TEST_NO_TAG("Create simple ICE conference", LinphoneTest::create_simple_ice_conference),
	TEST_NO_TAG("Create simple STUN+ICE conference", LinphoneTest::create_simple_stun_ice_conference),
	TEST_NO_TAG("Create simple SRTP conference", LinphoneTest::create_simple_srtp_conference),
	TEST_NO_TAG("Create simple ICE SRTP conference", LinphoneTest::create_simple_ice_srtp_conference),
	TEST_NO_TAG("Create simple STUN+ICE SRTP conference", LinphoneTest::create_simple_stun_ice_srtp_conference),
	TEST_NO_TAG("Create simple ICE conference with audio only participant", LinphoneTest::create_simple_ice_conference_with_audio_only_participant),
	TEST_NO_TAG("Create simple STUN+ICE conference with audio only participant", LinphoneTest::create_simple_stun_ice_conference_with_audio_only_participant),
	TEST_NO_TAG("Create simple STUN+ICE SRTP conference with audio only participant", LinphoneTest::create_simple_stun_ice_srtp_conference_with_audio_only_participant),
	TEST_ONE_TAG("Abort call to ICE conference", LinphoneTest::abort_call_to_ice_conference, "LeaksMemory") /* because of aborted calls*/
};

static test_t local_conference_inpromptu_conference_tests[] = {
	TEST_NO_TAG("Create simple dial out conference", LinphoneTest::create_simple_conference_dial_out),
	TEST_NO_TAG("Create simple dial out conference and ICS sent", LinphoneTest::create_simple_conference_dial_out_and_ics),
	TEST_NO_TAG("Create simple dial out conference with calls declined", LinphoneTest::create_simple_conference_dial_out_with_calls_declined),
	TEST_NO_TAG("Create simple dial out conference with some calls declined", LinphoneTest::create_simple_conference_dial_out_with_some_calls_declined),
	TEST_NO_TAG("Create simple dial out conference with some calls busy", LinphoneTest::create_simple_conference_dial_out_with_some_calls_busy),
	TEST_NO_TAG("Create simple dial out conference with late participant addition", LinphoneTest::create_simple_conference_dial_out_with_late_participant_addition),
	TEST_NO_TAG("Create simple dial out conference with many late participant additions", LinphoneTest::create_simple_conference_dial_out_with_many_late_participant_additions),
	TEST_ONE_TAG("Create simple conference by merging calls", LinphoneTest::create_simple_conference_merging_calls, "LeaksMemory"), /* because of aborted calls*/
	TEST_ONE_TAG("Create simple conference by merging calls with video toggling", LinphoneTest::create_simple_conference_merging_calls_with_video_toggling, "LeaksMemory"), /* because of aborted calls*/
	TEST_ONE_TAG("Create simple ICE conference by merging calls", LinphoneTest::create_simple_ice_conference_merging_calls, "LeaksMemory"), /* because of aborted calls*/
	TEST_NO_TAG("Create dial out conference with active call", LinphoneTest::create_dial_out_conference_with_active_call)
};

test_suite_t local_conference_test_suite_chat = {
	"Local conference tester (Chat)",
	NULL,
	NULL,
	liblinphone_tester_before_each,
	liblinphone_tester_after_each,
	sizeof(local_conference_chat_tests) / sizeof(local_conference_chat_tests[0]), local_conference_chat_tests
};

test_suite_t local_conference_test_suite_scheduled_conference = {
	"Local conference tester (Scheduled Conference)",
	NULL,
	NULL,
	liblinphone_tester_before_each,
	liblinphone_tester_after_each,
	sizeof(local_conference_scheduled_conference_tests) / sizeof(local_conference_scheduled_conference_tests[0]), local_conference_scheduled_conference_tests
};

test_suite_t local_conference_test_suite_scheduled_ice_conference = {
	"Local conference tester (Scheduled ICE Conference)",
	NULL,
	NULL,
	liblinphone_tester_before_each,
	liblinphone_tester_after_each,
	sizeof(local_conference_scheduled_ice_conference_tests) / sizeof(local_conference_scheduled_ice_conference_tests[0]), local_conference_scheduled_ice_conference_tests
};

test_suite_t local_conference_test_suite_inpromptu_conference = {
	"Local conference tester (Inpromptu Conference)",
	NULL,
	NULL,
	liblinphone_tester_before_each,
	liblinphone_tester_after_each,
	sizeof(local_conference_inpromptu_conference_tests) / sizeof(local_conference_inpromptu_conference_tests[0]), local_conference_inpromptu_conference_tests
};

#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic pop
#endif
