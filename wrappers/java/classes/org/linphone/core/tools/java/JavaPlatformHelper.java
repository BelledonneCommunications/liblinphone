/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

package org.linphone.core.tools.java;

import org.linphone.core.tools.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URISyntaxException;
import java.net.URL;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Map;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.HashMap;

/**
 * This class is instanciated directly by the linphone library in order to
 * access specific features only accessible in java.
 **/
public class JavaPlatformHelper {
    private long mNativePtr;
    private File mTmpDir;

    private static String assetsDirectoryName = "assets/org.linphone.core/share/";
    private static String linphonePluginsDirectoryName = "liblinphone/";
    private static String ms2PluginsDirectoryName = "mediastreamer/";

    private native void setNativePreviewWindowId(long nativePtr, Object view);

    private native void setNativeVideoWindowId(long nativePtr, Object view);

    private native void setParticipantDeviceNativeVideoWindowId(long nativePtr, long participantDevicePtr, Object view);

    public JavaPlatformHelper(long nativePtr) {
        mNativePtr = nativePtr;

        Log.i("[Platform Helper] Created");

        try {
            copyResourcesFromJar();
        } catch (IOException e) {
            Log.e("[Platform Helper] failed to install some resources.");
        }
    }

    public String getResourcesDir() {
        return mTmpDir.toPath().resolve("share").toString();
    }

    public String getMsPluginsDir() {
        return mTmpDir.toPath().resolve("mediastreamer").resolve("plugins").toString();
    }

    private void copyResourcesFromJar() throws IOException {
        Log.i("[Platform Helper] Starting copy from resources to application files directory");

        mTmpDir = new File(Files.createTempDirectory("linphone-sdk").toString());
        mTmpDir.deleteOnExit();
        Log.i("[Platform Helper] " + mTmpDir.getAbsolutePath());

        copyResourcesFromJar(assetsDirectoryName);
        copyResourcesFromJar(linphonePluginsDirectoryName);
        copyResourcesFromJar(ms2PluginsDirectoryName);

        Log.i("[Platform Helper] Copy from resources done");
    }

    private void copyResourcesFromJar(String prefix) throws IOException {
        String jarPath = getClass().getProtectionDomain().getCodeSource().getLocation().getPath();
        File jarFile = new File(jarPath);
        if (jarFile.isFile()) {
            JarFile jar = new JarFile(jarFile);
            Enumeration<JarEntry> entries = jar.entries();
            String destPrefix = "";
            while (entries.hasMoreElements()) {
                JarEntry entry = entries.nextElement();
                String name = entry.getName();
                if (name.startsWith(prefix)) {
                    String relativeName = name.substring(prefix.length());
                    if (relativeName.isEmpty()) {
                        String[] parts = prefix.split("/");
                        destPrefix = parts[parts.length - 1] + "/";
                        Files.createDirectory(mTmpDir.toPath().resolve(destPrefix));
                    } else {
                        Path destPath = mTmpDir.toPath().resolve(destPrefix).resolve(relativeName);
                        if (entry.isDirectory()) {
                            Files.createDirectory(destPath);
                        } else {
                            Log.i("[Platform Helper] Installing resource " + destPrefix + relativeName);
                            File dest = new File(destPath.toString());
                            InputStream ist = jar.getInputStream(entry);
                            Files.copy(ist, dest.getAbsoluteFile().toPath());
                        }
                    }
                }
            }
        }
    }
};
