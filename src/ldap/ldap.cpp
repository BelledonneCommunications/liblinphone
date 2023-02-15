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

#include "ldap.h"

#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "ldap-config-keys.h"
#include "linphone/api/c-ldap-params.h"
#include "linphone/api/c-ldap.h"
#include "linphone/core.h"
#include "linphone/utils/utils.h"
#include "private.h"
#include <bctoolbox/defs.h>

#include <string>

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

const std::string Ldap::gSectionRootKey = "ldap";

Ldap::Ldap(const std::shared_ptr<Core> &lc, int id) : CoreAccessor(lc) {
	setIndex(id);
	mParams = LdapParams::create();
	bctbx_message("LinphoneLdap[%p] created", toC());
}

Ldap::Ldap(const std::shared_ptr<Core> &lc, std::shared_ptr<LdapParams> params, int id) : CoreAccessor(lc) {
	setIndex(id);
	mParams = params;
	bctbx_message("LinphoneLdap[%p] created with params", toC());
}

Ldap::~Ldap() {
	bctbx_message("LinphoneLdap[%p] destroyed", toC());
}

std::shared_ptr<Ldap> Ldap::create(const std::shared_ptr<Core> &lc, const std::string &sectionKey) {
	std::shared_ptr<Ldap> ldap;
	int id = getIdFromSectionName(sectionKey);
	if (id >= 0) {
		ldap = bellesip::HybridObject<LinphoneLdap, Ldap>::create(
		    lc, LdapParams::create(lc->getCCore()->config, sectionKey), id); // From Hybrid
	}
	return ldap;
}

int Ldap::getIdFromSectionName(std::string sectionKey) {
	int id = -1;
	std::string sectionName;
	size_t sectionNameIndex = sectionKey.length() - 1;
	sectionKey = Utils::stringToLower(sectionKey);
	while (sectionNameIndex > 0 && sectionKey[sectionNameIndex] != '_') // Get the name strip number
		--sectionNameIndex;
	if (sectionNameIndex > 0) {
		sectionName = sectionKey.substr(0, sectionNameIndex);
		if (sectionName == gSectionRootKey) id = atoi(sectionKey.substr(sectionNameIndex + 1).c_str());
	} else {
		if (sectionKey == gSectionRootKey) id = 0;
	}
	return id;
}

void Ldap::setLdapParams(std::shared_ptr<LdapParams> params) {
	mParams = params;
	getCore()->addLdap(this->getSharedFromThis());
}

std::shared_ptr<const LdapParams> Ldap::getLdapParams() const {
	return mParams;
}

void Ldap::setIndex(int index) {
	mId = index;
}

int Ldap::getIndex() const {
	return mId;
}

int Ldap::check() const {
	return mParams && mParams->check();
}

int Ldap::getNewId() const {
	LpConfig *lConfig = linphone_core_get_config(getCore()->getCCore());
	// Read configuration
	bctbx_list_t *bcSections = linphone_config_get_sections_names_list(lConfig);
	std::vector<int> allIds;
	for (auto itSections = bcSections; itSections; itSections = itSections->next) {
		std::string section = static_cast<char *>(itSections->data);
		int id = getIdFromSectionName(section);
		if (id >= 0) allIds.push_back(id);
	}
	if (bcSections) bctbx_list_free(bcSections);
	int id = 0;
	while (std::find(allIds.begin(), allIds.end(), id) != allIds.end())
		++id;
	return id;
}

void Ldap::writeToConfigFile() {
	auto lConfig = linphone_core_get_config(getCore()->getCCore());
	if (!mParams) {
		lWarning() << "writeToConfigFile is called but no LdapParams is set on Ldap [" << this->toC() << "]";
		return;
	}
	if (mId < 0) { // This is a new configuration
		setIndex(getNewId());
	} // else this is an update of the configuration
	mParams->writeToConfigFile(lConfig, gSectionRootKey + "_" + Utils::toString(mId));
}

void Ldap::removeFromConfigFile() {
	auto lConfig = linphone_core_get_config(getCore()->getCCore());
	linphone_config_clean_section(lConfig, (gSectionRootKey + "_" + Utils::toString(mId)).c_str());
}

LINPHONE_END_NAMESPACE
