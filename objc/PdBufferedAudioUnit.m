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
	}
	return self;
}

- (void)dealloc {
	if(_inputBuffer) {
		rb_free(_inputBuffer);
	}
	if(_outputBuffer) {
		rb_free(_outputBuffer);
	}
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
		int bufferLen = numChannels * kMaxIOBufferDuration * sizeof(Float32);
		if(_inputEnabled) {
			_inputBuffer = rb_create(bufferLen);
			rb_set_atomic(_inputBuffer, 0);
		}
		_outputBuffer = rb_create(bufferLen);
		rb_set_atomic(_outputBuffer, 0);
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

	if (pdAudioUnit->_inputEnabled) {
		// audio unit -> input buffer
		AudioUnitRender(pdAudioUnit->_audioUnit, ioActionFlags, inTimeStamp,
			            kInputElement, inNumberFrames, ioData);
		rb_write_to_buffer(pdAudioUnit->_inputBuffer, 1, auBuffer, auBufferLen);
		rb_read_from_buffer(pdAudioUnit->_inputBuffer, (char *)auBuffer, bytes);
	}

	// input buffer -> pd -> output buffer
	[PdBase processFloatWithInputBuffer:auBuffer outputBuffer:auBuffer ticks:ticks];
	rb_write_to_buffer(pdAudioUnit->_outputBuffer, 1, auBuffer, bytes);

	// output buffer -> audio unit
	rb_read_from_buffer(pdAudioUnit->_outputBuffer, (char *)auBuffer, bytes);
	if (auBufferLen - bytes > 0) {
		// underflow: pd couldn't keep up so generate 0s
		auBuffer += bytes;
		memset(auBuffer, 0, auBufferLen - bytes);
	}

	return noErr;
}

- (AURenderCallback)renderCallback {
	return AudioRenderCallback;
}

@end
