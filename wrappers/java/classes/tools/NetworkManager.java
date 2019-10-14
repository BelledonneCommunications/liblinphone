/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

package org.linphone.core.tools;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkInfo;
import java.util.ArrayList;

import org.linphone.core.tools.AndroidPlatformHelper;

/**
 * Intercept network state changes and update linphone core.
 */
public class NetworkManager extends BroadcastReceiver implements NetworkManagerInterface {
    private AndroidPlatformHelper mHelper;
	private IntentFilter mNetworkIntentFilter;
    private ConnectivityManager mConnectivityManager;
    private boolean mWifiOnly;

    public NetworkManager(AndroidPlatformHelper helper, ConnectivityManager cm, boolean wifiOnly) {
        mConnectivityManager = cm;
        mWifiOnly = wifiOnly;
        mHelper = helper;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.i("[Platform Helper] [Network Manager] Broadcast receiver called");
        if (mHelper != null) {
            mHelper.updateNetworkReachability();
        }
    }

    public void setWifiOnly(boolean isWifiOnlyEnabled) {
		mWifiOnly = isWifiOnlyEnabled;
	}

	public void registerNetworkCallbacks(Context context) {
		mNetworkIntentFilter = new IntentFilter(ConnectivityManager.CONNECTIVITY_ACTION);
        context.registerReceiver(this, mNetworkIntentFilter);
	}

	public void unregisterNetworkCallbacks(Context context) {
        context.unregisterReceiver(this);
	}

    public NetworkInfo getActiveNetworkInfo() {
        return mConnectivityManager.getActiveNetworkInfo();
    }

    public Network getActiveNetwork() {
        return null;
    }

    public boolean isCurrentlyConnected(Context context) {
        NetworkInfo[] networkInfos = mConnectivityManager.getAllNetworkInfo();
		boolean connected = false;
        for (NetworkInfo networkInfo : networkInfos) {
            Log.i("[Platform Helper] [Network Manager] Found network type: " + networkInfo.getTypeName() + ", isConnectedOrConnecting() = " + networkInfo.isConnectedOrConnecting());
			if (networkInfo.isConnectedOrConnecting()) {
				Log.i("[Platform Helper] [Network Manager] Network state is " + networkInfo.getState() + " / " + networkInfo.getDetailedState());
				if (networkInfo.getType() != ConnectivityManager.TYPE_WIFI && mWifiOnly) {
					Log.i("[Platform Helper] [Network Manager] Wifi only mode enabled, skipping");
				} else {
					return true;
				}
			}
        }
        return connected;
    }

    public boolean hasHttpProxy(Context context) {
        return false;
    }

    public String getProxyHost(Context context) {
        return null;
    }

    public int getProxyPort(Context context) {
        return 0;
    }

	public void updateDnsServers() {
		ArrayList<String> dnsServers = new ArrayList<>();
		mHelper.updateDnsServers(dnsServers);
	}
}
