//
//  PdMidiDispatcher.h
//  libpd
//
//  Copyright (c) 2013, 2020 Dan Wilcox (danomatika@gmail.com)
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//

#import <Foundation/Foundation.h>
#import "PdBase.h"

/// PdMidiReceiverDelegate protocol default implementation
///
/// client code registers one instance of this class with PdBase, then listeners
/// for individual channels will be registered with the dispatcher object
///
/// raw MIDI bytes are only printed by default, subclass and override the
/// receiveMidiByte method if you want to handle raw MIDI aka data sent to
/// [midiout]
@interface PdMidiDispatcher : NSObject<PdMidiReceiverDelegate> {
	NSMutableDictionary *listenerMap;
}

/// add a listener for the given MIDI channel in pd
- (int)addListener:(NSObject<PdMidiListener> *)listener
        forChannel:(int)channel;

/// remove a listener for a channel
- (int)removeListener:(NSObject<PdMidiListener> *)listener
           forChannel:(int)channel;

/// removes all listeners
- (void)removeAllListeners;

@end

/// subclass of PdMidiDisptcher that logs all callbacks,
/// mostly for development and debugging
@interface LoggingMidiDispatcher : PdMidiDispatcher {}
@end
