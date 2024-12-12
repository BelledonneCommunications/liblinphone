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
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;
import java.util.Enumeration;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.regex.Pattern;

/**
 * This class is instantiated directly by the linphone library in order to
 * access specific features only accessible in java.
 **/
public class JavaPlatformHelper {
    private long mNativePtr;
    private File mTmpDir;

    private static AtomicBoolean copyAlreadyDone = new AtomicBoolean(false);
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
            Log.e("[Platform Helper] failed to install some resources.\n" + e);
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

        File systemTmpDir = new File(System.getProperty("java.io.tmpdir"));
        mTmpDir = new File(systemTmpDir, "linphone-sdk-java-linux");
        if (!mTmpDir.exists()) {
            mTmpDir.mkdir();
        }
        mTmpDir.deleteOnExit();
        Log.i("[Platform Helper] tmp dir: " + mTmpDir.getAbsolutePath());

        if (!copyAlreadyDone.getAndSet(true)) {
            try (JarFile sdkJarFile = findSdkJar()) {
                copyResourcesFromJarIfNeeded(sdkJarFile, assetsDirectoryName);
                copyResourcesFromJarIfNeeded(sdkJarFile, linphonePluginsDirectoryName);
                copyResourcesFromJarIfNeeded(sdkJarFile, ms2PluginsDirectoryName);

                Log.i("[Platform Helper] Copy from resources done");
            }
        } else {
            Log.i("[Platform Helper] Copy from resources already done");
        }
    }

    private JarFile findSdkJar() throws IOException {
        String jarPath = getClass().getProtectionDomain().getCodeSource().getLocation().getPath();
        Log.i("[Platform Helper] SDK jar path: " + jarPath);
        File jarFile = new File(jarPath);
        if (jarFile.isFile()) {
            return new JarFile(jarFile);
        }

        String nestedJarPathPattern = "^nested:.*\\.jar/!.*\\.jar!/";
        if (Pattern.matches(nestedJarPathPattern, jarPath)) {
            Log.i("[Platform Helper] SDK jar is nested in another jar (might be a a Spring Boot app), "
            + "extracting sdk to tmp dir");
            // We need to extract sdk jar first
            // Extract outer and nested jar path from jarPath
            String outerJarPath = extractOuterJarPath(jarPath);
            String nestedEntryPath = extractNestedEntryPath(jarPath);

            // Find and extract the nested entry
            try (JarFile outerJarFile = new JarFile(outerJarPath)) {
                JarEntry nestedEntry = outerJarFile.getJarEntry(nestedEntryPath);
                if (nestedEntry == null)
                    throw new IOException(
                            "Nested entry[" + nestedEntryPath + "] not found in outer jar[" + outerJarPath + "]");

                Path outputPath = mTmpDir.toPath().resolve("linphone-sdk.jar");
                Files.copy(outerJarFile.getInputStream(nestedEntry), outputPath, StandardCopyOption.REPLACE_EXISTING);
                return new JarFile(outputPath.toFile());
            }
        }

        throw new IOException("No way open SDK jar[" + jarPath + "]");
    }

    /**
     * @param jarPath jar path respecting the pattern "^nested:.*\\.jar/!.*\\.jar!/"
     * @return the String between "nested:" and "/!"
     */
    private String extractOuterJarPath(String jarPath) {
        return extractStringBetween(jarPath, "nested:", "/!");
    }

    /**
     * @param jarPath jar path respecting the pattern "^nested:.*\\.jar/!.*\\.jar!/"
     * @return the String between "/!" and "!/"
     */
    private String extractNestedEntryPath(String jarPath) {
        return extractStringBetween(jarPath, "/!", "!/");
    }

    private String extractStringBetween(String string, String startMarker, String endMarker) {
        int startIndex = string.indexOf(startMarker) + startMarker.length();
        int endIndex = string.indexOf(endMarker, startIndex);
        return string.substring(startIndex, endIndex);
    }

    private void copyResourcesFromJarIfNeeded(JarFile sdkJarFile, String prefix) throws IOException {
        Enumeration<JarEntry> entries = sdkJarFile.entries();
        String destPrefix = "";
        while (entries.hasMoreElements()) {
            JarEntry entry = entries.nextElement();
            String name = entry.getName();
            if (name.startsWith(prefix)) {
                String relativeName = name.substring(prefix.length());
                if (relativeName.isEmpty()) {
                    String[] parts = prefix.split("/");
                    destPrefix = parts[parts.length - 1] + "/";
                    if (!Files.exists(mTmpDir.toPath().resolve(destPrefix))) {
                        Files.createDirectory(mTmpDir.toPath().resolve(destPrefix));
                    }
                } else {
                    Path destPath = mTmpDir.toPath().resolve(destPrefix).resolve(relativeName);
                    if (entry.isDirectory()) {
                        if (!Files.exists(destPath)) {
                            Files.createDirectory(destPath);
                        }
                    } else {
                        Log.i("[Platform Helper] Installing resource " + destPrefix + relativeName);
                        File dest = new File(destPath.toString());
                        try (InputStream ist = sdkJarFile.getInputStream(entry)) {
                            Files.copy(ist, dest.getAbsoluteFile().toPath(), StandardCopyOption.REPLACE_EXISTING);
                        }
                    }
                }
            }
        }
    }
};
