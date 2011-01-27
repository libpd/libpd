/** 
 * 
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 */

package org.puredata.core;

/**
 * 
 * PdReceiver is an interface for receiving MIDI events from pd, to be used with setMidiReceiver in {@link PdBase}.
 * 
 * @author Peter Brinkmann (peter.brinkmann@gmail.com) 
 *
 */
public interface PdMidiReceiver {

	/**
	 * receives a note on event from pd
	 * 
	 * @param channel starting at 0
	 * @param pitch
	 * @param velocity
	 */
	void receiveNoteOn(int channel, int pitch, int velocity);

	/**
	 * receives a control change event from pd
	 * 
	 * @param channel starting at 0
	 * @param controller
	 * @param value
	 */
	void receiveControlChange(int channel, int controller, int value);

	/**
	 * receives a program event from pd
	 * 
	 * @param channel starting at 0
	 * @param value
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
	 * @param value
	 */
	void receiveAftertouch(int channel, int value);

	/**
	 * receives a polyphonic aftertouch event from pd
	 * 
	 * @param channel starting at 0
	 * @param pitch
	 * @param value
	 */
	void receivePolyAftertouch(int channel, int pitch, int value);

	/**
	 * receives one raw MIDI byte from pd
	 * 
	 * @param port
	 * @param value
	 */
	void receiveMidiByte(int port, int value);
}
