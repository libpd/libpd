/* ---------------------------- iso ------------------------------------------- */
/*                                                                              */
/* Queue up pitch and attack point series.                                      */
/* Written by Olaf Matthes (olaf.matthes@gmx.de)                                */
/* Based on iso for Max by Charlie Baker (baker@foxtrot.ccmrc.ucsb.edu).        */
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

#define MAXPOLY 32

static char *version = "iso v0.1, written for Max by Charlie Baker <baker@foxtrot.ccmrc.ucsb.edu>\n"
                       "          ported to Pd by Olaf Matthes <olaf.matthes@gmx.de>";

/* Iso object data structure */

typedef struct iso
{
	t_object iso_ob;		
	t_outlet *iso_out1;			/* outlet 1*/
	t_outlet *iso_out2;			/* outlet 2*/
	t_inlet *iso_in2;           /* inlet 2 (attack list) */
	t_clock *iso_clock;
	t_int ptchlength,atklength,curptch,curatk;
	t_float pitches[MAXPOLY];   
	t_float atks[MAXPOLY];
	t_int loop,stop;
	t_float hook,duty;
} t_iso;

static t_class *iso_class;

/* take list and create matrix */

static void iso_bang(t_iso *x)
{
	x->stop = 0;
	x->curptch = 0;
	x->curatk = 0;
	clock_delay(x->iso_clock, 0);
}


static void iso_clock_fun(t_iso *x)
{
	if (!x->stop) {
		clock_delay(x->iso_clock, (double)(x->atks[x->curatk] * x->hook)); 
		outlet_float(x->iso_out2,(t_float)(x->atks[x->curatk] * x->hook * x->duty)); 
		outlet_float(x->iso_out1,x->pitches[x->curptch] );
		if (x->loop) {
				x->curatk = ((x->curatk + 1) % x->atklength);
				x->curptch = ((x->curptch + 1) % x->ptchlength);
		}
		else {
			if (((x->curatk + 1) >= x->atklength) || ((x->curptch + 1) >= x->ptchlength)) 
				 x->stop = 1;
			else { 
				x->curptch += 1;
				x->curatk += 1;
			}
		}
	}
}

static void iso_hook(t_iso *x, t_floatarg hook)
{
	if (hook < 1.0) hook = 1.0;
	x->hook = (t_float)hook;
}
	
static void iso_duty(t_iso *x, t_floatarg duty)
{
	if (duty < 1.0) duty = 1.0;
	x->duty = (t_float)duty;
}

static void iso_list(t_iso *x, t_symbol *s, t_int argc, t_atom* argv)
{
	int i;
	if (argc > MAXPOLY) post("iso: only %d values max. allowed in list!", MAXPOLY);
	for (i = 0; i < argc; i++) x->atks[i] = argv[i].a_w.w_float;
	x->atklength = argc;
}

static void iso_pitch(t_iso *x, t_symbol *s, t_int argc, t_atom* argv)
{
	int i;
	if (argc > MAXPOLY) post("iso: only %d values max. allowed in list!", MAXPOLY);
	for (i = 0; i < argc; i++)  x->pitches[i] = argv[i].a_w.w_float;
	x->ptchlength = argc;
}

static void iso_start(t_iso *x, t_symbol *s, t_int argc, t_atom* argv)
{
	t_int start = atom_getfloatarg(0, argc, argv);
	x->stop = 0;
	if (start) {
		x->curptch = (t_int)((start - 1) % x->ptchlength);
		x->curatk = (t_int)((start - 1) % x->atklength);
			   }
	else	{
		x->curptch = 0;
		x->curatk = 0;
	}
	clock_delay(x->iso_clock, 0);
}

static void iso_stop(t_iso *x)
{
	x->stop = 1;
	x->curatk = 0;
	x->curptch = 0;
}

static void iso_pause(t_iso *x)
{
	x->stop = 1;
}

static void iso_loop(t_iso *x)
{
	x->loop = 1;
}

static void iso_resume(t_iso *x)
{
	x->stop = 0;
	clock_delay(x->iso_clock, 0);
}

static void iso_unloop(t_iso *x)
{
	x->loop = 0;
}


static void *iso_new(void) {
	t_iso *x = (t_iso *)pd_new(iso_class);	/* allocates memory and sticks in an inlet */
	x->iso_clock = clock_new(x, (t_method)iso_clock_fun);
	x->iso_out1 = outlet_new(&x->iso_ob, gensym("float"));
	x->iso_out2 = outlet_new(&x->iso_ob, gensym("float"));
	x->iso_in2 = inlet_new(&x->iso_ob, &x->iso_ob.ob_pd, gensym("list"), gensym("attack"));
	x->stop = 0;
	x->loop = 1;
	x->hook = 1.0;
	x->curptch = 0;
	x->curatk = 0;
	x->ptchlength = 1;
	x->atklength = 1;
	x->pitches[0] = 60;
	x->atks[0] = 500;
	x->duty = 1.0;

	return (x);					/* always return a copy of the created object */
}

static void iso_free(t_iso *x) {

	clock_free(x->iso_clock);
}

#ifndef MAXLIB
void iso_setup(void) {

    iso_class = class_new(gensym("iso"), (t_newmethod)iso_new,
    	(t_method)iso_free, sizeof(t_iso), 0, 0);
    class_addmethod(iso_class, (t_method)iso_duty, gensym("duty"), A_FLOAT, 0);
	class_addmethod(iso_class, (t_method)iso_list, gensym("attack"), A_GIMME, 0);
    class_addmethod(iso_class, (t_method)iso_start, gensym("start"), A_GIMME, 0);
    class_addmethod(iso_class, (t_method)iso_stop, gensym("stop"), 0);
    class_addmethod(iso_class, (t_method)iso_pause, gensym("pause"), 0);
    class_addmethod(iso_class, (t_method)iso_loop, gensym("loop"), 0);
    class_addmethod(iso_class, (t_method)iso_unloop, gensym("unloop"), 0);
    class_addmethod(iso_class, (t_method)iso_resume, gensym("resume"), 0);
    class_addmethod(iso_class, (t_method)iso_hook, gensym("hook"), A_FLOAT, 0);
    class_addbang(iso_class, iso_bang);
	class_addlist(iso_class, iso_pitch);
    
	logpost(NULL, 4, version);
}
#else
void maxlib_iso_setup(void) {

    iso_class = class_new(gensym("maxlib_iso"), (t_newmethod)iso_new,
    	(t_method)iso_free, sizeof(t_iso), 0, 0);
	class_addcreator((t_newmethod)iso_new, gensym("iso"), 0);
    class_addmethod(iso_class, (t_method)iso_duty, gensym("duty"), A_FLOAT, 0);
	class_addmethod(iso_class, (t_method)iso_list, gensym("attack"), A_GIMME, 0);
    class_addmethod(iso_class, (t_method)iso_start, gensym("start"), A_GIMME, 0);
    class_addmethod(iso_class, (t_method)iso_stop, gensym("stop"), 0);
    class_addmethod(iso_class, (t_method)iso_pause, gensym("pause"), 0);
    class_addmethod(iso_class, (t_method)iso_loop, gensym("loop"), 0);
    class_addmethod(iso_class, (t_method)iso_unloop, gensym("unloop"), 0);
    class_addmethod(iso_class, (t_method)iso_resume, gensym("resume"), 0);
    class_addmethod(iso_class, (t_method)iso_hook, gensym("hook"), A_FLOAT, 0);
    class_addbang(iso_class, iso_bang);
	class_addlist(iso_class, iso_pitch);
    class_sethelpsymbol(iso_class, gensym("maxlib/iso-help.pd"));
}
#endif
