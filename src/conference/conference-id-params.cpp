/*
 * Copyright (c) 2010-2025 Belledonne Communications SARL.
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

#include "conference-id-params.h"

#include "core/core.h"
#include "linphone/core.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

ConferenceIdParams::ConferenceIdParams(const std::shared_ptr<const Core> &core) {
	if (core) {
		setKeepGruu(!!linphone_core_gruu_in_conference_address_enabled(core->getCCore()));
	}
}

bool ConferenceIdParams::getKeepGruu() const {
	return mKeepGruu;
}

void ConferenceIdParams::setKeepGruu(bool keepGruu) {
	mKeepGruu = keepGruu;
}

bool ConferenceIdParams::extractUriEnabled() const {
	return mExtractUri;
}

void ConferenceIdParams::enableExtractUri(bool extractUri) {
	mExtractUri = extractUri;
}

LINPHONE_END_NAMESPACE
