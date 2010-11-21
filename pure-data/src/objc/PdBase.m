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
    switch (a.a_type) {
      case A_FLOAT: {
        float x = a.a_w.w_float;  
        [list addObject:[NSNumber numberWithFloat:x]];
        break;
      }
      case A_SYMBOL: {
        char *s = a.a_w.w_symbol->s_name;  
        [list addObject:[NSString stringWithCString:s encoding:NSASCIIStringEncoding]];
        break;
      }
      default: {
        NSLog(@"PdBase: element type unknown: %i", a.a_type);
        break;
      }
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
  [delegate release];
  delegate = newDelegate;
  [delegate retain];
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

+ (void)sendBangToReceiver:(NSString *)receiverName {
  @synchronized(self) {
    libpd_bang([receiverName cStringUsingEncoding:NSASCIIStringEncoding]);
  }
}

+ (void)sendFloat:(float)value toReceiver:(NSString *)receiverName {
  @synchronized(self) {
    libpd_float([receiverName cStringUsingEncoding:NSASCIIStringEncoding], value);
  }
}

+ (void)sendSymbol:(NSString *)symbol toReceiver:(NSString *)receiverName {
  @synchronized(self) {
    libpd_symbol([receiverName cStringUsingEncoding:NSASCIIStringEncoding],
        [symbol cStringUsingEncoding:NSASCIIStringEncoding]);
  }
}

+ (void)sendList:(NSArray *)list toReceiver:(NSString *)receiverName {
  @synchronized(self) {
    libpd_start_message();
    encodeList(list);
    libpd_finish_list([receiverName cStringUsingEncoding:NSASCIIStringEncoding]);
  }
}

+ (void)sendMessage:(NSString *)message withArguments:(NSArray *)list toReceiver:(NSString *)receiverName {
  @synchronized(self) {
    libpd_start_message();
    encodeList(list);
    libpd_finish_message([receiverName cStringUsingEncoding:NSASCIIStringEncoding],
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

+ (NSString *)openPatch:(NSString *)path {
  NSString *pathToPatch = [path stringByDeletingLastPathComponent];
  NSString *patchFile = [path lastPathComponent];
  NSString *patchName = [NSString stringWithFormat:@"pd-%@", patchFile];
  if ([PdBase exists:patchName]) {
    NSLog(@"PdBase: patch is already open: %@", path);
    return nil;
  }
  [PdBase sendMessage:@"open" withArguments:[NSArray arrayWithObjects:patchFile, pathToPatch, nil]
      toReceiver:@"pd"];
  if (![PdBase exists:patchName]) {
    NSLog(@"PdBase: patch failed to open: %@", path);
    return nil;
  }
  return patchName;
}

+ (void)closePatch:(NSString *)patchName {
  [PdBase sendMessage:@"menuclose" withArguments:nil toReceiver:patchName];
}

@end
