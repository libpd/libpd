/* gaussian~ - gaussian windowing function for Pure Data 
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

#define DEFDELTA 0.5
#define DEFBLOCKSIZE 64

/* MSW and OSX don't appear to have single-precision ANSI math */
#if defined(_WIN32) || defined(__APPLE__)
#define powf pow
#endif

void fillGaussian(float *vec, int n, float delta) {
  int i;
  float xShift = (float)n / 2;
  float x;
  if (delta == 0) {
    delta = 1;
  }
  for (i = 0; i < n; i++) {
    x = (i - xShift) / xShift;
    vec[i] = (float)(pow(2, (-1 * (x / delta) * (x / delta))));
  }
}

static t_class *gaussian_class;

typedef struct _gaussian {
  t_object x_obj;
  int x_blocksize;
  float *x_table;
  float x_delta;
} t_gaussian;

static t_int* gaussian_perform(t_int *w) {
  t_gaussian *x = (t_gaussian *)(w[1]);
  t_float *in = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  int i;
  if (x->x_blocksize != n) {
    if (x->x_blocksize < n) {
      x->x_table = realloc(x->x_table, n * sizeof(float));
    }
    x->x_blocksize = n;
    fillGaussian(x->x_table, x->x_blocksize, x->x_delta);
  }
  for (i = 0; i < n; i++) {
    *out++ = *(in++) * x->x_table[i];
  }
  return (w + 5);
}

static void gaussian_dsp(t_gaussian *x, t_signal **sp) {
  dsp_add(gaussian_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void gaussian_float(t_gaussian *x, t_float delta) {
  if (delta != 0) {
    x->x_delta = delta;
    fillGaussian(x->x_table, x->x_blocksize, x->x_delta);
  }
}

static void* gaussian_new(float delta) {
  t_gaussian *x = (t_gaussian *)pd_new(gaussian_class);
  x->x_blocksize = DEFBLOCKSIZE;
  if (delta == 0) {
    x->x_delta = DEFDELTA;
  }
  else {
    x->x_delta = delta;
  }
  x->x_table = malloc(x->x_blocksize * sizeof(float));
  fillGaussian(x->x_table, x->x_blocksize, x->x_delta);
  outlet_new(&x->x_obj, gensym("signal"));
  return (x);
}

static void gaussian_free(t_gaussian *x) {
  free(x->x_table);
}

void gaussian_tilde_setup(void) {
  gaussian_class = class_new(gensym("gaussian~"),
			    (t_newmethod)gaussian_new, 
			    (t_method)gaussian_free,
    	                    sizeof(t_gaussian),
			    0,
			    A_DEFFLOAT,
			    0);
  class_addmethod(gaussian_class, nullfn, gensym("signal"), 0);
  class_addmethod(gaussian_class, (t_method)gaussian_dsp, gensym("dsp"), 0);
  class_addfloat(gaussian_class, (t_method)gaussian_float);
}
