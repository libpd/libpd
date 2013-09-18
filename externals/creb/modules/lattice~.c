/*
 *   lattice.c  - a lattice filter for pd
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

#define MAXORDER 1024
#define MAXREFCO 0.9999

typedef struct latticesegment
{
    t_float delay; // delay element
    t_float rc;    // reflection coefficient
} t_latticesegment;

typedef struct latticectl
{
    t_latticesegment c_segment[MAXORDER]; // array of lattice segment data
    t_int c_segments;
} t_latticectl;

typedef struct lattice
{
  t_object x_obj;
  t_float x_f;
  t_latticectl x_ctl;
} t_lattice;



static t_int *lattice_perform(t_int *w)
{


  t_float *in    = (t_float *)(w[3]);
  t_float *out    = (t_float *)(w[4]);
  t_latticectl *ctl  = (t_latticectl *)(w[1]);
  t_int i,j;
  t_int n = (t_int)(w[2]);
  t_float x,rc,d;

  t_latticesegment* seg = ctl->c_segment;
  t_int segments = ctl->c_segments;
  for (i=0; i<n; i++)
  {
      x = *in++;
      // section 0
      rc = seg[0].rc;
      x += seg[0].delay * rc;
      
      // section 1 to segments-1
      for (j=1; j < segments; j++)
      {
	  rc = seg[j].rc;
	  d  = seg[j].delay;
	  x += d * rc;
	  seg[j-1].delay = d - rc * x; 
      }
      // stub
      seg[segments-1].delay = x;

      *out++ = x;
  }

  return (w+5);
}

static void lattice_dsp(t_lattice *x, t_signal **sp)
{
    dsp_add(lattice_perform, 4, &x->x_ctl, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec);

}                                  
static void lattice_free(void)
{

}

t_class *lattice_class;

static void lattice_rc(t_lattice *x, t_float segment, t_float refco)
{
    t_int seg = (t_float)segment;
    if ((seg >= 0) && (seg < x->x_ctl.c_segments)){
	if (refco >= MAXREFCO) refco = MAXREFCO;
	if (refco <= -MAXREFCO) refco = -MAXREFCO;
	x->x_ctl.c_segment[seg].rc = refco;
    }
}

static void lattice_reset(t_lattice *x)
{
    t_float* buf = (t_float *)x->x_ctl.c_segment;
    t_int n = x->x_ctl.c_segments;
    t_int i;
    for (i=0; i<n; i++) buf[i]=0;
}

static void *lattice_new(t_floatarg segments)
{
    t_lattice *x = (t_lattice *)pd_new(lattice_class);
    t_int seg = (t_int)segments;

    outlet_new(&x->x_obj, gensym("signal")); 

    if (seg < 1) seg = 1;
    if (seg > MAXORDER) seg = MAXORDER;

    x->x_ctl.c_segments = seg;

    lattice_reset(x);

    return (void *)x;
}

void lattice_tilde_setup(void)
{
  //post("lattice~ v0.1");
    lattice_class = class_new(gensym("lattice~"), (t_newmethod)lattice_new,
    	(t_method)lattice_free, sizeof(t_lattice), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(lattice_class, t_lattice, x_f); 
    class_addmethod(lattice_class, (t_method)lattice_dsp, gensym("dsp"), 0); 
    class_addmethod(lattice_class, (t_method)lattice_reset, gensym("reset"), 0); 
    class_addmethod(lattice_class, (t_method)lattice_rc,
		    gensym("rc"), A_FLOAT, A_FLOAT, 0); 

}

