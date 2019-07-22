/*
DeviceUtils28.java
Copyright (C) 2019 Belledonne Communications, Grenoble, France

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

package org.linphone.core.tools;

import android.app.ActivityManager;
import android.app.usage.UsageStatsManager;
import android.content.Context;

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
}
