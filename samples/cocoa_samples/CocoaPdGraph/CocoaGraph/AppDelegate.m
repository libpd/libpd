//
//  AppDelegate.m
//  CocoaGraph
//
//  Created by Raccoon on 12/25/12.
//  Copyright (c) 2012 Racoon. All rights reserved.
//

//#import "AppDelegate.h"
#import "PdBase.h"
#import "AudioHelpers.h"

static const NSString *PATCH_NAME = @"test.pd";

#import <Cocoa/Cocoa.h>

#include <AudioToolbox/AudioToolbox.h>

static NSString *AUGraphStatusCodeAsString(OSStatus status) {
	switch (status) {
		case kAUGraphErr_NodeNotFound:
			return @"kAUGraphErr_NodeNotFound";
		case kAUGraphErr_InvalidConnection:
			return @"kAUGraphErr_InvalidConnection";
		case kAUGraphErr_OutputNodeErr:
			return @"kAUGraphErr_OutputNodeErr";
		case kAUGraphErr_CannotDoInCurrentContext:
			return @"kAUGraphErr_CannotDoInCurrentContext";
		case kAUGraphErr_InvalidAudioUnit:
			return @"kAUGraphErr_InvalidAudioUnit";
			//#endif
		default:
			return [NSString stringWithFormat:@"unknown error code %ld", status];
	}
}

// check if the audio unit had an error, and if so print it and return false
#define AUGRAPH_CHECK(status) do {\
if(status) {\
NSLog(@"*** ERROR *** %s[%d] status code =  %@", __func__, __LINE__, AUGraphStatusCodeAsString(status));\
return -1;\
}\
} while (0)


@interface AppDelegate : NSObject <NSApplicationDelegate> {
	NSWindow	*_window;
	AudioUnit	_outputUnit;
	AudioUnit	_speechUnit;
	AUGraph		_graph;
}

@property (assign) IBOutlet NSWindow *window;
@property (nonatomic) AudioUnit outputUnit;

- (int)createGraph;
- (int)startGraph;
- (void)saySomething:(NSString *)string;
- (IBAction)speakButtonTapped:(id)sender;

@end

@implementation AppDelegate

@synthesize window = _window;
@synthesize outputUnit = _outputUnit;

- (void)dealloc
{
    [super dealloc];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	if( [self createGraph] ) {
		AU_LOG(@"error: failed to initialize graph.");
		return;
	}
	[self saySomething:@"Hey there handsome dude."];
	[self startGraph];
	
//	void *handle = [PdBase openFile:PATCH_NAME path:[[NSBundle mainBundle] resourcePath]];
//	if( handle ) {
//		AU_LOG(@"successfully opened patch %@.", PATCH_NAME);
//	} else {
//		AU_LOG(@"error: patch failed to open %@.", PATCH_NAME);
//	}

	// TODO: need to init PdBase same as PdAudioUnit
//	[PdBase computeAudio:YES];
}

- (int)createGraph {
	AUGRAPH_CHECK( NewAUGraph( &_graph ) );

	AudioComponentDescription outputCD, speechCD;
	AUNode outputNode, speechNode;

	// output device (speakers)
	memset(&outputCD, 0, sizeof(outputCD));
	outputCD.componentType = kAudioUnitType_Output;
	outputCD.componentSubType = kAudioUnitSubType_DefaultOutput;
	outputCD.componentManufacturer = kAudioUnitManufacturer_Apple;

	// speech synthesizer
	memset(&speechCD, 0, sizeof(speechCD));
	speechCD.componentType = kAudioUnitType_Generator;
	speechCD.componentSubType = kAudioUnitSubType_SpeechSynthesis;
	speechCD.componentManufacturer = kAudioUnitManufacturer_Apple;

	AudioComponent comp = AudioComponentFindNext( NULL, &outputCD );
	if( ! comp ) {
		AU_LOG( @"error: can't get kAudioUnitSubType_DefaultOutput." );
		return -1;
	}

	AUGRAPH_CHECK( AUGraphAddNode( _graph, &outputCD, &outputNode ) );
	AUGRAPH_CHECK( AUGraphAddNode( _graph, &speechCD, &speechNode ) );


	AUGRAPH_CHECK( AUGraphOpen( _graph ) );
	AUGRAPH_CHECK( AUGraphNodeInfo( _graph, speechNode, NULL, &_speechUnit ) );

	// connect the output source of the speech synthesis AU to the input source of the output node
	AUGRAPH_CHECK( AUGraphConnectNodeInput( _graph, speechNode, 0, outputNode, 0 ) );

	AUGRAPH_CHECK( AUGraphInitialize( _graph ) );
	AU_LOG(@"AUGraph successfully initialized" );

	CAShow( _graph );

	return 0;
}

- (int)startGraph {
	AUGRAPH_CHECK( AUGraphStart( _graph ) );
	return 0;
}


- (void)saySomething:(NSString *)str {
	SpeechChannel chan;

	UInt32 propsize = sizeof(SpeechChannel);
	AU_RETURN_IF_ERROR( AudioUnitGetProperty( _speechUnit, kAudioUnitProperty_SpeechChannel, kAudioUnitScope_Global, 0, &chan, &propsize) );

//	AU_LOG(@"speech chan: %ld", chan->data[0] );
	SpeakCFString( chan, (CFStringRef)str, NULL );
}

- (IBAction)speakButtonTapped:(id)sender {
	AU_LOG(@"trying to speak..");
	[self saySomething:@"Hey there ugly dude."];
}

@end
