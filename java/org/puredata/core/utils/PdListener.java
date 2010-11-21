/**
 * 
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 * 
 */

package org.puredata.core.utils;

/**
 *
 * PdListener provides an interface and adapter class for handling dispatches from {@link PdDispatcher}.
 * 
 * @author Peter Brinkmann (peter.brinkmann@gmail.com) 
 *
 */
public interface PdListener {
	
	public void receiveBang();
	
	public void receiveFloat(float x);
	
	public void receiveSymbol(String symbol);
	
	/**
	 * @param args  elements may be of type Integer, Float, or String
	 */
	public void receiveList(Object... args);
	
	/**
	 * @param symbol
	 * @param args  elements may be of type Integer, Float, or String
	 */
	public void receiveMessage(String symbol, Object... args);
	
	/**
	 * Adapter for PdListener implementations that only need to handle a subset of Pd messages
	 */
	public class Adapter implements PdListener {
		@Override public void receiveBang() {}
		@Override public void receiveFloat(float x) {}
		@Override public void receiveSymbol(String symbol) {}
		@Override public void receiveList(Object... args) {}
		@Override public void receiveMessage(String symbol, Object... args) {}
	}
}
