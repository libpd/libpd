/* 
 * symbol2list: convert a symbol into a list (with given delimiters)
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
#include <stdlib.h>
#include <string.h>

/* ------------------------- symbol2list ------------------------------- */

static t_class *symbol2list_class;

typedef struct _symbol2list
{
  t_object x_obj;
  t_symbol *s, *delimiter;
  t_atom   *argv;
  int      argc, argnum; /* "argnum" is the number of reserved atoms (might be >argc) */
} t_symbol2list;

static void symbol2list_delimiter(t_symbol2list *x, t_symbol *s){
  x->delimiter = s;
}

STATIC_INLINE void string2atom(t_atom *ap, char* cp, int clen){
  char *buffer=getbytes(sizeof(char)*(clen+1));
  char *endptr[1];
  t_float ftest;
  strncpy(buffer, cp, clen);
  buffer[clen]=0;
  ftest=strtod(buffer, endptr);
  /* what should we do with the special cases of hexadecimal values, "INF" and "NAN" ???
   * strtod() parses them to numeric values:
   * symbol "hallo 0x12" will become "list hallo 18"
   * do we want this ??
   */
  if (buffer+clen!=*endptr){
    /* strtof() failed, we have a symbol */
    SETSYMBOL(ap, gensym(buffer));    
  } else {
    /* it is a number. */
    SETFLOAT(ap,ftest);
  }
  freebytes(buffer, sizeof(char)*(clen+1));
}
static void symbol2list_process(t_symbol2list *x)
{
  char *cc;
  char *deli;
  int   dell;
  char *cp, *d;
  int i=1;

  if (x->s==NULL){
    x->argc=0;
    return;
  }
  cc=x->s->s_name;
  cp=cc;
  
  if (x->delimiter==NULL || x->delimiter==gensym("")){
    i=strlen(cc);
    if(x->argnum<i){
      freebytes(x->argv, x->argnum*sizeof(t_atom));
      x->argnum=i+10;
      x->argv=getbytes(x->argnum*sizeof(t_atom));
    }
    x->argc=i;
    while(i--)string2atom(x->argv+i, cc+i, 1);
    return;
  }

  deli=x->delimiter->s_name;
  dell=strlen(deli);

  
  /* get the number of tokens */
  while((d=strstr(cp, deli))){
    if (d!=NULL && d!=cp){
      i++;
    }
    cp=d+dell;
  }

  /* resize the list-buffer if necessary */
  if(x->argnum<i){
    freebytes(x->argv, x->argnum*sizeof(t_atom));
    x->argnum=i+10;
    x->argv=getbytes(x->argnum*sizeof(t_atom));
  }
  x->argc=i;
  /* parse the tokens into the list-buffer */
  i=0;
  
    /* find the first token */
    cp=cc;
    while(cp==(d=strstr(cp,deli))){cp+=dell;}
    while((d=strstr(cp, deli))){
      if(d!=cp){
	string2atom(x->argv+i, cp, d-cp);
	i++;
      }
      cp=d+dell;
    }

    if(cp)string2atom(x->argv+i, cp, strlen(cp));
}
static void symbol2list_bang(t_symbol2list *x){
  if(!(x->s) || x->s==gensym("")){
    outlet_bang(x->x_obj.ob_outlet);
    return;
  }
  symbol2list_process(x);
  if(x->argc)outlet_list(x->x_obj.ob_outlet, 0, x->argc, x->argv);
}
static void symbol2list_symbol(t_symbol2list *x, t_symbol *s){
  x->s = s;
  symbol2list_bang(x);
}
static void *symbol2list_new(t_symbol *s, int argc, t_atom *argv)
{
  t_symbol2list *x = (t_symbol2list *)pd_new(symbol2list_class);
  ZEXY_USEVAR(s);

  outlet_new(&x->x_obj, 0);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("symbol"), gensym(""));

  x->argc=0;
  x->argnum=16;
  x->argv=getbytes(x->argnum*sizeof(t_atom));
  symbol2list_delimiter(x, (argc)?atom_getsymbol(argv):gensym(" "));
  
  return (x);
}

static void symbol2list_free(t_symbol2list *x)
{}

static void symbol2list_help(t_symbol2list*x)
{
  post("\n%c symbol2list\t:: split a symbol into a list of atoms", HEARTSYMBOL);
}

void symbol2list_setup(void)
{
  symbol2list_class = class_new(gensym("symbol2list"), (t_newmethod)symbol2list_new, 
			 (t_method)symbol2list_free, sizeof(t_symbol2list), 0, A_GIMME, 0);

  class_addcreator((t_newmethod)symbol2list_new, gensym("s2l"), A_GIMME, 0);
  class_addsymbol (symbol2list_class, symbol2list_symbol);
  class_addbang   (symbol2list_class, symbol2list_bang);
  class_addmethod  (symbol2list_class, (t_method)symbol2list_delimiter, gensym(""), A_SYMBOL, 0);
  class_addmethod(symbol2list_class, (t_method)symbol2list_help, gensym("help"), A_NULL);

  zexy_register("symbol2list");
}
void s2l_setup(void)
{
  symbol2list_setup();
}
