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
 */
public class PdUtils {
	
	/**
	 * Same as "compute audio" checkbox in pd gui, like [;pd dsp 0/1(
	 * 
	 * @param flag
	 */
	public static void computeAudio(boolean flag) {
		PdBase.sendMessage("pd", "dsp", flag ? 1 : 0);
	}
	
	/**
	 * Read a patch from a file
	 * 
	 * @param patch file
	 * @return pd symbol representing patch
	 * @throws IOException in case patch fails to open
	 */
	public static String openPatch(File file) throws IOException {
		if (!file.exists()) {
			throw new FileNotFoundException(file.getPath());
		}
		String folder = file.getParentFile().getAbsolutePath();
		String filename = file.getName();
		String patch = "pd-" + filename;
		if (PdBase.exists(patch)) {
			throw new IOException("patch is already open; close first, then reload");
		}
		PdBase.sendMessage("pd", "open", filename, folder);
		if (!PdBase.exists(patch)) {
			throw new IOException("patch " + file.getPath() + " failed to open, no idea why");
		}
		return patch;
	}
	
	/**
	 * Read a patch from a file
	 * 
	 * @param path to file
	 * @return pd symbol representing patch
	 * @throws IOException in case patch fails to open
	 */
	public static String openPatch(String path) throws IOException {
		return openPatch(new File(path));
	}
	
	/**
	 * Close a patch
	 * 
	 * @param patch name of patch, as returned by openPatch
	 */
	public static void closePatch(String patch) {
		PdBase.sendMessage(patch, "menuclose");
	}
	
	/**
	 * Send a MIDI note event to Pd
	 * 
	 * @param channel  according to MIDI standard, ranging from 1 to 16
	 * @param pitch
	 * @param velocity
	 */
	public static void sendNote(int channel, int pitch, int velocity) {
		checkMidiChannel(channel);
		checkMidiValue(pitch);
		checkMidiValue(velocity);
		PdBase.sendList("#notein", pitch, velocity, channel);  // note: sending MIDI event as a message may be fragile since it uses an undocumented Pd feature
	}
	
	/**
	 * Send a MIDI control change event to Pd
	 * 
	 * @param channel  according to MIDI standard, ranging from 1 to 16
	 * @param controller
	 * @param value
	 */
	public static void sendControlChange(int channel, int controller, int value) {
		checkMidiChannel(channel);
		checkMidiValue(controller);
		checkMidiValue(value);
		PdBase.sendList("#ctlin", controller, value, channel);  // note: sending MIDI event as a message may be fragile since it uses an undocumented Pd feature
	}
	
	/**
	 * Send a MIDI program change event to Pd
	 * 
	 * @param channel  according to MIDI standard, ranging from 1 to 16
	 * @param program
	 */
	public static void sendProgramChange(int channel, int program) {
		checkMidiChannel(channel);
		checkMidiValue(program);
		PdBase.sendList("#pgmin", program, channel);  // note: sending MIDI event as a message may be fragile since it uses an undocumented Pd feature
	}
	
	/**
	 * Send MIDI pitch bend event to Pd
	 * 
	 * @param channel  according to MIDI standard, ranging from 1 to 16
	 * @param value    in range from -8192 to 8191
	 */
	public static void sendPitchBend(int channel, int value) {
		checkMidiChannel(channel);
		if (value < -8192 || value > 8191) {
			throw new IllegalArgumentException("MIDI pitch bend out of range");
		}
		PdBase.sendList("#bendin", value, channel);  // note: sending MIDI event as a message may be fragile since it uses an undocumented Pd feature
	}
	
	/**
	 * Send MIDI after touch event to Pd
	 * 
	 * @param channel  according to MIDI standard, ranging from 1 to 16
	 * @param value
	 */
	public static void sendAfterTouch(int channel, int value) {
		checkMidiChannel(channel);
		checkMidiValue(value);
		PdBase.sendList("#touchin", value, channel);  // note: sending MIDI event as a message may be fragile since it uses an undocumented Pd feature
	}

	/**
	 * Send polyphonic MIDI touch event to Pd
	 * 
	 * @param channel  according to MIDI standard, ranging from 1 to 16
	 * @param pitch
	 * @param value
	 */
	public static void sendPolyTouch(int channel, int pitch, int value) {
		checkMidiChannel(channel);
		checkMidiValue(pitch);
		checkMidiValue(value);
		PdBase.sendList("#polytouchin", pitch, value, channel);  // note: sending MIDI event as a message may be fragile since it uses an undocumented Pd feature
	}

	private static void checkMidiChannel(int channel) {
		if (channel < 1 || channel > 16) {
			throw new IllegalArgumentException("MIDI channel out of range");
		}
	}
	
	private static void checkMidiValue(int value) {
		if (value < 0 || value > 127) {
			throw new IllegalArgumentException("MIDI value out of range");
		}
	}
}
