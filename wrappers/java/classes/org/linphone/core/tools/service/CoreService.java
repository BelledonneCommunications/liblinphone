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
import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

import org.linphone.core.Factory;
import org.linphone.core.tools.Log;

public abstract class CoreService extends Service {
    private static CoreService sInstance;

    private boolean mIsInForegroundMode = false;

    public static boolean isReady() {
        return sInstance != null;
    }

    public static CoreService instance() {
        if (isReady()) return sInstance;

        throw new RuntimeException("CoreService not instantiated yet");
    }

    @Override
    public void onCreate() {
        super.onCreate();

        // No-op, just to ensure libraries have been loaded and thus prevent crash in log below 
        // if service has been started directly by Android (that can happen...)
        Factory.instance(); 

        Log.i("[Core Service] Created");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        sInstance = this;

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
            CoreManager.instance().getCore().stop();
        }
        sInstance = null;

        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    /* Foreground notification related */

    public abstract void showForegroundServiceNotification();
    public abstract void hideForegroundServiceNotification();

    public void startForeground() {
        Log.i("[Core Service] Starting service as foreground");
        showForegroundServiceNotification();
        mIsInForegroundMode = true;
    }

    public void stopForeground() {
        if (!mIsInForegroundMode) {
            Log.w("[Core Service] Service isn't in foreground mode, nothing to do");
            return;
        }

        Log.i("[Core Service] Stopping service as foreground");
        hideForegroundServiceNotification();
        mIsInForegroundMode = false;
    }

    public boolean isInForegroundMode() {
        return mIsInForegroundMode;
    }
}
