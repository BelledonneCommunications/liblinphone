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

#ifndef _L_CARDDAV_PARAMS_H_
#define _L_CARDDAV_PARAMS_H_

#include "belle-sip/object++.hh"
#include "linphone/api/c-types.h"
#include "linphone/types.h"

#include "core/core-accessor.h"
#include "core/core.h"

#include <list>
#include <string>

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class CardDavParams : public bellesip::HybridObject<LinphoneCardDavParams, CardDavParams>, public CoreAccessor {
public:
	CardDavParams(const std::shared_ptr<Core> &core);
	CardDavParams(const std::shared_ptr<Core> &core, int index);
	CardDavParams(const CardDavParams &other);
	virtual ~CardDavParams() {};

	CardDavParams *clone() const override;

	const std::string &getServerUrl() const {
		return mServerUrl;
	}

	void setServerUrl(const std::string &serverUrl) {
		mServerUrl = serverUrl;
	}

	unsigned int getLimit() const {
		return mLimit;
	}

	void setLimit(unsigned int limit) {
		mLimit = limit;
	}

	unsigned int getMinCharactersToStartQuery() const {
		return mMinCharactersToStartQuery;
	}

	void setMinCharactersToStartQuery(unsigned int min) {
		mMinCharactersToStartQuery = min;
	}

	unsigned int getTimeout() const {
		return mTimeoutInSeconds;
	}

	void setTimeout(unsigned int seconds) {
		mTimeoutInSeconds = seconds;
	}

	const std::list<std::string> &getFieldsToFilterUserInputOn() const {
		return mFieldsToUseToFilterUsingUserInput;
	}

	void setFieldsToFilterUserInputOn(std::list<std::string> list) {
		mFieldsToUseToFilterUsingUserInput = list;
	}

	void addFieldToFilterUserInputOn(const std::string &field) {
		mFieldsToUseToFilterUsingUserInput.push_back(field);
	}

	const std::list<std::string> &getFieldsToFilterDomainOn() const {
		return mFieldsToUseToFilterUsingDomain;
	}

	void setFieldsToFilterDomainOn(std::list<std::string> list) {
		mFieldsToUseToFilterUsingDomain = list;
	}

	void addFieldToFilterDomainOn(const std::string &field) {
		mFieldsToUseToFilterUsingDomain.push_back(field);
	}

	bool getUseExactMatchPolicy() const {
		return mUseExactMatchPolicy;
	}

	void setUseExactMatchPolicy(bool exactMatch) {
		mUseExactMatchPolicy = exactMatch;
	}

	void writeToConfigFile() const;
	void removeFromConfigFile() const;

private:
	void lookupConfigEntryIndex();
	void readFromConfigFile();

	int mConfigIndex = -1;

	std::string mServerUrl;
	unsigned int mLimit = 0;
	unsigned int mMinCharactersToStartQuery = 1;
	unsigned int mTimeoutInSeconds = 5;
	std::list<std::string> mFieldsToUseToFilterUsingUserInput;
	std::list<std::string> mFieldsToUseToFilterUsingDomain;
	bool mUseExactMatchPolicy = false;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CARDDAV_PARAMS_H_
