/* 
 * operating_system :  get currently used OS
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

/* ------------------------- operating_system ------------------------------- */

/*
MESSAGE OPERATING_SYSTEM: simple and easy
*/

static t_class *operating_system_class;

typedef struct _operating_system
{
  t_object x_obj;

} t_operating_system;


static void operating_system_bang(t_operating_system *x)
{
  /* LATER think about querying the version of the system at runtime! */
  t_symbol *s=gensym("unknown");
#ifdef __linux__
  s=gensym("linux");
#elif defined __APPLE__
  s=gensym("macos");
#elif defined __WIN32__
  s=gensym("windows");
#endif
  outlet_symbol(x->x_obj.ob_outlet, s);
}

static void *operating_system_new(void)
{
  t_operating_system *x = (t_operating_system *)pd_new(operating_system_class);
  outlet_new(&x->x_obj, 0);
  return (x);
}

static void operating_system_help(t_operating_system*x)
{
  post("\n%c operating_system\t:: get the current operating system", HEARTSYMBOL);
}

void operating_system_setup(void)
{
  operating_system_class = class_new(gensym("operating_system"), (t_newmethod)operating_system_new, 
                                     0, sizeof(t_operating_system), 0, A_NULL);
  
  class_addbang  (operating_system_class, operating_system_bang);
  class_addmethod(operating_system_class, (t_method)operating_system_help, gensym("help"), A_NULL);
  zexy_register("operating_system");
}
