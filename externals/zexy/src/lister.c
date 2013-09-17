/* 
 * lister:  this is for lists, what "float" is for floats  (use [list]  instead)
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

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#endif

#define LIST_NGETBYTE 100 /* bigger that this we use alloc, not alloca */


static t_class *mypdlist_class;

#ifdef HAVE_ALLOCA_H
# define ATOMS_ALLOCA(x, n) ((x) = (t_atom *)((n) < LIST_NGETBYTE ?  \
        alloca((n) * sizeof(t_atom)) : getbytes((n) * sizeof(t_atom))))
# define ATOMS_FREEA(x, n) ( \
    ((n) < LIST_NGETBYTE || (freebytes((x), (n) * sizeof(t_atom)), 0)))
#else
# define ATOMS_ALLOCA(x, n) ((x) = (t_atom *)getbytes((n) * sizeof(t_atom)))
# define ATOMS_FREEA(x, n) (freebytes((x), (n) * sizeof(t_atom)))
#endif

static void atoms_copy(int argc, t_atom *from, t_atom *to)
{
  int i;
  for (i = 0; i < argc; i++)
    to[i] = from[i];
}


static void mypdlist_storelist(t_mypdlist *x, int argc, t_atom *argv)
{
  if(x->x_list)freebytes(x->x_list, x->x_n*sizeof(t_atom));
  x->x_n=argc;
  x->x_list=(t_atom*)getbytes(x->x_n*sizeof(t_atom));

  atoms_copy(argc, argv, x->x_list);
}
static void mypdlist_secondlist(t_mypdlist *x, t_symbol *s, int argc, t_atom *argv)
{
  mypdlist_storelist(x, argc, argv);
}

static void mypdlist_bang(t_mypdlist *x)
{ 
  int outc=x->x_n;
  t_atom*outv;
  ATOMS_ALLOCA(outv, outc);
  atoms_copy(x->x_n, x->x_list, outv);
  outlet_list(x->x_obj.ob_outlet, gensym("list"), outc, outv);
  ATOMS_FREEA(outv, outc);
}


static void mypdlist_list(t_mypdlist *x, t_symbol *s, int argc, t_atom *argv)
{
  mypdlist_secondlist(x, s, argc, argv);
  mypdlist_bang(x);
}


static void mypdlist_free(t_mypdlist *x)
{ freebytes(x->x_list, x->x_n * sizeof(t_atom)); }

static void *mypdlist_new(t_symbol *s, int argc, t_atom *argv)
{
  t_mypdlist *x = (t_mypdlist *)pd_new(mypdlist_class);

  outlet_new(&x->x_obj, 0);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("list"), gensym("lst2"));

  x->x_n = 0;
  x->x_list = 0;

  if(argc)
    mypdlist_secondlist(x, gensym("list"), argc, argv);

  return (x);
}


static void mypdlist_help(t_mypdlist*x)
{
  post("\n%c lister\t\t:: basic list storage (use pd>=0.39 for real [list] objects)", HEARTSYMBOL);
}

void lister_setup(void)
{
  mypdlist_class = class_new(gensym("lister"), (t_newmethod)mypdlist_new, 
                             (t_method)mypdlist_free, sizeof(t_mypdlist), 0, A_GIMME, 0);
  /* i don't know how to get this work with name=="list" !!! */

  class_addcreator((t_newmethod)mypdlist_new, gensym("l"), A_GIMME, 0);

  class_addbang    (mypdlist_class, mypdlist_bang);
  class_addlist    (mypdlist_class, mypdlist_list);
  class_addmethod  (mypdlist_class, (t_method)mypdlist_secondlist, gensym("lst2"), A_GIMME, 0);

  class_addmethod(mypdlist_class, (t_method)mypdlist_help, gensym("help"), A_NULL);
  zexy_register("lister");
}
void l_setup(void)
{
  lister_setup();
}
