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

package org.linphone.core.tools.audio;

import android.content.Context;
import android.media.AudioManager;
import android.media.AudioManager.OnAudioFocusChangeListener;

import androidx.media.AudioAttributesCompat;
import androidx.media.AudioAttributesCompat.Builder;
import androidx.media.AudioManagerCompat;
import androidx.media.AudioFocusRequestCompat;

import org.linphone.core.tools.Log;
import org.linphone.core.tools.service.CoreManager;

public class AudioFocusHelper implements OnAudioFocusChangeListener {
    private AudioManager mAudioManager;
    private AudioFocusRequestCompat mRingingRequest;
    private AudioFocusRequestCompat mCallRequest;

    public AudioFocusHelper(Context context) {
        mAudioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        mRingingRequest = null;
        mCallRequest = null;
        Log.i("[Audio Focus Helper] Focus helper created");
    }

    public void requestRingingAudioFocus() {
        if (mRingingRequest != null) {
            Log.w("[Audio Focus Helper] Ringing audio focus request still active, skipping");
            return;
        }

        AudioAttributesCompat audioAttrs = new AudioAttributesCompat.Builder()
            .setUsage(AudioAttributesCompat.USAGE_NOTIFICATION_RINGTONE)
            .setContentType(AudioAttributesCompat.CONTENT_TYPE_MUSIC)
            .build();

         mRingingRequest = new AudioFocusRequestCompat.Builder(AudioManagerCompat.AUDIOFOCUS_GAIN_TRANSIENT)
            .setAudioAttributes(audioAttrs)
            .setOnAudioFocusChangeListener(this)
            .build();

        int result = AudioManagerCompat.requestAudioFocus(mAudioManager, mRingingRequest);
        if (result == AudioManager.AUDIOFOCUS_REQUEST_GRANTED) {
            Log.i("[Audio Focus Helper] Ringing audio focus request granted");
        } else if (result == AudioManager.AUDIOFOCUS_REQUEST_FAILED) {
            Log.w("[Audio Focus Helper] Ringing audio focus request failed");
        } else if (result == AudioManager.AUDIOFOCUS_REQUEST_DELAYED) {
            Log.w("[Audio Focus Helper] Ringing audio focus request delayed");
        }
    }

    public void releaseRingingAudioFocus() {
        if (mRingingRequest != null) {
            AudioManagerCompat.abandonAudioFocusRequest(mAudioManager, mRingingRequest);
            Log.i("[Audio Focus Helper] Ringing audio focus request abandonned");
            mRingingRequest = null;
        }
    }

    public void requestCallAudioFocus() {
        if (mRingingRequest != null) {
            Log.w("[Audio Focus Helper] Ringing audio focus request not abandonned, let's do it");
            releaseRingingAudioFocus();
        }

        if (mCallRequest != null) {
            Log.w("[Audio Focus Helper] Call audio focus request still active, skipping");
            return;
        }

        AudioAttributesCompat audioAttrs = new AudioAttributesCompat.Builder()
            .setUsage(AudioAttributesCompat.USAGE_VOICE_COMMUNICATION)
            .setContentType(AudioAttributesCompat.CONTENT_TYPE_SPEECH)
            .build();

         mCallRequest = new AudioFocusRequestCompat.Builder(AudioManagerCompat.AUDIOFOCUS_GAIN)
            .setAudioAttributes(audioAttrs)
            .setOnAudioFocusChangeListener(this)
            .build();

        int result = AudioManagerCompat.requestAudioFocus(mAudioManager, mCallRequest);
        if (result == AudioManager.AUDIOFOCUS_REQUEST_GRANTED) {
            Log.i("[Audio Focus Helper] Call audio focus request granted, setting AudioManager in MODE_IN_COMMUNICATION");
            mAudioManager.setMode(AudioManager.MODE_IN_COMMUNICATION);
        } else if (result == AudioManager.AUDIOFOCUS_REQUEST_FAILED) {
            Log.w("[Audio Focus Helper] Call audio focus request failed");
        } else if (result == AudioManager.AUDIOFOCUS_REQUEST_DELAYED) {
            Log.w("[Audio Focus Helper] Call audio focus request delayed");
        }
    }

    public void releaseCallAudioFocus() {
        if (mCallRequest != null) {
            AudioManagerCompat.abandonAudioFocusRequest(mAudioManager, mCallRequest);
            Log.i("[Audio Focus Helper] Call audio focus request abandonned, restoring AudioManager mode to MODE_NORMAL");
            mCallRequest = null;
            mAudioManager.setMode(AudioManager.MODE_NORMAL);
        }
    }

    @Override
    public void onAudioFocusChange(int focusChange) {
        switch (focusChange) {
            case AudioManager.AUDIOFOCUS_GAIN:
                Log.i("[Audio Focus Helper] Focus gained");
                break;
            case AudioManager.AUDIOFOCUS_LOSS:
                Log.w("[Audio Focus Helper] Focus lost");
                if (CoreManager.isReady()) CoreManager.instance().onAudioFocusLost();
                break;
            case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT:
                Log.w("[Audio Focus Helper] Focus lost (transient)");
                if (CoreManager.isReady()) CoreManager.instance().onAudioFocusLost();
                break;
            case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
                Log.w("[Audio Focus Helper] Focus lost (transient, can duck)");
                if (CoreManager.isReady()) CoreManager.instance().onAudioFocusLost();
                break;
        }
    }
}