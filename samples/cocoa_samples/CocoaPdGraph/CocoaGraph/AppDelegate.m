//
//  AppDelegate.m
//  CocoaGraph
//
//  Created by Raccoon on 12/25/12.
//  Copyright (c) 2012 Racoon. All rights reserved.
//

#import "PdBase.h"
#import "AudioHelpers.h"

#import <Cocoa/Cocoa.h>
#include <AudioToolbox/AudioToolbox.h>

#define GRAPH_TYPE_SYNTH 1
#define PATCH_NAME @"test.pd"
#define kAudioFileLocation CFSTR ("/Volumes/disk/audio/samples/Seville.wav")

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
		default:
			return [NSString stringWithFormat:@"unknown error code %ld", status];
	}
}

// check if the audio unit had an error, and if so print it and return
#define AU_CHECK(status) do {\
if(status) {\
NSLog(@"*** ERROR *** %s[%d] status code =  %@, number: %ld", __func__, __LINE__, AUStatusCodeAsString(status), status);\
return -1;\
}\
} while (0)

// check if the audio unit graph had an error, and if so print it and return false
#define AUGRAPH_CHECK(status) do {\
if(status) {\
NSLog(@"*** ERROR *** %s[%d] status code =  %@, number: %ld", __func__, __LINE__, AUGraphStatusCodeAsString(status), status);\
return -1;\
}\
} while (0)

@interface AppDelegate : NSObject <NSApplicationDelegate, NSTextFieldDelegate> {
	NSWindow	*_window;
	NSTextField *_textField;

	AUGraph		_graph;
	AudioUnit	_speechUnit;
	AudioUnit	_playerUnit;
	AudioUnit	_outputUnit;
}

@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet NSTextField *textField;
@property (nonatomic) AudioUnit outputUnit;

- (int)startGraph;
- (int)createSynthGraph;
- (int)createFilePlayerGraph;
- (void)saySomething:(NSString *)string;
- (IBAction)speakButtonTapped:(id)sender;

@end

@implementation AppDelegate

@synthesize window = _window;
@synthesize outputUnit = _outputUnit;
@synthesize textField = _textField;

- (void)dealloc
{
    [super dealloc];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	[NSApp setDelegate:self];
	[self.textField setDelegate:self];

#ifdef GRAPH_TYPE_SYNTH
	if( [self createSynthGraph] ) {
		AU_LOG(@"error: failed to initialize synth graph.");
		return;
	}
#else
	if( [self createFilePlayerGraph] ) {
		AU_LOG(@"error: failed to initialize fileplayer graph.");
		return;
	}
	[self playFile];
#endif


	[self printStreamFormats];


	void *handle = [PdBase openFile:PATCH_NAME path:[[NSBundle mainBundle] resourcePath]];
	if( handle ) {
		AU_LOG(@"successfully opened patch %@.", PATCH_NAME);
	} else {
		AU_LOG(@"error: patch failed to open %@.", PATCH_NAME);
		return;
	}

	[PdBase openAudioWithSampleRate:44100 inputChannels:2 outputChannels:2];
	[PdBase computeAudio:YES];



	[self startGraph];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app {
	return YES;
}

- (int)startGraph {
	AUGRAPH_CHECK( AUGraphStart( _graph ) );
	return 0;
}

#pragma mark - Audio Render Callbacks

static OSStatus PdRenderCallback(void *inRefCon,
									AudioUnitRenderActionFlags *ioActionFlags,
									const AudioTimeStamp *inTimeStamp,
									UInt32 inBusNumber,
									UInt32 inNumberFrames,
									AudioBufferList *ioData) {

	AppDelegate *app = (AppDelegate *)inRefCon;
	AudioUnit inputUnit = GRAPH_TYPE_SYNTH ? app->_speechUnit : app->_playerUnit;
	Float32 *auBuffer = (Float32 *)ioData->mBuffers[0].mData;
	OSStatus status = noErr;
	static BOOL sOneTime = NO;

	status = AudioUnitRender(inputUnit, ioActionFlags, inTimeStamp, 0, inNumberFrames, ioData);
	if(status && !sOneTime) {
		sOneTime = YES;
		NSLog(@"*** ERROR *** %s[%d] status code =  %@, number: %ld", __func__, __LINE__, AUStatusCodeAsString(status), status);
	}

	int ticks = inNumberFrames / [PdBase getBlockSize];
	[PdBase processFloatWithInputBuffer:auBuffer outputBuffer:auBuffer ticks:ticks];
	return status;
}

#pragma mark - Speech Synth

- (int)createSynthGraph {
	AUGRAPH_CHECK( NewAUGraph( &_graph ) );

	AUNode speechNode, outputNode;

	// speech synthesizer
	AudioComponentDescription speechCD = {0};
	speechCD.componentType = kAudioUnitType_Generator;
	speechCD.componentSubType = kAudioUnitSubType_SpeechSynthesis;
	speechCD.componentManufacturer = kAudioUnitManufacturer_Apple;

	// output device (speakers)
	AudioComponentDescription outputCD = {0};
	outputCD.componentType = kAudioUnitType_Output;
	outputCD.componentSubType = kAudioUnitSubType_DefaultOutput;
	outputCD.componentManufacturer = kAudioUnitManufacturer_Apple;

	// ???: how to log devices to console
	AudioComponent component = AudioComponentFindNext( NULL, &outputCD );
	if( ! component ) {
		AU_LOG( @"error: can't get kAudioUnitSubType_DefaultOutput." );
		return -1;
	}

	AUGRAPH_CHECK( AUGraphAddNode( _graph, &outputCD, &outputNode ) );
	AUGRAPH_CHECK( AUGraphAddNode( _graph, &speechCD, &speechNode ) );


	AUGRAPH_CHECK( AUGraphOpen( _graph ) );
	AUGRAPH_CHECK( AUGraphNodeInfo( _graph, outputNode, NULL, &_outputUnit ) );
	AUGRAPH_CHECK( AUGraphNodeInfo( _graph, speechNode, NULL, &_speechUnit ) );

	// setup stream formats:
	AudioStreamBasicDescription streamFormat = [self streamFormat];
	AU_CHECK( AudioUnitSetProperty( _speechUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &streamFormat, sizeof(streamFormat) ) );
	AU_CHECK( AudioUnitSetProperty( _outputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &streamFormat, sizeof(streamFormat) ) );

	// setup callback:
	AURenderCallbackStruct callback;
	callback.inputProc = PdRenderCallback;
	callback.inputProcRefCon = self;
	AUGRAPH_CHECK( AUGraphSetNodeInputCallback ( _graph, outputNode, 0, &callback ) );


	// finally init
	AUGRAPH_CHECK( AUGraphInitialize( _graph ) );

	AU_LOG(@"AUGraph successfully initialized" );

	CAShow( _graph );

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
	[self saySomething:[self.textField stringValue]];
}

- (BOOL)control:(NSControl *)control textShouldEndEditing:(NSText *)fieldEditor {
	[self saySomething:[self.textField stringValue]];
	return YES;
}

#pragma mark - File Player

- (int)createFilePlayerGraph {
	AUGRAPH_CHECK( NewAUGraph( &_graph ) );

	AUNode playerNode, outputNode;

	// create audio file player
	AudioComponentDescription playerCD = {0};
	playerCD.componentType = kAudioUnitType_Generator;
	playerCD.componentSubType = kAudioUnitSubType_AudioFilePlayer;
	playerCD.componentManufacturer = kAudioUnitManufacturer_Apple;

	// output device (speakers)
	AudioComponentDescription outputCD = {0};
	outputCD.componentType = kAudioUnitType_Output;
	outputCD.componentSubType = kAudioUnitSubType_DefaultOutput;
	outputCD.componentManufacturer = kAudioUnitManufacturer_Apple;

	AUGRAPH_CHECK( AUGraphAddNode( _graph, &playerCD, &playerNode ) );
	AUGRAPH_CHECK( AUGraphAddNode( _graph, &outputCD, &outputNode ) );

	AUGRAPH_CHECK( AUGraphOpen( _graph ) );
	AUGRAPH_CHECK( AUGraphNodeInfo( _graph, playerNode, NULL, &_playerUnit ) );
	AUGRAPH_CHECK( AUGraphNodeInfo( _graph, outputNode, NULL, &_outputUnit ) );

	// setup stream formats:
	AudioStreamBasicDescription streamFormat = [self streamFormat];
	AU_CHECK( AudioUnitSetProperty( _playerUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &streamFormat, sizeof(streamFormat) ) );
	AU_CHECK( AudioUnitSetProperty( _outputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &streamFormat, sizeof(streamFormat) ) );

	// setup callback
	AURenderCallbackStruct callback;
	callback.inputProc = PdRenderCallback;
	callback.inputProcRefCon = self;
	AUGRAPH_CHECK( AUGraphSetNodeInputCallback ( _graph, outputNode, 0, &callback ) );


	// connect and init
//	AUGRAPH_CHECK( AUGraphConnectNodeInput( _graph, playerNode, 0, outputNode, 0 ) );

	AUGRAPH_CHECK( AUGraphInitialize( _graph ) );

	AU_LOG( @"AUGraph successfully initialized" );
	CAShow( _graph );
	return 0;
}

- (int)playFile {

	// open audio file
	AudioFileID audioFile;
	CFURLRef audioFileURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, kAudioFileLocation, kCFURLPOSIXPathStyle, false);

	AU_CHECK( AudioFileOpenURL( audioFileURL, kAudioFileReadPermission, 0, &audioFile ) );
	CFRelease(audioFileURL);

	// get audio file's format
	AudioStreamBasicDescription fileFormat;
	UInt32 propSize = sizeof(fileFormat);
	AU_CHECK( AudioFileGetProperty( audioFile, kAudioFilePropertyDataFormat, &propSize, &fileFormat ) );


	// prepare unit with the file:

	// tell the file player unit to load the file we want to play
	AU_CHECK( AudioUnitSetProperty( _playerUnit, kAudioUnitProperty_ScheduledFileIDs, kAudioUnitScope_Global, 0, &audioFile, sizeof(audioFile)) );

	UInt64 nPackets;
	propSize = sizeof(nPackets);
	AU_CHECK( AudioFileGetProperty( audioFile, kAudioFilePropertyAudioDataPacketCount, &propSize, &nPackets ) );

	// tell the file player AU to play the entire file
	ScheduledAudioFileRegion region;
	memset(&region.mTimeStamp, 0, sizeof(region.mTimeStamp));
	region.mTimeStamp.mFlags = kAudioTimeStampSampleTimeValid;
	region.mTimeStamp.mSampleTime = 0;
	region.mCompletionProc = NULL;
	region.mCompletionProcUserData = NULL;
	region.mAudioFile = audioFile;
	region.mLoopCount = 1;
	region.mStartFrame = 0;
	region.mFramesToPlay = (UInt32)nPackets * fileFormat.mFramesPerPacket;

	// FIXME: kAudioUnitErr_Uninitialized here? file format?
	AU_CHECK( AudioUnitSetProperty( _playerUnit, kAudioUnitProperty_ScheduledFileRegion, kAudioUnitScope_Global, 0, &region, sizeof(region)) );

	// prime the file player AU with default values
	UInt32 defaultVal = 0;
	AU_CHECK( AudioUnitSetProperty( _playerUnit, kAudioUnitProperty_ScheduledFilePrime, kAudioUnitScope_Global, 0, &defaultVal, sizeof(defaultVal)) );

	// tell the file player AU when to start playing (-1 sample time means next render cycle)
	AudioTimeStamp startTime;
	memset (&startTime, 0, sizeof(startTime));
	startTime.mFlags = kAudioTimeStampSampleTimeValid;
	startTime.mSampleTime = -1;
	AU_CHECK( AudioUnitSetProperty( _playerUnit, kAudioUnitProperty_ScheduleStartTimeStamp, kAudioUnitScope_Global, 0, &startTime, sizeof(startTime)) );

	AU_LOG(@"audio file scheduled, duration: %f", (nPackets * fileFormat.mFramesPerPacket) / fileFormat.mSampleRate);
	return 0;
}

#pragma mark - Utilities

- (AudioStreamBasicDescription)streamFormat {

	AudioStreamBasicDescription stereoStreamFormat;
    // The AudioUnitSampleType data type is the recommended type for sample data in audio
    //    units. This obtains the byte size of the type for use in filling in the ASBD.
    size_t bytesPerSample = sizeof (AudioUnitSampleType);

    // Fill the application audio format struct's fields to define a linear PCM,
    //        stereo, noninterleaved stream at the hardware sample rate.
    stereoStreamFormat.mFormatID          = kAudioFormatLinearPCM;
    stereoStreamFormat.mFormatFlags       = kAudioFormatFlagsAudioUnitCanonical; // TODO: pd expects kAudioFormatFlagsNativeFloatPacked
    stereoStreamFormat.mBytesPerPacket    = bytesPerSample;
    stereoStreamFormat.mFramesPerPacket   = 1;
    stereoStreamFormat.mBytesPerFrame     = bytesPerSample;
    stereoStreamFormat.mChannelsPerFrame  = 2;                    // 2 indicates stereo
    stereoStreamFormat.mBitsPerChannel    = 8 * bytesPerSample;
    stereoStreamFormat.mSampleRate        = 44100;

	return stereoStreamFormat;
}

static void printASBD( AudioStreamBasicDescription asbd ) {
	char formatIDString[5];
    UInt32 formatID = CFSwapInt32HostToBig (asbd.mFormatID);
    bcopy (&formatID, formatIDString, 4);
    formatIDString[4] = '\0';

    NSLog (@"  Sample Rate:         %10.0f",  asbd.mSampleRate);
    NSLog (@"  Format ID:           %10s",    formatIDString);
    NSLog (@"  Format Flags:        %10X",    (unsigned int)asbd.mFormatFlags);
    NSLog (@"  Bytes per Packet:    %10d",    (unsigned int)asbd.mBytesPerPacket);
    NSLog (@"  Frames per Packet:   %10d",    (unsigned int)asbd.mFramesPerPacket);
    NSLog (@"  Bytes per Frame:     %10d",    (unsigned int)asbd.mBytesPerFrame);
    NSLog (@"  Channels per Frame:  %10d",    (unsigned int)asbd.mChannelsPerFrame);
    NSLog (@"  Bits per Channel:    %10d",    (unsigned int)asbd.mBitsPerChannel);
}

-
(void)printStreamFormats {

	UInt32 sizeASBD = sizeof(AudioStreamBasicDescription);
	AudioUnit inputUnit = GRAPH_TYPE_SYNTH ? _speechUnit : _playerUnit;

	// input:
	AudioStreamBasicDescription intputStreamDescription;
	memset (&intputStreamDescription, 0, sizeof(intputStreamDescription));
	AU_RETURN_IF_ERROR(AudioUnitGetProperty(inputUnit,
											kAudioUnitProperty_StreamFormat,
											kAudioUnitScope_Output,
											0,
											&intputStreamDescription,
											&sizeASBD));
	AU_LOG(@"input ASBD:");
	printASBD( intputStreamDescription );

	// output:
	AudioStreamBasicDescription outputStreamDescription;
	memset(&outputStreamDescription, 0, sizeASBD);
	AU_RETURN_IF_ERROR(AudioUnitGetProperty(_outputUnit,
                                            kAudioUnitProperty_StreamFormat,
                                            kAudioUnitScope_Input,
                                            0,
                                            &outputStreamDescription,
                                            &sizeASBD));
	AU_LOG(@"output ASBD:");
	printASBD( outputStreamDescription );
}

@end
