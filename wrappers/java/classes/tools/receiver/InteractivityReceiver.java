package org.linphone.core.tools;

/*
InteractivityReceiver.java
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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import org.linphone.core.tools.Log;
import org.linphone.core.tools.Log;

/*
 * Purpose of this receiver is to disable keep alives when screen is off
 * */
public class InteractivityReceiver extends BroadcastReceiver {
	private AndroidPlatformHelper mHelper;

    public InteractivityReceiver(final AndroidPlatformHelper helper) {
        mHelper = helper;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if (action.equalsIgnoreCase(Intent.ACTION_SCREEN_ON)) {
            Log.i("[Platform Helper] Device is in interactive mode");
            mHelper.setInteractiveMode(true);
        } else if (action.equalsIgnoreCase(Intent.ACTION_SCREEN_OFF)) {
            Log.i("[Platform Helper] Device is not in interactive mode");
            mHelper.setInteractiveMode(false);
        }
    }
}
