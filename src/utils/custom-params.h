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

#ifndef _L_CUSTOM_PARAMS_H_
#define _L_CUSTOM_PARAMS_H_

#include <map>
#include <string>

#include "linphone/lpconfig.h"
#include "linphone/utils/general.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC CustomParams {

public:
	CustomParams();
	CustomParams(const CustomParams &other);
	virtual ~CustomParams();

	void addCustomParam(const std::string &key, const std::string &value);
	const std::string &getCustomParam(const std::string &key) const;

protected:
	void writeCustomParamsToConfigFile(LinphoneConfig *config, std::string configKey) const;
	void readCustomParamsFromConfigFile(LinphoneConfig *config, const char *key);

private:
	static const std::string paramPrefix;
	std::map<std::string, std::string> params;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CUSTOM_PARAMS_H_
