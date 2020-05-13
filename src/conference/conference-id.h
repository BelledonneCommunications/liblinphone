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

#ifndef _L_CONFERENCE_ID_H_
#define _L_CONFERENCE_ID_H_

#include "address/identity-address.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC ConferenceId {
public:
	ConferenceId ();
	ConferenceId (const ConferenceAddress &peerAddress, const ConferenceAddress &localAddress);
	ConferenceId (const ConferenceId &other);

	virtual ~ConferenceId() = default;

	ConferenceId *clone () const {
		return new ConferenceId(*this);
	}

	ConferenceId &operator= (const ConferenceId &other);

	bool operator== (const ConferenceId &other) const;
	bool operator!= (const ConferenceId &other) const;

	bool operator< (const ConferenceId &other) const;

	const IdentityAddress &getPeerAddress () const;
	const IdentityAddress &getLocalAddress () const;

	bool isValid () const;

private:

	ConferenceAddress peerAddress;
	ConferenceAddress localAddress;

};

inline std::ostream &operator<< (std::ostream &os, const ConferenceId &conferenceId) {
	os << "ConferenceId(peer=" << conferenceId.getPeerAddress() << ", local=" << conferenceId.getLocalAddress() << ")";
	return os;
}

LINPHONE_END_NAMESPACE

// Add map key support.
namespace std {
	template<>
	struct hash<LinphonePrivate::ConferenceId> {
		std::size_t operator() (const LinphonePrivate::ConferenceId &conferenceId) const {
			return hash<string>()(conferenceId.getPeerAddress().asString()) ^
				(hash<string>()(conferenceId.getLocalAddress().asString()) << 1);
		}
	};
}

#endif // ifndef _L_CONFERENCE_ID_H_
