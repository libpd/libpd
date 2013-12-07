/*
Written by Matt Wright, The Center for New Music and Audio Technologies,
University of California, Berkeley.  Copyright (c) 1996,97,98,99,2000,01,02,03
The Regents of the University of California (Regents).  

Permission to use, copy, modify, distribute, and distribute modified versions
of this software and its documentation without fee and without a signed
licensing agreement, is hereby granted, provided that the above copyright
notice, this paragraph and the following two paragraphs appear in all copies,
modifications, and distributions.

IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/


/* 
  Author: Matt Wright
  Version 2.2: Calls htonl in the right places 20000620
  Version 2.3: Gets typed messages right.
 */

/*
	pd
	-------------

	raf@interaccess.com:
	rev. for Win32 build  (verified under Win-2ooo)		11-April-2002

	-- changed licence part (20040820) jdl
	-- Version 2.4 changes not in here (20040820) jdl

*/

/* Here are the possible values of the state field: */

#define EMPTY 0	       /* Nothing written to packet yet */
#define ONE_MSG_ARGS 1 /* Packet has a single message; gathering arguments */
#define NEED_COUNT 2   /* Just opened a bundle; must write message name or
			  open another bundle */
#define GET_ARGS 3     /* Getting arguments to a message.  If we see a message
			  name or a bundle open/close then the current message
			  will end. */
#define DONE 4         /* All open bundles have been closed, so can't write 
		          anything else */

#include <sys/types.h>
#include <stdio.h>    
#include <string.h>    

#ifdef _WIN32
	#include	<winsock2.h>
	#include 	<io.h>    
	#include 	<errno.h>
	#include 	<fcntl.h>
	#include 	<sys/stat.h>
#endif

#ifdef unix
  #include <netinet/in.h>
#endif
	
#include "OSC-client.h"

char *OSC_errorMessage;

static int OSC_padString(char *dest, char *str);
static int OSC_padStringWithAnExtraStupidComma(char *dest, char *str);
static int OSC_WritePadding(char *dest, int i);
static int CheckTypeTag(OSCbuf *buf, char expectedType);

void OSC_initBuffer(OSCbuf *buf, int size, char *byteArray) {
    buf->buffer = byteArray;
    buf->size = size;
    OSC_resetBuffer(buf);
}

void OSC_resetBuffer(OSCbuf *buf) {	
    buf->bufptr = buf->buffer;
    buf->state = EMPTY;
    buf->bundleDepth = 0;
    buf->prevCounts[0] = 0;
    buf->gettingFirstUntypedArg = 0;
    buf->typeStringPtr = 0;
}

int OSC_isBufferEmpty(OSCbuf *buf) {
    return buf->bufptr == buf->buffer;
}

int OSC_freeSpaceInBuffer(OSCbuf *buf) {
    return buf->size - (buf->bufptr - buf->buffer);
}

int OSC_isBufferDone(OSCbuf *buf) {
    return (buf->state == DONE || buf->state == ONE_MSG_ARGS);
}

char *OSC_getPacket(OSCbuf *buf) {
#ifdef ERROR_CHECK_GETPACKET
    if (buf->state == DONE || buf->state == ONE_MSG_ARGS) {
	return buf->buffer;
    } else {
	OSC_errorMessage = "Packet has unterminated bundles";
	return 0;
    }
#else
    return buf->buffer;
#endif
}

int OSC_packetSize(OSCbuf *buf) {
#ifdef ERROR_CHECK_PACKETSIZE
    if (buf->state == DONE || buf->state == ONE_MSG_ARGS) {
	return (buf->bufptr - buf->buffer);
    } else {
        OSC_errorMessage = "Packet has unterminated bundles";
        return 0;
    }
#else
    return (buf->bufptr - buf->buffer);
#endif
}

#define CheckOverflow(buf, bytesNeeded) { \
    if ((bytesNeeded) > OSC_freeSpaceInBuffer(buf)) { \
	OSC_errorMessage = "buffer overflow"; \
	return 1; \
    } \
}

static void PatchMessageSize(OSCbuf *buf) {
    int32_t size;
    size = buf->bufptr - ((char *) buf->thisMsgSize) - 4;
    *(buf->thisMsgSize) = htonl(size);
}

int OSC_openBundle(OSCbuf *buf, OSCTimeTag tt) {
    if (buf->state == ONE_MSG_ARGS) {
	OSC_errorMessage = "Can't open a bundle in a one-message packet";
	return 3;
    }

    if (buf->state == DONE) {
	OSC_errorMessage = "This packet is finished; can't open a new bundle";
	return 4;
    }

    if (++(buf->bundleDepth) >= MAX_BUNDLE_NESTING) {
	OSC_errorMessage = "Bundles nested too deeply; change MAX_BUNDLE_NESTING in OpenSoundControl.h";
	return 2;
    }

    if (CheckTypeTag(buf, '\0')) return 9;

    if (buf->state == GET_ARGS) {
	PatchMessageSize(buf);
    }

    if (buf->state == EMPTY) {
	/* Need 16 bytes for "#bundle" and time tag */
	CheckOverflow(buf, 16);
    } else {
	/* This bundle is inside another bundle, so we need to leave
	   a blank size count for the size of this current bundle. */
	CheckOverflow(buf, 20);
	*((int32_t *)buf->bufptr) = 0xaaaaaaaa;
        buf->prevCounts[buf->bundleDepth] = (int32_t *)buf->bufptr;

	buf->bufptr += 4;
    }

    buf->bufptr += OSC_padString(buf->bufptr, "#bundle");


    *((OSCTimeTag *) buf->bufptr) = tt;

    if (htonl(1) != 1) {
	/* Byte swap the 8-byte integer time tag */
	int32_t *intp = (int32_t *)buf->bufptr;
	intp[0] = htonl(intp[0]);
	intp[1] = htonl(intp[1]);

#ifdef HAS8BYTEINT
	{ /* tt is a 64-bit int so we have to swap the two 32-bit words. 
	    (Otherwise tt is a struct of two 32-bit words, and even though
	     each word was wrong-endian, they were in the right order
	     in the struct.) */
	    int32_t temp = intp[0];
	    intp[0] = intp[1];
	    intp[1] = temp;
	}
#endif
    }

    buf->bufptr += sizeof(OSCTimeTag);

    buf->state = NEED_COUNT;

    buf->gettingFirstUntypedArg = 0;
    buf->typeStringPtr = 0;
    return 0;
}


int OSC_closeBundle(OSCbuf *buf) {
    if (buf->bundleDepth == 0) {
	/* This handles EMPTY, ONE_MSG, ARGS, and DONE */
	OSC_errorMessage = "Can't close bundle; no bundle is open!";
	return 5;
    }

    if (CheckTypeTag(buf, '\0')) return 9;

    if (buf->state == GET_ARGS) {
        PatchMessageSize(buf);
    }

    if (buf->bundleDepth == 1) {
	/* Closing the last bundle: No bundle size to patch */
	buf->state = DONE;
    } else {
	/* Closing a sub-bundle: patch bundle size */
	int size = buf->bufptr - ((char *) buf->prevCounts[buf->bundleDepth]) - 4;
	*(buf->prevCounts[buf->bundleDepth]) = htonl(size);
	buf->state = NEED_COUNT;
    }

    --buf->bundleDepth;
    buf->gettingFirstUntypedArg = 0;
    buf->typeStringPtr = 0;
    return 0;
}


int OSC_closeAllBundles(OSCbuf *buf) {
    if (buf->bundleDepth == 0) {
        /* This handles EMPTY, ONE_MSG, ARGS, and DONE */
        OSC_errorMessage = "Can't close all bundles; no bundle is open!";
        return 6;
    }

    if (CheckTypeTag(buf, '\0')) return 9;

    while (buf->bundleDepth > 0) {
	OSC_closeBundle(buf);
    }
    buf->typeStringPtr = 0;
    return 0;
}

int OSC_writeAddress(OSCbuf *buf, char *name) {
    int32_t paddedLength;

    if (buf->state == ONE_MSG_ARGS) {
	OSC_errorMessage = "This packet is not a bundle, so you can't write another address";
	return 7;
    }

    if (buf->state == DONE) {
        OSC_errorMessage = "This packet is finished; can't write another address";
        return 8;
    }

    if (CheckTypeTag(buf, '\0')) return 9;

    paddedLength = OSC_effectiveStringLength(name);

    if (buf->state == EMPTY) {
	/* This will be a one-message packet, so no sizes to worry about */
	CheckOverflow(buf, paddedLength);
	buf->state = ONE_MSG_ARGS;
    } else {
	/* GET_ARGS or NEED_COUNT */
	CheckOverflow(buf, 4+paddedLength);
	if (buf->state == GET_ARGS) {
	    /* Close the old message */
	    PatchMessageSize(buf);
	}
	buf->thisMsgSize = (int32_t *)buf->bufptr;
	*(buf->thisMsgSize) = 0xbbbbbbbb;
	buf->bufptr += 4;
	buf->state = GET_ARGS;
    }

    /* Now write the name */
    buf->bufptr += OSC_padString(buf->bufptr, name);
    buf->typeStringPtr = 0;
    buf->gettingFirstUntypedArg = 1;

    return 0;
}

int OSC_writeAddressAndTypes(OSCbuf *buf, char *name, char *types) {
    int result;
    int32_t paddedLength;

    if (CheckTypeTag(buf, '\0')) return 9;

    result = OSC_writeAddress(buf, name);

    if (result) return result;

    paddedLength = OSC_effectiveStringLength(types);

    CheckOverflow(buf, paddedLength);    

    buf->typeStringPtr = buf->bufptr + 1; /* skip comma */
    buf->bufptr += OSC_padString(buf->bufptr, types);

    buf->gettingFirstUntypedArg = 0;
    return 0;
}

static int CheckTypeTag(OSCbuf *buf, char expectedType) {
    if (buf->typeStringPtr) {
	if (*(buf->typeStringPtr) != expectedType) {
	    if (expectedType == '\0') {
		OSC_errorMessage =
		    "According to the type tag I expected more arguments.";
	    } else if (*(buf->typeStringPtr) == '\0') {
		OSC_errorMessage =
		    "According to the type tag I didn't expect any more arguments.";
	    } else {
		OSC_errorMessage =
		    "According to the type tag I expected an argument of a different type.";
		printf("* Expected %c, string now %s\n", expectedType, buf->typeStringPtr);
	    }
	    return 9; 
	}
	++(buf->typeStringPtr);
    }
    return 0;
}


int OSC_writeFloatArg(OSCbuf *buf, float arg) {
    ls_pcast32 *pc;
    //int result;

    CheckOverflow(buf, 4);

    if (CheckTypeTag(buf, 'f')) return 9;

    /* Pretend arg is a long int so we can use htonl() */
    pc = (ls_pcast32 *)&arg;
    *((int32_t *) buf->bufptr) = htonl((*pc).i);

    buf->bufptr += 4;

    buf->gettingFirstUntypedArg = 0;
    return 0;
}



int OSC_writeFloatArgs(OSCbuf *buf, int numFloats, float *args) {
    int i;
    int32_t *intp;

    CheckOverflow(buf, 4 * numFloats);

    /* Pretend args are long ints so we can use htonl() */
    intp = ((int32_t *) args);

    for (i = 0; i < numFloats; i++) {
	if (CheckTypeTag(buf, 'f')) return 9;
	*((int32_t *) buf->bufptr) = htonl(intp[i]);
	buf->bufptr += 4;
    }

    buf->gettingFirstUntypedArg = 0;
    return 0;
}

int OSC_writeIntArg(OSCbuf *buf, int32_t arg) {
    CheckOverflow(buf, 4);
    if (CheckTypeTag(buf, 'i')) return 9;

    *((int32_t *) buf->bufptr) = htonl(arg);
    buf->bufptr += 4;

    buf->gettingFirstUntypedArg = 0;
    return 0;
}

int OSC_writeStringArg(OSCbuf *buf, char *arg) {
    int len;

    if (CheckTypeTag(buf, 's')) return 9;

    len = OSC_effectiveStringLength(arg);

    if (buf->gettingFirstUntypedArg && arg[0] == ',') {
	/* This un-type-tagged message starts with a string
	   that starts with a comma, so we have to escape it
	   (with a double comma) so it won't look like a type
	   tag string. */

	CheckOverflow(buf, len+4); /* Too conservative */
	buf->bufptr += 
	    OSC_padStringWithAnExtraStupidComma(buf->bufptr, arg);

    } else {
	CheckOverflow(buf, len);
	buf->bufptr += OSC_padString(buf->bufptr, arg);
    }

    buf->gettingFirstUntypedArg = 0;
    return 0;

}

/* String utilities */

#define STRING_ALIGN_PAD 4
int OSC_effectiveStringLength(char *string) {
    int len = strlen(string) + 1;  /* We need space for the null char. */
    
    /* Round up len to next multiple of STRING_ALIGN_PAD to account for alignment padding */
    if ((len % STRING_ALIGN_PAD) != 0) {
        len += STRING_ALIGN_PAD - (len % STRING_ALIGN_PAD);
    }
    return len;
}

static int OSC_padString(char *dest, char *str) {
    int i;
    
    for (i = 0; str[i] != '\0'; i++) {
        dest[i] = str[i];
    }
    
    return OSC_WritePadding(dest, i);
}

static int OSC_padStringWithAnExtraStupidComma(char *dest, char *str) {
    int i;
    
    dest[0] = ',';
    for (i = 0; str[i] != '\0'; i++) {
        dest[i+1] = str[i];
    }

    return OSC_WritePadding(dest, i+1);
}
 
static int OSC_WritePadding(char *dest, int i) {
    dest[i] = '\0';
    i++;

    for (; (i % STRING_ALIGN_PAD) != 0; i++) {
	dest[i] = '\0';
    }

    return i;
}
