/* 
 * lpt: read/write the parallel port
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


/* 
   (c) 2000:forum::für::umläute:2005

   write to the parallel port
   extended to write to any port (if we do have permissions)

   2005-09-28: write to devices instead of hardware-addresses
     http://people.redhat.com/twaugh/parport/html/ppdev.html
     TODO: don't lock when multiple objects refer to the same device
     TODO: allow readonly/writeonly access
     TODO: test for timeouts,...

   thanks to
    Olaf Matthes: porting to WindozeNT/2000/XP
    Thomas Musil: adding "control-output" and "input"
*/
#define BASE0  0x3bc
#define BASE1  0x378
#define BASE2  0x278

#define MODE_IOPERM 1
#define MODE_IOPL   0
#define MODE_NONE   -1

#include "zexy.h"

/* ----------------------- lpt --------------------- */

#ifdef Z_WANT_LPT
# include <stdlib.h>
# include <errno.h>

# ifdef HAVE_LINUX_PPDEV_H
#  include <unistd.h>
#  include <sys/ioctl.h>
#  include <linux/ppdev.h>
#  include <linux/parport.h>
#  include <fcntl.h>
#  include <stdio.h>
# endif /* HAVE_LINUX_PPDEV_H */


# ifdef __WIN32__
/* on windoze everything is so complicated... */
extern int read_parport(int port);
extern void write_parport(int port, int value);
extern int open_port(int port);

static int ioperm(int port, int a, int b)
{
  if(open_port(port) == -1)return(1);
  return(0);
}

static int iopl(int i)
{
  return(-1);
}

static void sys_outb(unsigned char byte, int port)
{
  write_parport(port, byte);
}
static int sys_inb(int port)
{
  return read_parport(port);
}
# else
/* thankfully there is linux */
#  include <sys/io.h>

static void sys_outb(unsigned char byte, int port)
{
  outb(byte, port);
}
static int sys_inb(int port)
{
  return inb(port);
}
# endif /* __WIN32__ */
#endif /* Z_WANT_LP */


static int count_iopl = 0;
static t_class *lpt_class;

typedef struct _lpt
{
  t_object x_obj;

  unsigned long port;
  int device; /* file descriptor of device, in case we are using one ...*/

  int mode; /* MODE_IOPERM, MODE_IOPL */
} t_lpt;

static void lpt_float(t_lpt *x, t_floatarg f)
{
#ifdef Z_WANT_LPT
  unsigned char b = f;
# ifdef HAVE_LINUX_PPDEV_H
  if (x->device>0){
    ioctl (x->device, PPWDATA, &b);
  } else
# endif
  if (x->port) {
    sys_outb(b, x->port+0);
  }
#endif /*  Z_WANT_LPT */
}

static void lpt_control(t_lpt *x, t_floatarg f)
{
#ifdef Z_WANT_LPT
  unsigned char b = f;
# ifdef HAVE_LINUX_PPDEV_H
  if (x->device>0){
    ioctl (x->device, PPWCONTROL, &b);
  } else
# endif
  if (x->port) {
    sys_outb(b, x->port+2);
  }
#endif /*  Z_WANT_LPT */
}

static void lpt_bang(t_lpt *x)
{
#ifdef Z_WANT_LPT
# ifdef HAVE_LINUX_PPDEV_H
  if (x->device>0){
    unsigned char b=0;
    ioctl (x->device, PPRCONTROL, &b);
      outlet_float(x->x_obj.ob_outlet, (t_float)b);
  } else
# endif
  if (x->port)	{
    outlet_float(x->x_obj.ob_outlet, (t_float)sys_inb(x->port+1));
  }
#endif /*  Z_WANT_LPT */
}


static void *lpt_new(t_symbol *s, int argc, t_atom *argv)
{
  t_lpt *x = (t_lpt *)pd_new(lpt_class);
  char*devname=atom_getsymbol(argv)->s_name;
  if(s==gensym("lp"))
    error("lpt: the use of 'lp' has been deprecated; use 'lpt' instead");


  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("control"));
  outlet_new(&x->x_obj, gensym("float"));
  x->mode = MODE_NONE;
  x->port = 0;
  x->device = -1;

#ifdef Z_WANT_LPT
  if ((argc==0)||(argv->a_type==A_FLOAT)) {
    /* FLOAT specifies a parallel port */
    switch ((int)((argc)?atom_getfloat(argv):0)) {
    case 0:
      x->port = BASE0;
      break;
    case 1:
      x->port = BASE1;
      break;  
    case 2:
      x->port = BASE2;
      break;
    default:
      error("lpt : only lpt0, lpt1 and lpt2 are accessible");
      x->port = 0;
      return (x);
    }
  } else { 
    /* SYMBOL might be a file or a hex port-number;
       we ignore the file (device) case by now;
       LATER think about this
    */
    x->device=-1;
    x->port=strtol(devname, 0, 16);
    if(0==x->port){
#ifdef HAVE_LINUX_PPDEV_H
      x->device = open(devname, O_RDWR);
      if(x->device<=0){
        error("lpt: bad device %s", devname);
        return(x);
      } else {
        if (ioctl (x->device, PPCLAIM)) {
          perror ("PPCLAIM");
          close (x->device);
          x->device=-1;
        }
      }
#endif /* HAVE_LINUX_PPDEV_H */
    }
  }

  if ((x->device<0) && (!x->port || x->port>65535)){
    post("lpt : bad port %x", x->port);
    x->port = 0;
    return (x);
  }
  if (x->device<0){
    /* this is ugly: when using a named device,
     * we are currently assuming that we have read/write-access
     * of course, this is not necessary true
     */
    /* furthermore, we might also use the object
     * withOUT write permissions
     * (just reading the parport)
     */
    if (x->port && x->port < 0x400){
      if (ioperm(x->port, 8, 1)) {
        x->mode=MODE_NONE;
      } else x->mode = MODE_IOPERM;
    }
    if(x->mode==MODE_NONE){
      if (iopl(3)){
        x->mode=MODE_NONE;
      } else x->mode=MODE_IOPL;
      count_iopl++;
    }
    
    if(x->mode==MODE_NONE){
      error("lpt : couldn't get write permissions");
      x->port = 0;
      return (x);
    }
  }
  if(x->device>0)
    post("lpt: connected to device %s", devname);
  else
    post("lpt: connected to port %x in mode '%s'", x->port, (x->mode==MODE_IOPL)?"iopl":"ioperm");
  if (x->mode==MODE_IOPL)post("lpt-warning: this might seriously damage your pc...");
#else
  error("zexy has been compiled without [lpt]!");
  count_iopl=0;
#endif /* Z_WANT_LPT */

  devname=0;

  return (x);
}

static void lpt_free(t_lpt *x)
{
#ifdef Z_WANT_LPT
# ifdef HAVE_LINUX_PPDEV_H
  if (x->device>0){
    ioctl (x->device, PPRELEASE);
    close(x->device);
    x->device=0;
  } else
# endif
  if (x->port) {
    if (x->mode==MODE_IOPERM && ioperm(x->port, 8, 0)) error("lpt: couldn't clean up device");
    else if (x->mode==MODE_IOPL && (!--count_iopl) && iopl(0))
      error("lpt: couldn't clean up device");
  }
#endif /* Z_WANT_LPT */
}


static void lpt_helper(t_lpt *x)
{
  ZEXY_USEVAR(x);
  post("\n%c lpt :: direct access to the parallel port", HEARTSYMBOL);
  post("<byte>\t: write byte to the parallel-port");
  post("\ncreation:\t\"lpt [<port>]\": connect to parallel port <port> (0..2)");
  post("\t\t\"lpt <portaddr>\": connect to port @ <portaddr> (hex)");
}

void lpt_setup(void)
{
  lpt_class = class_new(gensym("lpt"),
			  (t_newmethod)lpt_new, (t_method)lpt_free,
			  sizeof(t_lpt), 0, A_GIMME, 0);
  class_addcreator((t_newmethod)lpt_new, gensym("lp"), A_GIMME, 0);

  class_addfloat(lpt_class, (t_method)lpt_float);
  class_addmethod(lpt_class, (t_method)lpt_control, gensym("control"), A_FLOAT, 0);
  class_addbang(lpt_class, (t_method)lpt_bang);

  class_addmethod(lpt_class, (t_method)lpt_helper, gensym("help"), 0);
  zexy_register("lpt");
}
