/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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
import android.content.pm.PackageManager;
import android.os.Vibrator;
import android.os.VibrationAttributes;
import android.os.VibrationEffect;

import org.linphone.core.tools.Log;

public class DeviceUtils33 {
    public static void startForegroundService(Context context, Intent intent) {
		try {
			if (context.checkSelfPermission("android.permission.POST_NOTIFICATIONS") == PackageManager.PERMISSION_GRANTED) {
				Log.i("[Device Utils 33] POST_NOTIFICATIONS permission granted, start service as foreground (will require foreground service notification or process will crash due to ForegroundServiceDidNotStartInTimeException)");
				context.startForegroundService(intent);
			} else {
				Log.e("[Device Utils 33] POST_NOTIFICATIONS permission not granted, can't start service as foreground!");
				context.startService(intent);
			}
		} catch (Exception e) {
			Log.e("[Device Utils 33] Can't start service with promise it will run as foreground: ", e);
		}
    }

	public static void vibrate(Vibrator vibrator) {
		long[] timings = {0, 1000, 1000};
		int[] amplitudes = {0, VibrationEffect.DEFAULT_AMPLITUDE, 0};
		VibrationEffect effect = VibrationEffect.createWaveform(timings, amplitudes, 1);
		vibrator.vibrate(effect, VibrationAttributes.createForUsage(VibrationAttributes.USAGE_RINGTONE));
	}
}
