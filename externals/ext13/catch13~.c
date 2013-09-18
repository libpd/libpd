/* Copyright (c) 1997-1999 Miller Puckette source modified by dieb13.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*  catch13~ ...  = original objects with set-message for changing - bad hack - but works somehow*/

#include "m_pd.h"
#include "d_global13.h"

#define DEFSENDVS 64	/* LATER get send to get this from canvas */


/* ----------------------------- catch13~ ----------------------------- */

 void *sigcatch13_new(t_symbol *s)
{
    t_sigcatch13 *x = (t_sigcatch13 *)pd_new(sigcatch13_class);
    if (!*s->s_name) s = gensym("catch~");
    pd_bind(&x->x_obj.ob_pd, s);
    x->x_sym = s;
    x->x_n = DEFSENDVS;
    x->x_vec = (float *)getbytes(DEFSENDVS * sizeof(float));
    outlet_new(&x->x_obj, &s_signal);
    return (x);
}

t_int *sigcatch13_perform(t_int *w)
{
    t_float *in = (t_float *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    int n = (int)(w[3]);
    while (n--) *out++ = *in, *in++ = 0; 
    return (w+4);
}

/* tb: vectorized */
t_int *sigcatch13_perf8(t_int *w)
{
    t_float *in = (t_float *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    int n = (int)(w[3]);
    for (; n; n -= 8, in += 8, out += 8)
    {
	out[0] = in[0]; out[1] = in[1]; out[2] = in[2]; out[3] = in[3]; 
	out[4] = in[4]; out[5] = in[5]; out[6] = in[6]; out[7] = in[7]; 
    
	in[0] = 0; in[1] = 0; in[2] = 0; in[3] = 0; 
	in[4] = 0; in[5] = 0; in[6] = 0; in[7] = 0; 
    }
    return (w+4);
}

 void sigcatch13_set(t_sigcatch13 *x, t_symbol *s)
{
    pd_unbind(&x->x_obj.ob_pd, x->x_sym);
    x->x_sym = s;
    pd_bind(&x->x_obj.ob_pd, s);
}

 void sigcatch13_dsp(t_sigcatch13 *x, t_signal **sp)
{
    if (x->x_n == sp[0]->s_n)
    {
	if(sp[0]->s_n&7)
	    dsp_add(sigcatch13_perform, 3, x->x_vec, sp[0]->s_vec, sp[0]->s_n);
	else
	    dsp_add(sigcatch13_perf8, 3, x->x_vec, sp[0]->s_vec, sp[0]->s_n);
    }
    else error("sigcatch13 %s: unexpected vector size", x->x_sym->s_name);
}

 void sigcatch13_free(t_sigcatch13 *x)
{
    pd_unbind(&x->x_obj.ob_pd, x->x_sym);
    freebytes(x->x_vec, x->x_n * sizeof(float));
}

void sigcatch13_setup(void)
{
    sigcatch13_class = class_new(gensym("catch13~"), (t_newmethod)sigcatch13_new,
    	(t_method)sigcatch13_free, sizeof(t_sigcatch13), 0, A_DEFSYM, 0);
    class_addcreator((t_newmethod)sigcatch13_new, gensym("c13~"), A_DEFSYM, 0);
    class_addmethod(sigcatch13_class, (t_method)sigcatch13_set, gensym("set"),
    	A_SYMBOL, 0);
    class_addmethod(sigcatch13_class, (t_method)sigcatch13_dsp, gensym("dsp"), 0);
}

void catch13_tilde_setup()
{
  sigcatch13_setup();
}


