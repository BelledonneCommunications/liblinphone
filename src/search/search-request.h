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

#ifndef _L_SEARCH_REQUEST_H_
#define _L_SEARCH_REQUEST_H_

#include "linphone/types.h"
#include "linphone/utils/general.h"

#include "object/clonable-object.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC SearchRequest {
public:
	SearchRequest() {
	}
	SearchRequest(const std::string &filter,
	              const std::string &withDomain,
	              int sourceFlags,
	              LinphoneMagicSearchAggregation aggregation);
	SearchRequest(const SearchRequest &request);
	~SearchRequest();

	void operator=(const SearchRequest &other);

	/**
	 * @return filter of the search
	 **/
	const std::string &getFilter() const;

	/**
	 * @return domain which we want to search only
	 **/
	const std::string &getWithDomain() const;

	/**
	 * @return all sources where to search #LinphoneMagicSearchSource
	 **/
	int getSourceFlags() const;

	/**
	 * @return The #LinphoneMagicSearchAggregation mode set for this request
	 **/
	LinphoneMagicSearchAggregation getAggregation() const;

	/**
	 * @brief initStartTime To be call when starting a search. This will store the start time when the request begin.
	 */
	void initStartTime();

	/**
	 * @brief getStartTime Get the start time initialized by initStartTime().
	 * @return The start time.
	 */
	uint64_t getStartTime() const;

private:
	std::string mFilter;
	std::string mWithDomain;
	int mSourceFlags = LinphoneMagicSearchSourceAll;
	LinphoneMagicSearchAggregation mAggregation = LinphoneMagicSearchAggregationNone;

	/**
	 * @brief mStartTime Start time of the request
	 */
	uint64_t mStartTime;
};

LINPHONE_END_NAMESPACE

#endif
