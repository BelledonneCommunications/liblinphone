/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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
#include "tester_utils.h"

#include <cstring>

#include "account/account.h"
#include "chat/encryption/encryption-engine.h"
#include "conference/client-conference.h"
#include "conference/encryption/client-ekt-manager.h"
#include "conference/encryption/ekt-info.h"

// =============================================================================

using namespace std;

using namespace LinphonePrivate;
using namespace LinphoneTest;

// -----------------------------------------------------------------------------

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
	auto sSpiVector = rng.randomize(2);
	uint16_t sspi = (uint16_t)((sSpiVector.at(0) << 8) & 0xffff);
	sspi = (uint16_t)((sSpiVector.at(1)) & 0xffff);
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

	string xmlBody = L_GET_CPP_PTR_FROM_C_OBJECT(marie->lc)->createXmlFromEktInfo(ei);
	lInfo() << "Generated XML body : " << endl << xmlBody;
	auto outputEi = L_GET_CPP_PTR_FROM_C_OBJECT(marie->lc)->createEktInfoFromXml(xmlBody);

	BC_ASSERT_EQUAL(outputEi->getSSpi(), ei->getSSpi(), uint16_t, "%u");
	if (exc == EktXmlContent::SpiInfo || exc == EktXmlContent::CipherTransport) {
		BC_ASSERT_EQUAL(outputEi->getCSpi().size(), ei->getCSpi().size(), int, "%i");
		BC_ASSERT_TRUE(outputEi->getCSpi() == ei->getCSpi());
		if (exc == EktXmlContent::CipherTransport) {
			BC_ASSERT_TRUE(outputEi->getFrom()->asStringUriOnly() == ei->getFrom()->asStringUriOnly());
			auto outputCipher = vector(outputEi->getCiphers()->getLinphoneBuffer(paulineAddr)->content,
			                           outputEi->getCiphers()->getLinphoneBuffer(paulineAddr)->content +
			                               outputEi->getCiphers()->getLinphoneBuffer(paulineAddr)->size);
			auto eiCipher = vector(ei->getCiphers()->getLinphoneBuffer(paulineAddr)->content,
			                       ei->getCiphers()->getLinphoneBuffer(paulineAddr)->content +
			                           ei->getCiphers()->getLinphoneBuffer(paulineAddr)->size);
			BC_ASSERT_TRUE(outputCipher == eiCipher);
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
	create_conference_base(ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionSRTP, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, FALSE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_simple_end_to_end_encrypted_conference_with_server_restart(void) {
	create_conference_base(ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionDTLS, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, TRUE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, TRUE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_simple_end_to_end_encrypted_conference_with_client_restart(void) {
	create_conference_base(ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionZRTP, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE,
	                       TRUE, FALSE, LinphoneMediaDirectionRecvOnly, TRUE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_end_to_end_encrypted_conference_with_uninvited_participant(void) {
	create_conference_base(ms_time(NULL), -1, TRUE, LinphoneConferenceParticipantListTypeOpen, TRUE,
	                       LinphoneMediaEncryptionSRTP, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, TRUE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_end_to_end_encrypted_conference_with_uninvited_participant_not_allowed(void) {
	create_conference_base(ms_time(NULL), -1, TRUE, LinphoneConferenceParticipantListTypeClosed, FALSE,
	                       LinphoneMediaEncryptionDTLS, FALSE, LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE,
	                       FALSE, FALSE, FALSE, FALSE, LinphoneMediaDirectionSendRecv, FALSE,
	                       LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_end_to_end_encrypted_conference_starting_immediately(void) {
	create_conference_base(ms_time(NULL), 0, FALSE, LinphoneConferenceParticipantListTypeClosed, FALSE,
	                       LinphoneMediaEncryptionZRTP, FALSE, LinphoneConferenceLayoutGrid, FALSE, FALSE, FALSE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, FALSE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_end_to_end_encrypted_conference_starting_in_the_past(void) {
	create_conference_base(ms_time(NULL) - 640, 11, FALSE, LinphoneConferenceParticipantListTypeClosed, TRUE,
	                       LinphoneMediaEncryptionSRTP, FALSE, LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE,
	                       FALSE, FALSE, FALSE, FALSE, LinphoneMediaDirectionSendRecv, FALSE,
	                       LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_simple_end_to_end_encrypted_conference_with_audio_only_participant(void) {
	create_conference_base(ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionDTLS, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, TRUE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionSendRecv, FALSE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_end_to_end_encrypted_conference_with_audio_only_and_uninvited_participant(void) {
	create_conference_base(ms_time(NULL), -1, TRUE, LinphoneConferenceParticipantListTypeOpen, TRUE,
	                       LinphoneMediaEncryptionZRTP, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, TRUE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionSendRecv, FALSE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_simple_end_to_end_encrypted_conference_with_audio_only_participant_enabling_video(void) {
	create_conference_base(ms_time(NULL), -1, FALSE, LinphoneConferenceParticipantListTypeOpen, FALSE,
	                       LinphoneMediaEncryptionSRTP, TRUE, LinphoneConferenceLayoutGrid, FALSE, FALSE, TRUE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionSendRecv, FALSE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_end_to_end_encrypted_conference_with_late_participant_addition(void) {
	create_conference_with_late_participant_addition_base(ms_time(NULL), -1, LinphoneConferenceLayoutGrid,
	                                                      LinphoneConferenceParticipantListTypeClosed, TRUE, TRUE,
	                                                      LinphoneConferenceSecurityLevelEndToEnd);
}

static void create_end_to_end_encrypted_conference_with_late_participant_addition_declined(void) {
	create_conference_with_late_participant_addition_base(ms_time(NULL), -1, LinphoneConferenceLayoutActiveSpeaker,
	                                                      LinphoneConferenceParticipantListTypeClosed, FALSE, TRUE,
	                                                      LinphoneConferenceSecurityLevelEndToEnd);
}

static void create_simple_end_to_end_encrypted_ice_conference(void) {
	create_conference_base(ms_time(NULL), -1, TRUE, LinphoneConferenceParticipantListTypeOpen, TRUE,
	                       LinphoneMediaEncryptionDTLS, TRUE, LinphoneConferenceLayoutGrid, TRUE, FALSE, FALSE, FALSE,
	                       FALSE, FALSE, LinphoneMediaDirectionRecvOnly, FALSE, LinphoneConferenceSecurityLevelEndToEnd,
	                       {LinphoneParticipantRoleSpeaker, LinphoneParticipantRoleListener}, FALSE);
}

static void create_simple_end_to_end_encrypted_conference_dial_out(void) {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
	create_conference_dial_out_base(FALSE, LinphoneConferenceLayoutActiveSpeaker, pol, FALSE, FALSE,
	                                LinphoneConferenceParticipantListTypeClosed, TRUE, FALSE,
	                                LinphoneConferenceSecurityLevelEndToEnd);
	linphone_video_activation_policy_unref(pol);
}

static void create_end_to_end_encryption_conference_with_audio_only_participants(void) {
	create_conference_with_audio_only_participants_base(LinphoneConferenceSecurityLevelEndToEnd);
}

static void create_simple_end_to_end_encrypted_conference_dial_out_with_some_calls_declined(void) {
	create_simple_conference_dial_out_with_some_calls_declined_base(LinphoneReasonDeclined,
	                                                                LinphoneConferenceSecurityLevelEndToEnd);
}

static void create_simple_end_to_end_encrypted_conference_dial_out_with_some_calls_busy(void) {
	create_simple_conference_dial_out_with_some_calls_declined_base(LinphoneReasonBusy,
	                                                                LinphoneConferenceSecurityLevelEndToEnd);
}

static void create_simple_end_to_end_encrypted_conference_with_screen_sharing(void) {
	create_conference_with_screen_sharing_base(
	    ms_time(NULL), -1, LinphoneMediaEncryptionZRTP, LinphoneConferenceLayoutActiveSpeaker, TRUE, TRUE, FALSE,
	    LinphoneMediaDirectionSendOnly, LinphoneConferenceSecurityLevelEndToEnd, {LinphoneParticipantRoleSpeaker});
}

static test_t local_conference_end_to_end_encryption_scheduled_conference_tests[] = {
    TEST_ONE_TAG("First notify", first_notify_ekt_xml_composing_parsing_test, "End2EndConf"),
    TEST_ONE_TAG("SPI info", spi_info_ekt_xml_composing_parsing_test, "End2EndConf"),
    TEST_ONE_TAG("Cipher transport", cipher_transport_ekt_xml_composing_parsing_test, "End2EndConf"),
    TEST_ONE_TAG(
        "Create simple end-to-end encrypted conference", create_simple_end_to_end_encrypted_conference, "End2EndConf"),
    TEST_TWO_TAGS("Create simple end-to-end encrypted conference with server restart",
                  create_simple_end_to_end_encrypted_conference_with_server_restart,
                  "LeaksMemory",
                  "End2EndConf"),
    TEST_TWO_TAGS("Create simple end-to-end encrypted conference with client restart",
                  create_simple_end_to_end_encrypted_conference_with_client_restart,
                  "LeaksMemory",
                  "End2EndConf"),
    TEST_TWO_TAGS("Create simple end-to-end encrypted ICE conference",
                  create_simple_end_to_end_encrypted_ice_conference,
                  "ICE",
                  "End2EndConf"),
    TEST_ONE_TAG("Create simple end-to-end encrypted conference with screen sharing override",
                 create_simple_end_to_end_encrypted_conference_with_screen_sharing,
                 "End2EndConf"),
    TEST_ONE_TAG("Create end-to-end encrypted conference with uninvited participant",
                 create_end_to_end_encrypted_conference_with_uninvited_participant,
                 "End2EndConf"),
    TEST_ONE_TAG("Create end-to-end encrypted conference with uninvited participant not allowed",
                 create_end_to_end_encrypted_conference_with_uninvited_participant_not_allowed,
                 "End2EndConf"),
    TEST_ONE_TAG("Create end-to-end encrypted conference with late participant addition declined",
                 create_end_to_end_encrypted_conference_with_late_participant_addition_declined,
                 "End2EndConf"),
    TEST_ONE_TAG("Create end-to-end encrypted conference starting immediately",
                 create_end_to_end_encrypted_conference_starting_immediately,
                 "End2EndConf"),
    TEST_ONE_TAG("Create end-to-end encrypted conference starting in the past",
                 create_end_to_end_encrypted_conference_starting_in_the_past,
                 "End2EndConf"),
    TEST_ONE_TAG("Create end-to-end encrypted conference with late participant addition",
                 create_end_to_end_encrypted_conference_with_late_participant_addition,
                 "End2EndConf"),
};

static test_t local_conference_end_to_end_encryption_scheduled_conference_audio_only_participant_tests[] = {
    TEST_ONE_TAG("Create simple end-to-end encrypted conference with audio only participant",
                 create_simple_end_to_end_encrypted_conference_with_audio_only_participant,
                 "End2EndConf"),
    TEST_ONE_TAG("Create end-to-end encrypted conference with audio only and uninvited participant",
                 create_end_to_end_encrypted_conference_with_audio_only_and_uninvited_participant,
                 "End2EndConf"),
    TEST_ONE_TAG("Create simple end-to-end encrypted conference with audio only participant enabling video",
                 create_simple_end_to_end_encrypted_conference_with_audio_only_participant_enabling_video,
                 "End2EndConf"),
    TEST_ONE_TAG("Create end-to-end encrypted conference with audio only participants",
                 create_end_to_end_encryption_conference_with_audio_only_participants,
                 "End2EndConf"),
};

static test_t local_conference_end_to_end_encryption_impromptu_conference_tests[] = {
    TEST_ONE_TAG("Create simple end-to-end encrypted dial out conference",
                 create_simple_end_to_end_encrypted_conference_dial_out,
                 "End2EndConf"),
    TEST_ONE_TAG("Create simple end-to-end encrypted conference dial out with some calls declined",
                 create_simple_end_to_end_encrypted_conference_dial_out_with_some_calls_declined,
                 "End2EndConf"),
    TEST_ONE_TAG("Create simple end-to-end encrypted conference dial out with some calls busy",
                 create_simple_end_to_end_encrypted_conference_dial_out_with_some_calls_busy,
                 "End2EndConf"),
};

test_suite_t local_conference_test_suite_end_to_end_encryption_scheduled_conference = {
    "Local conference tester (Scheduled Conference End to end encryption)",
    NULL,
    NULL,
    liblinphone_tester_before_each,
    liblinphone_tester_after_each,
    sizeof(local_conference_end_to_end_encryption_scheduled_conference_tests) /
        sizeof(local_conference_end_to_end_encryption_scheduled_conference_tests[0]),
    local_conference_end_to_end_encryption_scheduled_conference_tests,
    0,
    4};

test_suite_t local_conference_test_suite_end_to_end_encryption_scheduled_conference_audio_only_participant = {
    "Local conference tester (Audio only participants End to end encryption)",
    NULL,
    NULL,
    liblinphone_tester_before_each,
    liblinphone_tester_after_each,
    sizeof(local_conference_end_to_end_encryption_scheduled_conference_audio_only_participant_tests) /
        sizeof(local_conference_end_to_end_encryption_scheduled_conference_audio_only_participant_tests[0]),
    local_conference_end_to_end_encryption_scheduled_conference_audio_only_participant_tests,
    0,
    4};

test_suite_t local_conference_test_suite_end_to_end_encryption_impromptu_conference = {
    "Local conference tester (Impromptu Conference End to end encryption)",
    NULL,
    NULL,
    liblinphone_tester_before_each,
    liblinphone_tester_after_each,
    sizeof(local_conference_end_to_end_encryption_impromptu_conference_tests) /
        sizeof(local_conference_end_to_end_encryption_impromptu_conference_tests[0]),
    local_conference_end_to_end_encryption_impromptu_conference_tests,
    0,
    4};
