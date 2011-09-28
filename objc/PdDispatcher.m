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
    @synchronized(self) {
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
}

- (int)removeListener:(NSObject<PdListener> *)listener forSource:(NSString *)symbol {
    @synchronized(self) {
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
}

// Override this method in subclasses if you want different printing behavior.
// No need to synchronize here.
- (void)receivePrint:(NSString *)message {
    NSLog(@"Pd: %@\n", message);
}

- (void)receiveBangFromSource:(NSString *)source {
    @synchronized(self) {
        NSArray *listeners = [listenerMap objectForKey:source];
        for (NSObject<PdListener> *listener in listeners) {
            [listener receiveBang];
        }
    }
}

- (void)receiveFloat:(float)received fromSource:(NSString *)source {
    @synchronized(self) {
        NSArray *listeners = [listenerMap objectForKey:source];
        for (NSObject<PdListener> *listener in listeners) {
            [listener receiveFloat:received];
        }
    }
}

- (void)receiveSymbol:(NSString *)symbol fromSource:(NSString *)source {
    @synchronized(self) {
        NSArray *listeners = [listenerMap objectForKey:source];
        for (NSObject<PdListener> *listener in listeners) {
            [listener receiveSymbol:symbol];
        }
    }
}

- (void)receiveList:(NSArray *)list fromSource:(NSString *)source {
    @synchronized(self) {
        NSArray *listeners = [listenerMap objectForKey:source];
        for (NSObject<PdListener> *listener in listeners) {
            [listener receiveList:list];
        }
    }
}

- (void) receiveMessage:(NSString *)message withArguments:(NSArray *)arguments fromSource:(NSString *)source {
    @synchronized(self) {
        NSArray *listeners = [listenerMap objectForKey:source];
        for (NSObject<PdListener> *listener in listeners) {
            [listener receiveMessage:message withArguments:arguments];
        }
    }
}

@end


#import "dispatch/dispatch.h"

#define ON_MAIN_THREAD(f) \
    dispatch_async(dispatch_get_main_queue(), ^(void) { f; });

@implementation PdUiDispatcher

-(void)receiveBangFromSource:(NSString *)source {
    ON_MAIN_THREAD([super receiveBangFromSource:source]);
}

-(void)receiveFloat:(float)received fromSource:(NSString *)source {
    ON_MAIN_THREAD([super receiveFloat:received fromSource:source]);
}

-(void)receiveSymbol:(NSString *)symbol fromSource:(NSString *)source {
    ON_MAIN_THREAD([super receiveSymbol:symbol fromSource:source]);
}

-(void)receiveList:(NSArray *)list fromSource:(NSString *)source {
    ON_MAIN_THREAD([super receiveList:list fromSource:source]);
}

-(void)receiveMessage:(NSString *)message withArguments:(NSArray *)arguments fromSource:(NSString *)source {
    ON_MAIN_THREAD([super receiveMessage:message withArguments:arguments fromSource:source]);
}

@end
