/*
 * Copyright (c) 2012 Dan Wilcox <danomatika@gmail.com>
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

/// a pd midi receiver base class
class PdMidiReceiver {

    public:

        /// midi
        virtual void receiveNoteOn(const int channel, const int pitch, const int velocity) {}
        virtual void receiveControlChange(const int channel, const int controller, const int value) {}
        virtual void receiveProgramChange(const int channel, const int value) {} // note: pgm value is 1-128
        virtual void receivePitchBend(const int channel, const int value) {}
        virtual void receiveAftertouch(const int channel, const int value) {}
        virtual void receivePolyAftertouch(const int channel, const int pitch, const int value) {}

        /// raw midi byte
        virtual void receiveMidiByte(const int port, const int byte) {}
};

} // namespace
