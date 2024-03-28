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
package org.linphone.core.tools.firebase;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;

import com.google.firebase.messaging.FirebaseMessagingService;
import com.google.firebase.messaging.RemoteMessage;

import org.linphone.core.Core;
import org.linphone.core.GlobalState;
import org.linphone.core.tools.Log;
import org.linphone.core.tools.compatibility.DeviceUtils;
import org.linphone.core.tools.service.CoreManager;
import org.linphone.core.tools.service.AndroidDispatcher;

import java.lang.StringBuilder;
import java.util.List;
import java.util.Map;

import org.json.JSONObject;

/**
 * Firebase cloud messaging service implementation used to received push notification.
 */
public class FirebaseMessaging extends FirebaseMessagingService {
    public FirebaseMessaging() {
    }

    @Override
    public void onNewToken(final String token) {
        android.util.Log.i("FirebaseIdService", "[Push Notification] Refreshed token: " + token);
        if (CoreManager.isReady()) {
            Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    CoreManager.instance().setPushToken(token);
                }
            };
            CoreManager.instance().dispatchOnCoreThread(runnable);
        }
    }

    @Override
    public void onMessageReceived(RemoteMessage remoteMessage) {
        android.util.Log.i("FirebaseMessaging", "[Push Notification] Received");
        Runnable pushRunnable = new Runnable() {
            @Override
            public void run() {
                onPushReceived(remoteMessage);
            }
        };
        AndroidDispatcher.dispatchOnUIThread(pushRunnable);
        
    }

    private void onPushReceived(RemoteMessage remoteMessage) {
        if (!CoreManager.isReady()) {
            storePushRemoteMessage(remoteMessage);
            notifyAppPushReceivedWithoutCoreAvailable();
        } else {
            Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    Log.i("[Push Notification] Received: " + remoteMessageToString(remoteMessage));
                    Core core = CoreManager.instance().getCore();
                    if (core != null && core.getGlobalState() == GlobalState.On) {
                        Map<String, String> data = remoteMessage.getData();
                        String callId = DeviceUtils.getStringOrDefaultFromMap(data, "call-id", "");                    
                        String payload = new JSONObject(data).toString();
                        Log.i("[Push Notification] Notifying Core we have received a push for Call-ID [" + callId + "]");
                        CoreManager.instance().processPushNotification(callId, payload, false);
                    } else {
                        if (core == null) {
                            Log.w("[Push Notification] No Core found, notifying application directly");
                        } else {
                            Log.w("[Push Notification] Core was found but it isn't in GlobalState On yet, notifying application directly");
                        }
                        Runnable pushRunnable = new Runnable() {
                            @Override
                            public void run() {
                                storePushRemoteMessage(remoteMessage);
                                notifyAppPushReceivedWithoutCoreAvailable();
                            }
                        };
                        AndroidDispatcher.dispatchOnUIThread(pushRunnable);
                    }
                }
            };
            CoreManager.instance().dispatchOnCoreThread(runnable);
        }
    }

    private void storePushRemoteMessage(RemoteMessage remoteMessage) {
        Context context = getApplicationContext();
        SharedPreferences sharedPref = context.getSharedPreferences("push_notification_storage", Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPref.edit();

        Map<String, String> data = remoteMessage.getData();
        String callId = DeviceUtils.getStringOrDefaultFromMap(data, "call-id", "");      
        editor.putString("call-id", callId);
        String payload = new JSONObject(data).toString();
        editor.putString("payload", payload);
        editor.commit();
        android.util.Log.i("FirebaseMessaging", "[Push Notification] Push information stored for Call-ID [" + callId + "]");
    }

    private void notifyAppPushReceivedWithoutCoreAvailable() {
        Intent intent = new Intent();
        String action = "org.linphone.core.action.PUSH_RECEIVED";
        intent.setAction(action);

        String appPackageName = getPackageName();
        boolean found = false;
        android.util.Log.i("FirebaseMessaging", "[Push Notification] Looking for an application with package name [" + appPackageName + "] that has registered a BroadcastReceiver for an Intent with action [" + action + "]");

        PackageManager pm = getPackageManager();
        List<ResolveInfo> matches = pm.queryBroadcastReceivers(intent, 0);
        for (ResolveInfo resolveInfo : matches) {
            String packageName = resolveInfo.activityInfo.applicationInfo.packageName;
            if (packageName.equals(appPackageName)) {
                android.util.Log.i("FirebaseMessaging", "[Push Notification] Notifying component [" + resolveInfo.activityInfo.name + "] using a Broadcast Intent with action [" + action + "]");
                Intent explicit = new Intent(intent);
                ComponentName cn = new ComponentName(packageName, resolveInfo.activityInfo.name);
                explicit.setComponent(cn);
                sendBroadcast(explicit);
                found = true;
                break;
            }
        }

        if (!found) {
            android.util.Log.e("FirebaseMessaging", "[Push Notification] No matching BroadcastReceiver found for an Intent with action [" + action + "]!");
        }
    }

    private String remoteMessageToString(RemoteMessage remoteMessage) {
        StringBuilder builder = new StringBuilder();
        builder.append("From [");
        builder.append(remoteMessage.getFrom());
        builder.append("], Message Id [");
        builder.append(remoteMessage.getMessageId());
        builder.append("], TTL [");
        builder.append(remoteMessage.getTtl());
        builder.append("], Original Priority [");
        builder.append(remoteMessage.getOriginalPriority());
        builder.append("], Received Priority [");
        builder.append(remoteMessage.getPriority());
        builder.append("], Sent Time [");
        builder.append(remoteMessage.getSentTime());
        builder.append("], Data [");
        builder.append(new JSONObject(remoteMessage.getData()).toString());
        builder.append("]");
        return builder.toString();
    }
}
