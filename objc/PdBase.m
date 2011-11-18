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
#import "ringbuffer.h"
#include "z_libpd.h"


static NSObject<PdReceiverDelegate> *delegate = nil;
static volatile ring_buffer *ringBuffer = NULL;
static void *tempBuffer = NULL;

static NSArray *decodeList(int argc, t_atom *argv) {
  NSMutableArray *list = [[NSMutableArray alloc] initWithCapacity:argc];
  for (int i = 0; i < argc; i++) {
    t_atom a = argv[i];
    if (libpd_is_float(a)) {
      float x = libpd_get_float(a);
      NSNumber *num = [[NSNumber alloc] initWithFloat:x];
      [list addObject:num];
      [num release];
    } else if (libpd_is_symbol(a)) {
      const char *s = libpd_get_symbol(a);
      NSString *str = [[NSString alloc] initWithCString:s encoding:NSASCIIStringEncoding];
      [list addObject:str];
      [str release];
    } else {
      NSLog(@"PdBase: element type unsupported: %i", a.a_type);
    }
  }
  return (NSArray *)list; // The receiver owns the array and is responsible for releasing it.
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

typedef struct _params {
  enum {
    PRINT, BANG, FLOAT, SYMBOL, LIST, MESSAGE
  } type;
  const char *src;
  float x;
  const char *sym;
  int argc;
} params;

#define S_PARAMS sizeof(params)
#define S_ATOM sizeof(t_atom)

static void printHook(const char *s) {
  int len = strlen(s) + 1; // remember terminating null char
  if (rb_available_to_write(ringBuffer) >= S_PARAMS + len) {
    params p = {PRINT, NULL, 0.0f, NULL, len};
    rb_write_to_buffer(ringBuffer, &p, S_PARAMS);
    rb_write_to_buffer(ringBuffer, s, len);
  }
}

static void evaluatePrintMessage(params *p, void **buffer) {
  if ([delegate respondsToSelector:@selector(receivePrint:)]) {
    NSString *s = [[NSString alloc] initWithCString:*buffer encoding:NSASCIIStringEncoding];
    [delegate receivePrint:s];
    [s release];
  }
  *buffer += p->argc;
}

static void bangHook(const char *src) {
  if (rb_available_to_write(ringBuffer) >= S_PARAMS) {
    params p = {BANG, src, 0.0f, NULL, 0};
    rb_write_to_buffer(ringBuffer, &p, S_PARAMS);
  }
}

static void evaluateBangMessage(params *p, void **buffer) {
  if ([delegate respondsToSelector:@selector(receiveBangFromSource:)]) {
    NSString *src = [[NSString alloc] initWithCString:p->src encoding:NSASCIIStringEncoding];
    [delegate receiveBangFromSource:src];
    [src release];
  }
}

static void floatHook(const char *src, float x) {
  if (rb_available_to_write(ringBuffer) >= S_PARAMS) {
    params p = {FLOAT, src, x, NULL, 0};
    rb_write_to_buffer(ringBuffer, &p, S_PARAMS);
  }
}

static void evaluateFloatMessage(params *p, void **buffer) {
  if ([delegate respondsToSelector:@selector(receiveFloat:fromSource:)]) {
    NSString *src = [[NSString alloc] initWithCString:p->src encoding:NSASCIIStringEncoding];
    [delegate receiveFloat:p->x fromSource:src];
    [src release];
  }
}

static void symbolHook(const char *src, const char *sym) {
  if (rb_available_to_write(ringBuffer) >= S_PARAMS) {
    params p = {SYMBOL, src, 0.0f, sym, 0};
    rb_write_to_buffer(ringBuffer, &p, S_PARAMS);
  }
}

static void evaluateSymbolMessage(params *p, void **buffer) {
  if ([delegate respondsToSelector:@selector(receiveSymbol:fromSource:)]) {
    NSString *src = [[NSString alloc] initWithCString:p->src encoding:NSASCIIStringEncoding];
    NSString *sym = [[NSString alloc] initWithCString:p->sym encoding:NSASCIIStringEncoding];
    [delegate receiveSymbol:sym fromSource:src];
    [src release];
    [sym release];
  }
}

static void listHook(const char *src, int argc, t_atom *argv) {
  int n = argc * S_ATOM;
  if (rb_available_to_write(ringBuffer) >= S_PARAMS + n) {
    params p = {LIST, src, 0.0f, NULL, argc};
    rb_write_to_buffer(ringBuffer, &p, S_PARAMS);
    rb_write_to_buffer(ringBuffer, argv, n);
  }
}

static void evaluateListMessage(params *p, void **buffer) {
  if ([delegate respondsToSelector:@selector(receiveList:fromSource:)]) {
    NSString *src = [[NSString alloc] initWithCString:p->src encoding:NSASCIIStringEncoding];
    NSArray *args = decodeList(p->argc, *buffer);
    [delegate receiveList:args fromSource:src];
    [src release];
    [args release];
  }
  *buffer += p->argc * S_ATOM;
}

static void messageHook(const char *src, const char* sym, int argc, t_atom *argv) {
  int n = argc * S_ATOM;
  if (rb_available_to_write(ringBuffer) >= S_PARAMS + n) {
    params p = {MESSAGE, src, 0.0f, sym, argc};
    rb_write_to_buffer(ringBuffer, &p, S_PARAMS);
    rb_write_to_buffer(ringBuffer, argv, n);
  }
}

static void evaluateTypedMessage(params *p, void **buffer) {
  if ([delegate respondsToSelector:@selector(receiveMessage:withArguments:fromSource:)]) {
    NSString *src = [[NSString alloc] initWithCString:p->src encoding:NSASCIIStringEncoding];
    NSString *sym = [[NSString alloc] initWithCString:p->sym encoding:NSASCIIStringEncoding];
    NSArray *args = decodeList(p->argc, *buffer);
    [delegate receiveMessage:sym withArguments:args fromSource:src];
    [src release];
    [sym release];
    [args release];
  }
  *buffer += p->argc * S_ATOM;
}

@interface PdMessageHandler : NSObject {}
-(void)pollQueue:(NSTimer *)timer;
@end

@implementation PdMessageHandler

-(void)pollQueue:(NSTimer *)timer {
  size_t available = rb_available_to_read(ringBuffer);
  if (!available) return;
  rb_read_from_buffer(ringBuffer, tempBuffer, available);
  void *end = tempBuffer + available;
  void *buffer = tempBuffer;
  while (buffer < end) {
    params *p = buffer;
    buffer += S_PARAMS;
    switch (p->type) {
      case PRINT: {
        evaluatePrintMessage(p, &buffer);
        break;
      }
      case BANG: {
        evaluateBangMessage(p, &buffer);
        break;
      }
      case FLOAT: {
        evaluateFloatMessage(p, &buffer);
        break;
      }
      case SYMBOL: {
        evaluateSymbolMessage(p, &buffer);
        break;
      }
      case LIST: {
        evaluateListMessage(p, &buffer);
        break;
      }
      case MESSAGE: {
        evaluateTypedMessage(p, &buffer);
        break;
      }
      default:
        break;
    }
  }
}

@end

static NSTimer *pollTimer;
static PdMessageHandler *messageHandler;


@implementation PdBase

+ (void)initialize {
  libpd_printhook = (t_libpd_printhook) printHook;
  libpd_banghook = (t_libpd_banghook) bangHook;
  libpd_floathook = (t_libpd_floathook) floatHook;
  libpd_symbolhook = (t_libpd_symbolhook) symbolHook;
  libpd_listhook = (t_libpd_listhook) listHook;
  libpd_messagehook = (t_libpd_messagehook) messageHook;   
  
  messageHandler = [[PdMessageHandler alloc] init];
  libpd_init();
}

// Only to called from main thread.
+ (size_t)setMessageBufferSize:(size_t)size {
  if (!ringBuffer) {
    ringBuffer = rb_create(size);
    if (!ringBuffer) return 0;
    tempBuffer = malloc(size);
    if (!tempBuffer) {
      rb_free(ringBuffer);
      ringBuffer = NULL;
      return 0;
    }
  }
  return ringBuffer->size;
}

// Only to be called from main thread.
+ (void)setDelegate:(NSObject<PdReceiverDelegate> *)newDelegate {
  if (newDelegate == delegate) return;
  if (!newDelegate) {
    [pollTimer invalidate]; // This also releases the timer.
    pollTimer = nil;
  } else {
    [self setMessageBufferSize:32768]; // Will do nothing if buffer is already initialized.
  }
  [newDelegate retain];
  [delegate release];
  delegate = newDelegate;
  if (delegate && !pollTimer) {
    pollTimer = [NSTimer scheduledTimerWithTimeInterval:0.02 target:messageHandler selector:@selector(pollQueue:) userInfo:nil repeats:YES];
  }
}

// Only to be initialized from main thread.
+ (NSObject<PdReceiverDelegate> *)getDelegate {
  return delegate;
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
  NSNumber *val = [[NSNumber alloc] initWithBool:enable];
  NSArray *args = [[NSArray alloc] initWithObjects:val, nil];
  [PdBase sendMessage:@"dsp" withArguments:args toReceiver:@"pd"];
  [args release];
  [val release];
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

@end
