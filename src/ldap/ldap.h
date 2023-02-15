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

#ifndef _L_LDAP_H_
#define _L_LDAP_H_

#include <memory>

#include "c-wrapper/c-wrapper.h"

#include "core/core.h"
#include "ldap-params.h"
#include "linphone/api/c-types.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Ldap : public bellesip::HybridObject<LinphoneLdap, Ldap>, public CoreAccessor {
public:
	Ldap(const std::shared_ptr<Core> &lc, int id = -1);
	Ldap(const std::shared_ptr<Core> &lc, std::shared_ptr<LdapParams> params, int id = -1);
	~Ldap();

	// Create a Ldap instance from configuration at section name. Return nullptr if this is not an ldap configuration
	// (section name different from "ldap_n" or "ldap")
	static std::shared_ptr<Ldap> create(const std::shared_ptr<Core> &lc, const std::string &sectionKey);
	static bool isLdapConfigSection(const std::string &sectionKey);
	static int getIdFromSectionName(std::string sectionKey);

	// Ldap* clone () const override;

	// Account params configuration
	void setLdapParams(std::shared_ptr<LdapParams> params);
	std::shared_ptr<const LdapParams> getLdapParams() const;

	void setIndex(int index);
	int getIndex() const;

	// Other
	int check() const;
	void writeToConfigFile();
	void removeFromConfigFile();
	int getNewId() const;

	static const std::string gSectionRootKey;

private:
	std::shared_ptr<LdapParams> mParams;
	int mId = -1; // -1: get an unique identifier on saving.
	std::string mSectionKey;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_LDAP_H_
