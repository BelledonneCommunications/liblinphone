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

#include <chrono>

#include <bctoolbox/logging.h>

#include "object/base-object-p.h"

#include "logger.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

DurationLogger::DurationLogger(const string &label, BctbxLogLevel level) : mLabel(label), mLevel(level) {
	BCTBX_SLOG(BCTBX_LOG_DOMAIN, mLevel) << "Start measurement of [" + label + "].";
	mStart = chrono::high_resolution_clock::now();
}

DurationLogger::~DurationLogger() {
	chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
	BCTBX_SLOG(BCTBX_LOG_DOMAIN, mLevel) << "Duration of [" + mLabel + "]: "
	                                     << chrono::duration_cast<chrono::milliseconds>(end - mStart).count() << "ms.";
}

LINPHONE_END_NAMESPACE
