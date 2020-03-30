/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
 *
 * This file is part of linphone-android
 * (see https://www.linphone.org).
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
package org.linphone.core.tools.receiver;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothHeadset;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;

import org.linphone.core.tools.Log;
import org.linphone.core.tools.service.CoreManager;

public class BluetoothReceiver extends BroadcastReceiver {
    public BluetoothReceiver() {
        super();
        Log.i("[Bluetooth] Bluetooth receiver created");
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        Log.i("[Bluetooth] Bluetooth broadcast received");

        if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
            int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR);
            switch (state) {
                case BluetoothAdapter.STATE_OFF:
                    Log.w("[Bluetooth] Adapter has been turned off");
                    break;
                case BluetoothAdapter.STATE_TURNING_OFF:
                    Log.w("[Bluetooth] Adapter is being turned off");
                    break;
                case BluetoothAdapter.STATE_ON:
                    Log.i("[Bluetooth] Adapter has been turned on");
                    break;
                case BluetoothAdapter.STATE_TURNING_ON:
                    Log.i("[Bluetooth] Adapter is being turned on");
                    break;
                case BluetoothAdapter.ERROR:
                    Log.e("[Bluetooth] Adapter is in error state !");
                    break;
                default:
                    Log.w("[Bluetooth] Unknown adapter state: ", state);
                    break;
            }
        } else if (action.equals(BluetoothHeadset.ACTION_CONNECTION_STATE_CHANGED)) {
            int state = intent.getIntExtra(BluetoothHeadset.EXTRA_STATE, BluetoothHeadset.STATE_DISCONNECTED);
            if (state == BluetoothHeadset.STATE_CONNECTED) {
                Log.i("[Bluetooth] Bluetooth headset connected");
                if (CoreManager.isReady()) CoreManager.instance().onBluetoothHeadsetStateChanged();
            } else if (state == BluetoothHeadset.STATE_DISCONNECTED) {
                Log.i("[Bluetooth] Bluetooth headset disconnected");
                if (CoreManager.isReady()) CoreManager.instance().onBluetoothHeadsetStateChanged();
            } else if (state == BluetoothHeadset.STATE_CONNECTING) {
                Log.i("[Bluetooth] Bluetooth headset connecting");
            } else {
                Log.w("[Bluetooth] Bluetooth headset unknown state changed: " + state);
            }
        } else {
            Log.w("[Bluetooth] Bluetooth unknown action: " + action);
        }
    }
}
