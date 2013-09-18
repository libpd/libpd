/* Copyright (c) 2002 Yves Degoyon.                                             */
/* For information on usage and redistribution, and for a DISCLAIMER OF ALL     */
/* WARRANTIES, see the file, "COPYING"  in this distribution.                   */
/*                                                                              */
/* wahwah~ -- a kind of effect used in psychedelic music                        */
/* the algorithm was taken from Digital Effects,                                */
/* a guitar effects software for DOS which rocks, written by Alexey Smoli       */
/* ( http://st.karelia.ru/~smlalx/ )                                            */
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
/* Crass -- "Shaved Women ( Collaborators )"                                    */
/* Zounds -- "Can't Cheat Karma"                                                */
/*                                                                              */
/* Special message for the french :                                             */
/* "Delay all your work...and go vote against national front"                   */
/* ---------------------------------------------------------------------------- */

#include "m_pd.h"
#include <stdlib.h>
#include <math.h>

#ifdef _MSC_VER
#define M_PI 3.14159265358979323846
#endif

#define BFSZ            4096   /* main delay buffer                  */
#define BLOCK_DELAY     10     /* number of blocks to delay          */
#define NA              0.0    /* param not applicable               */
#define COEFFSIZE       3000   /* coefficients size                  */
#define NBCOEFFS        5      /* number of coefficients             */

static char   *wahwah_version = "wahwah~: an audio wahwah, version 0.1 (ydegoyon@free.fr)";

typedef struct _wahwah
{
    t_object x_obj;
    double *x_buf;
    t_int x_samplerate;
    t_float *x_coeffs;
    double x_minfc;       /* min frequency Hz */                                            /* 0< <3000 */
    double x_maxfc;       /* max frequency Hz */                                            /* 0< <3000 */
    double x_sense;       /* increase/decrease frequency change by one mouse movement Hz */ /* 0< <=1000 */
    double x_maxstep;     /* maximum frequency change step KHz */                           /* 0< <=100 */
    double x_dbgain;      /* peaking filter gain dB */                                      /* -15<= <=15 */
    double x_bandwidth;   /* bandwidth in octaves (between midpoint (dBgain/2)
                             gain frequencies) or */                                        /* 0< <10 */
    double x_Q;           /* the EE kinda definition */                                     /* 0< <=1 */

    /* variables for audio computation */
    double min_coef,max_coef,cur_coef,step,maxstep;
    t_int down;
    /* amplitudes */
    short x0,x1,x2;
    t_int y0,y1,y2;

    t_float x_f;
} t_wahwah;

static t_class *wahwah_class;

static void wahwah_set_coeffs (t_wahwah *x)
{
    t_int i;
    double omega;
    double sn,cs;
    double alpha = 0.0;
    /* filter coefficients */
    double a0,a1,a2,b0,b1,b2;
    double A;

    A   = exp(x->x_dbgain/40.0)*log(10.0);
    for (i = 0; i < COEFFSIZE; i++)
    {
        omega = 2.0*M_PI*(double)i/(double)x->x_samplerate;
        sn    = sin(omega);
        cs    = cos(omega);
        if (x->x_bandwidth)
            alpha = sn*sin(log(2.0)/2.0*x->x_bandwidth*omega/sn);
        else
            /* if Q is specified instead of bandwidth */
            if (x->x_Q)
                alpha = sn/(2.0*x->x_Q);

        /* then compute the coefs for whichever filter type you want */
        b0 =   1.0 + alpha*A;
        b1 =  -2.0*cs       ;
        b2 =   1.0 - alpha*A;
        a0 =   1.0 + alpha/A;
        a1 =  -2.0*cs       ;
        a2 =   1.0 - alpha/A;

        *(x->x_coeffs+i*NBCOEFFS) = (b0/a0);
        *(x->x_coeffs+i*NBCOEFFS+1) = (b1/a0);
        *(x->x_coeffs+i*NBCOEFFS+2) = (b2/a0);
        *(x->x_coeffs+i*NBCOEFFS+3) = (a1/a0);
        *(x->x_coeffs+i*NBCOEFFS+4) = (a2/a0);
        // post( "wahwah~ : coeff : %f", *(x->x_coeffs+i*NBCOEFFS+4) );
    }
    x->min_coef = x->x_minfc;
    x->max_coef = x->x_maxfc;
    x->cur_coef = x->min_coef;
    x->down = 1;
    x->step = 0;
    x->maxstep = x->x_maxstep/(double)x->x_samplerate*1000.0;
}

static void wahwah_bandwidth(t_wahwah *x, t_floatarg fbandwidth )
{
    if ( fbandwidth > 10.0 )
    {
        fbandwidth = 10.0;
    }
    if ( fbandwidth < 0.0 )
    {
        fbandwidth = 0.0;
    }
    x->x_bandwidth = fbandwidth;
    // post( "wahwah~ : bandwidth: %f", x->x_bandwidth );
    wahwah_set_coeffs( x );
}

static void wahwah_dbgain(t_wahwah *x, t_floatarg fdbgain )
{
    if ( fdbgain > 15.0 )
    {
        fdbgain = 15.0;
    }
    if ( fdbgain < -15.0 )
    {
        fdbgain = -15.0;
    }
    x->x_dbgain = fdbgain;
    // post( "wahwah~ : dbgain: %f", x->x_dbgain );
    wahwah_set_coeffs( x );
}

static void wahwah_maxstep(t_wahwah *x, t_floatarg fmaxstep )
{
    if ( fmaxstep > 100.0 )
    {
        fmaxstep = 100.0;
    }
    if ( fmaxstep < 0.0 )
    {
        fmaxstep = 0.0;
    }
    x->x_maxstep = fmaxstep;
    // post( "wahwah~ : maxstep: %f", x->x_maxstep );
    wahwah_set_coeffs( x );
}

static void wahwah_sensibility(t_wahwah *x, t_floatarg fsensibility )
{
    if ( fsensibility > 1000.0 )
    {
        fsensibility = 1000.0;
    }
    if ( fsensibility < 0.0 )
    {
        fsensibility = 0.0;
    }
    x->x_sense = fsensibility;
    // post( "wahwah~ : sensibility: %f", x->x_sense );
    wahwah_set_coeffs( x );
}

static void wahwah_maxfreq(t_wahwah *x, t_floatarg fmaxfreq )
{
    if ( fmaxfreq > 3000.0 )
    {
        fmaxfreq = 3000.0;
    }
    if ( fmaxfreq < 0.0 )
    {
        fmaxfreq = 0.0;
    }
    x->x_maxfc = fmaxfreq;
    // post( "wahwah~ : maxfreq: %f", x->x_maxfc );
    wahwah_set_coeffs( x );
}

static void wahwah_minfreq(t_wahwah *x, t_floatarg fminfreq )
{
    if ( fminfreq > 3000.0 )
    {
        fminfreq = 3000.0;
    }
    if ( fminfreq < 0.0 )
    {
        fminfreq = 0.0;
    }
    x->x_minfc = fminfreq;
    // post( "wahwah~ : minfreq: %f", x->x_minfc );
    wahwah_set_coeffs( x );
}

static void wahwah_step(t_wahwah *x, t_floatarg fstep )
{
    if ( fstep > x->x_maxstep )
    {
        fstep = x->x_maxstep;
    }
    if ( fstep < 0 )
    {
        fstep = 0;
    }
    x->step = fstep*x->x_sense/x->x_samplerate;
}

static t_int *wahwah_perform(t_int *w)
{
    t_float *in = (t_float *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    t_int n = (int)(w[3]);
    t_wahwah *x = (t_wahwah*)(w[4]);
    t_int i;

    for (i=0; i<n; i++)
    {
        /* read input */
        x->x0 = (int)((*(in+i))*32768);

        x->y0 = (*(x->x_coeffs+(int)x->cur_coef*NBCOEFFS))*x->x0
                + (*(x->x_coeffs+(int)x->cur_coef*NBCOEFFS+1))*x->x1
                + (*(x->x_coeffs+(int)x->cur_coef*NBCOEFFS+2))*x->x2
                - (*(x->x_coeffs+(int)x->cur_coef*NBCOEFFS+3))*x->y1
                - (*(x->x_coeffs+(int)x->cur_coef*NBCOEFFS+4))*x->y2;

        x->y2 = x->y1;
        x->y1 = x->y0;
        x->x2 = x->x1;
        x->x1 = x->x0;

        if(x->y0 > 32767.0)
            x->y0 = 32767.0;
        else if(x->y0 < -32768.0)
            x->y0 = -32768.0;

        *(out+i) = (t_float)(x->y0) / 32768.0;

        x->cur_coef = x->cur_coef+x->down*x->step*(abs(x->x0)/(0.5*32768.0));
        // post ( "wahwah~ : cur coeff : %f", x->cur_coef );
        if(x->cur_coef > x->max_coef)
        {
            x->cur_coef = x->max_coef;
            x->down = -1;
        }
        else if(x->cur_coef < x->min_coef)
        {
            x->cur_coef = x->min_coef;
            x->down = 1;
        }

    }

    return (w+5);
}

static void wahwah_preset(t_wahwah *x, t_float pnumber)
{
    switch ( (int)pnumber )
    {
        /* fast change medium wah wah */
    case 1:
        x->x_minfc = 100.0;
        x->x_maxfc = 1600.0;
        x->x_sense = 100.0;
        x->x_maxstep = 60.0;
        x->x_dbgain = 15.0;
        x->x_Q = 1.0;
        break;

        /* slow change medium wah wah */
    case 2:
        x->x_minfc = 100.0;
        x->x_maxfc = 1600.0;
        x->x_sense = 50.0;
        x->x_maxstep = 36.0;
        x->x_dbgain = 15.0;
        x->x_Q = 1.0;
        break;

        /* fast wah wah */
    case 3:
        x->x_minfc = 100.0;
        x->x_maxfc = 600.0;
        x->x_sense = 100.0;
        x->x_maxstep = 66.0;
        x->x_dbgain = 15.0;
        x->x_Q = 1.0;
        break;

        /* ranged wah wah */
    case 4:
        x->x_minfc = 10.0;
        x->x_maxfc = 2900.0;
        x->x_sense = 100.0;
        x->x_maxstep = 66.0;
        x->x_dbgain = 15.0;
        x->x_Q = 1.0;
        break;

        /* wah wah 400 - 2000 */
    case 5:
        x->x_minfc = 400.0;
        x->x_maxfc = 2000.0;
        x->x_sense = 100.0;
        x->x_maxstep = 66.0;
        x->x_dbgain = 15.0;
        x->x_Q = 1.0;
        break;

    default:
        post( "wahwah~ : unknown preset requested : %d", pnumber );
        return;
        break;
    }
    wahwah_set_coeffs( x );
}

static void wahwah_dsp(t_wahwah *x, t_signal **sp)
{
    dsp_add(wahwah_perform, 4, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n, x );
}

/* clean up */
static void wahwah_free(t_wahwah *x)
{
    if ( x->x_buf != NULL )
    {
        freebytes(x->x_buf, BFSZ*sizeof( double ) );
        post( "Freed %d bytes",  BFSZ*sizeof( double ) );
        x->x_buf = NULL;
    }
    if ( x->x_coeffs != NULL )
    {
        freebytes(x->x_coeffs, COEFFSIZE*NBCOEFFS*sizeof( t_float ) );
        post( "Freed %d bytes", COEFFSIZE*NBCOEFFS*sizeof( t_float ));
        x->x_coeffs = NULL;
    }
}

static void *wahwah_new(void)
{
    t_wahwah *x = (t_wahwah *)pd_new(wahwah_class);
    outlet_new(&x->x_obj, &s_signal);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("minfreq"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("maxfreq"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("sensibility"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("maxstep"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("dbgain"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("bandwidth"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("step"));

    x->x_samplerate = (int)sys_getsr();

    if ( !( x->x_buf = ( double* ) getbytes( BFSZ*sizeof( double ) ) ) )
    {
        post ("wahwah~ : could not allocate buffer" );
        return NULL;
    }
    if ( !( x->x_coeffs = ( t_float* ) getbytes( COEFFSIZE*NBCOEFFS*sizeof( t_float ) ) ) )
    {
        post ("wahwah~ : could not allocate coeffs" );
        return NULL;
    }

    // set default parameters
    wahwah_preset( x, 1 );
    wahwah_set_coeffs( x );

    return (x);
}

void wahwah_tilde_setup(void)
{
    logpost(NULL, 4,  wahwah_version );
    wahwah_class = class_new(gensym("wahwah~"), (t_newmethod)wahwah_new, (t_method)wahwah_free,
                             sizeof(t_wahwah), 0, 0);

    CLASS_MAINSIGNALIN( wahwah_class, t_wahwah, x_f );
    class_addmethod(wahwah_class, (t_method)wahwah_step, gensym("step"), A_FLOAT, 0);
    class_addmethod(wahwah_class, (t_method)wahwah_minfreq, gensym("minfreq"), A_FLOAT, 0);
    class_addmethod(wahwah_class, (t_method)wahwah_maxfreq, gensym("maxfreq"), A_FLOAT, 0);
    class_addmethod(wahwah_class, (t_method)wahwah_sensibility, gensym("sensibility"), A_FLOAT, 0);
    class_addmethod(wahwah_class, (t_method)wahwah_maxstep, gensym("maxstep"), A_FLOAT, 0);
    class_addmethod(wahwah_class, (t_method)wahwah_dbgain, gensym("dbgain"), A_FLOAT, 0);
    class_addmethod(wahwah_class, (t_method)wahwah_bandwidth, gensym("bandwidth"), A_FLOAT, 0);
    class_addmethod(wahwah_class, (t_method)wahwah_dsp, gensym("dsp"), 0);
    class_addmethod(wahwah_class, (t_method)wahwah_preset, gensym("preset"), A_FLOAT, 0);
}
