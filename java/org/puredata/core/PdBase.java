/**
 * 
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL WARRANTIES, see the
 * file, "LICENSE.txt," in this distribution.
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
 * PdBase provides basic Java bindings for Pd.
 * 
 * Some random notes:
 * 
 * - This is a low-level library that aims to leave most design decisions to higher-level code. In
 * particular, it will throw no exceptions (except for the methods for opening files, which use
 * instances of {@link File} and may throw {@link IOException} when appropriate). At the same time,
 * it is designed to be fairly robust in that it is thread-safe and does as much error checking as I
 * find reasonable at this level. Client code is still responsible for proper dimensioning of
 * buffers and such, though.
 * 
 * - The MIDI methods choose sanity over consistency with Pd or the MIDI standard. To wit, channel
 * numbers always start at 0, and pitch bend values are centered at 0, i.e., they range from -8192
 * to 8191.
 * 
 * - The basic idea is to turn Pd into a library that essentially offers a rendering callback
 * (process) mimicking the design of JACK, the JACK Audio Connection Kit.
 * 
 * - The release method is mostly there as a reminder that some sort of cleanup might be necessary;
 * for the time being, it only releases the resources held by the print handler, closes all patches,
 * and cancels all subscriptions. Shutting down Pd itself wouldn't make sense because it might be
 * needed in the future, at which point the native library may not be reloaded.
 * 
 * @author Peter Brinkmann (peter.brinkmann@gmail.com)
 * 
 */
public final class PdBase {

  private final static Map<String, Long> bindings = new HashMap<String, Long>();
  private final static Map<Integer, Long> patches = new HashMap<Integer, Long>();
  private static PdMidiReceiver midiReceiver = null;

  static {
    PdBaseLoader.loaderHandler.load();
    initialize();
  }

  private PdBase() {
    // do nothing
  };

  /**
   * Releases resources held by native bindings (receiver objects and subscriptions, as well as
   * patches); otherwise, the state of Pd will remain unaffected.
   */
  public synchronized static void release() {
    closeAudio();
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
   * Clears the search path for Pd externals.
   */
  public native static void clearSearchPath();

  /**
   * Adds a directory to the search path.
   *
   * @param s Directory path to add.
   */
  public native static void addToSearchPath(String s);

  /**
   * Sets the handler for receiving messages from Pd.
   *
   * @param receiver Receiver to use.
   */
  public native static void setReceiver(PdReceiver receiver);

  /**
   * Sets the handler for receiving MIDI events from Pd.
   *
   * @param receiver MIDI receiver to use.
   */
  public static void setMidiReceiver(PdMidiReceiver receiver) {
    midiReceiver = receiver;
    setMidiReceiverInternal(receiver);
  }

  private native static void setMidiReceiverInternal(PdMidiReceiver receiver);

  /**
   * Sets up Pd audio; must be called before rendering audio with process or startAudio.
   *
   * @param inputChannels Number of input channles
   * @param outputChannels Number of output channels
   * @param sampleRate Audio sample rate
   * @return error code, 0 on success
   */
  public static int openAudio(int inputChannels, int outputChannels, int sampleRate) {
    return openAudio(inputChannels, outputChannels, sampleRate, null);
  }

  /**
   * Sets up Pd audio; must be called before rendering audio with process or startAudio.
   *
   * @param inputChannels Number of input channles
   * @param outputChannels Number of output channels
   * @param sampleRate Audio sample rate
   * @param options Audio backend options, reserved for future use.
   * @return error code, 0 on success
   */
  public native static int openAudio(int inputChannels, int outputChannels, int sampleRate,
      Map<String, String> options);

  /**
   * Indicates whether the underlying binary implements audio, e.g., with OpenSL or PortAudio or
   * JACK.
   *
   * @return true there is an audio implementation
   */
  public native static boolean implementsAudio();

  /**
   * Indicates how the underlying binary implements audio, e.g., with OpenSL or PortAudio or JACK.
   *
   * @return audio backend name or null if there is no audio implementation
   */
  public native static String audioImplementation();

  /**
   * Returns a sample rate recommendation, or a negative value if no recommendation is available.
   *
   * @return recommended sample rate
   */
  public native static int suggestSampleRate();

  /**
   * Returns a recommendation for the number of input channels, or a negative value if no
   * recommendation is available.
   *
   * @return recommended number of input channels
   */
  public native static int suggestInputChannels();

  /**
   * Returns a recommendation for the number of output channels, or a negative value if no
   * recommendation is available.
   *
   * @return recommended number of output channels
   */
  public native static int suggestOutputChannels();

  /**
   * Closes the audio components if implementsAudio is true, otherwise no-op.
   */
  public native static void closeAudio();

  /**
   * Starts audio rendering if implementsAudio is true, otherwise no-op.
   *
   * @return error code, 0 on success
   */
  public native static int startAudio();

  /**
   * Pauses audio rendering if implementsAudio is true, otherwise no-op.
   *
   * @return error code, 0 on success
   */
  public native static int pauseAudio();

  /**
   * Indicates whether audio is running if implementsAudio is true; returns false otherwise.
   *
   * @return true if audio is running
   */
  public native static boolean isRunning();

  /**
   * Reads a patch from a file.
   * 
   * @param file Patch to open
   * @return an integer handle that identifies this patch; this handle is the $0 value of the patch
   * @throws IOException thrown if the file doesn't exist or can't be opened
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
   * Reads a patch from a file.
   * 
   * @param path to the file
   * @return an integer handle that identifies this patch; this handle is the $0 value of the patch
   * @throws IOException thrown if the file doesn't exist or can't be opened
   */
  public synchronized static int openPatch(String path) throws IOException {
    return openPatch(new File(path));
  }

  /**
   * Closes a patch; will do nothing if the handle is invalid.
   * 
   * @param handle representing the patch, as returned by openPatch
   */
  public synchronized static void closePatch(int handle) {
    Long ptr = patches.remove(handle);
    if (ptr != null) {
      closeFile(ptr);
    }
  }

  /**
   * Same as "compute audio" checkbox in Pd GUI, or [;pd dsp 0/1(
   * 
   * Note: Maintaining a DSP state that's separate from the state of the audio rendering thread
   * doesn't make much sense in libpd. In most applications, you probably just want to call
   * {@code computeAudio(true)} at the beginning and then forget that this method exists.
   *
   * @param state DSP state: true for on, false for off
   */
  public static void computeAudio(boolean state) {
    sendMessage("pd", "dsp", state ? 1 : 0);
  }

  /**
   * Sends a bang to the object associated with the given symbol.
   * 
   * @param recv symbol associated with receiver
   * @return error code, 0 on success
   */
  public native static int sendBang(String recv);

  /**
   * Sends a float to the object associated with the given symbol.
   * 
   * @param recv symbol associated with receiver
   * @param x float value to send to receiver
   * @return error code, 0 on success
   */
  public native static int sendFloat(String recv, float x);

  /**
   * Sends a symbol to the object associated with the given symbol.
   * 
   * @param recv symbol associated with receiver
   * @param sym symbol to send to receiver
   * @return error code, 0 on success
   */
  public native static int sendSymbol(String recv, String sym);

  /**
   * Sends a list to an object in Pd.
   * 
   * @param recv symbol associated with receiver
   * @param args list of arguments of type Integer, Float, or String
   * @return error code, 0 on success
   */
  public synchronized static int sendList(String recv, Object... args) {
    int err = processArgs(args);
    return (err == 0) ? finishList(recv) : err;
  }

  /**
   * Sends a typed message to an object in Pd.
   * 
   * @param recv symbol associated with receiver
   * @param msg first symbol of message
   * @param args list of arguments of type Integer, Float, or String
   * @return error code, 0 on success
   */
  public synchronized static int sendMessage(String recv, String msg, Object... args) {
    int err = processArgs(args);
    return (err == 0) ? finishMessage(recv, msg) : err;
  }

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

  /**
   * Checks whether a symbol represents a Pd object.
   * 
   * @param s String representing Pd symbol
   * @return true if and only if the symbol given by s is associated with something in Pd
   */
  public native static boolean exists(String s);

  /**
   * Subscribes to Pd messages sent to the given symbol.
   * 
   * @param symbol to subscribe to
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
   * Unsubscribes from Pd messages sent to the given symbol; will do nothing if there is no
   * subscription to this symbol.
   * 
   * @param symbol to unsubscribe from
   */
  public synchronized static void unsubscribe(String symbol) {
    Long ptr = bindings.remove(symbol);
    if (ptr != null) {
      unbindSymbol(ptr);
    }
  }

  /**
   * Returns the size of an array in Pd.
   * 
   * @param name of the array in Pd
   * @return size of the array, or a negative error code if the array does not exist
   */
  public native static int arraySize(String name);

  /**
   * Reads values from an array in Pd.
   * 
   * @param destination float array to write to
   * @param destOffset index at which to start writing
   * @param source array in Pd to read from
   * @param srcOffset index at which to start reading
   * @param n number of values to read
   * @return 0 on success, or a negative error code on failure
   */
  public static int readArray(float[] destination, int destOffset, String source, int srcOffset,
      int n) {
    if (destOffset < 0 || destOffset + n > destination.length) {
      return -2;
    }
    return readArrayNative(destination, destOffset, source, srcOffset, n);
  }

  /**
   * Writes values to an array in Pd.
   * 
   * @param destination name of the array in Pd to write to
   * @param destOffset index at which to start writing
   * @param source float array to read from
   * @param srcOffset index at which to start reading
   * @param n number of values to write
   * @return 0 on success, or a negative error code on failure
   */
  public static int writeArray(String destination, int destOffset, float[] source, int srcOffset,
      int n) {
    if (srcOffset < 0 || srcOffset + n > source.length) {
      return -2;
    }
    return writeArrayNative(destination, destOffset, source, srcOffset, n);
  }

  /**
   * Sends a note on event to Pd.
   * 
   * @param channel starting at 0
   * @param pitch 0..0x7f
   * @param velocity 0..0x7f
   * @return error code, 0 on success
   */
  public native static int sendNoteOn(int channel, int pitch, int velocity);

  /**
   * Sends a control change event to Pd.
   * 
   * @param channel starting at 0
   * @param controller 0..0x7f
   * @param value 0..0x7f
   * @return error code, 0 on success
   */
  public native static int sendControlChange(int channel, int controller, int value);

  /**
   * Sends a program change event to Pd.
   * 
   * @param channel starting at 0
   * @param value 0..0x7f
   * @return error code, 0 on success
   */
  public native static int sendProgramChange(int channel, int value);

  /**
   * Sends a pitch bend event to Pd.
   * 
   * @param channel starting at 0
   * @param value -8192..8191 (note that Pd has some offset bug in its pitch bend objects, but libpd
   *        corrects for this)
   * @return error code, 0 on success
   */
  public native static int sendPitchBend(int channel, int value);

  /**
   * Sends an aftertouch event to Pd.
   * 
   * @param channel starting at 0
   * @param value 0..0x7f
   * @return error code, 0 on success
   */
  public native static int sendAftertouch(int channel, int value);

  /**
   * Sends a polyphonic aftertouch event to Pd.
   * 
   * @param channel starting at 0
   * @param pitch 0..0x7f
   * @param value 0..0x7f
   * @return error code, 0 on success
   */
  public native static int sendPolyAftertouch(int channel, int pitch, int value);

  /**
   * Sends one raw MIDI byte to Pd.
   * 
   * @param port 0..0x0fff
   * @param value 0..0xff
   * @return error code, 0 on success
   */
  public native static int sendMidiByte(int port, int value);

  /**
   * Sends one byte of a sysex message to Pd.
   * 
   * @param port 0..0x0fff
   * @param value 0..0x7f
   * @return error code, 0 on success
   */
  public native static int sendSysex(int port, int value);

  /**
   * Sends one byte to the realtimein object of Pd.
   * 
   * @param port 0..0x0fff
   * @param value 0..0xff
   * @return error code, 0 on success
   */
  public native static int sendSysRealTime(int port, int value);

  /**
   * Polls the Pd message queue and invokes Pd message callbacks as appropriate.
   */
  public native static void pollPdMessageQueue();

  /**
   * Polls the MIDI queue and invokes MIDI callbacks as appropriate.
   */
  public static void pollMidiQueue() {
    if (midiReceiver != null) {
      midiReceiver.beginBlock();
      pollMidiQueueInternal();
      midiReceiver.endBlock();
    } else {
      pollMidiQueueInternal();
    }
  }

  private native static void pollMidiQueueInternal();

  /**
   * Returns the number of frames per Pd tick (currently 64).
   *
   * @return number of frames per Pd tick
   */
  public native static int blockSize();

  /**
   * Raw process callback, processes one Pd tick, writes raw data to buffers without interlacing.
   * 
   * @param inBuffer must be an array of the right size, never null; use inBuffer = new short[0] if
   *        no input is desired
   * @param outBuffer must be an array of size outBufferSize from openAudio call
   * @return error code, 0 on success
   */
  public native static int processRaw(float[] inBuffer, float[] outBuffer);

  /**
   * Main process callback; reads samples from inBuffer and writes samples to outBuffer, using
   * arrays of type short.
   * 
   * @param ticks the number of Pd ticks (i.e., blocks of 64 frames) to compute
   * @param inBuffer must be an array of the right size, never null; use inBuffer = new short[0] if
   *        no input is desired
   * @param outBuffer must be an array of size outBufferSize from openAudio call
   * @return error code, 0 on success
   */
  public native static int process(int ticks, short[] inBuffer, short[] outBuffer);

  /**
   * Main process callback; reads samples from inBuffer and writes samples to outBuffer, using
   * arrays of type float.
   * 
   * @param ticks the number of Pd ticks (i.e., blocks of 64 frames) to compute
   * @param inBuffer must be an array of the right size, never null; use inBuffer = new short[0] if
   *        no input is desired
   * @param outBuffer must be an array of size outBufferSize from openAudio call
   * @return error code, 0 on success
   */
  public native static int process(int ticks, float[] inBuffer, float[] outBuffer);

  /**
   * Main process callback; reads samples from inBuffer and writes samples to outBuffer, using
   * arrays of type double.
   * 
   * @param ticks the number of Pd ticks (i.e., blocks of 64 frames) to compute
   * @param inBuffer must be an array of the right size, never null; use inBuffer = new short[0] if
   *        no input is desired
   * @param outBuffer must be an array of size outBufferSize from openAudio call
   * @return error code, 0 on success
   */
  public native static int process(int ticks, double[] inBuffer, double[] outBuffer);

  private native static void initialize();

  private native static long openFile(String patch, String dir);

  private native static void closeFile(long p);

  private native static int getDollarZero(long p);

  private native static int startMessage(int length);

  private native static void addFloat(float x);

  private native static void addSymbol(String s);

  private native static int finishList(String receive);

  private native static int finishMessage(String receive, String message);

  private native static int readArrayNative(float[] destination, int destOffset, String source,
      int srcOffset, int n);

  private native static int writeArrayNative(String destination, int destOffset, float[] source,
      int srcOffset, int n);

  private native static long bindSymbol(String s);

  private native static void unbindSymbol(long p);

}
