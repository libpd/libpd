/**
 * 
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL WARRANTIES, see the
 * file, "LICENSE.txt," in this distribution.
 * 
 */

package org.puredata.core;

/**
 * 
 * Interface for receiving MIDI channel messages from Pd.
 * 
 * @author Peter Brinkmann (peter.brinkmann@gmail.com)
 */
public interface PdMidiListener {

  /**
   * receives a note on event from pd
   * 
   * @param channel starting at 0
   * @param pitch 0-127
   * @param velocity 0-127
   */
  void receiveNoteOn(int channel, int pitch, int velocity);

  /**
   * receives a control change event from pd
   * 
   * @param channel starting at 0
   * @param controller 0-127
   * @param value 0-127
   */
  void receiveControlChange(int channel, int controller, int value);

  /**
   * receives a program event from pd
   * 
   * @param channel starting at 0
   * @param value 0-127
   */
  void receiveProgramChange(int channel, int value);

  /**
   * receives a pitch bend event from pd
   * 
   * @param channel starting at 0
   * @param value centered at 0; no 8192 offset
   */
  void receivePitchBend(int channel, int value);

  /**
   * receives an aftertouch event from pd
   * 
   * @param channel starting at 0
   * @param value 0-127
   */
  void receiveAftertouch(int channel, int value);

  /**
   * receives a polyphonic aftertouch event from pd
   * 
   * @param channel starting at 0
   * @param pitch 0-127
   * @param value 0-127
   */
  void receivePolyAftertouch(int channel, int pitch, int value);

  /**
   * Begin assembling subsequent MIDI messages into one buffer. This is an optional optimization
   * that allows wire format converters to reduce the number of buffers they need to send, and it
   * provides a hint that several messages are supposed to occur at the same time.
   * 
   * @return true if block mode is supported
   */
  boolean beginBlock();

  /**
   * Optionally concludes a block of buffers. If block mode is supported, this call will cause the
   * messages received since the beginBlock() call to be handled, e.g., by writing them to a USB or
   * other device.
   */
  void endBlock();
}
