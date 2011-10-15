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

#import "PdBase.h"
#include "z_libpd.h"


@implementation PdBase

static NSObject<PdReceiverDelegate> *delegate = nil;

static NSArray *decodeList(int argc, t_atom *argv) {
  NSMutableArray *list = [[[NSMutableArray alloc] initWithCapacity:argc] autorelease];
  for (int i = 0; i < argc; i++) {
    t_atom a = argv[i];
    if (libpd_is_float(a)) {
      float x = libpd_get_float(a);
      [list addObject:[NSNumber numberWithFloat:x]];
    } else if (libpd_is_symbol(a)) {
      const char *s = libpd_get_symbol(a);
      [list addObject:[NSString stringWithCString:s encoding:NSASCIIStringEncoding]];
    } else {
      NSLog(@"PdBase: element type unsupported: %i", a.a_type);
    }
  }
  return (NSArray *)list;
}

static void encodeList(NSArray *list) {
  for (int i = 0; i < [list count]; i++) {
    NSObject *object = [list objectAtIndex:i];
    
    if ([object isKindOfClass:[NSNumber class]]) {
      libpd_add_float([(NSNumber *)object floatValue]);
    } else if ([object isKindOfClass:[NSString class]]) {
      libpd_add_symbol([(NSString *)object cStringUsingEncoding:NSASCIIStringEncoding]);
    } else {
      NSLog(@"message not supported. %@", [object class]);
    }
  }
}

static void printHook(const char *s) {
  if ([delegate respondsToSelector:@selector(receivePrint:)]) {
    [delegate receivePrint:[NSString stringWithCString:s encoding:NSASCIIStringEncoding]];
  }
}

static void bangHook(const char *src) {
  if ([delegate respondsToSelector:@selector(receiveBangFromSource:)]) {
    [delegate receiveBangFromSource:[NSString stringWithCString:src encoding:NSASCIIStringEncoding]];
  }
}

static void floatHook(const char *src, float x) {
  if ([delegate respondsToSelector:@selector(receiveFloat:fromSource:)]) {
    [delegate receiveFloat:x fromSource:[NSString stringWithCString:src encoding:NSASCIIStringEncoding]];
  }
}

static void symbolHook(const char *src, const char *sym) {
  if ([delegate respondsToSelector:@selector(receiveSymbol:fromSource:)]) {
    [delegate receiveSymbol:[NSString stringWithCString:sym encoding:NSASCIIStringEncoding] 
        fromSource:[NSString stringWithCString:src encoding:NSASCIIStringEncoding]];
  }
}

static void listHook(const char *src, int argc, t_atom *argv) {
  if ([delegate respondsToSelector:@selector(receiveList:fromSource:)]) {
    NSArray *list = decodeList(argc, argv);
    [delegate receiveList:list fromSource:[NSString stringWithCString:src encoding:NSASCIIStringEncoding]];
  }
}

static void messageHook(const char *src, const char* sym, int argc, t_atom *argv) {
  if ([delegate respondsToSelector:@selector(receiveMessage:withArguments:fromSource:)]) {
    NSArray *list = decodeList(argc, argv);
    [delegate receiveMessage:[NSString stringWithCString:sym encoding:NSASCIIStringEncoding] 
        withArguments:list fromSource:[NSString stringWithCString:src encoding:NSASCIIStringEncoding]];
  }
}

+ (void)initialize {
  @synchronized(self) {    
    libpd_printhook = (t_libpd_printhook) printHook;
    libpd_banghook = (t_libpd_banghook) bangHook;
    libpd_floathook = (t_libpd_floathook) floatHook;
    libpd_symbolhook = (t_libpd_symbolhook) symbolHook;
    libpd_listhook = (t_libpd_listhook) listHook;
    libpd_messagehook = (t_libpd_messagehook) messageHook;   
    
    libpd_init();
  }
}

+ (void)setDelegate:(NSObject<PdReceiverDelegate> *)newDelegate {
  @synchronized(self) {
    [newDelegate retain];
    [delegate release];
    delegate = newDelegate;
  }
}

+ (void *)subscribe:(NSString *)symbol {
  @synchronized(self) {
    return libpd_bind([symbol cStringUsingEncoding:NSASCIIStringEncoding]);
  }
}

+ (void)unsubscribe:(void *)subscription {
  @synchronized(self) {
    libpd_unbind(subscription);
  }
}

+ (int)sendBangToReceiver:(NSString *)receiverName {
  @synchronized(self) {
    return libpd_bang([receiverName cStringUsingEncoding:NSASCIIStringEncoding]);
  }
}

+ (int)sendFloat:(float)value toReceiver:(NSString *)receiverName {
  @synchronized(self) {
    return libpd_float([receiverName cStringUsingEncoding:NSASCIIStringEncoding], value);
  }
}

+ (int)sendSymbol:(NSString *)symbol toReceiver:(NSString *)receiverName {
  @synchronized(self) {
    return libpd_symbol([receiverName cStringUsingEncoding:NSASCIIStringEncoding],
                        [symbol cStringUsingEncoding:NSASCIIStringEncoding]);
  }
}

+ (int)sendList:(NSArray *)list toReceiver:(NSString *)receiverName {
  @synchronized(self) {
    if (libpd_start_message([list count])) return -100;
    encodeList(list);
    return libpd_finish_list([receiverName cStringUsingEncoding:NSASCIIStringEncoding]);
  }
}

+ (int)sendMessage:(NSString *)message withArguments:(NSArray *)list toReceiver:(NSString *)receiverName {
  @synchronized(self) {
    if (libpd_start_message([list count])) return -100;
    encodeList(list);
    return libpd_finish_message([receiverName cStringUsingEncoding:NSASCIIStringEncoding],
                                [message cStringUsingEncoding:NSASCIIStringEncoding]);
  }
}

+ (void)clearSearchPath {
  @synchronized(self) {
    libpd_clear_search_path();
  }
}

+ (void)addToSearchPath:(NSString *)path {
  @synchronized(self) {
    libpd_add_to_search_path([path cStringUsingEncoding:NSASCIIStringEncoding]);
  }
}

+ (int)getBlockSize {
  @synchronized(self) {
    return libpd_blocksize();
  }
}

+ (BOOL)exists:(NSString *)symbol {
  @synchronized(self) {
    return (BOOL) libpd_exists([symbol cStringUsingEncoding:NSASCIIStringEncoding]);
  }
}

+ (int)openAudioWithSampleRate:(int)samplerate andInputChannels:(int)inputChannels 
    andOutputChannels:(int)outputchannels andTicksPerBuffer:(int)ticksPerBuffer {
  @synchronized(self) {
    return libpd_init_audio(inputChannels, outputchannels, samplerate, ticksPerBuffer);
  }
}

+ (int)processFloatWithInputBuffer:(float *)inputBuffer andOutputBuffer:(float *)outputBuffer {
  @synchronized(self) {
    return libpd_process_float(inputBuffer, outputBuffer);
  }
}

+ (int)processDoubleWithInputBuffer:(double *)inputBuffer andOutputBuffer:(double *)outputBuffer {
  @synchronized(self) {
    return libpd_process_double(inputBuffer, outputBuffer);
  }
}

+ (int)processShortWithInputBuffer:(short *)inputBuffer andOutputBuffer:(short *)outputBuffer {
  @synchronized(self) {
    return libpd_process_short(inputBuffer, outputBuffer);
  }
}

+ (void)computeAudio:(BOOL)enable {
  [PdBase sendMessage:@"dsp" withArguments:[NSArray arrayWithObject:[NSNumber numberWithBool:enable]]
      toReceiver:@"pd"];
}

+ (void *)openFile:(NSString *)baseName path:(NSString *)pathName {
  @synchronized(self) {
    const char *base = [baseName cStringUsingEncoding:NSASCIIStringEncoding];
    const char *path = [pathName cStringUsingEncoding:NSASCIIStringEncoding];
		return libpd_openfile(base, path);
  }
}

+ (void)closeFile:(void *)x {
  @synchronized(self) {
    libpd_closefile(x);
  }
}

+ (int)dollarZeroForFile:(void *)x {
  @synchronized(self) {
    return libpd_getdollarzero(x);
  }
}

+ (int)arraySizeForArrayNamed:(NSString *)arrayName {
  @synchronized(self) {
    return libpd_arraysize([arrayName cStringUsingEncoding:NSASCIIStringEncoding]);
  }
}

+ (int)copyArrayNamed:(NSString *)arrayName withOffset:(int)offset toArray:(float *)destinationArray count:(int)n {
  @synchronized(self) {
    const char *name = [arrayName cStringUsingEncoding:NSASCIIStringEncoding];
    return libpd_read_array(destinationArray, name, offset, n);
  }
}

+ (int)copyArray:(float *)sourceArray toArrayNamed:(NSString *)arrayName withOffset:(int)offset count:(int)n {
  @synchronized(self) {
    const char *name = [arrayName cStringUsingEncoding:NSASCIIStringEncoding];
    return libpd_write_array(name, offset, sourceArray, n);
  }
}

+ (int)sendNoteOn:(int)channel pitch:(int)pitch velocity:(int)velocity {
  @synchronized(self) {
    return libpd_noteon(channel, pitch, velocity);
  }
}

+ (int)sendControlChange:(int)channel controller:(int)controller value:(int)value {
  @synchronized(self) {
    return libpd_controlchange(channel, controller, value);
  }
}

+ (int)sendProgramChange:(int)channel value:(int)value {
  @synchronized(self) {
    return libpd_programchange(channel, value);
  }
}

+ (int)sendPitchBend:(int)channel value:(int)value {
  @synchronized(self) {
    return libpd_pitchbend(channel, value);
  }
}

+ (int)sendAftertouch:(int)channel value:(int)value {
  @synchronized(self) {
    return libpd_aftertouch(channel, value);
  }
}

+ (int)sendPolyAftertouch:(int)channel pitch:(int)pitch value:(int)value {
  @synchronized(self) {
    return libpd_polyaftertouch(channel, pitch, value);
  }
}

+ (int)sendMidiByte:(int)port byte:(int)byte {
  @synchronized(self) {
    return libpd_midibyte(port, byte);
  }
}

+ (int)sendSysex:(int)port byte:(int)byte {
  @synchronized(self) {
    return libpd_sysex(port, byte);
  }
}

+ (int)sendSysRealTime:(int)port byte:(int)byte {
  @synchronized(self) {
    return libpd_sysrealtime(port, byte);
  }
}

@end
