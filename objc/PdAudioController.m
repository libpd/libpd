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
#if TARGET_OS_IPHONE
	#import <AVFoundation/AVFoundation.h>
#else
// OSX Helpers
OSStatus getDefaultInputDevice(AudioDeviceID defaultDevice);
OSStatus getDefaultOutputDevice(AudioDeviceID defaultDevice);
int getNumChannelsForDevice(AudioDeviceID device, AudioObjectPropertyScope scope);
#endif

@interface PdAudioController ()

@property (nonatomic, retain, readwrite) PdAudioUnit *audioUnit; // our PdAudioUnit

// updates the sample rate while verifying it is in sync with the audio session and PdAudioUnit
- (PdAudioStatus)updateSampleRate:(int)sampleRate;

// Not all inputs make sense, but that's okay in the private interface.
- (PdAudioStatus)selectCategoryWithInputs:(BOOL)hasInputs isAmbient:(BOOL)isAmbient allowsMixing:(BOOL)allowsMixing;

- (PdAudioStatus)configureAudioUnitWithNumberChannels:(int)numChannels inputEnabled:(BOOL)inputEnabled;

#if TARGET_OS_IPHONE && __IPHONE_OS_VERSION_MIN_ALLOWED >= 60000
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
	self = [self initWithAudioUnit:[[[PdAudioUnit alloc] init] autorelease]];
	return self;
}

- (id)initWithAudioUnit:(PdAudioUnit *)audioUnit {
	self = [super init];
	if (self) {
	#if TARGET_OS_IPHONE
		AVAudioSession *globalSession = [AVAudioSession sharedInstance];
		#if __IPHONE_OS_VERSION_MIN_REQUIRED >= 60000
			[[NSNotificationCenter defaultCenter] addObserver:self
													 selector:@selector(interruptionOcurred:)
														 name:AVAudioSessionInterruptionNotification
													   object:nil];
		#else
			// AVAudioSessionDelegate is deprecated starting in iOS 6
			globalSession.delegate = self;
		#endif
		NSError *error = nil;
		[globalSession setActive:YES error:&error];
		AU_LOG_IF_ERROR(error, @"Audio Session activation failed");
		if(!error) {
			AU_LOGV(@"Audio Session initialized");
		}
	#else // OSX
		
	#endif
		self.audioUnit = audioUnit;
	}
	return self;
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
#if TARGET_OS_IPHONE
	if (inputEnabled && ![[AVAudioSession sharedInstance] inputIsAvailable]) {
		inputEnabled = NO;
		status |= PdAudioPropertyChanged;
	}
#else // OSX
	// try to get default input as a check
	AudioObjectID inputDevice = kAudioObjectUnknown;
	UInt32 size = sizeof (inputDevice);
	AudioObjectPropertyAddress property;
	property.mScope = kAudioObjectPropertyScopeGlobal;
	property.mElement = kAudioObjectPropertyElementMaster;
	property.mSelector = kAudioHardwarePropertyDefaultInputDevice;
	OSStatus ret = AudioObjectGetPropertyData(kAudioObjectSystemObject, &property, 0, NULL, &size, &inputDevice);
	AU_LOG_IF_ERROR(ret, @"error getting input device id (status = %ld)", status);
	if (inputEnabled && inputDevice == kAudioDeviceUnknown) {
		inputEnabled = NO;
		status |= PdAudioPropertyChanged;
	}
#endif
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
#if TARGET_OS_IPHONE
	NSError *error = nil;
	AVAudioSession *globalSession = [AVAudioSession sharedInstance];
	[globalSession setPreferredIOBufferDuration:bufferDuration error:&error];
	if (error) return PdAudioError;
	
	AU_LOGV(@"numberFrames: %d, specified bufferDuration: %f", numberFrames, bufferDuration);
	AU_LOGV(@"preferredIOBufferDuration: %f", globalSession.preferredIOBufferDuration);
#else // OSX
	AudioObjectID outputDevice = kAudioObjectUnknown;
	UInt32 size = sizeof (outputDevice);
	AudioObjectPropertyAddress property;
	property.mScope = kAudioObjectPropertyScopeGlobal;
	property.mElement = kAudioObjectPropertyElementMaster;
	property.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
	OSStatus status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &property, 0, NULL, &size, &outputDevice);
	AU_LOG_IF_ERROR(status, @"error getting output device id (status = %ld)", status);
	if (status == kAudioObjectUnknown) {
		return PdAudioError;
	}
	size = sizeof(Float32);
	property.mSelector = kAudioDevicePropertyNominalSampleRate;
	status = AudioHardwareServiceSetPropertyData(outputDevice, &property, 0, NULL, &size, &bufferDuration);
	AU_LOG_IF_ERROR(status, @"error setting output device sample rate (status = %ld)", status);
	if (status != kAudioHardwareNoError) {
		return PdAudioError;
	}
#endif
	int tpb = self.ticksPerBuffer;
	if (tpb != ticksPerBuffer) {
		AU_LOG(@"*** WARNING *** could not set IO buffer duration to match %d ticks, got %d ticks instead", ticksPerBuffer, tpb);
		return PdAudioPropertyChanged;
	}
	return PdAudioOK;
}

- (void)print {
#if TARGET_OS_IPHONE
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
#else
	AU_LOG(@"----- AudioHardwareService properties -----");

	UInt32 size; OSStatus status;
	AudioObjectPropertyAddress property;
	property.mScope = kAudioObjectPropertyScopeGlobal;
	property.mElement = kAudioObjectPropertyElementMaster;
	
	// get default devices
	AudioDeviceID inputDevice = kAudioDeviceUnknown;
	//OSStatus status = [self getDefaultInputDevice:&inputDevice];
	size = sizeof(AudioDeviceID);
	property.mSelector = kAudioHardwarePropertyDefaultInputDevice;
	status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &property, 0, NULL, &size, &inputDevice);
	AU_LOG_IF_ERROR(status, @"error getting intput device id (status = %ld)", status);
	if(status != kAudioHardwareNoError) {
		return;
	}
	
	AudioDeviceID outputDevice = kAudioDeviceUnknown;
	//status = [self getDefaultOutputDevice:&outputDevice];
	property.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
	status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &property, 0, NULL, &size, &outputDevice);
	AU_LOG_IF_ERROR(status, @"error getting output device id (status = %ld)", status);
	if(status != kAudioHardwareNoError) {
		return;
	}
	
	// print nominal (preferred) sample rate
	double sampleRate = 0;
	property.mSelector = kAudioDevicePropertyNominalSampleRate;
	size = sizeof(double);
	status = AudioHardwareServiceGetPropertyData(outputDevice, &property, 0, NULL, &size, &sampleRate);
	AU_LOG_IF_ERROR(status, @"error getting output device nominal sample rate (status = %ld)", status);
	AU_LOG(@"nominal sample rate: %d", (int)sampleRate);
	
	// print actual sample rate
	property.mSelector = kAudioDevicePropertyActualSampleRate;
	size = sizeof(double);
	status = AudioHardwareServiceGetPropertyData(outputDevice, &property, 0, NULL, &size, &sampleRate);
	AU_LOG_IF_ERROR(status, @"error getting output device actual sample rate (status = %ld)", status);
	AU_LOG(@"actual sample rate: %d", (int)sampleRate);
	
	// print buffer size
	UInt32 bufferSize = 0;
	property.mSelector = kAudioDevicePropertyBufferSize;
	size = sizeof(UInt32);
	status = AudioHardwareServiceGetPropertyData(outputDevice, &property, 0, NULL, &size, &bufferSize);
	AU_LOG_IF_ERROR(status, @"error getting output device buffer size (status = %ld)", status);
	AU_LOG(@"buffer size: %d", bufferSize);
	
	// print number of inputs
	int numChannels = 0;
	size = 0;
	//UInt32 dataSize = 0;
//	AudioBufferList *bufferList;
//	property.mSelector = kAudioDevicePropertyStreams;
//    property.mScope = kAudioDevicePropertyScopeInput;
//	status = AudioHardwareServiceGetPropertyDataSize(inputDevice, &property, 0, NULL, &size);
//	AU_LOG_IF_ERROR(status, @"error getting device stream config size (status = %ld)", status);
//	if(kAudioHardwareNoError == status) {
//		AudioBufferList *bufferList = (AudioBufferList *)(malloc(size));
//		if(bufferList) { // this shouldn't happen
//			status = AudioHardwareServiceGetPropertyData(inputDevice, &property, 0, NULL, &size, bufferList);
//			AU_LOG_IF_ERROR(status, @"error getting device buffer list (status = %ld)", status);
//			if(kAudioHardwareNoError != status && bufferList->mNumberBuffers > 0) {
//				int numChannels = bufferList->mBuffers[0].mNumberChannels;
//			}
//			free(bufferList);
//		}
//		else {
//			AU_LOG(@"unable to allocate AudioBufferList");
//		}
//	}
	
	numChannels = getNumChannelsForDevice(inputDevice, kAudioDevicePropertyScopeInput);
	AU_LOG(@"number of input channels: %d", numChannels);
	
	// print number of outputs
	numChannels = getNumChannelsForDevice(outputDevice, kAudioDevicePropertyScopeOutput);
	AU_LOG(@"number of output channels: %d", numChannels);
#endif
	[self.audioUnit print];
}

#pragma mark - Overridden Getters/Setters

- (int)ticksPerBuffer {
	Float32 asBufferDuration = 0;
#if TARGET_OS_IPHONE
	#if __IPHONE_OS_VERSION_MIN_REQUIRED >= 60000
		NSTimeInterval asBufferDuration = [[AVAudioSession sharedInstance] IOBufferDuration];
		AU_LOGV(@"IOBufferDuration: %f seconds", asBufferDuration);
	#else
		UInt32 size = sizeof(asBufferDuration);
		OSStatus status = AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareIOBufferDuration, &size, &asBufferDuration);
		AU_LOG_IF_ERROR(status, @"error getting audio session buffer duration (status = %ld)", status);
		AU_LOGV(@"kAudioSessionProperty_CurrentHardwareIOBufferDuration: %f seconds", asBufferDuration);
	#endif
#else // OSX
	AudioObjectID outputDevice = kAudioObjectUnknown;
	UInt32 size = sizeof (outputDevice); OSStatus status;
	AudioObjectPropertyAddress property;
	property.mScope = kAudioObjectPropertyScopeGlobal;
	property.mElement = kAudioObjectPropertyElementMaster;
	property.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
	status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &property, 0, NULL, &size, &outputDevice);
	//OSStatus status = [self getDefaultOutputDevice:&outputDevice];
	AU_LOG_IF_ERROR(status, @"error getting output device id (status = %ld)", status);
	if(!outputDevice == kAudioObjectUnknown) {
		size = sizeof(Float32);
		property.mSelector = kAudioDevicePropertyBufferSize;
		status = AudioHardwareServiceGetPropertyData(outputDevice, &property, 0, NULL, &size, &asBufferDuration);
		AU_LOG_IF_ERROR(status, @"error getting output device buffer size (status = %ld)", status);
	}
#endif
	ticksPerBuffer_ = round((asBufferDuration * self.sampleRate) /  (NSTimeInterval)[PdBase getBlockSize]);
	return ticksPerBuffer_;
}

- (void)setActive:(BOOL)active {
	self.audioUnit.active = active;
	active_ = self.audioUnit.isActive;
}

#pragma mark - Private

- (PdAudioStatus)updateSampleRate:(int)sampleRate {
	double currentHardwareSampleRate = 0;
#if TARGET_OS_IPHONE
	AVAudioSession *globalSession = [AVAudioSession sharedInstance];
	NSError *error = nil;
	#if __IPHONE_OS_VERSION_MIN_REQUIRED >= 60000
		[globalSession setPreferredSampleRate:sampleRate error:&error];
		if (error) {
			return PdAudioError;
		}
		currentHardwareSampleRate = globalSession.sampleRate;
	#else
		[globalSession setPreferredHardwareSampleRate:sampleRate error:&error];
		if (error) {
			return PdAudioError;
		}
		currentHardwareSampleRate = globalSession.currentHardwareSampleRate;
	#endif
#else // OSX
	AudioObjectID outputDevice = kAudioObjectUnknown;
	UInt32 size = sizeof (outputDevice);
	AudioObjectPropertyAddress property;
	property.mScope = kAudioObjectPropertyScopeGlobal;
	property.mElement = kAudioObjectPropertyElementMaster;
	property.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
	OSStatus status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &property, 0, NULL, &size, &outputDevice);
	AU_LOG_IF_ERROR(status, @"error getting output device id (status = %ld)", status);
	if (status != kAudioHardwareNoError) {
		return PdAudioError;
	}
	size = sizeof(double);
	property.mSelector = kAudioDevicePropertyActualSampleRate;
	status = AudioHardwareServiceGetPropertyData(outputDevice, &property, 0, NULL, &size, &currentHardwareSampleRate);
	AU_LOG_IF_ERROR(status, @"error getting output device sample rate (status = %ld)", status);
	if(status != kAudioHardwareNoError) {
		return PdAudioError;
	}
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
#if TARGET_OS_IPHONE
	NSString *category;
	OSStatus status;
	if (hasInputs && isAmbient) {
		AU_LOG(@"impossible session config (hasInputs && isAmbient), this should never happen");
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
#endif
	mixingEnabled_ = allowsMixing;
	return PdAudioOK;
}

#if TARGET_OS_IPHONE

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

#endif // TARGET_OS_IPHONE

#pragma mark - Private Mac Helpers

#if TARGET_OS_MAC

OSStatus getDefaultInputDevice(AudioDeviceID defaultDevice) {
	UInt32 size = sizeof(AudioDeviceID);
	AudioObjectPropertyAddress defaultDeviceProperty;
	defaultDeviceProperty.mSelector = kAudioHardwarePropertyDefaultInputDevice;
	defaultDeviceProperty.mScope = kAudioObjectPropertyScopeGlobal;
	defaultDeviceProperty.mElement = kAudioObjectPropertyElementMaster;
	return AudioObjectGetPropertyData(kAudioObjectSystemObject, &defaultDeviceProperty, 0, NULL, &size, defaultDevice);
}

OSStatus getDefaultOutputDevice(AudioDeviceID defaultDevice) {
	UInt32 size = sizeof(AudioDeviceID);
	AudioObjectPropertyAddress defaultDeviceProperty;
	defaultDeviceProperty.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
	defaultDeviceProperty.mScope = kAudioObjectPropertyScopeGlobal;
	defaultDeviceProperty.mElement = kAudioObjectPropertyElementMaster;
	return AudioObjectGetPropertyData(kAudioObjectSystemObject, &defaultDeviceProperty, 0, NULL, &size, defaultDevice);
}

int getNumChannelsForDevice(AudioDeviceID device, AudioObjectPropertyScope scope) {
	
	UInt32 numChannels = 0;
	AudioBufferList *bufferList;
	
	UInt32 size = 0;
	AudioObjectPropertyAddress property;
	property.mScope = scope;
	property.mElement = kAudioObjectPropertyElementMaster;
	property.mSelector = kAudioDevicePropertyStreams;
	OSStatus status = AudioHardwareServiceGetPropertyDataSize(device, &property, 0, NULL, &size);
	AU_LOG_IF_ERROR(status, @"error getting device stream config size (status = %ld)", status);
	if(kAudioHardwareNoError == status) {
	
		AudioBufferList *bufferList = (AudioBufferList *)(malloc(size));
		if(!bufferList) { // this shouldn't happen
			AU_LOG(@"unable to allocate AudioBufferList");
			return numChannels;
		}

		status = AudioHardwareServiceGetPropertyData(device, &property, 0, NULL, &size, bufferList);
		AU_LOG_IF_ERROR(status, @"error getting device buffer list (status = %ld)", status);
		if(status == kAudioHardwareNoError && bufferList->mNumberBuffers > 0) {
			numChannels = bufferList->mBuffers[0].mNumberChannels;
		}
		
		free(bufferList);
	}
	return numChannels;
}

#endif // TARGET_OS_MAC

@end
