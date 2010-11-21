/**
 * 
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 */

package org.puredata.core;

/**
 * 
 * PdReceiver is an interface for printing and receiving messages from Pd, to be used with setReceiver in {@link PdBase}.
 * 
 * @author Peter Brinkmann (peter.brinkmann@gmail.com) 
 *
 */
public interface PdReceiver {
	
	/**
	 * Print output from Pd print objects
	 * 
	 * @param s  String to be printed
	 */
	public void print(String s);
	
	/**
	 * Receive bang from Pd
	 * 
	 * @param source  symbol to which the bang was sent
	 */
	public void receiveBang(String source);
	
	/**
	 * Receive float from Pd
	 * 
	 * @param source  symbol to which the float was sent
	 * @param x  float value
	 */
	public void receiveFloat(String source, float x);
	
	public void receiveSymbol(String source, String symbol);
	
	/**
	 * Receive a list from Pd
	 * 
	 * @param source  symbol to which the list was sent
	 * @param args  elements may be of type Integer, Float, or String
	 */
	public void receiveList(String source, Object... args);
	
	/**
	 * Receive a typed message from Pd; e.g., [;foo bar a b c( corresponds to the call receiveMessage("foo", "bar", { "a", "b", "c"});
	 * 
	 * @param source  symbol to which the typed message was sent
	 * @param symbol
	 * @param args  elements may be of type Integer, Float, or String
	 */
	public void receiveMessage(String source, String symbol, Object... args);
	
	/**
	 * Adapter for PdReceiver implementations that only need to handle a subset of Pd messages
	 */
	public static class Adapter implements PdReceiver {
		@Override public void print(String s) {}
		@Override public void receiveBang(String source) {}
		@Override public void receiveFloat(String source, float x) {}
		@Override public void receiveList(String source, Object... args) {}
		@Override public void receiveMessage(String source, String symbol, Object... args) {}
		@Override public void receiveSymbol(String source, String symbol) {}
	}
}
