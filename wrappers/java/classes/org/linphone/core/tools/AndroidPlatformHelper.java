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
import org.linphone.core.tools.service.CoreManager;
import org.linphone.core.tools.service.PushService;
import org.linphone.core.tools.service.FileTransferService;
import org.linphone.mediastream.MediastreamerAndroidContext;
import org.linphone.mediastream.video.capture.CaptureTextureView;
import org.linphone.mediastream.Version;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;

/**
 * This class is instanciated directly by the linphone library in order to access specific features only accessible in java.
 **/
public class AndroidPlatformHelper {
    private long mNativePtr;
    private Context mContext;
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

    private static int mTempCountWifi = 0;
    private static int mTempCountMCast = 0;
    private static int mTempCountCPU = 0;

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

    public AndroidPlatformHelper(long nativePtr, Object ctx_obj, boolean wifiOnly) {
        mNativePtr = nativePtr;
        mContext = ((Context) ctx_obj).getApplicationContext();
        mWifiOnly = wifiOnly;
        mDnsServersList = new ArrayList<String>();
        mResources = mContext.getResources();
        
        Looper myLooper = Looper.myLooper();
        if (myLooper == null) {
            Log.w("[Platform Helper] Failed to detect current process Looper (have you called Looper.prepare()?), using main one");
            myLooper = Looper.getMainLooper();
        }
        mHandler = new Handler(myLooper);

        MediastreamerAndroidContext.setContext(mContext);

        Log.i("[Platform Helper] Created, wifi only mode is " + (mWifiOnly ? "enabled" : "disabled"));

        WifiManager wifiMgr = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
        mPowerManager = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
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

        if (Version.sdkAboveOrEqual(Version.API29_ANDROID_10)) {
            mNetworkSignalMonitor = new NetworkSignalMonitor(mContext, this);
        } else {
            Log.w("[Platform Helper] Device is running Android < 10, can't use network signal strength monitoring");
        }

        mPushServiceClass = getPushServiceClass();
        if (mPushServiceClass == null) {
            mPushServiceClass = org.linphone.core.tools.service.PushService.class;
        }

        mFileTransferServiceClass = getFileTransferServiceClass();
        if (mFileTransferServiceClass == null) {
            mFileTransferServiceClass = org.linphone.core.tools.service.FileTransferService.class;
        }
    }

    public synchronized void onLinphoneCoreStart(boolean monitoringEnabled) {
        Log.i("[Platform Helper] onLinphoneCoreStart, network monitoring is " + monitoringEnabled);
        mMonitoringEnabled = monitoringEnabled;
        startNetworkMonitoring();
    }

    public synchronized void onLinphoneCoreStop() {
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
    }

    public synchronized void requestWifiSignalStrengthUpdate() {
        if (mNetworkSignalMonitor != null) {
            mNetworkSignalMonitor.updateWifiConnectionSignalStrength();
        }
    }

    public synchronized void setSignalInfo(SignalType type, SignalStrengthUnit unit, int value, String details) {
        setSignalInfo(mNativePtr, type.toInt(), unit.toInt(), value, details);
    }

    public synchronized void onWifiOnlyEnabled(boolean enabled) {
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

    public synchronized void acquireWifiLock() {
        mTempCountWifi++;
        Log.d("[Platform Helper] acquireWifiLock(). count = " + mTempCountWifi);
        if (!mWifiLock.isHeld()) {
            mWifiLock.acquire();
        }
    }

    public synchronized void releaseWifiLock() {
        mTempCountWifi--;
        Log.d("[Platform Helper] releaseWifiLock(). count = " + mTempCountWifi);
        if (mWifiLock.isHeld()) {
            mWifiLock.release();
        }
    }

    public synchronized void acquireMcastLock() {
        mTempCountMCast++;
        Log.d("[Platform Helper] acquireMcastLock(). count = " + mTempCountMCast);
        if (!mMcastLock.isHeld()) {
            mMcastLock.acquire();
        }
    }

    public synchronized void releaseMcastLock() {
        mTempCountMCast--;
        Log.d("[Platform Helper] releaseMcastLock(). count = " + mTempCountMCast);
        if (mMcastLock.isHeld()) {
            mMcastLock.release();
        }
    }

    public synchronized void acquireCpuLock() {
        mTempCountCPU++;
        Log.d("[Platform Helper] acquireCpuLock(). count = " + mTempCountCPU);
        if (!mWakeLock.isHeld()) {
            mWakeLock.acquire();
        }
    }

    public synchronized void releaseCpuLock() {
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
        String mGrammarCpimFile = basePath + "/share/belr/grammars/cpim_grammar";
        String mGrammarIcsFile = basePath + "/share/belr/grammars/ics_grammar";
        String mGrammarIdentityFile = basePath + "/share/belr/grammars/identity_grammar";
        String mGrammarMwiFile = basePath + "/share/belr/grammars/mwi_grammar";
        String mGrammarVcardFile = basePath + "/share/belr/grammars/vcard_grammar";

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

            Log.i("[Platform Helper] Installing Resource " + f);
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

    private synchronized void setNativePreviewWindowIdOnCoreThread(TextureView view) {
        Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    setNativePreviewWindowId(mNativePtr, view);
                }
        };
        mHandler.post(runnable);
    }

    public synchronized void setVideoPreviewView(Object view) {
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
                setNativePreviewWindowIdOnCoreThread(mPreviewTextureView);
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
                        mPreviewTextureView = null;
                        setNativePreviewWindowIdOnCoreThread(null);
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

    private synchronized void setNativeVideoWindowIdOnCoreThread(SurfaceTexture view) {
        Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    setNativeVideoWindowId(mNativePtr, view);
                }
        };
        mHandler.post(runnable);
    }

    public synchronized void setVideoRenderingView(Object view) {
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
                setNativeVideoWindowIdOnCoreThread(surface);
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
                        mVideoTextureView = null;
                        setNativeVideoWindowIdOnCoreThread(null);
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

    private synchronized void setParticipantDeviceNativeVideoWindowIdOnCoreThread(long participantDevice, SurfaceTexture view) {
        Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    setParticipantDeviceNativeVideoWindowId(mNativePtr, participantDevice, view);
                }
        };
        mHandler.post(runnable);
    }

    public synchronized void setParticipantDeviceVideoRenderingView(long participantDevice, Object view) {
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
               setParticipantDeviceNativeVideoWindowIdOnCoreThread(participantDevice, surface);
            }

            @Override
            public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
                Log.i("[Platform Helper] Surface texture [" + surface + "] of participant device size changed: " + width + "x" + height);
            }

            @Override
            public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
               Log.w("[Platform Helper] TextureView [" + surface + "] for participant device has been destroyed");
               mParticipantTextureView.remove(participantDevice);
               setParticipantDeviceNativeVideoWindowIdOnCoreThread(participantDevice, null);
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

    public synchronized void resizeVideoPreview(int width, int height) {
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
        if (CoreManager.isReady()) {
            if (CoreManager.instance().isServiceRunningAsForeground()) {
                Log.i("[Platform Helper] CoreService seems to be running as foreground, consider app is in foreground");
                return false;
            }
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

    public synchronized void updateDnsServers(ArrayList<String> dnsServers) {
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

    public synchronized void updateNetworkReachability() {
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

    private synchronized void startNetworkMonitoring() {
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

        updateNetworkReachability();
    }

    private synchronized void stopNetworkMonitoring() {
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

    public synchronized void disableAudioRouteChanges(boolean disable) {
        if (disable) {
            Log.i("[Platform Helper] Disabling audio route changes in mediastreamer2");
        } else {
            Log.i("[Platform Helper] Enabling audio route changes in mediastreamer2");
        }
        MediastreamerAndroidContext.disableAudioRouteChanges(disable);
    }

    public synchronized void startPushService() {
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

    public synchronized void stopPushService() {
        if (mPushServiceStarted) {
            Log.i("[Platform Helper] Foreground push service is no longer required");
            Intent i = new Intent(mContext, mPushServiceClass);
            mContext.stopService(i);
            mPushServiceStarted = false;
        }
    }

    public synchronized void startFileTransferService() {
        if (!mFileTransferServiceStarted) {
            Log.i("[Platform Helper] Starting foreground file transfer service");
            Intent i = new Intent(mContext, mFileTransferServiceClass);
            DeviceUtils.startForegroundService(mContext, i);
            mFileTransferServiceStarted = true;
        }
    }

    public synchronized void stopFileTransferService() {
        if (mFileTransferServiceStarted) {
            Log.i("[Platform Helper] Foreground file transfer service is no longer required");
            Intent i = new Intent(mContext, mFileTransferServiceClass);
            mContext.stopService(i);
            mFileTransferServiceStarted = false;
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
