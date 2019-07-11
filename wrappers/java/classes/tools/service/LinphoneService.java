package org.linphone.core.tools.service;

/*
LinphoneService.java
Copyright (C) 2017-2019 Belledonne Communications, Grenoble, France

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

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
import org.linphone.mediastream.Version;

public abstract class LinphoneService extends Service {
    private static final String START_LINPHONE_LOGS = " ==== Phone information dump ====";
    private static LinphoneService sInstance;

    public static boolean isReady() {
        return sInstance != null;
    }

    public static LinphoneService instance() {
        if (isReady()) return sInstance;

        throw new RuntimeException("LinphoneService not instantiated yet");
    }

    public static Core getCore() {
        if (!isReady()) return null;
        return sInstance.mCore;
    }

    public abstract String getDefaultConfigFilePath();

    public abstract String getFactoryConfigFilePath();

    public abstract String getPushNotificationAppId();

    public final Handler handler = new Handler();

    private Application.ActivityLifecycleCallbacks mActivityCallbacks;
    private Timer mTimer;
    private Core mCore;
    private CoreListenerStub mListener;

    @SuppressWarnings("unchecked")
    @Override
    public void onCreate() {
        super.onCreate();

        setupActivityMonitor();

        boolean isDebugEnabled = Factory.instance().createConfig(getFactoryConfigFilePath()).getBool("app", "debug", false);
        if (isDebugEnabled) {
            Factory.instance().enableLogCollection(LogCollectionState.Enabled);
            Factory.instance().setDebugMode(isDebugEnabled, "Linphone");
            Factory.instance().setLogCollectionPath(getFilesDir().getAbsolutePath());
        }

        // Dump some debugging information to the logs
        Log.i(START_LINPHONE_LOGS);
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
            mCore.configurePushNotifications("firebase", getPushNotificationAppId());
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

    private void setupActivityMonitor() {
        if (mActivityCallbacks != null) return;
        getApplication()
                .registerActivityLifecycleCallbacks(mActivityCallbacks = new ActivityMonitor());
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
        StringBuilder sb = new StringBuilder();
        sb.append("DEVICE=").append(Build.DEVICE).append("\n");
        sb.append("MODEL=").append(Build.MODEL).append("\n");
        sb.append("MANUFACTURER=").append(Build.MANUFACTURER).append("\n");
        sb.append("SDK=").append(Build.VERSION.SDK_INT).append("\n");
        sb.append("Supported ABIs=");
        for (String abi : Version.getCpuAbis()) {
            sb.append(abi).append(", ");
        }
        sb.append("\n");
        Log.i(sb.toString());
    }

    private void dumpInstalledLinphoneInformation() {
        PackageInfo info = null;
        try {
            info = getPackageManager().getPackageInfo(getPackageName(), 0);
        } catch (NameNotFoundException nnfe) {
            Log.e(nnfe);
        }

        if (info != null) {
            Log.i(
                    "[Service] Linphone version is ",
                    info.versionName + " (" + info.versionCode + ")");
        } else {
            Log.i("[Service] Linphone version is unknown");
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onTaskRemoved(Intent rootIntent) {
        stopSelf();
        super.onTaskRemoved(rootIntent);
    }

    @Override
    public synchronized void onDestroy() {
        if (mActivityCallbacks != null) {
            getApplication().unregisterActivityLifecycleCallbacks(mActivityCallbacks);
            mActivityCallbacks = null;
        }

        mTimer.cancel();

        mCore.removeListener(mListener);
        mCore.stop();
        mCore = null;

        sInstance = null;
        super.onDestroy();
    }

    /*Believe me or not, but knowing the application visibility state on Android is a nightmare.
    After two days of hard work I ended with the following class, that does the job more or less reliabily.
    */
    class ActivityMonitor implements Application.ActivityLifecycleCallbacks {
        private final ArrayList<Activity> activities = new ArrayList<>();
        private boolean mActive = false;
        private int mRunningActivities = 0;
        private InactivityChecker mLastChecker;

        @Override
        public synchronized void onActivityCreated(Activity activity, Bundle savedInstanceState) {
            Log.i("[Service] Activity created:" + activity);
            if (!activities.contains(activity)) activities.add(activity);
        }

        @Override
        public void onActivityStarted(Activity activity) {
            Log.i("Activity started:" + activity);
        }

        @Override
        public synchronized void onActivityResumed(Activity activity) {
            Log.i("[Service] Activity resumed:" + activity);
            if (activities.contains(activity)) {
                mRunningActivities++;
                Log.i("[Service] runningActivities=" + mRunningActivities);
                checkActivity();
            }
        }

        @Override
        public synchronized void onActivityPaused(Activity activity) {
            Log.i("[Service] Activity paused:" + activity);
            if (activities.contains(activity)) {
                mRunningActivities--;
                Log.i("[Service] runningActivities=" + mRunningActivities);
                checkActivity();
            }
        }

        @Override
        public void onActivityStopped(Activity activity) {
            Log.i("[Service] Activity stopped:" + activity);
        }

        @Override
        public void onActivitySaveInstanceState(Activity activity, Bundle outState) {}

        @Override
        public synchronized void onActivityDestroyed(Activity activity) {
            Log.i("[Service] Activity destroyed:" + activity);
            activities.remove(activity);
        }

        void startInactivityChecker() {
            if (mLastChecker != null) mLastChecker.cancel();
            LinphoneService.this.handler.postDelayed(
                    (mLastChecker = new InactivityChecker()), 2000);
        }

        void checkActivity() {
            if (mRunningActivities == 0) {
                if (mActive) startInactivityChecker();
            } else if (mRunningActivities > 0) {
                if (!mActive) {
                    mActive = true;
                    LinphoneService.this.onForegroundMode();
                }
                if (mLastChecker != null) {
                    mLastChecker.cancel();
                    mLastChecker = null;
                }
            }
        }

        class InactivityChecker implements Runnable {
            private boolean isCanceled;

            void cancel() {
                isCanceled = true;
            }

            @Override
            public void run() {
                synchronized (LinphoneService.this) {
                    if (!isCanceled) {
                        if (ActivityMonitor.this.mRunningActivities == 0 && mActive) {
                            mActive = false;
                            LinphoneService.this.onBackgroundMode();
                        }
                    }
                }
            }
        }
    }
}
