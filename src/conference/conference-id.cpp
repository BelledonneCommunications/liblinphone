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

#include "conference-id.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

ConferenceId::ConferenceId () {}

ConferenceId::ConferenceId (
	const ConferenceAddress &peerAddress,
	const ConferenceAddress &localAddress
) {
	this->peerAddress = peerAddress;
	this->localAddress = localAddress;
}

ConferenceId::ConferenceId (const ConferenceId &other) :
	peerAddress(other.peerAddress),
	localAddress(other.localAddress)
{ }

ConferenceId &ConferenceId::operator= (const ConferenceId &other) {
	this->peerAddress = other.peerAddress;
	this->localAddress = other.localAddress;
	return *this;
}

bool ConferenceId::operator== (const ConferenceId &other) const {
	return peerAddress == other.peerAddress && localAddress == other.localAddress;
}

bool ConferenceId::operator!= (const ConferenceId &other) const {
	return !(*this == other);
}

bool ConferenceId::operator< (const ConferenceId &other) const {
	return peerAddress < other.peerAddress
		|| (peerAddress == other.peerAddress && localAddress < other.localAddress);
}

const ConferenceAddress &ConferenceId::getPeerAddress () const {
	return peerAddress;
}

const ConferenceAddress &ConferenceId::getLocalAddress () const {
	return localAddress;
}

bool ConferenceId::isValid () const {
	return peerAddress.isValid() && localAddress.isValid();
}

LINPHONE_END_NAMESPACE
