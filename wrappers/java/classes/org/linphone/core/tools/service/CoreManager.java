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

import org.linphone.core.Call;
import org.linphone.core.Core;
import org.linphone.core.CoreListenerStub;
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
    protected CoreListenerStub mListener;

    private Timer mTimer;
    private Runnable mIterateRunnable;
    private Application.ActivityLifecycleCallbacks mActivityCallbacks;

    public CoreManager(Object context, Core core) {
        mContext = ((Context) context).getApplicationContext();
        mCore = core;
        sInstance = this;
        Log.i("[Core Manager] Ready");

        mActivityCallbacks = new ActivityMonitor();
        ((Application) mContext).registerActivityLifecycleCallbacks(mActivityCallbacks);

        mListener = new CoreListenerStub() {
            @Override
            public void onCallStateChanged(Core core, Call call, Call.State state, String message) {
                if (state == Call.State.IncomingReceived || state == Call.State.IncomingEarlyMedia || state == Call.State.OutgoingInit) {
                    if (core.getCallsNb() == 1) {
                        // There is only one call, service shouldn't be in foreground mode yet
                        if (CoreService.isReady() && !CoreService.instance().isInForegroundMode()) {
                            CoreService.instance().startForeground();
                        }
                    }
                } else if (state == Call.State.End || state == Call.State.Released || state == Call.State.Error) {
                    if (core.getCallsNb() == 0) {
                        if (CoreService.isReady() && CoreService.instance().isInForegroundMode()) {
                            CoreService.instance().stopForeground();
                        }
                    }
                }
            }
        };
        mCore.addListener(mListener);

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
        mTimer = new Timer("Linphone scheduler");
        mTimer.schedule(lTask, 0, 20);

        String serviceName = getServiceClassName();
        if (serviceName != null) {
            Intent intent = new Intent(Intent.ACTION_MAIN);
            intent.setClassName(mContext, serviceName);

            try {
                mContext.startService(intent);
                Log.i("[Core Manager] Starting service");
            } catch (IllegalStateException ise) {
                Log.w("[Core Manager] Failed to start service: ", ise);
                // On Android > 8, if app in background, startService will trigger an IllegalStateException when called from background
                // If not whitelisted temporary by the system like after a push, so assume background
                mCore.enterBackground();
            }
        }
    }

    public void onLinphoneCoreStop() {
        Log.i("[Core Manager] Destroying");
        if (mCore != null) {
            mCore.removeListener(mListener);
            mCore = null; // To allow the gc calls below to free the Core
        }

        mTimer.cancel();
        mTimer.purge();
        mTimer = null;

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
        
        String contactUriParams = "app-id=" + appId + ";pn-type=firebase;pn-timeout=0;pn-tok=" + token + ";pn-silent=1";
        for (ProxyConfig proxyConfig : mCore.getProxyConfigList()) {
            if (proxyConfig.isPushNotificationAllowed()) {
                String currentUriParams = proxyConfig.getContactUriParameters();
                if (currentUriParams == null || !currentUriParams.equals(contactUriParams)) {
                    proxyConfig.edit();
                    proxyConfig.setContactUriParameters(contactUriParams);
                    proxyConfig.done();
                    Log.i("[Core Manager] Updated contact URI parameters for proxy config [", proxyConfig, "]: ", contactUriParams);
                }
            }
        }
    }

    private String getServiceClassName() {
        // Inspect services in package to get the class name of the Service that extends LinphoneService, assume first one
        String className = null;
        try {
            PackageInfo packageInfo = mContext.getPackageManager().getPackageInfo(mContext.getPackageName(), PackageManager.GET_SERVICES);
            ServiceInfo[] services = packageInfo.services;
            for (ServiceInfo service : services) {
                String serviceName = service.name;
                // Do not attempt to start any firebase related service...
                if (!serviceName.equals("org.linphone.core.tools.firebase.FirebaseMessaging") && !serviceName.startsWith("com.google.firebase")) {
                    className = serviceName;
                    Log.i("[Core Manager] Found Service class [", className, "]");
                    break;
                }
            }
        } catch (Exception e) {
            Log.e("[Core Manager] Couldn't find Service class: ", e);
        }

        if (className == null) {
            Log.w("[Core Manager] Failed to find a valid Service, continuing without it...");
        }
        return className;
    }
}
