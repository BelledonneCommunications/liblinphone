/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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
import android.content.pm.ServiceInfo;

import org.linphone.core.tools.Log;

public class DeviceUtils34 {
    public static void startCallForegroundService(Service service, int notifId, Notification notif, boolean isVideoCall) {
		try {
			int mask = ServiceInfo.FOREGROUND_SERVICE_TYPE_PHONE_CALL | ServiceInfo.FOREGROUND_SERVICE_TYPE_MICROPHONE;
			if (isVideoCall) {
				mask |= ServiceInfo.FOREGROUND_SERVICE_TYPE_CAMERA;
			}
			service.startForeground(notifId, notif, mask);
		} catch (Exception e) {
			Log.e("[Device Utils 34] Can't start service as foreground: ", e);
		}
	}

	public static void startDataSyncForegroundService(Service service, int notifId, Notification notif) {
		try {
			service.startForeground(
				notifId,
				notif,
				ServiceInfo.FOREGROUND_SERVICE_TYPE_DATA_SYNC
			);
		} catch (Exception e) {
			Log.e("[Device Utils 34] Can't start service as foreground: ", e);
		}
	}
}
