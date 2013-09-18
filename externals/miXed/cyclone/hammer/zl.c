/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* FIXME zl sub */

#include <string.h>
#include "m_pd.h"
#include "common/loud.h"
#include "common/grow.h"

/* CHECKME bang behaviour (every mode) */
/* LATER test reentrancy, tune speedwise */

#ifdef KRZYSZCZ
//#define ZL_DEBUG
#endif

#define ZL_INISIZE     32  /* LATER rethink */
#define ZL_MAXSIZE    256
#define ZL_MAXMODES    16
#define ZL_DEFMODE      0

struct _zl;
typedef int (*t_zlintargfn)(struct _zl *, int);
typedef void (*t_zlanyargfn)(struct _zl *, t_symbol *, int, t_atom *);
typedef int (*t_zlnatomsfn)(struct _zl *);
typedef void (*t_zldoitfn)(struct _zl *, int, t_atom *, int);

static int           zl_nmodes = 0;
static t_symbol     *zl_modesym[ZL_MAXMODES];
static int           zl_modeflags[ZL_MAXMODES];
static t_zlintargfn  zl_intargfn[ZL_MAXMODES];
static t_zlanyargfn  zl_anyargfn[ZL_MAXMODES];
static t_zlnatomsfn  zl_natomsfn[ZL_MAXMODES];
static t_zldoitfn    zl_doitfn[ZL_MAXMODES];

typedef struct _zldata
{
    int      d_size;    /* as allocated */
    int      d_natoms;  /* as used */
    t_atom  *d_buf;
    t_atom   d_bufini[ZL_INISIZE];
} t_zldata;

typedef struct _zl
{
    t_object          x_ob;
    struct _zlproxy  *x_proxy;
    int               x_entered;
    int               x_locked;  /* locking inbuf1 in modes: iter, reg, slice */
    t_zldata          x_inbuf1;
    t_zldata          x_inbuf2;
    t_zldata          x_outbuf;
    int               x_mode;
    int               x_modearg;
    t_outlet         *x_out2;
} t_zl;

typedef struct _zlproxy
{
    t_object  p_ob;
    t_zl     *p_master;
} t_zlproxy;

static t_class *zl_class;
static t_class *zlproxy_class;

static void zldata_init(t_zldata *d)
{
    d->d_size = ZL_INISIZE;
    d->d_natoms = 0;
    d->d_buf = d->d_bufini;
}

static void zldata_free(t_zldata *d)
{
    if (d->d_buf != d->d_bufini)
	freebytes(d->d_buf, d->d_size * sizeof(*d->d_buf));
}

static void zldata_setfloat(t_zldata *d, t_float f)
{
    SETFLOAT(d->d_buf, f);
    d->d_natoms = 1;
}

static void zldata_addfloat(t_zldata *d, t_float f)
{
    int natoms = d->d_natoms;
    int nrequested = natoms + 1;
    if (nrequested > d->d_size)
    {
	d->d_buf = grow_withdata(&nrequested, &natoms, &d->d_size,
				 d->d_buf, ZL_INISIZE, d->d_bufini,
				 sizeof(*d->d_buf));
	if (natoms >= nrequested)
	    natoms = nrequested - 1;
    }
    SETFLOAT(d->d_buf + natoms, f);
    d->d_natoms = natoms + 1;
}

static void zldata_setsymbol(t_zldata *d, t_symbol *s)
{
    SETSYMBOL(d->d_buf, s);
    d->d_natoms = 1;
}

static void zldata_addsymbol(t_zldata *d, t_symbol *s)
{
    int natoms = d->d_natoms;
    int nrequested = natoms + 1;
    if (nrequested > d->d_size)
    {
	d->d_buf = grow_withdata(&nrequested, &natoms, &d->d_size,
				 d->d_buf, ZL_INISIZE, d->d_bufini,
				 sizeof(*d->d_buf));
	if (natoms >= nrequested)
	    natoms = nrequested - 1;
    }
    SETSYMBOL(d->d_buf + natoms, s);
    d->d_natoms = natoms + 1;
}

static void zldata_setlist(t_zldata *d, int ac, t_atom *av)
{
    int nrequested = ac;
    if (nrequested > d->d_size)
	d->d_buf = grow_nodata(&nrequested, &d->d_size, d->d_buf,
			       ZL_INISIZE, d->d_bufini, sizeof(*d->d_buf));
    if (d->d_natoms = nrequested)
	memcpy(d->d_buf, av, nrequested * sizeof(*d->d_buf));
}

static void zldata_addlist(t_zldata *d, int ac, t_atom *av)
{
    int natoms = d->d_natoms;
    int nrequested = natoms + ac;
    if (nrequested > d->d_size)
    {
	d->d_buf = grow_withdata(&nrequested, &natoms, &d->d_size,
				 d->d_buf, ZL_INISIZE, d->d_bufini,
				 sizeof(*d->d_buf));
	if (natoms + ac > nrequested)
	{
	    natoms = nrequested - ac;
	    if (natoms < 0)
		natoms = 0, ac = nrequested;
	}
    }
    if (d->d_natoms = natoms + ac)
	memcpy(d->d_buf + natoms, av, ac * sizeof(*d->d_buf));
}

static void zldata_set(t_zldata *d, t_symbol *s, int ac, t_atom *av)
{
    if (s && s != &s_)
    {
	int nrequested = ac + 1;
	if (nrequested > d->d_size)
	    d->d_buf = grow_nodata(&nrequested, &d->d_size, d->d_buf,
				   ZL_INISIZE, d->d_bufini, sizeof(*d->d_buf));
	if (d->d_natoms = nrequested)
	{
	    SETSYMBOL(d->d_buf, s);
	    if (--nrequested)
		memcpy(d->d_buf + 1, av, nrequested * sizeof(*d->d_buf));
	}
    }
    else zldata_setlist(d, ac, av);
}

static void zldata_add(t_zldata *d, t_symbol *s, int ac, t_atom *av)
{
    if (s && s != &s_)
    {
	int natoms = d->d_natoms;
	int nrequested = natoms + 1 + ac;
	if (nrequested > d->d_size)
	{
	    d->d_buf = grow_withdata(&nrequested, &natoms, &d->d_size,
				     d->d_buf, ZL_INISIZE, d->d_bufini,
				     sizeof(*d->d_buf));
	    if (natoms + 1 + ac > nrequested)
	    {
		natoms = nrequested - 1 - ac;
		if (natoms < 0)
		    natoms = 0, ac = nrequested - 1;
	    }
	}
	if (d->d_natoms = natoms + 1 + ac)
	{
	    SETSYMBOL(d->d_buf + natoms, s);
	    if (ac > 0)
		memcpy(d->d_buf + natoms + 1, av, ac * sizeof(*d->d_buf));
	}
    }
    else zldata_addlist(d, ac, av);
}

/* LATER rethink */
static void zl_dooutput(t_outlet *o, int ac, t_atom *av)
{
    if (ac > 1)
    {
	if (av->a_type == A_FLOAT)
	    outlet_list(o, &s_list, ac, av);
	else if (av->a_type == A_SYMBOL)
	    outlet_anything(o, av->a_w.w_symbol, ac - 1, av + 1);
    }
    else if (ac)
    {
	if (av->a_type == A_FLOAT)
	    outlet_float(o, av->a_w.w_float);
	else if (av->a_type == A_SYMBOL)
#if 1
	    outlet_anything(o, av->a_w.w_symbol, 0, 0);  /* CHECKED */
#else
	    outlet_symbol(o, av->a_w.w_symbol);  /* LATER rethink */
#endif
    }
}

static void zl_output(t_zl *x, int ac, t_atom *av)
{
    zl_dooutput(((t_object *)x)->ob_outlet, ac, av);
}

static void zl_output2(t_zl *x, int ac, t_atom *av)
{
    zl_dooutput(x->x_out2, ac, av);
}

static int zl_equal(t_atom *ap1, t_atom *ap2)
{
    return (ap1->a_type == ap2->a_type
	    &&
	    ((ap1->a_type == A_FLOAT
	      && ap1->a_w.w_float == ap2->a_w.w_float)
	     ||
	     (ap1->a_type == A_SYMBOL
	      && ap1->a_w.w_symbol == ap2->a_w.w_symbol)));
}

/* Mode handlers:
   If zl_<mode>_count's return value is positve, then the main routine
   uses an output buffer 'buf' (outbuf, or a separately allocated one).
   If zl_<mode>_count's return value is zero, then the main routine is
   passed a null 'buf' (see below); if it is negative, then the main
   routine is not being called.
   zl_<mode> (main routine) arguments:  if 'buf' is null, 'natoms'
   is always zero -- in modes other than len (no buffer used), group,
   iter, reg, slice/ecils (inbuf1 used), there should be no output.
   If 'buf' is not null, then 'natoms' is guaranteed to be positive.
*/

static int zl_nop_count(t_zl *x)
{
    return (0);
}

static void zl_nop(t_zl *x, int natoms, t_atom *buf, int banged)
{
    loud_warning((t_pd *)x, 0, "unknown mode");
}

static int zl_ecils_intarg(t_zl *x, int i)
{
    return (i > 0 ? i : 0);  /* CHECKED */
}

static int zl_ecils_count(t_zl *x)
{
    return (x->x_entered ? -1 : 0);
}

static void zl_ecils(t_zl *x, int natoms, t_atom *buf, int banged)
{
    int cnt1, cnt2 = x->x_modearg;
    natoms = x->x_inbuf1.d_natoms;
    buf = x->x_inbuf1.d_buf;
    if (cnt2 > natoms)
	cnt2 = natoms, cnt1 = 0;  /* CHECKED */
    else
	cnt1 = natoms - cnt2;
    x->x_locked = 1;
    if (cnt2)
	zl_output2(x, cnt2, buf + cnt1);
    if (cnt1)
	zl_output(x, cnt1, buf);
}

static int zl_group_intarg(t_zl *x, int i)
{
    return (i > 0 ? i : 0);  /* CHECKED */
}

static int zl_group_count(t_zl *x)
{
    return (x->x_entered ? -1 : 0);
}

static void zl_group(t_zl *x, int natoms, t_atom *buf, int banged)
{
    int cnt = x->x_modearg;
    if (cnt > 0)
    {
	natoms = x->x_inbuf1.d_natoms;
	buf = x->x_inbuf1.d_buf;
	if (natoms >= cnt)
	{
	    t_atom *from;
	    x->x_locked = 1;
	    for (from = buf; natoms >= cnt; natoms -= cnt, from += cnt)
		zl_output(x, cnt, from);
	    x->x_inbuf1.d_natoms = natoms;
	    while (natoms--) *buf++ = *from++;
	}
	if (banged && x->x_inbuf1.d_natoms)
	{
	    zl_output(x, x->x_inbuf1.d_natoms, buf);
	    x->x_inbuf1.d_natoms = 0;
	}
    }
    else x->x_inbuf1.d_natoms = 0;  /* CHECKED */
}

static int zl_iter_intarg(t_zl *x, int i)
{
    return (i > 0 ? i : 0);  /* CHECKED */
}

static int zl_iter_count(t_zl *x)
{
    return (x->x_entered ?
	    (x->x_modearg < x->x_inbuf1.d_natoms ?
	     x->x_modearg : x->x_inbuf1.d_natoms)
	    : 0);
}

static void zl_iter(t_zl *x, int natoms, t_atom *buf, int banged)
{
    int nremaining = x->x_inbuf1.d_natoms;
    t_atom *ptr = x->x_inbuf1.d_buf;
    if (!buf)
    {
	if (natoms = (x->x_modearg < nremaining ?
		      x->x_modearg : nremaining))
	    x->x_locked = 1;
	else
	    return;
    }
    while (nremaining)
    {
	if (natoms > nremaining)
	    natoms = nremaining;
	if (buf)
	{
	    memcpy(buf, ptr, natoms * sizeof(*buf));
	    zl_output(x, natoms, buf);
	}
	else zl_output(x, natoms, ptr);
	nremaining -= natoms;
	ptr += natoms;
    }
}

static int zl_join_count(t_zl *x)
{
    return (x->x_inbuf1.d_natoms + x->x_inbuf2.d_natoms);
}

static void zl_join(t_zl *x, int natoms, t_atom *buf, int banged)
{
    if (buf)
    {
	int ac1 = x->x_inbuf1.d_natoms, ac2 = x->x_inbuf2.d_natoms;
	if (ac1)
	    memcpy(buf, x->x_inbuf1.d_buf, ac1 * sizeof(*buf));
	if (ac2)
	    memcpy(buf + ac1, x->x_inbuf2.d_buf, ac2 * sizeof(*buf));
	zl_output(x, natoms, buf);
    }
}

static int zl_len_count(t_zl *x)
{
    return (0);
}

static void zl_len(t_zl *x, int natoms, t_atom *buf, int banged)
{
/* CHECKED 'mode len, bang'->[zl]->[print] crashes max 4.0.7... */
    if (!banged)  /* CHECKED bang is a nop in len mode */
	outlet_float(((t_object *)x)->ob_outlet, x->x_inbuf1.d_natoms);
}

static int zl_nth_intarg(t_zl *x, int i)
{
    return (i > 0 ? i : 0);  /* CHECKED */
}

static void zl_nth_anyarg(t_zl *x, t_symbol *s, int ac, t_atom *av)
{
    if (!s && ac && av->a_type == A_FLOAT)
	zldata_setlist(&x->x_inbuf2, ac - 1, av + 1);
}

static int zl_nth_count(t_zl *x)
{
    int ac1 = x->x_inbuf1.d_natoms;
    if (ac1)
    {
	if (x->x_modearg > 0)
	    return (ac1 - 1 + x->x_inbuf2.d_natoms);
	else
	    return (x->x_entered ? ac1 : 0);
    }
    else return (-1);
}

static void zl_nth(t_zl *x, int natoms, t_atom *buf, int banged)
{
    int ac1 = x->x_inbuf1.d_natoms,
	ndx = x->x_modearg - 1;  /* CHECKED one-based */
    if (ac1 && ndx < ac1)  /* CHECKED */
    {
	t_atom *av1 = x->x_inbuf1.d_buf;
	if (ndx < 0)
	{
	    if (buf) memcpy(buf, av1, ac1 * sizeof(*buf));
	    else
	    {
		buf = av1;
		x->x_locked = 1;
	    }
	    zl_output2(x, ac1, buf);
	}
	else
	{
	    t_atom at = av1[ndx];
	    if (buf)
	    {
		int ac2 = x->x_inbuf2.d_natoms, ntail = ac1 - ndx - 1;
		t_atom *ptr = buf;
		if (ndx)
		{
		    memcpy(ptr, av1, ndx * sizeof(*buf));
		    ptr += ndx;
		}
		if (ac2)  /* replacement */
		{
		    memcpy(ptr, x->x_inbuf2.d_buf, ac2 * sizeof(*buf));
		    ptr += ac2;
		}
		if (ntail)
		    memcpy(ptr, av1 + ndx + 1, ntail * sizeof(*buf));
		zl_output2(x, natoms, buf);
	    }
	    zl_output(x, 1, &at);
	}
    }
}

static void zl_reg_anyarg(t_zl *x, t_symbol *s, int ac, t_atom *av)
{
    if (!x->x_locked)
	zldata_set(&x->x_inbuf1, s, ac, av);
}

static int zl_reg_count(t_zl *x)
{
    return (x->x_entered ? x->x_inbuf1.d_natoms : 0);
}

static void zl_reg(t_zl *x, int natoms, t_atom *buf, int banged)
{
    if (buf) memcpy(buf, x->x_inbuf1.d_buf, natoms * sizeof(*buf));
    else
    {
	natoms = x->x_inbuf1.d_natoms;
	buf = x->x_inbuf1.d_buf;
	x->x_locked = 1;
    }
    if (natoms)
	zl_output(x, natoms, buf);
}

static int zl_rev_count(t_zl *x)
{
    return (x->x_inbuf1.d_natoms);
}

static void zl_rev(t_zl *x, int natoms, t_atom *buf, int banged)
{
    if (buf)
    {
	t_atom *from = x->x_inbuf1.d_buf, *to = buf + natoms;
	while (to-- > buf)
	    *to = *from++;
	zl_output(x, natoms, buf);
    }
}

static int zl_rot_intarg(t_zl *x, int i)
{
    return (i);  /* CHECKED anything goes (modulo) */
}

static int zl_rot_count(t_zl *x)
{
    return (x->x_inbuf1.d_natoms);
}

static void zl_rot(t_zl *x, int natoms, t_atom *buf, int banged)
{
    if (buf)
    {
	int cnt1 = x->x_modearg, cnt2;
	if (cnt1)
	{
	    if (cnt1 > 0)
	    {
		cnt1 %= natoms;
		cnt2 = natoms - cnt1;
	    }
	    else
	    {
		cnt2 = -cnt1 % natoms;
		cnt1 = natoms - cnt2;
	    }
	    /* CHECKED right rotation for positive args */
	    memcpy(buf, x->x_inbuf1.d_buf + cnt2, cnt1 * sizeof(*buf));
	    memcpy(buf + cnt1, x->x_inbuf1.d_buf, cnt2 * sizeof(*buf));
	}
	else memcpy(buf, x->x_inbuf1.d_buf, natoms * sizeof(*buf));
	zl_output(x, natoms, buf);
    }
}

/* LATER rethink */
static int zl_sect_count(t_zl *x)
{
    int result = 0;
    int ac1 = x->x_inbuf1.d_natoms, ac2 = x->x_inbuf2.d_natoms, i1;
    t_atom *av1 = x->x_inbuf1.d_buf, *av2 = x->x_inbuf2.d_buf, *ap1;
    for (i1 = 0, ap1 = av1; i1 < ac1; i1++, ap1++)
    {
	int i2;
	t_atom *testp;
	for (i2 = 0, testp = av1; i2 < i1; i2++, testp++)
	    if (zl_equal(ap1, testp))
		goto skip;
	for (i2 = 0, testp = av2; i2 < ac2; i2++, testp++)
	{
	    if (zl_equal(ap1, testp))
	    {
		result++;
		break;
	    }
	}
    skip:;
    }
    return (result);
}

/* CHECKED in-buffer duplicates are skipped */
static void zl_sect(t_zl *x, int natoms, t_atom *buf, int banged)
{
    if (buf)
    {
	int ac1 = x->x_inbuf1.d_natoms, ac2 = x->x_inbuf2.d_natoms, i1;
	t_atom *ap1 = x->x_inbuf1.d_buf, *av2 = x->x_inbuf2.d_buf, *to = buf;
	for (i1 = 0; i1 < ac1; i1++, ap1++)
	{
	    int i2;
	    t_atom *testp;
	    for (testp = buf; testp < to; testp++)
		if (zl_equal(ap1, testp))
		    goto skip;
	    for (i2 = 0, testp = av2; i2 < ac2; i2++, testp++)
	    {
		if (zl_equal(ap1, testp))
		{
		    *to++ = *ap1;
		    break;
		}
	    }
	skip:;
	}
	zl_output(x, natoms, buf);
    }
}

static int zl_slice_intarg(t_zl *x, int i)
{
    return (i > 0 ? i : 0);  /* CHECKED */
}

static int zl_slice_count(t_zl *x)
{
    return (x->x_entered ? -1 : 0);
}

static void zl_slice(t_zl *x, int natoms, t_atom *buf, int banged)
{
    int cnt1 = x->x_modearg, cnt2;
    natoms = x->x_inbuf1.d_natoms;
    buf = x->x_inbuf1.d_buf;
    if (cnt1 > natoms)
	cnt1 = natoms, cnt2 = 0;  /* CHECKED */
    else
	cnt2 = natoms - cnt1;
    x->x_locked = 1;
    if (cnt2)
	zl_output2(x, cnt2, buf + cnt1);
    if (cnt1)
	zl_output(x, cnt1, buf);
}

static int zl_sub_count(t_zl *x)
{
    return (0);
}

static void zl_sub(t_zl *x, int natoms, t_atom *buf, int banged)
{
    int natoms2 = x->x_inbuf2.d_natoms;
    if (natoms2)
    {
	int ndx1, natoms1 = x->x_inbuf1.d_natoms;
	t_atom *av1 = x->x_inbuf1.d_buf, *av2 = x->x_inbuf2.d_buf;
	for (ndx1 = 0; ndx1 < natoms1; ndx1++, av1++)
	{
	    int ndx2;
	    t_atom *ap1 = av1, *ap2 = av2;
	    for (ndx2 = 0; ndx2 < natoms2; ndx2++, ap1++, ap2++)
		if (!zl_equal(ap1, ap2))
		    break;
	    if (ndx2 == natoms2)
		/* CHECKED output position is zero-based */
		outlet_float(((t_object *)x)->ob_outlet, ndx1);
	}
    }
}

/* LATER rethink */
static int zl_union_count(t_zl *x)
{
    int result, ac1 = x->x_inbuf1.d_natoms, ac2 = x->x_inbuf2.d_natoms, i2;
    t_atom *av1 = x->x_inbuf1.d_buf, *ap2 = x->x_inbuf2.d_buf;
    result = ac1 + ac2;
    for (i2 = 0; i2 < ac2; i2++, ap2++)
    {
	int i1;
	t_atom *ap1;
	for (i1 = 0, ap1 = av1; i1 < ac1; i1++, ap1++)
	{
	    if (zl_equal(ap1, ap2))
	    {
		result--;
		break;
	    }
	}
    }
    return (result);
}

/* CHECKED in-buffer duplicates not skipped */
static void zl_union(t_zl *x, int natoms, t_atom *buf, int banged)
{
    if (buf)
    {
	int ac1 = x->x_inbuf1.d_natoms, ac2 = x->x_inbuf2.d_natoms, i2;
	t_atom *av1 = x->x_inbuf1.d_buf, *ap2 = x->x_inbuf2.d_buf;
	if (ac1)
	{
	    t_atom *to = buf + ac1;
	    memcpy(buf, av1, ac1 * sizeof(*buf));
	    for (i2 = 0; i2 < ac2; i2++, ap2++)
	    {
		int i1;
		t_atom *ap1;
		for (i1 = 0, ap1 = av1; i1 < ac1; i1++, ap1++)
		    if (zl_equal(ap1, ap2))
			break;
		if (i1 == ac1)
		    *to++ = *ap2;
	    }
	}
	else memcpy(buf, ap2, ac2 * sizeof(*buf));
	zl_output(x, natoms, buf);
    }
}

static void zl_doit(t_zl *x, int banged)
{
    int reentered = x->x_entered;
    int prealloc = !reentered;
    int natoms = (*zl_natomsfn[x->x_mode])(x);
    if (natoms < 0)
	return;
    x->x_entered = 1;
    if (natoms)
    {
	t_zldata *d = &x->x_outbuf;
	t_atom *buf;
	if (prealloc && natoms > d->d_size)
	{
	    if (natoms > ZL_MAXSIZE)
		prealloc = 0;
	    else
	    {
		int nrequested = natoms;
		d->d_buf = grow_nodata(&nrequested, &d->d_size, d->d_buf,
				       ZL_INISIZE, d->d_bufini,
				       sizeof(*d->d_buf));
		if (nrequested != natoms)
		    prealloc = 0;
	    }
	}
	/* LATER consider using the stack if !prealloc && natoms <= MAXSTACK */
	if (buf = (prealloc ? d->d_buf : getbytes(natoms * sizeof(*buf))))
	{
	    (*zl_doitfn[x->x_mode])(x, natoms, buf, banged);
	    if (buf != d->d_buf)
		freebytes(buf, natoms * sizeof(*buf));
	}
    }
    else (*zl_doitfn[x->x_mode])(x, 0, 0, banged);
    if (!reentered)
	x->x_entered = x->x_locked = 0;
}

static void zl_bang(t_zl *x)
{
    zl_doit(x, 1);
}

static void zl_float(t_zl *x, t_float f)
{
    if (!x->x_locked)
    {
	if (zl_modeflags[x->x_mode])
	    zldata_addfloat(&x->x_inbuf1, f);
	else
	    zldata_setfloat(&x->x_inbuf1, f);
    }
    zl_doit(x, 0);
}

static void zl_symbol(t_zl *x, t_symbol *s)
{
    if (!x->x_locked)
    {
	if (zl_modeflags[x->x_mode])
	    zldata_addsymbol(&x->x_inbuf1, s);
	else
	    zldata_setsymbol(&x->x_inbuf1, s);
    }
    zl_doit(x, 0);
}

/* LATER gpointer */

static void zl_list(t_zl *x, t_symbol *s, int ac, t_atom *av)
{
    if (!x->x_locked)
    {
	if (zl_modeflags[x->x_mode])
	    zldata_addlist(&x->x_inbuf1, ac, av);
	else
	    zldata_setlist(&x->x_inbuf1, ac, av);
    }
    zl_doit(x, 0);
}

static void zl_anything(t_zl *x, t_symbol *s, int ac, t_atom *av)
{
    if (!x->x_locked)
    {
	if (zl_modeflags[x->x_mode])
	    zldata_add(&x->x_inbuf1, s, ac, av);
	else
	    zldata_set(&x->x_inbuf1, s, ac, av);
    }
    zl_doit(x, 0);
}

static int zl_modeargfn(t_zl *x)
{
    return (zl_intargfn[x->x_mode] || zl_anyargfn[x->x_mode]);
}

static void zl_setmodearg(t_zl *x, t_symbol *s, int ac, t_atom *av)
{
    if (zl_intargfn[x->x_mode])
    {
	int i = (!s && ac && av->a_type == A_FLOAT ?
		 (int)av->a_w.w_float :  /* CHECKED silent truncation */
		 0);  /* CHECKED current x->x_modearg not kept */
	x->x_modearg = (*zl_intargfn[x->x_mode])(x, i);
    }
    if (zl_anyargfn[x->x_mode])
	(*zl_anyargfn[x->x_mode])(x, s, ac, av);
}

static void zl_mode(t_zl *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac && av->a_type == A_SYMBOL)
    {
	t_symbol *modesym = av->a_w.w_symbol;
	int i;
	for (i = 0; i < zl_nmodes; i++)
	    if (modesym == zl_modesym[i])
		break;
	/* LATER consider making this compatible:
	   CHECKED setting unknown mode makes a zl nop */
	if (i && i < zl_nmodes)
	{
	    x->x_mode = i;
	    /* CHECKED incompatible (LATER warn):
	       c74 rejects creation args, if not a single int */
	    zl_setmodearg(x, 0, ac - 1, av + 1);
	}
    }
}

static void zlproxy_bang(t_zlproxy *d)
{
    /* CHECKED a nop */
}

static void zlproxy_float(t_zlproxy *p, t_float f)
{
    t_zl *x = p->p_master;
    if (zl_modeargfn(x))
    {
	t_atom at;
	SETFLOAT(&at, f);
	zl_setmodearg(x, 0, 1, &at);
    }
    else  /* CHECKED inbuf2 filled only when used */
	zldata_setfloat(&x->x_inbuf2, f);
}

static void zlproxy_symbol(t_zlproxy *p, t_symbol *s)
{
    t_zl *x = p->p_master;
    if (zl_modeargfn(x))
    {
	t_atom at;
	SETSYMBOL(&at, s);
	zl_setmodearg(x, 0, 1, &at);
    }
    else  /* CHECKED inbuf2 filled only when used */
	zldata_setsymbol(&x->x_inbuf2, s);
}

/* LATER gpointer */

static void zlproxy_list(t_zlproxy *p, t_symbol *s, int ac, t_atom *av)
{
    if (ac)
    {
	t_zl *x = p->p_master;
	if (zl_modeargfn(x))
	    zl_setmodearg(x, 0, ac, av);
	else  /* CHECKED inbuf2 filled only when used */
	    zldata_setlist(&x->x_inbuf2, ac, av);
    }
}

static void zlproxy_anything(t_zlproxy *p, t_symbol *s, int ac, t_atom *av)
{
    t_zl *x = p->p_master;
    if (zl_modeargfn(x))
	zl_setmodearg(x, s, ac, av);
    else  /* CHECKED inbuf2 filled only when used */
	zldata_set(&x->x_inbuf2, s, ac, av);
}

#ifdef ZL_DEBUG
static void zl_debug(t_zl *x, t_floatarg f)
{
    loudbug_startpost("mode %s", zl_modesym[x->x_mode]->s_name);
    if (zl_intargfn[x->x_mode])
	loudbug_post(" %d", x->x_modearg);
    else
	loudbug_endpost();
    if ((int)f)
    {
	loudbug_startpost("first:");
	loudbug_postatom(x->x_inbuf1.d_natoms, x->x_inbuf1.d_buf);
	loudbug_endpost();
	loudbug_startpost("second:");
	loudbug_postatom(x->x_inbuf2.d_natoms, x->x_inbuf2.d_buf);
	loudbug_endpost();
    }
}
#endif

static void zl_free(t_zl *x)
{
    zldata_free(&x->x_inbuf1);
    zldata_free(&x->x_inbuf2);
    zldata_free(&x->x_outbuf);
    if (x->x_proxy) pd_free((t_pd *)x->x_proxy);
}

static void *zl_new(t_symbol *s, int ac, t_atom *av)
{
    t_zl *x = (t_zl *)pd_new(zl_class);
    t_zlproxy *y = (t_zlproxy *)pd_new(zlproxy_class);
    x->x_proxy = y;
    y->p_master = x;
    x->x_entered = 0;
    x->x_locked = 0;
    zldata_init(&x->x_inbuf1);
    zldata_init(&x->x_inbuf2);
    zldata_init(&x->x_outbuf);
    x->x_mode = ZL_DEFMODE;
    zl_mode(x, s, ac, av);
    inlet_new((t_object *)x, (t_pd *)y, 0, 0);
    outlet_new((t_object *)x, &s_anything);
    x->x_out2 = outlet_new((t_object *)x, &s_anything);
    return (x);
}

static void zl_setupmode(char *id, int flags,
			 t_zlintargfn ifn, t_zlanyargfn afn,
			 t_zlnatomsfn nfn, t_zldoitfn dfn)
{
    if (zl_nmodes < ZL_MAXMODES)
    {
	zl_modesym[zl_nmodes] = gensym(id);
	zl_modeflags[zl_nmodes] = flags;
	zl_intargfn[zl_nmodes] = ifn;
	zl_anyargfn[zl_nmodes] = afn;
	zl_natomsfn[zl_nmodes] = nfn;
	zl_doitfn[zl_nmodes] = dfn;
	zl_nmodes++;
    }
    else loudbug_bug("zl_setupmode");
}

static void zl_setupallmodes(void)
{
    zl_setupmode("unknown", 0, 0, 0, zl_nop_count, zl_nop);
    zl_setupmode("ecils", 0, zl_ecils_intarg, 0, zl_ecils_count, zl_ecils);
    zl_setupmode("group", 1, zl_group_intarg, 0, zl_group_count, zl_group);
    zl_setupmode("iter", 0, zl_iter_intarg, 0, zl_iter_count, zl_iter);
    zl_setupmode("join", 0, 0, 0, zl_join_count, zl_join);
    zl_setupmode("len", 0, 0, 0, zl_len_count, zl_len);
    zl_setupmode("nth", 0, zl_nth_intarg, zl_nth_anyarg, zl_nth_count, zl_nth);
    zl_setupmode("reg", 0, 0, zl_reg_anyarg, zl_reg_count, zl_reg);
    zl_setupmode("rev", 0, 0, 0, zl_rev_count, zl_rev);
    zl_setupmode("rot",  /* CHECKED (refman's error) */
		 0, zl_rot_intarg, 0, zl_rot_count, zl_rot);
    zl_setupmode("sect", 0, 0, 0, zl_sect_count, zl_sect);
    zl_setupmode("slice", 0, zl_slice_intarg, 0, zl_slice_count, zl_slice);
    zl_setupmode("sub", 0, 0, 0, zl_sub_count, zl_sub);
    zl_setupmode("union", 0, 0, 0, zl_union_count, zl_union);
}

void zl_setup(void)
{
    zl_class = class_new(gensym("zl"),
			 (t_newmethod)zl_new,
			 (t_method)zl_free,
			 sizeof(t_zl), 0,
			 A_GIMME, 0);
    class_addbang(zl_class, zl_bang);
    class_addfloat(zl_class, zl_float);
    class_addsymbol(zl_class, zl_symbol);
    class_addlist(zl_class, zl_list);
    class_addanything(zl_class, zl_anything);
    class_addmethod(zl_class, (t_method)zl_mode,
		    gensym("mode"), A_GIMME, 0);
#ifdef ZL_DEBUG
    class_addmethod(zl_class, (t_method)zl_debug,
		    gensym("debug"), A_DEFFLOAT, 0);
#endif
    zlproxy_class = class_new(gensym("_zlproxy"), 0, 0,
			      sizeof(t_zlproxy),
			      CLASS_PD | CLASS_NOINLET, 0);
    class_addbang(zlproxy_class, zlproxy_bang);
    class_addfloat(zlproxy_class, zlproxy_float);
    class_addsymbol(zlproxy_class, zlproxy_symbol);
    class_addlist(zlproxy_class, zlproxy_list);
    class_addanything(zlproxy_class, zlproxy_anything);
    zl_setupallmodes();
}
