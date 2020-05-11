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

#include "object/clonable-object.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Address;
class IdentityAddressPrivate;

class LINPHONE_PUBLIC IdentityAddress : public ClonableObject {
public:
	explicit IdentityAddress (const std::string &address);
	IdentityAddress (const Address &address);
	IdentityAddress (const IdentityAddress &other);
	IdentityAddress ();
	~IdentityAddress () = default;

	IdentityAddress *clone () const override {
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
	
	const std::string &getDisplayName () const;
	void setDisplayName (const std::string &displayName);

	const std::string &getDomain () const;
	void setDomain (const std::string &domain);

	bool hasGruu () const;
	const std::string &getGruu () const;
	void setGruu (const std::string &gruu);

	IdentityAddress getAddressWithoutGruu () const;

	virtual std::string asString () const;
	virtual std::string asStringUriOnly () const;

private:
	L_DECLARE_PRIVATE(IdentityAddress);
};

inline std::ostream &operator<< (std::ostream &os, const IdentityAddress &identityAddress) {
	os << "IdentityAddress(" << identityAddress.asString() << ")";
	return os;
}

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
