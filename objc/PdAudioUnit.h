//
//  PdAudioUnit.h
//  libpd
//
//  Created on 29/09/11.
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//
//  Updated 2018, 2020 Dan Wilcox <danomatika@gmail.com>
//

#import <Foundation/Foundation.h>
#import "AudioUnit/AudioUnit.h"

/// PdAudioUnit: object that operates pd's audio input and output through an
/// Audio Unit. The parameters can be changed after it has been instatiated with
/// a configure method, which will cause the underlying audio unit to be
/// reconstructed. The internal pd sample rate may be different then that of the
/// actual audio session and the audio unit attempts to set up sample rate
/// conversion and buffering automatically.
///
/// For debugging, AU_DEBUG_VERBOSE can be defined to print extra information.
@interface PdAudioUnit : NSObject {
@protected
	AudioUnit _audioUnit;    ///< the underlying audio unit instance
	BOOL _initialized;       ///< has the audio unit been successfully inited?

	// these are precomputed for use in the render callback
	UInt32 _blockSizeAsLog;  ///< log(blockSize) for fast bitshift division
	UInt32 _inputBlockSize;  ///< pd input block size in bytes
	UInt32 _outputBlockSize; ///< pd output block size in bytes
	UInt32 _maxFrames;       ///< max buffer size in frames

	// underlying property ivars for subclass access
	Float64 _sampleRate;
	BOOL _inputEnabled;
	int _inputChannels;
	int _outputChannels;
}

/// A reference to the audio unit which can be used to query or set other
/// properties.
@property (nonatomic, readonly) AudioUnit audioUnit;

/// A reference to the audio unit callback function. Override the getter method
/// if you want to subclass PdAudioUnit and implement your own custom sample
/// rendering.
///
/// In your custom PdAudioUnit subclass .m:
///
/// // your custom render callback
/// static OSStatus myRenderCallback(void *inRefCon,
///                                  AudioUnitRenderActionFlags *ioActionFlags,
///                                  const AudioTimeStamp *inTimeStamp,
///                                  UInt32 inBusNumber,
///                                  UInt32 inNumberFrames,
///                                  AudioBufferList *ioData) {
///     // do something with the samples, see PdAudioUnit.m audioRenderCallback
///     return noErr;
/// }
///
/// // return method
///   (AURenderCallback)renderCallback {
///     return myRenderCallback;
/// }
///
@property (nonatomic, readonly) AURenderCallback renderCallback;

/// is the audio unit active?
@property (nonatomic, getter=isActive) BOOL active;

// Read only properties that are set by the configure methods

/// sample rate, buffering is performed if this does not match the session rate
@property (nonatomic, assign, readonly) Float64 sampleRate;

/// number of input channels, may not match number of session inputs
@property (nonatomic, assign, readonly) int inputChannels;

/// number of output channels, may not match number of session outputs
@property (nonatomic, assign, readonly) int outputChannels;

/// is the audio input stream enabled?
@property (nonatomic, assign, readonly) BOOL inputEnabled;

/// Configure audio unit with preferred sample rate and number of input and
/// output channels. This is an expensive process and will stop the audio unit
/// before any reconstruction, causing a momentary pause in audio and UI if run
/// from the main thread.
/// Returns zero on success, ie. OSStatus noErr.
- (int)configureWithSampleRate:(Float64)sampleRate
                 inputChannels:(int)inputChannels
                outputChannels:(int)outputChannels;

/// Note: legacy method kept for compatibility
///
/// Configure audio unit with preferred sample rate, number of channels, and
/// whether to enable the input stream. This is an expensive process and will
/// stop the audio unit before any reconstruction, causing a momentary pause in
/// audio and UI if run from the main thread.
/// Returns zero on success, ie. OSStatus noErr.
- (int)configureWithSampleRate:(Float64)sampleRate
                numberChannels:(int)numChannels
                  inputEnabled:(BOOL)inputEnabled;

/// Print info on the audio unit settings to the console
- (void)print;

/// Called by configureWithSampleRate when setting up the internal audio unit.
/// Default format: 32 bit, floating point, linear PCM, interleaved
- (AudioStreamBasicDescription)ASBDForSampleRate:(Float64)sampleRate
                                  numberChannels:(UInt32)numChannels;

@end
