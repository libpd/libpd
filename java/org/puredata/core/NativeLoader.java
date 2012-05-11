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
 * Utilities for loading platform-specific libraries from the jar file.
 * 
 * @author Bill Robinson (airbaggins@gmail.com)
 */
public class NativeLoader {

	
	private static String osName = null;
	private static String osArch = null;
	private static String libPrefix = "";
	private static String libSuffix = "";
	
	static {
		detectSystem();
	}
	
	private static void detectSystem()
	{
		if (osName == null)
		{
			osArch = System.getProperty("os.arch").toLowerCase();
			boolean arch64 = osArch.indexOf("64") != -1;
			if (osArch.indexOf("86") != -1 || osArch.indexOf("amd64") != -1) {
				osArch = "x86";
			} else if (osArch.indexOf("powerpc") != -1 || osArch.indexOf("ppc") != -1) {
				osArch = "ppc";
			} else {
				throw new RuntimeException("Unknown architecture " + osName);
			}
			if (arch64) {
				osArch += "_64";
			}

			osName = System.getProperty("os.name").toLowerCase();

			if (osName.indexOf("windows") != -1) {
				osName = "windows";
				libSuffix = ".dll";
			}
			else {
				libPrefix = "lib";
				if (osName.indexOf("linux") != -1) {
					osName = "linux";
					libSuffix = ".so";
				} else if (osName.indexOf("mac") != -1) {
					osName = "mac";
					libSuffix = ".dylib";
				} else {
					throw new RuntimeException("Unknown OS " + osName);
				}
			}
		}
	}
	
	/** Load the library named, if osNameCheck is the current operating system and osArchCheck is the current architecture. */
	static void loadLibrary(String library, String osNameCheck, String osArchCheck)
	{
		if (osArchCheck == null || osArchCheck.equals(osArch))
		{
			loadLibrary(library, osNameCheck);
		}
	}
	
	/** Load the library named, if osNameCheck is the current operating system. */
	static void loadLibrary(String library, String osNameCheck)
	{
		if (osNameCheck == null || osNameCheck.equals(osName))
		{
			loadLibrary(library);
		}
	}
	
	/** Load the library named. */
	static void loadLibrary(String library) {
		try {
			System.loadLibrary(library);
		} catch (UnsatisfiedLinkError error) {
			loadLibraryFromJar(library);
		}
	}

	/** Try to extract the native library from this Jar file. */
	private static void loadLibraryFromJar(String library) {
		File cacheDir = new File(System.getProperty("java.io.tmpdir") + File.separator + "pdnative");
		if (!cacheDir.isDirectory()) {
			cacheDir.mkdirs();
		}
		library = libPrefix + library + libSuffix;
		File fileOut = new File(cacheDir, library);
		InputStream in = PdBase.class.getResourceAsStream("/org/puredata/natives/" + osName + "/" + osArch + "/" + library);
		if (in == null) {
		    in = PdBase.class.getResourceAsStream("/org/puredata/natives/" + osName + "/" + library);
		}
		if (in == null) {
			throw new RuntimeException("Couldn't find " + library + " for this platform " + osName + "/" + osArch);
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
			System.load(fileOut.toString());
		} catch (IOException error) {
			throw new RuntimeException(error);
		}
	}
}
