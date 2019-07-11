package org.linphone.core.tools.firebase;

/*
FirebasePushHelper.java
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

import android.content.Context;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.GoogleApiAvailability;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;

import com.google.firebase.iid.FirebaseInstanceId;
import com.google.firebase.iid.InstanceIdResult;

import org.linphone.core.tools.Log;
import org.linphone.core.tools.service.LinphoneService;
import org.linphone.core.tools.service.PushNotificationUtils;

public class FirebasePushHelper implements PushNotificationUtils.PushHelperInterface {
    public FirebasePushHelper() {}

    @Override
    public void init(Context context) {
        try {
            FirebaseInstanceId.getInstance()
                    .getInstanceId()
                    .addOnCompleteListener(
                            new OnCompleteListener<InstanceIdResult>() {
                                @Override
                                public void onComplete(Task<InstanceIdResult> task) {
                                    if (!task.isSuccessful()) {
                                        Log.e(
                                                "[Push Notification] firebase getInstanceId failed: "
                                                        + task.getException());
                                        return;
                                    }
                                    String token = task.getResult().getToken();
                                    LinphoneService.getCore().setPushNotificationToken(token);
                                }
                            });
        } catch (Exception e) {
            Log.e("[Push Notification] firebase not available: " + e);
        }
    }

    @Override
    public boolean isAvailable(Context context) {
        GoogleApiAvailability googleApiAvailability = GoogleApiAvailability.getInstance();
        int resultCode = googleApiAvailability.isGooglePlayServicesAvailable(context);
        return resultCode == ConnectionResult.SUCCESS;
    }
}
