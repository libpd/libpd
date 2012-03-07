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

#include <string>
#include <vector>

namespace pd {

/// \section Pd Patch

/// a pd patch
///
/// if you use the copy constructor/operator, keep in mind the libpd void* pointer
/// patch handle is copied and problems can arise if one object is used to close
/// a patch that other copies may be referring to 
class Patch {

	public:

		Patch();
		Patch(const std::string& filename, const std::string& path);
		Patch(void* handle, int dollarZero, const std::string& filename, const std::string& path);
			
		/// data access
		void* handle()				const	{return _handle;}
		int dollarZero()			const	{return _dollarZero;}
		std::string filename()		const	{return _filename;}
		std::string path()			const	{return _path;}
		
		/// get dollarZero as a string
		std::string dollarZeroStr()	const	{return _dollarZeroStr;}
			
		/// is the patch pointer valid?
		bool isValid() const;
		
		/// clear patch pointer and dollar zero (does not close patch!)
		///
		/// note: does not clear filename and path so the object can be reused
		//        for opening multiple instances
		void clear();

		/// copy constructor
		Patch(const Patch& from);

        /// copy operator
        void operator=(const Patch& from);
		
		/// print to ostream
		friend std::ostream& operator<<(std::ostream& os, const Patch& from);

	private:
	
		void* _handle;				///< patch handle pointer
		int _dollarZero;			///< the unique patch id, ie $0
		std::string _dollarZeroStr;	///< $0 as a string
		
		std::string _filename;	///< filename
		std::string _path;		///< full path
};

/// \section Pd stream interface message objects

/// bang event
struct Bang {

	const std::string dest; ///< dest receiver name
	
	explicit Bang(const std::string& dest) : dest(dest) {}
};

/// float value
struct Float : public Bang {

	const float value;
	
	explicit Float(const std::string& dest, const float value) :
		Bang(dest), value(value) {}
};

/// symbol value
struct Symbol : public Bang {

	const std::string symbol;

	explicit Symbol(const std::string& dest, const std::string& symbol) :
		Bang(dest), symbol(symbol) {}
};

/// list, a compound message
class List : public Bang {

	public:

        List() : Bang("") {}
		List(const std::string& dest) : Bang(dest) {}
	
		/// \section Read

		/// check type
		bool isFloat(const unsigned int index) const;
		bool isSymbol(const unsigned int index) const;

		/// get item as type
		float getFloat(const unsigned int index) const;
		std::string getSymbol(const unsigned int index) const;

		/// \section Write
		
        /// add elements to the list
        ///
        /// List list;
        /// list.addSymbol("hello");
        /// list.addFloat(1.23);
		///
        void addFloat(const float value);
		void addSymbol(const std::string& symbol);
        
        /// \section Write Stream Interface
        
        /// list << "hello" << 1.23;
        
        /// add a float to the message
        List& operator<<(const bool var);
        List& operator<<(const int var);
        List& operator<<(const float var);
        List& operator<<(const double var);
        
        /// add a symbol to the message
        List& operator<<(const char var);
        List& operator<<(const char* var);
        List& operator<<(const std::string& var);
		
        /// \section Util
		
		const unsigned int len() const;		///< number of items
		const std::string& types() const;	///< OSC style type string ie "fsfs"
		void clear(); 						///< clear all objects

		/// get list as a string
		std::string toString() const;

		/// print to ostream
		friend std::ostream& operator<<(std::ostream& os, const List& from);

	private:

		std::string typeString;	///< OSC style type string
		
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
		
		std::vector<MsgObject> objects;	///< list objects
};

/// start a compound message
struct StartMessage {
    explicit
        StartMessage() {}
};

/// finish a compound message as a list
struct FinishList : public Bang {
    explicit
		FinishList(const std::string& dest) : Bang(dest) {}
};

/// finish a compound message as a typed message
struct FinishMessage : public Bang {

	const std::string msg; ///< target msg at the dest

    explicit
		FinishMessage(const std::string& dest, const std::string& msg) :
			Bang(dest), msg(msg) {}
};

/// /section Pd stream interface midi objects
/// ref: http://www.gweep.net/~prefect/eng/reference/protocol/midispec.html

/// send a note on event (set vel = 0 for noteoff)
struct NoteOn {

	const int channel;	///< channel (0 - 15 * dev#)
	const int pitch;	///< pitch (0 - 127)
	const int velocity;	///< velocity (0 - 127)
	
	explicit NoteOn(const int channel, const int pitch, const int velocity=64) :
		channel(channel), pitch(pitch), velocity(velocity) {}
};

/// change a control value aka send a CC message
struct ControlChange {

	const int channel;		///< channel (0 - 15 * dev#)
	const int controller;	///< controller (0 - 127)
	const int value;		///< value (0 - 127)
	
	explicit ControlChange(const int channel, const int controller, const int value) :
		channel(channel), controller(controller), value(value) {}
};

/// change a program value (ie an instrument)
struct ProgramChange {

	const int channel;	///< channel (0 - 15 * dev#)
	const int value;	///< value (0 - 127)
	
	explicit ProgramChange(const int channel, const int value) :
		channel(channel), value(value) {}
};

/// change the pitch bend value
struct PitchBend {

	const int channel;	///< channel (0 - 15 * dev#)
	const int value;	///< value (-8192 - 8192)
	
	explicit PitchBend(const int channel, const int value) :
		channel(channel), value(value) {}
};

/// change an aftertouch value
struct Aftertouch {

	const int channel;	///< channel (0 - 15 * dev#)
	const int value;	///< value (0 - 127)
	
	explicit Aftertouch(const int channel, const int value) :
		channel(channel), value(value) {}
};

/// change a poly aftertouch value
struct PolyAftertouch {

	const int channel;	///< channel (0 - 15 * dev#)
	const int pitch;	///< controller (0 - 127)
	const int value;	///< value (0 - 127)
	
	explicit PolyAftertouch(const int channel, const int pitch, const int value) :
		channel(channel), pitch(pitch), value(value) {}
};

/// start a raw midi byte stream
struct StartMidi {

	const int port; 	///< raw portmidi port, see http://en.wikipedia.org/wiki/PortMidi
	
	explicit StartMidi(const int port=0) : port(port) {}
};

/// start a raw sysex byte stream
struct StartSysex {

	const int port; 	///< raw portmidi port
	
	explicit StartSysex(const int port=0) : port(port) {}
};

/// start a sys realtime byte stream
struct StartSysRealTime {

	const int port; 	///< raw portmidi port
	
	explicit StartSysRealTime(const int port=0) : port(port) {}
};

/// finish a midi byte stream
struct Finish {
	explicit Finish() {}
};

} // namespace
