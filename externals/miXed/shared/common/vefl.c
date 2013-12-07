/* Copyright (c) 1997-2003 Miller Puckette, krzYszcz, and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* The simplest of garrays: vector of floats */

/* Array checking is done in three points:
   1. vefl_new(): never complains
   2. vefl_renew(): this should be called once per every message
   3. vefl_tick(): no template checking (only redraw is involved)
*/

/* LATER rethink indsp flag */

#include "m_pd.h"
#include "g_canvas.h"
#include "shared.h"
#include "unstable/fragile.h"
#include "common/loud.h"
#include "vefl.h"

#ifdef KRZYSZCZ
//#define VEFL_DEBUG
#endif
#define VEFL_VERBOSE

/* on failure *vszp is not modified */
t_float *vefl_get(t_symbol *name, int *vszp, int indsp, t_pd *complain)
{
    if (name && name != &s_)
    {
	t_garray *ap = (t_garray *)pd_findbyclass(name, garray_class);
	if (ap)
	{
	    int vsz;
	    t_float *vec;
	    if (garray_getfloatarray(ap, &vsz, &vec))
	    {
		if (indsp) garray_usedindsp(ap);
		if (vszp) *vszp = vsz;
		return (vec);
	    }
	    else loud_error(complain,  /* always complain */
			    "bad template of array '%s'", name->s_name);
	}
	else if (complain)
	    loud_error(complain, "no such array '%s'", name->s_name);
    }
    return (0);
}

static void vefl_tick(t_vefl *vp)
{
    if (vp->v_name && vp->v_name != &s_
	/* Check if an array has not been deleted
	   (e.g. during patch closing sequence). */
	&& (vp->v_garray =
	    (t_garray *)pd_findbyclass(vp->v_name, garray_class)))
    {
	vp->v_glist = fragile_garray_glist(vp->v_garray);
	garray_redraw(vp->v_garray);
    }
    vp->v_clockset = 0;
    vp->v_updtime = clock_getsystime();
}

t_vefl *vefl_placement_new(t_vefl *vp, t_symbol *name,
			   int writable, t_glist *gl, t_garray *arr)
{
    if (sizeof(t_word) != sizeof(t_float))
    {
	loudbug_bug("vefl_new: sizeof(t_word) != sizeof(t_float)");
	return (0);
    }
    if (!vp)
    {
	if (!(vp = getbytes(sizeof(*vp))))
	    return (0);
	vp->v_autoalloc = 1;
    }
    else vp->v_autoalloc = 0;
    vp->v_name = name;
    if (writable)
    {
	vp->v_updtime = clock_getsystime();
	vp->v_clock = clock_new(vp, (t_method)vefl_tick);
	vp->v_clockset = 0;
    }
    else vp->v_clock = 0;
    vp->v_glist = gl;
    vp->v_garray = arr;
    vp->v_size = 0;
    vp->v_data = 0;
    vp->v_type = &s_float;
    if (!arr && name && name != &s_)
    {
	vp->v_garray = (t_garray *)pd_findbyclass(name, garray_class);
	vp->v_glist = vp->v_garray ? fragile_garray_glist(vp->v_garray) : 0;
    }
    if (vp->v_garray
	&& !garray_getfloatarray(vp->v_garray, &vp->v_size, &vp->v_data))
    {
	vp->v_glist = 0;
	vp->v_garray = 0;
	vp->v_type = 0;  /* template mismatch */
    }
    return (vp);
}

t_vefl *vefl_new(t_symbol *name, int writable, t_glist *gl, t_garray *arr)
{
    return (vefl_placement_new(0, name, writable, gl, arr));    
}

void vefl_free(t_vefl *vp)
{
    if (vp->v_clock) clock_free(vp->v_clock);
    if (vp->v_autoalloc) freebytes(vp, sizeof(*vp));
}

/* LATER handle yonset */
int vefl_renew(t_vefl *vp, t_symbol *name, t_pd *complain)
{
    if (!name || name == &s_) name = vp->v_name;
    if (name && name != &s_)
    {
	vp->v_glist = 0;
	/* There are three possible ways: */
#if 0
	vp->v_name = 0;
#elif 1  /* , do nothing, and */
	vp->v_name = name;
#endif  /* LATER check all the cases and decide... */
	if (!(vp->v_garray = (t_garray *)pd_findbyclass(name, garray_class)))
	{
	    if (complain)
		loud_error(complain, "no such array '%s'", name->s_name);
	}
	else if (!garray_getfloatarray(vp->v_garray, &vp->v_size, &vp->v_data))
	{
	    vp->v_garray = 0;
	    loud_error(complain,  /* always complain */
		       "bad template of array '%s'", name->s_name);
	}
	else
	{
	    vp->v_glist = fragile_garray_glist(vp->v_garray);
	    vp->v_name = name;
	    return (1);
	}
    }
    return (0);
}

void vefl_redraw(t_vefl *vp, float suppresstime)
{
    if (vp->v_clock)  /* requests from readers are ignored */
    {
	if (suppresstime > 0)
	{
	    double timesince = clock_gettimesince(vp->v_updtime);
	    if (timesince > suppresstime)
	    {
		clock_unset(vp->v_clock);
		vefl_tick(vp);
	    }
	    else if (!vp->v_clockset)
	    {
		clock_delay(vp->v_clock, suppresstime - timesince);
		vp->v_clockset = 1;
	    }
	}
	else {
	    clock_unset(vp->v_clock);
	    vefl_tick(vp);
	}
    }
}

void vefl_redraw_stop(t_vefl *vp)
{
    if (vp->v_clock)  /* requests from readers are ignored */
    {
	clock_unset(vp->v_clock);
	vp->v_clockset = 0;
    }
}

/* Y-bounds flipped here */
void vefl_getbounds(t_vefl *vp, t_float *xminp, t_float *yminp,
		    t_float *xmaxp, t_float *ymaxp)
{
    t_glist *gl = vp->v_glist;
    if (gl)
    {
	*xminp = gl->gl_x1;
	*xmaxp = gl->gl_x2;
	*yminp = gl->gl_y2;
	*ymaxp = gl->gl_y1;
    }
}

/* Y-bounds flipped here */
void vefl_setbounds(t_vefl *vp, t_float xmin, t_float ymin,
		    t_float xmax, t_float ymax)
{
    vmess((t_pd *)vp->v_glist, gensym("bounds"), "ffff",
	  xmin, ymax, xmax, ymin);
}

void vefl_getrange(t_vefl *vp, t_float *yminp, t_float *ymaxp)
{
    int vsz = vp->v_size;
    t_float *vec = vp->v_data;
    if (vec && vsz)
    {
	t_float ymin = SHARED_FLT_MAX, ymax = -SHARED_FLT_MAX;
	while (vsz--)
	{
	    if (*vec > ymax)
	    {
		ymax = *vec;
		if (ymax < ymin) ymin = ymax;
	    }
	    else if (*vec < ymin) ymin = *vec;
	    vec++;
	}
	*yminp = ymin;
	*ymaxp = ymax;
    }
}
