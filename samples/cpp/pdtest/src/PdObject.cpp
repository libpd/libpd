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
	std::cout << message << std::endl;
}

//--------------------------------------------------------------		
void PdObject::receiveBang(const std::string &dest) {
	std::cout << "CPP: bang " << dest << std::endl;
}

void PdObject::receiveFloat(const std::string &dest, float num) {
	std::cout << "CPP: float " << dest << ": " << num << std::endl;
}

void PdObject::receiveSymbol(const std::string &dest, const std::string &symbol) {
	std::cout << "CPP: symbol " << dest << ": " << symbol << std::endl;
}

void PdObject::receiveList(const std::string &dest, const pd::List &list) {
	std::cout << "CPP: list " << dest << ": ";
	
	// step through the list
	for(int i = 0; i < list.len(); ++i) {
		if(list.isFloat(i)) {
			std::cout << list.getFloat(i);
		}
		else if(list.isSymbol(i)) {
			std::cout << list.getSymbol(i);
		}
		if(i < list.len()-1) {
			std::cout << " ";
		}
	}
	
	// you can also use the built in toString function or simply stream it out
	// std::cout << list.toString();
	// std::cout << list;
	
	// print an OSC-style type string
	std::cout << " " << list.types() << std::endl;
}

void PdObject::receiveMessage(const std::string &dest, const std::string &msg, const pd::List &list) {
	std::cout << "CPP: message " << dest << ": " << msg << " " << list.toString() << " " << list.types() << std::endl;
}

//--------------------------------------------------------------
void PdObject::receiveNoteOn(const int channel, const int pitch, const int velocity) {
	std::cout << "CPP MIDI: note on: " << channel << " " << pitch << " " << velocity << std::endl;
}

void PdObject::receiveControlChange(const int channel, const int controller, const int value) {
	std::cout << "CPP MIDI: control change: " << channel << " " << controller << " " << value << std::endl;
}

void PdObject::receiveProgramChange(const int channel, const int value) {
	std::cout << "CPP MIDI: program change: " << channel << " " << value << std::endl;
}

void PdObject::receivePitchBend(const int channel, const int value) {
	std::cout << "CPP MIDI: pitch bend: " << channel << " " << value << std::endl;
}

void PdObject::receiveAftertouch(const int channel, const int value) {
	std::cout << "CPP MIDI: aftertouch: " << channel << " " << value << std::endl;
}

void PdObject::receivePolyAftertouch(const int channel, const int pitch, const int value) {
	std::cout << "CPP MIDI: poly aftertouch: " << channel << " " << pitch << " " << value << std::endl;
}

void PdObject::receiveMidiByte(const int port, const int byte) {
	std::cout << "CPP MIDI: midi byte: " << port << " " << byte << std::endl;
}
