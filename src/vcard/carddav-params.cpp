/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#include "carddav-params.h"
#include "c-wrapper/internal/c-sal.h"
#include "c-wrapper/internal/c-tools.h"
#include "linphone/api/c-address.h"
#include "linphone/types.h"
#include "linphone/utils/utils.h"
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

CardDavParams::CardDavParams(const shared_ptr<Core> &core) : CoreAccessor(core) {
	lookupConfigEntryIndex();
}

static string getSection(int index) {
	ostringstream ss;
	ss << "carddav_" << index;
	return ss.str();
}

CardDavParams::CardDavParams(const shared_ptr<Core> &core, int index) : CoreAccessor(core) {
	string section = getSection(index);
	LpConfig *config = linphone_core_get_config(core->getCCore());
	if (linphone_config_has_section(config, section.c_str())) {
		mConfigIndex = index;
		readFromConfigFile();
	} else {
		lWarning() << "[CardDAV] Config section [" << section << "] doesn't exists, looking for first available index";
		lookupConfigEntryIndex();
	}
}

CardDavParams::CardDavParams(const CardDavParams &other) : HybridObject(other), CoreAccessor(getCore()) {
	mConfigIndex = other.mConfigIndex;

	mServerUrl = other.mServerUrl;
	mLimit = other.mLimit;
	mMinCharactersToStartQuery = other.mMinCharactersToStartQuery;
	mTimeoutInSeconds = other.mTimeoutInSeconds;
	mFieldsToUseToFilterUsingUserInput = other.mFieldsToUseToFilterUsingUserInput;
	mFieldsToUseToFilterUsingDomain = other.mFieldsToUseToFilterUsingDomain;
	mUseExactMatchPolicy = other.mUseExactMatchPolicy;
}

CardDavParams *CardDavParams::clone() const {
	return new CardDavParams(*this);
}

void CardDavParams::lookupConfigEntryIndex() {
	int index = 0;
	string section = getSection(index);

	LpConfig *config = linphone_core_get_config(getCore()->getCCore());
	while (linphone_config_has_section(config, section.c_str())) {
		index += 1;
		section = getSection(index);
	}

	mConfigIndex = index;
	lInfo() << "[CardDAV] This CardDavParams object will use config section [" << section << "]";
}

void CardDavParams::readFromConfigFile() {
	string section = getSection(mConfigIndex);
	LpConfig *config = linphone_core_get_config(getCore()->getCCore());
	lInfo() << "[CardDAV] Reading config section [" << section << "]";

	mServerUrl = linphone_config_get_string(config, section.c_str(), "server_url", "");

	mLimit = (unsigned int)linphone_config_get_int(config, section.c_str(), "results_limit", 0);
	mMinCharactersToStartQuery = (unsigned int)linphone_config_get_int(config, section.c_str(), "min_characters", 3);
	mTimeoutInSeconds = (unsigned int)linphone_config_get_int(config, section.c_str(), "timeout", 5);

	mFieldsToUseToFilterUsingUserInput = Utils::configGetStringList(config, section, "fields_for_user_input");
	mFieldsToUseToFilterUsingDomain = Utils::configGetStringList(config, section, "fields_for_domain");

	mUseExactMatchPolicy = !!linphone_config_get_bool(config, section.c_str(), "use_exact_match_policy", FALSE);
}

void CardDavParams::writeToConfigFile() const {
	string section = getSection(mConfigIndex);
	LpConfig *config = linphone_core_get_config(getCore()->getCCore());
	linphone_config_clean_section(config, section.c_str());

	if (!mServerUrl.empty()) {
		Utils::configSetString(config, section, "server_url", mServerUrl);
	}

	linphone_config_set_int(config, section.c_str(), "results_limit", (int)mLimit);
	linphone_config_set_int(config, section.c_str(), "min_characters", (int)mMinCharactersToStartQuery);
	linphone_config_set_int(config, section.c_str(), "timeout", (int)mTimeoutInSeconds);

	Utils::configSetStringList(config, section, "fields_for_user_input", mFieldsToUseToFilterUsingUserInput);
	Utils::configSetStringList(config, section, "fields_for_domain", mFieldsToUseToFilterUsingDomain);

	linphone_config_set_bool(config, section.c_str(), "use_exact_match_policy", mUseExactMatchPolicy);
}

void CardDavParams::removeFromConfigFile() const {
	string section = getSection(mConfigIndex);
	LpConfig *config = linphone_core_get_config(getCore()->getCCore());
	linphone_config_clean_section(config, section.c_str());
}

LINPHONE_END_NAMESPACE
