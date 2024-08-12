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

#include "bctoolbox/defs.h"

#include "linphone/core.h"
#include "linphone/core_utils.h"
#if _MSC_VER
#include <io.h>
#endif
static int done = 0;

static void calibration_finished(BCTBX_UNUSED(LinphoneCore *lc), LinphoneEcCalibratorStatus status, int delay) {
	ms_message("echo calibration finished %s.", status == LinphoneEcCalibratorDone ? "successfully" : "with failure");
	if (status == LinphoneEcCalibratorDone) ms_message("Measured delay is %i", delay);
	done = 1;
}

static char config_file[1024] = {0};
void parse_args(int argc, char *argv[]) {
#ifndef F_OK
#define F_OK 4
#endif
	if (argc != 3 || strncmp("-c", argv[1], 2) || access(argv[2], F_OK) != 0) {
		printf("Usage: test_ecc [-c config_file] where config_file will be written with the detected value\n");
		exit(-1);
	}
	strncpy(config_file, argv[2], 1024);
	config_file[sizeof(config_file) - 1] = '\0';
}

int main(int argc, char *argv[]) {
	LinphoneCoreVTable vtable = {0};
	LinphoneCore *lc;
	if (argc > 1) parse_args(argc, argv);
	lc = linphone_core_new(&vtable, config_file[0] ? config_file : NULL, NULL, NULL);

	linphone_logging_service_set_log_level(linphone_logging_service_get(), LinphoneLogLevelMessage);

	LinphoneCoreCbs *cbs = linphone_core_get_current_callbacks(lc);
	linphone_core_enable_echo_cancellation(lc, TRUE);
	linphone_core_cbs_set_ec_calibration_result(cbs, calibration_finished);
	if (linphone_core_start_echo_canceller_calibration(lc)) {
		ms_message("Calibration failed");
	} else ms_message("Calibrating...");
	while (!done) {
		linphone_core_iterate(lc);
		ms_usleep(20000);
	}
	linphone_core_unref(lc);
	return 0;
}
