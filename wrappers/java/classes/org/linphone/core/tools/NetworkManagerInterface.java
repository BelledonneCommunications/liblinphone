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

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkInfo;

public interface NetworkManagerInterface {
    void registerNetworkCallbacks(Context context);

	void unregisterNetworkCallbacks(Context context);

    boolean isCurrentlyConnected(Context context);

    NetworkInfo getActiveNetworkInfo();

    Network getActiveNetwork();

    boolean hasHttpProxy(Context context);

    String getProxyHost(Context context);

    int getProxyPort(Context context);

    void setWifiOnly(boolean isWifiOnlyEnabled);

    void updateDnsServers();
}
