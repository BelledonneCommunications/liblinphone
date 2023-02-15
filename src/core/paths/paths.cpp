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

#include "paths.h"
#include "core/platform-helpers/platform-helpers.h"

#ifdef __APPLE__
#include "paths-apple.h"
#elif defined(__ANDROID__)
#include "paths-android.h"
#elif defined(_WIN32)
#include "paths-windows.h"
#elif defined(__linux__)
#include "paths-linux.h"
#else
#error "Unsupported system."
#endif // ifdef __APPLE__

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

string Paths::getPath(Paths::Type type, void *context) {
	switch (type) {
		case Data:
			return SysPaths::getDataPath(context);
		case Config:
			return SysPaths::getConfigPath(context);
		case Download:
			return SysPaths::getDownloadPath(context);
	}

	L_ASSERT(false);
	return "";
}

LINPHONE_END_NAMESPACE
