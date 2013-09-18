/* Copyright (c) 1997-1999 Miller Puckette source modified by dieb13.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*  catch13~ ...  = original objects with set-message for changing - bad hack - but works somehow*/

#include "m_pd.h"
#include "d_global13.h"

#define DEFSENDVS 64	/* LATER get send to get this from canvas */

/* ----------------------------- throw13~ ----------------------------- */

 void *sigthrow13_new(t_symbol *s)
{
    t_sigthrow13 *x = (t_sigthrow13 *)pd_new(sigthrow13_class);
    if (!*s->s_name) s = gensym("throw13~");
    x->x_sym = s;
    x->x_whereto  = 0;
    x->x_n = DEFSENDVS;
    return (x);
}

 t_int *sigthrow13_perform(t_int *w)
{
    t_sigthrow13 *x = (t_sigthrow13 *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    int n = (int)(w[3]);
    t_float *out = x->x_whereto;
    if (out)
    {
    	while (n--)
	{
	/* TB: denormal handling in pd >0.37-2 */
#ifdef PD_BIGORSMALL
	    *out += (PD_BIGORSMALL(*in) ? 0 : *in);
	    out++;
	    in++;
#else
	    *out++ += *in++; 
#endif
	}
    }
    return (w+4);
}

 void sigthrow13_set(t_sigthrow13 *x, t_symbol *s)
{
    t_sigcatch13 *catcher = (t_sigcatch13 *)pd_findbyclass((x->x_sym = s),
    	sigcatch13_class);
    if (catcher)
    {
    	if (catcher->x_n == x->x_n)
    	    x->x_whereto = catcher->x_vec;
	else
	{
	    pd_error(x, "throw13~ %s: vector size mismatch", x->x_sym->s_name);
	    x->x_whereto = 0;
	}
    }
    else
    {
    	pd_error(x, "throw13~ %s: no matching catch", x->x_sym->s_name);
    	x->x_whereto = 0;
    }
}

 void sigthrow13_dsp(t_sigthrow13 *x, t_signal **sp)
{
    if (sp[0]->s_n != x->x_n)
    {
    	pd_error(x, "throw13~ %s: vector size mismatch", x->x_sym->s_name);
    }
    else
    {
    	sigthrow13_set(x, x->x_sym);
    	dsp_add(sigthrow13_perform, 3,
    	    x, sp[0]->s_vec, sp[0]->s_n);
    }
}

void sigthrow13_setup(void)
{
    sigthrow13_class = class_new(gensym("throw13~"), (t_newmethod)sigthrow13_new, 0,
    	sizeof(t_sigthrow13), 0, A_DEFSYM, 0);
    class_addcreator((t_newmethod)sigthrow13_new, gensym("t13~"), A_DEFSYM, 0);
    class_addmethod(sigthrow13_class, (t_method)sigthrow13_set, gensym("set"),
    	A_SYMBOL, 0);
    class_addmethod(sigthrow13_class, nullfn, gensym("signal"), 0);
    class_addmethod(sigthrow13_class, (t_method)sigthrow13_dsp, gensym("dsp"), 0);
}

void throw13_tilde_setup()
{
  sigthrow13_setup();
}

