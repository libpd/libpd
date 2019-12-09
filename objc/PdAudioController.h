//
//  PdAudioController.h
//  libpd
//
//  Created on 11/10/11.
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//
//  Updated 2018 Dan Wilcox <danomatika@gmail.com>
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import "PdAudioUnit.h"

/// PdAudioStatus is used to indicate success, failure, or that parameters had
/// to be adjusted in order to work.
typedef enum PdAudioStatus {
	PdAudioOK = 0,             // success
	PdAudioError = -1,         // unrecoverable error
	PdAudioPropertyChanged = 1 // some properties have changed to run correctly
} PdAudioStatus;

/// PdAudioController: A class for managing a PdAudioUnit instance within iOS
/// by using the AVFoundation and Audio Services APIs. Handles phone
/// interruptions and provides high level configuration methods.
@interface PdAudioController : NSObject

/// Read only properties that are set by the configure methods
@property (nonatomic, readonly) int sampleRate;
@property (nonatomic, readonly) int numberChannels;
@property (nonatomic, readonly) BOOL inputEnabled;
@property (nonatomic, readonly) BOOL mixingEnabled;
@property (nonatomic, readonly) int ticksPerBuffer;

/// Read only access to the underlying pd audio unit
@property (nonatomic, strong, readonly) PdAudioUnit *audioUnit;

/// Check or set the active status of the audio unit
@property (nonatomic, getter=isActive) BOOL active;

/// Init with default pd audio unit.
- (instancetype)init;

/// Init with a custom pd audio unit.
///
/// Derive PdAudioUnit when you need to access to the raw samples when using,
/// for instance, AudioBus, and call this method after init.
- (instancetype)initWithAudioUnit:(PdAudioUnit *)audioUnit;

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
/// Specifying mixingEnabled = YES will allow the app to continue playing audio
/// along with other apps (such as Music).
- (PdAudioStatus)configurePlaybackWithSampleRate:(int)sampleRate
                                  numberChannels:(int)numChannels
                                    inputEnabled:(BOOL)inputEnabled
                                   mixingEnabled:(BOOL)mixingEnabled;

/// Configure audio for ambient use, without input channels.
///
/// Specifying mixingEnabled = YES will allow the app to continue playing audio
/// along with other apps (such as Music) and uses the Ambient AVAudioSession
/// category, while setting NO uses the SoloAmboent category.
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

/// Print current settings to the console.
- (void)print;

/// Returns the default audio session options when configuring for playback:
/// audio output only, no input.
/// Playback is chosen when inputEnabled = YEs when configuring.
/// Note: AVAudioSessionCategoryOptionMixWithOthers is set when configuring.
/// Override if you want to add new options such as bluetooth support, etc.
- (AVAudioSessionCategoryOptions)playbackOptions;

/// Returns the default audio session options when configuring for playback:
/// audio input and output, AVAudioSessionCategoryOptionDefaultToSpeaker.
/// PlayAndRecord is chosen when inputEnabled = YEs when configuring.
/// Note: AVAudioSessionCategoryOptionMixWithOthers is set when configuring.
/// Override if you want to add new options such as bluetooth support, etc.
- (AVAudioSessionCategoryOptions)playAndRecordOptions;

/// Returns the default audio session options when configuring for ambient use,
/// doesn't mix with other apps.
/// SoloAmbient is chosen with mixingEnabled = NO when configuring.
/// Override if you want to add new options such as bluetooth support, etc.
- (AVAudioSessionCategoryOptions)soloAmbientOptions;

/// Returns the default audio session options when configuring for ambient use,
/// mixes with other apps.
/// Ambient is chosen with mixingEnabled = YES when configuring.
/// Override if you want to add new options such as bluetooth support, etc.
- (AVAudioSessionCategoryOptions)ambientOptions;

/// Helper to add to the current audio session category options.
/// Returns YES on success.
+ (BOOL)addSessionOptions:(AVAudioSessionCategoryOptions)options;

/// Helper to replace the current audio session category options.
/// Returns YES on success.
+ (BOOL)setSessionOptions:(AVAudioSessionCategoryOptions)options;

@end
