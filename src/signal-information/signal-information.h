/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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
#ifndef SIGNAL_INFORMATION_H
#define SIGNAL_INFORMATION_H

#include "belle-sip/object++.hh"
#include "linphone/api/c-types.h"
#include "linphone/enums/c-enums.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC SignalInformation : public bellesip::HybridObject<LinphoneSignalInformation, SignalInformation> {

public:
	SignalInformation() {};
	SignalInformation(LinphoneSignalType type, LinphoneSignalStrengthUnit unit, float value, std::string details = "")
	    : mType(type), mUnit(unit), mStrength(value), mDetails(details) {};
	SignalInformation(const SignalInformation &other);
	virtual ~SignalInformation();
	SignalInformation *clone() const override;
	float getStrength();
	void setStrength(float value);
	LinphoneSignalType getSignalType();
	void setSignalType(LinphoneSignalType);
	LinphoneSignalStrengthUnit getSignalUnit();
	void setSignalUnit(LinphoneSignalStrengthUnit);
	std::ostream &toStream(std::ostream &stream) const;
	void setDetails(const std::string &details);
	std::string getDetails();

	static const char *signalTypeToString(LinphoneSignalType type);

private:
	LinphoneSignalType mType;
	LinphoneSignalStrengthUnit mUnit;
	float mStrength;
	std::string mDetails;
};
LINPHONE_END_NAMESPACE

#endif // SIGNAL_INFORMATION_H
