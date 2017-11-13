/*
 * Copyright (c) 2012-2017 Dan Wilcox <danomatika@gmail.com>
 *
 * BSD Simplified License.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * See https://github.com/libpd/libpd for documentation
 *
 * This file was originally written for the ofxPd openFrameworks addon:
 * https://github.com/danomatika/ofxPd
 *
 */
#pragma once

namespace pd {

/// a pd MIDI receiver base class
class PdMidiReceiver {

    public:

        /// receive a MIDI note on
        virtual void receiveNoteOn(const int channel, const int pitch, const int velocity) {}
        
        /// receive a MIDI control change
        virtual void receiveControlChange(const int channel, const int controller, const int value) {}
        
        /// receive a MIDI program change,
        /// note: pgm value is 1-128
        virtual void receiveProgramChange(const int channel, const int value) {}
        
        /// receive a MIDI pitch bend
        virtual void receivePitchBend(const int channel, const int value) {}
        
        /// receive a MIDI aftertouch message
        virtual void receiveAftertouch(const int channel, const int value) {}
        
        /// receive a MIDI poly aftertouch message
        virtual void receivePolyAftertouch(const int channel, const int pitch, const int value) {}

        /// receive a raw MIDI byte (sysex, realtime, etc)
        virtual void receiveMidiByte(const int port, const int byte) {}
};

} // namespace
