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

#include "linphone/api/c-account-cbs.h"
#include "account/account.h"
#include "c-wrapper/c-wrapper.h"

using namespace LinphonePrivate;

// =============================================================================

LinphoneAccountCbs *linphone_account_cbs_new(void) {
	return AccountCbs::createCObject();
}

LinphoneAccountCbs *linphone_account_cbs_ref(LinphoneAccountCbs *cbs) {
	AccountCbs::toCpp(cbs)->ref();
	return cbs;
}

void linphone_account_cbs_unref(LinphoneAccountCbs *cbs) {
	AccountCbs::toCpp(cbs)->unref();
}

void *linphone_account_cbs_get_user_data(const LinphoneAccountCbs *cbs) {
	return AccountCbs::toCpp(cbs)->getUserData();
}

void linphone_account_cbs_set_user_data(LinphoneAccountCbs *cbs, void *ud) {
	AccountCbs::toCpp(cbs)->setUserData(ud);
}

LinphoneAccountCbsRegistrationStateChangedCb
linphone_account_cbs_get_registration_state_changed(const LinphoneAccountCbs *cbs) {
	return AccountCbs::toCpp(cbs)->getRegistrationStateChanged();
}

void linphone_account_cbs_set_registration_state_changed(LinphoneAccountCbs *cbs,
                                                         LinphoneAccountCbsRegistrationStateChangedCb cb) {
	AccountCbs::toCpp(cbs)->setRegistrationStateChanged(cb);
}

LinphoneAccountCbsMessageWaitingIndicationChangedCb
linphone_account_cbs_get_message_waiting_indication_changed(const LinphoneAccountCbs *cbs) {
	return AccountCbs::toCpp(cbs)->getMessageWaitingIndicationChanged();
}

void linphone_account_cbs_set_message_waiting_indication_changed(
    LinphoneAccountCbs *cbs, LinphoneAccountCbsMessageWaitingIndicationChangedCb cb) {
	AccountCbs::toCpp(cbs)->setMessageWaitingIndicationChanged(cb);
}

LinphoneAccountCbsConferenceInformationUpdatedCb
linphone_account_cbs_get_conference_information_updated(const LinphoneAccountCbs *cbs) {
	return AccountCbs::toCpp(cbs)->getConferenceInformationUpdated();
}

void linphone_account_cbs_set_conference_information_updated(LinphoneAccountCbs *cbs,
                                                             LinphoneAccountCbsConferenceInformationUpdatedCb cb) {
	AccountCbs::toCpp(cbs)->setConferenceInformationUpdated(cb);
}
