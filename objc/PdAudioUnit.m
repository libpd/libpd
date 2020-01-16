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
#import <AVFoundation/AVFoundation.h>
#include "ringbuffer.h"

// default max io buffer size in frames, as outlined in
// https://developer.apple.com/library/content/qa/qa1606/_index.html
static const int kAUDefaultMaxFrames = 4096;

static const AudioUnitElement kAUInputElement = 1;
static const AudioUnitElement kAUOutputElement = 0;

@interface PdAudioUnit ()

/// create and start the audio unit
- (BOOL)initAudioUnitWithSampleRate:(Float64)sampleRate
                      inputChannels:(int)inputChannels
                     outputChannels:(int)outputChannels;

/// stop and release the audio unit
- (void)clearAudioUnit;

/// allocate buffers if there is sr conversion between audio unit and session,
/// also sets buffer size & frame ivars
- (BOOL)initBuffersWithInputChannels:(int)inputChannels
                      outputChannels:(int)outputChannels;

/// release buffers, if allocated
- (void)clearBuffers;

/// create default RemoteIO audio unit description
+ (AudioComponentDescription)defaultIODescription;

@end

@implementation PdAudioUnit {
	ring_buffer *_inputRingBuffer;  ///< input buffer
	ring_buffer *_outputRingBuffer; ///< output buffer
}

@synthesize sampleRate = _sampleRate;
@synthesize inputEnabled = _inputEnabled;
@synthesize inputChannels = _inputChannels;
@synthesize outputChannels = _outputChannels;

- (instancetype)init {
	self = [super init];
	if (self) {
		_initialized = NO;
		_active = NO;
		_blockSizeAsLog = log2int(PdBase.getBlockSize);
		_maxFrames = kAUDefaultMaxFrames;
	}
	return self;
}

- (void)dealloc {
	[self clearAudioUnit];
	[self clearBuffers];
}

- (int)configureWithSampleRate:(Float64)sampleRate
                 inputChannels:(int)inputChannels
                outputChannels:(int)outputChannels {
	BOOL wasActive = self.isActive;
	_sampleRate = sampleRate;
	_inputEnabled = (inputChannels > 0);
	_inputChannels = inputChannels;
	_outputChannels = outputChannels;
	if (![self initAudioUnitWithSampleRate:_sampleRate
	                         inputChannels:inputChannels
	                        outputChannels:outputChannels]) {
		return -1;
	}
	if (![self initBuffersWithInputChannels:_inputChannels
	                         outputChannels:_outputChannels]) {
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
	                      outputChannels:numChannels];
}

- (void)print {
	if (!_initialized) {
		AU_LOG(@"audio unit not initialized");
		return;
	}
	AU_LOG(@"----- audio unit properties -----");

	UInt32 size = sizeof(AudioStreamBasicDescription);
	if (_inputEnabled) {
		AudioStreamBasicDescription inputStreamDescription;
		memset (&inputStreamDescription, 0, sizeof(inputStreamDescription));
		AU_RETURN_IF_ERROR(AudioUnitGetProperty(_audioUnit,
		                   kAudioUnitProperty_StreamFormat,
		                   kAudioUnitScope_Output,
		                   kAUInputElement,
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

	AudioStreamBasicDescription outputStreamDescription;
	memset(&outputStreamDescription, 0, size);
	AU_RETURN_IF_ERROR(AudioUnitGetProperty(_audioUnit,
	                   kAudioUnitProperty_StreamFormat,
	                   kAudioUnitScope_Input,
	                   kAUOutputElement,
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
}

// sets the format to 32 bit, floating point, linear PCM, interleaved
- (AudioStreamBasicDescription)ASBDForSampleRate:(Float64)sampleRate numberChannels:(UInt32)numberChannels {
	AudioStreamBasicDescription description;
	memset(&description, 0, sizeof(description));
	
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

#pragma mark AURenderCallback

// note: access ivars directly to avoid slower method calls in audio thread
static OSStatus audioRenderCallback(void *inRefCon,
                                    AudioUnitRenderActionFlags *ioActionFlags,
                                    const AudioTimeStamp *inTimeStamp,
                                    UInt32 inBusNumber,
                                    UInt32 inNumberFrames,
                                    AudioBufferList *ioData) {

	PdAudioUnit *pdAudioUnit = (__bridge PdAudioUnit *)inRefCon;
	Float32 *auBuffer = (Float32 *)ioData->mBuffers[0].mData;
	UInt32 auBufferSize = ioData->mBuffers[0].mDataByteSize;

	// this is a faster way of computing (inNumberFrames / blockSize)
	int ticks = inNumberFrames >> pdAudioUnit->_blockSizeAsLog;

	// render input samples
	if (pdAudioUnit->_inputEnabled) {
		AudioUnitRender(pdAudioUnit->_audioUnit, ioActionFlags, inTimeStamp, kAUInputElement, inNumberFrames, ioData);
	}

	// buffer and process pd ticks one by one
	if (pdAudioUnit->_inputRingBuffer || pdAudioUnit->_outputRingBuffer) {

		// audio unit -> input ring buffer
		rb_write_to_buffer(pdAudioUnit->_inputRingBuffer, 1, auBuffer, auBufferSize);

		// input ring buffer -> pd -> output ring buffer
		char *copy = (char *)auBuffer;
		while (rb_available_to_read(pdAudioUnit->_outputRingBuffer) < auBufferSize) {
			rb_read_from_buffer(pdAudioUnit->_inputRingBuffer, copy, pdAudioUnit->_inputBlockSize);
			[PdBase processFloatWithInputBuffer:(float *)copy outputBuffer:(float *)copy ticks:1];
			rb_write_to_buffer(pdAudioUnit->_outputRingBuffer, 1, copy, pdAudioUnit->_outputBlockSize);
		}

		// output ring buffer -> audio unit
		rb_read_from_buffer(pdAudioUnit->_outputRingBuffer, (char *)auBuffer, auBufferSize);
	}
	else { // straight process: audio unit -> pd -> audio unit
		[PdBase processFloatWithInputBuffer:auBuffer outputBuffer:auBuffer ticks:ticks];
	}

	return noErr;
}

#pragma mark AudioUnitPropertyListener

// reinit buffers if frame size changes
static void propertyChangedCallback(void *inRefCon, AudioUnit inUnit, AudioUnitPropertyID inID,
                                    AudioUnitScope inScope, AudioUnitElement inElement) {
	PdAudioUnit *pdAudioUnit = (__bridge PdAudioUnit *)inRefCon;
	if (inID == kAudioUnitProperty_MaximumFramesPerSlice) {
		UInt32 frames, size = sizeof(frames);
		AudioUnitGetProperty(inUnit, inID, inScope, inElement, &frames, &size);
		if (frames != pdAudioUnit->_maxFrames) {
			pdAudioUnit->_maxFrames = frames;
			AU_LOGV(@"max frames property changed: %d", frames);
			[pdAudioUnit initBuffersWithInputChannels:pdAudioUnit->_inputChannels
			                           outputChannels:pdAudioUnit->_outputChannels];
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

	AudioStreamBasicDescription streamDescription;
	if (_inputEnabled && inputChannels > 0) {
		UInt32 enableInput = 1;
		AU_RETURN_FALSE_IF_ERROR(AudioUnitSetProperty(_audioUnit,
		                                              kAudioOutputUnitProperty_EnableIO,
		                                              kAudioUnitScope_Input,
		                                              kAUInputElement,
		                                              &enableInput,
		                                              sizeof(enableInput)));

		// output scope because we're defining the output of the input element _to_ our render callback
		streamDescription = [self ASBDForSampleRate:sampleRate numberChannels:inputChannels];
		AU_RETURN_FALSE_IF_ERROR(AudioUnitSetProperty(_audioUnit,
		                                              kAudioUnitProperty_StreamFormat,
		                                              kAudioUnitScope_Output,
		                                              kAUInputElement,
		                                              &streamDescription,
		                                              sizeof(streamDescription)));
	}

	// input scope because we're defining the input of the output element _from_ our render callback
	if (outputChannels > 0) {
		streamDescription = [self ASBDForSampleRate:sampleRate numberChannels:outputChannels];
		AU_RETURN_FALSE_IF_ERROR(AudioUnitSetProperty(_audioUnit,
		                                              kAudioUnitProperty_StreamFormat,
		                                              kAudioUnitScope_Input,
		                                              kAUOutputElement,
		                                              &streamDescription,
		                                              sizeof(streamDescription)));
	}
	
	AURenderCallbackStruct callbackStruct;
	callbackStruct.inputProc = self.renderCallback;
	callbackStruct.inputProcRefCon = (__bridge void * _Nullable)(self);
	AU_RETURN_FALSE_IF_ERROR(AudioUnitSetProperty(_audioUnit,
	                                              kAudioUnitProperty_SetRenderCallback,
	                                              kAudioUnitScope_Input,
	                                              kAUOutputElement,
	                                              &callbackStruct,
	                                              sizeof(callbackStruct)));

	AU_RETURN_FALSE_IF_ERROR(AudioUnitAddPropertyListener(_audioUnit,
	                                                      kAudioUnitProperty_MaximumFramesPerSlice,
	                                                      &propertyChangedCallback,
	                                                      (__bridge void *)self));

	AU_RETURN_FALSE_IF_ERROR(AudioUnitInitialize(_audioUnit));
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
	AU_RETURN_IF_ERROR(AudioUnitUninitialize(_audioUnit));
	AU_RETURN_IF_ERROR(AudioUnitRemovePropertyListenerWithUserData(_audioUnit,
	                                                               kAudioUnitProperty_MaximumFramesPerSlice,
	                                                               &propertyChangedCallback,
	                                                               (__bridge void *)self));
	AU_RETURN_IF_ERROR(AudioComponentInstanceDispose(_audioUnit));
	AU_LOGV(@"cleared audio unit");
}

- (BOOL)initBuffersWithInputChannels:(int)inputChannels outputChannels:(int)outputChannels {
	int inputFrameSize = inputChannels * sizeof(Float32);
	int outputFrameSize = outputChannels * sizeof(Float32);

	[self clearBuffers];
	_inputChannels = inputChannels;
	_outputChannels = outputChannels;
	_inputBlockSize = inputFrameSize * PdBase.getBlockSize;
	_outputBlockSize = outputFrameSize * PdBase.getBlockSize;

	UInt32 size = sizeof(_maxFrames);
	AU_RETURN_FALSE_IF_ERROR(AudioUnitGetProperty(_audioUnit,
	                                              kAudioUnitProperty_MaximumFramesPerSlice,
	                                              kAudioUnitScope_Global, kAUOutputElement,
	                                              &_maxFrames,
	                                              &size));

	if (_sampleRate != AVAudioSession.sharedInstance.sampleRate) {
		AU_LOGV(@"sample rate conversion: pd %g session %g",
		        self.sampleRate, AVAudioSession.sharedInstance.sampleRate);
		_inputRingBuffer = rb_create(inputFrameSize * _maxFrames);
		_outputRingBuffer = rb_create(outputFrameSize * _maxFrames);
		AU_LOGV(@"initialized buffers: inputs %d outputs %d max frames %d",
		        inputChannels, outputChannels, _maxFrames);
	}
	else {
		AU_LOGV(@"sample rate conversion: not needed");
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
	}
	AU_LOGV(@"cleared buffers");
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

@end
