
#include "m_pd.h"

static t_class *error_class;

typedef struct _error
{
    t_object x_obj;
    int level;
    t_symbol* tag;
} t_error;

static t_symbol* args2symbol(int argc, t_atom *argv)
{
    t_symbol* s;
    char* buf;
    int bufsize;
    t_binbuf *bb = binbuf_new();
    binbuf_add(bb, argc, argv);
    binbuf_gettext(bb, &buf, &bufsize);
    buf = resizebytes(buf, bufsize, bufsize+1);
    buf[bufsize] = 0;
    s = gensym(buf);
    freebytes(buf, bufsize+1);
    binbuf_free(bb);
    return s;
}

static void error_bang(t_error *x)
{
    logpost(x, x->level, "%s%sbang",
            x->tag->s_name, (*x->tag->s_name ? ": " : ""));
}

static void error_pointer(t_error *x, t_gpointer *gp)
{
    logpost(x, x->level, "%s%s(pointer %lx)",
            x->tag->s_name, (*x->tag->s_name ? ": " : ""), gp);
}

static void error_float(t_error *x, t_float f)
{
    logpost(x, (const int)x->level, "%s%s%g",
            x->tag->s_name, (*x->tag->s_name ? ": " : ""), f);
}

static void error_anything(t_error *x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol* output = args2symbol(argc, argv);
    logpost(x, (const int)x->level, "%s%s%s %s",
            x->tag->s_name, (*x->tag->s_name ? ": " : ""),
            s->s_name, output->s_name);
}

static void *error_new(t_symbol *s, int argc, t_atom *argv)
{
    t_error *x = (t_error *)pd_new(error_class);
    x->tag = &s_;
    x->level = 1;
    if (argc > 0)
        x->tag = args2symbol(argc, argv);
    return (x);
}

void error_setup(void)
{
    error_class = class_new(gensym("error"),
                            (t_newmethod)error_new,
                            0,
                            sizeof(t_error),
                            CLASS_DEFAULT,
                            A_GIMME, 0);
    class_addbang(error_class, error_bang);
    class_addfloat(error_class, error_float);
    class_addpointer(error_class, error_pointer);
    class_addanything(error_class, error_anything);
}
