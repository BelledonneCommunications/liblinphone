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

#include "linphone/utils/utils.h"

#include <cstring>
#include <regex>
#include <vector>

LINPHONE_BEGIN_NAMESPACE

namespace Utils {

constexpr const char *SEMVER =
    "^(0|[1-9]\\d*)\\.(0|[1-9]\\d*)\\.(0|[1-9]\\d*)(?:-((?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\\.(?:0|[1-9]\\d*|"
    "\\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?(?:\\+([0-9a-zA-Z-]+(?:\\.[0-9a-zA-Z-]+)*))?$";

Version::Version(int major, int minor) : mMajor(major), mMinor(minor) {
}

Version::Version(int major, int minor, int patch) : mMajor(major), mMinor(minor), mPatch(patch) {
}

Version::Version(const std::string &version) {
	const static std::regex semver{SEMVER};
	std::smatch matches;

	if (std::regex_search(version, matches, semver)) {
		auto size = matches.size();
		if (size > 3) {
			mMajor = atoi(matches[1].str().c_str());
			mMinor = atoi(matches[2].str().c_str());
			mPatch = atoi(matches[3].str().c_str());
		}
		if (size > 4) {
			mPreRelease = matches[4].str();
		}
		if (size > 5) {
			mBuildMetaData = matches[5].str();
		}
	} else {
		bctbx_debug("Version [%s] doesn't matches semantic versioning regex", version.c_str());
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
}

int Version::compare(const Version &other) const {
	int tmp = mMajor - other.mMajor;
	if (tmp == 0) tmp = mMinor - other.mMinor;
	if (tmp == 0) tmp = mPatch - other.mPatch;

	// To ensure 1.0.0 > 1.0.0-beta for example
	if (tmp == 0 && mPreRelease.empty() && !other.mPreRelease.empty()) tmp = 1;
	if (tmp == 0 && !mPreRelease.empty() && other.mPreRelease.empty()) tmp = -1;

	if (tmp == 0) tmp = mPreRelease.compare(other.mPreRelease);
	if (tmp == 0) tmp = mBuildMetaData.compare(other.mBuildMetaData);
	return tmp;
}

LINPHONE_PUBLIC std::string Version::toString() const {
	std::ostringstream ostr;
	ostr << *this;
	return ostr.str();
}

} // namespace Utils

std::ostream &operator<<(std::ostream &ostr, const Utils::Version &version) {
	ostr << version.getMajor() << "." << version.getMinor();
	if (version.getPatch() != 0) ostr << "." << version.getPatch();
	if (!version.getPreRelease().empty()) ostr << "-" << version.getPreRelease();
	if (!version.getBuildMetaData().empty()) ostr << "+" << version.getBuildMetaData();
	return ostr;
}

LINPHONE_END_NAMESPACE
