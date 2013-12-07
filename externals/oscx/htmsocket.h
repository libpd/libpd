/*
Written by Adrian Freed, The Center for New Music and Audio Technologies,
University of California, Berkeley.  Copyright (c) 1992,93,94,95,96,97,98,99,2000,01,02,03,04
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


The OSC webpage is http://cnmat.cnmat.berkeley.edu/OpenSoundControl
*/


 /* htmparam.h

	Adrian Freed
 	send parameters to htm servers by udp or UNIX protocol
 */
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
typedef int bool;

/* open a socket for HTM communication to given  host on given portnumber */
/* if host is 0 then UNIX protocol is used (i.e. local communication) */
void *OpenHTMSocket(char *host, int portnumber, short *multicast_TTL);

/* send a buffer of data over htm socket, returns TRUE on success.
 Note that udp sends rarely fail. UNIX sends fail if a kernal buffer overflows */
bool SendHTMSocket(void *htmsendhandle, int length_in_bytes, void *buffer);

/* close the socket(2) and release memory associated with it */
void CloseHTMSocket(void *htmsendhandle);
