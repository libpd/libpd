//
//  VirtualRingBuffer.h
//  PlayBufferedSoundFile
//
/*
 Copyright (c) 2002, Kurt Revis.  All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * Neither the name of Snoize nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//
// VirtualRingBuffer implements a classic ring buffer (or circular buffer), with a couple of twists.
//
// * It allows reads and writes to happen in different threads, with no explicit locking,
//   so readers and writers will never block. This is useful if either thread uses the
//   time-constraint scheduling policy, since it is bad for such threads to block for
//   indefinite amounts of time.
//   
// * It uses a virtual memory trick to allow the client to read or write using just one
//   operation, even if the data involved wraps around the end of the buffer. We allocate
//   our buffer normally, and then place a VM region immediately after it in the address
//   space which maps back to the "real" buffer. So reads and writes into both sections
//   are transparently translated into the same physical memory.
//   This makes the API much simpler to use, and saves us from doing some math to
//   calculate the wraparound points.
//   The tradeoff is that we use twice as much address space for the buffer than we would
//   otherwise.  Address space is not typically constrained for most applications, though,
//   so this isn't a big problem.
//   The idea for this trick came from <http://phil.ipal.org/freeware/vrb/> (via sweetcode.org),
//   although none of that code is used here. (We use the Mach VM API directly.)
//

// Threading note:
// It is expected that this object will be shared between exactly two threads; one will
// always read and the other will always write. In that situation, the implementation is
// thread-safe, and this object will never block or yield.
// It will also work in one thread, of course (although I don't know why you'd bother).
// However, if you have multiple reader or writer threads, all bets are off!

#import <Foundation/Foundation.h>


@interface VirtualRingBuffer : NSObject
{
    void *buffer;
    void *bufferEnd;
    UInt32 bufferLength;
        // buffer is the start of the ring buffer's address space.
        // bufferEnd is the end of the "real" buffer (always buffer + bufferLength).
        // Note that the "virtual" portion of the buffer extends from bufferEnd to bufferEnd+bufferLength.
    
    volatile void *readPointer;
    volatile void *writePointer;
}

- (id)initWithLength:(UInt32)length;
// Note: The specified length will be rounded up to an integral number of VM pages.

// Read operations:

// The reading thread must call this method first.
- (UInt32)lengthAvailableToReadReturningPointer:(void **)returnedReadPointer;
// Iff a value > 0 is returned, the reading thread may go on to read that much data from the returned pointer.
// Afterwards, the reading thread must call didReadLength:.
- (void)didReadLength:(UInt32)length;

// Write operations:

// The writing thread must call this method first.
- (UInt32)lengthAvailableToWriteReturningPointer:(void **)returnedWritePointer;
// Iff a value > 0 is returned, the writing thread may then write that much data into the returned pointer.
// Afterwards, the writing thread must call didWriteLength:.
- (void)didWriteLength:(UInt32)length;

@end
