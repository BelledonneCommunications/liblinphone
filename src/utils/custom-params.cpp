/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{
}
 without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "custom-params.h"
#include "logger/logger.h"
#include "linphone/utils/utils.h"

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

void CustomParams::addCustomParam(const std::string & key, const std::string & value) {
	params[key] = value;
}

const std::string & CustomParams::getCustomParam(const std::string & key) const {
	try {
		return params.at(key);
	} catch (std::out_of_range&) {
		lError() << "Unable to find parameter with key " << key;
		return Utils::getEmptyConstRefObject<std::string>();
	}
}

void CustomParams::writeCustomParamsToConfigFile (LinphoneConfig *config, std::string configKey) const {
	for (const auto & p : params) {
		const auto & key = p.first;
		const auto & value = p.second;
		const auto paramsName(std::string(paramPrefix) + key);
		linphone_config_set_string(config, configKey.c_str(), paramsName.c_str(), value.c_str());
	}
}

LINPHONE_END_NAMESPACE
