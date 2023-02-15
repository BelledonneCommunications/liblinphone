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

#include "logger/logger.h"
#include <unistd.h>

#include "linphone/api/c-factory.h"
#include "paths-linux.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

static string getBaseDirectory() {
	static string base;
	if (base.empty()) {
		char *dir = getenv("HOME");
		if (!dir) {
			lError() << "Unable to get $HOME directory, will use current directory instead as base directory.";
			dir = get_current_dir_name();
			base = dir;
			free(dir);
		} else {
			base = dir;
		}
	}
	return base;
}

string SysPaths::getDataPath(void *context) {
	static std::string dataPath;
	if (linphone_factory_is_data_dir_set(linphone_factory_get()))
		dataPath = linphone_factory_get_data_dir(linphone_factory_get(), context);
	else dataPath = getBaseDirectory() + "/.local/share/linphone/";
	return dataPath;
}

string SysPaths::getConfigPath(void *context) {
	static std::string configPath;
	if (linphone_factory_is_config_dir_set(linphone_factory_get()))
		configPath = linphone_factory_get_config_dir(linphone_factory_get(), context);
	else configPath = getBaseDirectory() + "/.config/linphone/";
	return configPath;
}

string SysPaths::getDownloadPath(void *context) {
	if (linphone_factory_is_download_dir_set(linphone_factory_get()))
		return linphone_factory_get_download_dir(linphone_factory_get(), context);
	else return getDataPath(NULL);
}

LINPHONE_END_NAMESPACE
