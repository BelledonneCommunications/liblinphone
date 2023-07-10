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

#include "custom-params.h"
#include "linphone/utils/utils.h"
#include "logger/logger.h"

LINPHONE_BEGIN_NAMESPACE

const std::string CustomParams::paramPrefix = "x-custom-property:";

CustomParams::CustomParams() {
	params.clear();
}

CustomParams::CustomParams(const CustomParams &other) {
	params = other.params;
}

CustomParams::~CustomParams() {
}

void CustomParams::addCustomParam(const std::string &key, const std::string &value) {
	params[key] = value;
}

const std::string &CustomParams::getCustomParam(const std::string &key) const {
	try {
		return params.at(key);
	} catch (std::out_of_range &) {
		lDebug() << "Unable to find parameter with key " << key;
		return Utils::getEmptyConstRefObject<std::string>();
	}
}

void CustomParams::writeCustomParamsToConfigFile(LinphoneConfig *config, std::string configKey) const {
	for (const auto &[key, value] : params) {
		const auto paramsName(std::string(paramPrefix) + key);
		linphone_config_set_string(config, configKey.c_str(), paramsName.c_str(), value.c_str());
	}
}

void CustomParams::readCustomParamsFromConfigFile(LinphoneConfig *config, const char *key) {
	bctbx_list_t *param_names = linphone_config_get_keys_names_list(config, key);
	for (auto param_name_it = param_names; param_name_it != NULL; param_name_it = param_name_it->next) {
		const char *param_name = (const char *)param_name_it->data;
		// If it is a custom parameter
		if (param_name && strstr(param_name, paramPrefix.c_str())) {
			const std::string value(linphone_config_get_string(config, key, param_name, ""));
			std::string param(param_name);
			const auto start_param_name = param.find(paramPrefix);
			const auto prunedKey = param.substr(start_param_name + paramPrefix.size());
			lInfo() << "Adding custom parameter " << prunedKey << " with value " << value << " from config section "
			        << std::string(key);
			addCustomParam(prunedKey, value);
		}
	}
	if (param_names) {
		bctbx_list_free(param_names);
	}
}

LINPHONE_END_NAMESPACE
