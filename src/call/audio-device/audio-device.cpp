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

#include "audio-device.h"
#include "logger/logger.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

AudioDevice::AudioDevice(const MSSndCard *soundCard, const string &deviceName, const string &driverName, const AudioDevice::Capabilities &capabilities, const AudioDevice::Type& deviceType)
    :soundCard(soundCard), deviceName(deviceName), driverName(driverName), capabilities(capabilities), deviceType(deviceType)
{

}

const string& AudioDevice::getDeviceName() const {
    return deviceName;
}

const string& AudioDevice::getDriverName() const {
    return driverName;
}

const AudioDevice::Capabilities& AudioDevice::getCapabilities() const {
    return capabilities;
}

const AudioDevice::Type& AudioDevice::getType() const {
    return deviceType;
}

string AudioDevice::toString() const {
    std::ostringstream ss;
    ss << driverName << ": driver [" << driverName << "]";
    return ss.str();
}

LINPHONE_END_NAMESPACE