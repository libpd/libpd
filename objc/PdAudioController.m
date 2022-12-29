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

/// set audio session category and apply options
- (PdAudioStatus)configureSessionWithCategory:(AVAudioSessionCategory)category;

/// try to update the preferred session sample rate, sets ivars
- (PdAudioStatus)updateSampleRate:(int)sampleRate;

/// configure audio unit, sets ivars
- (PdAudioStatus)configureAudioUnitWithInputChannels:(int)inputChannels
                                      outputChannels:(int)outputChannels
                                        inputEnabled:(BOOL)inputEnabled;

/// reconfigure audio unit with current channels, also checks if session input is enabled
- (void)reconfigureAudioUnit;

/// get current options for category
- (AVAudioSessionCategoryOptions)optionsForSessionCategory:(AVAudioSessionCategory)category;

/// update category options
- (void)updateSessionCategoryOptions;

@end

@implementation PdAudioController {
	BOOL _optionsChanged; ///< have the audio session options changed?
	AVAudioSessionCategory _category;       ///< cached category
	AVAudioSessionCategoryOptions _options; ///< cached options
}

#pragma mark Initialization

- (void)setupWithAudioUnit:(PdAudioUnit *)audioUnit {
	_mixWithOthers = YES;
	_defaultToSpeaker = YES;
	_preferStereo = YES;
	_bufferSamples = YES;
	_mode = AVAudioSessionModeDefault;
	[self setupNotifications];
	self.audioUnit = audioUnit;
}

- (instancetype)init {
	self = [super init];
	if (self) {
		[self setupWithAudioUnit:[PdAudioUnit defaultAudioUnit]];
	}
	return self;
}

- (instancetype)initWithAudioUnit:(PdAudioUnit *)audioUnit {
	self = [super init];
	if (self) {
		[self setupWithAudioUnit:audioUnit];
	}
	return self;
}

- (void)dealloc {
	[self clearNotifications];
	self.audioUnit = nil;
}

#pragma mark Configuration

- (PdAudioStatus)configurePlaybackWithSampleRate:(int)sampleRate
                                   inputChannels:(int)inputChannels
                                  outputChannels:(int)outputChannels
                                    inputEnabled:(BOOL)inputEnabled {
	PdAudioStatus status = PdAudioOK;
	AVAudioSessionCategory category = AVAudioSessionCategoryPlayback;
	if (inputEnabled) {
		category = AVAudioSessionCategoryPlayAndRecord;
		if (!AVAudioSession.sharedInstance.inputAvailable) {
			status |= PdAudioPropertyChanged;
			AU_LOG(@"*** WARN *** %@ input not available", category);
		}
		if (outputChannels < 1) {
			AU_LOG(@"*** ERROR *** %@ requires at least 1 input channel", category);
			return PdAudioError;
		}
	}
	else {
		if (outputChannels < 1) {
			AU_LOG(@"*** ERROR *** %@ requires at least 1 output channel", category);
			return PdAudioError;
		}
	}
	status |= [self updateSampleRate:sampleRate];
	if (status == PdAudioError) {
		return PdAudioError;
	}
	status |= [self configureSessionWithCategory:category];
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
                                inputChannels:(int)inputChannels {
	AVAudioSessionCategory category = AVAudioSessionCategoryRecord;
	if (inputChannels < 1) {
		AU_LOG(@"*** ERROR *** %@ requires at least 1 output channel", category);
		return PdAudioError;
	}
	PdAudioStatus status = [self updateSampleRate:sampleRate];
	if (status == PdAudioError) {
		return PdAudioError;
	}
	status |= [self configureSessionWithCategory:category];
	if (status == PdAudioError) {
		return PdAudioError;
	}
	status |= [self configureAudioUnitWithInputChannels:inputChannels
	                                     outputChannels:0
	                                       inputEnabled:YES];
	AU_LOGV(@"configuration finished: status %d", status);
	return status;
}

- (PdAudioStatus)configureAmbientWithSampleRate:(int)sampleRate
                                 outputChannels:(int)outputChannels {
	AVAudioSessionCategory category = AVAudioSessionCategorySoloAmbient;
	if (self.mixWithOthers) {
		category = AVAudioSessionCategoryAmbient;
	}
	if (outputChannels < 1) {
		AU_LOG(@"*** ERROR *** %@ requires at least 1 output channel", category);
		return PdAudioError;
	}
	PdAudioStatus status = [self updateSampleRate:sampleRate];
	if (status == PdAudioError) {
		return PdAudioError;
	}
	status |= [self configureSessionWithCategory:category];
	if (status == PdAudioError) {
		return PdAudioError;
	}
	status |= [self configureAudioUnitWithInputChannels:0
	                                     outputChannels:outputChannels
	                                       inputEnabled:NO];
	AU_LOGV(@"configuration finished: status %d", status);
	return status;
}

- (PdAudioStatus)configureMultiRouteWithSampleRate:(int)sampleRate
                                     inputChannels:(int)inputChannels
                                    outputChannels:(int)outputChannels {
	PdAudioStatus status = PdAudioOK;
	AVAudioSessionCategory category = AVAudioSessionCategoryMultiRoute;
	BOOL inputEnabled = (inputChannels != 0);
	if (inputEnabled) {
		if (!AVAudioSession.sharedInstance.inputAvailable) {
			inputEnabled = NO;
			status |= PdAudioPropertyChanged;
			AU_LOG(@"*** WARN *** %@ input not available", category);
		}
	}
	status |= [self updateSampleRate:sampleRate];
	if (status == PdAudioError) {
		return PdAudioError;
	}

	status |= [self configureSessionWithCategory:category];
	if (status == PdAudioError) {
		return PdAudioError;
	}
	status |= [self configureAudioUnitWithInputChannels:inputChannels
	                                     outputChannels:outputChannels
	                                       inputEnabled:inputEnabled];
	AU_LOGV(@"configuration finished: status %d", status);
	return status;
}

- (PdAudioStatus)configurePlaybackWithSampleRate:(int)sampleRate
                                  numberChannels:(int)numberChannels
                                    inputEnabled:(BOOL)inputEnabled
                                   mixingEnabled:(BOOL)mixingEnabled {
	_mixWithOthers = mixingEnabled;
	return [self configurePlaybackWithSampleRate:sampleRate
	                               inputChannels:numberChannels
	                              outputChannels:numberChannels
	                                inputEnabled:(BOOL)inputEnabled];
}

- (PdAudioStatus)configureAmbientWithSampleRate:(int)sampleRate
                                 numberChannels:(int)numberChannels
                                  mixingEnabled:(BOOL)mixingEnabled {
	_mixWithOthers = mixingEnabled;
	return [self configureAmbientWithSampleRate:sampleRate
	                             outputChannels:numberChannels];
}

// Note about the magic 0.5 added to numberFrames:
// Apple is doing some horrible rounding of the bufferDuration into what tries
// to give a power of two frames to the audio unit, which is inconsistent across
// different devices. As they are currently truncating, we add in this value to
// make sure the resulting ticks value is not halved.
- (PdAudioStatus)configureTicksPerBuffer:(int)ticksPerBuffer {
	AVAudioSession *session = AVAudioSession.sharedInstance;
	NSError *error = nil;
	int frames = [PdBase getBlockSize] * ticksPerBuffer;
	NSTimeInterval duration = (Float32) (frames + 0.5) / self.sampleRate;

	AU_LOGV(@"session preferred IO buffer duration: %g", session.preferredIOBufferDuration);
	AU_LOGV(@"requested IO buffer duration: %g (frames %d)", duration, frames);
	[session setPreferredIOBufferDuration:duration error:&error];
	if (error) {
		AU_LOGV(@"*** ERROR *** could not set preferred IO buffer duration: %@",
		        AVStatusCodeAsString((int)error.code));
		return PdAudioError;
	}

	int tpb = self.ticksPerBuffer;
	if (tpb != ticksPerBuffer) {
		AU_LOG(@"*** WARNING *** could not set preferred IO buffer duration to %d ticks, "
		       "using %d", ticksPerBuffer, tpb);
		return PdAudioPropertyChanged;
	}
	return PdAudioOK;
}

#pragma mark Subclass Overrides

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
	if (self.defaultToSpeaker) {
		options |= AVAudioSessionCategoryOptionDefaultToSpeaker;
	}
	if (self.allowBluetooth) {
		options |= AVAudioSessionCategoryOptionAllowBluetooth;
	}
	if (self.interruptSpokenAudioAndMixWithOthers) {
		options |= AVAudioSessionCategoryOptionInterruptSpokenAudioAndMixWithOthers;
	}
	if (@available(iOS 10.0, *)) {
		if (self.allowBluetoothA2DP) {
			options |= AVAudioSessionCategoryOptionAllowBluetoothA2DP;
		}
		if (self.allowAirPlay) {
			options |= AVAudioSessionCategoryOptionAllowAirPlay;
		}
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

#pragma mark Util

- (void)print {
	AVAudioSession *session = AVAudioSession.sharedInstance;
	AU_LOG(@"----- audio session properties -----");
	AU_LOG(@"category: %@", session.category);
	AU_LOG(@"sample rate: %g", session.sampleRate);
	AU_LOG(@"preferred sample rate: %g", session.preferredSampleRate);
	AU_LOG(@"preferred IO buffer duration: %g", session.preferredIOBufferDuration);
	AU_LOG(@"input available: %@", (session.inputAvailable ? @"yes" : @"no"));
	AU_LOG(@"input channels: %d", (int)session.inputNumberOfChannels);
	AU_LOG(@"output channels: %d", (int)session.outputNumberOfChannels);
	[self.audioUnit print];
}

+ (BOOL)addSessionOptions:(AVAudioSessionCategoryOptions)options {
	options = options | AVAudioSession.sharedInstance.categoryOptions;
	return [PdAudioController setSessionOptions:options];
}

+ (BOOL)setSessionOptions:(AVAudioSessionCategoryOptions)options {
	NSError *error;
	AVAudioSession *session = AVAudioSession.sharedInstance;
	if (![session setCategory:session.category withOptions:options error:&error]) {
		AU_LOG(@"*** ERROR *** could not set %@ options: %@",
		       session.category, AVStatusCodeAsString((int)error.code));
		return NO;
	}
	return YES;
}

+ (BOOL)setSessionMode:(AVAudioSessionMode)mode {
	NSError *error;
	AVAudioSession *session = AVAudioSession.sharedInstance;
	if (![session setMode:mode error:&error]) {
		AU_LOG(@"*** ERROR *** could not set %@ mode: %@",
		       mode, AVStatusCodeAsString((int)error.code));
		return NO;
	}
	return YES;
}

#pragma mark Notifications

- (void)setupNotifications {
	[NSNotificationCenter.defaultCenter addObserver:self
	                                       selector:@selector(interruptionOccurred:)
	                                           name:AVAudioSessionInterruptionNotification
	                                         object:nil];
	[NSNotificationCenter.defaultCenter addObserver:self
	                                       selector:@selector(routeChanged:)
	                                           name:AVAudioSessionRouteChangeNotification
	                                         object:nil];
	[NSNotificationCenter.defaultCenter addObserver:self
	                                       selector:@selector(silenceSecondaryAudio:)
	                                           name:AVAudioSessionSilenceSecondaryAudioHintNotification
	                                         object:nil];
	[NSNotificationCenter.defaultCenter addObserver:self
	                                       selector:@selector(mediaServicesWereReset:)
	                                           name:AVAudioSessionMediaServicesWereResetNotification
	                                         object:nil];
}

- (void)clearNotifications {
	[NSNotificationCenter.defaultCenter removeObserver:self
	                                              name:AVAudioSessionInterruptionNotification
	                                            object:nil];
	[NSNotificationCenter.defaultCenter removeObserver:self
	                                              name:AVAudioSessionRouteChangeNotification
	                                            object:nil];
	[NSNotificationCenter.defaultCenter removeObserver:self
	                                              name:AVAudioSessionSilenceSecondaryAudioHintNotification
	                                            object:nil];
	[NSNotificationCenter.defaultCenter removeObserver:self
	                                              name:AVAudioSessionMediaServicesWereResetNotification
	                                            object:nil];
}

- (void)interruptionOccurred:(NSNotification *)notification {
	NSDictionary *dict = notification.userInfo;
	NSUInteger type = [dict[AVAudioSessionInterruptionTypeKey] unsignedIntegerValue];
	if (type == AVAudioSessionInterruptionTypeBegan) {
		AU_LOGV(@"interrupted");
		self.audioUnit.active = NO; // leave _active unchanged
	}
	else if (type == AVAudioSessionInterruptionTypeEnded) {
		NSUInteger option = [dict[AVAudioSessionInterruptionOptionKey] unsignedIntegerValue];
		if (option == AVAudioSessionInterruptionOptionShouldResume) {
			AU_LOGV(@"ended interruption");
			self.active = _active; // retrigger
		}
		else {
			AU_LOGV(@"still interrupted");
			self.active = _active; // retrigger
		}
	}
}

- (void)routeChanged:(NSNotification *)notification {}

- (void)silenceSecondaryAudio:(NSNotification *)notification {}

- (void)mediaServicesWereReset:(NSNotification *)notification {
	AU_LOGV(@"media services were reset");
	if (!_category && !_options) {
		return;
	}
	[self configureSessionWithCategory:_category];
	if (self.active) {
		// force options update and restart
		_optionsChanged = YES;
		self.active = _active;
	}
	else {
		// manually reapply options
		[PdAudioController setSessionOptions:_options];
		_options = AVAudioSession.sharedInstance.categoryOptions;
	}
	AU_LOGV(@"reconfiguring audio unit");
	[self reconfigureAudioUnit];
}

- (AVAudioSessionCategory)currentCategory {
	return _category;
}

- (AVAudioSessionCategoryOptions)currentCategoryOptions {
	return _options;
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
	NSTimeInterval duration = AVAudioSession.sharedInstance.IOBufferDuration;
	AU_LOGV(@"IOBufferDuration: %g seconds", duration);
	_ticksPerBuffer = round((duration * _sampleRate) / (NSTimeInterval)[PdBase getBlockSize]);
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
	if (_defaultToSpeaker == defaultToSpeaker) {return;}
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

#pragma mark Private

- (PdAudioStatus)configureSessionWithCategory:(AVAudioSessionCategory)category {
	NSError *error = nil;
	AVAudioSession *session = AVAudioSession.sharedInstance;
	AVAudioSessionCategoryOptions options = [self optionsForSessionCategory:category];

	// set category, mode, & options
	[session setCategory:category mode:self.mode options:options error:&error];
	if (error) {
		AU_LOG(@"*** ERROR *** could not set %@ %@: %@", category, self.mode,
		       AVStatusCodeAsString((int)error.code));
		return PdAudioError;
	}
	_category = session.category;
	_options = session.categoryOptions;

	// activate
	[session setActive:YES error:&error];
	if (error) {
		AU_LOG(@"*** ERROR *** could not activate session: %@",
		       AVStatusCodeAsString((int)error.code));
		return PdAudioError;
	}
	AU_LOGV(@"session activated");

	return PdAudioOK;
}

- (PdAudioStatus)updateSampleRate:(int)sampleRate {
	AVAudioSession *session = AVAudioSession.sharedInstance;
	NSError *error = nil;
	[session setPreferredSampleRate:sampleRate error:&error];
	if (error) {
		AU_LOG(@"*** ERROR *** could not set session sample rate to %d: %@",
		       sampleRate, AVStatusCodeAsString((int)error.code));
		return PdAudioError;
	}
	_sampleRate = session.preferredSampleRate;
	AU_LOGV(@"session sample rate: %g", session.sampleRate);
	if (!floatsAreEqual(sampleRate, session.sampleRate)) {
		AU_LOG(@"*** WARN *** could not set session sample rate to %g, current %g",
		       session.preferredSampleRate, session.sampleRate);
		return PdAudioPropertyChanged;
	}
	return PdAudioOK;
}

- (PdAudioStatus)configureAudioUnitWithInputChannels:(int)inputChannels
                                      outputChannels:(int)outputChannels
                                        inputEnabled:(BOOL)inputEnabled {
	_inputEnabled = inputEnabled;
	_inputChannels = inputChannels;
	_outputChannels = outputChannels;
	if (self.preferStereo) {
		if (inputChannels > 0) {
			inputChannels = (int)MAX(inputChannels, 2);
		}
		if (outputChannels > 0) {
			outputChannels = (int)MAX(outputChannels, 2);
		}
	}
	if (!AVAudioSession.sharedInstance.inputAvailable) {
		inputEnabled = NO;
	}
	return [self.audioUnit configureWithSampleRate:self.sampleRate
	                                 inputChannels:(inputEnabled ? inputChannels : 0)
	                                outputChannels:outputChannels
	                              bufferingEnabled:self.bufferSamples] ? PdAudioError : PdAudioOK;
}

- (void)reconfigureAudioUnit {
	BOOL wasActive = self.audioUnit.isActive;
	self.audioUnit.active = NO;
	[self configureAudioUnitWithInputChannels:_inputChannels
	                           outputChannels:_outputChannels
	                             inputEnabled:_inputEnabled];
	self.audioUnit.active = wasActive;
}

- (void)updateSessionCategoryOptions {
	if (!self.audioUnit) {return;}
	if (!self.isActive) {
		// can't change now
		_optionsChanged = YES;
		return;
	}
	AVAudioSession *session = AVAudioSession.sharedInstance;
	AVAudioSessionCategoryOptions options = [self optionsForSessionCategory:session.category];
	if (options) {
		[PdAudioController setSessionOptions:options];
	}
	_options = session.categoryOptions;
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
