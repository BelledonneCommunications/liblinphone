/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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

#include "linphone/api/c-event-cbs.h"

#include "event/event.h"

// =============================================================================

using namespace LinphonePrivate;

LinphoneEventCbs *linphone_event_cbs_new(void) {
	return EventCbs::createCObject();
}

LinphoneEventCbs *linphone_event_cbs_ref(LinphoneEventCbs *cbs) {
	EventCbs::toCpp(cbs)->ref();
	return cbs;
}

void linphone_event_cbs_unref(LinphoneEventCbs *cbs) {
	EventCbs::toCpp(cbs)->unref();
}

void *linphone_event_cbs_get_user_data(const LinphoneEventCbs *cbs) {
	return EventCbs::toCpp(cbs)->getUserData();
}

void linphone_event_cbs_set_user_data(LinphoneEventCbs *cbs, void *ud) {
	EventCbs::toCpp(cbs)->setUserData(ud);
}

LinphoneEventCbsNotifyResponseCb linphone_event_cbs_get_notify_response(const LinphoneEventCbs *cbs) {
	return EventCbs::toCpp(cbs)->notifyResponseCb;
}

void linphone_event_cbs_set_notify_response(LinphoneEventCbs *cbs, LinphoneEventCbsNotifyResponseCb cb) {
	EventCbs::toCpp(cbs)->notifyResponseCb = cb;
}