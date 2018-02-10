//
//  PdBufferedAudioUnit.h
//  libpd
//
//  Created on 02/09/18.
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//

#import "PdAudioUnit.h"
#include "ringbuffer.h"

/// a buffered PdAudioUnit which should handle variable device audio buffer
/// sizes on newer devices due to power throttling, audio interface changes, etc
@interface PdBufferedAudioUnit : PdAudioUnit {
@protected
	ring_buffer *_inputBuffer;  ///< input FIFO
	ring_buffer *_outputBuffer; ///< output FIFO
	int _blockSize; ///< numer of bytes in one block of samples
}

@end
