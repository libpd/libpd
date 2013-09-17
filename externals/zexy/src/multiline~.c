/* 
 * multiline~: interpolating signal multiplier
 *
 * (c) 1999-2011 IOhannes m zmölnig, forum::für::umläute, institute of electronic music and acoustics (iem)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


/*
  a multiline that MULTIplicates MULTIple signals with "ramped floats" (--> like "line~")

  this is kind of multiplying some streams with the square diagonal matrix : diag*~
  for smooth-results we do this line~ thing

  1403:forum::für::umläute:2001
*/


/* i am not sure, whether there is a difference between loop-unrolling by hand or by compiler
 * i somehow have the feeling, that doing it by hand increases(!) performance for about 2%
 * anyhow, if you think that the compiler can do this job better, just disable the UNROLLED define below
 */
#define UNROLLED

#include "zexy.h"

/* --------------------------- multiline~ ---------------------------------- */

static t_class *mline_class;

typedef struct _mline {
  t_object x_obj;

  t_float time;

  int ticksleft;
  int retarget;

  t_float msec2tick;

  t_sample *value;
  t_sample *target;
  t_sample *increment; /* single precision is really a bad */

  t_sample **sigIN;
  t_sample **sigOUT;
  t_sample  *sigBUF;
  int       sigNUM;

} t_mline;

/* the message thing */

static void mline_list(t_mline *x, t_symbol *s, int argc, t_atom *argv)
{
  ZEXY_USEVAR(s);
  if (argc>x->sigNUM)x->time=atom_getfloat(argv+argc-1);

  if (x->time <= 0) {
    if (argc==1) {
      t_float f = atom_getfloat(argv);
      int i=x->sigNUM;
      while(i--)x->target[i]=x->value[i]=f;
    } else {
      int offset = (argc<x->sigNUM)?x->sigNUM-argc:0;
      int i=offset?argc:x->sigNUM;
      while(i--)x->target[i+offset]=x->value[i+offset]=atom_getfloat(argv++);
    }
    x->ticksleft=x->retarget=x->time=0;
  } else {
    if (argc==1) {
      int i = x->sigNUM;
      t_float f = atom_getfloat(argv);
      for(i=0;i<x->sigNUM;i++)x->target[i]=f;
    } else {
      int offset = (argc<x->sigNUM)?x->sigNUM-argc:0;
      int i=offset?argc:x->sigNUM;
      while(i--)x->target[i+offset]=atom_getfloat(argv++);
    }
    x->retarget = 1;
  }
}

static void mline_stop(t_mline *x)
{
  int i = x->sigNUM;
  while (i--) x->target[i] = x->value[i];
  x->ticksleft = x->retarget = 0;
}

/* the dsp thing */

static t_int *mline_perform(t_int *w)
{
  t_mline *x = (t_mline *)(w[1]);
  int n = (int)(w[2]);

  t_sample **out = x->sigOUT;
  t_sample **in  = x->sigIN;
  t_sample  *buf = x->sigBUF, *sigBUF = buf;
  t_sample  *inc = x->increment, *increment = inc;
  t_sample  *val = x->value, *value = val;
  t_sample  *tgt = x->target, *target = tgt;

  int sigNUM = x->sigNUM;

  if (x->retarget) {
    int nticks = x->time * x->msec2tick;

    if (!nticks) nticks = 1;
    x->ticksleft = nticks;

    x->retarget = 0;
  }

  if (x->ticksleft) {
    int N=n-1;
    t_sample oneovernos = 1./(x->ticksleft*n);

    int i=sigNUM;
    while(i--)*inc++=(*tgt++-*val++)*oneovernos;
    
    n=-1;
    while (n++<N) {
      buf = sigBUF;
      val = value;
      inc = increment;

      i = sigNUM;
      while (i--)*buf++=in[i][n]*(*val++ += *inc++);
      i=sigNUM;
      buf=sigBUF;
      while (i--)out[i][n]=*buf++;
    }

    if (!--x->ticksleft) {
      val = value;
      tgt = target;
      i = sigNUM;
      while(i--)*val++=*tgt++;
    }
      
  } else { /* no ticks left */
    int i = sigNUM;
    while (n--) {
      i = sigNUM;
      val = value;
      buf = sigBUF;
      while (i--)*buf++=in[i][n]**val++;
      i = sigNUM;
      buf = sigBUF;
      while (i--)out[i][n]=*buf++;
    }
  }

  return (w+3);
}



static void mline_dsp(t_mline *x, t_signal **sp)
{
  int i = x->sigNUM, n = 0;
  t_sample **dummy = x->sigIN;
  while(i--)*dummy++=sp[n++]->s_vec;

  i = x->sigNUM;
  dummy =x->sigOUT;
  while(i--)*dummy++=sp[n++]->s_vec;

  x->msec2tick = sp[0]->s_sr / (1000.f * sp[0]->s_n);
  dsp_add(mline_perform, 2, x, sp[0]->s_n);
}


/* setup/setdown things */

static void mline_free(t_mline *x)
{
  freebytes(x->value,     sizeof(x->value));
  freebytes(x->target,    sizeof(x->target));
  freebytes(x->increment, sizeof(x->increment));

  freebytes(x->sigIN,     sizeof(x->sigIN));
  freebytes(x->sigOUT,    sizeof(x->sigOUT));
  freebytes(x->sigBUF,    sizeof(x->sigBUF));
}


static void *mline_new(t_symbol *s, int argc, t_atom *argv)
{
  t_mline *x = (t_mline *)pd_new(mline_class);
  int i;
  ZEXY_USEVAR(s);

  if (!argc) {
    argc = 1;
    x->time = 0;
  } else {
    x->time = atom_getfloat(argv+argc-1);
    if (x->time < 0) x->time = 0;

    argc--;
    if (!argc) argc = 1;
  }

  x->sigNUM = argc;

  i = argc-1;

  outlet_new(&x->x_obj, gensym("signal"));

  while (i--) {
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("signal"), gensym("signal"));
    outlet_new(&x->x_obj, gensym("signal"));
  }

  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym(""));
  floatinlet_new(&x->x_obj, &x->time);
    
  x->sigIN  = (t_sample **)getbytes(x->sigNUM * sizeof(t_sample **));
  x->sigOUT = (t_sample **)getbytes(x->sigNUM * sizeof(t_sample **));
  x->sigBUF = (t_sample  *)getbytes(x->sigNUM * sizeof(t_sample  *));

  x->value     = (t_sample *)getbytes(x->sigNUM * sizeof(t_sample *));
  x->target    = (t_sample *)getbytes(x->sigNUM * sizeof(t_sample *));
  x->increment = (t_sample *)getbytes(x->sigNUM * sizeof(t_sample *));

  i = x->sigNUM;

  while (i--) {
    x->sigIN[i] = x->sigOUT[i] = 0;
    x->increment[i] = 0;
    x->value[x->sigNUM-i-1] = x->target[x->sigNUM-i-1] = atom_getfloat(argv+i);
  }

  x->msec2tick = x->ticksleft = x->retarget = 0;

  return (x);
}


static void mline_help(t_mline*x)
{
  post("\n%c multiline~\t:: ramped multiplication of multiple signals", HEARTSYMBOL);
}

void multiline_tilde_setup(void)
{
  mline_class = class_new(gensym("multiline~"), (t_newmethod)mline_new, (t_method)mline_free,
			  sizeof(t_mline), 0, A_GIMME, 0);

  class_addmethod(mline_class, (t_method)mline_dsp, gensym("dsp"), 0);
  class_addmethod(mline_class, nullfn, gensym("signal"), 0);

  class_addmethod(mline_class, (t_method)mline_list, gensym(""), A_GIMME, 0);
  class_addmethod(mline_class, (t_method)mline_stop, gensym("stop"), 0);

  class_addmethod  (mline_class, (t_method)mline_help, gensym("help"), A_NULL);
  zexy_register("multiline_tilde");
}
