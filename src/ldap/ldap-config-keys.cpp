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

#include "ldap-config-keys.h"
#include "linphone/utils/utils.h"
#include <algorithm>
#include <fstream>

LINPHONE_BEGIN_NAMESPACE

static const std::map<std::string, LdapConfigKeys> gLdapConfigKeys = {
    {"timeout", LdapConfigKeys("5", '\0', false)},
    {"timeout_tls_ms", LdapConfigKeys("1000", '\0', false)},
    {"max_results", LdapConfigKeys("5", '\0', false)},
    {"min_chars", LdapConfigKeys("0", '\0', false)},
    {"delay", LdapConfigKeys("500", '\0', false)},
    {"auth_method", LdapConfigKeys(Utils::toString((int)LinphoneLdapAuthMethodSimple), '\0', false)},
    {"password", LdapConfigKeys("", '\0', false)},
    {"bind_dn", LdapConfigKeys("", '\0', false)},
    {"base_object", LdapConfigKeys("dc=example,dc=com", '\0', true)},
    {"server", LdapConfigKeys("ldap:///", ',', true)},
    {"filter", LdapConfigKeys("(sn=*%s*)", '\0', false)},
    {"name_attribute", LdapConfigKeys("sn", ',', false)},
    {"sip_attribute", LdapConfigKeys("mobile,telephonenumber,homephone,sn", ',', false)},
    {"sip_domain", LdapConfigKeys("", '\0', false)},
    {"enable", LdapConfigKeys("0", '\0', false)},
    // Use our own DNS resolver, which is asynchronous. Much better than openldap blocking getaddrinfo().
    {"use_sal", LdapConfigKeys("1", '\0', false)},
    {"use_tls", LdapConfigKeys("1", '\0', false)},
    // deprecated: LDAP debug logs are now set according to the liblinphone log level.
    {"debug", LdapConfigKeys(Utils::toString((int)LinphoneLdapDebugLevelOff), '\0', false)},
    {"verify_server_certificates", LdapConfigKeys(Utils::toString((int)LinphoneLdapCertVerificationDefault),
                                                  '\0',
                                                  false)} // -1:auto from core, 0:deactivate, 1:activate
};

LdapConfigKeys::LdapConfigKeys(const std::string &value, const char separator, const bool required)
    : mValue(value), mSeparator(separator), mRequired(required) {
}

LdapConfigKeys LdapConfigKeys::getConfigKeys(const std::string &key) {
	return gLdapConfigKeys.count(key) > 0 ? gLdapConfigKeys.at(key) : LdapConfigKeys();
}

std::vector<std::string> LdapConfigKeys::split(const std::string &key, const std::string &values) {
	auto configKeys = getConfigKeys(key);
	char separator = configKeys.mSeparator;
	if (separator == '\0') return std::vector<std::string>{values};
	std::vector<std::string> tokens;
	std::istringstream iss(values);
	std::string s;
	while (std::getline(iss, s, separator)) {
		tokens.push_back(s);
	}
	return tokens;
}

std::string LdapConfigKeys::join(const std::string &key, const std::vector<std::string> &values) {
	auto configKeys = getConfigKeys(key);
	char separator = configKeys.mSeparator;
	if (separator == '\0') return values[0];
	std::string value = values[0];
	for (size_t i = 1; i < values.size(); ++i)
		value.append(separator + values[i]);
	return value;
}

bool LdapConfigKeys::validConfig(const std::map<std::string, std::vector<std::string>> &config) {
	bool valid = true;
	for (auto it = gLdapConfigKeys.begin(); it != gLdapConfigKeys.end(); ++it)
		if (it->second.mRequired && config.count(it->first) <= 0) {
			ms_error("[LDAP] : Missing LDAP config value for '%s'", it->first.c_str());
			valid = false;
		}
	return valid;
}

bool LdapConfigKeys::validConfig(const std::map<std::string, std::string> &config) {
	bool valid = true;
	for (auto it = gLdapConfigKeys.begin(); it != gLdapConfigKeys.end(); ++it)
		if (it->second.mRequired && config.count(it->first) <= 0) {
			ms_error("[LDAP] : Missing LDAP config value for '%s'", it->first.c_str());
			valid = false;
		}
	return valid;
}

std::unordered_set<std::string>
LdapConfigKeys::getUniqueAttributes(const std::map<std::string, std::vector<std::string>> &splittedConfig,
                                    const std::vector<std::string> &keys) {
	std::unordered_set<std::string> result;
	for (auto it = keys.begin(); it != keys.end(); ++it) {
		if (splittedConfig.count(*it) > 0) {
			const std::vector<std::string> &values = splittedConfig.at(*it);
			for (size_t i = 0; i < values.size(); ++i)
				result.insert(values[i]);
		}
	}
	return result;
}

std::map<std::string, std::vector<std::string>>
LdapConfigKeys::loadConfig(const std::map<std::string, std::vector<std::string>> &config) {
	std::map<std::string, std::vector<std::string>> finalConfig;
	for (auto conf : config)
		if (gLdapConfigKeys.count(conf.first) == 0) finalConfig[conf.first] = conf.second;
	for (auto it = gLdapConfigKeys.begin(); it != gLdapConfigKeys.end(); ++it) {
		finalConfig[it->first] =
		    config.count(it->first) > 0 ? config.at(it->first) : LdapConfigKeys::split(it->first, it->second.mValue);
	}
	return finalConfig;
}

std::map<std::string, std::string> LdapConfigKeys::loadConfig(const std::map<std::string, std::string> &config) {
	std::map<std::string, std::string> finalConfig;
	for (auto conf : config)
		if (gLdapConfigKeys.count(conf.first) == 0) finalConfig[conf.first] = conf.second;
	for (auto it = gLdapConfigKeys.begin(); it != gLdapConfigKeys.end(); ++it) {
		finalConfig[it->first] = config.count(it->first) > 0 ? config.at(it->first) : it->second.mValue;
	}
	return finalConfig;
}
LINPHONE_END_NAMESPACE
