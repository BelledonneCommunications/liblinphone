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

#ifndef _L_CONFERENCE_ID_PARAMS_H_
#define _L_CONFERENCE_ID_PARAMS_H_

#include <string>

#include "linphone/utils/general.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Core;

class LINPHONE_PUBLIC ConferenceIdParams {
public:
	ConferenceIdParams(const std::shared_ptr<const Core> &core = nullptr);
	ConferenceIdParams(const ConferenceIdParams &other) = default;
	ConferenceIdParams(ConferenceIdParams &&other) = default;
	ConferenceIdParams &operator=(const ConferenceIdParams &other) = default;

	virtual ~ConferenceIdParams() = default;

	bool getKeepGruu() const;
	void setKeepGruu(bool keepGruu);

	bool extractUriEnabled() const;
	void enableExtractUri(bool extractUri);

private:
	bool mKeepGruu = false;
	bool mExtractUri = true;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_ID_PARAMS_H_
