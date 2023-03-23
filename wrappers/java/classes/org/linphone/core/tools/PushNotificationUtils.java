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

        if (!isFirebaseCloudMessaging23Available()) {
            Log.w("[Push Utils] Firebase Cloud Messaging 23+ isn't available. Ensure you have a dependency on com.google.firebase:firebase-messaging (23.0.6 or newer) in your app's build.gradle file or that you use a BoM at least 'com.google.firebase:firebase-bom:30.2.0'.");
            return;
        }

        if (!isGoogleApiAvailable()) {
            Log.w("[Push Utils] Google services aren't available. Ensure you have correctly applied the com.google.gms.google-services plugin in your app's build.gradle file (cf https://firebase.google.com/docs/android/setup#add-config-file).");
            return;
        }

        int resId = context.getResources().getIdentifier("gcm_defaultSenderId", "string", context.getPackageName());
        if (resId == 0) {
            Log.e("[Push Utils] Couldn't find gcm_defaultSenderId string resource.");
            return;
        }
        
        FirebaseApp.initializeApp(context);

        try {
            String className = "org.linphone.core.tools.firebase.FirebasePushHelper";
            Class pushHelper = Class.forName(className);
            Class[] types = {};
            Constructor constructor = pushHelper.getConstructor(types);
            Object[] parameters = {};
            mHelper = (PushHelperInterface) constructor.newInstance(parameters);
            mHelper.init(context);
        } catch (NoSuchMethodException e) {
            Log.w("[Push Utils] Couldn't get push helper constructor");
        } catch (ClassNotFoundException e) {
            Log.w("[Push Utils] Couldn't find class: " + e);
        } catch (Exception e) {
            Log.w("[Push Utils] Couldn't get push helper instance: " + e);
        }
    }

    public static boolean isAvailable(Context context) {
        if (mHelper == null) return false;
        return mHelper.isAvailable(context);
    }

    private static boolean isFirebaseCloudMessaging23Available() {
        boolean available = false;
        try {
            Class firebaseApp = Class.forName("com.google.firebase.FirebaseApp");
            Class firebaseInstallationsId = Class.forName("com.google.firebase.messaging.FirebaseMessaging");
            available = true;
        } catch (ClassNotFoundException e) {
            Log.w("[Push Utils] Couldn't find class: ", e);
        } catch (Exception e) {
            Log.w("[Push Utils] Exception: " + e);
        }
        return available;
    }

    private static boolean isGoogleApiAvailable() {
        boolean available = false;
        try {
            Class googleAPiAvailability = Class.forName("com.google.android.gms.common.GoogleApiAvailability");
            Class googleTask = Class.forName("com.google.android.gms.tasks.Task");
            Class googleConnectionResult = Class.forName("com.google.android.gms.common.ConnectionResult");
            Class googleCompleteListener = Class.forName("com.google.android.gms.tasks.OnCompleteListener");
            available = true;
        } catch (ClassNotFoundException e) {
            Log.w("[Push Utils] Couldn't find class: ", e);
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
