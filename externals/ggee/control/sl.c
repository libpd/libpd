/* (C) Guenter Geiger <geiger@epy.co.at> */


#include <m_pd.h>
#include <stdio.h>
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


/* -------------------- lsend ------------------------------ */

static t_class *lsend_class;

typedef struct _lsend
{
    t_object x_obj;
    t_symbol *x_sym;
} t_lsend;

static void lsend_bang(t_lsend *x)
{
    if (x->x_sym->s_thing) pd_bang(x->x_sym->s_thing);
}

static void lsend_float(t_lsend *x, t_float f)
{
    if (x->x_sym->s_thing) pd_float(x->x_sym->s_thing, f);
}

static void lsend_symbol(t_lsend *x, t_symbol *s)
{
    if (x->x_sym->s_thing) pd_symbol(x->x_sym->s_thing, s);
}

static void lsend_pointer(t_lsend *x, t_gpointer *gp)
{
    if (x->x_sym->s_thing) pd_pointer(x->x_sym->s_thing, gp);
}

static void lsend_list(t_lsend *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_sym->s_thing) pd_list(x->x_sym->s_thing, s, argc, argv);
}

static void lsend_anything(t_lsend *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_sym->s_thing) typedmess(x->x_sym->s_thing, s, argc, argv);
}

static void *lsend_new(t_symbol *s)
{
    t_lsend *x = (t_lsend *)pd_new(lsend_class);
    char mysym[MAXPDSTRING];
    
    sprintf(mysym,"%s%p",s->s_name,canvas_getcurrent());
    x->x_sym = gensym(mysym);
    return (x);
}

void sl_setup(void)
{
    lsend_class = class_new(gensym("sendlocal"), (t_newmethod)lsend_new, 0,
    	sizeof(t_lsend), 0, A_DEFSYM, 0);
    class_addcreator((t_newmethod)lsend_new, gensym("sl"), A_DEFSYM, 0);
    class_addbang(lsend_class, lsend_bang);
    class_addfloat(lsend_class, lsend_float);
    class_addsymbol(lsend_class, lsend_symbol);
    class_addpointer(lsend_class, lsend_pointer);
    class_addlist(lsend_class, lsend_list);
    class_addanything(lsend_class, lsend_anything);
    post("Warning: sendlocal (sl) is deprecated, please use \"send $0-var\" instead");
}
