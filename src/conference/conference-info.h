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

#include "belle-sip/object++.hh"

#include "address/address.h"
#include "c-wrapper/list-holder.h"
#include "conference/conference-params-interface.h"
#include "linphone/api/c-types.h"
#include "linphone/types.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ParticipantInfo;
class LINPHONE_PUBLIC ConferenceInfo : public bellesip::HybridObject<LinphoneConferenceInfo, ConferenceInfo> {
public:
	struct AddressCmp {
		bool operator()(const std::shared_ptr<Address> &lhs, const std::shared_ptr<Address> &rhs) const {
			return *lhs < *rhs;
		}
	};

	using participant_list_t = std::list<std::shared_ptr<ParticipantInfo>>;
	using organizer_t = std::shared_ptr<ParticipantInfo>;

	enum class State {
		New = LinphoneConferenceInfoStateNew,
		Updated = LinphoneConferenceInfoStateUpdated,
		Cancelled = LinphoneConferenceInfoStateCancelled,
	};

	ConferenceInfo();

	ConferenceInfo *clone() const override {
		return new ConferenceInfo(*this);
	}

	const organizer_t &getOrganizer() const;
	const std::shared_ptr<Address> &getOrganizerAddress() const;
	void setOrganizer(const std::shared_ptr<const ParticipantInfo> &organizer);
	void setOrganizer(const std::shared_ptr<const Address> &organizer);
	bool isOrganizer(const std::shared_ptr<const ParticipantInfo> &participantInfo) const;

	const std::list<std::shared_ptr<Address>> &getParticipantAddressList() const;
	const bctbx_list_t *getParticipantAddressCList() const;
	const participant_list_t &getParticipants() const;
	const bctbx_list_t *getParticipantsCList() const;

	void setParticipants(const std::list<std::shared_ptr<Address>> &participants, bool logActivity = true);
	void setParticipants(const participant_list_t &participants, bool logActivity = true);

	void addParticipants(const std::list<std::shared_ptr<Address>> &participants, bool logActivity = true);
	void addParticipants(const participant_list_t &participants, bool logActivity = true);

	void addParticipant(const std::shared_ptr<const Address> &participant, bool logActivity = true);
	void addParticipant(const std::shared_ptr<const ParticipantInfo> &participantInfo, bool logActivity = true);

	void removeParticipant(const std::shared_ptr<const Address> &participant, bool logActivity = true);
	void removeParticipant(const std::shared_ptr<const ParticipantInfo> &participantInfo, bool logActivity = true);

	bool hasParticipant(const std::shared_ptr<const Address> &address) const;
	bool hasParticipant(const std::shared_ptr<const ParticipantInfo> &participantInfo) const;

	const std::shared_ptr<ParticipantInfo>
	findParticipant(const std::shared_ptr<const ParticipantInfo> &participantInfo) const;
	const std::shared_ptr<ParticipantInfo> findParticipant(const std::shared_ptr<const Address> &address) const;
	const std::shared_ptr<ParticipantInfo> findParticipant(const std::string &ccmpuri) const;

	void updateParticipant(const std::shared_ptr<const ParticipantInfo> &participantInfo);

	bool isValidUri() const;
	const std::shared_ptr<Address> &getUri() const;
	void setUri(const std::shared_ptr<const Address> uri);

	time_t getEarlierJoiningTime() const;
	void setEarlierJoiningTime(time_t joiningTime);

	time_t getExpiryTime() const;
	void setExpiryTime(time_t expiryTime);

	time_t getDateTime() const;
	void setDateTime(time_t dateTime);

	unsigned int getDuration() const;
	void setDuration(unsigned int duration);

	const std::string &getSubject() const;
	const std::string &getUtf8Subject() const;
	void setSubject(const std::string &subject);
	void setUtf8Subject(const std::string &subject);

	unsigned int getIcsSequence() const;
	void setIcsSequence(unsigned int icsSequence);

	const std::string &getCcmpUri() const;
	const std::string getUtf8CcmpUri() const;
	void setCcmpUri(const std::string &uid);
	void setUtf8CcmpUri(const std::string &uid);

	const std::string &getIcsUid() const;
	const std::string getUtf8IcsUid() const;
	void setIcsUid(const std::string &uid);
	void setUtf8IcsUid(const std::string &uid);

	const std::string &getDescription() const;
	const std::string &getUtf8Description() const;
	void setDescription(const std::string &description);
	void setUtf8Description(const std::string &description);

	const ConferenceInfo::State &getState() const;
	void setState(const ConferenceInfo::State &state);

	const std::string toIcsString(bool cancel = false, int sequence = -1) const;

	void updateFrom(const std::shared_ptr<ConferenceInfo> &info);

	ConferenceParamsInterface::SecurityLevel getSecurityLevel() const;
	void setSecurityLevel(ConferenceParamsInterface::SecurityLevel securityLevel);

	void setCapability(const LinphoneStreamType type, bool enable);
	bool getCapability(const LinphoneStreamType type) const;

	// Used only by the tester
	void setCreationTime(time_t time);

private:
	void updateParticipantAddresses() const;
	participant_list_t::const_iterator
	findParticipantIt(const std::shared_ptr<const ParticipantInfo> &participantInfo) const;
	participant_list_t::const_iterator findParticipantIt(const std::shared_ptr<const Address> &address) const;
	participant_list_t::const_iterator findParticipantIt(const std::string &ccmpUri) const;

	mutable std::string mCcmpUri = "";
	organizer_t mOrganizer;
	mutable std::shared_ptr<Address> mOrganizerAddress;
	participant_list_t mParticipants;
	mutable ListHolder<Address> mParticipantAddresses;
	mutable ListHolder<ParticipantInfo> mParticipantList;
	std::shared_ptr<Address> mUri;
	time_t mEarlierJoiningTime = -1;
	time_t mExpiryTime = -1;
	time_t mDateTime = -1;
	unsigned int mDuration = 0;
	mutable std::string mSubject;
	mutable std::string mDescription;
	std::string mSubjectUtf8;
	std::string mDescriptionUtf8;
	mutable unsigned int mIcsSequence = 0;
	mutable std::string mIcsUid = "";
	State mState = State::New;
	ConferenceParamsInterface::SecurityLevel mSecurityLevel = ConferenceParamsInterface::SecurityLevel::None;
	time_t mCreationTime = (time_t)-1;

	std::map<LinphoneStreamType, bool> capabilities;
};

std::ostream &operator<<(std::ostream &lhs, ConferenceInfo::State s);

inline std::ostream &operator<<(std::ostream &str, const ConferenceInfo &conferenceInfo) {
	const auto &uri = conferenceInfo.getUri();
	str << "ConferenceInfo [" << &conferenceInfo << "] (" << (uri ? uri->toString() : std::string("sip:")) << ")";
	return str;
}

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_INFO_H_
