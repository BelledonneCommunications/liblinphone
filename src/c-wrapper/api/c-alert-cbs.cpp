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

#include "linphone/api/c-alert-cbs.h"
#include "alert/alert.h"
#include "c-wrapper/c-wrapper.h"

using namespace LinphonePrivate;

LinphoneAlertCbs *linphone_alert_cbs_ref(LinphoneAlertCbs *cbs) {
	AlertCbs::toCpp(cbs)->ref();
	return cbs;
}

void linphone_alert_cbs_unref(LinphoneAlertCbs *cbs) {
	AlertCbs::toCpp(cbs)->unref();
}

void *linphone_alert_cbs_get_user_data(const LinphoneAlertCbs *cbs) {
	return AlertCbs::toCpp(cbs)->getUserData();
}

void linphone_alert_cbs_set_user_data(LinphoneAlertCbs *cbs, void *ud) {
	AlertCbs::toCpp(cbs)->setUserData(ud);
}

void linphone_alert_cbs_set_terminated(LinphoneAlertCbs *cbs, LinphoneAlertCbsTerminatedCb on_terminated) {
	AlertCbs::toCpp(cbs)->setOnTerminated(on_terminated);
}
LinphoneAlertCbsTerminatedCb linphone_alert_cbs_get_terminated(LinphoneAlertCbs *cbs) {
	return AlertCbs::toCpp(cbs)->getOnTerminated();
}