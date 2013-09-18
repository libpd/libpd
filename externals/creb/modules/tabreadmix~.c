/*
 *   tabreadmix.c  -  an overlap add tabread~ clone
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

#include "extlib_util.h"

/******************** tabreadmix~ ***********************/

static t_class *tabreadmix_tilde_class;

typedef struct _tabreadmix_tilde
{
    t_object x_obj;
    int x_npoints;
    t_float *x_vec;
    t_symbol *x_arrayname;
    t_float x_f;

    /* file position vars */
    int x_currpos;
    int x_prevpos;
    
    /* cross fader state vars */
    int x_xfade_size;
    int x_xfade_phase;
    t_float x_xfade_cos;
    t_float x_xfade_sin;
    t_float x_xfade_state_c;
    t_float x_xfade_state_s;

} t_tabreadmix_tilde;



inline void tabreadmix_tilde_wrapindices(t_tabreadmix_tilde *x)
{
    int max;

    /* modulo */
    x->x_currpos %= x->x_npoints;
    x->x_prevpos %= x->x_npoints;

    /* make sure 0<=..<x->x_npoints */
    //if (x->x_currpos < 0) x->x_currpos += x->x_npoints;
    //if (x->x_prevpos < 0) x->x_prevpos += x->x_npoints;
    x->x_currpos += (x->x_currpos < 0) * x->x_npoints;
    x->x_prevpos += (x->x_prevpos < 0) * x->x_npoints;

}



#define min(x,y) ((x)<(y)?(x):(y))

static t_int *tabreadmix_tilde_perform(t_int *w)
{
    t_tabreadmix_tilde *x = (t_tabreadmix_tilde *)(w[1]);
    t_float *pos = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);    
    int maxxindex;
    t_float *buf = x->x_vec;
    int i;
    t_float currgain, prevgain;
    t_float c,s;
    int chunk;
    int leftover;
    int newpos = (int)*pos;

    maxxindex = x->x_npoints;
    if (!buf) goto zero;
    if (maxxindex <= 0) goto zero;


    while (n){

	/* process as much data as possible */
	leftover = x->x_xfade_size - x->x_xfade_phase;
	chunk = min(n, leftover);

	for (i = 0; i < chunk; i++){
	    /* compute crossfade gains from oscillator state */
	    currgain = 0.5 - x->x_xfade_state_c;
	    prevgain = 0.5 + x->x_xfade_state_c;
	    
	    /* check indices & wrap */
	    tabreadmix_tilde_wrapindices(x);

	    /* mix and write */
	    newpos = (int)(*pos++);
	    *out++ = currgain * buf[x->x_currpos++] 
		+ prevgain * buf[x->x_prevpos++];
	    
	    /* advance oscillator */
	    c =   x->x_xfade_state_c * x->x_xfade_cos 
		- x->x_xfade_state_s * x->x_xfade_sin;
	    s =   x->x_xfade_state_c * x->x_xfade_sin 
		+ x->x_xfade_state_s * x->x_xfade_cos;
	    x->x_xfade_state_c = c;
	    x->x_xfade_state_s = s;
	}

	/* update indices */
	x->x_xfade_phase += chunk;
	n -= chunk;
	//pos += chunk;

	/* check if prev chunk is finished */
	if (x->x_xfade_size == x->x_xfade_phase){
	    x->x_prevpos = x->x_currpos;
	    x->x_currpos = newpos;
	    x->x_xfade_state_c = 0.5;
	    x->x_xfade_state_s = 0.0;
	    x->x_xfade_phase = 0;
	}

    }

    /* return if we ran out of data */
    return (w+5);


 zero:
    while (n--) *out++ = 0;
    return (w+5);
}


static void tabreadmix_tilde_blocksize(t_tabreadmix_tilde *x, t_float size)
{
    double prev_phase;
    int max;
    t_float fmax = (t_float)x->x_npoints * 0.5;

    if (size < 1.0) size = 1.0;

    prev_phase = (double)x->x_xfade_phase;
    prev_phase *= size;
    prev_phase /= (double)x->x_xfade_size;

    
    /* preserve the crossfader state */
    x->x_xfade_phase = (int)prev_phase;
    x->x_xfade_size = (int)size;


    x->x_xfade_cos = cos(M_PI / (t_float)x->x_xfade_size);
    x->x_xfade_sin = sin(M_PI / (t_float)x->x_xfade_size);


    /* make sure indices are inside array */
    if (x->x_npoints == 0){
	x->x_currpos = 0;
	x->x_prevpos = 0;
    }

    //else tabreadmix_tilde_wrapindices(x);



}

void tabreadmix_tilde_pitch(t_tabreadmix_tilde *x, t_float f)
{
    if (f < 1) f = 1;

    tabreadmix_tilde_blocksize(x, sys_getsr() / f);
}

void tabreadmix_tilde_chunks(t_tabreadmix_tilde *x, t_float f)
{
    if (f < 1.0) f = 1.0;
    tabreadmix_tilde_blocksize(x, (t_float)x->x_npoints / f);
}

void tabreadmix_tilde_bang(t_tabreadmix_tilde *x, t_float f)
{
    //trigger a chunk reset on next dsp call
    x->x_xfade_phase = x->x_xfade_size;
}

void tabreadmix_tilde_set(t_tabreadmix_tilde *x, t_symbol *s)
{
    t_garray *a;
    
    x->x_arrayname = s;
    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
    {
        if (*s->s_name)
            error("tabreadmix~: %s: no such array", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else if (!garray_getfloatarray(a, &x->x_npoints, &x->x_vec))
    {
        error("%s: bad template for tabreadmix~", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else garray_usedindsp(a);

    /* make sure indices are inside array */
    if (x->x_npoints == 0){
	x->x_currpos = 0;
	x->x_prevpos = 0;
    }

    //else tabreadmix_tilde_wrapindices(x);

}

static void tabreadmix_tilde_dsp(t_tabreadmix_tilde *x, t_signal **sp)
{
    tabreadmix_tilde_set(x, x->x_arrayname);

    dsp_add(tabreadmix_tilde_perform, 4, x,
        sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}

static void tabreadmix_tilde_free(t_tabreadmix_tilde *x)
{
}

static void *tabreadmix_tilde_new(t_symbol *s)
{
    t_tabreadmix_tilde *x = (t_tabreadmix_tilde *)pd_new(tabreadmix_tilde_class);
    x->x_arrayname = s;
    x->x_vec = 0;
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("blocksize"));  
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_f = 0;
    x->x_xfade_phase = 0;
    x->x_xfade_size = 1024;
    x->x_currpos = 0;
    x->x_prevpos = 0;
    x->x_xfade_state_c = 0.5;
    x->x_xfade_state_s = 0.0;
    tabreadmix_tilde_blocksize(x, 1024);
    return (x);
}

void tabreadmix_tilde_setup(void)
{
    tabreadmix_tilde_class = class_new(gensym("tabreadmix~"),
        (t_newmethod)tabreadmix_tilde_new, (t_method)tabreadmix_tilde_free,
        sizeof(t_tabreadmix_tilde), 0, A_DEFSYM, 0);
    CLASS_MAINSIGNALIN(tabreadmix_tilde_class, t_tabreadmix_tilde, x_f);
    class_addmethod(tabreadmix_tilde_class, (t_method)tabreadmix_tilde_dsp,
        gensym("dsp"), 0);
    class_addmethod(tabreadmix_tilde_class, (t_method)tabreadmix_tilde_set,
        gensym("set"), A_SYMBOL, 0);
    class_addmethod(tabreadmix_tilde_class, (t_method)tabreadmix_tilde_blocksize,
        gensym("blocksize"), A_FLOAT, 0);
    class_addmethod(tabreadmix_tilde_class, (t_method)tabreadmix_tilde_pitch,
        gensym("pitch"), A_FLOAT, 0);
    class_addmethod(tabreadmix_tilde_class, (t_method)tabreadmix_tilde_chunks,
        gensym("chunks"), A_FLOAT, 0);
    class_addmethod(tabreadmix_tilde_class, (t_method)tabreadmix_tilde_bang,
        gensym("bang"), 0);
}
