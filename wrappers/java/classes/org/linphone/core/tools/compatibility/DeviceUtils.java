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
import android.app.NotificationManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.SurfaceTexture;
import android.media.AudioAttributes;
import android.media.Ringtone;
import android.provider.ContactsContract;
import android.os.Vibrator;

import org.linphone.core.Address;
import org.linphone.core.tools.Log;

import org.linphone.mediastream.Version;

import java.util.Map;

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
		if (Version.sdkAboveOrEqual(Version.API30_ANDROID_11)) {
			return DeviceUtils30.getAppStandbyBucketNameFromValue(bucket);
		}
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
			return DeviceUtils26.isSurfaceTextureReleased(surfaceTexture);
		}
		return false;
	}
	
	public static void logPreviousCrashesIfAny(Context context) {
		if (Version.sdkAboveOrEqual(Version.API31_ANDROID_12)) {
			boolean protobufDependencyFound = false;
			try {
				String className = "com.google.protobuf.GeneratedMessageLite";
            	Class message = Class.forName(className);
				protobufDependencyFound = true;
			} catch (ClassNotFoundException e) {
				Log.w("[Device Utils] Couldn't find class [com.google.protobuf.GeneratedMessageLite]");
			} catch (Exception e) {
				Log.w("[Device Utils] Couldn't load protobuf classes: " + e);
			}

			if (!protobufDependencyFound) {
				Log.w("[Device Utils] Native crash tombstone can't be obtained, is app missing [implementation \"com.google.protobuf:protobuf-javalite:3.22.3\"] dependency?");
			}
			DeviceUtils31.logPreviousCrashesIfAny(context, protobufDependencyFound);
		} else if (Version.sdkAboveOrEqual(Version.API30_ANDROID_11)) {
			DeviceUtils30.logPreviousCrashesIfAny(context);
		}
	}

	public static void vibrate(Vibrator vibrator) {
		if (Version.sdkAboveOrEqual(Version.API33_ANDROID_13_TIRAMISU)) {
			DeviceUtils33.vibrate(vibrator);
		} else if (Version.sdkAboveOrEqual(Version.API26_O_80)) {
			DeviceUtils26.vibrate(vibrator);
		} else {
			DeviceUtils23.vibrate(vibrator);
		}
	}

	public static int getPerformanceClass() {
		if (Version.sdkAboveOrEqual(Version.API31_ANDROID_12)) {
			return DeviceUtils31.getPerformanceClass();
		}
		return -1;
	}

	public static boolean isBluetoothConnectPermissionGranted(Context context) {
		if (Version.sdkAboveOrEqual(Version.API31_ANDROID_12)) {
			return DeviceUtils31.isBluetoothConnectPermissionGranted(context);
		}
		return true;
	}

	public static void playRingtone(Ringtone ringtone, AudioAttributes audioAttrs) {
		if (Version.sdkAboveOrEqual(Version.API28_PIE_90)) {
			DeviceUtils28.playRingtone(ringtone, audioAttrs);
		} else {
			DeviceUtils23.playRingtone(ringtone, audioAttrs);
		}
	}

	public static boolean checkIfDoNotDisturbAllowsAllCalls(Context context) {
		try {
			NotificationManager notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
			NotificationManager.Policy policy = notificationManager.getNotificationPolicy();
			boolean senderCheck = policy.priorityCallSenders == NotificationManager.Policy.PRIORITY_SENDERS_ANY;
			boolean categoryCheck = policy.priorityCategories == NotificationManager.Policy.PRIORITY_CATEGORY_CALLS;
			Log.i("[Device Utils] Call sender check is [" + senderCheck + "], call caterogy check is [" + categoryCheck + "]");
			return senderCheck || categoryCheck;
		} catch (SecurityException se) {
			Log.e("[Device Utils] Can't check notification policy: " + se);
		}
		return false;
	}

	public static boolean checkIfDoNotDisturbAllowsKnownContacts(Context context) {
		try {
			NotificationManager notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
			NotificationManager.Policy policy = notificationManager.getNotificationPolicy();
			return policy.priorityCallSenders == NotificationManager.Policy.PRIORITY_SENDERS_CONTACTS;
		} catch (SecurityException se) {
			Log.e("[Device Utils] Can't check notification policy: " + se);
		}
		return false;
	}

	public static boolean checkIfIsKnownContact(Context context, Address caller) {
		if (caller == null) {
			return false;
		}

		if (context.checkSelfPermission(android.Manifest.permission.READ_CONTACTS) != PackageManager.PERMISSION_GRANTED) {
			Log.e("[Device Utils] Can't check for favorite contact, permission hasn't been granted yet");
			return false;
		}
		
		String number = caller.getUsername();
		String address = caller.asStringUriOnly();
		try {
			Cursor cursor = context.getContentResolver().query(
				ContactsContract.Data.CONTENT_URI,
				new String[] { },
				ContactsContract.CommonDataKinds.Phone.NORMALIZED_NUMBER + " LIKE ? OR " + ContactsContract.CommonDataKinds.SipAddress.SIP_ADDRESS + " LIKE ?",
				new String[] { number, address },
				null
			);

			if (cursor != null) {
				Log.i("[Device Utils] Cursor isn't null, at least one contact in native addressbook matches");
				cursor.close();
				return true;
			}
		} catch (IllegalArgumentException e) {
			Log.e("[Device Utils] Failed to check if username / SIP address is part of a favorite contact: ", e);
		}
		
		return false;
	}

	public static boolean checkIfDoNotDisturbAllowsExceptionForFavoriteContacts(Context context) {
		try {
			NotificationManager notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
			NotificationManager.Policy policy = notificationManager.getNotificationPolicy();
			return policy.priorityCallSenders == NotificationManager.Policy.PRIORITY_SENDERS_STARRED;
		} catch (SecurityException se) {
			Log.e("[Device Utils] Can't check notification policy: " + se);
		}
		return false;
	}

	public static boolean checkIfIsFavoriteContact(Context context, Address caller) {
		if (caller == null) {
			return false;
		}

		if (context.checkSelfPermission(android.Manifest.permission.READ_CONTACTS) != PackageManager.PERMISSION_GRANTED) {
			Log.e("[Device Utils] Can't check for favorite contact, permission hasn't been granted yet");
			return false;
		}
		
		String number = caller.getUsername();
		String address = caller.asStringUriOnly();
		try {
			Cursor cursor = context.getContentResolver().query(
				ContactsContract.Data.CONTENT_URI,
				new String[] { ContactsContract.CommonDataKinds.Phone.STARRED },
				ContactsContract.CommonDataKinds.Phone.NORMALIZED_NUMBER + " LIKE ? OR " + ContactsContract.CommonDataKinds.SipAddress.SIP_ADDRESS + " LIKE ?",
				new String[] { number, address },
				null
			);

			while (cursor != null && cursor.moveToNext()) {
				int favorite = cursor.getInt(cursor.getColumnIndexOrThrow(ContactsContract.CommonDataKinds.Phone.STARRED));
				if (favorite == 1) {
					Log.i("[Device Utils] Found phone number or SIP address in favorite contact");
					cursor.close();
					return true;
				}
			}

			if (cursor != null) {
				cursor.close();
			}
		} catch (IllegalArgumentException e) {
			Log.e("[Device Utils] Failed to check if username / SIP address is part of a favorite contact: ", e);
		}
		
		return false;
	}

	public static String getStringOrDefaultFromMap(Map<String, String> map, String key, String defaultValue) {
		// map.getOrDefault(key, defaultValue) isn't available on Android 23 and lower...
		if (Version.sdkAboveOrEqual(Version.API24_NOUGAT_70)) {
			return DeviceUtils24.getStringOrDefaultFromMap(map, key, defaultValue);
		}
		return DeviceUtils23.getStringOrDefaultFromMap(map, key, defaultValue);
	}

	public static void startForegroundService(Context context, Intent intent) {
		if (Version.sdkAboveOrEqual(Version.API33_ANDROID_13_TIRAMISU)) {
			DeviceUtils33.startForegroundService(context, intent);
		} else if (Version.sdkAboveOrEqual(Version.API31_ANDROID_12)) {
			DeviceUtils31.startForegroundService(context, intent);
		} else if (Version.sdkAboveOrEqual(Version.API26_O_80)) {
			DeviceUtils26.startForegroundService(context, intent);
		} else {
			DeviceUtils23.startForegroundService(context, intent);
		}
	}

	public static void startCallForegroundService(Service service, int notifId, Notification notif, boolean isVideoCall) {
		if (Version.sdkAboveOrEqual(Version.API34_ANDROID_14_UPSIDE_DOWN_CAKE)) {
			DeviceUtils34.startCallForegroundService(service, notifId, notif, isVideoCall);
		} else if (Version.sdkAboveOrEqual(Version.API31_ANDROID_12)) {
			DeviceUtils31.startForegroundService(service, notifId, notif);
		} else {
			DeviceUtils23.startForegroundService(service, notifId, notif);
		}
	}

	public static void startDataSyncForegroundService(Service service, int notifId, Notification notif) {
		if (Version.sdkAboveOrEqual(Version.API34_ANDROID_14_UPSIDE_DOWN_CAKE)) {
			DeviceUtils34.startDataSyncForegroundService(service, notifId, notif);
		} else if (Version.sdkAboveOrEqual(Version.API31_ANDROID_12)) {
			DeviceUtils31.startForegroundService(service, notifId, notif);
		} else {
			DeviceUtils23.startForegroundService(service, notifId, notif);
		}
	}
}
