/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#include <stdio.h>
#include <stdlib.h>

#include "bctoolbox/tester.h"
#include "liblinphone_tester.h"
#include "linphone/logging.h"
#include "mediastreamer2/mscommon.h"
#include <bctoolbox/defs.h>
#include <bctoolbox/vfs.h>

static void set_same_log_file_test(void) {
	int n = 5;
	int first_size = 0;
	int file_size[] = {0, 0, 0, 0, 0};
	int single_line_size = 0;
	bctbx_vfs_file_t *log_file = NULL;

	const char *filename = "logfile.log";
	char *file_path = bc_tester_file(filename);
	unlink(file_path);
	ms_error("Close current log file, then write on temporary files.");
	LinphoneLoggingService *logging_service = linphone_logging_service_get();
	linphone_logging_service_set_log_file(logging_service, bc_tester_get_writable_dir_prefix(), filename, 0);

	log_file = bctbx_file_open(bctbx_vfs_get_default(), file_path, "r");
	first_size = (int)bctbx_file_size(log_file);
	bctbx_file_close(log_file);
	ms_error("Oh no, not again (0)");
	log_file = bctbx_file_open(bctbx_vfs_get_default(), file_path, "r");
	file_size[0] = (int)bctbx_file_size(log_file);
	bctbx_file_close(log_file);
	single_line_size = file_size[0] - first_size;

	for (int i = 1; i < n; i++) {
		linphone_logging_service_set_log_file(logging_service, bc_tester_get_writable_dir_prefix(), filename, 0);
		ms_error("Oh no, not again (%d)", i);
		log_file = bctbx_file_open(bctbx_vfs_get_default(), file_path, "r");
		file_size[i] = (int)bctbx_file_size(log_file);
		bctbx_file_close(log_file);
		BC_ASSERT_EQUAL(file_size[i], file_size[i - 1] + single_line_size, int, "%d");
	}

	unlink(file_path);
	ms_free(file_path);
}

static void set_new_log_file_test(void) {
	int n = 8;

	const char *filename_1 = "logfile1.log";
	char *file_path_1 = bc_tester_file(filename_1);
	unlink(file_path_1);
	ms_error("Close current log file, then write on temporary files.");
	LinphoneLoggingService *logging_service = linphone_logging_service_get();

	linphone_logging_service_set_log_file(logging_service, bc_tester_get_writable_dir_prefix(), filename_1, 0);
	for (int i = 0; i < n; i++)
		ms_error("Oh no, not again (%i)", i);

	// set new log file
	const char *filename_2 = "logfile2.log";
	char *file_path_2 = bc_tester_file(filename_2);
	unlink(file_path_2);
	linphone_logging_service_set_log_file(logging_service, bc_tester_get_writable_dir_prefix(), filename_2, 0);
	ms_error("Oh no, not again (%d)", n);

	bctbx_vfs_file_t *log_file_1 = bctbx_file_open(bctbx_vfs_get_default(), file_path_1, "r");
	int file_size_1 = (int)bctbx_file_size(log_file_1);
	bctbx_file_close(log_file_1);
	bctbx_vfs_file_t *log_file_2 = bctbx_file_open(bctbx_vfs_get_default(), file_path_2, "r");
	int file_size_2 = (int)bctbx_file_size(log_file_2);
	bctbx_file_close(log_file_2);
	BC_ASSERT_GREATER_STRICT(file_size_2, 0, int, "%d");
	BC_ASSERT_EQUAL(n * file_size_2, file_size_1, int, "%d");

	// log file is NULL
	linphone_logging_service_set_log_file(logging_service, bc_tester_get_writable_dir_prefix(), NULL, 0);
	for (int i = 0; i < n; i++)
		ms_error("no log file (%d)", i);

	// reset first log file
	linphone_logging_service_set_log_file(logging_service, bc_tester_get_writable_dir_prefix(), filename_1, 0);
	ms_error("Oh no, not again (%d)", n + 1);

	log_file_2 = bctbx_file_open(bctbx_vfs_get_default(), file_path_2, "r");
	file_size_2 = (int)bctbx_file_size(log_file_2);
	bctbx_file_close(log_file_2);
	log_file_1 = bctbx_file_open(bctbx_vfs_get_default(), file_path_1, "r");
	file_size_1 = (int)bctbx_file_size(log_file_1);
	bctbx_file_close(log_file_1);
	BC_ASSERT_EQUAL((n + 1) * file_size_2, file_size_1, int, "%d");

	unlink(file_path_1);
	unlink(file_path_2);
	ms_free(file_path_1);
	ms_free(file_path_2);
}

test_t log_file_tests[] = {
    TEST_NO_TAG("Set new log file", set_new_log_file_test),
    TEST_NO_TAG("Set same log file", set_same_log_file_test),
};

test_suite_t log_file_test_suite = {"Log file",
                                    NULL,
                                    NULL,
                                    liblinphone_tester_before_each,
                                    liblinphone_tester_after_each,
                                    sizeof(log_file_tests) / sizeof(log_file_tests[0]),
                                    log_file_tests,
                                    0};