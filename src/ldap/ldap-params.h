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

#ifndef _L_LDAP_PARAMS_H_
#define _L_LDAP_PARAMS_H_

#include "belle-sip/object++.hh"
#include "core/core-accessor.h"
#include "core/core.h"
#include "linphone/api/c-types.h"
#include "linphone/types.h"

#include <map>
#include <string>
#include <vector>

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LdapParams : public bellesip::HybridObject<LinphoneLdapParams, LdapParams>, public CoreAccessor {
	friend class Ldap;

public:
	LdapParams(const std::shared_ptr<Core> &core);
	LdapParams(const std::shared_ptr<Core> &core, int index);
	LdapParams(const LdapParams &other);
	~LdapParams();

	LdapParams *clone() const override;

	// Setters
	void setCustomValue(const std::string &key, const std::string &value);
	void setServer(const std::string &server);
	void setBindDn(const std::string &bindDn);
	void setBaseObject(const std::string &baseObject);
	void setTimeout(const int timeout);
	void setTimeoutTlsMs(const int timeout);
	void setMaxResults(const int maxResults);
	void setMinChars(const int minChars);
	void setDelay(const int ms);
	void setPassword(const std::string &password);
	void setFilter(const std::string &filter);
	void setNameAttribute(const std::string &nameAttribute);
	void setSipAttribute(const std::string &sipAttribute);
	void setSipDomain(const std::string &sipDomain);
	void setEnabled(const bool enable);
	void enableSal(const bool enable);
	void enableTls(const bool enable);
	void setDebugLevel(LinphoneLdapDebugLevel level);
	void setAuthMethod(LinphoneLdapAuthMethod authMethod);
	void setServerCertificatesVerificationMode(LinphoneLdapCertVerificationMode mode);

	// Getters
	std::string &getCustomValue(const std::string &key);
	const std::string &getCustomValue(const std::string &key) const;
	const std::string &getServer() const;
	const std::string &getBindDn() const;
	const std::string &getBaseObject() const;
	int getTimeout() const;
	int getTimeoutTlsMs() const;
	int getMaxResults() const;
	int getMinChars() const;
	int getDelay() const;
	const std::string &getPassword() const;
	const std::string &getFilter() const;
	const std::string &getNameAttribute() const;
	const std::string &getSipAttribute() const;
	const std::string &getSipDomain() const;
	bool getEnabled() const;
	bool salEnabled() const;
	bool tlsEnabled() const;
	LinphoneLdapDebugLevel getDebugLevel() const;
	LinphoneLdapAuthMethod getAuthMethod() const;
	LinphoneLdapCertVerificationMode getServerCertificatesVerificationMode() const;

	std::map<std::string, std::vector<std::string>> getConfig() const;

	// Others
	int check() const;
	int checkServer() const;
	int checkBaseObject() const;

	void writeToConfigFile() const;
	void removeFromConfigFile() const;

private:
	void lookupConfigEntryIndex();
	void readFromConfigFile();

	int mConfigIndex = -1;
	std::map<std::string, std::string> mConfig;
	std::string
	    mDummyTxt; // For passing persistent data to wrapper (getCustomValue() can return "" without being in config)
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_ACCOUNT_PARAMS_H_
