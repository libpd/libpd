/* bartlett~ - bartlett windowing function for Pure Data 
**
** Copyright (C) 2002 Joseph A. Sarlo
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**
** jsarlo@mambo.peabody.jhu.edu
*/

#include "m_pd.h"
#include <stdlib.h>
#include <math.h>

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#define DEFBLOCKSIZE 64

void fillBartlett(float *vec, int n) {
  int i;
  float xShift = (float)n / 2;
  float x;
  for (i = 0; i < n; i++) {
    x = (i - xShift) / xShift;
    vec[i] = (float)(1 - fabs(x));
  }
}

static t_class *bartlett_class;

typedef struct _bartlett {
  t_object x_obj;
  int x_blocksize;
  float *x_table;
} t_bartlett;

static t_int* bartlett_perform(t_int *w) {
  t_bartlett *x = (t_bartlett *)(w[1]);
  t_float *in = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  int i;
  if (x->x_blocksize != n) {
    if (x->x_blocksize < n) {
      x->x_table = realloc(x->x_table, n * sizeof(float));
    }
    x->x_blocksize = n;
    fillBartlett(x->x_table, x->x_blocksize);
  }
  for (i = 0; i < n; i++) {
    *out++ = *(in++) * x->x_table[i];
  }
  return (w + 5);
}

static void bartlett_dsp(t_bartlett *x, t_signal **sp) {
  dsp_add(bartlett_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void* bartlett_new(void) {
  t_bartlett *x = (t_bartlett *)pd_new(bartlett_class);
  x->x_blocksize = DEFBLOCKSIZE;
  x->x_table = malloc(x->x_blocksize * sizeof(float));
  fillBartlett(x->x_table, x->x_blocksize);
  outlet_new(&x->x_obj, gensym("signal"));
  return (x);
}

static void bartlett_free(t_bartlett *x) {
  free(x->x_table);
}

void bartlett_tilde_setup(void) {
  bartlett_class = class_new(gensym("bartlett~"),
			    (t_newmethod)bartlett_new, 
			    (t_method)bartlett_free,
    	                    sizeof(t_bartlett),
			    0,
			    A_DEFFLOAT,
			    0);
  class_addmethod(bartlett_class, nullfn, gensym("signal"), 0);
  class_addmethod(bartlett_class, (t_method)bartlett_dsp, gensym("dsp"), 0);
}
