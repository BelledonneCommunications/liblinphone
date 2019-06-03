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

import org.linphone.core.tools.DozeReceiver;
import org.linphone.core.tools.NetworkManager;
import org.linphone.core.tools.NetworkManagerAbove21;
import org.linphone.core.tools.NetworkManagerAbove24;
import org.linphone.core.tools.NetworkManagerAbove26;
import org.linphone.core.tools.Log;
import org.linphone.mediastream.MediastreamerAndroidContext;
import org.linphone.mediastream.Version;

import android.content.res.Resources;
import android.graphics.SurfaceTexture;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.MulticastLock;
import android.net.wifi.WifiManager.WifiLock;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
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
import android.view.ViewGroup;

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
	private TextureView mPreviewTextureView;
	private boolean mDozeModeEnabled;
	private BroadcastReceiver mDozeReceiver;
	private IntentFilter mDozeIntentFilter;
	private boolean mWifiOnly;
	private boolean mUsingHttpProxy;
	private NetworkManagerInterface mNetworkManager;
	private Handler mMainHandler;
	private boolean mMonitoringEnabled;
	private boolean mIsInInteractiveMode;
	private InteractivityReceiver mInteractivityReceiver;
	private IntentFilter mInteractivityIntentFilter;
	private String[] mDnsServers;

	private native void setNativePreviewWindowId(long nativePtr, Object view);
	private native void setNativeVideoWindowId(long nativePtr, Object view);
	private native void setNetworkReachable(long nativePtr, boolean reachable);
	private native void setHttpProxy(long nativePtr, String host, int port);
	private native boolean isInBackground(long nativePtr);
	private native void enableKeepAlive(long nativePtr, boolean enable);
	private native boolean useSystemHttpProxy(long nativePtr);

	public AndroidPlatformHelper(long nativePtr, Object ctx_obj, boolean wifiOnly) {
		mNativePtr = nativePtr;
		mContext = (Context) ctx_obj;
		mWifiOnly = wifiOnly;
		mDnsServers = null;
		mResources = mContext.getResources();
		mMainHandler = new Handler(mContext.getMainLooper());

		mIsInInteractiveMode = true;
		mDozeModeEnabled = false;

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
		Network network = nm.getActiveNetwork();
		if (network != null) {
			storeDnsServers(network);
		}
	}

	public synchronized void onLinphoneCoreStart(boolean monitoringEnabled) {
		Log.i("[Platform Helper] onLinphoneCoreStart, network monitoring is " + monitoringEnabled);
		mMonitoringEnabled = monitoringEnabled;
		startNetworkMonitoring();
	}

	public synchronized void onLinphoneCoreStop() {
		Log.i("[Platform Helper] onLinphoneCoreStop, network monitoring is " + mMonitoringEnabled);
		mNativePtr = 0;
		mMainHandler.removeCallbacksAndMessages(null);
		stopNetworkMonitoring();
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
		String dnsList = "";
		for (String dns : mDnsServers) {
			dnsList += dns;
			dnsList += ", ";
		}
		Log.i("[Platform Helper] getDnsServers() returning " + dnsList);
		return mDnsServers;
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
		if (Build.VERSION.SDK_INT >= 23) {
			// This check seems mandatory for auto download to work...
			int write_permission = mContext.getPackageManager().checkPermission(android.Manifest.permission.WRITE_EXTERNAL_STORAGE, mContext.getPackageName());
			Log.i("[Platform Helper] WRITE_EXTERNAL_STORAGE permission is " + (write_permission == android.content.pm.PackageManager.PERMISSION_GRANTED ? "granted" : "denied"));
		}
		if (!Environment.getExternalStorageDirectory().canWrite()) {
			Log.w("[Platform Helper] We aren't allowed to write in external storage directory !");
		}
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
		Log.i("[Platform Helper] acquireWifiLock()");
		mWifiLock.acquire();
	}

	public void releaseWifiLock() {
		Log.i("[Platform Helper] releaseWifiLock()");
		mWifiLock.release();
	}

	public void acquireMcastLock() {
		Log.i("[Platform Helper] acquireMcastLock()");
		mMcastLock.acquire();
	}

	public void releaseMcastLock() {
		Log.i("[Platform Helper] releaseMcastLock()");
		mMcastLock.release();
	}

	public void acquireCpuLock() {
		Log.i("[Platform Helper] acquireCpuLock()");
		mWakeLock.acquire();
	}

	public void releaseCpuLock() {
		Log.i("[Platform Helper] releaseCpuLock()");
		mWakeLock.release();
	}

	private int getResourceIdentifierFromName(String name) {
		int resId = mResources.getIdentifier(name, "raw", mContext.getPackageName());
		if (resId == 0) {
			Log.d("[Platform Helper] App doesn't seem to embed resource " + name + " in it's res/raw/ directory, use linphone's instead");
			resId = mResources.getIdentifier(name, "raw", "org.linphone");
			if (resId == 0) {
				Log.i("[Platform Helper] App doesn't seem to embed resource " + name + " in it's res/raw/ directory. Make sure this file is either brought as an asset or a resource");
			}
		}
		return resId;
	}

	private void copyAssetsFromPackage() throws IOException {
		Log.i("[Platform Helper] Starting copy from assets to application files directory");
		copyAssetsFromPackage(mContext, "org.linphone.core",".");
		Log.i("[Platform Helper] Copy from assets done");
		Log.i("[Platform Helper] Starting copy from legacy  resources to application files directory");
		/*legacy code for 3.X*/
		copyEvenIfExists(getResourceIdentifierFromName("cpim_grammar"), mGrammarCpimFile);
		copyEvenIfExists(getResourceIdentifierFromName("vcard_grammar"), mGrammarVcardFile);
		copyEvenIfExists(getResourceIdentifierFromName("rootca"), mLinphoneRootCaFile);
		copyEvenIfExists(getResourceIdentifierFromName("notes_of_the_optimistic"), mRingSoundFile);
		copyEvenIfExists(getResourceIdentifierFromName("ringback"), mRingbackSoundFile);
		copyEvenIfExists(getResourceIdentifierFromName("hold"), mPauseSoundFile);
		copyEvenIfExists(getResourceIdentifierFromName("incoming_chat"), mErrorToneFile);
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
			Log.i("[Platform Helper] Resource identifier null for target ["+target.getName()+"]");
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
		if (view instanceof Surface) {
			Surface surface = (Surface) view;
			setNativePreviewWindowId(mNativePtr, surface);
			return;
		}
		
		if (!(view instanceof TextureView)) {
			throw new RuntimeException("[Platform Helper] Preview window id is not an instance of TextureView. " +
				"Please update your UI layer so that the preview video view is a TextureView (or an instance of it)" +
				"or enable compatibility mode by setting displaytype=MSAndroidOpenGLDisplay in the [video] section your linphonerc factory configuration file" +
				"so you can keep using your existing application code for managing video views.");
		}

		mPreviewTextureView = (TextureView)view;
		ViewGroup.LayoutParams lp = mPreviewTextureView.getLayoutParams();
		mPreviewTextureView.setSurfaceTextureListener(new TextureView.SurfaceTextureListener() {
			@Override
			public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
				Log.i("[Platform Helper] Preview window surface is available");
				setNativePreviewWindowId(mNativePtr, surface);
			}

			@Override
			public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
				
			}

			@Override
			public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
				Log.i("[Platform Helper] Preview window surface is no longer available");
				setNativePreviewWindowId(mNativePtr, null);
				mPreviewTextureView = null;
				return false;
			}

			@Override
			public void onSurfaceTextureUpdated(SurfaceTexture surface) {

			}
		});

		if (mPreviewTextureView.isAvailable()) {
			Log.i("[Platform Helper] Preview window surface is available");
			setNativePreviewWindowId(mNativePtr, mPreviewTextureView.getSurfaceTexture());
		}
	}

	public synchronized void setVideoRenderingView(Object view) {
		if (view instanceof Surface) {
			Surface surface = (Surface) view;
			setNativeVideoWindowId(mNativePtr, surface);
			return;
		}
		
		if (!(view instanceof TextureView)) {
			throw new RuntimeException("[Platform Helper] Rendering window id is not an instance of TextureView." +
				"Please update your UI layer so that the video rendering view is a TextureView (or an instance of it)" +
				"or enable compatibility mode by setting displaytype=MSAndroidOpenGLDisplay in the [video] section your linphonerc factory configuration file" +
				"so you can keep using your existing application code for managing video views.");
		}

		TextureView textureView = (TextureView)view;
		textureView.setSurfaceTextureListener(new TextureView.SurfaceTextureListener() {
			@Override
			public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
				Log.i("[Platform Helper] Rendering window surface is available");
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
					Log.i("[Platform Helper] Rendering window surface is no longer available");
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
			Log.i("[Platform Helper] Rendering window surface is available");
			mSurfaceTexture = textureView.getSurfaceTexture();
			mSurface = new Surface(mSurfaceTexture);
			setNativeVideoWindowId(mNativePtr, mSurface);
		}
	}

	public synchronized void resizeVideoPreview(int width, int height) {
		if (mPreviewTextureView != null) {
			Log.i("[Platform Helper] Video preview size is now: " + width + "x" + height);
			ViewGroup.LayoutParams lp = mPreviewTextureView.getLayoutParams();
			Log.i("[Platform Helper] Preview layout params are: " + lp.width + ", " + lp.height);

			int maxWidth, maxHeight;
			if (lp.width == ViewGroup.LayoutParams.MATCH_PARENT) {
				maxWidth = mPreviewTextureView.getWidth();
			} else if (lp.width == ViewGroup.LayoutParams.WRAP_CONTENT) {
				maxWidth = width;
			} else {
				maxWidth = lp.width;
			}
			if (lp.height == ViewGroup.LayoutParams.MATCH_PARENT) {
				maxHeight = mPreviewTextureView.getHeight();
			} else if (lp.height == ViewGroup.LayoutParams.WRAP_CONTENT) {
				maxHeight = height;
			} else {
				maxHeight = lp.height;
			}
			Log.i("[Platform Helper] Preview max width: " + maxWidth + ", max height: " + maxHeight);
			if (maxWidth == 0 || maxHeight == 0) {
				Log.w("[Platform Helper] Abort resizing preview so that it won't be visible");
				return;
			}

			// A MATCH_PARENT will take over a WRAP_CONTENT or a fixed size and maintain ratio
			if (lp.width == ViewGroup.LayoutParams.MATCH_PARENT && lp.height != ViewGroup.LayoutParams.MATCH_PARENT) {
				lp.width = maxWidth;
				lp.height = height * maxWidth / width;
			} else if (lp.height == ViewGroup.LayoutParams.MATCH_PARENT && lp.width != ViewGroup.LayoutParams.MATCH_PARENT) {
				lp.height = maxHeight;
				lp.width = width * maxHeight / height;
			}
			// A WRAP_CONTENT won't be used if a fixed size is given for the other constraint 
			else if (lp.width == ViewGroup.LayoutParams.WRAP_CONTENT && lp.height != ViewGroup.LayoutParams.WRAP_CONTENT) {
				lp.height = maxHeight;
				lp.width = width * maxHeight / height;
			} else if (lp.height == ViewGroup.LayoutParams.WRAP_CONTENT && lp.width != ViewGroup.LayoutParams.WRAP_CONTENT) {
				lp.width = maxWidth;
				lp.height = height * maxWidth / width;
			} else {
				if (width < height) {
					lp.width = maxWidth;
					lp.height = height * maxWidth / width;
				} else {
					lp.height = maxHeight;
					lp.width = width * maxHeight / height;
				}
			}
			
			Log.i("[Platform Helper] Preview layout params updated to: " + lp.width + ", " + lp.height);
			mPreviewTextureView.setLayoutParams(lp);
		} else {
			Log.w("[Platform Helper] Couldn't resize video preview to: " + width + "x" + height + ", no texture view found");
		}
	}

	public synchronized Handler getHandler() {
		return mMainHandler;
	}

	public synchronized boolean isInBackground() {
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

	public synchronized void updateDnsServers(List<InetAddress> inetServers) {
		if (inetServers == null) {
			Log.e("[Platform Helper] inet servers list is null, don't update DNS servers");
			return;
		}

		int i = 0;
		String[] servers = new String[inetServers.size()];
		for (InetAddress address : inetServers) {
			String host = address.getHostAddress();
			servers[i++] = host;
			Log.i("[Platform Helper] Adding " + host + " to DNS servers list");
		}

		mDnsServers = servers;
	}

	private synchronized void storeDnsServers(Network activeNetwork) {
		mDnsServers = null;

		if (activeNetwork == null || mConnectivityManager.getLinkProperties(activeNetwork) == null) {
			Log.e("[Platform Helper] Active network is null or we can't get it's link properties");
			return;
		}

		List<InetAddress> inetServers = null;
		inetServers = mConnectivityManager.getLinkProperties(activeNetwork).getDnsServers();
		updateDnsServers(inetServers);
	}

	public synchronized void updateNetworkReachability() {
		if (mNativePtr == 0) {
			Log.w("[Platform Helper] Native pointer has been reset, stopping there");
			return;
		}
	
		if (mDozeModeEnabled) {
			Log.i("[Platform Helper] Device in idle mode: shutting down network");
			setNetworkReachable(mNativePtr, false);
			return;
		}

		boolean connected = mNetworkManager.isCurrentlyConnected(mContext);
		if (!connected) {
			Log.i("[Platform Helper] No connectivity: setting network unreachable");
			setNetworkReachable(mNativePtr, false);
		} else {
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
			Network network = mNetworkManager.getActiveNetwork();
			storeDnsServers(network);

			int currentNetworkType = networkInfo.getType();
			if (mLastNetworkType != -1 && mLastNetworkType != currentNetworkType) {
				Log.i("[Platform Helper] Network type has changed (last one was " + networkTypeToString(mLastNetworkType) + "), disabling network reachability first");
				setNetworkReachable(mNativePtr, false);
			}

			mLastNetworkType = currentNetworkType;
			Log.i("[Platform Helper] Network reachability enabled");
			setNetworkReachable(mNativePtr, true);
		}
	}

	public synchronized void setDozeModeEnabled(boolean b) {
		mDozeModeEnabled = b;
		Log.i("[Platform Helper] Device idle mode: " + mDozeModeEnabled);
	}

	public synchronized void setInteractiveMode(boolean b) {
		mIsInInteractiveMode = b;
		Log.i("[Platform Helper] Device interactive mode: " + mIsInInteractiveMode);
		enableKeepAlive(mNativePtr, mIsInInteractiveMode);
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
			mDozeIntentFilter = new IntentFilter();
			mDozeIntentFilter.addAction(PowerManager.ACTION_DEVICE_IDLE_MODE_CHANGED);
			mDozeReceiver = new DozeReceiver(this);
			Log.i("[Platform Helper] Registering doze receiver");
			mContext.registerReceiver(mDozeReceiver, mDozeIntentFilter);
		}

		mInteractivityReceiver = new InteractivityReceiver(this);
		mInteractivityIntentFilter = new IntentFilter(Intent.ACTION_SCREEN_ON);
        mInteractivityIntentFilter.addAction(Intent.ACTION_SCREEN_OFF);
		Log.i("[Platform Helper] Registering interactivity receiver");
		mContext.registerReceiver(mInteractivityReceiver, mInteractivityIntentFilter);

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
};


