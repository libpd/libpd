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

@protocol PdListener

@optional
- (void)receiveBang;
- (void)receiveFloat:(float)val ;
- (void)receiveSymbol:(NSString *)symbol;
- (void)receiveList:(NSArray *)list;
- (void)receiveMessage:(NSString *)message withArguments:(NSArray *)arguments;

@end


@interface PdDispatcher : NSObject<PdReceiverDelegate> {
    NSMutableDictionary *listenerMap;
}

- (id)init;
- (int)addListener:(NSObject<PdListener> *)listener forSource:(NSString *)symbol;
- (int)removeListener:(NSObject<PdListener> *)listener forSource:(NSString *)symbol;
@end
