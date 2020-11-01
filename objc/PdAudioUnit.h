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

/// PdAudioUnit: operates pd's audio input and output through an AudioUnit
///
/// parameters can be changed after instantiation with a configure method which
/// will cause the underlying audio unit to be reconstructed
///
/// the internal pd sample rate may be different then that of the actual audio
/// session and the audio unit attempts to set up sample rate conversion and
/// buffering automatically
///
/// as of libpd 0.12 this is bridged to AudioUnit v3 which should allow for
/// adding a *single* instance of PdAudioUnit to an internal AUGraph, multi
/// instance support is not yet finished for this to work as a separate plugin
/// inside of an external host
///
/// for debugging, AU_DEBUG_VERBOSE can be defined to print extra information
@interface PdAudioUnit : AUAudioUnitV2Bridge {
@protected
	AudioUnit _audioUnit;     ///< the underlying audio unit instance
	BOOL _initialized;        ///< has the audio unit been successfully inited?

	// these are precomputed for use in the render callback
	UInt32 _blockFrames;      ///< pd block size in frames
	UInt32 _blockFramesAsLog; ///< log(blockFrames) for fast bitshift division
	UInt32 _inputFrameSize;   ///< pd input frame size in bytes
	UInt32 _inputBlockSize;   ///< pd input block size in bytes
	UInt32 _outputBlockSize;  ///< pd output block size in bytes
	UInt32 _maxFrames;        ///< max buffer size in frames

	// underlying property ivars for subclass access
	Float64 _sampleRate;
	BOOL _inputEnabled;
	int _inputChannels;
	int _outputChannels;
}

/// underlying audio unit which can be used to query or set other properties
@property (nonatomic, readonly) AudioUnit audioUnit;

/// audio unit callback function reference, override the getter method if you
/// want to subclass PdAudioUnit and implement your own custom sample rendering
///
/// in your custom PdAudioUnit subclass .m:
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

#pragma mark Read Only Configuration Properties

// read only properties set by the configure methods

/// sample rate, may not match the current session sample rate
@property (nonatomic, assign, readonly) Float64 sampleRate;

/// number of input channels, may not match number of session inputs
@property (nonatomic, assign, readonly) int inputChannels;

/// number of output channels, may not match number of session outputs
@property (nonatomic, assign, readonly) int outputChannels;

/// is the audio input stream enabled?
@property (nonatomic, assign, readonly) BOOL inputEnabled;

/// is the audio unit buffering samples between input and output?
/// may be required to handle variable buffer sizes due to sample rate
/// conversion (44.1k : 48k) or very long buffer sizes (8k : 48k)
@property (nonatomic, assign, readonly) BOOL bufferingEnabled;

#pragma mark Initialization

/// creates a default instance of PdAudioUnit.
+ (instancetype)defaultAudioUnit;

/// this is the designated init for the AudioUnit v2 to v3 bridge
///
/// to manually create a default instance, use:
/// * description: PdAudioUnit.defaultIODescription
/// * options: 0
/// * outError: nil
- (instancetype)initWithComponentDescription:(AudioComponentDescription)componentDescription
                                     options:(AudioComponentInstantiationOptions)options
                                       error:(NSError **)outError;

#pragma mark Configuration

/// configure audio unit with preferred sample rate and number of input and
/// output channels
///
/// if inputChannels is 0, the input will be disabled
/// if outputChannels is 0, the output will be disabled
///
/// buffering is enabled by default
///
/// note: this is an expensive process and will stop the audio unit before any
/// reconstruction, causing a momentary pause in audio and UI if run from the
/// main thread
///
/// returns zero on success, ie. OSStatus noErr
- (int)configureWithSampleRate:(Float64)sampleRate
                 inputChannels:(int)inputChannels
                outputChannels:(int)outputChannels;

/// configure audio unit with preferred sample rate, number of input and
/// output channels, and whether to enable buffering
///
/// if inputChannels is 0, the input will be disabled
/// if outputChannels is 0, the output will be disabled
///
/// note: this is an expensive process and will stop the audio unit before any
/// reconstruction, causing a momentary pause in audio and UI if run from the
/// main thread
///
/// returns zero on success, ie. OSStatus noErr
- (int)configureWithSampleRate:(Float64)sampleRate
                 inputChannels:(int)inputChannels
                outputChannels:(int)outputChannels
              bufferingEnabled:(BOOL)bufferingEnabled;

/// note: legacy method kept for compatibility
///
/// configure audio unit with preferred sample rate, number of channels, and
/// whether to enable the input stream
///
/// buffering is enabled by default
///
/// note: this is an expensive process and will stop the audio unit before any
/// reconstruction, causing a momentary pause in audio and UI if run from the
/// main thread
///
/// returns zero on success, ie. OSStatus noErr
- (int)configureWithSampleRate:(Float64)sampleRate
                numberChannels:(int)numChannels
                  inputEnabled:(BOOL)inputEnabled;

#pragma mark Util

/// print info on the audio unit settings to the console
- (void)print;

/// called by configureWithSampleRate when setting up the internal audio unit
/// default format: 32 bit, floating point, linear PCM, interleaved
- (AudioStreamBasicDescription)ASBDForSampleRate:(Float64)sampleRate
                                  numberChannels:(UInt32)numChannels;

/// create default RemoteIO audio unit description
+ (AudioComponentDescription)defaultIODescription;

@end
