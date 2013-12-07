
#include "m_pd.h"

static t_class *logpost_class;

typedef struct _logpost
{
    t_object x_obj;
    t_float level;
    t_symbol* tag;
} t_logpost;

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

static void logpost_bang(t_logpost *x)
{
    logpost(x, (const int)x->level, "%s%sbang",
            x->tag->s_name, (*x->tag->s_name ? ": " : ""));
}

static void logpost_pointer(t_logpost *x, t_gpointer *gp)
{
    logpost(x, (const int)x->level, "%s%s(pointer %lx)",
            x->tag->s_name, (*x->tag->s_name ? ": " : ""), gp);
}

static void logpost_float(t_logpost *x, t_float f)
{
    logpost(x, (const int)x->level, "%s%s%g",
            x->tag->s_name, (*x->tag->s_name ? ": " : ""), f);
}

static void logpost_anything(t_logpost *x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol* output = args2symbol(argc, argv);
    logpost(x, (const int)x->level, "%s%s%s %s",
            x->tag->s_name, (*x->tag->s_name ? ": " : ""),
            s->s_name, output->s_name);
}

static void *logpost_new(t_symbol *sel, int argc, t_atom *argv)
{
    t_logpost *x = (t_logpost *)pd_new(logpost_class);

    x->tag = &s_;
    x->level = 2;
    t_symbol *sym = atom_getsymbolarg(0, argc, argv);
    if (sym != &s_) // oops, we have a symbol, use default
        logpost(x, 3, "[logpost] got '%s' instead of a float for the first argument",
                sym->s_name);
    else // we have a float
        x->level = atom_getfloatarg(0, argc, argv);
    if (argc > 1)
    {
        argc--;
        argv++; // lose the level arg
        x->tag = args2symbol(argc, argv);
    }
    floatinlet_new(&x->x_obj, &x->level);
    return (x);
}

void logpost_setup(void)
{
    logpost_class = class_new(gensym("logpost"),
                              (t_newmethod)logpost_new,
                              0,
                              sizeof(t_logpost),
                              CLASS_DEFAULT,
                              A_GIMME, 0);
    class_addbang(logpost_class, logpost_bang);
    class_addfloat(logpost_class, logpost_float);
    class_addpointer(logpost_class, logpost_pointer);
    class_addanything(logpost_class, logpost_anything);
}
