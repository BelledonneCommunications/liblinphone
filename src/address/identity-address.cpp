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
		setScheme(parsedAddress->getScheme());
		tmp = belle_sip_to_unescaped_string(parsedAddress->getUsername().c_str());
		setUsername(tmp);
		ms_free(tmp);
		setDomain(parsedAddress->getDomain());
		setGruu(parsedAddress->getGruu());
	} else {
		Address tmpAddress(address);
		fillFromAddress(tmpAddress);
	}
}

IdentityAddress::IdentityAddress (const Address &address) {
	fillFromAddress(address);
}

void IdentityAddress::fillFromAddress(const Address &address) {
	if (address.isValid() && ((address.getScheme() == "sip") || (address.getScheme() == "sips"))) {
		setScheme(address.getScheme());
		setUsername(address.getUsername());
		setDomain(address.getDomain());
		if (address.hasUriParam("gr")) {
			setGruu(address.getUriParamValue("gr"));
		}
	}
}

IdentityAddress::IdentityAddress (const IdentityAddress &other) : Address(other) {

}

IdentityAddress::IdentityAddress () {
	
}

IdentityAddress &IdentityAddress::operator= (const IdentityAddress &other) {
	if (this != &other) {
		Address::operator= (other);
	}
	return *this;
}

bool IdentityAddress::operator== (const IdentityAddress &other) const {
	/* Scheme is not used for comparison. sip:toto@sip.linphone.org and sips:toto@sip.linphone.org refer to the same person. */
	// Operator < ignores scheme
	// If this is not smaller than other and other is not smaller than this, it means that they are identical
	return !(*this < other) && !(other < *this);
}

bool IdentityAddress::operator!= (const IdentityAddress &other) const {
	return !(*this == other);
}

bool IdentityAddress::operator< (const IdentityAddress &other) const {
	int diff = getUsername().compare(other.getUsername());
	if (diff == 0){
		diff = getDomain().compare(other.getDomain());
		if (diff == 0){
			diff = getGruu().compare(other.getGruu());
		}
	}
	return diff < 0;
}

bool IdentityAddress::isValid () const {
	return !getScheme().empty() && !getDomain().empty();
}

const string &IdentityAddress::getScheme () const {
	return Address::getScheme();
}

void IdentityAddress::setScheme (const string &scheme) {
	Address::setSecure (scheme.compare("sips") == 0);
}

const string &IdentityAddress::getUsername () const {
	return Address::getUsername();
}

void IdentityAddress::setUsername (const string &username) {
	Address::setUsername(username);
}

const string &IdentityAddress::getDomain () const {
	return Address::getDomain();
}

void IdentityAddress::setDomain (const string &domain) {
	Address::setDomain(domain);
}

bool IdentityAddress::hasGruu () const {
	return hasUriParam("gr");
}

const string &IdentityAddress::getGruu () const {
	return getUriParamValue("gr");
}

void IdentityAddress::setGruu (const string &gruu) {
	if (gruu.empty() == true) {
		removeUriParam ("gr");
	} else {
		setUriParam("gr",gruu);
	}
}

IdentityAddress IdentityAddress::getAddressWithoutGruu () const {
	Address address(*this);
	address.removeUriParam("gr");
	return address;
}

string IdentityAddress::asString () const {
	ostringstream res;
	res << getScheme() << ":";
	if (!getUsername().empty()){
		char *tmp = belle_sip_uri_to_escaped_username(getUsername().c_str());
		res << tmp << "@";
		ms_free(tmp);
	}

	if (getDomain().find(":") != string::npos) {
		res << "[" << getDomain() << "]";
	} else {
		res << getDomain();
	}

	if (!getGruu().empty()){
		res << ";gr=" << getGruu();
	}
	return res.str();
}

const Address & IdentityAddress::asAddress() const {
	return *this;
}

void IdentityAddress::removeFromLeakDetector() const {
	Address::removeFromLeakDetector();
}

ConferenceAddress::ConferenceAddress (const Address &address) :IdentityAddress(address) {
	if (address.hasUriParam("conf-id")) {
		setConfId(address.getUriParamValue("conf-id"));
	}
};
ConferenceAddress::ConferenceAddress (const std::string &address) : ConferenceAddress(Address(address)) {
}
ConferenceAddress::ConferenceAddress (const ConferenceAddress &other) :IdentityAddress(other) {
	if (other.getConfId().empty() == false) {
		setConfId(other.getConfId());
	}
}
ConferenceAddress::ConferenceAddress (const IdentityAddress &other) :IdentityAddress(other) {
}
ConferenceAddress &ConferenceAddress::operator= (const ConferenceAddress &other) {
	if (this != &other) {
		IdentityAddress::operator=(other);
		if (other.getConfId().empty() == false) {
			setConfId(other.getConfId());
		}
	}
	return *this;
}

bool ConferenceAddress::operator== (const IdentityAddress &other) const {
	/* Scheme is not used for comparison. sip:toto@sip.linphone.org and sips:toto@sip.linphone.org refer to the same person. */
	// Operator < ignores scheme
	// If this is not smaller than other and other is not smaller than this, it means that they are identical
	return !(*this < other) && !(other < *this);
}

bool ConferenceAddress::operator!= (const IdentityAddress &other) const {
	return !(*this == other);
}

bool ConferenceAddress::operator< (const ConferenceAddress &other) const {
	int diff = IdentityAddress::operator<(other);
	if (diff == 0){
		diff = getConfId().compare(other.getConfId());
	}
	return diff < 0;
}

string ConferenceAddress::asString () const {
	if (hasConfId())
		return IdentityAddress::asString() + ";conf-id="+getConfId();
	else {
		return IdentityAddress::asString();
	}
}

const string &ConferenceAddress::getConfId () const {
	return getUriParamValue("conf-id");
}

void ConferenceAddress::setConfId (const string &confId) {
	setUriParam("conf-id", confId);
}

bool ConferenceAddress::hasConfId () const {
	return hasUriParam("conf-id");
}

LINPHONE_END_NAMESPACE
