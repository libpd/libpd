//
//  PdAudioController.m
//  libpd
//
//  Created on 11/10/11.
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//
//  Updated 2018, 2020 Dan Wilcox <danomatika@gmail.com>
//

#import "PdAudioController.h"
#import "PdAudioUnit.h"
#import "AudioHelpers.h"
#import "PdBase.h"
#import <AudioToolbox/AudioToolbox.h>

@interface PdAudioController ()

@property (nonatomic, readwrite) int sampleRate;
@property (nonatomic, readwrite) int inputChannels;
@property (nonatomic, readwrite) int outputChannels;
@property (nonatomic, readwrite) BOOL inputEnabled;
@property (nonatomic, readwrite) BOOL mixingEnabled;
@property (nonatomic, readwrite) int ticksPerBuffer;

@property (nonatomic, strong, readwrite) PdAudioUnit *audioUnit;

/// update the sample rate while verifying it is in sync with the audio session
/// and PdAudioUnit
- (PdAudioStatus)updateSampleRate:(int)sampleRate;

/// set audio session category
- (PdAudioStatus)selectCategoryWithInputs:(BOOL)hasInputs
                                isAmbient:(BOOL)isAmbient
                             allowsMixing:(BOOL)allowsMixing;

/// configure audio unit
- (PdAudioStatus)configureAudioUnitWithInputChannels:(int)inputChannels
                                      outputChannels:(int)outputChannels
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
		NSError *error = nil;
		[self setupNotifications];
		[AVAudioSession.sharedInstance setActive:YES error:&error];
		AU_LOG_IF_ERROR(error, @"audio session activation failed");
		AU_LOGV(@"audio session initialized");
		self.audioUnit = audioUnit;
	}
	return self;
}

- (void)dealloc {
	[self clearNotifications];
	self.audioUnit = nil;
}

- (PdAudioStatus)configurePlaybackWithSampleRate:(int)sampleRate
                                  numberChannels:(int)numChannels
                                    inputEnabled:(BOOL)inputEnabled
                                   mixingEnabled:(BOOL)mixingEnabled {
	return [self configurePlaybackWithSampleRate:sampleRate
                                   inputChannels:(inputEnabled ? numChannels : 0)
                                  outputChannels:numChannels
                                   mixingEnabled:mixingEnabled];
}

- (PdAudioStatus)configurePlaybackWithSampleRate:(int)sampleRate
                                   inputChannels:(int)inputChannels
                                  outputChannels:(int)outputChannels
                                   mixingEnabled:(BOOL)mixingEnabled {
	PdAudioStatus status = PdAudioOK;
	BOOL inputEnabled = (inputChannels > 0);
	if (inputEnabled && !AVAudioSession.sharedInstance.inputAvailable) {
		inputEnabled = NO;
		status |= PdAudioPropertyChanged;
	}
	status |= [self updateSampleRate:sampleRate];
	if (status == PdAudioError) {
		return PdAudioError;
	}
	status |= [self selectCategoryWithInputs:inputEnabled
	                               isAmbient:NO
	                            allowsMixing:mixingEnabled];
	if (status == PdAudioError) {
		return PdAudioError;
	}
	status |= [self configureAudioUnitWithInputChannels:inputChannels
	                                     outputChannels:outputChannels
	                                       inputEnabled:inputEnabled];
	AU_LOGV(@"configuration finished: status %d", status);
	return status;
}

- (PdAudioStatus)configureAmbientWithSampleRate:(int)sampleRate
                                 numberChannels:(int)numberChannels
                                  mixingEnabled:(BOOL)mixingEnabled {
	PdAudioStatus status = [self updateSampleRate:sampleRate];
	if (status == PdAudioError) {
		return PdAudioError;
	}
	status |= [self selectCategoryWithInputs:NO
	                               isAmbient:YES
	                            allowsMixing:mixingEnabled];
	if (status == PdAudioError) {
		return PdAudioError;
	}
	status |= [self configureAudioUnitWithInputChannels:0
	                                     outputChannels:numberChannels
	                                       inputEnabled:NO];
	AU_LOGV(@"configuration finished: status %d", status);
	return status;
}

// Note about the magic 0.5 added to numberFrames:
// Apple is doing some horrible rounding of the bufferDuration into what tries
// to give a power of two frames to the audio unit, which is inconsistent across
// different devices. As they are currently truncating, we add in this value to
// make sure the resulting ticks value is not halved.
- (PdAudioStatus)configureTicksPerBuffer:(int)ticksPerBuffer {
	int numberFrames = PdBase.getBlockSize * ticksPerBuffer;
	NSTimeInterval bufferDuration = (Float32) (numberFrames + 0.5) / self.sampleRate;
	
	AVAudioSession *session = AVAudioSession.sharedInstance;
	NSError *error = nil;
	[session setPreferredIOBufferDuration:bufferDuration error:&error];
	if (error) return PdAudioError;
	
	AU_LOGV(@"numberFrames: %d, specified bufferDuration: %f",
	        numberFrames, bufferDuration);
	AU_LOGV(@"preferredIOBufferDuration: %f", session.preferredIOBufferDuration);
	
	int tpb = self.ticksPerBuffer;
	if (tpb != ticksPerBuffer) {
		AU_LOG(@"*** WARNING *** could not set IO buffer duration to match "
		       "%d ticks, got %d ticks instead", ticksPerBuffer, tpb);
		return PdAudioPropertyChanged;
	}
	return PdAudioOK;
}

- (void)print {
	AVAudioSession *session = AVAudioSession.sharedInstance;
	AU_LOG(@"----- audio session properties -----");
	AU_LOG(@"category: %@", session.category);
	AU_LOG(@"sampleRate: %g", session.sampleRate);
	AU_LOG(@"preferredSampleRate: %g", session.preferredSampleRate);
	AU_LOG(@"preferredIOBufferDuration: %g", session.preferredIOBufferDuration);
	AU_LOG(@"inputAvailable: %@", (session.inputAvailable ? @"YES" : @"NO"));
	AU_LOG(@"inputNumberOfChannels: %d", session.inputNumberOfChannels);
	AU_LOG(@"outputNumberOfChannels: %d", session.outputNumberOfChannels);
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
	options = options | AVAudioSession.sharedInstance.categoryOptions;
	return [PdAudioController setSessionOptions:options];
}

+ (BOOL)setSessionOptions:(AVAudioSessionCategoryOptions)options {
	NSError *error;
	NSString *const category = AVAudioSession.sharedInstance.category;
	if(![AVAudioSession.sharedInstance setCategory:category
	                                   withOptions:options error:&error]) {
		AU_LOG(@"error setting audio session options: %@", error.localizedDescription);
		return NO;
	}
	return YES;
}

#pragma mark Overridden Getters/Setters

- (void)setActive:(BOOL)active {
	self.audioUnit.active = active;
	_active = self.audioUnit.isActive;
}

- (int)ticksPerBuffer {
	NSTimeInterval bufferDuration = AVAudioSession.sharedInstance.IOBufferDuration;
	AU_LOGV(@"IOBufferDuration: %f seconds", bufferDuration);
	_ticksPerBuffer = round((bufferDuration * self.sampleRate) /
	                        (NSTimeInterval)PdBase.getBlockSize);
	return _ticksPerBuffer;
}

#pragma mark Notifications

- (void)setupNotifications {
	[NSNotificationCenter.defaultCenter addObserver:self
	                                       selector:@selector(interruptionOccurred:)
	                                           name:AVAudioSessionInterruptionNotification
	                                         object:nil];
}

- (void)clearNotifications {
	[NSNotificationCenter.defaultCenter removeObserver:self
	                                              name:AVAudioSessionInterruptionNotification
	                                            object:nil];
}

- (void)interruptionOccurred:(NSNotification *)notification {
	NSDictionary *dict = notification.userInfo;
	NSUInteger type = [dict[AVAudioSessionInterruptionTypeKey] unsignedIntegerValue];
	if (type == AVAudioSessionInterruptionTypeBegan) {
		self.audioUnit.active = NO; // leave _active unchanged
		AU_LOGV(@"interrupted");
	}
	else if (type == AVAudioSessionInterruptionTypeEnded) {
		NSUInteger option = [dict[AVAudioSessionInterruptionOptionKey] unsignedIntegerValue];
		// TODO: always resume for now, do we really need to handle the ShouldResume hint?
//		if (option == AVAudioSessionInterruptionOptionShouldResume) {
			self.active = _active; // retrigger
			AU_LOGV(@"ended interruption");
//		} else {
//			AU_LOGV(@"still interrupted");
//		}
	}
}

#pragma mark Private

- (PdAudioStatus)updateSampleRate:(int)sampleRate {
	AVAudioSession *session = AVAudioSession.sharedInstance;
	NSError *error = nil;
	[session setPreferredSampleRate:sampleRate error:&error];
	if (error) {
		return PdAudioError;
	}
	_sampleRate = session.preferredSampleRate;
	AU_LOGV(@"session sample rate: %g", session.sampleRate);
	if (!floatsAreEqual(sampleRate, session.sampleRate)) {
		AU_LOGV(@"could not set session sample rate to preferred: %g",
		        session.preferredSampleRate);
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
	[AVAudioSession.sharedInstance setCategory:category error:&error];
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
			if (![AVAudioSession.sharedInstance setCategory:category
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
			if (![AVAudioSession.sharedInstance setCategory:category
			                                      withOptions:options
			                                            error:&error]) {
				AU_LOG(@"error setting play and record options: %@",
				       error.localizedDescription);
				return PdAudioError;
			}
		}
	}
	else if([category isEqualToString:AVAudioSessionCategorySoloAmbient]) {
		AVAudioSessionCategoryOptions options = [self soloAmbientOptions];
		if (options) {
			if (![AVAudioSession.sharedInstance setCategory:category
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
			if (![AVAudioSession.sharedInstance setCategory:category
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

- (PdAudioStatus)configureAudioUnitWithInputChannels:(int)inputChannels
                                      outputChannels:(int)outputChannels
                                        inputEnabled:(BOOL)inputEnabled {
	_inputEnabled = inputEnabled;
	_inputChannels = inputChannels;
	_outputChannels = outputChannels;
	return [self.audioUnit configureWithSampleRate:self.sampleRate
	                                 inputChannels:(inputEnabled ? inputChannels : 0)
	                                outputChannels:outputChannels] ? PdAudioError : PdAudioOK;
}

@end
