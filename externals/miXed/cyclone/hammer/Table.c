/* Copyright (c) 2004-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* Write access is totally encapsulated in tablecommon calls, in order
   to simplify proper handling of the distribution cache.  Direct read
   access from table calls is allowed (for speed). */

#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "g_canvas.h"
#include "common/loud.h"
#include "common/grow.h"
#include "common/rand.h"
#include "hammer/file.h"

#ifdef KRZYSZCZ
#define TABLE_DEBUG
#endif

#define TABLE_INISIZE      256  /* LATER rethink */
#define TABLE_DEFLENGTH    128  /* CHECKED */
#define TABLE_MINLENGTH      2  /* CHECKED */
#define TABLE_MAXLENGTH  16383  /* CHECKED, LATER rethink */
#define TABLE_MINRANGE       2  /* CHECKED */
#define TABLE_MAXQ       32768  /* CHECKME */

typedef struct _tablecommon
{
    t_pd            c_pd;
    struct _table  *c_refs;
    int             c_increation;
    int             c_volatile;
    int             c_selfmodified;
    int             c_entered;  /* a counter, LATER rethink */
    /* CHECKED flags, etc. are common fields */
    int             c_visflag;
    int             c_embedflag;
    int             c_dontsaveflag;
    int             c_notenamesflag;
    int             c_signedflag;
    int             c_range;
    int             c_left;
    int             c_top;
    int             c_right;
    int             c_bottom;
    int             c_size;    /* as allocated */
    int             c_length;  /* as used */
    int            *c_table;
    int             c_tableini[TABLE_INISIZE];
    int             c_cacheisfresh;
    int             c_cachesum;
    int             c_cachemin;
    int             c_cachemax;
    int            *c_cache;
    int             c_cacheini[TABLE_INISIZE];
    t_symbol       *c_filename;
    t_canvas       *c_lastcanvas;
    t_hammerfile   *c_filehandle;
} t_tablecommon;

typedef struct _table
{
    t_object        x_ob;
    t_canvas       *x_glist;
    t_symbol       *x_name;
    t_tablecommon  *x_common;
    t_float         x_value;
    int             x_valueset;
    int             x_head;
    int             x_intraversal;  /* ``set-with-next/prev'' flag */
    int             x_loadflag;
    int             x_loadndx;
    unsigned int    x_seed;
    t_hammerfile   *x_filehandle;
    t_outlet       *x_bangout;
    struct _table  *x_next;
} t_table;

static t_class *table_class;
static t_class *tablecommon_class;

static void tablecommon_modified(t_tablecommon *cc, int relocated)
{
    cc->c_cacheisfresh = 0;
    if (cc->c_increation)
	return;
    if (relocated)
    {
	cc->c_volatile = 1;
    }
    if (cc->c_embedflag)
    {
	t_table *x;
	for (x = cc->c_refs; x; x = x->x_next)
	    if (x->x_glist && glist_isvisible(x->x_glist))
		canvas_dirty(x->x_glist, 1);
    }
}

static int tablecommon_getindex(t_tablecommon *cc, int ndx)
{
    int nmx = cc->c_length - 1;
    /* CHECKED ndx silently clipped */
    return (ndx < 0 ? 0 : (ndx > nmx ? nmx : ndx));
}

static int tablecommon_getvalue(t_tablecommon *cc, int ndx)
{
    int nmx = cc->c_length - 1;
    /* CHECKED ndx silently clipped */
    return (cc->c_table[ndx < 0 ? 0 : (ndx > nmx ? nmx : ndx)]);
}

static void tablecommon_setvalue(t_tablecommon *cc, int ndx, int v)
{
    int nmx = cc->c_length - 1;
    /* CHECKED ndx silently clipped, value not clipped */
    cc->c_table[ndx < 0 ? 0 : (ndx > nmx ? nmx : ndx)] = v;
    tablecommon_modified(cc, 0);
}

static int tablecommon_loadvalue(t_tablecommon *cc, int ndx, int v)
{
    /* CHECKME */
    if (ndx < cc->c_length)
    {
	cc->c_table[ndx] = v;
	tablecommon_modified(cc, 0);
	return (1);
    }
    else return (0);
}

static void tablecommon_setall(t_tablecommon *cc, int v)
{
    int ndx = cc->c_length;
    int *ptr = cc->c_table;
    while (ndx--) *ptr++ = v;
    tablecommon_modified(cc, 0);
}

static void tablecommon_setatoms(t_tablecommon *cc, int ndx, int ac, t_atom *av)
{
    if (ac > 1 && av->a_type == A_FLOAT)
    {
	/* CHECKED no resizing */
	int last = tablecommon_getindex(cc, ndx + ac - 1);
	int *ptr = cc->c_table + ndx;
	for (; ndx <= last; ndx++, av++)
	     *ptr++ = (av->a_type == A_FLOAT ? (int)av->a_w.w_float : 0);
	tablecommon_modified(cc, 0);
    }
}

static void tablecommon_setlength(t_tablecommon *cc, int length)
{
    int relocate;
    if (length < TABLE_MINLENGTH)
	length = TABLE_MINLENGTH;
    else if (length > TABLE_MAXLENGTH)
	length = TABLE_MAXLENGTH;
    if (relocate = (length > cc->c_size))
    {
	int l = length;
	/* CHECKED existing values are preserved */
	cc->c_table = grow_withdata(&length, &cc->c_length,
				    &cc->c_size, cc->c_table,
				    TABLE_INISIZE, cc->c_tableini,
				    sizeof(*cc->c_table));
	if (length == l)
	    cc->c_table = grow_nodata(&length, &cc->c_size, cc->c_cache,
				      TABLE_INISIZE, cc->c_cacheini,
				      sizeof(*cc->c_cache));
	if (length != l)
	{
	    if (cc->c_table != cc->c_tableini)
		freebytes(cc->c_table, cc->c_size * sizeof(*cc->c_table));
	    if (cc->c_cache != cc->c_cacheini)
		freebytes(cc->c_cache, cc->c_size * sizeof(*cc->c_cache));
	    cc->c_size = length = TABLE_INISIZE;
	    cc->c_table = cc->c_tableini;
	    cc->c_cache = cc->c_cacheini;
	}
    }
    cc->c_length = length;
    /* CHECKED values at common indices are preserved */
    /* CHECKED rewinding neither head, nor loadndx -- both are preserved,
       although there is a bug in handling of 'prev' after the head
       overflows due to shrinking. */
    tablecommon_modified(cc, relocate);
}

static void tablecommon_cacheupdate(t_tablecommon *cc)
{
    int ndx = cc->c_length, sum = 0, mn, mx;
    int *tptr = cc->c_table, *cptr = cc->c_cache;
    mn = mx = *tptr;
    while (ndx--)
    {
	int v = *tptr++;
	*cptr++ = (sum += v);
	if (mn > v)
	    mn = v;
	else if (mx < v)
	    mx = v;
    }
    cc->c_cachesum = sum;
    cc->c_cachemin = mn;
    cc->c_cachemax = mx;
    cc->c_cacheisfresh = 1;
}

static int tablecommon_quantile(t_tablecommon *cc, float f)
{
    /* CHECKME */
    float fv;
    int ndx, *ptr, nmx = cc->c_length - 1;
    if (!cc->c_cacheisfresh) tablecommon_cacheupdate(cc);
    fv = f * cc->c_cachesum;
    for (ndx = 0, ptr = cc->c_cache; ndx < nmx; ndx++, ptr++)
	if (*ptr >= fv)
	    break;
    return (ndx);
}

static void tablecommon_fromatoms(t_tablecommon *cc, int ac, t_atom *av)
{
    int i, size = 0, nsyms = 0;
    t_atom *ap;
    int *ptr;
    cc->c_increation = 1;
    for (i = 0, ap = av; i < ac; i++, ap++)
    {
	if (ap->a_type == A_FLOAT)
	    size++;
	else if (ap->a_type == A_SYMBOL)
	    nsyms++, size++;
    }
    if (size < ac)
	loud_warning(0, "Table", "%d invalid atom%s ignored",
		     ac - size, (ac - size > 1 ? "s" : ""));
    if (nsyms)
	loud_warning(0, "Table", "%d symbol%s bashed to zero",
		     nsyms, (nsyms > 1 ? "s" : ""));
    tablecommon_setlength(cc, size);
    size = cc->c_length;
    ptr = cc->c_table;
    for (i = 0; i < ac; i++, av++)
    {
	if (av->a_type == A_FLOAT)
	    *ptr++ = (int)av->a_w.w_float;
	else if (av->a_type == A_SYMBOL)
	    *ptr++ = 0;
	else
	    continue;
	if (size-- == 1)
	    break;
    }
    while (size--)
	*ptr++ = 0;
    cc->c_increation = 0;
}

/* FIXME keep int precision: save/load directly, not through a bb */
/* LATER binary files */
static void tablecommon_doread(t_tablecommon *cc, t_symbol *fn, t_canvas *cv)
{
    t_binbuf *bb = binbuf_new();
    int ac;
    t_atom *av;
    char buf[MAXPDSTRING];
    if (!fn)
	return;  /* CHECKME complaint */
    /* FIXME use open_via_path() */
    if (cv || (cv = cc->c_lastcanvas))  /* !cv: 'read' w/o arg */
	canvas_makefilename(cv, fn->s_name, buf, MAXPDSTRING);
    else
    {
    	strncpy(buf, fn->s_name, MAXPDSTRING);
    	buf[MAXPDSTRING-1] = 0;
    }
    binbuf_read(bb, buf, "", 0);
    if ((ac = binbuf_getnatom(bb)) &&
	(av = binbuf_getvec(bb)) &&
	av->a_type == A_SYMBOL &&
	av->a_w.w_symbol == gensym("table"))
    {
	tablecommon_fromatoms(cc, ac - 1, av + 1);
	post("Table: %s read successful", fn->s_name);  /* CHECKME */
    }
#if 0  /* FIXME */
    else  /* CHECKME complaint */
	loud_error((t_pd *)cc, "invalid file %s", fn->s_name);
#endif
    binbuf_free(bb);
}

static void tablecommon_readhook(t_pd *z, t_symbol *fn, int ac, t_atom *av)
{
    tablecommon_doread((t_tablecommon *)z, fn, 0);
}

static void tablecommon_dowrite(t_tablecommon *cc, t_symbol *fn, t_canvas *cv)
{
    t_binbuf *bb = binbuf_new();
    char buf[MAXPDSTRING];
    int ndx, *ptr;
    if (!fn)
	return;  /* CHECKME complaint */
    if (cv || (cv = cc->c_lastcanvas))  /* !cv: 'write' w/o arg */
	canvas_makefilename(cv, fn->s_name, buf, MAXPDSTRING);
    else
    {
    	strncpy(buf, fn->s_name, MAXPDSTRING);
    	buf[MAXPDSTRING-1] = 0;
    }
    binbuf_addv(bb, "s", atom_getsymbol(binbuf_getvec(cc->c_refs->x_ob.te_binbuf)));
    for (ndx = 0, ptr = cc->c_table; ndx < cc->c_length; ndx++, ptr++)
	binbuf_addv(bb, "i", *ptr);
    binbuf_write(bb, buf, "", 0);
    binbuf_free(bb);
}

static void tablecommon_writehook(t_pd *z, t_symbol *fn, int ac, t_atom *av)
{
    tablecommon_dowrite((t_tablecommon *)z, fn, 0);
}

static void table_embedhook(t_pd *z, t_binbuf *bb, t_symbol *bindsym)
{
    t_table *x = (t_table *)z;
    t_tablecommon *cc = x->x_common;
    if (cc->c_embedflag)
    {
	int ndx = 0, left = cc->c_length;
	int *ptr = cc->c_table;
	binbuf_addv(bb, "ssi;", bindsym, gensym("size"), cc->c_length);
	binbuf_addv(bb, "ssiiii;", bindsym, gensym("flags"), 1,
		    cc->c_dontsaveflag, cc->c_notenamesflag, cc->c_signedflag);
	binbuf_addv(bb, "ssi;", bindsym, gensym("tabrange"), cc->c_range);
	binbuf_addv(bb, "ssiiiii;", bindsym, gensym("_coords"),
		    cc->c_left, cc->c_top, cc->c_right, cc->c_bottom,
		    cc->c_visflag);
	while (left > 0)
	{
	    int cnt = (left > 128 ? 128 : left);
	    left -= cnt;
	    ndx += cnt;
	    binbuf_addv(bb, "ssi", bindsym, gensym("set"), ndx);
	    while (cnt--)
	    {
		t_atom at;
		SETFLOAT(&at, (float)*ptr);
		binbuf_add(bb, 1, &at);
		ptr++;
	    }
	    binbuf_addsemi(bb);
	}
    }
}

static void tablecommon_editorhook(t_pd *z, t_symbol *s, int ac, t_atom *av)
{
    tablecommon_fromatoms((t_tablecommon *)z, ac, av);
}

static void tablecommon_free(t_tablecommon *cc)
{
    if (cc->c_table != cc->c_tableini)
	freebytes(cc->c_table, cc->c_size * sizeof(*cc->c_table));
    if (cc->c_cache != cc->c_cacheini)
	freebytes(cc->c_cache, cc->c_size * sizeof(*cc->c_cache));
}

static void *tablecommon_new(void)
{
    t_tablecommon *cc = (t_tablecommon *)pd_new(tablecommon_class);
    cc->c_visflag = 0;
    cc->c_embedflag = 0;
    cc->c_dontsaveflag = 0;
    cc->c_notenamesflag = 0;
    cc->c_signedflag = 0;
    cc->c_size = TABLE_INISIZE;
    cc->c_length = TABLE_DEFLENGTH;
    cc->c_table = cc->c_tableini;
    cc->c_cache = cc->c_cacheini;
    cc->c_cacheisfresh = 0;
    return (cc);
}

static t_tablecommon *table_checkcommon(t_table *x)
{
    if (x->x_name &&
	x->x_common != (t_tablecommon *)pd_findbyclass(x->x_name,
						       tablecommon_class))
    {
	loudbug_bug("table_checkcommon");
	return (0);
    }
    return (x->x_common);
}

static void table_unbind(t_table *x)
{
    /* LATER consider calling table_checkcommon(x) */
    t_tablecommon *cc = x->x_common;
    t_table *prev, *next;
    if ((prev = cc->c_refs) == x)
    {
	if (!(cc->c_refs = x->x_next))
	{
	    hammerfile_free(cc->c_filehandle);
	    tablecommon_free(cc);
	    if (x->x_name) pd_unbind(&cc->c_pd, x->x_name);
	    pd_free(&cc->c_pd);
	}
    }
    else if (prev)
    {
	while (next = prev->x_next)
	{
	    if (next == x)
	    {
		prev->x_next = next->x_next;
		break;
	    }
	    prev = next;
	}
    }
    x->x_common = 0;
    x->x_name = 0;
    x->x_next = 0;
}

static void table_bind(t_table *x, t_symbol *name)
{
    t_tablecommon *cc = 0;
    if (name == &s_)
	name = 0;
    else if (name)
	cc = (t_tablecommon *)pd_findbyclass(name, tablecommon_class);
    if (!cc)
    {
	cc = (t_tablecommon *)tablecommon_new();
	cc->c_refs = 0;
	cc->c_increation = 0;
	if (name)
	{
	    pd_bind(&cc->c_pd, name);
	    /* LATER rethink canvas unpredictability */
	    tablecommon_doread(cc, name, x->x_glist);
	}
	else
	{
	    cc->c_filename = 0;
	    cc->c_lastcanvas = 0;
	}
	cc->c_filehandle = hammerfile_new((t_pd *)cc, 0, tablecommon_readhook,
					  tablecommon_writehook,
					  tablecommon_editorhook);
    }
    x->x_common = cc;
    x->x_name = name;
    x->x_next = cc->c_refs;
    cc->c_refs = x;
}

static int table_rebind(t_table *x, t_symbol *name)
{
    t_tablecommon *cc;
    if (name && name != &s_ &&
	(cc = (t_tablecommon *)pd_findbyclass(name, tablecommon_class)))
    {
	table_unbind(x);
	x->x_common = cc;
	x->x_name = name;
	x->x_next = cc->c_refs;
	cc->c_refs = x;
	return (1);
    }
    else return (0);
}

static void table_dooutput(t_table *x, int ndx)
{
    outlet_float(((t_object *)x)->ob_outlet,
		 (t_float)tablecommon_getvalue(x->x_common, ndx));
}

static void table_bang(t_table *x)
{
    /* CHECKME */
    outlet_float(((t_object *)x)->ob_outlet,
		 (t_float)tablecommon_quantile(x->x_common,
					       rand_unipolar(&x->x_seed)));
}

static void table_float(t_table *x, t_float f)
{
    if (x->x_loadflag)
    {
	/* CHECKME */
	if (tablecommon_loadvalue(x->x_common, x->x_loadndx, (int)f))
	    x->x_loadndx++;
    }
    else
    {
	int ndx = (int)f;  /* CHECKED floats are truncated */
	if (x->x_valueset)
	{
	    tablecommon_setvalue(x->x_common, ndx, x->x_value);
	    x->x_valueset = 0;
	}
	else table_dooutput(x, ndx);
	/* CHECKED head is not updated */
    }
}

static void table_ft1(t_table *x, t_floatarg f)
{
    x->x_value = (int)f;  /* CHECKED floats are truncated */
    x->x_valueset = 1;
}

static void table_size(t_table *x, t_floatarg f)
{
    tablecommon_setlength(x->x_common, (int)f);
}

static void table_set(t_table *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac > 1 && av->a_type == A_FLOAT)
    {
	int ndx = tablecommon_getindex(x->x_common, (int)av->a_w.w_float);
	tablecommon_setatoms(x->x_common, ndx, ac - 1, av + 1);
    }
}

static void table_flags(t_table *x, t_symbol *s, int ac, t_atom *av)
{
    t_tablecommon *cc = x->x_common;
    int i = 0, v;
    while (ac && av->a_type == A_FLOAT
	&& loud_checkint((t_pd *)x, av->a_w.w_float, &v, gensym("flags")))
    {
	/* CHECKED order, modifying only explicitly specified flags */
	if (i == 0)
	    cc->c_embedflag = (v != 0);
	else if (i == 1)
	    cc->c_dontsaveflag = (v != 0);
	else if (i == 2)
	    cc->c_notenamesflag = (v != 0);
	else if (i == 3)
	    cc->c_signedflag = (v != 0);
	else
	    break;
	i++; ac--; av++;
    }
}

static void table_tabrange(t_table *x, t_floatarg f)
{
    int i = (int)f;
    x->x_common->c_range = (i > TABLE_MINRANGE ? i : TABLE_MINRANGE);
}

static void table__coords(t_table *x, t_floatarg fl, t_floatarg ft,
			  t_floatarg fr, t_floatarg fb, t_floatarg fv)
{
    t_tablecommon *cc = x->x_common;
    /* FIXME constraints */
    cc->c_left = (int)fl;
    cc->c_top = (int)ft;
    cc->c_right = (int)fr;
    cc->c_bottom = (int)fb;
    cc->c_visflag = ((int)fv != 0);
}

static void table_cancel(t_table *x)
{
    x->x_valueset = 0;
}

static void table_clear(t_table *x)
{
    tablecommon_setall(x->x_common, 0);
    /* CHECKED head preserved */
}

static void table_const(t_table *x, t_floatarg f)
{
    tablecommon_setall(x->x_common, (int)f);
    /* CHECKED head preserved */
}

static void table_load(t_table *x)
{
    x->x_loadflag = 1;
    x->x_loadndx = 0;  /* CHECKED rewind, head not affected */
}

static void table_normal(t_table *x)
{
    x->x_loadflag = 0;
}

static void table_next(t_table *x)
{
    if (!x->x_intraversal)
	x->x_intraversal = 1;
    else if (++x->x_head >= x->x_common->c_length)
	x->x_head = 0;
    table_dooutput(x, x->x_head);
}

static void table_prev(t_table *x)
{
    if (!x->x_intraversal)
	x->x_intraversal = 1;
    else if (--x->x_head < 0)
	x->x_head = x->x_common->c_length - 1;
    table_dooutput(x, x->x_head);
}

static void table_goto(t_table *x, t_floatarg f)
{
    /* CHECKED floats are truncated */
    x->x_head = tablecommon_getindex(x->x_common, (int)f);
    /* CHECKED the head should be pre-updated during traversal, in order
       to maintain the logical way of direction change.  Therefore, we
       need the flag below, which locks head in place that has just been
       set by goto.  The flag is then set by next or prev. */
    x->x_intraversal = 0;
}

static void table_send(t_table *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac > 1 && av->a_type == A_SYMBOL)
    {
	t_symbol *target = av->a_w.w_symbol;
	if (!target->s_thing)
	    return;  /* CHECKED no complaint */
	ac--; av++;
	if (av->a_type == A_FLOAT)
	{
	    if (ac == 1)
	    {
		int ndx = (int)av->a_w.w_float;
		pd_float(target->s_thing,
			 (t_float)tablecommon_getvalue(x->x_common, ndx));
	    }
	    /* CHECKED incompatible: 'send <target> <ndx> <value>'
	       stores <value> at <ndx> (a bug?) */
	}
	else if (av->a_type == A_SYMBOL)
	{
	    /* CHECKED 'send <target> length' works, but not max, min, sum... */
	    if (av->a_w.w_symbol == gensym("length"))
		pd_float(target->s_thing, (t_float)x->x_common->c_length);
	}
    }
}

static void table_length(t_table *x)
{
    outlet_float(((t_object *)x)->ob_outlet, (t_float)x->x_common->c_length);
}

static void table_sum(t_table *x)
{
    t_tablecommon *cc = x->x_common;
    if (!cc->c_cacheisfresh) tablecommon_cacheupdate(cc);
    outlet_float(((t_object *)x)->ob_outlet, (t_float)cc->c_cachesum);
}

static void table_min(t_table *x)
{
    t_tablecommon *cc = x->x_common;
    if (!cc->c_cacheisfresh) tablecommon_cacheupdate(cc);
    outlet_float(((t_object *)x)->ob_outlet, (t_float)cc->c_cachemin);
}

static void table_max(t_table *x)
{
    t_tablecommon *cc = x->x_common;
    if (!cc->c_cacheisfresh) tablecommon_cacheupdate(cc);
    outlet_float(((t_object *)x)->ob_outlet, (t_float)cc->c_cachemax);
}

static void table_getbits(t_table *x, t_floatarg f1,
			  t_floatarg f2, t_floatarg f3)
{
    /* FIXME */
}

static void table_setbits(t_table *x, t_floatarg f1,
			  t_floatarg f2, t_floatarg f3, t_floatarg f4)
{
    /* FIXME */
}

static void table_inv(t_table *x, t_floatarg f)
{
    /* CHECKME none found, float */
    int v = (int)f, ndx, *ptr, nmx = x->x_common->c_length - 1;
    for (ndx = 0, ptr = x->x_common->c_table; ndx < nmx; ndx++, ptr++)
	if (*ptr >= v)
	    break;
    outlet_float(((t_object *)x)->ob_outlet, (t_float)ndx);
}

static void table_quantile(t_table *x, t_floatarg f)
{
    /* CHECKME */
    outlet_float(((t_object *)x)->ob_outlet,
		 (t_float)tablecommon_quantile(x->x_common,
					       f / ((float)TABLE_MAXQ)));
}

static void table_fquantile(t_table *x, t_floatarg f)
{
    /* CHECKME constraints */
    outlet_float(((t_object *)x)->ob_outlet,
		 (t_float)tablecommon_quantile(x->x_common, f));
}

static void table_dump(t_table *x, t_symbol *s, int ac, t_atom *av)
{
    t_tablecommon *cc = x->x_common;
    int thelength = cc->c_length;
    int *thetable = cc->c_table;
    t_outlet *out = ((t_object *)x)->ob_outlet;
    int ndx, nmx, *ptr;
    /* CHECKED optional arguments: from, to,  Negative 'from' causes
       invalid output, symbols are bashed to zero for both arguments,
       inconsistent warnings, etc. -- no strict emulation attempted below. */
    if (ac && av->a_type == A_FLOAT)
	ndx = tablecommon_getindex(cc, (int)av->a_w.w_float);
    else
	ndx = 0;
    if (ac > 1 && av[1].a_type == A_FLOAT)
	nmx = tablecommon_getindex(cc, (int)av[1].a_w.w_float);
    else
	nmx = thelength - 1;
    for (ptr = thetable + ndx; ndx <= nmx; ndx++, ptr++)
    {
	/* Plain traversing by incrementing a pointer is not robust,
	   because calling outlet_float() may invalidate the pointer.
	   Continous storage does not require generic selfmod detection
	   (ala coll), so we can get away with the simpler test below. */
	if (cc->c_length != thelength || cc->c_table != thetable)
	    break;
	/* CHECKED no remote dumping */
	outlet_float(out, (t_float)*ptr);
    }
}

static void table_refer(t_table *x, t_symbol *s)
{
    if (!table_rebind(x, s))
    {
	/* LATER consider complaining */
    }
}

static void table_read(t_table *x, t_symbol *s)
{
    t_tablecommon *cc = x->x_common;
    if (s && s != &s_)
	tablecommon_doread(cc, s, x->x_glist);
    else
	hammerpanel_open(cc->c_filehandle, 0);
}

static void table_write(t_table *x, t_symbol *s)
{
    t_tablecommon *cc = x->x_common;
    if (s && s != &s_)
	tablecommon_dowrite(cc, s, x->x_glist);
    else
	hammerpanel_save(cc->c_filehandle, 0, 0);
}

static int tablecommon_editorappend(t_tablecommon *cc,
				    int v, char *buf, int col)
{
    char *bp = buf;
    int cnt = 0;
    if (col > 0)
	*bp++ = ' ', cnt++;
    cnt += sprintf(bp, "%d", v);
    if (col + cnt > 80)
	buf[0] = '\n', col = cnt - 1;  /* assuming col > 0 */
    else
	col += cnt;
    hammereditor_append(cc->c_filehandle, buf);
    return (col);
}

static void table_open(t_table *x)
{
    t_tablecommon *cc = x->x_common;
    char buf[MAXPDSTRING];
    int *bp = cc->c_table;
    int count = cc->c_length, col = 0;
    hammereditor_open(cc->c_filehandle, (x->x_name ? x->x_name->s_name : 0), 0);
    while (count--)
	col = tablecommon_editorappend(cc, *bp++, buf, col);
    hammereditor_setdirty(cc->c_filehandle, 0);
}

static void table_wclose(t_table *x)
{
    hammereditor_close(x->x_common->c_filehandle, 1);
}

static void table_click(t_table *x, t_floatarg xpos, t_floatarg ypos,
			t_floatarg shift, t_floatarg ctrl, t_floatarg alt)
{
    table_open(x);
}

#ifdef TABLE_DEBUG
static void table_debug(t_table *x, t_floatarg f)
{
    t_tablecommon *cc = table_checkcommon(x);
    if (cc)
    {
	t_table *x1 = cc->c_refs;
	int i = 0;
	while (x1) i++, x1 = x1->x_next;
	loudbug_post("refcount %d", i);
    }
}
#endif

static void table_free(t_table *x)
{
    hammerfile_free(x->x_filehandle);
    table_unbind(x);
}

static void *table_new(t_symbol *s)
{
    t_table *x = (t_table *)pd_new(table_class);
    static int warned = 0;
    if (!warned)
    {
	loud_warning((t_pd *)x, 0, "Table is not ready yet");
	warned = 1;
    }
    x->x_glist = canvas_getcurrent();
    x->x_valueset = 0;
    x->x_head = 0;
    x->x_intraversal = 0;  /* CHECKED */
    x->x_loadflag = 0;
    rand_seed(&x->x_seed, 0);
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft1"));
    outlet_new((t_object *)x, &s_float);
    x->x_bangout = outlet_new((t_object *)x, &s_bang);
    x->x_filehandle = hammerfile_new((t_pd *)x, table_embedhook, 0, 0, 0);
    table_bind(x, s);
    return (x);
}

void Table_setup(void)
{
    table_class = class_new(gensym("Table"),
			    (t_newmethod)table_new,
			    (t_method)table_free,
			    sizeof(t_table), 0, A_DEFSYM, 0);
    class_addbang(table_class, table_bang);
    class_addfloat(table_class, table_float);
    class_addmethod(table_class, (t_method)table_ft1,
		    gensym("ft1"), A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table_size,
		    gensym("size"), A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table_set,
		    gensym("set"), A_GIMME, 0);
    class_addmethod(table_class, (t_method)table_flags,
		    gensym("flags"), A_GIMME, 0);
    class_addmethod(table_class, (t_method)table_tabrange,
		    gensym("tabrange"), A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table__coords,
		    gensym("_coords"),
		    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table_cancel,
		    gensym("cancel"), 0);
    class_addmethod(table_class, (t_method)table_clear,
		    gensym("clear"), 0);
    class_addmethod(table_class, (t_method)table_const,
		    gensym("const"), A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table_load,
		    gensym("load"), 0);
    class_addmethod(table_class, (t_method)table_normal,
		    gensym("normal"), 0);
    class_addmethod(table_class, (t_method)table_next,
		    gensym("next"), 0);
    class_addmethod(table_class, (t_method)table_prev,
		    gensym("prev"), 0);
    class_addmethod(table_class, (t_method)table_goto,
		    gensym("goto"), A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table_send,
		    gensym("send"), A_GIMME, 0);
    class_addmethod(table_class, (t_method)table_length,
		    gensym("length"), 0);
    class_addmethod(table_class, (t_method)table_sum,
		    gensym("sum"), 0);
    class_addmethod(table_class, (t_method)table_min,
		    gensym("min"), 0);
    class_addmethod(table_class, (t_method)table_max,
		    gensym("max"), 0);
    class_addmethod(table_class, (t_method)table_getbits,
		    gensym("getbits"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table_setbits,
		    gensym("setbits"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table_inv,
		    gensym("inv"), A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table_quantile,
		    gensym("quantile"), A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table_fquantile,
		    gensym("fquantile"), A_FLOAT, 0);
    class_addmethod(table_class, (t_method)table_dump,
		    gensym("dump"), A_GIMME, 0);
    class_addmethod(table_class, (t_method)table_refer,
		    gensym("refer"), A_SYMBOL, 0);
    class_addmethod(table_class, (t_method)table_read,
		    gensym("read"), A_DEFSYM, 0);
    class_addmethod(table_class, (t_method)table_write,
		    gensym("write"), A_DEFSYM, 0);
    class_addmethod(table_class, (t_method)table_open,
		    gensym("open"), 0);
    class_addmethod(table_class, (t_method)table_wclose,
		    gensym("wclose"), 0);
    class_addmethod(table_class, (t_method)table_click,
		    gensym("click"),
		    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
#ifdef TABLE_DEBUG
    class_addmethod(table_class, (t_method)table_debug,
		    gensym("debug"), A_DEFFLOAT, 0);
#endif
    hammerfile_setup(table_class, 1);
    tablecommon_class = class_new(gensym("Table"), 0, 0,
				 sizeof(t_tablecommon), CLASS_PD, 0);
    /* this call is a nop (tablecommon does not embed, and the hammerfile
       class itself has been already set up above), but it is better to
       have it around, just in case... */
    hammerfile_setup(tablecommon_class, 0);
}
