/* -------------------------  sync  ------------------------------------------- */
/*                                                                              */
/* syncronises outputs depending on inputs.                                     */
/* Written by Olaf Matthes <olaf.matthes@gmx.de>                                */
/* Based on 'sync' from jMax.                                                   */
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
#include <stdio.h>
#include <math.h>

#define SYNC_MAX_SIZE (int)(sizeof(unsigned int) * 8)

static char *version = "sync v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>\n"
                       "           based on sync from jMax";

typedef struct sync
{
  t_object x_ob;
  t_float  x_min;                  /* low border of input range */
  t_float  x_max;                  /* high border of input range */
  t_outlet *x_outlet[SYNC_MAX_SIZE];

  t_int        x_n;
  unsigned int x_trigger; /* control bits: trigger on input at given inlets */
  unsigned int x_require; /* control bits: require input on given inlets */
  unsigned int x_reset; /* control bits: reset memory of given inputs after on each input */
  unsigned int x_wait; /* status bits: wait for input at given inlet before output */
  t_atom       x_a[SYNC_MAX_SIZE];
  enum {mode_all, mode_select} mode;
} t_sync;

typedef struct proxy
{
	t_object obj;
	t_int index;		/* number of proxy inlet(s) */
	t_sync *x;		/* we'll put the other struct in here */
} t_proxy;

static void sync_output(t_sync *x)
{
	int i;

	for(i=x->x_n-1; i>=0; i--)
		if(x->x_a[i].a_type != A_SEMI)
			outlet_list(x->x_outlet[i], NULL, 1, x->x_a + i);
}

static void sync_input(t_proxy *p, t_symbol *s, int ac, t_atom *at)
{
	t_sync *x = (t_sync *)(p->x);
	int winlet = p->index;

	if(ac)
	{
		unsigned int bit = 1 << winlet;

		x->x_a[winlet] = at[0];

		x->x_wait &= ~bit;

		if(!x->x_wait && (x->x_trigger & bit))
		{
			sync_output(x);
			x->x_wait |= x->x_reset & x->x_require;
		}
	}
}

static void sync_float_input(t_proxy *p, t_floatarg f)
{
	t_sync *x = (t_sync *)(p->x);
	int winlet = p->index;

	{
		unsigned int bit = 1 << winlet;

		SETFLOAT(x->x_a + winlet, f);

		x->x_wait &= ~bit;

		if(!x->x_wait && (x->x_trigger & bit))
		{
			sync_output(x);
			x->x_wait |= x->x_reset & x->x_require;
		}
	}
}

static void sync_set_bits(unsigned int *bits, int n, t_atom *at, int sign)
{
	if(at->a_type == A_SYMBOL)
	{
		t_symbol *mode = atom_getsymbol(at);

		if(mode == gensym("all"))
			*bits = (1 << n) - 1;
		else if(mode == gensym("none"))
			*bits = 0;
	}
	else if(at->a_type == A_FLOAT)
	{
		int in = (int)atom_getfloat(at) * sign;

		if(in >= 0 && in < n)
			*bits = 1 << in;
	}
	else if(at->a_type == A_GIMME)
	{
		int size = n;
		int i;

		*bits = 0;

		for(i=0; i<size; i++)
		{
			if(at[i].a_type == A_FLOAT)
			{
				int in = atom_getfloatarg(i, size, at) * sign;

				if(in >= 0 && in < n)
				*bits |= 1 << in;
			}  
		}
	}
}

static void sync_set_trigger(t_sync *x, t_symbol *s, int ac, t_atom *at)
{
	sync_set_bits(&x->x_trigger, x->x_n, at, 1);
}

static void sync_set_require(t_sync *x, t_symbol *s, int ac, t_atom *at)
{
  unsigned int once = 0;

  sync_set_bits(&x->x_require, x->x_n, at, 1);
  sync_set_bits(&once, x->x_n, at, -1);

  x->x_reset = ~once;
  x->x_wait = x->x_require | once;
}

static void sync_set_mode(t_sync *x, t_symbol *mode)
{
	if(mode == gensym("any"))
	{
		x->x_trigger = (1 << x->x_n) - 1;
		x->x_reset = 0;
		x->x_require = 0;
	}
	else if(mode == gensym("all"))
		x->x_trigger = x->x_require = x->x_reset = x->x_wait = (1 << x->x_n) - 1;
	else if(mode == gensym("left"))
	{
		x->x_trigger = 1;
		x->x_reset = 0;
		x->x_require = 0;
	}
	else if(mode == gensym("right"))
	{
		x->x_trigger = (1 << (x->x_n - 1));
		x->x_reset = 0;
		x->x_require = 0;
	}

	x->x_wait = x->x_require;
}

static void sync_float(t_sync *x, t_floatarg f)
{
	unsigned int bit = 1 << 0;

	SETFLOAT(x->x_a, f);

	x->x_wait &= ~bit;

	if(!x->x_wait && (x->x_trigger & bit))
	{
		sync_output(x);
		x->x_wait |= x->x_reset & x->x_require;
	}
}

static t_class *sync_class;
static t_class *proxy_class;

static void *sync_new(t_symbol *s, int ac, t_atom *at)
{
	int n = 0;
	int i;
    t_sync *x = (t_sync *)pd_new(sync_class);
	t_proxy *inlet[SYNC_MAX_SIZE];

		/* void state - we fill with SEMI and treat this as 'void' */
	for(i=0; i<SYNC_MAX_SIZE; i++)
		SETSEMI(x->x_a + i);

	if(ac == 1)
	{
		if(at->a_type == A_FLOAT)
		{
			n = atom_getfloat(at);

			if(n < 2) 
				n = 2;
			else if(n > SYNC_MAX_SIZE)
				n = SYNC_MAX_SIZE;
		}
		else
		{
			post("sync: wrong argument");
			return (0);
		}
	}
	else if(ac > 1)
	{
		if(ac > SYNC_MAX_SIZE)
			ac = SYNC_MAX_SIZE;

		n = ac;

		for(i=0; i<n; i++)
			x->x_a[i] = at[i];
	}

	x->x_n = n;
	x->x_trigger = x->x_require = x->x_reset = x->x_wait = (1 << n) - 1;

	x->x_outlet[0] = outlet_new(&x->x_ob, gensym("list"));

	for(i=1; i<n; i++)
	{
		inlet[i] = (t_proxy *)pd_new(proxy_class);	/* create the proxy inlet */
		inlet[i]->x = x;		/* make t_sync *x visible to the proxy inlets */
		inlet[i]->index = i;	/* remember it's number */
			/* it belongs to the object t_sync but the destination is t_proxy */
		inlet_new(&x->x_ob, &inlet[i]->obj.ob_pd, 0,0);

		x->x_outlet[i] = outlet_new(&x->x_ob, gensym("list"));
	}

    return (void *)x;
}

#ifndef MAXLIB
void sync_setup(void)
{
    sync_class = class_new(gensym("sync"), (t_newmethod)sync_new,
    	0, sizeof(t_sync), 0, A_GIMME, 0);
#else
void maxlib_sync_setup(void)
{
    sync_class = class_new(gensym("maxlib_sync"), (t_newmethod)sync_new,
    	0, sizeof(t_sync), 0, A_GIMME, 0);
#endif
		/* a class for the proxy inlet: */
	proxy_class = class_new(gensym("maxlib_sync_proxy"), NULL, NULL, sizeof(t_proxy),
		CLASS_PD|CLASS_NOINLET, A_NULL);

	class_addfloat(proxy_class, sync_float_input);
	class_addanything(proxy_class, sync_input);

    class_addfloat(sync_class, sync_float);
	class_addmethod(sync_class, (t_method)sync_set_trigger, gensym("trigger"), A_GIMME, 0);
	class_addmethod(sync_class, (t_method)sync_set_require, gensym("require"), A_GIMME, 0);
	class_addmethod(sync_class, (t_method)sync_set_mode, gensym("mode"), A_SYMBOL, 0);
#ifndef MAXLIB
    
    logpost(NULL, 4, version);
#else
	class_addcreator((t_newmethod)sync_new, gensym("sync"), A_GIMME, 0);
    class_sethelpsymbol(sync_class, gensym("maxlib/sync-help.pd"));
#endif
}

