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

import android.annotation.TargetApi;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.Network;
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
				Log.i("[Platform Helper] Network is available (24)");
				mHelper.postNetworkUpdateRunner();
			}

			@Override
			public void onLost(Network network) {
				Log.i("[Platform Helper] Network is lost (24)");
				mHelper.postNetworkUpdateRunner();
			}
		};
	}

	public void registerNetworkCallbacks(ConnectivityManager connectivityManager) {
		connectivityManager.registerDefaultNetworkCallback(mNetworkCallback);
	}

	public void unregisterNetworkCallbacks(ConnectivityManager connectivityManager) {
		connectivityManager.unregisterNetworkCallback(mNetworkCallback);
	}
}
