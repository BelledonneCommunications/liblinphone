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
import android.app.ApplicationExitInfo;
import android.app.usage.UsageStatsManager;
import android.content.Context;
import android.text.format.DateFormat;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.IOException;
import java.lang.IllegalStateException;
import java.util.Calendar;
import java.util.List;
import java.util.stream.Collectors;

import org.linphone.core.tools.Log;

public class DeviceUtils30 {
    public static void logPreviousCrashesIfAny(Context context) {
		Log.i("==== Fetching last five exit reasons if available ====");
		ActivityManager activityManager = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
		List<ApplicationExitInfo> exitInfos = activityManager.getHistoricalProcessExitReasons(null, 0, 5);

		for (ApplicationExitInfo exitInfo : exitInfos) {
			Log.i("==== Previous exit reason information dump ====");
			Log.i("REASON=", getReasonAsString(exitInfo.getReason()) + "[" + exitInfo.getStatus() + "]");
			Log.i("IMPORTANCE=", getImportanceAsString(exitInfo.getImportance()));
			Log.i("TIMESTAMP=", getHumanReadableDateAndTimeFromTimestamp(exitInfo.getTimestamp()));
			Log.i("DESCRIPTION=", exitInfo.getDescription());
			if (exitInfo.getReason() == ApplicationExitInfo.REASON_ANR) {
				try {
					InputStream inputStream = exitInfo.getTraceInputStream();
					if (inputStream != null) {
						BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(inputStream));
						String trace = bufferedReader.lines().collect(Collectors.joining("\n"));
						Log.w("TRACE=", trace);
						bufferedReader.close();
					} else {
						Log.w("[Device Utils 30] No input stream for exit info");
					}
				} catch (IOException ioe) {
					Log.e("[Device Utils 30] IO Exception while trying to get trace input stream: ", ioe);
				} catch (IllegalStateException ise) {
					Log.e("[Device Utils 30] Illegal State Exception while trying to get trace input stream: ", ise);
				} catch (Exception e) {
					Log.e("[Device Utils 30] Exception while trying to get trace input stream: ", e);
				}
			}
			Log.i("=========================================");
		}
	}

	public static String getImportanceAsString(int importance) {
		if (importance == ActivityManager.RunningAppProcessInfo.IMPORTANCE_FOREGROUND) {
			return "Foreground";
		} else if (importance == ActivityManager.RunningAppProcessInfo.IMPORTANCE_FOREGROUND_SERVICE) {
			return "Foreground Service";
		} else if (importance == ActivityManager.RunningAppProcessInfo.IMPORTANCE_TOP_SLEEPING) {
			return "Top sleeping";
		} else if (importance == ActivityManager.RunningAppProcessInfo.IMPORTANCE_VISIBLE) {
			return "Visible";
		} else if (importance == ActivityManager.RunningAppProcessInfo.IMPORTANCE_PERCEPTIBLE) {
			return "Perceptible";
		} else if (importance == ActivityManager.RunningAppProcessInfo.IMPORTANCE_CANT_SAVE_STATE) {
			return "Can't save state";
		} else if (importance == ActivityManager.RunningAppProcessInfo.IMPORTANCE_SERVICE) {
			return "Service";
		} else if (importance == ActivityManager.RunningAppProcessInfo.IMPORTANCE_CACHED) {
			return "Cached";
		} else if (importance == ActivityManager.RunningAppProcessInfo.IMPORTANCE_GONE) {
			return "Gone";
		} else if (importance == ActivityManager.RunningAppProcessInfo.IMPORTANCE_TOP_SLEEPING) {
			return "Top sleeping";
		}
		return "Unexpected: " + importance;
	}

	public static String getReasonAsString(int reason) {
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

	public static String getHumanReadableDateAndTimeFromTimestamp(long timestamp) {
		Calendar cal = Calendar.getInstance();
		cal.setTimeInMillis(timestamp);
		return DateFormat.format("dd-MM-yyyy HH:mm:ss", cal).toString();
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
			case UsageStatsManager.STANDBY_BUCKET_RESTRICTED:
				return "STANDBY_BUCKET_RESTRICTED";
		}
		return null;
	}
}
