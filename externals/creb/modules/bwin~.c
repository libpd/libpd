/*
 *   window.c  - window generation abstraction
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


static t_class *window_class;

typedef struct _window
{
    t_object x_obj;
    t_float x_f;
    t_float *x_window;
    t_int x_size;
    t_symbol *x_type;
    t_float x_typearg;
    
} t_window;

static t_int *window_perform(t_int *w)
{
    t_window *x = (t_window *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    t_float *window = x->x_window;
    int n = (int)(w[4]);
    while (n--)
    {
    	*out++ = (*in++) * (*window++);
    }
    return (w+5);
}

static void window_size(t_window *x, t_int n)
{
    if (x->x_size != n){
	if (x->x_window) free(x->x_window);
	x->x_window = malloc(sizeof(t_float)*n);
	x->x_size = n;
    }
}


static void window_type(t_window *x, t_symbol *s, t_float f)
{
    int i;
    t_float a = 0;
    t_float a_inc = 2 * M_PI / (t_float)(x->x_size);
    if (!s) s = gensym("hamming");
    if (s == gensym("hamming")){
	for (i=0; i<x->x_size; i++){
	    t_float c = cos(a);
	    x->x_window[i] = (0.54 - 0.46 * c);
	    a += a_inc;
	}
    }
    else if (s == gensym("hann")){
	for (i=0; i<x->x_size; i++){
	    t_float c = cos(a);
	    x->x_window[i] = (0.5 - 0.5 * c);
	    a += a_inc;
	}
    }
    else if (s == gensym("hann/hamming")){
	for (i=0; i<x->x_size; i++) {
	    t_float c = cos(a);
	    x->x_window[i] = (0.5 - 0.5 * c) / (0.54 - 0.46 * c);
	    a += a_inc;
	}
    }
    else if (s == gensym("bfft_pink")){
	x->x_window[0] = 1.0f; //DC
	x->x_window[1] = 1.0f / sqrt((double)(x->x_size>>1)); //NY
	for (i=2; i<x->x_size; i+=2) {
	    double freq = (double)(i>>1);
	    t_float amp = sqrt(1.0 / freq);
	    x->x_window[i] = amp;
	    x->x_window[i+1] = amp;
	}
    }   
    else if (s == gensym("bfft_blue")){
	x->x_window[0] = 1.0f; //DC
	x->x_window[1] = sqrt((double)(x->x_size>>1)); //NY
	for (i=2; i<x->x_size; i+=2) {
	    double freq = (double)(i>>1);
	    t_float amp = sqrt(freq);
	    x->x_window[i] = amp;
	    x->x_window[i+1] = amp;
	}
    }   
    else if (s == gensym("bfft_db/octave")){
	t_float power = f/6.0;
	x->x_window[0] = 1.0f; //DC
	x->x_window[1] = pow((double)(x->x_size>>1), power); //NY
	for (i=2; i<x->x_size; i+=2) {
	    double freq = (double)(i>>1);
	    t_float amp = pow(freq, power);
	    x->x_window[i] = amp;
	    x->x_window[i+1] = amp;
	}
    }   


    /* default is no window */
    else{
	post("bwin~: unknown window type, using rectangular");
	for (i=0; i<x->x_size; i++) x->x_window[i] = 1.0f;
    }

    x->x_type = s;
    x->x_typearg = f;

}

static void window_dsp(t_window *x, t_signal **sp)
{
    int n =  sp[0]->s_n;
    if (x->x_size != n){
	window_size(x, n);
	window_type(x, x->x_type, x->x_typearg);
    }

    dsp_add(window_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, n);
}

static void window_free(t_window *x)
{
    free(x->x_window);
}


static void *window_new(t_symbol *s)
{
    t_window *x = (t_window *)pd_new(window_class);
    outlet_new(&x->x_obj, &s_signal);
    x->x_window = 0;
    window_size(x, 64);
    window_type(x, s, 0);
    return (x);
}

void bwin_tilde_setup(void)
{
    window_class = class_new(gensym("bwin~"), 
			     (t_newmethod)window_new, (t_method)window_free,
			     sizeof(t_window), 0, A_DEFSYMBOL, A_NULL);
    CLASS_MAINSIGNALIN(window_class, t_window, x_f);
    class_addmethod(window_class, (t_method)window_dsp,
		    gensym("dsp"), A_NULL);
    class_addmethod(window_class, (t_method)window_type,
		    gensym("type"), A_SYMBOL, A_DEFFLOAT, A_NULL);
}

