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
    }
    return self;
}

- (void)dealloc {
    [listenerMap release];
    [super dealloc];
}

- (int)addListener:(NSObject<PdListener> *)listener forSource:(NSString *)symbol {
    NSMutableArray *listeners = [listenerMap objectForKey:symbol];
    if (!listeners) {
        void *handle = [PdBase subscribe:symbol];
        if (!handle) {
            return -1;
        }
        listeners = [[NSMutableArray alloc] init];
        [listenerMap setObject:listeners forKey:symbol];
        [listeners release];
    }
    [listeners addObject:listener];
    return 0;
}

- (int)removeListener:(NSObject<PdListener> *)listener forSource:(NSString *)symbol {
    NSMutableArray *listeners = [listenerMap objectForKey:symbol];
    [listeners removeObject:listener];
    return 0;
}

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
