/*
 * Copyright (c) 2010-2025 Belledonne Communications SARL.
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

#include "remote-contact-directory.h"

#include "ldap/ldap-params.h"
#include "linphone/utils/utils.h"
#include "logger/logger.h"
#include "vcard/carddav-params.h"

LINPHONE_BEGIN_NAMESPACE

RemoteContactDirectory::RemoteContactDirectory(const std::shared_ptr<CardDavParams> &cardDavParams) {
	mCardDavParams = cardDavParams;
	mType = LinphoneRemoteContactDirectoryTypeCardDav;
}

RemoteContactDirectory::RemoteContactDirectory(const std::shared_ptr<LdapParams> &ldapParams) {
	mLdapParams = ldapParams;
	mType = LinphoneRemoteContactDirectoryTypeLdap;
}

RemoteContactDirectory::~RemoteContactDirectory() {
	mCardDavParams = nullptr;
	mLdapParams = nullptr;
}

RemoteContactDirectory *RemoteContactDirectory::clone() const {
	return nullptr;
}

LinphoneRemoteContactDirectoryType RemoteContactDirectory::getType() const {
	return mType;
}

std::shared_ptr<CardDavParams> &RemoteContactDirectory::getCardDavParams() {
	return mCardDavParams;
}

std::shared_ptr<LdapParams> &RemoteContactDirectory::getLdapParams() {
	return mLdapParams;
}

const std::string &RemoteContactDirectory::getServerUrl() const {
	if (mType == LinphoneRemoteContactDirectoryTypeCardDav) {
		return mCardDavParams->getServerUrl();
	} else {
		return mLdapParams->getServer();
	}
}

void RemoteContactDirectory::setServerUrl(const std::string &serverUrl) {
	if (mType == LinphoneRemoteContactDirectoryTypeCardDav) {
		mCardDavParams->setServerUrl(serverUrl);
	} else {
		mLdapParams->setServer(serverUrl);
	}
}

unsigned int RemoteContactDirectory::getLimit() const {
	if (mType == LinphoneRemoteContactDirectoryTypeCardDav) {
		return mCardDavParams->getLimit();
	} else {
		return (unsigned int)mLdapParams->getMaxResults();
	}
}

void RemoteContactDirectory::setLimit(unsigned int limit) {
	if (mType == LinphoneRemoteContactDirectoryTypeCardDav) {
		mCardDavParams->setLimit(limit);
	} else {
		mLdapParams->setMaxResults((int)limit);
	}
}

unsigned int RemoteContactDirectory::getMinCharactersToStartQuery() const {
	if (mType == LinphoneRemoteContactDirectoryTypeCardDav) {
		return mCardDavParams->getMinCharactersToStartQuery();
	} else {
		return (unsigned int)mLdapParams->getMinChars();
	}
}

void RemoteContactDirectory::setMinCharactersToStartQuery(unsigned int min) {
	if (mType == LinphoneRemoteContactDirectoryTypeCardDav) {
		mCardDavParams->setMinCharactersToStartQuery(min);
	} else {
		mLdapParams->setMinChars((int)min);
	}
}

unsigned int RemoteContactDirectory::getTimeout() const {
	if (mType == LinphoneRemoteContactDirectoryTypeCardDav) {
		return mCardDavParams->getTimeout();
	} else {
		return (unsigned int)mLdapParams->getTimeout();
	}
}

void RemoteContactDirectory::setTimeout(unsigned int seconds) {
	if (mType == LinphoneRemoteContactDirectoryTypeCardDav) {
		mCardDavParams->setTimeout(seconds);
	} else {
		mLdapParams->setTimeout((int)seconds);
	}
}

void RemoteContactDirectory::writeToConfigFile() const {
	if (mType == LinphoneRemoteContactDirectoryTypeCardDav) {
		mCardDavParams->writeToConfigFile();
	} else {
		mLdapParams->writeToConfigFile();
	}
}

void RemoteContactDirectory::removeFromConfigFile() const {
	if (mType == LinphoneRemoteContactDirectoryTypeCardDav) {
		mCardDavParams->removeFromConfigFile();
	} else {
		mLdapParams->removeFromConfigFile();
	}
}

int RemoteContactDirectory::getConfigIndex() const {
	if (mType == LinphoneRemoteContactDirectoryTypeCardDav) {
		return mCardDavParams->getConfigIndex();
	} else {
		return mLdapParams->getConfigIndex();
	}
}

void RemoteContactDirectory::setConfigIndex(int configIndex) {
	if (mType == LinphoneRemoteContactDirectoryTypeCardDav) {
		mCardDavParams->setConfigIndex(configIndex);
	} else {
		mLdapParams->setConfigIndex(configIndex);
	}
}

LINPHONE_END_NAMESPACE
