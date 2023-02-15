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

#include <stdio.h>

#include "bctoolbox/defs.h"

#include "lpc2xml.h"

void cb_function(BCTBX_UNUSED(void *ctx), lpc2xml_log_level level, const char *msg, va_list list) {
	const char *header = "";
	switch (level) {
		case LPC2XML_DEBUG:
			header = "DEBUG";
			break;
		case LPC2XML_MESSAGE:
			header = "MESSAGE";
			break;
		case LPC2XML_WARNING:
			header = "WARNING";
			break;
		case LPC2XML_ERROR:
			header = "ERROR";
			break;
	}
	fprintf(stdout, "%s - ", header);
	vfprintf(stdout, msg, list);
	fprintf(stdout, "\n");
}

void show_usage(BCTBX_UNUSED(int argc), char *argv[]) {
	fprintf(stderr, "usage:\n%s convert <lpc_file> <xml_file>\n%s dump <lpc_file>\n", argv[0], argv[0]);
}

int main(int argc, char *argv[]) {
	lpc2xml_context *ctx;
	LpConfig *lpc;
	if (argc > 4 || argc < 3) {
		show_usage(argc, argv);
		return -1;
	}

	lpc = linphone_config_new(argv[2]);
	if (strcmp("convert", argv[1]) == 0 && argc == 4) {
		ctx = lpc2xml_context_new(cb_function, NULL);
		lpc2xml_set_lpc(ctx, lpc);
		lpc2xml_convert_file(ctx, argv[3]);
		lpc2xml_context_destroy(ctx);
	} else if (strcmp("dump", argv[1]) == 0 && argc == 3) {
		char *dump = linphone_config_dump_as_xml(lpc);
		fprintf(stdout, "%s", dump);
	} else {
		show_usage(argc, argv);
	}
	linphone_config_destroy(lpc);
	return 0;
}
