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
import android.content.Context;
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

import org.linphone.core.Call;
import org.linphone.core.Core;
import org.linphone.core.CoreListenerStub;
import org.linphone.core.Factory;
import org.linphone.core.LogCollectionState;
import org.linphone.core.tools.Log;
import org.linphone.core.tools.PushNotificationUtils;
import org.linphone.core.tools.compatibility.DeviceUtils;
import org.linphone.mediastream.Version;

public class CoreManager {
    private static CoreManager sInstance;

    public static boolean isReady() {
        return sInstance != null;
    }

    public static CoreManager instance() {
        if (isReady()) return sInstance;

        throw new RuntimeException("CoreManager not instantiated yet");
    }

    public static Core getCore() {
        if (!isReady()) return null;
        return sInstance.mCore;
    }

    protected Context mContext;
    protected Core mCore;
    protected CoreListenerStub mListener;

    protected CoreListenerStub mPrivateListener;
    private final Handler mHandler = new Handler();
    private Timer mTimer;
    private Runnable mIterateRunnable;

    public CoreManager(Context context) {
        mContext = context.getApplicationContext();

        boolean isDebugEnabled = Factory.instance().createConfig(null /*TODO*/).getBool("app", "debug", false);
        if (isDebugEnabled) {
            Factory.instance().enableLogCollection(LogCollectionState.Enabled);
            Factory.instance().setDebugMode(isDebugEnabled, "Linphone");
            Factory.instance().setLogCollectionPath(mContext.getFilesDir().getAbsolutePath());
        }

        // Dump some debugging information to the logs
        dumpDeviceInformation();
        dumpLinphoneInformation();

        sInstance = this;
        Log.i("[Core Manager] Ready");
        
        mPrivateListener = new CoreListenerStub() {
            @Override
            public void onCallStateChanged(Core core, Call call, Call.State state, String message) {
                Log.i("[Core Manager] Call state is [", state, "]");
                if (state == Call.State.IncomingReceived || state == Call.State.IncomingEarlyMedia) {
                    // In case of push notification Service won't be started until here
                    if (!CoreService.isReady()) {
                        Log.i("[Core Manager] Service not running, starting it");
                        Intent intent = new Intent(Intent.ACTION_MAIN);
                        intent.setClass(mContext, CoreService.class);
                        mContext.startService(intent);
                    }
                }
            }
        };
        mListener = new CoreListenerStub() {};

        PushNotificationUtils.init(mContext);
        if (!PushNotificationUtils.isAvailable(mContext)) {
            Log.w("[Core Manager] Push notifications won't work !");
        }
    }

    public void start(boolean isPush) {
        Log.i("[Core Manager] Starting, push status is ", isPush);
        mCore = Factory.instance().createCore(null /*TODO*/, null /*TODO*/, mContext);
        mCore.addListener(mPrivateListener);
        mCore.addListener(mListener);

        if (isPush) {
            Log.w("[Core Manager] We are here because of a received push notification, enter background mode before starting the Core");
            mCore.enterBackground();
        }

        mCore.start();

        mIterateRunnable =
            new Runnable() {
                @Override
                public void run() {
                    if (mCore != null) {
                        mCore.iterate();
                    }
                }
            };
        TimerTask lTask =
            new TimerTask() {
                @Override
                public void run() {
                    mHandler.post(mIterateRunnable);
                }
            };

        /*use schedule instead of scheduleAtFixedRate to avoid iterate from being call in burst after cpu wake up*/
        mTimer = new Timer("Linphone scheduler");
        mTimer.schedule(lTask, 0, 20);
    }

    public void destroy() {
        Log.i("[Core Manager] Destroying");
        if (mCore != null) {
            mCore.removeListener(mListener);
            mCore = null; // To allow the gc calls below to free the Core
        }

        // Wait for every other object to be destroyed to make CoreService.instance() invalid
        sInstance = null;
    }

    private void onBackgroundMode() {
        Log.i("[Core Manager] App has entered background mode");
        if (mCore != null) {
            mCore.enterBackground();
        }
    }

    private void onForegroundMode() {
        Log.i("[Core Manager] App has left background mode");
        if (mCore != null) {
            mCore.enterForeground();
        }
    }

    /* Log device related information */

    private void dumpDeviceInformation() {
        Log.i("==== Phone information dump ====");
        Log.i("DEVICE=" + Build.DEVICE);
        Log.i("MODEL=" + Build.MODEL);
        Log.i("MANUFACTURER=" + Build.MANUFACTURER);
        Log.i("ANDROID SDK=" + Build.VERSION.SDK_INT);
        
        StringBuilder sb = new StringBuilder();
        sb.append("ABIs=");
        for (String abi : Version.getCpuAbis()) {
            sb.append(abi).append(", ");
        }
        Log.i(sb.substring(0, sb.length() - 2));
    }

    private void dumpLinphoneInformation() {
        Log.i("==== Linphone information dump ====");
        Log.i("VERSION NAME=" + org.linphone.core.BuildConfig.VERSION_NAME);
        Log.i("VERSION CODE=" + org.linphone.core.BuildConfig.VERSION_CODE);
        Log.i("PACKAGE=" + org.linphone.core.BuildConfig.APPLICATION_ID);
        Log.i("BUILD TYPE=" + org.linphone.core.BuildConfig.BUILD_TYPE);
        Log.i("SDK VERSION=" + mContext.getString(org.linphone.core.R.string.linphone_sdk_version));
        Log.i("SDK BRANCH=" + mContext.getString(org.linphone.core.R.string.linphone_sdk_branch));
    }
}
