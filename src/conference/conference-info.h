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

#include <belle-sip/object++.hh>
#include "linphone/api/c-types.h"
#include "linphone/types.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC ConferenceInfo : public bellesip::HybridObject<LinphoneConferenceInfo, ConferenceInfo> {
public:
	ConferenceInfo ();
	~ConferenceInfo ();

	const LinphoneAddress *getOrganizer () const;
	void setOrganizer (LinphoneAddress *organizer);

	const bctbx_list_t *getParticipants () const;
	void setParticipants (bctbx_list_t *participants);
	void addParticipant (LinphoneAddress *participant);

	const LinphoneAddress *getUri () const;
	void setUri (LinphoneAddress *uri);

	time_t getDateTime () const;
	void setDateTime (time_t dateTime);

	int getDuration () const;
	void setDuration (int duration);

	const std::string &getSubject () const;
	void setSubject (const std::string &subject);

	const std::string &getDescription () const;
	void setDescription (const std::string &description);

	const std::string toIcsString () const;

private:
	LinphoneAddress *mOrganizer = nullptr;
	bctbx_list_t *mParticipants = nullptr;
	LinphoneAddress *mUri = nullptr;
	time_t mDateTime;
	int mDuration = 0;
	std::string mSubject = "";
	std::string mDescription = "";
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_INFO_H_
