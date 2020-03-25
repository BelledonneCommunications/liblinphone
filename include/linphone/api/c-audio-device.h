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