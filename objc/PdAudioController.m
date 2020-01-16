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

/// set audio session category and apply options
- (PdAudioStatus)configureSessionCategory:(AVAudioSessionCategory)category;

/// configure audio unit
- (PdAudioStatus)configureAudioUnitWithInputChannels:(int)inputChannels
                                      outputChannels:(int)outputChannels
                                        inputEnabled:(BOOL)inputEnabled;

/// get current options for category
- (AVAudioSessionCategoryOptions)optionsForSessionCategory:(AVAudioSessionCategory)category;

/// update category options
- (void)updateSessionCategoryOptions;

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
	AVAudioSessionCategory category = AVAudioSessionCategoryPlayAndRecord;
	BOOL inputEnabled = (inputChannels > 0);
	if (inputEnabled) {
		if (!AVAudioSession.sharedInstance.inputAvailable) {
			inputEnabled = NO;
			status |= PdAudioPropertyChanged;
		}
	}
	else {
		category = AVAudioSessionCategoryPlayback;
		if (outputChannels < 1) {
			AU_LOG(@"*** ERROR *** %@ requires at least 1 output channel", category);
			return PdAudioError;
		}
	}
	status |= [self updateSampleRate:sampleRate];
	if (status == PdAudioError) {
		return PdAudioError;
	}

	status |= [self configureSessionCategory:category];
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
	AVAudioSessionCategory category = AVAudioSessionCategoryRecord;
	if (numberChannels < 1) {
		AU_LOG(@"*** ERROR *** %@ requires at least 1 output channel", category);
		return PdAudioError;
	}
	PdAudioStatus status = [self updateSampleRate:sampleRate];
	if (status == PdAudioError) {
		return PdAudioError;
	}
	status |= [self configureSessionCategory:category];
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
	AVAudioSessionCategory category = AVAudioSessionCategorySoloAmbient;
	if (self.mixWithOthers) {
		category = AVAudioSessionCategoryAmbient;
	}
	if (numberChannels < 1) {
		AU_LOG(@"*** ERROR *** %@ requires at least 1 output channel", category);
		return PdAudioError;
	}
	PdAudioStatus status = [self updateSampleRate:sampleRate];
	if (status == PdAudioError) {
		return PdAudioError;
	}
	status |= [self configureSessionCategory:category];
	if (status == PdAudioError) {
		return PdAudioError;
	}
	status |= [self configureAudioUnitWithInputChannels:0
	                                     outputChannels:numberChannels
	                                       inputEnabled:NO];
	AU_LOGV(@"configuration finished: status %d", status);
	return status;
}

- (PdAudioStatus)configureMultiRouteWithSampleRate:(int)sampleRate
                                     inputChannels:(int)inputChannels
                                    outputChannels:(int)outputChannels {
	PdAudioStatus status = PdAudioOK;
	AVAudioSessionCategory category = AVAudioSessionCategoryMultiRoute;
	BOOL inputEnabled = (inputChannels > 0);
	if (inputEnabled) {
		if (!AVAudioSession.sharedInstance.inputAvailable) {
			inputEnabled = NO;
			status |= PdAudioPropertyChanged;
		}
	}
	status |= [self updateSampleRate:sampleRate];
	if (status == PdAudioError) {
		return PdAudioError;
	}

	status |= [self configureSessionCategory:category];
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
	AU_LOG(@"sample rate: %g", session.sampleRate);
	AU_LOG(@"preferred sample rate: %g", session.preferredSampleRate);
	AU_LOG(@"preferred IO buffer duration: %g", session.preferredIOBufferDuration);
	AU_LOG(@"input available: %@", (session.inputAvailable ? @"YES" : @"NO"));
	AU_LOG(@"input channels: %d", session.inputNumberOfChannels);
	AU_LOG(@"output channels: %d", session.outputNumberOfChannels);
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

- (AVAudioSessionCategoryOptions)multiRouteOptions {
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

+ (BOOL)addSessionOptions:(AVAudioSessionCategoryOptions)options {
	options = options | AVAudioSession.sharedInstance.categoryOptions;
	return [PdAudioController setSessionOptions:options];
}

+ (BOOL)setSessionOptions:(AVAudioSessionCategoryOptions)options {
	NSError *error;
	if (![AVAudioSession.sharedInstance setCategory:AVAudioSession.sharedInstance.category
	                                   withOptions:options error:&error]) {
		AU_LOG(@"*** ERROR *** could not set %@ options: %@",
			   AVAudioSession.sharedInstance.category, error.localizedDescription);
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
		[self updateSessionCategoryOptions];
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
	[self updateSessionCategoryOptions];
}

- (void)setDuckOthers:(BOOL)duckOthers {
	if (_duckOthers == duckOthers) {return;}
	_duckOthers = duckOthers;
	[self updateSessionCategoryOptions];
}

- (void)setInterruptSpokenAudioAndMixWithOthers:(BOOL)interruptSpokenAudioAndMixWithOthers {
	if (_interruptSpokenAudioAndMixWithOthers == interruptSpokenAudioAndMixWithOthers) {return;}
	_interruptSpokenAudioAndMixWithOthers = interruptSpokenAudioAndMixWithOthers;
	[self updateSessionCategoryOptions];
}

- (void)setDefaultToSpeaker:(BOOL)defaultToSpeaker {
	if (_mixWithOthers == defaultToSpeaker) {return;}
	_defaultToSpeaker = defaultToSpeaker;
	[self updateSessionCategoryOptions];
}

- (void)setAllowBluetooth:(BOOL)allowBluetooth {
	if (_allowBluetooth == allowBluetooth) {return;}
	_allowBluetooth = allowBluetooth;
	[self updateSessionCategoryOptions];
}

- (void)setAllowBluetoothA2DP:(BOOL)allowBluetoothA2DP {
	if (_allowBluetoothA2DP == allowBluetoothA2DP) {return;}
	_allowBluetoothA2DP = allowBluetoothA2DP;
	[self updateSessionCategoryOptions];
}

- (void)setAllowAirPlay:(BOOL)allowAirPlay {
	if (_allowAirPlay == allowAirPlay) {return;}
	_allowAirPlay = allowAirPlay;
	[self updateSessionCategoryOptions];
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
#if 1
		self.active = _active; // retrigger
		AU_LOGV(@"ended interruption");
#else
		if (option == AVAudioSessionInterruptionOptionShouldResume) {
			self.active = _active; // retrigger
			AU_LOGV(@"ended interruption");
		}
		else {
			AU_LOGV(@"still interrupted");
		}
#endif
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

- (PdAudioStatus)configureSessionCategory:(AVAudioSessionCategory)category {
	NSError *error = nil;
	[AVAudioSession.sharedInstance setCategory:category error:&error];
	if (error) {
		AU_LOG(@"*** ERROR *** could not set %@: %@", category, error);
		return PdAudioError;
	}
	AVAudioSessionCategoryOptions options = [self optionsForSessionCategory:category];
	if (![PdAudioController setSessionOptions:options]) {
		return PdAudioError;
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

- (void)updateSessionCategoryOptions {
	if (!self.isActive) {
		// can't change now
		_optionsChanged = YES;
		return;
	}
	AVAudioSession *session = AVAudioSession.sharedInstance;
	AVAudioSessionCategoryOptions options = [self optionsForSessionCategory:session.category];
	if (options) {
		[PdAudioController addSessionOptions:options];
	}
	_optionsChanged = NO;
}

- (AVAudioSessionCategoryOptions)optionsForSessionCategory:(AVAudioSessionCategory)category {
	AVAudioSessionCategoryOptions options = 0;
	if ([category isEqualToString:AVAudioSessionCategoryPlayback]) {
		options = [self playbackOptions];
	}
	else if ([category isEqualToString:AVAudioSessionCategoryPlayAndRecord]) {
		options = [self playAndRecordOptions];
	}
	else if ([category isEqualToString:AVAudioSessionCategoryRecord]) {
		options = [self recordOptions];
	}
	else if ([category isEqualToString:AVAudioSessionCategorySoloAmbient]) {
		options = [self soloAmbientOptions];
	}
	else if ([category isEqualToString:AVAudioSessionCategoryAmbient]) {
		options = [self ambientOptions];
	}
	else if ([category isEqualToString:AVAudioSessionCategoryMultiRoute]) {
		options = [self multiRouteOptions];
	}
	return options;
}

@end
