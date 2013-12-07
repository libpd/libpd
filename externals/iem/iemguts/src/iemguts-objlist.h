/******************************************************
 *
 * iemguts - implementation file
 *
 * copyleft (c) IOhannes m zmölnig
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

/* 
 * TODO: documentation
 */

#include "m_pd.h"


/* ------------------------- helper methods for callbacks ---------------------------- */

typedef struct _iemguts_objlist {
  const t_pd*obj;
  struct _iemguts_objlist*next;
} t_iemguts_objlist;

typedef struct _iemguts_canvaslist {
  const t_pd*parent;
  t_iemguts_objlist*obj;

  struct _iemguts_canvaslist*next;
} t_iemguts_canvaslist;

static t_iemguts_canvaslist*s_canvaslist=0;


static t_iemguts_canvaslist*findCanvas(const t_pd*parent) {
  t_iemguts_canvaslist*list=s_canvaslist;
  if(0==parent  || 0==list)
    return 0;

  for(list=s_canvaslist; list; list=list->next) {
    if(parent == list->parent) {
      return list;
    }
  }
  return 0; 
}

static t_iemguts_canvaslist*addCanvas(const t_pd*parent)
{
  t_iemguts_canvaslist*list=findCanvas(parent);
  if(!list) {
    list=(t_iemguts_canvaslist*)getbytes(sizeof(t_iemguts_canvaslist));
    list->parent=parent;
    list->obj=0;
    list->next=0;

    if(0==s_canvaslist) {
      /* new list */
      s_canvaslist=list;
    } else {
      /* add to the end of existing list */
      t_iemguts_canvaslist*dummy=s_canvaslist;
      while(dummy->next)
        dummy=dummy->next;
      dummy->next = list;
    }
  }
  return list;
}

static t_iemguts_objlist*objectsInCanvas(const t_pd*parent) {
  t_iemguts_canvaslist*list=findCanvas(parent);
  if(list)
    return list->obj;

  return 0;
}

static void addObjectToCanvas(const t_pd*parent, const t_pd*obj) {
  t_iemguts_canvaslist*p=addCanvas(parent);
  t_iemguts_objlist*list=0;
  t_iemguts_objlist*entry=0;
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
  entry=(t_iemguts_objlist*)getbytes(sizeof(t_iemguts_objlist));
  entry->obj=obj;
  entry->next=0;
  if(list) {
    list->next=entry;
  } else {
    p->obj=entry;
  }
}

static void removeObjectFromCanvas(const t_pd*parent, const t_pd*obj) {
  t_iemguts_canvaslist*p=findCanvas(parent);
  t_iemguts_objlist*list=0, *last=0, *next=0;
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

  freebytes((void*)list, sizeof(t_iemguts_objlist));
  list=0;
}

static void removeObjectFromCanvases(const t_pd*obj) {
   t_iemguts_canvaslist*parents=s_canvaslist;

  while(parents) {
    removeObjectFromCanvas(parents->parent, obj);
    parents=parents->next;
  }
}
