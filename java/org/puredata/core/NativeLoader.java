/**
 * 
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL WARRANTIES, see the
 * file, "LICENSE.txt," in this distribution.
 * 
 */

package org.puredata.core;

import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.FileOutputStream;
import java.io.IOException;

/**
 * 
 * Utilities for loading platform-specific libraries using System.loadLibrary, falling back to
 * trying to load from the jar file.
 * 
 * @author Bill Robinson (airbaggins@gmail.com)
 */
public class NativeLoader {

  private static String osName = null;
  private static String osArch = null;


  public static class NativeLibraryLoadError extends UnsatisfiedLinkError {

    private static final long serialVersionUID = 1L;

    public NativeLibraryLoadError(String message) {
      super(message);
    }

    public NativeLibraryLoadError(String message, Throwable cause) {
      // The convenience super-constructor, with cause as second argument, is only available from
      // Java 1.7
      super(message);
      initCause(cause);
    }

  }


  static {
    detectSystem();
  }

  private static void detectSystem() {
    osArch = System.getProperty("os.arch").toLowerCase();
    if (osArch.indexOf("64") != -1) {
      osArch = "x86_64";
    } else if (osArch.indexOf("86") != -1) {
      osArch = "x86";
    }

    osName = System.getProperty("os.name").toLowerCase();

    // Ordered by likeliness to appear in each others' names
    if (osName.indexOf("linux") != -1) {
      osName = "linux";
    } else if (osName.indexOf("windows") != -1) {
      osName = "windows";
    } else if (osName.indexOf("mac") != -1) {
      osName = "mac";
    }
  }

  /**
   * Load the library named, if osNameCheck is the current operating system and osArchCheck is the
   * current architecture.
   * 
   * @param library Name of library to load
   * @param osNameCheck Name of detected operating system (linux/mac/windows)
   * @param osArchCheck Architecture name (x86/x86_64)
   */
  public static void loadLibrary(String library, String osNameCheck, String osArchCheck) {
    if (osArch.equals(osArchCheck)) {
      loadLibrary(library, osNameCheck);
    }
  }

  /**
   * Load the library named, if osNameCheck is the current operating system.
   * 
   * @param library Name of library to load
   * @param osNameCheck Name of detected operating system (linux/mac/windows)
   */
  public static void loadLibrary(String library, String osNameCheck) {
    if (osName.equals(osNameCheck)) {
      loadLibrary(library);
    }
  }

  /** Load the library named.
   *
   * @param library Name of library to load
   */
  public static void loadLibrary(String library) {
    try {
      System.loadLibrary(library);
    } catch (UnsatisfiedLinkError error) {
      loadLibraryFromJar(library);
    }
  }

  /** Try to extract the native library from this Jar file. */
  private static void loadLibraryFromJar(String library) {
    library = System.mapLibraryName(library);
    InputStream in =
        PdBase.class.getResourceAsStream("natives/" + osName + "/" + osArch + "/" + library);
    if (in == null) {
      in = PdBase.class.getResourceAsStream("natives/" + osName + "/" + library);
    }
    if (in == null) {
      throw new NativeLibraryLoadError("Couldn't find " + library + " for this platform " + osName
          + "/" + osArch);
    }
    try {
      File fileOut =
          File.createTempFile(library.replaceFirst("\\.[^.]*$", ""),
              library.replaceFirst("^.*\\.", "."));
      OutputStream out = new FileOutputStream(fileOut);
      byte[] copyBuffer = new byte[1024];
      int amountRead;
      while ((amountRead = in.read(copyBuffer)) != -1) {
        out.write(copyBuffer, 0, amountRead);
      }
      in.close();
      out.close();
      System.load(fileOut.toString());
      fileOut.deleteOnExit();
    } catch (IOException error) {
      throw new NativeLibraryLoadError("Failed to save native library " + library
          + " to temporary file", error);
    }
  }
}
