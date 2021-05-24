/*
 * Copyright (c) 2010-2021 Belledonne Communications SARL.
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

package org.linphone.core.tools.compatibility;

import android.app.ActivityManager;
import android.app.ApplicationExitInfo;
import android.content.Context;
import android.text.format.DateFormat;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.IOException;
import java.util.Calendar;
import java.util.List;
import java.util.stream.Collectors;

import org.linphone.core.tools.Log;

public class DeviceUtils31 {
    public static void logPreviousCrashesIfAny(Context context) {
		ActivityManager activityManager = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
		List<ApplicationExitInfo> exitInfos = activityManager.getHistoricalProcessExitReasons(null, 0, 5);

		for (ApplicationExitInfo exitInfo : exitInfos) {
			Log.i("==== Previous exit reason information dump ====");
			Log.i("REASON=", DeviceUtils30.getReasonAsString(exitInfo.getReason()));
			Log.i("TIMESTAMP=", DeviceUtils30.getHumanReadableDateAndTimeFromTimestamp(exitInfo.getTimestamp()));
			Log.i("DESCRIPTION=", exitInfo.getDescription());
			if (exitInfo.getReason() == ApplicationExitInfo.REASON_ANR || exitInfo.getReason() == ApplicationExitInfo.REASON_CRASH_NATIVE) {
				try {
					String trace = new BufferedReader(new InputStreamReader(exitInfo.getTraceInputStream())).lines().collect(Collectors.joining("\n"));
					Log.w("TRACE=", trace);
				} catch (IOException e) {
					Log.e("Exception while trying to get trace input stream: ", e);
				}
			}
			Log.i("=========================================");
		}
	}
}
