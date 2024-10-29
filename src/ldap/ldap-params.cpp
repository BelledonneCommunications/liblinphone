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

#include "ldap-params.h"
#include "c-wrapper/internal/c-sal.h"
#include "c-wrapper/internal/c-tools.h"
#include "ldap-config-keys.h"
#include "linphone/api/c-address.h"
#include "linphone/types.h"
#include "linphone/utils/utils.h"
#include "private.h"

#include <string>

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

LdapParams::LdapParams(const shared_ptr<Core> &core) : CoreAccessor(core) {
	lookupConfigEntryIndex();
}

static string getSection(int index) {
	ostringstream ss;
	ss << "ldap_" << index;
	return ss.str();
}

LdapParams::LdapParams(const shared_ptr<Core> &core, int index) : CoreAccessor(core) {
	string section = getSection(index);
	LpConfig *config = linphone_core_get_config(core->getCCore());
	if (linphone_config_has_section(config, section.c_str())) {
		mConfigIndex = index;
		readFromConfigFile();
	} else {
		lWarning() << "[LDAP] Config section [" << section << "] doesn't exists, looking for first available index";
		lookupConfigEntryIndex();
	}
}

LdapParams::LdapParams(const LdapParams &other) : HybridObject(other), CoreAccessor(other.getCore()) {
	mConfigIndex = other.mConfigIndex;
	mConfig = other.mConfig;
}

LdapParams::~LdapParams() {
}

LdapParams *LdapParams::clone() const {
	return new LdapParams(*this);
}

// Setters
void LdapParams::setCustomValue(const std::string &key, const std::string &value) {
	mConfig[key] = value;
}

void LdapParams::setServer(const std::string &server) {
	setCustomValue("server", server);
}

void LdapParams::setBindDn(const std::string &bindDn) {
	setCustomValue("bind_dn", bindDn);
}
void LdapParams::setBaseObject(const std::string &baseObject) {
	setCustomValue("base_object", baseObject);
}

void LdapParams::setTimeout(const int timeout) {
	setCustomValue("timeout", Utils::toString(timeout >= 0 ? timeout : 0));
}

void LdapParams::setTimeoutTlsMs(const int timeout) {
	setCustomValue("timeout_tls_ms", Utils::toString(timeout >= 0 ? timeout : 0));
}

void LdapParams::setMaxResults(const int maxResults) {
	setCustomValue("max_results", Utils::toString(maxResults >= 1 ? maxResults : 1));
}

void LdapParams::setMinChars(const int minChars) {
	setCustomValue("min_chars", Utils::toString(minChars >= 0 ? minChars : 0));
}

void LdapParams::setDelay(const int ms) {
	setCustomValue("delay", Utils::toString(ms >= 0 ? ms : 0));
}

void LdapParams::setPassword(const std::string &password) {
	setCustomValue("password", password);
}

void LdapParams::setFilter(const std::string &filter) {
	setCustomValue("filter", filter);
}

void LdapParams::setNameAttribute(const std::string &nameAttribute) {
	setCustomValue("name_attribute", nameAttribute);
}

void LdapParams::setSipAttribute(const std::string &sipAttribute) {
	setCustomValue("sip_attribute", sipAttribute);
}

void LdapParams::setSipDomain(const std::string &sipDomain) {
	setCustomValue("sip_domain", sipDomain);
}

void LdapParams::setEnabled(bool enable) {
	setCustomValue("enable", (enable ? "1" : "0"));
}

void LdapParams::enableSal(bool enable) {
	setCustomValue("use_sal", (enable ? "1" : "0"));
}

void LdapParams::enableTls(bool enable) {
	setCustomValue("use_tls", (enable ? "1" : "0"));
}

void LdapParams::setDebugLevel(LinphoneLdapDebugLevel level) {
	setCustomValue("debug", Utils::toString((int)level));
}

void LdapParams::setAuthMethod(LinphoneLdapAuthMethod authMethod) {
	setCustomValue("auth_method", Utils::toString((int)authMethod));
}

void LdapParams::setServerCertificatesVerificationMode(LinphoneLdapCertVerificationMode mode) {
	setCustomValue("verify_server_certificates", Utils::toString((int)mode));
}

// Getters
std::string &LdapParams::getCustomValue(const std::string &key) {
	auto itValue = mConfig.find(key);
	if (itValue == mConfig.end()) return mDummyTxt;
	else return itValue->second;
}

const std::string &LdapParams::getCustomValue(const std::string &key) const {
	auto itValue = mConfig.find(key);
	if (itValue == mConfig.end()) return mDummyTxt;
	else return itValue->second;
}

const std::string &LdapParams::getServer() const {
	return getCustomValue("server");
}

const std::string &LdapParams::getBindDn() const {
	return getCustomValue("bind_dn");
}

const std::string &LdapParams::getBaseObject() const {
	return getCustomValue("base_object");
}

int LdapParams::getTimeout() const {
	return atoi(getCustomValue("timeout").c_str());
}

int LdapParams::getTimeoutTlsMs() const {
	return atoi(getCustomValue("timeout_tls_ms").c_str());
}

int LdapParams::getMaxResults() const {
	return atoi(getCustomValue("max_results").c_str());
}

int LdapParams::getMinChars() const {
	return atoi(getCustomValue("min_chars").c_str());
}

int LdapParams::getDelay() const {
	return atoi(getCustomValue("delay").c_str());
}

const std::string &LdapParams::getPassword() const {
	return getCustomValue("password");
}

const std::string &LdapParams::getFilter() const {
	return getCustomValue("filter");
}

const std::string &LdapParams::getNameAttribute() const {
	return getCustomValue("name_attribute");
}

const std::string &LdapParams::getSipAttribute() const {
	return getCustomValue("sip_attribute");
}

const std::string &LdapParams::getSipDomain() const {
	return getCustomValue("sip_domain");
}

bool LdapParams::getEnabled() const {
	return getCustomValue("enable") == "1";
}

bool LdapParams::salEnabled() const {
	return getCustomValue("use_sal") == "1";
}

bool LdapParams::tlsEnabled() const {
	return getCustomValue("use_tls") == "1";
}

LinphoneLdapDebugLevel LdapParams::getDebugLevel() const {
	return static_cast<LinphoneLdapDebugLevel>(atoi(getCustomValue("debug").c_str()));
}

LinphoneLdapAuthMethod LdapParams::getAuthMethod() const {
	return static_cast<LinphoneLdapAuthMethod>(atoi(getCustomValue("auth_method").c_str()));
}

LinphoneLdapCertVerificationMode LdapParams::getServerCertificatesVerificationMode() const {
	return static_cast<LinphoneLdapCertVerificationMode>(atoi(getCustomValue("verify_server_certificates").c_str()));
}

std::map<std::string, std::vector<std::string>> LdapParams::getConfig() const {
	std::map<std::string, std::vector<std::string>> result;
	for (auto it = mConfig.begin(); it != mConfig.end(); ++it) {
		result[it->first] = LdapConfigKeys::split(it->first, it->second);
	}
	return result;
}

int LdapParams::check() const {
	int checkResult = LinphoneLdapCheckOk;
	if (!LdapConfigKeys::validConfig(mConfig)) checkResult |= LinphoneLdapCheckMissingFields;
	checkResult |= checkServer();
	checkResult |= checkBaseObject();

	return checkResult;
}

int LdapParams::checkServer() const {
	int checkResult = LinphoneLdapCheckOk;
	auto servers = LdapConfigKeys::split("server", getServer());
	if (servers.size() == 0) checkResult |= LinphoneLdapCheckServerEmpty;
	else {
		for (size_t i = 0; i < servers.size(); ++i) {
			SalAddress *addr = sal_address_new(servers[i].c_str());
			if (!addr) checkResult |= LinphoneLdapCheckServerNotUrl;
			else {
				std::string scheme = sal_address_get_scheme(addr);
				if (scheme == "") checkResult |= LinphoneLdapCheckServerNoScheme;
				else {
					scheme = Utils::stringToLower(scheme);
					if (scheme == "ldaps") {
						checkResult |= LinphoneLdapCheckServerLdaps;
					} else if (scheme != "ldap") checkResult |= LinphoneLdapCheckServerNotLdap;
				}
				sal_address_unref(addr);
			}
		}
	}
	return checkResult;
}

int LdapParams::checkBaseObject() const {
	if (getBaseObject() == "") return LinphoneLdapCheckBaseObjectEmpty;
	else return LinphoneLdapCheckOk;
}

void LdapParams::lookupConfigEntryIndex() {
	int index = 0;
	string section = getSection(index);

	LpConfig *config = linphone_core_get_config(getCore()->getCCore());
	while (linphone_config_has_section(config, section.c_str())) {
		index += 1;
		section = getSection(index);
	}

	mConfigIndex = index;
	lInfo() << "[LDAP] This LdapParams object will use config section [" << section << "]";
}

void LdapParams::readFromConfigFile() {
	string section = getSection(mConfigIndex);
	LpConfig *config = linphone_core_get_config(getCore()->getCCore());
	lInfo() << "[LDAP] Reading config section [" << section << "]";

	bctbx_list_t *keys = linphone_config_get_keys_names_list(config, section.c_str());
	for (auto itKeys = keys; itKeys; itKeys = itKeys->next) {
		std::string key = static_cast<char *>(itKeys->data);
		mConfig[key] = linphone_config_get_string(config, section.c_str(), key.c_str(), "");
	}
	if (keys) bctbx_list_free(keys);
	mConfig = LdapConfigKeys::loadConfig(mConfig);
}

void LdapParams::writeToConfigFile() const {
	string section = getSection(mConfigIndex);
	LpConfig *config = linphone_core_get_config(getCore()->getCCore());

	for (auto it = mConfig.begin(); it != mConfig.end(); ++it) {
		linphone_config_set_string(config, section.c_str(), it->first.c_str(), it->second.c_str());
	}
	linphone_config_sync(config);
}

void LdapParams::removeFromConfigFile() const {
	string section = getSection(mConfigIndex);
	LpConfig *config = linphone_core_get_config(getCore()->getCCore());
	linphone_config_clean_section(config, section.c_str());
}

LINPHONE_END_NAMESPACE
