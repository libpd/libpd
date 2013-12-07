/* Copyright (c) 2003-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <string.h>
#ifdef UNIX
#include <unistd.h>
#endif
#ifdef NT
#include <io.h>
#endif
#include "m_pd.h"
#include "common/loud.h"
#include "common/port.h"

#ifdef KRZYSZCZ
//#define DUMMIES_DEBUG
#endif

static t_class *ccdummies_class;
static int dummy_nclasses = 0;
static int pending_nclasses = 0;
static t_class **dummy_classes;
static int dummy_nreps = 0;

typedef struct _dummy_slot
{
    char        *s_name;  /* do not use after setup: invalid for mapped names */
    int          s_nins;
    int          s_nouts;
    int          s_warned;
    t_newmethod  s_method;  /* a specialized constructor */
} t_dummy_slot;

static t_object *dummy_newobject(t_symbol *s, t_dummy_slot **slotp);
static void dummy_io(t_object *x, int nins, int nouts);

static void *dummy_2dwave_tilde_new(t_symbol *s, int ac, t_atom *av)
{
    t_dummy_slot *sl;
    t_object *x = dummy_newobject(s, &sl);
    int nouts = 0;
    if (ac && av->a_type == A_SYMBOL)
    {
	if (ac > 3 && av[3].a_type == A_FLOAT)
	    nouts = (int)av[3].a_w.w_float;
    }
    else loud_classarg(*(t_pd *)x);
    if (nouts < 1)
	nouts = 1;
    dummy_io(x, sl->s_nins, nouts);
    return (x);
}

static void *dummy_adoutput_tilde_new(t_symbol *s, int ac, t_atom *av)
{
    t_dummy_slot *sl;
    t_object *x = dummy_newobject(s, &sl);
    int nouts = 0;
    while (ac--)
    {
	if (av++->a_type == A_FLOAT) nouts++;
	else
	{
	    loud_classarg(*(t_pd *)x);
	    break;
	}
    }
    if (nouts < 1)
	nouts = 2;
    dummy_io(x, sl->s_nins, nouts);
    return (x);
}

static void *dummy_fffb_tilde_new(t_symbol *s, int ac, t_atom *av)
{
    t_dummy_slot *sl;
    t_object *x = dummy_newobject(s, &sl);
    int nouts = 0;
    if (ac && av->a_type == A_FLOAT)
	nouts = (int)av->a_w.w_float;
    if (nouts < 1)
    {
	loud_classarg(*(t_pd *)x);
	nouts = 1;
    }
    dummy_io(x, sl->s_nins, nouts);
    return (x);
}

static void *dummy_gate_tilde_new(t_symbol *s, int ac, t_atom *av)
{
    t_dummy_slot *sl;
    t_object *x = dummy_newobject(s, &sl);
    int nouts = 0;
    if (ac && av->a_type == A_FLOAT)
	nouts = (int)av->a_w.w_float;
    if (nouts < 1)
	nouts = 1;
    dummy_io(x, sl->s_nins, nouts);
    return (x);
}

static void *dummy_groove_tilde_new(t_symbol *s, int ac, t_atom *av)
{
    t_dummy_slot *sl;
    t_object *x = dummy_newobject(s, &sl);
    int nouts = 0;
    if (ac && av->a_type == A_SYMBOL)
    {
	if (ac > 1 && av[1].a_type == A_FLOAT)
	    nouts = (int)av[1].a_w.w_float;
    }
    else loud_classarg(*(t_pd *)x);
    if (nouts < 1)
	nouts = 1;
    dummy_io(x, sl->s_nins, nouts + 1);
    return (x);
}

/* FIXME */
static void *dummy_if_new(t_symbol *s, int ac, t_atom *av)
{
    t_object *x = dummy_newobject(s, 0);
    int nins = 0;
    int nouts = 1;
    t_symbol *ps_out2 = gensym("out2");
    while (ac--)
    {
	if (av->a_type == A_SYMBOL)
	{
	    if (av->a_w.w_symbol == ps_out2)
		nouts = 2;
	    else
	    {
		char *name = av->a_w.w_symbol->s_name;
		if (strlen(name) >= 3 && *name == '$')
		{
		    char c = name[2];
		    if (c > '1' && c <= '9' && c > '0' + nins)
			nins = c - '0';
		}
	    }
	}
	av++;
    }
    dummy_io(x, nins, nouts);
    return (x);
}

#if 0
static void *dummy_matrix_tilde_new(t_symbol *s, int ac, t_atom *av)
{
    t_dummy_slot *sl;
    t_object *x = dummy_newobject(s, &sl);
    int nins = 0, nouts = 0;
    if (ac && av->a_type == A_FLOAT)
    {
	nins = (int)av->a_w.w_float;
	if (ac > 1 && av[1].a_type == A_FLOAT)
	    nouts = (int)av[1].a_w.w_float;
    }
    if (nins < 1 || nouts < 1)
    {
	loud_classarg(*(t_pd *)x);
	if (nins < 1) nins = 1;
	if (nouts < 1) nouts = 1;
    }
    dummy_io(x, nins, nouts + 1);  /* CHECKME */
    return (x);
}
#endif

static void *dummy_rewire_tilde_new(t_symbol *s, int ac, t_atom *av)
{
    t_dummy_slot *sl;
    t_object *x = dummy_newobject(s, &sl);
    int nouts = 0;
    if (ac && av->a_type == A_FLOAT)
	nouts = (int)av->a_w.w_float;
    else if (ac > 1 && av[1].a_type == A_FLOAT)
	nouts = (int)av[1].a_w.w_float;
    if (nouts < 1)
	nouts = 1;  /* CHECKME */
    dummy_io(x, sl->s_nins, nouts + 4);
    return (x);
}

static void *dummy_selector_tilde_new(t_symbol *s, int ac, t_atom *av)
{
    t_dummy_slot *sl;
    t_object *x = dummy_newobject(s, &sl);
    int nins = 0;
    if (ac && av->a_type == A_FLOAT)
	nins = (int)av->a_w.w_float;
    if (nins < 1)
	nins = 1;
    dummy_io(x, nins + 1, sl->s_nouts);
    return (x);
}

static void *dummy_sfplay_tilde_new(t_symbol *s, int ac, t_atom *av)
{
    t_dummy_slot *sl;
    t_object *x = dummy_newobject(s, &sl);
    int nouts = 0;
    if (ac)
    {
	if (av->a_type == A_FLOAT)
	    nouts = (int)av->a_w.w_float;
	else if (ac > 1 && av[1].a_type == A_FLOAT)
	    nouts = (int)av[1].a_w.w_float;
    }
    if (nouts < 1)
	nouts = 1;
    dummy_io(x, sl->s_nins, nouts + 1);
    return (x);
}

static void *dummy_sfrecord_tilde_new(t_symbol *s, int ac, t_atom *av)
{
    t_dummy_slot *sl;
    t_object *x = dummy_newobject(s, &sl);
    int nins = 0;
    if (ac && av->a_type == A_FLOAT)
	nins = (int)av->a_w.w_float;
    if (nins < 1)
	nins = 1;
    dummy_io(x, nins, sl->s_nouts);
    return (x);
}

static void *dummy_stutter_tilde_new(t_symbol *s, int ac, t_atom *av)
{
    t_dummy_slot *sl;
    t_object *x = dummy_newobject(s, &sl);
    int nios = 0;
    if (ac > 4 && av[4].a_type == A_FLOAT)
	nios = (int)av[4].a_w.w_float;
    if (nios < 1)
	nios = 1;
    dummy_io(x, nios + 2, nios);
    return (x);
}

static void *dummy_sxformat_new(t_symbol *s, int ac, t_atom *av)
{
    return (dummy_if_new(s, ac, av));  /* FIXME */
}

static void *dummy_tapout_tilde_new(t_symbol *s, int ac, t_atom *av)
{
    t_dummy_slot *sl;
    t_object *x = dummy_newobject(s, &sl);
    int warned = 0, nios = 0;
    while (ac--)
	if (av++->a_type == A_FLOAT)
	    nios++;
	else if (!warned++)
	    loud_classarg(*(t_pd *)x);
    if (nios < 1)
	nios = 1;
    dummy_io(x, nios, nios);
    return (x);
}

/* CHECKME */
static void *dummy_tiCmd_new(t_symbol *s, int ac, t_atom *av)
{
    t_dummy_slot *sl;
    t_object *x = dummy_newobject(s, &sl);
    int nouts = 0;
    if (ac > 1)
    {
	ac--; av++;
	while (ac--)
	{
	    char c = 0;
	    if (av->a_type == A_SYMBOL)
	    {
		c = *av->a_w.w_symbol->s_name;
		if (c == 'i' || c == 'f' || c == 'l'
		    || c == 'b' || c == 's' || c == 'a')
		    nouts++;
		else
		    c = 0;
	    }
	    if (c == 0)
	    {
		loud_classarg(*(t_pd *)x);
		break;
	    }
	    av++;
	}
    }
    if (nouts < 1)
	nouts = 0;
    dummy_io(x, sl->s_nins, nouts + 2);
    return (x);
}

/* CHECKME */
static void *dummy_timeline_new(t_symbol *s, int ac, t_atom *av)
{
    t_dummy_slot *sl;
    t_object *x = dummy_newobject(s, &sl);
    int nouts = 0;
    if (ac)
    {
	if (av->a_type == A_FLOAT)
	    nouts = (int)av->a_w.w_float;
	else if (ac > 1 && av[1].a_type == A_FLOAT)
	    nouts = (int)av[1].a_w.w_float;
    }
    if (nouts < 1)
	nouts = 0;
    dummy_io(x, sl->s_nins, nouts);
    return (x);
}

static void *dummy_vexpr_new(t_symbol *s, int ac, t_atom *av)
{
    return (dummy_if_new(s, ac, av));  /* FIXME */
}

static void *dummy_vst_tilde_new(t_symbol *s, int ac, t_atom *av)
{
    t_dummy_slot *sl;
    t_object *x = dummy_newobject(s, &sl);
    int nins = 0, nouts = 0;
    if (ac > 1 && av[1].a_type == A_FLOAT)
    {
	if (av->a_type == A_FLOAT)
	{
	    nins = (int)av->a_w.w_float;
	    nouts = (int)av[1].a_w.w_float;
	}
    }
    else if (ac && av->a_type == A_FLOAT)
	nouts = (int)av->a_w.w_float;
    if (nins < 1)
	nins = 2;  /* CHECKME */
    if (nouts < 1)
	nouts = 2;  /* CHECKME */
    dummy_io(x, nins, nouts + 4);  /* CHECKME */
    return (x);
}

static t_dummy_slot dummy_slots[] =
{
    { "2d.wave~",        4, -1, 0, (t_newmethod)dummy_2dwave_tilde_new },
    { "absolutepath",    1, 1, 0, 0 },
    { "acosh",           1, 1, 0, 0 },
    { "adoutput~",       1, -1, 0, (t_newmethod)dummy_adoutput_tilde_new },
    { "adstatus",        2, 2, 0, 0 },
    { "asinh",           1, 1, 0, 0 },
    { "atanh",           1, 1, 0, 0 },
    { "begin~",          0, 1, 0, 0 },
    /* LATER try mapping bpatcher to a gop abstraction/subpatch */
    { "buffer~",         1, 2, 0, 0 },
    { "cd",              1, 2, 0, 0 },  /* CHECKED (refman error?) */
    { "cd~",             1, 6, 0, 0 },  /* CHECKED (refman error?) */
    { "clocker",         2, 1, 0, 0 },
    { "closebang",       0, 1, 0, 0 },
    { "colorpicker",     1, 1, 0, 0 },
    { "date",            1, 3, 0, 0 },
    { "defer",           1, 1, 0, 0 },  /* LATER pass anything through */
    { "degrade~",        3, 1, 0, 0 },
    { "detonate",        8, 8, 0, 0 },
    { "dial",            1, 1, 0, 0 },
    { "dialog",          2, 1, 0, 0 },
    { "downsamp~",       2, 1, 0, 0 },
    { "dropfile",        1, 2, 0, 0 },
    { "dspstate~",       1, 3, 0, 0 },
    { "dsptime~",        1, 1, 0, 0 },
    { "env",             1, 1, 0, 0 },
    { "envi",            1, 1, 0, 0 },
    { "error",           1, 1, 0, 0 },
    { "ezadc~",          1, 2, 0, 0 },
    { "ezdac~",          2, 0, 0, 0 },
    { "fffb~",           1, -1, 0, (t_newmethod)dummy_fffb_tilde_new },
    /* LATER Fft~ */
    /* LATER pfft~-specific classes: fftin~, fftinfo~, fftout~ */
    { "filedate",        1, 1, 0, 0 },
    { "filein",          3, 3, 0, 0 },
    { "filepath",        1, 1, 0, 0 },
    { "filtergraph~",    8, 6, 0, 0 },
    { "folder",          1, 2, 0, 0 },
    { "follow",          1, 2, 0, 0 },
    { "fpic",            1, 0, 0, 0 },
    { "frame",           6, 0, 0, 0 },
    { "function",        1, 4, 0, 0 },
    { "gain~",           2, 2, 0, 0 },
    { "gate~",           2, -1, 0, (t_newmethod)dummy_gate_tilde_new },
    { "gestalt",         1, 2, 0, 0 },
    { "Ggate",           2, 2, 0, 0 },
    { "graphic",         1, 0, 0, 0 },
    { "groove~",         3, -1, 0, (t_newmethod)dummy_groove_tilde_new },
    { "Gswitch",         3, 1, 0, 0 },
    { "hint",            1, 1, 0, 0 },
    { "if",             -1, -1, 0, (t_newmethod)dummy_if_new },
    /* LATER Ifft~ */
    { "imovie",          1, 3, 0, 0 },
    { "IncDec",          1, 1, 0, 0 },
    { "info~",           1, 8, 0, 0 },  /* CHECKME nouts */
    { "ioscbank~",       4, 1, 0, 0 },
    { "kslider",         2, 2, 0, 0 },
    { "lcd",             1, 4, 0, 0 },  /* CHECKME nouts */
    { "led",             1, 1, 0, 0 },
    { "matrixctrl",      1, 1, 0, 0 },  /* CHECKME nins, nouts */
#if 0
    { "matrix~",        -1, -1, 0, (t_newmethod)dummy_matrix_tilde_new },
#endif
    { "menubar",         1, 4, 0, 0 },  /* LATER parse #Xs (additional outs) */
    { "meter~",          1, 1, 0, 0 },  /* LATER consider mapping to the vu */
    { "movie",           1, 3, 0, 0 },
    /* CHECKME msd */
    { "multiSlider",     1, 1, 0, 0 },
    { "mute~",           1, 1, 0, 0 },
    { "normalize~",      2, 1, 0, 0 },
    { "number~",         2, 2, 0, 0 },
    { "numkey",          1, 2, 0, 0 },
    { "omscontrollers",  4, 2, 0, 0 },  /* CHECKME osx */
    { "omsinfo",         2, 1, 0, 0 },  /* LATER midiinfo? */
    { "omsnotes",        4, 2, 0, 0 },  /* CHECKME osx */
    { "omspatches",      3, 2, 0, 0 },  /* CHECKME osx */
    { "onecopy",         0, 0, 0, 0 },  /* CHECKME */
    { "opendialog",      1, 2, 0, 0 },
    { "oscbank~",        4, 1, 0, 0 },
    { "oval",            6, 0, 0, 0 },
    { "overdrive~",      2, 1, 0, 0 },
    { "panel",           1, 0, 0, 0 },
    { "pass~",           1, 1, 0, 0 },
    { "pcontrol",        1, 1, 0, 0 },
    /* LATER pfft~ */
    { "phaseshift~",     3, 1, 0, 0 },
    { "pics",            3, 0, 0, 0 },  /* CHECKME */
    { "pics2",           3, 0, 0, 0 },  /* CHECKME */
    { "pict",            3, 0, 0, 0 },
    { "pictctrl",        1, 1, 0, 0 },
    { "pictslider",      2, 2, 0, 0 },  /* CHECKME one-dimensional mode */
    { "playbar",         1, 2, 0, 0 },  /* CHECKME */
    { "polyin",          1, 3, 0, 0 },  /* LATER parse args for nouts */
    { "polyout",         3, 0, 0, 0 },  /* CHECKME nins */
    /* LATER poly~ */
    { "preset",          1, 3, 0, 0 },
    { "radiogroup",      1, 1, 0, 0 },
    { "rate~",           2, 1, 0, 0 },  /* CHECKME */
    /* LATER settable Receive? */
    { "rect",            6, 0, 0, 0 },
    { "relativepath",    1, 1, 0, 0 },
    { "ring",            6, 0, 0, 0 },
    { "round~",          2, 1, 0, 0 },
    { "rslider",         2, 2, 0, 0 },
    { "rtin",            1, 1, 0, 0 },
    { "savedialog",      1, 3, 0, 0 },
    { "screensize",      1, 2, 0, 0 },
    { "selector~",      -1, 1, 0, (t_newmethod)dummy_selector_tilde_new },
    { "seq~",            1, 2, 0, 0 },
    { "serial",          1, 2, 0, 0 },
    { "setclock",        2, 1, 0, 0 },
    { "sfinfo~",         1, 6, 0, 0 },  /* CHECKME nouts */
    { "sflist~",         1, 0, 0, 0 },
    { "sfplay~",         1, -1, 0, (t_newmethod)dummy_sfplay_tilde_new },
    { "sfrecord~",      -1, 0, 0, (t_newmethod)dummy_sfrecord_tilde_new },
    { "strippath",       1, 2, 0, 0 },
    { "stutter~",       -1, -1, 0, (t_newmethod)dummy_stutter_tilde_new },
    { "suspend",         0, 1, 0, 0 },
    { "swatch",          3, 2, 0, 0 },
    { "sxformat",       -1, 1, 0, (t_newmethod)dummy_sxformat_new },
    { "sysexin",         1, 1, 0, 0 },
    { "tapin~",          1, 1, 0, 0 },
    { "tapout~",        -1, -1, 0, (t_newmethod)dummy_tapout_tilde_new },
    { "teeth~",          6, 1, 0, 0 },
    { "tempo",           4, 1, 0, 0 },
    { "Text",            1, 1, 0, 0 },
    { "textedit",        1, 3, 0, 0 },
    { "thisobject",      1, 3, 0, 0 },  /* CHECKME */
    { "thispatcher",     1, 2, 0, 0 },
    { "thisTimeline",    1, 1, 0, 0 },
    { "thisTrack",       1, 0, 0, 0 },
    { "thispoly~",       1, 1, 0, 0 },
    { "thresh~",         3, 1, 0, 0 },
    { "tiCmd",           0, -1, 0, (t_newmethod)dummy_tiCmd_new },
    { "timeline",        1, -1, 0, (t_newmethod)dummy_timeline_new },
    { "tiOut",           1, 0, 0, 0 },
    { "timein",          3, 4, 0, 0 },
    { "timeout",         4, 0, 0, 0 },
    /* LATER touchin's inlet (Touchin?) */
    { "trunc~",          1, 1, 0, 0 },  /* CHECKME */
    { "ubutton",         1, 4, 0, 0 },
    { "umenu",           1, 2, 0, 0 },
    { "vexpr",          -1, 1, 0, (t_newmethod)dummy_vexpr_new },
    { "vpicture",        0, 0, 0, 0 },
    { "waveform~",       5, 6, 0, 0 },  /* CHECKME */
    { "zigzag~",         2, 4, 0, 0 },

    /* mapped names (cf the structure `importmapping_default' in port.c) */
    /* clashing dummies go first */
    { "biquad~",         6, 1, 0, 0 },
    { "change",          1, 3, 0, 0 },
    { "key",             0, 3, 0, 0 },
    { "keyup",           0, 3, 0, 0 },
    { "line",            3, 2, 0, 0 },
    { "poly",            2, 4, 0, 0 },

    /* remaining slots define `doomed' kind of dummies */
    { "appledvd",        1, 2, 0, 0 },
    /* LATER glove? */
    { "plugconfig",      1, 0, 0, 0 },
    { "plugin~",         2, 2, 0, 0 },
    { "plugmidiin",      0, 1, 0, 0 },
    { "plugmidiout",     1, 0, 0, 0 },
    { "plugmod",         5, 3, 0, 0 },
    { "plugmorph",       2, 3, 0, 0 },
    { "plugmultiparam",  1, 2, 0, 0 },
    { "plugout~",        2, 2, 0, 0 },  /* CHECKME nouts */
    { "plugphasor~",     0, 1, 0, 0 },  /* CHECKME nouts */
    { "plugreceive~",    1, 1, 0, 0 },
    { "plugsend~",       1, 0, 0, 0 },
    { "plugstore",       1, 1, 0, 0 },  /* CHECKME nouts */
    { "plugsync~",       0, 9, 0, 0 },  /* CHECKME nouts */
    { "pp",              2, 2, 0, 0 },  /* CHECKME nins */
    { "pptempo",         2, 2, 0, 0 },
    { "pptime",          4, 4, 0, 0 },
    { "rewire~",         1, -1, 0, (t_newmethod)dummy_rewire_tilde_new },
    { "sndmgrin~",       0, 2, 0, 0 },  /* CHECKME */
    { "vdp",             3, 4, 0, 0 },
    { "vst~",           -1, -1, 0, (t_newmethod)dummy_vst_tilde_new },
    { "_dummy",          0, 0, 0, 0 }
};

static void *dummy_new(t_symbol *s, int ac, t_atom *av)
{
    t_dummy_slot *sl;
    t_object *x = dummy_newobject(s, &sl);
    dummy_io(x, sl->s_nins, sl->s_nouts);
    return (x);
}

static void dummy_io(t_object *x, int nins, int nouts)
{
    nins = (nins > 0 ? nins - 1 : 0);
    while (nins--) inlet_new(x, (t_pd *)x, 0, 0);
    while (nouts--) outlet_new(x, &s_anything);
}

static t_object *dummy_newobject(t_symbol *s, t_dummy_slot **slotp)
{
    t_object *x;
    t_dummy_slot *sl;
    int fnd;
    for (fnd = 0; fnd < dummy_nclasses; fnd++)
	/* LATER compare symbols, rather than strings */
	if (dummy_classes[fnd]  /* empty slot: abstraction replacement */
	    && !strcmp(class_getname(dummy_classes[fnd]), s->s_name))
	    break;
    x = (t_object *)pd_new(dummy_classes[fnd]);
    sl = &dummy_slots[fnd];
    if (fnd == dummy_nclasses)
	loudbug_bug("dummy_newobject");  /* create a "_dummy" in this case */
    else if (!sl->s_warned)
    {
	loud_warning((t_pd *)x, 0, "dummy substitution");
	sl->s_warned = 1;
    }
    if (slotp) *slotp = sl;
    return (x);
}

static void ccdummies_bang(t_pd *x)
{
    if (dummy_nreps)
    {
	char *msg = "replacement abstractions are: ";
	int i, len = strlen(msg);
	t_dummy_slot *sl;
	startpost(msg);
	for (i = 0, sl = dummy_slots; i < dummy_nclasses; i++, sl++)
	{
	    if (!dummy_classes[i])
	    {
		/* name field is valid here (reps are never mapped) */
		int l = 1 + strlen(sl->s_name);
		if ((len += l) > 66)
		{
		    endpost();
		    startpost("   ");
		    len = 3 + l;
		}
		poststring(sl->s_name);
	    }
	}
	endpost();
    }
    else post("no replacement abstractions");
}

void dummies_setup(void)
{
    t_dummy_slot *sl;
    int i, mapsize;
    char **mapping = import_getmapping(&mapsize);
    int ndoomed = 0;
    dummy_nclasses = sizeof(dummy_slots)/sizeof(*dummy_slots);
    /* never freed: */
    dummy_classes = getbytes(dummy_nclasses * sizeof(*dummy_classes));
    for (i = 0, sl = dummy_slots; i < dummy_nclasses; i++, sl++)
    {
	int fd;
	char dirbuf[MAXPDSTRING], *nameptr;
	char *name = port_usemapping(sl->s_name, mapsize, mapping);
	if (name)
	    ndoomed++;
	else if (ndoomed && i < dummy_nclasses - 1)
	{
	    loudbug_bug("dummies_setup");
	    loudbug_post
		("(\"%s\": clashing or doomed dummy not registered for import)",
		 sl->s_name);
	}
	if ((fd = open_via_path("", sl->s_name, ".pd",
				dirbuf, &nameptr, MAXPDSTRING, 0)) >= 0)
	{
	    close(fd);
	    dummy_nreps++;
	}
	else
	{
	    dummy_classes[i] =
		class_new((name ? gensym(name) : gensym(sl->s_name)),
			  sl->s_method ? sl->s_method : (t_newmethod)dummy_new,
			  0, sizeof(t_object),
			  (sl->s_nins ? 0 : CLASS_NOINLET), A_GIMME, 0);
	}
    }
    dummy_nclasses--;  /* use "_dummy" as a sentinel */

    ccdummies_class = class_new(gensym("_cc.dummies"), 0, 0,
				sizeof(t_pd), CLASS_PD | CLASS_NOINLET, 0);
    class_addbang(ccdummies_class, ccdummies_bang);
    pd_bind(pd_new(ccdummies_class), gensym("_cc.dummies"));  /* never freed */
}
