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

#include "liblinphone_tester.h"
#include "local_conference_tester_functions.h"
#include "shared_tester_functions.h"

namespace LinphoneTest {

static void create_conference_dial_out_base(bool_t send_ics,
                                            LinphoneConferenceLayout layout,
                                            LinphoneVideoActivationPolicy *pol,
                                            bool_t enable_stun,
                                            bool_t enable_ice,
                                            LinphoneConferenceParticipantListType participant_list_type,
                                            bool_t accept,
                                            bool_t participant_codec_mismatch,
                                            LinphoneConferenceSecurityLevel security_level) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity());
		ClientConference pauline("pauline_rc", focus.getIdentity());
		ClientConference laure("laure_tcp_rc", focus.getIdentity());
		ClientConference michelle("michelle_rc", focus.getIdentity());
		ClientConference berthe("berthe_rc", focus.getIdentity());

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

			enable_stun_in_core(mgr, enable_stun, enable_ice);
			linphone_core_manager_wait_for_stun_resolution(mgr);

			coresList = bctbx_list_append(coresList, mgr->lc);
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

		std::map<LinphoneCoreManager *, LinphoneParticipantRole> participantList;
		LinphoneParticipantRole role = LinphoneParticipantRoleUnknown;
		for (auto &p : participants) {
			participantList.insert(std::make_pair(p, role));
			if (role == LinphoneParticipantRoleSpeaker) {
				role = LinphoneParticipantRoleListener;
			} else if (role == LinphoneParticipantRoleListener) {
				role = LinphoneParticipantRoleUnknown;
			} else if (role == LinphoneParticipantRoleUnknown) {
				role = LinphoneParticipantRoleSpeaker;
			}
		}
		LinphoneAddress *confAddr = create_conference_on_server(focus, marie, participantList, -1, -1, initialSubject,
		                                                        description, send_ics, security_level);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		// Chat room creation to send ICS
		int marie_conferences = ((send_ics) ? 1 : 0) + 1;
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated,
		                             marie_conferences, liblinphone_tester_sip_timeout));

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
					BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(focus_call_log), 1, unsigned int, "%u");
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
					BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(member_call_log), 1, unsigned int, "%u");
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
		                             marie_stat.number_of_LinphoneSubscriptionOutgoingProgress + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
		                             marie_stat.number_of_LinphoneSubscriptionActive + 1,
		                             liblinphone_tester_sip_timeout));

		if (BC_ASSERT_PTR_NOT_NULL(oconference)) {
			BC_ASSERT_EQUAL(linphone_conference_get_participant_count(oconference), 4, int, "%0d");
			bctbx_list_t *devices = linphone_conference_get_participant_device_list(oconference);
			BC_ASSERT_EQUAL(bctbx_list_size(devices), members.size(), size_t, "%zu");
			if (devices) {
				bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
			}
		}

		if (confAddr) {
			for (auto mgr : participants) {
				// Encryption is None because we haven't received yet the NOTIFY full stae with this information
				check_conference_info(mgr, confAddr, marie.getCMgr(), (send_ics) ? 4 : 5, 0, 0, initialSubject,
				                      (accept && send_ics) ? description : NULL, 0, LinphoneConferenceInfoStateNew,
				                      LinphoneConferenceSecurityLevelNone);

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

		int participant_conference_info_participants = 5;
		if (accept) {
			int participant_no = static_cast<int>(participants.size());
			for (auto mgr : participants) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
				                             liblinphone_tester_sip_timeout));
				check_conference_info(mgr, confAddr, marie.getCMgr(), participant_conference_info_participants, 0, 0,
				                      initialSubject, (send_ics) ? description : NULL, 0,
				                      LinphoneConferenceInfoStateNew, security_level);
				LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
				BC_ASSERT_PTR_NOT_NULL(pcall);
				if (pcall) {
					LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
					BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
					LinphoneConferenceInfo *call_log_info = linphone_call_log_get_conference_info(call_log);
					if (BC_ASSERT_PTR_NOT_NULL(call_log_info)) {
						BC_ASSERT_TRUE(linphone_address_weak_equal(
						    linphone_conference_info_get_organizer(call_log_info), marie.getCMgr()->identity));
						BC_ASSERT_TRUE(
						    linphone_address_weak_equal(linphone_conference_info_get_uri(call_log_info), confAddr));
						const bctbx_list_t *info_participants =
						    linphone_conference_info_get_participant_infos(call_log_info);
						// Original participants + Marie who joined the conference
						BC_ASSERT_EQUAL(bctbx_list_size(info_participants), participant_conference_info_participants,
						                size_t, "%zu");
						BC_ASSERT_GREATER_STRICT((long long)linphone_conference_info_get_date_time(call_log_info), 0,
						                         long long, "%lld");
						const int duration_m = linphone_conference_info_get_duration(call_log_info);
						BC_ASSERT_EQUAL(duration_m, 0, int, "%d");
						if (initialSubject) {
							BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_subject(call_log_info), initialSubject);
						} else {
							BC_ASSERT_PTR_NULL(linphone_conference_info_get_subject(call_log_info));
						}
						if (send_ics) {
							BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_description(call_log_info),
							                       description);
						} else {
							BC_ASSERT_PTR_NULL(linphone_conference_info_get_description(call_log_info));
						}
					}
				}
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
			                             marie_stat.number_of_LinphoneSubscriptionOutgoingProgress + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
			                             marie_stat.number_of_LinphoneSubscriptionActive + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyFullStateReceived,
			                             marie_stat.number_of_NotifyFullStateReceived + 1,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreated,
			                             focus_stat.number_of_LinphoneConferenceStateCreated + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionIncomingReceived,
			                             focus_stat.number_of_LinphoneSubscriptionIncomingReceived + participant_no,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
			                             focus_stat.number_of_LinphoneSubscriptionActive + participant_no,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
			                             focus_stat.number_of_participants_added + participant_no,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
			                             focus_stat.number_of_participant_devices_added + participant_no,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
			                             focus_stat.number_of_participant_devices_joined + participant_no,
			                             liblinphone_tester_sip_timeout));

			LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
			BC_ASSERT_PTR_NOT_NULL(fconference);

			std::map<LinphoneCoreManager *, LinphoneParticipantRole> memberList;
			for (const auto &member : members) {
				try {
					const auto &participantInfo = participantList.at(member);
					auto role = participantInfo;
					if (role == LinphoneParticipantRoleUnknown) {
						role = LinphoneParticipantRoleSpeaker;
					}
					memberList.insert(std::make_pair(member, role));
				} catch (std::out_of_range &) {
					if (member == marie.getCMgr()) {
						memberList.insert(std::make_pair(marie.getCMgr(), LinphoneParticipantRoleSpeaker));
					} else {
						ms_fatal("Unable to find active participant %s in the participant list",
						         linphone_core_get_identity(member->lc));
					}
				}
			}
			wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conferenceMgrs,
			                            focus.getCMgr(), memberList, confAddr, enable_video);

			// wait bit more to detect side effect if any
			CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(15), [] {
				return false;
			});

			for (auto mgr : conferenceMgrs) {
				LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
				LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
				linphone_address_unref(uri);
				BC_ASSERT_PTR_NOT_NULL(pconference);

				LinphoneVideoActivationPolicy *mgr_pol = linphone_core_get_video_activation_policy(mgr->lc);
				bool_t video_enabled =
				    !!((mgr == marie.getCMgr()) ? linphone_video_activation_policy_get_automatically_initiate(mgr_pol)
				                                : linphone_video_activation_policy_get_automatically_accept(mgr_pol));
				linphone_video_activation_policy_unref(mgr_pol);

				if (pconference) {
					int no_participants = 0;
					if (mgr == focus.getCMgr()) {
						no_participants = static_cast<int>(members.size());
						BC_ASSERT_FALSE(linphone_conference_is_in(pconference));
					} else {

						check_conference_info(mgr, confAddr, marie.getCMgr(), participant_conference_info_participants,
						                      0, 0, initialSubject,
						                      (send_ics || (mgr == marie.getCMgr())) ? description : NULL, 0,
						                      LinphoneConferenceInfoStateNew, security_level);

						no_participants = participant_no;
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
						bool_t enabled = !!((mgr == marie.getCMgr())
						                        ? linphone_video_activation_policy_get_automatically_initiate(pol)
						                        : linphone_video_activation_policy_get_automatically_accept(pol));
						linphone_video_activation_policy_unref(pol);

						size_t no_streams_audio = 0;
						size_t no_max_streams_audio =
						    (security_level == LinphoneConferenceSecurityLevelEndToEnd)
						        ? static_cast<size_t>(participant_conference_info_participants)
						        : 1;
						size_t no_streams_video = 0;
						size_t no_max_streams_video = 0;
						size_t no_streams_text = 0;

						LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
						BC_ASSERT_PTR_NOT_NULL(pcall);
						if (pcall) {
							no_streams_audio = compute_no_audio_streams(pcall, pconference);
							// Even if video is not enabled, the server will offer it and clients reject the video
							// stream if they do not want to send or receive it.
							no_streams_video = compute_no_video_streams(enable_video, pcall, pconference);
							no_max_streams_video = (enabled || (mgr == marie.getCMgr())) ? no_streams_video : 1;
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
								BC_ASSERT_TRUE(linphone_address_weak_equal(
								    linphone_conference_info_get_organizer(call_log_info), marie.getCMgr()->identity));
								BC_ASSERT_TRUE(linphone_address_weak_equal(
								    linphone_conference_info_get_uri(call_log_info), confAddr));

								const bctbx_list_t *info_participants =
								    linphone_conference_info_get_participant_infos(call_log_info);
								// Original participants + Marie who joined the conference
								BC_ASSERT_EQUAL(bctbx_list_size(info_participants),
								                participant_conference_info_participants, size_t, "%zu");

								BC_ASSERT_GREATER_STRICT(
								    (long long)linphone_conference_info_get_date_time(call_log_info), 0, long long,
								    "%lld");
								const int duration_m = linphone_conference_info_get_duration(call_log_info);
								BC_ASSERT_EQUAL(duration_m, 0, int, "%d");
								BC_ASSERT_EQUAL((int)linphone_conference_info_get_security_level(call_log_info),
								                (int)security_level, int, "%0d");
								if (initialSubject) {
									BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_subject(call_log_info),
									                       initialSubject);
								} else {
									BC_ASSERT_PTR_NULL(linphone_conference_info_get_subject(call_log_info));
								}
								if (send_ics || (mgr == marie.getCMgr())) {
									BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_description(call_log_info),
									                       description);
								} else {
									BC_ASSERT_PTR_NULL(linphone_conference_info_get_description(call_log_info));
								}
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
						check_conference_ssrc(fconference, pconference);
					}

					LinphoneConference *conference = linphone_core_search_conference_2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(conference);
					if (conference) {
						bctbx_list_t *devices = linphone_conference_get_participant_device_list(conference);
						for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
							LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
							// If we are currently carrying out checks on the conference server side, the must set the
							// value of video enabled flag for each participant. In fact each call session may have the
							// video enabled or not and this is taken into account to compute the video availablity
							// flag. Nonetheless, this is not required for the participants as they only have one call
							// session towards the conference server therefore we can reuse the value computed earlier
							// on.
							if (mgr == focus.getCMgr()) {
								if (linphone_address_weak_equal(marie.getIdentity().toC(),
								                                linphone_participant_device_get_address(d))) {
									// The organizer will not offer video streams in its INVITE to join a conference if
									// the policy doesn't allow it
									if (!initiate_video) {
										video_enabled = FALSE;
									} else {
										video_enabled = TRUE;
									}
								} else {
									// The participants will not accept video streams to answer the conference server
									// INVITE to join a conference if the policy doesn't allow it
									if (!accept_video) {
										video_enabled = FALSE;
									} else {
										video_enabled = TRUE;
									}
								}
							}

							if (enable_video) {
								if (linphone_conference_is_me(conference, linphone_participant_device_get_address(d))) {
									BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(
									                   d, LinphoneStreamTypeVideo) == video_enabled);
								} else {
									bool_t video_available =
									    linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo);
									LinphoneMediaDirection video_direction =
									    linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo);
									BC_ASSERT_TRUE(video_available ==
									               (((video_direction == LinphoneMediaDirectionSendOnly) ||
									                 (video_direction == LinphoneMediaDirectionSendRecv)) &&
									                video_enabled));
								}
							} else {
								BC_ASSERT_FALSE(
								    linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
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

				LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(pauline.getLc());
				bool_t enable = !!!linphone_video_activation_policy_get_automatically_accept(pol);
				linphone_video_activation_policy_unref(pol);

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
					BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateDeleted, 1,
					                             liblinphone_tester_sip_timeout));

					LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
					LinphoneConference *pconference =
					    linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
					BC_ASSERT_PTR_NULL(pconference);
					linphone_address_unref(uri);
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
			BC_ASSERT_TRUE(wait_for_list(coresList,
			                             &focus.getStats().number_of_LinphoneConferenceStateTerminationPending, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateTerminated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateDeleted, 1,
			                             liblinphone_tester_sip_timeout));
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
			check_conference_info(
			    mgr, confAddr, marie.getCMgr(), participant_conference_info_participants, 0, 0, initialSubject,
			    ((accept && send_ics) || (mgr == marie.getCMgr())) ? description : NULL, 0,
			    LinphoneConferenceInfoStateNew, (accept) ? security_level : LinphoneConferenceSecurityLevelNone);
		}

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
			return false;
		});

		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void create_simple_conference_dial_out(void) {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, FALSE);
	linphone_video_activation_policy_set_automatically_initiate(pol, FALSE);
	create_conference_dial_out_base(FALSE, LinphoneConferenceLayoutActiveSpeaker, pol, FALSE, FALSE,
	                                LinphoneConferenceParticipantListTypeClosed, TRUE, FALSE,
	                                LinphoneConferenceSecurityLevelNone);
	linphone_video_activation_policy_unref(pol);
}

static void create_simple_conference_dial_out_and_ics(void) {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
	create_conference_dial_out_base(TRUE, LinphoneConferenceLayoutGrid, pol, TRUE, TRUE,
	                                LinphoneConferenceParticipantListTypeOpen, TRUE, FALSE,
	                                LinphoneConferenceSecurityLevelNone);
	linphone_video_activation_policy_unref(pol);
}

static void create_simple_conference_dial_out_with_calls_declined(void) {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
	create_conference_dial_out_base(FALSE, LinphoneConferenceLayoutGrid, pol, TRUE, TRUE,
	                                LinphoneConferenceParticipantListTypeOpen, FALSE, FALSE,
	                                LinphoneConferenceSecurityLevelNone);
	linphone_video_activation_policy_unref(pol);
}

static void create_simple_point_to_point_encrypted_conference_dial_out(void) {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
	create_conference_dial_out_base(FALSE, LinphoneConferenceLayoutActiveSpeaker, pol, FALSE, FALSE,
	                                LinphoneConferenceParticipantListTypeClosed, TRUE, FALSE,
	                                LinphoneConferenceSecurityLevelPointToPoint);
	linphone_video_activation_policy_unref(pol);
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

static void create_simple_conference_dial_out_participant_codec_mismatch(void) {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, FALSE);
	linphone_video_activation_policy_set_automatically_initiate(pol, FALSE);
	create_conference_dial_out_base(FALSE, LinphoneConferenceLayoutActiveSpeaker, pol, FALSE, FALSE,
	                                LinphoneConferenceParticipantListTypeClosed, TRUE, TRUE,
	                                LinphoneConferenceSecurityLevelNone);
	linphone_video_activation_policy_unref(pol);
}

static void create_simple_conference_dial_out_with_video_not_accepted(void) {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, FALSE);
	linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);
	create_conference_dial_out_base(FALSE, LinphoneConferenceLayoutActiveSpeaker, pol, FALSE, FALSE,
	                                LinphoneConferenceParticipantListTypeClosed, TRUE, FALSE,
	                                LinphoneConferenceSecurityLevelNone);
	linphone_video_activation_policy_unref(pol);
}

static void create_simple_conference_dial_out_with_video_not_initiated(void) {
	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(pol, FALSE);
	create_conference_dial_out_base(FALSE, LinphoneConferenceLayoutGrid, pol, FALSE, FALSE,
	                                LinphoneConferenceParticipantListTypeClosed, TRUE, FALSE,
	                                LinphoneConferenceSecurityLevelNone);
	linphone_video_activation_policy_unref(pol);
}

static void create_simple_conference_dial_out_organizer_codec_mismatch(void) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity());
		ClientConference pauline("pauline_rc", focus.getIdentity());
		ClientConference laure("laure_tcp_rc", focus.getIdentity());
		ClientConference michelle("michelle_rc", focus.getIdentity());
		ClientConference berthe("berthe_rc", focus.getIdentity());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(berthe);

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t *coresList = NULL;

		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(),
		                 berthe.getCMgr()}) {
			if (mgr == marie.getCMgr()) {
				disable_all_audio_codecs_except_one(mgr->lc, "pcmu", -1);
			} else {
				disable_all_audio_codecs_except_one(mgr->lc, "pcma", -1);
			}

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
		stats marie_stat = marie.getStats();

		std::list<LinphoneCoreManager *> conferenceMgrs{focus.getCMgr(), marie.getCMgr(),    pauline.getCMgr(),
		                                                laure.getCMgr(), michelle.getCMgr(), berthe.getCMgr()};
		std::list<LinphoneCoreManager *> members{marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(),
		                                         michelle.getCMgr(), berthe.getCMgr()};
		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(),
		                                              berthe.getCMgr()};

		const char *initialSubject = "Schedule of the trip towards the top of Europe";
		const char *description = "To the Goutier mountain hut!!!! :-)";
		LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;

		std::map<LinphoneCoreManager *, LinphoneParticipantRole> participantList;
		LinphoneParticipantRole role = LinphoneParticipantRoleSpeaker;
		for (auto &p : participants) {
			participantList.insert(std::make_pair(p, role));
			role = (role == LinphoneParticipantRoleSpeaker) ? LinphoneParticipantRoleListener
			                                                : LinphoneParticipantRoleSpeaker;
		}
		LinphoneAddress *confAddr = create_conference_on_server(focus, marie, participantList, -1, -1, initialSubject,
		                                                        description, TRUE, security_level);
		BC_ASSERT_PTR_NULL(confAddr);

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneConferenceStateCreationFailed,
		                             focus_stat.number_of_LinphoneConferenceStateCreationFailed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated,
		                              marie_stat.number_of_LinphoneConferenceStateCreated + 1, 3000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallIncomingReceived,
		                              focus_stat.number_of_LinphoneCallIncomingReceived + 1, 1000));

		LinphoneConference *fconference = linphone_core_search_conference_2(focus.getLc(), confAddr);
		BC_ASSERT_PTR_NULL(fconference);

		if (confAddr) linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void create_simple_conference_dial_out_with_some_calls_declined_base(LinphoneReason reason) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity());
		ClientConference pauline("pauline_rc", focus.getIdentity());
		ClientConference laure("laure_tcp_rc", focus.getIdentity());
		ClientConference michelle("michelle_rc", focus.getIdentity());
		ClientConference berthe("berthe_rc", focus.getIdentity());

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

			enable_stun_in_core(mgr, TRUE, TRUE);
			linphone_core_manager_wait_for_stun_resolution(mgr);

			coresList = bctbx_list_append(coresList, mgr->lc);
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
		LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;

		std::map<LinphoneCoreManager *, LinphoneParticipantRole> participantList;
		LinphoneParticipantRole role = LinphoneParticipantRoleSpeaker;
		for (auto &p : participants) {
			participantList.insert(std::make_pair(p, role));
			role = (role == LinphoneParticipantRoleSpeaker) ? LinphoneParticipantRoleListener
			                                                : LinphoneParticipantRoleSpeaker;
		}
		LinphoneAddress *confAddr = create_conference_on_server(focus, marie, participantList, -1, -1, initialSubject,
		                                                        description, FALSE, security_level);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

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
		                             marie_stat.number_of_LinphoneSubscriptionOutgoingProgress + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneSubscriptionActive,
		                             marie_stat.number_of_LinphoneSubscriptionActive + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_NotifyFullStateReceived,
		                             marie_stat.number_of_NotifyFullStateReceived + 1, liblinphone_tester_sip_timeout));

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
			for (auto mgr : participants) {
				check_conference_info(mgr, confAddr, marie.getCMgr(), 5, 0, 0, initialSubject, NULL, 0,
				                      LinphoneConferenceInfoStateNew, security_level);

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
			}
		}

		for (auto mgr : active_participants) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallUpdatedByRemote, 1, 20000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 2,
			                             liblinphone_tester_sip_timeout));

			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(
			    wait_for_list(coresList, &mgr->stat.number_of_NotifyReceived, 1, liblinphone_tester_sip_timeout));

			check_conference_info(mgr, confAddr, marie.getCMgr(), 5, 0, 0, initialSubject, NULL, 0,
			                      LinphoneConferenceInfoStateNew, security_level);

			LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pcall);
			if (pcall) {
				LinphoneCallLog *call_log = linphone_call_get_call_log(pcall);
				BC_ASSERT_TRUE(linphone_call_log_was_conference(call_log));
				LinphoneConferenceInfo *call_log_info = linphone_call_log_get_conference_info(call_log);
				if (BC_ASSERT_PTR_NOT_NULL(call_log_info)) {
					BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_conference_info_get_organizer(call_log_info),
					                                           marie.getCMgr()->identity));
					BC_ASSERT_TRUE(
					    linphone_address_weak_equal(linphone_conference_info_get_uri(call_log_info), confAddr));

					const bctbx_list_t *info_participants =
					    linphone_conference_info_get_participant_infos(call_log_info);
					// Original participants + Marie who joined the conference
					BC_ASSERT_EQUAL(bctbx_list_size(info_participants), 5, size_t, "%zu");

					BC_ASSERT_GREATER_STRICT((long long)linphone_conference_info_get_date_time(call_log_info), 0,
					                         long long, "%lld");
					const int duration_m = linphone_conference_info_get_duration(call_log_info);
					BC_ASSERT_EQUAL(duration_m, 0, int, "%d");
					BC_ASSERT_EQUAL((int)linphone_conference_info_get_security_level(call_log_info),
					                (int)security_level, int, "%0d");
					if (initialSubject) {
						BC_ASSERT_STRING_EQUAL(linphone_conference_info_get_subject(call_log_info), initialSubject);
					} else {
						BC_ASSERT_PTR_NULL(linphone_conference_info_get_subject(call_log_info));
					}
					BC_ASSERT_PTR_NULL(linphone_conference_info_get_description(call_log_info));
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
		                                 static_cast<int>(active_participants.size()) + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneSubscriptionActive,
		                             focus_stat.number_of_LinphoneSubscriptionActive +
		                                 static_cast<int>(active_participants.size()) + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participants_added,
		                             focus_stat.number_of_participants_added + 5, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_added,
		                             focus_stat.number_of_participant_devices_added + 5,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_joined,
		                             focus_stat.number_of_participant_devices_joined +
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

		std::map<LinphoneCoreManager *, LinphoneParticipantRole> memberList;
		for (const auto &member : all_active_participants) {
			try {
				const auto &participantInfo = participantList.at(member);
				memberList.insert(std::make_pair(member, participantInfo));
			} catch (std::out_of_range &) {
				if (member == marie.getCMgr()) {
					memberList.insert(std::make_pair(marie.getCMgr(), LinphoneParticipantRoleSpeaker));
				} else {
					ms_fatal("Unable to find active participant %s in the participant list",
					         linphone_core_get_identity(member->lc));
				}
			}
		}
		wait_for_conference_streams({focus, marie, pauline, laure, michelle, berthe}, conference_members,
		                            focus.getCMgr(), memberList, confAddr, TRUE);

		// wait bit more to detect side effect if any
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

					LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(mgr->lc);
					bool_t enabled = !!linphone_video_activation_policy_get_automatically_initiate(pol);
					linphone_video_activation_policy_unref(pol);

					size_t no_streams_audio = 0;
					size_t no_streams_video = (enabled) ? (static_cast<int>(all_active_participants.size()) + 1) : 0;
					size_t no_streams_text = 0;

					LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						no_streams_audio = compute_no_audio_streams(pcall, pconference);
						_linphone_call_check_max_nb_streams(pcall, no_streams_audio, no_streams_video, no_streams_text);
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
						_linphone_call_check_max_nb_streams(ccall, no_streams_audio, no_streams_video, no_streams_text);
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
				LinphoneParticipant *me = linphone_conference_get_me(pconference);
				BC_ASSERT_TRUE(linphone_participant_is_admin(me) ==
				               ((mgr == marie.getCMgr()) || (mgr == focus.getCMgr())));
				BC_ASSERT_TRUE(linphone_address_weak_equal(linphone_participant_get_address(me), mgr->identity));
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
					check_conference_ssrc(fconference, pconference);
				}

				bctbx_list_t *devices = linphone_conference_get_participant_device_list(pconference);
				BC_ASSERT_EQUAL(bctbx_list_size(devices), all_active_participants.size(), size_t, "%zu");
				for (bctbx_list_t *itd = devices; itd; itd = bctbx_list_next(itd)) {
					LinphoneParticipantDevice *d = (LinphoneParticipantDevice *)bctbx_list_get_data(itd);
					if (linphone_conference_is_me(pconference, linphone_participant_device_get_address(d))) {
						BC_ASSERT_TRUE(linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
					} else {
						BC_ASSERT_TRUE(
						    linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) ==
						    (linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) ==
						     LinphoneMediaDirectionSendRecv));
					}
				}

				if (devices) {
					bctbx_list_free_with_data(devices, (void (*)(void *))linphone_participant_device_unref);
				}
			}
		}

		LinphoneCall *pauline_call = linphone_core_get_call_by_remote_address2(pauline.getLc(), confAddr);
		BC_ASSERT_PTR_NOT_NULL(pauline_call);

		Address paulineAddr = pauline.getIdentity();
		LinphoneCall *focus_call = linphone_core_get_call_by_remote_address2(focus.getLc(), paulineAddr.toC());
		BC_ASSERT_PTR_NOT_NULL(focus_call);

		LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(pauline.getLc());
		bool_t enable = !!!linphone_video_activation_policy_get_automatically_initiate(pol);
		linphone_video_activation_policy_unref(pol);

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
					if (enable) {
						if (linphone_conference_is_me(paulineConference, linphone_participant_device_get_address(d))) {
							BC_ASSERT_TRUE(
							    linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo));
						} else {
							BC_ASSERT_TRUE(
							    linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) ==
							    (linphone_participant_device_get_stream_capability(d, LinphoneStreamTypeVideo) ==
							     LinphoneMediaDirectionSendRecv));
						}
					} else {
						BC_ASSERT_TRUE(
						    linphone_participant_device_get_stream_availability(d, LinphoneStreamTypeVideo) == enable);
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
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionTerminated, 1,
				                             liblinphone_tester_sip_timeout));
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
		                                 static_cast<int>(all_active_participants.size()),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(
		    wait_for_list(coresList, &focus.getStats().number_of_participants_removed,
		                  focus_stat.number_of_participants_removed + static_cast<int>(all_active_participants.size()),
		                  liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_participant_devices_removed,
		                             focus_stat.number_of_participant_devices_removed +
		                                 static_cast<int>(all_active_participants.size()),
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminationPending,
		                focus_stat.number_of_LinphoneConferenceStateTerminationPending, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateTerminated,
		                focus_stat.number_of_LinphoneConferenceStateTerminated, int, "%d");
		BC_ASSERT_EQUAL(focus.getStats().number_of_LinphoneConferenceStateDeleted,
		                focus_stat.number_of_LinphoneConferenceStateDeleted, int, "%d");

		BC_ASSERT_EQUAL(focus.getStats().number_of_participant_devices_joined,
		                static_cast<int>(all_active_participants.size()), int, "%d");
		BC_ASSERT_EQUAL(marie.getStats().number_of_participant_devices_joined,
		                static_cast<int>(all_active_participants.size()), int, "%d");

		for (auto mgr : {focus.getCMgr()}) {
			LinphoneConference *pconference = linphone_core_search_conference_2(mgr->lc, confAddr);
			BC_ASSERT_PTR_NOT_NULL(pconference);
			if (pconference) {
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference),
				                static_cast<int>((reason == LinphoneReasonBusy) ? declining_participants.size() : 0),
				                int, "%0d");
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

		for (auto mgr : {marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(), berthe.getCMgr()}) {
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
			check_conference_info(mgr, confAddr, marie.getCMgr(), 5, 0, 0, initialSubject,
			                      (mgr == marie.getCMgr()) ? description : NULL, 0, LinphoneConferenceInfoStateNew,
			                      security_level);
		}

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure, michelle, berthe}).waitUntil(chrono::seconds(2), [] {
			return false;
		});

		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

static void create_simple_conference_dial_out_with_some_calls_declined(void) {
	create_simple_conference_dial_out_with_some_calls_declined_base(LinphoneReasonDeclined);
}

static void create_simple_conference_dial_out_with_some_calls_busy(void) {
	create_simple_conference_dial_out_with_some_calls_declined_base(LinphoneReasonBusy);
}

static void create_simple_conference_dial_out_with_late_participant_addition(void) {
	create_conference_with_late_participant_addition_base(-1, -1, LinphoneConferenceLayoutActiveSpeaker,
	                                                      LinphoneConferenceParticipantListTypeOpen, TRUE, TRUE,
	                                                      LinphoneConferenceSecurityLevelNone);
}

static void create_simple_conference_dial_out_with_many_late_participant_additions(void) {
	create_conference_with_late_participant_addition_base(-1, -1, LinphoneConferenceLayoutGrid,
	                                                      LinphoneConferenceParticipantListTypeOpen, TRUE, FALSE,
	                                                      LinphoneConferenceSecurityLevelNone);
}

static void organizer_creates_two_dialout_conferences(void) {
	two_overlapping_conferences_base(TRUE, TRUE);
}

static void two_overlapping_dialout_conferences_from_different_organizers(void) {
	two_overlapping_conferences_base(FALSE, TRUE);
}

static void create_simple_conference_merging_calls(void) {
	create_simple_conference_merging_calls_base(FALSE, LinphoneConferenceLayoutActiveSpeaker, FALSE, FALSE, FALSE,
	                                            LinphoneConferenceSecurityLevelNone);
}

static void create_simple_conference_merging_calls_with_video_toggling(void) {
	create_simple_conference_merging_calls_base(FALSE, LinphoneConferenceLayoutGrid, TRUE, TRUE, TRUE,
	                                            LinphoneConferenceSecurityLevelNone);
}

static void create_dial_out_conference_with_active_call(void) {
	create_conference_with_active_call_base(TRUE);
}

static void simple_dial_out_conference_with_no_payloads(void) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getIdentity());
		ClientConference pauline("pauline_rc", focus.getIdentity());
		ClientConference laure("laure_tcp_rc", focus.getIdentity());
		ClientConference michelle("michelle_rc", focus.getIdentity());
		ClientConference berthe("berthe_rc", focus.getIdentity());

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(berthe);

		const bctbx_list_t *elem = linphone_core_get_audio_codecs(pauline.getLc());
		disable_all_codecs(elem, pauline.getCMgr());

		setup_conference_info_cbs(marie.getCMgr());

		bctbx_list_t *coresList = NULL;

		linphone_core_set_avpf_mode(focus.getCMgr()->lc, LinphoneAVPFEnabled);
		for (auto mgr : {focus.getCMgr(), marie.getCMgr(), pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(),
		                 berthe.getCMgr()}) {
			coresList = bctbx_list_append(coresList, mgr->lc);
		}

		linphone_core_set_file_transfer_server(marie.getLc(), file_transfer_url);

		stats focus_stat = focus.getStats();

		std::list<LinphoneCoreManager *> participants{pauline.getCMgr(), laure.getCMgr(), michelle.getCMgr(),
		                                              berthe.getCMgr()};

		const char *initialSubject = "Schedule of the trip towards the top of Europe";
		const char *description = "To the top of the Mont Blanc!!!! :-)";

		LinphoneConferenceSecurityLevel security_level = LinphoneConferenceSecurityLevelNone;
		std::map<LinphoneCoreManager *, LinphoneParticipantRole> participantList;
		LinphoneParticipantRole role = LinphoneParticipantRoleSpeaker;
		for (auto &p : participants) {
			participantList.insert(std::make_pair(p, role));
			role = (role == LinphoneParticipantRoleSpeaker) ? LinphoneParticipantRoleListener
			                                                : LinphoneParticipantRoleSpeaker;
		}
		LinphoneAddress *confAddr = create_conference_on_server(focus, marie, participantList, -1, -1, initialSubject,
		                                                        description, FALSE, security_level);
		BC_ASSERT_PTR_NOT_NULL(confAddr);

		LinphoneConference *fconference =
		    (confAddr) ? linphone_core_search_conference_2(focus.getLc(), confAddr) : NULL;

		// Chat room creation to send ICS
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneConferenceStateCreated, 1,
		                             liblinphone_tester_sip_timeout));

		std::list<LinphoneCoreManager *> mgr_in_conference{marie.getCMgr(), laure.getCMgr(), michelle.getCMgr(),
		                                                   berthe.getCMgr()};

		BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallOutgoingInit,
		                             focus_stat.number_of_LinphoneCallOutgoingInit + 5,
		                             liblinphone_tester_sip_timeout));
		for (auto mgr : participants) {
			if (mgr == pauline.getCMgr()) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallError,
				                             focus_stat.number_of_LinphoneCallError + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &focus.getStats().number_of_LinphoneCallReleased,
				                             focus_stat.number_of_LinphoneCallReleased + 1,
				                             liblinphone_tester_sip_timeout));
			} else {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallIncomingReceived, 1,
				                             liblinphone_tester_sip_timeout));
			}
		}

		if (confAddr) {
			for (auto mgr : participants) {
				LinphoneCall *pcall = linphone_core_get_call_by_remote_address2(mgr->lc, confAddr);
				if (mgr == pauline.getCMgr()) {
					BC_ASSERT_PTR_NULL(pcall);
				} else {
					BC_ASSERT_PTR_NOT_NULL(pcall);
					if (pcall) {
						linphone_call_accept(pcall);
					}
				}
			}
		}

		for (auto mgr : participants) {
			if (mgr != pauline.getCMgr()) {
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneCallStreamsRunning, 2,
				                             liblinphone_tester_sip_timeout));

				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneConferenceStateCreated, 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(
				    wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress, 1, 5000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_LinphoneSubscriptionActive, 1, 5000));
				BC_ASSERT_TRUE(wait_for_list(coresList, &mgr->stat.number_of_NotifyFullStateReceived, 1,
				                             liblinphone_tester_sip_timeout));
			}
		}

		focus_stat = focus.getStats();
		for (auto mgr : {marie.getCMgr(), laure.getCMgr(), michelle.getCMgr(), berthe.getCMgr()}) {
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

				LinphoneAddress *uri = linphone_address_new(linphone_core_get_identity(mgr->lc));
				LinphoneConference *pconference = linphone_core_search_conference(mgr->lc, NULL, uri, confAddr, NULL);
				BC_ASSERT_PTR_NULL(pconference);
				linphone_address_unref(uri);
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
				BC_ASSERT_EQUAL(linphone_conference_get_participant_count(pconference), 0, int, "%0d");
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

		linphone_address_unref(confAddr);
		bctbx_list_free(coresList);
	}
}

} // namespace LinphoneTest

static test_t local_conference_inpromptu_conference_tests[] = {
    TEST_NO_TAG("Create simple dial out conference", LinphoneTest::create_simple_conference_dial_out),
    TEST_NO_TAG("Create simple point-to-point encrypted dial out conference",
                LinphoneTest::create_simple_point_to_point_encrypted_conference_dial_out),
    TEST_NO_TAG("Create simple end-to-end encrypted dial out conference",
                LinphoneTest::create_simple_end_to_end_encrypted_conference_dial_out),
    TEST_NO_TAG("Create simple dial out conference and ICS sent",
                LinphoneTest::create_simple_conference_dial_out_and_ics),
    TEST_NO_TAG("Create simple dial out conference with late participant addition",
                LinphoneTest::create_simple_conference_dial_out_with_late_participant_addition),
    TEST_NO_TAG("Create simple dial out conference with many late participant additions",
                LinphoneTest::create_simple_conference_dial_out_with_many_late_participant_additions),
    TEST_NO_TAG("Create simple conference by merging calls", LinphoneTest::create_simple_conference_merging_calls),
    TEST_ONE_TAG("Create simple conference by merging calls with video toggling",
                 LinphoneTest::create_simple_conference_merging_calls_with_video_toggling,
                 "LeaksMemory"), /* because of aborted calls*/
    TEST_NO_TAG("Create dial out conference with active call",
                LinphoneTest::create_dial_out_conference_with_active_call),
    TEST_NO_TAG("Organizer creates 2 dialout conferences", LinphoneTest::organizer_creates_two_dialout_conferences),
    TEST_NO_TAG("2 overlapping dialout conferences from different organizers",
                LinphoneTest::two_overlapping_dialout_conferences_from_different_organizers)};

static test_t local_conference_inpromptu_mismatch_conference_tests[] = {
    TEST_NO_TAG("Create simple dial out conference with calls declined",
                LinphoneTest::create_simple_conference_dial_out_with_calls_declined),
    TEST_NO_TAG("Create simple dial out conference with some calls declined",
                LinphoneTest::create_simple_conference_dial_out_with_some_calls_declined),
    TEST_NO_TAG("Create simple dial out conference with some calls busy",
                LinphoneTest::create_simple_conference_dial_out_with_some_calls_busy),
    TEST_NO_TAG("Create simple dial out conference with participant codec mismatch",
                LinphoneTest::create_simple_conference_dial_out_participant_codec_mismatch),
    TEST_NO_TAG("Create simple dial out conference with organizer codec mismatch",
                LinphoneTest::create_simple_conference_dial_out_organizer_codec_mismatch),
    TEST_NO_TAG("Create simple dial out conference with video not initiated",
                LinphoneTest::create_simple_conference_dial_out_with_video_not_initiated),
    TEST_NO_TAG("Create simple dial out conference with video not accepted",
                LinphoneTest::create_simple_conference_dial_out_with_video_not_accepted),
    TEST_NO_TAG("Simple dial out conference with no payloads",
                LinphoneTest::simple_dial_out_conference_with_no_payloads)};

test_suite_t local_conference_test_suite_inpromptu_conference = {
    "Local conference tester (Inpromptu Conference)",
    NULL,
    NULL,
    liblinphone_tester_before_each,
    liblinphone_tester_after_each,
    sizeof(local_conference_inpromptu_conference_tests) / sizeof(local_conference_inpromptu_conference_tests[0]),
    local_conference_inpromptu_conference_tests,
    0};

test_suite_t local_conference_test_suite_inpromptu_mismatch_conference = {
    "Local conference tester (Inpromptu Conference with mismatch)",
    NULL,
    NULL,
    liblinphone_tester_before_each,
    liblinphone_tester_after_each,
    sizeof(local_conference_inpromptu_mismatch_conference_tests) /
        sizeof(local_conference_inpromptu_mismatch_conference_tests[0]),
    local_conference_inpromptu_mismatch_conference_tests,
    0};
