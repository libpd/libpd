/*
Written by Matt Wright and Adrian Freed, The Center for New Music and
Audio Technologies, University of California, Berkeley.  Copyright (c)
1992,93,94,95,96,97,98,99,2000,01,02,03,04 The Regents of the University of
California (Regents).

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


  /* 

     dumpOSC.c
	server that displays OpenSoundControl messages sent to it
	for debugging client udp and UNIX protocol

     by Matt Wright, 6/3/97
       modified from dumpSC.c, by Matt Wright and Adrian Freed

     version 0.2: Added "-silent" option a.k.a. "-quiet"

     version 0.3: Incorporated patches from Nicola Bernardini to make
       things Linux-friendly.  Also added ntohl() in the right places
       to support little-endian architectures.
 


	compile:
		cc -o dumpOSC dumpOSC.c

	to-do:

	    More robustness in saying exactly what's wrong with ill-formed
	    messages.  (If they don't make sense, show exactly what was
	    received.)

	    Time-based features: print time-received for each packet

	    Clean up to separate OSC parsing code from socket/select stuff

	pd: branched from http://www.cnmat.berkeley.edu/OpenSoundControl/src/dumpOSC/dumpOSC.c
	-------------
	-- added pd functions
	-- socket is made differently than original via pd mechanisms
	-- tweaks for Win32    www.zeggz.com/raf	13-April-2002
	-- the OSX changes from cnmat didnt make it here yet but this compiles
	   on OSX anyway.
	
*/

#if HAVE_CONFIG_H 
#include <config.h> 
#endif

#include "m_pd.h"
//#include "m_imp.h"
#include "s_stuff.h"

/* declarations */

// typedef void (*t_fdpollfn)(void *ptr, int fd);
//void sys_addpollfn(int fd, t_fdpollfn fn, void *ptr);


#if defined(__sgi) || defined(__linux) || defined(_WIN32) || defined(__APPLE__)

#ifdef _WIN32
   #ifdef _MSC_VER
	#include "OSC-common.h"
   #endif /* _MSC_VER */
	#include <winsock2.h>	
	#include <string.h>
	#include <stdlib.h>
	#include <fcntl.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <ctype.h>
	#include <signal.h>
	#include <stdio.h>
	#include <stdint.h>
    #include <ws2tcpip.h>
#else
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <netinet/in.h>
	#include <rpc/rpc.h>
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
	#include <sys/file.h>
	//#include <sys/prctl.h>

	#ifdef NEED_SCHEDCTL_AND_LOCK
	#include <sys/schedctl.h>
	#include <sys/lock.h>
	#endif
#endif /* _WIN32 */


char *htm_error_string;
typedef int Boolean;
typedef void *OBJ;

typedef struct ClientAddressStruct {
        struct sockaddr_in  cl_addr;
        int clilen;
        int sockfd;
} *ClientAddr;



Boolean ShowBytes = FALSE;
Boolean Silent = FALSE;

/* Declarations */
#ifndef _WIN32
static int unixinitudp(int chan);
#endif

#ifdef _WIN32
  typedef unsigned __int64 osc_time_t;
#else
  typedef unsigned long long osc_time_t;
#endif


/* 32 bit "pointer cast" union */
typedef union {
    float f;
    long i;
} ls_pcast32;


static int initudp(int chan);
static void closeudp(int sockfd);
Boolean ClientReply(int packetsize, void *packet, int socketfd, 
	void *clientaddresspointer, int clientaddressbufferlength);
void sgi_CleanExit(void);
Boolean sgi_HaveToQuit(void);
int RegisterPollingDevice(int fd, void (*callbackfunction)(int , void *), void *dummy);
static void catch_sigint();
static int Synthmessage(char *m, int n, void *clientdesc, int clientdesclength, int fd) ;
char *DataAfterAlignedString(char *string, char *boundary) ;
Boolean IsNiceString(char *string, char *boundary) ;
void complain(char *s, ...);
osc_time_t ReadTime(const char* src);

#define MAXMESG 32768
static char mbuf[MAXMESG];

/* ----------------------------- dumpOSC ------------------------- */

#define MAXOUTAT 50

static t_class *dumpOSC_class;

typedef struct _dumpOSC
{
  t_object x_obj;
  t_outlet *x_msgout;
  t_outlet *x_connectout;
  t_atom x_outat[MAXOUTAT];
  int x_outatc;
  t_binbuf *x_b;
  int x_connectsocket;
  int x_nconnections;
  int x_udp;
  struct sockaddr_in x_server;
  int x_clilen;
} t_dumpOSC;

void dumpOSC_ParsePacket(t_dumpOSC *x, char *buf, int n, ClientAddr returnAddr);
Boolean dumpOSC_SendReply(char *buf, int n, void *clientDesc, int clientDescLenght, int fd);
static void dumpOSC_Smessage(t_dumpOSC *x, char *address, void *v, int n, ClientAddr returnAddr);
static void dumpOSC_PrintTypeTaggedArgs(t_dumpOSC *x, void *v, int n);
static void dumpOSC_PrintHeuristicallyTypeGuessedArgs(t_dumpOSC *x, void *v, int n, int skipComma);

static void dumpOSC_read(t_dumpOSC *x, int sockfd) {
  int clilen = x->x_clilen;
  int n;
  struct ClientAddressStruct ras;
  ClientAddr ra = &ras;
  
  //catchupflag= FALSE;
    
/*   if (ShowBytes) { */
/*     int i; */
/*     printf("%d byte message:\n", n); */
/*     for (i = 0; i < n; ++i) { */
/*       printf(" %x (%c)\t", m[i], m[i]); */
/*       if (i%4 == 3) printf("\n"); */
/*     } */
/*     printf("\n"); */
/*   } */

    //    return catchupflag;
  //struct sockaddr_in x->x_server;
  //while( (n = recvfrom(sockfd, mbuf, MAXMESG, 0, &cl_addr, &clilen)) >0) 
  //  while((

	#ifdef _WIN32
	if ((n = recvfrom(sockfd, mbuf, MAXMESG, 0, (SOCKADDR*)&x->x_server, &clilen)) >0)
	#else
	if ((n = recvfrom(sockfd, mbuf, MAXMESG, 0, (struct sockaddr *)&x->x_server, &clilen)) >0)
	#endif 
	{
		//int r;
		ras.cl_addr = *((struct sockaddr_in *) &x->x_server);
		ras.clilen = x->x_clilen;
		ras.sockfd = x->x_connectsocket;

		#ifdef DEBUG
		printf("dumpOSC_read: received UDP packet of length %d\n",  n);
		#endif
		
		//if(!dumpOSC_SendReply(mbuf, n, &x->x_server, clilen, sockfd))
		{
			dumpOSC_ParsePacket(x, mbuf, n, ra);
		}
		//r = Synthmessage(mbuf, n, &x->x_server, clilen, sockfd);
		//post ("%d", r);
		//outlet_anything(x->x_msgout, at[msg].a_w.w_symbol,
		//		      emsg-msg-1, at + msg + 1);
		//      outlet_list(x->x_msgout, 0, n, mbuf);
		//if( sgi_HaveToQuit()) goto out;
		//if(r>0) goto back;
		//clilen = maxclilen;
	}  
}

static void *dumpOSC_new(t_symbol *compatflag,
			 int argc, t_atom* argv) {
  t_dumpOSC *x;
  struct sockaddr_in server;
  int clilen=sizeof(server);
  int sockfd;
  int portno=0;
  int udp = 1;
  t_symbol *castgroup=NULL;


  if (argc == 1) {
	  if (argv[0].a_type==A_FLOAT)
		  portno = (int)argv[0].a_w.w_float;
	  else
		  return NULL;
  }

  else if (argc == 2) {
	  if (argv[0].a_type==A_SYMBOL)
		  castgroup = argv[0].a_w.w_symbol;
	  else
		  return NULL;

	  if (argv[1].a_type==A_FLOAT)
		  portno = (int)argv[1].a_w.w_float;
	  else
		  return NULL;
  }

  else {
      error("[dumpOSC] needs at least one argument (port number) to function");
      return NULL;
  }
  

  //x->x_b = binbuf_new();
  //x->x_outat = binbuf_getvec(x->x_b);

  //{{raf}} pointer not valid yet...moving this down
  //x->x_outatc = 0;   {{raf}}

  /* create a socket */
  if ((sockfd = socket(AF_INET, (udp ? SOCK_DGRAM : SOCK_STREAM), 0)) == -1)
    {
      sys_sockerror("socket");
      return (0);
    }

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  /* assign server port number */
  server.sin_port = htons((u_short)portno);

  // ss 2006
  if (castgroup) {
	  struct ip_mreq mreq;
      int t = 1;
	  mreq.imr_multiaddr.s_addr = inet_addr(castgroup->s_name);
	  mreq.imr_interface.s_addr = htonl(INADDR_ANY);
      if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&t,sizeof(t)) < 0) {
		  sys_sockerror("setsockopt");
      }
	  if (setsockopt(sockfd,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char*)&mreq,sizeof(mreq)) < 0) {
		  sys_sockerror("setsockopt");
	  }
  }

  /* name the socket */
  if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
      sys_sockerror("bind");
      sys_closesocket(sockfd);
      return (0);
    }

  x = (t_dumpOSC *)pd_new(dumpOSC_class);
  x->x_outatc = 0;				// {{raf}} now pointer is valid (less invalid)

  x->x_msgout = outlet_new(&x->x_obj, &s_anything);
  
  // if (udp)	    /* datagram protocol */
    {
      
      sys_addpollfn(sockfd, (t_fdpollfn)dumpOSC_read, x);
      x->x_connectout = 0;
    }
  //    else    	/* streaming protocol */
  /*     { */
  /* 	if (listen(sockfd, 5) < 0) */
  /* 	{ */
  /*     	    sys_sockerror("listen"); */
  /*     	    sys_closesocket(sockfd); */
  /* 	    sockfd = -1; */
  /* 	} */
  /*     	else */
  /* 	{ */
  /* 	    sys_addpollfn(sockfd, (t_fdpollfn)dumpOSC_connectpoll, x); */
  /*     	    x->x_connectout = outlet_new(&x->x_obj, &s_float); */
  /* 	} */
  /*     } */
  
  x->x_connectsocket = sockfd;
  x->x_server = server;
  x->x_clilen = clilen;
  x->x_nconnections = 0;
  x->x_udp = udp;
  
  return (x);
}

static void dumpOSC_free(t_dumpOSC *x)
{
    	/* LATER make me clean up open connections */
    if (x->x_connectsocket >= 0)
    {
    	sys_rmpollfn(x->x_connectsocket);
    	sys_closesocket(x->x_connectsocket);
    }
}

#ifdef _MSC_VER
OSC_API void dumpOSC_setup(void)
#else
void dumpOSC_setup(void)
#endif
{
    dumpOSC_class = class_new(gensym("dumpOSC"),
    	(t_newmethod)dumpOSC_new, (t_method)dumpOSC_free,
							  sizeof(t_dumpOSC), CLASS_NOINLET, A_GIMME, 0);
    class_sethelpsymbol(dumpOSC_class, gensym("dumpOSC-help.pd"));

    logpost(NULL, 3, "[dumpOSC]: OSCx is deprecated! \n\tConsider switching to mrpeach's [unpackOSC] and [udpreceive]");
}


#ifndef _WIN32
	#define UNIXDG_PATH "/tmp/htm"
	#define UNIXDG_TMP "/tmp/htm.XXXXXX"
	static int unixinitudp(int chan)
	{
		struct sockaddr_un serv_addr;
		int  sockfd;
		
		if((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
				return sockfd;
		
		bzero((char *)&serv_addr, sizeof(serv_addr));
		serv_addr.sun_family = AF_UNIX;
		strcpy(serv_addr.sun_path, UNIXDG_PATH);
		sprintf(serv_addr.sun_path+strlen(serv_addr.sun_path), "%d", chan);
		 unlink(serv_addr.sun_path);
		if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr.sun_family)+strlen(serv_addr.sun_path)) < 0)
		{
			perror("unable to  bind\n");
			return -1;
		}

		fcntl(sockfd, F_SETFL, FNDELAY); 
		return sockfd;
	}
#endif	// #ifndef _WIN32



static int initudp(int chan)
{

#ifdef _WIN32
	struct sockaddr_in serv_addr;
	unsigned int sockfd;
	ULONG nonBlocking = (ULONG) TRUE;

	if(   (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) != INVALID_SOCKET     ) {
		ZeroMemory((char *)&serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr.sin_port = htons(chan);
		if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) >= 0) {
		//	set for non-blocking mode
			if(ioctlsocket(sockfd, FIONBIO, &nonBlocking) == SOCKET_ERROR) {
				perror("unable to set non-blocking\n"); 
				return -1;
			}
		}
		else  { perror("unable to  bind\n"); return -1; }
	}
	return (sockfd == INVALID_SOCKET ? -1 : (int)sockfd);
#else
	struct sockaddr_in serv_addr;
	int  sockfd;
	
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
			return sockfd;

	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(chan);
	
	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("unable to  bind\n");
		return -1;
	}

	fcntl(sockfd, F_SETFL, FNDELAY); 
	return sockfd;
#endif
}

static void closeudp(int sockfd) {
	#ifdef _WIN32
		closesocket(sockfd);
	#else
		close(sockfd);
	#endif
}

static Boolean catchupflag=FALSE;
Boolean ClientReply(int packetsize, void *packet, int socketfd, 
	void *clientaddresspointer, int clientaddressbufferlength)
{
	if(!clientaddresspointer) return FALSE;
	catchupflag= TRUE;
	return packetsize==sendto(socketfd, packet, packetsize, 0, clientaddresspointer, clientaddressbufferlength);
}

static Boolean exitflag= FALSE;
void sgi_CleanExit(void) {
	exitflag = TRUE;
}

Boolean sgi_HaveToQuit(void) {
	return exitflag;
}


/* file descriptor poll table */
static int npolldevs =0;
typedef struct polldev
{
	int fd;
	void (*callbackfunction)(int , void *);
	void *dummy;
} polldev;
#define TABMAX 8
static polldev polldevs[TABMAX];


/*      Register a device (referred to by a file descriptor that the caller
	should have already successfully obtained from a system call) to be
	polled as real-time constraints allowed.
        
        When a select(2) call indicates activity on the file descriptor, the
	callback function is called with the file descripter as first
	argument and the given dummy argument (presumably a pointer to the
	instance variables associated with the device).
*/
int RegisterPollingDevice(int fd, void (*callbackfunction)(int , void *), void *dummy)
{
	if(npolldevs<TABMAX)
	{
		polldevs[npolldevs].fd = fd;
		polldevs[npolldevs].callbackfunction = callbackfunction;
		polldevs[npolldevs].dummy = dummy;
	}
	else return -1;
	return npolldevs++;
}

static int caught_sigint;

static void catch_sigint()  {
   caught_sigint = 1;
}
static int sockfd, usockfd;


void PrintClientAddr(ClientAddr CA) {
    unsigned long addr = CA->cl_addr.sin_addr.s_addr;
    printf("Client address %p:\n", CA);
    printf("  clilen %d, sockfd %d\n", CA->clilen, CA->sockfd);
    printf("  sin_family %d, sin_port %d\n", CA->cl_addr.sin_family,
	   CA->cl_addr.sin_port);
    printf("  address: (%lx) %s\n", addr, inet_ntoa(CA->cl_addr.sin_addr));

    printf("  sin_zero = \"%c%c%c%c%c%c%c%c\"\n", 
	   CA->cl_addr.sin_zero[0],
	   CA->cl_addr.sin_zero[1],
	   CA->cl_addr.sin_zero[2],
	   CA->cl_addr.sin_zero[3],
	   CA->cl_addr.sin_zero[4],
	   CA->cl_addr.sin_zero[5],
	   CA->cl_addr.sin_zero[6],
	   CA->cl_addr.sin_zero[7]);

    printf("\n");
}

//*******************

/*void WriteTime(char* dst, osc_time_t osctime)
{
	*(int32_t*)dst = htonl((int32_t)(osctime >> 32));
	*(int32_t*)(dst+4) = htonl((int32_t)osctime);
}

void WriteMode(char* dst)
{
	*(int32_t*)dst = htonl(0);
}

osc_time_t ReadTime(const char* src)
{
  osc_time_t osctime = ntohl(*(int32_t*)src);
  return (osctime << 32) + ntohl(*(int32_t*)(src+4));
}

double TimeToSeconds(osc_time_t osctime)
{
  return (double)osctime * 2.3283064365386962890625e-10 // 1/2^32 ;
//       ^^^^^^^^^^^^^^^ isn't working on WIN32 (and on other machines: doesn't make sense!)
}

int timeRound(double x)
{	
	return x >= 0.0 ? x+0.5 : x-0.5;
}
*/
/*
void WriteLogicalTime(char* dst)
{
        static double startTime = -1.0;
        double sTime;
                                                                                
        // Initialisierung der Startzeit.
        // Knnte effizienter (ohne 'if') auch irgendwo vorher passieren.
        // Knnte wahrscheinlich auch 0.0 sein.
        if (startTime < 0.0) {
                startTime = clock_getlogicaltime();
        }
                                                                                
        sTime = clock_gettimesince(startTime) * 0.001;
        *(int32_t*)dst = hton'K l((int32_t)sTime);
        *(int32_t*)(dst+4) = htonl((int32_t)(4294967296.0 * sTime));
}
*/

/*void WriteLogicalTime(char* dst)
{
	double sTime = clock_gettimesince(19230720) / 1000.0;
	double tau = sTime - (double)timeRound(sTime);
	
	//fprintf(stderr, "sSec = %f tau = %f\n", sTime, tau);
	
	*(int32_t*)dst = htonl((int32_t)(sTime));
	*(int32_t*)(dst+4) = htonl((int32_t)(4294967296 * tau));
}

int dumpOSC_SendReply(char *buf, int n, void *clientDesc, int clientDescLenght, int fd)
{
	if((n == 24) && (strcmp(buf, "#time") == 0))
	{		
		osc_time_t t0, t1, t2;
		double dt0, dt1, dt2;		

		WriteMode(buf+6);
		
		t0 = ReadTime(buf+8);		
		    
		WriteLogicalTime(buf+16);
		t1 = ReadTime(buf+16); // reverse
		dt0 = TimeToSeconds(t0); // client time
		dt1 = TimeToSeconds(t1); // server time	
		
		//		fprintf(stderr, "%f\t%f\t%f\n", dt0, dt1, dt0 - dt1);

		sendto(fd, buf, n, 0, (struct sockaddr *)clientDesc, clientDescLenght);		
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}*/

//**********************

void dumpOSC_ParsePacket(t_dumpOSC *x, char *buf, int n, ClientAddr returnAddr) {
  //  t_dumpOSC *x;
  int size, messageLen, i;
  char *messageName;
  char *args;
  
  //#ifdef PRINTADDRS
  #ifdef DEBUG
	//PrintClientAddr(returnAddr);
  #endif


  if ((n%4) != 0) {
    complain("SynthControl packet size (%d) not a multiple of 4 bytes: dropping", n);
    return;
  }

  if ((n >= 8) && (strncmp(buf, "#bundle", 8) == 0)) {
		/* This is a bundle message. */
		#ifdef DEBUG
			printf("dumpOSC_ParsePacket: bundle msg: bundles not yet supported\n");
		#endif

		if (n < 16) {
		  complain("Bundle message too small (%d bytes) for time tag", n);
		  return;
		}

		/* Print the time tag */
		#ifdef DEBUG
			printf("[ %lx%08lx\n", ntohl(*((unsigned long *)(buf+8))), ntohl(*((unsigned long *)(buf+12))));
		#endif

		/* Note: if we wanted to actually use the time tag as a little-endian
		   64-bit int, we'd have to word-swap the two 32-bit halves of it */

		i = 16; /* Skip "#group\0" and time tag */

		while(i<n) {
			size = ntohl(*((int *) (buf + i)));
			if ((size % 4) != 0) {
				complain("Bad size count %d in bundle (not a multiple of 4)", size);
				return;
			}
			if ((size + i + 4) > n) {
				complain("Bad size count %d in bundle (only %d bytes left in entire bundle)",
				size, n-i-4);
				return;	
			}

			/* Recursively handle element of bundle */
			dumpOSC_ParsePacket(x, buf+i+4, size, returnAddr);
			i += 4 + size;
		}

		if (i != n) {
			complain("This can't happen");
		}
		#ifdef DEBUG
			printf("]\n");
		#endif

  } 
  else if ((n == 24) && (strcmp(buf, "#time") == 0))
  {
 		complain("Time message: %s\n :).\n", htm_error_string);
		return; 	

  }
  else
  {
		/* This is not a bundle message */

		messageName = buf;
		args = DataAfterAlignedString(messageName, buf+n);
		if (args == 0) {
			complain("Bad message name string: %s\nDropping entire message.\n",
			htm_error_string);
			return;
		}
		messageLen = args-messageName;
		dumpOSC_Smessage(x, messageName, (void *)args, n-messageLen, returnAddr);
  }
}

#define SMALLEST_POSITIVE_FLOAT 0.000001f

static void dumpOSC_Smessage(t_dumpOSC *x, char *address, void *v, int n, ClientAddr returnAddr) {
  char *chars = v;
  //t_atom at;
  //t_atom myargv[50];

  //int myargc = x->x_outatc;
  //t_atom* mya = x->x_outat;
  //int myi;

#ifdef DEBUG
  printf("%s ", address);
#endif

  // ztoln+cvt from envgen.c, ggee-0.18 ..
  // outlet_anything's 'symbol' gets set to address
  // so we dont need to append address to the atomlist
  /*
    SETSYMBOL(mya,gensym(address));myargc++;
    x->x_outatc = myargc;
  */

  if (n != 0) {
    if (chars[0] == ',') {
      if (chars[1] != ',') {
	/* This message begins with a type-tag string */
	dumpOSC_PrintTypeTaggedArgs(x, v, n);
      } else {
	/* Double comma means an escaped real comma, not a type string */
	dumpOSC_PrintHeuristicallyTypeGuessedArgs(x, v, n, 1);
      }
    } else {
      dumpOSC_PrintHeuristicallyTypeGuessedArgs(x, v, n, 0);
    }
  }

  outlet_anything(x->x_msgout,gensym(address),x->x_outatc,(t_atom*)&x->x_outat);
  x->x_outatc = 0;
#ifdef DEBUG
  printf("\n");
#endif
  fflush(stdout);	/* Added for Sami 5/21/98 */
}

static void dumpOSC_PrintTypeTaggedArgs(t_dumpOSC *x, void *v, int n) { 
  char *typeTags, *thisType;
  char *p;
  
  int myargc = x->x_outatc;
  t_atom* mya = x->x_outat;
  //int myi;

  typeTags = v;
  
  if (!IsNiceString(typeTags, typeTags+n)) {
    /* No null-termination, so maybe it wasn't a type tag
       string after all */
    dumpOSC_PrintHeuristicallyTypeGuessedArgs(x, v, n, 0);
    return;
  }

  p = DataAfterAlignedString(typeTags, typeTags+n);
  

  for (thisType = typeTags + 1; *thisType != 0; ++thisType) {
    switch (*thisType) {
    case 'i': case 'r': case 'm': case 'c':
#ifdef DEBUG
      //post("integer: %d", ntohl(*((int *) p)));
#endif
      /* Martin Peach fix for negative floats:
	   * was: SETFLOAT(mya+myargc,ntohl(*((int *) p))); 
	   * now is: 
	   */
      SETFLOAT(mya+myargc,(signed)ntohl(*((int *) p)));
      myargc++;

      p += 4;
      break;

    case 'f': {
      long i = ntohl(*((long *) p));
      ls_pcast32 *pc = (ls_pcast32 *)(&i);
#ifdef DEBUG
      post("float: %f", (*pc).f);
#endif
      SETFLOAT(mya+myargc, (*pc).f);
      myargc++;

      p += 4;
    }
    break;

    case 'h': case 't':
#ifdef DEBUG
      printf("[A 64-bit int] ");
#endif
      post("[A 64-bit int] not implemented");

      p += 8;
      break;

    case 'd':
#ifdef DEBUG
      printf("[A 64-bit float] ");
#endif
      post("[A 64-bit float] not implemented");

      p += 8;
      break;

    case 's': case 'S':
      if (!IsNiceString(p, typeTags+n)) {
	post("Type tag said this arg is a string but it's not!\n");
	return;
      } else {
#ifdef DEBUG
	post("string: \"%s\"", p);
#endif
	SETSYMBOL(mya+myargc,gensym(p));
	myargc++;
	//outlet_list(x->x_msgout, 0,sizeof(p), p);
	//outlet_anything(x->x_msgout, 0, sizeof(p), p);
	p = DataAfterAlignedString(p, typeTags+n);
	// append to output vector ..
      }
      break;

    case 'T':
#ifdef DEBUG
      printf("[True] ");
#endif
      SETFLOAT(mya+myargc,1.);
      myargc++;
      break;
    case 'F':
#ifdef DEBUG
      printf("[False] ");
#endif
      SETFLOAT(mya+myargc,0.);
      myargc++;
      break;
    case 'N':
#ifdef DEBUG
      printf("[Nil]");
#endif
      post("sendOSC: [Nil] not implemented");
      break;
    case 'I':
#ifdef DEBUG
      printf("[Infinitum]");
#endif
      post("sendOSC: [Infinitum] not implemented");
      break;

    default:
      post("sendOSC: [Unrecognized type tag %c]", *thisType);
      // return;
    }
  }
  x->x_outatc = myargc;
}

static void dumpOSC_PrintHeuristicallyTypeGuessedArgs(t_dumpOSC *x, void *v, int n, int skipComma) {
  int i, thisi;
  t_float thisf;
  int *ints;
  char *chars;
  char *string, *nextString;
	
  int myargc= x->x_outatc;
  t_atom* mya = x->x_outat;
  //int myi;

  union
  {
      float f;
      int32_t i;
  }u;

  /* Go through the arguments 32 bits at a time */
  ints = v;
  chars = v;

  for (i = 0; i<n/4; ) {
    string = &chars[i*4];
    u.i = thisi = ntohl(ints[i]);
    /* Reinterpret the (potentially byte-reversed) thisi as a float */
    thisf = (t_float) u.f;

    if  (thisi >= -1000 && thisi <= 1000000) {
#ifdef DEBUG
      printf("%d ", thisi);
#endif
      // append to output vector ..
      SETFLOAT(mya+myargc,(t_float) (thisi));
      myargc++;
      //      outlet_float(x->x_msgout, thisi);
      i++;
    } else if (thisf >= -1000.f && thisf <= 1000000.f &&
	       (thisf <=0.0f || thisf >= SMALLEST_POSITIVE_FLOAT)) {
#ifdef DEBUG
      printf("%f ",  thisf);
#endif
      // append to output vector ..
      SETFLOAT(mya+myargc,thisf);
      myargc++;
      //outlet_float(x->x_msgout, thisf);
      i++;
    } else if (IsNiceString(string, chars+n)) {
      nextString = DataAfterAlignedString(string, chars+n);
#ifdef DEBUG
      printf("\"%s\" ", (i == 0 && skipComma) ? string +1 : string);
#endif
      // append to output vector ..
      SETSYMBOL(mya+myargc,gensym(string));
      myargc++;
      //outlet_symbol(x->x_msgout, gensym((i == 0 && skipComma) ? string +1 : string));
      i += (nextString-string) / 4;
    } else {
      // unhandled .. ;)
#ifdef DEBUG
      printf("0x%x xx", ints[i]);
#endif
      i++;
    }
    x->x_outatc = myargc;
  }
}


#define STRING_ALIGN_PAD 4

char *DataAfterAlignedString(char *string, char *boundary) 
{
    /* The argument is a block of data beginning with a string.  The
       string has (presumably) been padded with extra null characters
       so that the overall length is a multiple of STRING_ALIGN_PAD
       bytes.  Return a pointer to the next byte after the null
       byte(s).  The boundary argument points to the character after
       the last valid character in the buffer---if the string hasn't
       ended by there, something's wrong.

       If the data looks wrong, return 0, and set htm_error_string */

    int i;

    if ((boundary - string) %4 != 0) {
	fprintf(stderr, "Internal error: DataAfterAlignedString: bad boundary\n");
	return 0;
    }

    for (i = 0; string[i] != '\0'; i++) {
	if (string + i >= boundary) {
	    htm_error_string = "DataAfterAlignedString: Unreasonably long string";
	    return 0;
	}
    }

    /* Now string[i] is the first null character */
    i++;

    for (; (i % STRING_ALIGN_PAD) != 0; i++) {
	if (string + i >= boundary) {
	    htm_error_string = "DataAfterAlignedString: Unreasonably long string";
	    return 0;
	}
	if (string[i] != '\0') {
	    htm_error_string = "DataAfterAlignedString: Incorrectly padded string.";
	    return 0;
	}
    }

    return string+i;
}

Boolean IsNiceString(char *string, char *boundary) 
{
    /* Arguments same as DataAfterAlignedString().  Is the given "string"
       really a string?  I.e., is it a sequence of isprint() characters
       terminated with 1-4 null characters to align on a 4-byte boundary? */

    int i;

    if ((boundary - string) %4 != 0) {
	fprintf(stderr, "Internal error: IsNiceString: bad boundary\n");
	return 0;
    }

    for (i = 0; string[i] != '\0'; i++) {
	if (!isprint(string[i])) return FALSE;
	if (string + i >= boundary) return FALSE;
    }

    /* If we made it this far, it's a null-terminated sequence of printing characters 
       in the given boundary.  Now we just make sure it's null padded... */

    /* Now string[i] is the first null character */
    i++;
    for (; (i % STRING_ALIGN_PAD) != 0; i++) {
	if (string[i] != '\0') return FALSE;
    }

    return TRUE;
}









#include <stdarg.h>
void complain(char *s, ...) {
    va_list ap;
    va_start(ap, s);
    fprintf(stderr, "*** ERROR: ");
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}

#endif /* __sgi or LINUX or _WIN32 */
