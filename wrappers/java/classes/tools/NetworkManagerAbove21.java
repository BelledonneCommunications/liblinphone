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

import android.annotation.TargetApi;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkInfo;
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
				mHelper.postNetworkUpdateRunner();
			}

			@Override
			public void onLost(Network network) {
				Log.i("[Platform Helper] [Network Manager 21] A network is lost");
				mHelper.postNetworkUpdateRunner();
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

    public boolean isCurrentlyConnected(Context context, ConnectivityManager connectivityManager, boolean wifiOnly) {
		Network[] networks = connectivityManager.getAllNetworks();
		boolean connected = false;
		for (Network network : networks) {
			NetworkInfo networkInfo = connectivityManager.getNetworkInfo(network);
			Log.i("[Platform Helper] [Network Manager 21] Found network type: " + networkInfo.getTypeName());
			if (networkInfo.isAvailable() && networkInfo.isConnected()) {
				Log.i("[Platform Helper] [Network Manager 21] Network is available");
				if (networkInfo.getType() != ConnectivityManager.TYPE_WIFI && wifiOnly) {
					Log.i("[Platform Helper] [Network Manager 21] Wifi only mode enabled, skipping");
				} else {
					connected = true;
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
