/*
NetworkManagerAbove24.java
Copyright (C) 2019 Belledonne Communications, Grenoble, France

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

import android.Manifest;
import android.annotation.TargetApi;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkInfo;
import android.net.ProxyInfo;
import android.net.NetworkRequest;
import android.os.Build;

import org.linphone.core.tools.AndroidPlatformHelper;

/**
 * Intercept network state changes and update linphone core.
 */
public class NetworkManagerAbove24 implements NetworkManagerInterface {
	private AndroidPlatformHelper mHelper;
	private ConnectivityManager.NetworkCallback mNetworkCallback;

	public NetworkManagerAbove24(final AndroidPlatformHelper helper) {
		mHelper = helper;
		mNetworkCallback = new ConnectivityManager.NetworkCallback() {
			@Override
			public void onAvailable(Network network) {
				Log.i("[Platform Helper] [Network Manager 24] A network is available");
				mHelper.postNetworkUpdateRunner();
			}

			@Override
			public void onLost(Network network) {
				Log.i("[Platform Helper] [Network Manager 24] A network is lost");
				mHelper.postNetworkUpdateRunner();
			}
		};
	}

	public void registerNetworkCallbacks(Context context, ConnectivityManager connectivityManager) {
		int permissionGranted = context.getPackageManager().checkPermission(Manifest.permission.ACCESS_NETWORK_STATE, context.getPackageName());
		Log.i("[Platform Helper] [Network Manager 24] ACCESS_NETWORK_STATE permission is " + (permissionGranted == PackageManager.PERMISSION_GRANTED ? "granted" : "denied"));
		if (permissionGranted == PackageManager.PERMISSION_GRANTED) {
			connectivityManager.registerDefaultNetworkCallback(mNetworkCallback);
		}
	}

	public void unregisterNetworkCallbacks(Context context, ConnectivityManager connectivityManager) {
		connectivityManager.unregisterNetworkCallback(mNetworkCallback);
	}

    public boolean isCurrentlyConnected(Context context, ConnectivityManager connectivityManager, boolean wifiOnly) {
		int restrictBackgroundStatus = connectivityManager.getRestrictBackgroundStatus();
		if (restrictBackgroundStatus == ConnectivityManager.RESTRICT_BACKGROUND_STATUS_ENABLED) {
			// Device is restricting metered network activity while application is running on background.
			// In this state, application should not try to use the network while running on background, because it would be denied.
			Log.w("[Platform Helper] [Network Manager 24] Device is restricting metered network activity while application is running on background");
			if (mHelper.isInBackground()) {
				Log.w("[Platform Helper] [Network Manager 26] Device is in background, returning false");
				return false;
			}
		}

		Network[] networks = connectivityManager.getAllNetworks();
		boolean connected = false;
		for (Network network : networks) {
			NetworkInfo networkInfo = connectivityManager.getNetworkInfo(network);
			Log.i("[Platform Helper] [Network Manager 24] Found network type: " + networkInfo.getTypeName());
			if (networkInfo.isAvailable() && networkInfo.isConnected()) {
				Log.i("[Platform Helper] [Network Manager 24] Network is available");
				if (networkInfo.getType() != ConnectivityManager.TYPE_WIFI && wifiOnly) {
					Log.i("[Platform Helper] [Network Manager 24] Wifi only mode enabled, skipping");
				} else {
					connected = true;
				}
			}
		}
		return connected;
    }

    public boolean hasHttpProxy(Context context, ConnectivityManager connectivityManager) {
		ProxyInfo proxy = connectivityManager.getDefaultProxy();
		if (proxy != null && proxy.getHost() != null) {
			Log.i("[Platform Helper] [Network Manager 24] The active network is using a http proxy: " + proxy.toString());
			return true;
		}
		Log.i("[Platform Helper] [Network Manager 24] The active network isn't using a http proxy: " + proxy.toString());
		return false;
    }

    public String getProxyHost(Context context, ConnectivityManager connectivityManager) {
		ProxyInfo proxy = connectivityManager.getDefaultProxy();
		return proxy.getHost();
    }

    public int getProxyPort(Context context, ConnectivityManager connectivityManager) {
        ProxyInfo proxy = connectivityManager.getDefaultProxy();
		return proxy.getPort();
    }
}
