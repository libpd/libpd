/**
 * This software is copyrighted by Reality Jockey Ltd. and Peter Brinkmann.
 * The following terms (the "Standard Improved BSD License") apply to
 * all files associated with the software unless explicitly disclaimed
 * in individual files:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided
 * with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 * products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Updated 2013 Dan Wilcox (danomatika@gmail.com)
 *
 */

#import <Foundation/Foundation.h>


// Listener interface for messages from Pd.
@protocol PdListener<NSObject>
@optional
- (void)receiveBangFromSource:(NSString *)source;
- (void)receiveFloat:(float)received fromSource:(NSString *)source;
- (void)receiveSymbol:(NSString *)symbol fromSource:(NSString *)source;
- (void)receiveList:(NSArray *)list fromSource:(NSString *)source;
- (void)receiveMessage:(NSString *)message withArguments:(NSArray *)arguments fromSource:(NSString *)source;
@end

// Receiver interface for printing and receiving messages from Pd.
@protocol PdReceiverDelegate<PdListener>
@optional
- (void)receivePrint:(NSString *)message;
@end


// Listener interface for MIDI from Pd.
@protocol PdMidiListener<NSObject>
@optional
- (void)receiveNoteOn:(int)pitch withVelocity:(int)velocity forChannel:(int)channel;
- (void)receiveControlChange:(int)value forController:(int)controller forChannel:(int)channel;
- (void)receiveProgramChange:(int)value forChannel:(int)channel;
- (void)receivePitchBend:(int)value forChannel:(int)channel;
- (void)receiveAftertouch:(int)value forChannel:(int)channel;
- (void)receivePolyAftertouch:(int)value forPitch:(int)pitch forChannel:(int)channel;
@end


// Receiver interface for MIDI messages from Pd.
@protocol PdMidiReceiverDelegate<PdMidiListener>
@optional
- (void)receiveMidiByte:(int)byte forPort:(int)port;
@end


@interface PdBase : NSObject {
  // Not meant to be instantiated. No member variables.
}

+ (void)initialize;

// Set the delegates to receive messages and midi. Only to be called from main thread.
// An NSTimer is used to poll for messages be default. However, the input queues can be processed
// manually when setting the respective delegate using pollingEnabled:NO.
//
// Setting the delegate to nil disconnects the existing delegate and turns off message polling if it's running.
//
// Note: PdBase retains the delegates: call setDelegate or setMidiDelegate with nil in order to release delegate.
+ (void)setDelegate:(NSObject<PdReceiverDelegate> *)newDelegate pollingEnabled:(BOOL)pollingEnabled;
+ (void)setMidiDelegate:(NSObject<PdMidiReceiverDelegate> *)newDelegate pollingEnabled:(BOOL)pollingEnabled;
+ (void)setDelegate:(NSObject<PdReceiverDelegate> *)newDelegate;         // Starts with polling enabled.
+ (void)setMidiDelegate:(NSObject<PdMidiReceiverDelegate> *)newDelegate; // Starts with polling enabled.

+ (NSObject<PdReceiverDelegate> *)delegate;
+ (NSObject<PdMidiReceiverDelegate> *)midiDelegate;

// Process the message and midi input queues manually.
// Only required if the respective delegate was set with pollingEnabled:NO.
+ (void)receiveMessages;
+ (void)receiveMidi;

+ (void)clearSearchPath;
+ (void)addToSearchPath:(NSString *)path;

+ (void *)openFile:(NSString *)baseName path:(NSString *)pathName;
+ (void)closeFile:(void *)x;
+ (int)dollarZeroForFile:(void *)x;

+ (int)getBlockSize;
+ (int)openAudioWithSampleRate:(int)samplerate
    inputChannels:(int)inputChannels
    outputChannels:(int)outputchannels;
+ (int)processFloatWithInputBuffer:(const float *)inputBuffer
    outputBuffer:(float *)outputBuffer ticks:(int)ticks;
+ (int)processDoubleWithInputBuffer:(const double *)inputBuffer
    outputBuffer:(double *)outputBuffer ticks:(int)ticks;
+ (int)processShortWithInputBuffer:(const short *)inputBuffer
    outputBuffer:(short *)outputBuffer ticks:(int)ticks;
+ (void)computeAudio:(BOOL)enable;
+ (void *)subscribe:(NSString *)symbol;
+ (void)unsubscribe:(void *)subscription;
+ (BOOL)exists:(NSString *)symbol;

+ (int)sendBangToReceiver:(NSString *)receiverName;
+ (int)sendFloat:(float)value toReceiver:(NSString *)receiverName;
+ (int)sendSymbol:(NSString *)symbol toReceiver:(NSString *)receiverName;
+ (int)sendList:(NSArray *)list toReceiver:(NSString *)receiverName;  // list may be nil
+ (int)sendMessage:(NSString *)message withArguments:(NSArray *)list toReceiver:(NSString *)receiverName;  // list may be nil

+ (int)arraySizeForArrayNamed:(NSString *)arrayName;
+ (int)copyArrayNamed:(NSString *)arrayName withOffset:(int)offset toArray:(float *)destinationArray count:(int)n;
+ (int)copyArray:(float *)sourceArray toArrayNamed:(NSString *)arrayName withOffset:(int)offset count:(int)n;

+ (int)sendNoteOn:(int)channel pitch:(int)pitch velocity:(int)velocity;
+ (int)sendControlChange:(int)channel controller:(int)controller value:(int)value;
+ (int)sendProgramChange:(int)channel value:(int)value;
+ (int)sendPitchBend:(int)channel value:(int)value;
+ (int)sendAftertouch:(int)channel value:(int)value;
+ (int)sendPolyAftertouch:(int)channel pitch:(int)pitch value:(int)value;
+ (int)sendMidiByte:(int)port byte:(int)byte;
+ (int)sendSysex:(int)port byte:(int)byte;
+ (int)sendSysRealTime:(int)port byte:(int)byte;

@end
