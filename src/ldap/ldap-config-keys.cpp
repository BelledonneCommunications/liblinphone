/*
 * Copyright (c) 2021 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include "ldap-config-keys.h"

#include <algorithm>
#include <fstream>

LINPHONE_BEGIN_NAMESPACE

static const std::map<std::string, LdapConfigKeys> gLdapConfigKeys={
	{"timeout", LdapConfigKeys("5")},
	{"max_results", LdapConfigKeys("5")},
	{"auth_method", LdapConfigKeys("SIMPLE")},
	{"password", LdapConfigKeys("")},
	{"bind_dn", LdapConfigKeys("")},
	{"base_object", LdapConfigKeys("dc=example,dc=com", TRUE)},
	{"server", LdapConfigKeys("ldap:///", TRUE)},
	{"filter", LdapConfigKeys("(sn=*%s*)")},
	{"name_attribute", LdapConfigKeys("sn")},
	{"sip_attribute", LdapConfigKeys("mobile,telephoneNumber,homePhone,sn")},
	{"sip_domain", LdapConfigKeys("")},
	{"enable", LdapConfigKeys("1")},
	{"use_sal", LdapConfigKeys("0")},
	{"use_tls", LdapConfigKeys("1")},
	{"debug", LdapConfigKeys("0")},
	{"verify_server_certificates", LdapConfigKeys("-1")}// -1:auto from core, 0:deactivate, 1:activate
};

LdapConfigKeys::LdapConfigKeys(const std::string& pValue, const bool_t& pRequired) : value(pValue), required(pRequired){}
	
std::vector<std::string> LdapConfigKeys::split(const std::string& pValue){
	std::vector<std::string> tokens;
	std::istringstream iss(pValue);
	std::string s;    
	while (std::getline(iss, s, ',')) {
		tokens.push_back(s);
	}
	return tokens;
}

bool_t LdapConfigKeys::validConfig(const std::map<std::string, std::string>& config) {
	bool_t valid = TRUE;
	for(auto it = gLdapConfigKeys.begin() ; it != gLdapConfigKeys.end() ; ++it)
		if( it->second.required && config.count(it->first)<=0){
			ms_error("[LDAP] : Missing LDAP config value for '%s'", it->first.c_str());
			valid = FALSE;
		}
	return valid;
}

std::map<std::string,std::string>  LdapConfigKeys::loadConfig(const std::map<std::string, std::string>& config
															  , std::vector<std::string> * pNameAttributes
															  , std::vector<std::string> * pSipAttributes
															  , std::vector<std::string> * pAttributes
															  ) {
	std::map<std::string,std::string> finalConfig;
	for(auto it = gLdapConfigKeys.begin() ; it != gLdapConfigKeys.end() ; ++it)
		finalConfig[it->first] = (config.count(it->first)>0 ? config.at(it->first) : it->second.value);
	*pNameAttributes = LdapConfigKeys::split(finalConfig["name_attribute"]);
	*pSipAttributes = LdapConfigKeys::split(finalConfig["sip_attribute"]);
// Get first array and then keep only unique
	*pAttributes = *pNameAttributes;
	for(auto it = pSipAttributes->begin() ; it != pSipAttributes->end() ; ++it)
		if( std::find(pAttributes->begin(), pAttributes->end(), *it) == pAttributes->end())
			pAttributes->push_back(*it);
	return finalConfig;
}

LINPHONE_END_NAMESPACE
