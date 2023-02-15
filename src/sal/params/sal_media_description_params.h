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
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _L_SAL_MEDIA_DESCRIPTION_PARAMS_H_
#define _L_SAL_MEDIA_DESCRIPTION_PARAMS_H_

// =============================================================================
#include "linphone/utils/general.h"

LINPHONE_BEGIN_NAMESPACE

class CallSessionParams;

class LINPHONE_PUBLIC SalMediaDescriptionParams {

public:
	SalMediaDescriptionParams();
	SalMediaDescriptionParams(const CallSessionParams *callParams);
	SalMediaDescriptionParams(const SalMediaDescriptionParams &other);
	SalMediaDescriptionParams &operator=(const SalMediaDescriptionParams &other);
	virtual ~SalMediaDescriptionParams();

	void enableCapabilityNegotiationSupport(const bool enable);
	bool capabilityNegotiationSupported() const;
	void enableCfgLinesMerging(const bool enable);
	bool cfgLinesMerged() const;
	void enableTcapLineMerging(const bool enable);
	bool tcapLinesMerged() const;

private:
	bool capabilityNegotiation = false;
	bool mergeCfgLines = false;
	bool mergeTcapLines = false;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _SAL_MEDIA_DESCRIPTION_PARAMS_H_
