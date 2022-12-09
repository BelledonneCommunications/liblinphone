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

import android.app.NotificationManager;
import android.content.Context;
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
			return DeviceUtils26.isSurfaceTextureReleased(surfaceTexture);
		}
		return false;
	}
	
	public static void logPreviousCrashesIfAny(Context context) {
		if (Version.sdkAboveOrEqual(Version.API31_ANDROID_12)) {
			DeviceUtils31.logPreviousCrashesIfAny(context);
		} else if (Version.sdkAboveOrEqual(Version.API30_ANDROID_11)) {
			DeviceUtils30.logPreviousCrashesIfAny(context);
		}
	}

	public static void vibrate(Vibrator vibrator) {
		if (Version.sdkAboveOrEqual(Version.API26_O_80)) {
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
}
