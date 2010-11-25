package org.puredata.core;

public interface PdMidiReceiver {

	void receiveNoteOn(int channel, int pitch, int velocity);
	void receiveControlChange(int channel, int controller, int value);
	void receiveProgramChange(int channel, int value);
	void receivePitchBend(int channel, int value);
	void receiveAfterTouch(int channel, int value);
	void receivePolyAfterTouch(int channel, int pitch, int value);

}
