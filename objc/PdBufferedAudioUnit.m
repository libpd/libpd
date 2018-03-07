//
//  PdBufferedAudioUnit.m
//  libpd
//
//  Created on 02/09/18.
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//

#import "PdBufferedAudioUnit.h"
#import "PdBase.h"
#import "AudioHelpers.h"

// max AU io buffer duration as outlined in
// https://developer.apple.com/library/content/qa/qa1606/_index.html
static const int kMaxIOBufferDuration = 4096;

static const AudioUnitElement kInputElement = 1;

@implementation PdBufferedAudioUnit

#pragma mark - Init / Dealloc

- (instancetype)init {
	self = [super init];
	if (self) {
		_blockSize = 0;
		_blockSizeAsLog = 0;
		_inputBuffer = NULL;
		_outputBuffer = NULL;
		_copyBuffer = NULL;
	}
	return self;
}

- (void)dealloc {
	if(_inputBuffer)  {rb_free(_inputBuffer);}
	if(_outputBuffer) {rb_free(_outputBuffer);}
	if(_copyBuffer)   {free(_copyBuffer);}
}

#pragma mark - Public Methods

- (int)configureWithSampleRate:(Float64)sampleRate
				numberChannels:(int)numChannels
                  inputEnabled:(BOOL)inputEnabled {
	int ret = [super configureWithSampleRate:sampleRate
	                          numberChannels:numChannels
	                            inputEnabled:inputEnabled];
	if (ret == 0) {
		_blockSize = [PdBase getBlockSize] * numChannels * sizeof(Float32);
		_blockSizeAsLog = log2int(_blockSize);
		if(_inputBuffer) {
			rb_free(_inputBuffer);
			_inputBuffer = NULL;
		}
		if(_outputBuffer) {
			rb_free(_outputBuffer);
			_outputBuffer = NULL;
		}
		if(_copyBuffer) {
			free(_copyBuffer);
			_copyBuffer = NULL;
		}
		int bufferLen = numChannels * kMaxIOBufferDuration * sizeof(Float32);
		if(_inputEnabled) {
			_inputBuffer = rb_create(bufferLen);
			_inputBuffer->is_atomic = 0;
		}
		_outputBuffer = rb_create(bufferLen);
		_outputBuffer->is_atomic = 0;
		_copyBuffer = malloc(_blockSize);
	}
	return ret;
}

#pragma mark - AURenderCallback

static OSStatus AudioRenderCallback(void *inRefCon,
                                    AudioUnitRenderActionFlags *ioActionFlags,
                                    const AudioTimeStamp *inTimeStamp,
                                    UInt32 inBusNumber,
                                    UInt32 inNumberFrames,
                                    AudioBufferList *ioData) {

	PdBufferedAudioUnit *pdAudioUnit = (__bridge PdBufferedAudioUnit *)inRefCon;
	Float32 *auBuffer = (Float32 *)ioData->mBuffers[0].mData;
	int auBufferLen = ioData->mBuffers[0].mDataByteSize;

	// this is a faster way of computing (auBufferLen / blockLen)
	int ticks = auBufferLen >> pdAudioUnit->_blockSizeAsLog;
	int bytes = ticks * pdAudioUnit->_blockSize;
	ring_buffer *input = pdAudioUnit->_inputBuffer;
	ring_buffer *output = pdAudioUnit->_outputBuffer;
	if (bytes == auBufferLen) {
		// auBufferLen is a multiple of pd's block size, no need to use buffers
		if (pdAudioUnit->_inputEnabled) {
			AudioUnitRender(pdAudioUnit->_audioUnit, ioActionFlags, inTimeStamp,
			                kInputElement, inNumberFrames, ioData);
		}

		// audio unit -> pd -> audio unit
		[PdBase processFloatWithInputBuffer:auBuffer outputBuffer:auBuffer ticks:ticks];

		// clear buffers in case auBufferLen changes and
		// buffering needs to be restarted
		if(rb_available_to_read(input) > 0)  {rb_clear_buffer(input);}
		if(rb_available_to_read(output) > 0) {rb_clear_buffer(output);}
	}
	else {
		// auBufferLen is not a multiple of pd's block size, use FIFO buffers to
		// make sure we have enough samples to render each callback at the expense
		// of being 1-2 pd blocks behind
		// TODO: reduce amount of buffer copying
		char *copy = pdAudioUnit->_copyBuffer;
		if (pdAudioUnit->_inputEnabled) {
			AudioUnitRender(pdAudioUnit->_audioUnit, ioActionFlags, inTimeStamp,
							kInputElement, inNumberFrames, ioData);

			// audio unit -> input buffer
			while (bytes + rb_available_to_read(input) < auBufferLen) {
				// pad input buffer to make sure we have enough blocks to fill auBuffer,
				// this should hopefully only happen when the audio unit is started
				rb_write_value_to_buffer(input, 0, pdAudioUnit->_blockSize);
			}
			rb_write_to_buffer(input, 1, (char *)auBuffer, auBufferLen);

			// input buffer -> pd -> output buffer
			while (rb_available_to_read(output) < auBufferLen) {
				rb_read_from_buffer(input, copy, pdAudioUnit->_blockSize);
				[PdBase processFloatWithInputBuffer:(float *)copy outputBuffer:(float *)copy ticks:1];
				rb_write_to_buffer(output, 1, copy, pdAudioUnit->_blockSize);
			}
		}
		else {
			// pd -> output buffer
			while (rb_available_to_read(output) < auBufferLen) {
				[PdBase processFloatWithInputBuffer:auBuffer outputBuffer:(float *)copy ticks:1];
				rb_write_to_buffer(output, 1, copy, pdAudioUnit->_blockSize);
			}
		}

		// output buffer -> audio unit
		rb_read_from_buffer(output, (char *)auBuffer, auBufferLen);
	}

	return noErr;
}

- (AURenderCallback)renderCallback {
	return AudioRenderCallback;
}

@end
