/* ------------------------ randomblock~ -------------------------------------- */
/*                                                                              */
/* Generates a random signal block                                              */
/* Written by Yves Degoyon (ydegoyon@free.fr).                                  */
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
/* ---------------------------------------------------------------------------- */



#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "m_pd.h"            /* standard pd stuff */

static char   *randomblock_version = "randomblock~: generates a random audio block : author : ydegoyon@free.fr";

static t_class *randomblock_class;

typedef struct _randomblock
{
    t_object x_obj;
    t_int x_limit;
} t_randomblock;

/* clean up */
static void randomblock_free(t_randomblock *x)
{
}

static void *randomblock_new(t_float flimit)
{
    t_randomblock *x = (t_randomblock *)pd_new(randomblock_class);
    outlet_new(&x->x_obj, &s_signal);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("limit"));
    if ( flimit <= 0 || flimit > RAND_MAX )
    {
        post( "randomblock~: wrong creation argument" );
        return NULL;
    }
    x->x_limit = (int) flimit;
    return(x);
}

static void *randomblock_limit(t_randomblock* x, t_float flimit)
{
    if ( flimit < 0 || flimit > RAND_MAX )
    {
        post( "randomblock~: wrong random limit" );
        return;
    }
    else
    {
        x->x_limit=(int)flimit;
    }
}

static t_int *randomblock_perform(t_int *w)
{
    t_float *out = (t_float*) w[1];
    int n = (int)(w[2]);
    t_randomblock *x = (t_randomblock*) w[3];

    int rvalue = rand();
    // post("random value : %d", rvalue );
    rvalue = rvalue%(x->x_limit-n);
    // post("modulated by %d : %d", (x->x_limit-n), rvalue );
    if ( rvalue < 0 ) rvalue=0;

    while (n--)
    {
        *(out)++=(float)rvalue++;
    }

    return (w+4);
}

static void randomblock_dsp(t_randomblock *x, t_signal **sp)
{
    dsp_add( randomblock_perform, 3, sp[0]->s_vec, sp[0]->s_n, x ) ;
}

void randomblock_tilde_setup(void)
{
    logpost(NULL, 4, randomblock_version);
    randomblock_class = class_new(gensym("randomblock~"), (t_newmethod)randomblock_new,
                                  (t_method)randomblock_free,
                                  sizeof(t_randomblock), 0, A_DEFFLOAT, 0);
    class_addmethod( randomblock_class, (t_method)randomblock_dsp, gensym("dsp"), 0);
    class_addmethod( randomblock_class, (t_method)randomblock_limit, gensym("limit"), A_FLOAT, 0);
}
