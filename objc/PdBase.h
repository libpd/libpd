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
 */

#import <Foundation/Foundation.h>

@protocol PdReceiverDelegate

@optional
- (void)receivePrint:(NSString *)message;
- (void)receiveBangFromSource:(NSString *)source;
- (void)receiveFloat:(float)received fromSource:(NSString *)source;
- (void)receiveSymbol:(NSString *)symbol fromSource:(NSString *)source;
- (void)receiveList:(NSArray *)list fromSource:(NSString *)source;
- (void)receiveMessage:(NSString *)message withArguments:(NSArray *)arguments fromSource:(NSString *)source;

@end

@interface PdBase {
  // Not meant to be instantiated. No member variables.
}

/** http://www.gitorious.org/pdlib/pages/Libpd */

+ (void)initialize;
/** PdBase retains the delegate: call setDelegate with nil in order to release delegate. */
+ (void)setDelegate:(NSObject<PdReceiverDelegate> *)newDelegate;
+ (void *)subscribe:(NSString *)symbol;
+ (void)unsubscribe:(void *)subscription;
+ (int)sendBangToReceiver:(NSString *)receiverName;
+ (int)sendFloat:(float)value toReceiver:(NSString *)receiverName;
+ (int)sendSymbol:(NSString *)symbol toReceiver:(NSString *)receiverName;
+ (int)sendList:(NSArray *)list toReceiver:(NSString *)receiverName;
+ (int)sendMessage:(NSString *)message withArguments:(NSArray *)list toReceiver:(NSString *)receiverName;
+ (void)clearSearchPath;
+ (void)addToSearchPath:(NSString *)path;
+ (int)getBlockSize;
+ (BOOL)exists:(NSString *)symbol;
+ (int)openAudioWithSampleRate:(int)samplerate andInputChannels:(int)inputChannels 
    andOutputChannels:(int)outputchannels andTicksPerBuffer:(int)ticksPerBuffer;
+ (int)processFloatWithInputBuffer:(float *)inputBuffer andOutputBuffer:(float *)outputBuffer;
+ (int)processDoubleWithInputBuffer:(double *)inputBuffer andOutputBuffer:(double *)outputBuffer;
+ (int)processShortWithInputBuffer:(short *)inputBuffer andOutputBuffer:(short *)outputBuffer;
+ (void)computeAudio:(BOOL)enable;
+ (void *)openFile:(NSString *)baseName path:(NSString *)pathName;
+ (void)closeFile:(void *)x;
+ (int)dollarZeroForFile:(void *)x;
+ (int)arraySizeForArrayNamed:(NSString *)arrayName;
+ (int)copyArrayNamed:(NSString *)arrayName withOffset:(int)offset toArray:(float *)destinationArray count:(int)n;
+ (int)copyArray:(float *)sourceArray toArrayNamed:(NSString *)arrayName withOffset:(int)offset count:(int)n;

@end
