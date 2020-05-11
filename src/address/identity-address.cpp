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
#include "object/clonable-object-p.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

class IdentityAddressPrivate : public ClonableObjectPrivate {
public:
	std::string scheme;
	std::string username;
	std::string domain;
	std::string gruu;
};

// -----------------------------------------------------------------------------

IdentityAddress::IdentityAddress (const string &address) : ClonableObject(*new IdentityAddressPrivate) {
	L_D();
	shared_ptr<IdentityAddress> parsedAddress = IdentityAddressParser::getInstance()->parseAddress(address);
	if (parsedAddress != nullptr) {
		char *tmp;
		d->scheme = parsedAddress->getScheme();
		tmp = belle_sip_to_unescaped_string(parsedAddress->getUsername().c_str());
		d->username = tmp;
		ms_free(tmp);
		d->domain = parsedAddress->getDomain();
		d->gruu = parsedAddress->getGruu();
	} else {
		Address tmpAddress(address);
		if (tmpAddress.isValid() && ((tmpAddress.getScheme() == "sip") || (tmpAddress.getScheme() == "sips"))) {
			d->scheme = tmpAddress.getScheme();
			d->username = tmpAddress.getUsername();
			d->domain = tmpAddress.getDomain();
			d->gruu = tmpAddress.getUriParamValue("gr");
		}
	}
}

IdentityAddress::IdentityAddress (const Address &address) : ClonableObject(*new IdentityAddressPrivate) {
	L_D();
	d->scheme = address.getScheme();
	d->username = address.getUsername();
	d->domain = address.getDomain();
	if (address.hasUriParam("gr"))
		d->gruu = address.getUriParamValue("gr");
}

IdentityAddress::IdentityAddress (const IdentityAddress &other) : ClonableObject(*new IdentityAddressPrivate) {
	L_D();
	d->scheme = other.getScheme();
	d->username = other.getUsername();
	d->domain = other.getDomain();
	d->gruu = other.getGruu();
}

IdentityAddress::IdentityAddress () : ClonableObject(*new IdentityAddressPrivate) {
	
}

IdentityAddress &IdentityAddress::operator= (const IdentityAddress &other) {
	L_D();
	if (this != &other) {
		d->scheme = other.getScheme();
		d->username = other.getUsername();
		d->domain = other.getDomain();
		d->gruu = other.getGruu();
	}
	return *this;
}

bool IdentityAddress::operator== (const IdentityAddress &other) const {
	L_D();
	/* Scheme is not used for comparison. sip:toto@sip.linphone.org and sips:toto@sip.linphone.org refer to the same person. */
	return d->username == other.getUsername() && d->domain == other.getDomain() && d->gruu == other.getGruu();
}

bool IdentityAddress::operator!= (const IdentityAddress &other) const {
	return !(*this == other);
}

bool IdentityAddress::operator< (const IdentityAddress &other) const {
	L_D();
	
	int diff = d->username.compare(other.getUsername());
	if (diff == 0){
		diff = d->domain.compare(other.getDomain());
		if (diff == 0){
			diff = d->gruu.compare(other.getGruu());
		}
	}
	return diff < 0;
}

bool IdentityAddress::isValid () const {
	L_D();
	return !d->scheme.empty() && !d->domain.empty();
}

const string &IdentityAddress::getScheme () const {
	L_D();
	return d->scheme;
}

void IdentityAddress::setScheme (const string &scheme) {
	L_D();
	d->scheme = scheme;
}

const string &IdentityAddress::getUsername () const {
	L_D();
	return d->username;
}

void IdentityAddress::setUsername (const string &username) {
	L_D();
	d->username = username;
}

const string &IdentityAddress::getDomain () const {
	L_D();
	return d->domain;
}

void IdentityAddress::setDomain (const string &domain) {
	L_D();
	d->domain = domain;
}

bool IdentityAddress::hasGruu () const {
	L_D();
	return !d->gruu.empty();
}

const string &IdentityAddress::getGruu () const {
	L_D();
	return d->gruu;
}

void IdentityAddress::setGruu (const string &gruu) {
	L_D();
	d->gruu = gruu;
}

IdentityAddress IdentityAddress::getAddressWithoutGruu () const {
	IdentityAddress address(*this);
	address.setGruu("");
	return address;
}

string IdentityAddress::asString () const {
	L_D();
	ostringstream res;
	res << d->scheme << ":";
	if (!d->username.empty()){
		char *tmp = belle_sip_uri_to_escaped_username(d->username.c_str());
		res << tmp << "@";
		ms_free(tmp);
	}
	
	if (d->domain.find(":") != string::npos) {
		res << "[" << d->domain << "]";
	} else {
		res << d->domain;
	}
	
	if (!d->gruu.empty()){
		res << ";gr=" << d->gruu;
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
