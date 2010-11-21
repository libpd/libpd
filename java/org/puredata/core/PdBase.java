/** 
 * 
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 */

package org.puredata.core;

import java.util.HashMap;
import java.util.Map;

/**
 * 
 * PdBase provides basic Java bindings for pd.
 * 
 * Some random notes:
 *    - This is a low-level library that aims to leave most design decisions to higher-level code.  In particular, it will
 *      throw no exceptions (although client code may choose to translate error codes into exceptions).  At the same time,
 *      it is designed to be fairly robust in that it is thread-safe and does as much error checking as I find reasonable
 *      at this level.  Client code is still responsible for proper dimensioning of buffers and such, though.
 *      
 *    - {@link PdUtils} is an example of how to wrap PdBase in higher-level functionality.
 *    
 *    - The basic idea is to turn pd into a library that essentially offers a rendering callback (process) mimicking the
 *      design of JACK, the JACK Audio Connection Kit.
 *      
 *    - The release method is mostly there as a reminder that some sort of cleanup might be necessary; for the time being,
 *      it only releases the resources held by the print handler and cancels all subscriptions.  Shutting down pd itself
 *      wouldn't make sense because it might be needed in the future, at which point the native library may not be reloaded.
 *      If you're concerned about resources held by pd, make sure to call closePatch when you're done with a patch.
 *      
 *    - I'm a little fuzzy on how/when to use sys_lock, sys_unlock, etc., and so I decided to handle all synchronization on
 *      the Java side.  It appears that sys_lock is for top-level locking in scheduling routines only, and so Java-side sync
 *      conveys the same benefits without the risk of deadlocks.
 *      
 * @author Peter Brinkmann (peter.brinkmann@gmail.com)
 *
 */
public final class PdBase {

	private final static Map<String, Long> bindings = new HashMap<String, Long>();
	
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
	 * @param s
	 */
	public synchronized native static void addToSearchPath(String s);
	
	/**
	 * releases resources held by native bindings (PdReceiver object and subscriptions); otherwise, the state of pd will remain unaffected
	 * 
	 * Note:  It would be nice to free pd's I/O buffers here, but sys_close_audio doesn't seem
	 * to do that, and so we'll just skip this for now.
	 */
	public synchronized static void release() {
		setReceiver(null);
		for (long ptr: bindings.values()) {
			unbindSymbol(ptr);
		}
		bindings.clear();
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
	 * @param ticksPerBuffer number of pd ticks per process call
	 * @return error code, 0 on success
	 */
	public synchronized native static int openAudio(int inputChannels, int outputChannels, int sampleRate, int ticksPerBuffer);

	/**
	 * raw process callback, processes one pd tick, writes raw data to buffers without interlacing
	 * 
	 * @param inBuffer  must be an array of the right size, never null; use inBuffer = new short[0] if no input is desired
	 * @param outBuffer must be an array of size outBufferSize from openAudio call
	 * @return error code, 0 on success
	 */
	public synchronized native static int processRaw(float[] inBuffer, float[] outBuffer);
	
	/**
	 * main process callback, reads samples from inBuffer and writes samples to outBuffer, using arrays of type short
	 * 
	 * @param inBuffer  must be an array of the right size, never null; use inBuffer = new short[0] if no input is desired
	 * @param outBuffer must be an array of size outBufferSize from openAudio call
	 * @return error code, 0 on success
	 */
	public synchronized native static int process(short[] inBuffer, short[] outBuffer);
	
	/**
	 * main process callback, reads samples from inBuffer and writes samples to outBuffer, using arrays of type float
	 * 
	 * @param inBuffer  must be an array of the right size, never null; use inBuffer = new short[0] if no input is desired
	 * @param outBuffer must be an array of size outBufferSize from openAudio call
	 * @return error code, 0 on success
	 */
	public synchronized native static int process(float[] inBuffer, float[] outBuffer);
	
	/**
	 * main process callback, reads samples from inBuffer and writes samples to outBuffer, using arrays of type double
	 * 
	 * @param inBuffer  must be an array of the right size, never null; use inBuffer = new short[0] if no input is desired
	 * @param outBuffer must be an array of size outBufferSize from openAudio call
	 * @return error code, 0 on success
	 */
	public synchronized native static int process(double[] inBuffer, double[] outBuffer);
	
	/**
	 * checks whether a symbol represents a pd object
	 * 
	 * @param s String representing pd symbol
	 * @return true if and only if the symbol given by s is associated with something in pd
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
	 * unsubscribes from pd messages sent to the given symbol
	 * 
	 * @param symbol
	 */
	public synchronized static void unsubscribe(String symbol) {
		Long ptr = bindings.get(symbol);
		if (ptr == null) return;
		bindings.remove(symbol);
		unbindSymbol(ptr);
	}

	/**
	 * sends a bang to the object associated with the given symbol
	 * 
	 * @param recv symbol associated with receiver
	 * @return error code, 0 on success
	 */
	public synchronized native static int sendBang(String recv);
	
	/**
	 * sends a float to the object associated with the given symbol
	 * 
	 * @param recv symbol associated with receiver
	 * @param x
	 * @return error code, 0 on success
	 */
	public synchronized native static int sendFloat(String recv, float x);
	
	/**
	 * sends a symbol to the object associated with the given symbol
	 * 
	 * @param recv symbol associated with receiver
	 * @param sym
	 * @return error code, 0 on success
	 */
	public synchronized native static int sendSymbol(String recv, String sym);
	
	/**
	 * sends a list to an object in pd
	 * @param receiver
	 * @param args list of arguments of type Integer, Float, or String; no more than 32 arguments
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
	 * @param args  list of arguments of type Integer, Float, or String; no more than 32 arguments
	 * @return error code, 0 on success
	 */
	public synchronized static int sendMessage(String receiver, String message, Object... args) {
		int err = processArgs(args);			
		return (err == 0) ? finishMessage(receiver, message) : err;
	}

	/**
	 * @return default pd block size, DEFDACBLKSIZE (currently 64) (aka number of samples per tick per channel)
	 */
	public native static int blockSize();
	
	
	private static int processArgs(Object[] args) {
		int maxArgs = startMessage();
		if (args.length > maxArgs) {
			return -100;
		}
		for (Object arg: args) {
			if (arg instanceof Integer) {
				addFloat(((Integer) arg).intValue());
			} else if (arg instanceof Float) {
				addFloat(((Float) arg).floatValue());
			} else if (arg instanceof String) {
				addSymbol((String) arg);
			} else {
				return -101;  // illegal argument
			}
		}
		return 0;
	}
	
	private native static void initialize();  // no sync necessary because it only runs in static initializer
	private native static int startMessage();
	private native static void addFloat(float x);
	private native static void addSymbol(String s);
	private native static int finishList(String receive);
	private native static int finishMessage(String receive, String message);
	private native static long bindSymbol(String s);
	private native static void unbindSymbol(long p);
}
