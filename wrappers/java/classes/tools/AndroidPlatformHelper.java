/*
AndroidPlatformHelper.java
Copyright (C) 2017  Belledonne Communications, Grenoble, France

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


package org.linphone.core.tools;

import org.linphone.core.Core;
import org.linphone.core.tools.DozeReceiver;
import org.linphone.core.tools.NetworkManager;
import org.linphone.core.tools.NetworkManagerAbove21;
import org.linphone.mediastream.Log;
import org.linphone.mediastream.MediastreamerAndroidContext;
import org.linphone.mediastream.Version;

import android.content.res.Resources;
import android.graphics.SurfaceTexture;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.MulticastLock;
import android.net.wifi.WifiManager.WifiLock;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.IntentFilter;
import android.content.pm.ApplicationInfo;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkInfo;
import android.net.ProxyInfo;
import android.os.Environment;
import android.os.Handler;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.Build;
import android.view.Surface;
import android.view.TextureView;

import java.lang.Runnable;
import java.net.InetAddress;
import java.util.List;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

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
	private String mLinphoneRootCaFile;
	private String mRingSoundFile;
	private String mRingbackSoundFile;
	private String mPauseSoundFile;
	private String mErrorToneFile;
	private String mGrammarCpimFile;
	private String mGrammarVcardFile ;
	private String mUserCertificatePath;
	private Surface mSurface;
	private SurfaceTexture mSurfaceTexture;
	private boolean dozeModeEnabled;
	private BroadcastReceiver mDozeReceiver;
	private BroadcastReceiver mNetworkReceiver;
	private IntentFilter mDozeIntentFilter;
	private IntentFilter mNetworkIntentFilter;
	private boolean mWifiOnly;
	private boolean mUsingHttpProxy;
	private NetworkManagerAbove21 mNetworkManagerAbove21;
	private Handler mMainHandler;
	private Runnable mNetworkUpdateRunner;
	private boolean mMonitoringEnabled;

	private native void setNativePreviewWindowId(long nativePtr, Object view);
	private native void setNativeVideoWindowId(long nativePtr, Object view);
	private native void setNetworkReachable(long nativePtr, boolean reachable);
	private native void setHttpProxy(long nativePtr, String host, int port);

	public AndroidPlatformHelper(long nativePtr, Object ctx_obj, boolean wifiOnly) {
		mNativePtr = nativePtr;
		mContext = (Context) ctx_obj;
		mWifiOnly = wifiOnly;
		mResources = mContext.getResources();
		MediastreamerAndroidContext.setContext(mContext);

		mMainHandler = new Handler(mContext.getMainLooper());
		mNetworkUpdateRunner = new Runnable() {
			@Override
			public void run() {
				synchronized(AndroidPlatformHelper.this) {
					updateNetworkReachability();
				}
			}
		};

		WifiManager wifiMgr = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
		mPowerManager = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
		mConnectivityManager = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);

		mWakeLock = mPowerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "AndroidPlatformHelper");
		mWakeLock.setReferenceCounted(true);
		mMcastLock = wifiMgr.createMulticastLock("AndroidPlatformHelper");
		mMcastLock.setReferenceCounted(true);
		mWifiLock = wifiMgr.createWifiLock(WifiManager.WIFI_MODE_FULL_HIGH_PERF, "AndroidPlatformHelper");
		mWifiLock.setReferenceCounted(true);

		String basePath = mContext.getFilesDir().getAbsolutePath();
		//make sure to follow same path as unix version of the sdk
		mLinphoneRootCaFile = basePath + "/share/linphone/rootca.pem";
		mRingSoundFile = basePath + "/share/sounds/linphone/rings/notes_of_the_optimistic.mkv";
		mRingbackSoundFile = basePath + "/share/sounds/linphone/ringback.wav";
		mPauseSoundFile = basePath + "/share/sounds/linphone/rings/dont_wait_too_long.mkv";
		mErrorToneFile = basePath + "/share/sounds/linphone/incoming_chat.wav";
		mGrammarCpimFile = basePath + "/share/belr/grammars/cpim_grammar";
		mGrammarVcardFile = basePath + "/share/belr/grammars/vcard_grammar";
		mUserCertificatePath = basePath;

		try {
			copyAssetsFromPackage();
		} catch (IOException e) {
			Log.e("AndroidPlatformHelper(): failed to install some resources.");
		}
	}

	public synchronized void onLinphoneCoreStart(boolean monitoringEnabled) {
		mMonitoringEnabled = monitoringEnabled;
		startNetworkMonitoring();
	}

	public synchronized void onLinphoneCoreStop() {
		mNativePtr = 0;
		mMainHandler.removeCallbacksAndMessages(null);
		stopNetworkMonitoring();
	}

	public synchronized void onWifiOnlyEnabled(boolean enabled) {
		mWifiOnly = enabled;
		postNetworkUpdateRunner();
	}

	public synchronized Object getPowerManager() {
		return mPowerManager;
	}

	public synchronized String[] getDnsServers() {
		if (mConnectivityManager == null || Build.VERSION.SDK_INT < Version.API23_MARSHMALLOW_60)
			return null;

		if (mConnectivityManager.getActiveNetwork() == null
				|| mConnectivityManager.getLinkProperties(mConnectivityManager.getActiveNetwork()) == null)
			return null;

		int i = 0;
		List<InetAddress> inetServers = null;
		inetServers = mConnectivityManager.getLinkProperties(mConnectivityManager.getActiveNetwork()).getDnsServers();

		String[] servers = new String[inetServers.size()];

		for (InetAddress address : inetServers) {
			servers[i++] = address.getHostAddress();
		}
		Log.i("getDnsServers() returning");
		return servers;
	}

	public String getDataPath() {
		return mContext.getFilesDir().getAbsolutePath();
	}

	public String getConfigPath() {
		return mContext.getFilesDir().getAbsolutePath();
	}

	public String getCachePath() {
		return mContext.getCacheDir().getAbsolutePath();
	}

	public String getDownloadPath() {
		String downloadPath = Environment.getExternalStorageDirectory() + "/" + mContext.getString(mResources.getIdentifier("app_name", "string", mContext.getPackageName()));
		File dir = new File(downloadPath);
		if (!dir.exists()) {
			dir.mkdirs();
		}
		return downloadPath;
	}
	
	public String getNativeLibraryDir(){
		ApplicationInfo info = mContext.getApplicationInfo();
		return info.nativeLibraryDir;
	}

	public void acquireWifiLock() {
		Log.i("acquireWifiLock()");
		mWifiLock.acquire();
	}

	public void releaseWifiLock() {
		Log.i("releaseWifiLock()");
		mWifiLock.release();
	}

	public void acquireMcastLock() {
		Log.i("acquireMcastLock()");
		mMcastLock.acquire();
	}

	public void releaseMcastLock() {
		Log.i("releaseMcastLock()");
		mMcastLock.release();
	}

	public void acquireCpuLock() {
		Log.i("acquireCpuLock()");
		mWakeLock.acquire();
	}

	public void releaseCpuLock() {
		Log.i("releaseCpuLock()");
		mWakeLock.release();
	}

	private int getResourceIdentifierFromName(String name) {
		int resId = mResources.getIdentifier(name, "raw", mContext.getPackageName());
		if (resId == 0) {
			Log.d("App doesn't seem to embed resource " + name + " in it's res/raw/ directory, use linphone's instead");
			resId = mResources.getIdentifier(name, "raw", "org.linphone");
			if (resId == 0) {
				Log.i("App doesn't seem to embed resource " + name + " in it's res/raw/ directory. Make sure this file is either brought as an asset or a resource");
			}
		}
		return resId;
	}

	private void copyAssetsFromPackage() throws IOException {
		Log.i("Starting copy from assets to application files directory");
		copyAssetsFromPackage(mContext, "org.linphone.core",".");
		Log.i("Copy from assets done");
		Log.i("Starting copy from legacy  resources to application files directory");
		/*legacy code for 3.X*/
		copyEvenIfExists(getResourceIdentifierFromName("cpim_grammar"), mGrammarCpimFile);
		copyEvenIfExists(getResourceIdentifierFromName("vcard_grammar"), mGrammarVcardFile);
		copyEvenIfExists(getResourceIdentifierFromName("rootca"), mLinphoneRootCaFile);
		copyEvenIfExists(getResourceIdentifierFromName("notes_of_the_optimistic"), mRingSoundFile);
		copyEvenIfExists(getResourceIdentifierFromName("ringback"), mRingbackSoundFile);
		copyEvenIfExists(getResourceIdentifierFromName("hold"), mPauseSoundFile);
		copyEvenIfExists(getResourceIdentifierFromName("incoming_chat"), mErrorToneFile);
		Log.i("Copy from legacy resources done");
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
			Log.i("Resource identifier null for target ["+target.getName()+"]");
			return;
		}
		if (!target.getParentFile().exists())
			target.getParentFile().mkdirs();

		InputStream lInputStream = mResources.openRawResource(ressourceId);
		FileOutputStream lOutputStream = new FileOutputStream(target);
		int readByte;
		byte[] buff = new byte[8048];
		while (( readByte = lInputStream.read(buff)) != -1) {
			lOutputStream.write(buff,0, readByte);
		}
		lOutputStream.flush();
		lOutputStream.close();
		lInputStream.close();
	}

	public static void copyAssetsFromPackage(Context ctx,String fromPath, String toPath) throws IOException {
		new File(ctx.getFilesDir().getPath()+"/"+toPath).mkdir();

		for (String f :ctx.getAssets().list(fromPath)) {
			String current_name = fromPath+"/"+f;
			String current_dest = toPath+"/"+f;
			InputStream lInputStream;
			try {
				lInputStream = ctx.getAssets().open(current_name);
			} catch (IOException e) {
				//probably a dir
				copyAssetsFromPackage(ctx,current_name,current_dest);
				continue;
			}
			FileOutputStream lOutputStream =  new FileOutputStream(new File(ctx.getFilesDir().getPath()+"/"+current_dest));//ctx.openFileOutput (fromPath+"/"+f, 0);


			int readByte;
			byte[] buff = new byte[8048];
			while (( readByte = lInputStream.read(buff)) != -1) {
				lOutputStream.write(buff,0, readByte);
			}
			lOutputStream.flush();
			lOutputStream.close();
			lInputStream.close();
		}
	}

	public synchronized void setVideoPreviewView(Object view) {
		if (!(view instanceof TextureView)) {
			throw new RuntimeException("Preview window id is not an instance of TextureView. " +
				"Please update your UI layer so that the preview video view is a TextureView (or an instance of it)" +
				"or enable compatibility mode by setting displaytype=MSAndroidOpenGLDisplay in the [video] section your linphonerc factory configuration file" +
				"so you can keep using your existing application code for managing video views.");
		}
		TextureView textureView = (TextureView)view;
		textureView.setSurfaceTextureListener(new TextureView.SurfaceTextureListener() {
			@Override
			public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
				Log.i("Preview window surface is available");
				setNativePreviewWindowId(mNativePtr, surface);
			}

			@Override
			public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {

			}

			@Override
			public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
				Log.i("Preview window surface is no longer available");
				setNativePreviewWindowId(mNativePtr, null);
				return false;
			}

			@Override
			public void onSurfaceTextureUpdated(SurfaceTexture surface) {

			}
		});
		if (textureView.isAvailable()) {
			Log.i("Preview window surface is available");
			setNativePreviewWindowId(mNativePtr, textureView.getSurfaceTexture());
		}
	}

	public synchronized void setVideoRenderingView(Object view) {
		if (!(view instanceof TextureView)) {
			throw new RuntimeException("Rendering window id is not an instance of TextureView." +
				"Please update your UI layer so that the video rendering view is a TextureView (or an instance of it)" +
				"or enable compatibility mode by setting displaytype=MSAndroidOpenGLDisplay in the [video] section your linphonerc factory configuration file" +
				"so you can keep using your existing application code for managing video views.");
		}
		TextureView textureView = (TextureView)view;
		textureView.setSurfaceTextureListener(new TextureView.SurfaceTextureListener() {
			@Override
			public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
				Log.i("Rendering window surface is available");
				mSurfaceTexture = surface;
				mSurface = new Surface(mSurfaceTexture);
				setNativeVideoWindowId(mNativePtr, mSurface);
			}

			@Override
			public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {

			}

			@Override
			public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
				if (mSurfaceTexture == surface) {
					Log.i("Rendering window surface is no longer available");
					setNativeVideoWindowId(mNativePtr, null);
					mSurfaceTexture = null;
				}
				return false;
			}

			@Override
			public void onSurfaceTextureUpdated(SurfaceTexture surface) {

			}
		});
		if (textureView.isAvailable()) {
			Log.i("Rendering window surface is available");
			mSurfaceTexture = textureView.getSurfaceTexture();
			mSurface = new Surface(mSurfaceTexture);
			setNativeVideoWindowId(mNativePtr, mSurface);
		}
	}

	public synchronized void postNetworkUpdateRunner() {
		mMainHandler.removeCallbacksAndMessages(null);
		mMainHandler.post(mNetworkUpdateRunner);
	}

	public synchronized void updateNetworkReachability() {
		if (mConnectivityManager == null) return;
		if (mNativePtr == 0) {
			return;
		}

		boolean usingHttpProxyBefore = mUsingHttpProxy;
		boolean connected = false;
		NetworkInfo networkInfo = mConnectivityManager.getActiveNetworkInfo();
		connected = networkInfo != null && networkInfo.isConnected();

		if (connected && Build.VERSION.SDK_INT >= Version.API23_MARSHMALLOW_60){
			ProxyInfo proxy = mConnectivityManager.getDefaultProxy();
			if (proxy != null && proxy.getHost() != null){
				Log.i("The active network is using an http proxy: " + proxy.toString());
				setHttpProxy(mNativePtr, proxy.getHost(), proxy.getPort());
				mUsingHttpProxy = true;
			}else{
				setHttpProxy(mNativePtr, "", 0);
				mUsingHttpProxy = false;
			}
		}

		if (networkInfo == null || !connected) {
			Log.i("No connectivity: setting network unreachable");
			setNetworkReachable(mNativePtr, false);
		} else if (dozeModeEnabled) {
			Log.i("Doze Mode enabled: shutting down network");
			setNetworkReachable(mNativePtr, false);
		} else if (connected) {
			if (mWifiOnly) {
				if (networkInfo.getType() == ConnectivityManager.TYPE_WIFI) {
					setNetworkReachable(mNativePtr, true);
				} else {
					Log.i("Wifi-only mode, setting network not reachable");
					setNetworkReachable(mNativePtr, false);
				}
			} else {
				int curtype = networkInfo.getType();

				if (curtype != mLastNetworkType || mUsingHttpProxy != usingHttpProxyBefore) {
					//if kind of network has changed, we need to notify network_reachable(false) to make sure all current connections are destroyed.
					//they will be re-created during setNetworkReachable(true).
					Log.i("Connectivity has changed.");
					setNetworkReachable(mNativePtr, false);
				}
				setNetworkReachable(mNativePtr, true);
				mLastNetworkType = curtype;
			}
		}
	}

	public synchronized void setDozeModeEnabled(boolean b) {
		dozeModeEnabled = b;
	}

	private synchronized void startNetworkMonitoring() {
		if (!mMonitoringEnabled) return;
		
		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
			mNetworkReceiver = new NetworkManager(this);
			mNetworkIntentFilter = new IntentFilter(ConnectivityManager.CONNECTIVITY_ACTION);
			mContext.registerReceiver(mNetworkReceiver, mNetworkIntentFilter);
		} else {
			mNetworkManagerAbove21 = new NetworkManagerAbove21(this);
			mNetworkManagerAbove21.registerNetworkCallbacks(mConnectivityManager);
		}

		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
			mDozeIntentFilter = new IntentFilter();
			mDozeIntentFilter.addAction(PowerManager.ACTION_DEVICE_IDLE_MODE_CHANGED);
			mDozeReceiver = new DozeReceiver(this);
			dozeModeEnabled = ((PowerManager) mContext.getSystemService(Context.POWER_SERVICE)).isDeviceIdleMode();
			mContext.registerReceiver(mDozeReceiver, mDozeIntentFilter);
		}

		postNetworkUpdateRunner();
	}

	private synchronized void stopNetworkMonitoring() {
		if (!mMonitoringEnabled) return;
		
		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
			mContext.unregisterReceiver(mNetworkReceiver);
		} else {
			mNetworkManagerAbove21.unregisterNetworkCallbacks(mConnectivityManager);
		}

		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
			mContext.unregisterReceiver(mDozeReceiver);
		}

		mMonitoringEnabled = false;
	}
};


