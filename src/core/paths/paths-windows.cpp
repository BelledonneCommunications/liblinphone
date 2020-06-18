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

#include <algorithm>
#include <comutil.h>
#include <ShlObj.h>

#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "kernel32.lib")

#include "paths-windows.h"
#include "config.h"

// =============================================================================
#include <ppltasks.h>

using namespace std;

static bool dirExists(const std::string& dirName) {
  DWORD ftyp = GetFileAttributesA(dirName.c_str());
  if (ftyp == INVALID_FILE_ATTRIBUTES) return false;
  if (ftyp & FILE_ATTRIBUTE_DIRECTORY) return true;
  return false;
}

LINPHONE_BEGIN_NAMESPACE

static string getPath (const GUID &id) {
#ifdef ENABLE_MICROSOFT_STORE_APP

    //if( id ==FOLDERID_LocalAppData)
    string strPath;
    char * env = getenv("LOCALAPPDATA");
    if( env != NULL) {
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
#endif //ENABLE_MICROSOFT_STORE_APP
}


string SysPaths::getDataPath (void *) {
	static string dataPath = getPath(FOLDERID_LocalAppData);
	return dataPath;
}

string SysPaths::getConfigPath (void *) {
	// Yes, same path.
	return getDataPath(NULL);
}

string SysPaths::getDownloadPath (void *) {
	// TODO
	return getDataPath(NULL);
}

LINPHONE_END_NAMESPACE
