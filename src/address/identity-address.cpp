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
	return getUsername() == other.getUsername() && getDomain() == other.getDomain() && getGruu() == other.getGruu();
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
	fillUriParams(address);
};
ConferenceAddress::ConferenceAddress (const std::string &address) : ConferenceAddress(Address(address)) {
}
ConferenceAddress::ConferenceAddress (const ConferenceAddress &other) :IdentityAddress(other) {
	fillUriParams(other);
}

ConferenceAddress::ConferenceAddress (const IdentityAddress &other) :IdentityAddress(other) {

}
ConferenceAddress &ConferenceAddress::operator= (const ConferenceAddress &other) {
	if (this != &other) {
		IdentityAddress::operator=(other);
		fillUriParams(other);
	}
	return *this;
}

bool ConferenceAddress::operator== (const ConferenceAddress &other) const {
	return Address::operator==(other);
}

bool ConferenceAddress::operator!= (const ConferenceAddress &other) const {
	return !(*this == other);
}

bool ConferenceAddress::operator< (const ConferenceAddress &other) const {
	return Address::operator<(other);
}

string ConferenceAddress::asString () const {
	std::string addressStr = IdentityAddress::asString();
	bctbx_map_t* uriParamMap = getUriParams();
	bctbx_iterator_t * uriParamMapEnd = bctbx_map_cchar_end(uriParamMap);
	bctbx_iterator_t * it = bctbx_map_cchar_begin(uriParamMap);
	for (;!bctbx_iterator_cchar_equals(it,uriParamMapEnd); it = bctbx_iterator_cchar_get_next(it)) {
		bctbx_pair_t *pair = bctbx_iterator_cchar_get_pair(it);
		const char * key = bctbx_pair_cchar_get_first(reinterpret_cast<bctbx_pair_cchar_t *>(pair));
		// GRUU is already added by Identity address asString() function
		if (strcmp(key, "gr") != 0) {
			const char * value = (const char *)bctbx_pair_cchar_get_second(pair);
			addressStr = addressStr + ";" + key;
			if (value) {
				addressStr = addressStr  + "=" + value;
			}
		}
	}
	bctbx_iterator_cchar_delete(it);
	bctbx_iterator_cchar_delete(uriParamMapEnd);
	bctbx_mmap_cchar_delete_with_data(uriParamMap, bctbx_free);
	return addressStr;
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

void ConferenceAddress::fillUriParams (const Address &address) {
	bctbx_map_t* uriParamMap = address.getUriParams();
	bctbx_iterator_t * uriParamMapEnd = bctbx_map_cchar_end(uriParamMap);
	bctbx_iterator_t * it = bctbx_map_cchar_begin(uriParamMap);
	for (;!bctbx_iterator_cchar_equals(it,uriParamMapEnd); it = bctbx_iterator_cchar_get_next(it)) {
		bctbx_pair_t *pair = bctbx_iterator_cchar_get_pair(it);
		const char * key = bctbx_pair_cchar_get_first(reinterpret_cast<bctbx_pair_cchar_t *>(pair));
		const char * value = (const char *)bctbx_pair_cchar_get_second(pair);
		if (value) {
			setUriParam(key, value);
		} else {
			setUriParams(key);
		}
	}
	bctbx_iterator_cchar_delete(it);
	bctbx_iterator_cchar_delete(uriParamMapEnd);
	bctbx_mmap_cchar_delete_with_data(uriParamMap, bctbx_free);
}

int ConferenceAddress::compareUriParams (const bctbx_map_t* otherUriParamMap) const {
	bctbx_map_t* thisUriParamMap = getUriParams();
	// Check that this and other uri parameter maps have the same number of elements
	size_t thisMapSize = bctbx_map_cchar_size(thisUriParamMap);
	size_t otherMapSize = bctbx_map_cchar_size(otherUriParamMap);
	int diff = (int)(thisMapSize - otherMapSize);

	bctbx_iterator_t * thisUriParamMapEnd = bctbx_map_cchar_end(thisUriParamMap);
	bctbx_iterator_t * otherUriParamMapEnd = bctbx_map_cchar_end(otherUriParamMap);

	bctbx_iterator_t * thisIt = bctbx_map_cchar_begin(thisUriParamMap);

	// Loop through URI parameter map until:
	// - diff is 0
	// - end of map has not been reached
	while((diff == 0) && (!bctbx_iterator_cchar_equals(thisIt, thisUriParamMapEnd))) {
		bctbx_pair_t *thisPair = bctbx_iterator_cchar_get_pair(thisIt);
		const char * thisKey = bctbx_pair_cchar_get_first(reinterpret_cast<bctbx_pair_cchar_t *>(thisPair));
		const char * thisValue = (const char *)bctbx_pair_cchar_get_second(thisPair);

		bctbx_iterator_t * otherIt = bctbx_map_cchar_find_key(otherUriParamMap, thisKey);
		// Test that key exists
		if (!bctbx_iterator_cchar_equals(otherIt, otherUriParamMapEnd)) {
			bctbx_pair_t *otherPair = bctbx_iterator_cchar_get_pair(otherIt);
			const char * otherValue = (const char *)bctbx_pair_cchar_get_second(otherPair);
			if ((otherValue == NULL) || (thisValue == NULL)) {
				// keep diff at 0 if both other and this value are NULL
				diff = ((otherValue == NULL) && (thisValue == NULL)) ? 0 : -1;
			} else {
				diff = strcmp(thisValue, otherValue);
			}
		} else {
			diff = -1;
		}

		bctbx_iterator_cchar_delete(otherIt);
		thisIt = bctbx_iterator_cchar_get_next(thisIt);
	}

	bctbx_mmap_cchar_delete_with_data(thisUriParamMap, bctbx_free);
	bctbx_iterator_cchar_delete(thisIt);
	bctbx_iterator_cchar_delete(thisUriParamMapEnd);
	bctbx_iterator_cchar_delete(otherUriParamMapEnd);

	return diff;
}
LINPHONE_END_NAMESPACE
