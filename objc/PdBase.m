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
// Updated 2013, 2018 Dan Wilcox <danomatika@gmail.com>
//

#import "PdBase.h"
#include "z_libpd.h"
#include "z_queued.h"
#include "z_print_util.h"

static NSObject<PdReceiverDelegate> *delegate = nil;
static NSObject<PdMidiReceiverDelegate> *midiDelegate = nil;

#pragma mark - List Conversion

static NSArray *decodeList(int argc, t_atom *argv) {
	NSMutableArray *list = [[NSMutableArray alloc] initWithCapacity:argc];
	for (int i = 0; i < argc; i++) {
		t_atom *a = &argv[i];
		if (libpd_is_float(a)) {
			float x = libpd_get_float(a);
			NSNumber *num = @(x);
			[list addObject:num];
		} else if (libpd_is_symbol(a)) {
			const char *s = libpd_get_symbol(a);
			NSString *str = [[NSString alloc] initWithCString:s encoding:NSUTF8StringEncoding];
			[list addObject:str];
		} else {
			NSLog(@"PdBase: element type unsupported: %i", a->a_type);
		}
	}
	return (NSArray *)list;
}

static void encodeList(NSArray *list) {
	for (int i = 0; i < list.count; i++) {
		NSObject *object = list[i];
		if ([object isKindOfClass:[NSNumber class]]) {
			libpd_add_float(((NSNumber *)object).floatValue);
		} else if ([object isKindOfClass:[NSString class]]) {
			if ([(NSString *)object canBeConvertedToEncoding:NSUTF8StringEncoding]) {
				libpd_add_symbol([(NSString *)object cStringUsingEncoding:NSUTF8StringEncoding]);
			} else {
				// If string contains non-ASCII characters, allow a lossy conversion (instead of returning null).
				NSData *data = [(NSString *)object dataUsingEncoding:NSUTF8StringEncoding allowLossyConversion:YES];
				NSString* newString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
				libpd_add_symbol([newString cStringUsingEncoding:NSUTF8StringEncoding]);
			}
		} else {
			NSLog(@"PdBase: message not supported. %@", [object class]);
		}
	}
}

#pragma mark - Hooks

static void printHook(const char *s) {
	if ([delegate respondsToSelector:@selector(receivePrint:)]) {
		NSString *msg = [[NSString alloc] initWithCString:s encoding:NSUTF8StringEncoding];
		[delegate receivePrint:msg];
	}
}

static void bangHook(const char *src) {
	if ([delegate respondsToSelector:@selector(receiveBangFromSource:)]) {
		NSString *source = [[NSString alloc] initWithCString:src encoding:NSUTF8StringEncoding];
		[delegate receiveBangFromSource:source];
	}
}

static void floatHook(const char *src, float x) {
	if ([delegate respondsToSelector:@selector(receiveFloat:fromSource:)]) {
		NSString *source = [[NSString alloc] initWithCString:src encoding:NSUTF8StringEncoding];
		[delegate receiveFloat:x fromSource:source];
	}
}

static void symbolHook(const char *src, const char *sym) {
	if ([delegate respondsToSelector:@selector(receiveSymbol:fromSource:)]) {
		NSString *source = [[NSString alloc] initWithCString:src encoding:NSUTF8StringEncoding];
		NSString *symbol = [[NSString alloc] initWithCString:sym encoding:NSUTF8StringEncoding];
		[delegate receiveSymbol:symbol fromSource:source];
	}
}

static void listHook(const char *src, int argc, t_atom *argv) {
	if ([delegate respondsToSelector:@selector(receiveList:fromSource:)]) {
		NSString *source = [[NSString alloc] initWithCString:src encoding:NSUTF8StringEncoding];
		NSArray *args = decodeList(argc, argv);
		[delegate receiveList:args fromSource:source];
	}
}

static void messageHook(const char *src, const char* sym, int argc, t_atom *argv) {
	if ([delegate respondsToSelector:@selector(receiveMessage:withArguments:fromSource:)]) {
		NSString *source = [[NSString alloc] initWithCString:src encoding:NSUTF8StringEncoding];
		NSString *symbol = [[NSString alloc] initWithCString:sym encoding:NSUTF8StringEncoding];
		NSArray *args = decodeList(argc, argv);
		[delegate receiveMessage:symbol withArguments:args fromSource:source];
	}
}

static void noteonHook(int channel, int pitch, int velocity) {
	if ([midiDelegate respondsToSelector:@selector(receiveNoteOn:withVelocity:forChannel:)]) {
		[midiDelegate receiveNoteOn:pitch withVelocity:velocity forChannel:channel];
	}
}

static void controlChangeHook(int channel, int controller, int value) {
	if ([midiDelegate respondsToSelector:@selector(receiveControlChange:forController:forChannel:)]) {
		[midiDelegate receiveControlChange:value forController:controller forChannel:channel];
	}
}

static void programChangeHook(int channel, int value) {
	if ([midiDelegate respondsToSelector:@selector(receiveProgramChange:forChannel:)]) {
		[midiDelegate receiveProgramChange:value forChannel:channel];
	}
}

static void pitchBendHook(int channel, int value) {
	if ([midiDelegate respondsToSelector:@selector(receivePitchBend:forChannel:)]) {
		[midiDelegate receivePitchBend:value forChannel:channel];
	}
}

static void aftertouchHook(int channel, int value) {
	if ([midiDelegate respondsToSelector:@selector(receiveAftertouch:forChannel:)]) {
		[midiDelegate receiveAftertouch:value forChannel:channel];
	}
}

static void polyAftertouchHook(int channel, int pitch, int value) {
	if ([midiDelegate respondsToSelector:@selector(receiveAftertouch:forChannel:)]) {
		[midiDelegate receivePolyAftertouch:value forPitch:pitch forChannel:channel];
	}
}

static void midiByteHook(int port, int byte) {
	if ([midiDelegate respondsToSelector:@selector(receiveMidiByte:forPort:)]) {
		[midiDelegate receiveMidiByte:byte forPort:port];
	}
}

#pragma mark -

@interface PdBase () {}

// timer methods, same as receiveMessage & receiveMidi
+ (void)receiveMessagesTimer:(NSTimer*)theTimer;
+ (void)receiveMidiTimer:(NSTimer*)theTimer;

@end

static NSTimer *messagePollTimer;
static NSTimer *midiPollTimer;

@implementation PdBase

// Queued by default.
+ (void)initialize {
	libpd_set_queued_printhook(libpd_print_concatenator);
	libpd_set_concatenated_printhook(printHook);

	libpd_set_queued_banghook(bangHook);
	libpd_set_queued_floathook(floatHook);
	libpd_set_queued_symbolhook(symbolHook);
	libpd_set_queued_listhook(listHook);
	libpd_set_queued_messagehook(messageHook);

	libpd_set_queued_noteonhook(noteonHook);
	libpd_set_queued_controlchangehook(controlChangeHook);
	libpd_set_queued_programchangehook(programChangeHook);
	libpd_set_queued_pitchbendhook(pitchBendHook);
	libpd_set_queued_aftertouchhook(aftertouchHook);
	libpd_set_queued_polyaftertouchhook(polyAftertouchHook);
	libpd_set_queued_midibytehook(midiByteHook);

	libpd_queued_init();
}

// Only to be called from main thread.
+ (void)setDelegate:(NSObject<PdReceiverDelegate> *)newDelegate {
	[self setDelegate:newDelegate pollingEnabled:YES];
}

// Only to be called from main thread.
+ (void)setMidiDelegate:(NSObject<PdMidiReceiverDelegate> *)newDelegate {
	[self setMidiDelegate:newDelegate pollingEnabled:YES];
}

+ (void)setDelegate:(NSObject<PdReceiverDelegate> *)newDelegate pollingEnabled:(BOOL)pollingEnabled {
	if (messagePollTimer) {
		[messagePollTimer invalidate];
		messagePollTimer = nil;
	}
	delegate = newDelegate;
	if (delegate && pollingEnabled) {
		messagePollTimer = [NSTimer timerWithTimeInterval:0.02 target:self selector:@selector(receiveMessagesTimer:) userInfo:nil repeats:YES];
		[[NSRunLoop mainRunLoop] addTimer:messagePollTimer forMode:NSRunLoopCommonModes];
	}
}

+ (void)setMidiDelegate:(NSObject<PdMidiReceiverDelegate> *)newDelegate pollingEnabled:(BOOL)pollingEnabled {
	if (midiPollTimer) {
		[midiPollTimer invalidate];
		midiPollTimer = nil;
	}
	midiDelegate = newDelegate;
	if (midiDelegate && pollingEnabled) {
		midiPollTimer = [NSTimer timerWithTimeInterval:0.02 target:self selector:@selector(receiveMidiTimer:) userInfo:nil repeats:YES];
		[[NSRunLoop mainRunLoop] addTimer:midiPollTimer forMode:NSRunLoopCommonModes];
	}
}

// Only to be called from main thread.
+ (NSObject<PdReceiverDelegate> *)delegate {
	return delegate;
}

// Only to be called from main thread.
+ (NSObject<PdMidiReceiverDelegate> *)midiDelegate {
	return midiDelegate;
}

+ (void)receiveMessages {
	libpd_queued_receive_pd_messages();
}

+ (void)receiveMidi {
	libpd_queued_receive_midi_messages();
}

+ (void)receiveMessagesTimer:(NSTimer*)theTimer {
	libpd_queued_receive_pd_messages();
}

+ (void)receiveMidiTimer:(NSTimer*)theTimer {
	libpd_queued_receive_midi_messages();
}

+ (void *)subscribe:(NSString *)symbol {
	return libpd_bind([symbol cStringUsingEncoding:NSUTF8StringEncoding]);
}

+ (void)unsubscribe:(void *)subscription {
	libpd_unbind(subscription);
}

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

+ (void)clearSearchPath {
	libpd_clear_search_path();
}

+ (void)addToSearchPath:(NSString *)path {
	libpd_add_to_search_path([path cStringUsingEncoding:NSUTF8StringEncoding]);
}

+ (int)getBlockSize {
	return libpd_blocksize();
}

+ (BOOL)exists:(NSString *)symbol {
	return (BOOL) libpd_exists([symbol cStringUsingEncoding:NSUTF8StringEncoding]);
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

+ (int)processDoubleWithInputBuffer:(const double *)inputBuffer
                       outputBuffer:(double *)outputBuffer
                              ticks:(int)ticks {
	return libpd_process_double(ticks, inputBuffer, outputBuffer);
}

+ (int)processShortWithInputBuffer:(const short *)inputBuffer
                      outputBuffer:(short *)outputBuffer
                             ticks:(int)ticks {
	return libpd_process_short(ticks, inputBuffer, outputBuffer);
}

+ (void)computeAudio:(BOOL)enable {
	NSNumber *val = @(enable);
	NSArray *args = @[val];
	[PdBase sendMessage:@"dsp" withArguments:args toReceiver:@"pd"];
}

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

+ (int)arraySizeForArrayNamed:(NSString *)arrayName {
	return libpd_arraysize([arrayName cStringUsingEncoding:NSUTF8StringEncoding]);
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

@end
