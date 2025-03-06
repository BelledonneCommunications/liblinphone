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

#include "participant-info.h"
#include "address/address.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

const std::string ParticipantInfo::sequenceParameter = "X-SEQ";
const std::string ParticipantInfo::roleParameter = "X-ROLE";

ParticipantInfo::ParticipantInfo(const std::string &ccmpUri) {
	mCcmpUri = ccmpUri;
}

ParticipantInfo::ParticipantInfo(const std::shared_ptr<const Address> &address) {
	setAddress(address);
}

ParticipantInfo::~ParticipantInfo() {
	if (mCcmpUriCstr) {
		ms_free(mCcmpUriCstr);
		mCcmpUriCstr = nullptr;
	}
}

ParticipantInfo *ParticipantInfo::clone() const {
	return new ParticipantInfo(*this);
}

ParticipantInfo::ParticipantInfo(const ParticipantInfo &other) : HybridObject(other) {
	mCcmpUri = other.mCcmpUri;
	mAddress = other.mAddress;
	mRole = other.mRole;
	mSequence = other.mSequence;
	mParameters = other.mParameters;
}

void ParticipantInfo::setAddress(const std::shared_ptr<const Address> &address) {
	mAddress = Address::create(address->getUri());
}

const std::shared_ptr<Address> &ParticipantInfo::getAddress() const {
	return mAddress;
}

void ParticipantInfo::setRole(Participant::Role role) {
	mRole = role;
};

Participant::Role ParticipantInfo::getRole() const {
	return mRole;
};

void ParticipantInfo::setCcmpUri(const std::string &ccmpUri) {
	mCcmpUri = ccmpUri;
};

const char *ParticipantInfo::getCcmpUriCstr() const {
	if (mCcmpUriCstr) {
		ms_free(mCcmpUriCstr);
		mCcmpUriCstr = nullptr;
	}
	if (!mCcmpUri.empty()) {
		mCcmpUriCstr = ms_strdup(mCcmpUri.c_str());
	}
	return mCcmpUriCstr;
};

std::string ParticipantInfo::getCcmpUri() const {
	return mCcmpUri;
};

void ParticipantInfo::setSequenceNumber(const int &nb) {
	mSequence = nb;
};

const int &ParticipantInfo::getSequenceNumber() const {
	return mSequence;
};

void ParticipantInfo::setParameters(const ParticipantInfo::participant_params_t &params) {
	mParameters.clear();
	addParameters(params);
}

void ParticipantInfo::addParameter(const std::string &name, const std::string &value) {
	if (name.compare(ParticipantInfo::sequenceParameter) == 0) {
		setSequenceNumber(std::stoi(value));
	} else if (name.compare(ParticipantInfo::roleParameter) == 0) {
		setRole(Participant::textToRole(value));
	} else {
		mParameters[name] = value;
	}
}

void ParticipantInfo::addParameters(const ParticipantInfo::participant_params_t &params) {
	for (const auto &[name, value] : params) {
		addParameter(name, value);
	}
}

bool ParticipantInfo::hasParameter(const std::string &name) const {
	return (mParameters.find(name) == mParameters.end());
}

const std::string &ParticipantInfo::getParameterValue(const std::string &name) const {
	try {
		return mParameters.at(name);
	} catch (std::out_of_range &) {
		lInfo() << "Unable to find parameter " << name << " associated to participant info " << this << " with address "
		        << *getAddress();
		return Utils::getEmptyConstRefObject<string>();
	}
}

void ParticipantInfo::removeParameter(const std::string &name) {
	auto it = mParameters.find(name);
	if (it == mParameters.end()) {
		lInfo() << "Unable to remove parameter " << name << " associated to participant info " << this
		        << " with address " << *getAddress();
	} else {
		mParameters.erase(it);
	}
}

ParticipantInfo::participant_params_t ParticipantInfo::getParameters() const {
	return mParameters;
}

ParticipantInfo::participant_params_t ParticipantInfo::getAllParameters() const {
	auto params = mParameters;
	if (mSequence >= 0) {
		params[ParticipantInfo::sequenceParameter] = std::to_string(mSequence);
	}
	const auto &role = getRole();
	if (role != Participant::Role::Unknown) {
		params[ParticipantInfo::roleParameter] = Participant::roleToText(role);
	}
	return params;
}

const std::string ParticipantInfo::memberParametersToString(const ParticipantInfo::participant_params_t &params) {
	std::string str;
	for (const auto &[name, value] : params) {
		if (!str.empty()) {
			str.append(";");
		}
		str.append(name + "=" + value);
	}
	return str;
}

const ParticipantInfo::participant_params_t ParticipantInfo::stringToMemberParameters(const std::string &paramsString) {
	ParticipantInfo::participant_params_t params;
	if (!paramsString.empty()) {
		const auto &splittedValue = bctoolbox::Utils::split(Utils::trim(paramsString), ";");
		for (const auto &param : splittedValue) {
			auto equal = param.find("=");
			string name = param.substr(0, equal);
			string value = param.substr(equal + 1, param.size());
			params.insert(std::make_pair(name, value));
		}
	}

	return params;
}

LINPHONE_END_NAMESPACE
