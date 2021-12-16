/*
 * Copyright (c) 2010-2021 Belledonne Communications SARL.
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

#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "call/call-log.h"
#include "conference/conference-info.h"
#include "core/core.h"
#include "core/core-p.h"

#include "liblinphone_tester.h"
#include "tester_utils.h"

// =============================================================================

using namespace std;

using namespace LinphonePrivate;

static void get_conference_info_from_call_log () {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");

	// Create a fake call log
	auto callLog = CallLog::create(L_GET_CPP_PTR_FROM_C_OBJECT(marie->lc)->getSharedFromThis(),
		LinphoneCallIncoming,
		linphone_address_clone(marie->identity),
		linphone_address_new("sip:video-conf-test@sip.linphone.org;conf-id=K4lHv;gr=60610d90-d695-0009-b3a1-331c5842bae0"));

	callLog->setDuration(120);
	callLog->setStatus(LinphoneCallSuccess);
	callLog->setQuality(1.0);
	callLog->setCallId("FTD9-zVeBW");

	// Create a conference info
	auto conferenceInfo = ConferenceInfo::create();

	conferenceInfo->setOrganizer(IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(marie->identity)));
	conferenceInfo->setUri(ConferenceAddress("sip:video-conf-test@sip.linphone.org;gr=60610d90-d695-0009-b3a1-331c5842bae0;conf-id=K4lHv"));
	conferenceInfo->setDateTime(std::time(nullptr));
	conferenceInfo->setDuration(30);
	conferenceInfo->setSubject("Test de vidéo conférence");
	conferenceInfo->setDescription("Réunion pour parler de la vidéo conférence.");
	conferenceInfo->addParticipant(IdentityAddress("sip:laure@sip.linphone.org"));

	L_GET_CPP_PTR_FROM_C_OBJECT(marie->lc)->reportConferenceCallEvent(EventLog::Type::ConferenceCallStarted, callLog, conferenceInfo);

	BC_ASSERT_TRUE(callLog->wasConference());
	BC_ASSERT_PTR_NOT_NULL(callLog->getConferenceInfo());

	linphone_core_manager_destroy(marie);
}

static void get_existing_conference_info_from_call_log () {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");

	// Create a fake call log
	auto callLog = CallLog::create(L_GET_CPP_PTR_FROM_C_OBJECT(marie->lc)->getSharedFromThis(),
		LinphoneCallIncoming,
		linphone_address_clone(marie->identity),
		linphone_address_new("sip:video-conf-test@sip.linphone.org;conf-id=K4lHv;gr=60610d90-d695-0009-b3a1-331c5842bae0"));

	callLog->setDuration(120);
	callLog->setStatus(LinphoneCallSuccess);
	callLog->setQuality(1.0);
	callLog->setCallId("FTD9-zVeBW");

	// Create a conference info
	auto conferenceInfo = ConferenceInfo::create();

	conferenceInfo->setOrganizer(IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(marie->identity)));
	conferenceInfo->setUri(ConferenceAddress("sip:video-conf-test@sip.linphone.org;gr=60610d90-d695-0009-b3a1-331c5842bae0;conf-id=K4lHv"));
	conferenceInfo->setDateTime(std::time(nullptr));
	conferenceInfo->setDuration(30);
	conferenceInfo->setSubject("Test de vidéo conférence");
	conferenceInfo->setDescription("Réunion pour parler de la vidéo conférence.");
	conferenceInfo->addParticipant(IdentityAddress("sip:laure@sip.linphone.org"));

	// First insert the conference info into DB
	L_GET_PRIVATE_FROM_C_OBJECT(marie->lc)->mainDb->insertConferenceInfo(conferenceInfo);

	// Report the call event without specifying the conference info
	L_GET_CPP_PTR_FROM_C_OBJECT(marie->lc)->reportConferenceCallEvent(EventLog::Type::ConferenceCallStarted, callLog, nullptr);

	BC_ASSERT_TRUE(callLog->wasConference());
	BC_ASSERT_PTR_NOT_NULL(callLog->getConferenceInfo());

	linphone_core_manager_destroy(marie);
}

static void last_outgoing_call_without_conference () {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneAddress *paulineAddr = linphone_address_new("sip:pauline@sip.linphone.org");

	// Create a fake call log
	auto callLog = CallLog::create(L_GET_CPP_PTR_FROM_C_OBJECT(marie->lc)->getSharedFromThis(),
		LinphoneCallOutgoing,
		linphone_address_clone(marie->identity),
		linphone_address_clone(paulineAddr));

	callLog->setDuration(60);
	callLog->setStatus(LinphoneCallSuccess);
	callLog->setQuality(0.69f);
	callLog->setCallId("cAlL-iDdD");

	// Report the call
	L_GET_CPP_PTR_FROM_C_OBJECT(marie->lc)->reportConferenceCallEvent(EventLog::Type::ConferenceCallStarted, callLog, nullptr);

	// Create a new fake call log to a conference
	callLog = CallLog::create(L_GET_CPP_PTR_FROM_C_OBJECT(marie->lc)->getSharedFromThis(),
		LinphoneCallOutgoing,
		linphone_address_clone(marie->identity),
		linphone_address_new("sip:video-conf-test@sip.linphone.org;conf-id=K4lHv;gr=60610d90-d695-0009-b3a1-331c5842bae0"));

	callLog->setDuration(120);
	callLog->setStatus(LinphoneCallSuccess);
	callLog->setQuality(1.0);
	callLog->setCallId("FTD9-zVeBW");

	// Create a conference info
	auto conferenceInfo = ConferenceInfo::create();

	conferenceInfo->setOrganizer(IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(marie->identity)));
	conferenceInfo->setUri(ConferenceAddress("sip:video-conf-test@sip.linphone.org;gr=60610d90-d695-0009-b3a1-331c5842bae0;conf-id=K4lHv"));
	conferenceInfo->setDateTime(std::time(nullptr));
	conferenceInfo->setDuration(30);
	conferenceInfo->setSubject("Test de vidéo conférence");
	conferenceInfo->setDescription("Réunion pour parler de la vidéo conférence.");
	conferenceInfo->addParticipant(IdentityAddress("sip:laure@sip.linphone.org"));

	// Report the call event without specifying the conference info
	L_GET_CPP_PTR_FROM_C_OBJECT(marie->lc)->reportConferenceCallEvent(EventLog::Type::ConferenceCallEnded, callLog, conferenceInfo);

	auto lastCall = L_GET_PRIVATE_FROM_C_OBJECT(marie->lc)->mainDb->getLastOutgoingCall();

	BC_ASSERT_PTR_NOT_NULL(lastCall);
	if (lastCall != nullptr) {
		BC_ASSERT_TRUE(linphone_address_equal(lastCall->getToAddress(), paulineAddr));
	}

	linphone_address_unref(paulineAddr);
	linphone_core_manager_destroy(marie);
}

test_t conference_tests[] = {
	TEST_NO_TAG("Get conference info from call log", get_conference_info_from_call_log),
	TEST_NO_TAG("Get existing conference info from call log", get_existing_conference_info_from_call_log),
	TEST_NO_TAG("Last outgoing call does not return calls with conference info", last_outgoing_call_without_conference),
};

test_suite_t conference_info_tester = {
	"Conference Info", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
	sizeof(conference_tests) / sizeof(conference_tests[0]), conference_tests
};
