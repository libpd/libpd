/** 
 * 
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
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
 * Utilities for loading platform-specific libraries using System.loadLibrary,
 * or falls back to trying to load an appropriate library from the jar file.
 * 
 * @author Bill Robinson (airbaggins@gmail.com)
 */
public class NativeLoader {

	private static String osName = null;
	private static String osArch = null;
	
	
	public static class NativeLibraryLoadError extends LinkageError {
		
		public NativeLibraryLoadError(String message) {
			super(message);
		}
		
		public NativeLibraryLoadError(String message, Throwable cause) {
			//Convenience constructor with cause as second argument only available in Java 1.7
			super(message);
			initCause(cause);
		}
		
	};
	

	static {
		detectSystem();
	}

	private static void detectSystem() {
		osArch = System.getProperty("os.arch").toLowerCase();
		boolean arch64 = osArch.indexOf("64") != -1;
		if (osArch.indexOf("86") != -1 || osArch.indexOf("amd64") != -1) {
			osArch = "x86";
		}
		if (arch64) {
			osArch += "_64";
		}

		osName = System.getProperty("os.name").toLowerCase();

		if (osName.indexOf("windows") != -1) {
			osName = "windows";
		} else {
			if (osName.indexOf("linux") != -1) {
				osName = "linux";
			} else if (osName.indexOf("mac") != -1) {
				osName = "mac";
			}
		}
	}

	/**
	 * Load the library named, if osNameCheck is the current operating system
	 * and osArchCheck is the current architecture.
	 */
	public static void loadLibrary(String library, String osNameCheck, String osArchCheck) {
		if (osArchCheck == null || osArchCheck.equals(osArch)) {
			loadLibrary(library, osNameCheck);
		}
	}

	/** Load the library named, if osNameCheck is the current operating system. */
	public static void loadLibrary(String library, String osNameCheck) {
		if (osNameCheck == null || osNameCheck.equals(osName)) {
			loadLibrary(library);
		}
	}

	/** Load the library named. */
	public static void loadLibrary(String library) {
		try {
			System.loadLibrary(library);
		} catch (UnsatisfiedLinkError error) {
			loadLibraryFromJar(library);
		}
	}

	/** Try to extract the native library from this Jar file. */
	private static void loadLibraryFromJar(String library) {
		File cacheDir = new File(System.getProperty("java.io.tmpdir"), "pdnative");
		if (!cacheDir.isDirectory()) {
			cacheDir.mkdirs();
		}
		library = System.mapLibraryName(library);
		//TODO What about multiple running instances? Will try to overwrite existing file - Windows might not like that. 
		File fileOut = new File(cacheDir, library);
		InputStream in = PdBase.class.getResourceAsStream("natives/" + osName + "/" + osArch + "/" + library);
		if (in == null) {
			in = PdBase.class.getResourceAsStream("natives/" + osName + "/" + library);
		}
		if (in == null) {
			throw new NativeLibraryLoadError("Couldn't find " + library + " for this platform " + osName + "/" + osArch);
		}
		try {
			OutputStream out = new FileOutputStream(fileOut);
			byte[] copyBuffer = new byte[1024];
			int amountRead;
			while ((amountRead = in.read(copyBuffer)) != -1) {
				out.write(copyBuffer, 0, amountRead);
			}
			in.close();
			out.close();
		} catch (IOException error) {
			throw new NativeLibraryLoadError("Failed to save native library " + library + " to " + cacheDir, error);
		}
		System.load(fileOut.toString());
	}
}
