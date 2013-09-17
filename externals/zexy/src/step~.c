/* 
 * step~: unity step function
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
  step~  : will make a unity step at a desired point in the signal-vector; the second input specifies a 
  length:	after the so-specified time has elapsed, the step will toggle back to the previous
  value;
  the length can be passed as an argument when creating the object
  with length==1 you might do the dirac~ thing a little bit more complicated
  with length==0 the output just toggles between 0 and 1 every time you bang the object

  NOTE : the inlets do NOT specify any times but sample-NUMBERS; there are 64 samples in a signal-vector,
  each "lasting" for 1/44100 secs.
*/

#include "zexy.h"

/* ------------------------ step~ ----------------------------- */ 

static t_class *step_class;

typedef struct _step
{
  t_object x_obj;
  int position;
  int length;

  int toggle;

  int wait4start;
  int wait4stop;
} t_step;

static void step_bang(t_step *x)
{
  x->wait4stop = x->length + (x->wait4start = x->position);
}

static void step_float(t_step *x, t_float where)
{
  x->wait4stop = x->length + 
    (x->wait4start =
     (x->position = (where>0)*where)
     );
}

static void step_setlength(t_step *x, t_float arg)
{
  x->length = 1 + (arg>0)*arg;
}



static t_int *step_perform(t_int *w)
{
  t_step *x = (t_step *)(w[1]);
  t_sample *out = (t_sample *)(w[2]);
  int n = (int)(w[3]);

  int toggle = x->toggle;

  int wait4start = x->wait4start, wait4stop = x->wait4stop;

  while (n--)
    {
      wait4stop--;
      if (!wait4start--) toggle ^= 1;
      else if (!wait4stop) toggle ^= 1;

      *out++ = toggle;		
    }

  x->wait4start = wait4start;
  x->wait4stop = wait4stop;

  x->toggle = toggle;
  return (w+4);
}

static void step_dsp(t_step *x, t_signal **sp)
{
  dsp_add(step_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}


static void step_helper(void)
{
  post("%c step~-object :: generates a unity-step", HEARTSYMBOL);
  post("creation : \"dirac~ [<position> [<length>]]\" : create a rectangular window\n"
       "\t\t\tat specified position and with specified length (in samples)\n"
       "inlet1\t: <position>\t: create a rectangular window at new position\n"
       "\t  'bang'\t: create a rectangular window at specified position\n"
       "\t  'help'\t: view this\n"
       "inlet2\t: <length>\t: define new window length ('0' will make a unity-step)\n"
       "outlet\t: signal~");
}


static void *step_new(t_floatarg farg)
{
  t_step *x = (t_step *)pd_new(step_class);

  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("ft1"));
  outlet_new(&x->x_obj, gensym("signal"));

  x->position = 0;
  x->wait4start = x->wait4stop = 0;
  x->toggle = 1;

  step_setlength(x, farg);

  return (x);
}
 
void step_tilde_setup(void)
{
  step_class = class_new(gensym("step~"), (t_newmethod)step_new, 0,
                         sizeof(t_step), 0, A_DEFFLOAT, 0);

  class_addfloat(step_class, step_float);
  class_addbang(step_class, step_bang); 
  class_addmethod(step_class, (t_method)step_setlength, gensym("ft1"), A_FLOAT, 0);
  class_addmethod(step_class, (t_method)step_dsp, gensym("dsp"), 0);

  class_addmethod(step_class, (t_method)step_helper, gensym("help"), 0);

  zexy_register("step~");
}
