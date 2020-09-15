//
//  PdMidiDispatcher.m
//  libpd
//
//  Copyright (c) 2013 Dan Wilcox (danomatika@gmail.com)
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//

#import "PdMidiDispatcher.h"

@implementation PdMidiDispatcher

- (instancetype)init {
	self = [super init];
	if (self) {
		listenerMap = [[NSMutableDictionary alloc] init];
	}
	return self;
}

- (void)dealloc {
	[listenerMap removeAllObjects];
}

- (int)addListener:(NSObject<PdListener> *)listener forChannel: (int)channel {
	NSMutableArray *listeners = listenerMap[@(channel)];
	if (!listeners) {
		listeners = [[NSMutableArray alloc] init];
		listenerMap[@(channel)] = listeners;
	}
	[listeners addObject:listener];
	return 0;
}

- (int)removeListener:(NSObject<PdListener> *)listener forChannel:(int)channel {
	NSMutableArray *listeners = listenerMap[@(channel)];
	if (listeners) {
		[listeners removeObject:listener];
		if (listeners.count == 0) {
			[listenerMap removeObjectForKey:@(channel)];
		}
	}
	return 0;
}

- (void)removeAllListeners {
	[listenerMap removeAllObjects];
}

- (void)receiveNoteOn:(int)pitch withVelocity:(int)velocity forChannel:(int)channel {
	NSArray *listeners = listenerMap[@(channel)];
	for (NSObject<PdMidiListener> *listener in listeners) {
		if ([listener respondsToSelector:@selector(receiveNoteOn:withVelocity:forChannel:)]) {
			[listener receiveNoteOn:pitch withVelocity:velocity forChannel:channel];
		} else {
			NSLog(@"Unhandled noteon on channel %d", channel);
		}
	}
}

- (void)receiveControlChange:(int)value forController:(int)controller forChannel:(int)channel {
	NSArray *listeners = listenerMap[@(channel)];
	for (NSObject<PdMidiListener> *listener in listeners) {
		if ([listener respondsToSelector:@selector(receiveControlChange:forController:forChannel:)]) {
			[listener receiveControlChange:value forController:controller forChannel:channel];
		} else {
			NSLog(@"Unhandled control change on channel %d", channel);
		}
	}
}

- (void)receiveProgramChange:(int)value forChannel:(int)channel {
	NSArray *listeners = listenerMap[@(channel)];
	for (NSObject<PdMidiListener> *listener in listeners) {
		if ([listener respondsToSelector:@selector(receiveProgramChange:forChannel:)]) {
			[listener receiveProgramChange:value forChannel:channel];
		} else {
			NSLog(@"Unhandled program change on channel %d", channel);
		}
	}
}

- (void)receivePitchBend:(int)value forChannel:(int)channel {
	NSArray *listeners = listenerMap[@(channel)];
	for (NSObject<PdMidiListener> *listener in listeners) {
		if ([listener respondsToSelector:@selector(receivePitchBend:forChannel:)]) {
			[listener receivePitchBend:value forChannel:channel];
		} else {
			NSLog(@"Unhandled pitch bend on channel %d", channel);
		}
	}
}

- (void)receiveAftertouch:(int)value forChannel:(int)channel {
	NSArray *listeners = listenerMap[@(channel)];
	for (NSObject<PdMidiListener> *listener in listeners) {
		if ([listener respondsToSelector:@selector(receiveAftertouch:forChannel:)]) {
			[listener receiveAftertouch:value forChannel:channel];
		} else {
			NSLog(@"Unhandled aftertouch on channel %d", channel);
		}
	}
}

- (void)receivePolyAftertouch:(int)value forPitch:(int)pitch forChannel:(int)channel {
	NSArray *listeners = listenerMap[@(channel)];
	for (NSObject<PdMidiListener> *listener in listeners) {
		if ([listener respondsToSelector:@selector(receivePolyAftertouch:forPitch:forChannel:)]) {
			[listener receivePolyAftertouch:value forPitch:pitch forChannel:channel];
		} else {
			NSLog(@"Unhandled poly aftertouch on channel %d", channel);
		}
	}
}

// Override this method in subclasses if you want different printing behavior.
- (void)receiveMidiByte:(int)byte forPort: (int)port {
	NSLog(@"Received midi byte: %d 0x%X", port, byte);
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

- (void)receivePolyAftertouch: (int)value forPitch:(int)pitch forChannel:(int)channel {
	NSLog(@"Received poly aftertouch: %d %d %d", channel, pitch, value);
	[super receivePolyAftertouch:value forPitch:pitch forChannel:channel];
}

- (void)receiveMidiByte: (int)byte forPort: (int)port {
	NSLog(@"Received midi byte: %d 0x%X", port, byte);
	[super receiveMidiByte:byte forPort:port];
}

@end
