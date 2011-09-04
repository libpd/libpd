//
//  PdDispatcher.m
//  libpd
//
//  Copyright (c) 2011 Peter Brinkmann (peter.brinkmann@gmail.com)
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//

#import "PdDispatcher.h"


@implementation PdDispatcher

- (id)init {
    self = [super init];
    if (self) {
        listenerMap = [[NSMutableDictionary alloc] init];
        subscriptions = [[NSMutableDictionary alloc] init];
    }
    return self;
}

- (void)dealloc {
    for (NSValue *handle in subscriptions) {
        void *ptr = [handle pointerValue];
        [PdBase unsubscribe:ptr];
    }
    [subscriptions release];
    [listenerMap release];
    [super dealloc];
}

- (int)addListener:(NSObject<PdListener> *)listener forSource:(NSString *)symbol {
    NSMutableArray *listeners = [listenerMap objectForKey:symbol];
    if (!listeners) {
        void *ptr = [PdBase subscribe:symbol];
        if (!ptr) {
            return -1;
        }
        NSValue *handle = [NSValue valueWithPointer:ptr];
        [subscriptions setObject:handle forKey:symbol];
        listeners = [[NSMutableArray alloc] init];
        [listenerMap setObject:listeners forKey:symbol];
        [listeners release];
    }
    [listeners addObject:listener];
    return 0;
}

- (int)removeListener:(NSObject<PdListener> *)listener forSource:(NSString *)symbol {
    NSMutableArray *listeners = [listenerMap objectForKey:symbol];
    if (listeners) {
        [listeners removeObject:listener];
        if ([listeners count] == 0) {
            NSValue *handle = [subscriptions objectForKey:symbol];
            void *ptr = [handle pointerValue];
            [PdBase unsubscribe:ptr];
            [subscriptions removeObjectForKey:symbol];
            [listenerMap removeObjectForKey:symbol];
        }
    }
    return 0;
}

// Override this method in subclasses if you want different printing behavior.
- (void)receivePrint:(NSString *)message {
    NSLog(@"Pd: %@\n", message);
}

- (void)receiveBangFromSource:(NSString *)source {
    NSArray *listeners = [listenerMap objectForKey:source];
    for (NSObject<PdListener> *listener in listeners) {
        [listener receiveBang];
    }
}

- (void)receiveFloat:(float)received fromSource:(NSString *)source {
    NSArray *listeners = [listenerMap objectForKey:source];
    for (NSObject<PdListener> *listener in listeners) {
        [listener receiveFloat:received];
    }
}

- (void)receiveSymbol:(NSString *)symbol fromSource:(NSString *)source {
    NSArray *listeners = [listenerMap objectForKey:source];
    for (NSObject<PdListener> *listener in listeners) {
        [listener receiveSymbol:symbol];
    }
}

- (void)receiveList:(NSArray *)list fromSource:(NSString *)source {
    NSArray *listeners = [listenerMap objectForKey:source];
    for (NSObject<PdListener> *listener in listeners) {
        [listener receiveList:list];
    }
}

- (void) receiveMessage:(NSString *)message withArguments:(NSArray *)arguments fromSource:(NSString *)source {
    NSArray *listeners = [listenerMap objectForKey:source];
    for (NSObject<PdListener> *listener in listeners) {
        [listener receiveMessage:message withArguments:arguments];
    }
}

@end


@implementation InvokeOnMainThread

- (id)initWithTarget:(id)t {
    self = [super init];
    if (self) {
        target = t;
        [target retain];
    }
    return self;
}

- (void)dealloc {
    [target release];
    [super dealloc];
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [target methodSignatureForSelector:aSelector];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    [anInvocation retainArguments];  // make sure the arguments survive until we make it to the main thread
    [anInvocation performSelectorOnMainThread:@selector(invokeWithTarget:) withObject:target waitUntilDone:NO];
}

+ (id)withTarget:(id)target {
    return [[[InvokeOnMainThread alloc] initWithTarget:target] autorelease];
}

@end
