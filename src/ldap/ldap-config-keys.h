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

#ifndef LINPHONE_LDAP_CONFIG_KEYS_H
#define LINPHONE_LDAP_CONFIG_KEYS_H

#include "linphone/types.h"

#include <map>
#include <string>
#include <unordered_set>
#include <vector>

LINPHONE_BEGIN_NAMESPACE

class LdapConfigKeys {
public:
	LdapConfigKeys(const std::string &pValue = "", const char pSeparator = '\0', const bool pRequired = false);

	/**
	 *  Manage Configurations.
	 *	An instance store a configuration value in order to customize attributes.
	 *
	 * Available Keys : default.
	 *
	 *   - "server" : "ldap:///", Required.
	 * LDAP Server. eg: ldap:/// for a localhost server or ldap://ldap.example.org/
	 *
	 *   - "bind_dn" : "".
	 * Bind DN to use for bindings. The bindDN DN is the credential that is used to authenticate against an LDAP. If
	 *empty, the connection will be Anonymous. eg: cn=ausername,ou=people,dc=bc,dc=com
	 *
	 *   - "base_object" : "dc=example,dc=com", Required.
	 * BaseObject is a specification for LDAP Search Scopes that specifies that the Search Request should only be
	 *performed against the entry specified as the search base DN. No entries above it will be considered.
	 *
	 *   - "timeout" : "5".
	 * Timeout in seconds
	 *
	 *   - "timeout_tls_ms" : "1000".
	 * Timeout in milliseconds
	 *
	 *   - "min_chars" : "0"
	 * The minimum characters needed for doing a search.
	 *
	 *   - "max_results" : "5".
	 * The max results when requesting searches.
	 *
	 *   - "delay" : "500".
	 * The delay between each search in milliseconds.
	 *
	 *   - "auth_method" : "SIMPLE".
	 * Authentification method. Only "SIMPLE" and "ANONYMOUS" are supported.
	 *
	 *   - "password" : "".
	 * Password to pass to server when binding.
	 *
	 *   - "filter" : "(sn=*%s*)".
	 * The search is based on this filter to search contacts.
	 * Multiple criteria can be write as (|(sn=*%s*)(cn=*%s*))
	 *
	 *   - "name_attribute" : "sn".
	 * Check these attributes to build Name Friend, separated by a comma and the first is the highest priority.
	 * Name concatenation is specified with '+' like "cn+sn"
	 *
	 *   - "sip_attribute" : "mobile,telephoneNumber,homePhone,sn".
	 * Check these attributes to build the SIP username in address of Friend. Attributes are separated by a comma.
	 *
	 *   - "sip_domain" : "".
	 * Add the domain to the sip address(sip:username@domain). If empty, the domain will be specify while searching on
	 *the current proxy account.
	 *
	 *   - "enable" : "0".
	 * If this config is enabled.
	 *
	 *   - "use_sal" : "0".
	 * The dns resolution is done by Linphone using Sal. It will pass an IP to LDAP. By doing that, the TLS negociation
	 *could not check the hostname. You may deactivate the verifications if wanted to force the connection.
	 *
	 *   - "use_tls" : "1".
	 * Encrypt transactions by LDAP over TLS(StartTLS). You must use \'ldap\' scheme. \'ldaps\' for LDAP over SSL is
	 *non-standardized and deprecated. StartTLS in an extension to the LDAP protocol which uses the TLS protocol to
	 *encrypt communication. It works by establishing a normal - i.e. unsecured - connection with the LDAP server before
	 *a handshake negotiation between the server and the web services is carried out. Here, the server sends its
	 *certificate to prove its identity before the secure connection is established.
	 *
	 *   - "debug" : "0".
	 * Debug mode
	 *
	 *   - "verify_server_certificates" : "-1". values: -1:auto from core, 0:deactivate, 1:activate
	 * Specify whether the tls server certificate must be verified when connecting to a LDAP server.
	 **/

	std::string mValue;

	/**
	 *	Specify the separator character if splittable. Use null character ('\0') to disable the feature.
	 **/
	char mSeparator;

	/**
	 *	Specify if this value is required for the configuration.
	 **/
	bool mRequired;

	/**
	 * Get the default LdapConfigKeys from key.
	 * @param key the key configuration.
	 * @return The associated LdapConfigKeys
	 **/
	static LdapConfigKeys getConfigKeys(const std::string &key);

	/**
	 * Split a string into an array of token using ',' separator if it is not defined by config key.
	 * @param key the key configuration that is used to retrieve separator from keys list.
	 * @param value The string to split
	 * @return An array of string
	 **/
	static std::vector<std::string> split(const std::string &key, const std::string &value);

	/**
	 * Join an array of string into a string using ',' separator by default or the separator coming from config key.
	 * @param key the key configuration that is used to retrieve separator from keys list.
	 * @param value The string to join
	 * @return the joined string.
	 **/
	static std::string join(const std::string &key, const std::vector<std::string> &pValues);

	/**
	 * Load a full configuration from an existant. The return value is the config with default value and a parsing is
	 *done to give attributes arrays.
	 * @param config The configuration <name,value>
	 * @return If the configuration is valid. @notnil
	 **/
	static bool validConfig(const std::map<std::string, std::vector<std::string>> &config);
	static bool validConfig(const std::map<std::string, std::string> &config);

	/**
	 * Parse Ldap configuration coming from loadConfig to retrieve all keys and return all unique attributes.
	 * @param splittedConfig The configuration <name,<value>>
	 * @param keys Keys to check
	 * @return The unique attributes
	 **/
	std::unordered_set<std::string>
	getUniqueAttributes(const std::map<std::string, std::vector<std::string>> &splittedConfig,
	                    const std::vector<std::string> &keys);

	/**
	 * Load a full configuration from an existant. The return value is the config with default value and a parsing is
	 *done to give attributes arrays.
	 * @param config The configuration <name,value>
	 * @return The configuration map keys.
	 **/
	static std::map<std::string, std::vector<std::string>>
	loadConfig(const std::map<std::string, std::vector<std::string>> &config =
	               std::map<std::string, std::vector<std::string>>());
	static std::map<std::string, std::string> loadConfig(const std::map<std::string, std::string> &config);
};
LINPHONE_END_NAMESPACE

#endif /* LINPHONE_LDAP_CONFIG_KEYS_H */
