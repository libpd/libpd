//
//  PdAudioController.h
//  libpd
//
//  Created on 11/10/11.
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//
//  Updated 2018, 2020 Dan Wilcox <danomatika@gmail.com>
//

#import "PdAudioUnit.h"
#import <AVFoundation/AVFoundation.h>

/// PdAudioStatus is used to indicate success, failure, or that parameters had
/// to be adjusted in order to work.
typedef enum PdAudioStatus {
	PdAudioOK = 0,             // success
	PdAudioError = -1,         // unrecoverable error
	PdAudioPropertyChanged = 1 // some properties have changed to run correctly
} PdAudioStatus;

/// PdAudioController: A class for managing a PdAudioUnit instance within iOS
/// by using the AVFoundation and Audio Services APIs. Handles phone
/// interruptions and provides high level configuration methods by wrapping
/// relevant AVAudioSession methods.
@interface PdAudioController : NSObject

/// check or set the active status of the audio unit
@property (nonatomic, getter=isActive) BOOL active;

// Read only properties set by the configure methods

/// desired sample rate, may be different from session rate
@property (nonatomic, readonly) int sampleRate;

/// number of input channels, may not match number of session inputs
@property (nonatomic, readonly) int inputChannels;

/// number of output channels, may not match number of session inputs
@property (nonatomic, readonly) int outputChannels;

/// is the audio input stream enabled?
@property (nonatomic, readonly) BOOL inputEnabled;

/// number of pd ticks per buffer size, computed from session buffer duration
@property (nonatomic, readonly) int ticksPerBuffer;

/// read only access to the underlying pd audio unit
@property (nonatomic, strong, readonly) PdAudioUnit *audioUnit;

// Audio session category options applied by the configure methods and while
// active, see Apple docs for category and category options info

/// is audio mixing with other apps enabled? (default YES)
/// applied to categories: PlayAndRecord, Playback, MultiRoute
@property (nonatomic, assign) BOOL mixWithOthers;

/// duck (ie. lower) audio output from other apps while active?
/// applied to categories: Ambient, PlayAndRecord, Playback, MultiRoute
@property (nonatomic, assign) BOOL duckOthers;

/// interrupt another app in AVAudioSessionModeSpokenAudio mode while active?
/// applied to categories: PlayAndRecord, Playback, MultiRoute
@property (nonatomic, assign) BOOL interruptSpokenAudioAndMixWithOthers;

/// output to speaker instead of receiver (earpiece)?  (default YES)
/// applied to categories: PlayAndRecord
@property (nonatomic, assign) BOOL defaultToSpeaker;

/// use Bluetooth HFP (Hands-Free Profile)?
/// note: this is *older* 8k samplerate 1 channel IO, ie. headset/earpiece
/// note: this may override allowBluetoothA2DP if both are set
/// applied to categories: Record, PlayAndRecord
@property (nonatomic, assign) BOOL allowBluetooth;

/// use Bluetooth A2DP (Advanced Audio Distribution Profile)?
/// note: this is higher-quality and stereo, ie. jambox/headphones/earbuds
/// note: this may be overridden by allowBluetooth if both are set
/// applied to categories: PlayAndRecord,
/// always supported for output-only categories: Playback, Ambient, SoloAmbient
@property (nonatomic, assign) BOOL allowBluetoothA2DP;

/// use AirPlay?
/// applied to categories: PlayAndRecord,
/// always supported for output-only categories: Playback, Ambient, SoloAmbient
@property (nonatomic, assign) BOOL allowAirPlay;

// Other options applied only during configuration.

/// prefer stereo over mono input/output (default YES)
///
/// ensures a minimum of stereo IO as some routes (mono mic -> built-in
/// speaker) don't seem to like mismatched sessions (ie. 1 input and 2 outputs),
/// this also seems to enable automatic mixdown to mono for some outputs
@property (nonatomic, assign) BOOL preferStereo;

/// ignore audio session route changes (default NO)
///
/// by default, the audio controller will reconfigure the audio unit whenever
/// an IO device is changes, use this to override if you have your own custom
/// route change handling
@property (nonatomic, assign) BOOL ignoreRouteChanges;

/// Init with default pd audio unit.
- (instancetype)init;

/// Init with a custom pd audio unit.
///
/// Derive PdAudioUnit when you need to access to the raw samples when using,
/// for instance, AudioBus, and call this method after init.
- (instancetype)initWithAudioUnit:(PdAudioUnit *)audioUnit;

/// Configure audio with the specified samplerate, as well as number of input
/// and output channels. If inputChannels = 0, the input will be disabled.
///
/// Specifying inputChannels = 0 uses the Playback AVAudioSession category
/// while setting inputChannels > 0 uses the PlaybackAndRecord category.
///
/// A channel value == -1 will use the current channel number(s) from
/// the audio session and automatically change them when the active audio
/// session route changes.
///
/// Note: Sets min stereo channels by default, see preferStereo property
- (PdAudioStatus)configurePlaybackWithSampleRate:(int)sampleRate
                                   inputChannels:(int)inputChannels
                                  outputChannels:(int)outputChannels;

/// Configure audio for recording, without output channels.
///
/// Uses the Record AVAudioSession category.
///
/// A channel value == -1 will use the current maximum channel number(s) from
/// the audio session and automatically change them when the active audio
/// session route changes.
- (PdAudioStatus)configureRecordWithSampleRate:(int)sampleRate
                                 inputChannels:(int)inputChannels;

/// Configure audio for ambient use, without input channels.
///
/// If mixWithOthers = YES, uses the Ambient AVAudioSession category, while NO
/// uses the SoloAmbient category.
///
/// A channel value == -1 will use the current channel number(s) from
/// the audio session and automatically change them when the active audio
/// session route changes.
- (PdAudioStatus)configureAmbientWithSampleRate:(int)sampleRate
                                 outputChannels:(int)outputChannels;

/// Configure audio for more advanced multi route port configuration,
/// see Apple docs. Note: does not allow Bluetooth or AirPlay.
/// Uses the MultiRoute AVAudioSession category.
///
/// A channel value == -1 will use the current channel number(s) from
/// the audio session and automatically change them when the active audio
/// session route changes.
- (PdAudioStatus)configureMultiRouteWithSampleRate:(int)sampleRate
                                     inputChannels:(int)inputChannels
                                    outputChannels:(int)outputChannels;

/// Note: legacy method kept for compatibility
///
/// Configure the audio with the specified samplerate, as well as number of
/// output channels (which will also be the number of input channels if input is
/// enabled).
///
/// Note that this method has three possible outcomes: success, failure, or
/// conditional success, where parameters had to be adjusted to set up the
/// audio. In the third case, you can query the sample rate and channel
/// properties to determine whether the selected configuration is acceptable.
///
/// Specifying inputEnabled = YES uses the PlayAndRecord AVAudioSession category
/// while setting NO uses the Playback category.
///
/// A channel value == -1 will use the current channel number(s) from
/// the audio session and automatically change them when the active audio
/// session route changes.
///
/// MixingEnabled = YES will allow the app to continue playing audio
/// along with other apps (such as Music), also sets the mixWithOthers property.
- (PdAudioStatus)configurePlaybackWithSampleRate:(int)sampleRate
                                  numberChannels:(int)numChannels
                                    inputEnabled:(BOOL)inputEnabled
                                   mixingEnabled:(BOOL)mixingEnabled;

/// Note: legacy method kept for compatibility
///
/// Configure audio for ambient use, without input channels.
///
/// A channel value == -1 will use the current channel number(s) from
/// the audio session and automatically change them when the active audio
/// session route changes.
///
/// Specifying mixingEnabled = YES will allow the app to continue playing audio
/// along with other apps (such as Music) and uses the Ambient AVAudioSession
/// category, while setting NO uses the SoloAmbient category. This also sets the
/// mixWithOthers property.
- (PdAudioStatus)configureAmbientWithSampleRate:(int)sampleRate
                                 numberChannels:(int)numChannels
                                  mixingEnabled:(BOOL)mixingEnabled;

/// Configure the ticksPerBuffer parameter, which will change the audio session
/// IO buffer size. This can be done on the fly, while audio is running.
///
/// Note that the audio session only accepts values that correspond to a number
/// of frames that are a power of 2 and sometimes this value is ignored by the
/// audio unit, which tries to work with whatever number of frames it is
/// provided.
- (PdAudioStatus)configureTicksPerBuffer:(int)ticksPerBuffer;

/// Print info on the audio session and audio unit to the console.
- (void)print;

/// Returns the combined audio session options when configuring for playback:
/// audio output only, no input.
/// Playback is chosen when inputEnabled = NO and/or inputChannels = 0
/// when configuring.
/// Override if you want to customize option handling.
- (AVAudioSessionCategoryOptions)playbackOptions;

/// Returns the combined audio session options when configuring for playback:
/// audio input and output.
/// PlayAndRecord is chosen when inputEnabled = YES and/or inputChannels > 0
/// when configuring.
/// Override if you want to customize option handling.
- (AVAudioSessionCategoryOptions)playAndRecordOptions;

/// Returns the combined audio session options when configuring for playback:
/// audio input only, no output.
/// Record is chosen when inputEnabled = YES and outputChannels > 0
/// when configuring.
/// Override if you want to customize option handling.
- (AVAudioSessionCategoryOptions)recordOptions;

/// Returns the default audio session options when configuring for ambient use,
/// doesn't mix with other apps.
/// SoloAmbient is chosen with mixingEnabled = NO and/or mixWithOthers = NO
/// when configuring.
/// Override if you want to customize option handling.
- (AVAudioSessionCategoryOptions)soloAmbientOptions;

/// Returns the default audio session options when configuring for ambient use,
/// mixes with other apps.
/// Ambient is chosen with mixingEnabled = YES and/or mixWithOthers = YES
/// when configuring.
/// Override if you want to customize option handling.
- (AVAudioSessionCategoryOptions)ambientOptions;

/// Returns the default audio session options when configuring for multi route
/// use, can mix with other apps, allows more advanced port configuration.
/// Override if you want to customize option handling.
- (AVAudioSessionCategoryOptions)multiRouteOptions;

/// Helper to add to the current audio session category options.
/// Returns YES on success.
+ (BOOL)addSessionOptions:(AVAudioSessionCategoryOptions)options;

/// Helper to replace the current audio session category options.
/// Returns YES on success.
+ (BOOL)setSessionOptions:(AVAudioSessionCategoryOptions)options;

@end
