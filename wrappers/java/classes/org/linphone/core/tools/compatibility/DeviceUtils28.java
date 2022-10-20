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

import android.app.ActivityManager;
import android.app.usage.UsageStatsManager;
import android.content.Context;
import android.media.AudioAttributes;
import android.media.Ringtone;

import org.linphone.core.tools.Log;

public class DeviceUtils28 {
	public static boolean isAppUserRestricted(Context context) {
		ActivityManager activityManager =
				(ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
		return activityManager.isBackgroundRestricted();
	}

	public static int getAppStandbyBucket(Context context) {
		UsageStatsManager usageStatsManager =
				(UsageStatsManager) context.getSystemService(Context.USAGE_STATS_SERVICE);
		return usageStatsManager.getAppStandbyBucket();
	}

	public static String getAppStandbyBucketNameFromValue(int bucket) {
		switch (bucket) {
			case UsageStatsManager.STANDBY_BUCKET_ACTIVE:
				return "STANDBY_BUCKET_ACTIVE";
			case UsageStatsManager.STANDBY_BUCKET_FREQUENT:
				return "STANDBY_BUCKET_FREQUENT";
			case UsageStatsManager.STANDBY_BUCKET_RARE:
				return "STANDBY_BUCKET_RARE";
			case UsageStatsManager.STANDBY_BUCKET_WORKING_SET:
				return "STANDBY_BUCKET_WORKING_SET";
		}
		return null;
	}

	public static void playRingtone(Ringtone ringtone, AudioAttributes audioAttrs) {
		ringtone.setAudioAttributes(audioAttrs);
		ringtone.setLooping(true);
		ringtone.play();
		Log.i("[Audio Helper] Ringtone ringing started (looping)");
	}
}
