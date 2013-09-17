/* 
 * listfind: find a sublist in a list and return the index of the occurence (or indices if there are more)
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

#if 0
# define DEBUG
#endif

#ifdef DEBUG
# define DEBUGFUN(x) x
#else
# define DEBUGFUN(x)
#endif

static t_class *listfind_class;


typedef struct _listfind
{
  t_object       x_obj;
  t_outlet      *x_outlet;

  int x_n;

  t_inlet*x_listin;
  int     x_argc;
  t_atom *x_argv;
} t_listfind;



static void listfind_list2(t_listfind*x,t_symbol*s, int argc, t_atom*argv)
{
  if(x->x_argv!=0) {
    freebytes(x->x_argv, sizeof(t_atom)*x->x_argc);
  }
  x->x_argc=0;
  x->x_argv=0;

  DEBUGFUN(post("list of length %d", argc));

  if(argc>0) {
    int i;
    x->x_argc=argc;
    x->x_argv=(t_atom*)getbytes((x->x_argc)*sizeof(t_atom));
    for(i=0; i<argc; i++) {
      x->x_argv[i]=argv[i];
    }
  }

  DEBUGFUN(post("list2: %d %x", x->x_argc, x->x_argv));
}

static int atom_equals(t_atom*a1, t_atom*a2) {
  if(a1->a_type!=a2->a_type) return 0;

  return(a1->a_w.w_symbol==a2->a_w.w_symbol);  
}

static int list_equals(int count, t_atom*a1, t_atom*a2) {
  int i=0;
  DEBUGFUN(post("list(%d) equals?", count));
  DEBUGFUN(postatom(count, a1));
  DEBUGFUN(endpost());
  DEBUGFUN(postatom(count, a2));
  DEBUGFUN(endpost());
  DEBUGFUN(endpost());

  for(i=0; i<count; i++, a1++, a2++) {
    if(a1->a_type!=a2->a_type) {
      DEBUGFUN(post("atomtypes do not match!"));
      return 0;
    }
    if(A_FLOAT==a1->a_type) {
      if(atom_getfloat(a1)!=atom_getfloat(a2)) {
	return 0;
      }
    } else 
      if(a1->a_w.w_symbol!=a2->a_w.w_symbol) { /* is it that simple? */
      DEBUGFUN(post("atom values do not match: %x != %x", 
		    a1->a_w.w_symbol, 
		    a2->a_w.w_symbol
		    ));
      return 0;
    }
  }
  DEBUGFUN(post("lists match"));
  return 1;
}

static int listfind_find(int argc, t_atom*argv, int matchc, t_atom*matchv) {
  int i=0;

  DEBUGFUN(post("match: %d vs %d elements", argc, matchc));

  if(matchc>argc) {
    DEBUGFUN(post("list find -1"));

     return -1;
  }
  if(matchc==0) {
  DEBUGFUN(post("list find 0"));

    return 0;
  }

  for(i=0; i<=(argc-matchc); i++, argv++) {
    DEBUGFUN(post("checking at %d", i));
    if(list_equals(matchc, argv, matchv))
      return i;
  }
  return -1;
}

static void listfind_doit(t_outlet*out, int longcount, t_atom*longlist, int patterncount, t_atom*patternlist) {
  int count=0;
  int index;
  int offset=0;

  t_atom*ap=0;
  int length=1+((patterncount>0)?(longcount/patterncount):longcount); /* we shan't have more hits than this! */
  if(length<1) {
    outlet_bang(out);
  }
  ap=(t_atom*)getbytes(length*sizeof(t_atom));

  DEBUGFUN(post("expecting no more than %d results", length));

  while((index=listfind_find(longcount-offset, longlist+offset, patterncount, patternlist))>=0) {
    offset+=index;
    SETFLOAT(ap+count, offset);

    count++;

    DEBUGFUN(post("new offset=%d", offset));
    offset++; /* proceed to the next element */
  }

  DEBUGFUN(post("got %d results", count));

  outlet_list(out, gensym("list"), count, ap);
  freebytes(ap, length*sizeof(t_atom));
}

static void listfind_list(t_listfind *x, t_symbol *s, int argc, t_atom *argv)
{
#if 0
  /* entire list hot: 
   * this is more intuitive when searching a pattern in many lists 
   */
  listfind_doit(x->x_obj.ob_outlet, argc, argv, x->x_argc, x->x_argv);

#else

  /* pattern is hot
   * this is compatible with foobar's [list-find]
   * this is more intuitive when searching different patterns in a list
   */
  listfind_doit(x->x_obj.ob_outlet, x->x_argc, x->x_argv, argc, argv);

#endif

  /* personally i think that searching 1 pattern in many lists is more useful */
}

static void listfind_free(t_listfind *x)
{ 
  if(x->x_argv) {
    freebytes(x->x_argv, x->x_argc*sizeof(int));
    x->x_argv=0;
    x->x_argc=0;
  }
  inlet_free(x->x_listin);

}

static void *listfind_new(t_symbol *s, int argc, t_atom *argv)
{
  t_listfind *x = (t_listfind *)pd_new(listfind_class);
  ZEXY_USEVAR(s);

  outlet_new(&x->x_obj, 0);
  x->x_listin=inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("list"), gensym("lst2"));

  x->x_argc=0;
  x->x_argv=0;

  listfind_list2(x, gensym("list"), argc, argv);

  return (x);
}


static void listfind_help(t_listfind*x)
{
  post("\n%c listfind\t\t:: split lists into multiple sublists based on matches", HEARTSYMBOL);
}

void listfind_setup(void)
{
  listfind_class = class_new(gensym("listfind"), (t_newmethod)listfind_new, 
                             (t_method)listfind_free, sizeof(t_listfind), 0, A_GIMME, 0);
  class_addlist    (listfind_class, listfind_list);
  class_addmethod  (listfind_class, (t_method)listfind_list2, gensym("lst2"), A_GIMME, 0);

  class_addmethod(listfind_class, (t_method)listfind_help, gensym("help"), A_NULL);
  zexy_register("listfind");
}
