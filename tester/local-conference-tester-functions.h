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

#ifndef LOCAL_CONFERENCE_TESTER_FUNCTIONS_H_
#define LOCAL_CONFERENCE_TESTER_FUNCTIONS_H_

#include <map>

#include "bctoolbox/crypto.h"
#include "bctoolbox/ownership.hh"
#include <bctoolbox/defs.h>

#include "conference/participant.h"
#include "liblinphone_tester++.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-chat-message.h"
#include "linphone/api/c-chat-room-cbs.h"
#include "linphone/api/c-chat-room.h"
#include "linphone/api/c-participant-device-identity.h"
#include "linphone/core.h"
#include "shared_tester_functions.h"

using namespace LinphonePrivate;
using namespace std;
using namespace Linphone::Tester;
using namespace ownership;

namespace LinphoneTest {

class BcAssert {
public:
	void addCustomIterate(const std::function<void()> &iterate) {
		mIterateFuncs.push_back(iterate);
	}
	bool waitUntil(std::chrono::duration<double> timeout, const std::function<bool()> &condition) {
		auto start = std::chrono::steady_clock::now();

		bool_t result;
		while (!(result = condition()) && (std::chrono::steady_clock::now() - start < timeout)) {
			for (const std::function<void()> &iterate : mIterateFuncs) {
				iterate();
			}
			ms_usleep(100);
		}
		return result;
	}
	bool wait(const std::function<bool()> &condition) {
		return waitUntil(std::chrono::seconds(10), condition);
	}

private:
	std::list<std::function<void()>> mIterateFuncs;
};

class CoreAssert : public BcAssert {
public:
	CoreAssert(std::initializer_list<std::shared_ptr<Core>> cores) {
		for (shared_ptr<Core> core : cores) {
			addCustomIterate([core] { linphone_core_iterate(L_GET_C_BACK_PTR(core)); });
		}
	}
};

class ClientConference;

class ConfCoreManager : public CoreManager {
public:
	ConfCoreManager(std::string rc) : CoreManager(rc.c_str()) {
		mMgr->user_info = this;
	}

	ConfCoreManager(std::string rc, const std::function<void()> &preStart)
	    : CoreManager(owned(linphone_core_manager_create(rc.c_str()))), mPreStart(preStart) {
		mMgr->user_info = this;
		mPreStart();
		start(true);
	}

	void reStart(bool check_for_proxies = true) {
		linphone_core_manager_reinit(mMgr.get());
		mPreStart();
		start(check_for_proxies);
	}

	void configureCoreForConference(const Address &factoryUri) {
		_configure_core_for_conference(mMgr.get(), factoryUri.toC());
	}
	void setupMgrForConference(const char *conferenceVersion = nullptr) {
		setup_mgr_for_conference(mMgr.get(), conferenceVersion);
	}
	BorrowedMut<LinphoneChatRoom> searchChatRoom(const LinphoneAddress *localAddr,
	                                             const LinphoneAddress *remoteAddr,
	                                             const bctbx_list_t *participants = nullptr,
	                                             const LinphoneChatRoomParams *params = nullptr) {
		return borrowed_mut(linphone_core_search_chat_room(mMgr->lc, params, localAddr, remoteAddr, participants));
	}
	LinphoneAccount *getDefaultAccount() const {
		return linphone_core_get_default_account(mMgr->lc);
	}
	stats &getStats() const {
		return mMgr->stat;
	}
	LinphoneCoreManager *getCMgr() {
		return mMgr.get();
	};
	LinphoneCore *getLc() const {
		return mMgr->lc;
	};

private:
	const std::function<void()> mPreStart = [] { return; };
};

class CoreManagerAssert : public BcAssert {
public:
	CoreManagerAssert(std::initializer_list<std::reference_wrapper<CoreManager>> coreMgrs) : BcAssert() {
		for (CoreManager &coreMgr : coreMgrs) {
			addCustomIterate([&coreMgr] { coreMgr.iterate(); });
		}
	}

	CoreManagerAssert(std::list<std::reference_wrapper<CoreManager>> coreMgrs) : BcAssert() {
		for (CoreManager &coreMgr : coreMgrs) {
			addCustomIterate([&coreMgr] { coreMgr.iterate(); });
		}
	}
};
class Focus;

/*Core manager acting as a client*/
class ClientConference : public ConfCoreManager {
public:
	ClientConference(std::string rc, Address factoryUri, bool encrypted = false)
	    : ConfCoreManager(rc,
	                      [this, factoryUri, encrypted] {
		                      configureCoreForConference(factoryUri);
		                      _configure_core_for_audio_video_conference(mMgr.get(), factoryUri.toC());
		                      setupMgrForConference();
		                      LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
		                      linphone_core_cbs_set_chat_room_state_changed(cbs, core_chat_room_state_changed);
		                      linphone_core_cbs_set_chat_room_subject_changed(cbs, core_chat_room_subject_changed);
		                      linphone_core_cbs_set_message_sent(cbs, encrypted_message_sent);
		                      linphone_core_cbs_set_message_received(cbs, encrypted_message_received);
		                      linphone_core_add_callbacks(getLc(), cbs);
		                      linphone_core_cbs_unref(cbs);
		                      if (encrypted) {
			                      set_lime_server_and_curve(25519, mMgr.get());
		                      }
	                      }),
	      mFocus(nullptr) {
	}

	void deleteChatRoomSync(AbstractChatRoom &chatroom) {
		linphone_core_delete_chat_room(getLc(), chatroom.toC());
		CoreManagerAssert({*mFocus, *this}).wait([&chatroom] {
			return chatroom.getState() == ConferenceInterface::State::Deleted;
		});
	}

	~ClientConference() {
		for (auto chatRoom : getCore().getChatRooms()) {
			deleteChatRoomSync(*chatRoom);
		}
	}

	static LinphoneChatMessage *sendTextMsg(LinphoneChatRoom *cr, const std::string text) {
		LinphoneChatMessage *msg = nullptr;
		if (cr && !text.empty()) {
			lInfo() << " Chat room " << AbstractChatRoom::toCpp(cr)->getConferenceId()
			        << " is sending message with text " << text;
			msg = linphone_chat_room_create_message_from_utf8(cr, text.c_str());
			BC_ASSERT_PTR_NOT_NULL(msg);
			LinphoneChatMessageCbs *cbs = linphone_factory_create_chat_message_cbs(linphone_factory_get());
			linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
			linphone_chat_message_add_callbacks(msg, cbs);
			linphone_chat_message_cbs_unref(cbs);
			linphone_chat_message_send(msg);
		}
		return msg;
	}

	static void deleteAllDevices(std::shared_ptr<Participant> &participant) {
		if (participant) {
			participant->clearDevices();
		}
	}

	static void
	encrypted_message_sent(BCTBX_UNUSED(LinphoneCore *lc), LinphoneChatRoom *room, LinphoneChatMessage *msg) {
		LinphoneChatRoomCapabilitiesMask capabilities = linphone_chat_room_get_capabilities(room);
		std::shared_ptr<ChatMessage> cppMsg = L_GET_CPP_PTR_FROM_C_OBJECT(msg);
		Content internalContent = cppMsg->getInternalContent();
		ContentType contentType = internalContent.getContentType();
		if ((capabilities & LinphoneChatRoomCapabilitiesEncrypted) && (contentType != ContentType::ImIsComposing)) {
			std::string contentEncoding = internalContent.getContentEncoding();
			BC_ASSERT_TRUE(contentEncoding.compare("deflate") == 0);
		}
	}

	static void
	encrypted_message_received(BCTBX_UNUSED(LinphoneCore *lc), LinphoneChatRoom *room, LinphoneChatMessage *msg) {
		LinphoneChatRoomCapabilitiesMask capabilities = linphone_chat_room_get_capabilities(room);
		std::shared_ptr<ChatMessage> cppMsg = L_GET_CPP_PTR_FROM_C_OBJECT(msg);
		Content internalContent = cppMsg->getInternalContent();
		ContentType contentType = internalContent.getContentType();
		if ((capabilities & LinphoneChatRoomCapabilitiesEncrypted) && (contentType != ContentType::ImIsComposing)) {
			// Header "Content-Encoding" is remove by belle-sip in function uncompress_body_if_required. Nonetheless, it
			// adds a custom header "X-BelleSip-Removed-Content-Encoding" with the same value
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_custom_header(msg, "X-BelleSip-Removed-Content-Encoding"),
			                       "deflate");
		}
	}

	friend Focus;

protected:
	void setFocus(BorrowedMut<CoreManager> myFocus) {
		mFocus = myFocus;
	}

private:
	BorrowedMut<CoreManager> mFocus;
};

/* Core manager acting as a focus*/
class Focus : public ConfCoreManager {
public:
	Focus(std::string rc) : ConfCoreManager(rc, [this] { linphone_core_enable_conference_server(getLc(), TRUE); }) {
		configureFocus();
	}
	~Focus() {
		CoreManagerAssert({*this}).waitUntil(chrono::seconds(1), [] { return false; });
	}

	void registerAsParticipantDevice(ClientConference &otherMgr) {
		const bctbx_list_t *accounts = linphone_core_get_account_list(otherMgr.getLc());
		for (const bctbx_list_t *accountIt = accounts; accountIt != NULL; accountIt = accountIt->next) {
			LinphoneAccount *account = (LinphoneAccount *)bctbx_list_get_data(accountIt);
			const LinphoneAddress *cAddr = linphone_account_get_contact_address(account);
			if (cAddr) {
				Address participantDevice = Address::toCpp(cAddr)->getUri();
				Address participant = participantDevice.getUriWithoutGruu();
				mParticipantDevices.insert({participant, std::reference_wrapper<ClientConference>(otherMgr)});
			}
		}
		// to allow client conference to delete chatroom in its destructor
		otherMgr.setFocus(borrowed_mut(this));
	}

	void subscribeParticipantDevice(const LinphoneAddress *conferenceAddress,
	                                const LinphoneAddress *participantDevice) {
		auto cr = searchChatRoom(conferenceAddress, conferenceAddress);
		BC_ASSERT_PTR_NOT_NULL(cr);
		//	CALL_CHAT_ROOM_CBS(cr, ParticipantRegistrationSubscriptionRequested,
		// participant_registration_subscription_requested, cr, participantDevice)
		_linphone_chat_room_notify_participant_registration_subscription_requested(cr, participantDevice);
	}

	void notifyParticipantDeviceRegistration(const LinphoneAddress *conferenceAddress,
	                                         const LinphoneAddress *participantDevice) {
		auto cr = searchChatRoom(conferenceAddress, conferenceAddress);
		BC_ASSERT_PTR_NOT_NULL(cr);
		linphone_chat_room_notify_participant_device_registration(cr, participantDevice);
	}

	void reStart(bool check_for_proxies = TRUE) {
		ConfCoreManager::reStart(check_for_proxies);
		configureFocus();
	}

	const Address getConferenceFactoryAddress() const {
		LinphoneAccount *account = linphone_core_get_default_account(getLc());
		const LinphoneAccountParams *account_params = linphone_account_get_params(account);
		const LinphoneAddress *factory_uri = linphone_account_params_get_conference_factory_address(account_params);
		return *Address::toCpp(factory_uri);
	}

private:
	/*
	    static void server_core_chat_room_conference_address_generation(LinphoneChatRoom *cr) {
	        Focus *focus =
	            (Focus *)(((LinphoneCoreManager
	   *)linphone_core_get_user_data(linphone_chat_room_get_core(cr)))->user_info); char config_id[6];
	        belle_sip_random_token(config_id, sizeof(config_id));
	        LinphoneAddress *conference_address =
	            linphone_address_clone(linphone_account_get_contact_address(focus->getDefaultAccount()));
	        linphone_address_set_uri_param(conference_address, "conf-id", config_id);
	        linphone_chat_room_set_conference_address(cr, conference_address);
	        linphone_address_unref(conference_address);
	    }
	*/

	static void
	server_core_chat_room_state_changed(LinphoneCore *core, LinphoneChatRoom *cr, LinphoneChatRoomState state) {
		Focus *focus = (Focus *)(((LinphoneCoreManager *)linphone_core_get_user_data(core))->user_info);
		switch (state) {
			case LinphoneChatRoomStateInstantiated: {
				LinphoneChatRoomCbs *cbs = linphone_factory_create_chat_room_cbs(linphone_factory_get());
				linphone_chat_room_cbs_set_participant_registration_subscription_requested(
				    cbs, chat_room_participant_registration_subscription_requested);
				//				linphone_chat_room_cbs_set_conference_address_generation(
				//				    cbs, server_core_chat_room_conference_address_generation);
				setup_chat_room_callbacks(cbs);
				linphone_chat_room_add_callbacks(cr, cbs);
				linphone_chat_room_cbs_set_user_data(cbs, focus);
				linphone_chat_room_cbs_unref(cbs);
				break;
			}
			default:
				break;
		}
	}

	static void chat_room_participant_registration_subscription_requested(LinphoneChatRoom *cr,
	                                                                      const LinphoneAddress *participantAddr) {
		BC_ASSERT_PTR_NOT_NULL(participantAddr);
		if (participantAddr) {
			Address participant = Address::toCpp(participantAddr)->getUri();
			BC_ASSERT_TRUE(participant.isValid());
			if (participant.isValid()) {
				Focus *focus =
				    (Focus *)(linphone_chat_room_cbs_get_user_data(linphone_chat_room_get_current_callbacks(cr)));
				bctbx_list_t *devices = NULL;
				auto participantRange = focus->mParticipantDevices.equal_range(participant);
				for (auto participantIt = participantRange.first; participantIt != participantRange.second;
				     participantIt++) {
					ClientConference &client = participantIt->second;
					bctbx_list_t *specs = linphone_core_get_linphone_specs_list(client.getLc());
					bool groupchat_enabled = false;
					for (const bctbx_list_t *specIt = specs; specIt != NULL; specIt = specIt->next) {
						const char *spec = (const char *)bctbx_list_get_data(specIt);
						// Search for "groupchat" string in the capability
						groupchat_enabled |= (strstr(spec, "groupchat") != NULL);
					}
					if (groupchat_enabled) {
						LinphoneAccount *account = linphone_core_lookup_known_account(client.getLc(), participantAddr);
						const LinphoneAddress *deviceAddr = linphone_account_get_contact_address(account);
						LinphoneParticipantDeviceIdentity *identity =
						    linphone_factory_create_participant_device_identity(linphone_factory_get(), deviceAddr, "");
						linphone_participant_device_identity_set_capability_descriptor_2(identity, specs);
						devices = bctbx_list_append(devices, identity);
					}
					bctbx_list_free_with_data(specs, ms_free);
				}
				linphone_chat_room_set_participant_devices(cr, participant.toC(), devices);
				bctbx_list_free_with_data(devices, (bctbx_list_free_func)belle_sip_object_unref);
			}
		}
	}

	void configureFocus() {
		LinphoneCoreCbs *cbs = linphone_core_get_first_callbacks(getLc());
		linphone_config_set_int(linphone_core_get_config(getLc()), "misc", "hide_empty_chat_rooms", 0);
		linphone_config_set_int(linphone_core_get_config(getLc()), "sip", "reject_duplicated_calls", 0);
		linphone_config_set_int(linphone_core_get_config(getLc()), "misc", "hide_chat_rooms_from_removed_proxies", 0);
		linphone_core_enable_rtp_bundle(getLc(), TRUE);
		linphone_core_set_conference_cleanup_period(getLc(), 1);

		LinphoneAccount *account = linphone_core_get_default_account(getLc());
		const LinphoneAccountParams *account_params = linphone_account_get_params(account);
		LinphoneAccountParams *new_account_params = linphone_account_params_clone(account_params);
		linphone_account_params_enable_rtp_bundle(new_account_params, TRUE);
		Address factoryAddress = getIdentity();
		linphone_account_params_set_conference_factory_address(new_account_params, factoryAddress.toC());
		linphone_account_set_params(account, new_account_params);
		linphone_account_params_unref(new_account_params);
		BC_ASSERT_TRUE(linphone_account_params_rtp_bundle_enabled(linphone_account_get_params(account)));

		linphone_core_cbs_set_subscription_state_changed(cbs, linphone_subscription_state_change);
		linphone_core_cbs_set_chat_room_state_changed(cbs, server_core_chat_room_state_changed);
		linphone_core_cbs_set_message_sent(cbs, encrypted_message_sent);
		//		linphone_core_cbs_set_refer_received(cbs, linphone_conference_server_refer_received);
	}

	static void
	encrypted_message_sent(BCTBX_UNUSED(LinphoneCore *lc), LinphoneChatRoom *room, LinphoneChatMessage *msg) {
		LinphoneChatRoomCapabilitiesMask capabilities = linphone_chat_room_get_capabilities(room);
		std::shared_ptr<ChatMessage> cppMsg = L_GET_CPP_PTR_FROM_C_OBJECT(msg);
		Content internalContent = cppMsg->getInternalContent();
		ContentType contentType = internalContent.getContentType();
		if ((capabilities & LinphoneChatRoomCapabilitiesEncrypted) && (contentType != ContentType::ImIsComposing)) {
			check_chat_message_properties(msg);
		}
	}

	std::multimap<Address, std::reference_wrapper<ClientConference>> mParticipantDevices;
};

// Chat rooms
void group_chat_room_server_admin_managed_messages_base(bool_t encrypted);
void group_chat_room_lime_server_message(bool encrypted);
void group_chat_room_with_client_restart_base(bool encrypted);
void group_chat_room_with_sip_errors_base(bool invite_error, bool subscribe_error, bool encrypted);
void one_to_one_group_chat_room_deletion_by_server_client_base(bool encrypted);
void sendEphemeralMessageInAdminMode(Focus &focus,
                                     ClientConference &sender,
                                     ClientConference &recipient,
                                     LinphoneChatRoom *senderCr,
                                     LinphoneChatRoom *recipientCr,
                                     const std::string basicText,
                                     const int noMsg);

bool checkChatroom(Focus &focus, const ConfCoreManager &core, const time_t baseJoiningTime);

// Conference
void create_simple_conference_merging_calls_base(bool_t enable_ice,
                                                 LinphoneConferenceLayout layout,
                                                 bool_t toggle_video,
                                                 bool_t toggle_all_mananger_video,
                                                 bool_t change_layout,
                                                 LinphoneConferenceSecurityLevel security_level,
                                                 bool_t enable_screen_sharing);

void create_conference_base(time_t start_time,
                            int duration,
                            bool_t uninvited_participant_dials,
                            LinphoneConferenceParticipantListType participant_list_type,
                            bool_t remove_participant,
                            const LinphoneMediaEncryption encryption,
                            bool_t enable_video,
                            LinphoneConferenceLayout layout,
                            bool_t enable_ice,
                            bool_t enable_stun,
                            bool_t audio_only_participant,
                            bool_t server_restart,
                            bool_t client_restart,
                            bool_t do_not_use_proxy,
                            LinphoneMediaDirection video_direction,
                            bool_t network_restart,
                            LinphoneConferenceSecurityLevel security_level,
                            std::list<LinphoneParticipantRole> allowedRoles,
                            bool_t add_participant_after_end);

void create_conference_with_screen_sharing_base(time_t start_time,
                                                int duration,
                                                const LinphoneMediaEncryption encryption,
                                                LinphoneConferenceLayout layout,
                                                bool_t enable_video,
                                                bool_t enable_camera,
                                                bool_t turn_off_screen_sharing,
                                                LinphoneMediaDirection video_direction,
                                                LinphoneConferenceSecurityLevel security_level,
                                                std::list<LinphoneParticipantRole> allowedRoles);

void wait_for_conference_streams(std::initializer_list<std::reference_wrapper<CoreManager>> coreMgrs,
                                 std::list<LinphoneCoreManager *> conferenceMgrs,
                                 LinphoneCoreManager *focus,
                                 std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> members,
                                 const LinphoneAddress *confAddr,
                                 bool_t enable_video);

void two_overlapping_conferences_base(bool_t same_organizer, bool_t dialout);

void create_conference_with_late_participant_addition_base(time_t start_time,
                                                           int duration,
                                                           LinphoneConferenceLayout layout,
                                                           LinphoneConferenceParticipantListType participant_list_type,
                                                           bool_t accept,
                                                           bool_t one_addition,
                                                           LinphoneConferenceSecurityLevel security_level);

void create_one_participant_conference_toggle_video_base(LinphoneConferenceLayout layout,
                                                         bool_t enable_ice,
                                                         bool_t enable_stun);

void create_conference_with_active_call_base(bool_t dialout);

LinphoneAddress *
create_conference_on_server(Focus &focus,
                            ClientConference &organizer,
                            std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> requested_participants,
                            time_t start_time,
                            time_t end_time,
                            const char *subject,
                            const char *description,
                            bool_t send_ics,
                            LinphoneConferenceSecurityLevel security_level,
                            bool_t enable_video,
                            bool_t enable_chat);

void set_video_settings_in_conference(LinphoneCoreManager *focus,
                                      LinphoneCoreManager *participant,
                                      std::list<LinphoneCoreManager *> members,
                                      const LinphoneAddress *confAddr,
                                      bool_t enable_video,
                                      LinphoneMediaDirection video_direction,
                                      bool_t answer_enable_video,
                                      LinphoneMediaDirection answer_video_direction);

size_t compute_no_video_streams(bool_t enable_video, LinphoneCall *call, LinphoneConference *conference);

std::map<LinphoneCoreManager *, LinphoneParticipantInfo *>
fill_memmber_list(std::list<LinphoneCoreManager *> members,
                  std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList,
                  LinphoneCoreManager *organizer,
                  bctbx_list_t *participants_info);

size_t compute_no_audio_streams(LinphoneCall *call, LinphoneConference *conference);

void update_sequence_number(bctbx_list_t **participants_info,
                            const std::list<LinphoneAddress *> new_participants,
                            int exp_sequence,
                            int exp_new_participant_sequence);

void create_conference_dial_out_base(bool_t send_ics,
                                     LinphoneConferenceLayout layout,
                                     LinphoneVideoActivationPolicy *pol,
                                     bool_t enable_stun,
                                     bool_t enable_ice,
                                     LinphoneConferenceParticipantListType participant_list_type,
                                     bool_t accept,
                                     bool_t participant_codec_mismatch,
                                     LinphoneConferenceSecurityLevel security_level);

void create_conference_with_audio_only_participants_base(LinphoneConferenceSecurityLevel security_level);

void create_simple_conference_dial_out_with_some_calls_declined_base(LinphoneReason reason,
                                                                     LinphoneConferenceSecurityLevel securityLevel);

void change_active_speaker_base(bool transfer_mode);
} // namespace LinphoneTest

#endif // LOCAL_CONFERENCE_TESTER_FUNCTIONS_H_
