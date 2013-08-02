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
#import <AVFoundation/AVFoundation.h>

@interface PdAudioController ()

@property (nonatomic, retain) PdAudioUnit *audioUnit;	// out private PdAudioUnit
- (PdAudioStatus)updateSampleRate:(int)sampleRate;		// updates the sample rate while verifying it is in sync with the audio session and PdAudioUnit
- (PdAudioStatus)selectCategoryWithInputs:(BOOL)hasInputs isAmbient:(BOOL)isAmbient allowsMixing:(BOOL)allowsMixing;  // Not all inputs make sense, but that's okay in the private interface.
- (PdAudioStatus)configureAudioUnitWithNumberChannels:(int)numChannels inputEnabled:(BOOL)inputEnabled;

#if __IPHONE_OS_VERSION_MIN_ALLOWED >= 60000
// AVAudioSessionDelegate is deprecated starting in iOS 6, so we declare it's methods here
- (void)beginInterruption;
- (void)endInterruptionWithFlags:(NSUInteger)flags;
- (void)interruptionOcurred:(NSNotification*)notification; // handles AVAudioSessionInterruptionNotification
#endif

@end

@implementation PdAudioController

@synthesize sampleRate = sampleRate_;
@synthesize numberChannels = numberChannels_;
@synthesize inputEnabled = inputEnabled_;
@synthesize mixingEnabled = mixingEnabled_;
@synthesize ticksPerBuffer = ticksPerBuffer_;
@synthesize active = active_;
@synthesize audioUnit = audioUnit_;

- (id)init {
	self = [super init];
    if (self) {
        AVAudioSession *globalSession = [AVAudioSession sharedInstance];
		
		[AVAudioSession sharedInstance];
	#if __IPHONE_OS_VERSION_MIN_REQUIRED >= 60000
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(interruptionOcurred:) name:AVAudioSessionInterruptionNotification object:nil];
    #else
		// AVAudioSessionDelegate is deprecated starting in iOS 6
		globalSession.delegate = self;
	#endif
        NSError *error = nil;
        [globalSession setActive:YES error:&error];
        AU_LOG_IF_ERROR(error, @"Audio Session activation failed");
        AU_LOGV(@"Audio Session initialized");
        self.audioUnit = [[[PdAudioUnit alloc] init] autorelease];
	}
	return self;
}

// using Audio Session C API because the AVAudioSession only provides the
// 'preferred' buffer duration, not what is actually set
- (int)ticksPerBuffer {
	Float32 asBufferDuration = 0;
	UInt32 size = sizeof(asBufferDuration);
	
	OSStatus status = AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareIOBufferDuration, &size, &asBufferDuration);
	AU_LOG_IF_ERROR(status, @"error getting audio session buffer duration (status = %ld)", status);
	AU_LOGV(@"kAudioSessionProperty_CurrentHardwareIOBufferDuration: %f seconds", asBufferDuration);
    
	ticksPerBuffer_ = round((asBufferDuration * self.sampleRate) /  (NSTimeInterval)[PdBase getBlockSize]);
    return ticksPerBuffer_;
}

- (void)dealloc {
	self.audioUnit = nil;
	[super dealloc];
}

- (PdAudioStatus)configurePlaybackWithSampleRate:(int)sampleRate
                                  numberChannels:(int)numChannels
                                    inputEnabled:(BOOL)inputEnabled
                                   mixingEnabled:(BOOL)mixingEnabled {
	PdAudioStatus status = PdAudioOK;
    if (inputEnabled && ![[AVAudioSession sharedInstance] inputIsAvailable]) {
        inputEnabled = NO;
        status |= PdAudioPropertyChanged;
    }
    status |= [self updateSampleRate:sampleRate];
    if (status == PdAudioError) {
        return PdAudioError;
    }
    status |= [self selectCategoryWithInputs:inputEnabled isAmbient:NO allowsMixing:mixingEnabled];
    if (status == PdAudioError) {
        return PdAudioError;
    }
    status |= [self configureAudioUnitWithNumberChannels:numChannels inputEnabled:inputEnabled];
	AU_LOGV(@"configuration finished. status: %d", status);
	return status;
}

- (PdAudioStatus)configureAmbientWithSampleRate:(int)sampleRate
                                 numberChannels:(int)numChannels
                                  mixingEnabled:(BOOL)mixingEnabled {
	PdAudioStatus status = [self updateSampleRate:sampleRate];
    if (status == PdAudioError) {
        return PdAudioError;
    }
    status |= [self selectCategoryWithInputs:NO isAmbient:YES allowsMixing:mixingEnabled];
    if (status == PdAudioError) {
        return PdAudioError;
    }
    status |= [self configureAudioUnitWithNumberChannels:numChannels inputEnabled:NO];
	AU_LOGV(@"configuration finished. status: %d", status);
	return status;
}

- (PdAudioStatus)configureAudioUnitWithNumberChannels:(int)numChannels inputEnabled:(BOOL)inputEnabled {
    inputEnabled_ = inputEnabled;
    numberChannels_ = numChannels;
    return [self.audioUnit configureWithSampleRate:self.sampleRate numberChannels:numChannels inputEnabled:inputEnabled] ? PdAudioError : PdAudioOK;
}

- (PdAudioStatus)updateSampleRate:(int)sampleRate {
	AVAudioSession *globalSession = [AVAudioSession sharedInstance];
	NSError *error = nil;
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= 60000
	[globalSession setPreferredSampleRate:sampleRate error:&error];
#else
	[globalSession setPreferredHardwareSampleRate:sampleRate error:&error];
#endif
    if (error) {
        return PdAudioError;
    }
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= 60000
	double currentHardwareSampleRate = globalSession.sampleRate;
#else
	double currentHardwareSampleRate = globalSession.currentHardwareSampleRate;
#endif
	AU_LOGV(@"currentHardwareSampleRate: %.0f", currentHardwareSampleRate);
    sampleRate_ = currentHardwareSampleRate;
	if (!floatsAreEqual(sampleRate, currentHardwareSampleRate)) {
		AU_LOG(@"*** WARNING *** could not update samplerate, resetting to match audio session (%.0fHz)", currentHardwareSampleRate);
		return PdAudioPropertyChanged;
	}
	return PdAudioOK;
}

- (PdAudioStatus)selectCategoryWithInputs:(BOOL)hasInputs isAmbient:(BOOL)isAmbient allowsMixing:(BOOL)allowsMixing {
    NSString *category;
	OSStatus status;
    if (hasInputs && isAmbient) {
        AU_LOG(@"impossible session config; this should never happen");
        return PdAudioError;
    } else if (!isAmbient) {
        category = hasInputs ? AVAudioSessionCategoryPlayAndRecord : AVAudioSessionCategoryPlayback;
    } else {
        category = allowsMixing ? AVAudioSessionCategoryAmbient : AVAudioSessionCategorySoloAmbient;
    }
    NSError *error = nil;
    [[AVAudioSession sharedInstance] setCategory:category error:&error];
    if (error) {
        AU_LOG(@"failed to set session category, error %@", error);
        return PdAudioError;
    }
	if ([category isEqualToString:AVAudioSessionCategoryPlayAndRecord]) {
		UInt32 defaultToSpeaker = 1;
		status = AudioSessionSetProperty(kAudioSessionProperty_OverrideCategoryDefaultToSpeaker, sizeof(defaultToSpeaker), &defaultToSpeaker);
		if (status) {
			AU_LOG(@"error setting kAudioSessionProperty_OverrideCategoryDefaultToSpeaker (status = %ld)", status);
			return PdAudioError;
		}
	}
    UInt32 mix = allowsMixing ? 1 : 0;
    status = AudioSessionSetProperty(kAudioSessionProperty_OverrideCategoryMixWithOthers, sizeof(mix), &mix);
    if (status) {
        AU_LOG(@"error setting kAudioSessionProperty_OverrideCategoryMixWithOthers to %@ (status = %ld)", (allowsMixing ? @"YES" : @"NO"), status);
        return PdAudioError;
    }
    mixingEnabled_ = allowsMixing;
    return PdAudioOK;
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
    if (error) return PdAudioError;
    
    AU_LOGV(@"numberFrames: %d, specified bufferDuration: %f", numberFrames, bufferDuration);
    AU_LOGV(@"preferredIOBufferDuration: %f", globalSession.preferredIOBufferDuration);
    
    int tpb = self.ticksPerBuffer;
    if (tpb != ticksPerBuffer) {
        AU_LOG(@"*** WARNING *** could not set IO buffer duration to match %d ticks, got %d ticks instead", ticksPerBuffer, tpb);
        return PdAudioPropertyChanged;
    }
    return PdAudioOK;
}

- (void)setActive:(BOOL)active {
    self.audioUnit.active = active;
    active_ = self.audioUnit.isActive;
}

#if __IPHONE_OS_VERSION_MIN_REQUIRED >= 60000
// receives interrupt notification and calls ex-AVAudioSessionDelegate methods
- (void)interruptionOcurred:(NSNotification*)notification {
    NSDictionary *interuptionDict = notification.userInfo;
    NSUInteger interuptionType = (NSUInteger)[interuptionDict valueForKey:AVAudioSessionInterruptionTypeKey];
    if (interuptionType == AVAudioSessionInterruptionTypeBegan) {
        [self beginInterruption];
	}
    else if (interuptionType == AVAudioSessionInterruptionTypeEnded) {
        [self endInterruptionWithFlags:(NSUInteger)[interuptionDict valueForKey:AVAudioSessionInterruptionOptionKey]];
	}
}
#endif

- (void)beginInterruption {
    self.audioUnit.active = NO;  // Leave active_ unchanged.
	AU_LOGV(@"interrupted");
}

- (void)endInterruptionWithFlags:(NSUInteger)flags {
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= 60000
	if (flags == AVAudioSessionInterruptionOptionShouldResume) {
#else
	if (flags == AVAudioSessionInterruptionFlags_ShouldResume) {
#endif
        self.active = active_;  // Not redundant due to weird ObjC accessor.
		AU_LOGV(@"ended interruption");
	} else {
		AU_LOGV(@"still interrupted");
	}
}

- (void)print {
	AVAudioSession *globalSession = [AVAudioSession sharedInstance];
	AU_LOG(@"----- AVAudioSession properties -----");
	AU_LOG(@"category: %@", globalSession.category);
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= 60000
	AU_LOG(@"sampleRate: %.0f", globalSession.sampleRate);
	AU_LOG(@"preferredSampleRate: %.0f", globalSession.preferredSampleRate);
	AU_LOG(@"preferredIOBufferDuration: %f", globalSession.preferredIOBufferDuration);

	AU_LOG(@"inputAvailable: %@", (globalSession.inputAvailable ? @"YES" : @"NO"));
	AU_LOG(@"inputNumberOfChannels: %d", globalSession.inputNumberOfChannels);
	AU_LOG(@"outputNumberOfChannels: %d", globalSession.outputNumberOfChannels);
#else
	AU_LOG(@"currentHardwareSampleRate: %.0f", globalSession.currentHardwareSampleRate);
	AU_LOG(@"preferredHardwareSampleRate: %.0f", globalSession.preferredHardwareSampleRate);
	AU_LOG(@"preferredIOBufferDuration: %f", globalSession.preferredIOBufferDuration);

	AU_LOG(@"inputIsAvailable: %@", (globalSession.inputIsAvailable ? @"YES" : @"NO"));
	AU_LOG(@"currentHardwareInputNumberOfChannels: %d", globalSession.currentHardwareInputNumberOfChannels);
	AU_LOG(@"currentHardwareOutputNumberOfChannels: %d", globalSession.currentHardwareOutputNumberOfChannels);
#endif
	[self.audioUnit print];
}

@end
