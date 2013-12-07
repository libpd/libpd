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


The OSC webpage is http://cnmat.cnmat.berkeley.edu/OpenSoundControl
*/



/* sendOSC.c

    Matt Wright, 6/3/97
    based on sendOSC.c, which was based on a version by Adrian Freed

    Text-based OpenSoundControl client.  User can enter messages via command
    line arguments or standard input.

    Version 0.1: "play" feature
    Version 0.2: Message type tags.
   
    pd version branched from http://www.cnmat.berkeley.edu/OpenSoundControl/src/sendOSC/sendOSC.c
    -------------
    -- added bundle stuff to send. jdl 20020416
    -- tweaks for Win32    www.zeggz.com/raf	13-April-2002
    -- ost_at_test.at + i22_at_test.at, 2000-2002
       modified to compile as pd externel
    --20060308 MP clear out some unused code and improve conversion from pd types to OSC types
*/

#if HAVE_CONFIG_H 
#include <config.h> 
#endif

#define MAX_ARGS 2000
#define SC_BUFFER_SIZE 64000

#include "m_pd.h"

#include "OSC-common.h"
#include "OSC-client.h"
#include "htmsocket.h"

#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <winsock2.h>	
#else
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#endif

///////////////////////
// from sendOSC

typedef struct
{
  enum {INT_osc, FLOAT_osc, STRING_osc, NOTYPE_osc} type;
  union
  {
        int i;
        t_float f;
        char *s;
    } datum;
} typedArg;


//static int exitStatus = 0;  
static int useTypeTags = 0;

static char bufferForOSCbuf[SC_BUFFER_SIZE];


/////////
// end from sendOSC

static t_class *sendOSC_class;

typedef struct _sendOSC
{
  t_object x_obj;
  int x_protocol;      // UDP/TCP (udp only atm)
  t_int x_typetags;    // typetag flag
  void *x_htmsocket;   // sending socket
  int x_bundle;        // bundle open flag
  OSCbuf x_oscbuf[1];  // OSCbuffer
  t_outlet *x_bdpthout;// bundle-depth floatoutlet
} t_sendOSC;

static void *sendOSC_new(t_floatarg udpflag);
void sendOSC_openbundle(t_sendOSC *x);
static void sendOSC_closebundle(t_sendOSC *x);
static void sendOSC_settypetags(t_sendOSC *x, t_float f);
static void sendOSC_connect(t_sendOSC *x, t_symbol *s, int argc, t_atom *argv);
void sendOSC_disconnect(t_sendOSC *x);
static void sendOSC_sendtyped(t_sendOSC *x, t_symbol *s, int argc, t_atom *argv);
void sendOSC_send(t_sendOSC *x, t_symbol *s, int argc, t_atom *argv);
static void sendOSC_free(t_sendOSC *x);
void sendOSC_setup(void);
typedArg ParseAtom(t_atom *a);
int WriteMessage(OSCbuf *buf, char *messageName, int numArgs, typedArg *args);
void SendBuffer(void *htmsocket, OSCbuf *buf);
void SendData(void *htmsocket, int size, char *data);

static void *sendOSC_new(t_floatarg udpflag)
{
    t_sendOSC *x = (t_sendOSC *)pd_new(sendOSC_class);

    outlet_new(&x->x_obj, &s_float);
    x->x_htmsocket = 0;		// {{raf}}
    // set udp
  x->x_protocol = SOCK_DGRAM; ////MP20060308// not SOCK_STREAM but we don't use it anyway...
    // set typetags to 1 by default
    x->x_typetags = 1;
  // bundle is closed
    x->x_bundle   = 0;
    OSC_initBuffer(x->x_oscbuf, SC_BUFFER_SIZE, bufferForOSCbuf);
    x->x_bdpthout = outlet_new(&x->x_obj, 0); // outlet_float();
    //x->x_oscbuf   =
    return (x);
}


void sendOSC_openbundle(t_sendOSC *x)
{
  if (x->x_oscbuf->bundleDepth + 1 >= MAX_BUNDLE_NESTING ||
      OSC_openBundle(x->x_oscbuf, OSCTT_Immediately()))
    {
    error("Problem opening bundle: %s\n", OSC_errorMessage);
    return;
  }
  x->x_bundle = 1;
  outlet_float(x->x_bdpthout, (t_float)x->x_oscbuf->bundleDepth);
}

static void sendOSC_closebundle(t_sendOSC *x)
{
  if (OSC_closeBundle(x->x_oscbuf))
  {
    error("Problem closing bundle: %s\n", OSC_errorMessage);
    return;
  }
  outlet_float(x->x_bdpthout, (t_float)x->x_oscbuf->bundleDepth);
  // in bundle mode we send when bundle is closed?
  if(!OSC_isBufferEmpty(x->x_oscbuf) > 0 && OSC_isBufferDone(x->x_oscbuf))
  {
    // post("x_oscbuf: something inside me?");
    if (x->x_htmsocket)
	{
      SendBuffer(x->x_htmsocket, x->x_oscbuf);
    }
	else
	{
      post("sendOSC: not connected");
    }
    OSC_initBuffer(x->x_oscbuf, SC_BUFFER_SIZE, bufferForOSCbuf);
    x->x_bundle = 0;
    return;
  }
  // post("x_oscbuf: something went wrong");
}

static void sendOSC_settypetags(t_sendOSC *x, t_float f)
 {
   x->x_typetags = (int)f;
   post("sendOSC.c: setting typetags %d",x->x_typetags);
 }

static void sendOSC_connect(t_sendOSC *x, t_symbol *s, int argc, t_atom *argv) // t_symbol *hostname, t_floatarg fportno, int argc, t_atom *argv)
{
  t_float fportno=0;
  t_symbol *hostname;
  int portno = fportno;
  short ttl=-1;
  char *protocolStr;
  /* create a socket */

  if (argc < 2)
	  return;

  if (argv[0].a_type==A_SYMBOL)
	  hostname = argv[0].a_w.w_symbol;
  else
	  return;

  if (argv[1].a_type==A_FLOAT)
	  portno = (int)argv[1].a_w.w_float;
  else
	  return;

  if (argc >= 3) {
	  if (argv[2].a_type==A_FLOAT)
		  ttl = (short)(unsigned char)argv[2].a_w.w_float;
	  else
		  return;
  }

	//	make sure handle is available
  if(x->x_htmsocket == 0)
  {
		x->x_htmsocket = OpenHTMSocket(hostname->s_name, portno, &ttl);
		if (!x->x_htmsocket) {
            post("sendOSC: Couldn't open socket: ");
            if (ttl==-2)
                post("sendOSC: Multicast group range 224.0.0.[0-255] is reserved.\n");
        }
    else
	{
      switch (x->x_protocol)
      {
        case SOCK_DGRAM:
          protocolStr = "UDP";
          break;
        case SOCK_STREAM:
          protocolStr = "TCP";
          break;
        default:
          protocolStr = "unknown";
          break;
	  }
      if (ttl>=0)
          post("sendOSC: connected to port %s:%d (hSock=%d) protocol = %s ttl = %d", 
               hostname->s_name, portno, x->x_htmsocket, protocolStr, ttl);
      else
          post("sendOSC: connected to port %s:%d (hSock=%d) protocol = %s", 
               hostname->s_name, portno, x->x_htmsocket, protocolStr);
      outlet_float(x->x_obj.ob_outlet, 1);
    }
  }
  else 
    perror("call to sendOSC_connect() against unavailable socket handle");
}

void sendOSC_disconnect(t_sendOSC *x)
{
  if (x->x_htmsocket)
    {
    post("sendOSC: disconnecting htmsock (hSock=%d)...", x->x_htmsocket);
      CloseHTMSocket(x->x_htmsocket);
	  x->x_htmsocket = 0;	// {{raf}}  semi-quasi-semaphorize this
      outlet_float(x->x_obj.ob_outlet, 0);
    }
  else
  {
	perror("call to sendOSC_disconnect() against unused socket handle");
  }
}


//////////////////////////////////////////////////////////////////////
// this is the real and only sending routine now, for both typed and
// undtyped mode.

static void sendOSC_sendtyped(t_sendOSC *x, t_symbol *s, int argc, t_atom *argv)
{
  char messageName[MAXPDSTRING];
  //  char *token;
  typedArg args[MAX_ARGS];
  int i;

  messageName[0] = '\0'; // empty

  if(argc>MAX_ARGS) 
  {
    post ("sendOSC: too many arguments! (max: %d)", MAX_ARGS);
	return;
  }

  // this sock needs to be larger than 0, not >= ..
  if (x->x_htmsocket > 0)
    { 
#ifdef DEBUG
    post ("sendOSC: type tags? %d", useTypeTags);
#endif 

    atom_string(&argv[0], messageName, MAXPDSTRING);
//    messageName = strtok(targv[0], ",");
    for (i = 0; i < argc-1; i++)
    {
//      token = strtok(targv[i],",");

      args[i] = ParseAtom(&argv[i+1]);
#ifdef DEBUG
      switch (args[i].type)
	  {
        case INT_osc:
          printf("cell-cont: %d\n", args[i].datum.i);
          break;
        case FLOAT_osc:
          printf("cell-cont: %f\n", args[i].datum.f);
          break;
        case STRING_osc:
          printf("cell-cont: %s\n", args[i].datum.s);
          break;
        case NOTYPE_osc:
          printf("unknown type\n");
          break;
      }
      printf("  type-id: %d\n", args[i].type);
#endif
      }
      
    if(WriteMessage(x->x_oscbuf, messageName, i, args))
	{
		post("sendOSC: usage error, write-msg failed: %s", OSC_errorMessage);
		return;
      }
      
    if(!x->x_bundle)
	{
		SendBuffer(x->x_htmsocket, x->x_oscbuf);
		OSC_initBuffer(x->x_oscbuf, SC_BUFFER_SIZE, bufferForOSCbuf);
      }
      
    }
  else post("sendOSC: not connected");
}

void sendOSC_send(t_sendOSC *x, t_symbol *s, int argc, t_atom *argv) 
{
  if(!argc)
  {
    post("not sending empty message.");
    return;
  }
  if(x->x_typetags)
  {
    useTypeTags = 1;
    sendOSC_sendtyped(x,s,argc,argv);
    useTypeTags = 0;
  }
  else
  {
    sendOSC_sendtyped(x,s,argc,argv);
  }
}

static void sendOSC_free(t_sendOSC *x)
{
    sendOSC_disconnect(x);
}

void sendOSC_setup(void)
{ 
    sendOSC_class = class_new(gensym("sendOSC"), (t_newmethod)sendOSC_new,
			      (t_method)sendOSC_free,
			      sizeof(t_sendOSC), 0, A_DEFFLOAT, 0);
    class_addmethod(sendOSC_class, (t_method)sendOSC_connect,
		    gensym("connect"), A_GIMME, 0);
    class_addmethod(sendOSC_class, (t_method)sendOSC_disconnect,
		    gensym("disconnect"), 0);
    class_addmethod(sendOSC_class, (t_method)sendOSC_settypetags,
		    gensym("typetags"),
		    A_FLOAT, 0);
    class_addmethod(sendOSC_class, (t_method)sendOSC_send,
		    gensym("send"),
		    A_GIMME, 0);
    class_addmethod(sendOSC_class, (t_method)sendOSC_send,
		    gensym("senduntyped"),
		    A_GIMME, 0);
    class_addmethod(sendOSC_class, (t_method)sendOSC_send,
		    gensym("sendtyped"),
		    A_GIMME, 0);
    class_addmethod(sendOSC_class, (t_method)sendOSC_openbundle,
		    gensym("["),
		    0, 0);
    class_addmethod(sendOSC_class, (t_method)sendOSC_closebundle,
		    gensym("]"),
		    0, 0);
    class_sethelpsymbol(sendOSC_class, gensym("sendOSC-help.pd"));
    logpost(NULL, 3, "[sendOSC]: OSCx is deprecated! \n\tConsider switching to mrpeach's [packOSC] and [udpsend]");
}

/* Exit status codes:
    0: successful
    2: Message(s) dropped because of buffer overflow
    3: Socket error
    4: Usage error
    5: Internal error
*/


typedArg ParseAtom(t_atom *a)
{
  typedArg returnVal;
  t_float f;
  t_int i;
  t_symbol s;
  char buf[MAXPDSTRING];

  atom_string(a, buf, MAXPDSTRING);
#ifdef DEBUG
  post("sendOSC: atom type %d (%s)", a->a_type, buf);
#endif
    /* It might be an int, a float, or a string */
  switch (a->a_type)
  {

    case A_FLOAT:
      f = atom_getfloat(a);
	  i = atom_getint(a);
	  if (f == (t_float)i)
	  { // assume that if the int and float are the same, it's an int
            returnVal.type = INT_osc;
        returnVal.datum.i = i;
        }
	  else
	  {
                returnVal.type = FLOAT_osc;
        returnVal.datum.f = f;
        }
	  return returnVal;
    case A_SYMBOL:
	  s = *atom_getsymbol(a);
    returnVal.type = STRING_osc;
	  returnVal.datum.s = s.s_name;
      return returnVal;
    default:
      atom_string(a, buf, MAXPDSTRING);
      error("sendOSC: atom type %d not implemented (%s)", a->a_type, buf);
      returnVal.type = NOTYPE_osc;
	  returnVal.datum.s = NULL;
    return returnVal;
}
}

int WriteMessage(OSCbuf *buf, char *messageName, int numArgs, typedArg *args)
{
  int j, returnVal;

  returnVal = 0;

#ifdef DEBUG
  printf("WriteMessage: %s ", messageName);

  for (j = 0; j < numArgs; j++)
  {
    switch (args[j].type)
	{
    case INT_osc:
      printf("%d ", args[j].datum.i);
      break;
      
    case FLOAT_osc:
      printf("%f ", args[j].datum.f);
      break;
      
    case STRING_osc:
      printf("%s ", args[j].datum.s);
      break;
      
    default:
        error("Unrecognized arg type %d", args[j].type);
        break;
    }
  }
  printf("\n");
#endif
  
  if (!useTypeTags)
  {
    returnVal = OSC_writeAddress(buf, messageName);
    if (returnVal)
	{
      error("Problem writing address: %s\n", OSC_errorMessage);
    }
    }
  else
  {
    /* First figure out the type tags */
    char typeTags[MAX_ARGS+2];
    
    typeTags[0] = ',';
    
    for (j = 0; j < numArgs; ++j)
	{
      switch (args[j].type)
	  {
      case INT_osc:
          typeTags[j+1] = 'i';
	break;
	
      case FLOAT_osc:
          typeTags[j+1] = 'f';
	break;
	
      case STRING_osc:
          typeTags[j+1] = 's';
	break;
	
      default:
          error("sendOSC: arg %d type is unrecognized(%d)", j, args[j].type);
          break;
      }
    }
    typeTags[j+1] = '\0';
    
    returnVal = OSC_writeAddressAndTypes(buf, messageName, typeTags);
    if (returnVal)
    {
      error("Problem writing address: %s\n", OSC_errorMessage);
    }
  }

  for (j = 0; j < numArgs; j++)
  {
    switch (args[j].type)
    {
    case INT_osc:
        if ((returnVal = OSC_writeIntArg(buf, args[j].datum.i)) != 0)
        {
	return returnVal;
      }
      break;
      
    case FLOAT_osc:
        if ((returnVal = OSC_writeFloatArg(buf, args[j].datum.f)) != 0)
        {
	return returnVal;
      }
      break;
      
    case STRING_osc:
        if ((returnVal = OSC_writeStringArg(buf, args[j].datum.s)) != 0)
        {
	return returnVal;
      }
      break;
      
    default:
        break; // just skip bad types (which we won't get anyway unless this code is buggy)
    }
  }
  return returnVal;
}

void SendBuffer(void *htmsocket, OSCbuf *buf)
{
#ifdef DEBUG
  printf("Sending buffer...\n");
#endif
  if (OSC_isBufferEmpty(buf))
  {
		post("SendBuffer() called but buffer empty");
		return;
  }
  if (!OSC_isBufferDone(buf))
  {
		fatal_error("SendBuffer() called but buffer not ready!, not exiting");
		return;	//{{raf}}
  }
  SendData(htmsocket, OSC_packetSize(buf), OSC_getPacket(buf));
}

void SendData(void *htmsocket, int size, char *data)
{
  if (!SendHTMSocket(htmsocket, size, data))
  {
    post("SendData::SendHTMSocket()failure -- not connected");
    CloseHTMSocket(htmsocket);
}
}

