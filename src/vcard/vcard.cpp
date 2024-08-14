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

#include <bctoolbox/defs.h>

#include "friend/friend-phone-number.h"
#include "sal/sal.h"
#include "vcard.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

static const std::string emptyString;

#ifdef VCARD_ENABLED

Vcard::Vcard(bool useVCard3Grammar) {
	mUseVCard3Grammar = useVCard3Grammar;

	if (useVCard3Grammar) {
		mBelCard = std::shared_ptr<belcard::BelCard>(belcard::BelCardGeneric::createV3<belcard::BelCard>());
	} else {
		mBelCard = std::shared_ptr<belcard::BelCard>(belcard::BelCardGeneric::create<belcard::BelCard>());
	}
}

Vcard::Vcard(const std::shared_ptr<belcard::BelCard> &belcard) {
	mBelCard = belcard;
	mUseVCard3Grammar = belcard->isUsingV3Grammar();
}

Vcard::Vcard(const Vcard &other) : HybridObject(other) {
	mUseVCard3Grammar = other.mBelCard->isUsingV3Grammar();
	mBelCard = std::shared_ptr<belcard::BelCard>(
	    belcard::BelCardParser::getInstance(mUseVCard3Grammar)->parseOne(other.mBelCard->toFoldedString()));
	mUrl = other.mUrl;
	mEtag = other.mEtag;
	mMd5 = other.mMd5;
}

Vcard::~Vcard() {
	cleanCache();
}

Vcard *Vcard::clone() const {
	return new Vcard(*this);
}

// -----------------------------------------------------------------------------

void Vcard::setEtag(const std::string &etag) {
	mEtag = etag;
}

void Vcard::setFamilyName(const std::string &name) {
	if (mBelCard->getName()) {
		mBelCard->getName()->setFamilyName(name);
	} else {
		std::shared_ptr<belcard::BelCardName> n;
		if (mUseVCard3Grammar) {
			n = belcard::BelCardGeneric::createV3<belcard::BelCardName>();
		} else {
			n = belcard::BelCardGeneric::create<belcard::BelCardName>();
		}
		n->setFamilyName(name);
		mBelCard->setName(n);
	}
}

void Vcard::setFullName(const std::string &name) {
	if (mBelCard->getFullName()) {
		mBelCard->getFullName()->setValue(name);
	} else {
		std::shared_ptr<belcard::BelCardFullName> fn;
		if (mUseVCard3Grammar) {
			fn = belcard::BelCardGeneric::createV3<belcard::BelCardFullName>();
		} else {
			fn = belcard::BelCardGeneric::create<belcard::BelCardFullName>();
		}
		fn->setValue(name);
		mBelCard->setFullName(fn);
	}
}

void Vcard::setJobTitle(const std::string &jobTitle) {
	if (jobTitle.empty()) {
		removeJobTitle();
	} else if (mBelCard->getTitles().size() > 0) {
		const std::shared_ptr<belcard::BelCardTitle> title = mBelCard->getTitles().front();
		title->setValue(jobTitle);
	} else {
		std::shared_ptr<belcard::BelCardTitle> title;
		if (mUseVCard3Grammar) {
			title = belcard::BelCardGeneric::createV3<belcard::BelCardTitle>();
		} else {
			title = belcard::BelCardGeneric::create<belcard::BelCardTitle>();
		}
		title->setValue(jobTitle);
		if (!mBelCard->addTitle(title)) {
			lError() << "[vCard] Couldn't add TITLE value [" << jobTitle << "] to vCard [" << toC() << "]";
		}
	}
}

void Vcard::setOrganization(const std::string &organization) {
	if (organization.empty()) {
		removeOrganization();
	} else if (mBelCard->getOrganizations().size() > 0) {
		const std::shared_ptr<belcard::BelCardOrganization> org = mBelCard->getOrganizations().front();
		org->setValue(organization);
	} else {
		std::shared_ptr<belcard::BelCardOrganization> org;
		if (mUseVCard3Grammar) {
			org = belcard::BelCardGeneric::createV3<belcard::BelCardOrganization>();
		} else {
			org = belcard::BelCardGeneric::create<belcard::BelCardOrganization>();
		}
		org->setValue(organization);
		if (!mBelCard->addOrganization(org)) {
			lError() << "[vCard] Couldn't add ORG value [" << organization << "] to vCard [" << toC() << "]";
		}
	}
}

void Vcard::setGivenName(const std::string &name) {
	if (mBelCard->getName()) {
		mBelCard->getName()->setGivenName(name);
	} else {
		std::shared_ptr<belcard::BelCardName> n;
		if (mUseVCard3Grammar) {
			n = belcard::BelCardGeneric::createV3<belcard::BelCardName>();
		} else {
			n = belcard::BelCardGeneric::create<belcard::BelCardName>();
		}
		n->setGivenName(name);
		mBelCard->setName(n);
	}
}

void Vcard::setPhoto(const std::string &picture) {
	if (picture.empty()) {
		removePhoto();
	} else if (mBelCard->getPhotos().size() > 0) {
		const std::shared_ptr<belcard::BelCardPhoto> photo = mBelCard->getPhotos().front();
		photo->setValue(picture);
	} else {
		std::shared_ptr<belcard::BelCardPhoto> photo;
		if (mUseVCard3Grammar) {
			photo = belcard::BelCardGeneric::createV3<belcard::BelCardPhoto>();
		} else {
			photo = belcard::BelCardGeneric::create<belcard::BelCardPhoto>();
		}
		photo->setValue(picture);
		if (!mBelCard->addPhoto(photo)) {
			lError() << "[vCard] Couldn't add PHOTO value [" << picture << "] to vCard [" << toC() << "]";
		}
	}
}

void Vcard::setUid(const std::string &uid) {
	if (uid.empty()) return;
	std::shared_ptr<belcard::BelCardUniqueId> uniqueId;
	if (mUseVCard3Grammar) {
		uniqueId = belcard::BelCardGeneric::createV3<belcard::BelCardUniqueId>();
	} else {
		uniqueId = belcard::BelCardGeneric::create<belcard::BelCardUniqueId>();
	}
	uniqueId->setValue(uid);
	mBelCard->setUniqueId(uniqueId);
}

void Vcard::setUrl(const std::string &url) {
	mUrl = url;
}

// -----------------------------------------------------------------------------

const std::string &Vcard::getEtag() const {
	return mEtag;
}

std::list<std::string> Vcard::getExtendedPropertiesValuesByName(const std::string &name) const {
	std::list<std::string> result;
	for (const auto &property : mBelCard->getExtendedProperties()) {
		if (property->getName() == name) result.push_back(property->getValue());
	}
	return result;
}

const std::string &Vcard::getFamilyName() const {
	return mBelCard->getName() ? mBelCard->getName()->getFamilyName() : emptyString;
}

const std::string &Vcard::getFullName() const {
	return mBelCard->getFullName() ? mBelCard->getFullName()->getValue() : emptyString;
}

const std::string &Vcard::getGivenName() const {
	return mBelCard->getName() ? mBelCard->getName()->getGivenName() : emptyString;
}

const std::string &Vcard::getJobTitle() const {
	return (mBelCard->getTitles().size() > 0) ? mBelCard->getTitles().front()->getValue() : emptyString;
}

const std::string &Vcard::getOrganization() const {
	if (mBelCard->getOrganizations().size() > 0) return mBelCard->getOrganizations().front()->getValue();
	return emptyString;
}

std::list<std::string> Vcard::getPhoneNumbers() const {
	std::list<std::string> result;
	for (const auto &phoneNumber : mBelCard->getPhoneNumbers())
		result.push_back(phoneNumber->getValue());
	return result;
}

std::list<std::shared_ptr<FriendPhoneNumber>> Vcard::getPhoneNumbersWithLabel() const {
	std::list<std::shared_ptr<FriendPhoneNumber>> result;
	for (const auto &number : mBelCard->getPhoneNumbers()) {
		result.push_back(FriendPhoneNumber::create(number));
	}
	return result;
}

const std::string &Vcard::getPhoto() const {
	return (mBelCard->getPhotos().size() > 0) ? mBelCard->getPhotos().front()->getValue() : emptyString;
}

const std::list<std::shared_ptr<Address>> &Vcard::getSipAddresses() const {
	if (mSipAddressesCache.empty()) {
		for (auto &impp : mBelCard->getImpp()) {
			std::string value = impp->getValue();
			// Only parse SIP URIs or URIs without a scheme
			if (value.rfind("sip:", 0) == 0 || value.rfind("sips:", 0) == 0 || value.rfind(":") == std::string::npos) {
				std::shared_ptr<Address> addr = Address::create(value);
				if (addr) {
					auto displayName = mBelCard->getFullName();
					if (addr->getDisplayName().empty() && displayName) addr->setDisplayName(displayName->getValue());
					mSipAddressesCache.push_back(addr);
					mBctbxSipAddressesCache = bctbx_list_append(mBctbxSipAddressesCache, addr->toC());
				}
			}
		}
	}
	return mSipAddressesCache;
}

std::list<std::string> Vcard::getImppAddresses() const {
	std::list<std::string> result;
	for (const auto &impp : mBelCard->getImpp())
		result.push_back(impp->getValue());
	return result;
}

const std::string &Vcard::getUid() const {
	return mBelCard->getUniqueId() ? mBelCard->getUniqueId()->getValue() : emptyString;
}

const std::string &Vcard::getUrl() const {
	return mUrl;
}

// -----------------------------------------------------------------------------

void Vcard::addExtendedProperty(const std::string &name, const std::string &value) {
	if (name.empty()) return;
	std::shared_ptr<belcard::BelCardProperty> property;
	if (mUseVCard3Grammar) {
		property = belcard::BelCardGeneric::createV3<belcard::BelCardProperty>();
	} else {
		property = belcard::BelCardGeneric::create<belcard::BelCardProperty>();
	}
	property->setName(name);
	property->setValue(value);
	if (!mBelCard->addExtendedProperty(property)) {
		lError() << "[vCard] Couldn't add extended property name [" << name << "] value [" << value << "] to vCard ["
		         << toC() << "]";
	}
}

void Vcard::addPhoneNumber(const std::string &phoneNumber) {
	std::shared_ptr<belcard::BelCardPhoneNumber> belcardPhoneNumber;
	if (mUseVCard3Grammar) {
		belcardPhoneNumber = belcard::BelCardGeneric::createV3<belcard::BelCardPhoneNumber>();
	} else {
		belcardPhoneNumber = belcard::BelCardGeneric::create<belcard::BelCardPhoneNumber>();
	}
	belcardPhoneNumber->setValue(phoneNumber);
	if (!mBelCard->addPhoneNumber(belcardPhoneNumber)) {
		lError() << "[vCard] Couldn't add TEL value [" << phoneNumber << "] to vCard [" << toC() << "]";
	}
}

void Vcard::addPhoneNumberWithLabel(const std::shared_ptr<const FriendPhoneNumber> &phoneNumber) {
	if (!phoneNumber) return;
	std::shared_ptr<belcard::BelCardPhoneNumber> belcardPhoneNumber =
	    phoneNumber->toBelcardPhoneNumber(mUseVCard3Grammar);
	if (!mBelCard->addPhoneNumber(belcardPhoneNumber)) {
		const std::string &phone = phoneNumber->getPhoneNumber();
		const std::string &label = phoneNumber->getLabel();
		lError() << "[vCard] Couldn't add TEL value [" << phone << "] with label [" << label << "] to vCard [" << toC()
		         << "]";
		std::shared_ptr<belcard::BelCardPhoneNumber> belcardPhoneNumberWithoutLabel;
		if (mUseVCard3Grammar) {
			belcardPhoneNumberWithoutLabel = belcard::BelCardGeneric::createV3<belcard::BelCardPhoneNumber>();
		} else {
			belcardPhoneNumberWithoutLabel = belcard::BelCardGeneric::create<belcard::BelCardPhoneNumber>();
		}
		belcardPhoneNumberWithoutLabel->setValue(phone);
		if (!mBelCard->addPhoneNumber(belcardPhoneNumberWithoutLabel)) {
			lError() << "[vCard] Couldn't add TEL value [" << phone << "] without label to vCard [" << toC()
			         << "] either!";
		} else {
			lWarning() << "[vCard] TEL value [" << phone << "] added to vCard [" << toC() << "] without label";
		}
	}
}

void Vcard::addSipAddress(const std::string &sipAddress) {
	std::shared_ptr<belcard::BelCardImpp> impp;
	if (mUseVCard3Grammar) {
		impp = belcard::BelCardGeneric::createV3<belcard::BelCardImpp>();
	} else {
		impp = belcard::BelCardGeneric::create<belcard::BelCardImpp>();
	}
	impp->setValue(sipAddress);
	if (!mBelCard->addImpp(impp)) {
		lError() << "[vCard] Couldn't add IMPP value [" << sipAddress << "] to vCard [" << toC() << "]";
	} else {
		// Clean cache so next call to getSipAddresses() will return the newly added one if it wasn't the first one
		cleanCache();
	}
}

const std::string &Vcard::asVcard4String() const {
	return mBelCard->toFoldedString();
}

void Vcard::editMainSipAddress(const std::string &sipAddress) {
	if (mBelCard->getImpp().size() > 0) {
		const std::shared_ptr<belcard::BelCardImpp> impp = mBelCard->getImpp().front();
		impp->setValue(sipAddress);
	} else {
		std::shared_ptr<belcard::BelCardImpp> impp;
		if (mUseVCard3Grammar) {
			impp = belcard::BelCardGeneric::createV3<belcard::BelCardImpp>();
		} else {
			impp = belcard::BelCardGeneric::create<belcard::BelCardImpp>();
		}
		impp->setValue(sipAddress);
		if (!mBelCard->addImpp(impp)) {
			lError() << "[vCard] Couldn't add IMPP value [" << sipAddress << "] to vCard [" << toC() << "]";
		} else {
			// Clean cache so next call to getSipAddresses() will return the newly added one if it wasn't the first one
			cleanCache();
		}
	}
}

bool Vcard::generateUniqueId() {
	if (!getUid().empty()) return false;
	stringstream ss;
	ss << "urn:" << LinphonePrivate::Sal::generateUuid();
	setUid(ss.str());
	return true;
}

void Vcard::removeExtentedPropertiesByName(const std::string &name) {
	for (const auto &property : mBelCard->getExtendedProperties()) {
		if (property->getName() == name) {
			mBelCard->removeExtendedProperty(property);
			break;
		}
	}
}

void Vcard::removeJobTitle() {
	if (mBelCard->getTitles().size() > 0) {
		const std::shared_ptr<belcard::BelCardTitle> title = mBelCard->getTitles().front();
		mBelCard->removeTitle(title);
	}
}

void Vcard::removeOrganization() {
	if (mBelCard->getOrganizations().size() > 0) {
		const std::shared_ptr<belcard::BelCardOrganization> org = mBelCard->getOrganizations().front();
		mBelCard->removeOrganization(org);
	}
}

void Vcard::removePhoneNumber(const std::string &phoneNumber) {
	for (auto &belcardPhoneNumber : mBelCard->getPhoneNumbers()) {
		if (belcardPhoneNumber->getValue() == phoneNumber) {
			mBelCard->removePhoneNumber(belcardPhoneNumber);
			break;
		}
	}
}

void Vcard::removePhoneNumberWithLabel(const std::shared_ptr<const FriendPhoneNumber> &phoneNumber) {
	const std::string &phone = phoneNumber->getPhoneNumber();
	for (auto &number : mBelCard->getPhoneNumbers()) {
		if (number->getValue() == phone) {
			mBelCard->removePhoneNumber(number);
			break;
		}
	}
}

void Vcard::removePhoto() {
	if (mBelCard->getPhotos().size() > 0) {
		const std::shared_ptr<belcard::BelCardPhoto> photo = mBelCard->getPhotos().front();
		mBelCard->removePhoto(photo);
	}
}

bool Vcard::removeSipAddress(const std::string &sipAddress) {
	bool found = false;
	for (auto &impp : mBelCard->getImpp()) {
		if (strcasecmp(impp->getValue().c_str(), sipAddress.c_str()) == 0) {
			mBelCard->removeImpp(impp);
			found = true;
			break;
		}
	}

	if (!found) {
		lWarning() << "[vCard] SIP URI [" << sipAddress << "] to remove wasn't found in vCard's IMPP";
	} else {
		cleanCache();
	}
	return found;
}

// -----------------------------------------------------------------------------

bool Vcard::compareMd5Hash() {
	std::array<unsigned char, VCARD_MD5_HASH_SIZE> previousMd5 = mMd5;
	computeMd5Hash();
	return !!memcmp(mMd5.data(), previousMd5.data(), VCARD_MD5_HASH_SIZE);
}

void Vcard::computeMd5Hash() {
	std::string text = asVcard4String();
	bctbx_md5(reinterpret_cast<const unsigned char *>(text.c_str()), text.length(), mMd5.data());
}

void Vcard::cleanCache() {
	mSipAddressesCache.clear();
	bctbx_list_free(mBctbxSipAddressesCache), mBctbxSipAddressesCache = nullptr;
}

void *Vcard::getBelcard() {
	return &mBelCard;
}

#else

Vcard::Vcard(bool) {
	bctbx_message("LinphoneVcard[%p] created", toC());
}

Vcard::Vcard(const Vcard &other) : HybridObject(other) {
}

Vcard::~Vcard() {
	bctbx_message("LinphoneVcard[%p] destroyed", toC());
}

Vcard *Vcard::clone() const {
	return new Vcard(*this);
}

// -----------------------------------------------------------------------------

void Vcard::setEtag(BCTBX_UNUSED(const std::string &etag)) {
}

void Vcard::setFamilyName(BCTBX_UNUSED(const std::string &name)) {
}

void Vcard::setFullName(BCTBX_UNUSED(const std::string &name)) {
}

void Vcard::setJobTitle(BCTBX_UNUSED(const std::string &jobTitle)) {
}

void Vcard::setOrganization(BCTBX_UNUSED(const std::string &organization)) {
}

void Vcard::setGivenName(BCTBX_UNUSED(const std::string &name)) {
}

void Vcard::setPhoto(BCTBX_UNUSED(const std::string &picture)) {
}

void Vcard::setUid(BCTBX_UNUSED(const std::string &uid)) {
}

void Vcard::setUrl(BCTBX_UNUSED(const std::string &url)) {
}

// -----------------------------------------------------------------------------

const std::string &Vcard::getEtag() const {
	return emptyString;
}

std::list<std::string> Vcard::getExtendedPropertiesValuesByName(BCTBX_UNUSED(const std::string &name)) const {
	return std::list<std::string>();
}

const std::string &Vcard::getFamilyName() const {
	return emptyString;
}

const std::string &Vcard::getFullName() const {
	return emptyString;
}

const std::string &Vcard::getGivenName() const {
	return emptyString;
}

const std::string &Vcard::getJobTitle() const {
	return emptyString;
}

const std::string &Vcard::getOrganization() const {
	return emptyString;
}

std::list<std::string> Vcard::getPhoneNumbers() const {
	return std::list<std::string>();
}

std::list<std::shared_ptr<FriendPhoneNumber>> Vcard::getPhoneNumbersWithLabel() const {
	return std::list<std::shared_ptr<FriendPhoneNumber>>();
}

const std::string &Vcard::getPhoto() const {
	return emptyString;
}

const std::list<std::shared_ptr<Address>> &Vcard::getSipAddresses() const {
	return mSipAddressesCache;
}

std::list<std::string> Vcard::getImppAddresses() const {
	return std::list<std::string>();
}

const std::string &Vcard::getUid() const {
	return emptyString;
}

const std::string &Vcard::getUrl() const {
	return emptyString;
}

// -----------------------------------------------------------------------------

void Vcard::addExtendedProperty(BCTBX_UNUSED(const std::string &name), BCTBX_UNUSED(const std::string &value)) {
}

void Vcard::addPhoneNumber(BCTBX_UNUSED(const std::string &phoneNumber)) {
}

void Vcard::addPhoneNumberWithLabel(BCTBX_UNUSED(const std::shared_ptr<const FriendPhoneNumber> &phoneNumber)) {
}

void Vcard::addSipAddress(BCTBX_UNUSED(const std::string &sipAddress)) {
}

const std::string &Vcard::asVcard4String() const {
	return emptyString;
}

void Vcard::editMainSipAddress(BCTBX_UNUSED(const std::string &sipAddress)) {
}

bool Vcard::generateUniqueId() {
	return false;
}

void Vcard::removeExtentedPropertiesByName(BCTBX_UNUSED(const std::string &name)) {
}

void Vcard::removeJobTitle() {
}

void Vcard::removeOrganization() {
}

void Vcard::removePhoneNumber(BCTBX_UNUSED(const std::string &phoneNumber)) {
}

void Vcard::removePhoneNumberWithLabel(BCTBX_UNUSED(const std::shared_ptr<const FriendPhoneNumber> &phoneNumber)) {
}

void Vcard::removePhoto() {
}

bool Vcard::removeSipAddress(BCTBX_UNUSED(const std::string &sipAddress)) {
	return false;
}

// -----------------------------------------------------------------------------

bool Vcard::compareMd5Hash() {
	return false;
}

void Vcard::computeMd5Hash() {
}

void Vcard::cleanCache() {
}

void *Vcard::getBelcard() {
	return nullptr;
}

#endif /* VCARD_ENABLED */

LINPHONE_END_NAMESPACE
