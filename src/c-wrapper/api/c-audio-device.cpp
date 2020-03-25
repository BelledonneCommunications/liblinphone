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

#include "linphone/api/c-audio-device.h"
#include "call/audio-device/audio-device.h"
#include "c-wrapper/c-wrapper.h"

using namespace LinphonePrivate;

const char *linphone_audio_device_get_device_name(const LinphoneAudioDevice *audioDevice) {
    if (audioDevice) {
        return L_STRING_TO_C(AudioDevice::toCpp(audioDevice)->getDeviceName());
    }
    return NULL;
}

const char *linphone_audio_device_get_driver_name(const LinphoneAudioDevice *audioDevice) {
    if (audioDevice) {
        return L_STRING_TO_C(AudioDevice::toCpp(audioDevice)->getDriverName());
    }
    return NULL;
}

LinphoneAudioDeviceCapabilities linphone_audio_device_get_capabilities(const LinphoneAudioDevice *audioDevice) {
    return static_cast<LinphoneAudioDeviceCapabilities>(AudioDevice::toCpp(audioDevice)->getCapabilities());
}

LinphoneAudioDeviceType linphone_audio_device_get_type(const LinphoneAudioDevice *audioDevice) {
    return static_cast<LinphoneAudioDeviceType>(AudioDevice::toCpp(audioDevice)->getType());
}

LinphoneAudioDevice *linphone_audio_device_ref(LinphoneAudioDevice *audioDevice) {
    if (audioDevice) {
        AudioDevice::toCpp(audioDevice)->ref();
        return audioDevice;
    }
    return NULL;
}

void linphone_audio_device_unref(LinphoneAudioDevice *audioDevice) {
    if (audioDevice) {
        AudioDevice::toCpp(audioDevice)->unref();
    }
}