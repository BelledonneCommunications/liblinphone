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
		"PRODID:-//Linphone//Conference calendar//EN\r\n"
		"VERSION:2.0\r\n"
		"BEGIN:VEVENT\r\n"
		"DTSTART:00000100T000000Z\r\n"
		"END:VEVENT\r\n"
		"END:VCALENDAR\r\n";

	shared_ptr<const Ics::Icalendar> ics = Ics::Icalendar::createFromString(str);
	if (!BC_ASSERT_PTR_NOT_NULL(ics)) return;

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

static void build_ics () {
	Ics::Icalendar calendar;
	auto event = make_shared<Ics::Event>();

	event->setSummary("Conf chat vidéo");
	event->setDescription("Parler de la vidéo conférence et répartir les tâches.");
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

	const string strIcs = calendar.asString();
	const string expectedIcs= "BEGIN:VCALENDAR\r\n"
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
		"END:VEVENT\r\n"
		"END:VCALENDAR\r\n";

	BC_ASSERT_STRING_EQUAL(strIcs.c_str(), expectedIcs.c_str());

	auto confInfo = calendar.toConferenceInfo();

	BC_ASSERT_PTR_NOT_NULL(confInfo->getOrganizer());
	BC_ASSERT_PTR_NOT_NULL(confInfo->getUri());
	BC_ASSERT_PTR_NOT_NULL(confInfo->getParticipants());
	BC_ASSERT_EQUAL((int)bctbx_list_size(confInfo->getParticipants()), 2, int, "%d");
	BC_ASSERT_EQUAL(confInfo->getDuration(), 165, int, "%d");

	const string confStr = confInfo->toIcsString();
	BC_ASSERT_STRING_EQUAL(confStr.c_str(), expectedIcs.c_str());
}

static void conference_info_participant_sent(LinphoneCore *core, LinphoneAddress *address) {
	stats *stat = get_stats(core);
	stat->number_of_LinphoneConferenceInfoOnParticipantSent++;
}

static void conference_info_participant_error(LinphoneCore *core, LinphoneAddress *address, LinphoneConferenceInfoError error) {
	stats *stat = get_stats(core);
	stat->number_of_LinphoneConferenceInfoOnParticipantError++;
}

static void send_conference_invitations(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new( "pauline_tcp_rc");
	LinphoneCoreManager* laure = linphone_core_manager_new("laure_tcp_rc");

	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_conference_info_on_participant_sent(cbs, conference_info_participant_sent);
	linphone_core_cbs_set_conference_info_on_participant_error(cbs, conference_info_participant_error);
	linphone_core_add_callbacks(marie->lc, cbs);
	linphone_core_cbs_unref(cbs);

	linphone_core_set_file_transfer_server(marie->lc, file_transfer_url);

	time_t conf_time = ms_time(NULL);
	LinphoneConferenceInfo *conf_info = linphone_conference_info_new();
	linphone_conference_info_set_organizer(conf_info, marie->identity);
	linphone_conference_info_add_participant(conf_info, pauline->identity);
	linphone_conference_info_add_participant(conf_info, laure->identity);
	linphone_conference_info_set_duration(conf_info, 120);
	linphone_conference_info_set_date_time(conf_info, conf_time);
	linphone_conference_info_set_subject(conf_info, "Video conference presentation");
	linphone_conference_info_set_description(conf_info, "Talk about video conference and divide tasks.");

	LinphoneAddress *conf_uri = linphone_address_new("sip:confvideo@sip.linphone.org");
	linphone_conference_info_set_uri(conf_info, conf_uri);
	linphone_address_unref(conf_uri);

	linphone_core_send_conference_information(marie->lc, conf_info, NULL);

	linphone_conference_info_unref(conf_info);

	BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneMessageReceived,1));
	BC_ASSERT_TRUE(wait_for(laure->lc,marie->lc,&laure->stat.number_of_LinphoneMessageReceived,1));

	BC_ASSERT_TRUE(wait_for_until(pauline->lc,marie->lc,&pauline->stat.number_of_LinphoneMessageReceivedWithFile,1, 60000));
	BC_ASSERT_TRUE(wait_for_until(laure->lc,marie->lc,&laure->stat.number_of_LinphoneMessageReceivedWithFile,1, 60000));

	BC_ASSERT_PTR_NOT_NULL(pauline->stat.last_received_chat_message);
	if (pauline->stat.last_received_chat_message != NULL) {
		const string expected = ContentType::Icalendar.getMediaType();
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(pauline->stat.last_received_chat_message), expected.c_str());
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
			BC_ASSERT_EQUAL((int)bctbx_list_size(linphone_conference_info_get_participants(conf_info_from_content)), 2, int, "%d");
			BC_ASSERT_EQUAL(linphone_conference_info_get_duration(conf_info_from_content), 120, int, "%d");
			BC_ASSERT_TRUE(linphone_conference_info_get_date_time(conf_info_from_content) == conf_time);
			linphone_conference_info_unref(conf_info_from_content);
		}
	}

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceInfoOnParticipantSent, 2, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneConferenceInfoOnParticipantError, 0, int, "%d");

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

test_t ics_tests[] = {
	TEST_NO_TAG("Parse minimal Ics", parse_minimal_ics),
	TEST_NO_TAG("Parse RFC example", parse_rfc_example),
	TEST_NO_TAG("Build Ics", build_ics),
	TEST_NO_TAG("Send conference invitations", send_conference_invitations),
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
