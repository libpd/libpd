/* Copyright 2002 Benjamin R. Saylor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "m_pd.h"

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

enum { ATTACK, DECAY, RELEASE, LINEAR, LOGARITHMIC };
#define THRESHOLD 0.99
#define SCALE 4600.0
#define CLIP(x) ((x < 0.1) ? 0.1 : x)
#define IS_DENORMAL(f) (((*(unsigned int *)&(f))&0x7f800000) == 0)

static t_class *aenv_class;

typedef struct _aenv {
	t_object x_obj;
	t_float srate;
	t_float a;
	t_float d;
	t_float s;
	t_float r;
	t_float lastval;
	int stage;
	int attacktype;
} t_aenv;

static t_int *aenv_perform(t_int *w)
{
	t_aenv *x = (t_aenv *)(w[1]);
	t_float *out = (t_float *)(w[2]);
	int n = (int)(w[3]);

	t_float lastval = x->lastval;
	t_float a, d, s, r;

	switch (x->stage) {
		case ATTACK:
			if (x->attacktype == LINEAR) {
				a = 1000 / (x->a * x->srate);
				while (n--) {
					*out = lastval + a;
					lastval = *(out++);
					if (lastval > THRESHOLD) {
						x->stage = DECAY;
						break;
					}
				}
			} else {
				a = SCALE / (CLIP(x->a) * x->srate);
				while (n--) {
					*out = lastval + a * (1 - lastval);
					lastval = *(out++);
					if (lastval > THRESHOLD) {
						x->stage = DECAY;
						break;
					}
				}
			}
		case DECAY:
			d = SCALE / (CLIP(x->d) * x->srate);
			s = x->s;
			while (n-- > 0) {
				*out = lastval - d * (lastval - s);
				if (IS_DENORMAL(*out)) *out = 0.0;
				lastval = *(out++);
			}
			break;
		case RELEASE:
			r = SCALE / (CLIP(x->r) * x->srate);
			while (n--) {
				*out = lastval - r * lastval;
				if (IS_DENORMAL(*out)) *out = 0.0;
				lastval = *(out++);
			}
	}

	x->lastval = lastval;

	return (w+4);
}

static void aenv_dsp(t_aenv *x, t_signal **sp)
{
	dsp_add(aenv_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}

static void aenv_float(t_aenv *x, t_float f)
{
	if (f == 0)
		x->stage = RELEASE;
	else
		x->stage = ATTACK;
}

static void *aenv_new(t_symbol *s, int argc, t_atom *argv)
{
	t_aenv *x = (t_aenv *)pd_new(aenv_class);
	floatinlet_new(&x->x_obj, &x->a);
	floatinlet_new(&x->x_obj, &x->d);
	floatinlet_new(&x->x_obj, &x->s);
	floatinlet_new(&x->x_obj, &x->r);
	outlet_new(&x->x_obj, gensym("signal"));

	x->srate = sys_getsr();
	x->a = 500;
	x->d = 500;
	x->s = 0.5;
	x->r = 500;
	x->lastval = 0;
	x->stage = RELEASE;
	x->attacktype = LOGARITHMIC;

	if (argc == 4) {
		x->a = atom_getfloat(argv);
		x->d = atom_getfloat(argv+1);
		x->s = atom_getfloat(argv+2);
		x->r = atom_getfloat(argv+3);
	}

	return (x);
}

static void aenv_lina(t_aenv *x)
{
	x->attacktype = LINEAR;
}

static void aenv_loga(t_aenv *x)
{
	x->attacktype = LOGARITHMIC;
}

static void aenv_zero(t_aenv *x)
{
	x->stage = RELEASE;
	x->lastval = 0.0;
}

void aenv_tilde_setup(void)
{
	aenv_class = class_new(gensym("aenv~"), (t_newmethod)aenv_new, 0, sizeof(t_aenv), 0, A_GIMME, 0);
	class_addmethod(aenv_class, (t_method)aenv_dsp, gensym("dsp"), 0);
	class_addfloat(aenv_class, (t_method)aenv_float);
	class_addmethod(aenv_class, (t_method)aenv_lina, gensym("lina"), 0);
	class_addmethod(aenv_class, (t_method)aenv_loga, gensym("loga"), 0);
	class_addmethod(aenv_class, (t_method)aenv_zero, gensym("zero"), 0);
}
