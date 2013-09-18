////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.at
//
// Implementation file
//
//    Copyright (c) 2008-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "gemreceive.h"

#if 0
# define debug_post ::post
#else
# define debug_post 
#endif

static t_class *gemreceive_proxy_class;

struct _bind_element {
  gemreceive *object;
  t_float priority;
  struct _bind_element*next;
};

struct _gemreceive_proxy {
  t_object p_obj;

  t_symbol*key;
  t_bind_element*elements;
  struct _gemreceive_proxy*next;
};





t_gemreceive_proxy*gemreceive :: proxy_list = NULL;

t_gemreceive_proxy* gemreceive::find_key(t_symbol*key)
{
  t_gemreceive_proxy*binding=0;

  for(binding=proxy_list; binding; binding=binding->next) {
    if(binding->key == key)
      return binding;
  }
  /* not found */
  return 0;
}

t_gemreceive_proxy*gemreceive::add_key(t_symbol*key)
{
  t_gemreceive_proxy*bind_list=0;
  bind_list=reinterpret_cast<t_gemreceive_proxy*>(pd_new(gemreceive_proxy_class));
  bind_list->key=key;
  bind_list->elements=0;
  bind_list->next=0;

  debug_post("binding %x to %s", bind_list, key->s_name);
  pd_bind(&bind_list->p_obj.ob_pd, key);

  t_gemreceive_proxy*last=proxy_list;

  if(last) {
    while(last->next) {
      last=last->next;
    }
    last->next = bind_list;
  } else {
    proxy_list = bind_list;
  }

  return bind_list;
}

void gemreceive::add_element(t_gemreceive_proxy*bind_list, t_bind_element*element)
{
  /* insert the object according to it's priority 
   * this is already the right queue
   */
  t_float priority=element->priority;
  t_bind_element*elements=bind_list->elements, *last=0;
  debug_post("priority insert of %x:%g", element, priority);

  if(!elements || elements->priority >= priority) {
    bind_list->elements = element;
    element->next = elements;
    debug_post("inserting in the beginngin");
    return;
  }


  debug_post("trying %x:%g", elements, elements->priority);
  while(elements && elements->priority < priority) {
    debug_post("skipping %x:%g to %x", elements, elements->priority, elements->next);
    last=elements;
    elements=elements->next;
  }

  debug_post("inserting after %x:%g", last,     (last    ?    (last->priority):0));
  debug_post("inserting befor %x:%g", elements, (elements?(elements->priority):0));

  element->next=elements;    
  if(last) {
    last->next = element;
  } else {
    bug("\nlast object invalid when inserting prioritized receiver\n");
  }
}


void gemreceive::bind(gemreceive*x, t_symbol*key, t_float priority) {
  t_gemreceive_proxy*bind_list=0;
  t_bind_element*element=0;
  debug_post("trying to bind 0x%X:: '%s':%g via %x", x, key->s_name, priority, proxy_list);

  bind_list=find_key(key);
  if(!bind_list)
    bind_list=add_key(key);
  if(!bind_list)return;

  element=(t_bind_element*)getbytes(sizeof(t_bind_element));

  element->object=x;

  element->priority=priority;
  element->next=0;

  add_element(bind_list, element);
}


void gemreceive::unbind(gemreceive*x, t_symbol*key) {
  t_gemreceive_proxy*list=0, *last=0;
  t_bind_element*elements=0, *lastlmn=0;

  debug_post("trying to unbind 0x%X:: '%s' from %x", x, key->s_name, proxy_list);

  for(list=proxy_list; list && list->key!=key; list=list->next){
    last=list;
  }
  if(!list)return;

  for(elements=list->elements; elements && elements->object != x; elements=elements->next) {
    lastlmn=elements;
  }

  if(elements) {
    /* found it, now remove it */
    if(lastlmn) {
      lastlmn->next=elements->next;
    } else {
      list->elements=elements->next;
    }

    elements->object=0;
    elements->priority=0;
    elements->next=0;

    freebytes(elements, sizeof(elements));
  } else {
    // not here...
  }

  if(0==list->elements) {
    // should we unbind ??
    if(last) {
      last->next=list->next;
    } else {
      proxy_list=list->next;
    }    

    pd_unbind(&list->p_obj.ob_pd, list->key);
    list->next=0;
    list->key=0;

    pd_free(&list->p_obj.ob_pd);
  }
}







CPPEXTERN_NEW_WITH_TWO_ARGS(gemreceive, t_symbol*, A_DEFSYMBOL, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// gemreceive
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
gemreceive :: gemreceive(t_symbol*s,t_floatarg f) :
  m_name(s), m_priority(f),
  m_outlet(NULL), m_fltin(NULL)
{
  debug_post("hi, i am gemreceive 0x%X", this);


  m_fltin = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float,  gensym(""));
  m_outlet = outlet_new(this->x_obj, 0);

  bind(this, m_name, m_priority);

}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
gemreceive :: ~gemreceive()
{
  unbind(this, m_name);

  if(m_fltin)inlet_free(m_fltin);m_fltin=NULL;
  if(m_outlet)outlet_free(m_outlet);m_outlet=NULL;
}


/////////////////////////////////////////////////////////
// receiving
//
/////////////////////////////////////////////////////////
void gemreceive :: receive(t_symbol*s, int argc, t_atom*argv)
{
  debug_post("receiveing....%x", m_outlet);
  outlet_anything(m_outlet, s, argc, argv);
}


void gemreceive :: nameMess(t_symbol*s) {
  if(m_name) {
    unbind(this, m_name);
  }
  m_name=s;
  bind(this, m_name, m_priority);
}

void gemreceive :: priorityMess(t_float f) {
  m_priority=f;
  if(m_name) {
    unbind(this, m_name);
    bind(this, m_name, m_priority);
  }
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void gemreceive :: obj_setupCallback(t_class *classPtr)
{ 
  class_addsymbol(classPtr, reinterpret_cast<t_method>(gemreceive::nameCallback));
  class_addmethod(classPtr, reinterpret_cast<t_method>(gemreceive::priorityCallback), gensym(""), A_FLOAT, 0);


  gemreceive_proxy_class = class_new(0, 0, 0, 
                                     sizeof(t_gemreceive_proxy), 
                                     CLASS_NOINLET | CLASS_PD, 
                                     A_NULL);
  class_addanything(gemreceive_proxy_class, reinterpret_cast<t_method>(gemreceive::proxyCallback));
}

void gemreceive :: proxyCallback(t_gemreceive_proxy*p, t_symbol*s, int argc, t_atom*argv)
{
  t_bind_element*elements=p->elements;

  debug_post("proxy anything: %x", p);

  while(elements) {
    gemreceive*o=elements->object;
    elements=elements->next;
    debug_post("proxy for 0x%X", o);
    if(o) {
      o->receive(s, argc, argv);
    }
  } 
}



void gemreceive :: nameCallback(void *data, t_symbol*s)
{
  GetMyClass(data)->nameMess(s);
}
void gemreceive :: priorityCallback(void *data, t_float f)
{
  GetMyClass(data)->priorityMess(f);
}
