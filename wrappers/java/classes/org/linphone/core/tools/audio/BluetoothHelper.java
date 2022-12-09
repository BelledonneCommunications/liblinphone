/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
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

package org.linphone.core.tools.audio;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothHeadset;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioManager;

import org.linphone.core.tools.Log;
import org.linphone.core.tools.receiver.BluetoothReceiver;
import org.linphone.core.tools.service.CoreManager;

public class BluetoothHelper {
    private AudioManager mAudioManager;
    private BluetoothAdapter mBluetoothAdapter;
    private BluetoothReceiver mBluetoothReceiver;

    public BluetoothHelper(Context context) {
        mAudioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);

        BluetoothManager bluetoothManager = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = bluetoothManager.getAdapter();
        
        if (mBluetoothAdapter != null) {
            Log.i("[Bluetooth] Adapter found");
            if (mAudioManager.isBluetoothScoAvailableOffCall()) {
                Log.i("[Bluetooth] SCO available off call, continue");
            } else {
                Log.w("[Bluetooth] SCO not available off call !");
            }

            mBluetoothReceiver = new BluetoothReceiver();
            IntentFilter filter = new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED);
            filter.addAction(BluetoothHeadset.ACTION_CONNECTION_STATE_CHANGED);
            context.registerReceiver(mBluetoothReceiver, filter);
            Log.i("[Bluetooth] Bluetooth broadcast receiver registered");
        }
        
        Log.i("[Bluetooth] Bluetooth helper created");
    }

    public void destroy(Context context) {
        if (mBluetoothReceiver != null) {
            context.unregisterReceiver(mBluetoothReceiver);
            mBluetoothReceiver = null;
            Log.i("[Bluetooth] Bluetooth broadcast receiver unregistered");
        }

        mBluetoothAdapter = null;
        mAudioManager = null;
        Log.i("[Bluetooth] Bluetooth helper destroyed");
    }
}