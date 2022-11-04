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
#include "PdObject.h"

#include <iostream>

//--------------------------------------------------------------
void PdObject::print(const std::string &message) {
	std::cout << "pd" << id << ": " << message << std::endl;
}

//--------------------------------------------------------------		
void PdObject::receiveBang(const std::string &dest) {
	std::cout << "pd" << id << ": bang " << dest << std::endl;
}

void PdObject::receiveFloat(const std::string &dest, float num) {
	std::cout << "pd" << id << ": float " << dest << ": " << num << std::endl;
}

void PdObject::receiveSymbol(const std::string &dest, const std::string &symbol) {
	std::cout << "pd" << id << ": symbol " << dest << ": " << symbol << std::endl;
}

void PdObject::receiveList(const std::string &dest, const pd::List &list) {
	std::cout << "pd" << id << ": list " << dest << ": " << list.toString() << " " << list.types() << std::endl;
}

void PdObject::receiveMessage(const std::string &dest, const std::string &msg, const pd::List &list) {
	std::cout << "pd" << id << ": message " << dest << ": " << msg << " " << list.toString() << " " << list.types() << std::endl;
}

//--------------------------------------------------------------
void PdObject::receiveNoteOn(const int channel, const int pitch, const int velocity) {
	std::cout << "pd" << id << " MIDI: note on: " << channel << " " << pitch << " " << velocity << std::endl;
}

void PdObject::receiveControlChange(const int channel, const int controller, const int value) {
	std::cout << "pd" << id << " MIDI: control change: " << channel << " " << controller << " " << value << std::endl;
}

void PdObject::receiveProgramChange(const int channel, const int value) {
	std::cout << "pd" << id << " MIDI: program change: " << channel << " " << value << std::endl;
}

void PdObject::receivePitchBend(const int channel, const int value) {
	std::cout << "pd" << id << " MIDI: pitch bend: " << channel << " " << value << std::endl;
}

void PdObject::receiveAftertouch(const int channel, const int value) {
	std::cout << "pd" << id << " MIDI: aftertouch: " << channel << " " << value << std::endl;
}

void PdObject::receivePolyAftertouch(const int channel, const int pitch, const int value) {
	std::cout << "pd" << id << " MIDI: poly aftertouch: " << channel << " " << pitch << " " << value << std::endl;
}

void PdObject::receiveMidiByte(const int port, const int byte) {
	std::cout << "pd" << id << " MIDI: midi byte: " << port << " " << byte << std::endl;
}
