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

// Removes all listeners.
- (void)removeAllListeners;
@end

// Subclass of PdDispatcher that logs all callbacks, mostly for development and debugging.
@interface LoggingDispatcher : PdDispatcher {}
@end
