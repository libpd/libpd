/* 
 * date: gets the current date from the system
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

/******************************************************
 *
 * zexy - implementation file
 *
 * copyleft (c) IOhannes m zmölnig
 *
 *   1999:forum::für::umläute:2004
 *
 *   institute of electronic music and acoustics (iem)
 *
 ******************************************************
 *
 * license: GNU General Public License v.2
 *
 ******************************************************/

/* 
   (c) 1202:forum::für::umläute:2000
   1506:forum::für::umläute:2003: use timeb only if needed (like on windoze)   
*/

#include "zexy.h"

#ifdef __WIN32__
# define USE_TIMEB
#endif

#ifdef __APPLE__
# include <sys/types.h>
/* typedef     _BSD_TIME_T_    time_t;                */
#endif


#include <time.h>

#ifdef USE_TIMEB
# include <sys/timeb.h>
#else
# include <sys/time.h>
#endif


/* ----------------------- date --------------------- */

static t_class *date_class;

typedef struct _date
{
  t_object x_obj;
  
  int GMT;
  
  t_outlet *x_outlet1;
  t_outlet *x_outlet2;
  t_outlet *x_outlet3;
  t_outlet *x_outlet4;
  t_outlet *x_outlet5;
  t_outlet *x_outlet6;
} t_date;

static void *date_new(t_symbol *s, int argc, t_atom *argv)
{
  t_date *x = (t_date *)pd_new(date_class);
  char buf[5];
  ZEXY_USEVAR(s);
 
  x->GMT=0;
  if (argc) {
    atom_string(argv, buf, 5);
    if (buf[0]=='G' && buf[1]=='M' && buf[2]=='T')
      x->GMT = 1;
  }
  
  x->x_outlet1 = outlet_new(&x->x_obj, gensym("float"));
  x->x_outlet2 = outlet_new(&x->x_obj, gensym("float"));
  x->x_outlet3 = outlet_new(&x->x_obj, gensym("float"));
  x->x_outlet4 = outlet_new(&x->x_obj, gensym("float"));
  x->x_outlet5 = outlet_new(&x->x_obj, gensym("float"));
  x->x_outlet6 = outlet_new(&x->x_obj, gensym("float"));
  
  return (x);
}

static void date_bang(t_date *x)
{
  struct tm *resolvetime;
#ifdef USE_TIMEB
  struct timeb mytime;
  ftime(&mytime);
  resolvetime=(x->GMT)?gmtime(&mytime.time):localtime(&mytime.time);
#else
  struct timeval tv;
  gettimeofday(&tv, 0);
  resolvetime = (x->GMT)?gmtime(&tv.tv_sec):localtime(&tv.tv_sec);
#endif
  outlet_float(x->x_outlet6, (t_float)resolvetime->tm_isdst);
  outlet_float(x->x_outlet5, (t_float)resolvetime->tm_yday);
  outlet_float(x->x_outlet4, (t_float)resolvetime->tm_wday);
  outlet_float(x->x_outlet3, (t_float)resolvetime->tm_mday);
  outlet_float(x->x_outlet2, (t_float)resolvetime->tm_mon + 1);
  outlet_float(x->x_outlet1, (t_float)resolvetime->tm_year + 1900);
}

static void help_date(t_date *x)
{
  ZEXY_USEVAR(x);
  post("\n%c date\t\t:: get the current system date", HEARTSYMBOL);
  post("\noutputs are\t: year / month / day / day of week /day of year / daylightsaving (1/0)");
  post("\ncreation\t::'date [GMT]': show local date or GMT");
}

void date_setup(void)
{
  date_class = class_new(gensym("date"),
			 (t_newmethod)date_new, 0,
			 sizeof(t_date), 0, A_GIMME, 0);
  
  class_addbang(date_class, date_bang);
  
  class_addmethod(date_class, (t_method)help_date, gensym("help"), 0);
  zexy_register("date");
}
