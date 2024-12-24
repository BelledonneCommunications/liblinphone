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
import androidx.core.content.ContextCompat;

import org.linphone.core.tools.Log;
import org.linphone.core.tools.AndroidPlatformHelper;

public class ShutdownReceiver extends BroadcastReceiver {
    public ShutdownReceiver() { }

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.w("[Platform Helper] Shutdown intent received, stopping Core");
        if (AndroidPlatformHelper.isReady()) AndroidPlatformHelper.instance().stop();
    }
}
