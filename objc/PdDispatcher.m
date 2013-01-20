//
//  PdDispatcher.m
//  libpd
//
//  Copyright (c) 2011 Peter Brinkmann (peter.brinkmann@gmail.com)
//  Updated       2013 Dan Wilcox (danomatika@gmail.com)
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
    for (NSValue *handle in [subscriptions allValues]) {
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

- (void)removeAllListeners {
	[listenerMap removeAllObjects];
	NSEnumerator *enumerator = [subscriptions objectEnumerator];
	id object;
	while (object = [enumerator nextObject]) {
		NSValue *handle = object;
		void *ptr = [handle pointerValue];
		[PdBase unsubscribe:ptr];
	}
	[subscriptions removeAllObjects];
}

// Override this method in subclasses if you want different printing behavior.
// No need to synchronize here.
- (void)receivePrint:(NSString *)message {
    NSLog(@"Pd: %@\n", message);
}

- (void)receiveBangFromSource:(NSString *)source {
    NSArray *listeners = [listenerMap objectForKey:source];
    for (NSObject<PdListener> *listener in listeners) {
        if ([listener respondsToSelector:@selector(receiveBangFromSource:)]) {
            [listener receiveBangFromSource:source];
        } else {
            NSLog(@"Unhandled bang from %@", source);
        }
    }
}

- (void)receiveFloat:(float)received fromSource:(NSString *)source {
    NSArray *listeners = [listenerMap objectForKey:source];
    for (NSObject<PdListener> *listener in listeners) {
        if ([listener respondsToSelector:@selector(receiveFloat:fromSource:)]) {
            [listener receiveFloat:received fromSource:source];
        } else {
            NSLog(@"Unhandled float from %@", source);
        }
    }
}

- (void)receiveSymbol:(NSString *)symbol fromSource:(NSString *)source {
    NSArray *listeners = [listenerMap objectForKey:source];
    for (NSObject<PdListener> *listener in listeners) {
        if ([listener respondsToSelector:@selector(receiveSymbol:fromSource:)]) {
            [listener receiveSymbol:symbol fromSource:source];
        } else {
            NSLog(@"Unhandled symbol from %@", source);
        }
    }
}

- (void)receiveList:(NSArray *)list fromSource:(NSString *)source {
    NSArray *listeners = [listenerMap objectForKey:source];
    for (NSObject<PdListener> *listener in listeners) {
        if ([listener respondsToSelector:@selector(receiveList:fromSource:)]) {
            [listener receiveList:list fromSource:source];
        } else {
            NSLog(@"Unhandled list from %@", source);
        }
    }
}

- (void) receiveMessage:(NSString *)message withArguments:(NSArray *)arguments fromSource:(NSString *)source {
    NSArray *listeners = [listenerMap objectForKey:source];
    for (NSObject<PdListener> *listener in listeners) {
        if ([listener respondsToSelector:@selector(receiveMessage:withArguments:fromSource:)]) {
            [listener receiveMessage:message withArguments:arguments fromSource:source];
        } else {
            NSLog(@"Unhandled typed message from %@", source);
        }
    }
}

@end


@implementation PdMidiDispatcher

- (id)init {
    self = [super init];
    if (self) {
        listenerMap = [[NSMutableDictionary alloc] init];
        channels = [[NSMutableSet alloc] init];
    }
    return self;
}

- (void)dealloc {
	[channels removeAllObjects];
	[listenerMap removeAllObjects];
    [channels release];
    [listenerMap release];
    [super dealloc];
}

- (int)addListener:(NSObject<PdListener> *)listener forChannel:(int)channel {
    NSMutableArray *listeners = [listenerMap objectForKey:[NSNumber numberWithInt:channel]];
    if (!listeners) {
        if(![channels member:[NSNumber numberWithInt:channel]]) {
			[channels addObject:[NSNumber numberWithInt:channel]];
		}
        listeners = [[NSMutableArray alloc] init];
        [listenerMap setObject:listeners forKey:[NSNumber numberWithInt:channel]];
        [listeners release];
    }
    [listeners addObject:listener];
    return 0;
}

- (int)removeListener:(NSObject<PdListener> *)listener forChannel:(int)channel {
    NSMutableArray *listeners = [listenerMap objectForKey:[NSNumber numberWithInt:channel]];
    if (listeners) {
        [listeners removeObject:listener];
        if ([listeners count] == 0) {
			NSNumber *c = [channels member:[NSNumber numberWithInt:channel]];
			if (c) {
				[channels removeObject:c];
			}
            [listenerMap removeObjectForKey:[NSNumber numberWithInt:channel]];
        }
    }
    return 0;
}

- (void)removeAllListeners {
	[listenerMap removeAllObjects];
	[channels removeAllObjects];
}

// Override this method in subclasses if you want different printing behavior.
// No need to synchronize here.
- (void)receivePrint:(NSString *)message {
    NSLog(@"Pd: %@\n", message);
}

- (void)receiveNoteOn:(int)pitch withVelocity:(int)velocity forChannel:(int)channel {
	NSArray *listeners = [listenerMap objectForKey:[NSNumber numberWithInt:channel]];
    for (NSObject<PdMidiListener> *listener in listeners) {
        if ([listener respondsToSelector:@selector(receiveNoteOn:pitch:velocity:)]) {
            [listener receiveNoteOn:pitch withVelocity:velocity forChannel:channel];
        } else {
            NSLog(@"Unhandled noteon on channel %d", channel);
        }
    }
}

- (void)receiveControlChange:(int)value forController:(int)controller forChannel:(int)channel {
	NSArray *listeners = [listenerMap objectForKey:[NSNumber numberWithInt:channel]];
    for (NSObject<PdMidiListener> *listener in listeners) {
        if ([listener respondsToSelector:@selector(receiveControlChange:controller:value:)]) {
            [listener receiveControlChange:value forController:controller forChannel:channel];
        } else {
            NSLog(@"Unhandled control change on channel %d", channel);
        }
    }
}

- (void)receiveProgramChange:(int)value forChannel:(int)channel {
	NSArray *listeners = [listenerMap objectForKey:[NSNumber numberWithInt:channel]];
    for (NSObject<PdMidiListener> *listener in listeners) {
        if ([listener respondsToSelector:@selector(receiveProgramChange:value:)]) {
            [listener receiveProgramChange:value forChannel:channel];
        } else {
            NSLog(@"Unhandled program change on channel %d", channel);
        }
    }
}

- (void)receivePitchBend:(int)value forChannel:(int)channel {
	NSArray *listeners = [listenerMap objectForKey:[NSNumber numberWithInt:channel]];
    for (NSObject<PdMidiListener> *listener in listeners) {
        if ([listener respondsToSelector:@selector(receivePitchBend:value:)]) {
            [listener receivePitchBend:value forChannel:channel];
        } else {
            NSLog(@"Unhandled pitch bend on channel %d", channel);
        }
    }
}

- (void)receiveAftertouch:(int)value forChannel:(int)channel {
	NSArray *listeners = [listenerMap objectForKey:[NSNumber numberWithInt:channel]];
    for (NSObject<PdMidiListener> *listener in listeners) {
        if ([listener respondsToSelector:@selector(receiveAftertouch:value:)]) {
            [listener receiveAftertouch:value forChannel:channel];
        } else {
            NSLog(@"Unhandled aftertouch on channel %d", channel);
        }
    }
}

- (void)receivePolyAftertouch:(int)value forPitch:(int)pitch forChannel:(int)channel {
	NSArray *listeners = [listenerMap objectForKey:[NSNumber numberWithInt:channel]];
    for (NSObject<PdMidiListener> *listener in listeners) {
        if ([listener respondsToSelector:@selector(receivePolyAftertouch:pitch:value:)]) {
            [listener receivePolyAftertouch:value forPitch:pitch forChannel:channel];
        } else {
            NSLog(@"Unhandled poly aftertouch on channel %d", channel);
        }
    }
}

// single midi bytes pass through as they aren't filtered by channel
- (void)receiveMidiByte:(int)byte forPort:(int)port {
	for (NSArray *listeners in listenerMap) {
		for (NSObject<PdMidiListener> *listener in listeners) {
			if ([listener respondsToSelector:@selector(receiveMidiByte:byte:)]) {
				[listener receiveMidiByte:byte forPort:port];
			} else {
				NSLog(@"Unhandled midi byte to port %d", port);
			}
		}
	}
}

@end


@implementation LoggingDispatcher

-(void)receiveBangFromSource:(NSString *)source {
    NSLog(@"Received bang from source %@", source);
    [super receiveBangFromSource:source];
}

-(void)receiveFloat:(float)received fromSource:(NSString *)source {
    NSLog(@"Received float %f from source %@", received, source);
    [super receiveFloat:received fromSource:source];
}

-(void)receiveSymbol:(NSString *)symbol fromSource:(NSString *)source {
    NSLog(@"Received symbol %@ from source %@", symbol, source);
    [super receiveSymbol:symbol fromSource:source];
}

-(void)receiveList:(NSArray *)list fromSource:(NSString *)source {
    NSLog(@"Received list %@ from source %@", list, source);
    [super receiveList:list fromSource:source];
}

-(void)receiveMessage:(NSString *)message withArguments:(NSArray *)arguments fromSource:(NSString *)source {
    NSLog(@"Received message %@ with arguments %@ from source %@", message, arguments, source);
    [super receiveMessage:message withArguments:arguments fromSource:source];
}

@end


@implementation LoggingMidiDispatcher

- (void)receiveNoteOn:(int)pitch withVelocity:(int)velocity forChannel:(int)channel {
	NSLog(@"Received noteon %d %d %d", channel, pitch, velocity);
	[super receiveNoteOn:pitch withVelocity:velocity forChannel:channel];
}

- (void)receiveControlChange:(int)value forController:(int)controller forChannel:(int)channel {
	NSLog(@"Received control change %d %d %d", channel, controller, value);
	[super receiveControlChange:value forController:controller forChannel:channel];
}

- (void)receiveProgramChange:(int)value forChannel:(int)channel {
	NSLog(@"Received program change %d %d", channel, value);
	[super receiveProgramChange:value forChannel:channel];
}

- (void)receivePitchBend:(int)value forChannel:(int)channel {
	NSLog(@"Received pitch bend %d %d", channel, value);
	[super receivePitchBend:value forChannel:channel];
}

- (void)receiveAftertouch:(int)value forChannel:(int)channel {
	NSLog(@"Received aftertouch %d %d", channel, value);
	[super receiveAftertouch:value forChannel:channel];
}

- (void)receivePolyAftertouch:(int)value forPitch:(int)pitch forChannel:(int)channel {
	NSLog(@"Received poly aftertouch: %d %d %d", channel, pitch, value);
	[super receivePolyAftertouch:value forPitch:pitch forChannel:channel];
}

- (void)receiveMidiByte:(int)byte forPort:(int)port {
	NSLog(@"Received midi byte: %d 0x%X", port, byte);
	[super receiveMidiByte:byte forPort:port];
}

@end
