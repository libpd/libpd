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

/// trt to update the preferred session sample rate, sets ivars
- (PdAudioStatus)updateSampleRate:(int)sampleRate;

/// check input and output channels, sets ivars,
/// returns PdAudioPropertyChanged if either were changed
- (PdAudioStatus)updateInputChannels:(int)inputChannels outputChannels:(int)outputChannels;

/// configure audio unit, sets ivars
- (PdAudioStatus)configureAudioUnitWithInputChannels:(int)inputChannels
                                      outputChannels:(int)outputChannels
                                        inputEnabled:(BOOL)inputEnabled;

/// get current options for category
- (AVAudioSessionCategoryOptions)optionsForSessionCategory:(AVAudioSessionCategory)category;

/// update category options
- (void)updateSessionCategoryOptions;

@end

@implementation PdAudioController {
	BOOL _optionsChanged;     ///< have the audio session options changed?
	BOOL _autoInputChannels;  ///< automatically reconfigure input channels?
	BOOL _autoOutputChannels; ///< automatically reconfigure output channels?
}

- (instancetype)init {
	self = [self initWithAudioUnit:[PdAudioUnit defaultAudioUnit]];
	return self;
}

- (instancetype)initWithAudioUnit:(PdAudioUnit *)audioUnit {
	self = [super init];
	if (self) {
		_mixWithOthers = YES;
		_defaultToSpeaker = YES;
		_preferStereo = YES;
		[self setupNotifications];
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
	BOOL inputEnabled = (inputChannels != 0);
	if (inputEnabled) {
		if (!AVAudioSession.sharedInstance.inputAvailable) {
			inputEnabled = NO;
			status |= PdAudioPropertyChanged;
			AU_LOG(@"*** WARN *** input not available", category);
		}
	}
	else {
		category = AVAudioSessionCategoryPlayback;
		if (outputChannels == 0) {
			AU_LOG(@"*** ERROR *** %@ requires at least 1 output channel", category);
			return PdAudioError;
		}
	}
	_autoInputChannels = (inputChannels < 0);
	_autoOutputChannels = (outputChannels < 0);
	status |= [self updateSampleRate:sampleRate];
	if (status == PdAudioError) {
		return PdAudioError;
	}
	status |= [self configureSessionWithCategory:category];
	if (status == PdAudioError) {
		return PdAudioError;
	}
	[self updateInputChannels:inputChannels outputChannels:outputChannels];
	status |= [self configureAudioUnitWithInputChannels:_inputChannels
	                                     outputChannels:_outputChannels
	                                       inputEnabled:inputEnabled];
	AU_LOGV(@"configuration finished: status %d", status);
	return status;
}

- (PdAudioStatus)configureRecordWithSampleRate:(int)sampleRate
                                inputChannels:(int)inputChannels {
	AVAudioSessionCategory category = AVAudioSessionCategoryRecord;
	if (inputChannels == 0) {
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
	_autoInputChannels = (inputChannels < 0);
	_autoOutputChannels = NO;
	[self updateInputChannels:inputChannels outputChannels:0];
	status |= [self configureAudioUnitWithInputChannels:_inputChannels
	                                     outputChannels:_outputChannels
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
	if (outputChannels == 0) {
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
	_autoInputChannels = NO;
	_autoOutputChannels = (_outputChannels < 0);
	[self updateInputChannels:0 outputChannels:outputChannels];
	status |= [self configureAudioUnitWithInputChannels:_inputChannels
	                                     outputChannels:_outputChannels
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
			AU_LOG(@"*** WARN *** input not available", category);
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
	_autoInputChannels = (_inputChannels < 0);
	_autoOutputChannels = (_outputChannels < 0);
	[self updateInputChannels:inputChannels outputChannels:outputChannels];
	status |= [self configureAudioUnitWithInputChannels:_inputChannels
	                                     outputChannels:_outputChannels
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
	                               inputChannels:(inputEnabled ? numberChannels : 0)
	                              outputChannels:numberChannels];
}

- (PdAudioStatus)configureAmbientWithSampleRate:(int)sampleRate
                                 numberChannels:(int)numberChannels
                                  mixingEnabled:(BOOL)mixingEnabled {
	_mixWithOthers = mixingEnabled;
	return [self configureAmbientWithSampleRate:sampleRate outputChannels:numberChannels];
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
	[NSNotificationCenter.defaultCenter addObserver:self
	                                       selector:@selector(routeChanged:)
	                                           name:AVAudioSessionRouteChangeNotification
	                                         object:nil];
}

- (void)clearNotifications {
	[NSNotificationCenter.defaultCenter removeObserver:self
	                                              name:AVAudioSessionInterruptionNotification
	                                            object:nil];
	[NSNotificationCenter.defaultCenter removeObserver:self
	                                              name:AVAudioSessionRouteChangeNotification
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
		// TODO: always resume for now, do we really need to handle the ShouldResume hint?
#if 1
		self.active = _active; // retrigger
		AU_LOGV(@"ended interruption");
#else
		NSUInteger option = [dict[AVAudioSessionInterruptionOptionKey] unsignedIntegerValue];
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

- (void)routeChanged:(NSNotification *)notification {
	BOOL reconfigure = NO;
	NSDictionary *dict = notification.userInfo;
	NSUInteger reason = [dict[AVAudioSessionRouteChangeReasonKey] unsignedIntegerValue];
	if (reason == AVAudioSessionRouteChangeReasonNewDeviceAvailable) {
		AU_LOGV(@"route changed: new device");
		reconfigure = YES;
	}
	else if (reason == AVAudioSessionRouteChangeReasonOldDeviceUnavailable) {
		AU_LOGV(@"route changed: old device unavailable");
		reconfigure = YES;
	}
	if (reconfigure) {
		dispatch_async(dispatch_get_main_queue(), ^{
			AVAudioSession *session = AVAudioSession.sharedInstance;
			AU_LOGV(@"session current route: inputs %d outputs %d",
					session.inputNumberOfChannels, session.outputNumberOfChannels);
			if ([self updateInputChannels:self->_inputChannels
				           outputChannels:self->_outputChannels] == PdAudioPropertyChanged) {
				AU_LOG(@"reconfiguring audio unit: inputs %d outputs %d",
						self->_inputChannels, self->_outputChannels);
				BOOL inputEnabled = (self->_inputChannels > 0);
				if (!session.inputAvailable ||
					[session.category isEqualToString:AVAudioSessionCategoryPlayback] ||
					[session.category isEqualToString:AVAudioSessionCategoryAmbient] ||
					[session.category isEqualToString:AVAudioSessionCategorySoloAmbient]) {
					inputEnabled = NO;
				}
				if ([self configureAudioUnitWithInputChannels:self->_inputChannels
					                           outputChannels:self->_outputChannels
					                             inputEnabled:inputEnabled] != PdAudioOK) {
				}
			}
		});
	}
}

#pragma mark Private

- (PdAudioStatus)configureSessionWithCategory:(AVAudioSessionCategory)category {
	NSError *error = nil;
	AVAudioSession *session = AVAudioSession.sharedInstance;

	// category
	[session setCategory:category error:&error];
	if (error) {
		AU_LOG(@"*** ERROR *** could not set %@: %@", category,
		    AVStatusCodeAsString((int)error.code));
		return PdAudioError;
	}
	AVAudioSessionCategoryOptions options = [self optionsForSessionCategory:category];
	if (![PdAudioController setSessionOptions:options]) {
		return PdAudioError;
	}

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
		AU_LOG(@"*** ERROR *** could not set session sample rate to %g: %@",
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

- (PdAudioStatus)updateInputChannels:(int)inputChannels outputChannels:(int)outputChannels {
	NSError *error = nil;
	AVAudioSession *session = AVAudioSession.sharedInstance;
	PdAudioStatus ret = PdAudioOK;

	// inputs
	AU_LOGV(@"session input channels: %d", session.inputNumberOfChannels);
	if (_autoInputChannels) {
		inputChannels = (int)session.inputNumberOfChannels;
	}

	// outputs
	AU_LOGV(@"session output channels: %d", session.outputNumberOfChannels);
	if (_autoOutputChannels) {
		outputChannels = (int)session.outputNumberOfChannels;
	}

	// stereo min?
	if (self.preferStereo) {
		if (inputChannels > 0) {
			inputChannels = (int)MAX(inputChannels, 2);
		}
		if (outputChannels > 0) {
			outputChannels = (int)MAX(outputChannels, 2);
		}
	}

	if (_inputChannels != inputChannels || _outputChannels != outputChannels) {
		ret = PdAudioPropertyChanged;
		AU_LOGV(@"channels changed: inputs %d -> %d outputs %d -> %d",
				_inputChannels, inputChannels, _outputChannels, outputChannels);
	}
	_inputChannels = inputChannels;
	_outputChannels = outputChannels;

	return ret;
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
	if (!self.audioUnit) {return;}
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
