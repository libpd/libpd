
/******************************************************
 *
 * canvasposition - implementation file
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
 * this object provides a way to get and set the position of the containing
 * abstraction within the parent-patch
 *
 * by default the position of the containing abstraction within the parent-patch is 
 * queried
 * you can give the "depth" as argument;
 * e.g. [canvasobjectposition 1] will set/get the position of the abstraction containing the
 * abstraction within its canvas.
 */

#include "m_pd.h"

#include "g_canvas.h"
#include "m_imp.h"

/* ------------------------- canvasobjectposition ---------------------------- */

static t_class *canvasobjectposition_class;

typedef struct _canvasobjectposition
{
  t_object  x_obj;
  t_canvas *x_parent;   // the canvas we are acting on
  t_canvas *x_canvas; // an object in the x_canvas selected via the 2nd inlet (index)

  t_outlet*xoutlet, *youtlet;

} t_canvasobjectposition;

static void canvasobjectposition_object(t_canvasobjectposition *x, t_float f);

static void canvasobjectposition_bang(t_canvasobjectposition *x)
{
  t_canvas*c=x->x_canvas;
  t_canvas*c0=x->x_parent;

  int x1=0, y1=0, width=0, height=0;
  t_atom alist[2];

  if(!c) return;

  x1=c->gl_obj.te_xpix;
  y1=c->gl_obj.te_ypix;

  if(NULL!=c0) {
    width= (int)(c0->gl_screenx2 - c0->gl_screenx1);
    height=(int)(c0->gl_screeny2 - c0->gl_screeny1);
  }

  SETFLOAT(alist, (t_float)width);
  SETFLOAT(alist+1, (t_float)height);
  outlet_list(x->youtlet, 0, 2, alist);

  //  outlet_float(x->youtlet, y1);
  SETFLOAT(alist, (t_float)x1);
  SETFLOAT(alist+1, (t_float)y1);
  outlet_list(x->xoutlet, 0, 2, alist);
}

static void canvasobjectposition_list(t_canvasobjectposition *x, t_symbol*s, int argc, t_atom*argv)
{
  t_canvas*c =x->x_canvas;
  t_canvas*c0=x->x_parent;
  int dx, dy;

  if(!c) return;
  //  c0=c->gl_owner;

  if(argc==0){
    canvasobjectposition_bang(x);
    return;
  }

  if(argc!=2 || (A_FLOAT != (argv+0)->a_type) || (A_FLOAT != (argv+1)->a_type)) {
    pd_error(x, "expected <x> <y> as new position");
    return;
  }
  dx = atom_getint(argv+0) - c->gl_obj.te_xpix;
  dy = atom_getint(argv+1) - c->gl_obj.te_ypix;

  
  if(c0&&glist_isvisible(c0))  {
    gobj_displace((t_gobj*)c, c0, dx, dy);
    canvas_fixlinesfor(c0, (t_text*)c);
  } else {
    c->gl_obj.te_xpix+=dx;
    c->gl_obj.te_ypix+=dy;
  }
}

static void canvasobjectposition_free(t_canvasobjectposition *x)
{
  outlet_free(x->xoutlet);
  outlet_free(x->youtlet);
}

static void *canvasobjectposition_new(t_symbol*s, int argc, t_atom*argv)
{
  if(argc>2) {
    error("syntax: canvasobjectposition <canvasdepth> [<objectindex>]");
    return NULL;
  } else {
    t_canvasobjectposition *x = (t_canvasobjectposition *)pd_new(canvasobjectposition_class);
    t_glist *glist=(t_glist *)canvas_getcurrent();
    t_canvas *canvas=(t_canvas*)glist_getcanvas(glist);
    int index=-1;
    int depth=0;
    
    switch(argc) {
    case 2:
      index=atom_getint(argv+1);
    case 1:
      depth=atom_getint(argv);
    default:
      break;
    }

    if(depth<0)depth=0;

    post("depth=%d\tindex=%d", depth, index);

    while(depth && canvas) {
      canvas=canvas->gl_owner;
      depth--;
    }
    
    x->x_parent = canvas;
    x->x_canvas=NULL;
    
    x->xoutlet=outlet_new(&x->x_obj, &s_list);
    x->youtlet=outlet_new(&x->x_obj, &s_list);
    
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("object"));

    if(index>=0)
      canvasobjectposition_object(x, index);

    return (x);
  }
  return NULL;
}


static void canvasobjectposition_object(t_canvasobjectposition *x, t_float f)
{
  t_gobj*obj=NULL;
  int index=(int)f;
  int i=index;
  t_glist*glist=x->x_parent;

  if(x->x_parent==NULL) return;

  if(index<0) return;

  if(NULL==glist) return;

  obj=glist->gl_list;
  while(i-- && obj) {
    obj=obj->g_next;
  }

  x->x_canvas=NULL;
  if(obj) {
    x->x_canvas=(t_canvas*)obj;
  }
}

void canvasobjectposition_setup(void)
{
  canvasobjectposition_class = class_new(gensym("canvasobjectposition"), 
                                   (t_newmethod)canvasobjectposition_new, (t_method)canvasobjectposition_free, 
                                   sizeof(t_canvasobjectposition), 0,
                                   A_GIMME, 0);
  class_addbang(canvasobjectposition_class, (t_method)canvasobjectposition_bang);
  class_addlist(canvasobjectposition_class, (t_method)canvasobjectposition_list);

  class_addmethod(canvasobjectposition_class, (t_method)canvasobjectposition_object, gensym("object"), A_FLOAT, 0);

}
