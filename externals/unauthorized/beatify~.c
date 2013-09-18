/* Copyright (c) 2002 Yves Degoyon.                                             */
/* For information on usage and redistribution, and for a DISCLAIMER OF ALL     */
/* WARRANTIES, see the file, "COPYING"  in this distribution.                   */
/*                                                                              */
/* beatify~ -- modulates an audio signal amplitude,                             */
/*             making it sounding like drums (sometimes)                        */
/*                                                                              */
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* See file LICENSE for further informations on licensing terms.                */
/*                                                                              */
/* This program is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/* GNU General Public License for more details.                                 */
/*                                                                              */
/* You should have received a copy of the GNU General Public License            */
/* along with this program; if not, write to the Free Software                  */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  */
/*                                                                              */
/* Based on PureData by Miller Puckette and others.                             */
/*                                                                              */
/* Made while listening to :                                                    */
/*                                                                              */
/* "We wanted succes"                                                           */
/* "We got lost into the night"                                                 */
/* "Take life as it comes"                                                      */
/* PJ Harvey -- We Float                                                        */
/* ---------------------------------------------------------------------------- */

#include "m_pd.h"
#include <stdlib.h>
#include <math.h>

static char   *beatify_version = "beatify~: an audio beatify, version 0.1 (ydegoyon@free.fr)";

typedef struct _beatify
{
    t_object x_obj;
    t_float x_attack;
    t_float x_sustain;
    t_float x_decay;
    t_float x_size;
    t_float x_gamplitude;
    t_float x_vol;
    t_int x_current;
    t_float x_f;
} t_beatify;

static t_class *beatify_class;

static void beatify_attack(t_beatify *x, t_floatarg fattack )
{
    if (fattack < 1.0)
    {
        x->x_attack = 1.0;
    }
    else
    {
        x->x_attack = fattack;
    }
}

static void beatify_sustain(t_beatify *x, t_floatarg fsustain )
{
    if (fsustain < 0.0)
    {
        x->x_sustain = 0.0;
    }
    else
    {
        x->x_sustain = fsustain;
    }
}

static void beatify_decay(t_beatify *x, t_floatarg fdecay )
{
    if (fdecay < 1.0)
    {
        x->x_decay = 1.0;
    }
    else
    {
        x->x_decay = fdecay;
    }
}

static void beatify_size(t_beatify *x, t_floatarg fsize )
{
    if (fsize < 100.0)
    {
        x->x_size = 100.0;
    }
    else
    {
        x->x_size = fsize;
    }
}

static void beatify_gamplitude(t_beatify *x, t_floatarg fgamplitude )
{
    if (fgamplitude < 0.0)
    {
        x->x_gamplitude = 0.0;
    }
    else if (fgamplitude > 1.0)
    {
        x->x_gamplitude = 1.0;
    }
    else
    {
        x->x_gamplitude = fgamplitude;
    }
}

static void *beatify_new(void)
{
    t_beatify *x = (t_beatify *)pd_new(beatify_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("attack"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("sustain"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("decay"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("size"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("gamplitude"));
    x->x_attack = 10;
    x->x_decay = 50;
    x->x_sustain = 2;
    x->x_size = 700;
    x->x_gamplitude = 0.5;
    x->x_current = x->x_size;
    outlet_new(&x->x_obj, &s_signal);
    return (x);
}

static t_int *beatify_perform(t_int *w)
{
    t_float *in = (t_float *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    t_int n = (int)(w[3]);
    t_beatify *x = (t_beatify*)(w[4]);
    t_float adelta = 0., ddelta = 0.;

    while (n--)
    {
        if ( x->x_current>=x->x_size )
        {
            adelta = (x->x_gamplitude-x->x_vol)/x->x_attack;
            ddelta = x->x_gamplitude/x->x_decay;
            x->x_current = 0;
        }
        if ( x->x_current<x->x_attack ) x->x_vol+= adelta;
        if ( x->x_current>x->x_attack+x->x_sustain && x->x_current<x->x_attack+x->x_sustain+x->x_decay )
        {
            x->x_vol-= ddelta;
        }
        x->x_current++;

        *(out) = *(in)*x->x_vol;
        out++;
        in++;
    }
    return (w+5);
}

static void beatify_dsp(t_beatify *x, t_signal **sp)
{
    dsp_add(beatify_perform, 4, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n, x );
}

void beatify_tilde_setup(void)
{
    logpost(NULL, 4,  beatify_version );
    beatify_class = class_new(gensym("beatify~"), (t_newmethod)beatify_new, 0,
                              sizeof(t_beatify), 0, 0);
    CLASS_MAINSIGNALIN( beatify_class, t_beatify, x_f );
    class_addmethod(beatify_class, (t_method)beatify_dsp, gensym("dsp"), 0);
    class_addmethod(beatify_class, (t_method)beatify_attack, gensym("attack"), A_FLOAT, 0);
    class_addmethod(beatify_class, (t_method)beatify_sustain, gensym("sustain"), A_FLOAT, 0);
    class_addmethod(beatify_class, (t_method)beatify_decay, gensym("decay"), A_FLOAT, 0);
    class_addmethod(beatify_class, (t_method)beatify_size, gensym("size"), A_FLOAT, 0);
    class_addmethod(beatify_class, (t_method)beatify_gamplitude, gensym("gamplitude"), A_FLOAT, 0);
}
