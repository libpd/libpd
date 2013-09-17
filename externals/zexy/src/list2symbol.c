/* 
 *  list2symbol: convert a list into a single symbol
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
#include <string.h>

/* ------------------------- list2symbol ------------------------------- */

static t_class *list2symbol_class;

typedef struct _list2symbol
{
  t_object x_obj;
  int       ac;
  t_atom   *ap;
  t_symbol *s,*connector;
  t_inlet *x_inlet2;
  t_outlet*x_outlet;
} t_list2symbol;

static void list2symbol_connector(t_list2symbol *x, t_symbol *s){
  x->connector = s;
}

static void list2symbol_bang(t_list2symbol *x)
{
  t_atom *argv=x->ap;
  int     argc=x->ac;
  char *result = 0;
  int length = 0, len=0;
  int i= argc;
  char *connector=0;
  char connlen=0;
  char*buffer = (char*)getbytes(MAXPDSTRING*sizeof(char));
  if(x->connector){
    connector=x->connector->s_name;
    connlen=strlen(connector);
  }
      
  /* 1st get the length of the symbol */
  if(x->s)length+=strlen(x->s->s_name);
  else length-=connlen;

  length+=i*connlen;

  while(i--){
    int len2=0;
    if(A_SYMBOL==argv->a_type){
      len2=strlen(argv->a_w.w_symbol->s_name);
    } else {
      atom_string(argv, buffer, MAXPDSTRING);
      len2=strlen(buffer);
    }
    length+=len2;
    argv++;
  }

  if (length<=0){
    outlet_symbol(x->x_obj.ob_outlet, gensym(""));
    return;
  }

  result = (char*)getbytes((length+1)*sizeof(char));

  /* 2nd create the symbol */
  if (x->s){
    char *buf = x->s->s_name;
    int buflen=strlen(buf);
    strncpy(result+len, buf, length-len);
    len+=buflen;
    if(i && connector){
      strncpy(result+len, connector, length-len);
      len += connlen;
    }
  }
  i=argc;
  argv=x->ap;
  while(i--){
    if(A_SYMBOL==argv->a_type){
      strncpy(result+len, argv->a_w.w_symbol->s_name, length-len);
      len+= strlen(argv->a_w.w_symbol->s_name);
    } else {
      atom_string(argv, buffer, MAXPDSTRING);
      strncpy(result+len, buffer, length-len);
      len += strlen(buffer);
    }
    argv++;
    if(i && connector){
      strncpy(result+len, connector, length-len);
      len += connlen;
    }
  }
  freebytes(buffer, MAXPDSTRING*sizeof(char));

  result[length]=0;
  outlet_symbol(x->x_obj.ob_outlet, gensym(result));
  freebytes(result, (length+1)*sizeof(char));
}

static void list2symbol_anything(t_list2symbol *x, t_symbol *s, int argc, t_atom *argv)
{
  if(x->ap){
    freebytes(x->ap, x->ac*sizeof(t_atom));
    x->ap=0;
  }

  x->s =s;
  x->ac=argc;

  if(x->ac){
    x->ap=(t_atom*)getbytes(x->ac*sizeof(t_atom));
  }
  if(x->ap){
    t_atom*ap=x->ap;
    while(argc--){
      *ap++=*argv++;
    }
  }
  list2symbol_bang(x);
}

static void list2symbol_list(t_list2symbol *x, t_symbol *s, int argc, t_atom *argv)
{
  list2symbol_anything(x, 0, argc, argv);
}
static void *list2symbol_new(t_symbol *s, int argc, t_atom *argv)
{
  t_list2symbol *x = (t_list2symbol *)pd_new(list2symbol_class);

  x->x_outlet=outlet_new(&x->x_obj, 0);
  x->x_inlet2=inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("symbol"), gensym(""));

#if 0
  /* old behaviour: the argument list is used as the list-to-be-converted */
  x->connector = gensym(" ");
  list2symbol_anything(x, 0, argc, argv);
#else
  /* new behaviour: set the delimiter with the argument */
  list2symbol_connector(x, (argc)?atom_getsymbol(argv):gensym(" "));
#endif
   

  return (x);
}

static void list2symbol_free(t_list2symbol *x)
{
  if(x->ap){
    freebytes(x->ap, x->ac*sizeof(t_atom));
    x->ap=0;
  }
  outlet_free(x->x_outlet);
  inlet_free(x->x_inlet2);
}


void list2symbol_setup(void)
{
  list2symbol_class = class_new(gensym("list2symbol"), (t_newmethod)list2symbol_new, 
                                (t_method)list2symbol_free, sizeof(t_list2symbol), 0,
                                A_GIMME, 0);

  class_addcreator((t_newmethod)list2symbol_new, gensym("l2s"), A_GIMME, 0);
  class_addbang    (list2symbol_class, list2symbol_bang);
  class_addlist    (list2symbol_class, list2symbol_list);
  class_addanything(list2symbol_class, list2symbol_anything);
  class_addmethod  (list2symbol_class, (t_method)list2symbol_connector, gensym(""), A_SYMBOL, 0);

  zexy_register("list2symbol");
}
void l2s_setup(void)
{
  list2symbol_setup();
}
