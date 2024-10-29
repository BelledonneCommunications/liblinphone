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

#include "search-request.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

SearchRequest::SearchRequest(const string &filter,
                             const string &withDomain,
                             int sourceFlags,
                             LinphoneMagicSearchAggregation aggregation) {
	mFilter = filter;
	mWithDomain = withDomain;
	mSourceFlags = sourceFlags;
	mAggregation = aggregation;
	mStartTime = 0;
}

SearchRequest::SearchRequest(const SearchRequest &request) {
	mFilter = request.mFilter;
	mWithDomain = request.mWithDomain;
	mSourceFlags = request.mSourceFlags;
	mAggregation = request.mAggregation;
	mStartTime = request.mStartTime;
}

SearchRequest::~SearchRequest() {
}

void SearchRequest::operator=(const SearchRequest &request) {
	mFilter = request.mFilter;
	mWithDomain = request.mWithDomain;
	mSourceFlags = request.mSourceFlags;
	mAggregation = request.mAggregation;
	mStartTime = request.mStartTime;
}

const string &SearchRequest::getFilter() const {
	return mFilter;
}

const string &SearchRequest::getWithDomain() const {
	return mWithDomain;
}

int SearchRequest::getSourceFlags() const {
	return mSourceFlags;
}

LinphoneMagicSearchAggregation SearchRequest::getAggregation() const {
	return mAggregation;
}

void SearchRequest::initStartTime() {
	mStartTime = bctbx_get_cur_time_ms();
}

uint64_t SearchRequest::getStartTime() const {
	return mStartTime;
}

LINPHONE_END_NAMESPACE
