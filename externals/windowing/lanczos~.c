/* lanczos~ - lanczos windowing function for Pure Data 
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

#ifdef _WIN32
#define M_PI 3.14159265358979323846
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#define DEFBLOCKSIZE 64

void fillLanczos(float *vec, int n) {
  int i;
  float xShift = (float)n / 2;
  float x;
  for (i = 0; i < n; i++) {
    x = (i - xShift) / xShift;
    if (x == 0) {
      vec[i] = 1;
    }
    else {
      vec[i] = (float)(sin(M_PI * x) / (M_PI * x));
    }
  }
}

static t_class *lanczos_class;

typedef struct _lanczos {
  t_object x_obj;
  int x_blocksize;
  float *x_table;
} t_lanczos;

static t_int* lanczos_perform(t_int *w) {
  t_lanczos *x = (t_lanczos *)(w[1]);
  t_float *in = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  int i;
  if (x->x_blocksize != n) {
    if (x->x_blocksize < n) {
      x->x_table = realloc(x->x_table, n * sizeof(float));
    }
    x->x_blocksize = n;
    fillLanczos(x->x_table, x->x_blocksize);
  }
  for (i = 0; i < n; i++) {
    *out++ = *(in++) * x->x_table[i];
  }
  return (w + 5);
}

static void lanczos_dsp(t_lanczos *x, t_signal **sp) {
  dsp_add(lanczos_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void* lanczos_new(void) {
  t_lanczos *x = (t_lanczos *)pd_new(lanczos_class);
  x->x_blocksize = DEFBLOCKSIZE;
  x->x_table = malloc(x->x_blocksize * sizeof(float));
  fillLanczos(x->x_table, x->x_blocksize);
  outlet_new(&x->x_obj, gensym("signal"));
  return (x);
}

static void lanczos_free(t_lanczos *x) {
  free(x->x_table);
}

void lanczos_tilde_setup(void) {
  lanczos_class = class_new(gensym("lanczos~"),
			    (t_newmethod)lanczos_new, 
			    (t_method)lanczos_free,
    	                    sizeof(t_lanczos),
			    0,
			    A_DEFFLOAT,
			    0);
  class_addmethod(lanczos_class, nullfn, gensym("signal"), 0);
  class_addmethod(lanczos_class, (t_method)lanczos_dsp, gensym("dsp"), 0);
}
