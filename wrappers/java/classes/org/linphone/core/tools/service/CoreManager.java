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

import org.linphone.core.AudioDevice;
import org.linphone.core.Call;
import org.linphone.core.Core;
import org.linphone.core.CoreListenerStub;
import org.linphone.core.tools.Log;
import org.linphone.core.tools.PushNotificationUtils;
import org.linphone.core.tools.audio.AudioHelper;
import org.linphone.core.tools.audio.BluetoothHelper;
import org.linphone.mediastream.Version;

import java.lang.reflect.Constructor;
import java.util.Timer;
import java.util.TimerTask;

/**
 * This class is instanciated by the Core and handles the foreground/background API calls,
 * the push notification configuration of the Core when a token is received,
 * the scheduling of the iterate() function and the start of the CoreService service.
 */
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
    private Class mServiceClass;

    private Timer mTimer;
    private Runnable mIterateRunnable;
    private Application.ActivityLifecycleCallbacks mActivityCallbacks;

    private CoreListenerStub mListener;
    private AudioHelper mAudioHelper;
    private BluetoothHelper mBluetoothHelper;

    private native void updatePushNotificationInformation(long ptr, String appId, String token);

    public CoreManager(Object context, Core core) {
        mContext = ((Context) context).getApplicationContext();
        mCore = core;
        sInstance = this;

        // DO NOT ADD A LISTENER ON THE CORE HERE!
        // Wait for onLinphoneCoreStart()

        // Dump some debugging information to the logs
        dumpDeviceInformation();
        dumpLinphoneInformation();

        mActivityCallbacks = new ActivityMonitor();
        ((Application) mContext).registerActivityLifecycleCallbacks(mActivityCallbacks);

        if (mCore.isPushNotificationEnabled()) {
            Log.i("[Core Manager] Push notifications are enabled, starting push helper");
            PushNotificationUtils.init(mContext);
            if (!PushNotificationUtils.isAvailable(mContext)) {
                Log.w("[Core Manager] Push notifications aren't available");
            }
        } else {
            Log.w("[Core Manager] Push notifications aren't enabled");
        }
        
        if (isAndroidXMediaAvailable()) {
            mAudioHelper = new AudioHelper(mContext);
        } else {
            Log.w("[Core Manager] Do you have a dependency on androidx.media:media package?");
        }
        mBluetoothHelper = new BluetoothHelper(mContext);

        Log.i("[Core Manager] Ready");
    }

    public Core getCore() {
        return mCore;
    }

    public void onLinphoneCoreStart() {
        if (mCore.isAutoIterateEnabled()) {
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
            Log.i("[Core Manager] Call to core.iterate() scheduled every 20ms");
        } else {
            Log.w("[Core Manager] Auto core.iterate() isn't enabled, ensure you do it in your application!");
        }

        mListener = new CoreListenerStub() {
            @Override
            public void onLastCallEnded(Core core) {
                Log.i("[Core Manager] Last call ended");
                if (core.isNativeRingingEnabled()) {
                    mAudioHelper.stopRinging();
                } else {
                    mAudioHelper.releaseRingingAudioFocus();
                }
                mAudioHelper.releaseCallAudioFocus();
            }

            @Override
            public void onCallStateChanged(Core core, Call call, Call.State state, String message) {
                if (state == Call.State.IncomingReceived && core.getCallsNb() == 1) {
                    if (core.isNativeRingingEnabled()) {
                        Log.i("[Core Manager] Incoming call received, no other call, start ringing");
                        mAudioHelper.startRinging(mContext, core.getRing());
                    } else {
                        Log.i("[Core Manager] Incoming call received, no other call, acquire ringing audio focus");
                        mAudioHelper.requestRingingAudioFocus();
                    }
                } else if (state == Call.State.Connected) {
                    if (call.getDir() == Call.Dir.Incoming && core.isNativeRingingEnabled()) {
                        Log.i("[Core Manager] Stop incoming call ringing");
                        mAudioHelper.stopRinging();
                    } else {
                        mAudioHelper.releaseRingingAudioFocus();
                    }
                } else if (state == Call.State.OutgoingInit && core.getCallsNb() == 1) {
                    Log.i("[Core Manager] Outgoing call in progress, no other call, acquire ringing audio focus for ringback");
                    mAudioHelper.requestRingingAudioFocus();
                } else if (state == Call.State.StreamsRunning) {
                    Log.i("[Core Manager] Call active, ensure audio focus granted");
                    mAudioHelper.requestCallAudioFocus();
                }
            }
        };        
        if (mAudioHelper != null) mCore.addListener(mListener);

        try {
            mServiceClass = getServiceClass();
            if (mServiceClass == null) mServiceClass = CoreService.class;
            mContext.startService(new Intent().setClass(mContext, mServiceClass));
            Log.i("[Core Manager] Starting service ", mServiceClass.getName());
        } catch (IllegalStateException ise) {
            Log.w("[Core Manager] Failed to start service: ", ise);
            // On Android > 8, if app in background, startService will trigger an IllegalStateException when called from background
            // If not whitelisted temporary by the system like after a push, so assume background
            mCore.enterBackground();
        }
    }

    public void onLinphoneCoreStop() {
        Log.i("[Core Manager] Destroying");

        mContext.stopService(new Intent().setClass(mContext, mServiceClass));
        Log.i("[Core Manager] Stopping service ", mServiceClass.getName());

        if (mTimer != null) {
            mTimer.cancel();
            mTimer.purge();
            mTimer = null;
        }

        mCore = null; // To allow the garbage colletor to free the Core
        sInstance = null;
    }

    public void startAudioForEchoTestOrCalibration() {
        if (mAudioHelper == null) return;
        mAudioHelper.startAudioForEchoTestOrCalibration();
    }

    public void stopAudioForEchoTestOrCalibration() {
        if (mAudioHelper == null) return;
        mAudioHelper.stopAudioForEchoTestOrCalibration();
    }

    public void onAudioFocusLost() {
        Log.i("[Core Manager] App has lost audio focus, pause current call if any");
        if (mCore != null) {
            Call call = mCore.getCurrentCall();
            if (call != null) {
                call.pause();
            }
        }
    }

    public void onBluetoothHeadsetStateChanged() {
        Log.i("[Core Manager] Bluetooth headset state changed, reload sound devices");
        mCore.reloadSoundDevices();
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

    private boolean isAndroidXMediaAvailable() {
        boolean available = false;
        try {
            Class androixMedia = Class.forName("androidx.media.AudioAttributesCompat.Builder");
            Class[] types = {};
            Constructor constructor = androixMedia.getConstructor(types);
            available = true;
        } catch (ClassNotFoundException e) {
            Log.w("[Core Manager] Couldn't find class androidx.media.AudioAttributesCompat.Builder");
        } catch (Exception e) {
            Log.w("[Core Manager] Exception: " + e);
        }
        return available;
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
