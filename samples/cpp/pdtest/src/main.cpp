/*
 * Copyright (c) 2012 Dan Wilcox <danomatika@gmail.com>
 *
 * BSD Simplified License.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * See https://github.com/libpd/libpd for documentation
 *
 * This file was adapted from the ofxPd openFrameworks addon example:
 * https://github.com/danomatika/ofxPd
 *
 */
#include <stdlib.h>
#include <iostream>
#include "PdBase.hpp"
#include "PdObject.h"

// Howdy gentle libpd user,
//
// This is just a simple test to make sure message passing in the libpd c++ layer
// is working. Like the C test, this simulates 10 seconds of logical audio time
// by calling processFloat in a for loop. Running this app will not produce any
// sound as it is not using an audio api. You need to add that yourself using
// something like PortAudio, Jack, etc as C++ does not have a default audio library.
//
int main(int argc, char **argv) {

	// our pd engine
	pd::PdBase pd;
	
	// one input channel, two output channels
	// block size 64, one tick per buffer
	float inbuf[64], outbuf[128];
	
	// custom receiver object for messages and midi
	PdObject pdObject;

	// init pd
	//
	// set 4th arg to true for queued message passing using an internal ringbuffer
	//
	// in this test, messages should return immediately when not queued otherwise
	// they should all return at once when pd is processing at the end of this
	// function
	//
	int srate = 44100;
	if(!pd.init(1, 2, srate, true)) {
		std::cerr << "Could not init pd" << std::endl;
		exit(1);
	}
    
	int midiChan = 1; // midi channels are 0-15
	
	// subscribe to receive source names
	pd.subscribe("toCPP");
	pd.subscribe("env");

	// set receivers
	pd.setReceiver(&pdObject);
	pd.setMidiReceiver(&pdObject);

	// add the data/pd folder to the search path
	pd.addToSearchPath("pd/lib");

	// audio processing on
	pd.computeAudio(true);

	
	std::cout << std::endl << "BEGIN Patch Test" << std::endl;
	
	// open patch
	pd::Patch patch = pd.openPatch("pd/test.pd", ".");
	std::cout << patch << std::endl;
	
	// close patch
	pd.closePatch(patch);
	std::cout << patch << std::endl;
	
	// open patch again
	patch = pd.openPatch(patch);
	std::cout << patch << std::endl;
	
	// process any received messages
	//
	// in a normal case (not a test like this), you would call this in
	// your application main loop
	pd.processFloat(1, inbuf, outbuf);
	pd.receiveMessages();
	
	std::cout << "FINISH Patch Test" << std::endl;
	
	
	std::cout << std::endl << "BEGIN Message Test" << std::endl;
	
	// test basic atoms
	pd.sendBang("fromCPP");
	pd.sendFloat("fromCPP", 100);
	pd.sendSymbol("fromCPP", "test string");
    	
	// stream interface
	pd << pd::Bang("fromCPP")
		<< pd::Float("fromCPP", 100)
		<< pd::Symbol("fromCPP", "test string");
	
	// send a list
	pd.startMessage();
		pd.addFloat(1.23);
		pd.addSymbol("a symbol");
	pd.finishList("fromCPP");
	
	// send a message to the $0 receiver ie $0-toOF
	pd.startMessage();
		pd.addFloat(1.23);
		pd.addSymbol("a symbol");
	pd.finishList(patch.dollarZeroStr()+"-fromCPP");
	
	// send a list using the List object
	pd::List testList;
	testList.addFloat(1.23);
	testList.addSymbol("sent from a List object");
	pd.sendList("fromCPP", testList);
	pd.sendMessage("fromCPP", "msg", testList);

	// stream interface for list
	pd << pd::StartMessage() << 1.23 << "sent from a streamed list" << pd::FinishList("fromCPP");
	
	std::cout << "FINISH Message Test" << std::endl;
	
	
	std::cout << std::endl << "BEGIN MIDI Test" << std::endl;
	
	// send functions
	pd.sendNoteOn(midiChan, 60);
	pd.sendControlChange(midiChan, 0, 64);
	pd.sendProgramChange(midiChan, 100);   // note: [pgmin] range is 1 - 128
	pd.sendPitchBend(midiChan, 2000);   // note: libpd uses -8192 - 8192 while [bendin] returns 0 - 16383,
                                        // so sending a val of 2000 gives 10192 in pd
	pd.sendAftertouch(midiChan, 100);
	pd.sendPolyAftertouch(midiChan, 64, 100);
	pd.sendMidiByte(0, 239);    // note: pd adds +2 to the port number from [midiin], [sysexin], & [realtimein]
	pd.sendSysex(0, 239);       // so sending to port 0 gives port 2 in pd
	pd.sendSysRealTime(0, 239);
	
	// stream
	pd << pd::NoteOn(midiChan, 60) << pd::ControlChange(midiChan, 100, 64)
		<< pd::ProgramChange(midiChan, 100) << pd::PitchBend(midiChan, 2000)
		<< pd::Aftertouch(midiChan, 100) << pd::PolyAftertouch(midiChan, 64, 100)
		<< pd::StartMidi(0) << 239 << pd::Finish()
		<< pd::StartSysex(0) << 239 << pd::Finish()
		<< pd::StartSysRealTime(0) << 239 << pd::Finish();
	
	std::cout << "FINISH MIDI Test" << std::endl;
	
	
	std::cout << std::endl << "BEGIN Array Test" << std::endl;
	
	// array check length
	std::cout << "array1 len: " << pd.arraySize("array1") << std::endl;
	
	// read array
	std::vector<float> array1;
	pd.readArray("array1", array1);	// sets array to correct size
	std::cout << "array1 ";
	for(int i = 0; i < array1.size(); ++i)
		std::cout << array1[i] << " ";
	std::cout << std::endl;
	
	// write array
	for(int i = 0; i < array1.size(); ++i)
		array1[i] = i;
	pd.writeArray("array1", array1);
	
	// ready array
	pd.readArray("array1", array1);
	std::cout << "array1 ";
	for(int i = 0; i < array1.size(); ++i)
		std::cout << array1[i] << " ";
	std::cout << std::endl;
	
	// clear array
	pd.clearArray("array1", 10);
	
	// ready array
	pd.readArray("array1", array1);
	std::cout << "array1 ";
	for(int i = 0; i < array1.size(); ++i)
		std::cout << array1[i] << " ";
	std::cout << std::endl;

	std::cout << "FINISH Array Test" << std::endl << std::endl;

	
	std::cout << "BEGIN PD Test" << std::endl;
	pd.sendSymbol("fromCPP", "test");
	std::cout << "FINISH PD Test" << std::endl << std::endl;
	
	
	// play a tone by sending a list
	// [list tone pitch 72 (
	pd.startMessage();
		pd.addSymbol("pitch");
		pd.addFloat(72);
	pd.finishList("tone");
	pd.sendBang("tone");
	
	// now run pd for ten seconds (logical time)
	// you should see all the messages from pd print now
	// since processFloat actually runs the pd dsp engine and the recieve
	// functions pass messages to our PdObject
	std::cout << "Processing PD" << std::endl;
	for(int i = 0; i < 10 * srate / 64; i++) {
		// fill inbuf here
		pd.processFloat(1, inbuf, outbuf);
		pd.receiveMessages();
		pd.receiveMidi();
		// use outbuf here
	}
	
	// be nice and clean up on exit
	pd.closePatch(patch);
	pd.computeAudio(false);
	
  return 0;
}
