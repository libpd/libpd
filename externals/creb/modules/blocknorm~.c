/*
 *   blocknorm.c - Normalize an array of dsp blocks. (spectral
 *   processing primitive)
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
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define MAXCHANNELS 32

typedef struct blocknormctl
{
    t_int c_channels;
    t_float **c_input;
    t_float **c_output;
} t_blocknormctl;

typedef struct blocknorm
{
  t_object x_obj;
  t_float x_f;
  t_blocknormctl x_ctl;
} t_blocknorm;


static t_int *blocknorm_perform(t_int *word)
{

    t_blocknormctl *ctl  = (t_blocknormctl *)(word[1]);
    t_int n             = (t_int)(word[2]);
    t_float **in        = ctl->c_input;
    t_float **out       = ctl->c_output;
    t_int c             = ctl->c_channels;
    t_int i,j;

    t_float p = 0.0f;
    t_float x, s;

    /* get power */
    for (j=0;j<c;j++){
	for (i=0;i<n;i++){
	    x = in[j][i];
	    p += x*x;
	}
    }

    /* compute normalization */
    if (p == 0.0f) s = 1.0f;
    else s =sqrt(((t_float)(c * n)) /  p);

    /* normalize */
    for (j=0;j<c;j++){
	for (i=0;i<n;i++){
	    out[j][i] *= s;  // FIXME: clockwize addressing problem
	}
    }

    

    return (word+3);
}



static void blocknorm_dsp(t_blocknorm *x, t_signal **sp)
{

    int i;
    int c = x->x_ctl.c_channels;
    for (i=0;i<c;i++){
	x->x_ctl.c_input[i] = sp[i]->s_vec;
	x->x_ctl.c_output[i] = sp[c+i]->s_vec;
    }
    dsp_add(blocknorm_perform, 2, &x->x_ctl, sp[0]->s_n);
}                                  


static void blocknorm_free(t_blocknorm *x)
{
    free (x->x_ctl.c_output);
    free (x->x_ctl.c_input);
}

t_class *blocknorm_class;

static void *blocknorm_new(t_floatarg channels)
{
    int i = (int)channels;
    int j;
    t_blocknorm *x = (t_blocknorm *)pd_new(blocknorm_class);

    if (i<1) i = 1;
    if (i>MAXCHANNELS) i = MAXCHANNELS;
    x->x_ctl.c_channels = i;
    x->x_ctl.c_input = malloc(sizeof(t_float)*i);
    x->x_ctl.c_output = malloc(sizeof(t_float)*i);

    j = i;
    while (--j) inlet_new(&x->x_obj, &x->x_obj.ob_pd, 
			  gensym("signal"), gensym("signal"));  
    while (i--) outlet_new(&x->x_obj, gensym("signal")); 

    return (void *)x;
}

void blocknorm_tilde_setup(void)
{
    blocknorm_class = class_new(gensym("blocknorm~"), (t_newmethod)blocknorm_new,
    	(t_method)blocknorm_free, sizeof(t_blocknorm), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(blocknorm_class, t_blocknorm, x_f); 
    class_addmethod(blocknorm_class, (t_method)blocknorm_dsp, gensym("dsp"), 0); 
}

