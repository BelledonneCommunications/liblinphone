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

#include "signal-information.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

SignalInformation::SignalInformation(const SignalInformation &other) : HybridObject(other) {
}

SignalInformation::~SignalInformation() {
}

SignalInformation *SignalInformation::clone() const {
	return new SignalInformation(*this);
}
float SignalInformation::getStrength() {
	return mStrength;
}
void SignalInformation::setStrength(float value) {
	mStrength = value;
}
LinphoneSignalType SignalInformation::getSignalType() {
	return mType;
}
void SignalInformation::setSignalType(LinphoneSignalType type) {
	mType = type;
}
LinphoneSignalStrengthUnit SignalInformation::getSignalUnit() {
	return mUnit;
}
void SignalInformation::setSignalUnit(LinphoneSignalStrengthUnit unit) {
	mUnit = unit;
}
std::ostream &SignalInformation::toStream(std::ostream &stream) const {
	stream << "Type : " << mType << " | Unit : " << mUnit << " | Strength : " << mStrength << "Details : " << mDetails;
	return stream;
}
void SignalInformation::setDetails(const std::string &details) {
	mDetails = details;
}
std::string SignalInformation::getDetails() {
	return mDetails;
}

const char *SignalInformation::signalTypeToString(LinphoneSignalType type) {
	switch (type) {
		case LinphoneSignalTypeWifi:
			return "wifi";
		case LinphoneSignalTypeMobile:
			return "mobile";
		case LinphoneSignalTypeOther:
			return "other";
	}
	return "invalid";
}

LINPHONE_END_NAMESPACE
