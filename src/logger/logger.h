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

#ifndef _L_LOGGER_H_
#define _L_LOGGER_H_

#include "bctoolbox/logging.h"
#include "linphone/utils/general.h"

#include <chrono>

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class DurationLogger {
public:
	DurationLogger(const std::string &label, BctbxLogLevel level = BCTBX_LOG_MESSAGE);
	DurationLogger(DurationLogger &) = delete;
	~DurationLogger();

private:
	std::chrono::high_resolution_clock::time_point mStart;
	const std::string mLabel;
	const BctbxLogLevel mLevel;
};

LINPHONE_END_NAMESPACE

#define lDebug() BCTBX_SLOGD
#define lInfo() BCTBX_SLOGI
#define lWarning() BCTBX_SLOGW
#define lError() BCTBX_SLOGE
#define lFatal() BCTBX_SLOGF

#define L_BEGIN_LOG_EXCEPTION try {

#define L_END_LOG_EXCEPTION                                                                                            \
	}                                                                                                                  \
	catch (const exception &e) {                                                                                       \
		lWarning() << "Error: " << e.what();                                                                           \
	}

#endif // ifndef _L_LOGGER_H_
