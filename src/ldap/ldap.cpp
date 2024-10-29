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

#include "bctoolbox/defs.h"

#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "ldap-config-keys.h"
#include "ldap.h"
#include "linphone/api/c-ldap-params.h"
#include "linphone/core.h"
#include "linphone/utils/utils.h"
#include "private.h"
#include "search/remote-contact-directory.h"

#include <string>

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

Ldap::Ldap(const std::shared_ptr<Core> &lc) : CoreAccessor(lc) {
	mParams = LdapParams::create(lc)->toSharedPtr();
	bctbx_message("LinphoneLdap[%p] created", toC());
}

Ldap::Ldap(const std::shared_ptr<Core> &lc, const std::shared_ptr<LdapParams> &params) : CoreAccessor(lc) {
	mParams = params;
	bctbx_message("LinphoneLdap[%p] created with params", toC());
}

Ldap::~Ldap() {
	bctbx_message("LinphoneLdap[%p] destroyed", toC());
}

void Ldap::setLdapParams(std::shared_ptr<LdapParams> params) {
	shared_ptr<RemoteContactDirectory> found = nullptr;
	if (mParams) {
		for (auto rdc : getCore()->getRemoteContactDirectories()) {
			if (rdc->getType() == LinphoneRemoteContactDirectoryTypeLdap && rdc->getLdapParams() == mParams) {
				found = rdc;
				break;
			}
		}
	}
	mParams = params;

	if (found) {
		getCore()->removeRemoteContactDirectory(found);
	}
	auto remoteContactDirectory = RemoteContactDirectory::create(mParams);
	getCore()->addRemoteContactDirectory(remoteContactDirectory);
}

std::shared_ptr<LdapParams> Ldap::getLdapParams() const {
	return mParams;
}

void Ldap::setIndex(int index) {
	if (mParams->mConfigIndex != index) {
		bctbx_warning("Trying to change LDAPParams config index, don't do it");
	}
}

int Ldap::getIndex() const {
	return mParams->mConfigIndex;
}

int Ldap::check() const {
	return mParams && mParams->check();
}

int Ldap::getNewId() const {
	return getIndex();
}

void Ldap::writeToConfigFile() {
	mParams->writeToConfigFile();
}

void Ldap::removeFromConfigFile() {
	mParams->removeFromConfigFile();
}

LINPHONE_END_NAMESPACE
