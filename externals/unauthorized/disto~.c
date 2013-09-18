/* Copyleft (c) 2002 Yves Degoyon.                                              */
/* Copyright (c) 2001 Alexei Smoli                                              */
/* For information on usage and redistribution, and for a DISCLAIMER OF ALL     */
/* WARRANTIES, see the file, "COPYING"  in this distribution.                   */
/*                                                                              */
/* disto~ -- a kind of effect used in pop music                                 */
/* the algorithm was taken from Digital Effects (DISTORT3),                     */
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
/* Bruce Gilbert -- Ab Ovo                                                      */
/* Poison Girls -- Promenade Immortelle                                         */
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
#define NBEXPS          129

static char   *disto_version = "disto~: distortion, version 0.1 (ydegoyon@free.fr)";

struct hipass
{
    /* few intermediate variables */
    double omega;
    double sn,cs;
    double alpha;
    /* filter coefficients */
    double a0,a1,a2,b0,b1,b2;
    double k0,k1,k2,k3,k4;
    /* amplitudes */
    double x0,x1,x2;
    double y0,y1,y2;
} hipass;

struct lowpass
{
    /* few intermediate variables */
    double omega;
    double sn,cs;
    double alpha;
    /* filter coefficients */
    double a0,a1,a2,b0,b1,b2;
    double k0,k1,k2,k3,k4;
    /* amplitudes */
    double x0,x1,x2;
    double y0,y1,y2;
} lowpass;

typedef struct _disto
{
    t_object x_obj;
    double *x_buf;
    t_int x_samplerate;
    double x_drive;       /* distortion drive */                            /* 0<= <=25 */
    double x_drymix;      /* dry (unaffected) signal mix */                 /* -5<= <=5 */
    double x_wetmix;      /* wet (affected) signal mix */                   /* -5<= <=5 */
    double x_feedback;    /* feedback */                                    /* -10<= <=10 */
    double x_volume;      /* distortion volume */                           /* 0=< <=5 */
    double x_hipassfreq;  /* cutoff frequency for hi pass filter */         /* 0< <RATE/2 */
    double x_lowpassfreq; /* cutoff frequency for low pass filter */        /* 0< <RATE/2 */
    double x_hipassQ;     /* the EE kinda definition for hi pass filter */  /* 0.1< <=1 */
    double x_lowpassQ;    /* the EE kinda definition for low pass filter */ /* 0.1< <=1 */

    /* audio processing data ( not setable ) */
    double data,pred;
    double outval,outvol;
    double exps[NBEXPS];

    /* filters data ( not setable ) */
    struct hipass HPF;
    struct lowpass LPF;

    t_float x_f;
} t_disto;

static t_class *disto_class;

static void disto_init_filters (t_disto *x)
{
    t_int i;

    for (i=0; i<130; i++)
    {
        x->exps[i]=exp((double)i*x->x_drive)*x->x_wetmix;
    }

    x->HPF.omega = 2.0*M_PI*x->x_hipassfreq/(double)x->x_samplerate;
    x->HPF.sn    = sin(x->HPF.omega);
    x->HPF.cs    = cos(x->HPF.omega);
    x->HPF.alpha = x->HPF.sn/(2.0*x->x_hipassQ);
    x->HPF.b0 =  (1.0 + x->HPF.cs)/2.0;
    x->HPF.b1 = -(1.0 + x->HPF.cs)    ;
    x->HPF.b2 =  (1.0 + x->HPF.cs)/2.0;
    x->HPF.a0 =   1.0 + x->HPF.alpha  ;
    x->HPF.a1 =  -2.0*x->HPF.cs       ;
    x->HPF.a2 =   1.0 - x->HPF.alpha  ;
    x->HPF.k0 = (x->HPF.b0/x->HPF.a0);
    x->HPF.k1 = (x->HPF.b1/x->HPF.a0);
    x->HPF.k2 = (x->HPF.b2/x->HPF.a0);
    x->HPF.k3 = (x->HPF.a1/x->HPF.a0);
    x->HPF.k4 = (x->HPF.a2/x->HPF.a0);

    x->LPF.omega = 2.0*M_PI*x->x_lowpassfreq/(double)x->x_samplerate;
    x->LPF.sn    = sin(x->LPF.omega);
    x->LPF.cs    = cos(x->LPF.omega);
    x->LPF.alpha = x->LPF.sn/(2.0*x->x_lowpassQ);
    x->LPF.b0 =  (1.0 - x->LPF.cs)/2.0;
    x->LPF.b1 =   1.0 - x->LPF.cs     ;
    x->LPF.b2 =  (1.0 - x->LPF.cs)/2.0;
    x->LPF.a0 =   1.0 + x->LPF.alpha  ;
    x->LPF.a1 =  -2.0*x->LPF.cs       ;
    x->LPF.a2 =   1.0 - x->LPF.alpha  ;
    x->LPF.k0 = (x->LPF.b0/x->LPF.a0);
    x->LPF.k1 = (x->LPF.b1/x->LPF.a0);
    x->LPF.k2 = (x->LPF.b2/x->LPF.a0);
    x->LPF.k3 = (x->LPF.a1/x->LPF.a0);
    x->LPF.k4 = (x->LPF.a2/x->LPF.a0);
}

static void disto_drive(t_disto *x, t_floatarg fdrive )
{
    if ( fdrive > 25.0 )
    {
        fdrive = 25.0;
    }
    if ( fdrive < 0.0 )
    {
        fdrive = 0.0;
    }
    x->x_drive = fdrive;
    // post( "disto~ : drive: %f", x->x_drive );
    disto_init_filters( x );
}

static void disto_drymix(t_disto *x, t_floatarg fdrymix )
{
    if ( fdrymix > 5.0 )
    {
        fdrymix = 5.0;
    }
    if ( fdrymix < -5.0 )
    {
        fdrymix = -5.0;
    }
    x->x_drymix = fdrymix;
    // post( "disto~ : drymix: %f", x->x_drymix );
    disto_init_filters( x );
}

static void disto_wetmix(t_disto *x, t_floatarg fwetmix )
{
    if ( fwetmix > 5.0 )
    {
        fwetmix = 5.0;
    }
    if ( fwetmix < -5.0 )
    {
        fwetmix = -5.0;
    }
    x->x_wetmix = fwetmix;
    // post( "disto~ : wetmix: %f", x->x_wetmix );
    disto_init_filters( x );
}

static void disto_feedback(t_disto *x, t_floatarg ffeedback )
{
    if ( ffeedback > 10.0 )
    {
        ffeedback = 10.0;
    }
    if ( ffeedback < -10.0 )
    {
        ffeedback = -10.0;
    }
    x->x_feedback = ffeedback;
    // post( "disto~ : feedback: %f", x->x_feedback );
    disto_init_filters( x );
}

static void disto_volume(t_disto *x, t_floatarg fvolume )
{
    if ( fvolume > 5.0 )
    {
        fvolume = 5.0;
    }
    if ( fvolume < 0.0 )
    {
        fvolume = 0.0;
    }
    x->x_volume = fvolume;
    // post( "disto~ : volume: %f", x->x_volume );
    disto_init_filters( x );
}

static void disto_hipassfreq(t_disto *x, t_floatarg fhipassfreq )
{
    if ( fhipassfreq > x->x_samplerate/2 )
    {
        fhipassfreq = x->x_samplerate/2;
    }
    if ( fhipassfreq < 0.0 )
    {
        fhipassfreq = 0.0;
    }
    x->x_hipassfreq = fhipassfreq;
    // post( "disto~ : hipassfreq: %f", x->x_hipassfreq );
    disto_init_filters( x );
}

static void disto_hipassQ(t_disto *x, t_floatarg fhipassQ )
{
    if ( fhipassQ > 1.0 )
    {
        fhipassQ = 1.0;
    }
    if ( fhipassQ < 0.1 )
    {
        fhipassQ = 0.1;
    }
    x->x_hipassQ = fhipassQ;
    // post( "disto~ : hipassQ: %f", x->x_hipassQ );
    disto_init_filters( x );
}

static void disto_lowpassfreq(t_disto *x, t_floatarg flowpassfreq )
{
    if ( flowpassfreq > x->x_samplerate/2 )
    {
        flowpassfreq = x->x_samplerate/2;
    }
    if ( flowpassfreq < 0.0 )
    {
        flowpassfreq = 0.0;
    }
    x->x_lowpassfreq = flowpassfreq;
    // post( "disto~ : lowpassfreq: %f", x->x_lowpassfreq );
    disto_init_filters( x );
}

static void disto_lowpassQ(t_disto *x, t_floatarg flowpassQ )
{
    if ( flowpassQ > 1.0 )
    {
        flowpassQ = 1.0;
    }
    if ( flowpassQ < 0.1 )
    {
        flowpassQ = 0.1;
    }
    x->x_lowpassQ = flowpassQ;
    // post( "disto~ : lowpassQ: %f", x->x_lowpassQ );
    disto_init_filters( x );
}

static t_int *disto_perform(t_int *w)
{
    t_float *in = (t_float *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    t_int n = (int)(w[3]);
    t_disto *x = (t_disto*)(w[4]);
    t_int i;

    for (i = 0; i < n; i++)
    {

        x->HPF.x0 = *(in++);
        x->HPF.y0 = (x->HPF.k0*x->HPF.x0+x->HPF.k1*x->HPF.x1+x->HPF.k2*x->HPF.x2-x->HPF.k3*x->HPF.y1-x->HPF.k4*x->HPF.y2);
        x->HPF.y2 = x->HPF.y1;
        x->HPF.y1 = x->HPF.y0;
        x->HPF.x2 = x->HPF.x1;
        x->HPF.x1 = x->HPF.x0;
        x->data = (int)x->HPF.y0;

        if ((x->data-x->pred)>0)
            x->outval += (x->data*x->x_drymix+ x->exps[abs(x->data)]);
        else if ((x->data-x->pred)<0)
            x->outval += (x->data*x->x_drymix- x->exps[abs(x->data)]);
        x->pred = x->data;

        x->LPF.x0 = *(out);
        x->LPF.y0 = (x->LPF.k0*x->LPF.x0+x->LPF.k1*x->LPF.x1+x->LPF.k2*x->LPF.x2-x->LPF.k3*x->LPF.y1-x->LPF.k4*x->LPF.y2);
        x->LPF.y2 = x->LPF.y1;
        x->LPF.y1 = x->LPF.y0;
        x->LPF.x2 = x->LPF.x1;
        x->LPF.x1 = x->LPF.x0;

        x->outvol = x->LPF.y0*x->x_volume;

        if(x->outvol > 1.0)
            x->data = 1.0;
        else if(x->outvol < -1.0)
            x->data = -1.0;
        else
            x->data = x->outvol;

        *(out++) = x->data;

        x->outval *= x->x_feedback;

    }

    return (w+5);
}

static void disto_preset(t_disto *x, t_float pnumber)
{
    switch ( (int)pnumber )
    {
        /* "Hard Distortion 100-10000Hz" */
    case 1:
        x->x_drive = 1.5;
        x->x_drymix = 1.0;
        x->x_wetmix = 0.5;
        x->x_feedback = 0.0;
        x->x_volume = 1.0;
        x->x_hipassfreq = 100.0;
        x->x_hipassQ = 0.5;
        x->x_lowpassfreq = 10000.0;
        x->x_lowpassQ = 0.5;
        break;

        /* "Hard Distortion 100-6000Hz" */
    case 2:
        x->x_drive = 1.5;
        x->x_drymix = 1.0;
        x->x_wetmix = 0.5;
        x->x_feedback = 0.0;
        x->x_volume = 1.0;
        x->x_hipassfreq = 100.0;
        x->x_hipassQ = 0.5;
        x->x_lowpassfreq = 2000.0;
        x->x_lowpassQ = 0.5;
        break;

        /* "Very Hard Distortion" */
    case 3:
        x->x_drive = 2.0;
        x->x_drymix = 0.0;
        x->x_wetmix = 1.0;
        x->x_feedback = 1.0;
        x->x_volume = 5.0;
        x->x_hipassfreq = 100.0;
        x->x_hipassQ = 0.5;
        x->x_lowpassfreq = 6000.0;
        x->x_lowpassQ = 0.5;
        break;

        /* "Medium Distortion 0.2" */
    case 4:
        x->x_drive = 0.2;
        x->x_drymix = 1.0;
        x->x_wetmix = 1.0;
        x->x_feedback = 0.1;
        x->x_volume = 1.0;
        x->x_hipassfreq = 100.0;
        x->x_hipassQ = 0.5;
        x->x_lowpassfreq = 6000.0;
        x->x_lowpassQ = 0.5;
        break;

        /* "Medium Distortion 0.8" */
    case 5:
        x->x_drive = 0.8;
        x->x_drymix = 1.0;
        x->x_wetmix = 1.0;
        x->x_feedback = 0.1;
        x->x_volume = 1.0;
        x->x_hipassfreq = 100.0;
        x->x_hipassQ = 1.0;
        x->x_lowpassfreq = 6000.0;
        x->x_lowpassQ = 0.5;
        break;

        /* "Soft Distortion 0.8" */
    case 6:
        x->x_drive = 0.8;
        x->x_drymix = 0.4;
        x->x_wetmix = 0.8;
        x->x_feedback = 0.0;
        x->x_volume = 0.5;
        x->x_hipassfreq = 100.0;
        x->x_hipassQ = 1.0;
        x->x_lowpassfreq = 10000.0;
        x->x_lowpassQ = 0.5;
        break;

    default:
        post( "disto~ : unknown preset requested : %d", pnumber );
        return;
        break;
    }
    disto_init_filters( x );
}

static void disto_dsp(t_disto *x, t_signal **sp)
{
    dsp_add(disto_perform, 4, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n, x );
}

/* clean up */
static void disto_free(t_disto *x)
{
    if ( x->x_buf != NULL )
    {
        freebytes(x->x_buf, BFSZ*sizeof( double ) );
        post( "Freed %d bytes",  BFSZ*sizeof( double ) );
        x->x_buf = NULL;
    }
}

static void *disto_new(void)
{
    t_disto *x = (t_disto *)pd_new(disto_class);
    outlet_new(&x->x_obj, &s_signal);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("drive"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("drymix"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("wetmix"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("feedback"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("volume"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("hipassfreq"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("hipassQ"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("lowpassfreq"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("lowpassQ"));

    x->x_samplerate = (int)sys_getsr();
    x->pred = 0.0;
    x->data = 0.0;

    if ( !( x->x_buf = ( double* ) getbytes( BFSZ*sizeof( double ) ) ) )
    {
        post ("disto~ : could not allocate buffer" );
        return NULL;
    }

    // set default parameters
    disto_preset( x, 3 );
    disto_init_filters( x );

    return (x);
}

void disto_tilde_setup(void)
{
    logpost(NULL, 4,  disto_version );
    disto_class = class_new(gensym("disto~"), (t_newmethod)disto_new, (t_method)disto_free,
                            sizeof(t_disto), 0, 0);

    CLASS_MAINSIGNALIN( disto_class, t_disto, x_f );
    class_addmethod(disto_class, (t_method)disto_drive, gensym("drive"), A_FLOAT, 0);
    class_addmethod(disto_class, (t_method)disto_drymix, gensym("drymix"), A_FLOAT, 0);
    class_addmethod(disto_class, (t_method)disto_wetmix, gensym("wetmix"), A_FLOAT, 0);
    class_addmethod(disto_class, (t_method)disto_feedback, gensym("feedback"), A_FLOAT, 0);
    class_addmethod(disto_class, (t_method)disto_volume, gensym("volume"), A_FLOAT, 0);
    class_addmethod(disto_class, (t_method)disto_hipassfreq, gensym("hipassfreq"), A_FLOAT, 0);
    class_addmethod(disto_class, (t_method)disto_hipassQ, gensym("hipassQ"), A_FLOAT, 0);
    class_addmethod(disto_class, (t_method)disto_lowpassfreq, gensym("lowpassfreq"), A_FLOAT, 0);
    class_addmethod(disto_class, (t_method)disto_lowpassQ, gensym("lowpassQ"), A_FLOAT, 0);
    class_addmethod(disto_class, (t_method)disto_dsp, gensym("dsp"), 0);
    class_addmethod(disto_class, (t_method)disto_preset, gensym("preset"), A_FLOAT, 0);
}
