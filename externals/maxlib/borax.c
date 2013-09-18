/* ------------------------- borax   ------------------------------------------ */
/*                                                                              */
/* "swiss army knife" for music analysis. Inspired by 'borax' for Max.          */
/* Written by Olaf Matthes (olaf.matthes@gmx.de)                                */
/* Get source at http://www.akustische-kunst.org/puredata/maxlib/               */
/*                                                                              */
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* This program is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/* GNU General Public License for more details.                                 */
/*                                                                              */
/* You should have received a copy of the GNU General Public License            */
/* along with this program; if not, write to the Free Software                  */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  */
/*                                                                              */
/* Based on PureData by Miller Puckette and others.                             */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

#include "m_pd.h"

#define MAX_POLY 128                /* maximum number of notes played at a time */

static char *version = "borax v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";
 
typedef struct borax
{
  t_object x_ob;
  t_inlet  *x_invelo;               /* inlet for velocity */
  t_inlet  *x_inreset;              /* inlet to reset the object */
  t_outlet *x_outnotecount;         /* counts notes */
  t_outlet *x_outvoicealloc;        /* assigns every note a unique number */
  t_outlet *x_outpoly;              /* number of notes playing (polyphony) */
  t_outlet *x_outpitch;             /* pitch of current note */
  t_outlet *x_outvelo;              /* velocity of current note */
  t_outlet *x_outdurcount;          /* number assigned to duration value */
  t_outlet *x_outdurval;            /* duration value */
  t_outlet *x_outtimecount;         /* number assigned to delta time value */
  t_outlet *x_outtimeval;           /* delta time value */


  t_float  x_notecount;
  t_int    x_pitch;
  t_int    x_velo;
  t_float  x_voicecount;
  t_int    x_voicealloc;
  t_int    x_poly;
  t_float  x_durcount;
  t_float  x_durval;
  t_float  x_timecount;
  t_float  x_timeval;
	/* helpers needed to do the calculations */
  double   x_starttime[MAX_POLY];
  double   x_laststarttime;
  t_int    x_alloctable[MAX_POLY];

} t_borax;

static void borax_float(t_borax *x, t_floatarg f)
{
	t_int velo = x->x_velo;
	t_int allloc = 0;
	int i;

	x->x_pitch = (t_int)f;

	if(velo == 0)
	{
			/* note off received... */
		if(x->x_poly > 0)x->x_poly--; /* polyphony has decreased by one */
		x->x_durcount++;              /* we can calculate the duration */
		for(i = 0; i < MAX_POLY; i++) /* search for voice allocation number */
		{
				/* search for corresponding alloc number */
			if(x->x_alloctable[i] == x->x_pitch)
			{
				x->x_voicealloc = i;
				x->x_alloctable[i] = 0;     /* free the alloc number */
				break;
			}
				/* couldn't find it ? */
			if(i == MAX_POLY - 1)
			{
				post("borax: no corresponding note-on found (ignored)");
				return;
			}
		}
		x->x_durval = clock_gettimesince(x->x_starttime[x->x_voicealloc]);
	}
	else if(velo != 0)
	{
			/* note on received... */
		x->x_poly++;                  /* number of currently playing notes has increased */
		x->x_notecount++;             /* total number of notes has increased */
			/* assign a voice allocation number */
		for(i = 0; i < MAX_POLY; i++)
		{
				/* search for free alloc number */
			if(x->x_alloctable[i] == 0)
			{
				x->x_voicealloc = i;               /* take the number */
				x->x_alloctable[i] = x->x_pitch;   /* ... and store pitch */
				break;
			}
				/* couldn't find any ? */
			if(i == MAX_POLY - 1)
			{
				post("borax: too many note-on messages (ignored)");
				return;
			}
		}
			/* calculate time in case it's not the first note */
		if(x->x_notecount > 1)
		{
			x->x_timecount++;
			x->x_timeval = clock_gettimesince(x->x_laststarttime);
		}
			/* save the new start time */
		x->x_laststarttime = x->x_starttime[x->x_voicealloc] = clock_getlogicaltime();
	}
		/* output values from right to left */
	outlet_float(x->x_outtimeval, x->x_timeval);
	outlet_float(x->x_outtimecount, x->x_timecount);
	outlet_float(x->x_outdurval, x->x_durval);
	outlet_float(x->x_outdurcount, x->x_durcount);
	outlet_float(x->x_outvelo, velo);
	outlet_float(x->x_outpitch, x->x_pitch);
	outlet_float(x->x_outpoly, x->x_poly);
	outlet_float(x->x_outvoicealloc, x->x_voicealloc);
	outlet_float(x->x_outnotecount, x->x_notecount);
}

static void borax_ft1(t_borax *x, t_floatarg f)
{
	x->x_velo = (t_int)f;
}

static void borax_reset(t_borax *x)
{
	int i;
	post("borax: reset");
	x->x_notecount = 0;
	x->x_pitch = 0;
	x->x_velo = 0;
	x->x_voicecount = 0;
	x->x_voicealloc = 0;
	x->x_poly = 0;
	x->x_durcount = 0;
	x->x_durval = 0;
	x->x_timecount = 0;
	x->x_timeval = 0;
	outlet_float(x->x_outtimeval, x->x_timeval);
	outlet_float(x->x_outtimecount, x->x_timecount);
	outlet_float(x->x_outdurval, x->x_durval);
	outlet_float(x->x_outdurcount, x->x_durcount);
	for(i = 0; i < MAX_POLY; i++)
	{	
		if(x->x_alloctable[i] != 0)
		{
			x->x_poly--;
				/* send note-off */
			outlet_float(x->x_outvelo, 0);
			outlet_float(x->x_outpitch, x->x_alloctable[i]);
			outlet_float(x->x_outpoly, x->x_poly);
			outlet_float(x->x_outvoicealloc, i);
		}
		x->x_alloctable[i] = 0;
	}
	outlet_float(x->x_outvelo, x->x_velo);
	outlet_float(x->x_outpitch, x->x_pitch);
	outlet_float(x->x_outpoly, x->x_poly);
	outlet_float(x->x_outvoicealloc, x->x_voicealloc);
	outlet_float(x->x_outnotecount, x->x_notecount);
}

static t_class *borax_class;

static void *borax_new(void)
{
	int i;

    t_borax *x = (t_borax *)pd_new(borax_class);
    x->x_invelo = inlet_new(&x->x_ob, &x->x_ob.ob_pd, gensym("float"), gensym("ft1"));
    x->x_inreset = inlet_new(&x->x_ob, &x->x_ob.ob_pd, gensym("bang"), gensym("ft2"));
	x->x_outnotecount = outlet_new(&x->x_ob, gensym("float"));
	x->x_outvoicealloc = outlet_new(&x->x_ob, gensym("float"));
	x->x_outpoly = outlet_new(&x->x_ob, gensym("float"));
	x->x_outpitch = outlet_new(&x->x_ob, gensym("float"));
	x->x_outvelo = outlet_new(&x->x_ob, gensym("float"));
	x->x_outdurcount = outlet_new(&x->x_ob, gensym("float"));
	x->x_outdurval = outlet_new(&x->x_ob, gensym("float"));
	x->x_outtimecount = outlet_new(&x->x_ob, gensym("float"));
	x->x_outtimeval = outlet_new(&x->x_ob, gensym("float"));

	for(i = 0; i < MAX_POLY; i++)x->x_alloctable[i] = 0;
	x->x_notecount = 0;
	x->x_pitch = 0;
	x->x_velo = 0;
	x->x_voicecount = 0;
	x->x_voicealloc = 0;
	x->x_poly = 0;
	x->x_durcount = 0;
	x->x_durval = 0;
	x->x_timecount = 0;
	x->x_timeval = 0;

    return (void *)x;
}

#ifndef MAXLIB
void borax_setup(void)
{
    borax_class = class_new(gensym("borax"), (t_newmethod)borax_new,
    	0, sizeof(t_borax), 0, 0);
#else
void maxlib_borax_setup(void)
{
    borax_class = class_new(gensym("maxlib_borax"), (t_newmethod)borax_new,
    	0, sizeof(t_borax), 0, 0);
#endif
    class_addmethod(borax_class, (t_method)borax_reset, gensym("reset"), 0);
    class_addmethod(borax_class, (t_method)borax_ft1, gensym("ft1"), A_FLOAT, 0);
    class_addmethod(borax_class, (t_method)borax_reset, gensym("ft2"), A_GIMME, 0);
    class_addfloat(borax_class, borax_float);
#ifndef MAXLIB
    
    logpost(NULL, 4, version);
#else
	class_addcreator((t_newmethod)borax_new, gensym("borax"), 0);
    class_sethelpsymbol(borax_class, gensym("maxlib/borax-help.pd"));
#endif
}

