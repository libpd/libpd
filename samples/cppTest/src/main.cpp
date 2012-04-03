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
#include <iostream>
#include "PdObject.h"

using namespace std;
using namespace pd;

void testEventPolling(PdBase& pd);

int main(int argc, char **argv) {

	// our pd engine
	PdBase pd;
	
	// one input channel, two output channels
	// block size 64, one tick per buffer
	float inbuf[64], outbuf[128];
	
	// receives messages and midi
	PdObject pdObject;

	// init pd
	int srate = 44100;
	if(!pd.init(1, 2, srate)) {
		cerr << "Could not init pd" << endl;
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
	pd.addToSearchPath("pd");

	// audio processing on
	pd.computeAudio(true);

	
	cout << endl << "BEGIN Patch Test" << endl;
	
	// open patch
	Patch patch = pd.openPatch("test.pd", ".");
	cout << patch << endl;
	
	// close patch
	pd.closePatch(patch);
	cout << patch << endl;
	
	// open patch again
	patch = pd.openPatch(patch);
	cout << patch << endl;
	
	cout << "FINISH Patch Test" << endl;
	
	
	cout << endl << "BEGIN Message Test" << endl;
	
	// test basic atoms
	pd.sendBang("fromCPP");
	pd.sendFloat("fromCPP", 100);
	pd.sendSymbol("fromCPP", "test string");
    
    // stream interface
    pd << Bang("fromCPP")
       << Float("fromCPP", 100)
       << Symbol("fromCPP", "test string");
	
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
    List testList;
    testList.addFloat(1.23);
    testList.addSymbol("sent from a List object");
    pd.sendList("fromCPP", testList);
    pd.sendMessage("fromCPP", "msg", testList);
    
    // stream interface for list
    pd << StartMessage() << 1.23 << "sent from a streamed list" << FinishList("fromCPP");
    
	cout << "FINISH Message Test" << endl;
	
	
	cout << endl << "BEGIN MIDI Test" << endl;
	
	// send functions
	pd.sendNoteOn(midiChan, 60);
	pd.sendControlChange(midiChan, 0, 64);
	pd.sendProgramChange(midiChan, 100);   // note: pgm num range is 1 - 128
	pd.sendPitchBend(midiChan, 2000);   // note: libpd uses -8192 - 8192 while [bendin] returns 0 - 16383,
                                        // so sending a val of 2000 gives 10192 in pd
	pd.sendAftertouch(midiChan, 100);
	pd.sendPolyAftertouch(midiChan, 64, 100);
	pd.sendMidiByte(0, 239);    // note: pd adds +2 to the port number from [midiin], [sysexin], & [realtimein]
	pd.sendSysex(0, 239);       // so sending to port 0 gives port 2 in pd
	pd.sendSysRealTime(0, 239);
	
	// stream
	pd << NoteOn(midiChan, 60) << ControlChange(midiChan, 100, 64)
       << ProgramChange(midiChan, 100) << PitchBend(midiChan, 2000)
       << Aftertouch(midiChan, 100) << PolyAftertouch(midiChan, 64, 100)
	   << StartMidi(0) << 239 << Finish()
	   << StartSysex(0) << 239 << Finish()
	   << StartSysRealTime(0) << 239 << Finish();
    
	cout << "FINISH MIDI Test" << endl;
	
	
	cout << endl << "BEGIN Array Test" << endl;
	
	// array check length
	cout << "array1 len: " << pd.arraySize("array1") << endl;
	
	// read array
	std::vector<float> array1;
	pd.readArray("array1", array1);	// sets array to correct size
	cout << "array1 ";
	for(int i = 0; i < array1.size(); ++i)
		cout << array1[i] << " ";
	cout << endl;
	
	// write array
	for(int i = 0; i < array1.size(); ++i)
		array1[i] = i;
	pd.writeArray("array1", array1);
	
	// ready array
	pd.readArray("array1", array1);
	cout << "array1 ";
	for(int i = 0; i < array1.size(); ++i)
		cout << array1[i] << " ";
	cout << endl;
	
	// clear array
	pd.clearArray("array1", 10);
	
	// ready array
	pd.readArray("array1", array1);
	cout << "array1 ";
	for(int i = 0; i < array1.size(); ++i)
		cout << array1[i] << " ";
	cout << endl;

	cout << "FINISH Array Test" << endl;

	
	cout << endl << "BEGIN PD Test" << endl;
	pd.sendSymbol("fromCPP", "test");
	cout << "FINISH PD Test" << endl << endl;
	
	
	cout << endl << "BEGIN Event Polling Test" << endl;
	
	// disable receivers, enable polling
	pd.setReceiver(NULL);
    pd.setMidiReceiver(NULL);
	
	pd.sendSymbol("fromCPP", "test");
	testEventPolling(pd);
	
	// reenable receivers, disable polling
	pd.setReceiver(&pdObject);
    pd.setMidiReceiver(&pdObject);
	
	cout << "FINISH Event Polling Test" << endl << endl;
	
	
	// play a tone by sending a list
	// [list tone pitch 72 (
	pd.startMessage();
		pd.addSymbol("pitch");
		pd.addFloat(72);
	pd.finishList("tone");
	pd.sendBang("tone");
	
	
	// now run pd for ten seconds (logical time)
	for(int i = 0; i < 10 * srate / 64; i++) {
		// fill inbuf here
		pd.processFloat(1, inbuf, outbuf);
		// use outbuf here
	}
	
	// be nice and clean up on exit
	pd.closePatch(patch);
	pd.computeAudio(false);
	
  return 0;
}

void testEventPolling(PdBase& pd) {
	
	cout << "Number of waiting messages: " << pd.numMessages() << endl;
	
	while(pd.numMessages() > 0) {
		Message& msg = pd.nextMessage();

		switch(msg.type) {
			
			case PRINT:
				cout << "CPP: " << msg.symbol << endl;
				break;
			
			// events
			case BANG:
				cout << "CPP: bang " << msg.dest << endl;
				break;
			case FLOAT:
				cout << "CPP: float " << msg.dest << ": " << msg.num << endl;
				break;
			case SYMBOL:
				cout << "CPP: symbol " << msg.dest << ": " << msg.symbol << endl;
				break;
			case LIST:
				cout << "CPP: list " << msg.list << msg.list.types() << endl;
				break;
			case MESSAGE:
				cout << "CPP: message " << msg.dest << ": " << msg.symbol << " " 
					 << msg.list << msg.list.types() << endl;
				break;
			
			// midi
			case NOTE_ON:
				cout << "CPP MIDI: note on: " << msg.channel << " "
					 << msg.pitch << " " << msg.velocity << endl;
				break;
			case CONTROL_CHANGE:
				cout << "CPP MIDI: control change: " << msg.channel
					 << " " << msg.controller << " " << msg.value << endl;
				break;
			case PROGRAM_CHANGE:
				cout << "CPP MIDI: program change: " << msg.channel << " "
					 << msg.value << endl;
				break;
			case PITCH_BEND:
				cout << "CPP MIDI: pitch bend: " << msg.channel << " "
					 << msg.value << endl;
				break;
			case AFTERTOUCH:
				cout << "CPP MIDI: aftertouch: " << msg.channel << " "
					 << msg.value << endl;
				break;
			case POLY_AFTERTOUCH:
				cout << "CPP MIDI: poly aftertouch: " << msg.channel << " "
					 << msg.pitch << " " << msg.value << endl;
				break;
			case BYTE:
				cout << "CPP MIDI: midi byte: " << msg.port << " 0x"
					 << hex << (int) msg.byte << dec << endl;
				break;
		
			case NONE:
				cout << "CPP: NONE ... empty message" << endl;
				break;
		}
	}
}
