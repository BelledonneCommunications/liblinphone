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
package org.linphone.core.tools;

import android.content.Context;

import com.google.firebase.FirebaseApp;

import java.lang.reflect.Constructor;

import org.linphone.core.tools.Log;

/**
 * This class wraps the FirebasePushHelper class.
 */
public class PushNotificationUtils {
    private static PushHelperInterface mHelper;

    public static void init(Context context) {
        mHelper = null;

        if (!isFirebaseAvailable()) return;
        FirebaseApp.initializeApp(context);

        String className = "org.linphone.core.tools.firebase.FirebasePushHelper";
        try {
            Class pushHelper = Class.forName(className);
            Class[] types = {};
            Constructor constructor = pushHelper.getConstructor(types);
            Object[] parameters = {};
            mHelper = (PushHelperInterface) constructor.newInstance(parameters);
            mHelper.init(context);
        } catch (NoSuchMethodException e) {
            Log.w("[Push Utils] Couldn't get push helper constructor");
        } catch (ClassNotFoundException e) {
            Log.w("[Push Utils] Couldn't find class " + className);
        } catch (Exception e) {
            Log.w("[Push Utils] Couldn't get push helper instance: " + e);
        }
    }

    public static boolean isAvailable(Context context) {
        if (mHelper == null) return false;
        return mHelper.isAvailable(context);
    }

    private static boolean isFirebaseAvailable() {
        boolean available = false;
        try {
            Class firebaseApp = Class.forName("com.google.firebase.FirebaseApp");
            available = true;
        } catch (ClassNotFoundException e) {
            Log.w("[Push Utils] Couldn't find class com.google.firebase.FirebaseApp");
        } catch (Exception e) {
            Log.w("[Push Utils] Exception: " + e);
        }
        return available;
    }

    public interface PushHelperInterface {
        void init(Context context);

        boolean isAvailable(Context context);
    }
}
