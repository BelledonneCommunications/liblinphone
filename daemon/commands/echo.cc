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

#include <bctoolbox/defs.h>

#include "echo.h"

using namespace std;

void echoResultCbs(LinphoneCore *core, LinphoneEcCalibratorStatus status, int delay_ms) {
	Daemon *app = (Daemon *)linphone_core_get_user_data(core);
	ostringstream ost;
	switch (status) {
		case LinphoneEcCalibratorInProgress:
			ost << "Calibration in progress";
			break;
		case LinphoneEcCalibratorDone:
			ost << "Calibration Done";
			break;
		case LinphoneEcCalibratorFailed:
			ost << "Calibration failed";
			break;
		case LinphoneEcCalibratorDoneNoEcho:
			ost << "Calibration done but no echo";
			break;
	}
	ost << ", delay: " << delay_ms << "ms";
	app->sendResponse(Response(ost.str(), Response::Ok));
}

EchoCalibrationCommand::EchoCalibrationCommand()
    : DaemonCommand("echo-calibration", "echo-calibration", "Make an echo calibration and return the result in ms.") {
	addExample(make_unique<DaemonCommandExample>("echo-calibration", "Status: Error\n"
	                                                                 "Reason: Calibration failed"));
	addExample(make_unique<DaemonCommandExample>("echo-calibration", "Status: Ok"));
}

void EchoCalibrationCommand::exec(Daemon *app, BCTBX_UNUSED(const string &args)) {
	LinphoneCore *lc = app->getCore();
	// LinphoneCoreCbs * cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	LinphoneCoreCbs *cbs = linphone_core_get_current_callbacks(lc);
	linphone_core_enable_echo_cancellation(lc, TRUE);
	linphone_core_cbs_set_ec_calibration_result(cbs, echoResultCbs);
	if (linphone_core_start_echo_canceller_calibration(lc)) {
		app->sendResponse(Response("Calibration failed", Response::Error));
	} else app->sendResponse(Response("Calibrating...", Response::Ok));
}
