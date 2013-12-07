/* ------------------------ formant~ ------------------------------------------ */
/*                                                                              */
/* Tilde object to produce formant synthesis.                                   */
/* By using several ones, you should be able to generate voice sounds.          */
/* Written by Yves Degoyon ( ydegoyon@free.fr )                                 */
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
/* Also inspired by an article by Jean-Paul Smets - Solanes                     */
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
#include <math.h>
#ifdef _MSC_VER
# define M_PI 3.14159265358979323846
#else
# include <unistd.h>
#endif

#include <m_pd.h>            /* standard pd stuff */

static char   *formant_version = "formant~: formant synthesis version 0.1, written by Yves Degoyon";


static t_class *formant_class;

typedef struct _formant
{
    t_object x_obj;

    t_int x_size;                 /* size of the sample */
    t_float x_central_freq;       /* central frequency of the filter */
    t_float x_filter_width;       /* width of the filter */
    t_float x_skirt_width;        /* width of the skirt */
    t_float x_samplerate;         /* sample rate */
    t_float x_time_stretch;        /* time stretch */
    t_int x_readpos;              /* data's reading position */
    t_int x_play;                 /* playing on/off flag */
    t_outlet *x_end;              /* outlet for outputing a bang at the end */
    t_float x_gendata;            /* flag to avoid reading data during generation ( might be null )*/
    t_float* x_data;              /* sample containing formant synthesis*/

} t_formant;

/* clean up */
static void formant_free(t_formant *x)
{
    if ( x->x_data != NULL )
    {
        freebytes(x->x_data, x->x_size*sizeof(float) );
        x->x_data = NULL;
    }
}

/* generate sample data */
static t_int formant_gendata(t_formant *x)
{
    t_float t, b, fs;

    if ( x->x_size <= 0 || x->x_central_freq <= 0 || x->x_filter_width <= 0 || x->x_skirt_width <= 0 )
    {
        error( "formant~ : error generating data : negative or null parameter(s)" );
        return -1;
    }

    x->x_gendata = 1;

    /* freeing data */
    formant_free( x );
    if ( !( x->x_data = (float*) getbytes( x->x_size*sizeof(float) ) ) )
    {
        post( "formant~ : error generating data : cannot allocate table" );
        return -1;
    }

    fs = 0;
    while( fs < x->x_size-1 )
    {
        t = (fs/x->x_samplerate) * x->x_time_stretch; /* time taken from zero */
        b = M_PI / x->x_skirt_width ;
        if ( t < b )
        {
            *(x->x_data+(int)fs) = 0.5*(1-cos(x->x_skirt_width*t))*exp(- x->x_filter_width*t )*sin( x->x_central_freq*t);
        }
        else
        {
            *(x->x_data+(int)fs) = exp(- x->x_filter_width*t )*sin( x->x_central_freq*t);
        }
        fs++;
    }

    /* everything went fine */
    x->x_gendata = 0;
    return 0;
}

/* generates a formant */
static t_int *formant_perform(t_int *w)
{
    t_formant *x = (t_formant *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    int n = (int)(w[3]);                      /* number of samples */

    while (n--)
    {
        if ( !x->x_gendata && x->x_play)
        {
            *out=*(x->x_data+x->x_readpos);
            x->x_readpos = (x->x_readpos+1)%x->x_size;
            if ( x->x_readpos == 0 )
            {
                x->x_play=0;
                outlet_bang(x->x_end);
            }
        }
        else
        {
            *out=0.0;
        }
        out++;
    }
    return (w+4);
}

static void formant_dsp(t_formant *x, t_signal **sp)
{
    dsp_add(formant_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}

/* replay the sample */
static void formant_bang(t_formant *x, t_floatarg fsize)
{
    x->x_play=1;
    x->x_readpos=0;
}

/* set size of the sample */
static void formant_size(t_formant *x, t_floatarg fsize)
{
    if ( fsize <= 0 )
    {
        post( "formant~ : error : sample size should be >0" );
        return;
    }
    x->x_size = fsize;
    formant_gendata( x );
}

/* set central frequency of the formant */
static void formant_central_freq(t_formant *x, t_floatarg ffreq)
{
    if ( ffreq <= 0 )
    {
        post( "formant~ : error : filter central frequency should be >0" );
        return;
    }
    x->x_central_freq = ffreq;
    formant_gendata( x );
}

/* set filter width of the formant */
static void formant_filter_width(t_formant *x, t_floatarg fwidth)
{
    if ( fwidth <= 0 )
    {
        post( "formant~ : error : filter width should be >0" );
        return;
    }
    x->x_filter_width = fwidth;
    formant_gendata( x );
}

/* set skirt width of the formant */
static void formant_skirt_width(t_formant *x, t_floatarg swidth)
{
    if ( swidth <= 0 )
    {
        post( "formant~ : error : skirt width should be >0" );
        return;
    }
    x->x_skirt_width = swidth;
    formant_gendata( x );
}

/* set time stretch factor */
static void formant_time_stretch(t_formant *x, t_floatarg fstretch)
{
    if ( fstretch <= 0 )
    {
        post( "formant~ : error : time stretch should be >0" );
        return;
    }
    x->x_time_stretch = fstretch;
    formant_gendata( x );
}

static void *formant_new(t_floatarg fsize, t_floatarg ffreq, t_floatarg ffwidth, t_floatarg fswidth)
{
    t_formant *x = (t_formant *)pd_new(formant_class);
    outlet_new(&x->x_obj, &s_signal);

    if ( fsize <= 0 || ffreq <= 0 || ffwidth <= 0 || fswidth <= 0 )
    {
        error( "formant~ : missing or negative creation arguments" );
        return NULL;
    }

    x->x_size = fsize;
    x->x_central_freq = ffreq;
    x->x_filter_width = ffwidth;
    x->x_skirt_width = fswidth;
    x->x_readpos = 0;
    x->x_data = NULL;
    x->x_time_stretch = 1.0;
    x->x_samplerate = sys_getsr();
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("size"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("freq"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fwidth"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("swidth"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("stretch"));
    x->x_end = outlet_new( &x->x_obj, &s_bang );
    if ( formant_gendata( x ) )
    {
        post( "formant~ : error generating data" );
        return NULL;
    }
    else
    {
        return(x);
    }
}

void formant_tilde_setup(void)
{
    logpost(NULL, 4, formant_version);
    formant_class = class_new(gensym("formant~"), (t_newmethod)formant_new, (t_method)formant_free,
                              sizeof(t_formant), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(formant_class, (t_method)formant_dsp, gensym("dsp"), 0);
    class_addmethod(formant_class, (t_method)formant_size, gensym("size"), A_FLOAT, 0);
    class_addmethod(formant_class, (t_method)formant_bang, gensym("bang"), 0);
    class_addmethod(formant_class, (t_method)formant_central_freq, gensym("freq"), A_FLOAT, 0);
    class_addmethod(formant_class, (t_method)formant_filter_width, gensym("fwidth"), A_FLOAT, 0);
    class_addmethod(formant_class, (t_method)formant_skirt_width, gensym("swidth"), A_FLOAT, 0);
    class_addmethod(formant_class, (t_method)formant_time_stretch, gensym("stretch"), A_FLOAT, 0);
}
