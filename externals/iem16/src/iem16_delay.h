/* copyleft (c) 2003 forum::für::umläute -- IOhannes m zmölnig @ IEM
 * based on d_array.c from pd:
 * Copyright (c) 1997-1999 Miller Puckette and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef INCLUDE_IEM16_DELAY_H__
#define INCLUDE_IEM16_DELAY_H__

/* sampling */

#include "iem16.h"
#include <string.h>


#if defined __WIN32 || defined __WIN32__
static int ugen_getsortno(void){return 0;}
#else
extern int ugen_getsortno(void);
#endif


t_class *sigdel16write_class;

typedef struct del16writectl{
  int c_n;
  t_iem16_16bit *c_vec;
  int c_phase;
} t_del16writectl;

typedef struct _sigdel16write{
  t_object x_obj;
  t_symbol *x_sym;
  t_del16writectl x_cspace;
  int x_sortno;   /* DSP sort number at which this was last put on chain */
  int x_rsortno;  /* DSP sort # for first del16read or write in chain */
  int x_vecsize;  /* vector size for del16read~ to use */
  float x_f;
} t_sigdel16write;


void sigdel16write_checkvecsize(t_sigdel16write *x, int vecsize);


# define XTRASAMPS 4
# define SAMPBLK 4


#define DEFDELVS 64	    	/* LATER get this from canvas at DSP time */

#endif
