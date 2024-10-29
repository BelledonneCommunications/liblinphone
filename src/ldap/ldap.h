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
	Ldap(const std::shared_ptr<Core> &lc);
	Ldap(const std::shared_ptr<Core> &lc, const std::shared_ptr<LdapParams> &params);
	~Ldap();

	// Ldap* clone () const override;

	// Account params configuration
	void setLdapParams(std::shared_ptr<LdapParams> params);
	std::shared_ptr<LdapParams> getLdapParams() const;

	void setIndex(int index);
	int getIndex() const;

	// Other
	int check() const;
	void writeToConfigFile();
	void removeFromConfigFile();
	int getNewId() const;

private:
	std::shared_ptr<LdapParams> mParams;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_LDAP_H_
