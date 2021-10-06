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

#include "linphone/api/c-participant-cbs.h"

#include "c-wrapper/c-wrapper.h"

// =============================================================================

struct _LinphoneParticipantCbs {
	belle_sip_object_t base;
	void *userData;
};

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneParticipantCbs);

BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneParticipantCbs);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneParticipantCbs, belle_sip_object_t,
	NULL, // destroy
	NULL, // clone
	NULL, // marshal
	FALSE
);

// =============================================================================

LinphoneParticipantCbs * _linphone_participant_cbs_new (void) {
	return belle_sip_object_new(LinphoneParticipantCbs);
}

LinphoneParticipantCbs * linphone_participant_cbs_ref (LinphoneParticipantCbs *cbs) {
	belle_sip_object_ref(cbs);
	return cbs;
}

void linphone_participant_cbs_unref (LinphoneParticipantCbs *cbs) {
	belle_sip_object_unref(cbs);
}

void * linphone_participant_cbs_get_user_data (const LinphoneParticipantCbs *cbs) {
	return cbs->userData;
}

void linphone_participant_cbs_set_user_data (LinphoneParticipantCbs *cbs, void *ud) {
	cbs->userData = ud;
}
