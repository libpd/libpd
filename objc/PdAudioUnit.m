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
#import "PdBuffer.h"
#import "AudioHelpers.h"
#import <AudioToolbox/AudioToolbox.h>

static const AudioUnitElement kInputElement = 1;
static const AudioUnitElement kOutputElement = 0;

@interface PdAudioUnit () {
@private
	BOOL _inputEnabled;
	BOOL _initialized;
}

@property (nonatomic) PdBuffer *buffer;
@property (nonatomic) NSInteger samplesPerBlock;

- (BOOL)initAudioUnitWithSampleRate:(Float64)sampleRate numberChannels:(int)numChannels inputEnabled:(BOOL)inputEnabled;
- (void)destroyAudioUnit;
- (AudioComponentDescription)ioDescription;
@end

@implementation PdAudioUnit

//@synthesize audioUnit = audioUnit_;
//@synthesize active = active_;

#pragma mark - Init / Dealloc

- (instancetype)init {
	self = [super init];
	if (self) {
		_initialized = NO;
		_active = NO;
	}
	return self;
}

- (void)dealloc {
	[self destroyAudioUnit];
}

#pragma mark - Public Methods

- (void)setActive:(BOOL)active {
	if (!_initialized) {
		return;
	}
	if (active == _active) {
		return;
	}
	if (active) {
		AU_RETURN_IF_ERROR(AudioOutputUnitStart(_audioUnit));
	} else {
		AU_RETURN_IF_ERROR(AudioOutputUnitStop(_audioUnit));
	}
	_active = active;
}

- (int)configureWithSampleRate:(Float64)sampleRate numberChannels:(int)numChannels inputEnabled:(BOOL)inputEnabled {
	Boolean wasActive = self.isActive;
	_inputEnabled = inputEnabled;
	if (![self initAudioUnitWithSampleRate:sampleRate numberChannels:numChannels inputEnabled:_inputEnabled]) {
		return -1;
	}
	[PdBase openAudioWithSampleRate:sampleRate inputChannels:(_inputEnabled ? numChannels : 0) outputChannels:numChannels];
	[PdBase computeAudio:YES];
    self.samplesPerBlock = [PdBase getBlockSize] * numChannels;
    self.buffer = [[PdBuffer alloc] initWithCapacity:self.samplesPerBlock];
	self.active = wasActive;
	return 0;
}

- (void)print {
	if (!_initialized) {
		AU_LOG(@"Audio Unit not initialized");
		return;
	}
	
	UInt32 sizeASBD = sizeof(AudioStreamBasicDescription);
	
	if (_inputEnabled) {
		AudioStreamBasicDescription inputStreamDescription;
		memset (&inputStreamDescription, 0, sizeof(inputStreamDescription));
		AU_RETURN_IF_ERROR(AudioUnitGetProperty(_audioUnit,
                           kAudioUnitProperty_StreamFormat,
                           kAudioUnitScope_Output,
                           kInputElement,
                           &inputStreamDescription,
                           &sizeASBD));
		AU_LOG(@"input ASBD:");
		AU_LOG(@"  mSampleRate: %.0fHz", inputStreamDescription.mSampleRate);
		AU_LOG(@"  mChannelsPerFrame: %u", (unsigned int)inputStreamDescription.mChannelsPerFrame);
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
	AU_RETURN_IF_ERROR(AudioUnitGetProperty(_audioUnit,
                       kAudioUnitProperty_StreamFormat,
                       kAudioUnitScope_Input,
                       kOutputElement,
                       &outputStreamDescription,
                       &sizeASBD));
	AU_LOG(@"output ASBD:");
	AU_LOG(@"  mSampleRate: %.0fHz", outputStreamDescription.mSampleRate);
	AU_LOG(@"  mChannelsPerFrame: %u", (unsigned int)outputStreamDescription.mChannelsPerFrame);
	AU_LOGV(@"  mFormatID: %lu", outputStreamDescription.mFormatID);
	AU_LOGV(@"  mFormatFlags: %lu", outputStreamDescription.mFormatFlags);
	AU_LOGV(@"  mBytesPerPacket: %lu", outputStreamDescription.mBytesPerPacket);
	AU_LOGV(@"  mFramesPerPacket: %lu", outputStreamDescription.mFramesPerPacket);
	AU_LOGV(@"  mBytesPerFrame: %lu", outputStreamDescription.mBytesPerFrame);
	AU_LOGV(@"  mBitsPerChannel: %lu", outputStreamDescription.mBitsPerChannel);
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

#pragma mark - AURenderCallback

static OSStatus AudioRenderCallback(void *inRefCon,
                                    AudioUnitRenderActionFlags *ioActionFlags,
                                    const AudioTimeStamp *inTimeStamp,
                                    UInt32 inBusNumber,
                                    UInt32 inNumberFrames,
                                    AudioBufferList *ioData) {
	
	PdAudioUnit *pdAudioUnit = (__bridge PdAudioUnit *)inRefCon;
	Float32 *auBuffer = (Float32 *)ioData->mBuffers[0].mData;
	
	if (pdAudioUnit->_inputEnabled) {
		AudioUnitRender(pdAudioUnit->_audioUnit, ioActionFlags, inTimeStamp, kInputElement, inNumberFrames, ioData);
	}

    NSInteger remainSamples = ioData->mBuffers[0].mDataByteSize / sizeof(Float32);
    NSInteger offsetSamples = 0;

    while (0 < remainSamples) {
        NSInteger cachedSamples = MIN(remainSamples, pdAudioUnit.buffer.writable);
        if (0 < cachedSamples) {
            [pdAudioUnit.buffer writeTo:auBuffer + offsetSamples count:cachedSamples];
            remainSamples -= cachedSamples;
            offsetSamples += cachedSamples;
        }
        NSInteger samplesPerBlock = pdAudioUnit.samplesPerBlock;
        NSInteger numBlocks = remainSamples / samplesPerBlock;
        if (0 < numBlocks) {
            [PdBase processFloatWithInputBuffer:auBuffer + offsetSamples outputBuffer:auBuffer + offsetSamples ticks:(int)numBlocks];
            NSInteger numSamples = samplesPerBlock * numBlocks;
            remainSamples -= numSamples;
            offsetSamples += numSamples;
        }
        if (0 < remainSamples) {
            [pdAudioUnit.buffer setBytes:^NSInteger(Float32 *ptr) {
                [PdBase processFloatWithInputBuffer:ptr outputBuffer:ptr ticks:1];
                return samplesPerBlock;
            }];
        }
    }
	return noErr;
}


- (AURenderCallback)renderCallback {
	return AudioRenderCallback;
}

#pragma mark - Private

- (void)destroyAudioUnit {
	if (!_initialized) {
		return;
	}
	self.active = NO;
	_initialized = NO;
	AU_RETURN_IF_ERROR(AudioUnitUninitialize(_audioUnit));
	AU_RETURN_IF_ERROR(AudioComponentInstanceDispose(_audioUnit));
	AU_LOGV(@"destroyed audio unit");
}

- (BOOL)initAudioUnitWithSampleRate:(Float64)sampleRate numberChannels:(int)numChannels inputEnabled:(BOOL)inputEnabled {
	[self destroyAudioUnit];
	AudioComponentDescription ioDescription = [self ioDescription];
	AudioComponent audioComponent = AudioComponentFindNext(NULL, &ioDescription);
	AU_RETURN_FALSE_IF_ERROR(AudioComponentInstanceNew(audioComponent, &_audioUnit));
	
	AudioStreamBasicDescription streamDescription = [self ASBDForSampleRate:sampleRate numberChannels:numChannels];
	if (inputEnabled) {
		UInt32 enableInput = 1;
		AU_RETURN_FALSE_IF_ERROR(AudioUnitSetProperty(_audioUnit,
                                                      kAudioOutputUnitProperty_EnableIO,
                                                      kAudioUnitScope_Input,
                                                      kInputElement,
                                                      &enableInput,
                                                      sizeof(enableInput)));

		// Output scope because we're defining the output of the input element _to_ our render callback
		AU_RETURN_FALSE_IF_ERROR(AudioUnitSetProperty(_audioUnit,
                                                      kAudioUnitProperty_StreamFormat,
                                                      kAudioUnitScope_Output,
                                                      kInputElement,
                                                      &streamDescription,
                                                      sizeof(streamDescription)));
	}

	// Input scope because we're defining the input of the output element _from_ our render callback.
	AU_RETURN_FALSE_IF_ERROR(AudioUnitSetProperty(_audioUnit,
                                                  kAudioUnitProperty_StreamFormat,
                                                  kAudioUnitScope_Input,
                                                  kOutputElement,
                                                  &streamDescription,
                                                  sizeof(streamDescription)));
	
	AURenderCallbackStruct callbackStruct;
	callbackStruct.inputProc = self.renderCallback;
	callbackStruct.inputProcRefCon = (__bridge void * _Nullable)(self);
	AU_RETURN_FALSE_IF_ERROR(AudioUnitSetProperty(_audioUnit,
                                                  kAudioUnitProperty_SetRenderCallback,
                                                  kAudioUnitScope_Input,
                                                  kOutputElement,
                                                  &callbackStruct,
                                                  sizeof(callbackStruct)));
	
	AU_RETURN_FALSE_IF_ERROR(AudioUnitInitialize(_audioUnit));
	_initialized = YES;
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

@end
