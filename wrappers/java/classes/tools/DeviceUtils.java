/*
DeviceUtils.java
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

import android.content.Context;
import android.graphics.SurfaceTexture;
import org.linphone.mediastream.Version;

public class DeviceUtils {
    public static boolean isAppUserRestricted(Context context) {
        if (Version.sdkAboveOrEqual(Version.API28_PIE_90)) {
            return DeviceUtils28.isAppUserRestricted(context);
        }
        return false;
    }

    public static int getAppStandbyBucket(Context context) {
        if (Version.sdkAboveOrEqual(Version.API28_PIE_90)) {
            return DeviceUtils28.getAppStandbyBucket(context);
        }
        return 0;
    }

    public static String getAppStandbyBucketNameFromValue(int bucket) {
        if (Version.sdkAboveOrEqual(Version.API28_PIE_90)) {
            return DeviceUtils28.getAppStandbyBucketNameFromValue(bucket);
        }
        return null;
    }

    public static boolean isAppBatteryOptimizationEnabled(Context context) {
        if (Version.sdkAboveOrEqual(Version.API23_MARSHMALLOW_60)) {
            return DeviceUtils23.isAppBatteryOptimizationEnabled(context);
        }
        return false;
    }

    public static boolean isSurfaceTextureReleased(SurfaceTexture surfaceTexture) {
        if (Version.sdkAboveOrEqual(Version.API26_O_80)) {
            return DeviceUtils26.isSurfaceReleased(surfaceTexture);
        }
        return false;
    }
}
