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

#ifndef _L_IDENTITY_ADDRESS_H_
#define _L_IDENTITY_ADDRESS_H_

#include <ostream>
#include <string>

#include "address.h"
#include "linphone/utils/general.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC IdentityAddress : protected Address {
public:
	explicit IdentityAddress (const std::string &address);
	IdentityAddress (const Address &address);
	IdentityAddress (const IdentityAddress &other);
	IdentityAddress ();
	virtual ~IdentityAddress () = default;

	virtual IdentityAddress *clone () const override {
		return new IdentityAddress(*this);
	}

	IdentityAddress &operator= (const IdentityAddress &other);

	bool operator== (const IdentityAddress &other) const;
	bool operator!= (const IdentityAddress &other) const;

	bool operator< (const IdentityAddress &other) const;

	bool isValid () const;

	const std::string &getScheme () const;
	void setScheme (const std::string &scheme);

	const std::string &getUsername () const;
	void setUsername (const std::string &username);

	const std::string &getDomain () const;
	void setDomain (const std::string &domain);

	bool hasGruu () const;
	const std::string &getGruu () const;
	void setGruu (const std::string &gruu);

	IdentityAddress getAddressWithoutGruu () const;

	virtual std::string asString () const;

	const Address & asAddress() const;

	// This method is necessary when creating static variables of type address as they canot be freed before the leak detector runs
	void removeFromLeakDetector() const;

private:
	void fillFromAddress(const Address &address);
};

inline std::ostream &operator<< (std::ostream &os, const IdentityAddress &identityAddress) {
	os << "IdentityAddress(" << identityAddress.asString() << ")";
	return os;
}
class LINPHONE_PUBLIC ConferenceAddress : public IdentityAddress {
public:
	ConferenceAddress (const std::string &address);
	ConferenceAddress (const Address &address);
	ConferenceAddress (const IdentityAddress &other);
	ConferenceAddress (const ConferenceAddress &other);
	ConferenceAddress() : IdentityAddress(){};
	virtual ~ConferenceAddress () = default;
	
	ConferenceAddress *clone () const override {
		return new ConferenceAddress(*this);
	}

	ConferenceAddress &operator= (const ConferenceAddress &other);
	ConferenceAddress &operator= (const IdentityAddress &other){
		if (this != &other) {
			IdentityAddress::operator=(other);
		}
		return *this;
	} ;

	bool operator== (const ConferenceAddress &other) const;
	bool operator!= (const ConferenceAddress &other) const;

	bool operator< (const ConferenceAddress &other) const;

	virtual std::string asString () const override;

	bool hasConfId () const;
	const std::string &getConfId () const;
	void setConfId (const std::string &confId);

private:
	void fillUriParams (const Address &address);
	int compareUriParams (const bctbx_map_t* otherUriParamMap) const;
};

LINPHONE_END_NAMESPACE

// Add map key support.
namespace std {
	template<>
	struct hash<LinphonePrivate::IdentityAddress> {
		std::size_t operator() (const LinphonePrivate::IdentityAddress &identityAddress) const {
			if (!identityAddress.isValid()) return std::size_t(-1);
			return hash<string>()(identityAddress.asString());
		}
	};
}

#endif // ifndef _L_IDENTITY_ADDRESS_H_
