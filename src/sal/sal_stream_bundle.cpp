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

#include <algorithm>

#include "sal/sal_stream_bundle.h"
#include "sal/sal_stream_configuration.h"
#include "linphone/utils/utils.h"

LINPHONE_BEGIN_NAMESPACE

SalStreamBundle::SalStreamBundle() {
	mids.clear();
}

SalStreamBundle::SalStreamBundle(const char *ids) {
	char *tmp = (char *)ms_malloc0(strlen(ids) + 1);
	int err;
	do {
		int consumed = 0;
		err = sscanf(ids, "%s%n", tmp, &consumed);
		if (err > 0) {
			mids.push_back(tmp);
			ids += consumed;
		} else break;
	} while (*ids != '\0');
	ms_free(tmp);
}

SalStreamBundle::SalStreamBundle(const SalStreamBundle &other) {
	mids = other.mids;
}

SalStreamBundle::~SalStreamBundle() {
	mids.clear();
}

void SalStreamBundle::addToSdp(belle_sdp_session_description_t *session_desc) const {
	char *attr_value = ms_strdup("BUNDLE");
	for (const auto &mid : mids) {
		char *tmp = ms_strdup_printf("%s %s", attr_value, mid.c_str());
		ms_free(attr_value);
		attr_value = tmp;
	}
	belle_sdp_session_description_add_attribute(session_desc, belle_sdp_attribute_create("group", attr_value));
	bctbx_free(attr_value);
}

SalStreamBundle &SalStreamBundle::operator=(const SalStreamBundle &other) {
	mids = other.mids;
	return *this;
}

bool SalStreamBundle::operator==(const SalStreamBundle &other) const {
	return (mids == other.mids);
}

bool SalStreamBundle::operator!=(const SalStreamBundle &other) const {
	return !(*this == other);
}

void SalStreamBundle::addStream(SalStreamConfiguration &cfg, const std::string &mid) {
	cfg.mid = mid;
	// If the stream is tagged as bundle only and the list of mid is empty, then append an empty element to reserve the slot for the transport owner
	if (cfg.bundle_only && mids.empty()) {
		mids.push_back(std::string());
	} else if (!cfg.bundle_only && !mids.empty() && (mids.front() == std::string())){
		mids.pop_front();
		mids.push_front(mid);
		return;
	}
	mids.push_back(mid);
}

const std::string & SalStreamBundle::getMidOfTransportOwner() const {
	if (!mids.empty()) {
		return mids.front(); /* the first one is the transport owner*/
	}
	return Utils::getEmptyConstRefObject<std::string>();
}

bool SalStreamBundle::hasMid(const std::string &mid) const {
	const auto &midIt = std::find_if(mids.cbegin(), mids.cend(),
	                                 [&mid](const auto &bundleMid) { return (bundleMid.compare(mid) == 0); });
	return (midIt != mids.cend());
}

LINPHONE_END_NAMESPACE
