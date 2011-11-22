//
//  PdAudioController.h
//  libpd
//
//  Created on 11/10/11.
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import "PdAudioUnit.h"

/* PdAudioController: A class for managing PdAudioUnit within iOS
 * by using the AVFoundation and Audio Services API's.
 * Handles phone interruptions and provides high level configuration methods
 * The returned PdAudioStatus is used to indicate success, failure, or
 * that parameters had to be adjusted in order to work.
 */
@interface PdAudioController : NSObject <AVAudioSessionDelegate>

// read only properties that are set by the configure methods
@property (nonatomic, readonly) int sampleRate;
@property (nonatomic, readonly) int numberInputChannels;
@property (nonatomic, readonly) int numberOutputChannels;
@property (nonatomic, readonly) int ticksPerBuffer;

// whether or not PdAudioUnit is active
@property (nonatomic, getter=isActive) BOOL active;

// configure audio with specified samplerate, # inputs, and # outputs. First checks whether
// inputs are availble and if not, will modify the session and audio unit properties to still function
- (PdAudioStatus)configureWithSampleRate:(int)sampleRate numberInputChannels:(int)numInputs numberOutputChannels:(int)numOutputs;

// configure audio for background use, so audio continues to play when the app looses focus. If inputs
// were previous specified, the numberInputChannels property will be set to 0. Specifying
// mixingEnabled = YES will allow the app to continue playing audio along with other apps (such as iPod
// music player)
- (PdAudioStatus)configureForBackgroundAudioWithSampleRate:(int)sampleRate numberOutputChannels:(int)numOutputs mixingEnabled:(BOOL)mixingEnabled;

// configure the ticksPerBuffer parameter, which will change the audio sessions IO buffer size.
// This can be done on the fly, while audio is running.  Note that the audio session only accepts
// values that correspond to a number of frames that are a power of 2 and sometimes this value
// is ignored by the audio unit, which tries to work with whatever number of frames it is provided.
- (PdAudioStatus)configureTicksPerBuffer:(int)ticksPerBuffer;

- (void)print;

@end
