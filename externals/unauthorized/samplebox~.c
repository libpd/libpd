/*------------------------ samplebox~ ----------------------------------------- */
/*                                                                              */
/* samplebox~ : records and plays back a sound                                  */
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
/*                                                                              */
/* All i wanted is your time                                                    */
/* All you gave me was tomorrow                                                  */
/* But, tomorrow never comes, tomorrow never comes                              */
/* Vini Reilly -- "Tomorrow"                                                    */
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

#include "m_pd.h"            /* standard pd stuff */

static char   *samplebox_version = "samplebox~: stores and plays back a sound version 0.3, written by Yves Degoyon (ydegoyon@free.fr)";


static t_class *samplebox_class;

typedef struct _samplebox
{
    t_object x_obj;

    t_int x_size;                  /* size of the stored sound ( in blocks~ ) */
    t_float x_samplerate;          /* sample rate */
    t_int x_blocksize;             /* current block size ( might be modified by block~ object ) */
    t_float x_readpos;             /* data's playing position */
    t_int x_writepos;              /* data's recording position */
    t_int x_readstart;             /* data's starting position for reading */
    t_int x_readend;               /* data's ending position for reading */
    t_int x_modstart;              /* data's starting position for modifications */
    t_int x_modend;                /* data's ending position for modifications */
    t_int x_play;                  /* playing on/off flag */
    t_float x_readspeed;           /* number of grouped blocks for reading */
    t_float x_record;              /* flag to start recording process */
    t_float x_allocate;            /* flag to avoid reading data during generation */
    t_float *x_rdata;              /* table containing left channel of the sound */
    t_float *x_idata;              /* table containing right channel of the sound */
    t_float *x_rootsquares;        /* sum of the root squares of a block ( energy ) */
    t_float x_phase;               /* phase to apply on output */
    t_outlet *x_recordend;         /* outlet for end of recording */
    t_outlet *x_playend;           /* outlet for end of playing back */
    t_float x_f;                   /* float needed for signal input */

} t_samplebox;

/* clean up */
static void samplebox_free(t_samplebox *x)
{
    if ( x->x_rdata != NULL )
    {
        freebytes(x->x_rdata, x->x_size*x->x_blocksize*sizeof(float) );
        post( "Freed %d bytes", x->x_size*x->x_blocksize*sizeof(float) );
        x->x_rdata = NULL;
    }
    if ( x->x_idata != NULL )
    {
        freebytes(x->x_idata, x->x_size*x->x_blocksize*sizeof(float) );
        post( "Freed %d bytes", x->x_size*x->x_blocksize*sizeof(float) );
        x->x_idata = NULL;
    }
    if ( x->x_rootsquares != NULL )
    {
        freebytes(x->x_rootsquares, x->x_size*sizeof(float) );
        post( "Freed %d bytes", x->x_size*sizeof(float) );
        x->x_rootsquares = NULL;
    }
}

/* allocate tables for storing sound */
static t_int samplebox_allocate(t_samplebox *x)
{
    if ( !(x->x_rdata = getbytes( x->x_size*x->x_blocksize*sizeof(float) ) ) )
    {
        return -1;
    }
    else
    {
        post( "samplebox~ : allocated %d bytes", x->x_size*x->x_blocksize*sizeof(float) );
    }
    if ( !(x->x_idata = getbytes( x->x_size*x->x_blocksize*sizeof(float) ) ) )
    {
        return -1;
    }
    else
    {
        post( "samplebox~ : allocated %d bytes", x->x_size*x->x_blocksize*sizeof(float) );
    }
    if ( !(x->x_rootsquares = getbytes( x->x_size*sizeof(float) ) ) )
    {
        return -1;
    }
    else
    {
        post( "samplebox~ : allocated %d bytes", x->x_size*sizeof(float) );
    }
    return 0;
}

/* reallocate tables for storing sound */
static t_int samplebox_reallocate(t_samplebox *x, t_int ioldsize, t_int inewsize)
{
    t_float *prdata=x->x_rdata, *pidata=x->x_idata, *prootsquares=x->x_rootsquares;

    if ( !(x->x_rdata = getbytes( inewsize*x->x_blocksize*sizeof(float) ) ) )
    {
        return -1;
    }
    else
    {
        post( "samplebox~ : allocated %d bytes", inewsize*x->x_blocksize*sizeof(float) );
    }
    if ( !(x->x_idata = getbytes( inewsize*x->x_blocksize*sizeof(float) ) ) )
    {
        return -1;
    }
    else
    {
        post( "samplebox~ : allocated %d bytes", inewsize*x->x_blocksize*sizeof(float) );
    }
    if ( !(x->x_rootsquares = getbytes( inewsize*sizeof(float) ) ) )
    {
        return -1;
    }
    else
    {
        post( "samplebox~ : allocated %d bytes", inewsize*sizeof(float) );
    }
    if ( prdata != NULL )
    {
        freebytes(prdata, ioldsize*x->x_blocksize*sizeof(float) );
        post( "Freed %d bytes", ioldsize*x->x_blocksize*sizeof(float) );
    }
    if ( pidata != NULL )
    {
        freebytes(pidata, ioldsize*x->x_blocksize*sizeof(float) );
        post( "Freed %d bytes", ioldsize*x->x_blocksize*sizeof(float) );
    }
    if ( prootsquares != NULL )
    {
        freebytes(prootsquares, ioldsize*sizeof(float) );
        post( "Freed %d bytes", ioldsize*sizeof(float) );
    }
    return 0;
}

/* records or playback the sonogram */
static t_int *samplebox_perform(t_int *w)
{
    t_float *rin = (t_float *)(w[1]);
    t_float *iin = (t_float *)(w[2]);
    t_float *rout = (t_float *)(w[3]);
    t_float *iout = (t_float *)(w[4]);
    t_float fspectrum = 0.0;
    t_float fphase = 0.0;
    t_int   rpoint;
    t_int n = (int)(w[5]);                      /* number of samples */
    t_samplebox *x = (t_samplebox *)(w[6]);
    t_int bi;
    t_float v[4];
    t_float z[4];

    // reallocate tables if blocksize has been changed
    if ( n != x->x_blocksize )
    {
        post( "samplebox~ : reallocating tables" );
        x->x_allocate = 1;
        samplebox_free(x);
        x->x_blocksize = n;
        samplebox_allocate(x);
        x->x_allocate = 0;
    }

    // new block : energy is set to zero
    // if ( x->x_record ) {
    //   *(x->x_rootsquares+x->x_writepos) = 0.0;
    // }

    if ( x->x_play || x->x_record )
    {
        bi = 0;
        while (bi<n)
        {
            // eventually records input
            if ( !x->x_allocate && x->x_record)
            {
                *(x->x_rdata+(x->x_writepos*x->x_blocksize)+bi)=*rin;
                *(x->x_idata+(x->x_writepos*x->x_blocksize)+bi)=*iin;
                // *(x->x_rootsquares+x->x_writepos) += sqrt( pow((*rin),2) + pow((*iin),2) );
            }
            // set outputs
            if ( !x->x_allocate && x->x_play)
            {
                *rout = 0.;
                *iout = 0.;
                // interpolates 4 points like tabread4
                rpoint = ((int)x->x_readpos*x->x_blocksize)+bi;

                if ( rpoint == 0 )
                {
                    v[0]=0.0;
                    v[1]=*(x->x_rdata+rpoint);
                    v[2]=*(x->x_rdata+rpoint+1);
                    v[3]=*(x->x_rdata+rpoint+2);
                    z[0]=0.0;
                    z[1]=*(x->x_idata+rpoint);
                    z[2]=*(x->x_idata+rpoint+1);
                    z[3]=*(x->x_idata+rpoint+2);
                }
                else if ( rpoint == (x->x_size*x->x_blocksize-1) )
                {
                    v[0]=*(x->x_rdata+rpoint-1);
                    v[1]=*(x->x_rdata+rpoint);
                    v[2]=*(x->x_rdata+rpoint+1);
                    v[3]=0.0;
                    z[0]=*(x->x_idata+rpoint-1);
                    z[1]=*(x->x_idata+rpoint);
                    z[2]=*(x->x_idata+rpoint+1);
                    z[3]=0.0;
                }
                else if ( rpoint == (x->x_size*x->x_blocksize) )
                {
                    v[0]=*(x->x_rdata+rpoint-1);
                    v[1]=*(x->x_rdata+rpoint);
                    v[2]=0.0;
                    v[3]=0.0;
                    z[0]=*(x->x_idata+rpoint-1);
                    z[1]=*(x->x_idata+rpoint);
                    z[2]=0.0;
                    z[3]=0.0;
                }
                else
                {
                    v[0]=*(x->x_rdata+rpoint-1);
                    v[1]=*(x->x_rdata+rpoint);
                    v[2]=*(x->x_rdata+rpoint+1);
                    v[3]=*(x->x_rdata+rpoint+2);
                    z[0]=*(x->x_idata+rpoint-1);
                    z[1]=*(x->x_idata+rpoint);
                    z[2]=*(x->x_idata+rpoint+1);
                    z[3]=*(x->x_idata+rpoint+2);
                }

                {
                    t_float frac = rpoint-(int)rpoint;

                    // taken from tabread4_tilde
                    *rout = v[1]+frac*((v[2]-v[1])-0.5f*(frac-1.)*((v[0]-v[3]+3.0f*(v[2]-v[1]))*frac+(2.0f*v[1]-v[0]-v[2])));
                    *iout = z[1]+frac*((z[2]-z[1])-0.5f*(frac-1.)*((z[0]-z[3]+3.0f*(z[2]-z[1]))*frac+(2.0f*z[1]-z[0]-z[2])));
                }

                // add phase argument
                fspectrum = sqrt( pow( *rout, 2) + pow( *iout, 2) );
                fphase = atan2( *iout, *rout );
                fphase += (x->x_phase/180.0)*(M_PI);
                *rout = fspectrum*cos( fphase );
                *iout = fspectrum*sin( fphase );
            }
            else
            {
                *rout=0.0;
                *iout=0.0;
            }
            rout++;
            iout++;
            rin++;
            iin++;
            bi++;
        }
        // reset playing position until next play
        if ( x->x_play )
        {
            x->x_readpos+=x->x_readspeed;
            // post( "xreadpos : %f (added %f)", x->x_readpos, x->x_readspeed );
            if ( x->x_readpos >= (x->x_readend*x->x_size)/100 )
            {
                x->x_play=0;
                x->x_readpos=(x->x_readstart*x->x_size)/100;
                // post( "samplebox~ : stopped playing (readpos=%d)", x->x_readpos );
                outlet_bang(x->x_playend);
            }
        }
        // reset recording position until next record
        if ( x->x_record )
        {
            x->x_writepos++;
            if ( x->x_writepos >= x->x_size )
            {
                x->x_record=0;
                x->x_writepos=0;
                outlet_bang(x->x_recordend);
                // post( "samplebox~ : stopped recording" );
            }
        }
    }
    return (w+7);
}

static void samplebox_dsp(t_samplebox *x, t_signal **sp)
{
    dsp_add(samplebox_perform, 6, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n, x);
}

/* record the sonogram */
static void samplebox_record(t_samplebox *x)
{
    x->x_record=1;
    x->x_writepos=0;
}

/* play the sonogram */
static void samplebox_play(t_samplebox *x)
{
    x->x_play=1;
    // reset read position
    x->x_readpos=(x->x_readstart*x->x_size)/100;
}

/* setting the starting point for reading ( in percent ) */
static void samplebox_readstart(t_samplebox *x, t_floatarg fstart)
{
    t_float startpoint = fstart;

    if (startpoint < 0) startpoint = 0;
    if (startpoint > 100) startpoint = 100;
    if ( startpoint > x->x_readend )
    {
        x->x_readstart = x->x_readend;
        post( "samplebox~ : warning : range for reading is null" );
    }
    else
    {
        x->x_readstart=startpoint;
    }
}

/* setting the starting point for modification ( in percent ) */
static void samplebox_modstart(t_samplebox *x, t_floatarg fstart)
{
    t_float startpoint = fstart;

    if (startpoint < 0) startpoint = 0;
    if (startpoint > 100) startpoint = 100;
    if ( startpoint > x->x_modend )
    {
        x->x_modstart = x->x_modend;
        post( "samplebox~ : warning : range for modifications is null" );
    }
    else
    {
        x->x_modstart=startpoint;
    }
}

/* setting the ending point for reading ( in percent ) */
static void samplebox_readend(t_samplebox *x, t_floatarg fend)
{
    t_float endpoint = fend;

    if (endpoint < 0) endpoint = 0;
    if (endpoint > 100) endpoint = 100;
    if ( endpoint < x->x_readstart )
    {
        x->x_readend = x->x_readstart;
        post( "samplebox~ : warning : range for reading is null" );
    }
    else
    {
        x->x_readend=endpoint;
    }
}

/* setting the ending point for modification ( in percent ) */
static void samplebox_modend(t_samplebox *x, t_floatarg fend)
{
    t_float endpoint = fend;

    if (endpoint < 0) endpoint = 0;
    if (endpoint > 100) endpoint = 100;
    if ( endpoint < x->x_modstart )
    {
        x->x_modend = x->x_modstart;
        post( "samplebox~ : warning : range for modifications is null" );
    }
    else
    {
        x->x_modend=endpoint;
    }
}

/* sets the reading speed */
static void samplebox_readspeed(t_samplebox *x, t_floatarg freadspeed)
{
    if ((int)freadspeed < 0 )
    {
        post( "samplebox~ : wrong readspeed argument" );
    }
    x->x_readspeed=freadspeed;
}

/* resize sonogram */
static void samplebox_resize(t_samplebox *x, t_floatarg fnewsize )
{
    if (fnewsize <= 0)
    {
        post( "samplebox~ : error : wrong size" );
        return;
    }
    post( "samplebox~ : reallocating tables" );
    x->x_record = 0;
    x->x_play = 0;
    x->x_allocate = 1;
    samplebox_reallocate(x, x->x_size, fnewsize);
    x->x_size = fnewsize;
    x->x_allocate = 0;
}

/* flip blocks */
static void samplebox_flipblocks(t_samplebox *x)
{
    t_int samplestart, sampleend, middlesample, fi, si;
    t_float fvalue;

    samplestart=(x->x_modstart*(x->x_size-1))/100;
    sampleend=(x->x_modend*(x->x_size-1))/100;
    middlesample = ( sampleend+samplestart+1 ) / 2;
    post( "flip blocks [%d,%d] and [%d,%d]", samplestart, middlesample, middlesample, sampleend );

    for ( si=samplestart; si<=middlesample; si++ )
    {
        for ( fi=0; fi<x->x_blocksize; fi++ )
        {
            fvalue = *(x->x_rdata+((si)*x->x_blocksize)+fi);
            *(x->x_rdata+((si)*x->x_blocksize)+fi) = *(x->x_rdata+((sampleend+samplestart-si)*x->x_blocksize)+fi);
            *(x->x_rdata+((sampleend+samplestart-si)*x->x_blocksize)+fi) = fvalue;
            fvalue = *(x->x_idata+((si)*x->x_blocksize)+fi);
            *(x->x_idata+((si)*x->x_blocksize)+fi) = *(x->x_idata+((sampleend+samplestart-si)*x->x_blocksize)+fi);
            *(x->x_idata+((sampleend+samplestart-si)*x->x_blocksize)+fi) = fvalue;
        }
    }
}

/* change the phase */
static void samplebox_phase(t_samplebox *x, t_floatarg fincphase)
{
    if (fincphase < 0 || fincphase > 90)
    {
        post( "samplebox~ : error : wrong phase in phase function : out of [0,90]" );
        return;
    }
    x->x_phase = fincphase;
}

/* swap blocks */
static void samplebox_swapblocks(t_samplebox *x, t_floatarg fperstart, t_floatarg fperend, t_floatarg fpersize)
{
    t_int samplestart, samplestartb, samplesize, sp, sf;
    t_int iperstart, iperend, ipersize;
    t_float s1, s2;
    t_float fvalue;

    iperstart = fperstart;
    iperend = fperend;
    ipersize = fpersize;

    if (iperstart < 0 || iperstart > iperend ||
            iperend <= 0 || iperend+ipersize > 100 ||
            ipersize < 0 || fpersize > 100 )
    {
        post( "samplebox~ : error : wrong interval [%d%%, %d%%] <-> [%d%%, %d%%]",
              iperstart, iperstart+ipersize, iperend, iperend+ipersize );
        return;
    }
    samplestart=(x->x_modstart*(x->x_size-1))/100;
    samplestartb=(x->x_modend*(x->x_size-1))/100;
    samplesize=((samplestartb-samplestart)*ipersize)/100;
    samplestart=samplestart+((samplestartb-samplestart)*iperstart)/100;
    samplestartb=samplestart+((samplestartb-samplestart)*iperend)/100;

    // post( "swap blocks [%d,%d] and [%d,%d]", samplestart, samplestart+samplesize, samplestartb, samplestartb+samplesize );

    for ( sp=samplesize; sp>=0; sp-- )
    {
        for ( sf=0; sf<x->x_blocksize; sf++)
        {
            fvalue = *(x->x_rdata+((int)(samplestart+sp)*x->x_blocksize)+sf);
            *(x->x_rdata+((int)(samplestart+sp)*x->x_blocksize)+sf) = *(x->x_rdata+((int)(samplestartb+sp)*x->x_blocksize)+sf);
            *(x->x_rdata+((int)(samplestartb+sp)*x->x_blocksize)+sf) = fvalue;
            fvalue = *(x->x_idata+((int)(samplestart+sp)*x->x_blocksize)+sf);
            *(x->x_idata+((int)(samplestart+sp)*x->x_blocksize)+sf) = *(x->x_idata+((int)(samplestartb+sp)*x->x_blocksize)+sf);
            *(x->x_idata+((int)(samplestartb+sp)*x->x_blocksize)+sf) = fvalue;
        }
    }
}

static void *samplebox_new(t_floatarg fsize)
{
    t_samplebox *x = (t_samplebox *)pd_new(samplebox_class);
    outlet_new(&x->x_obj, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    x->x_recordend = outlet_new(&x->x_obj, &s_bang );
    x->x_playend = outlet_new(&x->x_obj, &s_bang );
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("readstart"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("readend"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("modstart"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("modend"));

    if ( fsize <= 0 )
    {
        error( "samplebox~ : missing or negative creation arguments" );
        return NULL;
    }

    x->x_size = fsize;
    x->x_blocksize = sys_getblksize();
    x->x_play = 0;
    x->x_readspeed = 1.;
    x->x_record = 0;
    x->x_readpos = 0.;
    x->x_writepos = 0;
    x->x_modstart = 0;
    x->x_readstart = 0;
    x->x_modend = 100;
    x->x_readend = 100;
    x->x_allocate = 0;
    x->x_rdata = NULL;
    x->x_idata = NULL;
    x->x_rootsquares = NULL;
    x->x_phase = 0.0;
    x->x_samplerate = sys_getsr();
    if ( samplebox_allocate(x) <0 )
    {
        return NULL;
    }
    else
    {
        return(x);
    }
}

void samplebox_tilde_setup(void)
{
    logpost(NULL, 4, samplebox_version);
    samplebox_class = class_new(gensym("samplebox~"), (t_newmethod)samplebox_new, (t_method)samplebox_free,
                                sizeof(t_samplebox), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN( samplebox_class, t_samplebox, x_f );
    class_addmethod(samplebox_class, (t_method)samplebox_dsp, gensym("dsp"), 0);
    class_addmethod(samplebox_class, (t_method)samplebox_record, gensym("record"), 0);
    class_addmethod(samplebox_class, (t_method)samplebox_resize, gensym("resize"), A_FLOAT, 0);
    class_addmethod(samplebox_class, (t_method)samplebox_swapblocks, gensym("swapblocks"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(samplebox_class, (t_method)samplebox_flipblocks, gensym("flipblocks"), 0);
    class_addmethod(samplebox_class, (t_method)samplebox_play, gensym("play"), 0);
    class_addmethod(samplebox_class, (t_method)samplebox_phase, gensym("phase"), A_FLOAT, 0);
    class_addmethod(samplebox_class, (t_method)samplebox_modstart, gensym("modstart"), A_FLOAT, 0);
    class_addmethod(samplebox_class, (t_method)samplebox_modend, gensym("modend"), A_FLOAT, 0);
    class_addmethod(samplebox_class, (t_method)samplebox_readstart, gensym("readstart"), A_FLOAT, 0);
    class_addmethod(samplebox_class, (t_method)samplebox_readend, gensym("readend"), A_FLOAT, 0);
    class_addmethod(samplebox_class, (t_method)samplebox_readspeed, gensym("readspeed"), A_FLOAT, 0);
}
