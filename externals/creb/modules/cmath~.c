/*
 *   cmath.c  - some complex math dsp objects
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

#define MINNORM 0.0000000001

typedef struct cmath
{
    t_object x_obj;
    t_float x_f;
    t_perfroutine x_perf;
} t_cmath;


static t_int *cmath_perform_clog(t_int *w)
{
    t_float *inx    = (t_float *)(w[2]);
    t_float *iny    = (t_float *)(w[3]);
    t_float *outx    = (t_float *)(w[5]); // clockwize addressing
    t_float *outy    = (t_float *)(w[4]);
    t_int i;
    t_int n = (t_int)(w[1]);
    t_float x;

    while (n--){
	t_float x = *inx++;
	t_float y = *iny++;
 	t_float norm = sqrt(x*x + y*y);
	t_float arg = atan2(y, x);
	if (norm < MINNORM){
	    norm = MINNORM;
	}
	*outx++ = log(norm);
	*outy++ = arg;
    }
    
    return (w+6);
}


static t_int *cmath_perform_cexp(t_int *w)
{
    t_float *inx    = (t_float *)(w[2]);
    t_float *iny    = (t_float *)(w[3]);
    t_float *outx    = (t_float *)(w[5]); // clockwize addressing
    t_float *outy    = (t_float *)(w[4]);
    t_int i;
    t_int n = (t_int)(w[1]);
    t_float x;

    while (n--){
	t_float x = *inx++;
	t_float y = *iny++;
	t_float norm = exp(x);
	*outx++ = norm * cos(y);
	*outy++ = norm * sin(y);
    }
    
    return (w+6);
}

static t_int *cmath_perform_nfft(t_int *w)
{
    t_float *inx    = (t_float *)(w[2]);
    t_float *iny    = (t_float *)(w[3]);
    t_float *outx    = (t_float *)(w[5]); // clockwize addressing
    t_float *outy    = (t_float *)(w[4]);
    t_int i;
    t_int n = (t_int)(w[1]);
    t_float x;
    t_float scale = 1.0 / (sqrt((t_float)n));

    mayer_fft(n, inx, outx);

    while (n--){
	t_float x = *inx++;
	t_float y = *iny++;
	*outx++ = scale * x;
	*outy++ = scale * y;
    }
    
    return (w+6);
}

static t_int *cmath_perform_nifft(t_int *w)
{
    t_float *inx    = (t_float *)(w[2]);
    t_float *iny    = (t_float *)(w[3]);
    t_float *outx    = (t_float *)(w[5]); // clockwize addressing
    t_float *outy    = (t_float *)(w[4]);
    t_int i;
    t_int n = (t_int)(w[1]);
    t_float x;
    t_float scale = 1.0 / (sqrt((t_float)n));

    mayer_ifft(n, inx, outx);

    while (n--){
	t_float x = *inx++;
	t_float y = *iny++;
	*outx++ = scale * x;
	*outy++ = scale * y;
    }
    
    return (w+6);
}

static void cmath_dsp(t_cmath *x, t_signal **sp)
{
    dsp_add(x->x_perf, 5, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec);

}                                  
void cmath_free(void)
{

}

t_class *cmath_class;

t_cmath *cmath_new_common(void)
{
    t_cmath *x = (t_cmath *)pd_new(cmath_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("signal"), gensym("signal"));  
    outlet_new(&x->x_obj, gensym("signal")); 
    outlet_new(&x->x_obj, gensym("signal")); 
    return x;
}

#define DEFNEWCMATH(name, perfmethod)		\
void * name (void)				\
{						\
    t_cmath *x = cmath_new_common();		\
    x->x_perf = perfmethod ;			\
    return (void*)x;				\
}

DEFNEWCMATH(cmath_new_clog, cmath_perform_clog)
DEFNEWCMATH(cmath_new_cexp, cmath_perform_cexp)
DEFNEWCMATH(cmath_new_nfft, cmath_perform_nfft)
DEFNEWCMATH(cmath_new_nifft, cmath_perform_nifft)


void cmath_tilde_setup(void)
{
  //post("cmath~ v0.1");
    cmath_class = class_new(gensym("clog~"), (t_newmethod)cmath_new_clog,
    	(t_method)cmath_free, sizeof(t_cmath), 0, 0);

    class_addcreator((t_newmethod)cmath_new_cexp, gensym("cexp~"), A_NULL);
    class_addcreator((t_newmethod)cmath_new_nfft, gensym("nfft~"), A_NULL);
    class_addcreator((t_newmethod)cmath_new_nifft, gensym("nifft~"), A_NULL);

    CLASS_MAINSIGNALIN(cmath_class, t_cmath, x_f);

    class_addmethod(cmath_class, (t_method)cmath_dsp, gensym("dsp"), 0); 
}

