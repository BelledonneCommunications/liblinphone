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

#include "logger/logger.h"

#include "paths-linux.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

static string getBaseDirectory () {
	static string base;
	if (base.empty()) {
		char *dir = getenv("HOME");
		if (!dir)
			lFatal() << "Unable to get home directory.";
		base = dir;
	}
	return base;
}

string SysPaths::getDataPath (void *) {
	static string dataPath = getBaseDirectory() + "/.local/share/linphone/";
	return dataPath;
}

string SysPaths::getConfigPath (void *) {
	static string configPath = getBaseDirectory() + "/.config/linphone/";
	return configPath;
}

string SysPaths::getDownloadPath (void *) {
	//TODO
	static string downloadPath = getBaseDirectory() + "/.config/linphone/";
	return downloadPath;
}

LINPHONE_END_NAMESPACE
