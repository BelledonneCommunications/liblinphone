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

#include <belle-sip/utils.h>
#include "linphone/utils/utils.h"

#include "address.h"
#include "identity-address-parser.h"

#include "c-wrapper/c-wrapper.h"
#include "identity-address.h"
#include "logger/logger.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

IdentityAddress::IdentityAddress (const string &address) {
	shared_ptr<IdentityAddress> parsedAddress = IdentityAddressParser::getInstance()->parseAddress(address);
	if (parsedAddress != nullptr) {
		char *tmp;
		scheme = parsedAddress->getScheme();
		tmp = belle_sip_to_unescaped_string(parsedAddress->getUsername().c_str());
		username = tmp;
		ms_free(tmp);
		domain = parsedAddress->getDomain();
		gruu = parsedAddress->getGruu();
	} else {
		Address tmpAddress(address);
		if (tmpAddress.isValid() && ((tmpAddress.getScheme() == "sip") || (tmpAddress.getScheme() == "sips"))) {
			scheme = tmpAddress.getScheme();
			username = tmpAddress.getUsername();
			domain = tmpAddress.getDomain();
			gruu = tmpAddress.getUriParamValue("gr");
		}
	}
}

IdentityAddress::IdentityAddress (const Address &address) {
	scheme = address.getScheme();
	username = address.getUsername();
	domain = address.getDomain();
	if (address.hasUriParam("gr"))
		gruu = address.getUriParamValue("gr");
}

IdentityAddress::IdentityAddress (const IdentityAddress &other) {
	scheme = other.getScheme();
	username = other.getUsername();
	domain = other.getDomain();
	gruu = other.getGruu();
}

IdentityAddress::IdentityAddress () {
	
}

IdentityAddress &IdentityAddress::operator= (const IdentityAddress &other) {
	if (this != &other) {
		scheme = other.getScheme();
		username = other.getUsername();
		domain = other.getDomain();
		gruu = other.getGruu();
	}
	return *this;
}

bool IdentityAddress::operator== (const IdentityAddress &other) const {
	/* Scheme is not used for comparison. sip:toto@sip.linphone.org and sips:toto@sip.linphone.org refer to the same person. */
	return username == other.getUsername() && domain == other.getDomain() && gruu == other.getGruu();
}

bool IdentityAddress::operator!= (const IdentityAddress &other) const {
	return !(*this == other);
}

bool IdentityAddress::operator< (const IdentityAddress &other) const {
	int diff = username.compare(other.getUsername());
	if (diff == 0){
		diff = domain.compare(other.getDomain());
		if (diff == 0){
			diff = gruu.compare(other.getGruu());
		}
	}
	return diff < 0;
}

bool IdentityAddress::isValid () const {
	return !scheme.empty() && !domain.empty();
}

const string &IdentityAddress::getScheme () const {
	return scheme;
}

void IdentityAddress::setScheme (const string &scheme) {
	this->scheme = scheme;
}

const string &IdentityAddress::getUsername () const {
	return username;
}

void IdentityAddress::setUsername (const string &username) {
	this->username = username;
}

const string &IdentityAddress::getDomain () const {
	return domain;
}

void IdentityAddress::setDomain (const string &domain) {
	this->domain = domain;
}

bool IdentityAddress::hasGruu () const {
	return !gruu.empty();
}

const string &IdentityAddress::getGruu () const {
	return gruu;
}

void IdentityAddress::setGruu (const string &gruu) {
	this->gruu = gruu;
}

IdentityAddress IdentityAddress::getAddressWithoutGruu () const {
	IdentityAddress address(*this);
	address.setGruu("");
	return address;
}

string IdentityAddress::asString () const {
	ostringstream res;
	res << scheme << ":";
	if (!username.empty()){
		char *tmp = belle_sip_uri_to_escaped_username(username.c_str());
		res << tmp << "@";
		ms_free(tmp);
	}
	
	if (domain.find(":") != string::npos) {
		res << "[" << domain << "]";
	} else {
		res << domain;
	}
	
	if (!gruu.empty()){
		res << ";gr=" << gruu;
	}
	return res.str();
}

ConferenceAddress::ConferenceAddress (const Address &address) :IdentityAddress(address) {
	mConfId= address.getUriParamValue("conf-id");
};
ConferenceAddress::ConferenceAddress (const std::string &address) : ConferenceAddress(Address(address)) {
}
ConferenceAddress::ConferenceAddress (const ConferenceAddress &other) :IdentityAddress(other) {
	mConfId = other.mConfId;
}
ConferenceAddress::ConferenceAddress (const IdentityAddress &other) :IdentityAddress(other) {
}
ConferenceAddress &ConferenceAddress::operator= (const ConferenceAddress &other) {
	if (this != &other) {
		IdentityAddress::operator=(other);
		mConfId = other.mConfId;
	}
	return *this;
}

bool ConferenceAddress::operator== (const ConferenceAddress &other) const {
	return IdentityAddress::operator==(other) && mConfId == other.mConfId;
};
bool ConferenceAddress::operator!= (const ConferenceAddress &other) const {
	return !operator==(other);
}
bool ConferenceAddress::operator< (const ConferenceAddress &other) const {
	int diff = IdentityAddress::operator<(other);
	if (diff == 0){
		diff = mConfId.compare(other.mConfId);
	}
	return diff < 0;
}

std::string ConferenceAddress::asString () const {
	if (mConfId.empty())
		return IdentityAddress::asString();
	else {
		return IdentityAddress::asString() + ";conf-id="+mConfId;
	}
}

LINPHONE_END_NAMESPACE
