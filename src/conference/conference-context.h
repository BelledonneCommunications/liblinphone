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

#ifndef _L_CONFERENCE_CONTEXT_H_
#define _L_CONFERENCE_CONTEXT_H_

#include <list>
#include <memory>

#include "address/address.h"
#include "linphone/utils/general.h"

LINPHONE_BEGIN_NAMESPACE

class ConferenceParams;

class ConferenceContext {
public:
	ConferenceContext() = delete;
	ConferenceContext(const std::shared_ptr<ConferenceParams> &params,
	                  const std::shared_ptr<const Address> &localAddress,
	                  const std::shared_ptr<const Address> &remoteAddress,
	                  const std::list<std::shared_ptr<Address>> &participants);
	ConferenceContext(ConferenceContext &&other) = delete;
	ConferenceContext(const ConferenceContext &other) = delete;
	virtual ~ConferenceContext() = default;

	inline const std::shared_ptr<const ConferenceParams> getParams() const {
		return mParams;
	}
	inline const std::list<std::shared_ptr<Address>> &getParticipants() const {
		return mParticipants;
	}
	inline const Address &getLocalAddress() const {
		return mLocalAddress;
	}
	inline const Address &getRemoteAddress() const {
		return mRemoteAddress;
	}

	bool operator==(const ConferenceContext &other) const;

private:
	std::shared_ptr<const ConferenceParams> mParams;
	std::list<std::shared_ptr<Address>> mParticipants;
	Address mLocalAddress;
	Address mRemoteAddress;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_CONTEXT_H_
