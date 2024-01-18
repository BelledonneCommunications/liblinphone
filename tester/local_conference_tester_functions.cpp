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

#include "local_conference_tester_functions.h"
#include "shared_tester_functions.h"

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

void setup_conference_info_cbs(LinphoneCoreManager *mgr) {
	// Needed to send the ICS
	linphone_core_set_file_transfer_server(mgr->lc, file_transfer_url);
}

namespace LinphoneTest {

void conference_scheduler_state_changed(LinphoneConferenceScheduler *scheduler,
                                        LinphoneConferenceSchedulerState state) {
	stats *stat = get_stats(linphone_conference_scheduler_get_core(scheduler));
	switch (state) {
		case LinphoneConferenceSchedulerStateIdle:
			stat->number_of_ConferenceSchedulerStateIdle++;
			break;
		case LinphoneConferenceSchedulerStateAllocationPending:
			stat->number_of_ConferenceSchedulerStateAllocationPending++;
			break;
		case LinphoneConferenceSchedulerStateReady:
			stat->number_of_ConferenceSchedulerStateReady++;
			break;
		case LinphoneConferenceSchedulerStateError:
			stat->number_of_ConferenceSchedulerStateError++;
			break;
		case LinphoneConferenceSchedulerStateUpdating:
			stat->number_of_ConferenceSchedulerStateUpdating++;
			break;
	}
}

void conference_scheduler_invitations_sent(LinphoneConferenceScheduler *scheduler,
                                           BCTBX_UNUSED(const bctbx_list_t *failed_addresses)) {
	stats *stat = get_stats(linphone_conference_scheduler_get_core(scheduler));
	stat->number_of_ConferenceSchedulerInvitationsSent++;
}

std::map<LinphoneCoreManager *, LinphoneParticipantInfo *>
fill_memmber_list(std::list<LinphoneCoreManager *> members,
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

LinphoneAddress *
create_conference_on_server(Focus &focus,
                            ClientConference &organizer,
                            std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> requested_participants,
                            time_t start_time,
                            time_t end_time,
                            const char *subject,
                            const char *description,
                            bool_t send_ics,
                            LinphoneConferenceSecurityLevel security_level) {
	bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
	coresList = bctbx_list_append(coresList, organizer.getLc());
	std::vector<stats> participant_stats;
	std::map<LinphoneCoreManager *, LinphoneCall *> previous_calls;
	bctbx_list_t *participants_info = NULL;
	std::list<LinphoneCoreManager *> participants;
	const LinphoneConferenceInfo *updated_conf_info = NULL;
	bool focus_organizer_common_payload = have_common_audio_payload(organizer.getCMgr(), focus.getCMgr());
	bool dialout = ((end_time <= 0) && (start_time <= 0));
	bool found_me = false;
	char *conference_address_str = NULL;
	char *uid = NULL;
	LinphoneConferenceInfo *info = NULL;
	for (const auto &[mgr, participant_info] : requested_participants) {
		LinphoneParticipantInfo *info = linphone_participant_info_clone(participant_info);
		participants_info = bctbx_list_append(participants_info, info);
		if (mgr == organizer.getCMgr()) {
			found_me = true;
		} else {
			coresList = bctbx_list_append(coresList, mgr->lc);
			participant_stats.push_back(mgr->stat);
			participants.push_back(mgr);
		}
	}

	int duration = 0;
	if ((end_time >= 0) && (start_time >= 0) && (end_time > start_time)) {
		duration = static_cast<int>((end_time - start_time) / 60); // duration is expected to be set in minutes
	}
	stats organizer_stat = organizer.getStats();
	stats focus_stat = focus.getStats();

	for (auto &mgr : participants) {
		previous_calls[mgr] = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
	}

	// The organizer creates a conference scheduler
	LinphoneConferenceScheduler *conference_scheduler = linphone_core_create_conference_scheduler(organizer.getLc());
	LinphoneConferenceSchedulerCbs *cbs = linphone_factory_create_conference_scheduler_cbs(linphone_factory_get());
	linphone_conference_scheduler_cbs_set_state_changed(cbs, conference_scheduler_state_changed);
	linphone_conference_scheduler_cbs_set_invitations_sent(cbs, conference_scheduler_invitations_sent);
	linphone_conference_scheduler_add_callbacks(conference_scheduler, cbs);
	linphone_conference_scheduler_cbs_unref(cbs);

	LinphoneConferenceInfo *conf_info = linphone_conference_info_new();

	LinphoneAccount *default_account = linphone_core_get_default_account(organizer.getLc());
	LinphoneAddress *organizer_address = default_account
	                                         ? linphone_address_clone(linphone_account_params_get_identity_address(
	                                               linphone_account_get_params(default_account)))
	                                         : linphone_address_clone(organizer.getCMgr()->identity);
	linphone_conference_info_set_organizer(conf_info, organizer_address);
	linphone_conference_info_set_participant_infos(conf_info, participants_info);
	linphone_conference_info_set_duration(conf_info, duration);
	linphone_conference_info_set_date_time(conf_info, start_time);
	linphone_conference_info_set_subject(conf_info, subject);
	linphone_conference_info_set_description(conf_info, description);
	linphone_conference_info_set_security_level(conf_info, security_level);

	linphone_conference_scheduler_set_info(conference_scheduler, conf_info);
	linphone_conference_info_unref(conf_info);

	BC_ASSERT_TRUE(wait_for_list(coresList, &organizer.getStats().number_of_ConferenceSchedulerStateAllocationPending,
	                             organizer_stat.number_of_ConferenceSchedulerStateAllocationPending + 1,
	                             liblinphone_tester_sip_timeout));

	int idx = 0;
	std::list<LinphoneCoreManager *> actual_participants;
	int call_errors_cnt = 0;
	LinphoneAddress *conference_address = NULL;
	if (dialout) {
		if (focus_organizer_common_payload) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &organizer.getStats().number_of_LinphoneCallOutgoingInit,
			                             organizer_stat.number_of_LinphoneCallOutgoingInit + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
			                             focus_stat.number_of_LinphoneCallIncomingReceived + 1,
			                             liblinphone_tester_sip_timeout));
		} else {
			BC_ASSERT_TRUE(wait_for_list(coresList, &organizer.getStats().number_of_ConferenceSchedulerStateError,
			                             organizer_stat.number_of_ConferenceSchedulerStateError + 1,
			                             liblinphone_tester_sip_timeout));

			updated_conf_info = linphone_conference_scheduler_get_info(conference_scheduler);
			goto end;
		}
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingInit,
		                  focus_stat.number_of_LinphoneCallOutgoingInit + static_cast<int>(participants.size()),
		                  liblinphone_tester_sip_timeout));

		int call_ok_cnt = 0;
		// Conference server dials out participants
		for (auto &mgr : participants) {
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
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_alerting,
		                             focus_stat.number_of_participant_devices_alerting + call_ok_cnt,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallError,
		                             focus_stat.number_of_LinphoneCallError + call_errors_cnt,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed + call_errors_cnt,
		                             liblinphone_tester_sip_timeout));

		if (focus_organizer_common_payload) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &organizer.getStats().number_of_participant_devices_removed,
			                             organizer_stat.number_of_participant_devices_removed + call_errors_cnt,
			                             liblinphone_tester_sip_timeout));
		}

	} else {
		actual_participants = participants;
	}

	BC_ASSERT_TRUE(wait_for_list(coresList, &organizer.getStats().number_of_ConferenceSchedulerStateReady,
	                             organizer_stat.number_of_ConferenceSchedulerStateReady + 1,
	                             liblinphone_tester_sip_timeout));

	updated_conf_info = linphone_conference_scheduler_get_info(conference_scheduler);
	conference_address = linphone_address_clone(linphone_conference_info_get_uri(updated_conf_info));

	if (!!send_ics) {
		LinphoneChatRoomParams *chat_room_params = linphone_core_create_default_chat_room_params(organizer.getLc());
		linphone_chat_room_params_set_backend(chat_room_params, LinphoneChatRoomBackendBasic);
		linphone_conference_scheduler_send_invitations(conference_scheduler, chat_room_params);
		linphone_chat_room_params_unref(chat_room_params);
		BC_ASSERT_TRUE(wait_for_list(coresList, &organizer.getStats().number_of_ConferenceSchedulerInvitationsSent,
		                             organizer_stat.number_of_ConferenceSchedulerInvitationsSent + 1,
		                             liblinphone_tester_sip_timeout));
	}

	info = linphone_core_find_conference_information_from_uri(organizer.getLc(), conference_address);
	if (BC_ASSERT_PTR_NOT_NULL(info)) {
		uid = ms_strdup(linphone_conference_info_get_ics_uid(info));
		if (!!send_ics) {
			BC_ASSERT_PTR_NOT_NULL(uid);
			for (auto &p : participants) {
				linphone_conference_info_check_participant(info, p->identity, 0);
			}
		} else {
			BC_ASSERT_PTR_NULL(uid);
		}
		linphone_conference_info_unref(info);
	}

	if (dialout && !send_ics && !found_me) {
		add_participant_info_to_list(&participants_info, organizer.getCMgr()->identity, LinphoneParticipantRoleSpeaker,
		                             0);
	}

	for (bctbx_list_t *info_it = participants_info; info_it; info_it = bctbx_list_next(info_it)) {
		LinphoneParticipantInfo *participant_info_el = (LinphoneParticipantInfo *)bctbx_list_get_data(info_it);
		if (dialout && !send_ics) {
			linphone_participant_info_set_role(participant_info_el, LinphoneParticipantRoleUnknown);
			linphone_participant_info_set_sequence_number(participant_info_el, -1);
		} else {
			linphone_participant_info_set_sequence_number(participant_info_el, 0);
			if (dialout && send_ics &&
			    (linphone_participant_info_get_role(participant_info_el) == LinphoneParticipantRoleUnknown)) {
				linphone_participant_info_set_role(participant_info_el, LinphoneParticipantRoleSpeaker);
			}
		}
	}

	if (!dialout) {
		// This check is not reliable when the conference is dialing participants
		check_conference_info_in_db(organizer.getCMgr(), uid, conference_address, organizer.getCMgr()->identity,
		                            participants_info, start_time, duration, subject, description, 0,
		                            LinphoneConferenceInfoStateNew, security_level, FALSE);
	}

	idx = 0;
	for (auto &mgr : participants) {
		if (!dialout || !previous_calls[mgr]) {
			auto old_stats = participant_stats[idx];
			if (!!send_ics) {
				// chat room in created state
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceived,
				                             old_stats.number_of_LinphoneMessageReceived + 1,
				                             liblinphone_tester_sip_timeout));
				if (!linphone_core_conference_ics_in_message_body_enabled(organizer.getLc())) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneMessageReceivedWithFile,
					                             old_stats.number_of_LinphoneMessageReceivedWithFile + 1,
					                             liblinphone_tester_sip_timeout));
				}

				BC_ASSERT_PTR_NOT_NULL(mgr->stat.last_received_chat_message);
				if (mgr->stat.last_received_chat_message != NULL) {
					const string expected = ContentType::Icalendar.getMediaType();
					BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(mgr->stat.last_received_chat_message),
					                       expected.c_str());
				}

				bctbx_list_t *chat_room_participants = bctbx_list_append(NULL, mgr->identity);
				LinphoneChatRoom *cr = linphone_core_search_chat_room(organizer.getLc(), NULL, organizer_address, NULL,
				                                                      chat_room_participants);
				bctbx_list_free(chat_room_participants);
				BC_ASSERT_PTR_NOT_NULL(cr);
				if (cr) {
					LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(cr);
					linphone_chat_room_unref(cr);
					BC_ASSERT_PTR_NOT_NULL(msg);

					if (msg) {
						const bctbx_list_t *original_contents = linphone_chat_message_get_contents(msg);
						BC_ASSERT_EQUAL((int)bctbx_list_size(original_contents), 1, int, "%d");
						LinphoneContent *original_content = (LinphoneContent *)bctbx_list_get_data(original_contents);
						BC_ASSERT_PTR_NOT_NULL(original_content);

						LinphoneConferenceInfo *conf_info_in_db =
						    linphone_core_find_conference_information_from_uri(mgr->lc, conference_address);
						if (!BC_ASSERT_PTR_NOT_NULL(conf_info_in_db)) {
							goto end;
						}
						check_conference_info_members(conf_info_in_db, uid, conference_address, organizer_address,
						                              participants_info, start_time, duration, subject, description, 0,
						                              LinphoneConferenceInfoStateNew,
						                              LinphoneConferenceSecurityLevelNone, dialout);

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

			if (!!send_ics || dialout) {
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
					    subject, (!!send_ics) ? description : NULL, 0, LinphoneConferenceInfoStateNew,
					    LinphoneConferenceSecurityLevelNone, dialout);
					if (!!send_ics) {
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

	if (conference_address && !dialout) {
		check_conference_info_in_db(organizer.getCMgr(), uid, conference_address, organizer.getCMgr()->identity,
		                            participants_info, start_time, duration, subject, description, 0,
		                            LinphoneConferenceInfoStateNew, security_level, FALSE);
	}

	BC_ASSERT_EQUAL(organizer.getStats().number_of_LinphoneConferenceStateTerminationPending,
	                organizer_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
	BC_ASSERT_EQUAL(organizer.getStats().number_of_LinphoneConferenceStateTerminated,
	                organizer_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
	BC_ASSERT_EQUAL(organizer.getStats().number_of_LinphoneConferenceStateDeleted,
	                organizer_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

	conference_address_str =
	    (conference_address) ? linphone_address_as_string(conference_address) : ms_strdup("<unknown>");
	ms_message("%s is creating conference %s on server %s", linphone_core_get_identity(organizer.getLc()),
	           conference_address_str, linphone_core_get_identity(focus.getLc()));
	ms_free(conference_address_str);

end:
	if (uid) {
		ms_free(uid);
	}

	if (organizer_address) linphone_address_unref(organizer_address);
	linphone_conference_scheduler_unref(conference_scheduler);
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

	if (enable_video && call_video_enabled && (call_video_dir != LinphoneMediaDirectionInactive)) {
		bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
		for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
			LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
			LinphoneMediaDirection dir = linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo);
			const LinphoneAddress *device_address = linphone_participant_device_get_address(d);
			bool_t is_me = is_in_conference
			                   ? linphone_address_weak_equal(device_address, linphone_call_get_remote_address(call))
			                   : linphone_conference_is_me(conference, device_address);
			bool_t dir_has_send_component =
			    ((dir == LinphoneMediaDirectionSendRecv) || (dir == LinphoneMediaDirectionSendOnly));
			LinphoneConferenceLayout call_video_layout = linphone_call_params_get_conference_video_layout(call_params);
			LinphoneConferenceSecurityLevel conference_security_level =
			    linphone_conference_params_get_security_level(linphone_conference_get_current_params(conference));
			if (dir_has_send_component) {
				if (is_me || ((call_video_layout == LinphoneConferenceLayoutActiveSpeaker) &&
				              (conference_security_level == LinphoneConferenceSecurityLevelEndToEnd))) {
					nb_video_streams += 2;
				} else {
					nb_video_streams++;
				}
			} else if (is_me && (call_video_layout == LinphoneConferenceLayoutActiveSpeaker) &&
			           (dir == LinphoneMediaDirectionRecvOnly) &&
			           (conference_security_level != LinphoneConferenceSecurityLevelEndToEnd)) {
				nb_video_streams++;
			}
		}
		bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
	} else {
		nb_video_streams = 0;
	}

	return nb_video_streams;
}

size_t compute_no_audio_streams(LinphoneCall *call, LinphoneConference *conference) {
	size_t nb_audio_streams = 0;
	const LinphoneConferenceParams *conference_params = linphone_conference_get_current_params(conference);
	if (linphone_conference_params_get_security_level(conference_params) == LinphoneConferenceSecurityLevelEndToEnd) {
		const LinphoneAddress *remote_address = linphone_call_get_remote_address(call);
		bctbx_list_t *participants = linphone_conference_get_participant_list(conference);
		for (bctbx_list_t *itp = participants; itp; itp = bctbx_list_next(itp)) {
			LinphoneParticipant *participant = (LinphoneParticipant *)bctbx_list_get_data(itp);
			LinphoneParticipantRole role = linphone_participant_get_role(participant);
			if (role == LinphoneParticipantRoleSpeaker) {
				bctbx_list_t *devices = linphone_participant_get_devices(participant);
				for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					LinphoneParticipantDevice *device = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					if (linphone_participant_device_get_stream_availability(device, LinphoneStreamTypeAudio)) {
						nb_audio_streams++;
					}
				}
				bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
			} else if (linphone_address_weak_equal(linphone_participant_get_address(participant), remote_address)) {
				// Add an audio stream anyway if the participant holding the call is a listener
				nb_audio_streams++;
			}
		}
		bctbx_list_free_with_data(participants, (void (*)(void *))linphone_participant_unref);
		if (!linphone_core_conference_server_enabled(linphone_conference_get_core(conference))) {
			// Add own audio stream if the core holding the conference is not a server
			nb_audio_streams++;
		}
	} else {
		nb_audio_streams = 1;
	}
	return nb_audio_streams;
}

void wait_for_conference_streams(std::initializer_list<std::reference_wrapper<CoreManager>> coreMgrs,
                                 std::list<LinphoneCoreManager *> conferenceMgrs,
                                 LinphoneCoreManager *focus,
                                 std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> members,
                                 const LinphoneAddress *confAddr,
                                 bool_t enable_video) {
	for (auto mgr : conferenceMgrs) {
		// wait bit more to detect side effect if any
		BC_ASSERT_TRUE(CoreManagerAssert(coreMgrs).waitUntil(chrono::seconds(50), [mgr, &focus, &members, confAddr,
		                                                                           enable_video] {
			size_t nb_text_streams = 0;
			std::list<LinphoneCall *> calls;

			bool video_check = false;
			bool participant_check = false;
			bool device_check = false;
			bool call_check = true;
			bool audio_direction_check = true;

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
			LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
			for (auto call : calls) {
				if (call) {
					size_t nb_audio_streams = compute_no_audio_streams(call, conference);
					size_t nb_video_streams = compute_no_video_streams(enable_video, call, conference);
					const SalMediaDescription *call_result_desc = _linphone_call_get_result_desc(call);
					call_check &= ((call_result_desc->nbActiveStreamsOfType(SalAudio) == nb_audio_streams) &&
					               (call_result_desc->nbActiveStreamsOfType(SalVideo) == nb_video_streams) &&
					               (call_result_desc->nbActiveStreamsOfType(SalText) == nb_text_streams));
					;
				}
			}

			if (conference) {
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
				video_check = (bctbx_list_size(devices) > 0);
				participant_check = (bctbx_list_size(devices) > 0);
				device_check = (bctbx_list_size(devices) > 0);
				LinphoneCall *call = NULL;
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
					if (p) {
						LinphoneParticipantRole role = linphone_participant_get_role(p);
						expected_audio_direction =
						    ((role == LinphoneParticipantRoleSpeaker) ? LinphoneMediaDirectionSendRecv
						                                              : LinphoneMediaDirectionRecvOnly);
						LinphoneMediaDirection audio_dir =
						    linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeAudio);
						audio_direction_check &= (audio_dir == expected_audio_direction);
						if (role == LinphoneParticipantRoleListener) {
							LinphoneMediaDirection video_dir =
							    linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo);
							video_check &= ((video_dir == LinphoneMediaDirectionRecvOnly) ||
							                (video_dir == LinphoneMediaDirectionInactive));
						}
					} else {
						audio_direction_check = false;
						video_check = false;
					}

					LinphoneParticipantDeviceState expected_state = LinphoneParticipantDeviceStateJoining;
					if (found) {
						expected_state = LinphoneParticipantDeviceStatePresent;
					} else {
						expected_state = LinphoneParticipantDeviceStateLeft;
					}
					device_check &= (linphone_participant_device_get_state(d) == expected_state);
					if (call) {
						if (is_me) {
							const LinphoneCallParams *call_current_params = linphone_call_get_current_params(call);
							LinphoneMediaDirection call_audio_direction =
							    linphone_call_params_get_audio_direction(call_current_params);
							audio_direction_check &= (call_audio_direction == expected_audio_direction);
						}

						const LinphoneCallParams *call_params = linphone_call_get_params(call);
						bool_t call_video_enabled = linphone_call_params_video_enabled(call_params);
						if (enable_video && ((mgr == focus) || call_video_enabled)) {
							// Video is available if the direction is sendrecv or sendonly
							LinphoneMediaDirection video_dir =
							    linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo);
							video_check &=
							    (linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) ==
							     ((video_dir == LinphoneMediaDirectionSendRecv) ||
							      (video_dir == LinphoneMediaDirectionSendOnly)));
						} else {
							video_check &=
							    !linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo);
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
							participant_check |= !linphone_address_has_uri_param(participant_address, "gr");
						} else {
							participant_check = false;
						}
						participant_check &=
						    (linphone_participant_get_role(p) == linphone_participant_info_get_role(info));
					}
				}
			}
			return audio_direction_check && video_check && device_check && call_check && participant_check;
		}));
	}
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
	LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;

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
				LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(conference);
				if (conference) {
					const LinphoneConferenceParams *conference_params =
					    linphone_conference_get_current_params(conference);
					security_level = linphone_conference_params_get_security_level(conference_params);
				}
			}
		}

		// Increment counter
		counter++;
	}

	if (inactive_call_video) {
		recvonly_video = FALSE;
	}

	// During an encrypted conference or whenever a participant chooses a grid layout the receive and send video streams
	// are split therefore if the other participants have an inactive video or the can only receive it and the chose
	// video direction is RecvOnly, then no video streams will be sent out
	bool_t negotiated_inactive_video =
	    (((call_conference_layout == LinphoneConferenceLayoutGrid) ||
	      (security_level == LinphoneConferenceSecurityLevelEndToEnd)) &&
	     (inactive_call_video || recvonly_video) && (video_direction == LinphoneMediaDirectionRecvOnly));

	// Whenever a participant chooses a grid layout, the participant requestes 2 streams for its send component and the
	// streams with recvonly direction are those of the others participants
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
	bool_t media_changed = ((!previous_enable_video && enable && inactive_video_sdp_sent) ||
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
			BC_ASSERT_EQUAL(mgr->stat.number_of_participant_devices_joined,
			                participants_initial_stats[counter].number_of_participant_devices_joined, int, "%0d");
		}
		if (media_changed) {
			BC_ASSERT_FALSE(wait_for_list(
			    coresList, &mgr->stat.number_of_participant_devices_media_capability_changed,
			    participants_initial_stats[counter].number_of_participant_devices_media_capability_changed + 1,
			    liblinphone_tester_sip_timeout));
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
		BC_ASSERT_EQUAL(mgr->stat.number_of_participant_devices_joined,
		                participants_initial_stats[counter].number_of_participant_devices_joined, int, "%0d");
		counter++;
	}
	BC_ASSERT_TRUE(wait_for_list(coresList, &focus->stat.number_of_LinphoneCallUpdatedByRemote,
	                             focus_stat.number_of_LinphoneCallUpdatedByRemote + no_updates,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &focus->stat.number_of_LinphoneCallStreamsRunning,
	                             focus_stat.number_of_LinphoneCallStreamsRunning + no_updates,
	                             liblinphone_tester_sip_timeout));
	if (media_changed) {
		BC_ASSERT_FALSE(wait_for_list(coresList, &focus->stat.number_of_participant_devices_media_capability_changed,
		                              focus_stat.number_of_participant_devices_media_capability_changed + 1,
		                              liblinphone_tester_sip_timeout));
	} else {
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus->stat.number_of_participant_devices_media_capability_changed,
		                             focus_stat.number_of_participant_devices_media_capability_changed + 1,
		                             liblinphone_tester_sip_timeout));
	}
	BC_ASSERT_EQUAL(focus->stat.number_of_participants_added, focus_stat.number_of_participants_added, int, "%0d");
	BC_ASSERT_EQUAL(focus->stat.number_of_participant_devices_added, focus_stat.number_of_participant_devices_added,
	                int, "%0d");
	BC_ASSERT_EQUAL(focus->stat.number_of_participant_devices_joined, focus_stat.number_of_participant_devices_joined,
	                int, "%0d");

	for (auto mgr : active_cores) {
		LinphoneAddress *uri2 = mgr->identity;
		LinphoneConference *pconference = linphone_core_search_conference(mgr->lc, NULL, uri2, confAddr, NULL);
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
						} else if (((call_conference_layout == LinphoneConferenceLayoutGrid) ||
						            (security_level == LinphoneConferenceSecurityLevelEndToEnd)) &&
						           (inactive_call_video || recvonly_video) &&
						           (video_direction == LinphoneMediaDirectionSendRecv)) {
							// Layout Grid doesn't allow the server to always deduce the right client's video direction
							// because the send and recv parts of the video streams are separate.
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
                            std::list<LinphoneParticipantRole> allowedRoles) {
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

			if (mgr != focus.getCMgr()) {
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

		int nortp_timeout = 10;
		linphone_core_set_nortp_timeout(marie.getLc(), nortp_timeout);
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

		LinphoneAddress *confAddr = create_conference_on_server(focus, marie, participantList, start_time, end_time,
		                                                        initialSubject, description, TRUE, security_level);
		BC_ASSERT_PTR_NOT_NULL(confAddr);
		char *conference_address_str = (confAddr) ? linphone_address_as_string(confAddr) : ms_strdup("<unknown>");

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 2,
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
		}

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
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated,
			                             ((mgr == marie.getCMgr()) ? 3 : 2), liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
			                             liblinphone_tester_sip_timeout));

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
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + (focus_no_streams_running - 3),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + focus_no_streams_running,
		                             liblinphone_tester_sip_timeout));
		// If ICE is enabled, the addition to a conference may go through a resume of the call
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
		                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 3, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + 3, 5000));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + 3, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + 3,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
		                             focus_stat.number_of_participant_devices_joined + 3,
		                             liblinphone_tester_sip_timeout));

		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
		    fill_memmber_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs, focus.getCMgr(),
		                            memberList, confAddr, enable_video);

		LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		// wait to know if the no RTP timeout is triggered
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
		    .waitUntil(chrono::seconds(nortp_timeout + 1), [&marie, confAddr, nortp_timeout] {
			    LinphoneCall *marie_call = linphone_core_get_call_by_remote_address2(marie.getLc(), confAddr);
			    return marie_call && (linphone_call_get_duration(marie_call) > nortp_timeout);
		    });

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

				BC_ASSERT_EQUAL((int)linphone_conference_params_get_security_level(conference_params),
				                (int)security_level, int, "%0d");

				bctbx_list_t *participant_device_list = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(participant_device_list), members.size(), size_t, "%zu");
				for (bctbx_list_t *d_it = participant_device_list; d_it; d_it = bctbx_list_next(d_it)) {
					LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(d_it);
					BC_ASSERT_PTR_NOT_NULL(d);
					if (d) {
						const LinphoneAddress *device_address = linphone_participant_device_get_address(d);
						bool_t expect_mute = (linphone_address_weak_equal(device_address, pauline.getCMgr()->identity));
						BC_ASSERT_TRUE((!!linphone_participant_device_get_is_muted(d)) == expect_mute);
						linphone_participant_device_set_user_data(d, mgr->lc);
						LinphoneParticipantDeviceCbs *cbs =
						    linphone_factory_create_participant_device_cbs(linphone_factory_get());
						linphone_participant_device_cbs_set_is_muted(cbs, on_muted_notified);
						linphone_participant_device_add_callbacks(d, cbs);
						linphone_participant_device_cbs_unref(cbs);
					}
				}
				bctbx_list_free_with_data(participant_device_list, (void (*)(void *))linphone_participant_device_unref);

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

					LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(mgr->lc);
					bool_t enabled = !!linphone_video_activation_policy_get_automatically_initiate(pol);
					linphone_video_activation_policy_unref(pol);

					size_t no_streams_audio = 0;
					size_t no_active_streams_video = 0;
					size_t no_streams_video = (enabled)
					                              ? (((security_level == LinphoneConferenceSecurityLevelEndToEnd) &&
					                                  (layout == LinphoneConferenceLayoutActiveSpeaker))
					                                     ? 6
					                                     : 4)
					                              : 0;
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
							BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_rparams), video_negotiated, int,
							                "%0d");
						}
						const LinphoneCallParams *call_cparams = linphone_call_get_current_params(ccall);
						BC_ASSERT_EQUAL(linphone_call_params_video_enabled(call_cparams), video_negotiated, int, "%0d");
					}
				}
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), no_participants, int, "%0d");
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
				LinphoneParticipant *me = linphone_conference_get_me(pconference);
				BC_ASSERT_TRUE(linphone_participant_is_admin(me) ==
				               ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())));
				BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_participant_get_address(me), mgr->identity));
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

		ms_message("Marie mutes its microphone");
		LinphoneConference *marie_conference = linphone_core_search_conference_2(marie.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(marie_conference);
		if (marie_conference) {
			linphone_conference_set_microphone_muted(marie_conference, TRUE);
		}

		for (auto mgr : conferenceMgrs) {
			if (mgr != marie.getCMgr()) {
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

			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);

			if (pconference) {
				bctbx_list_t *participant_device_list = linphone_conference_get_participant_device_list(pconference);
				for (bctbx_list_t *d_it = participant_device_list; d_it; d_it = bctbx_list_next(d_it)) {
					LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(d_it);
					BC_ASSERT_PTR_NOT_NULL(d);
					if (d) {
						const LinphoneAddress *device_address = linphone_participant_device_get_address(d);
						bool_t expect_mute = (linphone_address_weak_equal(device_address, marie.getCMgr()->identity)) ||
						                     (linphone_address_weak_equal(device_address, pauline.getCMgr()->identity));
						BC_ASSERT_TRUE((!!linphone_participant_device_get_is_muted(d)) == expect_mute);
					}
				}
				bctbx_list_free_with_data(participant_device_list, (void (*)(void *))linphone_participant_device_unref);
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
			                             marie_stat2.number_of_LinphoneSubscriptionOutgoingProgress + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
			                             marie_stat2.number_of_LinphoneSubscriptionActive + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyReceived,
			                             marie_stat2.number_of_NotifyReceived + 1, liblinphone_tester_sip_timeout));

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
			                             focus_stat2.number_of_LinphoneSubscriptionIncomingReceived + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
			                             focus_stat2.number_of_LinphoneSubscriptionActive + 1, 5000));

			// wait bit more to detect side effect if any
			CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
				return false;
			});

			for (auto mgr : conferenceMgrs) {
				LinphoneAddress *uri = mgr->identity;
				LinphoneConference *pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
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

					bctbx_list_t *participant_device_list =
					    linphone_conference_get_participant_device_list(pconference);
					for (bctbx_list_t *d_it = participant_device_list; d_it; d_it = bctbx_list_next(d_it)) {
						LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(d_it);
						BC_ASSERT_PTR_NOT_NULL(d);
						if (d) {
							BC_ASSERT_TRUE((!!linphone_participant_device_get_is_muted(d)) ==
							               (linphone_address_weak_equal(linphone_participant_device_get_address(d),
							                                            pauline.getCMgr()->identity)));
						}
					}
					bctbx_list_free_with_data(participant_device_list,
					                          (void (*)(void *))linphone_participant_device_unref);

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

						LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(mgr->lc);
						bool_t enabled = !!linphone_video_activation_policy_get_automatically_initiate(pol);
						linphone_video_activation_policy_unref(pol);

						size_t no_streams_audio = 0;
						size_t no_active_streams_video = 0;
						size_t no_streams_video = (enabled)
						                              ? (((security_level == LinphoneConferenceSecurityLevelEndToEnd) &&
						                                  (layout == LinphoneConferenceLayoutActiveSpeaker))
						                                     ? 6
						                                     : 4)
						                              : 0;
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
					LinphoneParticipant *me = linphone_conference_get_me(pconference);
					BC_ASSERT_TRUE(linphone_participant_is_admin(me) ==
					               ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())));
					BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_participant_get_address(me), mgr->identity));
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
			                             marie_stat.number_of_LinphoneSubscriptionTerminated + 1,
			                             liblinphone_tester_sip_timeout));

			// Wait a little bit
			wait_for_list(coresList, NULL, 0, 1000);

			linphone_core_set_network_reachable(marie.getLc(), TRUE);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionOutgoingProgress,
			                             marie_stat.number_of_LinphoneSubscriptionOutgoingProgress + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
			                             focus_stat2.number_of_LinphoneSubscriptionIncomingReceived + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
			                             marie_stat.number_of_LinphoneSubscriptionActive + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
			                             focus_stat2.number_of_LinphoneSubscriptionActive + 1,
			                             liblinphone_tester_sip_timeout));
		}

		if (enable_video) {
			LinphoneCall *pauline_call = linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(pauline_call);
			bool_t enable = FALSE;
			if (pauline_call) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pauline_call);
				LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(pauline.getLc());
				enable = !(!!linphone_video_activation_policy_get_automatically_initiate(pol) &&
				           !!linphone_call_params_video_enabled(call_cparams));
				linphone_video_activation_policy_unref(pol);
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
						BC_ASSERT_EQUAL(focus.getStats().number_of_participant_devices_joined,
						                focus_stat2.number_of_participant_devices_joined, int, "%0d");
						BC_ASSERT_EQUAL(marie.getStats().number_of_participants_added,
						                marie_stat2.number_of_participants_added, int, "%0d");
						BC_ASSERT_EQUAL(marie.getStats().number_of_participant_devices_added,
						                marie_stat2.number_of_participant_devices_added, int, "%0d");
						BC_ASSERT_EQUAL(marie.getStats().number_of_participant_devices_joined,
						                marie_stat2.number_of_participant_devices_joined, int, "%0d");
						BC_ASSERT_EQUAL(pauline.getStats().number_of_participants_added,
						                pauline_stat2.number_of_participants_added, int, "%0d");
						BC_ASSERT_EQUAL(pauline.getStats().number_of_participant_devices_added,
						                pauline_stat2.number_of_participant_devices_added, int, "%0d");
						BC_ASSERT_EQUAL(pauline.getStats().number_of_participant_devices_joined,
						                pauline_stat2.number_of_participant_devices_joined, int, "%0d");
						BC_ASSERT_EQUAL(laure.getStats().number_of_participants_added,
						                laure_stat2.number_of_participants_added, int, "%0d");
						BC_ASSERT_EQUAL(laure.getStats().number_of_participant_devices_added,
						                laure_stat2.number_of_participant_devices_added, int, "%0d");
						BC_ASSERT_EQUAL(laure.getStats().number_of_participant_devices_joined,
						                laure_stat2.number_of_participant_devices_joined, int, "%0d");

						for (auto mgr : conferenceMgrs) {
							const LinphoneAddress *uri = mgr->identity;
							LinphoneConference *pconference =
							    linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
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
											} else if ((video_direction == LinphoneMediaDirectionRecvOnly) &&
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

						BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_on_hold,
						                             focus_stat3.number_of_participant_devices_on_hold + 1,
						                             liblinphone_tester_sip_timeout));

						BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_on_hold,
						                             laure_stat3.number_of_participant_devices_on_hold + 1,
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(
						    coresList, &laure.getStats().number_of_participant_devices_media_capability_changed,
						    laure_stat3.number_of_participant_devices_media_capability_changed + 1,
						    liblinphone_tester_sip_timeout));

						BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_on_hold,
						                             marie_stat3.number_of_participant_devices_on_hold + 1,
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(
						    coresList, &marie.getStats().number_of_participant_devices_media_capability_changed,
						    marie_stat3.number_of_participant_devices_media_capability_changed + 1,
						    liblinphone_tester_sip_timeout));

						for (auto mgr : conferenceMgrs) {
							LinphoneConference *pconference =
							    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr, NULL);
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

						BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
						                             focus_stat3.number_of_participant_devices_joined + 1,
						                             liblinphone_tester_sip_timeout));

						BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_joined,
						                             laure_stat3.number_of_participant_devices_joined + 1,
						                             liblinphone_tester_sip_timeout));

						BC_ASSERT_TRUE(wait_for_list(
						    coresList, &laure.getStats().number_of_participant_devices_media_capability_changed,
						    laure_stat3.number_of_participant_devices_media_capability_changed + 2,
						    liblinphone_tester_sip_timeout));

						BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneCallStreamsRunning,
						                             laure_stat3.number_of_LinphoneCallStreamsRunning +
						                                 participant_streams_running,
						                             liblinphone_tester_sip_timeout));

						BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_joined,
						                             marie_stat3.number_of_participant_devices_joined + 1,
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
							LinphoneConference *pconference =
							    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr, NULL);
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
				LinphoneConference *pconference =
				    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr, NULL);
				BC_ASSERT_PTR_NOT_NULL(pconference);
				if (pconference) {
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
			stats marie_stat2 = marie.getStats();
			stats focus_stat2 = focus.getStats();
			stats pauline_stat2 = pauline.getStats();
			stats laure_stat2 = laure.getStats();

			extraParticipantMgrs.push_back(michelle.getCMgr());

			ms_message("%s is entering conference %s", linphone_core_get_identity(michelle.getLc()),
			           conference_address_str);

			LinphoneCallParams *params = linphone_core_create_call_params(michelle.getLc(), nullptr);
			LinphoneCall *michelle_call = linphone_core_invite_address_with_params(michelle.getLc(), confAddr, params);
			BC_ASSERT_PTR_NOT_NULL(michelle_call);
			linphone_call_params_unref(params);
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallOutgoingProgress, 1,
			                             liblinphone_tester_sip_timeout));
			int extra_participants = 0;
			if (participant_list_type == LinphoneConferenceParticipantListTypeOpen) {

				if (network_restart) {
					ms_message("%s switches off network before %s is added to conference %s",
					           linphone_core_get_identity(marie.getLc()), linphone_core_get_identity(michelle.getLc()),
					           conference_address_str);
					linphone_core_set_network_reachable(marie.getLc(), FALSE);
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionTerminated,
					                             marie_stat2.number_of_LinphoneSubscriptionTerminated + 1,
					                             liblinphone_tester_sip_timeout));
				}

				conferenceMgrs.push_back(michelle.getCMgr());
				members.push_back(michelle.getCMgr());
				participantList.insert(std::make_pair(
				    michelle.getCMgr(), add_participant_info_to_list(&participants_info, michelle.getCMgr()->identity,
				                                                     (listenerAllowed) ? LinphoneParticipantRoleListener
				                                                                       : LinphoneParticipantRoleSpeaker,
				                                                     0)));

				extra_participants = 1;

				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallStreamsRunning, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallUpdating, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallStreamsRunning, 2,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateCreated,
				                             1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(
				    coresList, &michelle.getStats().number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
				BC_ASSERT_TRUE(
				    wait_for_list(coresList, &michelle.getStats().number_of_LinphoneSubscriptionActive, 1, 5000));
				BC_ASSERT_TRUE(wait_for_list(
				    coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
				    focus_stat2.number_of_LinphoneSubscriptionIncomingReceived + 1, liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
				                             focus_stat2.number_of_LinphoneSubscriptionActive + 1,
				                             liblinphone_tester_sip_timeout));

				if (enable_video) {
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

					if (enable_ice) {
						BC_ASSERT_TRUE(check_ice(michelle.getCMgr(), focus.getCMgr(), LinphoneIceStateHostConnection));
						BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallUpdating, 1,
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(coresList,
						                             &michelle.getStats().number_of_LinphoneCallStreamsRunning, 2,
						                             liblinphone_tester_sip_timeout));
					}
				}

				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
				                             focus_stat2.number_of_participants_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
				                             focus_stat2.number_of_participant_devices_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
				                             focus_stat2.number_of_participant_devices_joined + 1,
				                             liblinphone_tester_sip_timeout));

				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_added,
				                             pauline_stat2.number_of_participants_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_added,
				                             pauline_stat2.number_of_participant_devices_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_joined,
				                             pauline_stat2.number_of_participant_devices_joined + 1,
				                             liblinphone_tester_sip_timeout));

				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participants_added,
				                             laure_stat2.number_of_participants_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_added,
				                             laure_stat2.number_of_participant_devices_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_joined,
				                             laure_stat2.number_of_participant_devices_joined + 1,
				                             liblinphone_tester_sip_timeout));

				if (network_restart) {
					ms_message("%s is back online after %s is added to conference %s",
					           linphone_core_get_identity(marie.getLc()), linphone_core_get_identity(michelle.getLc()),
					           conference_address_str);
					linphone_core_set_network_reachable(marie.getLc(), TRUE);
					BC_ASSERT_TRUE(wait_for_list(coresList,
					                             &marie.getStats().number_of_LinphoneSubscriptionOutgoingProgress,
					                             marie_stat2.number_of_LinphoneSubscriptionOutgoingProgress + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList,
					                             &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
					                             focus_stat2.number_of_LinphoneSubscriptionIncomingReceived + 2,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
					                             marie_stat2.number_of_LinphoneSubscriptionActive + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
					                             focus_stat2.number_of_LinphoneSubscriptionActive + 2,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallUpdating,
					                             marie_stat2.number_of_LinphoneCallUpdating + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneCallStreamsRunning,
					                             marie_stat2.number_of_LinphoneCallStreamsRunning + 1,
					                             liblinphone_tester_sip_timeout));
				}

				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_added,
				                             marie_stat2.number_of_participants_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_added,
				                             marie_stat2.number_of_participant_devices_added + 1,
				                             liblinphone_tester_sip_timeout));
				if (!network_restart) {
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_joined,
					                             marie_stat2.number_of_participant_devices_joined + 1,
					                             liblinphone_tester_sip_timeout));
				}

				std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
				    fill_memmber_list(members, participantList, marie.getCMgr(), participants_info);
				wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs,
				                            focus.getCMgr(), memberList, confAddr, enable_video);

			} else if (participant_list_type == LinphoneConferenceParticipantListTypeClosed) {
				extra_participants = 0;

				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallError, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneCallReleased, 1,
				                             liblinphone_tester_sip_timeout));

				// wait bit more to detect side effect if any
				CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
					return false;
				});

				BC_ASSERT_EQUAL(michelle.getStats().number_of_LinphoneConferenceStateCreated, 0, int, "%0d");
				BC_ASSERT_EQUAL(michelle.getStats().number_of_LinphoneSubscriptionOutgoingProgress, 0, int, "%0d");
				BC_ASSERT_EQUAL(michelle.getStats().number_of_LinphoneSubscriptionActive, 0, int, "%0d");
				BC_ASSERT_EQUAL(focus.getStats().number_of_participants_added, focus_stat2.number_of_participants_added,
				                int, "%0d");
				BC_ASSERT_EQUAL(focus.getStats().number_of_participant_devices_added,
				                focus_stat2.number_of_participant_devices_added, int, "%0d");
				BC_ASSERT_EQUAL(focus.getStats().number_of_participant_devices_joined,
				                focus_stat2.number_of_participant_devices_joined, int, "%0d");
				BC_ASSERT_EQUAL(marie.getStats().number_of_participants_added, marie_stat2.number_of_participants_added,
				                int, "%0d");
				BC_ASSERT_EQUAL(marie.getStats().number_of_participant_devices_added,
				                marie_stat2.number_of_participant_devices_added, int, "%0d");
				BC_ASSERT_EQUAL(marie.getStats().number_of_participant_devices_joined,
				                marie_stat2.number_of_participant_devices_joined, int, "%0d");
				BC_ASSERT_EQUAL(pauline.getStats().number_of_participants_added,
				                pauline_stat2.number_of_participants_added, int, "%0d");
				BC_ASSERT_EQUAL(pauline.getStats().number_of_participant_devices_added,
				                pauline_stat2.number_of_participant_devices_added, int, "%0d");
				BC_ASSERT_EQUAL(pauline.getStats().number_of_participant_devices_joined,
				                pauline_stat2.number_of_participant_devices_joined, int, "%0d");
				BC_ASSERT_EQUAL(laure.getStats().number_of_participants_added, laure_stat2.number_of_participants_added,
				                int, "%0d");
				BC_ASSERT_EQUAL(laure.getStats().number_of_participant_devices_added,
				                laure_stat2.number_of_participant_devices_added, int, "%0d");
				BC_ASSERT_EQUAL(laure.getStats().number_of_participant_devices_joined,
				                laure_stat2.number_of_participant_devices_joined, int, "%0d");
			}

			for (auto mgr : conferenceMgrs) {
				LinphoneConference *pconference =
				    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr, NULL);
				if (participant_list_type == LinphoneConferenceParticipantListTypeOpen) {
					BC_ASSERT_PTR_NOT_NULL(pconference);
				} else if (mgr == michelle.getCMgr()) {
					BC_ASSERT_PTR_NULL(pconference);
				}
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
							no_streams_video = (all_speakers)
							                       ? no_active_streams_video
							                       : (((security_level == LinphoneConferenceSecurityLevelEndToEnd) &&
							                           (layout == LinphoneConferenceLayoutActiveSpeaker))
							                              ? 6
							                              : 4);

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
					BC_ASSERT_EQUAL(bctbx_list_size(devices),
					                static_cast<size_t>(no_local_participants + extra_participants), size_t, "%zu");
					if (devices) {
						bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
					}
					BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
				}
			}
		}

		// wait bit more to detect side effect if any
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
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneSubscriptionTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &laure.getStats().number_of_LinphoneConferenceStateTerminationPending, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateDeleted, 1,
			                             liblinphone_tester_sip_timeout));

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

			LinphoneConference *conference = linphone_core_search_conference(laure.getLc(), NULL, puri, confAddr, NULL);
			BC_ASSERT_PTR_NULL(conference);

			no_local_participants = 3;
			if (uninvited_participant_dials) {
				stats marie_stat2 = marie.getStats();
				stats focus_stat2 = focus.getStats();
				stats pauline_stat2 = pauline.getStats();
				stats michelle_stat2 = michelle.getStats();

				extraParticipantMgrs.push_back(berthe.getCMgr());
				conferenceMgrs.push_back(berthe.getCMgr());
				members.push_back(berthe.getCMgr());
				participantList.insert(std::make_pair(
				    berthe.getCMgr(), add_participant_info_to_list(&participants_info, michelle.getCMgr()->identity,
				                                                   (listenerAllowed) ? LinphoneParticipantRoleListener
				                                                                     : LinphoneParticipantRoleSpeaker,
				                                                   0)));
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
					BC_ASSERT_TRUE(wait_for_list(
					    coresList, &berthe.getStats().number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
					BC_ASSERT_TRUE(
					    wait_for_list(coresList, &berthe.getStats().number_of_LinphoneSubscriptionActive, 1, 5000));

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
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
					                             focus_stat2.number_of_participant_devices_joined + 1,
					                             liblinphone_tester_sip_timeout));

					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_added,
					                             marie_stat2.number_of_participants_added + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_added,
					                             marie_stat2.number_of_participant_devices_added + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_joined,
					                             marie_stat2.number_of_participant_devices_joined + 1,
					                             liblinphone_tester_sip_timeout));

					BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_added,
					                             pauline_stat2.number_of_participants_added + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_added,
					                             pauline_stat2.number_of_participant_devices_added + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_joined,
					                             pauline_stat2.number_of_participant_devices_joined + 1,
					                             liblinphone_tester_sip_timeout));

					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participants_added,
					                             michelle_stat2.number_of_participants_added + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_added,
					                             michelle_stat2.number_of_participant_devices_added + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_joined,
					                             michelle_stat2.number_of_participant_devices_joined + 1,
					                             liblinphone_tester_sip_timeout));

					std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
					    fill_memmber_list(members, participantList, marie.getCMgr(), participants_info);
					wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs,
					                            focus.getCMgr(), memberList, confAddr, enable_video);
				} else if (participant_list_type == LinphoneConferenceParticipantListTypeClosed) {
					extra_participants = 0;

					BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallError, 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallReleased, 1,
					                             liblinphone_tester_sip_timeout));

					// wait bit more to detect side effect if any
					CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
					    .waitUntil(chrono::seconds(2), [] { return false; });

					BC_ASSERT_EQUAL(berthe.getStats().number_of_LinphoneConferenceStateCreated, 0, int, "%0d");
					BC_ASSERT_EQUAL(berthe.getStats().number_of_LinphoneSubscriptionOutgoingProgress, 0, int, "%0d");
					BC_ASSERT_EQUAL(berthe.getStats().number_of_LinphoneSubscriptionActive, 0, int, "%0d");
					BC_ASSERT_EQUAL(focus.getStats().number_of_participants_added,
					                focus_stat2.number_of_participants_added, int, "%0d");
					BC_ASSERT_EQUAL(focus.getStats().number_of_participant_devices_added,
					                focus_stat2.number_of_participant_devices_added, int, "%0d");
					BC_ASSERT_EQUAL(focus.getStats().number_of_participant_devices_joined,
					                focus_stat2.number_of_participant_devices_joined, int, "%0d");
					BC_ASSERT_EQUAL(marie.getStats().number_of_participants_added,
					                marie_stat2.number_of_participants_added, int, "%0d");
					BC_ASSERT_EQUAL(marie.getStats().number_of_participant_devices_added,
					                marie_stat2.number_of_participant_devices_added, int, "%0d");
					BC_ASSERT_EQUAL(marie.getStats().number_of_participant_devices_joined,
					                marie_stat2.number_of_participant_devices_joined, int, "%0d");
					BC_ASSERT_EQUAL(pauline.getStats().number_of_participants_added,
					                pauline_stat2.number_of_participants_added, int, "%0d");
					BC_ASSERT_EQUAL(pauline.getStats().number_of_participant_devices_added,
					                pauline_stat2.number_of_participant_devices_added, int, "%0d");
					BC_ASSERT_EQUAL(pauline.getStats().number_of_participant_devices_joined,
					                pauline_stat2.number_of_participant_devices_joined, int, "%0d");
					BC_ASSERT_EQUAL(michelle.getStats().number_of_participants_added,
					                michelle_stat2.number_of_participants_added, int, "%0d");
					BC_ASSERT_EQUAL(michelle.getStats().number_of_participant_devices_added,
					                michelle_stat2.number_of_participant_devices_added, int, "%0d");
					BC_ASSERT_EQUAL(michelle.getStats().number_of_participant_devices_joined,
					                michelle_stat2.number_of_participant_devices_joined, int, "%0d");
				}

				for (auto mgr : conferenceMgrs) {
					LinphoneConference *pconference =
					    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr, NULL);
					if ((participant_list_type == LinphoneConferenceParticipantListTypeOpen) ||
					    ((mgr != berthe.getCMgr()) && (mgr != michelle.getCMgr()))) {
						BC_ASSERT_PTR_NOT_NULL(pconference);
					} else if ((mgr == berthe.getCMgr()) || (mgr == michelle.getCMgr())) {
						BC_ASSERT_PTR_NULL(pconference);
					}
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
								no_streams_video = ((security_level == LinphoneConferenceSecurityLevelEndToEnd) &&
								                    (layout == LinphoneConferenceLayoutActiveSpeaker))
								                       ? 6
								                       : 4;
								no_max_streams_audio =
								    (security_level == LinphoneConferenceSecurityLevelEndToEnd) ? 3 : 1;

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

				// wait bit more to detect side effect if any
				CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
					return false;
				});
			}

		} else {
			mgrsToRemove.push_back(laure.getCMgr());
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
			if (pauline_call) {
				const LinphoneCallParams *call_cparams = linphone_call_get_current_params(pauline_call);
				video_enabled = linphone_call_params_video_enabled(call_cparams);
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

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_on_hold,
			                             focus_stat2.number_of_participant_devices_on_hold + 1,
			                             liblinphone_tester_sip_timeout));

			if (!remove_participant) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_on_hold,
				                             laure_stat2.number_of_participant_devices_on_hold + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList,
				                             &laure.getStats().number_of_participant_devices_media_capability_changed,
				                             laure_stat2.number_of_participant_devices_media_capability_changed + 1,
				                             liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_on_hold,
			                             marie_stat2.number_of_participant_devices_on_hold + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &marie.getStats().number_of_participant_devices_media_capability_changed,
			                             marie_stat2.number_of_participant_devices_media_capability_changed + 1,
			                             liblinphone_tester_sip_timeout));

			if (uninvited_participant_dials && (participant_list_type == LinphoneConferenceParticipantListTypeOpen)) {
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

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
			                             focus_stat2.number_of_participant_devices_joined + 1,
			                             liblinphone_tester_sip_timeout));

			if (!remove_participant) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_joined,
				                             laure_stat2.number_of_participant_devices_joined + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList,
				                             &laure.getStats().number_of_participant_devices_media_capability_changed,
				                             laure_stat2.number_of_participant_devices_media_capability_changed + 2,
				                             liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_joined,
			                             marie_stat2.number_of_participant_devices_joined + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &marie.getStats().number_of_participant_devices_media_capability_changed,
			                             marie_stat2.number_of_participant_devices_media_capability_changed + 2,
			                             liblinphone_tester_sip_timeout));

			if (uninvited_participant_dials && (participant_list_type == LinphoneConferenceParticipantListTypeOpen)) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_joined,
				                             michelle_stat2.number_of_participant_devices_joined + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(
				    coresList, &michelle.getStats().number_of_participant_devices_media_capability_changed,
				    michelle_stat2.number_of_participant_devices_media_capability_changed + 1,
				    liblinphone_tester_sip_timeout));
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
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionTerminated,
					                             marie_stat2.number_of_LinphoneSubscriptionTerminated + 1,
					                             liblinphone_tester_sip_timeout));
				}

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

				LinphoneConference *pconference =
				    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr, NULL);
				BC_ASSERT_PTR_NULL(pconference);

				if (network_restart) {
					ms_message("%s is back online after %s leaves conference %s",
					           linphone_core_get_identity(marie.getLc()), linphone_core_get_identity(mgr->lc),
					           conference_address_str);
					linphone_core_set_network_reachable(marie.getLc(), TRUE);
					BC_ASSERT_TRUE(wait_for_list(coresList,
					                             &marie.getStats().number_of_LinphoneSubscriptionOutgoingProgress,
					                             marie_stat2.number_of_LinphoneSubscriptionOutgoingProgress + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList,
					                             &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
					                             focus_stat2.number_of_LinphoneSubscriptionIncomingReceived + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
					                             marie_stat2.number_of_LinphoneSubscriptionActive + 1,
					                             liblinphone_tester_sip_timeout));
					BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
					                             focus_stat2.number_of_LinphoneSubscriptionActive + 1,
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

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
		                             focus_stat.number_of_participants_removed + 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed + 2,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participants_removed,
		                             marie_stat.number_of_participants_removed + 2, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_removed,
		                             marie_stat.number_of_participant_devices_removed + 2,
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
					LinphoneConference *pconference =
					    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr, NULL);
					if (!remove_participant && (mgr == berthe.getCMgr())) {
						BC_ASSERT_PTR_NULL(pconference);
					} else {
						BC_ASSERT_PTR_NOT_NULL(pconference);
					}
					if (pconference) {
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
						BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated, 1,
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(coresList,
						                             &mgr->stat.number_of_LinphoneConferenceStateTerminationPending, 1,
						                             liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated,
						                             1, liblinphone_tester_sip_timeout));
						BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1,
						                             liblinphone_tester_sip_timeout));
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

		// wait bit more to detect side effect if any
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

			// Explicitely terminate conference as those on server are static by default
			LinphoneConference *pconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				linphone_conference_terminate(pconference);
			}
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &focus.getStats().number_of_LinphoneConferenceStateTerminationPending, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
			                             liblinphone_tester_sip_timeout));
		}

		std::list<LinphoneCoreManager *> allMembers{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()};
		if ((participant_list_type == LinphoneConferenceParticipantListTypeOpen) && uninvited_participant_dials) {
			allMembers.push_back(michelle.getCMgr());
			if (remove_participant) {
				allMembers.push_back(berthe.getCMgr());
			}
		}
		for (auto mgr : allMembers) {
			const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
			BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(call_logs),
			                ((client_restart && (mgr == marie.getCMgr())) ? 2 : 1), unsigned int, "%u");

			bctbx_list_t *mgr_focus_call_log =
			    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
			if (mgr_focus_call_log) {
				BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(mgr_focus_call_log),
				                ((client_restart && (mgr == marie.getCMgr())) ? 2 : 1), unsigned int, "%u");
				for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
					LinphoneCallLog *call_log = (LinphoneCallLog *)it->data;
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				}
				bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
			}
		}

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle}).waitUntil(chrono::seconds(2), [] { return false; });

		ms_free(conference_address_str);
		linphone_address_unref(confAddr);
		bctbx_list_free_with_data(participants_info, (bctbx_list_free_func)linphone_participant_info_unref);
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

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);
		linphone_core_set_conference_participant_list_type(focus.getLc(), participant_list_type);

		stats focus_stat = focus.getStats();
		stats marie_stat = marie.getStats();

		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr()};
		if (one_addition) {
			participants.push_back(michelle.getCMgr());
		}

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
		LinphoneAddress *confAddr = create_conference_on_server(focus, marie, participantList, start_time, end_time,
		                                                        initialSubject, description, TRUE, security_level);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 2,
		                             liblinphone_tester_sip_timeout));

		auto members = participants;
		members.push_back(marie.getCMgr());
		auto conferenceMgrs = members;
		conferenceMgrs.push_back(focus.getCMgr());

		update_sequence_number(&participants_info, {}, 0, -1);

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
				check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info,
				                            start_time, actual_duration, initialSubject, description, 0,
				                            LinphoneConferenceInfoStateNew, security_level, FALSE);

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
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyFullStateReceived,
		                             marie_stat.number_of_NotifyFullStateReceived + 1, liblinphone_tester_sip_timeout));

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

		add_participant_info_to_list(&participants_info, marie.getCMgr()->identity, LinphoneParticipantRoleSpeaker, 0);
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

			check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info, start_time,
			                            actual_duration, initialSubject, description, 0, LinphoneConferenceInfoStateNew,
			                            security_level, FALSE);

			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
				BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				LinphoneConferenceInfo *call_log_info = linphone_call_log_get_conference_info(call_log);
				if (BC_ASSERT_PTR_NOT_NULL(call_log_info)) {
					check_conference_info_against_db(mgr, confAddr, call_log_info);
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
		                             marie_stat.number_of_LinphoneSubscriptionOutgoingProgress + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
		                             marie_stat.number_of_LinphoneSubscriptionActive + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyFullStateReceived,
		                             marie_stat.number_of_NotifyFullStateReceived + 1, liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
		                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		                  focus_stat.number_of_LinphoneSubscriptionIncomingReceived + static_cast<int>(members.size()),
		                  liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + static_cast<int>(members.size()),
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + static_cast<int>(members.size()),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + static_cast<int>(members.size()),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
		                             focus_stat.number_of_participant_devices_joined + static_cast<int>(members.size()),
		                             liblinphone_tester_sip_timeout));

		LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		std::map<LinphoneCoreManager *, LinphoneParticipantInfo *> memberList =
		    fill_memmber_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs, focus.getCMgr(),
		                            memberList, confAddr, TRUE);

		// wait bit more to detect side effect if any
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

					LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(mgr->lc);
					bool_t enabled = !!linphone_video_activation_policy_get_automatically_initiate(pol);
					linphone_video_activation_policy_unref(pol);

					LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					size_t no_streams_audio = 0;
					size_t no_max_streams_video = (enabled) ? (static_cast<int>(members.size()) + 1) : 0;
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
				LinphoneParticipant *me = linphone_conference_get_me(pconference);
				BC_ASSERT_TRUE(linphone_participant_is_admin(me) ==
				               ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())));
				BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_participant_get_address(me), mgr->identity));
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
		stats michelle_stat = michelle.getStats();
		stats berthe_stat = berthe.getStats();
		stats pauline_stat = pauline.getStats();
		stats laure_stat = laure.getStats();

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

		LinphoneCall *berthe_call =
		    linphone_core_get_call_by_remote_address2(berthe.getLc(), focus.getCMgr()->identity);
		BC_ASSERT_PTR_NOT_NULL(berthe_call);

		LinphoneCall *michelle_call =
		    linphone_core_get_call_by_remote_address2(michelle.getLc(), focus.getCMgr()->identity);
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
			                             berthe_stat.number_of_LinphoneSubscriptionOutgoingProgress + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneSubscriptionActive,
			                             berthe_stat.number_of_LinphoneSubscriptionActive + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_NotifyFullStateReceived,
			                             berthe_stat.number_of_NotifyFullStateReceived + 1,
			                             liblinphone_tester_sip_timeout));

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
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_NotifyReceived,
				                             michelle_stat.number_of_NotifyReceived + 1,
				                             liblinphone_tester_sip_timeout));
			}

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
			                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + participant_added,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
			                             focus_stat.number_of_LinphoneSubscriptionActive + participant_added,
			                             liblinphone_tester_sip_timeout));

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

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_alerting,
			                             focus_stat.number_of_participant_devices_alerting + participant_added,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_alerting,
			                             marie_stat.number_of_participant_devices_alerting + participant_added,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_alerting,
			                             pauline_stat.number_of_participant_devices_alerting + participant_added,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_alerting,
			                             laure_stat.number_of_participant_devices_alerting + participant_added,
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
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
			                             focus_stat.number_of_participant_devices_joined + participant_added,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_joined,
			                             marie_stat.number_of_participant_devices_joined + participant_added,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_joined,
			                             pauline_stat.number_of_participant_devices_joined + participant_added,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_joined,
			                             laure_stat.number_of_participant_devices_joined + participant_added,
			                             liblinphone_tester_sip_timeout));
			if (one_addition) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_alerting,
				                             michelle_stat.number_of_participant_devices_alerting + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_added,
				                             michelle_stat.number_of_participant_devices_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_joined,
				                             michelle_stat.number_of_participant_devices_joined + 1,
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

		LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(pauline.getLc());
		bool_t enable = !!!linphone_video_activation_policy_get_automatically_initiate(pol);
		linphone_video_activation_policy_unref(pol);

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
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneSubscriptionTerminated, 1,
			                             liblinphone_tester_sip_timeout));
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
		                             focus_stat.number_of_LinphoneSubscriptionTerminated + 1,
		                             liblinphone_tester_sip_timeout));
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
			LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, focus.getCMgr()->identity);
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
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated, call_ended,
				                             liblinphone_tester_sip_timeout));
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
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
		                             focus_stat.number_of_LinphoneSubscriptionTerminated + ((accept) ? 5 : 4),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
		                             focus_stat.number_of_participants_removed + ((accept) ? 5 : 4),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed + ((accept) ? 5 : 4),
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                focus_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminated,
		                focus_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateDeleted,
		                focus_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		for (auto mgr : {focus.getCMgr()}) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), 0, int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), 0, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
			}
		}

		focus_stat = focus.getStats();
		const bctbx_list_t *calls = linphone_core_get_calls(focus.getLc());
		BC_ASSERT_EQUAL(bctbx_list_size(calls), 0, size_t, "%zu");

		// Explicitely terminate conference as those on server are static by default
		if (fconference) {
			linphone_conference_terminate(fconference);
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : members) {
			unsigned int expected_call_logs = (mgr == pauline.getCMgr()) ? 2 : 1;
			const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
			BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(call_logs), expected_call_logs, unsigned int, "%u");

			bctbx_list_t *mgr_focus_call_log =
			    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
			if (mgr_focus_call_log) {
				BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(mgr_focus_call_log), expected_call_logs, unsigned int,
				                "%u");
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
			                            FALSE);

			bctbx_list_free_with_data(participants_info2, (bctbx_list_free_func)linphone_participant_info_unref);
		}

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
			return false;
		});

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
		time_t start_time1 = ms_time(NULL);
		time_t end_time1 = (start_time1 + 600);
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
		LinphoneAddress *confAddr1 = create_conference_on_server(focus, marie, participantList1, start_time1, end_time1,
		                                                         subject1, description1, TRUE, security_level);
		BC_ASSERT_PTR_NOT_NULL(confAddr1);
		char *conference1_address_str = (confAddr1) ? linphone_address_as_string(confAddr1) : ms_strdup("<unknown>");
		BC_ASSERT_PTR_NOT_NULL(confAddr1);
		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 2,
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
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 2,
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
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
		                             focus_stat.number_of_participant_devices_joined + 3,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + 3,
		                             liblinphone_tester_sip_timeout));

		LinphoneAddress *focus_uri1 = focus.getCMgr()->identity;
		LinphoneConference *fconference1 =
		    linphone_core_search_conference(focus.getLc(), NULL, focus_uri1, confAddr1, NULL);
		BC_ASSERT_PTR_NOT_NULL(fconference1);

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle}).waitUntil(chrono::seconds(2), [] { return false; });

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr1, NULL);
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
				LinphoneParticipant *me = linphone_conference_get_me(pconference);
				BC_ASSERT_TRUE(linphone_participant_is_admin(me) ==
				               ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())));
				BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_participant_get_address(me), mgr->identity));
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

		time_t start_time2 = (dialout) ? -1 : ms_time(NULL);
		time_t end_time2 = (dialout) ? -1 : (start_time2 + 600);
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
		                                description2, TRUE, security_level);
		BC_ASSERT_PTR_NOT_NULL(confAddr2);
		char *conference2_address_str = (confAddr2) ? linphone_address_as_string(confAddr2) : ms_strdup("<unknown>");

		// Chat room creation to send ICS
		if (same_organizer) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 3,
			                             liblinphone_tester_sip_timeout));
		} else {
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateCreated, 2,
			                             liblinphone_tester_sip_timeout));
		}

		if (confAddr2) {
			if (dialout) {
				for (auto mgr : participants2) {
					LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr2);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					auto it = std::find(participants1.cbegin(), participants1.cend(), mgr);
					if (pcall && (it == participants1.cend())) {
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
				mgr_having_two_confs.push_back(pauline.getCMgr());
				mgr_in_conf2.push_back(pauline.getCMgr());
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
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated,
			                             ((mgr == marie.getCMgr()) ? 4 : 3), liblinphone_tester_sip_timeout));
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
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated,
			                             ((same_organizer) ? 2 : 3), liblinphone_tester_sip_timeout));
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
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
		                             focus_stat.number_of_participant_devices_joined + subscription,
		                             liblinphone_tester_sip_timeout));

		// Marie and Pauline leave conference1
		const int onhold = (dialout) ? ((same_organizer) ? 1 : 0) : 2;
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_on_hold,
		                             focus_stat.number_of_participant_devices_on_hold + onhold,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList,
		                             &laure.getStats().number_of_participant_devices_media_capability_changed, onhold,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_participant_devices_on_hold, onhold,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList,
		                             &marie.getStats().number_of_participant_devices_media_capability_changed,
		                             onhold - 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_on_hold, onhold - 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList,
		                             &pauline.getStats().number_of_participant_devices_media_capability_changed, onhold,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_on_hold, onhold,
		                             liblinphone_tester_sip_timeout));

		LinphoneAddress *focus_uri2 = focus.getCMgr()->identity;
		LinphoneConference *fconference2 =
		    linphone_core_search_conference(focus.getLc(), NULL, focus_uri2, confAddr2, NULL);
		BC_ASSERT_PTR_NOT_NULL(fconference2);

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr()}) {
			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr1, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				if (mgr == focus.getCMgr()) {
					BC_ASSERT_EQUAL(bctbx_list_size(devices), 3, size_t, "%zu");
					BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), 3, int, "%0d");
				} else {
					BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), 2, int, "%0d");
					if ((mgr == laure.getCMgr()) ||
					    (dialout && ((mgr == pauline.getCMgr()) || (!same_organizer && (mgr == marie.getCMgr()))))) {
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

		// wait bit more to detect side effect if any
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
				BC_ASSERT_EQUAL(bctbx_list_size(devices), (dialout) ? ((same_organizer) ? 2 : 1) : 3, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), subject2);

				LinphoneParticipant *me = linphone_conference_get_me(pconference);

				BC_ASSERT_TRUE(linphone_participant_is_admin(me) ==
				               ((mgr == organizer2.getCMgr()) || (mgr == focus.getCMgr())));
				BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_participant_get_address(me), mgr->identity));
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

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

		focus_stat = focus.getStats();
		stats pauline_stat = pauline.getStats();

		std::list<LinphoneCoreManager *> mgr_conf2_to_remove{michelle.getCMgr()};
		if (!dialout || same_organizer) {
			mgr_conf2_to_remove.push_back(marie.getCMgr());
		}
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

		if (!dialout) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_removed,
			                             pauline_stat.number_of_participants_removed +
			                                 static_cast<int>(mgr_conf2_to_remove.size()),
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_removed,
			                             pauline_stat.number_of_participant_devices_removed +
			                                 static_cast<int>(mgr_conf2_to_remove.size()),
			                             liblinphone_tester_sip_timeout));
		}

		// wait bit more to detect side effect if any
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
			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr2, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference),
				                ((mgr == focus.getCMgr()) ? ((!dialout || same_organizer) ? 1 : 2) : 0), int, "%0d");
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
		if (!dialout) {
			mgr_rejoining.push_back(pauline.getCMgr());
		}

		for (auto mgr : mgr_rejoining) {
			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr1, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				ms_message("%s is joining conference %s", linphone_core_get_identity(mgr->lc), conference1_address_str);
				linphone_conference_enter(pconference);
			}
		}

		LinphoneAddress *focusUri = focus.getCMgr()->identity;
		LinphoneConference *conference1 =
		    linphone_core_search_conference(focus.getLc(), NULL, focusUri, confAddr1, NULL);

		if (!dialout) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallPaused,
			                             pauline_stat.number_of_LinphoneCallPaused + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneCallStreamsRunning,
			                             pauline_stat.number_of_LinphoneCallStreamsRunning + 1,
			                             liblinphone_tester_sip_timeout));
		}
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

		if (!dialout) {
			// Pauline leaves conference2
			// Pauline and Marie enter conference1
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_on_hold,
			                             focus_stat.number_of_participant_devices_on_hold + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
			                             focus_stat.number_of_participant_devices_joined + 2,
			                             liblinphone_tester_sip_timeout));
		}

		for (auto mgr : mgr_in_conf2) {
			LinphoneConference *pconference =
			    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr2, NULL);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference),
				                ((mgr == focus.getCMgr()) ? ((!dialout || same_organizer) ? 1 : 2) : 0), int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), ((mgr == focus.getCMgr() && !dialout) ? 1 : 0), size_t,
				                "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), subject2);
			}
		}

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle}).waitUntil(chrono::seconds(2), [] { return false; });

		// Laure and Pauline are removed from conference1
		focus_stat = focus.getStats();
		marie_stat = marie.getStats();
		pauline_stat = pauline.getStats();
		stats laure_stat = laure.getStats();
		int cnt = 0;
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
			                             focus_stat.number_of_LinphoneCallEnd + cnt, liblinphone_tester_sip_timeout));
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

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle}).waitUntil(chrono::seconds(2), [] { return false; });

		if (!dialout) {
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
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
			                             focus_stat.number_of_participant_devices_joined + 1,
			                             liblinphone_tester_sip_timeout));

			for (auto mgr : mgr_in_conf2) {
				LinphoneConference *pconference =
				    linphone_core_search_conference(mgr->lc, NULL, mgr->identity, confAddr2, NULL);
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

			// wait bit more to detect side effect if any
			CoreManagerAssert({focus, marie, pauline, laure, michelle}).waitUntil(chrono::seconds(2), [] {
				return false;
			});

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
			                             pauline_stat.number_of_LinphoneCallReleased + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneSubscriptionTerminated,
			                             pauline_stat.number_of_LinphoneSubscriptionTerminated + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(
			    coresList, &pauline.getStats().number_of_LinphoneConferenceStateTerminationPending,
			    pauline_stat.number_of_LinphoneConferenceStateTerminationPending + 1, liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateTerminated,
			                             pauline_stat.number_of_LinphoneConferenceStateTerminated + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateDeleted,
			                             pauline_stat.number_of_LinphoneConferenceStateDeleted + 1,
			                             liblinphone_tester_sip_timeout));
		}

		// Explicitely terminate conference as those on server are static by default
		LinphoneConference *conference2 =
		    linphone_core_search_conference(focus.getLc(), NULL, focusUri, confAddr2, NULL);
		linphone_conference_terminate(conference2);
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             focus_stat.number_of_LinphoneConferenceStateTerminationPending + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated,
		                             focus_stat.number_of_LinphoneConferenceStateTerminated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted,
		                             focus_stat.number_of_LinphoneConferenceStateDeleted + 1,
		                             liblinphone_tester_sip_timeout));

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

		// Explicitely terminate conference as those on server are static by default
		linphone_conference_terminate(conference1);
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             focus_stat.number_of_LinphoneConferenceStateTerminationPending + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated,
		                             focus_stat.number_of_LinphoneConferenceStateTerminated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted,
		                             focus_stat.number_of_LinphoneConferenceStateDeleted + 1,
		                             liblinphone_tester_sip_timeout));

		// wait bit more to detect side effect if any
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
		LinphoneAddress *confAddr = create_conference_on_server(focus, marie, participantList, start_time, end_time,
		                                                        initialSubject, description, TRUE, security_level);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 2,
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
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated,
			                             ((mgr == marie.getCMgr()) ? 3 : 2), liblinphone_tester_sip_timeout));
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
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
		                             focus_stat.number_of_participant_devices_joined + 1,
		                             liblinphone_tester_sip_timeout));

		LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		// wait bit more to detect side effect if any
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
				LinphoneParticipant *me = linphone_conference_get_me(pconference);
				BC_ASSERT_TRUE(linphone_participant_is_admin(me) ==
				               ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())));
				BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_participant_get_address(me), mgr->identity));
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

		// wait bit more to detect side effect if any
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

		// wait bit more to detect side effect if any
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

		// wait bit more to detect side effect if any
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
		if (layout == LinphoneConferenceLayoutGrid) {
			BC_ASSERT_FALSE(wait_for_list(coresList,
			                              &marie.getStats().number_of_participant_devices_media_capability_changed,
			                              marie_stat.number_of_participant_devices_media_capability_changed + 1, 2000));
		} else {
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &marie.getStats().number_of_participant_devices_media_capability_changed,
			                             marie_stat.number_of_participant_devices_media_capability_changed + 1, 2000));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallUpdatedByRemote,
		                             focus_stat.number_of_LinphoneCallUpdatedByRemote + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		if (layout == LinphoneConferenceLayoutGrid) {
			BC_ASSERT_FALSE(wait_for_list(coresList,
			                              &focus.getStats().number_of_participant_devices_media_capability_changed,
			                              focus_stat.number_of_participant_devices_media_capability_changed + 1, 2000));
		} else {
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &focus.getStats().number_of_participant_devices_media_capability_changed,
			                             focus_stat.number_of_participant_devices_media_capability_changed + 1, 2000));
		}

		// wait bit more to detect side effect if any
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

		// wait bit more to detect side effect if any
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

			// Explicitely terminate conference as those on server are static by default
			LinphoneConference *pconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				linphone_conference_terminate(pconference);
			}
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
				BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(call_logs), 1, unsigned int, "%u");

				bctbx_list_t *mgr_focus_call_log =
				    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
				BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
				if (mgr_focus_call_log) {
					BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(mgr_focus_call_log), 1, unsigned int, "%u");
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

		time_t start_time = (dialout) ? -1 : (ms_time(NULL) + 10);
		bool_t send_ics = TRUE;

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

		LinphoneAddress *confAddr = create_conference_on_server(focus, marie, participantList, start_time, -1,
		                                                        initialSubject, description, send_ics, security_level);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

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

		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallStreamsRunning,
		                             berthe_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallStreamsRunning,
		                             focus_stat.number_of_LinphoneCallStreamsRunning + 1,
		                             liblinphone_tester_sip_timeout));

		if (dialout) {
			BC_ASSERT_PTR_NULL(linphone_core_get_current_call(focus.getLc()));
		} else {
			BC_ASSERT_PTR_NOT_NULL(linphone_core_get_current_call(focus.getLc()));
		}

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
		                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionTerminated,
		                             focus_stat.number_of_LinphoneSubscriptionTerminated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneSubscriptionOutgoingProgress, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneSubscriptionError, 1,
		                             liblinphone_tester_sip_timeout));
		if (dialout) {
			// Chat room creation to send ICS
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated,
			                             (send_ics ? 2 : 1), liblinphone_tester_sip_timeout));

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

		if (!dialout) {
			add_participant_info_to_list(&participants_info, berthe.getCMgr()->identity,
			                             LinphoneParticipantRoleListener, -1);
		}

		update_sequence_number(&participants_info, {berthe.getCMgr()->identity}, 0, -1);
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

			if (!dialout) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_participant_devices_added, 1,
				                             liblinphone_tester_sip_timeout));
			}
			check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info, 0, 0,
			                            initialSubject, (send_ics) ? description : NULL, 0,
			                            LinphoneConferenceInfoStateNew, security_level, FALSE);

			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
				BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				LinphoneConferenceInfo *call_log_info = linphone_call_log_get_conference_info(call_log);
				if (BC_ASSERT_PTR_NOT_NULL(call_log_info)) {
					check_conference_info_against_db(mgr, confAddr, call_log_info);
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
		                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + 5, 5000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive + 4, 5000));

		int no_participants = (dialout) ? 4 : 5;
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + no_participants,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + no_participants,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
		                             focus_stat.number_of_participant_devices_joined + no_participants,
		                             liblinphone_tester_sip_timeout));

		LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		for (auto mgr : conferenceMgrs) {
			// wait bit more to detect side effect if any
			CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe})
			    .waitUntil(chrono::seconds(50), [mgr, &focus, &members, confAddr, &participantList] {
				    size_t nb_audio_streams = 1;
				    size_t nb_video_streams = 0;
				    size_t nb_text_streams = 0;
				    std::list<LinphoneCall *> calls;

				    if (mgr == focus.getCMgr()) {
					    for (auto m : members) {
						    LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, m->identity);
						    BC_ASSERT_PTR_NOT_NULL(call);
						    if (call) {
							    calls.push_back(call);
						    } else {
							    return false;
						    }
					    }
				    } else {
					    LinphoneCall *call = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					    BC_ASSERT_PTR_NOT_NULL(call);
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
				    BC_ASSERT_PTR_NOT_NULL(conference);
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
				    }
				    if (!video_check) {
					    return false;
				    }

				    bool role_check = true;
				    for (const auto &[mMgr, info] : participantList) {
					    LinphoneParticipant *p = linphone_conference_is_me(conference, mMgr->identity)
					                                 ? linphone_conference_get_me(conference)
					                                 : linphone_conference_find_participant(conference, mMgr->identity);
					    role_check &= (p != nullptr);
					    if (p) {
						    role_check &=
						        (linphone_participant_get_role(p) == linphone_participant_info_get_role(info));
					    }
				    }
				    return role_check;
			    });

			if (mgr != focus.getCMgr()) {
				check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info, 0, 0,
				                            initialSubject, (send_ics) ? description : NULL, 0,
				                            LinphoneConferenceInfoStateNew, security_level, FALSE);

				LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(pcall);
				if (pcall) {
					LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
					LinphoneConferenceInfo *call_log_info = linphone_call_log_get_conference_info(call_log);
					if (BC_ASSERT_PTR_NOT_NULL(call_log_info)) {
						check_conference_info_against_db(mgr, confAddr, call_log_info);
					}
				}
			}
		}

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(15), [] {
			return false;
		});

		for (auto mgr : conferenceMgrs) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				int no_participants = 0;
				if (mgr == focus.getCMgr()) {
					no_participants = (dialout) ? 4 : 5;
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
					no_participants = (dialout) ? 3 : 4;
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
				BC_ASSERT_EQUAL(bctbx_list_size(devices), (dialout) ? 4 : 5, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
				LinphoneParticipant *me = linphone_conference_get_me(pconference);
				BC_ASSERT_TRUE(linphone_participant_is_admin(me) ==
				               ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())));
				BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_participant_get_address(me), mgr->identity));
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

		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                focus_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminated,
		                focus_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateDeleted,
		                focus_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		for (auto mgr : {focus.getCMgr()}) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), (dialout) ? 0 : 1, int, "%0d");
				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), (dialout) ? 0 : 1, size_t, "%zu");
				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
				BC_ASSERT_STRING_EQUAL(linphone_conference_get_subject(pconference), initialSubject);
			}
		}

		const bctbx_list_t *calls = linphone_core_get_calls(focus.getLc());
		BC_ASSERT_EQUAL(bctbx_list_size(calls), 1, size_t, "%zu");

		LinphoneCall *focus_berthe_call =
		    linphone_core_get_call_by_remote_address2(focus.getLc(), berthe.getCMgr()->identity);
		BC_ASSERT_PTR_NOT_NULL(focus_berthe_call);
		if (focus_berthe_call) {
			linphone_call_terminate(focus_berthe_call);
		}

		if (!dialout) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
			                             focus_stat.number_of_participants_removed + 5,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
			                             focus_stat.number_of_participant_devices_removed + 5,
			                             liblinphone_tester_sip_timeout));
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallEnd,
		                             focus_stat.number_of_LinphoneCallEnd + 5, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
		                             focus_stat.number_of_LinphoneCallReleased + 5, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallEnd, 1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &berthe.getStats().number_of_LinphoneCallReleased, 1,
		                             liblinphone_tester_sip_timeout));

		// Explicitely terminate conference as those on server are static by default
		if (fconference) {
			linphone_conference_terminate(fconference);
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                             1, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
		                             liblinphone_tester_sip_timeout));

		for (auto mgr : members) {
			const bctbx_list_t *call_logs = linphone_core_get_call_logs(mgr->lc);
			BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(call_logs), 1, unsigned int, "%u");

			bctbx_list_t *mgr_focus_call_log =
			    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
			BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
			if (mgr_focus_call_log) {
				BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(mgr_focus_call_log), 1, unsigned int, "%u");
				for (bctbx_list_t *it = mgr_focus_call_log; it; it = bctbx_list_next(it)) {
					LinphoneCallLog *call_log = (LinphoneCallLog *)it->data;
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
					LinphoneConferenceInfo *call_log_info = linphone_call_log_get_conference_info(call_log);
					if (BC_ASSERT_PTR_NOT_NULL(call_log_info)) {
						check_conference_info_against_db(mgr, confAddr, call_log_info);
					}
				}
				bctbx_list_free_with_data(mgr_focus_call_log, (bctbx_list_free_func)linphone_call_log_unref);
			}

			check_conference_info_in_db(mgr, NULL, confAddr, marie.getCMgr()->identity, participants_info, start_time,
			                            0, initialSubject, (send_ics || (mgr == marie.getCMgr())) ? description : NULL,
			                            0, LinphoneConferenceInfoStateNew, security_level, FALSE);
		}

		// wait bit more to detect side effect if any
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
                                                 LinphoneConferenceSecurityLevel security_level) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress());
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress());
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);

		setup_conference_info_cbs(marie.getCMgr());

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);

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
		    fill_memmber_list(members, participantList, marie.getCMgr(), participants_info);
		wait_for_conference_streams({focus, marie, pauline, laure}, conferenceMgrs, focus.getCMgr(), memberList,
		                            confAddr, toggle_video);

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added, 3,
		                             liblinphone_tester_sip_timeout));

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

		for (auto mgr : conferenceMgrs) {
			LinphoneAddress *uri = mgr->identity;
			LinphoneConference *pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
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
				                            FALSE);
			}
		}

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

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
				                            newSubject, NULL, 0, LinphoneConferenceInfoStateNew, security_level, FALSE);
			}
			participant_stats.pop_front();
		}

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

		LinphoneAddress *focus_addr = focus.getCMgr()->identity;
		LinphoneConference *fconference =
		    linphone_core_search_conference(focus.getLc(), NULL, focus_addr, confAddr, NULL);
		BC_ASSERT_PTR_NOT_NULL(fconference);

		for (auto mgr : members) {
			// Explicitely terminate conference as those on server are static by default
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
		ms_message("%s core suddently loses network and restarts", linphone_core_get_identity(laure.getLc()));
		linphone_core_set_network_reachable(laure.getLc(), FALSE);
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

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
		                             focus_stat.number_of_participant_devices_joined + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_participant_devices_joined,
		                             marie_stat.number_of_participant_devices_joined + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_joined,
		                             pauline_stat.number_of_participant_devices_joined + 1,
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
			LinphoneAddress *uri = mgr->identity;
			LinphoneConference *pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
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
				                            newSubject, NULL, 0, LinphoneConferenceInfoStateNew, security_level, FALSE);
			}
		}

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(5), [] { return false; });

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

					// wait bit more to detect side effect if any. During this time, some participants light enable or
					// disable video streams due to video toggling actions. It occurs for example when everybody is
					// using a Grid layout and one participants enables its video stream with direction either SendOnly
					// or SendRecv.
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

		if (confAddr && fconference) {
			linphone_conference_terminate(fconference);
		}

		for (auto mgr : conferenceMgrs) {
			// Wait for all conferences to be terminated
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminationPending, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1,
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
				BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(call_logs),
				                (((mgr == marie.getCMgr()) || (mgr == laure.getCMgr())) ? 3 : 2), unsigned int, "%u");

				bctbx_list_t *mgr_focus_call_log =
				    linphone_core_get_call_history_2(mgr->lc, focus.getCMgr()->identity, mgr->identity);
				BC_ASSERT_PTR_NOT_NULL(mgr_focus_call_log);
				if (mgr_focus_call_log) {
					BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(mgr_focus_call_log), (mgr == laure.getCMgr()) ? 2 : 1,
					                unsigned int, "%u");
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

} // namespace LinphoneTest
