/*
 * Copyright (c) 2017 Dan Wilcox <danomatika@gmail.com>
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
#include "PdObject.h"

#include <iostream>

using namespace std;
using namespace pd;

//--------------------------------------------------------------
void PdObject::print(const std::string& message) {
	cout << message << endl;
}

//--------------------------------------------------------------		
void PdObject::receiveBang(const std::string& dest) {
	cout << "CPP: bang " << dest << endl;
}

void PdObject::receiveFloat(const std::string& dest, float num) {
	cout << "CPP: float " << dest << ": " << num << endl;
}

void PdObject::receiveSymbol(const std::string& dest, const std::string& symbol) {
	cout << "CPP: symbol " << dest << ": " << symbol << endl;
}

void PdObject::receiveList(const std::string& dest, const List& list) {
	cout << "CPP: list " << dest << ": " << list << endl;
}

void PdObject::receiveMessage(const std::string& dest, const std::string& msg, const List& list) {
	cout << "CPP: message " << dest << ": " << msg << " " << list.toString() << list.types() << endl;
}

//--------------------------------------------------------------
void PdObject::receiveNoteOn(const int channel, const int pitch, const int velocity) {
	cout << "CPP MIDI: note on: " << channel << " " << pitch << " " << velocity << endl;
}

void PdObject::receiveControlChange(const int channel, const int controller, const int value) {
	cout << "CPP MIDI: control change: " << channel << " " << controller << " " << value << endl;
}

void PdObject::receiveProgramChange(const int channel, const int value) {
	cout << "CPP MIDI: program change: " << channel << " " << value << endl;
}

void PdObject::receivePitchBend(const int channel, const int value) {
	cout << "CPP MIDI: pitch bend: " << channel << " " << value << endl;
}

void PdObject::receiveAftertouch(const int channel, const int value) {
	cout << "CPP MIDI: aftertouch: " << channel << " " << value << endl;
}

void PdObject::receivePolyAftertouch(const int channel, const int pitch, const int value) {
	cout << "CPP MIDI: poly aftertouch: " << channel << " " << pitch << " " << value << endl;
}

void PdObject::receiveMidiByte(const int port, const int byte) {
	cout << "CPP MIDI: midi byte: " << port << " " << byte << endl;
}
