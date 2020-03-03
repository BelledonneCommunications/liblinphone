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

package org.linphone.core.tools.service;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

import org.linphone.core.tools.Log;

public class CoreService extends Service {
    private static CoreService sInstance;

    public static boolean isReady() {
        return sInstance != null;
    }

    public static CoreService instance() {
        if (isReady()) return sInstance;

        throw new RuntimeException("CoreService not instantiated yet");
    }

    @SuppressWarnings("unchecked")
    @Override
    public void onCreate() {
        super.onCreate();

        if (!CoreManager.isReady()) {
            new CoreManager(getApplicationContext()).startFromService();
        }

        Log.i("[Core Service] Created");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        sInstance = this;

        Log.i("[Core Service] Started");
        return START_STICKY;
    }

    @Override
    public void onTaskRemoved(Intent rootIntent) {
        Log.i("[Core Service] Task removed");
        stopSelf();

        super.onTaskRemoved(rootIntent);
    }

    @Override
    public synchronized void onDestroy() {
        Log.i("[Core Service] Stopping");

        CoreManager.instance().destroy();
        sInstance = null;

        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}
