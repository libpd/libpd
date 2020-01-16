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
                                  outputs:(BOOL)hasOutputs
                                isAmbient:(BOOL)isAmbient;

/// configure audio unit
- (PdAudioStatus)configureAudioUnitWithInputChannels:(int)inputChannels
                                      outputChannels:(int)outputChannels
                                        inputEnabled:(BOOL)inputEnabled;

@end

@implementation PdAudioController {
	BOOL _optionsChanged; ///< have the audio session options changed?
}

- (instancetype)init {
	self = [self initWithAudioUnit:[[PdAudioUnit alloc] init]];
	return self;
}

- (instancetype)initWithAudioUnit:(PdAudioUnit *)audioUnit {
	self = [super init];
	if (self) {
		_mixWithOthers = YES;
		_defaultToSpeaker = YES;
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
                                   inputChannels:(int)inputChannels
                                  outputChannels:(int)outputChannels {
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
	                                 outputs:(outputChannels > 0)
	                               isAmbient:NO];
	if (status == PdAudioError) {
		return PdAudioError;
	}
	status |= [self configureAudioUnitWithInputChannels:inputChannels
	                                     outputChannels:outputChannels
	                                       inputEnabled:inputEnabled];
	AU_LOGV(@"configuration finished: status %d", status);
	return status;
}

- (PdAudioStatus)configureRecordWithSampleRate:(int)sampleRate
                                numberChannels:(int)numberChannels {
	PdAudioStatus status = [self updateSampleRate:sampleRate];
	if (status == PdAudioError) {
		return PdAudioError;
	}
	status |= [self selectCategoryWithInputs:YES outputs:NO isAmbient:NO];
	if (status == PdAudioError) {
		return PdAudioError;
	}
	status |= [self configureAudioUnitWithInputChannels:numberChannels
	                                     outputChannels:0
	                                       inputEnabled:YES];
	AU_LOGV(@"configuration finished: status %d", status);
	return status;
}

- (PdAudioStatus)configureAmbientWithSampleRate:(int)sampleRate
                                 numberChannels:(int)numberChannels {
	PdAudioStatus status = [self updateSampleRate:sampleRate];
	if (status == PdAudioError) {
		return PdAudioError;
	}
	status |= [self selectCategoryWithInputs:NO outputs:numberChannels isAmbient:YES];
	if (status == PdAudioError) {
		return PdAudioError;
	}
	status |= [self configureAudioUnitWithInputChannels:0
	                                     outputChannels:numberChannels
	                                       inputEnabled:NO];
	AU_LOGV(@"configuration finished: status %d", status);
	return status;
}

- (PdAudioStatus)configureAmbientWithSampleRate:(int)sampleRate
                                 numberChannels:(int)numberChannels
                                  mixingEnabled:(BOOL)mixingEnabled {
	_mixWithOthers = mixingEnabled;
	return [self configureAmbientWithSampleRate:sampleRate numberChannels:numberChannels];
}

- (PdAudioStatus)configurePlaybackWithSampleRate:(int)sampleRate
                                  numberChannels:(int)numberChannels
                                    inputEnabled:(BOOL)inputEnabled
                                   mixingEnabled:(BOOL)mixingEnabled {
	_mixWithOthers = mixingEnabled;
	return [self configurePlaybackWithSampleRate:sampleRate
	                               inputChannels:(inputEnabled ? numberChannels : 0)
	                              outputChannels:numberChannels];
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
	AVAudioSessionCategoryOptions options = 0;
	if (self.mixWithOthers) {
		options |= AVAudioSessionCategoryOptionMixWithOthers;
	}
	if (self.duckOthers) {
		options |= AVAudioSessionCategoryOptionDuckOthers;
	}
	if (self.interruptSpokenAudioAndMixWithOthers) {
		options |= AVAudioSessionCategoryOptionInterruptSpokenAudioAndMixWithOthers;
	}
	return options;
}

- (AVAudioSessionCategoryOptions)playAndRecordOptions {
	AVAudioSessionCategoryOptions options = 0;
	if (self.mixWithOthers) {
		options |= AVAudioSessionCategoryOptionMixWithOthers;
	}
	if (self.duckOthers) {
		options |= AVAudioSessionCategoryOptionDuckOthers;
	}
	if (self.interruptSpokenAudioAndMixWithOthers) {
		options |= AVAudioSessionCategoryOptionInterruptSpokenAudioAndMixWithOthers;
	}
	if (self.defaultToSpeaker) {
		options |= AVAudioSessionCategoryOptionDefaultToSpeaker;
	}
	if (self.allowBluetooth) {
		options |= AVAudioSessionCategoryOptionAllowBluetooth;
	}
	if (self.allowBluetoothA2DP) {
		options |= AVAudioSessionCategoryOptionAllowBluetoothA2DP;
	}
	if (self.allowAirPlay) {
		options |= AVAudioSessionCategoryOptionAllowAirPlay;
	}
	return options;
}

- (AVAudioSessionCategoryOptions)recordOptions {
	AVAudioSessionCategoryOptions options = 0;
	if (self.allowBluetooth) {
		options |= AVAudioSessionCategoryOptionAllowBluetooth;
	}
	return options;
}

- (AVAudioSessionCategoryOptions)soloAmbientOptions {
	return 0;
}

- (AVAudioSessionCategoryOptions)ambientOptions {
	AVAudioSessionCategoryOptions options = 0;
	if (self.duckOthers) {
		options |= AVAudioSessionCategoryOptionDuckOthers;
	}
	return options;
}

+ (BOOL)addSessionOptions:(AVAudioSessionCategoryOptions)options {
	options = options | AVAudioSession.sharedInstance.categoryOptions;
	return [PdAudioController setSessionOptions:options];
}

+ (BOOL)setSessionOptions:(AVAudioSessionCategoryOptions)options {
	NSError *error;
	if (![AVAudioSession.sharedInstance setCategory:AVAudioSession.sharedInstance.category
	                                   withOptions:options error:&error]) {
		AU_LOG(@"error setting audio session options: %@", error.localizedDescription);
		return NO;
	}
	return YES;
}

#pragma mark Overridden Getters/Setters

// make sure option changes are applied on restart
- (void)setActive:(BOOL)active {
	self.audioUnit.active = active;
	_active = self.audioUnit.isActive;
	if (_optionsChanged) {
		[self updateAudioSessionOptions];
	}
}

- (int)ticksPerBuffer {
	NSTimeInterval bufferDuration = AVAudioSession.sharedInstance.IOBufferDuration;
	AU_LOGV(@"IOBufferDuration: %f seconds", bufferDuration);
	_ticksPerBuffer = round((bufferDuration * self.sampleRate) /
	                        (NSTimeInterval)PdBase.getBlockSize);
	return _ticksPerBuffer;
}

- (void)setMixWithOthers:(BOOL)mixWithOthers {
	if (_mixWithOthers == mixWithOthers) {return;}
	_mixWithOthers = mixWithOthers;
	[self updateAudioSessionOptions];
}

- (void)setDuckOthers:(BOOL)duckOthers {
	if (_duckOthers == duckOthers) {return;}
	_duckOthers = duckOthers;
	[self updateAudioSessionOptions];
}

- (void)setInterruptSpokenAudioAndMixWithOthers:(BOOL)interruptSpokenAudioAndMixWithOthers {
	if (_interruptSpokenAudioAndMixWithOthers == interruptSpokenAudioAndMixWithOthers) {return;}
	_interruptSpokenAudioAndMixWithOthers = interruptSpokenAudioAndMixWithOthers;
	[self updateAudioSessionOptions];
}

- (void)setDefaultToSpeaker:(BOOL)defaultToSpeaker {
	if (_mixWithOthers == defaultToSpeaker) {return;}
	_defaultToSpeaker = defaultToSpeaker;
	[self updateAudioSessionOptions];
}

- (void)setAllowBluetooth:(BOOL)allowBluetooth {
	if (_allowBluetooth == allowBluetooth) {return;}
	_allowBluetooth = allowBluetooth;
	[self updateAudioSessionOptions];
}

- (void)setAllowBluetoothA2DP:(BOOL)allowBluetoothA2DP {
	if (_allowBluetoothA2DP == allowBluetoothA2DP) {return;}
	_allowBluetoothA2DP = allowBluetoothA2DP;
	[self updateAudioSessionOptions];
}

- (void)setAllowAirPlay:(BOOL)allowAirPlay {
	if (_allowAirPlay == allowAirPlay) {return;}
	_allowAirPlay = allowAirPlay;
	[self updateAudioSessionOptions];
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
#ifdef 1
		self.active = _active; // retrigger
		AU_LOGV(@"ended interruption");
#else
		if (option == AVAudioSessionInterruptionOptionShouldResume) {
			self.active = _active; // retrigger
			AU_LOGV(@"ended interruption");
		} else {
			AU_LOGV(@"still interrupted");
		}
	}
#endif
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
                                  outputs:(BOOL)hasOutputs
                                isAmbient:(BOOL)isAmbient {
	NSString *category;
	OSStatus status;
	if (hasInputs && isAmbient) {
		AU_LOG(@"impossible session config, this should never happen");
		return PdAudioError;
	}

	// set category
	if (!isAmbient) {
		if (!hasOutputs) {
			category = AVAudioSessionCategoryRecord;
		}
		else {
			category = hasInputs ? AVAudioSessionCategoryPlayAndRecord : AVAudioSessionCategoryPlayback;
		}
	}
	else {
		category = _mixWithOthers ? AVAudioSessionCategoryAmbient : AVAudioSessionCategorySoloAmbient;
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
		if (options) {
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
	else if ([category isEqualToString:AVAudioSessionCategoryAmbient]) {
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

- (void)updateAudioSessionOptions {
	if (!self.isActive) {
		// can't change now
		_optionsChanged = YES;
		return;
	}
	AVAudioSession *session = AVAudioSession.sharedInstance;
	AVAudioSessionCategoryOptions options = 0;
	if ([session.category isEqualToString:AVAudioSessionCategoryPlayback]) {
		options = [self playbackOptions];
	}
	if ([session.category isEqualToString:AVAudioSessionCategoryPlayAndRecord]) {
		options = [self playAndRecordOptions];
	}
	if ([session.category isEqualToString:AVAudioSessionCategoryRecord]) {
		options = [self recordOptions];
	}
	if ([session.category isEqualToString:AVAudioSessionCategorySoloAmbient]) {
		options = [self soloAmbientOptions];
	}
	if ([session.category isEqualToString:AVAudioSessionCategoryAmbient]) {
		options = [self ambientOptions];
	}
	if (options) {
		[PdAudioController addSessionOptions:options];
	}
	_optionsChanged = NO;
}

@end
