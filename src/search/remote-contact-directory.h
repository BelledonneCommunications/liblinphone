/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#ifndef _L_REMOTE_CONTACT_DIRECTORY_H_
#define _L_REMOTE_CONTACT_DIRECTORY_H_

#include "core/core.h"
#include "ldap/ldap-params.h"
#include "vcard/carddav-params.h"

LINPHONE_BEGIN_NAMESPACE

class RemoteContactDirectory : public bellesip::HybridObject<LinphoneRemoteContactDirectory, RemoteContactDirectory> {
public:
	RemoteContactDirectory(const std::shared_ptr<CardDavParams> &cardDavParams) {
		mCardDavParams = cardDavParams;
		mType = LinphoneRemoteContactDirectoryTypeCardDav;
	}

	RemoteContactDirectory(const std::shared_ptr<LdapParams> &ldapParams) {
		mLdapParams = ldapParams;
		mType = LinphoneRemoteContactDirectoryTypeLdap;
	}

	RemoteContactDirectory(const RemoteContactDirectory &ms) = delete;
	virtual ~RemoteContactDirectory() {
		mCardDavParams = nullptr;
		mLdapParams = nullptr;
	}

	RemoteContactDirectory *clone() const override {
		return nullptr;
	}

	LinphoneRemoteContactDirectoryType getType() const {
		return mType;
	}

	std::shared_ptr<CardDavParams> getCardDavParams() const {
		return mCardDavParams;
	}

	std::shared_ptr<LdapParams> getLdapParams() const {
		return mLdapParams;
	}

	const std::string &getServerUrl() const {
		if (mType == LinphoneRemoteContactDirectoryTypeCardDav) {
			return mCardDavParams->getServerUrl();
		} else {
			return mLdapParams->getServer();
		}
	}

	void setServerUrl(const std::string &serverUrl) {
		if (mType == LinphoneRemoteContactDirectoryTypeCardDav) {
			mCardDavParams->setServerUrl(serverUrl);
		} else {
			mLdapParams->setServer(serverUrl);
		}
	}

	unsigned int getLimit() const {
		if (mType == LinphoneRemoteContactDirectoryTypeCardDav) {
			return mCardDavParams->getLimit();
		} else {
			return (unsigned int)mLdapParams->getMaxResults();
		}
	}

	void setLimit(unsigned int limit) {
		if (mType == LinphoneRemoteContactDirectoryTypeCardDav) {
			mCardDavParams->setLimit(limit);
		} else {
			mLdapParams->setMaxResults((int)limit);
		}
	}

	unsigned int getMinCharactersToStartQuery() const {
		if (mType == LinphoneRemoteContactDirectoryTypeCardDav) {
			return mCardDavParams->getMinCharactersToStartQuery();
		} else {
			return (unsigned int)mLdapParams->getMinChars();
		}
	}

	void setMinCharactersToStartQuery(unsigned int min) {
		if (mType == LinphoneRemoteContactDirectoryTypeCardDav) {
			mCardDavParams->setMinCharactersToStartQuery(min);
		} else {
			mLdapParams->setMinChars((int)min);
		}
	}

	unsigned int getTimeout() const {
		if (mType == LinphoneRemoteContactDirectoryTypeCardDav) {
			return mCardDavParams->getTimeout();
		} else {
			return (unsigned int)mLdapParams->getTimeout();
		}
	}

	void setTimeout(unsigned int seconds) {
		if (mType == LinphoneRemoteContactDirectoryTypeCardDav) {
			mCardDavParams->setTimeout(seconds);
		} else {
			mLdapParams->setTimeout((int)seconds);
		}
	}

	void writeToConfigFile() const {
		if (mType == LinphoneRemoteContactDirectoryTypeCardDav) {
			mCardDavParams->writeToConfigFile();
		} else {
			mLdapParams->writeToConfigFile();
		}
	}

	void removeFromConfigFile() const {
		if (mType == LinphoneRemoteContactDirectoryTypeCardDav) {
			mCardDavParams->removeFromConfigFile();
		} else {
			mLdapParams->removeFromConfigFile();
		}
	}

private:
	std::shared_ptr<CardDavParams> mCardDavParams;
	std::shared_ptr<LdapParams> mLdapParams;
	LinphoneRemoteContactDirectoryType mType;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_REMOTE_CONTACT_DIRECTORY_H_
