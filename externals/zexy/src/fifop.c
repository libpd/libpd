/* 
 * fifop: a FIFO (first-in first-out) with priorities
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

/* ------------------------- fifop ------------------------------- */

/*
 * a FIFO (first-in first-out) with priorities
 *
 * an incoming list is added to a fifo (based on its priority)
 * "bang" outputs the next element of the non-empty fifo with the highest priority
 *
 * high priority means low numeric value
 */

static t_class *fifop_class;

typedef struct _fifop_list {
  int                 argc;
  t_atom             *argv;
  struct _fifop_list *next;
} t_fifop_list;

typedef struct _fifop_prioritylist {
  t_float                     priority;
  t_fifop_list               *fifo_start;
  t_fifop_list               *fifo_end;
  struct _fifop_prioritylist *next;
} t_fifop_prioritylist;

typedef struct _fifop
{
  t_object              x_obj;
  t_fifop_prioritylist *fifo_list;
  t_float               priority; /* current priority */
  unsigned long               counter;
  t_outlet             *x_out, *x_infout;
} t_fifop;

static t_fifop_prioritylist*fifop_genprioritylist(t_fifop*x, t_float priority)
{
  t_fifop_prioritylist*result=0, *dummy=0;

  if(x->fifo_list!=0)
    {
      /*
       * do we already have this priority ?
       * if so, just return a pointer to that fifo
       * else set the dummy-pointer to the fifo BEFORE the new one
       */
      dummy=x->fifo_list;
      while(dummy!=0){
        t_float prio=dummy->priority;
        if(prio==priority)return dummy;
        if(prio>priority)break;
        result=dummy;
        dummy=dummy->next;
      }
      dummy=result; /* dummy points to the FIFO-before the one we want to insert */
    }
  /* create a new priority list */
  result = (t_fifop_prioritylist*)getbytes(sizeof( t_fifop_prioritylist));
  result->priority=priority;
  result->fifo_start=0;
  result->fifo_end=0;
  result->next=0;

  /* insert it into the list of priority lists */
  if(dummy==0){
    /* insert at the beginning */
    result->next=x->fifo_list;
    x->fifo_list=result;
  } else {
    /* post insert into the list of FIFOs */
    result->next=dummy->next;
    dummy->next =result;
  }

  /* return the result */
  return result;
}

static int add2fifo(t_fifop_prioritylist*fifoprio, int argc, t_atom *argv)
{
  t_fifop_list*fifo=0;
  t_fifop_list*entry=0;

  if(fifoprio==0){
        error("pfifo: no fifos available");
    return -1;
  }

  /* create an entry for the fifo */
  if(!(entry = (t_fifop_list*)getbytes(sizeof(t_fifop_list))))
    {
      error("pfifo: couldn't add entry to end of fifo");
      return -1;
    }
  if(!(entry->argv=(t_atom*)getbytes(argc*sizeof(t_atom)))){
    error("pfifo: couldn't add list to fifo!");
    return -1;
  }
  memcpy(entry->argv, argv, argc*sizeof(t_atom));
  entry->argc=argc;
  entry->next=0;

  /* insert entry into fifo */
  if(fifoprio->fifo_end){
    /* append to the end of the fifo */
    fifo=fifoprio->fifo_end;

    /* add new entry to end of fifo */
    fifo->next=entry;
    fifoprio->fifo_end=entry;    
  } else {
    /* the new entry is the 1st entry of the fifo */
    fifoprio->fifo_start=entry;
    /* and at the same time, it is the last entry */
    fifoprio->fifo_end  =entry;
  }
  return 0;
}
static t_fifop_prioritylist*getFifo(t_fifop_prioritylist*pfifo)
{
  if(pfifo==0)return 0;
  /* get the highest non-empty fifo */
  while(pfifo->fifo_start==0 && pfifo->next!=0)pfifo=pfifo->next;
  return pfifo;
}

static void fifop_list(t_fifop *x, t_symbol *s, int argc, t_atom *argv)
{
  t_fifop_prioritylist*pfifo=0;
  ZEXY_USEVAR(s);
  if(!(pfifo=fifop_genprioritylist(x, x->priority))) {
    error("[fifop]: couldn't get priority fifo");
    return;
  }
  if(!add2fifo(pfifo, argc, argv))
    {
      x->counter++;
    }
}
static void fifop_bang(t_fifop *x)
{
  t_fifop_prioritylist*pfifo=0;
  t_fifop_list*fifo=0;
  t_atom*argv=0;
  int argc=0;

  if(!(pfifo=getFifo(x->fifo_list))){
    outlet_bang(x->x_infout);
    return;
  }
  if(!(fifo=pfifo->fifo_start)){
    outlet_bang(x->x_infout);
    return;
  }

  x->counter--;

  pfifo->fifo_start=fifo->next;
  if(0==pfifo->fifo_start){
    pfifo->fifo_end=0;
  }
  /* get the list from the entry */
  argc=fifo->argc;
  argv=fifo->argv;

  fifo->argc=0;
  fifo->argv=0;
  fifo->next=0;

  /* destroy the fifo-entry (important for recursion! */
  freebytes(fifo, sizeof(t_fifop_list));

  /* output the list */
  outlet_list(x->x_out, gensym("list"), argc, argv);

  /* free the list */
  freebytes(argv, argc*sizeof(t_atom));
}
static void fifop_query(t_fifop*x)
{
  z_verbose(1, "%d elements in fifo", (int)x->counter);
  
  outlet_float(x->x_infout, (t_float)x->counter);
}

static void fifop_clear(t_fifop*x)
{
  t_fifop_prioritylist *fifo_list=x->fifo_list;
  while(fifo_list){
    t_fifop_prioritylist *fifo_list2=fifo_list;

    t_fifop_list*fifo=fifo_list2->fifo_start;
    fifo_list=fifo_list->next;

    while(fifo){
      t_fifop_list*fifo2=fifo;
      fifo=fifo->next;

      if(fifo2->argv)freebytes(fifo2->argv, fifo2->argc*sizeof(t_atom));
      fifo2->argv=0;
      fifo2->argc=0;
      fifo2->next=0;
      freebytes(fifo2, sizeof(t_fifop_list));
    }
    fifo_list2->priority  =0;
    fifo_list2->fifo_start=0;
    fifo_list2->fifo_end  =0;
    fifo_list2->next      =0;
    freebytes(fifo_list2, sizeof( t_fifop_prioritylist));
  }
  x->fifo_list=0;
  x->counter=0;
}
/* this is NOT re-entrant! */
static void fifop_dump(t_fifop*x)
{  
  t_fifop_prioritylist*pfifo=getFifo(x->fifo_list);

  if(!pfifo||!pfifo->fifo_start) {
    outlet_bang(x->x_infout);
    return;
  }

  while(pfifo) {
    t_fifop_list*fifo=pfifo->fifo_start;
    while(fifo) {
      t_atom*argv=fifo->argv;
      int argc=fifo->argc;

      /* output the list */
      outlet_list(x->x_out, gensym("list"), argc, argv);

      fifo=fifo->next;
    }
    pfifo=pfifo->next;
  }
}

static void fifop_help(t_fifop*x)
{
  post("\n%c fifop\t\t:: a First-In-First-Out queue with priorities", HEARTSYMBOL);
}


static void fifop_free(t_fifop *x)
{
  fifop_clear(x);

  outlet_free(x->x_out);
  outlet_free(x->x_infout);
}

static void *fifop_new(void)
{
  t_fifop *x = (t_fifop *)pd_new(fifop_class);

  floatinlet_new(&x->x_obj, &x->priority);
  x->x_out   =outlet_new(&x->x_obj, gensym("list" ));
  x->x_infout=outlet_new(&x->x_obj, gensym("float"));

  x->fifo_list = 0;
  x->priority=0;

  return (x);
}

void fifop_setup(void)
{
  fifop_class = class_new(gensym("fifop"), (t_newmethod)fifop_new,
                             (t_method)fifop_free, sizeof(t_fifop), 0, A_NULL);

  class_addbang    (fifop_class, fifop_bang);
  class_addlist    (fifop_class, fifop_list);

  class_addmethod  (fifop_class, (t_method)fifop_clear, gensym("clear"), A_NULL);
  class_addmethod  (fifop_class, (t_method)fifop_dump, gensym("dump"), A_NULL);

  class_addmethod  (fifop_class, (t_method)fifop_query, gensym("info"), A_NULL);
  class_addmethod  (fifop_class, (t_method)fifop_help, gensym("help"), A_NULL);

  zexy_register("fifop");
}
