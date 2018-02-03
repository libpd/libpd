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
#import "TPCircularBuffer.h"

#define kWorkingBufferLength    (1024 * 64)
#define kRenderedBufferLength   (1024 * 64)

static const AudioUnitElement kInputElement = 1;
static const AudioUnitElement kOutputElement = 0;

@interface PdAudioUnit () {
@private
	BOOL _inputEnabled;
	BOOL _initialized;
}

// `workingBuffer` is a buffer for processing audio data with pd. This is used only when input enabled.
@property (nonatomic) TPCircularBuffer workingBuffer;

// `renderedBuffer` is a buffer for storing output audio data.
@property (nonatomic) TPCircularBuffer renderedBuffer;

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
        TPCircularBufferInit(&_workingBuffer, kWorkingBufferLength);
        TPCircularBufferInit(&_renderedBuffer, kRenderedBufferLength);
	}
	return self;
}

- (void)dealloc {
    TPCircularBufferCleanup(&_workingBuffer);
    TPCircularBufferCleanup(&_renderedBuffer);
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
    TPCircularBufferCleanup(&_workingBuffer);
    TPCircularBufferCleanup(&_renderedBuffer);
    TPCircularBufferInit(&_workingBuffer, kWorkingBufferLength);
    TPCircularBufferInit(&_renderedBuffer, kRenderedBufferLength);
    // When input is enabled, we insert silent samples which size is equal to PdBase's block size.
    // This affects latency, but since processing of audio samples in `PdBase` is done for each block size,
    // it is necessary to process the number of frames passed in `AudioRenderCallback()`.
    if (_inputEnabled) {
        uint32_t blockBytes = (uint32_t)self.samplesPerBlock * sizeof(Float32);
        uint32_t availableBytes;
        void *ptr = TPCircularBufferHead(&_workingBuffer, &availableBytes);
        if (ptr != NULL && blockBytes <= availableBytes) {
            bzero(ptr, blockBytes);
            TPCircularBufferProduce(&_workingBuffer, (uint32_t)blockBytes);
        }
    }
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
    void *buffer = ioData->mBuffers[0].mData;
    void *src, *dst;
    uint32_t readableBytes, writableBytes, targetBytes;

    if (pdAudioUnit->_inputEnabled) {
        // Get input audio data.
        AudioUnitRender(pdAudioUnit->_audioUnit, ioActionFlags, inTimeStamp, kInputElement, inNumberFrames, ioData);

        // Copy input audio data to `_workingBuffer`.
        src = buffer;
        targetBytes = ioData->mBuffers[0].mDataByteSize;
        dst = TPCircularBufferHead(&pdAudioUnit->_workingBuffer, &writableBytes);
        assert(dst != NULL && targetBytes <= writableBytes);
        memcpy(dst, src, targetBytes);
        TPCircularBufferProduce(&pdAudioUnit->_workingBuffer, targetBytes);

        // Render audio data to `_renderedBuffer`.
        src = TPCircularBufferTail(&pdAudioUnit->_workingBuffer, &readableBytes);
        assert(src != NULL);
        dst = TPCircularBufferHead(&pdAudioUnit->_renderedBuffer, &writableBytes);
        assert(dst != NULL);
        NSInteger remainSamples = readableBytes / sizeof(Float32);
        NSInteger samplesPerBlock = pdAudioUnit.samplesPerBlock;
        NSInteger numBlocks = remainSamples / samplesPerBlock;
        if (0 < numBlocks) {
            targetBytes = (uint32_t)(numBlocks * samplesPerBlock * sizeof(Float32));
            assert(targetBytes <= writableBytes);
            [PdBase processFloatWithInputBuffer:src outputBuffer:dst ticks:(int)numBlocks];
            TPCircularBufferProduce(&pdAudioUnit->_renderedBuffer, targetBytes);
            TPCircularBufferConsume(&pdAudioUnit->_workingBuffer, targetBytes);
        }

        // Copy result audio data to `ioData`.
        src = TPCircularBufferTail(&pdAudioUnit->_renderedBuffer, &readableBytes);
        dst = buffer;
        targetBytes = ioData->mBuffers[0].mDataByteSize;
        assert(src != NULL && targetBytes <= readableBytes);
        memcpy(dst, src, targetBytes);
        TPCircularBufferConsume(&pdAudioUnit->_renderedBuffer, targetBytes);
    } else {
        uint32_t remainBytes = ioData->mBuffers[0].mDataByteSize;
        uint32_t blockBytes = (uint32_t)pdAudioUnit.samplesPerBlock * sizeof(Float32);
        uint32_t offsetBytes = 0;

        while (0 < remainBytes) {
            // Copy cached result data to `ioData`.
            src = TPCircularBufferTail(&pdAudioUnit->_renderedBuffer, &readableBytes);
            if (src != NULL && 0 < readableBytes) {
                targetBytes = MIN(readableBytes, remainBytes);
                memcpy(buffer + offsetBytes, src, targetBytes);
                remainBytes -= targetBytes;
                offsetBytes += targetBytes;
                TPCircularBufferConsume(&pdAudioUnit->_renderedBuffer, targetBytes);
            }

            // Render audio data to `ioData`.
            uint32_t numBlocks = remainBytes / blockBytes;
            if (0 < numBlocks) {
                targetBytes = blockBytes * numBlocks;
                [PdBase processFloatWithInputBuffer:buffer + offsetBytes outputBuffer:buffer + offsetBytes ticks:(int)numBlocks];
                remainBytes -= targetBytes;
                offsetBytes += targetBytes;
            }

            // Render audio data to `_renderedBuffer` and cache it.
            if (0 < remainBytes) {
                dst = TPCircularBufferHead(&pdAudioUnit->_renderedBuffer, &writableBytes);
                targetBytes = blockBytes;
                assert(dst != NULL && targetBytes <= writableBytes);
                bzero(dst, targetBytes);
                [PdBase processFloatWithInputBuffer:dst outputBuffer:dst ticks:1];
                TPCircularBufferProduce(&pdAudioUnit->_renderedBuffer, targetBytes);
            }
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
