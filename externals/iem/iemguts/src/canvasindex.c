
/******************************************************
 *
 * canvasindex - implementation file
 *
 * copyleft (c) IOhannes m zmölnig
 *
 *   2007:forum::für::umläute:2007
 *
 *   institute of electronic music and acoustics (iem)
 *
 ******************************************************
 *
 * license: GNU General Public License v.2
 *
 ******************************************************/


/* 
 * this object provides a way to get the position of the containing abstraction
 * within the parent-patch
 * this makes it easy to (dis)connect this abstraction to others
 *
 * by default the index of the containing abstraction within the parent-patch is 
 * queried; however you can give the "depth" as argument:
 * e.g. [canvasindex 1] will give you the index of the abstraction containing the
 * abstraction that holds this object
 */

#include "m_pd.h"
#include "m_imp.h"
#include "g_canvas.h"

int glist_getindex(t_glist *x, t_gobj *y);

/* ------------------------- canvasindex ---------------------------- */

static t_class *canvasindex_class;

typedef struct _canvasindex
{
  t_object  x_obj;
  t_canvas  *x_canvas;
  t_outlet*xoutlet, *youtlet;
} t_canvasindex;

typedef struct _intlist
{
  int value;
  struct _intlist*next;
} t_intlist;

static void canvasindex_symbol(t_canvasindex *x, t_symbol*s)
{
  /* check whether an object of name <s> is in the canvas */
  t_canvas*c=x->x_canvas;
  t_gobj*y;
  int index=0;

  if(!c || !c->gl_owner) return;
  c =c->gl_owner;

  for (y = (t_gobj*)c->gl_list; y; y = y->g_next) /* traverse all objects in canvas */
    {
      t_object*obj=(t_object*)y;
      t_class*obj_c=y->g_pd;
      t_symbol*cname=obj_c->c_name;
      t_binbuf*b=obj->te_binbuf;
      t_atom*ap=binbuf_getvec(b);
      int    ac=binbuf_getnatom(b);
      if(s!=cname && ac) {
	cname=atom_getsymbol(ap);
      }
      if(s==cname){
#warning LATER think about output format
	outlet_float(x->xoutlet, (t_float)index);
      }
      index++;
    }
}


static void canvasindex_float(t_canvasindex *x, t_floatarg f)
{
  /* get the objectname of object #<f> */
  int index=f, cur=0;
  t_canvas*c=x->x_canvas;
  t_gobj*y;

  if(index < 0 || !c || !c->gl_owner) return;
  c =c->gl_owner;

  for (y = (t_gobj*)c->gl_list; y && cur<index; y = y->g_next) /* traverse all objects in canvas */
    {
      cur++;
    }
  if(y) {
      t_object*obj=(t_object*)y;
      t_binbuf*b=obj->te_binbuf;
      t_atom*ap=binbuf_getvec(b);
      int    ac=binbuf_getnatom(b);
      t_atom classatom[1];
      SETSYMBOL(classatom, y->g_pd->c_name);
      /* LATER: shan't we output the index of the object as well? */
      outlet_anything(x->youtlet, gensym("class"), 1, classatom);
      outlet_anything(x->xoutlet, gensym("binbuf"), ac, ap);
  }  
}

static void canvasindex_bang(t_canvasindex *x)
{
  t_canvas*c=x->x_canvas;
  t_canvas*c0=0;

  if(!c) return;
  c0=c->gl_owner;
  if(!c0)return;

  outlet_float(x->youtlet, (t_float)(glist_getindex(c0, 0)));
  outlet_float(x->xoutlet, (t_float)(glist_getindex(c0, (t_gobj*)c)));
}

static void canvasindex_free(t_canvasindex *x)
{
  outlet_free(x->xoutlet);
  outlet_free(x->youtlet);
}

static void *canvasindex_new(t_floatarg f)
{
  t_canvasindex *x = (t_canvasindex *)pd_new(canvasindex_class);
  t_glist *glist=(t_glist *)canvas_getcurrent();
  t_canvas *canvas=(t_canvas*)glist_getcanvas(glist);

  int depth=(int)f;

  if(depth<0)depth=0;
  while(depth && canvas) {
    canvas=canvas->gl_owner;
    depth--;
  }

  x->x_canvas = canvas;

  x->xoutlet=outlet_new(&x->x_obj, &s_float);
  x->youtlet=outlet_new(&x->x_obj, &s_float);

  return (x);
}

void canvasindex_setup(void)
{
  canvasindex_class = class_new(gensym("canvasindex"), 
                                (t_newmethod)canvasindex_new, (t_method)canvasindex_free, 
                                sizeof(t_canvasindex), 0, 
                                A_DEFFLOAT, 0);
  class_addbang(canvasindex_class, (t_method)canvasindex_bang);
  class_addsymbol(canvasindex_class, (t_method)canvasindex_symbol);
  class_addfloat(canvasindex_class, (t_method)canvasindex_float);
}
