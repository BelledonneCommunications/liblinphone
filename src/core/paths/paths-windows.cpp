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

#define _WINSOCKAPI_ // stops windows.h including winsock.h
#include "paths-windows.h"
#include "config.h"
#include "linphone/api/c-factory.h"
#include "linphone/utils/utils.h"
#include <ShlObj.h>
#include <algorithm>
#include <comutil.h>
#include <ppltasks.h>
#include <windows.h>

#include "private.h" // To get LINPHONE_WINDOWS_UWP
#if !defined(LINPHONE_WINDOWS_UWP)
#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "kernel32.lib")
#endif

// =============================================================================

using namespace std;

static bool dirExists(const std::string &dirName) {
	DWORD ftyp = GetFileAttributesA(dirName.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES) return false;
	if (ftyp & FILE_ATTRIBUTE_DIRECTORY) return true;
	return false;
}

LINPHONE_BEGIN_NAMESPACE

static string getPath(const GUID &id) {
#if defined(ENABLE_MICROSOFT_STORE_APP) || defined(LINPHONE_WINDOWS_UWP)

	// if( id ==FOLDERID_LocalAppData)
	string strPath;
	char *env = getenv("LOCALAPPDATA");
	if (env != NULL) {
		strPath = env;
		strPath = strPath.append("/linphone/");
		if (!dirExists(strPath)) CreateDirectoryA(strPath.c_str(), nullptr);
	}
	return strPath;
#else
	string strPath;
	LPWSTR path;
	if (SHGetKnownFolderPath(id, KF_FLAG_DONT_VERIFY, 0, &path) == S_OK) {
		strPath = _bstr_t(path);
		replace(strPath.begin(), strPath.end(), '\\', '/');
		CoTaskMemFree(path);
	}

	strPath = strPath.append("/linphone/");
	if (!dirExists(strPath)) CreateDirectoryA(strPath.c_str(), nullptr);
	return strPath;
#endif // ENABLE_MICROSOFT_STORE_APP
}

string SysPaths::getDataPath(void *context) {
	static std::string dataPath;
	if (linphone_factory_is_data_dir_set(linphone_factory_get()))
		dataPath = linphone_factory_get_data_dir(linphone_factory_get(), context);
	else {
#if defined(LINPHONE_WINDOWS_UWP)
		dataPath = getPath(GUID_NULL);
#else
		dataPath = getPath(FOLDERID_LocalAppData); // FOLDERID_LocalAppData);
#endif
	}
	return dataPath;
}

string SysPaths::getConfigPath(void *context) {
	if (linphone_factory_is_config_dir_set(linphone_factory_get()))
		return linphone_factory_get_config_dir(linphone_factory_get(), context);
	else return getDataPath(NULL);
}

string SysPaths::getDownloadPath(void *context) {
	if (linphone_factory_is_download_dir_set(linphone_factory_get()))
		return linphone_factory_get_download_dir(linphone_factory_get(), context);
	else return getDataPath(NULL);
}

LINPHONE_END_NAMESPACE
