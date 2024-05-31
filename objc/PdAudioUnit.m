//
//  PdAudioUnit.m
//  libpd
//
//  Created on 29/09/11.
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//
//  Updated 2018, 2020 Dan Wilcox <danomatika@gmail.com>
//

#import "PdAudioUnit.h"
#import "PdBase.h"
#import "AudioHelpers.h"
#include "z_ringbuffer.h"

static const AudioUnitElement kRemoteIOElement_Input = 1;
static const AudioUnitElement kRemoteIOElement_Output = 0;

@interface PdAudioUnit ()

/// create and start the audio unit
- (BOOL)initAudioUnitWithSampleRate:(Float64)sampleRate
                      inputChannels:(int)inputChannels
                     outputChannels:(int)outputChannels;

/// stop and release the audio unit
- (void)clearAudioUnit;

/// update input & output channels, maxFrames, & (re)allocate buffers
/// also sets buffer size & frame ivars
- (BOOL)updateInputChannels:(int)inputChannels outputChannels:(int)outputChannels;

/// release buffers, if allocated
- (void)clearBuffers;

@end

@implementation PdAudioUnit {
	ring_buffer *_inputRingBuffer;  ///< input buffer
	ring_buffer *_outputRingBuffer; ///< output buffer
}

@synthesize audioUnit = _audioUnit;
@synthesize sampleRate = _sampleRate;
@synthesize inputEnabled = _inputEnabled;
@synthesize inputChannels = _inputChannels;
@synthesize outputChannels = _outputChannels;

#pragma mark Initialization

+ (instancetype)defaultAudioUnit {
	return [[PdAudioUnit alloc] initWithComponentDescription:PdAudioUnit.defaultIODescription
                                                     options:0
                                                       error:nil];
}

- (instancetype)initWithComponentDescription:(AudioComponentDescription)componentDescription
                                     options:(AudioComponentInstantiationOptions)options
                                       error:(NSError **)outError {
	self = [super initWithComponentDescription:componentDescription options:options error:outError];
	if (self) {
		_blockFrames = [PdBase getBlockSize];
		_blockFramesAsLog = log2int(_blockFrames);
	}
	return self;
}

- (void)dealloc {
	[self clearAudioUnit];
	[self clearBuffers];
}

#pragma mark Configuration

- (int)configureWithSampleRate:(Float64)sampleRate
                 inputChannels:(int)inputChannels
                outputChannels:(int)outputChannels {
	return [self configureWithSampleRate:sampleRate
	                       inputChannels:inputChannels
	                      outputChannels:outputChannels
	                    bufferingEnabled:YES];
}

- (int)configureWithSampleRate:(Float64)sampleRate
                 inputChannels:(int)inputChannels
                outputChannels:(int)outputChannels
              bufferingEnabled:(BOOL)bufferingEnabled {
	BOOL wasActive = self.isActive;
	_sampleRate = sampleRate;
	_inputEnabled = (inputChannels > 0);
	_inputChannels = inputChannels;
	_outputChannels = outputChannels;
	_bufferingEnabled = bufferingEnabled;
	if (![self initAudioUnitWithSampleRate:_sampleRate
	                         inputChannels:inputChannels
	                        outputChannels:outputChannels]) {
		return -1;
	}
	if (![self updateInputChannels:_inputChannels outputChannels:_outputChannels]) {
		return -1;
	}
	[PdBase openAudioWithSampleRate:sampleRate
	                  inputChannels:inputChannels
	                 outputChannels:outputChannels];
	[PdBase computeAudio:YES];
	self.active = wasActive;
	return 0;
}

- (int)configureWithSampleRate:(Float64)sampleRate
                numberChannels:(int)numChannels
                  inputEnabled:(BOOL)inputEnabled {
	return [self configureWithSampleRate:sampleRate
	                       inputChannels:(inputEnabled ? numChannels : 0)
	                      outputChannels:numChannels
	                    bufferingEnabled:YES];
}

#pragma mark Util

- (void)print {
	if (!_initialized) {
		AU_LOG(@"audio unit not initialized");
		return;
	}
	AU_LOG(@"----- audio unit properties -----");

	UInt32 size = sizeof(AudioStreamBasicDescription);
	if (_inputEnabled) {
		AudioStreamBasicDescription inputStreamDescription = {0};
		AU_RETURN_IF_ERROR(AudioUnitGetProperty(_audioUnit,
		                   kAudioUnitProperty_StreamFormat,
		                   kAudioUnitScope_Output,
		                   kRemoteIOElement_Input,
		                   &inputStreamDescription,
		                   &size));
		AU_LOG(@"input stream:");
		AU_LOG(@"  sample rate: %g", inputStreamDescription.mSampleRate);
		AU_LOG(@"  channels: %d", inputStreamDescription.mChannelsPerFrame);
		AU_LOGV(@"  format ID: %lu", inputStreamDescription.mFormatID);
		AU_LOGV(@"  format flags: %lu", inputStreamDescription.mFormatFlags);
		AU_LOGV(@"  bytes per packet: %lu", inputStreamDescription.mBytesPerPacket);
		AU_LOGV(@"  frames per packet: %lu", inputStreamDescription.mFramesPerPacket);
		AU_LOGV(@"  bytes per frame: %lu", inputStreamDescription.mBytesPerFrame);
		AU_LOGV(@"  bits per channel: %lu", inputStreamDescription.mBitsPerChannel);
	}
	else {
		AU_LOG(@"input stream: none");
	}

	AudioStreamBasicDescription outputStreamDescription = {0};
	AU_RETURN_IF_ERROR(AudioUnitGetProperty(_audioUnit,
	                   kAudioUnitProperty_StreamFormat,
	                   kAudioUnitScope_Input,
	                   kRemoteIOElement_Output,
	                   &outputStreamDescription,
	                   &size));
	if (outputStreamDescription.mSampleRate > 0) {
		AU_LOG(@"output stream:");
		AU_LOG(@"  sample rate: %g", outputStreamDescription.mSampleRate);
		AU_LOG(@"  channels: %d", outputStreamDescription.mChannelsPerFrame);
		AU_LOGV(@"  format ID: %lu", outputStreamDescription.mFormatID);
		AU_LOGV(@"  format flags: %lu", outputStreamDescription.mFormatFlags);
		AU_LOGV(@"  bytes per packet: %lu", outputStreamDescription.mBytesPerPacket);
		AU_LOGV(@"  frames per packet: %lu", outputStreamDescription.mFramesPerPacket);
		AU_LOGV(@"  bytes per frame: %lu", outputStreamDescription.mBytesPerFrame);
		AU_LOGV(@"  bits per channel: %lu", outputStreamDescription.mBitsPerChannel);
	}
	else {
		AU_LOG(@"output stream: none");
	}
	AU_LOG(@"buffering: %@", (self.isBuffering ? @"yes" : @"no"));
}

// sets the format to 32 bit, floating point, linear PCM, interleaved
- (AudioStreamBasicDescription)ASBDForSampleRate:(Float64)sampleRate numberChannels:(UInt32)numberChannels {
	AudioStreamBasicDescription description = {0};
	description.mSampleRate = sampleRate;
	description.mFormatID = kAudioFormatLinearPCM;
	description.mFormatFlags = kAudioFormatFlagsNativeFloatPacked;
	description.mBytesPerPacket = numberChannels * sizeof(Float32);
	description.mFramesPerPacket = 1;
	description.mBytesPerFrame = numberChannels * sizeof(Float32);
	description.mChannelsPerFrame = numberChannels;
	description.mBitsPerChannel = sizeof(Float32) * 8;
	return description;
}

+ (AudioComponentDescription)defaultIODescription {
	AudioComponentDescription description;
	description.componentType = kAudioUnitType_Output;
	description.componentSubType = kAudioUnitSubType_RemoteIO;
	description.componentManufacturer = kAudioUnitManufacturer_Apple;
	description.componentFlags = 0;
	description.componentFlagsMask = 0;
	return description;
}

#pragma mark Overridden Getters/Setters

- (AURenderCallback)renderCallback {
	return audioRenderCallback;
}

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

- (BOOL)isBuffering {
	return (_inputRingBuffer || _outputRingBuffer);
}

#pragma mark AURenderCallback

// note: access ivars directly to avoid slower method calls in audio thread
static OSStatus audioRenderCallback(void *inRefCon,
                                    AudioUnitRenderActionFlags *ioActionFlags,
                                    const AudioTimeStamp *inTimeStamp,
                                    UInt32 inBusNumber,
                                    UInt32 inNumberFrames,
                                    AudioBufferList *ioData) {

	PdAudioUnit *pd = (__bridge PdAudioUnit *)inRefCon;
	Float32 *auBuffer = (Float32 *)ioData->mBuffers[0].mData;

	// buffer and process pd ticks one by one
	if (pd->_inputRingBuffer || pd->_outputRingBuffer) {

		// output buffer info from ioData
		UInt32 outputBufferSize = ioData->mBuffers[0].mDataByteSize;
		UInt32 outputFrames = inNumberFrames;
		UInt32 outputChannels = ioData->mBuffers[0].mNumberChannels;

		// input buffer info from ioData *after* rendering input samples
		UInt32 inputBufferSize = outputBufferSize;
		UInt32 inputFrames = inNumberFrames;
		UInt32 inputChannels = 0;

		// render input samples
		// note: input buffer *may* be different from output buffer due to sr conversion,
		//       so reset ioData info to original output values
		if (pd->_inputEnabled) {
			AudioUnitRender(pd->_audioUnit, ioActionFlags, inTimeStamp, kRemoteIOElement_Input, inNumberFrames, ioData);
			inputBufferSize = ioData->mBuffers[0].mDataByteSize;
			inputFrames = inputBufferSize / (pd->_inputFrameSize);
			inputChannels = ioData->mBuffers[0].mNumberChannels;
			ioData->mBuffers[0].mDataByteSize = outputBufferSize;
			ioData->mBuffers[0].mNumberChannels = outputChannels;
		}

		// audio unit -> input ring buffer
		UInt32 framesAvailable = (UInt32)rb_available_to_read(pd->_inputRingBuffer) / pd->_inputFrameSize;
		while (inputFrames + framesAvailable < outputFrames) {
			// pad input buffer to make sure we have enough blocks to fill auBuffer,
			// this should hopefully only happen when the audio unit is started
			rb_write_value_to_buffer(pd->_inputRingBuffer, 0, pd->_inputBlockSize);
			framesAvailable += pd->_blockFrames;
		}
		rb_write_to_buffer(pd->_inputRingBuffer, 1, auBuffer, inputBufferSize);

		// input ring buffer -> pd -> output ring buffer
		char *copy = (char *)auBuffer;
		while (rb_available_to_read(pd->_outputRingBuffer) < outputBufferSize) {
			rb_read_from_buffer(pd->_inputRingBuffer, copy, pd->_inputBlockSize);
			[PdBase processFloatWithInputBuffer:(float *)copy outputBuffer:(float *)copy ticks:1];
			rb_write_to_buffer(pd->_outputRingBuffer, 1, copy, pd->_outputBlockSize);
		}

		// output ring buffer -> audio unit
		rb_read_from_buffer(pd->_outputRingBuffer, (char *)auBuffer, outputBufferSize);
	}
	else { // straight process: audio unit -> pd -> audio unit

		// render input samples
		if (pd->_inputEnabled) {
			AudioUnitRender(pd->_audioUnit, ioActionFlags, inTimeStamp, kRemoteIOElement_Input, inNumberFrames, ioData);
		}

		// this is a faster way of computing (inNumberFrames / pd->_blockFrames)
		int ticks = inNumberFrames >> pd->_blockFramesAsLog;
		[PdBase processFloatWithInputBuffer:auBuffer outputBuffer:auBuffer ticks:ticks];
	}

	return noErr;
}

#pragma mark AudioUnitPropertyListener

static void propertyChangedCallback(void *inRefCon, AudioUnit inUnit, AudioUnitPropertyID inID,
                                    AudioUnitScope inScope, AudioUnitElement inElement) {
	PdAudioUnit *pd = (__bridge PdAudioUnit *)inRefCon;
	if (!pd->_initialized) {return;}
	if (inID == kAudioUnitProperty_MaximumFramesPerSlice) {
		// buffer size changed, probably due to new input or output route
		UInt32 frames, size = sizeof(frames);
		AU_RETURN_IF_ERROR(AudioUnitGetProperty(inUnit, inID, inScope, inElement, &frames, &size));
		if (frames != pd->_maxFrames) {
			AU_LOGV(@"max frames property changed: %d", frames);
			[pd updateInputChannels:pd->_inputChannels outputChannels:pd->_outputChannels];
		}
	}
	else if (inID == kAudioUnitProperty_SampleRate) {
		// audio unit sr has changed, so fall back in worst case
		Float64 sr = 0;
		UInt32 size = sizeof(sr);
		AU_RETURN_IF_ERROR(AudioUnitGetProperty(inUnit, inID, inScope, inElement, &sr, &size));
		if (!floatsAreEqual(pd->_sampleRate, sr)) {
			pd->_sampleRate = sr;
			AU_LOG(@"*** WARNING *** audio unit sample rate property changed: %g", sr);
			[pd updateInputChannels:pd->_inputChannels outputChannels:pd->_outputChannels];
			[PdBase openAudioWithSampleRate:sr
			                  inputChannels:pd->_inputChannels
			                 outputChannels:pd->_outputChannels];
		}
	}
	else if (inID == kAudioUnitProperty_StreamFormat) {
		// audio unit internal input or output channels have changed, so fall back in worst case
		AudioStreamBasicDescription streamFormat = {0};
		UInt32 size = sizeof(streamFormat);
		AU_RETURN_IF_ERROR(AudioUnitGetProperty(inUnit, inID, inScope, inElement, &streamFormat, &size));
		if (inElement == kRemoteIOElement_Input && inScope == kAudioUnitScope_Output) {
			if (pd->_inputChannels != streamFormat.mChannelsPerFrame) {
				AU_LOG(@"*** WARNING *** audio unit input channels changed: %d", streamFormat.mChannelsPerFrame);
				[pd updateInputChannels:streamFormat.mChannelsPerFrame outputChannels:pd->_outputChannels];
				[PdBase openAudioWithSampleRate:pd->_sampleRate
				                  inputChannels:streamFormat.mChannelsPerFrame
				                 outputChannels:pd->_outputChannels];

			}
		}
		else if (inElement == kRemoteIOElement_Output && inScope == kAudioUnitScope_Input) {
			if (pd->_outputChannels != streamFormat.mChannelsPerFrame) {
				AU_LOG(@"*** WARNING *** audio unit output channels changed: %d", streamFormat.mChannelsPerFrame);
				[pd updateInputChannels:pd->_inputChannels outputChannels:streamFormat.mChannelsPerFrame];
				[PdBase openAudioWithSampleRate:pd->_sampleRate
				                  inputChannels:pd->_inputChannels
				                 outputChannels:streamFormat.mChannelsPerFrame];

			}
		}
	}
}

#pragma mark Private

- (BOOL)initAudioUnitWithSampleRate:(Float64)sampleRate
                      inputChannels:(int)inputChannels
                     outputChannels:(int)outputChannels {
	[self clearAudioUnit];
	AudioComponentDescription ioDescription = PdAudioUnit.defaultIODescription;
	AudioComponent audioComponent = AudioComponentFindNext(NULL, &ioDescription);
	AU_RETURN_FALSE_IF_ERROR(AudioComponentInstanceNew(audioComponent, &_audioUnit));

	// RemoteIO internal input stream is disabled by default
	if (_inputEnabled && inputChannels > 0) {
		UInt32 enableInput = 1;
		AU_DISPOSE_FALSE_IF_ERROR(AudioUnitSetProperty(_audioUnit,
		                                               kAudioOutputUnitProperty_EnableIO,
		                                               kAudioUnitScope_Input,
		                                               kRemoteIOElement_Input,
		                                               &enableInput,
		                                               sizeof(enableInput)), _audioUnit);

		// output scope because we're defining the output of the input element _to_ our render callback
		AudioStreamBasicDescription streamDescription = {0};
		streamDescription = [self ASBDForSampleRate:sampleRate numberChannels:inputChannels];
		AU_DISPOSE_FALSE_IF_ERROR(AudioUnitSetProperty(_audioUnit,
		                                               kAudioUnitProperty_StreamFormat,
		                                               kAudioUnitScope_Output,
		                                               kRemoteIOElement_Input,
		                                               &streamDescription,
		                                               sizeof(streamDescription)), _audioUnit);
	}

	// RemoteIO internal output stream is enabled by default
	if (outputChannels > 0) {
		// input scope because we're defining the input of the output element _from_ our render callback
		AudioStreamBasicDescription streamDescription = {0};
		streamDescription = [self ASBDForSampleRate:sampleRate numberChannels:outputChannels];
		AU_DISPOSE_FALSE_IF_ERROR(AudioUnitSetProperty(_audioUnit,
		                                               kAudioUnitProperty_StreamFormat,
		                                               kAudioUnitScope_Input,
		                                               kRemoteIOElement_Output,
		                                               &streamDescription,
		                                               sizeof(streamDescription)), _audioUnit);
	}
	else {
		UInt32 enableOutput = 1;
		AU_DISPOSE_FALSE_IF_ERROR(AudioUnitSetProperty(_audioUnit,
		                                               kAudioOutputUnitProperty_EnableIO,
		                                               kAudioUnitScope_Output,
		                                               kRemoteIOElement_Output,
		                                               &enableOutput,
		                                               sizeof(enableOutput)), _audioUnit);
	}
	
	AURenderCallbackStruct callbackStruct;
	callbackStruct.inputProc = self.renderCallback;
	callbackStruct.inputProcRefCon = (__bridge void * _Nullable)(self);
	AU_DISPOSE_FALSE_IF_ERROR(AudioUnitSetProperty(_audioUnit,
	                                               kAudioUnitProperty_SetRenderCallback,
	                                               kAudioUnitScope_Input,
	                                               kRemoteIOElement_Output,
	                                               &callbackStruct,
	                                               sizeof(callbackStruct)), _audioUnit);

	AU_DISPOSE_FALSE_IF_ERROR(AudioUnitAddPropertyListener(_audioUnit,
	                                                       kAudioUnitProperty_MaximumFramesPerSlice,
	                                                       &propertyChangedCallback,
	                                                       (__bridge void *)self), _audioUnit);
	AU_DISPOSE_FALSE_IF_ERROR(AudioUnitAddPropertyListener(_audioUnit,
	                                                       kAudioUnitProperty_SampleRate,
	                                                       &propertyChangedCallback,
	                                                       (__bridge void *)self), _audioUnit);
	AU_DISPOSE_FALSE_IF_ERROR(AudioUnitAddPropertyListener(_audioUnit,
	                                                       kAudioUnitProperty_StreamFormat,
	                                                       &propertyChangedCallback,
	                                                       (__bridge void *)self), _audioUnit);

	AU_DISPOSE_FALSE_IF_ERROR(AudioUnitInitialize(_audioUnit), _audioUnit);
	_initialized = YES;
	AU_LOGV(@"initialized audio unit: sr %g inputs %d outputs %d", sampleRate, inputChannels, outputChannels);

	return YES;
}

- (void)clearAudioUnit {
	if (!_initialized) {
		return;
	}
	self.active = NO;
	_initialized = NO;
	AU_DISPOSE_IF_ERROR(AudioUnitUninitialize(_audioUnit), _audioUnit);
	AU_DISPOSE_IF_ERROR(AudioUnitRemovePropertyListenerWithUserData(_audioUnit,
	                                                                kAudioUnitProperty_MaximumFramesPerSlice,
	                                                                &propertyChangedCallback,
	                                                                (__bridge void *)self), _audioUnit);
	AU_DISPOSE_IF_ERROR(AudioUnitRemovePropertyListenerWithUserData(_audioUnit,
	                                                                kAudioUnitProperty_SampleRate,
	                                                                &propertyChangedCallback,
	                                                                (__bridge void *)self), _audioUnit);
	AU_DISPOSE_IF_ERROR(AudioUnitRemovePropertyListenerWithUserData(_audioUnit,
	                                                                kAudioUnitProperty_StreamFormat,
	                                                                &propertyChangedCallback,
	                                                                (__bridge void *)self), _audioUnit);

	AU_RETURN_IF_ERROR(AudioComponentInstanceDispose(_audioUnit));
	_audioUnit = nil;
	AU_LOGV(@"cleared audio unit");
}

- (BOOL)updateInputChannels:(int)inputChannels outputChannels:(int)outputChannels {
	BOOL reallocate = (_inputChannels != inputChannels || _outputChannels != outputChannels);
	int inputFrameSize = inputChannels * sizeof(Float32);
	int outputFrameSize = outputChannels * sizeof(Float32);

	_inputChannels = inputChannels;
	_outputChannels = outputChannels;
	_inputBlockSize = inputFrameSize * _blockFrames;
	_outputBlockSize = outputFrameSize * _blockFrames;
	_inputFrameSize = inputFrameSize;

	UInt32 prevMaxFrames = _maxFrames;
	UInt32 size = sizeof(_maxFrames);
	AU_RETURN_FALSE_IF_ERROR(AudioUnitGetProperty(_audioUnit,
	                                              kAudioUnitProperty_MaximumFramesPerSlice,
	                                              kAudioUnitScope_Global, kRemoteIOElement_Output,
	                                              &_maxFrames,
	                                              &size));
	reallocate = reallocate || (prevMaxFrames != _maxFrames);

	if (_bufferingEnabled) {
		if (reallocate) {
			[self clearBuffers];
			// pad with one extra block
			// note: ring buffer sizes *must* be multiple of 256!
			_inputRingBuffer = rb_create(roundup(inputFrameSize * (_maxFrames + _blockFrames), 256));
			_outputRingBuffer = rb_create(roundup(outputFrameSize * (_maxFrames + _blockFrames), 256));
			AU_LOGV(@"initialized buffers: inputs %d outputs %d max frames %d",
			        inputChannels, outputChannels, _maxFrames);
		}
	}
	else {
		[self clearBuffers];
	}

	return YES;
}

- (void)clearBuffers {
	if (_inputRingBuffer) {
		rb_free(_inputRingBuffer);
		_inputRingBuffer = NULL;
	}
	if (_outputRingBuffer) {
		rb_free(_outputRingBuffer);
		_outputRingBuffer = NULL;
		AU_LOGV(@"cleared buffers");
	}
}

@end
