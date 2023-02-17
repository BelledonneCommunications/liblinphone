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

#ifndef _L_CONFERENCE_INFO_H_
#define _L_CONFERENCE_INFO_H_

#include <ctime>
#include <map>
#include <string>

#include "address/address.h"
#include "address/identity-address.h"

#include "belle-sip/object++.hh"
#include "linphone/api/c-types.h"
#include "linphone/types.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

/* Temporary utility for non HybridObject, that does almost the same as ListHolder
 FIXME: it has to be declared in c-wrapper.h like other c-wrapping utilities, however this cannot be
 done because of circular dependencies within liblinphone internal include files (linphone/utils/utils.h is problematic).
 This has to be fixed first.
 */
class CListCache{
public:
	CListCache() = default;
	CListCache(const CListCache& other) : mList(nullptr){
		(void)other;
	}
	template<typename _container, typename _functor>
	const bctbx_list_t * construct(const _container & container, _functor fun) const{
		if (mList) {
			bctbx_list_free_with_data(mList, belle_sip_object_unref);
			mList = nullptr;
		}
		for (const auto & obj : container){
			mList = bctbx_list_append(mList, fun(obj));
		}
		return mList;
	}
	~CListCache(){
		if (mList) bctbx_list_free_with_data(mList, belle_sip_object_unref);
	}
private:
	mutable bctbx_list_t *mList = nullptr;
};

class LINPHONE_PUBLIC ConferenceInfo : public bellesip::HybridObject<LinphoneConferenceInfo, ConferenceInfo> {
public:
	static const std::string sequenceParam;
	using participant_params_t = std::map<std::string, std::string>;
	using participant_list_t = std::map<IdentityAddress, participant_params_t>;
	using organizer_t = std::pair<IdentityAddress, participant_params_t>;

	enum class State {
		New = LinphoneConferenceInfoStateNew,
		Updated = LinphoneConferenceInfoStateUpdated,
		Cancelled = LinphoneConferenceInfoStateCancelled,
	};

	ConferenceInfo ();
	~ConferenceInfo();

	ConferenceInfo *clone()const override{
		return new ConferenceInfo(*this);
	}

	static const std::string memberParametersToString(const participant_params_t & params);
	static const participant_params_t stringToMemberParameters(const std::string & params);

	const organizer_t &getOrganizer () const;
	const IdentityAddress &getOrganizerAddress () const;
	void setOrganizer (const IdentityAddress & organizer, const participant_params_t & params);
	void setOrganizer (const IdentityAddress & organizer);

	void addOrganizerParam (const std::string & param, const std::string & value);
	const std::string getOrganizerParam (const std::string & param) const;

	const participant_list_t &getParticipants () const;
	const bctbx_list_t *getParticipantsCList() const;
	void setParticipants (const participant_list_t & participants);
	void addParticipant (const IdentityAddress & participant);
	void addParticipant (const IdentityAddress & participant, const participant_params_t & params);
	void removeParticipant (const IdentityAddress & participant);

	void addParticipantParam (const IdentityAddress & participant, const std::string & param, const std::string & value);
	const std::string getParticipantParam (const IdentityAddress & participant, const std::string & param) const;

	bool isValidUri () const;
	const ConferenceAddress &getUri () const;
	void setUri (const ConferenceAddress uri);

	time_t getDateTime () const;
	void setDateTime (time_t dateTime);

	unsigned int getDuration () const;
	void setDuration (unsigned int duration);

	const std::string &getSubject () const;
	const std::string getUtf8Subject () const;
	void setSubject (const std::string &subject);
	void setUtf8Subject (const std::string &subject);

	unsigned int getIcsSequence () const;
	void setIcsSequence (unsigned int icsSequence);

	const std::string &getIcsUid () const;
	const std::string getUtf8IcsUid () const;
	void setIcsUid (const std::string &uid);
	void setUtf8IcsUid (const std::string &uid);

	const std::string &getDescription () const;
	const std::string getUtf8Description () const;
	void setDescription (const std::string &description);
	void setUtf8Description (const std::string &description);

	const ConferenceInfo::State &getState () const;
	void setState (const ConferenceInfo::State &state);

	const std::string toIcsString (bool cancel = false, int sequence = -1) const;

	void updateFrom (const std::shared_ptr<ConferenceInfo> & info);

	// Used only by the tester
	void setCreationTime(time_t time);
private:
	organizer_t mOrganizer;
	participant_list_t mParticipants;
	ConferenceAddress mUri;
	time_t mDateTime = (time_t) -1;
	unsigned int mDuration = 0;
	std::string mSubject = "";
	std::string mDescription = "";
	mutable unsigned int mIcsSequence = 0;
	mutable std::string mIcsUid = "";
	State mState = State::New;
	CListCache mParticipantsList;
	time_t mCreationTime = (time_t) -1;
};

std::ostream& operator<<(std::ostream& lhs, ConferenceInfo::State s);

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_INFO_H_
