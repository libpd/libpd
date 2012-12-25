//
//  AppDelegate.m
//  CocoaPdBasic
//
//  Created by Raccoon on 12/18/12.
//  Copyright (c) 2012 Racoon. All rights reserved.
//

#import "AppDelegate.h"
#import "PdAudioUnit.h"
#import "PdBase.h"
#import "AudioHelpers.h"
#import <Cocoa/Cocoa.h>
#import <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AUGraph.h>

static NSString *const kPatchName = @"tone.pd";

@interface AppDelegate : NSObject <NSApplicationDelegate> {
	NSWindow	*_window;
	PdAudioUnit *_pdAudioUnit;
}

@property (assign) IBOutlet NSWindow *window;
@property (retain) PdAudioUnit *pdAudioUnit;

@end

@implementation AppDelegate

@synthesize window = _window;
@synthesize pdAudioUnit = _pdAudioUnit;

- (void)dealloc {
	self.pdAudioUnit = nil;
    [super dealloc];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	self.pdAudioUnit = [[[PdAudioUnit alloc] init] autorelease];
	[self.pdAudioUnit configureWithSampleRate:44100 numberChannels:2 inputEnabled:NO];
	[self.pdAudioUnit print];

	void *handle = [PdBase openFile:kPatchName path:[[NSBundle mainBundle] resourcePath]];
	if( handle ) {
		AU_LOG(@"patch successfully opened %@.", kPatchName);
	} else {
		AU_LOG(@"error: patch failed to open %@.", kPatchName);
	}

	self.pdAudioUnit.active = YES;
	AU_LOG(@"PdAudioUnit audio active: %@", (self.pdAudioUnit.isActive ? @"YES" : @"NO" ) );
}

@end
