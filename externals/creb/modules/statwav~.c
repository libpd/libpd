/*
 *   statwav.c  - static wavetable oscillator (scale + tabread)
 *
 *   Copyright (c) 2000-2003 by Tom Schouten
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "m_pd.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  

static t_class *statwav_tilde_class;

typedef struct _statwav_tilde
{
    t_object x_obj;
    int x_npoints;
    t_float *x_vec;
    t_symbol *x_arrayname;
    t_float x_f;
} t_statwav_tilde;

static void *statwav_tilde_new(t_symbol *s)
{
    t_statwav_tilde *x = (t_statwav_tilde *)pd_new(statwav_tilde_class);
    x->x_arrayname = s;
    x->x_vec = 0;
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_f = 0;
    return (x);
}

static t_int *statwav_tilde_perform(t_int *w)
{
    t_statwav_tilde *x = (t_statwav_tilde *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
    t_float maxindex;
    int imaxindex;
    t_float *buf = x->x_vec, *fp;
    int i;

    maxindex = x->x_npoints;
    imaxindex = x->x_npoints;

    if (!buf) goto zero;


    for (i = 0; i < n; i++)
    {
      t_float phase = *in++;
      t_float modphase = phase - (int)phase;
      t_float findex;
      int index;
      int ia, ib, ic, id;
      t_float frac,  a,  b,  c,  d, cminusb;
      static int count;

      if (modphase < 0.0) modphase += 1.0;
      findex = modphase * maxindex;
      index = findex;
    

      frac = findex - index;
      ia = (imaxindex+index-1) % imaxindex;
      ib = index;
      ic = (index+1) % imaxindex;
      id = (index+2) % imaxindex;

      a = buf[ia];
      b = buf[ib];
      c = buf[ic];
      d = buf[id];
      /* if (!i && !(count++ & 1023))
	 post("fp = %lx,  shit = %lx,  b = %f",  fp, buf->b_shit,  b); */
      cminusb = c-b;

      *out++ = b + frac * (
            cminusb - 0.5 * (frac-1.) * (
                (a - d + 3.0 * cminusb) * frac + (b - a - cminusb)
            )
        );                                                         
    }
	
    return (w+5);
 zero:
    while (n--) *out++ = 0;

    return (w+5);
}

static void statwav_tilde_set(t_statwav_tilde *x, t_symbol *s)
{
    t_garray *a;

    x->x_arrayname = s;
    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
    {
        if (*s->s_name)
            error("statwav~: %s: no such array", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else if (!garray_getfloatarray(a, &x->x_npoints, &x->x_vec))
    {
        error("%s: bad template for statwav~", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else garray_usedindsp(a);
}

static void statwav_tilde_dsp(t_statwav_tilde *x, t_signal **sp)
{
    statwav_tilde_set(x, x->x_arrayname);

    dsp_add(statwav_tilde_perform, 4, x,
        sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}

static void statwav_tilde_free(t_statwav_tilde *x)
{
}

void statwav_tilde_setup(void)
{
  //post("statwav~ v0.1");
    statwav_tilde_class = class_new(gensym("statwav~"),
        (t_newmethod)statwav_tilde_new, (t_method)statwav_tilde_free,
        sizeof(t_statwav_tilde), 0, A_DEFSYM, 0);
    CLASS_MAINSIGNALIN(statwav_tilde_class, t_statwav_tilde, x_f);
    class_addmethod(statwav_tilde_class, (t_method)statwav_tilde_dsp,
        gensym("dsp"), 0);
    class_addmethod(statwav_tilde_class, (t_method)statwav_tilde_set,
        gensym("set"), A_SYMBOL, 0);
}
                                     
