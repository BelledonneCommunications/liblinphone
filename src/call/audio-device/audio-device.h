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
#ifndef AUDIO_DEVICE_H
#define AUDIO_DEVICE_H

#include <belle-sip/object++.hh>
#include "linphone/api/c-types.h"
#include "linphone/enums/call-enums.h"
#include <mediastreamer2/mssndcard.h>

LINPHONE_BEGIN_NAMESPACE

class AudioDevice : public bellesip::HybridObject<LinphoneAudioDevice, AudioDevice> {
public:
	enum Type {
        Unknown = LinphoneAudioDeviceTypeUnknown,
		Microphone = LinphoneAudioDeviceTypeMicrophone,
        Earpiece = LinphoneAudioDeviceTypeEarpiece,
        Speaker = LinphoneAudioDeviceTypeSpeaker,
        Bluetooth = LinphoneAudioDeviceTypeBluetooth,
        BluetoothA2DP = LinphoneAudioDeviceTypeBluetoothA2DP,
        Telephony = LinphoneAudioDeviceTypeTelephony,
        AuxLine = LinphoneAudioDeviceTypeAuxLine,
        GenericUsb = LinphoneAudioDeviceTypeGenericUsb,
        Headset = LinphoneAudioDeviceTypeHeadset,
        Headphones
	};

    enum Capabilities {
        Record = LinphoneAudioDeviceCapabilityRecord,
        Play = LinphoneAudioDeviceCapabilityPlay
    };

    AudioDevice(MSSndCard *soundCard);
    ~AudioDevice();

    MSSndCard *getSoundCard() const;
    const std::string& getId() const;
    const std::string& getDeviceName() const;
    const std::string& getDriverName() const;
    const Capabilities& getCapabilities() const;
    const Type& getType() const;

    std::string toString() const override;

    std::ostream& operator << (std::ostream& str) {
        str << this->toString();
        return str;
    }

private:
    MSSndCard *soundCard;
    std::string deviceId;
    std::string deviceName;
    std::string driverName;
    Capabilities capabilities;
    Type deviceType;
};

LINPHONE_END_NAMESPACE
#endif
