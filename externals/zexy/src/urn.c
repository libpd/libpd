/* 
 * urn :  "generate random numbers without duplicates" (very max-like)
 *
 * (c) 1999-2011 IOhannes m zmölnig, forum::für::umläute, institute of electronic music and acoustics (iem)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "zexy.h"

/* ------------------------- urn ------------------------------- */

static t_class *urn_class;

typedef struct _urn
{
  t_object x_obj;
  unsigned int x_seed;       /* the seed of the generator */

  unsigned int x_range;      /* max. random-number + 1 */
  unsigned int x_count;      /* how many random numbers have we generated ? */
  char  *x_state;            /* has this number been generated already ? */

  t_outlet *x_floatout, *x_bangout;
  char x_noauto;
} t_urn;

static int makeseed(void)
{
  static unsigned int random_nextseed = 1489853723;
  random_nextseed = random_nextseed * 435898247 + 938284287;
  return (random_nextseed & 0x7fffffff);
}

static void makestate(t_urn *x, unsigned int newrange)
{
  if (x->x_range == newrange)return;

  if (x->x_range && x->x_state) {
    freebytes(x->x_state, sizeof(char)*x->x_range);
    x->x_state=0;
  }

  x->x_range=newrange;
  x->x_state=getbytes(sizeof(char)*x->x_range);
}

static void urn_clear(t_urn *x)
{
  unsigned int i=x->x_range;
  char *dummy=x->x_state;
  if (!dummy || !i)return;
  while(i--)*dummy++=0;
  x->x_count=0;
}
static void urn_bang(t_urn *x)
{
  unsigned int range = (x->x_range<1?1:x->x_range);
  unsigned int randval = x->x_seed;

  unsigned int nval=0, used=1;

  if (x->x_count>=range){
    outlet_bang(x->x_bangout);
    if (x->x_noauto)return;
    urn_clear(x);
  }

  while (used) {
    randval = randval * 472940017 + 832416023;
    nval = ((double)range) * ((double)randval)
    	* (1./4294967296.);
    if (nval >= range) nval = range-1;
    used=x->x_state[nval];
  }

  x->x_count++;
  x->x_state[nval]=1;
  x->x_seed = randval;
  outlet_float(x->x_floatout, nval);
}

static void urn_flt2(t_urn *x, t_float f)
{
  unsigned int range = (f<1)?1:f;
  makestate(x, range);
  urn_clear(x);
}


static void urn_seed(t_urn *x, t_float f)
{
  x->x_seed = f;
}

static void *urn_new(t_symbol *s, int argc, t_atom *argv)
{
  t_urn *x = (t_urn *)pd_new(urn_class);
  t_float f=0.;
  ZEXY_USEVAR(s);

  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym(""));
  x->x_floatout=outlet_new(&x->x_obj, gensym("float"));
  x->x_bangout =outlet_new(&x->x_obj, gensym("bang"));

  x->x_seed = makeseed();
  x->x_noauto = 0;

  while(argc--){
    if (argv->a_type==A_SYMBOL) {
      if (atom_getsymbol(argv)==gensym("no_auto")) {
	x->x_noauto=1;
      }
    } else f = atom_getfloat(argv);
    argv++;
  }

  if (f<1.0)f=1.0;
  makestate(x, f);
  x->x_range = f;
  urn_clear(x);

  return (x);
}

static void urn_help(t_urn*x)
{
  post("\n%c urn\t\t:: generate randum numbers without repetition", HEARTSYMBOL);
}

void urn_setup(void)
{
  urn_class = class_new(gensym("urn"), (t_newmethod)urn_new, 
			      0, sizeof(t_urn), 0, A_GIMME,  0);
  
  class_addbang (urn_class, urn_bang);
  class_addmethod(urn_class, (t_method)urn_clear, gensym("clear"), 0);
  class_addmethod(urn_class, (t_method)urn_flt2, gensym(""), A_DEFFLOAT, 0);
  class_addmethod(urn_class, (t_method)urn_seed, gensym("seed"), A_DEFFLOAT, 0);
  
  class_addmethod(urn_class, (t_method)urn_help, gensym("help"), A_NULL);

  zexy_register("urn");
}
