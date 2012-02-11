/** 
 * 
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 */

package org.puredata.core;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

/**
 * 
 * PdBase provides basic Java bindings for pd.
 * 
 * Some random notes:
 * 
 * - This is a low-level library that aims to leave most design decisions to
 * higher-level code. In particular, it will throw no exceptions (except for the
 * methods for opening files, which use instances of {@link File} and may throw
 * {@link IOException} when appropriate). At the same time, it is designed to be
 * fairly robust in that it is thread-safe and does as much error checking as I
 * find reasonable at this level. Client code is still responsible for proper
 * dimensioning of buffers and such, though.
 * 
 * - The MIDI methods choose sanity over consistency with pd or the MIDI
 * standard. To wit, channel numbers always start at 0, and pitch bend values
 * are centered at 0, i.e., they range from -8192 to 8191.
 * 
 * - The basic idea is to turn pd into a library that essentially offers a
 * rendering callback (process) mimicking the design of JACK, the JACK Audio
 * Connection Kit.
 * 
 * - The release method is mostly there as a reminder that some sort of cleanup
 * might be necessary; for the time being, it only releases the resources held
 * by the print handler, closes all patches, and cancels all subscriptions.
 * Shutting down pd itself wouldn't make sense because it might be needed in the
 * future, at which point the native library may not be reloaded.
 * 
 * - I'm a little fuzzy on how/when to use sys_lock, sys_unlock, etc., and so I
 * decided to handle all synchronization on the Java side. It appears that
 * sys_lock is for top-level locking in scheduling routines only, and so
 * Java-side sync conveys the same benefits without the risk of deadlocks.
 * 
 * @author Peter Brinkmann (peter.brinkmann@gmail.com)
 * 
 */
public final class PdBase {

	private final static Map<String, Long> bindings = new HashMap<String, Long>();
	private final static Map<Integer, Long> patches = new HashMap<Integer, Long>();

	static {
		System.loadLibrary("pdnative");
		initialize();
	}

	private PdBase() {
		// do nothing
	};

	/**
	 * clears the search path for pd externals
	 */
	public synchronized native static void clearSearchPath();

	/**
	 * adds a directory to the search path
	 * 
	 * @param s
	 */
	public synchronized native static void addToSearchPath(String s);

	/**
	 * same as "compute audio" checkbox in pd gui, or [;pd dsp 0/1(
	 * 
	 * Note: Maintaining a DSP state that's separate from the state of the audio
	 * rendering thread doesn't make much sense in libpd. In most applications,
	 * you probably just want to call {@code computeAudio(true)} at the
	 * beginning and then forget that this method exists.
	 * 
	 * @param state
	 */
	public static void computeAudio(boolean state) {
		sendMessage("pd", "dsp", state ? 1 : 0);
	}

	/**
	 * releases resources held by native bindings (PdReceiver object and
	 * subscriptions); otherwise, the state of pd will remain unaffected
	 * 
	 * Note: It would be nice to free pd's I/O buffers here, but sys_close_audio
	 * doesn't seem to do that, and so we'll just skip this for now.
	 */
	public synchronized static void release() {
		setReceiver(null);
		setMidiReceiver(null);
		for (long ptr : bindings.values()) {
			unbindSymbol(ptr);
		}
		bindings.clear();
		for (long ptr : patches.values()) {
			closeFile(ptr);
		}
		patches.clear();
	}

	/**
	 * sets handler for receiving messages from pd
	 * 
	 * @param handler
	 */
	public synchronized native static void setReceiver(PdReceiver handler);

	/**
	 * sets up pd audio; must be called before process callback
	 * 
	 * @param inputChannels
	 * @param outputChannels
	 * @param sampleRate
	 * @return error code, 0 on success
	 */
	public synchronized native static int openAudio(
			int inputChannels, int outputChannels, int sampleRate);

	/**
	 * raw process callback, processes one pd tick, writes raw data to buffers
	 * without interlacing
	 * 
	 * @param inBuffer
	 *            must be an array of the right size, never null; use inBuffer =
	 *            new short[0] if no input is desired
	 * @param outBuffer
	 *            must be an array of size outBufferSize from openAudio call
	 * @return error code, 0 on success
	 */
	public synchronized native static int processRaw(float[] inBuffer,
			float[] outBuffer);

	/**
	 * main process callback, reads samples from inBuffer and writes samples to
	 * outBuffer, using arrays of type short
	 * 
	 * @param ticks
	 *            the number of Pd ticks (i.e., blocks of 64 frames) to compute
	 * @param inBuffer
	 *            must be an array of the right size, never null; use inBuffer =
	 *            new short[0] if no input is desired
	 * @param outBuffer
	 *            must be an array of size outBufferSize from openAudio call
	 * @return error code, 0 on success
	 */
	public synchronized native static int process(int ticks,
			short[] inBuffer, short[] outBuffer);

	/**
	 * main process callback, reads samples from inBuffer and writes samples to
	 * outBuffer, using arrays of type float
	 * 
	 * @param ticks
	 *            the number of Pd ticks (i.e., blocks of 64 frames) to compute
	 * @param inBuffer
	 *            must be an array of the right size, never null; use inBuffer =
	 *            new short[0] if no input is desired
	 * @param outBuffer
	 *            must be an array of size outBufferSize from openAudio call
	 * @return error code, 0 on success
	 */
	public synchronized native static int process(int ticks,
			float[] inBuffer, float[] outBuffer);

	/**
	 * main process callback, reads samples from inBuffer and writes samples to
	 * outBuffer, using arrays of type double
	 * 
	 * @param ticks
	 *            the number of Pd ticks (i.e., blocks of 64 frames) to compute
	 * @param inBuffer
	 *            must be an array of the right size, never null; use inBuffer =
	 *            new short[0] if no input is desired
	 * @param outBuffer
	 *            must be an array of size outBufferSize from openAudio call
	 * @return error code, 0 on success
	 */
	public synchronized native static int process(int ticks,
			double[] inBuffer, double[] outBuffer);

	/**
	 * @param name of the array in Pd
	 * @return size of the array, or a negative error code if the array does not exist
	 */
	public synchronized native static int arraySize(String name);

	/**
	 * read values from an array in Pd
	 *
	 * @param destination float array to write to
	 * @param destOffset  index at which to start writing
	 * @param source      array in Pd to read from
	 * @param srcOffset   index at which to start reading
	 * @param n           number of values to read
	 * @return            0 on success, or a negative error code on failure
	 */
	public synchronized static int readArray(float[] destination, int destOffset,
			String source, int srcOffset, int n) {
		if (destOffset < 0 || destOffset + n > destination.length) {
			return -2;
		}
		return readArrayNative(destination, destOffset, source, srcOffset, n);
	}
	
	/**
	 * write values to an array in Pd
	 *
	 * @param destination name of the array in Pd to write to
	 * @param destOffset  index at which to start writing
	 * @param source      float array to read from
	 * @param srcOffset   index at which to start reading
	 * @param n           number of values to write
	 * @return            0 on success, or a negative error code on failure
	 */
	public synchronized static int writeArray(String destination, int destOffset, 
			float[] source, int srcOffset, int n) {
		if (srcOffset < 0 || srcOffset + n > source.length) {
			return -2;
		}
		return writeArrayNative(destination, destOffset, source, srcOffset, n);
	}
	
	/**
	 * checks whether a symbol represents a pd object
	 * 
	 * @param s
	 *            String representing pd symbol
	 * @return true if and only if the symbol given by s is associated with
	 *         something in pd
	 */
	public synchronized native static boolean exists(String s);

	/**
	 * subscribes to pd messages sent to the given symbol
	 * 
	 * @param symbol
	 * @return error code, 0 on success
	 */
	public synchronized static int subscribe(String symbol) {
		if (bindings.get(symbol) != null) {
			return 0;
		}
		long ptr = bindSymbol(symbol);
		if (ptr == 0) {
			return -1;
		}
		bindings.put(symbol, ptr);
		return 0;
	}

	/**
	 * unsubscribes from pd messages sent to the given symbol; will do nothing
	 * if there is no subscription to this symbol
	 * 
	 * @param symbol
	 */
	public synchronized static void unsubscribe(String symbol) {
		Long ptr = bindings.remove(symbol);
		if (ptr != null) {
			unbindSymbol(ptr);
		}
	}

	/**
	 * reads a patch from a file
	 * 
	 * @param file
	 * @return an integer handle that identifies this patch; this handle is the
	 *         $0 value of the patch
	 * @throws IOException
	 *             thrown if the file doesn't exist or can't be opened
	 */
	public synchronized static int openPatch(File file) throws IOException {
		if (!file.exists()) {
			throw new FileNotFoundException(file.getPath());
		}
		String name = file.getName();
		File dir = file.getParentFile();
		long ptr = openFile(name, (dir != null) ? dir.getAbsolutePath() : ".");
		if (ptr == 0) {
			throw new IOException("unable to open patch " + file.getPath());
		}
		int handle = getDollarZero(ptr);
		patches.put(handle, ptr);
		return handle;
	}

	/**
	 * reads a patch from a file
	 * 
	 * @param path
	 *            to the file
	 * @return an integer handle that identifies this patch; this handle is the
	 *         $0 value of the patch
	 * @throws IOException
	 *             thrown if the file doesn't exist or can't be opened
	 */
	public synchronized static int openPatch(String path) throws IOException {
		return openPatch(new File(path));
	}

	/**
	 * closes a patch; will do nothing if the handle is invalid
	 * 
	 * @param handle
	 *            representing the patch, as returned by openPatch
	 */
	public synchronized static void closePatch(int handle) {
		Long ptr = patches.remove(handle);
		if (ptr != null) {
			closeFile(ptr);
		}
	}

	/**
	 * sends a bang to the object associated with the given symbol
	 * 
	 * @param recv
	 *            symbol associated with receiver
	 * @return error code, 0 on success
	 */
	public synchronized native static int sendBang(String recv);

	/**
	 * sends a float to the object associated with the given symbol
	 * 
	 * @param recv
	 *            symbol associated with receiver
	 * @param x
	 * @return error code, 0 on success
	 */
	public synchronized native static int sendFloat(String recv, float x);

	/**
	 * sends a symbol to the object associated with the given symbol
	 * 
	 * @param recv
	 *            symbol associated with receiver
	 * @param sym
	 * @return error code, 0 on success
	 */
	public synchronized native static int sendSymbol(String recv, String sym);

	/**
	 * sends a list to an object in pd
	 * 
	 * @param receiver
	 * @param args
	 *            list of arguments of type Integer, Float, or String; no more
	 *            than 32 arguments
	 * @return error code, 0 on success
	 */
	public synchronized static int sendList(String receiver, Object... args) {
		int err = processArgs(args);
		return (err == 0) ? finishList(receiver) : err;
	}

	/**
	 * sends a message to an object in pd
	 * 
	 * @param receiver
	 * @param message
	 * @param args
	 *            list of arguments of type Integer, Float, or String; no more
	 *            than 32 arguments
	 * @return error code, 0 on success
	 */
	public synchronized static int sendMessage(String receiver, String message,
			Object... args) {
		int err = processArgs(args);
		return (err == 0) ? finishMessage(receiver, message) : err;
	}

	/**
	 * @return default pd block size, DEFDACBLKSIZE (currently 64) (aka number
	 *         of samples per tick per channel)
	 */
	public synchronized native static int blockSize();

	/**
	 * sets the handler for receiving MIDI events from pd
	 * 
	 * @param receiver
	 */
	public synchronized native static void setMidiReceiver(
			PdMidiReceiver receiver);

	/**
	 * sends a note on event to pd
	 * 
	 * @param channel
	 *            starting at 0
	 * @param pitch
	 *            0..0x7f
	 * @param velocity
	 *            0..0x7f
	 * @return error code, 0 on success
	 */
	public synchronized native static int sendNoteOn(int channel, int pitch,
			int velocity);

	/**
	 * sends a control change event to pd
	 * 
	 * @param channel
	 *            starting at 0
	 * @param controller
	 *            0..0x7f
	 * @param value
	 *            0..0x7f
	 * @return error code, 0 on success
	 */
	public synchronized native static int sendControlChange(int channel,
			int controller, int value);

	/**
	 * sends a program change event to Pd
	 * 
	 * @param channel
	 *            starting at 0
	 * @param value
	 *            0..0x7f
	 * @return error code, 0 on success
	 */
	public synchronized native static int sendProgramChange(int channel,
			int value);

	/**
	 * sends a pitch bend event to pd
	 * 
	 * @param channel
	 *            starting at 0
	 * @param value
	 *            -8192..8191 (note that Pd has some offset bug in its pitch
	 *            bend objects, but libpd corrects for this)
	 * @return error code, 0 on success
	 */
	public synchronized native static int sendPitchBend(int channel, int value);

	/**
	 * sends an aftertouch event to pd
	 * 
	 * @param channel
	 *            starting at 0
	 * @param value
	 *            0..0x7f
	 * @return error code, 0 on success
	 */
	public synchronized native static int sendAftertouch(int channel, int value);

	/**
	 * sends a polyphonic aftertouch event to pd
	 * 
	 * @param channel
	 *            starting at 0
	 * @param pitch
	 *            0..0x7f
	 * @param value
	 *            0..0x7f
	 * @return error code, 0 on success
	 */
	public synchronized native static int sendPolyAftertouch(int channel,
			int pitch, int value);

	/**
	 * sends one raw MIDI byte to pd
	 * 
	 * @param port
	 *            0..0x0fff
	 * @param value
	 *            0..0xff
	 * @return error code, 0 on success
	 */
	public synchronized native static int sendMidiByte(int port, int value);

	/**
	 * sends one byte of a sysex message to pd
	 * 
	 * @param port
	 *            0..0x0fff
	 * @param value
	 *            0..0x7f
	 * @return error code, 0 on success
	 */
	public synchronized native static int sendSysex(int port, int value);

	/**
	 * sends one byte to the realtimein object of pd
	 * 
	 * @param port
	 *            0..0x0fff
	 * @param value
	 *            0..0xff
	 * @return error code, 0 on success
	 */
	public synchronized native static int sendSysRealTime(int port, int value);

	private static int processArgs(Object[] args) {
		if (startMessage(args.length) != 0) {
			return -100;
		}
		for (Object arg : args) {
			if (arg instanceof Integer) {
				addFloat(((Integer) arg).intValue());
			} else if (arg instanceof Float) {
				addFloat(((Float) arg).floatValue());
			} else if (arg instanceof Double) {
				addFloat(((Double) arg).floatValue());
			} else if (arg instanceof String) {
				addSymbol((String) arg);
			} else {
				return -101; // illegal argument
			}
		}
		return 0;
	}

	// the remaining methods do not need to be synchronized because they are
	// protected by the public methods that call them

	private native static void initialize();

	private native static int startMessage(int length);

	private native static void addFloat(float x);

	private native static void addSymbol(String s);

	private native static int finishList(String receive);

	private native static int finishMessage(String receive, String message);

	private native static long bindSymbol(String s);

	private native static void unbindSymbol(long p);

	private native static long openFile(String patch, String dir);

	private native static void closeFile(long p);

	private native static int getDollarZero(long p);
	
	private native static int readArrayNative(float[] destination, int destOffset,
			String source, int srcOffset, int n);
	
	private native static int writeArrayNative(String destination, int destOffset, 
			float[] source, int srcOffset, int n);
}
