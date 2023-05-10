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

#include "linphone/api/c-signal-information.h"
#include "c-wrapper/c-wrapper.h"
#include "signal-information/signal-information.h"

using namespace LinphonePrivate;

LinphoneSignalInformation *linphone_signal_information_clone(const LinphoneSignalInformation *signalInformation) {
	return SignalInformation::toCpp(signalInformation)->clone()->toC();
}
LinphoneSignalInformation *linphone_signal_information_ref(LinphoneSignalInformation *signalInformation) {
	SignalInformation::toCpp(signalInformation)->ref();
	return signalInformation;
}
void linphone_signal_information_unref(LinphoneSignalInformation *signalInformation) {
	SignalInformation::toCpp(signalInformation)->unref();
}

float linphone_signal_information_get_strength(LinphoneSignalInformation *signalInformation) {
	return SignalInformation::toCpp(signalInformation)->getStrength();
}
void linphone_signal_information_set_value(LinphoneSignalInformation *signalInformation, float value) {
	SignalInformation::toCpp(signalInformation)->setStrength(value);
}
LinphoneSignalType linphone_signal_information_get_signal_type(LinphoneSignalInformation *signalInformation) {
	return SignalInformation::toCpp(signalInformation)->getSignalType();
}
void linphone_signal_information_set_signal_type(LinphoneSignalInformation *signalInformation,
                                                 LinphoneSignalType type) {
	SignalInformation::toCpp(signalInformation)->setSignalType(type);
}
LinphoneSignalStrengthUnit linphone_signal_information_get_signal_unit(LinphoneSignalInformation *signalInformation) {
	return SignalInformation::toCpp(signalInformation)->getSignalUnit();
}
void linphone_signal_information_set_signal_unit(LinphoneSignalInformation *signalInformation,
                                                 LinphoneSignalStrengthUnit unit) {
	SignalInformation::toCpp(signalInformation)->setSignalUnit(unit);
}