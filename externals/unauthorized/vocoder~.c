/* vocoder~ -- vocoder effect inspired by xvox
 * written by Simon Morlat ( http://simon.morlat.free.fr )
 *
 * Copyleft 2001 Yves Degoyon.
 * Permission is granted to use this software for any purpose provided you
 * keep this copyright notice intact.
 *
 * THE AUTHOR AND HIS EXPLOITERS MAKE NO WARRANTY, EXPRESS OR IMPLIED,
 * IN CONNECTION WITH THIS SOFTWARE.
 *
 */

#include "m_pd.h"
#include "filters.h"
#include "lpc.h"

#define OUTPUT_DELAY 50

typedef struct _vocoder
{
    t_object x_obj;
    t_float x_f;
    t_float x_cutoff;
    t_int   x_vfeedback;
    double  *x_in1buf;
    double  *x_in2buf;
    double  *x_outbuf;
    t_int   x_blocksize;
    t_int   x_process;
} t_vocoder;

static t_class *vocoder_class;

static char   *vocoder_version = "vocoder~: version 0.1, written by ydegoyon@free.fr, inspired by xvox (Simon Morlat)";

static void vocoder_cutoff(t_vocoder *x, t_floatarg fcutoff )
{
    if ( fcutoff > 128.0 )
    {
        fcutoff = 128.0;
    }
    if ( fcutoff < 0.0 )
    {
        fcutoff = 0.0;
    }
    x->x_cutoff = fcutoff;
}

static void vocoder_vfeedback(t_vocoder *x, t_floatarg fvfeedback )
{
    if ( fvfeedback > 100.0 )
    {
        fvfeedback = 100.0;
    }
    if ( fvfeedback < 0.0 )
    {
        fvfeedback = 0.0;
    }
    x->x_vfeedback = fvfeedback;
}

static t_int *vocoder_perform(t_int *w)
{
    t_vocoder *x = (t_vocoder *)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *fin1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out1 = (t_float *)(w[4]);
    double  correls[12], lpc_coef[11], f1[12], f2[12], lsp_coef[11];
    int offset=0;
    int n = (int)(w[5]), i;

    if ( !x->x_process ) return (w+6);
    if ( x->x_blocksize != n )
    {
        if ( x->x_in1buf ) freebytes( x->x_in1buf, 3*x->x_blocksize/2*sizeof( double ) );
        if ( x->x_in2buf ) freebytes( x->x_in2buf, 3*x->x_blocksize/2*sizeof( double ) );
        if ( x->x_outbuf ) freebytes( x->x_outbuf, (x->x_blocksize+OUTPUT_DELAY)*sizeof( double ) );
        x->x_blocksize = n;
        x->x_in1buf = (double*) getbytes( 3*x->x_blocksize/2*sizeof( double ) );
        x->x_in2buf = (double*) getbytes( 3*x->x_blocksize/2*sizeof( double ) );
        x->x_outbuf = (double*) getbytes( (x->x_blocksize+OUTPUT_DELAY)*sizeof( double ) );
        if ( !x->x_in1buf || !x->x_in2buf || !x->x_outbuf )
        {
            post( "vocoder~ : allocations failed : stop processing" );
            x->x_process = 0;
        }
    }

    for(i=0; i<x->x_blocksize/2; i++)
    {
        x->x_in1buf[i]=x->x_in1buf[i+x->x_blocksize];
    };
    for(i=0; i<OUTPUT_DELAY; i++) x->x_outbuf[i]=x->x_outbuf[i+x->x_blocksize];
    for(i=0; i<x->x_blocksize; i++)
    {
        x->x_in1buf[x->x_blocksize/2+i]=(double)(*(in1++));
        x->x_in2buf[x->x_blocksize/2+i]=(double)(*(in2++));
    }

    hp_filter(x->x_in2buf,x->x_cutoff/128.,n);
    for(i=0; i<4; i++)
    {
        comp_lpc(x->x_in1buf+offset,correls,lpc_coef,x->x_blocksize/4);
        if (lpc_coef[0]!=0)
        {
            lpc2lsp(lpc_coef,f1,f2,lsp_coef);
            lsp2lpc(lsp_coef,lpc_coef);
        };
        lpc_filter(x->x_in2buf+offset,lpc_coef,x->x_outbuf+OUTPUT_DELAY+offset,x->x_blocksize/4);
        offset+=x->x_blocksize/4;
    };
    for(i=0; i<x->x_blocksize; i++)
    {
        if ( x->x_outbuf[OUTPUT_DELAY+i] > 1.0 ) x->x_outbuf[OUTPUT_DELAY+i]=1.0;
        if ( x->x_outbuf[OUTPUT_DELAY+i] < -1.0 ) x->x_outbuf[OUTPUT_DELAY+i]=-1.0;
        *(out1++)=(t_float)(((100-x->x_vfeedback)*x->x_outbuf[OUTPUT_DELAY+i]
                             + x->x_vfeedback*(*fin1++))/100.0);
    };

    return (w+6);
}

static void vocoder_dsp(t_vocoder *x, t_signal **sp)
{
    dsp_add(vocoder_perform, 5, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void *vocoder_new(void)
{
    t_vocoder *x = (t_vocoder *)pd_new(vocoder_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("cutoff"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("vfeedback"));
    outlet_new(&x->x_obj, &s_signal);
    x->x_cutoff = 60.;
    x->x_vfeedback = 50;
    x->x_blocksize=-1;
    x->x_in1buf = NULL;
    x->x_in2buf = NULL;
    x->x_outbuf = NULL;
    x->x_process = 1;
    return (x);
}

/* clean up */
static void vocoder_free(t_vocoder *x)
{
    if ( x->x_in1buf ) freebytes( x->x_in1buf, 3*x->x_blocksize/2*sizeof( double ) );
    if ( x->x_in2buf ) freebytes( x->x_in2buf, 3*x->x_blocksize/2*sizeof( double ) );
    if ( x->x_outbuf ) freebytes( x->x_outbuf, (x->x_blocksize+OUTPUT_DELAY)*sizeof( double ) );
}

void vocoder_tilde_setup(void)
{
    logpost(NULL, 4, vocoder_version);
    vocoder_class = class_new(gensym("vocoder~"), (t_newmethod)vocoder_new, (t_method)vocoder_free,
                              sizeof(t_vocoder), 0, 0);
    CLASS_MAINSIGNALIN( vocoder_class, t_vocoder, x_f );
    class_addmethod(vocoder_class, (t_method)vocoder_dsp, gensym("dsp"), 0);
    class_addmethod(vocoder_class, (t_method)vocoder_cutoff, gensym("cutoff"), A_FLOAT, 0);
    class_addmethod(vocoder_class, (t_method)vocoder_vfeedback, gensym("vfeedback"), A_FLOAT, 0);
}
