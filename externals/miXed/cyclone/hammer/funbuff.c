/* Copyright (c) 2002-2004 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "common/loud.h"
#include "common/vefl.h"
#include "hammer/tree.h"
#include "hammer/file.h"

typedef struct _funbuff
{
    t_object   x_ob;
    t_canvas  *x_canvas;
    t_symbol  *x_defname;
    t_float    x_value;
    int        x_valueset;
    /* CHECKED filling with a large set, then sending 'goto', 'read', 'next'...
       outputs the previous, replaced contents (same with deletion)
       -- apparently a node pointer is stored, corrupt in these cases */
    t_hammernode  *x_pointer;
    int            x_pointerset;  /* set-with-goto flag */
    int            x_lastdelta;
    int            x_embedflag;
    t_hammerfile  *x_filehandle;
    t_hammertree   x_tree;
    t_outlet      *x_deltaout;
    t_outlet      *x_bangout;
} t_funbuff;

static t_class *funbuff_class;

static void funbuff_dooutput(t_funbuff *x, float value, float delta)
{
    /* CHECKED lastdelta sent for 'next', 'float', 'min', 'max',
       'interp', 'find' */
    outlet_float(x->x_deltaout, delta);
    outlet_float(((t_object *)x)->ob_outlet, value);
}

static void funbuff_bang(t_funbuff *x)
{
    t_hammernode *np;
    int count = 0;
    int xmin = 0, xmax = 0;
    t_float ymin = 0, ymax = 0;
    if (np = x->x_tree.t_first)
    {
	/* LATER consider using extra fields, updated on the fly */
	count = 1;
	xmin = np->n_key;
	xmax = x->x_tree.t_last->n_key;
	ymin = ymax = HAMMERNODE_GETFLOAT(np);
	while (np = np->n_next)
	{
	    t_float f = HAMMERNODE_GETFLOAT(np);
	    if (f < ymin)
		ymin = f;
	    else if (f > ymax)
		ymax = f;
	    count++;
	}
    }
    /* format CHECKED */
    post("funbuff info:  %d elements long", count);  /* CHECKED 0 and 1 */
    if (count)
    {
	post(" -> minX= %d maxX= %d", xmin, xmax);
	post(" -> minY= %g maxY= %g", ymin, ymax);
	post(" -> domain= %d range= %g", xmax - xmin, ymax - ymin);
    }
}

static void funbuff_float(t_funbuff *x, t_float f)
{
    int ndx = (int)f;  /* CHECKED float is silently truncated */
    t_hammernode *np;
    if (x->x_valueset)
    {
	np = hammertree_insertfloat(&x->x_tree, ndx, x->x_value, 1);
	x->x_valueset = 0;
    }
    else if (np = hammertree_closest(&x->x_tree, ndx, 0))
	funbuff_dooutput(x, HAMMERNODE_GETFLOAT(np), x->x_lastdelta);
    /* CHECKED pointer is updated --
       'next' outputs np also in a !valueset case (it is sent twice) */
    x->x_pointer = np;  /* FIXME */
    x->x_pointerset = 0;
}

static void funbuff_ft1(t_funbuff *x, t_floatarg f)
{
    /* this is incompatible -- CHECKED float is silently truncated */
    x->x_value = f;
    x->x_valueset = 1;
}

static void funbuff_clear(t_funbuff *x)
{
    hammertree_clear(&x->x_tree, 0);
    /* CHECKED valueset is not cleared */
    x->x_pointer = 0;
}

/* LATER dirty flag handling */
static void funbuff_embed(t_funbuff *x, t_floatarg f)
{
    x->x_embedflag = (f != 0);
}

static void funbuff_goto(t_funbuff *x, t_floatarg f)
{
    /* CHECKED truncation */
    x->x_pointer = hammertree_closest(&x->x_tree, (int)f, 1);
    x->x_pointerset = 1;  /* CHECKED delta output by 'next' will be zero */
}

/* LATER consider using an extra field, updated on the fly */
static void funbuff_min(t_funbuff *x)
{
    t_hammernode *np;
    if (np = x->x_tree.t_first)  /* CHECKED nop if empty */
    {
	t_float result = HAMMERNODE_GETFLOAT(np);
	while (np = np->n_next)
	    if (HAMMERNODE_GETFLOAT(np) < result)
		result = HAMMERNODE_GETFLOAT(np);
	funbuff_dooutput(x, result, x->x_lastdelta);
	/* CHECKED pointer not updated */
    }
}

/* LATER consider using an extra field, updated on the fly */
static void funbuff_max(t_funbuff *x)
{
    t_hammernode *np;
    if (np = x->x_tree.t_first)  /* CHECKED nop if empty */
    {
	t_float result = HAMMERNODE_GETFLOAT(np);
	while (np = np->n_next)
	    if (HAMMERNODE_GETFLOAT(np) > result)
		result = HAMMERNODE_GETFLOAT(np);
	funbuff_dooutput(x, result, x->x_lastdelta);
	/* CHECKED pointer not updated */
    }
}

static void funbuff_next(t_funbuff *x)
{
    t_hammernode *np;
    if (!x->x_tree.t_root)
	return;
    if (!(np = x->x_pointer))
    {
	outlet_bang(x->x_bangout);
	/* CHECKED banging until reset */
	return;
    }
    if (x->x_pointerset)
	x->x_lastdelta = 0;
    else if (np->n_prev)
	x->x_lastdelta = np->n_key - np->n_prev->n_key;
    else
	x->x_lastdelta = 0;  /* CHECKED corrupt delta sent here... */
    funbuff_dooutput(x, HAMMERNODE_GETFLOAT(np), x->x_lastdelta);
    x->x_pointer = np->n_next;
    x->x_pointerset = 0;
}

static void funbuff_set(t_funbuff *x, t_symbol *s, int ac, t_atom *av)
{
    /* CHECKED symbols somehow bashed to zeros,
       decreasing x coords corrupt the funbuff -- not emulated here... */
    int i = ac;
    t_atom *ap = av;
    while (i--) if (ap++->a_type != A_FLOAT)
    {
	loud_error((t_pd *)x, "bad input (not a number) -- no data to set");
	return;
    }
    if (!ac || (ac % 2))
    {
	/* CHECKED odd/null ac loudly rejected, current contents preserved */
	loud_error((t_pd *)x, "bad input (%s) -- no data to set",
		   (ac ? "odd arg count" : "no input"));
	return;
    }
    funbuff_clear(x);  /* CHECKED the contents is replaced */
    while (ac--)
    {
	int ndx = (int)av++->a_w.w_float;
	if (!hammertree_insertfloat(&x->x_tree, ndx, av++->a_w.w_float, 1))
	    return;
	ac--;
    }
}

static void funbuff_doread(t_funbuff *x, t_symbol *fn)
{
    t_binbuf *bb = binbuf_new();
    int ac;
    t_atom *av;
    char buf[MAXPDSTRING];
    /* FIXME use open_via_path() */
    canvas_makefilename(x->x_canvas, fn->s_name, buf, MAXPDSTRING);
    binbuf_read(bb, buf, "", 0);
    if ((ac = binbuf_getnatom(bb)) &&
	(av = binbuf_getvec(bb)) &&
	av->a_type == A_SYMBOL &&
	av->a_w.w_symbol == gensym("funbuff"))
    {
	post("funbuff_read: %s read successful", fn->s_name);  /* CHECKED */
	funbuff_set(x, 0, ac-1, av+1);
    }
    else  /* CHECKED no complaints... */
	loud_error((t_pd *)x, "invalid file %s", fn->s_name);
    binbuf_free(bb);
}

static void funbuff_readhook(t_pd *z, t_symbol *fn, int ac, t_atom *av)
{
    funbuff_doread((t_funbuff *)z, fn);
}

static void funbuff_dowrite(t_funbuff *x, t_symbol *fn)
{
    t_binbuf *bb = binbuf_new();
    char buf[MAXPDSTRING];
    t_hammernode *np;
    binbuf_addv(bb, "s", atom_getsymbol(binbuf_getvec(x->x_ob.te_binbuf)));
    for (np = x->x_tree.t_first; np; np = np->n_next)
	binbuf_addv(bb, "if", np->n_key, HAMMERNODE_GETFLOAT(np));
    canvas_makefilename(x->x_canvas, fn->s_name, buf, MAXPDSTRING);
    binbuf_write(bb, buf, "", 0);
    binbuf_free(bb);
}

static void funbuff_writehook(t_pd *z, t_symbol *fn, int ac, t_atom *av)
{
    funbuff_dowrite((t_funbuff *)z, fn);
}

static void funbuff_embedhook(t_pd *z, t_binbuf *bb, t_symbol *bindsym)
{
    t_funbuff *x = (t_funbuff *)z;
    if (x->x_embedflag)
    {
	t_hammernode *np;
	binbuf_addv(bb, "ssi;", bindsym, gensym("embed"), 1);
	if (np = x->x_tree.t_first)
	{
	    binbuf_addv(bb, "ss", bindsym, gensym("set"));
	    for (; np; np = np->n_next)
		binbuf_addv(bb, "if", np->n_key, HAMMERNODE_GETFLOAT(np));
	    binbuf_addsemi(bb);
	}
    }
}

/* CHECKED symbol arg ok */
static void funbuff_read(t_funbuff *x, t_symbol *s)
{
    if (s && s != &s_)
	funbuff_doread(x, s);
    else
	hammerpanel_open(x->x_filehandle, 0);
}

/* CHECKED symbol arg not allowed --
   a bug? but CHECKME other classes (cf seq's filetype dilemma) */
static void funbuff_write(t_funbuff *x, t_symbol *s)
{
    if (s && s != &s_)
	funbuff_dowrite(x, s);
    else  /* CHECKME default name */
	hammerpanel_save(x->x_filehandle,
			 canvas_getdir(x->x_canvas), x->x_defname);
}

static void funbuff_delete(t_funbuff *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac && av->a_type == A_FLOAT &&
	(ac == 1 || (ac == 2 && av[1].a_type == A_FLOAT)))
    {
	/* CHECKED float is silently truncated */
	int ndx = (int)av->a_w.w_float;
	t_hammernode *np;
	if ((np = hammertree_search(&x->x_tree, ndx)) &&
	    (ac == 1 || HAMMERNODE_GETFLOAT(np) == av[1].a_w.w_float))
	{
	    if (np == x->x_pointer)
		x->x_pointer = 0;  /* CHECKED corrupt pointer left here... */
	    hammertree_delete(&x->x_tree, np);  /* FIXME */
	}
	/* CHECKED mismatch silently ignored */
    }
    else loud_messarg((t_pd *)x, s);  /* CHECKED */
}

static void funbuff_find(t_funbuff *x, t_floatarg f)
{
    t_hammernode *np;
    if (np = x->x_tree.t_first)
    {
	do
	{
	    /* CHECKED lastdelta preserved */
	    if (HAMMERNODE_GETFLOAT(np) == f)
		funbuff_dooutput(x, np->n_key, x->x_lastdelta);
	}
	while (np = np->n_next);
	/* CHECKED no bangout, no complaint if nothing found */
    }
    else loud_error((t_pd *)x, "nothing to find");  /* CHECKED */
}

static void funbuff_dump(t_funbuff *x)
{
    t_hammernode *np;
    if (np = x->x_tree.t_first)
    {
	do
	{
	    x->x_lastdelta = HAMMERNODE_GETFLOAT(np);  /* CHECKED */
	    /* float value preserved (this is incompatible) */
	    funbuff_dooutput(x, np->n_key, x->x_lastdelta);
	}
	while (np = np->n_next);
	/* CHECKED no bangout */
    }
    else loud_error((t_pd *)x, "nothing to dump");  /* CHECKED */
}

/* CHECKME if pointer is updated */
static void funbuff_dointerp(t_funbuff *x, t_floatarg f, int vsz, t_float *vec)
{
    t_hammernode *np1;
    int trunc = (int)f;
    if (trunc > f) trunc--;  /* CHECKME negative floats */
    if (np1 = hammertree_closest(&x->x_tree, trunc, 0))
    {
	float value = HAMMERNODE_GETFLOAT(np1);
	t_hammernode *np2 = np1->n_next;
	if (np2)
	{
	    float delta = (float)(np2->n_key - np1->n_key);
	    /* this is incompatible -- CHECKED float argument is silently
	       truncated (which does not make much sense here), CHECKME again */
	    float frac = f - np1->n_key;
	    if (frac < 0 || frac >= delta)
	    {
		loudbug_bug("funbuff_dointerp");
		return;
	    }
	    frac /= delta;
	    if (vec)
	    {
		/* CHECKME */
		float vpos = (vsz - 1) * frac;
		int vndx = (int)vpos;
		float vfrac = vpos - vndx;
		if (vndx < 0 || vndx >= vsz - 1)
		{
		    loudbug_bug("funbuff_dointerp redundant test...");
		    return;
		}
		vec += vndx;
		frac = *vec + (vec[1] - *vec) * vfrac;
	    }
	    value +=
		(HAMMERNODE_GETFLOAT(np2) - HAMMERNODE_GETFLOAT(np1)) * frac;
	}
	funbuff_dooutput(x, value, x->x_lastdelta);  /* CHECKME !np2 */
    }
    else if (np1 = hammertree_closest(&x->x_tree, trunc, 1))
	/* CHECKME */
	funbuff_dooutput(x, HAMMERNODE_GETFLOAT(np1), x->x_lastdelta);
}

static void funbuff_interp(t_funbuff *x, t_floatarg f)
{
    funbuff_dointerp(x, f, 0, 0);
}

static void funbuff_interptab(t_funbuff *x, t_symbol *s, t_floatarg f)
{
    int vsz;
    t_float *vec;
    if (vec = vefl_get(s, &vsz, 0, (t_pd *)x))
    {
	if (vsz > 2)
	    funbuff_dointerp(x, f, vsz, vec);
	else
	    funbuff_dointerp(x, f, 0, 0);
    }
}

static void funbuff_reduce(t_funbuff *x, t_floatarg f)
{
    loud_notimplemented((t_pd *)x, "reduce");
}

static void funbuff_select(t_funbuff *x, t_floatarg f1, t_floatarg f2)
{
    loud_notimplemented((t_pd *)x, "select");
}

/* CHECKED (sub)buffer's copy is stored, as expected --
   'delete' does not modify the clipboard */
/* CHECKED cut entire contents if no selection */
static void funbuff_cut(t_funbuff *x)
{
    loud_notimplemented((t_pd *)x, "cut");
}

/* CHECKED copy entire contents if no selection */
static void funbuff_copy(t_funbuff *x)
{
    loud_notimplemented((t_pd *)x, "copy");
}

static void funbuff_paste(t_funbuff *x)
{
    loud_notimplemented((t_pd *)x, "paste");
}

static void funbuff_undo(t_funbuff *x)
{
    /* CHECKED apparently not working in 4.07 */
    loud_notimplemented((t_pd *)x, "undo");
}

#ifdef HAMMERTREE_DEBUG
static void funbuff_debug(t_funbuff *x, t_floatarg f)
{
    hammertree_debug(&x->x_tree, (int)f, 0);
}
#endif

static void funbuff_free(t_funbuff *x)
{
    hammerfile_free(x->x_filehandle);
    hammertree_clear(&x->x_tree, 0);
}

static void *funbuff_new(t_symbol *s)
{
    t_funbuff *x = (t_funbuff *)pd_new(funbuff_class);
    x->x_canvas = canvas_getcurrent();
    x->x_valueset = 0;
    x->x_pointer = 0;
    x->x_pointerset = 0;  /* CHECKME, rename to intraversal? */
    x->x_lastdelta = 0;
    x->x_embedflag = 0;
    hammertree_inittyped(&x->x_tree, HAMMERTYPE_FLOAT, 0);
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft1"));
    outlet_new((t_object *)x, &s_float);
    x->x_deltaout = outlet_new((t_object *)x, &s_float);
    x->x_bangout = outlet_new((t_object *)x, &s_bang);
    if (s && s != &s_)
    {
	x->x_defname = s;  /* CHECKME if 'read' changes this */
	funbuff_doread(x, s);
    }
    else x->x_defname = &s_;
    x->x_filehandle = hammerfile_new((t_pd *)x, funbuff_embedhook,
				     funbuff_readhook, funbuff_writehook, 0);
    return (x);
}

void funbuff_setup(void)
{
    funbuff_class = class_new(gensym("funbuff"),
			      (t_newmethod)funbuff_new,
			      (t_method)funbuff_free,
			      sizeof(t_funbuff), 0, A_DEFSYM, 0);
    class_addbang(funbuff_class, funbuff_bang);
    class_addfloat(funbuff_class, funbuff_float);
    class_addmethod(funbuff_class, (t_method)funbuff_ft1,
		    gensym("ft1"), A_FLOAT, 0);
    class_addmethod(funbuff_class, (t_method)funbuff_clear,
		    gensym("clear"), 0);
    class_addmethod(funbuff_class, (t_method)funbuff_goto,
		    gensym("goto"), A_FLOAT, 0);
    class_addmethod(funbuff_class, (t_method)funbuff_min,
		    gensym("min"), 0);
    class_addmethod(funbuff_class, (t_method)funbuff_max,
		    gensym("max"), 0);
    class_addmethod(funbuff_class, (t_method)funbuff_next,
		    gensym("next"), 0);
    class_addmethod(funbuff_class, (t_method)funbuff_embed,
		    gensym("embed"), A_FLOAT, 0);
    class_addmethod(funbuff_class, (t_method)funbuff_read,
		    gensym("read"), A_DEFSYM, 0);
    class_addmethod(funbuff_class, (t_method)funbuff_write,
		    gensym("write"), A_DEFSYM, 0);
    class_addmethod(funbuff_class, (t_method)funbuff_set,
		    gensym("set"), A_GIMME, 0);
    class_addmethod(funbuff_class, (t_method)funbuff_delete,
		    gensym("delete"), A_GIMME, 0);
    class_addmethod(funbuff_class, (t_method)funbuff_find,
		    gensym("find"), A_FLOAT, 0);
    class_addmethod(funbuff_class, (t_method)funbuff_dump,
		    gensym("dump"), 0);
    class_addmethod(funbuff_class, (t_method)funbuff_interp,
		    gensym("interp"), A_FLOAT, 0);
    class_addmethod(funbuff_class, (t_method)funbuff_interptab,
		    gensym("interptab"), A_FLOAT, A_SYMBOL, 0);
    class_addmethod(funbuff_class, (t_method)funbuff_reduce,
		    gensym("reduce"), A_FLOAT, 0);
    class_addmethod(funbuff_class, (t_method)funbuff_select,
		    gensym("select"), A_FLOAT, A_FLOAT, 0);
    class_addmethod(funbuff_class, (t_method)funbuff_cut,
		    gensym("cut"), 0);
    class_addmethod(funbuff_class, (t_method)funbuff_copy,
		    gensym("copy"), 0);
    class_addmethod(funbuff_class, (t_method)funbuff_paste,
		    gensym("paste"), 0);
    class_addmethod(funbuff_class, (t_method)funbuff_undo,
		    gensym("undo"), 0);
#ifdef HAMMERTREE_DEBUG
    class_addmethod(funbuff_class, (t_method)funbuff_debug,
		    gensym("debug"), A_DEFFLOAT, 0);
#endif
    hammerfile_setup(funbuff_class, 1);
}
