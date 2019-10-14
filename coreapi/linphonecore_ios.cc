/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
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
#include <stdio.h>
#include "linphone/core.h"

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

#if TARGET_OS_IPHONE
static void linphone_iphone_log_handler(const char *domain, OrtpLogLevel lev, const char *fmt, va_list args) {
	char* str = bctbx_strdup_vprintf(fmt, args);
    const char *levname = "undef";

	if (!domain)
		domain = "lib";

	switch (lev) {
		case ORTP_FATAL:
			levname = "Fatal";
			break;
		case ORTP_ERROR:
			levname = "Error";
			break;
		case ORTP_WARNING:
			levname = "Warning";
			break;
		case ORTP_MESSAGE:
			levname = "Message";
			break;
		case ORTP_DEBUG:
			levname = "Debug";
			break;
		case ORTP_TRACE:
			levname = "Trace";
			break;
		case ORTP_LOGLEV_END:
			return;
	}
	fprintf(stdout,"[%s] %s\n", levname, str);
}

extern "C" void linphone_iphone_enable_logs() {
	linphone_core_enable_logs_with_cb(linphone_iphone_log_handler);
}
#endif