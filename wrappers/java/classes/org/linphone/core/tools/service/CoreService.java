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

import org.linphone.core.Call;
import org.linphone.core.Core;
import org.linphone.core.CoreListenerStub;
import org.linphone.core.Factory;
import org.linphone.core.tools.Log;
import org.linphone.mediastream.Version;

public class CoreService extends Service {
    protected static final int SERVICE_NOTIF_ID = 1;
    protected static final String SERVICE_NOTIFICATION_CHANNEL_ID = "org_linphone_core_service_notification_channel";
    protected static final String SERVICE_NOTIFICATION_CHANNEL_NAME = "Linphone Core Service";
    protected static final String SERVICE_NOTIFICATION_CHANNEL_DESC = "Used to keep the call(s) alive";
    protected static final String SERVICE_NOTIFICATION_TITLE = "Linphone Core Service";
    protected static final String SERVICE_NOTIFICATION_CONTENT = "Used to keep the call(s) alive";

    protected CoreListenerStub mListener;
    protected boolean mIsInForegroundMode = false;
    protected Notification mServiceNotification = null;

    @Override
    public void onCreate() {
        super.onCreate();

        // No-op, just to ensure libraries have been loaded and thus prevent crash in log below 
        // if service has been started directly by Android (that can happen...)
        Factory.instance();

        if (Version.sdkAboveOrEqual(Version.API26_O_80)) {
            createServiceNotificationChannel();
        }

        mListener = new CoreListenerStub() {
            @Override
            public void onCallStateChanged(Core core, Call call, Call.State state, String message) {
                if (state == Call.State.IncomingReceived || state == Call.State.IncomingEarlyMedia || state == Call.State.OutgoingInit) {
                    if (core.getCallsNb() == 1) {
                        // There is only one call, service shouldn't be in foreground mode yet
                        if (!mIsInForegroundMode) {
                            startForeground();
                        }
                    }
                } else if (state == Call.State.End || state == Call.State.Released || state == Call.State.Error) {
                    if (core.getCallsNb() == 0) {
                        if (mIsInForegroundMode) {
                            stopForeground();
                        }
                    }
                }
            }
        };

        if (CoreManager.isReady()) {
            Core core = CoreManager.instance().getCore();
            if (core != null) {
                Log.i("[Core Service] Core Manager found, adding our listener");
                core.addListener(mListener);
            }
        }

        Log.i("[Core Service] Created");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);

        Log.i("[Core Service] Started");
        return START_STICKY;
    }

    @Override
    public void onTaskRemoved(Intent rootIntent) {
        Log.i("[Core Service] Task removed");
        stopSelf();

        super.onTaskRemoved(rootIntent);
    }

    @Override
    public synchronized void onDestroy() {
        Log.i("[Core Service] Stopping");

        if (CoreManager.isReady()) {
            Core core = CoreManager.instance().getCore();
            if (core != null) {
                core.removeListener(mListener);
                core.stop();
            }
        }

        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    /* Foreground notification related */

    @RequiresApi(api = Build.VERSION_CODES.O)
    public void createServiceNotificationChannel() {
        Log.i("[Core Service] Android >= 8.0 detected, creating notification channel");

        NotificationManagerCompat notificationManager = NotificationManagerCompat.from(this);

        NotificationChannel channel = new NotificationChannel(SERVICE_NOTIFICATION_CHANNEL_ID, SERVICE_NOTIFICATION_CHANNEL_NAME, NotificationManager.IMPORTANCE_LOW);
        channel.setDescription(SERVICE_NOTIFICATION_CHANNEL_DESC);
        channel.enableVibration(false);
        channel.enableLights(false);
        channel.setShowBadge(false);
        notificationManager.createNotificationChannel(channel);
    }

    public void createServiceNotification() {
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

    public void showForegroundServiceNotification() {
        if (mServiceNotification == null) {
            createServiceNotification();
        }
        startForeground(SERVICE_NOTIF_ID, mServiceNotification);
    }

    public void hideForegroundServiceNotification() {
        stopForeground(true);
    }

    void startForeground() {
        Log.i("[Core Service] Starting service as foreground");
        showForegroundServiceNotification();
        mIsInForegroundMode = true;
    }

    void stopForeground() {
        if (!mIsInForegroundMode) {
            Log.w("[Core Service] Service isn't in foreground mode, nothing to do");
            return;
        }

        Log.i("[Core Service] Stopping service as foreground");
        hideForegroundServiceNotification();
        mIsInForegroundMode = false;
    }
}
