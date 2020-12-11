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

package org.linphone.core.tools.compatibility;

import android.app.ActivityManager;
import android.app.ApplicationExitInfo;
import android.content.Context;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.IOException;
import java.util.List;
import java.util.stream.Collectors;

import org.linphone.core.tools.Log;

public class DeviceUtils30 {
    public static void logPreviousCrashesIfAny(Context context) {
		ActivityManager activityManager = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
		List<ApplicationExitInfo> exitInfos = activityManager.getHistoricalProcessExitReasons(null, 0, 0);

		for (ApplicationExitInfo exitInfo : exitInfos) {
			Log.w("==== Previous exit reason information dump ====");
			Log.w("REASON=", getReasonAsString(exitInfo.getReason()));
			Log.w("TIMESTAMP=", exitInfo.getTimestamp());
			Log.w("DESCRIPTION=", exitInfo.getDescription());
			if (exitInfo.getReason() == ApplicationExitInfo.REASON_ANR) {
				try {
					String trace = new BufferedReader(new InputStreamReader(exitInfo.getTraceInputStream())).lines().collect(Collectors.joining("\n"));
					Log.w("TRACE=", trace);
				} catch (IOException e) {
					Log.e("Exception while trying to get trace input stream: ", e);
				}
			}
			Log.w("=========================================");
		}
	}

	private static String getReasonAsString(int reason) {
		if (reason == ApplicationExitInfo.REASON_UNKNOWN) {
			return "Unknown";
		} else if (reason == ApplicationExitInfo.REASON_USER_REQUESTED) {
			return "User requested";
		} else if (reason == ApplicationExitInfo.REASON_USER_STOPPED) {
			return "User stopped";
		} else if (reason == ApplicationExitInfo.REASON_SIGNALED) {
			return "Signaled";
		} else if (reason == ApplicationExitInfo.REASON_PERMISSION_CHANGE) {
			return "Permission changed";
		} else if (reason == ApplicationExitInfo.REASON_OTHER) {
			return "Other";
		} else if (reason == ApplicationExitInfo.REASON_LOW_MEMORY) {
			return "Low memory";
		} else if (reason == ApplicationExitInfo.REASON_INITIALIZATION_FAILURE) {
			return "Initialization failure";
		} else if (reason == ApplicationExitInfo.REASON_EXIT_SELF) {
			return "Self stop";
		} else if (reason == ApplicationExitInfo.REASON_EXCESSIVE_RESOURCE_USAGE) {
			return "Excessive resource usage";
		} else if (reason == ApplicationExitInfo.REASON_DEPENDENCY_DIED) {
			return "Dependency died";
		} else if (reason == ApplicationExitInfo.REASON_CRASH_NATIVE) {
			return "Native crash";
		} else if (reason == ApplicationExitInfo.REASON_CRASH) {
			return "Crash";
		} else if (reason == ApplicationExitInfo.REASON_ANR) {
			return "ANR";
		}
		return "Unexpected: " + reason;
	}
}
