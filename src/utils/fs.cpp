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

#include <fstream>

#include "linphone/utils/fs.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

namespace Fs {
bool copy(const string &srcPath, const string &destPath) {
	ifstream src(srcPath, ios::binary);
	ofstream dest(destPath, ios::binary);
	dest << src.rdbuf();
	return !dest.fail();
}
} // namespace Fs

LINPHONE_END_NAMESPACE
