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

import android.app.Notification;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.media.AudioAttributes;
import android.media.Ringtone;
import android.os.PowerManager;
import android.os.Vibrator;

import java.util.Map;

import org.linphone.core.tools.Log;

public class DeviceUtils23 {
	public static boolean isAppBatteryOptimizationEnabled(Context context) {
		PowerManager powerManager = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
		boolean ignoringBatteryOptimizations = powerManager.isIgnoringBatteryOptimizations(context.getPackageName());
		Log.i("[Platform Helper] Is app in device battery optimization whitelist: " + ignoringBatteryOptimizations);
		return !ignoringBatteryOptimizations;
	}

	public static void vibrate(Vibrator vibrator) {
		long[] pattern = {0, 1000, 1000};
		vibrator.vibrate(pattern, 1);
	}

	public static void playRingtone(Ringtone ringtone, AudioAttributes audioAttrs) {
		ringtone.setAudioAttributes(audioAttrs);
		ringtone.play();
		Log.i("[Audio Helper] Ringtone ringing started");
	}
	public static String getStringOrDefaultFromMap(Map<String, String> map, String key, String defaultValue) {
		//return map.getOrDefault(key, defaultValue);
		if (map.containsKey(key)) {
			String result = map.get(key);
			if (result != null) {
				return result;
			}
		}
		return defaultValue;
	}

	public static void startForegroundService(Service service, int notifId, Notification notif) {
		service.startForeground(notifId, notif);
	}

    public static void startForegroundService(Context context, Intent intent) {
        context.startService(intent);
    }
}
