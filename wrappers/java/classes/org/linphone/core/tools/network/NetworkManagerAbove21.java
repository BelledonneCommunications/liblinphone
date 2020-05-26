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

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.net.NetworkRequest;
import android.net.RouteInfo;

import org.linphone.core.tools.AndroidPlatformHelper;
import org.linphone.core.tools.Log;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.List;

/**
 * Intercept network state changes and update linphone core.
 */
public class NetworkManagerAbove21 implements NetworkManagerInterface {
    private AndroidPlatformHelper mHelper;
    private ConnectivityManager mConnectivityManager;
    private ConnectivityManager.NetworkCallback mNetworkCallback;
    private Network mNetworkAvailable;
    private Network mLastNetworkAvailable;
    private boolean mWifiOnly;

    public NetworkManagerAbove21(final AndroidPlatformHelper helper, ConnectivityManager cm, boolean wifiOnly) {
        mHelper = helper;
        mConnectivityManager = cm;
        mWifiOnly = wifiOnly;
        mNetworkAvailable = null;
        mLastNetworkAvailable = null;
        mNetworkCallback = new ConnectivityManager.NetworkCallback() {
            @Override
            public void onAvailable(final Network network) {
                mHelper.getHandler().post(new Runnable() {
                    @Override
                    public void run() {
                        NetworkInfo info = mConnectivityManager.getNetworkInfo(network);
                        if (info == null) {
                            Log.e("[Platform Helper] [Network Manager 21] A network should be available but getNetworkInfo failed.");
                            return;
                        }

                        Log.i("[Platform Helper] [Network Manager 21] A network is available: " + info.getTypeName() + ", wifi only is " + (mWifiOnly ? "enabled" : "disabled"));
                        if (!mWifiOnly || info.getType() == ConnectivityManager.TYPE_WIFI || info.getType() == ConnectivityManager.TYPE_ETHERNET) {
                            mNetworkAvailable = network;
                            mHelper.updateNetworkReachability();
                        } else {
                            Log.i("[Platform Helper] [Network Manager 21] Network isn't wifi and wifi only mode is enabled");
                            if (mWifiOnly) {
                                mLastNetworkAvailable = network;
                            }
                        }
                    }
                });
            }

            @Override
            public void onLost(final Network network) {
                mHelper.getHandler().post(new Runnable() {
                    @Override
                    public void run() {
                        Log.i("[Platform Helper] [Network Manager 21] A network has been lost");
                        if (mNetworkAvailable != null && mNetworkAvailable.equals(network)) {
                            mNetworkAvailable = null;
                        }
                        if (mLastNetworkAvailable != null && mLastNetworkAvailable.equals(network)) {
                            mLastNetworkAvailable = null;
                        }
                        mHelper.updateNetworkReachability();
                    }
                });
            }

            @Override
            public void onCapabilitiesChanged(final Network network, final NetworkCapabilities networkCapabilities) {
                mHelper.getHandler().post(new Runnable() {
                    @Override
                    public void run() {
                        if (networkCapabilities == null) {
                            Log.e("[Platform Helper] [Network Manager 21] onCapabilitiesChanged called with null networkCapabilities, skipping...");
                            return;
                        }
                        if (networkCapabilities.hasTransport(NetworkCapabilities.TRANSPORT_WIFI)) {
                            // This callback can be called very often when on WIFI (for example on signal strenght change), so don't log it each time and no need to update network reachability
                            Log.d("[Platform Helper] [Network Manager 21] onCapabilitiesChanged " + networkCapabilities.toString());
                        } else {
                            Log.i("[Platform Helper] [Network Manager 21] onCapabilitiesChanged " + networkCapabilities.toString());
                            mHelper.updateNetworkReachability();
                        }
                    }
                });
            }

            @Override
            public void onLinkPropertiesChanged(final Network network, final LinkProperties linkProperties) {
                mHelper.getHandler().post(new Runnable() {
                    @Override
                    public void run() {
                        if (linkProperties == null) {
                            Log.e("[Platform Helper] [Network Manager 21] onLinkPropertiesChanged called with null linkProperties, skipping...");
                            return;
                        }
                        Log.i("[Platform Helper] [Network Manager 21] onLinkPropertiesChanged " + linkProperties.toString());
                        updateDnsServers();
                    }
                });
            }

            @Override
            public void onLosing(final Network network, final int maxMsToLive) {
                mHelper.getHandler().post(new Runnable() {
                    @Override
                    public void run() {
                        Log.i("[Platform Helper] [Network Manager 21] onLosing");
                    }
                });
            }

            @Override
            public void onUnavailable() {
                mHelper.getHandler().post(new Runnable() {
                    @Override
                    public void run() {
                        Log.i("[Platform Helper] [Network Manager 21] onUnavailable");
                    }
                });
            }
        };
    }

    public void setWifiOnly(boolean isWifiOnlyEnabled) {
        mWifiOnly = isWifiOnlyEnabled;
        if (mWifiOnly && mNetworkAvailable != null) {
            NetworkInfo networkInfo = mConnectivityManager.getNetworkInfo(mNetworkAvailable);
            if (networkInfo != null && networkInfo.getType() != ConnectivityManager.TYPE_WIFI && networkInfo.getType() != ConnectivityManager.TYPE_ETHERNET) {
                Log.i("[Platform Helper] [Network Manager 21] Wifi only mode enabled and current network isn't wifi or ethernet");
                mLastNetworkAvailable = mNetworkAvailable;
                mNetworkAvailable = null;
            }
        } else if (!mWifiOnly && mNetworkAvailable == null) {
            Log.i("[Platform Helper] [Network Manager 21] Wifi only mode disabled, restoring previous network");
            mNetworkAvailable = mLastNetworkAvailable;
            mLastNetworkAvailable = null;
        }
    }

    public void registerNetworkCallbacks(Context context) {
        mConnectivityManager.registerNetworkCallback(
                new NetworkRequest.Builder().build(),
                mNetworkCallback
        );
    }

    public void unregisterNetworkCallbacks(Context context) {
        mConnectivityManager.unregisterNetworkCallback(mNetworkCallback);
    }

    public NetworkInfo getActiveNetworkInfo() {
        if (mNetworkAvailable != null) {
            return mConnectivityManager.getNetworkInfo(mNetworkAvailable);
        }

        return mConnectivityManager.getActiveNetworkInfo();
    }

    public Network getActiveNetwork() {
        return mNetworkAvailable;
    }

    public boolean isCurrentlyConnected(Context context) {
        return mNetworkAvailable != null;
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
        ArrayList<String> dnsServers = new ArrayList<>();
        ArrayList<String> activeNetworkDnsServers = new ArrayList<>();

        if (mConnectivityManager != null) {
            NetworkInfo activeNetworkInfo = mConnectivityManager.getActiveNetworkInfo();
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
                                if (networkInfo.equals(activeNetworkInfo)) {
                                    Log.i("[Platform Helper] [Network Manager 21] Found DNS host " + dnsHost + " from active network " + networkType);
                                    activeNetworkDnsServers.add(dnsHost);
                                } else {
                                    if (prioritary) {
                                        Log.i("[Platform Helper] [Network Manager 21] Found DNS host " + dnsHost + " from network " + networkType + " with default route");
                                        dnsServers.add(0, dnsHost);
                                    } else {
                                        Log.i("[Platform Helper] [Network Manager 21] Found DNS host " + dnsHost + " from network " + networkType);
                                        dnsServers.add(dnsHost);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        activeNetworkDnsServers.addAll(dnsServers);
        mHelper.updateDnsServers(activeNetworkDnsServers);
    }
}
