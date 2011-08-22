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
 * Abstract base class for using libpd with Processing.
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

	public PureDataP5Base(Object parent) {
		this.parent = parent;
		Map<String, Method> methods = extractMethods(parent.getClass());
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
		Method[] methods = clazz.getDeclaredMethods();
		for (Method method: methods) {
			result.put(method.getName(), method);
		}
		return result;
	}
	
	public void dispose() {
		PdBase.release();
	}
	
	public int subscribe(String sym) {
		return PdBase.subscribe(sym);
	}
	
	public void unsubscribe(String sym) {
		PdBase.unsubscribe(sym);
	}

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

	public int openPatch(File file) {
		try {
			return PdBase.openPatch(file);
		} catch (IOException e) {
			throw new RuntimeException(e);
		}
	}

	public void closePatch(int handle) {
		PdBase.closePatch(handle);
	}

	public int arraySize(String name) {
		return PdBase.arraySize(name);
	}
	
	public int readArray(float[] to, int toOffset, String fromArray, int fromOffset, int n) {
		return PdBase.readArray(to, toOffset, fromArray, fromOffset, n);
	}
	
	public int writeArray(String toArray, int toOffset, float[] fromArray, int fromOffset, int n) {
		return PdBase.writeArray(toArray, toOffset, fromArray, fromOffset, n);
	}
	
	public void sendBang(String recv) {
		PdBase.sendBang(recv);
	}

	public void sendFloat(String recv, float x) {
		PdBase.sendFloat(recv, x);
	}

	public void sendSymbol(String recv, String sym) {
		PdBase.sendSymbol(recv, sym);
	}

	public void sendList(String recv, Object... args) {
		PdBase.sendList(recv, args);
	}

	public void sendMessage(String recv, String mesg, Object... args) {
		PdBase.sendMessage(recv, mesg, args);
	}
}