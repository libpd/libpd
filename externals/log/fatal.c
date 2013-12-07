
#include "m_pd.h"

static t_class *fatal_class;

typedef struct _fatal
{
    t_object x_obj;
    int level;
    t_symbol* tag;
} t_fatal;

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

static void fatal_bang(t_fatal *x)
{
    logpost(x, x->level, "%s%sbang",
            x->tag->s_name, (*x->tag->s_name ? ": " : ""));
}

static void fatal_pointer(t_fatal *x, t_gpointer *gp)
{
    logpost(x, x->level, "%s%s(pointer %lx)",
            x->tag->s_name, (*x->tag->s_name ? ": " : ""), gp);
}

static void fatal_float(t_fatal *x, t_float f)
{
    logpost(x, (const int)x->level, "%s%s%g",
            x->tag->s_name, (*x->tag->s_name ? ": " : ""), f);
}

static void fatal_anything(t_fatal *x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol* output = args2symbol(argc, argv);
    logpost(x, (const int)x->level, "%s%s%s %s",
            x->tag->s_name, (*x->tag->s_name ? ": " : ""),
            s->s_name, output->s_name);
}

static void *fatal_new(t_symbol *s, int argc, t_atom *argv)
{
    t_fatal *x = (t_fatal *)pd_new(fatal_class);
    x->tag = &s_;
    x->level = 0;
    if (argc > 0)
        x->tag = args2symbol(argc, argv);
    return (x);
}

void fatal_setup(void)
{
    fatal_class = class_new(gensym("fatal"),
                            (t_newmethod)fatal_new,
                            0,
                            sizeof(t_fatal),
                            CLASS_DEFAULT,
                            A_GIMME, 0);
    class_addbang(fatal_class, fatal_bang);
    class_addfloat(fatal_class, fatal_float);
    class_addpointer(fatal_class, fatal_pointer);
    class_addanything(fatal_class, fatal_anything);
}
