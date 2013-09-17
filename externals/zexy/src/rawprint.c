/* 
 * rawprint:  print the incoming message as raw as possible
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
#include <stdio.h>

#ifdef _MSC_VER 
# define snprintf _snprintf
#endif

#if !defined( _MSC_VER ) && defined (_WIN32)
int _get_output_format( void ){ return 0; }
#endif

static t_class *rawprint_class;

typedef struct _rawprint {
  t_object  x_obj;
  t_symbol*label;
} t_rawprint;

static void rawprint_any(t_rawprint *x, t_symbol*s, int argc, t_atom*argv)
{
  char buf[MAXPDSTRING];
  if(x->label) {
    startpost("%s: ", x->label->s_name);
  }
  startpost("\"%s\"", s->s_name);
  while(argc--) {
    switch(argv->a_type) {
    case A_FLOAT:
      snprintf(buf, MAXPDSTRING-1, "%f", atom_getfloat(argv));
      break;
    case A_SYMBOL:
      snprintf(buf, MAXPDSTRING-1, "'%s'", atom_getsymbol(argv)->s_name);
      break;
    case A_POINTER:
      snprintf(buf, MAXPDSTRING-1, "pointer[%p]", argv->a_w.w_gpointer);
      break;
    case A_SEMI:
      snprintf(buf, MAXPDSTRING-1, "SEMI");
      break;
    case A_COMMA:
      snprintf(buf, MAXPDSTRING-1, "COMMA");
      break;
    case A_DEFFLOAT:
      snprintf(buf, MAXPDSTRING-1, "DEFFLOAT[%f]", atom_getfloat(argv));
      break;
    case A_DEFSYM:
      snprintf(buf, MAXPDSTRING-1, "DEFSYM['%s']", atom_getsymbol(argv)->s_name);
      break;
    case A_DOLLAR:
      snprintf(buf, MAXPDSTRING-1, "DOLLAR['%s']", atom_getsymbol(argv)->s_name);
      break;
    case A_DOLLSYM:
      snprintf(buf, MAXPDSTRING-1, "DOLLSYM['%s']", atom_getsymbol(argv)->s_name);
      break;
    case A_GIMME:
      snprintf(buf, MAXPDSTRING-1, "GIMME");
      break;
    case A_CANT: // we _really_ cannot do CANT
      snprintf(buf, MAXPDSTRING-1, "CANT");
      break;
    default:
      snprintf(buf, MAXPDSTRING-1, "unknown[%d]", argv->a_type);
    }
    buf[MAXPDSTRING-1]=0;
    
    startpost(" %s", buf);
    argv++;
  }
  endpost();

}

static void *rawprint_new(t_symbol*s)
{
  t_rawprint *x = (t_rawprint *)pd_new(rawprint_class);
  x->label=NULL;
  if(s&&gensym("")!=s)
    x->label=s;

  return (x);
}

void rawprint_setup(void) {
  rawprint_class = class_new(gensym("rawprint"),
                             (t_newmethod)rawprint_new,
                             0, sizeof(t_rawprint),
                             CLASS_DEFAULT, A_DEFSYMBOL, 0);

  class_addanything(rawprint_class, rawprint_any);
  zexy_register("rawprint");
}
