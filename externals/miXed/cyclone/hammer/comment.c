/* Copyright (c) 2002-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* FIXME creation lag (X-specific) */
/* LATER think about pushing text to the text editor (ctrl-t)
   -- not easy, because we are not 'textedfor' */
/* LATER think about making the <Button> binding for the entire bbox,
   instead of the text item, to ease the pain of resizing, somewhat. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "m_pd.h"
#include "g_canvas.h"
#include "common/loud.h"
#include "unstable/forky.h"

/* our proxy of the text_class (not in the API), LATER do not cheat */
static t_class *makeshift_class;

#ifdef KRZYSZCZ
//#define COMMENT_DEBUG
#endif

#define COMMENT_LMARGIN        1
#define COMMENT_RMARGIN        1
#define COMMENT_TMARGIN        2
#define COMMENT_BMARGIN        2
#define COMMENT_MINWIDTH       8
#define COMMENT_HANDLEWIDTH    8
#define COMMENT_OUTBUFSIZE  1000

typedef struct _comment
{
    t_object   x_ob;
    t_glist   *x_glist;
    t_canvas  *x_canvas;  /* also an 'isvised' flag */
    t_symbol  *x_bindsym;
    char       x_tag[32];
    char       x_texttag[32];
    char       x_outlinetag[32];
    t_clock   *x_transclock;
    t_binbuf  *x_binbuf;
    char      *x_textbuf;
    int        x_textbufsize;
    int        x_pixwidth;
    int        x_bbset;
    int        x_bbpending;
    int        x_x1;
    int        x_y1;
    int        x_x2;
    int        x_y2;
    int        x_newx2;
    int        x_dragon;
    int        x_fontsize;    /* requested size */
    t_symbol  *x_fontfamily;  /* requested family */
    int        x_fontprops;   /* LATER pack weight and slant */
    t_symbol  *x_encoding;    /* requested encoding */
    unsigned char  x_red;
    unsigned char  x_green;
    unsigned char  x_blue;
    char       x_color[8];
    int        x_selstart;
    int        x_selend;
    int        x_active;
    int        x_ready;
} t_comment;

static t_class *comment_class;
static t_class *commentsink_class;

static t_pd *commentsink = 0;

static void comment_draw(t_comment *x)
{
    char buf[COMMENT_OUTBUFSIZE], *outbuf, *outp;
    int cvid = (int)x->x_canvas;
    int reqsize = x->x_textbufsize + 250;  /* FIXME estimation */
    if (reqsize > COMMENT_OUTBUFSIZE)
    {
#ifdef COMMENT_DEBUG
	loudbug_post("allocating %d outbuf bytes", reqsize);
#endif
	if (!(outbuf = getbytes(reqsize)))
	    return;
    }
    else outbuf = buf;
    outp = outbuf;
    sprintf(outp, "comment_draw %s .x%lx.c %s %s %f %f %s %d %s %s {%.*s} %d\n",
	    x->x_bindsym->s_name, cvid, x->x_texttag, x->x_tag,
	    (float)(text_xpix((t_text *)x, x->x_glist) + COMMENT_LMARGIN),
	    (float)(text_ypix((t_text *)x, x->x_glist) + COMMENT_TMARGIN),
	    x->x_fontfamily->s_name, x->x_fontsize,
	    (glist_isselected(x->x_glist, &x->x_glist->gl_gobj) ?
	     "blue" : x->x_color),
	    (x->x_encoding ? x->x_encoding->s_name : "\"\""),
	    x->x_textbufsize, x->x_textbuf, x->x_pixwidth);
    x->x_bbpending = 1;
    sys_gui(outbuf);
    if (outbuf != buf) freebytes(outbuf, reqsize);
}

static void comment_update(t_comment *x)
{
    char buf[COMMENT_OUTBUFSIZE], *outbuf, *outp;
    int cvid = (int)x->x_canvas;
    int reqsize = x->x_textbufsize + 250;  /* FIXME estimation */
    if (reqsize > COMMENT_OUTBUFSIZE)
    {
#ifdef COMMENT_DEBUG
	loudbug_post("allocating %d outbuf bytes", reqsize);
#endif
	if (!(outbuf = getbytes(reqsize)))
	    return;
    }
    else outbuf = buf;
    outp = outbuf;
    sprintf(outp, "comment_update .x%lx.c %s %s {%.*s} %d\n", cvid,
	    x->x_texttag, (x->x_encoding ? x->x_encoding->s_name : "\"\""),
	    x->x_textbufsize, x->x_textbuf, x->x_pixwidth);
    outp += strlen(outp);
    if (x->x_active)
    {
	if (x->x_selend > x->x_selstart)
	{
	    sprintf(outp, ".x%lx.c select from %s %d\n",
		    cvid, x->x_texttag, x->x_selstart);
	    outp += strlen(outp);
	    sprintf(outp, ".x%lx.c select to %s %d\n",
		    cvid, x->x_texttag, x->x_selend);
	    outp += strlen(outp);
	    sprintf(outp, ".x%lx.c focus {}\n", cvid);
	}
	else
	{
	    sprintf(outp, ".x%lx.c select clear\n", cvid);
	    outp += strlen(outp);
	    sprintf(outp, ".x%lx.c icursor %s %d\n",
		    cvid, x->x_texttag, x->x_selstart);
	    outp += strlen(outp);
	    sprintf(outp, ".x%lx.c focus %s\n", cvid, x->x_texttag);
	}
	outp += strlen(outp);
    }
    sprintf(outp, "comment_bbox %s .x%lx.c %s\n",
	    x->x_bindsym->s_name, cvid, x->x_texttag);
    x->x_bbpending = 1;
    sys_gui(outbuf);
    if (outbuf != buf) freebytes(outbuf, reqsize);
}

static void comment_validate(t_comment *x, t_glist *glist)
{
    if (!x->x_ready)
    {
	t_text *t = (t_text *)x;
	binbuf_free(t->te_binbuf);
	t->te_binbuf = x->x_binbuf;
	if (x->x_textbuf) freebytes(x->x_textbuf, x->x_textbufsize);
	binbuf_gettext(x->x_binbuf, &x->x_textbuf, &x->x_textbufsize);
	x->x_ready = 1;
#ifdef COMMENT_DEBUG
	loudbug_post("validation done");
#endif
    }
    if (glist)
    {
	if (glist != x->x_glist)
	{
	    loudbug_bug("comment_getcanvas");
	    x->x_glist = glist;
	}
	x->x_canvas = glist_getcanvas(glist);
    }
}

static void comment_grabbedkey(void *z, t_floatarg f)
{
    /* LATER think about replacing #key binding/comment_float() with grabbing */
#ifdef COMMENT_DEBUG
    loudbug_post("comment_grabbedkey %g", f);
#endif
}

static void comment_dograb(t_comment *x)
{
    /* LATER investigate the grabbing feature.
       Here we use it just to prevent backspace from erasing entire text.
       This has to be done also when we are already active, because
       after being clicked at we have lost our previous grab. */
    glist_grab(x->x_glist, (t_gobj *)x, 0, comment_grabbedkey, 0, 0);
}

static void comment__bboxhook(t_comment *x, t_symbol *bindsym,
			      t_floatarg x1, t_floatarg y1,
			      t_floatarg x2, t_floatarg y2)
{
#ifdef COMMENT_DEBUG
    loudbug_post("bbox %g %g %g %g", x1, y1, x2, y2);
#endif
    x->x_x1 = x1;
    x->x_y1 = y1;
    x->x_x2 = x2;
    x->x_y2 = y2;
    x->x_bbset = 1;
    x->x_bbpending = 0;
}

static void comment__clickhook(t_comment *x, t_symbol *s, int ac, t_atom *av)
{
    int xx, yy, ndx;
    if (ac == 8 && av->a_type == A_SYMBOL
	&& av[1].a_type == A_FLOAT && av[2].a_type == A_FLOAT
	&& av[3].a_type == A_FLOAT
	&& av[4].a_type == A_FLOAT && av[5].a_type == A_FLOAT
	&& av[6].a_type == A_FLOAT && av[7].a_type == A_FLOAT)
    {
	xx = (int)av[1].a_w.w_float;
	yy = (int)av[2].a_w.w_float;
	ndx = (int)av[3].a_w.w_float;
	comment__bboxhook(x, av->a_w.w_symbol,
			  av[4].a_w.w_float, av[5].a_w.w_float,
			  av[6].a_w.w_float, av[7].a_w.w_float);
    }
    else
    {
	loudbug_bug("comment__clickhook");
	return;
    }
    if (x->x_glist->gl_edit)
    {
	if (x->x_active)
	{
	    if (ndx >= 0 && ndx < x->x_textbufsize)
	    {
		/* set selection, LATER shift-click and drag */
		x->x_selstart = x->x_selend = ndx;
		comment_dograb(x);
		comment_update(x);
	    }
	}
	else if (xx > x->x_x2 - COMMENT_HANDLEWIDTH)
	{
	    /* start resizing */
	    char buf[COMMENT_OUTBUFSIZE], *outp = buf;
	    int cvid = (int)x->x_canvas;
	    sprintf(outp, ".x%lx.c bind %s <ButtonRelease> \
 {pdsend {%s _release %s}}\n", cvid, x->x_texttag,
		    x->x_bindsym->s_name, x->x_bindsym->s_name);
	    outp += strlen(outp);
	    sprintf(outp, ".x%lx.c bind %s <Motion> \
 {pdsend {%s _motion %s %%x %%y}}\n", cvid, x->x_texttag,
		    x->x_bindsym->s_name, x->x_bindsym->s_name);
	    outp += strlen(outp);
	    sprintf(outp, ".x%lx.c create rectangle %d %d %d %d -outline blue \
 -tags {%s %s}\n",
		    cvid, x->x_x1, x->x_y1, x->x_x2, x->x_y2,
		    x->x_outlinetag, x->x_tag);
	    sys_gui(buf);
	    x->x_newx2 = x->x_x2;
	    x->x_dragon = 1;
	}
    }
}

static void comment__releasehook(t_comment *x, t_symbol *bindsym)
{
    int cvid = (int)x->x_canvas;
    sys_vgui(".x%lx.c bind %s <ButtonRelease> {}\n", cvid, x->x_texttag);
    sys_vgui(".x%lx.c bind %s <Motion> {}\n", cvid, x->x_texttag);
    sys_vgui(".x%lx.c delete %s\n", cvid, x->x_outlinetag);
    x->x_dragon = 0;
    if (x->x_newx2 != x->x_x2)
    {
	x->x_pixwidth = x->x_newx2 - x->x_x1;
	x->x_x2 = x->x_newx2;
	comment_update(x);
    }
}

static void comment__motionhook(t_comment *x, t_symbol *bindsym,
				t_floatarg xx, t_floatarg yy)
{
    int cvid = (int)x->x_canvas;
    if (xx > x->x_x1 + COMMENT_MINWIDTH)
	sys_vgui(".x%lx.c coords %s %d %d %d %d\n",
		 cvid, x->x_outlinetag,
		 x->x_x1, x->x_y1, x->x_newx2 = xx, x->x_y2);
}

static void commentsink__bboxhook(t_pd *x, t_symbol *bindsym,
				  t_floatarg x1, t_floatarg y1,
				  t_floatarg x2, t_floatarg y2)
{
    if (bindsym->s_thing == x)  /* is the comment gone? */
    {
	pd_unbind(x, bindsym);  /* if so, no need for this binding anymore */
#ifdef COMMENT_DEBUG
	loudbug_post("sink: %s unbound", bindsym->s_name);
#endif
    }
}

static void commentsink_anything(t_pd *x, t_symbol *s, int ac, t_atom *av)
{
    /* nop */
}

static void comment_getrect(t_gobj *z, t_glist *glist,
			    int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_comment *x = (t_comment *)z;
    if (!glist->gl_havewindow)
    {
	/* LATER revisit gop behaviour.  Currently text_shouldvis() returns
	   true if we are on parent.  Here we return a null rectangle,
	   so that any true ui object is accessible, even if it happens
	   to be covered by a comment. */
	*xp1 = *yp1 = *xp2 = *yp2 = 0;
	return;
    }
    if (x->x_bbset)
    {
	/* LATER think about margins */
	*xp1 = x->x_x1;
	*yp1 = x->x_y1;
	*xp2 = x->x_x2;
	*yp2 = x->x_y2;
    }
    else
    {
	int width,  height;
	float x1, y1, x2, y2;
	comment_validate(x, glist);
	if ((width = x->x_pixwidth) < 1)
	    /* FIXME estimation */
	    width = x->x_fontsize * x->x_textbufsize;
	width += COMMENT_LMARGIN + COMMENT_RMARGIN;
	/* FIXME estimation */
	height = x->x_fontsize + COMMENT_TMARGIN + COMMENT_BMARGIN;
	x1 = text_xpix((t_text *)x, glist);
	y1 = text_ypix((t_text *)x, glist) + 1;  /* LATER revisit */
	x2 = x1 + width;
	y2 = y1 + height - 2;  /* LATER revisit */
#ifdef COMMENT_DEBUG
	loudbug_post("estimated rectangle: %g %g %g %g", x1, y1, x2, y2);
#endif
	*xp1 = x1;
	*yp1 = y1;
	*xp2 = x2;
	*yp2 = y2;
    }
}

static void comment_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_comment *x = (t_comment *)z;
    if (!x->x_active && !x->x_dragon)  /* LATER rethink */
    {
	t_text *t = (t_text *)z;
	comment_validate(x, glist);
	t->te_xpix += dx;
	t->te_ypix += dy;
	if (x->x_bbset)
	{
	    x->x_x1 += dx;
	    x->x_y1 += dy;
	    x->x_x2 += dx;
	    x->x_y2 += dy;
	}
	if (glist_isvisible(glist))
	    sys_vgui(".x%lx.c move %s %d %d\n", x->x_canvas, x->x_tag, dx, dy);
    }
}

static void comment_activate(t_gobj *z, t_glist *glist, int state)
{
    t_comment *x = (t_comment *)z;
    comment_validate(x, glist);
    if (state)
    {
	comment_dograb(x);
	if (x->x_active)
	    return;
	sys_vgui(".x%lx.c focus %s\n", x->x_canvas, x->x_texttag);
	x->x_selstart = 0;
	x->x_selend = x->x_textbufsize;
	x->x_active = 1;
	pd_bind((t_pd *)x, gensym("#key"));
	pd_bind((t_pd *)x, gensym("#keyname"));
    }
    else
    {
	if (!x->x_active)
	    return;
	pd_unbind((t_pd *)x, gensym("#key"));
	pd_unbind((t_pd *)x, gensym("#keyname"));
	sys_vgui("selection clear .x%lx.c\n", x->x_canvas);
	sys_vgui(".x%lx.c focus {}\n", x->x_canvas);
	x->x_active = 0;
    }
    comment_update(x);
}

static void comment_select(t_gobj *z, t_glist *glist, int state)
{
    t_comment *x = (t_comment *)z;
    comment_validate(x, glist);
    if (!state && x->x_active) comment_activate(z, glist, 0);
    sys_vgui(".x%lx.c itemconfigure %s -fill %s\n", x->x_canvas,
	     x->x_texttag, (state ? "blue" : x->x_color));
    /* A regular rtext should now set 'canvas_editing' variable to its canvas,
       but we do not do that, because we get the keys through a global binding
       to "#key" (and because 'canvas_editing' is not exported). */
}

static void comment_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_comment *x = (t_comment *)z;
    t_text *t = (t_text *)z;
    comment_validate(x, glist);
    if (vis)
    {
	/* We do not need no rtext -- we are never 'textedfor' (thus
	   avoiding rtext calls).  Creating an rtext has no other purpose
	   than complying to a Pd's assumption about every visible object
	   having an rtext (thus preventing canvas_doclick() from sending
	   garbage warnings).  LATER revisit. */
#if FORKY_VERSION < 37
	rtext_new(glist, t, glist->gl_editor->e_rtext, 0);
#endif
	if (glist->gl_havewindow)
	    comment_draw(x);
    }
    else
    {
#if FORKY_VERSION < 37
	t_rtext *rt = glist_findrtext(glist, t);
	if (rt) rtext_free(rt);
#endif
	/* FIXME should we test for having a window? */
#ifdef COMMENT_DEBUG
	loudbug_post("deleting...");
#endif
	sys_vgui(".x%lx.c delete %s\n", x->x_canvas, x->x_tag);
    }
}

static void comment_save(t_gobj *z, t_binbuf *b)
{
    t_comment *x = (t_comment *)z;
    t_text *t = (t_text *)x;
    comment_validate(x, 0);
    binbuf_addv(b, "ssiisiissiiii", gensym("#X"), gensym("obj"),
		(int)t->te_xpix, (int)t->te_ypix,
        gensym("comment"),
		x->x_pixwidth, x->x_fontsize, x->x_fontfamily,
		(x->x_encoding ? x->x_encoding : gensym("?")),
		x->x_fontprops,
		(int)x->x_red, (int)x->x_green, (int)x->x_blue);
    binbuf_addbinbuf(b, t->te_binbuf);
    binbuf_addv(b, ";");
}

static t_widgetbehavior comment_widgetbehavior =
{
    comment_getrect,
    comment_displace,
    comment_select,
    comment_activate,
    0,
    comment_vis,
    0,
    FORKY_WIDGETPADDING
};

/* this fires if a transform request was sent to a symbol we are bound to */
static void comment_transtick(t_comment *x)
{
    glist_delete(x->x_glist, (t_gobj *)x);
}

/* what follows is basically the original code of rtext_key() */

static void comment_float(t_comment *x, t_float f)
{
    if (x->x_active)
    {
	int keynum = (int)f;
	if (keynum)
	{
	    int i, newsize, ndel;
	    char *s1, *s2;
	    int n = keynum;
	    if (n == '\r') n = '\n';
	    if (n == '\b')
	    {
		if ((!x->x_selstart) && (x->x_selend == x->x_textbufsize))
		{
	    	    /* LATER delete the box... this causes reentrancy
		       problems now. */
		    /* glist_delete(x->x_glist, &x->x_text->te_g); */
		    goto donefloat;
		}
		else if (x->x_selstart && (x->x_selstart == x->x_selend))
		    x->x_selstart--;
	    }
	    ndel = x->x_selend - x->x_selstart;
	    for (i = x->x_selend; i < x->x_textbufsize; i++)
		x->x_textbuf[i- ndel] = x->x_textbuf[i];
	    newsize = x->x_textbufsize - ndel;
	    x->x_textbuf = resizebytes(x->x_textbuf, x->x_textbufsize, newsize);
	    x->x_textbufsize = newsize;

	    if (n == '\n' || !iscntrl(n))
	    {
#ifdef COMMENT_DEBUG
		loudbug_post("%d accepted", n);
#endif
		newsize = x->x_textbufsize+1;
		x->x_textbuf = resizebytes(x->x_textbuf,
					   x->x_textbufsize, newsize);
		for (i = x->x_textbufsize; i > x->x_selstart; i--)
		    x->x_textbuf[i] = x->x_textbuf[i-1];
		x->x_textbuf[x->x_selstart] = n;
		x->x_textbufsize = newsize;
		x->x_selstart = x->x_selstart + 1;
	    }
#ifdef COMMENT_DEBUG
	    else loudbug_post("%d rejected", n);
#endif
	    x->x_selend = x->x_selstart;
	    x->x_glist->gl_editor->e_textdirty = 1;
	    binbuf_text(x->x_binbuf, x->x_textbuf, x->x_textbufsize);
	    comment_update(x);
	}
    }
    else loudbug_bug("comment_float");
 donefloat:;
#ifdef COMMENT_DEBUG
    loudbug_post("donefloat");
#endif
}

static void comment_list(t_comment *x, t_symbol *s, int ac, t_atom *av)
{
    if (!x->x_active)
	loudbug_bug("comment_list");
    else if (ac > 1 && av->a_type == A_FLOAT && (int)av->a_w.w_float
	     && av[1].a_type == A_SYMBOL)
    {
	t_symbol *keysym = av[1].a_w.w_symbol;
	if (!strcmp(keysym->s_name, "Right"))
	{
	    if (x->x_selend == x->x_selstart &&
		x->x_selstart < x->x_textbufsize)
		x->x_selend = x->x_selstart = x->x_selstart + 1;
	    else
		x->x_selstart = x->x_selend;
	}
	else if (!strcmp(keysym->s_name, "Left"))
	{
	    if (x->x_selend == x->x_selstart && x->x_selstart > 0)
		x->x_selend = x->x_selstart = x->x_selstart - 1;
	    else
		x->x_selend = x->x_selstart;
	}
    	/* this should be improved...  life's too short */
	else if (!strcmp(keysym->s_name, "Up"))
	{
	    if (x->x_selstart)
		x->x_selstart--;
	    while (x->x_selstart > 0 && x->x_textbuf[x->x_selstart] != '\n')
		x->x_selstart--;
	    x->x_selend = x->x_selstart;
	}
	else if (!strcmp(keysym->s_name, "Down"))
	{
	    while (x->x_selend < x->x_textbufsize &&
		   x->x_textbuf[x->x_selend] != '\n')
		x->x_selend++;
	    if (x->x_selend < x->x_textbufsize)
		x->x_selend++;
	    x->x_selstart = x->x_selend;
	}
	else if (!strcmp(keysym->s_name, "F4"))
	{
	    t_text *newt, *oldt = (t_text *)x;
	    t_binbuf *bb = binbuf_new();
	    int ac = binbuf_getnatom(x->x_binbuf);
	    binbuf_addv(bb, "siissiiii", gensym("comment"), x->x_pixwidth,
			x->x_fontsize, x->x_fontfamily,
			(x->x_encoding ? x->x_encoding : gensym("?")),
			x->x_fontprops,
			(int)x->x_red, (int)x->x_green, (int)x->x_blue);
	    binbuf_add(bb, ac, binbuf_getvec(x->x_binbuf));
	    canvas_setcurrent(x->x_glist);
	    newt = (t_text *)pd_new(makeshift_class);
	    newt->te_width = 0;
	    newt->te_type = T_OBJECT;
	    newt->te_binbuf = bb;
	    newt->te_xpix = oldt->te_xpix;
	    newt->te_ypix = oldt->te_ypix;
	    glist_add(x->x_glist, &newt->te_g);
	    glist_noselect(x->x_glist);
	    glist_select(x->x_glist, &newt->te_g);
	    gobj_activate(&newt->te_g, x->x_glist, 1);
	    x->x_glist->gl_editor->e_textdirty = 1;  /* force evaluation */
	    canvas_unsetcurrent(x->x_glist);
	    canvas_dirty(x->x_glist, 1);
	    clock_delay(x->x_transclock, 0);  /* LATER rethink */
	    goto donelist;
	}
	else if (!strcmp(keysym->s_name, "F5"))
	{
	    t_text *t = (t_text *)x;
	    t_binbuf *bb = binbuf_new();
	    int ac = binbuf_getnatom(x->x_binbuf);
	    binbuf_addv(bb, "ii", (int)t->te_xpix + 5, (int)t->te_ypix + 5);
	    binbuf_add(bb, ac, binbuf_getvec(x->x_binbuf));
	    canvas_setcurrent(x->x_glist);
	    typedmess((t_pd *)x->x_glist, gensym("text"),
		      ac + 2, binbuf_getvec(bb));
	    canvas_unsetcurrent(x->x_glist);
	    canvas_dirty(x->x_glist, 1);
	    binbuf_free(bb);
	    goto donelist;
	}
	else goto donelist;
	comment_update(x);
    }
 donelist:;
#ifdef COMMENT_DEBUG
    loudbug_post("donelist");
#endif
}

static void comment_free(t_comment *x)
{
    if (x->x_active)
    {
	loudbug_bug("comment_free");
	pd_unbind((t_pd *)x, gensym("#key"));
	pd_unbind((t_pd *)x, gensym("#keyname"));
    }
    if (x->x_transclock) clock_free(x->x_transclock);
    if (x->x_bindsym)
    {
	pd_unbind((t_pd *)x, x->x_bindsym);
	if (!x->x_bbpending)
	    pd_unbind(commentsink, x->x_bindsym);
    }
    if (x->x_binbuf && !x->x_ready) binbuf_free(x->x_binbuf);
    if (x->x_textbuf) freebytes(x->x_textbuf, x->x_textbufsize);
}

/* the arguments in the full form of a creation message are:

   width fontsize fontfamily encoding fontprops red green blue text...

   For comments typed into an object box, the text part begins with
   the first atom satisfying one of the following conditions (skipped
   arguments get default values):

   . having a different type than the corresponding argument of the
   full form

   . being preceded with a dot atom ('.') put in place of a symbol
   argument (fontfamily or encoding)

   . being the 10th atom in a box

   The question mark atom ('?') may be used to supply a default fontfamily
   or an empty encoding value.
*/

static void *comment_new(t_symbol *s, int ac, t_atom *av)
{
    t_comment *x = (t_comment *)pd_new(comment_class);
    t_text *t = (t_text *)x;
    t_atom at;
    char buf[32];
    t->te_type = T_TEXT;
    x->x_glist = canvas_getcurrent();
    x->x_canvas = 0;
    sprintf(x->x_tag, "all%x", (int)x);
    sprintf(x->x_texttag, "t%x", (int)x);
    sprintf(x->x_outlinetag, "h%x", (int)x);
    x->x_pixwidth = 0;
    x->x_fontsize = 0;
    x->x_fontfamily = 0;
    x->x_encoding = 0;
    x->x_fontprops = 0;
    x->x_red = 0;
    x->x_green = 0;
    x->x_blue = 0;

    if (ac && av->a_type == A_FLOAT)
    {
	x->x_pixwidth = (int)av->a_w.w_float;
	ac--; av++;
	if (ac && av->a_type == A_FLOAT)
	{
	    x->x_fontsize = (int)av->a_w.w_float;
	    ac--; av++;
	    if (ac && av->a_type == A_SYMBOL)
	    {
		if (av->a_w.w_symbol == gensym("."))
		{
		    ac--; av++;
		    goto textpart;
		}
		else if (av->a_w.w_symbol != gensym("?"))
		    x->x_fontfamily = av->a_w.w_symbol;
		ac--; av++;
		if (ac && av->a_type == A_SYMBOL)
		{
		    if (av->a_w.w_symbol == gensym("."))
		    {
			ac--; av++;
			goto textpart;
		    }
		    else if (av->a_w.w_symbol != gensym("?"))
			x->x_encoding = av->a_w.w_symbol;
		    ac--; av++;
		    if (ac && av->a_type == A_FLOAT)
		    {
			x->x_fontprops = (int)av->a_w.w_float;
			ac--; av++;
			if (ac && av->a_type == A_FLOAT)
			{
			    x->x_red = (unsigned char)av->a_w.w_float;
			    ac--; av++;
			    if (ac && av->a_type == A_FLOAT)
			    {
				x->x_green = (unsigned char)av->a_w.w_float;
				ac--; av++;
				if (ac && av->a_type == A_FLOAT)
				{
				    x->x_blue = (unsigned char)av->a_w.w_float;
				    ac--; av++;
				}
			    }
			}
		    }
		}
	    }
	}
    }
textpart:
    if (x->x_fontsize < 1)
	x->x_fontsize = glist_getfont(x->x_glist);
    if (!x->x_fontfamily)
	x->x_fontfamily = gensym("helvetica");
    sprintf(x->x_color, "#%2.2x%2.2x%2.2x", x->x_red, x->x_green, x->x_blue);

    x->x_binbuf = binbuf_new();
    if (ac) binbuf_restore(x->x_binbuf, ac, av);
    else
    {
	SETSYMBOL(&at, gensym("comment"));
	binbuf_restore(x->x_binbuf, 1, &at);
    }
    x->x_textbuf = 0;
    x->x_textbufsize = 0;
    x->x_transclock = clock_new(x, (t_method)comment_transtick);
    x->x_bbset = 0;
    x->x_bbpending = 0;
    sprintf(buf, "miXed%x", (int)x);
    x->x_bindsym = gensym(buf);
    pd_bind((t_pd *)x, x->x_bindsym);
    if (!commentsink)
	commentsink = pd_new(commentsink_class);
    pd_bind(commentsink, x->x_bindsym);
    x->x_ready = 0;
    x->x_dragon = 0;
    return (x);
}

void comment_setup(void)
{
    comment_class = class_new(gensym("comment"),
			      (t_newmethod)comment_new,
			      (t_method)comment_free,
			      sizeof(t_comment),
			      CLASS_NOINLET | CLASS_PATCHABLE,
			      A_GIMME, 0);
    class_addfloat(comment_class, comment_float);
    class_addlist(comment_class, comment_list);
    class_addmethod(comment_class, (t_method)comment__bboxhook,
		    gensym("_bbox"),
		    A_SYMBOL, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(comment_class, (t_method)comment__clickhook,
		    gensym("_click"), A_GIMME, 0);
    class_addmethod(comment_class, (t_method)comment__releasehook,
		    gensym("_release"), A_SYMBOL, 0);
    class_addmethod(comment_class, (t_method)comment__motionhook,
		    gensym("_motion"), A_SYMBOL, A_FLOAT, A_FLOAT, 0);
    class_setwidget(comment_class, &comment_widgetbehavior);
    forky_setsavefn(comment_class, comment_save);

    makeshift_class = class_new(gensym("text"), 0, 0,
				sizeof(t_text),
				CLASS_NOINLET | CLASS_PATCHABLE, 0);

    commentsink_class = class_new(gensym("_commentsink"), 0, 0,
				  sizeof(t_pd), CLASS_PD, 0);
    class_addanything(commentsink_class, commentsink_anything);
    class_addmethod(commentsink_class, (t_method)commentsink__bboxhook,
		    gensym("_bbox"),
		    A_SYMBOL, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);

    /* if older than 0.43, create an 0.43-style pdsend */
    sys_gui("if {[llength [info procs ::pdsend]] == 0} {");
    sys_gui("proc ::pdsend {args} {::pd \"[join $args { }] ;\"}}\n");

    sys_gui("proc comment_bbox {target cvname tag} {\n\
 pdsend \"$target _bbox $target [$cvname bbox $tag]\"}\n");

    /* LATER think about window vs canvas coords */
    sys_gui("proc comment_click {target cvname x y tag} {\n\
 pdsend \"$target _click $target [$cvname canvasx $x] [$cvname canvasy $y]\
 [$cvname index $tag @$x,$y] [$cvname bbox $tag]\"}\n");

    /* LATER think how to conditionally (FORKY_VERSION >= 38)
       replace puts with pdtk_post */
    sys_gui("proc comment_entext {enc tt} {\n\
 if {$enc == \"\"} {concat $tt} else {\n\
  set rr [catch {encoding convertfrom $enc $tt} tt1]\n\
  if {$rr == 0} {concat $tt1} else {\n\
  puts stderr [concat tcl/tk error: $tt1]\n\
  concat $tt}}}\n");

    sys_gui("proc comment_draw {tgt cv tag1 tag2 x y fnm fsz clr enc tt wd} {\n\
  set tt1 [comment_entext $enc $tt]\n\
  if {$wd > 0} {\n\
   $cv create text $x $y -text $tt1 -tags [list $tag1 $tag2] \
    -font [list $fnm $fsz] -fill $clr -width $wd -anchor nw} else {\n\
   $cv create text $x $y -text $tt1 -tags [list $tag1 $tag2] \
    -font [list $fnm $fsz] -fill $clr -anchor nw}\n\
  comment_bbox $tgt $cv $tag1\n\
  $cv bind $tag1 <Button> [list comment_click $tgt %W %x %y $tag1]}\n");

    sys_gui("proc comment_update {cv tag enc tt wd} {\n\
 set tt1 [comment_entext $enc $tt]\n\
 if {$wd > 0} {$cv itemconfig $tag -text $tt1 -width $wd} else {\n\
  $cv itemconfig $tag -text $tt1}}\n");
}
