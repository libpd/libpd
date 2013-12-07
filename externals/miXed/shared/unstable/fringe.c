/* Copyright (c) 1997-2003 Miller Puckette, krzYszcz, and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* Put here calls which are likely to (and should) make into Pd API some day. */

#include "m_pd.h"
#include "g_canvas.h"
#include "unstable/forky.h"
#include "unstable/fringe.h"

#ifdef KRZYSZCZ
//#define FRINGE_DEBUG
#endif

static int gobj_getindex(t_glist *gl, t_gobj *ob)
{
    t_gobj *ob1;
    int ndx;
    for (ob1 = gl->gl_list, ndx = 0; ob1 && ob1 != ob; ob1 = ob1->g_next)
    	ndx++;
    return (ndx);
}

static void gobj_totail(t_glist *gl, t_gobj *ob)
{
    if (ob->g_next)
    {
	t_gobj *ob1;
	if (ob == gl->gl_list) ob1 = gl->gl_list = ob->g_next;
	else
	{
	    for (ob1 = gl->gl_list; ob1; ob1 = ob1->g_next)
		if (ob1->g_next == ob)
		    break;
	    if (!ob1)
	    {
		bug("gobj_totail");
		return;
	    }
	    ob1->g_next = ob->g_next;
	    ob1 = ob1->g_next;
	}
	while (ob1->g_next) ob1 = ob1->g_next;
	ob1->g_next = ob;
	ob->g_next = 0;
    }
}

static void gobj_stowconnections(t_glist *gl, t_gobj *ob, t_binbuf *bb)
{
    t_linetraverser lt;
    t_outconnect *oc;
    binbuf_clear(bb);
    linetraverser_start(&lt, gl);
    while (oc = linetraverser_next(&lt))
    {
	if ((t_gobj *)lt.tr_ob == ob || (t_gobj *)lt.tr_ob2 == ob)
	    binbuf_addv(bb, "ssiiii;",
			gensym("#X"), gensym("connect"),
			gobj_getindex(gl, (t_gobj *)lt.tr_ob), lt.tr_outno,
			gobj_getindex(gl, (t_gobj *)lt.tr_ob2), lt.tr_inno);
    }
#ifdef FRINGE_DEBUG
    fprintf(stderr, "packed connections:\n");
    binbuf_print(bb);
#endif
}

static void gobj_restoreconnections(t_glist *gl, t_binbuf *bb)
{
#ifdef FRINGE_DEBUG
    fprintf(stderr, "restoring connections:\n");
    binbuf_print(bb);
#endif
    canvas_setcurrent(gl);
    binbuf_eval(bb, 0, 0, 0);
    canvas_unsetcurrent(gl);
}

void gobj_recreate(t_glist *gl, t_gobj *ob, t_binbuf *bb)
{
    /* LATER revisit all gobj calls, and sort out the gop case */
    t_text *newt;
    int xpix = ((t_text *)ob)->te_xpix, ypix = ((t_text *)ob)->te_ypix;
    t_binbuf *bb1 = binbuf_new();
    int ac = binbuf_getnatom(bb);
    t_atom *av = binbuf_getvec(bb);
    canvas_setcurrent(gl);
    gobj_totail(gl, ob);
    gobj_stowconnections(gl, ob, bb1);
    glist_delete(gl, ob);
    if (newt = (t_text *)forky_newobject(av->a_w.w_symbol, ac - 1, av + 1))
    {
	newt->te_binbuf = bb;
	newt->te_xpix = xpix;
	newt->te_ypix = ypix;
	newt->te_width = 0;
	newt->te_type = T_OBJECT;
	glist_add(gl, (t_gobj *)newt);
	gobj_restoreconnections(gl, bb1);
    }
    else bug("gobj_recreate");
    binbuf_free(bb1);
    canvas_unsetcurrent(gl);
}
