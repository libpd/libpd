/* (C) Guenter Geiger <geiger@epy.co.at> */


#include <m_pd.h>
#include <stdio.h>
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


/* -------------------- lreceive ------------------------------ */

static t_class *lreceive_class;

typedef struct _lreceive
{
    t_object x_obj;
    t_symbol *x_sym;
} t_lreceive;

static void lreceive_bang(t_lreceive *x)
{
    outlet_bang(x->x_obj.ob_outlet);
}

static void lreceive_float(t_lreceive *x, t_float f)
{
    outlet_float(x->x_obj.ob_outlet, f);
}

static void lreceive_symbol(t_lreceive *x, t_symbol *s)
{
    outlet_symbol(x->x_obj.ob_outlet, s);
}

static void lreceive_pointer(t_lreceive *x, t_gpointer *gp)
{
    outlet_pointer(x->x_obj.ob_outlet, gp);
}

static void lreceive_list(t_lreceive *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_list(x->x_obj.ob_outlet, s, argc, argv);
}

static void lreceive_anything(t_lreceive *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_anything(x->x_obj.ob_outlet, s, argc, argv);
}

static void *lreceive_new(t_symbol *s)
{
    t_lreceive *x = (t_lreceive *)pd_new(lreceive_class);
    char mysym[MAXPDSTRING];
    
    sprintf(mysym,"%s%p",s->s_name,canvas_getcurrent());
    x->x_sym = gensym(mysym);
    pd_bind(&x->x_obj.ob_pd, x->x_sym);
    outlet_new(&x->x_obj, 0);
    return (x);
}

static void lreceive_free(t_lreceive *x)
{
    pd_unbind(&x->x_obj.ob_pd, x->x_sym);
}

void rl_setup(void)
{
    lreceive_class = class_new(gensym("receivelocal"), (t_newmethod)lreceive_new, 
    	(t_method)lreceive_free, sizeof(t_lreceive), CLASS_NOINLET, A_SYMBOL, 0);
    class_addcreator((t_newmethod)lreceive_new, gensym("rl"), A_DEFSYM, 0);
    class_addbang(lreceive_class, lreceive_bang);
    class_addfloat(lreceive_class, (t_method)lreceive_float);
    class_addsymbol(lreceive_class, lreceive_symbol);
    class_addpointer(lreceive_class, lreceive_pointer);
    class_addlist(lreceive_class, lreceive_list);
    class_addanything(lreceive_class, lreceive_anything);
    post("Warning: receivelocal (rl) is deprecated, please use \"receive $0-var\" instead");
}
