/* Copyright (c) 2002 Yves Degoyon.                                             */
/* For information on usage and redistribution, and for a DISCLAIMER OF ALL     */
/* WARRANTIES, see the file, "LICENSE.txt," in this distribution.               */
/*                                                                              */
/* compressor~ -- compresses audio signal according to a factor                 */
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
/* "I can't get you out of my head"                                             */
/* "Na, na, na, na, na, na, na, na, na, ...."                                   */
/* Kylie Minogue ( without Nick Cave )                                          */
/* ---------------------------------------------------------------------------- */

#include "m_pd.h"
#include <stdlib.h>
#include <math.h>

static char   *compressor_version = "compressor~: an audio compressor, version 0.1 (ydegoyon@free.fr)";

typedef struct _compressor
{
    t_object x_obj;
    t_float x_strength;
    t_float x_pifactor;
    t_float x_f;
} t_compressor;

static t_class *compressor_class;

static void compressor_strength(t_compressor *x, t_floatarg fstrength )
{
    if (fstrength < -1.0)
    {
        x->x_strength = -1.0;
    }
    else if (fstrength > 5)
    {
        x->x_strength = 5;
    }
    else
    {
        x->x_strength = fstrength;
    }
    x->x_pifactor = pow( M_PI, x->x_strength );
}

static void *compressor_new(void)
{
    t_compressor *x = (t_compressor *)pd_new(compressor_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("strength"));
    x->x_strength = 0.;
    x->x_pifactor = pow( M_PI, x->x_strength );
    outlet_new(&x->x_obj, &s_signal);
    return (x);
}

static t_int *compressor_perform(t_int *w)
{
    t_float *in = (t_float *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    int n = (int)(w[3]);
    t_compressor *x = (t_compressor*)(w[4]);
    t_float isample_fact = x->x_pifactor;
    t_float osample_fact = 2.0 / M_PI;

    while (n--)
    {
        *out = atan (*in * isample_fact) * osample_fact;
        out++;
        in++;
    }
    return (w+5);
}

static void compressor_dsp(t_compressor *x, t_signal **sp)
{
    dsp_add(compressor_perform, 4, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n, x );
}

void compressor_tilde_setup(void)
{
    logpost(NULL, 4,  compressor_version );
    compressor_class = class_new(gensym("compressor~"), (t_newmethod)compressor_new, 0,
                                 sizeof(t_compressor), 0, 0);
    CLASS_MAINSIGNALIN( compressor_class, t_compressor, x_f );
    class_addmethod(compressor_class, (t_method)compressor_dsp, gensym("dsp"), 0);
    class_addmethod(compressor_class, (t_method)compressor_strength, gensym("strength"), A_FLOAT, 0);
}
