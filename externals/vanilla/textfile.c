/* Copyright (c) 1997-1999 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include <string.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

typedef struct _qlist
{
    t_object x_ob;
    t_outlet *x_bangout;
    void *x_binbuf;
    int x_onset;                /* playback position */
    t_clock *x_clock;
    t_float x_tempo;
    double x_whenclockset;
    t_float x_clockdelay;
    t_symbol *x_dir;
    t_canvas *x_canvas;
    int x_reentered;
} t_qlist;

static t_class *textfile_class;
typedef t_qlist t_textfile;

static void qlist_rewind(t_qlist *x)
{
    x->x_onset = 0;
    if (x->x_clock) clock_unset(x->x_clock);
    x->x_whenclockset = 0;
    x->x_reentered = 1;
}

static void qlist_add(t_qlist *x, t_symbol *s, int ac, t_atom *av)
{
    t_atom a;
    SETSEMI(&a);
    binbuf_add(x->x_binbuf, ac, av);
    binbuf_add(x->x_binbuf, 1, &a);
}

static void qlist_add2(t_qlist *x, t_symbol *s, int ac, t_atom *av)
{
    binbuf_add(x->x_binbuf, ac, av);
}

static void qlist_clear(t_qlist *x)
{
    qlist_rewind(x);
    binbuf_clear(x->x_binbuf);
}

static void qlist_set(t_qlist *x, t_symbol *s, int ac, t_atom *av)
{
    qlist_clear(x);
    qlist_add(x, s, ac, av);
}

static void qlist_read(t_qlist *x, t_symbol *filename, t_symbol *format)
{
    int cr = 0;
    if (!strcmp(format->s_name, "cr"))
        cr = 1;
    else if (*format->s_name)
        pd_error(x, "qlist_read: unknown flag: %s", format->s_name);

    if (binbuf_read_via_canvas(x->x_binbuf, filename->s_name, x->x_canvas, cr))
            pd_error(x, "%s: read failed", filename->s_name);
    x->x_onset = 0x7fffffff;
    x->x_reentered = 1;
}

static void qlist_write(t_qlist *x, t_symbol *filename, t_symbol *format)
{
    int cr = 0;
    char buf[MAXPDSTRING];
    canvas_makefilename(x->x_canvas, filename->s_name,
        buf, MAXPDSTRING);
    if (!strcmp(format->s_name, "cr"))
        cr = 1;
    else if (*format->s_name)
        pd_error(x, "qlist_read: unknown flag: %s", format->s_name);
    if (binbuf_write(x->x_binbuf, buf, "", cr))
            pd_error(x, "%s: write failed", filename->s_name);
}

static void qlist_print(t_qlist *x)
{
    post("--------- textfile or qlist contents: -----------");
    binbuf_print(x->x_binbuf);
}

static void *textfile_new( void)
{
    t_symbol *name, *filename = 0;
    t_textfile *x = (t_textfile *)pd_new(textfile_class);
    x->x_binbuf = binbuf_new();
    outlet_new(&x->x_ob, &s_list);
    x->x_bangout = outlet_new(&x->x_ob, &s_bang);
    x->x_onset = 0x7fffffff;
    x->x_reentered = 0;
    x->x_tempo = 1;
    x->x_whenclockset = 0;
    x->x_clockdelay = 0;
    x->x_clock = NULL;
    x->x_canvas = canvas_getcurrent();
    return (x);
}

static void textfile_bang(t_textfile *x)
{
    int argc = binbuf_getnatom(x->x_binbuf),
        count, onset = x->x_onset, onset2;
    t_atom *argv = binbuf_getvec(x->x_binbuf);
    t_atom *ap = argv + onset, *ap2;
    while (onset < argc &&
        (ap->a_type == A_SEMI || ap->a_type == A_COMMA))
            onset++, ap++;
    onset2 = onset;
    ap2 = ap;
    while (onset2 < argc &&
        (ap2->a_type != A_SEMI && ap2->a_type != A_COMMA))
            onset2++, ap2++;
    if (onset2 > onset)
    {
        x->x_onset = onset2;
        if (ap->a_type == A_SYMBOL)
            outlet_anything(x->x_ob.ob_outlet, ap->a_w.w_symbol,
                onset2-onset-1, ap+1);
        else outlet_list(x->x_ob.ob_outlet, 0, onset2-onset, ap);
    }
    else
    {
        x->x_onset = 0x7fffffff;
        outlet_bang(x->x_bangout);
    }
}

static void textfile_rewind(t_qlist *x)
{
    x->x_onset = 0;
}

static void textfile_free(t_textfile *x)
{
    binbuf_free(x->x_binbuf);
}

void textfile_setup(void )
{
    textfile_class = class_new(gensym("textfile"), (t_newmethod)textfile_new,
        (t_method)textfile_free, sizeof(t_textfile), 0, 0);
    class_addmethod(textfile_class, (t_method)textfile_rewind, gensym("rewind"),
        0);
    class_addmethod(textfile_class, (t_method)qlist_set, gensym("set"),
        A_GIMME, 0);
    class_addmethod(textfile_class, (t_method)qlist_clear, gensym("clear"), 0);
    class_addmethod(textfile_class, (t_method)qlist_add, gensym("add"),
        A_GIMME, 0);
    class_addmethod(textfile_class, (t_method)qlist_add2, gensym("add2"),
        A_GIMME, 0);
    class_addmethod(textfile_class, (t_method)qlist_add, gensym("append"),
        A_GIMME, 0);
    class_addmethod(textfile_class, (t_method)qlist_read, gensym("read"), 
        A_SYMBOL, A_DEFSYM, 0);
    class_addmethod(textfile_class, (t_method)qlist_write, gensym("write"), 
        A_SYMBOL, A_DEFSYM, 0);
    class_addmethod(textfile_class, (t_method)qlist_print, gensym("print"),
        A_DEFSYM, 0);
    class_addbang(textfile_class, textfile_bang);
}

