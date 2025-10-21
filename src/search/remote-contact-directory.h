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

#include "linphone/utils/utils.h"

LINPHONE_BEGIN_NAMESPACE

class LdapParams;
class CardDavParams;

class RemoteContactDirectory : public bellesip::HybridObject<LinphoneRemoteContactDirectory, RemoteContactDirectory> {
public:
	RemoteContactDirectory(const std::shared_ptr<CardDavParams> &cardDavParams);
	RemoteContactDirectory(const std::shared_ptr<LdapParams> &ldapParams);

	RemoteContactDirectory(const RemoteContactDirectory &ms) = delete;
	virtual ~RemoteContactDirectory();

	RemoteContactDirectory *clone() const override;

	LinphoneRemoteContactDirectoryType getType() const;

	std::shared_ptr<CardDavParams> &getCardDavParams();
	std::shared_ptr<LdapParams> &getLdapParams();

	const std::string &getServerUrl() const;
	void setServerUrl(const std::string &serverUrl);

	unsigned int getLimit() const;
	void setLimit(unsigned int limit);

	unsigned int getMinCharactersToStartQuery() const;
	void setMinCharactersToStartQuery(unsigned int min);

	unsigned int getTimeout() const;
	void setTimeout(unsigned int seconds);

	void writeToConfigFile() const;
	void removeFromConfigFile() const;

	int getConfigIndex() const;
	void setConfigIndex(int configIndex);

	struct RemoteContactDirectorySharedPtrLess {
		bool operator()(const std::shared_ptr<RemoteContactDirectory> &lhs,
		                const std::shared_ptr<RemoteContactDirectory> &rhs) const {
			bool ret = false;
			auto lhsType = lhs->getType();
			auto rhsType = rhs->getType();
			if (lhsType == rhsType) {
				if (lhsType == LinphoneRemoteContactDirectoryTypeCardDav) {
					ret = (lhs->getCardDavParams() < rhs->getCardDavParams());
				} else {
					ret = (lhs->getLdapParams() < rhs->getLdapParams());
				}
			} else {
				ret = (lhsType < rhsType);
			}
			return ret;
		}
	};

private:
	std::shared_ptr<CardDavParams> mCardDavParams;
	std::shared_ptr<LdapParams> mLdapParams;
	LinphoneRemoteContactDirectoryType mType;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_REMOTE_CONTACT_DIRECTORY_H_
