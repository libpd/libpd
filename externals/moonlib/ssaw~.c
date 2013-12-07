/*
Copyright (C) 2004 Antoine Rousseau
all material Copyright (c) 1997-1999 Miller Puckette.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

/* a "sweet" saw generator, as described in pddoc/3.audio/K05.bandlimited .*/

#include "m_pd.h"
#include "math.h"
#include <stdlib.h>

#define UNITBIT32 1572864.  /* 3*2^19; bit 32 has place value 1 */

/* machine-dependent definitions.  These ifdefs really
should have been by CPU type and not by operating system! */
#ifdef IRIX
/* big-endian.  Most significant byte is at low address in memory */
#define HIOFFSET 0    /* word offset to find MSB */
#define LOWOFFSET 1    /* word offset to find LSB */
#define int32 long  /* a data type that has 32 bits */
#else
#ifdef _WIN32
/* little-endian; most significant byte is at highest address */
#define HIOFFSET 1
#define LOWOFFSET 0
#define int32 long
#else
#ifdef __FreeBSD__
#include <machine/endian.h>
#if BYTE_ORDER == LITTLE_ENDIAN
#define HIOFFSET 1
#define LOWOFFSET 0
#else
#define HIOFFSET 0    /* word offset to find MSB */
#define LOWOFFSET 1    /* word offset to find LSB */
#endif /* BYTE_ORDER */
#include <sys/types.h>
#define int32 int32_t
#endif
#ifdef __linux__

#include <endian.h>

#if !defined(__BYTE_ORDER) || !defined(__LITTLE_ENDIAN)
#error No byte order defined                                                    
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define HIOFFSET 1
#define LOWOFFSET 0
#else
#define HIOFFSET 0    /* word offset to find MSB */
#define LOWOFFSET 1    /* word offset to find LSB */
#endif /* __BYTE_ORDER */

#include <sys/types.h>
#define int32 int32_t

#else
#ifdef __APPLE__
#define HIOFFSET 0    /* word offset to find MSB */
#define LOWOFFSET 1    /* word offset to find LSB */
#define int32 int  /* a data type that has 32 bits */

#endif /* __APPLE__ */
#endif /* __linux__ */
#endif /* _WIN32 */
#endif /* SGI */

union tabfudge
{
    double tf_d;
    int32 tf_i[2];
};


/* -------------------------- ssaw~ ------------------------------ */
static t_class *ssaw_class, *scalarssaw_class;
static float ssaw_array[1002];
#define SAW_ARRAY_LEN 1002

typedef struct _ssaw
{
    t_object x_obj;
    //from phasor~:
    double x_phase;
    float x_conv;
    float x_f;	    /* scalar frequency */
    float x_band;	/* band limit (Hertz)*/
} t_ssaw;

static void *ssaw_new(t_floatarg f)
{
    t_ssaw *x = (t_ssaw *)pd_new(ssaw_class);
    x->x_f = f;
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
    x->x_phase = 0;
    x->x_conv = 0;
    x->x_band = 22000;
    outlet_new(&x->x_obj, gensym("signal"));
    return (x);
}

static t_int *ssaw_perform(t_int *w)
{
    t_ssaw *x = (t_ssaw *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int i,n = (int)(w[4]);
    double dphase = x->x_phase + UNITBIT32;
    union tabfudge tf;
    int normhipart;
    float conv = x->x_conv;
    float band=x->x_band*.33;
    float *buf = ssaw_array;

    tf.tf_d = UNITBIT32;
    normhipart = tf.tf_i[HIOFFSET];
    tf.tf_d = dphase;

    for (i = 0; i < n; i++)
        //while (n--)
    {
        float phase,band2,findex /*= *in++*/;
        int index /*= findex*/;
        float frac,  a,  b,  c,  d, cminusb, *fp;

        tf.tf_i[HIOFFSET] = normhipart;
        band2=abs(*in);
        if(band2>999999) band2=999999;
        else if(band2<1) band2=1;
        band2=band/band2;
        dphase += *in++ * conv;
        /**out++*/
        phase = (tf.tf_d - UNITBIT32)-0.5;
        tf.tf_d = dphase;

        findex=phase*band2;
        if(findex>0.5) findex=0.5;
        else if(findex<-0.5) findex=-0.5;

        /*findex=findex*1000+501;
        index=findex;*/
        /*if (index < 1)
            index = 1, frac = 0;
        else if (index > maxindex)
            index = maxindex, frac = 1;
        else*/
        frac = findex - index;
        /*fp = buf + index;
        a = fp[-1];
        b = fp[0];
        c = fp[1];
        d = fp[2];
        cminusb = c-b;
        *out++ = 0.5+ phase - (
        		 b + frac * ( cminusb - 0.1666667f * (1.-frac) * (
        		(d - a - 3.0f * cminusb) * frac + (d + 2.0f*a - 3.0f*b)))
        		);*/
        *out++ = 0.5+ phase - buf[(int)(findex*1000+501)];
    }

    tf.tf_i[HIOFFSET] = normhipart;
    x->x_phase = tf.tf_d - UNITBIT32;
    return (w+5);
}

static void ssaw_dsp(t_ssaw *x, t_signal **sp)
{
    x->x_conv = 1./sp[0]->s_sr;
    dsp_add(ssaw_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void ssaw_ft1(t_ssaw *x, t_float f)
{
    x->x_phase = f;
}

static void ssaw_initarray(void)
{
    int i;
    float j;

    for(i=0; i<1002; i++)
    {
        j=(i-1)*M_PI/1000.0; //period 2000 sample, 1 sample back phase
        ssaw_array[i]= 0.57692*
                       (-1*cos(j) + 0.333333*cos(j*3.0) -0.2* cos(j*5.0));
    }
}

void ssaw_tilde_setup(void)
{
    ssaw_class = class_new(gensym("ssaw~"), (t_newmethod)ssaw_new, 0,
                           sizeof(t_ssaw), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(ssaw_class, t_ssaw, x_f);
    class_addmethod(ssaw_class, (t_method)ssaw_dsp, gensym("dsp"), 0);
    class_addmethod(ssaw_class, (t_method)ssaw_ft1,
                    gensym("ft1"), A_FLOAT, 0);
    ssaw_initarray();

}

