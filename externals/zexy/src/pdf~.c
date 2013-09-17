/* 
 * pdf~:  get the ProbabilityDensityFunction of a signal
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

/* ------------------------ pdf~ ----------------------------- */

static t_class *pdf_class;

typedef struct _pdf
{
  t_object x_obj;

  t_float *buf;
  int size;
  t_float halfsize;
} t_pdf;

static void clear_pdfbuf(t_pdf *x)
{
  int n = x->size;
  t_float *buf = x->buf;

  while (n--) *buf++=0.;
}

static void pdf_bang(t_pdf *x)
{
  int n = x->size;
  t_float *buf = x->buf, max = 0;
  t_atom a[2];

  while (n--) if (max < *buf++) max = buf[-1];

  n=x->size;
  buf = x->buf;

  if (max==0.) max=1.;
  max = 1./max;

  while (n--)
    {
      SETFLOAT(a, *buf++*max);
      SETFLOAT(a+1,x->size-n-1);
      outlet_list(x->x_obj.ob_outlet, gensym("list"), 2, (t_atom*)&a);
    }
}

static void pdf_float(t_pdf *x, t_floatarg f)
{
  if (f) pdf_bang(x); else clear_pdfbuf(x);
}

static t_int *pdf_perform(t_int *w)
{
  t_sample *in = (t_sample *)(w[1]);
  t_pdf *x = (t_pdf *)(w[2]);
  int n = (int)(w[3]);

  t_float *buf = x->buf;
  t_float halfsize = x->halfsize;

  while (n--)
    {
      t_sample f = *in++;
      int iindex = ((f + 1.0) * halfsize)+0.5;
      buf[(iindex<0)?0:((iindex>=x->size)?x->size-1:iindex)]+=1.;
    }
  return (w+4);
}

static void pdf_dsp(t_pdf *x, t_signal **sp)
{
  x->halfsize = (x->size - 1) / 2.0;

  dsp_add(pdf_perform, 3, sp[0]->s_vec, x, sp[0]->s_n);
}

static void *pdf_new(t_floatarg f)
{
  int i = f;
  t_pdf *x = (t_pdf *)pd_new(pdf_class);

  x->size = (i)?i:64;
  x->buf = (t_float *)getbytes(x->size * sizeof(*x->buf));
  clear_pdfbuf(x);

  outlet_new(&x->x_obj, gensym("list"));
    
  return (x);
}

static void pdf_free(t_pdf *x)
{
  if(x->buf)
    freebytes(x->buf, x->size*sizeof(*x->buf));
}

static void pdf_tilde_helper(void)
{
  post("\n%c pdf~\t:: get the probability density function of a signal (-1.0 to +1.0)", HEARTSYMBOL);
  post("'bang'\t  : output a list of the probabilities of 'n' function values"
       "\n'clear'\t  : clear the buffer (set all probabilities to zero)"
       "\n<1/0>\t  : short for 'bang' and 'clear'"
       "\n'help'\t  : view this");
  post("creation :: 'pdf~ [<n>]':: get the pdf for <n> (default: 64) values");
}

void pdf_tilde_setup(void)
{
  pdf_class = class_new(gensym("pdf~"), (t_newmethod)pdf_new, (t_method)pdf_free,
			sizeof(t_pdf), 0, A_DEFFLOAT, 0);

  class_addmethod(pdf_class, nullfn, gensym("signal"), 0);
  class_addmethod(pdf_class, (t_method)pdf_dsp, gensym("dsp"), 0);

  class_addmethod(pdf_class, (t_method)pdf_bang, gensym("bang"), 0);
  class_addmethod(pdf_class, (t_method)clear_pdfbuf, gensym("clear"), 0);
  class_addfloat(pdf_class, pdf_float);

  class_addmethod(pdf_class, (t_method)pdf_tilde_helper, gensym("help"), 0);
  zexy_register("pdf~");
}
