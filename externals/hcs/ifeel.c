/* 
 * ifeel mouse object for Miller Puckette's Pure Data
 * copyright 2003 Hans-Christoph Steiner <hans@eds.org>

 * This program is free software; you can redistribute it and/or               
 * modify it under the terms of the GNU General Public License                 
 * as published by the Free Software Foundation; either version 3              
 * of the License, or (at your option) any later version.                      
 * This program is distributed in the hope that it will be useful,             
 * but WITHOUT ANY WARRANTY; without even the implied warranty of              
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               
 * GNU General Public License for more details.                                
 * You should have received a copy of the GNU General Public License           
 * along with this program; if not, write to the Free Software                 
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. 
 *
 *
 * based on ifeel_send.c from the linux iFeel driver:
 * http://sourceforge.net/projects/tactile
 *
 * there is a difference in the naming schemes of the ifeel driver and this 
 * object.  The ifeel driver uses strength, delay, and count.  This object 
 * uses strength, interval, and count.
 *
 * strength       - the strength of the pulse (I am searching for a better word)
 * delay/interval - the interval in between each pulse
 * count          - the total number of pulses to do
 */

#include "m_pd.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#ifndef NT
#include <sys/ioctl.h>
#endif

#include "ifeel.h"


#define DEBUG(x)
/*#define DEBUG(x) x */


#define IFEEL_DEVICE    "/dev/input/ifeel0"

static t_class *ifeel_class;

typedef struct _ifeel 
{
	t_object x_obj;
	int x_fd;
	struct ifeel_command x_ifeel_command;
} t_ifeel;



/******************************************************************************
support functions
******************************************************************************/

void ifeel_playcommand(t_ifeel *x) 
{
/*   const struct timespec *requested_time; */
/*   struct timespec *remaining; */
	
#ifdef __linux__
	if (ioctl(x->x_fd, USB_IFEEL_BUZZ_IOCTL, &x->x_ifeel_command) < 0) 
	{
		post("x->x_fd: %d",x->x_fd);
		post("strength: %d   interval: %d   count: %d",
			  x->x_ifeel_command.strength,x->x_ifeel_command.delay,x->x_ifeel_command.count);
		post("ERROR %s", strerror(errno));
		close(x->x_fd);  
	}
#endif  /* __linux__ */
	
	DEBUG(
		post("strength: %d   interval: %d   count: %d",
			  x->x_ifeel_command.strength,x->x_ifeel_command.delay,x->x_ifeel_command.count);
		post(""););
}


/******************************************************************************
input/control functions
******************************************************************************/
void ifeel_start(t_ifeel *x) {
  DEBUG(post("ifeel_start");)

  /* 
   * since ifeel_stop sets everything to zero, we need to
   * read the inlets again to get current values 
   */
  
  ifeel_playcommand(x);
}

void ifeel_stop(t_ifeel *x) 
{
	DEBUG(post("ifeel_stop"););
	struct ifeel_command temp_ifeel_command;

/* store previous command for restoring after stop  */
	temp_ifeel_command.strength = x->x_ifeel_command.strength;
	temp_ifeel_command.delay = x->x_ifeel_command.delay;
	temp_ifeel_command.count = x->x_ifeel_command.count;

	/* 
	 * there is no 'stop' ioctl, so set everything to zero 
	 * to achieve the same effect                          
	 */
	x->x_ifeel_command.strength = 0;
	x->x_ifeel_command.delay = 0;
	x->x_ifeel_command.count = 0;
	
	ifeel_playcommand(x);
	
	/* restore previous command so the start msg will work */
	x->x_ifeel_command.strength = temp_ifeel_command.strength;
	x->x_ifeel_command.delay = temp_ifeel_command.delay;
	x->x_ifeel_command.count = temp_ifeel_command.count;
}

void ifeel_strength(t_ifeel *x, t_floatarg strength) 
{
	DEBUG(post("ifeel_strength"););
	
/* 
 * make sure its in the proper range 
 * this object takes floats 0-1
 * the ifeel driver takes ints 0-255
 */
  strength = strength * 255;
  strength = (strength > 255 ? 255 : strength);
  strength = (strength < 0 ? 0 : strength);

  x->x_ifeel_command.strength  = (unsigned int)strength;
}

void ifeel_interval(t_ifeel *x, t_floatarg interval) 
{
	DEBUG(post("ifeel_interval"););
	
	interval = (interval < 0 ? 0 : interval);
	
	x->x_ifeel_command.delay  = (unsigned int)interval;
}

void ifeel_count(t_ifeel *x, t_floatarg count ) 
{
	DEBUG(post("ifeel_count"););
	
	count = (count < 0 ? 0 : count);  
	
	x->x_ifeel_command.count  = (unsigned int)count;
}

void ifeel_command(t_ifeel *x, t_floatarg interval, t_floatarg count, t_floatarg strength) 
{
	DEBUG(post("ifeel_command"););
	
	ifeel_strength(x,strength);
	ifeel_interval(x,interval);
	ifeel_count(x,count);
	
	ifeel_playcommand(x);
}

static int ifeel_open(t_ifeel *x) 
{
	return 1;
}

/******************************************************************************
  init/free functions
******************************************************************************/

void ifeel_free(t_ifeel *x)
{
	DEBUG(post("ifeel_free"););
	
   /* stop effect */
	ifeel_stop(x);
	
	/* close device */
	close(x->x_fd);
}

void *ifeel_new(t_symbol *device, t_floatarg strength, t_floatarg interval, t_floatarg count) {
	DEBUG(post("ifeel_new"););
	
	t_ifeel *x = (t_ifeel *)pd_new(ifeel_class);
  
	post("iFeel mouse, by Hans-Christoph Steiner <hans@eds.org>");
	post("");
	post ("WARNING * WARNING * WARNING * WARNING * WARNING * WARNING * WARNING");
	post ("This object is under development!  The interface could change at anytime!");
	post ("As I write cross-platform versions, the interface might have to change.");
	post ("WARNING * WARNING * WARNING * WARNING * WARNING * WARNING * WARNING");
	post("");
#ifndef __linux__
	post("    !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !!");
	post("     This is a dummy, since this object only works with a Linux kernel!");
	post("    !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !!");
#endif
  
  /* 
   * init to zero so I can use the ifeel_* methods to set the 
   * struct with the argument values
   */
	x->x_ifeel_command.strength = 0;
	x->x_ifeel_command.delay = 0;
	x->x_ifeel_command.count = 0;
	
	inlet_new(&x->x_obj,
				 &x->x_obj.ob_pd,
				 gensym("float"),
				 gensym("interval"));
	inlet_new(&x->x_obj,
				 &x->x_obj.ob_pd,
				 gensym("float"),
				 gensym("count"));
	inlet_new(&x->x_obj,
				 &x->x_obj.ob_pd,
				 gensym("float"),
				 gensym("strength"));
	
	if (device != &s_) 
	{
		post("Using %s",device->s_name);
		
		/* x->x_fd = open(IFEEL_DEVICE, O_RDWR); */
		if ((x->x_fd = open((char *) device->s_name, O_RDWR | O_NONBLOCK, 0)) <= 0) 
		{
			printf("ERROR %s\n", strerror(errno)); 
			return 0;
		}
		
/*     ifeel_strength(x,strength); */
/*     ifeel_interval(x,interval); */
/*     ifeel_count(x,count); */
	}
	
	else 
	{
		post("ifeel: You need to set an ifeel device (i.e /dev/input/ifeel0)");
	}
	
  return (void*)x;
}

void ifeel_setup(void)
{
	DEBUG(post("ifeel_setup"););
	
	ifeel_class = class_new(gensym("ifeel"),
									(t_newmethod)ifeel_new,
									(t_method)ifeel_free,
									sizeof(t_ifeel),
									CLASS_DEFAULT,
									A_DEFSYMBOL,
									A_DEFFLOAT,
									A_DEFFLOAT,
									A_DEFFLOAT,
									0);
	
	class_addbang(ifeel_class,ifeel_start);
	
	class_addmethod(ifeel_class, (t_method)ifeel_start,gensym("start"),0);
	class_addmethod(ifeel_class, (t_method)ifeel_stop,gensym("stop"),0);
	
	class_addmethod(ifeel_class, (t_method)ifeel_command,gensym("command"), 
						 A_DEFFLOAT,A_DEFFLOAT,A_DEFFLOAT,0);  
	
	class_addmethod(ifeel_class, (t_method)ifeel_strength,gensym("strength"),A_DEFFLOAT,0);  
	class_addmethod(ifeel_class, (t_method)ifeel_interval,gensym("interval"),A_DEFFLOAT,0);
	class_addmethod(ifeel_class, (t_method)ifeel_count,gensym("count"),A_DEFFLOAT,0);
}


