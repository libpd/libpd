
#include "m_pd.h"

static t_class *normal_class;

typedef struct _normal
{
    t_object x_obj;
    int level;
    t_symbol* tag;
} t_normal;

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

static void normal_bang(t_normal *x)
{
    logpost(x, x->level, "%s%sbang",
            x->tag->s_name, (*x->tag->s_name ? ": " : ""));
}

static void normal_pointer(t_normal *x, t_gpointer *gp)
{
    logpost(x, x->level, "%s%s(pointer %lx)",
            x->tag->s_name, (*x->tag->s_name ? ": " : ""), gp);
}

static void normal_float(t_normal *x, t_float f)
{
    logpost(x, (const int)x->level, "%s%s%g",
            x->tag->s_name, (*x->tag->s_name ? ": " : ""), f);
}

static void normal_anything(t_normal *x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol* output = args2symbol(argc, argv);
    logpost(x, (const int)x->level, "%s%s%s %s",
            x->tag->s_name, (*x->tag->s_name ? ": " : ""),
            s->s_name, output->s_name);
}

static void *normal_new(t_symbol *s, int argc, t_atom *argv)
{
    t_normal *x = (t_normal *)pd_new(normal_class);
    x->tag = &s_;
    x->level = 2;
    if (argc > 0)
        x->tag = args2symbol(argc, argv);
    return (x);
}

void normal_setup(void)
{
    normal_class = class_new(gensym("normal"),
                             (t_newmethod)normal_new,
                             0,
                             sizeof(t_normal),
                             CLASS_DEFAULT,
                             A_GIMME, 0);
    class_addbang(normal_class, normal_bang);
    class_addfloat(normal_class, normal_float);
    class_addpointer(normal_class, normal_pointer);
    class_addanything(normal_class, normal_anything);
}
