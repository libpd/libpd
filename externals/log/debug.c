
#include "m_pd.h"

static t_class *debug_class;

typedef struct _debug
{
    t_object x_obj;
    int level;
    t_symbol* tag;
} t_debug;

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

static void debug_bang(t_debug *x)
{
    logpost(x, x->level, "%s%sbang",
            x->tag->s_name, (*x->tag->s_name ? ": " : ""));
}

static void debug_pointer(t_debug *x, t_gpointer *gp)
{
    logpost(x, x->level, "%s%s(pointer %lx)",
            x->tag->s_name, (*x->tag->s_name ? ": " : ""), gp);
}

static void debug_float(t_debug *x, t_float f)
{
    logpost(x, (const int)x->level, "%s%s%g",
            x->tag->s_name, (*x->tag->s_name ? ": " : ""), f);
}

static void debug_anything(t_debug *x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol* output = args2symbol(argc, argv);
    logpost(x, (const int)x->level, "%s%s%s %s",
            x->tag->s_name, (*x->tag->s_name ? ": " : ""),
            s->s_name, output->s_name);
}

static void *debug_new(t_symbol *s, int argc, t_atom *argv)
{
    t_debug *x = (t_debug *)pd_new(debug_class);
    x->tag = &s_;
    x->level = 3;
    if (argc > 0)
        x->tag = args2symbol(argc, argv);
    return (x);
}

void debug_setup(void)
{
    debug_class = class_new(gensym("debug"),
                            (t_newmethod)debug_new,
                            0,
                            sizeof(t_debug),
                            CLASS_DEFAULT,
                            A_GIMME, 0);
    class_addbang(debug_class, debug_bang);
    class_addfloat(debug_class, debug_float);
    class_addpointer(debug_class, debug_pointer);
    class_addanything(debug_class, debug_anything);
}
