//
// This software is copyrighted by Reality Jockey Ltd. and Peter Brinkmann.
// The following terms (the "Standard Improved BSD License") apply to
// all files associated with the software unless explicitly disclaimed
// in individual files:
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following
// disclaimer in the documentation and/or other materials provided
// with the distribution.
// 3. The name of the author may not be used to endorse or promote
// products derived from this software without specific prior
// written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
// IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//
// Updated 2013, 2018, 2020, 2024 Dan Wilcox <danomatika@gmail.com>
//

#import "PdBase.h"
#include "z_libpd.h"
#import "PdInstance.h"

#pragma mark -

@implementation PdBase

#pragma mark Initializing Pd

// queued by default
+ (int)initialize {
	// any call to PdInstance.this or PdInstance.mainInstance creates the
	// default queued main instance, do
	int ret = libpd_init();
	if (libpd_get_instancedata() == NULL) {
		return [PdInstance initMainInstanceWithQueue:YES];
	}
	return ret;
}

+ (int)initializeWithQueue:(BOOL)queue {
	return [PdInstance initMainInstanceWithQueue:queue];
}

+ (BOOL)isQueued {
	return PdInstance.thisInstance.isQueued;
}

+ (void)clearSearchPath {
	[PdInstance.thisInstance clearSearchPath];
}

+ (void)addToSearchPath:(NSString *)path {
	[PdInstance.thisInstance addToSearchPath:path];
}

#pragma mark Opening Patches

+ (void *)openFile:(NSString *)baseName path:(NSString *)pathName {
	return [PdInstance.thisInstance openFile:baseName path:pathName];
}

+ (void)closeFile:(void *)x {
	[PdInstance.thisInstance closeFile:x];
}

+ (int)dollarZeroForFile:(void *)x {
	return [PdInstance.thisInstance dollarZeroForFile:x];
}

#pragma mark Audio Processing

+ (int)getBlockSize {
	return [PdInstance getBlockSize];
}

+ (int)openAudioWithSampleRate:(int)samplerate
                 inputChannels:(int)inputChannels
                 outputChannels:(int)outputchannels {
	return [PdInstance.thisInstance openAudioWithSampleRate:samplerate
	                                          inputChannels:inputChannels
	                                         outputChannels:outputchannels];
}

+ (int)processFloatWithInputBuffer:(const float *)inputBuffer
                      outputBuffer:(float *)outputBuffer
                             ticks:(int)ticks {
	return [PdInstance.thisInstance processFloatWithInputBuffer:inputBuffer
	                                               outputBuffer:outputBuffer
	                                                      ticks:ticks];
}

+ (int)processShortWithInputBuffer:(const short *)inputBuffer
                      outputBuffer:(short *)outputBuffer
                             ticks:(int)ticks {
	return [PdInstance.thisInstance processShortWithInputBuffer:inputBuffer
	                                               outputBuffer:outputBuffer
	                                                      ticks:ticks];
}

+ (int)processDoubleWithInputBuffer:(const double *)inputBuffer
                       outputBuffer:(double *)outputBuffer
                              ticks:(int)ticks {
	return [PdInstance.thisInstance processDoubleWithInputBuffer:inputBuffer
	                                                outputBuffer:outputBuffer
	                                                       ticks:ticks];
}

+ (void)computeAudio:(BOOL)enable {
	[PdInstance.thisInstance sendMessage:@"dsp" withArguments:@[@(enable)] toReceiver:@"pd"];
}

#pragma mark Array Access

+ (int)arraySizeForArrayNamed:(NSString *)arrayName {
	return [PdInstance.thisInstance arraySizeForArrayNamed:arrayName];
}

+ (int)resizeArrayNamed:(NSString *)arrayName toSize:(long)size {
	return [PdInstance.thisInstance resizeArrayNamed:arrayName toSize:size];
}

+ (int)copyArrayNamed:(NSString *)arrayName withOffset:(int)offset
              toArray:(float *)destinationArray count:(int)n {
	return [PdInstance.thisInstance copyArrayNamed:arrayName withOffset:offset toArray:destinationArray count:n];
}

+ (int)copyArray:(float *)sourceArray toArrayNamed:(NSString *)arrayName
      withOffset:(int)offset count:(int)n {
	return [PdInstance.thisInstance copyArray:sourceArray toArrayNamed:arrayName withOffset:offset count:n];
}

#pragma mark Sending Messages to Pd

+ (int)sendBangToReceiver:(NSString *)receiverName {
	return [PdInstance.thisInstance sendBangToReceiver:receiverName];
}

+ (int)sendFloat:(float)value toReceiver:(NSString *)receiverName {
	return [PdInstance.thisInstance sendFloat:value toReceiver:receiverName];
}

+ (int)sendSymbol:(NSString *)symbol toReceiver:(NSString *)receiverName {
	return [PdInstance.thisInstance sendSymbol:symbol toReceiver:receiverName];
}

+ (int)sendList:(NSArray *)list toReceiver:(NSString *)receiverName {
	return [PdInstance.thisInstance sendList:list toReceiver:receiverName];
}

+ (int)sendMessage:(NSString *)message withArguments:(NSArray *)list
        toReceiver:(NSString *)receiverName {
	return [PdInstance.thisInstance sendMessage:message withArguments:list toReceiver:receiverName];
}

#pragma mark Receiving Messages from Pd

+ (void *)subscribe:(NSString *)symbol {
	return [PdInstance.thisInstance subscribe:symbol];
}

+ (void)unsubscribe:(void *)subscription {
	[PdInstance.thisInstance unsubscribe:subscription];
}

+ (BOOL)exists:(NSString *)symbol {
	return [PdInstance.thisInstance exists:symbol];
}

// only to be called from main thread
+ (NSObject<PdReceiverDelegate> *)delegate {
	return PdInstance.thisInstance.delegate;
}

// only to be called from main thread
+ (void)setDelegate:(NSObject<PdReceiverDelegate> *)newDelegate {
	[PdInstance.thisInstance setDelegate:newDelegate];
}

+ (void)setDelegate:(NSObject<PdReceiverDelegate> *)newDelegate pollingEnabled:(BOOL)pollingEnabled {
	[PdInstance.thisInstance setDelegate:newDelegate pollingEnabled:pollingEnabled];
}

+ (void)receiveMessages {
	[PdInstance.thisInstance receiveMessages];
}

#pragma mark Sending MIDI messages to Pd

+ (int)sendNoteOn:(int)channel pitch:(int)pitch velocity:(int)velocity {
	return [PdInstance.thisInstance sendNoteOn:channel pitch:pitch velocity:velocity];
}

+ (int)sendControlChange:(int)channel controller:(int)controller value:(int)value {
	return [PdInstance.thisInstance sendControlChange:channel controller:controller value:value];
}

+ (int)sendProgramChange:(int)channel value:(int)value {
	return [PdInstance.thisInstance sendProgramChange:channel value:value];
}

+ (int)sendPitchBend:(int)channel value:(int)value {
	return [PdInstance.thisInstance sendPitchBend:channel value:value];
}

+ (int)sendAftertouch:(int)channel value:(int)value {
	return [PdInstance.thisInstance sendAftertouch:channel value:value];
}

+ (int)sendPolyAftertouch:(int)channel pitch:(int)pitch value:(int)value {
	return [PdInstance.thisInstance sendPolyAftertouch:channel pitch:pitch value:value];
}

+ (int)sendMidiByte:(int)port byte:(int)byte {
	return [PdInstance.thisInstance sendMidiByte:port byte:byte];
}

+ (int)sendSysex:(int)port byte:(int)byte {
	return [PdInstance.thisInstance sendSysex:port byte:byte];
}

+ (int)sendSysRealTime:(int)port byte:(int)byte {
	return [PdInstance.thisInstance sendSysRealTime:port byte:byte];
}

#pragma mark Receiving MIDI Messages from Pd

// only to be called from main thread
+ (NSObject<PdMidiReceiverDelegate> *)midiDelegate {
	return PdInstance.thisInstance.midiDelegate;
}

// only to be called from main thread
+ (void)setMidiDelegate:(NSObject<PdMidiReceiverDelegate> *)newDelegate {
	[PdInstance.thisInstance setMidiDelegate:newDelegate];
}

+ (void)setMidiDelegate:(NSObject<PdMidiReceiverDelegate> *)newDelegate pollingEnabled:(BOOL)pollingEnabled {
	return [PdInstance.thisInstance setMidiDelegate:newDelegate pollingEnabled:pollingEnabled];
}

+ (void)receiveMidi {
	[PdInstance.thisInstance receiveMidi];
}

@end
