//
//  PdInstance.h
//  libpd
//
//  Copyright (c) 2024 Dan Wilcox <danomatika@gmail.com>
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//
//  Adapted from original PdBase.h by Peter Brinkmann.
//

#import <Foundation/Foundation.h>

/// listener interface for messages from pd
@protocol PdListener<NSObject>
@optional

/// receive bang, source is the source receiver name
- (void)receiveBangFromSource:(NSString *)source;

/// receive float, source is the source receiver name
- (void)receiveFloat:(float)received fromSource:(NSString *)source;

/// receive symbol, source is the source receiver name
- (void)receiveSymbol:(NSString *)symbol fromSource:(NSString *)source;

/// receive list, source is the source receiver name
/// list is an NSArray whose data can be inspected & accessed with:
///     for (int i = 0; i < list.count; i++) {
///       id obj = list[i];
///       if ([obj isKindOfClass:NSNumber.class]) {
///         float x = [obj floatValue];
///         // do something with float x
///       } else if ([obj isKindOfClass:NSString.class]) {
///         NSString *s = obj;
///         // do something with string s
///       }
///     }
/// note: currently only float and symbol types are supported in pd lists
- (void)receiveList:(NSArray *)list fromSource:(NSString *)source;

/// receive typed message, source is the source receiver name and message is
/// the typed message name: a message like [; foo bar 1 2 a b( will trigger a
/// function call like [delegate receiveMessage:@"bar"
///                               withArguments:@[@(1), @(2), @"a", @"b"]
///                                  fromSource:@"foo"]
/// arguments is an NSArray whose data can be inspected & accessed with:
///     for (int i = 0; i < arguments.count; i++) {
///       id obj = arguments[i];
///       if ([obj isKindOfClass:NSNumber.class]) {
///         float x = [obj floatValue];
///         // do something with float x
///       } else if ([obj isKindOfClass:NSString.class]) {
///         NSString *s = obj;
///         // do something with string s
///       }
///     }
/// note: currently only float and symbol types are supported in pd lists
- (void)receiveMessage:(NSString *)message withArguments:(NSArray *)arguments
                                              fromSource:(NSString *)source;
@end

/// receiver interface for printing and receiving messages from pd
@protocol PdReceiverDelegate<PdListener>
@optional

/// receive print line, message is the string to be printed
- (void)receivePrint:(NSString *)message;
@end

#pragma mark -

/// listener interface for MIDI from pd
@protocol PdMidiListener<NSObject>
@optional

/// receive MIDI note on
/// channel is 0-indexed, pitch is 0-127, and value is 0-127
/// channels encode MIDI ports via: libpd_channel = pd_channel + 16 * pd_port
/// note: there is no note off message, note on w/ velocity = 0 is used instead
/// note: out of range values from pd are clamped
- (void)receiveNoteOn:(int)pitch withVelocity:(int)velocity
                                   forChannel:(int)channel;

/// receive MIDI control change
/// channel is 0-indexed, controller is 0-127, and value is 0-127
/// channels encode MIDI ports via: libpd_channel = pd_channel + 16 * pd_port
/// note: out of range values from pd are clamped
- (void)receiveControlChange:(int)value forController:(int)controller
                                           forChannel:(int)channel;

/// receive MIDI program change
/// channel is 0-indexed and value is 0-127
/// channels encode MIDI ports via: libpd_channel = pd_channel + 16 * pd_port
/// note: out of range values from pd are clamped
- (void)receiveProgramChange:(int)value forChannel:(int)channel;

/// receive MIDI pitch bend
/// channel is 0-indexed and value is -8192-8192
/// channels encode MIDI ports via: libpd_channel = pd_channel + 16 * pd_port
/// note: [bendin] outputs 0-16383 while [bendout] accepts -8192-8192
/// note: out of range values from pd are clamped
- (void)receivePitchBend:(int)value forChannel:(int)channel;

/// receive MIDI after touch
/// channel is 0-indexed and value is 0-127
/// channels encode MIDI ports via: libpd_channel = pd_channel + 16 * pd_port
/// note: out of range values from pd are clamped
- (void)receiveAftertouch:(int)value forChannel:(int)channel;

/// receive MIDI poly after touch
/// channel is 0-indexed, pitch is 0-127, and value is 0-127
/// channels encode MIDI ports via: libpd_channel = pd_channel + 16 * pd_port
/// note: out of range values from pd are clamped
- (void)receivePolyAftertouch:(int)value forPitch:(int)pitch
                                       forChannel:(int)channel;
@end

/// receiver interface for MIDI messages from pd
@protocol PdMidiReceiverDelegate<PdMidiListener>
@optional

/// receive raw MIDI byte
/// port is 0-indexed and byte is 0-256
/// note: out of range values from pd are clamped
- (void)receiveMidiByte:(int)byte forPort:(int)port;
@end

#pragma mark -

/// PdInstance: an instance wrapper for the libpd C API
///
/// behavior depends upon if libpd is compiled for single or multiple instances
/// * single instance mode (default):
///   - do not create PdInstances directly(!)
///   - use main instance via PdInstance.mainInstance,
///       ex. PdInstance *pd = PdInstance.mainInstance;
///           [pd setDelegate:self]; // ... do stuff with pd
///   - current "this" instance is always main instance: PdInstance.thisInstance
/// * multi instance mode: (define PDINSTANCE and PDTHREADS in CFLAGS)
///   - each PdInstance instance is unique
///   - shared main instance always valid via PdInstance.mainInstance
///   - current "this" instance is main instance by default
///   - most PdInstance methods set the current instance
///   - call [PdInstance setThisInstance] to explicitly set the current instance
///   - note: "this" instance is changed whenever a new PdInstance is created
@interface PdInstance : NSObject

#pragma mark Initializing Pd

/// create a new pd instance with message queuing
/// note: do not call this directly if compiling without PDINSTANCE,
///       use PdInstance.mainInstance to get the main instance
/// note: sets current "this" instance if compiled with PDINSTANCE
- (instancetype)init;

/// create a new pd instance with or without message queuing
/// note: do not call this directly if compiling without PDINSTANCE,
///       use [PdInstance initializeMainInstanceWithQueue:] and
///       PdInstance.mainInstance to get the main instance
/// note: sets current "this" instance if compiled with PDINSTANCE
///
/// for lowest latency, this will result in delegate receiver calls from the
/// audio thread directly which will probably require manual dispatch to the
/// main thread for anything UI-related otherwise there can be crashes or
/// exceptions
///
/// note: stops message polling if queue is NO
///
- (instancetype)initWithQueue:(BOOL)queue;

/// returns whether pd was initialized with message queuing
@property (nonatomic, readonly) BOOL isQueued;

/// add a path to the pd search paths
/// relative paths are relative to the current working directory
/// unlike desktop pd, *no* search paths are set by default (ie. extra)
- (void)addToSearchPath:(NSString *)path;

/// clear the pd search path for abstractions and externals
/// note: this is called when initializing
- (void)clearSearchPath;

#pragma mark Opening Patches

/// open a patch by filename and parent dir path
/// returns an opaque patch handle pointer or nil on failure
- (void *)openFile:(NSString *)baseName path:(NSString *)pathName;

/// close a patch by patch handle pointer
- (void)closeFile:(void *)x;

/// get the $0 id of the patch handle pointer
/// returns $0 value or 0 if the patch is non-existent
- (int)dollarZeroForFile:(void *)x;

#pragma mark Audio Processing

/// initialize audio rendering
/// returns 0 on success
- (int)openAudioWithSampleRate:(int)samplerate
                 inputChannels:(int)inputChannels
                outputChannels:(int)outputchannels;

/// process interleaved float samples from inputBuffer -> libpd -> outputBuffer
/// buffer sizes are based on # of ticks and channels where:
///     size = ticks * [PdBase getBlocksize] * (in/out)channels
/// returns 0 on success
- (int)processFloatWithInputBuffer:(const float *)inputBuffer
                      outputBuffer:(float *)outputBuffer
                             ticks:(int)ticks;

/// process interleaved short samples from inputBuffer -> libpd -> outputBuffer
/// buffer sizes are based on # of ticks and channels where:
///     size = ticks * [PdBase getBlocksize] * (in/out)channels
/// float samples are converted to short by multiplying by 32767 and casting,
/// so any values received from pd patches beyond -1 to 1 will result in garbage
/// note: for efficiency, does *not* clip input
/// returns 0 on success
- (int)processShortWithInputBuffer:(const short *)inputBuffer
                      outputBuffer:(short *)outputBuffer
                             ticks:(int)ticks;

/// process interleaved double samples from inputBuffer -> libpd -> outputBuffer
/// buffer sizes are based on # of ticks and channels where:
///     size = ticks * [PdBase getBlocksize]* (in/out)channels
/// returns 0 on success
- (int)processDoubleWithInputBuffer:(const double *)inputBuffer
                       outputBuffer:(double *)outputBuffer
                              ticks:(int)ticks;

/// enable/disable DSP, aka turn audio processing on/off
- (void)computeAudio:(BOOL)enable;

#pragma mark Array Access

/// get the size of an array by name
/// returns size or negative error code if non-existent
- (int)arraySizeForArrayNamed:(NSString *)arrayName;

/// (re)size an array by name, sizes <= 0 are clipped to 1
/// returns 0 on success or negative error code if non-existent
- (int)resizeArrayNamed:(NSString *)arrayName toSize:(long)size;

/// copy from a named pd array to a float array
/// returns 0 on success or a negative error code if the array is non-existent
/// or offset + n exceeds range of array
- (int)copyArrayNamed:(NSString *)arrayName withOffset:(int)offset
              toArray:(float *)destinationArray count:(int)n;

/// copy from a float array to a named pd array
/// returns 0 on success or a negative error code if the array is non-existent
/// or offset + n exceeds range of array
- (int)copyArray:(float *)sourceArray toArrayNamed:(NSString *)arrayName
      withOffset:(int)offset count:(int)n;

#pragma mark Sending Messages to Pd

/// send a bang to a destination receiver
/// returns 0 on success or -1 if receiver name is non-existent
/// ex: send a bang to [s foo] on the next tick with:
///     [pd sendBangToReceiver:@"foo"];
- (int)sendBangToReceiver:(NSString *)receiverName;

/// send a float to a destination receiver
/// returns 0 on success or -1 if receiver name is non-existent
/// ex: send a 1.0 to [s foo] on the next tick with:
///     [pd sendFloat:1 toReceiver:@"foo"];
- (int)sendFloat:(float)value toReceiver:(NSString *)receiverName;

/// send a symbol to a destination receiver
/// returns 0 on success or -1 if receiver name is non-existent
/// ex: send "bar" to [s foo] on the next tick with:
///     [pd sendSymbol:@"bar" toReceiver:@"foo"];
- (int)sendSymbol:(NSString *)symbol toReceiver:(NSString *)receiverName;

/// send a list to a destination receiver,
/// the list may be nil to specify an empty list
/// returns 0 on success or -1 if receiver name is non-existent
/// ex: send [list 1 2 bar( to [s foo] on the next tick with:
///     NSArray *list = @[@(1), @(2), @"bar", @"foo"];
///     [pd sendList:list toReceiver:@"foo"];
/// note: currently only float and symbol types are supported in pd lists
- (int)sendList:(NSArray *)list toReceiver:(NSString *)receiverName;

/// send a typed message, the list may be nil to specify an empty list

/// send as a typed message to a destination receiver,
/// the list may be nil to specify an empty list
/// note: typed message handling currently only supports up to 4 elements
///       internally, additional elements may be ignored
/// returns 0 on success or -1 if receiver name is non-existent
/// ex: send [; pd dsp 1( on the next tick with:
///     NSArray *list = @[@(1)];
///     [pd sendMessage:@"dsp" withArguments:list toReceiver:@"pd"];
/// note: currently only float and symbol types are supported in pd lists
- (int)sendMessage:(NSString *)message withArguments:(NSArray *)list
        toReceiver:(NSString *)receiverName;

#pragma mark Receiving Messages from Pd

/// subscribe to messages sent to a source receiver
/// returns an opaque receiver pointer or nil on failure

/// subscribe to messages sent to a source receiver
/// ex: [pd subscribe:@"foo"] adds a "virtual" [r foo] which forwards
/// messages to the PdMessageReceiver delegate
/// returns an opaque receiver pointer or NULL on failure
- (void *)subscribe:(NSString *)symbol;

/// unsubscribe and free a source receiver object created by subscribe
- (void)unsubscribe:(void *)subscription;

/// check if a source receiver object exists with a given name
- (BOOL)exists:(NSString *)symbol;

/// current receiver delegate
@property (nonatomic, weak) id<PdReceiverDelegate> delegate;

/// set receiver delegate and start with polling enabled,
/// setting to nil disconnects the existing delegate and turns off message
/// polling if it's running
///
/// polling is performed by an NSTimer using an interval which is "good enough"
/// for most cases, however if you need lower latency look into calling
/// receiveMessages: using a CADisplayLink or high resolution timer
///
/// for lowest latency, you can disable queing with initializeWithQueue NO
/// although this will result in delegate receiver calls from the audio thread
/// directly which may require manual dispatch to main thread for UI handling
///
/// note: call this from the main thread only
- (void)setDelegate:(id<PdReceiverDelegate>)newDelegate;

/// set the message receiver delegate,
/// setting to nil disconnects the existing delegate and turns off message
/// polling if it's running
/// note: call this from the main thread only
/// set pollingEnabled NO to process messages manually with receiveMessages
- (void)setDelegate:(id<PdReceiverDelegate>)newDelegate
     pollingEnabled:(BOOL)pollingEnabled;

/// process the message queue manually,
/// only required if the respective delegate was set with pollingEnabled NO
/// and queuing is enabled
- (void)receiveMessages;

#pragma mark Sending MIDI Messages to Pd

/// send a MIDI note on message to [notein] objects
/// channel is 0-indexed, pitch is 0-127, and velocity is 0-127
/// channels encode MIDI ports via: libpd_channel = pd_channel + 16 * pd_port
/// note: there is no note off message, send a note on with velocity = 0 instead
/// returns 0 on success or -1 if an argument is out of range
- (int)sendNoteOn:(int)channel pitch:(int)pitch velocity:(int)velocity;

/// send a MIDI control change message to [ctlin] objects
/// channel is 0-indexed, controller is 0-127, and value is 0-127
/// channels encode MIDI ports via: libpd_channel = pd_channel + 16 * pd_port
/// returns 0 on success or -1 if an argument is out of range
- (int)sendControlChange:(int)channel controller:(int)controller
                                           value:(int)value;

/// send a MIDI program change message to [pgmin] objects
/// channel is 0-indexed and value is 0-127
/// channels encode MIDI ports via: libpd_channel = pd_channel + 16 * pd_port
/// returns 0 on success or -1 if an argument is out of range
- (int)sendProgramChange:(int)channel value:(int)value;

/// send a MIDI pitch bend message to [bendin] objects
/// channel is 0-indexed and value is -8192-8192
/// channels encode MIDI ports via: libpd_channel = pd_channel + 16 * pd_port
/// note: [bendin] outputs 0-16383 while [bendout] accepts -8192-8192
/// returns 0 on success or -1 if an argument is out of range
- (int)sendPitchBend:(int)channel value:(int)value;

/// send a MIDI after touch message to [touchin] objects
/// channel is 0-indexed and value is 0-127
/// channels encode MIDI ports via: libpd_channel = pd_channel + 16 * pd_port
/// returns 0 on success or -1 if an argument is out of range
- (int)sendAftertouch:(int)channel value:(int)value;

/// send a MIDI poly after touch message to [polytouchin] objects
/// channel is 0-indexed, pitch is 0-127, and value is 0-127
/// channels encode MIDI ports via: libpd_channel = pd_channel + 16 * pd_port
/// returns 0 on success or -1 if an argument is out of range
- (int)sendPolyAftertouch:(int)channel pitch:(int)pitch
                                       value:(int)value;

/// send a raw MIDI byte to [midiin] objects
/// port is 0-indexed and byte is 0-256
/// returns 0 on success or -1 if an argument is out of range
- (int)sendMidiByte:(int)port byte:(int)byte;

/// send a raw MIDI byte to [sysexin] objects
/// port is 0-indexed and byte is 0-256
/// returns 0 on success or -1 if an argument is out of range
- (int)sendSysex:(int)port byte:(int)byte;

/// send a raw MIDI byte to [realtimein] objects
/// port is 0-indexed and byte is 0-256
/// returns 0 on success or -1 if an argument is out of range
- (int)sendSysRealTime:(int)port byte:(int)byte;

#pragma mark Receiving MIDI Messages from Pd

/// current MIDI receiver delegate
@property (nonatomic, weak) id<PdMidiReceiverDelegate> midiDelegate;

/// set midi receiver and start with polling enabled,
/// setting to nil disconnects the existing delegate and turns off message
/// polling if it's running
///
/// polling is performed by an NSTimer using an interval which is "good enough"
/// for most cases, however if you need lower latency look into calling
/// receiveMidiMessages: using a CADisplayLink or high resolution timer
///
/// for lowest latency, you can disable queuing with initializeWithQueue:NO
/// although this will result in delegate receiver calls from the audio thread
/// directly which may require manual dispatch to main thread for UI handling
///
/// note: call this from the main thread only
- (void)setMidiDelegate:(id<PdMidiReceiverDelegate>)newDelegate;

/// set the MIDI receiver delegate
/// setting to nil disconnects the existing delegate and turns off message
/// polling if it's running
/// note: call this from the main thread only
/// set pollingEnabled NO to process message queue manually with receiveMidi
- (void)setMidiDelegate:(id<PdMidiReceiverDelegate>)newDelegate
         pollingEnabled:(BOOL)pollingEnabled;

/// Process the MIDI message queue manually,
/// only required if the respective delegate was set with pollingEnabled NO
/// and queuing is enabled
- (void)receiveMidi;

#pragma mark Multiple Instances

/// returns YES if this is the main instance
@property (nonatomic, assign, readonly, getter=isMainInstance) BOOL mainInstance;

/// get the internal low-level t_pdinstance* pointer
/// returns "this" t_pdinstance pointer or main t_pdinstance pointer when libpd
/// is compiled without PDINSTANCE
@property (nonatomic, assign, readonly) void* pdinstance;

/// set the current pd instance to this instance for subsequent PdBase,
/// PdInstance.thisInstance, and libpd C API calls,
/// has no effect when libpd is compiled without PDINSTANCE
- (void)setThisInstance;

/// get the current pd instance, main instance by default
/// returns this or main instance when libpd is compiled without PDINSTANCE
+ (PdInstance *)thisInstance;

/// get the main instance, always valid
/// creates default queued instance, as needed
/// note: override with non-queued instance by calling
///       [PdInstance initializeMainInstanceWithQueue:NO]
+ (PdInstance *)mainInstance;

/// (re)initialize main instance with or without message queuing,
/// safe to call this more than once, overwrites main pd instance if changing
/// queue setting (so update any saved pointers), sets current instance
///
/// for lowest latency, this will result in delegate receiver calls from the
/// audio thread directly which will probably require manual dispatch to the
/// main thread for anything UI-related otherwise there can be crashes or
/// exceptions
///
/// note: stops message polling if queue is NO
///
/// returns 0 on success or -1 if libpd was already initialized
+ (int)initializeMainInstanceWithQueue:(BOOL)queue;

/// get the number of pd instances, including the main instance
/// returns number or 1 when libpd is compiled without PDINSTANCE
+ (int)numInstances;

@end
