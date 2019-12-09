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

#import <Foundation/Foundation.h>

/// Listener interface for messages from Pd.
@protocol PdListener<NSObject>
@optional
- (void)receiveBangFromSource:(NSString *)source;
- (void)receiveFloat:(float)received fromSource:(NSString *)source;
- (void)receiveSymbol:(NSString *)symbol fromSource:(NSString *)source;
- (void)receiveList:(NSArray *)list fromSource:(NSString *)source;
- (void)receiveMessage:(NSString *)message withArguments:(NSArray *)arguments
                                              fromSource:(NSString *)source;
@end

/// Receiver interface for printing and receiving messages from Pd.
@protocol PdReceiverDelegate<PdListener>
@optional
- (void)receivePrint:(NSString *)message;
@end

/// Listener interface for MIDI from Pd.
@protocol PdMidiListener<NSObject>
@optional
- (void)receiveNoteOn:(int)pitch withVelocity:(int)velocity
                                   forChannel:(int)channel;
- (void)receiveControlChange:(int)value forController:(int)controller
                                           forChannel:(int)channel;
- (void)receiveProgramChange:(int)value forChannel:(int)channel;
- (void)receivePitchBend:(int)value forChannel:(int)channel;
- (void)receiveAftertouch:(int)value forChannel:(int)channel;
- (void)receivePolyAftertouch:(int)value forPitch:(int)pitch
                                       forChannel:(int)channel;
@end

/// Receiver interface for MIDI messages from Pd.
@protocol PdMidiReceiverDelegate<PdMidiListener>
@optional
- (void)receiveMidiByte:(int)byte forPort:(int)port;
@end

/// PdBase: A class level wrapper for the libpd C API.
/// Not meant to be instantiated. No member variables.
@interface PdBase : NSObject

/// Class level intializer
+ (void)initialize;

/// \section Receiving
///
/// An NSTimer is used to poll for messages be default. However, the input
/// queues can be processed manually when setting the respective delegate using
/// pollingEnabled:NO.
///
/// The NSTimer's interval is *good enough* for most cases, however if you need
/// "low latency" messaging and MIDI, look into calling receiveMessages and
/// receiveMIDI using a CADisplayLink or a high resolution timer.

/// Set receiver delegate and start with polling enabled.
/// Setting to nil disconnects the existing delegate and turns off message
/// polling if it's running. Only to be called from main thread.
+ (void)setDelegate:(NSObject<PdReceiverDelegate> *)newDelegate;

/// Set midi receiver and start with polling enabled.
/// Setting to nil disconnects the existing delegate and turns off message
/// polling if it's running. Only to be called from main thread.
+ (void)setMidiDelegate:(NSObject<PdMidiReceiverDelegate> *)newDelegate;

/// Set the message receiver delegate.
/// Setting to nil disconnects the existing delegate and turns off message
/// polling if it's running. Only to be called from main thread.
/// Set pollingEnabled:NO to process messages manually with receiveMessages.
+ (void)setDelegate:(NSObject<PdReceiverDelegate> *)newDelegate
     pollingEnabled:(BOOL)pollingEnabled;

/// Set the MIDI receiver delegate.
/// Setting to nil disconnects the existing delegate and turns off message
/// polling if it's running. Only to be called from main thread.
/// Set pollingEnabled:NO to process messages manually with receiveMidi.
+ (void)setMidiDelegate:(NSObject<PdMidiReceiverDelegate> *)newDelegate
         pollingEnabled:(BOOL)pollingEnabled;

/// Current receiver delegate
+ (NSObject<PdReceiverDelegate> *)delegate;

/// Current midi receiver delegate
+ (NSObject<PdMidiReceiverDelegate> *)midiDelegate;

/// Process the message queue manually.
/// Only required if the respective delegate was set with pollingEnabled:NO.
+ (void)receiveMessages;

/// Process the midi message queue manually.
/// Only required if the respective delegate was set with pollingEnabled:NO.
+ (void)receiveMidi;

/// \section Paths

/// Add path to search paths.
+ (void)addToSearchPath:(NSString *)path;

/// Clear search paths.
+ (void)clearSearchPath;

/// \section Files

/// Open a pd file/patch and return a t_pd file pointer or nil on failure.
+ (void *)openFile:(NSString *)baseName path:(NSString *)pathName;

/// Close a pd file/patch using it's t_pd file pointer.
+ (void)closeFile:(void *)x;

/// Return the instance ID for pd file/patch using it's t_pd file pointer.
+ (int)dollarZeroForFile:(void *)x;

/// \section Audio

/// Return pd's fixed block size, basically the number of samples per 1 pd tick.
+ (int)getBlockSize;

/// Initialize libpd's audio settings.
+ (int)openAudioWithSampleRate:(int)samplerate
                 inputChannels:(int)inputChannels
				outputChannels:(int)outputchannels;

/// Process float input & output samples for a given number of ticks.
+ (int)processFloatWithInputBuffer:(const float *)inputBuffer
                      outputBuffer:(float *)outputBuffer
                             ticks:(int)ticks;

/// Process double input & output samples for a given number of ticks.
+ (int)processDoubleWithInputBuffer:(const double *)inputBuffer
					   outputBuffer:(double *)outputBuffer
					          ticks:(int)ticks;

/// Process short input & output samples for a given number of ticks.
+ (int)processShortWithInputBuffer:(const short *)inputBuffer
                      outputBuffer:(short *)outputBuffer
                             ticks:(int)ticks;

/// Enable/disable DSP aka turn audio on/off.
+ (void)computeAudio:(BOOL)enable;

/// \section Receivers

/// Start receiving messages from a receiver name.
+ (void *)subscribe:(NSString *)symbol;

/// Stop receiving messages from a receiver name.
+ (void)unsubscribe:(void *)subscription;

/// Does a receiver name exist?
+ (BOOL)exists:(NSString *)symbol;

/// \section Messages

/// Send a bang message.
+ (int)sendBangToReceiver:(NSString *)receiverName;

/// Send a float message.
+ (int)sendFloat:(float)value toReceiver:(NSString *)receiverName;

/// Send a symbol (string) message.
+ (int)sendSymbol:(NSString *)symbol toReceiver:(NSString *)receiverName;

/// Send a list message. The list may be nil to specify an empty list.
+ (int)sendList:(NSArray *)list toReceiver:(NSString *)receiverName;

/// Send a typed message. The list may be nil to specify an empty list.
+ (int)sendMessage:(NSString *)message withArguments:(NSArray *)list
        toReceiver:(NSString *)receiverName;

/// \section Array

/// Returns the size for a given array name or 0 if not found.
+ (int)arraySizeForArrayNamed:(NSString *)arrayName;

/// Copy from a named pd array to a float array.
+ (int)copyArrayNamed:(NSString *)arrayName withOffset:(int)offset
              toArray:(float *)destinationArray count:(int)n;

/// Copy from a float array to a named pd array.
+ (int)copyArray:(float *)sourceArray toArrayNamed:(NSString *)arrayName
	  withOffset:(int)offset count:(int)n;

/// \section MIDI

/// Send a MIDI note on message.
+ (int)sendNoteOn:(int)channel pitch:(int)pitch velocity:(int)velocity;

/// Send a MIDI control change message.
+ (int)sendControlChange:(int)channel controller:(int)controller
                                           value:(int)value;

/// Send a MIDI program change message.
+ (int)sendProgramChange:(int)channel value:(int)value;

/// Send a MIDI pitchbend message.
+ (int)sendPitchBend:(int)channel value:(int)value;

/// Send a MIDI after touch message.
+ (int)sendAftertouch:(int)channel value:(int)value;

/// Send a MIDI poly aftertouch message.
+ (int)sendPolyAftertouch:(int)channel pitch:(int)pitch
                                       value:(int)value;

/// Send a raw MIDI byte.
+ (int)sendMidiByte:(int)port byte:(int)byte;

/// Send a raw MIDI system exclusive byte.
+ (int)sendSysex:(int)port byte:(int)byte;

/// Send a raw MIDI realtime byte.
+ (int)sendSysRealTime:(int)port byte:(int)byte;

@end
