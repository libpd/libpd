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

@interface PdAudioUnit () {
@private
    BOOL inputEnabled_;
    BOOL initialized_;
	int blockSizeAsLog_;
}

- (BOOL)initAudioUnitWithSampleRate:(Float64)sampleRate numberChannels:(int)numChannels inputEnabled:(BOOL)inputEnabled;
- (void)destroyAudioUnit;
- (AudioComponentDescription)ioDescription;
- (AudioStreamBasicDescription)ASBDForSampleRate:(Float64)sampleRate numberChannels:(UInt32)numChannels;

@end

@implementation PdAudioUnit

@synthesize audioUnit = audioUnit_;
@synthesize active = active_;

#pragma mark - Init / Dealloc

- (id)init {
    self = [super init];
    if (self) {
        initialized_ = NO;
		active_ = NO;
		blockSizeAsLog_ = log2int([PdBase getBlockSize]);
	}
	return self;
}

- (void)dealloc {
	[self destroyAudioUnit];
	[super dealloc];
}

#pragma mark - Public Methods

- (int)configureWithSampleRate:(Float64)sampleRate numberChannels:(int)numChannels inputEnabled:(BOOL)inputEnabled {
	Boolean wasActive = self.isActive;
    inputEnabled_ = inputEnabled;
	if (![self initAudioUnitWithSampleRate:sampleRate numberChannels:numChannels inputEnabled:inputEnabled_]) {
        return -1;
    }
	[PdBase openAudioWithSampleRate:sampleRate inputChannels:(inputEnabled_ ? numChannels : 0) outputChannels:numChannels];
	[PdBase computeAudio:YES];
	self.active = wasActive;
	return 0;
}

- (void)setActive:(BOOL)active {
    if (!initialized_) {
        return;
    }
    if (active == active_) {
        return;
    }
    if (active) {
        AU_RETURN_IF_ERROR(AudioOutputUnitStart(audioUnit_));
    } else {
        AU_RETURN_IF_ERROR(AudioOutputUnitStop(audioUnit_));
    }
    active_ = active;
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
    
	if (pdAudioUnit->inputEnabled_) {
		AudioUnitRender(pdAudioUnit->audioUnit_, ioActionFlags, inTimeStamp, kInputElement, inNumberFrames, ioData);
	}
    
	int ticks = inNumberFrames >> pdAudioUnit->blockSizeAsLog_; // this is a faster way of computing (inNumberFrames / blockSize)
	[PdBase processFloatWithInputBuffer:auBuffer outputBuffer:auBuffer ticks:ticks];
	return noErr;
}

#pragma mark - Private

- (void)destroyAudioUnit {
    if (!initialized_) {
        return;
    }
    self.active = NO;
    initialized_ = NO;
	AU_RETURN_IF_ERROR(AudioUnitUninitialize(audioUnit_));
	AU_RETURN_IF_ERROR(AudioComponentInstanceDispose(audioUnit_));
	AU_LOGV(@"destroyed audio unit");
}

- (BOOL)initAudioUnitWithSampleRate:(Float64)sampleRate numberChannels:(int)numChannels inputEnabled:(BOOL)inputEnabled {
    [self destroyAudioUnit];
	AudioComponentDescription ioDescription = [self ioDescription];
	AudioComponent audioComponent = AudioComponentFindNext(NULL, &ioDescription);
	AU_RETURN_FALSE_IF_ERROR(AudioComponentInstanceNew(audioComponent, &audioUnit_));
    
    AudioStreamBasicDescription streamDescription = [self ASBDForSampleRate:sampleRate numberChannels:numChannels];
    if (inputEnabled) {
		UInt32 enableInput = 1;
		AU_RETURN_FALSE_IF_ERROR(AudioUnitSetProperty(audioUnit_,
                                                      kAudioOutputUnitProperty_EnableIO,
                                                      kAudioUnitScope_Input,
                                                      kInputElement,
                                                      &enableInput,
                                                      sizeof(enableInput)));
		
		AU_RETURN_FALSE_IF_ERROR(AudioUnitSetProperty(audioUnit_,
                                                      kAudioUnitProperty_StreamFormat,
                                                      kAudioUnitScope_Output,  // Output scope because we're defining the output of the input element _to_ our render callback
                                                      kInputElement,
                                                      &streamDescription,
                                                      sizeof(streamDescription)));
	}
	
	AU_RETURN_FALSE_IF_ERROR(AudioUnitSetProperty(audioUnit_,
                                                  kAudioUnitProperty_StreamFormat,
                                                  kAudioUnitScope_Input,  // Input scope because we're defining the input of the output element _from_ our render callback.
                                                  kOutputElement,
                                                  &streamDescription,
                                                  sizeof(streamDescription)));
	
	AURenderCallbackStruct callbackStruct;
	callbackStruct.inputProc = AudioRenderCallback;
	callbackStruct.inputProcRefCon = self;
	AU_RETURN_FALSE_IF_ERROR(AudioUnitSetProperty(audioUnit_,
                                                  kAudioUnitProperty_SetRenderCallback,
                                                  kAudioUnitScope_Input,
                                                  kOutputElement,
                                                  &callbackStruct,
                                                  sizeof(callbackStruct)));
    
	AU_RETURN_FALSE_IF_ERROR(AudioUnitInitialize(audioUnit_));
    initialized_ = YES;
	AU_LOGV(@"initialized audio unit");
	return true;
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
- (AudioStreamBasicDescription)ASBDForSampleRate:(Float64)sampleRate numberChannels:(UInt32)numberChannels {
	const int kFloatSize = 4;
	const int kBitSize = 8;
    
	AudioStreamBasicDescription description;
	memset(&description, 0, sizeof(description));
	
	description.mSampleRate = sampleRate;
	description.mFormatID = kAudioFormatLinearPCM;
	description.mFormatFlags = kAudioFormatFlagsNativeFloatPacked;
	description.mBytesPerPacket = kFloatSize * numberChannels;
	description.mFramesPerPacket = 1;
	description.mBytesPerFrame = kFloatSize * numberChannels;
	description.mChannelsPerFrame = numberChannels;
	description.mBitsPerChannel = kFloatSize * kBitSize;
	
	return description;
}

- (void)print {
    if (!initialized_) {
		AU_LOG(@"Audio Unit not initialized");
        return;
    }
    
	UInt32 sizeASBD = sizeof(AudioStreamBasicDescription);
    
	if (inputEnabled_) {
		AudioStreamBasicDescription inputStreamDescription;
		memset (&inputStreamDescription, 0, sizeof(inputStreamDescription));
		AU_RETURN_IF_ERROR(AudioUnitGetProperty(audioUnit_,
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
	AU_RETURN_IF_ERROR(AudioUnitGetProperty(audioUnit_,
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
