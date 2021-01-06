/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
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

#include <algorithm>

#include "sal/sal_stream_bundle.h"
#include "sal/sal_stream_description.h"

SalStreamBundle::SalStreamBundle(){
	mids.clear();
}

SalStreamBundle::SalStreamBundle(const SalStreamBundle &other){
	mids = other.mids;
}

SalStreamBundle::~SalStreamBundle(){
	mids.clear();
}

SalStreamBundle &SalStreamBundle::operator=(const SalStreamBundle & other){
	mids = other.mids;
	return *this;
}

void SalStreamBundle::addStream(SalStreamDescription & stream, const std::string &mid){
	stream.mid = mid;
	mids.push_back(mid);
}

const std::string & SalStreamBundle::getMidOfTransportOwner() const {
	return mids.front(); /* the first one is the transport owner*/
}

bool SalStreamBundle::hasMid(const std::string & mid) const{
	const auto & midIt = std::find_if(mids.cbegin(), mids.cend(), [&mid] (const auto & bundleMid) {
		return (bundleMid.compare(mid) == 0);
	});
	return (midIt != mids.cend());
}
