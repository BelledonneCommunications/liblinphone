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
package org.linphone.core.tools.firebase;

import android.content.Context;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.GoogleApiAvailability;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.messaging.FirebaseMessaging;

import org.linphone.core.tools.Log;
import org.linphone.core.tools.PushNotificationUtils;
import org.linphone.core.tools.service.CoreManager;

@Keep
/**
 * This class attempts to initialize firebase if it is available.
 */
public class FirebasePushHelper implements PushNotificationUtils.PushHelperInterface {
    public FirebasePushHelper() {
    }

    @Override
    public void init(Context context) {
        try {
            FirebaseMessaging.getInstance()
                    .getToken()
                    .addOnCompleteListener(
                            new OnCompleteListener<String>() {
                                @Override
                                public void onComplete(@NonNull Task<String> task) {
                                    if (!task.isSuccessful()) {
                                        Log.e(
                                                "[Push Notification] Firebase getToken failed: "
                                                        + task.getException());
                                        return;
                                    }
                                    String token = task.getResult();
                                    Log.i("[Push Notification] Token fetched from Firebase: " + token);
                                    if (CoreManager.isReady()) {
                                        Runnable runnable = new Runnable() {
                                            @Override
                                            public void run() {
                                                CoreManager.instance().setPushToken(token);
                                            }
                                        };
                                        CoreManager.instance().dispatchOnCoreThread(runnable);
                                    }
                                }
                            });
        } catch (Exception e) {
            Log.e("[Push Notification] Firebase not available.");
        }
    }

    @Override
    public boolean isAvailable(Context context) {
        GoogleApiAvailability googleApiAvailability = GoogleApiAvailability.getInstance();
        int resultCode = googleApiAvailability.isGooglePlayServicesAvailable(context);
        return resultCode == ConnectionResult.SUCCESS;
    }
}
