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

@property (nonatomic, readwrite) int sampleRate;
@property (nonatomic, readwrite) int numberChannels;
@property (nonatomic, readwrite) BOOL inputEnabled;
@property (nonatomic, readwrite) BOOL mixingEnabled;
@property (nonatomic, readwrite) int ticksPerBuffer;

@property (nonatomic, strong, readwrite) PdAudioUnit *audioUnit;

// updates the sample rate while verifying it is in sync with the audio session and PdAudioUnit
- (PdAudioStatus)updateSampleRate:(int)sampleRate;

// not all inputs make sense, but that's okay in the private interface
- (PdAudioStatus)selectCategoryWithInputs:(BOOL)hasInputs isAmbient:(BOOL)isAmbient allowsMixing:(BOOL)allowsMixing;
- (PdAudioStatus)configureAudioUnitWithNumberChannels:(int)numChannels inputEnabled:(BOOL)inputEnabled;

@end

@implementation PdAudioController

- (instancetype)init {
	self = [self initWithAudioUnit:[[PdAudioUnit alloc] init]];
	return self;
}

- (instancetype)initWithAudioUnit:(PdAudioUnit *)audioUnit {
	self = [super init];
	if (self) {
		AVAudioSession *globalSession = [AVAudioSession sharedInstance];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(interruptionOcurred:) name:AVAudioSessionInterruptionNotification object:nil];
		NSError *error = nil;
		[globalSession setActive:YES error:&error];
		AU_LOG_IF_ERROR(error, @"Audio Session activation failed");
		AU_LOGV(@"Audio Session initialized");
		self.audioUnit = audioUnit;
	}
	return self;
}

- (int)ticksPerBuffer {
	NSTimeInterval asBufferDuration = [AVAudioSession sharedInstance].IOBufferDuration;
	AU_LOGV(@"IOBufferDuration: %f seconds", asBufferDuration);
	_ticksPerBuffer = round((asBufferDuration * self.sampleRate) /  (NSTimeInterval)[PdBase getBlockSize]);
	return _ticksPerBuffer;
}

- (void)dealloc {
	self.audioUnit = nil;
}

- (PdAudioStatus)configurePlaybackWithSampleRate:(int)sampleRate
								  numberChannels:(int)numChannels
									inputEnabled:(BOOL)inputEnabled
								   mixingEnabled:(BOOL)mixingEnabled {
	PdAudioStatus status = PdAudioOK;
	BOOL isInputAvailable = [AVAudioSession sharedInstance].inputAvailable;
	if (inputEnabled && !isInputAvailable) {
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
	_inputEnabled = inputEnabled;
	_numberChannels = numChannels;
	return [self.audioUnit configureWithSampleRate:self.sampleRate numberChannels:numChannels inputEnabled:inputEnabled] ? PdAudioError : PdAudioOK;
}

- (PdAudioStatus)updateSampleRate:(int)sampleRate {
	AVAudioSession *globalSession = [AVAudioSession sharedInstance];
	NSError *error = nil;
	[globalSession setPreferredSampleRate:sampleRate error:&error];
	if (error) {
		return PdAudioError;
	}
	double currentHardwareSampleRate = globalSession.sampleRate;
	AU_LOGV(@"currentHardwareSampleRate: %.0f", currentHardwareSampleRate);
	_sampleRate = currentHardwareSampleRate;
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
	
	// set MixWithOthers property for Playback category
	if ([category isEqualToString:AVAudioSessionCategoryPlayback]) {
		// TODO: we should be checking allowsMixing flag here, but setting that requires an update to how we handle interruptions.
// 		if (allowsMixing) {
			if (![[AVAudioSession sharedInstance] setCategory:category withOptions:AVAudioSessionCategoryOptionMixWithOthers error:&error]) {
				AU_LOG(@"error setting AVAudioSessionCategoryOptionMixWithOthers: %@", error.localizedDescription);
				return PdAudioError;
			}
// 		}
	}
	
	// set MixWithOthers & DefaultToSpeaker properties for PlayAndRecord category
	else if ([category isEqualToString:AVAudioSessionCategoryPlayAndRecord]) {
		AVAudioSessionCategoryOptions options = (allowsMixing ? (AVAudioSessionCategoryOptionDefaultToSpeaker | AVAudioSessionCategoryOptionMixWithOthers) : AVAudioSessionCategoryOptionDefaultToSpeaker);
		if (![[AVAudioSession sharedInstance] setCategory:category withOptions:options error:&error]) {
			AU_LOG(@"error setting AVAudioSessionCategoryOptionDefaultToSpeaker & AVAudioSessionCategoryOptionMixWithOthers: %@", error.localizedDescription);
			return PdAudioError;
		}
	}
	
	_mixingEnabled = allowsMixing;
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
	_active = self.audioUnit.isActive;
}

// receives interrupt notification
- (void)interruptionOcurred:(NSNotification*)notification {
	NSDictionary *interuptionDict = notification.userInfo;
	NSUInteger interuptionType = (NSUInteger)[interuptionDict valueForKey:AVAudioSessionInterruptionTypeKey];
	if (interuptionType == AVAudioSessionInterruptionTypeBegan) {
		self.audioUnit.active = NO;  // Leave active_ unchanged.
		AU_LOGV(@"interrupted");
	}
	else if (interuptionType == AVAudioSessionInterruptionTypeEnded) {
		NSUInteger option = [[interuptionDict valueForKey:AVAudioSessionInterruptionOptionKey] unsignedIntegerValue];
		if (option == AVAudioSessionInterruptionOptionShouldResume) {
			self.active = _active;  // Not redundant due to weird ObjC accessor.
			AU_LOGV(@"ended interruption");
		} else {
			AU_LOGV(@"still interrupted");
		}
	}
}

- (void)print {
	AVAudioSession *globalSession = [AVAudioSession sharedInstance];
	AU_LOG(@"----- AVAudioSession properties -----");
	AU_LOG(@"category: %@", globalSession.category);
	AU_LOG(@"sampleRate: %.0f", globalSession.sampleRate);
	AU_LOG(@"preferredSampleRate: %.0f", globalSession.preferredSampleRate);
	AU_LOG(@"preferredIOBufferDuration: %f", globalSession.preferredIOBufferDuration);

	AU_LOG(@"inputAvailable: %@", (globalSession.inputAvailable ? @"YES" : @"NO"));
	AU_LOG(@"inputNumberOfChannels: %ld", (long)globalSession.inputNumberOfChannels);
	AU_LOG(@"outputNumberOfChannels: %ld", (long)globalSession.outputNumberOfChannels);
	[self.audioUnit print];
}

@end
