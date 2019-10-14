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

#ifndef _L_CONFERENCE_P_H_
#define _L_CONFERENCE_P_H_

#include "address/identity-address.h"
#include "conference.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class CallSessionListener;
class Participant;

class ConferencePrivate {
public:
	virtual ~ConferencePrivate () = default;

	IdentityAddress conferenceAddress;
	std::list<std::shared_ptr<Participant>> participants;
	std::string subject;

protected:
	std::shared_ptr<Participant> activeParticipant;
	std::shared_ptr<Participant> me;

	CallSessionListener *listener = nullptr;

	Conference *mPublic = nullptr;

private:
	L_DECLARE_PUBLIC(Conference);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_P_H_
