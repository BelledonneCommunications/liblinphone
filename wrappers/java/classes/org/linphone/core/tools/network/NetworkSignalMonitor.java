/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone 
 * (see https://gitlab.linphone.org/BC/public/liblinphone).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

package org.linphone.core.tools.network;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Looper;
import android.telephony.CellSignalStrength;
import android.telephony.PhoneStateListener;
import android.telephony.SignalStrength;
import android.telephony.TelephonyManager;
import android.telephony.CellSignalStrengthCdma;
import android.telephony.CellSignalStrengthGsm;
import android.telephony.CellSignalStrengthLte;
import android.telephony.CellSignalStrengthNr;
import android.telephony.CellSignalStrengthTdscdma;
import android.telephony.CellSignalStrengthWcdma;

import java.lang.SecurityException;

import org.linphone.core.SignalType;
import org.linphone.core.SignalStrengthUnit;
import org.linphone.core.tools.AndroidPlatformHelper;
import org.linphone.core.tools.Log;
import org.linphone.mediastream.Version;

public class NetworkSignalMonitor {
    private AndroidPlatformHelper mHelper;
	private TelephonyManager mTelephonyManager;
	private WifiManager mWifiManager;
	private PhoneStateListener mPhoneStateListener;

	public NetworkSignalMonitor(Context context, final AndroidPlatformHelper helper) {
		mHelper = helper;

		Looper myLooper = Looper.myLooper();
        if (myLooper == null) {
			Log.e("[Platform Helper] [Signal Strength Monitor] Looper not available from this thread (have you called Looper.prepare()?), can't create PhoneStateListener");
		} else {
			mTelephonyManager = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
			if (mTelephonyManager != null) {
				mPhoneStateListener = new PhoneStateListener() {
					@Override
					public void onSignalStrengthsChanged(SignalStrength signalStrength) {		
						if (isCurrentNetworkCellular()) {
							updateCellConnectionSignalStrengthFromSignalStrength(signalStrength);
						}
					}
				};
				mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_SIGNAL_STRENGTHS);
			} else {
				Log.w("[Platform Helper] [Signal Strength Monitor] TelephonyManager is not available");
			}
		}

		mWifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
		if (mWifiManager == null) {
			Log.w("[Platform Helper] [Signal Strength Monitor] WifiManager is not available");
		}
		Log.i("[Platform Helper] [Signal Strength Monitor] Created");
	}

	public void destroy() {
		Log.i("[Platform Helper] [Signal Strength Monitor] Destroying");
		if (mTelephonyManager != null) {
			mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);
		}

		mHelper = null;
	}

	public void updateWifiConnectionSignalStrength() {
		if (mWifiManager == null) return;
		if (isCurrentNetworkCellular()) return;

		try {
			WifiInfo info = mWifiManager.getConnectionInfo(); // Deprecated in API 31
			int rssi = info.getRssi();
			Log.i("[Platform Helper] [Signal Strength Monitor] Wifi RSSI is [", rssi, "]");

			if (mHelper != null) {
				mHelper.setSignalInfo(SignalType.Wifi, SignalStrengthUnit.Rssi, rssi, String.valueOf(info.getNetworkId()));
			}
		} catch (SecurityException se) {
			// Requires "android.permission.ACCESS_WIFI_STATE" permission
			Log.e("[Platform Helper] [Signal Strength Monitor] Can't get Wifi signal strength: ", se, ", did you grant ACCESS_WIFI_STATE permission?");
		}
	}

	public void updateCellConnectionSignalStrength() {
		if (mTelephonyManager == null) return;
		if (!isCurrentNetworkCellular()) return;
		
		SignalStrength signalStrength = mTelephonyManager.getSignalStrength();
		updateCellConnectionSignalStrengthFromSignalStrength(signalStrength);
	}

	private void updateCellConnectionSignalStrengthFromSignalStrength(SignalStrength signalStrength) {
		if (signalStrength == null) {
			Log.w("[Platform Helper] [Signal Strength Monitor] Failed to get a valid cellular SignalStrength object!");
			return;
		}

		for (CellSignalStrength cellSignal : signalStrength.getCellSignalStrengths()) { // Added in API 29
			int dbm = cellSignal.getDbm();
			String networkType = "";
			if (cellSignal instanceof CellSignalStrengthCdma) {
				Log.i("[Platform Helper] [Signal Strength Monitor] Cdma cell type signal is [", dbm, "] dbm");
				networkType = "CDMA";
			} else if (cellSignal instanceof CellSignalStrengthGsm) {
				if (Version.sdkAboveOrEqual(Version.API30_ANDROID_11)) {
					CellSignalStrengthGsm gsm = (CellSignalStrengthGsm)cellSignal;
					int rssi = gsm.getRssi(); // Added in API 30!
					Log.i("[Platform Helper] [Signal Strength Monitor] GSM cell type signal is [", dbm, "] dbm, RSSI is [", rssi, "]");
				} else {
					Log.i("[Platform Helper] [Signal Strength Monitor] GSM cell type signal is [", dbm, "] dbm, RSSI is only available on Android 11 and newer");
				}
				networkType = "GSM";
			} else if (cellSignal instanceof CellSignalStrengthLte) {
				CellSignalStrengthLte lte = (CellSignalStrengthLte)cellSignal;
				int rssi = lte.getRssi();
				Log.i("[Platform Helper] [Signal Strength Monitor] LTE cell type signal is [", dbm, "] dbm, RSSI is [", rssi, "]");
				networkType = "LTE";
			} else if (cellSignal instanceof CellSignalStrengthNr) { // Added in API 29
				Log.i("[Platform Helper] [Signal Strength Monitor] 5G NR cell type signal is [", dbm, "] dbm");
				networkType = "5G NR";
			} else if (cellSignal instanceof CellSignalStrengthTdscdma) { // Added in API 29
				Log.i("[Platform Helper] [Signal Strength Monitor] Tdscdma cell type signal is [", dbm, "] dbm");
				networkType = "TdsCDMA";
			} else if (cellSignal instanceof CellSignalStrengthWcdma) {
				Log.i("[Platform Helper] [Signal Strength Monitor] Wcdma cell type signal is [", dbm, "] dbm");
				networkType = "WCDMA";
			} else {
				Log.i("[Platform Helper] [Signal Strength Monitor] Unkown cell type signal is [", dbm, "] dbm");
				networkType = "Unknown";
			}

			if (mHelper != null) {
				mHelper.setSignalInfo(SignalType.Mobile, SignalStrengthUnit.Dbm, dbm, networkType);
			}
		}
	}

	private boolean isCurrentNetworkCellular() {
		if (mHelper == null) return false;
		NetworkInfo currentNetworkInfo = mHelper.getActiveNetworkInfo();

		if (currentNetworkInfo != null) {
			int networkType = currentNetworkInfo.getType();
			String networkTypeName = currentNetworkInfo.getTypeName();
			Log.i("[Platform Helper] [Signal Strength Monitor] Current active network type is [", networkTypeName, "](", networkType, ")");
			return networkType == ConnectivityManager.TYPE_MOBILE;
		}

		Log.e("[Platform Helper] [Signal Strength Monitor] Can't get active network info, assuming cellular");
		return true;
	}
}