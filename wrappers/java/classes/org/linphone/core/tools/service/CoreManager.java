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

import android.app.Application;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ServiceInfo;
import android.os.Build;

import com.google.firebase.FirebaseApp;

import org.linphone.core.Core;
import org.linphone.core.Factory;
import org.linphone.core.LogCollectionState;
import org.linphone.core.ProxyConfig;
import org.linphone.core.tools.Log;
import org.linphone.core.tools.PushNotificationUtils;
import org.linphone.mediastream.Version;

import java.lang.IllegalStateException;
import java.util.Timer;
import java.util.TimerTask;

public class CoreManager {
    private static CoreManager sInstance;

    public static boolean isReady() {
        return sInstance != null;
    }

    public static CoreManager instance() {
        if (isReady()) return sInstance;

        throw new RuntimeException("CoreManager not instantiated yet");
    }

    protected Context mContext;
    protected Core mCore;

    private Timer mTimer;
    private Runnable mIterateRunnable;
    private Application.ActivityLifecycleCallbacks mActivityCallbacks;

    private native void updatePushNotificationInformation(long ptr, String appId, String token);

    public CoreManager(Object context, Core core) {
        mContext = ((Context) context).getApplicationContext();
        mCore = core;
        sInstance = this;

        // Dump some debugging information to the logs
        dumpDeviceInformation();
        dumpLinphoneInformation();
        Log.i("[Core Manager] Ready");

        mActivityCallbacks = new ActivityMonitor();
        ((Application) mContext).registerActivityLifecycleCallbacks(mActivityCallbacks);

        if (mCore.isPushNotificationEnabled()) {
            Log.i("[Core Manager] Push notifications are enabled, starting Firebase");
            FirebaseApp.initializeApp(mContext);
            PushNotificationUtils.init(mContext);
            if (!PushNotificationUtils.isAvailable(mContext)) {
                Log.w("[Core Manager] Push notifications aren't available");
            }
        }
    }

    public Core getCore() {
        return mCore;
    }

    public void onLinphoneCoreStart() {
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
                        AndroidDispatcher.dispatchOnUIThread(mIterateRunnable);
                    }
                };

        /*use schedule instead of scheduleAtFixedRate to avoid iterate from being call in burst after cpu wake up*/
        mTimer = new Timer("Linphone Core iterate scheduler");
        mTimer.schedule(lTask, 0, 20);

        try {
            Class serviceClass = getServiceClass();
            if (serviceClass == null) serviceClass = CoreService.class;
            mContext.startService(new Intent().setClass(mContext, serviceClass));
            Log.i("[Core Manager] Starting service ", serviceClass.getName());
        } catch (IllegalStateException ise) {
            Log.w("[Core Manager] Failed to start service: ", ise);
            // On Android > 8, if app in background, startService will trigger an IllegalStateException when called from background
            // If not whitelisted temporary by the system like after a push, so assume background
            mCore.enterBackground();
        }
    }

    public void onLinphoneCoreStop() {
        Log.i("[Core Manager] Destroying");

        mTimer.cancel();
        mTimer.purge();
        mTimer = null;

        mCore = null; // To allow the garbage colletor to free the Core
        sInstance = null;
    }

    public void onBackgroundMode() {
        Log.i("[Core Manager] App has entered background mode");
        if (mCore != null) {
            mCore.enterBackground();
        }
    }

    public void onForegroundMode() {
        Log.i("[Core Manager] App has left background mode");
        if (mCore != null) {
            mCore.enterForeground();
        }
    }

    public void setPushToken(String token) {
        int resId = mContext.getResources().getIdentifier("gcm_defaultSenderId", "string", mContext.getPackageName());
        String appId = mContext.getString(resId);
        Log.i("[Core Manager] Push notification app id is [", appId, "] and token is [", token, "]");
        updatePushNotificationInformation(mCore.getNativePointer(), appId, token);
    }

    private Class getServiceClass() {
        // Inspect services in package to get the class name of the Service that extends LinphoneService, assume first one
        try {
            PackageInfo packageInfo = mContext.getPackageManager().getPackageInfo(mContext.getPackageName(), PackageManager.GET_SERVICES);
            ServiceInfo[] services = packageInfo.services;
            for (ServiceInfo service : services) {
                String serviceName = service.name;
                Class serviceClass = Class.forName(serviceName);
                if (CoreService.class.isAssignableFrom(serviceClass)) {
                    Log.i("[Core Manager] Found a service that herits from org.linphone.core.tools.service.CoreService: ", serviceName);
                    return serviceClass;
                }
            }
        } catch (Exception e) {
            Log.e("[Core Manager] Couldn't find Service class: ", e);
        }

        Log.w("[Core Manager] Failed to find a valid Service, continuing without it...");
        return null;
    }

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
        Log.i("==== Linphone SDK information dump ====");
        Log.i("VERSION=", mContext.getString(org.linphone.core.R.string.linphone_sdk_version));
        Log.i("BRANCH=", mContext.getString(org.linphone.core.R.string.linphone_sdk_branch));
        StringBuilder sb = new StringBuilder();
        sb.append("PLUGINS=");
        for (String plugin : org.linphone.core.BuildConfig.PLUGINS_ARRAY) {
            sb.append(plugin).append(", ");
        }
        Log.i(sb.substring(0, sb.length() - 2));
        Log.i("PACKAGE=", org.linphone.core.BuildConfig.APPLICATION_ID);
        Log.i("BUILD TYPE=", org.linphone.core.BuildConfig.BUILD_TYPE);
    }
}
