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

package org.linphone.core.tools.compatibility;

import android.content.Context;
import android.content.Intent;
import android.graphics.SurfaceTexture;
import android.media.AudioAttributes;
import android.os.Vibrator;
import android.os.VibrationEffect;

public class DeviceUtils26 {
    public static boolean isSurfaceTextureReleased(SurfaceTexture surfaceTexture) {
        return surfaceTexture.isReleased();
    }

	public static void vibrate(Vibrator vibrator) {
		long[] timings = {0, 1000, 1000};
        int[] amplitudes = {0, VibrationEffect.DEFAULT_AMPLITUDE, 0};
        VibrationEffect effect = VibrationEffect.createWaveform(timings, amplitudes, 1);
        AudioAttributes audioAttrs =
                new AudioAttributes.Builder()
                        .setUsage(AudioAttributes.USAGE_NOTIFICATION_RINGTONE)
                        .build();
        vibrator.vibrate(effect, audioAttrs);
	}

    public static void startForegroundService(Context context, Intent intent) {
        context.startForegroundService(intent);
    }
}
