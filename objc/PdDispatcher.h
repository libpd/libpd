//
//  PdDispatcher.h
//  libpd
//
//  Copyright (c) 2011 Peter Brinkmann (peter.brinkmann@gmail.com)
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//

#import <Foundation/Foundation.h>
#import "PdBase.h"


// Listener class for messages from Pd.  The idea is that there will be (at least)
// one listener for each source (i.e., send symbol) in Pd that client code is
// supposed to receive messages from, with the routing of messages being done by an
// instance of PdDispatcher that's handling all messages from Pd.
@protocol PdListener

@optional
- (void)receiveBang;
- (void)receiveFloat:(float)val;
- (void)receiveSymbol:(NSString *)symbol;
- (void)receiveList:(NSArray *)list;
- (void)receiveMessage:(NSString *)message withArguments:(NSArray *)arguments;

@end


// Implementation of the PdReceiverDelegate protocol from PdBase.h.  Client code
// registers one instance of this class with PdBase, and then listeners for individual
// sources will be registered with the dispatcher object.
//
// Printing from Pd is done via NSLog by default; subclass and override the receivePrint
// method if you want different printing behavior.
//
// Note: This class is not meant to be thread-safe.  Listeners should only be added or
// removed when PdAudio is not active.
@interface PdDispatcher : NSObject<PdReceiverDelegate> {
    NSMutableDictionary *listenerMap;
    NSMutableDictionary *subscriptions;
}

// Constructor.
- (id)init;

// Adds a listener for the given source (i.e., send symbol) in Pd.  If this is the first
// listener for this source, a subscription for this symbol will automatically be registered
// with PdBase.
- (int)addListener:(NSObject<PdListener> *)listener forSource:(NSString *)source;

// Removes a listener for a source symbol and unsubscribes from messages to this symbol if
// the listener was the last listener for this symbol.
- (int)removeListener:(NSObject<PdListener> *)listener forSource:(NSString *)source;
@end


// Utility class for executing methods on the main thread.
//
// In order to, say, set the text of an instance of UILabel from the audio thread, say something like
//   [InvokeOnMainThread withTarget:label] setText:@"Hello world!"];
// which is equivalent to
//   [label performSelectorOnMainThread:@selector(setText:) withObject:@"Hello world!" waitUntilDone:NO];
// The former is arguably simpler than the latter, but the main point of this class is (a) to maintain
// the original method signature, including the option of having more than one argument, and (b) to hide
// the waitUntilDone: flag, which should always be NO when invoked from the audio thread.
@interface InvokeOnMainThread : NSObject {
    id target;
}

+ (id)withTarget:(id)target;
@end
