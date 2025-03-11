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

#ifndef _L_CONFERENCE_ID_H_
#define _L_CONFERENCE_ID_H_

#include "address/address.h"
#include "conference-id-params.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC ConferenceId {
public:
	static const std::string IdentifierDelimiter;
	static std::pair<std::shared_ptr<Address>, std::shared_ptr<Address>> parseIdentifier(const std::string &identifier);

	ConferenceId();
	// Caution: this optimized constructor does not care about extracting the URI part only. Use it for URI only
	// Address.
	ConferenceId(Address &&peerAddress, Address &&localAddress, const ConferenceIdParams &params);
	ConferenceId(const std::shared_ptr<const Address> &peerAddress,
	             const std::shared_ptr<const Address> &localAddress,
	             const ConferenceIdParams &params);
	ConferenceId(const ConferenceId &other);
	ConferenceId(ConferenceId &&other) = default;

	virtual ~ConferenceId() = default;

	ConferenceId *clone() const {
		return new ConferenceId(*this);
	}

	ConferenceId &operator=(const ConferenceId &other);

	bool operator==(const ConferenceId &other) const;
	bool operator!=(const ConferenceId &other) const;

	bool operator<(const ConferenceId &other) const;

	const std::shared_ptr<Address> &getPeerAddress() const;
	const std::shared_ptr<Address> &getLocalAddress() const;

	void setPeerAddress(const std::shared_ptr<const Address> &addr, bool forceUpdate = false);
	void setLocalAddress(const std::shared_ptr<const Address> &addr, bool forceUpdate = false);

	const std::string &getIdentifier() const;

	bool isValid() const;
	size_t getHash() const;
	size_t getWeakHash() const;
	bool weakEqual(const ConferenceId &other) const;
	struct WeakHash {
		size_t operator()(const ConferenceId &id) const {
			return id.getWeakHash();
		}
	};
	struct WeakEqual {
		bool operator()(const ConferenceId &id1, const ConferenceId &id2) const {
			return id1.weakEqual(id2);
		}
	};

private:
	static Address reducedAddress(const Address &addr);
	std::shared_ptr<Address> mPeerAddress;
	std::shared_ptr<Address> mLocalAddress;
	mutable size_t mHash = 0;
	mutable size_t mWeakHash = 0;
	ConferenceIdParams mParams;
	mutable std::string mIdentifier;

	bool canUpdateAddress(const std::shared_ptr<const Address> &addr, bool useLocal) const;
	std::shared_ptr<Address> processAddress(const Address &addr) const;
};

inline std::ostream &operator<<(std::ostream &os, const ConferenceId &conferenceId) {
	auto peerAddress =
	    (conferenceId.getPeerAddress()) ? conferenceId.getPeerAddress()->asStringUriOnly() : std::string("sip:");
	auto localAddress =
	    (conferenceId.getLocalAddress()) ? conferenceId.getLocalAddress()->asStringUriOnly() : std::string("sip:");
	os << "ConferenceId(peer=" << peerAddress << ", local=" << localAddress << ")";
	return os;
}

LINPHONE_END_NAMESPACE

// Add map key support.
namespace std {
template <>
struct hash<LinphonePrivate::ConferenceId> {
	std::size_t operator()(const LinphonePrivate::ConferenceId &conferenceId) const {
		return conferenceId.getHash();
	}
};
} // namespace std

#endif // ifndef _L_CONFERENCE_ID_H_
