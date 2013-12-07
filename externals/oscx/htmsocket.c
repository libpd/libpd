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

 /* htmsocket.c

	Adrian Freed
 	send parameters to htm servers by udp or UNIX protocol

    Modified 6/6/96 by Matt Wright to understand symbolic host names
    in addition to X.X.X.X addresses.

    pd: branched jdl
    -- win additions raf 2002
    -- enabled broadcast jdl 2003
 */

#if HAVE_CONFIG_H 
#include <config.h> 
#endif

#ifdef __APPLE__
  #include <string.h>
#endif

#ifdef _WIN32
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <winsock2.h>	
	#include <ctype.h>
	#include <signal.h>
	#include <sys/types.h>
	#include <stdlib.h>
	#include "OSC-common.h"
	#include <stdio.h>
	#include <io.h>
    #include <ws2tcpip.h>
#else
	#include <stdio.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <netinet/in.h>

//	#include <rpc/rpc.h>
	#include <sys/socket.h>
	#include <sys/un.h>
	#include <sys/times.h>
	#include <sys/param.h>
	#include <sys/time.h>
	#include <sys/ioctl.h>

	#include <ctype.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <pwd.h>
	#include <signal.h>
	#include <grp.h>
	#include <sys/fcntl.h>
	#include <sys/file.h>
	#include <sys/time.h>
	#include <sys/types.h>
//	#include <sys/prctl.h>

	#include <stdlib.h>
#endif

#define UNIXDG_PATH "/tmp/htm"
#define UNIXDG_TMP "/tmp/htm.XXXXXX"
#include "htmsocket.h"                          
typedef struct
{
	float srate;

	struct sockaddr_in serv_addr; /* udp socket */
	#ifndef _WIN32
		struct sockaddr_un userv_addr; /* UNIX socket */
	#endif
	int sockfd;		/* socket file descriptor */
	int index, len,uservlen;
	void *addr;
	int id;
} desc;

int IsAddressMulticast(unsigned long address)
{
    unsigned long adr = ntohl(address);
    unsigned char A = (unsigned char)((adr & 0xFF000000) >> 24);
    unsigned char B = (unsigned char)((adr & 0xFF0000) >> 16);
    unsigned char C = (unsigned char)((adr & 0xFF00) >> 8);

    if (A==224 && B==0 && C==0) {
        // This multicast group range is reserved for routing
        // information and not meant to be used by applications.
        return -1;
    }

    // This is the multicast group IP range
    if (A>=224 && A<=239)
        return 1;

    return 0;
}

/* open a socket for HTM communication to given  host on given portnumber */
/* if host is 0 then UNIX protocol is used (i.e. local communication */
void *OpenHTMSocket(char *host, int portnumber, short *multicast_TTL)
{
	struct sockaddr_in  cl_addr;
	#ifndef _WIN32
		int sockfd;
		struct sockaddr_un ucl_addr;
	#else
		unsigned int sockfd;
	#endif

	char oval = 1;

	desc *o;
	o = malloc(sizeof(*o));
	if(!o)
		return 0;

  #ifndef _WIN32

	if(!host)
	{
		char *mktemp(char *);
		int clilen;
		  o->len = sizeof(ucl_addr);
		/*
		         * Fill in the structure "userv_addr" with the address of the
		         * server that we want to send to.
		*/
		
		bzero((char *) &o->userv_addr, sizeof(o->userv_addr));
		       o->userv_addr.sun_family = AF_UNIX;
		strcpy(o->userv_addr.sun_path, UNIXDG_PATH);
			sprintf(o->userv_addr.sun_path+strlen(o->userv_addr.sun_path), "%d", portnumber);
	        o->uservlen = sizeof(o->userv_addr.sun_family) + strlen(o->userv_addr.sun_path);
		o->addr = &(o->userv_addr);
		/*
		* Open a socket (a UNIX domain datagram socket).
		*/
		
		if ( (sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) >= 0)
		{
			/*
			 * Bind a local address for us.
			 * In the UNIX domain we have to choose our own name (that
			 * should be unique).  We'll use mktemp() to create a unique
			 * pathname, based on our process id.
			 */
		
			bzero((char *) &ucl_addr, sizeof(ucl_addr));    /* zero out */
			ucl_addr.sun_family = AF_UNIX;
			strcpy(ucl_addr.sun_path, UNIXDG_TMP);

			mktemp(ucl_addr.sun_path);
			clilen = sizeof(ucl_addr.sun_family) + strlen(ucl_addr.sun_path);
		
			if (bind(sockfd, (struct sockaddr *) &ucl_addr, clilen) < 0)
			{
			  perror("client: can't bind local address");
			  close(sockfd);
			  sockfd = -1;
			}
		}
		else
		  perror("unable to make socket\n");
		
	}else

  #endif

	{
		/*
		         * Fill in the structure "serv_addr" with the address of the
		         * server that we want to send to.
		*/
		o->len = sizeof(cl_addr);

		#ifdef _WIN32
			ZeroMemory((char *)&o->serv_addr, sizeof(o->serv_addr));
		#else
			bzero((char *)&o->serv_addr, sizeof(o->serv_addr));
		#endif

		o->serv_addr.sin_family = AF_INET;

	    /* MW 6/6/96: Call gethostbyname() instead of inet_addr(),
	       so that host can be either an Internet host name (e.g.,
	       "les") or an Internet address in standard dot notation 
	       (e.g., "128.32.122.13") */
	    {
			struct hostent *hostsEntry;
			unsigned long address;

			hostsEntry = gethostbyname(host);
			if (hostsEntry == NULL) {
				fprintf(stderr, "Couldn't decipher host name \"%s\"\n", host);
				#ifndef _WIN32
				herror(NULL);
				#endif
				return 0;
			}
			
			address = *((unsigned long *) hostsEntry->h_addr_list[0]);
			o->serv_addr.sin_addr.s_addr = address;
	    }

	    /* was: o->serv_addr.sin_addr.s_addr = inet_addr(host); */

	    /* End MW changes */

		/*
		 * Open a socket (a UDP domain datagram socket).
		 */


		#ifdef _WIN32
			o->serv_addr.sin_port = htons((USHORT)portnumber);
			o->addr = &(o->serv_addr);
			if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) != INVALID_SOCKET) {
				ZeroMemory((char *)&cl_addr, sizeof(cl_addr));
				cl_addr.sin_family = AF_INET;
				cl_addr.sin_addr.s_addr = htonl(INADDR_ANY);
				cl_addr.sin_port = htons(0);
				
				// enable broadcast: jdl ~2003
				if(setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &oval, sizeof(int)) == -1) {
				  perror("setsockopt broadcast");
				}

                // check if specified address is a multicast group: ss 2007
                int multicast = IsAddressMulticast(o->serv_addr.sin_addr.s_addr);

                if (multicast == -1) {
                    perror("Multicast group range 224.0.0.[0-255] is reserved.\n");
                    *multicast_TTL = -2;
                    close(sockfd);
                    sockfd = -1;
                }
                else {
                    // set TTL according to whether we have a multicast group or not
                    if (multicast) {
                        if (*multicast_TTL<0)
                            *multicast_TTL = 1;
                    } else
                        *multicast_TTL = -1;

                    // set multicast Time-To-Live only if it is a multicast group: ss 2007
                    if(*multicast_TTL>=0) {
                        unsigned char ttl = (unsigned char)*multicast_TTL;
                        if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL,
                                       &ttl, sizeof(ttl)) == -1)
                            perror("setsockopt TTL");
                    }

                    if(bind(sockfd, (struct sockaddr *) &cl_addr, sizeof(cl_addr)) < 0) {
                        perror("could not bind\n");
                        closesocket(sockfd);
                        sockfd = -1;
                    }
                }
			}
			else { perror("unable to make socket\n");}
		#else
			o->serv_addr.sin_port = htons(portnumber);
			o->addr = &(o->serv_addr);
			if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0) {
					bzero((char *)&cl_addr, sizeof(cl_addr));
				cl_addr.sin_family = AF_INET;
				cl_addr.sin_addr.s_addr = htonl(INADDR_ANY);
				cl_addr.sin_port = htons(0);
				
				// enable broadcast: jdl ~2003
				if(setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &oval, sizeof(int)) == -1) {
				  perror("setsockopt");
				}

                // check if specified address is a multicast group: ss 2007
                int multicast = IsAddressMulticast(o->serv_addr.sin_addr.s_addr);

                if (multicast == -1) {
                    perror("Multicast group range 224.0.0.[0-255] is reserved.\n");
                    *multicast_TTL = -2;
                    close(sockfd);
                    sockfd = -1;
                }
                else {
                    // check if specified address is a multicast group: ss 2007
                    if (multicast) {
                        if (*multicast_TTL<0)
                            *multicast_TTL = 1;
                    } else
                        *multicast_TTL = -1;

                    // set multicast Time-To-Live only if it is a multicast group: ss 2007
                    if(*multicast_TTL>=0) {
                        unsigned char ttl = (unsigned char)*multicast_TTL;
                        if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL,
                                       &ttl, sizeof(ttl)) == -1)
                            perror("setsockopt TTL");
                    }

                    if(bind(sockfd, (struct sockaddr *) &cl_addr, sizeof(cl_addr)) < 0) {
                        perror("could not bind\n");
                        close(sockfd);
                        sockfd = -1;
                    }
                }
			}
			else { perror("unable to make socket\n");}
		#endif
	}
	#ifdef _WIN32
		if(sockfd == INVALID_SOCKET) {
	#else
		if(sockfd < 0) {
	#endif
			free(o); 
			o = 0;
		}
		else
			o->sockfd = sockfd;
	return o;
}


#include <errno.h>

static  bool sendudp(const struct sockaddr *sp, int sockfd,int length, int count, void  *b)
{
	int rcount;
	if((rcount=sendto(sockfd, b, count, 0, sp, length)) != count)
	{
		printf("sockfd %d count %d rcount %dlength %d errno %d\n", sockfd,count,rcount,length, errno); 
		return FALSE;
	}
	return TRUE;
}
bool SendHTMSocket(void *htmsendhandle, int length_in_bytes, void *buffer)
{
	desc *o = (desc *)htmsendhandle;
	return sendudp(o->addr, o->sockfd, o->len, length_in_bytes, buffer);
}
void CloseHTMSocket(void *htmsendhandle)
{
	desc *o = (desc *)htmsendhandle;
	#ifdef _WIN32
	if(SOCKET_ERROR == closesocket(o->sockfd)) {
		perror("CloseHTMSocket::closesocket failed\n");
		return;
	}
	#else
	if(close(o->sockfd) == -1)
	  {
	    perror("CloseHTMSocket::closesocket failed");
	    return;
	  }
	#endif

	free(o);
}
