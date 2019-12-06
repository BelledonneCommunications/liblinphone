/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
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

#ifndef ice_service_h
#define ice_service_h

#include <memory>

#include "call-session.h"

LINPHONE_BEGIN_NAMESPACE


class IceService{
public:
	bool isEnabled() const;
	/**
	 * Called by a stream to obtain its ICE check list.
	 */
	IceCheckList *getCheckList(size_t index);
	/**
	 * Called by the StreamsGroup when the local media description must be filled with ICE parameters.
	 */
	void updateLocalMediaDescription(SalMediaDescription *localDesc);
	
};

class IceServiceListener{
	virtual void onGatheringFinished(IceService &service) = 0;
	virtual void onIceCompleted(IceService &service) = 0;
	virtual void onIceFailed(IceService & service) = 0;
};

LINPHONE_END_NAMESPACE


