//
//  PdAudioController.m
//  libpd
//
//  Created on 11/10/11.
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//

#import "PdAudioController.h"
#import "PdAudioUnit.h"
#import "AudioHelpers.h"
#import "PdBase.h"
#import <AudioToolbox/AudioToolbox.h>

@interface PdAudioController ()

@property (nonatomic, retain) PdAudioUnit *audioUnit;	// out private PdAudioUnit
- (int)audioSessionTicksPerBuffer;						// calculating ticks per buffer from the audio sessions buffer size (which is provided in seconds)
- (PdAudioStatus)updateSampleRate:(int)sampleRate;		// updates the sample rate while verifying it is in sync with the audio session and PdAudioUnit
- (PdAudioStatus)setCategory:(NSString *)category;
- (PdAudioStatus)configureAudioUnitWithSampleRate:(int)sampleRate numberInputChannels:(int)numInputs numberOutputChannels:(int)numOutputs;

@end

@implementation PdAudioController

@synthesize sampleRate = sampleRate_;
@synthesize numberInputChannels = numberInputChannels_;
@synthesize numberOutputChannels = numberOutputChannels_;
@synthesize ticksPerBuffer = ticksPerBuffer_;
@synthesize active = active_;
@synthesize audioUnit = audioUnit_;

- (id)init {
	self = [super init];
    if (self) {
        AVAudioSession *globalSession = [AVAudioSession sharedInstance];
        globalSession.delegate = self;
        NSError *error = nil;
        [globalSession setActive:YES error:&error];
        AV_CHECK_ERROR(error);
        AU_LOGV(@"Audio Session initialized");
        self.audioUnit = [[[PdAudioUnit alloc] init] autorelease];
        ticksPerBuffer_ = [self audioSessionTicksPerBuffer];
	}
	return self;
}

// using Audio Session C API because the AVAudioSession only provides the
// 'preferred' buffer duration, not what is actually set
- (int)audioSessionTicksPerBuffer {
	Float32 asBufferDuration = 0;
	UInt32 size = sizeof(asBufferDuration);
	
	OSStatus status = AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareIOBufferDuration, &size, &asBufferDuration);
	AU_CHECK(!status, @"error getting audio session buffer duration (status = %ld)", status);
	AU_LOGV(@"kAudioSessionProperty_CurrentHardwareIOBufferDuration: %f seconds", asBufferDuration);
    
	return round((asBufferDuration * self.sampleRate) /  (NSTimeInterval)[PdBase getBlockSize]);
}

- (void)dealloc {
	self.audioUnit = nil;
	[super dealloc];
}

- (PdAudioStatus)configureWithSampleRate:(int)sampleRate numberInputChannels:(int)numInputs numberOutputChannels:(int)numOutputs {
	PdAudioStatus status = [self updateSampleRate:sampleRate];
	status |= numInputs ? [self setCategory:AVAudioSessionCategoryPlayAndRecord] : [self setCategory:AVAudioSessionCategoryPlayback];
	status |= [self configureAudioUnitWithSampleRate:sampleRate numberInputChannels:numInputs numberOutputChannels:numOutputs];
	AU_LOGV(@"configuration finished. status: %d", status);
	return status;
}

- (PdAudioStatus)configureForBackgroundAudioWithSampleRate:(int)sampleRate numberOutputChannels:(int)numOutputs mixingEnabled:(BOOL)mixingEnabled {
	PdAudioStatus status = [self updateSampleRate:sampleRate];
	status |= mixingEnabled ? [self setCategory:AVAudioSessionCategoryAmbient] : [self setCategory:AVAudioSessionCategorySoloAmbient]; 
	status |= [self configureAudioUnitWithSampleRate:sampleRate numberInputChannels:0 numberOutputChannels:numOutputs];
	AU_LOGV(@"configuration finished. status: %d", status);
	return status;
}

- (PdAudioStatus)configureAudioUnitWithSampleRate:(int)sampleRate numberInputChannels:(int)numInputs numberOutputChannels:(int)numOutputs {
    PdAudioStatus status = [self.audioUnit configureWithSampleRate:sampleRate
                                               numberInputChannels:numInputs
                                              numberOutputChannels:numOutputs];
    sampleRate_ = self.audioUnit.sampleRate;
    numberInputChannels_ = self.audioUnit.numberInputChannels;
    numberOutputChannels_ = self.audioUnit.numberOutputChannels;
    return status;
}

- (PdAudioStatus)updateSampleRate:(int)sampleRate {
	AVAudioSession *globalSession = [AVAudioSession sharedInstance];
	NSError *error = nil;
	[globalSession setPreferredHardwareSampleRate:sampleRate error:&error];
	AV_CHECK_ERROR(error);
	double currentHardwareSampleRate = globalSession.currentHardwareSampleRate;
	AU_LOGV(@"currentHardwareSampleRate: %.0f", currentHardwareSampleRate);
    sampleRate_ = currentHardwareSampleRate;
	if (!floatsAreEqual(sampleRate, currentHardwareSampleRate)) {
		AU_LOG(@"*** WARNING *** could not update samplerate, resetting to match audio session (%.0fHz)", currentHardwareSampleRate);
		return PdAudioPropertyChanged;
	}
	return PdAudioOK;
}

- (PdAudioStatus)setCategory:(NSString *)category {
    NSError *error = nil;
    [[AVAudioSession sharedInstance] setCategory:category error:&error];
    return error ? PdAudioError : PdAudioOK;
}

/* note about the magic 0.5 added to numberFrames:
 * apple is doing some horrible rounding of the bufferDuration into
 * what tries to give a power of two frames to the audio unit, which
 * is inconsistent accross different devices.  As they are currently
 * truncating, we add in this value to make sure the resulting ticks
 * value is not halved.
 */
- (PdAudioStatus)configureTicksPerBuffer:(int)ticksPerBuffer {
    int numberFrames = [PdBase getBlockSize] * ticksPerBuffer;
    NSTimeInterval bufferDuration = (Float32) (numberFrames + 0.5) / self.sampleRate;
    
    NSError *error = nil;
    AVAudioSession *globalSession = [AVAudioSession sharedInstance];
    [globalSession setPreferredIOBufferDuration:bufferDuration error:&error];
    AV_CHECK_ERROR(error);
    
    AU_LOGV(@"numberFrames: %d, specified bufferDuration: %f", numberFrames, bufferDuration);
    AU_LOGV(@"preferredIOBufferDuration: %f", globalSession.preferredIOBufferDuration);
    
    ticksPerBuffer_ = [self audioSessionTicksPerBuffer];
    if (ticksPerBuffer_ != ticksPerBuffer) {
        AU_LOG(@"*** WARNING *** could not set IO buffer duration to match %d ticks, got %d ticks instead", ticksPerBuffer, ticksPerBuffer_);
        return PdAudioPropertyChanged;
    }
    return PdAudioOK;
}

- (void)setActive:(BOOL)active {
	if (active_ != active) {
		if (active) {
			[self.audioUnit start];
		} else {
			[self.audioUnit stop];
		}
		active_ = [self.audioUnit isRunning];
	}
}

- (void)beginInterruption {
	if (self.active) {
		[self.audioUnit stop];
	}
	AU_LOGV(@"interrupted");
}

- (void)endInterruptionWithFlags:(NSUInteger)flags {
	if (flags == AVAudioSessionInterruptionFlags_ShouldResume) {
		if (self.active) {
			[self.audioUnit start];
		}
		AU_LOGV(@"ended interruption");
	} else {
		AU_LOGV(@"still interrupted");
	}
}

- (void)inputIsAvailableChanged:(BOOL)isInputAvailable {
	AU_LOGV(@"inputIsAvailableChanged: %@", (isInputAvailable ? @"YES" : @"NO"));
}

- (void)print {
	AVAudioSession *globalSession = [AVAudioSession sharedInstance];
	AU_LOG(@"----- AVAudioSession properties -----");
	AU_LOG(@"category: %@", globalSession.category);
	AU_LOG(@"currentHardwareSampleRate: %.0f", globalSession.currentHardwareSampleRate);
	AU_LOG(@"preferredHardwareSampleRate: %.0f", globalSession.preferredHardwareSampleRate);
	AU_LOG(@"preferredIOBufferDuration: %f", globalSession.preferredIOBufferDuration);
	
	AU_LOG(@"inputIsAvailable: %@", (globalSession.inputIsAvailable ? @"YES" : @"NO"));
	AU_LOG(@"currentHardwareInputNumberOfChannels: %d", globalSession.currentHardwareInputNumberOfChannels);
	AU_LOG(@"currentHardwareOutputNumberOfChannels: %d", globalSession.currentHardwareOutputNumberOfChannels);
    
	[self.audioUnit print];
}

@end
