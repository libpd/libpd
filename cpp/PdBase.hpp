/*
 * Copyright (c) 2012-2022 Dan Wilcox <danomatika@gmail.com>
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

#ifdef PDINSTANCE
    #define PDBASE_SETINSTANCE libpd_set_instance(instance);
#else
    #define PDBASE_SETINSTANCE
#endif

typedef struct _atom t_atom;

namespace pd {

/// a Pure Data instance
///
/// use this class directly or extend it and any of its virtual functions
///
/// by default, each PdBase instance refers to the single main libpd
/// instance so it is recommended to use only one PdBase at a time
///
/// as of version 0.13, libpd supports multiple instances if compiled with
/// PDINSTANCE defined, in which case each PdBase instance can act separately
/// with it's own PdReceiver and PdMidiReceiver
///
class PdBase {

public:

    PdBase() {
        bMsgInProgress = false;
        maxMsgLen = 32;
        curMsgLen = 0;
        msgType = MSG;
        midiPort = 0;
        receiver = NULL;
        midiReceiver = NULL;
        bInited = false;
        bQueued = false;
        libpd_init();
        #ifdef PDINSTANCE
            instance = libpd_new_instance();
        #endif
        libpd_set_instancedata(this, NULL);
    }

    virtual ~PdBase() {
        PDBASE_SETINSTANCE
        libpd_set_instancedata(NULL, NULL);
        #ifdef PDINSTANCE
            libpd_set_instance(instance);
            libpd_free_instance(instance);
        #endif
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
    /// on a gui thread and don't want to use locking
    ///
    /// return true if inited successfully
    ///
    /// note: must be called before processing
    ///
    virtual bool init(const int numInChannels, const int numOutChannels,
                      const int sampleRate, bool queued=false) {
        PDBASE_SETINSTANCE

        // attach callbacks
        bQueued = queued;
        if(queued) {
            libpd_queued_init();

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
        }

        // init audio
        if(libpd_init_audio(numInChannels,
                            numOutChannels,
                            sampleRate) != 0) {
            return false;
        }
        bInited = true;

        return bInited;
    }

    /// clear resources
    virtual void clear() {
        PDBASE_SETINSTANCE

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

        unsubscribeAll();
    }

/// \section Adding Search Paths

    /// add to the pd search path
    /// takes an absolute or relative path (in data folder)
    ///
    /// note: fails silently if path not found
    ///
    virtual void addToSearchPath(const std::string &path) {
        PDBASE_SETINSTANCE
        libpd_add_to_search_path(path.c_str());
    }

    /// clear the current pd search path
    virtual void clearSearchPath() {
        PDBASE_SETINSTANCE
        libpd_clear_search_path();
    }

/// \section Opening Patches

    /// open a patch file (aka somefile.pd) at a specified parent dir path
    /// returns a pd::Patch object
    ///
    /// use pd::Patch::isValid() to check if a patch was opened successfully:
    ///
    ///     pd::Patch p1 = pd.openPatch("somefile.pd", "/some/dir/path/");
    ///     if(!p1.isValid()) {
    ///         std::cout << "aww ... p1 couldn't be opened" << std::endl;
    ///     }
    virtual pd::Patch openPatch(const std::string &patch,
                                const std::string &path) {
        PDBASE_SETINSTANCE
        // [; pd open file folder(
        void *handle = libpd_openfile(patch.c_str(), path.c_str());
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
    ///     pd::Patch p2("somefile.pd", "/some/path"); // set file and path
    ///     pd.openPatch(p2);
    ///
    ///     // open a new instance of "somefile.pd"
    ///     pd::Patch p3 = pd.openPatch(p2);
    ///
    ///     // p2 and p3 refer to 2 different instances of "somefile.pd"
    ///
    virtual pd::Patch openPatch(pd::Patch &patch) {
        return openPatch(patch.filename(), patch.path());
    }

    /// close a patch file
    /// takes only the patch's basename (filename without extension)
    virtual void closePatch(const std::string &patch) {
        PDBASE_SETINSTANCE
        // [; pd-name menuclose 1(
        std::string patchname = (std::string)"pd-" + patch;
        libpd_start_message(1);
        libpd_add_float(1);
        libpd_finish_message(patchname.c_str(), "menuclose");
    }

    /// close a patch file, takes a patch object
    /// note: clears the given Patch object
    virtual void closePatch(pd::Patch &patch) {
        if(!patch.isValid()) {
            return;
        }
        PDBASE_SETINSTANCE
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

    /// process float buffers for a given number of ticks
    /// returns false on error
    bool processFloat(int ticks, const float *inBuffer, float *outBuffer) {
        PDBASE_SETINSTANCE
        return libpd_process_float(ticks, inBuffer, outBuffer) == 0;
    }

    /// process short buffers for a given number of ticks
    /// returns false on error
    bool processShort(int ticks, const short *inBuffer, short *outBuffer) {
        PDBASE_SETINSTANCE
        return libpd_process_short(ticks, inBuffer, outBuffer) == 0;
    }

    /// process double buffers for a given number of ticks
    /// returns false on error
    bool processDouble(int ticks, const double *inBuffer, double *outBuffer) {
        PDBASE_SETINSTANCE
        return libpd_process_double(ticks, inBuffer, outBuffer) == 0;
    }

    /// process one pd tick, writes raw float data to/from buffers
    /// returns false on error
    bool processRaw(const float *inBuffer, float *outBuffer) {
        PDBASE_SETINSTANCE
        return libpd_process_raw(inBuffer, outBuffer) == 0;
    }

    /// process one pd tick, writes raw short data to/from buffers
    /// returns false on error
    bool processRawShort(const short *inBuffer, short *outBuffer) {
        PDBASE_SETINSTANCE
        return libpd_process_raw_short(inBuffer, outBuffer) == 0;
    }

    /// process one pd tick, writes raw double data to/from buffers
    /// returns false on error
    bool processRawDouble(const double *inBuffer, double *outBuffer) {
        PDBASE_SETINSTANCE
        return libpd_process_raw_double(inBuffer, outBuffer) == 0;
    }

/// \section Audio Processing Control

    /// start/stop audio processing
    ///
    /// in general, once started, you won't need to turn off audio
    ///
    /// shortcut for [; pd dsp 1( & [; pd dsp 0(
    ///
    virtual void computeAudio(bool state) {
        PDBASE_SETINSTANCE
        // [; pd dsp $1(
        libpd_start_message(1);
        libpd_add_float((float) state);
        libpd_finish_message("pd", "dsp");
    }

/// \section Message Receiving


    /// subscribe to messages sent by a pd send source
    ///
    /// aka this like a virtual pd receive object
    ///
    ///     [r source]
    ///     |
    ///
    virtual void subscribe(const std::string &source) {
        if(exists(source)) {
            std::cerr << "Pd: unsubscribe: ignoring duplicate source"
                      << std::endl;
            return;
        }
        PDBASE_SETINSTANCE
        void *pointer = libpd_bind(source.c_str());
        if(pointer != NULL) {
            sources.insert(std::pair<std::string,void*>(source, pointer));
        }
    }

    /// unsubscribe from messages sent by a pd send source
    virtual void unsubscribe(const std::string &source) {
        std::map<std::string,void*>::iterator iter;
        iter = sources.find(source);
        if(iter == sources.end()) {
            std::cerr << "Pd: unsubscribe: ignoring unknown source"
                      << std::endl;
            return;
        }
        PDBASE_SETINSTANCE
        libpd_unbind(iter->second);
        sources.erase(iter);
    }

    /// is a pd send source subscribed?
    virtual bool exists(const std::string &source) {
        PDBASE_SETINSTANCE
        if(sources.find(source) != sources.end()) {
            return true;
        }
        return false;
    }

    //// receivers will be unsubscribed from *all* pd send sources
    virtual void unsubscribeAll() {
        PDBASE_SETINSTANCE
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
/// needing to require locking if you call init() with queued = true
///
/// call these in a loop somewhere in order to receive waiting messages
/// or midi data which are then sent to your PdReceiver & PdMidiReceiver
///
/// *do not* use if inited with queued = false

    /// process waiting messages
    virtual void receiveMessages() {
        PDBASE_SETINSTANCE
        libpd_queued_receive_pd_messages();
    }

    /// process waiting midi messages
    virtual void receiveMidi() {
        PDBASE_SETINSTANCE
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
    void setReceiver(pd::PdReceiver *receiver) {
        this->receiver = receiver;
    }

/// \section Midi Receiving via Callbacks

    /// set the incoming midi event receiver, disables the midi queue
    ///
    /// automatically receives from all midi channels
    ///
    /// set this to NULL to disable midi events and re-enable the midi queue
    ///
    void setMidiReceiver(pd::PdMidiReceiver *midiReceiver) {
        this->midiReceiver = midiReceiver;
    }

/// \section Send Functions

    /// send a bang message
    virtual void sendBang(const std::string &dest) {
        PDBASE_SETINSTANCE
        libpd_bang(dest.c_str());
    }

    /// send a float
    virtual void sendFloat(const std::string &dest, float value) {
        PDBASE_SETINSTANCE
        libpd_float(dest.c_str(), value);
    }

    /// send a symbol
    virtual void sendSymbol(const std::string &dest,
                            const std::string &symbol) {
        PDBASE_SETINSTANCE
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
        if(bMsgInProgress) {
            std::cerr << "Pd: cannot start message, message in progress"
                      << std::endl;
            return;
        }
        PDBASE_SETINSTANCE
        if(libpd_start_message(maxMsgLen) == 0) {
            bMsgInProgress = true;
            msgType = MSG;
        }
    }

    /// add a float to the current compound list or message
    virtual void addFloat(const float num) {
        if(!bMsgInProgress) {
            std::cerr << "Pd: cannot add float, message not in progress"
                      << std::endl;
            return;
        }
        if(msgType != MSG) {
            std::cerr << "Pd: cannot add float, midi byte stream in progress"
                      << std::endl;
            return;
        }
        if(curMsgLen+1 >= maxMsgLen) {
            std::cerr << "Pd: cannot add float, max message len of "
                      << maxMsgLen << " reached" << std::endl;
            return;
        }
        PDBASE_SETINSTANCE
        libpd_add_float(num);
        curMsgLen++;
    }

    /// add a symbol to the current compound list or message
    virtual void addSymbol(const std::string &symbol) {
        if(!bMsgInProgress) {
            std::cerr << "Pd: cannot add symbol, message not in progress"
                      << std::endl;
            return;
        }
        if(msgType != MSG) {
            std::cerr << "Pd: cannot add symbol, midi byte stream in progress"
                      << std::endl;
            return;
        }
        if(curMsgLen+1 >= maxMsgLen) {
            std::cerr << "Pd: cannot add symbol, max message len of "
                      << maxMsgLen << " reached" << std::endl;
            return;
        }
        PDBASE_SETINSTANCE
        libpd_add_symbol(symbol.c_str());
        curMsgLen++;
    }

    /// finish and send as a list
    virtual void finishList(const std::string &dest) {
        if(!bMsgInProgress) {
            std::cerr << "Pd: cannot finish list, "
                      << "message not in progress" << std::endl;
            return;
        }
        if(msgType != MSG) {
            std::cerr << "Pd: cannot finish list, "
                      << "midi byte stream in progress" << std::endl;
            return;
        }
        PDBASE_SETINSTANCE
        libpd_finish_list(dest.c_str());
        bMsgInProgress = false;
        curMsgLen = 0;
    }

    /// finish and send as a list with a specific message name
    virtual void finishMessage(const std::string &dest,
                               const std::string &msg) {
        if(!bMsgInProgress) {
            std::cerr << "Pd: cannot finish message, "
                      << "message not in progress" << std::endl;
            return;
        }
        if(msgType != MSG) {
            std::cerr << "Pd: cannot finish message, "
                      << "midi byte stream in progress" << std::endl;
            return;
        }
        PDBASE_SETINSTANCE
        libpd_finish_message(dest.c_str(), msg.c_str());
        bMsgInProgress = false;
        curMsgLen = 0;
    }

    /// send a list using the pd::List type
    ///
    ///     pd::List list;
    ///     list.addSymbol("hello");
    ///     list.addFloat(1.23);
    ///     pd.sendList("test", list);
    ///
    /// sends [list hello 1.23( -> [r test]
    ///
    /// stream operators work as well:
    ///
    ///     list << "hello" << 1.23;
    ///     pd.sendList("test", list);
    ///
    virtual void sendList(const std::string &dest, const pd::List &list) {
        if(bMsgInProgress) {
            std::cerr << "Pd: cannot send list, message in progress"
                      << std::endl;
            return;
        }
        PDBASE_SETINSTANCE
        libpd_start_message(list.len());
        bMsgInProgress = true;
        // step through list
        for(int i = 0; i < (int)list.len(); ++i) {
            if(list.isFloat(i))
                addFloat(list.getFloat(i));
            else if(list.isSymbol(i))
                addSymbol(list.getSymbol(i));
        }
        finishList(dest);
    }

    /// send a message using the pd::List type
    ///
    ///     pd::List list;
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
    virtual void sendMessage(const std::string &dest,
                             const std::string &msg,
                             const pd::List &list = pd::List()) {
        if(bMsgInProgress) {
            std::cerr << "Pd: cannot send message, message in progress"
                      << std::endl;
            return;
        }
        PDBASE_SETINSTANCE
        libpd_start_message(list.len());
        bMsgInProgress = true;
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
/// * program value       0 - 127
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
        PDBASE_SETINSTANCE
        libpd_noteon(channel, pitch, velocity);
    }

    /// send a MIDI control change
    virtual void sendControlChange(const int channel,
                                   const int controller,
                                   const int value) {
        PDBASE_SETINSTANCE
        libpd_controlchange(channel, controller, value);
    }

    /// send a MIDI program change
    virtual void sendProgramChange(const int channel, const int value) {
        PDBASE_SETINSTANCE
        libpd_programchange(channel, value);
    }

    /// send a MIDI pitch bend
    ///
    /// in pd: [bendin] takes 0 - 16383 while [bendout] returns -8192 - 8192
    ///
    virtual void sendPitchBend(const int channel, const int value) {
        PDBASE_SETINSTANCE
        libpd_pitchbend(channel, value);
    }

    /// send a MIDI aftertouch
    virtual void sendAftertouch(const int channel, const int value) {
        PDBASE_SETINSTANCE
        libpd_aftertouch(channel, value);
    }

    /// send a MIDI poly aftertouch
    virtual void sendPolyAftertouch(const int channel,
                                    const int pitch,
                                    const int value) {
        PDBASE_SETINSTANCE
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
        PDBASE_SETINSTANCE
        libpd_midibyte(port, value);
    }

    /// send a raw MIDI sysex byte
    virtual void sendSysex(const int port, const int value) {
        PDBASE_SETINSTANCE
        libpd_sysex(port, value);
    }

    /// send a raw MIDI realtime byte
    virtual void sendSysRealTime(const int port, const int value) {
        PDBASE_SETINSTANCE
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
    PdBase& operator<<(const pd::Bang &var) {
        if(bMsgInProgress) {
            std::cerr << "Pd: cannot send Bang, message in progress"
                      << std::endl;
            return *this;
        }
        sendBang(var.dest.c_str());
        return *this;
    }

    /// send a float message
    PdBase& operator<<(const pd::Float &var) {
        if(bMsgInProgress) {
            std::cerr << "Pd: cannot send Float, message in progress"
                      << std::endl;
            return *this;
        }
        sendFloat(var.dest.c_str(), var.num);
        return *this;
    }

    /// send a symbol message
    PdBase& operator<<(const pd::Symbol &var) {
        if(bMsgInProgress) {
            std::cerr << "Pd: cannot send Symbol, message in progress"
                      << std::endl;
            return *this;
        }
        sendSymbol(var.dest.c_str(), var.symbol.c_str());
        return *this;
    }

/// \section Stream Interface for Compound Messages
///
/// pd << pd::StartMessage() << 100 << 1.2 << "a symbol" << pd::FinishList("test");
///

    /// start a compound message
    PdBase& operator<<(const pd::StartMessage &) {
        startMessage();
        return *this;
    }

    /// finish a compound message and send it as a list
    PdBase& operator<<(const pd::FinishList &var) {
        finishList(var.dest);
        return *this;
    }

    /// finish a compound message and send it as a message
    PdBase& operator<<(const pd::FinishMessage &var) {
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
        switch(msgType) {
            case MSG:
                addFloat((float) var);
                break;
            case MIDI:
                sendMidiByte(midiPort, var);
                break;
            case SYSEX:
                sendSysex(midiPort, var);
                break;
            case SYSRT:
                sendSysRealTime(midiPort, var);
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
    PdBase& operator<<(const char *var) {
        addSymbol((std::string)var);
        return *this;
    }

    // add a string as a symbol to the compound message
    PdBase& operator<<(const std::string &var) {
        addSymbol(var);
        return *this;
    }

/// \section Stream Interface for MIDI
///
/// pd << pd::NoteOn(64) << NoteOn(64, 60) << pd::NoteOn(64, 60, 1);
/// pd << pd::ControlChange(100, 64) << pd::ProgramChange(100, 1);
/// pd << pd::Aftertouch(127, 1) << pd::PolyAftertouch(64, 127, 1);
/// pd << pd::PitchBend(2000, 1);
///

    /// send a MIDI note on
    PdBase& operator<<(const pd::NoteOn &var) {
        sendNoteOn(var.channel, var.pitch, var.velocity);
        return *this;
    }

    /// send a MIDI control change
    PdBase& operator<<(const pd::ControlChange &var) {
        sendControlChange(var.channel, var.controller, var.value);
        return *this;
    }

    /// send a MIDI program change
    PdBase& operator<<(const pd::ProgramChange &var) {
        sendProgramChange(var.channel, var.value);
        return *this;
    }

    /// send a MIDI pitch bend
    PdBase& operator<<(const pd::PitchBend &var) {
        sendPitchBend(var.channel, var.value);
        return *this;
    }

    /// send a MIDI aftertouch
    PdBase& operator<<(const pd::Aftertouch &var) {
        sendAftertouch(var.channel, var.value);
        return *this;
    }

    /// send a MIDI poly aftertouch
    PdBase& operator<<(const pd::PolyAftertouch &var) {
        sendPolyAftertouch(var.channel, var.pitch, var.value);
        return *this;
    }

/// \section Stream Interface for Raw Bytes
///
/// pd << pd::StartMidi() << 0xEF << 0x45 << pd::Finish();
/// pd << pd::StartSysex() << 0xE7 << 0x45 << 0x56 << 0x17 << pd::Finish();
///

    /// start a raw byte MIDI message
    PdBase& operator<<(const pd::StartMidi &var) {
        if(bMsgInProgress) {
            std::cerr << "Pd: cannot start MidiByte stream, "
                      << "message in progress" << std::endl;
            return *this;
        }
        bMsgInProgress = true;
        msgType = MIDI;
        midiPort = var.port;
        return *this;
    }

    /// start a raw byte MIDI sysex message
    PdBase& operator<<(const pd::StartSysex &var) {
        if(bMsgInProgress) {
            std::cerr << "Pd: cannot start Sysex stream, "
                      << "message in progress" << std::endl;
            return *this;
        }
        bMsgInProgress = true;
        msgType = SYSEX;
        midiPort = var.port;
        return *this;
    }

    /// start a raw byte MIDI realtime message
    PdBase& operator<<(const pd::StartSysRealTime &var) {
        if(bMsgInProgress) {
            std::cerr << "Pd: cannot start SysRealRime stream, "
                      << "message in progress" << std::endl;
            return *this;
        }
        bMsgInProgress = true;
        msgType = SYSRT;
        midiPort = var.port;
        return *this;
    }

    /// finish and send a raw byte MIDI message
    PdBase& operator<<(const pd::Finish &) {
        if(!bMsgInProgress) {
            std::cerr << "Pd: cannot finish midi byte stream, "
                      << "stream not in progress" << std::endl;
            return *this;
        }
        if(msgType == MSG) {
            std::cerr << "Pd: cannot finish midi byte stream, "
                      << "message in progress" << std::endl;
            return *this;
        }
        bMsgInProgress = false;
        curMsgLen = 0;
        return *this;
    }

    /// is a message or byte stream currently in progress?
    bool isMessageInProgress() {
        return bMsgInProgress;
    }

/// \section Array Access

    /// get the size of a pd array
    /// returns 0 if array not found
    int arraySize(const std::string &name) {
        PDBASE_SETINSTANCE
        int len = libpd_arraysize(name.c_str());
        if(len < 0) {
            std::cerr << "Pd: cannot get size of unknown array \""
                      << name << "\"" << std::endl;
            return 0;
        }
        return len;
    }

    /// (re)size a pd array
    /// sizes <= 0 are clipped to 1
    /// returns true on success, false on failure
    bool resizeArray(const std::string &name, long size) {
        PDBASE_SETINSTANCE
        int ret = libpd_resize_array(name.c_str(), size);
        if(ret < 0) {
            std::cerr << "Pd: cannot resize unknown array \"" << name << "\""
                      << std::endl;
            return false;
        }
        return true;
    }

    /// read from a pd array
    ///
    /// resizes given vector to readLen, checks readLen and offset
    ///
    /// returns true on success, false on failure
    ///
    /// calling without setting readLen and offset reads the whole array:
    ///
    /// std::vector<float> array1;
    /// readArray("array1", array1);
    ///
    virtual bool readArray(const std::string &name,
                           std::vector<float> &dest,
                           int readLen=-1, int offset=0) {
        PDBASE_SETINSTANCE
        int len = libpd_arraysize(name.c_str());
        if(len < 0) {
            std::cerr << "Pd: cannot read unknown array \"" << name << "\""
                      << std::endl;
            return false;
        }
        // full array len?
        if(readLen < 0) {
            readLen = len;
        }
        // check read len
        else if(readLen > len) {
            std::cerr << "Pd: given read len " << readLen << " > len "
                      << len << " of array \"" << name << "\"" << std::endl;
            return false;
        }
        // check offset
        if(offset + readLen > len) {
            std::cerr << "Pd: given read len and offset > len " << readLen
                      << " of array \"" << name << "\"" << std::endl;
            return false;
        }
        // resize if necessary
        if(dest.size() != (std::size_t)readLen) {
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
    virtual bool writeArray(const std::string &name,
                            std::vector<float> &source,
                            int writeLen=-1, int offset=0) {
        PDBASE_SETINSTANCE
        int len = libpd_arraysize(name.c_str());
        if(len < 0) {
            std::cerr << "Pd: cannot write to unknown array \"" << name << "\""
                      << std::endl;
            return false;
        }

        // full array len?
        if(writeLen < 0) {
            writeLen = len;
        }

        // check write len
        else if(writeLen > len) {
            std::cerr << "Pd: given write len " << writeLen << " > len " << len
                      << " of array \"" << name << "\"" << std::endl;
            return false;
        }

        // check offset
        if(offset+writeLen > len) {
            std::cerr << "Pd: given write len and offset > len " << writeLen
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
    virtual void clearArray(const std::string &name, int value=0) {
        PDBASE_SETINSTANCE
        int len = libpd_arraysize(name.c_str());
        if(len < 0) {
            std::cerr << "Pd: cannot clear unknown array \""
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
        return bInited;
    }

    /// is the global pd instance using the ringbuffer queue
    /// for message padding?
    bool isQueued() {
        return bQueued;
    }

    /// get the blocksize of pd (sample length per channel)
    static int blockSize() {
        return libpd_blocksize();
    }

    /// set the max length of messages and lists, default: 32
    void setMaxMessageLen(unsigned int len) {
        maxMsgLen = len;
    }

    /// get the max length of messages and lists
    unsigned int maxMessageLen() {
        return maxMsgLen;
    }

    /// get the pd instance pointer
    /// returns main instance when libpd is not compiled with PDINSTANCE
    t_pdinstance *instancePtr() {
        #ifdef PDINSTANCE
            return instance;
        #endif
        return libpd_main_instance();
    }

    /// get the number of pd instances, including the main instance
    /// returns number or 1 when libpd is not compiled with PDINSTANCE
    static int numInstances() {
        return libpd_num_instances();
    }

protected:

    /// compound message status
    enum MsgType {
        MSG,
        MIDI,
        SYSEX,
        SYSRT
    };

/// \section Variables

    bool bMsgInProgress;    ///< is a compound message being constructed?
    int maxMsgLen;          ///< maximum allowed message length
    int curMsgLen;          ///< the length of the current message

    /// compound message status
    PdBase::MsgType msgType;

    int midiPort;   ///< target midi port

    std::map<std::string,void*> sources; ///< subscribed sources

    pd::PdReceiver *receiver;            ///< the message receiver
    pd::PdMidiReceiver *midiReceiver;    ///< the midi receiver

#ifdef PDINSTANCE
    t_pdinstance *instance; ///< instance pointer
#endif

protected:

    bool bInited; ///< is this pd instance inited?
    bool bQueued; ///< is this instance using the libpd_queued ringbuffer?

    // libpd static callback functions
    static void _print(const char *s) {
        PdBase *base = (PdBase *)libpd_get_instancedata();
        if(base->receiver) {
            base->receiver->print((std::string)s);
        }
    }

    static void _bang(const char *source) {
        PdBase *base = (PdBase *)libpd_get_instancedata();
        if(base->receiver) {
            base->receiver->receiveBang((std::string)source);
        }
    }

    static void _float(const char *source, float value) {
        PdBase *base = (PdBase *)libpd_get_instancedata();
        if(base->receiver) {
            base->receiver->receiveFloat((std::string)source, value);
        }
    }

    static void _symbol(const char *source, const char *symbol) {
        PdBase *base = (PdBase *)libpd_get_instancedata();
        if(base->receiver) {
            base->receiver->receiveSymbol((std::string)source,
                                          (std::string)symbol);
        }
    }

    static void _list(const char *source, int argc, t_atom *argv) {
        PdBase *base = (PdBase *)libpd_get_instancedata();
        pd::List list;
        for(int i = 0; i < argc; i++) {
            t_atom a = argv[i];
            if(a.a_type == A_FLOAT) {
                float f = a.a_w.w_float;
                list.addFloat(f);
            }
            else if(a.a_type == A_SYMBOL) {
                const char *s = a.a_w.w_symbol->s_name;
                list.addSymbol((std::string)s);
            }
        }
        if(base->receiver) {
            base->receiver->receiveList((std::string)source, list);
        }
    }

    static void _message(const char *source, const char *symbol,
                         int argc, t_atom *argv) {
        PdBase *base = (PdBase *)libpd_get_instancedata();
        pd::List list;
        for(int i = 0; i < argc; i++) {
            t_atom a = argv[i];
            if(a.a_type == A_FLOAT) {
                float f = a.a_w.w_float;
                list.addFloat(f);
            }
            else if(a.a_type == A_SYMBOL) {
                const char *s = a.a_w.w_symbol->s_name;
                list.addSymbol((std::string)s);
            }
        }
        if(base->receiver) {
            base->receiver->receiveMessage((std::string)source,
                                           (std::string)symbol, list);
        }
    }

    static void _noteon(int channel, int pitch, int velocity) {
        PdBase *base = (PdBase *)libpd_get_instancedata();
        if(base->midiReceiver) {
            base->midiReceiver->receiveNoteOn(channel, pitch, velocity);
        }
    }

    static void _controlchange(int channel, int controller, int value) {
        PdBase *base = (PdBase *)libpd_get_instancedata();
        if(base->midiReceiver) {
            base->midiReceiver->receiveControlChange(channel,
                                                     controller,
                                                     value);
        }
    }

    static void _programchange(int channel, int value) {
        PdBase *base = (PdBase *)libpd_get_instancedata();
        if(base->midiReceiver) {
            base->midiReceiver->receiveProgramChange(channel, value);
        }
    }

    static void _pitchbend(int channel, int value) {
        PdBase *base = (PdBase *)libpd_get_instancedata();
        if(base->midiReceiver) {
            base->midiReceiver->receivePitchBend(channel, value);
        }
    }

    static void _aftertouch(int channel, int value) {
        PdBase *base = (PdBase *)libpd_get_instancedata();
        if(base->midiReceiver) {
            base->midiReceiver->receiveAftertouch(channel, value);
        }
    }

    static void _polyaftertouch(int channel, int pitch, int value) {
        PdBase *base = (PdBase *)libpd_get_instancedata();
        if(base->midiReceiver) {
            base->midiReceiver->receivePolyAftertouch(channel,
                                                      pitch,
                                                      value);
        }
    }

    static void _midibyte(int port, int byte) {
        PdBase *base = (PdBase *)libpd_get_instancedata();
        if(base->midiReceiver) {
            base->midiReceiver->receiveMidiByte(port, byte);
        }
    }
};

} // namespace
