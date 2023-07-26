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

#include "linphone/api/c-alert.h"
#include "alert/alert.h"
#include "c-wrapper/c-wrapper.h"
#include "linphone/api/c-alert-cbs.h"

using namespace LinphonePrivate;

const char *linphone_alert_type_to_string(LinphoneAlertType type) {
	switch (type) {
		case LinphoneAlertQoSCameraMisfunction:
			return "LinphoneAlertQoSCameraMisfunction";
		case LinphoneAlertQoSCameraLowFramerate:
			return "LinphoneAlertQoSCameraLowFramerate";
		case LinphoneAlertQoSVideoStalled:
			return "LinphoneAlertQoSVideoStalled";
		case LinphoneAlertQoSHighLossLateRate:
			return "LinphoneAlertQoSHighLossLateRate";
		case LinphoneAlertQoSHighRemoteLossRate:
			return "LinphoneAlertQoSHighRemoteLossRate";
		case LinphoneAlertQoSRetransmissionFailures:
			return "LinphoneAlertQoSRetransmissionFailures";
		case LinphoneAlertQoSLowDownloadBandwidthEstimation:
			return "LinphoneAlertQoSLowDownloadBandwidthEstimation";
		case LinphoneAlertQoSLowQualityReceivedVideo:
			return "LinphoneAlertQoSLowQualityReceivedVideo";
		case LinphoneAlertQoSLowQualitySentVideo:
			return "LinphoneAlertQoSLowQualitySentVideo";
		case LinphoneAlertQoSLowSignal:
			return "LinphoneAlertQoSLowSignal";
		case LinphoneAlertQoSLostSignal:
			return "LinphoneAlertQoSLostSignal";
		case LinphoneAlertQoSBurstOccured:
			return "LinphoneAlertQoSBurstOccured";
	}
	return "LinphoneAlertType not found";
}

LinphoneAlert *linphone_alert_clone(const LinphoneAlert *alert) {
	return Alert::toCpp(alert)->clone()->toC();
}
LinphoneAlert *linphone_alert_ref(LinphoneAlert *alert) {
	Alert::toCpp(alert)->ref();
	return alert;
}
void linphone_alert_unref(LinphoneAlert *alert) {
	Alert::toCpp(alert)->unref();
}
time_t linphone_alert_get_start_time(const LinphoneAlert *alert) {
	return Alert::toCpp(alert)->getStartTime();
}
time_t linphone_alert_get_end_time(const LinphoneAlert *alert) {
	return Alert::toCpp(alert)->getEndTime();
}
LinphoneAlertType linphone_alert_get_type(const LinphoneAlert *alert) {
	return Alert::toCpp(alert)->getType();
}
const LinphoneDictionary *linphone_alert_get_informations(const LinphoneAlert *alert) {
	return Alert::toCpp(alert)->getInformations()->toC();
}
LinphoneCall *linphone_alert_get_call(const LinphoneAlert *alert) {
	auto call = Alert::toCpp(alert)->getCall().lock();
	return (call) ? call->toC() : NULL;
}
bool_t linphone_alert_get_state(const LinphoneAlert *alert) {
	return Alert::toCpp(alert)->getState();
}
void linphone_alert_add_callbacks(LinphoneAlert *alert, LinphoneAlertCbs *cbs) {
	Alert::toCpp(alert)->addCallbacks(AlertCbs::toCpp(cbs)->getSharedFromThis());
}
LinphoneAlertCbs *linphone_alert_get_current_callbacks(const LinphoneAlert *alert) {
	return Alert::toCpp(alert)->getCurrentCallbacks()->toC();
}
const bctbx_list_t *linphone_alert_get_callbacks_list(const LinphoneAlert *alert) {
	return Alert::toCpp(alert)->getCCallbacksList();
}
void linphone_alert_remove_callbacks(LinphoneAlert *alert, LinphoneAlertCbs *cbs) {
	Alert::toCpp(alert)->removeCallbacks(AlertCbs::toCpp(cbs)->getSharedFromThis());
}
void linphone_alert_notify_on_terminated(LinphoneAlert *alert) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS_NO_ARG(Alert, Alert::toCpp(alert), linphone_alert_cbs_get_terminated);
}
