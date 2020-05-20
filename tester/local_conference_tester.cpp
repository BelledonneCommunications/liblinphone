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
#include "linphone/api/c-chat-room-params.h"
#include "tester_utils.h"
#include "linphone/wrapper_utils.h"
#include "liblinphone_tester.h"
#include "bctoolbox/crypto.h"
#include <map>
#include "address/identity-address.h"
#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "chat/chat-room/chat-room.h"

#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic push
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

using namespace LinphonePrivate;
namespace LinphoneTest {
class CoreManager {
public:
	CoreManager(std::string rc) {
		mMgr = linphone_core_manager_new(rc.c_str());
		mMgr->user_info = this;
	}

	CoreManager(std::string rc,const std::function<void ()> &preStart) {
		mMgr = linphone_core_manager_create(rc.c_str());
		mMgr->user_info = this;
		preStart();
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
	IdentityAddress getIdentity() {
		return *L_GET_CPP_PTR_FROM_C_OBJECT(mMgr->identity);
	}
	stats & getStats() {
		return mMgr->stat;
	}
	LinphoneCoreManager * getCMgr() {
		return mMgr;
	}
protected:
	LinphoneCoreManager * mMgr;
};


/* Core manager acting as a focus*/
class Focus : public CoreManager {
public:
	Focus(std::string rc): CoreManager(rc) {
		linphone_core_enable_conference_server(getLc(),TRUE);
		linphone_core_cbs_set_chat_room_state_changed(linphone_core_get_first_callbacks(getLc()), server_core_chat_room_state_changed);
		linphone_proxy_config_set_conference_factory_uri(linphone_core_get_default_proxy_config(getLc()), getIdentity().asString().c_str());
	}
	~Focus() = default ;
	
	void registerAsParticipantDevice(const CoreManager &otherMgr) {
		const LinphoneAddress *cAddr = linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(otherMgr.getLc()));
		IdentityAddress participantDevice(*L_GET_CPP_PTR_FROM_C_OBJECT(cAddr));
		IdentityAddress participant = participantDevice.getAddressWithoutGruu();
		mParticipantDevices.insert({participant,participantDevice});
	}
private:
	static void server_core_chat_room_state_changed (LinphoneCore *core, LinphoneChatRoom *cr, LinphoneChatRoomState state) {
		Focus *focus = (Focus*)(((LinphoneCoreManager *)linphone_core_get_user_data(core))->user_info);
		switch (state) {
			case LinphoneChatRoomStateInstantiated: {
			if (linphone_chat_room_get_conference_address(cr) == NULL) {
				char config_id[6];
				belle_sip_random_token(config_id,sizeof(config_id));
				const LinphoneAddress *cAddr = linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(focus->getLc()));
				Address conference_address = (*L_GET_CPP_PTR_FROM_C_OBJECT(cAddr));
				conference_address.setUriParam("conf-id",config_id);
				linphone_chat_room_set_conference_address(cr, L_GET_C_BACK_PTR(&conference_address));
				LinphoneChatRoomCbs *cbs = linphone_factory_create_chat_room_cbs(linphone_factory_get());
				linphone_chat_room_cbs_set_participant_registration_subscription_requested(cbs, participant_registration_subscription_requested);
				linphone_chat_room_add_callbacks(cr, cbs);
				linphone_chat_room_cbs_set_user_data(cbs, focus);
				linphone_chat_room_cbs_unref(cbs);
			}
				break;
		}
			default:
				break;
		}
	}
	static void participant_registration_subscription_requested(LinphoneChatRoom *cr, const LinphoneAddress *participantAddr) {
		Focus *focus = (Focus*)(linphone_chat_room_cbs_get_user_data(linphone_chat_room_get_current_callbacks(cr)));
		const IdentityAddress participant = *L_GET_CPP_PTR_FROM_C_OBJECT(participantAddr);
		bctbx_list_t * devices = NULL;
		for (auto participantIt = focus->mParticipantDevices.find(participant); participantIt != focus->mParticipantDevices.end();participantIt++ ) {
			LinphoneAddress *deviceAddr = linphone_address_new(participantIt->second.asString().c_str());
			devices = bctbx_list_append(devices, linphone_factory_create_participant_device_identity(linphone_factory_get(),deviceAddr,""));
			linphone_address_unref(deviceAddr);
		}
		Address participantAddress(participant);
		linphone_chat_room_set_participant_devices(cr,L_GET_C_BACK_PTR(&participantAddress),devices);
		bctbx_list_free_with_data(devices,(bctbx_list_free_func)belle_sip_object_unref);
	}
	
	std::multimap<IdentityAddress,IdentityAddress,std::less<IdentityAddress>> mParticipantDevices;
};


/*Core manager acting as a client*/
class ClientConference :public CoreManager {
public:
	ClientConference(std::string rc,Address factoryUri):CoreManager(rc,[this,factoryUri] {
		_configure_core_for_conference(mMgr,L_GET_C_BACK_PTR(&factoryUri));
		LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
		linphone_core_cbs_set_chat_room_state_changed(cbs, core_chat_room_state_changed);
		linphone_core_cbs_set_chat_room_subject_changed(cbs, core_chat_room_subject_changed);
		linphone_core_add_callbacks(getLc(),cbs);
		linphone_core_cbs_unref(cbs);
		
	}){
		;
	}
	~ClientConference() = default;
};

static void group_chat_room_creation_server (void) {
	Focus focus("chloe_rc");
	ClientConference marie("marie_rc", focus.getIdentity());
	ClientConference pauline("pauline_rc", focus.getIdentity());

	focus.registerAsParticipantDevice(marie);
	focus.registerAsParticipantDevice(pauline);
	
	bctbx_list_t * coresList = bctbx_list_append(NULL, focus.getLc());
	coresList = bctbx_list_append(coresList, marie.getLc());
	coresList = bctbx_list_append(coresList, pauline.getLc());
	Address paulineAddr(pauline.getIdentity());
	bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(L_GET_C_BACK_PTR(&paulineAddr)));
	
	stats initialMarieStats = marie.getStats();
	stats initialPaulineStats = pauline.getStats();

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie.getCMgr(), &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &initialPaulineStats, confAddr, initialSubject, 1, FALSE);

	
	// Marie now changes the subject
	const char *newSubject = "Let's go drink a beer";
	linphone_chat_room_set_subject(marieCr, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_subject_changed, initialMarieStats.number_of_subject_changed + 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 1, 5000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);


	linphone_chat_room_leave(paulineCr);
	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie.getCMgr(), marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline.getCMgr(), paulineCr, coresList);
	bctbx_list_free(coresList);
	
}

}

static test_t local_conference_tests[] = {
	TEST_NO_TAG("Group chat room creation local server", LinphoneTest::group_chat_room_creation_server),
};

test_suite_t local_conference_test_suite = {
	"Local conference tester",
	NULL,
	NULL,
	liblinphone_tester_before_each,
	liblinphone_tester_after_each,
	sizeof(local_conference_tests) / sizeof(local_conference_tests[0]), local_conference_tests
};
#if __clang__ || ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4)
#pragma GCC diagnostic pop
#endif
