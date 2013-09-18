/*
 * Copyright (c) 1997 Regents of the University of California.
 * All rights reserved.
 *
 * The name of the University may not be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE
 * IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.
 */

/*
   test_OSC.c
   Trivial program to test OpenSoundControl.[ch]

   Matt Wright 6/2/97
*/

#include <stdio.h>
#include <ctype.h>
#include "OpenSoundControl.h"

#define SIZE 10000

void PrintBuf(OSCbuf *b) {
    printf("Buffer is %sempty.\n", OSC_isBufferEmpty(b) ? "" : "not ");
    printf("%d bytes free in buffer\n", OSC_freeSpaceInBuffer(b));
    printf("Buffer is %sready to send\n", OSC_isBufferDone(b) ?"":"not ");

    printf("Buffer: bufptr %p, state %d, thisMsgSize %p, bundleDepth %d\n"
	   "prevCounts[%d] %p\n", b->bufptr, b->state, b->thisMsgSize,
	   b->bundleDepth, b->bundleDepth, b->prevCounts[b->bundleDepth]);
}

void PrintPacket(OSCbuf *b) {
    char *p = OSC_getPacket(b);
    int size = OSC_packetSize(b);
    unsigned int *intp;
    int i;

    printf("PrintPacket: packet at %p, size %d\n", p, size);
    if (p == 0 || size == 0) return;

    printf("Hex version:");
    for (i = 0, intp = (unsigned int *)p; i < size; i += 4, intp++) {
	if (i % 40 == 0) printf("\n");
	printf("%x ", *intp);
    }

    printf("\n\nString version:");
    for (i = 0; i < size; i++) {
	if (i % 40 == 0) printf("\n");
	if (isprint(p[i])) {
	    printf("%c", p[i]);
	} else {
	    printf("\\%x", p[i] & 0x000000ff);
	}
    }
    printf("\n");
}
    

main() {
    OSCbuf myBuf;
    OSCbuf *b = &myBuf;
    char bytes[SIZE];
    OSCTimeTag tt;

    printf("OSC_initBuffer\n");
    OSC_initBuffer(b, SIZE, bytes);

    PrintBuf(b);

    printf("Testing one-message packet\n");
    if (OSC_writeAddress(b, "/blah/bleh/singlemessage")) {
	printf("** ERROR: %s\n", OSC_errorMessage);
    }

    if (OSC_writeFloatArg(b, 1.23456f)) {
        printf("** ERROR: %s\n", OSC_errorMessage);
    }

    {
	float floatarray[10];
	int i;
	for (i = 0; i < 10; ++i) {
	    floatarray[i] = i * 10.0f;
	}
	if (OSC_writeFloatArgs(b, 10, floatarray)) {
	    printf("** ERROR: %s\n", OSC_errorMessage);
	}
    }

    if (OSC_writeIntArg(b, 123456)) {
        printf("** ERROR: %s\n", OSC_errorMessage);
    }

    if (OSC_writeStringArg(b, "This is a cool string, dude.")) {
        printf("** ERROR: %s\n", OSC_errorMessage);
    }

    PrintBuf(b);
    PrintPacket(b);

    printf("Resetting\n");
    OSC_resetBuffer(b);

    printf("Testing time tags\n");
    tt = OSCTT_CurrentTime();
    printf("Time now is %llx\n", tt);

    printf("Testing bundles\n");
    if (OSC_openBundle(b, tt)) {
	printf("** ERROR: %s\n", OSC_errorMessage);
    }

    if (OSC_writeAddress(b, "/a/hello")) {
        printf("** ERROR: %s\n", OSC_errorMessage);
    }

    if (OSC_writeIntArg(b, 16)) {
        printf("** ERROR: %s\n", OSC_errorMessage);
    }

    if (OSC_writeIntArg(b, 32)) {
        printf("** ERROR: %s\n", OSC_errorMessage);
    }

    if (OSC_openBundle(b, OSCTT_PlusSeconds(tt, 1.0f))) {
        printf("** ERROR: %s\n", OSC_errorMessage);
    }

    if (OSC_writeAddress(b, "/b/hello")) {
        printf("** ERROR: %s\n", OSC_errorMessage);
    }

    if (OSC_writeAddress(b, "/c/hello")) {
        printf("** ERROR: %s\n", OSC_errorMessage);
    }

    OSC_closeAllBundles(b);

    PrintBuf(b);
    PrintPacket(b);
}


    
