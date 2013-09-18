/* ------------------------ countund~ -------------------------------------- */
/*                                                                              */
/* Count up to a value and then down to zero                                    */
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

#include <m_pd.h>            /* standard pd stuff */

static char   *countund_version = "countund~: count up to a value and then down to zero : author : ydegoyon@free.fr";

static t_class *countund_class;

typedef struct _countund
{
    t_object x_obj;
    t_int x_limit;
    t_int x_value;
    t_int x_up;
} t_countund;

/* clean up */
static void countund_free(t_countund *x)
{
}

static void *countund_new(t_float flimit)
{
    t_countund *x = (t_countund *)pd_new(countund_class);
    outlet_new(&x->x_obj, &s_float);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("limit"));
    if ( flimit < 0 )
    {
        post( "countund~: wrong creation argument" );
        return NULL;
    }
    x->x_limit = (int) flimit;
    x->x_value = 0;
    x->x_up = 1;
    return(x);
}

static void *countund_limit(t_countund* x, t_float flimit)
{
    if ( flimit < 0 )
    {
        post( "countund~: wrong count limit" );
        return;
    }
    else
    {
        x->x_limit=(int)flimit;
    }
}

static void *countund_bang(t_countund *x)
{

    if ( x->x_up )
    {
        x->x_value+=1;
        if (x->x_value>x->x_limit)
        {
            x->x_value=x->x_limit-1;
            x->x_up=0;
        }
    }
    else
    {
        x->x_value-=1;
        if (x->x_value<0)
        {
            x->x_value=1;
            x->x_up=1;
        }
    }
    outlet_float( x->x_obj.ob_outlet, x->x_value );
    return;
}

void countund_setup(void)
{
    logpost(NULL, 4, countund_version);
    countund_class = class_new(gensym("countund"), (t_newmethod)countund_new,
                               (t_method)countund_free,
                               sizeof(t_countund), 0, A_DEFFLOAT, 0);
    class_addmethod( countund_class, (t_method)countund_bang, &s_bang, 0);
    class_addmethod( countund_class, (t_method)countund_limit, gensym("limit"), A_FLOAT, 0);
}
