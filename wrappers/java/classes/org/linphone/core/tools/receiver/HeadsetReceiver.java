/*
 * Copyright (c) 2010-2021 Belledonne Communications SARL.
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
package org.linphone.core.tools.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;

import org.linphone.core.tools.Log;
import org.linphone.core.tools.service.CoreManager;

public class HeadsetReceiver extends BroadcastReceiver {
    public HeadsetReceiver() {
        super();
        Log.i("[Headset] Headset receiver created");
    }


    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        Log.i("[Headset] Headset broadcast received");

        // https://developer.android.com/reference/android/media/AudioManager#ACTION_HEADSET_PLUG
        if (action.equals(AudioManager.ACTION_HEADSET_PLUG)) {
            int state = intent.getIntExtra("state", 0);
            String name = intent.getStringExtra("name");
            int hasMicrophone = intent.getIntExtra("microphone", 0);

            if (state == 0) {
                Log.i("[Headset] Headset disconnected: " + name);
            } else if (state == 1) {
                Log.i("[Headset] Headset connected: " + name);
                if (hasMicrophone == 1) {
                    Log.i("[Headset] Headset has a microphone");
                }
            } else {
                Log.w("[Headset] Unknown headset plugged state: " + state);
            }

            if (CoreManager.isReady()) CoreManager.instance().onHeadsetStateChanged();
        }
    }
}