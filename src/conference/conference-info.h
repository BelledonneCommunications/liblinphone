/*
 * Copyright (c) 2010-2021 Belledonne Communications SARL.
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

#ifndef _L_CONFERENCE_INFO_H_
#define _L_CONFERENCE_INFO_H_

#include <ctime>
#include <string>

#include "address/address.h"
#include "address/identity-address.h"

#include <belle-sip/object++.hh>
#include "linphone/api/c-types.h"
#include "linphone/types.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC ConferenceInfo : public bellesip::HybridObject<LinphoneConferenceInfo, ConferenceInfo> {
public:
	ConferenceInfo ();
	virtual ~ConferenceInfo ();

	const IdentityAddress &getOrganizer () const;
	void setOrganizer (IdentityAddress organizer);

	const std::list<IdentityAddress> &getParticipants () const;
	void setParticipants (const std::list<IdentityAddress> participants);
	void addParticipant (const IdentityAddress participant);

	const ConferenceAddress &getUri () const;
	void setUri (const ConferenceAddress uri);

	time_t getDateTime () const;
	void setDateTime (time_t dateTime);

	int getDuration () const;
	void setDuration (int duration);

	const std::string &getSubject () const;
	void setSubject (const std::string &subject);

	const std::string &getDescription () const;
	void setDescription (const std::string &description);

	const std::string toIcsString () const;

	// Used only by the tester
	void setCreationTime(time_t time);
private:
	IdentityAddress mOrganizer;
	std::list<IdentityAddress> mParticipants;
	ConferenceAddress mUri;
	time_t mDateTime = (time_t) -1;
	int mDuration = 0;
	std::string mSubject = "";
	std::string mDescription = "";

	time_t mCreationTime = (time_t) -1;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_INFO_H_
