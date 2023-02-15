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

#ifndef _SAL_STREAM_BUNDLE_H_
#define _SAL_STREAM_BUNDLE_H_

#include <list>
#include <string>

#include "bellesip_sal/sal_impl.h"
#include "linphone/utils/general.h"

LINPHONE_BEGIN_NAMESPACE

class SalStreamConfiguration;

class SalStreamBundle {

public:
	SalStreamBundle();
	SalStreamBundle(const char *ids);
	SalStreamBundle(const SalStreamBundle &other);
	virtual ~SalStreamBundle();

	void addToSdp(belle_sdp_session_description_t *session_desc) const;

	SalStreamBundle &operator=(const SalStreamBundle &other);
	bool operator==(const SalStreamBundle &other) const;
	bool operator!=(const SalStreamBundle &other) const;

	void addStream(SalStreamConfiguration &stream, const std::string &mid);

	const std::string &getMidOfTransportOwner() const;

	bool hasMid(const std::string &mid) const;

	std::list<std::string>
	    mids; /* List of mids corresponding to streams associated in the bundle. The first one is the "tagged" one. */
};

LINPHONE_END_NAMESPACE

#endif // ifndef _SAL_STREAM_BUNDLE_H_
