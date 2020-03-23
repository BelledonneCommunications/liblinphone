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

#include <belle-sip/object.h>

#include "object.hh"
#include "tools.hh"

using namespace linphone;
using namespace std;


StringBctbxListWrapper::StringBctbxListWrapper(const std::list<std::string> &cppList): AbstractBctbxListWrapper() {
	for(const auto &str : cppList) {
		mCList = bctbx_list_append(mCList, const_cast<char *>(str.c_str()));
	}
}

StringBctbxListWrapper::~StringBctbxListWrapper() {
	bctbx_list_free(mCList);
}

list<string> StringBctbxListWrapper::bctbxListToCppList(const ::bctbx_list_t *bctbxList) {
	list<string> cppList;
	for(auto it=bctbxList; it; it=it->next) {
		cppList.push_back(string(static_cast<char *>(it->data)));
	}
	return cppList;
}

std::list<std::string> StringBctbxListWrapper::bctbxListToCppList(::bctbx_list_t *bctbxList) {
	auto cppList = bctbxListToCppList(const_cast<const ::bctbx_list_t *>(bctbxList));
	if (bctbxList) bctbx_free(bctbxList);
	return cppList;
}

std::string StringUtilities::cStringToCpp(const char *cstr) {
	if (cstr == NULL) {
		return std::string();
	} else {
		return std::string(cstr);
	}
}

std::string StringUtilities::cStringToCpp(char *cstr) {
	if (cstr == NULL) {
		return std::string();
	} else {
		std::string cppStr = cstr;
		bctbx_free(cstr);
		return cppStr;
	}
}

const char *StringUtilities::cppStringToC(const std::string &cppstr) {
	if (cppstr.empty()) {
		return NULL;
	} else {
		return cppstr.c_str();
	}
}

std::list<std::string> StringUtilities::cStringArrayToCppList(const char **cArray) {
	list<string> cppList;
	if (cArray == NULL) return cppList;
	for(int i=0; cArray[i]!=NULL; i++) {
		cppList.push_back(cArray[i]);
	}
	return cppList;
}
