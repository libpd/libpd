/*
 *   Pure Data Packet module. Standard sync rates.
 *   Copyright (c) by Tom Schouten <tom@zwizwa.be>
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
 *
 */

/* pdp_metro
   this is the main audio/video sync module. use this if
   you need to produce synchronized raw output. this uses
   'industry standard' integer sampling ratios.

   it works like this. for example, the most awkward 30 * 999/1000 Hz rate.


*/

#include "pdp.h"

typedef struct _metro_struct
{
    t_object  x_obj;
    t_float   x_f;
    t_clock  *x_clock;
    t_outlet *x_outlet;

    t_int    x_count;      // current count for (upsampled) audio ticks
    t_int    x_den;        // video framerate denominator and nominator
    t_int    x_nom;

} t_metro;




static t_int *metro_perform(t_int *w);
#define DSP_ARG(type, name, source) type name = (type)source
static void metro_dsp(t_metro *x, t_signal **sp)
{
    dsp_add(metro_perform, 3, x, sp[0]->s_n, (int)sys_getsr());
}
static t_int *metro_perform(t_int *w){
    DSP_ARG(t_metro*, x,  w[1]);
    DSP_ARG(t_int,    n,  w[2]);
    DSP_ARG(t_int,    sr, w[3]);

    t_int period   = sr * x->x_nom;
    t_int upsample = x->x_den;

    t_int dobang = 0;
    
    //fprintf(stderr, "%d/%d\t%d\t%d\n", x->x_den, x->x_nom, x->x_count, period);
    x->x_count += n * upsample;
    while (x->x_count > period) {
	dobang = 1;
	x->x_count -= period;
    }
    if (dobang) clock_set(x->x_clock, 0);
    return w+4;
}
static void metro_tick(t_metro *x)
{
    outlet_bang(x->x_outlet);
}

static void metro_reset(t_metro *x){
    x->x_count = 0;
}
static void metro_fps(t_metro *x, t_float fden, t_float fnom)
{
    int den = (int)fden;
    int nom = (int)fnom;
    if (den < 1) den = 1;
    if (nom < 1) nom = 1;
    x->x_den = den;
    x->x_nom = nom;
}
static void metro_free(t_metro *x)
{
    clock_unset(x->x_clock);
}

static t_class *metro_class;



static void *metro_new(t_symbol *s, int argc, t_atom *argv)
{
    t_metro *x = (t_metro *)pd_new(metro_class);
    x->x_outlet = outlet_new(&x->x_obj, &s_bang);
    x->x_clock = clock_new(x, (t_method)metro_tick);
    x->x_count = 0;

    metro_fps(x, 1, 1);

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_metro_setup(void)
{


    metro_class = class_new(gensym("pdp_metro~"), (t_newmethod)metro_new,
			    (t_method)metro_free, sizeof(t_metro), 0, A_NULL);

    CLASS_MAINSIGNALIN(metro_class, t_metro, x_f);   
    class_addmethod(metro_class, (t_method)metro_tick,   gensym("tick"),  A_NULL);
    class_addmethod(metro_class, (t_method)metro_dsp,    gensym("dsp"),  A_NULL);
    class_addmethod(metro_class, (t_method)metro_fps,    gensym("fps"),  A_FLOAT, A_DEFFLOAT, A_NULL);


}

#ifdef __cplusplus
}
#endif
