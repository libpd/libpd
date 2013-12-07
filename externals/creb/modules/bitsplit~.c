/*
 *   bitsplit.c  - Convert a signal to a binary vector.
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
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define MAXCHANNELS 24

typedef struct bitsplitctl
{
    t_int c_outputs;
    t_float *c_input;
    t_float **c_output;
} t_bitsplitctl;

typedef struct bitsplit
{
  t_object x_obj;
  t_float x_f;
  t_bitsplitctl x_ctl;
} t_bitsplit;


static t_int *bitsplit_perform(t_int *word)
{

    t_bitsplitctl *ctl  = (t_bitsplitctl *)(word[1]);
    t_int n             = (t_int)(word[2]);
    t_float *in         = ctl->c_input;
    t_int outputs       = ctl->c_outputs;
    t_float **out       = ctl->c_output;
    t_int i,j;

    for (i=0;i<n;i++){
	long word = (in[i] * (t_float)(0x7fffffff));
	for (j=0; j<outputs; j++){
	    out[j][i] = (t_float)((word >> 31) & 1);
	    word <<= 1;
	}
    }

    return (word+3);
}



static void bitsplit_dsp(t_bitsplit *x, t_signal **sp)
{

    int i;
    x->x_ctl.c_input = sp[0]->s_vec;
    for (i=0;i<x->x_ctl.c_outputs;i++){
	x->x_ctl.c_output[i] = sp[i+1]->s_vec;
    }
    dsp_add(bitsplit_perform, 2, &x->x_ctl, sp[0]->s_n);
}                                  


static void bitsplit_free(t_bitsplit *x)
{
    free (x->x_ctl.c_output);
}

t_class *bitsplit_class;

static void *bitsplit_new(t_floatarg channels)
{
    int i = (int)channels;
    t_bitsplit *x = (t_bitsplit *)pd_new(bitsplit_class);

    if (i<1) i = 1;
    if (i>MAXCHANNELS) i = MAXCHANNELS;
    x->x_ctl.c_outputs = i;
    x->x_ctl.c_output = malloc(sizeof(t_float)*i);

    while (i--) outlet_new(&x->x_obj, gensym("signal")); 

    return (void *)x;
}

void bitsplit_tilde_setup(void)
{
    bitsplit_class = class_new(gensym("bitsplit~"), (t_newmethod)bitsplit_new,
    	(t_method)bitsplit_free, sizeof(t_bitsplit), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(bitsplit_class, t_bitsplit, x_f); 
    class_addmethod(bitsplit_class, (t_method)bitsplit_dsp, gensym("dsp"), 0); 
}

