package org.linphone.core.tools.java;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;

public class LibraryLoader {
    private static File tempDir;

    private LibraryLoader() {
    }

    public static void loadLibrary(String name) throws IOException {
        String osName = getOsName(name);

        // Create a temporary directory where to extract the libraries, if not already
        // done.
        if (tempDir == null) {
            tempDir = new File(Files.createTempDirectory("linphone-sdk-library-loader").toString());
            tempDir.deleteOnExit();
        }

        // Extract the library from the jar file.
        File tempFile = new File(tempDir, osName);
        try(InputStream ist = LibraryLoader.class.getResourceAsStream("/" + osName);) {
            Files.copy(ist, tempFile.toPath(), StandardCopyOption.REPLACE_EXISTING);
        } catch (IOException e) {
            tempFile.delete();
            throw e;
        } catch (NullPointerException e) {
            tempFile.delete();
            throw new FileNotFoundException("Library " + osName + " was not found inside JAR.");
        }

        // Load the extracted library.
        try {
            System.load(tempFile.getAbsolutePath());
        } finally {
            if (isWindows()) {
                tempFile.deleteOnExit();
            } else {
                tempFile.delete();
            }
        }
    }

    public static void loadOptionalLibrary(String name) throws IOException {
        try {
            loadLibrary(name);
        } catch (FileNotFoundException e) {
            System.err.println(e.getMessage());
        }
    }

    private static String getOsName(String name) {
        if (isWindows()) {
            return name + ".dll";
        } else {
            if (name.startsWith("lib")) {
                return name + ".so";
            } else {
                return "lib" + name + ".so";
            }
        }
    }

    private static Boolean isWindows() {
        String os = System.getProperty("os.name").toLowerCase();
        return os.contains("win");
    }
}
