/**
 * 
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL WARRANTIES, see the
 * file, "LICENSE.txt," in this distribution.
 * 
 */

package org.puredata.core;

/**
 * 
 * Interface for receiving MIDI channel messages as well as raw MIDI bytes from Pd, to be used with
 * setMidiReceiver in {@link PdBase}.
 * 
 * @author Peter Brinkmann (peter.brinkmann@gmail.com)
 * 
 */
public interface PdMidiReceiver extends PdMidiListener {

  /**
   * receives one raw MIDI byte from pd
   * 
   * @param port
   * @param value
   */
  void receiveMidiByte(int port, int value);
}
