/* Copyright (c) 1997-2000 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* gfxstub stuff from pd-vanilla's x_gui.c */

/* dialogs.  LATER, deal with the situation where the object goes 
away before the panel does... */

#include "m_pd.h"
#include <stdio.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/* --------------------- graphics responder  ---------------- */

/* make one of these if you want to put up a dialog window but want to be
protected from getting deleted and then having the dialog call you back.  In
this design the calling object doesn't have to keep the address of the dialog
window around; instead we keep a list of all open dialogs.  Any object that
might have dialogs, when it is deleted, simply checks down the dialog window
list and breaks off any dialogs that might later have sent messages to it. 
Only when the dialog window itself closes do we delete the gfxstub object. */

static t_class *gfxstub_class;

typedef struct _gfxstub
{
    t_pd x_pd;
    t_pd *x_owner;
    void *x_key;
    t_symbol *x_sym;
    struct _gfxstub *x_next;
} t_gfxstub;

static t_gfxstub *gfxstub_list;

    /* create a new one.  the "key" is an address by which the owner
    will identify it later; if the owner only wants one dialog, this
    could just be a pointer to the owner itself.  The string "cmd"
    is a TK command to create the dialog, with "%s" embedded in
    it so we can provide a name by which the GUI can send us back
    messages; e.g., "pdtk_canvas_dofont %s 10". */

void gfxstub_new(t_pd *owner, void *key, const char *cmd)
{
    char buf[4*MAXPDSTRING];
    char namebuf[80];
    char sprintfbuf[MAXPDSTRING];
    char *afterpercent;
    t_int afterpercentlen;
    t_gfxstub *x;
    t_symbol *s;
        /* if any exists with matching key, burn it. */
    for (x = gfxstub_list; x; x = x->x_next)
        if (x->x_key == key)
            gfxstub_deleteforkey(key);
    if (strlen(cmd) + 50 > 4*MAXPDSTRING)
    {
        bug("audio dialog too long");
        bug("%s", cmd);
        return;
    }
    x = (t_gfxstub *)pd_new(gfxstub_class);
    sprintf(namebuf, ".gfxstub%lx", (t_int)x);

    s = gensym(namebuf);
    pd_bind(&x->x_pd, s);
    x->x_owner = owner;
    x->x_sym = s;
    x->x_key = key;
    x->x_next = gfxstub_list;
    gfxstub_list = x;
    /* only replace first %s so sprintf() doesn't crash */
    afterpercent = strchr(cmd, '%') + 2;
    afterpercentlen = afterpercent - cmd;
    strncpy(sprintfbuf, cmd, afterpercentlen);
    sprintfbuf[afterpercentlen] = '\0';
    sprintf(buf, sprintfbuf, s->s_name);
    strncat(buf, afterpercent, (4*MAXPDSTRING) - afterpercentlen);
    sys_gui(buf);
}

static void gfxstub_offlist(t_gfxstub *x)
{
    t_gfxstub *y1, *y2;
    if (gfxstub_list == x)
        gfxstub_list = x->x_next;
    else for (y1 = gfxstub_list; y2 = y1->x_next; y1 = y2)
        if (y2 == x) 
    {
        y1->x_next = y2->x_next;
        break;
    }
}

    /* if the owner disappears, we still may have to stay around until our
    dialog window signs off.  Anyway we can now tell the GUI to destroy the
    window.  */
void gfxstub_deleteforkey(void *key)
{
    t_gfxstub *y;
    int didit = 1;
    while (didit)
    {
        didit = 0;
        for (y = gfxstub_list; y; y = y->x_next)
        {
            if (y->x_key == key)
            {
                sys_vgui("destroy .gfxstub%lx\n", y);
                y->x_owner = 0;
                gfxstub_offlist(y);
                didit = 1;
                break;
            }
        }
    }
}

/* --------- pd messages for gfxstub (these come from the GUI) ---------- */

    /* "cancel" to request that we close the dialog window. */
static void gfxstub_cancel(t_gfxstub *x)
{
    gfxstub_deleteforkey(x->x_key);
}

    /* "signoff" comes from the GUI to say the dialog window closed. */
static void gfxstub_signoff(t_gfxstub *x)
{
    gfxstub_offlist(x);
    pd_free(&x->x_pd);
}

static t_binbuf *gfxstub_binbuf;

    /* a series of "data" messages rebuilds a scalar */
static void gfxstub_data(t_gfxstub *x, t_symbol *s, int argc, t_atom *argv)
{
    if (!gfxstub_binbuf)
        gfxstub_binbuf = binbuf_new();
    binbuf_add(gfxstub_binbuf, argc, argv);
    binbuf_addsemi(gfxstub_binbuf);
}
    /* the "end" message terminates rebuilding the scalar */
static void gfxstub_end(t_gfxstub *x)
{
    canvas_dataproperties((t_canvas *)x->x_owner,
        (t_scalar *)x->x_key, gfxstub_binbuf);
    binbuf_free(gfxstub_binbuf);
    gfxstub_binbuf = 0;
}

    /* anything else is a message from the dialog window to the owner;
    just forward it. */
static void gfxstub_anything(t_gfxstub *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_owner)
        pd_typedmess(x->x_owner, s, argc, argv);
}

static void gfxstub_free(t_gfxstub *x)
{
    pd_unbind(&x->x_pd, x->x_sym);
}

void gfxstub_setup(void)
{
    gfxstub_class = class_new(gensym("gfxstub"), 0, (t_method)gfxstub_free,
        sizeof(t_gfxstub), CLASS_PD, 0);
    class_addanything(gfxstub_class, gfxstub_anything);
    class_addmethod(gfxstub_class, (t_method)gfxstub_signoff,
        gensym("signoff"), 0);
    class_addmethod(gfxstub_class, (t_method)gfxstub_data,
        gensym("data"), A_GIMME, 0);
    class_addmethod(gfxstub_class, (t_method)gfxstub_end,
        gensym("end"), 0);
    class_addmethod(gfxstub_class, (t_method)gfxstub_cancel,
        gensym("cancel"), 0);
}
