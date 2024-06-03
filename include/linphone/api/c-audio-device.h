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
 * @param audio_device the #LinphoneAudioDevice. @notnil
 * @return the id of the audio device. @notnil
 */
LINPHONE_PUBLIC const char *linphone_audio_device_get_id(const LinphoneAudioDevice *audio_device);

/**
 * Returns the name of the audio device
 * @param audio_device the #LinphoneAudioDevice. @notnil
 * @return the name of the audio device. @notnil
 */
LINPHONE_PUBLIC const char *linphone_audio_device_get_device_name(const LinphoneAudioDevice *audio_device);

/**
 * Returns the driver name used by the device
 * @param audio_device the #LinphoneAudioDevice. @notnil
 * @returns the name of the driver used by this audio device. @notnil
 */
LINPHONE_PUBLIC const char *linphone_audio_device_get_driver_name(const LinphoneAudioDevice *audio_device);

/**
 * Returns the capabilities of the device
 * @param audio_device the #LinphoneAudioDevice. @notnil
 * @returns the #LinphoneAudioDeviceCapabilities of the audio device (RECORD, PLAY or both) as a bit mask
 */
LINPHONE_PUBLIC LinphoneAudioDeviceCapabilities
linphone_audio_device_get_capabilities(const LinphoneAudioDevice *audio_device);

/**
 * Returns the type of the device
 * @param audio_device the #LinphoneAudioDevice. @notnil
 * @returns the #LinphoneAudioDeviceType of the audio device (microphone, speaker, earpiece, bluetooth, etc...)
 */
LINPHONE_PUBLIC LinphoneAudioDeviceType linphone_audio_device_get_type(const LinphoneAudioDevice *audio_device);

/**
 * Returns whether or not the audio device has the given capability
 * @param audio_device the #LinphoneAudioDevice. @notnil
 * @param capability the #LinphoneAudioDeviceCapabilities to check
 * @returns TRUE if the audio device has the capability, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_audio_device_has_capability(const LinphoneAudioDevice *audio_device,
                                                            const LinphoneAudioDeviceCapabilities capability);

/**
 * Returns whether the audio device automatically follows the system's audio routing policy.
 * This capability is available on some system (typically iOS) and might be convenient to simply specify
 * liblinphone to let the system decide about which audio route is being used to handle a call.
 * The actual #LinphoneAudioDeviceType may be unknown at some point, typically when no calls are running, otherwise
 * it is reflected to be the actual system's audio route.
 * @param audio_device the #LinphoneAudioDevice. @notnil
 * @returns TRUE if the audio device automatically follows the system audio routing policy.
 */
LINPHONE_PUBLIC bool_t linphone_audio_device_get_follows_system_routing_policy(const LinphoneAudioDevice *audio_device);

/**
 * Takes a reference on a #LinphoneAudioDevice.
 * @param audio_device the #LinphoneAudioDevice. @notnil
 * @return the same #LinphoneAudioDevice object
 */
LINPHONE_PUBLIC LinphoneAudioDevice *linphone_audio_device_ref(LinphoneAudioDevice *audio_device);

/**
 * Releases a #LinphoneAudioDevice.
 * @param audio_device the #LinphoneAudioDevice. @notnil
 */
LINPHONE_PUBLIC void linphone_audio_device_unref(LinphoneAudioDevice *audio_device);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
