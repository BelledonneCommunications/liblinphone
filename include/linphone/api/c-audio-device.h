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

#ifndef LINPHONE_AUDIO_DEVICE_H
#define LINPHONE_AUDIO_DEVICE_H

#include "linphone/api/c-types.h"

/**
 * @addtogroup audio
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns the id of the audio device
 * @param[in] audioDevice the #LinphoneAudioDevice
 * @return the id of the audio device
 */
LINPHONE_PUBLIC const char *linphone_audio_device_get_id(const LinphoneAudioDevice *audioDevice);

/**
 * Returns the name of the audio device
 * @param[in] audioDevice the #LinphoneAudioDevice
 * @return the name of the audio device
 */
LINPHONE_PUBLIC const char *linphone_audio_device_get_device_name(const LinphoneAudioDevice *audioDevice);

/**
 * Returns the driver name used by the device
 * @param[in] audioDevice the #LinphoneAudioDevice
 * @returns the name of the driver used by this audio device
 */
LINPHONE_PUBLIC const char *linphone_audio_device_get_driver_name(const LinphoneAudioDevice *audioDevice);

/**
 * Returns the capabilities of the device
 * @param[in] audioDevice the #LinphoneAudioDevice
 * @returns the capabilities of the audio device (RECORD, PLAY or both) as a bit mask
 */
LINPHONE_PUBLIC LinphoneAudioDeviceCapabilities linphone_audio_device_get_capabilities(const LinphoneAudioDevice *audioDevice);

/**
 * Returns the type of the device
 * @param[in] audioDevice the #LinphoneAudioDevice
 * @returns the type of the audio device (microphone, speaker, earpiece, bluetooth, etc...)
 */
LINPHONE_PUBLIC LinphoneAudioDeviceType linphone_audio_device_get_type(const LinphoneAudioDevice *audioDevice);

/**
 * Returns whether or not the audio device has the given capability
 * @param[in] audioDevice the #LinphoneAudioDevice
 * @param[in] capability the capability to check
 * @returns TRUE if the audio device has the capability, FALSE otherwise
 */
LINPHONE_PUBLIC  bool_t linphone_audio_device_has_capability(const LinphoneAudioDevice *audioDevice, const LinphoneAudioDeviceCapabilities capability);

/**
 * Takes a reference on a #LinphoneAudioDevice.
 */
LINPHONE_PUBLIC LinphoneAudioDevice *linphone_audio_device_ref(LinphoneAudioDevice *audioDevice);

/**
 * Releases a #LinphoneAudioDevice.
 */
LINPHONE_PUBLIC void linphone_audio_device_unref(LinphoneAudioDevice *audioDevice);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif