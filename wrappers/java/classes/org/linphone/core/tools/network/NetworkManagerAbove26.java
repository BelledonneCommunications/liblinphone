/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
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

package org.linphone.core.tools.network;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.net.ConnectivityManager;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.net.ProxyInfo;
import android.net.RouteInfo;

import org.linphone.core.tools.AndroidPlatformHelper;
import org.linphone.core.tools.Log;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.List;

/**
 * Intercept network state changes and update linphone core.
 */
public class NetworkManagerAbove26 implements NetworkManagerInterface {
    private AndroidPlatformHelper mHelper;
    private ConnectivityManager mConnectivityManager;
    private ConnectivityManager.NetworkCallback mNetworkCallback;
    private Network mNetworkAvailable;
    private Network mLastNetworkAvailable;
    private boolean mWifiOnly;

    public NetworkManagerAbove26(final AndroidPlatformHelper helper, ConnectivityManager cm, boolean wifiOnly) {
        mHelper = helper;
        mConnectivityManager = cm;
        mWifiOnly = wifiOnly;
        mNetworkAvailable = null;
        mLastNetworkAvailable = null;
        mNetworkCallback = new ConnectivityManager.NetworkCallback() {
            @Override
            public void onAvailable(Network network) {
                NetworkInfo info = mConnectivityManager.getNetworkInfo(network);
                if (info == null) {
                    Log.e("[Platform Helper] [Network Manager 26] A network should be available but getNetworkInfo failed.");
                    return;
                }

                Log.i("[Platform Helper] [Network Manager 26] A network is available: " + info.getTypeName() + ", wifi only is " + (mWifiOnly ? "enabled" : "disabled"));
                if (!mWifiOnly || info.getType() == ConnectivityManager.TYPE_WIFI || info.getType() == ConnectivityManager.TYPE_ETHERNET) {
                    mNetworkAvailable = network;
                    mHelper.updateNetworkReachability();
                } else {
                    Log.i("[Platform Helper] [Network Manager 26] Network isn't wifi and wifi only mode is enabled");
                    if (mWifiOnly) {
                        mLastNetworkAvailable = network;
                    }
                }
            }

            @Override
            public void onLost(Network network) {
                Log.i("[Platform Helper] [Network Manager 26] A network has been lost");
                if (mNetworkAvailable != null && mNetworkAvailable.equals(network)) {
                    mNetworkAvailable = null;
                }
                if (mLastNetworkAvailable != null && mLastNetworkAvailable.equals(network)) {
                    mLastNetworkAvailable = null;
                }
                mHelper.updateNetworkReachability();
            }

            @Override
            public void onCapabilitiesChanged(Network network, NetworkCapabilities networkCapabilities) {
                if (networkCapabilities == null) {
                    Log.e("[Platform Helper] [Network Manager 26] onCapabilitiesChanged called with null networkCapabilities, skipping...");
                    return;
                }
                if (networkCapabilities.hasTransport(NetworkCapabilities.TRANSPORT_WIFI)) {
                    // This callback can be called very often when on WIFI (for example on signal strenght change), so don't log it each time and no need to update network reachability
                    Log.d("[Platform Helper] [Network Manager 26] onCapabilitiesChanged " + networkCapabilities.toString());
                } else {
                    Log.i("[Platform Helper] [Network Manager 26] onCapabilitiesChanged " + networkCapabilities.toString());
                    mHelper.updateNetworkReachability();
                }
            }

            @Override
            public void onLinkPropertiesChanged(Network network, LinkProperties linkProperties) {
                if (linkProperties == null) {
                    Log.e("[Platform Helper] [Network Manager 26] onLinkPropertiesChanged called with null linkProperties, skipping...");
                    return;
                }
                Log.i("[Platform Helper] [Network Manager 26] onLinkPropertiesChanged " + linkProperties.toString());
                updateDnsServers();
            }

            @Override
            public void onLosing(Network network, int maxMsToLive) {
                Log.i("[Platform Helper] [Network Manager 26] onLosing");
            }

            @Override
            public void onUnavailable() {
                Log.i("[Platform Helper] [Network Manager 26] onUnavailable");
            }
        };
    }

    public void setWifiOnly(boolean isWifiOnlyEnabled) {
        mWifiOnly = isWifiOnlyEnabled;
        if (mWifiOnly && mNetworkAvailable != null) {
            NetworkInfo networkInfo = mConnectivityManager.getNetworkInfo(mNetworkAvailable);
            if (networkInfo != null && networkInfo.getType() != ConnectivityManager.TYPE_WIFI && networkInfo.getType() != ConnectivityManager.TYPE_ETHERNET) {
                Log.i("[Platform Helper] [Network Manager 26] Wifi only mode enabled and current network isn't wifi or ethernet");
                mLastNetworkAvailable = mNetworkAvailable;
                mNetworkAvailable = null;
            }
        } else if (!mWifiOnly && mNetworkAvailable == null) {
            Log.i("[Platform Helper] [Network Manager 26] Wifi only mode disabled, restoring previous network");
            mNetworkAvailable = mLastNetworkAvailable;
            mLastNetworkAvailable = null;
        }
    }

    public void registerNetworkCallbacks(Context context) {
        int permissionGranted = context.getPackageManager().checkPermission(Manifest.permission.ACCESS_NETWORK_STATE, context.getPackageName());
        Log.i("[Platform Helper] [Network Manager 26] ACCESS_NETWORK_STATE permission is " + (permissionGranted == PackageManager.PERMISSION_GRANTED ? "granted" : "denied"));
        if (permissionGranted == PackageManager.PERMISSION_GRANTED) {
            mConnectivityManager.registerDefaultNetworkCallback(mNetworkCallback, mHelper.getHandler());
        }
    }

    public void unregisterNetworkCallbacks(Context context) {
        mConnectivityManager.unregisterNetworkCallback(mNetworkCallback);
    }

    public NetworkInfo getActiveNetworkInfo() {
        if (mNetworkAvailable != null) {
            return mConnectivityManager.getNetworkInfo(mNetworkAvailable);
        }

        Network network = mConnectivityManager.getActiveNetwork();
        if (network != null) {
            return mConnectivityManager.getNetworkInfo(network);
        }
        Log.i("[Platform Helper] [Network Manager 26] getActiveNetwork() returned null, using getActiveNetworkInfo() instead");
        return mConnectivityManager.getActiveNetworkInfo();
    }

    public Network getActiveNetwork() {
        if (mNetworkAvailable != null) {
            return mNetworkAvailable;
        }

        return mConnectivityManager.getActiveNetwork();
    }

    public boolean isCurrentlyConnected(Context context) {
        int restrictBackgroundStatus = mConnectivityManager.getRestrictBackgroundStatus();
        if (restrictBackgroundStatus == ConnectivityManager.RESTRICT_BACKGROUND_STATUS_ENABLED) {
            // Device is restricting metered network activity while application is running on background.
            // In this state, application should not try to use the network while running on background, because it would be denied.
            Log.w("[Platform Helper] [Network Manager 26] Device is restricting metered network activity while application is running on background");
            if (mHelper.isInBackground()) {
                Log.w("[Platform Helper] [Network Manager 26] Device is in background, returning false");
                return false;
            }
        }
        return mNetworkAvailable != null;
    }

    public boolean hasHttpProxy(Context context) {
        ProxyInfo proxy = mConnectivityManager.getDefaultProxy();
        if (proxy != null && proxy.getHost() != null) {
            Log.i("[Platform Helper] [Network Manager 26] The active network is using an http proxy: " + proxy.toString());
            return true;
        }
        return false;
    }

    public String getProxyHost(Context context) {
        ProxyInfo proxy = mConnectivityManager.getDefaultProxy();
        return proxy.getHost();
    }

    public int getProxyPort(Context context) {
        ProxyInfo proxy = mConnectivityManager.getDefaultProxy();
        return proxy.getPort();
    }

    private boolean hasLinkPropertiesDefaultRoute(LinkProperties linkProperties) {
        if (linkProperties != null) {
            for (RouteInfo route : linkProperties.getRoutes()) {
                if (route.isDefaultRoute()) {
                    return true;
                }
            }
        }
        return false;
    }

    public void updateDnsServers() {
        ArrayList<String> activeNetworkDnsServers = new ArrayList<>();

        if (mConnectivityManager != null) {
            Network activeNetwork = mConnectivityManager.getActiveNetwork();
            for (Network network : mConnectivityManager.getAllNetworks()) {
                NetworkInfo networkInfo = mConnectivityManager.getNetworkInfo(network);
                if (networkInfo != null) {
                    LinkProperties linkProperties = mConnectivityManager.getLinkProperties(network);
                    if (linkProperties != null) {
                        List<InetAddress> dnsServersList = linkProperties.getDnsServers();
                        boolean prioritary = hasLinkPropertiesDefaultRoute(linkProperties);
                        for (InetAddress dnsServer : dnsServersList) {
                            String dnsHost = dnsServer.getHostAddress();
                            if (!dnsServers.contains(dnsHost) && !activeNetworkDnsServers.contains(dnsHost)) {
                                String networkType = networkInfo.getTypeName();
                                if (network.equals(activeNetwork)) {
                                    Log.i("[Platform Helper] [Network Manager 26] Found DNS host " + dnsHost + " from active network " + networkType);
                                    activeNetworkDnsServers.add(dnsHost);
                                }
                            }
                        }
                    }
                }
            }
        }
        mHelper.updateDnsServers(activeNetworkDnsServers);
    }
}
