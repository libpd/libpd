/* 
 * liststorage: stores a number of lists
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
    this is heavily based on code from [textfile],
    which is part of pd and written by Miller S. Puckette
    pd (and thus [textfile]) come with their own license
*/

#include "zexy.h"

#ifdef __WIN32__
# include <io.h>
#else
# include <unistd.h>
#endif

#include <stdio.h>
#include <fcntl.h>
#include <string.h>



/* ****************************************************************************** */
/* liststorage : store several lists in a slots (array of lists of lists) */


/* a list of lists  */
typedef struct _msglist {
  int argc;
  t_atom *argv;
  struct _msglist *next;
} t_msglist;

typedef struct _liststorage
{
  t_object x_obj;              /* everything */

  t_outlet*x_dataout; /* where the data appears */
  t_outlet*x_infoout; /* where meta-information appears */

  t_inlet*x_slotin;   /* setting the current slot */

  int x_numslots, x_defaultnumslots;
  int x_currentslot;

  t_msglist**x_slots;
} t_liststorage;

static t_class *liststorage_class;


/* ************************************************************************ */
/* helper functions                                                           */

static t_msglist*_liststorage_getslot(t_liststorage*x, int slot) {
  //  post("getting slot %d of %d|%d", slot, 0, x->x_numslots);
  if(slot<0 || slot>=x->x_numslots) { pd_error(x, "[liststorage]: attempting to access invalid slot %d", slot); return NULL;  }
  return x->x_slots[slot];
}

static void _liststorage_deletemsglist(t_msglist*list) {
  t_msglist*x=list;
  while(x) {
    t_msglist*y=x;
    int i=0;
    x=x->next;

    freebytes(y->argv, y->argc*sizeof(t_atom));
    y->argc=0;
    y->argv=NULL;
    y->next=NULL;
    freebytes(y, sizeof(t_msglist));
  }
}

static void _liststorage_deleteslot(t_liststorage*x, int slot) {
  t_msglist*list=_liststorage_getslot(x, slot);
  if(list) {
    _liststorage_deletemsglist(list);
    x->x_slots[slot]=NULL;
  }
}

static t_msglist*_liststorage_newslot(int argc, t_atom*argv) {
  t_msglist*slot=getbytes(sizeof(t_msglist));
  int i=0;

  slot->argv=getbytes(sizeof(t_atom)*argc);
  for(i=0; i<argc; i++) {
    slot->argv[i]=argv[i];
  }

  slot->argc=argc;
  slot->next=NULL;

  return slot;
}


static t_msglist*_liststorage_add2slot(t_msglist*slot, int argc, t_atom*argv) {
  t_msglist*dummy=slot;
  t_msglist*newlist=_liststorage_newslot(argc, argv);
  if(NULL==slot) {
    //    post("no data yet: new data is %x", newlist);
    return newlist;
  }

  while(dummy->next) {
    dummy=dummy->next;
  }
  dummy->next=newlist;
  //  post("added data to slot @ %x", slot);
  return slot;
}


static int _liststorage_resize(t_liststorage*x, int size) {
  t_msglist**newarray=NULL;
  int i=0;

  if(size<0) {
    pd_error(x, "[liststorage]: refusing to resize for negative amount of slots");
    return 0;
  }

  if(size==x->x_numslots) {
    verbose(1, "[liststorate] no need to resize array");
    return size;
  }

  /* create a new array */
  newarray=getbytes(sizeof(t_msglist*)*size);
  for(i=0; i<size; i++) {
    newarray[i]=NULL;
  }

  /* copy over all the data from the old array (if there was one */
  i=(size<x->x_numslots)?size:x->x_numslots;
  while(i-->0) {
    newarray[i]=x->x_slots[i];
    x->x_slots[i]=NULL;
  }

  /* delete the old array */
  for(i=0; i<x->x_numslots; i++) {
    _liststorage_deleteslot(x, i);
  }
  freebytes(x->x_slots, sizeof(t_msglist*));

  /* make the new array the current */
  x->x_slots=newarray;
  x->x_numslots=size;

  return size;
}

static int _liststorage_checkslot(t_liststorage*x, const char*string, const int resize) {
  int slot=x->x_currentslot;
  t_atom atom;
  SETFLOAT(&atom, (t_float)slot);

  if(slot<0) { 
    if(NULL!=string)pd_error(x, "[liststorage]: %s %d", string, slot); 
    outlet_anything(x->x_infoout, gensym("invalidslot"), 1, &atom);
    return -1;
  }
  if(slot>=x->x_numslots) {
    if(resize) {
      _liststorage_resize(x, slot+1);
    } else {
      if(NULL!=string)pd_error(x, "[liststorage]: %s %d", string, slot);
      outlet_anything(x->x_infoout, gensym("invalidslot"), 1, &atom);
      return -1;
    }
  }
  return slot;
}
/* ************************************************************************ */
/* object methods                                                           */

  /* recall all lists from the current slot */
static void liststorage_bang(t_liststorage *x)
{ 
  t_atom atom;
  t_msglist*list=NULL;

  int slot=_liststorage_checkslot(x, "attempting to read data from invalid slot", 0);
  if(slot<0)return;
  list=_liststorage_getslot(x, slot);

  while(list) {
    outlet_list(x->x_dataout, gensym("list"), list->argc, list->argv);
    list=list->next;
  }
  SETFLOAT(&atom, (t_float)slot);

  /* no need for done: use [t b b b] to get beginning and end of output */
  //  outlet_anything(x->x_infoout, gensym("done"), 1, &atom);
}

  /* add a new list to the current slot */
static void liststorage_add(t_liststorage *x, t_symbol *s, int ac, t_atom *av)
{
  t_msglist*list=NULL;
  int slot=_liststorage_checkslot(x, "attempting to add data to invalid slot", 1);
  if(slot<0)return;
  list=_liststorage_getslot(x, slot);
  x->x_slots[slot]=_liststorage_add2slot(x->x_slots[slot], ac, av);
}

  /* clear the current slot */
static void liststorage_clear(t_liststorage *x)
{
  int slot=_liststorage_checkslot(x, "attempting to clear invalid slot", 0);
  if(slot<0)return;

  _liststorage_deleteslot(x, slot);
}

  /* clear all slots */
static void liststorage_clearall(t_liststorage *x)
{
  int i=0;
  for(i=0; i<x->x_numslots; i++) {
    _liststorage_deleteslot(x, i);
  }
}

  /* insert an empty slot at (before) given position */
static void liststorage_insert(t_liststorage *x, t_floatarg f)
{
  int current=x->x_currentslot;
  int slot=-1;
  int i=0;

  x->x_currentslot=f;
  slot=_liststorage_checkslot(x, "attempting to insert invalid slot", 1);
  x->x_currentslot=current;

  if(slot<0)return;

  _liststorage_resize(x, x->x_numslots+1); 

  for(i=x->x_numslots-1; i>slot; i--) {
    x->x_slots[i]=x->x_slots[i-1];
  }
  x->x_slots[slot]=NULL;  
}

  /* get the number of slots */
static void liststorage_info(t_liststorage *x)
{
  t_atom ap;
  SETFLOAT(&ap, (t_float)x->x_numslots);
  outlet_anything(x->x_infoout, gensym("numslots"), 1, &ap);
}


  /* get the number of slots */
static void liststorage_slot(t_liststorage *x, t_floatarg f)
{
  int slot=f;
  x->x_currentslot=slot;

}


  /* remove empty slots */
static void liststorage_compress(t_liststorage *x)
{
  t_msglist**newarray=NULL;
  int i=0, j=0;
  int size=0;
  for(i=0; i<x->x_numslots; i++) {
    if(NULL!=x->x_slots[i]) {
      
      size++;
    }
  }

  if(size>=x->x_numslots) {
    //    post("incomressible: %d of %d", size, x->x_numslots);
    return;
  }

  if(size<x->x_defaultnumslots)
    size=x->x_defaultnumslots;

  /* create a new array */
  newarray=getbytes(sizeof(t_msglist*)*size);
  for(i=0; i<size; i++) {
    newarray[i]=NULL;
  }

  /* copy over all the data from the old array (if there was one */
  for(i=0, j=0; i<x->x_numslots; i++) {
    if(NULL!=x->x_slots[i]) {
      newarray[j]=x->x_slots[i];
      j++;
    }
    x->x_slots[i]=NULL;
  }

  /* delete the old array */
  for(i=0; i<x->x_numslots; i++) {
    _liststorage_deleteslot(x, i);
  }
  freebytes(x->x_slots, sizeof(t_msglist*));

  /* make the new array the current */
  x->x_slots=newarray;
  x->x_numslots=size;
}


/* ************************************************************************ */
/* constructor/destructor                                                   */

static void liststorage_free(t_liststorage *x)
{
  liststorage_clearall(x);
  _liststorage_resize(x, 0);
}

/* constructor: argument is initial number of slots (can grow) */
static void *liststorage_new(t_floatarg f)
{
  t_liststorage *x = (t_liststorage *)pd_new(liststorage_class);
  int slots=f;

  x->x_slotin=inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("slot"));
  x->x_dataout=outlet_new(&x->x_obj, gensym("list"));
  x->x_infoout=outlet_new(&x->x_obj, 0);



  if(slots<=0)slots=20;
  x->x_defaultnumslots=slots;
  x->x_numslots=0;
  x->x_currentslot=0;
  x->x_slots=NULL;
  
  _liststorage_resize(x, x->x_defaultnumslots);



  return (x);
}

void liststorage_setup(void)
{
  liststorage_class = class_new(gensym("liststorage"), (t_newmethod)liststorage_new,
                            (t_method)liststorage_free, sizeof(t_liststorage), 0, A_DEFFLOAT, 0);

  /* recall all lists from the current slot */
  class_addbang(liststorage_class, (t_method)liststorage_bang);

  /* add a new list to the current slot */
  class_addmethod(liststorage_class, (t_method)liststorage_add, gensym("add"), A_GIMME, 0);
  /* clear the current slot */
  class_addmethod(liststorage_class, (t_method)liststorage_clear, gensym("clear"), 0);
  /* clear all slots */
  class_addmethod(liststorage_class, (t_method)liststorage_clearall, gensym("clearall"), 0);


  /* add a new list to the current slot */
  class_addmethod(liststorage_class, (t_method)liststorage_slot, gensym("slot"), A_FLOAT, 0);


  /* insert an empty slot at (before) given position */
  class_addmethod(liststorage_class, (t_method)liststorage_insert, gensym("insert"), A_DEFFLOAT, 0);

 /* remove empty slots */
  class_addmethod(liststorage_class, (t_method)liststorage_compress, gensym("compress"), 0);


  /* get the number of slots */
  class_addmethod(liststorage_class, (t_method)liststorage_info, gensym("info"), 0);

  zexy_register("liststorage");
}
