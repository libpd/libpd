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

#include "z_libpd.h"
#include "z_queued.h"
#include "z_print_util.h"

#include <map>  

#include "PdTypes.hpp"
#include "PdReceiver.hpp"
#include "PdMidiReceiver.hpp"

// needed for libpd audio passing
#ifndef USEAPI_DUMMY
    #define USEAPI_DUMMY
#endif

#ifndef HAVE_UNISTD_H
    #define HAVE_UNISTD_H
#endif

typedef struct _atom t_atom;

namespace pd {

/// a Pure Data instance
///
/// use this class directly or extend it and any of its virtual functions
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

    public:

        PdBase() {
            clear();
            PdContext::instance().addBase();
        }

        virtual ~PdBase() {
            clear();
            PdContext::instance().removeBase();
        }

    /// \section Initializing Pd

        /// initialize resources and set up the audio processing
        ///
        /// set the audio latency by setting the libpd ticks per buffer:
        /// ticks per buffer * lib pd block size (always 64)
        ///
        /// ie 4 ticks per buffer * 64 = buffer len of 512
        ///
        /// you can call this again after loading patches & setting receivers
        /// in order to update the audio settings
        ///
        /// the lower the number of ticks, the faster the audio processing
        /// if you experience audio dropouts (audible clicks), increase the
        /// ticks per buffer
        ///
        /// set queued = true to use the built in ringbuffers for message and
        /// midi event passing, you will then need to call receiveMessages() and
        /// receiveMidi() in order to pass messages from the ringbuffers to your
        /// PdReceiver and PdMidiReceiver implementations
        ///
        /// the queued ringbuffers are useful when you need to receive events
        /// on a gui thread and don't want to use locking (aka the mutex)
        ///
        /// return true if setup successfully
        ///
        /// note: must be called before processing
        ///
        virtual bool init(const int numInChannels, const int numOutChannels,
                          const int sampleRate, bool queued=false) {
            PdContext::instance().clear();
            return PdContext::instance().init(numInChannels,
                                              numOutChannels,
                                              sampleRate,
                                              queued);
        }

        /// clear resources
        virtual void clear() {
            PdContext::instance().clear();
            unsubscribeAll();
        }

    /// \section Adding Search Paths

        /// add to the pd search path
        /// takes an absolute or relative path (in data folder)
        ///
        /// note: fails silently if path not found
        ///
        virtual void addToSearchPath(const std::string& path) {
            libpd_add_to_search_path(path.c_str());
        }

        /// clear the current pd search path
        virtual void clearSearchPath() {
            libpd_clear_search_path();
        }

    /// \section Opening Patches

        /// open a patch file (aka somefile.pd) in a specified path
        /// returns a Patch object
        ///
        /// use Patch::isValid() to check if a patch was opened successfully:
        ///
        ///     Patch p1 = pd.openPatch("somefile.pd", "/some/path/");
        ///     if(!p1.isValid()) {
        ///         cout << "aww ... p1 couldn't be opened" << std::endl;
        ///     }
        virtual pd::Patch openPatch(const std::string& patch,
                                    const std::string& path) {
            // [; pd open file folder(
            void* handle = libpd_openfile(patch.c_str(), path.c_str());
            if(handle == NULL) {
                return Patch(); // return empty Patch
            }
            int dollarZero = libpd_getdollarzero(handle);
            return Patch(handle, dollarZero, patch, path);
        }

        /// open a patch file using the filename and path of an existing patch
        ///
        /// set the filename within the patch object or use a previously opened
        /// object
        ///
        ///     // open an instance of "somefile.pd"
        ///     Patch p2("somefile.pd", "/some/path"); // set file and path
        ///     pd.openPatch(p2);
        ///
        ///     // open a new instance of "somefile.pd"
        ///     Patch p3 = pd.openPatch(p2);
        ///
        ///     // p2 and p3 refer to 2 different instances of "somefile.pd"
        ///
        virtual pd::Patch openPatch(pd::Patch& patch) {
            return openPatch(patch.filename(), patch.path());
        }

        /// close a patch file
        /// takes only the patch's basename (filename without extension)
        virtual void closePatch(const std::string& patch) {
            // [; pd-name menuclose 1(
            std::string patchname = (std::string) "pd-"+patch;
            libpd_start_message(PdContext::instance().maxMsgLen);
            libpd_add_float(1.0f);
            libpd_finish_message(patchname.c_str(), "menuclose");
        }

        /// close a patch file, takes a patch object
        /// note: clears the given Patch object
        virtual void closePatch(pd::Patch& patch) {
            if(!patch.isValid()) {
                return;
            }
            libpd_closefile(patch.handle());
            patch.clear();
        }

    /// \section Audio Processing
    ///
    /// one of these must be called for audio dsp and message io to occur
    ///
    /// inBuffer must be an array of the right size and never null
    /// use inBuffer = new type[0] if no input is desired
    ///
    /// outBuffer must be an array of size outBufferSize from openAudio call
    ///
    /// note: raw does not interlace the buffers
    ///

        /// process one pd tick, writes raw float data to/from buffers
        /// returns false on error
        bool processRaw(const float *inBuffer, float *outBuffer) {
            return libpd_process_raw(inBuffer, outBuffer) == 0;
        }

        /// process short buffers for a given number of ticks
        /// returns false on error
        bool processShort(int ticks, const short *inBuffer, short *outBuffer) {
            return libpd_process_short(ticks, inBuffer, outBuffer) == 0;
        }

        /// process float buffers for a given number of ticks
        /// returns false on error
        bool processFloat(int ticks, const float *inBuffer, float *outBuffer) {
            bool ret = libpd_process_float(ticks, inBuffer, outBuffer) == 0;
            return ret;
        }

        /// process double buffers for a given number of ticks
        /// returns false on error
        bool processDouble(int ticks, const double *inBuffer,
                                            double *outBuffer) {
            return libpd_process_double(ticks, inBuffer, outBuffer) == 0;
        }

    /// \section Audio Processing Control

        /// start/stop audio processing
        ///
        /// in general, once started, you won't need to turn off audio
        ///
        /// shortcut for [; pd dsp 1( & [; pd dsp 0(
        ///
        virtual void computeAudio(bool state) {
            PdContext::instance().computeAudio(state);
        }

    /// \section Message Receiving


        /// subscribe to messages sent by a pd send source
        ///
        /// aka this like a virtual pd receive object
        ///
        ///     [r source]
        ///     |
        ///
        virtual void subscribe(const std::string& source) {
            if(exists(source)) {
                std::cerr << "Pd: unsubscribe: ignoring duplicate source" << std::endl;
                return;
            }
            ;
            void* pointer = libpd_bind(source.c_str());
            if(pointer != NULL) {
                std::map<std::string,void*>& sources = PdContext::instance().sources;
                sources.insert(std::pair<std::string,void*>(source, pointer));
            }
        }

        /// unsubscribe from messages sent by a pd send source
        virtual void unsubscribe(const std::string& source) {
            std::map<std::string,void*>& sources = PdContext::instance().sources;
            std::map<std::string,void*>::iterator iter;
            iter = sources.find(source);
            if(iter == sources.end()) {
                std::cerr << "Pd: unsubscribe: ignoring unknown source" << std::endl;
                return;
            }
            libpd_unbind(iter->second);
            sources.erase(iter);
        }

        /// is a pd send source subscribed?
        virtual bool exists(const std::string& source) {
            std::map<std::string,void*>& sources = PdContext::instance().sources;
            if(sources.find(source) != sources.end()) {
                return true;
            }
            return false;
        }

        //// receivers will be unsubscribed from *all* pd send sources
        virtual void unsubscribeAll() {
            std::map<std::string,void*>& sources = PdContext::instance().sources;
            std::map<std::string,void*>::iterator iter;
            for(iter = sources.begin(); iter != sources.end(); ++iter) {
                libpd_unbind(iter->second);
            }
            sources.clear();
        }

    /// \section Receiving from the Message Queues
    ///
    /// process the internal message queue if using the ringbuffer
    ///
    /// internally, libpd will use a ringbuffer to pass messages & midi without
    /// needing to require locking (mutexes) if you call init() with queued = true
    ///
    /// call these in a loop somewhere in order to receive waiting messages
    /// or midi data which are then sent to your PdReceiver & PdMidiReceiver
    ///

        /// process waiting messages
        virtual void receiveMessages() {
            libpd_queued_receive_pd_messages();
        }

        /// process waiting midi messages
        virtual void receiveMidi() {
            libpd_queued_receive_midi_messages();
        }

    /// \section Event Receiving via Callbacks

        /// set the incoming event receiver, disables the event queue
        ///
        /// automatically receives from all currently subscribed sources
        ///
        /// set this to NULL to disable callback receiving and re-enable the
        /// event queue
        ///
        void setReceiver(pd::PdReceiver* receiver) {
            PdContext::instance().receiver = receiver;
        }

    /// \section Midi Receiving via Callbacks

        /// set the incoming midi event receiver, disables the midi queue
        ///
        /// automatically receives from all midi channels
        ///
        /// set this to NULL to disable midi events and re-enable the midi queue
        ///
        void setMidiReceiver(pd::PdMidiReceiver* midiReceiver) {
            PdContext::instance().midiReceiver = midiReceiver;
        }

    /// \section Send Functions

        /// send a bang message
        virtual void sendBang(const std::string& dest) {
            libpd_bang(dest.c_str());
        }

        /// send a float
        virtual void sendFloat(const std::string& dest, float value) {
            libpd_float(dest.c_str(), value);
        }

        /// send a symbol
        virtual void sendSymbol(const std::string& dest, const std::string& symbol) {
            libpd_symbol(dest.c_str(), symbol.c_str());
        }

    /// \section Sending Compound Messages
    ///
    ///     pd.startMessage();
    ///     pd.addSymbol("hello");
    ///     pd.addFloat(1.23);
    ///     pd.finishList("test"); // "test" is the receiver name in pd
    ///
    /// sends [list hello 1.23( -> [r test],
    /// you will need to use the [list trim] object on the receiving end
    ///
    /// finishMsg sends a typed message -> [; test msg1 hello 1.23(
    ///
    ///     pd.startMessage();
    ///     pd.addSymbol("hello");
    ///     pd.addFloat(1.23);
    ///     pd.finishMessage("test", "msg1");
    ///

        /// start a compound list or message
        virtual void startMessage() {
            PdContext& context = PdContext::instance();
            if(context.bMsgInProgress) {
                std::cerr << "Pd: Can not start message, message in progress" << std::endl;
                return;
            }
            if(libpd_start_message(context.maxMsgLen) == 0) {
                context.bMsgInProgress = true;
                context.msgType = MSG;
            }
        }

        /// add a float to the current compound list or message
        virtual void addFloat(const float num) {
            PdContext& context = PdContext::instance();
            if(!context.bMsgInProgress) {
                std::cerr << "Pd: Can not add float, message not in progress" << std::endl;
                return;
            }
            if(context.msgType != MSG) {
                std::cerr << "Pd: Can not add float, midi byte stream in progress" << std::endl;
                return;
            }
            if(context.curMsgLen+1 >= context.maxMsgLen) {
                std::cerr << "Pd: Can not add float, max message len of " << context.maxMsgLen << " reached" << std::endl;
                return;
            }
            libpd_add_float(num);
            context.curMsgLen++;
        }

        /// add a symbol to the current compound list or message
        virtual void addSymbol(const std::string& symbol) {
            PdContext& context = PdContext::instance();
            if(!context.bMsgInProgress) {
                std::cerr << "Pd: Can not add symbol, message not in progress" << std::endl;;
                return;
            }
            if(context.msgType != MSG) {
                std::cerr << "Pd: Can not add symbol, midi byte stream in progress" << std::endl;;
                return;
            }
            if(context.curMsgLen+1 >= context.maxMsgLen) {
                std::cerr << "Pd: Can not add symbol, max message len of " << context.maxMsgLen << " reached" << std::endl;
                return;
            }
            libpd_add_symbol(symbol.c_str());
            context.curMsgLen++;
        }

        /// finish and send as a list
        virtual void finishList(const std::string& dest) {
            PdContext& context = PdContext::instance();
            if(!context.bMsgInProgress) {
                std::cerr << "Pd: Can not finish list, message not in progress" << std::endl;
                return;
            }
            if(context.msgType != MSG) {
                std::cerr << "Pd: Can not finish list, midi byte stream in progress" << std::endl;
                return;
            }
            libpd_finish_list(dest.c_str());
            context.bMsgInProgress = false;
            context.curMsgLen = 0;
        }

        /// finish and send as a list with a specific message name
        virtual void finishMessage(const std::string& dest, const std::string& msg) {
            PdContext& context = PdContext::instance();
            if(!context.bMsgInProgress) {
                std::cerr << "Pd: Can not finish message, message not in progress" << std::endl;
                return;
            }
            if(context.msgType != MSG) {
                std::cerr << "Pd: Can not finish message, midi byte stream in progress" << std::endl;
                return;
            }
            libpd_finish_message(dest.c_str(), msg.c_str());
            context.bMsgInProgress = false;
            context.curMsgLen = 0;
        }

        /// send a list using the PdBase List type
        ///
        ///     List list;
        ///     list.addSymbol("hello");
        ///     list.addFloat(1.23);
        ///     pd.sstd::endlist("test", list);
        ///
        /// sends [list hello 1.23( -> [r test]
        ///
        /// stream operators work as well:
        ///
        ///     list << "hello" << 1.23;
        ///     pd.sstd::endlist("test", list);
        ///
        virtual void sendList(const std::string& dest, const pd::List& list) {
            PdContext& context = PdContext::instance();
            if(context.bMsgInProgress) {
                std::cerr << "Pd: Can not send list, message in progress" << std::endl;
                return;
            }
            libpd_start_message(list.len());
            context.bMsgInProgress = true;
            // step through list
            for(int i = 0; i < (int)list.len(); ++i) {
                if(list.isFloat(i))
                    addFloat(list.getFloat(i));
                else if(list.isSymbol(i))
                    addSymbol(list.getSymbol(i));
            }
            finishList(dest);
        }

        /// send a message using the PdBase List type
        ///
        ///     List list;
        ///     list.addSymbol("hello");
        ///     list.addFloat(1.23);
        ///     pd.sendMessage("test", "msg1", list);
        ///
        /// sends a typed message -> [; test msg1 hello 1.23(
        ///
        /// stream operators work as well:
        ///
        //      list << "hello" << 1.23;
        ///     pd.sendMessage("test", "msg1", list);
        ///
        virtual void sendMessage(const std::string& dest,
                                 const std::string& msg,
                                 const pd::List& list = pd::List()) {
            PdContext& context = PdContext::instance();
            if(context.bMsgInProgress) {
                std::cerr << "Pd: Can not send message, message in progress" << std::endl;
                return;
            }
            libpd_start_message(list.len());
            context.bMsgInProgress = true;
            // step through list
            for(int i = 0; i < (int)list.len(); ++i) {
                if(list.isFloat(i))
                    addFloat(list.getFloat(i));
                else if(list.isSymbol(i))
                    addSymbol(list.getSymbol(i));
            }
            finishMessage(dest, msg);
        }

    /// \section Sending MIDI
    ///
    /// any out of range messages will be silently ignored
    ///
    /// number ranges:
    /// * channel             0 - 15 * dev# (dev #0: 0-15, dev #1: 16-31, etc)
    /// * pitch               0 - 127
    /// * velocity            0 - 127
    /// * controller value    0 - 127
    /// * program value       1 - 128
    /// * bend value          -8192 - 8191
    /// * touch value         0 - 127
    ///

        /// send a MIDI note on
        ///
        /// pd does not use note off MIDI messages, so send a note on with vel = 0
        ///
        virtual void sendNoteOn(const int channel,
                                const int pitch,
                                const int velocity=64) {
            libpd_noteon(channel, pitch, velocity);
        }

        /// send a MIDI control change
        virtual void sendControlChange(const int channel,
                                       const int controller,
                                       const int value) {
            libpd_controlchange(channel, controller, value);
        }

        /// send a MIDI program change
        ///
        /// in pd: [pgmin] and [pgmout] are 0 - 127
        ///
        virtual void sendProgramChange(const int channel, const int value) {
            libpd_programchange(channel, value);
        }
        
        /// send a MIDI pitch bend
        ///
        /// in pd: [bendin] takes 0 - 16383 while [bendout] returns -8192 - 8192
        ///
        virtual void sendPitchBend(const int channel, const int value) {
            libpd_pitchbend(channel, value);
        }
        
        /// send a MIDI aftertouch
        virtual void sendAftertouch(const int channel, const int value) {
            libpd_aftertouch(channel, value);
        }

        /// send a MIDI poly aftertouch
        virtual void sendPolyAftertouch(const int channel,
                                        const int pitch,
                                        const int value) {
            libpd_polyaftertouch(channel, pitch, value);
        }

        /// send a raw MIDI byte
        ///
        /// value is a raw midi byte value 0 - 255
        /// port is the raw portmidi port #, similar to a channel
        ///
        /// for some reason, [midiin], [sysexin] & [realtimein] add 2 to the
        /// port num, so sending to port 1 in PdBase returns port 3 in pd
        ///
        /// however, [midiout], [sysexout], & [realtimeout] do not add to the
        /// port num, so sending port 1 to [midiout] returns port 1 in PdBase
        ///
        virtual void sendMidiByte(const int port, const int value) {
            libpd_midibyte(port, value);
        }

        /// send a raw MIDI sysex byte
        virtual void sendSysex(const int port, const int value) {
            libpd_sysex(port, value);
        }

        /// send a raw MIDI realtime byte
        virtual void sendSysRealTime(const int port, const int value) {
            libpd_sysrealtime(port, value);
        }

    /// \section Stream Interface
    ///
    /// single messages
    ///
    ///     pd << Bang("test"); /// "test" is the receiver name in pd
    ///     pd << Float("test", 100);
    ///     pd << Symbol("test", "a symbol");
    ///

        /// send a bang message
        PdBase& operator<<(const pd::Bang& var) {
            if(PdContext::instance().bMsgInProgress) {
                std::cerr << "Pd: Can not send Bang, message in progress" << std::endl;
                return *this;
            }
            sendBang(var.dest.c_str());
            return *this;
        }

        /// send a float message
        PdBase& operator<<(const pd::Float& var) {
            if(PdContext::instance().bMsgInProgress) {
                std::cerr << "Pd: Can not send Float, message in progress" << std::endl;
                return *this;
            }
            sendFloat(var.dest.c_str(), var.num);
            return *this;
        }

        /// send a symbol message
        PdBase& operator<<(const pd::Symbol& var) {
            if(PdContext::instance().bMsgInProgress) {
                std::cerr << "Pd: Can not send Symbol, message in progress" << std::endl;
                return *this;
            }
            sendSymbol(var.dest.c_str(), var.symbol.c_str());
            return *this;
        }

    /// \section Stream Interface for Compound Messages
    ///
    /// pd << StartMessage() << 100 << 1.2 << "a symbol" << FinishList("test");
    ///

        /// start a compound message
        PdBase& operator<<(const pd::StartMessage& var) {
            startMessage();
            return *this;
        }

        /// finish a compound message and send it as a list
        PdBase& operator<<(const pd::FinishList& var) {
            finishList(var.dest);
            return *this;
        }

        /// finish a compound message and send it as a message
        PdBase& operator<<(const pd::FinishMessage& var) {
            finishMessage(var.dest, var.msg);
            return *this;
        }

        // add a boolean as a float to the compound message
        PdBase& operator<<(const bool var) {
            addFloat((float) var);
            return *this;
        }

        // add an integer as a float to the compound message
        PdBase& operator<<(const int var) {
            PdContext& context = PdContext::instance();
            switch(context.msgType) {
                case MSG:
                    addFloat((float) var);
                    break;
                case MIDI:
                    sendMidiByte(context.midiPort, var);
                    break;
                case SYSEX:
                    sendSysex(context.midiPort, var);
                    break;
                case SYSRT:
                    sendSysRealTime(context.midiPort, var);
                    break;
            }
            return *this;
        }

        // add a float to the compound message
        PdBase& operator<<(const float var) {
            addFloat((float) var);
            return *this;
        }

        // add a double as a float to the compound message
        PdBase& operator<<(const double var) {
            addFloat((float) var);
            return *this;
        }

        // add a character as a symbol to the compound message
        PdBase& operator<<(const char var) {
            std::string s;
            s = var;
            addSymbol(s);
            return *this;
        }

        // add a C-string char buffer as a symbol to the compound message
        PdBase& operator<<(const char* var) {
            addSymbol((std::string) var);
            return *this;
        }

        // add a string as a symbol to the compound message
        PdBase& operator<<(const std::string& var) {
            addSymbol(var);
            return *this;
        }

    /// \section Stream Interface for MIDI
    ///
    /// pd << NoteOn(64) << NoteOn(64, 60) << NoteOn(64, 60, 1);
    /// pd << ControlChange(100, 64) << ProgramChange(100, 1);
    /// pd << Aftertouch(127, 1) << PolyAftertouch(64, 127, 1);
    /// pd << PitchBend(2000, 1);
    ///

        /// send a MIDI note on
        PdBase& operator<<(const pd::NoteOn& var) {
            sendNoteOn(var.channel, var.pitch, var.velocity);
            return *this;
        }

        /// send a MIDI control change
        PdBase& operator<<(const pd::ControlChange& var) {
            sendControlChange(var.channel, var.controller, var.value);
            return *this;
        }

        /// send a MIDI program change
        PdBase& operator<<(const pd::ProgramChange& var) {
            sendProgramChange(var.channel, var.value);
            return *this;
        }

        /// send a MIDI pitch bend
        PdBase& operator<<(const pd::PitchBend& var) {
            sendPitchBend(var.channel, var.value);
            return *this;
        }

        /// send a MIDI aftertouch
        PdBase& operator<<(const pd::Aftertouch& var) {
            sendAftertouch(var.channel, var.value);
            return *this;
        }

        /// send a MIDI poly aftertouch
        PdBase& operator<<(const pd::PolyAftertouch& var) {
            sendPolyAftertouch(var.channel, var.pitch, var.value);
            return *this;
        }

    /// \section Stream Interface for Raw Bytes
    ///
    /// pd << StartMidi() << 0xEF << 0x45 << Finish();
    /// pd << StartSysex() << 0xE7 << 0x45 << 0x56 << 0x17 << Finish();
    ///

        /// start a raw byte MIDI message
        PdBase& operator<<(const pd::StartMidi& var) {
            PdContext& context = PdContext::instance();
            if(context.bMsgInProgress) {
                std::cerr << "Pd: Can not start MidiByte stream, "
                     << "message in progress" << std::endl;
                return *this;
            }
            context.bMsgInProgress = true;
            context.msgType = MIDI;
            context.midiPort = var.port;
            return *this;
        }

        /// start a raw byte MIDI sysex message
        PdBase& operator<<(const pd::StartSysex& var) {
            PdContext& context = PdContext::instance();
            if(context.bMsgInProgress) {
                std::cerr << "Pd: Can not start Sysex stream, "
                     << "message in progress" << std::endl;
                return *this;
            }
            context.bMsgInProgress = true;
            context.msgType = SYSEX;
            context.midiPort = var.port;
            return *this;
        }

        /// start a raw byte MIDI realtime message
        PdBase& operator<<(const pd::StartSysRealTime& var) {
            PdContext& context = PdContext::instance();
            if(context.bMsgInProgress) {
                std::cerr << "Pd: Can not start SysRealRime stream, "
                     << "message in progress" << std::endl;
                return *this;
            }
            context.bMsgInProgress = true;
            context.msgType = SYSRT;
            context.midiPort = var.port;
            return *this;
        }

        /// finish and send a raw byte MIDI message
        PdBase& operator<<(const pd::Finish& var) {
            PdContext& context = PdContext::instance();
            if(!context.bMsgInProgress) {
                std::cerr << "Pd: Can not finish midi byte stream, "
                     << "stream not in progress" << std::endl;
                return *this;
            }
            if(context.msgType == MSG) {
                std::cerr << "Pd: Can not finish midi byte stream, "
                     << "message in progress" << std::endl;
                return *this;
            }
            context.bMsgInProgress = false;
            context.curMsgLen = 0;
            return *this;
        }

        /// is a message or byte stream currently in progress?
        bool isMessageInProgress() {
            return PdContext::instance().bMsgInProgress;
        }

    /// \section Array Access

        /// get the size of a pd array
        /// returns 0 if array not found
        int arraySize(const std::string& name) {
            int len = libpd_arraysize(name.c_str());;
            if(len < 0) {
                std::cerr << "Pd: Cannot get size of unknown array \""
                     << name << "\"" << std::endl;
                return 0;
            }
            return len;
        }

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
        virtual bool readArray(const std::string& name,
                               std::vector<float>& dest,
                               int readLen=-1, int offset=0) {
            int len = libpd_arraysize(name.c_str());
            if(len < 0) {
                std::cerr << "Pd: Cannot read unknown array \""
                     << name << "\"" << std::endl;
                return false;
            }
            // full array len?
            if(readLen < 0) {
                readLen = len;
            }
            // check read len
            else if(readLen > len) {
                std::cerr << "Pd: Given read len " << readLen << " > len "
                     << len << " of array \"" << name << "\"" << std::endl;
                return false;
            }
            // check offset
            if(offset+readLen > len) {
                std::cerr << "Pd: Given read len and offset > len " << readLen
                     << " of array \"" << name << "\"" << std::endl;
                return false;
            }
            // resize if necessary
            if(dest.size() != readLen) {
                dest.resize(readLen, 0);
            }
            if(libpd_read_array(&dest[0], name.c_str(), offset, readLen) < 0) {
                std::cerr << "Pd: libpd_read_array failed for array \""
                     << name << "\"" << std::endl;
                return false;
            }
            return true;
        }


        /// write to a pd array
        ///
        /// calling without setting writeLen and offset writes the whole array:
        ///
        /// writeArray("array1", array1);
        ///
        virtual bool writeArray(const std::string& name,
                                std::vector<float>& source,
                                int writeLen=-1, int offset=0) {
            int len = libpd_arraysize(name.c_str());
            if(len < 0) {
                std::cerr << "Pd: Cannot write to unknown array \""
                     << name << "\"" << std::endl;
                return false;
            }

            // full array len?
            if(writeLen < 0) {
                writeLen = len;
            }

            // check write len
            else if(writeLen > len) {
                std::cerr << "Pd: Given write len " << writeLen << " > len " << len
                     << " of array \"" << name << "\"" << std::endl;
                return false;
            }

            // check offset
            if(offset+writeLen > len) {
                std::cerr << "Pd: Given write len and offset > len " << writeLen
                     << " of array \"" << name << "\"" << std::endl;
                return false;
            }

            if(libpd_write_array(name.c_str(), offset,
                                 &source[0], writeLen) < 0) {
                std::cerr << "Pd: libpd_write_array failed for array \""
                     << name << "\"" << std::endl;
                return false;
            }
            return true;
        }

        /// clear array and set to a specific value
        virtual void clearArray(const std::string& name, int value=0) {
            int len = libpd_arraysize(name.c_str());
            if(len < 0) {
                std::cerr << "Pd: Cannot clear unknown array \""
                     << name << "\"" << std::endl;
                return;
            }
            std::vector<float> array;
            array.resize(len, value);
            if(libpd_write_array(name.c_str(), 0, &array[0], len) < 0) {
                std::cerr << "Pd: libpd_write_array failed while clearing array \""
                     << name << "\"" << std::endl;
            }
        }

    /// \section Utils

        /// has the global pd instance been initialized?
        bool isInited() {
            return PdContext::instance().isInited();
        }

        /// is the global pd instance using the ringerbuffer queue
        /// for message padding?
        bool isQueued() {
            return PdContext::instance().isQueued();
        }

        /// get the blocksize of pd (sample length per channel)
        static int blockSize() {
            // shouldn't need to lock this for now, it's always 64
            return libpd_blocksize();
        }

        /// set the max length of messages and lists, default: 32
        void setMaxMessageLen(unsigned int len) {
            PdContext::instance().maxMsgLen = len;
        }

        /// get the max length of messages and lists
        unsigned int maxMessageLen() {
            return PdContext::instance().maxMsgLen;
        }

    protected:
    
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
                static PdContext& instance() {
                    static PdBase::PdContext *singletonInstance = new PdContext;
                    return *singletonInstance;
                }

                /// increments the num of pd base objects
                void addBase() {numBases++;}

                /// decrements the num of pd base objects
                /// clears if removing last base
                void removeBase() {
                    if(numBases > 0) {
                        numBases--;
                    }
                    else if(bInited) { // double check clear
                        clear();
                    }
                }

                /// init the pd instance
                bool init(const int numInChannels, const int numOutChannels,
                          const int sampleRate, bool queued) {

                    // attach callbacks
                    bQueued = queued;
                    if(queued) {
                        libpd_set_queued_printhook(libpd_print_concatenator);
                        libpd_set_concatenated_printhook(_print);

                        libpd_set_queued_banghook(_bang);
                        libpd_set_queued_floathook(_float);
                        libpd_set_queued_symbolhook(_symbol);
                        libpd_set_queued_listhook(_list);
                        libpd_set_queued_messagehook(_message);

                        libpd_set_queued_noteonhook(_noteon);
                        libpd_set_queued_controlchangehook(_controlchange);
                        libpd_set_queued_programchangehook(_programchange);
                        libpd_set_queued_pitchbendhook(_pitchbend);
                        libpd_set_queued_aftertouchhook(_aftertouch);
                        libpd_set_queued_polyaftertouchhook(_polyaftertouch);
                        libpd_set_queued_midibytehook(_midibyte);
                        
                        // init libpd, should only be called once!
                        if(!bLibPdInited) {
                            libpd_queued_init();
                            bLibPdInited = true;
                        }
                    }
                    else {
                        libpd_set_printhook(libpd_print_concatenator);
                        libpd_set_concatenated_printhook(_print);

                        libpd_set_banghook(_bang);
                        libpd_set_floathook(_float);
                        libpd_set_symbolhook(_symbol);
                        libpd_set_listhook(_list);
                        libpd_set_messagehook(_message);

                        libpd_set_noteonhook(_noteon);
                        libpd_set_controlchangehook(_controlchange);
                        libpd_set_programchangehook(_programchange);
                        libpd_set_pitchbendhook(_pitchbend);
                        libpd_set_aftertouchhook(_aftertouch);
                        libpd_set_polyaftertouchhook(_polyaftertouch);
                        libpd_set_midibytehook(_midibyte);
                        
                        // init libpd, should only be called once!
                        if(!bLibPdInited) {
                            libpd_init();
                            bLibPdInited = true;
                        }
                    }
                    
                    // init audio
                    if(libpd_init_audio(numInChannels, numOutChannels, sampleRate) != 0) {
                        return false;
                    }
                    bInited = true;

                    return bInited;
                }

                /// clear the pd instance
                void clear() {

                    // detach callbacks
                    if(bInited) {
                        computeAudio(false);
                        if(bQueued) {
                            libpd_set_queued_printhook(NULL);
                            libpd_set_concatenated_printhook(NULL);

                            libpd_set_queued_banghook(NULL);
                            libpd_set_queued_floathook(NULL);
                            libpd_set_queued_symbolhook(NULL);
                            libpd_set_queued_listhook(NULL);
                            libpd_set_queued_messagehook(NULL);

                            libpd_set_queued_noteonhook(NULL);
                            libpd_set_queued_controlchangehook(NULL);
                            libpd_set_queued_programchangehook(NULL);
                            libpd_set_queued_pitchbendhook(NULL);
                            libpd_set_queued_aftertouchhook(NULL);
                            libpd_set_queued_polyaftertouchhook(NULL);
                            libpd_set_queued_midibytehook(NULL);
                            
                            libpd_queued_release();
                        }
                        else {
                            libpd_set_printhook(NULL);
                            libpd_set_concatenated_printhook(NULL);

                            libpd_set_banghook(NULL);
                            libpd_set_floathook(NULL);
                            libpd_set_symbolhook(NULL);
                            libpd_set_listhook(NULL);
                            libpd_set_messagehook(NULL);

                            libpd_set_noteonhook(NULL);
                            libpd_set_controlchangehook(NULL);
                            libpd_set_programchangehook(NULL);
                            libpd_set_pitchbendhook(NULL);
                            libpd_set_aftertouchhook(NULL);
                            libpd_set_polyaftertouchhook(NULL);
                            libpd_set_midibytehook(NULL);
                        }
                    }
                    bInited = false;
                    bQueued = false;

                    bMsgInProgress = false;
                    curMsgLen = 0;
                    msgType = MSG;
                    midiPort = 0;
                }

                /// turn dsp on/off
                void computeAudio(bool state) {
                    // [; pd dsp $1(
                    libpd_start_message(1);
                    libpd_add_float((float) state);
                    libpd_finish_message("pd", "dsp");
                }

                /// is the instance inited?
                inline bool isInited() {return bInited;}

                /// is this instance queued?
                inline bool isQueued() {return bQueued;}

            /// \section Variables

                bool bMsgInProgress;    //< is a compound message being constructed?
                int maxMsgLen;          //< maximum allowed message length
                int curMsgLen;          //< the length of the current message

                /// compound message status
                PdBase::MsgType msgType;

                int midiPort;   //< target midi port

                std::map<std::string,void*> sources;    //< subscribed sources

                pd::PdReceiver* receiver;               //< the message receiver
                pd::PdMidiReceiver* midiReceiver;       //< the midi receiver

            private:

                bool bLibPdInited; //< has libpd_init be called?
                bool bInited;      //< is this pd context inited?
                bool bQueued; //< is this context using the libpd_queued ringbuffer?

                unsigned int numBases;  //< number of pd base objects

                // hide all the constructors, copy functions here
                PdContext() {                      // cannot create
                    bLibPdInited = false;
                    bInited = false;
                    bQueued = false;
                    numBases = false;
                    receiver = NULL;
                    midiReceiver = NULL;
                    clear();
                    maxMsgLen = 32;
                }                       
                virtual ~PdContext() {             // cannot destroy
                    // triple check clear
                    if(bInited) {clear();}
                }
                void operator =(PdContext& from) {} // not copyable

                /// libpd static callback functions
                static void _print(const char* s) {
                    PdContext& context = PdContext::instance();
                    if(context.receiver) {
                        context.receiver->print((std::string) s);
                    }
                }

                static void _bang(const char* source) {
                    PdContext& context = PdContext::instance();
                    if(context.receiver) {
                        context.receiver->receiveBang((std::string) source);
                    }
                }

                static void _float(const char* source, float value) {
                    PdContext& context = PdContext::instance();
                    if(context.receiver) {
                        context.receiver->receiveFloat((std::string) source, value);
                    }
                }

                static void _symbol(const char* source, const char* symbol) {
                    PdContext& context = PdContext::instance();
                    if(context.receiver) {
                        context.receiver->receiveSymbol((std::string) source, (std::string) symbol);
                    }
                }

                static void _list(const char* source, int argc, t_atom* argv) {
                    PdContext& context = PdContext::instance();
                    pd::List list;
                    for(int i = 0; i < argc; i++) {
                        t_atom a = argv[i];
                        if(a.a_type == A_FLOAT) {
                            float f = a.a_w.w_float;
                            list.addFloat(f);
                        }
                        else if(a.a_type == A_SYMBOL) {
                            const char* s = a.a_w.w_symbol->s_name;
                            list.addSymbol((std::string) s);
                        }
                    }
                    if(context.receiver) {
                        context.receiver->receiveList((std::string) source, list);
                    }
                }

                static void _message(const char* source, const char *symbol,
                                     int argc, t_atom *argv) {
                    PdContext& context = PdContext::instance();
                    List list;
                    for(int i = 0; i < argc; i++) {
                        t_atom a = argv[i];
                        if(a.a_type == A_FLOAT) {
                            float f = a.a_w.w_float;
                            list.addFloat(f);
                        }
                        else if(a.a_type == A_SYMBOL) {
                            const char* s = a.a_w.w_symbol->s_name;
                            list.addSymbol((std::string) s);
                        }
                    }
                    if(context.receiver) {
                        context.receiver->receiveMessage((std::string) source, (std::string) symbol, list);
                    }
                }

                static void _noteon(int channel, int pitch, int velocity) {
                    PdContext& context = PdContext::instance();
                    if(context.midiReceiver) {
                        context.midiReceiver->receiveNoteOn(channel, pitch, velocity);
                    }
                }

                static void _controlchange(int channel, int controller, int value) {
                    PdContext& context = PdContext::instance();
                    if(context.midiReceiver) {
                        context.midiReceiver->receiveControlChange(channel, controller, value);
                    }
                }

                static void _programchange(int channel, int value) {
                    PdContext& context = PdContext::instance();
                    if(context.midiReceiver) {
                        context.midiReceiver->receiveProgramChange(channel, value);
                    }
                }

                static void _pitchbend(int channel, int value) {
                    PdContext& context = PdContext::instance();
                    if(context.midiReceiver) {
                        context.midiReceiver->receivePitchBend(channel, value);
                    }
                }

                static void _aftertouch(int channel, int value) {
                    PdContext& context = PdContext::instance();
                    if(context.midiReceiver) {
                        context.midiReceiver->receiveAftertouch(channel, value);
                    }
                }

                static void _polyaftertouch(int channel, int pitch, int value) {
                    PdContext& context = PdContext::instance();
                    if(context.midiReceiver) {
                        context.midiReceiver->receivePolyAftertouch(channel, pitch, value);
                    }
                }

                static void _midibyte(int port, int byte) {
                    PdContext& context = PdContext::instance();
                    if(context.midiReceiver) {
                        context.midiReceiver->receiveMidiByte(port, byte);
                    }
                }
        };
};

} // namespace
