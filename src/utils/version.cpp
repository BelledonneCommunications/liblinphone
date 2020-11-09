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

#include "linphone/utils/utils.h"

#include <cstring>

LINPHONE_BEGIN_NAMESPACE

namespace Utils{

Version::Version(int major, int minor) : mMajor(major), mMinor(minor){
}

Version::Version(int major, int minor, int patch) : mMajor(major), mMinor(minor), mPatch(patch){
}

Version::Version(const std::string &version){
	const char *ptr = version.c_str();
	const char *next;
	
	next = strchr(ptr, '.');
	mMajor = atoi(ptr);
	ptr = next + 1;
	next = strchr(ptr, '.');
	mMinor = atoi(ptr);
	if (next != NULL) {
		ptr = next + 1;
		mPatch = atoi(ptr);
	}
}

int Version::compare(const Version &other)const{
	int tmp = mMajor - other.mMajor;
	if (tmp == 0) tmp = mMinor - other.mMinor;
	if (tmp == 0) tmp = mPatch - other.mPatch;
	return tmp;
}

}

std::ostream &operator<<(std::ostream & ostr, const Utils::Version &version){
	ostr << version.getMajor() << "." << version.getMinor();
	if (version.getPatch() != 0) ostr << "." << version.getPatch();
	return ostr;
}

LINPHONE_END_NAMESPACE

