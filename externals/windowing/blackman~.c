/* blackman~ - blackman windowing function for Pure Data 
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

void fillBlackman(float *vec, int n) {
  int i;
  float xShift = (float)n / 2;
  float x;
  for (i = 0; i < n; i++) {
    x = (i - xShift) / xShift;
    vec[i] = (float)(0.42 + (0.5 * cos(M_PI * x)) + (0.08 * cos (2 * M_PI * x)));
  }
}

static t_class *blackman_class;

typedef struct _blackman {
  t_object x_obj;
  int x_blocksize;
  float *x_table;
} t_blackman;

static t_int* blackman_perform(t_int *w) {
  t_blackman *x = (t_blackman *)(w[1]);
  t_float *in = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  int i;
  if (x->x_blocksize != n) {
    if (x->x_blocksize < n) {
      x->x_table = realloc(x->x_table, n * sizeof(float));
    }
    x->x_blocksize = n;
    fillBlackman(x->x_table, x->x_blocksize);
  }
  for (i = 0; i < n; i++) {
    *out++ = *(in++) * x->x_table[i];
  }
  return (w + 5);
}

static void blackman_dsp(t_blackman *x, t_signal **sp) {
  dsp_add(blackman_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void* blackman_new(void) {
  t_blackman *x = (t_blackman *)pd_new(blackman_class);
  x->x_blocksize = DEFBLOCKSIZE;
  x->x_table = malloc(x->x_blocksize * sizeof(float));
  fillBlackman(x->x_table, x->x_blocksize);
  outlet_new(&x->x_obj, gensym("signal"));
  return (x);
}

static void blackman_free(t_blackman *x) {
  free(x->x_table);
}

void blackman_tilde_setup(void) {
  blackman_class = class_new(gensym("blackman~"),
			    (t_newmethod)blackman_new, 
			    (t_method)blackman_free,
    	                    sizeof(t_blackman),
			    0,
			    A_DEFFLOAT,
			    0);
  class_addmethod(blackman_class, nullfn, gensym("signal"), 0);
  class_addmethod(blackman_class, (t_method)blackman_dsp, gensym("dsp"), 0);
}
