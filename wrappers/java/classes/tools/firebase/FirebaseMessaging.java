package org.linphone.core.tools.firebase;

/*
FirebaseMessaging.java
Copyright (C) 2017-2019 Belledonne Communications, Grenoble, France

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

import static android.content.Intent.ACTION_MAIN;

import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ServiceInfo;

import com.google.firebase.messaging.FirebaseMessagingService;
import com.google.firebase.messaging.RemoteMessage;

import org.linphone.core.tools.service.LinphoneService;
import org.linphone.core.tools.Log;

public class FirebaseMessaging extends FirebaseMessagingService {
    public FirebaseMessaging() {}

    @Override
    public void onNewToken(final String token) {
        if (LinphoneService.isReady()) {
            Log.i("FirebaseIdService", "[Push Notification] Refreshed token: " + token);
            LinphoneService.getCore().setPushNotificationToken(token);
        }
    }

    @Override
    public void onMessageReceived(RemoteMessage remoteMessage) {
        android.util.Log.i("FirebaseMessaging", "[Push Notification] Received");

        // Inspect services in package to get the class name of the Service that extends LinphoneService, assume first one
        String className = null;
        try {
            PackageInfo packageInfo = getPackageManager().getPackageInfo(getPackageName(), PackageManager.GET_SERVICES);
            ServiceInfo[] services = packageInfo.services;
            if (services.length > 0) {
                className = services[0].name;
            }
        } catch (Exception e) {
            android.util.Log.e("FirebaseMessaging", "Couldn't find Service class");
        }

        if (!LinphoneService.isReady()) {
            android.util.Log.i("FirebaseMessaging", "[Push Notification] Starting Service " + className);
            Intent intent = new Intent(ACTION_MAIN);
            intent.setClassName(this, className);
            intent.putExtra("PushNotification", true);
            startService(intent);
        } else {
            LinphoneService.getCore().ensureRegistered();
        }
    }
}
