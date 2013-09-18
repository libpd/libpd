/******************************************************
 *
 * This file is based on iem_objlist.h by IOhannes m zmölnig
 *
 * Original copyleft (c):
 *   IOhannes m zmölnig
 *
 *   2008:forum::für::umläute:2008
 *
 *   institute of electronic music and acoustics (iem)
 *
 ******************************************************
 *
 * license: GNU General Public License v.2
 *
 ******************************************************/

/* this file implements some helperr functions for dealing with lists of
 * objects (containing other objects)
 *
 * used for callbacks to enumerated objects without $0 tricks
 */



//#include "m_pd.h"

/*
 The basic element is a parent.
 Elements can then be added/removed to/from this parent.
 
 */


/* ------------------------- helper methods for callbacks ---------------------------- */

typedef struct _objectlist_element {
  const t_pd* obj;
  struct _objectlist_element* next;
} t_objectlist_element;

typedef struct _objectlist_list {
  const t_pd* parent;
  t_objectlist_element* obj;

  struct _objectlist_list *next;
} t_objectlist_list;

static t_objectlist_list* static_list=0;


// Find a list with the specified parent
static t_objectlist_list* getList(const t_pd*parent) {
  t_objectlist_list* list = static_list;
  if(0==parent  || 0==list)
    return 0;

  for(list=static_list; list; list=list->next) {
    if(parent == list->parent) {
      return list;
    }
  }
  return 0; 
}

static t_objectlist_list* addList(const t_pd* parent)
{
  t_objectlist_list* list=getList(parent);
  if(!list) {
    list=(t_objectlist_list*)getbytes(sizeof(t_objectlist_list));
    list->parent=parent;
    list->obj=0;
    list->next=0;

    if(0==static_list) {
      /* new list */
      static_list=list;
    } else {
      /* add to the end of existing list */
      t_objectlist_list* dummy=static_list;
      while(dummy->next)
        dummy=dummy->next;
      dummy->next = list;
    }
  }
  return list;
}

static t_objectlist_element* getElements (const t_pd*parent) {
  t_objectlist_list* list = getList(parent);
  if(list)
    return list->obj;

  return 0;
}

// Add element to a list
static void addElement (const t_pd*parent, const t_pd*obj) {
  t_objectlist_list* p=addList(parent);
  t_objectlist_element* list=0;
  t_objectlist_element* entry=0;
  if(!p || !obj)
    return;
  list=p->obj;

  if(list&&obj==list->obj)
    return;

  while(list && list->next) {
    if(obj==list->obj) /* obj already in list */
      return;
    list=list->next;
  }

  /* we are at the end of the list that does not contain obj yet, so add it */
  entry=(t_objectlist_element*)getbytes(sizeof(t_objectlist_element));
  entry->obj=obj;
  entry->next=0;
  if(list) {
    list->next=entry;
  } else {
    p->obj=entry;
  }
}

// Remove element from list
static void removeElement(const t_pd*parent, const t_pd*obj) {
  t_objectlist_list* p=getList(parent);
  t_objectlist_element* list=0, *last=0, *next=0;
  if(!p || !obj)return;
  list=p->obj;
  if(!list)
    return;

  while(list && obj!=list->obj) {
    last=list;
    list=list->next;
  }

  if(!list) /* couldn't find this object */
    return;

  next=list->next;

  if(last)
    last->next=next;
  else
    p->obj=next;

  freebytes((void*)list, sizeof(t_objectlist_element));
  list=0;
}

// Remove element from all lists!
static void removeElementFromLists(const t_pd*obj) {
   t_objectlist_list* parents=static_list;

  while(parents) {
    removeElement(parents->parent, obj);
    parents=parents->next;
  }
}
