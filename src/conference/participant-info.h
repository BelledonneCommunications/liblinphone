/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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

#ifndef _L_PARTICIPANT_INFO_H_
#define _L_PARTICIPANT_INFO_H_

#include "belle-sip/object++.hh"

#include "conference/participant.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Address;

class LINPHONE_PUBLIC ParticipantInfo : public bellesip::HybridObject<LinphoneParticipantInfo, ParticipantInfo> {
public:
	using participant_params_t = std::map<std::string, std::string>;

	static const std::string sequenceParameter;
	static const std::string roleParameter;

	ParticipantInfo(const std::string &ccmpUri);
	ParticipantInfo(const std::shared_ptr<const Address> &address);
	virtual ~ParticipantInfo();

	ParticipantInfo(const ParticipantInfo &other);

	ParticipantInfo *clone() const override;

	void setAddress(const std::shared_ptr<const Address> &address);
	const std::shared_ptr<Address> &getAddress() const;

	void setCcmpUri(const std::string &ccmpUri);
	std::string getCcmpUri() const;
	const char *getCcmpUriCstr() const;

	void setRole(Participant::Role role);
	Participant::Role getRole() const;

	void setSequenceNumber(const int &nb);
	const int &getSequenceNumber() const;

	void setParameters(const participant_params_t &params);
	void addParameter(const std::string &name, const std::string &value);
	void addParameters(const participant_params_t &params);
	const std::string &getParameterValue(const std::string &param) const;
	bool hasParameter(const std::string &name) const;
	void removeParameter(const std::string &name);

	// getParameters() return only parameters that do not have a dedicated member in the class, i.e. mParameters
	ParticipantInfo::participant_params_t getParameters() const;
	ParticipantInfo::participant_params_t getAllParameters() const;

	static const std::string memberParametersToString(const participant_params_t &params);
	static const participant_params_t stringToMemberParameters(const std::string &params);

private:
	std::string mCcmpUri;
	std::shared_ptr<Address> mAddress;
	Participant::Role mRole = Participant::Role::Unknown;
	int mSequence = -1;
	participant_params_t mParameters;

	mutable char *mCcmpUriCstr = nullptr;
};

inline std::ostream &operator<<(std::ostream &ostr, const ParticipantInfo &participantInfo) {
	const auto &address = participantInfo.getAddress();
	const auto addressString = (address ? address->toString() : std::string("sip:"));
	const auto &ccmpUri = participantInfo.getCcmpUri();
	const auto ccmpUriString = ccmpUri.empty() ? std::string("xcon:") : ccmpUri;
	ostr << "ParticipantInfo [" << (void *)&participantInfo << "]  (address: " << addressString
	     << " - CCMP uri: " << ccmpUriString << ")";
	return ostr;
}

LINPHONE_END_NAMESPACE

#endif // _L_PARTICIPANT_INFO_H_
