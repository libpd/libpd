//
//  PdAudioUnit.m
//  libpd
//
//  Created on 29/09/11.
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//

#import "PdAudioUnit.h"
#import "PdBase.h"
#import "AudioHelpers.h"
#import <AudioToolbox/AudioToolbox.h>

static const AudioUnitElement kInputElement = 1;
static const AudioUnitElement kOutputElement = 0;

@interface PdAudioUnit ()

@property (nonatomic) Float64 sampleRate;
@property (nonatomic) int numberInputChannels;
@property (nonatomic) int numberOutputChannels;

@property (nonatomic, getter=isRunning) BOOL running;
@property (nonatomic, getter=isInitialized) BOOL initialized;

- (void)initAudioUnit;
- (void)destroyAudioUnit;
- (void)initPd;
- (void)logASBDs;
- (void)checkSampleRateProperties;
- (AudioComponentDescription)ioDescription;
- (AudioStreamBasicDescription)ASBDForNumberChannels:(UInt32)numChannels;

@end

@implementation PdAudioUnit

@synthesize audioUnit = audioUnit_;
@synthesize running	= isRunning_;
@synthesize initialized = isInitialized_;
@synthesize sampleRate = sampleRate_;
@synthesize numberInputChannels = numberInputChannels_;
@synthesize numberOutputChannels = numberOutputChannels_;

#pragma mark - Init / Dealloc

- (id)init {
    self = [super init];
    if (self) {
		isInitialized_ = NO;
		isRunning_ = NO;
		blockSizeAsLog_ = log2int([PdBase getBlockSize]);
	}
	return self;
}

- (void)dealloc {
	[self destroyAudioUnit];
	[super dealloc];
}

#pragma mark - Public Methods

- (PdAudioStatus)configureWithSampleRate:(Float64)sampleRate numberInputChannels:(int)numInputs numberOutputChannels:(int)numOutputs {
	Boolean isRunning = self.isRunning;
	if (isRunning) {
		[self stop];
	}
	if (self.isInitialized) {
		[self destroyAudioUnit];
	}
	
	[self setSampleRate:sampleRate];
	[self setNumberInputChannels:numInputs];
	[self setNumberOutputChannels:numOutputs];

	[self initAudioUnit];
	[self initPd];
	[self checkSampleRateProperties];

	if (!self.isInitialized) {
		return PdAudioError;
	}
	
	if (isRunning) {
		[self start];
	}

	if (!floatsAreEqual(self.sampleRate, sampleRate) || (self.numberInputChannels != numInputs) || (self.numberOutputChannels != numOutputs)) {
		return PdAudioPropertyChanged;
	}
	return PdAudioOK;
}

- (void)start {
	AU_CHECK_RETURN(!self.isRunning, @"audio unit already already started");
	AU_CHECK_STATUS(AudioOutputUnitStart(audioUnit_));
	self.running = YES;
	AU_LOGV(@"started audio unit");
}

- (void)stop {
	AU_CHECK_RETURN(self.isRunning, @"audio unit already stopped");
	AU_CHECK_STATUS(AudioOutputUnitStop(audioUnit_));
	self.running = NO;
	AU_LOGV(@"stopped audio unit");
}

- (void)print {
	[self logASBDs];
}

#pragma mark - Overridden Setters

- (void)setSampleRate:(Float64)sampleRate {
	if (!floatsAreEqual(sampleRate_, sampleRate)) {
		sampleRate_ = sampleRate;
	}
}

- (void)setNumberInputChannels:(int)numberInputChannels {
	if (numberInputChannels_ != numberInputChannels  && numberInputChannels >= 0) {
		numberInputChannels_ = numberInputChannels;
	}
}

- (void)setNumberOutputChannels:(int)numberOutputChannels {
	if (numberOutputChannels_ != numberOutputChannels && numberOutputChannels > 0) {
		numberOutputChannels_ = numberOutputChannels;
	}
}

#pragma mark - AURenderCallback

static OSStatus AudioRenderCallback(void *inRefCon,
									AudioUnitRenderActionFlags *ioActionFlags,
									const AudioTimeStamp *inTimeStamp,
									UInt32 inBusNumber,
									UInt32 inNumberFrames,
									AudioBufferList *ioData) {
	
	PdAudioUnit *pdAudioUnit = (PdAudioUnit *)inRefCon;
	Float32 *auBuffer = (Float32 *)ioData->mBuffers[0].mData;

	if (pdAudioUnit->numberInputChannels_ > 0) {
		AudioUnitRender(pdAudioUnit->audioUnit_, ioActionFlags, inTimeStamp, kInputElement, inNumberFrames, ioData);
	}

	int ticks = inNumberFrames >> pdAudioUnit->blockSizeAsLog_; // this is a faster way of computing (inNumberFrames / blockSize)
	[PdBase processFloatWithInputBuffer:auBuffer outputBuffer:auBuffer ticks:ticks];
	return noErr;
}

#pragma mark - Private

- (void)initAudioUnit {
	AU_CHECK_RETURN(self.numberOutputChannels > 0, @"numberOutputChannels (%d) must be greater than 0", self.numberOutputChannels);

	AudioComponentDescription ioDescription = [self ioDescription];
	AudioComponent audioComponent = AudioComponentFindNext(NULL, &ioDescription);
	AU_CHECK_STATUS(AudioComponentInstanceNew(audioComponent, &audioUnit_));
	
	AURenderCallbackStruct callbackStruct;
	callbackStruct.inputProc = AudioRenderCallback;
	callbackStruct.inputProcRefCon = self;
	AU_CHECK_STATUS(AudioUnitSetProperty(audioUnit_,
										 kAudioUnitProperty_SetRenderCallback,
										 kAudioUnitScope_Input,
										 kOutputElement,
										 &callbackStruct,
										 sizeof(callbackStruct)));
	
	AudioStreamBasicDescription outputStreamDescription = [self ASBDForNumberChannels:self.numberOutputChannels];
	AU_CHECK_STATUS(AudioUnitSetProperty(audioUnit_,
										 kAudioUnitProperty_StreamFormat,
										 kAudioUnitScope_Input,
										 kOutputElement,
										 &outputStreamDescription,
										 sizeof(outputStreamDescription)));

	if (self.numberInputChannels > 0) {
		UInt32 enableInput = 1;
		AU_CHECK_STATUS(AudioUnitSetProperty(audioUnit_,
											 kAudioOutputUnitProperty_EnableIO,
											 kAudioUnitScope_Input,
											 kInputElement,
											 &enableInput,
											 sizeof(enableInput)));
		
		AudioStreamBasicDescription inputStreamDescription = [self ASBDForNumberChannels:self.numberInputChannels];
		AU_CHECK_STATUS(AudioUnitSetProperty(audioUnit_,
											 kAudioUnitProperty_StreamFormat,
											 kAudioUnitScope_Output,
											 kInputElement,
											 &inputStreamDescription,
											 sizeof(inputStreamDescription)));
	}
	
	AU_CHECK_STATUS(AudioUnitInitialize(audioUnit_));
	AU_LOGV(@"initialized audio unit");
	self.initialized = YES;
}

- (void)destroyAudioUnit {
	if (self.running) {
		[self stop];
	}
	AU_CHECK_RETURN(self.isInitialized, "not initialized, cannot destroy");
	AU_CHECK_STATUS(AudioUnitUninitialize(audioUnit_));
	AU_CHECK_STATUS(AudioComponentInstanceDispose(audioUnit_));
	audioUnit_ = nil;
	self.initialized = NO;
	AU_LOGV(@"destroyed audio unit");
}

- (void)initPd {
	[PdBase computeAudio:NO];
	[PdBase openAudioWithSampleRate:self.sampleRate
					  inputChannels:self.numberInputChannels
					 outputChannels:self.numberOutputChannels];
	[PdBase computeAudio:YES];
}

- (void)checkSampleRateProperties {
	UInt32 sizeFloat64 = sizeof(Float64);
	Float64 inputSampleRate = 0.0;
	Float64 outputSampleRate = 0.0;
	BOOL sampleRatesAreOK = NO;
	
	AU_CHECK_STATUS(AudioUnitGetProperty(audioUnit_,
										 kAudioUnitProperty_SampleRate,
										 kAudioUnitScope_Input,
										 kOutputElement,
										 &outputSampleRate,
										 &sizeFloat64));

	if (self.numberInputChannels > 0) {
		AU_CHECK_STATUS(AudioUnitGetProperty(audioUnit_,
											 kAudioUnitProperty_SampleRate,
											 kAudioUnitScope_Output,
											 kInputElement,
											 &inputSampleRate,
											 &sizeFloat64));
		
		sampleRatesAreOK = ((inputSampleRate > 0.1) &&
							(outputSampleRate > 0.1) &&
							floatsAreEqual(inputSampleRate, outputSampleRate) &&
							floatsAreEqual(outputSampleRate, self.sampleRate));
		AU_CHECK(sampleRatesAreOK, @"samplerate properties not correctly set (input: %f, output: %f)", inputSampleRate, outputSampleRate);
	} else {
		sampleRatesAreOK = ((outputSampleRate > 0.1) &&
							floatsAreEqual(outputSampleRate, self.sampleRate));
		AU_CHECK(sampleRatesAreOK, @"samplerate properties not correctly set (output: %f)", outputSampleRate);
	}

	if (sampleRatesAreOK) {
		AU_LOGV(@"samplerate correctly set to %0.fHz", outputSampleRate);
	}
}

- (AudioComponentDescription)ioDescription {
	AudioComponentDescription description;
	description.componentType = kAudioUnitType_Output;
	description.componentSubType = kAudioUnitSubType_RemoteIO;
	description.componentManufacturer = kAudioUnitManufacturer_Apple;
	description.componentFlags = 0;
	description.componentFlagsMask = 0;
	return description;
}

// sets the format to 32 bit, floating point, linear PCM, interleaved
- (AudioStreamBasicDescription)ASBDForNumberChannels:(UInt32)numberChannels {
	const int kFloatSize = 4;
	const int kBitSize = 8;

	AudioStreamBasicDescription description;
	memset(&description, 0, sizeof(description));
	
	description.mSampleRate = self.sampleRate;
	description.mFormatID = kAudioFormatLinearPCM;
	description.mFormatFlags = kAudioFormatFlagsNativeFloatPacked;
	description.mBytesPerPacket = kFloatSize * numberChannels;
	description.mFramesPerPacket = 1;
	description.mBytesPerFrame = kFloatSize * numberChannels;
	description.mChannelsPerFrame = numberChannels;
	description.mBitsPerChannel = kFloatSize * kBitSize;
	
	return description;
}

- (void)logASBDs {
	UInt32 sizeASBD = sizeof(AudioStreamBasicDescription);

	if (self.numberInputChannels > 0) {
		AudioStreamBasicDescription inputStreamDescription;
		memset (&inputStreamDescription, 0, sizeof(inputStreamDescription));
		AU_CHECK_STATUS(AudioUnitGetProperty(audioUnit_,
											 kAudioUnitProperty_StreamFormat,
											 kAudioUnitScope_Output,
											 kInputElement,
											 &inputStreamDescription,
											 &sizeASBD));
		AU_LOG(@"input ASBD:");
		AU_LOG(@"  mSampleRate: %.0fHz", inputStreamDescription.mSampleRate);
		AU_LOG(@"  mChannelsPerFrame: %lu", inputStreamDescription.mChannelsPerFrame);
		AU_LOGV(@"  mFormatID: %lu", inputStreamDescription.mFormatID);
		AU_LOGV(@"  mFormatFlags: %lu", inputStreamDescription.mFormatFlags);
		AU_LOGV(@"  mBytesPerPacket: %lu", inputStreamDescription.mBytesPerPacket);
		AU_LOGV(@"  mFramesPerPacket: %lu", inputStreamDescription.mFramesPerPacket);
		AU_LOGV(@"  mBytesPerFrame: %lu", inputStreamDescription.mBytesPerFrame);
		AU_LOGV(@"  mBitsPerChannel: %lu", inputStreamDescription.mBitsPerChannel);
	} else {
		AU_LOG(@"no input ASBD");
	}

	AudioStreamBasicDescription outputStreamDescription;
	memset(&outputStreamDescription, 0, sizeASBD);
	AU_CHECK_STATUS(AudioUnitGetProperty(audioUnit_,
										 kAudioUnitProperty_StreamFormat,
										 kAudioUnitScope_Input,
										 kOutputElement,
										 &outputStreamDescription,
										 &sizeASBD));
	AU_LOG(@"output ASBD:");
	AU_LOG(@"  mSampleRate: %.0fHz", outputStreamDescription.mSampleRate);
	AU_LOG(@"  mChannelsPerFrame: %lu", outputStreamDescription.mChannelsPerFrame);
	AU_LOGV(@"  mFormatID: %lu", outputStreamDescription.mFormatID);
	AU_LOGV(@"  mFormatFlags: %lu", outputStreamDescription.mFormatFlags);
	AU_LOGV(@"  mBytesPerPacket: %lu", outputStreamDescription.mBytesPerPacket);
	AU_LOGV(@"  mFramesPerPacket: %lu", outputStreamDescription.mFramesPerPacket);
	AU_LOGV(@"  mBytesPerFrame: %lu", outputStreamDescription.mBytesPerFrame);
	AU_LOGV(@"  mBitsPerChannel: %lu", outputStreamDescription.mBitsPerChannel);
	

}

@end
