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

// default buffer size: 2 channel * pd block size * sample size * 2x extra room
#define kDefaultBufferLength  (2 * 64 * sizeof(Float32) * 2)

static const AudioUnitElement kInputElement = 1;
static const AudioUnitElement kOutputElement = 0;

@interface PdAudioUnit () {
@private
	BOOL _inputEnabled;
	BOOL _initialized;
}

@property (nonatomic) TPCircularBuffer inputBuffer;
@property (nonatomic) TPCircularBuffer outputBuffer;
@property (nonatomic) NSInteger samplesPerBlock;

- (BOOL)initAudioUnitWithSampleRate:(Float64)sampleRate numberChannels:(int)numChannels inputEnabled:(BOOL)inputEnabled;
- (void)destroyAudioUnit;
- (AudioComponentDescription)ioDescription;
@end

@implementation PdAudioUnit

#pragma mark - Init / Dealloc

- (instancetype)init {
	self = [super init];
	if (self) {
		_initialized = NO;
		_active = NO;
		TPCircularBufferInit(&_inputBuffer, kDefaultBufferLength);
		TPCircularBufferInit(&_outputBuffer, kDefaultBufferLength);
	}
	return self;
}

- (void)dealloc {
	TPCircularBufferCleanup(&_inputBuffer);
	TPCircularBufferCleanup(&_outputBuffer);
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

	// (re)start audio unit and libpd
	if (![self initAudioUnitWithSampleRate:sampleRate numberChannels:numChannels inputEnabled:_inputEnabled]) {
		return -1;
	}
	[PdBase openAudioWithSampleRate:sampleRate inputChannels:(_inputEnabled ? numChannels : 0) outputChannels:numChannels];
	[PdBase computeAudio:YES];

	// allocate buffers
	self.samplesPerBlock = [PdBase getBlockSize] * numChannels;
	uint32_t blockBytes = (uint32_t)self.samplesPerBlock * sizeof(Float32);
	TPCircularBufferCleanup(&_inputBuffer);
	TPCircularBufferCleanup(&_outputBuffer);
	TPCircularBufferInit(&_inputBuffer, blockBytes * 2);  // 2x extra space
	TPCircularBufferInit(&_outputBuffer, blockBytes * 2); // 2x extra space
	if (_inputEnabled) {
		// When input is enabled, insert a number of silent samples equal to
		// PdBase's block size. This affects latency, but since processing of
		// audio samples in PdBase is done for each block size, it is necessary
		// to process the number of frames passed in AudioRenderCallback().
		uint32_t availableBytes;
		void *ptr = TPCircularBufferHead(&_inputBuffer, &availableBytes);
		if (ptr != NULL && blockBytes <= availableBytes) {
			bzero(ptr, blockBytes);
			TPCircularBufferProduce(&_inputBuffer, (uint32_t)blockBytes);
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

#pragma mark - AURenderCallbacks

// original unbuffered callback, set samplesPerBlock = log2int([PdBase getBlockSize])
//static OSStatus AudioRenderCallback(void *inRefCon,
//                                    AudioUnitRenderActionFlags *ioActionFlags,
//                                    const AudioTimeStamp *inTimeStamp,
//                                    UInt32 inBusNumber,
//                                    UInt32 inNumberFrames,
//                                    AudioBufferList *ioData) {
//
//	PdAudioUnit *pdAudioUnit = (__bridge PdAudioUnit *)inRefCon;
//	Float32 *auBuffer = (Float32 *)ioData->mBuffers[0].mData;
//
//	if (pdAudioUnit->_inputEnabled) {
//		AudioUnitRender(pdAudioUnit->_audioUnit, ioActionFlags, inTimeStamp,
//		                kInputElement, inNumberFrames, ioData);
//	}
//
//	// this is a faster way of computing (inNumberFrames / blockSize)
//	int ticks = inNumberFrames >> pdAudioUnit->_samplesPerBlock;
//	[PdBase processFloatWithInputBuffer:auBuffer outputBuffer:auBuffer ticks:ticks];
//	return noErr;
//}

// buffered callback which should handle changing device buffer lengths
static OSStatus BufferedAudioRenderCallback(void *inRefCon,
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
		// get input audio data
		AudioUnitRender(pdAudioUnit->_audioUnit, ioActionFlags, inTimeStamp,
		kInputElement, inNumberFrames, ioData);

		// copy input audio data to _inputBuffer
		src = buffer;
		targetBytes = ioData->mBuffers[0].mDataByteSize;
		dst = TPCircularBufferHead(&pdAudioUnit->_inputBuffer, &writableBytes);
		assert(dst != NULL && targetBytes <= writableBytes);
		memcpy(dst, src, targetBytes);
		TPCircularBufferProduce(&pdAudioUnit->_inputBuffer, targetBytes);

		// render audio data to _outputBuffer
		src = TPCircularBufferTail(&pdAudioUnit->_inputBuffer, &readableBytes);
		assert(src != NULL);
		dst = TPCircularBufferHead(&pdAudioUnit->_outputBuffer, &writableBytes);
		assert(dst != NULL);
		NSInteger remainingSamples = readableBytes / sizeof(Float32);
		NSInteger numBlocks = remainingSamples / pdAudioUnit->_samplesPerBlock;
		if (0 < numBlocks) {
			targetBytes = (uint32_t)(numBlocks * pdAudioUnit->_samplesPerBlock* sizeof(Float32));
			assert(targetBytes <= writableBytes);
			[PdBase processFloatWithInputBuffer:src outputBuffer:dst ticks:(int)numBlocks];
			TPCircularBufferProduce(&pdAudioUnit->_outputBuffer, targetBytes);
			TPCircularBufferConsume(&pdAudioUnit->_inputBuffer, targetBytes);
		}

		// copy result audio data to ioData
		src = TPCircularBufferTail(&pdAudioUnit->_outputBuffer, &readableBytes);
		dst = buffer;
		targetBytes = ioData->mBuffers[0].mDataByteSize;
		assert(src != NULL && targetBytes <= readableBytes);
		memcpy(dst, src, targetBytes);
		TPCircularBufferConsume(&pdAudioUnit->_outputBuffer, targetBytes);
	}
	else {
		uint32_t remainingBytes = ioData->mBuffers[0].mDataByteSize;
		uint32_t blockBytes = (uint32_t)pdAudioUnit->_samplesPerBlock * sizeof(Float32);
		uint32_t offsetBytes = 0;

		while (0 < remainingBytes) {
			// copy cached result data to ioData
			src = TPCircularBufferTail(&pdAudioUnit->_outputBuffer, &readableBytes);
			if (src != NULL && 0 < readableBytes) {
				targetBytes = MIN(readableBytes, remainingBytes);
				memcpy(buffer + offsetBytes, src, targetBytes);
				remainingBytes -= targetBytes;
				offsetBytes += targetBytes;
				TPCircularBufferConsume(&pdAudioUnit->_outputBuffer, targetBytes);
			}

			// render audio data to ioData
			uint32_t numBlocks = remainingBytes / blockBytes;
			if (0 < numBlocks) {
				targetBytes = blockBytes * numBlocks;
				[PdBase processFloatWithInputBuffer:buffer + offsetBytes
				                       outputBuffer:buffer + offsetBytes
				                              ticks:(int)numBlocks];
				remainingBytes -= targetBytes;
				offsetBytes += targetBytes;
			}

			// render audio data to _outputBuffer and cache it
			if (0 < remainingBytes) {
				dst = TPCircularBufferHead(&pdAudioUnit->_outputBuffer, &writableBytes);
				targetBytes = blockBytes;
				assert(dst != NULL && targetBytes <= writableBytes);
				bzero(dst, targetBytes);
				[PdBase processFloatWithInputBuffer:dst outputBuffer:dst ticks:1];
				TPCircularBufferProduce(&pdAudioUnit->_outputBuffer, targetBytes);
			}
		}
	}
	return noErr;
}

- (AURenderCallback)renderCallback {
	return BufferedAudioRenderCallback;
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
