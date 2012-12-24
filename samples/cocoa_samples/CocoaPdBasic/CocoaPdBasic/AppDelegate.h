//
//  AppDelegate.h
//  CocoaPdBasic
//
//  Created by Raccoon on 12/18/12.
//  Copyright (c) 2012 Racoon. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <AudioUnit/AudioUnit.h>

@class PdAudioUnit;


@interface AppDelegate : NSObject <NSApplicationDelegate> {
	NSWindow *_window;
	PdAudioUnit *_pdAudioUnit;
	AudioUnit _outputUnit;
}

@property (assign) IBOutlet NSWindow *window;
@property (retain) PdAudioUnit *pdAudioUnit;

@end
