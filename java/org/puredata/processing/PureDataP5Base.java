/**
 * 
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 * 
 */

package org.puredata.processing;


import java.io.File;
import java.io.IOException;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Map;

import org.puredata.core.PdBase;
import org.puredata.core.PdReceiver;


/**
 * Abstract base class for using libpd with Processing.  It provides a
 * wrapper for libpd that is in line with the general look and feel of
 * Processing libraries.  Subclasses will provide the connection between
 * libpd and the audio platform, such as JACK.
 * 
 * @author Peter Brinkmann (peter.brinkmann@gmail.com)
 */
public abstract class PureDataP5Base implements PdReceiver {

	private final Object parent;
	private final Method pdPrintCallback;
	private final Method receiveBangCallback;
	private final Method receiveFloatCallback;
	private final Method receiveSymbolCallback;
	private final Method receiveListCallback;
	private final Method receiveMessageCallback;

	/**
	 * Constructor.  The parent object will always be an instance of PApplet,
	 * but we don't use PApplet here because we don't want any Processing
	 * dependency in libpd itself.
	 * 
	 * @param parent owner, instance of PApplet
	 */
	public PureDataP5Base(Object parent) {
		this.parent = parent;
		Map<String, Method> methods = extractMethods(parent.getClass());
		try {
			methods.get("registerDispose").invoke(parent, this);
		} catch (Exception e) {
			// Not an instance of PApplet, perhaps?
			throw new RuntimeException(e);
		}
		pdPrintCallback = methods.get("pdPrint");
		receiveBangCallback = methods.get("receiveBang");
		receiveFloatCallback = methods.get("receiveFloat");
		receiveSymbolCallback = methods.get("receiveSymbol");
		receiveListCallback = methods.get("receiveList");
		receiveMessageCallback = methods.get("receiveMessage");
		PdBase.setReceiver(this);
		PdBase.computeAudio(true);
	}
	
	private Map<String, Method> extractMethods(Class<?> clazz) {
		Map<String, Method> result = new HashMap<String, Method>();
		Method[] methods = clazz.getMethods();
		for (Method method: methods) {
			result.put(method.getName(), method);
		}
		return result;
	}
	
	/**
	 * Processing dispose callback, automatically registered in the constructor.
	 */
	public void dispose() {
		stop();
		PdBase.release();
	}
	
	/**
	 * Start audio.
	 */
	public abstract void start();
	
	/**
	 * Stop audio.
	 */
	public abstract void stop();
	
	/**
	 * Delegates to the corresponding method in {@link PdBase}.
	 */
	public int subscribe(String sym) {
		return PdBase.subscribe(sym);
	}
	
	/**
	 * Delegates to the corresponding method in {@link PdBase}.
	 */
	public void unsubscribe(String sym) {
		PdBase.unsubscribe(sym);
	}

	/**
	 * Delegates to the corresponding method in {@link PdBase}.
	 */
	public int openPatch(File file) {
		try {
			return PdBase.openPatch(file);
		} catch (IOException e) {
			throw new RuntimeException(e);
		}
	}

	/**
	 * Delegates to the corresponding method in {@link PdBase}.
	 */
	public void closePatch(int handle) {
		PdBase.closePatch(handle);
	}

	/**
	 * Delegates to the corresponding method in {@link PdBase}.
	 */
	public int arraySize(String name) {
		return PdBase.arraySize(name);
	}
	
	/**
	 * Delegates to the corresponding method in {@link PdBase}.
	 */
	public int readArray(float[] to, int toOffset, String fromArray, int fromOffset, int n) {
		return PdBase.readArray(to, toOffset, fromArray, fromOffset, n);
	}
	
	/**
	 * Delegates to the corresponding method in {@link PdBase}.
	 */
	public int writeArray(String toArray, int toOffset, float[] fromArray, int fromOffset, int n) {
		return PdBase.writeArray(toArray, toOffset, fromArray, fromOffset, n);
	}
	
	/**
	 * Delegates to the corresponding method in {@link PdBase}.
	 */
	public void sendBang(String recv) {
		PdBase.sendBang(recv);
	}

	/**
	 * Delegates to the corresponding method in {@link PdBase}.
	 */
	public void sendFloat(String recv, float x) {
		PdBase.sendFloat(recv, x);
	}

	/**
	 * Delegates to the corresponding method in {@link PdBase}.
	 */
	public void sendSymbol(String recv, String sym) {
		PdBase.sendSymbol(recv, sym);
	}

	/**
	 * Delegates to the corresponding method in {@link PdBase}.
	 */
	public void sendList(String recv, Object... args) {
		PdBase.sendList(recv, args);
	}

	/**
	 * Delegates to the corresponding method in {@link PdBase}.
	 */
	public void sendMessage(String recv, String mesg, Object... args) {
		PdBase.sendMessage(recv, mesg, args);
	}
	
	/**
	 * Delegates to parent's pdPrint method, if it exists; no-op otherwise.
	 */
	@Override
	public void print(String s) {
		if (pdPrintCallback != null) {
			try {
				pdPrintCallback.invoke(parent, s);
			} catch (Exception e) {
				// Do nothing.
			}
		}
	}

	/**
	 * Delegates to parent's receiveBang method, if it exists; no-op otherwise.
	 */
	@Override
	public void receiveBang(String source) {
		if (receiveBangCallback != null) {
			try {
				receiveBangCallback.invoke(parent, source);
			} catch (Exception e) {
				// Do nothing.
			}
		}
	}

	/**
	 * Delegates to parent's receiveFloat method, if it exists; no-op otherwise.
	 */
	@Override
	public void receiveFloat(String source, float x) {
		if (receiveFloatCallback != null) {
			try {
				receiveFloatCallback.invoke(parent, source, x);
			} catch (Exception e) {
				// Do nothing.
			}
		}
	}

	/**
	 * Delegates to parent's receiveSymbol method, if it exists; no-op otherwise.
	 */
	@Override
	public void receiveSymbol(String source, String symbol) {
		if (receiveSymbolCallback != null) {
			try {
				receiveSymbolCallback.invoke(parent, source, symbol);
			} catch (Exception e) {
				// Do nothing.
			}
		}
	}

	/**
	 * Delegates to parent's receiveList method, if it exists; no-op otherwise.
	 */
	@Override
	public void receiveList(String source, Object... args) {
		if (receiveListCallback != null) {
			try {
				receiveListCallback.invoke(parent, source, args);
			} catch (Exception e) {
				// Do nothing.
			}
		}
	}

	/**
	 * Delegates to parent's receiveMessage method, if it exists; no-op otherwise.
	 */
	@Override
	public void receiveMessage(String source, String symbol, Object... args) {
		if (receiveMessageCallback != null) {
			try {
				receiveMessageCallback.invoke(parent, source, symbol, args);
			} catch (Exception e) {
				// Do nothing.
			}
		}
	}
}