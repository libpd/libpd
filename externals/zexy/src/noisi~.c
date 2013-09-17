/* 
 * noisi~: bandlimited noise generator
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
  30041999

  two bandlimited noise gnerators based on DODGE/JERSE "computer music" c3.9 : RANDI & RANDH 
	
  I do not care for copyrights
  (all in all, the used noise~-code (in fact, the pseude-random-code) is from Miller Puckette 
  and I made only very few modifications so look out for the LICENSE.TXT delivered with 
  puredata for further (c)-information

  forum für umläute 1999

  this is in fact the very same as the late "NOISE.C", except that I tried to optimize the code a little  bit
  (by partially removing those very expensive if..then's in about 15 minutes, so there are thousands of new bugs very likely)

  14071999
  finally added changing seeds, this is to prevent to noise~-units to output the very same, something quite unnatural even for pseudo-random-noise
*/

#include "zexy.h"

/* general */

typedef struct _nois
{
  t_object x_obj;
  int val;
  t_sample current;
  t_sample decrement;
  t_sample updater;		
  t_sample to_go;
} t_nois;


static void set_noisfreq(t_nois *x, t_floatarg freq)
{
  x->updater = (freq > 0) ? sys_getsr() / freq : 1;
  if (x->updater < 1)
    {
      x->updater = 1;
    }
  x->to_go = 0;
}


static void set_noisseed(t_nois *x, t_floatarg seed)
{
  x->val = seed;
}



/* ------------------------ noisi~ ----------------------------- */ 

static t_class *noisi_class;

static t_int *noisi_perform(t_int *w){
  t_nois *x = (t_nois *)(w[1]);
  t_sample *out = (t_sample *)(w[2]);
  int n = (int)(w[3]);

  int i_value = x->val;
  t_sample f_value = x->current;
  t_sample decrement = x->decrement;
  t_sample all_to_go = x->updater;
  t_sample still_to_go = x->to_go;

  if (all_to_go == 1)	{
    /* this is "pure white" noise, so we have to calculate each sample */ 
    while (n--) {
      i_value *= 435898247;
      i_value += 382842987;
      *out++ = ((t_sample)((i_value & 0x7fffffff) - 0x40000000)) * (t_sample)(1.0 / 0x40000000);
    }
  }  else if (n < still_to_go)   {
    /* signal won't change for the next 64 samples */ 
    still_to_go -= n;
    while (n--){
      *out++ = (f_value -= decrement);
    }
  }  else if (all_to_go + still_to_go > n) {
    /* only one update calculation necessary for 64 samples !!! */ 
    while (still_to_go-- > 0)	{
      n--;
      *out++ = (f_value -= decrement);
    }
    still_to_go += all_to_go + 1;
    decrement = (
		 (f_value = 
		  ((t_sample)((i_value & 0x7fffffff)-0x40000000))*(t_sample)(1.0 / 0x40000000)) -
		 ((t_sample)(((i_value = i_value * 435898247 + 382842987) & 0x7fffffff)
                             - 0x40000000)) * (t_sample)(1.0 / 0x40000000)
		 )  / all_to_go;

    while (n--)	{
      still_to_go--;
      *out++ = (f_value -= decrement);
    }
  }  else {
    /* anything else */ 
    while (n--)	{
      if (still_to_go-- <= 0) {		/* update only if all time has elapsed */ 
	still_to_go += all_to_go;
	decrement = (
		     (f_value = ((t_sample)((i_value & 0x7fffffff) - 0x40000000)) * (t_sample)(1.0 / 0x40000000)) -
		     ((t_sample)(((i_value = i_value * 435898247 + 382842987) & 0x7fffffff) - 0x40000000)) * (t_sample)(1.0 / 0x40000000)
		     ) / all_to_go;
      }
      *out++ = (f_value -= decrement);
    }
  }

  x->val = i_value;
  x->current = f_value;
  x->decrement = decrement;
  x->to_go = still_to_go;
  
  return (w+4);
}

static void noisi_dsp(t_nois *x, t_signal **sp){
  dsp_add(noisi_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}


static void noisi_helper(void){
  post("\n%c noisi~\t:: a bandlimited interpolating pseudo-noise generator", HEARTSYMBOL);
  post("<freq>\t : sampling-frequency (in Hz)\n"
       "'help'\t : view this");
  post("creation : \"noisi~ [<freq>]\"\t: ('0'(default) will produce 'white' noise)\n");
  post("note\t : the seed of the pseudo-noise generator changes from\n"
       "\t     instance to instance, so two noisi~-objects created at the\n"
       "\t     same time will produce different signals, something the original\n"
       "\t     noise~-object misses\n");
  post("for further details see DODGE/JERSE \"computer music\" c3.9\n");
}

static void *noisi_new(t_floatarg f){
  t_nois *x = (t_nois *)pd_new(noisi_class);
  
  static int init = 4259;
  x->val = (init *= 17);

  set_noisfreq (x, f);

  outlet_new(&x->x_obj, gensym("signal"));
  return (x);
}

void noisi_tilde_setup(void){
  noisi_class = class_new(gensym("noisi~"), (t_newmethod)noisi_new, 0, sizeof(t_nois), 0, A_DEFFLOAT, 0);

  class_addfloat(noisi_class, set_noisfreq);
  class_addmethod(noisi_class, (t_method)noisi_dsp, gensym("dsp"), 0);

  class_addmethod(noisi_class, (t_method)set_noisseed, gensym("seed"), A_FLOAT, 0);

  class_addmethod(noisi_class, (t_method)noisi_helper, gensym("help"), 0);
  zexy_register("noisi~");
}
