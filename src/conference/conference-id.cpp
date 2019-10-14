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

#include "object/clonable-object-p.h"

#include "conference-id.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

class ConferenceIdPrivate : public ClonableObjectPrivate {
public:
	IdentityAddress peerAddress;
	IdentityAddress localAddress;
};

// -----------------------------------------------------------------------------

ConferenceId::ConferenceId () : ClonableObject(*new ConferenceIdPrivate) {}

ConferenceId::ConferenceId (
	const IdentityAddress &peerAddress,
	const IdentityAddress &localAddress
) : ClonableObject(*new ConferenceIdPrivate) {
	L_D();
	d->peerAddress = peerAddress;
	d->localAddress = localAddress;
}

L_USE_DEFAULT_CLONABLE_OBJECT_SHARED_IMPL(ConferenceId);

bool ConferenceId::operator== (const ConferenceId &other) const {
	L_D();
	const ConferenceIdPrivate *dConferenceId = other.getPrivate();
	return d->peerAddress == dConferenceId->peerAddress && d->localAddress == dConferenceId->localAddress;
}

bool ConferenceId::operator!= (const ConferenceId &other) const {
	return !(*this == other);
}

bool ConferenceId::operator< (const ConferenceId &other) const {
	L_D();
	const ConferenceIdPrivate *dConferenceId = other.getPrivate();
	return d->peerAddress < dConferenceId->peerAddress
		|| (d->peerAddress == dConferenceId->peerAddress && d->localAddress < dConferenceId->localAddress);
}

const IdentityAddress &ConferenceId::getPeerAddress () const {
	L_D();
	return d->peerAddress;
}

const IdentityAddress &ConferenceId::getLocalAddress () const {
	L_D();
	return d->localAddress;
}

bool ConferenceId::isValid () const {
	L_D();
	return d->peerAddress.isValid() && d->localAddress.isValid();
}

LINPHONE_END_NAMESPACE
