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
package org.linphone.core.tools.h264;


import android.os.Build;

import org.linphone.core.Core;
import org.linphone.core.tools.Log;

/**
 * Created by brieucviel on 09/12/2016.
 */

public class H264Helper {
    private static String FILTER_NAME_OPENH264_ENC = "MSOpenH264Enc";
    private static String FILTER_NAME_OPENH264_DEC = "MSOpenH264Dec";
    private static String FILTER_NAME_MEDIA_CODEC_ENC = "MediaCodecH264Encoder";
    private static String FILTER_NAME_MEDIA_CODEC_DEC = "MediaCodecH264Decoder";

    public static String MODE_AUTO = "Auto";
    public static String MODE_OPENH264 = "OpenH264";
    public static String MODE_MEDIA_CODEC = "MediaCodec";


    /**
     * H264Helper
     */
    public H264Helper() {
    }


    /**
     * Define the Codec to use between MediaCodec and OpenH264
     * Possible mode are:
     * - Auto to let the system choose in function of you OS version,
     * - OpenH264 to enable OpenH264 Encoder and Decoder,
     * - Mediacodec to enable Mediacodec only.
     *
     * @param mode String value between Auto, OpenH264 and MediaCodec
     */
    public static void setH264Mode(String mode, Core linphoneCore) {
        if (mode.equals(MODE_OPENH264)) {
            Log.i("H264Helper", " setH264Mode  MODE_OPENH264 - Mode = " + mode);
            linphoneCore.getMediastreamerFactory().enableFilterFromName(FILTER_NAME_MEDIA_CODEC_DEC, false);
            linphoneCore.getMediastreamerFactory().enableFilterFromName(FILTER_NAME_MEDIA_CODEC_ENC, false);
            linphoneCore.getMediastreamerFactory().enableFilterFromName(FILTER_NAME_OPENH264_DEC, true);
            linphoneCore.getMediastreamerFactory().enableFilterFromName(FILTER_NAME_OPENH264_ENC, true);
        } else if (mode.equals(MODE_MEDIA_CODEC)) {
            Log.i("H264Helper", " setH264Mode  MODE_MEDIA_CODEC - Mode = " + mode);
            linphoneCore.getMediastreamerFactory().enableFilterFromName(FILTER_NAME_OPENH264_DEC, false);
            linphoneCore.getMediastreamerFactory().enableFilterFromName(FILTER_NAME_OPENH264_ENC, false);
            linphoneCore.getMediastreamerFactory().enableFilterFromName(FILTER_NAME_MEDIA_CODEC_DEC, true);
            linphoneCore.getMediastreamerFactory().enableFilterFromName(FILTER_NAME_MEDIA_CODEC_ENC, true);
        } else if (mode.equals(MODE_AUTO)) {
            Log.i("H264Helper", " setH264Mode  MODE_AUTO - Mode = " + mode);
            // if  android >= 5.0 use MediaCodec
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP_MR1) {
                Log.i("H264Helper", " setH264Mode  MODE_AUTO 1 - Mode = " + mode);
                Log.i("H264Helper", " Openh264 disabled on the project, now using MediaCodec");
                linphoneCore.getMediastreamerFactory().enableFilterFromName(FILTER_NAME_OPENH264_DEC, false);
                linphoneCore.getMediastreamerFactory().enableFilterFromName(FILTER_NAME_OPENH264_ENC, false);
                linphoneCore.getMediastreamerFactory().enableFilterFromName(FILTER_NAME_MEDIA_CODEC_DEC, true);
                linphoneCore.getMediastreamerFactory().enableFilterFromName(FILTER_NAME_MEDIA_CODEC_ENC, true);
            }
            //otherwise use OpenH264
            else {
                Log.i("H264Helper", " setH264Mode  MODE_AUTO 2 - Mode = " + mode);
                Log.i("H264Helper", " Openh264 enabled on the project");
                linphoneCore.getMediastreamerFactory().enableFilterFromName(FILTER_NAME_MEDIA_CODEC_DEC, false);
                linphoneCore.getMediastreamerFactory().enableFilterFromName(FILTER_NAME_MEDIA_CODEC_ENC, false);
                linphoneCore.getMediastreamerFactory().enableFilterFromName(FILTER_NAME_OPENH264_DEC, true);
                linphoneCore.getMediastreamerFactory().enableFilterFromName(FILTER_NAME_OPENH264_ENC, true);
            }
        } else {
            Log.i("H264Helper", " Error: Openh264 mode not reconized !");
        }
        Log.i("H264Helper", " setH264Mode - Mode = " + mode);
    }

}

