/*
 * Copyright (c) 2010-2025 Belledonne Communications SARL.
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

#ifdef HAVE_XERCESC
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
#endif // HAVE_XERCESC

static void create_simple_end_to_end_encrypted_conference() {
	create_conference_base(ms_time(nullptr), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionSRTP, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, FALSE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
	                       FALSE, C25519);
}

static void create_simple_post_quantum_end_to_end_encrypted_conference() {
	if (liblinphone_tester_is_lime_PQ_available()) {
		create_conference_base(ms_time(nullptr), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
		                       LinphoneMediaEncryptionSRTP, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE,
		                       FALSE, FALSE, FALSE, LinphoneMediaDirectionRecvOnly, FALSE,
		                       LinphoneConferenceSecurityLevelEndToEnd,
		                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
		                       FALSE, C25519MLK512);
	} else {
		ms_warning("This test requires post-quantum crypto support. Please rebuild with -DENABLE_NON_FREE_FEATURES=ON "
		           "and -DENABLE_PQCRYPTO=ON.");
	}
}

static void create_simple_end_to_end_encrypted_conference_with_server_restart() {
	create_conference_base(ms_time(nullptr), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionDTLS, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, TRUE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, TRUE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
	                       FALSE, C25519);
}

static void create_simple_end_to_end_encrypted_conference_with_client_restart() {
	create_conference_base(ms_time(nullptr), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionZRTP, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE,
	                       TRUE, FALSE, LinphoneMediaDirectionRecvOnly, TRUE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
	                       FALSE, C25519);
}

static void create_end_to_end_encrypted_conference_with_uninvited_participant() {
	create_conference_base(ms_time(nullptr), -1, TRUE, LinphoneConferenceParticipantListTypeOpen, TRUE,
	                       LinphoneMediaEncryptionSRTP, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, TRUE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
	                       FALSE, C25519);
}

static void create_end_to_end_encrypted_conference_with_uninvited_participant_not_allowed() {
	create_conference_base(
	    ms_time(nullptr), -1, TRUE, LinphoneConferenceParticipantListTypeClosed, FALSE, LinphoneMediaEncryptionDTLS,
	    FALSE, LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	    LinphoneMediaDirectionSendRecv, FALSE, LinphoneConferenceSecurityLevelEndToEnd,
	    {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE, FALSE, C25519);
}

static void create_end_to_end_encrypted_conference_starting_immediately() {
	create_conference_base(ms_time(nullptr), 0, FALSE, LinphoneConferenceParticipantListTypeClosed, FALSE,
	                       LinphoneMediaEncryptionZRTP, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, FALSE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
	                       FALSE, C25519);
}

static void create_end_to_end_encrypted_conference_starting_in_the_past() {
	create_conference_base(
	    ms_time(nullptr) - 640, 11, FALSE, LinphoneConferenceParticipantListTypeClosed, TRUE,
	    LinphoneMediaEncryptionSRTP, FALSE, LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE, FALSE, FALSE, FALSE,
	    FALSE, LinphoneMediaDirectionSendRecv, FALSE, LinphoneConferenceSecurityLevelEndToEnd,
	    {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE, FALSE, C25519);
}

static void create_simple_end_to_end_encrypted_conference_with_audio_only_participant() {
	create_conference_base(ms_time(nullptr), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionDTLS, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, TRUE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionSendRecv, FALSE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
	                       FALSE, C25519);
}

static void create_end_to_end_encrypted_conference_with_audio_only_and_uninvited_participant() {
	create_conference_base(ms_time(nullptr), -1, TRUE, LinphoneConferenceParticipantListTypeOpen, TRUE,
	                       LinphoneMediaEncryptionZRTP, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, TRUE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionSendRecv, FALSE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
	                       FALSE, C25519);
}

static void create_simple_end_to_end_encrypted_conference_with_audio_only_participant_enabling_video() {
	create_conference_base(ms_time(nullptr), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionSRTP, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, TRUE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionSendRecv, FALSE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
	                       FALSE, C25519);
}

static void create_simple_end_to_end_encrypted_ice_conference() {
	create_conference_base(ms_time(nullptr), -1, TRUE, LinphoneConferenceParticipantListTypeOpen, TRUE,
	                       LinphoneMediaEncryptionDTLS, TRUE, LinphoneConferenceLayoutGrid, TRUE, FALSE, FALSE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, FALSE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, FALSE, FALSE,
	                       FALSE, C25519);
}

static void create_end_to_end_encrypted_conference_terminate_call_on_version_mismatch() {
	create_conference_base(ms_time(nullptr), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionSRTP, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, FALSE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE, TRUE, FALSE, FALSE,
	                       C25519);
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
		LinphoneTesterLimeAlgo lime_algo = C25519;
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), lime_algo);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), lime_algo);
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress(), lime_algo);
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress(), lime_algo);
		ClientConference berthe("berthe_rc", focus.getConferenceFactoryAddress(), lime_algo);

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

#ifdef HAVE_ADVANCED_IM
		does_all_participants_have_matching_ekt(focus.getCMgr(), memberList, confAddr);
#endif // HAVE_ADVANCED_IM

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

#ifdef HAVE_ADVANCED_IM
			does_all_participants_have_matching_ekt(focus.getCMgr(), memberList, confAddr);
#endif // HAVE_ADVANCED_IM

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

#ifdef HAVE_ADVANCED_IM
			does_all_participants_have_matching_ekt(focus.getCMgr(), memberList, confAddr);
#endif // HAVE_ADVANCED_IM

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

static void create_encrypted_conference_with_chat_and_cores_restart() {
	create_conference_with_chat_base(LinphoneConferenceSecurityLevelEndToEnd, TRUE, TRUE, TRUE, 1, FALSE, TRUE, FALSE,
	                                 (ms_time(NULL) - 15), FALSE);
}

static void create_encrypted_conference_with_chat_network_drops_and_participant_rejoining() {
	create_conference_with_chat_base(LinphoneConferenceSecurityLevelEndToEnd, FALSE, FALSE, FALSE, -1, TRUE, TRUE, TRUE,
	                                 ms_time(NULL), TRUE);
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
		LinphoneTesterLimeAlgo lime_algo = C25519;
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), lime_algo);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), lime_algo);

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
		const LinphoneTesterLimeAlgo lime_algo = C25519;
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), lime_algo);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), lime_algo);

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

static void create_simple_end_to_end_encrypted_conference_with_chat_merging_calls() {
	create_simple_conference_merging_calls_base(FALSE, LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE, FALSE,
	                                            LinphoneConferenceSecurityLevelEndToEnd, FALSE, TRUE);
}

static void encrypted_conference_joined_multiple_times() {
	conference_joined_multiple_times_base(LinphoneConferenceSecurityLevelEndToEnd, FALSE, -1, FALSE);
}

static void encrypted_conference_joined_multiple_times_with_chat() {
	conference_joined_multiple_times_base(LinphoneConferenceSecurityLevelEndToEnd, TRUE, -1, FALSE);
}

static void encrypted_conference_joined_multiple_times_with_chat_keeping_client_ekt_refs() {
	conference_joined_multiple_times_base(LinphoneConferenceSecurityLevelEndToEnd, TRUE, -1, TRUE);
}

static void admin_removes_and_reinvite_participant_to_encrypted_conference_base(bool_t enable_chat,
                                                                                bool_t restarts_core) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		const LinphoneTesterLimeAlgo lime_algo = C25519;

		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), lime_algo);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), lime_algo);
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress(), lime_algo);
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress(), lime_algo);
		ClientConference berthe("berthe_rc", focus.getConferenceFactoryAddress(), lime_algo);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(berthe);

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t *coresList = NULL;

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

			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip", "update_call_when_ice_completed", TRUE);
			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip",
			                        "update_call_when_ice_completed_with_dtls", FALSE);

			// Enable ICE at the account level but not at the core level
			enable_stun_in_mgr(mgr, TRUE, TRUE, FALSE, FALSE);

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		configure_end_to_end_encrypted_conference_server(focus);

		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(berthe.getLc()));

		int nortp_timeout = 10;
		linphone_core_set_nortp_timeout(marie.getLc(), nortp_timeout);
		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), LinphoneConferenceParticipantListTypeOpen);

		stats focus_stat = focus.getStats();
		std::list<LinphoneCoreManager *> participants{laure.getCMgr(), michelle.getCMgr(), berthe.getCMgr(),
		                                              pauline.getCMgr()};
		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), laure.getCMgr(),  michelle.getCMgr(),
		                                                marie.getCMgr(), berthe.getCMgr(), pauline.getCMgr()};
		std::list<LinphoneCoreManager *> members{michelle.getCMgr(), berthe.getCMgr(), laure.getCMgr(), marie.getCMgr(),
		                                         pauline.getCMgr()};

		LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;
		time_t start_time(ms_time(NULL) - 3);
		time_t end_time = -1;
		const char *initialSubject = "Test characters: ^ :) ¤ çà @";
		const char *description = "Paris Baker";

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;

		for (auto mgr : participants) {
			LinphoneAddress *identity = linphone_address_new(linphone_core_get_identity(mgr->lc));
			participantList.insert(std::make_pair(
			    mgr, add_participant_info_to_list(&participants_info, identity, LinphoneParticipantRoleSpeaker, -1)));
			linphone_address_unref(identity);
		}

		bool_t enable_video = TRUE;
		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, security_level, enable_video, enable_chat, NULL);
		BC_ASSERT_PTR_NOT_NULL(confAddr);
		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:");

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 4,
		                             liblinphone_tester_sip_timeout));

		// All clients enter the conference
		const LinphoneMediaEncryption encryption = LinphoneMediaEncryptionNone;
		focus_stat = focus.getStats();
		std::list<std::pair<LinphoneCoreManager *, stats>> member_stats_list;
		for (auto mgr : members) {
			member_stats_list.push_back(std::make_pair(mgr, mgr->stat));
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_call_params_set_media_encryption(new_params, encryption);
			linphone_call_params_set_account(new_params, linphone_core_get_default_account(mgr->lc));
			linphone_call_params_enable_video(new_params, TRUE);
			ms_message("%s is entering conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
			LinphoneCall *participant_call = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(participant_call);
			if (participant_call) {
				LinphoneCallLog *call_log = linphone_call_get_call_log(participant_call);
				BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
			}
		}

		auto nb_members = static_cast<int>(members.size());
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
			int expected_chatroom_nb = 0;
			if (mgr == marie.getCMgr()) {
				expected_chatroom_nb = nb_members - 1;
			} else {
				expected_chatroom_nb = 1;
			}
			if (enable_chat) {
				expected_chatroom_nb++;
			}
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneChatRoomStateCreated,
			                             expected_chatroom_nb, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + nb_members,
		                             liblinphone_tester_sip_timeout));
		int focus_no_streams_running = 2 * nb_members;
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + nb_members,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running,
		                             liblinphone_tester_sip_timeout));
		// If ICE is enabled, the addition to a conference may go through a resume of the call
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
		                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + nb_members,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + nb_members,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + nb_members,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + nb_members,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
		                             focus_stat.number_of_conference_participant_devices_present + nb_members,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
		                             focus_stat.number_of_participant_devices_present + nb_members,
		                             liblinphone_tester_sip_timeout));

		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
		    fill_member_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs, focus.getCMgr(),
		                            memberList, confAddr, enable_video);

		int msg_cnt = 0;
		const std::initializer_list<std::reference_wrapper<ClientConference>> cores{marie, laure, pauline, berthe,
		                                                                            michelle};
		for (ClientConference &core : cores) {
			LinphoneConference *conference = linphone_core_search_conference_2(core.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(conference);
			if (conference) {
				LinphoneChatRoom *chat_room = linphone_conference_get_chat_room(conference);
				if (enable_chat) {
					BC_ASSERT_PTR_NOT_NULL(chat_room);
					std::string msg_text = std::string("Welcome to all to conference ") +
					                       ChatRoom::toCpp(chat_room)->getConferenceAddress()->toString() +
					                       std::string(" by ") + core.getIdentity().toString();
					LinphoneChatMessage *msg = ClientConference::sendTextMsg(chat_room, msg_text);
					BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).wait([&msg] {
						return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
					}));
					linphone_chat_message_unref(msg);

					msg_cnt++;

					for (ClientConference &core2 : cores) {
						LinphoneChatRoom *cr =
						    linphone_core_search_chat_room(core2.getLc(), NULL, NULL, confAddr, NULL);
						BC_ASSERT_PTR_NOT_NULL(cr);
						if (cr) {
							CoreManagerAssert({focus, marie, pauline, laure})
							    .waitUntil(std::chrono::seconds(10), [&cr, msg_cnt] {
								    return linphone_chat_room_get_history_size(cr) == msg_cnt;
							    });
						}
					}
				} else {
					BC_ASSERT_PTR_NULL(chat_room);
				}
			}
		}

		Address laureAddr = laure.getIdentity();
		LinphoneConference *marie_conference = linphone_core_search_conference_2(marie.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(marie_conference);
		if (marie_conference) {
			stats marie_stat = marie.getStats();
			stats pauline_stat = pauline.getStats();
			stats laure_stat = laure.getStats();
			stats michelle_stat = michelle.getStats();
			stats berthe_stat = berthe.getStats();
			focus_stat = focus.getStats();

			LinphoneParticipant *laure_participant =
			    linphone_conference_find_participant(marie_conference, laureAddr.toC());
			BC_ASSERT_PTR_NOT_NULL(laure_participant);
			ms_message("%s removes %s from conference %s", linphone_core_get_identity(marie.getLc()),
			           linphone_core_get_identity(laure.getLc()), conference_address_str);
			linphone_conference_remove_participant_2(marie_conference, laure_participant);
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
			                             focus_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallEnd,
			                             laure_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
			                             focus_stat.number_of_LinphoneCallReleased + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallReleased,
			                             laure_stat.number_of_LinphoneCallReleased + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed,
			                             marie_stat.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_removed,
			                             pauline_stat.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participants_removed,
			                             michelle_stat.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_participants_removed,
			                             berthe_stat.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateTerminated,
			                             laure_stat.number_of_LinphoneConferenceStateTerminated + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneSubscriptionTerminated,
			                             laure_stat.number_of_LinphoneSubscriptionTerminated + 1,
			                             liblinphone_tester_sip_timeout));
			if (enable_chat) {
				BC_ASSERT_FALSE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateDeleted,
				                              laure_stat.number_of_LinphoneConferenceStateDeleted + 1, 2000));
			} else {
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateDeleted,
				                             laure_stat.number_of_LinphoneConferenceStateDeleted + 1,
				                             liblinphone_tester_sip_timeout));
			}

			if (restarts_core) {
				coresList = bctbx_list_remove(coresList, laure.getLc());
				// Restart Laure
				ms_message("%s is restarting its core", linphone_core_get_identity(laure.getLc()));
				laure.reStart();
				coresList = bctbx_list_append(coresList, laure.getLc());
				if (enable_chat) {
					LinphoneConference *conference = linphone_core_search_conference_2(laure.getLc(), confAddr);
					BC_ASSERT_PTR_NOT_NULL(conference);
					LinphoneChatRoom *chat_room =
					    linphone_core_search_chat_room(laure.getLc(), NULL, NULL, confAddr, NULL);
					BC_ASSERT_PTR_NOT_NULL(chat_room);
					BC_ASSERT_PTR_EQUAL(chat_room, linphone_conference_get_chat_room(conference));
				}

				linphone_core_set_video_device(laure.getLc(), liblinphone_tester_mire_id);
				linphone_core_enable_video_capture(laure.getLc(), TRUE);
				linphone_core_enable_video_display(laure.getLc(), TRUE);

				LinphoneVideoActivationPolicy *pol =
				    linphone_factory_create_video_activation_policy(linphone_factory_get());
				linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
				linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
				linphone_core_set_video_activation_policy(laure.getLc(), pol);
				linphone_video_activation_policy_unref(pol);
				const LinphoneVideoActivationPolicy *current_pol =
				    linphone_core_get_video_activation_policy(laure.getLc());
				BC_ASSERT_TRUE(linphone_video_activation_policy_get_automatically_accept(current_pol));
				BC_ASSERT_TRUE(linphone_video_activation_policy_get_automatically_initiate(current_pol));
			}

			marie_stat = marie.getStats();
			pauline_stat = pauline.getStats();
			laure_stat = laure.getStats();
			michelle_stat = michelle.getStats();
			berthe_stat = berthe.getStats();
			focus_stat = focus.getStats();

			ms_message("%s adds again %s to conference %s", linphone_core_get_identity(marie.getLc()),
			           linphone_core_get_identity(laure.getLc()), conference_address_str);
			linphone_conference_add_participant_2(marie_conference, laure.getCMgr()->identity);
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingInit,
			                             focus_stat.number_of_LinphoneCallOutgoingInit + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallIncomingReceived,
			                             laure_stat.number_of_LinphoneCallIncomingReceived + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingRinging,
			                             focus_stat.number_of_LinphoneCallOutgoingRinging + 1,
			                             liblinphone_tester_sip_timeout));
			LinphoneCall *laure_call =
			    linphone_core_get_call_by_remote_address2(laure.getLc(), focus.getCMgr()->identity);
			BC_ASSERT_PTR_NOT_NULL(laure_call);
			if (laure_call) {
				BC_ASSERT_TRUE(linphone_call_get_state(laure_call) == LinphoneCallIncomingReceived);
				LinphoneCallParams *new_params = linphone_core_create_call_params(laure.getLc(), laure_call);
				linphone_call_params_enable_video(new_params, enable_video);
				linphone_call_params_set_video_direction(new_params, LinphoneMediaDirectionSendRecv);
				linphone_call_accept_with_params(laure_call, new_params);
				linphone_call_params_unref(new_params);
			}
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
			                             focus_stat.number_of_LinphoneCallStreamsRunning + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallStreamsRunning,
			                             laure_stat.number_of_LinphoneCallStreamsRunning + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_added,
			                             marie_stat.number_of_participants_added + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_added,
			                             pauline_stat.number_of_participants_added + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participants_added,
			                             michelle_stat.number_of_participants_added + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_participants_added,
			                             berthe_stat.number_of_participants_added + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateCreationPending,
			                             laure_stat.number_of_LinphoneConferenceStateCreationPending + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateCreated,
			                             laure_stat.number_of_LinphoneConferenceStateCreated + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneSubscriptionActive,
			                             laure_stat.number_of_LinphoneSubscriptionActive + 1,
			                             liblinphone_tester_sip_timeout));

			wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs,
			                            focus.getCMgr(), memberList, confAddr, enable_video);
		}

		for (ClientConference &core : cores) {
			LinphoneConference *conference = linphone_core_search_conference_2(core.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(conference);
			if (conference) {
				const LinphoneConferenceParams *conference_params = linphone_conference_get_current_params(conference);
				BC_ASSERT_TRUE(linphone_conference_params_audio_enabled(conference_params));
				BC_ASSERT_TRUE(linphone_conference_params_video_enabled(conference_params));
				BC_ASSERT_TRUE(linphone_conference_params_chat_enabled(conference_params) == enable_chat);
				LinphoneParticipant *laure_participant =
				    (core.getLc() == laure.getLc()) ? linphone_conference_get_me(conference)
				                                    : linphone_conference_find_participant(conference, laureAddr.toC());
				BC_ASSERT_PTR_NOT_NULL(laure_participant);
				if (laure_participant) {
					bctbx_list_t *laure_devices = linphone_participant_get_devices(laure_participant);
					for (bctbx_list_t *itd = laure_devices; itd; itd = bctbx_list_next(itd)) {
						LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
						BC_ASSERT_EQUAL(linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeAudio),
						                LinphoneMediaDirectionSendRecv, int, "%0d");
						BC_ASSERT_EQUAL(linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo),
						                LinphoneMediaDirectionSendRecv, int, "%0d");
					}
					if (laure_devices) {
						bctbx_list_free_with_data(laure_devices, (void (*)(void *))linphone_participant_device_unref);
					}
				}
				LinphoneChatRoom *chat_room = linphone_conference_get_chat_room(conference);
				if (enable_chat) {
					BC_ASSERT_PTR_NOT_NULL(chat_room);
					std::string msg_text = std::string("Verifying my network on conference ") +
					                       ChatRoom::toCpp(chat_room)->getConferenceAddress()->toString() +
					                       std::string(" client ") + core.getIdentity().toString();
					LinphoneChatMessage *msg = ClientConference::sendTextMsg(chat_room, msg_text);
					BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).wait([&msg] {
						return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
					}));
					linphone_chat_message_unref(msg);

					msg_cnt++;

					for (ClientConference &core2 : cores) {
						LinphoneChatRoom *cr =
						    linphone_core_search_chat_room(core2.getLc(), NULL, NULL, confAddr, NULL);
						BC_ASSERT_PTR_NOT_NULL(cr);
						if (cr) {
							CoreManagerAssert({focus, marie, pauline, laure})
							    .waitUntil(std::chrono::seconds(10), [&cr, msg_cnt] {
								    return linphone_chat_room_get_history_size(cr) == msg_cnt;
							    });
						}
					}
				} else {
					BC_ASSERT_PTR_NULL(chat_room);
				}
			}
		}

		focus_stat = focus.getStats();

		const bctbx_list_t *focus_calls = linphone_core_get_calls(focus.getLc());
		size_t focus_calls_nb = bctbx_list_size(focus_calls);
		BC_ASSERT_EQUAL(focus_calls_nb, members.size(), size_t, "%zu");

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(10), [] {
			return false;
		});

		auto remaining_members = members;
		for (auto mgr : members) {
			int participant_call_ended = ((mgr == laure.getCMgr() && !restarts_core) ? 2 : 1);
			stats focus_stat2 = focus.getStats();
			remaining_members.pop_front();
			std::list<stats> remaining_members_stats;
			int nb_members = 0;
			for (auto remaining_mgr : remaining_members) {
				remaining_members_stats.push_back(remaining_mgr->stat);
				LinphoneCall *pcall =
				    linphone_core_get_call_by_remote_address2(remaining_mgr->lc, focus.getCMgr()->identity);
				BC_ASSERT_PTR_NOT_NULL(pcall);
				if (pcall) {
					const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
					if (linphone_call_params_video_enabled(call_cparams)) {
						nb_members++;
					}
				}
			}
			const bctbx_list_t *participant_calls = linphone_core_get_calls(mgr->lc);
			int participant_calls_nb = static_cast<int>(bctbx_list_size(participant_calls));
			BC_ASSERT_EQUAL(participant_calls_nb, 1, int, "%d");

			LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
			BC_ASSERT_PTR_NOT_NULL(call);
			if (call) {
				ms_message("%s is terminating call with %s", linphone_core_get_identity(mgr->lc),
				           linphone_core_get_identity(focus.getLc()));
				linphone_call_terminate(call);
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, participant_call_ended,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, participant_call_ended,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated,
			                             participant_call_ended, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending,
			                             participant_call_ended, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated,
			                             participant_call_ended, liblinphone_tester_sip_timeout));
			if (enable_chat) {
				BC_ASSERT_FALSE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateDeleted,
				                              participant_call_ended, 1000));
			} else {
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateDeleted,
				                             participant_call_ended, liblinphone_tester_sip_timeout));
			}

			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr, NULL);
			if (enable_chat) {
				BC_ASSERT_PTR_NOT_NULL(pconference);
			} else {
				BC_ASSERT_PTR_NULL(pconference);
			}

			participant_calls = linphone_core_get_calls(mgr->lc);
			participant_calls_nb = static_cast<int>(bctbx_list_size(participant_calls));
			BC_ASSERT_EQUAL(participant_calls_nb, 0, int, "%d");

			for (auto remaining_mgr : remaining_members) {
				stats stat = remaining_members_stats.front();
				remaining_members_stats.pop_front();
				BC_ASSERT_TRUE(wait_for_list(coresList, &remaining_mgr->stat.number_of_participants_removed,
				                             stat.number_of_participants_removed + 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &remaining_mgr->stat.number_of_participant_devices_removed,
				                             stat.number_of_participant_devices_removed + 1,
				                             liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
			                             focus_stat2.number_of_LinphoneCallUpdatedByRemote + nb_members,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
			                             focus_stat2.number_of_LinphoneCallStreamsRunning + nb_members,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
			                             focus_stat2.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
			                             focus_stat2.number_of_participant_devices_removed + 1,
			                             liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
		                             focus_stat.number_of_LinphoneCallEnd + static_cast<int>(focus_calls_nb),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + static_cast<int>(focus_calls_nb),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
		                  focus_stat.number_of_LinphoneSubscriptionTerminated + static_cast<int>(focus_calls_nb),
		                  liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
		                             focus_stat.number_of_participants_removed + static_cast<int>(focus_calls_nb),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                  focus_stat.number_of_participant_devices_removed + static_cast<int>(focus_calls_nb),
		                  liblinphone_tester_sip_timeout));

		for (auto mgr : {focus.getCMgr()}) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NULL(pconference);
		}

		focus_stat = focus.getStats();
		const bctbx_list_t *calls = linphone_core_get_calls(focus.getLc());
		BC_ASSERT_EQUAL(bctbx_list_size(calls), 0, size_t, "%zu");

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : members) {
			unsigned int expected_call_logs = (mgr == laure.getCMgr()) ? 2 : 1;
			const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
			BC_ASSERT_EQUAL(bctbx_list_size(call_logs), expected_call_logs, size_t, "%zu");

			bctbx_list_t *mgr_focus_call_log =
			    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
			if (mgr_focus_call_log) {
				BC_ASSERT_EQUAL(bctbx_list_size(mgr_focus_call_log), expected_call_logs, size_t, "%zu");
				for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
					LinphoneCallLog *call_log = (LinphoneCallLog *)it->data;
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				}
				bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
			}

			bctbx_list_t *participants_info2 =
			    bctbx_list_copy_with_data(participants_info, (bctbx_list_copy_func)linphone_participant_info_clone);
			for (bctbx_list_t *it = participants_info2; it; it = bctbx_list_next(it)) {
				LinphoneParticipantInfo *participant_info = (LinphoneParticipantInfo *)bctbx_list_get_data(it);
				if (mgr == laure.getCMgr()) {
					linphone_participant_info_set_sequence_number(participant_info, -1);
				} else {
					linphone_participant_info_set_sequence_number(participant_info, 0);
				}
			}

			check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info2, 0, 0,
			                            initialSubject, description, 0, LinphoneConferenceInfoStateNew, security_level,
			                            FALSE, TRUE, enable_video, enable_chat);
			bctbx_list_free_with_data(participants_info2, (bctbx_list_free_func)linphone_participant_info_unref);
		}

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
			return false;
		});

		ms_free(conference_address_str);
		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void admin_removes_and_reinvite_participant_to_encrypted_conference() {
	admin_removes_and_reinvite_participant_to_encrypted_conference_base(FALSE, FALSE);
}

static void admin_removes_and_reinvite_participant_to_encrypted_conference_with_chat() {
	admin_removes_and_reinvite_participant_to_encrypted_conference_base(TRUE, FALSE);
}

static void admin_removes_and_reinvite_participant_to_encrypted_conference_with_chat_after_restarting_core() {
	admin_removes_and_reinvite_participant_to_encrypted_conference_base(TRUE, TRUE);
}

} // namespace LinphoneTest

static test_t local_conference_end_to_end_encryption_scheduled_conference_tests[] = {
#ifdef HAVE_XERCESC
    TEST_ONE_TAG("First notify", LinphoneTest::first_notify_ekt_xml_composing_parsing_test, "End2EndConf"),
    TEST_ONE_TAG("SPI info", LinphoneTest::spi_info_ekt_xml_composing_parsing_test, "End2EndConf"),
    TEST_ONE_TAG("Cipher transport", LinphoneTest::cipher_transport_ekt_xml_composing_parsing_test, "End2EndConf"),
#endif // HAVE_XERCESC
    TEST_ONE_TAG("End-to-End Conference joined multiple times",
                 LinphoneTest::encrypted_conference_joined_multiple_times,
                 "End2EndConf"),
    TEST_ONE_TAG("Create simple end-to-end encrypted conference",
                 LinphoneTest::create_simple_end_to_end_encrypted_conference,
                 "End2EndConf"),
    TEST_ONE_TAG("Create simple post-quantum end-to-end encrypted conference",
                 LinphoneTest::create_simple_post_quantum_end_to_end_encrypted_conference,
                 "End2EndConf"),
    TEST_ONE_TAG("Create simple end-to-end encrypted conference with server restart",
                 LinphoneTest::create_simple_end_to_end_encrypted_conference_with_server_restart,
                 "End2EndConf"),
    TEST_ONE_TAG("Create simple end-to-end encrypted conference with client restart",
                 LinphoneTest::create_simple_end_to_end_encrypted_conference_with_client_restart,
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
    TEST_TWO_TAGS(
        "Create simple end-to-end encrypted conference with participant added by admin call accepted",
        LinphoneTest::create_simple_end_to_end_encrypted_conference_with_participant_added_by_admin_call_accepted,
        "End2EndConf",
        "shaky"),
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
    TEST_ONE_TAG("Admin removes and reinvites participant to encrypted conference",
                 LinphoneTest::admin_removes_and_reinvite_participant_to_encrypted_conference,
                 "End2EndConf")};

static test_t local_conference_end_to_end_encryption_scheduled_conference_with_chat_tests[] = {
    TEST_ONE_TAG(
        "Create encrypted conference with chat", LinphoneTest::create_encrypted_conference_with_chat, "End2EndConf"),
    TEST_ONE_TAG("Create encrypted conference with chat and cores restart",
                 LinphoneTest::create_encrypted_conference_with_chat_and_cores_restart,
                 "End2EndConf"),
    TEST_ONE_TAG("Create encrypted conference with chat network drops and participant rejoining",
                 LinphoneTest::create_encrypted_conference_with_chat_network_drops_and_participant_rejoining,
                 "End2EndConf"),
    TEST_ONE_TAG("Create simple end-to-end encrypted dial out conference with chat",
                 LinphoneTest::create_simple_end_to_end_encrypted_conference_dial_out_with_chat,
                 "End2EndConf"),
    TEST_ONE_TAG("End-to-End Conference joined multiple times with chat",
                 LinphoneTest::encrypted_conference_joined_multiple_times_with_chat,
                 "End2EndConf"),
    TEST_ONE_TAG("End-to-End Conference joined multiple times with chat keeping client EKT manager refs",
                 LinphoneTest::encrypted_conference_joined_multiple_times_with_chat_keeping_client_ekt_refs,
                 "End2EndConf"),
    TEST_ONE_TAG("Admin removes and reinvites participant to encrypted conference with chat",
                 LinphoneTest::admin_removes_and_reinvite_participant_to_encrypted_conference_with_chat,
                 "End2EndConf"),
    TEST_ONE_TAG(
        "Admin removes and reinvites participant to encrypted conference with chat after restarting the core",
        LinphoneTest::admin_removes_and_reinvite_participant_to_encrypted_conference_with_chat_after_restarting_core,
        "End2EndConf")};

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
    TEST_ONE_TAG("Create simple end-to-end encrypted conference with chat by merging calls",
                 LinphoneTest::create_simple_end_to_end_encrypted_conference_with_chat_merging_calls,
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
