/* ------------------------- pitch   ------------------------------------------ */
/*                                                                              */
/* Get a lot of info about an incoming pitch (class, register, interval...).    */
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
#include <stdio.h>

static char *version = "pitch v0.1b, written by Olaf Matthes <olaf.matthes@gmx.de>";
 
typedef struct pitch
{
  t_object x_ob;
  t_inlet  *x_inpitch;              /* inlet for pitch */
  t_outlet *x_outpitchval;          /* pitch as MIDI note number */
  t_outlet *x_outpitchname;         /* pitch name, e.g. "C1" */
  t_outlet *x_outpitchclass;        /* pitch class */
  t_outlet *x_outintv;              /* interval */
  t_outlet *x_outregister;          /* register */

  t_int    x_lastpitch;

} t_pitch;

static void pitch_float(t_pitch *x, t_floatarg f) {

	char buf[8];
	int r, c, interval = 0, pitch;
	
	char* notes_up[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
	char* notes_down[12] = {"C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"};

	pitch = (t_int)f;
	if(pitch < 1) pitch = 0;
	if(pitch > 127) pitch = 127;

	if(x->x_lastpitch != 0)interval = pitch - x->x_lastpitch;
	x->x_lastpitch = pitch;

	r = (pitch / 12) - 1;
	c = pitch % 12;
	if(interval >= 0)
	{
		sprintf(buf, "%s%d", notes_up[c], r);
	}
	else
	{
		sprintf(buf, "%s%d", notes_down[c], r);
	}
	// post("note: %s %d", notes[c], r);

		/* output values from right to left */
	outlet_float(x->x_outregister, r);
	outlet_float(x->x_outintv, interval);
	outlet_float(x->x_outpitchclass, c);
	outlet_symbol(x->x_outpitchname, gensym(buf));
	outlet_float(x->x_outpitchval, pitch);
}

static t_class *pitch_class;

static void *pitch_new(t_floatarg f)
{
    t_pitch *x = (t_pitch *)pd_new(pitch_class);
    x->x_inpitch = inlet_new(&x->x_ob, &x->x_ob.ob_pd, gensym("float"), gensym("ft1"));
	x->x_outpitchval = outlet_new(&x->x_ob, gensym("float"));
	x->x_outpitchname = outlet_new(&x->x_ob, gensym("symbol"));
	x->x_outpitchclass = outlet_new(&x->x_ob, gensym("float"));
	x->x_outintv = outlet_new(&x->x_ob, gensym("float"));
	x->x_outregister = outlet_new(&x->x_ob, gensym("float"));

	x->x_lastpitch = f;

    return (void *)x;
}

#ifndef MAXLIB
void pitch_setup(void)
{
    pitch_class = class_new(gensym("pitch"), (t_newmethod)pitch_new,
    	0, sizeof(t_pitch), 0, A_DEFFLOAT, 0);
    class_addfloat(pitch_class, pitch_float);
    
    logpost(NULL, 4, version);
}
#else
void maxlib_pitch_setup(void)
{
    pitch_class = class_new(gensym("maxlib_pitch"), (t_newmethod)pitch_new,
    	0, sizeof(t_pitch), 0, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)pitch_new, gensym("pitch"), A_DEFFLOAT, 0);
    class_addfloat(pitch_class, pitch_float);
    class_sethelpsymbol(pitch_class, gensym("maxlib/pitch-help.pd"));
}
#endif
