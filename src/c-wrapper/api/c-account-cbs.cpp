/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
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

#include "linphone/api/c-account-cbs.h"

#include "c-wrapper/c-wrapper.h"

// =============================================================================

struct _LinphoneAccountCbs {
	belle_sip_object_t base;
	void *userData;
	LinphoneAccountCbsRegistrationStateChangedCb registration_state_changed;
};

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneAccountCbs);

BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneAccountCbs);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneAccountCbs, belle_sip_object_t,
	NULL, // destroy
	NULL, // clone
	NULL, // marshal
	FALSE
);

// =============================================================================

LinphoneAccountCbs * linphone_account_cbs_new (void) {
	return belle_sip_object_new(LinphoneAccountCbs);
}

LinphoneAccountCbs * linphone_account_cbs_ref (LinphoneAccountCbs *cbs) {
	belle_sip_object_ref(cbs);
	return cbs;
}

void linphone_account_cbs_unref (LinphoneAccountCbs *cbs) {
	belle_sip_object_unref(cbs);
}

void* linphone_account_cbs_get_user_data (const LinphoneAccountCbs *cbs) {
	return cbs->userData;
}

void linphone_account_cbs_set_user_data (LinphoneAccountCbs *cbs, void *ud) {
	cbs->userData = ud;
}

LinphoneAccountCbsRegistrationStateChangedCb linphone_account_cbs_get_registration_state_changed (
	const LinphoneAccountCbs *cbs
) {
	return cbs->registration_state_changed;
}

void linphone_account_cbs_set_registration_state_changed (
	LinphoneAccountCbs *cbs,
	LinphoneAccountCbsRegistrationStateChangedCb cb
) {
	cbs->registration_state_changed = cb;
}
