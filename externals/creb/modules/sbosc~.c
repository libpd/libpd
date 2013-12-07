/*
 *   sbosc.c - smallband oscillator. periodic, linear interpolated
 *   frequency center.  
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


#define LOGTABSIZE 10
#define TABSIZE (1<<LOGTABSIZE)
#define MASKTABSIZE (TABSIZE-1)

#define SHIFTTABSIZE ((sizeof(unsigned int) * 8) - LOGTABSIZE)
#define FRACTABSIZE (1<<SHIFTTABSIZE)
#define INVFRACTABSIZE (1.0 / (t_float)(FRACTABSIZE))
#define MASKFRACTABSIZE (FRACTABSIZE-1)

#define PITCHLIMIT 20.0

static t_float costable[TABSIZE];

static inline void _exp_j2pi(unsigned int t, t_float *real, t_float *imag)
{
    unsigned int i1 = t >> SHIFTTABSIZE;
    t_float f2 = (t & MASKFRACTABSIZE) * INVFRACTABSIZE;
    unsigned int i2 = (i1+1) & MASKTABSIZE;
    unsigned int i3 = (i1 - (TABSIZE>>2)) & MASKTABSIZE;
    unsigned int i4 = (i2 + 1 - (TABSIZE>>2)) & MASKTABSIZE;
    t_float f1 = 1.0 - f2;
    t_float a1 = f1 * costable[i1];
    t_float a2 = f2 * costable[i2];
    t_float b1 = f1 * costable[i3];
    t_float b2 = f2 * costable[i4];
    *real = a1 + a2;
    *imag = b1 + b2;
}

static t_class *sbosc_tilde_class;

typedef struct _sbosc_tilde
{
    t_object x_obj;
    t_float x_f;

    /* state vars */
    unsigned int x_phase;          // phase of main pitch osc
    unsigned int x_phase_inc;      // frequency of main pitch osc
    unsigned int x_harmonic;       // first harmonic
    t_float x_frac;                  // fraction of first harmonic


} t_sbosc_tilde;

static void *sbosc_tilde_new(void)
{
    t_sbosc_tilde *x = (t_sbosc_tilde *)pd_new(sbosc_tilde_class);
    x->x_phase = 0;
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("signal"), gensym("signal"));  
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("phase"));  
    outlet_new(&x->x_obj, gensym("signal"));
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_f = 0;
    return (x);
}


static t_int *sbosc_tilde_perform(t_int *w)
{
    t_sbosc_tilde *x = (t_sbosc_tilde *)(w[1]);
    t_float *pitch = (t_float *)(w[2]);
    t_float *center= (t_float *)(w[3]);
    t_float *out_real = (t_float *)(w[4]);
    t_float *out_imag = (t_float *)(w[5]);
    int n = (int)(w[6]);
    int i;

    t_float pitch_to_phase =  4294967295.0 / sys_getsr();

    for (i = 0; i < n; i++)
    {
	t_float p = *pitch++;
	t_float c = *center++;
      	t_float r1,r2,i1,i2;

	/* compute harmonic mixture */
	unsigned int h1 = x->x_phase * x->x_harmonic;
	unsigned int h2 = h1 + x->x_phase;
	_exp_j2pi(h1, &r1, &i1);
	_exp_j2pi(h2, &r2, &i2);
	r1 *= x->x_frac;
	i1 *= x->x_frac;
	r2 *= 1.0 - x->x_frac;
	i2 *= 1.0 - x->x_frac;

	*out_real++ = r1 + r2;
	*out_imag++ = i1 + i2;


	x->x_phase += x->x_phase_inc;

	/* check for phase wrap & update osc */
	if ((x->x_phase <= x->x_phase_inc)) 
	    {
		t_float p_plus = (p < 0.0) ? -p : p;
		t_float p_limit = (p_plus < PITCHLIMIT) ? PITCHLIMIT : p_plus;
		t_float c_plus = (c < 0.0) ? -c : c;
		t_float harmonic = c_plus/p_limit;
		x->x_phase_inc = pitch_to_phase * p_limit;
		x->x_harmonic = harmonic;
		x->x_frac = 1.0 - (harmonic - x->x_harmonic);
	    }
	

    }
	
    return (w+7);
}

static void sbosc_tilde_dsp(t_sbosc_tilde *x, t_signal **sp)
{
    dsp_add(sbosc_tilde_perform, 6, x,
        sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n);

}

static void sbosc_tilde_free(t_sbosc_tilde *x)
{
}

static void sbosc_tilde_phase(t_sbosc_tilde *x, t_floatarg f)
{
    x->x_phase = f * (1.0 / 4294967295.0);
}

void sbosc_tilde_setup(void)
{
    int i;

    // init tables
    for (i=0; i<TABSIZE; i++)
	costable[i] = cos(2.0 * M_PI * (double)i / (double)TABSIZE);
    


    // class setup
    sbosc_tilde_class = class_new(gensym("sbosc~"),
        (t_newmethod)sbosc_tilde_new, (t_method)sbosc_tilde_free,
        sizeof(t_sbosc_tilde), 0, A_DEFSYM, 0);
    CLASS_MAINSIGNALIN(sbosc_tilde_class, t_sbosc_tilde, x_f);
    class_addmethod(sbosc_tilde_class, (t_method)sbosc_tilde_dsp,
        gensym("dsp"), 0);
    class_addmethod(sbosc_tilde_class, (t_method)sbosc_tilde_phase,
        gensym("phase"), A_FLOAT, 0);
}
                                     
