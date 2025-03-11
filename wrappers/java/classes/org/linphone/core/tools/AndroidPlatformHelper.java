/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone 
 * (see https://gitlab.linphone.org/BC/public/liblinphone).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

package org.linphone.core.tools;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ServiceInfo;
import android.content.res.Resources;
import android.graphics.SurfaceTexture;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.MulticastLock;
import android.net.wifi.WifiManager.WifiLock;
import android.os.Build;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.view.Surface;
import android.view.TextureView;

import android.app.Application;
import android.app.ActivityManager;
import android.app.ActivityManager.RunningServiceInfo;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.hardware.display.DisplayManager;
import android.media.AudioManager;
import android.view.Display;

import org.linphone.core.SignalType;
import org.linphone.core.SignalStrengthUnit;
import org.linphone.core.tools.compatibility.DeviceUtils;
import org.linphone.core.tools.network.NetworkManager;
import org.linphone.core.tools.network.NetworkManagerAbove21;
import org.linphone.core.tools.network.NetworkManagerAbove23;
import org.linphone.core.tools.network.NetworkManagerAbove24;
import org.linphone.core.tools.network.NetworkManagerAbove26;
import org.linphone.core.tools.network.NetworkManagerInterface;
import org.linphone.core.tools.network.NetworkSignalMonitor;
import org.linphone.core.tools.receiver.DozeReceiver;
import org.linphone.core.tools.receiver.InteractivityReceiver;
import org.linphone.core.tools.service.ActivityMonitor;
import org.linphone.core.tools.service.CoreService;
import org.linphone.core.tools.service.PushService;
import org.linphone.core.tools.service.FileTransferService;
import org.linphone.mediastream.MediastreamerAndroidContext;
import org.linphone.mediastream.video.capture.CaptureTextureView;
import org.linphone.mediastream.Version;

import org.linphone.core.AudioDevice;
import org.linphone.core.Call;
import org.linphone.core.Config;
import org.linphone.core.Core;
import org.linphone.core.CoreListenerStub;
import org.linphone.core.GlobalState;
import org.linphone.core.tools.Log;
import org.linphone.core.tools.PushNotificationUtils;
import org.linphone.core.tools.audio.AudioHelper;
import org.linphone.core.tools.audio.BluetoothHelper;
import org.linphone.core.tools.receiver.ShutdownReceiver;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;

import java.lang.Error;
import java.lang.reflect.Constructor;
import java.util.Timer;
import java.util.TimerTask;

/**
 * This class is instanciated directly by the linphone library in order to access specific features only accessible in java.
 **/
public class AndroidPlatformHelper {
    private static int mTempCountWifi = 0;
    private static int mTempCountMCast = 0;
    private static int mTempCountCPU = 0;
    
    private static final int AUTO_ITERATE_TIMER_CORE_START_OR_PUSH_RECEIVED = 20; // 20ms
    private static final int AUTO_ITERATE_TIMER_RESET_AFTER = 20000; // 20s

    private static AndroidPlatformHelper sInstance;

    public static boolean isReady() {
        return sInstance != null;
    }

    public static AndroidPlatformHelper instance() {
        if (isReady()) return sInstance;
        Log.e("[Platform Helper] Trying to access instance that doesn't exists!");
        throw new RuntimeException("AndroidPlatformHelper not instantiated yet");
    }

    private long mNativePtr;
    private Context mContext;
    protected Core mCore;

    private WifiManager.WifiLock mWifiLock;
    private WifiManager.MulticastLock mMcastLock;
    private ConnectivityManager mConnectivityManager;
    private int mLastNetworkType = -1;
    private PowerManager mPowerManager;
    private WakeLock mWakeLock;
    private Resources mResources;
    private TextureView mPreviewTextureView, mVideoTextureView;
    private Map<Long, TextureView> mParticipantTextureView;
    private BroadcastReceiver mDozeReceiver;
    private boolean mWifiOnly;
    private boolean mUsingHttpProxy;
    private NetworkManagerInterface mNetworkManager;
    private Handler mHandler;
    private boolean mMonitoringEnabled;
    private InteractivityReceiver mInteractivityReceiver;
    private ArrayList<String> mDnsServersList;
    private NetworkSignalMonitor mNetworkSignalMonitor;

    private Class mPushServiceClass;
    private boolean mPushServiceStarted;

    private Class mFileTransferServiceClass;
    private boolean mFileTransferServiceStarted;
    private boolean mFileTransferServiceNotificationStarted;
    private boolean mFileTransferServiceStopPending;

    private Class mServiceClass;

    private Timer mTimer;
    private Timer mForcedIterateTimer;
    private Runnable mIterateRunnable;
    private int mIterateSchedule;
    private Application.ActivityLifecycleCallbacks mActivityCallbacks;

    private CoreListenerStub mListener;
    private AudioHelper mAudioHelper;
    private BluetoothHelper mBluetoothHelper;
    private ShutdownReceiver mShutdownReceiver;
    private boolean mReloadSoundDevicesScheduled;

    private DisplayManager mDisplayManager;
    private DisplayManager.DisplayListener mDisplayListener;

    // These methods will make sure the real core.<method> will be called on the same thread as the core.iterate()
    private native void updatePushNotificationInformation(long ptr, String appId, String token);
    private native void stopCore(long ptr);
    private native void leaveConference(long ptr);
    private native void pauseAllCalls(long ptr);
    private native void reloadSoundDevices(long ptr);
    private native void enterBackground(long ptr);
    private native void enterForeground(long ptr);
    private native void processPushNotification(long ptr, String callId, String payload, boolean isCoreStarting);
    private native void healNetworkConnections(long ptr);

    private boolean mServiceRunning;
    private boolean mServiceRunningInForeground;

    private native void setNativePreviewWindowId(long nativePtr, Object view);

    private native void setNativeVideoWindowId(long nativePtr, Object view);

    private native void setParticipantDeviceNativeVideoWindowId(long nativePtr, long participantDevicePtr, Object view);

    private native void setNetworkReachable(long nativePtr, boolean reachable);

    private native void setDnsServers(long nativePtr);

    private native void setHttpProxy(long nativePtr, String host, int port);

    private native boolean isInBackground(long nativePtr);

    private native void enableKeepAlive(long nativePtr, boolean enable);

    private native boolean useSystemHttpProxy(long nativePtr);

    private native void setSignalInfo(long nativePtr, int type, int unit, int value, String details);

    public AndroidPlatformHelper(long nativePtr, Object ctx_obj, Core core, boolean wifiOnly) {
        mNativePtr = nativePtr;
        mContext = ((Context) ctx_obj).getApplicationContext();
        mCore = core;
        mWifiOnly = wifiOnly;
        mDnsServersList = new ArrayList<String>();
        mResources = mContext.getResources();
        mServiceRunning = false;
        mServiceRunningInForeground = false;
        mReloadSoundDevicesScheduled = false;
        
        Looper myLooper = Looper.myLooper();
        if (myLooper == null) {
            Log.w("[Platform Helper] Failed to detect current process Looper (have you called Looper.prepare()?), using main one");
            myLooper = Looper.getMainLooper();
        }
        mHandler = new Handler(myLooper);

        MediastreamerAndroidContext.setContext(mContext);
        mPowerManager = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);

        Thread thread = myLooper.getThread();
        boolean isMainThread = myLooper == Looper.getMainLooper();
        if (isMainThread) {
            Log.i("[Platform Helper] Linphone SDK Android classes will use main thread: [", thread.getName(), "], id=", thread.getId());
        } else {
            Log.i("[Platform Helper] Linphone SDK Android classes won't use main thread: [", thread.getName(), "], id=", thread.getId());
        }
        sInstance = this;

        Log.i("[Platform Helper] Created, wifi only mode is " + (mWifiOnly ? "enabled" : "disabled"));
    }

    public void init() {
        Log.i("[Platform Helper] Initialization started");
        WifiManager wifiMgr = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
        mConnectivityManager = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);

        mWakeLock = mPowerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "AndroidPlatformHelper");
        mWakeLock.setReferenceCounted(true);
        mMcastLock = wifiMgr.createMulticastLock("AndroidPlatformHelper");
        mMcastLock.setReferenceCounted(true);
        mWifiLock = wifiMgr.createWifiLock(WifiManager.WIFI_MODE_FULL_HIGH_PERF, "AndroidPlatformHelper");
        mWifiLock.setReferenceCounted(true);

        try {
            copyAssetsFromPackage();
        } catch (IOException e) {
            Log.e("[Platform Helper] failed to install some resources.");
        }

        if (DeviceUtils.isAppUserRestricted(mContext)) {
            Log.w("[Platform Helper] Device has been restricted by user (Android 9+), push notifications won't work !");
        }

        int bucket = DeviceUtils.getAppStandbyBucket(mContext);
        if (bucket > 0) {
            Log.w("[Platform Helper] Device is in bucket " + DeviceUtils.getAppStandbyBucketNameFromValue(bucket));
        }

        // Update DNS servers lists
        NetworkManagerInterface nm = createNetworkManager();
        nm.updateDnsServers();

        mPushServiceClass = getPushServiceClass();
        if (mPushServiceClass == null) {
            mPushServiceClass = org.linphone.core.tools.service.PushService.class;
        }

        mFileTransferServiceClass = getFileTransferServiceClass();
        if (mFileTransferServiceClass == null) {
            mFileTransferServiceClass = org.linphone.core.tools.service.FileTransferService.class;
        }
        
        // DO NOT ADD A LISTENER ON THE CORE HERE!
        // Wait for onLinphoneCoreStart()

        // Dump some debugging information to the logs
        dumpDeviceInformation();
        dumpLinphoneInformation();
        DeviceUtils.logPreviousCrashesIfAny(mContext); // Android 11 only

        mActivityCallbacks = new ActivityMonitor();
        ((Application) mContext).registerActivityLifecycleCallbacks(mActivityCallbacks);

        PushNotificationUtils.init(mContext);
        if (!PushNotificationUtils.isAvailable(mContext)) {
            Log.w("[Platform Helper] Push notifications aren't available (see push utils log)");
        }
        
        if (isAndroidXMediaAvailable()) {
            mAudioHelper = new AudioHelper(mContext);
        } else {
            Log.w("[Platform Helper] Do you have a dependency on androidx.media:media:1.2.0 or newer?");
        }
        mBluetoothHelper = new BluetoothHelper(mContext);

        mDisplayListener = new DisplayManager.DisplayListener() {
            @Override
            public void onDisplayAdded(int displayId) {
                Log.d("[Platform Helper] Display added: ", displayId);
            }

            @Override
            public void onDisplayChanged(int displayId) {
                Log.d("[Platform Helper] Display changed: ", displayId);
                updateOrientation(displayId);
            }

            @Override
            public void onDisplayRemoved(int displayId) {
                Log.d("[Platform Helper] Display removed: ", displayId);
            }
        };
        mDisplayManager = (DisplayManager) mContext.getSystemService(Context.DISPLAY_SERVICE);
        mDisplayManager.registerDisplayListener(mDisplayListener, mHandler);

        IntentFilter shutdownIntentFilter = new IntentFilter(Intent.ACTION_SHUTDOWN);
        // Without that the broadcast timeout might be reached before we were called
        shutdownIntentFilter.setPriority(IntentFilter.SYSTEM_HIGH_PRIORITY - 1);
        mShutdownReceiver = new ShutdownReceiver();
        Log.i("[Platform Helper] Registering shutdown receiver");
        mContext.registerReceiver(mShutdownReceiver, shutdownIntentFilter);

        mServiceClass = getServiceClass();
        if (mServiceClass == null) mServiceClass = CoreService.class;

        Log.i("[Platform Helper] Initialization done");
    }

    public void onLinphoneCoreStart(boolean monitoringEnabled) {
        Log.i("[Platform Helper] onLinphoneCoreStart, network monitoring is " + monitoringEnabled);
        mMonitoringEnabled = monitoringEnabled;

        if (!isAndroidXMediaAvailable() && mCore.isNativeRingingEnabled()) {
            Log.e("[Platform Helper] Native ringing was enabled but condition isn't met (androidx.media:media dependency), disabling it.");
            mCore.setNativeRingingEnabled(false);
        }

        if (mCore.isAutoIterateEnabled()) {
            // Force the core.iterate() scheduling to a low value to ensure the Core will be ready as quickly as possible
            Log.i("[Platform Helper] Core is starting, scheduling core.iterate() every " + AUTO_ITERATE_TIMER_CORE_START_OR_PUSH_RECEIVED + "ms");
            startAutoIterate(AUTO_ITERATE_TIMER_CORE_START_OR_PUSH_RECEIVED);
            createTimerToResetAutoIterateSchedule();
        } else {
            Log.w("[Platform Helper] Auto core.iterate() isn't enabled, ensure you do it in your application!");
        }

        mListener = new CoreListenerStub() {
            @Override
            public void onFirstCallStarted(Core core) {
                Log.i("[Platform Helper] First call started");
                // Ensure Service is running. It will take care by itself to start as foreground.
                if (!mServiceRunning) {
                    Log.w("[Platform Helper] Service isn't running, let's start it");
                    try {
                        startService();
                    } catch (IllegalStateException ise) {
                        Log.w("[Platform Helper] Failed to start service: ", ise);
                    }
                } else {
                    Log.i("[Platform Helper] Service appears to be running, everything is fine");
                }
            }

            @Override
            public void onLastCallEnded(Core core) {
                Log.i("[Platform Helper] Last call ended");
                if (mAudioHelper == null) return;
                if (core.isNativeRingingEnabled()) {
                    mAudioHelper.stopRinging();
                } else {
                    mAudioHelper.releaseRingingAudioFocus();
                }
                mAudioHelper.releaseCallAudioFocus();
            }

            @Override
            public void onCallStateChanged(Core core, Call call, Call.State state, String message) {
                if (mAudioHelper == null) return;
                if (call.getState() != state) {
                    // This can happen if a listener set earlier than this one in the app automatically accepts an incoming call for example.
                    if (state == Call.State.IncomingReceived && call.getState() == Call.State.IncomingEarlyMedia) {
                        Log.w("[Platform Helper] It seems call was accepted with early-media during the incoming received call state changed, continuing anyway");
                    } else {
                        Log.w("[Platform Helper] Call state changed callback state variable doesn't match current call state, skipping");
                        return;
                    }
                }

                if (state == Call.State.IncomingReceived && core.getCallsNb() == 1) {
                    if (core.isNativeRingingEnabled()) {
                        if (core.getConfig().getInt("sound", "disable_ringing", 0) == 1) {
                            Log.w("[Platform Helper] Ringing was disabled in configuration (disable_ringing item in [sound] section is set to 1)");
                        } else {
                            Log.i("[Platform Helper] Incoming call received, no other call, start ringing");
                            mAudioHelper.startRinging(mContext, core.getRing(), call.getRemoteAddress());
                        }
                    } else {
                        Log.i("[Platform Helper] Incoming call received, no other call, acquire ringing audio focus");
                        mAudioHelper.requestRingingAudioFocus();
                    }
                } else if (state == Call.State.IncomingEarlyMedia && core.getCallsNb() == 1) {
                    if (core.getRingDuringIncomingEarlyMedia()) {
                        Log.i("[Platform Helper] Incoming call is early media and ringing is allowed");
                    } else {
                        if (core.isNativeRingingEnabled()) {
                            Log.w("[Platform Helper] Incoming call is early media and ringing is disabled, stop ringing");
                            mAudioHelper.stopRinging();
                        } else {
                            Log.i("[Platform Helper] Incoming call is early media and ringing is disabled, keep ringing audio focus as sound card will be using RING stream");
                        }
                    }
                } else if (state == Call.State.Connected) {
                    if (call.getDir() == Call.Dir.Incoming && core.isNativeRingingEnabled()) {
                        Log.i("[Platform Helper] Stop incoming call ringing");
                        mAudioHelper.stopRinging();
                    } else {
                        Log.i("[Platform Helper] Stop incoming call ringing audio focus");
                        mAudioHelper.releaseRingingAudioFocus();
                    }
                } else if (state == Call.State.OutgoingInit && core.getCallsNb() == 1) {
                    Log.i("[Platform Helper] Outgoing call in progress, no other call, acquire ringing audio focus for ringback");
                    mAudioHelper.requestRingingAudioFocus();
                } else if (state == Call.State.StreamsRunning) {
                    Log.i("[Platform Helper] Call active, ensure audio focus granted");
                    mAudioHelper.requestCallAudioFocus(false);
                } else if (state == Call.State.Resuming) {
                    Log.i("[Platform Helper] Call resuming, ensure audio focus granted");
                    mAudioHelper.requestCallAudioFocus(false);
                }
            }
        };
        
        mCore.addListener(mListener);

        SharedPreferences sharedPref = mContext.getSharedPreferences("push_notification_storage", Context.MODE_PRIVATE);
        String callId = sharedPref.getString("call-id", "");
        String payload = sharedPref.getString("payload", "");
        if (!callId.isEmpty()) {
            Log.i("[Platform Helper] Push notification information retrieved from storage, Call-ID is [" + callId + "]");
            processPushNotification(callId, payload, true);

            SharedPreferences.Editor editor = sharedPref.edit();
            editor.putString("call-id", "");
            editor.putString("payload", "");
            editor.apply();
            Log.i("[Platform Helper] Push information cleared from storage");
        }
        
        startNetworkMonitoring();
    }

    public void stop() {
        if (mCore != null) {
            Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    Log.i("[Platform Helper] Stopping Core");
                    stopCore(mCore.getNativePointer());
                }
            };
            dispatchOnCoreThread(runnable);
        }
    }

    public void onLinphoneCoreStop() {
        Log.i("[Platform Helper] onLinphoneCoreStop, network monitoring is " + mMonitoringEnabled);

        // The following will prevent a crash if a video view hasn't been set to null before the Core stops
        // The view listener will be called and the call to the native method will result in a crash in the Core accessor in the native PlatformHelper
        setVideoPreviewView(null);
        setVideoRenderingView(null);

        // Cleaning manually the wakelocks in case of unreleased ones (linphone_core_destroy for instance)
        while (mWakeLock.isHeld()) {
            mWakeLock.release();
        }
        while (mWifiLock.isHeld()) {
            mWifiLock.release();
        }
        while (mMcastLock.isHeld()) {
            mMcastLock.release();
        }

        if (mNetworkSignalMonitor != null) {
            mNetworkSignalMonitor.destroy();
        }

        mNativePtr = 0;
        mHandler.removeCallbacksAndMessages(null);
        stopNetworkMonitoring();

        if (mServiceRunning) {
            Log.i("[Platform Helper] Stopping service ", mServiceClass.getName());
            mContext.stopService(new Intent().setClass(mContext, mServiceClass));
        }

        mCore.removeListener(mListener);

        stopAutoIterate();
        stopTimerToResetAutoIterateSchedule();

        mCore = null; // To allow the garbage colletor to free the Core
        Log.i("[Platform Helper] Core released");
    }

    public void destroy() {
        Log.i("[Platform Helper] Destroying");

        if (mActivityCallbacks != null) {
            ((Application) mContext).unregisterActivityLifecycleCallbacks(mActivityCallbacks);
            mActivityCallbacks = null;
        }

        if (mBluetoothHelper != null) {
            mBluetoothHelper.destroy(mContext);
            mBluetoothHelper = null;
        }

        if (mShutdownReceiver != null) {
            Log.i("[Platform Helper] Unregistering shutdown receiver");
            mContext.unregisterReceiver(mShutdownReceiver);
            mShutdownReceiver = null;
        }
        
        if (mAudioHelper != null) {
            mAudioHelper.destroy(mContext);
            mAudioHelper = null;
        }

        if (mDisplayManager != null && mDisplayListener != null) {
            mDisplayManager.unregisterDisplayListener(mDisplayListener);
            mDisplayListener = null;
            mDisplayManager = null;
        }

        if (mHandler != null) {
            mHandler.removeCallbacksAndMessages(null);
            mHandler = null;
        }
        mServiceClass = null;
        mContext = null;
        sInstance = null;
        Log.i("[Platform Helper] Destroyed");
    }

    public synchronized Core getCore() {
        return mCore;
    }

    public void requestWifiSignalStrengthUpdate() {
        if (mNetworkSignalMonitor != null) {
            mNetworkSignalMonitor.updateWifiConnectionSignalStrength();
        }
    }

    public void setSignalInfo(SignalType type, SignalStrengthUnit unit, int value, String details) {
        setSignalInfo(mNativePtr, type.toInt(), unit.toInt(), value, details);
    }

    public void onWifiOnlyEnabled(boolean enabled) {
        mWifiOnly = enabled;
        Log.i("[Platform Helper] Wifi only mode is now " + (mWifiOnly ? "enabled" : "disabled"));
        if (mNetworkManager != null) {
            mNetworkManager.setWifiOnly(mWifiOnly);
        }
        updateNetworkReachability();
    }

    public synchronized Object getPowerManager() {
        return mPowerManager;
    }

    public synchronized String[] getDnsServers() {
        String[] dnsServers = new String[mDnsServersList.size()];
        mDnsServersList.toArray(dnsServers);
        return dnsServers;
    }

    public static String getDataPath(Context context) {
        return context.getFilesDir().getAbsolutePath() + "/";
    }

    public static String getConfigPath(Context context) {
        return context.getFilesDir().getAbsolutePath() + "/";
    }

    public String getCachePath() {
        return mContext.getCacheDir().getAbsolutePath() + "/";
    }

    public static String getDownloadPath(Context context) {
        String path = null;
        if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
            Log.i("[Platform Helper] External storage is mounted, using download directory");
            path = context.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS).getAbsolutePath();
        }

        if (path == null) {
            Log.w("[Platform Helper] Couldn't get external storage path, using internal");
            path = context.getFilesDir().getAbsolutePath();
        }

        Log.i("[Platform Helper] Download directory is " + path + "/");
        return path + "/";
    }

    public String getNativeLibraryDir() {
        ApplicationInfo info = mContext.getApplicationInfo();
        return info.nativeLibraryDir;
    }

    public void acquireWifiLock() {
        mTempCountWifi++;
        Log.d("[Platform Helper] acquireWifiLock(). count = " + mTempCountWifi);
        if (!mWifiLock.isHeld()) {
            mWifiLock.acquire();
        }
    }

    public void releaseWifiLock() {
        mTempCountWifi--;
        Log.d("[Platform Helper] releaseWifiLock(). count = " + mTempCountWifi);
        if (mWifiLock.isHeld()) {
            mWifiLock.release();
        }
    }

    public void acquireMcastLock() {
        mTempCountMCast++;
        Log.d("[Platform Helper] acquireMcastLock(). count = " + mTempCountMCast);
        if (!mMcastLock.isHeld()) {
            mMcastLock.acquire();
        }
    }

    public void releaseMcastLock() {
        mTempCountMCast--;
        Log.d("[Platform Helper] releaseMcastLock(). count = " + mTempCountMCast);
        if (mMcastLock.isHeld()) {
            mMcastLock.release();
        }
    }

    public void acquireCpuLock() {
        mTempCountCPU++;
        Log.d("[Platform Helper] acquireCpuLock(). count = " + mTempCountCPU);
        if (!mWakeLock.isHeld()) {
            mWakeLock.acquire();
        }
    }

    public void releaseCpuLock() {
        mTempCountCPU--;
        Log.d("[Platform Helper] releaseCpuLock(). count = " + mTempCountCPU);
        if (mWakeLock.isHeld()) {
            mWakeLock.release();
        }

    }

    private int getResourceIdentifierFromName(String name) {
        int resId = mResources.getIdentifier(name, "raw", mContext.getPackageName());
        if (resId == 0) {
            Log.d("[Platform Helper] App doesn't seem to embed resource " + name + " in it's res/raw/ directory, use linphone's instead");
            resId = mResources.getIdentifier(name, "raw", "org.linphone");
            if (resId == 0) {
                Log.d("[Platform Helper] App doesn't seem to embed resource " + name + " in it's res/raw/ directory. Make sure this file is either brought as an asset or a resource");
            }
        }
        return resId;
    }

    private void copyAssetsFromPackage() throws IOException {
        Log.i("[Platform Helper] Starting copy from assets to application files directory");
        copyAssetsFromPackage(mContext, "org.linphone.core", ".");
        Log.i("[Platform Helper] Copy from assets done");

        if (getResourceIdentifierFromName("cpim_grammar") != 0) {
            copyLegacyAssets();
        }
    }

    private void copyLegacyAssets() throws IOException {
        Log.i("[Platform Helper] Starting to copy legacy assets");
        /*legacy code for 3.X*/
        String basePath = mContext.getFilesDir().getAbsolutePath();
        //make sure to follow same path as unix version of the sdk
        String mLinphoneRootCaFile = basePath + "/share/linphone/rootca.pem";
        String mRingSoundFile = basePath + "/share/sounds/linphone/rings/notes_of_the_optimistic.mkv";
        String mRingbackSoundFile = basePath + "/share/sounds/linphone/ringback.wav";
        String mPauseSoundFile = basePath + "/share/sounds/linphone/rings/dont_wait_too_long.mkv";
        String mErrorToneFile = basePath + "/share/sounds/linphone/incoming_chat.wav";
        String mGrammarCpimFile = basePath + "/share/belr/grammars/cpim_grammar.belr";
        String mGrammarIcsFile = basePath + "/share/belr/grammars/ics_grammar.belr";
        String mGrammarIdentityFile = basePath + "/share/belr/grammars/identity_grammar.belr";
        String mGrammarMwiFile = basePath + "/share/belr/grammars/mwi_grammar.belr";
        String mGrammarVcardFile = basePath + "/share/belr/grammars/vcard_grammar.belr";

        copyEvenIfExists(getResourceIdentifierFromName("cpim_grammar"), mGrammarCpimFile);
        copyEvenIfExists(getResourceIdentifierFromName("ics_grammar"), mGrammarIcsFile);
        copyEvenIfExists(getResourceIdentifierFromName("identity_grammar"), mGrammarIdentityFile);
        copyEvenIfExists(getResourceIdentifierFromName("mwi_grammar"), mGrammarMwiFile);
        copyEvenIfExists(getResourceIdentifierFromName("vcard_grammar"), mGrammarVcardFile);
        copyEvenIfExists(getResourceIdentifierFromName("rootca"), mLinphoneRootCaFile);
        copyIfNotExist(getResourceIdentifierFromName("notes_of_the_optimistic"), mRingSoundFile);
        copyIfNotExist(getResourceIdentifierFromName("ringback"), mRingbackSoundFile);
        copyIfNotExist(getResourceIdentifierFromName("hold"), mPauseSoundFile);
        copyIfNotExist(getResourceIdentifierFromName("incoming_chat"), mErrorToneFile);
        Log.i("[Platform Helper] Copy from legacy resources done");
    }

    public void copyEvenIfExists(int ressourceId, String target) throws IOException {
        File lFileToCopy = new File(target);
        copyFromPackage(ressourceId, lFileToCopy);
    }

    public void copyIfNotExist(int ressourceId, String target) throws IOException {
        File lFileToCopy = new File(target);
        if (!lFileToCopy.exists()) {
            copyFromPackage(ressourceId, lFileToCopy);
        }
    }

    public void copyFromPackage(int ressourceId, File target) throws IOException {
        if (ressourceId == 0) {
            Log.i("[Platform Helper] Resource identifier null for target [" + target.getName() + "]");
            return;
        }
        if (!target.getParentFile().exists())
            target.getParentFile().mkdirs();

        InputStream lInputStream = mResources.openRawResource(ressourceId);
        FileOutputStream lOutputStream = new FileOutputStream(target);
        int readByte;
        byte[] buff = new byte[8048];
        while ((readByte = lInputStream.read(buff)) != -1) {
            lOutputStream.write(buff, 0, readByte);
        }
        lOutputStream.flush();
        lOutputStream.close();
        lInputStream.close();
    }

    public static void copyAssetsFromPackage(Context ctx, String fromPath, String toPath) throws IOException {
        File asset = new File(ctx.getFilesDir().getPath() + "/" + toPath);
        asset.mkdir();

        for (String f : ctx.getAssets().list(fromPath)) {
            String current_name = fromPath + "/" + f;
            String current_dest = toPath + "/" + f;
            File file = new File(ctx.getFilesDir().getPath() + "/" + current_dest);
            InputStream lInputStream;

            try {
                if (file.exists() && (f.endsWith(".wav") || f.endsWith(".mkv"))) {
                    Log.i("[Platform Helper] Resource " + f + " already installed, skipping...");
                    continue;
                }
                lInputStream = ctx.getAssets().open(current_name);
            } catch (IOException e) {
                //probably a dir
                copyAssetsFromPackage(ctx, current_name, current_dest);
                continue;
            }

            Log.i("[Platform Helper] Installing resource [" + f + "] to [" + file.getAbsolutePath() + "]");
            FileOutputStream lOutputStream = new FileOutputStream(file);
            int readByte;
            byte[] buff = new byte[8048];
            while ((readByte = lInputStream.read(buff)) != -1) {
                lOutputStream.write(buff, 0, readByte);
            }
            lOutputStream.flush();
            lOutputStream.close();
            lInputStream.close();
        }
    }

    public void setVideoPreviewView(Object view) {
        if (mPreviewTextureView != null) {
            Log.w("[Platform Helper] Found an existing preview TextureView, let's destroy it first");
            mPreviewTextureView.setSurfaceTextureListener(null);
            mPreviewTextureView = null;
        }

        if (view == null) {
            Log.i("[Platform Helper] Preview window surface set to null");
            setNativePreviewWindowId(mNativePtr, null);
            return;
        }

        if (view instanceof Surface) {
            Log.i("[Platform Helper] Preview window surface is a Surface");
            Surface surface = (Surface) view;
            setNativePreviewWindowId(mNativePtr, surface);
            return;
        }

        if (view instanceof SurfaceTexture) {
            Log.w("[Platform Helper] Preview window surface is a SurfaceTexture, rotation may be broken, prefer passing CaptureTextureView directly");
            SurfaceTexture surface = (SurfaceTexture) view;
            setNativePreviewWindowId(mNativePtr, surface);
            return;
        }

        if (!(view instanceof TextureView)) {
            throw new RuntimeException("[Platform Helper] Preview window id is not an instance of TextureView, Surface or SurfaceTexture. " +
                    "Please update your UI layer so that the preview video view is one of the above types (or an instance of it) " +
                    "or enable compatibility mode by setting displaytype=MSAndroidOpenGLDisplay in the [video] section your linphonerc factory configuration file " +
                    "so you can keep using your existing application code for managing video views.");
        }

        mPreviewTextureView = (TextureView) view;
        mPreviewTextureView.setSurfaceTextureListener(new TextureView.SurfaceTextureListener() {
            @Override
            public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
                Log.i("[Platform Helper] Preview window surface texture [" + surface + "] is available for texture view [" + mPreviewTextureView + "]");
                Runnable runnable = new Runnable() {
                    @Override
                    public void run() {
                        setNativePreviewWindowId(mNativePtr, mPreviewTextureView);
                    }
                };
                mHandler.post(runnable);
            }

            @Override
            public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
                Log.i("[Platform Helper] Preview surface texture [" + surface + "] size changed: " + width + "x" + height);
            }

            @Override
            public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
                Log.i("[Platform Helper] Preview surface texture [" + surface + "] destroyed");

                if (mNativePtr != 0 && mPreviewTextureView != null) {
                    if (surface.equals(mPreviewTextureView.getSurfaceTexture())) {
                        Log.i("[Platform Helper] Current preview surface texture is no longer available");
                        Runnable runnable = new Runnable() {
                            @Override
                            public void run() {
                                mPreviewTextureView = null;
                                setNativePreviewWindowId(mNativePtr, null);
                            }
                        };
                        mHandler.post(runnable);
                    }
                }

                if (!DeviceUtils.isSurfaceTextureReleased(surface)) {
                    Log.i("[Platform Helper] Releasing preview window surface texture [" + surface + "]");
                    surface.release();
                }
                return true;
            }

            @Override
            public void onSurfaceTextureUpdated(SurfaceTexture surface) {
                Log.d("[Platform Helper] Preview surface texture [" + surface + "] has been updated");
            }
        });

        if (mPreviewTextureView.isAvailable()) {
            Log.i("[Platform Helper] Preview window surface is directly available for texture view [" + mPreviewTextureView + "]");
            setNativePreviewWindowId(mNativePtr, mPreviewTextureView);
        }
    }

    public void setVideoRenderingView(Object view) {
        if (mVideoTextureView != null) {
            Log.w("[Platform Helper] Found an existing video TextureView [", mVideoTextureView, "], let's destroy it first");
            mVideoTextureView.setSurfaceTextureListener(null);
            mVideoTextureView = null;
        }

        if (view == null) {
            Log.i("[Platform Helper] Video window surface set to null");
            setNativeVideoWindowId(mNativePtr, null);
            return;
        }

        if (view instanceof Surface) {
            Log.i("[Platform Helper] Video window surface is a Surface");
            Surface surface = (Surface) view;
            setNativeVideoWindowId(mNativePtr, surface);
            return;
        }

        if (view instanceof SurfaceTexture) {
            Log.i("[Platform Helper] Video window surface is a SurfaceTexture");
            SurfaceTexture surface = (SurfaceTexture) view;
            setNativeVideoWindowId(mNativePtr, surface);
            return;
        }

        if (!(view instanceof TextureView)) {
            throw new RuntimeException("[Platform Helper] Rendering window id is not an instance of TextureView, Surface or SurfaceTexture. " +
                    "Please update your UI layer so that the video rendering view is an object of any above types (or an instance of it) " +
                    "or enable compatibility mode by setting displaytype=MSAndroidOpenGLDisplay in the [video] section your linphonerc factory configuration file " +
                    "so you can keep using your existing application code for managing video views.");
        }

        mVideoTextureView = (TextureView) view;
        mVideoTextureView.setSurfaceTextureListener(new TextureView.SurfaceTextureListener() {
            @Override
            public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
                Log.i("[Platform Helper] Rendering window surface texture [" + surface + "] is available for texture view [" + mVideoTextureView + "]");
                Runnable runnable = new Runnable() {
                    @Override
                    public void run() {
                        setNativeVideoWindowId(mNativePtr, surface);
                    }
                };
                mHandler.post(runnable);
            }

            @Override
            public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
                Log.i("[Platform Helper] Surface texture [" + surface + "] size changed: " + width + "x" + height);
            }

            @Override
            public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
                Log.i("[Platform Helper] Rendering surface texture [" + surface + "] destroyed");

                if (mNativePtr != 0 && mVideoTextureView != null) {
                    if (surface.equals(mVideoTextureView.getSurfaceTexture())) {
                        Log.i("[Platform Helper] Current rendering surface texture is no longer available");
                        Runnable runnable = new Runnable() {
                            @Override
                            public void run() {
                                mVideoTextureView = null;
                                setNativeVideoWindowId(mNativePtr, null);
                            }
                        };
                        mHandler.post(runnable);
                    }
                }

                if (!DeviceUtils.isSurfaceTextureReleased(surface)) {
                    Log.i("[Platform Helper] Releasing window surface texture [" + surface + "]");
                    surface.release();
                }

                return true;
            }

            @Override
            public void onSurfaceTextureUpdated(SurfaceTexture surface) {
                Log.d("[Platform Helper] Surface texture [" + surface + "] has been updated");
            }
        });

        if (mVideoTextureView.isAvailable()) {
            Log.i("[Platform Helper] Rendering window surface is directly available for texture view [" + mVideoTextureView + "]");
            setNativeVideoWindowId(mNativePtr, mVideoTextureView.getSurfaceTexture());
        }
    }

    public void setParticipantDeviceVideoRenderingView(long participantDevice, Object view) {
        if (mParticipantTextureView == null) {
            mParticipantTextureView = new HashMap<Long, TextureView>();
        }

        if (mParticipantTextureView.containsKey(participantDevice)) {
            Log.w("[Platform Helper] Found existing TextureView [" + mParticipantTextureView.get(participantDevice) + "] for participant device, let's destroy it first");
            mParticipantTextureView.get(participantDevice).setSurfaceTextureListener(null);
            mParticipantTextureView.remove(participantDevice);
        }

        if (view == null) {
            Log.i("[Platform Helper] Participant device video window surface set to null");
            setParticipantDeviceNativeVideoWindowId(mNativePtr, participantDevice, null);
            return;
        }

        if (view instanceof Surface) {
            Log.i("[Platform Helper] Participant device video window surface is a Surface");
            Surface surface = (Surface) view;
            setParticipantDeviceNativeVideoWindowId(mNativePtr, participantDevice, surface);
            return;
        }

        if (view instanceof SurfaceTexture) {
            Log.i("[Platform Helper] Participant device video window surface is a SurfaceTexture");
            SurfaceTexture surface = (SurfaceTexture) view;
            setParticipantDeviceNativeVideoWindowId(mNativePtr, participantDevice, surface);
            return;
        }

        if (!(view instanceof TextureView)) {
            throw new RuntimeException("[Platform Helper] Rendering window id is not an instance of TextureView, Surface or SurfaceTexture. " +
                    "Please update your UI layer so that the video rendering view is an object of any above types (or an instance of it) " +
                    "or enable compatibility mode by setting displaytype=MSAndroidOpenGLDisplay in the [video] section your linphonerc factory configuration file " +
                    "so you can keep using your existing application code for managing video views.");
        }

        TextureView textureView = (TextureView) view;
        textureView.setSurfaceTextureListener(new TextureView.SurfaceTextureListener() {
            @Override
            public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
               Log.i("[Platform Helper] Rendering participant device's window surface texture [" + surface + "] is available for texture view [" + textureView + "]");
               Runnable runnable = new Runnable() {
                    @Override
                    public void run() {
                        setParticipantDeviceNativeVideoWindowId(mNativePtr, participantDevice, surface);
                    }
                };
                mHandler.post(runnable);
            }

            @Override
            public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
                Log.i("[Platform Helper] Surface texture [" + surface + "] of participant device size changed: " + width + "x" + height);
            }

            @Override
            public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
                Log.w("[Platform Helper] TextureView [" + surface + "] for participant device has been destroyed");
                Runnable runnable = new Runnable() {
                    @Override
                    public void run() {
                        mParticipantTextureView.remove(participantDevice);
                        setParticipantDeviceNativeVideoWindowId(mNativePtr, participantDevice, null);
                    }
                };
                mHandler.post(runnable);
                return true;
            }

            public void onSurfaceTextureUpdated(SurfaceTexture surface) {
                Log.d("[Platform Helper] Surface texture [" + surface + "] of participant device has been updated");
            }
        });
        mParticipantTextureView.put(participantDevice, textureView);

        if (textureView.isAvailable()) {
            Log.i("[Platform Helper] Rendering participant device window surface is directly available for texture view [" + textureView + "]");
            setParticipantDeviceNativeVideoWindowId(mNativePtr, participantDevice, textureView.getSurfaceTexture());
        } else {
            Log.i("[Platform Helper] Rendering participant device window surface [" + textureView.getSurfaceTexture() + "] of texture [" + textureView + "] is not available !");
        }
    }

    public void resizeVideoPreview(int width, int height) {
        if (mPreviewTextureView != null && mPreviewTextureView instanceof CaptureTextureView) {
            Log.i("[Platform Helper] Found CaptureTextureView, setting video capture size to " + width + "x" + height);
            ((CaptureTextureView) mPreviewTextureView).setAspectRatio(width, height);
        } else if (mPreviewTextureView != null) {
            Log.w("[Platform Helper] It seems you are using a TextureView instead of our CaptureTextureView, we strongly advise you to use ours to benefit from correct rotation & ratio");
        } else {
            Log.w("[Platform Helper] No preview surface found, nothing to resize!");
        }
    }

    public synchronized Handler getHandler() {
        return mHandler;
    }

    public synchronized boolean isInBackground() {
        if (mServiceRunningInForeground) {
            Log.i("[Platform Helper] CoreService seems to be running as foreground, consider app is in foreground");
            return false;
        }

        return isInBackground(mNativePtr);
    }

    private String networkTypeToString(int type) {
        switch (type) {
            case ConnectivityManager.TYPE_BLUETOOTH:
                return "BLUETOOTH";
            case ConnectivityManager.TYPE_ETHERNET:
                return "ETHERNET";
            case ConnectivityManager.TYPE_MOBILE:
                return "MOBILE";
            case ConnectivityManager.TYPE_WIFI:
                return "WIFI";
            case ConnectivityManager.TYPE_VPN:
                return "VPN";
            default:
                return String.valueOf(type);
        }
    }

    public void updateDnsServers(ArrayList<String> dnsServers) {
        // Do not replace previous list of DNS by an empty one, keep them to be able to try a resolution while in DOZE mode
        if (dnsServers != null && !dnsServers.isEmpty()) {
            if (!dnsServers.equals(mDnsServersList)) {
                mDnsServersList = dnsServers;
                Log.i("[Platform Helper] DNS servers list updated, notifying Core");
                setDnsServers(mNativePtr);
            } else {
                Log.i("[Platform Helper] DNS servers list hasn't changed, doing nothing");
            }
        }
    }

    public synchronized NetworkInfo getActiveNetworkInfo() {
        if (mNetworkManager == null) {
            Log.w("[Platform Helper] Network Manager is null, won't be able to detect active network type");
            return null;
        }

        NetworkInfo networkInfo = mNetworkManager.getActiveNetworkInfo();
        return networkInfo;
    }

    public synchronized boolean isActiveNetworkWifiOnlyCompliant() {
        if (mNetworkManager == null) {
            Log.w("[Platform Helper] Network Manager is null, assuming network isn't WiFi only compliant");
            return false;
        }

        NetworkInfo networkInfo = mNetworkManager.getActiveNetworkInfo();
        if (networkInfo != null) {
            Log.i("[Platform Helper] Active network type is " + networkInfo.getTypeName());
            if (networkInfo.getType() == ConnectivityManager.TYPE_WIFI || networkInfo.getType() == ConnectivityManager.TYPE_ETHERNET) {
                return true;
            }
        } else {
            Log.w("[Platform Helper] Active network info is null, assuming network isn't WiFi only compliant");
        }
        return false;
    }

    public void updateNetworkReachability() {
        if (mNativePtr == 0) {
            Log.w("[Platform Helper] Native pointer has been reset, stopping there");
            return;
        }

        if (mNetworkManager == null) {
            Log.w("[Platform Helper] Network Manager is null, stopping there");
            return;
        }

        boolean connected = mNetworkManager.isCurrentlyConnected(mContext);
        if (!connected) {
            Log.i("[Platform Helper] No connectivity: setting network unreachable");
            setNetworkReachable(mNativePtr, false);
            return;
        }
        
        if (mNetworkManager.hasHttpProxy(mContext)) {
            if (useSystemHttpProxy(mNativePtr)) {
                String host = mNetworkManager.getProxyHost(mContext);
                int port = mNetworkManager.getProxyPort(mContext);
                setHttpProxy(mNativePtr, host, port);
                if (!mUsingHttpProxy) {
                    Log.i("[Platform Helper] Proxy wasn't set before, disabling network reachability first");
                    setNetworkReachable(mNativePtr, false);
                }
                mUsingHttpProxy = true;
            } else {
                Log.w("[Platform Helper] Proxy available but forbidden by linphone core [sip] use_system_http_proxy setting");
            }
        } else {
            setHttpProxy(mNativePtr, "", 0);
            if (mUsingHttpProxy) {
                Log.i("[Platform Helper] Proxy was set before, disabling network reachability first");
                setNetworkReachable(mNativePtr, false);
            }
            mUsingHttpProxy = false;
        }

        NetworkInfo networkInfo = mNetworkManager.getActiveNetworkInfo();
        if (networkInfo == null) {
            Log.e("[Platform Helper] getActiveNetworkInfo() returned null !");
            setNetworkReachable(mNativePtr, false);
            return;
        }

        Log.i("[Platform Helper] Active network type is " + networkInfo.getTypeName() + ", state " + networkInfo.getState() + " / " + networkInfo.getDetailedState());
        if (networkInfo.getState() == NetworkInfo.State.DISCONNECTED && networkInfo.getDetailedState() == NetworkInfo.DetailedState.BLOCKED) {
            Log.w("[Platform Helper] Active network is in bad state...");
        }

        // Update DNS servers lists
        mNetworkManager.updateDnsServers();

        int currentNetworkType = networkInfo.getType();
        if (mLastNetworkType != -1 && mLastNetworkType != currentNetworkType) {
            Log.i("[Platform Helper] Network type has changed (last one was " + networkTypeToString(mLastNetworkType) + "), disabling network reachability first");
            setNetworkReachable(mNativePtr, false);

            // When switching from WiFi to Mobile, update Cell signal strength if possible
            if (mLastNetworkType == ConnectivityManager.TYPE_WIFI) {
                if (mNetworkSignalMonitor != null) {
                    mNetworkSignalMonitor.updateCellConnectionSignalStrength();
                }
            }
        }

        mLastNetworkType = currentNetworkType;
        Log.i("[Platform Helper] Network reachability enabled");
        setNetworkReachable(mNativePtr, true);
    }

    private NetworkManagerInterface createNetworkManager() {
        NetworkManagerInterface networkManager = null;
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
            networkManager = new NetworkManager(this, mConnectivityManager, mWifiOnly);
        } else if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
            networkManager = new NetworkManagerAbove21(this, mConnectivityManager, mWifiOnly);
        } else if (Build.VERSION.SDK_INT < Build.VERSION_CODES.N) {
            networkManager = new NetworkManagerAbove23(this, mConnectivityManager, mWifiOnly);
        } else if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) {
            networkManager = new NetworkManagerAbove24(this, mConnectivityManager, mWifiOnly);
        } else {
            networkManager = new NetworkManagerAbove26(this, mConnectivityManager, mWifiOnly);
        }
        return networkManager;
    }

    private void startNetworkMonitoring() {
        if (!mMonitoringEnabled) return;

        mNetworkManager = createNetworkManager();
        Log.i("[Platform Helper] Registering network callbacks");
        mNetworkManager.registerNetworkCallbacks(mContext);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            IntentFilter dozeIntentFilter = new IntentFilter(PowerManager.ACTION_DEVICE_IDLE_MODE_CHANGED);
            mDozeReceiver = new DozeReceiver();
            Log.i("[Platform Helper] Registering doze receiver");
            mContext.registerReceiver(mDozeReceiver, dozeIntentFilter);
        }

        mInteractivityReceiver = new InteractivityReceiver();
        IntentFilter interactivityIntentFilter = new IntentFilter(Intent.ACTION_SCREEN_ON);
        interactivityIntentFilter.addAction(Intent.ACTION_SCREEN_OFF);
        Log.i("[Platform Helper] Registering interactivity receiver");
        mContext.registerReceiver(mInteractivityReceiver, interactivityIntentFilter);

        if (Version.sdkAboveOrEqual(Version.API29_ANDROID_10)) {
            mNetworkSignalMonitor = new NetworkSignalMonitor(mContext, this);
        } else {
            Log.w("[Platform Helper] Device is running Android < 10, can't use network signal strength monitoring");
        }

        updateNetworkReachability();
    }

    private void stopNetworkMonitoring() {
        if (mInteractivityReceiver != null) {
            Log.i("[Platform Helper] Unregistering interactivity receiver");
            mContext.unregisterReceiver(mInteractivityReceiver);
            mInteractivityReceiver = null;
        }

        if (mNetworkManager != null && mConnectivityManager != null) {
            Log.i("[Platform Helper] Unregistering network callbacks");
            mNetworkManager.unregisterNetworkCallbacks(mContext);
            mNetworkManager = null;
        }

        if (mDozeReceiver != null) {
            Log.i("[Platform Helper] Unregistering doze receiver");
            mContext.unregisterReceiver(mDozeReceiver);
            mDozeReceiver = null;
        }

        mMonitoringEnabled = false;
    }

    public void disableAudioRouteChanges(boolean disable) {
        if (disable) {
            Log.i("[Platform Helper] Disabling audio route changes in mediastreamer2");
        } else {
            Log.i("[Platform Helper] Enabling audio route changes in mediastreamer2");
        }
        MediastreamerAndroidContext.disableAudioRouteChanges(disable);
    }

    public void startPushService() {
        boolean connected = false;
        if (mNetworkManager != null) {
            connected = mNetworkManager.isCurrentlyConnected(mContext);
        } else {
            Log.w("[Platform Helper] Network Manager isn't available yet, assuming network is un-reachable just in case");
        }

        if (!connected) {
            Log.i("[Platform Helper] Push has been received but network seems unreachable, starting foreground push service");
            Intent i = new Intent(mContext, mPushServiceClass);
            DeviceUtils.startForegroundService(mContext, i);
            mPushServiceStarted = true;
        }
    }

    public void stopPushService() {
        if (mPushServiceStarted) {
            Log.i("[Platform Helper] Foreground push service is no longer required");
            Intent i = new Intent(mContext, mPushServiceClass);
            mContext.stopService(i);
            mPushServiceStarted = false;
        }
    }

    public void startFileTransferService() {
        if (!mFileTransferServiceStarted) {
            Log.i("[Platform Helper] Starting foreground file transfer service");
            Intent i = new Intent(mContext, mFileTransferServiceClass);
            DeviceUtils.startForegroundService(mContext, i);
            mFileTransferServiceNotificationStarted = false;
            mFileTransferServiceStopPending = false;
            mFileTransferServiceStarted = true;
        }
    }

    public void setFileTransferServiceNotificationStarted() {
        mFileTransferServiceNotificationStarted = true;
        Log.i("[Platform Helper] File transfer service notification was dispatched");

        if (mFileTransferServiceStopPending) {
            Log.i("[Platform Helper] File transfer service can now be stopped, doing it");
            mFileTransferServiceStopPending = false;
            stopFileTransferService();
        }
    }

    public void stopFileTransferService() {
        if (mFileTransferServiceStarted) {
            Log.i("[Platform Helper] Foreground file transfer service is no longer required");
            if (mFileTransferServiceNotificationStarted) {
                Intent i = new Intent(mContext, mFileTransferServiceClass);
                mContext.stopService(i);
                mFileTransferServiceNotificationStarted = false;
                mFileTransferServiceStopPending = false;
                mFileTransferServiceStarted = false;
                Log.i("[Platform Helper] Foreground file transfer service stopped");
            } else {
                mFileTransferServiceStopPending = true;
                Log.w("[Platform Helper] Trying to stop a foreground service for which notification wasn't dispatched yet, waiting...");
            }
        }
    }

    // Core thread may be the same as UI thread id Core was created from UI thread
    public void dispatchOnCoreThread(Runnable r) {
        if (mHandler != null) {
            mHandler.post(r);
        }
    }

    // Core thread may be the same as UI thread id Core was created from UI thread
    public void dispatchOnCoreThreadAfter(Runnable r, long after) {
        if (mHandler != null) {
            mHandler.postDelayed(r, after);
        }
    }

    public void processPushNotification(String callId, String payload, boolean isCoreStarting) {
        Log.i("[Platform Helper] Acquiring platform helper push notification wakelock");
        WakeLock wakeLock = mPowerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "Push Notification Processing");
        wakeLock.acquire(20000L);
        
        if (mCore.isAutoIterateEnabled() && mCore.isInBackground()) {
            // Force the core.iterate() scheduling to a low value to ensure the Core will process what triggered the push notification as quickly as possible
            Log.i("[Platform Helper] Push notification received, scheduling core.iterate() every " + AUTO_ITERATE_TIMER_CORE_START_OR_PUSH_RECEIVED + "ms");
            startAutoIterate(AUTO_ITERATE_TIMER_CORE_START_OR_PUSH_RECEIVED);
            createTimerToResetAutoIterateSchedule();
        }

        Log.i("[Platform Helper] Notifying Core a push with Call-ID [" + callId + "] has been received");
        processPushNotification(mCore.getNativePointer(), callId, payload, isCoreStarting);

        Log.i("[Platform Helper] Releasing platform helper push notification wakelock");
        wakeLock.release();
    }

    public void startAutoIterate() {
        if (mCore == null) return;
        
        if (mCore.isAutoIterateEnabled()) {
            if (mTimer != null) {
                Log.w("[Platform Helper] core.iterate() scheduling is already active");
                return;
            }

            if (mCore.isInBackground()) {
                Log.i("[Platform Helper] Start core.iterate() scheduling with background timer");
                startAutoIterate(mCore.getAutoIterateBackgroundSchedule());
            } else {
                Log.i("[Platform Helper] Start core.iterate() scheduling with foreground timer");
                startAutoIterate(mCore.getAutoIterateForegroundSchedule());
            }
        }
    }

    private void stopTimerToResetAutoIterateSchedule() {
        if (mForcedIterateTimer != null) {
            mForcedIterateTimer.cancel();
            mForcedIterateTimer.purge();
            mForcedIterateTimer = null;
        }
    }

    private void createTimerToResetAutoIterateSchedule() {
        // When we force the scheduling of the core.iterate(), reset it to the proper value (depending on background state) after a few seconds
        stopTimerToResetAutoIterateSchedule();

        TimerTask lTask =
            new TimerTask() {
                @Override
                public void run() {
                    Runnable resetRunnable = new Runnable() {
                        @Override
                        public void run() {
                            Log.i("[Platform Helper] Resetting core.iterate() schedule depending on background/foreground state");
                            stopAutoIterate();
                            startAutoIterate();
                        }
                    };
                    dispatchOnCoreThread(resetRunnable);
                }
            };

        mForcedIterateTimer = new Timer("Linphone core.iterate() reset scheduler");
        mForcedIterateTimer.schedule(lTask, AUTO_ITERATE_TIMER_RESET_AFTER);
        Log.i("[Platform Helper] Iterate scheduler will be reset in " + (AUTO_ITERATE_TIMER_RESET_AFTER / 1000) + " seconds");
    }

    private void startAutoIterate(int schedule) {
        if (mTimer != null && schedule == mIterateSchedule) {
            Log.i("[Platform Helper] core.iterate() is already scheduled every " + schedule + " ms");
            return;
        }

        stopAutoIterate();

        mIterateSchedule = schedule;

        mIterateRunnable = new Runnable() {
            @Override
            public void run() {
                if (mCore != null) {
                    mCore.iterate();
                }
            }
        };
        TimerTask lTask = new TimerTask() {
            @Override
            public void run() {
                dispatchOnCoreThread(mIterateRunnable);
            }
        };

        /*use schedule instead of scheduleAtFixedRate to avoid iterate from being call in burst after cpu wake up*/
        mTimer = new Timer("Linphone core.iterate() scheduler");
        mTimer.schedule(lTask, 0, mIterateSchedule);
        Log.i("[Platform Helper] Call to core.iterate() scheduled every " + mIterateSchedule + " ms");
    }

    public void stopAutoIterate() {
        if (mTimer != null) {
            Log.i("[Platform Helper] Stopping scheduling of core.iterate() every " + mIterateSchedule + " ms");
            mTimer.cancel();
            mTimer.purge();
            mTimer = null;
            Log.i("[Platform Helper] core.iterate() scheduler stopped");
        } else {
            Log.w("[Platform Helper] core.iterate() scheduling wasn't started or already stopped");
        }
    }

    public void healNetwork() {
        if (mCore != null) {
            Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    healNetworkConnections(mCore.getNativePointer());
                }
            };
            dispatchOnCoreThread(runnable);
        }
    }

    public void startAudioForEchoTestOrCalibration() {
        if (mAudioHelper == null) return;
        mAudioHelper.startAudioForEchoTestOrCalibration();
    }

    public void stopAudioForEchoTestOrCalibration() {
        if (mAudioHelper == null) return;
        mAudioHelper.stopAudioForEchoTestOrCalibration();
    }

    public void routeAudioToSpeaker () {
        if (mAudioHelper == null) return;
        mAudioHelper.routeAudioToSpeaker();
    }

    public void restorePreviousAudioRoute() {
        if (mAudioHelper == null) return;
        mAudioHelper.restorePreviousAudioRoute();
    }

    public void onAudioFocusLost() {
        if (mCore != null) {
            boolean pauseCallsWhenAudioFocusIsLost = mCore.getConfig().getBool("audio", "android_pause_calls_when_audio_focus_lost", true);
            if (pauseCallsWhenAudioFocusIsLost) {
                if (mCore.isInConference()) {
                    Log.i("[Platform Helper] App has lost audio focus, leaving conference");
                    leaveConference(mCore.getNativePointer());
                } else {
                    Log.i("[Platform Helper] App has lost audio focus, pausing all calls");
                    pauseAllCalls(mCore.getNativePointer());
                }
                mAudioHelper.releaseCallAudioFocus();
            } else {
                Log.w("[Platform Helper] Audio focus lost but keeping calls running");
            }
        }
    }

    public void onBluetoothAdapterTurnedOn() {
        if (DeviceUtils.isBluetoothConnectPermissionGranted(mContext)) {
            onBluetoothHeadsetStateChanged();
        } else {
            Log.w("[Platform Helper] Bluetooth Connect permission isn't granted, waiting longer before reloading sound devices to increase chances to get bluetooth device");
            onBluetoothHeadsetStateChanged(5000);
        }
    }

    public void onBluetoothHeadsetStateChanged() {
        onBluetoothHeadsetStateChanged(500);
    }

    private void onBluetoothHeadsetStateChanged(int delay) {
        if (mCore != null) {
            if (mCore.getConfig().getInt("audio", "android_monitor_audio_devices", 1) == 0) return;
            
            GlobalState globalState = mCore.getGlobalState();
            if (globalState == GlobalState.On || globalState == GlobalState.Ready) {
                Log.i("[Platform Helper] Bluetooth headset state changed, waiting for " + delay + " ms before reloading sound devices");
                if (mReloadSoundDevicesScheduled) {
                    Log.w("[Platform Helper] Sound devices reload is already pending, skipping...");
                    return;
                }
                mReloadSoundDevicesScheduled = true;

            
                Runnable reloadRunnable = new Runnable() {
                    @Override
                    public void run() {
                        Log.i("[Platform Helper] Reloading sound devices");
                        if (mCore != null) {
                            reloadSoundDevices(mCore.getNativePointer());
                            mReloadSoundDevicesScheduled = false;
                        }
                    }
                };
                dispatchOnCoreThreadAfter(reloadRunnable, delay);
            } else {
                Log.w("[Platform Helper] Bluetooth headset state changed but current global state is ", globalState.name(), ", skipping...");
            }
        }
    }

    public void onHeadsetStateChanged(boolean connected) {
        if (mCore != null) {
            if (mCore.getConfig().getInt("audio", "android_monitor_audio_devices", 1) == 0) return;

            GlobalState globalState = mCore.getGlobalState();
            if (globalState == GlobalState.On || globalState == GlobalState.Ready) {
                Log.i("[Platform Helper] Headset state changed, waiting for 500ms before reloading sound devices");
                if (mReloadSoundDevicesScheduled) {
                    Log.w("[Platform Helper] Sound devices reload is already pending, skipping...");
                    return;
                }
                mReloadSoundDevicesScheduled = true;

                Runnable reloadRunnable = new Runnable() {
                    @Override
                    public void run() {
                        Log.i("[Platform Helper] Reloading sound devices");
                        if (mCore != null) {
                            reloadSoundDevices(mCore.getNativePointer());
                            mReloadSoundDevicesScheduled = false;
                        }
                    }
                };
                dispatchOnCoreThread(reloadRunnable);
            } else {
                Log.w("[Platform Helper] Headset state changed but current global state is ", globalState.name(), ", skipping...");
            }
        }
    }

    public void onBackgroundMode() {
        Runnable backgroundRunnable = new Runnable() {
            @Override
            public void run() {
                Log.i("[Platform Helper] App has entered background mode");
                if (mCore != null) {
                    enterBackground(mCore.getNativePointer());
                    if (mCore.isAutoIterateEnabled()) {
                        stopTimerToResetAutoIterateSchedule();
                        Log.i("[Platform Helper] Restarting core.iterate() schedule with background timer");
                        startAutoIterate(mCore.getAutoIterateBackgroundSchedule());
                    }
                }
            }
        };
        dispatchOnCoreThread(backgroundRunnable);
    }

    public void onForegroundMode() {
        Runnable foregroundRunnable = new Runnable() {
            @Override
            public void run() {
                Log.i("[Platform Helper] App has left background mode");
                if (mCore != null) {
                    enterForeground(mCore.getNativePointer());
                    updateOrientation(Display.DEFAULT_DISPLAY);

                    if (mCore.isAutoIterateEnabled()) {
                        stopTimerToResetAutoIterateSchedule();
                        Log.i("[Platform Helper] Restarting core.iterate() schedule with foreground timer");
                        startAutoIterate(mCore.getAutoIterateForegroundSchedule());
                    }
                }
            }
        };
        dispatchOnCoreThread(foregroundRunnable);
    }

    public void setPushToken(String token) {
        int resId = mContext.getResources().getIdentifier("gcm_defaultSenderId", "string", mContext.getPackageName());
        String appId = mContext.getString(resId);
        Log.i("[Platform Helper] Push notification app id is [", appId, "] and token is [", token, "]");
        if (mCore != null) {
            updatePushNotificationInformation(mCore.getNativePointer(), appId, token);
        }
    }

    public void setAudioManagerInCommunicationMode() {
        if (mAudioHelper != null) mAudioHelper.setAudioManagerInCommunicationMode();
    }

    public void setAudioManagerInNormalMode() {
        if (mAudioHelper != null) mAudioHelper.setAudioManagerInNormalMode();
    }

    public synchronized boolean isPlayingSoundAllowed() {
        AudioManager audioManager = (AudioManager) mContext.getSystemService(Context.AUDIO_SERVICE);
        int ringerMode = audioManager.getRingerMode();
        if (ringerMode == AudioManager.RINGER_MODE_SILENT || ringerMode == AudioManager.RINGER_MODE_VIBRATE) {
            Log.w("[Platform Helper] Ringer mode is set to silent or vibrate (", ringerMode, ")");
            return false;
        }
        Log.i("[Platform Helper] Ringer mode is set to normal (", ringerMode, ")");
        return true;
    }

    public void stopRinging() {
        if (mAudioHelper != null) mAudioHelper.stopRinging();
    }

    private synchronized Class getServiceClass() {
        // Inspect services in package to get the class name of the Service that extends LinphoneService, assume first one
        try {
            PackageInfo packageInfo = mContext.getPackageManager().getPackageInfo(mContext.getPackageName(), PackageManager.GET_SERVICES);
            ServiceInfo[] services = packageInfo.services;
            if (services != null) {
                for (ServiceInfo service : services) {
                    String serviceName = service.name;
                    try {
                        Class serviceClass = Class.forName(serviceName);
                        if (CoreService.class.isAssignableFrom(serviceClass)) {
                            Log.i("[Platform Helper] Found a service that herits from org.linphone.core.tools.service.CoreService: ", serviceName);
                            return serviceClass;
                        }
                    } catch (Exception exception) {
                        Log.e("[Platform Helper] Exception trying to get Class from name [", serviceName, "]: ", exception);
                    } catch (Error error) {
                        Log.e("[Platform Helper] Error trying to get Class from name [", serviceName, "]: ", error);
                    }
                }
            } else {
                Log.w("[Platform Helper] No Service found in package info, continuing without it...");
                return null;
            }
        } catch (Exception e) {
            Log.e("[Platform Helper] Exception thrown while trying to find available Services: ", e);
            return null;
        }

        Log.w("[Platform Helper] Failed to find a valid Service, continuing without it...");
        return null;
    }

    private void startService() {
        Log.i("[Platform Helper] Starting service ", mServiceClass.getName());
        DeviceUtils.startForegroundService(mContext, new Intent().setClass(mContext, mServiceClass));
    }

    public void setServiceRunning(boolean running) {
        if (running == mServiceRunning) return;
        mServiceRunning = running;
        
        if (running) {
            Log.i("[Platform Helper] CoreService is now running");
        } else {
            Log.i("[Platform Helper] CoreService is no longer running");
        }
    }

    public void setServiceRunningAsForeground(boolean foreground) {
        if (foreground == mServiceRunningInForeground) return;
        mServiceRunningInForeground = foreground;
        
        if (mServiceRunningInForeground) {
            Log.i("[Platform Helper] CoreService is now running in foreground");
        } else {
            Log.i("[Platform Helper] CoreService is no longer running in foreground");
        }
    }

    private synchronized boolean isAndroidXMediaAvailable() {
        boolean available = false;
        try {
            Class audioAttributesCompat = Class.forName("androidx.media.AudioAttributesCompat");
            Class audioFocusRequestCompat = Class.forName("androidx.media.AudioFocusRequestCompat");
            Class audioManagerCompat = Class.forName("androidx.media.AudioManagerCompat");
            available = true;
        } catch (ClassNotFoundException e) {
            Log.w("[Platform Helper] Couldn't find class: ", e);
        } catch (Exception e) {
            Log.w("[Platform Helper] Exception: " + e);
        }
        return available;
    }

    private void dumpDeviceInformation() {
        Log.i("==== Phone information dump ====");
        Log.i("DEVICE=" + Build.DEVICE);
        Log.i("MODEL=" + Build.MODEL);
        Log.i("MANUFACTURER=" + Build.MANUFACTURER);
        Log.i("ANDROID SDK=" + Build.VERSION.SDK_INT);
        Log.i("PERFORMANCE CLASS=" + DeviceUtils.getPerformanceClass());
        StringBuilder sb = new StringBuilder();
        sb.append("ABIs=");
        for (String abi : Version.getCpuAbis()) {
            sb.append(abi).append(", ");
        }
        Log.i(sb.substring(0, sb.length() - 2));
        Log.i("=========================================");
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
        Log.i("PACKAGE=", org.linphone.core.BuildConfig.LIBRARY_PACKAGE_NAME);
        Log.i("BUILD TYPE=", org.linphone.core.BuildConfig.BUILD_TYPE);
        Log.i("=========================================");
    }

    private void updateOrientation(int displayId) {
        if (mCore == null) {
            Log.e("[Platform Helper] Core is null, don't notify device rotation");
            return;
        }

        if (mDisplayManager != null) {
            Display display = mDisplayManager.getDisplay(displayId);
            if (display != null) {
                int orientation = display.getRotation();
                int degrees = orientation * 90;
                Log.i("[Platform Helper] Device computed rotation is [", degrees, "] device display id is [", displayId, "])");
                mCore.setDeviceRotation(degrees);
            } else {
                Log.e("[Platform Helper] Display manager returned null display for id [", displayId, "], can't update device rotation!");
            }
        } else {
            Log.e("[Platform Helper] Android's display manager not available yet");
        }
    }

    private Class getPushServiceClass() {
        // Inspect services in package to get the class name of the Service that extends PushService, assume first one
        try {
            PackageInfo packageInfo = mContext.getPackageManager().getPackageInfo(mContext.getPackageName(), PackageManager.GET_SERVICES);
            ServiceInfo[] services = packageInfo.services;
            if (services != null) {
                for (ServiceInfo service : services) {
                    String serviceName = service.name;
                    try {
                        Class serviceClass = Class.forName(serviceName);
                        if (PushService.class.isAssignableFrom(serviceClass)) {
                            Log.i("[Platform Helper] Found a service that herits from org.linphone.core.tools.service.PushService: ", serviceName);
                            return serviceClass;
                        }
                    } catch (Exception exception) {
                        Log.e("[Platform Helper] Exception trying to get Class from name [", serviceName, "]: ", exception);
                    } catch (Error error) {
                        Log.e("[Platform Helper] Error trying to get Class from name [", serviceName, "]: ", error);
                    }
                }
            } else {
                Log.w("[Platform Helper] No Service found in package info, continuing without it...");
                return null;
            }
        } catch (Exception e) {
            Log.e("[Platform Helper] Exception thrown while trying to find available Services: ", e);
            return null;
        }

        Log.w("[Platform Helper] Failed to find a valid Service, continuing without it...");
        return null;
    }

    private Class getFileTransferServiceClass() {
        // Inspect services in package to get the class name of the Service that extends FileTransferService, assume first one
        try {
            PackageInfo packageInfo = mContext.getPackageManager().getPackageInfo(mContext.getPackageName(), PackageManager.GET_SERVICES);
            ServiceInfo[] services = packageInfo.services;
            if (services != null) {
                for (ServiceInfo service : services) {
                    String serviceName = service.name;
                    try {
                        Class serviceClass = Class.forName(serviceName);
                        if (FileTransferService.class.isAssignableFrom(serviceClass)) {
                            Log.i("[Platform Helper] Found a service that herits from org.linphone.core.tools.service.FileTransferService: ", serviceName);
                            return serviceClass;
                        }
                    } catch (Exception exception) {
                        Log.e("[Platform Helper] Exception trying to get Class from name [", serviceName, "]: ", exception);
                    } catch (Error error) {
                        Log.e("[Platform Helper] Error trying to get Class from name [", serviceName, "]: ", error);
                    }
                }
            } else {
                Log.w("[Platform Helper] No Service found in package info, continuing without it...");
                return null;
            }
        } catch (Exception e) {
            Log.e("[Platform Helper] Exception thrown while trying to find available Services: ", e);
            return null;
        }

        Log.w("[Platform Helper] Failed to find a valid Service, continuing without it...");
        return null;
    }
};
