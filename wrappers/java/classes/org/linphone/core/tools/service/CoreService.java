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

import android.app.Activity;
import android.app.Application;
import android.app.Service;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;

import java.util.ArrayList;
import java.util.Timer;
import java.util.TimerTask;

import org.linphone.core.Core;
import org.linphone.core.CoreListenerStub;
import org.linphone.core.Factory;
import org.linphone.core.LogCollectionState;
import org.linphone.core.tools.Log;
import org.linphone.core.tools.PushNotificationUtils;
import org.linphone.core.tools.compatibility.DeviceUtils;
import org.linphone.mediastream.Version;

public abstract class CoreService extends Service {
    private static CoreService sInstance;

    private boolean mIsCoreManagerOwned;

    public static boolean isReady() {
        return sInstance != null;
    }

    public static CoreService instance() {
        if (isReady()) return sInstance;

        throw new RuntimeException("CoreService not instantiated yet");
    }

    @SuppressWarnings("unchecked")
    @Override
    public void onCreate() {
        super.onCreate();

        mIsCoreManagerOwned = false;
        if (!CoreManager.isReady()) {
            new CoreManager(getApplicationContext());
            mIsCoreManagerOwned = true;
        }

        Log.i("[Core Service] Created");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        sInstance = this; // sInstance is ready once Core has been created
        
        boolean isPush = false;
        if (intent != null && intent.getBooleanExtra("PushNotification", false)) {
            Log.i("[Core Service] [Push Notification] Service started because of a push");
            isPush = true;
        }

        if (sInstance != null) {
            Log.w("[Core Service] Attempt to start the Service but it is already running !");
            return START_STICKY;
        }
        sInstance = this;

        if (mIsCoreManagerOwned) {
            CoreManager.instance().start(isPush);
        }

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

        CoreManager.instance().destroy();
        sInstance = null;

        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}
