/* 
 * unpack~ :: convert (list-of-) float inputs to signals
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

static t_class *sigunpack_class;

typedef struct _sigunpack
{
  t_object x_obj;
  t_sample *buffer;
  t_sample *rp, *wp;
  int bufsize;

} t_sigunpack;

static void sigunpack_float(t_sigunpack *x, t_float f)
{
  if (x->wp + 1  != x->rp) {
    *(x->wp)++ = f;
    if (x->wp == x->buffer + x->bufsize) x->wp = x->buffer;
  }
}

static void sigunpack_list(t_sigunpack *x, t_symbol *s, int argc, t_atom *argv)
{
  t_atom *ap = argv;
  int i;
  for (i = 0, ap = argv; i < argc; ap++, i++) {
    if (x->wp + 1  != x->rp) {
      *(x->wp)++ = atom_getfloat(ap);
      if (x->wp == x->buffer + x->bufsize) x->wp = x->buffer;
    }    
  }
}


static t_int *sigunpack_perform(t_int *w)
{
  t_sample *out = (t_sample *)(w[1]);
  t_sigunpack *x = (t_sigunpack *)w[2];
  int n = (int)(w[3]);

  t_sample *buf = x->rp;
  int hitchhike = 0;

  if ((x->wp >= x->rp) && (x->wp < x->rp+n)) hitchhike=1;
  x->rp += n;
  if (x->rp == x->buffer + x->bufsize) x->rp = x->buffer;
  if (hitchhike) x->wp = x->rp;

  while (n--) {
    *out++ = *buf;
    *buf++ = 0;
  }

  return (w+4);
}

static void sigunpack_dsp(t_sigunpack *x, t_signal **sp)
{
  if (x->bufsize % sp[0]->s_n) {
    int newsize = sp[0]->s_n*(1+(int)(x->bufsize/sp[0]->s_n));
    freebytes(x->buffer, x->bufsize * sizeof(*x->buffer));
    x->buffer = (t_sample *)getbytes(newsize * sizeof(*x->buffer));

    x->rp = x->wp = x->buffer;
    x->bufsize = newsize;
  }

  dsp_add(sigunpack_perform, 3, sp[0]->s_vec, x, sp[0]->s_n);
}

static void *sigunpack_new(t_floatarg f)
{
  t_sigunpack *x = (t_sigunpack *)pd_new(sigunpack_class);

  int suggestedsize = (int)f;
  int bufsize;
  if (!suggestedsize) bufsize = 64;
  else bufsize = (suggestedsize % 64)?(64*(1+(int)(suggestedsize/64))):suggestedsize;

  x->buffer = (t_sample *)getbytes(bufsize * sizeof(*x->buffer));
  x->bufsize = bufsize;
  x->rp = x->wp = x->buffer;

  outlet_new(&x->x_obj, gensym("signal"));

  return (x);
}

static void sigunpack_help(void)
{
  post("unpack~\t:: outputs a sequence of floats as a signal");
}

void unpack_tilde_setup(void)
{
  sigunpack_class = class_new(gensym("unpack~"), (t_newmethod)sigunpack_new, 0,
                              sizeof(t_sigunpack), 0, A_DEFFLOAT, 0);
  class_addmethod(sigunpack_class, (t_method)sigunpack_dsp, gensym("dsp"), 0);
  class_addfloat(sigunpack_class, (t_method)sigunpack_float);
  class_addlist (sigunpack_class, (t_method)sigunpack_list);


  class_addmethod(sigunpack_class, (t_method)sigunpack_help, gensym("help"), 0);
  zexy_register("unpack~");
}
