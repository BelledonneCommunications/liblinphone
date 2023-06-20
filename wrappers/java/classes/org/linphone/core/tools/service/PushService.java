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

package org.linphone.core.tools.service;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.IBinder;

import androidx.annotation.RequiresApi;
import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;

import java.util.Timer;
import java.util.TimerTask;

import org.linphone.core.Factory;
import org.linphone.core.tools.Log;
import org.linphone.core.tools.compatibility.DeviceUtils;
import org.linphone.mediastream.Version;

/**
 * This service is used briefly as a foreground service when a push notification is being received to ensure
 * the network will be opened up so we can receive the even causing the push notification (call or message).
 * This is required when data saver is ON, network is metered and app in background.
 */
public class PushService extends Service {
    protected static final int SERVICE_NOTIF_ID = 2;
    protected static final String SERVICE_NOTIFICATION_CHANNEL_ID = "org_linphone_core_push_service_notification_channel";
    protected static final String SERVICE_NOTIFICATION_CHANNEL_NAME = "Linphone Core Push Service";
    protected static final String SERVICE_NOTIFICATION_CHANNEL_DESC = "Push notification received, waiting for call/chat message to be received by SIP channel";
    protected static final String SERVICE_NOTIFICATION_TITLE = "Linphone Core Push Service";
    protected static final String SERVICE_NOTIFICATION_CONTENT = "Push notification received, waiting for call/chat message to be received by SIP channel";

    protected Notification mServiceNotification = null;

    private Timer mTimer;

    @Override
    public void onCreate() {
        super.onCreate();

        // No-op, just to ensure libraries have been loaded and thus prevent crash in log below 
        // if service has been started directly by Android (that can happen...)
        Factory.instance();

        if (Version.sdkAboveOrEqual(Version.API26_O_80)) {
            createServiceNotificationChannel();
        }
        mTimer = new Timer("Push service self-destroy scheduler");

        Log.i("[Push Service] Created");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);

        Log.i("[Push Service] Started");
        startForeground();
        if (CoreManager.isReady()) {
            CoreManager.instance().healNetwork();
        }

        TimerTask lTask =
            new TimerTask() {
                @Override
                public void run() {
                    Log.i("[Push Service] Timer expired, stopping service");
                    stopForeground();
                    stopSelf();
                }
            };
        mTimer.schedule(lTask, 20000); // Wait 20secs before destroying service

        return START_NOT_STICKY;
    }

    @Override
    public void onTaskRemoved(Intent rootIntent) {
        Log.i("[Push Service] Task removed");
        super.onTaskRemoved(rootIntent);
    }

    @Override
    public synchronized void onDestroy() {
        Log.i("[Push Service] Stopping");
        mTimer.cancel();
        mTimer.purge();

        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    /* Foreground notification related */

    @RequiresApi(api = Build.VERSION_CODES.O)
    /**
     * This method should create a notification channel for the foreground service notification.
     * On Android < 8 it is not called.
     */
    public void createServiceNotificationChannel() {
        Log.i("[Push Service] Android >= 8.0 detected, creating notification channel if not done yet");

        NotificationManagerCompat notificationManager = NotificationManagerCompat.from(this);

        NotificationChannel channel = new NotificationChannel(SERVICE_NOTIFICATION_CHANNEL_ID, SERVICE_NOTIFICATION_CHANNEL_NAME, NotificationManager.IMPORTANCE_LOW);
        channel.setDescription(SERVICE_NOTIFICATION_CHANNEL_DESC);
        channel.enableVibration(false);
        channel.enableLights(false);
        channel.setShowBadge(false);
        notificationManager.createNotificationChannel(channel);
    }

    /**
     * This method should create a notification for the foreground service notification.
     * Remember on Android > 8 to use the notification channel created in createServiceNotificationChannel().
     */
    public void createServiceNotification() {
        Log.i("[Push Service] Creating notification for foreground service");
        mServiceNotification = new NotificationCompat.Builder(this, SERVICE_NOTIFICATION_CHANNEL_ID)
                .setContentTitle(SERVICE_NOTIFICATION_TITLE)
                .setContentText(SERVICE_NOTIFICATION_CONTENT)
                .setSmallIcon(getApplicationInfo().icon)
                .setAutoCancel(false)
                .setCategory(Notification.CATEGORY_SERVICE)
                .setVisibility(NotificationCompat.VISIBILITY_SECRET)
                .setWhen(System.currentTimeMillis())
                .setShowWhen(true)
                .setOngoing(true)
                .build();
    }

    /*
     * This method is called when the service should be started as foreground.
     */
    public void showForegroundServiceNotification() {
        if (mServiceNotification == null) {
            createServiceNotification();
        }
        Log.i("[Push Service] Notification has been created, start service as foreground for real");
        DeviceUtils.startDataSyncForegroundService(this, SERVICE_NOTIF_ID, mServiceNotification);
    }

    void startForeground() {
        Log.i("[Push Service] Starting service as foreground");
        showForegroundServiceNotification();
    }

    void stopForeground() {
        Log.i("[Push Service] Stopping service as foreground");
        stopForeground(STOP_FOREGROUND_REMOVE);
    }
}
