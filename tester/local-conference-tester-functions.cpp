/*
 * copyright (c) 2010-2022 belledonne communications sarl.
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

#include "chat/encryption/encryption-engine.h"
#include "conference/client-conference.h"
#include "conference/conference.h"
#include "conference/encryption/client-ekt-manager.h"
#include "conference/participant.h"
#include "linphone/api/c-call-log.h"
#include "linphone/api/c-chat-room-params.h"
#include "linphone/api/c-conference-info.h"
#include "linphone/api/c-conference-params.h"
#include "linphone/api/c-conference-scheduler.h"
#include "linphone/api/c-conference.h"
#include "linphone/api/c-content.h"
#include "linphone/api/c-participant-device.h"
#include "linphone/api/c-participant-info.h"
#include "linphone/api/c-participant.h"
#include "linphone/chat.h"

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

void setup_conference_info_cbs(LinphoneCoreManager *mgr) {
	// Needed to send the ICS
	linphone_core_set_file_transfer_server(mgr->lc, file_transfer_url);
}

namespace LinphoneTest {

std::map<LinphoneCoreManager *, LinphoneParticipantInfo *>
fill_member_list(std::list<LinphoneCoreManager *> members,
                 std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList,
                 LinphoneCoreManager *organizer,
                 bctbx_list_t *participants_info) {
	std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList;
	for (const auto &member : members) {
		try {
			auto &participantInfo = participantList.at(member);
			LinphoneParticipantRole role = linphone_participant_info_get_role(participantInfo);
			if (role == LinphoneParticipantRoleUnknown) {
				role = LinphoneParticipantRoleSpeaker;
				linphone_participant_info_set_role(participantInfo, role);
			}
			memberList.insert(std::make_pair(member, participantInfo));
		} catch (std::out_of_range &) {
			if (member == organizer) {
				memberList.insert(
				    std::make_pair(member, add_participant_info_to_list(&participants_info, member->identity,
				                                                        LinphoneParticipantRoleSpeaker, 0)));
			} else {
				ms_fatal("Unable to find active participant %s in the participant list",
				         linphone_core_get_identity(member->lc));
			}
		}
	}
	return memberList;
}

void update_sequence_number(bctbx_list_t **participants_info,
                            const std::list<LinphoneAddress *> new_participants,
                            int exp_sequence,
                            int exp_new_participant_sequence) {
	// Increment sequence number of participants
	for (const bctbx_list_t *it = *participants_info; it; it = bctbx_list_next(it)) {
		LinphoneParticipantInfo *participant_info = (LinphoneParticipantInfo *)bctbx_list_get_data(it);
		bool is_new_participant = false;
		for (const auto &address : new_participants) {
			is_new_participant |=
			    !!linphone_address_weak_equal(linphone_participant_info_get_address(participant_info), address);
		}
		int sequence_number = 0;
		if (is_new_participant) {
			sequence_number = exp_new_participant_sequence;
		} else {
			sequence_number = exp_sequence;
		}
		linphone_participant_info_set_sequence_number(participant_info, sequence_number);
	}
}

static bool have_common_audio_payload(LinphoneCoreManager *mgr1, LinphoneCoreManager *mgr2) {
	bool found = false;
	bctbx_list_t *codecs = linphone_core_get_audio_payload_types(mgr1->lc);
	for (bctbx_list_t *elem = codecs; elem != NULL; elem = elem->next) {
		LinphonePayloadType *pt1 = (LinphonePayloadType *)elem->data;
		if (linphone_payload_type_enabled(pt1) == TRUE) {
			LinphonePayloadType *pt2 = linphone_core_get_payload_type(
			    mgr2->lc, linphone_payload_type_get_mime_type(pt1), linphone_payload_type_get_clock_rate(pt1),
			    linphone_payload_type_get_channels(pt1));
			if (pt2 && linphone_payload_type_enabled(pt2)) {
				found = true;
			}
			linphone_payload_type_unref(pt2);
		}
	}
	bctbx_list_free_with_data(codecs, (bctbx_list_free_func)linphone_payload_type_unref);
	return found;
}

static bool are_participants_camera_streams_requested(LinphoneCoreManager *mgr, LinphoneConference *conference) {
	int max_thumbnails = linphone_core_get_conference_max_thumbnails(mgr->lc);
	int nb_thumbnails = 0;
	bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
	for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
		LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
		const LinphoneAddress *device_address = linphone_participant_device_get_address(d);
		bool_t is_me = linphone_conference_is_me(conference, device_address);
		if (!is_me) {
			LinphoneMediaDirection thumbnail_dir = linphone_participant_device_get_thumbnail_stream_capability(d);
			if ((thumbnail_dir == LinphoneMediaDirectionSendOnly) ||
			    (thumbnail_dir == LinphoneMediaDirectionSendRecv)) {
				nb_thumbnails++;
			}
		}
	}
	if (devices) {
		bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
	}

	return (nb_thumbnails <= max_thumbnails);
}

static bool check_conference_info_by_participant(LinphoneCoreManager *mgr,
                                                 std::list<LinphoneCoreManager *> members,
                                                 size_t expected_infos,
                                                 LinphoneAddress *conference_address) {
	bool_t found_in_all_participants = TRUE;
	for (const auto &member : members) {
		bool_t found = FALSE;
		bctbx_list_t *infos = linphone_core_get_conference_informations_with_participant(mgr->lc, member->identity);
		size_t nb_infos = bctbx_list_size(infos);
		if (expected_infos <= 0) {
			BC_ASSERT_GREATER_STRICT(nb_infos, 0, size_t, "%zu");
		} else {
			BC_ASSERT_EQUAL(nb_infos, expected_infos, size_t, "%zu");
		}
		for (bctbx_list_t *info_it = infos; info_it; info_it = bctbx_list_next(info_it)) {
			LinphoneConferenceInfo *info = (LinphoneConferenceInfo *)bctbx_list_get_data(info_it);
			if (linphone_address_equal(linphone_conference_info_get_uri(info), conference_address)) {
				check_conference_info_against_db(mgr, conference_address, info, FALSE);
				found = TRUE;
			}
		}
		bctbx_list_free_with_data(infos, (bctbx_list_free_func)linphone_conference_info_unref);
		found_in_all_participants &= found;
	}
	return found_in_all_participants;
}

void check_conference_me(LinphoneConference *conference, bool_t is_admin) {
	LinphoneParticipant *me = linphone_conference_get_me(conference);
	BC_ASSERT_TRUE(linphone_participant_is_admin(me) == is_admin);
	LinphoneAccount *account = linphone_conference_get_account(conference);
	BC_ASSERT_PTR_NOT_NULL(account);
	if (account) {
		const LinphoneAccountParams *account_params = linphone_account_get_params(account);
		const LinphoneAddress *account_identity_address = linphone_account_params_get_identity_address(account_params);
		BC_ASSERT_PTR_NOT_NULL(account_identity_address);
		BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_participant_get_address(me), account_identity_address));
	}
}

void check_delete_focus_conference_info(std::initializer_list<std::reference_wrapper<CoreManager>> coreMgrs,
                                        std::list<LinphoneCoreManager *> conferenceMgrs,
                                        LinphoneCoreManager *focus,
                                        LinphoneAddress *confAddr,
                                        time_t end_time) {
	if (end_time > 0) {
		long focus_cleanup_window = linphone_core_get_conference_cleanup_period(focus->lc);
		if (focus_cleanup_window > 0) {
			// The conference information is only deleted by the cleanup timer. Hence even if the end time went by, the
			// conference information might not be deleted
			CoreManagerAssert(coreMgrs).waitUntil(chrono::seconds(focus_cleanup_window), [] { return false; });
		}

		// Compute here how much time is left to the conference to automatically account for the cleanup window delay
		time_t now = ms_time(NULL);
		time_t time_left = end_time - now;
		char *conferenceAddressString = linphone_address_as_string(confAddr);
		ms_message(
		    "Checking conference information in member's database for conference %s that has %0ld seconds left (a "
		    "negative time means that the conference cannot be joined anymore). Expected end time %s and now it is %s",
		    conferenceAddressString, time_left, Utils::timeToIso8601(end_time).c_str(),
		    Utils::timeToIso8601(now).c_str());
		for (const auto &mgr : conferenceMgrs) {
			LinphoneConferenceInfo *info = linphone_core_find_conference_information_from_uri(mgr->lc, confAddr);
			ms_message("Conference information in %s's database for conference %s is %p",
			           linphone_core_get_identity(mgr->lc), conferenceAddressString, info);
			if ((mgr == focus) && (time_left <= 0) && (focus_cleanup_window > 0)) {
				BC_ASSERT_PTR_NULL(info);
			} else {
				BC_ASSERT_PTR_NOT_NULL(info);
			}
			if (info) {
				linphone_conference_info_unref(info);
			}
		}

		if (time_left > 0) {
			time_t wait_time = time_left;
			if (focus_cleanup_window > 0) {
				wait_time += focus_cleanup_window;
			}
			ms_message("Waiting for %0ld seconds for the server to delete conference %s. Expected end time %s and now "
			           "it is %s",
			           wait_time, conferenceAddressString, Utils::timeToIso8601(end_time).c_str(),
			           Utils::timeToIso8601(now).c_str());
			// wait for the conference to end
			CoreManagerAssert(coreMgrs).waitUntil(chrono::seconds((wait_time + 1)), [] { return false; });
			LinphoneConferenceInfo *focus_info =
			    linphone_core_find_conference_information_from_uri(focus->lc, confAddr);
			if (focus_cleanup_window > 0) {
				BC_ASSERT_PTR_NULL(focus_info);
			} else {
				if (BC_ASSERT_PTR_NOT_NULL(focus_info)) {
					linphone_conference_info_unref(focus_info);
				}
			}
		}
		ms_free(conferenceAddressString);
	}
}

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
                            bool_t enable_chat,
                            LinphoneConferenceParams *ics_chat_room_params) {
	bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
	coresList = bctbx_list_append(coresList, organizer.getLc());
	std::vector<stats> participant_stats;
	std::map<LinphoneCoreManager *, LinphoneCall *> previous_calls;
	bctbx_list_t *participants_info = NULL;
	std::list<LinphoneCoreManager *> participants;
	const bctbx_list_t *organizer_call_logs = NULL;
	const bctbx_list_t *initial_organizer_call_logs = linphone_core_get_call_logs(organizer.getLc());
	const bctbx_list_t *focus_call_logs = NULL;
	const bctbx_list_t *initial_focus_call_logs = linphone_core_get_call_logs(focus.getLc());
	const LinphoneConferenceInfo *updated_conf_info = NULL;
	bool focus_organizer_common_payload = have_common_audio_payload(organizer.getCMgr(), focus.getCMgr());
	bool is_dialout = (start_time < 0);
	// Dialout conferences do not instantiate un conference scheduler therefore they cannot send an ICS
	bool will_send_ics = (is_dialout) ? false : !!send_ics;
	bool found_me = false;
	char *conference_address_str = NULL;
	int duration = 0;
	char *uid = NULL;
	LinphoneConferenceInfo *info = NULL;
	LinphoneConferenceScheduler *conference_scheduler = NULL;
	for (auto &[mgr, participant_info] : requested_participants) {
		if (is_dialout) {
			linphone_participant_info_set_role(participant_info, LinphoneParticipantRoleSpeaker);
		}
		LinphoneParticipantInfo *participant_info_clone = linphone_participant_info_clone(participant_info);
		participants_info = bctbx_list_append(participants_info, participant_info_clone);
		if (mgr == organizer.getCMgr()) {
			found_me = true;
		} else {
			coresList = bctbx_list_append(coresList, mgr->lc);
			participant_stats.push_back(mgr->stat);
			participants.push_back(mgr);
		}
	}

	stats organizer_stat = organizer.getStats();
	int idx = 0;
	std::list<LinphoneCoreManager *> actual_participants;
	int call_errors_cnt = 0;
	LinphoneAddress *conference_address = NULL;
	LinphoneAccount *default_account = linphone_core_get_default_account(organizer.getLc());
	LinphoneAddress *organizer_address = default_account
	                                         ? linphone_address_clone(linphone_account_params_get_identity_address(
	                                               linphone_account_get_params(default_account)))
	                                         : linphone_address_clone(organizer.getCMgr()->identity);

	if (is_dialout) {
		stats focus_stat = focus.getStats();
		// marie creates the conference
		LinphoneConferenceParams *conference_params = linphone_core_create_conference_params_2(organizer.getLc(), NULL);
		linphone_conference_params_set_account(conference_params, default_account);
		linphone_conference_params_enable_audio(conference_params, TRUE);
		linphone_conference_params_enable_video(conference_params, enable_video);
		linphone_conference_params_enable_chat(conference_params, enable_chat);
		linphone_conference_params_set_security_level(conference_params, security_level);
		linphone_conference_params_set_subject(conference_params, subject);
		LinphoneConference *conference =
		    linphone_core_create_conference_with_params(organizer.getLc(), conference_params);
		linphone_conference_params_unref(conference_params);
		BC_ASSERT_PTR_NOT_NULL(conference);
		if (conference) {
			bctbx_list_t *participant_addresses = NULL;
			for (const auto &mgr : participants) {
				participant_addresses = bctbx_list_append(participant_addresses, mgr->identity);
			}
			LinphoneCallParams *call_params = linphone_core_create_call_params(organizer.getLc(), NULL);
			linphone_call_params_enable_audio(call_params, TRUE);
			const LinphoneVideoActivationPolicy *video_policy =
			    linphone_core_get_video_activation_policy(organizer.getLc());
			linphone_call_params_enable_video(
			    call_params, linphone_video_activation_policy_get_automatically_initiate(video_policy));
			linphone_call_params_enable_realtime_text(call_params, FALSE);
			linphone_conference_invite_participants(conference, participant_addresses, call_params);
			bctbx_list_free(participant_addresses);
			linphone_call_params_unref(call_params);
			if (focus_organizer_common_payload) {
				BC_ASSERT_TRUE(wait_for_list(coresList,
				                             &organizer.getStats().number_of_LinphoneConferenceStateCreationPending,
				                             organizer_stat.number_of_LinphoneConferenceStateCreationPending + 1,
				                             liblinphone_tester_sip_timeout));
				conference_address = linphone_address_clone(linphone_conference_get_conference_address(conference));
			} else {
				BC_ASSERT_TRUE(wait_for_list(coresList,
				                             &organizer.getStats().number_of_LinphoneConferenceStateCreationFailed,
				                             organizer_stat.number_of_LinphoneConferenceStateCreationFailed + 1,
				                             liblinphone_tester_sip_timeout));
			}
			linphone_conference_unref(conference);
		}

		if (focus_organizer_common_payload) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &organizer.getStats().number_of_LinphoneCallOutgoingInit,
			                             organizer_stat.number_of_LinphoneCallOutgoingInit + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
			                             focus_stat.number_of_LinphoneCallIncomingReceived + 1,
			                             liblinphone_tester_sip_timeout));
		} else {
			linphone_conference_terminate(conference);
			goto end;
		}
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingInit,
		                  focus_stat.number_of_LinphoneCallOutgoingInit + static_cast<int>(participants.size()),
		                  liblinphone_tester_sip_timeout));

		int call_ok_cnt = 0;
		// Conference server dials out participants
		for (const auto &mgr : participants) {
			auto old_stats = participant_stats[idx];
			if (have_common_audio_payload(mgr, focus.getCMgr()) && !previous_calls[mgr]) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallIncomingReceived,
				                             old_stats.number_of_LinphoneCallIncomingReceived + 1,
				                             liblinphone_tester_sip_timeout));
				call_ok_cnt++;
				actual_participants.push_back(mgr);
			} else {
				call_errors_cnt++;
			}
			idx++;
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingProgress,
		                             focus_stat.number_of_LinphoneCallOutgoingProgress + call_ok_cnt,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingRinging,
		                             focus_stat.number_of_LinphoneCallOutgoingRinging + call_ok_cnt,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_alerting,
		                             focus_stat.number_of_conference_participant_devices_alerting + call_ok_cnt,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallError,
		                             focus_stat.number_of_LinphoneCallError + call_errors_cnt,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed + call_errors_cnt,
		                             liblinphone_tester_sip_timeout));

	} else {
		if ((end_time >= 0) && (start_time >= 0) && (end_time > start_time)) {
			duration = static_cast<int>((end_time - start_time) / 60); // duration is expected to be set in minutes
		}

		for (const auto &mgr : participants) {
			previous_calls[mgr] = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
		}

		// The organizer creates a conference scheduler
		conference_scheduler = linphone_core_create_sip_conference_scheduler(organizer.getLc(), default_account);
		LinphoneConferenceSchedulerCbs *cbs = linphone_factory_create_conference_scheduler_cbs(linphone_factory_get());
		linphone_conference_scheduler_cbs_set_state_changed(cbs, conference_scheduler_state_changed);
		linphone_conference_scheduler_cbs_set_invitations_sent(cbs, conference_scheduler_invitations_sent);
		linphone_conference_scheduler_add_callbacks(conference_scheduler, cbs);
		linphone_conference_scheduler_cbs_unref(cbs);

		LinphoneConferenceInfo *conf_info = linphone_conference_info_new();
		linphone_conference_info_set_organizer(conf_info, organizer_address);
		linphone_conference_info_set_participant_infos(conf_info, participants_info);
		linphone_conference_info_set_duration(conf_info, duration);
		linphone_conference_info_set_date_time(conf_info, start_time);
		linphone_conference_info_set_subject(conf_info, subject);
		linphone_conference_info_set_description(conf_info, description);
		linphone_conference_info_set_security_level(conf_info, security_level);
		linphone_conference_info_set_capability(conf_info, LinphoneStreamTypeVideo, enable_video);
		linphone_conference_info_set_capability(conf_info, LinphoneStreamTypeText, enable_chat);

		linphone_conference_scheduler_set_info(conference_scheduler, conf_info);
		linphone_conference_info_unref(conf_info);

		BC_ASSERT_TRUE(wait_for_list(
		    coresList, &organizer.getStats().number_of_ConferenceSchedulerStateAllocationPending,
		    organizer_stat.number_of_ConferenceSchedulerStateAllocationPending + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &organizer.getStats().number_of_ConferenceSchedulerStateReady,
		                             organizer_stat.number_of_ConferenceSchedulerStateReady + 1,
		                             liblinphone_tester_sip_timeout));

		actual_participants = participants;

		updated_conf_info = linphone_conference_scheduler_get_info(conference_scheduler);
		conference_address = linphone_address_clone(linphone_conference_info_get_uri(updated_conf_info));

		if (will_send_ics) {
			LinphoneConferenceParams *conference_params = NULL;
			if (ics_chat_room_params) {
				conference_params = linphone_conference_params_clone(ics_chat_room_params);
			} else {

				conference_params = linphone_core_create_conference_params_2(organizer.getLc(), NULL);
				linphone_conference_params_enable_chat(conference_params, TRUE);
				linphone_conference_params_enable_group(conference_params, FALSE);
				char subject[200];
				char *conference_address_string = linphone_address_as_string(conference_address);
				snprintf(subject, sizeof(subject) - 1, "ICS for conference %s", conference_address_string);
				linphone_conference_params_set_subject(conference_params, subject);
				ms_free(conference_address_string);
				LinphoneChatParams *chat_params = linphone_conference_params_get_chat_params(conference_params);
				linphone_chat_params_set_backend(chat_params, LinphoneChatRoomBackendBasic);
			}
			linphone_conference_scheduler_send_invitations_2(conference_scheduler, conference_params);
			linphone_conference_params_unref(conference_params);
			BC_ASSERT_TRUE(wait_for_list(coresList, &organizer.getStats().number_of_ConferenceSchedulerInvitationsSent,
			                             organizer_stat.number_of_ConferenceSchedulerInvitationsSent + 1,
			                             liblinphone_tester_sip_timeout));
		}
	}

	info = linphone_core_find_conference_information_from_uri(organizer.getLc(), conference_address);
	if (BC_ASSERT_PTR_NOT_NULL(info)) {
		uid = ms_strdup(linphone_conference_info_get_ics_uid(info));
		if (will_send_ics) {
			BC_ASSERT_PTR_NOT_NULL(uid);
			for (auto &p : participants) {
				linphone_conference_info_check_participant(info, p->identity, 0);
			}
		} else {
			BC_ASSERT_PTR_NULL(uid);
		}
		linphone_conference_info_unref(info);
	}

	if (is_dialout && !will_send_ics && !found_me) {
		add_participant_info_to_list(&participants_info, organizer.getCMgr()->identity, LinphoneParticipantRoleSpeaker,
		                             0);
	}

	for (bctbx_list_t *info_it = participants_info; info_it; info_it = bctbx_list_next(info_it)) {
		LinphoneParticipantInfo *participant_info_el = (LinphoneParticipantInfo *)bctbx_list_get_data(info_it);
		if (is_dialout && !will_send_ics) {
			linphone_participant_info_set_role(participant_info_el, LinphoneParticipantRoleUnknown);
			linphone_participant_info_set_sequence_number(participant_info_el, -1);
		} else {
			linphone_participant_info_set_sequence_number(participant_info_el, 0);
			if (is_dialout && will_send_ics &&
			    (linphone_participant_info_get_role(participant_info_el) == LinphoneParticipantRoleUnknown)) {
				linphone_participant_info_set_role(participant_info_el, LinphoneParticipantRoleSpeaker);
			}
		}
	}

	if (!is_dialout) {
		// This check is not reliable when the conference is dialing participants
		check_conference_info_in_db(organizer.getCMgr(), uid, conference_address, organizer.getCMgr()->identity,
		                            participants_info, start_time, duration, subject, description, 0,
		                            LinphoneConferenceInfoStateNew, security_level, FALSE, TRUE, enable_video,
		                            enable_chat);
	}

	idx = 0;
	for (const auto &mgr : participants) {
		if (!is_dialout || !previous_calls[mgr]) {
			auto old_stats = participant_stats[idx];
			if (will_send_ics) {
				// chat room in created state
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneChatRoomStateCreated, 1,
				                             liblinphone_tester_sip_timeout));
				if (L_GET_CPP_PTR_FROM_C_OBJECT(mgr->lc)->canAggregateChatMessages()) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneAggregatedMessagesReceived,
					                             old_stats.number_of_LinphoneAggregatedMessagesReceived + 1,
					                             liblinphone_tester_sip_timeout));
				} else {
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceived,
					                             old_stats.number_of_LinphoneMessageReceived + 1,
					                             liblinphone_tester_sip_timeout));
				}
				if (!linphone_core_conference_ics_in_message_body_enabled(organizer.getLc())) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceivedWithFile,
					                             old_stats.number_of_LinphoneMessageReceivedWithFile + 1,
					                             liblinphone_tester_sip_timeout));
				}

				bctbx_list_t *chat_room_participants = bctbx_list_append(NULL, mgr->identity);
				LinphoneChatRoom *cr = linphone_core_search_chat_room(organizer.getLc(), NULL, organizer_address, NULL,
				                                                      chat_room_participants);
				bctbx_list_free(chat_room_participants);
				BC_ASSERT_PTR_NOT_NULL(cr);
				if (cr) {
					LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(cr);
					BC_ASSERT_PTR_NOT_NULL(msg);

					if (msg) {
						const string ics_media_type = ContentType::Icalendar.getMediaType();
						BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(msg), ics_media_type.c_str());

						const bctbx_list_t *original_contents = linphone_chat_message_get_contents(msg);
						BC_ASSERT_EQUAL((int)bctbx_list_size(original_contents), 1, int, "%d");
						LinphoneContent *original_content = (LinphoneContent *)bctbx_list_get_data(original_contents);
						BC_ASSERT_PTR_NOT_NULL(original_content);

						LinphoneConferenceInfo *conf_info_in_db =
						    linphone_core_find_conference_information_from_uri(mgr->lc, conference_address);
						if (!BC_ASSERT_PTR_NOT_NULL(conf_info_in_db)) {
							goto end;
						}
						check_conference_info_members(
						    conf_info_in_db, uid, conference_address, organizer_address, participants_info, start_time,
						    duration, subject, description, 0, LinphoneConferenceInfoStateNew,
						    LinphoneConferenceSecurityLevelNone, is_dialout, TRUE, TRUE, FALSE);

						LinphoneConferenceInfo *conf_info_from_original_content =
						    linphone_factory_create_conference_info_from_icalendar_content(linphone_factory_get(),
						                                                                   original_content);
						if (BC_ASSERT_PTR_NOT_NULL(conf_info_from_original_content)) {
							compare_conference_infos(conf_info_from_original_content, conf_info_in_db, FALSE);
							linphone_conference_info_unref(conf_info_from_original_content);
						}
						linphone_conference_info_unref(conf_info_in_db);
						linphone_chat_message_unref(msg);
					}
				}
			}

			if (will_send_ics || is_dialout) {
				auto itConferenceMgrs = std::find(actual_participants.begin(), actual_participants.end(), mgr);
				if (itConferenceMgrs != actual_participants.end()) {
					LinphoneConferenceInfo *conf_info_in_db =
					    linphone_core_find_conference_information_from_uri(mgr->lc, conference_address);
					if (!BC_ASSERT_PTR_NOT_NULL(conf_info_in_db)) {
						goto end;
					}
					// Encryption is None because we haven't received yet the NOTIFY full state with this information
					check_conference_info_members(
					    conf_info_in_db, uid, conference_address, organizer.getCMgr()->identity, participants_info,
					    start_time, ((start_time > 0) && (end_time > 0)) ? (int)(end_time - start_time) / 60 : 0,
					    subject, (will_send_ics) ? description : NULL, 0, LinphoneConferenceInfoStateNew,
					    LinphoneConferenceSecurityLevelNone, is_dialout, TRUE, TRUE, FALSE);
					if (will_send_ics) {
						for (auto &p : participants) {
							linphone_conference_info_check_participant(conf_info_in_db, p->identity, 0);
						}
						linphone_conference_info_check_organizer(conf_info_in_db, 0);
					}
					if (conf_info_in_db) {
						linphone_conference_info_unref(conf_info_in_db);
					}
				}
			}
		}
		idx++;
	}

	if (conference_address && !is_dialout) {
		check_conference_info_in_db(organizer.getCMgr(), uid, conference_address, organizer.getCMgr()->identity,
		                            participants_info, start_time, duration, subject, description, 0,
		                            LinphoneConferenceInfoStateNew, security_level, FALSE, TRUE, enable_video,
		                            enable_chat);
		std::list<LinphoneCoreManager *> members{organizer.getCMgr()};
		for (const auto &[mgr, participant_info] : requested_participants) {
			members.push_back(mgr);
		}
		BC_ASSERT_TRUE(check_conference_info_by_participant(organizer.getCMgr(), members, 0, conference_address));
	}

	BC_ASSERT_EQUAL(organizer.getStats().number_of_LinphoneConferenceStateTerminationPending,
	                organizer_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
	BC_ASSERT_EQUAL(organizer.getStats().number_of_LinphoneConferenceStateTerminated,
	                organizer_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
	BC_ASSERT_EQUAL(organizer.getStats().number_of_LinphoneConferenceStateDeleted,
	                organizer_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

	conference_address_str = (conference_address) ? linphone_address_as_string(conference_address) : ms_strdup("sip:");
	ms_message("%s is creating conference %s on server %s", linphone_core_get_identity(organizer.getLc()),
	           conference_address_str, linphone_core_get_identity(focus.getLc()));
	ms_free(conference_address_str);

	// The following asserts might fail for dial out conference because the client automatically calls the server after
	// scheduling the conference. Indeed, by the time the verification below is carried out the client might have
	// already called the server therefore a genuine and valid call log may have already been created
	if (!is_dialout) {
		organizer_call_logs = linphone_core_get_call_logs(organizer.getLc());
		if (organizer_call_logs && initial_organizer_call_logs) {
			BC_ASSERT_EQUAL(bctbx_list_size(organizer_call_logs), bctbx_list_size(initial_organizer_call_logs), size_t,
			                "%zu");
		} else {
			BC_ASSERT_PTR_NULL(organizer_call_logs);
			BC_ASSERT_PTR_NULL(initial_organizer_call_logs);
		}
		focus_call_logs = linphone_core_get_call_logs(focus.getLc());
		if (focus_call_logs && initial_focus_call_logs) {
			BC_ASSERT_EQUAL(bctbx_list_size(focus_call_logs), bctbx_list_size(initial_focus_call_logs), size_t, "%zu");
		} else {
			BC_ASSERT_PTR_NULL(focus_call_logs);
			BC_ASSERT_PTR_NULL(initial_focus_call_logs);
		}
	}

end:
	if (uid) {
		ms_free(uid);
	}

	if (organizer_address) linphone_address_unref(organizer_address);
	if (conference_scheduler) linphone_conference_scheduler_unref(conference_scheduler);
	bctbx_list_free(coresList);
	bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
	return conference_address;
}

size_t compute_no_video_streams(bool_t enable_video, LinphoneCall *call, LinphoneConference *conference) {
	size_t nb_video_streams = 0;
	bool_t is_in_conference = linphone_call_is_in_conference(call);

	const LinphoneCallParams *call_params =
	    is_in_conference ? linphone_call_get_remote_params(call) : linphone_call_get_params(call);
	bool_t call_video_enabled = linphone_call_params_video_enabled(call_params);
	LinphoneMediaDirection call_video_dir = linphone_call_params_get_video_direction(call_params);
	bool_t camera_enabled = linphone_call_params_camera_enabled(call_params);
	if (enable_video && call_video_enabled && (call_video_dir != LinphoneMediaDirectionInactive)) {
		bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
		for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
			LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
			bool_t video_available = linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo);
			bool_t thumbnail_available = linphone_participant_device_get_thumbnail_stream_availability(d);
			LinphoneMediaDirection dir = linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo);
			const LinphoneAddress *device_address = linphone_participant_device_get_address(d);
			bool_t is_me = is_in_conference
			                   ? linphone_address_weak_equal(device_address, linphone_call_get_remote_address(call))
			                   : linphone_conference_is_me(conference, device_address);
			LinphoneConferenceLayout call_video_layout = linphone_call_params_get_conference_video_layout(call_params);
			bool_t is_active_speaker = (call_video_layout == LinphoneConferenceLayoutActiveSpeaker);
			bool_t is_grid = (call_video_layout == LinphoneConferenceLayoutGrid);

			const SalMediaDescription *call_desc =
			    is_in_conference ? _linphone_call_get_remote_desc(call) : _linphone_call_get_local_desc(call);
			bool screen_sharing_requested = (call_desc->findIdxStreamWithContent("slides") != -1);

			// Stream with content main/slides/speaker
			// The SDP has a main stream if
			// - it is available and the camera is on
			// - in active speaker layout, its direction is recvonly
			if (is_me &&
			    ((video_available && (is_active_speaker || (is_grid && camera_enabled))) ||
			     (is_active_speaker && !screen_sharing_requested && (dir == LinphoneMediaDirectionRecvOnly)))) {
				nb_video_streams++;
			}

			if (thumbnail_available) {
				if (is_me) {
					// Stream with content thumbnail
					if (camera_enabled) {
						nb_video_streams++;
					}
				} else {
					nb_video_streams++;
				}
			}
		}
		bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
	} else {
		nb_video_streams = 0;
	}

	return nb_video_streams;
}

size_t compute_no_audio_streams(BCTBX_UNUSED(LinphoneCall *call), BCTBX_UNUSED(LinphoneConference *conference)) {
	size_t nb_audio_streams = 1;
	return nb_audio_streams;
}

static bool check_conference_security(BCTBX_UNUSED(LinphoneConference *conference)) {
	bool security_check = true;

#ifdef HAVE_ADVANCED_IM
	const LinphoneConferenceParams *conference_params = linphone_conference_get_current_params(conference);
	auto security_level = linphone_conference_params_get_security_level(conference_params);
	if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
		auto clientConference = dynamic_cast<const LinphonePrivate::ClientConference *>(Conference::toCpp(conference));
		if (clientConference) {
			const auto &clientEktManager = clientConference->getClientEktManager();
			if (clientEktManager) {
				// Check the EKT is known
				return clientEktManager->getSelectedEkt();
			} else {
				security_check = false;
			}
		} else {
			security_check = false;
		}
	}
#endif // HAVE_ADVANCED_IM

	return security_check;
}
void configure_end_to_end_encrypted_conference_server(Focus &focus) {
	// Focus is in full packet mode: transfer packet not payload
	LinphoneConfig *focus_config = linphone_core_get_config(focus.getLc());
	linphone_config_set_int(focus_config, "sound", "conference_mode",
	                        static_cast<int>(MSConferenceModeRouterFullPacket));
	linphone_config_set_int(focus_config, "video", "conference_mode",
	                        static_cast<int>(MSConferenceModeRouterFullPacket));
}

bool verify_participant_addition_stats(bctbx_list_t *coresList,
                                       const ClientConference &participant,
                                       stats participantStat,
                                       int nbParticipantsAdded,
                                       int nbNotifyEktReceived) {
	bool result = BC_ASSERT_TRUE(wait_for_list(coresList, &participant.getStats().number_of_participants_added,
	                                           participantStat.number_of_participants_added + nbParticipantsAdded,
	                                           liblinphone_tester_sip_timeout));
	result = result &&
	         BC_ASSERT_TRUE(wait_for_list(coresList, &participant.getStats().number_of_participant_devices_added,
	                                      participantStat.number_of_participant_devices_added + nbParticipantsAdded,
	                                      liblinphone_tester_sip_timeout));
	result = result && BC_ASSERT_TRUE(wait_for_list(
	                       coresList, &participant.getStats().number_of_conference_participant_devices_present,
	                       participantStat.number_of_conference_participant_devices_present + nbParticipantsAdded,
	                       liblinphone_tester_sip_timeout));
	result = result &&
	         BC_ASSERT_TRUE(wait_for_list(coresList, &participant.getStats().number_of_participant_devices_present,
	                                      participantStat.number_of_participant_devices_present + nbParticipantsAdded,
	                                      liblinphone_tester_sip_timeout));
	result = result && BC_ASSERT_TRUE(wait_for_list(coresList, &participant.getStats().number_of_NotifyEktReceived,
	                                                participantStat.number_of_NotifyEktReceived + nbNotifyEktReceived,
	                                                liblinphone_tester_sip_timeout));
	return result;
}

bool verify_participant_removal_stats(bctbx_list_t *coresList,
                                      const ClientConference &participant,
                                      stats participantStat,
                                      int nbParticipantsAdded,
                                      int nbNotifyEktReceived) {
	bool result = BC_ASSERT_TRUE(wait_for_list(coresList, &participant.getStats().number_of_participants_removed,
	                                           participantStat.number_of_participants_removed + nbParticipantsAdded,
	                                           liblinphone_tester_sip_timeout));
	result = result &&
	         BC_ASSERT_TRUE(wait_for_list(coresList, &participant.getStats().number_of_participant_devices_removed,
	                                      participantStat.number_of_participant_devices_removed + nbParticipantsAdded,
	                                      liblinphone_tester_sip_timeout));
	result = result && BC_ASSERT_TRUE(wait_for_list(coresList, &participant.getStats().number_of_NotifyEktReceived,
	                                                participantStat.number_of_NotifyEktReceived + nbNotifyEktReceived,
	                                                liblinphone_tester_sip_timeout));
	return result;
}

#ifdef HAVE_ADVANCED_IM
void does_all_participants_have_matching_ekt(LinphoneCoreManager *focus,
                                             std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> members,
                                             const LinphoneAddress *confAddr) {
	auto focusConference = linphone_core_search_conference_2(focus->lc, confAddr);
	BC_ASSERT_PTR_NOT_NULL(focusConference);
	if (focusConference) {
		const LinphoneConferenceParams *conference_params = linphone_conference_get_current_params(focusConference);
		BC_ASSERT_PTR_NOT_NULL(conference_params);
		if (conference_params) {
			auto security_level = linphone_conference_params_get_security_level(conference_params);
			if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
				auto firstClientConf = dynamic_cast<const LinphonePrivate::ClientConference *>(
				    Conference::toCpp(linphone_core_search_conference_2(members.begin()->first->lc, confAddr)));
				BC_ASSERT_PTR_NOT_NULL(firstClientConf);
				if (firstClientConf) {
					const auto &firstClientEktManager = firstClientConf->getClientEktManager();
					BC_ASSERT_PTR_NOT_NULL(firstClientEktManager);
					if (firstClientEktManager) {
						const auto &firstClientEktCtx = firstClientEktManager->getEktCtx();
						BC_ASSERT_PTR_NOT_NULL(firstClientEktCtx);
						for (auto member : members) {
							auto conf = linphone_core_search_conference_2(member.first->lc, confAddr);
							BC_ASSERT_PTR_NOT_NULL(conf);
							if (conf) {
								auto rcConf =
								    dynamic_cast<const LinphonePrivate::ClientConference *>(Conference::toCpp(conf));
								BC_ASSERT_PTR_NOT_NULL(rcConf);
								if (rcConf) {
									const auto &clientEktManager = rcConf->getClientEktManager();
									BC_ASSERT_PTR_NOT_NULL(clientEktManager);
									if (clientEktManager) {
										const auto &rcEktCtx = clientEktManager->getEktCtx();
										BC_ASSERT_PTR_NOT_NULL(rcEktCtx);
										if (rcEktCtx && firstClientEktCtx) {
											BC_ASSERT_EQUAL(firstClientEktCtx->getSSpi(), rcEktCtx->getSSpi(), uint16_t,
											                "%d");
											BC_ASSERT_TRUE(firstClientEktCtx->getCSpi() == rcEktCtx->getCSpi());
											BC_ASSERT_TRUE(firstClientEktCtx->getEkt() == rcEktCtx->getEkt());
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}
#endif // HAVE_ADVANCED_IM

void check_muted(std::initializer_list<std::reference_wrapper<CoreManager>> coreMgrs,
                 const LinphoneParticipantDevice *d,
                 std::list<LinphoneCoreManager *> mutedMgrs) {
	const LinphoneAddress *device_address = linphone_participant_device_get_address(d);
	bool_t expect_mute = FALSE;
	for (const auto &mgr : mutedMgrs) {
		expect_mute |= (linphone_address_weak_equal(device_address, mgr->identity));
	}

	BC_ASSERT_TRUE(CoreManagerAssert(coreMgrs).waitUntil(chrono::seconds(10), [&d, &expect_mute] {
		return ((!!linphone_participant_device_get_is_muted(d)) == expect_mute);
	}));
}

void wait_for_conference_streams(std::initializer_list<std::reference_wrapper<CoreManager>> coreMgrs,
                                 std::list<LinphoneCoreManager *> conferenceMgrs,
                                 LinphoneCoreManager *focus,
                                 std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> members,
                                 const LinphoneAddress *confAddr,
                                 bool_t enable_video) {
	std::map<LinphoneCoreManager *, bool_t> camera_enabled_map;
	for (auto mgr : conferenceMgrs) {
		if (mgr != focus) {
			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				const LinphoneCallParams *local_params = linphone_call_get_params(pcall);
				camera_enabled_map[mgr] = linphone_call_params_camera_enabled(local_params);
			}
		}
	}

	for (auto mgr : conferenceMgrs) {
		ms_message("Waiting for manager %s to reach a stable state", linphone_core_get_identity(mgr->lc));
		// wait a bit longer to detect side effect if any
		BC_ASSERT_TRUE(CoreManagerAssert(coreMgrs).waitUntil(chrono::seconds(50), [mgr, &focus, &members, confAddr,
		                                                                           enable_video, &camera_enabled_map,
		                                                                           conferenceMgrs] {
			size_t nb_text_streams = 0;
			std::list<LinphoneCall *> calls;

			bool video_check = false;
			bool participant_check = false;
			bool device_check = false;
			bool call_check = true;
			bool audio_direction_check = true;
			bool security_check = true;

			LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
			if (conference && mgr != focus) {
				security_check = check_conference_security(conference);
			}

			if (mgr == focus) {
				for (const auto &m : members) {
					LinphoneCoreManager *mMgr = m.first;
					LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mMgr->lc, confAddr);
					call_check &=
					    (pcall != nullptr) && (linphone_call_get_state(pcall) == LinphoneCallStateStreamsRunning);

					LinphoneCall *fcall = linphone_core_get_call_by_remote_address2(mgr->lc, mMgr->identity);
					call_check &=
					    (fcall != nullptr) && (linphone_call_get_state(fcall) == LinphoneCallStateStreamsRunning);
					if (fcall) {
						calls.push_back(fcall);
					} else {
						calls.push_back(nullptr);
					}
				}
			} else {
				LinphoneCall *fcall = linphone_core_get_call_by_remote_address2(focus->lc, mgr->identity);
				call_check &= (fcall != nullptr) && (linphone_call_get_state(fcall) == LinphoneCallStateStreamsRunning);

				LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
				call_check &= (pcall != nullptr) && (linphone_call_get_state(pcall) == LinphoneCallStateStreamsRunning);
				if (pcall) {
					calls.push_back(pcall);
				} else {
					calls.push_back(nullptr);
				}
			}
			call_check &= (calls.size() > 0);

			if (mgr != focus) {
				for (auto call : calls) {
					if (call) {

						size_t nb_audio_streams = compute_no_audio_streams(call, conference);
						size_t nb_video_streams = 0;
						if (!!linphone_config_get_int(linphone_core_get_config(mgr->lc), "misc",
						                              "add_participant_label_to_sdp", 1)) {
							nb_video_streams = compute_no_video_streams(enable_video, call, conference);
						} else {
							const LinphoneCallParams *call_params = linphone_call_get_params(call);
							LinphoneConferenceLayout call_video_layout =
							    linphone_call_params_get_conference_video_layout(call_params);
							bool_t is_active_speaker = (call_video_layout == LinphoneConferenceLayoutActiveSpeaker);
							LinphoneMediaDirection call_video_dir =
							    linphone_call_params_get_video_direction(call_params);
							if (is_active_speaker) {
								if (call_video_dir == LinphoneMediaDirectionInactive) {
									nb_video_streams = 0;
								} else if (call_video_dir == LinphoneMediaDirectionRecvOnly) {
									nb_video_streams = 1;
								} else {
									nb_video_streams = 2;
								}
							} else {
								if ((call_video_dir == LinphoneMediaDirectionRecvOnly) ||
								    (call_video_dir == LinphoneMediaDirectionInactive)) {
									nb_video_streams = 0;
								} else {
									nb_video_streams = 1;
								}
							}
						}

						LinphoneCall *fcall = linphone_core_get_call_by_remote_address2(focus->lc, mgr->identity);
						call_check &= (fcall != nullptr);
						for (auto call2 : {call, fcall}) {
							const SalMediaDescription *call_result_desc = _linphone_call_get_result_desc(call2);
							call_check &= ((call_result_desc->nbActiveStreamsOfType(SalAudio) == nb_audio_streams) &&
							               (call_result_desc->nbActiveStreamsOfType(SalVideo) == nb_video_streams) &&
							               (call_result_desc->nbActiveStreamsOfType(SalText) == nb_text_streams));
						}
					}
				}
			}

			if (conference) {
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
				bool thumbnails_requested = Conference::toCpp(conference)->areThumbnailsRequested(true);
				size_t devices_nb = bctbx_list_size(devices);
				video_check = (devices_nb > 0);
				participant_check = (devices_nb > 0);
				device_check = (devices_nb > 0);
				LinphoneCall *call = NULL;
				if (enable_video) {
					bool expected_thumbnails_requested = are_participants_camera_streams_requested(mgr, conference);
					video_check &= (expected_thumbnails_requested == thumbnails_requested);
				}
				for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					bool_t found = FALSE;
					const LinphoneAddress *device_address = linphone_participant_device_get_address(d);
					if (device_address) {
						device_check |= !!linphone_address_has_uri_param(device_address, "gr");
					} else {
						device_check = false;
					}
					for (const auto &m : members) {
						LinphoneCoreManager *mMgr = m.first;
						found |= linphone_address_weak_equal(mMgr->identity, device_address);
					}
					if (mgr == focus) {
						call = linphone_core_get_call_by_remote_address2(mgr->lc, device_address);
					} else {
						if (calls.front()) {
							call = calls.front();
						}
					}
					bool_t is_me = linphone_conference_is_me(conference, device_address);
					LinphoneParticipant *p = is_me ? linphone_conference_get_me(conference)
					                               : linphone_conference_find_participant(conference, device_address);
					participant_check &= (p != nullptr);
					LinphoneMediaDirection expected_audio_direction = LinphoneMediaDirectionInactive;
					bool_t video_available =
					    linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo);
					LinphoneMediaDirection video_dir =
					    linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo);
					LinphoneMediaDirection thumbnail_dir =
					    linphone_participant_device_get_thumbnail_stream_capability(d);
					bool is_thumbnail_inactive = false;
					if (p) {
						LinphoneParticipantRole role = linphone_participant_get_role(p);
						expected_audio_direction =
						    ((role == LinphoneParticipantRoleSpeaker) ? LinphoneMediaDirectionSendRecv
						                                              : LinphoneMediaDirectionRecvOnly);
						LinphoneMediaDirection audio_dir =
						    linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeAudio);
						audio_direction_check &= (audio_dir == expected_audio_direction);
						if (role == LinphoneParticipantRoleListener) {
							video_check &= ((video_dir == LinphoneMediaDirectionRecvOnly) ||
							                (video_dir == LinphoneMediaDirectionInactive));
						}
						// Thumbnail
						try {
							auto participant_mgr_it =
							    std::find_if(conferenceMgrs.cbegin(), conferenceMgrs.cend(),
							                 [device_address](const LinphoneCoreManager *mgr) {
								                 return linphone_address_weak_equal(mgr->identity, device_address);
							                 });
							if (participant_mgr_it != conferenceMgrs.cend()) {
								LinphoneCoreManager *participant_mgr = (*participant_mgr_it);
								bool_t camera_enabled = camera_enabled_map.at(participant_mgr);
								// A participant has an inactive thumbnail stream if:
								// - it is a listener
								// - the camera is disabled
								// - participant video is not available (i.e. its capability is either RecvOnly or
								// Inactive)
								is_thumbnail_inactive =
								    (role == LinphoneParticipantRoleListener) || !camera_enabled || !video_available;
							}
						} catch (std::out_of_range &) {
							ms_fatal("Unable to find manager with address %s",
							         linphone_address_as_string(device_address));
						}
					} else {
						audio_direction_check = false;
						video_check = false;
					}

					LinphoneParticipantDeviceState expected_state = LinphoneParticipantDeviceStateJoining;
					if (found) {
						expected_state = LinphoneParticipantDeviceStatePresent;
						device_check &= (linphone_participant_device_is_in_conference(d) ? true : false);
					} else {
						expected_state = LinphoneParticipantDeviceStateLeft;
						device_check &= (linphone_participant_device_is_in_conference(d) ? false : true);
					}
					device_check &= (linphone_participant_device_get_state(d) == expected_state);
					if (call) {
						if (is_me) {
							const LinphoneCallParams *call_current_params = linphone_call_get_current_params(call);
							LinphoneMediaDirection call_audio_direction =
							    linphone_call_params_get_audio_direction(call_current_params);
							audio_direction_check &= (call_audio_direction == expected_audio_direction);
						}

						bool_t thumbnail_available = linphone_participant_device_get_thumbnail_stream_availability(d);
						const LinphoneCallParams *call_params = linphone_call_get_params(call);
						bool_t call_video_enabled = linphone_call_params_video_enabled(call_params);
						bool_t video_available =
						    linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo);
						if (enable_video && ((mgr == focus) || call_video_enabled)) {
							// Video is available if the direction is sendrecv or sendonly
							bool_t expected_video_available = FALSE;
							bool_t expected_thumbnail_available = FALSE;
							if (thumbnails_requested || is_me) {
								video_check &= (is_thumbnail_inactive)
								                   ? (thumbnail_dir == LinphoneMediaDirectionInactive)
								                   : (thumbnail_dir == LinphoneMediaDirectionSendOnly);
								expected_thumbnail_available = (is_thumbnail_inactive) ? FALSE : TRUE;
							}
							expected_video_available = ((video_dir == LinphoneMediaDirectionSendRecv) ||
							                            (video_dir == LinphoneMediaDirectionSendOnly));
							video_check &= (video_available == expected_video_available);
							video_check &= (thumbnail_available == expected_thumbnail_available);

							if (!is_thumbnail_inactive) {
								bool_t is_screen_sharing = linphone_participant_device_screen_sharing_enabled(d);
								const char *video_label =
								    linphone_participant_device_get_stream_label(d, LinphoneStreamTypeVideo);
								const char *thumbnail_label = linphone_participant_device_get_thumbnail_stream_label(d);
								if (is_screen_sharing) {
									video_check &= (strcmp(video_label, thumbnail_label) != 0);
								} else {
									if (video_label && thumbnail_label) {
										video_check &= (strcmp(video_label, thumbnail_label) == 0);
									} else {
										video_check &= (video_label == nullptr) && (thumbnail_label == nullptr);
									}
								}
							}
						} else {
							video_check &= (video_available == FALSE);
							video_check &= (thumbnail_available == FALSE);
						}
					}
				}
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}

				for (const auto &[mMgr, info] : members) {
					LinphoneParticipant *p = linphone_conference_is_me(conference, mMgr->identity)
					                             ? linphone_conference_get_me(conference)
					                             : linphone_conference_find_participant(conference, mMgr->identity);
					participant_check &= (p != nullptr);
					if (p) {
						const LinphoneAddress *participant_address = linphone_participant_get_address(p);
						if (participant_address) {
							participant_check &= !linphone_address_has_uri_param(participant_address, "gr");
						} else {
							participant_check = false;
						}
						participant_check &=
						    ((linphone_participant_get_role(p) == linphone_participant_info_get_role(info)) &&
						     (linphone_participant_get_role(p) != LinphoneParticipantRoleUnknown));
					}
				}
			}
			return audio_direction_check && video_check && device_check && call_check && participant_check &&
			       security_check;
		}));
	}
#ifdef HAVE_ADVANCED_IM
	does_all_participants_have_matching_ekt(focus, members, confAddr);
#endif // HAVE_ADVANCED_IM
}

void set_video_settings_in_conference(LinphoneCoreManager *focus,
                                      LinphoneCoreManager *participant,
                                      std::list<LinphoneCoreManager *> members,
                                      const LinphoneAddress *confAddr,
                                      bool_t enable_video,
                                      LinphoneMediaDirection video_direction,
                                      bool_t answer_enable_video,
                                      LinphoneMediaDirection answer_video_direction) {
	std::list<LinphoneCoreManager *> active_cores = members;
	active_cores.push_back(focus);

	bctbx_list_t *coresList = bctbx_list_append(NULL, focus->lc);
	stats *participants_initial_stats = NULL;
	stats focus_stat = focus->stat;
	int counter = 1;
	bool_t recvonly_video = TRUE;
	bool_t inactive_call_video = TRUE;
	LinphoneConferenceLayout call_conference_layout = LinphoneConferenceLayoutGrid;
	for (auto mgr : members) {
		coresList = bctbx_list_append(coresList, mgr->lc);
		// Allocate memory
		participants_initial_stats = (stats *)realloc(participants_initial_stats, counter * sizeof(stats));
		// Append element
		participants_initial_stats[counter - 1] = mgr->stat;

		LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
		BC_ASSERT_PTR_NOT_NULL(call);
		if (call) {
			if (participant != mgr) {
				const LinphoneCallParams *call_params = linphone_call_get_current_params(call);
				bool_t call_enable_video = linphone_call_params_video_enabled(call_params);
				LinphoneMediaDirection call_video_direction = linphone_call_params_get_video_direction(call_params);
				inactive_call_video &= (!call_enable_video);

				if (call_enable_video) {
					recvonly_video &= (call_video_direction == LinphoneMediaDirectionRecvOnly);
				}
			} else {
				const LinphoneCallParams *call_params = linphone_call_get_params(call);
				call_conference_layout = linphone_call_params_get_conference_video_layout(call_params);
			}
		}

		// Increment counter
		counter++;
	}

	if (inactive_call_video) {
		recvonly_video = FALSE;
	}

	// During an encrypted conference or whenever a participant chooses a grid layout the receive and send video
	// streams are split therefore if the other participants have an inactive video or the can only receive it and
	// the chose video direction is RecvOnly, then no video streams will be sent out
	bool_t negotiated_inactive_video =
	    ((call_conference_layout == LinphoneConferenceLayoutGrid) && (inactive_call_video || recvonly_video) &&
	     (video_direction == LinphoneMediaDirectionRecvOnly));

	// Whenever a participant chooses a grid layout, the participant requestes 2 streams for its send component and
	// the streams with recvonly direction are those of the others participants
	bool_t inactive_video_sdp_sent =
	    ((call_conference_layout == LinphoneConferenceLayoutGrid) && (inactive_call_video || recvonly_video) &&
	     (video_direction == LinphoneMediaDirectionRecvOnly));
	bool_t previous_enable_video = FALSE;
	LinphoneMediaDirection previous_video_direction = LinphoneMediaDirectionInvalid;

	LinphoneCall *participant_call = linphone_core_get_call_by_remote_address2(participant->lc, confAddr);
	BC_ASSERT_PTR_NOT_NULL(participant_call);
	if (participant_call) {
		const LinphoneCallParams *call_params = linphone_call_get_current_params(participant_call);
		previous_enable_video = linphone_call_params_video_enabled(call_params);
		previous_video_direction = linphone_call_params_get_video_direction(call_params);

		ms_message("%s %s video with direction %s", linphone_core_get_identity(participant->lc),
		           ((enable_video) ? "enables" : "disables"), linphone_media_direction_to_string(video_direction));

		LinphoneCallParams *new_params = linphone_core_create_call_params(participant->lc, participant_call);
		linphone_call_params_enable_video(new_params, enable_video);
		linphone_call_params_set_video_direction(new_params, video_direction);
		linphone_call_update(participant_call, new_params);
		linphone_call_params_unref(new_params);
	}

	BC_ASSERT_TRUE(wait_for_list(coresList, &focus->stat.number_of_LinphoneCallUpdatedByRemote,
	                             focus_stat.number_of_LinphoneCallUpdatedByRemote + 1, liblinphone_tester_sip_timeout));

	int focus_defer_update =
	    !!linphone_config_get_int(linphone_core_get_config(focus->lc), "sip", "defer_update_default", FALSE);
	bool_t enable = enable_video && ((focus_defer_update == TRUE) ? answer_enable_video : TRUE);
	if (focus_defer_update == TRUE) {
		LinphoneAddress *uri = participant->identity;
		LinphoneCall *focus_call = linphone_core_get_call_by_remote_address2(focus->lc, uri);
		BC_ASSERT_PTR_NOT_NULL(focus_call);
		if (focus_call) {
			ms_message("%s %s video with direction %s", linphone_core_get_identity(focus->lc),
			           ((answer_enable_video) ? "accepts" : "refuses"),
			           linphone_media_direction_to_string(answer_video_direction));
			LinphoneCallParams *focus_params = linphone_core_create_call_params(focus->lc, focus_call);
			linphone_call_params_enable_video(focus_params, answer_enable_video);
			linphone_call_params_set_video_direction(focus_params, answer_video_direction);
			linphone_call_accept_update(focus_call, focus_params);
			linphone_call_params_unref(focus_params);
		}
	}

	counter = 0;
	int no_updates = 0;
	bool_t media_not_changed = ((!previous_enable_video && enable && inactive_video_sdp_sent) ||
	                            ((previous_enable_video == enable) && (!enable)));
	bool_t changing_video_direction =
	    (enable && previous_enable_video && (previous_video_direction != video_direction));
	for (auto mgr : members) {
		LinphoneCall *member_call = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
		BC_ASSERT_PTR_NOT_NULL(member_call);
		bool_t member_enable_video = FALSE;
		if (member_call) {
			const LinphoneCallParams *call_params = linphone_call_get_current_params(member_call);
			member_enable_video = linphone_call_params_video_enabled(call_params);
		}

		if ((member_enable_video &&
		     (((previous_enable_video != enable) && (previous_video_direction == LinphoneMediaDirectionSendRecv)) ||
		      changing_video_direction)) ||
		    (mgr == participant)) {
			LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(call);
			if (call) {
				const LinphoneCallParams *call_lparams = linphone_call_get_params(call);
				if (linphone_call_params_video_enabled(call_lparams)) {

					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating,
					                             participants_initial_stats[counter].number_of_LinphoneCallUpdating + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(
					    wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning,
					                  participants_initial_stats[counter].number_of_LinphoneCallStreamsRunning + 1,
					                  liblinphone_tester_sip_timeout));

					no_updates++;
				}
			}

			if (((previous_enable_video == enable) && (!enable)) ||
			    (enable && (video_direction == LinphoneMediaDirectionRecvOnly) && inactive_call_video &&
			     !changing_video_direction)) {
				BC_ASSERT_FALSE(wait_for_list(
				    coresList, &mgr->stat.number_of_participant_devices_media_capability_changed,
				    participants_initial_stats[counter].number_of_participant_devices_media_capability_changed + 1,
				    5000));
			} else {
				BC_ASSERT_TRUE(wait_for_list(
				    coresList, &mgr->stat.number_of_participant_devices_media_capability_changed,
				    participants_initial_stats[counter].number_of_participant_devices_media_capability_changed + 1,
				    liblinphone_tester_sip_timeout));
			}
			BC_ASSERT_EQUAL(mgr->stat.number_of_participants_added,
			                participants_initial_stats[counter].number_of_participants_added, int, "%0d");
			BC_ASSERT_EQUAL(mgr->stat.number_of_participant_devices_added,
			                participants_initial_stats[counter].number_of_participant_devices_added, int, "%0d");
			BC_ASSERT_EQUAL(mgr->stat.number_of_conference_participant_devices_present,
			                participants_initial_stats[counter].number_of_conference_participant_devices_present, int,
			                "%0d");
			BC_ASSERT_EQUAL(mgr->stat.number_of_participant_devices_present,
			                participants_initial_stats[counter].number_of_participant_devices_present, int, "%0d");
		}
		if (media_not_changed) {
			BC_ASSERT_FALSE(wait_for_list(
			    coresList, &mgr->stat.number_of_participant_devices_media_capability_changed,
			    participants_initial_stats[counter].number_of_participant_devices_media_capability_changed + 1, 5000));
		} else {
			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &mgr->stat.number_of_participant_devices_media_capability_changed,
			    participants_initial_stats[counter].number_of_participant_devices_media_capability_changed + 1,
			    liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_EQUAL(mgr->stat.number_of_participants_added,
		                participants_initial_stats[counter].number_of_participants_added, int, "%0d");
		BC_ASSERT_EQUAL(mgr->stat.number_of_participant_devices_added,
		                participants_initial_stats[counter].number_of_participant_devices_added, int, "%0d");
		BC_ASSERT_EQUAL(mgr->stat.number_of_conference_participant_devices_present,
		                participants_initial_stats[counter].number_of_conference_participant_devices_present, int,
		                "%0d");
		BC_ASSERT_EQUAL(mgr->stat.number_of_participant_devices_present,
		                participants_initial_stats[counter].number_of_participant_devices_present, int, "%0d");
		counter++;
	}
	BC_ASSERT_TRUE(wait_for_list(coresList, &focus->stat.number_of_LinphoneCallUpdatedByRemote,
	                             focus_stat.number_of_LinphoneCallUpdatedByRemote + no_updates,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &focus->stat.number_of_LinphoneCallStreamsRunning,
	                             focus_stat.number_of_LinphoneCallStreamsRunning + no_updates,
	                             liblinphone_tester_sip_timeout));
	if (media_not_changed) {
		BC_ASSERT_FALSE(wait_for_list(coresList, &focus->stat.number_of_participant_devices_media_capability_changed,
		                              focus_stat.number_of_participant_devices_media_capability_changed + 1, 1000));
	} else {
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus->stat.number_of_participant_devices_media_capability_changed,
		                             focus_stat.number_of_participant_devices_media_capability_changed + 1,
		                             liblinphone_tester_sip_timeout));
	}
	BC_ASSERT_EQUAL(focus->stat.number_of_participants_added, focus_stat.number_of_participants_added, int, "%0d");
	BC_ASSERT_EQUAL(focus->stat.number_of_participant_devices_added, focus_stat.number_of_participant_devices_added,
	                int, "%0d");
	BC_ASSERT_EQUAL(focus->stat.number_of_conference_participant_devices_present,
	                focus_stat.number_of_conference_participant_devices_present, int, "%0d");
	BC_ASSERT_EQUAL(focus->stat.number_of_participant_devices_present, focus_stat.number_of_participant_devices_present,
	                int, "%0d");

	for (auto mgr : active_cores) {
		const LinphoneAddress *local_address = (mgr == focus) ? confAddr : mgr->identity;
		LinphoneConference *pconference = linphone_core_search_conference(mgr->lc, NULL, local_address, confAddr, NULL);
		BC_ASSERT_PTR_NOT_NULL(pconference);
		if (pconference) {
			LinphoneParticipant *p = (mgr == participant)
			                             ? linphone_conference_get_me(pconference)
			                             : linphone_conference_find_participant(pconference, participant->identity);
			BC_ASSERT_PTR_NOT_NULL(p);
			if (p) {
				bctbx_list_t *devices = linphone_participant_get_devices(p);
				for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					LinphoneMediaDirection expected_video_direction = video_direction;
					if (enable == TRUE) {
						if (negotiated_inactive_video) {
							expected_video_direction = LinphoneMediaDirectionInactive;
						} else if ((call_conference_layout == LinphoneConferenceLayoutGrid) &&
						           (inactive_call_video || recvonly_video) &&
						           (video_direction == LinphoneMediaDirectionSendRecv)) {
							// Layout Grid doesn't allow the server to always deduce the right client's video
							// direction because the send and recv parts of the video streams are separate.
							expected_video_direction = LinphoneMediaDirectionSendOnly;
						} else {
							expected_video_direction = video_direction;
						}
					} else {
						expected_video_direction = LinphoneMediaDirectionInactive;
					}
					BC_ASSERT_EQUAL(linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo),
					                expected_video_direction, int, "%0d");
				}
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
			}
		}
	}

	LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(participant->lc, confAddr);
	BC_ASSERT_PTR_NOT_NULL(pcall);
	if (pcall) {
		const LinphoneCallParams *call_lparams = linphone_call_get_params(pcall);
		BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enable_video, int, "%0d");
		const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(pcall);
		BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams),
		                ((negotiated_inactive_video) ? FALSE : ((focus_defer_update == TRUE) ? enable : enable_video)),
		                int, "%0d");
		const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
		BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams),
		                ((negotiated_inactive_video) ? FALSE : enable), int, "%0d");
	}

	LinphoneAddress *uri = participant->identity;
	LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus->lc, uri);
	BC_ASSERT_PTR_NOT_NULL(ccall);
	if (ccall) {
		const LinphoneCallParams *call_lparams = linphone_call_get_params(ccall);
		BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams),
		                (focus_defer_update == TRUE) ? answer_enable_video
		                                             : ((negotiated_inactive_video) ? FALSE : enable_video),
		                int, "%0d");
		const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(ccall);
		BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams),
		                (negotiated_inactive_video) ? FALSE : enable_video, int, "%0d");
		const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
		BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams),
		                ((negotiated_inactive_video) ? FALSE : enable), int, "%0d");
	}

	bctbx_list_free(coresList);
	ms_free(participants_initial_stats);
}

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
                            bool_t add_participant_after_end,
                            bool_t version_mismatch,
                            bool_t use_relay_ice_candidates,
                            bool_t enable_chat) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		bool_t enable_lime = (security_level == LinphoneConferenceSecurityLevelEndToEnd ? TRUE : FALSE);

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

		linphone_core_set_add_admin_information_to_contact(marie.getLc(), TRUE);
		linphone_core_set_add_admin_information_to_contact(laure.getLc(), TRUE);

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t *coresList = NULL;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(),
		                 berthe.getCMgr()}) {
			if (enable_video) {
				if ((audio_only_participant == FALSE) || (mgr != pauline.getCMgr())) {
					LinphoneVideoActivationPolicy *pol =
					    linphone_factory_create_video_activation_policy(linphone_factory_get());
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
					linphone_config_set_string(linphone_core_get_config(mgr->lc), "video", "max_conference_size",
					                           "vga");
				}
			}

			if ((mgr == laure.getCMgr()) && (!!use_relay_ice_candidates)) {
				linphone_core_set_user_agent(mgr->lc, "Natted Linphone", NULL);
				linphone_core_enable_forced_ice_relay(mgr->lc, TRUE);
			}

			if (mgr == pauline.getCMgr()) {
				linphone_core_set_conference_max_thumbnails(mgr->lc, 2);
			}

			if (mgr == focus.getCMgr()) {
				// ICE relay candidates will only be tester with Laure
				linphone_core_set_user_agent(mgr->lc, "Natted Linphone", NULL);
			} else {
				linphone_core_set_default_conference_layout(mgr->lc, layout);
				linphone_core_set_media_encryption(mgr->lc, encryption);
			}

			if (do_not_use_proxy) {
				linphone_core_set_default_proxy_config(mgr->lc, NULL);
			}

			// Enable ICE at the account level but not at the core level
			enable_stun_in_mgr(mgr, enable_stun, enable_ice, FALSE, FALSE);

			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip", "update_call_when_ice_completed", TRUE);
			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip",
			                        "update_call_when_ice_completed_with_dtls", FALSE);

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		if (enable_lime) {
			configure_end_to_end_encrypted_conference_server(focus);
		}

		int nortp_timeout = 10;
		linphone_core_set_nortp_timeout(marie.getLc(), nortp_timeout);

		if (enable_lime) {
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(berthe.getLc()));
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), participant_list_type);

		stats focus_stat = focus.getStats();

		const bool oneRoleAllowed = (allowedRoles.size() == 1);
		bool speakerAllowed = std::find(allowedRoles.cbegin(), allowedRoles.cend(), LinphoneParticipantRoleSpeaker) !=
		                      allowedRoles.cend();
		bool listenerAllowed = std::find(allowedRoles.cbegin(), allowedRoles.cend(), LinphoneParticipantRoleListener) !=
		                       allowedRoles.cend();
		bool all_speakers = speakerAllowed && oneRoleAllowed;
		bool all_listeners = listenerAllowed && oneRoleAllowed;

		std::list<LinphoneCoreManager *> participants{laure.getCMgr(), pauline.getCMgr()};
		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(),
		                                                laure.getCMgr()};
		std::list<LinphoneCoreManager *> allConferenceMgrs{focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(),
		                                                   laure.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()};

		time_t end_time = (duration <= 0) ? -1 : (start_time + duration * 60);
		const char *initialSubject = "Test characters: ^ :) ¤ çà @";
		const char *description = "Paris Baker";

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		participantList.insert(std::make_pair(
		    laure.getCMgr(), add_participant_info_to_list(&participants_info, laure.getCMgr()->identity,
		                                                  (listenerAllowed) ? LinphoneParticipantRoleListener
		                                                                    : LinphoneParticipantRoleSpeaker,
		                                                  -1)));
		participantList.insert(std::make_pair(
		    pauline.getCMgr(), add_participant_info_to_list(&participants_info, pauline.getCMgr()->identity,
		                                                    (speakerAllowed) ? LinphoneParticipantRoleSpeaker
		                                                                     : LinphoneParticipantRoleListener,
		                                                    -1)));
		if (all_speakers || all_listeners) {
			participantList.insert(std::make_pair(
			    marie.getCMgr(), add_participant_info_to_list(&participants_info, marie.getCMgr()->identity,
			                                                  (all_listeners) ? LinphoneParticipantRoleListener
			                                                                  : LinphoneParticipantRoleSpeaker,
			                                                  -1)));
		}

		if (version_mismatch) {
			linphone_core_remove_linphone_spec(laure.getLc(), "conference");
			linphone_core_add_linphone_spec(laure.getLc(), "conference/1.0");
		}

		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, security_level, enable_video, enable_chat, NULL);
		BC_ASSERT_PTR_NOT_NULL(confAddr);
		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:");

		char *uid = NULL;
		if (confAddr) {
			LinphoneConferenceInfo *info = linphone_core_find_conference_information_from_uri(marie.getLc(), confAddr);
			if (BC_ASSERT_PTR_NOT_NULL(info)) {
				uid = ms_strdup(linphone_conference_info_get_ics_uid(info));
				linphone_conference_info_unref(info);
			}
		}

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 2,
		                             liblinphone_tester_sip_timeout));

		if (server_restart) {
			coresList = bctbx_list_remove(coresList, focus.getLc());

			ms_message("%s is restarting its core", linphone_core_get_identity(focus.getLc()));
			// Restart flexisip
			focus.reStart();

			if (enable_video) {
				LinphoneVideoActivationPolicy *pol =
				    linphone_factory_create_video_activation_policy(linphone_factory_get());
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

			if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
				configure_end_to_end_encrypted_conference_server(focus);
			}
		}

		for (auto mgr : members) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_call_params_set_media_encryption(new_params, encryption);
			linphone_call_params_set_video_direction(new_params, video_direction);
			LinphoneAddress *confAddr2 = linphone_address_clone(confAddr);
			if (mgr == pauline.getCMgr()) {
				linphone_call_params_enable_mic(new_params, FALSE);
			}

			// Let Marie to call the conference after lowercasing the conf-id.
			// In some tests, Marie's core restarts later on in the test and it will call with the unmodified conference
			// address (the conf-id parameter will be a mix of uppercase and lowercase characters) and this will allow
			// to verify that only one ConferenceInfo is created because the address parameter comparison is case
			// insensitive
			if (mgr == marie.getCMgr()) {
				// Convert conf-id to lowercase
				const char *conf_id_param_name = "conf-id";
				if (linphone_address_has_uri_param(confAddr2, conf_id_param_name)) {
					const char *conf_id = linphone_address_get_uri_param(confAddr2, conf_id_param_name);
					char *conf_id_lowercase = ms_strdup(conf_id);
					// Convert uppercase letters to lowercase
					for (size_t i = 0; i < strlen(conf_id_lowercase); i++) {
						conf_id_lowercase[i] = (char)tolower(conf_id_lowercase[i]);
					}
					linphone_address_remove_uri_param(confAddr2, conf_id_param_name);
					linphone_address_set_uri_param(confAddr2, conf_id_param_name, conf_id_lowercase);
					ms_free(conf_id_lowercase);
				}
			}
			char *conference_address2_str = (confAddr2) ? linphone_address_as_string(confAddr2) : ms_strdup("sip:");
			ms_message("%s is entering conference %s", linphone_core_get_identity(mgr->lc), conference_address2_str);
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr2, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr2);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
				BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
			}
			linphone_address_unref(confAddr2);
			ms_free(conference_address2_str);
		}

		int nb_subscriptions = 1;
		if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
			nb_subscriptions = 2; // One more subscription for the EKT
		}

		if (version_mismatch) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallError, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallReleased, 1,
			                             liblinphone_tester_sip_timeout));
			conferenceMgrs.remove(laure.getCMgr());
			members.remove(laure.getCMgr());
			participants.remove(laure.getCMgr());
			BC_ASSERT_FALSE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallOutgoingInit, 2, 3000));
		}

		int idx = 1;
		for (auto mgr : members) {
			ms_message("Checking that the call of %s to conference %s has been established successfully",
			           linphone_core_get_identity(mgr->lc), conference_address_str);
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
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress,
			                             nb_subscriptions, 5000));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, nb_subscriptions, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_conference_full_state_received, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));
			if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyEktReceived, 1,
				                             liblinphone_tester_sip_timeout));
			}

			if ((encryption == LinphoneMediaEncryptionDTLS) || (encryption == LinphoneMediaEncryptionZRTP)) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEncryptedOn, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEncryptedOn, idx,
				                             liblinphone_tester_sip_timeout));
			}

			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
				const LinphoneMediaEncryption pcall_enc = linphone_call_params_get_media_encryption(call_cparams);
				BC_ASSERT_EQUAL(pcall_enc, encryption, int, "%d");
			}
			LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(ccall);
			if (ccall) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
				const LinphoneMediaEncryption ccall_enc = linphone_call_params_get_media_encryption(call_cparams);
				BC_ASSERT_EQUAL(ccall_enc, encryption, int, "%d");
			}

			idx++;
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + 3,
		                             liblinphone_tester_sip_timeout));
		int focus_no_streams_running = 6;
		if (version_mismatch) focus_no_streams_running -= 2;
		int no_members = static_cast<int>(members.size());
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                  focus_stat.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running - no_members),
		                  liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running,
		                             liblinphone_tester_sip_timeout));
		// If ICE is enabled, the addition to a conference may go through a resume of the call
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
		                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(
		    coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		    focus_stat.number_of_LinphoneSubscriptionIncomingReceived + (no_members * nb_subscriptions), 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + (no_members * nb_subscriptions),
		                             5000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + no_members,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + no_members,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
		                             focus_stat.number_of_conference_participant_devices_present + no_members,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
		                             focus_stat.number_of_participant_devices_present + no_members,
		                             liblinphone_tester_sip_timeout));

		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
		    fill_member_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs, focus.getCMgr(),
		                            memberList, confAddr, enable_video);

		update_sequence_number(&participants_info, {}, 0, -1);
		for (auto mgr : members) {
			const int duration2 = (duration < 0) ? 0 : duration;
			check_conference_info_in_db(mgr, uid, confAddr, marie.getCMgr()->identity, participants_info, start_time,
			                            duration2, initialSubject, description, 0, LinphoneConferenceInfoStateNew,
			                            security_level, FALSE, TRUE, enable_video, enable_chat);
		}

		for (const auto &mgr : conferenceMgrs) {
			BC_ASSERT_TRUE(check_conference_info_by_participant(mgr, members, 0, confAddr));
		}

		LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		// wait to know if the no RTP timeout is triggered
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
		    .waitUntil(chrono::seconds(nortp_timeout + 1), [&marie, confAddr, nortp_timeout] {
			    LinphoneCall *marie_call = linphone_core_get_call_by_remote_address2(marie.getLc(), confAddr);
			    return marie_call && (linphone_call_get_duration(marie_call) > nortp_timeout);
		    });

		// Test invalid peer address
		BC_ASSERT_PTR_NULL(linphone_core_search_conference_by_identifier(
		    marie.getLc(), "==sip:toto@sip.conference.org##sip:me@sip.local.org"));
		// Test inexistent chat room identifier
		BC_ASSERT_PTR_NULL(linphone_core_search_conference_by_identifier(
		    marie.getLc(), "sip:toto@sip.conference.org##sip:me@sip.local.org"));

		for (auto mgr : conferenceMgrs) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				const char *conference_identifier = linphone_conference_get_identifier(pconference);
				LinphoneConference *found_pconference =
				    linphone_core_search_conference_by_identifier(mgr->lc, conference_identifier);
				BC_ASSERT_PTR_NOT_NULL(found_pconference);
				BC_ASSERT_PTR_EQUAL(pconference, found_pconference);

				const LinphoneConferenceParams *conference_params = linphone_conference_get_current_params(pconference);
				int no_participants = 0;
				if (start_time >= 0) {
					BC_ASSERT_EQUAL((long long)linphone_conference_params_get_start_time(conference_params),
					                (long long)start_time, long long, "%lld");
				}
				BC_ASSERT_EQUAL((long long)linphone_conference_params_get_end_time(conference_params),
				                (long long)end_time, long long, "%lld");

				BC_ASSERT_EQUAL((int)linphone_conference_params_get_security_level(conference_params),
				                (int)security_level, int, "%0d");
				if (!all_listeners) {
					bctbx_list_t *participant_device_list =
					    linphone_conference_get_participant_device_list(pconference);
					BC_ASSERT_EQUAL(bctbx_list_size(participant_device_list), members.size(), size_t, "%zu");
					if (!(mgr == focus.getCMgr() && security_level == LinphoneConferenceSecurityLevelEndToEnd)) {
						for (bctbx_list_t *d_it = participant_device_list; d_it; d_it = bctbx_list_next(d_it)) {
							LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(d_it);
							BC_ASSERT_PTR_NOT_NULL(d);
							if (d) {
								check_muted({focus, marie, pauline, laure, michelle, berthe}, d, {pauline.getCMgr()});
								linphone_participant_device_set_user_data(d, mgr->lc);
								LinphoneParticipantDeviceCbs *cbs =
								    linphone_factory_create_participant_device_cbs(linphone_factory_get());
								linphone_participant_device_cbs_set_is_muted(cbs, on_muted_notified);
								linphone_participant_device_add_callbacks(d, cbs);
								linphone_participant_device_cbs_unref(cbs);
							}
						}
					}
					bctbx_list_free_with_data(participant_device_list,
					                          (void (*)(void *))linphone_participant_device_unref);
				}

				if (mgr == focus.getCMgr()) {
					no_participants = static_cast<int>(members.size());
					BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
				} else {
					no_participants = static_cast<int>(members.size() - 1);
					BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
					LinphoneCall *current_call = linphone_core_get_current_call(mgr->lc);
					BC_ASSERT_PTR_NOT_NULL(current_call);
					const LinphoneCallParams *current_call_lparams = linphone_call_get_params(current_call);
					BC_ASSERT_PTR_NOT_NULL(current_call_lparams);
					if (current_call_lparams) {
						LinphoneConferenceLayout current_layout =
						    linphone_call_params_get_conference_video_layout(current_call_lparams);
						BC_ASSERT_EQUAL(current_layout, layout, int, "%i");
					}
					if (enable_video && video_direction == LinphoneMediaDirectionSendRecv &&
					    layout == LinphoneConferenceLayoutActiveSpeaker) {
						BC_ASSERT_PTR_NOT_NULL(linphone_call_params_get_used_video_payload_type(
						    linphone_call_get_current_params(current_call)));
					}
					if (current_call) {
						BC_ASSERT_EQUAL((int)linphone_call_get_state(current_call),
						                (int)LinphoneCallStateStreamsRunning, int, "%0d");
					}
					if (enable_ice) {
						if (video_direction == LinphoneMediaDirectionSendRecv) {
							LinphoneParticipantRole role =
							    linphone_participant_get_role(linphone_conference_get_me(pconference));
							if ((role != LinphoneParticipantRoleListener) && (layout != LinphoneConferenceLayoutGrid)) {
								BC_ASSERT_TRUE(check_ice(mgr, focus.getCMgr(), LinphoneIceStateHostConnection));
							}
						} else {
							LinphoneCall *c1, *c2;
							MSTimeSpec ts;

							c1 = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
							c2 = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
							BC_ASSERT_TRUE(linphone_call_get_microphone_muted(c1) == (mgr == pauline.getCMgr()));

							BC_ASSERT_PTR_NOT_NULL(c1);
							BC_ASSERT_PTR_NOT_NULL(c2);
							if (!c1 || !c2) {
								BC_ASSERT_EQUAL(
								    linphone_call_params_video_enabled(linphone_call_get_current_params(c1)),
								    linphone_call_params_video_enabled(linphone_call_get_current_params(c2)), int,
								    "%d");
								BC_ASSERT_EQUAL(
								    linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(c1)),
								    linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(c2)),
								    int, "%d");
								bool_t audio_enabled =
								    linphone_call_params_audio_enabled(linphone_call_get_current_params(c1));
								if (audio_enabled) {
									liblinphone_tester_clock_start(&ts);
									LinphoneCallStats *stats1 = NULL;
									LinphoneCallStats *stats2 = NULL;
									do {
										if ((c1 != NULL) && (c2 != NULL)) {
											stats1 = linphone_call_get_audio_stats(c1);
											stats2 = linphone_call_get_audio_stats(c2);
											if (linphone_call_stats_get_ice_state(stats1) ==
											        LinphoneIceStateHostConnection &&
											    linphone_call_stats_get_ice_state(stats2) ==
											        LinphoneIceStateHostConnection) {
												break;
											}
											linphone_core_iterate(mgr->lc);
											linphone_core_iterate(focus.getLc());
											linphone_call_stats_unref(stats1);
											linphone_call_stats_unref(stats2);
											stats1 = stats2 = NULL;
										}
										ms_usleep(20000);
									} while (!liblinphone_tester_clock_elapsed(&ts, liblinphone_tester_sip_timeout));
									if (stats1) linphone_call_stats_unref(stats1);
									if (stats2) linphone_call_stats_unref(stats2);
								}
							}
						}
					}

					const LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(mgr->lc);
					bool_t enabled = !!linphone_video_activation_policy_get_automatically_initiate(pol);

					size_t no_streams_audio = 0;
					size_t no_active_streams_video = 0;
					size_t no_streams_video = (enabled) ? 4 : 0;
					size_t no_streams_text = 0;
					bool_t video_negotiated = enabled;

					LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						no_streams_audio = compute_no_audio_streams(pcall, pconference);
						no_active_streams_video = compute_no_video_streams(enabled, pcall, pconference);
						video_negotiated = enabled && (no_active_streams_video > 0);
						if (!enable_ice) {
							_linphone_call_check_max_nb_streams(pcall, no_streams_audio, no_streams_video,
							                                    no_streams_text);
						}
						_linphone_call_check_nb_active_streams(pcall, no_streams_audio, no_active_streams_video,
						                                       no_streams_text);
						const LinphoneCallParams *call_lparams = linphone_call_get_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
						const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), video_negotiated, int, "%0d");
						const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), video_negotiated, int, "%0d");

						const LinphoneMediaDirection negotiated_video_direction =
						    linphone_call_params_get_video_direction(call_cparams);
						if (video_negotiated && layout == LinphoneConferenceLayoutGrid &&
						    ((negotiated_video_direction == LinphoneMediaDirectionSendOnly) ||
						     (negotiated_video_direction == LinphoneMediaDirectionSendRecv))) {
							MSVideoSize vsize = linphone_call_params_get_sent_video_size(call_cparams);
							BC_ASSERT_EQUAL(vsize.width, 640, int, "%d");
							BC_ASSERT_EQUAL(vsize.height, 480, int, "%d");
						}
					}
					LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
					BC_ASSERT_PTR_NOT_NULL(ccall);
					if (ccall) {
						_linphone_call_check_nb_active_streams(ccall, no_streams_audio, no_active_streams_video,
						                                       no_streams_text);
						if (!enable_ice) {
							_linphone_call_check_max_nb_streams(ccall, no_streams_audio, no_streams_video,
							                                    no_streams_text);
							const LinphoneCallParams *call_lparams = linphone_call_get_params(ccall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams),
							                (all_listeners ? 1 : video_negotiated), int, "%0d");
							const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(ccall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams),
							                (all_listeners ? enable_video : video_negotiated), int, "%0d");
						}
						const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), video_negotiated, int, "%0d");
					}
				}
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int, "%0d");
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
				check_conference_me(pconference, ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())));
				bctbx_list_t *participants = linphone_conference_get_participant_list(pconference);
				for (bctbx_list_t *itp = participants; itp; itp = bctbx_list_next(itp)) {
					LinphoneParticipant *p = (LinphoneParticipant *)bctbx_list_get_data(itp);
					BC_ASSERT_TRUE(
					    linphone_participant_is_admin(p) ==
					    linphone_address_weak_equal(linphone_participant_get_address(p), marie.getCMgr()->identity));
				}
				bctbx_list_free_with_data(participants, (void (*)(void *))linphone_participant_unref);

				if (mgr != focus.getCMgr()) {
					BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
					                   .waitUntil(chrono::seconds(10), [&fconference, &pconference] {
						                   return check_conference_ssrc(fconference, pconference);
					                   }));
				}
			}
		}

		if (!all_listeners) {
			ms_message("Marie mutes its microphone");
			LinphoneConference *marie_conference = linphone_core_search_conference_2(marie.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(marie_conference);
			if (marie_conference) {
				linphone_conference_set_microphone_muted(marie_conference, TRUE);
			}

			for (auto mgr : conferenceMgrs) {
				if (mgr != marie.getCMgr() &&
				    !(mgr == focus.getCMgr() && security_level == LinphoneConferenceSecurityLevelEndToEnd)) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneParticipantDeviceMuted, 1,
					                             liblinphone_tester_sip_timeout));
				}
				if (mgr != focus.getCMgr()) {
					LinphoneCall *c1 = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
					if (c1) {
						BC_ASSERT_TRUE(linphone_call_get_microphone_muted(c1) ==
						               ((mgr == pauline.getCMgr()) || (mgr == marie.getCMgr())));
					}
				}

				if (!(mgr == focus.getCMgr() && security_level == LinphoneConferenceSecurityLevelEndToEnd)) {
					LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
					if (pconference) {
						bctbx_list_t *participant_device_list =
						    linphone_conference_get_participant_device_list(pconference);
						for (bctbx_list_t *d_it = participant_device_list; d_it; d_it = bctbx_list_next(d_it)) {
							LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(d_it);
							BC_ASSERT_PTR_NOT_NULL(d);
							if (d) {
								check_muted({focus, marie, pauline, laure, michelle, berthe}, d,
								            {marie.getCMgr(), pauline.getCMgr()});
							}
						}
						bctbx_list_free_with_data(participant_device_list,
						                          (void (*)(void *))linphone_participant_device_unref);
					}
				}
			}
		}

		if (client_restart) {
			ms_message("Marie restarts its core");
			coresList = bctbx_list_remove(coresList, marie.getLc());
			marie.reStart();

			if (enable_video) {
				LinphoneVideoActivationPolicy *pol =
				    linphone_factory_create_video_activation_policy(linphone_factory_get());
				linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
				linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
				linphone_core_set_video_activation_policy(marie.getLc(), pol);
				linphone_video_activation_policy_unref(pol);

				linphone_core_enable_video_capture(marie.getLc(), TRUE);
				linphone_core_enable_video_display(marie.getLc(), TRUE);
			}

			linphone_core_set_default_conference_layout(marie.getLc(), layout);
			coresList = bctbx_list_append(coresList, marie.getLc());

			stats focus_stat2 = focus.getStats();
			stats marie_stat2 = marie.getStats();

			ms_message("%s is entering conference %s again", linphone_core_get_identity(marie.getLc()),
			           conference_address_str);
			LinphoneCallParams *new_params = linphone_core_create_call_params(marie.getLc(), nullptr);
			linphone_call_params_set_media_encryption(new_params, encryption);
			linphone_core_invite_address_with_params_2(marie.getLc(), confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallOutgoingProgress,
			                             marie_stat2.number_of_LinphoneCallOutgoingProgress + 1,
			                             liblinphone_tester_sip_timeout));
			int no_streams_running = ((enable_ice) ? 3 : 2);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating,
			                             marie_stat2.number_of_LinphoneCallUpdating + (no_streams_running - 1),
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
			                             marie_stat2.number_of_LinphoneCallStreamsRunning + no_streams_running,
			                             liblinphone_tester_sip_timeout));
			// Update to add to conference.
			// If ICE is enabled, the addition to a conference may go through a resume of the call
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated,
			                             marie_stat2.number_of_LinphoneConferenceStateCreated + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionOutgoingProgress,
			                             marie_stat2.number_of_LinphoneSubscriptionOutgoingProgress + nb_subscriptions,
			                             5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
			                             marie_stat2.number_of_LinphoneSubscriptionActive + nb_subscriptions, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyFullStateReceived,
			                             marie_stat2.number_of_NotifyFullStateReceived + 1,
			                             liblinphone_tester_sip_timeout));

			if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyEktReceived,
				                             marie_stat2.number_of_NotifyEktReceived + 1,
				                             liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
			                             focus_stat2.number_of_LinphoneCallIncomingReceived + 1,
			                             liblinphone_tester_sip_timeout));
			int focus_no_streams_running2 = ((enable_ice) ? 2 : 1);
			// Update to end ICE negotiations
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
			                  focus_stat2.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running2 - 1),
			                  liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
			                             focus_stat2.number_of_LinphoneCallStreamsRunning + focus_no_streams_running2,
			                             liblinphone_tester_sip_timeout));

			if ((encryption == LinphoneMediaEncryptionDTLS) || (encryption == LinphoneMediaEncryptionZRTP)) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallEncryptedOn,
				                             marie_stat2.number_of_LinphoneCallEncryptedOn + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEncryptedOn,
				                             focus_stat2.number_of_LinphoneCallEncryptedOn + 1,
				                             liblinphone_tester_sip_timeout));
			}

			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(marie.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
				const LinphoneMediaEncryption pcall_enc = linphone_call_params_get_media_encryption(call_cparams);
				BC_ASSERT_EQUAL(pcall_enc, encryption, int, "%d");
			}
			LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), marie.getCMgr()->identity);
			BC_ASSERT_PTR_NOT_NULL(ccall);
			if (ccall) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
				const LinphoneMediaEncryption ccall_enc = linphone_call_params_get_media_encryption(call_cparams);
				BC_ASSERT_EQUAL(ccall_enc, encryption, int, "%d");
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
			                             focus_stat2.number_of_LinphoneSubscriptionIncomingReceived + nb_subscriptions,
			                             5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
			                             focus_stat2.number_of_LinphoneSubscriptionActive + nb_subscriptions, 5000));

#ifdef HAVE_ADVANCED_IM
			if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
				// wait a bit longer to receive all EKT packets
				CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
					return false;
				});
				does_all_participants_have_matching_ekt(focus.getCMgr(), memberList, confAddr);
			}
#endif // HAVE_ADVANCED_IM

			// wait a bit longer to detect side effect if any, at least 4 second to be sure we do not miss any silent
			// pauline rtp packets
			CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(5), [] {
				return false;
			});

			for (auto mgr : conferenceMgrs) {
				const LinphoneAddress *local_address = (mgr == focus.getCMgr()) ? confAddr : mgr->identity;
				LinphoneConference *pconference =
				    linphone_core_search_conference(mgr->lc, NULL, local_address, confAddr, NULL);
				BC_ASSERT_PTR_NOT_NULL(pconference);
				if (pconference) {
					const LinphoneConferenceParams *conference_params =
					    linphone_conference_get_current_params(pconference);
					int no_participants = 0;
					if (start_time >= 0) {
						BC_ASSERT_EQUAL((long long)linphone_conference_params_get_start_time(conference_params),
						                (long long)start_time, long long, "%lld");
					}
					BC_ASSERT_EQUAL((long long)linphone_conference_params_get_end_time(conference_params),
					                (long long)end_time, long long, "%lld");
					BC_ASSERT_EQUAL((int)linphone_conference_params_get_security_level(conference_params),
					                (int)security_level, int, "%0d");

					if (!(mgr == focus.getCMgr() && security_level == LinphoneConferenceSecurityLevelEndToEnd)) {
						bctbx_list_t *participant_device_list =
						    linphone_conference_get_participant_device_list(pconference);
						for (bctbx_list_t *d_it = participant_device_list; d_it; d_it = bctbx_list_next(d_it)) {
							LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(d_it);
							BC_ASSERT_PTR_NOT_NULL(d);
							if (d) {
								check_muted({focus, marie, pauline, laure, michelle, berthe}, d, {pauline.getCMgr()});
							}
						}
						bctbx_list_free_with_data(participant_device_list,
						                          (void (*)(void *))linphone_participant_device_unref);
					}

					if (mgr == focus.getCMgr()) {
						no_participants = static_cast<int>(members.size());
						BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
					} else {

						no_participants = static_cast<int>(members.size() - 1);
						BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
						LinphoneCall *current_call = linphone_core_get_current_call(mgr->lc);
						BC_ASSERT_PTR_NOT_NULL(current_call);
						if (current_call) {
							BC_ASSERT_EQUAL((int)linphone_call_get_state(current_call),
							                (int)LinphoneCallStateStreamsRunning, int, "%0d");
						}
						if (enable_ice) {
							BC_ASSERT_TRUE(check_ice(mgr, focus.getCMgr(), LinphoneIceStateHostConnection));
						}

						const LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(mgr->lc);
						bool_t enabled = !!linphone_video_activation_policy_get_automatically_initiate(pol);

						size_t no_streams_audio = 0;
						size_t no_active_streams_video = 0;
						size_t no_streams_video = (enabled) ? 4 : 0;
						size_t no_streams_text = 0;

						LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
						BC_ASSERT_PTR_NOT_NULL(pcall);
						if (pcall) {
							no_streams_audio = compute_no_audio_streams(pcall, pconference);
							no_active_streams_video = compute_no_video_streams(enabled, pcall, pconference);
							_linphone_call_check_max_nb_streams(pcall, no_streams_audio, no_streams_video,
							                                    no_streams_text);
							_linphone_call_check_nb_active_streams(pcall, no_streams_audio, no_active_streams_video,
							                                       no_streams_text);
							const LinphoneCallParams *call_lparams = linphone_call_get_params(pcall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
							const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(pcall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
							const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
						}
						LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
						BC_ASSERT_PTR_NOT_NULL(ccall);
						if (ccall) {
							_linphone_call_check_max_nb_streams(ccall, no_streams_audio, no_streams_video,
							                                    no_streams_text);
							_linphone_call_check_nb_active_streams(ccall, no_streams_audio, no_active_streams_video,
							                                       no_streams_text);
							const LinphoneCallParams *call_lparams = linphone_call_get_params(ccall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
							const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(ccall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
							const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
						}
					}
					BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int,
					                "%0d");
					bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
					BC_ASSERT_EQUAL(bctbx_list_size(devices), 3, size_t, "%zu");
					if (devices) {
						bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
					}
					BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
					check_conference_me(pconference, ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())));
					bctbx_list_t *participants = linphone_conference_get_participant_list(pconference);
					for (bctbx_list_t *itp = participants; itp; itp = bctbx_list_next(itp)) {
						LinphoneParticipant *p = (LinphoneParticipant *)bctbx_list_get_data(itp);
						BC_ASSERT_TRUE(linphone_participant_is_admin(p) ==
						               linphone_address_weak_equal(linphone_participant_get_address(p),
						                                           marie.getCMgr()->identity));
					}
					bctbx_list_free_with_data(participants, (void (*)(void *))linphone_participant_unref);

					if (mgr != focus.getCMgr()) {
						BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
						                   .waitUntil(chrono::seconds(10), [&fconference, &pconference] {
							                   return check_conference_ssrc(fconference, pconference);
						                   }));
					}
				}
			}
		}

		// Wait a little bit
		wait_for_list(coresList, NULL, 0, 3000);

		if (network_restart) {
			ms_message("Marie toggles its network");
			stats focus_stat2 = focus.getStats();
			stats marie_stat = marie.getStats();
			linphone_core_set_network_reachable(marie.getLc(), FALSE);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionTerminated,
			                             marie_stat.number_of_LinphoneSubscriptionTerminated + nb_subscriptions,
			                             liblinphone_tester_sip_timeout));

			// Wait a little bit
			wait_for_list(coresList, NULL, 0, 1000);

			linphone_core_set_network_reachable(marie.getLc(), TRUE);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionOutgoingProgress,
			                             marie_stat.number_of_LinphoneSubscriptionOutgoingProgress + nb_subscriptions,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
			                             focus_stat2.number_of_LinphoneSubscriptionIncomingReceived + nb_subscriptions,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
			                             marie_stat.number_of_LinphoneSubscriptionActive + nb_subscriptions,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
			                             focus_stat2.number_of_LinphoneSubscriptionActive + nb_subscriptions,
			                             liblinphone_tester_sip_timeout));

#ifdef HAVE_ADVANCED_IM
			if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
				does_all_participants_have_matching_ekt(focus.getCMgr(), memberList, confAddr);
			}
#endif // HAVE_ADVANCED_IM
		}

		if (enable_video) {
			LinphoneCall *pauline_call = linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(pauline_call);
			bool_t enable = FALSE;
			if (pauline_call) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pauline_call);
				const LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(pauline.getLc());
				enable = !(!!linphone_video_activation_policy_get_automatically_initiate(pol) &&
				           !!linphone_call_params_video_enabled(call_cparams));
			}

			Address paulineAddr = pauline.getIdentity();
			LinphoneCall *focus_call = linphone_core_get_call_by_remote_address2(focus.getLc(), paulineAddr.toC());
			BC_ASSERT_PTR_NOT_NULL(focus_call);

			LinphoneConference *paulineConference =
			    linphone_core_search_conference(pauline.getLc(), NULL, paulineAddr.toC(), confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(paulineConference);

			for (int i = 0; i < 4; i++) {
				stats focus_stat2 = focus.getStats();
				stats marie_stat2 = marie.getStats();
				stats pauline_stat2 = pauline.getStats();
				stats laure_stat2 = laure.getStats();

				try {
					const LinphoneParticipantInfo *paulineInfo = participantList.at(pauline.getCMgr());
					const LinphoneParticipantRole paulineRole = linphone_participant_info_get_role(paulineInfo);
					bool isPaulineListener = (paulineRole == LinphoneParticipantRoleListener);
					LinphoneMediaDirection new_video_direction = video_direction;
					if ((video_direction == LinphoneMediaDirectionRecvOnly) &&
					    (layout == LinphoneConferenceLayoutGrid)) {
						new_video_direction = LinphoneMediaDirectionSendRecv;
					}

					ms_message("%s %s video with direction %s", linphone_core_get_identity(pauline.getLc()),
					           (enable ? "enables" : "disables"),
					           linphone_media_direction_to_string(new_video_direction));

					if (pauline_call) {
						LinphoneCallParams *new_params =
						    linphone_core_create_call_params(pauline.getLc(), pauline_call);
						linphone_call_params_enable_video(new_params, enable);
						linphone_call_params_set_video_direction(new_params, new_video_direction);
						linphone_call_update(pauline_call, new_params);
						linphone_call_params_unref(new_params);

						BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallUpdating,
						                             pauline_stat2.number_of_LinphoneCallUpdating + 1,
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(
						    coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning,
						    pauline_stat2.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
						                             focus_stat2.number_of_LinphoneCallUpdatedByRemote + 1,
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
						                             focus_stat2.number_of_LinphoneCallStreamsRunning + 1,
						                             liblinphone_tester_sip_timeout));

						if (isPaulineListener && (layout == LinphoneConferenceLayoutGrid)) {
							BC_ASSERT_FALSE(wait_for_list(
							    coresList, &focus.getStats().number_of_participant_devices_media_capability_changed,
							    focus_stat2.number_of_participant_devices_media_capability_changed + 1, 1000));
							BC_ASSERT_FALSE(wait_for_list(
							    coresList, &marie.getStats().number_of_participant_devices_media_capability_changed,
							    marie_stat2.number_of_participant_devices_media_capability_changed + 1, 1000));
							BC_ASSERT_FALSE(wait_for_list(
							    coresList, &pauline.getStats().number_of_participant_devices_media_capability_changed,
							    pauline_stat2.number_of_participant_devices_media_capability_changed + 1, 1000));
							BC_ASSERT_FALSE(wait_for_list(
							    coresList, &laure.getStats().number_of_participant_devices_media_capability_changed,
							    laure_stat2.number_of_participant_devices_media_capability_changed + 1, 1000));
						} else {
							if (new_video_direction == LinphoneMediaDirectionSendRecv) {
								BC_ASSERT_TRUE(wait_for_list(
								    coresList, &marie.getStats().number_of_LinphoneCallUpdating,
								    marie_stat2.number_of_LinphoneCallUpdating + 1, liblinphone_tester_sip_timeout));
								BC_ASSERT_TRUE(wait_for_list(coresList,
								                             &marie.getStats().number_of_LinphoneCallStreamsRunning,
								                             marie_stat2.number_of_LinphoneCallStreamsRunning + 1,
								                             liblinphone_tester_sip_timeout));
								BC_ASSERT_TRUE(wait_for_list(
								    coresList, &laure.getStats().number_of_LinphoneCallUpdating,
								    laure_stat2.number_of_LinphoneCallUpdating + 1, liblinphone_tester_sip_timeout));
								BC_ASSERT_TRUE(wait_for_list(coresList,
								                             &laure.getStats().number_of_LinphoneCallStreamsRunning,
								                             laure_stat2.number_of_LinphoneCallStreamsRunning + 1,
								                             liblinphone_tester_sip_timeout));
								BC_ASSERT_TRUE(wait_for_list(coresList,
								                             &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
								                             focus_stat2.number_of_LinphoneCallUpdatedByRemote + 3,
								                             liblinphone_tester_sip_timeout));
								BC_ASSERT_TRUE(wait_for_list(coresList,
								                             &focus.getStats().number_of_LinphoneCallStreamsRunning,
								                             focus_stat2.number_of_LinphoneCallStreamsRunning + 3,
								                             liblinphone_tester_sip_timeout));
							}

							BC_ASSERT_TRUE(wait_for_list(
							    coresList, &focus.getStats().number_of_participant_devices_media_capability_changed,
							    focus_stat2.number_of_participant_devices_media_capability_changed + 1,
							    liblinphone_tester_sip_timeout));
							BC_ASSERT_TRUE(wait_for_list(
							    coresList, &marie.getStats().number_of_participant_devices_media_capability_changed,
							    marie_stat2.number_of_participant_devices_media_capability_changed + 1,
							    liblinphone_tester_sip_timeout));
							BC_ASSERT_TRUE(wait_for_list(
							    coresList, &pauline.getStats().number_of_participant_devices_media_capability_changed,
							    pauline_stat2.number_of_participant_devices_media_capability_changed + 1,
							    liblinphone_tester_sip_timeout));
							BC_ASSERT_TRUE(wait_for_list(
							    coresList, &laure.getStats().number_of_participant_devices_media_capability_changed,
							    laure_stat2.number_of_participant_devices_media_capability_changed + 1,
							    liblinphone_tester_sip_timeout));
						}

						BC_ASSERT_EQUAL(focus.getStats().number_of_participants_added,
						                focus_stat2.number_of_participants_added, int, "%0d");
						BC_ASSERT_EQUAL(focus.getStats().number_of_participant_devices_added,
						                focus_stat2.number_of_participant_devices_added, int, "%0d");
						BC_ASSERT_EQUAL(focus.getStats().number_of_conference_participant_devices_present,
						                focus_stat2.number_of_conference_participant_devices_present, int, "%0d");
						BC_ASSERT_EQUAL(focus.getStats().number_of_participant_devices_present,
						                focus_stat2.number_of_participant_devices_present, int, "%0d");
						BC_ASSERT_EQUAL(marie.getStats().number_of_participants_added,
						                marie_stat2.number_of_participants_added, int, "%0d");
						BC_ASSERT_EQUAL(marie.getStats().number_of_participant_devices_added,
						                marie_stat2.number_of_participant_devices_added, int, "%0d");
						BC_ASSERT_EQUAL(marie.getStats().number_of_conference_participant_devices_present,
						                marie_stat2.number_of_conference_participant_devices_present, int, "%0d");
						BC_ASSERT_EQUAL(marie.getStats().number_of_participant_devices_present,
						                marie_stat2.number_of_participant_devices_present, int, "%0d");
						BC_ASSERT_EQUAL(pauline.getStats().number_of_participants_added,
						                pauline_stat2.number_of_participants_added, int, "%0d");
						BC_ASSERT_EQUAL(pauline.getStats().number_of_participant_devices_added,
						                pauline_stat2.number_of_participant_devices_added, int, "%0d");
						BC_ASSERT_EQUAL(pauline.getStats().number_of_conference_participant_devices_present,
						                pauline_stat2.number_of_conference_participant_devices_present, int, "%0d");
						BC_ASSERT_EQUAL(pauline.getStats().number_of_participant_devices_present,
						                pauline_stat2.number_of_participant_devices_present, int, "%0d");
						BC_ASSERT_EQUAL(laure.getStats().number_of_participants_added,
						                laure_stat2.number_of_participants_added, int, "%0d");
						BC_ASSERT_EQUAL(laure.getStats().number_of_participant_devices_added,
						                laure_stat2.number_of_participant_devices_added, int, "%0d");
						BC_ASSERT_EQUAL(laure.getStats().number_of_conference_participant_devices_present,
						                laure_stat2.number_of_conference_participant_devices_present, int, "%0d");
						BC_ASSERT_EQUAL(laure.getStats().number_of_participant_devices_present,
						                laure_stat2.number_of_participant_devices_present, int, "%0d");

						bool pauline_expected_thumbnails_requested =
						    are_participants_camera_streams_requested(pauline.getCMgr(), paulineConference);
						for (auto mgr : conferenceMgrs) {
							const LinphoneAddress *local_address = (mgr == focus.getCMgr()) ? confAddr : mgr->identity;
							LinphoneConference *pconference =
							    linphone_core_search_conference(mgr->lc, NULL, local_address, confAddr, NULL);
							BC_ASSERT_PTR_NOT_NULL(pconference);
							if (pconference) {
								LinphoneParticipant *p = (mgr == pauline.getCMgr())
								                             ? linphone_conference_get_me(pconference)
								                             : linphone_conference_find_participant(
								                                   pconference, pauline.getCMgr()->identity);
								BC_ASSERT_PTR_NOT_NULL(p);
								if (p) {
									bctbx_list_t *devices = linphone_participant_get_devices(p);
									for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
										LinphoneParticipantDevice *d =
										    (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
										LinphoneMediaDirection expected_video_direction = video_direction;
										if (enable == TRUE) {
											if (all_listeners) {
												if (layout == LinphoneConferenceLayoutGrid) {
													expected_video_direction = LinphoneMediaDirectionInactive;
												} else {
													expected_video_direction = LinphoneMediaDirectionRecvOnly;
												}
											} else if ((!pauline_expected_thumbnails_requested ||
											            (video_direction == LinphoneMediaDirectionRecvOnly)) &&
											           (layout == LinphoneConferenceLayoutGrid)) {
												expected_video_direction = LinphoneMediaDirectionSendOnly;
											} else {
												expected_video_direction = video_direction;
											}
										} else {
											expected_video_direction = LinphoneMediaDirectionInactive;
										}
										BC_ASSERT_EQUAL(linphone_participant_device_get_stream_capability(
										                    d, LinphoneStreamTypeVideo),
										                expected_video_direction, int, "%0d");
									}
									if (devices) {
										bctbx_list_free_with_data(devices,
										                          (void (*)(void *))linphone_participant_device_unref);
									}
								}
							}
						}

						int negotiated_video_enabled =
						    (all_listeners && (layout == LinphoneConferenceLayoutGrid)) ? 0 : enable;
						pauline_call = linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
						BC_ASSERT_PTR_NOT_NULL(pauline_call);
						if (pauline_call) {
							const LinphoneCallParams *call_lparams = linphone_call_get_params(pauline_call);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enable, int, "%0d");
							const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(pauline_call);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), negotiated_video_enabled,
							                int, "%0d");
							const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pauline_call);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), negotiated_video_enabled,
							                int, "%0d");
						}
						focus_call = linphone_core_get_call_by_remote_address2(focus.getLc(), paulineAddr.toC());
						BC_ASSERT_PTR_NOT_NULL(focus_call);
						if (focus_call) {
							const LinphoneCallParams *call_lparams = linphone_call_get_params(focus_call);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enable, int, "%0d");
							const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(focus_call);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enable, int, "%0d");
							const LinphoneCallParams *call_cparams = linphone_call_get_current_params(focus_call);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), negotiated_video_enabled,
							                int, "%0d");
						}
					}

					paulineConference =
					    linphone_core_search_conference(pauline.getLc(), NULL, paulineAddr.toC(), confAddr, NULL);
					BC_ASSERT_PTR_NOT_NULL(paulineConference);
					if (paulineConference) {
						bctbx_list_t *devices = linphone_conference_get_participant_device_list(paulineConference);
						for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
							LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
							bool video_available =
							    !!linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo);
							if (enable) {
								LinphoneMediaDirection video_direction =
								    linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo);
								BC_ASSERT_TRUE(video_available ==
								               (((video_direction == LinphoneMediaDirectionSendOnly) ||
								                 (video_direction == LinphoneMediaDirectionSendRecv))));
							} else {
								BC_ASSERT_FALSE(video_available);
							}
						}
						if (devices) {
							bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
						}

						stats focus_stat3 = focus.getStats();
						stats marie_stat3 = marie.getStats();
						stats pauline_stat3 = pauline.getStats();
						stats laure_stat3 = laure.getStats();

						LinphoneCall *pauline_call =
						    linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
						BC_ASSERT_PTR_NOT_NULL(pauline_call);

						bool_t video_enabled = FALSE;
						if (pauline_call) {
							const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pauline_call);
							video_enabled = linphone_call_params_video_enabled(call_cparams);
						}

						linphone_conference_leave(paulineConference);

						BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallPausing,
						                             pauline_stat3.number_of_LinphoneCallPausing + 1,
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallPaused,
						                             pauline_stat3.number_of_LinphoneCallPaused + 1,
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallPausedByRemote,
						                             focus_stat3.number_of_LinphoneCallPausedByRemote + 1,
						                             liblinphone_tester_sip_timeout));

						BC_ASSERT_TRUE(wait_for_list(coresList,
						                             &focus.getStats().number_of_conference_participant_devices_on_hold,
						                             focus_stat3.number_of_conference_participant_devices_on_hold + 1,
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_on_hold,
						                             focus_stat3.number_of_participant_devices_on_hold + 1,
						                             liblinphone_tester_sip_timeout));

						BC_ASSERT_TRUE(wait_for_list(coresList,
						                             &laure.getStats().number_of_conference_participant_devices_on_hold,
						                             laure_stat3.number_of_conference_participant_devices_on_hold + 1,
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_on_hold,
						                             laure_stat3.number_of_participant_devices_on_hold + 1,
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(
						    coresList, &laure.getStats().number_of_participant_devices_media_capability_changed,
						    laure_stat3.number_of_participant_devices_media_capability_changed + 1,
						    liblinphone_tester_sip_timeout));

						BC_ASSERT_TRUE(wait_for_list(coresList,
						                             &marie.getStats().number_of_conference_participant_devices_on_hold,
						                             marie_stat3.number_of_conference_participant_devices_on_hold + 1,
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_on_hold,
						                             marie_stat3.number_of_participant_devices_on_hold + 1,
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(
						    coresList, &marie.getStats().number_of_participant_devices_media_capability_changed,
						    marie_stat3.number_of_participant_devices_media_capability_changed + 1,
						    liblinphone_tester_sip_timeout));

						for (auto mgr : conferenceMgrs) {
							const LinphoneAddress *local_address = (mgr == focus.getCMgr()) ? confAddr : mgr->identity;
							LinphoneConference *pconference =
							    linphone_core_search_conference(mgr->lc, NULL, local_address, confAddr, NULL);
							BC_ASSERT_PTR_NOT_NULL(pconference);
							if (pconference) {
								if (mgr == pauline.getCMgr()) {
									BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
								} else {
									LinphoneParticipant *participant =
									    linphone_conference_find_participant(pconference, pauline.getCMgr()->identity);
									BC_ASSERT_PTR_NOT_NULL(participant);
									if (participant) {
										bctbx_list_t *devices = linphone_participant_get_devices(participant);
										for (bctbx_list_t *it_d = devices; it_d != NULL; it_d = it_d->next) {
											LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)it_d->data;
											BC_ASSERT_PTR_NOT_NULL(d);
											if (d) {
												BC_ASSERT_EQUAL(linphone_participant_device_get_state(d),
												                LinphoneParticipantDeviceStateOnHold, int, "%0d");
											}
										}
										bctbx_list_free_with_data(devices,
										                          (void (*)(void *))linphone_participant_device_unref);
									}
								}
							}
						}

						linphone_conference_enter(paulineConference);

						int participant_streams_running = 0;
						int pauline_streams_running = 0;
						int focus_streams_running = 0;
						if (isPaulineListener) {
							participant_streams_running = 0;
							pauline_streams_running = 1;
							focus_streams_running = 1;
						} else if (video_direction == LinphoneMediaDirectionRecvOnly) {
							if (layout == LinphoneConferenceLayoutGrid) {
								/*
								 * If the participant video direction is set to RecvOnly, the conference
								 * server will see it as if everybody had disabled the video streams. The
								 * test explicitely changes Pauline's video direction to SendRecv to trigger
								 * events such as media capability and availability changed Leaving and
								 * rejoining a conference, therefore, triggers media events on participant
								 * devices only when Pauline enables video capabilities with direction
								 * SendRecv
								 */
								if (enable) {
									participant_streams_running = 1;
									focus_streams_running = static_cast<int>(members.size());
									pauline_streams_running = 1;
								} else {
									focus_streams_running = 1;
									pauline_streams_running = 1;
								}
							} else if (layout == LinphoneConferenceLayoutActiveSpeaker) {
								focus_streams_running = 1;
								pauline_streams_running = 1;
							}
						} else {
							participant_streams_running = ((enable) ? 1 : 0);
							focus_streams_running = static_cast<int>((enable) ? (members.size()) : 1);
							pauline_streams_running = 1;
						}

						BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallResuming,
						                             pauline_stat3.number_of_LinphoneCallResuming + 1,
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(
						    coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning,
						    (pauline_stat3.number_of_LinphoneCallStreamsRunning + pauline_streams_running),
						    liblinphone_tester_sip_timeout));
						// 2 streams running for Pauline and one for each participant
						BC_ASSERT_TRUE(
						    wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
						                  focus_stat3.number_of_LinphoneCallStreamsRunning + focus_streams_running,
						                  liblinphone_tester_sip_timeout));

						BC_ASSERT_TRUE(wait_for_list(coresList,
						                             &focus.getStats().number_of_conference_participant_devices_present,
						                             focus_stat3.number_of_conference_participant_devices_present + 1,
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
						                             focus_stat3.number_of_participant_devices_present + 1,
						                             liblinphone_tester_sip_timeout));

						BC_ASSERT_TRUE(wait_for_list(coresList,
						                             &laure.getStats().number_of_conference_participant_devices_present,
						                             laure_stat3.number_of_conference_participant_devices_present + 1,
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_present,
						                             laure_stat3.number_of_participant_devices_present + 1,
						                             liblinphone_tester_sip_timeout));

						BC_ASSERT_TRUE(wait_for_list(
						    coresList, &laure.getStats().number_of_participant_devices_media_capability_changed,
						    laure_stat3.number_of_participant_devices_media_capability_changed + 2,
						    liblinphone_tester_sip_timeout));

						BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallStreamsRunning,
						                             laure_stat3.number_of_LinphoneCallStreamsRunning +
						                                 participant_streams_running,
						                             liblinphone_tester_sip_timeout));

						BC_ASSERT_TRUE(wait_for_list(coresList,
						                             &marie.getStats().number_of_conference_participant_devices_present,
						                             marie_stat3.number_of_conference_participant_devices_present + 1,
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_present,
						                             marie_stat3.number_of_participant_devices_present + 1,
						                             liblinphone_tester_sip_timeout));

						BC_ASSERT_TRUE(wait_for_list(
						    coresList, &marie.getStats().number_of_participant_devices_media_capability_changed,
						    marie_stat3.number_of_participant_devices_media_capability_changed + 2,
						    liblinphone_tester_sip_timeout));

						BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
						                             marie_stat3.number_of_LinphoneCallStreamsRunning +
						                                 participant_streams_running,
						                             liblinphone_tester_sip_timeout));

						if (pauline_call) {
							const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pauline_call);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), video_enabled, int,
							                "%0d");
						}

						for (auto mgr : conferenceMgrs) {
							const LinphoneAddress *local_address = (mgr == focus.getCMgr()) ? confAddr : mgr->identity;
							LinphoneConference *pconference =
							    linphone_core_search_conference(mgr->lc, NULL, local_address, confAddr, NULL);
							BC_ASSERT_PTR_NOT_NULL(pconference);
							if (pconference) {
								if (mgr == pauline.getCMgr()) {
									BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
								} else {
									LinphoneParticipant *participant =
									    linphone_conference_find_participant(pconference, pauline.getCMgr()->identity);
									BC_ASSERT_PTR_NOT_NULL(participant);
									if (participant) {
										bctbx_list_t *devices = linphone_participant_get_devices(participant);
										for (bctbx_list_t *it_d = devices; it_d != NULL; it_d = it_d->next) {
											LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)it_d->data;
											BC_ASSERT_PTR_NOT_NULL(d);
											if (d) {
												BC_ASSERT_EQUAL(linphone_participant_device_get_state(d),
												                LinphoneParticipantDeviceStatePresent, int, "%0d");
											}
										}
										bctbx_list_free_with_data(devices,
										                          (void (*)(void *))linphone_participant_device_unref);
									}
								}
							}
						}
					}
					// Wait a little bit
					wait_for_list(coresList, NULL, 0, 1000);

					enable = !enable;
				} catch (std::out_of_range &) {
					BC_FAIL("Participant Pauline has not been found in the list of participants");
				}
			}

			stats marie_stat3 = marie.getStats();
			stats focus_stat3 = focus.getStats();
			stats pauline_stat3 = pauline.getStats();
			stats laure_stat3 = laure.getStats();

			const LinphoneAddress *laureUri = laure.getCMgr()->identity;
			LinphoneConference *laureConference =
			    linphone_core_search_conference(laure.getLc(), NULL, laureUri, confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(laureConference);

			LinphoneCall *laure_call = linphone_core_get_call_by_remote_address2(laure.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(laure_call);

			// Laure is a listener, therefore we expect the send component to be rejected
			LinphoneMediaDirection laure_video_direction = LinphoneMediaDirectionSendRecv;
			ms_message("%s enables video with direction %s", linphone_core_get_identity(laure.getLc()),
			           linphone_media_direction_to_string(laure_video_direction));

			if (laure_call) {
				LinphoneCallParams *new_params = linphone_core_create_call_params(laure.getLc(), laure_call);
				linphone_call_params_enable_video(new_params, TRUE);
				linphone_call_params_set_video_direction(new_params, laure_video_direction);
				linphone_call_update(laure_call, new_params);
				linphone_call_params_unref(new_params);

				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallUpdating,
				                             laure_stat3.number_of_LinphoneCallUpdating + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallStreamsRunning,
				                             laure_stat3.number_of_LinphoneCallStreamsRunning + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
				                             focus_stat3.number_of_LinphoneCallUpdatedByRemote + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
				                             focus_stat3.number_of_LinphoneCallStreamsRunning + 1,
				                             liblinphone_tester_sip_timeout));

				// If video direction is RecvOnly for all participants, then laure attempt to enable its video with
				// SendRecv capabilities will not bring about any changes as she is a listener. Its send component will
				// be always rejected as she is a listener Its recv component will only be accepted if at least one
				// participant streams out its video stream causing a change in its capabilities
				if ((video_direction == LinphoneMediaDirectionSendOnly) ||
				    (video_direction == LinphoneMediaDirectionInactive)) {
					BC_ASSERT_TRUE(wait_for_list(
					    coresList, &focus.getStats().number_of_participant_devices_media_capability_changed,
					    focus_stat3.number_of_participant_devices_media_capability_changed + 1,
					    liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(
					    coresList, &marie.getStats().number_of_participant_devices_media_capability_changed,
					    marie_stat3.number_of_participant_devices_media_capability_changed + 1,
					    liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(
					    coresList, &pauline.getStats().number_of_participant_devices_media_capability_changed,
					    pauline_stat3.number_of_participant_devices_media_capability_changed + 1,
					    liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(
					    coresList, &laure.getStats().number_of_participant_devices_media_capability_changed,
					    laure_stat3.number_of_participant_devices_media_capability_changed + 1,
					    liblinphone_tester_sip_timeout));
				} else {
					BC_ASSERT_FALSE(wait_for_list(
					    coresList, &focus.getStats().number_of_participant_devices_media_capability_changed,
					    focus_stat3.number_of_participant_devices_media_capability_changed + 1, 1000));
					BC_ASSERT_FALSE(wait_for_list(
					    coresList, &marie.getStats().number_of_participant_devices_media_capability_changed,
					    marie_stat3.number_of_participant_devices_media_capability_changed + 1, 1000));
					BC_ASSERT_FALSE(wait_for_list(
					    coresList, &pauline.getStats().number_of_participant_devices_media_capability_changed,
					    pauline_stat3.number_of_participant_devices_media_capability_changed + 1, 1000));
					BC_ASSERT_FALSE(wait_for_list(
					    coresList, &laure.getStats().number_of_participant_devices_media_capability_changed,
					    laure_stat3.number_of_participant_devices_media_capability_changed + 1, 1000));
				}
			}

			for (auto mgr : conferenceMgrs) {
				const LinphoneAddress *local_address = (mgr == focus.getCMgr()) ? confAddr : mgr->identity;
				LinphoneConference *pconference =
				    linphone_core_search_conference(mgr->lc, NULL, local_address, confAddr, NULL);
				BC_ASSERT_PTR_NOT_NULL(pconference);
				if (pconference) {
					auto info = linphone_conference_get_info(pconference);
					check_conference_info_against_db(mgr, confAddr, info, FALSE);
					LinphoneParticipant *p =
					    (mgr == laure.getCMgr())
					        ? linphone_conference_get_me(pconference)
					        : linphone_conference_find_participant(pconference, laure.getCMgr()->identity);
					BC_ASSERT_PTR_NOT_NULL(p);
					if (p) {
						bctbx_list_t *devices = linphone_participant_get_devices(p);
						for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
							LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
							LinphoneMediaDirection expected_video_direction = LinphoneMediaDirectionInactive;
							if (all_speakers) {
								expected_video_direction = LinphoneMediaDirectionSendRecv;
							} else {
								if ((all_listeners || (video_direction == LinphoneMediaDirectionInactive) ||
								     (video_direction == LinphoneMediaDirectionRecvOnly)) &&
								    (layout == LinphoneConferenceLayoutGrid)) {
									expected_video_direction = LinphoneMediaDirectionInactive;
								} else {
									expected_video_direction = LinphoneMediaDirectionRecvOnly;
								}
							}
							BC_ASSERT_EQUAL(
							    linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo),
							    expected_video_direction, int, "%0d");
						}
						if (devices) {
							bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
						}
					}
				}
			}
		}

		std::list<LinphoneCoreManager *> extraParticipantMgrs;
		int no_local_participants = 3;
		if (uninvited_participant_dials) {
			if (end_time > 0) {
				for (auto mgr : conferenceMgrs) {
					LinphoneConferenceInfo *info =
					    linphone_core_find_conference_information_from_uri(mgr->lc, confAddr);
					if (BC_ASSERT_PTR_NOT_NULL(info)) {
						linphone_conference_info_unref(info);
					}
				}
				time_t now = ms_time(NULL);
				time_t time_left = end_time - now + linphone_core_get_conference_cleanup_period(focus.getLc());
				// Verify that a participant can be added after the conference ends if the conference is active
				if (add_participant_after_end && (time_left > 0)) {
					// wait for the conference to end
					CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
					    .waitUntil(chrono::seconds((time_left + 1)), [] { return false; });
					for (auto mgr : conferenceMgrs) {
						LinphoneConferenceInfo *info =
						    linphone_core_find_conference_information_from_uri(mgr->lc, confAddr);
						if (BC_ASSERT_PTR_NOT_NULL(info)) {
							linphone_conference_info_unref(info);
						}
					}
				}
			}
			stats marie_stat2 = marie.getStats();
			stats focus_stat2 = focus.getStats();
			stats pauline_stat2 = pauline.getStats();
			stats laure_stat2 = laure.getStats();

			extraParticipantMgrs.push_back(michelle.getCMgr());

			linphone_core_set_nortp_timeout(michelle.getLc(), nortp_timeout);

			ms_message("%s is entering conference %s", linphone_core_get_identity(michelle.getLc()),
			           conference_address_str);

			LinphoneCallParams *params = linphone_core_create_call_params(michelle.getLc(), nullptr);
			linphone_call_params_enable_video(params, FALSE);
			LinphoneCall *michelle_call = linphone_core_invite_address_with_params(michelle.getLc(), confAddr, params);
			BC_ASSERT_PTR_NOT_NULL(michelle_call);
			linphone_call_params_unref(params);
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallOutgoingProgress, 1,
			                             liblinphone_tester_sip_timeout));
			int extra_participants = 1;
			if (participant_list_type == LinphoneConferenceParticipantListTypeOpen) {

				if (network_restart) {
					ms_message("%s switches off network before %s is added to conference %s",
					           linphone_core_get_identity(marie.getLc()), linphone_core_get_identity(michelle.getLc()),
					           conference_address_str);
					linphone_core_set_network_reachable(marie.getLc(), FALSE);
					BC_ASSERT_TRUE(
					    wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionTerminated,
					                  marie_stat2.number_of_LinphoneSubscriptionTerminated + nb_subscriptions,
					                  liblinphone_tester_sip_timeout));
				}

				conferenceMgrs.push_back(michelle.getCMgr());
				allConferenceMgrs.push_back(michelle.getCMgr());
				members.push_back(michelle.getCMgr());
				participantList.insert(std::make_pair(
				    michelle.getCMgr(), add_participant_info_to_list(&participants_info, michelle.getCMgr()->identity,
				                                                     (listenerAllowed) ? LinphoneParticipantRoleListener
				                                                                       : LinphoneParticipantRoleSpeaker,
				                                                     -1)));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallStreamsRunning, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateCreated,
				                             1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallUpdating, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallStreamsRunning, 2,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList,
				                             &michelle.getStats().number_of_LinphoneSubscriptionOutgoingProgress,
				                             nb_subscriptions, 5000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneSubscriptionActive,
				                             nb_subscriptions, 5000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_conference_full_state_received,
				                             1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(
				    wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
				                  focus_stat2.number_of_LinphoneSubscriptionIncomingReceived + nb_subscriptions,
				                  liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
				                             focus_stat2.number_of_LinphoneSubscriptionActive + nb_subscriptions,
				                             liblinphone_tester_sip_timeout));

				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
				                             focus_stat2.number_of_participants_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
				                             focus_stat2.number_of_participant_devices_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(
				    coresList, &focus.getStats().number_of_conference_participant_devices_present,
				    focus_stat2.number_of_conference_participant_devices_present + 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
				                             focus_stat2.number_of_participant_devices_present + 1,
				                             liblinphone_tester_sip_timeout));

				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_added,
				                             pauline_stat2.number_of_participants_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_added,
				                             pauline_stat2.number_of_participant_devices_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList,
				                             &pauline.getStats().number_of_conference_participant_devices_present,
				                             pauline_stat2.number_of_conference_participant_devices_present + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_present,
				                             pauline_stat2.number_of_participant_devices_present + 1,
				                             liblinphone_tester_sip_timeout));

				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participants_added,
				                             laure_stat2.number_of_participants_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_added,
				                             laure_stat2.number_of_participant_devices_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(
				    coresList, &laure.getStats().number_of_conference_participant_devices_present,
				    laure_stat2.number_of_conference_participant_devices_present + 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_present,
				                             laure_stat2.number_of_participant_devices_present + 1,
				                             liblinphone_tester_sip_timeout));

				if (network_restart) {
					ms_message("%s is back online after %s is added to conference %s",
					           linphone_core_get_identity(marie.getLc()), linphone_core_get_identity(michelle.getLc()),
					           conference_address_str);
					linphone_core_set_network_reachable(marie.getLc(), TRUE);
					BC_ASSERT_TRUE(
					    wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionOutgoingProgress,
					                  marie_stat2.number_of_LinphoneSubscriptionOutgoingProgress + nb_subscriptions,
					                  liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(
					    coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
					    focus_stat2.number_of_LinphoneSubscriptionIncomingReceived + (2 * nb_subscriptions),
					    liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
					                             marie_stat2.number_of_LinphoneSubscriptionActive + nb_subscriptions,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_conference_full_state_received,
					                             marie_stat2.number_of_conference_full_state_received + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
					                             focus_stat2.number_of_LinphoneSubscriptionActive + 2,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(
					    wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
					                  focus_stat2.number_of_LinphoneSubscriptionActive + (2 * nb_subscriptions),
					                  liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating,
					                             marie_stat2.number_of_LinphoneCallUpdating + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
					                             marie_stat2.number_of_LinphoneCallStreamsRunning + 2,
					                             liblinphone_tester_sip_timeout));
				}

				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_added,
				                             marie_stat2.number_of_participants_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_added,
				                             marie_stat2.number_of_participant_devices_added + 1,
				                             liblinphone_tester_sip_timeout));
				if (!network_restart) {
					BC_ASSERT_TRUE(wait_for_list(coresList,
					                             &marie.getStats().number_of_conference_participant_devices_present,
					                             marie_stat2.number_of_conference_participant_devices_present + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_present,
					                             marie_stat2.number_of_participant_devices_present + 1,
					                             liblinphone_tester_sip_timeout));
				}

				std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
				    fill_member_list(members, participantList, marie.getCMgr(), participants_info);
				wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs,
				                            focus.getCMgr(), memberList, confAddr, enable_video);

				for (auto mgr : members) {
					const int duration2 = (duration < 0) ? 0 : duration;
					const char *description2 = (mgr == michelle.getCMgr()) ? NULL : description;
					const char *uid2 = (mgr == michelle.getCMgr()) ? NULL : uid;
					if (mgr == michelle.getCMgr()) {
						update_sequence_number(&participants_info, {}, -1, -1);
					} else {
						update_sequence_number(&participants_info, {michelle.getCMgr()->identity}, 0, -1);
					}
					check_conference_info_in_db(mgr, uid2, confAddr, marie.getCMgr()->identity, participants_info,
					                            start_time, duration2, initialSubject, description2, 0,
					                            LinphoneConferenceInfoStateNew, security_level, FALSE, TRUE,
					                            enable_video, enable_chat);
				}
				update_sequence_number(&participants_info, {michelle.getCMgr()->identity}, 0, -1);

				if (enable_ice) {
					if (!!!use_relay_ice_candidates) {
						BC_ASSERT_TRUE(check_ice(michelle.getCMgr(), focus.getCMgr(), LinphoneIceStateHostConnection));
					}
					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallUpdating, 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallStreamsRunning,
					                             2, liblinphone_tester_sip_timeout));
				}
				const LinphoneCallParams *michelle_call_lparams = linphone_call_get_params(michelle_call);
				BC_ASSERT_PTR_NOT_NULL(michelle_call_lparams);
				if (michelle_call_lparams) {
					LinphoneConferenceLayout michelle_layout =
					    linphone_call_params_get_conference_video_layout(michelle_call_lparams);
					BC_ASSERT_EQUAL(michelle_layout, layout, int, "%i");
				}

				if (enable_video) {
					ms_message("%s enables video", linphone_core_get_identity(michelle.getLc()));
					stats michelle_stat2 = michelle.getStats();
					LinphoneCallParams *michelle_new_params =
					    linphone_core_create_call_params(michelle.getLc(), michelle_call);
					linphone_call_params_enable_video(michelle_new_params, TRUE);
					linphone_call_update(michelle_call, michelle_new_params);
					linphone_call_params_unref(michelle_new_params);
					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallUpdating,
					                             michelle_stat2.number_of_LinphoneCallUpdating + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallStreamsRunning,
					                             michelle_stat2.number_of_LinphoneCallStreamsRunning + 1,
					                             liblinphone_tester_sip_timeout));

					const LinphoneCallParams *michelle_call_cparams = linphone_call_get_current_params(michelle_call);
					if ((layout == LinphoneConferenceLayoutGrid) &&
					    (all_listeners || ((video_direction == LinphoneMediaDirectionRecvOnly) ||
					                       (video_direction == LinphoneMediaDirectionInactive)))) {
						BC_ASSERT_FALSE(linphone_call_params_video_enabled(michelle_call_cparams));
					} else {
						BC_ASSERT_TRUE(linphone_call_params_video_enabled(michelle_call_cparams));
						LinphoneMediaDirection michelle_video_direction =
						    linphone_call_params_get_video_direction(michelle_call_cparams);
						LinphoneMediaDirection expected_video_direction =
						    (all_speakers) ? video_direction : LinphoneMediaDirectionRecvOnly;
						BC_ASSERT_EQUAL(michelle_video_direction, expected_video_direction, int, "%i");
					}

					if (!all_listeners && (audio_only_participant == FALSE) &&
					    ((video_direction != LinphoneMediaDirectionRecvOnly) ||
					     (layout == LinphoneConferenceLayoutActiveSpeaker))) {
						BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallUpdating,
						                             pauline_stat2.number_of_LinphoneCallUpdating + 1,
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(
						    coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning,
						    pauline_stat2.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));
					}
				}

			} else if (participant_list_type == LinphoneConferenceParticipantListTypeClosed) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingEarlyMedia,
				                             focus_stat2.number_of_LinphoneCallIncomingEarlyMedia + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallOutgoingEarlyMedia,
				                             1, liblinphone_tester_sip_timeout));

				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
				                             focus_stat2.number_of_participants_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_added,
				                             pauline_stat2.number_of_participants_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participants_added,
				                             laure_stat2.number_of_participants_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_added,
				                             marie_stat2.number_of_participants_added + 1,
				                             liblinphone_tester_sip_timeout));

				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joining_request,
				                             focus_stat2.number_of_participant_devices_joining_request + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_joining_request,
				                             marie_stat2.number_of_participant_devices_joining_request + 1,
				                             liblinphone_tester_sip_timeout));

				BC_ASSERT_TRUE(wait_for_list(
				    coresList, &pauline.getStats().number_of_participant_devices_joining_request,
				    pauline_stat2.number_of_participant_devices_joining_request + 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_joining_request,
				                             laure_stat2.number_of_participant_devices_joining_request + 1,
				                             liblinphone_tester_sip_timeout));

				BC_ASSERT_FALSE(wait_for_list(coresList, &marie.getStats().number_of_NotifyFullStateReceived,
				                              marie_stat2.number_of_NotifyFullStateReceived + 1, 1000));

				BC_ASSERT_EQUAL(michelle.getStats().number_of_LinphoneConferenceStateCreated, 0, int, "%0d");
				BC_ASSERT_EQUAL(michelle.getStats().number_of_LinphoneSubscriptionOutgoingProgress, 0, int, "%0d");
				BC_ASSERT_EQUAL(michelle.getStats().number_of_LinphoneSubscriptionActive, 0, int, "%0d");
				BC_ASSERT_EQUAL(focus.getStats().number_of_participant_devices_added,
				                focus_stat2.number_of_participant_devices_added, int, "%0d");
				BC_ASSERT_EQUAL(focus.getStats().number_of_conference_participant_devices_present,
				                focus_stat2.number_of_conference_participant_devices_present, int, "%0d");
				BC_ASSERT_EQUAL(focus.getStats().number_of_participant_devices_present,
				                focus_stat2.number_of_participant_devices_present, int, "%0d");
				BC_ASSERT_EQUAL(marie.getStats().number_of_participant_devices_added,
				                marie_stat2.number_of_participant_devices_added, int, "%0d");
				BC_ASSERT_EQUAL(marie.getStats().number_of_conference_participant_devices_present,
				                marie_stat2.number_of_conference_participant_devices_present, int, "%0d");
				BC_ASSERT_EQUAL(marie.getStats().number_of_participant_devices_present,
				                marie_stat2.number_of_participant_devices_present, int, "%0d");
				BC_ASSERT_EQUAL(pauline.getStats().number_of_participant_devices_added,
				                pauline_stat2.number_of_participant_devices_added, int, "%0d");
				BC_ASSERT_EQUAL(pauline.getStats().number_of_conference_participant_devices_present,
				                pauline_stat2.number_of_conference_participant_devices_present, int, "%0d");
				BC_ASSERT_EQUAL(pauline.getStats().number_of_participant_devices_present,
				                pauline_stat2.number_of_participant_devices_present, int, "%0d");
				BC_ASSERT_EQUAL(laure.getStats().number_of_participant_devices_added,
				                laure_stat2.number_of_participant_devices_added, int, "%0d");
				BC_ASSERT_EQUAL(laure.getStats().number_of_conference_participant_devices_present,
				                laure_stat2.number_of_conference_participant_devices_present, int, "%0d");
				BC_ASSERT_EQUAL(laure.getStats().number_of_participant_devices_present,
				                laure_stat2.number_of_participant_devices_present, int, "%0d");
			}

			LinphoneParticipantDeviceState michelle_device_expected_state =
			    (participant_list_type == LinphoneConferenceParticipantListTypeClosed)
			        ? LinphoneParticipantDeviceStateRequestingToJoin
			        : LinphoneParticipantDeviceStatePresent;
			size_t no_devices = static_cast<size_t>(no_local_participants + extra_participants);
			for (auto mgr : conferenceMgrs) {
				const LinphoneAddress *local_address = (mgr == focus.getCMgr()) ? confAddr : mgr->identity;
				LinphoneConference *pconference =
				    linphone_core_search_conference(mgr->lc, NULL, local_address, confAddr, NULL);
				if ((mgr == michelle.getCMgr()) &&
				    (participant_list_type == LinphoneConferenceParticipantListTypeClosed)) {
					BC_ASSERT_PTR_NULL(pconference);
				} else {
					BC_ASSERT_PTR_NOT_NULL(pconference);
				}
				if (pconference) {
					if (mgr != michelle.getCMgr()) {
						const LinphoneParticipant *michelle_participant =
						    linphone_conference_find_participant(pconference, michelle.getCMgr()->identity);
						BC_ASSERT_PTR_NOT_NULL(michelle_participant);
						if (michelle_participant) {
							bctbx_list_t *devices = linphone_participant_get_devices(michelle_participant);
							for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
								LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
								BC_ASSERT_EQUAL((int)linphone_participant_device_get_state(d),
								                (int)michelle_device_expected_state, int, "%0d");
							}
							if (devices) {
								bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
							}
						}
					}
					const LinphoneConferenceParams *conference_params =
					    linphone_conference_get_current_params(pconference);
					int no_participants = 0;
					if (start_time >= 0) {
						BC_ASSERT_EQUAL((long long)linphone_conference_params_get_start_time(conference_params),
						                (long long)start_time, long long, "%lld");
					}
					BC_ASSERT_EQUAL((long long)linphone_conference_params_get_end_time(conference_params),
					                (long long)end_time, long long, "%lld");
					BC_ASSERT_EQUAL((int)linphone_conference_params_get_security_level(conference_params),
					                (int)security_level, int, "%0d");
					if (mgr == focus.getCMgr()) {
						no_participants = no_local_participants + extra_participants;
						BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
					} else {
						// Substracting one because we conference server is not in the
						// conference
						no_participants = (no_local_participants - 1) + extra_participants;
						BC_ASSERT_TRUE(linphone_conference_is_in(pconference));

						LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
						BC_ASSERT_PTR_NOT_NULL(pcall);

						size_t no_streams_audio = 0;
						size_t no_streams_video = 0;
						size_t no_active_streams_video = 0;
						size_t no_streams_text = 0;
						if (pcall) {
							const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
							const bool_t enabled = linphone_call_params_video_enabled(call_cparams);
							no_streams_audio = compute_no_audio_streams(pcall, pconference);
							no_active_streams_video = compute_no_video_streams(enabled, pcall, pconference);
							no_streams_video = (all_speakers) ? 6 : 4;

							_linphone_call_check_max_nb_streams(pcall, no_streams_audio, no_streams_video,
							                                    no_streams_text);
							_linphone_call_check_nb_active_streams(pcall, no_streams_audio, no_active_streams_video,
							                                       no_streams_text);
						}

						LinphoneCall *fcall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
						BC_ASSERT_PTR_NOT_NULL(fcall);
						if (fcall) {
							_linphone_call_check_max_nb_streams(fcall, no_streams_audio, no_streams_video,
							                                    no_streams_text);
							_linphone_call_check_nb_active_streams(fcall, no_streams_audio, no_active_streams_video,
							                                       no_streams_text);
						}
					}
					BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int,
					                "%0d");
					bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
					BC_ASSERT_EQUAL(bctbx_list_size(devices), no_devices, size_t, "%zu");
					if (devices) {
						bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
					}
					BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
				}
			}

			if (participant_list_type == LinphoneConferenceParticipantListTypeClosed) {
				ms_message("%s terminates conference %s", linphone_core_get_identity(michelle.getLc()),
				           conference_address_str);
				LinphoneCall *michelle_call = linphone_core_get_call_by_remote_address2(michelle.getLc(), confAddr);
				BC_ASSERT_PTR_NOT_NULL(michelle_call);
				if (michelle_call) {
					linphone_call_terminate(michelle_call);
					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallEnd, 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
					                             focus_stat2.number_of_LinphoneCallEnd + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallReleased, 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
					                             focus_stat2.number_of_LinphoneCallReleased + 1,
					                             liblinphone_tester_sip_timeout));
				}
			}
		}

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
			return false;
		});

		stats marie_stat = marie.getStats();

		std::list<LinphoneCoreManager *> mgrsToRemove{pauline.getCMgr()};
		if (remove_participant) {
			stats pauline_stat = pauline.getStats();
			stats michelle_stat = michelle.getStats();

			LinphoneAddress *uri = marie.getCMgr()->identity;
			LinphoneConference *pconference = linphone_core_search_conference(marie.getLc(), NULL, uri, confAddr, NULL);

			ms_message("%s is removing %s from conference %s", linphone_core_get_identity(marie.getLc()),
			           linphone_core_get_identity(laure.getLc()), conference_address_str);

			BC_ASSERT_PTR_NOT_NULL(pconference);
			LinphoneAddress *puri = laure.getCMgr()->identity;
			if (pconference) {
				LinphoneParticipant *participant = linphone_conference_find_participant(pconference, puri);
				BC_ASSERT_PTR_NOT_NULL(participant);
				linphone_conference_remove_participant_2(pconference, participant);
			}

			LinphoneCoreManager *laureMgr = laure.getCMgr();
			auto itConferenceMgrs = std::find(conferenceMgrs.begin(), conferenceMgrs.end(), laureMgr);
			if (itConferenceMgrs != conferenceMgrs.end()) {
				conferenceMgrs.erase(itConferenceMgrs);
			}

			auto itMembers = std::find(members.begin(), members.end(), laureMgr);
			if (itMembers != members.end()) {
				members.erase(itMembers);
			}

			participantList.erase(laure.getCMgr());

			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallEnd, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallReleased, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneSubscriptionTerminated,
			                             nb_subscriptions, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &laure.getStats().number_of_LinphoneConferenceStateTerminationPending, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			if (!enable_chat) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateDeleted, 1,
				                             liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
			                             focus_stat.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
			                             focus_stat.number_of_participant_devices_removed + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed,
			                             marie_stat.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_removed,
			                             marie_stat.number_of_participant_devices_removed + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_removed,
			                             pauline_stat.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_removed,
			                             pauline_stat.number_of_participant_devices_removed + 1,
			                             liblinphone_tester_sip_timeout));

			if (enable_video) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating,
				                             marie_stat.number_of_LinphoneCallUpdating + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
				                             marie_stat.number_of_LinphoneCallStreamsRunning + 1,
				                             liblinphone_tester_sip_timeout));

				if (!all_listeners && (audio_only_participant == FALSE) &&
				    ((video_direction != LinphoneMediaDirectionRecvOnly) ||
				     (layout == LinphoneConferenceLayoutActiveSpeaker))) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallUpdating,
					                             pauline_stat.number_of_LinphoneCallUpdating + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning,
					                             pauline_stat.number_of_LinphoneCallStreamsRunning + 1,
					                             liblinphone_tester_sip_timeout));
				}
			}

			if ((uninvited_participant_dials) && (participant_list_type == LinphoneConferenceParticipantListTypeOpen)) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participants_removed,
				                             michelle_stat.number_of_participants_removed + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_removed,
				                             michelle_stat.number_of_participant_devices_removed + 1,
				                             liblinphone_tester_sip_timeout));

				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallUpdating,
				                             michelle_stat.number_of_LinphoneCallUpdating + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallStreamsRunning,
				                             michelle_stat.number_of_LinphoneCallStreamsRunning + 1,
				                             liblinphone_tester_sip_timeout));
			}

			std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
			    fill_member_list(members, participantList, marie.getCMgr(), participants_info);
			wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs,
			                            focus.getCMgr(), memberList, confAddr, enable_video);

			LinphoneConference *conference = linphone_core_search_conference(laure.getLc(), NULL, puri, confAddr, NULL);
			if (enable_chat) {
				BC_ASSERT_PTR_NOT_NULL(conference);
			} else {
				BC_ASSERT_PTR_NULL(conference);
			}

			no_local_participants = 3;
			if (uninvited_participant_dials) {
				stats marie_stat2 = marie.getStats();
				stats focus_stat2 = focus.getStats();
				stats pauline_stat2 = pauline.getStats();
				stats michelle_stat2 = michelle.getStats();

				extraParticipantMgrs.push_back(berthe.getCMgr());
				conferenceMgrs.push_back(berthe.getCMgr());
				allConferenceMgrs.push_back(berthe.getCMgr());
				members.push_back(berthe.getCMgr());
				participantList.insert(std::make_pair(
				    berthe.getCMgr(), add_participant_info_to_list(&participants_info, berthe.getCMgr()->identity,
				                                                   (listenerAllowed) ? LinphoneParticipantRoleListener
				                                                                     : LinphoneParticipantRoleSpeaker,
				                                                   -1)));
				ms_message("%s is entering conference %s", linphone_core_get_identity(berthe.getLc()),
				           conference_address_str);

				LinphoneCallParams *params = linphone_core_create_call_params(berthe.getLc(), nullptr);
				LinphoneCall *berthe_call = linphone_core_invite_address_with_params(berthe.getLc(), confAddr, params);
				BC_ASSERT_PTR_NOT_NULL(berthe_call);
				linphone_call_params_unref(params);
				BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallOutgoingProgress, 1,
				                             liblinphone_tester_sip_timeout));
				int extra_participants = 0;
				if (participant_list_type == LinphoneConferenceParticipantListTypeOpen) {
					extra_participants = 1;

					BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallStreamsRunning, 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallUpdating, 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallStreamsRunning, 2,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneConferenceStateCreated,
					                             1, liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList,
					                             &berthe.getStats().number_of_LinphoneSubscriptionOutgoingProgress,
					                             nb_subscriptions, 5000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneSubscriptionActive,
					                             nb_subscriptions, 5000));
					BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_conference_full_state_received,
					                             1, liblinphone_tester_sip_timeout));

					if (enable_ice) {
						BC_ASSERT_TRUE(check_ice(berthe.getCMgr(), focus.getCMgr(), LinphoneIceStateHostConnection));
						BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallUpdating, 1,
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallStreamsRunning,
						                             2, liblinphone_tester_sip_timeout));
					}
					BC_ASSERT_TRUE(wait_for_list(
					    coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
					    focus_stat2.number_of_LinphoneCallUpdatedByRemote + (audio_only_participant) ? 3 : 4,
					    liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(
					    coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
					    focus_stat2.number_of_LinphoneCallStreamsRunning + (audio_only_participant) ? 4 : 5,
					    liblinphone_tester_sip_timeout));

					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
					                             focus_stat2.number_of_participants_added + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
					                             focus_stat2.number_of_participant_devices_added + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList,
					                             &focus.getStats().number_of_conference_participant_devices_present,
					                             focus_stat2.number_of_conference_participant_devices_present + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
					                             focus_stat2.number_of_participant_devices_present + 1,
					                             liblinphone_tester_sip_timeout));

					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_added,
					                             marie_stat2.number_of_participants_added + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_added,
					                             marie_stat2.number_of_participant_devices_added + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList,
					                             &marie.getStats().number_of_conference_participant_devices_present,
					                             marie_stat2.number_of_conference_participant_devices_present + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_present,
					                             marie_stat2.number_of_participant_devices_present + 1,
					                             liblinphone_tester_sip_timeout));

					BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_added,
					                             pauline_stat2.number_of_participants_added + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_added,
					                             pauline_stat2.number_of_participant_devices_added + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList,
					                             &pauline.getStats().number_of_conference_participant_devices_present,
					                             pauline_stat2.number_of_conference_participant_devices_present + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_present,
					                             pauline_stat2.number_of_participant_devices_present + 1,
					                             liblinphone_tester_sip_timeout));

					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participants_added,
					                             michelle_stat2.number_of_participants_added + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_added,
					                             michelle_stat2.number_of_participant_devices_added + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList,
					                             &michelle.getStats().number_of_conference_participant_devices_present,
					                             michelle_stat2.number_of_conference_participant_devices_present + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_present,
					                             michelle_stat2.number_of_participant_devices_present + 1,
					                             liblinphone_tester_sip_timeout));

					memberList = fill_member_list(members, participantList, marie.getCMgr(), participants_info);
					wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs,
					                            focus.getCMgr(), memberList, confAddr, enable_video);
				} else if (participant_list_type == LinphoneConferenceParticipantListTypeClosed) {
					extra_participants = 0;

					BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallError, 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallReleased, 1,
					                             liblinphone_tester_sip_timeout));

					// wait a bit longer to detect side effect if any
					CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
					    .waitUntil(chrono::seconds(2), [] { return false; });

					BC_ASSERT_EQUAL(berthe.getStats().number_of_LinphoneConferenceStateCreated, 0, int, "%0d");
					BC_ASSERT_EQUAL(berthe.getStats().number_of_LinphoneSubscriptionOutgoingProgress, 0, int, "%0d");
					BC_ASSERT_EQUAL(berthe.getStats().number_of_LinphoneSubscriptionActive, 0, int, "%0d");
					BC_ASSERT_EQUAL(focus.getStats().number_of_participants_added,
					                focus_stat2.number_of_participants_added, int, "%0d");
					BC_ASSERT_EQUAL(focus.getStats().number_of_participant_devices_added,
					                focus_stat2.number_of_participant_devices_added, int, "%0d");
					BC_ASSERT_EQUAL(focus.getStats().number_of_conference_participant_devices_present,
					                focus_stat2.number_of_conference_participant_devices_present, int, "%0d");
					BC_ASSERT_EQUAL(focus.getStats().number_of_participant_devices_present,
					                focus_stat2.number_of_participant_devices_present, int, "%0d");
					BC_ASSERT_EQUAL(marie.getStats().number_of_participants_added,
					                marie_stat2.number_of_participants_added, int, "%0d");
					BC_ASSERT_EQUAL(marie.getStats().number_of_participant_devices_added,
					                marie_stat2.number_of_participant_devices_added, int, "%0d");
					BC_ASSERT_EQUAL(marie.getStats().number_of_conference_participant_devices_present,
					                marie_stat2.number_of_conference_participant_devices_present, int, "%0d");
					BC_ASSERT_EQUAL(marie.getStats().number_of_participant_devices_present,
					                marie_stat2.number_of_participant_devices_present, int, "%0d");
					BC_ASSERT_EQUAL(pauline.getStats().number_of_participants_added,
					                pauline_stat2.number_of_participants_added, int, "%0d");
					BC_ASSERT_EQUAL(pauline.getStats().number_of_participant_devices_added,
					                pauline_stat2.number_of_participant_devices_added, int, "%0d");
					BC_ASSERT_EQUAL(pauline.getStats().number_of_conference_participant_devices_present,
					                pauline_stat2.number_of_conference_participant_devices_present, int, "%0d");
					BC_ASSERT_EQUAL(pauline.getStats().number_of_participant_devices_present,
					                pauline_stat2.number_of_participant_devices_present, int, "%0d");
					BC_ASSERT_EQUAL(michelle.getStats().number_of_participants_added,
					                michelle_stat2.number_of_participants_added, int, "%0d");
					BC_ASSERT_EQUAL(michelle.getStats().number_of_participant_devices_added,
					                michelle_stat2.number_of_participant_devices_added, int, "%0d");
					BC_ASSERT_EQUAL(michelle.getStats().number_of_conference_participant_devices_present,
					                michelle_stat2.number_of_conference_participant_devices_present, int, "%0d");
					BC_ASSERT_EQUAL(michelle.getStats().number_of_participant_devices_present,
					                michelle_stat2.number_of_participant_devices_present, int, "%0d");
				}

				for (auto mgr : conferenceMgrs) {
					const LinphoneAddress *local_address = (mgr == focus.getCMgr()) ? confAddr : mgr->identity;
					LinphoneConference *pconference =
					    linphone_core_search_conference(mgr->lc, NULL, local_address, confAddr, NULL);
					if ((participant_list_type == LinphoneConferenceParticipantListTypeOpen) ||
					    ((mgr != berthe.getCMgr()) && (mgr != michelle.getCMgr()))) {
						BC_ASSERT_PTR_NOT_NULL(pconference);
					} else if ((mgr == berthe.getCMgr()) || (mgr == michelle.getCMgr())) {
						BC_ASSERT_PTR_NULL(pconference);
					}
					if (pconference) {
						auto info = linphone_conference_get_info(pconference);
						check_conference_info_against_db(mgr, confAddr, info, FALSE);
						const LinphoneConferenceParams *conference_params =
						    linphone_conference_get_current_params(pconference);
						int no_participants = 0;
						if (start_time >= 0) {
							BC_ASSERT_EQUAL((long long)linphone_conference_params_get_start_time(conference_params),
							                (long long)start_time, long long, "%lld");
						}
						BC_ASSERT_EQUAL((long long)linphone_conference_params_get_end_time(conference_params),
						                (long long)end_time, long long, "%lld");
						BC_ASSERT_EQUAL((int)linphone_conference_params_get_security_level(conference_params),
						                (int)security_level, int, "%0d");
						if (mgr == focus.getCMgr()) {
							no_participants = no_local_participants + extra_participants;
							BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
						} else {
							LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
							BC_ASSERT_PTR_NOT_NULL(pcall);

							size_t no_streams_audio = 0;
							size_t no_max_streams_audio = 0;
							size_t no_streams_video = 0;
							size_t no_active_streams_video = 0;
							size_t no_streams_text = 0;
							if (pcall) {
								const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
								const bool_t enabled = linphone_call_params_video_enabled(call_cparams);
								no_streams_audio = compute_no_audio_streams(pcall, pconference);
								no_active_streams_video = compute_no_video_streams(enabled, pcall, pconference);
								no_streams_video = 5;
								no_max_streams_audio = 1;

								_linphone_call_check_max_nb_streams(pcall, no_max_streams_audio, no_streams_video,
								                                    no_streams_text);
								_linphone_call_check_nb_active_streams(pcall, no_streams_audio, no_active_streams_video,
								                                       no_streams_text);
							}

							LinphoneCall *fcall =
							    linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
							BC_ASSERT_PTR_NOT_NULL(fcall);
							if (fcall) {
								_linphone_call_check_max_nb_streams(fcall, no_max_streams_audio, no_streams_video,
								                                    no_streams_text);
								_linphone_call_check_nb_active_streams(fcall, no_streams_audio, no_active_streams_video,
								                                       no_streams_text);
							}

							// Substracting one because we conference server is not in the
							// conference
							no_participants = (no_local_participants - 1) + extra_participants;
							BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
						}
						BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int,
						                "%0d");
						bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
						BC_ASSERT_EQUAL(bctbx_list_size(devices),
						                static_cast<size_t>(no_local_participants + extra_participants), size_t, "%zu");
						if (devices) {
							bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
						}
						BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
					}
				}

				// wait a bit longer to detect side effect if any
				CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
					return false;
				});
			}

		} else {
			if (!version_mismatch) mgrsToRemove.push_back(laure.getCMgr());
		}

		LinphoneAddress *paulineUri = pauline.getCMgr()->identity;
		LinphoneConference *paulineConference =
		    linphone_core_search_conference(pauline.getLc(), NULL, paulineUri, confAddr, NULL);
		BC_ASSERT_PTR_NOT_NULL(paulineConference);

		if (paulineConference) {
			stats focus_stat2 = focus.getStats();
			stats marie_stat2 = marie.getStats();
			stats pauline_stat2 = pauline.getStats();
			stats laure_stat2 = laure.getStats();
			stats michelle_stat2 = michelle.getStats();

			LinphoneCall *pauline_call = linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(pauline_call);

			bool_t video_enabled = FALSE;
			bool_t sending_video = FALSE;
			if (pauline_call) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pauline_call);
				video_enabled = linphone_call_params_video_enabled(call_cparams);
				LinphoneMediaDirection video_direction = linphone_call_params_get_video_direction(call_cparams);
				sending_video = video_enabled && ((video_direction == LinphoneMediaDirectionSendRecv) &&
				                                  (video_direction == LinphoneMediaDirectionSendOnly));
			}

			ms_message("%s is leaving conference %s", linphone_core_get_identity(pauline.getLc()),
			           conference_address_str);
			linphone_conference_leave(paulineConference);

			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallPausing,
			                             pauline_stat2.number_of_LinphoneCallPausing + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallPaused,
			                             pauline_stat2.number_of_LinphoneCallPaused + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallPausedByRemote,
			                             focus_stat2.number_of_LinphoneCallPausedByRemote + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_on_hold,
			                             focus_stat2.number_of_conference_participant_devices_on_hold + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_on_hold,
			                             focus_stat2.number_of_participant_devices_on_hold + 1,
			                             liblinphone_tester_sip_timeout));

			if (!remove_participant && !version_mismatch) {
				BC_ASSERT_TRUE(wait_for_list(
				    coresList, &laure.getStats().number_of_conference_participant_devices_on_hold,
				    laure_stat2.number_of_conference_participant_devices_on_hold + 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_on_hold,
				                             laure_stat2.number_of_participant_devices_on_hold + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList,
				                             &laure.getStats().number_of_participant_devices_media_capability_changed,
				                             laure_stat2.number_of_participant_devices_media_capability_changed + 1,
				                             liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_conference_participant_devices_on_hold,
			                             marie_stat2.number_of_conference_participant_devices_on_hold + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_on_hold,
			                             marie_stat2.number_of_participant_devices_on_hold + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &marie.getStats().number_of_participant_devices_media_capability_changed,
			                             marie_stat2.number_of_participant_devices_media_capability_changed + 1,
			                             liblinphone_tester_sip_timeout));

			if (uninvited_participant_dials && (participant_list_type == LinphoneConferenceParticipantListTypeOpen)) {
				BC_ASSERT_TRUE(wait_for_list(coresList,
				                             &michelle.getStats().number_of_conference_participant_devices_on_hold,
				                             michelle_stat2.number_of_conference_participant_devices_on_hold + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_on_hold,
				                             michelle_stat2.number_of_participant_devices_on_hold + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(
				    coresList, &michelle.getStats().number_of_participant_devices_media_capability_changed,
				    michelle_stat2.number_of_participant_devices_media_capability_changed + 1,
				    liblinphone_tester_sip_timeout));
			}

			linphone_conference_enter(paulineConference);

			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallResuming,
			                             pauline_stat2.number_of_LinphoneCallResuming + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning,
			                             pauline_stat2.number_of_LinphoneCallStreamsRunning + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
			                             focus_stat2.number_of_LinphoneCallStreamsRunning + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
			                             focus_stat2.number_of_conference_participant_devices_present + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
			                             focus_stat2.number_of_participant_devices_present + 1,
			                             liblinphone_tester_sip_timeout));

			if (!remove_participant && !version_mismatch) {
				BC_ASSERT_TRUE(wait_for_list(
				    coresList, &laure.getStats().number_of_conference_participant_devices_present,
				    laure_stat2.number_of_conference_participant_devices_present + 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_present,
				                             laure_stat2.number_of_participant_devices_present + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList,
				                             &laure.getStats().number_of_participant_devices_media_capability_changed,
				                             laure_stat2.number_of_participant_devices_media_capability_changed + 2,
				                             liblinphone_tester_sip_timeout));
				if (sending_video) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallStreamsRunning,
					                             laure_stat2.number_of_LinphoneCallStreamsRunning + 1,
					                             liblinphone_tester_sip_timeout));
				}
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_conference_participant_devices_present,
			                             marie_stat2.number_of_conference_participant_devices_present + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_present,
			                             marie_stat2.number_of_participant_devices_present + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &marie.getStats().number_of_participant_devices_media_capability_changed,
			                             marie_stat2.number_of_participant_devices_media_capability_changed + 2,
			                             liblinphone_tester_sip_timeout));
			if (sending_video) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
				                             marie_stat2.number_of_LinphoneCallStreamsRunning + 1,
				                             liblinphone_tester_sip_timeout));
			}

			if (uninvited_participant_dials && (participant_list_type == LinphoneConferenceParticipantListTypeOpen)) {
				BC_ASSERT_TRUE(wait_for_list(coresList,
				                             &michelle.getStats().number_of_conference_participant_devices_present,
				                             michelle_stat2.number_of_conference_participant_devices_present + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_present,
				                             michelle_stat2.number_of_participant_devices_present + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(
				    coresList, &michelle.getStats().number_of_participant_devices_media_capability_changed,
				    michelle_stat2.number_of_participant_devices_media_capability_changed + 1,
				    liblinphone_tester_sip_timeout));
				if (sending_video) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallStreamsRunning,
					                             michelle_stat2.number_of_LinphoneCallStreamsRunning + 1,
					                             liblinphone_tester_sip_timeout));
				}
			}

			if (pauline_call) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pauline_call);
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

			LinphoneCall *call = linphone_core_get_current_call(mgr->lc);
			BC_ASSERT_PTR_NOT_NULL(call);
			if (call) {
				stats marie_stat2 = marie.getStats();
				stats focus_stat2 = focus.getStats();
				if (network_restart) {
					ms_message("%s switches off network before %s leaves conference %s",
					           linphone_core_get_identity(marie.getLc()), linphone_core_get_identity(mgr->lc),
					           conference_address_str);
					linphone_core_set_network_reachable(marie.getLc(), FALSE);
					BC_ASSERT_TRUE(
					    wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionTerminated,
					                  marie_stat2.number_of_LinphoneSubscriptionTerminated + nb_subscriptions,
					                  liblinphone_tester_sip_timeout));
				}

				ms_message("%s exits conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
				linphone_call_terminate(call);
				BC_ASSERT_TRUE(
				    wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated,
				                             nb_subscriptions, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending,
				                             1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated, 1,
				                             liblinphone_tester_sip_timeout));
				if (!enable_chat) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1,
					                             liblinphone_tester_sip_timeout));
				}

				LinphoneConference *pconference =
				    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr, NULL);
				if (enable_chat) {
					BC_ASSERT_PTR_NOT_NULL(pconference);
				} else {
					BC_ASSERT_PTR_NULL(pconference);
				}

				if (network_restart) {
					ms_message("%s is back online after %s leaves conference %s",
					           linphone_core_get_identity(marie.getLc()), linphone_core_get_identity(mgr->lc),
					           conference_address_str);
					linphone_core_set_network_reachable(marie.getLc(), TRUE);
					BC_ASSERT_TRUE(
					    wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionOutgoingProgress,
					                  marie_stat2.number_of_LinphoneSubscriptionOutgoingProgress + nb_subscriptions,
					                  liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(
					    wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
					                  focus_stat2.number_of_LinphoneSubscriptionIncomingReceived + nb_subscriptions,
					                  liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
					                             marie_stat2.number_of_LinphoneSubscriptionActive + nb_subscriptions,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_conference_full_state_received,
					                             marie_stat2.number_of_conference_full_state_received + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
					                             focus_stat2.number_of_LinphoneSubscriptionActive + nb_subscriptions,
					                             liblinphone_tester_sip_timeout));

					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed,
					                             marie_stat2.number_of_participants_removed + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_removed,
					                             marie_stat2.number_of_participant_devices_removed + 1,
					                             liblinphone_tester_sip_timeout));

					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
					                             marie_stat2.number_of_LinphoneCallStreamsRunning + 2,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
					                             focus_stat2.number_of_LinphoneCallStreamsRunning + 2,
					                             liblinphone_tester_sip_timeout));
				}
			}
		}

		int removeSize = static_cast<int>(mgrsToRemove.size());
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
		                             focus_stat.number_of_participants_removed + removeSize,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed + removeSize,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed,
		                             marie_stat.number_of_participants_removed + removeSize,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_removed,
		                             marie_stat.number_of_participant_devices_removed + removeSize,
		                             liblinphone_tester_sip_timeout));

		if (uninvited_participant_dials) {
			if (participant_list_type == LinphoneConferenceParticipantListTypeOpen) {
				int extra_participants = static_cast<int>(extraParticipantMgrs.size());

				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participants_removed, 2,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_removed, 2,
				                             liblinphone_tester_sip_timeout));

				if (remove_participant) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_participants_removed, 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_participant_devices_removed, 1,
					                             liblinphone_tester_sip_timeout));
				}

				for (auto mgr : {focus.getCMgr(), marie.getCMgr(), michelle.getCMgr(), berthe.getCMgr()}) {
					const LinphoneAddress *local_address = (mgr == focus.getCMgr()) ? confAddr : mgr->identity;
					LinphoneConference *pconference =
					    linphone_core_search_conference(mgr->lc, NULL, local_address, confAddr, NULL);
					if (!remove_participant && (mgr == berthe.getCMgr())) {
						BC_ASSERT_PTR_NULL(pconference);
					} else {
						BC_ASSERT_PTR_NOT_NULL(pconference);
					}
					if (pconference) {
						BC_ASSERT_TRUE(
						    CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
						        .waitUntil(chrono::seconds(10), [&mgr, &focus, &pconference, &extra_participants] {
							        return (linphone_conference_get_participant_count(pconference) ==
							                ((mgr == focus.getCMgr()) ? (extra_participants + 1) : extra_participants));
						        }));

						BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference),
						                ((mgr == focus.getCMgr()) ? (extra_participants + 1) : extra_participants), int,
						                "%0d");
						bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
						BC_ASSERT_EQUAL(bctbx_list_size(devices), (extra_participants + 1), size_t, "%zu");
						if (devices) {
							bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
						}
						BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
					}
				}

				stats marie_stat2 = marie.getStats();
				stats focus_stat2 = focus.getStats();

				for (auto mgr : extraParticipantMgrs) {
					LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);

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
						BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1,
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1,
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated,
						                             nb_subscriptions, liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(coresList,
						                             &mgr->stat.number_of_LinphoneConferenceStateTerminationPending, 1,
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated,
						                             1, liblinphone_tester_sip_timeout));
						if (!enable_chat) {
							BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted,
							                             1, liblinphone_tester_sip_timeout));
						}
					}
				}

				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
				                             focus_stat2.number_of_participants_removed + extra_participants,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
				                             focus_stat2.number_of_participant_devices_removed + extra_participants,
				                             liblinphone_tester_sip_timeout));

				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed,
				                             marie_stat2.number_of_participants_removed + extra_participants,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_removed,
				                             marie_stat2.number_of_participant_devices_removed + extra_participants,
				                             liblinphone_tester_sip_timeout));
			} else if (participant_list_type == LinphoneConferenceParticipantListTypeClosed) {
				LinphoneCall *call = linphone_core_get_current_call(michelle.getLc());
				BC_ASSERT_PTR_NULL(call);

				LinphoneAddress *uri = michelle.getCMgr()->identity;
				LinphoneConference *pconference =
				    linphone_core_search_conference(michelle.getLc(), NULL, uri, confAddr, NULL);
				BC_ASSERT_PTR_NULL(pconference);
			}
		}

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
			return false;
		});

		BC_ASSERT_EQUAL(marie.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                marie_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(marie.getStats().number_of_LinphoneConferenceStateTerminated,
		                marie_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(marie.getStats().number_of_LinphoneConferenceStateDeleted,
		                marie_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                focus_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminated,
		                focus_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateDeleted,
		                focus_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		for (auto mgr : conferenceMgrs) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference),
				                ((mgr == focus.getCMgr()) ? 1 : 0), int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 1, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
			}
		}

		const bctbx_list_t *calls = linphone_core_get_calls(marie.getLc());
		BC_ASSERT_EQUAL(bctbx_list_size(calls), 1, size_t, "%zu");

		LinphoneCall *call = linphone_core_get_call_by_remote_address2(marie.getLc(), focus.getCMgr()->identity);
		BC_ASSERT_PTR_NOT_NULL(call);
		if (call) {
			stats marie_stat3 = marie.getStats();
			linphone_call_terminate(call);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallEnd, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallReleased, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionTerminated,
			                             marie_stat3.number_of_LinphoneSubscriptionTerminated + nb_subscriptions,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &marie.getStats().number_of_LinphoneConferenceStateTerminationPending, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			if (!enable_chat) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateDeleted, 1,
				                             liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &focus.getStats().number_of_LinphoneConferenceStateTerminationPending, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
			                             liblinphone_tester_sip_timeout));
		}

		check_delete_focus_conference_info({focus, marie, pauline, laure, michelle, berthe}, allConferenceMgrs,
		                                   focus.getCMgr(), confAddr, end_time);

		std::list<LinphoneCoreManager *> allMembers{marie.getCMgr(), pauline.getCMgr()};
		if (!version_mismatch) allMembers.push_back(laure.getCMgr());
		if ((participant_list_type == LinphoneConferenceParticipantListTypeOpen) && uninvited_participant_dials) {
			allMembers.push_back(michelle.getCMgr());
			if (remove_participant) {
				allMembers.push_back(berthe.getCMgr());
			}
		}
		for (auto mgr : allMembers) {
			bctbx_list_t *conference_infos = linphone_core_get_conference_information_list(mgr->lc);
			BC_ASSERT_PTR_NOT_NULL(conference_infos);
			if (conference_infos) {
				BC_ASSERT_EQUAL(bctbx_list_size(conference_infos), 1, size_t, "%zu");
				bctbx_list_free_with_data(conference_infos, (bctbx_list_free_func)linphone_conference_info_unref);
			}

			bctbx_list_t *infos = linphone_core_get_conference_information_list(mgr->lc);
			if (BC_ASSERT_PTR_NOT_NULL(infos)) {
				BC_ASSERT_EQUAL((int)bctbx_list_size(infos), 1, int, "%d");
				bctbx_list_free_with_data(infos, (bctbx_list_free_func)linphone_conference_info_unref);
			}

			const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
			BC_ASSERT_PTR_NOT_NULL(call_logs);
			if (call_logs) {
				BC_ASSERT_EQUAL(bctbx_list_size(call_logs), ((client_restart && (mgr == marie.getCMgr())) ? 2 : 1),
				                size_t, "%zu");
			}

			bctbx_list_t *mgr_focus_call_log =
			    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
			if (mgr_focus_call_log) {
				BC_ASSERT_EQUAL(bctbx_list_size(mgr_focus_call_log),
				                ((client_restart && (mgr == marie.getCMgr())) ? 2 : 1), size_t, "%zu");
				for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
					LinphoneCallLog *call_log = (LinphoneCallLog *)it->data;
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				}
				bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
			}
		}

		LinphoneConference *fconference2 = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NULL(fconference2);

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
			return false;
		});

		ms_free(uid);
		ms_free(conference_address_str);
		linphone_address_unref(confAddr);
		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		bctbx_list_free(coresList);
	}
}

void create_conference_with_screen_sharing_base(time_t start_time,
                                                int duration,
                                                const LinphoneMediaEncryption encryption,
                                                LinphoneConferenceLayout layout,
                                                bool_t enable_video,
                                                bool_t enable_camera,
                                                bool_t turn_off_screen_sharing,
                                                LinphoneMediaDirection video_direction,
                                                LinphoneConferenceSecurityLevel security_level,
                                                std::list<LinphoneParticipantRole> allowedRoles) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		bool_t enable_lime = (security_level == LinphoneConferenceSecurityLevelEndToEnd ? TRUE : FALSE);

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

		int nb_subscriptions = 1;
		if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
			nb_subscriptions = 2; // One more subscription for the EKT
		}

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t *coresList = NULL;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(),
		                 berthe.getCMgr()}) {
			if (enable_video) {
				LinphoneVideoActivationPolicy *pol =
				    linphone_factory_create_video_activation_policy(linphone_factory_get());
				linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
				linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
				linphone_core_set_video_activation_policy(mgr->lc, pol);
				linphone_video_activation_policy_unref(pol);

				linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);
				linphone_core_enable_video_capture(mgr->lc, TRUE);
				linphone_core_enable_video_display(mgr->lc, TRUE);

				if (layout == LinphoneConferenceLayoutGrid) {
					linphone_core_set_preferred_video_definition_by_name(mgr->lc, "720p");
					linphone_config_set_string(linphone_core_get_config(mgr->lc), "video", "max_conference_size",
					                           "vga");
				}

				if (mgr == pauline.getCMgr()) {
					linphone_core_set_conference_max_thumbnails(mgr->lc, 3);
				}
			}

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, layout);
				linphone_core_set_media_encryption(mgr->lc, encryption);
			}

			// Enable ICE at the account level but not at the core level
			enable_stun_in_mgr(mgr, TRUE, TRUE, FALSE, FALSE);

			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip", "update_call_when_ice_completed", TRUE);
			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip",
			                        "update_call_when_ice_completed_with_dtls", FALSE);

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		if (enable_lime) {
			configure_end_to_end_encrypted_conference_server(focus);
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(berthe.getLc()));
		}

		int nortp_timeout = 10;
		linphone_core_set_nortp_timeout(marie.getLc(), nortp_timeout);
		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), LinphoneConferenceParticipantListTypeClosed);

		stats focus_stat = focus.getStats();

		const bool oneRoleAllowed = (allowedRoles.size() == 1);
		bool speakerAllowed = std::find(allowedRoles.cbegin(), allowedRoles.cend(), LinphoneParticipantRoleSpeaker) !=
		                      allowedRoles.cend();
		bool listenerAllowed = std::find(allowedRoles.cbegin(), allowedRoles.cend(), LinphoneParticipantRoleListener) !=
		                       allowedRoles.cend();
		bool all_listeners = listenerAllowed && oneRoleAllowed;

		std::list<LinphoneCoreManager *> participants{laure.getCMgr(), pauline.getCMgr(), michelle.getCMgr(),
		                                              berthe.getCMgr()};
		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(),    pauline.getCMgr(),
		                                                laure.getCMgr(), michelle.getCMgr(), berthe.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(),
		                                         michelle.getCMgr(), berthe.getCMgr()};

		time_t end_time = (duration <= 0) ? -1 : (start_time + duration * 60);
		const char *initialSubject = "Test characters: ^ :) ¤ çà @";
		const char *description = "Paris Baker";

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		participantList.insert(std::make_pair(
		    laure.getCMgr(), add_participant_info_to_list(&participants_info, laure.getCMgr()->identity,
		                                                  (listenerAllowed) ? LinphoneParticipantRoleListener
		                                                                    : LinphoneParticipantRoleSpeaker,
		                                                  -1)));
		participantList.insert(std::make_pair(
		    pauline.getCMgr(), add_participant_info_to_list(&participants_info, pauline.getCMgr()->identity,
		                                                    LinphoneParticipantRoleSpeaker, -1)));
		participantList.insert(std::make_pair(
		    michelle.getCMgr(), add_participant_info_to_list(&participants_info, michelle.getCMgr()->identity,
		                                                     (listenerAllowed) ? LinphoneParticipantRoleListener
		                                                                       : LinphoneParticipantRoleSpeaker,
		                                                     -1)));
		participantList.insert(std::make_pair(
		    berthe.getCMgr(), add_participant_info_to_list(&participants_info, berthe.getCMgr()->identity,
		                                                   (speakerAllowed) ? LinphoneParticipantRoleSpeaker
		                                                                    : LinphoneParticipantRoleListener,
		                                                   -1)));

		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, security_level, TRUE, FALSE, NULL);
		BC_ASSERT_PTR_NOT_NULL(confAddr);
		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:");

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 4,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : members) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_call_params_set_media_encryption(new_params, encryption);
			linphone_call_params_set_video_direction(new_params, video_direction);
			if (mgr == pauline.getCMgr()) {
				linphone_call_params_enable_mic(new_params, FALSE);
			}
			ms_message("%s is entering conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
				BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
			}
		}

		int idx = 1;
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
			                             ((mgr == marie.getCMgr()) ? 4 : 1), liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress,
			                             1 * nb_subscriptions, 5000));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1 * nb_subscriptions, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));
			if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyEktReceived, 1,
				                             liblinphone_tester_sip_timeout));
			}

			if ((encryption == LinphoneMediaEncryptionDTLS) || (encryption == LinphoneMediaEncryptionZRTP)) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEncryptedOn, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEncryptedOn, idx,
				                             liblinphone_tester_sip_timeout));
			}

			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
				const LinphoneMediaEncryption pcall_enc = linphone_call_params_get_media_encryption(call_cparams);
				BC_ASSERT_EQUAL(pcall_enc, encryption, int, "%d");
			}
			LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(ccall);
			if (ccall) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
				const LinphoneMediaEncryption ccall_enc = linphone_call_params_get_media_encryption(call_cparams);
				BC_ASSERT_EQUAL(ccall_enc, encryption, int, "%d");
			}

			idx++;
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + 5,
		                             liblinphone_tester_sip_timeout));
		int focus_no_streams_running = 10;
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running - 5),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running,
		                             liblinphone_tester_sip_timeout));
		// If ICE is enabled, the addition to a conference may go through a resume of the call
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
		                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + (5 * nb_subscriptions),
		                             5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + (5 * nb_subscriptions), 5000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + 5, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + 5,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
		                             focus_stat.number_of_conference_participant_devices_present + 5,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
		                             focus_stat.number_of_participant_devices_present + 5,
		                             liblinphone_tester_sip_timeout));

		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
		    fill_member_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs, focus.getCMgr(),
		                            memberList, confAddr, enable_video);

		for (const auto &mgr : conferenceMgrs) {
			BC_ASSERT_TRUE(
			    CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
			        .waitUntil(chrono::seconds(50), [mgr, enable_video, video_direction, confAddr] {
				        bool_t video_ok = TRUE;
				        LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
				        bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
				        for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					        LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					        const LinphoneAddress *device_address = linphone_participant_device_get_address(d);
					        LinphoneParticipant *p =
					            linphone_conference_is_me(conference, device_address)
					                ? linphone_conference_get_me(conference)
					                : linphone_conference_find_participant(conference, device_address);
					        if (p) {
						        bool_t video_available =
						            linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo);
						        if (enable_video &&
						            ((video_direction == LinphoneMediaDirectionSendOnly) ||
						             (video_direction == LinphoneMediaDirectionSendRecv)) &&
						            (linphone_participant_get_role(p) == LinphoneParticipantRoleSpeaker)) {
							        video_ok &= video_available;
						        } else {
							        video_ok &= !video_available;
						        }
					        } else {
						        video_ok = false;
					        }
				        }
				        if (devices) {
					        bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				        }
				        return video_ok;
			        }));
		}

		focus_stat = focus.getStats();
		stats marie_stat = marie.getStats();
		stats michelle_stat = michelle.getStats();
		stats berthe_stat = berthe.getStats();
		stats pauline_stat = pauline.getStats();
		stats laure_stat = laure.getStats();

		bool_t clients_have_video_send_component = ((video_direction == LinphoneMediaDirectionSendOnly) ||
		                                            (video_direction == LinphoneMediaDirectionSendRecv));
		bool_t berthe_can_screen_share = (enable_video && !all_listeners && clients_have_video_send_component);

		// Berthe enables screen sharing
		ms_message("%s enables screen sharing", linphone_core_get_identity(berthe.getLc()));
		LinphoneCall *berthe_call = linphone_core_get_call_by_remote_address2(berthe.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(berthe_call);
		if (berthe_call) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(berthe.getLc(), berthe_call);
			linphone_call_params_enable_video(new_params, TRUE);
			linphone_call_params_enable_camera(new_params, enable_camera);
			linphone_call_params_enable_screen_sharing(new_params, TRUE);
			linphone_call_update(berthe_call, new_params);
			linphone_call_params_unref(new_params);

			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallUpdating,
			                             berthe_stat.number_of_LinphoneCallUpdating + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallStreamsRunning,
			                             berthe_stat.number_of_LinphoneCallStreamsRunning + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
			                             focus_stat.number_of_LinphoneCallUpdatedByRemote + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
			                             focus_stat.number_of_LinphoneCallStreamsRunning + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(check_screen_sharing_sdp(focus.getCMgr(), berthe.getCMgr(), berthe_can_screen_share));
		}
		if (berthe_can_screen_share) {
			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &focus.getStats().number_of_participant_devices_screen_sharing_enabled,
			    focus_stat.number_of_participant_devices_screen_sharing_enabled + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &marie.getStats().number_of_participant_devices_screen_sharing_enabled,
			    marie_stat.number_of_participant_devices_screen_sharing_enabled + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &michelle.getStats().number_of_participant_devices_screen_sharing_enabled,
			                             michelle_stat.number_of_participant_devices_screen_sharing_enabled + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &laure.getStats().number_of_participant_devices_screen_sharing_enabled,
			    laure_stat.number_of_participant_devices_screen_sharing_enabled + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &pauline.getStats().number_of_participant_devices_screen_sharing_enabled,
			    pauline_stat.number_of_participant_devices_screen_sharing_enabled + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &berthe.getStats().number_of_participant_devices_screen_sharing_enabled,
			    berthe_stat.number_of_participant_devices_screen_sharing_enabled + 1, liblinphone_tester_sip_timeout));

			if (!enable_camera) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating,
				                             marie_stat.number_of_LinphoneCallUpdating + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
				                             marie_stat.number_of_LinphoneCallStreamsRunning + 1,
				                             liblinphone_tester_sip_timeout));

				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallUpdating,
				                             michelle_stat.number_of_LinphoneCallUpdating + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallStreamsRunning,
				                             michelle_stat.number_of_LinphoneCallStreamsRunning + 1,
				                             liblinphone_tester_sip_timeout));

				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallUpdating,
				                             laure_stat.number_of_LinphoneCallUpdating + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallStreamsRunning,
				                             laure_stat.number_of_LinphoneCallStreamsRunning + 1,
				                             liblinphone_tester_sip_timeout));

				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
				                             focus_stat.number_of_LinphoneCallUpdatedByRemote + 4,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
				                             focus_stat.number_of_LinphoneCallStreamsRunning + 4,
				                             liblinphone_tester_sip_timeout));
			}

			if (berthe_call) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(berthe_call);
				BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), 1, int, "%0d");
				BC_ASSERT_EQUAL(linphone_call_params_screen_sharing_enabled(call_cparams), 1, int, "%0d");
			}

			LinphoneCall *focus_call =
			    linphone_core_get_call_by_remote_address2(focus.getLc(), berthe.getCMgr()->identity);
			BC_ASSERT_PTR_NOT_NULL(focus_call);
			if (focus_call) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(focus_call);
				BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), 1, int, "%0d");
				BC_ASSERT_EQUAL(linphone_call_params_screen_sharing_enabled(call_cparams), 1, int, "%0d");

				const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(focus_call);
				BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), 1, int, "%0d");
				BC_ASSERT_EQUAL(linphone_call_params_screen_sharing_enabled(call_rparams), 1, int, "%0d");
				BC_ASSERT_EQUAL(linphone_call_params_camera_enabled(call_rparams), enable_camera, int, "%0d");
			}

			for (const auto &mgr : conferenceMgrs) {
				BC_ASSERT_TRUE(
				    CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
				        .waitUntil(chrono::seconds(50), [mgr, &berthe, confAddr] {
					        LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
					        LinphoneParticipantDevice *screen_sharing_device =
					            linphone_conference_get_screen_sharing_participant_device(conference);
					        LinphoneParticipant *screen_sharing_participant =
					            linphone_conference_get_screen_sharing_participant(conference);
					        bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
					        bool_t screen_sharing_ok = TRUE;
					        for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
						        LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
						        const LinphoneAddress *device_address = linphone_participant_device_get_address(d);
						        bool_t is_berthe =
						            linphone_address_weak_equal(device_address, berthe.getCMgr()->identity);
						        bool_t is_screen_sharing = linphone_participant_device_screen_sharing_enabled(d);
						        screen_sharing_ok &= (is_screen_sharing == is_berthe);
						        LinphoneParticipant *participant =
						            linphone_conference_is_me(conference, device_address)
						                ? linphone_conference_get_me(conference)
						                : linphone_conference_find_participant(conference, device_address);
						        if (is_berthe) {
							        screen_sharing_ok &= (d == screen_sharing_device);
							        screen_sharing_ok &= (participant == screen_sharing_participant);
						        } else {
							        screen_sharing_ok &= (d != screen_sharing_device);
							        screen_sharing_ok &= (participant != screen_sharing_participant);
						        }

						        bool_t thumbnail_available =
						            linphone_participant_device_get_thumbnail_stream_availability(d);
						        if (thumbnail_available) {
							        const char *video_label =
							            linphone_participant_device_get_stream_label(d, LinphoneStreamTypeVideo);
							        const char *thumbnail_label =
							            linphone_participant_device_get_thumbnail_stream_label(d);
							        if (is_screen_sharing) {
								        screen_sharing_ok &= (strcmp(video_label, thumbnail_label) != 0);
							        } else {
								        screen_sharing_ok &= (strcmp(video_label, thumbnail_label) == 0);
							        }
						        }
					        }
					        if (devices) {
						        bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
					        }
					        return screen_sharing_ok;
				        }));
			}
		} else {
			BC_ASSERT_FALSE(wait_for_list(coresList,
			                              &focus.getStats().number_of_participant_devices_screen_sharing_enabled,
			                              focus_stat.number_of_participant_devices_screen_sharing_enabled + 1, 1000));
			BC_ASSERT_FALSE(wait_for_list(coresList,
			                              &marie.getStats().number_of_participant_devices_screen_sharing_enabled,
			                              marie_stat.number_of_participant_devices_screen_sharing_enabled + 1, 1000));
			BC_ASSERT_FALSE(
			    wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_screen_sharing_enabled,
			                  michelle_stat.number_of_participant_devices_screen_sharing_enabled + 1, 1000));
			BC_ASSERT_FALSE(wait_for_list(coresList,
			                              &berthe.getStats().number_of_participant_devices_screen_sharing_enabled,
			                              berthe_stat.number_of_participant_devices_screen_sharing_enabled + 1, 1000));
			BC_ASSERT_FALSE(wait_for_list(coresList,
			                              &pauline.getStats().number_of_participant_devices_screen_sharing_enabled,
			                              pauline_stat.number_of_participant_devices_screen_sharing_enabled + 1, 1000));
			BC_ASSERT_FALSE(wait_for_list(coresList,
			                              &laure.getStats().number_of_participant_devices_screen_sharing_enabled,
			                              laure_stat.number_of_participant_devices_screen_sharing_enabled + 1, 1000));

			if (berthe_call) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(berthe_call);
				BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams),
				                enable_video && clients_have_video_send_component, int, "%0d");
				BC_ASSERT_EQUAL(linphone_call_params_screen_sharing_enabled(call_cparams), 0, int, "%0d");
			}

			for (const auto &mgr : conferenceMgrs) {
				LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NULL(linphone_conference_get_screen_sharing_participant_device(conference));
				BC_ASSERT_PTR_NULL(linphone_conference_get_screen_sharing_participant(conference));
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
				for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					BC_ASSERT_FALSE(linphone_participant_device_screen_sharing_enabled(d));
				}
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
			}
		}

		focus_stat = focus.getStats();
		marie_stat = marie.getStats();
		michelle_stat = michelle.getStats();
		berthe_stat = berthe.getStats();
		pauline_stat = pauline.getStats();
		laure_stat = laure.getStats();

		// Laure enables screen sharing
		ms_message("%s tries to take over screen sharing from %s", linphone_core_get_identity(laure.getLc()),
		           linphone_core_get_identity(berthe.getLc()));
		LinphoneCall *laure_call = linphone_core_get_call_by_remote_address2(laure.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(laure_call);
		if (laure_call) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(laure.getLc(), laure_call);
			linphone_call_params_enable_video(new_params, TRUE);
			linphone_call_params_enable_camera(new_params, enable_camera);
			linphone_call_params_enable_screen_sharing(new_params, TRUE);
			linphone_call_update(laure_call, new_params);
			linphone_call_params_unref(new_params);

			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallUpdating,
			                             laure_stat.number_of_LinphoneCallUpdating + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallStreamsRunning,
			                             laure_stat.number_of_LinphoneCallStreamsRunning + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
			                             focus_stat.number_of_LinphoneCallUpdatedByRemote + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
			                             focus_stat.number_of_LinphoneCallStreamsRunning + 1,
			                             liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_FALSE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_screen_sharing_enabled,
		                              focus_stat.number_of_participant_devices_screen_sharing_enabled + 1, 1000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_screen_sharing_enabled,
		                              marie_stat.number_of_participant_devices_screen_sharing_enabled + 1, 1000));
		BC_ASSERT_FALSE(wait_for_list(coresList,
		                              &michelle.getStats().number_of_participant_devices_screen_sharing_enabled,
		                              michelle_stat.number_of_participant_devices_screen_sharing_enabled + 1, 1000));
		BC_ASSERT_FALSE(wait_for_list(coresList,
		                              &berthe.getStats().number_of_participant_devices_screen_sharing_enabled,
		                              berthe_stat.number_of_participant_devices_screen_sharing_enabled + 1, 1000));
		BC_ASSERT_FALSE(wait_for_list(coresList,
		                              &pauline.getStats().number_of_participant_devices_screen_sharing_enabled,
		                              pauline_stat.number_of_participant_devices_screen_sharing_enabled + 1, 1000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_screen_sharing_enabled,
		                              laure_stat.number_of_participant_devices_screen_sharing_enabled + 1, 1000));

		if (laure_call) {
			const LinphoneCallParams *laure_call_cparams = linphone_call_get_current_params(laure_call);
			BC_ASSERT_EQUAL(linphone_call_params_video_enabled(laure_call_cparams),
			                enable_video && clients_have_video_send_component, int, "%0d");
			BC_ASSERT_EQUAL(linphone_call_params_screen_sharing_enabled(laure_call_cparams), 0, int, "%0d");
		}

		for (const auto &mgr : conferenceMgrs) {
			LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
			LinphoneParticipantDevice *screen_sharing_device =
			    linphone_conference_get_screen_sharing_participant_device(conference);
			LinphoneParticipant *screen_sharing_participant =
			    linphone_conference_get_screen_sharing_participant(conference);
			bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
			for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
				LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
				const LinphoneAddress *device_address = linphone_participant_device_get_address(d);
				bool_t is_berthe = linphone_address_weak_equal(device_address, berthe.getCMgr()->identity);
				bool_t is_screen_sharing = linphone_participant_device_screen_sharing_enabled(d);
				BC_ASSERT_TRUE(is_screen_sharing == (is_berthe && berthe_can_screen_share));
				LinphoneParticipant *participant =
				    linphone_conference_is_me(conference, device_address)
				        ? linphone_conference_get_me(conference)
				        : linphone_conference_find_participant(conference, device_address);
				if (is_berthe && berthe_can_screen_share) {
					BC_ASSERT_PTR_EQUAL(d, screen_sharing_device);
					BC_ASSERT_PTR_EQUAL(participant, screen_sharing_participant);
				} else {
					BC_ASSERT_PTR_NOT_EQUAL(d, screen_sharing_device);
					BC_ASSERT_PTR_NOT_EQUAL(participant, screen_sharing_participant);
				}

				bool_t thumbnail_available = linphone_participant_device_get_thumbnail_stream_availability(d);
				if (thumbnail_available) {
					const char *video_label = linphone_participant_device_get_stream_label(d, LinphoneStreamTypeVideo);
					const char *thumbnail_label = linphone_participant_device_get_thumbnail_stream_label(d);
					if (is_screen_sharing) {
						BC_ASSERT_TRUE(strcmp(video_label, thumbnail_label) != 0);
					} else {
						BC_ASSERT_TRUE(strcmp(video_label, thumbnail_label) == 0);
					}
				}
			}
			if (devices) {
				bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
			}
		}

		bool_t laure_can_screen_share = (enable_video && clients_have_video_send_component);

		LinphoneCall *focus_laure_call =
		    linphone_core_get_call_by_remote_address2(focus.getLc(), laure.getCMgr()->identity);
		BC_ASSERT_PTR_NOT_NULL(focus_laure_call);
		if (focus_laure_call) {
			bool_t video_enabled = (enable_video && (video_direction != LinphoneMediaDirectionInactive));
			const LinphoneCallParams *call_cparams = linphone_call_get_current_params(focus_laure_call);
			BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams),
			                video_enabled && clients_have_video_send_component, int, "%0d");
			BC_ASSERT_EQUAL(linphone_call_params_screen_sharing_enabled(call_cparams), 0, int, "%0d");

			const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(focus_laure_call);
			BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), clients_have_video_send_component, int,
			                "%0d");
			BC_ASSERT_EQUAL(linphone_call_params_screen_sharing_enabled(call_rparams), laure_can_screen_share, int,
			                "%0d");
			if (video_enabled) {
				BC_ASSERT_EQUAL(linphone_call_params_camera_enabled(call_rparams),
				                laure_can_screen_share ? enable_camera : clients_have_video_send_component, int, "%0d");
			}
		}

		LinphoneCall *focus_berthe_call =
		    linphone_core_get_call_by_remote_address2(focus.getLc(), berthe.getCMgr()->identity);
		BC_ASSERT_PTR_NOT_NULL(focus_berthe_call);
		if (focus_berthe_call) {
			bool_t video_enabled = (enable_video && !all_listeners) || berthe_can_screen_share;
			const LinphoneCallParams *call_cparams = linphone_call_get_current_params(focus_berthe_call);
			BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams),
			                video_enabled && clients_have_video_send_component, int, "%0d");
			BC_ASSERT_EQUAL(linphone_call_params_screen_sharing_enabled(call_cparams), berthe_can_screen_share, int,
			                "%0d");

			const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(focus_berthe_call);
			BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), clients_have_video_send_component, int,
			                "%0d");
			BC_ASSERT_EQUAL(linphone_call_params_screen_sharing_enabled(call_rparams), berthe_can_screen_share, int,
			                "%0d");
			if (video_enabled) {
				BC_ASSERT_EQUAL(linphone_call_params_camera_enabled(call_rparams),
				                berthe_can_screen_share ? enable_camera : clients_have_video_send_component, int,
				                "%0d");
			}
		}

		if (enable_video && clients_have_video_send_component) {
			focus_stat = focus.getStats();
			marie_stat = marie.getStats();
			michelle_stat = michelle.getStats();
			berthe_stat = berthe.getStats();
			pauline_stat = pauline.getStats();
			laure_stat = laure.getStats();

			LinphoneMediaDirection new_video_direction = LinphoneMediaDirectionRecvOnly;
			set_video_settings_in_conference(focus.getCMgr(), marie.getCMgr(), participants, confAddr, TRUE,
			                                 new_video_direction, TRUE, new_video_direction);

			// Check that screen_sharing participant hasn't changed
			for (const auto &mgr : conferenceMgrs) {
				LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
				LinphoneParticipantDevice *screen_sharing_device =
				    linphone_conference_get_screen_sharing_participant_device(conference);
				LinphoneParticipant *screen_sharing_participant =
				    linphone_conference_get_screen_sharing_participant(conference);
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
				for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					const LinphoneAddress *device_address = linphone_participant_device_get_address(d);
					bool_t is_berthe = linphone_address_weak_equal(device_address, berthe.getCMgr()->identity);
					bool_t is_screen_sharing = linphone_participant_device_screen_sharing_enabled(d);
					BC_ASSERT_TRUE(is_screen_sharing == (is_berthe && berthe_can_screen_share));
					LinphoneParticipant *participant =
					    linphone_conference_is_me(conference, device_address)
					        ? linphone_conference_get_me(conference)
					        : linphone_conference_find_participant(conference, device_address);
					if (is_berthe && berthe_can_screen_share) {
						BC_ASSERT_PTR_EQUAL(d, screen_sharing_device);
						BC_ASSERT_PTR_EQUAL(participant, screen_sharing_participant);
					} else {
						BC_ASSERT_PTR_NOT_EQUAL(d, screen_sharing_device);
						BC_ASSERT_PTR_NOT_EQUAL(participant, screen_sharing_participant);
					}
				}
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
			}
		}

		focus_stat = focus.getStats();
		marie_stat = marie.getStats();
		michelle_stat = michelle.getStats();
		berthe_stat = berthe.getStats();
		pauline_stat = pauline.getStats();
		laure_stat = laure.getStats();

		ms_message("%s terminates its call to conference %s", linphone_core_get_identity(michelle.getLc()),
		           conference_address_str);
		LinphoneCall *michelle_call = linphone_core_get_call_by_remote_address2(michelle.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(michelle_call);
		if (michelle_call) {
			linphone_call_terminate(michelle_call);
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallEnd,
			                             michelle_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallReleased,
			                             michelle_stat.number_of_LinphoneCallReleased + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneSubscriptionTerminated,
			                             michelle_stat.number_of_LinphoneSubscriptionTerminated + nb_subscriptions,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
			                             focus_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
			                             focus_stat.number_of_LinphoneCallReleased + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
			                             focus_stat.number_of_LinphoneSubscriptionTerminated + nb_subscriptions,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
			                             focus_stat.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
			                             focus_stat.number_of_participant_devices_removed + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_removed,
			                             pauline_stat.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_removed,
			                             pauline_stat.number_of_participant_devices_removed + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed,
			                             marie_stat.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_removed,
			                             marie_stat.number_of_participant_devices_removed + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participants_removed,
			                             laure_stat.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_removed,
			                             laure_stat.number_of_participant_devices_removed + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_participants_removed,
			                             berthe_stat.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_participant_devices_removed,
			                             berthe_stat.number_of_participant_devices_removed + 1,
			                             liblinphone_tester_sip_timeout));
		}

		// Check that screen_sharing participant hasn't changed
		for (const auto &mgr : conferenceMgrs) {
			// Michelle left the conference
			if (mgr == michelle.getCMgr()) continue;
			LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
			LinphoneParticipantDevice *screen_sharing_device =
			    linphone_conference_get_screen_sharing_participant_device(conference);
			LinphoneParticipant *screen_sharing_participant =
			    linphone_conference_get_screen_sharing_participant(conference);
			bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
			for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
				LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
				const LinphoneAddress *device_address = linphone_participant_device_get_address(d);
				bool_t is_berthe = linphone_address_weak_equal(device_address, berthe.getCMgr()->identity);
				bool_t is_screen_sharing = linphone_participant_device_screen_sharing_enabled(d);
				BC_ASSERT_TRUE(is_screen_sharing == (is_berthe && berthe_can_screen_share));
				LinphoneParticipant *participant =
				    linphone_conference_is_me(conference, device_address)
				        ? linphone_conference_get_me(conference)
				        : linphone_conference_find_participant(conference, device_address);
				if (is_berthe && berthe_can_screen_share) {
					BC_ASSERT_PTR_EQUAL(d, screen_sharing_device);
					BC_ASSERT_PTR_EQUAL(participant, screen_sharing_participant);
				} else {
					BC_ASSERT_PTR_NOT_EQUAL(d, screen_sharing_device);
					BC_ASSERT_PTR_NOT_EQUAL(participant, screen_sharing_participant);
				}
			}
			if (devices) {
				bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
			}
		}

		focus_stat = focus.getStats();
		marie_stat = marie.getStats();
		michelle_stat = michelle.getStats();
		berthe_stat = berthe.getStats();
		pauline_stat = pauline.getStats();
		laure_stat = laure.getStats();

		LinphoneCallParams *michelle_params = linphone_core_create_call_params(michelle.getLc(), nullptr);
		linphone_call_params_set_media_encryption(michelle_params, encryption);
		linphone_call_params_set_video_direction(michelle_params, video_direction);
		ms_message("%s joins again conference %s", linphone_core_get_identity(michelle.getLc()),
		           conference_address_str);
		linphone_core_invite_address_with_params_2(michelle.getLc(), confAddr, michelle_params, NULL, nullptr);
		linphone_call_params_unref(michelle_params);
		LinphoneCall *new_michelle_call = linphone_core_get_call_by_remote_address2(michelle.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(new_michelle_call);
		if (new_michelle_call) {
			LinphoneCallLog *call_log = linphone_call_get_call_log(new_michelle_call);
			BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
		}

		wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs, focus.getCMgr(),
		                            memberList, confAddr, enable_video);

		// Check that screen_sharing participant hasn't changed
		for (const auto &mgr : conferenceMgrs) {
			LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
			LinphoneParticipantDevice *screen_sharing_device =
			    linphone_conference_get_screen_sharing_participant_device(conference);
			LinphoneParticipant *screen_sharing_participant =
			    linphone_conference_get_screen_sharing_participant(conference);
			bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
			for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
				LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
				const LinphoneAddress *device_address = linphone_participant_device_get_address(d);
				bool_t is_berthe = linphone_address_weak_equal(device_address, berthe.getCMgr()->identity);
				bool_t is_screen_sharing = linphone_participant_device_screen_sharing_enabled(d);
				BC_ASSERT_TRUE(is_screen_sharing == (is_berthe && berthe_can_screen_share));
				LinphoneParticipant *participant =
				    linphone_conference_is_me(conference, device_address)
				        ? linphone_conference_get_me(conference)
				        : linphone_conference_find_participant(conference, device_address);
				if (is_berthe && berthe_can_screen_share) {
					BC_ASSERT_PTR_EQUAL(d, screen_sharing_device);
					BC_ASSERT_PTR_EQUAL(participant, screen_sharing_participant);
				} else {
					BC_ASSERT_PTR_NOT_EQUAL(d, screen_sharing_device);
					BC_ASSERT_PTR_NOT_EQUAL(participant, screen_sharing_participant);
				}
			}
			if (devices) {
				bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
			}
		}

		focus_stat = focus.getStats();
		marie_stat = marie.getStats();
		michelle_stat = michelle.getStats();
		berthe_stat = berthe.getStats();
		pauline_stat = pauline.getStats();
		laure_stat = laure.getStats();

		// Berthe disables screen sharing by removing video capabilities
		ms_message("%s stops screen sharing by removing video capabilities",
		           linphone_core_get_identity(berthe.getLc()));
		if (berthe_call) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(berthe.getLc(), berthe_call);
			if (turn_off_screen_sharing) {
				linphone_call_params_enable_screen_sharing(new_params, FALSE);
			} else {
				linphone_call_params_enable_video(new_params, FALSE);
			}
			linphone_call_update(berthe_call, new_params);
			linphone_call_params_unref(new_params);

			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallUpdating,
			                             berthe_stat.number_of_LinphoneCallUpdating + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallStreamsRunning,
			                             berthe_stat.number_of_LinphoneCallStreamsRunning + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
			                             focus_stat.number_of_LinphoneCallUpdatedByRemote + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
			                             focus_stat.number_of_LinphoneCallStreamsRunning + 1,
			                             liblinphone_tester_sip_timeout));

			const LinphoneCallParams *call_cparams = linphone_call_get_current_params(berthe_call);
			BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), turn_off_screen_sharing, int, "%0d");
			BC_ASSERT_EQUAL(linphone_call_params_screen_sharing_enabled(call_cparams), 0, int, "%0d");
			BC_ASSERT_TRUE(check_screen_sharing_sdp(focus.getCMgr(), berthe.getCMgr(), FALSE));
		}

		if (berthe_can_screen_share) {
			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &focus.getStats().number_of_participant_devices_screen_sharing_disabled,
			    focus_stat.number_of_participant_devices_screen_sharing_disabled + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &marie.getStats().number_of_participant_devices_screen_sharing_disabled,
			    marie_stat.number_of_participant_devices_screen_sharing_disabled + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &michelle.getStats().number_of_participant_devices_screen_sharing_disabled,
			                             michelle_stat.number_of_participant_devices_screen_sharing_disabled + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &berthe.getStats().number_of_participant_devices_screen_sharing_disabled,
			    berthe_stat.number_of_participant_devices_screen_sharing_disabled + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &pauline.getStats().number_of_participant_devices_screen_sharing_disabled,
			                             pauline_stat.number_of_participant_devices_screen_sharing_disabled + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &laure.getStats().number_of_participant_devices_screen_sharing_disabled,
			    laure_stat.number_of_participant_devices_screen_sharing_disabled + 1, liblinphone_tester_sip_timeout));

			if (!turn_off_screen_sharing) {
				BC_ASSERT_TRUE(wait_for_list(coresList,
				                             &marie.getStats().number_of_participant_devices_media_capability_changed,
				                             marie_stat.number_of_participant_devices_media_capability_changed + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList,
				                             &pauline.getStats().number_of_participant_devices_media_capability_changed,
				                             pauline_stat.number_of_participant_devices_media_capability_changed + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList,
				                             &laure.getStats().number_of_participant_devices_media_capability_changed,
				                             laure_stat.number_of_participant_devices_media_capability_changed + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(
				    coresList, &michelle.getStats().number_of_participant_devices_media_capability_changed,
				    michelle_stat.number_of_participant_devices_media_capability_changed + 1,
				    liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList,
				                             &berthe.getStats().number_of_participant_devices_media_capability_changed,
				                             berthe_stat.number_of_participant_devices_media_capability_changed + 1,
				                             liblinphone_tester_sip_timeout));

				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
				                             focus_stat.number_of_LinphoneCallUpdatedByRemote + 5,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
				                             focus_stat.number_of_LinphoneCallStreamsRunning + 5,
				                             liblinphone_tester_sip_timeout));
			}

			for (const auto &mgr : conferenceMgrs) {
				LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NULL(linphone_conference_get_screen_sharing_participant_device(conference));
				BC_ASSERT_PTR_NULL(linphone_conference_get_screen_sharing_participant(conference));
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
				for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					BC_ASSERT_FALSE(linphone_participant_device_screen_sharing_enabled(d));
					bool_t thumbnail_available = linphone_participant_device_get_thumbnail_stream_availability(d);
					if (thumbnail_available) {
						const char *video_label =
						    linphone_participant_device_get_stream_label(d, LinphoneStreamTypeVideo);
						const char *thumbnail_label = linphone_participant_device_get_thumbnail_stream_label(d);
						BC_ASSERT_TRUE(strcmp(video_label, thumbnail_label) == 0);
					}
				}
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
			}
		} else {
			BC_ASSERT_FALSE(wait_for_list(coresList,
			                              &focus.getStats().number_of_participant_devices_screen_sharing_disabled,
			                              focus_stat.number_of_participant_devices_screen_sharing_disabled + 1, 1000));
			BC_ASSERT_FALSE(wait_for_list(coresList,
			                              &marie.getStats().number_of_participant_devices_screen_sharing_disabled,
			                              marie_stat.number_of_participant_devices_screen_sharing_disabled + 1, 1000));
			BC_ASSERT_FALSE(
			    wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_screen_sharing_disabled,
			                  michelle_stat.number_of_participant_devices_screen_sharing_disabled + 1, 1000));
			BC_ASSERT_FALSE(wait_for_list(coresList,
			                              &berthe.getStats().number_of_participant_devices_screen_sharing_disabled,
			                              berthe_stat.number_of_participant_devices_screen_sharing_disabled + 1, 1000));
			BC_ASSERT_FALSE(
			    wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_screen_sharing_disabled,
			                  pauline_stat.number_of_participant_devices_screen_sharing_disabled + 1, 1000));
			BC_ASSERT_FALSE(wait_for_list(coresList,
			                              &laure.getStats().number_of_participant_devices_screen_sharing_disabled,
			                              laure_stat.number_of_participant_devices_screen_sharing_disabled + 1, 1000));

			for (const auto &mgr : conferenceMgrs) {
				LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
				LinphoneParticipantDevice *screen_sharing_device =
				    linphone_conference_get_screen_sharing_participant_device(conference);
				LinphoneParticipant *screen_sharing_participant =
				    linphone_conference_get_screen_sharing_participant(conference);
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
				for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					const LinphoneAddress *device_address = linphone_participant_device_get_address(d);
					bool_t is_laure = linphone_address_weak_equal(device_address, laure.getCMgr()->identity);
					bool_t is_screen_sharing = linphone_participant_device_screen_sharing_enabled(d);
					BC_ASSERT_TRUE(is_screen_sharing == (is_laure && laure_can_screen_share));
					LinphoneParticipant *participant =
					    linphone_conference_is_me(conference, device_address)
					        ? linphone_conference_get_me(conference)
					        : linphone_conference_find_participant(conference, device_address);
					if (is_laure && laure_can_screen_share) {
						BC_ASSERT_PTR_EQUAL(d, screen_sharing_device);
						BC_ASSERT_PTR_EQUAL(participant, screen_sharing_participant);
					} else {
						BC_ASSERT_PTR_NOT_EQUAL(d, screen_sharing_device);
						BC_ASSERT_PTR_NOT_EQUAL(participant, screen_sharing_participant);
					}

					bool_t thumbnail_available = linphone_participant_device_get_thumbnail_stream_availability(d);
					if (thumbnail_available) {
						const char *video_label =
						    linphone_participant_device_get_stream_label(d, LinphoneStreamTypeVideo);
						const char *thumbnail_label = linphone_participant_device_get_thumbnail_stream_label(d);
						if (is_screen_sharing) {
							BC_ASSERT_TRUE(strcmp(video_label, thumbnail_label) != 0);
						} else {
							BC_ASSERT_TRUE(strcmp(video_label, thumbnail_label) == 0);
						}
					}
				}

				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
			}
		}

		focus_laure_call = linphone_core_get_call_by_remote_address2(focus.getLc(), laure.getCMgr()->identity);
		BC_ASSERT_PTR_NOT_NULL(focus_laure_call);
		if (focus_laure_call) {
			CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
			    .waitUntil(chrono::seconds(50), [focus_laure_call, enable_video, enable_camera, video_direction,
			                                     clients_have_video_send_component, laure_can_screen_share,
			                                     turn_off_screen_sharing] {
				    bool params_ok = true;
				    bool_t video_enabled = (enable_video && (video_direction != LinphoneMediaDirectionInactive));
				    const LinphoneCallParams *call_cparams = linphone_call_get_current_params(focus_laure_call);
				    params_ok &= (linphone_call_params_video_enabled(call_cparams) ==
				                  (video_enabled && clients_have_video_send_component));
				    params_ok &= !!!linphone_call_params_screen_sharing_enabled(call_cparams);

				    const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(focus_laure_call);
				    params_ok &=
				        (linphone_call_params_video_enabled(call_rparams) == clients_have_video_send_component);
				    // Laure send an INVITE without requesting to share its screen If Berthe disables video
				    // capabilities
				    params_ok = (linphone_call_params_screen_sharing_enabled(call_rparams) ==
				                 (turn_off_screen_sharing && laure_can_screen_share));
				    if (video_enabled) {
					    params_ok &= (linphone_call_params_camera_enabled(call_rparams) ==
					                  (laure_can_screen_share ? enable_camera : clients_have_video_send_component));
				    }
				    return params_ok;
			    });
		}

		focus_berthe_call = linphone_core_get_call_by_remote_address2(focus.getLc(), berthe.getCMgr()->identity);
		BC_ASSERT_PTR_NOT_NULL(focus_berthe_call);
		if (focus_berthe_call) {
			bool_t video_enabled = enable_video && turn_off_screen_sharing;
			const LinphoneCallParams *call_cparams = linphone_call_get_current_params(focus_berthe_call);
			BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), video_enabled, int, "%0d");
			BC_ASSERT_EQUAL(linphone_call_params_screen_sharing_enabled(call_cparams), 0, int, "%0d");

			const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(focus_berthe_call);
			BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), video_enabled, int, "%0d");
			BC_ASSERT_EQUAL(linphone_call_params_screen_sharing_enabled(call_rparams), 0, int, "%0d");
			if (video_enabled) {
				BC_ASSERT_EQUAL(linphone_call_params_camera_enabled(call_rparams),
				                berthe_can_screen_share && enable_camera, int, "%0d");
			}
		}

		wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs, focus.getCMgr(),
		                            memberList, confAddr, enable_video);

		focus_stat = focus.getStats();
		marie_stat = marie.getStats();
		michelle_stat = michelle.getStats();
		berthe_stat = berthe.getStats();
		pauline_stat = pauline.getStats();
		laure_stat = laure.getStats();

		const bctbx_list_t *focus_calls = linphone_core_get_calls(focus.getLc());
		int focus_calls_nb = static_cast<int>(bctbx_list_size(focus_calls));
		BC_ASSERT_EQUAL(focus_calls_nb, 5, int, "%d");

		auto remaining_members = members;
		for (auto mgr : members) {
			stats focus_stat2 = focus.getStats();
			stats mgr_stats = mgr->stat;
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

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd,
			                             mgr_stats.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased,
			                             mgr_stats.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated,
			                             mgr_stats.number_of_LinphoneSubscriptionTerminated + nb_subscriptions,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending,
			                             mgr_stats.number_of_LinphoneConferenceStateTerminationPending + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated,
			                             mgr_stats.number_of_LinphoneConferenceStateTerminated + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted,
			                             mgr_stats.number_of_LinphoneConferenceStateDeleted + 1,
			                             liblinphone_tester_sip_timeout));

			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr, NULL);
			BC_ASSERT_PTR_NULL(pconference);

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
		                             focus_stat.number_of_LinphoneCallEnd + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
		                  focus_stat.number_of_LinphoneSubscriptionTerminated + (focus_calls_nb * nb_subscriptions),
		                  liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
		                             focus_stat.number_of_participants_removed + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed + focus_calls_nb,
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
			size_t expected_call_logs = (mgr == michelle.getCMgr()) ? 2 : 1;
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
				linphone_participant_info_set_sequence_number(participant_info, 0);
			}

			check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info2, 0, 0,
			                            initialSubject, description, 0, LinphoneConferenceInfoStateNew, security_level,
			                            FALSE, TRUE, TRUE, FALSE);
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

void create_conference_with_screen_sharing_chat_base(time_t start_time,
                                                     int duration,
                                                     LinphoneConferenceSecurityLevel security_level,
                                                     LinphoneConferenceLayout layout,
                                                     bool_t rejoin_with_screen_sharing) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		bool_t enable_lime = (security_level == LinphoneConferenceSecurityLevelEndToEnd ? TRUE : FALSE);

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

		int nb_subscriptions = 1;
		if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
			nb_subscriptions = 2; // One more subscription for the EKT
		}

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

			if (layout == LinphoneConferenceLayoutGrid) {
				linphone_core_set_preferred_video_definition_by_name(mgr->lc, "720p");
				linphone_config_set_string(linphone_core_get_config(mgr->lc), "video", "max_conference_size", "vga");
			}

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, layout);
			}

			// Enable ICE at the account level but not at the core level
			enable_stun_in_mgr(mgr, TRUE, TRUE, FALSE, FALSE);

			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip", "update_call_when_ice_completed", TRUE);
			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip",
			                        "update_call_when_ice_completed_with_dtls", FALSE);

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		if (enable_lime) {
			configure_end_to_end_encrypted_conference_server(focus);
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(berthe.getLc()));
		}

		int nortp_timeout = 10;
		linphone_core_set_nortp_timeout(marie.getLc(), nortp_timeout);
		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), LinphoneConferenceParticipantListTypeClosed);
		linphone_core_set_conference_cleanup_period(focus.getLc(), 1);

		stats focus_stat = focus.getStats();

		std::list<LinphoneCoreManager *> participants{laure.getCMgr(), pauline.getCMgr(), michelle.getCMgr(),
		                                              berthe.getCMgr()};
		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(),    pauline.getCMgr(),
		                                                laure.getCMgr(), michelle.getCMgr(), berthe.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(),
		                                         michelle.getCMgr(), berthe.getCMgr()};

		time_t end_time = (duration <= 0) ? -1 : (start_time + duration * 60);
		const char *initialSubject = "Test characters: ^ :) ¤ çà @";
		const char *description = "Paris Baker";

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		participantList.insert(
		    std::make_pair(laure.getCMgr(), add_participant_info_to_list(&participants_info, laure.getCMgr()->identity,
		                                                                 LinphoneParticipantRoleListener, -1)));
		participantList.insert(std::make_pair(
		    pauline.getCMgr(), add_participant_info_to_list(&participants_info, pauline.getCMgr()->identity,
		                                                    LinphoneParticipantRoleSpeaker, -1)));
		participantList.insert(std::make_pair(
		    michelle.getCMgr(), add_participant_info_to_list(&participants_info, michelle.getCMgr()->identity,
		                                                     LinphoneParticipantRoleListener, -1)));
		participantList.insert(std::make_pair(
		    berthe.getCMgr(), add_participant_info_to_list(&participants_info, berthe.getCMgr()->identity,
		                                                   LinphoneParticipantRoleSpeaker, -1)));

		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, security_level, TRUE, TRUE, NULL);
		BC_ASSERT_PTR_NOT_NULL(confAddr);
		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:");

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 4,
		                             liblinphone_tester_sip_timeout));

		const LinphoneMediaEncryption encryption = (security_level == LinphoneConferenceSecurityLevelEndToEnd)
		                                               ? LinphoneMediaEncryptionZRTP
		                                               : LinphoneMediaEncryptionNone;
		for (auto mgr : members) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_call_params_set_media_encryption(new_params, encryption);
			linphone_call_params_set_video_direction(new_params, LinphoneMediaDirectionSendRecv);
			if (mgr == pauline.getCMgr()) {
				ms_message("%s is willing to join conference %s sharing its screen",
				           linphone_core_get_identity(mgr->lc), conference_address_str);
				linphone_call_params_enable_screen_sharing(new_params, TRUE);
			}
			ms_message("%s is entering conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
				BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
			}
		}

		int idx = 1;
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
			                             ((mgr == marie.getCMgr()) ? 4 : 1), liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress,
			                             1 * nb_subscriptions, 5000));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1 * nb_subscriptions, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));
			if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyEktReceived, 1,
				                             liblinphone_tester_sip_timeout));
			}

			if ((encryption == LinphoneMediaEncryptionDTLS) || (encryption == LinphoneMediaEncryptionZRTP)) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEncryptedOn, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEncryptedOn, idx,
				                             liblinphone_tester_sip_timeout));
			}

			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
				const LinphoneMediaEncryption pcall_enc = linphone_call_params_get_media_encryption(call_cparams);
				BC_ASSERT_EQUAL(pcall_enc, encryption, int, "%d");
			}
			LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(ccall);
			if (ccall) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
				const LinphoneMediaEncryption ccall_enc = linphone_call_params_get_media_encryption(call_cparams);
				BC_ASSERT_EQUAL(ccall_enc, encryption, int, "%d");
			}

			idx++;
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + 5,
		                             liblinphone_tester_sip_timeout));
		int focus_no_streams_running = 10;
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running - 5),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running,
		                             liblinphone_tester_sip_timeout));
		// If ICE is enabled, the addition to a conference may go through a resume of the call
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
		                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + (5 * nb_subscriptions),
		                             5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + (5 * nb_subscriptions), 5000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + 5, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + 5,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
		                             focus_stat.number_of_conference_participant_devices_present + 5,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
		                             focus_stat.number_of_participant_devices_present + 5,
		                             liblinphone_tester_sip_timeout));

		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
		    fill_member_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs, focus.getCMgr(),
		                            memberList, confAddr, TRUE);

		for (const auto &mgr : conferenceMgrs) {
			BC_ASSERT_TRUE(
			    CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
			        .waitUntil(chrono::seconds(50), [mgr, &pauline, confAddr] {
				        bool_t video_ok = TRUE;
				        LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
				        LinphoneParticipantDevice *screen_sharing_device =
				            linphone_conference_get_screen_sharing_participant_device(conference);
				        LinphoneParticipant *screen_sharing_participant =
				            linphone_conference_get_screen_sharing_participant(conference);
				        bool_t screen_sharing_ok = screen_sharing_participant && screen_sharing_device;
				        bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
				        for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					        LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					        const LinphoneAddress *device_address = linphone_participant_device_get_address(d);
					        LinphoneParticipant *p =
					            linphone_conference_is_me(conference, device_address)
					                ? linphone_conference_get_me(conference)
					                : linphone_conference_find_participant(conference, device_address);
					        if (p) {
						        bool_t video_available =
						            linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo);
						        if (linphone_participant_get_role(p) == LinphoneParticipantRoleSpeaker) {
							        video_ok &= video_available;
						        } else {
							        video_ok &= !video_available;
						        }
					        } else {
						        video_ok = false;
					        }
					        if (linphone_address_weak_equal(device_address, pauline.getCMgr()->identity)) {
						        screen_sharing_ok &= (d == screen_sharing_device);
						        screen_sharing_ok &= (p == screen_sharing_participant);
					        } else {
						        screen_sharing_ok &= (d != screen_sharing_device);
						        screen_sharing_ok &= (p != screen_sharing_participant);
					        }
				        }
				        if (devices) {
					        bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				        }
				        return video_ok && screen_sharing_ok;
			        }));
		}

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(1), [] {
			return false;
		});

		focus_stat = focus.getStats();
		stats marie_stat = marie.getStats();
		stats michelle_stat = michelle.getStats();
		stats berthe_stat = berthe.getStats();
		stats pauline_stat = pauline.getStats();
		stats laure_stat = laure.getStats();

		ms_message("%s terminates its call to conference %s", linphone_core_get_identity(pauline.getLc()),
		           conference_address_str);
		LinphoneCall *pauline_call = linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(pauline_call);
		if (pauline_call) {
			linphone_call_terminate(pauline_call);
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallEnd,
			                             pauline_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallReleased,
			                             pauline_stat.number_of_LinphoneCallReleased + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneSubscriptionTerminated,
			                             pauline_stat.number_of_LinphoneSubscriptionTerminated + nb_subscriptions,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
			                             focus_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
			                             focus_stat.number_of_LinphoneCallReleased + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
			                             focus_stat.number_of_LinphoneSubscriptionTerminated + nb_subscriptions,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
			                             focus_stat.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
			                             focus_stat.number_of_participant_devices_removed + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participants_removed,
			                             michelle_stat.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_removed,
			                             michelle_stat.number_of_participant_devices_removed + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed,
			                             marie_stat.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_removed,
			                             marie_stat.number_of_participant_devices_removed + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participants_removed,
			                             laure_stat.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_removed,
			                             laure_stat.number_of_participant_devices_removed + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_participants_removed,
			                             berthe_stat.number_of_participants_removed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_participant_devices_removed,
			                             berthe_stat.number_of_participant_devices_removed + 1,
			                             liblinphone_tester_sip_timeout));
		}

		for (const auto &mgr : conferenceMgrs) {
			BC_ASSERT_TRUE(
			    CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
			        .waitUntil(chrono::seconds(50), [mgr, confAddr] {
				        bool_t video_ok = TRUE;
				        LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
				        LinphoneParticipantDevice *screen_sharing_device =
				            linphone_conference_get_screen_sharing_participant_device(conference);
				        LinphoneParticipant *screen_sharing_participant =
				            linphone_conference_get_screen_sharing_participant(conference);
				        bool_t screen_sharing_ok = !screen_sharing_participant && !screen_sharing_device;
				        bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
				        for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					        LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					        const LinphoneAddress *device_address = linphone_participant_device_get_address(d);
					        LinphoneParticipant *p =
					            linphone_conference_is_me(conference, device_address)
					                ? linphone_conference_get_me(conference)
					                : linphone_conference_find_participant(conference, device_address);
					        if (p) {
						        bool_t video_available =
						            linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo);
						        if (linphone_participant_get_role(p) == LinphoneParticipantRoleSpeaker) {
							        video_ok &= video_available;
						        } else {
							        video_ok &= !video_available;
						        }
					        } else {
						        video_ok = false;
					        }
				        }
				        if (devices) {
					        bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				        }
				        return video_ok && screen_sharing_ok;
			        }));
		}

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(1), [] {
			return false;
		});

		focus_stat = focus.getStats();
		marie_stat = marie.getStats();
		michelle_stat = michelle.getStats();
		berthe_stat = berthe.getStats();
		pauline_stat = pauline.getStats();
		laure_stat = laure.getStats();

		ms_message("%s rejoins conference %s", linphone_core_get_identity(pauline.getLc()), conference_address_str);
		LinphoneCallParams *pauline_new_params = linphone_core_create_call_params(pauline.getLc(), nullptr);
		linphone_call_params_set_video_direction(pauline_new_params, LinphoneMediaDirectionSendRecv);
		linphone_call_params_set_media_encryption(pauline_new_params, encryption);
		linphone_call_params_enable_screen_sharing(pauline_new_params, rejoin_with_screen_sharing);
		linphone_core_invite_address_with_params_2(pauline.getLc(), confAddr, pauline_new_params, NULL, nullptr);
		linphone_call_params_unref(pauline_new_params);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning,
		                             pauline_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneSubscriptionActive,
		                             pauline_stat.number_of_LinphoneSubscriptionActive + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participants_added,
		                             michelle_stat.number_of_participants_added + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_added,
		                             michelle_stat.number_of_participant_devices_added + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_added,
		                             marie_stat.number_of_participants_added + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_added,
		                             marie_stat.number_of_participant_devices_added + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participants_added,
		                             laure_stat.number_of_participants_added + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_added,
		                             laure_stat.number_of_participant_devices_added + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_participants_added,
		                             berthe_stat.number_of_participants_added + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_participant_devices_added,
		                             berthe_stat.number_of_participant_devices_added + 1,
		                             liblinphone_tester_sip_timeout));

		wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs, focus.getCMgr(),
		                            memberList, confAddr, TRUE);

		for (const auto &mgr : conferenceMgrs) {
			BC_ASSERT_TRUE(
			    CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
			        .waitUntil(chrono::seconds(50), [mgr, &pauline, confAddr, rejoin_with_screen_sharing] {
				        bool_t video_ok = TRUE;
				        LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
				        LinphoneParticipantDevice *screen_sharing_device =
				            linphone_conference_get_screen_sharing_participant_device(conference);
				        LinphoneParticipant *screen_sharing_participant =
				            linphone_conference_get_screen_sharing_participant(conference);
				        bool_t screen_sharing_ok = (rejoin_with_screen_sharing)
				                                       ? (screen_sharing_participant && screen_sharing_device)
				                                       : (!screen_sharing_participant && !screen_sharing_device);
				        bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
				        for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					        LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					        const LinphoneAddress *device_address = linphone_participant_device_get_address(d);
					        LinphoneParticipant *p =
					            linphone_conference_is_me(conference, device_address)
					                ? linphone_conference_get_me(conference)
					                : linphone_conference_find_participant(conference, device_address);
					        if (p) {
						        bool_t video_available =
						            linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo);
						        if (linphone_participant_get_role(p) == LinphoneParticipantRoleSpeaker) {
							        video_ok &= video_available;
						        } else {
							        video_ok &= !video_available;
						        }
					        } else {
						        video_ok = false;
					        }
					        if (rejoin_with_screen_sharing) {
						        if (linphone_address_weak_equal(device_address, pauline.getCMgr()->identity)) {
							        screen_sharing_ok &= (d == screen_sharing_device);
							        screen_sharing_ok &= (p == screen_sharing_participant);
						        } else {
							        screen_sharing_ok &= (d != screen_sharing_device);
							        screen_sharing_ok &= (p != screen_sharing_participant);
						        }
					        }
				        }
				        if (devices) {
					        bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				        }
				        return video_ok && screen_sharing_ok;
			        }));
		}

		if (!rejoin_with_screen_sharing) {
			ms_message("%s enables screen sharing", linphone_core_get_identity(pauline.getLc()));
			LinphoneCall *pauline_call = linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(pauline_call);
			if (pauline_call) {

				focus_stat = focus.getStats();
				marie_stat = marie.getStats();
				michelle_stat = michelle.getStats();
				berthe_stat = berthe.getStats();
				pauline_stat = pauline.getStats();
				laure_stat = laure.getStats();

				LinphoneCallParams *screen_sharing_new_params =
				    linphone_core_create_call_params(pauline.getLc(), pauline_call);
				linphone_call_params_enable_video(screen_sharing_new_params, TRUE);
				linphone_call_params_enable_camera(screen_sharing_new_params, TRUE);
				linphone_call_params_enable_screen_sharing(screen_sharing_new_params, TRUE);
				linphone_call_update(pauline_call, screen_sharing_new_params);
				linphone_call_params_unref(screen_sharing_new_params);

				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallUpdating,
				                             pauline_stat.number_of_LinphoneCallUpdating + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning,
				                             pauline_stat.number_of_LinphoneCallStreamsRunning + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
				                             focus_stat.number_of_LinphoneCallUpdatedByRemote + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
				                             focus_stat.number_of_LinphoneCallStreamsRunning + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(check_screen_sharing_sdp(focus.getCMgr(), pauline.getCMgr(), TRUE));

				BC_ASSERT_TRUE(wait_for_list(coresList,
				                             &focus.getStats().number_of_participant_devices_screen_sharing_enabled,
				                             focus_stat.number_of_participant_devices_screen_sharing_enabled + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList,
				                             &marie.getStats().number_of_participant_devices_screen_sharing_enabled,
				                             marie_stat.number_of_participant_devices_screen_sharing_enabled + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList,
				                             &michelle.getStats().number_of_participant_devices_screen_sharing_enabled,
				                             michelle_stat.number_of_participant_devices_screen_sharing_enabled + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList,
				                             &laure.getStats().number_of_participant_devices_screen_sharing_enabled,
				                             laure_stat.number_of_participant_devices_screen_sharing_enabled + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList,
				                             &pauline.getStats().number_of_participant_devices_screen_sharing_enabled,
				                             pauline_stat.number_of_participant_devices_screen_sharing_enabled + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList,
				                             &berthe.getStats().number_of_participant_devices_screen_sharing_enabled,
				                             berthe_stat.number_of_participant_devices_screen_sharing_enabled + 1,
				                             liblinphone_tester_sip_timeout));

				if (pauline_call) {
					const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pauline_call);
					BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), 1, int, "%0d");
					BC_ASSERT_EQUAL(linphone_call_params_screen_sharing_enabled(call_cparams), 1, int, "%0d");
				}

				LinphoneCall *focus_call =
				    linphone_core_get_call_by_remote_address2(focus.getLc(), pauline.getCMgr()->identity);
				BC_ASSERT_PTR_NOT_NULL(focus_call);
				if (focus_call) {
					const LinphoneCallParams *call_cparams = linphone_call_get_current_params(focus_call);
					BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), 1, int, "%0d");
					BC_ASSERT_EQUAL(linphone_call_params_screen_sharing_enabled(call_cparams), 1, int, "%0d");

					const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(focus_call);
					BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), 1, int, "%0d");
					BC_ASSERT_EQUAL(linphone_call_params_screen_sharing_enabled(call_rparams), 1, int, "%0d");
					BC_ASSERT_EQUAL(linphone_call_params_camera_enabled(call_rparams), 1, int, "%0d");
				}

				for (const auto &mgr : conferenceMgrs) {
					BC_ASSERT_TRUE(
					    CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
					        .waitUntil(chrono::seconds(50), [mgr, &pauline, confAddr] {
						        LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
						        LinphoneParticipantDevice *screen_sharing_device =
						            linphone_conference_get_screen_sharing_participant_device(conference);
						        BC_ASSERT_PTR_NOT_NULL(screen_sharing_device);
						        LinphoneParticipant *screen_sharing_participant =
						            linphone_conference_get_screen_sharing_participant(conference);
						        BC_ASSERT_PTR_NOT_NULL(screen_sharing_participant);
						        bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
						        bool_t screen_sharing_ok = TRUE;
						        for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
							        LinphoneParticipantDevice *d =
							            (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
							        const LinphoneAddress *device_address = linphone_participant_device_get_address(d);
							        bool_t is_pauline =
							            linphone_address_weak_equal(device_address, pauline.getCMgr()->identity);
							        bool_t is_screen_sharing = linphone_participant_device_screen_sharing_enabled(d);
							        screen_sharing_ok &= (is_screen_sharing == is_pauline);
							        LinphoneParticipant *participant =
							            linphone_conference_is_me(conference, device_address)
							                ? linphone_conference_get_me(conference)
							                : linphone_conference_find_participant(conference, device_address);
							        if (is_pauline) {
								        screen_sharing_ok &= (d == screen_sharing_device);
								        screen_sharing_ok &= (participant == screen_sharing_participant);
							        } else {
								        screen_sharing_ok &= (d != screen_sharing_device);
								        screen_sharing_ok &= (participant != screen_sharing_participant);
							        }

							        bool_t thumbnail_available =
							            linphone_participant_device_get_thumbnail_stream_availability(d);
							        if (thumbnail_available) {
								        const char *video_label =
								            linphone_participant_device_get_stream_label(d, LinphoneStreamTypeVideo);
								        const char *thumbnail_label =
								            linphone_participant_device_get_thumbnail_stream_label(d);
								        if (is_screen_sharing) {
									        screen_sharing_ok &= (strcmp(video_label, thumbnail_label) != 0);
								        } else {
									        screen_sharing_ok &= (strcmp(video_label, thumbnail_label) == 0);
								        }
							        }
						        }
						        if (devices) {
							        bctbx_list_free_with_data(devices,
							                                  (void (*)(void *))linphone_participant_device_unref);
						        }
						        return screen_sharing_ok;
					        }));
				}
			}
		}

		const bctbx_list_t *focus_calls = linphone_core_get_calls(focus.getLc());
		int focus_calls_nb = static_cast<int>(bctbx_list_size(focus_calls));
		BC_ASSERT_EQUAL(focus_calls_nb, 5, int, "%d");

		auto remaining_members = members;
		for (auto mgr : members) {
			stats focus_stat2 = focus.getStats();
			stats mgr_stats = mgr->stat;
			remaining_members.pop_front();
			std::list<stats> remaining_members_stats;
			int nb_members = 0;
			for (auto remaining_mgr : remaining_members) {
				remaining_members_stats.push_back(remaining_mgr->stat);
				LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(remaining_mgr->lc, confAddr);
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

			LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(call);
			if (call) {
				ms_message("%s is terminating call with %s", linphone_core_get_identity(mgr->lc),
				           linphone_core_get_identity(focus.getLc()));
				linphone_call_terminate(call);
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd,
			                             mgr_stats.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased,
			                             mgr_stats.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated,
			                             mgr_stats.number_of_LinphoneSubscriptionTerminated + nb_subscriptions,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending,
			                             mgr_stats.number_of_LinphoneConferenceStateTerminationPending + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated,
			                             mgr_stats.number_of_LinphoneConferenceStateTerminated + 1,
			                             liblinphone_tester_sip_timeout));

			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);

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
		                             focus_stat.number_of_LinphoneCallEnd + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
		                  focus_stat.number_of_LinphoneSubscriptionTerminated + (focus_calls_nb * nb_subscriptions),
		                  liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
		                             focus_stat.number_of_participants_removed + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed + focus_calls_nb,
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

		check_delete_focus_conference_info({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs,
		                                   focus.getCMgr(), confAddr, end_time);

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
		                             liblinphone_tester_sip_timeout));
		for (auto mgr : members) {
			size_t expected_call_logs = (mgr == pauline.getCMgr()) ? 2 : 1;
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
				linphone_participant_info_set_sequence_number(participant_info, 0);
			}

			check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info2, 0, 0,
			                            initialSubject, description, 0, LinphoneConferenceInfoStateNew, security_level,
			                            FALSE, TRUE, TRUE, TRUE);
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

void create_conference_with_late_participant_addition_base(time_t start_time,
                                                           int duration,
                                                           LinphoneConferenceLayout layout,
                                                           LinphoneConferenceParticipantListType participant_list_type,
                                                           bool_t accept,
                                                           bool_t one_addition,
                                                           LinphoneConferenceSecurityLevel security_level) {
	Focus focus("chloe_dual_proxy_rc");
	{ // to make sure focus is destroyed after clients.
		bool_t enable_lime = (security_level == LinphoneConferenceSecurityLevelEndToEnd ? TRUE : FALSE);
		// Create conference on the server using an account that is not the default one
		LinphoneAccount *focus_default_account = linphone_core_get_default_account(focus.getLc());
		LinphoneAccount *focus_conference_account = NULL;
		const bctbx_list_t *focus_accounts = linphone_core_get_account_list(focus.getLc());
		for (const bctbx_list_t *focus_account_it = focus_accounts; focus_account_it != NULL;
		     focus_account_it = focus_account_it->next) {
			LinphoneAccount *account = (LinphoneAccount *)(bctbx_list_get_data(focus_account_it));
			if (account != focus_default_account) {
				focus_conference_account = account;
				break;
			}
		}

		BC_ASSERT_PTR_NOT_NULL(focus_conference_account);
		if (!focus_conference_account) {
			// Fallback to default account just to execute the test. Nonetheless the goal of this test is of using a
			// non default account
			focus_conference_account = focus_default_account;
		}

		const LinphoneAccountParams *focus_account_params = linphone_account_get_params(focus_conference_account);
		const LinphoneAddress *factory_uri =
		    linphone_account_params_get_conference_factory_address(focus_account_params);
		Address focus_conference_factory = *Address::toCpp(factory_uri);
		// Change the conference factory of Marie only
		ClientConference marie("marie_rc", focus_conference_factory, enable_lime);
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

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, layout);
			}

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		stats marie_stat = marie.getStats();
		stats pauline_stat = pauline.getStats();
		stats michelle_stat = michelle.getStats();
		stats laure_stat = laure.getStats();
		stats berthe_stat = berthe.getStats();

		if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(berthe.getLc()));

			configure_end_to_end_encrypted_conference_server(focus);
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), participant_list_type);

		stats focus_stat = focus.getStats();
		marie_stat = marie.getStats();

		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr()};
		if (one_addition) {
			participants.push_back(michelle.getCMgr());
		}

		bool is_dialout = (start_time < 0);
		time_t end_time = (duration <= 0) ? -1 : (start_time + duration * 60);
		int actual_duration = (duration < 0) ? 0 : duration;
		const char *initialSubject = "Weekly recap";
		const char *description = "What happened in the past week";

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		LinphoneParticipantRole role = LinphoneParticipantRoleSpeaker;
		for (auto &p : participants) {
			participantList.insert(
			    std::make_pair(p, add_participant_info_to_list(&participants_info, p->identity, role, -1)));
			role = (role == LinphoneParticipantRoleSpeaker) ? LinphoneParticipantRoleListener
			                                                : LinphoneParticipantRoleSpeaker;
		}
		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, security_level, TRUE, FALSE, NULL);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		if (!is_dialout) {
			// Chat room creation to send ICS
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 2,
			                             liblinphone_tester_sip_timeout));
			update_sequence_number(&participants_info, {}, 0, -1);
		}

		auto members = participants;
		members.push_back(marie.getCMgr());
		auto conferenceMgrs = members;
		conferenceMgrs.push_back(focus.getCMgr());

		if (start_time < 0) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallOutgoingInit,
			                             marie_stat.number_of_LinphoneCallOutgoingInit + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingInit,
			                  focus_stat.number_of_LinphoneCallOutgoingInit + static_cast<int>(participants.size()),
			                  liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
			                             focus_stat.number_of_LinphoneCallIncomingReceived + 1,
			                             liblinphone_tester_sip_timeout));

			for (auto mgr : participants) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallIncomingReceived, 1,
				                             liblinphone_tester_sip_timeout));
			}
		} else if (confAddr) {
			for (auto mgr : members) {
				check_conference_info_in_db(
				    mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info, start_time, actual_duration,
				    initialSubject, description, 0, LinphoneConferenceInfoStateNew,
				    (mgr == marie.getCMgr()) ? security_level : LinphoneConferenceSecurityLevelNone, FALSE, TRUE, TRUE,
				    FALSE);

				LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
				linphone_call_params_set_video_direction(new_params, LinphoneMediaDirectionSendRecv);
				if (mgr == laure.getCMgr()) {
					linphone_call_params_enable_mic(new_params, FALSE);
				}
				linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
				linphone_call_params_unref(new_params);
			}

			for (auto mgr : members) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1,
				                             liblinphone_tester_sip_timeout));
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
		                             marie_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
		                             marie_stat.number_of_LinphoneSubscriptionActive + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_conference_full_state_received,
		                             marie_stat.number_of_conference_full_state_received + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyFullStateReceived,
		                             marie_stat.number_of_NotifyFullStateReceived + 1, liblinphone_tester_sip_timeout));
		if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyEktReceived,
			                             marie_stat.number_of_NotifyEktReceived + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphonePublishOutgoingProgress,
			                             marie_stat.number_of_LinphonePublishOutgoingProgress + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphonePublishOk,
			                             marie_stat.number_of_LinphonePublishOk + 1, liblinphone_tester_sip_timeout));
		}

		LinphoneConference *oconference = linphone_core_search_conference_2(marie.getLc(), confAddr);
		if (BC_ASSERT_PTR_NOT_NULL(oconference)) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(oconference),
			                static_cast<int>(participants.size()), int, "%0d");
			bctbx_list_t *devices = linphone_conference_get_participant_device_list(oconference);
			BC_ASSERT_EQUAL(bctbx_list_size(devices), (participants.size() + 1), size_t, "%zu");
			if (devices) {
				bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
			}
		}

		if (confAddr) {
			for (auto mgr : participants) {
				LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(pcall);
				if (pcall) {
					LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
					if (start_time < 0) {
						linphone_call_accept(pcall);
					}
				}
			}
		}

		int nb_subscriptions = 1;
		if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
			nb_subscriptions = 2; // One more subscription for the EKT
		}

		add_participant_info_to_list(&participants_info, marie.getCMgr()->identity, LinphoneParticipantRoleSpeaker,
		                             (is_dialout ? -1 : 0));
		for (auto mgr : participants) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 2,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress,
			                             nb_subscriptions, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, nb_subscriptions,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));
			if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyEktReceived, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphonePublishOutgoingProgress, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphonePublishOk, 1,
				                             liblinphone_tester_sip_timeout));
			}

			check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info, start_time,
			                            actual_duration, initialSubject, description, 0, LinphoneConferenceInfoStateNew,
			                            security_level, FALSE, TRUE, TRUE, FALSE);

			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
				BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				LinphoneConferenceInfo *call_log_info = linphone_call_log_get_conference_info(call_log);
				if (BC_ASSERT_PTR_NOT_NULL(call_log_info)) {
					check_conference_info_against_db(mgr, confAddr, call_log_info, FALSE);
				}
			}
		}

		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                  focus_stat.number_of_LinphoneCallStreamsRunning + static_cast<int>(participants.size() + 1),
		                  liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated,
		                             marie_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionOutgoingProgress,
		                             marie_stat.number_of_LinphoneSubscriptionOutgoingProgress + nb_subscriptions,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
		                             marie_stat.number_of_LinphoneSubscriptionActive + nb_subscriptions,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyFullStateReceived,
		                             marie_stat.number_of_NotifyFullStateReceived + 1, liblinphone_tester_sip_timeout));
		if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyEktReceived, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphonePublishOutgoingProgress, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphonePublishOk, 1,
			                             liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
		                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived +
		                                 (static_cast<int>(members.size()) * nb_subscriptions),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive +
		                                 (static_cast<int>(members.size()) * nb_subscriptions),
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + static_cast<int>(members.size()),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + static_cast<int>(members.size()),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
		                             focus_stat.number_of_conference_participant_devices_present +
		                                 static_cast<int>(members.size()),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
		                  focus_stat.number_of_participant_devices_present + static_cast<int>(members.size()),
		                  liblinphone_tester_sip_timeout));

		LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
		    fill_member_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs, focus.getCMgr(),
		                            memberList, confAddr, TRUE);

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(15), [] {
			return false;
		});

		for (auto mgr : conferenceMgrs) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				int no_participants = 0;
				if (mgr == focus.getCMgr()) {
					no_participants = static_cast<int>(members.size());
					BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
				} else {
					no_participants = static_cast<int>(participants.size());
					BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
					LinphoneCall *current_call = linphone_core_get_current_call(mgr->lc);
					BC_ASSERT_PTR_NOT_NULL(current_call);
					if (current_call) {
						BC_ASSERT_EQUAL((int)linphone_call_get_state(current_call),
						                (int)LinphoneCallStateStreamsRunning, int, "%0d");
					}

					const LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(mgr->lc);
					bool_t enabled = !!linphone_video_activation_policy_get_automatically_initiate(pol);

					LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					size_t no_streams_audio = 0;
					size_t no_max_streams_video = (enabled || (mgr == marie.getCMgr()))
					                                  ? ((security_level == LinphoneConferenceSecurityLevelEndToEnd)
					                                         ? 2 * (participants.size() + 1)
					                                         : (participants.size() + 2))
					                                  : 1;
					size_t no_streams_video = 0;
					size_t no_streams_text = 0;

					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						no_streams_audio = compute_no_audio_streams(pcall, pconference);
						no_streams_video = compute_no_video_streams(enabled, pcall, pconference);
						_linphone_call_check_max_nb_streams(pcall, no_streams_audio, no_max_streams_video,
						                                    no_streams_text);
						_linphone_call_check_nb_active_streams(pcall, no_streams_audio, no_streams_video,
						                                       no_streams_text);
						const LinphoneCallParams *call_lparams = linphone_call_get_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
						const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
						const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
					}

					LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
					BC_ASSERT_PTR_NOT_NULL(ccall);
					if (ccall) {
						_linphone_call_check_max_nb_streams(ccall, no_streams_audio, no_max_streams_video,
						                                    no_streams_text);
						_linphone_call_check_nb_active_streams(ccall, no_streams_audio, no_streams_video,
						                                       no_streams_text);
						const LinphoneCallParams *call_lparams = linphone_call_get_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
						const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
						const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
					}
				}
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), members.size(), size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
				check_conference_me(pconference, ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())));
				bctbx_list_t *participants = linphone_conference_get_participant_list(pconference);
				for (bctbx_list_t *itp = participants; itp; itp = bctbx_list_next(itp)) {
					LinphoneParticipant *p = (LinphoneParticipant *)bctbx_list_get_data(itp);
					BC_ASSERT_TRUE(
					    linphone_participant_is_admin(p) ==
					    linphone_address_weak_equal(linphone_participant_get_address(p), marie.getCMgr()->identity));
				}
				bctbx_list_free_with_data(participants, (void (*)(void *))linphone_participant_unref);

				if (mgr != focus.getCMgr()) {
					BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
					                   .waitUntil(chrono::seconds(10), [&fconference, &pconference] {
						                   return check_conference_ssrc(fconference, pconference);
					                   }));
				}

				LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(conference);
				if (conference) {
					bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
					for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
						LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
						bool video_available =
						    !!linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo);
						LinphoneMediaDirection video_direction =
						    linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo);
						BC_ASSERT_TRUE(video_available == (((video_direction == LinphoneMediaDirectionSendOnly) ||
						                                    (video_direction == LinphoneMediaDirectionSendRecv))));
					}

					if (devices) {
						bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
					}
				}
			}
		}

		focus_stat = focus.getStats();
		marie_stat = marie.getStats();
		michelle_stat = michelle.getStats();
		berthe_stat = berthe.getStats();
		pauline_stat = pauline.getStats();
		laure_stat = laure.getStats();

		if (one_addition) {
			linphone_conference_add_participant_2(oconference, berthe.getCMgr()->identity);
		} else {
			bctbx_list_t *addresses = NULL;
			addresses = bctbx_list_append(addresses, berthe.getCMgr()->identity);
			addresses = bctbx_list_append(addresses, michelle.getCMgr()->identity);
			linphone_conference_add_participants_2(oconference, addresses);
			bctbx_list_free(addresses);
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingInit,
		                             focus_stat.number_of_LinphoneCallOutgoingInit + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingProgress,
		                             focus_stat.number_of_LinphoneCallOutgoingProgress + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallIncomingReceived,
		                             berthe_stat.number_of_LinphoneCallIncomingReceived + 1,
		                             liblinphone_tester_sip_timeout));

		if (!one_addition) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingInit,
			                             focus_stat.number_of_LinphoneCallOutgoingInit + 2,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingProgress,
			                             focus_stat.number_of_LinphoneCallOutgoingProgress + 2,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallIncomingReceived,
			                             michelle_stat.number_of_LinphoneCallIncomingReceived + 1,
			                             liblinphone_tester_sip_timeout));
		}

		LinphoneCall *berthe_call = linphone_core_get_call_by_remote_address2(berthe.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(berthe_call);

		LinphoneCall *michelle_call = linphone_core_get_call_by_remote_address2(michelle.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(michelle_call);

		int participant_added = ((one_addition) ? 1 : 2);

		LinphoneParticipantInfo *berthe_participant_info = add_participant_info_to_list(
		    &participants_info, berthe.getCMgr()->identity, LinphoneParticipantRoleSpeaker, -1);
		if (accept) {
			if (berthe_call) {
				linphone_call_accept(berthe_call);
			}

			conferenceMgrs.push_back(berthe.getCMgr());
			members.push_back(berthe.getCMgr());
			participantList.insert(std::make_pair(berthe.getCMgr(), berthe_participant_info));
			memberList.insert(std::make_pair(berthe.getCMgr(), berthe_participant_info));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
			                             focus_stat.number_of_LinphoneCallStreamsRunning + 2,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallStreamsRunning,
			                             berthe_stat.number_of_LinphoneCallStreamsRunning + 2,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneConferenceStateCreated,
			                             berthe_stat.number_of_LinphoneConferenceStateCreated + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneSubscriptionOutgoingProgress,
			                             berthe_stat.number_of_LinphoneSubscriptionOutgoingProgress + nb_subscriptions,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneSubscriptionActive,
			                             berthe_stat.number_of_LinphoneSubscriptionActive + nb_subscriptions,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_NotifyFullStateReceived,
			                             berthe_stat.number_of_NotifyFullStateReceived + 1,
			                             liblinphone_tester_sip_timeout));
			if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
				// The EKT NOTIFY may arrive quite late
				BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_NotifyEktReceived,
				                             berthe_stat.number_of_NotifyEktReceived + 1,
				                             2 * liblinphone_tester_sip_timeout));
			}

			if (!one_addition) {
				if (michelle_call) {
					linphone_call_accept(michelle_call);
				}

				conferenceMgrs.push_back(michelle.getCMgr());
				members.push_back(michelle.getCMgr());
				LinphoneParticipantInfo *participant_info = add_participant_info_to_list(
				    &participants_info, michelle.getCMgr()->identity, LinphoneParticipantRoleSpeaker, -1);
				participantList.insert(std::make_pair(michelle.getCMgr(), participant_info));
				memberList.insert(std::make_pair(michelle.getCMgr(), participant_info));

				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
				                             focus_stat.number_of_LinphoneCallStreamsRunning + 4,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallStreamsRunning,
				                             michelle_stat.number_of_LinphoneCallStreamsRunning + 2,
				                             liblinphone_tester_sip_timeout));

				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateCreated,
				                             michelle_stat.number_of_LinphoneConferenceStateCreated + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(
				    coresList, &michelle.getStats().number_of_LinphoneSubscriptionOutgoingProgress,
				    michelle_stat.number_of_LinphoneSubscriptionOutgoingProgress + 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneSubscriptionActive,
				                             michelle_stat.number_of_LinphoneSubscriptionActive + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_NotifyFullStateReceived,
				                             michelle_stat.number_of_NotifyFullStateReceived + 1,
				                             liblinphone_tester_sip_timeout));
				if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_NotifyEktReceived, 1,
					                             liblinphone_tester_sip_timeout));
				}
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
			                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived +
			                                 (participant_added * nb_subscriptions),
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
			                  focus_stat.number_of_LinphoneSubscriptionActive + (participant_added * nb_subscriptions),
			                  liblinphone_tester_sip_timeout));

#ifdef HAVE_ADVANCED_IM
			if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphonePublishRefreshing,
				                             focus_stat.number_of_LinphonePublishRefreshing + participant_added, 5000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphonePublishOk,
				                             focus_stat.number_of_LinphonePublishOk + participant_added, 5000));
				// wait a bit longer to receive all EKT packets
				CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
					return false;
				});
				does_all_participants_have_matching_ekt(focus.getCMgr(), memberList, confAddr);
			}
#endif // HAVE_ADVANCED_IM

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
			                             focus_stat.number_of_participants_added + participant_added,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_added,
			                             marie_stat.number_of_participants_added + participant_added,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_added,
			                             pauline_stat.number_of_participants_added + participant_added,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participants_added,
			                             laure_stat.number_of_participants_added + participant_added,
			                             liblinphone_tester_sip_timeout));
			if (one_addition) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participants_added,
				                             michelle_stat.number_of_participants_added + 1,
				                             liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_alerting,
			                  focus_stat.number_of_conference_participant_devices_alerting + participant_added,
			                  liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &marie.getStats().number_of_conference_participant_devices_alerting,
			                  marie_stat.number_of_conference_participant_devices_alerting + participant_added,
			                  liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &pauline.getStats().number_of_conference_participant_devices_alerting,
			                  pauline_stat.number_of_conference_participant_devices_alerting + participant_added,
			                  liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &laure.getStats().number_of_conference_participant_devices_alerting,
			                  laure_stat.number_of_conference_participant_devices_alerting + participant_added,
			                  liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
			                             focus_stat.number_of_participant_devices_added + participant_added,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_added,
			                             marie_stat.number_of_participant_devices_added + participant_added,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_added,
			                             pauline_stat.number_of_participant_devices_added + participant_added,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_added,
			                             laure_stat.number_of_participant_devices_added + participant_added,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
			                  focus_stat.number_of_conference_participant_devices_present + participant_added,
			                  liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
			                             focus_stat.number_of_participant_devices_present + participant_added,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &marie.getStats().number_of_conference_participant_devices_present,
			                  marie_stat.number_of_conference_participant_devices_present + participant_added,
			                  liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_present,
			                             marie_stat.number_of_participant_devices_present + participant_added,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &pauline.getStats().number_of_conference_participant_devices_present,
			                  pauline_stat.number_of_conference_participant_devices_present + participant_added,
			                  liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_present,
			                             pauline_stat.number_of_participant_devices_present + participant_added,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &laure.getStats().number_of_conference_participant_devices_present,
			                  laure_stat.number_of_conference_participant_devices_present + participant_added,
			                  liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_present,
			                             laure_stat.number_of_participant_devices_present + participant_added,
			                             liblinphone_tester_sip_timeout));
			if (one_addition) {
				BC_ASSERT_TRUE(wait_for_list(coresList,
				                             &michelle.getStats().number_of_conference_participant_devices_alerting,
				                             michelle_stat.number_of_conference_participant_devices_alerting + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_added,
				                             michelle_stat.number_of_participant_devices_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList,
				                             &michelle.getStats().number_of_conference_participant_devices_present,
				                             michelle_stat.number_of_conference_participant_devices_present + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_present,
				                             michelle_stat.number_of_participant_devices_present + 1,
				                             liblinphone_tester_sip_timeout));
			}
		} else {
			if (berthe_call) {
				linphone_call_decline(berthe_call, LinphoneReasonDeclined);
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallEnd,
			                             berthe_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallReleased,
			                             berthe_stat.number_of_LinphoneCallReleased + 1,
			                             liblinphone_tester_sip_timeout));

			if (!one_addition) {
				if (michelle_call) {
					linphone_call_decline(michelle_call, LinphoneReasonDeclined);
				}

				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallEnd,
				                             michelle_stat.number_of_LinphoneCallEnd + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallReleased,
				                             michelle_stat.number_of_LinphoneCallReleased + 1,
				                             liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
			                             focus_stat.number_of_LinphoneCallEnd + participant_added,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
			                             focus_stat.number_of_LinphoneCallReleased + participant_added,
			                             liblinphone_tester_sip_timeout));
		}

		wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs, focus.getCMgr(),
		                            memberList, confAddr, TRUE);

		LinphoneCall *pauline_call = linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(pauline_call);

		Address paulineAddr = pauline.getIdentity();
		LinphoneCall *focus_call = linphone_core_get_call_by_remote_address2(focus.getLc(), paulineAddr.toC());
		BC_ASSERT_PTR_NOT_NULL(focus_call);

		const LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(pauline.getLc());
		bool_t enable = !!!linphone_video_activation_policy_get_automatically_initiate(pol);

		LinphoneAddress *paulineUri = pauline.getCMgr()->identity;
		LinphoneConference *paulineConference =
		    linphone_core_search_conference(pauline.getLc(), NULL, paulineUri, confAddr, NULL);
		BC_ASSERT_PTR_NOT_NULL(paulineConference);

		for (int i = 0; i < 4; i++) {
			set_video_settings_in_conference(focus.getCMgr(), pauline.getCMgr(), members, confAddr, enable,
			                                 LinphoneMediaDirectionSendRecv, enable, LinphoneMediaDirectionSendRecv);

			if (paulineConference) {
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(paulineConference);
				for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					bool video_available =
					    !!linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo);
					if (enable) {
						LinphoneMediaDirection video_direction =
						    linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo);
						BC_ASSERT_TRUE(video_available == (((video_direction == LinphoneMediaDirectionSendOnly) ||
						                                    (video_direction == LinphoneMediaDirectionSendRecv))));
					} else {
						BC_ASSERT_FALSE(video_available);
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

		focus_stat = focus.getStats();
		pauline_stat = pauline.getStats();
		if (pauline_call) {
			ms_message("%s is terminating call with %s", linphone_core_get_identity(pauline.getLc()),
			           linphone_core_get_identity(focus.getLc()));
			linphone_call_terminate(pauline_call);
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallEnd, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallReleased, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneSubscriptionTerminated,
			                             nb_subscriptions, liblinphone_tester_sip_timeout));
			if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphonePublishCleared, 1,
				                             liblinphone_tester_sip_timeout));
			}
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &pauline.getStats().number_of_LinphoneConferenceStateTerminationPending, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateDeleted, 1,
			                             liblinphone_tester_sip_timeout));

			LinphoneConference *pconference =
			    linphone_core_search_conference(pauline.getLc(), NULL, pauline.getIdentity().toC(), confAddr, NULL);
			BC_ASSERT_PTR_NULL(pconference);

			for (auto mgr : members) {
				if (mgr != pauline.getCMgr()) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_participants_removed, 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_participant_devices_removed, 1,
					                             liblinphone_tester_sip_timeout));
				}
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
		                             focus_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
		                             focus_stat.number_of_LinphoneSubscriptionTerminated + nb_subscriptions,
		                             liblinphone_tester_sip_timeout));
		if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphonePublishCleared,
			                             focus_stat.number_of_LinphonePublishCleared + 1,
			                             liblinphone_tester_sip_timeout));
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
		                             focus_stat.number_of_participants_removed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed + 1,
		                             liblinphone_tester_sip_timeout));

		ms_message("%s rejoins conference %s", linphone_core_get_identity(pauline.getLc()),
		           Address::toCpp(confAddr)->toString().c_str());
		LinphoneCallParams *pauline_new_params = linphone_core_create_call_params(pauline.getLc(), nullptr);
		linphone_call_params_set_video_direction(pauline_new_params, LinphoneMediaDirectionSendRecv);
		linphone_core_invite_address_with_params_2(pauline.getLc(), confAddr, pauline_new_params, NULL, nullptr);
		linphone_call_params_unref(pauline_new_params);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning,
		                             pauline_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneSubscriptionActive,
		                             pauline_stat.number_of_LinphoneSubscriptionActive + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_NotifyFullStateReceived,
		                             pauline_stat.number_of_NotifyFullStateReceived + 1,
		                             liblinphone_tester_sip_timeout));

		wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs, focus.getCMgr(),
		                            memberList, confAddr, TRUE);

		focus_stat = focus.getStats();
		for (auto mgr : members) {
			LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(call);
			if (call) {
				ms_message("%s is terminating call with %s", linphone_core_get_identity(mgr->lc),
				           linphone_core_get_identity(focus.getLc()));
				linphone_call_terminate(call);
				int call_ended = (mgr == pauline.getCMgr()) ? 2 : 1;
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, call_ended,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, call_ended,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated,
				                             (call_ended * nb_subscriptions), liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending,
				                             call_ended, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated,
				                             call_ended, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, call_ended,
				                             liblinphone_tester_sip_timeout));

				LinphoneConference *pconference =
				    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr, NULL);
				BC_ASSERT_PTR_NULL(pconference);
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
		                             focus_stat.number_of_LinphoneCallEnd + ((accept) ? 5 : 4),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + ((accept) ? 5 : 4),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
		                  focus_stat.number_of_LinphoneSubscriptionTerminated + (((accept) ? 5 : 4) * nb_subscriptions),
		                  liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
		                             focus_stat.number_of_participants_removed + ((accept) ? 5 : 4),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed + ((accept) ? 5 : 4),
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
		                             liblinphone_tester_sip_timeout));

		const bctbx_list_t *calls = linphone_core_get_calls(focus.getLc());
		BC_ASSERT_EQUAL(bctbx_list_size(calls), 0, size_t, "%zu");

		for (auto mgr : {focus.getCMgr()}) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NULL(pconference);
		}

		const LinphoneAddress *focus_identity = linphone_account_params_get_identity_address(focus_account_params);
		for (auto mgr : members) {
			size_t expected_call_logs = (mgr == pauline.getCMgr()) ? 2 : 1;
			const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
			BC_ASSERT_EQUAL(bctbx_list_size(call_logs), expected_call_logs, size_t, "%zu");

			bctbx_list_t *mgr_focus_call_log = linphone_core_get_call_history_2(mgr->lc, focus_identity, mgr->identity);
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
			const char *description2 = NULL;
			if ((!one_addition && (mgr == michelle.getCMgr())) || (mgr == berthe.getCMgr())) {
				for (bctbx_list_t *it = participants_info2; it; it = bctbx_list_next(it)) {
					LinphoneParticipantInfo *participant_info = (LinphoneParticipantInfo *)bctbx_list_get_data(it);
					linphone_participant_info_set_sequence_number(participant_info, -1);
				}
			} else {
				description2 = description;
			}

			check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info2, 0, 0,
			                            initialSubject, description2, 0, LinphoneConferenceInfoStateNew, security_level,
			                            FALSE, TRUE, TRUE, FALSE);

			bctbx_list_free_with_data(participants_info2, (bctbx_list_free_func)linphone_participant_info_unref);
		}

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
			return false;
		});

		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

void create_conference_with_chat_base(LinphoneConferenceSecurityLevel security_level,
                                      bool_t server_restart,
                                      bool_t client_restart,
                                      bool_t join_after_termination,
                                      long cleanup_window,
                                      bool_t use_relay_ice_candidates,
                                      bool_t client_reenter_conference,
                                      bool_t network_drops,
                                      time_t start_time,
                                      bool_t enable_gruu_in_conference_address) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		bool_t enable_lime = (security_level == LinphoneConferenceSecurityLevelEndToEnd ? TRUE : FALSE);
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

		bctbx_list_t *coresList = NULL;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(),
		                 berthe.getCMgr()}) {
			LinphoneVideoActivationPolicy *pol =
			    linphone_factory_create_video_activation_policy(linphone_factory_get());
			linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
			linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
			linphone_core_set_video_activation_policy(mgr->lc, pol);
			linphone_video_activation_policy_unref(pol);

			if (mgr != pauline.getCMgr()) {
				linphone_core_enable_gruu_in_conference_address(mgr->lc, enable_gruu_in_conference_address);
			}

			linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);
			linphone_core_enable_video_capture(mgr->lc, TRUE);
			linphone_core_enable_video_display(mgr->lc, TRUE);

			if (mgr == focus.getCMgr()) {
				// ICE relay candidates will only be tester with Michelle
				linphone_core_set_user_agent(mgr->lc, "Natted Linphone", NULL);
			} else {
				linphone_core_set_default_conference_layout(mgr->lc, LinphoneConferenceLayoutActiveSpeaker);
				linphone_core_set_media_encryption(mgr->lc, enable_lime ? LinphoneMediaEncryptionZRTP
				                                                        : LinphoneMediaEncryptionNone);
				linphone_config_set_bool(linphone_core_get_config(mgr->lc), "sip", "chat_messages_aggregation", TRUE);
				linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip", "chat_messages_aggregation_delay",
				                        2000);

				LinphoneAccount *account = linphone_core_get_default_account(mgr->lc);
				const LinphoneAccountParams *account_params = linphone_account_get_params(account);
				LinphoneAccountParams *new_account_params = linphone_account_params_clone(account_params);
				linphone_account_params_set_conference_factory_address(new_account_params,
				                                                       focus.getConferenceFactoryAddress().toC());
				linphone_account_params_set_audio_video_conference_factory_address(
				    new_account_params, focus.getConferenceFactoryAddress().toC());
				linphone_account_set_params(account, new_account_params);
				linphone_account_params_unref(new_account_params);
			}

			if ((mgr == michelle.getCMgr()) && (!!use_relay_ice_candidates)) {
				linphone_core_set_user_agent(mgr->lc, "Natted Linphone", NULL);
				linphone_core_enable_forced_ice_relay(mgr->lc, TRUE);
			}

			// Enable ICE at the account level but not at the core level
			enable_stun_in_mgr(mgr, TRUE, TRUE, FALSE, FALSE);

			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip", "update_call_when_ice_completed", TRUE);
			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip",
			                        "update_call_when_ice_completed_with_dtls", FALSE);

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		int nortp_timeout = 10;
		linphone_core_set_nortp_timeout(marie.getLc(), nortp_timeout);
		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), LinphoneConferenceParticipantListTypeClosed);
		linphone_core_set_conference_cleanup_period(focus.getLc(), cleanup_window);

		stats focus_stat = focus.getStats();
		std::list<LinphoneCoreManager *> participants{laure.getCMgr(), pauline.getCMgr(), michelle.getCMgr(),
		                                              berthe.getCMgr()};
		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(),    pauline.getCMgr(),
		                                                laure.getCMgr(), michelle.getCMgr(), berthe.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(),
		                                         michelle.getCMgr(), berthe.getCMgr()};

		if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
			configure_end_to_end_encrypted_conference_server(focus);
		}

		int duration = 1;
		time_t end_time = (start_time + duration * 60);
		const char *initialSubject = "Test characters: ^ :) ¤ çà @";
		const char *description = "Paris Baker";

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		participantList.insert(
		    std::make_pair(laure.getCMgr(), add_participant_info_to_list(&participants_info, laure.getCMgr()->identity,
		                                                                 LinphoneParticipantRoleSpeaker, -1)));
		participantList.insert(std::make_pair(
		    pauline.getCMgr(), add_participant_info_to_list(&participants_info, pauline.getCMgr()->identity,
		                                                    LinphoneParticipantRoleListener, -1)));
		participantList.insert(std::make_pair(
		    michelle.getCMgr(), add_participant_info_to_list(&participants_info, michelle.getCMgr()->identity,
		                                                     LinphoneParticipantRoleSpeaker, -1)));
		participantList.insert(std::make_pair(
		    berthe.getCMgr(), add_participant_info_to_list(&participants_info, berthe.getCMgr()->identity,
		                                                   LinphoneParticipantRoleListener, -1)));
		participantList.insert(
		    std::make_pair(laure.getCMgr(), add_participant_info_to_list(&participants_info, laure.getCMgr()->identity,
		                                                                 LinphoneParticipantRoleSpeaker, -1)));

		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, security_level, TRUE, TRUE, NULL);
		BC_ASSERT_PTR_NOT_NULL(confAddr);
		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:");

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 4,
		                             liblinphone_tester_sip_timeout));

		if (!!server_restart) {
			coresList = bctbx_list_remove(coresList, focus.getLc());
			ms_message("%s is restarting its core", linphone_core_get_identity(focus.getLc()));
			// Restart flexisip
			focus.reStart();
			coresList = bctbx_list_append(coresList, focus.getLc());
			if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
				configure_end_to_end_encrypted_conference_server(focus);
			}
			LinphoneConference *fconference =
			    linphone_core_search_conference(focus.getLc(), NULL, confAddr, confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(fconference);
			if (fconference) {
				LinphoneChatRoom *cr = linphone_conference_get_chat_room(fconference);
				BC_ASSERT_PTR_NOT_NULL(cr);
				if (cr) {
					LinphoneChatRoomCbs *cbs = linphone_factory_create_chat_room_cbs(linphone_factory_get());
					setup_chat_room_callbacks(cbs);
					linphone_chat_room_add_callbacks(cr, cbs);
					linphone_chat_room_cbs_set_user_data(
					    cbs, ((LinphoneCoreManager *)linphone_core_get_user_data(focus.getLc()))->user_info);
					linphone_chat_room_cbs_unref(cbs);
				}
			}
		}

		const LinphoneMediaEncryption encryption =
		    enable_lime ? LinphoneMediaEncryptionZRTP : LinphoneMediaEncryptionNone;
		for (auto mgr : members) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_call_params_set_media_encryption(new_params, encryption);
			linphone_call_params_set_video_direction(new_params, LinphoneMediaDirectionSendRecv);
			ms_message("%s is entering conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
				BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
			}
		}

		int idx = 1;
		int nb_client_subscribes = (security_level == LinphoneConferenceSecurityLevelEndToEnd) ? 2 : 1;
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
			                             ((mgr == marie.getCMgr()) ? 4 : 1), liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress,
			                             nb_client_subscribes, 5000));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, nb_client_subscribes, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));
			if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyEktReceived, 1,
				                             liblinphone_tester_sip_timeout));
			}

			if ((encryption == LinphoneMediaEncryptionDTLS) || (encryption == LinphoneMediaEncryptionZRTP)) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEncryptedOn, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEncryptedOn, idx,
				                             liblinphone_tester_sip_timeout));
			}

			const LinphoneAddress *from = ((mgr == marie.getCMgr()) || (start_time > 0)) ? mgr->identity : confAddr;
			const LinphoneAddress *to = ((mgr == marie.getCMgr()) || (start_time > 0)) ? confAddr : mgr->identity;

			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
				const LinphoneMediaEncryption pcall_enc = linphone_call_params_get_media_encryption(call_cparams);
				BC_ASSERT_EQUAL(pcall_enc, encryption, int, "%d");
				LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
				BC_ASSERT_PTR_NOT_NULL(call_log);
				if (call_log) {
					BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_call_log_get_from_address(call_log), from));
					BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_call_log_get_to_address(call_log), to));
				}
			}
			LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(ccall);
			if (ccall) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
				const LinphoneMediaEncryption ccall_enc = linphone_call_params_get_media_encryption(call_cparams);
				BC_ASSERT_EQUAL(ccall_enc, encryption, int, "%d");
				LinphoneCallLog *call_log = linphone_call_get_call_log(ccall);
				BC_ASSERT_PTR_NOT_NULL(call_log);
				if (call_log) {
					BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_call_log_get_from_address(call_log), from));
					BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_call_log_get_to_address(call_log), to));
				}
			}

			LinphoneAddress *deviceAddr =
			    linphone_account_get_contact_address(linphone_core_get_default_account(mgr->lc));
			LinphoneChatRoom *core_chat_room =
			    linphone_core_search_chat_room(mgr->lc, NULL, deviceAddr, confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(core_chat_room);

			LinphoneChatRoom *conference_chat_room = NULL;
			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				conference_chat_room = linphone_conference_get_chat_room(pconference);
				BC_ASSERT_PTR_NOT_NULL(conference_chat_room);
				if (conference_chat_room) {
					BC_ASSERT_FALSE(linphone_chat_room_is_read_only(conference_chat_room));
				}
			}
			BC_ASSERT_PTR_EQUAL(conference_chat_room, core_chat_room);
			idx++;
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + 5,
		                             liblinphone_tester_sip_timeout));
		int focus_no_streams_running = 10;
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running - 5),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running,
		                             liblinphone_tester_sip_timeout));
		bctbx_list_t *infos = linphone_core_get_conference_information_list(focus.getLc());
		for (bctbx_list_t *it = infos; it; it = bctbx_list_next(it)) {
			LinphoneConferenceInfo *info = (LinphoneConferenceInfo *)it->data;
			const LinphoneAddress *uri = linphone_conference_info_get_uri(info);
			BC_ASSERT_PTR_NOT_NULL(uri);
			LinphoneConference *conference = linphone_core_search_conference_2(focus.getLc(), uri);
			BC_ASSERT_PTR_NOT_NULL(conference);
			if (conference) {
				BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, laure, berthe}).wait([&conference] {
					return (linphone_conference_get_state(conference) == LinphoneConferenceStateCreated);
				}));
			}
		}
		if (infos) {
			BC_ASSERT_EQUAL((int)bctbx_list_size(infos), 1, int, "%d");
			bctbx_list_free_with_data(infos, (bctbx_list_free_func)linphone_conference_info_unref);
		}
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		                  focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 5 * nb_client_subscribes, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + 5 * nb_client_subscribes, 5000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_chat_room_participants_added,
		                             focus_stat.number_of_chat_room_participants_added + 5,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_chat_room_participant_devices_added,
		                             focus_stat.number_of_chat_room_participant_devices_added + 5,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + 5, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + 5,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
		                             focus_stat.number_of_conference_participant_devices_present + 5,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
		                             focus_stat.number_of_participant_devices_present + 5,
		                             liblinphone_tester_sip_timeout));

		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
		    fill_member_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs, focus.getCMgr(),
		                            memberList, confAddr, TRUE);

		for (auto mgr : members) {
			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				LinphoneChatRoom *chat_room = linphone_conference_get_chat_room(pconference);
				BC_ASSERT_PTR_NOT_NULL(chat_room);
				if (chat_room) {
					bctbx_list_t *chat_room_participants = linphone_chat_room_get_participants(chat_room);
					BC_ASSERT_EQUAL(bctbx_list_size(chat_room_participants), 4, size_t, "%zu");
					bctbx_list_free_with_data(chat_room_participants, (bctbx_list_free_func)linphone_participant_unref);
					BC_ASSERT_FALSE(linphone_chat_room_is_read_only(chat_room));
					const LinphoneChatRoomParams *params = linphone_chat_room_get_current_params(chat_room);
					LinphoneChatParams *chat_params = linphone_conference_params_get_chat_params(params);
					BC_ASSERT_PTR_NOT_NULL(chat_params);
					if (chat_params) {
						LinphoneChatRoomEncryptionBackend encryption_backend =
						    enable_lime ? LinphoneChatRoomEncryptionBackendLime : LinphoneChatRoomEncryptionBackendNone;
						BC_ASSERT_EQUAL(linphone_chat_params_get_encryption_backend(chat_params), encryption_backend,
						                int, "%d");
						BC_ASSERT_EQUAL(linphone_chat_params_encryption_enabled(chat_params), !!enable_lime, int, "%d");
					}
				}
				BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, laure, berthe}).wait([&pconference] {
					bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
					size_t nb_devices = bctbx_list_size(devices);
					if (devices) {
						bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
					}
					return nb_devices == 5;
				}));
			}

			const LinphoneAddress *from = ((mgr == marie.getCMgr()) || (start_time > 0)) ? mgr->identity : confAddr;
			const LinphoneAddress *to = ((mgr == marie.getCMgr()) || (start_time > 0)) ? confAddr : mgr->identity;

			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
				const LinphoneMediaEncryption pcall_enc = linphone_call_params_get_media_encryption(call_cparams);
				BC_ASSERT_EQUAL(pcall_enc, encryption, int, "%d");
				LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
				BC_ASSERT_PTR_NOT_NULL(call_log);
				if (call_log) {
					BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_call_log_get_from_address(call_log), from));
					BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_call_log_get_to_address(call_log), to));
				}
			}
			LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(ccall);
			if (ccall) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
				const LinphoneMediaEncryption ccall_enc = linphone_call_params_get_media_encryption(call_cparams);
				BC_ASSERT_EQUAL(ccall_enc, encryption, int, "%d");
				LinphoneCallLog *call_log = linphone_call_get_call_log(ccall);
				BC_ASSERT_PTR_NOT_NULL(call_log);
				if (call_log) {
					BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_call_log_get_from_address(call_log), from));
					BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_call_log_get_to_address(call_log), to));
				}
			}
		}

		const std::initializer_list<std::reference_wrapper<ClientConference>> cores2{marie, laure, pauline, michelle,
		                                                                             berthe};
		LinphoneConference *marie_conference =
		    linphone_core_search_conference(marie.getLc(), NULL, marie.getIdentity().toC(), confAddr, NULL);
		BC_ASSERT_PTR_NOT_NULL(marie_conference);
		if (marie_conference) {
			LinphoneChatRoom *marie_chat_room = linphone_conference_get_chat_room(marie_conference);
			BC_ASSERT_PTR_NOT_NULL(marie_chat_room);
			if (marie_chat_room) {
				std::string msg_text =
				    std::string("Welcome to all to conference ") + Address::toCpp(confAddr)->toString();
				LinphoneChatMessage *msg = ClientConference::sendTextMsg(marie_chat_room, msg_text);

				BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, laure, berthe}).wait([&msg] {
					return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
				}));
				linphone_chat_message_unref(msg);

				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageSent, 1,
				                             liblinphone_tester_sip_timeout));

				for (ClientConference &core : cores2) {
					LinphoneAccount *account = linphone_core_get_default_account(core.getLc());
					LinphoneAddress *deviceAddress = linphone_account_get_contact_address(account);
					LinphoneChatRoom *chatRoom = core.searchChatRoom(deviceAddress, confAddr);
					BC_ASSERT_PTR_NOT_NULL(chatRoom);
					if (chatRoom) {
						BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, laure, berthe})
						                   .waitUntil(chrono::seconds(20), [&chatRoom] {
							                   return (linphone_chat_room_get_history_size(chatRoom) == 1);
						                   }));
					}
				}
			}
		}

		LinphoneConference *pauline_conference =
		    linphone_core_search_conference(pauline.getLc(), NULL, pauline.getIdentity().toC(), confAddr, NULL);
		BC_ASSERT_PTR_NOT_NULL(pauline_conference);
		if (pauline_conference) {
			LinphoneChatRoom *pauline_chat_room = linphone_conference_get_chat_room(pauline_conference);
			BC_ASSERT_PTR_NOT_NULL(pauline_chat_room);
			if (pauline_chat_room) {
				std::string msg_text2("Happy to meet you all");
				LinphoneChatMessage *msg = ClientConference::sendTextMsg(pauline_chat_room, msg_text2);

				BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, laure, berthe}).wait([&msg] {
					return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
				}));
				linphone_chat_message_unref(msg);

				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageSent, 1,
				                             liblinphone_tester_sip_timeout));

				for (ClientConference &core : cores2) {
					LinphoneAccount *account = linphone_core_get_default_account(core.getLc());
					LinphoneAddress *deviceAddress = linphone_account_get_contact_address(account);
					LinphoneChatRoom *chatRoom = core.searchChatRoom(deviceAddress, confAddr);
					BC_ASSERT_PTR_NOT_NULL(chatRoom);
					if (chatRoom) {
						BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, laure, berthe})
						                   .waitUntil(chrono::seconds(20), [&chatRoom] {
							                   return (linphone_chat_room_get_history_size(chatRoom) == 2);
						                   }));
					}
				}
			}

			stats focus_stat2 = focus.getStats();
			stats marie_stat2 = marie.getStats();
			stats pauline_stat2 = pauline.getStats();
			stats laure_stat2 = laure.getStats();
			stats michelle_stat2 = michelle.getStats();
			stats berthe_stat2 = berthe.getStats();

			linphone_conference_leave(pauline_conference);

			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallPausing,
			                             pauline_stat2.number_of_LinphoneCallPausing + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallPaused,
			                             pauline_stat2.number_of_LinphoneCallPaused + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallPausedByRemote,
			                             focus_stat2.number_of_LinphoneCallPausedByRemote + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_on_hold,
			                             focus_stat2.number_of_conference_participant_devices_on_hold + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_on_hold,
			                             focus_stat2.number_of_participant_devices_on_hold + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_conference_participant_devices_on_hold,
			                             laure_stat2.number_of_conference_participant_devices_on_hold + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_on_hold,
			                             laure_stat2.number_of_participant_devices_on_hold + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &laure.getStats().number_of_participant_devices_media_capability_changed,
			                             laure_stat2.number_of_participant_devices_media_capability_changed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_conference_participant_devices_on_hold,
			                             marie_stat2.number_of_conference_participant_devices_on_hold + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_on_hold,
			                             marie_stat2.number_of_participant_devices_on_hold + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &marie.getStats().number_of_participant_devices_media_capability_changed,
			                             marie_stat2.number_of_participant_devices_media_capability_changed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &michelle.getStats().number_of_conference_participant_devices_on_hold,
			    michelle_stat2.number_of_conference_participant_devices_on_hold + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_on_hold,
			                             michelle_stat2.number_of_participant_devices_on_hold + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &michelle.getStats().number_of_participant_devices_media_capability_changed,
			                             michelle_stat2.number_of_participant_devices_media_capability_changed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_conference_participant_devices_on_hold,
			                             berthe_stat2.number_of_conference_participant_devices_on_hold + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_participant_devices_on_hold,
			                             berthe_stat2.number_of_participant_devices_on_hold + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &berthe.getStats().number_of_participant_devices_media_capability_changed,
			                             berthe_stat2.number_of_participant_devices_media_capability_changed + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_FALSE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_removed,
			                              pauline_stat2.number_of_participant_devices_removed + 1, 1000));
			BC_ASSERT_FALSE(wait_for_list(coresList, &pauline.getStats().number_of_participants_removed,
			                              pauline_stat2.number_of_participants_removed + 1, 1000));
			BC_ASSERT_FALSE(linphone_conference_is_in(pauline_conference));

			linphone_conference_enter(pauline_conference);

			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallResuming,
			                             pauline_stat2.number_of_LinphoneCallResuming + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning,
			                             pauline_stat2.number_of_LinphoneCallStreamsRunning + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
			                             focus_stat2.number_of_LinphoneCallStreamsRunning + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
			                             focus_stat2.number_of_conference_participant_devices_present + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
			                             focus_stat2.number_of_participant_devices_present + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_conference_participant_devices_present,
			                             laure_stat2.number_of_conference_participant_devices_present + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_present,
			                             laure_stat2.number_of_participant_devices_present + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &laure.getStats().number_of_participant_devices_media_capability_changed,
			                             laure_stat2.number_of_participant_devices_media_capability_changed + 2,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_conference_participant_devices_present,
			                             marie_stat2.number_of_conference_participant_devices_present + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_present,
			                             marie_stat2.number_of_participant_devices_present + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &marie.getStats().number_of_participant_devices_media_capability_changed,
			                             marie_stat2.number_of_participant_devices_media_capability_changed + 2,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &michelle.getStats().number_of_conference_participant_devices_present,
			    michelle_stat2.number_of_conference_participant_devices_present + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_present,
			                             michelle_stat2.number_of_participant_devices_present + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &michelle.getStats().number_of_participant_devices_media_capability_changed,
			                             michelle_stat2.number_of_participant_devices_media_capability_changed + 2,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_conference_participant_devices_present,
			                             berthe_stat2.number_of_conference_participant_devices_present + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_participant_devices_present,
			                             berthe_stat2.number_of_participant_devices_present + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &berthe.getStats().number_of_participant_devices_media_capability_changed,
			                             berthe_stat2.number_of_participant_devices_media_capability_changed + 2,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_FALSE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_added,
			                              pauline_stat2.number_of_participant_devices_added + 1, 1000));
			BC_ASSERT_FALSE(wait_for_list(coresList, &pauline.getStats().number_of_participants_added,
			                              pauline_stat2.number_of_participants_added + 1, 1000));
			BC_ASSERT_TRUE(linphone_conference_is_in(pauline_conference));
		}

		for (auto mgr : members) {
			LinphoneAddress *deviceAddr =
			    linphone_account_get_contact_address(linphone_core_get_default_account(mgr->lc));
			LinphoneChatRoom *chat_room = linphone_core_search_chat_room(mgr->lc, NULL, deviceAddr, confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(chat_room);
			if (chat_room) {
				BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(chat_room), 2, int, "%d");
			}
		}

		for (auto mgr : members) {
			LinphoneAddress *deviceAddr =
			    linphone_account_get_contact_address(linphone_core_get_default_account(mgr->lc));
			LinphoneChatRoom *chat_room = linphone_core_search_chat_room(mgr->lc, NULL, deviceAddr, confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(chat_room);
			if (chat_room) {
				BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(chat_room), 2, int, "%d");
			}
		}

		if (!!client_reenter_conference) {
			stats marie_stat = marie.getStats();
			focus_stat = focus.getStats();
			if (network_drops) {
				LinphoneChatRoom *chat_room = linphone_core_search_chat_room(focus.getLc(), NULL, NULL, confAddr, NULL);
				BC_ASSERT_PTR_NOT_NULL(chat_room);
				ms_message("%s shuts down its network", linphone_core_get_identity(marie.getLc()));
				linphone_core_set_network_reachable(marie.getLc(), FALSE);

				CoreManagerAssert({focus, marie, pauline, michelle, laure, berthe}).waitUntil(chrono::seconds(5), [] {
					return false;
				});

				ms_message("%s turns on its network", linphone_core_get_identity(marie.getLc()));
				linphone_core_set_network_reachable(marie.getLc(), TRUE);

				marie_stat = marie.getStats();
				LinphoneAccount *account = linphone_core_get_default_account(marie.getLc());
				LinphoneAddress *deviceAddress = linphone_account_get_contact_address(account);
				chat_room = linphone_core_search_chat_room(marie.getLc(), NULL, deviceAddress, confAddr, NULL);
				BC_ASSERT_PTR_NOT_NULL(chat_room);
				if (chat_room) {
					BC_ASSERT_FALSE(linphone_chat_room_is_read_only(chat_room));
					BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(chat_room), 2, int, "%d");
					BC_ASSERT_EQUAL(linphone_chat_room_get_state(chat_room), LinphoneChatRoomStateCreated, int, "%i");
				}
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
				                             marie_stat.number_of_LinphoneSubscriptionActive + nb_client_subscribes,
				                             liblinphone_tester_sip_timeout));
				LinphoneParticipantDeviceIdentity *identity =
				    linphone_factory_create_participant_device_identity(linphone_factory_get(), deviceAddress, "");
				bctbx_list_t *specs = linphone_core_get_linphone_specs_list(marie.getLc());
				linphone_participant_device_identity_set_capability_descriptor_2(identity, specs);
				bctbx_list_t *devices = NULL;
				devices = bctbx_list_append(devices, identity);
				bctbx_list_free_with_data(specs, ms_free);
				linphone_chat_room_set_participant_devices(chat_room, marie.getIdentity().toC(), devices);
				bctbx_list_free_with_data(devices, (bctbx_list_free_func)belle_sip_object_unref);
				BC_ASSERT_FALSE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingInit,
				                              focus_stat.number_of_LinphoneCallOutgoingInit + 1, 500));
				BC_ASSERT_FALSE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallIncomingReceived,
				                              marie_stat.number_of_LinphoneCallIncomingReceived + 1, 500));
				// Account for reINVITE to reestablish the session as well as the ICE reINVITE
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
				                             marie_stat.number_of_LinphoneCallStreamsRunning + 1,
				                             2 * liblinphone_tester_sip_timeout));
			} else {
				ms_message("%s hangs up its call to conference %s", linphone_core_get_identity(marie.getLc()),
				           conference_address_str);
				LinphoneCall *pcall =
				    linphone_core_get_call_by_remote_address2(marie.getLc(), focus.getCMgr()->identity);
				BC_ASSERT_PTR_NOT_NULL(pcall);
				if (pcall) {
					linphone_call_terminate(pcall);
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallEnd,
					                             marie_stat.number_of_LinphoneCallEnd + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallReleased,
					                             marie_stat.number_of_LinphoneCallReleased + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(
					    wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionTerminated,
					                  marie_stat.number_of_LinphoneSubscriptionTerminated + nb_client_subscribes,
					                  liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList,
					                             &marie.getStats().number_of_LinphoneConferenceStateTerminationPending,
					                             marie_stat.number_of_LinphoneConferenceStateTerminationPending + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(
					    coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminated,
					    marie_stat.number_of_LinphoneConferenceStateTerminated + 1, liblinphone_tester_sip_timeout));

					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
					                             focus_stat.number_of_LinphoneCallEnd + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
					                             focus_stat.number_of_LinphoneCallReleased + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(
					    wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
					                  focus_stat.number_of_LinphoneSubscriptionTerminated + nb_client_subscribes,
					                  liblinphone_tester_sip_timeout));

					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_chat_room_participants_removed,
					                             focus_stat.number_of_chat_room_participants_removed + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList,
					                             &focus.getStats().number_of_chat_room_participant_devices_removed,
					                             focus_stat.number_of_chat_room_participant_devices_removed + 1,
					                             liblinphone_tester_sip_timeout));

					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
					                             focus_stat.number_of_participants_removed + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
					                             focus_stat.number_of_participant_devices_removed + 1,
					                             liblinphone_tester_sip_timeout));

					for (auto mgr : members) {
						if (mgr != marie.getCMgr()) {
							BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_participants_removed, 1,
							                             liblinphone_tester_sip_timeout));
							BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_participant_devices_removed, 1,
							                             liblinphone_tester_sip_timeout));
						}
					}
				}

				if (client_restart) {
					LinphoneChatRoom *chat_room =
					    linphone_core_search_chat_room(focus.getLc(), NULL, NULL, confAddr, NULL);
					BC_ASSERT_PTR_NOT_NULL(chat_room);
					ms_message("%s is restarting its core", linphone_core_get_identity(marie.getLc()));
					coresList = bctbx_list_remove(coresList, marie.getLc());
					marie.reStart();
					coresList = bctbx_list_append(coresList, marie.getLc());
					marie_stat = marie.getStats();
					LinphoneAccount *account = linphone_core_get_default_account(marie.getLc());
					LinphoneAddress *deviceAddress = linphone_account_get_contact_address(account);
					chat_room = linphone_core_search_chat_room(marie.getLc(), NULL, deviceAddress, confAddr, NULL);
					BC_ASSERT_PTR_NOT_NULL(chat_room);
					if (chat_room) {
						BC_ASSERT_TRUE(linphone_chat_room_is_read_only(chat_room));
						BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(chat_room), 2, int, "%d");
						BC_ASSERT_EQUAL(linphone_chat_room_get_state(chat_room), LinphoneChatRoomStateTerminated, int,
						                "%i");
						LinphoneParticipantDeviceIdentity *identity =
						    linphone_factory_create_participant_device_identity(linphone_factory_get(), deviceAddress,
						                                                        "");
						bctbx_list_t *specs = linphone_core_get_linphone_specs_list(marie.getLc());
						linphone_participant_device_identity_set_capability_descriptor_2(identity, specs);
						bctbx_list_t *devices = NULL;
						devices = bctbx_list_append(devices, identity);
						bctbx_list_free_with_data(specs, ms_free);
						linphone_chat_room_set_participant_devices(chat_room, marie.getIdentity().toC(), devices);
						bctbx_list_free_with_data(devices, (bctbx_list_free_func)belle_sip_object_unref);
					}
					BC_ASSERT_FALSE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingInit,
					                              focus_stat.number_of_LinphoneCallOutgoingInit + 1, 500));
					BC_ASSERT_FALSE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallIncomingReceived,
					                              marie_stat.number_of_LinphoneCallIncomingReceived + 1, 500));
				}
			}

			// wait for the sound resources to be freed
			BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
			                   .waitUntil(chrono::seconds(10), [&marie] {
				                   for (const auto &call : marie.getCore().getCalls()) {
					                   if (call->getState() != CallSession::State::StreamsRunning) {
						                   return false;
					                   }
				                   }
				                   return true;
			                   }));

			LinphoneCallParams *new_params = linphone_core_create_call_params(marie.getLc(), nullptr);
			linphone_call_params_set_media_encryption(new_params, encryption);
			linphone_call_params_set_video_direction(new_params, LinphoneMediaDirectionSendRecv);
			ms_message("%s is joining again conference %s", linphone_core_get_identity(marie.getLc()),
			           conference_address_str);
			linphone_core_invite_address_with_params_2(marie.getLc(), confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(marie.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
				BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallOutgoingProgress,
				                             marie_stat.number_of_LinphoneCallOutgoingProgress + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated,
				                             marie_stat.number_of_LinphoneConferenceStateCreated + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated,
				                             marie_stat.number_of_LinphoneChatRoomStateCreated + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(
				    coresList, &marie.getStats().number_of_LinphoneSubscriptionOutgoingProgress,
				    marie_stat.number_of_LinphoneSubscriptionOutgoingProgress + nb_client_subscribes, 5000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
				                             marie_stat.number_of_LinphoneSubscriptionActive + nb_client_subscribes,
				                             5000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyFullStateReceived,
				                             marie_stat.number_of_NotifyFullStateReceived + 1,
				                             liblinphone_tester_sip_timeout));
				if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyEktReceived,
					                             marie_stat.number_of_NotifyEktReceived + 1,
					                             liblinphone_tester_sip_timeout));
				}

				if ((encryption == LinphoneMediaEncryptionDTLS) || (encryption == LinphoneMediaEncryptionZRTP)) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallEncryptedOn,
					                             marie_stat.number_of_LinphoneCallEncryptedOn + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEncryptedOn,
					                             focus_stat.number_of_LinphoneCallEncryptedOn + 1,
					                             liblinphone_tester_sip_timeout));
				}
				wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs,
				                            focus.getCMgr(), memberList, confAddr, TRUE);
			}

			for (auto mgr : members) {
				LinphoneAddress *deviceAddr =
				    linphone_account_get_contact_address(linphone_core_get_default_account(mgr->lc));
				LinphoneChatRoom *chat_room = linphone_core_search_chat_room(mgr->lc, NULL, deviceAddr, confAddr, NULL);
				BC_ASSERT_PTR_NOT_NULL(chat_room);
				if (chat_room) {
					BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(chat_room), 2, int, "%d");
				}
			}

			marie_conference =
			    linphone_core_search_conference(marie.getLc(), NULL, marie.getIdentity().toC(), confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(marie_conference);
			if (marie_conference) {
				const LinphoneConferenceParams *conference_params =
				    linphone_conference_get_current_params(marie_conference);
				BC_ASSERT_TRUE(linphone_conference_params_audio_enabled(conference_params));
				BC_ASSERT_TRUE(linphone_conference_params_video_enabled(conference_params));
				BC_ASSERT_TRUE(linphone_conference_params_chat_enabled(conference_params));
				LinphoneChatRoom *marie_chat_room = linphone_conference_get_chat_room(marie_conference);
				BC_ASSERT_PTR_NOT_NULL(marie_chat_room);
				if (marie_chat_room) {
					std::string msg_text = std::string("Sorry my call to conference ") +
					                       Address::toCpp(confAddr)->toString() +
					                       std::string(" dropped a few seconds ago");
					LinphoneChatMessage *msg = ClientConference::sendTextMsg(marie_chat_room, msg_text);

					BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, laure, berthe}).wait([&msg] {
						return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
					}));
					linphone_chat_message_unref(msg);

					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageSent, 1,
					                             liblinphone_tester_sip_timeout));

					for (ClientConference &core : cores2) {
						LinphoneAccount *account = linphone_core_get_default_account(core.getLc());
						LinphoneAddress *deviceAddress = linphone_account_get_contact_address(account);
						LinphoneChatRoom *chatRoom = core.searchChatRoom(deviceAddress, confAddr);
						BC_ASSERT_PTR_NOT_NULL(chatRoom);
						if (chatRoom) {
							BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, laure, berthe})
							                   .waitUntil(chrono::seconds(20), [&chatRoom] {
								                   return (linphone_chat_room_get_history_size(chatRoom) == 3);
							                   }));
						}
					}
				}
			}

			pauline_conference =
			    linphone_core_search_conference(pauline.getLc(), NULL, pauline.getIdentity().toC(), confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(pauline_conference);
			if (pauline_conference) {
				LinphoneChatRoom *pauline_chat_room = linphone_conference_get_chat_room(pauline_conference);
				BC_ASSERT_PTR_NOT_NULL(pauline_chat_room);
				if (pauline_chat_room) {
					std::string msg_text2("It is great to have you back with us");
					LinphoneChatMessage *msg = ClientConference::sendTextMsg(pauline_chat_room, msg_text2);

					BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, laure, berthe}).wait([&msg] {
						return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
					}));
					linphone_chat_message_unref(msg);

					BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageSent, 1,
					                             liblinphone_tester_sip_timeout));

					for (ClientConference &core : cores2) {
						LinphoneAccount *account = linphone_core_get_default_account(core.getLc());
						LinphoneAddress *deviceAddress = linphone_account_get_contact_address(account);
						LinphoneChatRoom *chatRoom = core.searchChatRoom(deviceAddress, confAddr);
						BC_ASSERT_PTR_NOT_NULL(chatRoom);
						if (chatRoom) {
							BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, laure, berthe})
							                   .waitUntil(chrono::seconds(20), [&chatRoom] {
								                   return (linphone_chat_room_get_history_size(chatRoom) == 4);
							                   }));
						}
					}
				}
			}
		}

		focus_stat = focus.getStats();

		const bctbx_list_t *focus_calls = linphone_core_get_calls(focus.getLc());
		int focus_calls_nb = static_cast<int>(bctbx_list_size(focus_calls));
		BC_ASSERT_EQUAL(focus_calls_nb, 5, int, "%d");

		auto remaining_members = members;

		for (auto mgr : members) {
			int participant_call_ended =
			    (!!client_reenter_conference && !client_restart && (mgr == marie.getCMgr())) ? 2 : 1;
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
			                             participant_call_ended * nb_client_subscribes,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending,
			                             participant_call_ended, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated,
			                             participant_call_ended, liblinphone_tester_sip_timeout));

			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				BC_ASSERT_PTR_NOT_NULL(linphone_conference_get_chat_room(pconference));
			}

			participant_calls = linphone_core_get_calls(mgr->lc);
			participant_calls_nb = static_cast<int>(bctbx_list_size(participant_calls));
			BC_ASSERT_EQUAL(participant_calls_nb, 0, int, "%d");

			for (auto remaining_mgr : remaining_members) {
				stats stat = remaining_members_stats.front();
				remaining_members_stats.pop_front();
				BC_ASSERT_TRUE(wait_for_list(coresList, &remaining_mgr->stat.number_of_chat_room_participants_removed,
				                             stat.number_of_chat_room_participants_removed + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(
				    coresList, &remaining_mgr->stat.number_of_chat_room_participant_devices_removed,
				    stat.number_of_chat_room_participant_devices_removed + 1, liblinphone_tester_sip_timeout));

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
		                             focus_stat.number_of_LinphoneCallEnd + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
		                             focus_stat.number_of_LinphoneSubscriptionTerminated + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
		                             focus_stat.number_of_participants_removed + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed + focus_calls_nb,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
		                             liblinphone_tester_sip_timeout));

		check_delete_focus_conference_info({focus, marie, pauline, michelle, laure, berthe}, conferenceMgrs,
		                                   focus.getCMgr(), confAddr, end_time);

		if (!!server_restart) {
			focus_stat = focus.getStats();
			coresList = bctbx_list_remove(coresList, focus.getLc());
			ms_message("%s is restarting its core after the conference expired",
			           linphone_core_get_identity(focus.getLc()));
			// Restart flexisip
			focus.reStart();
			coresList = bctbx_list_append(coresList, focus.getLc());

			BC_ASSERT_FALSE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingInit,
			                              focus_stat.number_of_LinphoneCallOutgoingInit + 1, 500));
			for (auto mgr : members) {
				BC_ASSERT_FALSE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallIncomingReceived, 1, 500));
			}

			const std::initializer_list<std::reference_wrapper<ClientConference>> cores2{marie, laure, pauline,
			                                                                             michelle, berthe};
			for (auto chatRoom : focus.getCore().getChatRooms()) {
				for (ClientConference &core : cores2) {
					LinphoneAccount *account = linphone_core_get_default_account(core.getLc());
					LinphoneAddress *deviceAddress = linphone_account_get_contact_address(account);
					LinphoneParticipantDeviceIdentity *identity =
					    linphone_factory_create_participant_device_identity(linphone_factory_get(), deviceAddress, "");
					bctbx_list_t *specs = linphone_core_get_linphone_specs_list(core.getLc());
					linphone_participant_device_identity_set_capability_descriptor_2(identity, specs);
					bctbx_list_t *devices = NULL;
					devices = bctbx_list_append(devices, identity);
					bctbx_list_free_with_data(specs, ms_free);
					linphone_chat_room_set_participant_devices(chatRoom->toC(), core.getIdentity().toC(), devices);
					bctbx_list_free_with_data(devices, (bctbx_list_free_func)belle_sip_object_unref);
				}
			}

			BC_ASSERT_FALSE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingInit,
			                              focus_stat.number_of_LinphoneCallOutgoingInit + 1, 500));
			for (auto mgr : members) {
				BC_ASSERT_FALSE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallIncomingReceived, 1, 500));
			}
		}

		for (auto mgr : conferenceMgrs) {
			if (mgr && (mgr != focus.getCMgr())) {
				BC_ASSERT_EQUAL(mgr->stat.number_of_LinphoneConferenceStateDeleted, 0, size_t, "%zu");

				LinphoneCall *participant_call =
				    linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
				BC_ASSERT_PTR_NULL(participant_call);
				LinphoneCall *conference_call = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
				BC_ASSERT_PTR_NULL(conference_call);

				const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
				size_t expected_call_logs = (!!client_reenter_conference && (mgr == marie.getCMgr())) ? 2 : 1;
				BC_ASSERT_EQUAL(bctbx_list_size(call_logs), expected_call_logs, size_t, "%zu");

				bctbx_list_t *mgr_focus_call_log =
				    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
				BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
				if (mgr_focus_call_log) {
					BC_ASSERT_EQUAL(bctbx_list_size(mgr_focus_call_log), expected_call_logs, size_t, "%zu");
					for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
						LinphoneCallLog *call_log = (LinphoneCallLog *)it->data;
						BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
						BC_ASSERT_PTR_NOT_NULL(linphone_call_log_get_chat_room(call_log));
					}
					bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
				}
			}
		}

		focus_stat = focus.getStats();
		const bctbx_list_t *calls = linphone_core_get_calls(focus.getLc());
		BC_ASSERT_EQUAL(bctbx_list_size(calls), 0, size_t, "%zu");

		LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NULL(fconference);

		for (auto mgr : members) {
			size_t expected_call_logs = (!!client_reenter_conference && (mgr == marie.getCMgr())) ? 2 : 1;
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
					BC_ASSERT_PTR_NOT_NULL(linphone_call_log_get_chat_room(call_log));
				}
				bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
			}

			bctbx_list_t *participants_info2 =
			    bctbx_list_copy_with_data(participants_info, (bctbx_list_copy_func)linphone_participant_info_clone);
			for (bctbx_list_t *it = participants_info2; it; it = bctbx_list_next(it)) {
				LinphoneParticipantInfo *participant_info = (LinphoneParticipantInfo *)bctbx_list_get_data(it);
				linphone_participant_info_set_sequence_number(participant_info, 0);
			}

			check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info2, start_time,
			                            duration, initialSubject, description, 0, LinphoneConferenceInfoStateNew,
			                            security_level, FALSE, TRUE, TRUE, TRUE);
			bctbx_list_free_with_data(participants_info2, (bctbx_list_free_func)linphone_participant_info_unref);

			LinphoneAddress *deviceAddr =
			    linphone_account_get_contact_address(linphone_core_get_default_account(mgr->lc));
			LinphoneChatRoom *chat_room = linphone_core_search_chat_room(mgr->lc, NULL, deviceAddr, confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(chat_room);
			if (chat_room) {
				BC_ASSERT_TRUE(linphone_chat_room_is_read_only(chat_room));
				int expected_history_size = (!!client_reenter_conference) ? 4 : 2;
				BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(chat_room), expected_history_size, int, "%d");
			}
		}

		if (!!client_restart) {
			coresList = bctbx_list_remove(coresList, marie.getLc());
			coresList = bctbx_list_remove(coresList, pauline.getLc());
			ms_message("%s and %s are restarting their core", linphone_core_get_identity(marie.getLc()),
			           linphone_core_get_identity(pauline.getLc()));
			// Restart marie and pauline
			marie.reStart();
			pauline.reStart();
			coresList = bctbx_list_append(coresList, marie.getLc());
			coresList = bctbx_list_append(coresList, pauline.getLc());
		}

		focus_stat = focus.getStats();
		for (auto mgr : {marie.getCMgr(), pauline.getCMgr()}) {
			LinphoneChatRoom *cr = linphone_core_search_chat_room(mgr->lc, NULL, NULL, confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(cr);
			if (cr) {
				std::string msg_text = std::string("Last message of ") + Address::toCpp(mgr->identity)->toString() +
				                       std::string(" in conference ") + Address::toCpp(confAddr)->toString();
				LinphoneChatMessage *msg = ClientConference::sendTextMsg(cr, msg_text);

				BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, laure, berthe}).wait([&msg] {
					return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateNotDelivered);
				}));
				linphone_chat_message_unref(msg);
			}

			if (!!join_after_termination) {
				stats mgr_stat = mgr->stat;
				LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
				linphone_call_params_set_media_encryption(new_params, encryption);
				linphone_call_params_set_video_direction(new_params, LinphoneMediaDirectionSendRecv);
				ms_message("%s is trying to join again conference %s", linphone_core_get_identity(mgr->lc),
				           conference_address_str);
				linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
				linphone_call_params_unref(new_params);
				LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(pcall);
				if (pcall) {
					linphone_call_ref(pcall);
				}
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingInit,
				                             mgr_stat.number_of_LinphoneCallOutgoingInit + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress,
				                             mgr_stat.number_of_LinphoneCallOutgoingProgress + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallError,
				                             mgr_stat.number_of_LinphoneCallError + 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased,
				                             mgr_stat.number_of_LinphoneCallReleased + 1,
				                             liblinphone_tester_sip_timeout));
				if (pcall) {
					LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
					const LinphoneErrorInfo *error_info = linphone_call_log_get_error_info(call_log);
					BC_ASSERT_PTR_NOT_NULL(error_info);
					if (error_info) {
						long focus_cleanup_window = linphone_core_get_conference_cleanup_period(focus.getLc());
						LinphoneReason reason =
						    (focus_cleanup_window <= 0) ? LinphoneReasonForbidden : LinphoneReasonGone;
						BC_ASSERT_EQUAL(linphone_error_info_get_reason(error_info), reason, int, "%d");
					}
					linphone_call_unref(pcall);
				}
			}
		}

		if (!!client_restart) {
			// Do not hide chat rooms with media so that we can detect it has been actually deleted
			linphone_config_set_int(linphone_core_get_config(marie.getLc()), "chat", "hide_chat_rooms_with_media", 0);
			stats marie_stat = marie.getStats();
			auto nbChatRoomsBefore = marie.getCore().getChatRooms().size();
			BC_ASSERT_GREATER_STRICT(nbChatRoomsBefore, 0, size_t, "%zu");
			LinphoneConferenceInfo *info = linphone_core_find_conference_information_from_uri(marie.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(info);
			if (info) {
				linphone_core_delete_conference_information(marie.getLc(), info);
				linphone_conference_info_unref(info);
				info = linphone_core_find_conference_information_from_uri(marie.getLc(), confAddr);
				BC_ASSERT_PTR_NULL(info);
			}
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateDeleted,
			                             marie_stat.number_of_LinphoneConferenceStateDeleted + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_PTR_NULL(linphone_core_search_chat_room(marie.getLc(), NULL, NULL, confAddr, NULL));
			auto nbChatRoomsAfter = marie.getCore().getChatRooms().size();
			BC_ASSERT_EQUAL(nbChatRoomsAfter, (nbChatRoomsBefore - 1), size_t, "%zu");
			coresList = bctbx_list_remove(coresList, marie.getLc());
			ms_message("%s is restarting again its core after deleting the conference information",
			           linphone_core_get_identity(marie.getLc()));
			// Restart marie and pauline
			marie.reStart();
			coresList = bctbx_list_append(coresList, marie.getLc());
			auto nbChatRoomsAfterRestart = marie.getCore().getChatRooms().size();
			BC_ASSERT_EQUAL(nbChatRoomsAfter, nbChatRoomsAfterRestart, size_t, "%zu");
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

void conference_joined_multiple_times_base(LinphoneConferenceSecurityLevel security_level,
                                           bool_t enable_chat,
                                           long cleanup_window) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		bool_t enable_lime = (security_level == LinphoneConferenceSecurityLevelEndToEnd ? TRUE : FALSE);
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

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, LinphoneConferenceLayoutActiveSpeaker);
				linphone_core_set_media_encryption(mgr->lc, enable_lime ? LinphoneMediaEncryptionZRTP
				                                                        : LinphoneMediaEncryptionNone);
				linphone_config_set_bool(linphone_core_get_config(mgr->lc), "sip", "chat_messages_aggregation", TRUE);
				linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip", "chat_messages_aggregation_delay",
				                        2000);

				LinphoneAccount *account = linphone_core_get_default_account(mgr->lc);
				const LinphoneAccountParams *account_params = linphone_account_get_params(account);
				LinphoneAccountParams *new_account_params = linphone_account_params_clone(account_params);
				linphone_account_params_set_conference_factory_address(new_account_params,
				                                                       focus.getConferenceFactoryAddress().toC());
				linphone_account_params_set_audio_video_conference_factory_address(
				    new_account_params, focus.getConferenceFactoryAddress().toC());
				linphone_account_set_params(account, new_account_params);
				linphone_account_params_unref(new_account_params);
			}

			// Enable ICE at the account level but not at the core level
			enable_stun_in_mgr(mgr, TRUE, TRUE, FALSE, FALSE);

			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip", "update_call_when_ice_completed", TRUE);
			linphone_config_set_int(linphone_core_get_config(mgr->lc), "sip",
			                        "update_call_when_ice_completed_with_dtls", FALSE);

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		int nortp_timeout = 10;
		linphone_core_set_nortp_timeout(marie.getLc(), nortp_timeout);
		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), LinphoneConferenceParticipantListTypeClosed);
		linphone_core_set_conference_cleanup_period(focus.getLc(), cleanup_window);
		long expiry_after_s = 1;
		linphone_core_set_conference_expire_period(focus.getLc(), expiry_after_s);

		std::list<LinphoneCoreManager *> participants{laure.getCMgr(), pauline.getCMgr(), michelle.getCMgr(),
		                                              berthe.getCMgr()};
		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(),    pauline.getCMgr(),
		                                                laure.getCMgr(), michelle.getCMgr(), berthe.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(),
		                                         michelle.getCMgr(), berthe.getCMgr()};

		if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
			configure_end_to_end_encrypted_conference_server(focus);
		}

		time_t start_time = ms_time(NULL);
		int duration = 2;
		time_t end_time = (start_time + duration * 60);
		const char *initialSubject = "Test characters: ^ :) ¤ çà @";
		const char *description = "Paris Baker";

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		participantList.insert(
		    std::make_pair(laure.getCMgr(), add_participant_info_to_list(&participants_info, laure.getCMgr()->identity,
		                                                                 LinphoneParticipantRoleSpeaker, -1)));
		participantList.insert(std::make_pair(
		    pauline.getCMgr(), add_participant_info_to_list(&participants_info, pauline.getCMgr()->identity,
		                                                    LinphoneParticipantRoleListener, -1)));
		participantList.insert(std::make_pair(
		    michelle.getCMgr(), add_participant_info_to_list(&participants_info, michelle.getCMgr()->identity,
		                                                     LinphoneParticipantRoleSpeaker, -1)));
		participantList.insert(std::make_pair(
		    berthe.getCMgr(), add_participant_info_to_list(&participants_info, berthe.getCMgr()->identity,
		                                                   LinphoneParticipantRoleListener, -1)));
		participantList.insert(
		    std::make_pair(laure.getCMgr(), add_participant_info_to_list(&participants_info, laure.getCMgr()->identity,
		                                                                 LinphoneParticipantRoleSpeaker, -1)));

		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, security_level, TRUE, enable_chat, NULL);
		BC_ASSERT_PTR_NOT_NULL(confAddr);
		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:");

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 4,
		                             liblinphone_tester_sip_timeout));

		int nb_client_subscribes = (security_level == LinphoneConferenceSecurityLevelEndToEnd) ? 2 : 1;
		const LinphoneMediaEncryption encryption = (security_level == LinphoneConferenceSecurityLevelNone)
		                                               ? LinphoneMediaEncryptionNone
		                                               : LinphoneMediaEncryptionZRTP;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
		    fill_member_list(members, participantList, marie.getCMgr(), participants_info);

		long focus_cleanup_window = linphone_core_get_conference_cleanup_period(focus.getLc());

		time_t end_joining_window = -1;
		if (expiry_after_s >= 0) {
			LinphoneConferenceInfo *info = linphone_core_find_conference_information_from_uri(focus.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(info);
			if (info) {
				end_joining_window = linphone_conference_info_get_expiry_time(info);
				BC_ASSERT_EQUAL((long long)(end_time + expiry_after_s), (long long)end_joining_window, long long,
				                "%0lld");
				linphone_conference_info_unref(info);
			}
		}

		time_t joining_duration = 0;
		// Client will join and leave the conference a number of times
		for (int attempt = 1; attempt <= 5; attempt++) {
			time_t initial_time = ms_time(NULL);
			if (end_joining_window > 0) {
				time_t time_left = end_joining_window - initial_time - joining_duration;
				if (time_left <= 0) {
					ms_message("Attempt #%0d - conference %s is likely to expire before all participants join it",
					           attempt, conference_address_str);
					// In order to verify the rejoining of conferences, the loop must have at least 2 iterations
					BC_ASSERT_GREATER_STRICT(attempt, 2, int, "%0d");
					break;
				}
			}
			stats focus_stat = focus.getStats();
			for (auto mgr : members) {
				LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
				linphone_call_params_set_media_encryption(new_params, encryption);
				linphone_call_params_set_video_direction(new_params, LinphoneMediaDirectionSendRecv);
				ms_message("Attempt #%0d - %s is entering conference %s", attempt, linphone_core_get_identity(mgr->lc),
				           conference_address_str);
				linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
				linphone_call_params_unref(new_params);
				LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(pcall);
				if (pcall) {
					LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				}
			}

			for (auto mgr : members) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, attempt,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating, attempt,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 2 * attempt,
				                             liblinphone_tester_sip_timeout));
				// Update to add to conference.
				// If ICE is enabled, the addition to a conference may go through a resume of the call
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, attempt,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneChatRoomStateCreated,
				                             ((mgr == marie.getCMgr()) ? 4 : 1), liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress,
				                             attempt * nb_client_subscribes, 5000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive,
				                             attempt * nb_client_subscribes, 5000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, attempt,
				                             liblinphone_tester_sip_timeout));
				if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyEktReceived, attempt,
					                             liblinphone_tester_sip_timeout));
				}

				if ((encryption == LinphoneMediaEncryptionDTLS) || (encryption == LinphoneMediaEncryptionZRTP)) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEncryptedOn, attempt,
					                             liblinphone_tester_sip_timeout));
				}

				LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(pcall);
				if (pcall) {
					const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
					const LinphoneMediaEncryption pcall_enc = linphone_call_params_get_media_encryption(call_cparams);
					BC_ASSERT_EQUAL(pcall_enc, encryption, int, "%d");
				}
				LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
				BC_ASSERT_PTR_NOT_NULL(ccall);
				if (ccall) {
					const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
					const LinphoneMediaEncryption ccall_enc = linphone_call_params_get_media_encryption(call_cparams);
					BC_ASSERT_EQUAL(ccall_enc, encryption, int, "%d");
				}

				LinphoneAddress *deviceAddr =
				    linphone_account_get_contact_address(linphone_core_get_default_account(mgr->lc));
				LinphoneChatRoom *core_chat_room =
				    linphone_core_search_chat_room(mgr->lc, NULL, deviceAddr, confAddr, NULL);
				if (enable_chat) {
					BC_ASSERT_PTR_NOT_NULL(core_chat_room);
				} else {
					BC_ASSERT_PTR_NULL(core_chat_room);
				}

				LinphoneChatRoom *conference_chat_room = NULL;
				LinphoneConference *pconference =
				    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr, NULL);
				BC_ASSERT_PTR_NOT_NULL(pconference);
				if (pconference) {
					conference_chat_room = linphone_conference_get_chat_room(pconference);
					if (enable_chat) {
						BC_ASSERT_PTR_NOT_NULL(conference_chat_room);
					} else {
						BC_ASSERT_PTR_NULL(conference_chat_room);
					}
					if (conference_chat_room) {
						BC_ASSERT_FALSE(linphone_chat_room_is_read_only(conference_chat_room));
					}
				}
				BC_ASSERT_PTR_EQUAL(conference_chat_room, core_chat_room);
			}

			if ((encryption == LinphoneMediaEncryptionDTLS) || (encryption == LinphoneMediaEncryptionZRTP)) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEncryptedOn,
				                             attempt * static_cast<int>(members.size()),
				                             liblinphone_tester_sip_timeout));
			}
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
			                             focus_stat.number_of_LinphoneCallIncomingReceived + 5,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
			                             focus_stat.number_of_LinphoneCallUpdatedByRemote + 5,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
			                             focus_stat.number_of_LinphoneCallStreamsRunning + 10,
			                             liblinphone_tester_sip_timeout));
			// If ICE is enabled, the addition to a conference may go through a resume of the call
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
			                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
			    focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 5 * nb_client_subscribes, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
			                             focus_stat.number_of_LinphoneSubscriptionActive + 5 * nb_client_subscribes,
			                             5000));

			if (enable_chat) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_chat_room_participants_added,
				                             focus_stat.number_of_chat_room_participants_added + 5,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_chat_room_participant_devices_added,
				                             focus_stat.number_of_chat_room_participant_devices_added + 5,
				                             liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
			                             focus_stat.number_of_participants_added + 5, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
			                             focus_stat.number_of_participant_devices_added + 5,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
			                             focus_stat.number_of_conference_participant_devices_present + 5,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
			                             focus_stat.number_of_participant_devices_present + 5,
			                             liblinphone_tester_sip_timeout));

			wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs,
			                            focus.getCMgr(), memberList, confAddr, TRUE);

			// Estimate the duration of an iteration to avoid that the conference expires while an INVITE session is
			// establishing
			joining_duration = ms_time(NULL) - initial_time;

			for (auto mgr : members) {
				LinphoneConference *pconference =
				    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr, NULL);
				BC_ASSERT_PTR_NOT_NULL(pconference);
				if (pconference) {
					LinphoneChatRoom *chat_room = linphone_conference_get_chat_room(pconference);
					if (enable_chat) {
						BC_ASSERT_PTR_NOT_NULL(chat_room);
					} else {
						BC_ASSERT_PTR_NULL(chat_room);
					}
					if (chat_room) {
						bctbx_list_t *chat_room_participants = linphone_chat_room_get_participants(chat_room);
						BC_ASSERT_EQUAL(bctbx_list_size(chat_room_participants), 4, size_t, "%zu");
						bctbx_list_free_with_data(chat_room_participants,
						                          (bctbx_list_free_func)linphone_participant_unref);
						BC_ASSERT_FALSE(linphone_chat_room_is_read_only(chat_room));
						const LinphoneChatRoomParams *params = linphone_chat_room_get_current_params(chat_room);
						LinphoneChatParams *chat_params = linphone_conference_params_get_chat_params(params);
						BC_ASSERT_PTR_NOT_NULL(chat_params);
						if (chat_params) {
							LinphoneChatRoomEncryptionBackend encryption_backend =
							    enable_lime ? LinphoneChatRoomEncryptionBackendLime
							                : LinphoneChatRoomEncryptionBackendNone;
							BC_ASSERT_EQUAL(linphone_chat_params_get_encryption_backend(chat_params),
							                encryption_backend, int, "%d");
							BC_ASSERT_EQUAL(linphone_chat_params_encryption_enabled(chat_params), !!enable_lime, int,
							                "%d");
						}
					}
					BC_ASSERT_TRUE(
					    CoreManagerAssert({focus, marie, pauline, michelle, laure, berthe}).wait([&pconference] {
						    bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
						    size_t nb_devices = bctbx_list_size(devices);
						    if (devices) {
							    bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
						    }
						    return nb_devices == 5;
					    }));
				}
			}

			// wait a bit longer to detect side effect if any
			CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
				return false;
			});

			if (expiry_after_s >= 0) {
				LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
				BC_ASSERT_PTR_NOT_NULL(fconference);
				if (fconference) {
					bctbx_list_t *devices = linphone_conference_get_participant_device_list(fconference);
					for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
						LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
						time_t time_of_joining = linphone_participant_device_get_time_of_joining(d);
						time_t expiry_time = time_of_joining + expiry_after_s;
						if (expiry_time > end_joining_window) {
							ms_message("Updating end of joining window for conference %s from %0ld to %0ld",
							           conference_address_str, end_joining_window, expiry_time);
							end_joining_window = expiry_time;
						}
					}

					if (devices) {
						bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
					}
				}
				BC_ASSERT_GREATER(end_joining_window, 0, long long int, "%lld");
			}

			auto remaining_members = members;
			for (auto mgr : members) {
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
					ms_message("Attempt #%0d - %s is terminating call to %s", attempt,
					           linphone_core_get_identity(mgr->lc), conference_address_str);
					linphone_call_terminate(call);
				}

				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, attempt,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, attempt,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated,
				                             attempt * nb_client_subscribes, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending,
				                             attempt, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated, attempt,
				                             liblinphone_tester_sip_timeout));
				if (!enable_chat) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted,
					                             attempt, liblinphone_tester_sip_timeout));
				}

				LinphoneConference *pconference =
				    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr, NULL);
				if (enable_chat) {
					BC_ASSERT_PTR_NOT_NULL(pconference);
					if (pconference) {
						BC_ASSERT_PTR_NOT_NULL(linphone_conference_get_chat_room(pconference));
					}
				} else {
					BC_ASSERT_PTR_NULL(pconference);
				}

				participant_calls = linphone_core_get_calls(mgr->lc);
				participant_calls_nb = static_cast<int>(bctbx_list_size(participant_calls));
				BC_ASSERT_EQUAL(participant_calls_nb, 0, int, "%d");

				for (auto remaining_mgr : remaining_members) {
					stats stat = remaining_members_stats.front();
					remaining_members_stats.pop_front();
					if (enable_chat) {
						BC_ASSERT_TRUE(wait_for_list(
						    coresList, &remaining_mgr->stat.number_of_chat_room_participants_removed,
						    stat.number_of_chat_room_participants_removed + 1, liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(
						    coresList, &remaining_mgr->stat.number_of_chat_room_participant_devices_removed,
						    stat.number_of_chat_room_participant_devices_removed + 1, liblinphone_tester_sip_timeout));
					}

					BC_ASSERT_TRUE(wait_for_list(coresList, &remaining_mgr->stat.number_of_participants_removed,
					                             stat.number_of_participants_removed + 1,
					                             liblinphone_tester_sip_timeout));
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
			                             focus_stat.number_of_LinphoneCallEnd + 5, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
			                             focus_stat.number_of_LinphoneCallReleased + 5,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
			                             focus_stat.number_of_LinphoneSubscriptionTerminated + 5,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
			                             focus_stat.number_of_participants_removed + 5,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
			                             focus_stat.number_of_participant_devices_removed + 5,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
			    focus_stat.number_of_LinphoneConferenceStateTerminationPending + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated,
			                             focus_stat.number_of_LinphoneConferenceStateTerminated + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted,
			                             focus_stat.number_of_LinphoneConferenceStateDeleted + 1,
			                             liblinphone_tester_sip_timeout));

			for (auto mgr : conferenceMgrs) {
				bool info_deleted = false;
				if (!!linphone_core_conference_server_enabled(mgr->lc) && (focus_cleanup_window > 0) &&
				    (end_joining_window > 0)) {
					time_t now = ms_time(NULL);
					time_t time_left = end_joining_window - now;
					if (time_left <= 0) {
						ms_message("Attempt #%0d - conference information of conference %s was deleted on the server "
						           "core %s because the conference expired %ld seconds ago",
						           attempt, conference_address_str, linphone_core_get_identity(mgr->lc), -time_left);
						// wait for the cleanup window to be sure the conference information is deleted
						CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
						    .waitUntil(chrono::seconds(focus_cleanup_window), [] { return false; });
						info_deleted = true;
					}
				}
				LinphoneConferenceInfo *info = linphone_core_find_conference_information_from_uri(mgr->lc, confAddr);
				if (info_deleted) {
					BC_ASSERT_PTR_NULL(info);
				} else {
					BC_ASSERT_PTR_NOT_NULL(info);
				}
				if (info) {
					linphone_conference_info_unref(info);
				}
			}

			// wait a bit longer to detect side effect if any
			CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(5), [] {
				return false;
			});
		}

		check_delete_focus_conference_info({focus, marie, pauline, michelle, laure, berthe}, conferenceMgrs,
		                                   focus.getCMgr(), confAddr, end_joining_window);

		ms_free(conference_address_str);
		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

void two_overlapping_conferences_base(bool_t same_organizer, bool_t dialout) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress());
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress());
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress());
		ClientConference berthe("berthe_rc", focus.getConferenceFactoryAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(berthe);

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		setup_conference_info_cbs(marie.getCMgr());
		if (!same_organizer) {
			linphone_core_set_file_transfer_server(michelle.getLc(), file_transfer_url);
			setup_conference_info_cbs(michelle.getCMgr());
		}

		linphone_core_set_conference_participant_list_type(focus.getLc(), LinphoneConferenceParticipantListTypeOpen);
		linphone_core_set_default_conference_layout(focus.getLc(), LinphoneConferenceLayoutGrid);

		int i = 0;
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

			linphone_core_set_default_conference_layout(mgr->lc, (i % 2) ? LinphoneConferenceLayoutGrid
			                                                             : LinphoneConferenceLayoutActiveSpeaker);
		}

		stats focus_stat = focus.getStats();

		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, laure.getLc());
		coresList = bctbx_list_append(coresList, michelle.getLc());
		coresList = bctbx_list_append(coresList, berthe.getLc());

		LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;

		std::list<LinphoneCoreManager *> participants1{pauline.getCMgr(), laure.getCMgr()};
		time_t start_time1 = ms_time(NULL) - 40;
		time_t end_time1 = (start_time1 + 60);
		const char *subject1 = "Colleagues";
		const char *description1 = NULL;
		bctbx_list_t *participants_info1 = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList1;
		LinphoneParticipantRole role = LinphoneParticipantRoleSpeaker;
		for (auto &p : participants1) {
			participantList1.insert(
			    std::make_pair(p, add_participant_info_to_list(&participants_info1, p->identity, role, -1)));
			role = (role == LinphoneParticipantRoleSpeaker) ? LinphoneParticipantRoleListener
			                                                : LinphoneParticipantRoleSpeaker;
		}
		LinphoneAddress *confAddr1 =
		    create_conference_on_server(focus, marie, participantList1, start_time1, end_time1, subject1, description1,
		                                TRUE, security_level, TRUE, FALSE, NULL);
		BC_ASSERT_PTR_NOT_NULL(confAddr1);
		char *conference1_address_str = (confAddr1) ? linphone_address_as_string(confAddr1) : ms_strdup("sip:");
		BC_ASSERT_PTR_NOT_NULL(confAddr1);
		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 2,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			ms_message("%s is entering conference %s", linphone_core_get_identity(mgr->lc), conference1_address_str);
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr1, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
		}

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1,
			                             liblinphone_tester_sip_timeout));
			LinphoneCall *currentCall = linphone_core_get_current_call(mgr->lc);
			BC_ASSERT_PTR_NOT_NULL(currentCall);
			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr1, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (currentCall && pconference) {
				BC_ASSERT_PTR_EQUAL(linphone_call_get_conference(currentCall), pconference);
			}
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 2,
			                             liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + 3,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + 3,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + 6,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
		                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 3,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + 3,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + 3, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + 3,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
		                             focus_stat.number_of_conference_participant_devices_present + 3,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
		                             focus_stat.number_of_participant_devices_present + 3,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + 3,
		                             liblinphone_tester_sip_timeout));

		LinphoneConference *fconference1 =
		    linphone_core_search_conference(focus.getLc(), NULL, confAddr1, confAddr1, NULL);
		BC_ASSERT_PTR_NOT_NULL(fconference1);

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle}).waitUntil(chrono::seconds(2), [] { return false; });

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			const LinphoneAddress *local_address = (mgr == focus.getCMgr()) ? confAddr1 : mgr->identity;
			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, local_address, confAddr1, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				const LinphoneConferenceParams *conference_params = linphone_conference_get_current_params(pconference);
				int no_participants = 0;
				if (start_time1 >= 0) {
					BC_ASSERT_EQUAL((long long)linphone_conference_params_get_start_time(conference_params),
					                (long long)start_time1, long long, "%lld");
				}
				BC_ASSERT_EQUAL((long long)linphone_conference_params_get_end_time(conference_params),
				                (long long)end_time1, long long, "%lld");
				BC_ASSERT_EQUAL((int)linphone_conference_params_get_security_level(conference_params),
				                (int)security_level, int, "%0d");
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
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), subject1);
				check_conference_me(pconference, ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())));
				bctbx_list_t *participants = linphone_conference_get_participant_list(pconference);
				for (bctbx_list_t *itp = participants; itp; itp = bctbx_list_next(itp)) {
					LinphoneParticipant *p = (LinphoneParticipant *)bctbx_list_get_data(itp);
					BC_ASSERT_TRUE(
					    linphone_participant_is_admin(p) ==
					    linphone_address_weak_equal(linphone_participant_get_address(p), marie.getCMgr()->identity));
				}
				bctbx_list_free_with_data(participants, (void (*)(void *))linphone_participant_unref);

				if (mgr != focus.getCMgr()) {
					BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
					                   .waitUntil(chrono::seconds(10), [&fconference1, &pconference] {
						                   return check_conference_ssrc(fconference1, pconference);
					                   }));
				}
			}
		}

		time_t start_time2 = (dialout) ? -1 : ms_time(NULL) - 40;
		time_t end_time2 = (dialout) ? -1 : (start_time2 + 60);
		const char *subject2 = "All Hands Q3 FY2021 - Attendance Mandatory";
		const char *description2 = "Financial result - Internal only - Strictly confidential";
		std::list<LinphoneCoreManager *> participants2{pauline.getCMgr()};
		std::list<LinphoneCoreManager *> mgr_having_two_confs{};
		std::list<LinphoneCoreManager *> mgr_in_conf2{focus.getCMgr(), michelle.getCMgr()};
		ClientConference &confCreator2 = (same_organizer) ? marie : michelle;
		if (same_organizer) {
			participants2.push_back(michelle.getCMgr());
			mgr_having_two_confs.push_back(marie.getCMgr());
		} else {
			participants2.push_back(marie.getCMgr());
			if (!dialout) {
				mgr_having_two_confs.push_back(marie.getCMgr());
				mgr_in_conf2.push_back(marie.getCMgr());
			}
		}
		bctbx_list_t *participants_info2 = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList2;
		for (auto &p : participants2) {
			role = (role == LinphoneParticipantRoleSpeaker) ? LinphoneParticipantRoleListener
			                                                : LinphoneParticipantRoleSpeaker;
			participantList2.insert(
			    std::make_pair(p, add_participant_info_to_list(&participants_info2, p->identity, role, -1)));
		}

		LinphoneAddress *confAddr2 =
		    create_conference_on_server(focus, confCreator2, participantList2, start_time2, end_time2, subject2,
		                                description2, TRUE, security_level, TRUE, FALSE, NULL);
		BC_ASSERT_PTR_NOT_NULL(confAddr2);
		char *conference2_address_str = (confAddr2) ? linphone_address_as_string(confAddr2) : ms_strdup("sip:");

		// Chat room creation to send ICS
		if (dialout) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 2,
			                             liblinphone_tester_sip_timeout));
		} else if (same_organizer) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 3,
			                             liblinphone_tester_sip_timeout));
		} else {
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneChatRoomStateCreated, 1,
			                             liblinphone_tester_sip_timeout));
		}

		if (confAddr2) {
			mgr_having_two_confs.push_back(pauline.getCMgr());
			mgr_in_conf2.push_back(pauline.getCMgr());
			if (dialout) {
				// Need to search the call by looking at the conference id parameter (conf-id) as both calls to a
				// conference have the same remote address the conference server.
				const char *confId2 = linphone_address_get_uri_param(confAddr2, "conf-id");
				for (auto mgr : participants2) {
					LinphoneCall *pcall = NULL;
					const bctbx_list_t *call_list = linphone_core_get_calls(mgr->lc);
					for (const bctbx_list_t *elem = call_list; elem != NULL; elem = elem->next) {
						LinphoneCall *call = (LinphoneCall *)elem->data;
						const LinphoneAddress *remote_contact_address = linphone_call_get_remote_contact_address(call);
						if (confId2 &&
						    (strcmp(confId2, linphone_address_get_uri_param(remote_contact_address, "conf-id")) == 0)) {
							pcall = call;
							break;
						}
					}
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
						BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
						ms_message("%s is entering conference %s", linphone_core_get_identity(mgr->lc),
						           conference2_address_str);
						linphone_call_accept(pcall);
						BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 2,
						                             liblinphone_tester_sip_timeout));
					}
				}
			} else {
				for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), michelle.getCMgr()}) {
					ms_message("%s is entering conference %s", linphone_core_get_identity(mgr->lc),
					           conference2_address_str);
					LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
					linphone_core_invite_address_with_params_2(mgr->lc, confAddr2, new_params, NULL, nullptr);
					linphone_call_params_unref(new_params);
				}

				for (auto mgr : {marie.getCMgr(), pauline.getCMgr()}) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 2,
					                             liblinphone_tester_sip_timeout));
				}

				for (auto mgr : {michelle.getCMgr()}) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1,
					                             liblinphone_tester_sip_timeout));
				}

				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
				                             focus_stat.number_of_LinphoneCallIncomingReceived + 6,
				                             liblinphone_tester_sip_timeout));
			}
		}

		for (auto mgr : mgr_having_two_confs) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 3,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallPaused, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 2,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 2,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 2,
			                             liblinphone_tester_sip_timeout));
			LinphoneCall *currentCall = linphone_core_get_current_call(mgr->lc);
			BC_ASSERT_PTR_NOT_NULL(currentCall);
			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr2, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (currentCall && pconference) {
				BC_ASSERT_PTR_EQUAL(linphone_call_get_conference(currentCall), pconference);
			}
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating, 2, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 4,
			                             liblinphone_tester_sip_timeout));
		}

		for (auto mgr : {michelle.getCMgr()}) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1,
			                             liblinphone_tester_sip_timeout));
			LinphoneCall *currentCall = linphone_core_get_current_call(mgr->lc);
			BC_ASSERT_PTR_NOT_NULL(currentCall);
			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr2, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (currentCall && pconference) {
				BC_ASSERT_PTR_EQUAL(linphone_call_get_conference(currentCall), pconference);
			}
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 2,
			                             liblinphone_tester_sip_timeout));
		}

		const int subscription = (dialout) ? ((same_organizer) ? 5 : 4) : 6;
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + subscription,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + 2 * subscription,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
		                             focus_stat.number_of_LinphoneConferenceStateCreated + 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + subscription,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + subscription,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + subscription,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + subscription,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
		                             focus_stat.number_of_conference_participant_devices_present + subscription,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
		                             focus_stat.number_of_participant_devices_present + subscription,
		                             liblinphone_tester_sip_timeout));

		// Marie and Pauline leave conference1
		const int onhold = (dialout) ? ((same_organizer) ? 1 : 0) : 2;
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_on_hold,
		                             focus_stat.number_of_conference_participant_devices_on_hold + onhold,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_on_hold,
		                             focus_stat.number_of_participant_devices_on_hold + onhold,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList,
		                             &laure.getStats().number_of_participant_devices_media_capability_changed, onhold,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_conference_participant_devices_on_hold,
		                             onhold, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_on_hold, onhold,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList,
		                             &marie.getStats().number_of_participant_devices_media_capability_changed,
		                             onhold - 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_conference_participant_devices_on_hold,
		                             onhold - 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_on_hold, onhold - 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList,
		                             &pauline.getStats().number_of_participant_devices_media_capability_changed, onhold,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_conference_participant_devices_on_hold,
		                             onhold, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_on_hold, onhold,
		                             liblinphone_tester_sip_timeout));

		LinphoneConference *fconference2 =
		    linphone_core_search_conference(focus.getLc(), NULL, confAddr2, confAddr2, NULL);
		BC_ASSERT_PTR_NOT_NULL(fconference2);

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			const LinphoneAddress *local_address = (mgr == focus.getCMgr()) ? confAddr1 : mgr->identity;
			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, local_address, confAddr1, NULL);
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
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
			}
		}

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle}).waitUntil(chrono::seconds(2), [] { return false; });

		auto &organizer2 = (same_organizer) ? marie : michelle;
		for (auto mgr : mgr_in_conf2) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr2);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				const LinphoneConferenceParams *conference_params = linphone_conference_get_current_params(pconference);
				int no_participants = 0;
				if (start_time2 >= 0) {
					BC_ASSERT_EQUAL((long long)linphone_conference_params_get_start_time(conference_params),
					                (long long)start_time2, long long, "%lld");
				}
				BC_ASSERT_EQUAL((long long)linphone_conference_params_get_end_time(conference_params),
				                (long long)end_time2, long long, "%lld");
				BC_ASSERT_EQUAL((int)linphone_conference_params_get_security_level(conference_params),
				                (int)security_level, int, "%0d");
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
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), subject2);
				check_conference_me(pconference, ((mgr == organizer2.getCMgr()) || (mgr == focus.getCMgr())));
				bctbx_list_t *participants = linphone_conference_get_participant_list(pconference);
				for (bctbx_list_t *itp = participants; itp; itp = bctbx_list_next(itp)) {
					LinphoneParticipant *p = (LinphoneParticipant *)bctbx_list_get_data(itp);
					BC_ASSERT_TRUE(linphone_participant_is_admin(p) ==
					               linphone_address_weak_equal(linphone_participant_get_address(p),
					                                           organizer2.getCMgr()->identity));
				}
				bctbx_list_free_with_data(participants, (void (*)(void *))linphone_participant_unref);

				if (mgr != focus.getCMgr()) {
					BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
					                   .waitUntil(chrono::seconds(10), [&fconference2, &pconference] {
						                   return check_conference_ssrc(fconference2, pconference);
					                   }));
				}
			}
		}

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

		focus_stat = focus.getStats();
		stats pauline_stat = pauline.getStats();

		std::list<LinphoneCoreManager *> mgr_conf2_to_remove{michelle.getCMgr(), marie.getCMgr()};
		// Marie and Michelle leave conference2
		for (auto mgr : mgr_conf2_to_remove) {
			LinphoneAddress *uri = mgr->identity;
			LinphoneConference *pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr2, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				ms_message("%s is terminating conference %s", linphone_core_get_identity(mgr->lc),
				           conference2_address_str);
				linphone_conference_terminate(pconference);
				BC_ASSERT_TRUE(
				    wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending,
				                             1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1,
				                             liblinphone_tester_sip_timeout));

				LinphoneConference *pconference1 = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr2, NULL);
				BC_ASSERT_PTR_NULL(pconference1);
			}
			auto it_having_two_confs = std::find(mgr_having_two_confs.begin(), mgr_having_two_confs.end(), mgr);
			if (it_having_two_confs != mgr_having_two_confs.end()) {
				mgr_having_two_confs.erase(it_having_two_confs);
			}

			auto it_conf2 = std::find(mgr_in_conf2.begin(), mgr_in_conf2.end(), mgr);
			if (it_conf2 != mgr_in_conf2.end()) {
				mgr_in_conf2.erase(it_conf2);
			}
		}

		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
		                  focus_stat.number_of_participants_removed + static_cast<int>(mgr_conf2_to_remove.size()),
		                  liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed +
		                                 static_cast<int>(mgr_conf2_to_remove.size()),
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &pauline.getStats().number_of_participants_removed,
		                  pauline_stat.number_of_participants_removed + static_cast<int>(mgr_conf2_to_remove.size()),
		                  liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_removed,
		                             pauline_stat.number_of_participant_devices_removed +
		                                 static_cast<int>(mgr_conf2_to_remove.size()),
		                             liblinphone_tester_sip_timeout));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                pauline_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneConferenceStateTerminated,
		                pauline_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneConferenceStateDeleted,
		                pauline_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                focus_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminated,
		                focus_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateDeleted,
		                focus_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		for (auto mgr : mgr_in_conf2) {
			const LinphoneAddress *local_address = (mgr == focus.getCMgr()) ? confAddr2 : mgr->identity;
			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, local_address, confAddr2, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference),
				                ((mgr == focus.getCMgr()) ? 1 : 0), int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), mgr_in_conf2.size() - 1, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), subject2);
			}
		}

		// Marie and Pauline rejoin conference1 (Pauline leaves conference2)
		focus_stat = focus.getStats();
		pauline_stat = pauline.getStats();
		stats marie_stat = marie.getStats();
		std::list<LinphoneCoreManager *> mgr_rejoining{marie.getCMgr()};
		mgr_rejoining.push_back(pauline.getCMgr());

		for (auto mgr : mgr_rejoining) {
			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr1, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				ms_message("%s is joining conference %s", linphone_core_get_identity(mgr->lc), conference1_address_str);
				linphone_conference_enter(pconference);
			}
		}

		LinphoneConference *conference1 =
		    linphone_core_search_conference(focus.getLc(), NULL, confAddr1, confAddr1, NULL);
		BC_ASSERT_PTR_NOT_NULL(conference1);

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallPaused,
		                             pauline_stat.number_of_LinphoneCallPaused + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning,
		                             pauline_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));

		if (!dialout || same_organizer) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
			                             marie_stat.number_of_LinphoneCallStreamsRunning + 1,
			                             liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                pauline_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneConferenceStateTerminated,
		                pauline_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneConferenceStateDeleted,
		                pauline_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                focus_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminated,
		                focus_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateDeleted,
		                focus_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		// Pauline leaves conference2
		// Pauline and Marie enter conference1
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_on_hold,
		                             focus_stat.number_of_conference_participant_devices_on_hold + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_on_hold,
		                             focus_stat.number_of_participant_devices_on_hold + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
		                             focus_stat.number_of_conference_participant_devices_present + 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
		                             focus_stat.number_of_participant_devices_present + 2,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : mgr_in_conf2) {
			const LinphoneAddress *local_address = (mgr == focus.getCMgr()) ? confAddr2 : mgr->identity;
			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, local_address, confAddr2, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference),
				                ((mgr == focus.getCMgr()) ? 1 : 0), int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), ((mgr == focus.getCMgr()) ? 1 : 0), size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), subject2);
			}
		}

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle}).waitUntil(chrono::seconds(2), [] { return false; });

		// Laure and Pauline are removed from conference1
		focus_stat = focus.getStats();
		marie_stat = marie.getStats();
		pauline_stat = pauline.getStats();
		stats laure_stat = laure.getStats();
		int cnt = 0;
		if (conference1) {
			for (auto mgr : {laure.getCMgr(), pauline.getCMgr()}) {
				cnt++;
				LinphoneParticipant *participant = linphone_conference_find_participant(conference1, mgr->identity);
				BC_ASSERT_PTR_NOT_NULL(participant);
				if (participant) {
					char *conference1_me = linphone_address_as_string(
					    linphone_participant_get_address(linphone_conference_get_me(conference1)));
					ms_message("%s is removing participant %s from conference %s", conference1_me,
					           linphone_core_get_identity(mgr->lc), conference1_address_str);
					ms_free(conference1_me);
					linphone_conference_remove_participant_2(conference1, participant);
				}
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
				                             focus_stat.number_of_LinphoneCallEnd + cnt,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
				                             focus_stat.number_of_LinphoneCallReleased + cnt,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
				                             focus_stat.number_of_participants_removed + cnt,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
				                             focus_stat.number_of_participant_devices_removed + cnt,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed,
				                             marie_stat.number_of_participants_removed + cnt,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_removed,
				                             marie_stat.number_of_participant_devices_removed + cnt,
				                             liblinphone_tester_sip_timeout));
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallEnd,
		                             laure_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallReleased,
		                             laure_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallEnd,
		                             pauline_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallReleased,
		                             pauline_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneSubscriptionTerminated,
		                             laure_stat.number_of_LinphoneSubscriptionTerminated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             laure_stat.number_of_LinphoneConferenceStateTerminationPending + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateTerminated,
		                             laure_stat.number_of_LinphoneConferenceStateTerminated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateDeleted,
		                             laure_stat.number_of_LinphoneConferenceStateDeleted + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneSubscriptionTerminated,
		                             pauline_stat.number_of_LinphoneSubscriptionTerminated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             pauline_stat.number_of_LinphoneConferenceStateTerminationPending + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateTerminated,
		                             pauline_stat.number_of_LinphoneConferenceStateTerminated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateDeleted,
		                             pauline_stat.number_of_LinphoneConferenceStateDeleted + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_EQUAL(marie.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                marie_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(marie.getStats().number_of_LinphoneConferenceStateTerminated,
		                marie_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(marie.getStats().number_of_LinphoneConferenceStateDeleted,
		                marie_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle}).waitUntil(chrono::seconds(2), [] { return false; });

		// Pauline rejoins and leaves conference2 (conference2 is destroyed on the server)
		focus_stat = focus.getStats();
		pauline_stat = pauline.getStats();
		for (auto mgr : {pauline.getCMgr()}) {
			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr2, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				ms_message("%s is entering conference %s", linphone_core_get_identity(mgr->lc),
				           conference2_address_str);
				linphone_conference_enter(pconference);
			}
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning,
		                             pauline_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                pauline_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneConferenceStateTerminated,
		                pauline_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneConferenceStateDeleted,
		                pauline_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		// Pauline enters conference2
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
		                             focus_stat.number_of_conference_participant_devices_present + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
		                             focus_stat.number_of_participant_devices_present + 1,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : mgr_in_conf2) {
			const LinphoneAddress *local_address = (mgr == focus.getCMgr()) ? confAddr2 : mgr->identity;
			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, local_address, confAddr2, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference),
				                ((mgr == focus.getCMgr()) ? 1 : 0), int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 1, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_TRUE(linphone_conference_is_in(pconference) == (mgr != focus.getCMgr()));
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), subject2);
			}
		}

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle}).waitUntil(chrono::seconds(2), [] { return false; });

		// Pauline leaves conference2
		focus_stat = focus.getStats();
		pauline_stat = pauline.getStats();
		for (auto mgr : {pauline.getCMgr()}) {
			LinphoneAddress *uri = mgr->identity;
			LinphoneConference *pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr2, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				ms_message("%s is terminating conference %s", linphone_core_get_identity(mgr->lc),
				           conference2_address_str);
				linphone_conference_terminate(pconference);
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallEnd,
		                             pauline_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallReleased,
		                             pauline_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneSubscriptionTerminated,
		                             pauline_stat.number_of_LinphoneSubscriptionTerminated + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             pauline_stat.number_of_LinphoneConferenceStateTerminationPending + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateTerminated,
		                             pauline_stat.number_of_LinphoneConferenceStateTerminated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateDeleted,
		                             pauline_stat.number_of_LinphoneConferenceStateDeleted + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
		                             liblinphone_tester_sip_timeout));

		LinphoneConference *conference2 =
		    linphone_core_search_conference(focus.getLc(), NULL, confAddr2, confAddr2, NULL);
		BC_ASSERT_PTR_NULL(conference2);

		// Marie terminates conference1 (conference1 is destroyed on the server)
		focus_stat = focus.getStats();
		marie_stat = marie.getStats();
		for (auto mgr : {marie.getCMgr()}) {
			LinphoneAddress *uri = mgr->identity;
			LinphoneConference *pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr1, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				ms_message("%s is terminating conference %s", linphone_core_get_identity(mgr->lc),
				           conference1_address_str);
				linphone_conference_terminate(pconference);
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallEnd,
		                             marie_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallReleased,
		                             marie_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionTerminated,
		                             marie_stat.number_of_LinphoneSubscriptionTerminated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             marie_stat.number_of_LinphoneConferenceStateTerminationPending + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminated,
		                             marie_stat.number_of_LinphoneConferenceStateTerminated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateDeleted,
		                             marie_stat.number_of_LinphoneConferenceStateDeleted + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
		                             focus_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             focus_stat.number_of_LinphoneConferenceStateTerminationPending + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated,
		                             focus_stat.number_of_LinphoneConferenceStateTerminated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted,
		                             focus_stat.number_of_LinphoneConferenceStateDeleted + 1,
		                             liblinphone_tester_sip_timeout));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle}).waitUntil(chrono::seconds(2), [] { return false; });

		bctbx_list_free_with_data(participants_info1, (bctbx_list_free_func)linphone_participant_info_unref);
		bctbx_list_free_with_data(participants_info2, (bctbx_list_free_func)linphone_participant_info_unref);
		ms_free(conference1_address_str);
		ms_free(conference2_address_str);
		linphone_address_unref(confAddr1);
		linphone_address_unref(confAddr2);
		bctbx_list_free(coresList);
	}
}

void create_one_participant_conference_toggle_video_base(LinphoneConferenceLayout layout,
                                                         bool_t enable_ice,
                                                         bool_t enable_stun) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress());
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress());

		focus.registerAsParticipantDevice(marie);

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t *coresList = NULL;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
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
				linphone_core_set_default_conference_layout(mgr->lc, layout);
			}

			enable_stun_in_mgr(mgr, enable_stun, enable_ice, enable_stun, enable_ice);

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);

		stats focus_stat = focus.getStats();
		stats marie_stat = marie.getStats();

		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr()};

		time_t start_time = ms_time(NULL);
		int duration = -1;
		time_t end_time = (duration <= 0) ? -1 : (start_time + duration * 60);
		const char *initialSubject = "Test characters: ^ :) ¤ çà @";
		const char *description = "- <F2><F3>\n\\çà";
		LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		LinphoneParticipantRole role = LinphoneParticipantRoleSpeaker;
		for (auto &p : participants) {
			participantList.insert(
			    std::make_pair(p, add_participant_info_to_list(&participants_info, p->identity, role, -1)));
			role = (role == LinphoneParticipantRoleSpeaker) ? LinphoneParticipantRoleListener
			                                                : LinphoneParticipantRoleSpeaker;
		}
		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, security_level, TRUE, FALSE, NULL);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 2,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : {marie.getCMgr()}) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_call_params_set_video_direction(new_params, LinphoneMediaDirectionSendRecv);
			linphone_call_params_enable_video(new_params, TRUE);
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);
		}

		for (auto mgr : {marie.getCMgr()}) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1,
			                             liblinphone_tester_sip_timeout));
			int no_streams_running = ((enable_ice) ? 3 : 2);
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating, (no_streams_running - 1),
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, no_streams_running,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneChatRoomStateCreated,
			                             ((mgr == marie.getCMgr()) ? 2 : 1), liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + 1,
		                             liblinphone_tester_sip_timeout));
		int focus_no_streams_running = ((enable_ice) ? 3 : 2);
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running - 1),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
		                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + 1,
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

		LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

		size_t no_streams_audio = 1;
		size_t no_streams_video = 2;
		size_t no_active_streams_video = (layout == LinphoneConferenceLayoutGrid) ? 0 : 1;
		size_t no_streams_text = 0;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr()}) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				const LinphoneConferenceParams *conference_params = linphone_conference_get_current_params(pconference);
				int no_participants = 0;
				if (start_time >= 0) {
					BC_ASSERT_EQUAL((long long)linphone_conference_params_get_start_time(conference_params),
					                (long long)start_time, long long, "%lld");
				}
				BC_ASSERT_EQUAL((long long)linphone_conference_params_get_end_time(conference_params),
				                (long long)end_time, long long, "%lld");
				BC_ASSERT_EQUAL((int)linphone_conference_params_get_security_level(conference_params),
				                (int)security_level, int, "%0d");
				if (mgr == focus.getCMgr()) {
					no_participants = 1;
					BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
				} else {
					no_participants = 0;
					BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
					LinphoneCall *current_call = linphone_core_get_current_call(mgr->lc);
					BC_ASSERT_PTR_NOT_NULL(current_call);
					if (current_call) {
						BC_ASSERT_EQUAL((int)linphone_call_get_state(current_call),
						                (int)LinphoneCallStateStreamsRunning, int, "%0d");
					}
					if (enable_ice) {
						BC_ASSERT_TRUE(check_ice(mgr, focus.getCMgr(), LinphoneIceStateHostConnection));
					}

					LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						_linphone_call_check_max_nb_streams(pcall, no_streams_audio, no_streams_video, no_streams_text);
						_linphone_call_check_nb_active_streams(pcall, no_streams_audio, no_streams_video,
						                                       no_streams_text);
						const LinphoneCallParams *call_lparams = linphone_call_get_params(pcall);
						BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
						const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(pcall);
						BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
						const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
						BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams));
					}
					LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
					BC_ASSERT_PTR_NOT_NULL(ccall);
					if (ccall) {
						_linphone_call_check_max_nb_streams(ccall, no_streams_audio, no_streams_video, no_streams_text);
						_linphone_call_check_nb_active_streams(ccall, no_streams_audio, no_streams_video,
						                                       no_streams_text);
						const LinphoneCallParams *call_lparams = linphone_call_get_params(ccall);
						BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
						const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(ccall);
						BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
						const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
						BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams));
					}
				}
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 1, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
				check_conference_me(pconference, ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())));
				bctbx_list_t *participants = linphone_conference_get_participant_list(pconference);
				for (bctbx_list_t *itp = participants; itp; itp = bctbx_list_next(itp)) {
					LinphoneParticipant *p = (LinphoneParticipant *)bctbx_list_get_data(itp);
					BC_ASSERT_TRUE(
					    linphone_participant_is_admin(p) ==
					    linphone_address_weak_equal(linphone_participant_get_address(p), marie.getCMgr()->identity));
				}
				bctbx_list_free_with_data(participants, (void (*)(void *))linphone_participant_unref);

				if (mgr != focus.getCMgr()) {
					BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, laure})
					                   .waitUntil(chrono::seconds(10), [&fconference, &pconference] {
						                   return check_conference_ssrc(fconference, pconference);
					                   }));
				}
			}
		}

		focus_stat = focus.getStats();
		marie_stat = marie.getStats();

		LinphoneCall *marie_calls_focus = linphone_core_get_call_by_remote_address2(marie.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(marie_calls_focus);
		LinphoneCall *focus_called_by_marie =
		    linphone_core_get_call_by_remote_address2(focus.getLc(), marie.getCMgr()->identity);
		BC_ASSERT_PTR_NOT_NULL(focus_called_by_marie);

		LinphoneParticipant *marie_participant =
		    linphone_conference_find_participant(fconference, marie.getCMgr()->identity);
		BC_ASSERT_PTR_NOT_NULL(marie_participant);
		if (marie_participant) {
			bctbx_list_t *devices = linphone_participant_get_devices(marie_participant);
			for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
				LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
				LinphoneMediaDirection video_direction = (layout == LinphoneConferenceLayoutGrid)
				                                             ? LinphoneMediaDirectionSendOnly
				                                             : LinphoneMediaDirectionSendRecv;
				BC_ASSERT_TRUE(linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) ==
				               video_direction);
				BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
			}
			if (devices) {
				bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
			}
		}

		if (marie_calls_focus) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(marie.getLc(), marie_calls_focus);
			linphone_call_params_enable_video(new_params, TRUE);
			linphone_call_params_set_video_direction(new_params, LinphoneMediaDirectionRecvOnly);
			linphone_call_update(marie_calls_focus, new_params);
			linphone_call_params_unref(new_params);
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating,
		                             marie_stat.number_of_LinphoneCallUpdating + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
		                             marie_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(
		    coresList, &marie.getStats().number_of_participant_devices_media_capability_changed,
		    marie_stat.number_of_participant_devices_media_capability_changed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(
		    coresList, &focus.getStats().number_of_participant_devices_media_capability_changed,
		    focus_stat.number_of_participant_devices_media_capability_changed + 1, liblinphone_tester_sip_timeout));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

		if (marie_calls_focus) {
			_linphone_call_check_max_nb_streams(marie_calls_focus, no_streams_audio, no_streams_video, no_streams_text);
			_linphone_call_check_nb_active_streams(marie_calls_focus, no_streams_audio, no_active_streams_video,
			                                       no_streams_text);
			const LinphoneCallParams *call_lparams = linphone_call_get_params(marie_calls_focus);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
			const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(marie_calls_focus);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams) !=
			               (layout == LinphoneConferenceLayoutGrid));
			const LinphoneCallParams *call_cparams = linphone_call_get_current_params(marie_calls_focus);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams) !=
			               (layout == LinphoneConferenceLayoutGrid));
		}
		if (focus_called_by_marie) {
			_linphone_call_check_max_nb_streams(focus_called_by_marie, no_streams_audio, no_streams_video,
			                                    no_streams_text);
			_linphone_call_check_nb_active_streams(focus_called_by_marie, no_streams_audio, no_active_streams_video,
			                                       no_streams_text);
			const LinphoneCallParams *call_lparams = linphone_call_get_params(focus_called_by_marie);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams) !=
			               (layout == LinphoneConferenceLayoutGrid));
			const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(focus_called_by_marie);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams) !=
			               (layout == LinphoneConferenceLayoutGrid));
			const LinphoneCallParams *call_cparams = linphone_call_get_current_params(focus_called_by_marie);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams) !=
			               (layout == LinphoneConferenceLayoutGrid));
		}

		if (marie_participant) {
			bctbx_list_t *devices = linphone_participant_get_devices(marie_participant);
			for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
				LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
				LinphoneMediaDirection video_direction = (layout == LinphoneConferenceLayoutGrid)
				                                             ? LinphoneMediaDirectionInactive
				                                             : LinphoneMediaDirectionRecvOnly;
				BC_ASSERT_TRUE(linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) ==
				               video_direction);
				BC_ASSERT_FALSE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
			}
			if (devices) {
				bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
			}
		}

		focus_stat = focus.getStats();
		marie_stat = marie.getStats();

		if (marie_calls_focus) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(marie.getLc(), marie_calls_focus);
			linphone_call_params_enable_video(new_params, TRUE);
			linphone_call_params_set_video_direction(new_params, LinphoneMediaDirectionSendRecv);
			linphone_call_update(marie_calls_focus, new_params);
			linphone_call_params_unref(new_params);
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating,
		                             marie_stat.number_of_LinphoneCallUpdating + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
		                             marie_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(
		    coresList, &marie.getStats().number_of_participant_devices_media_capability_changed,
		    marie_stat.number_of_participant_devices_media_capability_changed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(
		    coresList, &focus.getStats().number_of_participant_devices_media_capability_changed,
		    focus_stat.number_of_participant_devices_media_capability_changed + 1, liblinphone_tester_sip_timeout));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

		if (marie_calls_focus) {
			_linphone_call_check_max_nb_streams(marie_calls_focus, no_streams_audio, no_streams_video, no_streams_text);
			_linphone_call_check_nb_active_streams(marie_calls_focus, no_streams_audio, no_streams_video,
			                                       no_streams_text);
			const LinphoneCallParams *call_lparams = linphone_call_get_params(marie_calls_focus);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
			const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(marie_calls_focus);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
			const LinphoneCallParams *call_cparams = linphone_call_get_current_params(marie_calls_focus);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams));
		}
		if (focus_called_by_marie) {
			_linphone_call_check_max_nb_streams(focus_called_by_marie, no_streams_audio, no_streams_video,
			                                    no_streams_text);
			_linphone_call_check_nb_active_streams(focus_called_by_marie, no_streams_audio, no_streams_video,
			                                       no_streams_text);
			const LinphoneCallParams *call_lparams = linphone_call_get_params(focus_called_by_marie);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
			const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(focus_called_by_marie);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
			const LinphoneCallParams *call_cparams = linphone_call_get_current_params(focus_called_by_marie);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams));
		}

		if (marie_participant) {
			bctbx_list_t *devices = linphone_participant_get_devices(marie_participant);
			for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
				LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
				LinphoneMediaDirection video_direction = (layout == LinphoneConferenceLayoutGrid)
				                                             ? LinphoneMediaDirectionSendOnly
				                                             : LinphoneMediaDirectionSendRecv;
				BC_ASSERT_TRUE(linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) ==
				               video_direction);
				BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
			}
			if (devices) {
				bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
			}
		}

		focus_stat = focus.getStats();
		marie_stat = marie.getStats();

		if (marie_calls_focus) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(marie.getLc(), marie_calls_focus);
			linphone_call_params_enable_video(new_params, FALSE);
			linphone_call_params_set_video_direction(new_params, LinphoneMediaDirectionRecvOnly);
			linphone_call_update(marie_calls_focus, new_params);
			linphone_call_params_unref(new_params);
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating,
		                             marie_stat.number_of_LinphoneCallUpdating + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
		                             marie_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(
		    coresList, &marie.getStats().number_of_participant_devices_media_capability_changed,
		    marie_stat.number_of_participant_devices_media_capability_changed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(
		    coresList, &focus.getStats().number_of_participant_devices_media_capability_changed,
		    focus_stat.number_of_participant_devices_media_capability_changed + 1, liblinphone_tester_sip_timeout));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

		if (marie_calls_focus) {
			_linphone_call_check_max_nb_streams(marie_calls_focus, no_streams_audio, no_streams_video, no_streams_text);
			_linphone_call_check_nb_active_streams(marie_calls_focus, no_streams_audio, 0, no_streams_text);
			const LinphoneCallParams *call_lparams = linphone_call_get_params(marie_calls_focus);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_lparams));
			const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(marie_calls_focus);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_rparams));
			const LinphoneCallParams *call_cparams = linphone_call_get_current_params(marie_calls_focus);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_cparams));
		}
		if (focus_called_by_marie) {
			_linphone_call_check_max_nb_streams(focus_called_by_marie, no_streams_audio, no_streams_video,
			                                    no_streams_text);
			_linphone_call_check_nb_active_streams(focus_called_by_marie, no_streams_audio, 0, no_streams_text);
			const LinphoneCallParams *call_lparams = linphone_call_get_params(focus_called_by_marie);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_lparams));
			const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(focus_called_by_marie);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_rparams));
			const LinphoneCallParams *call_cparams = linphone_call_get_current_params(focus_called_by_marie);
			BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_cparams));
		}

		if (marie_participant) {
			bctbx_list_t *devices = linphone_participant_get_devices(marie_participant);
			for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
				LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
				LinphoneMediaDirection video_direction = LinphoneMediaDirectionInactive;
				BC_ASSERT_TRUE(linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) ==
				               video_direction);
				BC_ASSERT_FALSE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
			}
			if (devices) {
				bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
			}
		}

		focus_stat = focus.getStats();
		marie_stat = marie.getStats();

		if (marie_calls_focus) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(marie.getLc(), marie_calls_focus);
			linphone_call_params_enable_video(new_params, TRUE);
			linphone_call_update(marie_calls_focus, new_params);
			linphone_call_params_unref(new_params);
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating,
		                             marie_stat.number_of_LinphoneCallUpdating + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
		                             marie_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		if (layout == LinphoneConferenceLayoutGrid) {
			BC_ASSERT_FALSE(wait_for_list(coresList,
			                              &marie.getStats().number_of_participant_devices_media_capability_changed,
			                              marie_stat.number_of_participant_devices_media_capability_changed + 1, 2000));
			BC_ASSERT_FALSE(wait_for_list(coresList,
			                              &focus.getStats().number_of_participant_devices_media_capability_changed,
			                              focus_stat.number_of_participant_devices_media_capability_changed + 1, 2000));
		} else {
			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &marie.getStats().number_of_participant_devices_media_capability_changed,
			    marie_stat.number_of_participant_devices_media_capability_changed + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &focus.getStats().number_of_participant_devices_media_capability_changed,
			    focus_stat.number_of_participant_devices_media_capability_changed + 1, liblinphone_tester_sip_timeout));
		}

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

		if (marie_calls_focus) {
			_linphone_call_check_max_nb_streams(marie_calls_focus, no_streams_audio, no_streams_video, no_streams_text);
			_linphone_call_check_nb_active_streams(marie_calls_focus, no_streams_audio, no_active_streams_video,
			                                       no_streams_text);
			const LinphoneCallParams *call_lparams = linphone_call_get_params(marie_calls_focus);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
			const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(marie_calls_focus);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams) !=
			               (layout == LinphoneConferenceLayoutGrid));
			const LinphoneCallParams *call_cparams = linphone_call_get_current_params(marie_calls_focus);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams) !=
			               (layout == LinphoneConferenceLayoutGrid));
		}
		if (focus_called_by_marie) {
			_linphone_call_check_max_nb_streams(focus_called_by_marie, no_streams_audio, no_streams_video,
			                                    no_streams_text);
			_linphone_call_check_nb_active_streams(focus_called_by_marie, no_streams_audio, no_active_streams_video,
			                                       no_streams_text);
			const LinphoneCallParams *call_lparams = linphone_call_get_params(focus_called_by_marie);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams) !=
			               (layout == LinphoneConferenceLayoutGrid));
			const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(focus_called_by_marie);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams) !=
			               (layout == LinphoneConferenceLayoutGrid));
			const LinphoneCallParams *call_cparams = linphone_call_get_current_params(focus_called_by_marie);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams) !=
			               (layout == LinphoneConferenceLayoutGrid));
		}

		if (marie_participant) {
			bctbx_list_t *devices = linphone_participant_get_devices(marie_participant);
			for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
				LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
				LinphoneMediaDirection video_direction = (layout == LinphoneConferenceLayoutGrid)
				                                             ? LinphoneMediaDirectionInactive
				                                             : LinphoneMediaDirectionRecvOnly;
				BC_ASSERT_TRUE(linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) ==
				               video_direction);
				BC_ASSERT_FALSE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
			}
			if (devices) {
				bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
			}
		}

		focus_stat = focus.getStats();
		marie_stat = marie.getStats();

		if (marie_calls_focus) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(marie.getLc(), marie_calls_focus);
			linphone_call_params_set_video_direction(new_params, LinphoneMediaDirectionSendRecv);
			linphone_call_update(marie_calls_focus, new_params);
			linphone_call_params_unref(new_params);
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating,
		                             marie_stat.number_of_LinphoneCallUpdating + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
		                             marie_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(
		    coresList, &marie.getStats().number_of_participant_devices_media_capability_changed,
		    marie_stat.number_of_participant_devices_media_capability_changed + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(
		    coresList, &focus.getStats().number_of_participant_devices_media_capability_changed,
		    focus_stat.number_of_participant_devices_media_capability_changed + 1, liblinphone_tester_sip_timeout));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

		if (marie_calls_focus) {
			_linphone_call_check_max_nb_streams(marie_calls_focus, no_streams_audio, no_streams_video, no_streams_text);
			_linphone_call_check_nb_active_streams(marie_calls_focus, no_streams_audio, no_streams_video,
			                                       no_streams_text);
			const LinphoneCallParams *call_lparams = linphone_call_get_params(marie_calls_focus);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
			const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(marie_calls_focus);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
			const LinphoneCallParams *call_cparams = linphone_call_get_current_params(marie_calls_focus);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams));
		}
		if (focus_called_by_marie) {
			_linphone_call_check_max_nb_streams(focus_called_by_marie, no_streams_audio, no_streams_video,
			                                    no_streams_text);
			_linphone_call_check_nb_active_streams(focus_called_by_marie, no_streams_audio, no_streams_video,
			                                       no_streams_text);
			const LinphoneCallParams *call_lparams = linphone_call_get_params(focus_called_by_marie);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
			const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(focus_called_by_marie);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
			const LinphoneCallParams *call_cparams = linphone_call_get_current_params(focus_called_by_marie);
			BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams));
		}

		if (marie_participant) {
			bctbx_list_t *devices = linphone_participant_get_devices(marie_participant);
			for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
				LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
				LinphoneMediaDirection video_direction = (layout == LinphoneConferenceLayoutGrid)
				                                             ? LinphoneMediaDirectionSendOnly
				                                             : LinphoneMediaDirectionSendRecv;
				BC_ASSERT_TRUE(linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) ==
				               video_direction);
				BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
			}
			if (devices) {
				bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
			}
		}

		LinphoneCall *call = linphone_core_get_call_by_remote_address2(marie.getLc(), focus.getCMgr()->identity);
		BC_ASSERT_PTR_NOT_NULL(call);
		if (call) {
			linphone_call_terminate(call);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallEnd, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallReleased, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &marie.getStats().number_of_LinphoneConferenceStateTerminationPending, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateDeleted, 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &focus.getStats().number_of_LinphoneConferenceStateTerminationPending, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
			                             liblinphone_tester_sip_timeout));
		}

		for (auto mgr : {focus.getCMgr(), marie.getCMgr()}) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated, 1,
			                             liblinphone_tester_sip_timeout));

			if (mgr && (mgr != focus.getCMgr())) {
				LinphoneCall *participant_call =
				    linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
				BC_ASSERT_PTR_NULL(participant_call);
				LinphoneCall *conference_call = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
				BC_ASSERT_PTR_NULL(conference_call);

				const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
				BC_ASSERT_EQUAL(bctbx_list_size(call_logs), 1, size_t, "%zu");

				bctbx_list_t *mgr_focus_call_log =
				    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
				BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
				if (mgr_focus_call_log) {
					BC_ASSERT_EQUAL(bctbx_list_size(mgr_focus_call_log), 1, size_t, "%zu");
					for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
						LinphoneCallLog *call_log = (LinphoneCallLog *)it->data;
						BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
					}
					bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
				}
			}
		}

		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

void create_conference_with_active_call_base(bool_t dialout) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress());
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress());
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress());
		ClientConference berthe("berthe_rc", focus.getConferenceFactoryAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(berthe);

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t *coresList = NULL;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(),
		                 berthe.getCMgr()}) {
			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);

		stats focus_stat = focus.getStats();
		stats marie_stat = marie.getStats();
		stats berthe_stat = berthe.getStats();

		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(),
		                                                laure.getCMgr(), michelle.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(),
		                                         michelle.getCMgr()};
		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr()};

		const char *initialSubject = "Schedule of the trip towards the top of Europe";
		const char *description = "To the top of the Mont Blanc!!!! :-)";
		LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;

		time_t start_time = (dialout) ? -1 : (ms_time(NULL) + 5);
		const int duration = 0;
		time_t end_time = -1;
		bool_t send_ics = !dialout;

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		LinphoneParticipantRole role = LinphoneParticipantRoleSpeaker;
		for (auto &p : participants) {
			participantList.insert(
			    std::make_pair(p, add_participant_info_to_list(&participants_info, p->identity, role, -1)));
			if (role == LinphoneParticipantRoleSpeaker) {
				role = LinphoneParticipantRoleListener;
			} else if (role == LinphoneParticipantRoleListener) {
				role = LinphoneParticipantRoleUnknown;
			} else if (role == LinphoneParticipantRoleUnknown) {
				role = LinphoneParticipantRoleSpeaker;
			}
		}
		participantList.insert(
		    std::make_pair(marie.getCMgr(), add_participant_info_to_list(&participants_info, marie.getCMgr()->identity,
		                                                                 LinphoneParticipantRoleListener, -1)));

		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, send_ics, security_level, FALSE, FALSE, NULL);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(5), [] {
			return false;
		});

		// Berthe calls the conference server  - who knows why......
		LinphoneCallParams *new_params = linphone_core_create_call_params(berthe.getLc(), NULL);

		LinphoneContent *content = linphone_core_create_content(berthe.getLc());
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
		linphone_call_params_add_custom_content(new_params, content);

		// Berthe sends an INVITE with a resource list but she hasn't the right to do so because she was never
		// invited to the conference. The conference server declines it with a 403 Forbidden
		LinphoneCall *berthe_focus_call =
		    linphone_core_invite_address_with_params_2(berthe.getLc(), confAddr, new_params, NULL, nullptr);
		BC_ASSERT_PTR_NOT_NULL(berthe_focus_call);
		linphone_content_unref(content);

		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallOutgoingInit,
		                             berthe_stat.number_of_LinphoneCallOutgoingInit + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallError,
		                             berthe_stat.number_of_LinphoneCallError + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallReleased,
		                             berthe_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
		                             focus_stat.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_PTR_NULL(linphone_core_get_current_call(focus.getLc()));

		if (dialout) {
			// Chat room creation to send ICS
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated,
			                             (send_ics ? 1 : 0), liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallOutgoingInit,
			                             marie_stat.number_of_LinphoneCallOutgoingInit + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingInit,
			                             focus_stat.number_of_LinphoneCallOutgoingInit + 3,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
			                             focus_stat.number_of_LinphoneCallIncomingReceived + 2,
			                             liblinphone_tester_sip_timeout));

			for (auto mgr : participants) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallIncomingReceived, 1,
				                             liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
			                             marie_stat.number_of_LinphoneCallStreamsRunning + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
			                             marie_stat.number_of_LinphoneCallStreamsRunning + 2,
			                             liblinphone_tester_sip_timeout));

			LinphoneConference *oconference = linphone_core_search_conference_2(marie.getLc(), confAddr);
			if (BC_ASSERT_PTR_NOT_NULL(oconference)) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(oconference), 3, int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(oconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 4, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
			}

			if (confAddr) {
				for (auto mgr : participants) {
					LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
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
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1,
				                             liblinphone_tester_sip_timeout));
				int no_streams_running = 2;
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating,
				                             (no_streams_running - 1), liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning,
				                             no_streams_running, liblinphone_tester_sip_timeout));
			}
		}
		linphone_call_params_unref(new_params);

		if (!!send_ics) {
			update_sequence_number(&participants_info, {}, 0, -1);
		}
		for (bctbx_list_t *info_it = participants_info; info_it; info_it = bctbx_list_next(info_it)) {
			LinphoneParticipantInfo *participant_info_el = (LinphoneParticipantInfo *)bctbx_list_get_data(info_it);
			if (linphone_participant_info_get_role(participant_info_el) == LinphoneParticipantRoleUnknown) {
				linphone_participant_info_set_role(participant_info_el, LinphoneParticipantRoleSpeaker);
			}
		}

		for (auto mgr : participants) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 2,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(
			    CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
			        .waitUntil(chrono::seconds(15), [&mgr, confAddr, &participants, &members] {
				        LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
				        if (!conference) {
					        return false;
				        }
				        if (linphone_conference_get_participant_count(conference) !=
				            static_cast<int>(participants.size())) {
					        return false;
				        }
				        bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
				        size_t nb_devices = bctbx_list_size(devices);
				        if (devices) {
					        bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				        }
				        if (nb_devices != members.size()) {
					        return false;
				        }
				        return true;
			        }));
			check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info, start_time,
			                            duration, initialSubject, (send_ics) ? description : NULL, 0,
			                            LinphoneConferenceInfoStateNew, security_level, FALSE, TRUE, FALSE, FALSE);

			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
				BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				LinphoneConferenceInfo *call_log_info = linphone_call_log_get_conference_info(call_log);
				if (BC_ASSERT_PTR_NOT_NULL(call_log_info)) {
					check_conference_info_against_db(mgr, confAddr, call_log_info, FALSE);
				}
			}
		}

		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                  focus_stat.number_of_LinphoneCallStreamsRunning + static_cast<int>(participants.size() + 1),
		                  liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated,
		                             marie_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionOutgoingProgress,
		                             marie_stat.number_of_LinphoneSubscriptionOutgoingProgress + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
		                             marie_stat.number_of_LinphoneSubscriptionActive + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyFullStateReceived,
		                             marie_stat.number_of_NotifyReceived + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
		                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 4, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + 4, 5000));

		int no_participants = 4;
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + no_participants,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + no_participants,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
		                             focus_stat.number_of_conference_participant_devices_present + no_participants,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
		                             focus_stat.number_of_participant_devices_present + no_participants,
		                             liblinphone_tester_sip_timeout));

		LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		for (auto mgr : conferenceMgrs) {
			// wait a bit longer to detect side effect if any
			BC_ASSERT_TRUE(
			    CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
			        .waitUntil(chrono::seconds(50), [mgr, &focus, &members, confAddr, &participantList] {
				        size_t nb_audio_streams = 1;
				        size_t nb_video_streams = 0;
				        size_t nb_text_streams = 0;
				        std::list<LinphoneCall *> calls;

				        if (mgr == focus.getCMgr()) {
					        for (auto m : members) {
						        LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, m->identity);
						        if (call) {
							        calls.push_back(call);
						        } else {
							        return false;
						        }
					        }
				        } else {
					        LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					        if (call) {
						        calls.push_back(call);
					        } else {
						        return false;
					        }
				        }

				        for (auto call : calls) {
					        if (call) {
						        const SalMediaDescription *call_result_desc = _linphone_call_get_result_desc(call);
						        if (!((call_result_desc->getNbStreams() ==
						               nb_audio_streams + nb_video_streams + nb_text_streams) &&
						              (call_result_desc->nbStreamsOfType(SalAudio) == nb_audio_streams) &&
						              (call_result_desc->nbStreamsOfType(SalVideo) == nb_video_streams) &&
						              (call_result_desc->nbStreamsOfType(SalText) == nb_text_streams) &&
						              (linphone_call_get_state(call) == LinphoneCallStateStreamsRunning))) {
							        return false;
						        }
					        }
				        }

				        bool video_check = true;
				        LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
				        if (conference) {
					        bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
					        for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
						        LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
						        video_check &=
						            !linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo);
					        }

					        if (devices) {
						        bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
					        }
				        } else {
					        return false;
				        }
				        if (!video_check) {
					        return false;
				        }

				        bool role_check = true;
				        for (const auto &[mMgr, info] : participantList) {
					        LinphoneParticipant *p =
					            linphone_conference_is_me(conference, mMgr->identity)
					                ? linphone_conference_get_me(conference)
					                : linphone_conference_find_participant(conference, mMgr->identity);
					        role_check &= (p != nullptr);
					        if (p) {
						        role_check &=
						            (linphone_participant_get_role(p) == linphone_participant_info_get_role(info));
					        }
				        }
				        return role_check;
			        }));

			if (mgr != focus.getCMgr()) {
				check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info,
				                            start_time, duration, initialSubject, (send_ics) ? description : NULL, 0,
				                            LinphoneConferenceInfoStateNew, security_level, FALSE, TRUE, FALSE, FALSE);

				LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(pcall);
				if (pcall) {
					LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
					LinphoneConferenceInfo *call_log_info = linphone_call_log_get_conference_info(call_log);
					if (BC_ASSERT_PTR_NOT_NULL(call_log_info)) {
						check_conference_info_against_db(mgr, confAddr, call_log_info, FALSE);
					}
				}
			}
		}

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(5), [] {
			return false;
		});

		for (auto mgr : conferenceMgrs) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				int no_participants = 0;
				if (mgr == focus.getCMgr()) {
					no_participants = 4;
					BC_ASSERT_FALSE(linphone_conference_is_in(pconference));

					int focus_no_streams_running = 8;
					int focus_no_updating = focus_no_streams_running - 4;
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
					                             focus_stat.number_of_LinphoneCallUpdatedByRemote + focus_no_updating,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(
					    wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
					                  focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running,
					                  liblinphone_tester_sip_timeout));
				} else {
					no_participants = 3;
					BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
					LinphoneCall *current_call = linphone_core_get_current_call(mgr->lc);
					BC_ASSERT_PTR_NOT_NULL(current_call);
					if (current_call) {
						BC_ASSERT_EQUAL((int)linphone_call_get_state(current_call),
						                (int)LinphoneCallStateStreamsRunning, int, "%0d");
					}

					bool_t enabled = FALSE;
					bool_t video_strem_enabled = dialout && (mgr->lc == marie.getLc());
					size_t no_streams_audio = 1;
					size_t no_streams_video = (video_strem_enabled) ? 1 : 0;
					size_t no_active_streams_video = 0;
					size_t no_streams_text = 0;

					LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						_linphone_call_check_max_nb_streams(pcall, no_streams_audio, no_streams_video, no_streams_text);
						_linphone_call_check_nb_active_streams(pcall, no_streams_audio, no_active_streams_video,
						                                       no_streams_text);
						const LinphoneCallParams *call_lparams = linphone_call_get_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
						const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
						const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
					}

					LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
					BC_ASSERT_PTR_NOT_NULL(ccall);
					if (ccall) {
						_linphone_call_check_max_nb_streams(ccall, no_streams_audio, no_streams_video, no_streams_text);
						_linphone_call_check_nb_active_streams(ccall, no_streams_audio, no_active_streams_video,
						                                       no_streams_text);
						const LinphoneCallParams *call_lparams = linphone_call_get_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
						const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
						const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
					}
				}
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 4, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
				check_conference_me(pconference, ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())));
				bctbx_list_t *participants = linphone_conference_get_participant_list(pconference);
				for (bctbx_list_t *itp = participants; itp; itp = bctbx_list_next(itp)) {
					LinphoneParticipant *p = (LinphoneParticipant *)bctbx_list_get_data(itp);
					BC_ASSERT_TRUE(
					    linphone_participant_is_admin(p) ==
					    linphone_address_weak_equal(linphone_participant_get_address(p), marie.getCMgr()->identity));
				}
				bctbx_list_free_with_data(participants, (void (*)(void *))linphone_participant_unref);

				if (mgr != focus.getCMgr()) {
					BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
					                   .waitUntil(chrono::seconds(10), [&fconference, &pconference] {
						                   return check_conference_ssrc(fconference, pconference);
					                   }));
				}

				LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(conference);
				if (conference) {
					bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
					for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
						LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
						BC_ASSERT_FALSE(
						    linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
					}

					if (devices) {
						bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
					}
				}
			}
		}

		focus_stat = focus.getStats();
		for (auto mgr : members) {
			LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
			BC_ASSERT_PTR_NOT_NULL(call);
			if (call) {
				ms_message("%s is terminating call with %s", linphone_core_get_identity(mgr->lc),
				           linphone_core_get_identity(focus.getLc()));
				linphone_call_terminate(call);
				BC_ASSERT_TRUE(
				    wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending,
				                             1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1,
				                             liblinphone_tester_sip_timeout));

				LinphoneAddress *uri = mgr->identity;
				LinphoneConference *pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
				BC_ASSERT_PTR_NULL(pconference);
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
		                             focus_stat.number_of_LinphoneCallEnd + 4, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + 4, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
		                             focus_stat.number_of_LinphoneSubscriptionTerminated + 4,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
		                             focus_stat.number_of_participants_removed + 4, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed + 4,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : {focus.getCMgr()}) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NULL(pconference);
		}

		const bctbx_list_t *calls = linphone_core_get_calls(focus.getLc());
		BC_ASSERT_EQUAL(bctbx_list_size(calls), 0, size_t, "%zu");

		LinphoneCall *focus_berthe_call =
		    linphone_core_get_call_by_remote_address2(focus.getLc(), berthe.getCMgr()->identity);
		BC_ASSERT_PTR_NULL(focus_berthe_call);

		if (!dialout) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
			                             focus_stat.number_of_participants_removed + 4,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
			                             focus_stat.number_of_participant_devices_removed + 4,
			                             liblinphone_tester_sip_timeout));
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
		                             focus_stat.number_of_LinphoneCallEnd + 4, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + 4, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : members) {
			const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
			BC_ASSERT_EQUAL(bctbx_list_size(call_logs), 1, size_t, "%zu");

			bctbx_list_t *mgr_focus_call_log =
			    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
			if (mgr_focus_call_log) {
				BC_ASSERT_EQUAL(bctbx_list_size(mgr_focus_call_log), 1, size_t, "%zu");
				for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
					LinphoneCallLog *call_log = (LinphoneCallLog *)it->data;
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
					LinphoneConferenceInfo *call_log_info = linphone_call_log_get_conference_info(call_log);
					if (BC_ASSERT_PTR_NOT_NULL(call_log_info)) {
						check_conference_info_against_db(mgr, confAddr, call_log_info, FALSE);
					}
				}
				bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
			}

			check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info, start_time,
			                            duration, initialSubject,
			                            (send_ics || (mgr == marie.getCMgr())) ? description : NULL, 0,
			                            LinphoneConferenceInfoStateNew, security_level, FALSE, TRUE, FALSE, FALSE);
		}

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
			return false;
		});

		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

void create_simple_conference_merging_calls_base(bool_t enable_ice,
                                                 LinphoneConferenceLayout layout,
                                                 bool_t toggle_video,
                                                 bool_t toggle_all_mananger_video,
                                                 bool_t change_layout,
                                                 LinphoneConferenceSecurityLevel security_level,
                                                 bool_t enable_screen_sharing) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		bool is_encrypted = (security_level == LinphoneConferenceSecurityLevelEndToEnd);
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), is_encrypted);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), is_encrypted);
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress(), is_encrypted);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);

		setup_conference_info_cbs(marie.getCMgr());

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		LinphoneMediaEncryption encryption = is_encrypted ? LinphoneMediaEncryptionZRTP : LinphoneMediaEncryptionNone;

		auto focus_proxy_config = focus.getDefaultProxyConfig();
		const char *focus_uri = linphone_proxy_config_get_identity(focus_proxy_config);

		auto marie_proxy = marie.getDefaultProxyConfig();
		linphone_proxy_config_edit(marie_proxy);
		linphone_proxy_config_set_conference_factory_uri(marie_proxy, focus_uri);
		linphone_proxy_config_done(marie_proxy);

		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(),
		                                                laure.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()};
		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr()};

		for (auto mgr : conferenceMgrs) {
			if (toggle_video) {
				LinphoneVideoActivationPolicy *pol =
				    linphone_factory_create_video_activation_policy(linphone_factory_get());
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
				linphone_core_set_media_encryption(mgr->lc, encryption);
			}
		}

		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, laure.getLc());

		BC_ASSERT_TRUE(call(marie.getCMgr(), pauline.getCMgr()));

		for (auto mgr : conferenceMgrs) {
			enable_stun_in_mgr(mgr, enable_ice, enable_ice, enable_ice, enable_ice);
		}
		if (is_encrypted) {
			configure_end_to_end_encrypted_conference_server(focus);
		}

		LinphoneCall *marie_call_pauline = linphone_core_get_current_call(marie.getLc());
		BC_ASSERT_PTR_NOT_NULL(marie_call_pauline);
		LinphoneCall *pauline_called_by_marie = linphone_core_get_current_call(pauline.getLc());
		BC_ASSERT_PTR_NOT_NULL(pauline_called_by_marie);
		// linphone_call_set_microphone_muted (pauline_called_by_marie, TRUE);
		BC_ASSERT_TRUE(pause_call_1(marie.getCMgr(), marie_call_pauline, pauline.getCMgr(), pauline_called_by_marie));

		BC_ASSERT_TRUE(call(marie.getCMgr(), laure.getCMgr()));
		LinphoneCall *marie_call_laure = linphone_core_get_current_call(marie.getLc());
		BC_ASSERT_PTR_NOT_NULL(marie_call_laure);

		// marie creates the conference
		LinphoneConferenceParams *conf_params = linphone_core_create_conference_params_2(marie.getLc(), NULL);
		const char *initialSubject = "Test characters: ^ :) ¤ çà @";
		linphone_conference_params_enable_audio(conf_params, TRUE);
		linphone_conference_params_enable_video(conf_params, toggle_video);
		linphone_conference_params_set_security_level(conf_params, security_level);
		linphone_conference_params_set_subject(conf_params, initialSubject);
		LinphoneConference *conf = linphone_core_create_conference_with_params(marie.getLc(), conf_params);
		linphone_conference_params_unref(conf_params);
		BC_ASSERT_PTR_NOT_NULL(conf);

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		std::list<stats> participant_stats;
		for (auto mgr : members) {
			if (mgr != marie.getCMgr()) {
				participant_stats.push_back(mgr->stat);
			}
			participantList.insert(
			    std::make_pair(mgr, add_participant_info_to_list(&participants_info, mgr->identity,
			                                                     LinphoneParticipantRoleSpeaker, -1)));
		}

		if (conf) {
			const bctbx_list_t *calls = linphone_core_get_calls(marie.getLc());
			for (const bctbx_list_t *it = calls; it; it = bctbx_list_next(it)) {
				LinphoneCall *call = (LinphoneCall *)it->data;
				linphone_conference_add_participant(conf, call);
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreationPending, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 1, 20000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_added, 2,
		                             liblinphone_tester_sip_timeout));

		LinphoneAddress *confAddr =
		    conf ? linphone_address_clone(linphone_conference_get_conference_address(conf)) : NULL;
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		int counter = 0;
		for (auto mgr : participants) {
			counter++;
			auto old_stats = participant_stats.front();
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreationPending,
			                             old_stats.number_of_LinphoneConferenceStateCreationPending + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated,
			                             old_stats.number_of_LinphoneConferenceStateCreated + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress,
			                             old_stats.number_of_LinphoneSubscriptionOutgoingProgress + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive,
			                             old_stats.number_of_LinphoneSubscriptionActive + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning, counter + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneTransferCallConnected, counter,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning,
			                             old_stats.number_of_LinphoneCallStreamsRunning + 1,
			                             liblinphone_tester_sip_timeout));

			// End of call between conference and participant
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallEnd, counter,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd,
			                             old_stats.number_of_LinphoneCallEnd + 1, liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallReleased, counter,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased,
			                             old_stats.number_of_LinphoneCallReleased + 1, liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));
			participant_stats.pop_front();
		}

		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
		    fill_member_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure}, conferenceMgrs, focus.getCMgr(), memberList,
		                            confAddr, toggle_video);

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added, 3,
		                             liblinphone_tester_sip_timeout));

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

		for (auto mgr : conferenceMgrs) {
			const LinphoneAddress *local_address = (mgr == focus.getCMgr()) ? confAddr : mgr->identity;
			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, local_address, confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);

			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference),
				                (mgr == focus.getCMgr()) ? 3 : 2, int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 3, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
			}

			if (mgr != focus.getCMgr()) {
				// Local conference
				LinphoneCall *focus_call = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
				BC_ASSERT_PTR_NOT_NULL(focus_call);
				if (focus_call) {
					BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(focus_call));
					BC_ASSERT_TRUE(linphone_call_is_in_conference(focus_call));
					_linphone_call_check_nb_active_streams(focus_call, 1, (toggle_video) ? 4 : 0, 0);
				}

				// Remote  conference
				LinphoneCall *participant_call =
				    linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
				BC_ASSERT_PTR_NOT_NULL(participant_call);
				if (participant_call) {
					BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(participant_call));
					BC_ASSERT_FALSE(linphone_call_is_in_conference(participant_call));
					_linphone_call_check_nb_active_streams(participant_call, 1, (toggle_video) ? 4 : 0, 0);
				}
			}

			if (confAddr) {
				check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info, 0, 0,
				                            initialSubject, NULL, 0, LinphoneConferenceInfoStateNew, security_level,
				                            FALSE, TRUE, toggle_video, FALSE);
			}
		}

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

		// marie creates the conference
		LinphoneConferenceParams *conf_params2 = linphone_core_create_conference_params_2(marie.getLc(), NULL);
		linphone_conference_params_enable_audio(conf_params2, TRUE);
		linphone_conference_params_enable_video(conf_params2, toggle_video);
		const char *initialSubject2 = "This is an error";
		linphone_conference_params_set_subject(conf_params2, initialSubject2);
		LinphoneConference *conf2 = linphone_core_create_conference_with_params(marie.getLc(), conf_params2);
		linphone_conference_params_unref(conf_params2);
		BC_ASSERT_PTR_NOT_NULL(conf2);
		if (conf2) {
			linphone_conference_terminate(conf2);
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &marie.getStats().number_of_LinphoneConferenceStateTerminationPending, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateDeleted, 1,
			                             liblinphone_tester_sip_timeout));
			linphone_conference_unref(conf2);
		}

		participant_stats.clear();
		for (auto mgr : members) {
			participant_stats.push_back(mgr->stat);
		}

		stats focus_stat = focus.getStats();
		const char *newSubject = "Let's go drink a beer";
		linphone_conference_set_subject(conf, newSubject);

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_subject_changed,
		                             focus_stat.number_of_subject_changed + 1, liblinphone_tester_sip_timeout));
		for (auto mgr : members) {
			auto old_stats = participant_stats.front();
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_subject_changed,
			                             old_stats.number_of_subject_changed + 1, liblinphone_tester_sip_timeout));
			if (confAddr) {
				check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info, 0, 0,
				                            newSubject, NULL, 0, LinphoneConferenceInfoStateNew, security_level, FALSE,
				                            TRUE, toggle_video, FALSE);
			}
			participant_stats.pop_front();
		}

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

		LinphoneConference *fconference =
		    linphone_core_search_conference(focus.getLc(), NULL, confAddr, confAddr, NULL);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		for (auto mgr : members) {
			LinphoneAddress *uri = mgr->identity;
			LinphoneConference *pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, laure})
			                   .waitUntil(chrono::seconds(10), [&fconference, &pconference] {
				                   return check_conference_ssrc(fconference, pconference);
			                   }));
		}

		focus_stat = focus.getStats();
		stats marie_stat = marie.getStats();
		stats pauline_stat = pauline.getStats();

		// Laure's core suddenly stops
		ms_message("%s core suddenly loses network and restarts", linphone_core_get_identity(laure.getLc()));
		linphone_core_set_network_reachable(laure.getLc(), FALSE);
		LinphoneSipTransports tr = {0};
		linphone_core_set_sip_transports(laure.getLc(), &tr);
		coresList = bctbx_list_remove(coresList, laure.getLc());

		laure.reStart(false);

		if (toggle_video) {
			LinphoneVideoActivationPolicy *pol =
			    linphone_factory_create_video_activation_policy(linphone_factory_get());
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

		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneRegistrationOk, 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateCreationPending, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateCreated, 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneSubscriptionOutgoingProgress, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneSubscriptionActive, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &laure.getStats().number_of_NotifyReceived, 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallUpdating, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallStreamsRunning, 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + 2,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_added,
		                             marie_stat.number_of_participant_devices_added + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_added,
		                             pauline_stat.number_of_participant_devices_added + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
		                             focus_stat.number_of_conference_participant_devices_present + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
		                             focus_stat.number_of_participant_devices_present + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_conference_participant_devices_present,
		                             marie_stat.number_of_conference_participant_devices_present + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_present,
		                             marie_stat.number_of_participant_devices_present + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_conference_participant_devices_present,
		                             pauline_stat.number_of_conference_participant_devices_present + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_present,
		                             pauline_stat.number_of_participant_devices_present + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_removed,
		                             marie_stat.number_of_participant_devices_removed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_removed,
		                             pauline_stat.number_of_participant_devices_removed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_removed, 1,
		                             liblinphone_tester_sip_timeout));

		if (toggle_video) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating,
			                             marie_stat.number_of_LinphoneCallUpdating + 2,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallUpdating,
			                             pauline_stat.number_of_LinphoneCallUpdating + 2,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallUpdating, 2,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
			                             focus_stat.number_of_LinphoneCallUpdatedByRemote + 6,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
			                             marie_stat.number_of_LinphoneCallStreamsRunning + 2,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning,
			                             pauline_stat.number_of_LinphoneCallStreamsRunning + 2,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallStreamsRunning, 3,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
			                             focus_stat.number_of_LinphoneCallStreamsRunning + 7,
			                             liblinphone_tester_sip_timeout));
		}

		for (auto mgr : conferenceMgrs) {
			const LinphoneAddress *local_address = (mgr == focus.getCMgr()) ? confAddr : mgr->identity;
			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, local_address, confAddr, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);

			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference),
				                (mgr == focus.getCMgr()) ? 3 : 2, int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 3, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), newSubject);
			}

			if (mgr != focus.getCMgr()) {
				// Local conference
				LinphoneCall *focus_call = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
				BC_ASSERT_PTR_NOT_NULL(focus_call);
				if (focus_call) {
					BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(focus_call));
					BC_ASSERT_TRUE(linphone_call_is_in_conference(focus_call));
				}

				// Remote  conference
				LinphoneCall *participant_call =
				    linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
				BC_ASSERT_PTR_NOT_NULL(participant_call);
				if (participant_call) {
					BC_ASSERT_PTR_NOT_NULL(linphone_call_get_conference(participant_call));
					BC_ASSERT_FALSE(linphone_call_is_in_conference(participant_call));
					// BC_ASSERT_TRUE(linphone_call_get_microphone_muted(participant_call)
					// == (mgr == pauline.getCMgr()));
				}
			}

			if (confAddr) {
				check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info, 0, 0,
				                            newSubject, NULL, 0, LinphoneConferenceInfoStateNew, security_level, FALSE,
				                            TRUE, toggle_video, FALSE);
			}
		}

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(5), [] { return false; });

		focus_stat = focus.getStats();
		marie_stat = marie.getStats();
		pauline_stat = pauline.getStats();
		stats laure_stat = laure.getStats();

		if (enable_screen_sharing) {
			// Marie starts sharing its screen
			ms_message("%s enables screen sharing", linphone_core_get_identity(marie.getLc()));
			LinphoneCall *marie_call = linphone_core_get_call_by_remote_address2(marie.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(marie_call);
			if (marie_call) {
				LinphoneCallParams *new_params = linphone_core_create_call_params(marie.getLc(), marie_call);
				linphone_call_params_enable_screen_sharing(new_params, TRUE);
				linphone_call_update(marie_call, new_params);
				linphone_call_params_unref(new_params);

				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating,
				                             marie_stat.number_of_LinphoneCallUpdating + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
				                             marie_stat.number_of_LinphoneCallStreamsRunning + 1,
				                             liblinphone_tester_sip_timeout));

				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
				                             focus_stat.number_of_LinphoneCallUpdatedByRemote + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
				                             focus_stat.number_of_LinphoneCallStreamsRunning + 1,
				                             liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &marie.getStats().number_of_participant_devices_screen_sharing_enabled,
			    marie_stat.number_of_participant_devices_screen_sharing_enabled + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &pauline.getStats().number_of_participant_devices_screen_sharing_enabled,
			    pauline_stat.number_of_participant_devices_screen_sharing_enabled + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &laure.getStats().number_of_participant_devices_screen_sharing_enabled,
			    laure_stat.number_of_participant_devices_screen_sharing_enabled + 1, liblinphone_tester_sip_timeout));
		}

		std::list<LinphoneCoreManager *> mgrList = {pauline.getCMgr()};
		if (toggle_all_mananger_video) {
			mgrList.push_back(marie.getCMgr());
			mgrList.push_back(laure.getCMgr());
		}

		if (toggle_video) {
			for (int i = 0; i < 4; i++) {
				for (auto mgr : mgrList) {
					LinphoneMediaDirection video_direction = LinphoneMediaDirectionSendRecv;

					LinphoneCall *participant_call = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(participant_call);
					if (participant_call) {
						const LinphoneCallParams *call_lparams = linphone_call_get_params(participant_call);
						BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
						video_direction = linphone_call_params_get_video_direction(call_lparams);

						if (video_direction == LinphoneMediaDirectionRecvOnly) {
							video_direction = LinphoneMediaDirectionSendRecv;
						} else if (video_direction == LinphoneMediaDirectionSendRecv) {
							video_direction = LinphoneMediaDirectionRecvOnly;
						}
					}

					set_video_settings_in_conference(focus.getCMgr(), mgr, participants, confAddr, TRUE,
					                                 video_direction, TRUE, video_direction);

					// wait a bit longer to detect side effect if any. During this time, some participants might
					// enable or disable video streams due to video toggling actions. It occurs for example when
					// everybody is using a Grid layout and one participants enables its video stream with direction
					// either SendOnly or SendRecv.
					CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(5), [] {
						return false;
					});

					LinphoneAddress *uri = mgr->identity;
					LinphoneConference *pconference =
					    linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
					BC_ASSERT_PTR_NOT_NULL(pconference);

					if (pconference) {
						bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
						for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
							LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
							LinphoneMediaDirection video_dir =
							    linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo);
							if (linphone_conference_is_me(pconference, linphone_participant_device_get_address(d))) {
								BC_ASSERT_TRUE(
								    linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) ==
								    (video_direction == LinphoneMediaDirectionSendRecv));
							} else {
								BC_ASSERT_TRUE(
								    linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) ==
								    ((video_dir == LinphoneMediaDirectionSendRecv) ||
								     (video_dir == LinphoneMediaDirectionSendOnly)));
							}
						}

						if (devices) {
							bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
						}

						if (change_layout) {
							stats mgr_stat2 = mgr->stat;
							stats focus_stat2 = focus.getStats();

							LinphoneConferenceLayout new_layout = LinphoneConferenceLayoutActiveSpeaker;
							LinphoneCall *pcall2 = linphone_conference_get_call(pconference);
							BC_ASSERT_PTR_NOT_NULL(pcall2);
							if (pcall2) {
								const LinphoneCallParams *pcall2_local_params = linphone_call_get_params(pcall2);
								const LinphoneConferenceLayout conference_layout =
								    linphone_call_params_get_conference_video_layout(pcall2_local_params);

								if (conference_layout == LinphoneConferenceLayoutGrid) {
									new_layout = LinphoneConferenceLayoutActiveSpeaker;
								} else {
									new_layout = LinphoneConferenceLayoutGrid;
								}
								ms_message("%s changes conference layout from %s to %s",
								           linphone_core_get_identity(mgr->lc),
								           linphone_conference_layout_to_string(conference_layout),
								           linphone_conference_layout_to_string(new_layout));

								LinphoneCallParams *call_params = linphone_core_create_call_params(mgr->lc, pcall2);
								linphone_call_params_set_conference_video_layout(call_params, new_layout);
								linphone_call_update(pcall2, call_params);
								linphone_call_params_unref(call_params);
							}

							BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating,
							                             mgr_stat2.number_of_LinphoneCallUpdating + 1,
							                             liblinphone_tester_sip_timeout));
							BC_ASSERT_TRUE(wait_for_list(
							    coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
							    focus_stat2.number_of_LinphoneCallUpdatedByRemote + 1, liblinphone_tester_sip_timeout));
							BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning,
							                             mgr_stat2.number_of_LinphoneCallStreamsRunning + 1,
							                             liblinphone_tester_sip_timeout));
							BC_ASSERT_TRUE(wait_for_list(
							    coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
							    focus_stat2.number_of_LinphoneCallStreamsRunning + 1, liblinphone_tester_sip_timeout));
							if (pcall2) {
								const LinphoneCallParams *pcall2_local_params = linphone_call_get_params(pcall2);
								const LinphoneConferenceLayout remote_conf_layout =
								    linphone_call_params_get_conference_video_layout(pcall2_local_params);
								BC_ASSERT_EQUAL(new_layout, remote_conf_layout, int, "%d");
							}
							LinphoneConference *fconference =
							    linphone_core_search_conference(focus.getLc(), NULL, NULL, confAddr, NULL);
							LinphoneParticipant *participant =
							    linphone_conference_find_participant(fconference, mgr->identity);
							BC_ASSERT_PTR_NOT_NULL(participant);
							if (participant) {
								bctbx_list_t *devices = linphone_participant_get_devices(participant);

								for (bctbx_list_t *it_d = devices; it_d != NULL; it_d = it_d->next) {
									LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)it_d->data;
									BC_ASSERT_PTR_NOT_NULL(d);
									LinphoneCall *participant_call = linphone_core_get_call_by_remote_address2(
									    focus.getLc(), linphone_participant_device_get_address(d));
									BC_ASSERT_PTR_NOT_NULL(participant_call);
									if (participant_call) {
										const LinphoneCallParams *call_remote_params =
										    linphone_call_get_remote_params(participant_call);
										const LinphoneConferenceLayout device_layout =
										    linphone_call_params_get_conference_video_layout(call_remote_params);
										BC_ASSERT_EQUAL(device_layout, new_layout, int, "%d");
									}
								}
								bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
							}

							LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
							BC_ASSERT_PTR_NOT_NULL(pcall);
							if (pcall) {
								const LinphoneCallParams *call_lparams = linphone_call_get_params(pcall);
								BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
								const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(pcall);
								BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
								const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
								BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams));
							}

							LinphoneAddress *uri = mgr->identity;
							LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), uri);
							BC_ASSERT_PTR_NOT_NULL(ccall);
							if (ccall) {
								const LinphoneCallParams *call_lparams = linphone_call_get_params(ccall);
								BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_lparams));
								const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(ccall);
								BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_rparams));
								const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
								BC_ASSERT_TRUE(linphone_call_params_video_enabled(call_cparams));
							}
						}
					}

					// Wait a little bit
					wait_for_list(coresList, NULL, 0, 1000);
				}
			}
		}

		const int total_marie_calls =
		    marie.getStats().number_of_LinphoneCallEnd + (int)bctbx_list_size(linphone_core_get_calls(marie.getLc()));
		const int total_focus_calls =
		    focus.getStats().number_of_LinphoneCallEnd + (int)bctbx_list_size(linphone_core_get_calls(focus.getLc()));
		const int total_pauline_calls = pauline.getStats().number_of_LinphoneCallEnd +
		                                (int)bctbx_list_size(linphone_core_get_calls(pauline.getLc()));
		const int total_laure_calls =
		    laure.getStats().number_of_LinphoneCallEnd + (int)bctbx_list_size(linphone_core_get_calls(laure.getLc()));

		linphone_core_terminate_all_calls(pauline.getLc());
		linphone_core_terminate_all_calls(laure.getLc());
		linphone_core_terminate_all_calls(marie.getLc());

		// Wait for calls to be terminated
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallEnd, total_marie_calls, 30000));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallEnd, total_pauline_calls, 30000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallEnd, total_laure_calls, 30000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd, total_focus_calls, 40000));

		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallReleased, total_marie_calls, 30000));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallReleased, total_pauline_calls, 30000));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallReleased, total_laure_calls, 30000));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased, total_focus_calls, 40000));

		for (auto mgr : conferenceMgrs) {
			int nb_conferences = (mgr == marie.getCMgr()) ? 2 : 1;
			// Wait for all conferences to be terminated
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending,
			                             nb_conferences, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated,
			                             nb_conferences, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, nb_conferences,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated,
			                             (mgr == focus.getCMgr()) ? 3 : 1, liblinphone_tester_sip_timeout));

			if (mgr && (mgr != focus.getCMgr())) {
				LinphoneCall *participant_call =
				    linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
				BC_ASSERT_PTR_NULL(participant_call);
				LinphoneCall *conference_call = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
				BC_ASSERT_PTR_NULL(conference_call);

				const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
				BC_ASSERT_EQUAL(bctbx_list_size(call_logs),
				                (((mgr == marie.getCMgr()) || (mgr == laure.getCMgr())) ? 3 : 2), size_t, "%zu");

				bctbx_list_t *mgr_focus_call_log =
				    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
				BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
				if (mgr_focus_call_log) {
					BC_ASSERT_EQUAL(bctbx_list_size(mgr_focus_call_log), (mgr == laure.getCMgr()) ? 2 : 1, size_t,
					                "%zu");
					for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
						LinphoneCallLog *call_log = (LinphoneCallLog *)it->data;
						BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
					}
					bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
				}
			}
		}

		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		linphone_conference_unref(conf);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

void create_conference_dial_out_base(LinphoneConferenceLayout layout,
                                     LinphoneVideoActivationPolicy *pol,
                                     bool_t enable_stun,
                                     bool_t enable_ice,
                                     LinphoneConferenceParticipantListType participant_list_type,
                                     bool_t accept,
                                     bool_t participant_codec_mismatch,
                                     LinphoneConferenceSecurityLevel security_level,
                                     bool_t version_mismatch,
                                     bool_t enable_chat) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		bool_t enable_lime = (security_level == LinphoneConferenceSecurityLevelEndToEnd ? TRUE : FALSE);
		// Cannot send ICS in dial out conferences
		bool_t send_ics = FALSE;

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

		bctbx_list_t *coresList = NULL;

		bool_t enable_video = !!linphone_video_activation_policy_get_automatically_accept(pol) ||
		                      !!linphone_video_activation_policy_get_automatically_initiate(pol);
		bool_t initiate_video = !!linphone_video_activation_policy_get_automatically_initiate(pol);
		bool_t accept_video = !!linphone_video_activation_policy_get_automatically_accept(pol);

		int nb_subscriptions = 1;
		if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
			nb_subscriptions = 2; // One more subscription for the EKT
			configure_end_to_end_encrypted_conference_server(focus);
		}

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(),
		                 berthe.getCMgr()}) {
			if (participant_codec_mismatch) {
				if (mgr == michelle.getCMgr()) {
					disable_all_audio_codecs_except_one(mgr->lc, "pcmu", -1);
				} else {
					disable_all_audio_codecs_except_one(mgr->lc, "pcma", -1);
				}
			}

			linphone_core_set_video_activation_policy(mgr->lc, pol);

			linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);
			linphone_core_enable_video_capture(mgr->lc, TRUE);
			linphone_core_enable_video_display(mgr->lc, TRUE);

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, layout);
			}

			enable_stun_in_mgr(mgr, enable_stun, enable_ice, enable_stun, enable_ice);

			coresList = bctbx_list_append(coresList, mgr->lc);

			if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
				BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(mgr->lc));
			}
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), participant_list_type);

		stats focus_stat = focus.getStats();
		stats marie_stat = marie.getStats();

		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(),    pauline.getCMgr(),
		                                                laure.getCMgr(), michelle.getCMgr(), berthe.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(),
		                                         michelle.getCMgr(), berthe.getCMgr()};
		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(),
		                                              berthe.getCMgr()};
		std::list<LinphoneCoreManager *> codec_mismatch_members;

		const char *initialSubject = "Schedule of the trip towards the top of Europe";
		const char *description = "To the top of the Mont Blanc!!!! :-)";

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		LinphoneParticipantRole role = LinphoneParticipantRoleSpeaker;
		for (auto &p : participants) {
			participantList.insert(
			    std::make_pair(p, add_participant_info_to_list(&participants_info, p->identity, role, -1)));
			if (role == LinphoneParticipantRoleSpeaker) {
				role = LinphoneParticipantRoleListener;
			} else if (role == LinphoneParticipantRoleListener) {
				role = LinphoneParticipantRoleUnknown;
			} else if (role == LinphoneParticipantRoleUnknown) {
				role = LinphoneParticipantRoleSpeaker;
			}
		}

		if (version_mismatch) {
			linphone_core_remove_linphone_spec(laure.getLc(), "conference");
			linphone_core_add_linphone_spec(laure.getLc(), "conference/1.0");
		}

		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, -1, -1, initialSubject, description, send_ics,
		                                security_level, TRUE, enable_chat, NULL);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		// Chat room creation to send ICS
		int marie_conferences = ((send_ics) ? 1 : 0);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated,
		                             marie_conferences, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallOutgoingInit,
		                             marie_stat.number_of_LinphoneCallOutgoingInit + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingInit,
		                             focus_stat.number_of_LinphoneCallOutgoingInit + 4,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + 1,
		                             liblinphone_tester_sip_timeout));

		if (participant_codec_mismatch) {
			codec_mismatch_members.push_back(michelle.getCMgr());

			for (const auto &m : codec_mismatch_members) {
				auto itConferenceMgrs = std::find(conferenceMgrs.begin(), conferenceMgrs.end(), m);
				if (itConferenceMgrs != conferenceMgrs.end()) {
					conferenceMgrs.erase(itConferenceMgrs);
				}

				auto itParticipants = std::find(participants.begin(), participants.end(), m);
				if (itParticipants != participants.end()) {
					participants.erase(itParticipants);
				}

				auto itMembers = std::find(members.begin(), members.end(), m);
				if (itMembers != members.end()) {
					members.erase(itMembers);
				}

				bctbx_list_t *focus_call_log =
				    linphone_core_get_call_history_2(focus.getLc(), m->identity, focus.getCMgr()->identity);
				BC_ASSERT_PTR_NOT_NULL(focus_call_log);
				if (focus_call_log) {
					BC_ASSERT_EQUAL(bctbx_list_size(focus_call_log), 1, size_t, "%zu");
					for (bctbx_list_t *it = focus_call_log; it; it = bctbx_list_next(it)) {
						LinphoneCallLog *call_log = (LinphoneCallLog *)it->data;
						BC_ASSERT_EQUAL(linphone_call_log_get_status(call_log), LinphoneCallAborted, int, "%d");
					}
					bctbx_list_free_with_data(focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
				}

				bctbx_list_t *member_call_log =
				    linphone_core_get_call_history_2(m->lc, focus.getCMgr()->identity, m->identity);
				BC_ASSERT_PTR_NOT_NULL(member_call_log);
				if (member_call_log) {
					BC_ASSERT_EQUAL(bctbx_list_size(member_call_log), 1, size_t, "%zu");
					for (bctbx_list_t *it = member_call_log; it; it = bctbx_list_next(it)) {
						LinphoneCallLog *call_log = (LinphoneCallLog *)it->data;
						BC_ASSERT_EQUAL(linphone_call_log_get_status(call_log), LinphoneCallAborted, int, "%d");
					}
					bctbx_list_free_with_data(member_call_log, (bctbx_list_free_func)linphone_call_log_unref);
				}
			}
		}

		for (auto mgr : participants) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallIncomingReceived, 1,
			                             liblinphone_tester_sip_timeout));
		}

		LinphoneConference *oconference = linphone_core_search_conference_2(marie.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(oconference);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
		                             marie_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		if (enable_ice) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating,
			                             marie_stat.number_of_LinphoneCallUpdating + 1, 20000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
			                             focus_stat.number_of_LinphoneCallUpdatedByRemote + 1,
			                             liblinphone_tester_sip_timeout));
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
		                             marie_stat.number_of_LinphoneCallStreamsRunning + 2,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionOutgoingProgress,
		                             marie_stat.number_of_LinphoneSubscriptionOutgoingProgress + nb_subscriptions,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
		                             marie_stat.number_of_LinphoneSubscriptionActive + nb_subscriptions,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyFullStateReceived,
		                             marie_stat.number_of_NotifyFullStateReceived + 1, liblinphone_tester_sip_timeout));
		if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyEktReceived,
			                             marie_stat.number_of_NotifyEktReceived + 1, liblinphone_tester_sip_timeout));
		}

		if (BC_ASSERT_PTR_NOT_NULL(oconference)) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(oconference), 4, int, "%0d");
			CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
			    .waitUntil(chrono::seconds(2), [&oconference, &members] {
				    bctbx_list_t *devices = linphone_conference_get_participant_device_list(oconference);
				    const size_t no_devices = bctbx_list_size(devices);
				    if (devices) {
					    bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				    }
				    return (no_devices == members.size());
			    });
		}

		bctbx_list_t *participants_info2 = NULL;
		for (bctbx_list_t *info_it = participants_info; info_it; info_it = bctbx_list_next(info_it)) {
			const LinphoneParticipantInfo *participant_info_el =
			    (LinphoneParticipantInfo *)bctbx_list_get_data(info_it);
			const LinphoneAddress *address = linphone_participant_info_get_address(participant_info_el);
			const int sequence = (send_ics) ? 0 : -1;
			LinphoneParticipantRole role = LinphoneParticipantRoleUnknown;
			LinphoneParticipantRole current_role = linphone_participant_info_get_role(participant_info_el);
			if (send_ics) {
				if (current_role == LinphoneParticipantRoleUnknown) {
					role = LinphoneParticipantRoleSpeaker;
				} else {
					role = current_role;
				}
			} else {
				role = LinphoneParticipantRoleUnknown;
			}
			add_participant_info_to_list(&participants_info2, address, role, sequence);
		}

		if (!send_ics) {
			add_participant_info_to_list(&participants_info2, marie.getCMgr()->identity, LinphoneParticipantRoleUnknown,
			                             -1);
		}

		if (confAddr) {
			for (auto mgr : participants) {
				// Encryption is None because we haven't received yet the NOTIFY full state yet
				check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info2, 0, 0,
				                            initialSubject, (accept && send_ics) ? description : NULL, 0,
				                            LinphoneConferenceInfoStateNew, LinphoneConferenceSecurityLevelNone, FALSE,
				                            TRUE, TRUE, FALSE);

				LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(pcall);
				if (pcall) {
					LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
					if (accept) {
						linphone_call_accept(pcall);
					} else {
						linphone_call_decline(pcall, LinphoneReasonDeclined);
					}
				}
			}
		}

		bctbx_list_free_with_data(participants_info2, (bctbx_list_free_func)linphone_participant_info_unref);

		add_participant_info_to_list(&participants_info, marie.getCMgr()->identity, LinphoneParticipantRoleSpeaker,
		                             (send_ics) ? 0 : -1);

		if (accept) {
			update_sequence_number(&participants_info, {}, (send_ics) ? 0 : -1, -1);

			int participant_no = static_cast<int>(participants.size());
			for (auto mgr : participants) {
				bctbx_list_t *participants_info3 = NULL;
				for (bctbx_list_t *info_it = participants_info; info_it; info_it = bctbx_list_next(info_it)) {
					const LinphoneParticipantInfo *participant_info_el =
					    (LinphoneParticipantInfo *)bctbx_list_get_data(info_it);
					const LinphoneAddress *address = linphone_participant_info_get_address(participant_info_el);
					const int sequence = linphone_participant_info_get_sequence_number(participant_info_el);
					bool is_laure_info = linphone_address_weak_equal(address, laure.getCMgr()->identity);
					bool found = false;
					for (const auto &mgr : codec_mismatch_members) {
						found |= !!linphone_address_weak_equal(address, mgr->identity);
					}
					LinphoneParticipantRole role = LinphoneParticipantRoleUnknown;
					if ((!version_mismatch) || (mgr == pauline.getCMgr()) ||
					    (mgr == laure.getCMgr() && is_laure_info) ||
					    ((mgr == berthe.getCMgr() || mgr == michelle.getCMgr()) && !is_laure_info)) {
						LinphoneParticipantRole current_role = linphone_participant_info_get_role(participant_info_el);
						if (found) {
							role = LinphoneParticipantRoleUnknown;
						} else if (current_role == LinphoneParticipantRoleUnknown) {
							role = LinphoneParticipantRoleSpeaker;
						} else {
							role = current_role;
						}
					}
					add_participant_info_to_list(&participants_info3, address, role, sequence);
				}

				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1,
				                             liblinphone_tester_sip_timeout));
				if (mgr == laure.getCMgr() && version_mismatch) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1,
					                             liblinphone_tester_sip_timeout));
				} else {
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress,
					                             nb_subscriptions, liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive,
					                             nb_subscriptions, liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
					                             liblinphone_tester_sip_timeout));
					if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
						// Network traffic might cause that it takes along time to receive the EKT NOTIFY
						BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyEktReceived, 1,
						                             2 * liblinphone_tester_sip_timeout));
					}
					check_conference_info_in_db(
					    mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info3, 0, 0, initialSubject,
					    (send_ics) ? description : NULL, 0, LinphoneConferenceInfoStateNew,
					    mgr == laure.getCMgr() && version_mismatch ? LinphoneConferenceSecurityLevelNone
					                                               : security_level,
					    FALSE, TRUE, TRUE, enable_chat);
				}

				LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
				if (mgr == laure.getCMgr() && version_mismatch) {
					BC_ASSERT_PTR_NULL(pcall);
				} else {
					BC_ASSERT_PTR_NOT_NULL(pcall);
				}
				if (pcall) {
					LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
					LinphoneConferenceInfo *call_log_info = linphone_call_log_get_conference_info(call_log);
					if (BC_ASSERT_PTR_NOT_NULL(call_log_info)) {
						check_conference_info_against_db(mgr, confAddr, call_log_info, FALSE);
					}
				}

				bctbx_list_free_with_data(participants_info3, (bctbx_list_free_func)linphone_participant_info_unref);
			}

			if (version_mismatch) {
				conferenceMgrs.remove(laure.getCMgr());
				members.remove(laure.getCMgr());
				participants.remove(laure.getCMgr());
			}

			if (enable_ice) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdating,
				                             focus_stat.number_of_LinphoneCallUpdating + participant_no - 1,
				                             liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
			                             focus_stat.number_of_LinphoneCallStreamsRunning +
			                                 ((enable_ice) ? 2 : 1) * static_cast<int>(participants.size() + 1),
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated,
			                             marie_stat.number_of_LinphoneConferenceStateCreated + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionOutgoingProgress,
			                             marie_stat.number_of_LinphoneSubscriptionOutgoingProgress + nb_subscriptions,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
			                             marie_stat.number_of_LinphoneSubscriptionActive + nb_subscriptions,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyFullStateReceived,
			                             marie_stat.number_of_NotifyFullStateReceived + 1,
			                             liblinphone_tester_sip_timeout));
			if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyEktReceived,
				                             marie_stat.number_of_NotifyEktReceived + participant_no,
				                             liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
			                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
			                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived +
			                                 (participant_no * nb_subscriptions),
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
			                  focus_stat.number_of_LinphoneSubscriptionActive + (participant_no * nb_subscriptions),
			                  liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
			                             focus_stat.number_of_participants_added + participant_no,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
			                             focus_stat.number_of_participant_devices_added + participant_no,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
			                             focus_stat.number_of_conference_participant_devices_present + participant_no,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
			                             focus_stat.number_of_participant_devices_present + participant_no,
			                             liblinphone_tester_sip_timeout));

			LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(fconference);

			std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
			    fill_member_list(members, participantList, marie.getCMgr(), participants_info);
			wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs,
			                            focus.getCMgr(), memberList, confAddr, enable_video);

			for (const auto &mgr : conferenceMgrs) {
				BC_ASSERT_TRUE(check_conference_info_by_participant(mgr, members, 1, confAddr));
			}

			// wait a bit longer to detect side effect if any
			CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(15), [] {
				return false;
			});

			for (auto mgr : conferenceMgrs) {
				LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
				LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
				linphone_address_unref(uri);
				BC_ASSERT_PTR_NOT_NULL(pconference);

				const LinphoneVideoActivationPolicy *mgr_pol = linphone_core_get_video_activation_policy(mgr->lc);
				bool_t video_enabled =
				    !!((mgr == marie.getCMgr()) ? linphone_video_activation_policy_get_automatically_initiate(mgr_pol)
				                                : linphone_video_activation_policy_get_automatically_accept(mgr_pol));

				if (mgr != focus.getCMgr()) {
					const LinphoneAddress *from = (mgr == marie.getCMgr()) ? mgr->identity : confAddr;
					const LinphoneAddress *to = (mgr == marie.getCMgr()) ? confAddr : mgr->identity;

					LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
						BC_ASSERT_PTR_NOT_NULL(call_log);
						if (call_log) {
							BC_ASSERT_TRUE(
							    linphone_address_weak_equal(linphone_call_log_get_from_address(call_log), from));
							BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_call_log_get_to_address(call_log), to));
						}
					}
					LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
					BC_ASSERT_PTR_NOT_NULL(ccall);
					if (ccall) {
						LinphoneCallLog *call_log = linphone_call_get_call_log(ccall);
						BC_ASSERT_PTR_NOT_NULL(call_log);
						if (call_log) {
							BC_ASSERT_TRUE(
							    linphone_address_weak_equal(linphone_call_log_get_from_address(call_log), from));
							BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_call_log_get_to_address(call_log), to));
						}
					}
				}

				if (pconference) {
					int no_participants = 0;
					if (mgr == focus.getCMgr()) {
						no_participants = static_cast<int>(members.size());
						BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
					} else {
						bctbx_list_t *participants_info3 = NULL;
						for (bctbx_list_t *info_it = participants_info; info_it; info_it = bctbx_list_next(info_it)) {
							const LinphoneParticipantInfo *participant_info_el =
							    (LinphoneParticipantInfo *)bctbx_list_get_data(info_it);
							const LinphoneAddress *address = linphone_participant_info_get_address(participant_info_el);
							const int sequence = linphone_participant_info_get_sequence_number(participant_info_el);
							bool found = false;
							for (const auto &mgr : codec_mismatch_members) {
								found |= !!linphone_address_weak_equal(address, mgr->identity);
							}
							LinphoneParticipantRole role = LinphoneParticipantRoleUnknown;
							if ((!version_mismatch) || (mgr == marie.getCMgr()) || (mgr == pauline.getCMgr()) ||
							    ((mgr == michelle.getCMgr() || mgr == berthe.getCMgr()) &&
							     !linphone_address_weak_equal(address, laure.getCMgr()->identity))) {
								LinphoneParticipantRole current_role =
								    linphone_participant_info_get_role(participant_info_el);
								if (found && (mgr != marie.getCMgr())) {
									role = LinphoneParticipantRoleUnknown;
								} else if (((mgr == marie.getCMgr()) || !found) &&
								           (current_role == LinphoneParticipantRoleUnknown)) {
									role = LinphoneParticipantRoleSpeaker;
								} else {
									role = current_role;
								}
							}
							add_participant_info_to_list(&participants_info3, address, role, sequence);
						}

						check_conference_info_in_db(
						    mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info3, 0, 0, initialSubject,
						    (send_ics || (mgr == marie.getCMgr())) ? description : NULL, 0,
						    LinphoneConferenceInfoStateNew, security_level, FALSE, TRUE, TRUE, enable_chat);
						bctbx_list_free_with_data(participants_info3,
						                          (bctbx_list_free_func)linphone_participant_info_unref);

						no_participants = participant_no;
						if (version_mismatch) no_participants -= 1;
						BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
						LinphoneCall *current_call = linphone_core_get_current_call(mgr->lc);
						BC_ASSERT_PTR_NOT_NULL(current_call);
						if (current_call) {
							BC_ASSERT_EQUAL((int)linphone_call_get_state(current_call),
							                (int)LinphoneCallStateStreamsRunning, int, "%0d");
						}
						LinphoneParticipantRole role =
						    linphone_participant_get_role(linphone_conference_get_me(pconference));
						if (enable_ice && (role != LinphoneParticipantRoleListener) &&
						    (layout != LinphoneConferenceLayoutGrid)) {
							BC_ASSERT_TRUE(check_ice(mgr, focus.getCMgr(), LinphoneIceStateHostConnection));
						}

						const LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(mgr->lc);
						bool_t enabled = !!((mgr == marie.getCMgr())
						                        ? linphone_video_activation_policy_get_automatically_initiate(pol)
						                        : linphone_video_activation_policy_get_automatically_accept(pol));

						size_t no_streams_audio = 0;
						size_t no_max_streams_audio = (security_level == LinphoneConferenceSecurityLevelEndToEnd)
						                                  ? bctbx_list_size(participants_info)
						                                  : 1;
						size_t no_max_streams_video = (enabled || (mgr == marie.getCMgr()))
						                                  ? ((security_level == LinphoneConferenceSecurityLevelEndToEnd)
						                                         ? 2 * (participants.size() + 1)
						                                         : (participants.size() + 2))
						                                  : 1;
						size_t no_streams_video = 0;
						size_t no_streams_text = 0;

						LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
						BC_ASSERT_PTR_NOT_NULL(pcall);
						if (pcall) {
							no_streams_audio = compute_no_audio_streams(pcall, pconference);
							// Even if video is not enabled, the server will offer it and clients reject the video
							// stream if they do not want to send or receive it.
							no_streams_video = compute_no_video_streams(enable_video, pcall, pconference);
							_linphone_call_check_max_nb_streams(pcall, no_max_streams_audio, no_max_streams_video,
							                                    no_streams_text);
							_linphone_call_check_nb_active_streams(pcall, no_streams_audio, no_streams_video,
							                                       no_streams_text);
							const LinphoneCallParams *call_lparams = linphone_call_get_params(pcall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), video_enabled, int,
							                "%0d");
							const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(pcall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), video_enabled, int,
							                "%0d");
							const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");

							LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
							BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
							LinphoneConferenceInfo *call_log_info = linphone_call_log_get_conference_info(call_log);
							if (BC_ASSERT_PTR_NOT_NULL(call_log_info)) {
								check_conference_info_against_db(mgr, confAddr, call_log_info, FALSE);
							}
						}

						LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
						BC_ASSERT_PTR_NOT_NULL(ccall);
						if (ccall) {
							_linphone_call_check_max_nb_streams(ccall, no_max_streams_audio, no_max_streams_video,
							                                    no_streams_text);
							_linphone_call_check_nb_active_streams(ccall, no_streams_audio, no_streams_video,
							                                       no_streams_text);
							const LinphoneCallParams *call_lparams = linphone_call_get_params(ccall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), video_enabled, int,
							                "%0d");
							const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(ccall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), video_enabled, int,
							                "%0d");
							const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), video_enabled, int,
							                "%0d");
						}
					}
					BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int,
					                "%0d");
					bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
					BC_ASSERT_EQUAL(bctbx_list_size(devices), members.size(), size_t, "%zu");
					if (devices) {
						bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
					}
					BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
					check_conference_me(pconference, ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())));
					bctbx_list_t *participants = linphone_conference_get_participant_list(pconference);
					for (bctbx_list_t *itp = participants; itp; itp = bctbx_list_next(itp)) {
						LinphoneParticipant *p = (LinphoneParticipant *)bctbx_list_get_data(itp);
						BC_ASSERT_TRUE(linphone_participant_is_admin(p) ==
						               linphone_address_weak_equal(linphone_participant_get_address(p),
						                                           marie.getCMgr()->identity));
					}
					bctbx_list_free_with_data(participants, (void (*)(void *))linphone_participant_unref);

					if (mgr != focus.getCMgr()) {
						BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
						                   .waitUntil(chrono::seconds(10), [&fconference, &pconference] {
							                   return check_conference_ssrc(fconference, pconference);
						                   }));
					}

					LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(conference);
					if (conference) {
						bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
						for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
							LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
							// If we are currently carrying out checks on the conference server side, the must set
							// the value of video enabled flag for each participant. In fact each call session may
							// have the video enabled or not and this is taken into account to compute the video
							// availablity flag. Nonetheless, this is not required for the participants as they only
							// have one call session towards the conference server therefore we can reuse the value
							// computed earlier on.
							if (mgr == focus.getCMgr()) {
								if (linphone_address_weak_equal(marie.getIdentity().toC(),
								                                linphone_participant_device_get_address(d))) {
									// The organizer will not offer video streams in its INVITE to join a conference
									// if the policy doesn't allow it
									if (!initiate_video) {
										video_enabled = FALSE;
									} else {
										video_enabled = TRUE;
									}
								} else {
									// The participants will not accept video streams to answer the conference
									// server INVITE to join a conference if the policy doesn't allow it
									if (!accept_video) {
										video_enabled = FALSE;
									} else {
										video_enabled = TRUE;
									}
								}
							}

							bool video_available =
							    !!linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo);
							if (enable_video) {
								LinphoneMediaDirection video_direction =
								    linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo);
								BC_ASSERT_TRUE(video_available ==
								               (((video_direction == LinphoneMediaDirectionSendOnly) ||
								                 (video_direction == LinphoneMediaDirectionSendRecv)) &&
								                video_enabled));
							} else {
								BC_ASSERT_FALSE(video_available);
							}
						}

						if (devices) {
							bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
						}
					}
				}
			}

			if (enable_video) {
				LinphoneCall *pauline_call = linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
				BC_ASSERT_PTR_NOT_NULL(pauline_call);

				Address paulineAddr = pauline.getIdentity();
				LinphoneCall *focus_call = linphone_core_get_call_by_remote_address2(focus.getLc(), paulineAddr.toC());
				BC_ASSERT_PTR_NOT_NULL(focus_call);

				const LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(pauline.getLc());
				bool_t enable = !!!linphone_video_activation_policy_get_automatically_accept(pol);

				LinphoneAddress *paulineUri = linphone_address_new(linphone_core_get_identity(pauline.getLc()));
				LinphoneConference *paulineConference =
				    linphone_core_search_conference(pauline.getLc(), NULL, paulineUri, confAddr, NULL);
				linphone_address_unref(paulineUri);
				BC_ASSERT_PTR_NOT_NULL(paulineConference);

				for (int i = 0; i < 4; i++) {
					set_video_settings_in_conference(focus.getCMgr(), pauline.getCMgr(), members, confAddr, enable,
					                                 LinphoneMediaDirectionSendRecv, enable,
					                                 LinphoneMediaDirectionSendRecv);

					if (paulineConference) {
						bctbx_list_t *devices = linphone_conference_get_participant_device_list(paulineConference);
						for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
							LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
							if (enable) {
								if (linphone_conference_is_me(paulineConference,
								                              linphone_participant_device_get_address(d))) {
									BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(
									    d, LinphoneStreamTypeVideo));
								} else {
									BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(
									                   d, LinphoneStreamTypeVideo) ==
									               (linphone_participant_device_get_stream_capability(
									                    d, LinphoneStreamTypeVideo) == LinphoneMediaDirectionSendRecv));
								}
							} else {
								BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(
								                   d, LinphoneStreamTypeVideo) == enable);
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

			time_t expiry_time = -1;
			long expire_period = linphone_core_get_conference_expire_period(focus.getLc());
			if (expire_period >= 0) {
				LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
				BC_ASSERT_PTR_NOT_NULL(fconference);
				if (fconference) {
					bctbx_list_t *devices = linphone_conference_get_participant_device_list(fconference);
					for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
						LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
						time_t time_of_joining = linphone_participant_device_get_time_of_joining(d);
						if (time_of_joining > expiry_time) {
							expiry_time = time_of_joining;
						}
					}

					if (devices) {
						bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
					}
				}

				BC_ASSERT_GREATER(expiry_time, 0, long long int, "%lld");
				if (expiry_time >= 0) {
					expiry_time += expire_period;
				}
			}

			focus_stat = focus.getStats();
			for (auto mgr : members) {
				LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
				BC_ASSERT_PTR_NOT_NULL(call);
				if (call) {
					ms_message("%s is terminating call with %s", linphone_core_get_identity(mgr->lc),
					           linphone_core_get_identity(focus.getLc()));
					linphone_call_terminate(call);
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated, 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList,
					                             &mgr->stat.number_of_LinphoneConferenceStateTerminationPending, 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated, 1,
					                             liblinphone_tester_sip_timeout));
					if (enable_chat) {
						BC_ASSERT_FALSE(
						    wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1, 2000));
					} else {
						BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1,
						                             liblinphone_tester_sip_timeout));
					}

					LinphoneConference *pconference =
					    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr, NULL);
					if (enable_chat) {
						BC_ASSERT_PTR_NOT_NULL(pconference);
					} else {
						BC_ASSERT_PTR_NULL(pconference);
					}
				}
			}

			int members_no = static_cast<int>(members.size());
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
			                             focus_stat.number_of_LinphoneCallEnd + members_no,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
			                             focus_stat.number_of_LinphoneCallReleased + members_no,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
			                             focus_stat.number_of_LinphoneSubscriptionTerminated + members_no,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
			                             focus_stat.number_of_participants_removed + members_no,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
			                             focus_stat.number_of_participant_devices_removed + members_no,
			                             liblinphone_tester_sip_timeout));

			for (auto mgr : {focus.getCMgr()}) {
				LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NULL(pconference);
			}

			focus_stat = focus.getStats();
			const bctbx_list_t *calls = linphone_core_get_calls(focus.getLc());
			BC_ASSERT_EQUAL(bctbx_list_size(calls), 0, size_t, "%zu");

			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &focus.getStats().number_of_LinphoneConferenceStateTerminationPending, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
			                             liblinphone_tester_sip_timeout));

			if (expiry_time >= 0) {
				check_delete_focus_conference_info({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs,
				                                   focus.getCMgr(), confAddr, expiry_time);
			}
		} else {
			int members_no = static_cast<int>(members.size());

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
			                             focus_stat.number_of_LinphoneCallEnd + 4, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
			                             focus_stat.number_of_LinphoneCallReleased + 4,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
			                             focus_stat.number_of_participants_removed + 4,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
			                             focus_stat.number_of_participant_devices_removed + 4,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed, 4,
			                             2 * liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_removed, 4,
			                             liblinphone_tester_sip_timeout));

			for (auto mgr : participants) {
				BC_ASSERT_TRUE(
				    wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1,
				                             liblinphone_tester_sip_timeout));
			}

			ms_message("%s is terminating call with %s", linphone_core_get_identity(marie.getLc()),
			           linphone_core_get_identity(focus.getLc()));
			LinphoneCall *call = linphone_core_get_call_by_remote_address2(marie.getLc(), focus.getCMgr()->identity);
			linphone_call_terminate(call);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallEnd, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallReleased, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
			                             focus_stat.number_of_LinphoneCallEnd + members_no,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
			                             focus_stat.number_of_LinphoneCallReleased + members_no,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
			                             focus_stat.number_of_participants_removed + members_no,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
			                             focus_stat.number_of_participant_devices_removed + members_no,
			                             liblinphone_tester_sip_timeout));
		}

		for (auto mgr : members) {
			const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
			BC_ASSERT_EQUAL(bctbx_list_size(call_logs), 1, size_t, "%zu");

			bctbx_list_t *mgr_focus_call_log =
			    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
			if (mgr_focus_call_log) {
				BC_ASSERT_EQUAL(bctbx_list_size(mgr_focus_call_log), 1, size_t, "%zu");
				for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
					LinphoneCallLog *call_log = (LinphoneCallLog *)it->data;
					LinphoneConferenceInfo *call_log_info = linphone_call_log_get_conference_info(call_log);
					if (BC_ASSERT_PTR_NOT_NULL(call_log_info)) {
						check_conference_info_against_db(mgr, confAddr, call_log_info, FALSE);
					}
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				}
				bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
			}

			bctbx_list_t *participants_info3 = NULL;
			for (bctbx_list_t *info_it = participants_info; info_it; info_it = bctbx_list_next(info_it)) {
				const LinphoneParticipantInfo *participant_info_el =
				    (LinphoneParticipantInfo *)bctbx_list_get_data(info_it);
				const LinphoneAddress *address = linphone_participant_info_get_address(participant_info_el);
				const int sequence = linphone_participant_info_get_sequence_number(participant_info_el);
				bool found = false;
				for (const auto &mgr : codec_mismatch_members) {
					found |= !!linphone_address_weak_equal(address, mgr->identity);
				}
				LinphoneParticipantRole role = LinphoneParticipantRoleUnknown;
				if ((!version_mismatch) || (mgr == marie.getCMgr()) || (mgr == pauline.getCMgr()) ||
				    ((mgr == michelle.getCMgr() || mgr == berthe.getCMgr()) &&
				     !linphone_address_weak_equal(address, laure.getCMgr()->identity))) {
					LinphoneParticipantRole current_role = linphone_participant_info_get_role(participant_info_el);
					if ((mgr != marie.getCMgr()) && (found || !accept)) {
						role = LinphoneParticipantRoleUnknown;
					} else if (((mgr == marie.getCMgr()) || !found) &&
					           (current_role == LinphoneParticipantRoleUnknown)) {
						role = LinphoneParticipantRoleSpeaker;
					} else {
						role = current_role;
					}
				}
				add_participant_info_to_list(&participants_info3, address, role, sequence);
			}

			check_conference_info_in_db(
			    mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info3, 0, 0, initialSubject,
			    ((accept && send_ics) || (mgr == marie.getCMgr())) ? description : NULL, 0,
			    LinphoneConferenceInfoStateNew, (accept) ? security_level : LinphoneConferenceSecurityLevelNone, FALSE,
			    TRUE, TRUE, enable_chat);
			bctbx_list_free_with_data(participants_info3, (bctbx_list_free_func)linphone_participant_info_unref);
		}

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
			return false;
		});

		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

void create_conference_with_audio_only_participants_base(LinphoneConferenceSecurityLevel security_level) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		bool_t enable_lime = (security_level == LinphoneConferenceSecurityLevelEndToEnd ? TRUE : FALSE);

		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), enable_lime);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), enable_lime);
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress(), enable_lime);
		ClientConference berthe("berthe_rc", focus.getConferenceFactoryAddress(), enable_lime);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(berthe);

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t *coresList = NULL;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), berthe.getCMgr()}) {
			LinphoneVideoActivationPolicy *pol =
			    linphone_factory_create_video_activation_policy(linphone_factory_get());
			linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
			linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
			linphone_core_set_video_activation_policy(mgr->lc, pol);
			linphone_video_activation_policy_unref(pol);

			linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);
			linphone_core_enable_video_capture(mgr->lc, TRUE);
			linphone_core_enable_video_display(mgr->lc, TRUE);

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);

		stats focus_stat = focus.getStats();

		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr()};
		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(),
		                                                laure.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()};

		if (enable_lime) {
			configure_end_to_end_encrypted_conference_server(focus);
		}

		time_t start_time = ms_time(NULL) + 5;
		int duration = 1;
		time_t end_time = (start_time + duration * 60);
		const char *initialSubject = "Test characters: ^ :) ¤ çà @";
		const char *description = "Chamrousse Pub";

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		LinphoneParticipantRole role = LinphoneParticipantRoleUnknown;
		for (auto &p : participants) {
			participantList.insert(
			    std::make_pair(p, add_participant_info_to_list(&participants_info, p->identity, role, -1)));
			role = (role == LinphoneParticipantRoleListener) ? LinphoneParticipantRoleSpeaker
			                                                 : LinphoneParticipantRoleListener;
		}
		participantList.insert(std::make_pair(
		    berthe.getCMgr(), add_participant_info_to_list(&participants_info, berthe.getCMgr()->identity,
		                                                   LinphoneParticipantRoleSpeaker, -1)));

		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, security_level, TRUE, FALSE, NULL);

		BC_ASSERT_PTR_NOT_NULL(confAddr);
		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:");

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 2,
		                             liblinphone_tester_sip_timeout));

		coresList = bctbx_list_remove(coresList, focus.getLc());
		// Restart flexisip
		focus.reStart();

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(5), [] { return false; });

		LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
		linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
		linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
		linphone_core_set_video_activation_policy(focus.getLc(), pol);
		linphone_video_activation_policy_unref(pol);

		linphone_core_enable_video_capture(focus.getLc(), TRUE);
		linphone_core_enable_video_display(focus.getLc(), TRUE);

		coresList = bctbx_list_append(coresList, focus.getLc());
		LinphoneConferenceLayout layout = LinphoneConferenceLayoutGrid;
		for (auto mgr : members) {
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_call_params_enable_video(new_params, FALSE);
			linphone_call_params_set_conference_video_layout(new_params, layout);
			ms_message("%s is calling conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);

			if (layout == LinphoneConferenceLayoutGrid) {
				layout = LinphoneConferenceLayoutActiveSpeaker;
			} else if (layout == LinphoneConferenceLayoutActiveSpeaker) {
				layout = LinphoneConferenceLayoutGrid;
			}
		}

		int nb_subscriptions = 1;
		if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
			nb_subscriptions = 2; // One more subscription for the EKT
		}

		for (auto mgr : members) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1,
			                             liblinphone_tester_sip_timeout));
			int no_streams_running = 2;
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating, (no_streams_running - 1),
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, no_streams_running,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneChatRoomStateCreated,
			                             ((mgr == marie.getCMgr()) ? 3 : 1), liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress,
			                             nb_subscriptions, 5000));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, nb_subscriptions, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));
			if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyEktReceived, 1,
				                             liblinphone_tester_sip_timeout));
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + 3,
		                             liblinphone_tester_sip_timeout));
		int focus_no_streams_running = 6;
		// Update to end ICE negotiations
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running - 3),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
		                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + (3 * nb_subscriptions),
		                             5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + (3 * nb_subscriptions), 5000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + 3, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + 3,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
		                             focus_stat.number_of_conference_participant_devices_present + 3,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
		                             focus_stat.number_of_participant_devices_present + 3,
		                             liblinphone_tester_sip_timeout));

		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
		    fill_member_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure, berthe}, conferenceMgrs, focus.getCMgr(), memberList,
		                            confAddr, TRUE);

		LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

		for (auto mgr : conferenceMgrs) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				const LinphoneConferenceParams *conference_params = linphone_conference_get_current_params(pconference);
				int no_participants = 0;
				if (start_time >= 0) {
					BC_ASSERT_EQUAL((long long)linphone_conference_params_get_start_time(conference_params),
					                (long long)start_time, long long, "%lld");
				}
				BC_ASSERT_EQUAL((long long)linphone_conference_params_get_end_time(conference_params),
				                (long long)end_time, long long, "%lld");
				bctbx_list_t *participant_device_list = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(participant_device_list), 3, size_t, "%zu");
				bctbx_list_free_with_data(participant_device_list, (void (*)(void *))linphone_participant_device_unref);

				if (mgr == focus.getCMgr()) {
					no_participants = 3;
					BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
				} else {
					no_participants = 2;
					BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
					LinphoneCall *current_call = linphone_core_get_current_call(mgr->lc);
					BC_ASSERT_PTR_NOT_NULL(current_call);
					if (current_call) {
						BC_ASSERT_EQUAL((int)linphone_call_get_state(current_call),
						                (int)LinphoneCallStateStreamsRunning, int, "%0d");
					}

					size_t no_streams_audio = 0;
					size_t no_streams_video = 0;
					size_t no_active_streams_video = 0;
					size_t no_streams_text = 0;

					LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						no_streams_audio = compute_no_audio_streams(pcall, pconference);
						_linphone_call_check_nb_streams(pcall, no_streams_audio, no_streams_video, no_streams_text);
						_linphone_call_check_nb_active_streams(pcall, no_streams_audio, no_active_streams_video,
						                                       no_streams_text);
						const LinphoneCallParams *call_lparams = linphone_call_get_params(pcall);
						BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_lparams));
						const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(pcall);
						BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_rparams));
						const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
						BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_cparams));
					}
					LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
					BC_ASSERT_PTR_NOT_NULL(ccall);
					if (ccall) {
						_linphone_call_check_nb_streams(ccall, no_streams_audio, no_streams_video, no_streams_text);
						_linphone_call_check_nb_active_streams(ccall, no_streams_audio, no_active_streams_video,
						                                       no_streams_text);
						const LinphoneCallParams *call_lparams = linphone_call_get_params(ccall);
						BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_lparams));
						const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(ccall);
						BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_rparams));
						const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
						BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_cparams));
					}
				}
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int, "%0d");
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
				check_conference_me(pconference, ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())));
				bctbx_list_t *participants = linphone_conference_get_participant_list(pconference);
				for (bctbx_list_t *itp = participants; itp; itp = bctbx_list_next(itp)) {
					LinphoneParticipant *p = (LinphoneParticipant *)bctbx_list_get_data(itp);
					BC_ASSERT_TRUE(
					    linphone_participant_is_admin(p) ==
					    linphone_address_weak_equal(linphone_participant_get_address(p), marie.getCMgr()->identity));
				}
				bctbx_list_free_with_data(participants, (void (*)(void *))linphone_participant_unref);

				if (mgr != focus.getCMgr()) {
					BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, laure, berthe})
					                   .waitUntil(chrono::seconds(10), [&fconference, &pconference] {
						                   return check_conference_ssrc(fconference, pconference);
					                   }));
				}
			}
		}

		focus_stat = focus.getStats();

		LinphoneCallParams *new_params = linphone_core_create_call_params(berthe.getCMgr()->lc, nullptr);
		linphone_call_params_enable_video(new_params, FALSE);
		linphone_call_params_set_conference_video_layout(new_params, LinphoneConferenceLayoutActiveSpeaker);
		ms_message("%s is calling conference %s", linphone_core_get_identity(berthe.getCMgr()->lc),
		           conference_address_str);
		linphone_core_invite_address_with_params_2(berthe.getCMgr()->lc, confAddr, new_params, NULL, nullptr);
		linphone_call_params_unref(new_params);

		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getCMgr()->stat.number_of_LinphoneCallOutgoingProgress, 1,
		                             liblinphone_tester_sip_timeout));
		int no_streams_running = 2;
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getCMgr()->stat.number_of_LinphoneCallUpdating,
		                             (no_streams_running - 1), liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getCMgr()->stat.number_of_LinphoneCallStreamsRunning,
		                             no_streams_running, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getCMgr()->stat.number_of_LinphoneConferenceStateCreated, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getCMgr()->stat.number_of_LinphoneChatRoomStateCreated, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &berthe.getCMgr()->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getCMgr()->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getCMgr()->stat.number_of_NotifyFullStateReceived, 1,
		                             liblinphone_tester_sip_timeout));
		if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getCMgr()->stat.number_of_NotifyEktReceived, 1,
			                             liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + 1,
		                             liblinphone_tester_sip_timeout));
		focus_no_streams_running = 2;
		// Update to end ICE negotiations
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running - 1),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + nb_subscriptions,
		                             5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + nb_subscriptions, 5000));

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

		participants.push_back(berthe.getCMgr());
		members.push_back(berthe.getCMgr());
		conferenceMgrs.push_back(berthe.getCMgr());

		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList2 =
		    fill_member_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure, berthe}, conferenceMgrs, focus.getCMgr(),
		                            memberList2, confAddr, TRUE);

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

		for (auto mgr : conferenceMgrs) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				const LinphoneConferenceParams *conference_params = linphone_conference_get_current_params(pconference);
				int no_participants = 0;
				if (start_time >= 0) {
					BC_ASSERT_EQUAL((long long)linphone_conference_params_get_start_time(conference_params),
					                (long long)start_time, long long, "%lld");
				}
				BC_ASSERT_EQUAL((long long)linphone_conference_params_get_end_time(conference_params),
				                (long long)end_time, long long, "%lld");
				bctbx_list_t *participant_device_list = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(participant_device_list), 4, size_t, "%zu");
				bctbx_list_free_with_data(participant_device_list, (void (*)(void *))linphone_participant_device_unref);

				if (mgr == focus.getCMgr()) {
					no_participants = 4;
					BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
				} else {
					no_participants = 3;
					BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
					LinphoneCall *current_call = linphone_core_get_current_call(mgr->lc);
					BC_ASSERT_PTR_NOT_NULL(current_call);
					if (current_call) {
						BC_ASSERT_EQUAL((int)linphone_call_get_state(current_call),
						                (int)LinphoneCallStateStreamsRunning, int, "%0d");
					}

					size_t no_streams_audio = 0;
					size_t no_streams_video = 0;
					size_t no_active_streams_video = 0;
					size_t no_streams_text = 0;

					LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						no_streams_audio = compute_no_audio_streams(pcall, pconference);
						_linphone_call_check_nb_streams(pcall, no_streams_audio, no_streams_video, no_streams_text);
						_linphone_call_check_nb_active_streams(pcall, no_streams_audio, no_active_streams_video,
						                                       no_streams_text);
						const LinphoneCallParams *call_lparams = linphone_call_get_params(pcall);
						BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_lparams));
						const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(pcall);
						BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_rparams));
						const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
						BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_cparams));
					}
					LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
					BC_ASSERT_PTR_NOT_NULL(ccall);
					if (ccall) {
						_linphone_call_check_nb_streams(ccall, no_streams_audio, no_streams_video, no_streams_text);
						_linphone_call_check_nb_active_streams(ccall, no_streams_audio, no_active_streams_video,
						                                       no_streams_text);
						const LinphoneCallParams *call_lparams = linphone_call_get_params(ccall);
						BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_lparams));
						const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(ccall);
						BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_rparams));
						const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
						BC_ASSERT_FALSE(linphone_call_params_video_enabled(call_cparams));
					}
				}
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int, "%0d");
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
				check_conference_me(pconference, ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())));
				bctbx_list_t *participants = linphone_conference_get_participant_list(pconference);
				for (bctbx_list_t *itp = participants; itp; itp = bctbx_list_next(itp)) {
					LinphoneParticipant *p = (LinphoneParticipant *)bctbx_list_get_data(itp);
					BC_ASSERT_TRUE(
					    linphone_participant_is_admin(p) ==
					    linphone_address_weak_equal(linphone_participant_get_address(p), marie.getCMgr()->identity));
				}
				bctbx_list_free_with_data(participants, (void (*)(void *))linphone_participant_unref);

				if (mgr != focus.getCMgr()) {
					BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, laure, berthe})
					                   .waitUntil(chrono::seconds(10), [&fconference, &pconference] {
						                   return check_conference_ssrc(fconference, pconference);
					                   }));
				}
			}
		}

		// Everybody adds video
		std::list<LinphoneCoreManager *> withVideo{};
		int mCnt = 0;
		for (auto m : {pauline.getCMgr(), marie.getCMgr()}) {
			withVideo.push_back(m);
			int videoDirCnt = 0;
			std::list<LinphoneMediaDirection> videoDirectionSeq;
			if ((mCnt % 2) == 1) {
				videoDirectionSeq.push_back(LinphoneMediaDirectionSendOnly);
				videoDirectionSeq.push_back(LinphoneMediaDirectionRecvOnly);
				videoDirectionSeq.push_back(LinphoneMediaDirectionSendRecv);
			} else {
				videoDirectionSeq.push_back(LinphoneMediaDirectionSendRecv);
				videoDirectionSeq.push_back(LinphoneMediaDirectionSendOnly);
				videoDirectionSeq.push_back(LinphoneMediaDirectionRecvOnly);
			}
			for (auto video_direction : videoDirectionSeq) {
				std::list<bool_t> enableSeq{TRUE, FALSE};
				if ((videoDirCnt % 2) == 1) {
					enableSeq.push_back(TRUE);
				}
				for (auto enable : enableSeq) {
					set_video_settings_in_conference(focus.getCMgr(), m, members, confAddr, enable, video_direction,
					                                 enable, video_direction);
					for (auto mgr : members) {
						LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
						LinphoneConference *pconference =
						    linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
						linphone_address_unref(uri);
						BC_ASSERT_PTR_NOT_NULL(pconference);
						if (pconference) {
							size_t no_streams_audio = 0;
							size_t no_active_streams_video = 0;
							size_t no_streams_text = 0;

							LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
							BC_ASSERT_PTR_NOT_NULL(pcall);
							if (pcall) {
								no_streams_audio = compute_no_audio_streams(pcall, pconference);
								no_active_streams_video =
								    static_cast<int>(compute_no_video_streams(enable, pcall, pconference));
								_linphone_call_check_nb_active_streams(pcall, no_streams_audio, no_active_streams_video,
								                                       no_streams_text);
							}
							LinphoneCall *ccall =
							    linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
							BC_ASSERT_PTR_NOT_NULL(ccall);
							if (ccall) {
								_linphone_call_check_nb_active_streams(ccall, no_streams_audio, no_active_streams_video,
								                                       no_streams_text);
							}
						}
					}
				}
				videoDirCnt++;
			}
			mCnt++;
		}

		const int total_marie_calls =
		    marie.getStats().number_of_LinphoneCallEnd + (int)bctbx_list_size(linphone_core_get_calls(marie.getLc()));
		const int total_focus_calls =
		    focus.getStats().number_of_LinphoneCallEnd + (int)bctbx_list_size(linphone_core_get_calls(focus.getLc()));
		const int total_pauline_calls = pauline.getStats().number_of_LinphoneCallEnd +
		                                (int)bctbx_list_size(linphone_core_get_calls(pauline.getLc()));
		const int total_laure_calls =
		    laure.getStats().number_of_LinphoneCallEnd + (int)bctbx_list_size(linphone_core_get_calls(laure.getLc()));
		const int total_berthe_calls =
		    berthe.getStats().number_of_LinphoneCallEnd + (int)bctbx_list_size(linphone_core_get_calls(berthe.getLc()));

		linphone_core_terminate_all_calls(pauline.getLc());
		linphone_core_terminate_all_calls(laure.getLc());
		linphone_core_terminate_all_calls(marie.getLc());
		linphone_core_terminate_all_calls(berthe.getLc());

		// Wait for calls to be terminated
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallEnd, total_marie_calls, 30000));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallEnd, total_pauline_calls, 30000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallEnd, total_laure_calls, 30000));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallEnd, total_berthe_calls, 30000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd, total_focus_calls, 40000));

		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallReleased, total_marie_calls, 30000));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallReleased, total_pauline_calls, 30000));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallReleased, total_laure_calls, 30000));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallReleased, total_berthe_calls, 30000));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased, total_focus_calls, 40000));

		check_delete_focus_conference_info({focus, marie, pauline, laure, berthe}, conferenceMgrs, focus.getCMgr(),
		                                   confAddr, end_time);

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), berthe.getCMgr()}) {

			// Wait for all conferences to be terminated
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated,
			                             (mgr == focus.getCMgr()) ? 4 : 1, liblinphone_tester_sip_timeout));

			if (mgr && (mgr != focus.getCMgr())) {
				LinphoneCall *participant_call =
				    linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
				BC_ASSERT_PTR_NULL(participant_call);
				LinphoneCall *conference_call = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
				BC_ASSERT_PTR_NULL(conference_call);

				const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
				BC_ASSERT_EQUAL(bctbx_list_size(call_logs), 1, size_t, "%zu");

				bctbx_list_t *mgr_focus_call_log =
				    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
				BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
				if (mgr_focus_call_log) {
					BC_ASSERT_EQUAL(bctbx_list_size(mgr_focus_call_log), 1, size_t, "%zu");
					for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
						LinphoneCallLog *call_log = (LinphoneCallLog *)it->data;
						BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
					}
					bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
				}
			}
		}

		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		ms_free(conference_address_str);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

void create_simple_conference_dial_out_with_some_calls_declined_base(LinphoneReason reason,
                                                                     LinphoneConferenceSecurityLevel securityLevel) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		bool_t enable_lime = (securityLevel == LinphoneConferenceSecurityLevelEndToEnd ? TRUE : FALSE);

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

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, LinphoneConferenceLayoutActiveSpeaker);
			}

			enable_stun_in_mgr(mgr, TRUE, TRUE, FALSE, TRUE);

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		if (securityLevel == LinphoneConferenceSecurityLevelEndToEnd) {
			configure_end_to_end_encrypted_conference_server(focus);

			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(berthe.getLc()));
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), LinphoneConferenceParticipantListTypeOpen);

		stats focus_stat = focus.getStats();
		stats marie_stat = marie.getStats();

		std::list<LinphoneCoreManager *> active_participants{pauline.getCMgr(), michelle.getCMgr()};
		std::list<LinphoneCoreManager *> declining_participants{laure.getCMgr(), berthe.getCMgr()};
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
		LinphoneConferenceSecurityLevel security_level = securityLevel;

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		LinphoneParticipantRole role = LinphoneParticipantRoleSpeaker;
		for (auto &p : participants) {
			participantList.insert(
			    std::make_pair(p, add_participant_info_to_list(&participants_info, p->identity, role, -1)));
			role = (role == LinphoneParticipantRoleSpeaker) ? LinphoneParticipantRoleListener
			                                                : LinphoneParticipantRoleSpeaker;
		}
		LinphoneAddress *confAddr = create_conference_on_server(focus, marie, participantList, -1, -1, initialSubject,
		                                                        description, FALSE, security_level, TRUE, FALSE, NULL);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		int nb_subscriptions = 1;
		if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
			nb_subscriptions = 2; // One more subscription for the EKT
		}

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallOutgoingInit,
		                             marie_stat.number_of_LinphoneCallOutgoingInit + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingInit,
		                             focus_stat.number_of_LinphoneCallOutgoingInit + 4,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                             focus_stat.number_of_LinphoneCallIncomingReceived + 1,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : participants) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallIncomingReceived, 1,
			                             liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
		                             marie_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating,
		                             marie_stat.number_of_LinphoneCallUpdating + 1, 20000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
		                             marie_stat.number_of_LinphoneCallStreamsRunning + 2,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated,
		                             marie_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionOutgoingProgress,
		                             marie_stat.number_of_LinphoneSubscriptionOutgoingProgress + nb_subscriptions,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
		                             marie_stat.number_of_LinphoneSubscriptionActive + nb_subscriptions,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyFullStateReceived,
		                             marie_stat.number_of_NotifyFullStateReceived + 1, liblinphone_tester_sip_timeout));
		if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyEktReceived,
			                             marie_stat.number_of_NotifyEktReceived + 1, liblinphone_tester_sip_timeout));
		}

		LinphoneConference *oconference = linphone_core_search_conference_2(marie.getLc(), confAddr);
		if (BC_ASSERT_PTR_NOT_NULL(oconference)) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(oconference), 4, int, "%0d");
			bctbx_list_t *devices = linphone_conference_get_participant_device_list(oconference);
			BC_ASSERT_EQUAL(bctbx_list_size(devices), 5, size_t, "%zu");
			if (devices) {
				bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
			}
		}

		if (confAddr) {
			add_participant_info_to_list(&participants_info, marie.getCMgr()->identity, LinphoneParticipantRoleSpeaker,
			                             -1);
			for (auto mgr : participants) {
				bctbx_list_t *participants_info2 = NULL;
				for (bctbx_list_t *info_it = participants_info; info_it; info_it = bctbx_list_next(info_it)) {
					LinphoneParticipantInfo *participant_info_el =
					    (LinphoneParticipantInfo *)bctbx_list_get_data(info_it);
					LinphoneParticipantRole role = LinphoneParticipantRoleUnknown;
					const LinphoneAddress *address = linphone_participant_info_get_address(participant_info_el);
					add_participant_info_to_list(&participants_info2, address, role, -1);
				}

				check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info2, 0, 0,
				                            initialSubject, NULL, 0, LinphoneConferenceInfoStateNew,
				                            (mgr == marie.getCMgr()) ? security_level
				                                                     : LinphoneConferenceSecurityLevelNone,
				                            FALSE, TRUE, TRUE, FALSE);

				LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(pcall);
				if (pcall) {
					LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
					if (std::find(active_participants.cbegin(), active_participants.cend(), mgr) !=
					    active_participants.cend()) {
						linphone_call_accept(pcall);
					} else {
						linphone_call_decline(pcall, reason);
					}
				}

				bctbx_list_free_with_data(participants_info2, (bctbx_list_free_func)linphone_participant_info_unref);
			}
		}

		for (auto mgr : active_participants) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 2,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress,
			                             nb_subscriptions, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, nb_subscriptions,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));
			if (security_level == LinphoneConferenceSecurityLevelEndToEnd) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyEktReceived, 1,
				                             liblinphone_tester_sip_timeout));
			}

			check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info, 0, 0,
			                            initialSubject, NULL, 0, LinphoneConferenceInfoStateNew, security_level, TRUE,
			                            TRUE, TRUE, FALSE);

			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
				BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				LinphoneConferenceInfo *call_log_info = linphone_call_log_get_conference_info(call_log);
				if (BC_ASSERT_PTR_NOT_NULL(call_log_info)) {
					check_conference_info_against_db(mgr, confAddr, call_log_info, FALSE);
				}
			}
		}

		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdating,
		                  focus_stat.number_of_LinphoneCallUpdating + static_cast<int>(active_participants.size()),
		                  liblinphone_tester_sip_timeout) +
		    1);

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning +
		                                 2 * static_cast<int>(active_participants.size() + 1),
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
		                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived +
		                                 (static_cast<int>(active_participants.size()) + 1) * nb_subscriptions,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive +
		                                 (static_cast<int>(active_participants.size()) + 1) * nb_subscriptions,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + 5, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + 5,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
		                             focus_stat.number_of_conference_participant_devices_present +
		                                 static_cast<int>(all_active_participants.size()),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
		                             focus_stat.number_of_participant_devices_present +
		                                 static_cast<int>(all_active_participants.size()),
		                             liblinphone_tester_sip_timeout));

		// Participants that declined the call
		for (auto mgr : declining_participants) {
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1, liblinphone_tester_sip_timeout));
		}

		if (reason == LinphoneReasonBusy) {
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallError,
			                  focus_stat.number_of_LinphoneCallError + static_cast<int>(declining_participants.size()),
			                  liblinphone_tester_sip_timeout));
			BC_ASSERT_FALSE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed,
			                              marie_stat.number_of_participants_removed + 1, 3000));
			BC_ASSERT_FALSE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
			                              focus_stat.number_of_participants_removed + 1, 1000));
		} else {
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
			                  focus_stat.number_of_LinphoneCallEnd + static_cast<int>(declining_participants.size()),
			                  liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed,
			                             marie_stat.number_of_participants_removed + 2,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
			                             focus_stat.number_of_participants_removed + 2,
			                             liblinphone_tester_sip_timeout));
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_removed,
		                             marie_stat.number_of_participant_devices_removed + 2,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed + 2,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                  focus_stat.number_of_LinphoneCallReleased + static_cast<int>(declining_participants.size()),
		                  liblinphone_tester_sip_timeout));

		LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
		    fill_member_list(all_active_participants, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conference_members,
		                            focus.getCMgr(), memberList, confAddr, TRUE);

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(15), [] {
			return false;
		});

		for (auto mgr : conference_members) {
			LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
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
					LinphoneCall *current_call = linphone_core_get_current_call(mgr->lc);
					BC_ASSERT_PTR_NOT_NULL(current_call);
					if (current_call) {
						BC_ASSERT_EQUAL((int)linphone_call_get_state(current_call),
						                (int)LinphoneCallStateStreamsRunning, int, "%0d");
					}
					BC_ASSERT_TRUE(check_ice(mgr, focus.getCMgr(), LinphoneIceStateHostConnection));

					const LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(mgr->lc);
					bool_t enabled = !!linphone_video_activation_policy_get_automatically_initiate(pol);

					size_t no_streams_audio = 0;
					size_t no_max_streams_audio = (security_level == LinphoneConferenceSecurityLevelEndToEnd)
					                                  ? bctbx_list_size(participants_info)
					                                  : 1;
					size_t no_max_streams_video = (enabled || (mgr == marie.getCMgr()))
					                                  ? ((security_level == LinphoneConferenceSecurityLevelEndToEnd)
					                                         ? 2 * (participants.size() + 1)
					                                         : (participants.size() + 2))
					                                  : 1;
					size_t no_streams_video = 0;
					size_t no_streams_text = 0;

					LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						no_streams_audio = compute_no_audio_streams(pcall, pconference);
						no_streams_video = compute_no_video_streams(enabled, pcall, pconference);
						_linphone_call_check_max_nb_streams(pcall, no_max_streams_audio, no_max_streams_video,
						                                    no_streams_text);
						_linphone_call_check_nb_active_streams(pcall, no_streams_audio, no_streams_video,
						                                       no_streams_text);
						const LinphoneCallParams *call_lparams = linphone_call_get_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
						const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
						const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pcall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
					}

					LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
					BC_ASSERT_PTR_NOT_NULL(ccall);
					if (ccall) {
						_linphone_call_check_max_nb_streams(ccall, no_max_streams_audio, no_max_streams_video,
						                                    no_streams_text);
						_linphone_call_check_nb_active_streams(ccall, no_streams_audio, no_streams_video,
						                                       no_streams_text);
						const LinphoneCallParams *call_lparams = linphone_call_get_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_lparams), enabled, int, "%0d");
						const LinphoneCallParams *call_rparams = linphone_call_get_remote_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), enabled, int, "%0d");
						const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), enabled, int, "%0d");
					}
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
				check_conference_me(pconference, ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())));
				bctbx_list_t *participants_list = linphone_conference_get_participant_list(pconference);
				if (reason == LinphoneReasonBusy) {
					no_participants += declining_participants.size();
				}
				BC_ASSERT_EQUAL(bctbx_list_size(participants_list), no_participants, size_t, "%zu");
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference),
				                static_cast<int>(no_participants), int, "%0d");
				for (bctbx_list_t *itp = participants_list; itp; itp = bctbx_list_next(itp)) {
					LinphoneParticipant *p = (LinphoneParticipant *)bctbx_list_get_data(itp);
					BC_ASSERT_TRUE(
					    linphone_participant_is_admin(p) ==
					    linphone_address_weak_equal(linphone_participant_get_address(p), marie.getCMgr()->identity));
				}
				bctbx_list_free_with_data(participants_list, (void (*)(void *))linphone_participant_unref);

				if (mgr != focus.getCMgr()) {
					BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
					                   .waitUntil(chrono::seconds(10), [&fconference, &pconference] {
						                   return check_conference_ssrc(fconference, pconference);
					                   }));
				}

				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), all_active_participants.size(), size_t, "%zu");
				for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					bool video_available =
					    !!linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo);
					LinphoneMediaDirection video_direction =
					    linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo);
					BC_ASSERT_TRUE(video_available == (((video_direction == LinphoneMediaDirectionSendOnly) ||
					                                    (video_direction == LinphoneMediaDirectionSendRecv))));
				}

				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}

				if (mgr != focus.getCMgr()) {
					check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info, 0, 0,
					                            initialSubject, NULL, 0, LinphoneConferenceInfoStateNew, security_level,
					                            TRUE, TRUE, TRUE, FALSE);

					LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
						BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
						LinphoneConferenceInfo *call_log_info = linphone_call_log_get_conference_info(call_log);
						if (BC_ASSERT_PTR_NOT_NULL(call_log_info)) {
							check_conference_info_against_db(mgr, confAddr, call_log_info, FALSE);
						}
					}
				}
			}
		}

		LinphoneCall *pauline_call = linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(pauline_call);

		Address paulineAddr = pauline.getIdentity();
		LinphoneCall *focus_call = linphone_core_get_call_by_remote_address2(focus.getLc(), paulineAddr.toC());
		BC_ASSERT_PTR_NOT_NULL(focus_call);

		const LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(pauline.getLc());
		bool_t enable = !!!linphone_video_activation_policy_get_automatically_initiate(pol);

		LinphoneAddress *paulineUri = linphone_address_new(linphone_core_get_identity(pauline.getLc()));
		LinphoneConference *paulineConference =
		    linphone_core_search_conference(pauline.getLc(), NULL, paulineUri, confAddr, NULL);
		linphone_address_unref(paulineUri);
		BC_ASSERT_PTR_NOT_NULL(paulineConference);

		for (int i = 0; i < 4; i++) {
			set_video_settings_in_conference(focus.getCMgr(), pauline.getCMgr(), all_active_participants, confAddr,
			                                 enable, LinphoneMediaDirectionSendRecv, enable,
			                                 LinphoneMediaDirectionSendRecv);

			if (paulineConference) {
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(paulineConference);
				for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);

					CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
					    .waitUntil(chrono::seconds(10), [enable, paulineConference, d] {
						    bool stream_ok = false;
						    if (enable) {
							    if (linphone_conference_is_me(paulineConference,
							                                  linphone_participant_device_get_address(d))) {
								    stream_ok =
								        linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo);
							    } else {
								    stream_ok = (linphone_participant_device_get_stream_availability(
								                     d, LinphoneStreamTypeVideo) ==
								                 (linphone_participant_device_get_stream_capability(
								                      d, LinphoneStreamTypeVideo) == LinphoneMediaDirectionSendRecv));
							    }
						    } else {
							    stream_ok = (linphone_participant_device_get_stream_availability(
							                     d, LinphoneStreamTypeVideo) == enable);
						    }
						    return stream_ok;
					    });
				}

				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
			}
			// Wait a little bit
			wait_for_list(coresList, NULL, 0, 1000);

			enable = !enable;
		}

		focus_stat = focus.getStats();
		for (auto mgr : all_active_participants) {
			LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
			BC_ASSERT_PTR_NOT_NULL(call);
			if (call) {
				ms_message("%s is terminating call with %s", linphone_core_get_identity(mgr->lc),
				           linphone_core_get_identity(focus.getLc()));
				linphone_call_terminate(call);
				BC_ASSERT_TRUE(
				    wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated,
				                             nb_subscriptions, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending,
				                             1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1,
				                             liblinphone_tester_sip_timeout));

				LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
				LinphoneConference *pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
				BC_ASSERT_PTR_NULL(pconference);
				linphone_address_unref(uri);
			}
		}

		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
		                  focus_stat.number_of_LinphoneCallEnd + static_cast<int>(all_active_participants.size()),
		                  liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                  focus_stat.number_of_LinphoneCallReleased + static_cast<int>(all_active_participants.size()),
		                  liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
		                             focus_stat.number_of_LinphoneSubscriptionTerminated +
		                                 (static_cast<int>(all_active_participants.size())) * nb_subscriptions,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
		                  focus_stat.number_of_participants_removed + static_cast<int>(all_active_participants.size()),
		                  liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed +
		                                 static_cast<int>(all_active_participants.size()),
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_EQUAL(focus.getStats().number_of_conference_participant_devices_present,
		                static_cast<int>(all_active_participants.size()), int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_participant_devices_present,
		                static_cast<int>(all_active_participants.size()), int, "%d");
		BC_ASSERT_EQUAL(marie.getStats().number_of_conference_participant_devices_present,
		                static_cast<int>(all_active_participants.size()), int, "%d");
		BC_ASSERT_EQUAL(marie.getStats().number_of_participant_devices_present,
		                static_cast<int>(all_active_participants.size()), int, "%d");

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

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(), berthe.getCMgr()}) {
			const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
			BC_ASSERT_EQUAL(bctbx_list_size(call_logs), 1, size_t, "%zu");

			bctbx_list_t *mgr_focus_call_log =
			    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
			if (mgr_focus_call_log) {
				BC_ASSERT_EQUAL(bctbx_list_size(mgr_focus_call_log), 1, size_t, "%zu");
				for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
					LinphoneCallLog *call_log = (LinphoneCallLog *)it->data;
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
					LinphoneConferenceInfo *call_log_info = linphone_call_log_get_conference_info(call_log);
					if (BC_ASSERT_PTR_NOT_NULL(call_log_info)) {
						check_conference_info_against_db(mgr, confAddr, call_log_info, FALSE);
					}
				}
				bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
			}

			bool mgr_declining_call = false;
			for (auto declining_mgr : declining_participants) {
				mgr_declining_call |= !!linphone_address_weak_equal(mgr->identity, declining_mgr->identity);
			}

			bctbx_list_t *participants_info2 = NULL;
			for (bctbx_list_t *info_it = participants_info; info_it; info_it = bctbx_list_next(info_it)) {
				LinphoneParticipantInfo *participant_info_el = (LinphoneParticipantInfo *)bctbx_list_get_data(info_it);
				const LinphoneAddress *address = linphone_participant_info_get_address(participant_info_el);
				bool declining_call = false;
				for (auto declining_mgr : declining_participants) {
					declining_call |= !!linphone_address_weak_equal(address, declining_mgr->identity);
				}

				LinphoneParticipantRole role = LinphoneParticipantRoleUnknown;
				int sequence = -1;
				if (mgr_declining_call || (declining_call && (mgr != marie.getCMgr()))) {
					role = LinphoneParticipantRoleUnknown;
					sequence = -1;
				} else {
					role = linphone_participant_info_get_role(participant_info_el);
					sequence = linphone_participant_info_get_sequence_number(participant_info_el);
				}
				add_participant_info_to_list(&participants_info2, address, role, sequence);
			}
			check_conference_info_in_db(
			    mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info2, 0, 0, initialSubject,
			    (mgr == marie.getCMgr()) ? description : NULL, 0, LinphoneConferenceInfoStateNew,
			    (mgr == laure.getCMgr() || mgr == berthe.getCMgr()) ? LinphoneConferenceSecurityLevelNone
			                                                        : security_level,
			    !((mgr == marie.getCMgr()) || mgr_declining_call), TRUE, TRUE, FALSE);

			bctbx_list_free_with_data(participants_info2, (bctbx_list_free_func)linphone_participant_info_unref);
		}

		// wait a bit longer to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
			return false;
		});

		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

void change_active_speaker_base(bool transfer_mode) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress()); // audio only
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress());

		focus.registerAsParticipantDevice(marie);
		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t *coresList = NULL;
		std::list<LinphoneCoreManager *> invitesList{pauline.getCMgr(), laure.getCMgr()};
		std::list<LinphoneCoreManager *> members{pauline.getCMgr(), laure.getCMgr(), marie.getCMgr()};
		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(),
		                                                laure.getCMgr()};
		for (LinphoneCoreManager *mgr : conferenceMgrs) {
			if (mgr != pauline.getCMgr()) {
				LinphoneVideoActivationPolicy *pol =
				    linphone_factory_create_video_activation_policy(linphone_factory_get());
				linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
				linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
				linphone_core_set_video_activation_policy(mgr->lc, pol);
				linphone_video_activation_policy_unref(pol);
			}

			disable_all_video_codecs_except_one(mgr->lc, "VP8");

			linphone_core_set_video_device(mgr->lc, liblinphone_tester_mire_id);
			linphone_core_enable_video_capture(mgr->lc, TRUE);
			linphone_core_enable_video_display(mgr->lc, TRUE);
			linphone_core_set_video_display_filter(mgr->lc, "MSAnalyseDisplay");

			if (mgr != focus.getCMgr()) {
				linphone_core_set_default_conference_layout(mgr->lc, LinphoneConferenceLayoutActiveSpeaker);
			}

			enable_stun_in_mgr(mgr, TRUE, FALSE, TRUE, FALSE);

			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		if (transfer_mode) {
			configure_end_to_end_encrypted_conference_server(focus);
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		stats focus_stat = focus.getStats();

		time_t start_time = ms_time(NULL);
		time_t end_time = -1;
		const char *initialSubject = "Test group";
		const char *description = "hello";
		LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;

		bctbx_list_t *participants_info = NULL;
		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> participantList;
		LinphoneParticipantRole role = LinphoneParticipantRoleSpeaker;
		for (auto &p : invitesList) {
			participantList.insert(
			    std::make_pair(p, add_participant_info_to_list(&participants_info, p->identity, role, -1)));
		}
		LinphoneAddress *confAddr =
		    create_conference_on_server(focus, marie, participantList, start_time, end_time, initialSubject,
		                                description, TRUE, security_level, TRUE, FALSE, NULL);
		BC_ASSERT_PTR_NOT_NULL(confAddr);
		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("sip:");

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneChatRoomStateCreated, 2,
		                             liblinphone_tester_sip_timeout));

		for (LinphoneCoreManager *mgr : members) {
			ms_message("%s is entering conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
			LinphoneCallParams *new_params = linphone_core_create_call_params(mgr->lc, nullptr);
			linphone_call_params_set_video_direction(new_params, LinphoneMediaDirectionSendRecv);
			if (mgr != pauline.getCMgr()) {
				linphone_call_params_enable_video(new_params, TRUE);
			}
			linphone_core_invite_address_with_params_2(mgr->lc, confAddr, new_params, NULL, nullptr);
			linphone_call_params_unref(new_params);

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallOutgoingProgress, 1,
			                             liblinphone_tester_sip_timeout));
			int nbStreamsRunning = 2;
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdating, (nbStreamsRunning - 1),
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, nbStreamsRunning,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneChatRoomStateCreated,
			                             ((mgr == marie.getCMgr()) ? 2 : 1), liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));
		}

		{
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
			                             focus_stat.number_of_LinphoneCallIncomingReceived + 3,
			                             liblinphone_tester_sip_timeout));
			int focusNbStreamsRunning = 2;
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
			                  focus_stat.number_of_LinphoneCallUpdatedByRemote + 3 * (focusNbStreamsRunning - 1),
			                  liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
			                             focus_stat.number_of_LinphoneCallStreamsRunning + 3 * focusNbStreamsRunning,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
			                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
			                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 3,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
			                             focus_stat.number_of_LinphoneSubscriptionActive + 3,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
			                             focus_stat.number_of_participants_added + 3, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
			                             focus_stat.number_of_participant_devices_added + 3,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_conference_participant_devices_present,
			                             focus_stat.number_of_conference_participant_devices_present + 3,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_present,
			                             focus_stat.number_of_participant_devices_present + 3,
			                             liblinphone_tester_sip_timeout));
		}

		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
		    fill_member_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure}, conferenceMgrs, focus.getCMgr(), memberList,
		                            confAddr, TRUE);

		LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		int nbStreamsAudio = 1;
		int nbStreamsVideo = 0;
		int nbStreamsText = 0;

		for (LinphoneCoreManager *mgr : conferenceMgrs) {
			nbStreamsVideo = ((mgr != pauline.getCMgr()) ? 3 : 0);
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				const LinphoneConferenceParams *conference_params = linphone_conference_get_current_params(pconference);
				int nbParticipants = 0;
				if (start_time >= 0) {
					BC_ASSERT_EQUAL((long long)linphone_conference_params_get_start_time(conference_params),
					                (long long)start_time, long long, "%lld");
				}
				BC_ASSERT_EQUAL((long long)linphone_conference_params_get_end_time(conference_params),
				                (long long)end_time, long long, "%lld");
				if (mgr == focus.getCMgr()) {
					nbParticipants = 3;
					BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
				} else {
					nbParticipants = 2;
					BC_ASSERT_TRUE(linphone_conference_is_in(pconference));
					LinphoneCall *current_call = linphone_core_get_current_call(mgr->lc);
					BC_ASSERT_PTR_NOT_NULL(current_call);
					if (current_call) {
						BC_ASSERT_EQUAL((int)linphone_call_get_state(current_call),
						                (int)LinphoneCallStateStreamsRunning, int, "%0d");
					}

					LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						_linphone_call_check_max_nb_streams(
						    pcall, nbStreamsAudio, (mgr != pauline.getCMgr() ? nbStreamsVideo : 0), nbStreamsText);
					}
					LinphoneCall *ccall = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
					BC_ASSERT_PTR_NOT_NULL(ccall);
					if (ccall) {
						_linphone_call_check_max_nb_streams(
						    ccall, nbStreamsAudio, (mgr != pauline.getCMgr() ? nbStreamsVideo : 0), nbStreamsText);
					}
				}
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), nbParticipants, int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 3, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
				check_conference_me(pconference, ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())));
				bctbx_list_t *participants = linphone_conference_get_participant_list(pconference);
				for (bctbx_list_t *itp = participants; itp; itp = bctbx_list_next(itp)) {
					LinphoneParticipant *p = (LinphoneParticipant *)bctbx_list_get_data(itp);
					BC_ASSERT_TRUE(
					    linphone_participant_is_admin(p) ==
					    linphone_address_weak_equal(linphone_participant_get_address(p), marie.getCMgr()->identity));
				}
				bctbx_list_free_with_data(participants, (void (*)(void *))linphone_participant_unref);

				if (mgr != focus.getCMgr()) {
					BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, laure})
					                   .waitUntil(chrono::seconds(10), [&fconference, &pconference] {
						                   return check_conference_ssrc(fconference, pconference);
					                   }));
				}
			}
		}

		// Make pauline and then Marie speak so that we are sure that laure is not the active speaker of none of the
		// conference members
		const std::initializer_list<std::reference_wrapper<ClientConference>> speakingClients{pauline, marie};
		for (const ClientConference &speakingClient : speakingClients) {
			for (const auto &mgr : members) {
				linphone_core_enable_mic(mgr->lc, (mgr == speakingClient.getCMgr()));
			}

			for (const auto &mgr : members) {
				// wait for Marie to become the active speaker
				CoreManagerAssert({focus, marie, pauline, laure})
				    .waitUntil(chrono::seconds(20), [&mgr, &speakingClient, &confAddr] {
					    LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
					    LinphoneParticipantDevice *device =
					        linphone_conference_get_active_speaker_participant_device(pconference);
					    return device ? linphone_address_weak_equal(linphone_participant_device_get_address(device),
					                                                speakingClient.getIdentity().toC())
					                  : false;
				    });
			}
		}

		// need time to decode video
		wait_for_list(coresList, NULL, 1, liblinphone_tester_sip_timeout);
		auto speaking_mgr =
		    _linphone_conference_video_change(coresList, marie.getCMgr(), pauline.getCMgr(), laure.getCMgr());
		BC_ASSERT_PTR_NOT_NULL(speaking_mgr);

		if (speaking_mgr) {
			// TODO: Uncomment lines below when C++20 will be supported
			// auto count =
			members.remove_if([&speaking_mgr](const auto &client) { return (client == speaking_mgr); });
			// BC_ASSERT_EQUAL(count, 1, size_t, "%zu");
		}

		// end
		ms_message("Speaking participant (%s) exits conference %s", linphone_core_get_identity(speaking_mgr->lc),
		           conference_address_str);
		stats initial_pauline_stat = pauline.getStats();
		stats initial_laure_stat = laure.getStats();
		stats initial_focus_stat = focus.getStats();
		LinphoneCall *speaking_mgr_call =
		    linphone_core_get_call_by_remote_address2(speaking_mgr->lc, focus.getCMgr()->identity);
		BC_ASSERT_PTR_NOT_NULL(speaking_mgr_call);
		if (speaking_mgr_call) {
			linphone_call_terminate(speaking_mgr_call);
			BC_ASSERT_TRUE(wait_for_list(coresList, &speaking_mgr->stat.number_of_LinphoneCallEnd, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &speaking_mgr->stat.number_of_LinphoneCallReleased, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &speaking_mgr->stat.number_of_LinphoneSubscriptionTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &speaking_mgr->stat.number_of_LinphoneConferenceStateTerminationPending, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &speaking_mgr->stat.number_of_LinphoneConferenceStateTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &speaking_mgr->stat.number_of_LinphoneConferenceStateDeleted, 1,
			                             liblinphone_tester_sip_timeout));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_active_speaker_participant_device_changed,
		                             initial_laure_stat.number_of_active_speaker_participant_device_changed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participants_removed,
		                             initial_laure_stat.number_of_participants_removed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_removed,
		                             initial_laure_stat.number_of_participant_devices_removed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_active_speaker_participant_device_changed,
		                             initial_pauline_stat.number_of_active_speaker_participant_device_changed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_removed,
		                             initial_pauline_stat.number_of_participants_removed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_removed,
		                             initial_pauline_stat.number_of_participant_devices_removed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
		                             initial_focus_stat.number_of_participants_removed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             initial_focus_stat.number_of_participant_devices_removed + 1,
		                             liblinphone_tester_sip_timeout));

		for (LinphoneCoreManager *mgr : members) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				BC_ASSERT_PTR_NULL(linphone_conference_get_active_speaker_participant_device(pconference));
			}
			ms_message("%s exits conference %s", linphone_core_get_identity(mgr->lc), conference_address_str);
			LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
			BC_ASSERT_PTR_NOT_NULL(call);
			if (call) {
				linphone_call_terminate(call);
				BC_ASSERT_TRUE(
				    wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallReleased, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending,
				                             1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1,
				                             liblinphone_tester_sip_timeout));
			}
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : conferenceMgrs) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated, 1,
			                             liblinphone_tester_sip_timeout));

			if (mgr && (mgr != focus.getCMgr())) {
				LinphoneCall *participant_call =
				    linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
				BC_ASSERT_PTR_NULL(participant_call);
				LinphoneCall *conference_call = linphone_core_get_call_by_remote_address2(focus.getLc(), mgr->identity);
				BC_ASSERT_PTR_NULL(conference_call);

				const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
				BC_ASSERT_EQUAL(bctbx_list_size(call_logs), 1, size_t, "%zu");

				bctbx_list_t *mgr_focus_call_log =
				    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
				BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
				if (mgr_focus_call_log) {
					BC_ASSERT_EQUAL(bctbx_list_size(mgr_focus_call_log), 1, size_t, "%zu");
					for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
						LinphoneCallLog *call_log = (LinphoneCallLog *)it->data;
						BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
					}
					bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
				}
			}
		}

		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
		ms_free(conference_address_str);
		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

} // namespace LinphoneTest
