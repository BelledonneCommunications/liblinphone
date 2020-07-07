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

package org.linphone.core.tools.receiver;

import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.PowerManager;

import org.linphone.core.tools.Log;

/*
 * Purpose of this receiver is to disable keep alives when device is on idle
 */
public class DozeReceiver extends android.content.BroadcastReceiver {
    public DozeReceiver() { }

    @Override
    public void onReceive(Context context, Intent intent) {
        PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            boolean dozeM = pm.isDeviceIdleMode();
            Log.i("[Platform Helper] [Doze Receiver] Doze mode enabled: " + dozeM);
        }
    }
}
