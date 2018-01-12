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

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

namespace pd {

/// \section Pd Patch

/// a pd patch
///
/// if you use the copy constructor/operator, keep in mind the libpd void*
/// pointer patch handle is copied and problems can arise if one object is used
/// to close a patch that other copies may be referring to
class Patch {

    public:

        Patch() :
            _handle(NULL), _dollarZero(0), _dollarZeroStr("0"),
            _filename(""), _path("") {}

        Patch(const std::string& filename, const std::string& path) :
            _handle(NULL), _dollarZero(0), _dollarZeroStr("0"),
            _filename(filename), _path(path) {}
        
        Patch(void* handle, int dollarZero,
              const std::string& filename,
              const std::string& path) :
            _handle(handle), _dollarZero(dollarZero),
            _filename(filename), _path(path) {
                std::stringstream itoa;
                itoa << dollarZero;
                _dollarZeroStr = itoa.str();
            }

        /// get the raw pointer to the patch instance
        void* handle() const {return _handle;}

        /// get the unqiue instance $0 ID
        int dollarZero() const {return _dollarZero;}

        /// get the patch filename
        std::string filename() const {return _filename;}

        /// get the parent path for the file
        std::string path() const {return _path;}

        /// get the unique instance $0 ID as a string
        std::string dollarZeroStr() const {return _dollarZeroStr;}

        /// is the patch pointer valid?
        bool isValid() const {return _handle != NULL;}

        /// clear patch pointer and dollar zero (does not close patch!)
        ///
        /// note: does not clear filename and path so the object can be reused
        //        for opening multiple instances
        void clear()  {
            _handle = NULL;
            _dollarZero = 0;
            _dollarZeroStr = "0";
        }

        /// copy constructor
        Patch(const Patch& from) {
            _handle = from._handle;
            _dollarZero = from._dollarZero;
            _dollarZeroStr = from._dollarZeroStr;
            _filename = from._filename;
            _path = from._path;
        }

        /// copy operator
        void operator=(const Patch& from) {
            _handle = from._handle;
            _dollarZero = from._dollarZero;
            _dollarZeroStr = from._dollarZeroStr;
            _filename = from._filename;
            _path = from._path;
        }

        /// print info to ostream
        friend std::ostream& operator<<(std::ostream& os, const Patch& from) {
            return os << "Patch: \"" << from.filename() << "\" $0: "
                      << from.dollarZeroStr() << " valid: " << from.isValid();
        }

    private:

        void* _handle;              //< patch handle pointer
        int _dollarZero;            //< the unique $0 patch ID
        std::string _dollarZeroStr; //< $0 as a string

        std::string _filename;      //< filename
        std::string _path;          //< full path to parent folder
};

/// \section Pd stream interface message objects

/// bang event
struct Bang {

    const std::string dest; //< dest receiver name

    explicit Bang(const std::string& dest) : dest(dest) {}
};

/// float value
struct Float {

    const std::string dest; //< dest receiver name
    const float num;        //< the float value

    Float(const std::string& dest, const float num) :
        dest(dest), num(num) {}
};

/// symbol value
struct Symbol {

    const std::string dest;   //< dest receiver name
    const std::string symbol; //< the symbol value

    Symbol(const std::string& dest, const std::string& symbol) :
        dest(dest), symbol(symbol) {}
};

/// a compound message containing floats and symbols
class List {

    public:

        List() {}

    /// \section Read

        /// check if index is a float type
        bool isFloat(const unsigned int index) const {
            if(index < objects.size())
                if(objects[index].type == List::FLOAT)
                    return true;
            return false;
        }

        /// check if index is a symbol type
        bool isSymbol(const unsigned int index) const {
            if(index < objects.size())
                if(objects[index].type == List::SYMBOL)
                    return true;
            return false;
        }

        /// get index as a float
        float getFloat(const unsigned int index) const {
            if(!isFloat(index)) {
                std::cerr << "Pd: List: object " << index
                          << " is not a float"   << std::endl;
                return 0;
            }
            return objects[index].value;
        }

        /// get index as a symbol
        std::string getSymbol(const unsigned int index) const {
            if(!isSymbol(index)) {
                std::cerr << "Pd: List: object " << index
                          << " is not a symbol"  << std::endl;
                return "";
            }
            return objects[index].symbol;
        }

    /// \section Write
    ///
    /// add elements to the list
    ///
    /// List list;
    /// list.addSymbol("hello");
    /// list.addFloat(1.23);
    ///

        /// add a float to the list
        void addFloat(const float num) {
            MsgObject o;
            o.type = List::FLOAT;
            o.value = num;
            objects.push_back(o);
            typeString += 'f';
        }

        /// add a symbol to the list
        void addSymbol(const std::string& symbol) {
            MsgObject o;
            o.type = List::SYMBOL;
            o.symbol = symbol;
            objects.push_back(o);
            typeString += 's';
        }

    /// \section Write Stream Interface
    ///
    /// list << "hello" << 1.23;
    ///

        /// add a float to the message
        List& operator<<(const bool var) {
            addFloat((float) var);
            return *this;
        }

        /// add a float to the message
        List& operator<<(const int var) {
            addFloat((float) var);
            return *this;
        }

        /// add a float to the message
        List& operator<<(const float var) {
            addFloat((float) var);
            return *this;
        }

        /// add a float to the message
        List& operator<<(const double var) {
            addFloat((float) var);
            return *this;
        }

        /// add a symbol to the message
        List& operator<<(const char var) {
            std::string s;
            s = var;
            addSymbol(s);
            return *this;
        }

        /// add a symbol to the message
        List& operator<<(const char* var) {
            addSymbol((std::string) var);
            return *this;
        }

        /// add a symbol to the message
        List& operator<<(const std::string& var) {
            addSymbol((std::string) var);
            return *this;
        }

    /// \section Util

        /// return number of items
        const unsigned int len() const {return (unsigned int) objects.size();}

        /// return OSC style type string ie "fsfs"
        const std::string& types() const {return typeString;}

        /// clear all objects
        void clear() {
            typeString = "";
            objects.clear();
        }

        /// get list as a string
        std::string toString() const {
            std::string line;
            std::stringstream itoa;
            for(int i = 0; i < (int)objects.size(); ++i) {
                if(isFloat(i)) {
                    itoa << getFloat(i);
                    line += itoa.str();
                    itoa.str("");
                }
                else
                    line += getSymbol(i);
                line += " ";
            }
            return line;
        }

        /// print to ostream
        friend std::ostream& operator<<(std::ostream& os, const List& from) {
            return os << from.toString();
        }

    private:

        std::string typeString; //< OSC style type string

        // object type
        enum MsgType {
            FLOAT,
            SYMBOL
        };

        // object wrapper
        struct MsgObject {
            MsgType type;
            float value;
            std::string symbol;
        };

        std::vector<MsgObject> objects; //< list objects
};

/// start a compound message
struct StartMessage {
    explicit StartMessage() {}
};

/// finish a compound message as a list
struct FinishList {

    const std::string dest; //< dest receiver name

    explicit FinishList(const std::string& dest) : dest(dest) {}
};

/// finish a compound message as a typed message
struct FinishMessage {

    const std::string dest; //< dest receiver name
    const std::string msg;  //< target msg at the dest

    FinishMessage(const std::string& dest, const std::string& msg) :
            dest(dest), msg(msg) {}
};

/// \section Pd stream interface midi objects
/// ref: http://www.gweep.net/~prefect/eng/reference/protocol/midispec.html

/// send a note on event (set vel = 0 for noteoff)
struct NoteOn {

    const int channel;  //< channel (0 - 15 * dev#)
    const int pitch;    //< pitch (0 - 127)
    const int velocity; //< velocity (0 - 127)

    NoteOn(const int channel, const int pitch, const int velocity=64) :
        channel(channel), pitch(pitch), velocity(velocity) {}
};

/// change a control value aka send a CC message
struct ControlChange {

    const int channel;    //< channel (0 - 15 * dev#)
    const int controller; //< controller (0 - 127)
    const int value;      //< value (0 - 127)

    ControlChange(const int channel, const int controller, const int value) :
        channel(channel), controller(controller), value(value) {}
};

/// change a program value (ie an instrument)
struct ProgramChange {

    const int channel; //< channel (0 - 15 * dev#)
    const int value;   //< value (0 - 127)

    ProgramChange(const int channel, const int value) :
        channel(channel), value(value) {}
};

/// change the pitch bend value
struct PitchBend {

    const int channel; //< channel (0 - 15 * dev#)
    const int value;   //< value (-8192 - 8192)

    PitchBend(const int channel, const int value) :
        channel(channel), value(value) {}
};

/// change an aftertouch value
struct Aftertouch {

    const int channel; //< channel (0 - 15 * dev#)
    const int value;   //< value (0 - 127)

    Aftertouch(const int channel, const int value) :
        channel(channel), value(value) {}
};

/// change a poly aftertouch value
struct PolyAftertouch {

    const int channel; //< channel (0 - 15 * dev#)
    const int pitch;   //< pitch (0 - 127)
    const int value;   //< value (0 - 127)

    PolyAftertouch(const int channel, const int pitch, const int value) :
        channel(channel), pitch(pitch), value(value) {}
};

/// a raw midi byte
struct MidiByte {

    const int port; //< raw portmidi port
                    //< see http://en.wikipedia.org/wiki/PortMidi
    const unsigned char byte; //< the raw midi byte value

    MidiByte(const int port, unsigned char byte) : port(port), byte(byte) {}
};

/// start a raw midi byte stream
struct StartMidi {

    const int port; //< raw portmidi port

    explicit StartMidi(const int port=0) : port(port) {}
};

/// start a raw sysex byte stream
struct StartSysex {

    const int port; //< raw portmidi port

    explicit StartSysex(const int port=0) : port(port) {}
};

/// start a sys realtime byte stream
struct StartSysRealTime {

    const int port; //< raw portmidi port

    explicit StartSysRealTime(const int port=0) : port(port) {}
};

/// finish a midi byte stream
struct Finish {
    explicit Finish() {}
};

} // namespace
