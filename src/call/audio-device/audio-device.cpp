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

#include "audio-device.h"
#include "logger/logger.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

AudioDevice::AudioDevice(MSSndCard *soundCard) : mSoundCard(ms_snd_card_ref(soundCard)) {
	const char *id = ms_snd_card_get_string_id(soundCard);
	mDeviceId = id;

	const char *name = ms_snd_card_get_name(soundCard);
	mDeviceName = name;

	unsigned int cap = ms_snd_card_get_capabilities(soundCard);
	if (cap & MS_SND_CARD_CAP_CAPTURE && cap & MS_SND_CARD_CAP_PLAYBACK) {
		mCapabilities =
		    static_cast<Capabilities>(static_cast<int>(Capabilities::Record) | static_cast<int>(Capabilities::Play));
	} else if (cap & MS_SND_CARD_CAP_CAPTURE) {
		mCapabilities = Capabilities::Record;
	} else if (cap & MS_SND_CARD_CAP_PLAYBACK) {
		mCapabilities = Capabilities::Play;
	}

	mDriverName = ms_snd_card_get_driver_type(soundCard);
}

AudioDevice::~AudioDevice() {
	ms_snd_card_unref(mSoundCard);
}

bool AudioDevice::operator==(const AudioDevice &device) const {
	return ((mSoundCard == device.getSoundCard()) && (mDeviceId.compare(device.getId()) == 0) &&
	        (mDeviceName.compare(device.getDeviceName()) == 0) && (mDriverName.compare(device.getDriverName()) == 0) &&
	        (mCapabilities == device.getCapabilities()) && (getType() == device.getType()));
}

bool AudioDevice::operator!=(const AudioDevice &device) const {
	return !(*this == device);
}

MSSndCard *AudioDevice::getSoundCard() const {
	return mSoundCard;
}

const string &AudioDevice::getId() const {
	return mDeviceId;
}

const string &AudioDevice::getDeviceName() const {
	return mDeviceName;
}

const string &AudioDevice::getDriverName() const {
	return mDriverName;
}

const AudioDevice::Capabilities &AudioDevice::getCapabilities() const {
	return mCapabilities;
}

const AudioDevice::Type &AudioDevice::getType() const {
	MSSndCardDeviceType type = ms_snd_card_get_device_type(mSoundCard);
	switch (type) {
		case MS_SND_CARD_DEVICE_TYPE_MICROPHONE:
			mDeviceType = AudioDevice::Type::Microphone;
			break;
		case MS_SND_CARD_DEVICE_TYPE_EARPIECE:
			mDeviceType = AudioDevice::Type::Earpiece;
			break;
		case MS_SND_CARD_DEVICE_TYPE_SPEAKER:
			mDeviceType = AudioDevice::Type::Speaker;
			break;
		case MS_SND_CARD_DEVICE_TYPE_BLUETOOTH:
			mDeviceType = AudioDevice::Type::Bluetooth;
			break;
		case MS_SND_CARD_DEVICE_TYPE_BLUETOOTH_A2DP:
			mDeviceType = AudioDevice::Type::BluetoothA2DP;
			break;
		case MS_SND_CARD_DEVICE_TYPE_TELEPHONY:
			mDeviceType = AudioDevice::Type::Telephony;
			break;
		case MS_SND_CARD_DEVICE_TYPE_AUX_LINE:
			mDeviceType = AudioDevice::Type::AuxLine;
			break;
		case MS_SND_CARD_DEVICE_TYPE_GENERIC_USB:
			mDeviceType = AudioDevice::Type::GenericUsb;
			break;
		case MS_SND_CARD_DEVICE_TYPE_HEADSET:
			mDeviceType = AudioDevice::Type::Headset;
			break;
		case MS_SND_CARD_DEVICE_TYPE_HEADPHONES:
			mDeviceType = AudioDevice::Type::Headphones;
			break;
		case MS_SND_CARD_DEVICE_TYPE_HEARING_AID:
			mDeviceType = AudioDevice::Type::HearingAid;
			break;
		case MS_SND_CARD_DEVICE_TYPE_UNKNOWN:
			mDeviceType = AudioDevice::Type::Unknown;
			break;
		default:
			lWarning() << "AudioDevice(): unmatched device type";
			mDeviceType = AudioDevice::Type::Unknown;
			break;
	}
	return mDeviceType;
}

bool AudioDevice::followsSystemRoutingPolicy() const {
	return !!(ms_snd_card_get_capabilities(mSoundCard) & MS_SND_CARD_CAP_FOLLOWS_SYSTEM_POLICY);
}

string AudioDevice::toString() const {
	std::ostringstream ss;
	ss << mDeviceName << ": driver [" << mDriverName << "], type [";
	switch (getType()) {
		case AudioDevice::Type::Microphone:
			ss << "Microphone";
			break;
		case AudioDevice::Type::Earpiece:
			ss << "Earpiece";
			break;
		case AudioDevice::Type::Speaker:
			ss << "Speaker";
			break;
		case AudioDevice::Type::Bluetooth:
			ss << "Bluetooth";
			break;
		case AudioDevice::Type::BluetoothA2DP:
			ss << "BluetoothA2DP";
			break;
		case AudioDevice::Type::Telephony:
			ss << "Telephony";
			break;
		case AudioDevice::Type::AuxLine:
			ss << "AuxLine";
			break;
		case AudioDevice::Type::GenericUsb:
			ss << "Generic USB";
			break;
		case AudioDevice::Type::Headset:
			ss << "Headset";
			break;
		case AudioDevice::Type::Headphones:
			ss << "Headphones";
			break;
		case AudioDevice::Type::HearingAid:
			ss << "Hearing Aid";
			break;
		case AudioDevice::Type::Unknown:
		default:
			ss << "Unknown";
			break;
	}
	ss << "]";
	return ss.str();
}

LINPHONE_END_NAMESPACE
