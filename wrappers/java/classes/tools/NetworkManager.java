/*
NetworkManager.java
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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkInfo;

import org.linphone.core.tools.AndroidPlatformHelper;

/**
 * Intercept network state changes and update linphone core.
 */
public class NetworkManager extends BroadcastReceiver implements NetworkManagerInterface {
    private AndroidPlatformHelper mHelper;
	private IntentFilter mNetworkIntentFilter;
    private ConnectivityManager mConnectivityManager;

    public NetworkManager(AndroidPlatformHelper helper) {
        mHelper = helper;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        mConnectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        Log.i("[Platform Helper] [Network Manager] Broadcast receiver called");
        if (mHelper != null) {
            mHelper.updateNetworkReachability();
        }
    }

	public void registerNetworkCallbacks(Context context, ConnectivityManager connectivityManager) {
		mNetworkIntentFilter = new IntentFilter(ConnectivityManager.CONNECTIVITY_ACTION);
        context.registerReceiver(this, mNetworkIntentFilter);
	}

	public void unregisterNetworkCallbacks(Context context, ConnectivityManager connectivityManager) {
        context.unregisterReceiver(this);
	}

    public NetworkInfo getActiveNetworkInfo(ConnectivityManager connectivityManager) {
        return connectivityManager.getActiveNetworkInfo();
    }

    public Network getActiveNetwork(ConnectivityManager connectivityManager) {
        return null;
    }

    public boolean isCurrentlyConnected(Context context, ConnectivityManager connectivityManager, boolean wifiOnly) {
        NetworkInfo[] networkInfos = connectivityManager.getAllNetworkInfo();
		boolean connected = false;
        for (NetworkInfo networkInfo : networkInfos) {
            Log.i("[Platform Helper] [Network Manager] Found network type: " + networkInfo.getTypeName() + ", isConnectedOrConnecting() = " + networkInfo.isConnectedOrConnecting());
			if (networkInfo.isConnectedOrConnecting()) {
				Log.i("[Platform Helper] [Network Manager] Network state is " + networkInfo.getState() + " / " + networkInfo.getDetailedState());
				if (networkInfo.getType() != ConnectivityManager.TYPE_WIFI && wifiOnly) {
					Log.i("[Platform Helper] [Network Manager] Wifi only mode enabled, skipping");
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
