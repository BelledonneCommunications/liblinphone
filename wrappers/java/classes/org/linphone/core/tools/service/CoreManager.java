/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
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

package org.linphone.core.tools.service;

import android.app.Application;
import android.app.ActivityManager;
import android.app.ActivityManager.RunningServiceInfo;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ServiceInfo;
import android.hardware.display.DisplayManager;
import android.media.AudioManager;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.view.Display;
import android.view.Surface;

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
import org.linphone.core.tools.compatibility.DeviceUtils;
import org.linphone.core.tools.receiver.ShutdownReceiver;
import org.linphone.mediastream.Version;

import java.lang.Error;
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
    private static final int AUTO_ITERATE_TIMER_CORE_START_OR_PUSH_RECEIVED = 20; // 20ms
    private static final int AUTO_ITERATE_TIMER_RESET_AFTER = 20000; // 20s

    public static boolean isReady() {
        return sInstance != null;
    }

    public static CoreManager instance() {
        if (isReady()) return sInstance;
        Log.e("[Core Manager] Trying to access instance that doesn't exists!");
        throw new RuntimeException("CoreManager not instantiated yet");
    }

    protected Context mContext;
    protected Core mCore;
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

    private Handler mHandler;
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

    public CoreManager(Object context, Core core) {
        mContext = ((Context) context).getApplicationContext();
        mCore = core;
        mServiceRunning = false;
        mServiceRunningInForeground = false;
        mReloadSoundDevicesScheduled = false;

        mTimer = null;
        mForcedIterateTimer = null;

        Looper myLooper = Looper.myLooper();
        if (myLooper == null) {
            Log.w("[Core Manager] Failed to detect current process Looper (have you called Looper.prepare()?), using main one");
            myLooper = Looper.getMainLooper();
        }
        
        mHandler = new Handler(myLooper);
        Thread thread = myLooper.getThread();
        boolean isMainThread = myLooper == Looper.getMainLooper();
        if (isMainThread) {
            Log.i("[Core Manager] Linphone SDK Android classes will use main thread: [", thread.getName(), "], id=", thread.getId());
        } else {
            Log.i("[Core Manager] Linphone SDK Android classes won't use main thread: [", thread.getName(), "], id=", thread.getId());
        }
        sInstance = this;

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
            Log.w("[Core Manager] Push notifications aren't available (see push utils log)");
        }
        
        if (isAndroidXMediaAvailable()) {
            mAudioHelper = new AudioHelper(mContext);
        } else {
            Log.w("[Core Manager] Do you have a dependency on androidx.media:media:1.2.0 or newer?");
        }
        mBluetoothHelper = new BluetoothHelper(mContext);

        mDisplayListener = new DisplayManager.DisplayListener() {
            @Override
            public void onDisplayAdded(int displayId) {
                Log.d("[Core Manager] Display added: ", displayId);
            }

            @Override
            public void onDisplayChanged(int displayId) {
                Log.i("[Core Manager] Display changed: ", displayId);
                updateOrientation(displayId);
            }

            @Override
            public void onDisplayRemoved(int displayId) {
                Log.d("[Core Manager] Display removed: ", displayId);
            }
        };
        mDisplayManager = (DisplayManager) mContext.getSystemService(Context.DISPLAY_SERVICE);
        mDisplayManager.registerDisplayListener(mDisplayListener, mHandler);

        IntentFilter shutdownIntentFilter = new IntentFilter(Intent.ACTION_SHUTDOWN);
        // Without that the broadcast timeout might be reached before we were called
        shutdownIntentFilter.setPriority(IntentFilter.SYSTEM_HIGH_PRIORITY - 1);
        mShutdownReceiver = new ShutdownReceiver();
        Log.i("[Core Manager] Registering shutdown receiver");
        mContext.registerReceiver(mShutdownReceiver, shutdownIntentFilter);

        mServiceClass = getServiceClass();
        if (mServiceClass == null) mServiceClass = CoreService.class;

        Log.i("[Core Manager] Ready");
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

    public void destroy() {
        Log.i("[Core Manager] Destroying");

        if (mActivityCallbacks != null) {
            ((Application) mContext).unregisterActivityLifecycleCallbacks(mActivityCallbacks);
            mActivityCallbacks = null;
        }

        if (mBluetoothHelper != null) {
            mBluetoothHelper.destroy(mContext);
            mBluetoothHelper = null;
        }

        if (mShutdownReceiver != null) {
            Log.i("[Core Manager] Unregistering shutdown receiver");
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
        Log.i("[Core Manager] Destroyed");
    }

    public Core getCore() {
        return mCore;
    }

    public void processPushNotification(String callId, String payload, boolean isCoreStarting) {
        if (mCore.isAutoIterateEnabled() && mCore.isInBackground()) {
            // Force the core.iterate() scheduling to a low value to ensure the Core will process what triggered the push notification as quickly as possible
            Log.i("[Core Manager] Push notification received, scheduling core.iterate() every " + AUTO_ITERATE_TIMER_CORE_START_OR_PUSH_RECEIVED + "ms");
            startAutoIterate(AUTO_ITERATE_TIMER_CORE_START_OR_PUSH_RECEIVED);
            createTimerToResetAutoIterateSchedule();
        }

        Log.i("[Core Manager] Notifying Core a push with Call-ID [" + callId + "] has been received");
        processPushNotification(mCore.getNativePointer(), callId, payload, isCoreStarting);
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

    public void onLinphoneCoreStart() {
        Log.i("[Core Manager] Starting");

        if (!isAndroidXMediaAvailable() && mCore.isNativeRingingEnabled()) {
            Log.e("[Core Manager] Native ringing was enabled but condition isn't met (androidx.media:media dependency), disabling it.");
            mCore.setNativeRingingEnabled(false);
        }

        if (mCore.isAutoIterateEnabled()) {
            // Force the core.iterate() scheduling to a low value to ensure the Core will be ready as quickly as possible
            Log.i("[Core Manager] Core is starting, scheduling core.iterate() every " + AUTO_ITERATE_TIMER_CORE_START_OR_PUSH_RECEIVED + "ms");
            startAutoIterate(AUTO_ITERATE_TIMER_CORE_START_OR_PUSH_RECEIVED);
            createTimerToResetAutoIterateSchedule();
        } else {
            Log.w("[Core Manager] Auto core.iterate() isn't enabled, ensure you do it in your application!");
        }

        mListener = new CoreListenerStub() {
            @Override
            public void onFirstCallStarted(Core core) {
                Log.i("[Core Manager] First call started");
                // Ensure Service is running. It will take care by itself to start as foreground.
                if (!mServiceRunning) {
                    Log.w("[Core Manager] Service isn't running, let's start it");
                    try {
                        startService();
                    } catch (IllegalStateException ise) {
                        Log.w("[Core Manager] Failed to start service: ", ise);
                    }
                } else {
                    Log.i("[Core Manager] Service appears to be running, everything is fine");
                }
            }

            @Override
            public void onLastCallEnded(Core core) {
                Log.i("[Core Manager] Last call ended");
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
                        Log.w("[Core Manager] It seems call was accepted with early-media during the incoming received call state changed, continuing anyway");
                    } else {
                        Log.w("[Core Manager] Call state changed callback state variable doesn't match current call state, skipping");
                        return;
                    }
                }

                if (state == Call.State.IncomingReceived && core.getCallsNb() == 1) {
                    if (core.isNativeRingingEnabled()) {
                        if (core.getConfig().getInt("sound", "disable_ringing", 0) == 1) {
                            Log.w("[Core Manager] Ringing was disabled in configuration (disable_ringing item in [sound] section is set to 1)");
                        } else {
                            Log.i("[Core Manager] Incoming call received, no other call, start ringing");
                            mAudioHelper.startRinging(mContext, core.getRing(), call.getRemoteAddress());
                        }
                    } else {
                        Log.i("[Core Manager] Incoming call received, no other call, acquire ringing audio focus");
                        mAudioHelper.requestRingingAudioFocus();
                    }
                } else if (state == Call.State.IncomingEarlyMedia && core.getCallsNb() == 1) {
                    if (core.getRingDuringIncomingEarlyMedia()) {
                        Log.i("[Core Manager] Incoming call is early media and ringing is allowed");
                    } else {
                        if (core.isNativeRingingEnabled()) {
                            Log.w("[Core Manager] Incoming call is early media and ringing is disabled, stop ringing");
                            mAudioHelper.stopRinging();
                        } else {
                            Log.i("[Core Manager] Incoming call is early media and ringing is disabled, keep ringing audio focus as sound card will be using RING stream");
                        }
                    }
                } else if (state == Call.State.Connected) {
                    if (call.getDir() == Call.Dir.Incoming && core.isNativeRingingEnabled()) {
                        Log.i("[Core Manager] Stop incoming call ringing");
                        mAudioHelper.stopRinging();
                    } else {
                        Log.i("[Core Manager] Stop incoming call ringing audio focus");
                        mAudioHelper.releaseRingingAudioFocus();
                    }
                } else if (state == Call.State.OutgoingInit && core.getCallsNb() == 1) {
                    Log.i("[Core Manager] Outgoing call in progress, no other call, acquire ringing audio focus for ringback");
                    mAudioHelper.requestRingingAudioFocus();
                } else if (state == Call.State.StreamsRunning) {
                    Log.i("[Core Manager] Call active, ensure audio focus granted");
                    mAudioHelper.requestCallAudioFocus(false);
                } else if (state == Call.State.Resuming) {
                    Log.i("[Core Manager] Call resuming, ensure audio focus granted");
                    mAudioHelper.requestCallAudioFocus(false);
                }
            }
        };
        
        mCore.addListener(mListener);
        Log.i("[Core Manager] Started");

        SharedPreferences sharedPref = mContext.getSharedPreferences("push_notification_storage", Context.MODE_PRIVATE);
        String callId = sharedPref.getString("call-id", "");
        String payload = sharedPref.getString("payload", "");
        if (!callId.isEmpty()) {
            Log.i("[Core Manager] Push notification information retrieved from storage, Call-ID is [" + callId + "]");
            processPushNotification(callId, payload, true);

            SharedPreferences.Editor editor = sharedPref.edit();
            editor.putString("call-id", "");
            editor.putString("payload", "");
            editor.apply();
            Log.i("[Core Manager] Push information cleared from storage");
        }
        
    }

    public void stop() {
        Log.i("[Core Manager] Stopping");
        stopCore(mCore.getNativePointer());
    }

    public void onLinphoneCoreStop() {
        Log.i("[Core Manager] Core stopped");

        if (mServiceRunning) {
            Log.i("[Core Manager] Stopping service ", mServiceClass.getName());
            mContext.stopService(new Intent().setClass(mContext, mServiceClass));
        }

        mCore.removeListener(mListener);

        stopAutoIterate();
        stopTimerToResetAutoIterateSchedule();

        mCore = null; // To allow the garbage colletor to free the Core
        Log.i("[Core Manager] Core released");
    }

    public void startAutoIterate() {
        if (mCore == null) return;
        
        if (mCore.isAutoIterateEnabled()) {
            if (mTimer != null) {
                Log.w("[Core Manager] core.iterate() scheduling is already active");
                return;
            }

            if (mCore.isInBackground()) {
                Log.i("[Core Manager] Start core.iterate() scheduling with background timer");
                startAutoIterate(mCore.getAutoIterateBackgroundSchedule());
            } else {
                Log.i("[Core Manager] Start core.iterate() scheduling with foreground timer");
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
                            Log.i("[Core Manager] Resetting core.iterate() schedule depending on background/foreground state");
                            stopAutoIterate();
                            startAutoIterate();
                        }
                    };
                    dispatchOnCoreThread(resetRunnable);
                }
            };

        mForcedIterateTimer = new Timer("Linphone core.iterate() reset scheduler");
        mForcedIterateTimer.schedule(lTask, AUTO_ITERATE_TIMER_RESET_AFTER);
        Log.i("[Core Manager] Iterate scheduler will be reset in " + (AUTO_ITERATE_TIMER_RESET_AFTER / 1000) + " seconds");
    }

    private void startAutoIterate(int schedule) {
        if (mTimer != null && schedule == mIterateSchedule) {
            Log.i("[Core Manager] core.iterate() is already scheduled every " + schedule + " ms");
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
        Log.i("[Core Manager] Call to core.iterate() scheduled every " + mIterateSchedule + " ms");
    }

    public void stopAutoIterate() {
        if (mTimer != null) {
            Log.i("[Core Manager] Stopping scheduling of core.iterate() every " + mIterateSchedule + " ms");
            mTimer.cancel();
            mTimer.purge();
            mTimer = null;
            Log.i("[Core Manager] core.iterate() scheduler stopped");
        } else {
            Log.w("[Core Manager] core.iterate() scheduling wasn't started or already stopped");
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
                    Log.i("[Core Manager] App has lost audio focus, leaving conference");
                    leaveConference(mCore.getNativePointer());
                } else {
                    Log.i("[Core Manager] App has lost audio focus, pausing all calls");
                    pauseAllCalls(mCore.getNativePointer());
                }
                mAudioHelper.releaseCallAudioFocus();
            } else {
                Log.w("[Core Manager] Audio focus lost but keeping calls running");
            }
        }
    }

    public void onBluetoothAdapterTurnedOn() {
        if (DeviceUtils.isBluetoothConnectPermissionGranted(mContext)) {
            onBluetoothHeadsetStateChanged();
        } else {
            Log.w("[Core Manager] Bluetooth Connect permission isn't granted, waiting longer before reloading sound devices to increase chances to get bluetooth device");
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
                Log.i("[Core Manager] Bluetooth headset state changed, waiting for " + delay + " ms before reloading sound devices");
                if (mReloadSoundDevicesScheduled) {
                    Log.w("[Core Manager] Sound devices reload is already pending, skipping...");
                    return;
                }
                mReloadSoundDevicesScheduled = true;

            
                Runnable reloadRunnable = new Runnable() {
                    @Override
                    public void run() {
                        Log.i("[Core Manager] Reloading sound devices");
                        if (mCore != null) {
                            reloadSoundDevices(mCore.getNativePointer());
                            mReloadSoundDevicesScheduled = false;
                        }
                    }
                };
                dispatchOnCoreThreadAfter(reloadRunnable, delay);
            } else {
                Log.w("[Core Manager] Bluetooth headset state changed but current global state is ", globalState.name(), ", skipping...");
            }
        }
    }

    public void onHeadsetStateChanged(boolean connected) {
        if (mCore != null) {
            if (mCore.getConfig().getInt("audio", "android_monitor_audio_devices", 1) == 0) return;

            GlobalState globalState = mCore.getGlobalState();
            if (globalState == GlobalState.On || globalState == GlobalState.Ready) {
                Log.i("[Core Manager] Headset state changed, waiting for 500ms before reloading sound devices");
                if (mReloadSoundDevicesScheduled) {
                    Log.w("[Core Manager] Sound devices reload is already pending, skipping...");
                    return;
                }
                mReloadSoundDevicesScheduled = true;

                Runnable reloadRunnable = new Runnable() {
                    @Override
                    public void run() {
                        Log.i("[Core Manager] Reloading sound devices");
                        if (mCore != null) {
                            reloadSoundDevices(mCore.getNativePointer());
                            mReloadSoundDevicesScheduled = false;
                        }
                    }
                };
                dispatchOnCoreThread(reloadRunnable);
            } else {
                Log.w("[Core Manager] Headset state changed but current global state is ", globalState.name(), ", skipping...");
            }
        }
    }

    public void onBackgroundMode() {
        Runnable backgroundRunnable = new Runnable() {
            @Override
            public void run() {
                Log.i("[Core Manager] App has entered background mode");
                if (mCore != null) {
                    enterBackground(mCore.getNativePointer());
                    if (mCore.isAutoIterateEnabled()) {
                        stopTimerToResetAutoIterateSchedule();
                        Log.i("[Core Manager] Restarting core.iterate() schedule with background timer");
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
                Log.i("[Core Manager] App has left background mode");
                if (mCore != null) {
                    enterForeground(mCore.getNativePointer());
                    updateOrientation(Display.DEFAULT_DISPLAY);

                    if (mCore.isAutoIterateEnabled()) {
                        stopTimerToResetAutoIterateSchedule();
                        Log.i("[Core Manager] Restarting core.iterate() schedule with foreground timer");
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
        Log.i("[Core Manager] Push notification app id is [", appId, "] and token is [", token, "]");
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

    public boolean isRingingAllowed() {
        AudioManager audioManager = (AudioManager) mContext.getSystemService(Context.AUDIO_SERVICE);
        int ringerMode = audioManager.getRingerMode();
        if (ringerMode == AudioManager.RINGER_MODE_SILENT || ringerMode == AudioManager.RINGER_MODE_VIBRATE) {
            Log.w("[Core Manager] Ringer mode is set to silent or vibrate (", ringerMode, ")");
            return false;
        }
        Log.i("[Core Manager] Ringer mode is set to normal (", ringerMode, ")");
        return true;
    }

    public void stopRinging() {
        if (mAudioHelper != null) mAudioHelper.stopRinging();
    }

    private Class getServiceClass() {
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
                            Log.i("[Core Manager] Found a service that herits from org.linphone.core.tools.service.CoreService: ", serviceName);
                            return serviceClass;
                        }
                    } catch (Exception exception) {
                        Log.e("[Core Manager] Exception trying to get Class from name [", serviceName, "]: ", exception);
                    } catch (Error error) {
                        Log.e("[Core Manager] Error trying to get Class from name [", serviceName, "]: ", error);
                    }
                }
            } else {
                Log.w("[Core Manager] No Service found in package info, continuing without it...");
                return null;
            }
        } catch (Exception e) {
            Log.e("[Core Manager] Exception thrown while trying to find available Services: ", e);
            return null;
        }

        Log.w("[Core Manager] Failed to find a valid Service, continuing without it...");
        return null;
    }

    private void startService() {
        Log.i("[Core Manager] Starting service ", mServiceClass.getName());
        DeviceUtils.startForegroundService(mContext, new Intent().setClass(mContext, mServiceClass));
    }

    public void setServiceRunning(boolean running) {
        if (running == mServiceRunning) return;
        mServiceRunning = running;
        
        if (running) {
            Log.i("[Core Manager] CoreService is now running");
        } else {
            Log.i("[Core Manager] CoreService is no longer running");
        }
    }

    public void setServiceRunningAsForeground(boolean foreground) {
        if (foreground == mServiceRunningInForeground) return;
        mServiceRunningInForeground = foreground;
        
        if (mServiceRunningInForeground) {
            Log.i("[Core Manager] CoreService is now running in foreground");
        } else {
            Log.i("[Core Manager] CoreService is no longer running in foreground");
        }
    }

    public boolean isServiceRunningAsForeground() {
        return mServiceRunningInForeground;
    }

    private boolean isAndroidXMediaAvailable() {
        boolean available = false;
        try {
            Class audioAttributesCompat = Class.forName("androidx.media.AudioAttributesCompat");
            Class audioFocusRequestCompat = Class.forName("androidx.media.AudioFocusRequestCompat");
            Class audioManagerCompat = Class.forName("androidx.media.AudioManagerCompat");
            available = true;
        } catch (ClassNotFoundException e) {
            Log.w("[Core Manager] Couldn't find class: ", e);
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
            Log.e("[Core Manager] Core is null, don't notify device rotation");
            return;
        }

        if (mDisplayManager != null) {
            Display display = mDisplayManager.getDisplay(displayId);
            if (display != null) {
                int orientation = display.getRotation();
                int degrees = orientation * 90;
                Log.i("[Core Manager] Device computed rotation is [", degrees, "] device display id is [", displayId, "])");
                mCore.setDeviceRotation(degrees);
            } else {
                Log.e("[Core Manager] Display manager returned null display for id [", displayId, "], can't update device rotation!");
            }
        } else {
            Log.e("[Core Manager] Android's display manager not available yet");
        }
    }
}
