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

#ifndef _L_VCARD_H_
#define _L_VCARD_H_

#include <array>

#ifdef VCARD_ENABLED
#include <belcard/belcard.hpp>
#endif /* VCARD_ENABLED */

#include "c-wrapper/c-wrapper.h"
#include "linphone/api/c-vcard.h"
#include "linphone/wrapper_utils.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

static constexpr size_t VCARD_MD5_HASH_SIZE = 16;

class Friend;
class FriendPhoneNumber;

class LINPHONE_PUBLIC Vcard : public bellesip::HybridObject<LinphoneVcard, Vcard>, public UserDataAccessor {
public:
	Vcard(bool useVCard3Grammar = false);
#ifdef VCARD_ENABLED
	Vcard(const std::shared_ptr<belcard::BelCard> &belcard);
#endif /* VCARD_ENABLED */
	Vcard(const Vcard &other);
	virtual ~Vcard();

	Vcard *clone() const override;

	// Friends
	friend Friend;
	friend void * ::linphone_vcard_get_belcard(LinphoneVcard *vcard);
	friend const bctbx_list_t * ::linphone_vcard_get_sip_addresses(LinphoneVcard *vCard);

	// Setters
	void setEtag(const std::string &etag);
	void setFamilyName(const std::string &name);
	void setFullName(const std::string &name);
	void setGivenName(const std::string &name);
	void setJobTitle(const std::string &jobTitle);
	void setOrganization(const std::string &organization);
	void setPhoto(const std::string &picture);
	void setUid(const std::string &uid);
	void setUrl(const std::string &url);
	void setStarred(bool starred);

	// Getters
	const std::string &getEtag() const;
	std::list<std::string> getExtendedPropertiesValuesByName(const std::string &name) const;
	const std::string &getFamilyName() const;
	const std::string &getFullName() const;
	const std::string &getGivenName() const;
	const std::string &getJobTitle() const;
	const std::string &getOrganization() const;
	std::list<std::string> getPhoneNumbers() const;
	std::list<std::shared_ptr<FriendPhoneNumber>> getPhoneNumbersWithLabel() const;
	const std::string &getPhoto() const;
	const std::list<std::shared_ptr<Address>> &getSipAddresses() const;
	std::list<std::string> getImppAddresses() const;
	const std::string &getUid() const;
	const std::string &getUrl() const;
	bool getStarred() const;

	// Other
	void addExtendedProperty(const std::string &name, const std::string &value);
	void addPhoneNumber(const std::string &phoneNumber);
	void addPhoneNumberWithLabel(const std::shared_ptr<const FriendPhoneNumber> &phoneNumber);
	void addSipAddress(const std::string &sipAddress);
	const std::string &asVcard4String() const;
	const std::string &asVcard4StringWithBase64Picture() const;
	void editMainSipAddress(const std::string &sipAddress);
	bool generateUniqueId();
	void removeExtentedPropertiesByName(const std::string &name);
	void removeJobTitle();
	void removeOrganization();
	void removePhoneNumber(const std::string &phoneNumber);
	void removePhoneNumberWithLabel(const std::shared_ptr<const FriendPhoneNumber> &phoneNumber);
	void removePhoto();
	bool removeSipAddress(const std::string &sipAddress);

private:
	void cleanCache();
	bool compareMd5Hash();
	void computeMd5Hash();
	void *getBelcard();

#ifdef VCARD_ENABLED
	std::shared_ptr<belcard::BelCard> mBelCard;
	std::string mEtag;
	std::string mUrl;
	std::array<unsigned char, VCARD_MD5_HASH_SIZE> mMd5;
#endif /* VCARD_ENABLED */
	mutable ListHolder<Address> mSipAddresses;
	mutable std::string vcard4String;

	bool mUseVCard3Grammar = false;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_VCARD_H_
