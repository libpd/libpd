/* 
 *  sleepgrain :  get (and set?) the sleepgrain of Pd
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

EXTERN int* get_sys_sleepgrain(void ) ;

/* ------------------------- sleepgrain ------------------------------- */


static t_class *sleepgrain_class;

typedef struct _sleepgrain
{
  t_object x_obj;

} t_sleepgrain;


static void sleepgrain_bang(t_sleepgrain *x)
{
  int*current=get_sys_sleepgrain();
  t_float f=*current;
  outlet_float(x->x_obj.ob_outlet, f);
}

static void sleepgrain_float(t_sleepgrain *x, t_float f)
{
  int value=(int)f;
  int*current=get_sys_sleepgrain();

  if(value<=0) {
    pd_error(x, "[sleepgrain]: sleepgrain cannot be <= 0");
    return;
  }

  *current=value;

  //  outlet_float(x->x_obj.ob_outlet, f);
}

static void *sleepgrain_new(void)
{
  t_sleepgrain *x = (t_sleepgrain *)pd_new(sleepgrain_class);
  outlet_new(&x->x_obj, 0);
  return (x);
}

void sleepgrain_setup(void)
{
  sleepgrain_class = class_new(gensym("sleepgrain"), (t_newmethod)sleepgrain_new, 
                                     0, sizeof(t_sleepgrain), 0, A_NULL);
  
  class_addbang  (sleepgrain_class, sleepgrain_bang);
  class_addfloat (sleepgrain_class, sleepgrain_float);
  zexy_register("sleepgrain");
}
