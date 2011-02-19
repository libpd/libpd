/**
 * 
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 * 
 */

package org.puredata.core.utils;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;

import org.puredata.core.PdBase;

/**
 * 
 * PdUtils provides some convenience methods for interacting with Pd.
 * 
 * @author Peter Brinkmann (peter.brinkmann@gmail.com)
 * 
 * @deprecated The functionality of this class has been integrated into
 *             {@link PdBase}.
 */
public class PdUtils {

	/**
	 * Same as "compute audio" checkbox in pd gui, like [;pd dsp 0/1(
	 * 
	 * Note: Maintaining a DSP state that's separate from the state of the audio
	 * rendering thread doesn't make much sense in libpd. In most applications,
	 * you probably just want to call {@code PdUtils.computeAudio(true)} at the
	 * beginning and then forget that this method exists.
	 * 
	 * @param state
	 * 
	 * @deprecated Use {@link PdBase.computeAudio} instead.
	 */
	public static void computeAudio(boolean state) {
		PdBase.sendMessage("pd", "dsp", state ? 1 : 0);
	}

	/**
	 * Read a patch from a file
	 * 
	 * @param patch
	 *            file
	 * @return pd symbol representing patch
	 * @throws IOException
	 *             in case patch fails to open
	 * 
	 * @deprecated Use {@link PdBase.openPatch} instead.
	 */
	public static String openPatch(File file) throws IOException {
		if (!file.exists()) {
			throw new FileNotFoundException(file.getPath());
		}
		String folder = file.getParentFile().getAbsolutePath();
		String filename = file.getName();
		String patch = "pd-" + filename;
		if (PdBase.exists(patch)) {
			throw new IOException(
					"patch is already open; close first, then reload");
		}
		PdBase.sendMessage("pd", "open", filename, folder);
		if (!PdBase.exists(patch)) {
			throw new IOException("patch " + file.getPath()
					+ " failed to open, no idea why");
		}
		return patch;
	}

	/**
	 * Read a patch from a file
	 * 
	 * @param path
	 *            to file
	 * @return pd symbol representing patch
	 * @throws IOException
	 *             in case patch fails to open
	 * 
	 * @deprecated Use {@link PdBase.openPatch} instead.
	 */
	public static String openPatch(String path) throws IOException {
		return openPatch(new File(path));
	}

	/**
	 * Close a patch
	 * 
	 * @param patch
	 *            name of patch, as returned by openPatch
	 * 
	 * @deprecated Use {@link PdBase.closePatch} instead.
	 */
	public static void closePatch(String patch) {
		PdBase.sendMessage(patch, "menuclose", 1);
	}
}
