//
//  PdInstance.m
//  libpd
//
//  Copyright (c) 2022 Dan Wilcox <danomatika@gmail.com>
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//

#import "PdInstance.h"
#include "z_libpd.h"
#include "z_queued.h"
#include "z_print_util.h"

#ifdef PDINSTANCE
    #define PDBASE_SETINSTANCE libpd_set_instance(instance);
#else
    #define PDBASE_SETINSTANCE
#endif

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
	PdInstance *instance = (__bridge PdInstance *)libpd_get_instancedata();
	if (instance.delegate && [instance.delegate respondsToSelector:@selector(receivePrint:)]) {
		NSString *msg = [[NSString alloc] initWithCString:s encoding:NSUTF8StringEncoding];
		[instance.delegate receivePrint:msg];
	}
}

static void bangHook(const char *src) {
	PdInstance *instance = (__bridge PdInstance *)libpd_get_instancedata();
	if (instance.delegate && [instance.delegate respondsToSelector:@selector(receiveBangFromSource:)]) {
		NSString *source = [[NSString alloc] initWithCString:src encoding:NSUTF8StringEncoding];
		[instance.delegate receiveBangFromSource:source];
	}
}

static void floatHook(const char *src, float x) {
	PdInstance *instance = (__bridge PdInstance *)libpd_get_instancedata();
	if (instance.delegate && [instance.delegate respondsToSelector:@selector(receiveFloat:fromSource:)]) {
		NSString *source = [[NSString alloc] initWithCString:src encoding:NSUTF8StringEncoding];
		[instance.delegate receiveFloat:x fromSource:source];
	}
}

static void symbolHook(const char *src, const char *sym) {
	PdInstance *instance = (__bridge PdInstance *)libpd_get_instancedata();
	if (instance.delegate && [instance.delegate respondsToSelector:@selector(receiveSymbol:fromSource:)]) {
		NSString *source = [[NSString alloc] initWithCString:src encoding:NSUTF8StringEncoding];
		NSString *symbol = [[NSString alloc] initWithCString:sym encoding:NSUTF8StringEncoding];
		[instance.delegate receiveSymbol:symbol fromSource:source];
	}
}

static void listHook(const char *src, int argc, t_atom *argv) {
	PdInstance *instance = (__bridge PdInstance *)libpd_get_instancedata();
	if (instance.delegate && [instance.delegate respondsToSelector:@selector(receiveList:fromSource:)]) {
		NSString *source = [[NSString alloc] initWithCString:src encoding:NSUTF8StringEncoding];
		NSArray *args = decodeList(argc, argv);
		[instance.delegate receiveList:args fromSource:source];
	}
}

static void messageHook(const char *src, const char* sym, int argc, t_atom *argv) {
	PdInstance *instance = (__bridge PdInstance *)libpd_get_instancedata();
	if (instance.delegate && [instance.delegate respondsToSelector:@selector(receiveMessage:withArguments:fromSource:)]) {
		NSString *source = [[NSString alloc] initWithCString:src encoding:NSUTF8StringEncoding];
		NSString *symbol = [[NSString alloc] initWithCString:sym encoding:NSUTF8StringEncoding];
		NSArray *args = decodeList(argc, argv);
		[instance.delegate receiveMessage:symbol withArguments:args fromSource:source];
	}
}

static void noteonHook(int channel, int pitch, int velocity) {
	PdInstance *instance = (__bridge PdInstance *)libpd_get_instancedata();
	if (instance.midiDelegate && [instance.midiDelegate respondsToSelector:@selector(receiveNoteOn:withVelocity:forChannel:)]) {
		[instance.midiDelegate receiveNoteOn:pitch withVelocity:velocity forChannel:channel];
	}
}

static void controlChangeHook(int channel, int controller, int value) {
	PdInstance *instance = (__bridge PdInstance *)libpd_get_instancedata();
	if (instance.midiDelegate && [instance.midiDelegate respondsToSelector:@selector(receiveControlChange:forController:forChannel:)]) {
		[instance.midiDelegate receiveControlChange:value forController:controller forChannel:channel];
	}
}

static void programChangeHook(int channel, int value) {
	PdInstance *instance = (__bridge PdInstance *)libpd_get_instancedata();
	if (instance.midiDelegate && [instance.midiDelegate respondsToSelector:@selector(receiveProgramChange:forChannel:)]) {
		[instance.midiDelegate receiveProgramChange:value forChannel:channel];
	}
}

static void pitchBendHook(int channel, int value) {
	PdInstance *instance = (__bridge PdInstance *)libpd_get_instancedata();
	if (instance.midiDelegate && [instance.midiDelegate respondsToSelector:@selector(receivePitchBend:forChannel:)]) {
		[instance.midiDelegate receivePitchBend:value forChannel:channel];
	}
}

static void aftertouchHook(int channel, int value) {
	PdInstance *instance = (__bridge PdInstance *)libpd_get_instancedata();
	if (instance.midiDelegate && [instance.midiDelegate respondsToSelector:@selector(receiveAftertouch:forChannel:)]) {
		[instance.midiDelegate receiveAftertouch:value forChannel:channel];
	}
}

static void polyAftertouchHook(int channel, int pitch, int value) {
	PdInstance *instance = (__bridge PdInstance *)libpd_get_instancedata();
	if ([instance.midiDelegate respondsToSelector:@selector(receiveAftertouch:forChannel:)]) {
		[instance.midiDelegate receivePolyAftertouch:value forPitch:pitch forChannel:channel];
	}
}

static void midiByteHook(int port, int byte) {
	PdInstance *instance = (__bridge PdInstance *)libpd_get_instancedata();
	if ([instance.midiDelegate respondsToSelector:@selector(receiveMidiByte:forPort:)]) {
		[instance.midiDelegate receiveMidiByte:byte forPort:port];
	}
}

#pragma mark -

@interface PdInstance () {
	BOOL queued;
	NSTimer *messagePollTimer;
	NSTimer *midiPollTimer;
#ifdef PDINSTANCE
	t_pdinstance *instance; ///< instance pointer
#endif
}

// timer methods, same as receiveMessage & receiveMidi
- (void)receiveMessagesTimer:(NSTimer*)theTimer;
- (void)receiveMidiTimer:(NSTimer*)theTimer;

@end

@implementation PdInstance

#pragma mark Initializing Pd

// queued by default
- (instancetype)init {
	self = [super init];
	if (self) {
		[self setupWithQueue:YES];
	}
	return self;
}

- (instancetype)initWithQueue:(BOOL)queue {
	self = [super init];
	if (self) {
		[self setupWithQueue:queue];
	}
	return self;
}

- (void)setupWithQueue:(BOOL)queue {
	libpd_init();
	#ifdef PDINSTANCE
        instance = libpd_new_instance();
    #endif
	libpd_set_instancedata((__bridge void *)(self), NULL);
	if (queue) {
		queued = YES;
		PDBASE_SETINSTANCE
		libpd_queued_init();

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
	}
	else {
		[self stopMessagesTimer];
		[self stopMidiTimer];
		queued = NO;
		libpd_set_printhook(libpd_print_concatenator);
		libpd_set_concatenated_printhook(printHook);

		libpd_set_banghook(bangHook);
		libpd_set_floathook(floatHook);
		libpd_set_symbolhook(symbolHook);
		libpd_set_listhook(listHook);
		libpd_set_messagehook(messageHook);

		libpd_set_noteonhook(noteonHook);
		libpd_set_controlchangehook(controlChangeHook);
		libpd_set_programchangehook(programChangeHook);
		libpd_set_pitchbendhook(pitchBendHook);
		libpd_set_aftertouchhook(aftertouchHook);
		libpd_set_polyaftertouchhook(polyAftertouchHook);
		libpd_set_midibytehook(midiByteHook);
	}
}

- (BOOL)isQueued {
	return queued;
}

- (void)clearSearchPath {
	PDBASE_SETINSTANCE
	libpd_clear_search_path();
}

- (void)addToSearchPath:(NSString *)path {
	PDBASE_SETINSTANCE
	libpd_add_to_search_path([path cStringUsingEncoding:NSUTF8StringEncoding]);
}

#pragma mark Opening Patches

- (void *)openFile:(NSString *)baseName path:(NSString *)pathName {
	PDBASE_SETINSTANCE
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

- (void)closeFile:(void *)x {
	PDBASE_SETINSTANCE
	if (x) {
		libpd_closefile(x);
	}
}

- (int)dollarZeroForFile:(void *)x {
	PDBASE_SETINSTANCE
	return libpd_getdollarzero(x);
}

#pragma mark Audio Processing

+ (int)getBlockSize {
	return libpd_blocksize();
}

- (int)openAudioWithSampleRate:(int)samplerate
                 inputChannels:(int)inputChannels
                 outputChannels:(int)outputchannels {
	PDBASE_SETINSTANCE
	return libpd_init_audio(inputChannels, outputchannels, samplerate);
}

- (int)processFloatWithInputBuffer:(const float *)inputBuffer
                      outputBuffer:(float *)outputBuffer
                             ticks:(int)ticks {
	PDBASE_SETINSTANCE
	return libpd_process_float(ticks, inputBuffer, outputBuffer);
}

- (int)processShortWithInputBuffer:(const short *)inputBuffer
                      outputBuffer:(short *)outputBuffer
                             ticks:(int)ticks {
	PDBASE_SETINSTANCE
	return libpd_process_short(ticks, inputBuffer, outputBuffer);
}

- (int)processDoubleWithInputBuffer:(const double *)inputBuffer
                       outputBuffer:(double *)outputBuffer
                              ticks:(int)ticks {
	PDBASE_SETINSTANCE
	return libpd_process_double(ticks, inputBuffer, outputBuffer);
}

- (void)computeAudio:(BOOL)enable {
	PDBASE_SETINSTANCE
	[PdBase sendMessage:@"dsp" withArguments:@[@(enable)] toReceiver:@"pd"];
}

#pragma mark Array Access

- (int)arraySizeForArrayNamed:(NSString *)arrayName {
	PDBASE_SETINSTANCE
	return libpd_arraysize([arrayName cStringUsingEncoding:NSUTF8StringEncoding]);
}

- (int)resizeArrayNamed:(NSString *)arrayName toSize:(long)size {
	PDBASE_SETINSTANCE
	return libpd_resize_array([arrayName cStringUsingEncoding:NSUTF8StringEncoding], size);
}

- (int)copyArrayNamed:(NSString *)arrayName withOffset:(int)offset
              toArray:(float *)destinationArray count:(int)n {
    PDBASE_SETINSTANCE
	const char *name = [arrayName cStringUsingEncoding:NSUTF8StringEncoding];
	return libpd_read_array(destinationArray, name, offset, n);
}

- (int)copyArray:(float *)sourceArray toArrayNamed:(NSString *)arrayName
      withOffset:(int)offset count:(int)n {
    PDBASE_SETINSTANCE
	const char *name = [arrayName cStringUsingEncoding:NSUTF8StringEncoding];
	return libpd_write_array(name, offset, sourceArray, n);
}

#pragma mark Sending Messages to Pd

- (int)sendBangToReceiver:(NSString *)receiverName {
	PDBASE_SETINSTANCE
	return libpd_bang([receiverName cStringUsingEncoding:NSUTF8StringEncoding]);
}

- (int)sendFloat:(float)value toReceiver:(NSString *)receiverName {
	PDBASE_SETINSTANCE
	return libpd_float([receiverName cStringUsingEncoding:NSUTF8StringEncoding], value);
}

- (int)sendSymbol:(NSString *)symbol toReceiver:(NSString *)receiverName {
	PDBASE_SETINSTANCE
	return libpd_symbol([receiverName cStringUsingEncoding:NSUTF8StringEncoding],
			[symbol cStringUsingEncoding:NSUTF8StringEncoding]);
}

- (int)sendList:(NSArray *)list toReceiver:(NSString *)receiverName {
	PDBASE_SETINSTANCE
	if (libpd_start_message((int) list.count)) return -100;
			encodeList(list);
		return libpd_finish_list([receiverName cStringUsingEncoding:NSUTF8StringEncoding]);
}

- (int)sendMessage:(NSString *)message withArguments:(NSArray *)list
        toReceiver:(NSString *)receiverName {
    PDBASE_SETINSTANCE
	if (libpd_start_message((int) list.count)) return -100;
	encodeList(list);
	return libpd_finish_message([receiverName cStringUsingEncoding:NSUTF8StringEncoding],
		[message cStringUsingEncoding:NSUTF8StringEncoding]);
}

#pragma mark Receiving Messages from Pd

- (void *)subscribe:(NSString *)symbol {
	PDBASE_SETINSTANCE
	return libpd_bind([symbol cStringUsingEncoding:NSUTF8StringEncoding]);
}

- (void)unsubscribe:(void *)subscription {
	PDBASE_SETINSTANCE
	libpd_unbind(subscription);
}

- (BOOL)exists:(NSString *)symbol {
	PDBASE_SETINSTANCE
	return (BOOL) libpd_exists([symbol cStringUsingEncoding:NSUTF8StringEncoding]);
}

// only to be called from main thread
- (void)setDelegate:(id<PdReceiverDelegate>)newDelegate {
	[self setDelegate:newDelegate pollingEnabled:YES];
}

- (void)setDelegate:(id<PdReceiverDelegate>)newDelegate pollingEnabled:(BOOL)pollingEnabled {
	[self stopMessagesTimer];
	_delegate = newDelegate;
	if (_delegate && queued && pollingEnabled) {
		[self startMessagesTimer];
	}
}

- (void)receiveMessages {
	PDBASE_SETINSTANCE
	libpd_queued_receive_pd_messages();
}

- (void)startMessagesTimer {
	messagePollTimer = [NSTimer timerWithTimeInterval:0.02 target:self selector:@selector(receiveMessagesTimer:) userInfo:nil repeats:YES];
	[[NSRunLoop mainRunLoop] addTimer:messagePollTimer forMode:NSRunLoopCommonModes];
}

- (void)stopMessagesTimer {
	if (messagePollTimer) {
		[messagePollTimer invalidate];
		messagePollTimer = nil;
	}
}

- (void)receiveMessagesTimer:(NSTimer*)theTimer {
	PDBASE_SETINSTANCE
	libpd_queued_receive_pd_messages();
}

#pragma mark Sending MIDI messages to Pd

- (int)sendNoteOn:(int)channel pitch:(int)pitch velocity:(int)velocity {
	PDBASE_SETINSTANCE
	return libpd_noteon(channel, pitch, velocity);
}

- (int)sendControlChange:(int)channel controller:(int)controller value:(int)value {
	PDBASE_SETINSTANCE
	return libpd_controlchange(channel, controller, value);
}

- (int)sendProgramChange:(int)channel value:(int)value {
	PDBASE_SETINSTANCE
	return libpd_programchange(channel, value);
}

- (int)sendPitchBend:(int)channel value:(int)value {
	PDBASE_SETINSTANCE
	return libpd_pitchbend(channel, value);
}

- (int)sendAftertouch:(int)channel value:(int)value {
	PDBASE_SETINSTANCE
	return libpd_aftertouch(channel, value);
}

- (int)sendPolyAftertouch:(int)channel pitch:(int)pitch value:(int)value {
	PDBASE_SETINSTANCE
	return libpd_polyaftertouch(channel, pitch, value);
}

- (int)sendMidiByte:(int)port byte:(int)byte {
	PDBASE_SETINSTANCE
	return libpd_midibyte(port, byte);
}

- (int)sendSysex:(int)port byte:(int)byte {
	PDBASE_SETINSTANCE
	return libpd_sysex(port, byte);
}

- (int)sendSysRealTime:(int)port byte:(int)byte {
	PDBASE_SETINSTANCE
	return libpd_sysrealtime(port, byte);
}

#pragma mark Receiving MIDI Messages from Pd

// only to be called from main thread
- (void)setMidiDelegate:(id<PdMidiReceiverDelegate>)newDelegate {
	[self setMidiDelegate:newDelegate pollingEnabled:YES];
}

- (void)setMidiDelegate:(id<PdMidiReceiverDelegate>)newDelegate pollingEnabled:(BOOL)pollingEnabled {
	[self stopMidiTimer];
	_midiDelegate = newDelegate;
	if (_midiDelegate && queued && pollingEnabled) {
		[self startMidiTimer];
	}
}

- (void)receiveMidi {
	PDBASE_SETINSTANCE
	libpd_queued_receive_midi_messages();
}

- (void)startMidiTimer {
	midiPollTimer = [NSTimer timerWithTimeInterval:0.02 target:self selector:@selector(receiveMidiTimer:) userInfo:nil repeats:YES];
	[[NSRunLoop mainRunLoop] addTimer:midiPollTimer forMode:NSRunLoopCommonModes];
}

- (void)stopMidiTimer {
	if (midiPollTimer) {
		[midiPollTimer invalidate];
		midiPollTimer = nil;
	}
}

- (void)receiveMidiTimer:(NSTimer*)theTimer {
	PDBASE_SETINSTANCE
	libpd_queued_receive_midi_messages();
}

#pragma mark Log Level

- (void)setVerbose:(BOOL)verbose {
	PDBASE_SETINSTANCE
	libpd_set_verbose((int)verbose);
}

- (BOOL)getVerbose {
	PDBASE_SETINSTANCE
	return libpd_get_verbose();
}

@end
