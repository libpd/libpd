/* 
 * dirac~: unity sample
 *
 * (c) 1999-2011 IOhannes m zmölnig, forum::für::umläute, institute of electronic music and acoustics (iem)
 *
 * optimizations:
 *  (c) 2005 tim blechmann 
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
 * This external makes the two main test-functions available :
 * dirac~: will make a single peak (eg: a 1 in all the 0s) at
 *	   a desired position in the signal-vector
 *	   the position can be passed as an argument when creating the object
 *
 * NOTE : the inlets do NOT specify any times but sample-NUMBERS;
 *	  there are 64 samples in a "standard" signal-vector,
 *	  each "lasting" for 1/44100 secs.
 */

#include "zexy.h"

/* ------------------------ dirac~ ----------------------------- */ 


static t_class *dirac_class;

typedef struct _dirac
{
  t_object x_obj;
  t_int position;
  t_int do_it;
} t_dirac;

static void dirac_bang(t_dirac *x)
{
  x->do_it = x->position;
}

static void dirac_float(t_dirac *x, t_float where)
{
  x->do_it = x->position = (t_int)where;
}

static t_int *dirac_perform(t_int *w)
{
  t_dirac *x = (t_dirac *)(w[1]);
  t_sample *out = (t_sample *)(w[2]);
  int n = (int)(w[3]);
	
  t_int do_it = x->do_it;

  zero_perform(w+1);

  if (do_it >= n)
    x->do_it -= n;
  else if(do_it >= 0)
    {
      out[do_it] = 1.f;
      x->do_it = -1;
    }

  return (w+4);
}

#ifndef __WIN32__
  /* LATER: investigate the occurence of zero_perf8() */
  /* it seems, like pd has the symbol zero_perf8(),
   * but it is not exported by m_pd.h:
   * so linux can use it, but w32 not
   * have to tell miller about that
   */
t_int *zero_perf8(t_int *w);
#else
/* on w32 we have no access to this hidden function anyhow... */
# define zero_perf8 zero_perform
#endif

static t_int *dirac_perf8(t_int *w)
{
  t_dirac *x = (t_dirac *)(w[1]);
  t_sample *out = (t_sample *)(w[2]);
  int n = (int)(w[3]);
	
  t_int do_it = x->do_it;
  zero_perf8(w+1);

  if (do_it >= n)
    x->do_it -= n;
  else if(do_it >= 0)
    {
      out[do_it] = 1.f;
      x->do_it = -1;
    }

  return (w+4);
}

static void dirac_dsp(t_dirac *x, t_signal **sp)
{
  if (sp[0]->s_n & 7)
    dsp_add(dirac_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
  else
    dsp_add(dirac_perf8, 3, x, sp[0]->s_vec, sp[0]->s_n);
}

static void dirac_helper(void)
{
  post("%c dirac~-object :: generates a dirac (unity-pulse)", HEARTSYMBOL);
  post("creation : \"dirac~ [<position>]\" : create a dirac at specified position (in samples)\n"
       "inlet\t: <position>\t: create a dirac at new position\n"
       "\t  'bang'\t: create a dirac at specified position\n"
       "\t  'help'\t: view this\n"
       "outlet\t: signal~");
}



static void *dirac_new(t_floatarg where)
{
  t_dirac *x = (t_dirac *)pd_new(dirac_class);

  outlet_new(&x->x_obj, gensym("signal"));

  //  x->do_it = where;
  x->do_it = -1;

  if (where > 0)
    x->position = where;
  else
    x->position = -where;
  return (x);
}
 
void dirac_tilde_setup(void)
{
  dirac_class = class_new(gensym("dirac~"), (t_newmethod)dirac_new, 0,
                          sizeof(t_dirac), 0, A_DEFFLOAT, 0);
  class_addfloat(dirac_class, dirac_float); 
  class_addbang(dirac_class, dirac_bang); 
  class_addmethod(dirac_class, (t_method)dirac_dsp, gensym("dsp"), 0);

  class_addmethod(dirac_class, (t_method)dirac_helper, gensym("help"), 0);
  zexy_register("dirac~");
}
