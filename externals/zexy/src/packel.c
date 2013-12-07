/* 
 * packel:  get the nth element of a package
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

static t_class *packel_class;

typedef struct _packel
{
  t_object x_obj;
  t_float *position;
  int count;

  t_inlet**x_inlet;
  t_outlet**x_outlet;

  int x_warningflag;
} t_packel;


static void packel_outelement(t_packel*x, int id, t_symbol*s,int argc, t_atom*argv)
{
  t_outlet*out=x->x_outlet[id];
  int index= x->position[id];
 
  if (index) {
    t_atom *current;
    int pos = (index < 0)?(argc+index):(index-1);

    if(argc==0){
      if (pos==0||pos==-1)outlet_bang(out);
      return;
    }
    
    if (pos < 0 || pos >= argc)return;

    current = &(argv[pos]);

    switch (current->a_type) {
    case A_NULL:
      outlet_bang(out);
    default:
      outlet_list(out, gensym("list"), 1, current);
    }
  } else outlet_list(out, s, argc, argv); 
}

static void packel_list(t_packel *x, t_symbol *s, int argc, t_atom *argv)
{
  int c=x->count;
  while(--c>=0) {
    packel_outelement(x, c, s, argc, argv);
  }
}

static void packel_anything(t_packel *x, t_symbol *s, int argc, t_atom *argv)
{
  t_atom *av2 = (t_atom *)getbytes((argc + 1) * sizeof(t_atom));
  int i;

  if(x->x_warningflag){
    pd_error(x, "deprecation warning: you should only use lists for list data");
    x->x_warningflag=0;
  }

  for (i = 0; i < argc; i++)
    av2[i + 1] = argv[i];
  SETSYMBOL(av2, s);
  packel_list(x, gensym("list"), argc+1, av2);
  freebytes(av2, (argc + 1) * sizeof(t_atom));
}


static void packel_free(t_packel *x)
{
  int i=0;

  for(i=0; i<x->count; i++) {
    if(x->x_inlet &&x->x_inlet [i])inlet_free (x->x_inlet [i]);
    if(x->x_outlet&&x->x_outlet[i])outlet_free(x->x_outlet[i]);
  }

  if(x->position)freebytes(x->position, x->count*sizeof(t_float));
  if(x->x_inlet) freebytes(x->x_inlet, x->count*sizeof(t_inlet*));
  if(x->x_outlet)freebytes(x->x_outlet, x->count*sizeof(t_outlet*));

}


static void *packel_new(t_symbol*s, int argc, t_atom*argv)
{
  t_packel *x = (t_packel *)pd_new(packel_class);
  
  x->count=(argc>0)?argc:1;

  x->position=(t_float*)getbytes(x->count*sizeof(t_float));
  x->x_inlet=(t_inlet**)getbytes(x->count*sizeof(t_inlet*));
  x->x_outlet=(t_outlet**)getbytes(x->count*sizeof(t_outlet*));

  if(argc<1) {
    x->position[0]=0.f;
    x->x_inlet[0]=floatinlet_new(&x->x_obj, x->position);
    x->x_outlet[0]=outlet_new(&x->x_obj, 0);
  } else {
    int i=0;
    for(i=0; i<x->count; i++) {
      x->position[i]=atom_getfloat(argv+i);
      x->x_inlet   [i]=floatinlet_new(&x->x_obj, x->position+i);
      x->x_outlet  [i]=outlet_new(&x->x_obj, 0);
    }
  }
  x->x_warningflag=1;


  return (x);
}

void packel_setup(void)
{
  packel_class = class_new(gensym("packel"), 
                           (t_newmethod)packel_new, (t_method)packel_free, 
                           sizeof(t_packel), 0,
                           A_GIMME, 0);

  class_addlist  (packel_class, packel_list);
  class_addanything(packel_class, packel_anything);

  zexy_register("packel");
}
