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

 /* OSC-route.c
  Max object for OSC-style dispatching

  To-do:

  	Match a pattern against a pattern?
      [Done: Only Slash-Star is allowed, see MyPatternMatch.]
  	Declare outlet types / distinguish leaf nodes from other children
  	More sophisticated (2-pass?) allmessages scheme
  	set message?


	pd
	-------------
		-- tweaks for Win32    www.zeggz.com/raf	13-April-2002


  */

#if HAVE_CONFIG_H 
#include <config.h> 
#endif


/* the required include files */
#include "m_pd.h"
#include "OSC-common.h"
#include "OSC-pattern-match.h"

#include <stdio.h>
#ifdef _WIN32
	#include <stdlib.h>
	#include <string.h>
#endif

/* structure definition of your object */
#define MAX_NUM 128
#define OSC_ROUTE_VERSION "1.05"
/* Version 1.04: Allows #1 thru #9 as typed-in arguments
   Version 1.05: Allows "list" messages as well as "message" messages.
*/

static Boolean MyPatternMatch (const char *pattern, const char *test)
{
    // This allows the special case of "OSCroute /* " to be an outlet that
    // matches anything; i.e., it always outputs the input with the first level
    // of the address stripped off. 
    
    if (test[0] == '*' && test[1] == '\0') {
        return 1;
    } else {
        return PatternMatch(pattern, test);
    }
}


static t_class *OSCroute_class;

typedef struct _OSCroute
{
  t_object x_obj;			// required header
  t_int x_num;				// Number of address prefixes we store
  t_int x_complainmode;			// Do we print a message if no match?
  t_int x_sendmode;                     // use pd internal sends instead of outlets
  char *x_prefixes[MAX_NUM];
  void *x_outlets[MAX_NUM+1];
} t_OSCroute;

t_symbol *ps_list, *ps_complain, *ps_emptySymbol;

/* prototypes  */

void OSCroute_doanything(t_OSCroute *x, t_symbol *s, int argc, t_atom *argv);
void OSCroute_anything(t_OSCroute *x, t_symbol *s, int argc, t_atom *argv);
void OSCroute_list(t_OSCroute *x, t_symbol *s, int argc, t_atom *argv);
/* //void *OSCroute_new(t_symbol *s, int argc, atom *argv); */
void *OSCroute_new(t_symbol *s, int argc, t_atom *argv);
void OSCroute_version (t_OSCroute *x);
/* void OSCroute_assist (OSCroute *x, void *box, long msg, long arg,  */
/* 		      char *dstString); */
void OSCroute_allmessages(t_OSCroute *x, t_symbol *s, int argc, t_atom *argv);
#ifdef _MSC_VER
OSC_API void OSCroute_setup(void);
#else
void OSCroute_setup(void);
#endif /* _MSC_VER */

static char *NextSlashOrNull(char *p);
static void StrCopyUntilSlash(char *target, const char *source);


// free
static void OSCroute_free(t_OSCroute *x)
{
  //    freebytes(x->x_vec, x->x_nelement * sizeof(*x->x_vec));
}

/* initialization routine */

// setup
#ifdef _MSC_VER
  OSC_API void OSCroute_setup(void) { 
#else
void OSCroute_setup(void) {
#endif
  OSCroute_class = class_new(gensym("OSCroute"), (t_newmethod)OSCroute_new,
			     (t_method)OSCroute_free,sizeof(t_OSCroute), 0, A_GIMME, 0);
  class_addlist(OSCroute_class, OSCroute_list);
  class_addanything(OSCroute_class, OSCroute_anything);
  class_addmethod(OSCroute_class, (t_method)OSCroute_version, gensym("version"), A_NULL, 0, 0);
  class_sethelpsymbol(OSCroute_class, gensym("OSCroute-help.pd"));

  /*
  class_addmethod(OSCroute_class, (t_method)OSCroute_connect,
		  gensym("connect"), A_SYMBOL, A_FLOAT, 0);
  class_addmethod(OSCroute_class, (t_method)OSCroute_disconnect,
		  gensym("disconnect"), 0);
  class_addmethod(OSCroute_class, (t_method)OSCroute_send, gensym("send"),
		  A_GIMME, 0);
  */
/*   ps_list = gensym("list"); */
/*   ps_complain = gensym("complain"); */
  ps_emptySymbol = gensym("");
  
  post("OSCroute object version " OSC_ROUTE_VERSION " by Matt Wright. pd: jdl Win32 raf.");
  post("OSCroute Copyright © 1999 Regents of the Univ. of California. All Rights Reserved.");
  logpost(NULL, 3, "[OSCroute]: OSCx is deprecated! \n\tConsider switching to mrpeach's [routeOSC]");
}



/* instance creation routine */

void *OSCroute_new(t_symbol *s, int argc, t_atom *argv)
{

  t_OSCroute *x = (t_OSCroute *)pd_new(OSCroute_class);   // get memory for a new object & initialize

  int i;	//{{raf}} n not used
  
  // EnterCallback();

  if (argc > MAX_NUM) {
    post("* OSC-route: too many arguments: %ld (max %ld)", argc, MAX_NUM);
    // ExitCallback();
    return 0;
  }

  x->x_complainmode = 0;
  x->x_num = 0;
  for (i = 0; i < argc; ++i) {
    if (argv[i].a_type == A_SYMBOL) {
      if (argv[i].a_w.w_symbol->s_name[0] == '/') {
	/* Now that's a nice prefix */
	x->x_prefixes[i] = argv[i].a_w.w_symbol->s_name;
	++(x->x_num);
      } else if (argv[i].a_w.w_symbol->s_name[0] == '#' &&
		 argv[i].a_w.w_symbol->s_name[1] >= '1' &&
		 argv[i].a_w.w_symbol->s_name[1] <= '9') {
	/* The Max programmer is trying to make a patch that will be
	   a subpatch with arguments.  We have to make an outlet for this
	   argument. */
	x->x_prefixes[i] = "dummy";
	++(x->x_num);
      } else {
	/* Maybe this is an option we support */

/* 	if (argv[i].a_w.w_sym == ps_complain) { */
/* 	  x->x_complainmode = 1; */
/* 	} else { */
/* 	  post("* OSC-route: Unrecognized argument %s", argv[i].a_w.w_sym->s_name); */
/* 	} */

      }

      // no LONG

/*     } else if (argv[i].a_type == A_FLOAD) { */
/*       // Convert to a numeral.  Max ints are -2147483648 to 2147483647 */
/*       char *string = getbytes(12); */
/*       // I can't be bothered to plug this 12 byte memory leak */
/*       if (string == 0) { */
/* 	post("* OSC-route: out of memory!"); */
/* 	// ExitCallback(); */
/* 	return 0; */
/*       } */
/*       sprintf(string, "%d", argv[i].a_w.w_long); */
/*       x->x_prefixes[i] = string; */
/*       ++(x->x_num); */

    } else if (argv[i].a_type == A_FLOAT) {
      post("* OSC-route: float arguments are not OK.");
      // ExitCallback();
      return 0;
    } else {
      post("* OSC-route: unrecognized argument type!");
      // ExitCallback();
      return 0;
    }
  }
  
  
  /* Have to create the outlets in reverse order */
  /* well, not in pd ? */
  //  for (i = x->x_num-1; i >= 0; --i) {
  // for (i = 0; i <= x->x_num-1; i++) {
  for (i = 0; i <= x->x_num; i++) {
    //    x->x_outlets[i] = listout(x);
    x->x_outlets[i] = outlet_new(&x->x_obj, &s_list);
  }
  
  // ExitCallback();
  return (x);
}


void OSCroute_version (t_OSCroute *x) {
  // EnterCallback();
  post("OSCroute Version " OSC_ROUTE_VERSION
       ", by Matt Wright. pd jdl, win32: raf.\nOSCroute Compiled " __TIME__ " " __DATE__);
  // ExitCallback();
}

/* I don't know why these aren't defined in some Max #include file. */
#define ASSIST_INLET 1
#define ASSIST_OUTLET 2

void OSCroute_assist (t_OSCroute *x, void *box, long msg, long arg, 
		      char *dstString) {
  // EnterCallback();
  
  if (msg==ASSIST_INLET) {
    sprintf(dstString, "Incoming OSC messages");
  } else if (msg==ASSIST_OUTLET) {
    if (arg < 0 || arg >= x->x_num) {
      post("* OSCroute_assist: No outlet corresponds to arg %ld!", arg);
    } else {
      sprintf(dstString, "subaddress + args for prefix %s", x->x_prefixes[arg]);
    }
  } else {
    post("* OSCroute_assist: unrecognized message %ld", msg);
  }
  
  // ExitCallback();
}

void OSCroute_list(t_OSCroute *x, t_symbol *s, int argc, t_atom *argv) {
  // EnterCallback();
  if (argc > 0 && argv[0].a_type == A_SYMBOL) {
    /* Ignore the fact that this is a "list" */
    OSCroute_doanything(x, argv[0].a_w.w_symbol, argc-1, argv+1);
  } else if (argc > 0)  {
    // post("* OSC-route: invalid list beginning with a number");
    // output on unmatched outlet jdl 20020908
    if (argv[0].a_type == A_FLOAT) {
      outlet_float(x->x_outlets[x->x_num], argv[0].a_w.w_float);
    } else {
      post("* OSC-route: unrecognized atom type!");
    }
  }
  else {
      //  output a bang on the rejected outlet if no arguments
      outlet_bang(x->x_outlets[x->x_num]);
  }
  // ExitCallback();
}


void OSCroute_anything(t_OSCroute *x, t_symbol *s, int argc, t_atom *argv) {
  // EnterCallback();
  OSCroute_doanything(x, s, argc, argv);
  // ExitCallback();
}




void OSCroute_doanything(t_OSCroute *x, t_symbol *s, int argc, t_atom *argv) {
  char *pattern, *nextSlash;
  int i;
  int matchedAnything;
  // post("*** OSCroute_anything(s %s, argc %ld)", s->s_name, (long) argc);
  
  pattern = s->s_name;
  if (pattern[0] != '/') {
    post("* OSC-route: invalid message pattern %s does not begin with /", s->s_name);
    outlet_anything(x->x_outlets[x->x_num], s, argc, argv);
    return;
  }
  
  matchedAnything = 0;
  
  nextSlash = NextSlashOrNull(pattern+1);
  if (*nextSlash == '\0') {
    /* last level of the address, so we'll output the argument list */
    

#ifdef NULL_IS_DIFFERENT_FROM_BANG
    if (argc==0) {
      post("* OSC-route: why are you matching one level pattern %s with no args?",
	   pattern);
      return;
    }
#endif
    
    for (i = 0; i < x->x_num; ++i) {
      if (MyPatternMatch(pattern+1, x->x_prefixes[i]+1)) {
	++matchedAnything;
	
	// I hate stupid Max lists with a special first element
	if (argc == 0) {
	  outlet_bang(x->x_outlets[i]);
	} else if (argv[0].a_type == A_SYMBOL) {
	  // Promote the symbol that was argv[0] to the special symbol
	  outlet_anything(x->x_outlets[i], argv[0].a_w.w_symbol, argc-1, argv+1);
	} else if (argc > 1) {
	  // Multiple arguments starting with a number, so naturally we have
	  // to use a special function to output this "list", since it's what
	  // Max originally meant by "list".
	  outlet_list(x->x_outlets[i], 0L, argc, argv);
	} else {
	  // There was only one argument, and it was a number, so we output it
	  // not as a list
/* 	  if (argv[0].a_type == A_LONG) { */
	    
/* 	    outlet_int(x->x_outlets[i], argv[0].a_w.w_long); */
	  //	  } else 
	  if (argv[0].a_type == A_FLOAT) {
	    
	    outlet_float(x->x_outlets[i], argv[0].a_w.w_float);
	  } else {
	    post("* OSC-route: unrecognized atom type!");
	  }
	}
      }
    }
  } else {
    /* There's more address after this part, so our output list will begin with
       the next slash.  */
    t_symbol *restOfPattern = 0; /* avoid the gensym unless we have to output */
    char patternBegin[1000];
    
    
    /* Get the first level of the incoming pattern to match against all our prefixes */
    StrCopyUntilSlash(patternBegin, pattern+1);
    
    for (i = 0; i < x->x_num; ++i) {
      if (MyPatternMatch(patternBegin, x->x_prefixes[i]+1)) {
	++matchedAnything;
	if (restOfPattern == 0) {
	  restOfPattern = gensym(nextSlash);
	}
	outlet_anything(x->x_outlets[i], restOfPattern, argc, argv);
      }
    }
  }

  if (x->x_complainmode) {
    if (!matchedAnything) {
      post("* OSC-route: pattern %s did not match any prefixes", pattern);
    }
  }

  // output unmatched data on rightmost outlet a la normal 'route' object, jdl 20020908
  if (!matchedAnything) {
    outlet_anything(x->x_outlets[x->x_num], s, argc, argv);
  }


}

static char *NextSlashOrNull(char *p) {
  while (*p != '/' && *p != '\0') {
    p++;
  }
  return p;
}

static void StrCopyUntilSlash(char *target, const char *source) {
  while (*source != '/' && *source != '\0') {
    *target = *source;
    ++target;
    ++source;
  }
  *target = 0;
}

static int MyStrCopy(char *target, const char *source) {
  int i = 0;
  while (*source != '\0') {
    *target = *source;
    ++target;
    ++source;
    ++i;
  }
  *target = 0;
  return i;
}



void OSCroute_allmessages(t_OSCroute *x, t_symbol *s, int argc, t_atom *argv) {
  int i;
  t_symbol *prefixSymbol = 0;
  char prefixBuf[1000];
  char *endOfPrefix;
  t_atom a[1];
  
  if (argc >= 1 && argv[0].a_type == A_SYMBOL) {
    prefixSymbol = argv[0].a_w.w_symbol;
    endOfPrefix = prefixBuf + MyStrCopy(prefixBuf, 
					prefixSymbol->s_name);
  } else {
    prefixSymbol = ps_emptySymbol;
    prefixBuf[0] = '\0';
    endOfPrefix = prefixBuf;
  }
  

  for (i = 0; i < x->x_num; ++i) {
    post("OSC:  %s%s", prefixSymbol->s_name, x->x_prefixes[i]);
    MyStrCopy(endOfPrefix, x->x_prefixes[i]);
    SETSYMBOL(a, gensym(prefixBuf));
    outlet_anything(x->x_outlets[i], s, 1, a);
  }
}
