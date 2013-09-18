/* Copyright (c) 1997-2005 Miller Puckette, krzYszcz, and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* LATER think about abstractions */
/* LATER sort out escaping rules (also revisit binport.c) */
/* LATER quoting */
/* LATER rethink inlet/inlet~ case */

#ifdef UNIX
#include <unistd.h>
#endif
#ifdef NT
#include <io.h>
#endif
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "m_pd.h"
#include "g_canvas.h"
#include "unstable/forky.h"
#include "unstable/fragile.h"
#include "unstable/fringe.h"
#include "common/loud.h"
#include "common/grow.h"
#include "common/binport.h"
#include "port.h"

#ifdef KRZYSZCZ
//#define PORT_DEBUG
#endif
#define PORT_LOG
#define PORT_DUMP  /* fill separate files with ignored data, e.g. pictures */

#define PORT_INISTACK  256  /* LATER rethink */
#define PORT_INISIZE   512  /* LATER rethink */

/* FIXME use messtree api */

enum { PORT_OK,    /* MESSTREE_CONTINUE */
       PORT_NEXT,  /* next line, please: MESSTREE_OK */
       PORT_UNKNOWN, PORT_CORRUPT, PORT_FATAL };

/* cf binport.c */
#define A_INT  A_DEFFLOAT

/* without access to sys_defaultfont, we just mimic defs from s_main.c */
#ifdef MSW
#define PORT_DEFFONTSIZE  12.
#else
#define PORT_DEFFONTSIZE  10.
#endif

#define PORT_XSTRETCH      1.25
#define PORT_YSTRETCH      1.1
#define PORT_WSTRETCH      1.25

typedef struct _port
{
    t_binbuf  *x_outbb;
    int        x_messcount;
    int        x_illmess;
    int        x_lastunexpected;
    int        x_lastbroken;
    int        x_lastinconsistent;
    int        x_nobj;
    int        x_withbogus;
    int        x_inatoms;
    t_atom    *x_inmess;
    int        x_outsize;
    int        x_outatoms;
    t_atom    *x_outmess;
    t_atom     x_outini[PORT_INISIZE];
    int        x_stacksize;
    int        x_stackdepth;
    int       *x_stack;
    int        x_stackini[PORT_INISTACK];
    t_symbol  *x_emstate;
    t_binbuf  *x_embb;
    t_symbol  *x_emname;
    int        x_emsize;
    int        x_emcount;
    int        x_dumping;
    /* class-specifics, LATER find a better way */
    FILE      *x_pictfp;
    int        x_pictno;
} t_port;

static t_symbol *portps_bogus;
static t_symbol *portps_cleanup;
static t_symbol *portps_inlet;
static t_symbol *portps_outlet;
static t_symbol *portps_vtable;
static t_symbol *portps_coll;
static t_symbol *portps_funbuff;
static t_symbol *portps_prob;
static t_symbol *portps_picture;

static char *import_defmapping[] =
{
    /* clashing clones */
    "append", "Append",
    "b", "bangbang",
    "clip", "Clip",
    "clip~", "Clip~",
    "line~", "Line~",
    "scope~", "Scope~",
    "snapshot~", "Snapshot~",

    /* clashing dummies */
    "biquad~", "Biquad~",
    "change", "Change",
    "key", "Key",
    "keyup", "Keyup",
    "line", "Line",
    "poly", "Poly",

    /* doomed dummies */
    "appledvd", "c74.appledvd",
    "plugconfig", "c74.plugconfig",
    "plugin~", "c74.plugin~",
    "plugmidiin", "c74.plugmidiin",
    "plugmidiout", "c74.plugmidiout",
    "plugmod", "c74.plugmod",
    "plugmorph", "c74.plugmorph",
    "plugmultiparam", "c74.plugmultiparam",
    "plugout~", "c74.plugout~",
    "plugphasor~", "c74.plugphasor~",
    "plugreceive~", "c74.plugreceive~",
    "plugsend~", "c74.plugsend~",
    "plugstore", "c74.plugstore",
    "plugsync~", "c74.plugsync~",
    "pp", "c74.pp",
    "pptempo", "c74.pptempo",
    "pptime", "c74.pptime",
    "rewire~", "c74.rewire~",
    "sndmgrin~", "c74.sndmgrin~",
    "vdp", "c74.vdp",
    "vst~", "c74.vst~"
};

static int import_mapsize = 0;
static char **import_mapping = 0;

static void import_setdefmapping(void)
{
    import_mapsize = sizeof(import_defmapping)/(2 * sizeof(*import_defmapping));
    import_mapping = import_defmapping;
}

void import_setmapping(int size, char **mapping)
{
    import_mapsize = size;
    import_mapping = mapping;
}

char **import_getmapping(int *sizep)
{
    if (!import_mapping) import_setdefmapping();
    *sizep = import_mapsize;
    return (import_mapping);
}

char *port_usemapping(char *from, int mapsize, char **mapping)
{
    while (mapsize--)
    {
	if (strcmp(*mapping, from))
	    mapping += 2;
	else
	    return (mapping[1]);
    }
    return (0);
}

static t_int port_getint(t_port *x, int ndx)
{
    if (ndx < x->x_inatoms)
    {
	t_atom *av = &x->x_inmess[ndx];
	if (av->a_type == A_INT)
	    return (av->a_w.w_index);
	else if (av->a_type == A_FLOAT)
	{
	    loud_warning(0, "import", "[%d] float atom %d, int expected",
			 x->x_messcount, ndx);
	    return ((t_int)av->a_w.w_float);
	}
    }
    return (0);
}

static t_float port_getfloat(t_port *x, int ndx)
{
    if (ndx < x->x_inatoms)
    {
	t_atom *av = &x->x_inmess[ndx];
	return (av->a_type == A_FLOAT ? av->a_w.w_float : 0);
    }
    else return (0);
}

static t_symbol *port_getsymbol(t_port *x, int ndx)
{
    if (ndx < x->x_inatoms)
    {
	t_atom *av = &x->x_inmess[ndx];
	return (av->a_type == A_SYMBOL ? av->a_w.w_symbol : &s_);
    }
    else return (&s_);
}

static t_symbol *port_getanysymbol(t_port *x, int ndx)
{
    t_symbol *sel = &s_;
    if (ndx < x->x_inatoms)
    {
	t_atom *av = &x->x_inmess[ndx];
	if (av->a_type == A_SYMBOL)
	    sel = av->a_w.w_symbol;
	else if (av->a_type == A_INT)
	    sel = gensym("int");
	else if (av->a_type == A_FLOAT)
	    sel = &s_float;
    }
    return (sel);
}

static t_symbol *port_gettarget(t_port *x)
{
    t_symbol *sel = port_getsymbol(x, 0);
    if (sel == &s_) loudbug_bug("port_gettarget");
    return (sel);
}

static t_symbol *port_getselector(t_port *x)
{
    t_symbol *sel = port_getanysymbol(x, 1);
    if (sel == &s_) loudbug_bug("port_getselector");
    return (sel);
}

static int port_xstretch(float f)
{
    return ((int)(f * PORT_XSTRETCH + 0.5));
}

static int port_ystretch(float f)
{
    return ((int)(f * PORT_YSTRETCH + 0.5));
}

static int port_wstretch(float f)
{
    return ((int)(f * PORT_WSTRETCH + 0.5));
}

static t_float port_getx(t_port *x, int ndx)
{
    return ((t_float)port_xstretch(port_getint(x, ndx)));
}

static t_float port_gety(t_port *x, int ndx)
{
    return ((t_float)port_ystretch(port_getint(x, ndx)));
}

static t_float port_getwidth(t_port *x, int ndx)
{
    return ((t_float)port_wstretch(port_getint(x, ndx)));
}

static void port_setxy(t_port *x, int ndx, t_atom *ap)
{
    float f = port_getx(x, ndx);
    SETFLOAT(ap, f);
    ndx++; ap++;
    f = port_gety(x, ndx);
    SETFLOAT(ap, f);
}

static t_atom *import_copyatoms(t_atom *out, t_atom *in, int ac)
{
    while (ac-- > 0)
    {
	if (in->a_type == A_INT)
	{
	    out->a_type = A_FLOAT;
	    out++->a_w.w_float = (float)in++->a_w.w_index;
	}
	else *out++ = *in++;
    }
    return (out);
}

static void import_unexpected(t_port *x)
{
    if (x->x_lastunexpected < x->x_messcount)  /* ignore redundant calls */
    {
	x->x_lastunexpected = x->x_messcount;
	loud_warning(0, "import", "[%d] unexpected \"%s %s\"", x->x_messcount,
		     port_gettarget(x)->s_name, port_getselector(x)->s_name);
    }
}

static void import_illegal(t_port *x)
{
    x->x_illmess++;
}

static void import_flushillegal(t_port *x)
{
    if (x->x_illmess)
    {
	if (x->x_illmess == 1)
	    loud_warning(0, "import", "[%d] illegal line", x->x_messcount);
	else
	    loud_warning(0, "import", "[%d] %d illegal lines",
			 x->x_messcount, x->x_illmess);
	x->x_illmess = 0;
    }
}

static void import_embroken(t_port *x, char *cause)
{
    if (x->x_lastbroken < x->x_messcount)  /* ignore redundant calls */
    {
	x->x_lastbroken = x->x_messcount;
	loud_warning(0, "import", "[%d] %s embedding broken by %s",
		     x->x_messcount, x->x_emstate->s_name, cause);
    }
}

static int import_emcheck(t_port *x, t_symbol *state)
{
    if (x->x_emstate == state)
	return (1);
    else if (x->x_emstate)
	import_embroken(x, state->s_name);
    else
	import_unexpected(x);
    return (0);
}

static void import_eminconsistent(t_port *x, t_symbol *state)
{
    if (import_emcheck(x, state) &&
	x->x_lastinconsistent < x->x_messcount)  /* ignore redundant calls */
    {
	x->x_lastinconsistent = x->x_messcount;
	loud_warning(0, "import", "[%d] %s embedding ended inconsistently",
		     x->x_messcount, state->s_name);
    }
}

static int import_emcheckend(t_port *x, t_symbol *state, t_symbol *name)
{
    if (import_emcheck(x, state))
    {
	if (x->x_emcount  /* empty ok for vtable, CHECKME other cases */
	    && x->x_emsize != x->x_emcount)
	    loud_warning(0, "import",
			 "[%d] corrupt %s (%d atoms declared, %d provided)",
			 x->x_messcount, state->s_name,
			 x->x_emsize, x->x_emcount);
	else
	{
	    if (name != x->x_emname)  /* warn and accept, LATER rethink */
		import_eminconsistent(x, state);
	    return (1);
	}
    }
    return (0);
}

static void import_emstart(t_port *x, t_symbol *state, t_symbol *name, int size)
{
    if (x->x_emstate) import_embroken(x, state->s_name);
    x->x_emstate = state;
    binbuf_clear(x->x_embb);
    x->x_emname = name;
    x->x_emsize = size;
    x->x_emcount = 0;
}

static void import_emend(t_port *x, t_symbol *state, t_symbol *name)
{
    import_emcheckend(x, state, name);
    x->x_emstate = 0;
    x->x_emname = 0;
    x->x_emsize = 0;
    x->x_emcount = 0;
    binbuf_clear(x->x_embb);
}

static void import_emflush(t_port *x, t_symbol *state, t_symbol *name)
{
    int ac = binbuf_getnatom(x->x_embb);
    if (import_emcheckend(x, state, name) && ac)
	binbuf_add(x->x_outbb, ac, binbuf_getvec(x->x_embb));
    x->x_emstate = 0;
    x->x_emname = 0;
    x->x_emsize = 0;
    x->x_emcount = 0;
    if (ac) binbuf_clear(x->x_embb);
    binbuf_addv(x->x_outbb, "ss;", gensym("#C"), gensym("restore"));
}

static int import_emcopy(t_port *x, t_symbol *state)
{
    if (import_emcheck(x, state))
    {
	t_atom *out = x->x_outmess;
	SETSYMBOL(out, gensym("#C")); out++;
	out = import_copyatoms(out, x->x_inmess + 1, x->x_inatoms - 1);
	SETSEMI(out);
	binbuf_add(x->x_embb, x->x_inatoms + 1, x->x_outmess);
	return (1);
    }
    else return (0);
}

static int import_emadd(t_port *x, t_symbol *state, int ac, t_atom *av)
{
    if (import_emcheck(x, state))
    {
	t_atom at;
	SETSYMBOL(&at, gensym("#C"));
	binbuf_add(x->x_embb, 1, &at);
	binbuf_add(x->x_embb, ac, av);
	binbuf_addsemi(x->x_embb);
	return (1);
    }
    else return (0);
}

static int import_emaddv(t_port *x, t_symbol *state, char *fmt, ...)
{
    va_list ap;
    t_atom arg[64], *at = arg;
    int nargs = 0;
    char *fp = fmt;
    va_start(ap, fmt);
    SETSYMBOL(at, gensym("#C"));
    at++; nargs++;
    if (import_emcheck(x, state)) while (1)
    {
	switch(*fp++)
	{
	case 'i': SETFLOAT(at, va_arg(ap, t_int)); break;
	case 'f': SETFLOAT(at, va_arg(ap, double)); break;
	case 's': SETSYMBOL(at, va_arg(ap, t_symbol *)); break;
	case ';': SETSEMI(at); break;
	case 0: goto done;
	default: nargs = 0; goto done;
	}
	at++; nargs++;
    }
done:
    va_end(ap);
    if (nargs > 1)
    {
	binbuf_add(x->x_embb, nargs, arg);
	return (1);
    }
    else return (0);
}

static void import_addclassname(t_port *x, char *outname, t_atom *inatom)
{
    t_atom at;
    if (outname)
	SETSYMBOL(&at, gensym(outname));
    else
    {
	t_symbol *insym = 0;
	if (inatom->a_type == A_SYMBOL)
	{
	    /* LATER bash inatom to lowercase (CHECKME first) */
	    insym = inatom->a_w.w_symbol;
	    if (import_mapping && import_mapsize)
	    {
		char **fromp = import_mapping, **top = import_mapping + 1;
		int cnt = import_mapsize;
		while (cnt--)
		{
		    if (strcmp(*fromp, insym->s_name))
		    {
			fromp += 2;
			top += 2;
		    }
		    else
		    {
			insym = gensym(*top);
			inatom = 0;
			break;
		    }
		}
	    }
	    if (insym != &s_bang && insym != &s_float &&
		insym != &s_symbol && insym != &s_list &&
		(insym == portps_inlet || insym == portps_outlet ||
		 zgetfn(&pd_objectmaker, insym) == 0))
	    {
		x->x_withbogus = 1;
		SETSYMBOL(&at, portps_bogus);
		binbuf_add(x->x_outbb, 1, &at);
	    }
	}
	if (inatom)
	    import_copyatoms(&at, inatom, 1);
	else if (insym)
	    SETSYMBOL(&at, insym);
	else
	{
	    loudbug_bug("import_addclassname");
	    SETSYMBOL(&at, gensym("???"));
	}
    }
    binbuf_add(x->x_outbb, 1, &at);
}

static int import_obj(t_port *x, char *name)
{
    int ndx = (x->x_inmess[1].a_w.w_symbol == gensym("user") ? 3 : 2);
    binbuf_addv(x->x_outbb, "ssff",
		gensym("#X"), gensym("obj"),
		port_getx(x, ndx), port_gety(x, ndx + 1));
    import_addclassname(x, name, &x->x_inmess[ndx == 2 ? 6 : 2]);
    binbuf_addsemi(x->x_outbb);
    x->x_nobj++;
    return (PORT_NEXT);
}

static int import_objarg(t_port *x, char *name)
{
    int ndx = (x->x_inmess[1].a_w.w_symbol == gensym("user") ? 3 : 2);
    if (x->x_inatoms > 6
	|| (ndx == 3 && x->x_inatoms > 4))
    {
	t_atom *out = x->x_outmess;
	SETSYMBOL(out, gensym("#X")); out++;
	SETSYMBOL(out, gensym("obj")); out++;
	port_setxy(x, ndx, out);
	binbuf_add(x->x_outbb, 4, x->x_outmess);
	import_addclassname(x, name, &x->x_inmess[ndx == 2 ? 6 : 2]);
	out = import_copyatoms(x->x_outmess, x->x_inmess + 7, x->x_inatoms - 7);
	SETSEMI(out);
	binbuf_add(x->x_outbb, x->x_inatoms - 6, x->x_outmess);
	x->x_nobj++;
	return (PORT_NEXT);
    }
    else return (PORT_CORRUPT);
}

static int imaction_N1_vpatcher(t_port *x, char *arg)
{
    if (x->x_stackdepth >= x->x_stacksize)
    {
	int rqsz = x->x_stackdepth + 1;
	int sz = rqsz;
	x->x_stack = grow_withdata(&rqsz, &x->x_stackdepth,
				   &x->x_stacksize, x->x_stack,
				   PORT_INISTACK, x->x_stackini,
				   sizeof(*x->x_stack));
	if (rqsz != sz)
	{
	    post("too many embedded patches");
	    return (PORT_FATAL);
	}
    }
    x->x_stack[x->x_stackdepth++] = x->x_nobj;
    x->x_nobj = 0;
    binbuf_addv(x->x_outbb, "ssfffff;",
		gensym("#N"), gensym("canvas"),
		port_getx(x, 2), port_gety(x, 3),
		(float)port_xstretch(port_getint(x, 4) - port_getint(x, 2)),
		(float)port_ystretch(port_getint(x, 5) - port_getint(x, 3)),
		PORT_DEFFONTSIZE);
    return (PORT_NEXT);
}

static int imaction_N1_vtable(t_port *x, char *arg)
{
    int range = port_getint(x, 8),
	left = port_getint(x, 3),
	top = port_getint(x, 4),
	right = port_getint(x, 5),
	bottom = port_getint(x, 6),
	flags = port_getint(x, 7);
    import_emstart(x, portps_vtable, port_getsymbol(x, 9), port_getint(x, 2));
#ifdef PORT_DEBUG
    loudbug_post(
	"vtable \"%s\": size %d, range %d, coords %d %d %d %d, flags %d",
	x->x_emname->s_name, x->x_emsize,
	range, left, top, right, bottom, flags);
#endif
    import_emaddv(x, portps_vtable, "si;", gensym("size"), x->x_emsize);
    import_emaddv(x, portps_vtable, "siiii;", gensym("flags"),
		  /* CHECKED */
		  (flags & 16) != 0, (flags & 4) != 0,
		  (flags & 8) != 0, (flags & 2) != 0);
    import_emaddv(x, portps_vtable, "si;", gensym("tabrange"), range);
    import_emaddv(x, portps_vtable, "siiiii;", gensym("_coords"),
		  left, top, right, bottom, flags & 1);
    return (PORT_NEXT);
}

static int imaction_N1_coll(t_port *x, char *arg)
{
    import_emstart(x, portps_coll, port_getsymbol(x, 2), 0);
    return (PORT_NEXT);
}

static int imaction_N1_funbuff(t_port *x, char *arg)
{
    import_emstart(x, portps_funbuff, &s_, 0);
    import_emaddv(x, portps_funbuff, "si;", gensym("embed"),
		  port_getint(x, 2) != 0);
    return (PORT_NEXT);
}

static int imaction_N1_prob(t_port *x, char *arg)
{
    import_emstart(x, portps_prob, &s_, 0);
    return (PORT_NEXT);
}

static int imaction_N1_picture(t_port *x, char *arg)
{
    import_emstart(x, portps_picture, 0, 0);
    if (x->x_pictfp)
    {
	import_unexpected(x);
	if (x->x_dumping)
	    fclose(x->x_pictfp);
	x->x_pictfp = 0;
    }
    return (PORT_NEXT);
}

static int imaction_P6_patcher(t_port *x, char *arg)
{
    if (x->x_withbogus)
	binbuf_addv(x->x_outbb, "ss;", portps_cleanup, portps_cleanup);
    binbuf_addv(x->x_outbb, "ssffss;",
		gensym("#X"), gensym("restore"),
		port_getx(x, 2), port_gety(x, 3),
		gensym("pd"), port_getsymbol(x, 7));
    if (x->x_stackdepth)  /* LATER consider returning PORT_FATAL otherwise */
	x->x_stackdepth--;
    x->x_nobj = x->x_stack[x->x_stackdepth];
    x->x_nobj++;
    return (PORT_NEXT);
}

static int imaction_P6_table(t_port *x, char *arg)
{
    t_symbol *tablename = port_getsymbol(x, 7);
    binbuf_addv(x->x_outbb, "ssffs",
		gensym("#X"), gensym("obj"),
		port_getx(x, 2), port_gety(x, 3), gensym("Table"));
    if (tablename != &s_)
    {
	t_atom at;
	SETSYMBOL(&at, tablename);
	binbuf_add(x->x_outbb, 1, &at);
    }
    binbuf_addsemi(x->x_outbb);
    import_emflush(x, portps_vtable, tablename);
    x->x_nobj++;
    return (PORT_NEXT);
}

static int imaction_P6_coll(t_port *x, char *arg)
{
    t_symbol *collname = port_getsymbol(x, 7);
    binbuf_addv(x->x_outbb, "ssffs",
		gensym("#X"), gensym("obj"),
		port_getx(x, 2), port_gety(x, 3), portps_coll);
    if (collname != &s_)
    {
	t_atom at;
	SETSYMBOL(&at, collname);
	binbuf_add(x->x_outbb, 1, &at);
    }
    binbuf_addsemi(x->x_outbb);
    import_emflush(x, portps_coll, collname);
    x->x_nobj++;
    return (PORT_NEXT);
}

static int imaction_P6_funbuff(t_port *x, char *arg)
{
    binbuf_addv(x->x_outbb, "ssffs;",
		gensym("#X"), gensym("obj"),
		port_getx(x, 2), port_gety(x, 3), portps_funbuff);
    import_emflush(x, portps_funbuff, &s_);
    x->x_nobj++;
    return (PORT_NEXT);
}

static int imaction_P6_prob(t_port *x, char *arg)
{
    binbuf_addv(x->x_outbb, "ssffs;",
		gensym("#X"), gensym("obj"),
		port_getx(x, 2), port_gety(x, 3), portps_prob);
    import_emflush(x, portps_prob, &s_);
    x->x_nobj++;
    return (PORT_NEXT);
}

/* LATER use hammer replacements */
static int imaction_P6_pack(t_port *x, char *arg)
{
    int i;
    for (i = 7; i < x->x_inatoms; i++)
    {
	if (x->x_inmess[i].a_type == A_SYMBOL)
	{
	    t_symbol *s = x->x_inmess[i].a_w.w_symbol;
	    if (s->s_name[1])
	    {
		loud_warning(0, "import",
			     "%s's argument '%s' bashed to 's'",
			     port_getsymbol(x, 6)->s_name, s->s_name);
		x->x_inmess[i].a_w.w_symbol = gensym("s");
	    }
	    else switch (*s->s_name)
	    {
	    case 'b': case 'f': case 's': case 'l':
		break;
	    case 'i':
		x->x_inmess[i].a_w.w_symbol = gensym("f");
		break;
	    default:
		x->x_inmess[i].a_w.w_symbol = gensym("s");
	    }
	}
    }
    return (PORT_OK);
}

/* LATER consider using hammer replacements */
static int imaction_P6_midi(t_port *x, char *arg)
{
    x->x_inatoms = 7;  /* ugly, LATER rethink */
    return (PORT_OK);
}

static int imaction_P2_scope(t_port *x, char *name)
{
    if (x->x_inatoms > 6)
    {
	t_atom *out = x->x_outmess;
	int i, xpix, ypix;
	SETSYMBOL(out, gensym("#X")); out++;
	SETSYMBOL(out, gensym("obj")); out++;
	port_setxy(x, 3, out);
	xpix = (int)out++->a_w.w_float;
	ypix = (int)out->a_w.w_float;
	binbuf_add(x->x_outbb, 4, x->x_outmess);
	import_addclassname(x, name, &x->x_inmess[2]);
	out = x->x_outmess;
	port_setxy(x, 5, out);
	out++->a_w.w_float -= xpix;
	out++->a_w.w_float -= ypix;
	out = import_copyatoms(out, x->x_inmess + 7, x->x_inatoms - 7);
	SETSEMI(out);
	binbuf_add(x->x_outbb, x->x_inatoms - 4, x->x_outmess);
	x->x_nobj++;
	return (PORT_NEXT);
    }
    else return (PORT_CORRUPT);
}

/* width fontsize fontfamily encoding fontprops red green blue text... */
static int imaction_P1_comment(t_port *x, char *arg)
{
    int outatoms;
    SETSYMBOL(x->x_outmess, gensym("#X"));
    SETSYMBOL(x->x_outmess + 1, gensym("obj"));
    port_setxy(x, 2, x->x_outmess + 2);
    SETSYMBOL(x->x_outmess + 4, gensym("comment"));
    if (x->x_inatoms > 5)
    {
	int i, fontsize, fontprops;
	float width = port_getwidth(x, 4);
	t_atom *ap = x->x_inmess + 5;
	SETFLOAT(x->x_outmess + 5, width);
	if (ap->a_type == A_INT)
	{
	    fontsize = ap->a_w.w_index & 0x0ff;
	    fontprops = ap->a_w.w_index >> 8;
	}
	else if (ap->a_type == A_FLOAT)  /* FIXME */
	{
	    fontsize = ((int)ap->a_w.w_float) & 0x0ff;
	    fontprops = ((int)ap->a_w.w_float) >> 8;
	}
	else fontsize = 10, fontprops = 0;
	SETFLOAT(x->x_outmess + 6, fontsize);
	SETSYMBOL(x->x_outmess + 7, gensym("helvetica"));
	SETSYMBOL(x->x_outmess + 8, gensym("?"));
	SETFLOAT(x->x_outmess + 9, fontprops);
	SETFLOAT(x->x_outmess + 10, 0);
	SETFLOAT(x->x_outmess + 11, 0);
	SETFLOAT(x->x_outmess + 12, 0);
	outatoms = x->x_inatoms + 7;
	import_copyatoms(x->x_outmess + 13, x->x_inmess + 6, x->x_inatoms - 6);
    }
    else outatoms = 5;
    SETSEMI(x->x_outmess + outatoms);
    binbuf_add(x->x_outbb, outatoms + 1, x->x_outmess);
    x->x_nobj++;
    return (PORT_NEXT);
}

static int imaction_P1_message(t_port *x, char *arg)
{
    int i;
    t_atom *out;
    SETSYMBOL(x->x_outmess, gensym("#X"));
    SETSYMBOL(x->x_outmess + 1, gensym("msg"));
    port_setxy(x, 2, x->x_outmess + 2);
    out = import_copyatoms(x->x_outmess + 4, x->x_inmess + 6, x->x_inatoms - 6);
    SETSEMI(out);
    binbuf_add(x->x_outbb, x->x_inatoms - 1, x->x_outmess);
    x->x_nobj++;
    return (PORT_NEXT);
}

static int imaction_P1_io(t_port *x, char *arg)
{
    binbuf_addv(x->x_outbb, "ssff",
		gensym("#X"), gensym("obj"),
		port_getx(x, 2), port_gety(x, 3));
    if (x->x_inmess[1].a_w.w_symbol == portps_inlet ||
	x->x_inmess[1].a_w.w_symbol == portps_outlet)
    {
	t_atom at;
	SETSYMBOL(&at, portps_bogus);
	binbuf_add(x->x_outbb, 1, &at);
    }
    binbuf_add(x->x_outbb, 1, &x->x_inmess[1]);
    binbuf_addsemi(x->x_outbb);
    x->x_nobj++;
    return (PORT_NEXT);
}

static int imaction_P1_number(t_port *x, char *arg)
{
    binbuf_addv(x->x_outbb, "ssff;",
		gensym("#X"), gensym("floatatom"),
		port_getx(x, 2), port_gety(x, 3));
    x->x_nobj++;
    return (PORT_NEXT);
}

static int imaction_P1_vpicture(t_port *x, char *arg)
{
    import_emend(x, portps_picture, 0);
    if (x->x_pictfp)
    {
	if (x->x_dumping)
	    fclose(x->x_pictfp);
	x->x_pictfp = 0;
    }
    else import_unexpected(x);
    binbuf_addv(x->x_outbb, "ssffs;",
		gensym("#X"), gensym("obj"),
		port_getx(x, 2), port_gety(x, 3),
		gensym("vpicture"));
    x->x_nobj++;
    return (PORT_NEXT);
}

static int imaction_P1_connect(t_port *x, char *arg)
{
    binbuf_addv(x->x_outbb, "ssiiii;",
		gensym("#X"), gensym("connect"),
		x->x_nobj - port_getint(x, 2) - 1,
		port_getint(x, 3),
		x->x_nobj - port_getint(x, 4) - 1,
		port_getint(x, 5));
    return (PORT_NEXT);
}

static int imaction_T1_int(t_port *x, char *arg)
{
    if (x->x_emstate == portps_coll)
	import_emcopy(x, portps_coll);
    else if (x->x_emstate == portps_prob)
	import_emcopy(x, portps_prob);
    else
	import_unexpected(x);
    return (PORT_NEXT);
}

static int imaction_T1_flags(t_port *x, char *arg)
{
    import_emcopy(x, portps_coll);
    return (PORT_NEXT);
}

static int imaction_T1_set(t_port *x, char *arg)
{
    if (x->x_emstate == portps_vtable)
    {
	if (import_emcopy(x, portps_vtable))
	{
	    int count = port_getint(x, 2);
	    if (count != x->x_emcount)
		loud_warning(0, "import",
			    "[%d] bad vtable chunk index %d (%d already taken)",
			     x->x_messcount, count, x->x_emcount);
	    x->x_emcount += x->x_inatoms - 3;
	}
    }
    else if (x->x_emstate == portps_funbuff)
	import_emcopy(x, portps_funbuff);
    else
	import_unexpected(x);
    return (PORT_NEXT);
}

static int imaction_T1_reset(t_port *x, char *arg)
{
    import_emcopy(x, portps_prob);
    return (PORT_NEXT);
}

static int imaction_T1_embed(t_port *x, char *arg)
{
    import_emcopy(x, portps_prob);
    return (PORT_NEXT);
}

static int imaction_K1_replace(t_port *x, char *arg)
{
    if (x->x_pictfp)
    {
	import_unexpected(x);
	if (x->x_dumping)
	    fclose(x->x_pictfp);
	x->x_pictfp = 0;
    }
    else if (import_emcheck(x, portps_picture))
    {
	char buf[32];
	x->x_emsize = port_getint(x, 2);
	x->x_emcount = 0;
	sprintf(buf, "port-%02d.pict", ++x->x_pictno);
	if (x->x_dumping)
	{
	    if (x->x_pictfp = sys_fopen(buf, "wb"))
	    {
		int i;
		for (i = 0; i < 512; i++) fputc(0, x->x_pictfp);
	    }
	}
	else x->x_pictfp = (FILE *)1;
    }
    return (PORT_NEXT);
}

static int imaction_K1_set(t_port *x, char *arg)
{
    if (!x->x_pictfp)
	import_unexpected(x);
    else if (import_emcheck(x, portps_picture))
    {
	int i, count = port_getint(x, 2);
	if (count != x->x_emcount)
	    loud_warning(0, "import",
			 "[%d] bad picture chunk index %d (%d already taken)",
			 x->x_messcount, count, x->x_emcount);
	x->x_emcount += x->x_inatoms - 3;
	if (x->x_dumping)
	{
	    for (i = 3; i < x->x_inatoms; i++)
	    {
		int v = port_getint(x, i);
		fputc(v >> 24, x->x_pictfp);
		fputc((v >> 16) & 0x0ff, x->x_pictfp);
		fputc((v >> 8) & 0x0ff, x->x_pictfp);
		fputc(v & 0x0ff, x->x_pictfp);
	    }
	}
    }
    return (PORT_NEXT);
}

typedef int (*t_portaction)(t_port *, char *arg);

typedef struct _portslot
{
    char              *s_name;
    t_portaction       s_action;
    char              *s_actionarg;
    struct _portnode  *s_subtree;
    t_symbol          *s_symbol;
} t_portslot;

typedef struct _portnode  /* a parser's symbol definition, sort of... */
{
    t_portslot  *n_table;
    int          n_nslots;
    int          n_index;
} t_portnode;

#define PORT_NSLOTS(slots)  (sizeof(slots)/sizeof(*(slots)))

static t_portslot imslots__N[] =
{
    { "vpatcher",    imaction_N1_vpatcher, 0, 0, 0 },
    { "vtable",      imaction_N1_vtable, 0, 0, 0 },
    { "coll",        imaction_N1_coll, 0, 0, 0 },
    { "funbuff",     imaction_N1_funbuff, 0, 0, 0 },
    { "prob",        imaction_N1_prob, 0, 0, 0 },
    { "picture",     imaction_N1_picture, 0, 0, 0 }
};
static t_portnode imnode__N = { imslots__N, PORT_NSLOTS(imslots__N), 1 };

static t_portslot imslots_newobj[] =
{
    { "patcher",     imaction_P6_patcher, 0, 0, 0 },
    { "p",           imaction_P6_patcher, 0, 0, 0 },
    { "table",       imaction_P6_table, 0, 0, 0 },
    { "coll",        imaction_P6_coll, 0, 0, 0 },
    { "funbuff",     imaction_P6_funbuff, 0, 0, 0 },
    { "prob",        imaction_P6_prob, 0, 0, 0 }
};
static t_portnode imnode_newobj = { imslots_newobj,
				    PORT_NSLOTS(imslots_newobj), 6 };

/* LATER consider merging newobj and newex */
static t_portslot imslots_newex[] =
{
    { "key",         import_obj, "Key", 0, 0 },
    { "keyup",       import_obj, "Keyup", 0, 0 },

    { "pack",        imaction_P6_pack, 0, 0, 0 },
    { "unpack",      imaction_P6_pack, 0, 0, 0 },
    { "trigger",     imaction_P6_pack, 0, 0, 0 },
    { "t",           imaction_P6_pack, 0, 0, 0 },

    { "midiin",      imaction_P6_midi, 0, 0, 0 },
    { "midiout",     imaction_P6_midi, 0, 0, 0 },
    { "notein",      imaction_P6_midi, 0, 0, 0 },
    { "noteout",     imaction_P6_midi, 0, 0, 0 },
    { "pgmin",       imaction_P6_midi, 0, 0, 0 },
    { "pgmout",      imaction_P6_midi, 0, 0, 0 },
    { "ctlin",       imaction_P6_midi, 0, 0, 0 },
    { "ctlout",      imaction_P6_midi, 0, 0, 0 },
    { "bendin",      imaction_P6_midi, 0, 0, 0 },
    { "bendout",     imaction_P6_midi, 0, 0, 0 },

    /* LATER rethink */
    { "Borax",       import_objarg, "Borax", 0, 0 },
    { "Bucket",      import_objarg, "Bucket", 0, 0 },
    { "Decode",      import_objarg, "Decode", 0, 0 },
    { "Histo",       import_objarg, "Histo", 0, 0 },
    { "MouseState",  import_objarg, "MouseState", 0, 0 },
    { "Peak",        import_objarg, "Peak", 0, 0 },
    { "TogEdge",     import_objarg, "TogEdge", 0, 0 },
    { "Trough",      import_objarg, "Trough", 0, 0 },
    { "Uzi",         import_objarg, "Uzi", 0, 0 }
};
static t_portnode imnode_newex = { imslots_newex,
				   PORT_NSLOTS(imslots_newex), 6 };

static t_portslot imslots_user[] =
{
    { "GSwitch",     import_objarg, "Gswitch", 0, 0 },
    { "GSwitch2",    import_objarg, "Ggate", 0, 0 },
    { "number~",     import_obj, 0, 0, 0 },
    { "scope~",      imaction_P2_scope, "Scope~", 0, 0 },
    { "uslider",     import_obj, "vsl", 0, 0 }  /* LATER range and offset */
};
static t_portnode imnode_user = { imslots_user,
				  PORT_NSLOTS(imslots_user), 2 };

static t_portslot imslots__P[] =
{
    { "comment",     imaction_P1_comment, 0, 0, 0 },
    { "message",     imaction_P1_message, 0, 0, 0 },
    { "newobj",      import_objarg, 0, &imnode_newobj, 0 },
    { "newex",       import_objarg, 0, &imnode_newex, 0 },
    { "inlet",       imaction_P1_io, 0, 0, 0 },
    { "inlet~",      imaction_P1_io, 0, 0, 0 },
    { "outlet",      imaction_P1_io, 0, 0, 0 },
    { "outlet~",     imaction_P1_io, 0, 0, 0 },
    { "number",      imaction_P1_number, 0, 0, 0 },
    { "flonum",      imaction_P1_number, 0, 0, 0 },
    { "button",      import_obj, "bng", 0, 0 },
    { "slider" ,     import_obj, "vsl", 0, 0 },  /* LATER range and offset */
    { "hslider",     import_obj, "hsl", 0, 0 },  /* LATER range and offset */
    { "toggle",      import_obj, "tgl", 0, 0 },
    { "user",        import_objarg, 0, &imnode_user, 0 },
    /* state is embedded in #N vpreset <nslots>; #X append... */
    { "preset",      import_obj, "preset", 0, 0 },
    /* an object created from the "Paste Picture" menu,
       state is embedded in #N picture; #K...; */
    { "vpicture",    imaction_P1_vpicture, 0, 0, 0 },
    { "connect",     imaction_P1_connect, 0, 0, 0 },
    { "fasten",      imaction_P1_connect, 0, 0, 0 }
};
static t_portnode imnode__P = { imslots__P, PORT_NSLOTS(imslots__P), 1 };

static t_portslot imslots__T[] =
{
    { "int",         imaction_T1_int, 0, 0, 0 },
    { "flags",       imaction_T1_flags, 0, 0, 0 },
    { "set",         imaction_T1_set, 0, 0, 0 },
    { "reset",       imaction_T1_reset, 0, 0, 0 },
    { "embed",       imaction_T1_embed, 0, 0, 0 }
};
static t_portnode imnode__T = { imslots__T, PORT_NSLOTS(imslots__T), 1 };

static t_portslot imslots__K[] =
{
    { "replace",     imaction_K1_replace, 0, 0, 0 },
    { "set",         imaction_K1_set, 0, 0, 0 }
};
static t_portnode imnode__K = { imslots__K, PORT_NSLOTS(imslots__K), 1 };

static t_portslot imslots_[] =
{
    { "#N",          0, 0, &imnode__N, 0 },
    { "#P",          0, 0, &imnode__P, 0 },
    { "#T",          0, 0, &imnode__T, 0 },
    { "#K",          0, 0, &imnode__K, 0 }
};
static t_portnode imnode_ = { imslots_, PORT_NSLOTS(imslots_), 0 };

static int port_doparse(t_port *x, t_portnode *node)
{
    int nslots = node->n_nslots;
    if (nslots > 0)
    {
	t_portslot *slot = node->n_table;
	t_symbol *insym = port_getanysymbol(x, node->n_index);
	char *inname = 0;
secondpass:
	while (nslots--)
	{
	    if (slot->s_symbol == insym
		|| (inname && shared_matchignorecase(inname, slot->s_name)))
	    {
		if (slot->s_subtree)
		{
		    int nobj = x->x_nobj;
		    int result = port_doparse(x, slot->s_subtree);
		    if (result == PORT_FATAL || result == PORT_CORRUPT ||
			result == PORT_NEXT)
			return (result);
		}
		if (slot->s_action)
		    return (slot->s_action(x, slot->s_actionarg));
		else
		    return (PORT_OK);  /* LATER rethink */
	    }
	    slot++;
	}
	if (!inname)
	{
	    inname = insym->s_name;
	    nslots = node->n_nslots;
	    slot = node->n_table;
	    goto secondpass;
	}
    }
    else loudbug_bug("port_doparse");
    return (PORT_UNKNOWN);
}

static int port_parsemessage(t_port *x)
{
    import_flushillegal(x);
    x->x_messcount++;
    return (port_doparse(x, &imnode_));
}

static void port_startparsing(t_port *x)
{
#ifdef PORT_DEBUG
    loudbug_post("parsing...");
#endif
    x->x_messcount = 0;
    x->x_illmess = 0;
    x->x_lastunexpected = -1;
    x->x_lastbroken = -1;
    x->x_lastinconsistent = -1;
    x->x_nobj = 0;
    x->x_emstate = 0;
    binbuf_clear(x->x_embb);
    x->x_pictno = 0;
    x->x_pictfp = 0;
}

static void port_endparsing(t_port *x)
{
    import_flushillegal(x);
    if (x->x_emstate)
    {
	import_embroken(x, "end of file");
	x->x_emstate = 0;
    }
    binbuf_clear(x->x_embb);
    if (x->x_pictfp)
    {
	loud_warning(0, "import", "incomplete picture");
	if (x->x_dumping)
	    fclose(x->x_pictfp);
	x->x_pictfp = 0;
    }
#ifdef PORT_DEBUG
    loudbug_post("end of parsing");
#endif
}

static void port_dochecksetup(t_portnode *node)
{
    t_portslot *slots = node->n_table;
    int i, nslots = node->n_nslots;
    for (i = 0; i < nslots; i++)
    {
	t_portnode *subtree = slots[i].s_subtree;
	slots[i].s_symbol = gensym(slots[i].s_name);
	if (subtree)
	    port_dochecksetup(subtree);
    }
    import_setdefmapping();
}

#define BOGUS_NINLETS   23
#define BOGUS_NOUTLETS  24

typedef struct _bogus
{
    t_object   x_ob;
    t_glist   *x_glist;  /* used also as 'dirty' flag */
    int        x_bound;
    t_inlet   *x_inlets[BOGUS_NINLETS];
    t_outlet  *x_outlets[BOGUS_NOUTLETS];
    t_clock   *x_clock;
} t_bogus;

typedef struct _bogushook
{
    t_pd      x_pd;
    t_pd     *x_who;
    t_glist  *x_glist;  /* used also as 'dirty' flag */
    t_clock  *x_clock;
} t_bogushook;

static t_class *bogus_class;
static t_class *bogushook_class;

static void bogus_tick(t_bogus *x)
{
    if (x->x_bound)
    {
#ifdef PORT_DEBUG
	loudbug_post("bogus_tick: unbinding '%x'", (int)x);
#endif
	pd_unbind((t_pd *)x, portps_cleanup);
	x->x_bound = 0;
    }
}

static void bogushook_tick(t_bogushook *x)
{
    pd_free((t_pd *)x);
}

static void bogus_cleanup(t_bogus *x)
{
    if (x->x_glist && x->x_glist == canvas_getcurrent())
    {
	t_text *t = (t_text *)x;
	int ac = binbuf_getnatom(t->te_binbuf);
	if (ac)
	{
	    t_atom *av = binbuf_getvec(t->te_binbuf);
	    t_binbuf *bb = binbuf_new();
	    t_inlet **ip;
	    t_outlet **op;
	    int i;
#ifdef PORT_DEBUG
	    loudbug_startpost("self-adjusting ");
	    loudbug_postbinbuf(t->te_binbuf);
#endif
	    binbuf_add(bb, ac - 1, av + 1);
	    binbuf_free(t->te_binbuf);
	    t->te_binbuf = bb;

	    for (i = BOGUS_NINLETS, ip = x->x_inlets + BOGUS_NINLETS - 1;
		 i ; i--, ip--)
	    {
		if (forky_hasfeeders((t_object *)x, x->x_glist, i, 0))
		    break;
		else
		    inlet_free(*ip);
	    }
#ifdef PORT_DEBUG
	    loudbug_post("%d inlets deleted", BOGUS_NINLETS - i);
#endif
	    for (i = 0, op = x->x_outlets + BOGUS_NOUTLETS - 1;
		 i < BOGUS_NOUTLETS; i++, op--)
	    {
		if (fragile_outlet_connections(*op))
		    break;
		else
		    outlet_free(*op);
	    }
#ifdef PORT_DEBUG
	    loudbug_post("%d outlets deleted", i);
#endif
	    glist_retext(x->x_glist, t);
	}
	else loudbug_bug("bogus_cleanup");
	x->x_glist = 0;
	clock_delay(x->x_clock, 0);
    }
}

static void bogus_free(t_bogus *x)
{
    if (x->x_bound) pd_unbind((t_pd *)x, portps_cleanup);
    if (x->x_clock) clock_free(x->x_clock);
}

static void *bogus_new(t_symbol *s, int ac, t_atom *av)
{
    t_bogus *x = 0;
    t_glist *glist;
    if (glist = canvas_getcurrent())
    {
    	char buf[80];
	int i;
	if (av->a_type == A_SYMBOL)
	{
	    t_pd *z;
	    if (z = forky_newobject(av->a_w.w_symbol, ac - 1, av + 1))
	    {
		t_bogushook *y = (t_bogushook *)pd_new(bogushook_class);
		y->x_who = z;
		y->x_glist = glist;
		pd_bind((t_pd *)y, portps_cleanup);
		y->x_clock = clock_new(y, (t_method)bogushook_tick);
#ifdef PORT_DEBUG
		loudbug_post("reclaiming %s", av->a_w.w_symbol->s_name);
#endif
		return (z);
	    }
	}
	x = (t_bogus *)pd_new(bogus_class);
	atom_string(av, buf, 80);
	loud_error((t_pd *)x, "unknown class '%s'", buf);
	x->x_glist = glist;
	for (i = 0; i < BOGUS_NINLETS; i++)
	    x->x_inlets[i] = inlet_new((t_object *)x, (t_pd *)x, 0, 0);
	for (i = 0; i < BOGUS_NOUTLETS; i++)
	    x->x_outlets[i] = outlet_new((t_object *)x, &s_anything);
	pd_bind((t_pd *)x, portps_cleanup);
	x->x_bound = 1;
	x->x_clock = clock_new(x, (t_method)bogus_tick);
    }
    return (x);
}

static void bogushook_cleanup(t_bogushook *x)
{
    if (x->x_glist)
    {
	t_text *t = (t_text *)x->x_who;
	int ac = binbuf_getnatom(t->te_binbuf);
	if (ac > 1)
	{
	    int dorecreate = 0;
	    t_atom *av = binbuf_getvec(t->te_binbuf);
	    t_binbuf *bb = binbuf_new();
#ifdef PORT_DEBUG
	    loudbug_startpost("hook-adjusting ");
	    loudbug_postbinbuf(t->te_binbuf);
#endif
	    ac--; av++;
	    if (av->a_type == A_SYMBOL)
	    {
		if (av->a_w.w_symbol == portps_outlet)
		{
		    if (forky_hasfeeders((t_object *)x->x_who, x->x_glist,
					 0, &s_signal))
		    {
			t_atom at;
			SETSYMBOL(&at, gensym("outlet~"));
			binbuf_add(bb, 1, &at);
			ac--; av++;
			dorecreate = 1;
		    }
		}
		else if (av->a_w.w_symbol == portps_inlet)
		{
		    /* LATER */
		}
	    }
	    if (ac) binbuf_add(bb, ac, av);
	    if (dorecreate) gobj_recreate(x->x_glist, (t_gobj *)t, bb);
	    else
	    {
		binbuf_free(t->te_binbuf);
		t->te_binbuf = bb;
		glist_retext(x->x_glist, t);
	    }
	}
	else loudbug_bug("bogushook_cleanup");
	x->x_glist = 0;
	clock_delay(x->x_clock, 0);
    }
}

static void bogushook_free(t_bogushook *x)
{
#ifdef PORT_DEBUG
    loudbug_post("destroing the hook of '%s'", class_getname(*x->x_who));
#endif
    pd_unbind((t_pd *)x, portps_cleanup);
    if (x->x_clock) clock_free(x->x_clock);
}

static void port_checksetup(void)
{
    static int done = 0;
    if (!done)
    {
	port_dochecksetup(&imnode_);

	portps_bogus = gensym("_port.bogus");
	portps_cleanup = gensym("_port.cleanup");
	portps_inlet = gensym("inlet");
	portps_outlet = gensym("outlet");
	portps_vtable = gensym("vtable");
	portps_coll = gensym("coll");
	portps_funbuff = gensym("funbuff");
	portps_prob = gensym("prob");
	portps_picture = gensym("picture");

	if (zgetfn(&pd_objectmaker, portps_bogus) == 0)
	{
	    bogus_class = class_new(portps_bogus,
				    (t_newmethod)bogus_new,
				    (t_method)bogus_free,
				    sizeof(t_bogus), 0, A_GIMME, 0);
	    class_addmethod(bogus_class, (t_method)bogus_cleanup,
			    portps_cleanup, 0);
	    bogushook_class = class_new(gensym("_port.bogushook"), 0,
					(t_method)bogushook_free,
					sizeof(t_bogushook), CLASS_PD, 0);
	    class_addmethod(bogushook_class, (t_method)bogushook_cleanup,
			    portps_cleanup, 0);
	}
	done = 1;
    }
}

static t_port *port_new(void)
{
    t_port *x = (t_port *)getbytes(sizeof(*x));
    x->x_outbb = 0;
    x->x_withbogus = 0;
    x->x_outsize = PORT_INISIZE;
    x->x_outatoms = 0;
    x->x_outmess = x->x_outini;
    x->x_stacksize = PORT_INISTACK;
    x->x_stackdepth = 0;
    x->x_stack = x->x_stackini;
    x->x_emstate = 0;
    x->x_embb = binbuf_new();
#ifdef PORT_DUMP
    x->x_dumping = 1;
#else
    x->x_dumping = 0;
#endif
    return (x);
}

static void port_free(t_port *x)
{
    if (portps_cleanup->s_thing)
    {
	/* clean up toplevel glist */
	typedmess(portps_cleanup->s_thing, portps_cleanup, 0, 0);
	/* LATER unbind all bogus objects, and destroy all bogushooks
	   by traversing the portps_cleanup's bindlist, instead of
	   using per-object clocks.  Need to have bindlist traversal
	   in Pd API first...  Otherwise, consider fragilizing this
	   (and fragilizing grab too). */
    }
    if (x->x_outmess != x->x_outini)
	freebytes(x->x_outmess, x->x_outsize * sizeof(*x->x_outmess));
    if (x->x_stack != x->x_stackini)
	freebytes(x->x_stack, x->x_stacksize * sizeof(*x->x_stack));
    if (x->x_embb)
	binbuf_free(x->x_embb);
    freebytes(x, sizeof(*x));
}

static int import_binbuf(t_port *x, t_binbuf *inbb, t_binbuf *outbb)
{
    int result = PORT_OK;
    t_atom *av = binbuf_getvec(inbb);
    int ac = binbuf_getnatom(inbb);
    int startmess, endmess;
    x->x_outbb = outbb;
    port_startparsing(x);
    for (startmess = 0; startmess < ac; startmess = endmess + 1)
    {
	t_atom *mess = av + startmess, *ap;
	int i;
    	for (endmess = startmess, ap = mess;
	     ap->a_type != A_SEMI; endmess++, ap++)
	    if (endmess == ac)
	    {
		result = PORT_CORRUPT;  /* no final semi */
		goto endparsing;
	    }
	if (endmess == startmess || endmess == startmess + 1
	    || mess->a_type != A_SYMBOL)
	{
	    startmess = endmess + 1;
	    import_illegal(x);
	    continue;
	}
	if (mess[1].a_type != A_SYMBOL)
	{
	    if (mess[1].a_type != A_INT && mess[1].a_type != A_FLOAT)
	    {
		startmess = endmess + 1;
		import_illegal(x);
		continue;
	    }
	}
	else if (mess[1].a_w.w_symbol == gensym("hidden"))
	{
	    t_symbol *sel = mess[1].a_w.w_symbol;
	    mess[1].a_w.w_symbol = mess->a_w.w_symbol;
	    startmess++;
	    mess++;
	    if (endmess == startmess + 1 || mess[1].a_type != A_SYMBOL)
	    {
		startmess = endmess + 1;
		import_illegal(x);
		continue;
	    }
	}
	x->x_inatoms = endmess - startmess;
	x->x_inmess = mess;
	if ((i = x->x_inatoms + 16) > x->x_outsize)  /* LATER rethink */
	{
	    int sz = i;
	    x->x_outmess = grow_nodata(&sz, &x->x_outsize, x->x_outmess,
				       PORT_INISIZE, x->x_outini,
				       sizeof(*x->x_outmess));
	    if (sz != i)
	    {
		startmess = endmess + 1;
		continue;  /* LATER rethink */
	    }
	}

	/* dollar signs in file translate to symbols,
	   LATER rethink, also #-signs */
	for (i = 0, ap = x->x_inmess; i < x->x_inatoms; i++, ap++)
	{
	    if (ap->a_type == A_DOLLAR)
	    {
		char buf[100];
		sprintf(buf, "$%d", ap->a_w.w_index);
		SETSYMBOL(ap, gensym(buf));
	    }
	    else if (ap->a_type == A_DOLLSYM)
	    {
		char buf[100];
		sprintf(buf, "$%s", ap->a_w.w_symbol->s_name);
		SETSYMBOL(ap, gensym(buf));
	    }
	}
	if (port_parsemessage(x) == PORT_FATAL)
	{
	    result = PORT_FATAL;
	    goto endparsing;
	}
    }
endparsing:
    port_endparsing(x);
    return (result);
}

int import_max(char *fn, char *dir)
{
    int result;
    t_port *x;
    t_binbuf *inbb, *outbb;
    int fd;
    char buf[MAXPDSTRING], *bufp;
    t_pd *stackp = 0;
    int dspstate = canvas_suspend_dsp();
    port_checksetup();
    if ((fd = open_via_path(dir, fn, "", buf, &bufp, MAXPDSTRING, 0)) < 0)
    {
    	loud_error(0, "%s: can't open", fn);
    	return (BINPORT_NOFILE);
    }
    else close (fd);

    x = port_new();
    inbb = binbuf_new();
    glob_setfilename(0, gensym(bufp), gensym(buf));
    result = binport_read(inbb, bufp, buf);
    if (result == BINPORT_MAXBINARY ||
	result == BINPORT_MAXTEXT ||
	result == BINPORT_MAXOLD)
    {
	int bbresult;
#ifdef PORT_DEBUG
	binport_write(inbb, "import-debug.pat", "");
#endif
	outbb = binbuf_new();
	if ((bbresult = import_binbuf(x, inbb, outbb)) != PORT_OK)
	{
	    loud_error(0, "%s: import failed (%d)", fn, bbresult);
	    if (bbresult == PORT_CORRUPT)
		result = BINPORT_CORRUPT;
	    else
		result = BINPORT_FAILED;
	    binbuf_free(outbb);
	    outbb = 0;
	}		
	binbuf_free(inbb);
#ifdef PORT_LOG
	if (outbb) binbuf_write(outbb, "import-result.pd", "", 0);
#endif
    }
    else if (result == BINPORT_PDFILE)
	outbb = inbb;
    else
    {
    	perror(fn);  /* FIXME */
	binbuf_free(inbb);
	outbb = 0;
    }
    if (outbb)
    {
	binbuf_eval(outbb, 0, 0, 0);
	binbuf_free(outbb);
    }
    port_free(x);

    glob_setfilename(0, &s_, &s_);
    canvas_resume_dsp(dspstate);
    while ((stackp != s__X.s_thing) && (stackp = s__X.s_thing))
    	vmess(stackp, gensym("pop"), "i", 1);

#if 0  /* LATER */
    pd_doloadbang();
#endif

    return (result);
}
