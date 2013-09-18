/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "unstable/pd_imp.h"
#include "unstable/fragile.h"
#include "common/loud.h"

/* LATER handle canvas grabbing (bypass) */
/* LATER check self-grabbing */
/* LATER fragilize */

/* It would be nice to have write access to o_connections field... */

struct _outlet
{
    t_object *o_owner;
    struct _outlet *o_next;
    t_outconnect *o_connections;
    t_symbol *o_sym;
};

/* ...and to have bindlist traversal routines in Pd API. */

static t_class *bindlist_class = 0;

typedef struct _bindelem
{
    t_pd *e_who;
    struct _bindelem *e_next;
} t_bindelem;

typedef struct _bindlist
{
    t_pd b_pd;
    t_bindelem *b_list;
} t_bindlist;

typedef struct _grab
{
    t_object        x_ob;
    t_symbol       *x_target;
    int             x_noutlets;   /* not counting right one */
    t_outconnect  **x_grabcons;   /* grabbed connections */
    t_outlet       *x_rightout;   /* right outlet */
    /* traversal helpers: */
    t_object       *x_grabbed;    /* currently grabbed object */
    t_outconnect   *x_tograbbed;  /* a connection to grabbed object */
    int             x_ngrabout;   /* number of grabbed object's outlets */
    t_bindelem     *x_bindelem;
} t_grab;

static t_class *grab_class;

static void grab_start(t_grab *x)
{
    x->x_tograbbed = 0;
    x->x_bindelem = 0;
    if (x->x_target)
    {
	t_pd *proxy = x->x_target->s_thing;
	t_object *ob;
	if (proxy && bindlist_class)
	{
	    if (*proxy == bindlist_class)
	    {
		x->x_bindelem = ((t_bindlist *)proxy)->b_list;
		while (x->x_bindelem)
		{
		    if (ob = pd_checkobject(x->x_bindelem->e_who))
		    {
			x->x_tograbbed =
			    fragile_outlet_connections(ob->ob_outlet);
			return;
		    }
		    x->x_bindelem = x->x_bindelem->e_next;
		}
	    }
	    else if (ob = pd_checkobject(proxy))
		x->x_tograbbed = fragile_outlet_connections(ob->ob_outlet);
	}
    }
    else x->x_tograbbed = fragile_outlet_connections(x->x_rightout);
}

static t_pd *grab_next(t_grab *x)
{
nextremote:
    if (x->x_tograbbed)
    {
	int inno;
	x->x_tograbbed =
	    fragile_outlet_nextconnection(x->x_tograbbed, &x->x_grabbed, &inno);
	if (x->x_grabbed)
	{
	    if (inno)
	    {
		if (x->x_target)
		    loud_error((t_pd *)x,
			       "right outlet must feed leftmost inlet");
		else
		    loud_error((t_pd *)x,
			       "remote proxy must feed leftmost inlet");
	    }
	    else
	    {
		t_outlet *op;
		t_outlet *goutp;
		int goutno = x->x_noutlets;
		x->x_ngrabout = obj_noutlets(x->x_grabbed);
		if (goutno > x->x_ngrabout) goutno = x->x_ngrabout;
		while (goutno--)
		{
		    x->x_grabcons[goutno] =
			obj_starttraverseoutlet(x->x_grabbed, &goutp, goutno);
		    goutp->o_connections =
			obj_starttraverseoutlet((t_object *)x, &op, goutno);
		}
		return ((t_pd *)x->x_grabbed);
	    }
	}
    }
    if (x->x_bindelem) while (x->x_bindelem = x->x_bindelem->e_next)
    {
	t_object *ob;
	if (ob = pd_checkobject(x->x_bindelem->e_who))
	{
	    x->x_tograbbed = fragile_outlet_connections(ob->ob_outlet);
	    goto nextremote;
	}
    }
    return (0);
}

static void grab_restore(t_grab *x)
{
    t_outlet *goutp;
    int goutno = x->x_noutlets;
    if (goutno > x->x_ngrabout) goutno = x->x_ngrabout;
    while (goutno--)
    {
	obj_starttraverseoutlet(x->x_grabbed, &goutp, goutno);
	goutp->o_connections = x->x_grabcons[goutno];
    }
}

static void grab_bang(t_grab *x)
{
    t_pd *grabbed;
    grab_start(x);
    while (grabbed = grab_next(x))
    {
	pd_bang(grabbed);
	grab_restore(x);
    }
}

static void grab_float(t_grab *x, t_float f)
{
    t_pd *grabbed;
    grab_start(x);
    while (grabbed = grab_next(x))
    {
	pd_float(grabbed, f);
	grab_restore(x);
    }
}

static void grab_symbol(t_grab *x, t_symbol *s)
{
    t_pd *grabbed;
    grab_start(x);
    while (grabbed = grab_next(x))
    {
	pd_symbol(grabbed, s);
	grab_restore(x);
    }
}

static void grab_pointer(t_grab *x, t_gpointer *gp)
{
    t_pd *grabbed;
    grab_start(x);
    while (grabbed = grab_next(x))
    {
	pd_pointer(grabbed, gp);
	grab_restore(x);
    }
}

static void grab_list(t_grab *x, t_symbol *s, int ac, t_atom *av)
{
    t_pd *grabbed;
    grab_start(x);
    while (grabbed = grab_next(x))
    {
	pd_list(grabbed, s, ac, av);
	grab_restore(x);
    }
}

static void grab_anything(t_grab *x, t_symbol *s, int ac, t_atom *av)
{
    t_pd *grabbed;
    grab_start(x);
    while (grabbed = grab_next(x))
    {
	typedmess(grabbed, s, ac, av);
	grab_restore(x);
    }
}

static void grab_set(t_grab *x, t_symbol *s)
{
    if (x->x_target && s && s != &s_) x->x_target = s;
}

/* LATER use A_GIMME */
static void *grab_new(t_symbol *s, t_floatarg f)
{
    t_grab *x;
    t_outconnect **grabcons;
    int i, noutlets = (int)f;
    if (noutlets < 1) noutlets = 1;
    if (!(grabcons = getbytes(noutlets * sizeof(*grabcons))))
	return (0);
    x = (t_grab *)pd_new(grab_class);
    x->x_noutlets = noutlets;
    x->x_grabcons = grabcons;
    while (noutlets--) outlet_new((t_object *)x, &s_anything);
    if (s && s != &s_)
    {
	x->x_target = s;
	x->x_rightout = 0;
    }
    else
    {
	x->x_target = 0;
	x->x_rightout = outlet_new((t_object *)x, &s_anything);
    }
    return (x);
}

static void grab_free(t_grab *x)
{
    if (x->x_grabcons)
	freebytes(x->x_grabcons, x->x_noutlets * sizeof(*x->x_grabcons));
}

void grab_setup(void)
{
    t_symbol *s = gensym("grab");
    grab_class = class_new(s, (t_newmethod)grab_new,
			   (t_method)grab_free,
			   sizeof(t_grab), 0,
			   A_DEFFLOAT, A_DEFSYMBOL, 0);
    class_addfloat(grab_class, grab_float);
    class_addbang(grab_class, grab_bang);
    class_addsymbol(grab_class, grab_symbol);
    class_addpointer(grab_class, grab_pointer);
    class_addlist(grab_class, grab_list);
    class_addanything(grab_class, grab_anything);
    class_addmethod(grab_class, (t_method)grab_set,
		    gensym("set"), A_SYMBOL, 0);
    if (!bindlist_class)
    {
	t_class *c = grab_class;
	pd_bind(&grab_class, s);
	pd_bind(&c, s);
	if (!s->s_thing
	    || !(bindlist_class = *s->s_thing)
	    || bindlist_class->c_name != gensym("bindlist"))
	    error("grab: failure to initialize remote grabbing feature");
	pd_unbind(&c, s);
	pd_unbind(&grab_class, s);
    }
}
