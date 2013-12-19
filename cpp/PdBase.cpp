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
#include "z_print_util.h"

#include <iostream>

// needed for libpd audio passing
#ifndef USEAPI_DUMMY
    #define USEAPI_DUMMY
#endif

using namespace std;

namespace pd {

//--------------------------------------------------------------------
PdBase::PdBase() {
    PdContext::instance().addBase();
}

//--------------------------------------------------------------------
PdBase::~PdBase() {
    clear();
    PdContext::instance().removeBase();
}

//--------------------------------------------------------------------
bool PdBase::init(const int numInChannels, const int numOutChannels, const int sampleRate) {
    clear();
    return PdContext::instance().init(numInChannels, numOutChannels, sampleRate);
}

void PdBase::clear() {
    PdContext::instance().clear();
    unsubscribeAll();
}

//--------------------------------------------------------------------
void PdBase::addToSearchPath(const std::string& path) {
    libpd_add_to_search_path(path.c_str());
}

void PdBase::clearSearchPath() {
    libpd_clear_search_path();
}

//--------------------------------------------------------------------
Patch PdBase::openPatch(const std::string& patch, const std::string& path) {
    // [; pd open file folder(
    void* handle = libpd_openfile(patch.c_str(), path.c_str());
    if(handle == NULL) {
        return Patch(); // return empty Patch
    }
    int dollarZero = libpd_getdollarzero(handle);
    return Patch(handle, dollarZero, patch, path);
}

Patch PdBase::openPatch(pd::Patch& patch) {
    return openPatch(patch.filename(), patch.path());
}

void PdBase::closePatch(const std::string& patch) {
    // [; pd-name menuclose 1(
    string patchname = (string) "pd-"+patch;
    libpd_start_message(PdContext::instance().maxMsgLen);
    libpd_add_float(1.0f);
    libpd_finish_message(patchname.c_str(), "menuclose");
}

void PdBase::closePatch(Patch& patch) {
    if(!patch.isValid()) {
        return;
    }
    libpd_closefile(patch.handle());
    patch.clear();
}

//--------------------------------------------------------------------
bool PdBase::processRaw(const float* inBuffer, float* outBuffer) {
    return libpd_process_raw(inBuffer, outBuffer) == 0;
}

bool PdBase::processShort(int ticks, const short* inBuffer, short* outBuffer) {
    return libpd_process_short(ticks, inBuffer, outBuffer) == 0;
}

bool PdBase::processFloat(int ticks, const float* inBuffer, float* outBuffer) {
    return libpd_process_float(ticks, inBuffer, outBuffer) == 0;
}

bool PdBase::processDouble(int ticks, const double* inBuffer, double* outBuffer) {
    return libpd_process_double(ticks, inBuffer, outBuffer) == 0;
}


//--------------------------------------------------------------------
void PdBase::computeAudio(bool state) {
    PdContext::instance().computeAudio(state);
}

//----------------------------------------------------------
void PdBase::subscribe(const std::string& source) {

    if(exists(source)) {
        cerr << "Pd: unsubscribe: ignoring duplicate source" << endl;
        return;
    }

    void* pointer = libpd_bind(source.c_str());
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

    libpd_unbind(iter->second);
    sources.erase(iter);
}

bool PdBase::exists(const std::string& source) {
    map<string,void*>& sources = PdContext::instance().sources;
    if(sources.find(source) != sources.end())
        return true;
    return false;
}

void PdBase::unsubscribeAll(){
    map<string,void*>& sources = PdContext::instance().sources;
    map<string,void*>::iterator iter;
    for(iter = sources.begin(); iter != sources.end(); ++iter)
        libpd_unbind(iter->second);
    sources.clear();
}

//--------------------------------------------------------------------
int PdBase::numMessages() {
    return (int) PdContext::instance().messages.size();
}

Message& PdBase::nextMessage() {

    PdContext& context = PdContext::instance();

    if(context.messages.size() > 0) {
        context.message = context.messages.front();
        context.messages.pop_front();
    }
    else {
        if(context.message.type != NONE) {
            context.message.clear();
        }
    }

    return context.message;
}

void PdBase::clearMessages() {
    PdContext::instance().messages.clear();
}

//--------------------------------------------------------------------
void PdBase::setReceiver(PdReceiver* receiver) {
    PdContext::instance().receiver = receiver;
}

void PdBase::setMidiReceiver(PdMidiReceiver* midiReceiver) {
    PdContext::instance().midiReceiver = midiReceiver;
}

//----------------------------------------------------------
void PdBase::sendBang(const std::string& dest) {
    libpd_bang(dest.c_str());
}

void PdBase::sendFloat(const std::string& dest, float value) {
    libpd_float(dest.c_str(), value);
}

void PdBase::sendSymbol(const std::string& dest, const std::string& symbol) {
    libpd_symbol(dest.c_str(), symbol.c_str());
}

//----------------------------------------------------------
void PdBase::startMessage() {

    PdContext& context = PdContext::instance();

    if(context.bMsgInProgress) {
        cerr << "Pd: Can not start message, message in progress" << endl;
        return;
    }

    if(libpd_start_message(context.maxMsgLen) == 0) {
		context.bMsgInProgress = true;
        context.msgType = MSG;
    }
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

    libpd_add_float(num);
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

    libpd_add_symbol(symbol.c_str());
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

    libpd_finish_list(dest.c_str());

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

    libpd_finish_message(dest.c_str(), msg.c_str());

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

    libpd_start_message(list.len());

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

    libpd_start_message(list.len());

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
    libpd_noteon(channel, pitch, velocity);
}

void PdBase::sendControlChange(const int channel, const int controller, const int value) {
    libpd_controlchange(channel, controller, value);
}

void PdBase::sendProgramChange(const int channel, int program) {
    libpd_programchange(channel, program);
}

void PdBase::sendPitchBend(const int channel, const int value) {
    libpd_pitchbend(channel, value);
}

void PdBase::sendAftertouch(const int channel, const int value) {
    libpd_aftertouch(channel, value);
}

void PdBase::sendPolyAftertouch(const int channel, int pitch, int value) {
    libpd_polyaftertouch(channel, pitch, value);
}

//----------------------------------------------------------
void PdBase::sendMidiByte(const int port, const int value) {
    libpd_midibyte(port, value);
}

void PdBase::sendSysex(const int port, const int value) {
    libpd_sysex(port, value);
}

void PdBase::sendSysRealTime(const int port, const int value) {
    libpd_sysrealtime(port, value);
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
    int len = libpd_arraysize(arrayName.c_str());;
    if(len < 0) {
        cerr << "Pd: Cannot get size of unknown array \"" << arrayName << "\"" << endl;
        return 0;
    }
    return len;
}

bool PdBase::readArray(const std::string& arrayName, std::vector<float>& dest, int readLen, int offset) {

    int arrayLen = libpd_arraysize(arrayName.c_str());
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

    if(libpd_read_array(&dest[0], arrayName.c_str(), offset, readLen) < 0) {
        cerr << "Pd: libpd_read_array failed for array \""
             << arrayName << "\"" << endl;
        return false;
    }
    return true;
}

bool PdBase::writeArray(const std::string& arrayName, std::vector<float>& source, int writeLen, int offset) {

    int arrayLen = libpd_arraysize(arrayName.c_str());
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

    if(libpd_write_array(arrayName.c_str(), offset, &source[0], writeLen) < 0) {
        cerr << "Pd: libpd_write_array failed for array \"" << arrayName << "\"" << endl;
        return false;
    }
    return true;
}

void PdBase::clearArray(const std::string& arrayName, int value) {

    int arrayLen = libpd_arraysize(arrayName.c_str());
    if(arrayLen < 0) {
        cerr << "Pd: Cannot clear unknown array \"" << arrayName << "\"" << endl;
        return;
    }

    std::vector<float> array;
    array.resize(arrayLen, value);

    if(libpd_write_array(arrayName.c_str(), 0, &array[0], arrayLen) < 0) {
        cerr << "Pd: libpd_write_array failed while clearing array \""
             << arrayName << "\"" << endl;
    }
}

//----------------------------------------------------------
bool PdBase::isInited() {
    return PdContext::instance().isInited();
}

int PdBase::blockSize() {
    return libpd_blocksize();
}

void PdBase::setMaxMessageLen(unsigned int len) {
    PdContext::instance().maxMsgLen = len;
}

unsigned int PdBase::maxMessageLen() {
    return PdContext::instance().maxMsgLen;
}

void PdBase::setMaxQueueLen(unsigned int len) {
    PdContext::instance().maxQueueLen = len;
}

unsigned int PdBase::maxQueueLen() {
    return PdContext::instance().maxQueueLen;
}

/* ***** PD CONTEXT ***** */

//----------------------------------------------------------
PdBase::PdContext& PdBase::PdContext::instance()
{
    static PdBase::PdContext * pointerToTheSingletonInstance = new PdContext;
    return *pointerToTheSingletonInstance;
}

void PdBase::PdContext::addBase() {
    numBases++;
}

void PdBase::PdContext::removeBase() {
    if(numBases > 0)
        numBases--;
    else if(bInited)    // double check clear
        clear();
}

/// init the pd instance
bool PdBase::PdContext::init(const int numInChannels, const int numOutChannels, const int sampleRate) {

    // attach callbacks
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
	if(!bLibPDInited) {
		libpd_init();
		bLibPDInited = true;
	}
	// init audio
    if(libpd_init_audio(numInChannels, numOutChannels, sampleRate) != 0) {
        return false;
    }
    bInited = true;

    messages.clear();

    return bInited;
}

void PdBase::PdContext::clear() {

    // detach callbacks
    if(bInited) {

        computeAudio(false);

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

    messages.clear();

    bInited = false;

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

void PdBase::PdContext::addMessage(pd::Message& msg) {
    if(messages.size() >= maxQueueLen) {
        cerr << "Pd: message queue max len of " << maxQueueLen
             << " reached, dropping oldest message" << endl;
        messages.pop_front();
    }
    messages.push_back(msg);
}

/* ***** PD CONTEXT PRIVATE ***** */

//----------------------------------------------------------
PdBase::PdContext::PdContext() {
    receiver = NULL;
    midiReceiver = NULL;
    clear();
    maxMsgLen = 32;

	bLibPDInited = false;
    bInited = false;
    numBases = false;

    maxQueueLen = 1000;
}

PdBase::PdContext::~PdContext() {
    if(bInited)
        clear();    // triple check clear
}

//----------------------------------------------------------
void PdBase::PdContext::_print(const char* s) {
    PdContext& context = PdContext::instance();
	if(context.receiver)
		context.receiver->print((string) s);
	else {
		Message m(PRINT);
		m.symbol = (string) s;
		context.messages.push_back(m);
	}
}

void PdBase::PdContext::_bang(const char* source) {
    PdContext& context = PdContext::instance();
    if(context.receiver)
        context.receiver->receiveBang((string) source);
    else {
        Message m(BANG);
        m.dest = (string) source;
        context.addMessage(m);
    }
}

void PdBase::PdContext::_float(const char* source, float num)
{
    PdContext& context = PdContext::instance();
    if(context.receiver)
        context.receiver->receiveFloat((string) source, num);
    else {
        Message m(FLOAT);
        m.dest = (string) source;
        m.num = num;
        context.addMessage(m);
    }
}

void PdBase::PdContext::_symbol(const char* source, const char* symbol)
{
    PdContext& context = PdContext::instance();
    if(context.receiver)
        context.receiver->receiveSymbol((string) source, (string) symbol);
    else {
        Message m(SYMBOL);
        m.dest = (string) source;
        m.symbol = (string) symbol;
        context.addMessage(m);
    }
}

void PdBase::PdContext::_list(const char* source, int argc, t_atom* argv)
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
        context.receiver->receiveList((string) source, list);
    }
    else {
        Message m(LIST);
        m.dest = (string) source;
        m.list = list;
        context.addMessage(m);
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
    else {
        Message m(MESSAGE);
        m.dest = (string) source;
        m.symbol = (string) symbol;
        m.list = list;
        context.addMessage(m);
    }
}

void PdBase::PdContext::_noteon(int channel, int pitch, int velocity) {
    PdContext& context = PdContext::instance();
    if(context.midiReceiver)
        context.midiReceiver->receiveNoteOn(channel, pitch, velocity);
    else {
        Message m(NOTE_ON);
        m.channel = channel;
        m.pitch = pitch;
        m.velocity = velocity;
        context.addMessage(m);
    }
}

void PdBase::PdContext::_controlchange(int channel, int controller, int value) {
    PdContext& context = PdContext::instance();
    if(context.midiReceiver)
        context.midiReceiver->receiveControlChange(channel, controller, value);
    else {
        Message m(CONTROL_CHANGE);
        m.channel = channel;
        m.controller = controller;
        m.value = value;
        context.addMessage(m);
    }
}

void PdBase::PdContext::_programchange(int channel, int value) {
    PdContext& context = PdContext::instance();
    if(context.midiReceiver)
        context.midiReceiver->receiveProgramChange(channel, value);
    else {
        Message m(PROGRAM_CHANGE);
        m.channel = channel;
        m.value = value;
        context.addMessage(m);
    }
}

void PdBase::PdContext::_pitchbend(int channel, int value) {
    PdContext& context = PdContext::instance();
    if(context.midiReceiver)
        context.midiReceiver->receivePitchBend(channel, value);
    else {
        Message m(PITCH_BEND);
        m.channel = channel;
        m. value = value;
        context.addMessage(m);
    }
}

void PdBase::PdContext::_aftertouch(int channel, int value) {
    PdContext& context = PdContext::instance();
    if(context.midiReceiver)
        context.midiReceiver->receiveAftertouch(channel, value);
    else {
        Message m(AFTERTOUCH);
        m.channel = channel;
        m.value = value;
        context.addMessage(m);
    }
}

void PdBase::PdContext::_polyaftertouch(int channel, int pitch, int value) {
    PdContext& context = PdContext::instance();
    if(context.midiReceiver)
        context.midiReceiver->receivePolyAftertouch(channel, pitch, value);
    else {
        Message m(POLY_AFTERTOUCH);
        m.channel = channel;
        m.pitch = pitch;
        m.value = value;
        context.addMessage(m);
    }
}

void PdBase::PdContext::_midibyte(int port, int byte) {
    PdContext& context = PdContext::instance();
    if(context.midiReceiver)
        context.midiReceiver->receiveMidiByte(port, byte);
    else {
        Message m(BYTE);
        m.port = port;
        m.byte = byte;
        context.addMessage(m);
    }
}

} // namespace
