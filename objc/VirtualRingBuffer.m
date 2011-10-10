//
//  VirtualRingBuffer.m
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


#import "VirtualRingBuffer.h"

#include <mach/mach.h>
#include <mach/mach_error.h>


@implementation VirtualRingBuffer

@synthesize bufferLength;

static void *allocateVirtualBuffer(UInt32 bufferLength);
static void deallocateVirtualBuffer(void *buffer, UInt32 bufferLength);

- (id)initWithLength:(UInt32)length
{
    if (![super init])
        return nil;

    // We need to allocate entire VM pages, so round the specified length up to the next page if necessary.
    bufferLength = round_page(length);

    buffer = allocateVirtualBuffer(bufferLength);
    if (buffer) {
        bufferEnd = buffer + bufferLength;
    } else {
        [self release];
        return nil;        
    }

    readPointer = NULL;
    writePointer = NULL;

    return self;
}

- (void)dealloc
{
    if (buffer)
        deallocateVirtualBuffer(buffer, bufferLength);

    [super dealloc];
}

//
// Theory of operation:
//
// This class keeps a pointer to the next byte to be read (readPointer) and a pointer to the next byte to be written (writePointer).
// readPointer is only advanced in the reading thread (except for one case: when the buffer first has data written to it).
// writePointer is only advanced in the writing thread.
//
// Since loading and storing word length data is atomic, each pointer can safely be modified in one thread while the other thread
// uses it, IF each thread is careful to make a local copy of the "opposite" pointer when necessary.
// 

//
// Read operations
//

- (UInt32)lengthAvailableToReadReturningPointer:(void **)returnedReadPointer
{
    // Assumptions:
    // returnedReadPointer != NULL

    UInt32 length;
    // Read this pointer exactly once, so we're safe in case it is changed in another thread
    void *localWritePointer = writePointer;

    // Depending on out-of-order execution and memory storage, either one of these may be NULL when the buffer is empty. So we must check both.
    if (!readPointer || !localWritePointer) {
        // The buffer is empty
        length = 0;
    } else if (localWritePointer > readPointer) {
        // Write is ahead of read in the buffer
        length = localWritePointer - readPointer;
    } else {
        // Write has wrapped around past read, OR write == read (the buffer is full)
        length = bufferLength - (readPointer - localWritePointer);
    }

    *returnedReadPointer = readPointer;
    OSMemoryBarrier();  // Make sure that we can see the changes from the other thread.
    return length;
}

- (void)didReadLength:(UInt32)length
{
    // Assumptions:
    // [self lengthAvailableToReadReturningPointer:] currently returns a value >= length
    // length > 0

    void *newReadPointer;

    newReadPointer = readPointer + length;
    if (newReadPointer >= bufferEnd)
        newReadPointer -= bufferLength;

    if (newReadPointer == writePointer) {
        // We just read the last data out of the buffer, so it is now empty.
        newReadPointer = NULL;
    }

    // Store the new read pointer. This is the only place this happens in the read thread.
    readPointer = newReadPointer;    
}


//
// Write operations
//

- (UInt32)lengthAvailableToWriteReturningPointer:(void **)returnedWritePointer
{
    // Assumptions:
    // returnedWritePointer != NULL
    
    UInt32 length;
    // Read this pointer exactly once, so we're safe in case it is changed in another thread
    void *localReadPointer = readPointer;
    
    // Either one of these may be NULL when the buffer is empty. So we must check both.
    if (!localReadPointer || !writePointer) {
        // The buffer is empty. Set it up to be written into.
        // This is one of the two places the write pointer can change; both are in the write thread.
        writePointer = buffer;
        length = bufferLength;
    } else if (writePointer <= localReadPointer) {
        // Write is before read in the buffer, OR write == read (meaning that the buffer is full).
        length = localReadPointer - writePointer;
    } else {
        // Write is behind read in the buffer. The available space wraps around.
        length = (bufferEnd - writePointer) + (localReadPointer - buffer);
    }

    *returnedWritePointer = writePointer;
    return length;
}

- (void)didWriteLength:(UInt32)length
{
    OSMemoryBarrier();  // Make sure that the other thread can see our changes.
    
    // Assumptions:
    // [self lengthAvailableToWriteReturningPointer:] currently returns a value >= length
    // length > 0

    void *oldWritePointer = writePointer;
    void *newWritePointer;

    // Advance the write pointer, wrapping around if necessary.
    newWritePointer = writePointer + length;
    if (newWritePointer >= bufferEnd)
        newWritePointer -= bufferLength;

    // This is one of the two places the write pointer can change; both are in the write thread.
    writePointer = newWritePointer;

    // Also, if the read pointer is NULL, then we just wrote into a previously empty buffer, so set the read pointer.
    // This is the only place the read pointer is changed in the write thread.
    // The read thread should never change the read pointer when it is NULL, so this is safe.
    if (!readPointer)
        readPointer = oldWritePointer;
}

@end


void *allocateVirtualBuffer(UInt32 bufferLength)
{
    kern_return_t error;
    vm_address_t originalAddress = NULL;
    vm_address_t realAddress = NULL;
    mach_port_t memoryEntry;
    vm_size_t memoryEntryLength;
    vm_address_t virtualAddress = NULL;

    // We want to find where we can get 2 * bufferLength bytes of contiguous address space.
    // So let's just allocate that space, remember its address, and deallocate it.
    // (This doesn't actually have to touch all of that memory so it's not terribly expensive.)
    error = vm_allocate(mach_task_self(), &originalAddress, 2 * bufferLength, TRUE);
    if (error) {
#if DEBUG
        mach_error("vm_allocate initial chunk", error);
#endif
        return NULL;
    }

    error = vm_deallocate(mach_task_self(), originalAddress, 2 * bufferLength);
    if (error) {
#if DEBUG
        mach_error("vm_deallocate initial chunk", error);
#endif
        return NULL;
    }

    // Then allocate a "real" block of memory at the same address, but with the normal bufferLength.
    realAddress = originalAddress;
    error = vm_allocate(mach_task_self(), &realAddress, bufferLength, FALSE);
    if (error) {
#if DEBUG
        mach_error("vm_allocate real chunk", error);
#endif
        return NULL;
    }
    if (realAddress != originalAddress) {
#if DEBUG
        NSLog(@"allocateVirtualBuffer: vm_allocate 2nd time didn't return same address (%p vs %p)", originalAddress, realAddress);
#endif
        goto errorReturn;
    }

    // Then make a memory entry for the area we just allocated.
    memoryEntryLength = bufferLength;
    error = mach_make_memory_entry(mach_task_self(), &memoryEntryLength, realAddress, VM_PROT_READ | VM_PROT_WRITE, &memoryEntry, NULL);
    if (error) {
#if DEBUG
        mach_error("mach_make_memory_entry", error);
#endif
        goto errorReturn;
    }
    if (!memoryEntry) {
#if DEBUG
        NSLog(@"mach_make_memory_entry: returned memoryEntry of NULL");
#endif
        goto errorReturn;
    }
    if (memoryEntryLength != bufferLength) {
#if DEBUG
        NSLog(@"mach_make_memory_entry: size changed (from %0x to %0x)", bufferLength, memoryEntryLength);
#endif
        goto errorReturn;
    }

    // And map the area immediately after the first block, with length bufferLength, to that memory entry.
    virtualAddress = realAddress + bufferLength;
    error = vm_map(mach_task_self(), &virtualAddress, bufferLength, 0, FALSE, memoryEntry, 0, FALSE, VM_PROT_READ | VM_PROT_WRITE, VM_PROT_READ | VM_PROT_WRITE, VM_INHERIT_DEFAULT);
    if (error) {
#if DEBUG
        mach_error("vm_map", error);
#endif
        // TODO Retry from the beginning, instead of failing completely. There is a tiny (but > 0) probability that someone
        // will allocate this space out from under us.
        virtualAddress = NULL;
        goto errorReturn;
    }
    if (virtualAddress != realAddress + bufferLength) {
#if DEBUG
        NSLog(@"vm_map: didn't return correct address (%p vs %p)", realAddress + bufferLength, virtualAddress);
#endif
        goto errorReturn;
    }
    
    // Success!
    return (void *)realAddress;

#if DEBUG
    // Here's a little test...
    *realAddress = '?';
    if (*virtualAddress != '?')
        NSLog(@"VirtualRingBuffer: Test 1: vm magic failed");

    *(virtualAddress + 1) = '!';
    if (*(realAddress + 1) != '!')
        NSLog(@"VirtualRingBuffer: Test 2: vm magic failed");
#endif
    
errorReturn:
    if (realAddress)
        vm_deallocate(mach_task_self(), realAddress, bufferLength);
    if (virtualAddress)
        vm_deallocate(mach_task_self(), virtualAddress, bufferLength);

    return NULL;
}

void deallocateVirtualBuffer(void *buffer, UInt32 bufferLength)
{
    kern_return_t error;

    // We can conveniently deallocate both the vm_allocated memory and
    // the vm_mapped region at the same time.
    error = vm_deallocate(mach_task_self(), (vm_address_t)buffer, bufferLength * 2);
    if (error) {
#if DEBUG
        mach_error("vm_deallocate in dealloc", error);
#endif
    }
}
