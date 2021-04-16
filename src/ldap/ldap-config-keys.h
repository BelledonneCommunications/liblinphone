/*
 * Copyright (c) 21021 Belledonne Communications SARL.
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

#ifndef LINPHONE_LDAP_CONFIG_KEYS_H
#define LINPHONE_LDAP_CONFIG_KEYS_H

#include "linphone/types.h"

#include <map>
#include <string>
#include <vector>

LINPHONE_BEGIN_NAMESPACE

class LdapConfigKeys{
public:
	LdapConfigKeys(const std::string& pValue, const bool_t& pRequired=FALSE);

	/**
	 *  Manage Configurations.
	 *	An instance store a configuration value in order to customize attributes.
	 * 
	 **/

	std::string value;

	/**
	 *	Specify if this value is required for the configuration. 
	 **/
	bool_t required;

	/**
	 * Split a string into an array of token using ',' separator
	 * @param value The string to split
	 * @return An array of string
	**/
	static std::vector<std::string> split(const std::string& value);
	
	/**
	 * Load a full configuration from an existant. The return value is the config with default value and a parsing is done to give attributes arrays.
	 * @param config The configuration <name,value>
	 * @param nameAttributes All values attributes from 'name_attribute' key
	 * @param sipAttributes All values attributes from 'sip_attribute' key
	 * @param attributes All unique attributes from keys'name_attribute' and 'sip_attribute' both
	 * @return The #LinphoneAccountCbs object. @notnil
	**/
	static bool_t validConfig(const std::map<std::string, std::string>& config);
	/**
	 * Load a full configuration from an existant. The return value is the config with default value and a parsing is done to give attributes arrays.
	 * @param config The configuration <name,value>
	 * @param nameAttributes All values attributes from 'name_attribute' key
	 * @param sipAttributes All values attributes from 'sip_attribute' key
	 * @param attributes All unique attributes from keys'name_attribute' and 'sip_attribute' both
	 * @return The #LinphoneAccountCbs object. @notnil
	**/
	static std::map<std::string,std::string> loadConfig(const std::map<std::string, std::string>& config
																  , std::vector<std::string> * nameAttributes
																  , std::vector<std::string> * sipAttributes
																  , std::vector<std::string> * attributes);
};
LINPHONE_END_NAMESPACE

#endif /* LINPHONE_LDAP_CONFIG_KEYS_H */
