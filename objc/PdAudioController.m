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

// private setters:
@property (nonatomic) int sampleRate;
@property (nonatomic) int numberInputChannels;
@property (nonatomic) int numberOutputChannels;
@property (nonatomic) int ticksPerBuffer;

@property (nonatomic, retain) PdAudioUnit *audioUnit;	// out private PdAudioUnit
@property (nonatomic, assign) NSString *category;		// weak reference to a const NSString defined within the AudioToolbox
@property (nonatomic) BOOL inputIsAvailable;			// retreived from AVAudioSession indicating whether inputs are present
@property (nonatomic) BOOL backgroundAudioEnabled;		// specified by user in the configureBackgourndAudio.. method
@property (nonatomic) UInt32 mixingEnabled;				// store as an unsigned int for convenient use in the Audio Session API

- (void)initAudioSession;								// activates the audio session
- (PdAudioStatus)configureAudioUnit;					// calls PdAudioUnit's configure method
- (int)audioSessionTicksPerBuffer;						// calculating ticks per buffer from the audio sessions buffer size (which is provided in seconds)
- (PdAudioStatus)updateHardwareProperties;				// decides appropriate settings for the category and mixing capabilities
- (PdAudioStatus)updateSampleRate;						// updates the sample rate while verifying it is in sync with the audio session and PdAudioUnit
- (void)printAVSession;									// print log info about the current audio session

@end

@implementation PdAudioController

@synthesize sampleRate = sampleRate_,
			numberInputChannels = numberInputChannels_,
			numberOutputChannels = numberOutputChannels_,
			ticksPerBuffer = ticksPerBuffer_,
			active = active_,
			audioUnit = audioUnit_,
			category = category_,
			inputIsAvailable = inputIsAvailable_,
			backgroundAudioEnabled = backgroundAudioEnabled_,
			mixingEnabled = mixingEnabled_;

#pragma mark - Init / Dealloc

- (id)init {
	self = [super init];
    if (self) {
		[self initAudioSession];
	}
	return self;
}

- (void)dealloc {
	self.audioUnit = nil;
	self.category = nil;
	[super dealloc];
}

#pragma mark - Public Methods`

- (PdAudioStatus)configureWithSampleRate:(int)sampleRate numberInputChannels:(int)numInputs numberOutputChannels:(int)numOutputs {
	self.sampleRate = sampleRate;
	self.numberInputChannels = numInputs;
	self.numberOutputChannels = numOutputs;
	self.backgroundAudioEnabled = NO;
	self.mixingEnabled = NO;

	PdAudioStatus status = [self updateSampleRate];
	status |= [self updateHardwareProperties];
	status |= [self configureAudioUnit];

	AU_LOGV(@"configuration finished. status: %d", status);
	return status;
}

- (PdAudioStatus)configureForBackgroundAudioWithSampleRate:(int)sampleRate numberOutputChannels:(int)numOutputs mixingEnabled:(BOOL)mixingEnabled {
	self.sampleRate = sampleRate;
	self.numberOutputChannels = numOutputs;
	self.backgroundAudioEnabled = YES;
	self.mixingEnabled = (UInt32)mixingEnabled;
	
	PdAudioStatus status = PdAudioOK;
	if (self.numberInputChannels) {
		self.numberInputChannels = 0;
		AU_LOGV(@"*** Warning *** disabling inputs for background audio session to function properly.");
		status = PdAudioPropertyChanged;
	}
	
	status |= [self updateSampleRate];
	status |= [self updateHardwareProperties];
	status |= [self configureAudioUnit];

	AU_LOGV(@"configuration finished. status: %d", status);
	return status;
}

- (PdAudioStatus)configureTicksPerBuffer:(int)ticksPerBuffer {
	self.ticksPerBuffer = ticksPerBuffer;
	if (self.ticksPerBuffer != ticksPerBuffer) {
		return PdAudioPropertyChanged;
	}
	return PdAudioOK;
}

#pragma mark - Overridden Accessors

- (PdAudioUnit *)audioUnit {
	if (!audioUnit_) {
		self.audioUnit = [[[PdAudioUnit alloc] init] autorelease];
	}
	return audioUnit_;
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

- (void)setCategory:(NSString *)category {
	if (![category_ isEqualToString:category]) {
		if (category) {
			NSError *error = nil;
			[[AVAudioSession sharedInstance] setCategory:category error:&error];
			AV_CHECK_ERROR(error);
		}
		category_ = category;
	}
}

- (int)ticksPerBuffer {
	if (!ticksPerBuffer_) {
		ticksPerBuffer_ = [self audioSessionTicksPerBuffer];
		AU_LOGV(@"ticksPerBuffer was uninitialized and now set to match the audio session (%d ticks)", ticksPerBuffer_);
	}
	return ticksPerBuffer_;
}

/* note about the magic 0.5 added to numberFrames:
 * apple is doing some horrible rounding of the bufferDuration into
 * what tries to give a power of two frames to the audio unit, which
 * is inconsistent accross different devices.  As they are currently
 * truncating, we add in this value to make sure the resulting ticks
 * value is not halved.  This may break in future releases.
 */
- (void)setTicksPerBuffer:(int)ticksPerBuffer {
	if (self.ticksPerBuffer != ticksPerBuffer) {
		int numberFrames = [PdBase getBlockSize] * ticksPerBuffer;
		NSTimeInterval bufferDuration = (Float32) (numberFrames + 0.5) / self.sampleRate;
		
		NSError *error = nil;
		AVAudioSession *globalSession = [AVAudioSession sharedInstance];
		[globalSession setPreferredIOBufferDuration:bufferDuration error:&error];
		AV_CHECK_ERROR(error);
		
		AU_LOGV(@"numberFrames: %d, specified bufferDuration: %f", numberFrames, bufferDuration);
		AU_LOGV(@"preferredIOBufferDuration: %f", globalSession.preferredIOBufferDuration);
		
		int avTicks = [self audioSessionTicksPerBuffer];
		if (avTicks != ticksPerBuffer) {
			AU_LOG(@"*** WARNING *** could not set IO buffer duration to match %d ticks, got %d ticks instead", ticksPerBuffer, avTicks);
		} 
		ticksPerBuffer_ = avTicks;
	}
}

- (BOOL)inputIsAvailable {
	return [[AVAudioSession sharedInstance] inputIsAvailable];
}

#pragma mark - AVAudioSessionDelegate Protocol Methods

- (void)beginInterruption {
	if (self.active) {
		[self.audioUnit stop];
	}
	AU_LOGV(@"interrupted");
}

// FIXME: starting the audio unit here while in background mode /w mixing isn't working
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
	self.inputIsAvailable = isInputAvailable;
}

#pragma mark - Private

- (void)initAudioSession {
	AVAudioSession *globalSession = [AVAudioSession sharedInstance];
	globalSession.delegate = self;
	NSError *error = nil;
	[[AVAudioSession sharedInstance] setActive:YES error:&error];
	AV_CHECK_ERROR(error);
	AU_LOGV(@"Audio Session initialized");
}

- (PdAudioStatus)configureAudioUnit {
	PdAudioStatus status = [self.audioUnit configureWithSampleRate:self.sampleRate
											   numberInputChannels:self.numberInputChannels
											  numberOutputChannels:self.numberOutputChannels];
	
	if (status == PdAudioPropertyChanged) {
		self.sampleRate = self.audioUnit.sampleRate;
		self.numberInputChannels = self.audioUnit.numberInputChannels;
		self.numberOutputChannels = self.audioUnit.numberOutputChannels;
		return PdAudioPropertyChanged;
	}
	return status;
}

- (PdAudioStatus)updateSampleRate {
	AVAudioSession *globalSession = [AVAudioSession sharedInstance];
	
	NSError *error = nil;
	[globalSession setPreferredHardwareSampleRate:self.sampleRate error:&error];
	AV_CHECK_ERROR(error);
	
	double preferredHardwareSampleRate = globalSession.preferredHardwareSampleRate;
	double currentHardwareSampleRate = globalSession.currentHardwareSampleRate;
	AU_LOGV(@"preferredHardwareSampleRate: %.0f", preferredHardwareSampleRate);
	AU_LOGV(@"currentHardwareSampleRate: %.0f", currentHardwareSampleRate);
	
	if (!floatsAreEqual((int)self.sampleRate, currentHardwareSampleRate)) {
		AU_LOG(@"*** WARNING *** could not update samplerate, resetting to match audio session (%.0fHz)", currentHardwareSampleRate);
		self.sampleRate = round(globalSession.currentHardwareSampleRate);
		return PdAudioPropertyChanged;
	}
	return PdAudioOK;
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

- (PdAudioStatus)updateHardwareProperties {
	if (self.backgroundAudioEnabled) {
		self.category = AVAudioSessionCategoryPlayback;
		if (self.mixingEnabled) {
			OSStatus status = AudioSessionSetProperty(kAudioSessionProperty_OverrideCategoryMixWithOthers, sizeof(mixingEnabled_), &mixingEnabled_);
			if (status) {
				AU_LOG(@"error setting kAudioSessionProperty_OverrideCategoryMixWithOthers to %@ (status = %ld)", (self.mixingEnabled ? @"YES" : @"NO"), status);
				return PdAudioError;
			}
		}
	} else if ([self inputIsAvailable]) {
		self.category = AVAudioSessionCategoryPlayAndRecord;
	} else {
		self.category = AVAudioSessionCategoryAmbient;
		if (self.numberInputChannels > 0) {
			AU_LOGV(@"*** Warning *** disabling inputs as AVAudioSession reports the device doesn't have any");
			self.numberInputChannels = 0;
			return PdAudioPropertyChanged;
		}
	}
	return PdAudioOK;
}

#pragma mark - Private (printing)

- (void)print {
	[self printAVSession];
	[self.audioUnit print];
}

- (void)printAVSession {
	AVAudioSession *globalSession = [AVAudioSession sharedInstance];
	AU_LOG(@"----- AVAudioSession properties -----");
	AU_LOG(@"category: %@", globalSession.category);
	AU_LOG(@"currentHardwareSampleRate: %.0f", globalSession.currentHardwareSampleRate);
	AU_LOG(@"preferredHardwareSampleRate: %.0f", globalSession.preferredHardwareSampleRate);
	AU_LOG(@"preferredIOBufferDuration: %f", globalSession.preferredIOBufferDuration);
	
	AU_LOG(@"inputIsAvailable: %@", (globalSession.inputIsAvailable ? @"YES" : @"NO"));
	AU_LOG(@"currentHardwareInputNumberOfChannels: %d", globalSession.currentHardwareInputNumberOfChannels);
	AU_LOG(@"currentHardwareOutputNumberOfChannels: %d", globalSession.currentHardwareOutputNumberOfChannels);
}

@end
