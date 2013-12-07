
/******************************************************
 *
 * oreceive - implementation file
 *
 * copyleft (c) IOhannes m zmölnig
 *
 *   2307:forum::für::umläute:2008
 *
 *   institute of electronic music and acoustics (iem)
 *   unsiversity of music and dramatic arts graz (kug)
 *
 ******************************************************
 *
 * license: GNU General Public License v.2
 *
 ******************************************************/


#include "m_pd.h"

#if 0
# define debug_post post
#else
# define debug_post
#endif

static t_class *oreceive_class, *oreceive_proxy_class, *oreceive_guts_class;


/* ------------------------------------------------------------- */

/* 
 * [oreceive] : ordered receive
 *
 *
 * there come the guts: our own bind/unbind mechanism that
 * includes priorities
 *
 * the plan is as follows:
 *  [oreceive] tells us, which name it wants to bind to; 
 *   it doesn't _really_ get bound to this name!
 *
 *  we have an invisible permanent object, that keeps track of 
 *  sorted (by priority) lists of the [oreceive] objects
 *
 *  this object binds itself to the requested name
 *  and when it gets called, it in turn calls all the [oreceive] objects
 *
 * that's it
 */


typedef struct _bind_element {
  t_pd   *object;
  t_float priority;
  struct _bind_element*next;
} t_bind_element;

typedef struct _bind_list {
  t_object p_obj;

  t_symbol*key;
  t_bind_element*elements;
  struct _bind_list*next;
} t_oreceive_proxy;

static t_oreceive_proxy*priority_receiver=0;

static t_oreceive_proxy*guts_find_key(t_symbol*key)
{
  t_oreceive_proxy*binding=0;

  for(binding=priority_receiver; binding; binding=binding->next) {
    if(binding->key == key)
      return binding;
  }
  /* not found */
  return 0;
}


static t_oreceive_proxy*guts_add_key(t_symbol*key)
{
  t_oreceive_proxy*bind_list=0;
  bind_list=(t_oreceive_proxy*)pd_new(oreceive_proxy_class);
  bind_list->key=key;
  bind_list->elements=0;
  bind_list->next=0;

  debug_post("binding %x to %s", bind_list, key->s_name);
  pd_bind(&bind_list->p_obj.ob_pd, key);

  t_oreceive_proxy*last=priority_receiver;

  if(last) {
    while(last->next) {
      last=last->next;
    }
    last->next = bind_list;
  } else {
    priority_receiver = bind_list;
  }

  return bind_list;
}

static void guts_add_element(t_oreceive_proxy*bind_list, t_bind_element*element)
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


static void pd_bind_priority(t_pd*x, t_symbol*key, t_float priority) {
  t_oreceive_proxy*bind_list=0;
  t_bind_element*element=0;
  debug_post("trying to bind '%s':%g via %x", key->s_name, priority, priority_receiver);

  bind_list=guts_find_key(key);
  if(!bind_list)
    bind_list=guts_add_key(key);
  if(!bind_list)return;

  element=(t_bind_element*)getbytes(sizeof(t_bind_element));
  element->object=x;
  element->priority=priority;
  element->next=0;

  guts_add_element(bind_list, element);
}


static void pd_unbind_priority(t_pd*x, t_symbol*key) {
  t_oreceive_proxy*list=0, *last=0;
  t_bind_element*elements=0, *lastlmn=0;

  debug_post("trying to unbind '%s' from %x", key->s_name, priority_receiver);

  for(list=priority_receiver; list && list->key!=key; list=list->next){
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
      priority_receiver=list->next;
    }    

    pd_unbind(&list->p_obj.ob_pd, list->key);
    list->next=0;
    list->key=0;

    pd_free(&list->p_obj.ob_pd);
  }
}

/* -------------------- oreceive ------------------------------ */

typedef struct _oreceive
{
  t_object x_obj;
  t_symbol *x_sym;
  t_float   x_priority;

  t_inlet*x_symin,*x_fltin;
} t_oreceive;

static void oreceive_anything(t_oreceive *x, t_symbol *s, int argc, t_atom *argv);

static void oreceive_proxy_anything(t_oreceive_proxy*p, t_symbol*s, int argc, t_atom*argv)
{
  t_bind_element*elements=p->elements;

  debug_post("proxy anything: %x", p);

  while(elements) {
    t_pd*o=elements->object;
    elements=elements->next;
    if(o) {
      oreceive_anything((t_oreceive*)o, s, argc, argv);
    }
  }
}

/* ------------------------------------------------------------- */

static void oreceive_anything(t_oreceive *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_anything(x->x_obj.ob_outlet, s, argc, argv);
}

static void oreceive_priority(t_oreceive *x, t_float p)
{
  x->x_priority=p;
  if(x->x_sym) {
    pd_unbind_priority(&x->x_obj.ob_pd, x->x_sym);
  }
  pd_bind_priority(&x->x_obj.ob_pd, x->x_sym, x->x_priority);
}

static void oreceive_name(t_oreceive *x, t_symbol*s)
{
  if(x->x_sym) {
    pd_unbind_priority(&x->x_obj.ob_pd, x->x_sym);
  }
  x->x_sym=s;
  pd_bind_priority(&x->x_obj.ob_pd, x->x_sym, x->x_priority);
}


static void *oreceive_new(t_symbol *s, t_float priority)
{
    t_oreceive *x = (t_oreceive *)pd_new(oreceive_class);

    x->x_symin = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_symbol, &s_symbol);
    x->x_fltin = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, &s_float);

    x->x_sym = s;
    x->x_priority = priority;
    pd_bind_priority(&x->x_obj.ob_pd, s, priority);
    outlet_new(&x->x_obj, 0);
    return (x);
}

static void oreceive_free(t_oreceive *x)
{
    pd_unbind_priority(&x->x_obj.ob_pd, x->x_sym);
}

void oreceive_setup(void)
{
    oreceive_class = class_new(gensym("oreceive"), (t_newmethod)oreceive_new, 
                               (t_method)oreceive_free, sizeof(t_oreceive), CLASS_NOINLET, A_DEFSYM, A_DEFFLOAT, 0);
    class_addcreator((t_newmethod)oreceive_new, gensym("r"), A_DEFSYM, A_DEFFLOAT, 0);
    class_addsymbol(oreceive_class, oreceive_name);
    class_addmethod(oreceive_class, (t_method)oreceive_priority, &s_float, A_FLOAT, 0);

    oreceive_proxy_class = class_new(0, 0, 0, sizeof(t_oreceive_proxy), CLASS_NOINLET | CLASS_PD, 0);
    class_addanything(oreceive_proxy_class, oreceive_proxy_anything);
}
