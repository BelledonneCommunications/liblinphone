/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
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

#include "linphone/api/c-conference-scheduler.h"
#include "account/account.h"
#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "conference/conference-scheduler.h"
#include "core/core.h"
#include "linphone/api/c-address.h"
#include "linphone/wrapper_utils.h"
#include "private_functions.h"

// =============================================================================

using namespace LinphonePrivate;

LinphoneConferenceScheduler *linphone_conference_scheduler_ref(LinphoneConferenceScheduler *conference_scheduler) {
	ConferenceScheduler::toCpp(conference_scheduler)->ref();
	return conference_scheduler;
}

void linphone_conference_scheduler_unref(LinphoneConferenceScheduler *conference_scheduler) {
	ConferenceSchedulerLogContextualizer logContextualizer(conference_scheduler);
	ConferenceScheduler::toCpp(conference_scheduler)->unref();
}

LinphoneCore *linphone_conference_scheduler_get_core(const LinphoneConferenceScheduler *conference_scheduler) {
	return ConferenceScheduler::toCpp(conference_scheduler)->getCore()->getCCore();
}

void linphone_conference_scheduler_set_account(LinphoneConferenceScheduler *conference_scheduler,
                                               LinphoneAccount *account) {
	ConferenceSchedulerLogContextualizer logContextualizer(conference_scheduler);
	ConferenceScheduler::toCpp(conference_scheduler)
	    ->setAccount(LinphonePrivate::Account::toCpp(account)->getSharedFromThis());
}

LinphoneAccount *linphone_conference_scheduler_get_account(const LinphoneConferenceScheduler *conference_scheduler) {
	auto account = ConferenceScheduler::toCpp(conference_scheduler)->getAccount();
	return account != nullptr ? account->toC() : NULL;
}

const LinphoneConferenceInfo *
linphone_conference_scheduler_get_info(const LinphoneConferenceScheduler *conference_scheduler) {
	ConferenceSchedulerLogContextualizer logContextualizer(conference_scheduler);
	return ConferenceScheduler::toCpp(conference_scheduler)->getInfo()
	           ? ConferenceScheduler::toCpp(conference_scheduler)->getInfo()->toC()
	           : NULL;
}

void linphone_conference_scheduler_set_info(LinphoneConferenceScheduler *conference_scheduler,
                                            LinphoneConferenceInfo *conference_info) {
	ConferenceSchedulerLogContextualizer logContextualizer(conference_scheduler);
	ConferenceScheduler::toCpp(conference_scheduler)
	    ->setInfo(ConferenceInfo::toCpp(conference_info)->getSharedFromThis());
}

void linphone_conference_scheduler_cancel_conference(LinphoneConferenceScheduler *conference_scheduler,
                                                     LinphoneConferenceInfo *conference_info) {
	ConferenceSchedulerLogContextualizer logContextualizer(conference_scheduler);
	ConferenceScheduler::toCpp(conference_scheduler)
	    ->cancelConference(ConferenceInfo::toCpp(conference_info)->getSharedFromThis());
}

void linphone_conference_scheduler_send_invitations(LinphoneConferenceScheduler *conference_scheduler,
                                                    LinphoneChatRoomParams *chat_room_params) {
	ConferenceSchedulerLogContextualizer logContextualizer(conference_scheduler);
	ConferenceScheduler::toCpp(conference_scheduler)
	    ->sendInvitations(ConferenceParams::toCpp(chat_room_params)->getSharedFromThis());
}

void linphone_conference_scheduler_send_invitations_2(LinphoneConferenceScheduler *conference_scheduler,
                                                      LinphoneConferenceParams *conference_params) {
	ConferenceSchedulerLogContextualizer logContextualizer(conference_scheduler);
	ConferenceScheduler::toCpp(conference_scheduler)
	    ->sendInvitations(ConferenceParams::toCpp(conference_params)->getSharedFromThis());
}

void linphone_conference_scheduler_notify_state_changed(LinphoneConferenceScheduler *conference_scheduler,
                                                        LinphoneConferenceSchedulerState state) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ConferenceScheduler, ConferenceScheduler::toCpp(conference_scheduler),
	                                  linphone_conference_scheduler_cbs_get_state_changed, state);
}

void linphone_conference_scheduler_notify_invitations_sent(LinphoneConferenceScheduler *conference_scheduler,
                                                           const bctbx_list_t *failed_invites) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ConferenceScheduler, ConferenceScheduler::toCpp(conference_scheduler),
	                                  linphone_conference_scheduler_cbs_get_invitations_sent, failed_invites);
}

void linphone_conference_scheduler_add_callbacks(LinphoneConferenceScheduler *conference_scheduler,
                                                 LinphoneConferenceSchedulerCbs *cbs) {
	ConferenceScheduler::toCpp(conference_scheduler)
	    ->addCallbacks(ConferenceSchedulerCbs::toCpp(cbs)->getSharedFromThis());
}

void linphone_conference_scheduler_remove_callbacks(LinphoneConferenceScheduler *conference_scheduler,
                                                    LinphoneConferenceSchedulerCbs *cbs) {
	ConferenceScheduler::toCpp(conference_scheduler)
	    ->removeCallbacks(ConferenceSchedulerCbs::toCpp(cbs)->getSharedFromThis());
}

LinphoneConferenceSchedulerCbs *
linphone_conference_scheduler_get_current_callbacks(const LinphoneConferenceScheduler *conference_scheduler) {
	return ConferenceScheduler::toCpp(conference_scheduler)->getCurrentCallbacks()->toC();
}

void linphone_conference_scheduler_set_current_callbacks(LinphoneConferenceScheduler *conference_scheduler,
                                                         LinphoneConferenceSchedulerCbs *cbs) {
	ConferenceScheduler::toCpp(conference_scheduler)
	    ->setCurrentCallbacks(ConferenceSchedulerCbs::toCpp(cbs)->getSharedFromThis());
}

const bctbx_list_t *
linphone_conference_scheduler_get_callbacks_list(const LinphoneConferenceScheduler *conference_scheduler) {
	return ConferenceScheduler::toCpp(conference_scheduler)->getCCallbacksList();
}

LinphoneConferenceSchedulerCbs *linphone_conference_scheduler_cbs_new(void) {
	return ConferenceSchedulerCbs::createCObject();
}

LinphoneConferenceSchedulerCbs *linphone_conference_scheduler_cbs_ref(LinphoneConferenceSchedulerCbs *cbs) {
	ConferenceSchedulerCbs::toCpp(cbs)->ref();
	return cbs;
}

void linphone_conference_scheduler_cbs_unref(LinphoneConferenceSchedulerCbs *cbs) {
	ConferenceSchedulerCbs::toCpp(cbs)->unref();
}

void *linphone_conference_scheduler_cbs_get_user_data(const LinphoneConferenceSchedulerCbs *cbs) {
	return ConferenceSchedulerCbs::toCpp(cbs)->getUserData();
}

void linphone_conference_scheduler_cbs_set_user_data(LinphoneConferenceSchedulerCbs *cbs, void *ud) {
	ConferenceSchedulerCbs::toCpp(cbs)->setUserData(ud);
}

LinphoneConferenceSchedulerCbsStateChangedCb
linphone_conference_scheduler_cbs_get_state_changed(const LinphoneConferenceSchedulerCbs *cbs) {
	return ConferenceSchedulerCbs::toCpp(cbs)->getStateChanged();
}

void linphone_conference_scheduler_cbs_set_state_changed(LinphoneConferenceSchedulerCbs *cbs,
                                                         LinphoneConferenceSchedulerCbsStateChangedCb cb) {
	ConferenceSchedulerCbs::toCpp(cbs)->setStateChanged(cb);
}

LinphoneConferenceSchedulerCbsInvitationsSentCb
linphone_conference_scheduler_cbs_get_invitations_sent(const LinphoneConferenceSchedulerCbs *cbs) {
	return ConferenceSchedulerCbs::toCpp(cbs)->getInvitationsSent();
}

void linphone_conference_scheduler_cbs_set_invitations_sent(LinphoneConferenceSchedulerCbs *cbs,
                                                            LinphoneConferenceSchedulerCbsInvitationsSentCb cb) {
	ConferenceSchedulerCbs::toCpp(cbs)->setInvitationsSent(cb);
}
