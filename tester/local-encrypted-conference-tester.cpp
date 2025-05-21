/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#include "local-conference-tester-functions.h"

#include "account/account.h"
#include "chat/encryption/encryption-engine.h"
#include "conference/encryption/client-ekt-manager.h"
#include "conference/encryption/ekt-info.h"

// =============================================================================

using namespace std;

using namespace LinphonePrivate;

// -----------------------------------------------------------------------------

namespace LinphoneTest {

enum class EktXmlContent {
	FirstNotify,    // sSPI
	SpiInfo,        // sSPI + cSPI
	CipherTransport // all fields
};

static void ekt_xml_composing_parsing_test(EktXmlContent exc) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneAccount *marieAccount = linphone_core_get_default_account(marie->lc);
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");
	LinphoneAccount *paulineAccount = linphone_core_get_default_account(pauline->lc);
	string paulineAddr = Account::toCpp(paulineAccount)->getContactAddress()->asStringUriOnly();
	LinphoneCoreManager *laure = linphone_core_manager_new("laure_tcp_rc");
	LinphoneAccount *laureAccount = linphone_core_get_default_account(laure->lc);
	string laureAddr = Account::toCpp(laureAccount)->getContactAddress()->asStringUriOnly();
	bctoolbox::RNG rng = bctoolbox::RNG();

	shared_ptr<EktInfo> ei = make_shared<EktInfo>();

	vector<uint8_t> sSpiVector;
	uint16_t sspi = 0;
	do {
		sSpiVector = rng.randomize(2);
		memcpy(&sspi, &sSpiVector[0], sizeof(uint16_t));
	} while (sspi == 0);
	ei->setSSpi(sspi);

	vector<uint8_t> cspi = rng.randomize(16);
	vector<uint8_t> paulineCipher = rng.randomize(16);
	vector<uint8_t> laureCipher = rng.randomize(16);
	if (exc == EktXmlContent::SpiInfo || exc == EktXmlContent::CipherTransport) {
		ei->setCSpi(cspi);
		if (exc == EktXmlContent::CipherTransport) {
			ei->setFrom(*Account::toCpp(marieAccount)->getContactAddress());
			ei->addCipher(paulineAddr, paulineCipher);
			ei->addCipher(laureAddr, laureCipher);
		}
	}

	string xmlBody = L_GET_CPP_PTR_FROM_C_OBJECT(marie->lc)->createXmlFromEktInfo(
	    ei, Account::toCpp(marieAccount)->getSharedFromThis());
	lInfo() << "Generated XML body : " << endl << xmlBody;
	auto outputEi = L_GET_CPP_PTR_FROM_C_OBJECT(marie->lc)->createEktInfoFromXml(xmlBody);

	BC_ASSERT_EQUAL(outputEi->getSSpi(), ei->getSSpi(), uint16_t, "%u");
	if (exc == EktXmlContent::SpiInfo || exc == EktXmlContent::CipherTransport) {
		BC_ASSERT_EQUAL(outputEi->getCSpi().size(), ei->getCSpi().size(), int, "%i");
		BC_ASSERT_TRUE(outputEi->getCSpi() == ei->getCSpi());
		if (exc == EktXmlContent::CipherTransport) {
			auto outputFromAddress = outputEi->getFrom();
			auto eiFromAddress = ei->getFrom();
			BC_ASSERT_PTR_NOT_NULL(outputFromAddress);
			BC_ASSERT_PTR_NOT_NULL(eiFromAddress);
			if (outputFromAddress && eiFromAddress) {
				BC_ASSERT_TRUE(outputEi->getFrom()->asStringUriOnly() == ei->getFrom()->asStringUriOnly());
				auto outputCipher = outputEi->getCiphers()->getBuffer(paulineAddr)->getContent();
				auto eiCipher = ei->getCiphers()->getBuffer(paulineAddr)->getContent();
				BC_ASSERT_TRUE(outputCipher == eiCipher);
			}
		}
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void first_notify_ekt_xml_composing_parsing_test() {
	ekt_xml_composing_parsing_test(EktXmlContent::FirstNotify);
}

static void spi_info_ekt_xml_composing_parsing_test() {
	ekt_xml_composing_parsing_test(EktXmlContent::SpiInfo);
}

static void cipher_transport_ekt_xml_composing_parsing_test() {
	ekt_xml_composing_parsing_test(EktXmlContent::CipherTransport);
}

static void create_simple_end_to_end_encrypted_conference() {
	create_conference_base(ms_time(nullptr), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionSRTP, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, FALSE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
	                       FALSE);
}

static void create_simple_end_to_end_encrypted_conference_with_server_restart() {
	create_conference_base(ms_time(nullptr), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionDTLS, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, TRUE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, TRUE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
	                       FALSE);
}

static void create_simple_end_to_end_encrypted_conference_with_client_restart() {
	create_conference_base(ms_time(nullptr), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionZRTP, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE,
	                       TRUE, FALSE, LinphoneMediaDirectionRecvOnly, TRUE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
	                       FALSE);
}

static void create_end_to_end_encrypted_conference_with_uninvited_participant() {
	create_conference_base(ms_time(nullptr), -1, TRUE, LinphoneConferenceParticipantListTypeOpen, TRUE,
	                       LinphoneMediaEncryptionSRTP, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, TRUE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
	                       FALSE);
}

static void create_end_to_end_encrypted_conference_with_uninvited_participant_not_allowed() {
	create_conference_base(
	    ms_time(nullptr), -1, TRUE, LinphoneConferenceParticipantListTypeClosed, FALSE, LinphoneMediaEncryptionDTLS,
	    FALSE, LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	    LinphoneMediaDirectionSendRecv, FALSE, LinphoneConferenceSecurityLevelEndToEnd,
	    {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE, FALSE);
}

static void create_end_to_end_encrypted_conference_starting_immediately() {
	create_conference_base(ms_time(nullptr), 0, FALSE, LinphoneConferenceParticipantListTypeClosed, FALSE,
	                       LinphoneMediaEncryptionZRTP, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, FALSE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
	                       FALSE);
}

static void create_end_to_end_encrypted_conference_starting_in_the_past() {
	create_conference_base(
	    ms_time(nullptr) - 640, 11, FALSE, LinphoneConferenceParticipantListTypeClosed, TRUE,
	    LinphoneMediaEncryptionSRTP, FALSE, LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE, FALSE, FALSE, FALSE,
	    FALSE, LinphoneMediaDirectionSendRecv, FALSE, LinphoneConferenceSecurityLevelEndToEnd,
	    {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE, FALSE);
}

static void create_simple_end_to_end_encrypted_conference_with_audio_only_participant() {
	create_conference_base(ms_time(nullptr), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionDTLS, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, TRUE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionSendRecv, FALSE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
	                       FALSE);
}

static void create_end_to_end_encrypted_conference_with_audio_only_and_uninvited_participant() {
	create_conference_base(ms_time(nullptr), -1, TRUE, LinphoneConferenceParticipantListTypeOpen, TRUE,
	                       LinphoneMediaEncryptionZRTP, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, TRUE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionSendRecv, FALSE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
	                       FALSE);
}

static void create_simple_end_to_end_encrypted_conference_with_audio_only_participant_enabling_video() {
	create_conference_base(ms_time(nullptr), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionSRTP, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, TRUE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionSendRecv, FALSE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
	                       FALSE);
}

static void create_simple_end_to_end_encrypted_ice_conference() {
	create_conference_base(ms_time(nullptr), -1, TRUE, LinphoneConferenceParticipantListTypeOpen, TRUE,
	                       LinphoneMediaEncryptionDTLS, TRUE, LinphoneConferenceLayoutGrid, TRUE, FALSE, FALSE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, FALSE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
	                       FALSE);
}

static void create_end_to_end_encrypted_conference_terminate_call_on_version_mismatch() {
	create_conference_base(ms_time(nullptr), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionSRTP, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, FALSE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, TRUE, FALSE,
	                       FALSE);
}

static void create_end_to_end_encrypted_conference_with_late_participant_addition() {
	create_conference_with_late_participant_addition_base(ms_time(nullptr), -1, LinphoneConferenceLayoutGrid,
	                                                      LinphoneConferenceParticipantListTypeClosed, TRUE, TRUE,
	                                                      LinphoneConferenceSecurityLevelEndToEnd);
}

static void create_end_to_end_encrypted_conference_with_late_participant_addition_declined() {
	create_conference_with_late_participant_addition_base(ms_time(nullptr), -1, LinphoneConferenceLayoutActiveSpeaker,
	                                                      LinphoneConferenceParticipantListTypeClosed, FALSE, TRUE,
	                                                      LinphoneConferenceSecurityLevelEndToEnd);
}

static void create_simple_end_to_end_encrypted_conference_dial_out() {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
	create_conference_dial_out_base(LinphoneConferenceLayoutActiveSpeaker, pol, FALSE, FALSE,
	                                LinphoneConferenceParticipantListTypeClosed, TRUE, FALSE,
	                                LinphoneConferenceSecurityLevelEndToEnd, FALSE, FALSE);
	linphone_video_activation_policy_unref(pol);
}

static void create_simple_end_to_end_encrypted_conference_dial_out_with_chat() {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
	create_conference_dial_out_base(LinphoneConferenceLayoutActiveSpeaker, pol, FALSE, FALSE,
	                                LinphoneConferenceParticipantListTypeClosed, TRUE, FALSE,
	                                LinphoneConferenceSecurityLevelEndToEnd, FALSE, TRUE);
	linphone_video_activation_policy_unref(pol);
}

static void create_end_to_end_encrypted_conference_dial_out_terminate_call_on_version_mismatch() {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
	create_conference_dial_out_base(LinphoneConferenceLayoutActiveSpeaker, pol, FALSE, FALSE,
	                                LinphoneConferenceParticipantListTypeClosed, TRUE, FALSE,
	                                LinphoneConferenceSecurityLevelEndToEnd, TRUE, FALSE);
	linphone_video_activation_policy_unref(pol);
}

static void create_end_to_end_encryption_conference_with_audio_only_participants() {
	create_conference_with_audio_only_participants_base(LinphoneConferenceSecurityLevelEndToEnd);
}

static void create_simple_end_to_end_encrypted_conference_dial_out_with_some_calls_declined() {
	create_simple_conference_dial_out_with_some_calls_declined_base(LinphoneReasonDeclined,
	                                                                LinphoneConferenceSecurityLevelEndToEnd);
}

static void create_simple_end_to_end_encrypted_conference_dial_out_with_some_calls_busy() {
	create_simple_conference_dial_out_with_some_calls_declined_base(LinphoneReasonBusy,
	                                                                LinphoneConferenceSecurityLevelEndToEnd);
}

static void create_simple_end_to_end_encrypted_conference_with_screen_sharing() {
	create_conference_with_screen_sharing_base(
	    ms_time(nullptr), -1, LinphoneMediaEncryptionZRTP, LinphoneConferenceLayoutActiveSpeaker, TRUE, TRUE, FALSE,
	    LinphoneMediaDirectionSendOnly, LinphoneConferenceSecurityLevelEndToEnd, {LinphoneParticipantRoleSpeaker});
}

static void connection_method_check(LinphoneConference *conference, LinphoneParticipantDevice *participant_device) {
	LinphoneConferenceCbs *cbs = linphone_conference_get_current_callbacks(conference);
	auto berthe = static_cast<LinphoneCoreManager *>(linphone_conference_cbs_get_user_data(cbs));
	LinphoneParticipantDeviceJoiningMethod expected_joining_method = LinphoneParticipantDeviceJoiningMethodDialedIn;
	if (linphone_address_weak_equal(berthe->identity, linphone_participant_device_get_address(participant_device))) {
		expected_joining_method = LinphoneParticipantDeviceJoiningMethodDialedOut;
	}
	LinphoneParticipantDeviceJoiningMethod joining_method =
	    linphone_participant_device_get_joining_method(participant_device);
	BC_ASSERT_EQUAL((int)joining_method, (int)expected_joining_method, int, "%d");
}

static void decline_connection_method_check(LinphoneConference *conference,
                                            LinphoneParticipantDevice *participant_device) {
	LinphoneConferenceCbs *cbs = linphone_conference_get_current_callbacks(conference);
	auto berthe = (LinphoneCoreManager *)linphone_conference_cbs_get_user_data(cbs);
	LinphoneParticipantDeviceJoiningMethod expected_joining_method = LinphoneParticipantDeviceJoiningMethodDialedIn;
	if (linphone_address_weak_equal(berthe->identity, linphone_participant_device_get_address(participant_device))) {
		expected_joining_method = LinphoneParticipantDeviceJoiningMethodDialedIn;
	}
	LinphoneParticipantDeviceJoiningMethod joining_method =
	    linphone_participant_device_get_joining_method(participant_device);
	BC_ASSERT_EQUAL((int)joining_method, (int)expected_joining_method, int, "%d");
}

static void disconnection_method_check(LinphoneConference *conference,
                                       const LinphoneParticipantDevice *participant_device) {
	LinphoneConferenceCbs *cbs = linphone_conference_get_current_callbacks(conference);
	auto berthe = (LinphoneCoreManager *)linphone_conference_cbs_get_user_data(cbs);
	LinphoneParticipantDeviceDisconnectionMethod expected_disconnection_method =
	    LinphoneParticipantDeviceDisconnectionMethodDeparted;
	if (linphone_address_weak_equal(berthe->identity, linphone_participant_device_get_address(participant_device))) {
		expected_disconnection_method = LinphoneParticipantDeviceDisconnectionMethodBooted;
	}
	LinphoneParticipantDeviceDisconnectionMethod disconnection_method =
	    linphone_participant_device_get_disconnection_method(participant_device);
	BC_ASSERT_EQUAL((int)disconnection_method, (int)expected_disconnection_method, int, "%d");
}

static void decline_disconnection_method_check(LinphoneConference *conference,
                                               const LinphoneParticipantDevice *participant_device) {
	LinphoneConferenceCbs *cbs = linphone_conference_get_current_callbacks(conference);
	auto berthe = (LinphoneCoreManager *)linphone_conference_cbs_get_user_data(cbs);
	LinphoneParticipantDeviceDisconnectionMethod expected_disconnection_method =
	    LinphoneParticipantDeviceDisconnectionMethodDeparted;
	if (linphone_address_weak_equal(berthe->identity, linphone_participant_device_get_address(participant_device))) {
		expected_disconnection_method = LinphoneParticipantDeviceDisconnectionMethodFailed;
	}
	LinphoneParticipantDeviceDisconnectionMethod disconnection_method =
	    linphone_participant_device_get_disconnection_method(participant_device);
	BC_ASSERT_EQUAL((int)disconnection_method, (int)expected_disconnection_method, int, "%d");
}

static void create_simple_end_to_end_encrypted_conference_with_participant_added_by_admin(bool accept) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		bool enable_lime = true;
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), enable_lime);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), enable_lime);
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress(), enable_lime);
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress(), enable_lime);
		ClientConference berthe("berthe_rc", focus.getConferenceFactoryAddress(), enable_lime);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(berthe);

		setup_conference_info_cbs(marie.getCMgr());
		LinphoneMediaEncryption encryption = LinphoneMediaEncryptionZRTP;

		bctbx_list_t *coresList = nullptr;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(),
		                 berthe.getCMgr()}) {
			LinphoneVideoActivationPolicy *pol =
			    linphone_factory_create_video_activation_policy(linphone_factory_get());
			linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
			linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
			linphone_core_set_video_activation_policy(mgr->lc, pol);
			linphone_video_activation_policy_unref(pol);

			linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);
			linphone_core_enable_video_capture(mgr->lc, TRUE);
			linphone_core_enable_video_display(mgr->lc, TRUE);

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, LinphoneConferenceLayoutActiveSpeaker);
				linphone_core_set_media_encryption(mgr->lc, encryption);
			}

			// Enable ICE at the account level but not at the core level
			enable_stun_in_mgr(mgr, TRUE, TRUE, FALSE, FALSE);

			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip", "update_call_when_ice_completed", TRUE);
			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip",
			                        "update_call_when_ice_completed_with_dtls", FALSE);

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		configure_end_to_end_encrypted_conference_server(focus);

		int nortp_timeout = 10;
		linphone_core_set_nortp_timeout(marie.getLc(), nortp_timeout);
		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), LinphoneConferenceParticipantListTypeClosed);

		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(berthe.getLc()));

		stats focus_stat = focus.getStats();

		std::list<LinphoneCoreManager *> participants{laure.getCMgr(), pauline.getCMgr(), michelle.getCMgr()};
		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(),
		                                                laure.getCMgr(), michelle.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(),
		                                         michelle.getCMgr()};

		time_t start_time = ms_time(nullptr);
		time_t end_time = -1;
		const char *initialSubject = "E2E conference";
		const char *description = "Using REFER method";

		bctbx_list_t *participants_info = nullptr;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		participantList.insert(
		    std::make_pair(laure.getCMgr(), add_participant_info_to_list(&participants_info, laure.getCMgr()->identity,
		                                                                 LinphoneParticipantRoleListener, -1)));
		participantList.insert(std::make_pair(
		    pauline.getCMgr(), add_participant_info_to_list(&participants_info, pauline.getCMgr()->identity,
		                                                    LinphoneParticipantRoleSpeaker, -1)));
		participantList.insert(std::make_pair(
		    michelle.getCMgr(), add_participant_info_to_list(&participants_info, michelle.getCMgr()->identity,
		                                                     LinphoneParticipantRoleSpeaker, -1)));

		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, LinphoneConferenceSecurityLevelEndToEnd, TRUE, FALSE, NULL);
		BC_ASSERT_PTR_NOT_NULL(confAddr);
		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:");

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 3,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : members) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_call_params_set_media_encryption(new_params, encryption);
			linphone_call_params_set_video_direction(new_params, LinphoneMediaDirectionSendRecv);
			if (mgr == pauline.getCMgr()) {
				linphone_call_params_enable_mic(new_params, FALSE);
			}
			ms_message("%s is entering conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, nullptr, nullptr);
			linphone_call_params_unref(new_params);
			LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(call);
			if (call) {
				LinphoneCallLog *call_log = linphone_call_get_call_log(call);
				BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
			}
		}

		int idx = 1;
		int nb_subscriptions = 2;
		for (auto mgr : members) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1,
			                             liblinphone_tester_sip_timeout));
			int no_streams_running = 2;
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating, (no_streams_running - 1),
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, no_streams_running,
			                             liblinphone_tester_sip_timeout));
			// Update to add to conference.
			// If ICE is enabled, the addition to a conference may go through a resume of the call
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneChatRoomStateCreated,
			                             ((mgr == marie.getCMgr()) ? 2 : 1), liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress,
			                             nb_subscriptions, 5000));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, nb_subscriptions, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_NotifyEktReceived, 1, liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEncryptedOn, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEncryptedOn, idx,
			                             liblinphone_tester_sip_timeout));

			LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(call);
			if (call) {
				const LinphoneCallParams *call_current_params = linphone_call_get_current_params(call);
				const LinphoneMediaEncryption call_encryption =
				    linphone_call_params_get_media_encryption(call_current_params);
				BC_ASSERT_EQUAL(call_encryption, encryption, int, "%d");
			}
			LinphoneCall *conference_call = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(conference_call);
			if (conference_call) {
				const LinphoneCallParams *call_conference_params = linphone_call_get_current_params(conference_call);
				const LinphoneMediaEncryption conference_call_enc =
				    linphone_call_params_get_media_encryption(call_conference_params);
				BC_ASSERT_EQUAL(conference_call_enc, encryption, int, "%d");
			}

			idx++;
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + 4,
		                             liblinphone_tester_sip_timeout));
		int focus_no_streams_running = 8;
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running - 4),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running,
		                             liblinphone_tester_sip_timeout));
		// If ICE is enabled, the addition to a conference may go through a resume of the call
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
		                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + (4 * nb_subscriptions),
		                             5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + (4 * nb_subscriptions), 5000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + 4, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + 4,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
		                             focus_stat.number_of_conference_participant_devices_present + 4,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
		                             focus_stat.number_of_participant_devices_present + 4,
		                             liblinphone_tester_sip_timeout));

		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
		    fill_member_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs, focus.getCMgr(),
		                            memberList, confAddr, TRUE);

		does_all_participants_have_matching_ekt(focus.getCMgr(), memberList, confAddr);

		int nb_admins = 0;
		LinphoneConference *focus_conference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		LinphoneConferenceCbs *cbs = linphone_factory_create_conference_cbs(linphone_factory_get());
		if (BC_ASSERT_PTR_NOT_NULL(focus_conference)) {
			linphone_conference_cbs_set_participant_device_added(cbs, connection_method_check);
			if (accept) {
				linphone_conference_cbs_set_participant_device_removed(cbs, disconnection_method_check);
			} else {
				linphone_conference_cbs_set_participant_device_removed(cbs, decline_disconnection_method_check);
			}
			linphone_conference_cbs_set_user_data(cbs, berthe.getCMgr());
			linphone_conference_add_callbacks(focus_conference, cbs);

			bctbx_list_t *participants_list = linphone_conference_get_participant_list(focus_conference);
			for (bctbx_list_t *itp = participants_list; itp; itp = bctbx_list_next(itp)) {
				auto p = (LinphoneParticipant *)bctbx_list_get_data(itp);
				if (linphone_participant_is_admin(p)) {
					nb_admins++;
				}
			}
			bctbx_list_free_with_data(participants_list, (void (*)(void *))linphone_participant_unref);
			BC_ASSERT_EQUAL(nb_admins, 1, int, "%d");
		}

		for (auto mgr : conferenceMgrs) {
			BC_ASSERT_EQUAL(mgr->stat.number_of_allowed_participant_list_changed, 0, int, "%0d");
		}

		LinphoneConference *marie_conference = linphone_core_search_conference_2(marie.getLc(), confAddr);
		if (BC_ASSERT_PTR_NOT_NULL(marie_conference)) {
			conferenceMgrs.push_back(berthe.getCMgr());
			members.push_back(berthe.getCMgr());
			LinphoneParticipantInfo *berthe_participant_info = add_participant_info_to_list(
			    &participants_info, berthe.getCMgr()->identity, LinphoneParticipantRoleSpeaker, -1);
			participantList.insert(std::make_pair(berthe.getCMgr(), berthe_participant_info));
			memberList.insert(std::make_pair(berthe.getCMgr(), berthe_participant_info));

			focus_stat = focus.getStats();
			stats marie_stat = marie.getStats();
			stats laure_stat = laure.getStats();
			stats pauline_stat = pauline.getStats();
			stats michelle_stat = michelle.getStats();

			ms_message("%s adds %s to conference %s", linphone_core_get_identity(marie.getLc()),
			           linphone_core_get_identity(berthe.getLc()), conference_address_str);
			linphone_conference_add_participant_2(marie_conference, berthe.getCMgr()->identity);

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingProgress,
			                             focus_stat.number_of_LinphoneCallOutgoingProgress + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallIncomingReceived, 1,
			                             liblinphone_tester_sip_timeout));
			LinphoneCall *berthe_pcall = linphone_core_get_call_by_remote_address2(berthe.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(berthe_pcall);
			if (berthe_pcall) {
				if (accept) {
					linphone_call_accept(berthe_pcall);
				} else {
					linphone_call_decline(berthe_pcall, LinphoneReasonDeclined);

					BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallEnd, 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallReleased, 1,
					                             liblinphone_tester_sip_timeout));

					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
					                             focus_stat.number_of_LinphoneCallEnd + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
					                             focus_stat.number_of_LinphoneCallReleased + 1,
					                             liblinphone_tester_sip_timeout));

					linphone_conference_remove_callbacks(focus_conference, cbs);
					LinphoneConferenceCbs *newCbs = linphone_factory_create_conference_cbs(linphone_factory_get());
					linphone_conference_cbs_set_participant_device_added(newCbs, decline_connection_method_check);
					linphone_conference_cbs_set_participant_device_removed(newCbs, disconnection_method_check);
					linphone_conference_cbs_set_user_data(newCbs, berthe.getCMgr());
					linphone_conference_add_callbacks(focus_conference, newCbs);
					linphone_conference_cbs_unref(newCbs);

					linphone_core_invite_address(berthe.getLc(), confAddr);
				}
			}
			linphone_conference_cbs_unref(cbs);

			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallUpdating, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallStreamsRunning, 2,
			                             liblinphone_tester_sip_timeout));
			// Update to add to conference.
			// If ICE is enabled, the addition to a conference may go through a resume of the call
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneConferenceStateCreated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneSubscriptionOutgoingProgress,
			                             nb_subscriptions, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneSubscriptionActive,
			                             nb_subscriptions, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_NotifyEktReceived, 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallEncryptedOn, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEncryptedOn, idx,
			                             liblinphone_tester_sip_timeout));

			berthe_pcall = linphone_core_get_call_by_remote_address2(berthe.getLc(), confAddr);
			if (berthe_pcall) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(berthe_pcall);
				const LinphoneMediaEncryption pcall_enc = linphone_call_params_get_media_encryption(call_cparams);
				BC_ASSERT_EQUAL(pcall_enc, encryption, int, "%d");
			}
			LinphoneCall *berthe_ccall =
			    linphone_core_get_call_by_remote_address2(focus.getLc(), berthe.getCMgr()->identity);
			BC_ASSERT_PTR_NOT_NULL(berthe_ccall);
			if (berthe_ccall) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(berthe_ccall);
				const LinphoneMediaEncryption ccall_enc = linphone_call_params_get_media_encryption(call_cparams);
				BC_ASSERT_EQUAL(ccall_enc, encryption, int, "%d");
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
			                             focus_stat.number_of_LinphoneCallUpdatedByRemote + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
			                             focus_stat.number_of_LinphoneCallStreamsRunning + 2,
			                             liblinphone_tester_sip_timeout));
			// If ICE is enabled, the addition to a conference may go through a resume of the call
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
			                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + nb_subscriptions,
			                             5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
			                             focus_stat.number_of_LinphoneSubscriptionActive + nb_subscriptions, 5000));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_allowed_participant_list_changed,
			                             focus_stat.number_of_allowed_participant_list_changed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
			                             focus_stat.number_of_participants_added + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
			                             focus_stat.number_of_participant_devices_added + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
			                             focus_stat.number_of_conference_participant_devices_present + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
			                             focus_stat.number_of_participant_devices_present + 1,
			                             liblinphone_tester_sip_timeout));

			int nbParticipantsAdded = 1;
			int nbNotifyEktReceived = 2;
			BC_ASSERT_TRUE(verify_participant_addition_stats(coresList, marie, marie_stat, nbParticipantsAdded,
			                                                 nbNotifyEktReceived));
			BC_ASSERT_TRUE(verify_participant_addition_stats(coresList, pauline, pauline_stat, nbParticipantsAdded,
			                                                 nbNotifyEktReceived));
			BC_ASSERT_TRUE(verify_participant_addition_stats(coresList, michelle, michelle_stat, nbParticipantsAdded,
			                                                 nbNotifyEktReceived));
			BC_ASSERT_TRUE(verify_participant_addition_stats(coresList, laure, laure_stat, nbParticipantsAdded,
			                                                 nbNotifyEktReceived));

			memberList = fill_member_list(members, participantList, marie.getCMgr(), participants_info);
			wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs,
			                            focus.getCMgr(), memberList, confAddr, TRUE);

			does_all_participants_have_matching_ekt(focus.getCMgr(), memberList, confAddr);

			int new_nb_admins = 0;
			if (focus_conference) {
				bctbx_list_t *participants_list = linphone_conference_get_participant_list(focus_conference);
				for (bctbx_list_t *itp = participants_list; itp; itp = bctbx_list_next(itp)) {
					auto p = (LinphoneParticipant *)bctbx_list_get_data(itp);
					if (linphone_participant_is_admin(p)) {
						new_nb_admins++;
					}
				}
				bctbx_list_free_with_data(participants_list, (void (*)(void *))linphone_participant_unref);
				BC_ASSERT_EQUAL(nb_admins, new_nb_admins, int, "%d");
			}

			focus_stat = focus.getStats();
			marie_stat = marie.getStats();
			laure_stat = laure.getStats();
			pauline_stat = pauline.getStats();
			michelle_stat = michelle.getStats();
			stats berthe_stat = berthe.getStats();

			ms_message("%s removes %s to conference %s", linphone_core_get_identity(marie.getLc()),
			           linphone_core_get_identity(berthe.getLc()), conference_address_str);
			LinphoneParticipant *participant = linphone_conference_find_participant(
			    marie_conference, const_cast<LinphoneAddress *>(berthe.getCMgr()->identity));
			linphone_conference_remove_participant_2(marie_conference, participant);

			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallEnd,
			                             berthe_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallReleased,
			                             berthe_stat.number_of_LinphoneCallReleased + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneSubscriptionTerminated,
			                             berthe_stat.number_of_LinphoneSubscriptionTerminated + nb_subscriptions,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &berthe.getStats().number_of_LinphoneConferenceStateTerminationPending,
			    berthe_stat.number_of_LinphoneConferenceStateTerminationPending + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneConferenceStateTerminated,
			                             berthe_stat.number_of_LinphoneConferenceStateTerminated + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneConferenceStateDeleted,
			                             berthe_stat.number_of_LinphoneConferenceStateDeleted + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
			                             focus_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
			                             focus_stat.number_of_LinphoneCallReleased + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
			                             focus_stat.number_of_LinphoneSubscriptionTerminated + nb_subscriptions, 5000));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_allowed_participant_list_changed,
			                             focus_stat.number_of_allowed_participant_list_changed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
			                             focus_stat.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
			                             focus_stat.number_of_participant_devices_removed + 1,
			                             liblinphone_tester_sip_timeout));

			nbNotifyEktReceived = 1;
			BC_ASSERT_TRUE(verify_participant_removal_stats(coresList, marie, marie_stat, nbParticipantsAdded,
			                                                nbNotifyEktReceived));
			BC_ASSERT_TRUE(verify_participant_removal_stats(coresList, pauline, pauline_stat, nbParticipantsAdded,
			                                                nbNotifyEktReceived));
			BC_ASSERT_TRUE(verify_participant_removal_stats(coresList, michelle, michelle_stat, nbParticipantsAdded,
			                                                nbNotifyEktReceived));
			BC_ASSERT_TRUE(verify_participant_removal_stats(coresList, laure, laure_stat, nbParticipantsAdded,
			                                                nbNotifyEktReceived));

			conferenceMgrs.remove(berthe.getCMgr());
			members.remove(berthe.getCMgr());

			memberList = fill_member_list(members, participantList, marie.getCMgr(), participants_info);
			wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs,
			                            focus.getCMgr(), memberList, confAddr, TRUE);

			does_all_participants_have_matching_ekt(focus.getCMgr(), memberList, confAddr);

			if (focus_conference) {
				new_nb_admins = 0;
				bctbx_list_t *participants_list = linphone_conference_get_participant_list(focus_conference);
				for (bctbx_list_t *itp = participants_list; itp; itp = bctbx_list_next(itp)) {
					auto p = (LinphoneParticipant *)bctbx_list_get_data(itp);
					if (linphone_participant_is_admin(p)) {
						new_nb_admins++;
					}
				}
				bctbx_list_free_with_data(participants_list, (void (*)(void *))linphone_participant_unref);
				BC_ASSERT_EQUAL(nb_admins, new_nb_admins, int, "%d");
			}

			focus_stat = focus.getStats();
			marie_stat = marie.getStats();
			laure_stat = laure.getStats();
			pauline_stat = pauline.getStats();
			michelle_stat = michelle.getStats();

			ms_message("%s exits conference %s", linphone_core_get_identity(marie.getLc()), conference_address_str);
			linphone_conference_terminate(marie_conference);

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallEnd,
			                             marie_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallReleased,
			                             marie_stat.number_of_LinphoneCallReleased + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionTerminated,
			                             marie_stat.number_of_LinphoneSubscriptionTerminated + nb_subscriptions,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminationPending,
			    marie_stat.number_of_LinphoneConferenceStateTerminationPending + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminated,
			                             marie_stat.number_of_LinphoneConferenceStateTerminated + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateDeleted,
			                             marie_stat.number_of_LinphoneConferenceStateDeleted + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
			                             focus_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
			                             focus_stat.number_of_LinphoneCallReleased + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
			                             focus_stat.number_of_LinphoneSubscriptionTerminated + nb_subscriptions, 5000));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
			                             focus_stat.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
			                             focus_stat.number_of_participant_devices_removed + 1,
			                             liblinphone_tester_sip_timeout));

			nbNotifyEktReceived = 0;
			BC_ASSERT_TRUE(verify_participant_removal_stats(coresList, pauline, pauline_stat, nbParticipantsAdded,
			                                                nbNotifyEktReceived));
			BC_ASSERT_TRUE(verify_participant_removal_stats(coresList, michelle, michelle_stat, nbParticipantsAdded,
			                                                nbNotifyEktReceived));
			BC_ASSERT_TRUE(verify_participant_removal_stats(coresList, laure, laure_stat, nbParticipantsAdded,
			                                                nbNotifyEktReceived));

			if (focus_conference) {
				new_nb_admins = 0;
				bctbx_list_t *participants_list = linphone_conference_get_participant_list(focus_conference);
				for (bctbx_list_t *itp = participants_list; itp; itp = bctbx_list_next(itp)) {
					auto p = (LinphoneParticipant *)bctbx_list_get_data(itp);
					if (linphone_participant_is_admin(p)) {
						new_nb_admins++;
					}
				}
				bctbx_list_free_with_data(participants_list, (void (*)(void *))linphone_participant_unref);
				BC_ASSERT_EQUAL(nb_admins - 1, new_nb_admins, int, "%d");
			}
		}

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
			return false;
		});

		if (focus_conference) {
			bctbx_list_t *participants_list = linphone_conference_get_participant_list(focus_conference);
			BC_ASSERT_EQUAL(bctbx_list_size(participants_list), 3, size_t, "%zu");
			bctbx_list_free_with_data(participants_list, (void (*)(void *))linphone_participant_unref);
		}

		linphone_core_terminate_all_calls(michelle.getLc());
		linphone_core_terminate_all_calls(pauline.getLc());
		linphone_core_terminate_all_calls(laure.getLc());
		linphone_core_terminate_all_calls(marie.getLc());
		linphone_core_terminate_all_calls(berthe.getLc());

		// Wait for calls to be terminated
		for (auto mgr : conferenceMgrs) {
			int no_calls = (mgr == focus.getCMgr()) ? static_cast<int>(members.size()) : 1;
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, no_calls, 30000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, no_calls, 30000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated,
			                             no_calls * nb_subscriptions, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_PTR_NULL(linphone_core_search_conference_2(mgr->lc, confAddr));
		}

		ms_free(conference_address_str);
		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void create_simple_end_to_end_encrypted_conference_with_participant_added_by_admin_call_accepted() {
	create_simple_end_to_end_encrypted_conference_with_participant_added_by_admin(true);
}

static void create_simple_end_to_end_encrypted_conference_with_participant_added_by_admin_call_declined() {
	create_simple_end_to_end_encrypted_conference_with_participant_added_by_admin(false);
}

static void create_encrypted_conference_with_chat() {
	create_conference_with_chat_base(LinphoneConferenceSecurityLevelEndToEnd, FALSE, FALSE, TRUE, 1, FALSE, TRUE, FALSE,
	                                 ms_time(NULL), FALSE);
}

static void create_encrypted_conference_with_chat_and_cores_restart(void) {
	create_conference_with_chat_base(LinphoneConferenceSecurityLevelEndToEnd, TRUE, TRUE, TRUE, 1, FALSE, TRUE, FALSE,
	                                 (ms_time(NULL) - 45), FALSE);
}

static void create_encrypted_conference_with_chat_network_drops_and_participant_rejoining(void) {
	create_conference_with_chat_base(LinphoneConferenceSecurityLevelEndToEnd, FALSE, FALSE, FALSE, -1, TRUE, TRUE, TRUE,
	                                 (ms_time(NULL) - 45), TRUE);
}

static void scheduling_failure_check(LinphoneConferenceScheduler *scheduler, LinphoneConferenceSchedulerState state) {
	switch (state) {
		case LinphoneConferenceSchedulerStateIdle:
		case LinphoneConferenceSchedulerStateAllocationPending:
		case LinphoneConferenceSchedulerStateReady:
		case LinphoneConferenceSchedulerStateUpdating:
			break;
		case LinphoneConferenceSchedulerStateError:
			check_session_error(scheduler, LinphoneReasonNotAcceptable);
			break;
	}
}

static void failure_in_creating_end_to_end_encrypted_conference_bad_server_config() {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), TRUE);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), TRUE);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t *coresList = nullptr;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr()}) {
			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), LinphoneConferenceParticipantListTypeClosed);

		std::list<LinphoneCoreManager *> participants{pauline.getCMgr()};
		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(), pauline.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr()};

		time_t start_time = ms_time(nullptr);
		int duration = 0;
		const char *initialSubject = "E2E encrypted conference";
		const char *description = "Failure in creating end-to-end encrypted conference";

		bctbx_list_t *participants_info = nullptr;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		participantList.insert(std::make_pair(
		    pauline.getCMgr(), add_participant_info_to_list(&participants_info, pauline.getCMgr()->identity,
		                                                    LinphoneParticipantRoleSpeaker, -1)));

		std::vector<stats> participant_stats;
		for (const auto &[mgr, participant_info] : participantList) {
			LinphoneParticipantInfo *participant_info_clone = linphone_participant_info_clone(participant_info);
			participants_info = bctbx_list_append(participants_info, participant_info_clone);
			if (mgr == pauline.getCMgr()) {
				coresList = bctbx_list_append(coresList, mgr->lc);
				participant_stats.push_back(mgr->stat);
				participants.push_back(mgr);
			}
		}

		stats marie_stat = marie.getStats();
		stats focus_stat = focus.getStats();

		// The organizer creates a conference scheduler
		LinphoneConferenceScheduler *conference_scheduler =
		    linphone_core_create_sip_conference_scheduler(marie.getLc(), nullptr);
		LinphoneConferenceSchedulerCbs *cbs = linphone_factory_create_conference_scheduler_cbs(linphone_factory_get());
		linphone_conference_scheduler_cbs_set_state_changed(cbs, conference_scheduler_state_changed);
		linphone_conference_scheduler_cbs_set_invitations_sent(cbs, conference_scheduler_invitations_sent);
		linphone_conference_scheduler_add_callbacks(conference_scheduler, cbs);
		linphone_conference_scheduler_cbs_unref(cbs);
		cbs = nullptr;
		cbs = linphone_factory_create_conference_scheduler_cbs(linphone_factory_get());
		linphone_conference_scheduler_cbs_set_state_changed(cbs, scheduling_failure_check);
		linphone_conference_scheduler_add_callbacks(conference_scheduler, cbs);
		linphone_conference_scheduler_cbs_unref(cbs);

		LinphoneConferenceInfo *conf_info = linphone_conference_info_new();

		LinphoneAccount *default_account = linphone_core_get_default_account(marie.getLc());
		LinphoneAddress *organizer_address = default_account
		                                         ? linphone_address_clone(linphone_account_params_get_identity_address(
		                                               linphone_account_get_params(default_account)))
		                                         : linphone_address_clone(marie.getCMgr()->identity);
		linphone_conference_info_set_organizer(conf_info, organizer_address);
		linphone_conference_info_set_participant_infos(conf_info, participants_info);
		linphone_conference_info_set_duration(conf_info, duration);
		linphone_conference_info_set_date_time(conf_info, start_time);
		linphone_conference_info_set_subject(conf_info, initialSubject);
		linphone_conference_info_set_description(conf_info, description);
		linphone_conference_info_set_security_level(conf_info, LinphoneConferenceSecurityLevelEndToEnd);
		linphone_conference_info_set_capability(conf_info, LinphoneStreamTypeVideo, TRUE);
		linphone_conference_info_set_capability(conf_info, LinphoneStreamTypeText, FALSE);

		linphone_conference_scheduler_set_info(conference_scheduler, conf_info);
		linphone_conference_info_unref(conf_info);
		linphone_address_unref(organizer_address);

		BC_ASSERT_PTR_NOT_NULL(conference_scheduler);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_ConferenceSchedulerStateAllocationPending,
		                             marie_stat.number_of_ConferenceSchedulerStateAllocationPending + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_ConferenceSchedulerStateError,
		                             marie_stat.number_of_ConferenceSchedulerStateError + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreationFailed,
		                             focus_stat.number_of_LinphoneConferenceStateCreationFailed + 1,
		                             liblinphone_tester_sip_timeout));

		linphone_conference_scheduler_unref(conference_scheduler);
		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		bctbx_list_free(coresList);
	}
}

static void create_simple_end_to_end_encrypted_conference_terminated_early() {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		bool enable_lime = true;
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), enable_lime);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), enable_lime);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);

		setup_conference_info_cbs(marie.getCMgr());
		LinphoneMediaEncryption encryption = LinphoneMediaEncryptionZRTP;

		bctbx_list_t *coresList = nullptr;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr()}) {
			LinphoneVideoActivationPolicy *pol =
			    linphone_factory_create_video_activation_policy(linphone_factory_get());
			linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
			linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
			linphone_core_set_video_activation_policy(mgr->lc, pol);
			linphone_video_activation_policy_unref(pol);

			linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);
			linphone_core_enable_video_capture(mgr->lc, TRUE);
			linphone_core_enable_video_display(mgr->lc, TRUE);

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, LinphoneConferenceLayoutActiveSpeaker);
				linphone_core_set_media_encryption(mgr->lc, encryption);
			}

			// Enable ICE at the account level but not at the core level
			enable_stun_in_mgr(mgr, TRUE, TRUE, FALSE, FALSE);

			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip", "update_call_when_ice_completed", TRUE);
			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip",
			                        "update_call_when_ice_completed_with_dtls", FALSE);

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		configure_end_to_end_encrypted_conference_server(focus);

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), LinphoneConferenceParticipantListTypeClosed);

		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline.getLc()));

		stats focus_stat = focus.getStats();

		std::list<LinphoneCoreManager *> participants{pauline.getCMgr()};
		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(), pauline.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr()};

		time_t start_time = ms_time(nullptr);
		time_t end_time = -1;
		const char *initialSubject = "E2E conference - short duration";
		const char *description = "Quick end";

		bctbx_list_t *participants_info = nullptr;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		participantList.insert(std::make_pair(
		    pauline.getCMgr(), add_participant_info_to_list(&participants_info, pauline.getCMgr()->identity,
		                                                    LinphoneParticipantRoleSpeaker, -1)));

		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, LinphoneConferenceSecurityLevelEndToEnd, TRUE, TRUE, NULL);
		BC_ASSERT_PTR_NOT_NULL(confAddr);
		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:");

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 1,
		                             liblinphone_tester_sip_timeout));

		LinphoneCallParams *new_params = linphone_core_create_call_params(pauline.getLc(), nullptr);
		linphone_call_params_set_media_encryption(new_params, encryption);
		linphone_call_params_set_video_direction(new_params, LinphoneMediaDirectionSendRecv);
		ms_message("%s is entering conference %s", linphone_core_get_identity(pauline.getLc()), conference_address_str);
		linphone_core_invite_address_with_params_2(pauline.getLc(), confAddr, new_params, nullptr, nullptr);
		linphone_call_params_unref(new_params);
		LinphoneCall *pauline_call = linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(pauline_call);
		if (pauline_call) {
			LinphoneCallLog *call_log = linphone_call_get_call_log(pauline_call);
			BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallOutgoingProgress, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &pauline.getStats().number_of_LinphoneSubscriptionOutgoingProgress, 2, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneSubscriptionActive, 2, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_NotifyFullStateReceived, 1,
		                             liblinphone_tester_sip_timeout));

		ms_message("%s terminates the call to conference %s", linphone_core_get_identity(pauline.getLc()),
		           conference_address_str);
		LinphoneConference *pauline_conference = linphone_core_search_conference_2(pauline.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(pauline_conference);
		linphone_conference_terminate(pauline_conference);
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallReleased, 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
		                             focus_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneSubscriptionTerminated, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateTerminated, 1,
		                             liblinphone_tester_sip_timeout));
		// The conference doesn't go through the Deleted state as it has chat capabilities
		BC_ASSERT_PTR_NOT_NULL(linphone_core_search_conference_2(pauline.getLc(), confAddr));

		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		ms_free(conference_address_str);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void create_simple_end_to_end_encrypted_conference_merging_calls() {
	create_simple_conference_merging_calls_base(FALSE, LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE, FALSE,
	                                            LinphoneConferenceSecurityLevelEndToEnd, FALSE);
}

static void encrypted_conference_joined_multiple_times(void) {
	conference_joined_multiple_times_base(LinphoneConferenceSecurityLevelEndToEnd, FALSE, -1);
}

} // namespace LinphoneTest

static test_t local_conference_end_to_end_encryption_scheduled_conference_tests[] = {
    TEST_ONE_TAG("First notify", LinphoneTest::first_notify_ekt_xml_composing_parsing_test, "End2EndConf"),
    TEST_ONE_TAG("SPI info", LinphoneTest::spi_info_ekt_xml_composing_parsing_test, "End2EndConf"),
    TEST_ONE_TAG("Cipher transport", LinphoneTest::cipher_transport_ekt_xml_composing_parsing_test, "End2EndConf"),
    TEST_ONE_TAG("End-to-End Conference joined multiple times",
                 LinphoneTest::encrypted_conference_joined_multiple_times,
                 "End2EndConf"),
    TEST_ONE_TAG("Create simple end-to-end encrypted conference",
                 LinphoneTest::create_simple_end_to_end_encrypted_conference,
                 "End2EndConf"),
    TEST_TWO_TAGS("Create simple end-to-end encrypted conference with server restart",
                  LinphoneTest::create_simple_end_to_end_encrypted_conference_with_server_restart,
                  "LeaksMemory",
                  "End2EndConf"),
    TEST_TWO_TAGS("Create simple end-to-end encrypted conference with client restart",
                  LinphoneTest::create_simple_end_to_end_encrypted_conference_with_client_restart,
                  "LeaksMemory",
                  "End2EndConf"),
    TEST_TWO_TAGS("Create simple end-to-end encrypted ICE conference",
                  LinphoneTest::create_simple_end_to_end_encrypted_ice_conference,
                  "ICE",
                  "End2EndConf"),
    TEST_ONE_TAG("Create simple end-to-end encrypted conference with screen sharing override",
                 LinphoneTest::create_simple_end_to_end_encrypted_conference_with_screen_sharing,
                 "End2EndConf"),
    TEST_ONE_TAG("Create end-to-end encrypted conference with uninvited participant",
                 LinphoneTest::create_end_to_end_encrypted_conference_with_uninvited_participant,
                 "End2EndConf"),
    TEST_ONE_TAG("Create end-to-end encrypted conference with uninvited participant not allowed",
                 LinphoneTest::create_end_to_end_encrypted_conference_with_uninvited_participant_not_allowed,
                 "End2EndConf"),
    TEST_ONE_TAG("Create end-to-end encrypted conference with late participant addition declined",
                 LinphoneTest::create_end_to_end_encrypted_conference_with_late_participant_addition_declined,
                 "End2EndConf"),
    TEST_ONE_TAG("Create end-to-end encrypted conference starting immediately",
                 LinphoneTest::create_end_to_end_encrypted_conference_starting_immediately,
                 "End2EndConf"),
    TEST_ONE_TAG("Create end-to-end encrypted conference starting in the past",
                 LinphoneTest::create_end_to_end_encrypted_conference_starting_in_the_past,
                 "End2EndConf"),
    TEST_ONE_TAG("Create end-to-end encrypted conference with late participant addition",
                 LinphoneTest::create_end_to_end_encrypted_conference_with_late_participant_addition,
                 "End2EndConf"),
    TEST_ONE_TAG("Create end-to-end encrypted conference terminate call on version mismatch",
                 LinphoneTest::create_end_to_end_encrypted_conference_terminate_call_on_version_mismatch,
                 "End2EndConf"),
    TEST_ONE_TAG(
        "Create simple end-to-end encrypted conference with participant added by admin call accepted",
        LinphoneTest::create_simple_end_to_end_encrypted_conference_with_participant_added_by_admin_call_accepted,
        "End2EndConf"),
    TEST_ONE_TAG(
        "Create simple end-to-end encrypted conference with participant added by admin call declined",
        LinphoneTest::create_simple_end_to_end_encrypted_conference_with_participant_added_by_admin_call_declined,
        "End2EndConf"),
    TEST_ONE_TAG("Failure in creating end-to-end encrypted conference bad server config",
                 LinphoneTest::failure_in_creating_end_to_end_encrypted_conference_bad_server_config,
                 "End2EndConf"),
    TEST_ONE_TAG("Create simple encrypted conference terminated early",
                 LinphoneTest::create_simple_end_to_end_encrypted_conference_terminated_early,
                 "End2EndConf"),
};

static test_t local_conference_end_to_end_encryption_scheduled_conference_with_chat_tests[] = {
    TEST_ONE_TAG(
        "Create encrypted conference with chat", LinphoneTest::create_encrypted_conference_with_chat, "End2EndConf"),
    TEST_TWO_TAGS("Create encrypted conference with chat and cores restart",
                  LinphoneTest::create_encrypted_conference_with_chat_and_cores_restart,
                  "LeaksMemory",
                  "End2EndConf"),
    TEST_ONE_TAG("Create encrypted conference with chat network drops and participant rejoining",
                 LinphoneTest::create_encrypted_conference_with_chat_network_drops_and_participant_rejoining,
                 "End2EndConf"),
    TEST_ONE_TAG("Create simple end-to-end encrypted dial out conference with chat",
                 LinphoneTest::create_simple_end_to_end_encrypted_conference_dial_out_with_chat,
                 "End2EndConf"),
};

static test_t local_conference_end_to_end_encryption_scheduled_conference_audio_only_participant_tests[] = {
    TEST_ONE_TAG("Create simple end-to-end encrypted conference with audio only participant",
                 LinphoneTest::create_simple_end_to_end_encrypted_conference_with_audio_only_participant,
                 "End2EndConf"),
    TEST_ONE_TAG("Create end-to-end encrypted conference with audio only and uninvited participant",
                 LinphoneTest::create_end_to_end_encrypted_conference_with_audio_only_and_uninvited_participant,
                 "End2EndConf"),
    TEST_ONE_TAG("Create simple end-to-end encrypted conference with audio only participant enabling video",
                 LinphoneTest::create_simple_end_to_end_encrypted_conference_with_audio_only_participant_enabling_video,
                 "End2EndConf"),
    TEST_ONE_TAG("Create end-to-end encrypted conference with audio only participants",
                 LinphoneTest::create_end_to_end_encryption_conference_with_audio_only_participants,
                 "End2EndConf"),
};

static test_t local_conference_end_to_end_encryption_impromptu_conference_tests[] = {
    TEST_ONE_TAG("Create simple end-to-end encrypted dial out conference",
                 LinphoneTest::create_simple_end_to_end_encrypted_conference_dial_out,
                 "End2EndConf"),
    TEST_ONE_TAG("Create simple end-to-end encrypted conference dial out with some calls declined",
                 LinphoneTest::create_simple_end_to_end_encrypted_conference_dial_out_with_some_calls_declined,
                 "End2EndConf"),
    TEST_ONE_TAG("Create simple end-to-end encrypted conference dial out with some calls busy",
                 LinphoneTest::create_simple_end_to_end_encrypted_conference_dial_out_with_some_calls_busy,
                 "End2EndConf"),
    TEST_ONE_TAG("Create end-to-end encrypted dial out conference terminate call on version mismatch",
                 LinphoneTest::create_end_to_end_encrypted_conference_dial_out_terminate_call_on_version_mismatch,
                 "End2EndConf"),
    TEST_ONE_TAG("Create simple end-to-end encrypted conference by merging calls",
                 LinphoneTest::create_simple_end_to_end_encrypted_conference_merging_calls,
                 "End2EndConf"),
};

test_suite_t local_conference_test_suite_end_to_end_encryption_scheduled_conference = {
    "Local conference tester (Scheduled Conference End to end encryption)",
    nullptr,
    nullptr,
    liblinphone_tester_before_each,
    liblinphone_tester_after_each,
    sizeof(local_conference_end_to_end_encryption_scheduled_conference_tests) /
        sizeof(local_conference_end_to_end_encryption_scheduled_conference_tests[0]),
    local_conference_end_to_end_encryption_scheduled_conference_tests,
    0,
    4};

test_suite_t local_conference_test_suite_end_to_end_encryption_scheduled_conference_audio_only_participant = {
    "Local conference tester (Audio only participants End to end encryption)",
    nullptr,
    nullptr,
    liblinphone_tester_before_each,
    liblinphone_tester_after_each,
    sizeof(local_conference_end_to_end_encryption_scheduled_conference_audio_only_participant_tests) /
        sizeof(local_conference_end_to_end_encryption_scheduled_conference_audio_only_participant_tests[0]),
    local_conference_end_to_end_encryption_scheduled_conference_audio_only_participant_tests,
    0,
    4};

test_suite_t local_conference_test_suite_end_to_end_encryption_scheduled_conference_with_chat = {
    "Local conference tester (End to end encryption Conference with chat)",
    NULL,
    NULL,
    liblinphone_tester_before_each,
    liblinphone_tester_after_each,
    sizeof(local_conference_end_to_end_encryption_scheduled_conference_with_chat_tests) /
        sizeof(local_conference_end_to_end_encryption_scheduled_conference_with_chat_tests[0]),
    local_conference_end_to_end_encryption_scheduled_conference_with_chat_tests,
    0,
    4 /*cpu_weight : video conference uses more resources */
};

test_suite_t local_conference_test_suite_end_to_end_encryption_impromptu_conference = {
    "Local conference tester (Impromptu Conference End to end encryption)",
    nullptr,
    nullptr,
    liblinphone_tester_before_each,
    liblinphone_tester_after_each,
    sizeof(local_conference_end_to_end_encryption_impromptu_conference_tests) /
        sizeof(local_conference_end_to_end_encryption_impromptu_conference_tests[0]),
    local_conference_end_to_end_encryption_impromptu_conference_tests,
    0,
    4};
