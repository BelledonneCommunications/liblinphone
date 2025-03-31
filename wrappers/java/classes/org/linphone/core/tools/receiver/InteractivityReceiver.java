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
package org.linphone.core.tools.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import org.linphone.core.tools.Log;
import org.linphone.core.tools.AndroidPlatformHelper;

/*
 * Purpose of this receiver is to disable keep alives when screen is off
 */
public class InteractivityReceiver extends BroadcastReceiver {
    public InteractivityReceiver() { }

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if (action.equalsIgnoreCase(Intent.ACTION_SCREEN_ON)) {
            Log.i("[Platform Helper] [Interactivity Receiver] Device screen is ON");
        } else if (action.equalsIgnoreCase(Intent.ACTION_SCREEN_OFF)) {
            Log.i("[Platform Helper] [Interactivity Receiver] Device screen is OFF");
            if (AndroidPlatformHelper.isReady()) {
                AndroidPlatformHelper.instance().stopRinging();
                AndroidPlatformHelper.instance().stopVibrating();
            }
        }
    }
}
