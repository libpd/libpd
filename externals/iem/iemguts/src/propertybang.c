
/******************************************************
 *
 * propertybang - implementation file
 *
 * copyleft (c) IOhannes m zmölnig
 *
 *   2007:forum::für::umläute:2010
 *
 *   institute of electronic music and acoustics (iem)
 *
 ******************************************************
 *
 * license: GNU General Public License v.2
 *
 ******************************************************/


/* 
 * this object provides a way to make an abstraction "property" aware
 * usage:
 *   + put this object into an abstraction
 *   + put the abstraction in a patch
 *   + you can now right-click on the abstraction and select the "properties" menu
 *   + selecting the "properties" menu, will send a bang to the outlet of this object
 *
 * nice, eh?
 */


#include "iemguts-objlist.h"
#include "g_canvas.h"


/* ------------------------- propertybang ---------------------------- */

static t_class *propertybang_class;

typedef struct _propertybang
{
  t_object  x_obj;
} t_propertybang;


t_propertiesfn s_orgfun=NULL;

static void propertybang_free(t_propertybang *x)
{
  removeObjectFromCanvases((t_pd*)x);
}

static void propertybang_bang(t_propertybang *x) {
  outlet_bang(x->x_obj.ob_outlet);
}

static void propertybang_properties(t_gobj*z, t_glist*owner) {
  t_iemguts_objlist*objs=objectsInCanvas((t_pd*)z);
  if(NULL==objs) {
    s_orgfun(z, owner);
  }
  while(objs) {
    t_propertybang*x=(t_propertybang*)objs->obj;
    propertybang_bang(x);
    objs=objs->next;
  } 
}

static void *propertybang_new(void)
{
  t_propertybang *x = (t_propertybang *)pd_new(propertybang_class);
  t_glist *glist=(t_glist *)canvas_getcurrent();
  t_canvas *canvas=(t_canvas*)glist_getcanvas(glist);
  t_class *class = ((t_gobj*)canvas)->g_pd;
  t_propertiesfn orgfun=NULL;
 
  outlet_new(&x->x_obj, &s_bang);

  orgfun=class_getpropertiesfn(class);
  if(orgfun!=propertybang_properties)
    s_orgfun=orgfun;
  
  class_setpropertiesfn(class, propertybang_properties);

  addObjectToCanvas((t_pd*)canvas, (t_pd*)x);
  return (x);
}

void propertybang_setup(void)
{
  propertybang_class = class_new(gensym("propertybang"), (t_newmethod)propertybang_new,
    (t_method)propertybang_free, sizeof(t_propertybang), CLASS_NOINLET, 0);
  class_addbang(propertybang_class, propertybang_bang);

  s_orgfun=NULL;
}
