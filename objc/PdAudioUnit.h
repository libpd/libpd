//
//  PdAudioUnit.h
//  libpd
//
//  Created on 29/09/11.
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//

#import <Foundation/Foundation.h>
#import "AudioUnit/AudioUnit.h"

typedef enum PdAudioStatus {
	PdAudioOK = 0,				// success
	PdAudioError = -1,			// unrecoverable error
	PdAudioPropertyChanged = 1  // some properties have changed to run correctly (not fatal)
} PdAudioStatus;

/* PdAudioUnit: object that operates pd's audio input and
 * output through an Audio Unit. The parameters can be changed
 * after it has been instatiated with it's configure method,
 * which will cause the underlying audio unit to be reconstructed.
 *
 * For debugging, AU_DEBUG_VERBOSE can be defined to print extra information.
 */
@interface PdAudioUnit : NSObject {
@private
	int blockSizeAsLog_;
}

// a reference to the audio unit, which can be used to query or set other properties
@property (nonatomic, readonly) AudioUnit audioUnit;

// sampling rate, stored as Float64 for convient use in the AudioUnit framework
@property (nonatomic, readonly) Float64 sampleRate;

// numberInputChannels and numberOutput channels specify the routing between
// the audio unit (and thereby the microphone and speaker) and pd. Setting
// numberInputChannels to 0 turns off input audio rendering. Note that you
// may experience silence when using unequal, positive values (i.e. ins = 1, outs = 2)
@property (nonatomic, readonly) int numberInputChannels;
@property (nonatomic, readonly) int numberOutputChannels;

// The configure method sets all parameters of the audio unit that may require it to be
// recreated at the same time.  This is an expensive process and will stop the audio unit
// before any reconstruction, causing a momentary pause in audio and UI if
// run from the main thread.
- (PdAudioStatus)configureWithSampleRate:(Float64)sampleRate
					 numberInputChannels:(int)numInputs
					numberOutputChannels:(int)numOutputs;

// start the audio unit
- (void)start;

// stop the audio unit
- (void)stop;

// whether or not the audio unit is running
- (BOOL)isRunning;

// print info on the audio unit settings to the console
- (void)print;

@end
