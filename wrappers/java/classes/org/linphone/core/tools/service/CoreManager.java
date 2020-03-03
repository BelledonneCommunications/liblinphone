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

    public static Core getCore() {
        if (!isReady()) return null;
        return sInstance.mCore;
    }

    protected Context mContext;
    protected Core mCore;
    protected CoreListenerStub mListener;

    private Timer mTimer;
    private Runnable mIterateRunnable;
    private Application.ActivityLifecycleCallbacks mActivityCallbacks;

    public CoreManager(Context context) {
        mContext = context.getApplicationContext();

        boolean isDebugEnabled = Factory.instance().createConfig(null /*TODO*/).getBool("app", "debug", true);
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

        FirebaseApp.initializeApp(mContext);
        PushNotificationUtils.init(mContext);
        if (!PushNotificationUtils.isAvailable(mContext)) {
            Log.w("[Core Manager] Push notifications won't work !");
        }
    }

    public void start() {
        Log.i("[Core Manager] Starting");
        start(false, true);
    }

    public void startFromPush() {
        Log.i("[Core Manager] Starting from push notification");
        start(true, true);
    }

    public void startFromService() {
        Log.i("[Core Manager] Starting from Service");
        start(false, false);
    }

    private void start(boolean isPush, boolean startService) {
        mCore = Factory.instance().createCore(null /*TODO*/, null /*TODO*/, mContext);
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
                        AndroidDispatcher.dispatchOnUIThread(mIterateRunnable);
                    }
                };

        /*use schedule instead of scheduleAtFixedRate to avoid iterate from being call in burst after cpu wake up*/
        mTimer = new Timer("Linphone scheduler");
        mTimer.schedule(lTask, 0, 20);

        if (startService) {
            Log.i("[Core Manager] Starting service");
            mContext.startService(new Intent(mContext, CoreService.class));
        }
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
        Log.i("[Core Manager] [Push Notification] App id is [", appId, "], with push token [", token, "]");
        
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

    /* Log device related information */

    private void dumpDeviceInformation() {
        Log.i("==== Device information dump ====");
        Log.i("NAME=" + Build.DEVICE);
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
        Log.i("BUILD TYPE=" + org.linphone.core.BuildConfig.BUILD_TYPE);
        Log.i("VERSION=" + mContext.getString(org.linphone.core.R.string.linphone_sdk_version));
        Log.i("BRANCH=" + mContext.getString(org.linphone.core.R.string.linphone_sdk_branch));
    }
}
