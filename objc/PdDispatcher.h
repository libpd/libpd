//
//  PdDispatcher.h
//  libpd
//
//  Copyright (c) 2011 Peter Brinkmann (peter.brinkmann@gmail.com)
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//
//  Updated 2013, 2018, 2020 Dan Wilcox <danomatika@gmail.com>
//

#import <Foundation/Foundation.h>
#import "PdBase.h"

/// PdReceiverDelegate protocol default implementation
///
/// client code registers one instance of this class with PdBase, then listeners
/// for individual sources will be registered with the dispatcher object
///
/// printing from pd is done via NSLog by default, subclass and override the
/// receivePrint method if you want different printing behavior
@interface PdDispatcher : NSObject<PdReceiverDelegate> {
	NSMutableDictionary *listenerMap;
	NSMutableDictionary *subscriptions;
}

/// add a listener for the given source (i.e. send symbol) in pd
/// if this is the first listener for this source, a subscription for this symbol
/// will be automatically registered with PdBase
- (int)addListener:(NSObject<PdListener> *)listener
         forSource:(NSString *)source;

/// remove a listener for a source symbol
/// if this is the last listener for this source, the symbol will be
/// automatically unsubscribed from PdBase
- (int)removeListener:(NSObject<PdListener> *)listener
            forSource:(NSString *)source;

/// remove all listeners
- (void)removeAllListeners;

@end

/// subclass of PdDispatcher that logs all callbacks,
/// mostly for development and debugging
@interface LoggingDispatcher : PdDispatcher {}
@end
