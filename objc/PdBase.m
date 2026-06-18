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
#include "z_queued.h"
#import "PdInstance.h"

#pragma mark - List Conversion

static void encodeList(NSArray *list) {
	for (int i = 0; i < list.count; i++) {
		NSObject *object = list[i];
		if ([object isKindOfClass:[NSNumber class]]) {
			libpd_add_float(((NSNumber *)object).floatValue);
		}
		else if ([object isKindOfClass:[NSString class]]) {
			if ([(NSString *)object canBeConvertedToEncoding:NSUTF8StringEncoding]) {
				libpd_add_symbol([(NSString *)object cStringUsingEncoding:NSUTF8StringEncoding]);
			}
			else {
				// If string contains non-ASCII characters, allow a lossy conversion (instead of returning null).
				NSData *data = [(NSString *)object dataUsingEncoding:NSUTF8StringEncoding allowLossyConversion:YES];
				NSString* newString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
				libpd_add_symbol([newString cStringUsingEncoding:NSUTF8StringEncoding]);
			}
		}
		else {
			NSLog(@"Pd: list object type not supported: %@", [object class]);
		}
	}
}

#pragma mark -

@implementation PdBase

#pragma mark Initializing Pd

// queued by default
+ (int)initialize {
		return [PdInstance initializeMainInstanceWithQueue:YES];
}

+ (int)initializeWithQueue:(BOOL)queue {
	return [PdInstance initializeMainInstanceWithQueue:queue];
}

+ (BOOL)isQueued {
	return PdInstance.thisInstance.isQueued;
}

+ (void)addToSearchPath:(NSString *)path {
	libpd_add_to_search_path([path cStringUsingEncoding:NSUTF8StringEncoding]);
}

+ (void)clearSearchPath {
	libpd_clear_search_path();
}

#pragma mark Opening Patches

+ (void *)openFile:(NSString *)baseName path:(NSString *)pathName {
	if (!baseName || !pathName) {
		return NULL;
	}
	if (![[NSFileManager defaultManager] fileExistsAtPath:[pathName stringByAppendingPathComponent:baseName]]) {
		return NULL;
	}
	const char *base = [baseName cStringUsingEncoding:NSUTF8StringEncoding];
	const char *path = [pathName cStringUsingEncoding:NSUTF8StringEncoding];
	return libpd_openfile(base, path);
}

+ (void)closeFile:(void *)x {
	if (x) {
		libpd_closefile(x);
	}
}

+ (int)dollarZeroForFile:(void *)x {
	return libpd_getdollarzero(x);
}

#pragma mark Audio Processing

+ (int)getBlockSize {
	return libpd_blocksize();
}

+ (int)openAudioWithSampleRate:(int)samplerate
                 inputChannels:(int)inputChannels
                 outputChannels:(int)outputchannels {
	return libpd_init_audio(inputChannels, outputchannels, samplerate);
}

+ (int)processFloatWithInputBuffer:(const float *)inputBuffer
                      outputBuffer:(float *)outputBuffer
                             ticks:(int)ticks {
	return libpd_process_float(ticks, inputBuffer, outputBuffer);
}

+ (int)processShortWithInputBuffer:(const short *)inputBuffer
                      outputBuffer:(short *)outputBuffer
                             ticks:(int)ticks {
	return libpd_process_short(ticks, inputBuffer, outputBuffer);
}

+ (int)processDoubleWithInputBuffer:(const double *)inputBuffer
                       outputBuffer:(double *)outputBuffer
                              ticks:(int)ticks {
	return libpd_process_double(ticks, inputBuffer, outputBuffer);
}

+ (void)computeAudio:(BOOL)enable {
	[PdBase sendMessage:@"dsp" withArguments:@[@(enable)] toReceiver:@"pd"];
}

#pragma mark Array Access

+ (int)arraySizeForArrayNamed:(NSString *)arrayName {
	return libpd_arraysize([arrayName cStringUsingEncoding:NSUTF8StringEncoding]);
}

+ (int)resizeArrayNamed:(NSString *)arrayName toSize:(long)size {
	return libpd_resize_array([arrayName cStringUsingEncoding:NSUTF8StringEncoding], size);
}

+ (int)copyArrayNamed:(NSString *)arrayName withOffset:(int)offset
              toArray:(float *)destinationArray count:(int)n {
	const char *name = [arrayName cStringUsingEncoding:NSUTF8StringEncoding];
	return libpd_read_array(destinationArray, name, offset, n);
}

+ (int)copyArray:(float *)sourceArray toArrayNamed:(NSString *)arrayName
      withOffset:(int)offset count:(int)n {
	const char *name = [arrayName cStringUsingEncoding:NSUTF8StringEncoding];
	return libpd_write_array(name, offset, sourceArray, n);
}

#pragma mark Sending Messages to Pd

+ (int)sendBangToReceiver:(NSString *)receiverName {
	return libpd_bang([receiverName cStringUsingEncoding:NSUTF8StringEncoding]);
}

+ (int)sendFloat:(float)value toReceiver:(NSString *)receiverName {
	return libpd_float([receiverName cStringUsingEncoding:NSUTF8StringEncoding], value);
}

+ (int)sendSymbol:(NSString *)symbol toReceiver:(NSString *)receiverName {
	return libpd_symbol([receiverName cStringUsingEncoding:NSUTF8StringEncoding],
	                    [symbol cStringUsingEncoding:NSUTF8StringEncoding]);
}

+ (int)sendList:(NSArray *)list toReceiver:(NSString *)receiverName {
		if (libpd_start_message((int) list.count)) return -100;
	encodeList(list);
	return libpd_finish_list([receiverName cStringUsingEncoding:NSUTF8StringEncoding]);
}

+ (int)sendMessage:(NSString *)message withArguments:(NSArray *)list
        toReceiver:(NSString *)receiverName {
	if (libpd_start_message((int) list.count)) return -100;
	encodeList(list);
	return libpd_finish_message([receiverName cStringUsingEncoding:NSUTF8StringEncoding],
	                            [message cStringUsingEncoding:NSUTF8StringEncoding]);
}

#pragma mark Receiving Messages from Pd

+ (void *)subscribe:(NSString *)symbol {
	return libpd_bind([symbol cStringUsingEncoding:NSUTF8StringEncoding]);
}

+ (void)unsubscribe:(void *)subscription {
	libpd_unbind(subscription);
}

+ (BOOL)exists:(NSString *)symbol {
	return (BOOL) libpd_exists([symbol cStringUsingEncoding:NSUTF8StringEncoding]);
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
	libpd_queued_receive_pd_messages();
}

#pragma mark Sending MIDI messages to Pd

+ (int)sendNoteOn:(int)channel pitch:(int)pitch velocity:(int)velocity {
	return libpd_noteon(channel, pitch, velocity);
}

+ (int)sendControlChange:(int)channel controller:(int)controller value:(int)value {
	return libpd_controlchange(channel, controller, value);
}

+ (int)sendProgramChange:(int)channel value:(int)value {
	return libpd_programchange(channel, value);
}

+ (int)sendPitchBend:(int)channel value:(int)value {
	return libpd_pitchbend(channel, value);
}

+ (int)sendAftertouch:(int)channel value:(int)value {
	return libpd_aftertouch(channel, value);
}

+ (int)sendPolyAftertouch:(int)channel pitch:(int)pitch value:(int)value {
	return libpd_polyaftertouch(channel, pitch, value);
}

+ (int)sendMidiByte:(int)port byte:(int)byte {
	return libpd_midibyte(port, byte);
}

+ (int)sendSysex:(int)port byte:(int)byte {
	return libpd_sysex(port, byte);
}

+ (int)sendSysRealTime:(int)port byte:(int)byte {
	return libpd_sysrealtime(port, byte);
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

#pragma mark Log Level

+ (void)setVerbose:(BOOL)verbose {
	libpd_set_verbose((int)verbose);
}

+ (BOOL)getVerbose {
	return libpd_get_verbose();
}

@end
