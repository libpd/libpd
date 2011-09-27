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
@interface PdDispatcher : NSObject<PdReceiverDelegate> {
    NSMutableDictionary *listenerMap;
    NSMutableDictionary *subscriptions;
}

// Adds a listener for the given source (i.e., send symbol) in Pd.  If this is the first
// listener for this source, a subscription for this symbol will automatically be registered
// with PdBase.
- (int)addListener:(NSObject<PdListener> *)listener forSource:(NSString *)source;

// Removes a listener for a source symbol and unsubscribes from messages to this symbol if
// the listener was the last listener for this symbol.
- (int)removeListener:(NSObject<PdListener> *)listener forSource:(NSString *)source;
@end


// Subclass of PdDispatcher that invokes its receive* methods on the main UI thread.
// This class is very experimental and may disappear again without
// warning.  Use at your own risk!
@interface PdUiDispatcher : PdDispatcher {}
@end

