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

import com.google.firebase.FirebaseApp;

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

public abstract class AndroidService extends Service {
    private static AndroidService sInstance;

    public static boolean isReady() {
        return sInstance != null;
    }

    public static AndroidService instance() {
        if (isReady()) return sInstance;

        throw new RuntimeException("AndroidService not instantiated yet");
    }

    public static Core getCore() {
        if (!isReady()) return null;
        return sInstance.mCore;
    }

    public abstract String getDefaultConfigFilePath();
    public abstract String getFactoryConfigFilePath();
    public abstract String getPushNotificationAppId();

    protected Core mCore;
    protected CoreListenerStub mListener;

    private final Handler handler = new Handler();
    private Timer mTimer;

    @SuppressWarnings("unchecked")
    @Override
    public void onCreate() {
        super.onCreate();

        boolean isDebugEnabled = Factory.instance().createConfig(getFactoryConfigFilePath()).getBool("app", "debug", false);
        if (isDebugEnabled) {
            Factory.instance().enableLogCollection(LogCollectionState.Enabled);
            Factory.instance().setDebugMode(isDebugEnabled, "Linphone");
            Factory.instance().setLogCollectionPath(getFilesDir().getAbsolutePath());
        }

        // Dump some debugging information to the logs
        dumpDeviceInformation();
        dumpInstalledLinphoneInformation();

        mListener = new CoreListenerStub() {};

        FirebaseApp.initializeApp(this);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);

        boolean isPush = false;
        if (intent != null && intent.getBooleanExtra("PushNotification", false)) {
            Log.i("[Service] [Push Notification] LinphoneService started because of a push");
            isPush = true;
        }

        if (sInstance != null) {
            Log.w("[Service] Attempt to start the LinphoneService but it is already running !");
            return START_STICKY;
        }

        mCore = Factory.instance().createCore(getDefaultConfigFilePath(), getFactoryConfigFilePath(), this);
        sInstance = this; // sInstance is ready once Core has been created

        PushNotificationUtils.init(this);
        if (PushNotificationUtils.isAvailable(this)) {
            // TODO
        }
        
        mCore.addListener(mListener);

        if (isPush) {
            Log.w("[Service] We are here because of a received push notification, enter background mode before starting the Core");
            mCore.enterBackground();
        }

        mCore.start();
        TimerTask lTask = new TimerTask() {
            @Override
            public void run() {
                handler.post(
                        new Runnable() {
                            @Override
                            public void run() {
                                if (mCore != null) {
                                    mCore.iterate();
                                }
                            }
                        });
            }
        };
        /*use schedule instead of scheduleAtFixedRate to avoid iterate from being call in burst after cpu wake up*/
        mTimer = new Timer("Linphone scheduler");
        mTimer.schedule(lTask, 0, 20);

        return START_STICKY;
    }

    @Override
    public void onTaskRemoved(Intent rootIntent) {
        Log.i("[Service] Task removed");
        stopSelf();

        super.onTaskRemoved(rootIntent);
    }

    @Override
    public synchronized void onDestroy() {
        Log.i("[Service] Stopping");
        mTimer.cancel();

        mCore.removeListener(mListener);
        mCore.stop();
        mCore = null;

        sInstance = null;
        super.onDestroy();
    }

    private void onBackgroundMode() {
        Log.i("[Service] App has entered background mode");
        if (mCore != null) {
            mCore.enterBackground();
        }
    }

    private void onForegroundMode() {
        Log.i("[Service] App has left background mode");
        if (mCore != null) {
            mCore.enterForeground();
        }
    }

    private void dumpDeviceInformation() {
        Log.i("==== Phone information dump ====");
        Log.i("DEVICE=", Build.DEVICE);
        Log.i("MODEL=", Build.MODEL);
        Log.i("MANUFACTURER=", Build.MANUFACTURER);
        Log.i("ANDROID SDK=", Build.VERSION.SDK_INT);

        StringBuilder sb = new StringBuilder();
        sb.append("Supported ABIs=");
        for (String abi : Version.getCpuAbis()) {
            sb.append(abi).append(", ");
        }
        sb.append("\n");
        Log.i(sb.toString());
    }

    private void dumpInstalledLinphoneInformation() {
        Log.i("==== Linphone information dump ====");
        Log.i("VERSION NAME=", org.linphone.core.BuildConfig.VERSION_NAME);
        Log.i("VERSION CODE=", org.linphone.core.BuildConfig.VERSION_CODE);
        Log.i("PACKAGE=", org.linphone.core.BuildConfig.APPLICATION_ID);
        Log.i("BUILD TYPE=", org.linphone.core.BuildConfig.BUILD_TYPE);
        Log.i("SDK VERSION=", getString(org.linphone.core.R.string.linphone_sdk_version));
        Log.i("SDK BRANCH=", getString(org.linphone.core.R.string.linphone_sdk_branch));
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}
