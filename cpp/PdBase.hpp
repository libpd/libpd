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

#include <vector>
#include <deque>
#include <map>

#include "PdReceiver.hpp"
#include "PdMidiReceiver.hpp"

#ifndef HAVE_UNISTD_H
    #define HAVE_UNISTD_H
#endif

typedef struct _atom t_atom;

namespace pd {

///
///    a Pure Data instance
///
/// use this class directly or extend it and any of its virtual functions
///
///
/// note: this object is not thread safe! use your own mutexes ...
///
///          see https://github.com/danomatika/ofxPd/tree/master/src for an example
///
///    note: if you need to grab events in your main thread (aka working with a gui),
///          you may find the message polling interface useful, see nextMessage()
///
/// note: libpd currently does not support multiple states and it is
///       suggested that you use only one PdBase-derived object at a time
///
///       calls from multiple PdBase instances currently use a global context
///       kept in a singleton object, thus only one Receiver & one MidiReceiver
///       can be used within a single program
///
///       multiple context support will be added if/when it is included within
///       libpd
///
class PdBase {

    public :

        PdBase();
        virtual ~PdBase();

        /// \section Initializing Pd

        /// initialize resources and set up the audio processing
        ///
        /// set the audio latency by setting the libpd ticks per buffer:
        /// ticks per buffer * lib pd block size (always 64)
        ///
        /// ie 4 ticks per buffer * 64 = buffer len of 512
        ///
        /// the lower the number of ticks, the faster the audio processing
        /// if you experience audio dropouts (audible clicks), increase the
        /// ticks per buffer
        ///
        /// return true if setup successfully
        ///
        /// note: must be called before processing
        ///
        virtual bool init(const int numInChannels, const int numOutChannels, const int sampleRate);

        /// clear resources
        virtual void clear();

        /// \section Adding Search Paths

        /// add to the pd search path
        /// takes an absolute or relative path (in data folder)
        ///
        /// note: fails silently if path not found
        ///
        virtual void addToSearchPath(const std::string& path);

        /// clear the current pd search path
        virtual void clearSearchPath();

        /// \section Opening Patches

        /// open a patch file (aka somefile.pd) in a specified path
        /// returns a Patch object
        ///
        /// use Patch::isValid() to check if a patch was opened successfully:
        ///
        /// Patch p1 = pd.openPatch("somefile.pd", "/some/path/");
        /// if(!p1.isValid()) {
        ///     cout << "aww ... p1 couldn't be opened" << endl;
        /// }
        virtual pd::Patch openPatch(const std::string& patch, const std::string& path);

        /// open a patch file using the filename and path of an existing patch
        ///
        /// set the filename within the patch object or use a previously opened
        /// object
        ///
        /// // open an instance of "somefile.pd"
        /// Patch p2("somefile.pd", "/some/path");    // set file and path
        /// pd.openPatch(p2);
        ///
        /// // open a new instance of "somefile.pd"
        /// Patch p3 = pd.openPatch(p2);
        ///
        /// // p2 and p3 refer to 2 different instances of "somefile.pd"
        ///
        virtual pd::Patch openPatch(pd::Patch& patch);

        /// close a patch file
        /// takes only the patch's basename (filename without extension)
        virtual void closePatch(const std::string& patch);

        /// close a patch file, takes a patch object
        /// note: clears the given Patch object
        virtual void closePatch(pd::Patch& patch);

        /// \section Audio Processing

        /// main process callbacks
        ///
        /// one of these must be called for audio dsp and message computation to occur
        ///
        /// processes one pd tick, writes raw data to buffers
        ///
        /// inBuffer must be an array of the right size, never null
        /// use inBuffer = new type[0] if no input is desired
        ///
        /// outBuffer must be an array of size outBufferSize from openAudio call
        /// returns false on error
        ///
        /// note: raw does not interlace the buffers
        ///
        bool processRaw(const float* inBuffer, float* outBuffer);
        bool processShort(int ticks, const short* inBuffer, short* outBuffer);
        bool processFloat(int ticks, const float* inBuffer, float* outBuffer);
        bool processDouble(int ticks, const double* inBuffer, double* outBuffer);

        /// \section Audio Processing Control

        /// start/stop audio processing
        ///
        /// note: in general, once started, you won't need to turn off audio processing
        ///
        /// shortcut for [; pd dsp 1( & [; pd dsp 0(
        ///
        virtual void computeAudio(bool state);

        //// \section Message Receiving

        /// subscribe/unsubscribe to source names from libpd
        ///
        /// aka the pd receive name
        ///
        /// [r source]
        /// |
        ///
        virtual void subscribe(const std::string& source);
        virtual void unsubscribe(const std::string& source);
        virtual bool exists(const std::string& source); ///< is a receiver subscribed?
        virtual void unsubscribeAll(); ///< receivers will be unsubscribed from *all* sources

        /// poll for messages
        ///
        /// by default, PdBase receieves print, event, and midi messages into a FIFO
        /// queue which can be polled
        ///
        /// while(pd.numMessages() > 0) {
        ///        pd::Message& msg = pd.nextMessage(&msg);
        ///
        ///        switch(msg.type) {
        ///            case PRINT:
        ///                cout << got print: " << msg.symbol << endl;
        ///                break;
        ///            case BANG:
        ///                cout << "go a bang to " << msg.dest << endl;
        ///                break;
        ///            case NOTE_ON:
        ///                cout << "got a note on " << msg.channel
        ///                     << msg.pitch << " " << msg.velocity << endl;
        ///                break;
        ///            ...
        ///        }
        ///    }
        ///
        /// if you set a PdReceiver callback receiver, then event messages will
        /// not be added to the queue
        ///
        /// the same goes for setting a PdMidiReceiver regarding midi messages
        ///
        /// if the message queue is full, the oldest message will be dropped
        /// see setMaxQueueLen()

        /// returns the number of waiting messages in the queue
        int numMessages();

        /// get the current waiting message
        ///
        /// copies current message into given message object
        ///
        /// returns true if message was copied, returns false if no message
        pd::Message& nextMessage();

        /// clear currently waiting messages
        void clearMessages();

        /// \section Event Receiving via Callbacks

        /// set the incoming event receiver, disables the event queue
        ///
        /// automatically receives from all currently subscribed sources
        ///
        /// set this to NULL to disable callback receiving and reenable the
        /// event queue
        ///
        void setReceiver(pd::PdReceiver* receiver);

        /// \section Midi Receiving via Callbacks

        /// set the incoming midi event receiver, disables the midi queue
        ///
        /// automatically receives from all midi channels
        ///
        /// set this to NULL to disable midi events and reenable the midi queue
        ///
        void setMidiReceiver(pd::PdMidiReceiver* midiReceiver);

        /// \section Sending Functions

        /// messages
        virtual void sendBang(const std::string& dest);
        virtual void sendFloat(const std::string& dest, float num);
        virtual void sendSymbol(const std::string& dest, const std::string& symbol);

        /// compound messages
        ///
        /// pd.startMessage();
        /// pd.addSymbol("hello");
        /// pd.addFloat(1.23);
        /// pd.finishList("test");  // "test" is the reciever name in pd
        ///
        /// sends [list hello 1.23( -> [r test],
        /// you will need to use the [list trim] object on the reciving end
        ///
        /// finishMsg sends a typed message -> [; test msg1 hello 1.23(
        ///
        /// pd.startMessage();
        /// pd.addSymbol("hello");
        /// pd.addFloat(1.23);
        /// pd.finishMessage("test", "msg1");
        ///
        virtual void startMessage();
        virtual void addFloat(const float num);
        virtual void addSymbol(const std::string& symbol);
        virtual void finishList(const std::string& dest);
        virtual void finishMessage(const std::string& dest, const std::string& msg);

        /// compound messages using the PdBase List type
        ///
        /// List list;
        /// list.addSymbol("hello");
        /// list.addFloat(1.23);
        /// pd.sendList("test", list);
        ///
        /// sends [list hello 1.23( -> [r test]
        ///
        /// clear the list:
        ///
        /// list.clear();
        ///
        /// stream operators work as well:
        ///
        /// list << "hello" << 1.23;
        /// pd.sendMessage("test", "msg1", list);
        ///
        /// sends a typed message -> [; test msg1 hello 1.23(
        ///
        virtual void sendList(const std::string& dest, const pd::List& list);
        virtual void sendMessage(const std::string& dest, const std::string& msg, const pd::List& list = pd::List());

        /// midi
        ///
        /// send midi messages, any out of range messages will be silently ignored
        ///
        /// number ranges:
        /// channel             0 - 15 * dev# (dev #0: 0-15, dev #1: 16-31, etc)
        /// pitch               0 - 127
        /// velocity            0 - 127
        /// controller value    0 - 127
        /// program value       1 - 128
        /// bend value          -8192 - 8191
        /// touch value         0 - 127
        ///
        /// note, in pd:
        /// [bendin] takes 0 - 16383 while [bendout] returns -8192 - 8192
        /// [pgmin] and [pgmout] are 0 - 127
        ///
        virtual void sendNoteOn(const int channel, const int pitch, const int velocity=64);
        virtual void sendControlChange(const int channel, const int controller, const int value);
        virtual void sendProgramChange(const int channel, const int value);
        virtual void sendPitchBend(const int channel, const int value);
        virtual void sendAftertouch(const int channel, const int value);
        virtual void sendPolyAftertouch(const int channel, const int pitch, const int value);

        /// raw midi bytes
        ///
        /// value is a raw midi byte value 0 - 255
        /// port is the raw portmidi port #, similar to a channel
        ///
        /// for some reason, [midiin], [sysexin] & [realtimein] add 2 to the port num,
        /// so sending to port 1 in PdBase returns port 3 in pd
        ///
        /// however, [midiout], [sysexout], & [realtimeout] do not add to the port num,
        /// so sending port 1 to [midiout] returns port 1 in PdBase
        ///
        virtual void sendMidiByte(const int port, const int value);
        virtual void sendSysex(const int port, const int value);
        virtual void sendSysRealTime(const int port, const int value);

        /// \section Sending Stream Interface

        /// single messages
        ///
        /// pd << Bang("test"); /// "test" is the reciever name in pd
        /// pd << Float("test", 100);
        /// pd << Symbol("test", "a symbol");
        ///
        PdBase& operator<<(const pd::Bang& var);
        PdBase& operator<<(const pd::Float& var);
        PdBase& operator<<(const pd::Symbol& var);

        /// compound messages
        ///
        /// pd << StartMessage() << 100 << 1.2 << "a symbol" << FinishList("test");
        ///
        PdBase& operator<<(const pd::StartMessage& var);
        PdBase& operator<<(const pd::FinishList& var);
        PdBase& operator<<(const pd::FinishMessage& var);

        /// add a float to the message
        PdBase& operator<<(const bool var);
        PdBase& operator<<(const int var);
        PdBase& operator<<(const float var);
        PdBase& operator<<(const double var);

        /// add a symbol to the message
        PdBase& operator<<(const char var);
        PdBase& operator<<(const char* var);
        PdBase& operator<<(const std::string& var);

        /// midi
        ///
        /// pd << NoteOn(64) << NoteOn(64, 60) << NoteOn(64, 60, 1);
        /// pd << ControlChange(100, 64) << ProgramChange(100, 1) << PitchBend(2000, 1);
        /// pd << Aftertouch(127, 1) << PolyAftertouch(64, 127, 1);
        ///
        PdBase& operator<<(const pd::NoteOn& var);
        PdBase& operator<<(const pd::ControlChange& var);
        PdBase& operator<<(const pd::ProgramChange& var);
        PdBase& operator<<(const pd::PitchBend& var);
        PdBase& operator<<(const pd::Aftertouch& var);
        PdBase& operator<<(const pd::PolyAftertouch& var);

        /// compound raw midi byte stream
        ///
        /// pd << StartMidi() << 0xEF << 0x45 << Finish();
        /// pd << StartSysex() << 0xE7 << 0x45 << 0x56 << 0x17 << Finish();
        ///
        PdBase& operator<<(const pd::StartMidi& var);
        PdBase& operator<<(const pd::StartSysex& var);
        PdBase& operator<<(const pd::StartSysRealTime& var);
        PdBase& operator<<(const pd::Finish& var);

        /// is a message or byte stream currently in progress?
        bool isMessageInProgress();

        /// \section Array Access

        /// get the size of a pd array
        /// returns 0 if array not found
        int arraySize(const std::string& arrayName);

        /// read from a pd array
        ///
        /// resizes given vector to readLen, checks readLen and offset
        ///
        /// returns true on success, false on failure
        ///
        /// calling without setting readLen and offset reads the whole array:
        ///
        /// vector<float> array1;
        /// readArray("array1", array1);
        ///
        virtual bool readArray(const std::string& arrayName, std::vector<float>& dest,
                               int readLen=-1, int offset=0);

        /// write to a pd array
        ///
        /// calling without setting writeLen and offset writes the whole array:
        ///
        /// writeArray("array1", array1);
        ///
        virtual bool writeArray(const std::string& arrayName, std::vector<float>& source,
                                int writeLen=-1, int offset=0);

        /// clear array and set to a specific value
        virtual void clearArray(const std::string& arrayName, int value=0);

        /// \section Global Utils

        /// has the global pd instance been initialized?
        bool isInited();

        /// get the blocksize of pd (sample length per channel)
        static int blockSize();

        /// get/set the max length of messages and lists, default: 32
        void setMaxMessageLen(unsigned int len);
        unsigned int maxMessageLen();

        /// get/set the max length of the message queue, default: 1000
        /// the oldest message will be dropped when the queue is full
        void setMaxQueueLen(unsigned int len);
        unsigned int maxQueueLen();

    private:

        /// compound message status
        enum MsgType {
            MSG,
            MIDI,
            SYSEX,
            SYSRT
        };

        /// a singleton libpd instance wrapper
        class PdContext {

            public:

                /// singleton data access
                /// returns a reference to itself
                /// note: only creates a new object on the first call
                static PdContext& instance();

                /// increments the num of pd base objects
                void addBase();

                /// decrements the num of pd base objects
                /// clears if removing last base
                void removeBase();

                /// init the pd instance
                bool init(const int numInChannels, const int numOutChannels, const int sampleRate);

                /// clear the pd instance
                void clear();

                /// turn dsp on/off
                void computeAudio(bool state);

                /// is the instance inited?
                inline bool isInited() {return bInited;}

                /// add a message to the event queue
                /// prints error when dropping messages if queue is full
                void addMessage(pd::Message& msg);

                /// \section Variables

                bool bMsgInProgress;    ///< is a compound message being constructed?
                int maxMsgLen;          ///< maximum allowed message length
                int curMsgLen;          ///< the length of the current message

                /// compound message status
                PdBase::MsgType msgType;

                int midiPort;   ///< target midi port

                std::map<std::string,void*> sources;    ///< subscribed sources

                pd::PdReceiver* receiver;               ///< the message receiver
                pd::PdMidiReceiver* midiReceiver;       ///< the midi receiver

                std::deque<pd::Message> messages;   ///< the event queue
                Message message;                    ///< the current message
                int maxQueueLen;                    ///< max len of queue

            private:

                bool bInited;           ///< is this pd context inited?
				bool bLibPDInited;		///< has libpd_init be called?

                unsigned int numBases;  ///< number of pd base objects

                // hide all the constructors, copy functions here
                PdContext();                        // cannot create
                virtual ~PdContext();               // cannot destroy
                void operator =(PdContext& from) {} // not copyable

                /// libpd static callback functions
                static void _print(const char* s);

                static void _bang(const char* source);
                static void _float(const char* source, float value);
                static void _symbol(const char* source, const char* symbol);

                static void _list(const char* source, int argc, t_atom* argv);
                static void _message(const char* source, const char *symbol,
                                                        int argc, t_atom *argv);

                static void _noteon(int channel, int pitch, int velocity);
                static void _controlchange(int channel, int controller, int value);
                static void _programchange(int channel, int value);
                static void _pitchbend(int channel, int value);
                static void _aftertouch(int channel, int value);
                static void _polyaftertouch(int channel, int pitch, int value);

                static void _midibyte(int port, int byte);
        };
};

} // namespace
