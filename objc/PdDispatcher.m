//
//  PdDispatcher.m
//  libpd
//
//  Copyright (c) 2011 Peter Brinkmann (peter.brinkmann@gmail.com)
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//
//  Updated 2013, 2018 Dan Wilcox <danomatika@gmail.com>
//

#import "PdDispatcher.h"

@implementation PdDispatcher

- (instancetype)init {
	self = [super init];
	if (self) {
		listenerMap = [[NSMutableDictionary alloc] init];
		subscriptions = [[NSMutableDictionary alloc] init];
	}
	return self;
}

- (void)dealloc {
	for (NSValue *handle in subscriptions.allValues) {
		void *ptr = handle.pointerValue;
		[PdBase unsubscribe:ptr];
	}
}

- (int)addListener:(NSObject<PdListener> *)listener forSource:(NSString *)symbol {
	NSMutableArray *listeners = listenerMap[symbol];
	if (!listeners) {
		void *ptr = [PdBase subscribe:symbol];
		if (!ptr) {
			return -1;
		}
		NSValue *handle = [NSValue valueWithPointer:ptr];
		subscriptions[symbol] = handle;
		listeners = [[NSMutableArray alloc] init];
		listenerMap[symbol] = listeners;
	}
	[listeners addObject:listener];
	return 0;
}

- (int)removeListener:(NSObject<PdListener> *)listener forSource:(NSString *)symbol {
	NSMutableArray *listeners = listenerMap[symbol];
	if (listeners) {
		[listeners removeObject:listener];
		if (listeners.count == 0) {
			NSValue *handle = subscriptions[symbol];
			void *ptr = handle.pointerValue;
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
		void *ptr = handle.pointerValue;
		[PdBase unsubscribe:ptr];
	}
	[subscriptions removeAllObjects];
}

// Override this method in subclasses if you want different printing behavior.
- (void)receivePrint:(NSString *)message {
	NSLog(@"Pd: %@\n", message);
}

- (void)receiveBangFromSource:(NSString *)source {
	NSArray *listeners = listenerMap[source];
	for (NSObject<PdListener> *listener in listeners) {
		if ([listener respondsToSelector:@selector(receiveBangFromSource:)]) {
		[listener receiveBangFromSource:source];
		} else {
		NSLog(@"Unhandled bang from %@", source);
		}
	}
}

- (void)receiveFloat:(float)received fromSource:(NSString *)source {
	NSArray *listeners = listenerMap[source];
	for (NSObject<PdListener> *listener in listeners) {
		if ([listener respondsToSelector:@selector(receiveFloat:fromSource:)]) {
			[listener receiveFloat:received fromSource:source];
		} else {
			NSLog(@"Unhandled float from %@", source);
		}
	}
}

- (void)receiveSymbol:(NSString *)symbol fromSource:(NSString *)source {
	NSArray *listeners = listenerMap[source];
	for (NSObject<PdListener> *listener in listeners) {
		if ([listener respondsToSelector:@selector(receiveSymbol:fromSource:)]) {
			[listener receiveSymbol:symbol fromSource:source];
		} else {
			NSLog(@"Unhandled symbol from %@", source);
		}
	}
}

- (void)receiveList:(NSArray *)list fromSource:(NSString *)source {
	NSArray *listeners = listenerMap[source];
	for (NSObject<PdListener> *listener in listeners) {
		if ([listener respondsToSelector:@selector(receiveList:fromSource:)]) {
			[listener receiveList:list fromSource:source];
		} else {
			NSLog(@"Unhandled list from %@", source);
		}
	}
}

- (void) receiveMessage:(NSString *)message withArguments:(NSArray *)arguments fromSource:(NSString *)source {
	NSArray *listeners = listenerMap[source];
	for (NSObject<PdListener> *listener in listeners) {
		if ([listener respondsToSelector:@selector(receiveMessage:withArguments:fromSource:)]) {
			[listener receiveMessage:message withArguments:arguments fromSource:source];
		} else {
			NSLog(@"Unhandled typed message from %@", source);
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
