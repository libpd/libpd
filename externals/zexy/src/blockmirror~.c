/*
 * blockmirror~: mirrors a signalblock around it's center
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


#include "zexy.h"

/* ------------------------ blockmirror~ ----------------------------- */

/* mirrors a signalblock around it's center:
   {x[0], x[1], ... x[n-1]} --> {x[n-1], x[n-2], ... x[0]}
*/

static t_class *blockmirror_class;

typedef struct _blockmirror
{
  t_object x_obj;
  int doit;
  int blocksize;
  t_sample *blockbuffer;
} t_blockmirror;

static void blockmirror_float(t_blockmirror *x, t_floatarg f)
{
  x->doit = (f != 0);
}

static t_int *blockmirror_perform(t_int *w)
{
  t_blockmirror	*x = (t_blockmirror *)(w[1]);
  t_sample *in = (t_sample *)(w[2]);
  t_sample *out = (t_sample *)(w[3]);
  int n = (int)(w[4]);
  if (x->doit) {
    if (in==out){
      int N=n;
      t_sample *dummy=x->blockbuffer;
      while(n--)*dummy++=*in++;
      dummy--;
      while(N--)*out++=*dummy--;
    } else {
      in+=n-1;
      while(n--)*out++=*in--;
    }
  } else while (n--) *out++ = *in++;
  return (w+5);
}

static void blockmirror_dsp(t_blockmirror *x, t_signal **sp)
{
  if (x->blocksize<sp[0]->s_n){
    if(x->blockbuffer)freebytes(x->blockbuffer, sizeof(*x->blockbuffer)*x->blocksize);
    x->blocksize = sp[0]->s_n;
    x->blockbuffer = getbytes(sizeof(*x->blockbuffer)*x->blocksize);
  }
  dsp_add(blockmirror_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void blockmirror_helper(t_blockmirror*x)
{
  post("\n%c blockmirror~-object for reverting a signal", HEARTSYMBOL);
  post("'help' : view this\n"
       "signal~");
  post("outlet : signal~");
}
static void blockmirror_free(t_blockmirror*x)
{
  if(x->blockbuffer)
    freebytes(x->blockbuffer, sizeof(*x->blockbuffer)*x->blocksize);
  x->blockbuffer=0;
}
static void *blockmirror_new(void)
{
  t_blockmirror *x = (t_blockmirror *)pd_new(blockmirror_class);
  outlet_new(&x->x_obj, gensym("signal"));
  x->doit = 1;
  x->blocksize=0;
  return (x);
}

void blockmirror_tilde_setup(void)
{
  blockmirror_class = class_new(gensym("blockmirror~"), (t_newmethod)blockmirror_new, 
                                (t_method)blockmirror_free,
                                sizeof(t_blockmirror), 0, A_NULL);
  class_addmethod(blockmirror_class, nullfn, gensym("signal"), 0);
  class_addmethod(blockmirror_class, (t_method)blockmirror_dsp, gensym("dsp"), 0);
  
  class_addfloat(blockmirror_class, blockmirror_float);
  
  class_addmethod(blockmirror_class, (t_method)blockmirror_helper, gensym("help"), 0);
  zexy_register("blockmirror~");
}
