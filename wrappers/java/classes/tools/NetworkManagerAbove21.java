/*
NetworkManagerAbove21.java
Copyright (C) 2017 Belledonne Communications, Grenoble, France

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

import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkInfo;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.os.Build;

import org.linphone.core.tools.AndroidPlatformHelper;

/**
 * Intercept network state changes and update linphone core.
 */
public class NetworkManagerAbove21 implements NetworkManagerInterface {
	private AndroidPlatformHelper mHelper;
	private ConnectivityManager.NetworkCallback mNetworkCallback;

	public NetworkManagerAbove21(final AndroidPlatformHelper helper) {
		mHelper = helper;
		mNetworkCallback = new ConnectivityManager.NetworkCallback() {
			@Override
			public void onAvailable(Network network) {
				Log.i("[Platform Helper] [Network Manager 21] A network is available");
				mHelper.updateNetworkReachability();
			}

			@Override
			public void onLost(Network network) {
				Log.i("[Platform Helper] [Network Manager 21] A network has been lost");
				mHelper.updateNetworkReachability();
			}

			@Override
			public void onCapabilitiesChanged(Network network, NetworkCapabilities networkCapabilities) {
				Log.i("[Platform Helper] [Network Manager 21] onCapabilitiesChanged " + network.toString() + ", " + networkCapabilities.toString());
				mHelper.updateNetworkReachability();
			}

			@Override
			public void onLinkPropertiesChanged(Network network, LinkProperties linkProperties) {
				Log.i("[Platform Helper] [Network Manager 21] onLinkPropertiesChanged " + network.toString() + ", " + linkProperties.toString());
			}

			@Override
			public void onLosing(Network network, int maxMsToLive) {
				Log.i("[Platform Helper] [Network Manager 21] onLosing " + network.toString());
			}

			@Override
			public void onUnavailable() {
				Log.i("[Platform Helper] [Network Manager 21] onUnavailable");
			}
		};
	}

	public void registerNetworkCallbacks(Context context, ConnectivityManager connectivityManager) {
		connectivityManager.registerNetworkCallback(
			new NetworkRequest.Builder().build(),
			mNetworkCallback
		);
	}

	public void unregisterNetworkCallbacks(Context context, ConnectivityManager connectivityManager) {
		connectivityManager.unregisterNetworkCallback(mNetworkCallback);
	}

    public NetworkInfo getActiveNetworkInfo(ConnectivityManager connectivityManager) {
        return connectivityManager.getActiveNetworkInfo();
    }

    public Network getActiveNetwork(ConnectivityManager connectivityManager) {
        return null;
    }

    public boolean isCurrentlyConnected(Context context, ConnectivityManager connectivityManager, boolean wifiOnly) {
		Network[] networks = connectivityManager.getAllNetworks();
		boolean connected = false;
		for (Network network : networks) {
			NetworkInfo networkInfo = connectivityManager.getNetworkInfo(network);
			Log.i("[Platform Helper] [Network Manager 21] Found network type: " + networkInfo.getTypeName() + ", isConnectedOrConnecting() = " + networkInfo.isConnectedOrConnecting());
			if (networkInfo.isConnectedOrConnecting()) {
				Log.i("[Platform Helper] [Network Manager 21] Network state is " + networkInfo.getState() + " / " + networkInfo.getDetailedState());
				if (networkInfo.getType() != ConnectivityManager.TYPE_WIFI && wifiOnly) {
					Log.i("[Platform Helper] [Network Manager 21] Wifi only mode enabled, skipping");
				} else {
					NetworkCapabilities capabilities = connectivityManager.getNetworkCapabilities(network);
					if (capabilities != null) {
						Log.i("[Platform Helper] [Network Manager 21] Network capabilities are " + capabilities.toString());
						connected = capabilities.hasCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET) 
							&& capabilities.hasCapability(NetworkCapabilities.NET_CAPABILITY_NOT_RESTRICTED);
					}
				}
			}
		}
		return connected;
    }

    public boolean hasHttpProxy(Context context, ConnectivityManager connectivityManager) {
        return false;
    }

    public String getProxyHost(Context context, ConnectivityManager connectivityManager) {
        return null;
    }

    public int getProxyPort(Context context, ConnectivityManager connectivityManager) {
        return 0;
    }
}
