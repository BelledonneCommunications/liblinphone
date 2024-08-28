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
import android.app.Notification;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.text.format.DateFormat;
import android.Manifest;

import com.android.server.os.LinphoneTombstoneProtos.Tombstone;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.IOException;
import java.lang.IllegalStateException;
import java.util.Calendar;
import java.util.List;
import java.util.stream.Collectors;

import org.linphone.core.tools.Log;

public class DeviceUtils31 {
    public static void logPreviousCrashesIfAny(Context context, boolean printNativeCrashTombstone) {
		Log.i("==== Fetching last five exit reasons if available ====");
		ActivityManager activityManager = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
		List<ApplicationExitInfo> exitInfos = activityManager.getHistoricalProcessExitReasons(null, 0, 5);

		for (ApplicationExitInfo exitInfo : exitInfos) {
			Log.i("==== Previous exit reason information dump ====");
			Log.i("REASON=", DeviceUtils30.getReasonAsString(exitInfo.getReason()) + "[" + exitInfo.getStatus() + "]");
			Log.i("IMPORTANCE=", DeviceUtils30.getImportanceAsString(exitInfo.getImportance()));
			Log.i("TIMESTAMP=", DeviceUtils30.getHumanReadableDateAndTimeFromTimestamp(exitInfo.getTimestamp()));
			Log.i("DESCRIPTION=", exitInfo.getDescription());
			if (exitInfo.getReason() == ApplicationExitInfo.REASON_ANR || exitInfo.getReason() == ApplicationExitInfo.REASON_CRASH_NATIVE) {
				try {
					InputStream inputStream = exitInfo.getTraceInputStream();
					if (inputStream != null) {
						if (exitInfo.getReason() == ApplicationExitInfo.REASON_CRASH_NATIVE && printNativeCrashTombstone) {
							try {
								Tombstone tombstone = Tombstone.parseFrom(inputStream);
								Log.w("TOMBSTONE=", tombstone.getAbortMessage());
							} catch (Exception e) {
								Log.e("Failed to obtain tombstone, is app missing [implementation \"com.google.protobuf:protobuf-javalite:3.22.3\"] dependency?");
							}
						} else if (exitInfo.getReason() == ApplicationExitInfo.REASON_ANR) {
							BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(inputStream));
							String trace = bufferedReader.lines().collect(Collectors.joining("\n"));
							Log.w("TRACE=", trace);
							bufferedReader.close();
						}
					} else {
						Log.w("[Device Utils 31] No input stream for exit info");
					}
				} catch (IOException ioe) {
					Log.e("[Device Utils 31] IO Exception while trying to get trace input stream: ", ioe);
				} catch (IllegalStateException ise) {
					Log.e("[Device Utils 31] Illegal State Exception while trying to get trace input stream: ", ise);
				} catch (Exception e) {
					Log.e("[Device Utils 31] Exception while trying to get trace input stream: ", e);
				}
			}
			Log.i("=========================================");
		}
	}

	public static int getPerformanceClass() {
		return android.os.Build.VERSION.MEDIA_PERFORMANCE_CLASS;
	}

	public static boolean isBluetoothConnectPermissionGranted(Context context) {
		return context.checkSelfPermission(Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED;
	}

    public static void startForegroundService(Context context, Intent intent) {
		try {
			context.startForegroundService(intent);
		} catch (Exception e) {
			Log.e("[Device Utils 31] Can't start service with promise it will run as foreground: ", e);
		}
    }

	public static void startForegroundService(Service service, int notifId, Notification notif) {
		try {
			service.startForeground(notifId, notif);
		} catch (Exception e) {
			Log.e("[Device Utils 31] Can't start service as foreground: ", e);
		}
	}
}
