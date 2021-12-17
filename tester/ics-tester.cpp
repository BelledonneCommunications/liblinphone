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

#include <ctime>

#include "address/address.h"
#include "belr/grammarbuilder.h"
#include "chat/ics/ics.h"
#include "chat/ics/parser/ics-parser.h"
#include "conference/conference-info.h"
#include "content/content-type.h"
#include "content/content.h"
#include "core/core.h"
#include "linphone/api/c-api.h"
// TODO: Remove me later.
#include "private.h"

#include "liblinphone_tester.h"
#include "tester_utils.h"

// =============================================================================

using namespace std;

using namespace LinphonePrivate;

static void parse_minimal_ics () {
	const string str = "BEGIN:VCALENDAR\r\n"
		"METHOD:REQUEST\r\n"
		"PRODID:-//Linphone//Conference calendar//EN\r\n"
		"VERSION:2.0\r\n"
		"BEGIN:VEVENT\r\n"
		"DTSTART:00000100T000000Z\r\n"
		"DTSTAMP:19700101T000000Z\r\n"
		"UID:19700101T000000Z@sip.linphone.org\r\n"
		"END:VEVENT\r\n"
		"END:VCALENDAR\r\n";

	shared_ptr<Ics::Icalendar> ics = Ics::Parser::getInstance()->parseIcs(str);
	if (!BC_ASSERT_PTR_NOT_NULL(ics)) return;

	ics->setCreationTime(0);
	const string str2 = ics->asString();
	BC_ASSERT_STRING_EQUAL(str2.c_str(), str.c_str());
}

static void parse_rfc_example () {
	const string str = "BEGIN:VCALENDAR\r\n"
		"PRODID:-//xyz Corp//NONSGML PDA Calendar Version 1.0//EN\r\n"
		"VERSION:2.0\r\n"
		"BEGIN:VEVENT\r\n"
		"DTSTAMP:19960704T120000Z\r\n"
		"UID:uid1@example.com\r\n"
		"ORGANIZER:mailto:jsmith@example.com\r\n"
		"DTSTART:19960918T143000Z\r\n"
		"DTEND:19960920T220000Z\r\n"
		"STATUS:CONFIRMED\r\n"
		"CATEGORIES:CONFERENCE\r\n"
		"SUMMARY:Networld+Interop Conference\r\n"
		"DESCRIPTION:Networld+Interop Conference and Exhibit\\nAtlanta World Congress Center\\n Atlanta\\, Georgia\r\n"
		"END:VEVENT\r\n"
		"END:VCALENDAR\r\n";

	shared_ptr<const Ics::Icalendar> ics = Ics::Icalendar::createFromString(str);
	BC_ASSERT_PTR_NOT_NULL(ics);
}

static void parse_folded_example () {
	const string str = "BEGIN:VCALENDAR\r\n"
		"PRODID:-//xyz Corp//NONSGML PDA Calendar Version 1.0//EN\r\n"
		"VERSION:2.0\r\n"
		"BEGIN:VEVENT\r\n"
		"DTSTAMP:19960704T120000Z\r\n"
		"UID:uid1@example.com\r\n"
		"ORGANIZER:mailto:jsmith@example.com\r\n"
		"DTSTART:19960918T143000Z\r\n"
		"DTEND:19960920T220000Z\r\n"
		"STATUS:CONFIRMED\r\n"
		"CATEGORIES:CONFERENCE\r\n"
		"SUMMARY:Networld+Interop Conference\r\n"
		"DESCRIPTION:Networld+Interop Conference\r\n"
		"  and Exhibit\\nAtlanta World Congress Center\\n\r\n"
		" Atlanta\\, Georgia\r\n"
		"END:VEVENT\r\n"
		"END:VCALENDAR\r\n";

	shared_ptr<const Ics::Icalendar> ics = Ics::Icalendar::createFromString(str);
	BC_ASSERT_PTR_NOT_NULL(ics);
}

static void build_ics () {
	Ics::Icalendar calendar;
	auto event = make_shared<Ics::Event>();

	event->setSummary("\n\n    Conf chat vidéo");
	event->setDescription("Parler de la vidéo conférence et répartir les tâches.\n\n\n    ");
	event->setOrganizer("sip:marie@sip.linphone.org");
	event->addAttendee("sip:pauline@sip.linphone.org");
	event->addAttendee("sip:laure@sip.linphone.org");
	event->setXConfUri("sip:videoconf1@sip.linphone.org");

	tm date = {0};
	date.tm_year = 2021 - 1900;
	date.tm_mon = 7;
	date.tm_mday = 22;
	date.tm_hour = 10;
	date.tm_min = 30;
	event->setDateTimeStart(date);

	tm duration = {0};
	duration.tm_hour = 2;
	duration.tm_min = 45;
	event->setDuration(duration);

	calendar.addEvent(event);
	calendar.setCreationTime(0);

	const string strIcs = calendar.asString();
	const string expectedIcs= "BEGIN:VCALENDAR\r\n"
		"METHOD:REQUEST\r\n"
		"PRODID:-//Linphone//Conference calendar//EN\r\n"
		"VERSION:2.0\r\n"
		"BEGIN:VEVENT\r\n"
		"DTSTART:20210822T103000Z\r\n"
		"DURATION:PT2H45M\r\n"
		"ORGANIZER:sip:marie@sip.linphone.org\r\n"
		"ATTENDEE:sip:pauline@sip.linphone.org\r\n"
		"ATTENDEE:sip:laure@sip.linphone.org\r\n"
		"X-CONFURI:sip:videoconf1@sip.linphone.org\r\n"
		"SUMMARY:Conf chat vidéo\r\n"
		"DESCRIPTION:Parler de la vidéo conférence et répartir les tâches.\r\n"
		"DTSTAMP:19700101T000000Z\r\n"
		"UID:19700101T000000Z@sip.linphone.org\r\n"
		"END:VEVENT\r\n"
		"END:VCALENDAR\r\n";

	BC_ASSERT_STRING_EQUAL(strIcs.c_str(), expectedIcs.c_str());

	auto confInfo = calendar.toConferenceInfo();

	BC_ASSERT_TRUE(confInfo->getOrganizer().isValid());
	BC_ASSERT_TRUE(confInfo->getUri().isValid());
	BC_ASSERT_EQUAL(confInfo->getParticipants().size(), 2, size_t, "%zu");
	BC_ASSERT_EQUAL(confInfo->getDuration(), 165, int, "%d");

	const string confStr = confInfo->toIcsString();
	BC_ASSERT_STRING_EQUAL(confStr.c_str(), expectedIcs.c_str());
}

static void conference_scheduler_invitations_sent(LinphoneConferenceScheduler *scheduler, const bctbx_list_t *failed_addresses) {
	stats *stat = get_stats(linphone_scheduler_get_core(scheduler));
	stat->number_of_ConferenceSchedulerInvitationsSent++;
	BC_ASSERT_PTR_NULL(failed_addresses);
}

static void conference_scheduler_invitations_sent_with_error(LinphoneConferenceScheduler *scheduler, const bctbx_list_t *failed_addresses) {
	stats *stat = get_stats(linphone_scheduler_get_core(scheduler));
	stat->number_of_ConferenceSchedulerInvitationsSent++;
	BC_ASSERT_PTR_NOT_NULL(failed_addresses);
	if (failed_addresses) {
		int failed_addresses_size = (int)(bctbx_list_size(failed_addresses));
		BC_ASSERT_EQUAL(failed_addresses_size, 1, int, "%d");
		LinphoneAddress *failed = (LinphoneAddress *)bctbx_list_get_data(failed_addresses);
		BC_ASSERT_STRING_EQUAL(linphone_address_as_string(failed), "sip:error404@sip.example.org");
	}
}

void setup_conference_info_cbs(LinphoneCoreManager * mgr) {
	// Needed to send the ICS
	linphone_core_set_file_transfer_server(mgr->lc, file_transfer_url);
}

static void send_conference_invitations(bool_t enable_group, bool_t enable_encryption, const char *subject, int curveId, bool_t add_participant_in_error) {
	bctbx_list_t *coresManagerList = NULL;
	LinphoneCoreManager* marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_create( "pauline_lime_x3dh_rc");
	LinphoneCoreManager* laure = linphone_core_manager_create("laure_lime_x3dh_rc");
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);

	set_lime_curve_list(curveId, coresManagerList);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	setup_conference_info_cbs(marie);

	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Wait for lime users to be created on x3dh server
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_X3dhUserCreationSuccess, initialMarieStats.number_of_X3dhUserCreationSuccess+1, 15000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_X3dhUserCreationSuccess, initialPaulineStats.number_of_X3dhUserCreationSuccess+1, 15000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_X3dhUserCreationSuccess, initialLaureStats.number_of_X3dhUserCreationSuccess+1, 15000));

	// Check encryption status for all participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure->lc));

	time_t conf_time = ms_time(NULL) - (time_t)3600;
	LinphoneConferenceInfo *conf_info = linphone_conference_info_new();
	linphone_conference_info_set_organizer(conf_info, marie->identity);
	linphone_conference_info_add_participant(conf_info, pauline->identity);
	linphone_conference_info_add_participant(conf_info, laure->identity);
	if (add_participant_in_error) {
		LinphoneAddress *error_participant = linphone_address_new("sip:error404@sip.example.org");
		linphone_conference_info_add_participant(conf_info, error_participant);
		linphone_address_unref(error_participant);
	}
	linphone_conference_info_set_duration(conf_info, 120);
	linphone_conference_info_set_date_time(conf_info, conf_time);
	linphone_conference_info_set_subject(conf_info, "Video conference presentation");
	linphone_conference_info_set_description(conf_info, "Talk about video conference and divide tasks.");

	LinphoneAddress *conf_uri = linphone_address_new("sip:confvideo@sip.linphone.org");
	linphone_conference_info_set_uri(conf_info, conf_uri);

	LinphoneConferenceScheduler *conference_scheduler = linphone_core_create_conference_scheduler(marie->lc);
	LinphoneConferenceSchedulerCbs *cbs = linphone_factory_create_conference_scheduler_cbs(linphone_factory_get());
	if (add_participant_in_error) {
		linphone_conference_scheduler_cbs_set_invitations_sent(cbs, conference_scheduler_invitations_sent_with_error);
	} else {
		linphone_conference_scheduler_cbs_set_invitations_sent(cbs, conference_scheduler_invitations_sent);
	}
	linphone_conference_scheduler_add_callbacks(conference_scheduler, cbs);
	linphone_conference_scheduler_cbs_unref(cbs);

	linphone_conference_scheduler_set_info(conference_scheduler, conf_info);
	LinphoneChatRoomParams *chat_room_params = linphone_core_create_default_chat_room_params(marie->lc);
	if (enable_group || enable_encryption) {
		linphone_chat_room_params_set_backend(chat_room_params, LinphoneChatRoomBackendFlexisipChat);
		linphone_chat_room_params_set_subject(chat_room_params, subject);
		linphone_chat_room_params_enable_group(chat_room_params, enable_group);
		linphone_chat_room_params_enable_encryption(chat_room_params, enable_encryption);
	}
	linphone_conference_scheduler_send_invitations(conference_scheduler, chat_room_params);
	linphone_chat_room_params_unref(chat_room_params);

	if (enable_encryption || enable_group) {
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneConferenceStateCreated, 1, 30000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomConferenceJoined, 1, 30000));
	}
	
	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_ConferenceSchedulerInvitationsSent, 1, 30000));
	linphone_conference_info_unref(conf_info);
	linphone_conference_scheduler_unref(conference_scheduler);

	bctbx_list_t *participants = bctbx_list_append(NULL, laure->identity);
	LinphoneChatRoom *marieCr = linphone_core_search_chat_room(marie->lc, NULL, marie->identity, NULL, participants);
	BC_ASSERT_PTR_NOT_NULL(marieCr);
	bctbx_list_free(participants);
	
	if (marieCr) {
		LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(marieCr);

		const bctbx_list_t* original_contents = linphone_chat_message_get_contents(msg);
		BC_ASSERT_EQUAL((int)bctbx_list_size(original_contents), 1, int, "%d");
		LinphoneContent *original_content = (LinphoneContent *) bctbx_list_get_data(original_contents);

		LinphoneConferenceInfo *conf_info_from_original_content = linphone_factory_create_conference_info_from_icalendar_content(linphone_factory_get(), original_content);
		if (BC_ASSERT_PTR_NOT_NULL(conf_info_from_original_content)) {
			BC_ASSERT_TRUE(linphone_address_weak_equal(marie->identity, linphone_conference_info_get_organizer(conf_info_from_original_content)));
			BC_ASSERT_TRUE(linphone_address_weak_equal(conf_uri, linphone_conference_info_get_uri(conf_info_from_original_content)));
			const bctbx_list_t * participants = linphone_conference_info_get_participants(conf_info_from_original_content);
			if (add_participant_in_error) {
				BC_ASSERT_EQUAL(bctbx_list_size(participants), 3, size_t, "%zu");
			} else {
				BC_ASSERT_EQUAL(bctbx_list_size(participants), 2, size_t, "%zu");
			}
			bctbx_list_free((bctbx_list_t *)participants);
			BC_ASSERT_EQUAL(linphone_conference_info_get_duration(conf_info_from_original_content), 120, int, "%d");
			BC_ASSERT_TRUE(linphone_conference_info_get_date_time(conf_info_from_original_content) == conf_time);
			linphone_conference_info_unref(conf_info_from_original_content);
		}
		linphone_chat_message_unref(msg);

		if (!enable_group) {
			linphone_chat_room_unref(marieCr);
		}
	}

	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneMessageReceived,1));
	BC_ASSERT_TRUE(wait_for(laure->lc,marie->lc,&laure->stat.number_of_LinphoneMessageReceived,1));

	BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneMessageReceivedWithFile,1, 60000));
	BC_ASSERT_TRUE(wait_for_until(laure->lc,marie->lc,&laure->stat.number_of_LinphoneMessageReceivedWithFile,1, 60000));

	BC_ASSERT_PTR_NOT_NULL(pauline->stat.last_received_chat_message);
	if (pauline->stat.last_received_chat_message != NULL) {
		const string expected = ContentType::Icalendar.getMediaType();
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(pauline->stat.last_received_chat_message), expected.c_str());
		LinphoneChatRoom *paulineCr = linphone_chat_message_get_chat_room(pauline->stat.last_received_chat_message);
		linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	}

	BC_ASSERT_PTR_NOT_NULL(laure->stat.last_received_chat_message);
	if (laure->stat.last_received_chat_message != NULL) {
		const string expected = ContentType::Icalendar.getMediaType();
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(laure->stat.last_received_chat_message), expected.c_str());

		const bctbx_list_t* contents = linphone_chat_message_get_contents(laure->stat.last_received_chat_message);
		BC_ASSERT_EQUAL((int)bctbx_list_size(contents), 1, int, "%d");
		LinphoneContent *content = (LinphoneContent *) bctbx_list_get_data(contents);

		LinphoneConferenceInfo *conf_info_from_content = linphone_factory_create_conference_info_from_icalendar_content(linphone_factory_get(), content);
		if (BC_ASSERT_PTR_NOT_NULL(conf_info_from_content)) {
			BC_ASSERT_TRUE(linphone_address_weak_equal(marie->identity, linphone_conference_info_get_organizer(conf_info_from_content)));
			BC_ASSERT_TRUE(linphone_address_weak_equal(conf_uri, linphone_conference_info_get_uri(conf_info_from_content)));
			const bctbx_list_t * participants = linphone_conference_info_get_participants(conf_info_from_content);
			if (add_participant_in_error) {
				BC_ASSERT_EQUAL(bctbx_list_size(participants), 3, size_t, "%zu");
			} else {
				BC_ASSERT_EQUAL(bctbx_list_size(participants), 2, size_t, "%zu");
			}
			bctbx_list_free((bctbx_list_t *)participants);
			BC_ASSERT_EQUAL(linphone_conference_info_get_duration(conf_info_from_content), 120, int, "%d");
			BC_ASSERT_TRUE(linphone_conference_info_get_date_time(conf_info_from_content) == conf_time);
			linphone_conference_info_unref(conf_info_from_content);
		}

		// Check that the core can retrieve the conference info list on DB
		bctbx_list_t *conf_info_list = linphone_core_get_conference_information_list(laure->lc);
		BC_ASSERT_EQUAL((int)bctbx_list_size(conf_info_list), 1, int, "%d");
		bctbx_list_free_with_data(conf_info_list, (bctbx_list_free_func)linphone_conference_info_unref);

		// Check that specifying the future parameter to TRUE returns no conference info
		bctbx_list_t *conf_info_list_future = linphone_core_get_future_conference_information_list(laure->lc);
		BC_ASSERT_PTR_NULL(conf_info_list_future);

		LinphoneChatRoom *laureCr = linphone_chat_message_get_chat_room(laure->stat.last_received_chat_message);
		linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	}

	linphone_address_unref(conf_uri);
	if (enable_group && marieCr) {
		linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	}

	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void send_conference_invitations_1(void) {
	send_conference_invitations(FALSE, FALSE, NULL, 0, FALSE);
}

static void send_conference_invitations_2(void) {
	send_conference_invitations(FALSE, TRUE, "dummy subject", 25519, FALSE);
	//send_conference_invitations(FALSE, TRUE, "dummy subject", 448, FALSE);
}

static void send_conference_invitations_3(void) {
	send_conference_invitations(TRUE, TRUE, "conference group!", 25519, FALSE);
	//send_conference_invitations(TRUE, TRUE, "conference group!", 448, FALSE);
}

static void send_conference_invitations_error_1(void) {
	send_conference_invitations(FALSE, FALSE, NULL, 0, TRUE);
}

static void send_conference_invitations_error_2(void) {
	send_conference_invitations(FALSE, TRUE, "dummy subject", 25519, TRUE);
	//send_conference_invitations(FALSE, TRUE, "dummy subject", 448, TRUE);
}

static void send_conference_invitations_error_3(void) {
	send_conference_invitations(TRUE, TRUE, "conference group!", 25519, TRUE);
	//send_conference_invitations(TRUE, TRUE, "conference group!", 448, TRUE);
}

test_t ics_tests[] = {
	TEST_NO_TAG("Parse minimal Ics", parse_minimal_ics),
	TEST_NO_TAG("Parse RFC example", parse_rfc_example),
	TEST_NO_TAG("Parse folded example", parse_folded_example),
	TEST_NO_TAG("Build Ics", build_ics),
	TEST_NO_TAG("Send conference invitations in basic chat room", send_conference_invitations_1),
	TEST_NO_TAG("Send conference invitations in one-to-one encrypted chat room", send_conference_invitations_2),
	TEST_NO_TAG("Send conference invitations in a group encrypted chat room", send_conference_invitations_3),
	TEST_NO_TAG("Send conference invitations error in basic chat room", send_conference_invitations_error_1),
	TEST_NO_TAG("Send conference invitations error in one-to-one encrypted chat room", send_conference_invitations_error_2),
	TEST_NO_TAG("Send conference invitations error in a group encrypted chat room", send_conference_invitations_error_3),
};

static int suite_begin(void) {
	//Supposed to be done by platform helper, but in this case, we don't have it"
	belr::GrammarLoader::get().addPath(std::string(bc_tester_get_resource_dir_prefix()).append("/share/belr/grammars"));
	return 0;
}
test_suite_t ics_test_suite = {
	"Ics", suite_begin, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
	sizeof(ics_tests) / sizeof(ics_tests[0]), ics_tests
};
