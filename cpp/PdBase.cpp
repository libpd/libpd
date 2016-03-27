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
#include "PdBase.hpp"
#include "z_libpd.h"
#include "z_queued.h"
#include "z_print_util.h"

#include <iostream>

#ifdef LIBPD_USE_STD_MUTEX
    #if __cplusplus <= 201103L // C++ 11 check
        #define _LOCK() mutex.lock()
        #define _UNLOCK() mutex.unlock()
    #endif
#else
    // no ops
    #define _LOCK()
    #define _UNLOCK()
#endif

// needed for libpd audio passing
#ifndef USEAPI_DUMMY
    #define USEAPI_DUMMY
#endif

using namespace std;

namespace pd {

//--------------------------------------------------------------------
PdBase::PdBase() {
    clear();
    PdContext::instance().addBase();
}

//--------------------------------------------------------------------
PdBase::~PdBase() {
    clear();
    PdContext::instance().removeBase();
}

//--------------------------------------------------------------------
bool PdBase::init(const int numInChannels, const int numOutChannels, const int sampleRate, bool queued) {
    _LOCK();
    PdContext::instance().clear();
    bool ret = PdContext::instance().init(numInChannels, numOutChannels, sampleRate, queued);
    _UNLOCK();
    return ret;
}

void PdBase::clear() {
    _LOCK();
    PdContext::instance().clear();
    _UNLOCK();
    unsubscribeAll();
}

//--------------------------------------------------------------------
void PdBase::addToSearchPath(const std::string& path) {
    _LOCK();
    libpd_add_to_search_path(path.c_str());
    _UNLOCK();
}

void PdBase::clearSearchPath() {
    _LOCK();
    libpd_clear_search_path();
    _UNLOCK();
}

//--------------------------------------------------------------------
Patch PdBase::openPatch(const std::string& patch, const std::string& path) {
    _LOCK();
    // [; pd open file folder(
    void* handle = libpd_openfile(patch.c_str(), path.c_str());
    if(handle == NULL) {
        return Patch(); // return empty Patch
    }
    int dollarZero = libpd_getdollarzero(handle);
    _UNLOCK();
    return Patch(handle, dollarZero, patch, path);
}

Patch PdBase::openPatch(pd::Patch& patch) {
    return openPatch(patch.filename(), patch.path());
}

void PdBase::closePatch(const std::string& patch) {
    // [; pd-name menuclose 1(
    string patchname = (string) "pd-"+patch;
    _LOCK();
    libpd_start_message(PdContext::instance().maxMsgLen);
    libpd_add_float(1.0f);
    libpd_finish_message(patchname.c_str(), "menuclose");
    _UNLOCK();
}

void PdBase::closePatch(Patch& patch) {
    if(!patch.isValid()) {
        return;
    }
    _LOCK();
    libpd_closefile(patch.handle());
    _UNLOCK();
    patch.clear();
}

//--------------------------------------------------------------------
bool PdBase::processRaw(const float* inBuffer, float* outBuffer) {
    _LOCK();
    bool ret = libpd_process_raw(inBuffer, outBuffer) == 0;
    _UNLOCK();
    return ret;
}

bool PdBase::processShort(int ticks, const short* inBuffer, short* outBuffer) {
    _LOCK();
    bool ret = libpd_process_short(ticks, inBuffer, outBuffer) == 0;
    _UNLOCK();
    return ret;
}

bool PdBase::processFloat(int ticks, const float* inBuffer, float* outBuffer) {
    _LOCK();
    bool ret = libpd_process_float(ticks, inBuffer, outBuffer) == 0;
    _UNLOCK();
    return ret;
}

bool PdBase::processDouble(int ticks, const double* inBuffer, double* outBuffer) {
    _LOCK();
    bool ret = libpd_process_double(ticks, inBuffer, outBuffer) == 0;
    _UNLOCK();
    return ret;
}


//--------------------------------------------------------------------
void PdBase::computeAudio(bool state) {
    _LOCK();
    PdContext::instance().computeAudio(state);
    _UNLOCK();
}

//----------------------------------------------------------
void PdBase::subscribe(const std::string& source) {

    if(exists(source)) {
        cerr << "Pd: unsubscribe: ignoring duplicate source" << endl;
        return;
    }

    _LOCK();
    void* pointer = libpd_bind(source.c_str());
    _UNLOCK();
    if(pointer != NULL) {
        map<string,void*>& sources = PdContext::instance().sources;
        sources.insert(pair<string,void*>(source, pointer));
    }
}

void PdBase::unsubscribe(const std::string& source) {

    map<string,void*>& sources = PdContext::instance().sources;

    map<string,void*>::iterator iter;
    iter = sources.find(source);
    if(iter == sources.end()) {
        cerr << "Pd: unsubscribe: ignoring unknown source" << endl;
        return;
    }

    _LOCK();
    libpd_unbind(iter->second);
    _UNLOCK();
    sources.erase(iter);
}

bool PdBase::exists(const std::string& source) {
    map<string,void*>& sources = PdContext::instance().sources;
    if(sources.find(source) != sources.end()) {
        return true;
    }
    return false;
}

void PdBase::unsubscribeAll(){
    map<string,void*>& sources = PdContext::instance().sources;
    map<string,void*>::iterator iter;
    _LOCK();
    for(iter = sources.begin(); iter != sources.end(); ++iter) {
        libpd_unbind(iter->second);
    }
    _UNLOCK();
    sources.clear();
}

//--------------------------------------------------------------------
void PdBase::receiveMessages() {
    libpd_queued_receive_pd_messages();
}

void PdBase::receiveMidi() {
    libpd_queued_receive_midi_messages();
}

//--------------------------------------------------------------------
void PdBase::setReceiver(PdReceiver* receiver) {
    _LOCK();
    PdContext::instance().receiver = receiver;
    _UNLOCK();
}

void PdBase::setMidiReceiver(PdMidiReceiver* midiReceiver) {
    _LOCK();
    PdContext::instance().midiReceiver = midiReceiver;
    _UNLOCK();
}

//----------------------------------------------------------
void PdBase::sendBang(const std::string& dest) {
    _LOCK();
    libpd_bang(dest.c_str());
    _UNLOCK();
}

void PdBase::sendFloat(const std::string& dest, float value) {
    _LOCK();
    libpd_float(dest.c_str(), value);
    _UNLOCK();
}

void PdBase::sendSymbol(const std::string& dest, const std::string& symbol) {
    _LOCK();
    libpd_symbol(dest.c_str(), symbol.c_str());
    _UNLOCK();
}

//----------------------------------------------------------
void PdBase::startMessage() {

    PdContext& context = PdContext::instance();

    if(context.bMsgInProgress) {
        cerr << "Pd: Can not start message, message in progress" << endl;
        return;
    }

    _LOCK();
    if(libpd_start_message(context.maxMsgLen) == 0) {
        context.bMsgInProgress = true;
        context.msgType = MSG;
    }
    _UNLOCK();
}

void PdBase::addFloat(const float num) {

    PdContext& context = PdContext::instance();

    if(!context.bMsgInProgress) {
        cerr << "Pd: Can not add float, message not in progress" << endl;
        return;
    }

    if(context.msgType != MSG) {
        cerr << "Pd: Can not add float, midi byte stream in progress" << endl;
        return;
    }

    if(context.curMsgLen+1 >= context.maxMsgLen) {
        cerr << "Pd: Can not add float, max message len of " << context.maxMsgLen << " reached" << endl;
        return;
    }

    _LOCK();
    libpd_add_float(num);
    _UNLOCK();
    
    context.curMsgLen++;
}

void PdBase::addSymbol(const std::string& symbol) {

    PdContext& context = PdContext::instance();

    if(!context.bMsgInProgress) {
        cerr << "Pd: Can not add symbol, message not in progress" << endl;;
        return;
    }

    if(context.msgType != MSG) {
        cerr << "Pd: Can not add symbol, midi byte stream in progress" << endl;;
        return;
    }

    if(context.curMsgLen+1 >= context.maxMsgLen) {
        cerr << "Pd: Can not add symbol, max message len of " << context.maxMsgLen << " reached" << endl;
        return;
    }

    _LOCK();
    libpd_add_symbol(symbol.c_str());
    _UNLOCK();
    
    context.curMsgLen++;
}

void PdBase::finishList(const std::string& dest) {

    PdContext& context = PdContext::instance();

    if(!context.bMsgInProgress) {
        cerr << "Pd: Can not finish list, message not in progress" << endl;
        return;
    }

    if(context.msgType != MSG) {
        cerr << "Pd: Can not finish list, midi byte stream in progress" << endl;
        return;
    }

    _LOCK();
    libpd_finish_list(dest.c_str());
    _UNLOCK();

    context.bMsgInProgress = false;
    context.curMsgLen = 0;
}

void PdBase::finishMessage(const std::string& dest, const std::string& msg) {

    PdContext& context = PdContext::instance();

    if(!context.bMsgInProgress) {
        cerr << "Pd: Can not finish message, message not in progress" << endl;
        return;
    }

    if(context.msgType != MSG) {
        cerr << "Pd: Can not finish message, midi byte stream in progress" << endl;
        return;
    }

    _LOCK();
    libpd_finish_message(dest.c_str(), msg.c_str());
    _UNLOCK();

    context.bMsgInProgress = false;
    context.curMsgLen = 0;
}

//----------------------------------------------------------
void PdBase::sendList(const std::string& dest, const List& list) {

    PdContext& context = PdContext::instance();

    if(context.bMsgInProgress) {
        cerr << "Pd: Can not send list, message in progress" << endl;
        return;
    }

    _LOCK();
    libpd_start_message(list.len());
    _UNLOCK();

    context.bMsgInProgress = true;

    // step through list
    for(int i = 0; i < list.len(); ++i) {
        if(list.isFloat(i))
            addFloat(list.getFloat(i));
        else if(list.isSymbol(i))
            addSymbol(list.getSymbol(i));
    }

    finishList(dest);
}

void PdBase::sendMessage(const std::string& dest, const std::string& msg, const List& list) {

    PdContext& context = PdContext::instance();

    if(context.bMsgInProgress) {
        cerr << "Pd: Can not send message, message in progress" << endl;
        return;
    }

    _LOCK();
    libpd_start_message(list.len());
    _UNLOCK();

    context.bMsgInProgress = true;

    // step through list
    for(int i = 0; i < list.len(); ++i) {
        if(list.isFloat(i))
            addFloat(list.getFloat(i));
        else if(list.isSymbol(i))
            addSymbol(list.getSymbol(i));
    }

    finishMessage(dest, msg);
}

//----------------------------------------------------------
void PdBase::sendNoteOn(const int channel, const int pitch, const int velocity) {
    _LOCK();
    libpd_noteon(channel, pitch, velocity);
    _UNLOCK();
}

void PdBase::sendControlChange(const int channel, const int controller, const int value) {
    _LOCK();
    libpd_controlchange(channel, controller, value);
    _UNLOCK();
}

void PdBase::sendProgramChange(const int channel, int program) {
    _LOCK();
    libpd_programchange(channel, program);
    _UNLOCK();
}

void PdBase::sendPitchBend(const int channel, const int value) {
    _LOCK();
    libpd_pitchbend(channel, value);
    _UNLOCK();
}

void PdBase::sendAftertouch(const int channel, const int value) {
    _LOCK();
    libpd_aftertouch(channel, value);
    _UNLOCK();
}

void PdBase::sendPolyAftertouch(const int channel, int pitch, int value) {
    _LOCK();
    libpd_polyaftertouch(channel, pitch, value);
    _UNLOCK();
}

//----------------------------------------------------------
void PdBase::sendMidiByte(const int port, const int value) {
    _LOCK();
    libpd_midibyte(port, value);
    _UNLOCK();
}

void PdBase::sendSysex(const int port, const int value) {
    _LOCK();
    libpd_sysex(port, value);
    _UNLOCK();
}

void PdBase::sendSysRealTime(const int port, const int value) {
    _LOCK();
    libpd_sysrealtime(port, value);
    _UNLOCK();
}

//----------------------------------------------------------
PdBase& PdBase::operator<<(const Bang& var) {

    if(PdContext::instance().bMsgInProgress) {
        cerr << "Pd: Can not send Bang, message in progress" << endl;
        return *this;
    }

    sendBang(var.dest.c_str());

    return *this;
}

PdBase& PdBase::operator<<(const Float& var) {

    if(PdContext::instance().bMsgInProgress) {
        cerr << "Pd: Can not send Float, message in progress" << endl;
        return *this;
    }

    sendFloat(var.dest.c_str(), var.num);

    return *this;
}

PdBase& PdBase::operator<<(const Symbol& var) {

    if(PdContext::instance().bMsgInProgress) {
        cerr << "Pd: Can not send Symbol, message in progress" << endl;
        return *this;
    }

    sendSymbol(var.dest.c_str(), var.symbol.c_str());

    return *this;
}

//----------------------------------------------------------
PdBase& PdBase::operator<<(const StartMessage& var) {
    startMessage();
    return *this;
}

PdBase& PdBase::operator<<(const FinishList& var) {
    finishList(var.dest);
    return *this;
}

PdBase& PdBase::operator<<(const FinishMessage& var) {
    finishMessage(var.dest, var.msg);
    return *this;
}

//----------------------------------------------------------
PdBase& PdBase::operator<<(const bool var) {
    addFloat((float) var);
    return *this;
}

PdBase& PdBase::operator<<(const int var) {

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

PdBase& PdBase::operator<<(const float var) {
    addFloat((float) var);
    return *this;
}

PdBase& PdBase::operator<<(const double var) {
    addFloat((float) var);
    return *this;
}

//----------------------------------------------------------
PdBase& PdBase::operator<<(const char var) {
    string s;
    s = var;
    addSymbol(s);
    return *this;
}

PdBase& PdBase::operator<<(const char* var) {
    addSymbol((string) var);
    return *this;
}

PdBase& PdBase::operator<<(const std::string& var) {
    addSymbol(var);
    return *this;
}

//----------------------------------------------------------
PdBase& PdBase::operator<<(const NoteOn& var) {
    sendNoteOn(var.channel, var.pitch, var.velocity);
    return *this;
}

PdBase& PdBase::operator<<(const ControlChange& var) {
    sendControlChange(var.channel, var.controller, var.value);
    return *this;
}

PdBase& PdBase::operator<<(const ProgramChange& var) {
    sendProgramChange(var.channel, var.value);
    return *this;
}

PdBase& PdBase::operator<<(const PitchBend& var) {
    sendPitchBend(var.channel, var.value);
    return *this;
}

PdBase& PdBase::operator<<(const Aftertouch& var) {
    sendAftertouch(var.channel, var.value);
    return *this;
}

PdBase& PdBase::operator<<(const PolyAftertouch& var) {
    sendPolyAftertouch(var.channel, var.pitch, var.value);
    return *this;
}

//----------------------------------------------------------
PdBase& PdBase::operator<<(const StartMidi& var) {

    PdContext& context = PdContext::instance();

    if(context.bMsgInProgress) {
        cerr << "Pd: Can not start MidiByte stream, message in progress" << endl;
        return *this;
    }

    context.bMsgInProgress = true;
    context.msgType = MIDI;
    context.midiPort = var.port;

    return *this;
}

PdBase& PdBase::operator<<(const StartSysex& var) {

    PdContext& context = PdContext::instance();

    if(context.bMsgInProgress) {
        cerr << "Pd: Can not start Sysex stream, message in progress" << endl;
        return *this;
    }

    context.bMsgInProgress = true;
    context.msgType = SYSEX;
    context.midiPort = var.port;

    return *this;
}

PdBase& PdBase::operator<<(const StartSysRealTime& var) {

    PdContext& context = PdContext::instance();

    if(context.bMsgInProgress) {
        cerr << "Pd: Can not start SysRealRime stream, message in progress" << endl;
        return *this;
    }

    context.bMsgInProgress = true;
    context.msgType = SYSRT;
    context.midiPort = var.port;

    return *this;
}

PdBase& PdBase::operator<<(const Finish& var) {

    PdContext& context = PdContext::instance();

    if(!context.bMsgInProgress) {
        cerr << "Pd: Can not finish midi byte stream, stream not in progress" << endl;
        return *this;
    }

    if(context.msgType == MSG) {
        cerr << "Pd: Can not finish midi byte stream, message in progress" << endl;
        return *this;
    }

    context.bMsgInProgress = false;
    context.curMsgLen = 0;
    return *this;
}

bool PdBase::isMessageInProgress() {
    return PdContext::instance().bMsgInProgress;
}

//----------------------------------------------------------
int PdBase::arraySize(const std::string& arrayName) {
    _LOCK();
    int len = libpd_arraysize(arrayName.c_str());;
    _UNLOCK();
    if(len < 0) {
        cerr << "Pd: Cannot get size of unknown array \"" << arrayName << "\"" << endl;
        return 0;
    }
    return len;
}

bool PdBase::readArray(const std::string& arrayName, std::vector<float>& dest, int readLen, int offset) {

    _LOCK();
    int arrayLen = libpd_arraysize(arrayName.c_str());
    _UNLOCK();
    if(arrayLen < 0) {
        cerr << "Pd: Cannot read unknown array \"" << arrayName << "\"" << endl;
        return false;
    }

    // full array len?
    if(readLen < 0) {
        readLen = arrayLen;
    }
    // check read len
    else if(readLen > arrayLen) {
        cerr << "Pd: Given read len " << readLen << " > len "
             << arrayLen << " of array \"" << arrayName << "\"" << endl;
        return false;
    }

    // check offset
    if(offset+readLen > arrayLen) {
        cerr << "Pd: Given read len and offset > len " << readLen
             << " of array \"" << arrayName << "\"" << endl;
        return false;
    }

    // resize if necessary
    if(dest.size() != readLen) {
        dest.resize(readLen, 0);
    }

    _LOCK();
    if(libpd_read_array(&dest[0], arrayName.c_str(), offset, readLen) < 0) {
        cerr << "Pd: libpd_read_array failed for array \""
             << arrayName << "\"" << endl;
        _UNLOCK();
        return false;
    }
    _UNLOCK();
    return true;
}

bool PdBase::writeArray(const std::string& arrayName, std::vector<float>& source, int writeLen, int offset) {

    _LOCK();
    int arrayLen = libpd_arraysize(arrayName.c_str());
    _UNLOCK();
    if(arrayLen < 0) {
        cerr << "Pd: Cannot write to unknown array \"" << arrayName << "\"" << endl;
        return false;
    }

    // full array len?
    if(writeLen < 0) {
        writeLen = arrayLen;
    }
    // check write len
    else if(writeLen > arrayLen) {
        cerr << "Pd: Given write len " << writeLen << " > len " << arrayLen
             << " of array \"" << arrayName << "\"" << endl;
        return false;
    }

    // check offset
    if(offset+writeLen > arrayLen) {
        cerr << "Pd: Given write len and offset > len " << writeLen
             << " of array \"" << arrayName << "\"" << endl;
        return false;
    }

    _LOCK();
    if(libpd_write_array(arrayName.c_str(), offset, &source[0], writeLen) < 0) {
        cerr << "Pd: libpd_write_array failed for array \"" << arrayName << "\"" << endl;
        _UNLOCK();
        return false;
    }
    _UNLOCK();
    return true;
}

void PdBase::clearArray(const std::string& arrayName, int value) {

    _LOCK();
    int arrayLen = libpd_arraysize(arrayName.c_str());
    _UNLOCK();
    if(arrayLen < 0) {
        cerr << "Pd: Cannot clear unknown array \"" << arrayName << "\"" << endl;
        return;
    }

    std::vector<float> array;
    array.resize(arrayLen, value);

    _LOCK();
    if(libpd_write_array(arrayName.c_str(), 0, &array[0], arrayLen) < 0) {
        cerr << "Pd: libpd_write_array failed while clearing array \""
             << arrayName << "\"" << endl;
    }
    _UNLOCK();
}

//----------------------------------------------------------
bool PdBase::isInited() {
    return PdContext::instance().isInited();
}

bool PdBase::isQueued() {
    return PdContext::instance().isQueued();
}

int PdBase::blockSize() {
    // shouldn't need to lock this for now, it's always 64
    return libpd_blocksize();
}

void PdBase::setMaxMessageLen(unsigned int len) {
    PdContext::instance().maxMsgLen = len;
}

unsigned int PdBase::maxMessageLen() {
    return PdContext::instance().maxMsgLen;
}

/* ***** PD CONTEXT ***** */

//----------------------------------------------------------
PdBase::PdContext& PdBase::PdContext::instance() {
    static PdBase::PdContext * pointerToTheSingletonInstance = new PdContext;
    return *pointerToTheSingletonInstance;
}

void PdBase::PdContext::addBase() {
    numBases++;
}

void PdBase::PdContext::removeBase() {
    if(numBases > 0) {
        numBases--;
    }
    else if(bInited) { // double check clear
        clear();
    }
}

/// init the pd instance
bool PdBase::PdContext::init(const int numInChannels, const int numOutChannels, const int sampleRate, bool queued) {

    bQueued = queued;

    // attach callbacks
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

void PdBase::PdContext::clear() {

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

void PdBase::PdContext::computeAudio(bool state) {
    // [; pd dsp $1(
    libpd_start_message(1);
    libpd_add_float((float) state);
    libpd_finish_message("pd", "dsp");
}

/* ***** PD CONTEXT PRIVATE ***** */

//----------------------------------------------------------
PdBase::PdContext::PdContext() {
    bLibPdInited = false;
    bInited = false;
    bQueued = false;
    numBases = false;
    receiver = NULL;
    midiReceiver = NULL;
    clear();
    maxMsgLen = 32;
}

PdBase::PdContext::~PdContext() {
    if(bInited) {
        clear(); // triple check clear
    }
}

//----------------------------------------------------------
void PdBase::PdContext::_print(const char* s) {
    PdContext& context = PdContext::instance();
    if(context.receiver) {
        context.receiver->print((string) s);
    }
}

void PdBase::PdContext::_bang(const char* source) {
    PdContext& context = PdContext::instance();
    if(context.receiver) {
        context.receiver->receiveBang((string) source);
    }
}

void PdBase::PdContext::_float(const char* source, float num) {
    PdContext& context = PdContext::instance();
    if(context.receiver) {
        context.receiver->receiveFloat((string) source, num);
    }
}

void PdBase::PdContext::_symbol(const char* source, const char* symbol) {
    PdContext& context = PdContext::instance();
    if(context.receiver) {
        context.receiver->receiveSymbol((string) source, (string) symbol);
    }
}

void PdBase::PdContext::_list(const char* source, int argc, t_atom* argv) {
    PdContext& context = PdContext::instance();

    List list;
    for(int i = 0; i < argc; i++) {

        t_atom a = argv[i];

        if(a.a_type == A_FLOAT) {
            float f = a.a_w.w_float;
            list.addFloat(f);
        }
        else if(a.a_type == A_SYMBOL) {
            char* s = a.a_w.w_symbol->s_name;
            list.addSymbol((string) s);
        }
    }

    if(context.receiver) {
        context.receiver->receiveList((string) source, list);
    }
}

void PdBase::PdContext::_message(const char* source, const char *symbol, int argc, t_atom *argv)
{
    PdContext& context = PdContext::instance();

    List list;
    for(int i = 0; i < argc; i++) {

        t_atom a = argv[i];

        if(a.a_type == A_FLOAT) {
            float f = a.a_w.w_float;
            list.addFloat(f);
        }
        else if(a.a_type == A_SYMBOL) {
            char* s = a.a_w.w_symbol->s_name;
            list.addSymbol((string) s);
        }
    }

    if(context.receiver) {
        context.receiver->receiveMessage((string) source, (string) symbol, list);
    }
}

void PdBase::PdContext::_noteon(int channel, int pitch, int velocity) {
    PdContext& context = PdContext::instance();
    if(context.midiReceiver) {
        context.midiReceiver->receiveNoteOn(channel, pitch, velocity);
    }
}

void PdBase::PdContext::_controlchange(int channel, int controller, int value) {
    PdContext& context = PdContext::instance();
    if(context.midiReceiver) {
        context.midiReceiver->receiveControlChange(channel, controller, value);
    }
}

void PdBase::PdContext::_programchange(int channel, int value) {
    PdContext& context = PdContext::instance();
    if(context.midiReceiver) {
        context.midiReceiver->receiveProgramChange(channel, value);
    }
}

void PdBase::PdContext::_pitchbend(int channel, int value) {
    PdContext& context = PdContext::instance();
    if(context.midiReceiver) {
        context.midiReceiver->receivePitchBend(channel, value);
    }
}

void PdBase::PdContext::_aftertouch(int channel, int value) {
    PdContext& context = PdContext::instance();
    if(context.midiReceiver) {
        context.midiReceiver->receiveAftertouch(channel, value);
    }
}

void PdBase::PdContext::_polyaftertouch(int channel, int pitch, int value) {
    PdContext& context = PdContext::instance();
    if(context.midiReceiver) {
        context.midiReceiver->receivePolyAftertouch(channel, pitch, value);
    }
}

void PdBase::PdContext::_midibyte(int port, int byte) {
    PdContext& context = PdContext::instance();
    if(context.midiReceiver) {
        context.midiReceiver->receiveMidiByte(port, byte);
    }
}

} // namespace
