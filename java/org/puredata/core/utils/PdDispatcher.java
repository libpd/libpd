/**
 * 
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL WARRANTIES, see the
 * file, "LICENSE.txt," in this distribution.
 * 
 */

package org.puredata.core.utils;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.puredata.core.PdBase;
import org.puredata.core.PdListener;
import org.puredata.core.PdReceiver;

/**
 * 
 * PdDispatcher is an implementation of {@link PdReceiver} that dispatches messages from Pd to
 * instances of {@link PdListener} based on the Pd symbol they originate from. Instances of this
 * class automatically handle subscriptions to Pd symbols.
 * 
 * @author Peter Brinkmann (peter.brinkmann@gmail.com)
 * 
 */
public abstract class PdDispatcher implements PdReceiver {

  private final Map<String, Set<PdListener>> listeners = new HashMap<String, Set<PdListener>>();

  /**
   * Register a listener for Pd messages to the given symbol; subscribe to the symbol if this is the
   * first listener for this symbol
   * 
   * @param symbol receiver symbol in Pd
   * @param listener
   */
  public synchronized void addListener(String symbol, PdListener listener) {
    Set<PdListener> selected = listeners.get(symbol);
    if (selected == null) {
      int err = PdBase.subscribe(symbol);
      if (err != 0) {
        throw new IllegalArgumentException("bad symbol: " + symbol);
      }
      selected = new HashSet<PdListener>();
      listeners.put(symbol, selected);
    }
    selected.add(listener);
  }

  /**
   * Remove a listener associated with a symbol; unsubscribe from Pd messages to symbol if no
   * listeners are left
   * 
   * @param symbol receiver symbol in Pd
   * @param listener
   */
  public synchronized void removeListener(String symbol, PdListener listener) {
    Set<PdListener> selected = listeners.get(symbol);
    if (selected != null) {
      selected.remove(listener);
      if (selected.isEmpty()) {
        PdBase.unsubscribe(symbol);
        listeners.remove(symbol);
      }
    }
  }

  /**
   * Unsubscribe from Pd messages and release all resources
   */
  public synchronized void release() {
    for (String symbol : listeners.keySet()) {
      PdBase.unsubscribe(symbol);
    }
    listeners.clear();
  }

  @Override
  protected void finalize() throws Throwable {
    release();
    super.finalize();
  }

  /**
   * Handle print messages from Pd
   */
  @Override
  public abstract void print(String s);

  @Override
  public synchronized void receiveBang(String source) {
    Set<PdListener> selected = listeners.get(source);
    if (selected != null) {
      for (PdListener listener : selected) {
        listener.receiveBang(source);
      }
    }
  }

  @Override
  public synchronized void receiveFloat(String source, float x) {
    Set<PdListener> selected = listeners.get(source);
    if (selected != null) {
      for (PdListener listener : selected) {
        listener.receiveFloat(source, x);
      }
    }
  }

  @Override
  public synchronized void receiveSymbol(String source, String symbol) {
    Set<PdListener> selected = listeners.get(source);
    if (selected != null) {
      for (PdListener listener : selected) {
        listener.receiveSymbol(source, symbol);
      }
    }
  }

  @Override
  public synchronized void receiveList(String source, Object... args) {
    Set<PdListener> selected = listeners.get(source);
    if (selected != null) {
      for (PdListener listener : selected) {
        listener.receiveList(source, args);
      }
    }
  }

  @Override
  public synchronized void receiveMessage(String source, String symbol, Object... args) {
    Set<PdListener> selected = listeners.get(source);
    if (selected != null) {
      for (PdListener listener : selected) {
        listener.receiveMessage(source, symbol, args);
      }
    }
  }
}
