/*
  cxc@web.fm, 200203
  interface to the linux proc filesystem
  TODO: stat, number of users, network stats (tx,rx,etc)
 */

#include "m_pd.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <unistd.h>
// later for number of users
// #include <utmp.h>

#ifndef SSIZE_MAX
#define SSIZE_MAX 255
#endif

#ifndef RAND_MAX
#define RAND_MAX 2147483647
#endif

#ifndef MAXOUTAT
#define MAXOUTAT 50
#endif

t_class *proc_class;

typedef struct proc
{
  t_object x_obj;
  // t_float  x_RM;
  t_atom   x_at[MAXOUTAT];
  int      x_atc;
} t_proc;

void proc_float(t_proc *x, t_floatarg f)
{
    post("cxc/proc.c: %f", f);
}

void proc_cpuinfo(t_proc *x)
{
  char name[255];        // filename
  char rest[255];        // string value
  t_float restf;         // float value
  char bla[1];
  FILE* fp;              // info file descriptor
  //  t_float val;
  int   ac;              // atom count
  t_atom* at = x->x_at;  // out atom
  int atc    = x->x_atc; // out atom count

  //name = "/proc/cpuinfo";
  sprintf(name,"%s","/proc/cpuinfo");
  //val   = 0;
  restf = 0;
  ac    = 0;
  
  fp = sys_fopen(name,"r");
  if (!fp) {
    post("cxc/proc.c: unable to open %s",name);
    return;
  }
  //*name = 0;
  while(!feof(fp))
    {
/*     fread(bla,1,1,fp); */
/*     post("cxc/proc.c: '%s'",bla); */
    //ac = fscanf(fp,"%s\t: %s",name,rest);
    ac = fscanf(fp,"%s\t: %f",name,&restf);
    // fscanf(fp,"%s\t%s %d",name,rest,&val);
    if((!strcmp("MHz",name) ||
	!strcmp("processor",name) ||
	!strcmp("bogomips",name)) &&
       ac != -1)
      {
	if(!strcmp("MHz",name)) {
	  //val = (t_float)sprintf("%f",rest);
	  SETFLOAT(at+atc,restf);
	  atc++;
	} else if (!strcmp("processor",name)) {
	  SETFLOAT(at+atc,restf);
	  atc++;
	} else if (!strcmp("bogomips",name)) {
	  SETFLOAT(at+atc,restf);
	  atc++;
	}
#ifdef DEBUG
	post("cxc/proc.c: count %d, '%s' -> '%f'",ac,name,restf);
#endif
      }
    }
 outlet_anything(x->x_obj.ob_outlet, gensym("cpuinfo"),atc,at);
}

/////////////////////////////////////////////////////////////////
// more generic proc function: file to open is the message itself
void proc_proc(t_proc *x, t_symbol *s)
{
  char name[255];        // filename
  char rest[255];        // string value
  t_float restf;         // float value
  t_float restg;         // another float, needing more etc
  t_float resth;         // another float, needing more etc
                         // unsmart, convert the whole line
                         // into an atom vector or something ...
  int a,b,c;             // few helper ints for scanf
  char bla[1];
  FILE* fp;              // info file descriptor
  //  t_float val;
  int   ac;              // atom count
  t_atom* at = x->x_at;  // out atom
  int atc    = x->x_atc; // out atom count
  
  //post("cxc/proc.c: %s",s->s_name);

  //sprintf(name,"%s","/proc/cpuinfo");
  sprintf(name,"/proc/%s",s->s_name);
  //val   = 0;
  restf = 0;
  ac    = 0;
  
  fp = sys_fopen(name,"r");
  if (!fp) {
    post("cxc/proc.c: unable to open %s",name);
    return;
  }
  //*name = 0;
  while(!feof(fp)) {
    /*     fread(bla,1,1,fp); */
    /*     post("cxc/proc.c: '%s'",bla); */
    //ac = fscanf(fp,"%s\t: %s",name,rest);
//    ac = fscanf(fp,"%s\t: %f",name,&restf);
    // fscanf(fp,"%s\t%s %d",name,rest,&val);

    //////////////////////////////////////////////////
    // cpuinfo (rel. compl. parse)
    if(!strcmp("cpuinfo",s->s_name)) {
      ac = fscanf(fp,"%s\t: %f",name,&restf);

#ifdef DEBUG
      if((!strcmp("MHz",name) ||
	  !strcmp("processor",name) ||
	  !strcmp("bogomips",name)) &&
	 ac != -1) {
	post("cxc/proc.c: count %d, '%s' -> '%f'",ac,name,restf);
      }
#endif

      if(!strcmp("MHz",name)) {
	//val = (t_float)sprintf("%f",rest);
	SETFLOAT(at+atc,restf);
	atc++;
      } else if (!strcmp("processor",name)) {
	SETFLOAT(at+atc,restf);
	atc++;
      } else if (!strcmp("bogomips",name)) {
	SETFLOAT(at+atc,restf);
	atc++;
      }
      //////////////////////////////////////////////////
      // uptime (easy, just two floats ...
    } else if (!strcmp("uptime",s->s_name)) {
      ac = fscanf(fp,"%f %f",&restf,&restg);
      SETFLOAT(at+atc,restf); atc++;
      SETFLOAT(at+atc,restg); atc++;
      // why break here?
      break;
      //////////////////////////////////////////////////
      // loadavg
    } else if (!strcmp("loadavg",s->s_name)) {
      ac = fscanf(fp,"%f %f %f %d/%d %d",&restf,&restg,&resth,&a,&b,&c);
      SETFLOAT(at+atc,restf); atc++;
      SETFLOAT(at+atc,restg); atc++;
      SETFLOAT(at+atc,resth); atc++;
      SETFLOAT(at+atc,(t_float)a); atc++;
      SETFLOAT(at+atc,(t_float)b); atc++;
      SETFLOAT(at+atc,(t_float)c); atc++;
      // why break here?
      break;
      //////////////////////////////////////////////////
      // version (linux version)
    } else if (!strcmp("version",s->s_name)) {
      ac = fscanf(fp,"%s %s %d.%d.%d *",name,rest,&a,&b,&c);
      SETFLOAT(at+atc,a); atc++;
      SETFLOAT(at+atc,b); atc++;
      SETFLOAT(at+atc,c); atc++;
      // why break here?
      break;
/*     } else if (!strcmp("stat",s->s_name)) { */
/*       break; */
    } else {
      post("cxc/proc.c: %s not yet implemented",s->s_name);
      break;
    }
  }
 fclose(fp);
 outlet_anything(x->x_obj.ob_outlet, gensym(s->s_name),atc,at);
}

void *proc_new(void)
{
    t_proc *x = (t_proc *)pd_new(proc_class);
    x->x_atc = 0;
    outlet_new(&x->x_obj, &s_anything);
    return (void *)x;
}

void proc_setup(void)
{
  // post("proc_setup");
    proc_class = class_new(gensym("proc"), (t_newmethod)proc_new, 0,
    	sizeof(t_proc), 0, 0);
    class_addmethod(proc_class, (t_method)proc_cpuinfo, gensym("cpuinfo"), 0);
    class_addmethod(proc_class, (t_method)proc_proc,    gensym("proc"),    A_SYMBOL);
    //class_addmethod(proc_class, (t_method)proc_RAND_MAX, gensym("RAND_MAX"), 0);
    //class_addmethod(proc_class, (t_method)proc_getenv, gensym("getenv"), A_SYMBOL);
    //class_addmethod(proc_class, (t_method)proc_setenv, gensym("setenv"), A_SYMBOL, A_SYMBOL);
    class_addfloat(proc_class, proc_float);
}

