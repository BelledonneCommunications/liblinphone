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
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.ConnectivityManager;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkInfo;
import android.net.NetworkCapabilities;
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
				mHelper.updateNetworkReachability();
			}

			@Override
			public void onLost(Network network) {
				Log.i("[Platform Helper] [Network Manager 24] A network has been lost");
				mHelper.updateNetworkReachability();
			}

			@Override
			public void onCapabilitiesChanged(Network network, NetworkCapabilities networkCapabilities) {
				Log.i("[Platform Helper] [Network Manager 24] onCapabilitiesChanged " + network.toString() + ", " + networkCapabilities.toString());
				mHelper.updateNetworkReachability();
			}

			@Override
			public void onLinkPropertiesChanged(Network network, LinkProperties linkProperties) {
				Log.i("[Platform Helper] [Network Manager 24] onLinkPropertiesChanged " + network.toString() + ", " + linkProperties.toString());
			}

			@Override
			public void onLosing(Network network, int maxMsToLive) {
				Log.i("[Platform Helper] [Network Manager 24] onLosing " + network.toString());
			}

			@Override
			public void onUnavailable() {
				Log.i("[Platform Helper] [Network Manager 24] onUnavailable");
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

    public NetworkInfo getActiveNetworkInfo(ConnectivityManager connectivityManager) {
        Network network = connectivityManager.getActiveNetwork();
		if (network != null) {
			return connectivityManager.getNetworkInfo(network);
		}
		Log.i("[Platform Helper] [Network Manager 24] getActiveNetwork() returned null, using getActiveNetworkInfo() instead");
        return connectivityManager.getActiveNetworkInfo();
    }

    public Network getActiveNetwork(ConnectivityManager connectivityManager) {
        return connectivityManager.getActiveNetwork();
    }

	public boolean isCurrentlyConnected(Context context, ConnectivityManager connectivityManager, boolean wifiOnly) {
		int restrictBackgroundStatus = connectivityManager.getRestrictBackgroundStatus();
		if (restrictBackgroundStatus == ConnectivityManager.RESTRICT_BACKGROUND_STATUS_ENABLED) {
			// Device is restricting metered network activity while application is running on background.
			// In this state, application should not try to use the network while running on background, because it would be denied.
			Log.w("[Platform Helper] [Network Manager 24] Device is restricting metered network activity while application is running on background");
			if (mHelper.isInBackground()) {
				Log.w("[Platform Helper] [Network Manager 24] Device is in background, returning false");
				return false;
			}
		}

		Network[] networks = connectivityManager.getAllNetworks();
		boolean connected = false;
		for (Network network : networks) {
			NetworkInfo networkInfo = connectivityManager.getNetworkInfo(network);
			Log.i("[Platform Helper] [Network Manager 24] Found network type: " + networkInfo.getTypeName() + ", isConnectedOrConnecting() = " + networkInfo.isConnectedOrConnecting());
			if (networkInfo.isConnectedOrConnecting()) {
				Log.i("[Platform Helper] [Network Manager 24] Network is state is " + networkInfo.getState() + " / " + networkInfo.getDetailedState());
				if (networkInfo.getType() != ConnectivityManager.TYPE_WIFI && wifiOnly) {
					Log.i("[Platform Helper] [Network Manager 24] Wifi only mode enabled, skipping");
				} else {
					NetworkCapabilities capabilities = connectivityManager.getNetworkCapabilities(network);
					if (capabilities != null) {
						Log.i("[Platform Helper] [Network Manager 24] Network capabilities are " + capabilities.toString());
						connected = capabilities.hasCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET) 
							&& capabilities.hasCapability(NetworkCapabilities.NET_CAPABILITY_NOT_RESTRICTED)
							&& capabilities.hasCapability(NetworkCapabilities.NET_CAPABILITY_VALIDATED);
					}
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
