/* 
 * quantize~: quantize a signal to various bit-depths
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
  the long waited for quantize~-object that quantizes in many possible (but equal) steps
  of course, weŽll make a comfortable quantize of the float-signal for 16bit and 8bit

  1110:forum::für::umläute:1999
*/

#include "zexy.h"

/* ------------------------ quantize~ ----------------------------- */

static t_class *quantize_class;

typedef struct _quantize
{
  t_object x_obj;
  t_sample quantiz, dequantiz;
} t_quantize;

static void quantize_float(t_quantize *x, t_floatarg f)
{
  x->quantiz   = f;
  x->dequantiz = 1./f;
}

static void quantize_16bit(t_quantize *x)
{
  x->quantiz   = 32768.;
  x->dequantiz = 1./32768.;
}

static void quantize_8bit(t_quantize *x)
{
  x->quantiz   = 128.;
  x->dequantiz = 1./128.;
}

static t_int *quantize_perform(t_int *w)
{
  t_quantize	*x = (t_quantize *)(w[1]);
  t_sample *in = (t_sample *)(w[2]);
  t_sample *out = (t_sample *)(w[3]);
  int n = (int)(w[4]);

  t_sample quantiz = x->quantiz, dequantiz = x->dequantiz;

  if (quantiz)
    while (n--) *out++ = dequantiz*(int)(quantiz**in++);
  else while (n--) *out++ = *in++;

  return (w+5);
}

static void quantize_dsp(t_quantize *x, t_signal **sp)
{
  dsp_add(quantize_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void quantize_tilde_helper(t_quantize *x)
{
  ZEXY_USEVAR(x);
  post("%c quantize~-object\t:: used for quantizing signals by various degrees", HEARTSYMBOL);
  post("<quants> : quantize a signal into <quants> steps ('0' turns quantizing off)\n"
       "'8bit'   : quantize to 8 bit\n"
       "'16bit'  : quantize to 16 bit (default)\n"
       "'float'  : pass-through the signal unchanged\n"
       "'help'   : view this\n"
       "signal~\n");
  post("creation:: \"quantize~ [<quants>]\"");

}

static void *quantize_new(t_floatarg f)
{
  t_quantize *x = (t_quantize *)pd_new(quantize_class);
  outlet_new(&x->x_obj, gensym("signal"));
  if (f) quantize_float(x, f);
  else quantize_16bit(x);
	
  return (x);
}

void quantize_tilde_setup(void)
{
  quantize_class = class_new(gensym("quantize~"), (t_newmethod)quantize_new, 0,
			     sizeof(t_quantize), 0, A_DEFFLOAT, 0);
  class_addmethod(quantize_class, nullfn, gensym("signal"), 0);
  class_addmethod(quantize_class, (t_method)quantize_dsp, gensym("dsp"), 0);

  class_addfloat(quantize_class, quantize_float);
  class_addmethod(quantize_class, (t_method)quantize_8bit, gensym("8bit"), 0);
  class_addmethod(quantize_class, (t_method)quantize_16bit, gensym("16bit"), 0);
  
  class_addmethod(quantize_class, (t_method)quantize_tilde_helper, gensym("help"), 0);
  zexy_register("quantize~");
}
