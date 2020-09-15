//
//  PdAudioController.m
//  libpd
//
//  Created on 11/10/11.
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//
//  Updated 2018 Dan Wilcox <danomatika@gmail.com>
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

// Update the sample rate while verifying it is in
// sync with the audio session and PdAudioUnit.
- (PdAudioStatus)updateSampleRate:(int)sampleRate;

// Not all inputs make sense, but that's okay in the private interface.
- (PdAudioStatus)selectCategoryWithInputs:(BOOL)hasInputs
                                isAmbient:(BOOL)isAmbient
							 allowsMixing:(BOOL)allowsMixing;
- (PdAudioStatus)configureAudioUnitWithNumberChannels:(int)numChannels
                                         inputEnabled:(BOOL)inputEnabled;

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
		[[NSNotificationCenter defaultCenter] addObserver:self
		                                         selector:@selector(interruptionOccurred:)
		                                             name:AVAudioSessionInterruptionNotification
		                                           object:nil];
		NSError *error = nil;
		[globalSession setActive:YES error:&error];
		AU_LOG_IF_ERROR(error, @"Audio Session activation failed");
		AU_LOGV(@"Audio Session initialized");
		self.audioUnit = audioUnit;
	}
	return self;
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

// Note about the magic 0.5 added to numberFrames:
// Apple is doing some horrible rounding of the bufferDuration into what tries
// to give a power of two frames to the audio unit, which is inconsistent across
// different devices. As they are currently truncating, we add in this value to
// make sure the resulting ticks value is not halved.
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

- (AVAudioSessionCategoryOptions)playbackOptions {
	return 0;
}

- (AVAudioSessionCategoryOptions)playAndRecordOptions {
	return AVAudioSessionCategoryOptionDefaultToSpeaker;
}

- (AVAudioSessionCategoryOptions)soloAmbientOptions {
	return 0;
}

- (AVAudioSessionCategoryOptions)ambientOptions {
	return 0;
}

+ (BOOL)addSessionOptions:(AVAudioSessionCategoryOptions)options {
	options = options | [[AVAudioSession sharedInstance] categoryOptions];
	return [PdAudioController setSessionOptions:options];
}

+ (BOOL)setSessionOptions:(AVAudioSessionCategoryOptions)options {
	NSError *error;
	NSString *const category = [[AVAudioSession sharedInstance] category];
	if(![[AVAudioSession sharedInstance] setCategory:category
	                                     withOptions:options error:&error]) {
		AU_LOG(@"error setting audio session options: %@", error.localizedDescription);
		return NO;
	}
	return YES;
}

#pragma mark Callbacks

// receives interrupt notification
- (void)interruptionOccurred:(NSNotification*)notification {
	NSDictionary *interuptionDict = notification.userInfo;
	NSUInteger interuptionType = (NSUInteger)[interuptionDict valueForKey:AVAudioSessionInterruptionTypeKey];
	if (interuptionType == AVAudioSessionInterruptionTypeBegan) {
		self.audioUnit.active = NO; // Leave _active unchanged.
		AU_LOGV(@"interrupted");
	}
	else if (interuptionType == AVAudioSessionInterruptionTypeEnded) {
		NSUInteger option = [[interuptionDict valueForKey:AVAudioSessionInterruptionOptionKey] unsignedIntegerValue];
		// TODO: always resume for now, do we really need to handle the ShouldResume hint?
//		if (option == AVAudioSessionInterruptionOptionShouldResume) {
			self.active = _active; // Not redundant due to weird ObjC accessor.
			AU_LOGV(@"ended interruption");
//		} else {
//			AU_LOGV(@"still interrupted");
//		}
	}
}

#pragma mark Overridden Getters/Setters

- (void)setActive:(BOOL)active {
	self.audioUnit.active = active;
	_active = self.audioUnit.isActive;
}

- (int)ticksPerBuffer {
	NSTimeInterval asBufferDuration = [AVAudioSession sharedInstance].IOBufferDuration;
	AU_LOGV(@"IOBufferDuration: %f seconds", asBufferDuration);
	_ticksPerBuffer = round((asBufferDuration * self.sampleRate) /
	                        (NSTimeInterval)[PdBase getBlockSize]);
	return _ticksPerBuffer;
}

#pragma mark Private

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
		AU_LOG(@"*** WARNING *** could not update samplerate, resetting to match audio session (%.0fHz)",
		       currentHardwareSampleRate);
		return PdAudioPropertyChanged;
	}
	return PdAudioOK;
}

- (PdAudioStatus)selectCategoryWithInputs:(BOOL)hasInputs
                                isAmbient:(BOOL)isAmbient
                             allowsMixing:(BOOL)allowsMixing {
	NSString *category;
	OSStatus status;
	if (hasInputs && isAmbient) {
		AU_LOG(@"impossible session config, this should never happen");
		return PdAudioError;
	}

	// set category
	if (!isAmbient) {
		category = hasInputs ? AVAudioSessionCategoryPlayAndRecord : AVAudioSessionCategoryPlayback;
	}
	else {
		category = allowsMixing ? AVAudioSessionCategoryAmbient : AVAudioSessionCategorySoloAmbient;
	}
	NSError *error = nil;
	[[AVAudioSession sharedInstance] setCategory:category error:&error];
	if (error) {
		AU_LOG(@"failed to set session category, error %@", error);
		return PdAudioError;
	}

	// configure options
	if ([category isEqualToString:AVAudioSessionCategoryPlayback]) {
		AVAudioSessionCategoryOptions options = [self playbackOptions];
		if (allowsMixing) {
			options = AVAudioSessionCategoryOptionMixWithOthers | options;
		}
		if (options) {
			if (![[AVAudioSession sharedInstance] setCategory:category
			                                      withOptions:options
			                                            error:&error]) {
				AU_LOG(@"error setting playback options: %@",
				       error.localizedDescription);
				return PdAudioError;
			}
		}
	}
	else if ([category isEqualToString:AVAudioSessionCategoryPlayAndRecord]) {
		AVAudioSessionCategoryOptions options = [self playAndRecordOptions];
		if (allowsMixing) {
			options = AVAudioSessionCategoryOptionMixWithOthers | options;
		}
		if(options) {
			if (![[AVAudioSession sharedInstance] setCategory:category
			                                      withOptions:options
			                                            error:&error]) {
				AU_LOG(@"error setting play and record options: %@", error.localizedDescription);
				return PdAudioError;
			}
		}
	}
	else if([category isEqualToString:AVAudioSessionCategorySoloAmbient]) {
		AVAudioSessionCategoryOptions options = [self soloAmbientOptions];
		if (options) {
			if (![[AVAudioSession sharedInstance] setCategory:category
			                                      withOptions:options
			                                            error:&error]) {
				AU_LOG(@"error setting solo ambient options: %@",
				       error.localizedDescription);
				return PdAudioError;
			}
		}
	}
	else if([category isEqualToString:AVAudioSessionCategoryAmbient]) {
		AVAudioSessionCategoryOptions options = [self ambientOptions];
		if (options) {
			if (![[AVAudioSession sharedInstance] setCategory:category
			                                      withOptions:options
			                                            error:&error]) {
				AU_LOG(@"error setting ambient options: %@",
				       error.localizedDescription);
				return PdAudioError;
			}
		}
	}

	_mixingEnabled = allowsMixing;
	return PdAudioOK;
}

- (PdAudioStatus)configureAudioUnitWithNumberChannels:(int)numChannels
                                         inputEnabled:(BOOL)inputEnabled {
	_inputEnabled = inputEnabled;
	_numberChannels = numChannels;
	return [self.audioUnit configureWithSampleRate:self.sampleRate
	                                numberChannels:numChannels
	                                  inputEnabled:inputEnabled] ? PdAudioError : PdAudioOK;
}

@end
