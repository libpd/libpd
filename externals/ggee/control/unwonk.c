/* (C) Guenter Geiger <geiger@epy.co.at> */


#include <m_pd.h>


/* -------------------------- unwonk ------------------------------ */

static t_class *unwonk_class;

typedef struct unwonkout
{
    t_atomtype u_type;
    t_outlet *u_outlet;
} t_unwonkout;

typedef struct _unwonk
{
    t_object x_obj;
    t_int x_n;
    t_unwonkout *x_vec;
} t_unwonk;

static void *unwonk_new(t_symbol *s, int argc, t_atom *argv)
{
    t_unwonk *x = (t_unwonk *)pd_new(unwonk_class);
    t_atom defarg[2], *ap;
    t_unwonkout *u;
    int i;
    if (!argc)
    {
    	argv = defarg;
    	argc = 2;
    	SETFLOAT(&defarg[0], 0);
    	SETFLOAT(&defarg[1], 0);
    }

    x->x_n = argc + 1;
    x->x_vec = (t_unwonkout *)getbytes(x->x_n * sizeof(*x->x_vec));

    for (i = 0, ap = argv, u = x->x_vec; i < argc; u++, ap++, i++)
    {
    	t_atomtype type = ap->a_type;
    	if (type == A_SYMBOL)
    	{
    	    char c = *ap->a_w.w_symbol->s_name;
    	    if (c == 's')
    	    {
    	    	u->u_type = A_SYMBOL;
    	    	u->u_outlet = outlet_new(&x->x_obj, &s_symbol);
    	    }
    	    else if (c == 'p')
    	    {
    	    	u->u_type =  A_POINTER;
    	    	u->u_outlet = outlet_new(&x->x_obj, &s_pointer);
    	    }
    	    else
    	    {
    	    	if (c != 'f') error("unwonk: %s: bad type",
    	    	    ap->a_w.w_symbol->s_name);
    	    	u->u_type = A_FLOAT;
    	    	u->u_outlet = outlet_new(&x->x_obj, &s_float);
    	    }
    	}
    	else
    	{
    	    u->u_type =  A_FLOAT;
    	    u->u_outlet = outlet_new(&x->x_obj, &s_float);
    	}
    }

    u->u_type =  A_GIMME;
    u->u_outlet = outlet_new(&x->x_obj, &s_list);
    
    return (x);
}

static void unwonk_list(t_unwonk *x, t_symbol *s, int argc, t_atom *argv)
{
    t_atom *ap;
    t_unwonkout *u;
    int i;
    int margc = argc;


    if (argc > x->x_n - 1) margc = x->x_n - 1;

    if (argc - margc > 0) {
	 ap = argv + margc;
	 u = x->x_vec + margc;
	 outlet_list(u->u_outlet,0,argc - margc, ap);
    }

    for (i = margc, u = x->x_vec + i, ap = argv + i; u--, ap--, i--;)
    {
    	t_atomtype type = u->u_type;
    	if (type != ap->a_type)
    	    error("unwonk: type mismatch");
    	else if (type == A_FLOAT)
    	    outlet_float(u->u_outlet, ap->a_w.w_float);
    	else if (type == A_SYMBOL)
    	    outlet_symbol(u->u_outlet, ap->a_w.w_symbol);
    	else outlet_pointer(u->u_outlet, ap->a_w.w_gpointer);
    }
    

}

static void unwonk_free(t_unwonk *x)
{
    freebytes(x->x_vec, x->x_n * sizeof(*x->x_vec));
}

void unwonk_setup(void)
{
    unwonk_class = class_new(gensym("unwonk"), (t_newmethod)unwonk_new,
    	(t_method)unwonk_free, sizeof(t_unwonk), 0, A_GIMME, 0);
    class_addlist(unwonk_class, unwonk_list);
}
