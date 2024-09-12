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
#include "bctoolbox/charconv.h"
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

static bool dirExists(const std::wstring &dirName) {
	DWORD ftyp = GetFileAttributesW(dirName.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES) return false;
	if (ftyp & FILE_ATTRIBUTE_DIRECTORY) return true;
	return false;
}

LINPHONE_BEGIN_NAMESPACE

static string getPath(const GUID &id) {
	string strPath;
	wstring wPath;
#if defined(ENABLE_MICROSOFT_STORE_APP) || defined(LINPHONE_WINDOWS_UWP)
	/* FIXME: should rather use, but requires C++/Cx
	 * https://learn.microsoft.com/en-us/uwp/api/windows.storage.userdatapaths.localappdata?view=winrt-26100#windows-storage-userdatapaths-localappdata
	 */
	// if( id ==FOLDERID_LocalAppData)
	wchar_t *env = _wgetenv(L"LOCALAPPDATA");
	if (env) {
		wPath = env;
	}
	lInfo() << "Got application data dir from env variable.";
#else
	LPWSTR path;
	if (SHGetKnownFolderPath(id, KF_FLAG_DONT_VERIFY, 0, &path) == S_OK) {
		wPath = path;
		CoTaskMemFree(path);
	}
	lInfo() << "Got application data dir via SHGetKnownFolderPath: " << strPath;
#endif

	wPath = wPath.append(L"\\linphone\\");
	char *str = bctbx_wide_string_to_string(wPath.c_str());
	if (str) {
		strPath = str;
		bctbx_free(str);
	}
	if (!dirExists(wPath)) {
		lInfo() << "Creating application data directory:" << strPath;
		if (!CreateDirectoryW(wPath.c_str(), nullptr)) {
			lError() << "Directory could not be created.";
		}
	}
	return strPath;
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
