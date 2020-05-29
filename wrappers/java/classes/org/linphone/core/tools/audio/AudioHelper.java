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
import android.media.AudioAttributes;
import android.media.AudioManager;
import android.media.AudioManager.OnAudioFocusChangeListener;
import android.media.MediaPlayer;
import android.net.Uri;
import android.provider.Settings;

import androidx.media.AudioAttributesCompat;
import androidx.media.AudioAttributesCompat.Builder;
import androidx.media.AudioManagerCompat;
import androidx.media.AudioFocusRequestCompat;

import java.io.IOException;
import java.io.FileInputStream;
import java.lang.SecurityException;

import org.linphone.core.Core;
import org.linphone.core.AudioDevice;
import org.linphone.core.tools.Log;
import org.linphone.core.tools.service.CoreManager;

public class AudioHelper implements OnAudioFocusChangeListener {
    private AudioManager mAudioManager;
    private AudioFocusRequestCompat mRingingRequest;
    private AudioFocusRequestCompat mCallRequest;
    private MediaPlayer mPlayer;
    private int mVolumeBeforeEchoTest;
    private AudioDevice mPreviousDefaultOutputAudioDevice;

    public AudioHelper(Context context) {
        mAudioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        mRingingRequest = null;
        mCallRequest = null;
        Log.i("[Audio Helper] Helper created");
    }

    public void startAudioForEchoTestOrCalibration() {
        requestCallAudioFocus();
        routeAudioToSpeaker();

        mVolumeBeforeEchoTest = mAudioManager.getStreamVolume(AudioManager.STREAM_VOICE_CALL);
        int maxVolume = mAudioManager.getStreamMaxVolume(AudioManager.STREAM_VOICE_CALL);
        try {
            mAudioManager.setStreamVolume(AudioManager.STREAM_VOICE_CALL, maxVolume, 0);
        } catch (SecurityException se) {
            Log.e("[Audio Helper] Couldn't increase volume: ", se);
        }
    }

    public void stopAudioForEchoTestOrCalibration() {
        try {
            mAudioManager.setStreamVolume(AudioManager.STREAM_VOICE_CALL, mVolumeBeforeEchoTest, 0);
        } catch (SecurityException se) {
            Log.e("[Audio Helper] Couldn't restore volume: ", se);
        }

        routeAudioToEarpiece();
        releaseCallAudioFocus();
    }

    public void startRinging(Context context, String ringtone) {
        if (mPlayer != null) {
            Log.w("[Audio Helper] Already ringing, skipping...");
            return;
        }

        requestRingingAudioFocus();

        AudioAttributes audioAttrs = new AudioAttributes.Builder()
            .setUsage(AudioAttributes.USAGE_NOTIFICATION_RINGTONE)
            .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
            .build();

        mPlayer = new MediaPlayer();
        mPlayer.setAudioAttributes(audioAttrs);

        if (ringtone == null || ringtone.isEmpty()) {
            Log.i("[Audio Helper] Core ringtone path is null, using device ringtone if possible");
            ringtone = Settings.System.DEFAULT_RINGTONE_URI.toString();
        }

        try {
            if (ringtone.startsWith("content://")) {
                mPlayer.setDataSource(context, Uri.parse(ringtone));
            } else {
                FileInputStream fis = new FileInputStream(ringtone);
                mPlayer.setDataSource(fis.getFD());
                fis.close();
            }

            mPlayer.prepare();
            mPlayer.setLooping(true);
            mPlayer.start();
            Log.i("[Audio Helper] Ringing started");
        } catch (IOException e) {
            Log.e(e, "[Audio Helper] Cannot set ringtone ", ringtone);
        }
    }

    public void stopRinging() {
        if (mPlayer != null) {
            releaseRingingAudioFocus();
            
            mPlayer.stop();
            mPlayer.release();
            mPlayer = null;
            Log.i("[Audio Helper] Ringing stopped");
        }
    }

    public void requestRingingAudioFocus() {
        if (mRingingRequest != null) {
            Log.w("[Audio Helper] Ringing audio focus request still active, skipping");
            return;
        }

        AudioAttributesCompat audioAttrs = new AudioAttributesCompat.Builder()
            .setUsage(AudioAttributesCompat.USAGE_NOTIFICATION_RINGTONE)
            .setContentType(AudioAttributesCompat.CONTENT_TYPE_MUSIC)
            .build();

        // Request AUDIOFOCUS_GAIN_TRANSIENT_EXCLUSIVE to prevent incoming call notification to "beep" at the same time we play the ringtone
        mRingingRequest = new AudioFocusRequestCompat.Builder(AudioManagerCompat.AUDIOFOCUS_GAIN_TRANSIENT_EXCLUSIVE)
            .setAudioAttributes(audioAttrs)
            .setOnAudioFocusChangeListener(this)
            .build();

        int result = AudioManagerCompat.requestAudioFocus(mAudioManager, mRingingRequest);
        if (result == AudioManager.AUDIOFOCUS_REQUEST_GRANTED) {
            Log.i("[Audio Helper] Ringing audio focus request granted");
        } else if (result == AudioManager.AUDIOFOCUS_REQUEST_FAILED) {
            Log.w("[Audio Helper] Ringing audio focus request failed");
        } else if (result == AudioManager.AUDIOFOCUS_REQUEST_DELAYED) {
            Log.w("[Audio Helper] Ringing audio focus request delayed");
        }
    }

    public void releaseRingingAudioFocus() {
        if (mRingingRequest != null) {
            AudioManagerCompat.abandonAudioFocusRequest(mAudioManager, mRingingRequest);
            Log.i("[Audio Helper] Ringing audio focus request abandonned");
            mRingingRequest = null;
        }
    }

    public void requestCallAudioFocus() {
        if (mRingingRequest != null) {
            Log.w("[Audio Helper] Ringing audio focus request not abandonned, let's do it");
            releaseRingingAudioFocus();
        }

        if (mCallRequest != null) {
            Log.w("[Audio Helper] Call audio focus request still active, skipping");
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
            Log.i("[Audio Helper] Call audio focus request granted, setting AudioManager in MODE_IN_COMMUNICATION");
            mAudioManager.setMode(AudioManager.MODE_IN_COMMUNICATION);
        } else if (result == AudioManager.AUDIOFOCUS_REQUEST_FAILED) {
            Log.w("[Audio Helper] Call audio focus request failed");
        } else if (result == AudioManager.AUDIOFOCUS_REQUEST_DELAYED) {
            Log.w("[Audio Helper] Call audio focus request delayed");
        }
    }

    public void releaseCallAudioFocus() {
        if (mCallRequest != null) {
            AudioManagerCompat.abandonAudioFocusRequest(mAudioManager, mCallRequest);
            Log.i("[Audio Helper] Call audio focus request abandonned, restoring AudioManager mode to MODE_NORMAL");
            mCallRequest = null;
            mAudioManager.setMode(AudioManager.MODE_NORMAL);
        }
    }

    @Override
    public void onAudioFocusChange(int focusChange) {
        switch (focusChange) {
            case AudioManager.AUDIOFOCUS_GAIN:
                Log.i("[Audio Helper] Focus gained");
                break;
            case AudioManager.AUDIOFOCUS_LOSS:
                Log.w("[Audio Helper] Focus lost");
                if (CoreManager.isReady()) CoreManager.instance().onAudioFocusLost();
                break;
            case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT:
                Log.w("[Audio Helper] Focus lost (transient)");
                if (CoreManager.isReady()) CoreManager.instance().onAudioFocusLost();
                break;
            case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
                Log.w("[Audio Helper] Focus lost (transient, can duck)");
                if (CoreManager.isReady()) CoreManager.instance().onAudioFocusLost();
                break;
        }
    }

    public void hackVolume() {
        mAudioManager.adjustSuggestedStreamVolume(AudioManager.ADJUST_MUTE, AudioManager.STREAM_VOICE_CALL, 0);
        mAudioManager.adjustSuggestedStreamVolume(AudioManager.ADJUST_UNMUTE, AudioManager.STREAM_VOICE_CALL, 0);

        mAudioManager.adjustSuggestedStreamVolume(AudioManager.ADJUST_LOWER, AudioManager.STREAM_VOICE_CALL, 0);
        mAudioManager.adjustSuggestedStreamVolume(AudioManager.ADJUST_RAISE, AudioManager.STREAM_VOICE_CALL, 0);
    }

    private void routeAudioToEarpiece() {
        // Let's restore the default output device before the echo calibration or test
        Core core = CoreManager.instance().getCore();
        core.setDefaultOutputAudioDevice(mPreviousDefaultOutputAudioDevice);
        Log.i("[Audio Helper] Restored previous default output audio device: " + mPreviousDefaultOutputAudioDevice);
        mPreviousDefaultOutputAudioDevice = null;
    }

    private void routeAudioToSpeaker() {
        // For echo canceller calibration & echo tester, we can't change the soundcard dynamically as the stream isn't created yet...
        Core core = CoreManager.instance().getCore();
        mPreviousDefaultOutputAudioDevice = core.getDefaultOutputAudioDevice();
        if (mPreviousDefaultOutputAudioDevice.getType() == AudioDevice.Type.Speaker) {
            Log.i("[Audio Helper] Current default output audio device is already the speaker: " + mPreviousDefaultOutputAudioDevice);
            return;
        }
        Log.i("[Audio Helper] Saved current default output audio device: " + mPreviousDefaultOutputAudioDevice);
        
        for (AudioDevice audioDevice : core.getAudioDevices()) {
            if (audioDevice.getType() == AudioDevice.Type.Speaker) {
                Log.i("[Audio Helper] Found speaker device, replacing default output audio device with: " + audioDevice);
                core.setDefaultOutputAudioDevice(audioDevice);
                return;
            }
        }
        Log.e("[Audio Helper] Couldn't find speaker audio device");
    }
}